/* ch66 — 场景变换 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;

	/* 构建基础场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, -50, -30, 100, 60, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 80, 0, 25, 25, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 平移 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 150, 100);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 旋转 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 400, 100);
	xgeShapeExSceneTransformRotate(pScene, 0.5f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 缩放 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 650, 100);
	xgeShapeExSceneTransformScale(pScene, 1.5f, 0.7f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 斜切 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 150, 300);
	xgeShapeExSceneTransformSkew(pScene, 0.4f, 0.0f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 组合变换 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 450, 300);
	xgeShapeExSceneTransformRotate(pScene, 0.3f);
	xgeShapeExSceneTransformScale(pScene, 1.3f, 1.3f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 环形排列场景 */
	{
		int i;
		for (i = 0; i < 6; i++) {
			float ang = i * 2.0f * 3.14159f / 6.0f;
			xgeShapeExSceneTransformIdentity(pScene);
			xgeShapeExSceneTransformTranslate(pScene, 400, 480);
			xgeShapeExSceneTransformRotate(pScene, ang);
			xgeShapeExSceneTransformTranslate(pScene, 120, 0);
			xgeShapeExSceneTransformScale(pScene, 0.6f, 0.6f);
			xgeShapeExSceneDraw(pScene, 0.5f);
		}
	}

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch66", argc, argv);
}
