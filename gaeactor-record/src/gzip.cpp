#include "gzip.h"

int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong nzdata)
{
    z_stream c_stream;
    int err = 0;
    if(data && ndata > 0)
    {
        c_stream.zalloc = nullptr;
        c_stream.zfree = nullptr;
        c_stream.opaque = nullptr;
        //只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
        if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            return -1;
        }
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out  = nzdata;

        while(c_stream.avail_in != 0 && c_stream.total_out < nzdata)
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
            {
                return -1;
            }
        }

        if(c_stream.avail_in != 0)
        {
            return c_stream.avail_in;
        }

        for(;;)
        {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
            {
                break;
            }

            if(err != Z_OK)
            {
                return -1;
            }
        }

        if(deflateEnd(&c_stream) != Z_OK)
        {
            return -1;
        }

        return c_stream.total_out;
    }
    return -1;
}

int gzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong ndata)

{
    int err = 0;
    z_stream d_stream = {0};
    /* decompression stream */
    static char dummy_head[2] = { 0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF};
    d_stream.zalloc = nullptr;
    d_stream.zfree = nullptr;
    d_stream.opaque = nullptr;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;
    //只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
    if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK)
    {
        return -1;
    }
    //if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
    while(d_stream.total_out < ndata && d_stream.total_in < nzdata)
    {
        d_stream.avail_in = d_stream.avail_out = 1;
        /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
        {
            break;
        }

        if(err != Z_OK)
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
    }

    if(inflateEnd(&d_stream) != Z_OK)
    {
        return -1;
    }
    return d_stream.total_out;
}


// gzCompress: do the compressing
int gzCompress(const char *src, int srcLen, char *dest, int destLen)
{
    z_stream c_stream;
    int err = 0;
    int windowBits = 15;
    int GZIP_ENCODING = 16;

    if(src && srcLen > 0)
    {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                    windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            return -1;
        }
        c_stream.next_in  = (Bytef *)src;
        c_stream.avail_in  = srcLen;
        c_stream.next_out = (Bytef *)dest;
        c_stream.avail_out  = destLen;
        while (c_stream.avail_in != 0 && c_stream.total_out < destLen)
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
            {
                return -1;
            }
        }

        if(c_stream.avail_in != 0)
        {
            return c_stream.avail_in;
        }
        for (;;)
        {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
            {
                break;
            }
            if(err != Z_OK)
            {
                return -1;
            }
        }
        if(deflateEnd(&c_stream) != Z_OK)
        {
            return -1;
        }
        return c_stream.total_out;
    }
    return -1;
}

// gzDecompress: do the decompressing
int gzDecompress(const char *src, int srcLen, const char *dst, int dstLen){
    z_stream strm;
    strm.zalloc=nullptr;
    strm.zfree=nullptr;
    strm.opaque=nullptr;

    strm.avail_in = srcLen;
    strm.avail_out = dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err=-1, ret=-1;
    err = inflateInit2(&strm, MAX_WBITS+16);
    if (err == Z_OK)
    {
        err = inflate(&strm, Z_FINISH);
        if (err == Z_STREAM_END)
        {
            ret = strm.total_out;
        }
        else
        {
            inflateEnd(&strm);
            {
                return err;
            }
        }
    }
    else
    {
        inflateEnd(&strm);
        {
            return err;
        }
    }
    inflateEnd(&strm);
    return err;
}
