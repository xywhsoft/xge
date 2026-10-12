#include "xui_style_basic_test.h"

static void choice_cases(pixel_fixture_t* f, xui_widget w, int radio)
{
	const uint32_t checked = XUI_CHECKBOX_STATE_CHECKED;
	int fill = radio ? BASIC_CIRCLE : BASIC_FILL;
	int border = radio ? BASIC_RING : BASIC_STROKE;
	basic_case(f, w, "choice.background.color", 0, fill);
	basic_case(f, w, "choice.background.hover_color", XUI_WIDGET_STATE_HOVER, fill);
	basic_case(f, w, "choice.background.active_color", XUI_WIDGET_STATE_ACTIVE, fill);
	basic_case(f, w, "choice.background.disabled_color", XUI_WIDGET_STATE_DISABLED, fill);
	basic_case(f, w, "choice.border.color", 0, border);
	basic_case(f, w, "choice.border.hover_color", XUI_WIDGET_STATE_HOVER, border);
	basic_case(f, w, "choice.border.disabled_color", XUI_WIDGET_STATE_DISABLED, border);
	basic_case(f, w, "choice.accent.color", checked, fill);
	basic_case(f, w, "choice.accent.hover_color", checked | XUI_WIDGET_STATE_HOVER, fill);
	basic_case(f, w, "choice.accent.active_color", checked | XUI_WIDGET_STATE_ACTIVE, fill);
	basic_case(f, w, "choice.focus.color", XUI_WIDGET_STATE_FOCUS, border);
	basic_case(f, w, radio ? "radio.dot.color" : "checkbox.check.color", checked, radio ? BASIC_CIRCLE : BASIC_LINE);
	basic_case(f, w, radio ? "radio.dot.disabled_color" : "checkbox.check.disabled_color", checked | XUI_WIDGET_STATE_DISABLED, radio ? BASIC_CIRCLE : BASIC_LINE);
	basic_case(f, w, "choice.text.color", 0, BASIC_TEXT);
	basic_case(f, w, "choice.text.disabled_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	basic_case(f, w, "text.color", 0, BASIC_TEXT);
	basic_case(f, w, "text.disabled_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	basic_cached_case(f, w, "choice.background.color", fill);
}

int main(void)
{
	pixel_fixture_t f;
	xui_widget checkbox = NULL, radio = NULL, toggle = NULL;
	xui_checkbox_desc_t cd = {0}; xui_radio_desc_t rd = {0}; xui_toggle_desc_t td = {0};
	const char* keys[] = {"toggle.track.color", "toggle.track.hover_color", "toggle.track.active_color",
		"toggle.track.border_color", "toggle.thumb.color", "toggle.thumb.border_color", "toggle.track.disabled_color",
		"toggle.track.disabled_border_color", "toggle.thumb.disabled_color", "toggle.thumb.disabled_border_color"};
	const uint32_t states[] = {0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE, 0, 0, 0,
		XUI_WIDGET_STATE_DISABLED, XUI_WIDGET_STATE_DISABLED, XUI_WIDGET_STATE_DISABLED, XUI_WIDGET_STATE_DISABLED};
	const int kinds[] = {BASIC_FILL, BASIC_FILL, BASIC_FILL, BASIC_STROKE, BASIC_FILL, BASIC_STROKE,
		BASIC_FILL, BASIC_STROKE, BASIC_FILL, BASIC_STROKE};
	size_t i;
	if (!basic_init(&f)) return 1;
	cd.iSize = sizeof(cd); cd.sText = "Check"; cd.iTextColor = 0x115533ffu;
	rd.iSize = sizeof(rd); rd.sText = "Radio"; rd.iTextColor = 0x335511ffu;
	td.iSize = sizeof(td); td.sText = "Toggle"; td.iTextColor = 0x553311ffu;
	PIXEL_CHECK(xuiCheckBoxCreate(f.context, &checkbox, &cd) == XUI_OK);
	PIXEL_CHECK(xuiRadioCreate(f.context, &radio, &rd) == XUI_OK);
	PIXEL_CHECK(xuiToggleCreate(f.context, &toggle, &td) == XUI_OK);
	basic_attach(&f, checkbox); basic_attach(&f, radio); basic_attach(&f, toggle);
	choice_cases(&f, checkbox, 0); choice_cases(&f, radio, 1);
	for (i = 0; i < sizeof(keys) / sizeof(*keys); ++i) basic_case(&f, toggle, keys[i], states[i], kinds[i]);
	basic_case(&f, toggle, "choice.focus.color", XUI_WIDGET_STATE_FOCUS, BASIC_STROKE);
	basic_case(&f, toggle, "choice.accent.color", XUI_TOGGLE_STATE_CHECKED, BASIC_FILL);
	basic_case(&f, toggle, "choice.accent.hover_color", XUI_TOGGLE_STATE_CHECKED | XUI_WIDGET_STATE_HOVER, BASIC_FILL);
	basic_case(&f, toggle, "choice.accent.active_color", XUI_TOGGLE_STATE_CHECKED | XUI_WIDGET_STATE_ACTIVE, BASIC_FILL);
	basic_case(&f, toggle, "choice.text.color", 0, BASIC_TEXT);
	basic_case(&f, toggle, "text.disabled_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	PIXEL_CHECK(xuiToggleSetInnerText(toggle, "Off", "On") == XUI_OK);
	PIXEL_CHECK(xuiToggleSetTrackSize(toggle, 90, 24) == XUI_OK);
	basic_case(&f, toggle, "toggle.inner_text.unchecked_color", 0, BASIC_TEXT);
	basic_case(&f, toggle, "toggle.inner_text.checked_color", XUI_TOGGLE_STATE_CHECKED, BASIC_TEXT);
	basic_case(&f, toggle, "choice.text.disabled_color", XUI_WIDGET_STATE_DISABLED, BASIC_TEXT);
	basic_cached_case(&f, toggle, "toggle.track.color", BASIC_FILL);
	pixel_cleanup(&f);
	return pixel_result("xui_style_basic_choices_test");
}
