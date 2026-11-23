/* Exercises the Windows drawable and checked retirement in the production
 * translation unit. Build with XRT_MODULE_MEMORY_DEBUG. */
#include "single/xge.c"

#undef NDEBUG
#include <assert.h>

#if defined(_WIN32)
static BOOL WINAPI refuse_delete(HGLRC context)
{
    (void)context;
    return FALSE;
}

static int32 check_other_thread(void* raw)
{
    xge_graphics_context context=raw;
    xge_graphics_context_info_t info={.size=sizeof(info),.width=-1};
    bool consumed=true;
    assert(xgeGraphicsContextInfoGet(context,&info)==XGE_ERROR_BUSY && info.width==-1);
    assert(xgeGraphicsContextMakeCurrent(context)==XGE_ERROR_BUSY);
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_ERROR_BUSY && !consumed);
    return 0;
}
#endif

int main(void)
{
    xge_graphics_context context=NULL;
    assert(xgeGraphicsContextCreateOffscreenOwned(&context,0,4,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(context==NULL);
#if defined(_WIN32)
    assert(xgeGraphicsContextCreateOffscreenOwned(&context,4,4,NULL)==XGE_ERROR_NOT_INITIALIZED);
    xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL,.iWidth=4,.iHeight=4};
    assert(xgeInit(&desc)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xgeGraphicsContextCreateOffscreenOwned(&context,4,4,NULL)==XGE_ERROR_OUT_OF_MEMORY);
    assert(context==NULL && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    assert(xgeGraphicsContextCreateOffscreenOwned(&context,4,4,NULL)==XGE_OK && context);
    xge_graphics_context_info_t info={.size=sizeof(info)};
    assert(xgeGraphicsContextInfoGet(context,&info)==XGE_OK && info.current && info.graphicsReady);
    assert(info.width==4 && info.height==4 && info.backend==XGE_GPU_BACKEND_OPENGL33);
    assert(!info.surfaceless && info.otherResources==0);
    HWND window=context->wgl.window;
    assert(IsWindow(window) && !IsWindowVisible(window) && xgePlatformNativeHandle()==NULL);
    xge_platform_caps_t caps;
    assert(xgePlatformCapsGet(&caps)==XGE_OK && caps.bOffscreen && !caps.bWindow);
    assert(caps.iPlatformBackend==XGE_PLATFORM_BACKEND_WGL);
    xthread* worker=xrtThreadCreate(check_other_thread,context,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);

    // A displaced context can be rebound before its renderer is reused.
    assert(context->wgl.makeCurrent(NULL,NULL));
    assert(xgeGraphicsContextInfoGet(context,&info)==XGE_OK && !info.current && !info.graphicsReady);
    assert(xgeGraphicsContextMakeCurrent(context)==XGE_OK);
    assert(xgeGraphicsContextInfoGet(context,&info)==XGE_OK && info.current && info.graphicsReady);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);

    xge_wgl_delete_proc deleteContext=context->wgl.deleteContext;
    context->wgl.deleteContext=refuse_delete;
    bool consumed=true;
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_ERROR_BACKEND_FAILED && !consumed);
    assert(context->wgl.coreContext && context->wgl.dc && context->wgl.library.handle);
    assert(IsWindow(window) && !context->tableLoaded);
    context->wgl.deleteContext=deleteContext;
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
    assert(!IsWindow(window));
    assert(xgeUnitChecked()==XGE_OK);
#elif !XGE_ENABLE_EGL
    assert(xgeGraphicsContextCreateOffscreenOwned(&context,4,4,NULL)==XGE_ERROR_UNSUPPORTED && !context);
#endif
    puts("XGE_WGL_OFFSCREEN_NATIVE_OK");
    return 0;
}
