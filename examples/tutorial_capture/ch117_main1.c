/* ch117 - UTF-8 text and IME composition with the ordered input-event API.
 *
 * The first frame posts a deterministic synthetic sequence so automated runs
 * verify the same code path used for native keyboard and IME events.  In an
 * interactive build, remove post_self_test() and keep consume_input_events():
 * native events arrive in precisely the same XGE_EVENT_* format.
 */
#include "tut_capture.h"

static xge_font_t g_font;
static int g_font_loaded;
static char g_committed[96];
static char g_composing[96];
static int g_event_count;
static int g_ran;
static int g_verified;

static const char* find_font(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
	};
	int i;
	for ( i = 0; i < (int)(sizeof(paths) / sizeof(paths[0])); ++i ) {
		FILE* file = fopen(paths[i], "rb");
		if ( file != NULL ) { fclose(file); return paths[i]; }
	}
	return NULL;
}

static void copy_utf8(char* dst, int capacity, const char* src, int size)
{
	if ( src == NULL ) src = "";
	if ( size < 0 ) size = (int)strlen(src);
	if ( size >= capacity ) size = capacity - 1;
	if ( size > 0 ) memcpy(dst, src, (size_t)size);
	dst[size] = '\0';
}

static void append_codepoint(char* dst, int capacity, uint32_t codepoint)
{
	char utf8[5] = {0};
	int size;
	if ( codepoint <= 0x7Fu ) { utf8[0] = (char)codepoint; size = 1; }
	else if ( codepoint <= 0x7FFu ) {
		utf8[0] = (char)(0xC0u | (codepoint >> 6));
		utf8[1] = (char)(0x80u | (codepoint & 0x3Fu)); size = 2;
	} else if ( codepoint <= 0xFFFFu ) {
		utf8[0] = (char)(0xE0u | (codepoint >> 12));
		utf8[1] = (char)(0x80u | ((codepoint >> 6) & 0x3Fu));
		utf8[2] = (char)(0x80u | (codepoint & 0x3Fu)); size = 3;
	} else {
		utf8[0] = (char)(0xF0u | (codepoint >> 18));
		utf8[1] = (char)(0x80u | ((codepoint >> 12) & 0x3Fu));
		utf8[2] = (char)(0x80u | ((codepoint >> 6) & 0x3Fu));
		utf8[3] = (char)(0x80u | (codepoint & 0x3Fu)); size = 4;
	}
	if ( (int)strlen(dst) + size < capacity ) strcat(dst, utf8);
}

static void post_event(int type, uint32_t codepoint, const char* text, int cursor)
{
	xge_input_event_t event;
	memset(&event, 0, sizeof(event));
	event.iSize = sizeof(event);
	event.iType = type;
	event.iCodepoint = codepoint;
	event.sText = text;
	event.iTextSize = (text != NULL) ? (int)strlen(text) : 0;
	event.iCursor = cursor;
	event.iSelectStart = cursor;
	event.iSelectEnd = cursor;
	(void)xgeInputEventPost(&event);
}

static void post_self_test(void)
{
	/* This sequence emulates: typing A, composing "ni", and committing 你好. */
	post_event(XGE_EVENT_TEXT, 'A', NULL, 0);
	post_event(XGE_EVENT_IME_START, 0, "", 0);
	post_event(XGE_EVENT_IME_UPDATE, 0, "ni", 2);
	post_event(XGE_EVENT_IME_COMMIT, 0, "\xE4\xBD\xA0\xE5\xA5\xBD", 6);
	post_event(XGE_EVENT_IME_END, 0, "", 0);
}

static void consume_input_events(void)
{
	xge_input_event_t event;
	while ( xgeInputEventGet(&event) > 0 ) {
		g_event_count++;
		switch ( event.iType ) {
		case XGE_EVENT_TEXT:
			append_codepoint(g_committed, (int)sizeof(g_committed), event.iCodepoint);
			break;
		case XGE_EVENT_IME_UPDATE:
			/* Copy now: event.sText expires on the next xgeInputEventGet call. */
			copy_utf8(g_composing, (int)sizeof(g_composing), event.sText, event.iTextSize);
			break;
		case XGE_EVENT_IME_COMMIT:
			if ( (int)strlen(g_committed) + event.iTextSize < (int)sizeof(g_committed) )
				strncat(g_committed, event.sText, (size_t)event.iTextSize);
			g_composing[0] = '\0';
			break;
		case XGE_EVENT_IME_END:
			g_composing[0] = '\0';
			break;
		default:
			break;
		}
	}
}

static void label(const char* text, float x, float y, uint32_t color)
{
	if ( g_font_loaded ) xgeTextDraw(&g_font, text, x, y, color);
}

static void draw_scene(void)
{
	uint32_t status;
	char status_text[96];
	if ( !g_ran ) {
		const char* path = find_font();
		if ( path != NULL && xgeFontLoad(&g_font, path, 22.0f) == XGE_OK ) g_font_loaded = 1;
		(void)xgeImeSetEnabled(1);
		post_self_test();
		consume_input_events();
		g_verified = (g_event_count == 5 && strcmp(g_committed, "A\xE4\xBD\xA0\xE5\xA5\xBD") == 0 && g_composing[0] == '\0');
		g_ran = 1;
		printf("ch117 ordered input self-test: %s (%d events)\n", g_verified ? "PASS" : "FAIL", g_event_count);
	}

	status = g_verified ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeRectFill((xge_rect_t){70, 58, 660, 82}, XGE_COLOR_RGBA(23, 34, 45, 255));
	xgeShapeRectStroke((xge_rect_t){70, 58, 660, 82}, 2.0f, status);
	xgeShapeCircleFill(108, 99, 16, status);
	label("XGE ordered input + IME composition", 142, 78, XGE_COLOR_RGBA(231, 238, 246, 255));
	snprintf(status_text, sizeof(status_text), "synthetic replay: %s / %d events", g_verified ? "PASS" : "FAIL", g_event_count);
	label(status_text, 142, 106, status);

	/* Committed text belongs to the document; composition is an overlay only. */
	xgeShapeRectFill((xge_rect_t){100, 190, 600, 106}, XGE_COLOR_RGBA(19, 27, 36, 255));
	xgeShapeRectStroke((xge_rect_t){100, 190, 600, 106}, 2.0f, XGE_COLOR_RGBA(109, 179, 242, 255));
	label("Committed UTF-8 text", 126, 210, XGE_COLOR_RGBA(148, 163, 184, 255));
	label(g_committed, 126, 245, XGE_COLOR_RGBA(246, 248, 252, 255));
	xgeShapeRectFill((xge_rect_t){100, 330, 600, 92}, XGE_COLOR_RGBA(32, 42, 55, 255));
	xgeShapeRectStroke((xge_rect_t){100, 330, 600, 92}, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	label("Composition overlay (cleared after IME_END)", 126, 350, XGE_COLOR_RGBA(255, 194, 115, 255));
	label(g_composing[0] ? g_composing : "<empty after commit>", 126, 383, XGE_COLOR_RGBA(225, 232, 240, 255));
	label("Do not mix xgeInputEventGet with legacy text/IME getters in one control.", 100, 495, XGE_COLOR_RGBA(148, 163, 184, 255));
	label("The replay is only for automation; native input uses the same event fields.", 100, 525, XGE_COLOR_RGBA(148, 163, 184, 255));
}

int main(int argc, char** argv)
{
	int ret = tut_run(draw_scene, "ch117-ime", argc, argv);
	return (ret == 0 && (!g_ran || g_verified)) ? 0 : 1;
}
