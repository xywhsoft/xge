#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef XGE_XRT_HOST
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

typedef struct Owner {
    size_t refs;
    unsigned calls;
    unsigned paints;
    bool active;
    bool refuse;
} Owner;

static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;
    if(owner->refuse)return false;
    ++owner->refs;
    return true;
}

static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;
    assert(owner->refs && (--owner->refs || !owner->active));
    return true;
}

static bool count(const void* raw,size_t* out)
{
    *out=((const Owner*)raw)->refs;
    return *out!=0;
}

static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    (void)raw;(void)visit;(void)user;
    return true;
}

static const xrtownershipops owner_ops={count,trace};

static xui_callback_owner_t binding(Owner* owner)
{
    return (xui_callback_owner_t){{owner,&owner_ops},hold,drop};
}

typedef struct Edges {
    const void* callback;
    xrtownershipref surface;
    unsigned callbacks, surfaces;
} Edges;

static bool find_edge(xrtownershipref ref,void* user)
{
    Edges* edges=user;
    if(ref.Data==edges->callback)++edges->callbacks;
    if(ref.Data==edges->surface.Data && ref.Ops==edges->surface.Ops)++edges->surfaces;
    return true;
}

static void assert_graph(xui_context ui,const Owner* owner,xui_surface surface,unsigned expected)
{
    Edges edges={.callback=owner,.surface=xuiSurfaceOwnership(surface)};
    assert(xuiContextTraceOwnership(ui,find_edge,&edges));
    assert(edges.callbacks==expected && edges.surfaces==expected);
}

static int finalize_surface(xui_proxy proxy,void* raw)
{
    (void)proxy;
    ++**(unsigned**)raw;
    return XUI_OK;
}

static xui_surface make_surface(xui_proxy proxy,unsigned* finalized)
{
    xui_surface surface=NULL;
    void* raw=NULL;
    assert(xuiSurfaceCreateOwned(proxy,sizeof(finalized),finalize_surface,&surface,&raw)==XUI_OK);
    *(unsigned**)raw=finalized;
    return surface;
}

static void remove_self(xui_widget widget,xui_input_decoration decoration,void* user)
{
    Owner* owner=user;
    xui_rect_t bounds;
    owner->active=true;
    ++owner->calls;
    assert(xuiInputDecorationGetBounds(decoration,&bounds)==XUI_OK);
    assert(xuiInputDecorationRemove(widget,decoration)==XUI_OK && owner->refs==1);
    assert(xuiInputDecorationGetBounds(decoration,&bounds)==XUI_ERROR_INVALID_STATE);
    owner->active=false;
}

static int paint(xui_widget widget,xui_input_decoration decoration,xui_draw_context draw,
    xui_rect_t area,uint32_t state,void* user)
{
    (void)state;
    Owner* owner=user;
    assert(draw && area.fW==40 && owner->refs==2);
    ++owner->paints;
    assert(xuiInputDecorationRemove(widget,decoration)==XUI_ERROR_BUSY);
    return XUI_OK;
}

static void render_input(xui_context ui,xui_widget input,xui_proxy backend)
{
    const xui_surface_desc_t size={.iWidth=320,.iHeight=120};
    xui_surface target=NULL;
    xui_draw_context draw=NULL;
    assert(xuiUpdate(ui,0)==XUI_OK);
    assert(backend->surfaceCreate(backend,&target,&size)==XUI_OK);
    assert(backend->drawBegin(backend,&draw,target)==XUI_OK);
    xui_widget_type type=xuiInputGetType(ui);
    assert(type->onCacheRender(input,draw,0,type->pUser)==XUI_OK);
    assert(backend->drawEnd(backend,draw)==XUI_OK);
    backend->surfaceDestroy(backend,target);
}

static void press(xui_context ui,xui_widget widget,xui_input_decoration decoration)
{
    assert(xuiUpdate(ui,0)==XUI_OK);
    xui_rect_t bounds=xuiInputDecorationGetRect(widget,decoration);
    xui_rect_t world=xuiWidgetGetWorldRect(widget);
    int x=(int)(world.fX+bounds.fX+bounds.fW/2), y=(int)(world.fY+bounds.fY+bounds.fH/2);
    assert(xuiInputPointerDown(ui,x,y,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK);
    assert(xuiDispatchPendingEvents(ui)==XUI_OK);
    assert(xuiInputPointerUp(ui,x,y,XUI_POINTER_BUTTON_LEFT,0)==XUI_OK);
    assert(xuiDispatchPendingEvents(ui)==XUI_OK);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    xui_test_proxy_state_t backend;
    xuiTestProxyInit(&backend);
    Owner backend_owner={.refs=1}, owner={.refs=1}, rejected={.refs=1,.refuse=true};
    const xui_callback_owner_t back=binding(&backend_owner), callbacks=binding(&owner), cannot_hold=binding(&rejected);
    xui_proxy proxy=NULL, foreign=NULL;
    xui_context ui=NULL;
    const xui_context_desc_t options={sizeof(options),XUI_SYSTEM_FONT_DISABLED,0,0};
    assert(xuiProxyCreateOwned(&backend.tProxy,&back,&proxy)==XUI_OK);
    assert(xuiProxyCreateOwned(&backend.tProxy,&back,&foreign)==XUI_OK);
    assert(xuiCreateOwnedEx(proxy,&options,&ui)==XUI_OK);
    xui_widget root=NULL, input=NULL;
    assert(xuiSetViewportSize(ui,320,120)==XUI_OK);
    assert(xuiWidgetCreate(ui,&root)==XUI_OK && xuiSetRootWidget(ui,root)==XUI_OK);
    assert(xuiWidgetSetFlowMode(root,XUI_FLOW_ABSOLUTE)==XUI_OK);
    assert(xuiInputCreate(ui,&input,NULL)==XUI_OK && xuiWidgetAddChild(root,input)==XUI_OK);
    assert(xuiWidgetSetRect(input,(xui_rect_t){10,10,200,36})==XUI_OK);

    unsigned finalized=0;
    xui_surface surface=make_surface(proxy,&finalized), other=make_surface(foreign,&finalized);
    xui_input_decoration_desc_t desc={.iSize=sizeof(desc),.iKind=XUI_INPUT_DECORATION_TEXT,
        .fWidth=40,.sText="Go",.pSurface=other,.onClick=remove_self,.onPaint=paint,.pUser=&owner};
    xui_input_decoration decoration=NULL;
    const int side=XUI_INPUT_DECORATION_SIDE_TRAILING;
    assert(xuiInputDecorationAddOwned(input,side,&decoration,&desc,&callbacks)==XUI_ERROR_INVALID_ARGUMENT);
    assert(decoration==NULL && owner.refs==1 && xuiSurfaceReleaseChecked(other)==XUI_OK && finalized==1);
    desc.pSurface=surface;
    assert(xuiInputDecorationAddOwned(input,side,&decoration,&desc,&cannot_hold)!=XUI_OK);
    assert(decoration==NULL && finalized==1 && xuiInputDecorationGetCount(input,side)==0);

    xrtMemDebugEnable(true);
    for(int allowance=0;allowance<3;++allowance) {
        assert(xrtMemDebugFailAfter(allowance));
        assert(xuiInputDecorationAddOwned(input,side,&decoration,&desc,&callbacks)==XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailTriggered());
        xrtMemDebugFailClear();
        xrtClearError();
        assert(decoration==NULL && owner.refs==1 && xuiInputDecorationGetCount(input,side)==0);
    }
    xrtMemDebugEnable(false);
    assert(xuiInputDecorationAddOwned(input,side,&decoration,&desc,&callbacks)==XUI_OK && owner.refs==2);
    assert_graph(ui,&owner,surface,1);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && finalized==1);
    xui_input_decoration_desc_t actual;
    assert(xuiInputDecorationDescribe(decoration,&actual)==XUI_OK && actual.pSurface==surface &&
        !actual.onClick && !actual.onPaint && !actual.pUser && !strcmp(actual.sText,"Go"));
    desc.sText="Replacement";
    assert(xuiInputDecorationSetOwned(input,decoration,&desc,&cannot_hold)!=XUI_OK);
    assert(xuiInputDecorationDescribe(decoration,&actual)==XUI_OK && !strcmp(actual.sText,"Go"));
    assert(xuiInputDecorationSetOwned(input,decoration,&desc,&callbacks)==XUI_OK && owner.refs==2);
    assert(xuiInputDecorationDescribe(decoration,&actual)==XUI_OK && !strcmp(actual.sText,"Replacement"));
    desc.iKind=XUI_INPUT_DECORATION_CUSTOM_PAINT;
    assert(xuiInputDecorationSetOwned(input,decoration,&desc,&callbacks)==XUI_OK);
    render_input(ui,input,&backend.tProxy);
    assert(owner.paints==1 && xuiInputDecorationGetCount(input,side)==1);
    assert(drop(&owner) && owner.refs==1);
    press(ui,input,decoration);
    assert(owner.calls==1 && owner.refs==0 && finalized==2 && !xuiInputDecorationGetCount(input,side));
    desc.iKind=XUI_INPUT_DECORATION_TEXTURE;
    desc.pUser=NULL;desc.onClick=NULL;desc.onPaint=NULL;
    surface=make_surface(proxy,&finalized);
    desc.pSurface=surface;
    assert(xuiInputDecorationAddOwned(input,side,&decoration,&desc,NULL)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && finalized==2);
    assert(xuiWidgetDestroyChecked(input)==XUI_OK && finalized==3);
    assert(xuiDestroyChecked(ui)==XUI_OK);
    assert(xuiProxyReleaseChecked(foreign)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    assert(backend_owner.refs==1);
    puts("owned Input decorations: graph, failure rollback and self-removal passed");
    return 0;
}
