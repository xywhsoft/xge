/* ch97 — Render Target */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_render_target_t rt;
	xge_texture tex;
	int ret;

	/* 创建离屏渲染目标 */
	ret = xgeRenderTargetCreate(&rt, 200, 200);
	if (ret != XGE_OK) return;

	/* 渲染到离屏目标 */
	{
		xge_pass_t pass;
		xgePassInit(&pass, &rt, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(40, 60, 80, 255));
		ret = xgePassBegin(&pass);
		if (ret == XGE_OK) {
			xgeShapeCircleFill(100.0f, 100.0f, 60.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
			xgeShapeRectFill((xge_rect_t){60.0f, 60.0f, 80.0f, 80.0f}, XGE_COLOR_RGBA(79, 216, 194, 180));
			xgePassEnd(&pass);
		}
	}

	/* 获取渲染目标纹理并绘制到主场景 */
	tex = xgeRenderTargetTexture(&rt);
	xgeDraw(tex, 80.0f, 60.0f);

	/* 缩放绘制 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = tex;
		draw.tSrc.fW = 200.0f; draw.tSrc.fH = 200.0f;
		draw.tDst.fX = 380.0f; draw.tDst.fY = 40.0f;
		draw.tDst.fW = 300.0f; draw.tDst.fH = 300.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeDrawEx(&draw);
	}

	xgeRenderTargetFree(&rt);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch97", argc, argv); }
