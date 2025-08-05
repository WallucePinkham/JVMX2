#pragma once

#ifndef _HELPERVMINETADDRESS__H_
#define _HELPERVMINETADDRESS__H_

#include "GlobalConstants.h"
#include "include/jni.h"

class HelperVMInetAddress
{
public:
  static jarray JNICALL java_net_VMInetAddress_lookupInaddrAny(JNIEnv* pEnv, jclass clazz);
  static jstring JNICALL java_net_VMInetAddress_getLocalHostname(JNIEnv* pEnv, jclass clazz);
  static jarray JNICALL java_net_VMInetAddress_aton(JNIEnv* pEnv, jclass clazz, jstring stringVal);
};

#endif // !_HELPERVMINETADDRESS__H_
