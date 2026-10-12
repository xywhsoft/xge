/* ch95 — 采样器与纹理更新 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[32 * 32 * 4];
	xge_texture_t tex;
	xge_sampler_t sampler;
	int x, y;

	/* 生成小尺寸纹理以展示采样差异 */
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			int idx = (y * 32 + x) * 4;
			pixels[idx + 0] = (x < 16) ? 255 : 50;
			pixels[idx + 1] = (y < 16) ? 216 : 50;
			pixels[idx + 2] = 194;
			pixels[idx + 3] = 255;
		}
	}

	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 32, 32, pixels);

	/* 默认采样器（线性过滤） */
	sampler = xgeSamplerDefault();
	xgeTextureSetSampler(&tex, &sampler);
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 32.0f; draw.tSrc.fH = 32.0f;
		draw.tDst.fX = 60.0f; draw.tDst.fY = 60.0f;
		draw.tDst.fW = 192.0f; draw.tDst.fH = 192.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	/* 局部更新纹理 */
	{
		unsigned char patch[16 * 16 * 4];
		for (y = 0; y < 16 * 16; y++) {
			patch[y * 4 + 0] = 255; patch[y * 4 + 1] = 100;
			patch[y * 4 + 2] = 50; patch[y * 4 + 3] = 255;
		}
		xgeTextureUpdateRGBA(&tex, 8, 8, 16, 16, patch, 16 * 4);
	}

	/* 更新后绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 32.0f; draw.tSrc.fH = 32.0f;
		draw.tDst.fX = 350.0f; draw.tDst.fY = 60.0f;
		draw.tDst.fW = 192.0f; draw.tDst.fH = 192.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch95", argc, argv); }
