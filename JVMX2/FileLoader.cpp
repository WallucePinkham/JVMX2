#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#elif defined(__linux__)
#include <unistd.h>
#else
#error "Unsupported platform."
#endif

#include "GlobalConstants.h"

#include "FileDoesNotExistException.h"
#include "FileInvalidException.h"
#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "GlobalCatalog.h"
#include "FileSearchPathCollection.h"
#include "JavaString.h"

#include "HelperConversion.h"

#include "FileLoader.h"

FileLoader::FileLoader() JVMX_NOEXCEPT :
m_pFile( nullptr )
{}

FileLoader::~FileLoader() JVMX_NOEXCEPT
{
  try
  {
    Close();
  }
  catch ( ... )
  {
  }
}

void FileLoader::Close()
{
  if ( IsOpen() )
  {
    fclose( m_pFile );
    m_pFile = nullptr;
  }
}

bool FileLoader::IsOpen() const
{
  return nullptr != m_pFile;
}

const char *FileLoader::GetFileName() const
{
  return m_FileName.c_str();
}

void FileLoader::Open( const JVMX_CHAR_TYPE *pFileName )
{
  if ( nullptr == pFileName )
  {
    throw InvalidArgumentException( JVMX_FUNCTION_MESSAGE( " - Filename can't be NULL" ) );
  }

  std::shared_ptr<FileSearchPathCollection> allSearchPaths = GlobalCatalog::GetInstance().Get("SearchPaths");
  auto searchPaths = allSearchPaths->GetPaths();

  std::string fileNameUtf8 = HelperConversion::ConvertJVMXCharToUtf8( pFileName );
  if ( 0 != access( fileNameUtf8.c_str(), 0 ) )
  {
    for ( std::string path : searchPaths)
    {
      if ( 0 == access( (path + fileNameUtf8).c_str(), 0 ) )
      {
        fileNameUtf8 = path + fileNameUtf8;
        break;
      }
    }
  }

  m_pFile = fopen( fileNameUtf8.c_str(), "rb" );

  if ( nullptr == m_pFile )
  {
    throw FileDoesNotExistException( ("File does not exist: " + fileNameUtf8).c_str() );
  }

  m_FileName = fileNameUtf8.c_str();
}

long FileLoader::GetFileSize() const
{
  if ( !IsOpen() )
  {
    throw InvalidStateException( "GetFileSize() called on file that wasn't open." );
  }

  long currentPosition = JVMX_FTELL( m_pFile );
  JVMX_FSEEK( m_pFile, 0, SEEK_END );

  long size = JVMX_FTELL( m_pFile );
  JVMX_FSEEK( m_pFile, currentPosition, SEEK_SET );

  return size;
}

void FileLoader::ReadBytes( uint8_t *pBuffer, size_t length )
{
  if ( !IsOpen() )
  {
    throw InvalidStateException( "ReadBytes() called on file that wasn't open." );
  }

  size_t elementsRead = JVMX_FREAD( pBuffer, length, 1, m_pFile );

  if ( elementsRead < 1 )
  {
    throw FileInvalidException( JVMX_FUNCTION_MESSAGE( " - Could not read bytes." ) );
  }
}

void FileLoader::AddSearchPath(const JavaString& path)
{
    std::shared_ptr<FileSearchPathCollection> paths = GlobalCatalog::GetInstance().Get("SearchPaths");
#if defined(__linux__)
    const std::string utf8Path = path.ToUtf8String();
    paths->AddSearchPath(utf8Path.c_str());
#else
    paths->AddSearchPath(path.ToCharacterArray());
#endif
}

void FileLoader::AddSearchPath(const JVMX_CHAR_TYPE* pPath)
{
    //m_SearchPaths.push_back( HelperConversion::ConvertJVMXCharToUtf8( pPath ) );
    std::shared_ptr<FileSearchPathCollection> paths = GlobalCatalog::GetInstance().Get("SearchPaths");
    paths->AddSearchPath(pPath);
}