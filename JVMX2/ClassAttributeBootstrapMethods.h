#ifndef _CLASSATTRIBUTEBOOTSTRAPMETHODS__H_
#define _CLASSATTRIBUTEBOOTSTRAPMETHODS__H_

#include "ExceptionTableEntry.h"
#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream;                    // Forward declaration

class ClassAttributeBootstrapMethods : public JavaCodeAttribute
{
public:
  typedef std::vector<uint16_t> BootstrapArgumentList;

  struct BootstrapMethodsListEntry
  {
    ConstantPoolIndex m_BootstrapMethodIndex;
    ConstantPoolIndex m_NumberOfBootstrapArguments;
    BootstrapArgumentList m_BootstrapArguments;
  };

  typedef std::vector<BootstrapMethodsListEntry> BootstrapMethodList;

private:
  ClassAttributeBootstrapMethods( const JavaString &name, BootstrapMethodList list );

public:
  ClassAttributeBootstrapMethods( const ClassAttributeBootstrapMethods &other );
  ClassAttributeBootstrapMethods( ClassAttributeBootstrapMethods &&other );

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_OVERRIDE;
  bool Equals( const ClassAttributeBootstrapMethods &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOBootstrapMethods() const;
  virtual BootstrapMethodsListEntry GetBootstrapMethodAt( size_t index ) const;

  virtual ~ClassAttributeBootstrapMethods() JVMX_NOEXCEPT;

  static ClassAttributeBootstrapMethods FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( ClassAttributeBootstrapMethods &left,
    ClassAttributeBootstrapMethods &right ) JVMX_NOEXCEPT;

protected:
  BootstrapMethodList m_MethodList;
};

#endif // _CLASSATTRIBUTEBOOTSTRAPMETHODS__H_
