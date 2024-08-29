
#include "MallocFreeMemoryManager.h"

#include "IndexOutOfBoundsException.h"
#include "InvalidArgumentException.h"
#include "UnsupportedTypeException.h"
#include "InvalidStateException.h"

#include "GlobalCatalog.h"
#include "IThreadManager.h"
#include "IVirtualMachineState.h"

#include "ObjectReference.h"

#include "JavaTypes.h"
#include "TypeParser.h"

#include "JavaArray.h"

JavaArray::JavaArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ e_JavaArrayTypes type, size_t size )
  : m_ContainedType( type )
  , m_Size( size )
  , m_pMonitor( new Lockable )
    //, m_pValues( size, TypeParser::GetDefaultValue( type ) )
#ifdef _DEBUG
  , debugInitialLength( size )
#endif // _DEBUG
{
#ifdef _DEBUG
  if ( 0 == size )
  {
    int i = 20;
  }
#endif // _DEBUG

  Initialise();

  DebugAssert();
}

size_t JavaArray::CalculateSizeInBytes( e_JavaArrayTypes type, size_t count )
{
  size_t sizeOfType = GetSizeOfValueType( type );
  return sizeOfType * count;
}

size_t JavaArray::GetSizeOfValueType( e_JavaArrayTypes type )
{
  switch ( type )
  {
    case e_JavaArrayTypes::Boolean:
      return sizeof( JavaBool );
      break;

    case e_JavaArrayTypes::Char:
      return sizeof( JavaChar );
      break;

    case e_JavaArrayTypes::Float:
      return sizeof( JavaFloat );
      break;

    case e_JavaArrayTypes::Double:
      return sizeof( JavaDouble );
      break;

    case e_JavaArrayTypes::Byte:
      return sizeof( JavaByte );
      break;

    case e_JavaArrayTypes::Short:
      return sizeof( JavaShort );
      break;

    case e_JavaArrayTypes::Integer:
      return sizeof( JavaInteger );
      break;

    case e_JavaArrayTypes::Long:
      return sizeof( JavaLong );
      break;

    case e_JavaArrayTypes::Reference:
      return sizeof( ObjectReference );
      break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Unknown type." );
      break;
  }

  return 0;
}

JavaArray::~JavaArray()
{

  for ( size_t i = 0; i < m_Size; ++ i )
  {
    IJavaVariableType *pValue = GetValueAtIndex( i );
    pValue->~IJavaVariableType();
  }
}

// JavaArray::JavaArray( const JavaArray &other )
//   : m_ContainedType( other.m_ContainedType )
//   , m_Values( other.m_Values )
//   , m_pMonitor( new std::recursive_mutex ) // NOT copying mutex
// #ifdef _DEBUG
//   , debugInitialLength( other.debugInitialLength )
// #endif // _DEBUG
// {
//   DebugAssert();
// }

void JavaArray::operator delete ( void *pObject ) throw()
{
  //ObjectFactory::FreeArray( pObject );
}

void JavaArray::operator delete ( void *pObject, void * ) throw()
{
  //ObjectFactory::FreeArray( pObject );
}

e_JavaVariableTypes JavaArray::GetVariableType() const
{
  DebugAssert();
  return e_JavaVariableTypes::Array;
}

bool JavaArray::operator==( const JavaArray &other ) const
{
  DebugAssert();
  return m_ContainedType == other.m_ContainedType && memcmp( m_pValues, other.m_pValues, CalculateSizeInBytes( m_ContainedType, m_Size ) );
}

bool JavaArray::operator==( const IJavaVariableType &other ) const
{
  DebugAssert();

  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  JVMX_ASSERT( nullptr != dynamic_cast<const ObjectReference *>( &other ) );
  return *this == *( dynamic_cast<const ObjectReference *>( &other )->GetContainedArray() );
}

IJavaVariableType *JavaArray::At( size_t index )
{
  DebugAssert();

  if ( index > m_Size )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed into array." );
  }

  DebugAssert();
#ifdef _DEBUG
  if ( GetValueAtIndex( index )->GetVariableType() == e_JavaVariableTypes::Object )
  {
    if ( reinterpret_cast<ObjectReference *>( GetValueAtIndex( index ) )->ToJObject() == ( void * )5079 )
    {
      int x = 0;
    }
  }
#endif // _DEBUG

  return GetValueAtIndex( index );
}

const IJavaVariableType *JavaArray::At( size_t index ) const
{
  return const_cast<JavaArray *>( this )->At( index );
}

size_t JavaArray::GetNumberOfElements() const
{
  //if ( m_Values.size( ) != debugInitialLength ) __asm int 3;
  DebugAssert();

  return m_Size;
}

bool JavaArray::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    JVMX_ASSERT( nullptr != dynamic_cast<const ObjectReference *>( &other ) );
    return *this < *( dynamic_cast<const ObjectReference *>( &other )->GetContainedArray() );
  }

  DebugAssert();

  return false;
}

bool JavaArray::operator<( const JavaArray &other ) const
{
  DebugAssert();

  if ( m_ContainedType < other.m_ContainedType )
  {
    return true;
  }

  for ( size_t i = 0; i < m_Size && i < other.m_Size; ++ i )
  {
    //if ( *reinterpret_cast<IJavaVariableType *> (m_pValues[i * GetSizeOfValueType( m_ContainedType )]) < *reinterpret_cast<IJavaVariableType *> (other.m_pValues[ i * GetSizeOfValueType( m_ContainedType ) ] ) )

    if ( *At( i ) < *other.At( i ) )
    {
      return true;
    }
  }

  return false;
}

e_JavaArrayTypes JavaArray::GetContainedType() const
{
  DebugAssert();

  return m_ContainedType;
}

void JavaArray::SetAt( const JavaInteger &index, const JavaInteger &value )
{
  DebugAssert();

  SetAt( index.ToHostInt32(), value );

  DebugAssert();
}

void JavaArray::SetAt( const JavaInteger &index, const JavaChar &value )
{
  DebugAssert();

  SetAt( index.ToHostInt32(), value );

  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaByte& value)
{
  DebugAssert();

  SetAt(index.ToHostInt32(), value);

  DebugAssert();
}


void JavaArray::SetAt( const JavaInteger &index, const IJavaVariableType *pValue )
{
  DebugAssert();

  SetAt( index.ToHostInt32(), pValue );

  DebugAssert();
}

void JavaArray::SetAt( const uint32_t &index, const JavaInteger &value )
{
  if ( !AreTypesCompatible( m_ContainedType, e_JavaArrayTypes::Integer ) )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Array Types are not compatible." );
  }

  if ( index < 0 )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in. Less than zero." );
  }

  if ( index >= m_Size )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in." );
  }

  if ( GetValueAtIndex( 0 )->GetVariableType() != value.GetVariableType() &&
       GetValueAtIndex( 0 )->IsIntegerCompatible() )
  {
    auto pInteger = TypeParser::UpCastToInteger( &value );
    auto result = TypeParser::DownCastFromInteger( pInteger, GetValueAtIndex( 0 )->GetVariableType() );

    InternalSetValue( index, result.get() );
  }
  else
  {
    InternalSetValue( index, &value );
  }
}

void JavaArray::SetAt( const uint32_t &index, const JavaChar &value )
{
  if ( m_ContainedType != e_JavaArrayTypes::Char )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Trying to add characters to an array that does not contain characters." );
  }

  if ( index < 0 )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in. Less than zero." );
  }

  if ( index > m_Size )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in." );
  }

  InternalSetValue( index, &value );
}

void JavaArray::SetAt(const uint32_t& index, const JavaByte& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Byte)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add byte to an array that does not contain bytes.");
  }

  if (index < 0)
  {
    throw IndexOutOfBoundsException(__FUNCTION__ " - Invalid index passed in. Less than zero.");
  }

  if (index > m_Size)
  {
    throw IndexOutOfBoundsException(__FUNCTION__ " - Invalid index passed in.");
  }

  InternalSetValue(index, &value);
}

void JavaArray::SetAt( const uint32_t &index, const IJavaVariableType *pValue )
{
  if ( !AreTypesCompatible( m_ContainedType, pValue->GetVariableType() ) )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Trying to add array element types are incompatible." );
  }

  if ( index < 0 )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in. Less than zero." );
  }

  if ( index > m_Size )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed in." );
  }

  if ( GetValueAtIndex( 0 )->GetVariableType() != pValue->GetVariableType() &&
       GetValueAtIndex( 0 )->IsIntegerCompatible() && pValue->IsIntegerCompatible() )
  {
    auto pInteger = TypeParser::UpCastToInteger( pValue );
    auto result = TypeParser::DownCastFromInteger( pInteger, GetValueAtIndex( 0 )->GetVariableType() );

    InternalSetValue( index, result.get() );
  }
  else
  {
    InternalSetValue( index, pValue );
  }

}

JavaString JavaArray::ConvertCharArrayToString() const
{
  /*  if ( m_ContainedType != e_JavaArrayTypes::Char )
    {
      throw InvalidStateException( __FUNCTION__ " - Array does not contain Characters." );
    }

    JavaString result = JavaString::EmptyString();

    for ( auto it = m_Values.begin(); it != m_Values.end(); ++ it )
    {
      result = result.Append( boost::dynamic_pointer_cast<JavaChar>(*it)->ToChar16());
    }

    return result;*/
  return JavaString::FromArray( *this );
}

JavaString JavaArray::ConvertByteArrayToString() const
{
  if ( m_ContainedType != e_JavaArrayTypes::Byte )
  {
    throw InvalidStateException( __FUNCTION__ " - Array does not contain Bytes." );
  }

  JavaString result = JavaString::EmptyString();

  char *pBuffer = new char[ m_Size + 1 ]; // +1 for terminator

  try
  {
    int i = 0;

    //for ( auto it = m_pValues.begin(); it != m_pValues.end(); ++ it )
    for ( size_t index = 0; index < m_Size; ++ index )
    {
      char chr = reinterpret_cast<const JavaByte *>( GetValueAtIndex( index ) )->ToHostInt8();
      pBuffer[ i++ ] = chr;

      // Exit after we have appended the null character.
      if ( '\0' == chr )
      {
        break;
      }
    }

    pBuffer[ i ] = '\0';

    result = result.Append( JavaString::FromCString( pBuffer ) );

    delete[] pBuffer;
    pBuffer = nullptr;
  }
  catch ( ... )
  {
    delete[] pBuffer;
  }

  return result;
}

void JavaArray::Initialise()
{
  DebugAssert();

  for ( size_t i = 0; i < m_Size; ++ i )
  {
    IJavaVariableType *pValue = GetValueAtIndex( i );
    //*pValue = *TypeParser::GetDefaultValue( m_ContainedType );

    switch ( m_ContainedType )
    {
      case e_JavaArrayTypes::Boolean:
        new ( pValue ) JavaBool( JavaBool::FromDefault() );
        break;

      case e_JavaArrayTypes::Char:
        new ( pValue ) JavaChar( JavaChar::DefaultChar() );
        break;

      case e_JavaArrayTypes::Float:
        new ( pValue ) JavaFloat( JavaFloat::FromDefault() );
        break;

      case e_JavaArrayTypes::Double:
        new ( pValue ) JavaDouble( JavaDouble::FromDefault() );
        break;

      case e_JavaArrayTypes::Byte:
        new ( pValue ) JavaByte( JavaByte::FromDefault() );
        break;

      case e_JavaArrayTypes::Short:
        new ( pValue ) JavaShort( JavaShort::FromDefault() );
        break;

      case e_JavaArrayTypes::Integer:
        new ( pValue ) JavaInteger( JavaInteger::FromDefault() );
        break;

      case e_JavaArrayTypes::Long:
        new ( pValue ) JavaLong( JavaLong::FromDefault() );
        break;

      case e_JavaArrayTypes::Reference:
        new ( pValue ) ObjectReference( nullptr );
        break;

      default:
        throw InvalidArgumentException( __FUNCTION__ " - Unknown type." );
        break;
    }
  }

  DebugAssert();
}

IJavaVariableType *JavaArray::GetValueAtIndex( size_t i )
{
  char *pValue = m_pValues + ( GetSizeOfValueType( m_ContainedType ) * i );
  JVMX_ASSERT( pValue >= m_pValues && pValue < m_pValues + CalculateSizeInBytes( m_ContainedType, m_Size ) );
  return reinterpret_cast<IJavaVariableType *>( pValue );
}

const IJavaVariableType *JavaArray::GetValueAtIndex( size_t i ) const
{
  const char *pValue = m_pValues + ( GetSizeOfValueType( m_ContainedType ) * i );
  JVMX_ASSERT( pValue >= m_pValues && pValue < m_pValues + CalculateSizeInBytes( m_ContainedType, m_Size ) );

  return reinterpret_cast<const IJavaVariableType *>( pValue );
}

std::shared_ptr<Lockable> JavaArray::MonitorEnter( const char *pFunctionName )
{
  m_pMonitor->Lock( pFunctionName );
  return m_pMonitor;
}

void JavaArray::MonitorExit( const char *pFunctionName )
{
  m_pMonitor->Unlock( pFunctionName );
}

void JavaArray::CloneOther( const JavaArray *pObjectToClone )
{
  JVMX_ASSERT( m_ContainedType == pObjectToClone->m_ContainedType );

  if ( m_ContainedType != pObjectToClone->m_ContainedType )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Contained types do not match." );
  }

  JVMX_ASSERT( pObjectToClone->m_Size == m_Size );

  for ( uint32_t i = 0; i < pObjectToClone->GetNumberOfElements(); ++ i )
  {
    SetAt( JavaInteger::FromHostInt32( i ), pObjectToClone->At( i ) );
  }
}

void JavaArray::DeepClone( const JavaArray *pObjectToClone )
{
  CloneOther( pObjectToClone );

  m_pMonitor = pObjectToClone->m_pMonitor;
}

bool JavaArray::AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaVariableTypes variableType )
{
  switch ( arrayType )
  {
    case e_JavaArrayTypes::Boolean:
      return variableType == e_JavaVariableTypes::Bool;
      break;

    case e_JavaArrayTypes::Char:
      return variableType == e_JavaVariableTypes::Char;
      break;

    case e_JavaArrayTypes::Float:
      return variableType == e_JavaVariableTypes::Float;
      break;

    case e_JavaArrayTypes::Double:
      return variableType == e_JavaVariableTypes::Double;
      break;

    case e_JavaArrayTypes::Byte:
      return variableType == e_JavaVariableTypes::Byte;
      break;

    case e_JavaArrayTypes::Short:
      return variableType == e_JavaVariableTypes::Short;
      break;

    case e_JavaArrayTypes::Integer:
      //return variableType == e_JavaVariableTypes::Integer;
      return IsTypeIntegerCompatible( variableType );
      break;

    case e_JavaArrayTypes::Long:
      return variableType == e_JavaVariableTypes::Long;
      break;

    case e_JavaArrayTypes::Reference:
      return IsVariableOfReferenceType( variableType );
      break;

    default:
      break;
  }

  return false;
}

bool JavaArray::AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaArrayTypes variableType )
{
  switch ( arrayType )
  {
    case e_JavaArrayTypes::Boolean:
      return variableType == e_JavaArrayTypes::Boolean || variableType == e_JavaArrayTypes::Integer;
      break;

    case e_JavaArrayTypes::Char:
      return variableType == e_JavaArrayTypes::Char || variableType == e_JavaArrayTypes::Integer;
      break;

    case e_JavaArrayTypes::Float:
      return variableType == e_JavaArrayTypes::Float;
      break;

    case e_JavaArrayTypes::Double:
      return variableType == e_JavaArrayTypes::Double;
      break;

    case e_JavaArrayTypes::Byte:
      return variableType == e_JavaArrayTypes::Byte || variableType == e_JavaArrayTypes::Integer;
      break;

    case e_JavaArrayTypes::Short:
      return variableType == e_JavaArrayTypes::Short || variableType == e_JavaArrayTypes::Integer;
      break;

    case e_JavaArrayTypes::Integer:
      return IsTypeIntegerCompatible( variableType );

      break;

    case e_JavaArrayTypes::Long:
      return variableType == e_JavaArrayTypes::Long;
      break;

    case e_JavaArrayTypes::Reference:
      return variableType == e_JavaArrayTypes::Reference;
      break;
  }

  return false;
}

bool JavaArray::IsVariableOfReferenceType( e_JavaVariableTypes variableType )
{
  return ( e_JavaVariableTypes::Object == variableType ) || ( e_JavaVariableTypes::Array == variableType ) || ( e_JavaVariableTypes::ClassReference == variableType ) || ( e_JavaVariableTypes::NullReference == variableType ) || ( e_JavaVariableTypes::ReturnAddress == variableType );
}

DataBuffer JavaArray::ConvertByteArrayToBuffer() const
{
  if ( m_ContainedType != e_JavaArrayTypes::Byte )
  {
    throw InvalidStateException( __FUNCTION__ " - Array does not contain Bytes." );
  }

  DataBuffer result = DataBuffer::EmptyBuffer();
  for ( size_t i = 0; i < m_Size; ++ i )
  {
    const IJavaVariableType *pValue = GetValueAtIndex( i );
    uint8_t byteValue = dynamic_cast<const JavaByte *>( pValue )->ToHostInt8();
    result = result.AppendUint8( byteValue );
  }

  return result;
}

e_JavaArrayTypes JavaArray::ConvertTypeFromChar( char16_t charType )
{
  switch ( charType )
  {
    case c_JavaTypeSpecifierByte:
      return e_JavaArrayTypes::Byte;
      break;

    case c_JavaTypeSpecifierChar:
      return e_JavaArrayTypes::Char;
      break;

    case c_JavaTypeSpecifierInteger:
      return e_JavaArrayTypes::Integer;
      break;

    case c_JavaTypeSpecifierShort:
      return e_JavaArrayTypes::Short;
      break;

    case c_JavaTypeSpecifierBool:
      return e_JavaArrayTypes::Boolean;
      break;

    case c_JavaTypeSpecifierFloat:
      return e_JavaArrayTypes::Float;
      break;

    case c_JavaTypeSpecifierDouble:
      return e_JavaArrayTypes::Double;
      break;

    case c_JavaTypeSpecifierLong:
      return e_JavaArrayTypes::Long;
      break;

    case c_JavaTypeSpecifierReference:
      return e_JavaArrayTypes::Reference;
      break;

    default:
      throw UnsupportedTypeException( __FUNCTION__ " - Unknown type found." );
      break;
  }
}

bool JavaArray::IsReferenceType() const
{
  DebugAssert();
  return true;
}

bool JavaArray::IsIntegerCompatible() const
{
  DebugAssert();
  return false;
}

JavaString JavaArray::ToString() const
{
  DebugAssert();
  int count = 0;

  char buffer[ 38 ] = { 0 };
  _snprintf( buffer, 37, "(%d)", m_Size );

  std::basic_stringstream<char16_t> outputStream;

  outputStream << buffer << "[";

  //JavaString result = JavaString::FromCString( buffer ).Append( JavaString::FromCString( JVMX_T("[") ) );
  for ( size_t i = 0; i < m_Size; ++ i )
  {
    const IJavaVariableType *pValue = GetValueAtIndex( i );
    outputStream << pValue->ToString().ToCharacterArray();
    if ( i != m_Size )
    {
      outputStream << u", ";
    }

    if ( count > 20 )
    {
      outputStream << u"...";
      break;
    }

    ++ count;
  }

  DebugAssert();

  outputStream << u"]";
  return JavaString::FromCString(outputStream.str().c_str());
}

void JavaArray::DebugAssert() const
{
#ifdef _DEBUG
  //if ( m_Values.size() != debugInitialLength ) __asm int 3;
  JVMX_ASSERT( m_Size == debugInitialLength );
#endif // _DEBUG
}

void JavaArray::InternalSetValue( size_t index, const IJavaVariableType *pFinalValue )
{
  IJavaVariableType *pValue = GetValueAtIndex( index );
  *pValue = *pFinalValue;

  DebugAssert();
}

bool JavaArray::IsNull() const
{
  return false;
}

// boost::intrusive_ptr<IJavaVariableType> JavaArray::ConvertReferenceTypeForArrayStorage( const IJavaVariableType *pValue ) const
// {
//   boost::intrusive_ptr<IJavaVariableType> pConvertedValue = nullptr;
//
//   if ( pValue->GetVariableType() == e_JavaVariableTypes::Object )
//   {
//     pConvertedValue = boost::dynamic_pointer_cast<ObjectReference>(pValue);
//   }
//   else if ( pValue->GetVariableType() == e_JavaVariableTypes::NullReference )
//   {
//     pConvertedValue = boost::dynamic_pointer_cast<ObjectReference>(pValue);
//   }
//   else if ( pValue->GetVariableType() == e_JavaVariableTypes::ClassReference )
//   {
//     pConvertedValue = boost::dynamic_pointer_cast<JavaClassReference>(pValue);
//   }
//   else if ( pValue->GetVariableType() == e_JavaVariableTypes::Array )
//   {
//     pConvertedValue = boost::dynamic_pointer_cast<ObjectReference>(pValue);
//   }
//   else if ( pValue->GetVariableType() == e_JavaVariableTypes::ReturnAddress )
//   {
//     pConvertedValue = boost::dynamic_pointer_cast<JavaReturnAddress>(pValue);
//   }
//   else
//   {
//     JVMX_ASSERT( false );
//     throw UnsupportedTypeException( __FUNCTION__ " - Type not supported in an array while trying to cast from reference." );
//   }
//
//   return pConvertedValue;
// }

bool JavaArray::IsTypeIntegerCompatible( e_JavaArrayTypes variableType )
{
  return variableType == e_JavaArrayTypes::Short || variableType == e_JavaArrayTypes::Integer || variableType == e_JavaArrayTypes::Char || variableType == e_JavaArrayTypes::Boolean || variableType == e_JavaArrayTypes::Byte;
}

bool JavaArray::IsTypeIntegerCompatible( e_JavaVariableTypes variableType )
{
  return variableType == e_JavaVariableTypes::Short || variableType == e_JavaVariableTypes::Integer || variableType == e_JavaVariableTypes::Char || variableType == e_JavaVariableTypes::Bool || variableType == e_JavaVariableTypes::Byte;
}

boost::intrusive_ptr<IJavaVariableType> JavaArray::ConvertIntegerTypeForArrayStorage( const JavaInteger &value ) const
{
  boost::intrusive_ptr<IJavaVariableType> pFinalValue = nullptr;

  switch ( m_ContainedType )
  {
    case e_JavaArrayTypes::Boolean:
      pFinalValue = value.ToBool();
      break;

    case e_JavaArrayTypes::Char:
      pFinalValue = value.ToChar();
      break;

    case e_JavaArrayTypes::Byte:
      pFinalValue = value.ToByte();
      break;

    case e_JavaArrayTypes::Short:
      pFinalValue = value.ToShort();
      break;

    case e_JavaArrayTypes::Integer:
      pFinalValue = new JavaInteger( value );
      break;

    default:
      JVMX_ASSERT( false );
      throw InvalidArgumentException( __FUNCTION__ " - Array Types are not compatible. This should have been caught before." );
      break;
  }

  return pFinalValue;
}
// 

boost::intrusive_ptr<ObjectReference> JavaArray::CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const char *pBuffer )
{
  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );

  size_t length = strlen( pBuffer );
  boost::intrusive_ptr<ObjectReference> pResult = pThreadManager->GetCurrentThreadState()->CreateArray( e_JavaArrayTypes::Char, length );
  //boost::intrusive_ptr<ObjectReference> pResult = new JavaArray( /*pMemoryManager, */e_JavaArrayTypes::Char, length );
  for ( size_t i = 0; i < length; ++ i )
  {
    pResult->GetContainedArray()->SetAt( static_cast<uint32_t>( i ), JavaChar::FromCChar( pBuffer[ i ] ) );
  }

  return pResult;
}

boost::intrusive_ptr<ObjectReference> JavaArray::CreateFromCArray(const uint8_t* pBuffer, size_t length)
{
  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get("ThreadManager");

  boost::intrusive_ptr<ObjectReference> pResult = pThreadManager->GetCurrentThreadState()->CreateArray(e_JavaArrayTypes::Byte, length);
  //boost::intrusive_ptr<ObjectReference> pResult = new JavaArray( /*pMemoryManager, */e_JavaArrayTypes::Char, length );
  for (size_t i = 0; i < length; ++i)
  {
    pResult->GetContainedArray()->SetAt(static_cast<uint32_t>(i), JavaByte::FromHostInt8(pBuffer[i]));
  }

  return pResult;
}