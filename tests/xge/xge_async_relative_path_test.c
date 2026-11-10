/* Run from a working directory distinct from the executable directory. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;for(size_t i=0;i<sizeof(symbols)/sizeof(*symbols);++i)if(!strcmp(symbols[i].name,name))return symbols[i].proc;return NULL;}
int main(int argc,char** argv)
{
 assert(argc==3);int missing=atoi(argv[1]);assert(missing==0 || missing==1);
 static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK && xrtMemDebugEnable(true));
 assert(!xrtPathIsAbs(argv[2]));
 static const unsigned char data[]={0,0,2,0,0,0,0,0,0,0,0,0,1,0,1,0,24,32,3,2,1};
 FILE* file=fopen(argv[2],"wb");assert(file);
 assert(fwrite(data,1,sizeof(data),file)==sizeof(data) && fclose(file)==0);
 if(missing)assert(!remove(argv[2]));
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 assert(xgeAsyncThreadingSet(1)==XGE_OK);
 xge_async_owned request=NULL;assert(xgeAsyncImageCreateOwned(&request,NULL)==XGE_OK);
 assert(xgeAsyncImageStartOwned(request,argv[2],XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);
 int status=XGE_ASYNC_LOADING;
 for(int n=0;n<5000 && status==XGE_ASYNC_LOADING;++n){
  assert(xgeAsyncLoadPollOwned(request,&status)==XGE_OK);if(status==XGE_ASYNC_LOADING)xgeSleep(1);
 }
 assert(status==(missing ? XGE_ASYNC_FAILED:XGE_ASYNC_READY));
 xge_async_load_info_t info={0};info.size=sizeof(info);
 assert(xgeAsyncLoadInfoGet(request,&info)==XGE_OK && info.hasResult==!missing && !xrtGetError());
 int failure=xgeAsyncLoadThrowIfFailed(request);
 if(missing){assert(failure==XGE_ERROR_FILE_NOT_FOUND && xrtGetError());xrtClearError();}
 else{
  assert(failure==XGE_OK && !xrtGetError());xge_image_t image={0};
  assert(xgeAsyncImageTakeOwned(request,&image)==XGE_OK && image.iWidth==1 && image.iHeight==1);
  const unsigned char* pixel=image.pPixels;assert(pixel && pixel[0]==1 && pixel[1]==2 && pixel[2]==3 && pixel[3]==255);
  xgeImageFree(&image);assert(!remove(argv[2]));
 }
 bool consumed=false;assert(xgeAsyncLoadReleaseOwnerChecked(request,&consumed)==XGE_OK && consumed);
 xrtMemDebugSnapshot(&after);
 assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes && !after.InvalidFreeCount && !after.DoubleFreeCount && !xrtGetError());
 puts("Relative async read: speculative application-directory errors isolated; real missing-file errors retained");return 0;
}
