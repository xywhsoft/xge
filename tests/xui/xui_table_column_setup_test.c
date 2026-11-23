#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct column_owner_t {
    size_t refs;
    int holds;
    int drops;
    bool refuse;
    xui_widget widget;
    int expected_id_on_drop;
} column_owner_t;

static bool owner_hold(const void* raw)
{
    column_owner_t* owner = (column_owner_t*)raw;
    if (owner->refuse) return false;
    ++owner->holds;
    ++owner->refs;
    return true;
}

static bool owner_drop(const void* raw)
{
    column_owner_t* owner = (column_owner_t*)raw;
    assert(owner->refs > 1);
    ++owner->drops;
    --owner->refs;
    if (owner->expected_id_on_drop != 0) {
        xui_table_column_desc_t visible;
        assert(xuiTableViewGetColumnDescriptor(owner->widget, 0, &visible) == XUI_OK);
        assert(visible.tSettings.iId == owner->expected_id_on_drop);
    }
    return true;
}

static bool owner_count(const void* raw, size_t* count)
{
    *count = ((const column_owner_t*)raw)->refs;
    return true;
}

static bool owner_trace(const void* raw, xrtownershipvisitor visit, void* user)
{
    (void)raw;
    (void)visit;
    (void)user;
    return true;
}

static const xrtownershipops owner_ops = {owner_count, owner_trace};

static int format(xui_widget widget, int row, int column, const xui_table_view_cell_t* cell,
    xui_table_view_format_output_t* output, void* user)
{
    (void)widget;
    (void)row;
    (void)column;
    (void)cell;
    assert(((column_owner_t*)user)->refs > 1);
    return xuiTableViewFormatOutputSetText(output, XRT_STR_LITERAL("Configured value"));
}

static int render(xui_widget widget, int row, int column, const xui_table_view_cell_t* cell,
    xui_draw_context draw, xui_rect_t rect, int state, void* user)
{
    (void)widget; (void)row; (void)column; (void)cell; (void)draw; (void)rect; (void)state;
    assert(((column_owner_t*)user)->refs > 1);
    return 0;
}

typedef struct setup_data_t {
    xui_widget widget;
    column_owner_t* format_owner;
    column_owner_t* render_owner;
    int previous_id;
    bool refuse;
    bool close;
} setup_data_t;

static int configure(xui_table_column_setup_t* setup, void* raw)
{
    setup_data_t* data = raw;
    xui_table_column_desc_t old;
    assert(xuiTableViewGetColumnDescriptor(data->widget, 0, &old) == XUI_OK);
    assert(old.tSettings.iId == data->previous_id);
    assert(xuiTableViewSetColumns(data->widget, NULL, 0) == XUI_ERROR_BUSY);
    assert(xuiTableViewSetColumnFormatterOwned(data->widget, 0, NULL, NULL, NULL) == XUI_ERROR_BUSY);
    assert(xuiTableColumnSetupSetFormatter(setup, 1, NULL, NULL, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    xui_callback_owner_t formatter = {{data->format_owner, &owner_ops}, owner_hold, owner_drop};
    xui_callback_owner_t renderer = {{data->render_owner, &owner_ops}, owner_hold, owner_drop};
    int result = xuiTableColumnSetupSetFormatter(setup, 0, format, data->format_owner, &formatter);
    if (result != XUI_OK) return result;
    result = xuiTableColumnSetupSetRenderer(setup, 0, render, data->render_owner, &renderer);
    if (result != XUI_OK) return result;
    if (data->close) assert(xuiWidgetDestroyChecked(data->widget) == XUI_OK);
    return data->refuse ? XUI_ERROR_INVALID_ARGUMENT : XUI_OK;
}

static void check_columns(xui_widget table, int expected)
{
    xui_table_column_desc_t view;
    assert(xuiTableViewGetColumnDescriptor(table, 0, &view) == XUI_OK);
    assert(view.tSettings.iId == expected);
}

static void check_format(xui_widget table, const char* name)
{
    xui_accessible_node_t node;
    assert(xuiWidgetGetAccessibleNode(table, 1, &node) == XUI_OK);
    assert(node.sName && !strcmp(node.sName, name));
}

int main(void)
{
    xui_test_proxy_state_t backend;
    xuiTestProxyInit(&backend);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &backend.tProxy) == XUI_OK);
    xui_widget table = NULL;
    assert(xuiTableViewCreate(context, &table, NULL) == XUI_OK);
    xui_table_column_desc_t column = {.tSettings = {.iId = 1, .fWidth = 100}, .sTitle = "Initial"};
    assert(xuiTableViewSetColumnDescriptors(table, &column, 1) == XUI_OK);
    xui_table_view_row_t row = {.fHeight = 20};
    assert(xuiTableViewSetRows(table, &row, 1) == XUI_OK);

    column_owner_t old = {.refs = 1, .widget = table};
    column_owner_t formatter = {.refs = 1};
    column_owner_t renderer = {.refs = 1};
    setup_data_t data = {table, &old, &old, 1, false, false};
    column.tSettings.iId = 2;
    assert(xuiTableViewSetColumnDescriptorsWithCallbacks(table, &column, 1, configure, &data) == XUI_OK);
    assert(old.refs == 3 && old.holds == 2 && old.drops == 0);
    check_format(table, "Configured value");
    data = (setup_data_t){table, &formatter, &renderer, 2, true, false};
    column.tSettings.iId = 3;
    assert(xuiTableViewSetColumnDescriptorsWithCallbacks(table, &column, 1, configure, &data) == XUI_ERROR_INVALID_ARGUMENT);
    assert(formatter.refs == 1 && renderer.refs == 1 && old.refs == 3);
    check_columns(table, 2);
    check_format(table, "Configured value");

    data.refuse = false;
    renderer.refuse = true;
    assert(xuiTableViewSetColumnDescriptorsWithCallbacks(table, &column, 1, configure, &data) == XUI_ERROR_CALLBACK_FAILED);
    assert(formatter.refs == 1 && renderer.refs == 1 && old.refs == 3);
    renderer.refuse = false;
    old.expected_id_on_drop = 3;
    int budget;
    for (budget = 0; budget < 32; ++budget) {
        assert(xrtMemDebugFailAfter((size_t)budget));
        int result = xuiTableViewSetColumnDescriptorsWithCallbacks(table, &column, 1, configure, &data);
        xrtMemDebugFailClear();
        xrtSetErrorTake(NULL);
        if (result == XUI_OK) break;
        assert(result == XUI_ERROR_OUT_OF_MEMORY && old.refs == 3);
        assert(formatter.refs == 1 && renderer.refs == 1);
        check_columns(table, 2);
    }
    assert(budget >= 4 && budget < 32);
    assert(formatter.refs == 2 && renderer.refs == 2 && old.refs == 1 && old.holds == old.drops);
    check_columns(table, 3);
    check_format(table, "Configured value");

    data.previous_id = 3;
    data.close = true;
    assert(xuiTableViewSetColumnDescriptorsWithCallbacks(table, &column, 1, configure, &data) == XUI_ERROR_INVALID_STATE);
    assert(formatter.refs == 1 && renderer.refs == 1);
    assert(formatter.holds == formatter.drops && renderer.holds == renderer.drops);

    xui_widget grid = NULL;
    assert(xuiTableGridCreate(context, &grid, NULL) == XUI_OK);
    assert(xuiTableGridSetColumnDescriptors(grid, &column, 1) == XUI_OK);
    assert(xuiTableGridSetRows(grid, &row, 1) == XUI_OK);
    table = xuiTableGridGetTableView(grid);
    data = (setup_data_t){table, &formatter, &renderer, 3, false, false};
    column.tSettings.iId = 4;
    assert(xuiTableGridSetColumnDescriptorsWithCallbacks(grid, &column, 1, configure, &data) == XUI_OK);
    check_columns(table, 4);
    assert(formatter.refs == 2 && renderer.refs == 2);
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(formatter.refs == 1 && renderer.refs == 1);
    puts("xui_table_column_setup_test passed");
    return 0;
}
