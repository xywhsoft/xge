/* ch51 — Trim Path 路径裁剪 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 完整圆形路径（无 trim）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 150, 150, 80, 80, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Trim 0~25%：四分之一弧 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 400, 150, 80, 80, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExTrimPath(pShape, 0.0f, 0.25f, 0);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Trim 25~75%：半圆弧 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 650, 150, 80, 80, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExTrimPath(pShape, 0.25f, 0.75f, 0);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 矩形路径 Trim：进度条效果 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 320, 700, 60, 30, 30, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 5.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExTrimPath(pShape, 0.0f, 0.6f, 0);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Simultaneous 模式：每段同时 trim */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 100, 450);
	xgeShapeExLineTo(pShape, 300, 450);
	xgeShapeExMoveTo(pShape, 100, 500);
	xgeShapeExLineTo(pShape, 300, 500);
	xgeShapeExMoveTo(pShape, 100, 550);
	xgeShapeExLineTo(pShape, 300, 550);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExTrimPath(pShape, 0.0f, 0.7f, 1);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 螺旋路径 trim */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 550, 480);
	xgeShapeExCubicTo(pShape, 650, 400, 700, 500, 620, 550);
	xgeShapeExCubicTo(pShape, 540, 600, 480, 520, 550, 480);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExTrimPath(pShape, 0.1f, 0.8f, 0);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch51", argc, argv);
}
