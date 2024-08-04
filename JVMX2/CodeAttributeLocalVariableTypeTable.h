
#ifndef _CODEATTRIBUTELOCALVARIABLETYPETABLE__H_
#define _CODEATTRIBUTELOCALVARIABLETYPETABLE__H_

#include "JavaCodeAttribute.h"
#include "LocalVariableTypeTableEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeLocalVariableTypeTable : public JavaCodeAttribute
{
private:
  CodeAttributeLocalVariableTypeTable( const JavaString &name, LocalVariableTypeTable localVariableTypes );

public:
  CodeAttributeLocalVariableTypeTable( const CodeAttributeLocalVariableTypeTable &other );
  CodeAttributeLocalVariableTypeTable( CodeAttributeLocalVariableTypeTable &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeLocalVariableTypeTable &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfLocalVariableTypes() const;
  virtual uint16_t GetStartPositionAt( size_t at ) const;
  virtual uint16_t GetLengthAt( size_t at ) const;
  virtual std::shared_ptr<ConstantPoolStringReference> GetNameReferenceAt( size_t at ) const;
  virtual uint16_t GetSignatureIndexAt( size_t at ) const;
  virtual uint16_t GetIndexAt( size_t at ) const;

  virtual ~CodeAttributeLocalVariableTypeTable() JVMX_NOEXCEPT;

  static CodeAttributeLocalVariableTypeTable FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeLocalVariableTypeTable &left, CodeAttributeLocalVariableTypeTable &right ) JVMX_NOEXCEPT;

protected:
  LocalVariableTypeTable m_LocalVariableTypes;
};


#endif // _CODEATTRIBUTELOCALVARIABLETYPETABLE__H_
