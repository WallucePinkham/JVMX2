
#include <cinttypes>
#include <thread>

#include "OsFunctions.h"

#include "FileDoesNotExistException.h"
#include "InvalidArgumentException.h"
#include "NotImplementedException.h"
#include "JavaNativeInterfaceLibrary.h"
#include "InvalidStateException.h"

#include "JavaTypes.h"
#include "TypeParser.h"

#include "ObjectReference.h"

#include "ILogger.h"
#include "GlobalCatalog.h"

#include "JavaNativeInterface.h"

#if defined(_MSC_VER) && defined(_M_IX86)
#define ASM_PUSH_INT( value ) __asm push value;
#define ASM_PUSH_FLOAT( value ) __asm fld (value) __asm  sub esp,8  __asm fstp QWORD PTR [esp]
#define ASM_CALL_FUNCTION( pointer ) __asm call pointer;
#define ASM_POP_INT( value ) __asm pop value;
#define ASM_GET_RETVAL( value ) __asm mov value, eax;
#define ASM_GET_RETVAL_EXTRABITS( value ) __asm mov value, edx;
#else // _MSC_VER
#error "You need to implement a ASM_PUSH_INT and ASM_PUSH_FLOAT for your platform."
#endif // _MSC_VER

#define ADD_INT_TO_STACK( array, index, value ) array[( index )] = ( value );


typedef void *JniMethodImplementation;

const JavaString c_JavaNativeInterfaceInitialMethodString = JavaString::FromCString( "Java_" );

JavaNativeInterface::JavaNativeInterface()
  : m_pExportedVM( nullptr )
  , m_pExportedEnvironment( nullptr )
#ifdef _DEBUG
  , m_DebugThreadID( std::this_thread::get_id() )
#endif // _DEBUG
{}

JavaNativeInterface::~JavaNativeInterface()
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
  m_DebugThreadID = std::thread::id();
#endif // _DEBUG

  if ( nullptr != m_pExportedEnvironment )
  {
    m_pExportedEnvironment->m_pInternal = nullptr;
  }

  delete m_pExportedVM;
  m_pExportedVM = nullptr;

  delete m_pExportedEnvironment;
  m_pExportedEnvironment = nullptr;

  m_Functions.clear();
}

void JavaNativeInterface::RegisterFunction( const JavaString &name, void *pFunction )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  m_Functions[ name ] = pFunction;
}

void JavaNativeInterface::ExecuteFunction( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pObject )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  ExecuteFunctionInternal( pName, pParameterTypes, pObject, e_IsFunctionStatic::No );
}

boost::intrusive_ptr<JavaString> JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( boost::intrusive_ptr<JavaString> pClassName, boost::intrusive_ptr<JavaString> pMethodName )
{
  return new JavaString( c_JavaNativeInterfaceInitialMethodString.Append( pClassName->Append( JVMX_T( "_" ) ).Append( *pMethodName ) ).ReplaceAll( JVMX_T( '/' ), JVMX_T( '_' ) ) );
}

bool JavaNativeInterface::LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  std::shared_ptr<JavaNativeInterfaceLibrary> pLib = OsFunctions::GetInstance().LoadOsLibrary( pLibraryName );

  if ( nullptr == pLib || nullptr == pLib->m_ModuleHandle )
  {
    return false;
  }

  std::shared_ptr<NativeLibraryContainer> pLibraries = GlobalCatalog::GetInstance().Get( "NativeLibraryContainer" );
  pLibraries->Add( pLib );

  // Need to Enumerate the methods in the library.

  return true;;
}

void JavaNativeInterface::ExecuteFunctionStdCall( void *pFunction, int *pIntParams, int paramCount, char16_t returnType, boost::intrusive_ptr<ObjectReference> pObjectOrClass )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  intptr_t env = reinterpret_cast<intptr_t>( GetEnvironment() );
  int index = 0;
  intptr_t retValInt = 0;
  intptr_t extraBits = 0;
  intptr_t objectOrClassAsJObject = reinterpret_cast<intptr_t>( pObjectOrClass->ToJObject() );

#ifdef _DEBUG
  intptr_t oldEsp = 0;
  __asm mov oldEsp, esp;
#endif // _DEBUG

  //for ( index = 0; index < paramCount; ++index )
  for ( index = paramCount - 1; index >= 0; --index )
  {
    //     if ( val.first )
    //     {
    //       double d = (*(double *)(val.second.get()));
    //       ASM_PUSH_FLOAT( d );
    //     }
    //     else
    //     {
    int value = pIntParams[ index ];
    ASM_PUSH_INT( value );
    //    }
  }

  GetVMState()->SetExecutingNative();
  GetVMState()->AddLocalReferenceFrame();

  //ASM_PUSH_INT( pObjectOrClass );
  ASM_PUSH_INT( objectOrClassAsJObject );
  ASM_PUSH_INT( env );

  ASM_CALL_FUNCTION( pFunction ); // Call the function

  ASM_GET_RETVAL( retValInt ); // Do this first. It can't hurt to take that value of EAX, even if we don't use it.
  ASM_GET_RETVAL_EXTRABITS( extraBits ); // Do this first. It can't hurt to take that value of EDX, even if we don't use it.

  GetVMState()->SetExecutingHosted();
  GetVMState()->DeleteLocalReferenceFrame();

  switch ( returnType )
  {
    case c_JavaTypeSpecifierVoid:
      break;

    case c_JavaTypeSpecifierInteger:
      {
        boost::intrusive_ptr< JavaInteger > pReturnValue = new JavaInteger( JavaInteger::FromHostInt32( retValInt ) );
        GetVMState()->PushOperand( pReturnValue );
      }
      break;

    case c_JavaTypeSpecifierBool:
      {
        boost::intrusive_ptr<JavaBool> pReturnValue = new JavaBool( JavaBool::FromBool( ( retValInt & 0xFF ) != 0 ) );
        GetVMState()->PushOperand( pReturnValue );
      }
      break;

    case c_JavaTypeSpecifierReference:
    case c_JavaTypeSpecifierArray:
      {
        //JVMX_ASSERT( nullptr != reinterpret_cast<IJavaVariableType *>(retValInt) && reinterpret_cast<IJavaVariableType *>(retValInt)->IsReferenceType() );

        boost::intrusive_ptr<IJavaVariableType> pReturnValue = nullptr;

        if ( 0 == retValInt )
        {
          pReturnValue = new ObjectReference( nullptr );
        }
        else
        {
          pReturnValue = new ObjectReference( reinterpret_cast<jobject>( retValInt ) );
        }

        GetVMState()->PushOperand( pReturnValue );
      }
      break;

    //     case c_JavaTypeSpecifierArray:
    //     {
    //       //JVMX_ASSERT( nullptr != reinterpret_cast<IJavaVariableType *>(retValInt) && reinterpret_cast<IJavaVariableType *>(retValInt)->IsReferenceType() );
    //
    //       boost::intrusive_ptr<IJavaVariableType> pReturnValue = nullptr;
    //
    //       if ( 0 == retValInt )
    //       {
    //         pReturnValue = new JavaNullReference;
    //       }
    //       else
    //       {
    //         pReturnValue = new ObjectReference( reinterpret_cast<jobject>(retValInt) );
    //       }
    //
    //       GetVMState()->PushOperand( pReturnValue );
    //     }
    //     break;

    case c_JavaTypeSpecifierLong:
      {
        int64_t value64Bits = extraBits;
        value64Bits <<= 32;

        value64Bits |= static_cast< uint32_t >( retValInt );

        boost::intrusive_ptr< JavaLong > pReturnValue = new JavaLong( JavaLong::FromHostInt64( value64Bits ) );
        GetVMState()->PushOperand( pReturnValue );
      }
      break;

    case c_JavaTypeSpecifierDouble:
      {
        int64_t value64Bits = extraBits;
        value64Bits <<= 32;

        value64Bits |= static_cast< uint32_t >( retValInt );

        boost::intrusive_ptr< JavaDouble > pReturnValue = new JavaDouble( JavaDouble::FromHostInt64( value64Bits ) );
        GetVMState()->PushOperand( pReturnValue );
      }
      break;

    default:
      throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (GetVMState()->HasUserCodeStarted())
  {
      GetVMState()->LogOperandStack();
  }
#endif // _DEBUG

#ifdef _DEBUG
  intptr_t newEsp = 0;
  __asm mov newEsp, esp;

  JVMX_ASSERT( oldEsp == newEsp );
#endif // _DEBUG

  // TODO: Do something with the return value
}

intptr_t JavaNativeInterface::ConvertReferencePointerToIntValue( boost::intrusive_ptr<IJavaVariableType> pValue )
{
  switch ( pValue->GetVariableType() )
  {
    case e_JavaVariableTypes::NullReference:
      return 0;
      break;

    case e_JavaVariableTypes::Object:
      return reinterpret_cast<intptr_t>( boost::dynamic_pointer_cast<ObjectReference>( pValue )->ToJObject() );
      break;

    case e_JavaVariableTypes::Array:
      return reinterpret_cast<intptr_t>( boost::dynamic_pointer_cast<ObjectReference>( pValue )->ToJObject() );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unexpected type." );
      break;
  }

  return reinterpret_cast<intptr_t>( pValue.get() );
}

void JavaNativeInterface::PushParametersStandardCall( int *pInteger, std::vector<boost::intrusive_ptr<JavaString>> parameterList, size_t sizeAsIntegers )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  size_t paramIndex = sizeAsIntegers - 1;

  for ( auto it = parameterList.rbegin(); it != parameterList.rend(); ++it )
    //for ( auto it = parameterList.begin( ); it != parameterList.end( ); ++it )
  {
    boost::intrusive_ptr<JavaString> pType = *it;

    switch ( pType->At( 0 ) )
    {
      case c_JavaTypeSpecifierByte:
        {
          boost::intrusive_ptr<JavaByte> pValue = boost::dynamic_pointer_cast<JavaByte>( GetVMState()->PopOperand() );
          jbyte value = pValue->ToHostInt8();

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %d", pType->ToUtf8String().c_str(), (int)value);
          }
#endif

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierChar:
        {
#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetVMState()->LogOperandStack();
          }
#endif
          boost::intrusive_ptr<IJavaVariableType> pStackValue = GetVMState()->PopOperand();

          if ( !pStackValue->IsIntegerCompatible() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Expected argument that could be converted to integer." );
          }

          boost::intrusive_ptr<JavaInteger> pIntegerValue = TypeParser::UpCastToInteger( pStackValue.get() );

          //boost::intrusive_ptr<JavaChar> pValue = boost::dynamic_pointer_cast<JavaChar>(pStackValue);
          jchar value = pIntegerValue->ToHostInt32() & USHRT_MAX;

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %u", pType->ToUtf8String().c_str(), static_cast<unsigned int>(value));
          }
#endif

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierInteger:
        {
          boost::intrusive_ptr< JavaInteger > pValue = boost::dynamic_pointer_cast<JavaInteger>( GetVMState()->PopOperand() );
          jint value = pValue->ToHostInt32();

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %d", pType->ToUtf8String().c_str(), static_cast<int>(value));
          }
#endif

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierShort:
        {
          boost::intrusive_ptr<JavaShort> pValue = boost::dynamic_pointer_cast<JavaShort>( GetVMState()->PopOperand() );
          jshort value = pValue->ToHostInt16();

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %d", pType->ToUtf8String().c_str(), static_cast<int>(value));
          }
#endif

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierBool:
        {
          boost::intrusive_ptr<IJavaVariableType> pStackValue = GetVMState()->PopOperand();
          boost::intrusive_ptr<JavaBool> pValue = boost::dynamic_pointer_cast<JavaBool>( pStackValue );
          if ( nullptr == pValue && e_JavaVariableTypes::Integer == pStackValue->GetVariableType() )
          {
            pValue = boost::dynamic_pointer_cast<JavaBool>( TypeParser::DownCastFromInteger( boost::dynamic_pointer_cast<JavaInteger>( pStackValue ), e_JavaVariableTypes::Bool ) );
          }

          jboolean value = pValue->ToBool() ? JNI_TRUE : JNI_FALSE;

          // GetLogger()->LogDebug( "Push Parameter %s, with value of %d", pType->ToByteArray(), (int)(value) );

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierFloat:
        {
          boost::intrusive_ptr<JavaFloat> pValue = boost::dynamic_pointer_cast<JavaFloat>( GetVMState()->PopOperand() );
          jfloat value = pValue->ToHostFloat();

          // GetLogger()->LogDebug( "Push Parameter %s, with value of %f", pType->ToByteArray(), (double)(value) );

          //JVMX_ASSERT( false );
          ADD_INT_TO_STACK( pInteger, paramIndex, *( reinterpret_cast<int32_t *>( &value ) ) );
          //ASM_PUSH_FLOAT( value );
        }
        break;

      case c_JavaTypeSpecifierDouble:
        {
          boost::intrusive_ptr<JavaDouble> pValue = boost::dynamic_pointer_cast<JavaDouble>( GetVMState()->PopOperand() );
          jdouble value = pValue->ToHostDouble();

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %f", pType->ToUtf8String().c_str(), value);
          }
#endif

          int64_t valueAsInteger = *( reinterpret_cast<int64_t *>( &value ) );
          ADD_INT_TO_STACK( pInteger, paramIndex, static_cast<uint32_t>( valueAsInteger >> 32 ) );

          -- paramIndex;
          ADD_INT_TO_STACK( pInteger, paramIndex, static_cast<uint32_t>( valueAsInteger & 0xFFFFFFFF ) );
        }
        break;

      case c_JavaTypeSpecifierLong:
        {
          boost::intrusive_ptr<JavaLong> pValue = boost::dynamic_pointer_cast<JavaLong>( GetVMState()->PopOperand() );
          jlong value = pValue->ToHostInt64();

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %lld", pType->ToUtf8String().c_str(), value);
          }
#endif

          int32_t firstWord = value & 0xFFFFFFFF;
          int32_t secondWord = value >> 32;

          ADD_INT_TO_STACK( pInteger, paramIndex, secondWord );

          -- paramIndex;
          ADD_INT_TO_STACK( pInteger, paramIndex, firstWord );
        }
        break;

      case c_JavaTypeSpecifierReference:
        {
          boost::intrusive_ptr<IJavaVariableType> pValue = GetVMState()->PopOperand();

          JVMX_ASSERT( pValue->GetVariableType() == e_JavaVariableTypes::Object || pValue->GetVariableType() == e_JavaVariableTypes::Array || pValue->GetVariableType() == e_JavaVariableTypes::ClassReference || pValue->GetVariableType() == e_JavaVariableTypes::ReturnAddress || pValue->GetVariableType() == e_JavaVariableTypes::NullReference );

          intptr_t value = ConvertReferencePointerToIntValue( pValue );

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %" PRIdPTR, pType->ToUtf8String().c_str(), value);
          }
#endif

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      case c_JavaTypeSpecifierArray:
        {
          boost::intrusive_ptr<IJavaVariableType> pValue = GetVMState()->PopOperand();

          JVMX_ASSERT( pValue->GetVariableType() == e_JavaVariableTypes::Array || pValue->GetVariableType() == e_JavaVariableTypes::NullReference );
          intptr_t value = 0;

          if ( pValue->GetVariableType() == e_JavaVariableTypes::Array )
          {
            value = ConvertReferencePointerToIntValue( pValue );
          }

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
          if (GetVMState()->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Push Parameter %s, with value of %" PRIdPTR, pType->ToUtf8String().c_str(), value);
          }
#endif
          //GetLogger()->LogDebug( "Push Parameter array." );

          ADD_INT_TO_STACK( pInteger, paramIndex, value );
        }
        break;

      default:
        throw InvalidArgumentException( __FUNCTION__ " - Unknown parameter type received." );
        break;
    }

    --paramIndex;
  }
}

JNIEnv *JavaNativeInterface::GetEnvironment()
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  if ( nullptr == m_pExportedEnvironment )
  {
    m_pExportedEnvironment = new JNIEnvExported;

    m_pExportedEnvironment->pReserved = nullptr;
    m_pExportedEnvironment->pReservedCOM1 = nullptr;
    m_pExportedEnvironment->pReservedCOM2 = nullptr;
    m_pExportedEnvironment->pReservedCOM3 = nullptr;

    m_pExportedEnvironment->GetVersion = JNIEnvInternal::GetVersion;
    m_pExportedEnvironment->DefineClass = JNIEnvInternal::DefineClass;
    m_pExportedEnvironment->FindClass = JNIEnvInternal::FindClass;
    m_pExportedEnvironment->FromReflectedMethod = JNIEnvInternal::FromReflectedMethod;
    m_pExportedEnvironment->FromReflectedField = JNIEnvInternal::FromReflectedField;
    m_pExportedEnvironment->ToReflectedMethod = JNIEnvInternal::ToReflectedMethod;
    m_pExportedEnvironment->GetSuperclass = JNIEnvInternal::GetSuperclass;
    m_pExportedEnvironment->IsAssignableFrom = JNIEnvInternal::IsAssignableFrom;
    m_pExportedEnvironment->ToReflectedField = JNIEnvInternal::ToReflectedField;
    m_pExportedEnvironment->Throw = JNIEnvInternal::Throw;
    m_pExportedEnvironment->ThrowNew = JNIEnvInternal::ThrowNew;
    m_pExportedEnvironment->ExceptionOccurred = JNIEnvInternal::ExceptionOccurred;
    m_pExportedEnvironment->ExceptionDescribe = JNIEnvInternal::ExceptionDescribe;
    m_pExportedEnvironment->ExceptionClear = JNIEnvInternal::ExceptionClear;
    m_pExportedEnvironment->FatalError = JNIEnvInternal::FatalError;
    m_pExportedEnvironment->PushLocalFrame = JNIEnvInternal::PushLocalFrame;
    m_pExportedEnvironment->PopLocalFrame = JNIEnvInternal::PopLocalFrame;
    m_pExportedEnvironment->NewGlobalRef = JNIEnvInternal::NewGlobalRef;
    m_pExportedEnvironment->DeleteGlobalRef = JNIEnvInternal::DeleteGlobalRef;
    m_pExportedEnvironment->DeleteLocalRef = JNIEnvInternal::DeleteLocalRef;
    m_pExportedEnvironment->IsSameObject = JNIEnvInternal::IsSameObject;
    m_pExportedEnvironment->NewLocalRef = JNIEnvInternal::NewLocalRef;
    m_pExportedEnvironment->EnsureLocalCapacity = JNIEnvInternal::EnsureLocalCapacity;
    m_pExportedEnvironment->AllocObject = JNIEnvInternal::AllocObject;
    m_pExportedEnvironment->NewObject = JNIEnvInternal::NewObject;
    m_pExportedEnvironment->NewObjectV = JNIEnvInternal::NewObjectV;
    m_pExportedEnvironment->NewObjectA = JNIEnvInternal::NewObjectA;
    m_pExportedEnvironment->GetObjectClass = JNIEnvInternal::GetObjectClass;
    m_pExportedEnvironment->IsInstanceOf = JNIEnvInternal::IsInstanceOf;
    m_pExportedEnvironment->GetMethodID = JNIEnvInternal::GetMethodID;
    m_pExportedEnvironment->CallObjectMethod = JNIEnvInternal::CallObjectMethod;
    m_pExportedEnvironment->CallObjectMethodV = JNIEnvInternal::CallObjectMethodV;
    m_pExportedEnvironment->CallObjectMethodA = JNIEnvInternal::CallObjectMethodA;
    m_pExportedEnvironment->CallBooleanMethod = JNIEnvInternal::CallBooleanMethod;
    m_pExportedEnvironment->CallBooleanMethodV = JNIEnvInternal::CallBooleanMethodV;
    m_pExportedEnvironment->CallBooleanMethodA = JNIEnvInternal::CallBooleanMethodA;
    m_pExportedEnvironment->CallByteMethod = JNIEnvInternal::CallByteMethod;
    m_pExportedEnvironment->CallByteMethodV = JNIEnvInternal::CallByteMethodV;
    m_pExportedEnvironment->CallByteMethodA = JNIEnvInternal::CallByteMethodA;
    m_pExportedEnvironment->CallCharMethod = JNIEnvInternal::CallCharMethod;
    m_pExportedEnvironment->CallCharMethodA = JNIEnvInternal::CallCharMethodA;
    m_pExportedEnvironment->CallCharMethodA = JNIEnvInternal::CallCharMethodA;
    m_pExportedEnvironment->CallShortMethod = JNIEnvInternal::CallShortMethod;
    m_pExportedEnvironment->CallShortMethodV = JNIEnvInternal::CallShortMethodV;
    m_pExportedEnvironment->CallShortMethodA = JNIEnvInternal::CallShortMethodA;
    m_pExportedEnvironment->CallIntMethod = JNIEnvInternal::CallIntMethod;
    m_pExportedEnvironment->CallIntMethodV = JNIEnvInternal::CallIntMethodV;
    m_pExportedEnvironment->CallIntMethodA = JNIEnvInternal::CallIntMethodA;
    m_pExportedEnvironment->CallLongMethod = JNIEnvInternal::CallLongMethod;
    m_pExportedEnvironment->CallLongMethodV = JNIEnvInternal::CallLongMethodV;
    m_pExportedEnvironment->CallLongMethodA = JNIEnvInternal::CallLongMethodA;
    m_pExportedEnvironment->CallFloatMethod = JNIEnvInternal::CallFloatMethod;
    m_pExportedEnvironment->CallFloatMethodV = JNIEnvInternal::CallFloatMethodV;
    m_pExportedEnvironment->CallFloatMethodA = JNIEnvInternal::CallFloatMethodA;
    m_pExportedEnvironment->CallDoubleMethod = JNIEnvInternal::CallDoubleMethod;
    m_pExportedEnvironment->CallDoubleMethodV = JNIEnvInternal::CallDoubleMethodV;
    m_pExportedEnvironment->CallDoubleMethodA = JNIEnvInternal::CallDoubleMethodA;
    m_pExportedEnvironment->CallVoidMethod = JNIEnvInternal::CallVoidMethod;
    m_pExportedEnvironment->CallVoidMethodV = JNIEnvInternal::CallVoidMethodV;
    m_pExportedEnvironment->CallVoidMethodA = JNIEnvInternal::CallVoidMethodA;
    m_pExportedEnvironment->CallNonvirtualObjectMethod = JNIEnvInternal::CallNonvirtualObjectMethod;
    m_pExportedEnvironment->CallNonvirtualObjectMethodV = JNIEnvInternal::CallNonvirtualObjectMethodV;
    m_pExportedEnvironment->CallNonvirtualObjectMethodA = JNIEnvInternal::CallNonvirtualObjectMethodA;
    m_pExportedEnvironment->CallNonvirtualBooleanMethod = JNIEnvInternal::CallNonvirtualBooleanMethod;
    m_pExportedEnvironment->CallNonvirtualBooleanMethodV = JNIEnvInternal::CallNonvirtualBooleanMethodV;
    m_pExportedEnvironment->CallNonvirtualBooleanMethodA = JNIEnvInternal::CallNonvirtualBooleanMethodA;
    m_pExportedEnvironment->CallNonvirtualByteMethod = JNIEnvInternal::CallNonvirtualByteMethod;
    m_pExportedEnvironment->CallNonvirtualByteMethodV = JNIEnvInternal::CallNonvirtualByteMethodV;
    m_pExportedEnvironment->CallNonvirtualByteMethodA = JNIEnvInternal::CallNonvirtualByteMethodA;
    m_pExportedEnvironment->CallNonvirtualCharMethod = JNIEnvInternal::CallNonvirtualCharMethod;
    m_pExportedEnvironment->CallNonvirtualCharMethodV = JNIEnvInternal::CallNonvirtualCharMethodV;
    m_pExportedEnvironment->CallNonvirtualCharMethodA = JNIEnvInternal::CallNonvirtualCharMethodA;
    m_pExportedEnvironment->CallNonvirtualShortMethod = JNIEnvInternal::CallNonvirtualShortMethod;
    m_pExportedEnvironment->CallNonvirtualShortMethodV = JNIEnvInternal::CallNonvirtualShortMethodV;
    m_pExportedEnvironment->CallNonvirtualShortMethodA = JNIEnvInternal::CallNonvirtualShortMethodA;
    m_pExportedEnvironment->CallNonvirtualIntMethod = JNIEnvInternal::CallNonvirtualIntMethod;
    m_pExportedEnvironment->CallNonvirtualIntMethodV = JNIEnvInternal::CallNonvirtualIntMethodV;
    m_pExportedEnvironment->CallNonvirtualIntMethodA = JNIEnvInternal::CallNonvirtualIntMethodA;
    m_pExportedEnvironment->CallNonvirtualLongMethod = JNIEnvInternal::CallNonvirtualLongMethod;
    m_pExportedEnvironment->CallNonvirtualLongMethodV = JNIEnvInternal::CallNonvirtualLongMethodV;
    m_pExportedEnvironment->CallNonvirtualLongMethodA = JNIEnvInternal::CallNonvirtualLongMethodA;
    m_pExportedEnvironment->CallNonvirtualFloatMethod = JNIEnvInternal::CallNonvirtualFloatMethod;
    m_pExportedEnvironment->CallNonvirtualFloatMethodV = JNIEnvInternal::CallNonvirtualFloatMethodV;
    m_pExportedEnvironment->CallNonvirtualFloatMethodA = JNIEnvInternal::CallNonvirtualFloatMethodA;
    m_pExportedEnvironment->CallNonvirtualDoubleMethod = JNIEnvInternal::CallNonvirtualDoubleMethod;
    m_pExportedEnvironment->CallNonvirtualDoubleMethodV = JNIEnvInternal::CallNonvirtualDoubleMethodV;
    m_pExportedEnvironment->CallNonvirtualDoubleMethodA = JNIEnvInternal::CallNonvirtualDoubleMethodA;
    m_pExportedEnvironment->CallNonvirtualVoidMethod = JNIEnvInternal::CallNonvirtualVoidMethod;
    m_pExportedEnvironment->CallNonvirtualVoidMethodV = JNIEnvInternal::CallNonvirtualVoidMethodV;
    m_pExportedEnvironment->CallNonvirtualVoidMethodA = JNIEnvInternal::CallNonvirtualVoidMethodA;
    m_pExportedEnvironment->GetFieldID = JNIEnvInternal::GetFieldID;
    m_pExportedEnvironment->GetObjectField = JNIEnvInternal::GetObjectField;
    m_pExportedEnvironment->GetBooleanField = JNIEnvInternal::GetBooleanField;
    m_pExportedEnvironment->GetByteField = JNIEnvInternal::GetByteField;
    m_pExportedEnvironment->GetCharField = JNIEnvInternal::GetCharField;
    m_pExportedEnvironment->GetShortField = JNIEnvInternal::GetShortField;
    m_pExportedEnvironment->GetIntField = JNIEnvInternal::GetIntField;
    m_pExportedEnvironment->GetLongField = JNIEnvInternal::GetLongField;
    m_pExportedEnvironment->GetFloatField = JNIEnvInternal::GetFloatField;
    m_pExportedEnvironment->GetDoubleField = JNIEnvInternal::GetDoubleField;
    m_pExportedEnvironment->SetObjectField = JNIEnvInternal::SetObjectField;
    m_pExportedEnvironment->SetBooleanField = JNIEnvInternal::SetBooleanField;
    m_pExportedEnvironment->SetByteField = JNIEnvInternal::SetByteField;
    m_pExportedEnvironment->SetCharField = JNIEnvInternal::SetCharField;
    m_pExportedEnvironment->SetShortField = JNIEnvInternal::SetShortField;
    m_pExportedEnvironment->SetIntField = JNIEnvInternal::SetIntField;
    m_pExportedEnvironment->SetLongField = JNIEnvInternal::SetLongField;
    m_pExportedEnvironment->SetFloatField = JNIEnvInternal::SetFloatField;
    m_pExportedEnvironment->SetDoubleField = JNIEnvInternal::SetDoubleField;
    m_pExportedEnvironment->GetStaticMethodID = JNIEnvInternal::GetStaticMethodID;
    m_pExportedEnvironment->CallStaticObjectMethod = JNIEnvInternal::CallStaticObjectMethod;
    m_pExportedEnvironment->CallStaticObjectMethodV = JNIEnvInternal::CallStaticObjectMethodV;
    m_pExportedEnvironment->CallStaticObjectMethodA = JNIEnvInternal::CallStaticObjectMethodA;
    m_pExportedEnvironment->CallStaticBooleanMethod = JNIEnvInternal::CallStaticBooleanMethod;
    m_pExportedEnvironment->CallStaticBooleanMethodV = JNIEnvInternal::CallStaticBooleanMethodV;
    m_pExportedEnvironment->CallStaticBooleanMethodA = JNIEnvInternal::CallStaticBooleanMethodA;
    m_pExportedEnvironment->CallStaticByteMethod = JNIEnvInternal::CallStaticByteMethod;
    m_pExportedEnvironment->CallStaticByteMethodV = JNIEnvInternal::CallStaticByteMethodV;
    m_pExportedEnvironment->CallStaticByteMethodA = JNIEnvInternal::CallStaticByteMethodA;
    m_pExportedEnvironment->CallStaticCharMethod = JNIEnvInternal::CallStaticCharMethod;
    m_pExportedEnvironment->CallStaticCharMethodV = JNIEnvInternal::CallStaticCharMethodV;
    m_pExportedEnvironment->CallStaticCharMethodA = JNIEnvInternal::CallStaticCharMethodA;
    m_pExportedEnvironment->CallStaticShortMethod = JNIEnvInternal::CallStaticShortMethod;
    m_pExportedEnvironment->CallStaticShortMethodV = JNIEnvInternal::CallStaticShortMethodV;
    m_pExportedEnvironment->CallStaticShortMethodA = JNIEnvInternal::CallStaticShortMethodA;
    m_pExportedEnvironment->CallStaticIntMethod = JNIEnvInternal::CallStaticIntMethod;
    m_pExportedEnvironment->CallStaticIntMethodV = JNIEnvInternal::CallStaticIntMethodV;
    m_pExportedEnvironment->CallStaticIntMethodA = JNIEnvInternal::CallStaticIntMethodA;
    m_pExportedEnvironment->CallStaticLongMethod = JNIEnvInternal::CallStaticLongMethod;
    m_pExportedEnvironment->CallStaticLongMethodV = JNIEnvInternal::CallStaticLongMethodV;
    m_pExportedEnvironment->CallStaticLongMethodA = JNIEnvInternal::CallStaticLongMethodA;
    m_pExportedEnvironment->CallStaticFloatMethod = JNIEnvInternal::CallStaticFloatMethod;
    m_pExportedEnvironment->CallStaticFloatMethodV = JNIEnvInternal::CallStaticFloatMethodV;
    m_pExportedEnvironment->CallStaticFloatMethodA = JNIEnvInternal::CallStaticFloatMethodA;
    m_pExportedEnvironment->CallStaticDoubleMethod = JNIEnvInternal::CallStaticDoubleMethod;
    m_pExportedEnvironment->CallStaticDoubleMethodV = JNIEnvInternal::CallStaticDoubleMethodV;
    m_pExportedEnvironment->CallStaticDoubleMethodA = JNIEnvInternal::CallStaticDoubleMethodA;
    m_pExportedEnvironment->CallStaticVoidMethod = JNIEnvInternal::CallStaticVoidMethod;
    m_pExportedEnvironment->CallStaticVoidMethodV = JNIEnvInternal::CallStaticVoidMethodV;
    m_pExportedEnvironment->CallStaticVoidMethodA = JNIEnvInternal::CallStaticVoidMethodA;
    m_pExportedEnvironment->GetStaticFieldID = JNIEnvInternal::GetStaticFieldID;
    m_pExportedEnvironment->GetStaticObjectField = JNIEnvInternal::GetStaticObjectField;
    m_pExportedEnvironment->GetStaticBooleanField = JNIEnvInternal::GetStaticBooleanField;
    m_pExportedEnvironment->GetStaticByteField = JNIEnvInternal::GetStaticByteField;
    m_pExportedEnvironment->GetStaticCharField = JNIEnvInternal::GetStaticCharField;
    m_pExportedEnvironment->GetStaticShortField = JNIEnvInternal::GetStaticShortField;
    m_pExportedEnvironment->GetStaticIntField = JNIEnvInternal::GetStaticIntField;
    m_pExportedEnvironment->GetStaticLongField = JNIEnvInternal::GetStaticLongField;
    m_pExportedEnvironment->GetStaticFloatField = JNIEnvInternal::GetStaticFloatField;
    m_pExportedEnvironment->GetStaticDoubleField = JNIEnvInternal::GetStaticDoubleField;
    m_pExportedEnvironment->SetStaticObjectField = JNIEnvInternal::SetStaticObjectField;
    m_pExportedEnvironment->SetStaticBooleanField = JNIEnvInternal::SetStaticBooleanField;
    m_pExportedEnvironment->SetStaticByteField = JNIEnvInternal::SetStaticByteField;
    m_pExportedEnvironment->SetStaticCharField = JNIEnvInternal::SetStaticCharField;
    m_pExportedEnvironment->SetStaticShortField = JNIEnvInternal::SetStaticShortField;
    m_pExportedEnvironment->SetStaticIntField = JNIEnvInternal::SetStaticIntField;
    m_pExportedEnvironment->SetStaticLongField = JNIEnvInternal::SetStaticLongField;
    m_pExportedEnvironment->SetStaticFloatField = JNIEnvInternal::SetStaticFloatField;
    m_pExportedEnvironment->SetStaticDoubleField = JNIEnvInternal::SetStaticDoubleField;
    m_pExportedEnvironment->NewString = JNIEnvInternal::NewString;
    m_pExportedEnvironment->GetStringLength = JNIEnvInternal::GetStringLength;
    m_pExportedEnvironment->GetStringChars = JNIEnvInternal::GetStringChars;
    m_pExportedEnvironment->ReleaseStringChars = JNIEnvInternal::ReleaseStringChars;
    m_pExportedEnvironment->NewStringUTF = JNIEnvInternal::NewStringUTF;
    m_pExportedEnvironment->GetStringUTFLength = JNIEnvInternal::GetStringUTFLength;
    m_pExportedEnvironment->GetStringUTFChars = JNIEnvInternal::GetStringUTFChars;
    m_pExportedEnvironment->ReleaseStringUTFChars = JNIEnvInternal::ReleaseStringUTFChars;
    m_pExportedEnvironment->GetArrayLength = JNIEnvInternal::GetArrayLength;
    m_pExportedEnvironment->NewObjectArray = JNIEnvInternal::NewObjectArray;
    m_pExportedEnvironment->GetObjectArrayElement = JNIEnvInternal::GetObjectArrayElement;
    m_pExportedEnvironment->SetObjectArrayElement = JNIEnvInternal::SetObjectArrayElement;
    m_pExportedEnvironment->NewBooleanArray = JNIEnvInternal::NewBooleanArray;
    m_pExportedEnvironment->NewByteArray = JNIEnvInternal::NewByteArray;
    m_pExportedEnvironment->NewCharArray = JNIEnvInternal::NewCharArray;
    m_pExportedEnvironment->NewShortArray = JNIEnvInternal::NewShortArray;
    m_pExportedEnvironment->NewIntArray = JNIEnvInternal::NewIntArray;
    m_pExportedEnvironment->NewLongArray = JNIEnvInternal::NewLongArray;
    m_pExportedEnvironment->NewFloatArray = JNIEnvInternal::NewFloatArray;
    m_pExportedEnvironment->NewDoubleArray = JNIEnvInternal::NewDoubleArray;
    m_pExportedEnvironment->GetBooleanArrayElements = JNIEnvInternal::GetBooleanArrayElements;
    m_pExportedEnvironment->GetByteArrayElements = JNIEnvInternal::GetByteArrayElements;
    m_pExportedEnvironment->GetCharArrayElements = JNIEnvInternal::GetCharArrayElements;
    m_pExportedEnvironment->GetShortArrayElements = JNIEnvInternal::GetShortArrayElements;
    m_pExportedEnvironment->GetIntArrayElements = JNIEnvInternal::GetIntArrayElements;
    m_pExportedEnvironment->GetLongArrayElements = JNIEnvInternal::GetLongArrayElements;
    m_pExportedEnvironment->GetFloatArrayElements = JNIEnvInternal::GetFloatArrayElements;
    m_pExportedEnvironment->GetDoubleArrayElements = JNIEnvInternal::GetDoubleArrayElements;
    m_pExportedEnvironment->ReleaseBooleanArrayElements = JNIEnvInternal::ReleaseBooleanArrayElements;
    m_pExportedEnvironment->ReleaseByteArrayElements = JNIEnvInternal::ReleaseByteArrayElements;
    m_pExportedEnvironment->ReleaseCharArrayElements = JNIEnvInternal::ReleaseCharArrayElements;
    m_pExportedEnvironment->ReleaseShortArrayElements = JNIEnvInternal::ReleaseShortArrayElements;
    m_pExportedEnvironment->ReleaseIntArrayElements = JNIEnvInternal::ReleaseIntArrayElements;
    m_pExportedEnvironment->ReleaseLongArrayElements = JNIEnvInternal::ReleaseLongArrayElements;
    m_pExportedEnvironment->ReleaseFloatArrayElements = JNIEnvInternal::ReleaseFloatArrayElements;
    m_pExportedEnvironment->ReleaseDoubleArrayElements = JNIEnvInternal::ReleaseDoubleArrayElements;
    m_pExportedEnvironment->GetBooleanArrayRegion = JNIEnvInternal::GetBooleanArrayRegion;
    m_pExportedEnvironment->GetByteArrayRegion = JNIEnvInternal::GetByteArrayRegion;
    m_pExportedEnvironment->GetCharArrayRegion = JNIEnvInternal::GetCharArrayRegion;
    m_pExportedEnvironment->GetShortArrayRegion = JNIEnvInternal::GetShortArrayRegion;
    m_pExportedEnvironment->GetIntArrayRegion = JNIEnvInternal::GetIntArrayRegion;
    m_pExportedEnvironment->GetLongArrayRegion = JNIEnvInternal::GetLongArrayRegion;
    m_pExportedEnvironment->GetFloatArrayRegion = JNIEnvInternal::GetFloatArrayRegion;
    m_pExportedEnvironment->GetDoubleArrayRegion = JNIEnvInternal::GetDoubleArrayRegion;
    m_pExportedEnvironment->SetBooleanArrayRegion = JNIEnvInternal::SetBooleanArrayRegion;
    m_pExportedEnvironment->SetByteArrayRegion = JNIEnvInternal::SetByteArrayRegion;
    m_pExportedEnvironment->SetCharArrayRegion = JNIEnvInternal::SetCharArrayRegion;
    m_pExportedEnvironment->SetShortArrayRegion = JNIEnvInternal::SetShortArrayRegion;
    m_pExportedEnvironment->SetIntArrayRegion = JNIEnvInternal::SetIntArrayRegion;
    m_pExportedEnvironment->SetLongArrayRegion = JNIEnvInternal::SetLongArrayRegion;
    m_pExportedEnvironment->SetFloatArrayRegion = JNIEnvInternal::SetFloatArrayRegion;
    m_pExportedEnvironment->SetDoubleArrayRegion = JNIEnvInternal::SetDoubleArrayRegion;
    m_pExportedEnvironment->RegisterNatives = JNIEnvInternal::RegisterNatives;
    m_pExportedEnvironment->UnregisterNatives = JNIEnvInternal::UnregisterNatives;
    m_pExportedEnvironment->MonitorEnter = JNIEnvInternal::MonitorEnter;
    m_pExportedEnvironment->MonitorExit = JNIEnvInternal::MonitorExit;
    m_pExportedEnvironment->GetJavaVM = JNIEnvInternal::GetJavaVM;
    m_pExportedEnvironment->GetStringRegion = JNIEnvInternal::GetStringRegion;
    m_pExportedEnvironment->GetStringUTFRegion = JNIEnvInternal::GetStringUTFRegion;
    m_pExportedEnvironment->GetPrimitiveArrayCritical = JNIEnvInternal::GetPrimitiveArrayCritical;
    m_pExportedEnvironment->ReleasePrimitiveArrayCritical = JNIEnvInternal::ReleasePrimitiveArrayCritical;
    m_pExportedEnvironment->GetStringCritical = JNIEnvInternal::GetStringCritical;
    m_pExportedEnvironment->ReleaseStringCritical = JNIEnvInternal::ReleaseStringCritical;
    m_pExportedEnvironment->NewWeakGlobalRef = JNIEnvInternal::NewWeakGlobalRef;
    m_pExportedEnvironment->DeleteWeakGlobalRef = JNIEnvInternal::DeleteWeakGlobalRef;
    m_pExportedEnvironment->ExceptionCheck = JNIEnvInternal::ExceptionCheck;

    // Special JVMX Methods
    m_pExportedEnvironment->JVMX_arraycopy = JNIEnvInternal::JVMX_arraycopy;

    m_pExportedEnvironment->m_pInternal = static_cast<const std::shared_ptr<IVirtualMachineState> &>( GetVMState() ).get();

    m_pExportedVM = new JavaVMExported();
    m_pExportedVM->pReservedCOM1 = nullptr;
    m_pExportedVM->pReservedCOM2 = nullptr;
    m_pExportedVM->pReservedCOM3 = nullptr;
    m_pExportedVM->GetEnv = JavaVMInternal::GetEnv;
    m_pExportedVM->AttachCurrentThread = JavaVMInternal::AttachCurrentThread;
    m_pExportedVM->DetachCurrentThread = JavaVMInternal::DetachCurrentThread;
    m_pExportedVM->DestroyJavaVM = JavaVMInternal::DestroyJavaVM;

    m_pExportedVM->m_pEnv = m_pExportedEnvironment;
  }

  return m_pExportedEnvironment;
}

JavaVM *JavaNativeInterface::GetVM()
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  JVMX_ASSERT( nullptr != m_pExportedVM );
  return reinterpret_cast< JavaVM * >( m_pExportedVM );
}

void JavaNativeInterface::ExecuteFunctionInternal( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pObjectOrClass, e_IsFunctionStatic isStatic )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  if ( nullptr == pObjectOrClass )
  {
    throw InvalidArgumentException( __FUNCTION__ " - We need to the object or the class to operate on." );
  }

  void *pFoundFunction = nullptr;
  auto pos = m_Functions.find( *pName );
  if ( pos == m_Functions.end() )
  {
    std::shared_ptr<NativeLibraryContainer> pLibraries = GlobalCatalog::GetInstance().Get( "NativeLibraryContainer" );
    pFoundFunction = pLibraries->FindFunction( reinterpret_cast<const JVMX_ANSI_CHAR_TYPE *>( pName->ToUtf8String().c_str() ) );
  }
  else
  {
    pFoundFunction = m_Functions.at( *pName );
  }

  if ( nullptr == pFoundFunction )
  {
    GetLogger()->LogError( "Native function (%s) not found.", pName->ToUtf8String().c_str() );

    throw InvalidArgumentException( __FUNCTION__ " - Native function not found." );
  }

  JniMethodImplementation pFinalFunction = static_cast<JniMethodImplementation>( pFoundFunction );

  TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *pParameterTypes );
  size_t paramCountAsIntegers = parsedType.GetParameterCountAsIntegers();
  std::unique_ptr<int> pParamArray( new int[ paramCountAsIntegers ] );

  PushParametersStandardCall( pParamArray.get(), parsedType.parameters, paramCountAsIntegers );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (GetVMState()->HasUserCodeStarted())
  {
      GetVMState()->LogOperandStack();
  }
#endif // _DEBUG

  ExecuteFunctionStdCall( pFinalFunction, pParamArray.get(), paramCountAsIntegers, parsedType.returnType.At( 0 ), pObjectOrClass );
}

void JavaNativeInterface::ExecuteFunctionStatic( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pJavaLangClass )
{
  ExecuteFunctionInternal( pName, pParameterTypes, pJavaLangClass, e_IsFunctionStatic::Yes );
}

std::shared_ptr<ILogger> JavaNativeInterface::GetLogger()
{
  return GlobalCatalog::GetInstance().Get( "Logger" );
}

void JavaNativeInterface::SetVMState( std::shared_ptr<IVirtualMachineState> pVMState )
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  m_pVMState = pVMState;
}

const std::shared_ptr<IVirtualMachineState> &JavaNativeInterface::GetVMState()
{
#ifdef _DEBUG
  JVMX_ASSERT( m_DebugThreadID == std::this_thread::get_id() );
#endif // _DEBUG

  if ( nullptr == m_pVMState )
  {
    throw InvalidStateException( __FUNCTION__ " - Java Native Interface was not initialized with the VM state object." );
  }

  return m_pVMState;
}