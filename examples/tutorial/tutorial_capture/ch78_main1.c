/* ch78 — Paint 节点变换与属性 */
#include "tut_capture.h"
#include <math.h>

static const char* SVG_ICON =
	"<svg xmlns='http://www.w3.org/2000/svg' width='100' height='100'>"
	"<rect id='body' x='20' y='30' width='60' height='50' rx='6' fill='#4FD8C2'/>"
	"<circle id='lens' cx='50' cy='25' r='15' fill='#FFB454'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_svg_paint pPaint = NULL;
	xge_rect_t tDst;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_ICON, (int)strlen(SVG_ICON));

	/* 原始 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* Paint 变换：平移 */
	xgeSvgPaintGetByName(pSvg, "body", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintTransformTranslate(pPaint, 150, 0);
	}
	tDst.fX = 50; tDst.fY = 200; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* Paint 变换：旋转 */
	xgeSvgPaintGetByName(pSvg, "lens", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintTransformTranslate(pPaint, 50, 25);
		xgeSvgPaintTransformRotate(pPaint, 0.8f);
		xgeSvgPaintTransformTranslate(pPaint, -50, -25);
	}
	tDst.fX = 300; tDst.fY = 200; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* Paint 透明度 */
	xgeSvgPaintGetByName(pSvg, "body", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintOpacitySet(pPaint, 0.4f);
	}
	xgeSvgPaintGetByName(pSvg, "lens", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintOpacitySet(pPaint, 0.6f);
	}
	tDst.fX = 500; tDst.fY = 200; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* Paint 缩放 */
	xgeSvgPaintGetByName(pSvg, "body", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintTransformScale(pPaint, 1.5f, 0.7f);
		xgeSvgPaintOpacitySet(pPaint, 1.0f);
	}
	xgeSvgPaintGetByName(pSvg, "lens", &pPaint);
	if (pPaint) {
		xgeSvgPaintTransformIdentity(pPaint);
		xgeSvgPaintOpacitySet(pPaint, 1.0f);
	}
	tDst.fX = 100; tDst.fY = 400; tDst.fW = 150; tDst.fH = 150;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch78", argc, argv);
}
