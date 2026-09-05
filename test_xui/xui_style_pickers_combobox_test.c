#include "xui_style_pickers_test.h"
#include "src/xui_combobox.c"

#define ROW(key, field) { "combobox." key, offsetof(xui_combobox_data_t, field) }
static const color_case colors[] = {
    ROW("text.color", iTextColor), ROW("text.disabled_color", iDisabledTextColor),
    ROW("background.color", iBackgroundColor), ROW("background.hover_color", iHoverBackgroundColor),
    ROW("background.open_color", iOpenBackgroundColor), ROW("background.disabled_color", iDisabledBackgroundColor),
    ROW("border.color", iBorderColor), ROW("border.hover_color", iHoverBorderColor), ROW("border.focus_color", iFocusBorderColor),
    ROW("arrow.color", iArrowColor), ROW("arrow.disabled_color", iDisabledArrowColor),
    ROW("button.color", iButtonColor), ROW("button.hover_color", iButtonHoverColor), ROW("button.open_color", iButtonOpenColor),
    ROW("popup.panel_color", iPopupPanelColor), ROW("popup.border_color", iPopupBorderColor),
    ROW("popup.shadow_color", iPopupShadowColor), ROW("popup.hover_color", iPopupHoverColor),
    ROW("popup.text_color", iPopupTextColor), ROW("popup.hover_text_color", iPopupHoverTextColor),
    ROW("popup.disabled_text_color", iPopupDisabledTextColor), ROW("popup.separator_color", iPopupSeparatorColor)
};
static uint32_t resolved(xui_widget w, size_t offset)
{
    xui_combobox_data_t r;
    __xuiComboBoxResolve(w, __xuiComboBoxGetData(w), &r);
    return *(uint32_t*)((char*)&r + offset);
}
int main(void)
{
    xui_widget w;
    xui_combobox_data_t* data;
    xui_combobox_desc_t desc = {0};
    xui_combobox_item_t items[4] = {0};
    xui_menu_colors_t menu;
    uint32_t base;
    int i;
    setup();
    items[0].sText = "one"; items[0].bEnabled = 1;
    items[1].sText = "disabled";
    items[2].bSeparator = 1;
    items[3].sText = "two"; items[3].bEnabled = 1;
    desc.iSize = sizeof(desc); desc.arrItemData = items; desc.iItemCount = 4; desc.iSelected = 0;
    CHECK(xuiComboBoxCreate(context, &w, &desc) == XUI_OK);
    attach(w);
    data = __xuiComboBoxGetData(w);
    test_colors(w, data, colors, sizeof(colors) / sizeof(colors[0]), resolved);
    test_owner_colors(w, data, colors, 14, xuiComboBoxOpen, xuiComboBoxClose);
    phase = "button paint and setter restoration";
    CHECK(xuiComboBoxSetButtonColors(w, 0x912345ffu, 0x923456ffu, 0x934567ffu) == XUI_OK);
    inline_color(w, "combobox.button.color", 0xa12345ffu);
    paint(); CHECK(has_color(0xa12345ffu));
    CHECK(xuiComboBoxGetButtonColors(w, &base, NULL, NULL) == XUI_OK && base == 0x912345ffu);
    inline_color(w, "combobox.button.color", 0);
    paint(); CHECK(!has_color(0x912345ffu) && !has_color(0xa12345ffu));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(0x912345ffu));
    CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    inline_color(w, "combobox.button.hover_color", 0xa23456ffu);
    paint(); CHECK(has_color(0xa23456ffu));
    phase = "live menu forwarding";
    CHECK(xuiComboBoxOpen(w) == XUI_OK);
    settle();
    inline_color(w, "combobox.button.open_color", 0xa34567ffu);
    paint(); CHECK(has_color(0xa34567ffu));
    for (i = 14; i < (int)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint32_t value = 0xb12345ffu + (uint32_t)i * 0x01000000u;
        phase = colors[i].key;
        if (strcmp(colors[i].key, "combobox.popup.hover_color") == 0 ||
            strcmp(colors[i].key, "combobox.popup.hover_text_color") == 0) {
            xui_rect_t rect = xuiWidgetGetWorldRect(data->pMenu);
            CHECK(xuiInputPointerMove(context, rect.fX + 10, rect.fY + 12, 0) == XUI_OK);
            CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
        }
        inline_color(w, colors[i].key, value);
        CHECK(xuiUpdate(context, 0) == XUI_OK);
        if (xuiStyleFindProperty(context, "menu.panel.color") != 0)
            CHECK((data->pMenu->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        paint();
        CHECK(xuiComboBoxIsOpen(w));
        CHECK(xuiMenuGetColors(data->pMenu, &menu) == XUI_OK);
        CHECK(has_color(value));
        inline_color(w, colors[i].key, 0);
        paint(); CHECK(!has_color(value));
        inline_color(w, colors[i].key, 0xabcdef00u);
        prepare_only(); CHECK(!has_color(value) && !has_color(0xabcdefffu));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        paint();
        CHECK(resolved(w, colors[i].offset) == *(uint32_t*)((char*)data + colors[i].offset));
    }
    test_live_token(w, "combobox.popup.panel_color", data->iPopupPanelColor);
    test_prepare_only(w, data->pMenu, "combobox.popup.panel_color", data->iPopupPanelColor);
    CHECK(xuiComboBoxClose(w) == XUI_OK);
    phase = "cached editable child";
    CHECK(xuiComboBoxSetMode(w, XUI_COMBOBOX_MODE_EDIT) == XUI_OK);
    settle();
    inline_color(w, "combobox.text.color", 0xe12345ffu);
    paint(); CHECK(has_color(0xe12345ffu));
    CHECK(xuiInputGetColors(data->pInput, NULL, &base, NULL, NULL) == XUI_OK && base == 0xe12345ffu);
    inline_color(w, "combobox.text.color", 0);
    paint(); CHECK(!has_color(0xe12345ffu));
    CHECK(xuiInputGetColors(data->pInput, NULL, &base, NULL, NULL) == XUI_OK && base == 0);
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(data->iTextColor));
    test_prepare_only(w, data->pInput, "combobox.text.color", data->iTextColor);
    CHECK(xuiComboBoxSetColors(w, 0x812345ffu, data->iDisabledTextColor, data->iBackgroundColor,
        data->iHoverBackgroundColor, data->iOpenBackgroundColor, data->iDisabledBackgroundColor) == XUI_OK);
    prepare_only(); CHECK(has_color(0x812345ffu));
    CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
    inline_color(w, "combobox.text.disabled_color", 0xe23456ffu);
    paint(); CHECK(has_color(0xe23456ffu));
    inline_color(w, "combobox.text.disabled_color", 0);
    paint(); CHECK(!has_color(0xe23456ffu));
    CHECK(xuiInputGetExtendedColors(data->pInput, NULL, &base, NULL, NULL, NULL, NULL, NULL) == XUI_OK && base == 0);
    inline_color(w, "combobox.border.color", 0);
    CHECK(__xuiComboBoxColorWithAlpha(resolved(w, offsetof(xui_combobox_data_t, iBorderColor)), 120) == 0);
    return finish();
}
