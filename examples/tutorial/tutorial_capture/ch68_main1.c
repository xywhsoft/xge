/* ch68 — 场景遮罩 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;
	xge_shape_ex pMask = NULL;

	/* 构建多图形场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 50, 50, 250, 200, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 250, 150, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 用圆形遮罩场景 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 180, 150, 100, 100, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeExSceneMaskShapeSet(pScene, pMask, XGE_SHAPE_EX_MASK_ALPHA);
	xgeShapeExSceneDraw(pScene, 0.5f);
	xgeShapeExDestroy(pMask);

	/* 清除遮罩 */
	xgeShapeExSceneMaskClear(pScene);

	/* 场景遮罩场景 */
	{
		xge_shape_ex_scene pMaskScene = NULL;
		xge_shape_ex pMS = NULL;

		xgeShapeExSceneCreate(&pMaskScene);
		xgeShapeExCreate(&pMS);
		xgeShapeExAppendCircle(pMS, 550, 150, 60, 60, 1);
		xgeShapeExFillColor(pMS, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeShapeExSceneAdd(pMaskScene, pMS);
		xgeShapeExDestroy(pMS);
		xgeShapeExCreate(&pMS);
		xgeShapeExAppendRect(pMS, 580, 100, 80, 100, 8, 8, 1);
		xgeShapeExFillColor(pMS, XGE_COLOR_RGBA(255, 255, 255, 200));
		xgeShapeExSceneAdd(pMaskScene, pMS);
		xgeShapeExDestroy(pMS);

		xgeShapeExSceneTransformIdentity(pScene);
		xgeShapeExSceneTransformTranslate(pScene, 350, 0);
		xgeShapeExSceneMaskSceneSet(pScene, pMaskScene, XGE_SHAPE_EX_MASK_ALPHA);
		xgeShapeExSceneDraw(pScene, 0.5f);

		xgeShapeExSceneDestroy(pMaskScene);
	}

	/* Luma 遮罩 */
	{
		xge_shape_ex_color_stop_t stops[2];
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(0, 0, 0, 255);

		xgeShapeExCreate(&pMask);
		xgeShapeExAppendRect(pMask, 50, 320, 300, 200, 0, 0, 1);
		xgeShapeExFillLinearGradient(pMask, 50, 420, 350, 420,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);

		xgeShapeExSceneTransformIdentity(pScene);
		xgeShapeExSceneTransformTranslate(pScene, 0, 270);
		xgeShapeExSceneMaskShapeSet(pScene, pMask, XGE_SHAPE_EX_MASK_LUMA);
		xgeShapeExSceneDraw(pScene, 0.5f);
		xgeShapeExDestroy(pMask);
	}

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch68", argc, argv);
}
