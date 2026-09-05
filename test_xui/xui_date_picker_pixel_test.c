#include "xui_control_pixel_test.h"

typedef struct box_t { float x, y, w, h; } box_t;
static xui_rect_t g_text[256];
static int g_text_count;

static int draw_text(xui_proxy proxy, xui_draw_context draw, xui_font font, const char* text,
	xui_rect_t rect, uint32_t color, uint32_t flags)
{
	if (text && text[0] && g_text_count < 256) g_text[g_text_count++] = rect;
	return XUI_OK;
}

static void configure(xui_proxy proxy) { proxy->drawText = draw_text; }

static xui_rect_t rounded(box_t r)
{
	int x = (int)floorf(r.x + 0.5f), y = (int)floorf(r.y + 0.5f);
	int right = (int)floorf(r.x + r.w + 0.5f), bottom = (int)floorf(r.y + r.h + 0.5f);
	return (xui_rect_t){x, y, right > x ? right - x : 0, bottom > y ? bottom - y : 0};
}

static void calendar(xui_widget picker, int panel, box_t r)
{
	int i;
	double grid_h;
	r.w = fmaxf(0, r.w); r.h = fmaxf(0, r.h);
	pixel_rect(xuiDatePickerGetCalendarPanelRect(picker, panel), r.x, r.y, r.w, r.h);
	pixel_rect(xuiDatePickerGetPrevRect(picker, panel), r.x + 2, r.y + 2, 28, 28);
	pixel_rect(xuiDatePickerGetNextRect(picker, panel), r.x + r.w - 30, r.y + 2, 28, 28);
	grid_h = fmax(120, (double)r.h - 54);
	for (i = 0; i < 42; i++) {
		xui_rect_t cell = xuiDatePickerGetDayRect(picker, panel, i);
		float x = (float)(r.x + (double)r.w * (i % 7) / 7);
		float right = (float)(r.x + (double)r.w * (i % 7 + 1) / 7);
		float y = (float)(r.y + 54 + grid_h * (i / 7) / 6);
		float bottom = (float)(r.y + 54 + grid_h * (i / 7 + 1) / 6);
		pixel_rect(cell, x, y, right - x, bottom - y);
		if (i % 7 != 6) PIXEL_CHECK(cell.fX + cell.fW == xuiDatePickerGetDayRect(picker, panel, i + 1).fX);
		if (i < 35) PIXEL_CHECK(cell.fY + cell.fH == xuiDatePickerGetDayRect(picker, panel, i + 7).fY);
	}
	PIXEL_CHECK(xuiDatePickerGetDayRect(picker, panel, 6).fX + xuiDatePickerGetDayRect(picker, panel, 6).fW == rounded(r).fX + rounded(r).fW);
}

static void time_panel(xui_widget picker, int panel, box_t r, int second)
{
	int i, count = second ? 3 : 2;
	float inner, fw, x, y;
	r.w = fmaxf(0, r.w); r.h = fmaxf(0, r.h);
	pixel_rect(xuiDatePickerGetTimePanelRect(picker, panel), r.x, r.y, r.w, r.h);
	inner = fmaxf((float)count, r.w - 24);
	fw = fminf(72, fmaxf(1, (inner - 8 * (count - 1)) / count));
	x = r.x + 12 + (inner - (fw * count + 8 * (count - 1))) * 0.5f;
	y = fminf(r.y + r.h - 36, fmaxf(r.y + 44, r.y + (r.h - 46) * 0.5f + 18));
	for (i = 0; i < count; i++) pixel_rect(xuiDatePickerGetTimeRect(picker, panel, i), x + i * (fw + 8), y, fw, 28);
	if (!second) pixel_rect(xuiDatePickerGetTimeRect(picker, panel, 2), 0, 0, 0, 0);
}

static void panel_case(xui_widget picker, int mode, float w, float h, int second)
{
	float col = (w - 36) * 0.5f, footer = h - 40;
	int range = mode >= XUI_DATE_PICKER_MODE_DATE_RANGE;
	int date = mode != XUI_DATE_PICKER_MODE_TIME && mode != XUI_DATE_PICKER_MODE_TIME_RANGE;
	int time = mode != XUI_DATE_PICKER_MODE_DATE && mode != XUI_DATE_PICKER_MODE_DATE_RANGE;
	int i;
	PIXEL_CHECK(xuiDatePickerSetPopupSize(picker, w, h) == XUI_OK);
	PIXEL_CHECK(xuiDatePickerSetShowSecond(picker, second) == XUI_OK);
	for (i = 0; i < (range ? 2 : 1); i++) {
		if (date && time && range) {
			calendar(picker, i, (box_t){12 + i * (col + 12), 12, col, 238});
			time_panel(picker, i, (box_t){12 + i * (col + 12), 256, col, fmaxf(84, footer - 24 - 238)}, second);
		} else if (date && time) {
			float calw = fminf(292, w * 0.58f);
			calendar(picker, i, (box_t){12, 12, calw, footer - 24});
			time_panel(picker, i, (box_t){24 + calw, 70, w - calw - 36, 96}, second);
		} else if (date) {
			calendar(picker, i, (box_t){range ? 12 + i * (col + 12) : 12, 12, range ? col : w - 24, footer - 24});
		} else {
			time_panel(picker, i, (box_t){range ? 12 + i * (col + 12) : 12, 24, range ? col : w - 24, footer - 24}, second);
		}
		if (!date) pixel_rect(xuiDatePickerGetCalendarPanelRect(picker, i), 0, 0, 0, 0);
		if (!time) pixel_rect(xuiDatePickerGetTimePanelRect(picker, i), 0, 0, 0, 0);
	}
	pixel_rect(xuiDatePickerGetFooterRect(picker, XUI_DATE_PICKER_FOOTER_OK), w - 78, footer, 66, 28);
	pixel_rect(xuiDatePickerGetFooterRect(picker, XUI_DATE_PICKER_FOOTER_CANCEL), w - 152, footer, 66, 28);
	pixel_rect(xuiDatePickerGetFooterRect(picker, XUI_DATE_PICKER_FOOTER_CLEAR), w - 226, footer, 66, 28);
	pixel_rect(xuiDatePickerGetFooterRect(picker, XUI_DATE_PICKER_FOOTER_TODAY), 12, footer, 78, 28);
}

static void popup_case(pixel_fixture_t* f, xui_widget picker, float width)
{
	xui_widget panel;
	xui_rect_t fields[2], cal, world, drop;
	float cw = width - 24, combo = fmaxf(104, fminf(180, cw - 68));
	float year = combo < 180 ? (combo - 6) * 0.64f : 112;
	float cx = 12 + (cw - combo) * 0.5f;
	int kind, i;
	PIXEL_CHECK(xuiDatePickerSetMode(picker, XUI_DATE_PICKER_MODE_DATE) == XUI_OK);
	PIXEL_CHECK(xuiDatePickerSetPopupSize(picker, width, 301.5f) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetRect(picker, (xui_rect_t){20, 12, 210, 31}) == XUI_OK);
	PIXEL_CHECK(xuiLayout(f->context) == XUI_OK);
	PIXEL_CHECK(xuiDatePickerOpen(picker) == XUI_OK);
	PIXEL_CHECK(xuiLayout(f->context) == XUI_OK);
	panel = xuiDatePickerGetPanelWidget(picker);
	PIXEL_CHECK(panel != NULL);
	if (!panel) return;
	fields[0] = rounded((box_t){cx, 16, year, 24});
	fields[1] = rounded((box_t){cx + year + 6, 16, combo - year - 6, 24});
	cal = xuiDatePickerGetCalendarPanelRect(picker, 0);
	g_text_count = 0;
	pixel_paint(f, panel, 0);
	PIXEL_CHECK(g_text_count >= 2);
	for (i = 0; i < 2; i++) pixel_rect(g_text[i], (float)fields[i].fX + 6, (float)fields[i].fY, (float)fields[i].fW - 28, (float)fields[i].fH);
	for (kind = 0; kind < 2; kind++) {
		int count = kind ? 12 : 9;
		world = xuiWidgetGetWorldRect(panel);
		PIXEL_CHECK(xuiInputPointerDown(f->context, world.fX + fields[kind].fX + fields[kind].fW / 2,
			world.fY + fields[kind].fY + 12, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
		PIXEL_CHECK(xuiDispatchPendingEvents(f->context) == XUI_OK);
		PIXEL_CHECK(xuiInputPointerUp(f->context, world.fX + fields[kind].fX + fields[kind].fW / 2,
			world.fY + fields[kind].fY + 12, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
		PIXEL_CHECK(xuiDispatchPendingEvents(f->context) == XUI_OK);
		g_text_count = 0;
		pixel_paint(f, panel, 0);
		PIXEL_CHECK(g_text_count >= 55 + count);
		drop = (xui_rect_t){kind ? fields[1].fX + fields[1].fW - 144 : fields[0].fX, 42, kind ? 144 : 112, kind ? 96 : 198};
		if (drop.fX < cal.fX + 2) drop.fX = cal.fX + 2;
		if (drop.fX + drop.fW > cal.fX + cal.fW - 2) drop.fX = cal.fX + cal.fW - drop.fW - 2;
		for (i = 0; g_text_count >= count && i < count; i++) {
			pixel_rect(g_text[g_text_count - count + i], (float)drop.fX + (kind ? (i % 3) * 48 : 0) + 2,
				(float)drop.fY + (kind ? (i / 3) * 24 : i * 22) + 2, kind ? 44.0f : 108.0f, kind ? 20.0f : 18.0f);
		}
		PIXEL_CHECK(xuiInputKeyDown(f->context, XUI_KEY_ESCAPE, 0) == XUI_OK);
		PIXEL_CHECK(xuiDispatchPendingEvents(f->context) == XUI_OK);
	}
	PIXEL_CHECK(xuiDatePickerClose(picker) == XUI_OK);
	PIXEL_CHECK(xuiDatePickerSetPopupSize(picker, width, 301.5f) == XUI_OK);
	pixel_rect(xuiWidgetGetRect(panel), 0, 0, width, 301.5f);
}

int main(void)
{
	pixel_fixture_t f;
	xui_widget picker = NULL;
	xui_date_picker_desc_t desc = {0};
	const float widths[] = {0.5f, 1.5f, 239, 240, 241.5f, 319, 320, 621.5f};
	const float heights[] = {0.5f, 1.5f, 299, 300, 301.5f, 423.5f};
	int mode, second, w;
	size_t wi, hi;
	if (!pixel_init_proxy(&f, configure)) return 1;
	desc.iSize = sizeof(desc); desc.bNullable = 1; desc.pFont = f.font;
	PIXEL_CHECK(xuiDatePickerCreate(f.context, &picker, &desc) == XUI_OK);
	if (!picker) { pixel_cleanup(&f); return 1; }
	PIXEL_CHECK(xuiWidgetAddChild(f.root, picker) == XUI_OK);
	for (w = 0; w <= 35; w++) {
		PIXEL_CHECK(xuiWidgetSetRect(picker, (xui_rect_t){-11, -13, w, 31}) == XUI_OK);
		pixel_rect(xuiDatePickerGetButtonRect(picker), (float)w - 31, 0, 31, 31);
		pixel_rect(xuiDatePickerGetTextRect(picker), 8, 0, fmaxf(1, (float)w - 45), 31);
	}
	for (mode = XUI_DATE_PICKER_MODE_DATE; mode <= XUI_DATE_PICKER_MODE_DATETIME_RANGE; mode++) {
		PIXEL_CHECK(xuiDatePickerSetMode(picker, mode) == XUI_OK);
		for (wi = 0; wi < sizeof(widths) / sizeof(widths[0]); wi++) for (hi = 0; hi < sizeof(heights) / sizeof(heights[0]); hi++) {
			for (second = 0; second <= 1; second++) panel_case(picker, mode, widths[wi], heights[hi], second);
		}
	}
	popup_case(&f, picker, 239.5f);
	popup_case(&f, picker, 320.5f);
	pixel_cleanup(&f);
	return pixel_result("xui_date_picker_pixel_test");
}
