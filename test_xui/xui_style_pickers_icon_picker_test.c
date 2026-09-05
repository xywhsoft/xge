#include "xui_style_pickers_test.h"
#include "src/xui_icon_picker.c"
#define ROW(key, field) { "iconpicker." key, offsetof(xui_icon_picker_data_t, field) }
static const color_case colors[] = {
    ROW("text.color", iTextColor),
    ROW("placeholder.color", iPlaceholderColor),
    ROW("text.disabled_color", iDisabledTextColor),
    ROW("background.color", iBackgroundColor),
    ROW("background.hover_color", iHoverBackgroundColor),
    ROW("background.open_color", iOpenBackgroundColor),
    ROW("background.disabled_color", iDisabledBackgroundColor),
    ROW("border.color", iBorderColor),
    ROW("border.hover_color", iHoverBorderColor),
    ROW("border.focus_color", iFocusBorderColor),
    ROW("arrow.color", iArrowColor),
    ROW("arrow.disabled_color", iDisabledArrowColor),
    ROW("button.color", iButtonColor),
    ROW("button.hover_color", iButtonHoverColor),
    ROW("button.open_color", iButtonOpenColor),
    ROW("popup.panel_color", iPopupPanelColor),
    ROW("popup.border_color", iPopupBorderColor),
    ROW("popup.shadow_color", iPopupShadowColor),
    ROW("item.hover_color", iItemHoverColor),
    ROW("item.selected_color", iItemSelectedColor),
    ROW("item.focus_color", iItemFocusColor),
};
static uint32_t resolved(xui_widget w, size_t offset)
{
    xui_icon_picker_data_t r;
    __xuiIconPickerResolve(w, __xuiIconPickerGetData(w), &r);
    return *(uint32_t*)((char*)&r + offset);
}
int main(void)
{
    xui_widget w;
    xui_icon_picker_data_t* data;
    xui_icon_picker_desc_t desc = {0};
    xui_icon_category category;
    xui_icon icon;
    xui_icon_id selected = XUI_ICON_ID_INVALID;
    xui_icon_desc_t icon_desc;
    xui_path_style_t path_style = {0};
    int i;
    setup();
    CHECK(xuiIconCategoryCreate(context, "style-picker-icons", NULL, &category) == XUI_OK);
    xuiIconDescDefault(&icon_desc);
    path_style.iSize = sizeof(path_style); path_style.iFillColor = 0x893456ccu;
    for (i = 0; i < 12; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "icon%d", i);
        CHECK(xuiIconAddSvgPath(category, name, "M2 2 H22 V22 H2 Z",
            (xui_rect_t){0, 0, 24, 24}, &path_style, &icon_desc, &icon) == XUI_OK);
        if (i == 0) selected = xuiIconGetId(icon);
    }
    desc.iSize = sizeof(desc); desc.pCategory = category; desc.iSelectedId = selected;
    desc.iTextMode = XUI_ICON_PICKER_TEXT_ID;
    desc.iVisibleRows = 2; desc.iVisibleColumns = 3; desc.sPlaceholder = "choose";
    CHECK(xuiIconPickerCreate(context, &w, &desc) == XUI_OK);
    attach(w);
    data = __xuiIconPickerGetData(w);
    test_colors(w, data, colors, sizeof(colors) / sizeof(colors[0]), resolved);
    for (i = 0; i < 15; ++i) {
        uint32_t value = 0xc12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(uint32_t*)((char*)data + colors[i].offset);
        phase = colors[i].key;
        CHECK(xuiSetFocusWidget(context, NULL) == XUI_OK);
        CHECK(xuiInputPointerMove(context, 900, 700, 0) == XUI_OK);
        CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
        if (strstr(colors[i].key, "placeholder") != NULL) CHECK(xuiIconPickerClearSelection(w) == XUI_OK);
        if (strstr(colors[i].key, "disabled") != NULL) CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
        if (strstr(colors[i].key, "hover") != NULL) {
            CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
            CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
        }
        if (strstr(colors[i].key, "focus") != NULL) CHECK(xuiSetFocusWidget(context, w) == XUI_OK);
        if (strstr(colors[i].key, "open") != NULL) CHECK(xuiIconPickerOpen(w) == XUI_OK);
        inline_color(w, colors[i].key, value);
        prepare_only(); CHECK(cache_has_color(w, value));
        inline_color(w, colors[i].key, 0);
        prepare_only(); CHECK(!cache_has_color(w, value));
        inline_color(w, colors[i].key, 0xabcdef00u);
        prepare_only(); CHECK(!cache_has_color(w, value) && !cache_has_color(w, 0xabcdefffu));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        prepare_only(); CHECK(cache_has_color(w, before));
        CHECK(xuiIconPickerClose(w) == XUI_OK);
        CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
        CHECK(xuiIconPickerSetSelectedId(w, selected) == XUI_OK);
    }
    CHECK(xuiIconPickerOpen(w) == XUI_OK);
    CHECK(xuiSetFocusWidget(context, data->pViewport) == XUI_OK);
    data->iHoverIndex = 1; data->iFocusIndex = 2;
    settle();
    for (i = 15; i < (int)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint32_t value = 0xd12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(uint32_t*)((char*)data + colors[i].offset);
        phase = colors[i].key;
        inline_color(w, colors[i].key, value);
        CHECK(xuiUpdate(context, 0) == XUI_OK);
        CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        CHECK((data->pViewport->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        CHECK((data->pFrame->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        paint(); CHECK(has_color(value));
        CHECK(has_color(path_style.iFillColor));
        CHECK(xuiIconPickerIsOpen(w));
        if (i == 15) CHECK(xuiScrollFrameGetBackgroundColor(data->pFrame) == value);
        inline_color(w, colors[i].key, 0);
        paint(); CHECK(!has_color(value));
        if (i == 15) CHECK(xuiScrollFrameGetBackgroundColor(data->pFrame) == 0);
        inline_color(w, colors[i].key, 0xabcdef00u);
        prepare_only(); CHECK(!has_color(value) && !has_color(0xabcdefffu));
        if (i == 15) CHECK(xuiScrollFrameGetBackgroundColor(data->pFrame) == 0xabcdef00u);
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        paint(); CHECK(has_color(before));
        CHECK(xuiIconPickerGetSelectedId(w) == selected);
    }
    phase = "popup setter restores nested frame";
    inline_color(w, "iconpicker.popup.panel_color", 0xe12345ffu);
    CHECK(xuiIconPickerSetPopupColors(w, 0xe23456ffu, data->iPopupBorderColor,
        data->iPopupShadowColor, data->iItemHoverColor, data->iItemSelectedColor, data->iItemFocusColor) == XUI_OK);
    paint(); CHECK(has_color(0xe12345ffu) && !has_color(0xe23456ffu));
    CHECK(xuiScrollFrameGetBackgroundColor(data->pFrame) == 0xe12345ffu);
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(0xe23456ffu));
    CHECK(xuiScrollFrameGetBackgroundColor(data->pFrame) == 0xe23456ffu);
    test_live_token(w, "iconpicker.popup.panel_color", data->iPopupPanelColor);
    test_prepare_only(w, data->pViewport, "iconpicker.popup.panel_color", data->iPopupPanelColor);
    CHECK(xuiIconCategoryRelease(category) == XUI_OK);
    return finish();
}
