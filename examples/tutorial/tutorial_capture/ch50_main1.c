/* ch50 — 描边渐变 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_color_stop_t stops[3];

	/* 线性渐变描边 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(255, 180, 84, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 300, 150, 16, 16, 1);
	xgeShapeExStrokeLinearGradient(pShape, 50, 125, 350, 125,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExStrokeWidth(pShape, 6.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 径向渐变描边 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 200, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(153, 102, 255, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 550, 130, 80, 80, 1);
	xgeShapeExStrokeRadialGradient(pShape, 550, 130, 80, 550, 130,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExStrokeWidth(pShape, 8.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 渐变描边 + 虚线 */
	{
		float dash[] = {15.0f, 8.0f};
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(54, 162, 235, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 99, 132, 255);
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, 50, 280, 700, 100, 12, 12, 1);
		xgeShapeExStrokeLinearGradient(pShape, 50, 330, 750, 330,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
		xgeShapeExStrokeWidth(pShape, 5.0f);
		xgeShapeExStrokeDash(pShape, dash, 2, 0.0f);
		xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	/* 渐变描边 + Repeat 扩散 */
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 400, 490, 70, 70, 1);
	xgeShapeExStrokeLinearGradient(pShape, 370, 490, 430, 490,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExStrokeGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT);
	xgeShapeExStrokeWidth(pShape, 10.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch50", argc, argv);
}
