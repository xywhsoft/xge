/* ch38 — 路径构建：椭圆弧 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 标准圆弧（largeArc=0, sweep=1）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 150, 200);
	xgeShapeExArcTo(pShape, 80, 80, 0, 0, 1, 310, 200);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 大弧（largeArc=1, sweep=1）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 150, 350);
	xgeShapeExArcTo(pShape, 80, 80, 0, 1, 1, 310, 350);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 椭圆弧（rx != ry）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 200);
	xgeShapeExArcTo(pShape, 120, 60, 0, 0, 1, 690, 200);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 旋转轴椭圆弧 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 400);
	xgeShapeExArcTo(pShape, 100, 50, 30, 1, 0, 650, 350);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 用 ArcTo 构建完整圆环路径 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 200, 480);
	xgeShapeExArcTo(pShape, 60, 60, 0, 1, 1, 200.01f, 480);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 100));
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 2.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch38", argc, argv);
}
