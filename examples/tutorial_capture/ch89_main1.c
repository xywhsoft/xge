/* ch89 — 字形绘制与装饰线 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	xge_text_shape_desc_t desc;
	xge_glyph_run_t run;
	xge_text_decoration_t decor[2];
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 36.0f);
	if (ret != XGE_OK) return;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = &font;
	desc.sText = "Underline Text";
	desc.iTextSize = -1;

	memset(&run, 0, sizeof(run));
	run.iSize = sizeof(run);
	ret = xgeTextShape(&desc, &run);
	if (ret != XGE_OK) { xgeFontFree(&font); return; }

	/* 下划线装饰 */
	memset(decor, 0, sizeof(decor));
	decor[0].iSize = sizeof(xge_text_decoration_t);
	decor[0].iType = XGE_TEXT_DECORATION_UNDERLINE;
	decor[0].iColor = XGE_COLOR_RGBA(255, 100, 100, 255);
	decor[0].iFlags = XGE_TEXT_DECORATION_USE_FONT_METRICS;
	decor[0].iStart = 0;
	decor[0].iEnd = -1;

	/* 删除线装饰 */
	decor[1].iSize = sizeof(xge_text_decoration_t);
	decor[1].iType = XGE_TEXT_DECORATION_STRIKE;
	decor[1].iColor = XGE_COLOR_RGBA(100, 200, 255, 255);
	decor[1].iFlags = XGE_TEXT_DECORATION_USE_FONT_METRICS;
	decor[1].iStart = 0;
	decor[1].iEnd = -1;

	xgeGlyphRunDrawDecorated(&run, 60.0f, 100.0f,
		XGE_COLOR_RGBA(255, 255, 255, 255), 0, decor, 2);

	xgeGlyphRunFree(&run);
	xgeTextDraw(&font, "Decorated: underline + strike", 60.0f, 180.0f, XGE_COLOR_RGBA(200, 200, 200, 255));
	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch89", argc, argv); }
