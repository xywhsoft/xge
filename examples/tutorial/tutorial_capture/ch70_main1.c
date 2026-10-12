/* ch70 — 场景特效：高斯模糊 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;

	/* 构建场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 30, 30, 120, 80, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 100, 80, 40, 40, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 无模糊 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 50, 50);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 轻微模糊 sigma=2 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 2.0f, 0, 0, 1);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 中等模糊 sigma=5 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 5.0f, 0, 0, 1);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 强模糊 sigma=10 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 10.0f, 0, 0, 1);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 水平方向模糊 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 6.0f, 1, 0, 1);
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 50, 300);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 垂直方向模糊 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 6.0f, 2, 0, 1);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 清除特效后正常 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch70", argc, argv);
}
