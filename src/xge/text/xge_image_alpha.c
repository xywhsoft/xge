/* Pure pixel processing, also usable as an independent translation unit. */
#include "xge.h"
#include "../core/xge_byte_rows.h"

static bool __xgeImageAlphaLayout(const xge_image_t* image,int* stride)
{
    return image && image->pPixels && image->iFormat==XGE_PIXEL_RGBA8 &&
        image->iWidth>0 && image->iWidth<=INT_MAX/4 &&
        __xgeByteRowsLayout((size_t)image->iWidth*4,image->iHeight,
            image->iStride,SIZE_MAX,NULL,stride);
}
static void __xgeImagePremultiplySpan(unsigned char* pixels,size_t count)
{
    for(size_t i=0;i<count;++i,pixels+=4){
        unsigned int alpha=(unsigned int)pixels[3];
        pixels[0]=(unsigned char)(((unsigned int)pixels[0]*alpha+127u)/255u);
        pixels[1]=(unsigned char)(((unsigned int)pixels[1]*alpha+127u)/255u);
        pixels[2]=(unsigned char)(((unsigned int)pixels[2]*alpha+127u)/255u);
    }
}
static void __xgeImagePremultiplyApply(xge_image image,int stride)
{
    unsigned char* pixels=(unsigned char*)image->pPixels;
    const int width=image->iWidth,height=image->iHeight;
    if(stride==width*4){
        __xgeImagePremultiplySpan(pixels,(size_t)width*(size_t)height);
    }else{
        for(int y=0;y<height;++y)
            __xgeImagePremultiplySpan(pixels+(size_t)y*(size_t)stride,(size_t)width);
    }
    image->iFlags&=~XGE_IMAGE_STRAIGHT_ALPHA;
    image->iFlags|=XGE_IMAGE_PREMULTIPLIED;
}
void xgeImagePremultiply(xge_image image)
{
    int stride;
    if(__xgeImageAlphaLayout(image,&stride))__xgeImagePremultiplyApply(image,stride);
}
int xgeImagePremultiplyChecked(xge_image image)
{
    int stride;
    if(!image || !image->pPixels)return XGE_ERROR_INVALID_ARGUMENT;
    if(image->iFormat!=XGE_PIXEL_RGBA8)return XGE_ERROR_UNSUPPORTED;
    if(!__xgeImageAlphaLayout(image,&stride))return XGE_ERROR_INVALID_ARGUMENT;
    uint32_t alpha=image->iFlags&(XGE_IMAGE_STRAIGHT_ALPHA|XGE_IMAGE_PREMULTIPLIED);
    if(alpha==(XGE_IMAGE_STRAIGHT_ALPHA|XGE_IMAGE_PREMULTIPLIED))return XGE_ERROR_INVALID_ARGUMENT;
    if(alpha!=XGE_IMAGE_PREMULTIPLIED)__xgeImagePremultiplyApply(image,stride);
    return XGE_OK;
}
