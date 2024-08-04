
#include "DefaultJavaLangClassList.h"
#include "ObjectReference.h"

extern const JavaString c_SyntheticField_ClassName;

void DefaultJavaLangClassList::Add( const JavaString &typeName, boost::intrusive_ptr<ObjectReference> entry )
{
  JVMX_ASSERT( !entry->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName )->IsNull() );

  m_Entries.push_back( { typeName, entry } );
}

boost::intrusive_ptr<ObjectReference> DefaultJavaLangClassList::Find( const JavaString &typeName ) const
{
  for ( size_t i = 0; i < m_Entries.size(); ++ i )
  {
    if ( m_Entries.at( i ) .first == typeName )
    {
      return m_Entries.at( i ).second;
    }
  }

  return nullptr;
}

size_t DefaultJavaLangClassList::GetCount() const
{
  return m_Entries.size();
}

boost::intrusive_ptr<ObjectReference> DefaultJavaLangClassList::GetByIndex( size_t index )
{
  return m_Entries.at( index ).second;
}
