
#include "IndexOutOfBoundsException.h"
#include "CodeAttributeAnnotationDefault.h"

CodeAttributeAnnotationDefault::CodeAttributeAnnotationDefault( const JavaString &name, AnnotationsEntry entry )
  : JavaCodeAttribute( name, e_JavaAttributeTypeAnnotationDefault )
  , m_Entry( std::move( entry ) )
{}

CodeAttributeAnnotationDefault::CodeAttributeAnnotationDefault( const CodeAttributeAnnotationDefault &other )
  : JavaCodeAttribute( other )
  , m_Entry( other.m_Entry )
{}

CodeAttributeAnnotationDefault::CodeAttributeAnnotationDefault( CodeAttributeAnnotationDefault &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeAnnotationDefault )
  , m_Entry( AnnotationsEntry::BlankEntry() )
{
  swap( *this, other );
}

bool CodeAttributeAnnotationDefault::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeAnnotationDefault &>(other) );
}

bool CodeAttributeAnnotationDefault::Equals( const CodeAttributeAnnotationDefault &other ) const JVMX_NOEXCEPT
{
  return m_Entry == other.m_Entry;
}

CodeAttributeAnnotationDefault::~CodeAttributeAnnotationDefault() JVMX_NOEXCEPT
{
}

CodeAttributeAnnotationDefault CodeAttributeAnnotationDefault::FromBinary( JavaString name, Stream &byteStream )
{
  return CodeAttributeAnnotationDefault( name, *AnnotationsEntry::FromBinary( byteStream ) );
}

DataBuffer CodeAttributeAnnotationDefault::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.Append( m_Entry.ToBinary() );

  return buffer;
}

void CodeAttributeAnnotationDefault::swap( CodeAttributeAnnotationDefault &left, CodeAttributeAnnotationDefault &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_Entry, right.m_Entry );
}

uint16_t CodeAttributeAnnotationDefault::GetTypeIndex() const JVMX_NOEXCEPT
{
  return m_Entry.GetTypeIndex();
}

uint16_t CodeAttributeAnnotationDefault::GetNumberOfElementValuePairs() const JVMX_NOEXCEPT
{
  return m_Entry.GetNumberOfElementValuePairs();
}

const AnnotationsElementValuePair &CodeAttributeAnnotationDefault::GetElementValuePairAt( size_t index ) const
{
  return m_Entry.GetElementValuePairAt( index );
}
