/* ch81 — TTF 字体加载 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 32.0f);
	if (ret != XGE_OK) return;

	xgeTextDraw(&font, "Hello XGE - TTF Font", 60.0f, 80.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeTextDraw(&font, "Segoe UI 32px", 60.0f, 140.0f, XGE_COLOR_RGBA(79, 216, 194, 255));

	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch81", argc, argv); }
