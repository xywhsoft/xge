/* ch119 - Gamepad state injection and polling.
 * The public SetState API makes the chapter repeatable without a physical pad.
 * A shipping game normally only polls; injection is reserved for tests/tools.
 */
#include "tut_capture.h"

#include <math.h>

static xge_font_t g_font;
static int g_font_loaded;
static int g_ran;
static int g_verified;
static xge_gamepad_state_t g_state;

static const char* find_font(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf"
	};
	int i;
	for ( i = 0; i < 2; ++i ) {
		FILE* file = fopen(paths[i], "rb");
		if ( file != NULL ) { fclose(file); return paths[i]; }
	}
	return NULL;
}

static void label(const char* text, float x, float y, uint32_t color)
{
	if ( g_font_loaded ) xgeTextDraw(&g_font, text, x, y, color);
}

static void replay_gamepad_state(void)
{
	xge_gamepad_state_t injected;
	memset(&injected, 0, sizeof(injected));
	injected.bConnected = 1;
	injected.iButtons = 0x00000001u; /* application-defined button-bit mapping */
	injected.arrAxes[0] = 0.68f;     /* left stick X */
	injected.arrAxes[1] = -0.42f;    /* left stick Y */
	(void)xgeGamepadSetState(0, &injected);
	(void)xgeGamepadGetState(0, &g_state);
	g_verified = xgeGamepadConnected(0) &&
		xgeGamepadButtonDown(0, 0x00000001u) &&
		xgeGamepadButtonPressed(0, 0x00000001u) &&
		fabsf(xgeGamepadAxis(0, 0) - 0.68f) < 0.001f &&
		fabsf(xgeGamepadAxis(0, 1) + 0.42f) < 0.001f;
	printf("ch119 gamepad self-test: %s\n", g_verified ? "PASS" : "FAIL");
}

static void draw_scene(void)
{
	float cx = 390.0f, cy = 338.0f;
	float knob_x, knob_y;
	uint32_t status;
	if ( !g_ran ) {
		const char* path = find_font();
		if ( path != NULL && xgeFontLoad(&g_font, path, 22.0f) == XGE_OK ) g_font_loaded = 1;
		replay_gamepad_state();
		g_ran = 1;
	}
	status = g_verified ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);
	knob_x = cx + g_state.arrAxes[0] * 88.0f;
	knob_y = cy + g_state.arrAxes[1] * 88.0f;
	xgeShapeRectFill((xge_rect_t){72, 58, 656, 78}, XGE_COLOR_RGBA(23, 34, 45, 255));
	xgeShapeRectStroke((xge_rect_t){72, 58, 656, 78}, 2.0f, status);
	xgeShapeCircleFill(110, 97, 16, status);
	label("XGE gamepad state polling", 142, 78, XGE_COLOR_RGBA(231, 238, 246, 255));
	label(g_verified ? "synthetic controller replay: PASS" : "synthetic controller replay: FAIL", 142, 106, status);

	/* The outer ring is the normalized [-1, 1] stick range. */
	xgeShapeCircleFill(cx, cy, 110, XGE_COLOR_RGBA(27, 39, 51, 255));
	xgeShapeCircleStroke(cx, cy, 110, 2.0f, XGE_COLOR_RGBA(109, 179, 242, 255));
	xgeShapeLine(cx - 110, cy, cx + 110, cy, 1.0f, XGE_COLOR_RGBA(72, 93, 115, 255));
	xgeShapeLine(cx, cy - 110, cx, cy + 110, 1.0f, XGE_COLOR_RGBA(72, 93, 115, 255));
	xgeShapeLine(cx, cy, knob_x, knob_y, 4.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(knob_x, knob_y, 21, XGE_COLOR_RGBA(255, 180, 84, 255));
	label("axis[0] = +0.68", 120, 490, XGE_COLOR_RGBA(219, 228, 240, 255));
	label("axis[1] = -0.42", 120, 522, XGE_COLOR_RGBA(219, 228, 240, 255));

	xgeShapeRectFill((xge_rect_t){565, 278, 110, 110}, XGE_COLOR_RGBA(30, 43, 56, 255));
	xgeShapeRectStroke((xge_rect_t){565, 278, 110, 110}, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeCircleFill(620, 333, 27, xgeGamepadButtonDown(0, 0x00000001u) ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(72, 93, 115, 255));
	label("button bit 0", 565, 416, XGE_COLOR_RGBA(219, 228, 240, 255));
	label("Use SetState only for tests and editor tooling.", 142, 555, XGE_COLOR_RGBA(148, 163, 184, 255));
}

int main(int argc, char** argv)
{
	int ret = tut_run(draw_scene, "ch119-gamepad", argc, argv);
	return (ret == 0 && (!g_ran || g_verified)) ? 0 : 1;
}
