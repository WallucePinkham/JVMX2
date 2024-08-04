
#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "ClassAttributeSourceFile.h"

ClassAttributeSourceFile::ClassAttributeSourceFile( const ClassAttributeSourceFile &other ) : JavaCodeAttribute( other ), m_SourceFileIndex( other.m_SourceFileIndex )
{}

ClassAttributeSourceFile::ClassAttributeSourceFile( const JavaString &name, ConstantPoolIndex sourceFileIndex ) : JavaCodeAttribute( name, e_JavaAttributeTypeSourceFile ), m_SourceFileIndex( sourceFileIndex )
{
}

ClassAttributeSourceFile::ClassAttributeSourceFile( ClassAttributeSourceFile &&other ) : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeSourceFile )
{
  swap( *this, other );
}

ClassAttributeSourceFile::~ClassAttributeSourceFile()
{}

ClassAttributeSourceFile ClassAttributeSourceFile::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t sourceFileIndex = byteStream.ReadUint16();

  return ClassAttributeSourceFile( name, sourceFileIndex );
}

void ClassAttributeSourceFile::swap( ClassAttributeSourceFile &left, ClassAttributeSourceFile &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_SourceFileIndex, right.m_SourceFileIndex );
}

DataBuffer ClassAttributeSourceFile::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  return buffer.AppendUint16( static_cast<uint16_t>(m_SourceFileIndex) );
}

bool ClassAttributeSourceFile::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const ClassAttributeSourceFile &>(other) );
}

bool ClassAttributeSourceFile::Equals( const ClassAttributeSourceFile &other ) const JVMX_NOEXCEPT
{
  return m_SourceFileIndex == other.GetIndex();
}

ConstantPoolIndex ClassAttributeSourceFile::GetIndex() const JVMX_NOEXCEPT
{
  return m_SourceFileIndex;
}



