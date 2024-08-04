
#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include "GlobalConstants.h"

class Endian
{
public:
  static uint16_t htons( uint16_t hostValue );
  static uint16_t ntohs( uint16_t networkValue );

  static int16_t htons( int16_t hostValue );
  static int16_t ntohs( int16_t networkValue );

  static uint32_t htonl( uint32_t hostValue );
  static uint32_t ntohl( uint32_t networkValue );

  static int32_t htonl( int32_t hostValue );
  static int32_t ntohl( int32_t networkValue );

  static uint64_t htonll( uint64_t hostValue );
  static uint64_t ntohll( uint64_t networkValue );

  static int64_t htonll( int64_t hostValue );
  static int64_t ntohll( int64_t networkValue );
  static bool IsBigEndian();
};

#endif // __ENDIAN_H__

