/* ch55 — 遮罩：Alpha / Luma */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pTarget = NULL;
	xge_shape_ex pMask = NULL;

	/* Alpha 遮罩：用圆形遮罩矩形 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 180, 150, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 80, 50, 200, 200, 0, 0, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_ALPHA);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Inv Alpha 遮罩：反转 */
	xgeShapeExCreate(&pMask);
	xgeShapeExAppendCircle(pMask, 500, 150, 80, 80, 1);
	xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));

	xgeShapeExCreate(&pTarget);
	xgeShapeExAppendRect(pTarget, 400, 50, 200, 200, 0, 0, 1);
	xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_INV_ALPHA);
	xgeShapeExDraw(pTarget, 0.5f);
	xgeShapeExDestroy(pTarget);
	xgeShapeExDestroy(pMask);

	/* Luma 遮罩：用渐变亮度做遮罩 */
	{
		xge_shape_ex_color_stop_t stops[2];
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(0, 0, 0, 255);

		xgeShapeExCreate(&pMask);
		xgeShapeExAppendRect(pMask, 80, 320, 250, 180, 0, 0, 1);
		xgeShapeExFillLinearGradient(pMask, 80, 410, 330, 410,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);

		xgeShapeExCreate(&pTarget);
		xgeShapeExAppendRect(pTarget, 80, 320, 250, 180, 0, 0, 1);
		xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(255, 107, 94, 255));
		xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_LUMA);
		xgeShapeExDraw(pTarget, 0.5f);
		xgeShapeExDestroy(pTarget);
		xgeShapeExDestroy(pMask);
	}

	/* Inv Luma 遮罩 */
	{
		xge_shape_ex_color_stop_t stops[2];
		stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(0, 0, 0, 255);

		xgeShapeExCreate(&pMask);
		xgeShapeExAppendRect(pMask, 420, 320, 250, 180, 0, 0, 1);
		xgeShapeExFillLinearGradient(pMask, 420, 410, 670, 410,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);

		xgeShapeExCreate(&pTarget);
		xgeShapeExAppendRect(pTarget, 420, 320, 250, 180, 0, 0, 1);
		xgeShapeExFillColor(pTarget, XGE_COLOR_RGBA(160, 200, 255, 255));
		xgeShapeExMaskShapeSet(pTarget, pMask, XGE_SHAPE_EX_MASK_INV_LUMA);
		xgeShapeExDraw(pTarget, 0.5f);
		xgeShapeExDestroy(pTarget);
		xgeShapeExDestroy(pMask);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch55", argc, argv);
}
