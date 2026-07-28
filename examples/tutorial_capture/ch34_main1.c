/* ch34 — 2D 仿射矩阵：平移/旋转/缩放 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_matrix_t mat;

	/* 原始位置 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -50, -30, 100, 60, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 150));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 200, 150);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 旋转 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -50, -30, 100, 60, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 400, 300);
	xgeShapeExTransformRotate(pShape, 30.0f * 3.14159f / 180.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 缩放 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -50, -30, 100, 60, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 600, 450);
	xgeShapeExTransformScale(pShape, 1.5f, 0.7f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch34", argc, argv);
}
