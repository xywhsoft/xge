/* ch79 — Paint 树遍历与遮罩 */
#include "tut_capture.h"

static const char* SVG_TREE =
	"<svg xmlns='http://www.w3.org/2000/svg' width='200' height='200'>"
	"<g id='group1'>"
	"<rect x='20' y='20' width='70' height='70' rx='8' fill='#4FD8C2'/>"
	"<circle cx='140' cy='55' r='35' fill='#FFB454'/>"
	"</g>"
	"<g id='group2'>"
	"<rect x='20' y='110' width='160' height='70' rx='8' fill='#FF6E5E'/>"
	"</g>"
	"</svg>";

static int paint_visitor(xge_svg_paint pPaint, void* pUser)
{
	int* pCount = (int*)pUser;
	(*pCount)++;
	return XGE_OK;
}

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_svg_paint pPaint = NULL, pRoot = NULL;
	xge_rect_t tDst;
	int iCount = 0;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_TREE, (int)strlen(SVG_TREE));

	/* 原始绘制 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 获取根 Paint 并遍历 */
	xgeSvgPaintGetPicture(pSvg, &pRoot);
	if (pRoot) {
		xgeSvgPaintTraverse(pRoot, paint_visitor, &iCount);
	}

	/* 子节点访问 */
	if (pRoot) {
		int iChildCount = 0;
		xgeSvgPaintChildGetCount(pRoot, &iChildCount);
		if (iChildCount > 0) {
			xge_svg_paint pChild = NULL;
			xgeSvgPaintChildGetAt(pRoot, 0, &pChild);
			if (pChild) {
				const char* sName = NULL;
				xgeSvgPaintNameGet(pChild, &sName);
			}
		}
	}

	/* Paint 遮罩 */
	xgeSvgPaintGetByName(pSvg, "group1", &pPaint);
	if (pPaint) {
		xge_shape_ex pMask = NULL;
		xgeShapeExCreate(&pMask);
		xgeShapeExAppendCircle(pMask, 100, 70, 60, 60, 1);
		xgeShapeExFillColor(pMask, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeSvgPaintMaskShapeSet(pPaint, pMask, XGE_SHAPE_EX_MASK_ALPHA);
		xgeShapeExDestroy(pMask);
	}

	/* 遮罩后绘制 */
	tDst.fX = 350; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 清除遮罩 */
	xgeSvgPaintGetByName(pSvg, "group1", &pPaint);
	if (pPaint) {
		xgeSvgPaintMaskClear(pPaint);
	}

	/* Paint 裁剪 */
	xgeSvgPaintGetByName(pSvg, "group2", &pPaint);
	if (pPaint) {
		xge_shape_ex pClip = NULL;
		xgeShapeExCreate(&pClip);
		xgeShapeExAppendCircle(pClip, 100, 145, 50, 50, 1);
		xgeSvgPaintClipShapeSet(pPaint, pClip);
		xgeShapeExDestroy(pClip);
	}

	tDst.fX = 100; tDst.fY = 330; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch79", argc, argv);
}
