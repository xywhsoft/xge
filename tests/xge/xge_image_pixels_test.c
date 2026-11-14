/* Caller storage ends at the last active row; no final padding is required. */
#include "xge.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef __cplusplus
#include <type_traits>
static_assert(std::is_same<decltype(&xgeImageGetPixelView),int(*)(const xge_image_t*,xbytesview*)>::value,"borrow ABI");
static_assert(std::is_same<decltype(&xgeImageCopyPixels),int(*)(const xge_image_t*,void*,size_t)>::value,"copy ABI");
#endif

static void unchanged(const unsigned char* data,size_t size,unsigned char value)
{for(size_t i=0;i<size;++i)assert(data[i]==value);}
int main(void)
{
    unsigned char source[20],destination[24];
    for(size_t i=0;i<sizeof(source);++i)source[i]=(unsigned char)(i+1);
    xge_image_t image={2,2,XGE_PIXEL_RGBA8,12,source,0x80000000u|XGE_IMAGE_STRAIGHT_ALPHA};
    xge_image_t metadata=image;xbytesview view;
    assert(xgeImageGetPixelView(&image,&view)==XGE_OK && view.Data==source && view.Size==20);
    memset(destination,0xCD,sizeof(destination));
    assert(xgeImageCopyPixels(&image,destination,19)==XGE_ERROR_BUFFER_TOO_SMALL);
    unchanged(destination,sizeof(destination),0xCD);
    assert(xgeImageCopyPixels(&image,NULL,20)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageCopyPixels(&image,destination,sizeof(destination))==XGE_OK);
    assert(!memcmp(source,destination,20));unchanged(destination+20,4,0xCD);
    assert(image.iWidth==metadata.iWidth && image.iHeight==metadata.iHeight &&
        image.iFormat==metadata.iFormat && image.iStride==metadata.iStride &&
        image.pPixels==metadata.pPixels && image.iFlags==metadata.iFlags);
    image.iStride=0;assert(xgeImageGetPixelView(&image,&view)==XGE_OK && view.Size==16);
    image.iStride=-7;assert(xgeImageGetPixelView(&image,&view)==XGE_OK && view.Size==16);
    image.iStride=7;
    view.Data=source;view.Size=99;
    assert(xgeImageGetPixelView(&image,&view)==XGE_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    memset(destination,0xCE,sizeof(destination));
    assert(xgeImageCopyPixels(&image,destination,sizeof(destination))==XGE_ERROR_INVALID_ARGUMENT);
    unchanged(destination,sizeof(destination),0xCE);
    image=metadata;image.iFormat=XGE_PIXEL_YUV420P;
    assert(xgeImageGetPixelView(&image,&view)==XGE_ERROR_UNSUPPORTED && !view.Data && !view.Size);
    image=metadata;image.iFormat=XGE_PIXEL_A8;image.iWidth=3;image.iStride=5;
    assert(xgeImageGetPixelView(&image,&view)==XGE_OK && view.Size==8);
    image=metadata;image.iWidth=INT_MAX/4+1;
    assert(xgeImageGetPixelView(&image,&view)==XGE_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    image=metadata;image.iHeight=0;
    assert(xgeImageGetPixelView(&image,&view)==XGE_ERROR_INVALID_ARGUMENT);
    image=metadata;image.pPixels=NULL;
    assert(xgeImageGetPixelView(&image,&view)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageGetPixelView(NULL,&view)==XGE_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    assert(xgeImageGetPixelView(&image,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    /* Overlapping valid spans retain memmove's ordinary C semantics. */
    unsigned char overlap[24],expected[24];
    for(size_t i=0;i<sizeof(overlap);++i)overlap[i]=expected[i]=(unsigned char)i;
    image=metadata;image.pPixels=overlap;memmove(expected+2,expected,20);
    assert(xgeImageCopyPixels(&image,overlap+2,22)==XGE_OK && !memcmp(overlap,expected,24));
    puts("Image pixels: exact span, padded rows, capacity, format, overlap and failure preservation passed");
    return 0;
}
