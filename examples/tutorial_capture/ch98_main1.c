/* ch98 — EGL 离屏上下文 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_offscreen_t offscreen;
	xge_render_target pRT;
	xge_texture tex;
	int ret;

	/* 初始化离屏上下文（内含 EGL） */
	ret = xgeOffscreenInit(&offscreen, 200, 150);
	if (ret != XGE_OK) return;

	pRT = xgeOffscreenRenderTarget(&offscreen);

	/* 在离屏上下文中渲染 */
	{
		xge_pass_t pass;
		xgePassInit(&pass, pRT, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(20, 30, 50, 255));
		ret = xgePassBegin(&pass);
		if (ret == XGE_OK) {
			xgeShapeEllipseFill(100.0f, 75.0f, 70.0f, 50.0f, XGE_COLOR_RGBA(255, 120, 80, 255));
			xgeShapeLine(20.0f, 20.0f, 180.0f, 130.0f, 3.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
			xgePassEnd(&pass);
		}
	}

	/* 获取纹理并绘制到主场景 */
	tex = xgeRenderTargetTexture(pRT);
	xgeDraw(tex, 80.0f, 60.0f);

	/* 放大绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = tex;
		draw.tSrc.fW = 200.0f; draw.tSrc.fH = 150.0f;
		draw.tDst.fX = 380.0f; draw.tDst.fY = 50.0f;
		draw.tDst.fW = 320.0f; draw.tDst.fH = 240.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeOffscreenUnit(&offscreen);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch98", argc, argv); }
