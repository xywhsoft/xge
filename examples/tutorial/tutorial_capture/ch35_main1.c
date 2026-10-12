/* ch35 — 矩阵进阶：斜切/逆/变换点 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_matrix_t mat, inv;
	xge_vec2_t pt, out;

	/* 斜切变换 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -40, -40, 80, 80, 0, 0, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 200));
	xgeShapeExTransformIdentity(pShape);
	xgeShapeExTransformTranslate(pShape, 150, 150);
	xgeShapeExTransformSkew(pShape, 0.4f, 0.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 逆矩阵：先变换再逆变换回原位 */
	xgeShapeExMatrixIdentity(&mat);
	xgeShapeExMatrixTranslate(&mat, 100, 50);
	xgeShapeExMatrixRotate(&mat, 0.5f);
	xgeShapeExMatrixInvert(&inv, &mat);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 0, 0, 20, 20, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 220));
	xgeShapeExTransformSet(pShape, &mat);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 0, 0, 20, 20, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 220));
	xgeShapeExTransformSet(pShape, &inv);
	xgeShapeExTransformTranslate(pShape, 500, 300);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 变换点：将网格点通过矩阵映射 */
	xgeShapeExMatrixIdentity(&mat);
	xgeShapeExMatrixTranslate(&mat, 400, 400);
	xgeShapeExMatrixRotate(&mat, 0.6f);
	xgeShapeExMatrixScale(&mat, 1.5f, 0.8f);

	{
		int i, j;
		for (i = 0; i < 5; i++) {
			for (j = 0; j < 5; j++) {
				pt.fX = (float)(i * 30 - 60);
				pt.fY = (float)(j * 30 - 60);
				xgeShapeExMatrixPoint(&out, &mat, pt);
				xgeShapeCircleFill(out.fX, out.fY, 4.0f, XGE_COLOR_RGBA(160, 200, 255, 200));
			}
		}
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch35", argc, argv);
}
