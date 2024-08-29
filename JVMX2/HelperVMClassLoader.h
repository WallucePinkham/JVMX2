#ifndef _HELPEVMCLASSLOADER__H_
#define _HELPEVMCLASSLOADER__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMClassLoader
{
public:
  static jobject JNICALL java_lang_VMClassLoader_getPrimitiveClass(JNIEnv* pEnv, jobject obj, jchar typeAsChar);
  static jobject JNICALL java_lang_VMClassLoader_defineClass(JNIEnv* pEnv, jobject obj, jobject classLoaderObj, jobject name, jarray data, jint offset, jint len, jobject pd);
};

#endif // _HELPEVMCLASSLOADER__H_

