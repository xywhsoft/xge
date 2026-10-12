/* ch69 — 场景裁剪 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_shape_ex_scene pScene = NULL;
	xge_shape_ex pShape = NULL;
	xge_rect_t tClip;

	/* 构建场景 */
	xgeShapeExSceneCreate(&pScene);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendCircle(pShape, 100, 100, 80, 80, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);
	xgeShapeExCreate(&pShape);
	xgeShapeExAppendRect(pShape, 150, 60, 150, 120, 10, 10, 1);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExSceneAdd(pScene, pShape);
	xgeShapeExDestroy(pShape);

	/* 矩形裁剪 */
	tClip.fX = 80; tClip.fY = 50; tClip.fW = 180; tClip.fH = 130;
	xgeShapeExSceneClipRectSet(pScene, tClip);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* 形状裁剪 */
	{
		xge_shape_ex pClipShape = NULL;
		xgeShapeExSceneClipClear(pScene);
		xgeShapeExCreate(&pClipShape);
		xgeShapeExAppendCircle(pClipShape, 500, 120, 90, 90, 1);
		xgeShapeExSceneTransformIdentity(pScene);
		xgeShapeExSceneTransformTranslate(pScene, 300, 0);
		xgeShapeExSceneClipShapeAdd(pScene, pClipShape);
		xgeShapeExSceneDraw(pScene, 0.5f);
		xgeShapeExDestroy(pClipShape);
	}

	/* 清除裁剪后完整绘制 */
	xgeShapeExSceneClipClear(pScene);
	xgeShapeExSceneTransformIdentity(pScene);
	xgeShapeExSceneTransformTranslate(pScene, 0, 280);
	xgeShapeExSceneDraw(pScene, 0.5f);

	/* Subtract 模式裁剪 */
	{
		xge_shape_ex pClipShape = NULL;
		xgeShapeExCreate(&pClipShape);
		xgeShapeExAppendCircle(pClipShape, 500, 400, 50, 50, 1);
		xgeShapeExSceneTransformIdentity(pScene);
		xgeShapeExSceneTransformTranslate(pScene, 300, 280);
		xgeShapeExSceneClipShapeAddEx(pScene, pClipShape, XGE_SHAPE_EX_CLIP_SUBTRACT);
		xgeShapeExSceneDraw(pScene, 0.5f);
		xgeShapeExDestroy(pClipShape);
	}

	xgeShapeExSceneDestroy(pScene);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch69", argc, argv);
}
