/* Actual synchronous image resource + asynchronous native request code.
 * CPU codec/backend only; no engine/device/window/browser/network. */
#define main xui_unused_image_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#include "xui_proxy_resource.h"
#include "../../include/xui_document_ui.h"
#include "../../src/xui/document/xui_document_image_resource.c"
static int imageDecodeGate(xge_image,const void*,int,uint32_t);
#define xgeImageLoadMemoryEx imageDecodeGate
#include "../../src/xui/document/xui_document_image_async.c"
#undef xgeImageLoadMemoryEx

typedef struct ImageTest {
    xui_test_proxy_state_t state;
    Owner owner;
    xui_proxy proxy;
    xui_context context;
    unsigned created,dropped;
    int loadResult,dropResult;
    unsigned char failPixel;
    bool removeOnRead,closeOnLoad,uploadReentry;
    xui_doc_image_request target,peer;
    atomic_int block,entered,allow;
} ImageTest;
typedef struct ImageSurface {ImageTest* test;xui_surface_desc_t desc;int result;} ImageSurface;
static const unsigned char rgba[16]={1,2,3,255,4,5,6,255,7,8,9,255,10,11,12,255};
static void* pngData;static size_t pngBytes;
static int imageSurfaceDrop(xui_proxy proxy,void* raw)
{(void)proxy;ImageSurface* surface=raw;++surface->test->dropped;return surface->result;}
static int imageCreate(xui_proxy proxy,xui_surface* output,int width,int height)
{
    ImageTest* test=proxy->pUser;void* raw=NULL;
    int status=xuiSurfaceCreateOwned(proxy,sizeof(ImageSurface),imageSurfaceDrop,output,&raw);
    if(status==XUI_OK){
        *(ImageSurface*)raw=(ImageSurface){test,{.iKind=XUI_SURFACE_KIND_TEXTURE,.iFormat=XUI_SURFACE_FORMAT_RGBA8,.iWidth=width,.iHeight=height},test->dropResult};
        ++test->created;
    }
    return status;
}
static int imageLoad(xui_proxy proxy,xui_surface* output,const void* data,int bytes,uint32_t flags)
{
    (void)flags;assert(data && bytes>0);ImageTest* test=proxy->pUser;
    int status=imageCreate(proxy,output,2,2);
    if(test->closeOnLoad){test->closeOnLoad=false;xuiDestroy(test->context);}
    return status==XUI_OK ? test->loadResult:status;
}
static int imageUpload(xui_proxy proxy,xui_surface* output,int width,int height,const void* pixels,int stride,uint32_t flags)
{
    (void)flags;assert(pixels && stride>=width*4);ImageTest* test=proxy->pUser;
    int status=imageCreate(proxy,output,width,height);
    if(test->uploadReentry){
        test->uploadReentry=false;xui_resource none=NULL;
        assert(xuiDocumentImageResourcePoll(test->target,&none)==XUI_DOC_ERROR_BUSY && !none);
        assert(xuiDocumentImageResourceReleaseChecked(test->peer)==XUI_OK);
        assert(xuiDocumentImageResourceReleaseChecked(test->target)==XUI_OK);
    }
    return status==XUI_OK && (!test->failPixel || *(const unsigned char*)pixels==test->failPixel) ? test->loadResult:status;
}
static int imageDesc(xui_proxy proxy,xui_surface surface,xui_surface_desc_t* output)
{assert(xuiSurfaceGetProxy(surface)==proxy);*output=((ImageSurface*)xuiInternalProxyResourcePayload(surface))->desc;return XUI_OK;}
static int imageRead(xui_proxy proxy,xui_surface surface,void* pixels,int stride)
{
    ImageTest* test=proxy->pUser;assert(xuiSurfaceGetProxy(surface)==proxy && stride==8);
    if(test->removeOnRead){
        test->removeOnRead=false;assert(xuiResourceRemove(xuiResourceFind(test->context,"image"))==XUI_OK);
        assert(test->dropped==0 && refs(xuiSurfaceOwnership(surface))==1);
    }
    memcpy(pixels,rgba,sizeof(rgba));return XUI_OK;
}
static void imageSetup(ImageTest* test)
{
    memset(test,0,sizeof(*test));atomic_init(&test->block,0);atomic_init(&test->entered,0);atomic_init(&test->allow,1);
    xuiTestProxyInit(&test->state);test->owner.refs=1;test->state.tProxy.pUser=test;
    test->state.tProxy.surfaceLoadMemory=imageLoad;test->state.tProxy.surfaceCreateRGBA=imageUpload;
    test->state.tProxy.surfaceGetDesc=imageDesc;test->state.tProxy.surfaceReadRGBA=imageRead;
    xui_callback_owner_t owner=ownerRef(&test->owner);
    assert(xuiProxyCreateOwned(&test->state.tProxy,&owner,&test->proxy)==XUI_OK && xuiCreateOwned(test->proxy,&test->context)==XUI_OK);
}
static void imageProxyFinish(ImageTest* test)
{assert(xuiProxyReleaseChecked(test->proxy)==XUI_OK && drop(&test->owner));}
static void imageFinish(ImageTest* test)
{assert(xuiDestroyChecked(test->context)==XUI_OK && test->created==test->dropped);imageProxyFinish(test);}
static int imageDecodeGate(xge_image image,const void* encoded,int bytes,uint32_t flags)
{
    xui_doc_image_request job=(void*)((unsigned char*)image - offsetof(struct xui_doc_image_request_t,image));
    ImageTest* test=doc_image_job_proxy(job->context)->pUser;
    if(atomic_load_explicit(&test->block,memory_order_acquire)){
        atomic_fetch_add_explicit(&test->entered,1,memory_order_release);
        while(!atomic_load_explicit(&test->allow,memory_order_acquire))xrtSleep(1);
    }
    return xgeImageLoadMemoryEx(image,encoded,bytes,flags);
}
static void waitEntered(ImageTest* test,int count)
{
    for(unsigned i=0;i<10000 && atomic_load_explicit(&test->entered,memory_order_acquire)<count;++i)xrtSleep(1);
    assert(atomic_load_explicit(&test->entered,memory_order_acquire)>=count);
}
static void waitRequest(xui_doc_image_request job)
{assert(job->worker && xrtThreadWaitFor(job->worker,10000)==XWAIT_OK);}
static void indexEmpty(void)
{doc_image_index_enter();assert(!doc_image_requests);doc_image_index_leave();}
static xrtownershipref matchedSurface;static unsigned matchedEdges;
static bool imageVisit(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==matchedSurface.Data && ref.Ops==matchedSurface.Ops)++matchedEdges;return true;}
static void syncRoundTrip(bool failedCleanup)
{
    ImageTest test;imageSetup(&test);test.dropResult=failedCleanup ? XUI_ERROR_BACKEND_FAILED:XUI_OK;
    xui_resource resource=NULL;assert(xuiDocumentImageResourceLoadMemory(test.context,"image",pngData,pngBytes,NULL,&resource)==XUI_OK && resource);
    xui_surface surface=xuiResourceGetHandle(resource);assert(refs(xuiSurfaceOwnership(surface))==1 && test.created==1 && !test.dropped);
    matchedSurface=xuiSurfaceOwnership(surface);matchedEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(test.context,imageVisit,NULL) && matchedEdges==1);assert(xrtOwnershipScopeEnd(&freeze));
    test.removeOnRead=true;void* encoded=NULL;size_t bytes=0;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);
    int status=doc_image_resource_export_png(test.context,"image",&encoded,&bytes);
    assert(status==(failedCleanup ? XUI_ERROR_BACKEND_FAILED:XUI_OK) && test.dropped==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==(failedCleanup ? XUI_ERROR_BACKEND_FAILED:XUI_OK) && !xrtGetError());
    if(failedCleanup)assert(!encoded && !bytes);
    else{int width=0,height=0;assert(encoded && bytes && xgeImageInfoMemory(encoded,(int)bytes,&width,&height)==XGE_OK && width==2 && height==2);xrtFree(encoded);}
    imageFinish(&test);
}
static void syncFailures(void)
{
    ImageTest test;imageSetup(&test);test.loadResult=XUI_ERROR_OUT_OF_MEMORY;test.dropResult=XUI_ERROR_BACKEND_FAILED;
    xui_resource resource=(void*)(uintptr_t)1;xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);
    assert(xuiDocumentImageResourceLoadMemory(test.context,"image",pngData,pngBytes,NULL,&resource)==XUI_ERROR_OUT_OF_MEMORY && !resource && !xrtGetError());
    assert(test.created==1 && test.dropped==1 && xuiCallScopeEnd(&outer)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError());imageFinish(&test);
    ImageTest other;imageSetup(&test);imageSetup(&other);xui_surface foreign=NULL;assert(imageCreate(other.proxy,&foreign,2,2)==XUI_OK);
    assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);
    assert(doc_image_resource_register_surface(test.context,"bad",foreign,&resource)==XUI_ERROR_INVALID_ARGUMENT && !resource && refs(xuiSurfaceOwnership(foreign))==1 && !other.dropped);
    assert(xuiCallScopeEnd(&outer)==XUI_OK);bool consumed=false;assert(xuiSurfaceReleaseOwnerChecked(foreign,&consumed)==XUI_OK && consumed);
    imageFinish(&test);imageFinish(&other);
}
static void countGone(void* raw){++*(unsigned*)raw;}
static void syncReentrantContextClose(void)
{
    ImageTest test;imageSetup(&test);unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(test.context,&watch,countGone,&gone)==XUI_OK);
    test.closeOnLoad=true;xui_resource resource=(void*)(uintptr_t)1;
    assert(xuiDocumentImageResourceLoadMemory(test.context,"image",pngData,pngBytes,NULL,&resource)==XUI_ERROR_INVALID_STATE && !resource);
    assert(gone==1 && !watch.ppPrevious && test.created==1 && test.dropped==1);imageProxyFinish(&test);
}
static xui_doc_image_request newRequest(ImageTest* test,const char* name)
{
    xui_doc_image_request job=NULL;assert(xuiDocumentImageResourceLoadMemoryAsync(test->context,name,pngData,pngBytes,NULL,&job)==XUI_OK && job);return job;
}
static void asyncRoundTrip(bool cancelled)
{
    ImageTest test;imageSetup(&test);xui_doc_image_request job=newRequest(&test,"image");
    if(cancelled)assert(xuiDocumentImageResourceCancel(job)==XUI_OK);
    waitRequest(job);xui_resource resource=NULL;
    assert(xuiDocumentImageResourcePoll(job,&resource)==(cancelled ? XUI_DOC_ERROR_CANCELLED:XUI_OK));
    assert((resource!=NULL)==!cancelled && xuiDocumentImageResourceReleaseChecked(job)==XUI_OK);xrtClearError();imageFinish(&test);indexEmpty();
}
static void asyncBlockedContextClose(void)
{
    ImageTest test;imageSetup(&test);atomic_store(&test.block,1);atomic_store(&test.allow,0);
    xui_doc_image_request job=newRequest(&test,"image");waitEntered(&test,1);
    unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(test.context,&watch,countGone,&gone)==XUI_OK);
    bool consumed=true;assert(xuiDestroyOwnerChecked(test.context,&consumed)==XUI_STATUS_DEFERRED && !consumed && !gone);
    assert(atomic_load(&job->cancelled) && job->worker && xrtThreadState(job->worker)==XTHREAD_RUNNING && !test.created);
    atomic_store_explicit(&test.allow,1,memory_order_release);waitRequest(job);assert(!gone);
    assert(xuiDestroyOwnerChecked(test.context,&consumed)==XUI_OK && consumed && gone==1 && !test.created);
    imageProxyFinish(&test);indexEmpty();
}
static void asyncFailureClassification(bool failedCleanup)
{
    ImageTest test;imageSetup(&test);xui_doc_image_request job=newRequest(&test,"image");waitRequest(job);
    test.loadResult=XUI_ERROR_OUT_OF_MEMORY;test.dropResult=failedCleanup ? XUI_ERROR_BACKEND_FAILED:XUI_OK;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);
    xui_resource resource=(void*)(uintptr_t)1;
    assert(xuiDocumentImageResourcePoll(job,&resource)==XUI_ERROR_OUT_OF_MEMORY && !resource && test.created==1 && test.dropped==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==(failedCleanup ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK) && !xrtGetError());
    assert(xuiDocumentImageResourceReleaseChecked(job)==XUI_OK);imageFinish(&test);indexEmpty();
}
static void asyncPeerFailureLocal(void)
{
    ImageTest test;imageSetup(&test);xui_doc_image_request success=newRequest(&test,"success");
    unsigned char other[16];memcpy(other,rgba,sizeof(other));other[0]=77;void* encoded=NULL;size_t bytes=0;
    assert(xgeImageEncodePNGEx(2,2,other,8,XGE_IMAGE_STRAIGHT_ALPHA,&encoded,&bytes)==XGE_OK);
    xui_doc_image_request failure=NULL;assert(xuiDocumentImageResourceLoadMemoryAsync(test.context,"failure",encoded,bytes,NULL,&failure)==XUI_OK);
    xrtFree(encoded);waitRequest(success);waitRequest(failure);test.loadResult=XUI_ERROR_OUT_OF_MEMORY;test.failPixel=77;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);xui_resource resource=NULL;
    assert(xuiDocumentImageResourcePoll(success,&resource)==XUI_OK && resource && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_OK && !xrtGetError());
    assert(xuiDocumentImageResourcePoll(failure,NULL)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError());
    assert(xuiDocumentImageResourceReleaseChecked(success)==XUI_OK && xuiDocumentImageResourceReleaseChecked(failure)==XUI_OK);
    imageFinish(&test);indexEmpty();
}
static void asyncPeerDiagnosticLocal(void)
{
    ImageTest test;imageSetup(&test);xui_doc_image_request success=newRequest(&test,"success"),failure=NULL;
    assert(xuiDocumentImageResourceLoadFileAsync(test.context,"failure","__xui_owned_image_missing_20261105__.png",NULL,&failure)==XUI_OK);
    waitRequest(success);waitRequest(failure);
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.context,&outer)==XUI_OK);xui_resource resource=NULL;
    assert(xuiDocumentImageResourcePoll(success,&resource)==XUI_OK && resource && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_OK && !xrtGetError());
    assert(xuiDocumentImageResourcePoll(failure,NULL)==XUI_DOC_ERROR_IO && xrtGetError());xrtClearError();
    assert(xuiDocumentImageResourceReleaseChecked(success)==XUI_OK && xuiDocumentImageResourceReleaseChecked(failure)==XUI_OK);
    imageFinish(&test);indexEmpty();
}
static void asyncQueuedCancellation(void)
{
    ImageTest test;imageSetup(&test);atomic_store(&test.block,1);atomic_store(&test.allow,0);
    xui_doc_image_request jobs[4];char name[8];
    for(unsigned i=0;i<4;++i){snprintf(name,sizeof(name),"job%u",i);jobs[i]=newRequest(&test,name);}
    waitEntered(&test,4);xui_doc_image_request queued=newRequest(&test,"queued");assert(!queued->started && !queued->worker);
    assert(xuiDocumentImageResourceCancel(queued)==XUI_OK && queued->terminal && !queued->encoded);
    assert(xuiDocumentImageResourceReleaseChecked(queued)==XUI_OK);
    for(unsigned i=0;i<4;++i)assert(xuiDocumentImageResourceReleaseChecked(jobs[i])==XUI_OK);
    atomic_store_explicit(&test.allow,1,memory_order_release);
    for(unsigned i=0;i<4;++i)waitRequest(jobs[i]);
    xui_call_scope_t boundary={0};assert(xuiCallScopeBegin(test.context,&boundary)==XUI_OK && xuiCallScopeEnd(&boundary)==XUI_OK);
    imageFinish(&test);indexEmpty();
}
static void asyncPeerClose(void)
{
    ImageTest test;imageSetup(&test);test.target=newRequest(&test,"target");test.peer=newRequest(&test,"peer");
    waitRequest(test.target);waitRequest(test.peer);test.uploadReentry=true;xui_resource resource=(void*)(uintptr_t)1;
    assert(xuiDocumentImageResourcePoll(test.target,&resource)==XUI_ERROR_INVALID_STATE && !resource && !test.uploadReentry);
    assert(!test.context->pOwnedManagers && !test.context->pPendingManagers && test.created==1 && test.dropped==1);
    imageFinish(&test);indexEmpty();
}
static void asyncConstructorOom(void)
{
    for(size_t budget=0;budget<24;++budget){
        ImageTest test;imageSetup(&test);xui_doc_image_request job=(void*)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(budget));
        int status=xuiDocumentImageResourceLoadMemoryAsync(test.context,"image",pngData,pngBytes,NULL,&job);
        xrtMemDebugFailClear();xrtClearError();
        if(status==XUI_OK){
            assert(job);if(job->worker)waitRequest(job);
            assert(xuiDocumentImageResourceReleaseChecked(job)==XUI_OK);
        }else assert(!job);
        assert(!test.context->pOwnedManagers && !test.context->pPendingManagers);
        imageFinish(&test);indexEmpty();
    }
}
typedef struct CreatorWork {atomic_int* ready;atomic_int* go;unsigned completed;} CreatorWork;
static int32 parallelCreator(ptr raw)
{
    CreatorWork* work=raw;atomic_fetch_add_explicit(work->ready,1,memory_order_release);
    while(!atomic_load_explicit(work->go,memory_order_acquire))xrtSleep(1);
    for(unsigned i=0;i<20;++i){
        ImageTest test;imageSetup(&test);xui_doc_image_request job=newRequest(&test,"image");
        assert(xuiDocumentImageResourceCancel(job)==XUI_OK);waitRequest(job);
        assert(xuiDocumentImageResourcePoll(job,NULL)==XUI_DOC_ERROR_CANCELLED && xuiDocumentImageResourceReleaseChecked(job)==XUI_OK);
        xrtClearError();imageFinish(&test);++work->completed;
    }
    return 0;
}
static void independentCreators(void)
{
    atomic_int ready=0,go=0;CreatorWork work[2]={{&ready,&go,0},{&ready,&go,0}};xthread* threads[2];
    for(unsigned i=0;i<2;++i){threads[i]=xrtThreadCreate(parallelCreator,&work[i],0);assert(threads[i]);}
    while(atomic_load_explicit(&ready,memory_order_acquire)!=2)xrtSleep(1);
    atomic_store_explicit(&go,1,memory_order_release);
    for(unsigned i=0;i<2;++i){assert(xrtThreadWaitFor(threads[i],10000)==XWAIT_OK && xrtThreadExitCode(threads[i])==0 && work[i].completed==20);xrtThreadDestroy(threads[i]);}
    indexEmpty();
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xgeImageEncodePNGEx(2,2,rgba,8,XGE_IMAGE_STRAIGHT_ALPHA,&pngData,&pngBytes)==XGE_OK && pngData && pngBytes);
    syncRoundTrip(false);syncRoundTrip(true);syncFailures();syncReentrantContextClose();
    asyncRoundTrip(false);asyncRoundTrip(true);asyncFailureClassification(false);asyncFailureClassification(true);asyncPeerFailureLocal();asyncPeerDiagnosticLocal();
    asyncBlockedContextClose();asyncQueuedCancellation();asyncPeerClose();asyncConstructorOom();independentCreators();
    xrtFree(pngData);xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("Document images: actual Surface register/read pin + checked first NULL; native worker cancel/context retirement/reentry and two independent creator indexes balanced");return 0;
}
