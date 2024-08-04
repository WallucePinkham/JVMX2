#include <memory>

#include "IndexOutOfBoundsException.h"
#include "InvalidArgumentException.h"

#include "JavaString.h"

#include "ConstantPool.h"

static const size_t c_FirstValidIndex = 1;

ConstantPool::ConstantPool()
{
  // This ensures that index 0 is always invalid.
  m_Entries.push_back( std::make_shared<ConstantPoolEntry>( ConstantPoolNullEntry() ) );
}

ConstantPool::ConstantPool( const ConstantPool &other ) : m_Entries( other.m_Entries )
{}

ConstantPool::ConstantPool( ConstantPool &&other ) : m_Entries( std::move( other.m_Entries ) )
{}

ConstantPool::~ConstantPool()
{}

size_t ConstantPool::GetCount() const
{
  return m_Entries.size();
}

void ConstantPool::AddConstant( std::shared_ptr<ConstantPoolEntry> constant )
{
  m_Entries.push_back( constant );

  // We need to keep the constant pool indexes matching to what's in the class file.
  if ( e_ConstantPoolEntryTypeLong == constant->GetType() || e_ConstantPoolEntryTypeDouble == constant->GetType() )
  {
    m_Entries.push_back( std::make_shared<ConstantPoolEntry>( ConstantPoolNullEntry() ) );
  }
}

std::shared_ptr<ConstantPoolEntry> ConstantPool::GetConstant( size_t index ) const
{
  if ( 0 == index || index > m_Entries.size() )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index was invalid. Use GetCount() to find the number of items, and remember that indexes start at 1." );
  }

  std::shared_ptr<ConstantPoolEntry> pEntry( m_Entries.at( index ) );

  if ( pEntry->GetType() == e_ConstantPoolEntryTypeNullEntry )
  {
    throw InvalidArgumentException( __FUNCTION__ " - The constant pool entry at this index is not a real value. This is likely due to a long or a double value that uses up two index values." );
  }

  return pEntry;
}

size_t ConstantPool::GetFirstValidIndex()
{
  return c_FirstValidIndex;
}

size_t ConstantPool::GetLastValidIndex() const
{
  return GetCount() + c_FirstValidIndex;
}

ConstantPoolIndex ConstantPool::GetStringConstantIndex( const JavaString &stringConstant ) const
{
  ConstantPoolIndex index = 0;
  for ( auto it = m_Entries.cbegin(); it != m_Entries.cend(); ++ it )
  {
    std::shared_ptr<ConstantPoolEntry> pEntry = *it;
    if ( e_ConstantPoolEntryTypeString == pEntry->GetType() && *pEntry->AsString() == stringConstant )
    {
      return index;
    }

    ++ index;
  }

  return 0;
}

void ConstantPool::Prepare()
{
  for ( auto pEntry : m_Entries )
  {
    pEntry->Prepare( this );
  }

  // #ifdef _DEBUG
  //   DebugDump();
  // #endif // _DEBUG
}

// #ifdef _DEBUG
// void ConstantPool::DebugDump()
// {
//   ConstantPoolIndex index = 0;
//   for ( auto it = m_Entries.cbegin(); it != m_Entries.cend(); ++ it )
//   {
//     std::shared_ptr<ConstantPoolEntry> pEntry = *it;
//     pEntry->DebugDump();
//
//     ++ index;
//   }
// #endif // _DEBUG
// }