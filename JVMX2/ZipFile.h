#ifndef _ZIPFILE__H_
#define _ZIPFILE__H_

#include <vector>
#include <string>
#include <memory>

#include "DataBuffer.h"

#define ZIP_OK 0
#define ZIP_COULD_NOT_OPEN_FILE -200
#define ZIP_FILE_NOT_FOUND -201

class ZipFile
{
public:
  static const char* GetError(int errorCode);
  static int GetFile(const char* pJarFileName, const char* pFileToFind, DataBuffer& data);
};

#endif // _ZIPFILE__H_

