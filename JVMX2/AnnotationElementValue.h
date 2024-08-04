
#ifndef _ANNOTATIONELEMENTVALUE__H_
#define _ANNOTATIONELEMENTVALUE__H_

class AnnotationsEntry; // Forward Declaration

enum e_AnnotationsElementTypes
{
  e_AnnotationsElementTypeUnset
  , e_AnnotationsElementTypeReferenceToArrayDimention
  , e_AnnotationsElementTypeBoolean
  , e_AnnotationsElementTypeShort
  , e_AnnotationsElementTypeReferenceToClass
  , e_AnnotationsElementTypeLong
  , e_AnnotationsElementTypeInt
  , e_AnnotationsElementTypeFloat
  , e_AnnotationsElementTypeDouble
  , e_AnnotationsElementTypeUnicodeChar
  , e_AnnotationsElementTypeSignedByte
  , e_AnnotationsElementTypeString
  , e_AnnotationsElementTypeEnumConstant
  //, e_AnnotationsElementTypeClass
  , e_AnnotationsElementTypeAnnotation
  , e_AnnotationsElementTypeArray
};

class AnnotationsElementValue
{
private:
  AnnotationsElementValue();

public:
  AnnotationsElementValue( const AnnotationsElementValue &other );
  AnnotationsElementValue( AnnotationsElementValue &&other );

  virtual ~AnnotationsElementValue() JVMX_NOEXCEPT;

  static std::shared_ptr<AnnotationsElementValue> FromBinary( Stream &stream );

  virtual DataBuffer ToBinary() const;

  virtual e_AnnotationsElementTypes GetType() const JVMX_NOEXCEPT;

  virtual AnnotationsElementValue &operator=(AnnotationsElementValue other);

  virtual bool operator==(const AnnotationsElementValue &other) const JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetConstValueIndex() const;
  virtual ConstantPoolIndex GetEnumConstValueTypeNameIndex() const;
  virtual ConstantPoolIndex GetEnumConstValueConstNameIndex() const;
  virtual ConstantPoolIndex GetClassInfoIndex() const;
  virtual std::shared_ptr<AnnotationsEntry> GetNestedAnnotation() const;
  virtual uint16_t GetArrayValueNumberOfValues() const;
  virtual std::shared_ptr<AnnotationsElementValue> GetArrayValueAt( size_t index );

protected:
  virtual void swap( AnnotationsElementValue &left, AnnotationsElementValue &right ) JVMX_NOEXCEPT;

  virtual e_AnnotationsElementTypes SetTypeFromTag( uint8_t tag );
  virtual uint8_t GetTagFromType() const;

private:
  e_AnnotationsElementTypes m_Type;

  ConstantPoolIndex m_ConstValueIndex;
  ConstantPoolIndex m_EnumConstValueTypeNameIndex;
  ConstantPoolIndex m_EnumConstValueConstNameIndex;
  ConstantPoolIndex m_ClassInfoIndex;
  std::shared_ptr<AnnotationsEntry> m_pAnnotation;
  std::vector<std::shared_ptr<AnnotationsElementValue>> m_Values;
};

struct AnnotationsElementValuePair
{
  AnnotationsElementValuePair( ConstantPoolIndex elementNameIndex, std::shared_ptr<AnnotationsElementValue> &pElementValue );

  bool operator==(const AnnotationsElementValuePair &other) const;

  virtual ~AnnotationsElementValuePair() JVMX_NOEXCEPT;

  ConstantPoolIndex m_ElementNameIndex;
  std::shared_ptr<AnnotationsElementValue> m_pElementValue;
};

typedef std::vector<AnnotationsElementValuePair> AnnotationsElementValuePairList;

#endif // _ANNOTATIONELEMENTVALUE__H_
