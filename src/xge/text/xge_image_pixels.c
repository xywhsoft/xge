/* Read-only pixel access, independently linkable without decoder/GL/XRT code. */
#include "xge.h"
#include "../core/xge_byte_rows.h"
#include <string.h>

int xgeImageGetPixelView(const xge_image_t* image,xbytesview* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=(xbytesview){0};
    if(!image || !image->pPixels || image->iWidth<=0 || image->iHeight<=0)
        return XGE_ERROR_INVALID_ARGUMENT;
    size_t channels;
    if(image->iFormat==XGE_PIXEL_RGBA8)channels=4;
    else if(image->iFormat==XGE_PIXEL_A8)channels=1;
    else return XGE_ERROR_UNSUPPORTED;
    if((size_t)image->iWidth>(size_t)INT_MAX/channels)return XGE_ERROR_INVALID_ARGUMENT;
    size_t required;
    if(!__xgeByteRowsLayout((size_t)image->iWidth*channels,image->iHeight,
        image->iStride,SIZE_MAX,&required,NULL))return XGE_ERROR_INVALID_ARGUMENT;
    output->Data=image->pPixels;output->Size=required;
    return XGE_OK;
}
int xgeImageCopyPixels(const xge_image_t* image,void* output,size_t capacity)
{
    xbytesview view;
    int status=xgeImageGetPixelView(image,&view);
    if(status!=XGE_OK)return status;
    if(capacity<view.Size)return XGE_ERROR_BUFFER_TOO_SMALL;
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    memmove(output,view.Data,view.Size);
    return XGE_OK;
}

int xgeImagePixelViewReadByte(const xbytesview* view, size_t offset, unsigned char* output)
{
    if (!view || !view->Data || !output || offset >= view->Size)
        return XGE_ERROR_INVALID_ARGUMENT;
    *output = ((const unsigned char*)view->Data)[offset];
    return XGE_OK;
}
