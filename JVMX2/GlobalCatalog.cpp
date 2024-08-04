#include "InvalidArgumentException.h"

#include "GlobalCatalog.h"
#include "InvalidStateException.h"

std::shared_ptr<GlobalCatalog> GlobalCatalog::s_pInstance = nullptr;
std::once_flag GlobalCatalog::s_OnceFlag;

void GlobalCatalog::Reset()
{
  //s_pInstance = new GlobalCatalog();
  GlobalCatalog::s_pInstance.reset( new GlobalCatalog() );
}

GlobalCatalog &GlobalCatalog::GetInstance()
{
  std::call_once( GlobalCatalog::s_OnceFlag, []() { GlobalCatalog::s_pInstance.reset( new GlobalCatalog() ); } );

  return *GlobalCatalog::s_pInstance;
}

void GlobalCatalog::Add( const std::string& id, const cxx0x::shared_ptr< wallaroo::Part >& dev )
{
  m_Catalog.Add( id, dev );
}

wallaroo::detail::PartShell GlobalCatalog::Get( const std::string& id ) const
{
  try
  {
    return m_Catalog[ id ];
  }
  catch ( wallaroo::ElementNotFound &ex )
  {
    throw InvalidArgumentException( ex.what() );
  }
}

void GlobalCatalog::CheckWiring() const
{
  try
  {
    m_Catalog.CheckWiring();
  }
  catch ( wallaroo::WiringError &ex )
  {
    throw InvalidStateException( ex.what() );
  }
}

wallaroo::Catalog &GlobalCatalog::GetCatalog()
{
  return m_Catalog;
}

GlobalCatalog::GlobalCatalog( const GlobalCatalog &other )
{
  s_pInstance = other.s_pInstance;
}

GlobalCatalog::GlobalCatalog()
{}

GlobalCatalog &GlobalCatalog::operator=( const GlobalCatalog &other )
{
  if ( this != &other )
  {
    s_pInstance = other.s_pInstance;
  }

  return *this;
}