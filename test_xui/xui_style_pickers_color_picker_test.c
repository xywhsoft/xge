#include "xui_style_pickers_test.h"
#include "src/xui_color_picker.c"
#define ROW(key, field) { "colorpicker." key, offsetof(xui_color_picker_data_t, field) }
static const color_case colors[] = {
    ROW("text.color", iTextColor),
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
    ROW("popup.text_color", iPopupTextColor),
    ROW("popup.muted_text_color", iPopupMutedTextColor),
    ROW("accent.color", iAccentColor),
    ROW("field.color", iFieldColor),
    ROW("field.border_color", iFieldBorderColor),
    ROW("separator.color", iSeparatorColor),
    ROW("field.error_border_color", iErrorBorderColor),
    ROW("selection.color", iSelectionColor),
    ROW("selection.text_color", iSelectionTextColor),
    ROW("track.color", iTrackColor),
    ROW("knob.color", iKnobColor),
    ROW("marker.color", iMarkerColor),
    ROW("marker.border_color", iMarkerBorderColor),
    ROW("checker.light_color", iCheckerLightColor),
    ROW("checker.dark_color", iCheckerDarkColor),
};
static uint32_t resolved(xui_widget w, size_t offset)
{
    xui_color_picker_data_t r;
    __xuiColorPickerResolve(w, __xuiColorPickerGetData(w), &r);
    return *(uint32_t*)((char*)&r + offset);
}
int main(void)
{
    xui_widget w;
    xui_color_picker_data_t* data;
    xui_color_picker_desc_t desc = {0};
    uint32_t palette[] = {0x891245ffu, 0x12345680u, 0x1267ab00u};
    uint32_t base;
    int i;
    setup();
    desc.iSize = sizeof(desc); desc.bAlphaEnabled = 1; desc.iColor = 0x1267ab00u;
    CHECK(xuiColorPickerCreate(context, &w, &desc) == XUI_OK);
    CHECK(xuiColorPickerSetPalette(w, palette, 3) == XUI_OK);
    attach(w);
    data = __xuiColorPickerGetData(w);
    test_colors(w, data, colors, sizeof(colors) / sizeof(colors[0]), resolved);
    phase = "colorpicker button states";
    CHECK(xuiColorPickerSetButtonColors(w, 0x912345ffu, 0x923456ffu, 0x934567ffu) == XUI_OK);
    inline_color(w, "colorpicker.button.color", 0xa12345ffu);
    paint(); CHECK(has_color(0xa12345ffu));
    CHECK(xuiColorPickerGetButtonColors(w, &base, NULL, NULL) == XUI_OK && base == 0x912345ffu);
    inline_color(w, "colorpicker.button.color", 0);
    paint(); CHECK(!has_color(0xa12345ffu) && !has_color(0x912345ffu));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(0x912345ffu));
    CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    inline_color(w, "colorpicker.button.hover_color", 0xa23456ffu);
    paint(); CHECK(has_color(0xa23456ffu));
    CHECK(xuiColorPickerOpen(w) == XUI_OK);
    settle();
    inline_color(w, "colorpicker.button.open_color", 0xa34567ffu);
    paint(); CHECK(has_color(0xa34567ffu));
    __xuiColorPickerBeginHexEdit(w, data);
    data->bEditError = 1;
    settle();
    for (i = 14; i < (int)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint32_t value = 0xc12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(uint32_t*)((char*)data + colors[i].offset);
        phase = colors[i].key;
        inline_color(w, colors[i].key, value);
        CHECK(xuiUpdate(context, 0) == XUI_OK);
        CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        CHECK((data->pPanel->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        paint(); CHECK(has_color(value));
        CHECK(xuiColorPickerIsOpen(w));
        CHECK(xuiColorPickerGetColor(w) == 0x1267ab00u);
        CHECK(has_color(palette[0]) && has_color(palette[1]));
        inline_color(w, colors[i].key, 0);
        paint(); CHECK(!has_color(value));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        paint(); CHECK(has_color(before));
    }
    phase = "channel edit error and selection";
    __xuiColorPickerCancelEdit(data);
    __xuiColorPickerBeginChannelEdit(data, 0);
    data->bEditError = 1;
    inline_color(w, "colorpicker.field.error_border_color", 0xe12345ffu);
    paint(); CHECK(has_color(0xe12345ffu));
    inline_color(w, "colorpicker.selection.text_color", 0xe23456ffu);
    paint(); CHECK(has_color(0xe23456ffu));
    phase = "checker alpha and explicit color content";
    {
        xui_style_property_t p[] = {
            property("colorpicker.checker.light_color", 0),
            property("colorpicker.checker.dark_color", 0)
        };
        CHECK(xuiWidgetSetInlineStyle(w, p, 2) == XUI_OK);
        paint();
        CHECK(!has_color(data->iCheckerLightColor) && !has_color(data->iCheckerDarkColor));
        CHECK(has_color(palette[0]) && has_color(palette[1]));
        for (i = 0; i < 3; ++i) CHECK(xuiColorPickerGetPaletteColor(w, i) == palette[i]);
        CHECK(xuiColorPickerGetColor(w) == 0x1267ab00u);
    }
    return finish();
}
