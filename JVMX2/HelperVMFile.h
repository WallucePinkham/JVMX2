#ifndef _HELPEVMFILE__H_
#define _HELPEVMFILE__H_

#include "GlobalConstants.h"
#include "TypeParser.h"
#include "include/jni.h"

class HelperVMFile
{
public:
  static jboolean JNICALL java_io_VMFile_isDirectory(JNIEnv* pEnv, jobject obj, jstring path);
  static jboolean JNICALL java_io_VMFile_exists(JNIEnv* pEnv, jobject obj, jstring path);
  static jobject  JNICALL java_io_VMFile_toCanonicalForm(JNIEnv* pEnv, jobject obj, jstring path);
};

#endif // _HELPEVMFILE__H_

