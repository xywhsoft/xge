#include "xui_style_basic_test.h"

static void decoration_click(xui_widget w, xui_input_decoration d, void* user)
{ (void)w; (void)d; (void)user; }

static void control_event(xui_widget w, int type, xui_rect_t local)
{
	xui_widget_event_proc proc = NULL;
	void* user = NULL;
	xui_event_t e = {0};
	xui_rect_t world = xuiWidgetGetWorldRect(w);
	e.iType = type; e.iButton = XUI_POINTER_BUTTON_LEFT;
	e.fX = world.fX + local.fX + local.fW / 2; e.fY = world.fY + local.fY + local.fH / 2;
	PIXEL_CHECK(xuiWidgetGetEventHandler(w, type, &proc, &user) == XUI_OK && proc != NULL);
	if (proc) (void)proc(w, &e, user);
}

static void pointer(pixel_fixture_t* f, int type, float x, float y)
{
	int ret;
	if (type == XUI_EVENT_POINTER_DOWN) ret = xuiInputPointerDown(f->context, (int)x, (int)y, XUI_POINTER_BUTTON_LEFT, 1);
	else if (type == XUI_EVENT_POINTER_UP) ret = xuiInputPointerUp(f->context, (int)x, (int)y, XUI_POINTER_BUTTON_LEFT, 0);
	else ret = xuiInputPointerMove(f->context, (int)x, (int)y, 0);
	PIXEL_CHECK(ret == XUI_OK);
	PIXEL_CHECK(xuiUpdate(f->context, 0.0f) == XUI_OK);
}

static void input_cases(pixel_fixture_t* f)
{
	xui_widget w = NULL;
	xui_input_desc_t d = {0};
	xui_input_decoration_desc_t dd = {0};
	xui_input_decoration decoration = NULL;
	xui_style_property_t p;
	xui_rect_t r, world;
	uint32_t color;
	d.iSize = sizeof(d); d.sText = "input"; d.sPlaceholder = "placeholder"; d.iTextColor = 0x246813ffu;
	PIXEL_CHECK(xuiInputCreate(f->context, &w, &d) == XUI_OK); basic_attach(f, w);
	basic_case(f, w, "input.text.color", 0, BASIC_TEXT);
	basic_case(f, w, "text.color", 0, BASIC_TEXT);
	basic_case(f, w, "input.background.color", 0, BASIC_FILL);
	basic_case(f, w, "input.border.color", 0, BASIC_STROKE);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
	basic_case(f, w, "input.background.disabled_color", 0, BASIC_FILL);
	basic_case(f, w, "input.border.disabled_color", 0, BASIC_STROKE);
	basic_case(f, w, "input.text.disabled_color", 0, BASIC_TEXT);
	basic_case(f, w, "text.disabled_color", 0, BASIC_TEXT);
	p = basic_color("input.border.color", 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_paint(f, w, 0); PIXEL_CHECK(basic_kind_count(BASIC_STROKE) == 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
	PIXEL_CHECK(xuiInputSetError(w, 1) == XUI_OK);
	basic_case(f, w, "input.error.background_color", 0, BASIC_FILL);
	basic_case(f, w, "input.error.border_color", 0, BASIC_STROKE);
	PIXEL_CHECK(xuiInputSetError(w, 0) == XUI_OK);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, w) == XUI_OK);
	basic_case(f, w, "input.border.focus_color", 0, BASIC_STROKE);
	PIXEL_CHECK(xuiInputSetSelection(w, 0, 3) == XUI_OK);
	basic_case(f, w, "input.selection.color", 0, BASIC_FILL);
	PIXEL_CHECK(xuiInputSetSelection(w, 0, 0) == XUI_OK);
	basic_case(f, w, "input.cursor.color", 0, BASIC_FILL);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, NULL) == XUI_OK);
	PIXEL_CHECK(xuiInputSetText(w, "") == XUI_OK);
	basic_case(f, w, "input.placeholder.color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiInputSetText(w, "input") == XUI_OK);
	basic_cached_case(f, w, "input.text.color", BASIC_TEXT);
	dd.iSize = sizeof(dd); dd.iKind = XUI_INPUT_DECORATION_TEXT; dd.sText = "@"; dd.fWidth = 24; dd.onClick = decoration_click;
	PIXEL_CHECK(xuiInputDecorationAdd(w, XUI_INPUT_DECORATION_SIDE_LEADING, &decoration, &dd) == XUI_OK);
	basic_case(f, w, "input.decoration.color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
	basic_case(f, w, "input.decoration.disabled_color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
	basic_render(f); basic_paint(f, w, 0);
	r = xuiInputDecorationGetRect(w, decoration); world = xuiWidgetGetWorldRect(w);
	pointer(f, XUI_EVENT_POINTER_MOVE, world.fX + r.fX + r.fW / 2, world.fY + r.fY + r.fH / 2);
	basic_case(f, w, "input.background.hover_color", 0, BASIC_FILL);
	basic_case(f, w, "input.border.hover_color", 0, BASIC_STROKE);
	control_event(w, XUI_EVENT_POINTER_MOVE, xuiInputDecorationGetRect(w, decoration));
	basic_case(f, w, "input.decoration.hover_color", 0, BASIC_TEXT);
	control_event(w, XUI_EVENT_POINTER_DOWN, xuiInputDecorationGetRect(w, decoration));
	basic_case(f, w, "input.decoration.active_color", 0, BASIC_TEXT);
	control_event(w, XUI_EVENT_POINTER_UP, xuiInputDecorationGetRect(w, decoration));
	pointer(f, XUI_EVENT_POINTER_MOVE, 500, 400);
	dd.iColor = 0xabcdef78u;
	dd.iHoverColor = dd.iActiveColor = dd.iDisabledColor = dd.iColor;
	PIXEL_CHECK(xuiInputDecorationSet(w, decoration, &dd) == XUI_OK);
	p = basic_color("input.decoration.color", 0x12345678u);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_paint(f, w, 0); PIXEL_CHECK(basic_seen(BASIC_TEXT, dd.iColor) > 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	/* The vector glyph path uses SVG stroke color, not text or atlas tint. */
	dd.iKind = XUI_INPUT_DECORATION_ICON; dd.iIcon = XUI_INPUT_ICON_SEARCH;
	dd.iColor = dd.iHoverColor = dd.iActiveColor = dd.iDisabledColor = 0;
	PIXEL_CHECK(xuiInputDecorationSet(w, decoration, &dd) == XUI_OK);
	control_event(w, XUI_EVENT_POINTER_LEAVE, (xui_rect_t){0});
	basic_case(f, w, "input.decoration.color", 0, BASIC_SVG_STROKE);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
	basic_case(f, w, "input.decoration.disabled_color", 0, BASIC_SVG_STROKE);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
	control_event(w, XUI_EVENT_POINTER_MOVE, xuiInputDecorationGetRect(w, decoration));
	basic_case(f, w, "input.decoration.hover_color", 0, BASIC_SVG_STROKE);
	control_event(w, XUI_EVENT_POINTER_DOWN, xuiInputDecorationGetRect(w, decoration));
	basic_case(f, w, "input.decoration.active_color", 0, BASIC_SVG_STROKE);
	control_event(w, XUI_EVENT_POINTER_UP, xuiInputDecorationGetRect(w, decoration));
	control_event(w, XUI_EVENT_POINTER_LEAVE, (xui_rect_t){0});
	basic_cached_case(f, w, "input.decoration.color", BASIC_SVG_STROKE);
	PIXEL_CHECK(xuiInputGetColors(w, NULL, &color, NULL, NULL) == XUI_OK && color == d.iTextColor);
	xuiWidgetDestroy(w);
}

static void numeric_cases(pixel_fixture_t* f)
{
	xui_widget w = NULL, input;
	xui_numeric_input_desc_t d = {0};
	xui_style_property_t p;
	uint32_t color;
	d.iSize = sizeof(d); d.fValue = 12; d.fMin = -100; d.fMax = 100; d.bSpinnerVisible = 1;
	d.sPlaceholder = "number"; d.iTextColor = 0x336699ffu;
	PIXEL_CHECK(xuiNumericInputCreate(f->context, &w, &d) == XUI_OK); basic_attach(f, w);
	input = xuiNumericInputGetInputWidget(w); PIXEL_CHECK(input != NULL);
	basic_render(f);
	basic_case(f, w, "numeric_input.background.color", 0, BASIC_FILL);
	basic_case(f, w, "numeric_input.border.color", 0, BASIC_STROKE);
	basic_case(f, w, "numeric_input.spinner.color", 0, BASIC_FILL);
	basic_case(f, w, "numeric_input.spinner.border_color", 0, BASIC_LINE);
	basic_case(f, w, "numeric_input.spinner.icon_color", 0, BASIC_TRIANGLE);
	control_event(w, XUI_EVENT_POINTER_MOVE, xuiNumericInputGetButtonRect(w, XUI_NUMERIC_INPUT_BUTTON_UP));
	basic_case(f, w, "numeric_input.background.hover_color", 0, BASIC_FILL);
	basic_case(f, w, "numeric_input.border.hover_color", 0, BASIC_STROKE);
	basic_case(f, w, "numeric_input.spinner.hover_color", 0, BASIC_FILL);
	control_event(w, XUI_EVENT_POINTER_DOWN, xuiNumericInputGetButtonRect(w, XUI_NUMERIC_INPUT_BUTTON_UP));
	basic_case(f, w, "numeric_input.spinner.active_color", 0, BASIC_FILL);
	control_event(w, XUI_EVENT_POINTER_UP, xuiNumericInputGetButtonRect(w, XUI_NUMERIC_INPUT_BUTTON_UP));
	control_event(w, XUI_EVENT_POINTER_LEAVE, (xui_rect_t){0});
	PIXEL_CHECK(xuiSetFocusWidget(f->context, NULL) == XUI_OK);
	basic_case_on(f, w, input, "numeric_input.text.color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 0) == XUI_OK);
	basic_case(f, w, "numeric_input.background.disabled_color", 0, BASIC_FILL);
	basic_case(f, w, "numeric_input.border.disabled_color", 0, BASIC_STROKE);
	basic_case(f, w, "input.border.disabled_color", 0, BASIC_STROKE);
	basic_case(f, w, "numeric_input.spinner.icon_disabled_color", 0, BASIC_TRIANGLE);
	basic_case_on(f, w, input, "numeric_input.text.disabled_color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, input) == XUI_OK);
	basic_case(f, w, "numeric_input.border.focus_color", 0, BASIC_STROKE);
	PIXEL_CHECK(xuiInputSetSelection(input, 0, 2) == XUI_OK);
	basic_case_on(f, w, input, "numeric_input.selection.color", 0, BASIC_FILL);
	PIXEL_CHECK(xuiInputSetSelection(input, 0, 0) == XUI_OK);
	basic_case_on(f, w, input, "numeric_input.cursor.color", 0, BASIC_FILL);
	PIXEL_CHECK(xuiSetFocusWidget(f->context, NULL) == XUI_OK);
	PIXEL_CHECK(xuiInputSetText(input, "") == XUI_OK);
	basic_case_on(f, w, input, "numeric_input.placeholder.color", 0, BASIC_TEXT);
	PIXEL_CHECK(xuiInputSetText(input, "12") == XUI_OK);
	basic_cached_case(f, w, "numeric_input.text.color", BASIC_TEXT);
	basic_render(f); basic_render(f);
	p = basic_color("numeric_input.text.color", 0x98765432u);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(BASIC_TEXT, 0x98765432u) > 0);
	PIXEL_CHECK(xuiInputGetColors(input, NULL, &color, NULL, NULL) == XUI_OK && color == d.iTextColor);
	p.tValue.iColor = 0;
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, &p, 1) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(BASIC_TEXT, d.iTextColor) == 0);
	PIXEL_CHECK(xuiWidgetSetInlineStyle(w, NULL, 0) == XUI_OK);
	basic_render(f); PIXEL_CHECK(basic_seen(BASIC_TEXT, d.iTextColor) > 0);
	p = basic_color("input.background.color", 0xabcdef55u);
	PIXEL_CHECK(xuiStyleSetDefault(f->context, &p, 1) == XUI_OK);
	basic_paint(f, input, 0); PIXEL_CHECK(basic_seen(BASIC_FILL, 0xabcdef55u) == 0);
	PIXEL_CHECK(xuiStyleClearDefault(f->context) == XUI_OK);
	PIXEL_CHECK(xuiNumericInputSetText(w, "bad") == XUI_OK);
	(void)xuiNumericInputCommit(w);
	PIXEL_CHECK(xuiNumericInputGetError(w));
	basic_case(f, w, "numeric_input.error.background_color", 0, BASIC_FILL);
	basic_case(f, w, "numeric_input.error.border_color", 0, BASIC_STROKE);
	xuiWidgetDestroy(w);
}

int main(void)
{
	pixel_fixture_t f;
	if (!basic_init(&f)) return 1;
	input_cases(&f); numeric_cases(&f);
	pixel_cleanup(&f);
	return pixel_result("xui_style_basic_input_test");
}
