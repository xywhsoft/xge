#include "src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>
#include "src/xui_dock_panel.c"
#include "src/xui_property_grid.c"

static int checks, failures;
#define CHECK(e) do { ++checks; if (!(e)) { printf("FAIL %d: %s\n", __LINE__, #e); ++failures; } } while (0)

static uint32_t colors[4096];
static int color_count;
static xui_draw_rect_fill_proc fill_original;
static xui_draw_rect_stroke_proc stroke_original;
static xui_draw_surface_proc surface_original;
static xui_draw_text_proc text_original;
static xui_draw_line_proc line_original;
static xui_draw_triangle_fill_proc triangle_original;
static int capture_triangle(xui_proxy p, xui_draw_context d, xui_vec2_t a,
    xui_vec2_t b, xui_vec2_t c, uint32_t color)
{
    if (color_count < 4096) colors[color_count++] = color;
    return triangle_original(p, d, a, b, c, color);
}
static xui_widget_cache_render_proc frame_render_original;
static void* frame_render_user;
static int frame_renders;
static xui_surface content_icon;
static uint32_t content_icon_tint;
static int content_icon_draws, button_renders;
static xui_widget_cache_render_proc button_render_original;
static void* button_render_user;
static int count_button_render(xui_widget w, xui_draw_context d, uint32_t state, void* user)
{
    (void)user;
    ++button_renders;
    return button_render_original(w, d, state, button_render_user);
}
static xui_test_proxy_state_t* active_proxy;
static int count_frame_render(xui_widget w, xui_draw_context d, uint32_t state, void* user)
{
    (void)user;
    ++frame_renders;
    return frame_render_original(w, d, state, frame_render_user);
}
static void record(uint32_t color)
{
    if (color_count < 4096) colors[color_count++] = color;
}
static int capture_surface(xui_proxy p, xui_draw_context d, xui_surface surface,
    xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
    if (surface == content_icon) { content_icon_tint = color; ++content_icon_draws; }
    record(color);
    return surface_original(p, d, surface, src, dst, color, flags);
}
static int capture_text(xui_proxy p, xui_draw_context d, xui_font font,
    const char* text, xui_rect_t r, uint32_t color, uint32_t flags)
{
    record(color);
    return text_original(p, d, font, text, r, color, flags);
}
static int capture_line(xui_proxy p, xui_draw_context d, float x, float y,
    float x1, float y1, float width, uint32_t color)
{
    record(color);
    return line_original(p, d, x, y, x1, y1, width, color);
}
static int capture_fill(xui_proxy p, xui_draw_context draw, xui_rect_t r, uint32_t color)
{
    if (color_count < 4096) colors[color_count++] = color;
    return fill_original(p, draw, r, color);
}
static int capture_stroke(xui_proxy p, xui_draw_context draw, xui_rect_t r, float width, uint32_t color)
{
    record(color);
    return stroke_original(p, draw, r, width, color);
}
static int seen(uint32_t color)
{
    int i;
    for (i = 0; i < color_count; ++i) if (colors[i] == color) return 1;
    return 0;
}
static xui_style_property_t prop(const char* name, uint32_t color)
{
    xui_style_property_t p;
    memset(&p, 0, sizeof(p));
    p.iSize = sizeof(p); p.sName = name;
    p.tValue.iSize = sizeof(p.tValue); p.tValue.iType = XUI_STYLE_VALUE_COLOR;
    p.tValue.iColor = color;
    return p;
}
static xui_style_desc_t style(xui_style_property_t* props, int count)
{
    xui_style_desc_t s;
    memset(&s, 0, sizeof(s)); s.iSize = sizeof(s);
    s.pProperties = props; s.iPropertyCount = count;
    return s;
}
static xui_context setup(xui_test_proxy_state_t* proxy)
{
    xui_context c = NULL;
    xuiTestProxyInit(proxy);
    active_proxy = proxy;
    fill_original = proxy->tProxy.drawRectFill;
    proxy->tProxy.drawRectFill = capture_fill;
    stroke_original = proxy->tProxy.drawRectStroke; proxy->tProxy.drawRectStroke = capture_stroke;
    surface_original = proxy->tProxy.drawSurface; proxy->tProxy.drawSurface = capture_surface;
    text_original = proxy->tProxy.drawText; proxy->tProxy.drawText = capture_text;
    line_original = proxy->tProxy.drawLine; proxy->tProxy.drawLine = capture_line;
    triangle_original = proxy->tProxy.drawTriangleFill; proxy->tProxy.drawTriangleFill = capture_triangle;
    CHECK(xuiCreate(&c) == XUI_OK);
    CHECK(xuiSetProxy(c, &proxy->tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(c, 640, 420) == XUI_OK);
    return c;
}
static void paint(xui_context c)
{
    xui_surface target = NULL;
    color_count = 0;
    frame_renders = 0;
    button_renders = 0;
    content_icon_draws = 0;
    CHECK(xuiTestSurfaceCreate(active_proxy, &target, 640, 420, 0) == XUI_OK);
    CHECK(xuiRenderPrepare(c) == XUI_OK);
    CHECK(xuiWidgetRenderTree(xuiGetRootWidget(c), target) == XUI_OK);
    CHECK(frame_renders <= 1);
    active_proxy->tProxy.surfaceDestroy(&active_proxy->tProxy, target);
}
static void paint_only(xui_widget widget)
{
    CHECK((xuiWidgetGetDirtyFlags(widget) & (XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_TREE)) == 0);
}
static void scroll_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget view, frame, bar;
    xui_scroll_view_desc_t desc;
    xui_style_property_t p[3];
    xui_style_desc_t s;
    xui_rect_t before;
    uint32_t layout_version, frame_layout_version;
    uint32_t base_bg = 0x123456ff, base_corner = 0x234567ff, base_grip = 0x345678ff;
    memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc);
    desc.fContentWidth = 1200; desc.fContentHeight = 900;
    desc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL; desc.fScrollbarSize = 16;
    desc.iCornerMode = XUI_SCROLL_FRAME_CORNER_GRIP;
    CHECK(xuiScrollViewCreate(c, &view, &desc) == XUI_OK);
    CHECK(xuiSetRootWidget(c, view) == XUI_OK);
    frame = xuiScrollViewGetFrameWidget(view);
    CHECK(xuiWidgetGetCacheRenderCallback(frame, &frame_render_original, &frame_render_user) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(frame, count_frame_render, NULL) == XUI_OK);
    bar = xuiScrollFrameGetVScrollBarWidget(frame);
    CHECK(xuiScrollViewSetBackgroundColor(view, base_bg) == XUI_OK);
    CHECK(xuiScrollViewSetCornerColors(view, base_corner, base_grip) == XUI_OK);
    paint(c);
    CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    before = xuiScrollFrameGetViewportRect(frame);
    layout_version = view->iLayoutVersion;
    frame_layout_version = frame->iLayoutVersion;
    paint(c); CHECK(frame_renders == 0);

    p[0] = prop("scrollframe.background.color", 0x445566ff);
    p[1] = prop("scrollframe.corner.color", 0x556677ff);
    p[2] = prop("scrollframe.grip.color", 0x667788ff);
    CHECK(xuiStyleSetDefault(c, p, 3) == XUI_OK);
    paint_only(frame); paint(c);
    CHECK(seen(p[0].tValue.iColor) && seen(p[1].tValue.iColor) && seen(p[2].tValue.iColor));
    CHECK(xuiScrollViewGetBackgroundColor(view) == base_bg);

    p[0] = prop("scrollview.background.color", 0x778899ff);
    p[1] = prop("scrollview.corner.color", 0x8899aaff);
    p[2] = prop("scrollview.grip.color", 0x99aabbff);
    s = style(p, 3);
    CHECK(xuiStyleSetType(c, xuiScrollViewGetType(c), &s) == XUI_OK);
    paint_only(view); paint(c);
    CHECK(seen(p[0].tValue.iColor) && seen(p[1].tValue.iColor) && seen(p[2].tValue.iColor));
    p[0].tValue.iType = XUI_STYLE_VALUE_TOKEN; p[0].tValue.sText = "containers.bg";
    {
        xui_style_value_t token = prop("unused", 0xabcdefff).tValue;
        CHECK(xuiStyleSetToken(c, "containers.bg", &token) == XUI_OK);
        CHECK(xuiStyleSetClass(c, "containers", &s) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(view, "containers") == XUI_OK);
        paint(c); CHECK(seen(token.iColor));
        token.iColor = 0x987654ff;
        CHECK(xuiStyleSetToken(c, "containers.bg", &token) == XUI_OK);
        paint_only(view); paint(c); CHECK(seen(token.iColor));
    }
    p[0] = prop("scrollview.background.color", 0);
    p[1] = prop("scrollview.corner.color", 0);
    p[2] = prop("scrollview.grip.color", 0);
    CHECK(xuiWidgetSetInlineStyle(view, p, 3) == XUI_OK);
    paint_only(view); paint(c);
    CHECK(!seen(base_bg) && !seen(base_corner) && !seen(base_grip));
    CHECK(!seen(0x987654ff) && !seen(0x8899aaff) && !seen(0x99aabbff));
    CHECK(xuiScrollFrameGetBackgroundColor(frame) == base_bg);
    CHECK(frame_renders == 1);
    CHECK(xuiWidgetSetInlineStyle(view, NULL, 0) == XUI_OK);
    xuiWidgetClearStyleClasses(view);
    CHECK(xuiStyleRemoveType(c, xuiScrollViewGetType(c)) == XUI_OK);
    CHECK(xuiStyleClearDefault(c) == XUI_OK);
    paint(c); CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    {
        xui_rect_t after = xuiScrollFrameGetViewportRect(frame);
        CHECK(memcmp(&before, &after, sizeof(before)) == 0);
    }
    CHECK(view->iLayoutVersion == layout_version);
    CHECK(frame->iLayoutVersion == frame_layout_version);

    /* The actual nested ScrollBar owns all thumb/track/button styling. */
    p[0] = prop("scrollbar.track.color", 0xa1b2c3ff);
    CHECK(xuiWidgetSetInlineStyle(bar, p, 1) == XUI_OK);
    paint_only(bar); paint(c); CHECK(seen(p[0].tValue.iColor));
    p[0].tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(bar, p, 1) == XUI_OK);
    paint(c); CHECK(!seen(0xa1b2c3ff));
    CHECK(xuiWidgetSetInlineStyle(bar, NULL, 0) == XUI_OK);
    paint(c); CHECK(seen(0xdee8f3ff));
    p[0] = prop("scrollframe.background.color", 0);
    p[1] = prop("scrollframe.corner.color", 0);
    p[2] = prop("scrollframe.grip.color", 0);
    CHECK(xuiWidgetSetInlineStyle(frame, p, 3) == XUI_OK);
    paint_only(frame); paint(c);
    CHECK(!seen(base_bg) && !seen(base_corner) && !seen(base_grip));
    base_bg = 0x456789ff;
    CHECK(xuiScrollFrameSetBackgroundColor(frame, base_bg) == XUI_OK);
    paint(c); CHECK(!seen(base_bg));
    CHECK(xuiWidgetSetInlineStyle(frame, NULL, 0) == XUI_OK);
    paint(c); CHECK(seen(base_bg) && seen(base_corner) && seen(base_grip));
    paint(c); CHECK(frame_renders == 0);
    xuiDestroy(c);
}
static void dock_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget dock;
    xui_font font = NULL;
    xui_dock_panel_data_t* data;
    xui_dock_panel_colors_t base, current;
    xui_dock_window_info_t info;
    xui_style_property_t props[40];
    xui_style_desc_t s;
    xui_rect_t before;
    uint32_t layout_version, host_layout_version;
    int doc, side, floating, pane, i;
    size_t count = sizeof(__xuiDockColorProperties) / sizeof(__xuiDockColorProperties[0]);
    CHECK(xuiDockPanelCreate(c, &dock, NULL) == XUI_OK);
    CHECK(xuiSetRootWidget(c, dock) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, NULL, 14, 0) == XUI_OK);
    CHECK(xuiDockPanelSetFont(dock, font) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Document", NULL, &doc) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, doc, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pane) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Side", NULL, &side) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, side, XUI_DOCK_PANEL_REGION_LEFT, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pane) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Floating", NULL, &floating) == XUI_OK);
    CHECK(xuiDockPanelFloatWindow(dock, floating, (xui_rect_t){240, 60, 250, 200}) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &content_icon, 16, 16, 0xc3a7ddff) == XUI_OK);
    CHECK(xuiDockPanelSetWindowIcon(dock, floating, content_icon, (xui_rect_t){0, 0, 16, 16}) == XUI_OK);
    paint(c);
    data = __xuiDockPanelGetData(dock);
    CHECK(xuiDockPanelGetColors(dock, &base) == XUI_OK);
    memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
    CHECK(xuiDockPanelGetWindowInfo(dock, floating, &info) == XUI_OK);
    before = xuiWidgetGetRect(info.pHostWidget);
    layout_version = dock->iLayoutVersion; host_layout_version = info.pHostWidget->iLayoutVersion;
    CHECK(xuiWidgetGetCacheRenderCallback(info.pHostWidget, &frame_render_original, &frame_render_user) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(info.pHostWidget, count_frame_render, NULL) == XUI_OK);
    CHECK(xuiWidgetGetCacheRenderCallback(info.pCloseButtonWidget, &button_render_original, &button_render_user) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(info.pCloseButtonWidget, count_button_render, NULL) == XUI_OK);
    {
        xui_cache_policy_t policy = xuiWidgetGetCachePolicy(info.pCloseButtonWidget);
        policy.iFlags |= XUI_CACHE_UPDATE_ALL_STATES;
        CHECK(xuiWidgetSetCachePolicy(info.pCloseButtonWidget, &policy) == XUI_OK);
        CHECK(xuiWidgetSetCacheStateCount(info.pCloseButtonWidget, 3) == XUI_OK);
        CHECK(xuiWidgetSetCacheStateId(info.pCloseButtonWidget, 0, 0) == XUI_OK);
        CHECK(xuiWidgetSetCacheStateId(info.pCloseButtonWidget, 1, XUI_WIDGET_STATE_HOVER) == XUI_OK);
        CHECK(xuiWidgetSetCacheStateId(info.pCloseButtonWidget, 2, XUI_WIDGET_STATE_ACTIVE) == XUI_OK);
    }
    paint(c); CHECK(frame_renders == 1 && button_renders == 3);
    paint(c); CHECK(frame_renders == 0 && button_renders == 0);
    for (i = 0; i < (int)count; ++i) props[i] = prop(__xuiDockColorProperties[i].sName, 0x100020ffu + (uint32_t)i * 0x00050100u);
    props[count] = prop("dockpanel.button.close_icon_color", 0x765432ff);
    s = style(props, (int)count + 1);
    CHECK(xuiStyleSetType(c, xuiDockPanelGetType(c), &s) == XUI_OK);
    paint_only(dock); paint(c);
    for (i = 0; i < (int)count; ++i) {
        const uint32_t* color = (const uint32_t*)((const char*)__xuiDockColors(data) + __xuiDockColorProperties[i].iOffset);
        CHECK(*color == props[i].tValue.iColor);
    }
    CHECK(seen(props[0].tValue.iColor));
    CHECK(seen(props[17].tValue.iColor));
    CHECK(seen(props[22].tValue.iColor));
    CHECK(seen(props[count].tValue.iColor));
    CHECK(seen(props[18].tValue.iColor) && seen(props[19].tValue.iColor));
    CHECK(frame_renders == 1 && button_renders == 3);
    CHECK(content_icon_draws == 1 && content_icon_tint == XUI_COLOR_WHITE);
    CHECK(xuiDockPanelGetColors(dock, &current) == XUI_OK);
    CHECK(memcmp(&base, &current, sizeof(base)) == 0);
    {
        xui_style_value_t token = prop("unused", 0x556699ff).tValue;
        props[22].tValue.iType = XUI_STYLE_VALUE_TOKEN; props[22].tValue.sText = "dock.title";
        CHECK(xuiStyleSetToken(c, "dock.title", &token) == XUI_OK);
        CHECK(xuiStyleSetClass(c, "dock-live", &s) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(dock, "dock-live") == XUI_OK);
        paint(c); CHECK(seen(token.iColor));
        token.iColor = 0x996655ff;
        CHECK(xuiStyleSetToken(c, "dock.title", &token) == XUI_OK);
        paint(c); CHECK(seen(token.iColor));
    }
    for (i = 0; i < (int)count + 1; ++i) props[i] = prop(props[i].sName, 0);
    CHECK(xuiWidgetSetInlineStyle(dock, props, (int)count + 1) == XUI_OK);
    paint_only(dock); paint(c);
    CHECK(!seen(0x996655ff) && !seen(base.iFloatTitleColor) && !seen(0x765432ff));
    CHECK(frame_renders == 1 && button_renders == 3);
    CHECK(content_icon_draws == 1 && content_icon_tint == XUI_COLOR_WHITE);
    for (i = 0; i < (int)count; ++i) {
        const uint32_t* color = (const uint32_t*)((const char*)__xuiDockColors(data) + __xuiDockColorProperties[i].iOffset);
        CHECK(*color == 0);
    }
    base.iFloatTitleColor = 0x386248ff;
    CHECK(xuiDockPanelSetColors(dock, &base) == XUI_OK);
    paint(c); CHECK(!seen(base.iFloatTitleColor));
    CHECK(xuiWidgetSetInlineStyle(dock, NULL, 0) == XUI_OK);
    xuiWidgetClearStyleClasses(dock);
    CHECK(xuiStyleRemoveType(c, xuiDockPanelGetType(c)) == XUI_OK);
    paint(c); CHECK(seen(base.iFloatTitleColor) && seen(base.iButtonColor));
    CHECK(seen(0xab484cff));
    {
        xui_rect_t after = xuiWidgetGetRect(info.pHostWidget);
        CHECK(memcmp(&before, &after, sizeof(before)) == 0);
    }
    CHECK(dock->iLayoutVersion == layout_version && info.pHostWidget->iLayoutVersion == host_layout_version);
    CHECK(xuiDockPanelAutoHideWindow(dock, side) == XUI_OK);
    CHECK(xuiDockPanelExpandAutoHideWindow(dock, side) == XUI_OK);
    paint(c);
    props[0] = prop("dockpanel.caption.active_color", 0x976543ff);
    CHECK(xuiWidgetSetInlineStyle(dock, props, 1) == XUI_OK);
    paint_only(dock); paint(c); CHECK(seen(props[0].tValue.iColor));
    CHECK(xuiWidgetSetInlineStyle(dock, NULL, 0) == XUI_OK);
    paint(c); CHECK(seen(base.iActiveCaptionColor));
    props[0] = prop("dockpanel.button.icon_color", 0xa17352ff);
    props[1] = prop("dockpanel.auto_hide.border_color", 0x375ae1ff);
    CHECK(xuiStyleSetDefault(c, props, 2) == XUI_OK);
    paint_only(dock); paint(c);
    CHECK(seen(props[0].tValue.iColor) && seen(props[1].tValue.iColor));
    CHECK(xuiStyleClearDefault(c) == XUI_OK);
    paint(c); CHECK(!seen(props[0].tValue.iColor) && !seen(props[1].tValue.iColor));
    paint(c); CHECK(frame_renders == 0 && button_renders == 0);
    xuiDestroy(c);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, content_icon); content_icon = NULL;
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}

static void dock_drag_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget dock;
    xui_style_property_t props[4];
    xui_dock_panel_data_t* data;
    xui_dock_panel_colors_t base;
    xui_rect_t split;
    uint32_t layout_version;
    int doc, side, pane, x, y;
    CHECK(xuiDockPanelCreate(c, &dock, NULL) == XUI_OK);
    CHECK(xuiSetRootWidget(c, dock) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Document", NULL, &doc) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, doc, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pane) == XUI_OK);
    CHECK(xuiDockPanelAddWindow(dock, "Side", NULL, &side) == XUI_OK);
    CHECK(xuiDockPanelDockWindow(dock, side, XUI_DOCK_PANEL_REGION_LEFT, XUI_DOCK_PANEL_SIDE_FILL, .5f, &pane) == XUI_OK);
    paint(c);
    data = __xuiDockPanelGetData(dock);
    CHECK(xuiDockPanelGetColors(dock, &base) == XUI_OK);
    split = data->arrRegions[XUI_DOCK_PANEL_REGION_LEFT].tSplitterRect;
    x = (int)(split.fX + split.fW / 2); y = (int)(split.fY + split.fH / 2);
    CHECK(xuiInputPointerDown(c, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiGetPointerCapture(c) == dock);
    CHECK(xuiInputPointerMove(c, x + 10, y, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    paint(c);
    CHECK(c->pDragAdornerOwner == dock && c->iDragAdornerPrimitiveCount == 1);
    layout_version = dock->iLayoutVersion;
    props[0] = prop("dockpanel.splitter.active_color", 0x864321ff);
    CHECK(xuiWidgetSetInlineStyle(dock, props, 1) == XUI_OK);
    paint_only(dock); paint(c);
    CHECK(c->arrDragAdornerPrimitives[0].iColor == props[0].tValue.iColor && seen(props[0].tValue.iColor));
    props[0].tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(dock, props, 1) == XUI_OK);
    paint(c); CHECK(c->arrDragAdornerPrimitives[0].iColor == 0 && !seen(0x864321ff));
    CHECK(xuiWidgetSetInlineStyle(dock, NULL, 0) == XUI_OK);
    paint(c); CHECK(c->arrDragAdornerPrimitives[0].iColor == base.iSplitterActiveColor && seen(base.iSplitterActiveColor));
    CHECK(dock->iLayoutVersion == layout_version);
    CHECK(xuiInputPointerUp(c, x + 10, y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);

    /* Exercise the detached preview cache using an actual title drag. */
    CHECK(xuiDockPanelFloatWindow(dock, side, (xui_rect_t){340, 80, 240, 180}) == XUI_OK);
    paint(c);
    CHECK(xuiInputPointerDown(c, 388, 94, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    CHECK(xuiInputPointerMove(c, 632, 210, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    paint(c);
    CHECK(data->tDragPreview.bValid && data->pDragOverlayWidget != NULL);
    props[0] = prop("dockpanel.drag.preview_color", 0x413276ab);
    props[1] = prop("dockpanel.drag.preview_border_color", 0x651493cd);
    props[2] = prop("dockpanel.drag.preview_inner_border_color", 0x159473ef);
    props[3] = prop("dockpanel.drag.indicator_color", 0x573412ff);
    CHECK(xuiWidgetSetInlineStyle(dock, props, 4) == XUI_OK);
    paint_only(dock); paint(c);
    CHECK(seen(props[0].tValue.iColor) && seen(props[1].tValue.iColor) && seen(props[2].tValue.iColor));
    CHECK(seen(props[3].tValue.iColor));
    for (x = 0; x < 4; ++x) props[x].tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(dock, props, 4) == XUI_OK);
    paint(c); CHECK(!seen(0x413276ab) && !seen(0x651493cd) && !seen(0x159473ef) && !seen(0x573412ff));
    CHECK(xuiWidgetSetInlineStyle(dock, NULL, 0) == XUI_OK);
    paint(c);
    CHECK(seen(XUI_COLOR_RGBA(47, 125, 214, 54)));
    CHECK(xuiInputPointerUp(c, 632, 210, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
    xuiDestroy(c);
}

static int grid_cell(xui_widget widget, int row, int column, xui_table_view_cell_t* cell, void* user)
{
    (void)widget; (void)column; (void)user;
    cell->sText = row == 0 ? "Content" : "One";
    cell->iType = row == 0 ? XUI_TABLE_CELL_TYPE_TEXT : XUI_TABLE_CELL_TYPE_ENUM;
    cell->bHasStyle = row == 0; cell->iBackgroundColor = 0xdabaccff;
    cell->iTextColor = 0x3e5164ff; cell->iGridColor = 0x6789abff;
    return XUI_OK;
}
static int grid_editor_config(xui_widget widget, int row, int column, int type,
    xui_table_grid_editor_config_t* config, void* user)
{
    static const char* items[] = {"One", "Two"};
    (void)widget; (void)row; (void)column; (void)user;
    if (type == XUI_TABLE_CELL_TYPE_ENUM) { config->arrEnumItems = items; config->iEnumItemCount = 2; }
    return 1;
}
static void table_grid_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget grid, table, input, viewport, combo;
    xui_font font = NULL;
    xui_table_grid_desc_t desc;
    xui_table_view_column_t column;
    xui_table_view_row_t rows[2];
    xui_table_view_colors_t palette, current;
    xui_style_property_t p;
    xui_style_desc_t s;
    uint32_t background, text, border, focus;
    uint32_t layout_version, table_layout_version;
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, NULL, 14, 0) == XUI_OK);
    memset(&desc, 0, sizeof(desc)); memset(&column, 0, sizeof(column)); memset(rows, 0, sizeof(rows));
    desc.iSize = sizeof(desc); desc.pFont = font;
    column.sTitle = "Column"; column.fWidth = 320; rows[0].fHeight = rows[1].fHeight = 32;
    desc.arrColumns = &column; desc.iColumnCount = 1; desc.arrRows = rows; desc.iRowCount = 2;
    desc.onCell = grid_cell;
    desc.onEditorConfig = grid_editor_config;
    CHECK(xuiTableGridCreate(c, &grid, &desc) == XUI_OK);
    CHECK(xuiSetRootWidget(c, grid) == XUI_OK);
    table = xuiTableGridGetTableView(grid);
    CHECK(xuiTableViewGetColors(table, &palette) == XUI_OK);
    palette.iRowColor = 0x223344ff; palette.iGridColor = 0x446688ff;
    palette.iTextColor = 0xddeeffff; palette.iFocusRingColor = 0x8899bbff;
    CHECK(xuiTableGridSetColorStyle(grid, &palette) == XUI_OK);
    paint(c); CHECK(seen(0xdabaccff));
    viewport = xuiTableViewGetViewportWidget(table);
    CHECK(xuiWidgetGetCacheRenderCallback(viewport, &frame_render_original, &frame_render_user) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(viewport, count_frame_render, NULL) == XUI_OK);
    paint(c); CHECK(frame_renders == 1);
    paint(c); CHECK(frame_renders == 0);
    layout_version = grid->iLayoutVersion; table_layout_version = table->iLayoutVersion;
    p = prop("tableview.background.color", 0x972351ff);
    CHECK(xuiStyleSetDefault(c, &p, 1) == XUI_OK);
    paint_only(grid); paint_only(table); paint(c); CHECK(seen(p.tValue.iColor));
    p = prop("tableview.row.alt_color", 0x159357ff); s = style(&p, 1);
    CHECK(xuiStyleSetType(c, xuiTableViewGetType(c), &s) == XUI_OK);
    paint_only(table); paint(c);
    CHECK(frame_renders == 1 && seen(p.tValue.iColor) && seen(0xdabaccff));
    p.tValue.iColor = 0x753951ff;
    CHECK(xuiStyleSetType(c, xuiTableViewGetType(c), &s) == XUI_OK);
    paint(c); CHECK(frame_renders == 1 && seen(p.tValue.iColor));
    {
        xui_style_value_t token = prop("unused", 0x459713ff).tValue;
        p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "grid.row";
        CHECK(xuiStyleSetToken(c, "grid.row", &token) == XUI_OK);
        CHECK(xuiStyleSetClass(c, "grid-live", &s) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(table, "grid-live") == XUI_OK);
        paint_only(table); paint(c); CHECK(seen(token.iColor) && frame_renders == 1);
        token.iColor = 0x731945ff;
        CHECK(xuiStyleSetToken(c, "grid.row", &token) == XUI_OK);
        paint(c); CHECK(seen(token.iColor) && frame_renders == 1);
    }
    p = prop("tableview.row.alt_color", 0);
    CHECK(xuiWidgetSetInlineStyle(table, &p, 1) == XUI_OK);
    paint_only(table); paint(c); CHECK(!seen(0x731945ff) && !seen(palette.iAltRowColor) && seen(0xdabaccff));
    CHECK(xuiTableViewGetColors(table, &current) == XUI_OK && memcmp(&current, &palette, sizeof(palette)) == 0);
    CHECK(xuiWidgetSetInlineStyle(table, NULL, 0) == XUI_OK);
    xuiWidgetClearStyleClasses(table);
    CHECK(xuiStyleRemoveType(c, xuiTableViewGetType(c)) == XUI_OK);
    CHECK(xuiStyleClearDefault(c) == XUI_OK);
    paint(c); CHECK(seen(palette.iAltRowColor) && seen(palette.iBackgroundColor) && seen(0xdabaccff));
    CHECK(grid->iLayoutVersion == layout_version && table->iLayoutVersion == table_layout_version);
    paint(c); CHECK(frame_renders == 0);
    CHECK(xuiTableGridBeginEdit(grid, 0, 0) != 0);
    input = xuiGetFocusWidget(c);
    CHECK(xuiWidgetIsType(input, xuiInputGetType(c)));
    CHECK(xuiInputGetColors(input, &background, &text, &border, &focus) == XUI_OK);
    CHECK(background == palette.iRowColor && text == palette.iTextColor);
    CHECK(border == palette.iGridColor && focus == palette.iFocusRingColor);
    paint(c); CHECK(seen(background));
    p = prop("input.background.color", 0x992255ff); s = style(&p, 1);
    CHECK(xuiStyleSetType(c, xuiInputGetType(c), &s) == XUI_OK);
    paint_only(input); paint(c); CHECK(seen(p.tValue.iColor));
    p.tValue.iColor = 0x559922ff;
    CHECK(xuiStyleSetType(c, xuiInputGetType(c), &s) == XUI_OK);
    paint(c); CHECK(seen(p.tValue.iColor));
    CHECK(xuiInputGetColors(input, &background, NULL, NULL, NULL) == XUI_OK);
    CHECK(background == palette.iRowColor);
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(input, &p, 1) == XUI_OK);
    paint_only(input); paint(c); CHECK(!seen(0x559922ff) && !seen(background));
    palette.iRowColor = 0x946235ff;
    CHECK(xuiTableGridSetColorStyle(grid, &palette) == XUI_OK);
    paint(c); CHECK(!seen(palette.iRowColor));
    CHECK(xuiInputGetColors(input, &background, NULL, NULL, NULL) == XUI_OK && background == palette.iRowColor);
    CHECK(xuiWidgetSetInlineStyle(input, NULL, 0) == XUI_OK);
    CHECK(xuiStyleRemoveType(c, xuiInputGetType(c)) == XUI_OK);
    paint(c); CHECK(seen(background));
    CHECK(xuiTableGridEndEdit(grid, 0) != 0);
    paint(c); CHECK(seen(0xdabaccff));
    CHECK(xuiTableGridBeginEdit(grid, 1, 0) != 0);
    for (combo = xuiWidgetGetFirstChild(grid); combo != NULL; combo = xuiWidgetGetNextSibling(combo))
        if (xuiWidgetIsType(combo, xuiComboBoxGetType(c))) break;
    CHECK(combo != NULL && xuiWidgetGetVisible(combo));
    CHECK(xuiComboBoxGetColors(combo, &text, NULL, &background, NULL, &focus, NULL) == XUI_OK);
    CHECK(background == palette.iRowColor && text == palette.iTextColor && focus == palette.iHoverColor);
    paint(c);
    CHECK(xuiComboBoxOpen(combo) == XUI_OK);
    paint(c); CHECK(xuiComboBoxIsOpen(combo));
    layout_version = grid->iLayoutVersion; table_layout_version = table->iLayoutVersion;
    p = prop("combobox.background.open_color", 0x193754ff); s = style(&p, 1);
    CHECK(xuiStyleSetType(c, xuiComboBoxGetType(c), &s) == XUI_OK);
    paint_only(combo); paint(c); CHECK(seen(p.tValue.iColor) && xuiComboBoxIsOpen(combo));
    p.tValue.iColor = 0x573419ff;
    CHECK(xuiWidgetSetInlineStyle(combo, &p, 1) == XUI_OK);
    paint(c); CHECK(seen(p.tValue.iColor));
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(combo, &p, 1) == XUI_OK);
    paint(c); CHECK(!seen(0x193754ff) && !seen(0x573419ff));
    CHECK(xuiComboBoxGetColors(combo, NULL, NULL, &background, NULL, &focus, NULL) == XUI_OK);
    CHECK(background == palette.iRowColor && focus == palette.iHoverColor);
    CHECK(xuiWidgetSetInlineStyle(combo, NULL, 0) == XUI_OK);
    CHECK(xuiStyleRemoveType(c, xuiComboBoxGetType(c)) == XUI_OK);
    paint(c); CHECK(seen(focus) && xuiComboBoxIsOpen(combo));
    CHECK(grid->iLayoutVersion == layout_version && table->iLayoutVersion == table_layout_version);
    CHECK(xuiTableGridEndEdit(grid, 0) != 0);
    xuiDestroy(c);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}
static void property_styles(void)
{
    xui_test_proxy_state_t proxy;
    xui_context c = setup(&proxy);
    xui_widget grid, table, viewport;
    xui_font font = NULL;
    xui_property_desc_t item;
    xui_property_grid_data_t* data;
    xui_property_grid_style_t base, current;
    xui_table_view_colors_t table_base, table_current;
    xui_style_property_t props[15], p;
    xui_style_desc_t s;
    xui_table_grid_editor_config_t config;
    const char* options[] = {"One", "Two"};
    xui_rect_t before;
    uint32_t owner_layout, table_layout, viewport_layout;
    int category, property, choice, i, painted_keys[15] = {0};
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, NULL, 14, 0) == XUI_OK);
    CHECK(xuiPropertyGridCreate(c, &grid, NULL) == XUI_OK);
    CHECK(xuiSetRootWidget(c, grid) == XUI_OK);
    CHECK(xuiPropertyGridSetFont(grid, font) == XUI_OK);
    category = xuiPropertyGridAddCategory(grid, "category", "Category", 1);
    CHECK(category >= 0);
    memset(&item, 0, sizeof(item));
    item.sId = "name"; item.sName = "Name"; item.sValue = "Value";
    item.iType = XUI_TABLE_CELL_TYPE_TEXT;
    item.iFlags = XUI_PROPERTY_FLAG_INVALID | XUI_PROPERTY_FLAG_DIRTY;
    property = xuiPropertyGridAddProperty(grid, category, &item);
    CHECK(property >= 0);
    item.sId = "choice"; item.sName = "Choice"; item.sValue = "One";
    item.iType = XUI_TABLE_CELL_TYPE_ENUM;
    choice = xuiPropertyGridAddProperty(grid, category, &item);
    CHECK(choice >= 0);
    memset(&config, 0, sizeof(config));
    config.arrEnumItems = options; config.iEnumItemCount = 2;
    CHECK(xuiPropertyGridSetEditorConfig(grid, choice, &config) == XUI_OK);
    item.sId = "readonly"; item.sName = "Read only"; item.sValue = "Fixed";
    item.iType = XUI_TABLE_CELL_TYPE_TEXT; item.iFlags = XUI_PROPERTY_FLAG_READONLY;
    CHECK(xuiPropertyGridAddProperty(grid, category, &item) >= 0);
    item.sId = "color"; item.sName = "Content color"; item.sValue = "#C8AB43FF";
    item.iType = XUI_TABLE_CELL_TYPE_COLOR; item.iFlags = 0;
    CHECK(xuiPropertyGridAddProperty(grid, category, &item) >= 0);
    paint(c);
    table = xuiPropertyGridGetTableView(grid);
    viewport = xuiTableViewGetViewportWidget(table);
    data = __xuiPropertyGridGetData(grid);
    CHECK(xuiPropertyGridGetStyle(grid, &base) == XUI_OK);
    CHECK(xuiTableViewGetColors(table, &table_base) == XUI_OK);
    CHECK(xuiWidgetGetCacheRenderCallback(viewport, &frame_render_original, &frame_render_user) == XUI_OK);
    CHECK(xuiWidgetSetCacheRenderCallback(viewport, count_frame_render, NULL) == XUI_OK);
    paint(c); CHECK(frame_renders == 1);
    paint(c); CHECK(frame_renders == 0);
    before = xuiWidgetGetRect(viewport);
    owner_layout = grid->iLayoutVersion; table_layout = table->iLayoutVersion; viewport_layout = viewport->iLayoutVersion;

    for (i = 0; i < 15; ++i) props[i] = prop(__xuiPropertyGridColorProperties[i].sName, 0x701040ffu + (uint32_t)i * 0x00050200u);
    CHECK(xuiStyleSetDefault(c, props, 15) == XUI_OK);
    paint_only(grid); paint_only(table); paint(c);
    CHECK(seen(props[2].tValue.iColor) && seen(props[4].tValue.iColor) && frame_renders == 1);
    for (i = 0; i < 15; ++i) props[i].tValue.iColor += 0x10000000u;
    s = style(props, 15);
    CHECK(xuiStyleSetType(c, xuiPropertyGridGetType(c), &s) == XUI_OK);
    paint_only(grid); paint(c);
    for (i = 0; i < 15; ++i) {
        const uint32_t* color = (const uint32_t*)((const char*)__xuiPropertyGridColors(data) + __xuiPropertyGridColorProperties[i].iOffset);
        CHECK(*color == props[i].tValue.iColor);
        painted_keys[i] |= seen(props[i].tValue.iColor);
    }
    CHECK(seen(props[0].tValue.iColor) && seen(props[1].tValue.iColor));
    CHECK(seen(props[2].tValue.iColor) && seen(props[4].tValue.iColor));
    CHECK(seen(props[5].tValue.iColor) && seen(props[6].tValue.iColor));
    CHECK(seen(props[7].tValue.iColor) && seen(props[9].tValue.iColor));
    CHECK(seen(props[10].tValue.iColor) && seen(props[12].tValue.iColor));
    CHECK(seen(props[13].tValue.iColor) && seen(props[14].tValue.iColor));
    CHECK(seen(0xc8ab43ff));
    CHECK(!seen(XUI_COLOR_RGBA(218, 82, 82, 255)) && !seen(XUI_COLOR_RGBA(245, 158, 11, 255)));
    CHECK(xuiPropertyGridGetStyle(grid, &current) == XUI_OK && memcmp(&base, &current, sizeof(base)) == 0);
    paint(c); CHECK(frame_renders == 0);
    {
        xui_style_value_t token = prop("unused", 0x632497ff).tValue;
        p = prop("propertygrid.category.text_color", 0);
        p.tValue.iType = XUI_STYLE_VALUE_TOKEN; p.tValue.sText = "property.category";
        s = style(&p, 1);
        CHECK(xuiStyleSetToken(c, "property.category", &token) == XUI_OK);
        CHECK(xuiStyleSetClass(c, "property-live", &s) == XUI_OK);
        CHECK(xuiWidgetAddStyleClass(grid, "property-live") == XUI_OK);
        paint_only(grid); paint(c); CHECK(seen(token.iColor) && frame_renders == 1);
        token.iColor = 0x976243ff;
        CHECK(xuiStyleSetToken(c, "property.category", &token) == XUI_OK);
        paint(c); CHECK(seen(token.iColor) && frame_renders == 1);
        paint(c); CHECK(frame_renders == 0);
    }
    CHECK(xuiPropertyGridSetSelected(grid, property) == XUI_OK);
    paint(c); CHECK(seen(props[11].tValue.iColor));
    painted_keys[11] |= seen(props[11].tValue.iColor);
    p = prop("propertygrid.selected.color", 0x193765ff);
    CHECK(xuiWidgetSetInlineStyle(grid, &p, 1) == XUI_OK);
    paint_only(grid); paint(c); CHECK(seen(p.tValue.iColor) && !seen(props[11].tValue.iColor));
    p.tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(grid, &p, 1) == XUI_OK);
    paint(c); CHECK(!seen(0x193765ff) && !seen(props[11].tValue.iColor) && !seen(base.iSelectedColor));
    paint(c); CHECK(frame_renders == 0);
    p.tValue.iColor = 0x193765ff;
    CHECK(xuiWidgetSetInlineStyle(grid, &p, 1) == XUI_OK);
    paint(c); CHECK(seen(p.tValue.iColor));
    {
        xui_style_property_t table_props[3];
        table_props[0] = prop("tableview.row.selected_color", 0xa16543ff);
        table_props[1] = prop("tableview.cell.invalid_color", 0x43a165ff);
        table_props[2] = prop("tableview.cell.dirty_color", 0x6543a1ff);
        CHECK(xuiWidgetSetInlineStyle(table, table_props, 3) == XUI_OK);
        paint_only(table); paint(c);
        for (i = 0; i < 3; ++i) CHECK(seen(table_props[i].tValue.iColor));
        CHECK(!seen(p.tValue.iColor) && !seen(props[13].tValue.iColor) && !seen(props[14].tValue.iColor));
        CHECK(xuiTableViewGetColors(table, &table_current) == XUI_OK && memcmp(&table_base, &table_current, sizeof(table_base)) == 0);
        paint(c); CHECK(frame_renders == 0);
        CHECK(xuiWidgetSetInlineStyle(table, NULL, 0) == XUI_OK);
        paint(c); CHECK(seen(p.tValue.iColor));
        paint(c); CHECK(frame_renders == 0);
    }
    CHECK(xuiWidgetSetInlineStyle(grid, NULL, 0) == XUI_OK);
    CHECK(xuiPropertyGridSetSelected(grid, -1) == XUI_OK);
    paint(c);
    {
        xui_rect_t category_rect, name_rect, world = xuiWidgetGetWorldRect(viewport);
        CHECK(xuiTableViewGetCellContentRect(table, 0, 0, &category_rect) == XUI_OK);
        CHECK(xuiInputPointerMove(c, world.fX + 30, world.fY + category_rect.fH / 2, 0) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
        paint(c); CHECK(seen(props[3].tValue.iColor));
        painted_keys[3] |= seen(props[3].tValue.iColor);
        CHECK(xuiTableViewGetCellContentRect(table, 1, 0, &name_rect) == XUI_OK);
        CHECK(xuiInputPointerMove(c, world.fX + 30, world.fY + name_rect.fY + name_rect.fH / 2, 0) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
        paint(c); CHECK(seen(props[8].tValue.iColor));
        painted_keys[8] |= seen(props[8].tValue.iColor);
        CHECK(xuiInputPointerMove(c, 639, 419, 0) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(c) == XUI_OK);
        paint(c);
    }
    for (i = 0; i < 15; ++i) {
        if (!painted_keys[i]) printf("Unpainted property: %s\n", props[i].sName);
        CHECK(painted_keys[i]);
    }
    for (i = 0; i < 15; ++i) props[i].tValue.iColor = 0;
    CHECK(xuiWidgetSetInlineStyle(grid, props, 15) == XUI_OK);
    paint_only(grid); paint(c); CHECK(frame_renders == 1);
    CHECK(!seen(base.iCategoryTextColor) && !seen(base.iNameTextColor) && !seen(base.iValueTextColor));
    CHECK(!seen(base.iCategoryBackgroundColor) && !seen(base.iNameBackgroundColor) && !seen(base.iValueBackgroundColor));
    CHECK(!seen(base.iGridColor));
    CHECK(seen(0xc8ab43ff));
    paint(c); CHECK(frame_renders == 0);
    base.iCategoryBackgroundColor = 0x173957ff; base.iValueTextColor = 0x795317ff;
    CHECK(xuiPropertyGridSetStyle(grid, &base) == XUI_OK);
    paint_only(grid); paint_only(table); paint(c);
    CHECK(!seen(base.iCategoryBackgroundColor) && !seen(base.iValueTextColor));
    CHECK(xuiPropertyGridGetStyle(grid, &current) == XUI_OK && memcmp(&base, &current, sizeof(base)) == 0);
    CHECK(xuiWidgetSetInlineStyle(grid, NULL, 0) == XUI_OK);
    xuiWidgetClearStyleClasses(grid);
    CHECK(xuiStyleRemoveType(c, xuiPropertyGridGetType(c)) == XUI_OK);
    CHECK(xuiStyleClearDefault(c) == XUI_OK);
    paint(c); CHECK(seen(base.iCategoryBackgroundColor) && seen(base.iValueTextColor));
    paint(c); CHECK(frame_renders == 0);

    {
        xui_style_property_t mixed[4];
        mixed[0] = prop("tableview.background.color", 0x153759ff);
        mixed[1] = prop("tableview.grid.color", 0x597315ff);
        mixed[2] = prop("propertygrid.background.color", 0x731559ff);
        mixed[3] = prop("propertygrid.grid.color", 0x375915ff);
        CHECK(xuiStyleSetDefault(c, mixed, 4) == XUI_OK);
        paint_only(grid); paint_only(table); paint(c);
        CHECK(seen(mixed[2].tValue.iColor) && seen(mixed[3].tValue.iColor));
        CHECK(!seen(mixed[0].tValue.iColor) && !seen(mixed[1].tValue.iColor));
        paint(c); CHECK(frame_renders == 0);
        mixed[2].tValue.iColor = 0; mixed[3].tValue.iColor = 0;
        CHECK(xuiStyleSetDefault(c, mixed, 4) == XUI_OK);
        paint(c); CHECK(!seen(mixed[0].tValue.iColor) && !seen(mixed[1].tValue.iColor));
        CHECK(xuiStyleSetDefault(c, mixed, 2) == XUI_OK);
        paint(c); CHECK(seen(mixed[0].tValue.iColor) && seen(mixed[1].tValue.iColor));
        paint(c); CHECK(frame_renders == 0);
        CHECK(xuiStyleClearDefault(c) == XUI_OK);
        paint(c); CHECK(seen(base.iBackgroundColor) && seen(base.iGridColor));
        paint(c); CHECK(frame_renders == 0);
    }
    p = prop("tableview.text.color", 0x416385ff); s = style(&p, 1);
    CHECK(xuiStyleSetType(c, xuiTableViewGetType(c), &s) == XUI_OK);
    paint_only(table); paint(c); CHECK(seen(p.tValue.iColor) && !seen(base.iValueTextColor));
    paint(c); CHECK(frame_renders == 0);
    p.tValue.iColor = 0x856341ff;
    CHECK(xuiWidgetSetInlineStyle(table, &p, 1) == XUI_OK);
    paint(c); CHECK(seen(p.tValue.iColor) && frame_renders == 1);
    paint(c); CHECK(frame_renders == 0);
    CHECK(xuiWidgetSetInlineStyle(table, NULL, 0) == XUI_OK);
    CHECK(xuiStyleRemoveType(c, xuiTableViewGetType(c)) == XUI_OK);
    paint(c); CHECK(seen(base.iValueTextColor));
    paint(c); CHECK(frame_renders == 0);
    CHECK(xuiPropertyGridSetColors(grid, 0x354719ff, base.iCategoryBackgroundColor,
        base.iValueBackgroundColor, base.iSelectedColor, base.iGridColor, base.iValueTextColor) == XUI_OK);
    paint_only(grid); paint_only(table); paint(c); CHECK(seen(0x354719ff));
    paint(c); CHECK(frame_renders == 0);
    {
        xui_rect_t after = xuiWidgetGetRect(viewport);
        CHECK(memcmp(&before, &after, sizeof(before)) == 0);
    }
    CHECK(grid->iLayoutVersion == owner_layout && table->iLayoutVersion == table_layout && viewport->iLayoutVersion == viewport_layout);
    {
        xui_widget ordinary;
        CHECK(xuiTableViewCreate(c, &ordinary, NULL) == XUI_OK);
        CHECK(xuiSetRootWidget(c, ordinary) == XUI_OK);
        paint(c);
        for (i = 0; i < 15; ++i) props[i].tValue.iColor = 0x451973ffu + (uint32_t)i * 0x00010200u;
        CHECK(xuiStyleSetDefault(c, props, 15) == XUI_OK);
        paint(c);
        for (i = 0; i < 15; ++i) CHECK(!seen(props[i].tValue.iColor));
        xuiWidgetDestroy(grid);
    }
    xuiDestroy(c);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}

int main(int argc, char** argv)
{
    if (argc == 1 || strcmp(argv[1], "scroll") == 0) scroll_styles();
    if (argc == 1 || strcmp(argv[1], "dock") == 0) { dock_styles(); dock_drag_styles(); }
    if (argc == 1 || strcmp(argv[1], "table") == 0) table_grid_styles();
    if (argc == 1 || strcmp(argv[1], "property") == 0) property_styles();
    printf("style_containers: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
