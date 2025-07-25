
#ifndef _HELPERVMSTRING__H_
#define _HELPERVMSTRING__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMString
{
private:
  HelperVMString() JVMX_FN_DELETE;

public:
  static jstring JNICALL java_lang_VMString_intern(JNIEnv* pEnv, jobject obj, jobject string);
};

#endif // _HELPERVMSTRING__H_
