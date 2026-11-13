/* The old-runtime guard exits from the formatter before a freed Icon can be
 * used by the caller. The actual path uses a CPU Proxy, not a native window. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct format_state {
    xui_context context;xui_widget picker;xui_icon_category category;xui_icon selected;
    xui_lifetime_link_t iconLink,contextLink,widgetLink;
    int action,entered,iconGone,contextGone,widgetGone,destroyed,drawn,writeError;
    xerror* expected;
} format_state;
static void iconGone(void* raw){++((format_state*)raw)->iconGone;}
static void contextGone(void* raw){++((format_state*)raw)->contextGone;}
static void widgetGone(void* raw){++((format_state*)raw)->widgetGone;}
static int measure(xui_icon icon,xui_vec2_t* size,void* raw)
{(void)icon;(void)raw;*size=(xui_vec2_t){16,16};return XUI_OK;}
static int draw(xui_icon icon,xui_painter painter,xui_rect_t rect,const xui_icon_draw_desc_t* desc,void* raw)
{(void)icon;(void)painter;(void)rect;(void)desc;++((format_state*)raw)->drawn;return XUI_OK;}
static void destroy(xui_icon icon,void* raw)
{(void)icon;++((format_state*)raw)->destroyed;}
static int format(xui_widget widget,xui_icon icon,char* output,int capacity,void* raw)
{
    format_state* s=raw;assert(widget==s->picker && icon==s->selected && capacity>=5);
    if(s->action!=5)assert(!s->entered);
    ++s->entered;
    switch(s->action){
    case 0:assert(xuiIconRemoveById(s->category,xuiIconGetId(icon))==XUI_OK);break;
    case 1:xuiIconClear(s->category);break;
    case 2:
        assert(xuiIconPickerSetCategory(widget,NULL)==XUI_OK);
        assert(xuiIconCategoryRemove(s->context,"category")==XUI_OK);break;
    case 3:assert(xuiWidgetDestroyChecked(widget)==XUI_OK);break;
    case 4:xuiDestroy(s->context);break;
    case 5:break;
    default:assert(0);
    }
    if(s->iconGone || s->contextGone){
        puts("ICON_RETIRED_INSIDE_PICKER_FORMATTER");fflush(stdout);_Exit(77);
    }
    assert(xuiIconGetName(icon)!=NULL);
    memcpy(output,"name",5);
    if(s->writeError){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);return XUI_ERROR_BACKEND_FAILED;}
    return XUI_OK;
}
static void check(int action,int alias,int ambientPresent,int writeError)
{
    format_state s={0};s.action=action;s.writeError=writeError;
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
    assert(xrtGetError()==NULL && xuiCreate(&s.context)==XUI_OK);
    assert(xuiSetProxy(s.context,&backend.tProxy)==XUI_OK);
    xui_font font=NULL;assert(backend.tProxy.fontLoadMemory(&backend.tProxy,&font,"font",4,14,XUI_FONT_FORMAT_TTF)==XUI_OK);
    assert(xuiSetDefaultFont(s.context,font)==XUI_OK && xuiInputViewport(s.context,640,480)==XUI_OK);
    xui_widget root=NULL;assert(xuiWidgetCreate(s.context,&root)==XUI_OK);
    assert(xuiWidgetSetRect(root,(xui_rect_t){0,0,640,480})==XUI_OK && xuiSetRootWidget(s.context,root)==XUI_OK);
    assert(xuiIconCategoryCreate(s.context,"category",NULL,&s.category)==XUI_OK);
    xui_icon_custom_desc_t custom={0};custom.iSize=sizeof(custom);custom.onMeasure=measure;custom.onDraw=draw;custom.onDestroy=destroy;custom.pUser=&s;
    assert(xuiIconAddCustom(s.category,"custom",&custom,NULL,&s.selected)==XUI_OK);
    if(alias){xui_icon target=s.selected;assert(xuiIconAddAlias(s.category,"alias",target,NULL,&s.selected)==XUI_OK);}
    assert(xuiIconPickerCreate(s.context,&s.picker,NULL)==XUI_OK);
    assert(xuiIconPickerSetCategory(s.picker,s.category)==XUI_OK && xuiIconPickerSetSelectedId(s.picker,xuiIconGetId(s.selected))==XUI_OK);
    assert(xuiWidgetSetRect(s.picker,(xui_rect_t){24,24,220,40})==XUI_OK && xuiWidgetAddChild(root,s.picker)==XUI_OK);
    assert(xuiIconPickerSetTextMode(s.picker,XUI_ICON_PICKER_TEXT_CUSTOM)==XUI_OK && xuiIconPickerSetFormatter(s.picker,format,&s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s.selected,&s.iconLink,iconGone,&s)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.contextLink,contextGone,&s)==XUI_OK);
    assert(xuiWidgetLifetimeWatch(s.picker,&s.widgetLink,widgetGone,&s)==XUI_OK);
    xui_surface target=NULL;assert(xuiTestSurfaceCreate(&backend,&target,640,480,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    assert(xuiLayout(s.context)==XUI_OK && xuiUpdate(s.context,0.016f)==XUI_OK);
    xerror* ambient=NULL;
    if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    /* The legacy raw renderer is best-effort. Language/checked native callers
     * own a receipt around the entire operation to collect callback failures. */
    xui_call_scope_t receipt={0};assert(xuiCallScopeBegin(s.context,&receipt)==XUI_OK);
    xui_rect_i_t full={0,0,640,480};int status=xuiRender(s.context,target,&full,1);
    int checked=xuiCallScopeEnd(&receipt);
    if(status==XUI_OK)status=checked;
    assert(s.entered==1);
    if(writeError){assert(status==XUI_ERROR_BACKEND_FAILED);assert(xrtGetError()==s.expected);}
    else{assert(status==XUI_OK);assert(xrtGetError()==ambient);}
    xrtClearError();xrtErrorFree(ambient);xrtErrorFree(s.expected);
    if(action==5){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        for(int i=0;i<1000;++i){
            assert(xuiWidgetInvalidate(s.picker,XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)==XUI_OK);
            assert(xuiRender(s.context,target,&full,1)==XUI_OK);
        }
        xrtMemDebugSnapshot(&after);
        uint64_t allocations=after.AllocCount-before.AllocCount;
        uint64_t frees=after.FreeCount-before.FreeCount;
        assert(s.entered==1001 && allocations<=1000 && allocations==frees && before.ReallocCount==after.ReallocCount);
        printf("IconPicker formatter heap: 1000 warm renders, %llu allocations, %llu frees, 0 reallocations\n",
            (unsigned long long)allocations,(unsigned long long)frees);
        puts("IconPicker formatter: warm renders stay within the prior native allocation budget");
    }
    if(action<3)assert(s.iconGone==1);
    if(!s.contextGone)xuiDestroy(s.context);
    assert(s.iconGone==1 && s.contextGone==1 && s.widgetGone==1 && s.destroyed==1);
    backend.tProxy.surfaceDestroy(&backend.tProxy,target);
    backend.tProxy.fontDestroy(&backend.tProxy,font);
    xrtClearError();
}
int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    if(getenv("XUI_ICON_PICKER_FORMAT_HEAP_ONLY")){check(5,0,0,0);return 0;}
    for(int action=0;action<5;++action)for(int alias=0;alias<2;++alias)
        for(int ambient=0;ambient<2;++ambient)for(int error=0;error<2;++error)check(action,alias,ambient,error);
    check(5,0,0,0);
    puts("IconPicker formatter: 40 selection/alias/removal/close/error combinations passed");
    return 0;
}
