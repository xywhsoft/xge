/* ch62 — 绘制 ShapeEx */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;
	xge_shape_ex_matrix_t parentMat;

	/* 基本绘制：Draw */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 130, 120, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* DrawPx：像素对齐绘制 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 260, 55, 130, 130, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExDrawPx(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* DrawEx：带父矩阵和父透明度 */
	xgeShapeExMatrixIdentity(&parentMat);
	xgeShapeExMatrixTranslate(&parentMat, 550, 120);
	xgeShapeExMatrixRotate(&parentMat, 0.4f);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -50, -50, 100, 100, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExDrawEx(pShape, 0.5f, &parentMat, 0.7f);
	xgeShapeExDestroy(pShape);

	/* Scene 绘制：多图形组合 */
	{
		xge_shape_ex_scene pScene = NULL;
		xge_shape_ex pS1 = NULL, pS2 = NULL, pS3 = NULL;

		xgeShapeExSceneCreate(&pScene);

		xgeShapeExCreate(&pS1);
		xgeShapeExAppendCircle(pS1, 150, 380, 60, 60, 1);
		xgeShapeExFillColor(pS1, XGE_COLOR_RGBA(54, 162, 235, 220));
		xgeShapeExSceneAdd(pScene, pS1);

		xgeShapeExCreate(&pS2);
		xgeShapeExAppendRect(pS2, 200, 330, 120, 100, 10, 10, 1);
		xgeShapeExFillColor(pS2, XGE_COLOR_RGBA(255, 206, 86, 220));
		xgeShapeExSceneAdd(pScene, pS2);

		xgeShapeExCreate(&pS3);
		xgeShapeExAppendTriangle(pS3, 350, 430, 420, 320, 490, 430, 1);
		xgeShapeExFillColor(pS3, XGE_COLOR_RGBA(153, 102, 255, 220));
		xgeShapeExSceneAdd(pScene, pS3);

		/* Scene 级变换 */
		xgeShapeExSceneTransformIdentity(pScene);
		xgeShapeExSceneTransformTranslate(pScene, 50, 20);
		xgeShapeExSceneOpacity(pScene, 0.9f);

		xgeShapeExSceneDraw(pScene, 0.5f);

		xgeShapeExSceneDestroy(pScene);
		xgeShapeExDestroy(pS1);
		xgeShapeExDestroy(pS2);
		xgeShapeExDestroy(pS3);
	}

	/* Scene DrawEx：带父矩阵 */
	{
		xge_shape_ex_scene pScene = NULL;
		xge_shape_ex pS = NULL;

		xgeShapeExSceneCreate(&pScene);
		xgeShapeExCreate(&pS);
		xgeShapeExAppendCircle(pS, 0, 0, 50, 50, 1);
		xgeShapeExFillColor(pS, XGE_COLOR_RGBA(79, 216, 194, 255));
		xgeShapeExSceneAdd(pScene, pS);

		xgeShapeExMatrixIdentity(&parentMat);
		xgeShapeExMatrixTranslate(&parentMat, 620, 400);
		xgeShapeExMatrixScale(&parentMat, 1.3f, 1.3f);

		xgeShapeExSceneDrawEx(pScene, 0.5f, &parentMat, 0.8f);

		xgeShapeExSceneDestroy(pScene);
		xgeShapeExDestroy(pS);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch62", argc, argv);
}
