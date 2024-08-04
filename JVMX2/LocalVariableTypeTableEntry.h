
#ifndef _LOCALVARIABLETYPETABLEENTRY__H_
#define _LOCALVARIABLETYPETABLEENTRY__H_

#include <vector>

#include "GlobalConstants.h"
#include "ConstantPoolClassReference.h"
#include "ConstantPoolStringReference.h"
#include "ConstantPoolFieldReference.h"

class LocalVariableTypeTableEntry
{
public:
  LocalVariableTypeTableEntry( uint16_t startPosition, uint16_t length, std::shared_ptr<ConstantPoolStringReference> pNameReference, uint16_t descriptorIndex, uint16_t index );

  LocalVariableTypeTableEntry( const LocalVariableTypeTableEntry &other );

  virtual ~LocalVariableTypeTableEntry() JVMX_NOEXCEPT;

  LocalVariableTypeTableEntry &operator=(const LocalVariableTypeTableEntry &other);

  bool operator==(const LocalVariableTypeTableEntry &other) const JVMX_NOEXCEPT;

  virtual uint16_t GetStartPosition() const JVMX_NOEXCEPT;
  virtual uint16_t GetLength() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<ConstantPoolStringReference> GetNameReference() const;
  virtual uint16_t GetDescriptorIndex() const JVMX_NOEXCEPT;
  virtual uint16_t GetIndex() const JVMX_NOEXCEPT;

private:
  uint16_t m_StartPosition;
  uint16_t m_Length;
  std::shared_ptr<ConstantPoolStringReference> m_pNameReference;
  uint16_t m_SignatureIndex;
  uint16_t m_Index;
};

typedef std::vector<LocalVariableTypeTableEntry> LocalVariableTypeTable;
//typedef std::vector<uint16_t> LocalVariableTableIndexList;

#endif // _LOCALVARIABLETYPETABLEENTRY__H_
