/* Fault-injected aggregate: short fwrite and failing fclose must be checked.
 * All FILE handles are still closed by the real CRT; no OS state is changed. */
#include <stdio.h>
#include <stddef.h>
static int pngTestFailWrite,pngTestFailClose;
static size_t pngTestWrite(const void* data,size_t size,size_t count,FILE* stream)
{
    if(pngTestFailWrite)return count?count-1:0;
    return fwrite(data,size,count,stream);
}
static int pngTestClose(FILE* stream)
{
    int status=fclose(stream);return pngTestFailClose?EOF:status;
}
#define XRT_MODULE_ALL
#define fwrite pngTestWrite
#define fclose pngTestClose
#include "single/xge.c"
#undef fwrite
#undef fclose
#undef NDEBUG
#include <assert.h>
#include <string.h>
int main(void)
{
    const unsigned char pixels[8]={64,0,0,128,0,64,0,128};
    const char* path="image-png-fault-output.png";
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    pngTestFailWrite=1;
    assert(xgeImageSavePNGEx(path,1,2,pixels,4,XGE_IMAGE_STRAIGHT_ALPHA)!=XGE_OK);
    pngTestFailWrite=0;assert(!remove(path));
    pngTestFailClose=1;
    assert(xgeImageSavePNGEx(path,1,2,pixels,4,XGE_IMAGE_PREMULTIPLIED)!=XGE_OK);
    pngTestFailClose=0;assert(!remove(path));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("PNG injected short-write and close failure passed");return 0;
}
