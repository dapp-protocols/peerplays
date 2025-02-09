#include <fc/crypto/base58.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/crypto/openssl.hpp>
#include <fc/crypto/ripemd160.hpp>

#ifdef _WIN32
# include <malloc.h>
#endif

/* stuff common to all ecc implementations */

#define BTC_EXT_PUB_MAGIC   (0x0488B21E)
#define BTC_EXT_PRIV_MAGIC  (0x0488ADE4)

namespace fc { namespace ecc {

    namespace detail {
        typedef zero_initialized_array<unsigned char,37> chr37;

        fc::sha256 _left( const fc::sha512& v )
        {
            fc::sha256 result;
            memcpy( result.data(), v.data(), 32 );
            return result;
        }

        fc::sha256 _right( const fc::sha512& v )
        {
            fc::sha256 result;
            memcpy( result.data(), v.data() + 32, 32 );
            return result;
        }

        static void _put( unsigned char** dest, unsigned int i)
        {
            *(*dest)++ = (i >> 24) & 0xff;
            *(*dest)++ = (i >> 16) & 0xff;
            *(*dest)++ = (i >>  8) & 0xff;
            *(*dest)++ =  i        & 0xff;
        }

        static unsigned int _get( unsigned char** src )
        {
            unsigned int result = *(*src)++ << 24;
            result |= *(*src)++ << 16;
            result |= *(*src)++ <<  8;
            result |= *(*src)++;
            return result;
        }

        static chr37 _derive_message( unsigned char first, const unsigned char* key32, int i )
        {
            chr37 result;
            unsigned char* dest = result.data();
            *dest++ = first;
            memcpy( dest, key32, 32 ); dest += 32;
            _put( &dest, i );
            return result;
        }

        chr37 _derive_message( const public_key_data& key, int i )
        {
            return _derive_message( *key.data(), key.data() + 1, i );
        }

        static chr37 _derive_message( const private_key_secret& key, int i )
        {
            return _derive_message( 0, (unsigned char*) key.data(), i );
        }

        const ec_group& get_curve()
        {
            static const ec_group secp256k1( EC_GROUP_new_by_curve_name( NID_secp256k1 ) );
            return secp256k1;
        }

        static private_key_secret _get_curve_order()
        {
            const ec_group& group = get_curve();
            bn_ctx ctx(BN_CTX_new());
            ssl_bignum order;
            FC_ASSERT( EC_GROUP_get_order( group, order, ctx ) );
            private_key_secret bin;
            size_t order_BN_num_bytes = BN_num_bytes( order );
            FC_ASSERT( order_BN_num_bytes == bin.data_size() );
            size_t order_BN_bn2bin = BN_bn2bin( order, (unsigned char*) bin.data() );
            FC_ASSERT( order_BN_bn2bin == bin.data_size() );
            return bin;
        }

        const private_key_secret& get_curve_order()
        {
            static private_key_secret order = _get_curve_order();
            return order;
        }

        static private_key_secret _get_half_curve_order()
        {
            const ec_group& group = get_curve();
            bn_ctx ctx(BN_CTX_new());
            ssl_bignum order;
            FC_ASSERT( EC_GROUP_get_order( group, order, ctx ) );
            BN_rshift1( order, order );
            private_key_secret bin;
            size_t order_BN_num_bytes = BN_num_bytes( order );
            FC_ASSERT( order_BN_num_bytes == bin.data_size() );
            size_t order_BN_bn2bin = BN_bn2bin( order, (unsigned char*) bin.data() );
            FC_ASSERT( order_BN_bn2bin == bin.data_size() );
            return bin;
        }

        const private_key_secret& get_half_curve_order()
        {
            static private_key_secret half_order = _get_half_curve_order();
            return half_order;
        }
    }

    public_key public_key::from_key_data( const public_key_data &data ) {
        return public_key(data);
    }

    public_key public_key::child( const fc::sha256& offset )const
    {
       fc::sha256::encoder enc;
       fc::raw::pack( enc, *this );
       fc::raw::pack( enc, offset );

       return add( enc.result() );
    }

    private_key private_key::child( const fc::sha256& offset )const
    {
       fc::sha256::encoder enc;
       fc::raw::pack( enc, get_public_key() );
       fc::raw::pack( enc, offset );
       return generate_from_seed( get_secret(), enc.result() );
    }

    std::string public_key::to_base58( const public_key_data &key )
    {
      sha256 check = sha256::hash((char*) key.data(), sizeof(key));
      static_assert(sizeof(key) + 4 == 37, "Elliptic public key size (or its hash) is incorrect");
      detail::chr37 data;
      memcpy(data.data(), key.data(), key.size());
      memcpy(data.data() + key.size(), (const char*)check._hash, 4);
      return fc::to_base58((char*) data.data(), data.size());
    }

    public_key public_key::from_base58( const std::string& b58 )
    {
        detail::chr37 data;
        size_t s = fc::from_base58(b58, (char*)&data, sizeof(data) );
        FC_ASSERT( s == sizeof(data) );

        public_key_data key;
        sha256 check = sha256::hash((char*) data.data(), sizeof(key));
        FC_ASSERT( memcmp( (char*)check._hash, data.data() + key.size(), 4 ) == 0 );
        memcpy( (char*)key.data(), data.data(), key.size() );
        return from_key_data(key);
    }

    unsigned int public_key::fingerprint() const
    {
        public_key_data key = serialize();
        ripemd160 hash = ripemd160::hash( sha256::hash( (char*) key.data(), key.size() ) );
        unsigned char* fp = (unsigned char*) hash._hash;
        return (fp[0] << 24) | (fp[1] << 16) | (fp[2] << 8) | fp[3];
    }

    bool public_key::is_canonical( const compact_signature& c ) {
        return !(c[1] & 0x80)
               && !(c[1] == 0 && !(c[2] & 0x80))
               && !(c[33] & 0x80)
               && !(c[33] == 0 && !(c[34] & 0x80));
    }

    private_key private_key::generate_from_seed( const fc::sha256& seed, const fc::sha256& offset )
    {
        ssl_bignum z;
        BN_bin2bn((unsigned char*)&offset, sizeof(offset), z);

        ec_group group(EC_GROUP_new_by_curve_name(NID_secp256k1));
        bn_ctx ctx(BN_CTX_new());
        ssl_bignum order;
        EC_GROUP_get_order(group, order, ctx);

        // secexp = (seed + z) % order
        ssl_bignum secexp;
        BN_bin2bn((unsigned char*)&seed, sizeof(seed), secexp);
        BN_add(secexp, secexp, z);
        BN_mod(secexp, secexp, order, ctx);

        fc::sha256 secret;
        FC_ASSERT(BN_num_bytes(secexp) <= int64_t(sizeof(secret)));
        auto shift = sizeof(secret) - BN_num_bytes(secexp);
        BN_bn2bin(secexp, ((unsigned char*)&secret)+shift);
        return regenerate( secret );
    }

    fc::sha256 private_key::get_secret( const EC_KEY * const k )
    {
       if( !k )
       {
          return fc::sha256();
       }

       fc::sha256 sec;
       const BIGNUM* bn = EC_KEY_get0_private_key(k);
       if( bn == NULL )
       {
         FC_THROW_EXCEPTION( exception, "get private key failed" );
       }
       int nbytes = BN_num_bytes(bn);
       BN_bn2bin(bn, &((unsigned char*)&sec)[32-nbytes] );
       return sec;
    }

    private_key private_key::generate()
    {
       EC_KEY* k = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !k ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );
       if( !EC_KEY_generate_key( k ) )
       {
          FC_THROW_EXCEPTION( exception, "ecc key generation error" );

       }

       return private_key( k );
    }

    static std::string _to_base58( const extended_key_data& key )
    {
        char buffer[std::tuple_size<extended_key_data>::value + 4]; // it's a small static array => allocate on stack
        memcpy( buffer, key.data(), key.size() );
        fc::sha256 double_hash = fc::sha256::hash( fc::sha256::hash( (char*)key.data(), key.size() ));
        memcpy( buffer + key.size(), double_hash.data(), 4 );
        return fc::to_base58( buffer, sizeof(buffer) );
    }

    static void _parse_extended_data( unsigned char* buffer, std::string base58 )
    {
        memset( buffer, 0, 78 );
        std::vector<char> decoded = fc::from_base58( base58 );
        unsigned int i = 0;
        for ( char c : decoded )
        {
            if ( i >= 78 || i > decoded.size() - 4 ) { break; }
            buffer[i++] = c;
        }
    }

    extended_public_key extended_public_key::derive_child(int i) const
    {
        FC_ASSERT( !(i&0x80000000), "Can't derive hardened public key!" );
        return derive_normal_child(i);
    }

    extended_key_data extended_public_key::serialize_extended() const
    {
        extended_key_data result;
        unsigned char* dest = (unsigned char*) result.data();
        detail::_put( &dest, BTC_EXT_PUB_MAGIC );
        *dest++ = depth;
        detail::_put( &dest, parent_fp );
        detail::_put( &dest, child_num );
        memcpy( dest, c.data(), c.data_size() ); dest += 32;
        public_key_data key = serialize();
        memcpy( dest, key.data(), key.size() );
        return result;
    }
    
    extended_public_key extended_public_key::deserialize( const extended_key_data& data )
    {
       return from_base58( _to_base58( data ) );
    }

    std::string extended_public_key::str() const
    {
        return _to_base58( serialize_extended() );
    }

    extended_public_key extended_public_key::from_base58( const std::string& base58 )
    {
        unsigned char buffer[78];
        unsigned char* ptr = buffer;
        _parse_extended_data( buffer, base58 );
        FC_ASSERT( detail::_get( &ptr ) == BTC_EXT_PUB_MAGIC, "Invalid extended private key" );
        uint8_t d = *ptr++;
        int fp = detail::_get( &ptr );
        int cn = detail::_get( &ptr );
        fc::sha256 chain;
        memcpy( chain.data(), ptr, chain.data_size() ); ptr += chain.data_size();
        public_key_data key;
        memcpy( key.data(), ptr, key.size() );
        return extended_public_key( key, chain, cn, fp, d );
    }

    extended_public_key extended_private_key::get_extended_public_key() const
    {
        return extended_public_key( get_public_key(), c, child_num, parent_fp, depth );
    }

    extended_private_key extended_private_key::derive_child(int i) const
    {
        return i < 0 ? derive_hardened_child(i) : derive_normal_child(i);
    }

    extended_private_key extended_private_key::derive_normal_child(int i) const
    {
        const detail::chr37 data = detail::_derive_message( get_public_key().serialize(), i );
        hmac_sha512 mac;
        fc::sha512 l = mac.digest( c.data(), c.data_size(), (char*) data.data(), data.size() );
        return private_derive_rest( l, i );
    }

    extended_private_key extended_private_key::derive_hardened_child(int i) const
    {
        hmac_sha512 mac;
        private_key_secret key = get_secret();
        const detail::chr37 data = detail::_derive_message( key, i );
        fc::sha512 l = mac.digest( c.data(), c.data_size(), (char*) data.data(), data.size() );
        return private_derive_rest( l, i );
    }

    extended_key_data extended_private_key::serialize_extended() const
    {
        extended_key_data result;
        unsigned char* dest = (unsigned char*) result.data();
        detail::_put( &dest, BTC_EXT_PRIV_MAGIC );
        *dest++ = depth;
        detail::_put( &dest, parent_fp );
        detail::_put( &dest, child_num );
        memcpy( dest, c.data(), c.data_size() ); dest += 32;
        *dest++ = 0;
        private_key_secret key = get_secret();
        memcpy( dest, key.data(), key.data_size() );
        return result;
    }
    
    extended_private_key extended_private_key::deserialize( const extended_key_data& data )
    {
       return from_base58( _to_base58( data ) );
    }

    std::string extended_private_key::str() const
    {
        return _to_base58( serialize_extended() );
    }

    extended_private_key extended_private_key::from_base58( const std::string& base58 )
    {
        unsigned char buffer[78];
        unsigned char* ptr = buffer;
        _parse_extended_data( buffer, base58 );
        FC_ASSERT( detail::_get( &ptr ) == BTC_EXT_PRIV_MAGIC, "Invalid extended private key" );
        uint8_t d = *ptr++;
        int fp = detail::_get( &ptr );
        int cn = detail::_get( &ptr );
        fc::sha256 chain;
        memcpy( chain.data(), ptr, chain.data_size() ); ptr += chain.data_size();
        ptr++;
        private_key_secret key;
        memcpy( key.data(), ptr, key.data_size() );
        return extended_private_key( private_key::regenerate(key), chain, cn, fp, d );
    }

    extended_private_key extended_private_key::generate_master( const std::string& seed )
    {
        return generate_master( seed.c_str(), seed.size() );
    }

    extended_private_key extended_private_key::generate_master( const char* seed, uint32_t seed_len )
    {
        hmac_sha512 mac;
        fc::sha512 hash = mac.digest( "Bitcoin seed", 12, seed, seed_len );
        extended_private_key result( private_key::regenerate( detail::_left(hash) ),
                                     detail::_right(hash) );
        return result;
    }
}

void to_variant( const ecc::private_key& var, variant& vo, uint32_t max_depth )
{
    to_variant( var.get_secret(), vo, max_depth );
}

void from_variant( const variant& var,  ecc::private_key& vo, uint32_t max_depth )
{
    fc::sha256 sec;
    from_variant( var, sec, max_depth );
    vo = ecc::private_key::regenerate(sec);
}

void to_variant( const ecc::public_key& var, variant& vo, uint32_t max_depth )
{
    to_variant( var.serialize(), vo, max_depth );
}

void from_variant( const variant& var,  ecc::public_key& vo, uint32_t max_depth )
{
    ecc::public_key_data dat;
    from_variant( var, dat, max_depth );
    vo = ecc::public_key(dat);
}

}
