
#ifndef _CONSTANTPOOLNULLENTRY__H_
#define _CONSTANTPOOLNULLENTRY__H_

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

// This is placeholder object for constant pool entries that take up two indices in the constant pool.
class ConstantPoolNullEntry : public IConstantPoolEntryValue
{
public:
  ConstantPoolNullEntry() {};

  ConstantPoolNullEntry( const ConstantPoolNullEntry & );
  ConstantPoolNullEntry& operator=(const ConstantPoolNullEntry &);

  virtual bool operator==(const ConstantPoolNullEntry &) const;
};

#endif // _CONSTANTPOOLNULLENTRY__H_
