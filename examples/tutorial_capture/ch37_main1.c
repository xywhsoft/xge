/* ch37 — 路径构建：贝塞尔曲线 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 二次贝塞尔曲线（QuadTo）：抛物线形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 80, 250);
	xgeShapeExQuadTo(pShape, 250, 50, 420, 250);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 三次贝塞尔曲线（CubicTo）：S 形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 80, 400);
	xgeShapeExCubicTo(pShape, 200, 300, 300, 500, 420, 400);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 心形：两段三次贝塞尔 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 600, 180);
	xgeShapeExCubicTo(pShape, 600, 140, 540, 100, 540, 160);
	xgeShapeExCubicTo(pShape, 540, 200, 600, 240, 600, 270);
	xgeShapeExCubicTo(pShape, 600, 240, 660, 200, 660, 160);
	xgeShapeExCubicTo(pShape, 660, 100, 600, 140, 600, 180);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 220));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 波浪线：多段 QuadTo 连续 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 450);
	xgeShapeExQuadTo(pShape, 500, 400, 550, 450);
	xgeShapeExQuadTo(pShape, 600, 500, 650, 450);
	xgeShapeExQuadTo(pShape, 700, 400, 750, 450);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 2.5f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch37", argc, argv);
}
