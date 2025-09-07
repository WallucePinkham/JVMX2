
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

JavaArray::JavaArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ e_JavaArrayTypes type, size_t size)
  : m_ContainedType(type)
  , m_Size(size)
  , m_pMonitor(new Lockable)
  //, m_pValues( size, TypeParser::GetDefaultValue( type ) )
#ifdef _DEBUG
  , debugInitialLength(size)
#endif // _DEBUG
{
  Initialise();

  DebugAssert();
}

size_t JavaArray::CalculateSizeInBytes(e_JavaArrayTypes type, size_t count)
{
  size_t sizeOfType = GetSizeOfValueType(type);
  return sizeOfType * count;
}

size_t JavaArray::GetSizeOfValueType(e_JavaArrayTypes type)
{
  switch (type)
  {
  case e_JavaArrayTypes::Boolean:
    return sizeof(uint8_t);
    break;

  case e_JavaArrayTypes::Char:
    return sizeof(char16_t);
    break;

  case e_JavaArrayTypes::Float:
    return sizeof(float);
    break;

  case e_JavaArrayTypes::Double:
    return sizeof(double);
    break;

  case e_JavaArrayTypes::Byte:
    return sizeof(uint8_t);
    break;

  case e_JavaArrayTypes::Short:
    return sizeof(int16_t);
    break;

  case e_JavaArrayTypes::Integer:
    return sizeof(int32_t);
    break;

  case e_JavaArrayTypes::Long:
    return sizeof(int64_t);
    break;

  case e_JavaArrayTypes::Reference:
    return sizeof(ObjectReference);
    break;

  default:
    throw InvalidStateException(__FUNCTION__ " - Unknown type.");
    break;
  }

  return 0;
}

JavaArray::~JavaArray()
{

  for (size_t i = 0; i < m_Size; ++i)
  {
    if (m_ContainedType == e_JavaArrayTypes::Reference)
    {
      IJavaVariableType* pValue = GetValueAtIndex(i);
      pValue->~IJavaVariableType();
    }
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

void JavaArray::operator delete (void* pObject) throw()
{
  //ObjectFactory::FreeArray( pObject );
}

void JavaArray::operator delete (void* pObject, void*) throw()
{
  //ObjectFactory::FreeArray( pObject );
}

e_JavaVariableTypes JavaArray::GetVariableType() const
{
  DebugAssert();
  return e_JavaVariableTypes::Array;
}

bool JavaArray::operator==(const JavaArray& other) const
{
  DebugAssert();
  return m_ContainedType == other.m_ContainedType &&
        m_Size == other.m_Size &&
        memcmp(m_pValues, other.m_pValues, CalculateSizeInBytes(m_ContainedType, m_Size));
}

bool JavaArray::operator==(const IJavaVariableType& other) const
{
  DebugAssert();

  if (GetVariableType() != other.GetVariableType())
  {
    return false;
  }

  JVMX_ASSERT(nullptr != dynamic_cast<const ObjectReference*>(&other));
  return *this == *(dynamic_cast<const ObjectReference*>(&other)->GetContainedArray());
}



IJavaVariableType* JavaArray::At(size_t index)
{
#ifdef _DEBUG
  if (m_ContainedType != e_JavaArrayTypes::Reference)
  {
    throw InvalidStateException(__FUNCTION__ " - Cannot use At() for arrays of primitive types. Use CharAt()/ByteAt()/etc. instead.");
  }
#endif

  DebugAssert();

  ValidateIndex(index);

  DebugAssert();

  return GetValueAtIndex(index);
}

const IJavaVariableType* JavaArray::At(size_t index) const
{
  return const_cast<JavaArray*>(this)->At(index);
}

size_t JavaArray::GetNumberOfElements() const
{
  //if ( m_Values.size( ) != debugInitialLength ) __asm int 3;
  DebugAssert();

  return m_Size;
}

bool JavaArray::operator<(const IJavaVariableType& other) const
{
  if (GetVariableType() == other.GetVariableType())
  {
    JVMX_ASSERT(nullptr != dynamic_cast<const ObjectReference*>(&other));
    return *this < *(dynamic_cast<const ObjectReference*>(&other)->GetContainedArray());
  }

  DebugAssert();

  return false;
}

bool JavaArray::operator<(const JavaArray& other) const
{
  DebugAssert();

  if (m_ContainedType < other.m_ContainedType)
  {
    return true;
  }

  for (size_t i = 0; i < m_Size && i < other.m_Size; ++i)
  {
    //if ( *reinterpret_cast<IJavaVariableType *> (m_pValues[i * GetSizeOfValueType( m_ContainedType )]) < *reinterpret_cast<IJavaVariableType *> (other.m_pValues[ i * GetSizeOfValueType( m_ContainedType ) ] ) )

    if (*At(i) < *other.At(i))
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

void JavaArray::SetAt(const JavaInteger& index, const JavaInteger& value)
{
  DebugAssert();

  SetAt(index.ToHostInt32(), value);

  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaChar& value)
{
  DebugAssert();

  SetAt(index.ToHostInt32(), value);

  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaByte& value)
{
  DebugAssert();

  SetAt(index.ToHostInt32(), value);

  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaBool& pValue)
{
  DebugAssert();
  SetAt(index.ToHostInt32(), pValue);
  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, JavaLong value)
{
  DebugAssert();
  SetAt(index.ToHostInt32(), value);
  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaFloat& value)
{
  DebugAssert();
  SetAt(index.ToHostInt32(), value);
  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaDouble& value)
{
  DebugAssert();
  SetAt(index.ToHostInt32(), value);
  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const JavaShort& value)
{
  DebugAssert();
  SetAt(index.ToHostInt32(), value);
  DebugAssert();
}

void JavaArray::SetAt(const JavaInteger& index, const IJavaVariableType* pValue)
{
  DebugAssert();

  SetAt(index.ToHostInt32(), pValue);

  DebugAssert();
}

bool IsIntegerCompatible(e_JavaArrayTypes type)
{
  switch (type)
  {
  case e_JavaArrayTypes::Boolean:
  case e_JavaArrayTypes::Byte:
  case e_JavaArrayTypes::Short:
  case e_JavaArrayTypes::Integer:
  case e_JavaArrayTypes::Long:
  case e_JavaArrayTypes::Char:
    return true;
  default:
    return false;
  }
}

e_JavaArrayTypes VariableTypeToArrayType(e_JavaVariableTypes type)
{
  switch (type)
  {
  case e_JavaVariableTypes::Bool:
    return e_JavaArrayTypes::Boolean;
  case e_JavaVariableTypes::Char:
    return e_JavaArrayTypes::Char;
  case e_JavaVariableTypes::Float:
    return e_JavaArrayTypes::Float;
  case e_JavaVariableTypes::Double:
    return e_JavaArrayTypes::Double;
  case e_JavaVariableTypes::Byte:
    return e_JavaArrayTypes::Byte;
  case e_JavaVariableTypes::Short:
    return e_JavaArrayTypes::Short;
  case e_JavaVariableTypes::Integer:
    return e_JavaArrayTypes::Integer;
  case e_JavaVariableTypes::Long:
    return e_JavaArrayTypes::Long;
  case e_JavaVariableTypes::Object:
  case e_JavaVariableTypes::Array:
  case e_JavaVariableTypes::ClassReference:
  case e_JavaVariableTypes::NullReference:
    return e_JavaArrayTypes::Reference;
  default:
    throw UnsupportedTypeException(__FUNCTION__ " - Unsupported variable type.");
  }
}

e_JavaVariableTypes ArrayTypeToVariableType(e_JavaArrayTypes type)
{
  switch (type)
  {
  case e_JavaArrayTypes::Boolean:
    return e_JavaVariableTypes::Bool;
  case e_JavaArrayTypes::Char:
    return e_JavaVariableTypes::Char;
  case e_JavaArrayTypes::Float:
    return e_JavaVariableTypes::Float;
  case e_JavaArrayTypes::Double:
    return e_JavaVariableTypes::Double;
  case e_JavaArrayTypes::Byte:
    return e_JavaVariableTypes::Byte;
  case e_JavaArrayTypes::Short:
    return e_JavaVariableTypes::Short;
  case e_JavaArrayTypes::Integer:
    return e_JavaVariableTypes::Integer;
  case e_JavaArrayTypes::Long:
    return e_JavaVariableTypes::Long;
  case e_JavaArrayTypes::Reference:
    return e_JavaVariableTypes::Object;
  default:
    throw UnsupportedTypeException(__FUNCTION__ " - Unsupported array type.");
  }
}

void JavaArray::SetAt(const uint32_t& index, const JavaInteger& value)
{
  if (!AreTypesCompatible(m_ContainedType, e_JavaArrayTypes::Integer))
  {
    throw InvalidArgumentException(__FUNCTION__ " - Array Types are not compatible.");
  }

  ValidateIndex(index);

  if (m_ContainedType != VariableTypeToArrayType(value.GetVariableType()) &&
    ::IsIntegerCompatible(m_ContainedType))
  {
    auto pInteger = TypeParser::UpCastToInteger(&value);
    auto result = TypeParser::DownCastFromInteger(pInteger, ArrayTypeToVariableType(m_ContainedType));

    if (m_ContainedType == e_JavaArrayTypes::Char)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaChar>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Byte)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaByte>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Float)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaFloat>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Double)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaDouble>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Long)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaLong>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Boolean)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaBool>(result)));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Short)
    {
      SetAt(index, *(boost::dynamic_pointer_cast<JavaShort>(result)));
    }
    else
    {
      throw InvalidStateException(__FUNCTION__ " - Unknown type.");
    }
    //else
    //{
    //  char* pCharValue = m_pValues + sizeof(int32_t) * index;
    //  reinterpret_cast<int32_t*>(pCharValue)[0] = value.ToHostInt32();
    //}
  }
  else
  {
    char* pCharValue = m_pValues + sizeof(int32_t) * index;
    reinterpret_cast<int32_t*>(pCharValue)[0] = value.ToHostInt32();
  }
}

void JavaArray::ValidateIndex(const uint32_t& index) const
{
  if (index < 0)
  {
    throw IndexOutOfBoundsException(__FUNCTION__ " - Invalid index passed in. Less than zero.");
  }

  if (index > m_Size)
  {
    throw IndexOutOfBoundsException(__FUNCTION__ " - Invalid index passed in.");
  }
}

void JavaArray::SetAt(const uint32_t& index, const JavaChar& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Char)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add characters to an array that does not contain characters.");
  }

  ValidateIndex(index);

  char* pCharValue = m_pValues + sizeof(char16_t) * index;
  reinterpret_cast<char16_t*>(pCharValue)[0] = value.ToChar16();

  //InternalSetValue( index, &value );
}

void JavaArray::SetAt(const uint32_t& index, const JavaByte& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Byte)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add byte to an array that does not contain bytes.");
  }

  ValidateIndex(index);

  char* pCharValue = m_pValues + sizeof(uint8_t) * index;
  reinterpret_cast<uint8_t*>(pCharValue)[0] = value.ToHostInt8();
}

void JavaArray::SetAt(const uint32_t& index, const JavaBool& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Boolean)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add boolean to an array that does not contain booleans.");
  }

  ValidateIndex(index);
  char* pCharValue = m_pValues + sizeof(uint8_t) * index;
  reinterpret_cast<uint8_t*>(pCharValue)[0] = value.ToBool() ? 1 : 0;
}

void JavaArray::SetAt(const uint32_t& index, const JavaFloat& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Float)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add float to an array that does not contain floats.");
  }

  ValidateIndex(index);

  char* pCharValue = m_pValues + sizeof(float) * index;
  *reinterpret_cast<float*>(pCharValue) = value.ToHostFloat();
}

void JavaArray::SetAt(const uint32_t& index, const JavaDouble& value)
{
  if (m_ContainedType != e_JavaArrayTypes::Double)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add double to an array that does not contain doubles.");
  }

  ValidateIndex(index);

  char* pCharValue = m_pValues + sizeof(double) * index;
  *reinterpret_cast<double*>(pCharValue) = value.ToHostDouble();
}

void JavaArray::SetAt(const uint32_t& index, JavaLong value)
{
  if (m_ContainedType != e_JavaArrayTypes::Long)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add long to an array that does not contain longs.");
  }

  ValidateIndex(index);

  char* pCharValue = m_pValues + sizeof(int64_t) * index;
  *reinterpret_cast<int64_t*>(pCharValue) = value.ToHostInt64();
}

void JavaArray::SetAt(const uint32_t& index, JavaShort value)
{
  if (m_ContainedType != e_JavaArrayTypes::Short)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add short to an array that does not contain shorts.");
  }

  ValidateIndex(index);
  
  char* pCharValue = m_pValues + sizeof(int16_t) * index;
  *reinterpret_cast<int16_t*>(pCharValue) = value.ToHostInt16();
}

void JavaArray::SetAt(const uint32_t& index, const IJavaVariableType* pValue)
{
  if (!AreTypesCompatible(m_ContainedType, pValue->GetVariableType()))
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to add array element types are incompatible.");
  }

  ValidateIndex(index);

  if (GetValueAtIndex(0)->GetVariableType() != pValue->GetVariableType() &&
      GetValueAtIndex(0)->IsIntegerCompatible() && pValue->IsIntegerCompatible())
  {
    auto pInteger = TypeParser::UpCastToInteger(pValue);
    auto result = TypeParser::DownCastFromInteger(pInteger, GetValueAtIndex(0)->GetVariableType());

    InternalSetValue(index, result.get());
  }
  else
  {
    InternalSetValue(index, pValue);
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
  return JavaString::FromArray(*this);
}

JavaString JavaArray::ConvertByteArrayToString() const
{
  if (m_ContainedType != e_JavaArrayTypes::Byte)
  {
    throw InvalidStateException(__FUNCTION__ " - Array does not contain Bytes.");
  }

  char* pBuffer = new char[m_Size + 1]; // +1 for terminator

  try
  {
    int i = 0;

    //for ( auto it = m_pValues.begin(); it != m_pValues.end(); ++ it )
    for (size_t index = 0; index < m_Size; ++index)
    {
      char chr = static_cast<char>(ByteAt(index).ToHostInt8());
      pBuffer[i++] = chr;

      // Exit after we have appended the null character.
      if ('\0' == chr)
      {
        break;
      }
    }

    pBuffer[i] = '\0';

    JavaString result = JavaString::FromCString(pBuffer);

    delete[] pBuffer;
    pBuffer = nullptr;

    return result;
  }
  catch (...)
  {
    delete[] pBuffer;
  }

  return JavaString::EmptyString();
}

void JavaArray::Initialise()
{
  DebugAssert();

  size_t sizeOfValue = GetSizeOfValueType(m_ContainedType);

  for (size_t i = 0; i < m_Size; ++i)
  {
    //*pValue = *TypeParser::GetDefaultValue( m_ContainedType );

    switch (m_ContainedType)
    {
    case e_JavaArrayTypes::Boolean:
    {
      char* pCharValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<uint8_t*>(pCharValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Char:
    {
      char* pCharValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<char16_t*>(pCharValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Float:
    {
      char* pFloatValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<float*>(pFloatValue)[0] = 0.0f;
    }
    break;

    case e_JavaArrayTypes::Double:
    {
      char* pDoubleValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<double*>(pDoubleValue)[0] = 0.0;
    }
    break;

    case e_JavaArrayTypes::Byte:
    {
      char* pCharValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<uint8_t*>(pCharValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Short:
    {
      char* pValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<int16_t*>(pValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Integer:
    {
      char* pValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<int32_t*>(pValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Long:
    {
      char* pValue = m_pValues + sizeOfValue * i;
      reinterpret_cast<int64_t*>(pValue)[0] = 0;
    }
    break;

    case e_JavaArrayTypes::Reference:
    {
      IJavaVariableType* pValue = GetValueAtIndex(i);
      new (pValue) ObjectReference(nullptr);
    }
    break;

    default:
      throw InvalidArgumentException(__FUNCTION__ " - Unknown type.");
      break;
    }
  }

  DebugAssert();
}

IJavaVariableType* JavaArray::GetValueAtIndex(size_t i)
{
  if (m_ContainedType != e_JavaArrayTypes::Reference)
  {
    throw InvalidStateException(__FUNCTION__ " - Cannot use GetValueAtIndex() for arrays of primitive types. Use CharAt()/ByteAt()/BoolAt()/etc. instead.");
  }

  char* pValue = m_pValues + (GetSizeOfValueType(m_ContainedType) * i);
  JVMX_ASSERT(pValue >= m_pValues && pValue < m_pValues + CalculateSizeInBytes(m_ContainedType, m_Size));
  return reinterpret_cast<IJavaVariableType*>(pValue);
}

const IJavaVariableType* JavaArray::GetValueAtIndex(size_t i) const
{
  if (m_ContainedType != e_JavaArrayTypes::Reference)
  {
    throw InvalidStateException(__FUNCTION__ " - Cannot use GetValueAtIndex() for arrays of primitive types. Use CharAt()/ByteAt()/BoolAt()/FloatAt()/DoubleAt() instead.");
  }

  const char* pValue = m_pValues + (GetSizeOfValueType(m_ContainedType) * i);
  JVMX_ASSERT(pValue >= m_pValues && pValue < m_pValues + CalculateSizeInBytes(m_ContainedType, m_Size));

  return reinterpret_cast<const IJavaVariableType*>(pValue);
}

std::shared_ptr<Lockable> JavaArray::MonitorEnter(const char* pFunctionName)
{
  m_pMonitor->Lock(pFunctionName);
  return m_pMonitor;
}

void JavaArray::MonitorExit(const char* pFunctionName)
{
  m_pMonitor->Unlock(pFunctionName);
}

void JavaArray::CloneOther(const JavaArray* pObjectToClone)
{
  JVMX_ASSERT(m_ContainedType == pObjectToClone->m_ContainedType);

  if (m_ContainedType != pObjectToClone->m_ContainedType)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Contained types do not match.");
  }

  JVMX_ASSERT(pObjectToClone->m_Size == m_Size);

  for (uint32_t i = 0; i < pObjectToClone->GetNumberOfElements(); ++i)
  {
    if (m_ContainedType == e_JavaArrayTypes::Char)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->CharAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Byte)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->ByteAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Boolean)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->BoolAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Float)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->FloatAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Double)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->DoubleAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Long)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->LongAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Short)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->ShortAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Integer)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->IntegerAt(i));
    }
    else if (m_ContainedType == e_JavaArrayTypes::Reference)
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->At(i));
    }
    else
    {
      SetAt(JavaInteger::FromHostInt32(i), pObjectToClone->At(i));
    }

  }
}

void JavaArray::DeepClone(const JavaArray* pObjectToClone)
{
  CloneOther(pObjectToClone);

  m_pMonitor = pObjectToClone->m_pMonitor;
}

bool JavaArray::AreTypesCompatible(e_JavaArrayTypes arrayType, e_JavaVariableTypes variableType)
{
  switch (arrayType)
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
    return IsTypeIntegerCompatible(variableType);
    break;

  case e_JavaArrayTypes::Long:
    return variableType == e_JavaVariableTypes::Long;
    break;

  case e_JavaArrayTypes::Reference:
    return IsVariableOfReferenceType(variableType);
    break;

  default:
    break;
  }

  return false;
}

bool JavaArray::AreTypesCompatible(e_JavaArrayTypes arrayType, e_JavaArrayTypes variableType)
{
  switch (arrayType)
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
    return IsTypeIntegerCompatible(variableType);

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

bool JavaArray::IsVariableOfReferenceType(e_JavaVariableTypes variableType)
{
  return (e_JavaVariableTypes::Object == variableType) || (e_JavaVariableTypes::Array == variableType) || (e_JavaVariableTypes::ClassReference == variableType) || (e_JavaVariableTypes::NullReference == variableType) || (e_JavaVariableTypes::ReturnAddress == variableType);
}

DataBuffer JavaArray::ConvertByteArrayToBuffer() const
{
  if (m_ContainedType != e_JavaArrayTypes::Byte)
  {
    throw InvalidStateException(__FUNCTION__ " - Array does not contain Bytes.");
  }

  DataBuffer result = DataBuffer::EmptyBuffer();
  for (size_t i = 0; i < m_Size; ++i)
  {
    uint8_t byteValue = ByteAt(i).ToHostInt8();
    result = result.AppendUint8(byteValue);
  }

  return result;
}

e_JavaArrayTypes JavaArray::ConvertTypeFromChar(char16_t charType)
{
  switch (charType)
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
    throw UnsupportedTypeException(__FUNCTION__ " - Unknown type found.");
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

  char buffer[38] = { 0 };
  _snprintf(buffer, 37, "(%d)", m_Size);

  std::basic_stringstream<char16_t> outputStream;

  outputStream << buffer << u"[";

  for (size_t i = 0; i < m_Size; ++i)
  {
    if (m_ContainedType == e_JavaArrayTypes::Char)
    {
      outputStream << CharAt(i).ToString().ToCharacterArray();
    }
    else if (m_ContainedType == e_JavaArrayTypes::Byte)
    {
      outputStream << ByteAt(i).ToString().ToCharacterArray();
    }
    else
    {
      const IJavaVariableType* pValue = GetValueAtIndex(i);
      outputStream << pValue->ToString().ToCharacterArray();
    }

    if (i != m_Size)
    {
      outputStream << u", ";
    }

    if (count > 20)
    {
      outputStream << u"...";
      break;
    }

    ++count;
  }

  DebugAssert();

  outputStream << u"]";
  return JavaString::FromCString(outputStream.str().c_str());
}

void JavaArray::DebugAssert() const
{
#ifdef _DEBUG
  //if ( m_Values.size() != debugInitialLength ) __asm int 3;
  JVMX_ASSERT(m_Size == debugInitialLength);
#endif // _DEBUG
}

void JavaArray::InternalSetValue(size_t index, const IJavaVariableType* pFinalValue)
{
  IJavaVariableType* pValue = GetValueAtIndex(index);
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

bool JavaArray::IsTypeIntegerCompatible(e_JavaArrayTypes variableType)
{
  return variableType == e_JavaArrayTypes::Short || variableType == e_JavaArrayTypes::Integer || variableType == e_JavaArrayTypes::Char || variableType == e_JavaArrayTypes::Boolean || variableType == e_JavaArrayTypes::Byte;
}

bool JavaArray::IsTypeIntegerCompatible(e_JavaVariableTypes variableType)
{
  return variableType == e_JavaVariableTypes::Short || variableType == e_JavaVariableTypes::Integer || variableType == e_JavaVariableTypes::Char || variableType == e_JavaVariableTypes::Bool || variableType == e_JavaVariableTypes::Byte;
}

boost::intrusive_ptr<IJavaVariableType> JavaArray::ConvertIntegerTypeForArrayStorage(const JavaInteger& value) const
{
  boost::intrusive_ptr<IJavaVariableType> pFinalValue = nullptr;

  switch (m_ContainedType)
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
    pFinalValue = new JavaInteger(value);
    break;

  default:
    JVMX_ASSERT(false);
    throw InvalidArgumentException(__FUNCTION__ " - Array Types are not compatible. This should have been caught before.");
    break;
  }

  return pFinalValue;
}


boost::intrusive_ptr<ObjectReference> JavaArray::CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const char* pBuffer)
{
  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get("ThreadManager");

  size_t length = strlen(pBuffer);
  boost::intrusive_ptr<ObjectReference> pResult = pThreadManager->GetCurrentThreadState()->CreateArray(e_JavaArrayTypes::Char, length);
  //boost::intrusive_ptr<ObjectReference> pResult = new JavaArray( /*pMemoryManager, */e_JavaArrayTypes::Char, length );
  for (size_t i = 0; i < length; ++i)
  {
    pResult->GetContainedArray()->SetAt(static_cast<uint32_t>(i), JavaChar::FromCChar(pBuffer[i]));
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

JavaChar JavaArray::CharAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Char)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a character from an array that does not contain characters.");
  }

  const char* charPtr = m_pValues + (sizeof(char16_t) * index);
  return JavaChar::FromChar16(*reinterpret_cast<const char16_t*>(charPtr));
}

JavaByte JavaArray::ByteAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Byte)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a byte from an array that does not contain bytes.");
  }

  const char* bytePtr = m_pValues + (sizeof(uint8_t) * index);
  return JavaByte::FromHostInt8(*reinterpret_cast<const uint8_t*>(bytePtr));
}

JavaBool JavaArray::BoolAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Boolean)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a boolean from an array that does not contain booleans.");
  }

  const char* boolPtr = m_pValues + (sizeof(uint8_t) * index);
  return JavaBool::FromBool(*reinterpret_cast<const uint8_t*>(boolPtr) > 0 ? true : false);
}

JavaFloat JavaArray::FloatAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Float)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a float from an array that does not contain floats.");
  }

  const char* floatPtr = m_pValues + (sizeof(float) * index);
  return JavaFloat::FromHostFloat(*reinterpret_cast<const float*>(floatPtr));
}

JavaDouble JavaArray::DoubleAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Double)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a double from an array that does not contain doubles.");
  }

  const char* doublePtr = m_pValues + (sizeof(double) * index);
  return JavaDouble::FromHostDouble(*reinterpret_cast<const double*>(doublePtr));
}

JavaInteger JavaArray::IntegerAt(size_t index) const
{
  ValidateIndex(index);

  if (m_ContainedType != e_JavaArrayTypes::Integer)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get an integer from an array that does not contain integers.");
  }

  const char* intPtr = m_pValues + (sizeof(int32_t) * index);
  return JavaInteger::FromHostInt32(*reinterpret_cast<const int32_t*>(intPtr));
}

JavaLong JavaArray::LongAt(size_t index) const
{
  ValidateIndex(index);
  if (m_ContainedType != e_JavaArrayTypes::Long)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a long from an array that does not contain longs.");
  }
  const char* longPtr = m_pValues + (sizeof(int64_t) * index);
  return JavaLong::FromHostInt64(*reinterpret_cast<const int64_t*>(longPtr));
}

JavaShort JavaArray::ShortAt(size_t index) const
{
  ValidateIndex(index);
  if (m_ContainedType != e_JavaArrayTypes::Short)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to get a short from an array that does not contain shorts.");
  }
  const char* shortPtr = m_pValues + (sizeof(int16_t) * index);
  return JavaShort::FromHostInt16(*reinterpret_cast<const int16_t*>(shortPtr));
}
