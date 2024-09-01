
#include "GlobalConstants.h"

#include "IJavaVariableType.h"

#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "JavaTypes.h"
#include "JavaClass.h"

#include "FieldInfo.h"
#include "TypeParser.h"

#include "GlobalCatalog.h"
#include "IThreadManager.h"
#include "IVirtualMachineState.h"
#include "ILogger.h"

#include "JavaObject.h"
#include "HelperTypes.h"
#include "JavaExceptionConstants.h"

extern const JavaString c_SyntheticField_ClassName;

JavaObject::JavaObject( std::shared_ptr<JavaClass> pClass )
  : m_pClass( pClass )
  , m_pMonitor( std::make_shared<Lockable>() )
  , m_Waitable( std::make_shared<std::condition_variable_any>() )
  , m_Notfied( false )
{
  if ( nullptr == pClass )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a non-null class pointer." );
  }

#ifdef _DEBUG
  memset( m_pFields, 0xAC, pClass->CalculateInstanceSizeInBytes() );
#endif // _DEBUG

  InitialiseFields( m_pClass );
}

size_t JavaObject::InitialiseFields( const std::shared_ptr<JavaClass> &pClass )
{
  if ( nullptr == pClass )
  {
    return 0;
  }

#ifdef _DEBUG
  size_t debugInstanceSize = pClass->CalculateInstanceSizeInBytes();
  //memset( m_pFields, 0xAC, debugInstanceSize );
#endif // _DEBUG

  size_t startingOffset = InitialiseFields( pClass->GetSuperClass() );

  //   std::shared_ptr<JavaClass> pSuperClass = m_pClass->GetSuperClass();
  //   while ( nullptr != pSuperClass )
  //   {
  //     for ( size_t i = 0; i < pSuperClass->GetFieldCount(); ++i )
  //     {
  //       startingOffset += InitialiseField( pSuperClass, i, startingOffset );
  //     }
  //
  //     pSuperClass = pSuperClass->GetSuperClass();
  //   }

  for ( size_t i = 0; i < pClass->GetLocalFieldCount( e_PublicOnly::No ); ++i )
  {
    InitialiseField( pClass, i, startingOffset );
  }

  return pClass->CalculateInstanceSizeInBytes();
}

size_t JavaObject::InitialiseField( std::shared_ptr<JavaClass> pClass, int i, size_t startingOffset )
{
  std::shared_ptr<FieldInfo> pInfo = pClass->GetFieldByIndex( i );
  if ( pInfo->IsStatic() )
  {
    return 0;
  }

  char *pAddress = m_pFields + pInfo->GetOffset() + startingOffset;

#ifdef _DEBUG
  //  m_DebugList.push_back( std::pair<JavaString, size_t>( *pInfo->GetName(), pInfo->GetOffset() + startingOffset ) );
#endif // _DEBUG

#ifdef _DEBUG
  char c = *pAddress;
  if ( c != ( char )0xAC )
  {
    JVMX_ASSERT( false );
  }
#endif // _DEBUG

  switch ( TypeParser::ConvertTypeDescriptorToVariableType( pInfo->GetType()->At( 0 ) ) )
  {
    case e_JavaVariableTypes::Char:
      new ( pAddress ) JavaChar( JavaChar::FromDefault() );
      break;
    case e_JavaVariableTypes::Byte:
      new ( pAddress ) JavaByte( JavaByte::FromDefault() );
      break;
    case e_JavaVariableTypes::Short:
      new ( pAddress ) JavaShort( JavaShort::FromDefault() );
      break;
    case e_JavaVariableTypes::Integer:
      new ( pAddress ) JavaInteger( JavaInteger::FromDefault() );
      break;
    case e_JavaVariableTypes::Long:
      new ( pAddress ) JavaLong( JavaLong::FromDefault() );
      break;
    case e_JavaVariableTypes::Float:
      new ( pAddress ) JavaFloat( JavaFloat::FromDefault() );
      break;
    case e_JavaVariableTypes::Double:
      new ( pAddress ) JavaDouble( JavaDouble::FromDefault() );
      break;
    case e_JavaVariableTypes::String:
      new ( pAddress ) JavaString( JavaString::EmptyString() );
      break;
    case e_JavaVariableTypes::NullReference:
      JVMX_ASSERT( false );
      new ( pAddress ) ObjectReference( nullptr );
      break;
    case e_JavaVariableTypes::ReturnAddress:
      new ( pAddress ) JavaReturnAddress( JavaReturnAddress( 0 ) );
      break;
    case e_JavaVariableTypes::ClassReference:
      JVMX_ASSERT( false );
      throw InvalidStateException( __FUNCTION__ " - Unexpected field type." );
      break;
    case e_JavaVariableTypes::Bool:
      new ( pAddress ) JavaBool( JavaBool::FromDefault() );
      break;
    case e_JavaVariableTypes::Array:
      {
        //new (pAddress) ObjectReference( *HelperTypes::CreateArray( e_JavaArrayTypes::Char, 0 ) );
        new ( pAddress ) ObjectReference( nullptr );
      }
      break;
    case e_JavaVariableTypes::Object:
      new ( pAddress ) ObjectReference( nullptr );
      break;
    default:
      throw InvalidStateException( __FUNCTION__ " - Invalid field type." );
      break;
  }

  return pInfo->GetByteSize();
}

bool JavaObject::ThrowJavaExceptionIfInterrupted() const
{
  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
  std::shared_ptr<IVirtualMachineState> pCurrentThreadState = pThreadManager->GetCurrentThreadState();

  if ( !pCurrentThreadState->GetInterruptedFlag() )
  {
    return false;
  }

  auto pClass = pCurrentThreadState->InitialiseClass( JavaString::FromCString( c_JavaInterruptedException ) );
  if ( nullptr == pClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Java exception class could not be loaded." );
  }

  //if ( !pCurrentThreadState->IsClassInitialised( *pClass->GetName() ) )
  //{
  //  pCurrentThreadState->InitialiseClass( *pClass->GetName() );
  //}

  auto pExceptionObject = pCurrentThreadState->CreateObject( pClass );
  pCurrentThreadState->SetExceptionThrown( pExceptionObject );

  return true;
}

void JavaObject::Wait( JavaLong milliSeconds, JavaInteger nanoSeconds )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "Waiting on object : (%s)", ToString().ToUtf8String().c_str() );
#endif // _DEBUG

  // We don't set m_Notified to false here, because if the notify happens before the wait, it would be ignored.

  size_t recursionLevel = m_pMonitor->GetRecursionLevel();
  for ( size_t i = 0; i < recursionLevel; ++i )
  {
    m_pMonitor->Unlock( __FUNCTION__ );
  }

  if ( 0 == milliSeconds.ToHostInt64() && 0 == nanoSeconds.ToHostInt32() )
  {
    std::unique_lock<Lockable> lock( *m_pMonitor );
    while ( !m_Notfied )
    {
      m_Waitable->wait_for( lock, std::chrono::milliseconds( 1000 ) + std::chrono::nanoseconds( nanoSeconds.ToHostInt32() ) );

      if ( ThrowJavaExceptionIfInterrupted() )
      {
        break;
      }
      //m_Waitable->wait( lock );
    }
    lock.unlock();
  }
  else
  {
    std::unique_lock<Lockable> lock( *m_pMonitor );
    m_Waitable->wait_for( lock, std::chrono::milliseconds( milliSeconds.ToHostInt64() ) + std::chrono::nanoseconds( nanoSeconds.ToHostInt32() ), [this] { return m_Notfied; } );

    ThrowJavaExceptionIfInterrupted();
  }

  for ( size_t i = 0; i < recursionLevel; ++i )
  {
    m_pMonitor->Lock( __FUNCTION__ );
  }

  // We set m_Notified to false here so that it is reset for next time.
  m_Notfied = false;
}

void JavaObject::NotifyOne()
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "Notifying one on object : (%s)", ToString().ToUtf8String().c_str() );
#endif // _DEBUG

  m_Notfied = true;
  m_Waitable->notify_one();
}

void JavaObject::NotifyAll()
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "Notifying all on object : (%s)\n", ToString().ToUtf8String().c_str() );
#endif // _DEBUG

  m_Notfied = true;
  m_Waitable->notify_all();
}

bool JavaObject::IsInstanceOf( const JavaString &pPossibleSuperClassName ) const
{
  std::shared_ptr<JavaClass> pSuperClass = this->GetClass();
  while ( nullptr != pSuperClass )
  {
    if ( *pSuperClass->GetName() == pPossibleSuperClassName )
    {
      return true;
    }

    if ( nullptr == pSuperClass->GetSuperClass() )
    {
      return false;
    }

    pSuperClass = pSuperClass->GetSuperClass();
  }

  return false;
}

size_t JavaObject::GetSizeInBytes() const
{
  return m_pClass->CalculateInstanceSizeInBytes();
}

e_JavaVariableTypes JavaObject::GetVariableType() const
{
  return e_JavaVariableTypes::Object;
}

bool JavaObject::operator==( const JavaObject &other ) const
{
  AssertValid();

  if ( this == &other )
  {
    return true;
  }

  if ( *m_pClass->GetName() != *other.m_pClass->GetName() )
  {
    AssertValid();
    return false;
  }

  //   if ( m_pClass->GetFieldCount() != other.GetClass()->GetFieldCount() )
  //   {
  //     AssertValid();
  //     return false;
  //   }

  AssertValid();

  return 0 == CompareFields( other );
}

bool JavaObject::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  JVMX_ASSERT( nullptr != dynamic_cast<const ObjectReference *>( &other ) );
  return *this == *( dynamic_cast<const ObjectReference *>( &other )->GetContainedObject() );
}

boost::intrusive_ptr<IJavaVariableType> JavaObject::GetFieldByName( const JavaString &name )
{
  return GetFieldByNameConst( name );
}

std::shared_ptr<FieldInfo>  JavaObject::ResolveField( const JavaString &name, size_t &fieldOffset ) const
{
  fieldOffset = 0;
  std::shared_ptr<FieldInfo> pFieldInfo = m_pClass->GetFieldByName( name );
  if ( nullptr != pFieldInfo )
  {
    std::shared_ptr<JavaClass> pSuperClass = m_pClass->GetSuperClass();
    if ( nullptr != pSuperClass )
    {
      fieldOffset = pSuperClass->CalculateInstanceSizeInBytes();
    }

    return pFieldInfo;
  }

  std::shared_ptr<JavaClass> pSuperClass = m_pClass->GetSuperClass();
  while ( nullptr != pSuperClass )
  {
    pFieldInfo = pSuperClass->GetFieldByName( name );

    pSuperClass = pSuperClass->GetSuperClass();

    if ( nullptr != pFieldInfo )
    {
      fieldOffset = pSuperClass->CalculateInstanceSizeInBytes();
      return pFieldInfo;
    }
  }

  return pFieldInfo;
}

boost::intrusive_ptr<IJavaVariableType> JavaObject::GetFieldByNameConst( const JavaString &name ) const
{
  //  this does not take into account super classes :(

  size_t startingOffset = 0; // CalculateStartingOffset();

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( name, startingOffset );

  const IJavaVariableType *pFieldValue = reinterpret_cast<const IJavaVariableType *>( m_pFields + startingOffset + pFieldInfo->GetOffset() );
  if ( nullptr == pFieldInfo )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Field does not exist or is static." );
  }

  JVMX_ASSERT( pFieldValue->IsNull() || pFieldValue->GetVariableType() == e_JavaVariableTypes::Array ||
               pFieldValue->GetVariableType() == TypeParser::ConvertTypeDescriptorToVariableType( pFieldInfo->GetType()->At( 0 ) ) );

  switch ( pFieldValue->GetVariableType() )
  {
    case e_JavaVariableTypes::Char:
      return new JavaChar( *dynamic_cast<const JavaChar *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Byte:
      return new JavaByte( *dynamic_cast<const JavaByte *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Short:
      return new JavaShort( *dynamic_cast<const JavaShort *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Integer:
      return new JavaInteger( *dynamic_cast<const JavaInteger *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Long:
      return new JavaLong( *dynamic_cast<const JavaLong *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Float:
      return new JavaFloat( *dynamic_cast<const JavaFloat *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Double:
      return new JavaDouble( *dynamic_cast<const JavaDouble *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::String:
      return new JavaString( *dynamic_cast<const JavaString *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::NullReference:
      return new ObjectReference( nullptr );
      break;
    case e_JavaVariableTypes::ReturnAddress:
      return new JavaReturnAddress( *dynamic_cast<const JavaReturnAddress *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::ClassReference:
      JVMX_ASSERT( false );
      throw InvalidStateException( __FUNCTION__ " - Unexpected field type." );
      break;
    case e_JavaVariableTypes::Bool:
      return new JavaBool( *dynamic_cast<const JavaBool *>( pFieldValue ) );
      break;
    case e_JavaVariableTypes::Array:
    case e_JavaVariableTypes::Object:
      return new ObjectReference( *dynamic_cast<const ObjectReference *>( pFieldValue ) );
      break;
    default:
      throw InvalidStateException( __FUNCTION__ " - Invalid field type." );
      break;
  }

  AssertValid();

  JVMX_ASSERT( false );
  return nullptr;
}

JavaObject &JavaObject::operator=( const JavaObject &other )
{
  AssertValid();

  // NB: Note that we are deliberately not copying the mutex or the notified field!
  m_pClass = other.m_pClass;
  memcpy( m_pFields, other.m_pFields, m_pClass->CalculateInstanceSizeInBytes() );
  m_JVMXFields = other.m_JVMXFields;

  AssertValid();

  return *this;
}

JavaObject::~JavaObject() JVMX_NOEXCEPT
{
  m_pClass.reset();
  //m_Fields.clear();
  m_JVMXFields.clear();
}

void JavaObject::operator delete ( void *pObject )
{
  //ObjectFactory::FreeObject( pObject );
}

void JavaObject::operator delete ( void *pObject, void * )
{
  //ObjectFactory::FreeObject( pObject );
}

std::shared_ptr<JavaClass> JavaObject::GetClass() const
{
  AssertValid();

  return m_pClass;
}

bool JavaObject::operator<( const IJavaVariableType &other ) const
{
  AssertValid();

  if ( GetVariableType() == other.GetVariableType() )
  {
    JVMX_ASSERT( nullptr != dynamic_cast<const ObjectReference *>( &other ) );
    return *this < *( dynamic_cast<const ObjectReference *>( &other )->GetContainedObject() );
  }

  return false;
}

bool JavaObject::operator<( const JavaObject &other ) const
{
  AssertValid();

  if ( *m_pClass->GetName() < *other.m_pClass->GetName() )
  {
    return true;
  }

  return CompareFields( other ) < 0;
}

void JavaObject::SetField( const JavaString &name, IJavaVariableType *pNewValue, bool ignoreFieldAccess )
{
  AssertValid();

  JVMX_ASSERT( nullptr != pNewValue );

  size_t startingOffset = 0;

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( name, startingOffset );
  if ( nullptr == pFieldInfo )
  {
#if defined(_DEBUG)
    std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
    pThreadManager->GetCurrentThreadState()->LogCallStack();
    pThreadManager->GetCurrentThreadState()->LogLocalVariables();
    pThreadManager->GetCurrentThreadState()->LogOperandStack();
#endif // _DEBUG
    throw IndexOutOfBoundsException( __FUNCTION__ " - Field does not exist or is static." );
  }

  if ( !ignoreFieldAccess && !pFieldInfo->IsPublic() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Field is not public." );
  }

  if ( !ignoreFieldAccess && pFieldInfo->IsFinal() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Field is final." );
  }

  IJavaVariableType *pFieldValue = reinterpret_cast<IJavaVariableType *>( m_pFields + startingOffset + pFieldInfo->GetOffset() );
  *pFieldValue = *pNewValue;

  AssertValid();
}

void JavaObject::SetField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue, bool ignoreFieldAccess )
{
  SetField( name, pValue.get(), ignoreFieldAccess );
}

std::shared_ptr<Lockable> JavaObject::MonitorEnter( const char *pFuctionName )
{
  m_pMonitor->Lock( pFuctionName );
  return m_pMonitor;
}

void JavaObject::MonitorExit( const char *pFuctionName )
{
  m_pMonitor->Unlock( pFuctionName );
}

bool JavaObject::IsReferenceType() const
{
  AssertValid();
  return true;
}

bool JavaObject::IsIntegerCompatible() const
{
  AssertValid();
  return false;
}

JavaString JavaObject::ToString() const
{
  AssertValid();

  if ( *m_pClass->GetName() == JavaString::FromCString( JVMX_T( "java/lang/String" ) ) )
  {
    //boost::intrusive_ptr<IJavaVariableType> pFieldValue = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "value" ) ) );
    JavaString value = HelperTypes::ExtractValueFromStringObject(this);

    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( value ).Append( JVMX_T( "}" ) );
  }

  if ( *m_pClass->GetName() == JavaString::FromCString( JVMX_T( "java/lang/Class" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pFieldValue = GetJVMXFieldByName( c_SyntheticField_ClassName );
    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( pFieldValue->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/lang/Throwable" ) ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pFieldValue = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "detailMessage" ) ) );
    boost::intrusive_ptr<IJavaVariableType> pVMStateValue = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "vmState" ) ) );
    boost::intrusive_ptr<IJavaVariableType> pStackTraceValue = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "stackTrace" ) ) );
    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {detailMessage" ) ).Append( pFieldValue->ToString() ).Append( JVMX_T( "}{vmState:" ) ).Append( pVMStateValue->ToString() ).Append( JVMX_T( "}{stackTrace:" ) ).Append( pStackTraceValue->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/util/Hashtable" ) ) ) )
  {
    boost::intrusive_ptr<JavaInteger> pSize = boost::dynamic_pointer_cast<JavaInteger>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "size" ) ) ) );
    JavaString result = JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "}(" ) ).Append( pSize->ToHostInt32() ).Append( JVMX_T( ") = {" ) );

    boost::intrusive_ptr<ObjectReference> pBuckets = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "buckets" ) ) ) );
    if ( !pBuckets->IsNull() )
    {
      for ( size_t i = 0; i < pBuckets->GetContainedArray()->GetNumberOfElements(); ++ i )
      {
        IJavaVariableType *pValueAtI = pBuckets->GetContainedArray()->At( i );
        ObjectReference *pEntry = static_cast< ObjectReference * >( static_cast< void * >( pValueAtI ) );

        if ( !pEntry->IsNull() )
        {
          result = result.Append( pEntry->GetContainedObject()->ToString() ).Append( JVMX_T( "\r\n" ) );

          boost::intrusive_ptr<ObjectReference> pNextObject = boost::dynamic_pointer_cast<ObjectReference>( pEntry->GetContainedObject()->GetFieldByNameConst( JavaString::FromCString( JVMX_T( "next" ) ) ) );
          while ( !pNextObject->IsNull() )
          {
            result = result.Append( pEntry->GetContainedObject()->ToString() ).Append( JVMX_T( "\r\n" ) );

            pNextObject = boost::dynamic_pointer_cast<ObjectReference>( pNextObject->GetContainedObject()->GetFieldByNameConst( JavaString::FromCString( JVMX_T( "next" ) ) ) );
          }
        }
      }
    }
    else
    {
      result.Append( JVMX_T( "empty" ) );
    }

    return result.Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/util/Hashtable$HashEntry" ) ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pKey = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "key" ) ) );
    boost::intrusive_ptr<IJavaVariableType> pValue = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "value" ) ) );
    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( pKey->ToString() ).Append( JVMX_T( " = " ) ).Append( pValue->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "gnu/java/awt/color/TagEntry" ) ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pSignature = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "signature" ) ) );
    boost::intrusive_ptr<ObjectReference> pData = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "data" ) ) ) );

    int32_t num = 0;
    if ( !pData->IsNull() && pData->GetContainedArray()->GetNumberOfElements() > 4 )
    {
      num = ( ( JavaByte * )( pData->GetContainedArray()->At( 0 ) ) )->ToHostInt8();
      num += ( ( JavaByte * )( pData->GetContainedArray()->At( 1 ) ) )->ToHostInt8() << 8;
      num += ( ( JavaByte * )( pData->GetContainedArray()->At( 2 ) ) )->ToHostInt8() << 16;
      num += ( ( JavaByte * )( pData->GetContainedArray()->At( 3 ) ) )->ToHostInt8() << 24;
    }

    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( pSignature->ToString() ).Append( JVMX_T( "(0x" ) ).AppendHex( num ).Append( JVMX_T( ")" ) ).Append( JVMX_T( " = " ) ).Append( pData->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/nio/ByteBuffer" ) ) ) )
  {
    boost::intrusive_ptr<ObjectReference> pBackingBuffer = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "backing_buffer" ) ) ) );
    boost::intrusive_ptr<IJavaVariableType> pOffset = GetFieldByNameConst( JavaString::FromCString( JVMX_T( "array_offset" ) ) );

    int32_t num = 0;
    if ( !pBackingBuffer->IsNull() && pBackingBuffer->GetContainedArray()->GetNumberOfElements() > 4 )
    {
      num = ( ( JavaByte * )( pBackingBuffer->GetContainedArray()->At( 0 ) ) )->ToHostInt8();
      num += ( ( JavaByte * )( pBackingBuffer->GetContainedArray()->At( 1 ) ) )->ToHostInt8() << 8;;
      num += ( ( JavaByte * )( pBackingBuffer->GetContainedArray()->At( 2 ) ) )->ToHostInt8() << 16;
      num += ( ( JavaByte * )( pBackingBuffer->GetContainedArray()->At( 3 ) ) )->ToHostInt8() << 24;
    }

    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( pBackingBuffer->ToString() ).Append( JVMX_T( "(0x" ) ).AppendHex( num ).Append( JVMX_T( ")" ) ).Append( JVMX_T( " = " ) ).Append( pOffset->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/lang/reflect/Field" ) ) ) )
  {
    boost::intrusive_ptr<ObjectReference> pVMField = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "f" ) ) ) );

    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = {" ) ).Append( pVMField->ToString() ).Append( JVMX_T( "}" ) );
  }

  if ( IsInstanceOf( JavaString::FromCString( JVMX_T( "java/lang/reflect/VMField" ) ) ) )
  {
    boost::intrusive_ptr<ObjectReference> pClazz = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "clazz" ) ) ) );
    boost::intrusive_ptr<ObjectReference> pName = boost::dynamic_pointer_cast<ObjectReference>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "name" ) ) ) );
    boost::intrusive_ptr<JavaInteger> pSlot = boost::dynamic_pointer_cast<JavaInteger>( GetFieldByNameConst( JavaString::FromCString( JVMX_T( "slot" ) ) ) );

    return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "} = { (" ) )
           .Append( JVMX_T( "clazz" ) ).Append( JVMX_T( " = " ) ).Append( pClazz->ToString() ).Append( JVMX_T( " ) ( " ) )
           .Append( JVMX_T( "name" ) ).Append( JVMX_T( " = " ) ).Append( pName->ToString() ).Append( JVMX_T( " ) ( " ) )
           .Append( JVMX_T( "slot" ) ).Append( JVMX_T( " = " ) ).Append( pSlot->ToString() ).Append( JVMX_T( " ) ( " ) )
           .Append( JVMX_T( ") }" ) );

  }

  if (IsInstanceOf(JavaString::FromCString(JVMX_T("java/net/URL"))))
  {
    boost::intrusive_ptr<ObjectReference> pProtocol = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("protocol"))));
    boost::intrusive_ptr<ObjectReference> pAuthority = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("authority"))));
    boost::intrusive_ptr<ObjectReference> pHost = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("host"))));
    boost::intrusive_ptr<ObjectReference> pUserInfo = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("userInfo"))));
    boost::intrusive_ptr<JavaInteger> pPort = boost::dynamic_pointer_cast<JavaInteger>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("port"))));
    boost::intrusive_ptr<ObjectReference> pFile = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("file"))));
    boost::intrusive_ptr<ObjectReference> pRef = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("ref"))));

    return JavaString::FromCString(JVMX_T("{")).Append(*(m_pClass->GetName())).Append(JVMX_T("} = { \n\t"))
      .Append(JVMX_T("protocol")).Append(JVMX_T(" = ")).Append(pProtocol->ToString()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("userInfo")).Append(JVMX_T(" = ")).Append(pUserInfo->ToString()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("authority")).Append(JVMX_T(" = ")).Append(pAuthority->ToString()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("host")).Append(JVMX_T(" = ")).Append(pHost->ToString()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("port")).Append(JVMX_T(" = ")).Append(pPort->ToHostInt32()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("file")).Append(JVMX_T(" = ")).Append(pFile->ToString()).Append(JVMX_T("\n\t"))
      .Append(JVMX_T("ref")).Append(JVMX_T(" = ")).Append(pRef->ToString().Append(JVMX_T("\n")))
      .Append(JVMX_T(" }")); 
  }

  if (IsInstanceOf(JavaString::FromCString(JVMX_T("java/lang/StringBuilder"))))
  {
    boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("value"))));
    boost::intrusive_ptr<JavaInteger> pCount = boost::dynamic_pointer_cast<JavaInteger>(GetFieldByNameConst(JavaString::FromCString(JVMX_T("count"))));
    JavaString content = JavaString::EmptyString();
    if (!pObject->IsNull())
    {
      auto pValue = pObject->GetContainedArray();
      content = pValue->ToString();
    }
    else
    {
      content = JavaString::FromCString(JVMX_T("{null}"));
    }
    
    return JavaString::FromCString(JVMX_T("{")).Append(*(m_pClass->GetName())).Append(JVMX_T("} = {")).Append(content).Append(JVMX_T(" }"));
  }

  return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClass->GetName() ) ).Append( JVMX_T( "}" ) );
}

void JavaObject::AssertValid() const
{}

bool JavaObject::IsNull() const
{
  return nullptr == m_pClass;
}

void JavaObject::CloneOther( const JavaObject *pObjectToClone )
{
  JVMX_ASSERT( pObjectToClone->m_pClass->GetName() == m_pClass->GetName() );
  memcpy( m_pFields, pObjectToClone->m_pFields, m_pClass->CalculateInstanceSizeInBytes() );

  //m_Fields = pObjectToClone->m_Fields;
  m_JVMXFields = pObjectToClone->m_JVMXFields;

  // for Garbage Collection
}

void JavaObject::DeepClone( const JavaObject *pObjectToClone )
{
  CloneOther( pObjectToClone );

  //m_Fields = pObjectToClone->m_Fields;
  m_JVMXFields = pObjectToClone->m_JVMXFields;
  m_pMonitor = pObjectToClone->m_pMonitor;
  m_Notfied = pObjectToClone->m_Notfied;
  m_Waitable = pObjectToClone->m_Waitable;
  m_pClass = pObjectToClone->m_pClass;
}

// const IJavaVariableType *JavaObject::GetFieldByIndex( size_t index ) const
// {
//   JVMX_ASSERT( false );
//   JVMX_ASSERT( m_pClass->GetLocalFieldCount() < index );
//
//   std::shared_ptr<JavaClass> pSuperClass = m_pClass->GetSuperClass();
//   size_t startingOffset = 0;
//   if ( nullptr == pSuperClass )
//   {
//     startingOffset = pSuperClass->CalculateInstanceSizeInBytes();
//   }
//
//   return reinterpret_cast<const IJavaVariableType *>(m_pFields + startingOffset + m_pClass->GetFieldByIndex( index )->GetOffset());
// }

int JavaObject::CompareFields( const JavaObject &other ) const
{
  AssertValid();

  for ( size_t i = 0; i < m_pClass->GetNonStaticFieldCount(); ++ i )
  {
    const IJavaVariableType *pThisFieldValue = reinterpret_cast<const IJavaVariableType *>( m_pFields + m_pClass->GetFieldByIndex( i )->GetOffset() );
    const IJavaVariableType *pOtherFieldValue = reinterpret_cast<const IJavaVariableType *>( other.m_pFields + m_pClass->GetFieldByIndex( i )->GetOffset() );

    if ( *pThisFieldValue < *pOtherFieldValue )
    {
      return -1;
    }

    // If it's not less, and it's not equal, it must be greater.
    // Note, we are using !( x == y ) because the != operator likely hasn't been defined.
    if ( !( *pThisFieldValue == *pOtherFieldValue ) )
    {
      return 1;
    }
  }

  return 0;
}

boost::intrusive_ptr<IJavaVariableType> JavaObject::GetJVMXFieldByName( const JavaString &name ) const
{
  return m_JVMXFields.at( name );
}

void JavaObject::SetJVMXField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue )
{
  m_JVMXFields[ name ] = pValue;
}