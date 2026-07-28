/* ch71 — 场景特效：投影/填充/色调 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;

	/* 构建场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 20, 20, 120, 80, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* Drop Shadow 投影 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectDropShadow(pScene, XGE_COLOR_RGBA(0, 0, 0, 180), 45.0f, 8.0f, 4.0f, 1);
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 50, 50);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 不同角度投影 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectDropShadow(pScene, XGE_COLOR_RGBA(0, 0, 0, 150), 135.0f, 10.0f, 5.0f, 1);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* Fill 特效：用颜色覆盖 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectFill(pScene, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* Tint 色调 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectTint(pScene, XGE_COLOR_RGBA(0, 0, 100, 255), XGE_COLOR_RGBA(255, 200, 100, 255), 1.0f);
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 50, 250);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* Tritone 三色调 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectTritone(pScene,
		XGE_COLOR_RGBA(20, 0, 80, 255),
		XGE_COLOR_RGBA(150, 50, 150, 255),
		XGE_COLOR_RGBA(255, 220, 100, 255),
		XGE_BLEND_ALPHA);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 无特效对照 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 组合：模糊 + 投影 */
	xgeShapeExSceneEffectClear(pScene);
	xgeShapeExSceneEffectGaussianBlur(pScene, 2.0f, 0, 0, 1);
	xgeShapeExSceneEffectDropShadow(pScene, XGE_COLOR_RGBA(0, 0, 0, 120), 90.0f, 6.0f, 3.0f, 1);
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 50, 430);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch71", argc, argv);
}
