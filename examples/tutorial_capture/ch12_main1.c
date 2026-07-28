/* ch12 — 混合模式：加法混合发光效果 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 底层圆形（正常 Alpha 混合） */
	xgeBlendSet(XGE_BLEND_ALPHA);
	xgeShapeCircleFill(400, 280, 100, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 切换为加法混合 → 重叠区域更亮 */
	xgeBlendSet(XGE_BLEND_ADD);
	xgeShapeCircleFill(350, 280, 80, XGE_COLOR_RGBA(255, 107, 94, 128));
	xgeShapeCircleFill(450, 280, 80, XGE_COLOR_RGBA(255, 180, 84, 128));

	/* 恢复默认 */
	xgeBlendSet(XGE_BLEND_ALPHA);

	/* 下方：正片叠底阴影演示 */
	rc.fX = 150; rc.fY = 400; rc.fW = 500; rc.fH = 150;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(109, 179, 242, 255));

	xgeBlendSet(XGE_BLEND_MULTIPLY);
	rc.fX = 250; rc.fY = 430; rc.fW = 250; rc.fH = 100;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(128, 128, 128, 255));

	xgeBlendSet(XGE_BLEND_ALPHA);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch12", argc, argv);
}
