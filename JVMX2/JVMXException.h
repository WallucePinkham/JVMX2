#pragma once

#ifndef __JVMXEXCEPTION_H__
#define __JVMXEXCEPTION_H__

#include <stdexcept>

#include "GlobalConstants.h"

class JVMXException : public std::runtime_error
{
public:
  JVMXException( const char *pMessage ) : std::runtime_error( pMessage ), m_ErrorCode( 0 )
  {}

  JVMXException( const char *pMessage, int errorCode ) : std::runtime_error( pMessage ), m_ErrorCode( errorCode )
  {}

  int GetErrorCode() const
  {
    return m_ErrorCode;
  }

  virtual ~JVMXException() JVMX_NOEXCEPT {};

private:
  int m_ErrorCode;
};


#endif // __JVMXEXCEPTION_H__
