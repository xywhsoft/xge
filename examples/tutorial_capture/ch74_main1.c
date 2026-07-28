/* ch74 — SVG 尺寸与 viewBox */
#include "tut_capture.h"

static const char* SVG_VIEWBOX =
	"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100' width='100' height='100'>"
	"<circle cx='50' cy='50' r='40' fill='#4FD8C2'/>"
	"<rect x='30' y='30' width='40' height='40' fill='#FF6E5E' opacity='0.7'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_rect_t tDst, tViewBox;
	float fW, fH;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_VIEWBOX, (int)strlen(SVG_VIEWBOX));

	/* 获取原始尺寸 */
	xgeSvgGetSize(pSvg, &fW, &fH);

	/* 获取 viewBox */
	xgeSvgGetViewBox(pSvg, &tViewBox);

	/* 原始大小绘制 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 放大 2x */
	tDst.fX = 220; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 非等比缩放 */
	tDst.fX = 500; tDst.fY = 50; tDst.fW = 250; tDst.fH = 120;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* SetSize 修改尺寸 */
	xgeSvgSetSize(pSvg, 150, 150);
	tDst.fX = 100; tDst.fY = 320; tDst.fW = 150; tDst.fH = 150;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* SetOrigin 偏移 */
	xgeSvgSetOrigin(pSvg, 20, 20);
	tDst.fX = 400; tDst.fY = 320; tDst.fW = 150; tDst.fH = 150;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch74", argc, argv);
}
