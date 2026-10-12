#include "xui_control_pixel_test.h"

static xui_rect_t g_body;
static int g_body_count;
#define TRACK_COLOR XUI_COLOR_RGBA(11, 22, 33, 255)

static int draw_rect(xui_proxy proxy, xui_draw_context draw, xui_rect_t rect, uint32_t color)
{
	if (color == TRACK_COLOR) { g_body = rect; g_body_count++; }
	return XUI_OK;
}

static void configure(xui_proxy proxy)
{
	proxy->drawRectFill = draw_rect;
}

static void check_knob(xui_rect_t rect, float w, float h, float length, float origin,
	float rate, float knob, int vertical)
{
	float size = fminf(knob, fminf(w, h));
	float center = origin + length * (vertical ? 1 - rate : rate);
	float pos = fmaxf(0, fminf(center - size * 0.5f, (vertical ? h : w) - size));
	if (w <= 0 || h <= 0) pixel_rect(rect, 0, 0, 0, 0);
	else if (vertical) pixel_rect(rect, (w - size) * 0.5f, pos, size, size);
	else pixel_rect(rect, pos, (h - size) * 0.5f, size, size);
}

int main(void)
{
	pixel_fixture_t f;
	xui_widget slider = NULL, range = NULL;
	xui_slider_desc_t sd = {0};
	xui_range_slider_desc_t rd = {0};
	const int lengths[] = {0, 1, 2, 17, 18, 99, 100};
	const int crosses[] = {0, 1, 2, 17, 18, 33, 34};
	const float knobs[] = {1.5f, 14.5f, 15.0f};
	const float rates[] = {0, 0.125f, 0.5f, 0.875f, 1};
	size_t li, ci, ki, ri;
	int vertical;
	if (!pixel_init_proxy(&f, configure)) return 1;
	sd.iSize = sizeof(sd); sd.fMax = 1;
	rd.iSize = sizeof(rd); rd.fMax = 1; rd.fEnd = 1;
	PIXEL_CHECK(xuiSliderCreate(f.context, &slider, &sd) == XUI_OK);
	PIXEL_CHECK(xuiRangeSliderCreate(f.context, &range, &rd) == XUI_OK);
	if (!slider || !range) { pixel_cleanup(&f); return 1; }
	PIXEL_CHECK(xuiWidgetAddChild(f.root, slider) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddChild(f.root, range) == XUI_OK);
	PIXEL_CHECK(xuiSliderSetColors(slider, TRACK_COLOR, XUI_COLOR_WHITE, XUI_COLOR_WHITE, XUI_COLOR_WHITE, XUI_COLOR_WHITE) == XUI_OK);
	for (vertical = 0; vertical <= 1; vertical++) {
		int orientation = vertical ? XUI_ORIENTATION_VERTICAL : XUI_ORIENTATION_HORIZONTAL;
		PIXEL_CHECK(xuiSliderSetOrientation(slider, orientation) == XUI_OK);
		PIXEL_CHECK(xuiRangeSliderSetOrientation(range, orientation) == XUI_OK);
		for (li = 0; li < sizeof(lengths) / sizeof(lengths[0]); li++) {
			for (ci = 0; ci < sizeof(crosses) / sizeof(crosses[0]); ci++) {
				int iw = vertical ? crosses[ci] : lengths[li], ih = vertical ? lengths[li] : crosses[ci];
				float w = (float)iw, h = (float)ih;
				PIXEL_CHECK(xuiWidgetSetRect(slider, (xui_rect_t){-5, -3, iw, ih}) == XUI_OK);
				PIXEL_CHECK(xuiWidgetSetRect(range, (xui_rect_t){-5, -3, iw, ih}) == XUI_OK);
				for (ki = 0; ki < sizeof(knobs) / sizeof(knobs[0]); ki++) {
					float axis = (float)lengths[li], cross = (float)crosses[ci];
					float inset = fminf(knobs[ki], axis) * 0.5f;
					float length = fmaxf(1, axis - inset * 2);
					float origin = (axis - length) * 0.5f;
					float thickness = fminf(4.5f, cross), offset = (cross - thickness) * 0.5f;
					PIXEL_CHECK(xuiSliderSetMetrics(slider, 4.5f, knobs[ki], 1.5f) == XUI_OK);
					PIXEL_CHECK(xuiRangeSliderSetMetrics(range, 4.5f, knobs[ki]) == XUI_OK);
					for (ri = 0; ri < sizeof(rates) / sizeof(rates[0]); ri++) {
						float a = fminf(rates[ri], 1 - rates[ri]), b = fmaxf(rates[ri], 1 - rates[ri]);
						xui_rect_t track;
						int slider_changes, range_changes;
						PIXEL_CHECK(xuiSliderSetValue(slider, rates[ri]) == XUI_OK);
						PIXEL_CHECK(xuiRangeSliderSetValues(range, a, b) == XUI_OK);
						slider_changes = xuiSliderGetChangeCount(slider);
						range_changes = xuiRangeSliderGetChangeCount(range);
						track = xuiSliderGetTrackRect(slider);
						if (vertical) {
							pixel_rect(track, offset, origin, thickness, length);
							pixel_rect(xuiRangeSliderGetTrackRect(range), offset, origin, thickness, length);
							pixel_rect(xuiSliderGetFillRect(slider), offset, origin + length * (1 - rates[ri]), thickness, length * rates[ri]);
							pixel_rect(xuiRangeSliderGetFillRect(range), offset, origin + length * (1 - b), thickness, length * (b - a));
						} else {
							pixel_rect(track, origin, offset, length, thickness);
							pixel_rect(xuiRangeSliderGetTrackRect(range), origin, offset, length, thickness);
							pixel_rect(xuiSliderGetFillRect(slider), origin, offset, length * rates[ri], thickness);
							pixel_rect(xuiRangeSliderGetFillRect(range), origin + length * a, offset, length * (b - a), thickness);
						}
						check_knob(xuiSliderGetKnobRect(slider), w, h, length, origin, rates[ri], knobs[ki], vertical);
						check_knob(xuiRangeSliderGetStartKnobRect(range), w, h, length, origin, a, knobs[ki], vertical);
						check_knob(xuiRangeSliderGetEndKnobRect(range), w, h, length, origin, b, knobs[ki], vertical);
						PIXEL_CHECK(xuiSliderGetValue(slider) == rates[ri]);
						PIXEL_CHECK(xuiRangeSliderGetStart(range) == a && xuiRangeSliderGetEnd(range) == b);
						if (ri == 0 && track.fW > 3 && track.fH > 3) {
							g_body_count = 0;
							pixel_paint(&f, slider, 0);
							PIXEL_CHECK(g_body_count == 1);
							if (track.fW >= track.fH) pixel_rect(g_body, track.fX + 1.5f, (float)track.fY, track.fW - 3.0f, (float)track.fH);
							else pixel_rect(g_body, (float)track.fX, track.fY + 1.5f, (float)track.fW, track.fH - 3.0f);
						}
						PIXEL_CHECK(xuiSliderGetChangeCount(slider) == slider_changes && xuiRangeSliderGetChangeCount(range) == range_changes);
					}
				}
			}
		}
	}
	pixel_cleanup(&f);
	return pixel_result("xui_slider_pixel_test");
}
