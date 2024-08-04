
#pragma once

#ifndef __GLOBALCONSTANTS_H__
#define __GLOBALCONSTANTS_H__

#include <cstdint>
#include <string>

#if defined (_MSC_VER)
#if _MSC_VER <= 1600
#define JVMX_NOEXCEPT throw()
#define JVMX_PURE = 0
#define JVMX_OVERRIDE override
#define JVMX_FN_DELETE
#elif _MSC_VER <= 1700
#define JVMX_NOEXCEPT throw()
#define JVMX_PURE abstract
#define JVMX_OVERRIDE override
#define JVMX_FN_DELETE
#elif _MSC_VER <= 1800
#define JVMX_NOEXCEPT throw()
#define JVMX_PURE abstract
#define JVMX_OVERRIDE override
#define JVMX_FN_DELETE = delete
#else
#define JVMX_NOEXCEPT noexcept
#define JVMX_PURE abstract
#define JVMX_FN_DELETE = delete
#define JVMX_OVERRIDE override
#endif
#else
#define JVMX_NOEXCEPT noexcept
#define JVMX_PURE abstract
#define JVMX_FN_DELETE = delete
#define JVMX_OVERRIDE override
#endif

#define JVMX_ABSTRACT abstract

#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_WARNINGS 1

// Disable these warnings completely.
#pragma warning ( disable: 4996 ) // _CRT_SECURE_NO_WARNINGS
#pragma warning ( disable: 4481 ) // nonstandard extension used: override specifier 'override'
#pragma warning ( disable: 4480 ) // nonstandard extension used: specifying underlying type for enum

// Treat these warnings as errors. They indicate we are doing something wrong.
#pragma warning ( error: 4717 )   // function recursive on all control paths
#pragma warning ( error: 4244 )   // conversion from 'int64_t' to 'uint32_t', possible loss of data

#define JVMX_WIDE_CHAR_TYPE char16_t
#define JVMX_ANSI_CHAR_TYPE char

#ifdef _UNICODE
#define JVMX_CHAR_TYPE JVMX_WIDE_CHAR_TYPE
#else
#define JVMX_CHAR_TYPE JVMX_ANSI_CHAR_TYPE
#endif // _UNICODE

#ifdef _UNICODE
#define JVMX_T( param ) u##param
#else // _UNICODE
#define JVMX_T( param ) u8##param
#endif // _UNICODE

// #ifdef _UNICODE
// #define JVMX_STRCMP( str1, str2 ) wcscmp( (str1), (str2) )
// #define JVMX_STRLEN( str1 ) wcslen( (str1) )
// #define JVMX_STRCHR( str1, ch1 ) wcschr( (str1), (ch1) )
// #define JVMX_STRNCPY( str1, str2, count ) wcsncpy( (str1), (str2), (count) )
// #else
// #define JVMX_STRCMP( str1, str2 ) strcmp( (str1), (str2) )
// #define JVMX_STRLEN( str1 ) strlen( (str1) )
// #define JVMX_STRCHR( str1, ch1 ) strchr( (str1), (ch1) )
// #define JVMX_STRNCPY( str1, str2, count ) strncpy( (str1), (str2), (count) )
// #endif // _UNICODE

#define JVMX_CUSTOM_ASSERT

#if !defined( JVMX_CUSTOM_ASSERT )
#include <cassert>
#define JVMX_ASSERT(expr) assert(expr);
#endif // !defined( JVMX_CUSTOM_ASSERT )

#if defined(_DEBUG) && defined( JVMX_CUSTOM_ASSERT )
#undef assert
void JvmxAssert( bool cond, const char *description );
#define JVMX_ASSERT( expr ) JvmxAssert( (expr), #expr )
#else // defined( JVMX_CUSTOM_ASSERT ) && defined(_DEBUG)
#define JVMX_ASSERT( expr )
#endif // JVMX_CUSTOM_ASSERT

#include "GlobalFileOperations.h"

#ifdef _UNICODE
typedef std::wstring JVMXString;
#else
typedef std::string JVMXString;
#endif // _UNICODE

#define JVMX_STRINGIZE(line) #line

//const uint32_t c_JavaClassFileMagicNumber = 0xBEBAFECA;
const uint32_t c_JavaClassFileMagicNumber = 0xCAFEBABE;

//#define JVMX_LOG_VERBOSE 1

// Forward declarations
class JavaString;
class JavaInteger;
class JavaLong;
class JavaFloat;
class JavaDouble;

typedef uint16_t ConstantPoolIndex;
typedef uint32_t LocalVariable;

#endif // __GLOBALCONSTANTS_H__
