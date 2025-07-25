#include <io.h>
#include <iostream>
#include <fcntl.h>

#include "JavaNativeInterface.h"

#include "IVirtualMachineState.h"
//#include "IClassLibrary.h"

#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "NotImplementedException.h"

#include "JavaExceptionConstants.h"

#include "GlobalCatalog.h"
#include "HelperConversion.h"
#include "HelperTypes.h"

#include "ObjectReference.h"

#include "HelperVMChannel.h"


//#include "JavaExceptionConstants.h"

extern const JavaString c_SyntheticField_ClassName;

#ifndef STDIN_FILENO
const int STDIN_FILENO = 0;
const int STDOUT_FILENO = 1;
const int STDERR_FILENO = 2;
#endif // !STDIN_FILENO

const int FILEMODE_READ = 1;
const int FILEMODE_WRITE = 2;
const int FILEMODE_APPEND = 4;

// EXCL is used only when making a temp file.
const int FILEMODE_EXCLUSIVE = 8;
const int FILEMODE_SYNC = 16;
const int FILEMODE_DSYNC = 32;


void JNICALL HelperVMChannel::gnu_java_nio_VMChannel_initIDs( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_initIDs()" );
#endif // _DEBUG

  return;
}

jint JNICALL HelperVMChannel::gnu_java_nio_VMChannel_stdin_fd( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_stdin_fd()" );
#endif // _DEBUG

  return STDIN_FILENO;
}

jint JNICALL HelperVMChannel::gnu_java_nio_VMChannel_stdout_fd( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_stdout_fd()" );
#endif // _DEBUG

  return STDOUT_FILENO;
}

jint JNICALL HelperVMChannel::gnu_java_nio_VMChannel_stderr_fd( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_stderr_fd()" );
#endif // _DEBUG

  return STDERR_FILENO;
}

void JNICALL HelperVMChannel::gnu_java_nio_VMChannel_write( JNIEnv *pEnv, jobject obj, jint fd, jobject byteBufferSrc )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_write(%d, %p)", fd, byteBufferSrc );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( byteBufferSrc );
  if ( nullptr == pObject )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Object parameter was invalid." );
  }

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

  boost::intrusive_ptr<IJavaVariableType> pFieldValue = pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( "backing_buffer" ) );
  if ( e_JavaVariableTypes::Array != pFieldValue->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected array type in backing_buffer." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pFieldValue);

  pFieldValue = pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( "pos" ) );
  if ( e_JavaVariableTypes::Integer != pFieldValue->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer type in pos." );
  }
  boost::intrusive_ptr<JavaInteger> pPos = boost::dynamic_pointer_cast<JavaInteger>(pFieldValue);


  pFieldValue = pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( "limit" ) );
  if ( e_JavaVariableTypes::Integer != pFieldValue->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer type in limit." );
  }
  boost::intrusive_ptr<JavaInteger> pLimit = boost::dynamic_pointer_cast<JavaInteger>(pFieldValue);

  JavaString buffer = pArray->GetContainedArray()->ConvertByteArrayToString();
  if ( buffer.GetLengthInCodePoints() > static_cast< size_t >( pLimit->ToHostInt32() ) )
  {
    buffer = buffer.SubString( 0, pLimit->ToHostInt32() );
  }

  //_setmode( fd, _O_U16TEXT );
  if ( STDOUT_FILENO == fd )
  {
    std::wcout << HelperConversion::ConvertUtf16StringToWideString( buffer.ToCharacterArray() + pPos->ToHostInt32() );
  }
  else if ( STDERR_FILENO == fd )
  {
    std::wcerr << HelperConversion::ConvertUtf16StringToWideString( buffer.ToCharacterArray() + pPos->ToHostInt32() );
  }
  else
  {
    const uint8_t *pbuffer = reinterpret_cast<const uint8_t *>(buffer.ToCharacterArray());
    _write( fd, pbuffer + pPos->ToHostInt32(), buffer.GetLengthInBytes() - pPos->ToHostInt32() );
  }
}

jint JNICALL HelperVMChannel::gnu_java_nio_VMChannel_read(JNIEnv* pEnv, jobject obj, jint fd, jobject byteBufferSrc)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: gnu_java_nio_VMChannel_read(%d, %p)", fd, byteBufferSrc);
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer(byteBufferSrc);
  if (nullptr == pObject)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Object parameter was invalid.");
  }

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<IJavaVariableType> pFieldValue = pObject->GetContainedObject()->GetFieldByName(JavaString::FromCString("backing_buffer"));
  if (e_JavaVariableTypes::Array != pFieldValue->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected array type in backing_buffer.");
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pFieldValue);

  pFieldValue = pObject->GetContainedObject()->GetFieldByName(JavaString::FromCString("pos"));
  if (e_JavaVariableTypes::Integer != pFieldValue->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected integer type in pos.");
  }
  boost::intrusive_ptr<JavaInteger> pPos = boost::dynamic_pointer_cast<JavaInteger>(pFieldValue);


  pFieldValue = pObject->GetContainedObject()->GetFieldByName(JavaString::FromCString("limit"));
  if (e_JavaVariableTypes::Integer != pFieldValue->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected integer type in limit.");
  }
  boost::intrusive_ptr<JavaInteger> pLimit = boost::dynamic_pointer_cast<JavaInteger>(pFieldValue);

  int32_t remaining = pLimit->ToHostInt32() - pPos->ToHostInt32();

  //_setmode( fd, _O_U16TEXT );
  //if (STDIN_FILENO == fd)
  //{
  //  std::string data
  //  std::wcin << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else if (STDERR_FILENO == fd)
  //{
  //  std::wcerr << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else
  //{
    uint8_t* pBuffer = new uint8_t[remaining];
    jint result = -1;
    try
    {
#ifdef _DEBUG
      int64_t pos = _telli64(fd);
#endif // _DEBUG

      result = _read(fd, pBuffer, remaining);

      if (result == 0) // eof
      {
        delete[] pBuffer;
        return -1;
      }

      if (result == -1) // error
      {
        delete[] pBuffer;
        pInternal->ThrowNew(pEnv, pInternal->FindClass(pEnv, c_JavaIOException), strerror(errno));
        return -1;
      }

      // Put bytes back into buffer
      for (int i = 0; i < std::min(result, remaining); ++i)
      {
        pArray->GetContainedArray()->SetAt(pPos->ToHostInt32() + i, JavaByte::FromHostInt8(pBuffer[i]));
      }

      delete[] pBuffer;
    }
    catch (...)
    {
      delete[] pBuffer;
      throw;
    }

    return result;
  //}
}

jlong JNICALL HelperVMChannel::gnu_java_nio_VMChannel_size(JNIEnv* pEnv, jobject obj, jint fd)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: gnu_java_nio_VMChannel_size(%d)", fd);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  //_setmode( fd, _O_U16TEXT );
  //if (STDIN_FILENO == fd)
  //{
  //  std::string data
  //  std::wcin << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else if (STDERR_FILENO == fd)
  //{
  //  std::wcerr << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else
  //{
  jlong oldPos = _telli64(fd);
  jlong fileSize = _lseeki64(fd, 0, SEEK_END);
  _lseeki64(fd, oldPos, SEEK_SET);

  return fileSize;
}

void JNICALL HelperVMChannel::gnu_java_nio_VMChannel_seek(JNIEnv* pEnv, jobject obj, jint fd, jlong pos)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: gnu_java_nio_VMChannel_seek(%d, %lld)", fd, pos);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  //_setmode( fd, _O_U16TEXT );
  //if (STDIN_FILENO == fd)
  //{
  //  std::string data
  //  std::wcin << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else if (STDERR_FILENO == fd)
  //{
  //  std::wcerr << HelperConversion::ConvertUtf16StringToWideString(buffer.ToCharacterArray() + pPos->ToHostInt32());
  //}
  //else
  //{
  _lseeki64(fd, pos, SEEK_SET);
}


void JNICALL HelperVMChannel::gnu_java_nio_VMChannel_close( JNIEnv *pEnv, jobject obj, jint fd )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_close(%d)", fd );
#endif // _DEBUG

  if ( MustCloseFile( fd ) )
  {
    _close( fd );
  }
}

jint JNICALL HelperVMChannel::gnu_java_nio_VMChannel_open( JNIEnv *pEnv, jobject obj, jstring path, jint mode )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_java_nio_VMChannel_open(%p, %d)", path, mode );
#endif // _DEBUG  

  boost::intrusive_ptr<ObjectReference> pPath = JNIEnvInternal::ConvertJObjectToObjectPointer( path );
  if ( nullptr == pPath )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Path parameter was invalid." );
  }

  //boost::intrusive_ptr<ObjectReference> pValue = boost::dynamic_pointer_cast<ObjectReference>( pPath->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"value" ) ) );
  //JavaString pathAsString = pValue->GetContainedArray()->ConvertCharArrayToString();

  JavaString pathAsString = HelperTypes::ExtractValueFromStringObject(pPath);

  int openFlags = 0;
  int openMode = 0;

  if ( (mode & FILEMODE_APPEND) == FILEMODE_APPEND )
  {
    openFlags |= _O_APPEND;
  }

  if ( (mode & FILEMODE_READ) == FILEMODE_READ && (mode & FILEMODE_WRITE) == FILEMODE_WRITE )
  {
    openFlags |= _O_RDWR;
  }
  else if ( (mode & FILEMODE_READ) == FILEMODE_READ )
  {
    openFlags |= _O_RDONLY;
  }
  else if ( (mode & FILEMODE_WRITE) == FILEMODE_WRITE )
  {
    if ( (mode & FILEMODE_APPEND) == 0 )
    {
      openFlags |= _O_TRUNC;
    }

    openFlags |= _O_WRONLY;
  }

  if ( (mode & FILEMODE_EXCLUSIVE) == FILEMODE_EXCLUSIVE )
  {
    openFlags |= _O_CREAT | _O_EXCL;

    if ( (mode & FILEMODE_READ) == FILEMODE_READ )
    {
      openMode |= _S_IREAD;
    }
    
    if ( (mode & FILEMODE_WRITE) == FILEMODE_WRITE )
    {
      openMode |= _S_IWRITE;
    }
  }

  int fd = _open( pathAsString.ToUtf8String().c_str(), openFlags | _O_BINARY, openMode );
  if ( -1 == fd )
  {
    JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
    IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

#if defined(_DEBUG) //&& defined(JVMX_LOG_VERBOSE)
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "*** Could not open file: (%s)", pathAsString.ToUtf8String().c_str() );
#endif // _DEBUG

    pInternal->ThrowNew( pEnv, pInternal->FindClass( pEnv, c_JavaFileNotFoundException), "Could not open file." );
    return 0;
  }

  return fd;
}

bool HelperVMChannel::MustCloseFile( int32_t fileNo )
{
  return fileNo != STDIN_FILENO && fileNo != STDERR_FILENO && fileNo != STDOUT_FILENO;
}