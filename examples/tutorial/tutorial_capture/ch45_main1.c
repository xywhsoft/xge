/* ch45 — 渐变坐标变换 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_color_stop_t stops[2];
	xge_shape_ex_matrix_t gmat;

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(79, 216, 194, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 107, 94, 255);

	/* 无变换的线性渐变 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 300, 120, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 50, 110, 350, 110,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 旋转渐变方向 45° */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 50, 300, 120, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 420, 110, 720, 110,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExMatrixIdentity(&gmat);
	xgeShapeExMatrixTranslate(&gmat, 570, 110);
	xgeShapeExMatrixRotate(&gmat, 0.785f);
	xgeShapeExMatrixTranslate(&gmat, -570, -110);
	xgeShapeExFillGradientTransformSet(pShape, &gmat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 缩放渐变（压缩） */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 240, 300, 120, 10, 10, 1);
	xgeShapeExFillLinearGradient(pShape, 50, 300, 350, 300,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExMatrixIdentity(&gmat);
	xgeShapeExMatrixTranslate(&gmat, 200, 300);
	xgeShapeExMatrixScale(&gmat, 0.4f, 1.0f);
	xgeShapeExMatrixTranslate(&gmat, -200, -300);
	xgeShapeExFillGradientTransformSet(pShape, &gmat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 径向渐变 + 旋转变换 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 420, 240, 300, 120, 10, 10, 1);
	xgeShapeExFillRadialGradient(pShape, 570, 300, 80, 570, 300,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExMatrixIdentity(&gmat);
	xgeShapeExMatrixTranslate(&gmat, 570, 300);
	xgeShapeExMatrixScale(&gmat, 1.8f, 0.6f);
	xgeShapeExMatrixTranslate(&gmat, -570, -300);
	xgeShapeExFillGradientTransformSet(pShape, &gmat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 渐变变换 + Repeat 组合 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 400, 480, 80, 80, 1);
	xgeShapeExFillLinearGradient(pShape, 360, 480, 440, 480,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(pShape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT);
	xgeShapeExMatrixIdentity(&gmat);
	xgeShapeExMatrixRotate(&gmat, 1.0f);
	xgeShapeExFillGradientTransformSet(pShape, &gmat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch45", argc, argv);
}
