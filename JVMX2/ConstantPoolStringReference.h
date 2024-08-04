
#ifndef __CONSTANTPOOLSTRINGREFERENCE_H__
#define __CONSTANTPOOLSTRINGREFERENCE_H__

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

class JavaObject;
class JavaClass;

class ConstantPoolStringReference : public IConstantPoolEntryValue
{
private:
  ConstantPoolStringReference( ConstantPoolIndex index );

public:
  ConstantPoolStringReference( const ConstantPoolStringReference &other );
  ConstantPoolStringReference& operator=(const ConstantPoolStringReference &other);

  virtual ~ConstantPoolStringReference();

  virtual bool operator==(const ConstantPoolStringReference &other) const;

  static std::shared_ptr<ConstantPoolStringReference> FromConstantPoolIndex( ConstantPoolIndex index );

  virtual ConstantPoolIndex ToConstantPoolIndex() const;

  virtual void Prepare( const ConstantPool *pPool );

  virtual boost::intrusive_ptr<JavaString> GetStringValue( ) const;

private:
  ConstantPoolIndex m_Value;

  boost::intrusive_ptr<JavaString> m_pString;
};

#endif // __CONSTANTPOOLSTRINGREFERENCE_H__
