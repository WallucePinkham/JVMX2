
#ifndef __JAVAFILELOADER_H__
#define __JAVAFILELOADER_H__

#include "GlobalConstants.h"

#include "FileLoader.h"
#include "JavaClass.h"
#include "FileInvalidException.h"
#include "BigEndianStream.h"

class CodeAttributeFactory; // Forward declaration.

class DefaultClassLoader : public FileLoader
{
public:
  DefaultClassLoader();

  virtual ~DefaultClassLoader() JVMX_NOEXCEPT;

  virtual std::shared_ptr<JavaClass> LoadClass( const JVMX_CHAR_TYPE *pFileName, const JavaString &path = JavaString::EmptyString());
  virtual std::shared_ptr<JavaClass> LoadClass( BigEndianStream stream, boost::intrusive_ptr<ObjectReference> pClassLoader = nullptr);

  Stream ReadFileIntoStream( const JVMX_CHAR_TYPE * pFileName );

  // This method is intended for unit testing purposes and shouldn't be called in production code. It is assumed that this class
  // now owns the pointer and the pointer will be deleted.
  virtual void ReplaceAttributeFactory( CodeAttributeFactory *pNewFactory );

private:
  DefaultClassLoader( const DefaultClassLoader & ) JVMX_FN_DELETE;
  DefaultClassLoader( const DefaultClassLoader && ) JVMX_FN_DELETE;

  DefaultClassLoader operator=(const DefaultClassLoader &)JVMX_FN_DELETE;
  DefaultClassLoader operator=(const DefaultClassLoader &&)JVMX_FN_DELETE;

protected:
  virtual void Open( const JVMX_CHAR_TYPE * pFileName ) JVMX_OVERRIDE;

protected:
  virtual uint32_t ReadMagicNumber();
  virtual uint16_t ReadMinorVersion();
  virtual uint16_t ReadMajorVersion();
  virtual uint16_t ReadConstantPoolCount();
  virtual uint8_t ReadNextConstantTag();
  virtual std::shared_ptr<ConstantPool> ReadConstantPoolTable( size_t poolCount );
  virtual uint16_t ReadAccessFlags();
  virtual uint16_t ReadInterfaceCount();
  virtual InterfaceInfoList ReadInterfaces( const ConstantPool &pPool, size_t count );
  virtual uint16_t ReadFieldCount();
  virtual FieldInfoList ReadFields( const ConstantPool &pPool, size_t count );
  virtual uint16_t ReadMethodCount();
  virtual MethodInfoList ReadMethods( const ConstantPool &pPool, size_t count );
  virtual uint16_t ReadAttributeCount();
  virtual CodeAttributeList ReadAttributes( const ConstantPool &pPool, size_t count );
  //  virtual CodeAttributeList ReadCodeAttributes( const ConstantPool &pPool, size_t count );


  virtual JavaString ReadString();

  virtual uint16_t ReadStringLength();

  virtual JavaInteger ReadInteger();
  virtual JavaLong ReadLong();
  virtual JavaFloat ReadFloat();
  virtual JavaDouble ReadDouble();
  virtual std::shared_ptr<ConstantPoolClassReference> ReadClassReference();
  virtual std::shared_ptr<ConstantPoolStringReference> ReadStringReference();
  virtual std::shared_ptr<ConstantPoolFieldReference> ReadFieldReference();
  virtual std::shared_ptr<ConstantPoolMethodReference> ReadMethodReference();
  virtual std::shared_ptr<ConstantPoolInterfaceMethodReference> ReadInterFaceMethodReference();
  virtual std::shared_ptr<ConstantPoolNameAndTypeDescriptor> ReadNameAndTypeDescriptor();
  virtual std::shared_ptr<ConstantPoolInvokeDynamic> ReadInvokeDynamic();
  virtual std::shared_ptr<ConstantPoolMethodHandle> ReadMethodHandle();
  virtual std::shared_ptr<ConstantPoolMethodType> ReadMethodType();

  virtual ConstantPoolIndex ReadIndex();

  //virtual uint8_t ReadUint8();
  //virtual uint16_t ReadUint16();
  //virtual uint32_t ReadUint32();
  //virtual uint64_t ReadUint64();
  virtual DataBuffer ReadBuffer( size_t length );

protected:
  CodeAttributeFactory *m_pAttributeFactory;
  BigEndianStream m_fileStream;
};

#endif // __JAVAFILELOADER_H__
