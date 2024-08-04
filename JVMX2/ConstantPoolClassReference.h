
#ifndef __CONSTANTPOOLCLASSREFERENCE_H__
#define __CONSTANTPOOLCLASSREFERENCE_H__

#include <memory>
#include <boost/intrusive_ptr.hpp>

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"
#include "JavaString.h"

class ConstantPoolClassReference : public IConstantPoolEntryValue
{
private:
  ConstantPoolClassReference( ConstantPoolIndex index );

public:
  ConstantPoolClassReference( const ConstantPoolClassReference &other );
  ConstantPoolClassReference& operator=(const ConstantPoolClassReference &other);

  virtual ~ConstantPoolClassReference();

  virtual boost::intrusive_ptr<JavaString> GetClassName() const;

  static std::shared_ptr<ConstantPoolClassReference> FromConstantPoolIndex( ConstantPoolIndex index );

  virtual ConstantPoolIndex ToConstantPoolIndex() const;
  virtual bool operator==(const ConstantPoolClassReference &other) const;

  virtual void Prepare( const ConstantPool *pPool ) JVMX_OVERRIDE;
  virtual bool IsPrepared() const;

private:
  ConstantPoolIndex m_Value;

  bool m_Prepared;

  boost::intrusive_ptr<JavaString> m_pName;
};

#endif // __CONSTANTPOOLCLASSREFERENCE_H__
