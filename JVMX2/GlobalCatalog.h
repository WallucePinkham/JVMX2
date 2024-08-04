
#ifndef _GLOBALCATALOG__H_
#define _GLOBALCATALOG__H_

#include <mutex>
#include <wallaroo/catalog.h>

class GlobalCatalog
{
public:
  static GlobalCatalog &GetInstance();

  void Add( const std::string& id, const cxx0x::shared_ptr< wallaroo::Part >& dev );
  
  wallaroo::detail::PartShell Get( const std::string& id ) const;
  void CheckWiring() const;
  wallaroo::Catalog &GetCatalog();

  static void Reset();

private:
  static std::shared_ptr<GlobalCatalog> s_pInstance;
  static std::once_flag s_OnceFlag;

  GlobalCatalog();
  GlobalCatalog( const GlobalCatalog &other );
  GlobalCatalog &operator=( const GlobalCatalog &other );

private:
  wallaroo::Catalog m_Catalog;
};

#endif // _GLOBALCATALOG__H_
