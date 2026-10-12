/* ch99 — Offscreen 封装 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_offscreen_t offscreen;
	xge_render_target pRT;
	unsigned char pixels[160 * 120 * 4];
	xge_texture_t tex;
	int ret;

	/* Offscreen 封装：一步完成 EGL + RenderTarget */
	ret = xgeOffscreenInit(&offscreen, 160, 120);
	if (ret != XGE_OK) return;

	pRT = xgeOffscreenRenderTarget(&offscreen);

	/* 渲染场景到离屏 */
	{
		xge_pass_t pass;
		xgePassInit(&pass, pRT, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(30, 20, 50, 255));
		ret = xgePassBegin(&pass);
		if (ret == XGE_OK) {
			xgeShapeRoundRectFill((xge_rect_t){20.0f, 20.0f, 120.0f, 80.0f}, 12.0f, XGE_COLOR_RGBA(100, 200, 255, 255));
			xgeShapeCircleFill(80.0f, 60.0f, 30.0f, XGE_COLOR_RGBA(255, 200, 80, 220));
			xgePassEnd(&pass);
		}
	}

	/* 读取像素 */
	ret = xgeOffscreenReadPixels(&offscreen, pixels, 160 * 4);
	if (ret == XGE_OK) {
		/* 用读回的像素创建纹理显示 */
		memset(&tex, 0, sizeof(tex));
		xgeTextureCreateRGBA(&tex, 160, 120, pixels);
		xgeDraw(&tex, 80.0f, 60.0f);

		/* 放大显示 */
		{
			xge_draw_t draw;
			memset(&draw, 0, sizeof(draw));
			draw.pTexture = &tex;
			draw.tSrc.fW = 160.0f; draw.tSrc.fH = 120.0f;
			draw.tDst.fX = 360.0f; draw.tDst.fY = 40.0f;
			draw.tDst.fW = 320.0f; draw.tDst.fH = 240.0f;
			draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			xgeDrawEx(&draw);
		}
		xgeTextureFree(&tex);
	}

	xgeOffscreenUnit(&offscreen);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch99", argc, argv); }
