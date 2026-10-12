/* ch77 — Paint 节点获取 */
#include "tut_capture.h"

static const char* SVG_PAINT =
	"<svg xmlns='http://www.w3.org/2000/svg' width='200' height='200'>"
	"<rect id='bg' x='10' y='10' width='180' height='180' rx='12' fill='#2D3748'/>"
	"<circle id='dot1' cx='60' cy='80' r='30' fill='#4FD8C2'/>"
	"<circle id='dot2' cx='140' cy='80' r='30' fill='#FFB454'/>"
	"<rect id='bar' x='40' y='130' width='120' height='30' rx='6' fill='#FF6E5E'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_svg_paint pPaint = NULL;
	xge_rect_t tDst;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_PAINT, (int)strlen(SVG_PAINT));

	/* 原始绘制 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 按名称获取 Paint 节点 */
	xgeSvgPaintGetByName(pSvg, "dot1", &pPaint);
	if (pPaint) {
		/* 修改颜色/透明度 */
		xgeSvgPaintOpacitySet(pPaint, 0.4f);
	}

	xgeSvgPaintGetByName(pSvg, "dot2", &pPaint);
	if (pPaint) {
		xgeSvgPaintVisibleSet(pPaint, 0); /* 隐藏 */
	}

	/* 修改后绘制 */
	tDst.fX = 320; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 按 Id 获取 */
	{
		uint32_t iId = xgeShapeExIdFromName("bar");
		xgeSvgPaintGetById(pSvg, iId, &pPaint);
		if (pPaint) {
			xge_svg pOwner = NULL;
			xgeSvgPaintOwnerGet(pPaint, &pOwner);
		}
	}

	/* 获取 Picture（根 paint）*/
	{
		xge_svg_paint pRoot = NULL;
		xgeSvgPaintGetPicture(pSvg, &pRoot);
		if (pRoot) {
			int iChildCount = 0;
			xgeSvgPaintChildGetCount(pRoot, &iChildCount);
		}
	}

	/* 恢复后绘制 */
	xgeSvgPaintGetByName(pSvg, "dot1", &pPaint);
	if (pPaint) xgeSvgPaintOpacitySet(pPaint, 1.0f);
	xgeSvgPaintGetByName(pSvg, "dot2", &pPaint);
	if (pPaint) xgeSvgPaintVisibleSet(pPaint, 1);

	tDst.fX = 100; tDst.fY = 330; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch77", argc, argv);
}
