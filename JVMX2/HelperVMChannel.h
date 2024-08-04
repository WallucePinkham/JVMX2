
#ifndef _HELPERVMCHANNEL__H_
#define _HELPERVMCHANNEL__H_

#include "include/jni.h"

class HelperVMChannel
{
public:
  static void JNICALL gnu_java_nio_VMChannel_initIDs( JNIEnv *pEnv, jobject obj );

  static jint JNICALL gnu_java_nio_VMChannel_stdin_fd( JNIEnv *pEnv, jobject obj );
  static jint JNICALL gnu_java_nio_VMChannel_stdout_fd( JNIEnv *pEnv, jobject obj );
  static jint JNICALL gnu_java_nio_VMChannel_stderr_fd( JNIEnv *pEnv, jobject obj );

  static void JNICALL gnu_java_nio_VMChannel_write( JNIEnv *pEnv, jobject obj, jint fd, jobject byteBufferSrc );
  static void JNICALL gnu_java_nio_VMChannel_close( JNIEnv *pEnv, jobject obj, jint fd );
  static jint JNICALL gnu_java_nio_VMChannel_open( JNIEnv *pEnv, jobject obj, jstring path, jint mode );
  

private:
  static bool MustCloseFile( int32_t fileNo );
};

#endif // _HELPERVMCHANNEL__H_
