

#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "ClassAttributeSourceDebugExtension.h"

ClassAttributeSourceDebugExtension::ClassAttributeSourceDebugExtension( const ClassAttributeSourceDebugExtension &other )
  : JavaCodeAttribute( other )
  , m_DebugExtension( other.m_DebugExtension )
{}

ClassAttributeSourceDebugExtension::ClassAttributeSourceDebugExtension( const JavaString &name, const DataBuffer &debugExtension )
  : JavaCodeAttribute( name, e_JavaAttributeTypeSourceDebugExtension )
  , m_DebugExtension( debugExtension )
{
}

ClassAttributeSourceDebugExtension::ClassAttributeSourceDebugExtension( ClassAttributeSourceDebugExtension &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeSourceDebugExtension )
  , m_DebugExtension( DataBuffer::EmptyBuffer() )
{
  swap( *this, other );
}

ClassAttributeSourceDebugExtension::~ClassAttributeSourceDebugExtension()
{}

ClassAttributeSourceDebugExtension ClassAttributeSourceDebugExtension::FromBinary( JavaString name, Stream &buffer )
{
  uint16_t length = buffer.ReadUint16();

  return ClassAttributeSourceDebugExtension( name, buffer.ReadBytes( length ) );
}

void ClassAttributeSourceDebugExtension::swap( ClassAttributeSourceDebugExtension &left, ClassAttributeSourceDebugExtension &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_DebugExtension, right.m_DebugExtension );
}

DataBuffer ClassAttributeSourceDebugExtension::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_DebugExtension.GetByteLength()) );
  return buffer.Append( m_DebugExtension );
}

bool ClassAttributeSourceDebugExtension::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const ClassAttributeSourceDebugExtension &>(other) );
}

bool ClassAttributeSourceDebugExtension::Equals( const ClassAttributeSourceDebugExtension &other ) const JVMX_NOEXCEPT
{
  return m_DebugExtension == other.GetDebugExtension();
}

const DataBuffer &ClassAttributeSourceDebugExtension::GetDebugExtension() const JVMX_NOEXCEPT
{
  return m_DebugExtension;
}



