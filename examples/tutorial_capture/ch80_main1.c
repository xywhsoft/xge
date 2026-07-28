/* ch80 — SVG 光栅化与纹理 */
#include "tut_capture.h"

static const char* SVG_RASTER =
	"<svg xmlns='http://www.w3.org/2000/svg' width='120' height='120'>"
	"<circle cx='60' cy='60' r='50' fill='#4FD8C2'/>"
	"<rect x='35' y='35' width='50' height='50' rx='8' fill='#FFB454' opacity='0.85'/>"
	"<circle cx='60' cy='60' r='15' fill='#FF6E5E'/>"
	"</svg>";

static void draw_scene(void)
{
	xge_svg pSvg = NULL;
	xge_rect_t tDst;

	xgeSvgCreate(&pSvg);
	xgeSvgLoadMemory(pSvg, SVG_RASTER, (int)strlen(SVG_RASTER));

	/* 矢量绘制（正常方式）*/
	tDst.fX = 50; tDst.fY = 50; tDst.fW = 120; tDst.fH = 120;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 放大矢量绘制 */
	tDst.fX = 250; tDst.fY = 50; tDst.fW = 240; tDst.fH = 240;
	xgeSvgDraw(pSvg, tDst, 0.5f);

	/* 光栅化到内存像素 */
	{
		unsigned char pixels[128 * 128 * 4];
		int iStride = 128 * 4;
		int ret = xgeSvgRasterizeMemory(SVG_RASTER, (int)strlen(SVG_RASTER), 128, 128, pixels, iStride);
		if (ret == XGE_OK) {
			/* 将光栅化结果显示为纹理（这里用矩形模拟位置标注）*/
			xge_shape_ex pFrame = NULL;
			xgeShapeExCreate(&pFrame);
			xgeShapeExAppendRect(pFrame, 560, 50, 128, 128, 4, 4, 1);
			xgeShapeExStrokeColor(pFrame, XGE_COLOR_RGBA(160, 200, 255, 200));
			xgeShapeExStrokeWidth(pFrame, 2.0f);
			xgeShapeExDraw(pFrame, 0.5f);
			xgeShapeExDestroy(pFrame);
		}
	}

	/* SVG 纹理加载（需要 GPU 纹理对象，这里演示 API 调用结构）*/
	{
		xge_texture pTex = NULL;
		/* 实际使用中需要先创建纹理：xgeTextureCreate(&pTex, ...) */
		/* xgeSvgTextureLoadMemory(pTex, SVG_RASTER, strlen(SVG_RASTER), 128, 128); */
		/* 然后用 xgeImageDraw 绘制纹理 */
	}

	/* 不同分辨率光栅化对比 */
	{
		unsigned char pix64[64 * 64 * 4];
		unsigned char pix256[256 * 256 * 4];
		xgeSvgRasterizeMemory(SVG_RASTER, (int)strlen(SVG_RASTER), 64, 64, pix64, 64 * 4);
		xgeSvgRasterizeMemory(SVG_RASTER, (int)strlen(SVG_RASTER), 256, 256, pix256, 256 * 4);

		/* 标注区域 */
		xge_shape_ex pF = NULL;
		xgeShapeExCreate(&pF);
		xgeShapeExAppendRect(pF, 50, 350, 64, 64, 2, 2, 1);
		xgeShapeExStrokeColor(pF, XGE_COLOR_RGBA(79, 216, 194, 200));
		xgeShapeExStrokeWidth(pF, 1.5f);
		xgeShapeExDraw(pF, 0.5f);
		xgeShapeExDestroy(pF);

		xgeShapeExCreate(&pF);
		xgeShapeExAppendRect(pF, 200, 300, 256, 256, 2, 2, 1);
		xgeShapeExStrokeColor(pF, XGE_COLOR_RGBA(255, 180, 84, 200));
		xgeShapeExStrokeWidth(pF, 1.5f);
		xgeShapeExDraw(pF, 0.5f);
		xgeShapeExDestroy(pF);
	}

	xgeSvgDestroy(pSvg);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch80", argc, argv);
}
