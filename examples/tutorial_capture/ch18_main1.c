/* ch18 — 按需渲染：静态工具界面 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	int i;

	/* 工具栏背景 */
	rc.fX = 0; rc.fY = 0; rc.fW = 800; rc.fH = 40;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(25, 35, 45, 255));

	/* 工具栏按钮 */
	for ( i = 0; i < 5; i++ ) {
		rc.fX = 10.0f + i * 55.0f; rc.fY = 8; rc.fW = 45; rc.fH = 24;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(40, 55, 70, 255));
		xgeShapeRectStroke(rc, 1.0f, XGE_COLOR_RGBA(79, 216, 194, 80));
	}

	/* 主编辑区 */
	rc.fX = 0; rc.fY = 40; rc.fW = 600; rc.fH = 530;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(18, 24, 30, 255));

	/* 静态内容（网格线） */
	for ( i = 1; i < 6; i++ ) {
		xgeShapeLine(0, 40.0f + i * 88.0f, 600, 40.0f + i * 88.0f,
			0.5f, XGE_COLOR_RGBA(50, 65, 80, 100));
	}
	for ( i = 1; i < 8; i++ ) {
		xgeShapeLine(i * 75.0f, 40, i * 75.0f, 570,
			0.5f, XGE_COLOR_RGBA(50, 65, 80, 100));
	}

	/* 右侧属性面板 */
	rc.fX = 600; rc.fY = 40; rc.fW = 200; rc.fH = 530;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(22, 30, 38, 255));
	xgeShapeLine(600, 40, 600, 570, 1.0f, XGE_COLOR_RGBA(79, 216, 194, 60));

	/* 属性面板条目 */
	for ( i = 0; i < 6; i++ ) {
		rc.fX = 615; rc.fY = 60.0f + i * 45.0f; rc.fW = 170; rc.fH = 30;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(35, 48, 60, 255));
	}

	/* 状态栏 */
	rc.fX = 0; rc.fY = 570; rc.fW = 800; rc.fH = 30;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(25, 35, 45, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch18", argc, argv);
}
