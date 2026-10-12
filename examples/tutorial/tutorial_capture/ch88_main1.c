/* ch88 — 文本整形 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	xge_text_shape_desc_t desc;
	xge_glyph_run_t run;
	xge_vec2_t size;
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 32.0f);
	if (ret != XGE_OK) return;

	/* 文本整形：将字符串转换为字形序列 */
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = &font;
	desc.sText = "Text Shaping";
	desc.iTextSize = -1;
	desc.iFlags = 0;

	memset(&run, 0, sizeof(run));
	run.iSize = sizeof(run);
	ret = xgeTextShape(&desc, &run);
	if (ret == XGE_OK) {
		size = xgeGlyphRunMeasure(&run);
		xgeGlyphRunDraw(&run, 60.0f, 80.0f, XGE_COLOR_RGBA(255, 255, 255, 255), 0);

		/* 绘制测量范围框 */
		xgeShapeRectStroke(
			(xge_rect_t){60.0f, 80.0f - run.fAscent, size.fX, run.fAscent + run.fDescent},
			1.0f, XGE_COLOR_RGBA(79, 216, 194, 128));
		xgeGlyphRunFree(&run);
	}

	xgeTextDraw(&font, "Shaped glyph run with bounds", 60.0f, 160.0f, XGE_COLOR_RGBA(200, 200, 200, 255));
	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch88", argc, argv); }
