/* Real public CodeEdit render/input plus standalone publication faults. */
#include "xui_code_margin_host.h"
#include "tests/xui/xui_test_proxy.c"
#if defined(XGE_TEST_STATIC)
#include "src/xui/code/xui_code_margin_internal.h"
#endif
#include <stdio.h>
typedef struct Owner {
    size_t refs;unsigned holds,drops,calls;
    bool holdFalse,holdError,dropFalse,dropError;
    int holdAction,dropAction,callAction;
    xui_code_margin_model model;xui_widget editor;
    void* activeFrame;
} Owner;
static void add(xui_code_margin_model m,int id)
{
    const xui_code_margin_desc_t d={sizeof(d),id,XUI_CODE_MARGIN_CUSTOM,20,XUI_CODE_MARGIN_VISIBLE|XUI_CODE_MARGIN_CLICKABLE,NULL,NULL,NULL};
    assert(xuiCodeMarginModelAdd(m,&d)==XUI_OK);
}
static void action(Owner* o,int code)
{
    if(code==1)assert(xuiCodeMarginModelClearChecked(o->model)==XUI_OK);
    if(code==2){bool consumed=false;assert(xuiCodeMarginModelDestroyOwnerChecked(o->model,&consumed)==XUI_OK&&consumed);}
    if(code==3)assert(xuiWidgetDestroyChecked(o->editor)==XUI_OK);
    if(code==4)add(o->model,30);
}
static bool hold(const void* raw)
{
    Owner* o=(Owner*)raw;++o->holds;if(o->holdFalse)return false;
    ++o->refs;int code=o->holdAction;o->holdAction=0;action(o,code);
    if(o->holdError)xrtSetErrorKind(XERR_STATE);
    return true;
}
static bool drop(const void* raw)
{
    Owner* o=(Owner*)raw;assert(o->refs>1);--o->refs;++o->drops;
    int code=o->dropAction;o->dropAction=0;action(o,code);
    if(o->dropError)xrtSetErrorKind(XERR_STATE);
    return !o->dropFalse;
}
static bool count(const void* raw,size_t* n){*n=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor fn,void* user){(void)raw;(void)fn;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_callback_owner_t owner(Owner* o){return (xui_callback_owner_t){{o,&ops},hold,drop};}
static int render(xui_widget widget,int id,int line,xui_draw_context draw,xui_rect_t rect,void* user)
{
    (void)widget;(void)id;(void)line;(void)draw;(void)rect;
    Owner* o=user;assert(o->refs>1);++o->calls;
#if defined(XGE_TEST_STATIC)
    if(o->activeFrame){
        xui_code_margin_snapshot_t* frame=o->activeFrame;
        assert(xuiInternalCodeMarginSnapshotEnd(frame,XUI_OK)==XUI_ERROR_BUSY);
        bool called=false;
        assert(xuiInternalCodeMarginSnapshotRender(frame,0,widget,line,draw,rect,&called)==XUI_ERROR_BUSY&&!called);
    }
#endif
    int code=o->callAction;o->callAction=0;action(o,code);
    assert(o->refs>1);return XUI_OK;
}
static int event(xui_widget widget,int id,int line,int type,xui_rect_t rect,void* user)
{
    (void)type;return render(widget,id,line,NULL,rect,user);
}
static bool edge(xrtownershipref ref,void* raw)
{assert(ref.Data&&ref.Ops==&ops);++*(unsigned*)raw;return true;}
static void balanced(const xmemdebugsnapshot* a)
{
    xmemdebugsnapshot b;xrtMemDebugSnapshot(&b);
    assert(b.LiveCount==a->LiveCount&&b.LiveBytes==a->LiveBytes&&b.InvalidFreeCount==a->InvalidFreeCount&&b.DoubleFreeCount==a->DoubleFreeCount);
}
static void publication(void)
{
    xui_code_margin_model m=NULL;assert(xuiCodeMarginModelCreate(&m)==XUI_OK);add(m,10);add(m,20);
    Owner a={.refs=1,.model=m},b={.refs=1,.model=m};xui_callback_owner_t oa=owner(&a),ob=owner(&b);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK&&a.refs==2);
    assert(xuiCodeMarginModelSetEventCallbackOwned(m,10,event,&b,&ob)==XUI_OK&&b.refs==2);
    unsigned edges=0;assert(xuiCodeMarginModelTraceOwnership(m,edge,&edges)&&edges==2);
    xui_code_margin_state_t state={0};assert(xuiCodeMarginModelGetState(m,0,&state)==XUI_OK&&state.iId==10);
    xui_code_margin_info_t info;memset(&info,0x5a,sizeof(info));
    assert(xuiCodeMarginModelGet(m,0,&info)==XUI_ERROR_UNSUPPORTED&&info.iSize==UINT32_C(0x5a5a5a5a));
    int n=77;assert(xuiCodeMarginModelLayout(m,(xui_rect_t){0,0,300,100},NULL,0,&n,NULL)==XUI_ERROR_UNSUPPORTED&&n==77);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,NULL,NULL,&oa)==XUI_ERROR_INVALID_ARGUMENT&&a.refs==2);
    b.holdFalse=true;unsigned oldDrops=b.drops;
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&b,&ob)==XUI_ERROR_CALLBACK_FAILED&&a.refs==2&&b.refs==2&&b.drops==oldDrops);
    b.holdFalse=false;b.holdError=true;
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&b,&ob)==XUI_ERROR_CALLBACK_FAILED&&a.refs==2&&b.refs==2&&b.drops==oldDrops+1);
    assert(xrtGetError());xrtClearError();b.holdError=false;
    unsigned held=b.holds;assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&b,&ob)==XUI_ERROR_OUT_OF_MEMORY&&b.holds==held&&a.refs==2);
    xrtMemDebugFailClear();xrtClearError();
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,20,render,&b,&ob)==XUI_OK&&xrtGetError()==ambient&&b.refs==3);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,20,NULL,NULL,NULL)==XUI_OK&&xrtGetError()==ambient&&b.refs==2);
    xrtClearError();
    a.dropFalse=true;
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&b,&ob)==XUI_ERROR_CALLBACK_FAILED&&a.refs==1&&b.refs==3);
    xrtClearError();a.dropFalse=false;
    // A failed consuming Drop never restores the old callback or retries it.
    assert(xuiCodeMarginModelRemove(m,10)==XUI_OK&&b.refs==1&&a.drops==1);
    add(m,10);assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK);
    a.dropAction=4;assert(xuiCodeMarginModelClearChecked(m)==XUI_OK&&a.refs==1&&xuiCodeMarginModelGetCount(m)==1);
    assert(xuiCodeMarginModelGetState(m,0,&state)==XUI_OK&&state.iId==30);
    add(m,10);assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK);
    assert(xuiCodeMarginModelLoadDefaults(m,1,1,1,1)==XUI_OK&&a.refs==1);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,1,render,&a,&oa)==XUI_OK);
    assert(xuiCodeMarginModelSetEventCallbackOwned(m,1,event,&a,&oa)==XUI_OK&&a.refs==3);
    edges=0;assert(xuiCodeMarginModelTraceOwnership(m,edge,&edges)&&edges==2);
    assert(xuiCodeMarginModelDestroyChecked(m)==XUI_OK);
    assert(a.refs==1);
    // Hold can remove its target or consume the unique owner before publish.
    for(int code=1;code<=2;++code){
        assert(xuiCodeMarginModelCreate(&m)==XUI_OK);add(m,10);a=(Owner){.refs=1,.model=m,.holdAction=code};oa=owner(&a);
        int s=xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa);
        assert(s==(code==1?XUI_ERROR_UNSUPPORTED:XUI_ERROR_INVALID_STATE)&&a.refs==1&&a.holds==1&&a.drops==1);
        xrtClearError();if(code==1)assert(xuiCodeMarginModelDestroyChecked(m)==XUI_OK);
    }
    assert(xuiCodeMarginModelCreate(&m)==XUI_OK);add(m,10);a=(Owner){.refs=1,.model=m};oa=owner(&a);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK);
    a.dropAction=2;
    assert(xuiCodeMarginModelClearChecked(m)==XUI_ERROR_INVALID_STATE&&a.refs==1&&a.drops==1);
    xrtClearError();
    puts("owned Margin publication: hold/drop faults, OOM, reentry and two independent edges");
}
static void cold(void)
{
    xmemdebugsnapshot before,modelBytes,slotBefore,slotAfter;
    xrtMemDebugSnapshot(&before);xui_code_margin_model m=NULL;
    assert(xuiCodeMarginModelCreate(&m)==XUI_OK);xrtMemDebugSnapshot(&modelBytes);
    add(m,10);Owner a={.refs=1,.model=m};xui_callback_owner_t oa=owner(&a);
    xrtMemDebugSnapshot(&slotBefore);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK);
    xrtMemDebugSnapshot(&slotAfter);
    assert(modelBytes.AllocCount-before.AllocCount==1&&slotAfter.AllocCount-slotBefore.AllocCount==1);
    printf("owned Margin cold: model=%llu slot=%llu requested bytes; one allocation each\n",
        (unsigned long long)(modelBytes.LiveBytes-before.LiveBytes),
        (unsigned long long)(slotAfter.LiveBytes-slotBefore.LiveBytes));
    assert(xuiCodeMarginModelDestroyChecked(m)==XUI_OK&&a.refs==1);balanced(&before);
}
static xui_test_proxy_state_t backend;
typedef struct UI {xui_context context;xui_widget editor;xui_surface target;xui_font font;} UI;
static UI ui(void)
{
    UI u={0};xuiTestProxyInit(&backend);
    assert(xuiCreate(&u.context)==XUI_OK&&xuiSetProxy(u.context,&backend.tProxy)==XUI_OK);
    assert(backend.tProxy.fontLoadFile(&backend.tProxy,&u.font,"test.ttf",14,0)==XUI_OK);
    assert(xuiSetDefaultFont(u.context,u.font)==XUI_OK&&xuiInputViewport(u.context,320,180)==XUI_OK);
    xui_code_edit_desc_t d={0};d.iSize=sizeof(d);d.sText="one\ntwo\nthree\n";d.pFont=u.font;
    assert(xuiCodeEditCreate(u.context,&u.editor,&d)==XUI_OK);
    xuiWidgetSetRect(u.editor,(xui_rect_t){0,0,320,180});assert(xuiSetRootWidget(u.context,u.editor)==XUI_OK);
    assert(xuiTestSurfaceCreate(&backend,&u.target,320,180,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    assert(xuiLayout(u.context)==XUI_OK&&xuiUpdate(u.context,0.016f)==XUI_OK);return u;
}
static void dispose(UI* u)
{
    assert(xuiDestroyChecked(u->context)==XUI_OK);u->context=NULL;
    backend.tProxy.surfaceDestroy(&backend.tProxy,u->target);
    backend.tProxy.fontDestroy(&backend.tProxy,u->font);
}
static void countedInvalidation(void* user)
{
    ++*(int*)user;
}
static void observedGeometry(void)
{
    UI u=ui();
    xui_code_margin_model model=xuiCodeEditGetMargins(u.editor);
    assert(xuiCodeMarginModelClearChecked(model)==XUI_OK);
    add(model,10);
    xui_lifetime_link_t link={0},detached={0};
    int invalidated=0,detachedCalls=0;
    assert(xuiCodeMarginModelEditorLifetimeWatch(model,&link,countedInvalidation,&invalidated)==XUI_OK);
    assert(xuiCodeMarginModelEditorLifetimeWatch(model,&detached,countedInvalidation,&detachedCalls)==XUI_OK);
    xuiLifetimeUnwatch(&detached);
    assert(xuiCodeMarginModelDestroyChecked(model)==XUI_ERROR_INVALID_STATE);

    const uint32_t dirty=XUI_WIDGET_DIRTY_LAYOUT|XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER;
    xuiWidgetClearDirty(u.editor,dirty);
    assert(xuiCodeMarginModelSetWidth(model,10,24)==XUI_OK);
    assert((xuiWidgetGetDirtyFlags(u.editor)&dirty)==dirty);
    xuiWidgetClearDirty(u.editor,dirty);
    assert(xuiCodeMarginModelSetWidth(model,10,24)==XUI_OK);
    assert((xuiWidgetGetDirtyFlags(u.editor)&dirty)==0);

    Owner state={.refs=1,.model=model,.editor=u.editor};
    xui_callback_owner_t callback=owner(&state);
    assert(xuiCodeEditSetMarginRenderCallbackOwned(u.editor,10,render,&state,&callback)==XUI_OK);
    xui_rect_t viewport={0,0,320,180},text={0};
    int count=0;
    assert(xuiCodeMarginModelMeasureLayout(model,viewport,&count,&text)==XUI_OK);
    assert(count==1&&text.fX==24&&text.fW==296);
    xui_code_margin_hit_t hit={0};
    assert(xuiCodeMarginModelFindHit(model,viewport,2,24,12,0,&hit)==XUI_OK&&hit.iId==10&&hit.iLine==2);
    assert(xuiCodeMarginModelFindHit(model,viewport,100,24,12,0,&hit)==XUI_OK&&hit.iId==0);
    assert(xuiCodeMarginModelFindHit(model,viewport,2,24,0,0,&hit)==XUI_ERROR_INVALID_ARGUMENT);

    xui_call_scope_t scope={0};
    assert(xuiCallScopeBegin(u.context,&scope)==XUI_OK);
    assert(xuiWidgetDestroyChecked(u.editor)==XUI_OK);
    assert(invalidated==1&&detachedCalls==0&&state.refs==2);
    assert(xuiCallScopeEnd(&scope)==XUI_OK&&state.refs==1);
    dispose(&u);
}
static void frameAndEvent(void)
{
    for(int mode=0;mode<3;++mode){
        UI u=ui();xui_code_margin_model m=xuiCodeEditGetMargins(u.editor);
        assert(xuiCodeMarginModelClearChecked(m)==XUI_OK);add(m,10);add(m,20);
        Owner a={.refs=1,.model=m,.editor=u.editor},b={.refs=1,.model=m,.editor=u.editor};
        xui_callback_owner_t oa=owner(&a),ob=owner(&b);
        if(mode<2){
            assert(xuiCodeEditSetMarginRenderCallbackOwned(u.editor,10,render,&a,&oa)==XUI_OK);
            assert(xuiCodeEditSetMarginRenderCallbackOwned(u.editor,20,render,&b,&ob)==XUI_OK);
            a.callAction=mode==0?1:3;
            xui_call_scope_t checked={0};
            if(mode==1)assert(xuiCallScopeBegin(u.context,&checked)==XUI_OK);
            int s=xuiRender(u.context,u.target,NULL,0);
            if(mode==0){assert(s==XUI_OK&&a.calls>0&&b.calls>0&&a.refs==1&&b.refs==1);}
            else {
                /* Legacy xuiRender walks a best-effort void tree. Its actual
                 * callback failure must reach the enclosing checked receipt. */
                assert(s==XUI_OK&&a.calls==1&&!b.calls);
                assert(xuiCallScopeEnd(&checked)==XUI_ERROR_INVALID_STATE&&a.refs==1&&b.refs==1);xrtClearError();
            }
        }else{
            assert(xuiCodeEditSetMarginEventCallbackOwned(u.editor,10,event,&a,&oa)==XUI_OK);
            assert(xuiCodeEditSetMarginRenderCallbackOwned(u.editor,10,render,&b,&ob)==XUI_OK);
            assert(xuiRender(u.context,u.target,NULL,0)==XUI_OK&&b.calls>0&&!a.calls);
            a.callAction=1;
            assert(xuiInputPointerDown(u.context,10,12,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK);
            assert(xuiDispatchPendingEvents(u.context)==XUI_OK&&a.calls>0&&a.refs==1&&b.refs==1);
        }
        dispose(&u);
    }
    puts("owned Margin actual CodeEdit: frame clear, widget close, independent render/event users");
}
#if defined(XGE_TEST_STATIC)
static void privateFrame(void)
{
    UI u=ui();xui_code_margin_model m=xuiCodeEditGetMargins(u.editor);assert(xuiCodeMarginModelClearChecked(m)==XUI_OK);add(m,10);
    Owner a={.refs=1,.model=m,.editor=u.editor};xui_callback_owner_t oa=owner(&a);
    assert(xuiCodeMarginModelSetRenderCallbackOwned(m,10,render,&a,&oa)==XUI_OK);
    xui_code_margin_entry_t entry[2];xui_code_margin_snapshot_t f={0};
    assert(xuiInternalCodeMarginSnapshotBegin(m,(xui_rect_t){0,0,300,100},entry,2,&f,NULL)==XUI_OK);
    xui_code_margin_snapshot_t moved=f;assert(xuiInternalCodeMarginSnapshotEnd(&moved,XUI_OK)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiInternalCodeMarginSnapshotBegin(m,(xui_rect_t){0,0,300,100},entry,2,&f,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiInternalCodeMarginSnapshotEnd(&f,XUI_OK)==XUI_OK);
    assert(xuiInternalCodeMarginSnapshotBegin(m,(xui_rect_t){0,0,300,100},NULL,0,&f,NULL)==XUI_ERROR_LIMIT_EXCEEDED&&!f.self&&a.refs==2);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    unsigned holds=a.holds,drops=a.drops;
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        bool called=false;
        assert(xuiInternalCodeMarginSnapshotBegin(m,(xui_rect_t){0,0,300,100},entry,2,&f,NULL)==XUI_OK);
        a.activeFrame=&f;
        assert(xuiInternalCodeMarginSnapshotRender(&f,0,u.editor,0,NULL,(xui_rect_t){0,0,20,20},&called)==XUI_OK&&called);
        a.activeFrame=NULL;
        assert(xuiInternalCodeMarginSnapshotEnd(&f,XUI_OK)==XUI_OK);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount&&before.ReallocCount==after.ReallocCount&&before.FreeCount==after.FreeCount&&a.holds==holds&&a.drops==drops);
    printf("owned Margin hot: 1000 snapshots/invocations,0 heap,0 external Hold/Drop; entry=%zu frame=%zu bytes\n",sizeof(entry[0]),sizeof(f));
    dispose(&u);assert(a.refs==1);
}
#endif
int main(void)
{
    marginBind();xmemdebugsnapshot baseline;xrtMemDebugSnapshot(&baseline);
    publication();balanced(&baseline);cold();balanced(&baseline);frameAndEvent();balanced(&baseline);
    observedGeometry();balanced(&baseline);
#if defined(XGE_TEST_STATIC)
    privateFrame();balanced(&baseline);
#endif
    puts("owned Margin passed");return 0;
}
