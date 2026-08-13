/* ch01 — 环境搭建与工具链
 * 这个最小可运行示例用一张“构建流水线”图说明从 C 源码到 XGE 画面的关系。
 * 它继承 tut_capture.h，因此可用 --frames N 自动退出或 --capture 输出 PNG。
 */
#include "tut_capture.h"

static void block(float x, float y, float w, float h, uint32_t color)
{
	xge_rect_t r = {x, y, w, h};
	xgeShapeRectFill(r, color);
	xgeShapeRectStroke(r, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
}

static void arrow(float x0, float x1, float y)
{
	xgeShapeLine(x0, y, x1, y, 3.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeLine(x1, y, x1 - 12.0f, y - 8.0f, 3.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeLine(x1, y, x1 - 12.0f, y + 8.0f, 3.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
}

static void draw_scene(void)
{
	/* 四个方块对应读者在本章要准备的工作链：源码、编译器、DLL 和窗口。 */
	block(55, 220, 135, 120, XGE_COLOR_RGBA(28, 54, 64, 255));
	block(245, 220, 135, 120, XGE_COLOR_RGBA(45, 57, 75, 255));
	block(435, 220, 135, 120, XGE_COLOR_RGBA(77, 59, 38, 255));
	block(625, 220, 120, 120, XGE_COLOR_RGBA(45, 75, 65, 255));
	arrow(195, 235, 280); arrow(385, 425, 280); arrow(575, 615, 280);

	/* 边框表示项目根目录，提醒读者在仓库根目录运行构建脚本。 */
	xgeShapeRectStroke((xge_rect_t){28, 160, 744, 240}, 1.0f, XGE_COLOR_RGBA(109, 179, 242, 190));
	xgeShapeCircleFill(122, 280, 20, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeCircleFill(312, 280, 20, XGE_COLOR_RGBA(109, 179, 242, 255));
	xgeShapeCircleFill(502, 280, 20, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(685, 280, 20, XGE_COLOR_RGBA(255, 107, 94, 255));
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch01-toolchain", argc, argv); }
