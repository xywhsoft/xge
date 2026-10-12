/* ch56 — 遮罩：复合运算 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pTarget = NULL;
	xge_shape_ex pMask = NULL;

	/* Add 复合：遮罩区域相加 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 160, 130, 70, 70, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 200));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 60, 40, 200, 180, 10, 10, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_ADD);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Subtract 复合 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 480, 130, 70, 70, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 380, 40, 200, 180, 10, 10, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_SUBTRACT);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Intersect 复合 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 160, 400, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 60, 310, 200, 180, 10, 10, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_INTERSECT);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Difference 复合 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 480, 400, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 380, 310, 200, 180, 10, 10, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(160, 200, 255, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_DIFFERENCE);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Lighten 复合 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 680, 300, 70, 70, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(200, 200, 200, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 610, 230, 150, 150, 10, 10, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(153, 102, 255, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_LIGHTEN);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch56", argc, argv);
}
