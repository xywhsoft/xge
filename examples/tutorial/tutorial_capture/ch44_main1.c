/* ch44 — 渐变扩散模式 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_color_stop_t stops[2];

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);

	/* Pad 模式（默认）：边缘颜色延伸 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 700, 100, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 200, 100, 550, 100,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_PAD);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Reflect 模式：镜像重复 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 200, 700, 100, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 200, 250, 550, 250,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Repeat 模式：平铺重复 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 350, 700, 100, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 200, 400, 550, 400,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 径向渐变 + Repeat */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 400, 530, 60, 60, 1);
	xgeShapeExFillRadialGradient(pShape, 400, 530, 30, 400, 530,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch44", argc, argv);
}
