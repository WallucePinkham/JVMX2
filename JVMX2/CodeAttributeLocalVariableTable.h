
#ifndef _CODEATTRIBUTELOCALVARIABLETABLE__H_
#define _CODEATTRIBUTELOCALVARIABLETABLE__H_

#include "JavaCodeAttribute.h"
#include "LocalVariableTableEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeLocalVariableTable : public JavaCodeAttribute
{
private:
  CodeAttributeLocalVariableTable( const JavaString &name, LocalVariableTable localVariables );

public:
  CodeAttributeLocalVariableTable( const CodeAttributeLocalVariableTable &other );
  CodeAttributeLocalVariableTable( CodeAttributeLocalVariableTable &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeLocalVariableTable &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfLocalVariables() const;
  virtual uint16_t GetStartPositionAt( size_t at ) const;
  virtual uint32_t GetLengthAt( size_t at ) const;
  virtual std::shared_ptr<ConstantPoolStringReference> GetNameReferenceAt( size_t at ) const;
  virtual uint16_t GetDescriptorIndexAt( size_t at ) const;
  virtual uint16_t GetIndexAt( size_t at ) const;

  virtual ~CodeAttributeLocalVariableTable() JVMX_NOEXCEPT;

  static CodeAttributeLocalVariableTable FromBinary( JavaString name, Stream &byteStream, const ConstantPool &constantPool );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeLocalVariableTable &left, CodeAttributeLocalVariableTable &right ) JVMX_NOEXCEPT;

protected:
  LocalVariableTable m_LocalVariables;
};

#endif // _CODEATTRIBUTELOCALVARIABLETABLE__H_
