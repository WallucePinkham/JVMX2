#include "jni.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace
{
  jclass g_testClass = nullptr;
  jclass g_objectClass = nullptr;
  jfieldID g_intField = nullptr;
  jfieldID g_longField = nullptr;
  jfieldID g_objectField = nullptr;
  jmethodID g_constructor = nullptr;
  jmethodID g_objectMethod = nullptr;
  jmethodID g_voidMethod = nullptr;
  jobject g_object = nullptr;
  jobject g_globalReference = nullptr;
  jobject g_localReference = nullptr;
  jobjectArray g_objectArray = nullptr;
  jstring g_utfString = nullptr;
  const jbyte *g_utfCharacters = nullptr;
  bool g_localFramePushed = false;

  template <typename Test>
  void RunJniTest(JNIEnv *env, const char *name, jint &totalTests, jint &failedTests, Test test)
  {
    ++totalTests;
    bool passed = false;

    try
    {
      passed = test();
    }
    catch (...)
    {
      passed = false;
    }

    try
    {
      if (JNI_TRUE == env->ExceptionCheck(env))
      {
        env->ExceptionClear(env);
        passed = false;
      }
    }
    catch (...)
    {
      passed = false;
    }

    if (!passed)
    {
      ++failedTests;
      std::printf("FAIL: JNI %s\n", name);
      std::fflush(stdout);
    }
  }

  jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
  {
    va_list args;
    va_start(args, methodID);
    jobject result = nullptr;
    try
    {
      result = env->NewObjectV(env, clazz, methodID, args);
    }
    catch (...)
    {
      va_end(args);
      throw;
    }
    va_end(args);
    return result;
  }

  jobject CallObjectMethodV(JNIEnv *env, jobject object, jmethodID methodID, ...)
  {
    va_list args;
    va_start(args, methodID);
    jobject result = nullptr;
    try
    {
      result = env->CallObjectMethodV(env, object, methodID, args);
    }
    catch (...)
    {
      va_end(args);
      throw;
    }
    va_end(args);
    return result;
  }

  void CallVoidMethodV(JNIEnv *env, jobject object, jmethodID methodID, ...)
  {
    va_list args;
    va_start(args, methodID);
    try
    {
      env->CallVoidMethodV(env, object, methodID, args);
    }
    catch (...)
    {
      va_end(args);
      throw;
    }
    va_end(args);
  }

  bool CheckArrayLength(JNIEnv *env, jarray array, jsize expectedLength)
  {
    if (nullptr == array)
    {
      return false;
    }

    const bool matches = env->GetArrayLength(env, array) == expectedLength;
    env->DeleteLocalRef(env, array);
    return matches;
  }

  bool CheckNegativeArrayLength(JNIEnv *env, jarray array)
  {
    if (nullptr != array || JNI_TRUE != env->ExceptionCheck(env))
    {
      if (nullptr != array)
      {
        env->DeleteLocalRef(env, array);
      }
      if (JNI_TRUE == env->ExceptionCheck(env))
      {
        env->ExceptionClear(env);
      }
      return false;
    }

    jthrowable exception = env->ExceptionOccurred(env);
    env->ExceptionClear(env);
    jclass expectedException = env->FindClass(env, "java/lang/NegativeArraySizeException");
    if (nullptr == expectedException || JNI_TRUE == env->ExceptionCheck(env))
    {
      if (nullptr != exception)
      {
        env->DeleteLocalRef(env, exception);
      }
      if (nullptr != expectedException)
      {
        env->DeleteLocalRef(env, expectedException);
      }
      if (JNI_TRUE == env->ExceptionCheck(env))
      {
        env->ExceptionClear(env);
      }
      return false;
    }

    const bool matches = JNI_TRUE == env->IsInstanceOf(env, exception, expectedException);
    env->DeleteLocalRef(env, exception);
    env->DeleteLocalRef(env, expectedException);
    return matches;
  }
}

extern "C"
{
  JNIEXPORT jlong JNICALL Java_TestJNIEntrypoints_runAll(JNIEnv *env, jclass testClass)
  {
    jint totalTests = 0;
    jint failedTests = 0;
    g_testClass = testClass;
    g_objectClass = nullptr;
    g_intField = nullptr;
    g_longField = nullptr;
    g_objectField = nullptr;
    g_constructor = nullptr;
    g_objectMethod = nullptr;
    g_voidMethod = nullptr;
    g_object = nullptr;
    g_globalReference = nullptr;
    g_localReference = nullptr;
    g_objectArray = nullptr;
    g_utfString = nullptr;
    g_utfCharacters = nullptr;
    g_localFramePushed = false;

    RunJniTest(env, "GetVersion", totalTests, failedTests, [&]() {
      return env->GetVersion(env) >= JNI_VERSION_1_2;
    });

    RunJniTest(env, "FindClass", totalTests, failedTests, [&]() {
      g_objectClass = env->FindClass(env, "java/lang/Object");
      return nullptr != g_objectClass;
    });

    RunJniTest(env, "GetSuperclass", totalTests, failedTests, [&]() {
      jclass stringClass = env->FindClass(env, "java/lang/String");
      if (nullptr == stringClass || nullptr == g_objectClass)
      {
        return false;
      }
      jclass superClass = env->GetSuperclass(env, stringClass);
      const bool matches = JNI_TRUE == env->IsSameObject(env, superClass, g_objectClass);
      env->DeleteLocalRef(env, stringClass);
      if (nullptr != superClass)
      {
        env->DeleteLocalRef(env, superClass);
      }
      return matches;
    });

    RunJniTest(env, "GetFieldID", totalTests, failedTests, [&]() {
      g_intField = env->GetFieldID(env, g_testClass, "intField", "I");
      g_longField = env->GetFieldID(env, g_testClass, "longField", "J");
      g_objectField = env->GetFieldID(env, g_testClass, "objectField", "Ljava/lang/Object;");
      return nullptr != g_intField && nullptr != g_longField && nullptr != g_objectField;
    });

    RunJniTest(env, "GetMethodID", totalTests, failedTests, [&]() {
      g_constructor = env->GetMethodID(env, g_testClass, "<init>", "()V");
      g_objectMethod = env->GetMethodID(env, g_testClass, "objectMethod", "()Ljava/lang/Object;");
      g_voidMethod = env->GetMethodID(env, g_testClass, "voidMethod", "()V");
      return nullptr != g_constructor && nullptr != g_objectMethod && nullptr != g_voidMethod;
    });

    RunJniTest(env, "NewObject", totalTests, failedTests, [&]() {
      g_object = nullptr == g_constructor ? nullptr : env->NewObject(env, g_testClass, g_constructor);
      return nullptr != g_object;
    });

    RunJniTest(env, "ToReflectedField", totalTests, failedTests, [&]() {
      if (nullptr == g_intField)
      {
        return false;
      }
      jobject field = env->ToReflectedField(env, g_testClass, g_intField, JNI_FALSE);
      if (nullptr == field)
      {
        return false;
      }
      env->DeleteLocalRef(env, field);
      return true;
    });

    RunJniTest(env, "Throw", totalTests, failedTests, [&]() {
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      jmethodID constructor = env->GetMethodID(env, exceptionClass, "<init>", "()V");
      jobject exception = nullptr == constructor ? nullptr : env->NewObject(env, exceptionClass, constructor);
      if (nullptr == exception)
      {
        env->DeleteLocalRef(env, exceptionClass);
        return false;
      }
      const jint throwResult = env->Throw(env, static_cast<jthrowable>(exception));
      const bool detected = JNI_TRUE == env->ExceptionCheck(env);
      if (detected)
      {
        env->ExceptionClear(env);
      }
      env->DeleteLocalRef(env, exception);
      env->DeleteLocalRef(env, exceptionClass);
      return JNI_OK == throwResult && detected;
    });

    RunJniTest(env, "ThrowNew", totalTests, failedTests, [&]() {
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      const jint result = env->ThrowNew(env, exceptionClass, "JNI ThrowNew test");
      const bool detected = JNI_TRUE == env->ExceptionCheck(env);
      if (detected)
      {
        env->ExceptionClear(env);
      }
      env->DeleteLocalRef(env, exceptionClass);
      return JNI_OK == result && detected;
    });

    RunJniTest(env, "ExceptionOccurred", totalTests, failedTests, [&]() {
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      env->ThrowNew(env, exceptionClass, "JNI ExceptionOccurred test");
      jthrowable exception = env->ExceptionOccurred(env);
      const bool found = nullptr != exception;
      env->ExceptionClear(env);
      if (nullptr != exception)
      {
        env->DeleteLocalRef(env, exception);
      }
      env->DeleteLocalRef(env, exceptionClass);
      return found;
    });

    RunJniTest(env, "ExceptionDescribe", totalTests, failedTests, [&]() {
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      env->ThrowNew(env, exceptionClass, "JNI ExceptionDescribe test");
      env->ExceptionDescribe(env);
      const bool cleared = JNI_FALSE == env->ExceptionCheck(env);
      if (!cleared && JNI_TRUE == env->ExceptionCheck(env))
      {
        env->ExceptionClear(env);
      }
      env->DeleteLocalRef(env, exceptionClass);
      return cleared;
    });

    RunJniTest(env, "ExceptionClear", totalTests, failedTests, [&]() {
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      env->ThrowNew(env, exceptionClass, "JNI ExceptionClear test");
      env->ExceptionClear(env);
      const bool cleared = JNI_FALSE == env->ExceptionCheck(env);
      env->DeleteLocalRef(env, exceptionClass);
      return cleared;
    });

    RunJniTest(env, "PushLocalFrame", totalTests, failedTests, [&]() {
      const jint result = env->PushLocalFrame(env, 4);
      g_localFramePushed = JNI_OK == result;
      return g_localFramePushed;
    });

    RunJniTest(env, "PopLocalFrame", totalTests, failedTests, [&]() {
      if (!g_localFramePushed)
      {
        return false;
      }
      jobject result = env->PopLocalFrame(env, g_object);
      g_localFramePushed = false;
      const bool matches = nullptr != g_object && JNI_TRUE == env->IsSameObject(env, result, g_object);
      if (nullptr != result)
      {
        env->DeleteLocalRef(env, result);
      }
      return matches;
    });

    RunJniTest(env, "NewGlobalRef", totalTests, failedTests, [&]() {
      if (nullptr == g_object)
      {
        return false;
      }
      g_globalReference = env->NewGlobalRef(env, g_object);
      return nullptr != g_globalReference;
    });

    RunJniTest(env, "DeleteGlobalRef", totalTests, failedTests, [&]() {
      if (nullptr == g_globalReference)
      {
        return false;
      }
      env->DeleteGlobalRef(env, g_globalReference);
      g_globalReference = nullptr;
      return true;
    });

    RunJniTest(env, "DeleteLocalRef", totalTests, failedTests, [&]() {
      g_localReference = env->NewLocalRef(env, g_object);
      if (nullptr == g_localReference)
      {
        return false;
      }
      env->DeleteLocalRef(env, g_localReference);
      g_localReference = nullptr;
      return true;
    });

    RunJniTest(env, "IsSameObject", totalTests, failedTests, [&]() {
      return nullptr != g_object && JNI_TRUE == env->IsSameObject(env, g_object, g_object);
    });

    RunJniTest(env, "NewLocalRef", totalTests, failedTests, [&]() {
      g_localReference = env->NewLocalRef(env, g_object);
      return nullptr != g_localReference &&
        JNI_TRUE == env->IsSameObject(env, g_localReference, g_object);
    });

    RunJniTest(env, "AllocObject", totalTests, failedTests, [&]() {
      jobject allocated = env->AllocObject(env, g_testClass);
      const bool created = nullptr != allocated;
      if (nullptr != allocated)
      {
        env->DeleteLocalRef(env, allocated);
      }
      return created;
    });

    RunJniTest(env, "NewObjectV", totalTests, failedTests, [&]() {
      jobject object = nullptr == g_constructor ? nullptr : NewObjectV(env, g_testClass, g_constructor);
      const bool created = nullptr != object;
      if (nullptr != object)
      {
        env->DeleteLocalRef(env, object);
      }
      return created;
    });

    RunJniTest(env, "NewObjectA", totalTests, failedTests, [&]() {
      jobject object = nullptr == g_constructor ? nullptr : env->NewObjectA(env, g_testClass, g_constructor, nullptr);
      const bool created = nullptr != object;
      if (nullptr != object)
      {
        env->DeleteLocalRef(env, object);
      }
      return created;
    });

    RunJniTest(env, "GetObjectClass", totalTests, failedTests, [&]() {
      jclass clazz = nullptr == g_object ? nullptr : env->GetObjectClass(env, g_object);
      const bool matches = nullptr != clazz && JNI_TRUE == env->IsSameObject(env, clazz, g_testClass);
      if (nullptr != clazz)
      {
        env->DeleteLocalRef(env, clazz);
      }
      return matches;
    });

    RunJniTest(env, "IsInstanceOf", totalTests, failedTests, [&]() {
      return nullptr != g_object && JNI_TRUE == env->IsInstanceOf(env, g_object, g_testClass);
    });

    RunJniTest(env, "CallObjectMethod", totalTests, failedTests, [&]() {
      jobject result = nullptr == g_objectMethod ? nullptr : env->CallObjectMethod(env, g_object, g_objectMethod);
      const bool matches = nullptr != result && JNI_TRUE == env->IsSameObject(env, result, g_object);
      if (nullptr != result)
      {
        env->DeleteLocalRef(env, result);
      }
      return matches;
    });

    RunJniTest(env, "CallObjectMethodV", totalTests, failedTests, [&]() {
      jobject result = nullptr == g_objectMethod ? nullptr : CallObjectMethodV(env, g_object, g_objectMethod);
      const bool matches = nullptr != result && JNI_TRUE == env->IsSameObject(env, result, g_object);
      if (nullptr != result)
      {
        env->DeleteLocalRef(env, result);
      }
      return matches;
    });

    RunJniTest(env, "CallVoidMethod", totalTests, failedTests, [&]() {
      if (nullptr == g_voidMethod || nullptr == g_object)
      {
        return false;
      }
      env->CallVoidMethod(env, g_object, g_voidMethod);
      return true;
    });

    RunJniTest(env, "CallVoidMethodV", totalTests, failedTests, [&]() {
      if (nullptr == g_voidMethod || nullptr == g_object)
      {
        return false;
      }
      CallVoidMethodV(env, g_object, g_voidMethod);
      return true;
    });

    RunJniTest(env, "GetObjectField", totalTests, failedTests, [&]() {
      jobject value = nullptr == g_objectField ? nullptr : env->GetObjectField(env, g_object, g_objectField);
      const bool found = nullptr != value;
      if (nullptr != value)
      {
        env->DeleteLocalRef(env, value);
      }
      return found;
    });

    RunJniTest(env, "GetIntField", totalTests, failedTests, [&]() {
      return nullptr != g_intField && 17 == env->GetIntField(env, g_object, g_intField);
    });

    RunJniTest(env, "GetLongField", totalTests, failedTests, [&]() {
      return nullptr != g_longField && 29 == env->GetLongField(env, g_object, g_longField);
    });

    RunJniTest(env, "SetLongField", totalTests, failedTests, [&]() {
      if (nullptr == g_longField)
      {
        return false;
      }
      env->SetLongField(env, g_object, g_longField, 31);
      return 31 == env->GetLongField(env, g_object, g_longField);
    });

    RunJniTest(env, "NewString", totalTests, failedTests, [&]() {
      const jchar chars[] = { static_cast<jchar>('o'), static_cast<jchar>('k'), 0 };
      jstring string = env->NewString(env, chars, 2);
      const bool matches = nullptr != string && 2 == env->GetStringLength(env, string);
      if (nullptr != string)
      {
        env->DeleteLocalRef(env, string);
      }
      return matches;
    });

    RunJniTest(env, "NewStringUTF", totalTests, failedTests, [&]() {
      g_utfString = env->NewStringUTF(env, "jni");
      return nullptr != g_utfString;
    });

    RunJniTest(env, "GetStringLength", totalTests, failedTests, [&]() {
      return nullptr != g_utfString && 3 == env->GetStringLength(env, g_utfString);
    });

    RunJniTest(env, "GetStringUTFLength", totalTests, failedTests, [&]() {
      return nullptr != g_utfString && 3 == env->GetStringUTFLength(env, g_utfString);
    });

    RunJniTest(env, "GetStringUTFChars", totalTests, failedTests, [&]() {
      if (nullptr == g_utfString)
      {
        return false;
      }
      jboolean isCopy = JNI_FALSE;
      g_utfCharacters = env->GetStringUTFChars(env, g_utfString, &isCopy);
      return nullptr != g_utfCharacters &&
        0 == std::strcmp(reinterpret_cast<const char *>(g_utfCharacters), "jni");
    });

    RunJniTest(env, "ReleaseStringUTFChars", totalTests, failedTests, [&]() {
      if (nullptr == g_utfString || nullptr == g_utfCharacters)
      {
        return false;
      }
      env->ReleaseStringUTFChars(
        env, g_utfString, reinterpret_cast<const char *>(g_utfCharacters));
      g_utfCharacters = nullptr;
      env->DeleteLocalRef(env, g_utfString);
      g_utfString = nullptr;
      return true;
    });

    RunJniTest(env, "GetArrayLength", totalTests, failedTests, [&]() {
      jarray array = static_cast<jarray>(env->NewIntArray(env, 4));
      if (nullptr == array)
      {
        return false;
      }
      const bool matches = 4 == env->GetArrayLength(env, array);
      env->DeleteLocalRef(env, array);
      return matches;
    });

    RunJniTest(env, "NewObjectArray", totalTests, failedTests, [&]() {
      g_objectArray = env->NewObjectArray(env, 2, g_objectClass, g_object);
      return nullptr != g_objectArray;
    });

    RunJniTest(env, "GetObjectArrayElement", totalTests, failedTests, [&]() {
      jobject value = nullptr == g_objectArray ? nullptr : env->GetObjectArrayElement(env, g_objectArray, 0);
      const bool matches = nullptr != value && JNI_TRUE == env->IsSameObject(env, value, g_object);
      if (nullptr != value)
      {
        env->DeleteLocalRef(env, value);
      }
      return matches;
    });

    RunJniTest(env, "SetObjectArrayElement", totalTests, failedTests, [&]() {
      if (nullptr == g_objectArray)
      {
        return false;
      }
      env->SetObjectArrayElement(env, g_objectArray, 1, g_object);
      jobject value = env->GetObjectArrayElement(env, g_objectArray, 1);
      const bool matches = nullptr != value && JNI_TRUE == env->IsSameObject(env, value, g_object);
      if (nullptr != value)
      {
        env->DeleteLocalRef(env, value);
      }
      env->DeleteLocalRef(env, g_objectArray);
      g_objectArray = nullptr;
      return matches;
    });

    RunJniTest(env, "NewBooleanArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewBooleanArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewBooleanArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewBooleanArray(env, -1)));
    });

    RunJniTest(env, "NewByteArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewByteArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewByteArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewByteArray(env, -1)));
    });

    RunJniTest(env, "NewCharArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewCharArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewCharArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewCharArray(env, -1)));
    });

    RunJniTest(env, "NewShortArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewShortArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewShortArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewShortArray(env, -1)));
    });

    RunJniTest(env, "NewIntArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewIntArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewIntArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewIntArray(env, -1)));
    });

    RunJniTest(env, "NewLongArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewLongArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewLongArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewLongArray(env, -1)));
    });

    RunJniTest(env, "NewFloatArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewFloatArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewFloatArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewFloatArray(env, -1)));
    });

    RunJniTest(env, "NewDoubleArray", totalTests, failedTests, [&]() {
      return CheckArrayLength(env, static_cast<jarray>(env->NewDoubleArray(env, 3)), 3) &&
        CheckArrayLength(env, static_cast<jarray>(env->NewDoubleArray(env, 0)), 0) &&
        CheckNegativeArrayLength(env, static_cast<jarray>(env->NewDoubleArray(env, -1)));
    });

    RunJniTest(env, "MonitorEnter", totalTests, failedTests, [&]() {
      return nullptr != g_object && JNI_OK == env->MonitorEnter(env, g_object);
    });

    RunJniTest(env, "MonitorExit", totalTests, failedTests, [&]() {
      return nullptr != g_object && JNI_OK == env->MonitorExit(env, g_object);
    });

    RunJniTest(env, "GetJavaVM", totalTests, failedTests, [&]() {
      JavaVM *vm = nullptr;
      return JNI_OK == env->GetJavaVM(env, &vm) && nullptr != vm;
    });

    RunJniTest(env, "ExceptionCheck", totalTests, failedTests, [&]() {
      if (JNI_TRUE == env->ExceptionCheck(env))
      {
        return false;
      }
      jclass exceptionClass = env->FindClass(env, "java/lang/IllegalStateException");
      if (nullptr == exceptionClass)
      {
        return false;
      }
      const jint result = env->ThrowNew(env, exceptionClass, "JNI ExceptionCheck test");
      const bool detected = JNI_TRUE == env->ExceptionCheck(env);
      if (detected)
      {
        env->ExceptionClear(env);
      }
      env->DeleteLocalRef(env, exceptionClass);
      return JNI_OK == result && detected && JNI_FALSE == env->ExceptionCheck(env);
    });

    RunJniTest(env, "JVMX_arraycopy", totalTests, failedTests, [&]() {
      env->JVMX_arraycopy(env, nullptr, nullptr, 0, nullptr, 0, 0);
      return true;
    });

#include "TestJNIStubProbes.inc"

    if (nullptr != g_utfString)
    {
      env->DeleteLocalRef(env, g_utfString);
    }
    if (nullptr != g_objectArray)
    {
      env->DeleteLocalRef(env, g_objectArray);
    }
    if (nullptr != g_localReference)
    {
      env->DeleteLocalRef(env, g_localReference);
    }
    if (nullptr != g_object)
    {
      env->DeleteLocalRef(env, g_object);
    }
    if (nullptr != g_objectClass)
    {
      env->DeleteLocalRef(env, g_objectClass);
    }

    return (static_cast<jlong>(totalTests) << 32) | static_cast<jlong>(failedTests);
  }
}
