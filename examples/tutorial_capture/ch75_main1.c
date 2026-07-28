/* ch75 — SVG 绘制 */
#include "tut_capture.h"

static const char* SVG_SHAPES =
	"<svg xmlns='http://www.w3.org/2000/svg' width='200' height='200'>"
	"<rect x='10' y='10' width='80' height='60' rx='8' fill='#4FD8C2'/>"
	"<circle cx='150' cy='50' r='40' fill='#FFB454'/>"
	"<polygon points='100,120 130,180 70,180' fill='#FF6E5E'/>"
	"<ellipse cx='50' cy='160' rx='40' ry='25' fill='#A0C8FF'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_rect_t tDst;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_SHAPES, (int)strlen(SVG_SHAPES));

	/* 标准绘制 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 像素对齐绘制 */
	tDst.fX = 320; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDrawPx(pSvg, tDst, 0.5f);

	/* 放大绘制 */
	tDst.fX = 50; tDst.fY = 320; tDst.fW = 300; tDst.fH = 300;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 缩小绘制 */
	tDst.fX = 450; tDst.fY = 350; tDst.fW = 80; tDst.fH = 80;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 获取绘制边界 */
	{
		xge_rect_t tBounds;
		tDst.fX = 580; tDst.fY = 320; tDst.fW = 180; tDst.fH = 180;
		xgeSvgGetDrawBounds(pSvg, tDst, 0.5f, &tBounds);
		xgeSvgDraw(pSvg, tDst, 0.5f);
		/* 绘制边界框 */
		{
			xge_shape_ex pFrame = NULL;
			xgeShapeExCreate(&pFrame);
			xgeShapeExAppendRect(pFrame, tBounds.fX, tBounds.fY, tBounds.fW, tBounds.fH, 0, 0, 1);
			xgeShapeExStrokeColor(pFrame, XGE_COLOR_RGBA(255, 255, 255, 100));
			xgeShapeExStrokeWidth(pFrame, 1.0f);
			xgeShapeExDraw(pFrame, 0.5f);
			xgeShapeExDestroy(pFrame);
		}
	}

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch75", argc, argv);
}
