/* ch105 — SpriteBatch */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[16 * 16 * 4];
	xge_texture_t tex;
	xge_sprite_batch_t batch;
	int x, y, i, ret;

	/* 生成 16x16 精灵纹理 */
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			int idx = (y * 16 + x) * 4;
			float dx = (x - 8.0f) / 8.0f;
			float dy = (y - 8.0f) / 8.0f;
			float dist = dx * dx + dy * dy;
			pixels[idx + 0] = 255;
			pixels[idx + 1] = (unsigned char)(180 - (int)(dist * 80));
			pixels[idx + 2] = 84;
			pixels[idx + 3] = (dist < 1.0f) ? 255 : 0;
		}
	}
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 16, 16, pixels);

	/* 初始化 SpriteBatch */
	ret = xgeSpriteBatchInit(&batch, &tex, 64, 0);
	if (ret != XGE_OK) { xgeTextureFree(&tex); return; }

	/* 批量添加精灵 */
	for (i = 0; i < 20; i++) {
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 16.0f; draw.tSrc.fH = 16.0f;
		draw.tDst.fX = 50.0f + (i % 5) * 140.0f;
		draw.tDst.fY = 50.0f + (i / 5) * 120.0f;
		draw.tDst.fW = 64.0f; draw.tDst.fH = 64.0f;
		draw.fRotation = (float)i * 0.3f;
		draw.tOrigin.fX = 32.0f; draw.tOrigin.fY = 32.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeSpriteBatchAdd(&batch, &draw);
	}

	/* 一次性提交绘制 */
	xgeSpriteBatchFlush(&batch);
	xgeSpriteBatchFree(&batch);
	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch105", argc, argv); }
