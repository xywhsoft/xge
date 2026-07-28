/* ch82 — XRF 位图字体 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	int ret;

	/* 先加载 TTF，再导出 XRF 并重新加载 */
	ret = xgeFontLoad(&font, FONT_PATH, 28.0f);
	if (ret != XGE_OK) return;
	xgeFontSaveXRF(&font, "ch82_test.xrf", 0x20, 95);
	xgeFontFree(&font);

	/* 从 XRF 加载位图字体 */
	ret = xgeFontLoadXRF(&font, "ch82_test.xrf");
	if (ret != XGE_OK) return;

	xgeTextDraw(&font, "XRF Bitmap Font", 60.0f, 80.0f, XGE_COLOR_RGBA(255, 200, 80, 255));
	xgeTextDraw(&font, "Pre-rasterized glyphs", 60.0f, 130.0f, XGE_COLOR_RGBA(200, 220, 255, 255));

	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch82", argc, argv); }
