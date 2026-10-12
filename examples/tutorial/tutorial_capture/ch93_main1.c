/* ch93 — 纹理创建与加载 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[64 * 64 * 4];
	xge_texture_t tex1, tex2;
	int x, y;

	/* 程序化生成圆形渐变图案 */
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			int idx = (y * 64 + x) * 4;
			float dx = (x - 32.0f) / 32.0f;
			float dy = (y - 32.0f) / 32.0f;
			float dist = dx * dx + dy * dy;
			if (dist < 1.0f) {
				pixels[idx + 0] = 255; pixels[idx + 1] = 180; pixels[idx + 2] = 84;
			} else {
				pixels[idx + 0] = 0; pixels[idx + 1] = 0; pixels[idx + 2] = 0;
			}
			pixels[idx + 3] = (dist < 1.0f) ? 255 : 0;
		}
	}

	/* 从 RGBA 像素创建纹理 */
	memset(&tex1, 0, sizeof(tex1));
	xgeTextureCreateRGBA(&tex1, 64, 64, pixels);
	xgeDraw(&tex1, 80.0f, 80.0f);

	/* 先保存再从文件加载 */
	xgeImageSavePNG("ch93_tex.png", 64, 64, pixels, 64 * 4);
	memset(&tex2, 0, sizeof(tex2));
	xgeTextureLoad(&tex2, "ch93_tex.png");
	xgeDraw(&tex2, 250.0f, 80.0f);

	/* 缩放绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex2;
		draw.tSrc.fW = 64.0f; draw.tSrc.fH = 64.0f;
		draw.tDst.fX = 420.0f; draw.tDst.fY = 60.0f;
		draw.tDst.fW = 128.0f; draw.tDst.fH = 128.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeTextureFree(&tex1);
	xgeTextureFree(&tex2);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch93", argc, argv); }
