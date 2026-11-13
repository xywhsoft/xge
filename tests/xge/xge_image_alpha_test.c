/* Pure public image operation: no engine, XRT implementation or GUI is needed.
 * The repro mode uses only an in-bounds padded buffer on the previous SDK. */
#include "xge.h"
#undef NDEBUG
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static const unsigned char source[16]={255,127,1,128,3,5,7,255,100,200,250,0,7,128,250,1};
static const unsigned char result[16]={128,64,1,128,3,5,7,255,0,0,0,0,0,1,1,1};
static int padded(void)
{
    unsigned char pixels[20],expected[20];
    memset(pixels,0xa5,sizeof(pixels));memcpy(pixels,source,8);memcpy(pixels+12,source+8,8);
    memcpy(expected,pixels,sizeof(pixels));memcpy(expected,result,8);memcpy(expected+12,result+8,8);
    xge_image_t image={2,2,XGE_PIXEL_RGBA8,12,pixels,XGE_IMAGE_STRAIGHT_ALPHA};
    xgeImagePremultiply(&image);
    return !memcmp(pixels,expected,sizeof(pixels)) && image.iFlags==XGE_IMAGE_PREMULTIPLIED;
}
static void tight(void)
{
    const int strides[3]={0,-7,8};
    for(unsigned i=0;i<3;++i){
        unsigned char pixels[16];memcpy(pixels,source,sizeof(pixels));
        xge_image_t image={2,2,XGE_PIXEL_RGBA8,strides[i],pixels,UINT32_C(0x40000000)|XGE_IMAGE_STRAIGHT_ALPHA};
        xgeImagePremultiply(&image);
        assert(!memcmp(pixels,result,sizeof(pixels)));
        assert(image.iFlags==(UINT32_C(0x40000000)|XGE_IMAGE_PREMULTIPLIED));
        /* The legacy raw C operation still multiplies on every invocation. */
        xgeImagePremultiply(&image);
        assert(pixels[0]==64 && pixels[1]==32 && pixels[2]==1 && pixels[3]==128);
    }
}
static void invalid(void)
{
    const int shapes[6][3]={{-1,-1,8},{0,2,8},{2,0,8},{2,-2,8},{2,2,7},{INT_MAX/4+1,1,0}};
    for(unsigned i=0;i<6;++i){
        unsigned char pixels[16];memcpy(pixels,source,sizeof(pixels));
        xge_image_t image={shapes[i][0],shapes[i][1],XGE_PIXEL_RGBA8,shapes[i][2],pixels,XGE_IMAGE_STRAIGHT_ALPHA};
        xgeImagePremultiply(&image);
        assert(!memcmp(pixels,source,sizeof(pixels)) && image.iFlags==XGE_IMAGE_STRAIGHT_ALPHA);
    }
    unsigned char pixels[16];memcpy(pixels,source,sizeof(pixels));
    xge_image_t other={2,2,XGE_PIXEL_YUV420P,8,pixels,XGE_IMAGE_STRAIGHT_ALPHA};
    xgeImagePremultiply(&other);assert(!memcmp(pixels,source,sizeof(pixels)));
    other.iFormat=XGE_PIXEL_RGBA8;other.pPixels=NULL;xgeImagePremultiply(&other);
    assert(other.iFlags==XGE_IMAGE_STRAIGHT_ALPHA);xgeImagePremultiply(NULL);
}
#if defined(XGE_TEST_IMAGE_ALPHA_CHECKED) && XGE_TEST_IMAGE_ALPHA_CHECKED
static void checked(void)
{
    const uint32_t states[3]={0,XGE_IMAGE_STRAIGHT_ALPHA,XGE_IMAGE_PREMULTIPLIED};
    for(unsigned i=0;i<3;++i){
        unsigned char pixels[16];memcpy(pixels,i==2?result:source,sizeof(pixels));
        xge_image_t image={2,2,XGE_PIXEL_RGBA8,8,pixels,states[i]|UINT32_C(0x40000000)};
        assert(xgeImagePremultiplyChecked(&image)==XGE_OK);
        assert(!memcmp(pixels,result,sizeof(pixels)));
        assert(image.iFlags==(XGE_IMAGE_PREMULTIPLIED|UINT32_C(0x40000000)));
        assert(xgeImagePremultiplyChecked(&image)==XGE_OK);
        assert(!memcmp(pixels,result,sizeof(pixels)));
    }
    unsigned char pixels[16];memcpy(pixels,source,sizeof(pixels));
    xge_image_t image={2,2,XGE_PIXEL_RGBA8,7,pixels,XGE_IMAGE_PREMULTIPLIED};
    assert(xgeImagePremultiplyChecked(&image)==XGE_ERROR_INVALID_ARGUMENT);
    assert(!memcmp(pixels,source,sizeof(pixels)) && image.iFlags==XGE_IMAGE_PREMULTIPLIED);
    image.iStride=8;image.iFlags=XGE_IMAGE_PREMULTIPLIED|XGE_IMAGE_STRAIGHT_ALPHA;
    assert(xgeImagePremultiplyChecked(&image)==XGE_ERROR_INVALID_ARGUMENT);
    assert(!memcmp(pixels,source,sizeof(pixels)) && image.iFlags==(XGE_IMAGE_PREMULTIPLIED|XGE_IMAGE_STRAIGHT_ALPHA));
    image.iFormat=XGE_PIXEL_YUV420P;
    assert(xgeImagePremultiplyChecked(&image)==XGE_ERROR_UNSUPPORTED);
    assert(!memcmp(pixels,source,sizeof(pixels)));
    image.iFormat=XGE_PIXEL_RGBA8;image.pPixels=NULL;
    assert(xgeImagePremultiplyChecked(&image)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImagePremultiplyChecked(NULL)==XGE_ERROR_INVALID_ARGUMENT);
    puts("Checked image alpha: unmarked/straight/premultiplied, idempotence and unchanged failure passed");
}
#endif
int main(int argc,char** argv)
{
    (void)argv;
    if(!padded()){puts("Image alpha padded-row mismatch");return 83;}
    if(argc==1){
        tight();invalid();
#if defined(XGE_TEST_IMAGE_ALPHA_CHECKED) && XGE_TEST_IMAGE_ALPHA_CHECKED
        checked();
#endif
    }
    puts("Image alpha: row padding, final extent, tight defaults, rounding and legacy repeat passed");
    return 0;
}
