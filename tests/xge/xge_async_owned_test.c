/* Actual owned request graph, private decode, result moves and return tails. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static atomic_int gate=1;
static xthreadproc worker;
static void* workerData;
static int32 gatedWorker(void* unused)
{(void)unused;while(!atomic_load_explicit(&gate,memory_order_acquire))xgeSleep(1);return worker(workerData);}
static xthread* createThread(xthreadproc proc,void* data,size_t stack)
{worker=proc;workerData=data;return xrtThreadCreate(gatedWorker,NULL,stack);}
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;if(!strcmp(name,"xrtThreadCreate"))return (xge_host_xrt_proc)createThread;
    for(size_t i=0;i<sizeof(symbols)/sizeof(*symbols);++i)if(!strcmp(symbols[i].name,name))return symbols[i].proc;
    return NULL;
}
typedef struct Owner {
    unsigned refs,calls,action,failHold;
    uint64_t creator;
    xge_async_owned request;
    xge_image_t taken;
} Owner;
static bool count(const void* data,size_t* output){*output=((const Owner*)data)->refs;return true;}
static bool trace(const void* data,xrtownershipvisitor visit,void* user){(void)data;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* data){Owner* owner=(Owner*)data;if(owner->failHold){--owner->failHold;return false;}++owner->refs;return true;}
static bool drop(const void* data){Owner* owner=(Owner*)data;assert(owner->refs>1);--owner->refs;return true;}
static xge_callback_owner_t descriptor(Owner* owner){return (xge_callback_owner_t){{owner,&ops},hold,drop};}
static xge_async_load_info_t info(xge_async_owned request)
{xge_async_load_info_t value={0};value.size=sizeof(value);assert(xgeAsyncLoadInfoGet(request,&value)==XGE_OK);return value;}
static int callback(void* data)
{
    Owner* owner=data;assert(owner->creator==xrtThreadCurrentId());++owner->calls;
    assert(info(owner->request).status!=XGE_ASYNC_LOADING);
    if(owner->action==5)assert(xgeAsyncLoadCallbackSetOwned(owner->request,NULL,NULL,NULL)==XGE_OK);
    if(owner->action==6){
        bool consumed=false;assert(xgeAsyncLoadReleaseOwnerChecked(owner->request,&consumed)==XGE_OK && consumed);
        assert(info(owner->request).status==XGE_ASYNC_READY);
    }
    if(owner->action==7)return -71;
    if(owner->action==15){
        assert(xgeAsyncLoadResetOwned(owner->request)==XGE_ERROR_BUSY);
        assert(xgeAsyncImageStartOwned(owner->request,"owned://image",0)==XGE_ERROR_BUSY);
        assert(xgeAsyncImageTakeOwned(owner->request,&owner->taken)==XGE_OK);
    }
    return XGE_OK;
}
static unsigned char imageBytes[]={0,0,2,0,0,0,0,0,0,0,0,0,1,0,1,0,24,32,3,2,1};
static void borrowedBytesFree(void* data,void* user)
{(void)user;assert(data==imageBytes);}
typedef struct Source {Owner* owner;bool cancel;int fail;} Source;
static int provider(const char* path,void** data,int* bytes,void* context)
{
    (void)path;Source* source=context;
    if(source->cancel)assert(xgeAsyncLoadCancelOwned(source->owner->request)==XGE_OK);
    if(source->fail){xrtSetErrorKind(XERR_IO);return source->fail;}
    *data=imageBytes;*bytes=(int)sizeof(imageBytes);return XGE_OK;
}
static void settle(xge_async_owned request)
{
    for(int i=0;i<5000;++i){int status=-1;assert(xgeAsyncLoadPollOwned(request,&status)==XGE_OK);
        xge_async_load_info_t value=info(request);
        if(!value.threaded && value.status!=XGE_ASYNC_LOADING)return;
        xgeSleep(1);
    }
    assert(!"decode timeout");
}
static unsigned visited;
static bool visit(xrtownershipref ref,void* expected)
{assert(ref.Data==expected && ref.Ops==&ops);++visited;return true;}
static int32 wrongThread(void* data)
{
    xge_async_owned request=data;xge_async_load_info_t value={0};value.size=sizeof(value);value.result=71;
    assert(xgeAsyncLoadInfoGet(request,&value)==XGE_ERROR_BUSY && value.result==71);
    assert(xgeAsyncImageStartOwned(request,"owned://image",0)==XGE_ERROR_BUSY);
    int status=71;assert(xgeAsyncLoadPollOwned(request,&status)==XGE_ERROR_BUSY && status==71);
    assert(xgeAsyncLoadCancelOwned(request)==XGE_ERROR_BUSY);
    assert(xgeAsyncLoadCallbackSetOwned(request,NULL,NULL,NULL)==XGE_ERROR_BUSY);
    bool consumed=true;assert(xgeAsyncLoadReleaseOwnerChecked(request,&consumed)==XGE_ERROR_BUSY && !consumed);
    return 0;
}
static void pixels(const xge_image_t* image)
{assert(image->iWidth==1 && image->iHeight==1 && image->pPixels);const unsigned char* p=image->pPixels;assert(p[0]==1 && p[1]==2 && p[2]==3 && p[3]==255);}
int main(int argc,char** argv)
{
    assert(argc==3);int mode=atoi(argv[1]);assert(mode>=1 && mode<=23);
    static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK && xrtMemDebugEnable(true));
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&desc)==XGE_OK);
    Owner owner={0};owner.refs=1;owner.creator=xrtThreadCurrentId();Source source={&owner,false,false};
    xge_resource_provider_t providerDesc={"owned",provider,borrowedBytesFree,&source};assert(xgeResourceProviderAdd(&providerDesc)==XGE_OK);
    FILE* file=fopen(argv[2],"wb");assert(file);assert(fwrite(imageBytes,1,sizeof(imageBytes),file)==sizeof(imageBytes));assert(!fclose(file));
    xge_callback_owner_t own=descriptor(&owner);
    /* Warm ownership TLS, callback and decoder paths before heap snapshots. */
    assert(xgeAsyncImageCreateOwned(&owner.request,&own)==XGE_OK);
    assert(xgeAsyncLoadCallbackSetOwned(owner.request,callback,&owner,&own)==XGE_OK);
    assert(xgeAsyncImageStartOwned(owner.request,"owned://image",XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);
    assert(xgeAsyncLoadReleaseChecked(owner.request)==XGE_OK && owner.refs==1);owner.request=NULL;owner.calls=0;
    xrtClearError();xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    bool texture=mode==11 || mode==12;
    assert((texture ? xgeAsyncTextureCreateOwned(&owner.request,&own):xgeAsyncImageCreateOwned(&owner.request,&own))==XGE_OK);
    xge_async_owned request=owner.request;
    if(mode==10 || mode==19){
        assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=owner.request=NULL;
        assert(xgeAsyncThreadingSet(mode==19)==XGE_OK);
        for(int budget=0;budget<24;++budget){
            xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(budget));
            int status=xgeAsyncImageCreateOwned(&request,&own);owner.request=request;
            if(status==XGE_OK)status=xgeAsyncLoadCallbackSetOwned(request,callback,&owner,&own);
            if(status==XGE_OK)status=xgeAsyncImageStartOwned(request,mode==19 ? argv[2]:"owned://image",0);
            if(status==XGE_OK && info(request).threaded)settle(request);
            (void)status;xrtMemDebugFailClear();xrtClearError();
            if(request)assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);
            request=owner.request=NULL;xrtClearError();xrtMemDebugSnapshot(&after);
            assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && owner.refs==1);
        }
    }else if(mode==9){
        xthread* thread=xrtThreadCreate(wrongThread,request,0);assert(thread && xrtThreadWait(thread)==XWAIT_OK);xrtThreadDestroy(thread);
    }else{
        owner.action=(unsigned)mode;
        assert(xgeAsyncLoadCallbackSetOwned(request,callback,&owner,&own)==XGE_OK && owner.refs==3);
        if(mode==18){
            xge_async_owned rejected=(xge_async_owned)(uintptr_t)1;owner.failHold=1;
            assert(xgeAsyncImageCreateOwned(&rejected,&own)!=XGE_OK && !rejected && owner.refs==3);
            owner.failHold=1;assert(xgeAsyncLoadCallbackSetOwned(request,callback,&owner,&own)!=XGE_OK && owner.refs==3);
        }
        bool threaded=mode==2 || mode==4 || mode==12 || mode==13 || mode==16 || mode==17 || mode==21;
        assert(xgeAsyncThreadingSet(threaded)==XGE_OK);
        source.cancel=mode==3 || mode==23;
        source.fail=mode==8 ? XGE_ERROR_FILE_NOT_FOUND:(mode==22 || mode==23) ? XGE_ERROR_CALLBACK_FAILED:0;
        if(mode==16 || mode==17 || mode==21)atomic_store(&gate,0);
        const char* path=threaded ? argv[2]:"owned://image";
        char missing[1024];if(mode==13){assert(snprintf(missing,sizeof(missing),"%s.missing",argv[2])>0);path=missing;}
        int status=texture ? xgeAsyncTextureStartOwned(request,path,XGE_IMAGE_STRAIGHT_ALPHA):xgeAsyncImageStartOwned(request,path,XGE_IMAGE_STRAIGHT_ALPHA);
        assert(status==(mode==7 ? -71:(mode==22 || mode==23) ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
        if(mode==6){request=owner.request=NULL;assert(owner.refs==1 && owner.calls==1);}
        else if(mode==21){
            xrtownershipref ref=xgeAsyncLoadOwnership(request);int token=0;
            const xrtownershipadapterv1* adapter=xgeAsyncLoadOwnershipAdapterV1(ref);
            const xrtownershippreparationv1* prepare=xgeAsyncLoadOwnershipPreparationV1(ref);
            const xrtownershipfinalizationv1* finalize=xgeAsyncLoadOwnershipFinalizationV1(ref);
            const xrtownershipfinishv2* finish=xgeAsyncLoadOwnershipCompletionV2(ref);
            assert(adapter && prepare && finalize && finish && adapter->Claim(ref.Data,&token));
            assert(prepare->Prepare(ref.Data,&token)==XRT_OWNERSHIP_PREPARE_READY);
            bool complete=false;assert(finalize->CanStep(ref.Data,&token) && finalize->Step(ref.Data,&token,&complete) && !complete);
            assert(owner.refs==3);atomic_store(&gate,1);
            for(int i=0;i<5000 && !complete;++i){assert(finalize->Step(ref.Data,&token,&complete));if(!complete)xgeSleep(1);}
            assert(complete && owner.refs==2 && owner.calls==0);adapter->Clear(ref.Data,&token);
            assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY && owner.refs==1);
            assert(finish->DropReady(ref.Data) && finish->DropChecked(ref.Data));request=owner.request=NULL;
        }
        else if(mode==16){
            bool consumed=true;assert(xgeAsyncLoadReleaseOwnerChecked(request,&consumed)==XGE_STATUS_DEFERRED && !consumed && owner.refs==3);
            atomic_store(&gate,1);
            for(int i=0;i<5000 && !consumed;++i){status=xgeAsyncLoadReleaseOwnerChecked(request,&consumed);if(!consumed){assert(status==XGE_STATUS_DEFERRED);xgeSleep(1);}}
            assert(consumed && status==XGE_OK && owner.refs==1 && owner.calls==0);request=owner.request=NULL;
        }else{
            if(mode==4 || mode==17)assert(xgeAsyncLoadCancelOwned(request)==XGE_OK);
            if(mode==17){assert(xgeAsyncLoadResetOwned(request)==XGE_ERROR_BUSY);atomic_store(&gate,1);}
            if(threaded)settle(request);
            xge_async_load_info_t value=info(request);
            if(mode==3 || mode==4 || mode==17){
                assert(value.status==XGE_ASYNC_CANCELLED && !value.hasResult && owner.calls==0);
                assert(xgeAsyncLoadThrowIfFailed(request)!=XGE_OK && xrtGetError());xrtClearError();
            }else if(mode==8 || mode==13 || mode==22 || mode==23){
                assert(value.status==XGE_ASYNC_FAILED && !value.hasResult && owner.calls==((mode==22 || mode==23) ? 0u:1u));
                assert(xgeAsyncLoadThrowIfFailed(request)==((mode==22 || mode==23) ? XGE_ERROR_CALLBACK_FAILED:XGE_ERROR_FILE_NOT_FOUND) && xrtGetError());xrtClearError();
            }else if(mode==14){
                xrtownershipref ref=xgeAsyncLoadOwnership(request);size_t refs=0;assert(ref.Ops->Count(ref.Data,&refs) && refs==1);
                visited=0;assert(ref.Ops->Trace(ref.Data,visit,&owner) && visited==2);
                xge_async_owned shared=NULL;assert(xgeAsyncLoadShareOwned(request,&shared)==XGE_OK && shared==request);
                assert(ref.Ops->Count(ref.Data,&refs) && refs==2 && xgeAsyncLoadReleaseChecked(shared)==XGE_OK);
            }else if(mode==15){pixels(&owner.taken);assert(!value.hasResult);xgeImageFree(&owner.taken);}
            else if(mode==20){
                assert(xrtMemDebugFailAfter(0));
                for(int i=0;i<1000;++i){int state=0;assert(xgeAsyncLoadPollOwned(request,&state)==XGE_OK && state==XGE_ASYNC_READY);assert(info(request).hasResult);}
                assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
            }
            else if(texture){
                assert(value.status==XGE_ASYNC_READY && value.hasResult && xgeGraphicsOwnedResourceCount()==1);
                assert(xgeUnitChecked()==XGE_ERROR_BUSY);xge_texture taken=NULL;
                assert(xrtMemDebugFailAfter(0));assert(xgeAsyncTextureTakeOwned(request,&taken)==XGE_ERROR_OUT_OF_MEMORY && !taken);
                xrtMemDebugFailClear();xrtClearError();assert(info(request).hasResult && xgeGraphicsOwnedResourceCount()==1);
                assert(xgeAsyncTextureTakeOwned(request,&taken)==XGE_OK && taken && !info(request).hasResult && xgeGraphicsOwnedResourceCount()==1);
                assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=owner.request=NULL;
                unsigned char output[4]={0};assert(xgeTextureReadPixelsSized(taken,output,sizeof(output),0)==XGE_OK);
                assert(output[0]==1 && output[1]==2 && output[2]==3 && output[3]==255);xgeTextureFree(taken);
                assert(!xgeGraphicsOwnedResourceCount());
            }else{
                xge_image_t taken={0};assert(value.status==XGE_ASYNC_READY && owner.calls==1);
                assert(xgeAsyncImageTakeOwned(request,&taken)==XGE_OK && !info(request).hasResult);pixels(&taken);
                xge_image_t empty={0};assert(xgeAsyncImageTakeOwned(request,&empty)==XGE_ERROR_INVALID_STATE && !empty.pPixels);
                xgeImageFree(&taken);
            }
            if(request){
                assert(xgeAsyncLoadCallbackSetOwned(request,NULL,NULL,NULL)==XGE_OK);
                assert(xgeAsyncLoadResetOwned(request)==XGE_OK && info(request).status==XGE_ASYNC_PENDING);
            }
        }
    }
    if(request)assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);
    assert(owner.refs==1);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(xgeResourceProviderClearChecked()==XGE_OK);
    assert(xgeUnitChecked()==XGE_OK && !remove(argv[2]));xrtClearError();xrtMemDebugSnapshot(&after);
    /* Provider itself belongs to the earlier baseline and was just cleared. */
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("owned async request/result/graph contract passed");return 0;
}
