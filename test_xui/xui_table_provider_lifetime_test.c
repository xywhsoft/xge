#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { COUNT, CELL, MERGE, CELL_FORMAT, COLUMN_FORMAT, CELL_RENDER, COLUMN_RENDER, TABLE_RENDER, HEADER_RENDER, CALLBACKS };
enum { LIVE, COLUMNS, ROWS, ADAPTER, REFRESH, DESTROY_SELF, DESTROY_PARENT, DESTROY_CONTEXT, READ, DESTROY_ROOT,
    CHANGE_RENDERER, CHANGE_FORMATTER, READ_OTHER, READ_SAME, ACTIONS };
enum { GET_COUNT, SET_COLUMNS, SET_ROWS, SET_ADAPTER, METRICS, WIDTH, REFRESH_PATH,
    CONTENT_RECT, CELL_RECT, ITEM_AT, ENSURE, SELECT_ROW, SELECT_CELL, PAINT, HEADER, RENDER, PATHS };

typedef struct fixture_t {
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_widget root, parent, table;
    xui_surface target;
    xui_widget_cache_render_proc bodyPaint, headerPaint;
    void* bodyUser;
    void* headerUser;
    int attempt, nextAttempt, invalidatedAttempt;
    int callback, action, path, armed, fired, inside, invalidated;
    int stale, missingBarrier, nestedResult, destroyed, contextDestroyed;
    int drawsAfter, calls[CALLBACKS];
    int destructs;
    int selectCalls, contextCalls, resizeCalls;
} fixture_t;

static int checks, failures, cases;
static int trace;
static fixture_t* painting;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; \
    if (failures <= 30) printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)

static int replacement_count(xui_widget table, void* user) { return 2; }
static int replacement_cell(xui_widget table, int row, int column, xui_table_view_cell_t* cell, void* user)
{
    cell->sText = "replacement";
    return 1;
}

static void probe(xui_widget table, fixture_t* f, int callback)
{
    xui_table_view_column_t column = {0};
    xui_table_view_row_t rows[3] = {{0}};
    xui_rect_t rect;
    if (!f->armed) return;
    ++f->calls[callback];
    if (f->invalidated && f->attempt == f->invalidatedAttempt && !f->inside) ++f->stale;
    if (f->fired || callback != f->callback) return;
    f->fired = 1;
    if (f->context->iOperationDepth == 0) ++f->missingBarrier;
    if (f->action == LIVE) return;
    f->inside = 1;
    switch (f->action) {
    case COLUMNS:
        column.sTitle = "replacement"; column.fWidth = 137;
        f->nestedResult = xuiTableViewSetColumns(table, &column, 1);
        break;
    case ROWS:
        rows[0].fHeight = 31; rows[1].fHeight = 37; rows[2].fHeight = 41;
        f->nestedResult = xuiTableViewSetRows(table, rows, 3);
        break;
    case ADAPTER: f->nestedResult = xuiTableViewSetAdapter(table, replacement_count, replacement_cell, f); break;
    case REFRESH: f->nestedResult = xuiTableViewRefresh(table); break;
    case CHANGE_RENDERER: f->nestedResult = xuiTableViewSetCellRenderer(table, NULL, NULL); break;
    case CHANGE_FORMATTER: f->nestedResult = xuiTableViewSetColumnFormatter(table, 0, NULL, NULL); break;
    case READ_OTHER: f->nestedResult = xuiTableViewGetCellContentRect(table, 3, 1, &rect); break;
    case READ_SAME: f->nestedResult = xuiTableViewGetCellContentRect(table, 0, 0, &rect); break;
    case DESTROY_SELF:
    case DESTROY_PARENT:
    case DESTROY_CONTEXT:
    case DESTROY_ROOT:
        /* Fail deterministically on an unprotected baseline instead of reading freed memory. */
        if (f->missingBarrier) break;
        f->destroyed = 1;
        if (f->action == DESTROY_CONTEXT) { f->contextDestroyed = 1; xuiDestroy(f->context); }
        else xuiWidgetDestroy(f->action == DESTROY_SELF ? table : (f->action == DESTROY_ROOT ? f->root : f->parent));
        break;
    case READ:
        CHECK(xuiTableViewGetColumnCount(table) == 3);
        CHECK(xuiTableViewGetSelectedRow(table) == -1);
        f->nestedResult = xuiTableViewGetRowCount(table);
        break;
    }
    f->inside = 0;
    f->invalidated = f->action != READ && f->action != READ_OTHER && f->action != READ_SAME;
    f->invalidatedAttempt = f->attempt;
}

static int provider_count(xui_widget table, void* user)
{
    probe(table, user, COUNT);
    return 7;
}
static int format_cell(xui_widget table, int row, int column, const xui_table_view_cell_t* cell, char* text, int capacity, void* user)
{
    fixture_t* f = user;
    probe(table, f, f->callback == CELL_FORMAT ? CELL_FORMAT : COLUMN_FORMAT);
    snprintf(text, (size_t)capacity, "old formatted value");
    return 1;
}
static int render_cell(xui_widget table, int row, int column, const xui_table_view_cell_t* cell,
    xui_draw_context draw, xui_rect_t rect, int state, void* user)
{
    fixture_t* f = user;
    probe(table, f, f->callback);
    return 0;
}
static int render_header(xui_widget table, int column, const xui_table_view_column_t* data,
    xui_draw_context draw, xui_rect_t rect, int state, void* user)
{
    probe(table, user, HEADER_RENDER);
    return 0;
}
static int provider_cell(xui_widget table, int row, int column, xui_table_view_cell_t* cell, void* user)
{
    fixture_t* f = user;
    probe(table, f, CELL);
    if (f->armed && f->invalidated && f->attempt == f->invalidatedAttempt && !f->inside) cell->sText = (const char*)(uintptr_t)1;
    if (f->callback == CELL_FORMAT) { cell->onFormat = format_cell; cell->pFormatUser = f; }
    if (f->callback == CELL_RENDER) { cell->onRender = render_cell; cell->pRenderUser = f; }
    return 1;
}
static int provider_merge(xui_widget table, int row, int column, int* rows, int* columns, void* user)
{
    probe(table, user, MERGE);
    *rows = row == 0 && column == 0 ? 2 : 1;
    *columns = 1;
    return 1;
}
static int draw_line(xui_proxy proxy, xui_draw_context draw, float x1, float y1, float x2, float y2, float width, uint32_t color)
{
    if (painting && painting->invalidated && painting->attempt == painting->invalidatedAttempt && !painting->inside) ++painting->drawsAfter;
    return XUI_OK;
}

static int paint_attempt(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
    fixture_t* f = user;
    int result, previous = f->attempt;
    f->attempt = ++f->nextAttempt;
    result = widget == f->table ? f->headerPaint(widget, draw, state, f->headerUser) : f->bodyPaint(widget, draw, state, f->bodyUser);
    f->attempt = previous;
    return result;
}

static void sentinel_destroy(xui_widget widget, void* data, void* user)
{
    fixture_t* f = user;
    CHECK(!f->inside);
    ++f->destructs;
}

static int setup(fixture_t* f)
{
    xui_table_view_column_t columns[3] = {{0}};
    xui_widget_type_desc_t desc = {0};
    xui_widget_type type = NULL;
    xui_widget sentinel = NULL;
    int i;
    xuiTestProxyInit(&f->proxy);
    f->proxy.tProxy.drawLine = draw_line;
    if (xuiCreate(&f->context) != XUI_OK || xuiSetProxy(f->context, &f->proxy.tProxy) != XUI_OK ||
        xuiInputViewport(f->context, 400, 240) != XUI_OK || xuiWidgetCreate(f->context, &f->root) != XUI_OK ||
        xuiWidgetCreate(f->context, &f->parent) != XUI_OK ||
        xuiTableViewCreate(f->context, &f->table, NULL) != XUI_OK) return 0;
    (void)xuiSetRootWidget(f->context, f->root);
    (void)xuiWidgetSetRect(f->root, (xui_rect_t){0, 0, 400, 240});
    (void)xuiWidgetAddChild(f->root, f->parent);
    (void)xuiWidgetSetRect(f->parent, (xui_rect_t){0, 0, 400, 240});
    (void)xuiWidgetAddChild(f->parent, f->table);
    desc.iSize = sizeof(desc); desc.sName = "table.provider.sentinel";
    desc.onDestroy = sentinel_destroy; desc.pUser = f;
    if (xuiWidgetRegisterType(f->context, &type, &desc) != XUI_OK ||
        xuiWidgetCreateTyped(f->context, type, &sentinel, NULL) != XUI_OK ||
        xuiWidgetAddChild(f->table, sentinel) != XUI_OK) return 0;
    (void)xuiWidgetSetRect(f->table, (xui_rect_t){10, 10, 320, 200});
    for (i = 0; i < 3; ++i) {
        columns[i].fWidth = 80;
        if (f->callback == COLUMN_FORMAT) { columns[i].onFormat = format_cell; columns[i].pFormatUser = f; }
        if (f->callback == COLUMN_RENDER) { columns[i].onRender = render_cell; columns[i].pRenderUser = f; }
    }
    if (xuiTableViewSetColumns(f->table, columns, 3) != XUI_OK ||
        xuiTableViewSetRows(f->table, NULL, 7) != XUI_OK ||
        xuiTableViewSetAdapter(f->table, provider_count, provider_cell, f) != XUI_OK ||
        xuiTableViewSetMergeProvider(f->table, provider_merge, f) != XUI_OK) return 0;
    if (f->callback == TABLE_RENDER) (void)xuiTableViewSetCellRenderer(f->table, render_cell, f);
    if (f->callback == HEADER_RENDER) (void)xuiTableViewSetHeaderRenderer(f->table, render_header, f);
    (void)xuiWidgetGetCacheRenderCallback(f->table, &f->headerPaint, &f->headerUser);
    (void)xuiWidgetGetCacheRenderCallback(xuiTableViewGetViewportWidget(f->table), &f->bodyPaint, &f->bodyUser);
    (void)xuiWidgetSetCacheRenderCallback(f->table, paint_attempt, f);
    (void)xuiWidgetSetCacheRenderCallback(xuiTableViewGetViewportWidget(f->table), paint_attempt, f);
    return xuiTestSurfaceCreate(&f->proxy, &f->target, 400, 240, XUI_SURFACE_USAGE_TARGET) == XUI_OK && xuiLayout(f->context) == XUI_OK;
}

static int run_case(int callback, int action, int path, int warm)
{
    fixture_t f = {0};
    xui_table_view_column_t columns[3] = {{0}};
    xui_rect_t rect = {0}, viewport;
    xui_rect_i_t damage = {0, 0, 400, 240};
    xui_widget widget;
    xui_widget_cache_render_proc paint;
    xui_draw_context draw = NULL;
    void* user;
    int result = XUI_OK, before = failures, row = -1, column = -1;
    if (trace) { printf("running callback=%d action=%d path=%d warm=%d\n", callback, action, path, warm); fflush(stdout); }
    f.callback = callback; f.action = action; f.path = path;
    CHECK(setup(&f));
    if (!f.table || !f.target) goto cleanup;
    if (warm) CHECK(xuiTableViewGetCellContentRect(f.table, 6, 2, &rect) == XUI_OK);
    viewport = xuiTableViewGetViewportRect(f.table);
    f.armed = 1;
    switch (path) {
    case GET_COUNT: result = xuiTableViewGetRowCount(f.table); break;
    case SET_COLUMNS: result = xuiTableViewSetColumns(f.table, columns, 3); break;
    case SET_ROWS: result = xuiTableViewSetRows(f.table, NULL, 7); break;
    case SET_ADAPTER: result = xuiTableViewSetAdapter(f.table, provider_count, provider_cell, &f); break;
    case METRICS: result = xuiTableViewSetDefaultMetrics(f.table, 81, 21, 24); break;
    case WIDTH: result = xuiTableViewSetColumnWidth(f.table, 0, 91); break;
    case REFRESH_PATH: result = xuiTableViewRefreshAdapter(f.table); break;
    case CONTENT_RECT: result = xuiTableViewGetCellContentRect(f.table, 0, 0, &rect); break;
    case CELL_RECT: result = xuiTableViewGetCellRect(f.table, 0, 0, &rect); break;
    case ITEM_AT: result = xuiTableViewGetItemAt(f.table, viewport.fX + 4, viewport.fY + 4, &row, &column); break;
    case ENSURE: result = xuiTableViewEnsureCellVisible(f.table, 0, 0); break;
    case SELECT_ROW: result = xuiTableViewSetSelectedRow(f.table, 0); break;
    case SELECT_CELL: result = xuiTableViewSetSelectedCell(f.table, 0, 0); break;
    case PAINT:
    case HEADER:
        widget = path == HEADER ? f.table : xuiTableViewGetViewportWidget(f.table);
        CHECK(xuiWidgetGetCacheRenderCallback(widget, &paint, &user) == XUI_OK && paint != NULL);
        CHECK(f.proxy.tProxy.drawBegin(&f.proxy.tProxy, &draw, f.target) == XUI_OK);
        painting = &f;
        result = paint(widget, draw, 0, user);
        painting = NULL;
        (void)f.proxy.tProxy.drawEnd(&f.proxy.tProxy, draw);
        break;
    case RENDER:
        result = xuiRender(f.context, f.target, &damage, 1);
        break;
    }
    CHECK(f.fired == 1);
    CHECK(f.missingBarrier == 0);
    CHECK(f.stale == 0);
    CHECK(f.drawsAfter == 0);
    if ((action >= DESTROY_SELF && action <= DESTROY_CONTEXT) || action == DESTROY_ROOT) CHECK(f.destroyed && f.destructs == 1);
    if (f.invalidated && path < PAINT) {
        CHECK(path == GET_COUNT ? result == 0 : result < 0);
        if (path == CONTENT_RECT || path == CELL_RECT) CHECK(rect.fW == 0 && rect.fH == 0);
        if (path == ITEM_AT) CHECK(row == -1 && column == -1);
    } else if (action == LIVE || action == READ || action == READ_OTHER || action == READ_SAME) {
        CHECK(path == GET_COUNT ? result == 7 : result == XUI_OK);
    }
    f.armed = 0;
    if (!f.destroyed) {
        if (action == COLUMNS) CHECK(xuiTableViewGetColumnCount(f.table) == 1 && xuiTableViewGetColumnWidth(f.table, 0) == 137);
        if (action == ADAPTER) {
            CHECK(xuiTableViewGetRowCount(f.table) == 2);
            CHECK(xuiTableViewGetModel(f.table)->fContentHeight == (path == METRICS ? 42 : 40));
        }
        if (action == ROWS) CHECK(xuiTableViewGetCellContentRect(f.table, 2, 0, &rect) == XUI_OK && rect.fH == 41);
        if (action == COLUMNS || action == ROWS || action == ADAPTER || action == REFRESH) CHECK(f.nestedResult == XUI_OK);
        if (action == CHANGE_RENDERER || action == CHANGE_FORMATTER) CHECK(f.nestedResult == XUI_OK);
        if (action == READ) CHECK(f.nestedResult == 7);
        if (action == READ_OTHER) CHECK(warm ? f.nestedResult == XUI_OK : f.nestedResult < 0);
        if (action == READ_SAME) CHECK(f.nestedResult < 0);
    }
    if (!f.contextDestroyed) CHECK(f.context->iOperationDepth == 0);
cleanup:
    if (f.context && !f.contextDestroyed) xuiDestroy(f.context);
    if (f.target) f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, f.target);
    CHECK(f.destructs == 1);
    ++cases;
    if (failures != before) printf("case callback=%d action=%d path=%d warm=%d result=%d failed\n", callback, action, path, warm, result);
    return failures == before;
}

static void event_select(xui_widget table, int row, int column, int mode, void* user)
{
    fixture_t* f = user;
    ++f->selectCalls;
    if (f->invalidated) ++f->stale;
}

static void event_hover(xui_widget table, int row, int column, int mode, void* user)
{
    fixture_t* f = user;
    if (f->invalidated) ++f->stale;
}

static void event_resize(xui_widget table, int column, float width, void* user)
{
    fixture_t* f = user;
    ++f->resizeCalls;
    if (f->invalidated) ++f->stale;
}

static int event_context(xui_widget table, int part, int row, int column, float x, float y, void* user)
{
    fixture_t* f = user;
    ++f->contextCalls;
    if (f->invalidated) ++f->stale;
    return XUI_OK;
}

/* Context pointer/key, hover during pointer-down, and count during resize share providers. */
static void event_case(int kind, int action, int callback)
{
    fixture_t f = {0};
    xui_event_t event = {0};
    xui_rect_t rect, world;
    int before = failures;
    f.callback = callback; f.action = action;
    CHECK(setup(&f));
    if (!f.table || !f.target) goto cleanup;
    CHECK(xuiTableViewSetSelect(f.table, event_select, &f) == XUI_OK);
    CHECK(xuiTableViewSetHover(f.table, event_hover, &f) == XUI_OK);
    CHECK(xuiTableViewSetColumnResize(f.table, event_resize, &f) == XUI_OK);
    CHECK(xuiTableViewSetContextMenu(f.table, event_context, &f) == XUI_OK);
    CHECK(xuiTableViewGetCellRect(f.table, 0, 0, &rect) == XUI_OK);
    world = xuiWidgetGetWorldRect(f.table);
    event.iSize = sizeof(event); event.pTarget = f.table;
    event.fX = world.fX + rect.fX + 4; event.fY = world.fY + rect.fY + 4;
    event.iButton = XUI_POINTER_BUTTON_LEFT;
    event.iType = XUI_EVENT_CONTEXT_MENU;
    if (kind == 1) event.iType = XUI_EVENT_POINTER_DOWN;
    if (kind == 2) {
        event.iType = XUI_EVENT_POINTER_DOWN;
        event.fX = world.fX + rect.fX + 80; event.fY = world.fY + 12;
        CHECK(xuiDispatchEvent(f.context, &event) >= 0);
        CHECK(xuiGetPointerCapture(f.context) == f.table);
        event.iType = XUI_EVENT_POINTER_MOVE; event.fX += 17;
    }
    if (kind == 3) {
        CHECK(xuiTableViewSetSelectedCell(f.table, 0, 0) == XUI_OK);
        event.iKey = XUI_KEY_CONTEXT_MENU;
    }
    if (kind == 4) {
        event.iType = XUI_EVENT_POINTER_MOVE;
        CHECK(xuiDispatchEvent(f.context, &event) >= 0);
    }
    if (kind == 0 || kind == 4) CHECK(xuiTableViewRefresh(f.table) == XUI_OK);
    f.armed = 1;
    (void)xuiDispatchEvent(f.context, &event);
    CHECK(f.fired == 1 && f.missingBarrier == 0);
    CHECK(f.stale == 0);
    CHECK(f.selectCalls == (action == LIVE && (kind == 0 || kind == 1) ? 1 : 0));
    CHECK(f.contextCalls == (action == LIVE && (kind == 0 || kind == 3) ? 1 : 0));
    CHECK(f.resizeCalls == (action == LIVE && kind == 2 ? 1 : 0));
    if (!f.destroyed) CHECK(xuiTableViewGetSelectedRow(f.table) ==
        (kind == 3 || (action == LIVE && (kind == 0 || kind == 1)) ? 0 : -1));
cleanup:
    f.armed = 0;
    if (f.context && !f.contextDestroyed) xuiDestroy(f.context);
    if (f.target) f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, f.target);
    CHECK(f.destructs == 1);
    ++cases;
    if (failures != before) printf("event kind=%d callback=%d action=%d failed\n", kind, callback, action);
}

int main(int argc, char** argv)
{
    int callback, action, path, warm, kind;
    trace = argc == 2 && strcmp(argv[1], "--trace") == 0;
    if (argc == 5 && strcmp(argv[1], "--event") == 0) event_case(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    else if (argc == 5) run_case(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    else for (callback = 0; callback < CALLBACKS; ++callback) for (action = 0; action < ACTIONS; ++action) {
        if ((action == READ_OTHER || action == READ_SAME) && callback != CELL && callback != MERGE) continue;
        for (path = 0; path < PATHS; ++path) {
            if (callback == HEADER_RENDER ? path != HEADER && path != RENDER : path == HEADER) continue;
            if (callback >= CELL_FORMAT && callback < HEADER_RENDER && path != PAINT && path != RENDER) continue;
            if ((callback == CELL || callback == MERGE) && (path < CONTENT_RECT || path == SELECT_ROW)) continue;
            for (warm = 0; warm <= 1; ++warm) {
                if (warm && (path < CONTENT_RECT || path == ITEM_AT || callback == HEADER_RENDER)) continue;
                run_case(callback, action, path, warm);
            }
        }
    }
    if (argc != 5) for (kind = 0; kind < 5; ++kind) for (callback = COUNT; callback <= MERGE; ++callback) {
        if (kind == 2 && callback != COUNT) continue;
        for (action = LIVE; action <= DESTROY_CONTEXT; ++action) event_case(kind, action, callback);
    }
    printf("xui_table_provider_lifetime_test: %d cases, %d checks, %d failures\n", cases, checks, failures);
    return failures ? 1 : 0;
}
