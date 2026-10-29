#include "xui_control_pixel_test.h"

int main(void)
{
	pixel_fixture_t f;
	xui_widget checkbox = NULL, radio = NULL, toggle = NULL;
	xui_checkbox_desc_t cd = {0};
	xui_radio_desc_t rd = {0};
	xui_toggle_desc_t td = {0};
	const int widths[] = {0, 1, 2, 63, 64, 101};
	const int heights[] = {0, 1, 2, 17, 18, 19, 33, 34};
	const float sizes[] = {0.5f, 1.5f, 17.0f, 17.5f, 18.5f};
	size_t wi, hi, si;
	int checked;
	if (!pixel_init(&f)) return 1;
	cd.iSize = sizeof(cd); cd.sText = "Label";
	rd.iSize = sizeof(rd); rd.sText = "Label";
	td.iSize = sizeof(td); td.sText = "Label";
	PIXEL_CHECK(xuiCheckBoxCreate(f.context, &checkbox, &cd) == XUI_OK);
	PIXEL_CHECK(xuiRadioCreate(f.context, &radio, &rd) == XUI_OK);
	PIXEL_CHECK(xuiToggleCreate(f.context, &toggle, &td) == XUI_OK);
	if (!checkbox || !radio || !toggle) { pixel_cleanup(&f); return 1; }
	PIXEL_CHECK(xuiWidgetAddChild(f.root, checkbox) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddChild(f.root, radio) == XUI_OK);
	PIXEL_CHECK(xuiWidgetAddChild(f.root, toggle) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetPadding(checkbox, (xui_thickness_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetPadding(radio, (xui_thickness_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetPadding(toggle, (xui_thickness_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiCheckBoxSetGap(checkbox, 0.5f) == XUI_OK);
	PIXEL_CHECK(xuiRadioSetGap(radio, 0.5f) == XUI_OK);
	PIXEL_CHECK(xuiToggleSetGap(toggle, 0.5f) == XUI_OK);
	PIXEL_CHECK(xuiToggleSetTrackSize(toggle, 64.5f, 22.5f) == XUI_OK);
	PIXEL_CHECK(xuiToggleSetThumbSize(toggle, 13.5f) == XUI_OK);
	PIXEL_CHECK(xuiToggleSetInnerText(toggle, "0", "1") == XUI_OK);
	PIXEL_CHECK(xuiToggleSetInnerTextMetrics(toggle, 2.5f, 1.5f) == XUI_OK);
	for (wi = 0; wi < sizeof(widths) / sizeof(widths[0]); wi++) {
		for (hi = 0; hi < sizeof(heights) / sizeof(heights[0]); hi++) {
			float w = (float)widths[wi], h = (float)heights[hi];
			xui_rect_t rect = {-3, -5, widths[wi], heights[hi]};
			PIXEL_CHECK(xuiWidgetSetRect(checkbox, rect) == XUI_OK);
			PIXEL_CHECK(xuiWidgetSetRect(radio, rect) == XUI_OK);
			PIXEL_CHECK(xuiWidgetSetRect(toggle, rect) == XUI_OK);
			for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
				float size = fminf(w, fminf(h, fmaxf(1.0f, sizes[si])));
				float available = w - size;
				float gap = fminf(0.5f, available);
				PIXEL_CHECK(xuiCheckBoxSetIndicatorSize(checkbox, sizes[si]) == XUI_OK);
				PIXEL_CHECK(xuiRadioSetIndicatorSize(radio, sizes[si]) == XUI_OK);
				pixel_paint(&f, checkbox, 0);
				pixel_paint(&f, radio, 0);
				pixel_rect(xuiCheckBoxGetIndicatorRect(checkbox), 0, (h - size) * 0.5f, size, size);
				pixel_rect(xuiRadioGetIndicatorRect(radio), 0, (h - size) * 0.5f, size, size);
				pixel_rect(xuiCheckBoxGetTextRect(checkbox), size + gap, 0, available - gap, h);
				pixel_rect(xuiRadioGetTextRect(radio), size + gap, 0, available - gap, h);
			}
			for (checked = 0; checked <= 1; checked++) {
				float tw = fminf(64.5f, w), th = fminf(22.5f, h);
				float maxThumb = fminf(tw, th);
				float limitedThumb = fmaxf(1, th - 4);
				float thumb;
				float maxInset;
				float inset;
				float ty = (h - th) * 0.5f;
				float tx;
				float innerStart;
				float innerEnd;
				float innerAvailable;
				float innerPadding;
				float innerGap;
				float ix;
				float iw;
				float available;
				float gap;
				if (maxThumb > limitedThumb) maxThumb = limitedThumb;
				if (maxThumb < 0) maxThumb = 0;
				thumb = fminf(13.5f, maxThumb);
				inset = fmaxf(1, (th - thumb) * 0.5f);
				maxInset = fmaxf(0, (tw - thumb) * 0.5f);
				if (inset > maxInset) inset = maxInset;
				tx = checked ? tw - inset - thumb : inset;
				innerStart = checked ? 0 : tx + thumb;
				innerEnd = checked ? tx : tw;
				innerAvailable = fmaxf(0, innerEnd - innerStart);
				innerPadding = fminf(2.5f, innerAvailable);
				innerAvailable -= innerPadding;
				innerGap = fminf(1.5f, innerAvailable);
				ix = checked ? innerStart + innerPadding : innerStart + innerGap;
				iw = checked ? innerEnd - innerGap - ix : innerEnd - innerPadding - ix;
				available = w - tw;
				gap = fminf(0.5f, available);
				PIXEL_CHECK(xuiToggleSetChecked(toggle, checked) == XUI_OK);
				pixel_paint(&f, toggle, checked ? XUI_TOGGLE_STATE_CHECKED : 0);
				pixel_rect(xuiToggleGetTrackRect(toggle), 0, ty, tw, th);
				pixel_rect(xuiToggleGetThumbRect(toggle), tx, ty + (th - thumb) * 0.5f, thumb, thumb);
				pixel_rect(xuiToggleGetInnerTextRect(toggle), ix, ty, iw, th);
				pixel_rect(xuiToggleGetTextRect(toggle), tw + gap, 0, available - gap, h);
			}
		}
	}
	pixel_cleanup(&f);
	return pixel_result("xui_indicator_pixel_test");
}
