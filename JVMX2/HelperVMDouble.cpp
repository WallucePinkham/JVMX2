#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <boost\intrusive_ptr.hpp>

#include "JavaNativeInterface.h"

#include "ILogger.h"
#include "IVirtualMachineState.h"

#include "InvalidStateException.h"

#include "ObjectReference.h"
#include "GlobalCatalog.h"

#include "HelperVMDouble.h"

void JNICALL HelperVMDouble::java_lang_Double_initIDs( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_Double_initIDs()\n" );
#endif // _DEBUG
}


jstring JNICALL HelperVMDouble::java_lang_Double_toString( JNIEnv *pEnv, jobject obj, jdouble value, jboolean isFloat )
{
#if defined(_DEBUG)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_Double_toString(%lf,%d)\n", value, isFloat ? 1 : 0 );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::string result;
  //char buffer[ _CVTBUFSIZE ];
  //const char *pResult = _gcvt( value, 12, buffer );
  if (isFloat)
  {
      //std::snprintf(buffer, sizeof(buffer), "%#F", static_cast<float>(value));
      result = FloatToString(static_cast<float>(value));
  }
  else
  {
      //std::snprintf(buffer, sizeof(buffer), "%#F", value);
      result = DoubleToString(value);
  }

  boost::intrusive_ptr<ObjectReference> pObjectResult = pVirtualMachineState->CreateStringObject( JavaString::FromCString( result.c_str() ) );

  return JNIEnvInternal::ConvertObjectPointerToJString( pVirtualMachineState, pObjectResult.get() );
}


const std::string HelperVMDouble::FloatToString(float f) {
    if (isnan(f))
    {
        return "NaN";
    }

    if (isinf(f))
    {
        return "Infinity";
    }
    
    //float rounded = std::round(f * 1000000.0f) / 1000000.0f;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << f;
    std::string str = ss.str();
    size_t pos = str.find('.');
    if (pos != std::string::npos) {
        while (str.size() > pos + 1 && str.back() == '0') {
            str.pop_back();
        }
        if (str.back() == '.') {
            str += '0';
        }
    }
    else {
        str += ".0";
    }
    return str;
}

const std::string HelperVMDouble::DoubleToString(double f) {
    if (isnan(f))
    {
        return "NaN";
    }

    if (isinf(f))
    {
        return "Infinity";
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << f;
    std::string str = ss.str();
    size_t pos = str.find('.');
    if (pos != std::string::npos) {
        while (str.size() > pos + 1 && str.back() == '0') {
            str.pop_back();
        }
        if (str.back() == '.') {
            str += '0';
        }
    }
    else {
        str += ".0";
    }
    return str;
}