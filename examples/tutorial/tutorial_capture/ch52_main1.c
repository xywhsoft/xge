/* ch52 — 填充规则与绘制顺序 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* NonZero 填充规则（默认）：重叠区域被填充 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 80, 60, 160, 160, 0, 0, 1);
	xgeShapeExAppendRect(pShape, 140, 120, 160, 160, 0, 0, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 180));
	xgeShapeExFillRule(pShape, XGE_SHAPE_EX_FILL_NON_ZERO);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* EvenOdd 填充规则：重叠区域镂空 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 60, 160, 160, 0, 0, 1);
	xgeShapeExAppendRect(pShape, 480, 120, 160, 160, 0, 0, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 180));
	xgeShapeExFillRule(pShape, XGE_SHAPE_EX_FILL_EVEN_ODD);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 同心圆 NonZero：全部填充 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 180, 400, 90, 90, 1);
	xgeShapeExAppendCircle(pShape, 180, 400, 50, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 180));
	xgeShapeExFillRule(pShape, XGE_SHAPE_EX_FILL_NON_ZERO);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 同心圆 EvenOdd：环形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 500, 400, 90, 90, 1);
	xgeShapeExAppendCircle(pShape, 500, 400, 50, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 180));
	xgeShapeExFillRule(pShape, XGE_SHAPE_EX_FILL_EVEN_ODD);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* PaintOrder：描边在填充下方 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 680, 400, 60, 60, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 12.0f);
	xgeShapeExPaintOrder(pShape, 1); /* stroke first (below fill) */
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch52", argc, argv);
}
