
#ifndef _JNI_EXTERNAL__H_
#define _JNI_EXTERNAL__H_

#if defined(_WIN32) || defined (_WIN64)
#define JNICALL __stdcall
#else

#endif // __cplusplus

#ifdef _MSC_VER
#define JNIEXPORT __declspec( dllexport )
#ifdef JNI_EXPORT_METHODS
#define JNIEXPORT_ JNIEXPORT
#else // JNI_EXPORT_METHODS
#define JNIEXPORT_ __declspec( dllimport )
#endif
#else // _MSC_VER
#error "You need to define JNIEXPORT for your compiler"
#endif // _MSC_VER


#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif // __cplusplus

typedef uint8_t jboolean;
typedef int8_t jbyte;
typedef uint16_t jchar;
typedef int16_t jshort;
typedef int32_t jint;
typedef int64_t jlong;
typedef float jfloat;
typedef double jdouble;
typedef jint jsize;

#ifdef __cplusplus

class _jobject {};
class _jclass : public _jobject {};
class _jthrowable : public _jobject {};

class _jstring : public _jobject {};
class _jarray : public _jobject {};
class _jbooleanArray : public _jarray {};
class _jbyteArray : public _jarray {};
class _jcharArray : public _jarray {};
class _jshortArray : public _jarray {};
class _jintArray : public _jarray {};
class _jlongArray : public _jarray {};
class _jfloatArray : public _jarray {};
class _jdoubleArray : public _jarray {};
class _jobjectArray : public _jarray {};
class _jweak : public _jobject {};

typedef _jobject *jobject;
typedef _jclass *jclass;
typedef _jthrowable *jthrowable;
typedef _jstring *jstring;
typedef _jarray *jarray;
typedef _jbooleanArray *jbooleanArray;
typedef _jbyteArray *jbyteArray;
typedef _jcharArray *jcharArray;
typedef _jshortArray *jshortArray;
typedef _jintArray *jintArray;
typedef _jlongArray *jlongArray;
typedef _jfloatArray *jfloatArray;
typedef _jdoubleArray *jdoubleArray;
typedef _jobjectArray *jobjectArray;

typedef _jweak *jweak;
typedef _jthrowable *jthrowable;

#else

typedef struct {} _jobject;

typedef _jobject *jobject;

typedef jobject jclass;
typedef jobject jvalue;
typedef jobject jstring;

typedef jobject jarray;
typedef jobject jbooleanArray;
typedef jobject jbyteArray;
typedef jobject jcharArray;
typedef jobject jshortArray;
typedef jobject jintArray;
typedef jobject jlongArray;
typedef jobject jfloatArray;
typedef jobject jdoubleArray;
typedef jobject jobjectArray;

typedef jobject jweak;
typedef jobject jthrowable;

#endif // __cplusplus

typedef union jvalue
{
  jboolean z;
  jbyte b;
  jchar c;
  jshort s;
  jint i;
  jlong j;
  jfloat f;
  jdouble d;
  jobject l;
} jvalue;

struct _jfieldID;
typedef struct _jfieldID *jfieldID;
struct _jmethodID;
typedef struct _jmethodID *jmethodID;

const jint JNI_COMMIT = 1;
const jint JNI_ABORT = 2;

const jint JNI_FALSE = 0;
const jint JNI_TRUE = 1;

const jint JNI_VERSION_1_1 = 0x00010001; /* JNI version 1.1 */
const jint JNI_VERSION_1_2 = 0x00010002; /* JNI version 1.2 */

const jint JNI_OK = 0;
const jint JNI_ERR = -1;
const jint JNI_EDETACHED = -2; /* thread detached from the VM */
const jint JNI_EVERSION = -3; /* JNI version error */
const jint JNI_ENOMEM = -4;
const jint JNI_EEXISTS = -5;
const jint JNI_EINVAL = -6;

extern "C"
{
  struct JavaVM
  {
    void * ( JNICALL *pReservedCOM1)(); // index 0
    void * (JNICALL *pReservedCOM2)(); // index 1
    void * (JNICALL *pReservedCOM3)(); // index 2

    jint( JNICALL *DestroyJavaVM )(JavaVM *vm); // index 3
    jint( JNICALL *AttachCurrentThread )(JavaVM *env, void **, void *); // index 4
    jint( JNICALL *DetachCurrentThread )(JavaVM *vm); // index 5
    jint( JNICALL *GetEnv )(JavaVM *vm, void **penv, jint interface_id); // index 6
  };

  struct JavaVMAttachArgs
  {
    jint version;
    char *name;
    jobject group;
  };

  struct JNINativeMethod
  {
    char *name;
    char *signature;
    void *fnPtr;
  };

  struct JNIEnv
  {
    void * (JNICALL *pReservedCOM1)(); // index 0
    void * (JNICALL*pReservedCOM2)(); // index 1
    void * (JNICALL*pReservedCOM3)(); // index 2
    void * (JNICALL *pReserved)();     // index 3

    jint( JNICALL *GetVersion )(JNIEnv *env); // index 4

    jclass( JNICALL *DefineClass )(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize bufLen); // index 5
    jclass( JNICALL *FindClass )(JNIEnv *env, const char *name); //index 6

    jmethodID( JNICALL *FromReflectedMethod )(JNIEnv *env, jobject method); //index 7
    jfieldID( JNICALL *FromReflectedField )(JNIEnv *env, jobject field); //index 8
    jobject( JNICALL *ToReflectedMethod )(JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic); // index 9

    jclass( JNICALL *GetSuperclass )(JNIEnv *env, jclass clazz); // index 10

    jboolean( JNICALL *IsAssignableFrom )(JNIEnv *env, jclass clazz1, jclass clazz2); // index 11
    jobject( JNICALL *ToReflectedField )(JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic); // index 12

    jint( JNICALL *Throw )(JNIEnv *env, jthrowable obj); // index 13
    jint( JNICALL *ThrowNew )(JNIEnv *env, jclass clazz, const char *message); // index 14
    jthrowable( JNICALL *ExceptionOccurred )(JNIEnv *env); // index 15

    void(JNICALL *ExceptionDescribe)(JNIEnv *env); // index 16
    void(JNICALL *ExceptionClear)(JNIEnv *env); // index 17
    void(JNICALL *FatalError)(JNIEnv *env, const char *msg); // index 18

    jint( JNICALL *PushLocalFrame )(JNIEnv *env, jint capacity); // index 19
    jobject( JNICALL *PopLocalFrame )(JNIEnv *env, jobject result); // index 20

    jobject( JNICALL *NewGlobalRef )(JNIEnv *env, jobject obj); // index 21
    void(JNICALL *DeleteGlobalRef)(JNIEnv *env, jobject gref); // index 22
    void(JNICALL *DeleteLocalRef)(JNIEnv *env, jobject lref); // index 23
    jboolean( JNICALL *IsSameObject )(JNIEnv *env, jobject ref1, jobject ref2); // index 24
    jobject( JNICALL *NewLocalRef )(JNIEnv *env, jobject ref);

    jint( JNICALL *EnsureLocalCapacity )(JNIEnv *env, jint capacity); // index 26
    jobject( JNICALL *AllocObject )(JNIEnv *env, jclass clazz); // index 27

    jobject( JNICALL *NewObject )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 28
    jobject( JNICALL *NewObjectV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args); // index 29
    jobject( JNICALL *NewObjectA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 30

    jclass( JNICALL *GetObjectClass )(JNIEnv *env, jobject obj); //index 31
    jboolean( JNICALL *IsInstanceOf )(JNIEnv *env, jobject obj, jclass clazz); //index 32
    jmethodID( JNICALL *GetMethodID )(JNIEnv *env, jclass clazz, const char *name, const char *sig);  // index 33

    jobject( JNICALL *CallObjectMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 34
    jobject( JNICALL *CallObjectMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 35
    jobject( JNICALL *CallObjectMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);  // index 36

    jboolean( JNICALL *CallBooleanMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 37
    jboolean( JNICALL *CallBooleanMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 38
    jboolean( JNICALL *CallBooleanMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 39

    jbyte( JNICALL *CallByteMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 40
    jbyte( JNICALL *CallByteMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 41
    jbyte( JNICALL *CallByteMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 42

    jchar( JNICALL *CallCharMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 43
    jchar( JNICALL *CallCharMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 44
    jchar( JNICALL *CallCharMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 45

    jshort( JNICALL *CallShortMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 46
    jshort( JNICALL *CallShortMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 47
    jshort( JNICALL *CallShortMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 48

    jint( JNICALL *CallIntMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 49
    jint( JNICALL *CallIntMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 50
    jint( JNICALL *CallIntMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 51

    jlong( JNICALL *CallLongMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 52
    jlong( JNICALL *CallLongMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 53
    jlong( JNICALL *CallLongMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 54

    jfloat( JNICALL *CallFloatMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 55
    jfloat( JNICALL *CallFloatMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 56
    jfloat( JNICALL *CallFloatMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 57

    jdouble( JNICALL *CallDoubleMethod )(JNIEnv *env, jobject obj, jmethodID methodID, ...); // index 58
    jdouble( JNICALL *CallDoubleMethodV )(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 59
    jdouble( JNICALL *CallDoubleMethodA )(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args); // index 60

    void(JNICALL *CallVoidMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);  // index 61
    void(JNICALL *CallVoidMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);  // index 62
    void(JNICALL *CallVoidMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);  // index 63

    jobject( JNICALL *CallNonvirtualObjectMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 64
    jobject( JNICALL *CallNonvirtualObjectMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 65
    jobject( JNICALL *CallNonvirtualObjectMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 66

    jboolean( JNICALL *CallNonvirtualBooleanMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 67
    jboolean( JNICALL *CallNonvirtualBooleanMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 68
    jboolean( JNICALL *CallNonvirtualBooleanMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 69

    jbyte( JNICALL *CallNonvirtualByteMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 70
    jbyte( JNICALL *CallNonvirtualByteMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 71
    jbyte( JNICALL *CallNonvirtualByteMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 72

    jchar( JNICALL *CallNonvirtualCharMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 73
    jchar( JNICALL *CallNonvirtualCharMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 74
    jchar( JNICALL *CallNonvirtualCharMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 75

    jshort( JNICALL *CallNonvirtualShortMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 76
    jshort( JNICALL *CallNonvirtualShortMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 77
    jshort( JNICALL *CallNonvirtualShortMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 78

    jint( JNICALL *CallNonvirtualIntMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 79
    jint( JNICALL *CallNonvirtualIntMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 80
    jint( JNICALL *CallNonvirtualIntMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 81

    jlong( JNICALL *CallNonvirtualLongMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 82
    jlong( JNICALL *CallNonvirtualLongMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 83
    jlong( JNICALL *CallNonvirtualLongMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 84

    jfloat( JNICALL *CallNonvirtualFloatMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 85
    jfloat( JNICALL *CallNonvirtualFloatMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 86
    jfloat( JNICALL *CallNonvirtualFloatMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 87

    jdouble( JNICALL *CallNonvirtualDoubleMethod )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...); // index 88
    jdouble( JNICALL *CallNonvirtualDoubleMethodV )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 89
    jdouble( JNICALL *CallNonvirtualDoubleMethodA )(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 90

    void(JNICALL *CallNonvirtualVoidMethod)(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);  // index 91
    void(JNICALL *CallNonvirtualVoidMethodV)(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);  // index 92
    void(JNICALL *CallNonvirtualVoidMethodA)(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args);  // index 93

    jfieldID( JNICALL *GetFieldID )(JNIEnv *env, jclass clazz, const char *name, const char *sig); // index 94

    jobject( JNICALL *GetObjectField )(JNIEnv *env, jobject obj, jfieldID fieldID); // index 95
    jboolean( JNICALL *GetBooleanField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 96
    jbyte( JNICALL *GetByteField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 97
    jchar( JNICALL *GetCharField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 98
    jshort( JNICALL *GetShortField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 99
    jint( JNICALL *GetIntField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 100
    jlong( JNICALL *GetLongField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 101
    jfloat( JNICALL *GetFloatField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 102
    jdouble( JNICALL *GetDoubleField )(JNIEnv *env, jobject obj, jfieldID fieldID); //index 103

    void(JNICALL *SetObjectField)(JNIEnv *env, jobject obj, jfieldID fieldID, jobject value); // index 104
    void(JNICALL *SetBooleanField)(JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value); // index 105
    void(JNICALL *SetByteField)(JNIEnv *env, jobject obj, jfieldID fieldID, jbyte value); // index 106
    void(JNICALL *SetCharField)(JNIEnv *env, jobject obj, jfieldID fieldID, jchar value); // index 107
    void(JNICALL *SetShortField)(JNIEnv *env, jobject obj, jfieldID fieldID, jshort value); // index 108
    void(JNICALL *SetIntField)(JNIEnv *env, jobject obj, jfieldID fieldID, jint value); // index 109
    void(JNICALL *SetLongField)(JNIEnv *env, jobject obj, jfieldID fieldID, jlong value); // index 110
    void(JNICALL *SetFloatField)(JNIEnv *env, jobject obj, jfieldID fieldID, jfloat value); // index 111
    void(JNICALL *SetDoubleField)(JNIEnv *env, jobject obj, jfieldID fieldID, jdouble value); // index 112

    jmethodID( JNICALL *GetStaticMethodID )(JNIEnv *env, jclass clazz, const char *name, const char *sig); // index 113

    jobject( JNICALL *CallStaticObjectMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 114
    jobject( JNICALL *CallStaticObjectMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 115
    jobject( JNICALL *CallStaticObjectMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 116

    jboolean( JNICALL *CallStaticBooleanMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 117
    jboolean( JNICALL *CallStaticBooleanMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 118
    jboolean( JNICALL *CallStaticBooleanMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 119

    jbyte( JNICALL *CallStaticByteMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 120
    jbyte( JNICALL *CallStaticByteMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 121
    jbyte( JNICALL *CallStaticByteMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 122

    jchar( JNICALL *CallStaticCharMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 123
    jchar( JNICALL *CallStaticCharMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 124
    jchar( JNICALL *CallStaticCharMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 125

    jshort( JNICALL *CallStaticShortMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 126
    jshort( JNICALL *CallStaticShortMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 127
    jshort( JNICALL *CallStaticShortMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 128

    jint( JNICALL *CallStaticIntMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 129
    jint( JNICALL *CallStaticIntMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 130
    jint( JNICALL *CallStaticIntMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 131

    jlong( JNICALL *CallStaticLongMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 132
    jlong( JNICALL *CallStaticLongMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 133
    jlong( JNICALL *CallStaticLongMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 134

    jfloat( JNICALL *CallStaticFloatMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 135
    jfloat( JNICALL *CallStaticFloatMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 136
    jfloat( JNICALL *CallStaticFloatMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 137

    jdouble( JNICALL *CallStaticDoubleMethod )(JNIEnv *env, jclass clazz, jmethodID methodID, ...); // index 138
    jdouble( JNICALL *CallStaticDoubleMethodV )(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 139
    jdouble( JNICALL *CallStaticDoubleMethodA )(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 140

    void(JNICALL *CallStaticVoidMethod)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);  // index 141
    void(JNICALL *CallStaticVoidMethodV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);  // index 142
    void(JNICALL *CallStaticVoidMethodA)(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);  // index 143

    jfieldID( JNICALL *GetStaticFieldID )(JNIEnv *env, jclass clazz, const char *name, const char *sig); // index 144

    jobject( JNICALL *GetStaticObjectField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 145
    jboolean( JNICALL *GetStaticBooleanField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 146
    jbyte( JNICALL *GetStaticByteField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 147
    jchar( JNICALL *GetStaticCharField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 148
    jshort( JNICALL *GetStaticShortField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 149
    jint( JNICALL *GetStaticIntField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 150
    jlong( JNICALL *GetStaticLongField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 151
    jfloat( JNICALL *GetStaticFloatField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 152
    jdouble( JNICALL *GetStaticDoubleField )(JNIEnv *env, jclass clazz, jfieldID fieldID); // index 153

    void(JNICALL *SetStaticObjectField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value); // 154
    void(JNICALL *SetStaticBooleanField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value); // 155
    void(JNICALL *SetStaticByteField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value); // 156
    void(JNICALL *SetStaticCharField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value); // 157
    void(JNICALL *SetStaticShortField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value); // 158
    void(JNICALL *SetStaticIntField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jint value); // 159
    void(JNICALL *SetStaticLongField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value); // 160
    void(JNICALL *SetStaticFloatField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value); // 161
    void(JNICALL *SetStaticDoubleField)(JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value); // 162

    jstring( JNICALL *NewString )(JNIEnv *env, const jchar *uchars, jsize len); // index 163
    jsize( JNICALL *GetStringLength )(JNIEnv *env, jstring string); // index 164
    const jchar *(JNICALL * GetStringChars)(JNIEnv *env, jstring string, jboolean *isCopy); // index 165

    void (JNICALL *ReleaseStringChars)(JNIEnv *env, jstring string, const jchar *chars); // index 166

    jstring( JNICALL *NewStringUTF )(JNIEnv *env, const char *bytes); // index 167
    jsize( JNICALL *GetStringUTFLength )(JNIEnv *env, jstring string); // index 168
    const jbyte *(JNICALL *GetStringUTFChars)(JNIEnv *env, jstring string, jboolean *isCopy); // index 169
    void(JNICALL *ReleaseStringUTFChars)(JNIEnv *env, jstring string, const char *utf); // index 170

    jsize( JNICALL *GetArrayLength )(JNIEnv *env, jarray array); // index 171

    jobjectArray( JNICALL *NewObjectArray )(JNIEnv *env, jsize length, jclass elementType, jobject initialElement); // index 172

    jobject( JNICALL *GetObjectArrayElement )(JNIEnv *env, jobjectArray array, jsize index); // index 173

    void (JNICALL *SetObjectArrayElement)(JNIEnv *env, jobjectArray array, jsize index, jobject value);  // index 174

    jbooleanArray( JNICALL *NewBooleanArray )(JNIEnv *env, jsize length); // index 176
    jbyteArray( JNICALL *NewByteArray )(JNIEnv *env, jsize length); // index 176
    jcharArray( JNICALL *NewCharArray )(JNIEnv *env, jsize length); // index 177
    jshortArray( JNICALL *NewShortArray )(JNIEnv *env, jsize length); // index 178
    jintArray( JNICALL *NewIntArray )(JNIEnv *env, jsize length); // index 179
    jlongArray( JNICALL *NewLongArray )(JNIEnv *env, jsize length); // index 180
    jfloatArray( JNICALL *NewFloatArray )(JNIEnv *env, jsize length); // index 181
    jdoubleArray( JNICALL *NewDoubleArray )(JNIEnv *env, jsize length); // index 182

    jboolean *(JNICALL *GetBooleanArrayElements)(JNIEnv *env, jbooleanArray array, jboolean *isCopy); // index 183
    jbyte *(JNICALL *GetByteArrayElements)(JNIEnv *env, jbyteArray array, jboolean *isCopy); // index 184
    jchar *(JNICALL *GetCharArrayElements)(JNIEnv *env, jcharArray array, jchar *isCopy); // index 185
    jshort *(JNICALL *GetShortArrayElements)(JNIEnv *env, jshortArray array, jshort *isCopy); // index 186
    jint *(JNICALL *GetIntArrayElements)(JNIEnv *env, jintArray array, jint *isCopy); // index 187
    jlong *(JNICALL *GetLongArrayElements)(JNIEnv *env, jlongArray array, jlong *isCopy); // index 188
    jfloat *(JNICALL *GetFloatArrayElements)(JNIEnv *env, jfloatArray array, jfloat *isCopy); // index 189
    jdouble *(JNICALL *GetDoubleArrayElements)(JNIEnv *env, jdoubleArray array, jdouble *isCopy); // index 190

    void(JNICALL *ReleaseBooleanArrayElements)(JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode); // index 191
    void(JNICALL *ReleaseByteArrayElements)(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode); // index 192
    void(JNICALL *ReleaseCharArrayElements)(JNIEnv *env, jcharArray array, jchar *elems, jint mode); // index 193
    void(JNICALL *ReleaseShortArrayElements)(JNIEnv *env, jshortArray array, jshort *elems, jint mode); // index 194
    void(JNICALL *ReleaseIntArrayElements)(JNIEnv *env, jintArray array, jint *elems, jint mode); // index 195
    void(JNICALL *ReleaseLongArrayElements)(JNIEnv *env, jlongArray array, jlong *elems, jint mode); // index 196
    void(JNICALL *ReleaseFloatArrayElements)(JNIEnv *env, jfloatArray array, jfloat *elems, jint mode); // index 197
    void(JNICALL *ReleaseDoubleArrayElements)(JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode); // index 198

    void(JNICALL *GetBooleanArrayRegion)(JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf); // index 199
    void(JNICALL *GetByteArrayRegion)(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf); // index 200
    void(JNICALL *GetCharArrayRegion)(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf); // index 201
    void(JNICALL *GetShortArrayRegion)(JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf); // index 202
    void(JNICALL *GetIntArrayRegion)(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf); // index 203
    void(JNICALL *GetLongArrayRegion)(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf); // index 204
    void(JNICALL *GetFloatArrayRegion)(JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf); // index 205
    void(JNICALL *GetDoubleArrayRegion)(JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf); // index 206

    void(JNICALL *SetBooleanArrayRegion)(JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf); // index 207
    void(JNICALL *SetByteArrayRegion)(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf); // index 208
    void(JNICALL *SetCharArrayRegion)(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf); // index 209
    void(JNICALL *SetShortArrayRegion)(JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf); // index 210
    void(JNICALL *SetIntArrayRegion)(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf); // index 211
    void(JNICALL *SetLongArrayRegion)(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf); // index 212
    void(JNICALL *SetFloatArrayRegion)(JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf); // index 213
    void(JNICALL *SetDoubleArrayRegion)(JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf); // index 214

    jint( JNICALL *RegisterNatives )(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods); // index 215
    jint( JNICALL *UnregisterNatives )(JNIEnv *env, jclass clazz); // index 216

    jint( JNICALL *MonitorEnter )(JNIEnv *env, jobject obj); // index 217
    jint( JNICALL *MonitorExit )(JNIEnv *env, jobject obj); // index 218

    jint( JNICALL *GetJavaVM )(JNIEnv *env, JavaVM **vm); // index 219

    void(JNICALL *GetStringRegion)(JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf); // index 220
    void(JNICALL *GetStringUTFRegion)(JNIEnv *env, jstring str, jsize start, jsize len, char *buf); //index 221

    void *(JNICALL *GetPrimitiveArrayCritical)(JNIEnv *env, jarray array, jboolean *isCopy); // index 222
    void (JNICALL *ReleasePrimitiveArrayCritical)(JNIEnv *env, jarray array, void *carray, jint mode); // index 223

    const jchar *(JNICALL *GetStringCritical)(JNIEnv *env, jstring string, jboolean *isCopy); // index 224
    void(JNICALL *ReleaseStringCritical)(JNIEnv *env, jstring string, const jchar *carray); // index 225

    jweak( JNICALL *NewWeakGlobalRef )(JNIEnv *env, jobject obj); // index 226
    void(JNICALL *DeleteWeakGlobalRef)(JNIEnv *env, jobject wref); // index 227
    jboolean( JNICALL *ExceptionCheck )(JNIEnv *env); // index 228

    // Special JVMX Methods
    void (JNICALL *JVMX_arraycopy)(JNIEnv *pEnv, jobject obj, jobject src, int srcOffset, jobject dest, int destOffset, int length);
  };
} // extern "C"

#endif // _JNI_EXTERNAL__H_
