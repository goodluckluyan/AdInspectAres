#ifndef _BITMAP_DEFINE_H_
#define _BITMAP_DEFINE_H_


#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C"
{
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif
}

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libavutil/avstring.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>

#ifdef __cplusplus
}
#endif

typedef struct BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
    u_int32_t biSize;
    u_int32_t biWidth;
    u_int32_t biHeight;
    u_int16_t biPlanes;
    u_int16_t biBitCount;
    u_int32_t biCompression;
    u_int32_t biSizeImage;
    u_int32_t biXPelsPerMeter;
    u_int32_t biYPelsPerMeter;
    u_int32_t biClrUsed;
    u_int32_t biClrImportant;
}__attribute__((packet)) BITMAPINFODEADER;



#endif
