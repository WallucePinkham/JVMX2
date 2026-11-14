
#include "Endian.h"

#include <cstring>
#include <limits.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Winsock2.h>
#elif defined(__linux__)
#include <arpa/inet.h>
#undef htonl
#undef ntohl
#undef htons
#undef ntohs
#else
#error "You need to find your platform's version of htonl and include the right header here."
#endif

#if CHAR_BIT != 8
#error "unsupported char size"
#endif // CHAR_BIT != 8

enum
{
  O32_LITTLE_ENDIAN = 0x03020100ul,
  O32_BIG_ENDIAN = 0x00010203ul,
  O32_PDP_ENDIAN = 0x01000302ul
};

static const union { unsigned char bytes[ 4 ]; uint32_t value; } o32_host_order =
{ { 0, 1, 2, 3 } };

#define O32_HOST_ORDER (o32_host_order.value)

template <typename Target, typename Source>
Target CopyBits( const Source &source )
{
  static_assert( sizeof(Target) == sizeof(Source), "Bitwise conversions require equal-sized types." );
  Target target;
  std::memcpy( &target, &source, sizeof(target) );
  return target;
}

uint32_t Endian::htonl( uint32_t hostValue )
{
  return ::htonl( hostValue );
}

int32_t Endian::htonl( int32_t hostValue )
{
  uint32_t tempValue = Endian::htonl( CopyBits<uint32_t>( hostValue ) );
  return CopyBits<int32_t>( tempValue );
}

uint32_t Endian::ntohl( uint32_t networkValue )
{
  return ::ntohl( networkValue );
}

int32_t Endian::ntohl( int32_t networkValue )
{
  uint32_t tempValue = Endian::ntohl( CopyBits<uint32_t>( networkValue ) );
  return CopyBits<int32_t>( tempValue );
}

uint64_t Endian::htonll( uint64_t hostValue )
{
  if ( IsBigEndian() )
  {
    return hostValue;
  }

  return (static_cast<uint64_t>( htonl( static_cast<uint32_t>( hostValue ) ) ) << 32)
    | htonl( static_cast<uint32_t>( hostValue >> 32 ) );
}

int64_t Endian::htonll( int64_t hostValue )
{
  uint64_t tempValue = Endian::htonll( CopyBits<uint64_t>( hostValue ) );
  return CopyBits<int64_t>( tempValue );
}

uint64_t Endian::ntohll( uint64_t networkValue )
{
  if ( IsBigEndian() )
  {
    return networkValue;
  }

  return (static_cast<uint64_t>( ntohl( static_cast<uint32_t>( networkValue ) ) ) << 32)
    | ntohl( static_cast<uint32_t>( networkValue >> 32 ) );
}

int64_t Endian::ntohll( int64_t networkValue )
{
  uint64_t tempValue = Endian::ntohll( CopyBits<uint64_t>( networkValue ) );
  return CopyBits<int64_t>( tempValue );
}

uint16_t Endian::htons( uint16_t hostValue )
{
  return ::htons( hostValue );
}

int16_t Endian::htons( int16_t hostValue )
{
  uint16_t tempValue = Endian::htons( CopyBits<uint16_t>( hostValue ) );
  return CopyBits<int16_t>( tempValue );
}

uint16_t Endian::ntohs( uint16_t networkValue )
{
  return ::ntohs( networkValue );
}

int16_t Endian::ntohs( int16_t networkValue )
{
  uint16_t tempValue = Endian::ntohs( CopyBits<uint16_t>( networkValue ) );
  return CopyBits<int16_t>( tempValue );
}

bool Endian::IsBigEndian()
{
  return O32_HOST_ORDER == O32_BIG_ENDIAN;
}
