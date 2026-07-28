/* ch101 — Render Pass */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_render_target_t rt1, rt2;
	xge_pass_t pass;
	xge_texture tex;
	int ret;

	/* 创建两个渲染目标 */
	ret = xgeRenderTargetCreate(&rt1, 180, 180);
	if (ret != XGE_OK) return;
	ret = xgeRenderTargetCreate(&rt2, 180, 180);
	if (ret != XGE_OK) { xgeRenderTargetFree(&rt1); return; }

	/* Pass 1: 渲染红色圆形 */
	xgePassInit(&pass, &rt1, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(20, 20, 40, 255));
	ret = xgePassBegin(&pass);
	if (ret == XGE_OK) {
		xgeShapeCircleFill(90.0f, 90.0f, 60.0f, XGE_COLOR_RGBA(255, 80, 80, 255));
		xgePassEnd(&pass);
	}

	/* Pass 2: 渲染蓝色矩形 */
	xgePassInit(&pass, &rt2, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(20, 20, 40, 255));
	ret = xgePassBegin(&pass);
	if (ret == XGE_OK) {
		xgeShapeRectFill((xge_rect_t){30.0f, 30.0f, 120.0f, 120.0f}, XGE_COLOR_RGBA(80, 120, 255, 255));
		xgePassEnd(&pass);
	}

	/* 将两个 pass 结果绘制到主场景 */
	tex = xgeRenderTargetTexture(&rt1);
	xgeDraw(tex, 60.0f, 60.0f);

	tex = xgeRenderTargetTexture(&rt2);
	xgeDraw(tex, 320.0f, 60.0f);

	xgeRenderTargetFree(&rt1);
	xgeRenderTargetFree(&rt2);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch101", argc, argv); }
