#include <fc/crypto/hex.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/sha.h>
#include <string.h>
#include <fc/crypto/sha1.hpp>
#include <fc/variant.hpp>
#include <vector>
#include "_digest_common.hpp"

namespace fc 
{
  
sha1::sha1() { memset( _hash, 0, sizeof(_hash) ); }
sha1::sha1( const string& hex_str ) {
  fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
}

string sha1::str()const {
  return fc::to_hex( (char*)_hash, sizeof(_hash) );
}
sha1::operator string()const { return  str(); }

char* sha1::data()const { return (char*)&_hash[0]; }


struct sha1::encoder::impl {
   SHA_CTX ctx;
};

sha1::encoder::~encoder() {}
sha1::encoder::encoder() {
  reset();
}

sha1 sha1::hash( const char* d, uint32_t dlen ) {
  encoder e;
  e.write(d,dlen);
  return e.result();
}
sha1 sha1::hash( const string& s ) {
  return hash( s.c_str(), s.size() );
}

void sha1::encoder::write( const char* d, uint32_t dlen ) {
  SHA1_Update( &my->ctx, d, dlen); 
}
sha1 sha1::encoder::result() {
  sha1 h;
  SHA1_Final((uint8_t*)h.data(), &my->ctx );
  return h;
}
void sha1::encoder::reset() {
  SHA1_Init( &my->ctx);  
}

sha1 operator << ( const sha1& h1, uint32_t i ) {
  sha1 result;
  fc::detail::shift_l( h1.data(), result.data(), result.data_size(), i );
  return result;
}
sha1 operator ^ ( const sha1& h1, const sha1& h2 ) {
  sha1 result;
  result._hash[0] = h1._hash[0].value() ^ h2._hash[0].value();
  result._hash[1] = h1._hash[1].value() ^ h2._hash[1].value();
  result._hash[2] = h1._hash[2].value() ^ h2._hash[2].value();
  result._hash[3] = h1._hash[3].value() ^ h2._hash[3].value();
  result._hash[4] = h1._hash[4].value() ^ h2._hash[4].value();
  return result;
}
bool operator >= ( const sha1& h1, const sha1& h2 ) {
  return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
}
bool operator > ( const sha1& h1, const sha1& h2 ) {
  return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
}
bool operator < ( const sha1& h1, const sha1& h2 ) {
  return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
}
bool operator != ( const sha1& h1, const sha1& h2 ) {
  return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) != 0;
}
bool operator == ( const sha1& h1, const sha1& h2 ) {
  return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) == 0;
}
  
  void to_variant( const sha1& bi, variant& v, uint32_t max_depth )
  {
     to_variant( std::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) ), v, max_depth );
  }
  void from_variant( const variant& v, sha1& bi, uint32_t max_depth )
  {
    std::vector<char> ve = v.as< std::vector<char> >( max_depth );
    for (size_t i = 0; i < bi.data_size(); i++) {
       bi.data()[i] = 0;
    }
    if( ve.size() )
       memcpy( &bi, ve.data(), std::min<size_t>(ve.size(),sizeof(bi)) );
  }
  
} // fc
