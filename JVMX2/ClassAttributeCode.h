
#ifndef __ATTRIBUTECODE_H__
#define __ATTRIBUTECODE_H__

#include <vector>

#include "CodeSegmentDataBuffer.h"
#include "ConstantPoolClassReference.h"
#include "JavaClassConstants.h"
#include "ExceptionTableEntry.h"
#include "CodeAttributeFactory.h"
#include "ConstantPool.h"

#include "JavaCodeAttribute.h"

class Stream; // Forward Declaration

class ClassAttributeCode : public JavaCodeAttribute
{
  ClassAttributeCode( uint16_t maximumOperandStackDepth, uint16_t localVariableArraySizeIncludingPassedParameters, DataBuffer code, ExceptionTable exceptionTable, CodeAttributeList attributes );

public:
  ClassAttributeCode( const ClassAttributeCode &other );
  ClassAttributeCode( ClassAttributeCode &&other );

  virtual ~ClassAttributeCode();

  static ClassAttributeCode FromBinary( Stream &byteStream, CodeAttributeFactory &attributeFactory, const ConstantPool &constantPool );

  virtual DataBuffer ToBinary( const ConstantPool &constantPool ) const JVMX_OVERRIDE;

  static void swap( ClassAttributeCode &left, ClassAttributeCode &right ) JVMX_NOEXCEPT;

  virtual uint16_t GetMaximumOperandStackDepth() const;
  virtual uint16_t GetLocalVariableArraySizeIncludingPassedParameters() const;
  virtual const DataBuffer &GetCode() const;
  virtual const ExceptionTable &GetExceptionTable() const;
  virtual const CodeAttributeList &GetAttributeList() const;

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual bool Equals( const ClassAttributeCode &other ) const JVMX_NOEXCEPT;

protected:
  uint16_t m_MaximumOperandStackDepth;
  uint16_t m_LocalVariableArraySizeIncludingPassedParameters;
  DataBuffer m_Code;
  ExceptionTable m_ExceptionTable;
  CodeAttributeList m_Attributes;
};

#endif // __ATTRIBUTECODE_H__
