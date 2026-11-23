#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC) && defined(_WIN32)
#define XRT_USE_SHARED
#endif
#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
#include "xge_host_xrt_contract.h"
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if (!strcmp(name, #symbol)) return (xge_host_xrt_proc)&symbol;
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

static void populate_and_clear(xui_widget grid, bool configure)
{
    const xui_property_desc_t property = {.sId="mode", .sValue="fast", .iType=XUI_TABLE_CELL_TYPE_ENUM};
    int category = xuiPropertyGridAddCategory(grid, "test", "Test", 1);
    assert(category == 0 && xuiPropertyGridAddProperty(grid, category, &property) == 0);
    const char* options[] = {"fast", "safe"};
    const uint32_t palette[] = {0xB4C2D8FFu};
    const xui_table_grid_editor_config_t config = {
        .arrEnumItems=options, .iEnumItemCount=2, .iEnumSelected=-1,
        .arrPalette=palette, .iPaletteCount=1, .sDateFormat="yyyy-MM-dd",
        .sActionText="Apply", .sFileFilter="*.txt", .sRangeSeparator=" ~ "
    };
    if ( configure ) assert(xuiPropertyGridSetEditorConfig(grid, 0, &config) == XUI_OK);
    assert(xuiPropertyGridClear(grid) == XUI_OK);
    assert(xuiPropertyGridGetCategoryCount(grid) == 0 && xuiPropertyGridGetPropertyCount(grid) == 0);
}

int main(void)
{
#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
    const xge_host_xrt_contract_t contract = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_context_desc_t settings;
    xuiContextDescDefault(&settings);
    settings.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    xui_widget grid = NULL;
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    assert(xuiCreateEx(&settings, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    assert(xuiPropertyGridCreate(context, &grid, NULL) == XUI_OK);
    populate_and_clear(grid, false);
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    for ( int i = 0; i < 8; ++i ) populate_and_clear(grid, true);
    xrtMemDebugSnapshot(&after);
    assert(xuiDestroyChecked(context) == XUI_OK);
    if ( after.LiveCount != before.LiveCount || after.LiveBytes != before.LiveBytes ) {
        fprintf(stderr, "property grid clear left live allocations: %llu -> %llu, bytes: %llu -> %llu\n",
            (unsigned long long)before.LiveCount, (unsigned long long)after.LiveCount,
            (unsigned long long)before.LiveBytes, (unsigned long long)after.LiveBytes);
        return 1;
    }
    puts("property_grid_config: repeated clear released every copied option and string");
    return 0;
}
