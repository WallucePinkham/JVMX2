
#ifndef __JAVAATTRIBUTEUNKOWN_H__
#define __JAVAATTRIBUTEUNKOWN_H__

#include "GlobalConstants.h"
#include "DataBuffer.h"

class JavaAttributeUnknown JVMX_ABSTRACT
{
protected:
  JavaAttributeUnknown( uint32_t length, const uint8_t *pBuffer );
  JavaAttributeUnknown();

public:
  JavaAttributeUnknown( const JavaAttributeUnknown &other );
  JavaAttributeUnknown( JavaAttributeUnknown &&other );

  virtual ~JavaAttributeUnknown() JVMX_NOEXCEPT;

  virtual uint32_t GetLength() const JVMX_NOEXCEPT;
  virtual const uint8_t *GetValue() const JVMX_NOEXCEPT;

protected:
  virtual void swap( JavaAttributeUnknown &left, JavaAttributeUnknown &right ) JVMX_NOEXCEPT;

protected:
  DataBuffer m_Data;
};

#endif // __JAVAATTRIBUTEUNKOWN_H__

