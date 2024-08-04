
#ifndef __EXCEPTIONTABLEENTRY_H__
#define __EXCEPTIONTABLEENTRY_H__

#include <vector>

#include "GlobalConstants.h"
#include "ConstantPoolClassReference.h"

class ExceptionTableEntry
{
public:
  ExceptionTableEntry( uint16_t startPosition, uint16_t endPosition, uint16_t handlerPosition, std::shared_ptr<ConstantPoolClassReference> pCatchType );

  ExceptionTableEntry( const ExceptionTableEntry &other );

  virtual ~ExceptionTableEntry();

  ExceptionTableEntry &operator=(const ExceptionTableEntry &other);

  bool operator==(const ExceptionTableEntry &other) const JVMX_NOEXCEPT;

  virtual uint16_t GetStartPosition() const;
  virtual uint16_t GetEndPosition() const;
  virtual uint16_t GetHandlerPosition() const;
  virtual std::shared_ptr<ConstantPoolClassReference> GetCatchType() const;

private:
  uint16_t m_StartPosition;
  uint16_t m_EndPosition;
  uint16_t m_HandlerPosition;
  std::shared_ptr<ConstantPoolClassReference> m_pCatchType;
};

typedef std::vector<ExceptionTableEntry> ExceptionTable;
typedef std::vector<uint16_t> ExceptionTableIndexList;

#endif // __EXCEPTIONTABLEENTRY_H__

