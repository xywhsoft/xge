/* ch48 — 描边：斜接限制与非缩放 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 尖角 Miter（默认 limit=4）：角度较小产生尖刺 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 100, 200);
	xgeShapeExLineTo(pShape, 200, 80);
	xgeShapeExLineTo(pShape, 300, 200);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 10.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExStrokeMiterLimit(pShape, 10.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 低 MiterLimit：尖角被截断为 Bevel */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 400, 200);
	xgeShapeExLineTo(pShape, 500, 80);
	xgeShapeExLineTo(pShape, 600, 200);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 10.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExStrokeMiterLimit(pShape, 1.5f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 非缩放描边：形状放大但描边宽度不变 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -60, -40, 120, 80, 8, 8, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeNonScaling(pShape, 1);
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 200, 380);
	xgeShapeExTransformScale(pShape, 2.0f, 2.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 对比：缩放描边（默认行为）*/
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -60, -40, 120, 80, 8, 8, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeNonScaling(pShape, 0);
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 550, 380);
	xgeShapeExTransformScale(pShape, 2.0f, 2.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 极小角度 + 高 miter limit */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 100, 550);
	xgeShapeExLineTo(pShape, 250, 480);
	xgeShapeExLineTo(pShape, 400, 550);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 200));
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExStrokeMiterLimit(pShape, 20.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch48", argc, argv);
}
