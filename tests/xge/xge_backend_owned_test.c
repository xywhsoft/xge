/* Public-API CPU test; this does not initialize an engine or graphics driver. */
#include "xge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct BackendOwner {
    size_t refs;
    unsigned holds,drops,calls;
    bool rejectHold,holdError,dropError;
    xge_platform_backend_ref backend;
} BackendOwner;
static bool count(const void* raw,size_t* out)
{*out=((const BackendOwner*)raw)->refs;return *out!=0;}
static bool trace(const void* raw,xrtownershipvisitor visitor,void* user)
{(void)raw;(void)visitor;(void)user;return true;}
static const xrtownershipops ownerOps={count,trace};
static bool hold(const void* raw)
{
    BackendOwner* owner=(BackendOwner*)raw;++owner->holds;
    if(owner->rejectHold)return false;
    ++owner->refs;if(owner->holdError)xrtSetErrorKind(XERR_ARGUMENT);
    return true;
}
static bool drop(const void* raw)
{
    BackendOwner* owner=(BackendOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    if(owner->backend){
        const xge_platform_backend_t* descriptor=(const void*)1;
        assert(xgePlatformBackendDescribe(owner->backend,&descriptor)==XGE_ERROR_INVALID_STATE && !descriptor);
    }
    if(owner->dropError)xrtSetErrorKind(XERR_STATE);
    return !owner->dropError;
}
static xge_callback_owner_t owned(BackendOwner* owner)
{return (xge_callback_owner_t){{owner,&ownerOps},hold,drop};}
static int call(void* raw){BackendOwner* owner=raw;++owner->calls;return 42;}
static bool edge(xrtownershipref ref,void* raw)
{BackendOwner* owner=raw;assert(ref.Data==owner && ref.Ops==&ownerOps);++owner->calls;return true;}
static void platform(void)
{
    BackendOwner owner={.refs=1};xge_callback_owner_t retained=owned(&owner);
    char name[]="backend-UTF8-中文";
    xge_platform_backend_t input={.iType=XGE_PLATFORM_BACKEND_CUSTOM,.sName=name,.init=call,.pUser=&owner};
    xge_platform_backend_ref backend=NULL,alias=NULL;
    assert(xgePlatformBackendCreateOwned(&backend,&input,&retained)==XGE_OK && owner.refs==2 && owner.holds==1);
    name[0]='!';input.init=NULL;input.pUser=NULL;input.iType=0;
    const xge_platform_backend_t* descriptor=NULL;
    assert(xgePlatformBackendDescribe(backend,&descriptor)==XGE_OK && descriptor);
    assert(!strcmp(descriptor->sName,"backend-UTF8-中文") && descriptor->iType==XGE_PLATFORM_BACKEND_CUSTOM);
    assert(!owner.calls && descriptor->init(descriptor->pUser)==42 && owner.calls==1);
    xrtownershipref ref=xgePlatformBackendOwnership(backend);size_t refs=0;
    assert(ref.Ops && ref.Ops->Count(ref.Data,&refs) && refs==1);
    assert(ref.Ops->Trace(ref.Data,edge,&owner) && owner.calls==2);
    assert(xgePlatformBackendOwnershipAdapterV1(ref) && xgePlatformBackendOwnershipPreparationV1(ref));
    assert(xgePlatformBackendOwnershipQuiescenceV1(ref) && xgePlatformBackendOwnershipCompletionV2(ref));
    assert(xgePlatformBackendOwnershipFinalizationV1(ref) && xgePlatformBackendOwnershipReadReady(ref));
    assert(xgePlatformBackendShareOwned(backend,&alias)==XGE_OK && alias==backend && owner.refs==2);
    bool consumed=false;assert(xgePlatformBackendReleaseOwnerChecked(backend,&consumed)==XGE_OK && consumed && !owner.drops);
    assert(xgePlatformBackendDescribe(alias,&descriptor)==XGE_OK && descriptor->init(descriptor->pUser)==42);
    owner.backend=alias;consumed=false;
    assert(xgePlatformBackendReleaseOwnerChecked(alias,&consumed)==XGE_OK && consumed && owner.refs==1 && owner.drops==1);
}
static void graphics(void)
{
    BackendOwner owner={.refs=1};xge_callback_owner_t retained=owned(&owner);
    xge_graphics_backend_t input={.iType=XGE_GPU_BACKEND_OPENGL33,.sName="",.begin=call,.pUser=&owner};
    xge_graphics_backend_ref backend=NULL,alias=NULL;
    assert(xgeGraphicsBackendCreateOwned(&backend,&input,&retained)==XGE_OK);
    const xge_graphics_backend_t* descriptor=NULL;
    assert(xgeGraphicsBackendDescribe(backend,&descriptor)==XGE_OK && descriptor->sName && !*descriptor->sName);
    assert(descriptor->begin(descriptor->pUser)==42);
    assert(xgeGraphicsBackendShareOwned(backend,&alias)==XGE_OK && owner.refs==2);
    assert(xgeGraphicsBackendReleaseChecked(backend)==XGE_OK);
    assert(xgeGraphicsBackendReleaseChecked(alias)==XGE_OK && owner.refs==1 && owner.drops==1);
    input.sName=NULL;
    assert(xgeGraphicsBackendCreateOwned(&backend,&input,NULL)==XGE_OK);
    assert(xgeGraphicsBackendDescribe(backend,&descriptor)==XGE_OK && !descriptor->sName);
    assert(xgeGraphicsBackendReleaseChecked(backend)==XGE_OK);
}
static void failures(void)
{
    for(int kind=0;kind<2;++kind){
        BackendOwner owner={.refs=1,.rejectHold=kind==0,.holdError=kind==1};
        xge_callback_owner_t retained=owned(&owner);xge_platform_backend_t input={.sName="rollback"};
        xge_platform_backend_ref backend=(void*)1;
        assert(xgePlatformBackendCreateOwned(&backend,&input,&retained)==XGE_ERROR_CALLBACK_FAILED && !backend);
        assert(owner.refs==1 && owner.holds==1 && owner.drops==(kind==1 ? 1u:0u));xrtClearError();
    }
    BackendOwner owner={.refs=1,.dropError=true};xge_callback_owner_t retained=owned(&owner);
    xge_platform_backend_t input={.sName="drop"};xge_platform_backend_ref backend=NULL;
    assert(xgePlatformBackendCreateOwned(&backend,&input,&retained)==XGE_OK);
    bool consumed=false;
    assert(xgePlatformBackendReleaseOwnerChecked(backend,&consumed)==XGE_ERROR_CALLBACK_FAILED && consumed);
    assert(owner.refs==1 && owner.drops==1 && xrtGetError());xrtClearError();
    xge_callback_owner_t invalid={{&owner,NULL},hold,drop};backend=(void*)1;
    assert(xgePlatformBackendCreateOwned(&backend,&input,&invalid)==XGE_ERROR_INVALID_ARGUMENT && !backend);
    assert(xgePlatformBackendCreateOwned(NULL,&input,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    backend=(void*)1;assert(xgePlatformBackendCreateOwned(&backend,NULL,NULL)==XGE_ERROR_INVALID_ARGUMENT && !backend);
}
static uint32_t wrongThread(void* raw)
{
    xge_platform_backend_ref backend=raw,alias=(void*)1;const xge_platform_backend_t* descriptor=(void*)1;
    assert(xgePlatformBackendShareOwned(backend,&alias)==XGE_ERROR_BUSY && !alias);
    assert(xgePlatformBackendDescribe(backend,&descriptor)==XGE_ERROR_BUSY && !descriptor);
    bool consumed=true;assert(xgePlatformBackendReleaseOwnerChecked(backend,&consumed)==XGE_ERROR_BUSY && !consumed);
    assert(!xgePlatformBackendOwnershipReadReady(xgePlatformBackendOwnership(backend)));return 0;
}
static void creator(void)
{
    xge_platform_backend_t input={.sName="thread"};xge_platform_backend_ref backend=NULL;
    assert(xgePlatformBackendCreateOwned(&backend,&input,NULL)==XGE_OK);
    xthread* thread=xrtThreadCreate((ptr)wrongThread,backend,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK && xrtThreadExitCode(thread)==0);xrtThreadDestroy(thread);
    assert(xgePlatformBackendReleaseChecked(backend)==XGE_OK);
}
static void protocol(void)
{
    BackendOwner owner={.refs=1};xge_callback_owner_t retained=owned(&owner);
    xge_platform_backend_t input={.sName="protocol"};xge_platform_backend_ref backend=NULL;
    assert(xgePlatformBackendCreateOwned(&backend,&input,&retained)==XGE_OK);
    xrtownershipref ref=xgePlatformBackendOwnership(backend);
    const xrtownershipadapterv1* adapter=xgePlatformBackendOwnershipAdapterV1(ref);
    const xrtownershippreparationv1* prepare=xgePlatformBackendOwnershipPreparationV1(ref);
    const xrtownershipfinalizationv1* finalize=xgePlatformBackendOwnershipFinalizationV1(ref);
    const xrtownershipfinishv2* finish=xgePlatformBackendOwnershipCompletionV2(ref);
    int token;
    assert(adapter->Hold(ref.Data));adapter->Drop(ref.Data);
    assert(adapter->Claim(ref.Data,&token) && !xgePlatformBackendOwnershipReadReady(ref));
    const xge_platform_backend_t* descriptor=(void*)1;
    assert(xgePlatformBackendDescribe(backend,&descriptor)==XGE_ERROR_INVALID_STATE && !descriptor);
    adapter->Restore(ref.Data,&token);assert(xgePlatformBackendOwnershipReadReady(ref));
    assert(adapter->Claim(ref.Data,&token));
    assert(prepare->Prepare(ref.Data,&token)==XRT_OWNERSHIP_PREPARE_READY);
    bool complete=false;assert(finalize->CanStep(ref.Data,&token));
    assert(finalize->Step(ref.Data,&token,&complete) && complete && owner.refs==1 && owner.drops==1);
    adapter->Clear(ref.Data,&token);
    assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY);
    assert(finish->DropReady(ref.Data) && finish->DropChecked(ref.Data));
}
static void oom(void)
{
    unsigned failures=0,successes=0;
    for(int n=0;n<16;++n){
        BackendOwner owner={.refs=1};xge_callback_owner_t retained=owned(&owner);
        xge_platform_backend_t input={.sName="OOM"};xge_platform_backend_ref backend=NULL;
        assert(xrtMemDebugFailAfter(n));int status=xgePlatformBackendCreateOwned(&backend,&input,&retained);xrtMemDebugFailClear();
        if(status==XGE_OK){++successes;assert(xgePlatformBackendReleaseChecked(backend)==XGE_OK);}
        else {++failures;assert(status==XGE_ERROR_OUT_OF_MEMORY && !backend);xrtClearError();}
        assert(owner.refs==1);
    }
    assert(failures && successes);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));platform();graphics();failures();creator();protocol();oom();
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    platform();graphics();failures();creator();protocol();oom();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("BACKEND_OWNED_CPU_OK");return 0;
}
