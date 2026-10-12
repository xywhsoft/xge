/* ch57 — 遮罩查询与清除 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pTarget = NULL;
	xge_shape_ex pMask = NULL;
	int iMethod = 0, iTargetType = 0;
	xge_shape_ex pQueryShape = NULL;
	xge_shape_ex_scene pQueryScene = NULL;

	/* 设置遮罩后查询状态 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 200, 150, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 100, 50, 200, 200, 12, 12, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_ALPHA);

	/* 查询遮罩信息 */
	xgeShapeExMaskGet(pTarget, &iMethod, &iTargetType, &pQueryShape, &pQueryScene);
	/* iMethod == XGE_SHAPE_EX_MASK_ALPHA, iTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE */

	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* 清除遮罩后正常绘制 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 550, 150, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 450, 50, 200, 200, 12, 12, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_ALPHA);
	xgeShapeExMaskClear(pTarget); /* 清除遮罩 */
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Scene 级遮罩 */
	{
		xge_shape_ex_scene pScene = NULL;
		xge_shape_ex pS1 = NULL, pS2 = NULL;

		xgeShapeExCreate(&pMask);
		xgeShapeExAppendCircle(pMask, 200, 430, 90, 90, 1);
		xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

		xgeShapeExSceneCreate(&pScene);
		xgeShapeExCreate(&pS1);
		xgeShapeExAppendRect(pS1, 80, 330, 120, 200, 8, 8, 1);
		xgeShapeExFillColor(pS1, XGE_COLOR_RGBA(255, 107, 94, 255));
		xgeShapeExSceneAdd(pScene, pS1);

		xgeShapeExCreate(&pS2);
		xgeShapeExAppendRect(pS2, 160, 380, 120, 150, 8, 8, 1);
		xgeShapeExFillColor(pS2, XGE_COLOR_RGBA(160, 200, 255, 255));
		xgeShapeExSceneAdd(pScene, pS2);

		xgeShapeExSceneMaskShapeSet(pScene, pMask, XGE_SHAPE_EX_MASK_ALPHA);
		xgeShapeExSceneDraw(pScene, 0.5f);

		xgeShapeExSceneDestroy(pScene);
		xgeShapeExDestroy(pS1);
		xgeShapeExDestroy(pS2);
		xgeShapeExDestroy(pMask);
	}

	/* 无遮罩对照 */
	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 450, 330, 200, 200, 12, 12, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(153, 102, 255, 200));
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch57", argc, argv);
}
