
#ifndef _JAVACLASSREFERENCE__H_
#define _JAVACLASSREFERENCE__H_

#include <memory>

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaClass; // Forward Declaration

class JavaClassReference : public IJavaVariableType
{
public:
  explicit JavaClassReference( std::shared_ptr<JavaClass> pClassFile );

  JavaClassReference( const JavaClassReference &other );

  virtual ~JavaClassReference() JVMX_NOEXCEPT{};

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaClassReference &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaClassReference &other) const;

  std::shared_ptr<JavaClass> GetClassFile() const;

private:
  void DebugAssert() const;

private:
  std::shared_ptr<JavaClass> m_pClassFile;
};

#endif // _JAVACLASSREFERENCE__H_
