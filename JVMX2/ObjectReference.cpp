
#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "ObjectReference.h"
#include "GlobalCatalog.h"

const intptr_t c_NullIndex = 0;

//#define JVMX_DEBUG_OBJECTREF 1

ObjectReference::ObjectReference( ObjectIndexT index )
  : IJavaVariableType()
  , m_Index( index )
{
#ifdef _DEBUG
  m_pDebugPointer = nullptr;
#endif // _DEBUG

#ifdef _DEBUG
 AssertValid();
#endif // _DEBUG

#ifdef _DEBUG
  if (index == 1339 )
  {
    int x = 0;
  }
#endif // _DEBUG
}

ObjectReference::ObjectReference( const ObjectReference &other )
  : IJavaVariableType( other )
  , m_Index( other.m_Index )
{
#ifdef _DEBUG
  m_pDebugPointer = other.m_pDebugPointer;
#endif // _DEBUG

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

}

ObjectReference::ObjectReference( const jobject other )
  : IJavaVariableType()
  , m_Index( reinterpret_cast<intptr_t>(other) )
{
#ifdef _DEBUG
  m_pDebugPointer = nullptr;
#endif // _DEBUG

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

}


const ObjectReference& ObjectReference::operator=( const ObjectReference &other )
{
  if ( &other != this )
  {
  //  IJavaVariableType::operator=( other );
    m_Index = other.m_Index;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  return *this;
}


ObjectIndexT ObjectReference::GetIndex() const
{
  return m_Index;
}

bool ObjectReference::operator!=( const ObjectReference &other ) const
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  if ( c_NullIndex == m_Index )
  {
    return c_NullIndex != other.m_Index;
  }

  if ( c_NullIndex == other.m_Index )
  {
    return true;
  }

  if ( m_Index == other.m_Index )
  {
    return false;
  }

  return (*(InternalGetObject( m_Index ))) != (*(InternalGetObject( other.m_Index )));
}

bool ObjectReference::operator!=( const IJavaVariableType &other ) const
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  if ( c_NullIndex == m_Index )
  {
    return !other.IsNull();
  }

  return (*(InternalGetObject( m_Index ))) != other;
}

bool ObjectReference::operator==( const ObjectReference &other ) const
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  if ( c_NullIndex == m_Index )
  {
    return c_NullIndex == other.m_Index;
  }

  if ( c_NullIndex == other.m_Index )
  {
    return c_NullIndex == m_Index;
  }

  if ( m_Index == other.m_Index )
  {
    return true;
  }

  return (*(InternalGetObject( m_Index ))) == (*(InternalGetObject( other.m_Index )));
}

bool ObjectReference::operator==( const IJavaVariableType & other) const
{
  if ( c_NullIndex == m_Index )
  {
    return other.IsNull();
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  return (*(InternalGetObject( m_Index ))) == other;
}

bool ObjectReference::operator<( const ObjectReference & other ) const
{

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  if ( c_NullIndex == m_Index )
  {
    return false;
  }

  if ( c_NullIndex == other.m_Index )
  {
    return false;
  }

  if ( m_Index == other.m_Index )
  {
    return false;
  }

  return (*(InternalGetObject( m_Index ))) < (*(InternalGetObject( other.m_Index )));
}

bool ObjectReference::operator<( const IJavaVariableType & other) const
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  if ( c_NullIndex == m_Index )
  {
    return false;
  }

  return (*(InternalGetObject( m_Index ))) < other;
}

JavaObject *ObjectReference::GetObject() const
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  if ( c_NullIndex == m_Index )
  {
    throw InvalidStateException( __FUNCTION__ " - Trying to de-reference null object." );
  }

  IJavaVariableType *pVariable = InternalGetObject( m_Index );

#ifdef _DEBUG
  m_pDebugPointer = pVariable;
#endif // _DEBUG

  if ( e_JavaVariableTypes::Object != pVariable->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected variable to be an object. " );
  }

  return reinterpret_cast<JavaObject *>(pVariable);
}

JavaArray *ObjectReference::GetArray() const
{

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  if ( c_NullIndex == m_Index )
  {
    throw InvalidStateException( __FUNCTION__ " - Trying to de-reference null object." );
  }

  IJavaVariableType *pVariable = InternalGetObject( m_Index );

#ifdef _DEBUG
  m_pDebugPointer = pVariable;
#endif // _DEBUG

  if ( e_JavaVariableTypes::Array != pVariable->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected variable to be an array. " );
  }

  return reinterpret_cast<JavaArray *>(pVariable);
}

void ObjectReference::CloneOther( const ObjectReference &other )
{
#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  if ( c_NullIndex == m_Index )
  {
    throw InvalidStateException( __FUNCTION__ " - This is a null reference." );
  }

  if ( c_NullIndex == other.m_Index )
  {
    throw InvalidStateException( __FUNCTION__ " - Trying to clone null object." );
  }
  
  if ( e_JavaVariableTypes::Object != GetVariableType() && e_JavaVariableTypes::Array != GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected this to be an object or array." );
  }

  if ( e_JavaVariableTypes::Object != other.GetVariableType() && e_JavaVariableTypes::Array != other.GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected parameter to be an object or array." );
  }

  if ( GetVariableType() != other.GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected both types to be the same." );
  }

  if ( e_JavaVariableTypes::Object == GetVariableType() )
  {
    GetContainedObject()->CloneOther( other.GetContainedObject() );
  }
  else
  {
    GetContainedArray()->CloneOther( other.GetContainedArray() );
  }
}

ObjectReference &ObjectReference::operator=( const IJavaVariableType &other )
{
  if ( other.IsNull() )
  {
    m_Index = c_NullIndex;
#ifdef _DEBUG
    m_pDebugPointer = nullptr;
#endif // _DEBUG
  }
  else if ( e_JavaVariableTypes::Object != other.GetVariableType() && e_JavaVariableTypes::Array != other.GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected reference type." );
  }
  else
  {
    m_Index = dynamic_cast<const ObjectReference *>(&other)->m_Index;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  return *this;
}

jobject ObjectReference::ToJObject()
{
  if ( c_NullIndex == m_Index )
  {
    return nullptr;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG


  return reinterpret_cast<jobject>(m_Index);
}

#ifdef _DEBUG
void ObjectReference::AssertValid() const
{
#if defined( _DEBUG ) && defined (JVMX_DEBUG_OBJECTREF)
  ObjectRegistryLocalMachine::GetInstance().VerifyEntry( m_Index );
#endif
}
#endif // _DEBUG

e_JavaVariableTypes ObjectReference::GetVariableType() const
{
  if ( c_NullIndex == m_Index )
  {
    return e_JavaVariableTypes::NullReference;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  IJavaVariableType *pVariable = InternalGetObject( m_Index );

#ifdef _DEBUG
  m_pDebugPointer = pVariable;
#endif // _DEBUG

  return pVariable->GetVariableType();
}

bool ObjectReference::IsReferenceType() const
{
  if ( c_NullIndex == m_Index )
  {
    return true;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  return InternalGetObject( m_Index )->IsReferenceType();
}

bool ObjectReference::IsIntegerCompatible() const
{
  if ( c_NullIndex == m_Index )
  {
    return false;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  return InternalGetObject( m_Index )->IsIntegerCompatible();
}

bool ObjectReference::IsNull() const
{
  if ( c_NullIndex == m_Index )
  {
    return true;
  }

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  return InternalGetObject( m_Index )->IsNull();
}

JavaString ObjectReference::ToString() const
{
  if ( c_NullIndex == m_Index )
  {
    return JavaString::FromCString( "{null}" );
  }

#ifdef _DEBUG
  AssertValid();
  char pBuffer[ 31 ];
  _i64toa_s( m_Index, pBuffer, 30, 10 );

  JavaString refString = JavaString::FromCString( pBuffer );

  return refString.Append( u" : " ).Append( InternalGetObject( m_Index )->ToString() );
#else
  return InternalGetObject( m_Index )->ToString();
#endif // _DEBUG
}

IJavaVariableType *ObjectReference::InternalGetObject( ObjectIndexT ref ) const
{
  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  return pObjectRegistry->GetObject_( ref );
}
