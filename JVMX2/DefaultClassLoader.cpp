
#include <memory>
#include <utility>

#include "GlobalConstants.h"

#include "FileInvalidException.h"
#include "UnsupportedTypeException.h"

#include "ConstantPool.h"
#include "CodeAttributeFactory.h"

#include "InterfaceInfo.h"

#include "JavaTypes.h"
#include "Stream.h"

#include "DefaultClassLoader.h"
#include "ClassFactory.h"
#include "HelperConversion.h"

DefaultClassLoader::DefaultClassLoader()
  : m_pAttributeFactory( new CodeAttributeFactory )
  , m_fileStream( Stream::EmptyStream() )
{}

DefaultClassLoader::~DefaultClassLoader() JVMX_NOEXCEPT
{
  delete m_pAttributeFactory;
}

std::shared_ptr<JavaClass> DefaultClassLoader::LoadClass( const JVMX_CHAR_TYPE *pFileName, const JavaString &path )
{
  //m_SearchPaths.push_back(path.ToUtf8String());
    AddSearchPath(path);
    return LoadClass( ReadFileIntoStream( pFileName ) );
}

void DefaultClassLoader::Open( const JVMX_CHAR_TYPE *pFileName )
{
  FileLoader::Open( pFileName );
}

uint32_t DefaultClassLoader::ReadMagicNumber()
{
  uint32_t magicNumber = m_fileStream.ReadUint32();

  //   if ( magicNumber != c_JavaClassFileMagicNumber )
  //   {
  //     throw FileInvalidException( __FUNCTION__ " - Magic Number Check Failed." );
  //   }

  return magicNumber;
}

uint16_t DefaultClassLoader::ReadMinorVersion()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read minor version number." );
  }
}

uint16_t DefaultClassLoader::ReadMajorVersion()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read major version number." );
  }
}

uint16_t DefaultClassLoader::ReadConstantPoolCount()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not constant pool count." );
  }
}

std::shared_ptr<ConstantPool> DefaultClassLoader::ReadConstantPoolTable( size_t poolCount )
{
  size_t nIndex = 0; // For Debugging
  std::shared_ptr<ConstantPool> pPool = std::make_shared<ConstantPool>();

  for ( nIndex = 0; nIndex < poolCount - 1; ++nIndex )
  {
    uint8_t tag = ReadNextConstantTag();
    switch ( tag )
    {
      case e_ConstantPoolEntryTypeString:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadString() ) );
        break;

      case e_ConstantPoolEntryTypeInteger:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadInteger() ) );
        break;

      case e_ConstantPoolEntryTypeFloat:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadFloat() ) );
        break;

      case e_ConstantPoolEntryTypeDouble:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadDouble() ) );
        ++nIndex; // (longs and doubles) take up two consecutive slots in the table, although the second
        // such slot is a phantom index that is never directly used.
        break;

      case e_ConstantPoolEntryTypeLong:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadLong() ) );
        ++nIndex; // (longs and doubles) take up two consecutive slots in the table, although the second
        // such slot is a phantom index that is never directly used.
        break;

      case e_ConstantPoolEntryTypeClassReference:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadClassReference() ) );
        break;

      case e_ConstantPoolEntryTypeStringReference:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadStringReference() ) );
        break;

      case e_ConstantPoolEntryTypeFieldReference:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadFieldReference() ) );
        break;

      case e_ConstantPoolEntryTypeMethodReference:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadMethodReference() ) );
        break;

      case e_ConstantPoolEntryTypeInterfaceMethodReference:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadInterFaceMethodReference() ) );
        break;

      case e_ConstantPoolEntryTypeNameAndTypeDescriptor:
        pPool->AddConstant( std::make_shared<ConstantPoolEntry>( ReadNameAndTypeDescriptor() ) );
        break;

      case e_ConstantPoolEntryTypeInvokeDynamicInfo:
        pPool->AddConstant(std::make_shared<ConstantPoolEntry>(ReadInvokeDynamic()));
        break;

      case e_ConstantPoolEntryTypeMethodHandle:
        pPool->AddConstant(std::make_shared<ConstantPoolEntry>(ReadMethodHandle()));
        break;

      case e_ConstantPoolEntryTypeMethodType:
        pPool->AddConstant(std::make_shared<ConstantPoolEntry>(ReadMethodType()));
        break;

      default:
        throw UnsupportedTypeException( __FUNCTION__ " - Unsupported tag type read from constant pool." );
        break;
    }
  }

  return pPool;
}

uint8_t DefaultClassLoader::ReadNextConstantTag()
{
  return m_fileStream.ReadUint8();
}

JavaString DefaultClassLoader::ReadString()
{
  uint16_t length = ReadStringLength();
  if ( 0 == length )
  {
    return JavaString::EmptyString();
  }

  DataBuffer buffer = ReadBuffer( length );
  JavaString result = JavaString::FromUtf8ByteArray( buffer.GetByteLength(), buffer.ToByteArray() );

  return result;
}

uint16_t DefaultClassLoader::ReadStringLength()
{
  return m_fileStream.ReadUint16();
}

JavaInteger DefaultClassLoader::ReadInteger()
{
  //return JavaInteger::FromNetworkInt32( m_fileStream.ReadUint32() );
  return JavaInteger::FromHostInt32( m_fileStream.ReadUint32() );
}

JavaLong DefaultClassLoader::ReadLong()
{
  return JavaLong::FromHostInt64( m_fileStream.ReadUint64() );
  //return JavaLong::FromNetworkInt64( m_fileStream.ReadUint64() );
}

JavaFloat DefaultClassLoader::ReadFloat()
{
  uint32_t bytesRead = m_fileStream.ReadUint32();

  return JavaFloat::FromHostFloat( *reinterpret_cast<float *>(&bytesRead) );
}

JavaDouble DefaultClassLoader::ReadDouble()
{
  uint64_t bytesRead = m_fileStream.ReadUint64();

  return JavaDouble::FromHostDouble( *reinterpret_cast<double *>(&bytesRead) );
}

std::shared_ptr<ConstantPoolClassReference> DefaultClassLoader::ReadClassReference()
{
  return ConstantPoolClassReference::FromConstantPoolIndex( m_fileStream.ReadUint16() );
}

std::shared_ptr<ConstantPoolStringReference> DefaultClassLoader::ReadStringReference()
{
  ConstantPoolIndex result = ReadIndex();

  return ConstantPoolStringReference::FromConstantPoolIndex( result );
}

std::shared_ptr<ConstantPoolFieldReference> DefaultClassLoader::ReadFieldReference()
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = ReadClassReference();
  ConstantPoolIndex nameAndTypeIndex = ReadIndex();

  return std::make_shared<ConstantPoolFieldReference>( pClassRef, nameAndTypeIndex );
}

std::shared_ptr<ConstantPoolMethodReference> DefaultClassLoader::ReadMethodReference()
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = ReadClassReference();
  ConstantPoolIndex nameAndTypeIndex = ReadIndex();

  return std::make_shared<ConstantPoolMethodReference>( pClassRef, nameAndTypeIndex );
}

std::shared_ptr<ConstantPoolInterfaceMethodReference> DefaultClassLoader::ReadInterFaceMethodReference()
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = ReadClassReference();
  ConstantPoolIndex nameAndTypeIndex = ReadIndex();

  return std::make_shared<ConstantPoolInterfaceMethodReference>( pClassRef, nameAndTypeIndex );
}

std::shared_ptr<ConstantPoolNameAndTypeDescriptor> DefaultClassLoader::ReadNameAndTypeDescriptor()
{
  ConstantPoolIndex nameIndex = ReadIndex();
  ConstantPoolIndex typeIndex = ReadIndex();

  return std::make_shared<ConstantPoolNameAndTypeDescriptor>( nameIndex, typeIndex );
}

std::shared_ptr<ConstantPoolInvokeDynamic> DefaultClassLoader::ReadInvokeDynamic()
{
  ConstantPoolIndex bootstrapMethodAttrIndex = ReadIndex();
  ConstantPoolIndex nameAndTypeIndex = ReadIndex();

  return std::make_shared<ConstantPoolInvokeDynamic>(bootstrapMethodAttrIndex, nameAndTypeIndex);
}

std::shared_ptr<ConstantPoolMethodHandle> DefaultClassLoader::ReadMethodHandle()
{
  uint8_t referenceKind = m_fileStream.ReadUint8();
  ConstantPoolIndex referenceIndex = ReadIndex();

  return std::make_shared<ConstantPoolMethodHandle>(referenceKind, referenceIndex);
}

std::shared_ptr<ConstantPoolMethodType> DefaultClassLoader::ReadMethodType()
{
  ConstantPoolIndex descriptorIndex = ReadIndex();

  return std::make_shared<ConstantPoolMethodType>(descriptorIndex);
}

ConstantPoolIndex DefaultClassLoader::ReadIndex()
{
  return (ConstantPoolIndex)m_fileStream.ReadUint16();
}

//uint16_t DefaultClassLoader::ReadUint16()
//{
//  return m_fileStream.ReadUint16();
//}
//
//uint32_t DefaultClassLoader::ReadUint32()
//{
//  return m_fileStream.ReadUint32();
//}

uint16_t DefaultClassLoader::ReadAccessFlags()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read access flags. Possible end of file." );
  }
}

// JavaClassFile DefaultClassLoader::CreateClassFile( uint16_t minorVersion, uint16_t majorVersion, ConstantPool *pConstantPool, uint16_t accessFlags, ConstantPoolIndex thisClassIndex, ConstantPoolIndex superIndex, InterfaceInfoList interfaces, FieldInfoList fields, MethodInfoList methods, CodeAttributeList attributes )
// {
//   return JavaClassFile( minorVersion, majorVersion, pConstantPool, accessFlags, thisClassIndex, superIndex, std::move( interfaces ), std::move( fields ), std::move( methods ), std::move( attributes ) );
// }

uint16_t DefaultClassLoader::ReadInterfaceCount()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read interface count. Possible end of file." );
  }
}

InterfaceInfoList DefaultClassLoader::ReadInterfaces( const ConstantPool &pPool, size_t count )
{
  InterfaceInfoList result;
  for ( size_t i = 0; i < count; ++i )
  {
    ConstantPoolIndex index = ReadIndex();
    std::shared_ptr<ConstantPoolClassReference> classRef = pPool.GetConstant( index )->AsClassReferencePointer();

    result.push_back( InterfaceInfo( classRef->GetClassName() ) );
  }

  return result;
}

uint16_t DefaultClassLoader::ReadFieldCount()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read field count. Possible end of file." );
  }
}

FieldInfoList DefaultClassLoader::ReadFields( const ConstantPool &pPool, size_t count )
{
  FieldInfoList result;
  intptr_t offset = 0;
  for ( size_t i = 0; i < count; ++i )
  {
    uint16_t accessFlags = m_fileStream.ReadUint16();
    std::shared_ptr<ConstantPoolNameAndTypeDescriptor> pMethodRef = ReadNameAndTypeDescriptor();
    uint16_t attributeCount = ReadAttributeCount();
    CodeAttributeList attributes = ReadAttributes( pPool, static_cast<size_t>(attributeCount) );

    boost::intrusive_ptr<JavaString> pName = pPool.GetConstant( pMethodRef->GetNameIndex() )->AsString();
    boost::intrusive_ptr<JavaString> pType = pPool.GetConstant( pMethodRef->GetTypeDescriptorIndex() )->AsString();

    auto pNewField = std::make_shared<FieldInfo>( pPool, accessFlags, pName, pType, attributes, offset );
    result.push_back( pNewField );
    if ( !pNewField->IsStatic() )
    {
      offset += pNewField->GetByteSize();
    }
  }

  return result;
}

uint16_t DefaultClassLoader::ReadMethodCount()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read Method count. Possible end of file." );
  }
}

MethodInfoList DefaultClassLoader::ReadMethods( const ConstantPool &pPool, size_t count )
{
  MethodInfoList result;
  for ( size_t i = 0; i < count; ++i )
  {
    uint16_t accessFlags = m_fileStream.ReadUint16();
    std::shared_ptr<ConstantPoolNameAndTypeDescriptor> pMethodRef = ReadNameAndTypeDescriptor();
    uint16_t attributeCount = ReadAttributeCount();
    CodeAttributeList attributes = ReadAttributes( pPool, static_cast<size_t>(attributeCount) );

    boost::intrusive_ptr<JavaString> methodName = pPool.GetConstant( pMethodRef->GetNameIndex() )->AsString();

    result.emplace( std::make_pair<JavaString, std::shared_ptr<MethodInfo> >( JavaString( *methodName ), std::make_shared<MethodInfo>( accessFlags, methodName, pPool.GetConstant( pMethodRef->GetTypeDescriptorIndex() )->AsString(), attributes ) ) );
  }

  return result;
}

uint16_t DefaultClassLoader::ReadAttributeCount()
{
  try
  {
    return m_fileStream.ReadUint16();
  }
  catch ( FileInvalidException & )
  {
    throw FileInvalidException( __FUNCTION__ " - Could not read Attribute count. Possible end of file." );
  }
}

CodeAttributeList DefaultClassLoader::ReadAttributes( const ConstantPool &pPool, size_t count )
{
  CodeAttributeList result;
  for ( size_t i = 0; i < count; ++i )
  {
    ConstantPoolIndex nameIndex = ReadIndex();
    uint32_t length = m_fileStream.ReadUint32();

    boost::intrusive_ptr<JavaString> pName = (pPool.GetConstant( nameIndex ))->AsString();

    DataBuffer buffer = ReadBuffer( length );
    BigEndianStream byteStream = BigEndianStream::FromDataBuffer( buffer );

    result.push_back( m_pAttributeFactory->CreateFromBinary( *pName, byteStream, pPool, length ) );
  }

  return result;
}

// CodeAttributeList DefaultClassLoader::ReadCodeAttributes( const ConstantPool &pPool, size_t count )
// {
//   // TODO: Come back and fix this!!
//   CodeAttributeList result;
//   for ( size_t i = 0; i < count; ++i )
//   {
//     ConstantPoolIndex nameIndex = ReadIndex();
//     uint32_t length = ReadUint32();
//
//     JavaString name = (pPool.GetConstant( nameIndex )).GetString();
//
//     DataBuffer buffer = ReadBuffer( length );
//     BigEndianStream byteStream = BigEndianStream::FromDataBuffer( buffer );
//
//     result.push_back( m_pAttributeFactory->CreateFromBinary( name, byteStream, pPool ) );
//   }
//
//   return result;
// }

DataBuffer DefaultClassLoader::ReadBuffer( size_t length )
{
  return m_fileStream.ReadBytes( length );
}

void DefaultClassLoader::ReplaceAttributeFactory( CodeAttributeFactory *pNewFactory )
{
  delete m_pAttributeFactory;
  m_pAttributeFactory = pNewFactory;
}

Stream DefaultClassLoader::ReadFileIntoStream( const JVMX_CHAR_TYPE *pFileName )
{
  Open( pFileName );

  long fileSize = GetFileSize();
  uint8_t *pBuffer = new uint8_t[ fileSize ];

  try
  {
    FileLoader::ReadBytes( pBuffer, fileSize );
  }
  catch ( ... )
  {
    delete[] pBuffer;
    throw;
  }

  Stream result = Stream::FromByteArray( fileSize, pBuffer );
  delete[] pBuffer;

  Close();

  return result;
}

std::shared_ptr<JavaClass> DefaultClassLoader::LoadClass( BigEndianStream stream, boost::intrusive_ptr<ObjectReference> pClassLoader )
{
  m_fileStream = stream;

  uint32_t magicNumber = ReadMagicNumber();
  if ( magicNumber != c_JavaClassFileMagicNumber )
  {
    throw FileInvalidException( __FUNCTION__ " - The magic number for this file was not correct." );
  }

  uint16_t minorVersion = ReadMinorVersion();
  uint16_t majorVersion = ReadMajorVersion();
  uint16_t constantPoolCount = ReadConstantPoolCount();

  std::shared_ptr<ConstantPool> pConstantPool = ReadConstantPoolTable( constantPoolCount );

// #ifdef _DEBUG
//   FILE *pFile = fopen( "ConstantPool.txt", "a+" );
// 
//   if ( nullptr != pFile )
//   {
//     fwprintf( pFile, L"Class = %s\n", m_FileName.c_str() );
//     fclose( pFile );
//   }
// #endif // _DEBUG

  pConstantPool->Prepare();

  uint16_t accessFlags = ReadAccessFlags();
  ConstantPoolIndex thisClassIndex = ReadIndex();
  ConstantPoolIndex superClassIndex = ReadIndex();
  uint16_t interfaceCount = ReadInterfaceCount();
  InterfaceInfoList interfaces = ReadInterfaces( *pConstantPool, interfaceCount );
  uint16_t fieldCount = ReadFieldCount();
  FieldInfoList fields = ReadFields( *pConstantPool, fieldCount );
  uint16_t methodCount = ReadMethodCount();
  MethodInfoList methods = ReadMethods( *pConstantPool, methodCount );
  uint16_t attributeCount = ReadAttributeCount();
  CodeAttributeList attributes = ReadAttributes( *pConstantPool, attributeCount );

  return ClassFactory::CreateClassFile( minorVersion, majorVersion, pConstantPool, accessFlags, thisClassIndex, superClassIndex, interfaces, fields, methods, attributes, pClassLoader );
}

//uint64_t DefaultClassLoader::ReadUint64()
//{
//  return m_fileStream.ReadUint64();
//}
//
//uint8_t DefaultClassLoader::ReadUint8()
//{
//  return m_fileStream.ReadUint8();
//}
