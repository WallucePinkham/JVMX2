
#ifndef _HELPERVMSYSTEM__H_
#define _HELPERVMSYSTEM__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMSystem
{
public:
  static jint JNICALL java_lang_VMSystem_identityHashCode( JNIEnv *pEnv, jobject obj, jobject objToHash );
  static void JNICALL java_lang_VMSystem_arraycopy( JNIEnv *pEnv, jobject obj, jobject src, jint srcOffset, jobject dest, jint destOffset, jint length );
  static jboolean JNICALL java_lang_VMSystem_isWordsBigEndian( JNIEnv *pEnv, jobject obj );
  static jlong JNICALL java_lang_VMSystem_currentTimeMillis( JNIEnv *pEnv, jobject obj );
  static void JNICALL gnu_classpath_VMSystemProperties_preInit( JNIEnv *pEnv, jobject obj, jobject properties );
  
  
};

#endif // _HELPERVMSYSTEM__H_
