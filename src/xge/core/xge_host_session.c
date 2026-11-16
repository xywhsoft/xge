/* Explicit host frame driver. Draw calls keep their existing direct GL paths;
 * only cold setup/retirement and frame boundaries consult backend callbacks. */
enum {XGE_HOST_IDLE,XGE_HOST_FRAME,XGE_HOST_PRESENT};
struct xge_host_session_t {
    atomic_uint refs;
    xge_platform_backend_ref platform;
    xge_graphics_backend_ref graphics;
    const xge_platform_backend_t* platformTable;
    const xge_graphics_backend_t* graphicsTable;
    xge_callback_owner_t code;
    xge_host_context_hooks_t hooks;
    xge_host_session_desc_t dimensions,pending;
    xge_graphics_snapshot_t previous;
    double frameStart,lastFrame;
    unsigned phase;
    int previousRunning,backend,maxWidth,maxHeight;
    bool reserved,attached,closing,rendererStopped,platformInitialized,
        graphicsInitialized,contextDuty,tableAttempted,systemLoader,resizePending,polling;
};
static xge_data_lifetime_t* __xgeHostRoot(xge_host_session host)
{return host ? (xge_data_lifetime_t*)(host+1):NULL;}
static bool __xgeHostObjectTrace(const void*,xrtownershipvisitor,void*);
static int __xgeHostObjectFinalize(void*,bool*);
static int __xgeHostObjectFinish(void*);
static int __xgeHostExtraReady(const xge_data_lifetime_t*);
static const xge_data_lifetime_type_t __xgeHostType={
    sizeof(struct xge_host_session_t),offsetof(struct xge_host_session_t,refs),
    NULL,__xgeHostObjectTrace,__xgeHostObjectFinalize,__xgeHostObjectFinish};
#define DO_FN(name) __xgeHost##name
#define DO_PUBLIC(name) xgeHostSession##name
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(host) __xgeHostRoot(host)
#define DO_HANDLE xge_host_session
#define DO_OBJECT_TYPE __xgeHostType
#define DO_GENERATION_TYPE __xgeHostType
#define DO_EXTRA_READY(root) __xgeHostExtraReady(root)
#define DO_READ_READY(root) (__xgeHostAdmit(root)==XGE_OK && !((xge_host_session)__xgeHostBody(root))->closing)
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE
#undef DO_EXTRA_READY
#undef DO_READ_READY

static int __xgeHostExtraReady(const xge_data_lifetime_t* root)
{
    const struct xge_host_session_t* host=__xgeHostBody(root);
    return host->phase==XGE_HOST_FRAME || __xgeContextFrameBusy() || g_xge.bRenderActive ? XGE_ERROR_BUSY:XGE_OK;
}
static bool __xgeHostObjectTrace(const void* raw,xrtownershipvisitor visit,void* user)
{
    const struct xge_host_session_t* host=raw;
    return (!host->platform || visit(xgePlatformBackendOwnership(host->platform),user)) &&
        (!host->graphics || visit(xgeGraphicsBackendOwnership(host->graphics),user)) &&
        (!host->code.onDrop || visit(host->code.tReference,user));
}
/* init/makeCurrent/begin/end are status callbacks, not arbitrary counters. */
static int __xgeHostStatus(int status)
{return status>0 ? XGE_ERROR_BACKEND_FAILED:status==XGE_OK && xrtGetError() ? XGE_ERROR_CALLBACK_FAILED:status;}
static void __xgeHostCurrentFlag(bool current)
{
    g_xge.bGraphicsRunning=current;
#if XGE_ENABLE_3D
    xrtAtomic32Store(&g_xge3d_context_active,current,XMEMORY_RELEASE);
#endif
}
static int __xgeHostCurrent(xge_host_session host,bool bind,bool* current)
{
    *current=false;int state=host->hooks.isCurrent(host->hooks.pUser);
    if(xrtGetError() || state<0 || state>1){__xgeHostCurrentFlag(false);return state<0 ? state:XGE_ERROR_CALLBACK_FAILED;}
    if(!state && bind){
        __xgeHostCurrentFlag(false);int status=__xgeHostStatus(host->hooks.makeCurrent(host->hooks.pUser));
        if(status!=XGE_OK)return status;
        state=host->hooks.isCurrent(host->hooks.pUser);
        if(xrtGetError() || state!=1)return state<0 ? state:XGE_ERROR_BACKEND_FAILED;
    }
    *current=state==1;__xgeHostCurrentFlag(*current && host->attached);return XGE_OK;
}
static bool __xgeHostDimensionsValid(int width,int height,float dpi)
{
    return width>0 && height>0 && isfinite(dpi) && dpi>0 &&
        (double)width/(double)dpi<=(double)INT_MAX && (double)height/(double)dpi<=(double)INT_MAX;
}
static int __xgeHostValidate(xge_host_session host,const xge_host_session_desc_t* size)
{
    if(!host->maxWidth){
        const char* version=(const char*)xge_glGetString(GL_VERSION);
        if(!version)return XGE_ERROR_BACKEND_FAILED;
        bool es=strstr(version,"OpenGL ES")!=NULL;
        if(es!=(host->backend==XGE_GPU_BACKEND_GLES30))return XGE_ERROR_UNSUPPORTED;
        GLint major=0,minor=0,limits[2]={0};
        xge_glGetIntegerv(GL_MAJOR_VERSION,&major);xge_glGetIntegerv(GL_MINOR_VERSION,&minor);
        xge_glGetIntegerv(GL_MAX_VIEWPORT_DIMS,limits);
        if(major<3 || (!es && major==3 && minor<3))return XGE_ERROR_UNSUPPORTED;
        host->maxWidth=limits[0];host->maxHeight=limits[1];
    }
    if(size->width>host->maxWidth || size->height>host->maxHeight)return XGE_ERROR_UNSUPPORTED;
    GLint previous=0;xge_glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&previous);
    xge_glBindFramebuffer(GL_DRAW_FRAMEBUFFER,size->framebuffer);
    int status=xge_glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE ? XGE_OK:XGE_ERROR_BACKEND_FAILED;
    xge_glBindFramebuffer(GL_DRAW_FRAMEBUFFER,(GLuint)previous);return status;
}
static void __xgeHostDimensionsApply(const xge_host_session_desc_t* size)
{
    g_xge.iWidth=g_xge.iFramebufferWidth=size->width;g_xge.iHeight=g_xge.iFramebufferHeight=size->height;
    g_xge.iWindowWidth=(int)((double)size->width/(double)size->dpiScale);
    g_xge.iWindowHeight=(int)((double)size->height/(double)size->dpiScale);
    g_xge.fDpiScale=size->dpiScale;g_xge.iCurrentFramebufferId=size->framebuffer;
    g_xge.tCamera.tViewport.fW=(float)size->width;g_xge.tCamera.tViewport.fH=(float)size->height;
    __xgePlatformRuntimeUpdate();
}
static int __xgeHostEnter(xge_host_session host,xerror** ambient)
{
    xge_data_lifetime_t* root=__xgeHostRoot(host);int status=__xgeHostAdmit(root);
    if(status!=XGE_OK)return status;
    if(!host->attached || host->closing)return XGE_ERROR_INVALID_STATE;
    if(atomic_load_explicit(&root->iActivity,memory_order_acquire))return XGE_ERROR_BUSY;
    status=__xgeHostAcquire(root,true,true);if(status==XGE_OK)*ambient=xrtTakeError();return status;
}
static int __xgeHostLeave(xge_host_session host,int status,xerror* ambient)
{
    xge_data_outcome_t outcome={0};__xgeHostOutcomeBegin(&outcome,status);
    __xgeHostOutcomeAdd(&outcome,__xgeHostActivityEnd(__xgeHostRoot(host)));
    return __xgeHostOutcomeEnd(&outcome,ambient);
}
static int __xgeHostObjectFinalize(void* raw,bool* complete)
{
    xge_host_session host=raw;*complete=false;
    if(__xgeHostExtraReady(__xgeHostRoot(host))!=XGE_OK)return XGE_STATUS_DEFERRED;
    host->closing=true;__xgeGraphicsBackendEnding(&g_xgeGraphicsLifetime);
    xge_data_outcome_t outcome={0};__xgeHostOutcomeBegin(&outcome,XGE_OK);
    if(host->attached && !host->rendererStopped){
        bool current=false;int status=__xgeHostCurrent(host,true,&current);
        __xgeHostOutcomeAdd(&outcome,status);
        if(status!=XGE_OK)return __xgeHostOutcomeEnd(&outcome,NULL);
#if XGE_ENABLE_2D
        __xgeRenderCommandReset();__xgeShapeAutoBatchReset();
#endif
        __xgeTextureUploadQueueFree();
        if(__xgeGraphicsOwnerCount(&g_xgeGraphicsLifetime)>1){
            __xgeHostOutcomeAdd(&outcome,XGE_STATUS_DEFERRED);return __xgeHostOutcomeEnd(&outcome,NULL);
        }
        __xgeContextRendererStop();host->rendererStopped=true;host->phase=XGE_HOST_IDLE;
    }
    if(host->graphicsInitialized){
        host->graphicsInitialized=false;
        if(host->graphicsTable->unit)host->graphicsTable->unit(host->graphicsTable->pUser);
        __xgeHostOutcomeAdd(&outcome,XGE_OK);
    }
    if(host->tableAttempted){
        int status=host->systemLoader ? __xgeGraphicsLibraryRetire():(xge_gl_unload(),XGE_OK);
        __xgeHostOutcomeAdd(&outcome,status);
        if(status!=XGE_OK)return __xgeHostOutcomeEnd(&outcome,NULL);
        host->tableAttempted=false;
    }
    if(host->contextDuty){
        xge_host_release_result_t released=host->hooks.release(host->hooks.pUser);
        if(released.consumed)host->contextDuty=false;
        int status=released.status>XGE_STATUS_DEFERRED || (released.consumed && released.status==XGE_STATUS_DEFERRED) ?
            XGE_ERROR_BACKEND_FAILED:released.status;
        __xgeHostOutcomeAdd(&outcome,status);
        if(!released.consumed){
            __xgeHostOutcomeAdd(&outcome,XGE_STATUS_DEFERRED);return __xgeHostOutcomeEnd(&outcome,NULL);
        }
    }
    if(host->platformInitialized){
        host->platformInitialized=false;
        if(host->platformTable->unit)host->platformTable->unit(host->platformTable->pUser);
        __xgeHostOutcomeAdd(&outcome,XGE_OK);
    }
    /* Child release may defer. Keep it in Finalize, before complete=true;
     * the generic Finish stage consumes its one code-drop obligation once. */
    if(host->graphics){
        xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){
            __xgeHostOutcomeAdd(&outcome,XGE_ERROR_INVALID_STATE);return __xgeHostOutcomeEnd(&outcome,NULL);
        }
        xge_graphics_backend_ref child=host->graphics;host->graphics=NULL;__xgeHostMutationEnd(&mutation);
        bool consumed=false;int status=xgeGraphicsBackendReleaseOwnerChecked(child,&consumed);
        if(!consumed){
            if(!xrtOwnershipMutationBegin(&mutation))abort();
            host->graphics=child;__xgeHostMutationEnd(&mutation);
        }
        __xgeHostOutcomeAdd(&outcome,status);
        if(!consumed)return __xgeHostOutcomeEnd(&outcome,NULL);
    }
    if(host->platform){
        xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){
            __xgeHostOutcomeAdd(&outcome,XGE_ERROR_INVALID_STATE);return __xgeHostOutcomeEnd(&outcome,NULL);
        }
        xge_platform_backend_ref child=host->platform;host->platform=NULL;__xgeHostMutationEnd(&mutation);
        bool consumed=false;int status=xgePlatformBackendReleaseOwnerChecked(child,&consumed);
        if(!consumed){
            if(!xrtOwnershipMutationBegin(&mutation))abort();
            host->platform=child;__xgeHostMutationEnd(&mutation);
        }
        __xgeHostOutcomeAdd(&outcome,status);
        if(!consumed)return __xgeHostOutcomeEnd(&outcome,NULL);
    }
    __xgeContextRestore(&host->previous);g_xge.bRunning=host->previousRunning;
    *complete=true;return __xgeHostOutcomeEnd(&outcome,NULL);
}
static int __xgeHostObjectFinish(void* raw)
{
    xge_host_session host=raw;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t code=host->code;host->code=(xge_callback_owner_t){0};__xgeHostMutationEnd(&mutation);
    xge_data_outcome_t outcome={0};__xgeHostOutcomeBegin(&outcome,__xgeCallbackOwnerDrop(code));
    if(host->reserved){__xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,host->attached);host->reserved=false;}
    xrtSetErrorTake(outcome.pError);return outcome.iStatus;
}
int xgeHostSessionShareOwned(xge_host_session host,xge_host_session* out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int status=__xgeHostAdmit(__xgeHostRoot(host));if(status!=XGE_OK)return status;
    if(host->closing || !host->attached)return XGE_ERROR_INVALID_STATE;
    status=__xgeHostAcquire(__xgeHostRoot(host),true,false);if(status==XGE_OK)*out=host;return status;
}
int xgeHostSessionReleaseOwnerChecked(xge_host_session host,bool* consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;xge_data_lifetime_t* root=__xgeHostRoot(host);
    if(!__xgeHostKnown(root))return XGE_ERROR_INVALID_ARGUMENT;
    if(root->iCreator!=xrtThreadCurrentId())return XGE_ERROR_BUSY;
    if(atomic_load_explicit(&root->iActivity,memory_order_acquire))return XGE_STATUS_DEFERRED;
    return __xgeHostRelease(root,consumed);
}
int xgeHostSessionReleaseChecked(xge_host_session host)
{bool consumed=false;return xgeHostSessionReleaseOwnerChecked(host,&consumed);}
int xgeHostSessionInfoGet(xge_host_session host,xge_host_session_info_t* out)
{
    if(!out || out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeHostAdmit(__xgeHostRoot(host));if(status!=XGE_OK)return status;
    xge_host_session_info_t info={0};info.size=sizeof(info);info.width=host->dimensions.width;info.height=host->dimensions.height;
    info.backend=host->backend;info.graphicsReady=g_xge.bGraphicsRunning!=0;
    info.frameOpen=host->phase==XGE_HOST_FRAME;info.presentPending=host->phase==XGE_HOST_PRESENT;
    info.running=g_xge.bRunning!=0;info.closing=host->closing;
    info.otherResources=__xgeGraphicsOwnerCount(&g_xgeGraphicsLifetime)-1;*out=info;return XGE_OK;
}
int xgeHostSessionIsCurrent(xge_host_session host,bool* current)
{
    if(!current)return XGE_ERROR_INVALID_ARGUMENT;
    *current=false;xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    status=__xgeHostCurrent(host,false,current);return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionMakeCurrent(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    bool current=false;status=__xgeHostCurrent(host,true,&current);return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionResize(xge_host_session host,int width,int height,float dpi,uint32_t framebuffer)
{
    int status=__xgeHostAdmit(__xgeHostRoot(host));if(status!=XGE_OK)return status;
    if(host->closing || !host->attached)return XGE_ERROR_INVALID_STATE;
    if(!__xgeHostDimensionsValid(width,height,dpi))return XGE_ERROR_INVALID_ARGUMENT;
    if(atomic_load_explicit(&__xgeHostRoot(host)->iActivity,memory_order_acquire) && !host->polling)return XGE_ERROR_BUSY;
    host->pending=(xge_host_session_desc_t){sizeof(host->pending),width,height,dpi,framebuffer};host->resizePending=true;
    return XGE_OK;
}
int xgeHostSessionPoll(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    int value=0;
    if(host->phase!=XGE_HOST_IDLE)status=XGE_ERROR_BUSY;
    else if(host->platformTable->poll){host->polling=true;value=host->platformTable->poll(host->platformTable->pUser);host->polling=false;}
    if(value<0)status=value;
    status=__xgeHostLeave(host,status,ambient);return status==XGE_OK ? value:status;
}
int xgeHostSessionBegin(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    if(host->phase!=XGE_HOST_IDLE || g_xge.bRenderActive || !xgePassIsIdle())status=XGE_ERROR_BUSY;
    else if(!g_xge.bRunning)status=XGE_ERROR_INVALID_STATE;
    bool current=false;if(status==XGE_OK)status=__xgeHostCurrent(host,true,&current);
    const xge_host_session_desc_t* size=host->resizePending ? &host->pending:&host->dimensions;
    if(status==XGE_OK && host->resizePending)status=__xgeHostValidate(host,size);
    bool accepted=false;
    if(status==XGE_OK){
        int raw=host->graphicsTable->begin ? host->graphicsTable->begin(host->graphicsTable->pUser):XGE_OK;
        accepted=raw==XGE_OK;status=__xgeHostStatus(raw);
    }
    if(status==XGE_OK){
        if(host->resizePending){host->dimensions=host->pending;host->resizePending=false;}
        __xgeHostDimensionsApply(&host->dimensions);
#if XGE_ENABLE_2D
        __xgeRenderCommandReset();__xgeShapeAutoBatchReset();
#endif
        __xgeFrameStatsBeginFrame();host->frameStart=xrtTimer();
        g_xge.fDelta=host->lastFrame>0 ? (float)(host->frameStart-host->lastFrame):0;
        double fps=g_xge.fDelta>0 ? 1.0/(double)g_xge.fDelta:0;
        g_xge.iFPS=fps>=(double)INT_MAX ? INT_MAX:(int)(fps+0.5);
        host->lastFrame=host->frameStart;++g_xge.iFrameCount;++g_xge.tFrameStats.iFrameCount;
        xge_glBindFramebuffer(GL_FRAMEBUFFER,host->dimensions.framebuffer);
        xge_glViewport(0,0,host->dimensions.width,host->dimensions.height);
        status=xgeBegin();
        if(status==XGE_OK){
            host->phase=XGE_HOST_FRAME;xgeTextureUploadFlush();
            if(xrtGetError())status=XGE_ERROR_CALLBACK_FAILED;
        }
    }
    if(status!=XGE_OK && accepted){
        /* The native begin accepted a frame, even if its language callback or
         * subsequent uploads reported an error. Balance that obligation once
         * and preserve the first diagnostic; do not leak an invisible frame. */
        xge_data_outcome_t failure={0};__xgeHostOutcomeBegin(&failure,status);
        if(host->phase==XGE_HOST_FRAME){__xgeHostOutcomeAdd(&failure,xgeEnd());host->phase=XGE_HOST_IDLE;}
        if(host->graphicsTable->end)__xgeHostOutcomeAdd(&failure,__xgeHostStatus(host->graphicsTable->end(host->graphicsTable->pUser)));
        status=__xgeHostOutcomeEnd(&failure,NULL);
    }
    return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionEnd(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    if(host->phase!=XGE_HOST_FRAME)status=XGE_ERROR_INVALID_STATE;
    else if(!xgePassIsIdle())status=XGE_ERROR_BUSY;
    else{
        bool current=false;status=__xgeHostCurrent(host,true,&current);
        if(status==XGE_OK){
            xge_data_outcome_t outcome={0};__xgeHostOutcomeBegin(&outcome,xgeEnd());host->phase=XGE_HOST_IDLE;
            if(host->graphicsTable->end)__xgeHostOutcomeAdd(&outcome,__xgeHostStatus(host->graphicsTable->end(host->graphicsTable->pUser)));
            status=__xgeHostOutcomeEnd(&outcome,NULL);if(status==XGE_OK)host->phase=XGE_HOST_PRESENT;
        }
    }
    return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionPresent(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    if(host->phase!=XGE_HOST_PRESENT)status=XGE_ERROR_INVALID_STATE;
    else{
        bool current=false;status=__xgeHostCurrent(host,true,&current);
        if(status==XGE_OK){
            host->phase=XGE_HOST_IDLE;
            if(host->platformTable->present)host->platformTable->present(host->platformTable->pUser);
            __xgeInputBeginFrame();__xgeFrameStatsRecordTime(xrtTimer()-host->frameStart);
        }
    }
    return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionDiscardFrame(xge_host_session host)
{
    xerror* ambient=NULL;int status=__xgeHostEnter(host,&ambient);if(status!=XGE_OK)return status;
    if(host->phase!=XGE_HOST_PRESENT)status=XGE_ERROR_INVALID_STATE;
    else{host->phase=XGE_HOST_IDLE;__xgeInputBeginFrame();__xgeFrameStatsRecordTime(xrtTimer()-host->frameStart);}
    return __xgeHostLeave(host,status,ambient);
}
int xgeHostSessionCreateOwned(xge_host_session* out,const xge_host_session_desc_t* desc,
    xge_platform_backend_ref platform,xge_graphics_backend_ref graphics,
    const xge_host_context_hooks_t* hooks,const xge_callback_owner_t* hooksOwner)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!desc || desc->size!=sizeof(*desc) || !__xgeHostDimensionsValid(desc->width,desc->height,desc->dpiScale) ||
        !hooks || !hooks->makeCurrent || !hooks->isCurrent || !hooks->release || !__xgeCallbackOwnerValid(hooksOwner))return XGE_ERROR_INVALID_ARGUMENT;
    const xge_platform_backend_t* pt=NULL;const xge_graphics_backend_t* gt=NULL;
    int status=xgePlatformBackendDescribe(platform,&pt);if(status!=XGE_OK)return status;
    status=xgeGraphicsBackendDescribe(graphics,&gt);if(status!=XGE_OK)return status;
    if(gt->iType!=XGE_GPU_BACKEND_OPENGL33 && gt->iType!=XGE_GPU_BACKEND_GLES30)return XGE_ERROR_UNSUPPORTED;
    status=__xgeGraphicsContextBegin(&g_xgeGraphicsLifetime);if(status!=XGE_OK)return status;
    if(!g_xge.bInitialized)status=XGE_ERROR_NOT_INITIALIZED;
    else if(g_xge.objDesc.iRunMode!=XGE_RUN_MANUAL)status=XGE_ERROR_UNSUPPORTED;
    else if(g_xge.bGraphicsRunning || g_xge.bRenderActive || __xgeContextFrameBusy() || xge_glGetString ||
        g_xgeGLLibrary.handle || g_xgeGLRetiredLibrary.handle)status=XGE_ERROR_BUSY;
    if(status!=XGE_OK){__xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);return status;}
    xerror* ambient=xrtTakeError();xge_host_session host=xrtCalloc(1,sizeof(*host)+sizeof(xge_data_lifetime_t));
    if(!host){
        __xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);xge_data_outcome_t outcome={0};
        __xgeHostOutcomeBegin(&outcome,XGE_ERROR_OUT_OF_MEMORY);return __xgeHostOutcomeEnd(&outcome,ambient);
    }
    atomic_init(&host->refs,1u);host->reserved=true;host->hooks=*hooks;host->dimensions=*desc;
    host->backend=gt->iType;host->previousRunning=g_xge.bRunning;__xgeContextSave(&host->previous);
    __xgeHostInit(__xgeHostRoot(host),&__xgeHostType,xrtThreadCurrentId());
    status=xgePlatformBackendShareOwned(platform,&host->platform);if(status==XGE_OK)host->platformTable=pt;
    if(status==XGE_OK){status=xgeGraphicsBackendShareOwned(graphics,&host->graphics);if(status==XGE_OK)host->graphicsTable=gt;}
    if(status==XGE_OK){
        xge_callback_owner_t code=hooksOwner ? *hooksOwner:(xge_callback_owner_t){0};bool held=false;
        status=__xgeCallbackOwnerHold(code,&held);if(held)host->code=code;
    }
    if(status==XGE_OK){
        host->contextDuty=true;
        status=pt->init ? __xgeHostStatus(pt->init(pt->pUser)):XGE_OK;
        if(status==XGE_OK)host->platformInitialized=true;
    }
    if(status==XGE_OK){
        status=gt->init ? __xgeHostStatus(gt->init(gt->pUser)):XGE_OK;
        if(status==XGE_OK)host->graphicsInitialized=true;
    }
    bool current=false;if(status==XGE_OK)status=__xgeHostCurrent(host,true,&current);
    if(status==XGE_OK){
        host->tableAttempted=true;host->systemLoader=!hooks->getProcAddress;
        status=host->systemLoader ? __xgeGraphicsLoadGL(gt->iType):
            xge_gl_load_ex(hooks->getProcAddress,hooks->pUser) ? XGE_OK:XGE_ERROR_BACKEND_FAILED;
        if(status==XGE_OK && xrtGetError())status=XGE_ERROR_CALLBACK_FAILED;
    }
    if(status==XGE_OK)status=__xgeHostValidate(host,desc);
    if(status==XGE_OK){
        host->attached=true;g_xge.tPlatformBackend=*pt;g_xge.tGraphicsBackend=*gt;g_xge.tGpuCaps.iBackend=gt->iType;
        g_xge.bClipEnabled=g_xge.bViewportEnabled=0;g_xge.tClipRect=(xge_rect_t){0};g_xge.tViewportRect=(xge_rect_t){0};
#if XGE_ENABLE_2D
        g_xge.tCamera=xgeCameraDefault((float)desc->width,(float)desc->height);
#endif
        __xgeHostDimensionsApply(desc);xge_glBindFramebuffer(GL_FRAMEBUFFER,desc->framebuffer);
        __xgeHostCurrentFlag(true);g_xge.bRunning=1;
#if XGE_ENABLE_3D
        xrtAtomic64FetchAdd(&g_xge3d_context_serial,1,XMEMORY_RELAXED);
        xrtAtomic64Store(&g_xge3d_context_thread,xrtThreadCurrentId(),XMEMORY_RELEASE);
#endif
        __xgeGraphicsContextCommit(&g_xgeGraphicsLifetime,true);__xgeBlendApply(g_xge.iBlend);__xgeDepthTestApply();
    }
    xge_data_outcome_t outcome={0};__xgeHostOutcomeBegin(&outcome,status);
    if(outcome.iStatus==XGE_OK)*out=host;
    else{
        bool consumed=false;__xgeHostOutcomeAdd(&outcome,xgeHostSessionReleaseOwnerChecked(host,&consumed));
        if(!consumed)*out=host;
    }
    return __xgeHostOutcomeEnd(&outcome,ambient);
}
