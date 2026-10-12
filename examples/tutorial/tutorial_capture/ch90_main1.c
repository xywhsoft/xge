/* ch90 — 简易文本绘制与度量 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	xge_vec2_t size;
	xge_rect_t rect;
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 28.0f);
	if (ret != XGE_OK) return;

	/* 简单文本绘制 */
	xgeTextDraw(&font, "Simple Text Draw", 60.0f, 60.0f, XGE_COLOR_RGBA(255, 255, 255, 255));

	/* 文本度量 */
	size = xgeTextMeasure(&font, "Measured Width");
	xgeTextDraw(&font, "Measured Width", 60.0f, 120.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeRectStroke((xge_rect_t){60.0f, 95.0f, size.fX, size.fY}, 1.0f, XGE_COLOR_RGBA(255, 200, 80, 180));

	/* 矩形区域文本绘制（自动换行） */
	rect.fX = 60.0f; rect.fY = 200.0f; rect.fW = 400.0f; rect.fH = 150.0f;
	xgeShapeRectStroke(rect, 1.0f, XGE_COLOR_RGBA(100, 100, 100, 255));
	xgeTextDrawRect(&font, "Text drawn within a rectangle area with automatic wrapping and alignment support.",
		rect, XGE_COLOR_RGBA(220, 220, 220, 255), XGE_TEXT_ALIGN_LEFT);

	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch90", argc, argv); }
