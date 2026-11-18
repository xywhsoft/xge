/* Inspect the production render queue without creating a window or GL. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>

static void sameRect(xge_rect_t a, xge_rect_t b)
{
    assert(a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH);
}

static void compareLegacy(xge_texture texture, xge_nine_patch_style_t style, xge_rect_t dst)
{
    xge_nine_patch_t legacy;
    xgeNinePatchInit(&legacy, texture, style.tSource, style.fX1, style.fY1, style.fX2, style.fY2);
    xgeNinePatchSetMode(&legacy, style.iMode);
    xgeNinePatchSetColor(&legacy, style.iColor);
    xgeNinePatchDraw(&legacy, dst, XGE_DRAW_SCREEN_SPACE);
    int count = g_xge.iRenderCommandCount;
    assert(count > 0);
    assert(xgeTextureDrawNinePatch(texture, &style, dst, XGE_DRAW_SCREEN_SPACE) == XGE_OK);
    assert(g_xge.iRenderCommandCount == 2 * count && texture->iRefCount == 2 * count + 1);
    for (int i = 0; i < count; ++i) {
        const xge_draw_t* a = &g_xge.pRenderCommands[i].u.tDraw;
        const xge_draw_t* b = &g_xge.pRenderCommands[count + i].u.tDraw;
        assert(a->pTexture == texture && b->pTexture == texture);
        assert(a->iColor == b->iColor && a->iFlags == b->iFlags);
        sameRect(a->tSrc, b->tSrc);
        sameRect(a->tDst, b->tDst);
    }
    __xgeRenderCommandUnit();
    assert(texture->iRefCount == 1);
}

static int32 wrongThread(ptr input)
{
    xge_nine_patch_style_t style = xgeNinePatchStyleDefault();
    return xgeTextureDrawNinePatch(input, &style, (xge_rect_t){0,0,8,8}, 0);
}

int main(void)
{
    xge_nine_patch_style_t style = xgeNinePatchStyleDefault();
    xge_rect_t dst = {1, 2, 17, 11};
    assert(style.iMode == XGE_NINE_PATCH_STRETCH && style.iColor == UINT32_MAX);
    assert(style.tSource.fW == 0 && style.fX1 == 0 && style.fX2 == 1);
    xge_texture_t empty = {0};
    assert(xgeTextureDrawNinePatch(&empty, &style, dst, 0) == XGE_ERROR_NOT_INITIALIZED);
    assert(xrtMemDebugEnable(true));
    xge_desc_t desc = {0};
    desc.sTitle = "Nine-patch queue qualification";
    assert(xgeInit(&desc) == XGE_OK);
    unsigned char pixels[4 * 4 * 4];
    memset(pixels, 255, sizeof(pixels));
    xge_texture texture = NULL;
    assert(xgeTextureCreateRGBAOwned(&texture, 4, 4, pixels, sizeof(pixels)) == XGE_OK);

    xthread* worker = xrtThreadCreate(wrongThread, texture, 0);
    assert(worker && xrtThreadWaitFor(worker, 30000) == XWAIT_OK);
    assert(xrtThreadExitCode(worker) == XGE_ERROR_INVALID_STATE);
    xrtThreadDestroy(worker);
    assert(g_xge.iRenderCommandCount == 0 && texture->iRefCount == 1);

    xge_nine_patch_style_t cuts = style;
    cuts.fX1 = cuts.fY1 = 0.75f;
    cuts.fX2 = cuts.fY2 = 0.25f;
    cuts.iColor = XGE_COLOR_RGBA(150, 70, 220, 235);
    compareLegacy(texture, cuts, dst);
    cuts.iMode = XGE_NINE_PATCH_TILE;
    compareLegacy(texture, cuts, dst);
    compareLegacy(texture, cuts, (xge_rect_t){1, 1, 0.5f, 0.75f});

    xge_nine_patch_style_t crop = style;
    crop.tSource = (xge_rect_t){1, 1, 2, 3};
    assert(xgeTextureDrawNinePatch(texture, &crop, dst, 0) == XGE_OK);
    assert(g_xge.iRenderCommandCount == 1);
    sameRect(g_xge.pRenderCommands[0].u.tDraw.tSrc, crop.tSource);
    sameRect(g_xge.pRenderCommands[0].u.tDraw.tDst, dst);
    __xgeRenderCommandUnit();

    crop.tSource = (xge_rect_t){0, 0, 1, 1};
    crop.iMode = XGE_NINE_PATCH_TILE;
    assert(xgeTextureDrawNinePatch(texture, &crop, (xge_rect_t){0,0,100,180}, 0) == XGE_OK);
    assert(g_xge.iRenderCommandCount == 16386);
    sameRect(g_xge.pRenderCommands[16384].u.tDraw.tDst, (xge_rect_t){84,163,16,1});
    sameRect(g_xge.pRenderCommands[16385].u.tDraw.tDst, (xge_rect_t){0,164,100,16});
    __xgeRenderCommandUnit();

    cuts.fX1 = NAN;
    assert(xgeTextureDrawNinePatch(texture, &cuts, dst, 0) == XGE_ERROR_INVALID_ARGUMENT);
    cuts = style;
    cuts.iMode = -1;
    assert(xgeTextureDrawNinePatch(texture, &cuts, dst, 0) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeTextureDrawNinePatch(texture, &style, (xge_rect_t){0,0,-1,2}, 0) == XGE_ERROR_INVALID_ARGUMENT);
    cuts = style;
    cuts.iColor = 0;
    assert(xgeTextureDrawNinePatch(texture, &cuts, dst, 0) == XGE_OK);
    assert(g_xge.iRenderCommandCount == 0 && texture->iRefCount == 1);

    __xgeTextureUploadQueueRemove(texture);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeTextureDrawNinePatch(texture, &style, dst, 0) == XGE_ERROR_OUT_OF_MEMORY);
    assert(g_xge.iRenderCommandCount == 0 && texture->iRefCount == 1 && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear(); xrtClearError();
    assert(xgeTextureUploadQueue(texture) == XGE_OK);
    cuts = style;
    cuts.fX1 = cuts.fY1 = 0.25f;
    cuts.fX2 = cuts.fY2 = 0.75f;
    cuts.iMode = XGE_NINE_PATCH_TILE;
    assert(xrtMemDebugFailAfter(1));
    assert(xgeTextureDrawNinePatch(texture, &cuts, (xge_rect_t){0,0,64,64}, 0) == XGE_ERROR_OUT_OF_MEMORY);
    assert(g_xge.iRenderCommandCount == 64 && texture->iRefCount == 65 && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear(); xrtClearError();
    __xgeRenderCommandUnit();
    assert(texture->iRefCount == 1);

    assert(xgeTextureDrawNinePatch(texture, &style, dst, 0) == XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    for (int i = 0; i < 32; ++i)
        assert(xgeTextureDrawNinePatch(texture, &style, dst, 0) == XGE_OK);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xgeTextureFree(texture);
    assert(xgeUnitChecked() == XGE_ERROR_BUSY);
    __xgeRenderCommandUnit();
    assert(xgeGraphicsOwnedResourceCount() == 0 && xgeUnitChecked() == XGE_OK);
    puts("NINE_PATCH_NATIVE_QUEUE_OK");
    return 0;
}
