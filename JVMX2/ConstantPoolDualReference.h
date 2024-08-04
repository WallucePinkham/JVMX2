
#ifndef __CONSTANTPOOLDUALREFERENCE_H__
#define __CONSTANTPOOLDUALREFERENCE_H__

#include <memory>

#include "GlobalConstants.h"
#include "ConstantPoolClassReference.h"
#include "ConstantPoolStringReference.h"

#include "IConstantPoolEntryValue.h"

class ConstantPool; // Forward declaration

class ConstantPoolDualReference : public IConstantPoolEntryValue
{
public:
  ConstantPoolDualReference( std::shared_ptr<ConstantPoolClassReference> pClassRef, ConstantPoolIndex nameAndTypeRefIndex );
  ConstantPoolDualReference( const ConstantPoolDualReference &other );

  virtual ~ConstantPoolDualReference() JVMX_NOEXCEPT;

  virtual bool operator==(const ConstantPoolDualReference &other) const JVMX_NOEXCEPT;

  virtual std::shared_ptr<ConstantPoolClassReference> GetClassReference() const JVMX_NOEXCEPT;
  virtual ConstantPoolIndex GetNameAndTypeReferenceIndex() const JVMX_NOEXCEPT;

  virtual void Prepare( const ConstantPool *pPool ) JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<JavaString> GetClassName() const;
  virtual boost::intrusive_ptr<JavaString> GetName() const;
  virtual boost::intrusive_ptr<JavaString> GetType() const;

private:
  virtual const ConstantPoolDualReference &operator=(const ConstantPoolDualReference & other);

protected:
  std::shared_ptr<ConstantPoolClassReference> m_pClassRef;
  ConstantPoolIndex m_NameAndTypeRefIndex;

  boost::intrusive_ptr<JavaString> m_ClassName;
  boost::intrusive_ptr<JavaString> m_Name;
  boost::intrusive_ptr<JavaString> m_Type;

  bool m_Prepared;
};

#endif // __CONSTANTPOOLDUALREFERENCE_H__
