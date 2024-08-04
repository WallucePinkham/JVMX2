
#ifndef _JNI_INTERNAL__H_
#define _JNI_INTERNAL__H_

#include <memory>

#include "GlobalConstants.h"
#include "JavaTypes.h"
#include "IGarbageCollector.h"
#include "IVirtualMachineState.h"

#include "include/jni.h"

class VirtualMachine;
class MethodInfo;

struct _jmethodID
{
  std::shared_ptr<MethodInfo> m_pMethodInfo;
};

extern "C" {
  struct JNIEnvInternal;

  struct JavaVMInternal
  {
    void *( JNICALL *pReservedCOM1 )(); // index 0
    void *( JNICALL *pReservedCOM2 )() ; // index 1
    void *( JNICALL *pReservedCOM3 )(); // index 2

    static JNIEXPORT jint JNICALL DestroyJavaVM( JavaVM *vm );                          // index 3
    static JNIEXPORT jint JNICALL AttachCurrentThread( JavaVM *env, void **, void * );  // index 4
    static JNIEXPORT jint JNICALL DetachCurrentThread( JavaVM *vm );                    // index 5
    static JNIEXPORT jint JNICALL GetEnv( JavaVM *vm, void **penv, jint interface_id ); // index 6
  };

  struct JNIEnvInternal
  {
    static JNIEXPORT jint JNICALL GetVersion( JNIEnv *env ); // index 4

    static JNIEXPORT jclass JNICALL DefineClass( JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize bufLen ); // index 5
    static JNIEXPORT jclass JNICALL FindClass( JNIEnv *env, const char *name ); // index 6

    static JNIEXPORT jmethodID JNICALL FromReflectedMethod( JNIEnv *env, jobject method ); // index 7
    static JNIEXPORT jfieldID JNICALL FromReflectedField( JNIEnv *env, jobject field ); // index 8
    static JNIEXPORT jobject JNICALL ToReflectedMethod( JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic ); // index 9

    static JNIEXPORT jclass JNICALL GetSuperclass( JNIEnv *env, jclass clazz ); // index 10

    static JNIEXPORT jboolean JNICALL IsAssignableFrom( JNIEnv *env, jclass clazz1, jclass clazz2 );                   // index 11
    static JNIEXPORT jobject JNICALL ToReflectedField( JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic ); // index 12

    static JNIEXPORT jint JNICALL Throw( JNIEnv *env, jthrowable obj );                       // index 13
    static JNIEXPORT jint JNICALL ThrowNew( JNIEnv *env, jclass clazz, const char *message ); // index 14
    static JNIEXPORT jthrowable JNICALL ExceptionOccurred( JNIEnv *env );                     // index 15

    static JNIEXPORT void JNICALL ExceptionDescribe( JNIEnv *env );           // index 16
    static JNIEXPORT void JNICALL ExceptionClear( JNIEnv *env );              // index 17
    static JNIEXPORT void JNICALL FatalError( JNIEnv *env, const char *msg ); // index 18

    static JNIEXPORT jint JNICALL PushLocalFrame( JNIEnv *env, jint capacity );    // index 19
    static JNIEXPORT jobject JNICALL PopLocalFrame( JNIEnv *env, jobject result ); // index 20

    static JNIEXPORT jobject JNICALL NewGlobalRef( JNIEnv *env, jobject obj );                 // index 21
    static JNIEXPORT void JNICALL DeleteGlobalRef( JNIEnv *env, jobject gref );                // index 22
    static JNIEXPORT void JNICALL DeleteLocalRef( JNIEnv *env, jobject lref );                 // index 23
    static JNIEXPORT jboolean JNICALL IsSameObject( JNIEnv *env, jobject ref1, jobject ref2 ); // index 24
    static JNIEXPORT jobject JNICALL NewLocalRef( JNIEnv *env, jobject ref );

    static JNIEXPORT jint JNICALL EnsureLocalCapacity( JNIEnv *env, jint capacity ); // index 26
    static JNIEXPORT jobject JNICALL AllocObject( JNIEnv *env, jclass clazz );       // index 27

    static JNIEXPORT jobject JNICALL NewObject( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 28
    static JNIEXPORT jobject JNICALL NewObjectV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 29
    static JNIEXPORT jobject JNICALL NewObjectA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 30

    static JNIEXPORT jclass JNICALL GetObjectClass( JNIEnv *env, jobject obj ); // index 31
    static JNIEXPORT jboolean JNICALL IsInstanceOf( JNIEnv *env, jobject obj, jclass clazz ); // index 32
    static JNIEXPORT jmethodID JNICALL GetMethodID( JNIEnv *env, jclass clazz, const char *name, const char *sig ); // index 33

    static JNIEXPORT jobject JNICALL CallObjectMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 34
    static JNIEXPORT jobject JNICALL CallObjectMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 35
    static JNIEXPORT jobject JNICALL CallObjectMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 36

    static JNIEXPORT jboolean JNICALL CallBooleanMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 37
    static JNIEXPORT jboolean JNICALL CallBooleanMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 38
    static JNIEXPORT jboolean JNICALL CallBooleanMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 39

    static JNIEXPORT jbyte JNICALL CallByteMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 40
    static JNIEXPORT jbyte JNICALL CallByteMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 41
    static JNIEXPORT jbyte JNICALL CallByteMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 42

    static JNIEXPORT jchar JNICALL CallCharMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 43
    static JNIEXPORT jchar JNICALL CallCharMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 44
    static JNIEXPORT jchar JNICALL CallCharMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 45

    static JNIEXPORT jshort JNICALL CallShortMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 46
    static JNIEXPORT jshort JNICALL CallShortMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 47
    static JNIEXPORT jshort JNICALL CallShortMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 48

    static JNIEXPORT jint JNICALL CallIntMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 49
    static JNIEXPORT jint JNICALL CallIntMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 50
    static JNIEXPORT jint JNICALL CallIntMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 51

    static JNIEXPORT jlong JNICALL CallLongMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 52
    static JNIEXPORT jlong JNICALL CallLongMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 53
    static JNIEXPORT jlong JNICALL CallLongMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 54

    static JNIEXPORT jfloat JNICALL CallFloatMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 55
    static JNIEXPORT jfloat JNICALL CallFloatMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 56
    static JNIEXPORT jfloat JNICALL CallFloatMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 57

    static JNIEXPORT jdouble JNICALL CallDoubleMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 58
    static JNIEXPORT jdouble JNICALL CallDoubleMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 59
    static JNIEXPORT jdouble JNICALL CallDoubleMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 60

    static JNIEXPORT void JNICALL CallVoidMethod( JNIEnv *env, jobject obj, jmethodID methodID, ... );           // index 61
    static JNIEXPORT void JNICALL CallVoidMethodV( JNIEnv *env, jobject obj, jmethodID methodID, va_list args ); // index 62
    static JNIEXPORT void JNICALL CallVoidMethodA( JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args ); // index 63

    static JNIEXPORT jobject JNICALL CallNonvirtualObjectMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 64
    static JNIEXPORT jobject JNICALL CallNonvirtualObjectMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 65
    static JNIEXPORT jobject JNICALL CallNonvirtualObjectMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 66

    static JNIEXPORT jboolean JNICALL CallNonvirtualBooleanMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 67
    static JNIEXPORT jboolean JNICALL CallNonvirtualBooleanMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 68
    static JNIEXPORT jboolean JNICALL CallNonvirtualBooleanMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 69

    static JNIEXPORT jbyte JNICALL CallNonvirtualByteMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 70
    static JNIEXPORT jbyte JNICALL CallNonvirtualByteMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 71
    static JNIEXPORT jbyte JNICALL CallNonvirtualByteMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 72

    static JNIEXPORT jchar JNICALL CallNonvirtualCharMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 73
    static JNIEXPORT jchar JNICALL CallNonvirtualCharMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 74
    static JNIEXPORT jchar JNICALL CallNonvirtualCharMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 75

    static JNIEXPORT jshort JNICALL CallNonvirtualShortMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 76
    static JNIEXPORT jshort JNICALL CallNonvirtualShortMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 77
    static JNIEXPORT jshort JNICALL CallNonvirtualShortMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 78

    static JNIEXPORT jint JNICALL CallNonvirtualIntMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 79
    static JNIEXPORT jint JNICALL CallNonvirtualIntMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 80
    static JNIEXPORT jint JNICALL CallNonvirtualIntMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 81

    static JNIEXPORT jlong JNICALL CallNonvirtualLongMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 82
    static JNIEXPORT jlong JNICALL CallNonvirtualLongMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 83
    static JNIEXPORT jlong JNICALL CallNonvirtualLongMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 84

    static JNIEXPORT jfloat JNICALL CallNonvirtualFloatMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 85
    static JNIEXPORT jfloat JNICALL CallNonvirtualFloatMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 86
    static JNIEXPORT jfloat JNICALL CallNonvirtualFloatMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 87

    static JNIEXPORT jdouble JNICALL  CallNonvirtualDoubleMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 88
    static JNIEXPORT jdouble JNICALL CallNonvirtualDoubleMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 89
    static JNIEXPORT jdouble JNICALL CallNonvirtualDoubleMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 90

    static JNIEXPORT void JNICALL  CallNonvirtualVoidMethod( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ... ); // index 91
    static JNIEXPORT void JNICALL CallNonvirtualVoidMethodV( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args ); // index 92
    static JNIEXPORT void JNICALL CallNonvirtualVoidMethodA( JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args ); // index 93

    static JNIEXPORT jfieldID JNICALL GetFieldID( JNIEnv *env, jclass clazz, const char *name, const char *sig ); // index 94

    static JNIEXPORT jobject JNICALL GetObjectField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 95
    static JNIEXPORT jboolean JNICALL GetBooleanField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 96
    static JNIEXPORT jbyte JNICALL GetByteField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 97
    static JNIEXPORT jchar JNICALL GetCharField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 98
    static JNIEXPORT jshort JNICALL GetShortField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 99
    static JNIEXPORT jint JNICALL GetIntField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 100
    static JNIEXPORT jlong JNICALL GetLongField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 101
    static JNIEXPORT jfloat JNICALL GetFloatField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 102
    static JNIEXPORT jdouble JNICALL GetDoubleField( JNIEnv *env, jobject obj, jfieldID fieldID ); // index 103

    static JNIEXPORT void JNICALL SetObjectField( JNIEnv *env, jobject obj, jfieldID fieldID, jobject value );   // index 104
    static JNIEXPORT void JNICALL SetBooleanField( JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value ); // index 105
    static JNIEXPORT void JNICALL SetByteField( JNIEnv *env, jobject obj, jfieldID fieldID, jbyte value );       // index 106
    static JNIEXPORT void JNICALL SetCharField( JNIEnv *env, jobject obj, jfieldID fieldID, jchar value );       // index 107
    static JNIEXPORT void JNICALL SetShortField( JNIEnv *env, jobject obj, jfieldID fieldID, jshort value );     // index 108
    static JNIEXPORT void JNICALL SetIntField( JNIEnv *env, jobject obj, jfieldID fieldID, jint value );         // index 109
    static JNIEXPORT void JNICALL SetLongField( JNIEnv *env, jobject obj, jfieldID fieldID, jlong value );       // index 110
    static JNIEXPORT void JNICALL SetFloatField( JNIEnv *env, jobject obj, jfieldID fieldID, jfloat value );     // index 111
    static JNIEXPORT void JNICALL SetDoubleField( JNIEnv *env, jobject obj, jfieldID fieldID, jdouble value );   // index 112

    static JNIEXPORT jmethodID JNICALL GetStaticMethodID( JNIEnv *env, jclass clazz, const char *name, const char *sig ); // index 113

    static JNIEXPORT jobject JNICALL CallStaticObjectMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 114
    static JNIEXPORT jobject JNICALL CallStaticObjectMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 115
    static JNIEXPORT jobject JNICALL CallStaticObjectMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 116

    static JNIEXPORT jboolean JNICALL CallStaticBooleanMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... ); // index 117
    static JNIEXPORT jboolean JNICALL CallStaticBooleanMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 118
    static JNIEXPORT jboolean JNICALL CallStaticBooleanMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 119

    static JNIEXPORT jbyte JNICALL CallStaticByteMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 120
    static JNIEXPORT jbyte JNICALL CallStaticByteMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 121
    static JNIEXPORT jbyte JNICALL CallStaticByteMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 122

    static JNIEXPORT jchar JNICALL CallStaticCharMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 123
    static JNIEXPORT jchar JNICALL CallStaticCharMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 124
    static JNIEXPORT jchar JNICALL CallStaticCharMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 125

    static JNIEXPORT jshort JNICALL CallStaticShortMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 126
    static JNIEXPORT jshort JNICALL CallStaticShortMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 127
    static JNIEXPORT jshort JNICALL CallStaticShortMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 128

    static JNIEXPORT jint JNICALL CallStaticIntMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 129
    static JNIEXPORT jint JNICALL CallStaticIntMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 130
    static JNIEXPORT jint JNICALL CallStaticIntMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 131

    static JNIEXPORT jlong JNICALL CallStaticLongMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 132
    static JNIEXPORT jlong JNICALL CallStaticLongMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 133
    static JNIEXPORT jlong JNICALL CallStaticLongMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 134

    static JNIEXPORT jfloat JNICALL CallStaticFloatMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 135
    static JNIEXPORT jfloat JNICALL CallStaticFloatMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 136
    static JNIEXPORT jfloat JNICALL CallStaticFloatMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 137

    static JNIEXPORT jdouble JNICALL CallStaticDoubleMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 138
    static JNIEXPORT jdouble JNICALL CallStaticDoubleMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 139
    static JNIEXPORT jdouble JNICALL CallStaticDoubleMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 140

    static JNIEXPORT void JNICALL CallStaticVoidMethod( JNIEnv *env, jclass clazz, jmethodID methodID, ... );           // index 141
    static JNIEXPORT void JNICALL CallStaticVoidMethodV( JNIEnv *env, jclass clazz, jmethodID methodID, va_list args ); // index 142
    static JNIEXPORT void JNICALL CallStaticVoidMethodA( JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args ); // index 143

    static JNIEXPORT jfieldID JNICALL GetStaticFieldID( JNIEnv *env, jclass clazz, const char *name, const char *sig ); // index 144

    static JNIEXPORT jobject JNICALL GetStaticObjectField( JNIEnv *env, jclass clazz, jfieldID fieldID );   // index 145
    static JNIEXPORT jboolean JNICALL GetStaticBooleanField( JNIEnv *env, jclass clazz, jfieldID fieldID ); // index 146
    static JNIEXPORT jbyte JNICALL GetStaticByteField( JNIEnv *env, jclass clazz, jfieldID fieldID );       // index 147
    static JNIEXPORT jchar JNICALL GetStaticCharField( JNIEnv *env, jclass clazz, jfieldID fieldID );       // index 148
    static JNIEXPORT jshort JNICALL GetStaticShortField( JNIEnv *env, jclass clazz, jfieldID fieldID );     // index 149
    static JNIEXPORT jint JNICALL GetStaticIntField( JNIEnv *env, jclass clazz, jfieldID fieldID );         // index 150
    static JNIEXPORT jlong JNICALL GetStaticLongField( JNIEnv *env, jclass clazz, jfieldID fieldID );       // index 151
    static JNIEXPORT jfloat JNICALL GetStaticFloatField( JNIEnv *env, jclass clazz, jfieldID fieldID );     // index 152
    static JNIEXPORT jdouble JNICALL GetStaticDoubleField( JNIEnv *env, jclass clazz, jfieldID fieldID );   // index 153

    static JNIEXPORT void JNICALL SetStaticObjectField( JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value );   // 154
    static JNIEXPORT void JNICALL SetStaticBooleanField( JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value ); // 155
    static JNIEXPORT void JNICALL SetStaticByteField( JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value );       // 156
    static JNIEXPORT void JNICALL SetStaticCharField( JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value );       // 157
    static JNIEXPORT void JNICALL SetStaticShortField( JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value );     // 158
    static JNIEXPORT void JNICALL SetStaticIntField( JNIEnv *env, jclass clazz, jfieldID fieldID, jint value );         // 159
    static JNIEXPORT void JNICALL SetStaticLongField( JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value );       // 160
    static JNIEXPORT void JNICALL SetStaticFloatField( JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value );     // 161
    static JNIEXPORT void JNICALL SetStaticDoubleField( JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value );   // 162

    static JNIEXPORT jstring JNICALL NewString( JNIEnv *env, const jchar *uchars, jsize len );                // index 163
    static JNIEXPORT jsize JNICALL GetStringLength( JNIEnv *env, jstring string );                            // index 164
    static JNIEXPORT const jchar *JNICALL GetStringChars( JNIEnv *env, jstring string, jboolean *isCopy );  // index 165

    static JNIEXPORT void JNICALL ReleaseStringChars( JNIEnv *env, jstring string, const jchar *chars ); // index 166

    static JNIEXPORT jstring JNICALL NewStringUTF( JNIEnv *env, const char *bytes ); // index 167
    static JNIEXPORT jsize JNICALL GetStringUTFLength( JNIEnv *env, jstring string ); // index 168
    static JNIEXPORT const jbyte *JNICALL GetStringUTFChars( JNIEnv *env, jstring string, jboolean *isCopy );  // index 169
    static JNIEXPORT void JNICALL ReleaseStringUTFChars( JNIEnv *env, jstring string, const char *utf ); // index 170

    static JNIEXPORT jsize JNICALL GetArrayLength( JNIEnv *env, jarray array ); // index 171

    static JNIEXPORT jobjectArray JNICALL NewObjectArray( JNIEnv *env, jsize length, jclass elementType, jobject initialElement ); // index 172

    static JNIEXPORT jobject JNICALL GetObjectArrayElement( JNIEnv *env, jobjectArray array, jsize index ); // index 173

    static JNIEXPORT void JNICALL SetObjectArrayElement( JNIEnv *env, jobjectArray array, jsize index, jobject value );  // index 174

    static JNIEXPORT jbooleanArray JNICALL NewBooleanArray( JNIEnv *env, jsize length ); // index 176
    static JNIEXPORT jbyteArray JNICALL NewByteArray( JNIEnv *env, jsize length ); // index 176
    static JNIEXPORT jcharArray JNICALL NewCharArray( JNIEnv *env, jsize length ); // index 177
    static JNIEXPORT jshortArray JNICALL NewShortArray( JNIEnv *env, jsize length ); // index 178
    static JNIEXPORT jintArray JNICALL NewIntArray( JNIEnv *env, jsize length ); // index 179
    static JNIEXPORT jlongArray JNICALL NewLongArray( JNIEnv *env, jsize length ); // index 180
    static JNIEXPORT jfloatArray JNICALL NewFloatArray( JNIEnv *env, jsize length ); // index 181
    static JNIEXPORT jdoubleArray JNICALL NewDoubleArray( JNIEnv *env, jsize length ); // index 182

    static JNIEXPORT jboolean *JNICALL GetBooleanArrayElements( JNIEnv *env, jbooleanArray array, jboolean *isCopy ); // index 183
    static JNIEXPORT jbyte *JNICALL GetByteArrayElements( JNIEnv *env, jbyteArray array, jboolean *isCopy ); // index 184
    static JNIEXPORT jchar *JNICALL GetCharArrayElements( JNIEnv *env, jcharArray array, jchar *isCopy ); // index 185
    static JNIEXPORT jshort *JNICALL GetShortArrayElements( JNIEnv *env, jshortArray array, jshort *isCopy ); // index 186
    static JNIEXPORT jint *JNICALL GetIntArrayElements( JNIEnv *env, jintArray array, jint *isCopy ); // index 187
    static JNIEXPORT jlong *JNICALL GetLongArrayElements( JNIEnv *env, jlongArray array, jlong *isCopy ); // index 188
    static JNIEXPORT jfloat *JNICALL GetFloatArrayElements( JNIEnv *env, jfloatArray array, jfloat *isCopy ); // index 189
    static JNIEXPORT jdouble *JNICALL GetDoubleArrayElements( JNIEnv *env, jdoubleArray array, jdouble *isCopy ); // index 190

    static JNIEXPORT void JNICALL ReleaseBooleanArrayElements( JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode ); // index 191
    static JNIEXPORT void JNICALL ReleaseByteArrayElements( JNIEnv *env, jbyteArray array, jbyte *elems, jint mode ); // index 192
    static JNIEXPORT void JNICALL ReleaseCharArrayElements( JNIEnv *env, jcharArray array, jchar *elems, jint mode ); // index 193
    static JNIEXPORT void JNICALL ReleaseShortArrayElements( JNIEnv *env, jshortArray array, jshort *elems, jint mode ); // index 194
    static JNIEXPORT void JNICALL ReleaseIntArrayElements( JNIEnv *env, jintArray array, jint *elems, jint mode ); // index 195
    static JNIEXPORT void JNICALL ReleaseLongArrayElements( JNIEnv *env, jlongArray array, jlong *elems, jint mode ); // index 196
    static JNIEXPORT void JNICALL ReleaseFloatArrayElements( JNIEnv *env, jfloatArray array, jfloat *elems, jint mode ); // index 197
    static JNIEXPORT void JNICALL ReleaseDoubleArrayElements( JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode ); // index 198

    static JNIEXPORT void JNICALL GetBooleanArrayRegion( JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf ); // index 199
    static JNIEXPORT void JNICALL GetByteArrayRegion( JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf ); // index 200
    static JNIEXPORT void JNICALL GetCharArrayRegion( JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf ); // index 201
    static JNIEXPORT void JNICALL GetShortArrayRegion( JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf ); // index 202
    static JNIEXPORT void JNICALL GetIntArrayRegion( JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf ); // index 203
    static JNIEXPORT void JNICALL GetLongArrayRegion( JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf ); // index 204
    static JNIEXPORT void JNICALL GetFloatArrayRegion( JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf ); // index 205
    static JNIEXPORT void JNICALL GetDoubleArrayRegion( JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf ); // index 206

    static JNIEXPORT void JNICALL SetBooleanArrayRegion( JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf ); // index 207
    static JNIEXPORT void JNICALL SetByteArrayRegion( JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf ); // index 208
    static JNIEXPORT void JNICALL SetCharArrayRegion( JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf ); // index 209
    static JNIEXPORT void JNICALL SetShortArrayRegion( JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf ); // index 210
    static JNIEXPORT void JNICALL SetIntArrayRegion( JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf ); // index 211
    static JNIEXPORT void JNICALL SetLongArrayRegion( JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf ); // index 212
    static JNIEXPORT void JNICALL SetFloatArrayRegion( JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf ); // index 213
    static JNIEXPORT void JNICALL SetDoubleArrayRegion( JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf ); // index 214

    static JNIEXPORT jint JNICALL RegisterNatives( JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods ); // index 215
    static JNIEXPORT jint JNICALL UnregisterNatives( JNIEnv *env, jclass clazz ); // index 216

    static JNIEXPORT jint JNICALL MonitorEnter( JNIEnv *env, jobject obj ); // index 217
    static JNIEXPORT jint JNICALL MonitorExit( JNIEnv *env, jobject obj ); // index 218

    static JNIEXPORT jint JNICALL GetJavaVM( JNIEnv *env, JavaVM **vm ); // index 219

    static JNIEXPORT void JNICALL GetStringRegion( JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf ); // index 220
    static JNIEXPORT void JNICALL GetStringUTFRegion( JNIEnv *env, jstring str, jsize start, jsize len, char *buf ); //index 221
    static JNIEXPORT void *JNICALL GetPrimitiveArrayCritical( JNIEnv *env, jarray array, jboolean *isCopy ); // index 222

    static JNIEXPORT void JNICALL ReleasePrimitiveArrayCritical( JNIEnv *env, jarray array, void *carray, jint mode ); // index 223

    static JNIEXPORT const jchar *JNICALL GetStringCritical( JNIEnv *env, jstring string, jboolean *isCopy ); // index 224
    static JNIEXPORT void JNICALL ReleaseStringCritical( JNIEnv *env, jstring string, const jchar *carray ); // index 225

    static JNIEXPORT jweak JNICALL NewWeakGlobalRef( JNIEnv *env, jobject obj ); // index 226
    static JNIEXPORT void JNICALL DeleteWeakGlobalRef( JNIEnv *env, jobject wref ); // index 227
    static JNIEXPORT jboolean JNICALL ExceptionCheck( JNIEnv *env ); // index 228

    // Special JVMX Methods
    static JNIEXPORT void JNICALL JVMX_arraycopy( JNIEnv *pEnv, jobject obj, jobject src, int srcOffset, jobject dest, int destOffset, int length );

  public:
    static boost::intrusive_ptr<ObjectReference> ConvertJObjectToObjectPointer( jobject obj );
    static boost::intrusive_ptr<ObjectReference> ConvertJObjectToObjectPointer( jstring obj );

    static boost::intrusive_ptr<ObjectReference> ConvertJArrayToArrayPointer( jarray obj );

    //static jobject ConvertObjectPointerToJObject( ObjectReference * ) JVMX_FN_DELETE;
    //static jarray ConvertArrayPointerToJArray( ObjectReference * ) JVMX_FN_DELETE;

    static jobject ConvertObjectPointerToJObject( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pObj );
    static jstring ConvertObjectPointerToJString( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pObj );
    static jarray ConvertArrayPointerToJArray( IVirtualMachineState *pVMState, boost::intrusive_ptr<ObjectReference> pInput );
    static jobject ConvertNullPointerToJObject();

    //static jmethodID ConvertSizeTToJMethodID( size_t methodIndex );
    static void AddToMethodCache( jclass clazz, const JavaString &name, const JavaString &signature, jmethodID methodID );
    static jmethodID LookupInMethodCache( jclass clazz, const JavaString &name, const JavaString &signature );

    // It is the responsibility of the caller to clean this up using delete().
    static JavaArray *CopyStringValueToJavaArray( const char *pValue );

    static void ExecuteMethodImpl( std::shared_ptr<MethodInfo> pMethodInfo, IVirtualMachineState *pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pObject );

    static jfieldID GetFieldIDFromName( const char *name );
    static const JavaString GetNameFromFieldID( jfieldID fieldID );

  private:
    struct MethodKey
    {
      MethodKey( jclass clazz, const JavaString &name, const JavaString &signature )
        : m_Class( clazz )
        , m_Name( name )
        , m_Signature( signature )
      {
      }

      bool operator<( const MethodKey &other ) const;

      jclass m_Class;
      JavaString m_Name;
      JavaString m_Signature;
    };

    static std::map< MethodKey, jmethodID > s_MethodCache;
  };

  struct JNIEnvExported : public JNIEnv
  {
    void *m_pInternal;
  };

  struct JavaVMExported : public JavaVM
  {
    JNIEnvExported *m_pEnv;
  };
} // extern "C"

// Creating a Free Function, for internal use only, that is not Extern "C"
boost::intrusive_ptr<IJavaVariableType> CreateJavaVariableFromJValue( jvalue arg, boost::intrusive_ptr<JavaString> type );

#endif // _JNI_INTERNAL__H_
