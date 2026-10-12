/* ch107 — 九宫格绘制 */
#include "tut_capture.h"

static void draw_scene(void)
{
	unsigned char pixels[48 * 48 * 4];
	xge_texture_t tex;
	xge_nine_patch_t patch;
	int x, y;

	/* 生成 48x48 带边框的纹理（模拟按钮背景） */
	for (y = 0; y < 48; y++) {
		for (x = 0; x < 48; x++) {
			int idx = (y * 48 + x) * 4;
			int border = (x < 4 || x >= 44 || y < 4 || y >= 44);
			if (border) {
				pixels[idx + 0] = 100; pixels[idx + 1] = 160; pixels[idx + 2] = 220;
			} else {
				pixels[idx + 0] = 40; pixels[idx + 1] = 60; pixels[idx + 2] = 90;
			}
			pixels[idx + 3] = 255;
		}
	}
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 48, 48, pixels);

	/* 九宫格初始化：指定拉伸区域边界 */
	{
		xge_rect_t src = {0.0f, 0.0f, 48.0f, 48.0f};
		xgeNinePatchInit(&patch, &tex, src, 8.0f, 8.0f, 40.0f, 40.0f);
	}

	/* 拉伸绘制到不同尺寸 */
	xgeNinePatchDraw(&patch, (xge_rect_t){60.0f, 60.0f, 200.0f, 80.0f}, 0);
	xgeNinePatchDraw(&patch, (xge_rect_t){60.0f, 180.0f, 300.0f, 120.0f}, 0);
	xgeNinePatchDraw(&patch, (xge_rect_t){420.0f, 60.0f, 150.0f, 250.0f}, 0);

	/* Tile 模式 */
	xgeNinePatchSetMode(&patch, XGE_NINE_PATCH_TILE);
	xgeNinePatchDraw(&patch, (xge_rect_t){420.0f, 350.0f, 200.0f, 100.0f}, 0);

	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch107", argc, argv); }
