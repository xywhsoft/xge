/* ch67 — 场景属性 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;

	/* 构建场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 0, 0, 50, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 60, -30, 80, 60, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 不透明度渐变 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 130, 100);
	xgeShapeExSceneOpacity(pScene, 1.0f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneTransformTranslate(pScene, 200, 0);
	xgeShapeExSceneOpacity(pScene, 0.6f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneTransformTranslate(pScene, 200, 0);
	xgeShapeExSceneOpacity(pScene, 0.3f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 可见性 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 130, 280);
	xgeShapeExSceneOpacity(pScene, 1.0f);
	xgeShapeExSceneVisible(pScene, 1);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneTransformTranslate(pScene, 200, 0);
	xgeShapeExSceneVisible(pScene, 0); /* 不可见 */
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneTransformTranslate(pScene, 200, 0);
	xgeShapeExSceneVisible(pScene, 1);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 混合模式 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 200, 450);
	xgeShapeExSceneBlend(pScene, XGE_BLEND_ADD);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneTransformTranslate(pScene, 250, 0);
	xgeShapeExSceneBlendClear(pScene);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch67", argc, argv);
}
