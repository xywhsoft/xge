/* ch61 — 变换：Shape 级 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_matrix_t mat;

	/* 平移 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -25, 80, 50, 6, 6, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 130, 100);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 旋转 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -25, 80, 50, 6, 6, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 350, 100);
	xgeShapeExTransformRotate(pShape, 0.5f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 缩放 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -25, 80, 50, 6, 6, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 600, 100);
	xgeShapeExTransformScale(pShape, 1.8f, 0.6f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 组合变换：平移 + 旋转 + 缩放 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -25, 80, 50, 6, 6, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 200, 280);
	xgeShapeExTransformRotate(pShape, 0.3f);
	xgeShapeExTransformScale(pShape, 1.5f, 1.2f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 使用 TransformSet 直接设置矩阵 */
	xgeShapeExMatrixIdentity(&mat);
	xgeShapeExMatrixTranslate(&mat, 500, 280);
	xgeShapeExMatrixSkew(&mat, 0.3f, 0.1f);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -25, 80, 50, 6, 6, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(153, 102, 255, 200));
	xgeShapeExTransformSet(pShape, &mat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 环形排列（旋转 + 平移组合）*/
	{
		int i;
		for (i = 0; i < 8; i++) {
			float ang = i * 2.0f * 3.14159f / 8.0f;
			xgeShapeExCreate(&pShape);
			xgeShapeExAppendRect(pShape, -20, -8, 40, 16, 4, 4, 1);
			xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, (uint32_t)(120 + i * 16)));
			xgeShapeExTransformIdentity(pShape);
			xgeShapeExTransformTranslate(pShape, 400, 460);
			xgeShapeExTransformRotate(pShape, ang);
			xgeShapeExTransformTranslate(pShape, 100, 0);
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch61", argc, argv);
}
