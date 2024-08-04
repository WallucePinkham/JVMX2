
#ifndef _HELPERVMRUNTIME__H_
#define _HELPERVMRUNTIME__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMRuntime
{
public:
  static void JNICALL java_lang_VMRuntime_insertSystemProperties( JNIEnv *pEnv, jobject obj, jobject properties );
  static jint JNICALL java_lang_VMRuntime_nativeLoad( JNIEnv *pEnv, jobject obj, jstring fileName, jobject classLoader );
  static jstring JNICALL Java_java_lang_VMRuntime_mapLibraryName( JNIEnv *pEnv, jobject obj, jstring libName );

 // static JavaString GetJavaStringFromStringObject( JNIEnv * pEnv, jstring libName );

  static void JNICALL java_lang_VMRuntime_runFinalizationForExit( JNIEnv *pEnv, jobject obj );
  static void JNICALL java_lang_VMRuntime_exit( JNIEnv *pEnv, jobject obj, jint statusCode );
};

#endif // _HELPERVMRUNTIME__H_
