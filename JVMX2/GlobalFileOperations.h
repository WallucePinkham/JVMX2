#ifndef __GLOBAL_FILE_OPERATIONS_H__
#define __GLOBAL_FILE_OPERATIONS_H__

#ifdef _UNICODE
#define JVMX_FOPEN( filename, mode ) _wfopen( (filename), (mode) )
#else
#define JVMX_FOPEN( filename, mode ) fopen( (filename), (mode) )
#endif // _UNICODE

#define JVMX_FREAD( pDestination, elementSize, count, filePointer ) fread( (pDestination), (elementSize), (count), (filePointer) )

#define JVMX_FTELL ftell

#define JVMX_FSEEK fseek

#endif __GLOBAL_FILE_OPERATIONS_H__

