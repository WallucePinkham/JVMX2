#ifndef __JVMREGISTERS_H__
#define __JVMREGISTERS_H__

#include "GlobalConstants.h"

 class JVMRegisters
  {
  public:
    uintptr_t m_ProgramCounter;
    const uint8_t *m_pCodeSegmentStart;
    size_t m_CodeSegmentLength;
  };

#endif // __JVMREGISTERS_H__

