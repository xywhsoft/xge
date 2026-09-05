#include "xui_style_pickers_test.h"
#include "src/xui_date_picker.c"
#define ROW(key, field) { "datepicker." key, offsetof(xui_date_picker_data_t, field) }
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
    ROW("popup.panel_color", iPopupPanelColor),
    ROW("popup.border_color", iPopupBorderColor),
    ROW("popup.shadow_color", iPopupShadowColor),
    ROW("popup.text_color", iPopupTextColor),
    ROW("popup.muted_text_color", iPopupMutedTextColor),
    ROW("accent.color", iAccentColor),
    ROW("field.color", iFieldColor),
    ROW("field.border_color", iFieldBorderColor),
    ROW("day.selected_text_color", iSelectedTextColor),
    ROW("day.disabled_color", iDisabledDayColor),
    ROW("separator.color", iSeparatorColor),
    ROW("button.color", iButtonColor),
    ROW("button.hover_color", iButtonHoverColor),
    ROW("button.open_color", iButtonOpenColor),
    ROW("button.disabled_color", iButtonDisabledColor),
    ROW("calendar.color", iCalendarColor),
    ROW("time.color", iTimePanelColor),
    ROW("day.range_color", iRangeColor),
    ROW("selection.color", iSelectionColor),
    ROW("field.error_border_color", iErrorBorderColor),
    ROW("option.hover_color", iOptionHoverColor),
};
static uint32_t resolved(xui_widget w, size_t offset)
{
    xui_date_picker_data_t r;
    __xuiDatePickerResolve(w, __xuiDatePickerGetData(w), &r);
    return *(uint32_t*)((char*)&r + offset);
}
int main(void)
{
    xui_widget w;
    xui_date_picker_data_t* data;
    xui_date_picker_desc_t desc = {0};
    xtime start = 0, end = 0, actual_start = 0, actual_end = 0;
    int i;
    setup();
    CHECK(xrtDate(2026, 1, 10, &start) != 0);
    end = start + 5 * XRT_TIME_DAY;
    desc.iSize = sizeof(desc); desc.iMode = XUI_DATE_PICKER_MODE_DATETIME_RANGE;
    desc.bHasRangeValue = 1; desc.tStart = start; desc.tEnd = end;
    desc.bHasMin = 1; desc.bHasMax = 1;
    desc.tMin = start - 2 * XRT_TIME_DAY; desc.tMax = end + 10 * XRT_TIME_DAY;
    CHECK(xuiDatePickerCreate(context, &w, &desc) == XUI_OK);
    attach(w);
    data = __xuiDatePickerGetData(w);
    test_colors(w, data, colors, sizeof(colors) / sizeof(colors[0]), resolved);
    phase = "datepicker button states";
    inline_color(w, "datepicker.button.color", 0xa12345ffu);
    paint(); CHECK(has_color(0xa12345ffu));
    inline_color(w, "datepicker.button.color", 0);
    paint(); CHECK(!has_color(0xa12345ffu) && !has_color(data->iButtonColor));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(data->iButtonColor));
    CHECK(xuiInputPointerMove(context, 40, 30, 0) == XUI_OK);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    inline_color(w, "datepicker.button.hover_color", 0xa23456ffu);
    paint(); CHECK(has_color(0xa23456ffu));
    CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
    inline_color(w, "datepicker.button.disabled_color", 0xa34567ffu);
    paint(); CHECK(has_color(0xa34567ffu));
    CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
    CHECK(xuiDatePickerOpen(w) == XUI_OK);
    settle();
    inline_color(w, "datepicker.button.open_color", 0xa45678ffu);
    paint(); CHECK(has_color(0xa45678ffu));
    __xuiDatePickerBeginYearEdit(data, 0);
    data->iEditError = 1;
    data->iOpenComboKind = XUI_DATE_PICKER_COMBO_YEAR; data->iOpenComboPanel = 0;
    data->iHoverPart = XUI_DATE_PICKER_PART_YEAR_OPTION;
    data->iHoverPanel = 0; data->iHoverIndex = 0;
    settle();
    for (i = 11; i < (int)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint32_t value = 0xc12345ffu + (uint32_t)i * 0x00010000u;
        uint32_t before = *(uint32_t*)((char*)data + colors[i].offset);
        if (i >= 22 && i <= 25) continue;
        phase = colors[i].key;
        inline_color(w, colors[i].key, value);
        CHECK(xuiUpdate(context, 0) == XUI_OK);
        CHECK((w->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        CHECK((data->pPanel->iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0);
        paint(); CHECK(has_color(value));
        CHECK(xuiDatePickerIsOpen(w));
        inline_color(w, colors[i].key, 0);
        paint(); CHECK(!has_color(value));
        CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
        paint(); CHECK(has_color(before));
    }
    phase = "month options and time selection";
    data->iOpenComboKind = XUI_DATE_PICKER_COMBO_MONTH;
    data->iHoverPart = XUI_DATE_PICKER_PART_MONTH_OPTION; data->iHoverIndex = 1;
    inline_color(w, "datepicker.option.hover_color", 0xe12345ffu);
    paint(); CHECK(has_color(0xe12345ffu));
    data->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
    __xuiDatePickerBeginTimeEdit(data, 0, 0);
    inline_color(w, "datepicker.selection.color", 0xe23456ffu);
    paint(); CHECK(has_color(0xe23456ffu));
    phase = "setter under active override";
    inline_color(w, "datepicker.field.color", 0xe34567ffu);
    CHECK(xuiDatePickerSetPopupColors(w, data->iPopupPanelColor, data->iPopupBorderColor,
        data->iPopupShadowColor, data->iPopupTextColor, data->iPopupMutedTextColor,
        data->iAccentColor, 0xe45678ffu, data->iFieldBorderColor,
        data->iSelectedTextColor, data->iDisabledDayColor, data->iSeparatorColor) == XUI_OK);
    paint(); CHECK(has_color(0xe34567ffu) && !has_color(0xe45678ffu));
    CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
    paint(); CHECK(has_color(0xe45678ffu));
    CHECK(xuiDatePickerGetRangeValue(w, &actual_start, &actual_end) == XUI_OK);
    CHECK(actual_start == start && actual_end == end);
    return finish();
}
