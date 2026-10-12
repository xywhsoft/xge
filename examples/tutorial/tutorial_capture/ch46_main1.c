/* ch46 — 描边：颜色与宽度 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 不同宽度的描边 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 80, 350, 80);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 1.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 130, 350, 130);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 190, 350, 190);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendLine(pShape, 50, 260, 350, 260);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExStrokeWidth(pShape, 12.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 不同颜色的描边圆形 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 550, 120, 60, 60, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 550, 280, 60, 60, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExStrokeWidth(pShape, 4.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 填充 + 描边组合 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 370, 280, 150, 16, 16, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 80));
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 渐变描边 */
	{
		xge_shape_ex_color_stop_t stops[2];
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, 50, 370, 300, 150, 16, 16, 1);
		xgeShapeExStrokeLinearGradient(pShape, 50, 445, 350, 445,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
		xgeShapeExStrokeWidth(pShape, 5.0f);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch46", argc, argv);
}
