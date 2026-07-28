/* ch76 — preserveAspectRatio */
#include "tut_capture.h"

static const char* SVG_PAR =
	"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 60' width='100' height='60'>"
	"<rect x='5' y='5' width='90' height='50' rx='5' fill='#4FD8C2'/>"
	"<circle cx='50' cy='30' r='20' fill='#FFB454'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_rect_t tDst;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_PAR, (int)strlen(SVG_PAR));

	/* xMidYMid meet（默认）：居中适配 */
	xgeSvgSetPreserveAspectRatio(pSvg, "xMidYMid meet");
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);
	/* 目标区域边框 */
	{
		xge_shape_ex pF = NULL;
		xgeShapeExCreate(&pF);
		xgeShapeExAppendRect(pF, 50, 50, 200, 200, 0, 0, 1);
		xgeShapeExStrokeColor(pF, XGE_COLOR_RGBA(255, 255, 255, 60));
		xgeShapeExStrokeWidth(pF, 1.0f);
		xgeShapeExDraw(pF, 0.5f);
		xgeShapeExDestroy(pF);
	}

	/* xMinYMin meet：左上对齐 */
	xgeSvgSetPreserveAspectRatio(pSvg, "xMinYMin meet");
	tDst.fX = 320; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);
	{
		xge_shape_ex pF = NULL;
		xgeShapeExCreate(&pF);
		xgeShapeExAppendRect(pF, 320, 50, 200, 200, 0, 0, 1);
		xgeShapeExStrokeColor(pF, XGE_COLOR_RGBA(255, 255, 255, 60));
		xgeShapeExStrokeWidth(pF, 1.0f);
		xgeShapeExDraw(pF, 0.5f);
		xgeShapeExDestroy(pF);
	}

	/* xMidYMid slice：裁切填满 */
	xgeSvgSetPreserveAspectRatio(pSvg, "xMidYMid slice");
	tDst.fX = 590; tDst.fY = 50; tDst.fW = 160; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);
	{
		xge_shape_ex pF = NULL;
		xgeShapeExCreate(&pF);
		xgeShapeExAppendRect(pF, 590, 50, 160, 200, 0, 0, 1);
		xgeShapeExStrokeColor(pF, XGE_COLOR_RGBA(255, 255, 255, 60));
		xgeShapeExStrokeWidth(pF, 1.0f);
		xgeShapeExDraw(pF, 0.5f);
		xgeShapeExDestroy(pF);
	}

	/* none：拉伸填满 */
	xgeSvgSetPreserveAspectRatio(pSvg, "none");
	tDst.fX = 50; tDst.fY = 350; tDst.fW = 300; tDst.fH = 150;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* xMaxYMax meet */
	xgeSvgSetPreserveAspectRatio(pSvg, "xMaxYMax meet");
	tDst.fX = 450; tDst.fY = 350; tDst.fW = 250; tDst.fH = 150;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch76", argc, argv);
}
