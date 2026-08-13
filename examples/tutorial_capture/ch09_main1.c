/* ch09 — 平台能力查询
 * 读取当前运行时返回的 capability flags，并将可用能力渲染成一组状态灯。
 */
#include "tut_capture.h"

static void capability(float x, float y, int enabled, uint32_t accent)
{
	uint32_t state = enabled ? accent : XGE_COLOR_RGBA(63, 77, 89, 255);
	xgeShapeRectFill((xge_rect_t){x, y, 220, 56}, XGE_COLOR_RGBA(22, 31, 40, 255));
	xgeShapeRectStroke((xge_rect_t){x, y, 220, 56}, 1.5f, state);
	xgeShapeCircleFill(x + 28, y + 28, 12, state);
	xgeShapeLine(x + 55, y + 28, x + 185, y + 28, 5.0f, state);
}

static void draw_scene(void)
{
	xge_platform_caps_t caps;
	/* 查询必须在 xgeInit 后进行；tut_run 已完成初始化。 */
	if ( xgePlatformCapsGet(&caps) != XGE_OK ) return;
	capability(150, 150, caps.bWindow, XGE_COLOR_RGBA(79, 216, 194, 255));
	capability(430, 150, caps.bRenderTarget, XGE_COLOR_RGBA(109, 179, 242, 255));
	capability(150, 250, caps.bKeyboard, XGE_COLOR_RGBA(255, 180, 84, 255));
	capability(430, 250, caps.bMouse, XGE_COLOR_RGBA(255, 107, 94, 255));
	capability(150, 350, caps.bAudio, XGE_COLOR_RGBA(79, 216, 194, 255));
	capability(430, 350, caps.bHighDPI, XGE_COLOR_RGBA(109, 179, 242, 255));
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch09-platform-caps", argc, argv); }
