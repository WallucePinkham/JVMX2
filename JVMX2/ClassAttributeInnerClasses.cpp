#include <stdexcept>

#include "Stream.h"

#include "IndexOutOfBoundsException.h"
#include "JavaCodeAttribute.h"

#include "ClassAttributeInnerClasses.h"

ClassAttributeInnerClasses::ClassAttributeInnerClasses( const JavaString &name, InnerClassList list ) :
  JavaCodeAttribute( name, e_JavaAttributeTypeInnerClasses ),
  m_ClassList( list )
{}

ClassAttributeInnerClasses::ClassAttributeInnerClasses( const ClassAttributeInnerClasses &other ) :
  JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeInnerClasses )
  , m_ClassList( other.m_ClassList )
{}

ClassAttributeInnerClasses::ClassAttributeInnerClasses( ClassAttributeInnerClasses &&other ) :
  JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeInnerClasses )
{
  swap( *this, other );
}

ClassAttributeInnerClasses::~ClassAttributeInnerClasses() JVMX_NOEXCEPT
{}

bool ClassAttributeInnerClasses::Equals( const JavaCodeAttribute &other ) const
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( reinterpret_cast<const ClassAttributeInnerClasses &>(other) );
}

bool ClassAttributeInnerClasses::Equals( const ClassAttributeInnerClasses &other ) const JVMX_NOEXCEPT
{
  auto comparisonFunction = []( const InnerClassListEntry &left, const InnerClassListEntry &right )
  { return left.m_InnerClassInfoIndex == right.m_InnerClassInfoIndex &&
    left.m_OuterClassInfoIndex == right.m_OuterClassInfoIndex &&
    left.m_InnerNameIndex == right.m_InnerNameIndex &&
    left.m_InnerClassAccessFlags == right.m_InnerClassAccessFlags; };

  return std::equal( m_ClassList.begin(), m_ClassList.end(), other.m_ClassList.begin(), comparisonFunction );
}

ClassAttributeInnerClasses ClassAttributeInnerClasses::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t numberOfClasses = byteStream.ReadUint16();
  InnerClassList innerClasses;

  for ( uint16_t i = 0; i < numberOfClasses; ++ i )
  {
    InnerClassListEntry entry;
    entry.m_InnerClassInfoIndex = byteStream.ReadUint16();
    entry.m_OuterClassInfoIndex = byteStream.ReadUint16();
    entry.m_InnerNameIndex = byteStream.ReadUint16();
    entry.m_InnerClassAccessFlags = byteStream.ReadUint16();

    innerClasses.push_back( entry );
  }

  return ClassAttributeInnerClasses( name, innerClasses );
}

DataBuffer ClassAttributeInnerClasses::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_ClassList.size()) );

  for (InnerClassListEntry entry : m_ClassList)
  {
    buffer = buffer.AppendUint16( entry.m_InnerClassInfoIndex );
    buffer = buffer.AppendUint16( entry.m_OuterClassInfoIndex );
    buffer = buffer.AppendUint16( entry.m_InnerNameIndex );
    buffer = buffer.AppendUint16( entry.m_InnerClassAccessFlags );
  }

  return buffer;
}

void ClassAttributeInnerClasses::swap( ClassAttributeInnerClasses &left, ClassAttributeInnerClasses &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_ClassList, right.m_ClassList );
}

uint16_t ClassAttributeInnerClasses::GetNumberOfInnerClasses() const
{
  return static_cast<uint16_t>(m_ClassList.size());
}

ClassAttributeInnerClasses::InnerClassListEntry ClassAttributeInnerClasses::GetInnerClassAt( size_t index ) const
{
  try
  {
    return m_ClassList.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for inner class list entry." );
  }
}