#ifndef _HELPERVMTHREAD__H_
#define _HELPERVMTHREAD__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMThread
{
public:
  static jobject JNICALL java_lang_VMThread_currentThread( JNIEnv *pEnv, jobject obj );
  static void JNICALL java_lang_VMThread_start( JNIEnv *pEnv, jobject obj, jlong stackSize );
  static void JNICALL java_lang_VMThread_sleep( JNIEnv *pEnv, jobject obj, jlong ms, jint ns );
  static void JNICALL java_lang_VMThread_yield( JNIEnv *pEnv, jobject obj );
  static void JNICALL java_lang_VMThread_suspend( JNIEnv *pEnv, jobject obj );
  static void JNICALL java_lang_VMThread_resume( JNIEnv *pEnv, jobject obj );
  static void JNICALL java_lang_VMThread_interrupt( JNIEnv *pEnv, jobject obj );
  static jboolean JNICALL java_lang_VMThread_interrupted( JNIEnv *pEnv, jobject obj );
};

#endif // _HELPERVMTHREAD__H_