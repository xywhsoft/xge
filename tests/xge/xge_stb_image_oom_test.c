/* Exercise the actual vendored decoder through its public memory interfaces. */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static int failBudget=-1;
static size_t live;
static void* checked_malloc(size_t bytes)
{
    if(failBudget==0){failBudget=-1;return NULL;}
    if(failBudget>0)--failBudget;
    void* value=malloc(bytes);if(value)++live;return value;
}
static void checked_free(void* value){if(value){assert(live);--live;free(value);}}
static void* checked_realloc(void* value,size_t bytes)
{
    if(!value)return checked_malloc(bytes);
    if(failBudget==0){failBudget=-1;return NULL;}
    if(failBudget>0)--failBudget;
    assert(bytes);return realloc(value,bytes);
}
#define STBI_MALLOC checked_malloc
#define STBI_FREE checked_free
#define STBI_REALLOC checked_realloc
#include "extlibs/stb/stb_image.h"

int main(void)
{
    static const unsigned char png[]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0,0,0,0x0d,0x49,0x48,0x44,0x52,0,0,0,1,0,0,0,1,8,6,0,0,0,0x1f,0x15,0xc4,0x89,0,0,0,0x0d,0x49,0x44,0x41,0x54,0x78,0x9c,0x63,0xf8,0xcf,0xc0,0xf0,0x1f,0,5,0,1,0xff,0x89,0x99,0x3d,0x1d,0,0,0,0,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
    static const unsigned char jpeg[]={0xff,0xd8,0xff,0xe0,0,2,0xff,0xd9};
    static const unsigned char gif[]={'G','I','F','8','9','a',1,0,1,0,0x80,0,0,0,0,0,255,255,255,0x2c,0,0,0,0,1,0,1,0,0,2,2,0x44,1,0,0x3b};
    const unsigned char* samples[]={png,jpeg,gif,gif};const int sizes[]={sizeof(png),sizeof(jpeg),sizeof(gif),19};
    for(int kind=0;kind<4;++kind)for(int budget=0;budget<20;++budget){
        int width=0,height=0,channels=0;failBudget=budget;
        unsigned char* pixels=stbi_load_from_memory(samples[kind],sizes[kind],&width,&height,&channels,4);
        failBudget=-1;checked_free(pixels);assert(!live);
        failBudget=budget;
        (void)stbi_info_from_memory(samples[kind],sizes[kind],&width,&height,&channels);
        failBudget=-1;assert(!live);
    }
    int w=0,h=0,n=0;unsigned char* pixel=stbi_load_from_memory(png,sizeof(png),&w,&h,&n,4);
    assert(pixel && w==1 && h==1 && !memcmp(pixel,"\xff\0\0\xff",4));checked_free(pixel);assert(!live);
    puts("stb image memory load/info: PNG, truncated JPEG and GIF allocation failures passed");return 0;
}
