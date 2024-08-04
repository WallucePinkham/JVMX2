
#include "JavaString.h"

#include "InvalidArgumentException.h"
#include "InvalidStateException.h"

#include "JavaTypes.h"
#include "ObjectReference.h"

#include "IJavaVariableType.h"

/*JavaString IJavaVariableType::ToString() const
{
return JavaString::FromCString( "Invalid" );
}*/

bool IJavaVariableType::operator<( const IJavaVariableType & ) const
{
  return false;
}

bool IJavaVariableType::operator==( const IJavaVariableType & ) const
{
  return false;
}

bool IJavaVariableType::operator!=( const IJavaVariableType &other ) const
{
  return !( *this == other );
}

IJavaVariableType &IJavaVariableType::operator=( const IJavaVariableType &other )
{
  if ( !IsReferenceType() && GetVariableType() != other.GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Invalid argument passed. Types do not match." );
  }

  switch ( GetVariableType() )
  {
    case e_JavaVariableTypes::Char:
      dynamic_cast<JavaChar *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Byte:
      dynamic_cast<JavaByte *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Short:
      dynamic_cast<JavaShort *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Integer:
      dynamic_cast<JavaInteger *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Long:
      dynamic_cast<JavaLong *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Float:
      dynamic_cast<JavaFloat *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Double:
      dynamic_cast<JavaDouble *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::String:
      dynamic_cast<JavaString *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::NullReference:
      reinterpret_cast<ObjectReference *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::ReturnAddress:
      //dynamic_cast<JavaReturnAddress *>(this)->operator=( other );
      JVMX_ASSERT( false );
      break;
    case e_JavaVariableTypes::ClassReference:
      JVMX_ASSERT( false );
      //dynamic_cast<JavaClassReference *>(this)->operator=( other );
      break;
    case e_JavaVariableTypes::Bool:
      dynamic_cast<JavaBool *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Array:
      reinterpret_cast<ObjectReference *>( this )->operator=( other );
      break;
    case e_JavaVariableTypes::Object:
      reinterpret_cast<ObjectReference *>( this )->operator=( other );
      break;
    default:
      throw InvalidStateException( __FUNCTION__ " - Invalid field type." );
      break;
  }

  return *this;
}

IJavaVariableType::~IJavaVariableType()
{}
