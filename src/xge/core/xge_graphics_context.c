/* Exclusive creator-thread renderer session. The shared DataObject kernel
 * provides graph/retirement mechanics; drawing still calls the normal XGE API. */
#include "xge_data_lifetime_internal.h"

typedef struct xge_graphics_snapshot_t {
    xge_platform_backend_t platform;
    xge_graphics_backend_t graphics;
    xge_gpu_caps_t caps;
    int width,height,windowWidth,windowHeight,framebufferWidth,framebufferHeight;
    float dpi;
    uint32_t framebuffer;
    xge_camera_t camera;
    int viewportEnabled,clipEnabled;
    xge_rect_t viewport,clip;
} xge_graphics_snapshot_t;
struct xge_graphics_context_t {
    atomic_uint refs;
    bool attached,closing,destroyed,nativeCleanupStarted,tableLoaded,reserved,restoreApi;
    unsigned previousApi;
    xge_callback_owner_t code;
    xge_egl_context_t egl;
    xge_graphics_snapshot_t previous;
};
static xge_data_lifetime_t* __xgeOwnedContextRoot(xge_graphics_context context)
{return context ? (xge_data_lifetime_t*)(context+1):NULL;}
static bool __xgeContextReadable(const void* data)
{
    const struct xge_graphics_context_t* context=data;
    return context->attached && !context->closing && !context->destroyed && context->egl.bInitialized;
}
static bool __xgeContextTrace(const void*,xrtownershipvisitor,void*);
static int __xgeContextFinalize(void*,bool*);
static int __xgeContextFinish(void*);
static const xge_data_lifetime_type_t __xgeOwnedContextType={
    sizeof(struct xge_graphics_context_t),offsetof(struct xge_graphics_context_t,refs),
    __xgeContextReadable,__xgeContextTrace,__xgeContextFinalize,__xgeContextFinish};
#define DO_FN(name) __xgeOwnedContext##name
#define DO_PUBLIC(name) xgeGraphicsContext##name
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(context) __xgeOwnedContextRoot(context)
#define DO_HANDLE xge_graphics_context
#define DO_OBJECT_TYPE __xgeOwnedContextType
#define DO_GENERATION_TYPE __xgeOwnedContextType
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE

static bool __xgeContextFrameBusy(void)
{
    (void)xrtSpinLock(&g_xgeFrameSpin);bool busy=g_xgeFrameDispatching || g_xgeFrameRun;
    (void)xrtSpinUnlock(&g_xgeFrameSpin);
    return busy || g_xgeSceneStepping || !xgePassIsIdle() || g_xge.bRenderThreadBusy;
}
static void __xgeContextSave(xge_graphics_snapshot_t* previous)
{
    *previous=(xge_graphics_snapshot_t){g_xge.tPlatformBackend,g_xge.tGraphicsBackend,g_xge.tGpuCaps,
        g_xge.iWidth,g_xge.iHeight,g_xge.iWindowWidth,g_xge.iWindowHeight,
        g_xge.iFramebufferWidth,g_xge.iFramebufferHeight,g_xge.fDpiScale,g_xge.iCurrentFramebufferId,
        g_xge.tCamera,g_xge.bViewportEnabled,g_xge.bClipEnabled,g_xge.tViewportRect,g_xge.tClipRect};
}
static void __xgeContextRestore(const xge_graphics_snapshot_t* previous)
{
    g_xge.tPlatformBackend=previous->platform;g_xge.tGraphicsBackend=previous->graphics;g_xge.tGpuCaps=previous->caps;
    g_xge.iWidth=previous->width;g_xge.iHeight=previous->height;
    g_xge.iWindowWidth=previous->windowWidth;g_xge.iWindowHeight=previous->windowHeight;
    g_xge.iFramebufferWidth=previous->framebufferWidth;g_xge.iFramebufferHeight=previous->framebufferHeight;
    g_xge.fDpiScale=previous->dpi;g_xge.iCurrentFramebufferId=previous->framebuffer;
    g_xge.tCamera=previous->camera;g_xge.bViewportEnabled=previous->viewportEnabled;g_xge.bClipEnabled=previous->clipEnabled;
    g_xge.tViewportRect=previous->viewport;g_xge.tClipRect=previous->clip;
    __xgePlatformRuntimeUpdate();
}
static bool __xgeContextTrace(const void* data,xrtownershipvisitor visitor,void* user)
{
    const struct xge_graphics_context_t* context=data;
    return !context->code.onDrop || visitor(context->code.tReference,user);
}
static int __xgeContextFinalize(void* data,bool* complete)
{
    xge_graphics_context context=data;*complete=false;
    if(__xgeContextFrameBusy())return XGE_STATUS_DEFERRED;
    context->closing=true;__xgeGraphicsBackendEnding(&g_xgeGraphicsLifetime);
    if(context->attached && !context->nativeCleanupStarted){
        /* Retained children must keep working until their final release. Only
         * the cold retirement path rebinds a displaced context; draw calls do
         * not acquire another context reference or look up a parent. */
#if defined(XGE_HAS_EGL)
        if(eglGetCurrentContext()!=(EGLContext)context->egl.pContext || eglQueryAPI()!=EGL_OPENGL_ES_API){
            g_xge.bGraphicsRunning=0;
#if XGE_ENABLE_3D
            xrtAtomic32Store(&g_xge3d_context_active,0,XMEMORY_RELEASE);
#endif
            if(!eglBindAPI(EGL_OPENGL_ES_API))return __xgeEGLFail(&context->egl,"eglBindAPI",XGE_ERROR_BACKEND_FAILED);
            int current=xgeEGLMakeCurrent(&context->egl);if(current!=XGE_OK)return current;
        }
#endif
        g_xge.bGraphicsRunning=1;
#if XGE_ENABLE_3D
        xrtAtomic32Store(&g_xge3d_context_active,1,XMEMORY_RELEASE);
#endif
#if XGE_ENABLE_2D
        __xgeRenderCommandReset();__xgeShapeAutoBatchReset();
#endif
        __xgeTextureUploadQueueFree();
        if(__xgeGraphicsOwnerCount(&g_xgeGraphicsLifetime)>1)return XGE_STATUS_DEFERRED;
        context->nativeCleanupStarted=true;
    }
    if(context->attached){
        g_xge.bGraphicsRunning=0;
#if XGE_ENABLE_3D
        xrtAtomic32Store(&g_xge3d_context_active,0,XMEMORY_RELEASE);
#endif
#if XGE_ENABLE_2D
        __xgeRenderCommandReset();__xgeShapeAutoBatchReset();
#endif
#if XGE_ENABLE_SHAPE_EX
        __xgeShapeExRendererUnit();
#endif
        __xgeBuiltinRenderersReset();
        xrtFree(g_xge.pShapeAutoVertices);g_xge.pShapeAutoVertices=NULL;g_xge.iShapeAutoVertexCapacity=0;
        xrtFree(g_xge.pShapeAutoIndices);g_xge.pShapeAutoIndices=NULL;g_xge.iShapeAutoIndexCapacity=0;
    }
    if(context->tableLoaded){xge_gl_unload();context->tableLoaded=false;}
    int status=xgeEGLUnitChecked(&context->egl);
    if(status!=XGE_OK)return status;
#if defined(XGE_HAS_EGL)
    if(context->restoreApi){
        if(!eglBindAPI((EGLenum)context->previousApi))
            return __xgeEGLFail(&context->egl,"eglRestoreAPI",XGE_ERROR_BACKEND_FAILED);
        context->restoreApi=false;
    }
#endif
    __xgeContextRestore(&context->previous);context->destroyed=true;*complete=true;return XGE_OK;
}
static int __xgeContextFinish(void* data)
{
    xge_graphics_context context=data;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t code=context->code;context->code=(xge_callback_owner_t){0};
    __xgeOwnedContextMutationEnd(&mutation);
    xge_data_outcome_t outcome={0};__xgeOwnedContextOutcomeBegin(&outcome,__xgeCallbackOwnerDrop(code));
    if(context->reserved){__xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,context->attached);context->reserved=false;}
    /* The native code Drop is resident and protects the complete return tail. */
    xrtSetErrorTake(outcome.pError);return outcome.iStatus;
}
int xgeGraphicsContextShareOwned(xge_graphics_context context,xge_graphics_context* out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;int status=__xgeOwnedContextAcquire(__xgeOwnedContextRoot(context),true,false);
    if(status==XGE_OK)*out=context;
    return status;
}
int xgeGraphicsContextReleaseOwnerChecked(xge_graphics_context context,bool* consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;xge_data_lifetime_t* root=__xgeOwnedContextRoot(context);
    if(!__xgeOwnedContextKnown(root))return XGE_ERROR_INVALID_ARGUMENT;
    if(root->iCreator!=xrtThreadCurrentId())return XGE_ERROR_BUSY;
    /* A synchronous native error callback may release its caller's owner. Do
     * not consume that obligation while a method still owns an activity ref. */
    if(atomic_load_explicit(&root->iActivity,memory_order_acquire))return XGE_STATUS_DEFERRED;
    return __xgeOwnedContextRelease(root,consumed);
}
int xgeGraphicsContextReleaseChecked(xge_graphics_context context)
{bool consumed=false;return xgeGraphicsContextReleaseOwnerChecked(context,&consumed);}
int xgeGraphicsContextInfoGet(xge_graphics_context context,xge_graphics_context_info_t* out)
{
    if(!out || out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeOwnedContextAdmit(__xgeOwnedContextRoot(context));if(status!=XGE_OK)return status;
    xge_graphics_context_info_t info={0};info.size=sizeof(info);
    info.width=context->egl.iWidth;info.height=context->egl.iHeight;info.backend=XGE_GPU_BACKEND_GLES30;
    info.surfaceless=context->egl.bSurfaceless!=0;info.graphicsReady=g_xge.bGraphicsRunning!=0;
    info.otherResources=__xgeGraphicsOwnerCount(&g_xgeGraphicsLifetime)-1;
#if defined(XGE_HAS_EGL)
    info.current=eglGetCurrentContext()==(EGLContext)context->egl.pContext;
#endif
    *out=info;return XGE_OK;
}
int xgeGraphicsContextMakeCurrent(xge_graphics_context context)
{
    xge_data_lifetime_t* root=__xgeOwnedContextRoot(context);int status=__xgeOwnedContextAdmit(root);
    if(status!=XGE_OK)return status;
    status=__xgeOwnedContextAcquire(root,true,true);if(status!=XGE_OK)return status;
    xerror* ambient=xrtTakeError();
    g_xge.bGraphicsRunning=0;
#if XGE_ENABLE_3D
    xrtAtomic32Store(&g_xge3d_context_active,0,XMEMORY_RELEASE);
#endif
#if defined(XGE_HAS_EGL)
    if(!eglBindAPI(EGL_OPENGL_ES_API))status=__xgeEGLFail(&context->egl,"eglBindAPI",XGE_ERROR_BACKEND_FAILED);
    else status=xgeEGLMakeCurrent(&context->egl);
#else
    status=XGE_ERROR_UNSUPPORTED;
#endif
    bool current=status==XGE_OK;
#if defined(XGE_HAS_EGL)
    current=eglGetCurrentContext()==(EGLContext)context->egl.pContext && eglQueryAPI()==EGL_OPENGL_ES_API;
#endif
    g_xge.bGraphicsRunning=current;
#if XGE_ENABLE_3D
    xrtAtomic32Store(&g_xge3d_context_active,current,XMEMORY_RELEASE);
#endif
    xge_data_outcome_t outcome={0};__xgeOwnedContextOutcomeBegin(&outcome,status);
    __xgeOwnedContextOutcomeAdd(&outcome,__xgeOwnedContextActivityEnd(root));
    return __xgeOwnedContextOutcomeEnd(&outcome,ambient);
}
#if defined(XGE_HAS_EGL)
static void* __xgeContextEglLoad(const char* name,void* unused)
{(void)unused;return (void*)eglGetProcAddress(name);}
static int __xgeContextEglValidate(xge_graphics_context context,int width,int height)
{
    PFNGLGETINTEGERVPROC get=(PFNGLGETINTEGERVPROC)eglGetProcAddress("glGetIntegerv");
    if(!get)return XGE_ERROR_UNSUPPORTED;
    GLint major=0,limits[2]={0};get(GL_MAJOR_VERSION,&major);get(GL_MAX_VIEWPORT_DIMS,limits);
    if(major<3 || width>limits[0] || height>limits[1])return XGE_ERROR_UNSUPPORTED;
    if(!context->egl.bSurfaceless){
        EGLint actualWidth=0,actualHeight=0;
        if(!eglQuerySurface((EGLDisplay)context->egl.pDisplay,(EGLSurface)context->egl.pSurface,EGL_WIDTH,&actualWidth) ||
           !eglQuerySurface((EGLDisplay)context->egl.pDisplay,(EGLSurface)context->egl.pSurface,EGL_HEIGHT,&actualHeight))
            return __xgeEGLFail(&context->egl,"eglQuerySurface",XGE_ERROR_BACKEND_FAILED);
        if(actualWidth!=width || actualHeight!=height)return XGE_ERROR_BACKEND_FAILED;
    }
    return xge_gl_load_ex(__xgeContextEglLoad,NULL) ? XGE_OK:XGE_ERROR_BACKEND_FAILED;
}
#endif
int xgeGraphicsContextCreateEglOwned(xge_graphics_context* out,int width,int height,bool surfaceless,
    const xge_callback_owner_t* codeOwner)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(width<=0 || height<=0 || !__xgeCallbackOwnerValid(codeOwner))return XGE_ERROR_INVALID_ARGUMENT;
#if !defined(XGE_HAS_EGL)
    (void)surfaceless;return XGE_ERROR_UNSUPPORTED;
#else
    int status=__xgeGraphicsContextBegin(&g_xgeGraphicsLifetime);if(status!=XGE_OK)return status;
    if(!g_xge.bInitialized)status=XGE_ERROR_NOT_INITIALIZED;
    else if(g_xge.objDesc.iRunMode!=XGE_RUN_MANUAL)status=XGE_ERROR_UNSUPPORTED;
    else if(g_xge.bGraphicsRunning || __xgeContextFrameBusy() || eglGetCurrentContext()!=EGL_NO_CONTEXT)status=XGE_ERROR_BUSY;
    if(status!=XGE_OK){__xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);return status;}
    xerror* ambient=xrtTakeError();
    xge_graphics_context context=xrtCalloc(1,sizeof(*context)+sizeof(xge_data_lifetime_t));
    if(!context){
        __xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);
        xge_data_outcome_t outcome={0};__xgeOwnedContextOutcomeBegin(&outcome,XGE_ERROR_OUT_OF_MEMORY);
        return __xgeOwnedContextOutcomeEnd(&outcome,ambient);
    }
    atomic_init(&context->refs,1u);context->reserved=true;__xgeContextSave(&context->previous);
    __xgeOwnedContextInit(__xgeOwnedContextRoot(context),&__xgeOwnedContextType,xrtThreadCurrentId());
    xge_callback_owner_t code=codeOwner ? *codeOwner:(xge_callback_owner_t){0};bool held=false;
    status=xrtGetError() ? XGE_ERROR_CALLBACK_FAILED:__xgeCallbackOwnerHold(code,&held);
    if(held)context->code=code;
    if(status==XGE_OK){
        context->previousApi=(unsigned)eglQueryAPI();
        xge_egl_desc_t desc={0};desc.iWidth=width;desc.iHeight=height;desc.bSurfaceless=surfaceless;desc.bPBuffer=!surfaceless;
        status=xgeEGLInit(&context->egl,&desc);
        context->restoreApi=(unsigned)eglQueryAPI()!=context->previousApi;
        if(status==XGE_OK)status=__xgeContextEglValidate(context,width,height);
        if(status==XGE_OK){
            context->tableLoaded=true;context->attached=true;
            g_xge.iWidth=g_xge.iWindowWidth=g_xge.iFramebufferWidth=width;
            g_xge.iHeight=g_xge.iWindowHeight=g_xge.iFramebufferHeight=height;
            g_xge.fDpiScale=1;g_xge.iCurrentFramebufferId=0;g_xge.bClipEnabled=g_xge.bViewportEnabled=0;
            memset(&g_xge.tClipRect,0,sizeof(g_xge.tClipRect));memset(&g_xge.tViewportRect,0,sizeof(g_xge.tViewportRect));
#if XGE_ENABLE_2D
            g_xge.tCamera=xgeCameraDefault((float)width,(float)height);
#endif
            g_xge.bGraphicsRunning=1;
#if XGE_ENABLE_3D
            xrtAtomic64FetchAdd(&g_xge3d_context_serial,1,XMEMORY_RELAXED);
            xrtAtomic64Store(&g_xge3d_context_thread,xrtThreadCurrentId(),XMEMORY_RELEASE);
            xrtAtomic32Store(&g_xge3d_context_active,1,XMEMORY_RELEASE);
#endif
            __xgeGraphicsContextCommit(&g_xgeGraphicsLifetime);__xgePlatformRuntimeUpdate();
            __xgeBlendApply(g_xge.iBlend);__xgeDepthTestApply();
        }
    }
    xge_data_outcome_t outcome={0};__xgeOwnedContextOutcomeBegin(&outcome,status);
    if(outcome.iStatus==XGE_OK)*out=context;
    else{
        /* A failed factory must preserve unrelated preexisting engine queues.
         * Its reserved language root can adopt a remaining cleanup duty. */
        if(!context->attached)__xgeContextRestore(&context->previous);
        bool consumed=false;__xgeOwnedContextOutcomeAdd(&outcome,xgeGraphicsContextReleaseOwnerChecked(context,&consumed));
        if(!consumed)*out=context;
    }
    return __xgeOwnedContextOutcomeEnd(&outcome,ambient);
#endif
}
