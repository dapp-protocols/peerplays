#include <fc/crypto/hex.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/sha.h>
#include <string.h>
#include <cmath>
#include <fc/crypto/sha256.hpp>
#include <fc/variant.hpp>
#include <fc/exception/exception.hpp>
#include "_digest_common.hpp"

namespace fc {

    sha256::sha256() { memset( _hash, 0, sizeof(_hash) ); }
    sha256::sha256( const char *data, size_t size ) { 
       if (size != sizeof(_hash))	 
	  FC_THROW_EXCEPTION( exception, "sha256: size mismatch" );
       memcpy(_hash, data, size );
    }
    sha256::sha256( const string& hex_str ) {
      fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
    }

    string sha256::str()const {
      return fc::to_hex( (char*)_hash, sizeof(_hash) );
    }
    sha256::operator string()const { return  str(); }

    char* sha256::data()const { return (char*)&_hash[0]; }


    struct sha256::encoder::impl {
       SHA256_CTX ctx;
    };

    sha256::encoder::~encoder() {}
    sha256::encoder::encoder() {
      reset();
    }

    sha256 sha256::hash( const char* d, uint32_t dlen ) {
      encoder e;
      e.write(d,dlen);
      return e.result();
    }

    sha256 sha256::hash( const string& s ) {
      return hash( s.c_str(), s.size() );
    }

    sha256 sha256::hash( const sha256& s )
    {
        return hash( s.data(), sizeof( s._hash ) );
    }

    void sha256::encoder::write( const char* d, uint32_t dlen ) {
      SHA256_Update( &my->ctx, d, dlen); 
    }
    sha256 sha256::encoder::result() {
      sha256 h;
      SHA256_Final((uint8_t*)h.data(), &my->ctx );
      return h;
    }
    void sha256::encoder::reset() {
      SHA256_Init( &my->ctx);  
    }

    sha256 operator << ( const sha256& h1, uint32_t i ) {
      sha256 result;
      fc::detail::shift_l( h1.data(), result.data(), result.data_size(), i );
      return result;
    }
    sha256 operator >> ( const sha256& h1, uint32_t i ) {
      sha256 result;
      fc::detail::shift_r( h1.data(), result.data(), result.data_size(), i );
      return result;
    }
    sha256 operator ^ ( const sha256& h1, const sha256& h2 ) {
      sha256 result;
      result._hash[0] = h1._hash[0].value() ^ h2._hash[0].value();
      result._hash[1] = h1._hash[1].value() ^ h2._hash[1].value();
      result._hash[2] = h1._hash[2].value() ^ h2._hash[2].value();
      result._hash[3] = h1._hash[3].value() ^ h2._hash[3].value();
      return result;
    }
    bool operator >= ( const sha256& h1, const sha256& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
    }
    bool operator > ( const sha256& h1, const sha256& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
    }
    bool operator < ( const sha256& h1, const sha256& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
    }
    bool operator != ( const sha256& h1, const sha256& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) != 0;
    }
    bool operator == ( const sha256& h1, const sha256& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) == 0;
    }

   void to_variant( const sha256& bi, variant& v, uint32_t max_depth )
   {
      to_variant( std::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) ), v, max_depth );
   }
   void from_variant( const variant& v, sha256& bi, uint32_t max_depth )
   {
      std::vector<char> ve = v.as< std::vector<char> >( max_depth );
      for (size_t i = 0; i < bi.data_size(); i++) {
         bi.data()[i] = 0;
      }
      if( ve.size() )
         memcpy( &bi, ve.data(), std::min<size_t>(ve.size(),sizeof(bi)) );
  }

    template<>
    unsigned int hmac<sha256>::internal_block_size() const { return 64; }
} //end namespace fc
