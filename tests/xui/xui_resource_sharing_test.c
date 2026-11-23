#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include <xge.h>
#include <xui.h>
#include <assert.h>
#include <stdio.h>

typedef struct {
    xui_proxy proxy;
    xui_font font;
    xui_surface surface;
    const xui_text_shape_t* shape;
    xui_draw_context draw;
} resource_views;

static int other_thread(void* raw)
{
    resource_views views=*(resource_views*)raw;
    const resource_views source=views;
    assert(xuiProxyShareOwned(source.proxy,&views.proxy)==XUI_ERROR_BUSY && !views.proxy);
    assert(xuiFontShareOwned(source.font,&views.font)==XUI_ERROR_BUSY && !views.font);
    assert(xuiSurfaceShareOwned(source.surface,&views.surface)==XUI_ERROR_BUSY && !views.surface);
    assert(xuiTextShapeShareOwned(source.shape,&views.shape)==XUI_ERROR_BUSY && !views.shape);
    assert(xuiDrawShareOwned(source.draw,&views.draw)==XUI_ERROR_BUSY && !views.draw);
    return 0;
}

static int end_draw(xui_proxy proxy,void* data)
{
    unsigned* ended=*(unsigned**)data;
    assert(xuiProxyIsOwned(proxy) && *ended==0);
    ++*ended;
    return XUI_OK;
}

int main(void)
{
    resource_views source={0},shared={0};
    assert(xuiProxyXgeCreate(&source.proxy)==XUI_OK);
    xui_proxy_t legacy=xuiProxyXge();
    shared.proxy=source.proxy;
    assert(xuiProxyShareOwned(&legacy,&shared.proxy)==XUI_ERROR_INVALID_ARGUMENT && !shared.proxy);
    assert(xuiProxyShareOwned(NULL,&shared.proxy)==XUI_ERROR_INVALID_ARGUMENT && !shared.proxy);
    xui_proxy_t view=*source.proxy;

    const xui_system_font_desc_t font_desc={.iSize=sizeof(font_desc),.fSize=16};
    assert(source.proxy->fontLoadSystem(source.proxy,&source.font,&font_desc)==XUI_OK);
    const xui_text_item_t item={.iSize=sizeof(item),.pFont=source.font,.sText="abc",
        .iTextSize=3,.iFlags=XUI_TEXT_SHAPE_DEFAULT};
    assert(xuiTextShapeCreateOwned(source.proxy,&item,&source.shape)==XUI_OK);
    const unsigned char pixel[]={44,18,7,255};
    assert(source.proxy->surfaceCreateRGBA(source.proxy,&source.surface,1,1,pixel,0,0)==XUI_OK);
    unsigned ended=0;
    void* payload=NULL;
    assert(xuiDrawCreateOwned(source.proxy,source.surface,sizeof(unsigned*),end_draw,&source.draw,&payload)==XUI_OK);
    *(unsigned**)payload=&ended;

    xthread* worker=xrtThreadCreate(other_thread,&source,0);
    assert(worker && xrtThreadWaitFor(worker,5000)==XWAIT_OK);
    xrtThreadDestroy(worker);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiProxyShareOwned(&view,&shared.proxy)==XUI_OK && shared.proxy==source.proxy);
    assert(xuiFontShareOwned(source.font,&shared.font)==XUI_OK && shared.font==source.font);
    assert(xuiSurfaceShareOwned(source.surface,&shared.surface)==XUI_OK && shared.surface==source.surface);
    assert(xuiTextShapeShareOwned(source.shape,&shared.shape)==XUI_OK && shared.shape==source.shape);
    assert(xuiDrawShareOwned(source.draw,&shared.draw)==XUI_OK && shared.draw==source.draw);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();

    assert(xuiProxyReleaseChecked(source.proxy)==XUI_OK);
    assert(xuiFontReleaseChecked(source.font)==XUI_OK && xuiFontGetProxy(shared.font)==shared.proxy);
    assert(xuiSurfaceReleaseChecked(source.surface)==XUI_OK && xuiSurfaceGetProxy(shared.surface)==shared.proxy);
    assert(xuiTextShapeReleaseChecked(source.shape)==XUI_OK);
    const xui_text_cluster_t* clusters=NULL;
    size_t count=0;
    assert(xuiTextShapeGetClusters(shared.shape,&clusters,&count)==XUI_OK && clusters && count==3);
    assert(xuiDrawReleaseChecked(source.draw)==XUI_OK && !ended && xuiDrawGetTarget(shared.draw)==shared.surface);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xuiDrawEndChecked(shared.draw)==XUI_OK && ended==1 && xuiDrawEnded(shared.draw));
    xui_draw_context rejected=shared.draw;
    assert(xuiDrawShareOwned(shared.draw,&rejected)==XUI_ERROR_INVALID_STATE && !rejected);
    assert(xuiDrawReleaseChecked(shared.draw)==XUI_OK && ended==1);
    assert(xuiTextShapeReleaseChecked(shared.shape)==XUI_OK);
    assert(xuiFontReleaseChecked(shared.font)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(shared.surface)==XUI_OK);
    assert(xuiProxyReleaseChecked(shared.proxy)==XUI_OK && xgeUnitChecked()==XGE_OK);
    puts("xui_resource_sharing_test passed");
    return 0;
}
