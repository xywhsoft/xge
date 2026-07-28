/* ch92 — 图像保存 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[128 * 128 * 4];
	xge_texture_t tex;
	int x, y;

	/* 生成彩色棋盘格图案 */
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 128; x++) {
			int idx = (y * 128 + x) * 4;
			int checker = ((x / 16) + (y / 16)) & 1;
			if (checker) {
				pixels[idx + 0] = 79; pixels[idx + 1] = 216; pixels[idx + 2] = 194;
			} else {
				pixels[idx + 0] = 30; pixels[idx + 1] = 40; pixels[idx + 2] = 55;
			}
			pixels[idx + 3] = 255;
		}
	}

	/* 保存为 PNG */
	xgeImageSavePNG("ch92_output.png", 128, 128, pixels, 128 * 4);

	/* 显示该图像 */
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 128, 128, pixels);
	xgeDraw(&tex, 100.0f, 60.0f);
	xgeTextureFree(&tex);

	/* 绘制保存成功提示区域 */
	xgeShapeRectFill((xge_rect_t){100.0f, 220.0f, 128.0f, 30.0f}, XGE_COLOR_RGBA(40, 60, 40, 200));
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch92", argc, argv); }
