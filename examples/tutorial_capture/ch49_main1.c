/* ch49 — 描边：虚线模式 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	float dash1[] = {10.0f, 10.0f};
	float dash2[] = {20.0f, 5.0f};
	float dash3[] = {15.0f, 5.0f, 5.0f, 5.0f};
	float dash4[] = {30.0f, 10.0f, 5.0f, 10.0f};

	/* 等长虚线 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 80, 750, 80);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeDash(pShape, dash1, 2, 0.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 长划短线 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 150, 750, 150);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeDash(pShape, dash2, 2, 0.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 点划线 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 220, 750, 220);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeDash(pShape, dash3, 4, 0.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 复杂虚线 + 偏移 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 290, 750, 290);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExStrokeDash(pShape, dash4, 4, 15.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 虚线矩形边框 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 350, 300, 180, 12, 12, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeDash(pShape, dash1, 2, 0.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 虚线圆形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 550, 440, 80, 80, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeDash(pShape, dash3, 4, 0.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch49", argc, argv);
}
