/* ch73 — SVG 加载与缓存 */
#include "tut_capture.h"

static const char* SVG_DATA =
	"<svg xmlns='http://www.w3.org/2000/svg' width='200' height='200'>"
	"<circle cx='100' cy='100' r='80' fill='#4FD8C2'/>"
	"<rect x='60' y='60' width='80' height='80' rx='10' fill='#FFB454' opacity='0.8'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_rect_t tDst;

	/* 从内存加载 SVG */
	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_DATA, (int)strlen(SVG_DATA));

	/* 绘制到指定区域 */
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 200; tDst.fH = 200;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 缩放到不同尺寸 */
	tDst.fX = 320; tDst.fY = 50; tDst.fW = 100; tDst.fH = 100;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	tDst.fX = 480; tDst.fY = 50; tDst.fW = 300; tDst.fH = 300;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	xgeSvgDestroy(pSvg);

	/* 缓存加载演示（LoadCached）*/
	{
		xge_svg pCached = NULL;
		/* 注意：LoadCached 需要文件路径，这里用 Clone 代替演示 */
		xgeSvgCreate(&pSvg);
		xgeSvgLoadMemory(pSvg, SVG_DATA, (int)strlen(SVG_DATA));
		xgeSvgClone(pSvg, &pCached);

		tDst.fX = 100; tDst.fY = 350; tDst.fW = 150; tDst.fH = 150;
		xgeSvgDraw(pCached, tDst, 0.5f);

		xgeSvgDestroy(pCached);
		xgeSvgDestroy(pSvg);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch73", argc, argv);
}
