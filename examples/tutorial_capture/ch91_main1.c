/* ch91 — 图像加载 */
#include "tut_capture.h"

/* 生成 64x64 渐变测试图像并保存为 PNG，再加载回来 */
static void draw_scene(void)
{
	xge_image_t img;
	xge_texture_t tex;
	unsigned char pixels[64 * 64 * 4];
	int x, y, ret;

	/* 程序化生成 RGBA 像素 */
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 64; x++) {
			int idx = (y * 64 + x) * 4;
			pixels[idx + 0] = (unsigned char)(x * 4);
			pixels[idx + 1] = (unsigned char)(y * 4);
			pixels[idx + 2] = 128;
			pixels[idx + 3] = 255;
		}
	}

	/* 保存为 PNG 文件 */
	xgeImageSavePNG("ch91_test.png", 64, 64, pixels, 64 * 4);

	/* 从文件加载图像 */
	memset(&img, 0, sizeof(img));
	ret = xgeImageLoad(&img, "ch91_test.png");
	if (ret != XGE_OK) return;

	/* 创建纹理并绘制 */
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateFromImage(&tex, &img);
	xgeDraw(&tex, 100.0f, 80.0f);

	/* 放大绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 64.0f; draw.tSrc.fH = 64.0f;
		draw.tDst.fX = 300.0f; draw.tDst.fY = 80.0f;
		draw.tDst.fW = 256.0f; draw.tDst.fH = 256.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeTextureFree(&tex);
	xgeImageFree(&img);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch91", argc, argv); }
