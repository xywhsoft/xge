/* Cancellation suppresses completion on both the worker and provider paths.
 * Only a local 1x1 TGA fixture is used; no graphics device or network. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
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
static unsigned char imageBytes[]={0,0,2,0,0,0,0,0,0,0,0,0,1,0,1,0,24,32,3,2,1};
static void borrowedBytesFree(void* data,void* user)
{(void)user;if(data!=imageBytes)abort();}
typedef struct State {xge_async_request request;int cancel,callbacks,freeOnComplete,callbackStatus;} State;
static void complete(xge_async_request request,void* user)
{
    State* state=user;if(request==state->request)++state->callbacks;else state->callbacks+=100;
    state->callbackStatus=request->iStatus;
    if(state->freeOnComplete)xgeAsyncRequestFree(request);
}
static int provider(const char* uri,void** data,int* bytes,void* user)
{
    (void)uri;State* state=user;
    if(state->cancel && xgeAsyncRequestCancel(state->request)!=XGE_OK)return XGE_ERROR;
    *data=imageBytes;*bytes=(int)sizeof(imageBytes);return XGE_OK;
}
#define CHECK(value) do{if(!(value)){fprintf(stderr,"async contract line %d: %s\n",__LINE__,#value);return 80;}}while(0)
int main(int argc,char** argv)
{
    CHECK(argc==3);int mode=atoi(argv[1]);CHECK(mode>=1 && mode<=6);
    static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    CHECK(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
    CHECK(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_async_request_t request; xge_image_t image={0};
    xgeAsyncRequestInit(&request);State state={&request,mode<=2,0,mode>=5,0};
    int threaded=mode==1 || mode==3 || mode==5;
    CHECK(xgeAsyncThreadingSet(threaded)==XGE_OK);
    const char* path="cancel://fixture";
    if(threaded){
        FILE* file=fopen(argv[2],"wb");CHECK(file);
        size_t written=fwrite(imageBytes,1,sizeof(imageBytes),file);int closed=fclose(file);
        CHECK(written==sizeof(imageBytes) && !closed);path=argv[2];
    }else{
        xge_resource_provider_t source={"cancel",provider,borrowedBytesFree,&state};
        CHECK(xgeResourceProviderAdd(&source)==XGE_OK);
    }
    int result=xgeAsyncImageLoad(&request,&image,path,XGE_IMAGE_STRAIGHT_ALPHA,complete,&state);
    CHECK(result==(mode==2 ? XGE_ERROR:XGE_OK));
    if(threaded && state.cancel)CHECK(xgeAsyncRequestCancel(&request)==XGE_OK);
    int status=mode==6 ? XGE_ASYNC_READY:XGE_ASYNC_LOADING;
    for(int i=0;i<5000 && status==XGE_ASYNC_LOADING;++i){status=xgeAsyncPoll(&request);if(status==XGE_ASYNC_LOADING)xgeSleep(1);}
    CHECK(status==(state.cancel ? XGE_ASYNC_CANCELLED:XGE_ASYNC_READY));
    if(threaded && state.cancel)CHECK(!image.pPixels);
    if(!state.cancel)CHECK(image.pPixels && image.iWidth==1 && image.iHeight==1);
    int calls=state.callbacks;
    CHECK(xgeAsyncPoll(&request)==(state.freeOnComplete ? XGE_ASYNC_PENDING:status) && state.callbacks==calls);
    if(!state.cancel)CHECK(state.callbackStatus==XGE_ASYNC_READY);
    xgeImageFree(&image);xgeAsyncRequestFree(&request);
    CHECK(xgeResourceProviderClearChecked()==XGE_OK);
    if(threaded)CHECK(!remove(argv[2]));
    xrtMemDebugSnapshot(&after);
    CHECK(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    printf("async mode %d: completion callbacks=%d, expected=%d\n",mode,calls,state.cancel ? 0:1);
    return calls==(state.cancel ? 0:1) ? 0:91;
}
