
#if defined(_WIN32) || defined (_WIN64)
#include "OperatingSystemWindows.h"
#else
#error "You need to create a operating system delegate for your OS."
#endif

#include "OsFunctions.h"
#include "OsFunctionsSingletonFactory.h"

IOperatingSystemDelegate *OsFunctionsSingletonFactory::m_pDelegate = nullptr;

OsFunctionsSingletonFactory::OsFunctionsSingletonFactory()
{}

OsFunctionsSingletonFactory::~OsFunctionsSingletonFactory()
{
  delete m_pDelegate;
}

IOperatingSystemDelegate *OsFunctionsSingletonFactory::GetOsDelegate()
{
  if ( nullptr == m_pDelegate )
  {
#if defined(_WIN32) || defined (_WIN64)
    m_pDelegate = new OperatingSystemWindows;
#else
    return nullptr;
#endif // defined(_WIN32) || defined (_WIN64)
  }

  return m_pDelegate;
}
