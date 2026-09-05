#include "xui_control_pixel_test.h"

typedef struct box_t { float x, y, w, h; } box_t;
static xui_surface g_icon, g_badge, g_patch;
static xui_rect_t g_icon_rect, g_badge_rect, g_text_rect;
static int g_icon_count, g_badge_count, g_text_count;
static int g_coverage[24][24];
static int g_patch_width, g_patch_height;

static int measure(xui_proxy proxy, xui_font font, const char* text, xui_vec2_t* size)
{
	*size = (xui_vec2_t){23.5f, 11.5f};
	return XUI_OK;
}

static int draw_text(xui_proxy proxy, xui_draw_context draw, xui_font font,
	const char* text, xui_rect_t rect, uint32_t color, uint32_t flags)
{
	g_text_rect = rect;
	g_text_count++;
	return XUI_OK;
}

static int draw_surface(xui_proxy proxy, xui_draw_context draw, xui_surface surface,
	xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
	int x, y;
	if (surface == g_icon) { g_icon_rect = dst; g_icon_count++; }
	if (surface == g_badge) { g_badge_rect = dst; g_badge_count++; }
	if (surface == g_patch) {
		PIXEL_CHECK(src.fX >= 3 && src.fY >= 5 && src.fX + src.fW <= 15 && src.fY + src.fH <= 15);
		PIXEL_CHECK(dst.fW > 0 && dst.fH > 0 && dst.fX >= 0 && dst.fY >= 0 &&
			dst.fX + dst.fW <= g_patch_width && dst.fY + dst.fH <= g_patch_height);
		for (y = dst.fY; y < dst.fY + dst.fH; y++) {
			for (x = dst.fX; x < dst.fX + dst.fW; x++) {
				if (x >= 0 && x < 24 && y >= 0 && y < 24) g_coverage[y][x]++;
			}
		}
	}
	return XUI_OK;
}

static void configure(xui_proxy proxy)
{
	proxy->textMeasure = measure;
	proxy->drawText = draw_text;
	proxy->drawSurface = draw_surface;
}

static void layout_case(pixel_fixture_t* f, xui_widget button, int w, int h, int placement, float size, int text)
{
	float gap = 0.5f, tw = 23.5f, th = 11.5f, gw, gh, x, y;
	box_t icon, label = {0}, group;
	int anchor;
	PIXEL_CHECK(xuiWidgetSetRect(button, (xui_rect_t){-5, -3, w, h}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetText(button, text ? "Label" : "") == XUI_OK);
	PIXEL_CHECK(xuiButtonSetIconLayout(button, placement, size, gap) == XUI_OK);
	size = fminf(size, fminf((float)w, (float)h));
	if (!text) {
		icon = (box_t){(w - size) * 0.5f, (h - size) * 0.5f, size, size};
		group = icon;
	} else if (placement == XUI_BUTTON_ICON_TOP || placement == XUI_BUTTON_ICON_BOTTOM) {
		tw = fminf(tw, (float)w);
		gw = fmaxf(tw, size); gh = fminf(size + gap + th, (float)h);
		th = fmaxf(0, gh - size - gap);
		x = (w - gw) * 0.5f; y = (h - gh) * 0.5f;
		icon = (box_t){x + (gw - size) * 0.5f, placement == XUI_BUTTON_ICON_BOTTOM ? y + th + gap : y, size, size};
		label = (box_t){x, placement == XUI_BUTTON_ICON_TOP ? y + size + gap : y, gw, th};
		group = (box_t){x, y, gw, gh};
	} else {
		tw = fmaxf(0, fminf(tw, w - size - gap));
		gw = size + gap + tw; gh = fminf(fmaxf(th, size), (float)h);
		x = (w - gw) * 0.5f; y = (h - gh) * 0.5f;
		icon = (box_t){placement == XUI_BUTTON_ICON_RIGHT ? x + tw + gap : x, y + (gh - size) * 0.5f, size, size};
		label = (box_t){placement == XUI_BUTTON_ICON_LEFT ? x + size + gap : x, y, tw, gh};
		group = (box_t){x, y, gw, gh};
	}
	for (anchor = XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT; anchor <= XUI_BUTTON_BADGE_TEXT_TOP_RIGHT; anchor++) {
		box_t a = group;
		if (anchor == XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT) a = (box_t){0, 0, (float)w, (float)h};
		if (anchor == XUI_BUTTON_BADGE_ICON_TOP_RIGHT) a = icon;
		if (anchor == XUI_BUTTON_BADGE_TEXT_TOP_RIGHT) a = text && label.w > 0 ? label : (text ? group : (box_t){0, 0, (float)w, (float)h});
		PIXEL_CHECK(xuiButtonSetBadgeAnchor(button, anchor) == XUI_OK);
		g_icon_count = g_badge_count = g_text_count = 0;
		pixel_paint(f, button, 0);
		PIXEL_CHECK(g_icon_count == 1 && g_badge_count == 1 && g_text_count == text);
		pixel_rect(g_icon_rect, icon.x, icon.y, icon.w, icon.h);
		if (text) pixel_rect(g_text_rect, label.x, label.y, label.w, label.h);
		pixel_rect(g_badge_rect, a.x + a.w - 7.5f - 40.5f, a.y - 0.5f, 7.5f, 7.5f);
	}
}

int main(void)
{
	pixel_fixture_t f;
	xui_widget button = NULL;
	xui_button_desc_t desc = {0};
	xui_nine_patch_t patch = {0};
	const int placements[] = {XUI_BUTTON_ICON_LEFT, XUI_BUTTON_ICON_RIGHT, XUI_BUTTON_ICON_TOP, XUI_BUTTON_ICON_BOTTOM};
	const float sizes[] = {1.5f, 15.5f, 16.0f};
	int w, h, text, mode, x, y;
	size_t pi, si;
	if (!pixel_init_proxy(&f, configure)) return 1;
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &g_icon, 19, 17, 0) == XUI_OK);
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &g_badge, 9, 9, 0) == XUI_OK);
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &g_patch, 19, 19, 0) == XUI_OK);
	desc.iSize = sizeof(desc);
	PIXEL_CHECK(xuiButtonCreate(f.context, &button, &desc) == XUI_OK);
	if (!button) { pixel_cleanup(&f); return 1; }
	PIXEL_CHECK(xuiWidgetAddChild(f.root, button) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetPadding(button, (xui_thickness_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetIcon(button, g_icon, (xui_rect_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeSurface(button, g_badge, (xui_rect_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeVisible(button, 1) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeSize(button, 7.5f) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeOffset(button, -40.5f, -0.5f) == XUI_OK);
	for (w = 31; w <= 32; w++) for (h = 17; h <= 18; h++) {
		for (pi = 0; pi < sizeof(placements) / sizeof(placements[0]); pi++) {
			for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
				for (text = 0; text <= 1; text++) layout_case(&f, button, w, h, placements[pi], sizes[si], text);
			}
		}
	}
	PIXEL_CHECK(xuiButtonSetIcon(button, NULL, (xui_rect_t){0}) == XUI_OK);
	PIXEL_CHECK(xuiButtonSetText(button, "") == XUI_OK);
	PIXEL_CHECK(xuiButtonSetBadgeVisible(button, 0) == XUI_OK);
	patch.iSize = sizeof(patch); patch.pSurface = g_patch;
	patch.tSrc = (xui_rect_t){3, 5, 12, 10};
	patch.tSlice = (xui_thickness_t){4, 3, 5, 4};
	patch.iColor = XUI_COLOR_WHITE;
	for (mode = XUI_NINE_PATCH_STRETCH; mode <= XUI_NINE_PATCH_TILE; mode++) {
		patch.iMode = mode;
		PIXEL_CHECK(xuiButtonSetPatch(button, 0, &patch) == XUI_OK);
		for (w = 1; w <= 23; w++) for (h = 1; h <= 23; h++) {
			g_patch_width = w; g_patch_height = h;
			memset(g_coverage, 0, sizeof(g_coverage));
			PIXEL_CHECK(xuiWidgetSetRect(button, (xui_rect_t){-3, -5, w, h}) == XUI_OK);
			pixel_paint(&f, button, 0);
			for (y = 0; y < h; y++) for (x = 0; x < w; x++) PIXEL_CHECK(g_coverage[y][x] == 1);
		}
	}
	pixel_cleanup(&f);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, g_icon);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, g_badge);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, g_patch);
	return pixel_result("xui_button_pixel_test");
}
