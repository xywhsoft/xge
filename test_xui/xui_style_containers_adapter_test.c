#include "../src/xui_table_view.c"
#define main provider_lifetime_main
#include "xui_table_provider_lifetime_test.c"
#undef main

static int adapter_calls;
static int adapter_chrome(xui_widget table, int row, int column, const xui_table_view_cell_t* cell,
    xui_draw_context draw, xui_rect_t rect, int state, void* user)
{
    fixture_t* f = user;
    ++adapter_calls;
    if (f->action == ACTIONS) {
        ++f->fired;
        CHECK(f->context->iOperationDepth > 0);
        CHECK(xuiInternalTableViewSetPaintAdapter(table, NULL) == XUI_OK);
        f->invalidated = 1; f->invalidatedAttempt = f->attempt;
    } else probe(table, f, CELL_RENDER);
    return 0;
}

static void adapter_lifetime(int decorations, int action)
{
    fixture_t f = {0};
    xui_table_view_paint_adapter_t adapter = {0};
    xui_draw_context draw = NULL;
    xui_widget_cache_render_proc paint;
    void* user;
    f.callback = CELL_RENDER; f.action = action;
    CHECK(setup(&f));
    CHECK(xuiTableViewSetAdapter(f.table, provider_count, replacement_cell, &f) == XUI_OK);
    adapter.pUser = &f;
    if (decorations) adapter.onCellDecorations = adapter_chrome;
    else adapter.onCellBackground = adapter_chrome;
    CHECK(xuiInternalTableViewSetPaintAdapter(f.table, &adapter) == XUI_OK);
    CHECK(xuiWidgetGetCacheRenderCallback(xuiTableViewGetViewportWidget(f.table), &paint, &user) == XUI_OK);
    CHECK(f.proxy.tProxy.drawBegin(&f.proxy.tProxy, &draw, f.target) == XUI_OK);
    f.armed = 1; adapter_calls = 0; painting = &f;
    CHECK(paint(xuiTableViewGetViewportWidget(f.table), draw, 0, user) == XUI_OK);
    painting = NULL;
    CHECK(f.proxy.tProxy.drawEnd(&f.proxy.tProxy, draw) == XUI_OK);
    CHECK(f.fired == 1 && f.missingBarrier == 0);
    CHECK(f.stale == 0 && f.drawsAfter == 0);
    CHECK(adapter_calls == 1);
    f.armed = 0;
    if (!f.contextDestroyed) {
        CHECK(f.context->iOperationDepth == 0);
        xuiDestroy(f.context);
    }
    f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, f.target);
    CHECK(f.destructs == 1);
    ++cases;
}

static void adapter_palette(void)
{
    fixture_t f = {0};
    xui_table_view_paint_adapter_t adapter = {0};
    xui_table_view_data_t* data;
    xui_table_view_data_t resolved;
    xui_table_view_colors_t base, current;
    xui_table_view_row_prefix_t* prefix;
    xui_style_property_t p = {0};
    uint64_t merges, generation;
    uint32_t layout, viewport_layout;
    int attempts;
    CHECK(setup(&f));
    CHECK(xuiRenderPrepare(f.context) == XUI_OK);
    data = __xuiTableViewGetData(f.table);
    prefix = data->arrRowPrefix; merges = data->iMergeGeneration;
    layout = f.table->iLayoutVersion;
    viewport_layout = xuiTableViewGetViewportWidget(f.table)->iLayoutVersion;
    CHECK(xuiTableViewGetColors(f.table, &base) == XUI_OK);
    adapter.bHasBackground = 1; adapter.bHasGrid = 1; adapter.bCellColorsPresent = 1;
    CHECK(xuiInternalTableViewSetPaintAdapter(f.table, &adapter) == XUI_OK);
    CHECK(!(xuiWidgetGetDirtyFlags(f.table) & XUI_WIDGET_DIRTY_LAYOUT));
    CHECK(xuiRenderPrepare(f.context) == XUI_OK);
    __xuiTableViewResolve(f.table, data, &resolved);
    CHECK(resolved.iBackgroundColor == 0 && resolved.iGridColor == 0);
    attempts = f.nextAttempt; generation = data->iCallbackGeneration;
    CHECK(xuiInternalTableViewSetPaintAdapter(f.table, &adapter) == XUI_OK);
    CHECK(data->iCallbackGeneration == generation);
    CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.nextAttempt == attempts);
    p.iSize = sizeof(p); p.sName = "tableview.background.color";
    p.tValue.iSize = sizeof(p.tValue); p.tValue.iType = XUI_STYLE_VALUE_COLOR; p.tValue.iColor = 0x635791ff;
    CHECK(xuiWidgetSetInlineStyle(f.table, &p, 1) == XUI_OK);
    CHECK(xuiRenderPrepare(f.context) == XUI_OK);
    __xuiTableViewResolve(f.table, data, &resolved);
    CHECK(resolved.iBackgroundColor == 0);
    CHECK(xuiInternalTableViewSetPaintAdapter(f.table, NULL) == XUI_OK);
    CHECK(xuiRenderPrepare(f.context) == XUI_OK);
    __xuiTableViewResolve(f.table, data, &resolved);
    CHECK(resolved.iBackgroundColor == p.tValue.iColor);
    CHECK(xuiWidgetSetInlineStyle(f.table, NULL, 0) == XUI_OK);
    CHECK(xuiInternalTableViewSetPaintAdapter(f.table, NULL) == XUI_OK);
    CHECK(xuiRenderPrepare(f.context) == XUI_OK);
    __xuiTableViewResolve(f.table, data, &resolved);
    CHECK(resolved.iBackgroundColor == base.iBackgroundColor && resolved.iGridColor == base.iGridColor);
    CHECK(xuiTableViewGetColors(f.table, &current) == XUI_OK && memcmp(&base, &current, sizeof(base)) == 0);
    CHECK(data->arrRowPrefix == prefix && data->iMergeGeneration == merges);
    CHECK(f.table->iLayoutVersion == layout && xuiTableViewGetViewportWidget(f.table)->iLayoutVersion == viewport_layout);
    attempts = f.nextAttempt;
    CHECK(xuiRenderPrepare(f.context) == XUI_OK && f.nextAttempt == attempts);
    xuiDestroy(f.context);
    f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, f.target);
    CHECK(f.destructs == 1);
}

int main(void)
{
    const int actions[] = {COLUMNS, ROWS, ADAPTER, REFRESH, DESTROY_SELF, DESTROY_PARENT,
        DESTROY_CONTEXT, DESTROY_ROOT, CHANGE_RENDERER, ACTIONS};
    int i, decorations;
    adapter_palette();
    for (decorations = 0; decorations < 2; ++decorations)
        for (i = 0; i < (int)(sizeof(actions) / sizeof(actions[0])); ++i)
            adapter_lifetime(decorations, actions[i]);
    printf("style_containers_adapter: %d lifecycle cases, %d checks, %d failures\n", cases, checks, failures);
    return failures ? 1 : 0;
}
