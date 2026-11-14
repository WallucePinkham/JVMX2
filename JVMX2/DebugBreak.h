#pragma once

#ifndef __DEBUGBREAK_H__
#define __DEBUGBREAK_H__

#include <csignal>

#if defined (_MSC_VER)
#include <intrin.h>
#endif

inline void JvmxDebugBreak()
{
#if defined (_MSC_VER)
  __debugbreak();
#elif defined (__GNUC__) || defined (__clang__)
  __builtin_trap();
#else
  raise( SIGTRAP );
#endif
}

#endif // __DEBUGBREAK_H__
