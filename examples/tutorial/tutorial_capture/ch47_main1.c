/* ch47 — 描边：端帽与连接 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* Butt Cap（默认）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 100, 80, 350, 80);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 16.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_BUTT);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Round Cap */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 100, 150, 350, 150);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 16.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Square Cap */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 100, 220, 350, 220);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 16.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_SQUARE);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Miter Join（默认）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 120);
	xgeShapeExLineTo(pShape, 550, 60);
	xgeShapeExLineTo(pShape, 650, 120);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 12.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Round Join */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 200);
	xgeShapeExLineTo(pShape, 550, 140);
	xgeShapeExLineTo(pShape, 650, 200);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 12.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Bevel Join */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 450, 280);
	xgeShapeExLineTo(pShape, 550, 220);
	xgeShapeExLineTo(pShape, 650, 280);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 12.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_BEVEL);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 组合：Round Cap + Round Join 的封闭多边形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 200, 350);
	xgeShapeExLineTo(pShape, 350, 400);
	xgeShapeExLineTo(pShape, 300, 530);
	xgeShapeExLineTo(pShape, 100, 530);
	xgeShapeExLineTo(pShape, 50, 400);
	xgeShapeExClose(pShape);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 8.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch47", argc, argv);
}
