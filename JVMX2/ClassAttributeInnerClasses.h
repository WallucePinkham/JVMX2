
#ifndef __CLASSATTRIBUTEINNERCLASSES_H__
#define __CLASSATTRIBUTEINNERCLASSES_H__

#include "ExceptionTableEntry.h"
#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class ClassAttributeInnerClasses : public JavaCodeAttribute
{
public:
  struct InnerClassListEntry
  {
    ConstantPoolIndex m_InnerClassInfoIndex;
    ConstantPoolIndex m_OuterClassInfoIndex;
    ConstantPoolIndex m_InnerNameIndex;
    uint16_t m_InnerClassAccessFlags;
  };

  typedef std::vector<InnerClassListEntry> InnerClassList;

private:
  ClassAttributeInnerClasses( const JavaString &name, InnerClassList list );

public:
  ClassAttributeInnerClasses( const ClassAttributeInnerClasses &other );
  ClassAttributeInnerClasses( ClassAttributeInnerClasses &&other );

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_OVERRIDE;
  bool Equals( const ClassAttributeInnerClasses &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfInnerClasses() const;
  virtual InnerClassListEntry GetInnerClassAt( size_t index ) const;

  virtual ~ClassAttributeInnerClasses() JVMX_NOEXCEPT;

  static ClassAttributeInnerClasses FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( ClassAttributeInnerClasses &left, ClassAttributeInnerClasses &right ) JVMX_NOEXCEPT;

protected:
  InnerClassList m_ClassList;
};

#endif // __CLASSATTRIBUTEINNERCLASSES_H__

