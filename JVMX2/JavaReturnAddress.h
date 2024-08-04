
#ifndef _JAVARETURNADDRESS__H_
#define _JAVARETURNADDRESS__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"

class JavaReturnAddress : public IJavaVariableType
{
public:
  JavaReturnAddress( uintptr_t programCounter );
  JavaReturnAddress( const JavaReturnAddress &other );

  virtual ~JavaReturnAddress() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  virtual uintptr_t GetAddress() const JVMX_NOEXCEPT;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaReturnAddress &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaReturnAddress &other) const;

private:
  uintptr_t m_pAddress;
};

#endif // _JAVARETURNADDRESS__H_
