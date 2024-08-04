
#ifndef __STACKFRAMEFULLFRAME_H__
#define __STACKFRAMEFULLFRAME_H__

#include <memory>

#include "StackFrame.h"
#include "VerificationTypeInfo.h"

class Stream;
class VerificationTypeInfoFactory; // Forward declaration

extern const uint8_t c_StackFrameFullFrameTag;

class StackFrameFullFrame : public StackFrame
{
  StackFrameFullFrame( uint16_t offset, uint16_t numberOfLocals, const VerificationTypeInfoList &localsVerificationList, uint16_t numberOfStackItems, const VerificationTypeInfoList &stackVerificationList );

public:
  virtual ~StackFrameFullFrame() JVMX_NOEXCEPT;

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual uint16_t GetNumberOfLocals() const JVMX_NOEXCEPT;
  virtual uint16_t GetNumberOfStackItems() const JVMX_NOEXCEPT;

  virtual std::shared_ptr<VerificationTypeInfo> GetLocalsVerificationType( size_t index ) const;
  virtual std::shared_ptr<VerificationTypeInfo> GetStackVerificationType( size_t index ) const;

  static StackFrameFullFrame FromBinary( Stream &byteStream );

  static VerificationTypeInfoList ReadVerificationTypeInfoList( uint16_t variableCount, Stream &byteStream, VerificationTypeInfoFactory &factory );

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameFullFrame &other ) const;

  DataBuffer ToBinary() const JVMX_OVERRIDE;

private:
  uint16_t m_OffsetDelta;
  uint16_t m_NumberOfLocals;
  VerificationTypeInfoList m_LocalsVerificationList;
  uint16_t m_NumberOfStackItems;
  VerificationTypeInfoList m_StackVerificationList;
};

#endif // __STACKFRAMEFULLFRAME_H__

