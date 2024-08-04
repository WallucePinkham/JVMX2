#ifndef _HELPERVMDOUBLE__H_
#define _HELPERVMDOUBLE__H_

#include "include/jni.h"

class HelperVMDouble
{
public:
  static void JNICALL java_lang_Double_initIDs( JNIEnv *pEnv, jobject obj );
  static jstring JNICALL java_lang_Double_toString( JNIEnv *pEnv, jobject obj, jdouble value, jboolean isFloat );

private:
  static const std::string FloatToString(float f);
  static const std::string DoubleToString(double f);
};

#endif // _HELPERVMDOUBLE__H_
