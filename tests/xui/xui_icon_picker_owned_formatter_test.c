/* Owned formatter publication, snapshot lifetime, text consumption and graph.
 * Raw CPU surfaces belong only to this explicitly legacy native fixture. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct owned_state {
    xui_test_proxy_state_t backend;
    xui_context context;xui_widget picker;xui_icon_category category;xui_icon icon;
    xui_font font;xui_surface target;xui_lifetime_link_t contextLink;
    unsigned refs,calls,drops,gone;int mode;
    bool failHold,failDrop,replaceOnHold;
    xui_draw_text_proc originalText;char lastText[256];
    xerror* expected;
} owned_state;
static int legacy(xui_widget w,xui_icon icon,char* output,int capacity,void* user)
{(void)w;(void)icon;(void)user;snprintf(output,(size_t)capacity,"legacy");return XUI_OK;}
static bool hold(const void* raw)
{
    owned_state* s=(owned_state*)raw;if(s->failHold)return false;++s->refs;
    if(s->replaceOnHold){s->replaceOnHold=false;assert(xuiIconPickerSetFormatter(s->picker,legacy,s)==XUI_OK);}
    return true;
}
static bool drop(const void* raw)
{owned_state* s=(owned_state*)raw;assert(s->refs>1);--s->refs;++s->drops;return !s->failDrop;}
static bool count(const void* raw,size_t* output)
{*output=((const owned_state*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops operations={count,trace};
static xui_callback_owner_t owner(owned_state* s)
{return (xui_callback_owner_t){{s,&operations},hold,drop};}
static void gone(void* raw){++((owned_state*)raw)->gone;}
static int captureText(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    owned_state* s=(owned_state*)proxy->pUser;
    size_t size=item->iTextSize<0 ? strlen(item->sText):(size_t)item->iTextSize;
    if(size>=sizeof(s->lastText))size=sizeof(s->lastText)-1;
    memcpy(s->lastText,item->sText,size);s->lastText[size]=0;
    return s->originalText(proxy,draw,item,rect,color,flags);
}
static xstrview text(xui_widget w,xui_icon icon,void* raw)
{
    owned_state* s=raw;assert(w==s->picker && icon==s->icon && s->refs>=2);++s->calls;
    if(s->mode==1)assert(xuiIconPickerSetFormatter(w,legacy,s)==XUI_OK);
    if(s->mode==2)assert(xuiIconRemoveById(s->category,xuiIconGetId(icon))==XUI_OK);
    if(s->mode==3)assert(xuiWidgetDestroyChecked(w)==XUI_OK);
    if(s->mode==4)xuiDestroy(s->context);
    assert(s->refs>=2 && !s->gone);
    if(s->mode==5){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);return (xstrview){0};}
    if(s->mode==6)return (xstrview){NULL,1};
    if(s->mode==7){char* p=xrtMalloc(1);assert(p);*p=0;return (xstrview){p,SIZE_MAX};}
    size_t size=s->mode==8 ? 257:5;
    char* p=xrtMalloc(size+1);assert(p);
    if(s->mode==8){memset(p,'a',254);p[254]=(char)0xe4;p[255]=(char)0xbd;p[256]=(char)0xa0;p[257]=0;}
    else if(s->mode==9)memcpy(p,"ow\0ed",6);
    else memcpy(p,"owned",6);
    return (xstrview){p,size};
}
static void create(owned_state* s)
{
    assert(!xrtGetError());s->refs=1;xuiTestProxyInit(&s->backend);
    s->originalText=s->backend.tProxy.drawText;s->backend.tProxy.drawText=captureText;
    assert(xuiCreate(&s->context)==XUI_OK && xuiSetProxy(s->context,&s->backend.tProxy)==XUI_OK);
    assert(s->backend.tProxy.fontLoadMemory(&s->backend.tProxy,&s->font,"font",4,14,XUI_FONT_FORMAT_TTF)==XUI_OK);
    assert(xuiSetDefaultFont(s->context,s->font)==XUI_OK && xuiInputViewport(s->context,640,480)==XUI_OK);
    xui_widget root=NULL;assert(xuiWidgetCreate(s->context,&root)==XUI_OK);
    assert(xuiWidgetSetRect(root,(xui_rect_t){0,0,640,480})==XUI_OK && xuiSetRootWidget(s->context,root)==XUI_OK);
    assert(xuiIconCategoryCreate(s->context,"category",NULL,&s->category)==XUI_OK);
    assert(xuiIconAddSvgPath(s->category,"icon","M0 0 H16 V16 H0 Z",(xui_rect_t){0,0,16,16},NULL,NULL,&s->icon)==XUI_OK);
    assert(xuiIconPickerCreate(s->context,&s->picker,NULL)==XUI_OK);
    assert(xuiIconPickerSetCategory(s->picker,s->category)==XUI_OK && xuiIconPickerSetSelectedId(s->picker,xuiIconGetId(s->icon))==XUI_OK);
    assert(xuiWidgetSetRect(s->picker,(xui_rect_t){24,24,220,40})==XUI_OK && xuiWidgetAddChild(root,s->picker)==XUI_OK);
    assert(xuiIconPickerSetTextMode(s->picker,XUI_ICON_PICKER_TEXT_CUSTOM)==XUI_OK);
    assert(xuiContextLifetimeWatch(s->context,&s->contextLink,gone,s)==XUI_OK);
    assert(xuiTestSurfaceCreate(&s->backend,&s->target,640,480,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    assert(xuiLayout(s->context)==XUI_OK && xuiUpdate(s->context,0.016f)==XUI_OK);
}
static int render(owned_state* s)
{
    xui_call_scope_t call={0};assert(xuiCallScopeBegin(s->context,&call)==XUI_OK);
    xui_rect_i_t full={0,0,640,480};int status=xuiRender(s->context,s->target,&full,1);
    int checked=xuiCallScopeEnd(&call);return status!=XUI_OK ? status:checked;
}
static void finish(owned_state* s)
{
    s->failDrop=false;
    if(!s->gone)xuiDestroy(s->context);
    assert(s->gone==1 && s->refs==1);
    s->backend.tProxy.surfaceDestroy(&s->backend.tProxy,s->target);
    s->backend.tProxy.fontDestroy(&s->backend.tProxy,s->font);
    xrtClearError();xrtErrorFree(s->expected);
}
static const char* preview(owned_state* s)
{return s->lastText;}
static bool visit(xrtownershipref ref,void* raw)
{unsigned* edges=raw;if(ref.Ops==&operations && ref.Data)++*edges;return true;}
static void check(int mode,int ambientPresent)
{
    owned_state s={0};s.mode=mode;create(&s);xui_callback_owner_t binding=owner(&s);
    assert(xuiIconPickerSetFormatterOwned(s.picker,text,&s,&binding)==XUI_OK && s.refs==2);
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(s.context,visit,&edges) && edges==1);assert(xrtOwnershipScopeEnd(&freeze));
    xerror* ambient=NULL;if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    int status=render(&s);assert(s.calls==1);
    if(mode==5){assert(status==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==s.expected);}
    else if(mode==6 || mode==7){assert(status==XUI_ERROR_INVALID_ARGUMENT && !xrtGetError());}
    else{
        assert(status==XUI_OK && xrtGetError()==ambient);
        if(mode!=3 && mode!=4){
            const char* value=preview(&s);
            if(mode==8){assert(strlen(value)==254);for(int i=0;i<254;++i)assert(value[i]=='a');}
            else assert(!strcmp(value,mode==9 ? "ow":"owned"));
        }
    }
    xrtClearError();xrtErrorFree(ambient);
    if(mode==1){
        assert(s.refs==1);assert(xuiWidgetInvalidate(s.picker,XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)==XUI_OK);
        assert(render(&s)==XUI_OK && !strcmp(preview(&s),"legacy"));
    }
    finish(&s);
}
static void registration(void)
{
    for(int allowance=0;allowance<8;++allowance){
        owned_state s={0};create(&s);xui_callback_owner_t binding=owner(&s);
        assert(xuiIconPickerSetFormatter(s.picker,legacy,&s)==XUI_OK);
        assert(xrtMemDebugFailAfter(allowance));int status=xuiIconPickerSetFormatterOwned(s.picker,text,&s,&binding);
        bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        if(status!=XUI_OK){assert(failed && s.refs==1);assert(render(&s)==XUI_OK && !strcmp(preview(&s),"legacy"));}
        else{assert(s.refs==2 && render(&s)==XUI_OK && !strcmp(preview(&s),"owned"));}
        finish(&s);
    }
    owned_state s={0};create(&s);xui_callback_owner_t binding=owner(&s);
    s.failHold=true;assert(xuiIconPickerSetFormatterOwned(s.picker,text,&s,&binding)==XUI_ERROR_CALLBACK_FAILED && s.refs==1);xrtClearError();s.failHold=false;
    assert(xuiIconPickerSetFormatterOwned(s.picker,text,&s,&binding)==XUI_OK);
    s.replaceOnHold=true;assert(render(&s)==XUI_OK && s.refs==1 && !strcmp(preview(&s),"owned"));
    assert(xuiIconPickerSetFormatterOwned(s.picker,text,&s,&binding)==XUI_OK);
    s.failDrop=true;assert(render(&s)==XUI_ERROR_CALLBACK_FAILED);xrtClearError();s.failDrop=false;
    assert(xuiIconPickerSetFormatter(s.picker,legacy,&s)==XUI_OK && s.refs==1);
    finish(&s);
}
int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    for(int mode=0;mode<10;++mode)for(int ambient=0;ambient<2;++ambient)check(mode,ambient);
    registration();puts("Owned IconPicker formatter: 20 text/reentry/lifetime/error cases, 8 OOM budgets and owner publication passed");return 0;
}
