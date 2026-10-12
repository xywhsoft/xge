/* ch120 - Clipboard round trip.
 * Clipboard data is platform-owned: copy it into application state immediately
 * after xgeClipboardGetText instead of retaining the returned pointer.
 */
#include "tut_capture.h"

static xge_font_t g_font;
static int g_font_loaded;
static int g_ran;
static int g_verified;
static char g_copied[96];

static const char* find_font(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc", "C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf"
	};
	int i;
	for ( i = 0; i < 3; ++i ) {
		FILE* file = fopen(paths[i], "rb");
		if ( file != NULL ) { fclose(file); return paths[i]; }
	}
	return NULL;
}

static void label(const char* text, float x, float y, uint32_t color)
{
	if ( g_font_loaded ) xgeTextDraw(&g_font, text, x, y, color);
}

static void replay_clipboard_round_trip(void)
{
	static const char sample[] = "XGE clipboard: \xE4\xBD\xA0\xE5\xA5\xBD";
	const char* text;
	/* A real input control would call these paths from Ctrl+C and Ctrl+V. */
	xgeClipboardSetText(sample);
	text = xgeClipboardGetText();
	if ( text != NULL ) {
		snprintf(g_copied, sizeof(g_copied), "%s", text);
		g_copied[sizeof(g_copied) - 1] = '\0';
	}
	g_verified = strcmp(g_copied, sample) == 0;
	printf("ch120 clipboard self-test: %s\n", g_verified ? "PASS" : "FAIL");
}

static void draw_scene(void)
{
	uint32_t status;
	if ( !g_ran ) {
		const char* path = find_font();
		if ( path != NULL && xgeFontLoad(&g_font, path, 22.0f) == XGE_OK ) g_font_loaded = 1;
		replay_clipboard_round_trip();
		g_ran = 1;
	}
	status = g_verified ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeRectFill((xge_rect_t){70, 58, 660, 80}, XGE_COLOR_RGBA(23, 34, 45, 255));
	xgeShapeRectStroke((xge_rect_t){70, 58, 660, 80}, 2.0f, status);
	xgeShapeCircleFill(108, 98, 16, status);
	label("XGE clipboard UTF-8 round trip", 142, 78, XGE_COLOR_RGBA(231, 238, 246, 255));
	label(g_verified ? "set -> get -> copied application buffer: PASS" : "clipboard self-test: FAIL", 142, 106, status);

	/* The arrow expresses ownership: platform clipboard -> copied app buffer. */
	xgeShapeRectFill((xge_rect_t){100, 220, 230, 105}, XGE_COLOR_RGBA(27, 39, 51, 255));
	xgeShapeRectStroke((xge_rect_t){100, 220, 230, 105}, 2.0f, XGE_COLOR_RGBA(109, 179, 242, 255));
	label("system clipboard", 133, 245, XGE_COLOR_RGBA(219, 228, 240, 255));
	label("xgeClipboardGetText()", 113, 280, XGE_COLOR_RGBA(148, 163, 184, 255));
	xgeShapeLine(350, 272, 450, 272, 7.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeTriangleFill((xge_vec2_t){465, 272}, (xge_vec2_t){440, 257}, (xge_vec2_t){440, 287}, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeRectFill((xge_rect_t){485, 220, 215, 105}, XGE_COLOR_RGBA(27, 39, 51, 255));
	xgeShapeRectStroke((xge_rect_t){485, 220, 215, 105}, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	label("your text buffer", 516, 245, XGE_COLOR_RGBA(219, 228, 240, 255));
	label("copy before next use", 515, 280, XGE_COLOR_RGBA(148, 163, 184, 255));

	xgeShapeRectFill((xge_rect_t){100, 375, 600, 85}, XGE_COLOR_RGBA(19, 27, 36, 255));
	xgeShapeRectStroke((xge_rect_t){100, 375, 600, 85}, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	label("read-back text", 125, 397, XGE_COLOR_RGBA(148, 163, 184, 255));
	label(g_copied, 125, 428, XGE_COLOR_RGBA(246, 248, 252, 255));
	label("Clipboard calls can fail or be unavailable on a target platform; keep paste optional.", 100, 535, XGE_COLOR_RGBA(148, 163, 184, 255));
}

int main(int argc, char** argv)
{
	int ret = tut_run(draw_scene, "ch120-clipboard", argc, argv);
	return (ret == 0 && (!g_ran || g_verified)) ? 0 : 1;
}
