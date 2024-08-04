
#ifndef __FILELOADER_H__
#define __FILELOADER_H__

#include "GlobalConstants.h"
#include "FileDoesNotExistException.h"

#include <cstdio>
#include <string>
#include <vector>

class FileLoader
{
public:
  FileLoader() JVMX_NOEXCEPT;

  virtual ~FileLoader() JVMX_NOEXCEPT;

  virtual bool IsOpen() const;

  virtual void Close();

  virtual const char *GetFileName() const;

  virtual void Open( const JVMX_CHAR_TYPE * pFileName );

  virtual long GetFileSize() const;

  virtual void ReadBytes( uint8_t *pBuffer, size_t length );

  virtual void AddSearchPath( const JVMX_CHAR_TYPE *pPath );
  virtual void AddSearchPath(const JavaString& path);

private:
  FileLoader( FileLoader &other );

  FileLoader &operator=(FileLoader &other) const;

protected:
  FILE *m_pFile;
  std::string m_FileName;

  //std::vector< std::string > m_SearchPaths;
};

#endif // __IFILELOADER_H__
