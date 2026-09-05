#include "xui_style_pickers_test.h"
#include "src/xui_cascader.c"
#define ROW(key, field) { "cascader." key, offsetof(xui_cascader_data_t, field) }
static const color_case colors[] = {
    ROW("text.color", iTextColor),
    ROW("placeholder.color", iPlaceholderColor),
    ROW("background.color", iBackgroundColor),
    ROW("background.hover_color", iHoverBackgroundColor),
    ROW("background.open_color", iOpenBackgroundColor),
    ROW("border.color", iBorderColor),
    ROW("border.hover_color", iHoverBorderColor),
    ROW("border.focus_color", iFocusBorderColor),
    ROW("arrow.color", iArrowColor),
    ROW("text.disabled_color", iDisabledTextColor),
    ROW("background.disabled_color", iDisabledBackgroundColor),
    ROW("arrow.disabled_color", iDisabledArrowColor),
    ROW("button.color", iButtonColor),
    ROW("button.hover_color", iButtonHoverColor),
    ROW("button.open_color", iButtonOpenColor),
    ROW("popup.panel_color", iPopupPanelColor),
    ROW("popup.border_color", iPopupBorderColor),
    ROW("popup.shadow_color", iPopupShadowColor),
    ROW("popup.text_color", iPopupTextColor),
    ROW("popup.muted_text_color", iPopupMutedTextColor),
    ROW("popup.hover_color", iPopupHoverColor),
    ROW("popup.active_color", iPopupActiveColor),
    ROW("popup.selected_color", iPopupSelectedColor),
    ROW("popup.active_text_color", iPopupActiveTextColor),
    ROW("popup.disabled_text_color", iPopupDisabledTextColor),
    ROW("popup.separator_color", iPopupSeparatorColor),
};
static uint32_t resolved(xui_widget w, size_t offset)
{
    xui_cascader_data_t r;
    __xuiCascaderResolve(w, __xuiCascaderGetData(w), &r);
    return *(uint32_t*)((char*)&r + offset);
}
int main(void)
{
    xui_widget w;
    xui_cascader_data_t* data;
    xui_cascader_desc_t desc = {0};
    xui_cascader_item_t items[] = {
        {"selected", 1, -1, 0, NULL}, {"active", 2, -1, 0, NULL},
        {"disabled", 3, -1, XUI_CASCADER_ITEM_DISABLED, NULL},
        {"hover", 4, -1, XUI_CASCADER_ITEM_LEAF, NULL},
        {"overflow", 5, -1, XUI_CASCADER_ITEM_LEAF, NULL},
        {"child one", 11, 0, XUI_CASCADER_ITEM_LEAF, NULL},
        {"child two", 21, 1, XUI_CASCADER_ITEM_LEAF, NULL}
    };
    int path[] = {1, 11};
    uint32_t base;
    int i;
    setup();
    desc.iSize = sizeof(desc); desc.arrItems = items; desc.iItemCount = 7;
    desc.arrSelectedValues = path; desc.iSelectedDepth = 2;
    desc.fPopupHeight = 128; desc.bClearable = 1;
    CHECK(xuiCascaderCreate(context, &w, &desc) == XUI_OK);
    attach(w);
    data = __xuiCascaderGetData(w);
    test_colors(w, data, colors, sizeof(colors) / sizeof(colors[0]), resolved);
    phase = "cascader owner states";
    CHECK(xuiCascaderSetButtonColors(w, 0x912345ffu, 0x923456ffu, 0x934567ffu) == XUI_OK);
    inline_color(w, "cascader.button.color", 0xa12345ffu);
    paint(); CHECK(has_color(0xa12345ffu));
    CHECK(xuiCascaderGetButtonColors(w, &base, NULL, NULL) == XUI_OK && base == 0x912345ffu);
    inline_color(w, "cascader.button.color", 0);
    paint(); CHECK(!has_color(0xa12345ffu) && !has_color(0x912345ffu));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(0x912345ffu));
    CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
    inline_color(w, "cascader.text.disabled_color", 0xa23456ffu);
    paint(); CHECK(has_color(0xa23456ffu));
    inline_color(w, "cascader.background.disabled_color", 0xa34567ffu);
    paint(); CHECK(has_color(0xa34567ffu));
    inline_color(w, "cascader.arrow.disabled_color", 0xa45678ffu);
    paint(); CHECK(has_color(0xa45678ffu));
    CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
    CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    inline_color(w, "cascader.button.hover_color", 0xa56789ffu);
    paint(); CHECK(has_color(0xa56789ffu));
    CHECK(xuiCascaderOpen(w) == XUI_OK);
    settle();
    inline_color(w, "cascader.button.open_color", 0xa6789affu);
    paint(); CHECK(has_color(0xa6789affu));
    __xuiCascaderSetActiveToItem(data, 1);
    data->iHoverColumn = 0; data->iHoverItem = 3;
    CHECK(__xuiCascaderApplyPopupStyle(w, data) == XUI_OK);
    settle();
    for (i = 15; i < (int)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint32_t value = 0xc12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(uint32_t*)((char*)data + colors[i].offset);
        phase = colors[i].key;
        inline_color(w, colors[i].key, value);
        CHECK(xuiUpdate(context, 0) == XUI_OK);
        CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        CHECK((data->pPanel->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        paint(); CHECK(has_color(value));
        CHECK(xuiCascaderIsOpen(w));
        inline_color(w, colors[i].key, 0);
        paint(); CHECK(!has_color(value));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        paint(); CHECK(has_color(before));
    }
    CHECK(xuiCascaderGetSelectedPath(w, path, 2) == 2 && path[0] == 1 && path[1] == 11);
    return finish();
}
