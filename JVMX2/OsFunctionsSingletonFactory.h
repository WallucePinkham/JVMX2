
#ifndef __OSFUNCTIONSSINGLETONFACTORY_H__
#define __OSFUNCTIONSSINGLETONFACTORY_H__

class OsFunctions; // Forward Declaration
class IOperatingSystemDelegate; // Forward Declaration

class OsFunctionsSingletonFactory
{
public:
  OsFunctionsSingletonFactory();
  virtual ~OsFunctionsSingletonFactory();

  static IOperatingSystemDelegate *GetOsDelegate();

protected:
  static IOperatingSystemDelegate *m_pDelegate;
};

#endif // __OSFUNCTIONSSINGLETONFACTORY_H__
