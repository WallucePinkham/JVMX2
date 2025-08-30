#pragma once

#include <cstddef>
#include <memory>

#include <vector>
#include <mutex>

#include <boost/intrusive_ptr.hpp>

#include "GlobalConstants.h"
#include "IJavaVariableType.h"

#include "JavaArrayTypes.h"

#include "JavaTypes.h"

class JavaInteger;
class JavaChar;
class JavaByte;
class JavaBool;
class ObjectReference;
class Lockable;

class JavaArray : public IJavaVariableType
{
public:
    JavaArray(e_JavaArrayTypes type, size_t size);
    virtual ~JavaArray() JVMX_NOEXCEPT;

    void operator delete (void* pObject) JVMX_NOEXCEPT;
    void operator delete (void* pObject, void*) JVMX_NOEXCEPT;

    // Public API (unchanged signatures)
    virtual IJavaVariableType* At(size_t index);
    virtual const IJavaVariableType* At(size_t index) const;
    virtual size_t GetNumberOfElements() const;
    virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
    virtual bool IsReferenceType() const JVMX_OVERRIDE;
    virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
    virtual bool IsNull() const JVMX_OVERRIDE;
    virtual JavaString ToString() const JVMX_OVERRIDE;

    e_JavaArrayTypes GetContainedType() const;

    virtual bool operator==( const IJavaVariableType &other ) const JVMX_OVERRIDE;
    virtual bool operator==( const JavaArray &other ) const;

    virtual bool operator<( const IJavaVariableType &other ) const JVMX_OVERRIDE;
    virtual bool operator<( const JavaArray &other ) const;

    static e_JavaArrayTypes ConvertTypeFromChar( char16_t charType );

    static boost::intrusive_ptr<ObjectReference> CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const char *pBuffer );
    static boost::intrusive_ptr<ObjectReference> CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const uint8_t* pBuffer, size_t length);

    void Initialise();
    virtual IJavaVariableType* GetValueAtIndex(size_t i);
    virtual const IJavaVariableType* GetValueAtIndex(size_t i) const;

    void DebugAssert() const;
    virtual void InternalSetValue(size_t index, const IJavaVariableType* pFinalValue);
    void ValidateIndex(const uint32_t& index) const;

    // Typed accessors made virtual so subclasses can optimize
    virtual JavaChar CharAt(size_t index) const;
    virtual JavaByte ByteAt(size_t index) const;
    virtual JavaBool BoolAt(size_t index) const;

    virtual void SetAt( const JavaInteger &index, const JavaInteger &value );
    void SetAt( const JavaInteger &index, const JavaChar &value );
    void SetAt( const JavaInteger &index, const IJavaVariableType *pValue );
    void SetAt(const JavaInteger& index, const JavaByte& value);
    void SetAt(const JavaInteger& index, const JavaBool& value);
    void SetAt(const JavaInteger& index, JavaLong value);

    void SetAt( const uint32_t &index, const JavaInteger &value );
    void SetAt( const uint32_t &index, const JavaChar &value );
    void SetAt( const uint32_t &index, const IJavaVariableType *pValue );
    void SetAt(const uint32_t& index, const JavaByte& value);
    void SetAt(const uint32_t& index, const JavaBool& value);
    void SetAt(const uint32_t& index, const JavaLong& value);
    void SetAt(const uint32_t& index, const JavaDouble& value);
    void SetAt(const uint32_t& index, const JavaFloat& value);
    void SetAt(const uint32_t& index, const JavaShort& value);

    JavaString ConvertCharArrayToString() const;
    DataBuffer ConvertByteArrayToBuffer() const;
    JavaString ConvertByteArrayToString() const;

    boost::intrusive_ptr<IJavaVariableType> ConvertIntegerTypeForArrayStorage( const JavaInteger &value ) const;
  //boost::intrusive_ptr<IJavaVariableType> ConvertReferenceTypeForArrayStorage( const IJavaVariableType *pValue ) const;

    std::shared_ptr<Lockable> MonitorEnter( const char *pFunctionName );
    void MonitorExit( const char *pFunctionName );

    virtual void CloneOther( const JavaArray *pObjectToClone );

    // ONLY TO BE USED FOR GARBAGE COLLECTION
    void DeepClone( const JavaArray *pObjectToClone );

    static size_t CalculateSizeInBytes(e_JavaArrayTypes type, size_t count);
    static size_t GetSizeOfValueType(e_JavaArrayTypes type);

private:
  static bool AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaVariableTypes variableType );
  static bool AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaArrayTypes variableType );

  static bool IsTypeIntegerCompatible( e_JavaArrayTypes variableType );
  static bool IsTypeIntegerCompatible( e_JavaVariableTypes variableType );

  static bool IsVariableOfReferenceType( e_JavaVariableTypes variableType );



private:
    e_JavaArrayTypes m_ContainedType;
    size_t m_Size;
    char m_pValues[1] = { 0 };
    std::shared_ptr<Lockable> m_pMonitor;

#ifdef _DEBUG
    size_t debugInitialLength;
#endif

};
