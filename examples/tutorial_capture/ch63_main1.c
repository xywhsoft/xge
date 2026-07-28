/* ch63 — 场景对象生命周期 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;
	int iCount = 0;

	/* 创建场景 */
	xgeShapeExSceneCreate(&pScene);

	/* 添加图形到场景 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 150, 150, 70, 70, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 280, 80, 140, 140, 12, 12, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendTriangle(pShape, 550, 220, 620, 80, 690, 220, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 查询子节点数 */
	xgeShapeExSceneGetCount(pScene, &iCount);

	/* 绘制场景 */
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 克隆场景 */
	{
		xge_shape_ex_scene pClone = NULL;
		xgeShapeExSceneClone(pScene, &pClone);
		xgeShapeExSceneTransformIdentity(pClone);
		xgeShapeExSceneTransformTranslate(pClone, 0, 280);
		xgeShapeExSceneOpacity(pClone, 0.6f);
		xgeShapeExSceneDraw(pClone, 0.5f);
		xgeShapeExSceneDestroy(pClone);
	}

	/* 清除场景 */
	xgeShapeExSceneClear(pScene);
	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch63", argc, argv);
}
