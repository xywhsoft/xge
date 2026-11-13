/* Only Icon cache load/clear operations are exercised on this CPU provider.
 * Their Surface callbacks create real owned controls. Other inherited raw
 * rendering callbacks are not used; this is not an owned frame backend. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
typedef struct owned_clear {
    xui_test_proxy_state_t backend;
    xui_context context;xui_icon icon;xui_lifetime_link_t contextLink;
    unsigned refs,created,freed,entered,gone;
    int action,fail,diagnostic,armed,tail;
    xerror* expected;
} owned_clear;
static bool hold(const void* raw){++((owned_clear*)raw)->refs;return true;}
static bool drop(const void* raw){owned_clear* s=(owned_clear*)raw;assert(s->refs>1);--s->refs;return true;}
static bool count(const void* raw,size_t* output){*output=((const owned_clear*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops operations={count,trace};
static void gone(void* raw){++((owned_clear*)raw)->gone;}
static int finalize(xui_proxy proxy,void* payload)
{
    owned_clear* s=*(owned_clear**)payload;assert(proxy->pUser==s);++s->freed;
    if(!s->armed){if(s->tail)xrtSetErrorKind(XERR_RANGE);return XUI_OK;}
    s->armed=0;++s->entered;
    if(s->action==1)assert(xuiIconTouch(s->icon)==XUI_OK);
    else if(s->action==2)xuiDestroy(s->context);
    assert(!s->gone);
    if(s->fail && s->diagnostic){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
    s->tail=s->fail;
    return s->fail?XUI_ERROR_BACKEND_FAILED:XUI_OK;
}
static int surface(xui_proxy proxy,xui_surface* output)
{
    owned_clear* s=proxy->pUser;void* payload=NULL;
    int status=xuiSurfaceCreateOwned(proxy,sizeof(owned_clear*),finalize,output,&payload);
    if(status==XUI_OK){*(owned_clear**)payload=s;++s->created;}return status;
}
static int svg(xui_proxy proxy,xui_surface* output,const void* data,int size,int width,int height,uint32_t flags)
{(void)data;(void)size;(void)width;(void)height;(void)flags;return surface(proxy,output);}
static int raster(xui_proxy proxy,xui_surface* output,const void* data,int size,uint32_t flags)
{(void)data;(void)size;(void)flags;return surface(proxy,output);}
static int descriptor(xui_proxy proxy,xui_surface value,xui_surface_desc_t* output)
{
    assert(xuiSurfaceGetProxy(value)->_pCapability==proxy->_pCapability);*output=(xui_surface_desc_t){0};
    output->iWidth=16;output->iHeight=16;return XUI_OK;
}
static void forbidden(xui_proxy proxy,xui_surface value)
{(void)proxy;(void)value;assert(!"owned cache must release its actual native reference");}
static void check(int source,int action,int fail,int diagnostic,int ambientPresent)
{
    owned_clear s={0};s.refs=1;s.action=action;s.fail=fail;s.diagnostic=diagnostic;
    xuiTestProxyInit(&s.backend);s.backend.tProxy.surfaceLoadSvgMemory=svg;s.backend.tProxy.surfaceLoadMemory=raster;
    s.backend.tProxy.surfaceGetDesc=descriptor;s.backend.tProxy.surfaceDestroy=forbidden;
    xui_callback_owner_t owner={{&s,&operations},hold,drop};xui_proxy proxy=NULL;
    assert(xuiProxyCreateOwned(&s.backend.tProxy,&owner,&proxy)==XUI_OK);
    xui_context_desc_t contextDesc;xuiContextDescDefault(&contextDesc);contextDesc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateOwnedEx(proxy,&contextDesc,&s.context)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.contextLink,gone,&s)==XUI_OK);
    xui_icon_category category=NULL;assert(xuiIconCategoryCreate(s.context,"icons",NULL,&category)==XUI_OK);
    const char data[]="cpu";
    if(source==1)assert(xuiIconAddRasterMemory(category,"image",data,sizeof(data),NULL,&s.icon)==XUI_OK);
    else assert(xuiIconAddSvgMemory(category,"image",data,sizeof(data),NULL,&s.icon)==XUI_OK);
    assert(xuiIconPrepare(s.icon,16,16)==XUI_OK);
    if(source!=1)assert(xuiIconPrepare(s.icon,32,32)==XUI_OK);
    xerror* ambient=NULL;if(ambientPresent){xrtSetErrorKind(XERR_ARGUMENT);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    s.armed=1;int status;
    if(source==2){xui_icon_category_desc_t desc;xuiIconCategoryDescDefault(&desc);desc.iCacheCapacity=3;status=xuiIconCategorySetDesc(category,&desc);}
    else status=xuiIconTouch(s.icon);
    xerror* result=xrtTakeError();assert(s.entered==1);
    assert(status==(fail?XUI_ERROR_BACKEND_FAILED:XUI_OK));
    assert(result==(fail?s.expected:ambient));
    s.tail=0;if(!s.gone)xuiDestroy(s.context);
    assert(s.gone==1 && s.created==s.freed && !xrtGetError());
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && s.refs==1);
    xrtErrorFree(result);xrtErrorFree(s.expected);xrtErrorFree(ambient);
}
int main(void)
{
    for(int source=0;source<3;++source)for(int action=0;action<3;++action)
        for(int fail=0;fail<2;++fail)for(int diagnostic=0;diagnostic<2;++diagnostic)
            for(int ambient=0;ambient<2;++ambient)check(source,action,fail,diagnostic,ambient);
    puts("Owned Icon cache clear: 72 exact status/nullable diagnostic, reentry and Context-close cases passed");return 0;
}
