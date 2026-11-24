#include "xui.h"

#include <assert.h>
#include <stdio.h>

typedef struct Provider {
    size_t references;
    int width, height;
    unsigned queries;
    bool reenter;
} Provider;

static bool count(const void* raw,size_t* output)
{*output=((const Provider*)raw)->references;return true;}

static bool trace(const void* raw,xrtownershipvisitor visit,void* state)
{(void)raw;(void)visit;(void)state;return true;}

static bool hold(const void* raw)
{++((Provider*)raw)->references;return true;}

static bool drop(const void* raw)
{--((Provider*)raw)->references;return true;}

static int describe(xui_proxy proxy,xui_surface surface,xui_surface_desc_t* output)
{
    Provider* provider=proxy->pUser;
    ++provider->queries;
    if(provider->reenter){
        xui_surface_read_extent_t extent={91,92};
        assert(xuiProxyGetSurfaceReadExtent(proxy,surface,&extent)==XUI_ERROR_BUSY);
        assert(extent.iWidth==91 && extent.iHeight==92);
    }
    output->iWidth=provider->width;
    output->iHeight=provider->height;
    return XUI_OK;
}

static int readPixels(xui_proxy proxy,xui_surface surface,void* pixels,int stride)
{
    (void)pixels;
    (void)stride;
    xui_surface_read_extent_t extent={0};
    return xuiProxyGetSurfaceReadExtent(proxy,surface,&extent);
}

typedef struct ExtentThread { xui_proxy proxy; xui_surface surface; } ExtentThread;

static int32 getOnWorker(ptr raw)
{
    const ExtentThread* args=raw;
    xui_surface_read_extent_t extent={0};
    return xuiProxyGetSurfaceReadExtent(args->proxy,args->surface,&extent);
}

int main(void)
{
    static const xrtownershipops ops={count,trace};
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    Provider provider={.references=1,.width=3,.height=4};
    const xui_proxy_t table={.iSize=sizeof(table),.iVersion=XUI_PROXY_VERSION,
        .pUser=&provider,.surfaceGetDesc=describe,.surfaceReadRGBA=readPixels};
    const xui_callback_owner_t owner={{&provider,&ops},hold,drop};
    xui_proxy proxy=NULL,other=NULL;
    assert(xuiProxyCreateCustomOwned(&table,&owner,&proxy)==XUI_OK);
    assert(xuiProxyCreateCustomOwned(&table,&owner,&other)==XUI_OK);
    xui_surface surface=NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface,proxy,NULL)==XUI_OK);

    xui_surface_read_extent_t cached={-1,-2};
    assert(xuiProxyGetSurfaceReadExtent(other,surface,&cached)==XUI_ERROR_INVALID_ARGUMENT);
    assert(cached.iWidth==-1 && cached.iHeight==-2 && provider.queries==0);
    provider.reenter=true;
    assert(xuiProxyGetSurfaceReadExtent(proxy,surface,&cached)==XUI_OK);
    provider.reenter=false;
    assert(cached.iWidth==3 && cached.iHeight==4 && provider.queries==1);
    assert(xrtMemDebugFailAfter(0));
    assert(proxy->surfaceReadRGBA(proxy,surface,NULL,0)==XUI_OK);
    assert(xuiProxyGetSurfaceReadExtent(proxy,surface,&cached)==XUI_OK && provider.queries==1);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();

    xui_surface_desc_t desc={.iWidth=97,.iHeight=98};
    provider.width=7;
    assert(proxy->surfaceGetDesc(proxy,surface,&desc)==XUI_ERROR_INVALID_STATE);
    assert(desc.iWidth==97 && desc.iHeight==98 && provider.queries==2);
    assert(xuiProxyGetSurfaceReadExtent(proxy,surface,&cached)==XUI_OK && provider.queries==2);
    assert(cached.iWidth==3 && cached.iHeight==4);
    provider.width=3;
    assert(proxy->surfaceGetDesc(proxy,surface,&desc)==XUI_OK && provider.queries==3);
    assert(desc.iWidth==3 && desc.iHeight==4);

    xui_surface invalid=NULL;
    assert(xuiSurfaceStageCallbackOutput(&invalid,proxy,NULL)==XUI_OK);
    provider.width=0;
    assert(xuiProxyGetSurfaceReadExtent(proxy,invalid,&cached)==XUI_ERROR_INVALID_ARGUMENT);
    assert(cached.iWidth==3 && cached.iHeight==4);
    assert(xuiSurfaceReleaseChecked(invalid)==XUI_OK);
    ExtentThread args={proxy,surface};
    xthread* worker=xrtThreadCreate(getOnWorker,&args,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK && xrtThreadExitCode(worker)==XUI_ERROR_BUSY);
    xrtThreadDestroy(worker);

    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);
    assert(xuiProxyReleaseChecked(other)==XUI_OK);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && provider.references==1);
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("XUI_CUSTOM_SURFACE_EXTENT_OK");
    return 0;
}
