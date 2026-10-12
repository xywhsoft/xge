/* ch83 — 字体缓存加载 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"
#define XRF_PATH  "ch83_cache.xrf"

static void draw_scene(void)
{
	xge_font_t font;
	int ret;

	/* 缓存加载：TTF + XRF 路径，首次生成 XRF，后续直接加载 */
	ret = xgeFontLoadCached(&font, FONT_PATH, XRF_PATH, 30.0f, 0x20, 95);
	if (ret != XGE_OK) return;

	xgeTextDraw(&font, "Cached Font Loading", 60.0f, 80.0f, XGE_COLOR_RGBA(130, 255, 180, 255));
	xgeTextDraw(&font, "TTF + XRF fast path", 60.0f, 135.0f, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch83", argc, argv); }
