#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "JavaString.h"
#include "Stream.h"
#include "StackFrameFactory.h"

#include "CodeAttributeStackMapTable.h"

CodeAttributeStackMapTable::CodeAttributeStackMapTable( StackFrameList stackFrames ) :
  JavaCodeAttribute( JavaString::FromCString( JVMX_T( "StackMapTable" ) ), e_JavaAttributeTypeStackMapTable )
  , m_StackFrames( stackFrames )
{}

CodeAttributeStackMapTable::CodeAttributeStackMapTable( CodeAttributeStackMapTable &&other ) :
  JavaCodeAttribute( JavaString::FromCString( JVMX_T( "StackMapTable" ) ), e_JavaAttributeTypeStackMapTable )
{
  swap( *this, other );
}

CodeAttributeStackMapTable::CodeAttributeStackMapTable( const CodeAttributeStackMapTable &other ) :
  JavaCodeAttribute( JavaString::FromCString( JVMX_T( "StackMapTable" ) ), e_JavaAttributeTypeStackMapTable )
  , m_StackFrames( other.m_StackFrames )
{}

CodeAttributeStackMapTable::~CodeAttributeStackMapTable() JVMX_NOEXCEPT
{}

bool CodeAttributeStackMapTable::Equals( const JavaCodeAttribute &other ) const
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeStackMapTable &>(other) );
}

bool CodeAttributeStackMapTable::Equals( const CodeAttributeStackMapTable &other ) const JVMX_NOEXCEPT
{
  return m_StackFrames.size() == other.m_StackFrames.size() &&
    std::equal( m_StackFrames.cbegin(), m_StackFrames.cend(), other.m_StackFrames.cbegin(), []( const std::shared_ptr<StackFrame> pLeft, const std::shared_ptr<StackFrame> pRight ) { return *pLeft == *pRight; } );
}

size_t CodeAttributeStackMapTable::GetStackFrameCount() const
{
  return m_StackFrames.size();
}

const StackFrame &CodeAttributeStackMapTable::GetStackFrame( size_t index ) const
{
  try
  {
    return *(m_StackFrames.at( index ));
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds for stack frame in StackMapTable." );
  }
}

CodeAttributeStackMapTable CodeAttributeStackMapTable::FromBinary( Stream &byteStream )
{
  uint16_t numberOfEntries = byteStream.ReadUint16();
  StackFrameFactory factory;

  StackFrameList stackFrames;
  for ( uint16_t index = 0; index < numberOfEntries; ++ index )
  {
    stackFrames.push_back( factory.CreateStackFrame( byteStream ) );
  }

  return CodeAttributeStackMapTable( stackFrames );
}

void CodeAttributeStackMapTable::swap( CodeAttributeStackMapTable &left, CodeAttributeStackMapTable &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );

  std::swap( left.m_StackFrames, right.m_StackFrames );
}

CodeAttributeStackMapTable &CodeAttributeStackMapTable::operator=( CodeAttributeStackMapTable other )
{
  swap( *this, other );

  return *this;
}

DataBuffer CodeAttributeStackMapTable::ToBinary( const ConstantPool & ) const
{
  DataBuffer result = DataBuffer::EmptyBuffer();
  result = result.AppendUint16( static_cast<uint16_t>(m_StackFrames.size()) );

  for (std::shared_ptr<StackFrame> pFrame : m_StackFrames)
  {
    result = result.Append( pFrame->ToBinary() );
  }

  return result;
}