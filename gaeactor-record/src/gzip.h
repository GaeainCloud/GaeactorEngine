#ifndef GZIP_H
#define GZIP_H
#include <qsystemdetection.h>
#ifdef Q_OS_WIN32
#include "zlib.h"
#elif defined(Q_OS_LINUX)
#include "zlib.h"
#endif

/* Compress gzip data */
/* data 原数据 ndata 原数据长度 zdata 压缩后数据 nzdata 压缩后长度 */
int gzcompress(Bytef *data,
           uLong ndata,
           Bytef *zdata,
           uLong nzdata);






/* Uncompress gzip data */


/* zdata 数据 nzdata 原数据长度 data 解压后数据 ndata 解压后长度 */
int gzdecompress(Byte *zdata,
                 uLong nzdata,
                 Byte *data,
                 uLong ndata);



// gzCompress: do the compressing
int gzCompress(const char *src, int srcLen, char *dest, int destLen);

// gzDecompress: do the decompressing
int gzDecompress(const char *src, int srcLen, const char *dst, int dstLen);
#endif // GZIP_H
