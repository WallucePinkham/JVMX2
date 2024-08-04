
#include "InterfaceInfo.h"

#include "JavaString.h"


InterfaceInfo::InterfaceInfo( boost::intrusive_ptr<JavaString> name )
  : m_Name( name )
{}


InterfaceInfo::InterfaceInfo( const InterfaceInfo &other )
  : m_Name( other.m_Name )
{
}

InterfaceInfo::InterfaceInfo( InterfaceInfo &&other )
  : m_Name( std::move( other.m_Name ) )
{
}

InterfaceInfo InterfaceInfo::operator=(InterfaceInfo other) JVMX_NOEXCEPT
{
  swap( *this, other );
  return *this;
}


void InterfaceInfo::swap( InterfaceInfo &left, InterfaceInfo &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Name, right.m_Name );
}

const JavaString &InterfaceInfo::GetName() const
{
  return *m_Name;
}

