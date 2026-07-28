/* ch96 — Fallback 与上传队列 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char fallbackPx[16 * 16 * 4];
	unsigned char pixels[64 * 64 * 4];
	xge_texture_t tex;
	int x, y;

	/* 设置全局 fallback 纹理（16x16 品红色） */
	for (y = 0; y < 16 * 16; y++) {
		fallbackPx[y * 4 + 0] = 255; fallbackPx[y * 4 + 1] = 0;
		fallbackPx[y * 4 + 2] = 255; fallbackPx[y * 4 + 3] = 255;
	}
	xgeTextureFallbackSetRGBA(16, 16, fallbackPx);

	/* 创建空纹理并获取 fallback */
	memset(&tex, 0, sizeof(tex));
	xgeTextureFallbackGet(&tex);
	xgeDraw(&tex, 80.0f, 80.0f);

	/* 生成实际像素并通过上传队列更新 */
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			int idx = (y * 64 + x) * 4;
			pixels[idx + 0] = (unsigned char)(x * 4);
			pixels[idx + 1] = (unsigned char)(255 - y * 4);
			pixels[idx + 2] = 100;
			pixels[idx + 3] = 255;
		}
	}
	xgeTextureCreateRGBA(&tex, 64, 64, pixels);
	xgeTextureUploadQueue(&tex);
	xgeTextureUploadFlush();

	/* 绘制上传后的纹理 */
	xgeDraw(&tex, 250.0f, 80.0f);

	xgeTextureFree(&tex);
	xgeTextureFallbackClear();
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch96", argc, argv); }
