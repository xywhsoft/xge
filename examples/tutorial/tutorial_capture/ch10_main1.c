/* ch10 — 自定义平台/图形后端
 * 本章不在运行中的引擎上替换后端（后端只能在 xgeInit 前设置），
 * 而是用默认后端对象验证接口可查询，并绘制平台层到图形层的调用关系。
 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_platform_backend_t platform = xgePlatformBackendGet();
	xge_graphics_backend_t graphics = xgeGraphicsBackendGet();
	uint32_t platformColor = platform.sName ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);
	uint32_t graphicsColor = graphics.sName ? XGE_COLOR_RGBA(109, 179, 242, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);

	/* 两层方框分别代表事件/窗口后端与 GPU 后端，连线代表每帧调用顺序。 */
	xgeShapeRectFill((xge_rect_t){145, 185, 510, 90}, XGE_COLOR_RGBA(23, 43, 51, 255));
	xgeShapeRectStroke((xge_rect_t){145, 185, 510, 90}, 3.0f, platformColor);
	xgeShapeRectFill((xge_rect_t){145, 345, 510, 90}, XGE_COLOR_RGBA(28, 39, 62, 255));
	xgeShapeRectStroke((xge_rect_t){145, 345, 510, 90}, 3.0f, graphicsColor);
	xgeShapeLine(400, 278, 400, 340, 4.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeLine(400, 340, 387, 325, 4.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeLine(400, 340, 413, 325, 4.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(188, 230, 18, platformColor);
	xgeShapeCircleFill(188, 390, 18, graphicsColor);
	/* 小方格暗示 init/poll/present 与 init/begin/end 回调。 */
	for ( int i = 0; i < 4; ++i ) {
		xgeShapeRectFill((xge_rect_t){270.0f + i * 72.0f, 220, 42, 20}, platformColor);
		xgeShapeRectFill((xge_rect_t){270.0f + i * 72.0f, 380, 42, 20}, graphicsColor);
	}
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch10-backends", argc, argv); }
