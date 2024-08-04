
#include "Stream.h"

#include "ClassAttributeEnclosingMethod.h"

ClassAttributeEnclosingMethod::ClassAttributeEnclosingMethod( const JavaString &name, ConstantPoolIndex classIndex, ConstantPoolIndex methodIndex ) :
JavaCodeAttribute( name, e_JavaAttributeTypeEnclosingMethod )
, m_ClassIndex( classIndex )
, m_MethodIndex( methodIndex )
{
}

ClassAttributeEnclosingMethod::ClassAttributeEnclosingMethod( const ClassAttributeEnclosingMethod &other ) :
JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeEnclosingMethod )
, m_ClassIndex( other.m_ClassIndex )
, m_MethodIndex( other.m_MethodIndex )
{
}

ClassAttributeEnclosingMethod::~ClassAttributeEnclosingMethod() JVMX_NOEXCEPT
{
}

ClassAttributeEnclosingMethod::ClassAttributeEnclosingMethod( ClassAttributeEnclosingMethod &&other ) :
JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeEnclosingMethod )
{
  swap( *this, other );
}

bool ClassAttributeEnclosingMethod::Equals( const JavaCodeAttribute &other ) const
{
  if ( m_Type != other.GetType() )
  {
    return false;
  }

  return Equals( reinterpret_cast<const ClassAttributeEnclosingMethod &>(other) );
}

bool ClassAttributeEnclosingMethod::Equals( const ClassAttributeEnclosingMethod &other ) const JVMX_NOEXCEPT
{
  return m_ClassIndex == other.m_ClassIndex &&
  m_MethodIndex == other.m_MethodIndex;
}

ConstantPoolIndex ClassAttributeEnclosingMethod::GetClassIndex() const JVMX_NOEXCEPT
{
  return m_ClassIndex;
}

ConstantPoolIndex ClassAttributeEnclosingMethod::GetMethodIndex() const JVMX_NOEXCEPT
{
  return m_MethodIndex;
}


ClassAttributeEnclosingMethod ClassAttributeEnclosingMethod::FromBinary( JavaString name, Stream &byteStream )
{
  ConstantPoolIndex classIndex = byteStream.ReadUint16();
  ConstantPoolIndex methodIndex = byteStream.ReadUint16();

  return ClassAttributeEnclosingMethod( name, classIndex, methodIndex );
}

DataBuffer ClassAttributeEnclosingMethod::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( m_ClassIndex );
  buffer = buffer.AppendUint16( m_MethodIndex );

  return buffer;
}

void ClassAttributeEnclosingMethod::swap( ClassAttributeEnclosingMethod &left, ClassAttributeEnclosingMethod &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_ClassIndex, right.m_ClassIndex );
  std::swap( left.m_MethodIndex, right.m_MethodIndex );
}
