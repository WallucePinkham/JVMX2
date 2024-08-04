
#include <cstdarg>

#include "FileDoesNotExistException.h"
#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "NotImplementedException.h"
#include "NullPointerException.h"
#include "IndexOutOfBoundsException.h"
#include "JavaNativeInterface.h"
#include "GlobalCatalog.h"

#include "TypeParser.h"

#include "HelperTypes.h"
#include "ObjectReference.h"

#include "IVirtualMachineState.h"
#include "IClassLibrary.h"

#include "jni_internal.h"
#include "JavaExceptionConstants.h"


extern const JavaString c_ClassInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodType;
extern const JavaString c_SyntheticField_ClassName;

const JavaString c_JavaLangReflectField_ClassName = JavaString::FromCString( u"java/lang/reflect/Field" );
const JavaString c_JavaLangReflectVMField_ClassName = JavaString::FromCString( u"java/lang/reflect/VMField" );

extern "C"
{
  JNIEXPORT jdoubleArray JNICALL JNIEnvInternal::NewDoubleArray( JNIEnv *pEnv, jsize length )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Double, length );

    return reinterpret_cast<jdoubleArray>( ConvertArrayPointerToJArray( pVirtualMachineState, pArray ) );
  }

  JNIEXPORT jfloatArray JNICALL JNIEnvInternal::NewFloatArray( JNIEnv *pEnv, jsize length )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Float, length );

    return reinterpret_cast<jfloatArray>( ConvertArrayPointerToJArray( pVirtualMachineState, pArray ) );
  }

  JNIEXPORT jlongArray JNICALL JNIEnvInternal::NewLongArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jintArray JNICALL JNIEnvInternal::NewIntArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshortArray JNICALL JNIEnvInternal::NewShortArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jcharArray JNICALL JNIEnvInternal::NewCharArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyteArray JNICALL JNIEnvInternal::NewByteArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbooleanArray JNICALL JNIEnvInternal::NewBooleanArray( JNIEnv *pEnv, jsize length )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetObjectArrayElement( JNIEnv *pEnv, jobjectArray array, jsize index, jobject value )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pArray = ConvertJArrayToArrayPointer( array );
    pArray->GetContainedArray()->SetAt( index, ConvertJObjectToObjectPointer( value ).get() );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::GetObjectArrayElement( JNIEnv *pEnv, jobjectArray array, jsize index )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pArray = ConvertJArrayToArrayPointer( array );

    IJavaVariableType *pTempReference = pArray->GetContainedArray()->At( index );
    boost::intrusive_ptr<ObjectReference> ref = new ObjectReference( *dynamic_cast<const ObjectReference *>( pTempReference ) );

    return  ConvertObjectPointerToJObject( pVirtualMachineState, ref );
  }

  JNIEXPORT jobjectArray JNICALL JNIEnvInternal::NewObjectArray( JNIEnv *pEnv, jsize length, jclass elementType, jobject initialElement )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    //boost::intrusive_ptr<ObjectReference> pArray = new JavaArray( e_JavaArrayTypes::Reference, length );
    boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, length );

    if ( nullptr != initialElement )
    {
      for ( size_t i = 0; i < pArray->GetContainedArray()->GetNumberOfElements(); ++i )
      {
        pArray->GetContainedArray()->SetAt( i, ConvertJObjectToObjectPointer( initialElement ).get() );
      }
    }

    return reinterpret_cast<jobjectArray>( ConvertArrayPointerToJArray( pVirtualMachineState, pArray ) );
  }

  JNIEXPORT jsize JNICALL JNIEnvInternal::GetArrayLength( JNIEnv *pEnv, jarray array )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    size_t elementsCount = ConvertJArrayToArrayPointer( array )->GetContainedArray()->GetNumberOfElements();
    return ( jsize )elementsCount;
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseStringUTFChars( JNIEnv *pEnv, jstring string, const char *utf )
  {
    delete[] utf;
  }

  JNIEXPORT const jbyte *JNICALL JNIEnvInternal::GetStringUTFChars( JNIEnv *pEnv, jstring string, jboolean *isCopy )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pString = ConvertJObjectToObjectPointer( string );
    JavaString stringValue = HelperTypes::ExtractValueFromStringObject( pString );
    std::string utf8String = stringValue.ToUtf8String();

    jbyte *pBuffer = nullptr;
    try
    {
      pBuffer = new jbyte[ utf8String.length() + 1 ];
    }
    catch ( std::bad_alloc & )
    {
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaOutOfMemoryException ), "Out of memory allocating buffer." );
      return nullptr;
    }

    if ( nullptr == pBuffer )
    {
      // Should never reach this.
      throw InvalidStateException( __FUNCTION__ " - Could not allocate buffer." );
    }

    memcpy( pBuffer, utf8String.c_str(), utf8String.length() );
    pBuffer[ utf8String.length() ] = '\0';

    if ( nullptr != isCopy )
    {
      *isCopy = JNI_TRUE;
    }

    return pBuffer;
  }

  JNIEXPORT jsize JNICALL JNIEnvInternal::GetStringUTFLength( JNIEnv *pEnv, jstring string )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pStringObject = ConvertJObjectToObjectPointer( string );

    JavaString stringValue = HelperTypes::ExtractValueFromStringObject( pStringObject );
    std::string utf8String = stringValue.ToUtf8String();

    return ( jsize )utf8String.length();
  }

  JNIEXPORT jstring JNICALL JNIEnvInternal::NewStringUTF( JNIEnv *pEnv, const char *bytes )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pStringObject = pVirtualMachineState->CreateStringObject( bytes );

    // Note, this string is cleaned up in the garbage collector!

    return ConvertObjectPointerToJString( pVirtualMachineState, pStringObject );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseStringChars( JNIEnv *pEnv, jstring string, const jchar *chars )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT const jchar *JNICALL JNIEnvInternal::GetStringChars( JNIEnv *pEnv, jstring string, jboolean *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jsize JNICALL JNIEnvInternal::GetStringLength( JNIEnv *pEnv, jstring string )
  {
    boost::intrusive_ptr<ObjectReference> pString = ConvertJObjectToObjectPointer( string );

    boost::intrusive_ptr<JavaInteger> pCount = boost::dynamic_pointer_cast<JavaInteger>( pString->GetContainedObject()->GetFieldByNameConst( JavaString::FromCString( u"count" ) ) );
    JVMX_ASSERT( nullptr != pCount );

    return static_cast<jsize>( pCount->ToHostInt32() );
  }

  JNIEXPORT jstring JNICALL JNIEnvInternal::NewString( JNIEnv *pEnv, const jchar *uchars, jsize len )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pStringObject = pVirtualMachineState->CreateStringObject( JavaString::FromCString( reinterpret_cast<const char16_t *>( uchars ) ) );

    // Note, this string is cleaned up in the garbage collector!

    return ConvertObjectPointerToJString( pVirtualMachineState, pStringObject );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticDoubleField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jdouble value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticFloatField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jfloat value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticLongField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jlong value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticIntField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jint value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticShortField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jshort value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticCharField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jchar value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticByteField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jbyte value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticBooleanField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jboolean value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetStaticObjectField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID, jobject value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::GetStaticDoubleField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::GetStaticFloatField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::GetStaticLongField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::GetStaticIntField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::GetStaticShortField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::GetStaticCharField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::GetStaticByteField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::GetStaticBooleanField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::GetStaticObjectField( JNIEnv *pEnv, jclass clazz, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfieldID JNICALL JNIEnvInternal::GetStaticFieldID( JNIEnv *pEnv, jclass clazz, const char *name, const char *sig )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallStaticVoidMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallStaticVoidMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallStaticVoidMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallStaticDoubleMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallStaticDoubleMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallStaticDoubleMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallStaticFloatMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallStaticFloatMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallStaticFloatMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallStaticLongMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallStaticLongMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallStaticLongMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallStaticIntMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallStaticIntMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallStaticIntMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallStaticShortMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallStaticShortMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallStaticShortMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallStaticCharMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallStaticCharMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallStaticCharMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallStaticByteMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallStaticByteMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallStaticByteMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallStaticBooleanMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallStaticBooleanMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallStaticBooleanMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallStaticObjectMethodA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallStaticObjectMethodV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallStaticObjectMethod( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jmethodID JNICALL JNIEnvInternal::GetStaticMethodID( JNIEnv *pEnv, jclass clazz, const char *name, const char *sig )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetDoubleField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jdouble value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetFloatField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jfloat value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetLongField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jlong value )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
    try
    {
      pObject->GetContainedObject()->SetField( GetNameFromFieldID( fieldID ), new JavaLong( JavaLong::FromHostInt64( value ) ) );
    }
    catch ( InvalidArgumentException &ex )
    {
      auto pClass = pVirtualMachineState->LoadClass( JavaString::FromCString( c_JavaIllegalAccessErrorException ) );

      if ( pClass->IsInitialsed() )
      {
        pVirtualMachineState->InitialiseClass( JavaString::FromCString( c_JavaIllegalAccessErrorException ) );
      }

      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaIllegalAccessErrorException ), ex.what() );
    }
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetIntField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jint value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetShortField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jshort value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetCharField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jchar value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetByteField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jbyte value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetBooleanField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jboolean value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetObjectField( JNIEnv *pEnv, jobject obj, jfieldID fieldID, jobject value )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::GetDoubleField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::GetFloatField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::GetLongField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
    boost::intrusive_ptr<JavaLong> pLong = boost::dynamic_pointer_cast<JavaLong>( pObject->GetContainedObject()->GetFieldByNameConst( GetNameFromFieldID( fieldID ) ) );

    return pLong->ToHostInt64();
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::GetIntField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
    boost::intrusive_ptr<JavaInteger> pValue = boost::dynamic_pointer_cast<JavaInteger>( pObject->GetContainedObject()->GetFieldByNameConst( GetNameFromFieldID( fieldID ) ) );

    if ( nullptr == pValue )
    {
      pValue = TypeParser::UpCastToInteger( pObject->GetContainedObject()->GetFieldByName( GetNameFromFieldID( fieldID ) ).get() );
    }

    return pValue->ToHostInt32();
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::GetShortField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::GetCharField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::GetByteField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::GetBooleanField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::GetObjectField( JNIEnv *pEnv, jobject obj, jfieldID fieldID )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
    boost::intrusive_ptr<ObjectReference> pValue = boost::dynamic_pointer_cast<ObjectReference>( pObject->GetContainedObject()->GetFieldByNameConst( GetNameFromFieldID( fieldID ) ) );

    return ConvertObjectPointerToJObject( pVirtualMachineState, pValue );
  }

  JNIEXPORT jfieldID JNICALL JNIEnvInternal::GetFieldID( JNIEnv *pEnv, jclass clazz, const char *name, const char *sig )
  {
    //     JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
    //     IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);
    //
    //     boost::intrusive_ptr<ObjectReference> pClassObject = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );
    //     boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>(pClassObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ));
    //
    //     std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    //     std::shared_ptr<FieldInfo> pFieldInfo = pClass->GetFieldByName( JavaString::FromCString( name ) );
    //
    //     pFieldInfo->
    return GetFieldIDFromName( name );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallNonvirtualVoidMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallNonvirtualVoidMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallNonvirtualVoidMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallNonvirtualDoubleMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallNonvirtualDoubleMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallNonvirtualDoubleMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallNonvirtualFloatMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallNonvirtualFloatMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallNonvirtualFloatMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallNonvirtualLongMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallNonvirtualLongMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallNonvirtualLongMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallNonvirtualIntMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallNonvirtualIntMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallNonvirtualIntMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallNonvirtualShortMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallNonvirtualShortMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallNonvirtualShortMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallNonvirtualCharMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallNonvirtualCharMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallNonvirtualCharMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallNonvirtualByteMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallNonvirtualByteMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallNonvirtualByteMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallNonvirtualBooleanMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallNonvirtualBooleanMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallNonvirtualBooleanMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallNonvirtualObjectMethodA( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallNonvirtualObjectMethodV( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallNonvirtualObjectMethod( JNIEnv *pEnv, jobject obj, jclass clazz, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallVoidMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallVoidMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    boost::intrusive_ptr<ObjectReference> pTargetObject = ConvertJObjectToObjectPointer( obj );
    if ( nullptr == pTargetObject )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Object passed for object argument." );
    }

    std::shared_ptr<MethodInfo> pMethodInfo = methodID->m_pMethodInfo;

    std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
    std::shared_ptr<IVirtualMachineState> pVirtualMachineState = pThreadManager->GetCurrentThreadState();

    TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *( pMethodInfo->GetType() ) );

#ifdef _DEBUG
    const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

    if ( !pMethodInfo->IsNative() )
    {
      // Push the object to act on.
      pVirtualMachineState->PushOperand( pTargetObject );

      for ( size_t currentParamIndex = 0; currentParamIndex < parsedType.parameters.size(); ++ currentParamIndex )
      {
        switch ( ( parsedType.parameters.begin() + currentParamIndex )->get()->At( 0 ) )
        {
          case c_JavaTypeSpecifierReference:
            {
              boost::intrusive_ptr<ObjectReference> pObject = boost::intrusive_ptr<ObjectReference>( ConvertJObjectToObjectPointer( va_arg( args, jobject ) ) );
              pVirtualMachineState->PushOperand( pObject );
            }
            break;

          case c_JavaTypeSpecifierInteger:
            {
              boost::intrusive_ptr<JavaInteger> pObject = new JavaInteger( JavaInteger::FromHostInt32( va_arg( args, jint ) ) );
              pVirtualMachineState->PushOperand( pObject );
            }
            break;

          default:
            throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
            break;
        }
      }
    }
    else
    {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            pVirtualMachineState->GetLogger()->LogDebug("Operand stack before native method call:");
            pVirtualMachineState->LogOperandStack();
        }
#endif // _DEBUG
    }

    JNIEnvInternal::ExecuteMethodImpl( pMethodInfo, pVirtualMachineState.get(), pTargetObject );

#ifdef _DEBUG
    JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
  }

  JNIEXPORT void JNICALL JNIEnvInternal::CallVoidMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    va_list arguments;
    va_start( arguments, methodID );

    CallVoidMethodV( pEnv, obj, methodID, arguments );

    va_end( arguments );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallDoubleMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallDoubleMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble JNICALL JNIEnvInternal::CallDoubleMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallFloatMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallFloatMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat JNICALL JNIEnvInternal::CallFloatMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallLongMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallLongMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong JNICALL JNIEnvInternal::CallLongMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallIntMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallIntMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::CallIntMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallShortMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallShortMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort JNICALL JNIEnvInternal::CallShortMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallCharMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallCharMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar JNICALL JNIEnvInternal::CallCharMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallByteMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallByteMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte JNICALL JNIEnvInternal::CallByteMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallBooleanMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallBooleanMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::CallBooleanMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallObjectMethodA( JNIEnv *pEnv, jobject obj, jmethodID methodID, jvalue *args )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallObjectMethodV( JNIEnv *pEnv, jobject obj, jmethodID methodID, va_list args )
  {
    boost::intrusive_ptr<ObjectReference> pTargetObject = ConvertJObjectToObjectPointer( obj );
    if ( nullptr == pTargetObject )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Object passed for object argument." );
    }

    std::shared_ptr<MethodInfo> pMethodInfo = methodID->m_pMethodInfo;

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *( pMethodInfo->GetType() ) );

#if defined (_DEBUG) //&& defined(JVMX_LOG_VERBOSE)
    pVirtualMachineState->GetLogger()->LogDebug( "Operand stack before native method call:" );
    pVirtualMachineState->LogOperandStack();
#endif // _DEBUG

#ifdef _DEBUG
    const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

    // Push the object to act on.
    pVirtualMachineState->PushOperand( boost::intrusive_ptr<ObjectReference>( pTargetObject ) );

    for ( size_t currentParamIndex = 0; currentParamIndex < parsedType.parameters.size(); ++ currentParamIndex )
    {
      switch ( ( parsedType.parameters.begin() + currentParamIndex )->get()->At( 0 ) )
      {
        case c_JavaTypeSpecifierReference:
          {
            boost::intrusive_ptr<ObjectReference> pObject = boost::intrusive_ptr<ObjectReference>( ConvertJObjectToObjectPointer( va_arg( args, jobject ) ) );
            pVirtualMachineState->PushOperand( pObject );
          }
          break;

        default:
          throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
          break;
      }
    }

    JNIEnvInternal::ExecuteMethodImpl( pMethodInfo, pVirtualMachineState, pTargetObject );

    boost::intrusive_ptr<IJavaVariableType> pReturnValue = pVirtualMachineState->PopOperand();
    if ( !pReturnValue->IsReferenceType() )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected reference on the stack." );
    }

#if defined (_DEBUG) //&& defined(JVMX_LOG_VERBOSE)
    pVirtualMachineState->GetLogger()->LogDebug( "Operand stack after native method call:" );
    pVirtualMachineState->LogOperandStack();
#endif // _DEBUG


#ifdef _DEBUG
    JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG

    return ConvertObjectPointerToJObject( pVirtualMachineState, boost::dynamic_pointer_cast<ObjectReference>( pReturnValue ) );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::CallObjectMethod( JNIEnv *pEnv, jobject obj, jmethodID methodID, ... )
  {
    va_list arguments;
    va_start( arguments, methodID );

    jobject result = CallObjectMethodV( pEnv, obj, methodID, arguments );

    va_end( arguments );
    return result;
  }

  JNIEXPORT jmethodID JNICALL JNIEnvInternal::GetMethodID( JNIEnv *pEnv, jclass clazz, const char *name, const char *sig )
  {
    boost::intrusive_ptr<ObjectReference> pJavaLangClass = ConvertJObjectToObjectPointer( clazz );
    if ( nullptr == pJavaLangClass )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Pointer passed for class argument." );
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    auto pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
    std::shared_ptr<JavaClass> pObjectClass = pVirtualMachineState->LoadClass( *pClassName );

    std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethod( pObjectClass.get(), JavaString::FromCString( name ), JavaString::FromCString( sig ) );

    if ( nullptr == pMethodInfo )
    {
      JVMX_ASSERT( false );
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaNoSuchMethodErrorException ), "Method was not found." );
      return nullptr;
    }

    jmethodID methodID = new _jmethodID;
    methodID->m_pMethodInfo = pMethodInfo;

    AddToMethodCache( clazz, JavaString::FromCString( name ), JavaString::FromCString( sig ), methodID );

    return methodID;
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::IsInstanceOf( JNIEnv *pEnv, jobject obj, jclass clazz )
  {
    boost::intrusive_ptr<ObjectReference> pJavaLangClass = ConvertJObjectToObjectPointer( clazz );
    if ( nullptr == pJavaLangClass )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Pointer passed for class argument." );
    }

    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( obj );
    if ( pObject->IsNull() )
    {
      return JNI_TRUE;
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    auto pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
    //std::shared_ptr<JavaClass> pObjectClass = pVirtualMachineState->LoadClass( *pClassName );

    if ( pObject->GetContainedObject()->IsInstanceOf( *pClassName ) )
    {
      return JNI_TRUE;
    }

    return JNI_FALSE;
  }

  JNIEXPORT jclass JNICALL JNIEnvInternal::GetObjectClass( JNIEnv *pEnv, jobject obj )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObjectRef = ConvertJObjectToObjectPointer( obj );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->FindJavaLangClass( *pObjectRef->GetContainedObject()->GetClass()->GetName() );
    if ( nullptr == pJavaLangClass )
    {
      pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pObjectRef->GetContainedObject()->GetClass()->GetName() );
    }

    return static_cast<jclass>( ConvertObjectPointerToJObject( pVirtualMachineState, pJavaLangClass ) );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::NewObjectA( JNIEnv *pEnv, jclass clazz, jmethodID methodID, jvalue *args )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass( ConvertJObjectToObjectPointer( clazz ) );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
    if ( nullptr == pClassName )
    {
      throw InvalidStateException( "Expected class name to be valid string." );
    }

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    boost::intrusive_ptr<ObjectReference> pNewObject = pVirtualMachineState->CreateObject( pClass );

    if ( nullptr == pNewObject )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected to be able to create an object." );
    }

    if ( nullptr != methodID )
    {
      std::shared_ptr<MethodInfo> pMethodInfo = methodID->m_pMethodInfo;

      pVirtualMachineState->PushOperand( pNewObject );

      TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *( methodID->m_pMethodInfo->GetType() ) );
      for ( size_t i = 0; i < parsedType.parameters.size(); ++ i )
      {
        pVirtualMachineState->PushOperand( CreateJavaVariableFromJValue( args[ i ], parsedType.parameters.at( i ) ) );
      }

      JNIEnvInternal::ExecuteMethodImpl( pMethodInfo, pVirtualMachineState, pNewObject );
    }

    return ConvertObjectPointerToJObject( pVirtualMachineState, pNewObject );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::NewObjectV( JNIEnv *pEnv, jclass clazz, jmethodID methodID, va_list args )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference > pJavaLangClass( JNIEnvInternal::ConvertJObjectToObjectPointer( clazz ) );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    if ( nullptr == pClass )
    {
      throw InvalidStateException( __FUNCTION__ " - Java exception class could not be loaded." );
    }

    if ( !pVirtualMachineState->IsClassInitialised( *pClass->GetName() ) )
    {
      pVirtualMachineState->InitialiseClass( *pClass->GetName() );
    }

    boost::intrusive_ptr<ObjectReference> pNewObject = pVirtualMachineState->CreateObject( pClass );
    if ( nullptr == pNewObject )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected to be able to create an object." );
    }

    if ( nullptr != methodID )
    {
      std::shared_ptr<MethodInfo> pMethodInfo = methodID->m_pMethodInfo;

#ifdef _DEBUG
      const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
          pVirtualMachineState->GetLogger()->LogDebug(__FUNCTION__ " Calling (%s::%s) - Operand stack before:", pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str());
          pVirtualMachineState->LogOperandStack();
      }
#endif // _DEBUG

      pVirtualMachineState->PushOperand( pNewObject );

      TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *( methodID->m_pMethodInfo->GetType() ) );
      for ( size_t i = 0; i < parsedType.parameters.size(); ++ i )
      {
        char16_t type = parsedType.parameters[ i ]->At( 0 );
        switch ( type )
        {
          case c_JavaTypeSpecifierLong:
            pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( va_arg( args, jlong ) ) ) );
            break;

          case c_JavaTypeSpecifierByte:
            pVirtualMachineState->PushOperand( new JavaByte( JavaByte::FromHostInt8( va_arg( args, jbyte ) ) ) );
            break;

          case c_JavaTypeSpecifierInteger:
            pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( va_arg( args, jint ) ) ) );
            break;

          case c_JavaTypeSpecifierChar:
            pVirtualMachineState->PushOperand( new JavaChar( JavaChar::FromUInt16( va_arg( args, jchar ) ) ) );
            break;

          case c_JavaTypeSpecifierBool:
            pVirtualMachineState->PushOperand( new JavaBool( JavaBool::FromUint16( ( uint16_t )va_arg( args, jboolean ) ) ) );
            break;

          case c_JavaTypeSpecifierShort:
            pVirtualMachineState->PushOperand( new JavaShort( JavaShort::FromHostInt16( va_arg( args, jshort ) ) ) );
            break;

          case c_JavaTypeSpecifierFloat:
            pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( va_arg( args, jfloat ) ) ) );
            break;

          case c_JavaTypeSpecifierDouble:
            pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( va_arg( args, jdouble ) ) ) );
            break;

          case c_JavaTypeSpecifierReference:
          case c_JavaTypeSpecifierArray:
            pVirtualMachineState->PushOperand( JNIEnvInternal::ConvertJObjectToObjectPointer( va_arg( args, jobject ) ) );
            break;

          default:
            throw InvalidStateException( __FUNCTION__ " - Unexpected variable type:" );
            break;
        }
      }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
          pVirtualMachineState->GetLogger()->LogDebug(__FUNCTION__ " Calling (%s::%s) - Operand stack after:", pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str());
          pVirtualMachineState->LogOperandStack();
      }
#endif // _DEBUG

      JNIEnvInternal::ExecuteMethodImpl( pMethodInfo, pVirtualMachineState, pNewObject );

#ifdef _DEBUG
      JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
    }

    return ConvertObjectPointerToJObject( pVirtualMachineState, pNewObject );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::NewObject( JNIEnv *pEnv, jclass clazz, jmethodID methodID, ... )
  {
    va_list arguments;
    va_start( arguments, methodID );

    jobject result = NewObjectV( pEnv, clazz, methodID, arguments );

    va_end( arguments );
    return result;
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::AllocObject( JNIEnv *pEnv, jclass clazz )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference > pJavaLangClass( JNIEnvInternal::ConvertJObjectToObjectPointer( clazz ) );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    if ( nullptr == pClass )
    {
      throw InvalidStateException( __FUNCTION__ " - Java exception class could not be loaded." );
    }

    if ( !pVirtualMachineState->IsClassInitialised( *pClass->GetName() ) )
    {
      pVirtualMachineState->InitialiseClass( *pClass->GetName() );
    }

    boost::intrusive_ptr<ObjectReference> pNewObject = pVirtualMachineState->CreateObject( pClass );
    if ( nullptr == pNewObject )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected to be able to create an object." );
    }

    return ConvertObjectPointerToJObject( pVirtualMachineState, pNewObject );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::EnsureLocalCapacity( JNIEnv *pEnv, jint capacity )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::NewLocalRef( JNIEnv *pEnv, jobject ref )
  {
    boost::intrusive_ptr<ObjectReference> pLocalReference = ConvertJObjectToObjectPointer( ref );
    if ( nullptr == pLocalReference )
    {
      return ConvertNullPointerToJObject();
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    pVirtualMachineState->AddLocalReference( pLocalReference );

    return ref;
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::IsSameObject( JNIEnv *pEnv, jobject ref1, jobject ref2 )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject1 = ConvertJObjectToObjectPointer( ref1 );
    boost::intrusive_ptr<ObjectReference> pObject2 = ConvertJObjectToObjectPointer( ref2 );

    if ( pObject1->GetIndex() == pObject2->GetIndex() )
    {
      return JNI_TRUE;
    }

    return JNI_FALSE;
  }

  JNIEXPORT void JNICALL JNIEnvInternal::DeleteLocalRef( JNIEnv *pEnv, jobject lref )
  {
    boost::intrusive_ptr<ObjectReference> pLocalReference = ConvertJObjectToObjectPointer( lref );
    if ( nullptr == pLocalReference )
    {
      // This is no-op.
      return;
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    pVirtualMachineState->DeleteLocalReference( pLocalReference );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::DeleteGlobalRef( JNIEnv *pEnv, jobject gref )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( gref );

    if ( pObject->IsNull() )
    {
      return ;
    }

    pVirtualMachineState->DeleteGlobalReference( pObject );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::NewGlobalRef( JNIEnv *pEnv, jobject obj )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( obj );

    if ( pObject->IsNull() )
    {
      return ConvertNullPointerToJObject();
    }

    pVirtualMachineState->AddGlobalReference( pObject );
    return obj;
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::PopLocalFrame( JNIEnv *pEnv, jobject result )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    pVirtualMachineState->DeleteLocalReferenceFrame();

    if ( nullptr == result )
    {
      return ConvertNullPointerToJObject();
    }

    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( result );

    // This will insert the object into the previous local reference frame.
    return ConvertObjectPointerToJObject( pVirtualMachineState, pObject );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::PushLocalFrame( JNIEnv *pEnv, jint capacity )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    pVirtualMachineState->AddLocalReferenceFrame();

    return 0;
  }

  JNIEXPORT void JNICALL JNIEnvInternal::FatalError( JNIEnv *pEnv, const char *msg )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ExceptionClear( JNIEnv *pEnv )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    pVirtualMachineState->ResetException();
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ExceptionDescribe( JNIEnv *pEnv )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pExceptionObject = pVirtualMachineState->GetException();
    std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethod( pExceptionObject->GetContainedObject()->GetClass().get(), JavaString::FromCString( u"printStackTrace" ), JavaString::FromCString( "()V" ) );

#if defined(_DEBUG) //&& defined(JVMX_LOG_VERBOSE)
    pVirtualMachineState->GetLogger()->LogDebug( " Exception Object: %s", pExceptionObject->ToString().ToUtf8String().c_str() );
#endif // _DEBUG

    pVirtualMachineState->ResetException();

    pVirtualMachineState->PushOperand( pExceptionObject );
    pVirtualMachineState->Execute( *pMethodInfo->GetClass()->GetName(), *pMethodInfo->GetName(), *pMethodInfo->GetType() );
  }

  JNIEXPORT jthrowable JNICALL JNIEnvInternal::ExceptionOccurred( JNIEnv *pEnv )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    if ( !pVirtualMachineState->HasExceptionOccurred() )
    {
      return ( jthrowable ) ConvertNullPointerToJObject();
    }

    return ( jthrowable ) ConvertObjectPointerToJObject( pVirtualMachineState, pVirtualMachineState->GetException() );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::ThrowNew( JNIEnv *pEnv, jclass clazz, const char *message )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = ConvertJObjectToObjectPointer( clazz );
    if ( nullptr == pJavaLangClass )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Pointer passed for class argument." );
    }

    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    if ( nullptr == pClass )
    {
      throw InvalidStateException( __FUNCTION__ " - Java exception class could not be loaded." );
    }

    if ( !pVirtualMachineState->IsClassInitialised( *pClass->GetName() ) )
    {
      pVirtualMachineState->InitialiseClass( *pClass->GetName() );
    }

    boost::intrusive_ptr<ObjectReference> pMessage = pVirtualMachineState->CreateStringObject( JavaString::FromCString( message ) );
    boost::intrusive_ptr<ObjectReference> pExceptionObject = pVirtualMachineState->CreateObject( pClass );

    pVirtualMachineState->PushOperand( pExceptionObject );
    pVirtualMachineState->PushOperand( pMessage );

    std::shared_ptr<MethodInfo> pMethodInfo = pClass->GetMethodByNameAndType( c_InstanceInitialisationMethodName, JavaString::FromCString( u"(Ljava/lang/String;)V" ) );
    pVirtualMachineState->ExecuteMethod( * pClassName, c_InstanceInitialisationMethodName, JavaString::FromCString( u"(Ljava/lang/String;)V" ), pMethodInfo );

    pVirtualMachineState->SetExceptionThrown( pExceptionObject );

    return 0;
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::Throw( JNIEnv *pEnv, jthrowable obj )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pObjectToThrow = ConvertJObjectToObjectPointer( obj );
    if ( nullptr == pObjectToThrow )
    {
      throw NullPointerException( __FUNCTION__ " - NULL Pointer passed for class argument." );
    }

    pVirtualMachineState->SetExceptionThrown( pObjectToThrow );

    return 0;
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::ToReflectedField( JNIEnv *pEnv, jclass cls, jfieldID fieldID, jboolean isStatic )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = JNIEnvInternal::ConvertJObjectToObjectPointer( cls );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    std::shared_ptr<FieldInfo> pFieldInfo = pClass->GetFieldByName( GetNameFromFieldID( fieldID ) );

    // Create VMField Object

    std::shared_ptr<JavaClass> pJavaLangRefectVMFieldClass = pVirtualMachineState->LoadClass( c_JavaLangReflectVMField_ClassName );
    if ( !pJavaLangRefectVMFieldClass->IsInitialsed() )
    {
      pVirtualMachineState->InitialiseClass( c_JavaLangReflectVMField_ClassName );
    }

    boost::intrusive_ptr<ObjectReference> pVMFieldObject = pVirtualMachineState->CreateObject( pJavaLangRefectVMFieldClass );
    if ( nullptr == pVMFieldObject )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected to be able to create an object." );
    }

    auto pVMFieldConstructor = pJavaLangRefectVMFieldClass->GetMethodByNameAndType( c_InstanceInitialisationMethodName, JavaString::FromCString( u"(Ljava/lang/Class;Ljava/lang/String;I)V" ) );
    if ( nullptr == pVMFieldConstructor )
    {
      throw InvalidStateException( __FUNCTION__ " - Could not get constructor for VMField." );
    }

    pVirtualMachineState->PushOperand( pVMFieldObject );
    pVirtualMachineState->PushOperand( pJavaLangClass );
    pVirtualMachineState->PushOperand( pVirtualMachineState->CreateStringObject( *pFieldInfo->GetName() ) );
    pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( pFieldInfo->GetOffset() ) ) );

    JNIEnvInternal::ExecuteMethodImpl( pVMFieldConstructor, pVirtualMachineState, pVMFieldObject );

    // Create proper Field Object to return

    std::shared_ptr<JavaClass> pJavaLangRefectFieldClass = pVirtualMachineState->LoadClass( c_JavaLangReflectField_ClassName );
    if ( !pJavaLangRefectFieldClass->IsInitialsed() )
    {
      pVirtualMachineState->InitialiseClass( c_JavaLangReflectField_ClassName );
    }

    boost::intrusive_ptr<ObjectReference> pResultObject = pVirtualMachineState->CreateObject( pJavaLangRefectFieldClass );
    if ( nullptr == pResultObject )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected to be able to create an object." );
    }

    auto pFieldConstructor = pJavaLangRefectFieldClass->GetMethodByNameAndType( c_InstanceInitialisationMethodName, JavaString::FromCString( u"(Ljava/lang/reflect/VMField;)V" ) );
    if ( nullptr == pFieldConstructor )
    {
      throw InvalidStateException( __FUNCTION__ " - Could not get constructor for VMField." );
    }

    pVirtualMachineState->PushOperand( pResultObject );
    pVirtualMachineState->PushOperand( pVMFieldObject );

    JNIEnvInternal::ExecuteMethodImpl( pFieldConstructor, pVirtualMachineState, pResultObject );

    return ConvertObjectPointerToJObject( pVirtualMachineState, pResultObject );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::IsAssignableFrom( JNIEnv *pEnv, jclass clazz1, jclass clazz2 )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jclass JNICALL JNIEnvInternal::GetSuperclass( JNIEnv *pEnv, jclass clazz )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );

    boost::intrusive_ptr<ObjectReference> pSuperClass = pVirtualMachineState->FindJavaLangClass( *pClass->GetSuperClassName() );
    if ( nullptr == pSuperClass )
    {
      pSuperClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pClass->GetSuperClassName() );
      JVMX_ASSERT( nullptr != pSuperClass );
    }

    return static_cast< jclass >( ConvertObjectPointerToJObject( pVirtualMachineState, pSuperClass ) );
  }

  JNIEXPORT jobject JNICALL JNIEnvInternal::ToReflectedMethod( JNIEnv *pEnv, jclass cls, jmethodID methodID, jboolean isStatic )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfieldID JNICALL JNIEnvInternal::FromReflectedField( JNIEnv *pEnv, jobject field )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jmethodID JNICALL JNIEnvInternal::FromReflectedMethod( JNIEnv *pEnv, jobject method )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jclass JNICALL JNIEnvInternal::FindClass( JNIEnv *pEnv, const char *name )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    std::shared_ptr<IClassLibrary> pConstantPool = pVirtualMachineState->GetRuntimeConstantPool();

    boost::intrusive_ptr<JavaString> pClassName = new JavaString( JavaString::FromCString( name ) );

    std::shared_ptr<JavaClass> pClassFile = pConstantPool->FindClass( *pClassName );
    if ( nullptr == pClassFile )
    {
      pClassFile = pVirtualMachineState->LoadClass( *pClassName );
      if ( nullptr == pClassFile )
      {
        throw FileDoesNotExistException( __FUNCTION__ " - Could not load class" );
      }
    }

    if ( !pClassFile->IsInitialsed() )
    {
      pVirtualMachineState->InitialiseClass( *pClassName );
    }

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pClassName );

    return static_cast<jclass>( ConvertObjectPointerToJObject( pVirtualMachineState, pJavaLangClass ) );
  }

  JNIEXPORT jclass JNICALL JNIEnvInternal::DefineClass( JNIEnv *pEnv, const char *name, jobject loader, const jbyte *buf, jsize bufLen )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::GetVersion( JNIEnv *pEnv )
  {
    return JNI_VERSION_1_2;
  }

  JNIEXPORT jboolean *JNICALL JNIEnvInternal::GetBooleanArrayElements( JNIEnv *pEnv, jbooleanArray array, jboolean *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jbyte *JNICALL JNIEnvInternal::GetByteArrayElements( JNIEnv *pEnv, jbyteArray array, jboolean *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jchar *JNICALL JNIEnvInternal::GetCharArrayElements( JNIEnv *pEnv, jcharArray array, jchar *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jshort *JNICALL JNIEnvInternal::GetShortArrayElements( JNIEnv *pEnv, jshortArray array, jshort *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint *JNICALL JNIEnvInternal::GetIntArrayElements( JNIEnv *pEnv, jintArray array, jint *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jlong *JNICALL JNIEnvInternal::GetLongArrayElements( JNIEnv *pEnv, jlongArray array, jlong *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jfloat *JNICALL JNIEnvInternal::GetFloatArrayElements( JNIEnv *pEnv, jfloatArray array, jfloat *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jdouble *JNICALL JNIEnvInternal::GetDoubleArrayElements( JNIEnv *pEnv, jdoubleArray array, jdouble *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseBooleanArrayElements( JNIEnv *pEnv, jbooleanArray array, jboolean *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseByteArrayElements( JNIEnv *pEnv, jbyteArray array, jbyte *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseCharArrayElements( JNIEnv *pEnv, jcharArray array, jchar *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseShortArrayElements( JNIEnv *pEnv, jshortArray array, jshort *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseIntArrayElements( JNIEnv *pEnv, jintArray array, jint *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseLongArrayElements( JNIEnv *pEnv, jlongArray array, jlong *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseFloatArrayElements( JNIEnv *pEnv, jfloatArray array, jfloat *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseDoubleArrayElements( JNIEnv *pEnv, jdoubleArray array, jdouble *elems, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetBooleanArrayRegion( JNIEnv *pEnv, jbooleanArray array, jsize start, jsize len, jboolean *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetByteArrayRegion( JNIEnv *pEnv, jbyteArray array, jsize start, jsize len, jbyte *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetCharArrayRegion( JNIEnv *pEnv, jcharArray array, jsize start, jsize len, jchar *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetShortArrayRegion( JNIEnv *pEnv, jshortArray array, jsize start, jsize len, jshort *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetIntArrayRegion( JNIEnv *pEnv, jintArray array, jsize start, jsize len, jint *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetLongArrayRegion( JNIEnv *pEnv, jlongArray array, jsize start, jsize len, jlong *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetFloatArrayRegion( JNIEnv *pEnv, jfloatArray array, jsize start, jsize len, jfloat *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetDoubleArrayRegion( JNIEnv *pEnv, jdoubleArray array, jsize start, jsize len, jdouble *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetBooleanArrayRegion( JNIEnv *pEnv, jbooleanArray array, jsize start, jsize len, jboolean *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetByteArrayRegion( JNIEnv *pEnv, jbyteArray array, jsize start, jsize len, jbyte *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetCharArrayRegion( JNIEnv *pEnv, jcharArray array, jsize start, jsize len, jchar *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetShortArrayRegion( JNIEnv *pEnv, jshortArray array, jsize start, jsize len, jshort *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetIntArrayRegion( JNIEnv *pEnv, jintArray array, jsize start, jsize len, jint *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetLongArrayRegion( JNIEnv *pEnv, jlongArray array, jsize start, jsize len, jlong *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetFloatArrayRegion( JNIEnv *pEnv, jfloatArray array, jsize start, jsize len, jfloat *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::SetDoubleArrayRegion( JNIEnv *pEnv, jdoubleArray array, jsize start, jsize len, jdouble *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::RegisterNatives( JNIEnv *pEnv, jclass clazz, const JNINativeMethod *methods, jint nMethods )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::UnregisterNatives( JNIEnv *pEnv, jclass clazz )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::MonitorEnter( JNIEnv *pEnv, jobject obj )
  {
    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( obj );
    if ( e_JavaVariableTypes::Array == pObject->GetVariableType() )
    {
      pObject->GetContainedArray()->MonitorEnter( __FUNCTION__ );
    }
    else
    {
      pObject->GetContainedObject()->MonitorEnter( __FUNCTION__ );
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );
    if ( pVirtualMachineState->HasExceptionOccurred() )
    {
      return -1;
    }

    return 0;
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::MonitorExit( JNIEnv *pEnv, jobject obj )
  {
    boost::intrusive_ptr<ObjectReference> pObject = ConvertJObjectToObjectPointer( obj );
    if ( e_JavaVariableTypes::Array == pObject->GetVariableType() )
    {
      pObject->GetContainedArray()->MonitorExit( __FUNCTION__ );
    }
    else
    {
      pObject->GetContainedObject()->MonitorExit( __FUNCTION__ );
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );
    if ( pVirtualMachineState->HasExceptionOccurred() )
    {
      return -1;
    }

    return 0;
  }

  JNIEXPORT jint JNICALL JNIEnvInternal::GetJavaVM( JNIEnv *pEnv, JavaVM **vm )
  {
    std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
    std::shared_ptr<IVirtualMachineState> pVirtualMachineState = pThreadManager->GetCurrentThreadState();

    JVMX_ASSERT( nullptr != pVirtualMachineState );

    *vm = pVirtualMachineState->GetJavaNativeInterface()->GetVM();

    return 0;
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetStringRegion( JNIEnv *pEnv, jstring str, jsize start, jsize len, jchar *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::GetStringUTFRegion( JNIEnv *pEnv, jstring str, jsize start, jsize len, char *buf )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void *JNICALL JNIEnvInternal::GetPrimitiveArrayCritical( JNIEnv *pEnv, jarray array, jboolean *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleasePrimitiveArrayCritical( JNIEnv *pEnv, jarray array, void *carray, jint mode )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT const jchar *JNICALL JNIEnvInternal::GetStringCritical( JNIEnv *pEnv, jstring string, jboolean *isCopy )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::ReleaseStringCritical( JNIEnv *pEnv, jstring string, const jchar *carray )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jweak JNICALL JNIEnvInternal::NewWeakGlobalRef( JNIEnv *pEnv, jobject obj )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::DeleteWeakGlobalRef( JNIEnv *pEnv, jobject wref )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT jboolean JNICALL JNIEnvInternal::ExceptionCheck( JNIEnv *pEnv )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  JNIEXPORT void JNICALL JNIEnvInternal::JVMX_arraycopy( JNIEnv *pEnv, jobject obj, jobject src, int srcOffset, jobject dest, int destOffset, int length )
  {
    if ( 0 == length )
    {
      return;
    }

    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

    boost::intrusive_ptr<ObjectReference> pSource( ConvertJArrayToArrayPointer( static_cast<jarray>( src ) ) );
    boost::intrusive_ptr<ObjectReference> pDestination( ConvertJArrayToArrayPointer( static_cast<jarray>( dest ) ) );

    if ( nullptr == pSource || pSource->GetVariableType() != e_JavaVariableTypes::Array )
    {
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaArrayStoreException ), "Expected an array in argument [src]." );
      return;
    }

    if ( nullptr == pDestination || pDestination->GetVariableType() != e_JavaVariableTypes::Array )
    {
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaArrayStoreException ), "Expected an array in argument [dest]." );
      return;
    }

    if ( pSource->GetContainedArray()->GetContainedType() != pDestination->GetContainedArray()->GetContainedType() )
    {
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaArrayStoreException ), "Trying to copy arrays of different types. Containted types should match." );
      return;
    }

    try
    {
      for ( int i = 0; i < length; ++ i )
      {
        pDestination->GetContainedArray()->SetAt( destOffset + i, pSource->GetContainedArray()->At( srcOffset + i ) );
      }
    }
    catch ( IndexOutOfBoundsException & )
    {
      pEnv->ThrowNew( pEnv, FindClass( pEnv, c_JavaArrayIndexOutOfBoundsException ), "Trying to copy arrays of different types. Containted types should match." );
      return;
    }
  }

  //   jmethodID JNIEnvInternal::ConvertSizeTToJMethodID( size_t methodIndex )
  //   {
  //     return reinterpret_cast<jmethodID>(methodIndex);
  //   }

  boost::intrusive_ptr<ObjectReference> JNIEnvInternal::ConvertJObjectToObjectPointer( jobject obj )
  {
    return boost::intrusive_ptr<ObjectReference>( new ObjectReference( obj ) );
  }

  boost::intrusive_ptr<ObjectReference> JNIEnvInternal::ConvertJObjectToObjectPointer( jstring obj )
  {
    return ConvertJObjectToObjectPointer( dynamic_cast<jobject>( obj ) );
  }

  boost::intrusive_ptr<ObjectReference> JNIEnvInternal::ConvertJArrayToArrayPointer( jarray obj )
  {
    return boost::intrusive_ptr<ObjectReference>( new ObjectReference( obj ) );
  }

  //   JavaArray *JNIEnvInternal::CopyStringValueToJavaArray( const char *pValue )
  //   {
  //     size_t length = strlen( pValue );
  //     JavaArray *pResult = new JavaArray( e_JavaArrayTypes::Char, length );
  //     for ( size_t i = 0; i < length; ++ i )
  //     {
  //       pResult->SetAt( JavaInteger::FromHostInt32( (uint32_t)i ), JavaChar::FromCChar( pValue[ i ] ) );
  //     }
  //
  //     return pResult;
  //   }

  void JNIEnvInternal::AddToMethodCache( jclass clazz, const JavaString &name, const JavaString &signature, jmethodID methodID )
  {
    MethodKey key( clazz, name, signature );
    s_MethodCache[ key ] = methodID;
  }

  jmethodID JNIEnvInternal::LookupInMethodCache( jclass clazz, const JavaString &name, const JavaString &signature )
  {
    MethodKey key( clazz, name, signature );
    if ( s_MethodCache.end() != s_MethodCache.find( key ) )
    {
      return s_MethodCache[ key ];
    }

    return nullptr;
  }

  bool JNIEnvInternal::MethodKey::operator<( const JNIEnvInternal::MethodKey &other ) const
  {
    return m_Class < other.m_Class && m_Name < other.m_Name && m_Signature < other.m_Signature;
  }

  // Static Variables
  std::map< JNIEnvInternal::MethodKey, jmethodID > JNIEnvInternal::s_MethodCache;
} // extern "C"

void JNIEnvInternal::ExecuteMethodImpl( std::shared_ptr<MethodInfo> pMethodInfo, IVirtualMachineState *pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pObject )
{
  if ( !pMethodInfo->IsNative() )
  {
    pVirtualMachineState->Execute( *pMethodInfo->GetClass()->GetName(), *pMethodInfo->GetName(), *pMethodInfo->GetType() );
  }
  else
  {
    if ( pMethodInfo->IsSynchronised() )
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            pVirtualMachineState->GetLogger()->LogDebug("Native Method %s is synchronized.", pMethodInfo->GetName()->ToUtf8String().c_str());
        }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor( pMethodInfo->GetClass()->MonitorEnter( pMethodInfo->GetFullName().ToUtf8String().c_str() ) );
    }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->GetLogger()->LogDebug("Method implementation is native: %s type: %s.", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str());
    }
#endif // _DEBUG

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();
    //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    pJNI->ExecuteFunction( JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( pMethodInfo->GetClass()->GetName(), pMethodInfo->GetName() ), pMethodInfo->GetType(), pObject );
    //pVirtualMachineState->PopState();

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif

    if ( pMethodInfo->IsSynchronised() )
    {
      pVirtualMachineState->PopMonitor();
      pMethodInfo->GetClass()->MonitorExit( pMethodInfo->GetFullName().ToUtf8String().c_str() );
    }
  }
}

jobject JNIEnvInternal::ConvertObjectPointerToJObject( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pObj )
{
  if ( nullptr == pObj || pObj->IsNull() )
  {
    return nullptr;
  }

  if ( nullptr != pVMState )
  {
    pVMState->AddLocalReference( pObj );
  }

  return pObj->ToJObject();
}

jstring JNIEnvInternal::ConvertObjectPointerToJString( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pObj )
{
#ifdef _DEBUG
  if ( *pObj->GetContainedObject()->GetClass()->GetName() != JavaString::FromCString( "java/lang/String" ) )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Object is not an instance of String" );
  }
#endif // _DEBUG

  if ( nullptr != pVMState )
  {
    pVMState->AddLocalReference( pObj );
  }

  return reinterpret_cast<jstring>( pObj->ToJObject() );
}

jarray JNIEnvInternal::ConvertArrayPointerToJArray( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pInput )
{
  if ( nullptr == pInput || pInput->IsNull() )
  {
    return nullptr;
  }

  if ( nullptr != pVMState )
  {
    pVMState->AddLocalReference( pInput );
  }

  return reinterpret_cast<jarray>( pInput->ToJObject() );
}

jobject JNIEnvInternal::ConvertNullPointerToJObject()
{
  return nullptr;
}

jfieldID JNIEnvInternal::GetFieldIDFromName( const char *name )
{
  return ( jfieldID )( name );
}

const JavaString JNIEnvInternal::GetNameFromFieldID( jfieldID fieldID )
{
  return JavaString::FromCString( ( const char * )( fieldID ) );
}


boost::intrusive_ptr<IJavaVariableType> CreateJavaVariableFromJValue( jvalue arg, boost::intrusive_ptr<JavaString> type )
{
  boost::intrusive_ptr<IJavaVariableType> pResult = nullptr;

  switch ( type->At( 0 ) )
  {
    case c_JavaTypeSpecifierBool:
      pResult = new JavaBool( JavaBool::FromUint16( arg.z ) );
      break;

    case c_JavaTypeSpecifierChar:
      pResult = new JavaChar( JavaChar::FromCChar( static_cast<char>( arg.c ) ) );
      break;

    case c_JavaTypeSpecifierFloat:
      pResult = new JavaFloat( JavaFloat::FromHostFloat( arg.f ) );
      break;

    case c_JavaTypeSpecifierDouble:
      pResult = new JavaDouble( JavaDouble::FromHostDouble( arg.d ) );
      break;

    case c_JavaTypeSpecifierByte:
      pResult = new JavaByte( JavaByte::FromHostInt8( arg.b ) );
      break;

    case c_JavaTypeSpecifierShort:
      pResult = new JavaShort( JavaShort::FromHostInt16( arg.s ) );
      break;

    case c_JavaTypeSpecifierInteger:
      pResult = new JavaInteger( JavaInteger::FromHostInt32( arg.i ) );
      break;

    case c_JavaTypeSpecifierLong:
      pResult = new JavaLong( JavaLong::FromHostInt64( arg.j ) );
      break;

    case c_JavaTypeSpecifierArray:
      pResult = boost::intrusive_ptr<ObjectReference>( JNIEnvInternal::ConvertJArrayToArrayPointer( static_cast<jarray>( arg.l ) ) );
      break;

    case c_JavaTypeSpecifierReference:
      if ( nullptr == arg.l )
      {
        pResult = new ObjectReference( nullptr );
      }
      else
      {
        pResult = boost::intrusive_ptr<ObjectReference>( JNIEnvInternal::ConvertJObjectToObjectPointer( arg.l ) );
      }
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }

  return pResult;
}

JNIEXPORT jint JNICALL JavaVMInternal::GetEnv( JavaVM *vm, void **penv, jint interface_id )
{
  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
  std::shared_ptr<IVirtualMachineState> pVirtualMachineState = pThreadManager->GetCurrentThreadState();

  //*penv = ((JavaVMExported *)vm)->m_pEnv;
  *penv = pVirtualMachineState->GetJavaNativeInterface()->GetEnvironment();
  return 0;
}

JNIEXPORT jint JNICALL JavaVMInternal::DestroyJavaVM( JavaVM *vm )
{
  return 0;
}

JNIEXPORT jint JNICALL JavaVMInternal::AttachCurrentThread( JavaVM *env, void **, void * )
{
  throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
}

JNIEXPORT jint JNICALL JavaVMInternal::DetachCurrentThread( JavaVM *vm )
{
  throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
}


