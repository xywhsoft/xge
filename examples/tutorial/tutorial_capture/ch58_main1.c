/* ch58 — 裁剪：矩形裁剪 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_rect_t tClip;

	/* 矩形裁剪：圆形被矩形裁剪 */
	tClip.fX = 100; tClip.fY = 60; tClip.fW = 180; tClip.fH = 140;
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 190, 130, 100, 100, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExClipRectSet(pShape, tClip);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 裁剪区域可视化边框 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 100, 60, 180, 140, 0, 0, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 255, 255, 100));
	xgeShapeExStrokeWidth(pShape, 1.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 渐变矩形被小矩形裁剪 */
	{
		xge_shape_ex_color_stop_t stops[2];
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(153, 102, 255, 255);
		tClip.fX = 430; tClip.fY = 60; tClip.fW = 200; tClip.fH = 120;
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, 380, 30, 300, 200, 16, 16, 1);
		xgeShapeExFillLinearGradient(pShape, 380, 130, 680, 130,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
		xgeShapeExClipRectSet(pShape, tClip);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	/* 多个图形共享裁剪区域 */
	tClip.fX = 80; tClip.fY = 300; tClip.fW = 300; tClip.fH = 220;

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 150, 380, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeExClipRectSet(pShape, tClip);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 280, 420, 90, 90, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(54, 162, 235, 200));
	xgeShapeExClipRectSet(pShape, tClip);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 裁剪边框 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 80, 300, 300, 220, 0, 0, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 255, 255, 80));
	xgeShapeExStrokeWidth(pShape, 1.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 无裁剪对照 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 580, 400, 90, 90, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch58", argc, argv);
}
