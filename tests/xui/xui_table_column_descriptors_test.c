#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include "xrt.h"
#include "xui_test_proxy.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef int (*column_setter)(xui_widget, const xui_table_column_desc_t*, int);
typedef int (*column_getter)(xui_widget, int, xui_table_column_desc_t*);

static void check_columns(xui_widget widget, column_setter set, column_getter get)
{
    char id[] = "price";
    char title[] = "Price";
    xui_table_column_desc_t columns[] = {
        { .tSettings = {
            .iId = 21, .fWidth = 120, .fMinWidth = 80, .fMaxWidth = 160,
            .bVisible = 1, .bVisibleSet = 1, .bResizable = 0, .bResizableSet = 1,
            .iAlign = XUI_TEXT_ALIGN_RIGHT, .iType = XUI_TABLE_CELL_TYPE_INT,
            .bHasStyle = 1, .iBackgroundColor = 0x123456ff,
            .iTextColor = 0xfedcbaff, .iGridColor = 0x234567ff
        }, .sId = id, .sTitle = title },
        { .tSettings = { .iId = 22, .fWidth = 70, .bVisibleSet = 1 }, .sTitle = "" },
        { .tSettings = { .iId = 23, .fWidth = 70 } }
    };
    assert(set(widget, columns, 3) == XUI_OK);
    id[0] = '!';
    title[0] = '!';
    xui_table_column_desc_t info = {0};
    assert(get(widget, 0, &info) == XUI_OK);
    assert(!strcmp(info.sId, "price") && !strcmp(info.sTitle, "Price"));
    const xui_table_column_settings_t* settings = &info.tSettings;
    assert(settings->iId == 21 && settings->fWidth == 120 && settings->fMinWidth == 80 && settings->fMaxWidth == 160);
    assert(settings->bVisible && settings->bVisibleSet && !settings->bResizable && settings->bResizableSet);
    assert(settings->iAlign == XUI_TEXT_ALIGN_RIGHT && settings->iType == XUI_TABLE_CELL_TYPE_INT);
    assert(settings->bHasStyle && settings->iBackgroundColor == 0x123456ff);
    assert(settings->iTextColor == 0xfedcbaff && settings->iGridColor == 0x234567ff);
    assert(get(widget, 1, &info) == XUI_OK && info.sId == NULL && info.sTitle != NULL && !info.sTitle[0]);
    assert(!info.tSettings.bVisible && info.tSettings.bResizable);
    assert(get(widget, 2, &info) == XUI_OK && info.sTitle == NULL && info.tSettings.bVisible);
    assert(get(widget, 3, &info) == XUI_ERROR_NOT_FOUND && info.tSettings.iId == 23);

    assert(xrtMemDebugFailAfter(0));
    assert(get(widget, 0, &info) == XUI_OK && !xrtMemDebugFailTriggered());
    assert(set(widget, columns, 3) == XUI_ERROR_OUT_OF_MEMORY && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    assert(get(widget, 0, &info) == XUI_OK && !strcmp(info.sTitle, "Price"));

    columns[0].tSettings.fMaxWidth = 30;
    assert(set(widget, columns, 3) == XUI_ERROR_INVALID_ARGUMENT);
    assert(set(widget, columns, XUI_TABLE_VIEW_COLUMN_CAPACITY + 1) == XUI_ERROR_INVALID_ARGUMENT);
    assert(get(widget, 0, &info) == XUI_OK && info.tSettings.fMaxWidth == 160);
    assert(set(widget, NULL, 0) == XUI_OK && get(widget, 0, &info) == XUI_ERROR_NOT_FOUND);
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context context = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_widget table = NULL;
    xui_widget grid = NULL;
    assert(xuiTableViewCreate(context, &table, NULL) == XUI_OK);
    assert(xuiTableGridCreate(context, &grid, NULL) == XUI_OK);
    check_columns(table, xuiTableViewSetColumnDescriptors, xuiTableViewGetColumnDescriptor);
    check_columns(grid, xuiTableGridSetColumnDescriptors, xuiTableGridGetColumnDescriptor);
    xuiDestroy(context);
    printf("XUI_TABLE_COLUMNS_NATIVE_OK native_record=%zu descriptor=%zu\n",
           sizeof(xui_table_view_column_t), sizeof(xui_table_column_desc_t));
    return 0;
}
