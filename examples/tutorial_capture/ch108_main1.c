/* ch108 — Draw 系列函数 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[32 * 32 * 4];
	xge_texture_t tex;
	int x, y;

	/* 生成测试纹理 */
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			int idx = (y * 32 + x) * 4;
			pixels[idx + 0] = (unsigned char)(x * 8);
			pixels[idx + 1] = 200;
			pixels[idx + 2] = (unsigned char)(y * 8);
			pixels[idx + 3] = 255;
		}
	}
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 32, 32, pixels);

	/* xgeDraw: 简单定位绘制 */
	xgeDraw(&tex, 60.0f, 50.0f);

	/* xgeDrawPx: 整数像素对齐 */
	xgeDrawPx(&tex, 150, 50);

	/* xgeDrawEx: 完整参数（缩放 + 旋转 + 颜色） */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 32.0f; draw.tSrc.fH = 32.0f;
		draw.tDst.fX = 280.0f; draw.tDst.fY = 50.0f;
		draw.tDst.fW = 96.0f; draw.tDst.fH = 96.0f;
		draw.tOrigin.fX = 48.0f; draw.tOrigin.fY = 48.0f;
		draw.fRotation = 0.4f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	/* xgeDrawEx: 翻转 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 32.0f; draw.tSrc.fH = 32.0f;
		draw.tDst.fX = 450.0f; draw.tDst.fY = 50.0f;
		draw.tDst.fW = 64.0f; draw.tDst.fH = 64.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		draw.iFlags = XGE_DRAW_FLIP_X;
		xgeDrawEx(&draw);
	}

	/* xgeDrawEx: 部分源区域 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fX = 8.0f; draw.tSrc.fY = 8.0f;
		draw.tSrc.fW = 16.0f; draw.tSrc.fH = 16.0f;
		draw.tDst.fX = 600.0f; draw.tDst.fY = 50.0f;
		draw.tDst.fW = 128.0f; draw.tDst.fH = 128.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 200, 150, 255);
		xgeDrawEx(&draw);
	}

	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch108", argc, argv); }
