#include "xui.h"

#include <assert.h>
#include <stdio.h>

typedef struct Backend {
    size_t references;
    xvalue* value;
    xvalueviewv1 read;
    xerror* fontError;
    int drawStatus;
    unsigned surfaces, fonts, draws;
    bool deferSurface, deferFont, deferDraw;
} Backend;

static bool ownerCount(const void* raw,size_t* output)
{
    *output=((const Backend*)raw)->references;
    return true;
}

static bool ownerTrace(const void* raw,xrtownershipvisitor visitor,void* user)
{
    (void)raw;
    (void)visitor;
    (void)user;
    return true;
}

static bool ownerHold(const void* raw)
{++((Backend*)raw)->references;return true;}

static bool ownerDrop(const void* raw)
{--((Backend*)raw)->references;return true;}

static void surfaceDestroy(xui_proxy proxy,xui_surface surface)
{
    Backend* state=proxy->pUser;
    xvalue* value=NULL;
    assert(xuiSurfaceGetProxy(surface)==proxy);
    assert(xuiSurfaceGetValueOwned(surface,&value)==XUI_OK && value==state->value);
    xui_surface resurrected=NULL;
    assert(xuiSurfaceShareOwned(surface,&resurrected)==XUI_ERROR_INVALID_STATE && !resurrected);
    xrtValueRelease(value);
    ++state->surfaces;
    if(state->deferSurface)assert(xrtValueViewBeginV1(state->value,&state->read));
}

static void fontDestroy(xui_proxy proxy,xui_font font)
{
    Backend* state=proxy->pUser;
    xvalue* value=NULL;
    assert(xuiFontGetProxy(font)==proxy);
    assert(xuiFontGetValueOwned(font,&value)==XUI_OK && value==state->value);
    xrtValueRelease(value);
    ++state->fonts;
    if(state->deferFont)assert(xrtValueViewBeginV1(state->value,&state->read));
    if(state->fontError)xrtSetErrorTake(xrtErrorRef(state->fontError));
}

static int drawEnd(xui_proxy proxy,xui_draw_context draw)
{
    Backend* state=proxy->pUser;
    xvalue* value=NULL;
    assert(xuiDrawGetProxy(draw)==proxy);
    assert(xuiDrawGetValueOwned(draw,&value)==XUI_OK && value==state->value);
    xrtValueRelease(value);
    ++state->draws;
    if(state->deferDraw)assert(xrtValueViewBeginV1(state->value,&state->read));
    return state->drawStatus;
}

static void testHooks(void)
{
    static const xrtownershipops ops={ownerCount,ownerTrace};
    Backend state={.references=1,.value=xrtValueObject()};
    const xui_proxy_t table={
        .iSize=sizeof(table),.iVersion=XUI_PROXY_VERSION,.pUser=&state,
        .surfaceDestroy=surfaceDestroy,.fontDestroy=fontDestroy,.drawEnd=drawEnd,
    };
    const xui_callback_owner_t owner={{&state,&ops},ownerHold,ownerDrop};
    xui_proxy proxy=NULL;
    assert(state.value && xuiProxyCreateCustomOwned(&table,&owner,&proxy)==XUI_OK);
    assert(state.references==2 && proxy->surfaceDestroy!=surfaceDestroy);
    assert(proxy->fontDestroy!=fontDestroy && proxy->drawEnd!=drawEnd);

    xui_surface surface=NULL, alias=NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface,proxy,state.value)==XUI_OK);
    assert(xuiSurfaceShareOwned(surface,&alias)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && state.surfaces==0);
    proxy->surfaceDestroy(proxy,alias);
    assert(!xrtGetError() && state.surfaces==1);

    /* The semantic hook starts a read. Checked retirement keeps exactly the
     * same owner, while the consuming table slot moves it into the queue. */
    state.deferSurface=true;
    surface=NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface,proxy,state.value)==XUI_OK);
    bool consumed=true;
    assert(xuiSurfaceReleaseOwnerChecked(surface,&consumed)==XUI_ERROR_BUSY && !consumed);
    assert(state.surfaces==2 && xuiSurfaceGetProxy(surface)==proxy && !xrtGetError());
    assert(xuiSurfaceReleaseOwnerChecked(surface,&consumed)==XUI_ERROR_BUSY && !consumed);
    assert(xrtValueViewEndV1(&state.read));
    assert(xuiSurfaceReleaseOwnerChecked(surface,&consumed)==XUI_OK && consumed && state.surfaces==2);

    surface=NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface,proxy,state.value)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    proxy->surfaceDestroy(proxy,surface);
    assert(state.surfaces==3 && !xrtGetError() && !xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    assert(xuiProxyDrainCallbackRollbacks(proxy)==XUI_ERROR_BUSY);
    assert(xrtValueViewEndV1(&state.read));
    assert(xuiProxyDrainCallbackRollbacks(proxy)==XUI_OK && state.surfaces==3);

    xui_font font=NULL;
    assert(xuiFontStageCallbackOutput(&font,proxy,state.value)==XUI_OK);
    state.fontError=xrtErrorCreate(XERR_STATE,"test",19,"font callback failure");
    assert(state.fontError);
    assert(xuiFontReleaseOwnerChecked(font,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed);
    assert(state.fonts==1 && xrtGetError()==state.fontError);
    xrtClearError();xrtErrorFree(state.fontError);state.fontError=NULL;

    xui_draw_context draw=NULL;
    assert(xuiDrawStageCallbackOutput(&draw,proxy,state.value,NULL)==XUI_OK);
    assert(xuiDrawEndChecked(draw)==XUI_OK && state.draws==1);
    assert(proxy->drawEnd(proxy,draw)==XUI_OK && state.draws==1);
    draw=NULL;state.drawStatus=XUI_ERROR_BUSY;
    assert(xuiDrawStageCallbackOutput(&draw,proxy,state.value,NULL)==XUI_OK);
    assert(proxy->drawEnd(proxy,draw)==XUI_ERROR_CALLBACK_FAILED && state.draws==2);
    assert(!xrtGetError());
    draw=NULL;state.drawStatus=XUI_OK;state.deferDraw=true;
    assert(xuiDrawStageCallbackOutput(&draw,proxy,state.value,NULL)==XUI_OK);
    assert(proxy->drawEnd(proxy,draw)==XUI_OK && state.draws==3);
    assert(xuiProxyDrainCallbackRollbacks(proxy)==XUI_ERROR_BUSY);
    assert(xrtValueViewEndV1(&state.read));
    assert(xuiProxyDrainCallbackRollbacks(proxy)==XUI_OK && state.draws==3);

    /* Explicitly constructing an ordinary C owned table retains its own
     * slots and previous payload behavior. */
    xui_proxy old=NULL;
    assert(xuiProxyCreateOwned(&table,&owner,&old)==XUI_OK && old->surfaceDestroy==surfaceDestroy);
    surface=NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface,old,state.value)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && state.surfaces==3);
    assert(xuiProxyReleaseChecked(old)==XUI_OK);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && state.references==1);
    xrtValueRelease(state.value);
}

static void testDetachedContextFont(void)
{
    static const xrtownershipops ops={ownerCount,ownerTrace};
    Backend state={.references=1,.value=xrtValueObject(),.deferFont=true};
    xui_proxy_t table=xuiProxyXge();
    table.pUser=&state;
    table.fontDestroy=fontDestroy;
    const xui_callback_owner_t owner={{&state,&ops},ownerHold,ownerDrop};
    xui_proxy proxy=NULL;
    xui_context context=NULL;
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    assert(state.value && xuiProxyCreateCustomOwned(&table,&owner,&proxy)==XUI_OK);
    assert(xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK);

    xui_font font=NULL;
    assert(xuiFontCreateWithValueOwned(proxy,state.value,&font)==XUI_OK);
    assert(xuiSetDefaultFont(context,font)==XUI_OK);
    assert(xuiFontReleaseChecked(font)==XUI_OK);
    bool consumed=true;
    assert(!xrtGetError());
    assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    assert(state.fonts==1 && !xrtGetError());
    assert(xuiProxyDrainCallbackRollbacks(proxy)==XUI_ERROR_BUSY);
    assert(xrtValueViewEndV1(&state.read));
    assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed);
    assert(state.fonts==1 && state.references==1);
    xrtValueRelease(state.value);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    testHooks();
    testDetachedContextFont();
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.DoubleFreeCount==after.DoubleFreeCount && before.InvalidFreeCount==after.InvalidFreeCount);
    puts("XUI_CUSTOM_PROXY_RETIREMENT_OK");
    return 0;
}
