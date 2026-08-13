/* ch07 — 日志系统
 * 日志同时写入终端和调试记录；画面以四种颜色模拟 INFO/WARN/ERROR/DEBUG 的层级。
 */
#include "tut_capture.h"

static void log_row(float y, uint32_t color, float width)
{
	xgeShapeRectFill((xge_rect_t){120, y, 560, 30}, XGE_COLOR_RGBA(18, 27, 35, 255));
	xgeShapeRectFill((xge_rect_t){140, y + 9, width, 12}, color);
	xgeShapeCircleFill(105, y + 15, 7, color);
}

static void draw_scene(void)
{
	/* 真正的日志 API 与视觉输出分开：这里写一次可在终端验证的启动记录。 */
	(void)xgeLogWrite(1, "tutorial", "ch07 log visualization initialized");
	xgeShapeRectFill((xge_rect_t){80, 120, 640, 340}, XGE_COLOR_RGBA(9, 17, 23, 255));
	xgeShapeRectStroke((xge_rect_t){80, 120, 640, 340}, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	log_row(175, XGE_COLOR_RGBA(79, 216, 194, 255), 350);  /* INFO */
	log_row(235, XGE_COLOR_RGBA(109, 179, 242, 255), 430); /* DEBUG */
	log_row(295, XGE_COLOR_RGBA(255, 180, 84, 255), 290);  /* WARN */
	log_row(355, XGE_COLOR_RGBA(255, 107, 94, 255), 470);  /* ERROR */
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch07-logging", argc, argv); }
