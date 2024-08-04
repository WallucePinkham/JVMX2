#include <stdexcept>

#include "Stream.h"

#include "IndexOutOfBoundsException.h"
#include "ClassAttributeBootstrapMethods.h"

ClassAttributeBootstrapMethods::ClassAttributeBootstrapMethods( const JavaString &name,
  BootstrapMethodList list )
  : JavaCodeAttribute( name, e_JavaAttributeTypeInnerClasses )
  , m_MethodList( list )
{}

ClassAttributeBootstrapMethods::ClassAttributeBootstrapMethods( const ClassAttributeBootstrapMethods &other )
  : JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeInnerClasses )
  , m_MethodList( other.m_MethodList )
{}

ClassAttributeBootstrapMethods::ClassAttributeBootstrapMethods( ClassAttributeBootstrapMethods &&other )
  : JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeInnerClasses )
{
  swap( *this, other );
}

ClassAttributeBootstrapMethods::~ClassAttributeBootstrapMethods() JVMX_NOEXCEPT
{}

bool ClassAttributeBootstrapMethods::Equals( const JavaCodeAttribute &other ) const
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( reinterpret_cast<const ClassAttributeBootstrapMethods &>(other) );
}

bool ClassAttributeBootstrapMethods::Equals( const ClassAttributeBootstrapMethods &other ) const JVMX_NOEXCEPT
{
  auto comparisonFunction = []( const BootstrapMethodsListEntry & left,
    const BootstrapMethodsListEntry & right )
  {
    return left.m_BootstrapMethodIndex == right.m_BootstrapMethodIndex &&
      left.m_NumberOfBootstrapArguments == right.m_NumberOfBootstrapArguments &&
      left.m_BootstrapArguments == right.m_BootstrapArguments;
  };

  return std::equal( m_MethodList.begin(), m_MethodList.end(), other.m_MethodList.begin(),
    comparisonFunction );
}

ClassAttributeBootstrapMethods ClassAttributeBootstrapMethods::FromBinary( JavaString name,
  Stream &byteStream )
{
  uint16_t numberOfMethods = byteStream.ReadUint16();
  BootstrapMethodList innerClasses;

  for ( uint16_t methodIndex = 0; methodIndex < numberOfMethods; ++methodIndex )
  {
    BootstrapMethodsListEntry entry;
    entry.m_BootstrapMethodIndex = byteStream.ReadUint16();
    entry.m_NumberOfBootstrapArguments = byteStream.ReadUint16();

    for ( uint16_t parameterIndex = 0; parameterIndex < entry.m_NumberOfBootstrapArguments; ++parameterIndex )
    {
      entry.m_BootstrapArguments.push_back( byteStream.ReadUint16() );
    }

    innerClasses.push_back( entry );
  }

  return ClassAttributeBootstrapMethods( name, innerClasses );
}

DataBuffer ClassAttributeBootstrapMethods::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_MethodList.size()) );

  for (BootstrapMethodsListEntry entry : m_MethodList)
  {
    buffer = buffer.AppendUint16( entry.m_BootstrapMethodIndex );
    buffer = buffer.AppendUint16( entry.m_NumberOfBootstrapArguments );

    for (uint16_t parameter : entry.m_BootstrapArguments)
    {
      buffer = buffer.AppendUint16( parameter );
    }
  }

  return buffer;
}

void ClassAttributeBootstrapMethods::swap( ClassAttributeBootstrapMethods &left,
  ClassAttributeBootstrapMethods &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_MethodList, right.m_MethodList );
}

uint16_t ClassAttributeBootstrapMethods::GetNumberOBootstrapMethods() const
{
  return static_cast<uint16_t>(m_MethodList.size());
}

ClassAttributeBootstrapMethods::BootstrapMethodsListEntry
ClassAttributeBootstrapMethods::GetBootstrapMethodAt( size_t index ) const
{
  try
  {
    return m_MethodList.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException(
      __FUNCTION__ " - Index out of bounds looking for inner class list entry." );
  }
}