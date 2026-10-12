#include "xui_control_pixel_test.h"

static xui_rect_t g_src, g_dst;
static int g_draws;

static int draw_surface(xui_proxy proxy, xui_draw_context draw, xui_surface surface,
	xui_rect_t src, xui_rect_t dst, uint32_t color, uint32_t flags)
{
	g_src = src; g_dst = dst; g_draws++;
	return XUI_OK;
}

static int draw_path(xui_proxy proxy, xui_draw_context draw, const char* path,
	xui_rect_t viewbox, xui_rect_t dst, const xui_path_style_t* style, float tolerance)
{
	g_src = viewbox; g_dst = dst; g_draws++;
	return XUI_OK;
}

static void configure(xui_proxy proxy)
{
	proxy->drawSurface = draw_surface;
	proxy->drawSvgPath = draw_path;
}

static void fit(float* w, float* h, float sw, float sh, int mode)
{
	float scale;
	if (mode == XUI_IMAGE_STRETCH || mode == XUI_IMAGE_CUSTOM) return;
	scale = mode == XUI_IMAGE_COVER ? fmaxf(*w / sw, *h / sh) : fminf(*w / sw, *h / sh);
	if (mode == XUI_IMAGE_NATURAL) scale = 1;
	if (mode == XUI_IMAGE_SCALE_DOWN) scale = fminf(scale, 1);
	*w = sw * scale; *h = sh * scale;
}

static void image_cases(pixel_fixture_t* f, xui_surface surface)
{
	xui_widget image = NULL;
	xui_image_desc_t desc = {0};
	const int modes[] = {XUI_IMAGE_NATURAL, XUI_IMAGE_STRETCH, XUI_IMAGE_CONTAIN, XUI_IMAGE_COVER, XUI_IMAGE_SCALE_DOWN};
	const int aligns[] = {XUI_ALIGN_START, XUI_ALIGN_CENTER, XUI_ALIGN_END};
	const int sizes[] = {0, 1, 2, 17, 18, 31, 32};
	const float sources[][4] = {{0, 0, 19, 13}, {-1.5f, 0.5f, 7, 6.25f}, {0.49f, -0.51f, 10.51f, 4.51f}, {0.5f, 0.5f, 1, 1}};
	size_t si, wi, hi, mi, ax, ay;
	desc.iSize = sizeof(desc); desc.pSurface = surface;
	PIXEL_CHECK(xuiImageCreate(f->context, &image, &desc) == XUI_OK);
	if (!image) return;
	PIXEL_CHECK(xuiWidgetAddChild(f->root, image) == XUI_OK);
	for (si = 0; si < sizeof(sources) / sizeof(sources[0]); si++) {
		float sx = sources[si][0], sy = sources[si][1], sw = sources[si][2] - sx, sh = sources[si][3] - sy;
		xui_vec2_t measured = {0};
		PIXEL_CHECK(xuiImageSetSourceRect(image, sx, sy, sx + sw, sy + sh) == XUI_OK);
		pixel_rect(xuiImageGetSource(image), sx, sy, sw, sh);
		PIXEL_CHECK(xuiWidgetMeasureContent(image, (xui_vec2_t){640, 480}, &measured) == XUI_OK);
		PIXEL_CHECK(measured.fX == sw && measured.fY == sh);
		for (wi = 0; wi < sizeof(sizes) / sizeof(sizes[0]); wi++) for (hi = 0; hi < sizeof(sizes) / sizeof(sizes[0]); hi++) {
			PIXEL_CHECK(xuiWidgetSetRect(image, (xui_rect_t){-11, -13, sizes[wi], sizes[hi]}) == XUI_OK);
			for (mi = 0; mi < sizeof(modes) / sizeof(modes[0]); mi++) {
				PIXEL_CHECK(xuiImageSetMode(image, modes[mi]) == XUI_OK);
				for (ax = 0; ax < 3; ax++) for (ay = 0; ay < 3; ay++) {
					float w = (float)sizes[wi], h = (float)sizes[hi], x, y;
					PIXEL_CHECK(xuiImageSetAlign(image, aligns[ax], aligns[ay]) == XUI_OK);
					if (w == 0 || h == 0) { pixel_rect(xuiImageGetDrawRect(image), 0, 0, 0, 0); continue; }
					fit(&w, &h, sw, sh, modes[mi]);
					x = (sizes[wi] - w) * (ax == 1 ? 0.5f : (ax == 2 ? 1.0f : 0.0f));
					y = (sizes[hi] - h) * (ay == 1 ? 0.5f : (ay == 2 ? 1.0f : 0.0f));
					pixel_rect(xuiImageGetDrawRect(image), x, y, w, h);
					if (ax == 1 && ay == 1 && si < 3) {
						xui_rect_t dst = xuiImageGetDrawRect(image);
						g_draws = 0;
						pixel_paint(f, image, 0);
						PIXEL_CHECK(g_draws == (dst.fW > 0 && dst.fH > 0));
						if (g_draws) { pixel_rect(g_src, sx, sy, sw, sh); pixel_rect(g_dst, x, y, w, h); }
					}
				}
			}
		}
	}
	PIXEL_CHECK(xuiWidgetSetRect(image, (xui_rect_t){-7, -5, 32, 31}) == XUI_OK);
	PIXEL_CHECK(xuiWidgetSetPadding(image, (xui_thickness_t){3, 5, 1, 2}) == XUI_OK);
	PIXEL_CHECK(xuiImageClearSource(image) == XUI_OK);
	PIXEL_CHECK(xuiImageSetCustomRect(image, -3.5f, -7.5f, 10.25f, 12.25f) == XUI_OK);
	pixel_rect(xuiImageGetCustomRect(image), -3.5f, -7.5f, 13.75f, 19.75f);
	pixel_rect(xuiImageGetDrawRect(image), -0.5f, -2.5f, 13.75f, 19.75f);
	pixel_paint(f, image, 0);
	pixel_rect(g_dst, -0.5f, -2.5f, 13.75f, 19.75f);
	PIXEL_CHECK(xuiImageSetCustomRect(image, 1, 0, 0.9f, 1) == XUI_ERROR_INVALID_ARGUMENT);
	PIXEL_CHECK(xuiImageSetSourceRect(image, 1, 0, 0.9f, 1) == XUI_ERROR_INVALID_ARGUMENT);
	PIXEL_CHECK(xuiImageSetCustomRect(image, NAN, 0, 1, 1) == XUI_ERROR_INVALID_ARGUMENT);
	PIXEL_CHECK(xuiImageSetSourceRect(image, 0, 0, INFINITY, 1) == XUI_ERROR_INVALID_ARGUMENT);
	pixel_rect(xuiImageGetCustomRect(image), -3.5f, -7.5f, 13.75f, 19.75f);
}

static void icon_cases(pixel_fixture_t* f, xui_surface surface)
{
	xui_icon_category category = NULL;
	xui_icon icon = NULL, path = NULL, alias = NULL, svg = NULL;
	xui_icon_category_desc_t cd;
	xui_icon_draw_desc_t dd;
	xui_painter painter = NULL;
	const int modes[] = {XUI_IMAGE_NATURAL, XUI_IMAGE_STRETCH, XUI_IMAGE_CONTAIN, XUI_IMAGE_COVER, XUI_IMAGE_SCALE_DOWN};
	const int sizes[] = {1, 2, 17, 18, 31, 32};
	const float dpis[] = {1, 1.25f, 1.5f, 2, 1};
	const char svg_data[] = "<svg xmlns='http://www.w3.org/2000/svg' width='19' height='13'><rect width='19' height='13'/></svg>";
	int fixed, source;
	size_t wi, hi, mi, di;
	xuiIconCategoryDescDefault(&cd);
	cd.fWidth = 17.5f; cd.fHeight = 12.5f;
	PIXEL_CHECK(xuiIconCategoryCreate(f->context, "pixel", &cd, &category) == XUI_OK);
	PIXEL_CHECK(xuiIconAddSurface(category, "surface", surface, (xui_rect_t){0}, 0, NULL, &icon) == XUI_OK);
	PIXEL_CHECK(xuiIconAddSvgPath(category, "path", "M0 0 H19 V13 H0 Z", (xui_rect_t){0, 0, 19, 13}, NULL, NULL, &path) == XUI_OK);
	PIXEL_CHECK(xuiIconAddAlias(category, "alias", icon, NULL, &alias) == XUI_OK);
	PIXEL_CHECK(xuiPainterBegin(f->context, f->target, &painter) == XUI_OK);
	xuiIconDrawDescDefault(&dd);
	for (fixed = 0; fixed <= 1; fixed++) for (source = 0; source <= 1; source++) {
		dd.iFlags = (fixed ? 0 : XUI_ICON_DRAW_IGNORE_CATEGORY_SIZE) | (source ? XUI_ICON_DRAW_USE_SOURCE_SIZE : 0);
		for (wi = 0; wi < sizeof(sizes) / sizeof(sizes[0]); wi++) for (hi = 0; hi < sizeof(sizes) / sizeof(sizes[0]); hi++) {
			for (mi = 0; mi < sizeof(modes) / sizeof(modes[0]); mi++) {
				xui_icon icons[] = {icon, path, alias};
				float w = source ? 19.0f : (fixed ? 17.5f : (float)sizes[wi]);
				float h = source ? 13.0f : (fixed ? 12.5f : (float)sizes[hi]);
				float x, y;
				int nonempty;
				size_t ii;
				dd.iFitMode = modes[mi];
				fit(&w, &h, 19, 13, modes[mi]);
				x = -20 + (sizes[wi] - w) * 0.5f;
				y = -11 + (sizes[hi] - h) * 0.5f;
				nonempty = floorf(x + w + 0.5f) > floorf(x + 0.5f) && floorf(y + h + 0.5f) > floorf(y + 0.5f);
				for (ii = 0; ii < 3; ii++) {
					g_draws = 0;
					PIXEL_CHECK(xuiIconDraw(painter, icons[ii], (xui_rect_t){-20, -11, sizes[wi], sizes[hi]}, &dd) == XUI_OK);
					PIXEL_CHECK(g_draws == nonempty);
					if (nonempty) { pixel_rect(g_dst, x, y, w, h); pixel_rect(g_src, 0, 0, 19, 13); }
				}
			}
		}
	}
	PIXEL_CHECK(xuiPainterEnd(painter) == XUI_OK);
	cd.iSizeMode = XUI_ICON_SIZE_UNRESTRICTED;
	PIXEL_CHECK(xuiIconCategorySetDesc(category, &cd) == XUI_OK);
	PIXEL_CHECK(xuiIconAddSvgMemory(category, "svg", svg_data, sizeof(svg_data), NULL, &svg) == XUI_OK);
	PIXEL_CHECK(xuiIconPrepare(svg, 17.5f, 12.5f) == XUI_OK);
	for (di = 0; di < sizeof(dpis) / sizeof(dpis[0]); di++) {
		xui_vec2_t intrinsic = {0};
		int before = xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy);
		PIXEL_CHECK(xuiSetVirtualDpi(f->context, dpis[di]) == XUI_OK);
		PIXEL_CHECK(xuiIconGetIntrinsicSize(svg, &intrinsic) == XUI_OK);
		PIXEL_CHECK(intrinsic.fX == 17.5f && intrinsic.fY == 12.5f);
		PIXEL_CHECK(xuiIconPrepare(svg, 0, 0) == XUI_OK);
		PIXEL_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy) == before + (di != 0));
		before = xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy);
		PIXEL_CHECK(xuiIconPrepare(svg, 0, 0) == XUI_OK);
		PIXEL_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy) == before);
	}
	{
		int before = xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy);
		PIXEL_CHECK(xuiPainterBegin(f->context, f->target, &painter) == XUI_OK);
		xuiIconDrawDescDefault(&dd);
		dd.iFitMode = XUI_IMAGE_CONTAIN;
		g_draws = 0;
		PIXEL_CHECK(xuiIconDraw(painter, svg, (xui_rect_t){-20, -11, 1, 2}, &dd) == XUI_OK);
		PIXEL_CHECK(g_draws == 0 && xuiTestProxyGetSvgSurfaceLoadCount(&f->proxy) == before);
		PIXEL_CHECK(xuiIconDraw(painter, svg, (xui_rect_t){0, 0, 0, 2}, &dd) == XUI_ERROR_INVALID_ARGUMENT);
		PIXEL_CHECK(xuiPainterEnd(painter) == XUI_OK);
	}
}

int main(void)
{
	pixel_fixture_t f;
	xui_surface surface = NULL;
	if (!pixel_init_proxy(&f, configure)) return 1;
	PIXEL_CHECK(xuiTestSurfaceCreate(&f.proxy, &surface, 19, 13, 0) == XUI_OK);
	image_cases(&f, surface);
	icon_cases(&f, surface);
	pixel_cleanup(&f);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, surface);
	return pixel_result("xui_image_icon_pixel_test");
}
