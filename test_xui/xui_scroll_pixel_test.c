#include "xui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int checks, failures;
#define CHECK(e) do { ++checks; if (!(e)) { if (failures < 12) { printf("FAIL %d: %s\n", __LINE__, #e); } ++failures; } } while (0)
static xui_rect_t pixels(float x, float y, float w, float h)
{
    int l = (int)floorf(x + .5f), t = (int)floorf(y + .5f);
    return (xui_rect_t){l, t, (int)floorf(x + w + .5f) - l, (int)floorf(y + h + .5f) - t};
}
static int same(xui_rect_t a, xui_rect_t b)
{
    return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}
static xui_surface tracked;
static xui_rect_t srcSeen, dstSeen;
static int copies;
static xui_surface_draw_to_proc copyOriginal;
static int copy_capture(xui_proxy p, xui_surface target, xui_surface source,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    if (source == tracked) { srcSeen = src; dstSeen = dst; ++copies; }
    return copyOriginal(p, target, source, src, dst, color, flags);
}
static void bars(xui_context c, xui_widget parent, int vertical, int odd)
{
    xui_widget bar;
    xui_rect_t track, thumb, dec, inc;
    int length = 150 + odd, cross = 19 + odd, i;
    float values[] = {0, .5f, 43.25f, 199.75f, 200};
    CHECK(xuiScrollBarCreate(c, &bar, NULL) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, bar) == XUI_OK);
    CHECK(xuiScrollBarSetOrientation(bar, vertical ? XUI_ORIENTATION_VERTICAL : XUI_ORIENTATION_HORIZONTAL) == XUI_OK);
    CHECK(xuiScrollBarSetButtonMode(bar, XUI_SCROLLBAR_BUTTONS_ON) == XUI_OK);
    CHECK(xuiScrollBarSetMetrics(bar, 7.75f, 9.5f, 12.5f) == XUI_OK);
    CHECK(xuiScrollBarSetRange(bar, 0, 200, 37.5f) == XUI_OK);
    CHECK(xuiWidgetSetRect(bar, (xui_rect_t){4, 6, vertical ? cross : length, vertical ? length : cross}) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    dec = vertical ? pixels(0, 0, cross, 12.5f) : pixels(0, 0, 12.5f, cross);
    inc = vertical ? pixels(0, length - 12.5f, cross, 12.5f) : pixels(length - 12.5f, 0, 12.5f, cross);
    track = vertical ? pixels(0, 12.5f, cross, length - 25.0f) : pixels(12.5f, 0, length - 25.0f, cross);
    CHECK(same(xuiScrollBarGetDecreaseRect(bar), dec));
    CHECK(same(xuiScrollBarGetIncreaseRect(bar), inc));
    CHECK(same(xuiScrollBarGetTrackRect(bar), track));
    CHECK(vertical ? dec.fY + dec.fH == track.fY && track.fY + track.fH == inc.fY :
        dec.fX + dec.fW == track.fX && track.fX + track.fW == inc.fX);
    for (i = 0; i < 5; ++i) {
        float len = (vertical ? track.fH : track.fW) * 37.5f / 237.5f;
        float pos = (vertical ? track.fY : track.fX) + ((vertical ? track.fH : track.fW) - len) * values[i] / 200;
        thumb = vertical ? pixels((cross - 7.75f) * .5f, pos, 7.75f, len) :
            pixels(pos, (cross - 7.75f) * .5f, len, 7.75f);
        CHECK(xuiScrollBarSetValue(bar, values[i]) == XUI_OK);
        CHECK(xuiScrollBarGetValue(bar) == values[i]);
        CHECK(same(xuiScrollBarGetThumbRect(bar), thumb));
        CHECK(thumb.fX >= track.fX && thumb.fY >= track.fY &&
            thumb.fX + thumb.fW <= track.fX + track.fW && thumb.fY + thumb.fH <= track.fY + track.fH);
    }
    CHECK(xuiScrollBarSetMetrics(bar, 8.5f, 9.5f, 11.75f) == XUI_OK);
    CHECK(!same(xuiScrollBarGetTrackRect(bar), track));
    CHECK(xuiScrollBarGetValue(bar) == 200);
    xuiWidgetDestroy(bar);
}
static void views(int odd, int offset, float reserve)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, view, frame, content, child;
    xui_surface target = NULL;
    xui_scroll_view_desc_t d;
    xui_rect_t viewport, hbar, vbar, corner, world, clipped, r;
    int w = 152 + odd, h = 108 + odd, i;
    const float offsets[] = {.1f, .49f, .5f, .75f, 13.75f, 20.6f};
    float ox, oy;
    xuiTestProxyInit(&p);
    copyOriginal = p.tProxy.surfaceDrawTo;
    p.tProxy.surfaceDrawTo = copy_capture;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 400, 300) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 380, 260}) == XUI_OK);
    bars(c, parent, 0, odd);
    bars(c, parent, 1, odd);
    memset(&d, 0, sizeof(d));
    d.iSize = sizeof(d);
    d.fContentWidth = 500.75f; d.fContentHeight = 400.5f;
    d.iPolicyX = d.iPolicyY = XUI_SCROLLBAR_POLICY_ALWAYS;
    d.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
    d.iCornerMode = XUI_SCROLL_FRAME_CORNER_GRIP;
    d.fScrollbarSize = reserve;
    CHECK(xuiScrollViewCreate(c, &view, &d) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){5, 7, w, h}) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    frame = xuiScrollViewGetFrameWidget(view);
    content = xuiScrollViewGetContentWidget(view);
    viewport = pixels(0, 0, w - reserve, h - reserve);
    hbar = pixels(0, h - reserve, w - reserve, reserve);
    vbar = pixels(w - reserve, 0, reserve, h - reserve);
    corner = pixels(w - reserve, h - reserve, reserve, reserve);
    CHECK(same(xuiScrollViewGetViewportRect(view), viewport));
    CHECK(same(xuiScrollFrameGetHScrollBarRect(frame), hbar));
    CHECK(same(xuiScrollFrameGetVScrollBarRect(frame), vbar));
    CHECK(same(xuiScrollFrameGetCornerRect(frame), corner));
    CHECK(viewport.fW == vbar.fX && viewport.fH == hbar.fY);
    CHECK(hbar.fY + hbar.fH == h && vbar.fX + vbar.fW == w);
    CHECK(same(xuiWidgetGetRect(xuiScrollFrameGetViewportWidget(frame)), viewport));
    CHECK(same(xuiWidgetGetRect(xuiScrollFrameGetCornerWidget(frame)), corner));
    for (i = 0; i < 6; ++i) {
        CHECK(xuiScrollViewSetOffset(view, offsets[i], offsets[5 - i]) == XUI_OK);
        CHECK(xuiScrollViewGetOffset(view, &ox, &oy) == XUI_OK);
        CHECK(ox == offsets[i] && oy == offsets[5 - i]);
        CHECK(same(xuiWidgetGetRect(content), pixels(-ox, -oy, 501, 401)));
        CHECK(xuiLayout(c) == XUI_OK);
        CHECK(same(xuiWidgetGetRect(content), pixels(-ox, -oy, 501, 401)));
    }
    CHECK(xuiWidgetCreate(c, &child) == XUI_OK);
    CHECK(xuiWidgetAddChild(content, child) == XUI_OK);
    CHECK(xuiWidgetSetRect(child, (xui_rect_t){480, 380, 10, 10}) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    world = xuiWidgetGetWorldRect(child);
    CHECK(xuiHitTest(c, world.fX, world.fY, XUI_WIDGET_HIT_DEFAULT) != child);
    CHECK(xuiTestSurfaceCreate(&p, &target, 400, 300, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    tracked = xuiWidgetGetCacheSurface(frame, xuiWidgetGetInputState(frame));
    CHECK(tracked != NULL);
    r = pixels(corner.fX + corner.fW - fminf(corner.fW, corner.fH) / 4.0f * 3 - 2,
        corner.fY + corner.fH - 10, fminf(corner.fW, corner.fH) / 4.0f * 3, 1);
    CHECK(same(xuiTestSurfaceGetLastRect(tracked), r));
    world = xuiWidgetGetWorldRect(frame);
    clipped = world;
    if (clipped.fX < 0) { clipped.fW += clipped.fX; clipped.fX = 0; }
    if (clipped.fY < 0) { clipped.fH += clipped.fY; clipped.fY = 0; }
    copies = 0;
    CHECK(xuiRender(c, target, NULL, 0) == XUI_OK);
    CHECK(copies > 0 && same(dstSeen, clipped));
    CHECK(same(srcSeen, (xui_rect_t){clipped.fX - world.fX, clipped.fY - world.fY, clipped.fW, clipped.fH}));
    CHECK(xuiScrollViewSetScrollbarPolicy(view, XUI_SCROLLBAR_POLICY_HIDDEN, XUI_SCROLLBAR_POLICY_ALWAYS) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(same(xuiScrollViewGetViewportRect(view), pixels(0, 0, w - reserve, h)));
    CHECK(xuiScrollViewGetOffset(view, &ox, &oy) == XUI_OK && ox == offsets[5] && oy == offsets[0]);
    CHECK(xuiScrollViewSetContentSize(view, 20.25f, 30.75f) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(xuiScrollViewGetOffset(view, &ox, &oy) == XUI_OK && ox == 0 && oy == 0);
    tracked = NULL;
    p.tProxy.surfaceDestroy(&p.tProxy, target);
    xuiDestroy(c);
}
static void translation_keeps_layout(int odd, int offset)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_widget root, parent, view, content, a, b, label;
    xui_font font = NULL;
    xui_surface cache;
    xui_scroll_view_desc_t d;
    xui_label_desc_t ld;
    xui_rect_t r, world, viewport;
    char text[168];
    const float offsets[] = {0, .25f, .5f, .75f, 1, 1.25f, 1.5f, 13.75f, 0};
    float ox, oy, cw, ch;
    int i, axis;
    xuiTestProxyInit(&p);
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &p.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 400, 300) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(c, root) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &parent) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK);
    CHECK(xuiWidgetSetLayoutType(parent, XUI_LAYOUT_MANUAL) == XUI_OK);
    CHECK(xuiWidgetSetOverflow(parent, XUI_OVERFLOW_HIDDEN) == XUI_OK);
    CHECK(xuiWidgetSetRect(parent, (xui_rect_t){offset, offset, 380, 260}) == XUI_OK);
    memset(&d, 0, sizeof(d)); d.iSize = sizeof(d);
    d.fContentWidth = d.fContentHeight = 500.5f;
    d.iPolicyX = d.iPolicyY = XUI_SCROLLBAR_POLICY_ALWAYS;
    CHECK(xuiScrollViewCreate(c, &view, &d) == XUI_OK);
    CHECK(xuiWidgetAddChild(parent, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){5, 7, 152 + odd, 108 + odd}) == XUI_OK);
    content = xuiScrollViewGetContentWidget(view);
    CHECK(xuiWidgetSetLayoutType(content, XUI_LAYOUT_FLOW) == XUI_OK);
    CHECK(xuiWidgetSetGap(content, 0) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &a) == XUI_OK);
    CHECK(xuiWidgetCreate(c, &b) == XUI_OK);
    CHECK(xuiWidgetAddChild(content, a) == XUI_OK);
    CHECK(xuiWidgetAddChild(content, b) == XUI_OK);
    CHECK(xuiWidgetSetSizeMode(a, XUI_SIZE_FIXED, XUI_SIZE_FIXED) == XUI_OK);
    CHECK(xuiWidgetSetSizeMode(b, XUI_SIZE_FIXED, XUI_SIZE_FIXED) == XUI_OK);
    CHECK(xuiWidgetSetFlowMode(a, XUI_FLOW_INLINE) == XUI_OK);
    CHECK(xuiWidgetSetFlowMode(b, XUI_FLOW_INLINE) == XUI_OK);
    CHECK(xuiWidgetSetPreferredSize(a, (xui_vec2_t){250, 20}) == XUI_OK);
    CHECK(xuiWidgetSetPreferredSize(b, (xui_vec2_t){251, 20}) == XUI_OK);
    CHECK(p.tProxy.fontLoadMemory(&p.tProxy, &font, "body", 4, 6, XUI_FONT_FORMAT_TTF) == XUI_OK);
    memset(text, 'a', sizeof(text) - 1); text[sizeof(text) - 1] = 0;
    memset(&ld, 0, sizeof(ld)); ld.iSize = sizeof(ld);
    ld.sText = text; ld.pFont = font; ld.iWrapMode = XUI_TEXT_WRAP_CHAR;
    ld.iTextColor = XUI_COLOR_RGBA(0, 0, 0, 255); ld.iTextFlags = XUI_TEXT_ALIGN_TOP;
    CHECK(xuiLabelCreate(c, &label, &ld) == XUI_OK);
    CHECK(xuiWidgetAddChild(content, label) == XUI_OK);
    CHECK(xuiWidgetSetFlowMode(label, XUI_FLOW_BLOCK) == XUI_OK);
    CHECK(xuiWidgetSetSizeMode(label, XUI_SIZE_FILL, XUI_SIZE_FIXED) == XUI_OK);
    CHECK(xuiWidgetSetPreferredSize(label, (xui_vec2_t){0, 20}) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    for (axis = 0; axis < 3; ++axis)
    for (i = 0; i < (int)(sizeof(offsets) / sizeof(offsets[0])); ++i) {
        float x = axis == 1 ? 0 : offsets[i], y = axis == 0 ? 0 : offsets[i];
        CHECK(xuiScrollViewSetOffset(view, x, y) == XUI_OK);
        /* Both the immediate sync and a complete layout must only translate. */
        CHECK(same(xuiWidgetGetRect(content), pixels(-x, -y, 501, 501)));
        CHECK(xuiLayout(c) == XUI_OK);
        CHECK(same(xuiWidgetGetRect(content), pixels(-x, -y, 501, 501)));
        CHECK(xuiScrollViewGetContentSize(view, &cw, &ch) == XUI_OK && cw == 500.5f && ch == 500.5f);
        CHECK(xuiScrollViewGetOffset(view, &ox, &oy) == XUI_OK && ox == x && oy == y);
        CHECK(same(xuiWidgetGetRect(a), (xui_rect_t){0, 0, 250, 20}));
        CHECK(same(xuiWidgetGetRect(b), (xui_rect_t){250, 0, 251, 20}));
        CHECK(same(xuiWidgetGetRect(label), (xui_rect_t){0, 20, 501, 20}));
        CHECK(xuiRenderPrepare(c) == XUI_OK);
        cache = xuiWidgetGetCacheSurface(label, xuiWidgetGetStateId(label));
        CHECK(cache != NULL);
        CHECK(same(xuiTestSurfaceGetLastTextRect(cache), (xui_rect_t){0, 0, 501, 6}));
        r = xuiWidgetGetRect(content);
        world = xuiWidgetGetWorldRect(content);
        viewport = xuiWidgetGetWorldRect(xuiScrollViewGetViewportWidget(view));
        CHECK(world.fX == viewport.fX + r.fX && world.fY == viewport.fY + r.fY);
    }
    /* A real size change still relayouts; small content still fills the viewport. */
    CHECK(xuiScrollViewSetContentSize(view, 501.5f, 502.5f) == XUI_OK);
    CHECK(xuiScrollViewSetOffset(view, .25f, .5f) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    CHECK(same(xuiWidgetGetRect(content), pixels(-.25f, -.5f, 502, 503)));
    CHECK(xuiWidgetGetRect(label).fW == 502);
    CHECK(xuiScrollViewSetContentSize(view, 20.25f, 30.75f) == XUI_OK);
    CHECK(xuiLayout(c) == XUI_OK);
    viewport = xuiScrollViewGetViewportRect(view);
    CHECK(same(xuiWidgetGetRect(content), (xui_rect_t){0, 0, viewport.fW, viewport.fH}));
    CHECK(xuiScrollViewGetContentSize(view, &cw, &ch) == XUI_OK && cw == 20.25f && ch == 30.75f);
    CHECK(xuiScrollViewGetOffset(view, &ox, &oy) == XUI_OK && ox == 0 && oy == 0);
    xuiDestroy(c);
    p.tProxy.fontDestroy(&p.tProxy, font);
}
int main(void)
{
    int odd, offset, metric;
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34) translation_keeps_layout(odd, offset);
    for (odd = 0; odd <= 1; ++odd)
    for (offset = -17; offset <= 17; offset += 34)
    for (metric = 0; metric < 2; ++metric) views(odd, offset, metric ? 16.5f : 16.75f);
    printf("xui_scroll_pixel_test: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
