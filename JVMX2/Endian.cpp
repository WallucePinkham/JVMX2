
#include "Endian.h"

#ifdef WIN32
#include <Winsock2.h>
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

union UnionOfUint64AndUint32
{
  uint32_t uint32Values[ 2 ];
  uint64_t uint64Values;
};

uint32_t Endian::htonl( uint32_t hostValue )
{
  return ::htonl( hostValue );
}

int32_t Endian::htonl( int32_t hostValue )
{
  uint32_t tempValue = Endian::htonl( *reinterpret_cast<uint32_t*>(&hostValue) );
  return *reinterpret_cast<int32_t*>(&tempValue);
}

uint32_t Endian::ntohl( uint32_t networkValue )
{
  return ::ntohl( networkValue );
}

int32_t Endian::ntohl( int32_t networkValue )
{
  uint32_t tempValue = Endian::ntohl( *reinterpret_cast<uint32_t*>(&networkValue) );
  return *reinterpret_cast<int32_t*>(&tempValue);
}

uint64_t Endian::htonll( uint64_t hostValue )
{
  UnionOfUint64AndUint32 u;

  u.uint32Values[ 0 ] = htonl( static_cast<uint32_t>(hostValue >> 32) );
  u.uint32Values[ 1 ] = htonl( static_cast<uint32_t>(hostValue & 0xFFFFFFFFULL) );

  return u.uint64Values;
}

int64_t Endian::htonll( int64_t hostValue )
{
  uint64_t tempValue = Endian::htonll( *reinterpret_cast<uint64_t*>(&hostValue) );
  return *reinterpret_cast<int64_t*>(&tempValue);
}

uint64_t Endian::ntohll( uint64_t networkValue )
{
  UnionOfUint64AndUint32 u;

  u.uint64Values = networkValue;

  return ((unsigned long long)ntohl( u.uint32Values[ 0 ] ) << 32) | (unsigned long long)ntohl( u.uint32Values[ 1 ] );
}

int64_t Endian::ntohll( int64_t networkValue )
{
  uint64_t tempValue = Endian::ntohll( *reinterpret_cast<uint64_t*>(&networkValue) );
  return *reinterpret_cast<int64_t*>(&tempValue);
}

uint16_t Endian::htons( uint16_t hostValue )
{
  return ::htons( hostValue );
}

int16_t Endian::htons( int16_t hostValue )
{
  uint16_t tempValue = Endian::htons( *reinterpret_cast<uint16_t*>(&hostValue) );
  return *reinterpret_cast<int16_t*>(&tempValue);
}

uint16_t Endian::ntohs( uint16_t networkValue )
{
  return ::ntohs( networkValue );
}

int16_t Endian::ntohs( int16_t networkValue )
{
  uint16_t tempValue = Endian::ntohs( *reinterpret_cast<uint16_t*>(&networkValue) );
  return *reinterpret_cast<int16_t*>(&tempValue);
}

bool Endian::IsBigEndian()
{
  return O32_HOST_ORDER == O32_BIG_ENDIAN;
}
