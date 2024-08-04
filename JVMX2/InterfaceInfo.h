
#ifndef _INTERFACEINFO__H_
#define _INTERFACEINFO__H_

#include "GlobalConstants.h"
#include "ConstantPoolMethodReference.h"
#include "JavaClassConstants.h"
#include "JavaString.h"

class InterfaceInfo
{
public:
  InterfaceInfo( boost::intrusive_ptr<JavaString> name );

  InterfaceInfo( const InterfaceInfo &other );
  InterfaceInfo( InterfaceInfo &&other );

  InterfaceInfo operator=(InterfaceInfo other) JVMX_NOEXCEPT;

  const JavaString &GetName() const;

  static void swap( InterfaceInfo &left, InterfaceInfo &right ) JVMX_NOEXCEPT;

private:
  boost::intrusive_ptr<JavaString> m_Name;
};

typedef std::vector<InterfaceInfo> InterfaceInfoList;

#endif // _INTERFACEINFO__H_
