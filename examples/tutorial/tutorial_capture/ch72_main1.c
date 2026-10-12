/* ch72 — 场景几何与命中测试 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;
	xge_rect_t tBounds;
	int bIntersects = 0, bContains = 0;

	/* 构建场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 150, 150, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 250, 100, 150, 100, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 获取场景边界 */
	xgeShapeExSceneGetBounds(pScene, 0.5f, &tBounds);
	/* 绘制边界框 */
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, tBounds.fX, tBounds.fY, tBounds.fW, tBounds.fH, 0, 0, 1);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 255, 255, 120));
	xgeShapeExStrokeWidth(pShape, 1.0f);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 矩形相交测试 */
	{
		xge_rect_t tTest = {100, 100, 80, 80};
		xgeShapeExSceneIntersects(pScene, tTest, 0.5f, &bIntersects);
		xgeShapeExCreate(&pShape);
		xgeShapeExAppendRect(pShape, tTest.fX, tTest.fY, tTest.fW, tTest.fH, 0, 0, 1);
		xgeShapeExStrokeColor(pShape, bIntersects ?
			XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255));
		xgeShapeExStrokeWidth(pShape, 2.0f);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}

	/* 点包含测试 */
	xgeShapeExSceneContainsPoint(pScene, 150, 150, 0.5f, &bContains);
	xgeShapeCircleFill(150, 150, 5.0f, bContains ?
		XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255));

	xgeShapeExSceneContainsPoint(pScene, 500, 500, 0.5f, &bContains);
	xgeShapeCircleFill(500, 500, 5.0f, bContains ?
		XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255));

	/* HitTest：找到最上层图形 */
	{
		xge_shape_ex pHit = NULL;
		xgeShapeExSceneHitTest(pScene, 280, 150, 0.5f, &pHit);
		if (pHit) {
			xgeShapeExStrokeColor(pHit, XGE_COLOR_RGBA(255, 255, 0, 255));
			xgeShapeExStrokeWidth(pHit, 3.0f);
		}
	}

	/* 下方区域：OBB 可视化 */
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 200, 250);
	xgeShapeExSceneTransformRotate(pScene, 0.4f);
	{
		xge_vec2_t obb[4];
		xgeShapeExSceneGetOBB(pScene, 0.5f, obb);
		xgeShapeExCreate(&pShape);
		xgeShapeExMoveTo(pShape, obb[0].fX, obb[0].fY);
		xgeShapeExLineTo(pShape, obb[1].fX, obb[1].fY);
		xgeShapeExLineTo(pShape, obb[2].fX, obb[2].fY);
		xgeShapeExLineTo(pShape, obb[3].fX, obb[3].fY);
		xgeShapeExClose(pShape);
		xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 200));
		xgeShapeExStrokeWidth(pShape, 1.5f);
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);
	}
	xgeShapeExSceneDraw(pScene, 0.5f);

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch72", argc, argv);
}
