
#include "InvalidStateException.h"

#include "GlobalCatalog.h"
#include "ObjectReference.h"
#include "ILogger.h"

#include "HelperTypes.h"

JavaString HelperTypes::ExtractValueFromStringObject(const JavaObject* pStringObject)
{
  boost::intrusive_ptr<IJavaVariableType> pOffset = pStringObject->GetFieldByNameConst(JavaString::FromCString(JVMX_T("offset")));

  int32_t offset = 0;
  if (nullptr != pOffset)
  {
    if (pOffset->GetVariableType() == e_JavaVariableTypes::Integer)
    {
      offset = boost::dynamic_pointer_cast<JavaInteger>(pOffset)->ToHostInt32();
    }
  }

  boost::intrusive_ptr<IJavaVariableType> pField = pStringObject->GetFieldByNameConst(JavaString::FromCString(JVMX_T("value")));

  if (nullptr == pField)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected Field (value) to exist.");
  }

#ifdef _DEBUG
  auto debug = pField->GetVariableType();
#endif // _DEBUG

  if (pField->GetVariableType() == e_JavaVariableTypes::NullReference)
  {
    return pField->ToString();
  }
  else if (pField->GetVariableType() != e_JavaVariableTypes::Array)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected Field (value) to be an array.");
  }

  boost::intrusive_ptr<ObjectReference> pStringValue = boost::dynamic_pointer_cast<ObjectReference>(pField);
  if (nullptr == pStringValue)
  {
    throw InvalidStateException(__FUNCTION__ " - Cast failed.");
  }

  JavaString result = pStringValue->GetContainedArray()->ConvertCharArrayToString();
  if (offset > 0)
  {
    if (offset > result.GetLengthInCodePoints())
    {
#ifdef _DEBUG
      int i = 0;
#endif
    }
    result = result.SubString(offset);
  }
  return result;
}

JavaString HelperTypes::ExtractValueFromStringObject(const ObjectReference* pStringObject)
{
  return ExtractValueFromStringObject(pStringObject->GetContainedObject());
}

JavaString HelperTypes::GetPackageNameFromClassName(const JavaString& className)
{
  size_t slashIndex = className.FindLast(JVMX_T('/'));

  if (slashIndex != className.GetLastStringPosition())
  {
    return className.SubString(0, slashIndex);
  }

  return JavaString::EmptyString();
}

JavaString HelperTypes::ExtractValueFromStringObject( boost::intrusive_ptr<ObjectReference> pStringObject )
{
  return HelperTypes::ExtractValueFromStringObject(pStringObject.get());
}

void HelperTypes::ConvertJavaStringToArray( const boost::intrusive_ptr<ObjectReference> &pArray, const JavaString &string )
{
  JVMX_ASSERT( pArray->GetContainedArray()->GetNumberOfElements() >= string.GetLengthInCodePoints() );

  size_t len = string.GetLengthInCodePoints();
  for ( size_t i = 0; i < len; ++ i )
  {
    pArray->GetContainedArray()->SetAt( i, JavaChar::FromChar16( string.At( i ) ) );
  }
}

jvalue HelperTypes::Unbox( const boost::intrusive_ptr<IJavaVariableType> &pBoxed )
{
  JVMX_ASSERT( pBoxed->IsReferenceType() );

  JVMX_ASSERT( !pBoxed->IsNull() );
  if ( pBoxed->IsNull() )
  {
    return jvalue{ 0 };
  }

  jvalue result{ 0 };

  JVMX_ASSERT( e_JavaVariableTypes::Array != pBoxed->GetVariableType() );
  boost::intrusive_ptr<ObjectReference> pObjectReference = boost::dynamic_pointer_cast<ObjectReference>( pBoxed );
  if ( e_JavaVariableTypes::Array == pObjectReference->GetVariableType() )
  {
    result.l = pObjectReference->ToJObject();
    return result;
  }

  auto pClassName = pObjectReference->GetContainedObject()->GetClass()->GetName();
  JVMX_ASSERT( nullptr != pClassName && !pClassName->IsEmpty() );

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Integer" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Integer == pValue->GetVariableType() );
    result.i = boost::dynamic_pointer_cast<JavaInteger>( pValue )->ToHostInt32();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Boolean" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Bool == pValue->GetVariableType() );
    result.z = boost::dynamic_pointer_cast<JavaBool>( pValue )->ToBool();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Character" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Char == pValue->GetVariableType() );
    result.c = boost::dynamic_pointer_cast<JavaChar>( pValue )->ToChar16();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Byte" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Byte == pValue->GetVariableType() );
    result.b = boost::dynamic_pointer_cast<JavaByte>( pValue )->ToHostInt8();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Short" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Short == pValue->GetVariableType() );
    result.s = boost::dynamic_pointer_cast<JavaShort>( pValue )->ToHostInt16();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Long" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Long == pValue->GetVariableType() );
    result.j = boost::dynamic_pointer_cast<JavaLong>( pValue )->ToHostInt64();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Float" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Float == pValue->GetVariableType() );
    result.f = boost::dynamic_pointer_cast<JavaFloat>( pValue )->ToHostFloat();
    return result;
  }

  if ( *pClassName == JavaString::FromCString( JVMX_T( "java/lang/Double" ) ) )
  {
    boost::intrusive_ptr<IJavaVariableType> pValue = pObjectReference->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "value" ) ) );
    JVMX_ASSERT( e_JavaVariableTypes::Double == pValue->GetVariableType() );
    result.d = boost::dynamic_pointer_cast<JavaDouble>( pValue )->ToHostDouble();
    return result;
  }

  result.l = pObjectReference->ToJObject();

  return result;
}

size_t HelperTypes::String16Length( const char16_t *pString )
{
  return std::char_traits<char16_t>::length( pString );
}

size_t HelperTypes::String32Length( const char32_t *pString )
{
  return std::char_traits<char32_t>::length( pString );
}

size_t HelperTypes::WideStringLength( const wchar_t *pString )
{
  return std::char_traits<wchar_t>::length( pString );
}

boost::intrusive_ptr<ObjectReference> HelperTypes::CreateArray( e_JavaArrayTypes type, size_t size )
{
//#if defined( _DEBUG ) && defined (JVMX_LOG_VERBOSE)
//  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
//  pLogger->LogDebug( "Creating array of type: %d with size %d", static_cast<int>( type ), static_cast<int>( size ) );
//#endif // _DEBUG

  std::shared_ptr<IGarbageCollector> pGC = GlobalCatalog::GetInstance().Get( "GarbageCollector" );

  JavaArray *pObjectMemory = reinterpret_cast<JavaArray *>( pGC->AllocateArray( sizeof( JavaArray ) + JavaArray::CalculateSizeInBytes( type, size ) ) );
  JavaArray *pArray = new ( pObjectMemory ) JavaArray( type, size );


  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  boost::intrusive_ptr<ObjectReference> ref = new ObjectReference( pObjectRegistry->AddObject( pArray ) );
  pGC->AddRecentAllocation( ref );

  return ref;
}
