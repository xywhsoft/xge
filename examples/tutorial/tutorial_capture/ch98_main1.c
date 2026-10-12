/* ch98 — EGL 离屏上下文
 * 能力探测必须与真正创建独立 EGL 上下文分离。本示例运行在默认 XGE 上下文中，
 * 只展示当前构建是否编入 EGL/PBuffer/Surfaceless 支持和可用回退路径。
 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_egl_caps_t caps;
	uint32_t compiled;

	memset(&caps, 0, sizeof(caps));
	(void)xgeEGLCapsGet(&caps);
	compiled = caps.bCompiled ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);

	/* 顶部状态栏：绿色表示当前 DLL 编入该 EGL 功能，灰色表示走 RenderTarget 回退。 */
	xgeShapeRectFill((xge_rect_t){85, 105, 630, 76}, XGE_COLOR_RGBA(18, 29, 38, 255));
	xgeShapeRectStroke((xge_rect_t){85, 105, 630, 76}, 2.0f, compiled);
	xgeShapeCircleFill(130, 143, 18, compiled);
	xgeShapeLine(170, 143, 650, 143, 7.0f, compiled);

	/* 四个能力格对应 xge_egl_caps_t 的公开字段。 */
	for ( int i = 0; i < 4; ++i ) {
		int enabled = (i == 0) ? caps.bOffscreen : (i == 1) ? caps.bPBuffer : (i == 2) ? caps.bSurfaceless : caps.bNativeWindow;
		uint32_t color = enabled ? XGE_COLOR_RGBA(109, 179, 242, 255) : XGE_COLOR_RGBA(70, 85, 100, 255);
		float x = 105.0f + i * 165.0f;
		xgeShapeRectFill((xge_rect_t){x, 240, 135, 115}, XGE_COLOR_RGBA(24, 36, 48, 255));
		xgeShapeRectStroke((xge_rect_t){x, 240, 135, 115}, 2.0f, color);
		xgeShapeCircleFill(x + 67.5f, 282, 20, color);
		xgeShapeLine(x + 32, 325, x + 103, 325, 5.0f, color);
	}

	/* 下方的目标纹理代表所有平台均可使用的 RenderTarget 回退。 */
	xgeShapeRectFill((xge_rect_t){170, 420, 460, 80}, XGE_COLOR_RGBA(43, 62, 78, 255));
	xgeShapeRectStroke((xge_rect_t){170, 420, 460, 80}, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(220, 460, 20, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeLine(260, 460, 575, 460, 6.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch98", argc, argv); }
