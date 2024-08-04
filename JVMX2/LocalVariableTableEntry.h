
#ifndef _LOCALVARIABLETABLEENTRY__H_
#define _LOCALVARIABLETABLEENTRY__H_

#include <vector>

#include "GlobalConstants.h"
#include "ConstantPoolClassReference.h"
#include "ConstantPoolStringReference.h"
#include "ConstantPoolFieldReference.h"

class LocalVariableTableEntry
{
public:
  LocalVariableTableEntry( uint16_t startPosition, uint32_t length, std::shared_ptr<ConstantPoolStringReference> pNameReference, uint16_t descriptorIndex, uint16_t index );

  LocalVariableTableEntry( const LocalVariableTableEntry &other );

  virtual ~LocalVariableTableEntry() JVMX_NOEXCEPT;

  LocalVariableTableEntry &operator=(const LocalVariableTableEntry &other);

  bool operator==(const LocalVariableTableEntry &other) const JVMX_NOEXCEPT;

  virtual uint16_t GetStartPosition() const JVMX_NOEXCEPT;
  virtual uint32_t GetLength() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<ConstantPoolStringReference> GetNameReference() const;
  virtual uint16_t GetDescriptorIndex() const JVMX_NOEXCEPT;
  virtual uint16_t GetIndex() const JVMX_NOEXCEPT;

private:
  uint16_t m_StartPosition;
  uint32_t m_Length;
  std::shared_ptr<ConstantPoolStringReference> m_pNameReference;
  uint16_t m_DescriptorIndex;
  uint16_t m_Index;
};

typedef std::vector<LocalVariableTableEntry> LocalVariableTable;
//typedef std::vector<uint16_t> LocalVariableTableIndexList;

#endif // _LOCALVARIABLETABLEENTRY__H_
