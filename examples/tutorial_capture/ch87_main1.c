/* ch87 — 字形查询与光栅化 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	xge_glyph_metrics_t gm;
	xge_glyph_bitmap_t bmp;
	xge_texture_t tex;
	char buf[128];
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 48.0f);
	if (ret != XGE_OK) return;

	/* 查询字形 'A' 的度量 */
	ret = xgeFontGlyphGet(&font, 'A', &gm);
	if (ret == XGE_OK) {
		snprintf(buf, sizeof(buf), "Glyph A: adv=%.1f bbox=(%.0f,%.0f)-(%.0f,%.0f)",
			gm.fAdvanceX, gm.fX0, gm.fY0, gm.fX1, gm.fY1);
		xgeTextDraw(&font, buf, 60.0f, 60.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	}

	/* 光栅化字形 'A' 为位图并显示为纹理 */
	memset(&bmp, 0, sizeof(bmp));
	ret = xgeFontGlyphRasterize(&font, 'A', &bmp);
	if (ret == XGE_OK && bmp.pPixels) {
		memset(&tex, 0, sizeof(tex));
		xgeTextureCreateRGBA(&tex, bmp.iWidth, bmp.iHeight, bmp.pPixels);
		xgeDraw(&tex, 60.0f, 100.0f);
		xgeTextureFree(&tex);
		xgeGlyphBitmapFree(&bmp);
	}

	xgeTextDraw(&font, "Rasterized 'A' above", 60.0f, 220.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch87", argc, argv); }
