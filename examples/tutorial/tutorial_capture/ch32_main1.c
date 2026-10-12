/* ch32 — 渲染模式：AA-Mesh vs SDF 对比 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 上半屏：AA-Mesh 模式 */
	xgeShapeRenderModeSet(XGE_SHAPE_RENDER_AA_MESH);
	xgeShapeCircleFill(200, 170, 80, XGE_COLOR_RGBA(79, 216, 194, 255));
	rc.fX = 350; rc.fY = 110; rc.fW = 160; rc.fH = 120;
	xgeShapeRoundRectFill(rc, 20.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 分隔线 */
	xgeShapeLine(50, 300, 750, 300, 1.0f, XGE_COLOR_RGBA(200, 210, 220, 100));

	/* 下半屏：SDF 模式 */
	xgeShapeRenderModeSet(XGE_SHAPE_RENDER_SDF);
	xgeShapeCircleFill(200, 430, 80, XGE_COLOR_RGBA(79, 216, 194, 255));
	rc.fX = 350; rc.fY = 370; rc.fW = 160; rc.fH = 120;
	xgeShapeRoundRectFill(rc, 20.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 恢复默认 */
	xgeShapeRenderModeSet(XGE_SHAPE_RENDER_AA_MESH);

	/* 标注区域 */
	rc.fX = 580; rc.fY = 130; rc.fW = 150; rc.fH = 30;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 60));
	rc.fX = 580; rc.fY = 390; rc.fW = 150; rc.fH = 30;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(255, 180, 84, 60));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch32", argc, argv);
}
