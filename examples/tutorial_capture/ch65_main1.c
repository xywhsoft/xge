/* ch65 — 子节点查询与遍历 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;
	int iCount = 0;

	xgeShapeExSceneCreate(&pScene);

	/* 构建场景 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 100, 100, 50, 50, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExId(pShape, xgeShapeExIdFromName("circle1"));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 200, 60, 100, 80, 8, 8, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExId(pShape, xgeShapeExIdFromName("rect1"));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	xgeShapeExCreate(&pShape);
	xgeShapeExAppendTriangle(pShape, 400, 140, 450, 50, 500, 140, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* GetCount */
	xgeShapeExSceneGetCount(pScene, &iCount);

	/* GetAt：按索引获取 */
	{
		xge_shape_ex pGot = NULL;
		xgeShapeExSceneGetAt(pScene, 0, &pGot);
		if (pGot) {
			/* 修改第一个图形的颜色 */
			xgeShapeExFillColor(pGot, XGE_COLOR_RGBA(54, 162, 235, 255));
		}
	}

	xgeShapeExSceneDraw(pScene, 0.5f);

	/* Traverse：遍历所有节点 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 0, 250);
	xgeShapeExSceneOpacity(pScene, 0.7f);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* ChildGetAt 获取子节点信息 */
	{
		xge_shape_ex_scene_child_t child;
		if (xgeShapeExSceneChildGetAt(pScene, 1, &child) == XGE_OK) {
			/* child 包含类型和指针信息 */
		}
	}

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch65", argc, argv);
}
