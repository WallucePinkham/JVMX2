#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "AnnotationsEntry.h"

AnnotationsEntry::AnnotationsEntry( const AnnotationsEntry &other )
  : m_TypeIndex( other.m_TypeIndex )
  , m_ElementValuePairs( other.m_ElementValuePairs )
{
}

AnnotationsEntry::AnnotationsEntry( AnnotationsEntry &&other )
{
  swap( *this, other );
}

AnnotationsEntry::AnnotationsEntry( ConstantPoolIndex typeIndex )
  : m_TypeIndex( typeIndex )
{}

AnnotationsEntry::~AnnotationsEntry() JVMX_NOEXCEPT
{
}

AnnotationsEntry &AnnotationsEntry::operator=(AnnotationsEntry other)
{
  swap( *this, other );
  return *this;
}

bool AnnotationsEntry::operator==(const AnnotationsEntry &other) const JVMX_NOEXCEPT
{
  return m_TypeIndex == other.m_TypeIndex &&
  std::equal( m_ElementValuePairs.begin(), m_ElementValuePairs.end(), other.m_ElementValuePairs.begin() );
}

uint16_t AnnotationsEntry::GetTypeIndex() const JVMX_NOEXCEPT
{
  return m_TypeIndex;
}

uint16_t AnnotationsEntry::GetNumberOfElementValuePairs() const JVMX_NOEXCEPT
{
  return static_cast<uint16_t>(m_ElementValuePairs.size());
}

const AnnotationsElementValuePair &AnnotationsEntry::GetElementValuePairAt( size_t index ) const
{
  try
  {
    return m_ElementValuePairs.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}

std::shared_ptr<AnnotationsEntry> AnnotationsEntry::FromBinary( Stream &stream )
{
  ConstantPoolIndex typeIndex = static_cast<ConstantPoolIndex>(stream.ReadUint16());
  uint16_t numberOfPairs = stream.ReadUint16();

  // Can't use make_shared here because of the private constructor
  std::shared_ptr<AnnotationsEntry> pEntry = std::shared_ptr<AnnotationsEntry>( new AnnotationsEntry( typeIndex ) );
  for ( uint16_t i = 0; i < numberOfPairs; ++ i )
  {
    uint16_t elementNameIndex = static_cast<ConstantPoolIndex>(stream.ReadUint16());
    std::shared_ptr<AnnotationsElementValue> pNewValue = AnnotationsElementValue::FromBinary( stream );
    pEntry->m_ElementValuePairs.push_back( AnnotationsElementValuePair( elementNameIndex, pNewValue ) );
  }

  return pEntry;
}

DataBuffer AnnotationsEntry::ToBinary() const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();
  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_TypeIndex) );
  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_ElementValuePairs.size()) );

  for (AnnotationsElementValuePair pair : m_ElementValuePairs)
  {
    buffer = buffer.AppendUint16( pair.m_ElementNameIndex );
    buffer = buffer.Append( pair.m_pElementValue->ToBinary() );
  }

  return buffer;
}

void AnnotationsEntry::swap( AnnotationsEntry &left, AnnotationsEntry &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_TypeIndex, right.m_TypeIndex );
  std::swap( left.m_ElementValuePairs, right.m_ElementValuePairs );
}

AnnotationsEntry AnnotationsEntry::BlankEntry()
{
  return AnnotationsEntry( static_cast<ConstantPoolIndex>(0) );
}

