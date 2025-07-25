#ifndef _HELPERTYPES__H_
#define _HELPERTYPES__H_

#include "GlobalConstants.h"

#include <boost/intrusive_ptr.hpp>

#include "JavaTypes.h"

union jvalue; // forward declaration

class HelperTypes
{
public:
  static JavaString ExtractValueFromStringObject(const ObjectReference* pStringObject);
  static JavaString ExtractValueFromStringObject(const JavaObject* pStringObject);
  static JavaString ExtractValueFromStringObject( boost::intrusive_ptr<ObjectReference> pStringObject );
  static JavaString GetPackageNameFromClassName( const JavaString &className );
  static void ConvertJavaStringToArray( const boost::intrusive_ptr<ObjectReference> &pArray, const JavaString &string );
  static jvalue Unbox( const boost::intrusive_ptr<IJavaVariableType> &pBoxed );

  static size_t String16Length( const char16_t *pString );
  static size_t String32Length( const char32_t *pString );
  static size_t WideStringLength( const wchar_t *pString );

  static boost::intrusive_ptr<ObjectReference> CreateArray( e_JavaArrayTypes type, size_t size );
};

#endif // _HELPERTYPES__H_
