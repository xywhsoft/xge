/* ch85 — Fallback 链 */
#include "tut_capture.h"

#define FONT_PRIMARY  "C:/Windows/Fonts/segoeui.ttf"
#define FONT_FALLBACK "C:/Windows/Fonts/msyh.ttc"

static void draw_scene(void)
{
	xge_font_t font, fallback;
	int ret;

	ret = xgeFontLoad(&font, FONT_PRIMARY, 30.0f);
	if (ret != XGE_OK) return;

	/* 设置 fallback 字体（用于缺失字形） */
	ret = xgeFontLoad(&fallback, FONT_FALLBACK, 30.0f);
	if (ret == XGE_OK) {
		xgeFontSetFallback(&font, &fallback);
	}

	xgeTextDraw(&font, "Primary: Segoe UI", 60.0f, 80.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeTextDraw(&font, "Fallback: \xE5\xBE\xAE\xE8\xBD\xAF\xE9\x9B\x85\xE9\xBB\x91", 60.0f, 140.0f, XGE_COLOR_RGBA(79, 216, 194, 255));

	xgeFontFree(&font);
	if (ret == XGE_OK) xgeFontFree(&fallback);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch85", argc, argv); }
