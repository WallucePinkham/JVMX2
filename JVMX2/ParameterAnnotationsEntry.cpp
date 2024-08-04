#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "ParameterAnnotationsEntry.h"

ParameterAnnotationsEntry::ParameterAnnotationsEntry( const ParameterAnnotationsEntry &other )
  : m_Annotations( other.m_Annotations )
{}

ParameterAnnotationsEntry::ParameterAnnotationsEntry( ParameterAnnotationsEntry &&other )
{
  swap( *this, other );
}

ParameterAnnotationsEntry::ParameterAnnotationsEntry()
{}

ParameterAnnotationsEntry::~ParameterAnnotationsEntry() JVMX_NOEXCEPT
{
}

ParameterAnnotationsEntry &ParameterAnnotationsEntry::operator=(ParameterAnnotationsEntry other)
{
  swap( *this, other );
  return *this;
}

bool ParameterAnnotationsEntry::operator==(const ParameterAnnotationsEntry &other) const JVMX_NOEXCEPT
{
  return std::equal( m_Annotations.begin(), m_Annotations.end(), other.m_Annotations.begin() );
}

uint16_t ParameterAnnotationsEntry::GetNumberOfAnnotations() const JVMX_NOEXCEPT
{
  return static_cast<uint16_t>(m_Annotations.size());
}

const std::shared_ptr<AnnotationsEntry> ParameterAnnotationsEntry::GetAnnotationAt( size_t index ) const
{
  try
  {
    return m_Annotations.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}

std::shared_ptr<ParameterAnnotationsEntry> ParameterAnnotationsEntry::FromBinary( Stream &stream )
{
  uint16_t numberOfAnnotations = stream.ReadUint16();

  // Can't use make_shared here because of the private constructor
  std::shared_ptr<ParameterAnnotationsEntry> pEntry = std::shared_ptr<ParameterAnnotationsEntry>( new ParameterAnnotationsEntry() );
  for ( uint16_t i = 0; i < numberOfAnnotations; ++ i )
  {
    std::shared_ptr<AnnotationsEntry> pNewValue = AnnotationsEntry::FromBinary( stream );
    pEntry->m_Annotations.push_back( pNewValue );
  }

  return pEntry;
}

DataBuffer ParameterAnnotationsEntry::ToBinary() const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();
  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_Annotations.size()) );

  for (auto pEntry : m_Annotations)
  {
    buffer = buffer.Append( pEntry->ToBinary() );
  }

  return buffer;
}

void ParameterAnnotationsEntry::swap( ParameterAnnotationsEntry &left, ParameterAnnotationsEntry &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Annotations, right.m_Annotations );
}


