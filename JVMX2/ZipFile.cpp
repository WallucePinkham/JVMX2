#include "ZipFile.h"

#include "Zip/unzip.h"



#define CASE_SENSITIVE 1
#define NOT_CASE_SENSITIVE 2

int ZipFile::GetFile(const char* pJarFileName, const char* pFileToFind, DataBuffer& data)
{
  unzFile hFile = unzOpen(pJarFileName);
  if (!hFile)
  {
    return ZIP_COULD_NOT_OPEN_FILE;
  }

  unz_global_info  globalInfo = { 0 };
  int res = unzGetGlobalInfo(hFile, &globalInfo);
  if (res != UNZ_OK)
  {
    return res;
  }

  res = unzLocateFile(hFile, pFileToFind, NOT_CASE_SENSITIVE);
  if (res != UNZ_OK)
  {
    return ZIP_FILE_NOT_FOUND;
  }

  res = unzOpenCurrentFile(hFile);
  if (res != UNZ_OK)
  {
    return res;
  }

  const int bufferSize = 32768;
  std::unique_ptr<uint8_t[]> buffer(new uint8_t[bufferSize]);
  memset(buffer.get(), 0, bufferSize);

  int ReadSize, Totalsize = 0;
  while ((ReadSize = unzReadCurrentFile(hFile, buffer.get(), bufferSize)) > 0)
  {
    Totalsize += ReadSize;
    data = data.Append(static_cast<size_t>(ReadSize), buffer.get());
  }

  unzCloseCurrentFile(hFile);
  unzClose(hFile);

  return 0;
}

const char* ZipFile::GetError(int errorCode)
{
  switch (errorCode)
  {
  case ZIP_COULD_NOT_OPEN_FILE:
    return "Zip / Jar file could not be opened.";

  case ZIP_FILE_NOT_FOUND:
    return "The requested file was not found inside the Zip/Jar.";

  case Z_ERRNO:
    return "File system error.";

  case Z_STREAM_ERROR:
    return "Compression stream state was inconsistent.";

  case Z_DATA_ERROR:
    return "Compression data error";

  case Z_MEM_ERROR:
    return "Out of memory during decompression.";

  case Z_BUF_ERROR:
    return "Non-fatal buffer error. This probably indicates a bug in the code calling zlib.";

  case Z_VERSION_ERROR:
    return "zlib library version (zlib_version) is incompatible with the version assumed by the caller";

  case UNZ_PARAMERROR:
    return "Unzip was called with bad parameters. Typically the file pointer is NULL.";

  case UNZ_BADZIPFILE:
    return "Invalid zip file. The magic number doesn't match.";

  case UNZ_INTERNALERROR:
    return "Unzip internal error.";

  case UNZ_CRCERROR:
    return "CRC Check failed.";
  }

  return "Unknown Error.";
}
