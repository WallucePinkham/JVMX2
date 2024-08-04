/*
#include "JavaNullReference.h"

#include "JavaString.h"

e_JavaVariableTypes JavaNullReference::GetVariableType() const
{
  return e_JavaVariableTypes::NullReference;
}

bool JavaNullReference::IsReferenceType() const
{
  return true;
}

bool JavaNullReference::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaNullReference::ToString() const
{
  return JavaString::FromCString( "{null}" );
}

bool JavaNullReference::IsNull() const
{
  return true;
}

bool JavaNullReference::operator==( const JavaNullReference & )
{
  return true;
}

bool JavaNullReference::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return true;
}

bool JavaNullReference::operator<( const IJavaVariableType & ) const
{
  return false;
}

bool JavaNullReference::operator<( const JavaNullReference & ) const
{
  return false;
}*/