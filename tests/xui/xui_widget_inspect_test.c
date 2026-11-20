#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#include "xui.h"
#if !defined(XUI_TEST_STATIC)
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#if !defined(XUI_TEST_STATIC)
static xge_host_xrt_proc resolve(void* context,const char* name)
{
    (void)context;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

static int rejectLayout(xui_widget widget,void* user)
{
    (void)widget;
    ++*(int*)user;
    return XUI_ERROR_BUSY;
}

static void checkInspector(void)
{
    xui_context context=NULL;
    xui_widget root=NULL, lower=NULL, upper=NULL;
    assert(xuiCreate(&context)==XUI_OK);
    assert(xuiWidgetCreate(context,&root)==XUI_OK);
    assert(xuiWidgetCreate(context,&lower)==XUI_OK);
    assert(xuiWidgetCreate(context,&upper)==XUI_OK);
    assert(xuiWidgetSetRect(root,(xui_rect_t){0,0,120,90})==XUI_OK);
    assert(xuiWidgetSetRect(lower,(xui_rect_t){10,15,50,35})==XUI_OK);
    assert(xuiWidgetSetRect(upper,(xui_rect_t){13,16,48,35})==XUI_OK);
    assert(xuiWidgetAddChild(root,lower)==XUI_OK);
    assert(xuiWidgetAddChild(root,upper)==XUI_OK);
    assert(xuiWidgetSetLayer(upper,0,4)==XUI_OK);
    assert(xuiSetRootWidget(context,root)==XUI_OK);

    xui_debug_widget_info_t info={.iSize=sizeof(info),.iDepth=-88};
    assert(xrtMemDebugFailAfter(0));
    int status=xuiDebugWidgetInspectAtChecked(context,25,25,0,&info);
    bool failed=xrtMemDebugFailTriggered();
    xrtMemDebugFailClear();
    xrtClearError();
    assert(failed && status==XUI_ERROR_OUT_OF_MEMORY && info.iDepth==-88);
    assert(xuiDebugWidgetInspectAtChecked(context,25.9f,25,0,&info)==XUI_OK);
    assert(info.pWidget==upper && info.pParent==root && info.iDepth==1 && info.iZIndex==4);
    assert(info.tRect.fX==13 && info.tWorldRect.fW==48);
    assert(xuiDebugWidgetInspectAt(context,25.9f,25,NULL)==upper);

    assert(xuiWidgetSetEnabled(upper,0)==XUI_OK);
    assert(xuiDebugWidgetInspectAtChecked(context,25,25,0,&info)==XUI_OK && info.pWidget==lower);
    assert(xuiDebugWidgetInspectAtChecked(context,25,25,
        XUI_WIDGET_HIT_DEFAULT & ~XUI_WIDGET_HIT_ENABLED,&info)==XUI_OK && info.pWidget==upper);
    assert(xuiWidgetSetEnabled(upper,1)==XUI_OK);

    info.iDepth=-88;
    assert(xuiDebugWidgetInspectAtChecked(context,NAN,25,0,&info)==XUI_ERROR_INVALID_ARGUMENT);
    assert(info.iDepth==-88 && xuiDebugWidgetInspectAt(context,INFINITY,25,NULL)==NULL);
    assert(xuiDebugWidgetInspectAtChecked(context,250,150,0,&info)==XUI_OK);
    assert(info.iSize==sizeof(info) && !info.pWidget && !info.pParent && !info.iDepth && !info.tRect.fW);

    int calls=0;
    assert(xuiWidgetSetLayoutPrepareCallback(root,rejectLayout,&calls)==XUI_OK);
    assert(xuiSetViewportSize(context,120,90)==XUI_OK);
    info.iDepth=-88;
    status=xuiDebugWidgetInspectAtChecked(context,25,25,0,&info);
    assert(status==XUI_OK && calls>0 && info.pWidget==upper && info.iDepth==1);
    assert(xuiWidgetSetLayoutPrepareCallback(root,NULL,NULL)==XUI_OK);
    assert(xuiLayout(context)==XUI_OK);

    assert(xuiDebugWidgetInspectAtChecked(context,25,25,0,&info)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i)
        assert(xuiDebugWidgetInspectAtChecked(context,25,25,0,&info)==XUI_OK &&
            info.pWidget==upper && info.pParent==root);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    assert(xuiDestroyChecked(context)==XUI_OK);
}

static int closeDuringLayout(xui_widget widget,void* user)
{
    (void)widget;
    xui_context* context=user;
    xuiDestroy(*context);
    *context=NULL;
    return XUI_OK;
}

static void checkDeferredClose(void)
{
    xui_context context=NULL;
    xui_widget root=NULL;
    assert(xuiCreate(&context)==XUI_OK);
    assert(xuiWidgetCreate(context,&root)==XUI_OK);
    assert(xuiSetRootWidget(context,root)==XUI_OK);
    assert(xuiSetViewportSize(context,120,90)==XUI_OK);
    assert(xuiWidgetSetLayoutPrepareCallback(root,closeDuringLayout,&context)==XUI_OK);
    xui_debug_widget_info_t info={.iSize=sizeof(info),.iDepth=-88};
    int status=xuiDebugWidgetInspectAtChecked(context,25,25,0,&info);
    assert(status!=XUI_OK && context==NULL && info.iDepth==-88 && info.pWidget==NULL);
    xrtClearError();
}

int main(void)
{
#if !defined(XUI_TEST_STATIC)
    const xge_host_xrt_contract_t abi=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&abi,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    checkInspector();
    checkDeferredClose();
    puts("XUI_WIDGET_INSPECT_NATIVE_OK");
    return 0;
}
