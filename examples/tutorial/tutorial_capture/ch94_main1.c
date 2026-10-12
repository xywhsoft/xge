/* ch94 — YUV420P 纹理 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_texture_t tex;
	unsigned char yPlane[128 * 128];
	unsigned char uPlane[64 * 64];
	unsigned char vPlane[64 * 64];
	int x, y;

	/* 生成 YUV420P 测试图案：渐变亮度 + 固定色度 */
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 128; x++) {
			yPlane[y * 128 + x] = (unsigned char)((x + y) & 0xFF);
		}
	}
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			uPlane[y * 64 + x] = 128;  /* 无色偏 */
			vPlane[y * 64 + x] = 200;  /* 偏红 */
		}
	}

	/* 创建 YUV420P 纹理 */
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateYUV420P(&tex, 128, 128);
	xgeTextureUpdateYUV420P(&tex, yPlane, 128, uPlane, 64, vPlane, 64);

	/* 绘制 */
	xgeDraw(&tex, 100.0f, 80.0f);

	/* 放大绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 128.0f; draw.tSrc.fH = 128.0f;
		draw.tDst.fX = 350.0f; draw.tDst.fY = 60.0f;
		draw.tDst.fW = 256.0f; draw.tDst.fH = 256.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch94", argc, argv); }
