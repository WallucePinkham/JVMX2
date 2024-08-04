
#include "ConstantPoolNullEntry.h"

ConstantPoolNullEntry::ConstantPoolNullEntry( const ConstantPoolNullEntry & )
{}

ConstantPoolNullEntry& ConstantPoolNullEntry::operator=(const ConstantPoolNullEntry &)
{
  return *this;
}


bool ConstantPoolNullEntry::operator==(const ConstantPoolNullEntry &) const
{
  return false;
}

