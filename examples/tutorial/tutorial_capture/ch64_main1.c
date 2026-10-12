/* ch64 — 子节点管理 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pS1 = NULL, pS2 = NULL, pS3 = NULL, pS4 = NULL;

	xgeShapeExSceneCreate(&pScene);

	/* Add：尾部追加 */
	xgeShapeExCreate(&pS1);
	xgeShapeExAppendCircle(pS1, 100, 120, 50, 50, 1);
	xgeShapeExFillColor(pS1, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pS1);

	xgeShapeExCreate(&pS2);
	xgeShapeExAppendCircle(pS2, 220, 120, 50, 50, 1);
	xgeShapeExFillColor(pS2, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pS2);

	/* Insert：在 pS1 之前插入 */
	xgeShapeExCreate(&pS3);
	xgeShapeExAppendCircle(pS3, 160, 120, 50, 50, 1);
	xgeShapeExFillColor(pS3, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExSceneInsert(pScene, pS3, pS1);

	/* InsertShapeAt：指定索引 */
	xgeShapeExCreate(&pS4);
	xgeShapeExAppendCircle(pS4, 340, 120, 50, 50, 1);
	xgeShapeExFillColor(pS4, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExSceneInsertShapeAt(pScene, pS4, 1);

	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 移除节点后绘制 */
	xgeShapeExSceneRemove(pScene, pS3);

	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 0, 200);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 子场景嵌套 */
	{
		xge_shape_ex_scene pChild = NULL;
		xge_shape_ex pCS = NULL;

		xgeShapeExSceneCreate(&pChild);
		xgeShapeExCreate(&pCS);
		xgeShapeExAppendRect(pCS, 450, 80, 100, 80, 8, 8, 1);
		xgeShapeExFillColor(pCS, XGE_COLOR_RGBA(153, 102, 255, 220));
		xgeShapeExSceneAdd(pChild, pCS);
		xgeShapeExDestroy(pCS);

		xgeShapeExSceneAddScene(pScene, pChild);
		xgeShapeExSceneDestroy(pChild);
	}

	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 0, 350);
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneDestroy(pScene);
	xgeShapeExDestroy(pS1);
	xgeShapeExDestroy(pS2);
	xgeShapeExDestroy(pS3);
	xgeShapeExDestroy(pS4);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch64", argc, argv);
}
