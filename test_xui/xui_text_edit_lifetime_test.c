#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

typedef struct test_t {
	xui_context context;
	xui_widget root;
	xui_widget edit;
	xui_font font;
	xui_test_proxy_state_t proxy;
	const char* name;
	int failures;
	int callbacks;
	int contextDestroyed;
} test_t;

static xui_widget bars[2];
static int frees[2];
static int destroyCalls[2];
static int watching;

void __real_xuiWidgetDestroy(xui_widget widget);
void __real_xrtFree(void* pointer);

/* Observe calls using pointer identity only, never by inspecting freed widgets.
 * Forward unchanged so ASan still reports the original invalid access. */
void __wrap_xuiWidgetDestroy(xui_widget widget)
{
	int i;
	for (i = 0; watching && i < 2; i++) {
		if (widget == bars[i]) destroyCalls[i]++;
	}
	__real_xuiWidgetDestroy(widget);
}

void __wrap_xrtFree(void* pointer)
{
	int i;
	for (i = 0; watching && i < 2; i++) {
		if (pointer == bars[i]) frees[i]++;
	}
	__real_xrtFree(pointer);
}

#define CHECK(s, expr, message) do { if (!(expr)) { \
	printf("FAIL %s: %s (%d)\n", (s)->name, message, __LINE__); (s)->failures++; \
} } while (0)

static void watch_bars(test_t* s)
{
	bars[0] = xuiTextEditGetHScrollBarWidget(s->edit);
	bars[1] = xuiTextEditGetVScrollBarWidget(s->edit);
	memset(frees, 0, sizeof(frees));
	memset(destroyCalls, 0, sizeof(destroyCalls));
	CHECK(s, bars[0] != NULL && bars[1] != NULL && bars[0] != bars[1], "two distinct scrollbars");
	CHECK(s, xuiWidgetGetParent(bars[0]) == s->edit && xuiWidgetGetParent(bars[1]) == s->edit,
		"widget tree owns both scrollbars");
	watching = 1;
}

static void check_released(test_t* s)
{
	int i;
	watching = 0;
	for (i = 0; i < 2; i++) {
		CHECK(s, frees[i] == 1, "scrollbar storage freed exactly once");
		CHECK(s, destroyCalls[i] == 0, "TextEdit must not destroy a tree-owned scrollbar again");
	}
}

static int destroy_event(xui_widget widget, const xui_event_t* event, void* user)
{
	test_t* s = (test_t*)user;
	if (event->iType != XUI_EVENT_COMMAND) return XUI_OK;
	s->callbacks++;
	if (strcmp(s->name, "event_destroy_context") == 0) {
		xuiDestroy(s->context);
		s->contextDestroyed = 1;
	} else xuiWidgetDestroy(widget);
	CHECK(s, frees[0] == 0 && frees[1] == 0, "scrollbar storage remains alive inside the callback barrier");
	return XUI_OK;
}

static int run_case(const char* name)
{
	test_t s;
	int i;
	memset(&s, 0, sizeof(s));
	s.name = name;
	watching = 0;
	xuiTestProxyInit(&s.proxy);
	if (xuiCreate(&s.context) != XUI_OK) return 1;
	CHECK(&s, xuiSetProxy(s.context, &s.proxy.tProxy) == XUI_OK, "set proxy");
	CHECK(&s, s.proxy.tProxy.fontLoadFile(&s.proxy.tProxy, &s.font, "test.ttf", 14, 0) == XUI_OK,
		"create test font");
	CHECK(&s, xuiSetDefaultFont(s.context, s.font) == XUI_OK, "set test font");
	CHECK(&s, xuiSetViewportSize(s.context, 360, 240) == XUI_OK, "set viewport");
	CHECK(&s, xuiTextEditCreate(s.context, &s.edit, NULL) == XUI_OK, "create TextEdit");
	if (s.edit == NULL) goto cleanup;
	if (strcmp(name, "editor_root") == 0) s.root = s.edit;
	else {
		CHECK(&s, xuiWidgetCreate(s.context, &s.root) == XUI_OK, "create root");
		CHECK(&s, xuiWidgetAddChild(s.root, s.edit) == XUI_OK, "attach TextEdit");
	}
	CHECK(&s, xuiSetRootWidget(s.context, s.root) == XUI_OK, "set root");
	CHECK(&s, xuiUpdate(s.context, 0) == XUI_OK, "initial update");
	watch_bars(&s);
	if (strcmp(name, "context_destroy") == 0 || strcmp(name, "editor_root") == 0) {
		xuiDestroy(s.context);
		s.contextDestroyed = 1;
	} else if (strncmp(name, "event_", 6) == 0) {
		xui_event_t event = {0};
		event.iSize = sizeof(event);
		event.iType = XUI_EVENT_COMMAND;
		event.pTarget = s.edit;
		CHECK(&s, xuiWidgetSetEventHandler(s.edit, XUI_EVENT_COMMAND, destroy_event, &s) == XUI_OK, "set callback");
		CHECK(&s, xuiDispatchEvent(s.context, &event) == XUI_OK, "dispatch destructive callback");
		CHECK(&s, s.callbacks == 1, "one destructive callback");
	} else if (strcmp(name, "scroll_then_destroy") == 0) {
		const char* text = "A long line that exceeds the narrow viewport width\n"
			"two\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten\neleven\ntwelve";
		CHECK(&s, xuiWidgetSetFlowMode(s.edit, XUI_FLOW_ABSOLUTE) == XUI_OK, "absolute editor");
		CHECK(&s, xuiWidgetSetRect(s.edit, (xui_rect_t){0, 0, 120, 70}) == XUI_OK, "narrow editor");
		CHECK(&s, xuiTextEditSetWordWrap(s.edit, 0) == XUI_OK, "disable wrapping");
		CHECK(&s, xuiTextEditSetText(s.edit, text) == XUI_OK, "overflowing content");
		CHECK(&s, xuiUpdate(s.context, 0) == XUI_OK, "layout scrollbars");
		CHECK(&s, xuiTextEditSetScroll(s.edit, 30, 45) == XUI_OK, "scroll content");
		CHECK(&s, xuiScrollBarGetValue(bars[0]) > 0 && xuiScrollBarGetValue(bars[1]) > 0,
			"both scrollbar values track the editor");
		xuiWidgetDestroy(s.edit);
	} else xuiWidgetDestroy(s.edit);
	check_released(&s);
	if (!s.contextDestroyed) {
		CHECK(&s, xuiWidgetGetChildCount(s.root) == 0, "destroyed editor is detached");
		CHECK(&s, xuiUpdate(s.context, 0) == XUI_OK, "context remains usable after editor destruction");
	}
	if (strcmp(name, "repeated_destroy") == 0) {
		for (i = 0; i < 32; i++) {
			CHECK(&s, xuiTextEditCreate(s.context, &s.edit, NULL) == XUI_OK, "recreate TextEdit");
			CHECK(&s, xuiWidgetAddChild(s.root, s.edit) == XUI_OK, "reattach TextEdit");
			watch_bars(&s);
			xuiWidgetDestroy(s.edit);
			check_released(&s);
		}
	}
cleanup:
	watching = 0;
	if (!s.contextDestroyed) xuiDestroy(s.context);
	if (s.font != NULL) s.proxy.tProxy.fontDestroy(&s.proxy.tProxy, s.font);
	if (s.failures == 0) printf("PASS %s\n", name);
	return s.failures != 0;
}

int main(int argc, char** argv)
{
	static const char* cases[] = {
		"context_destroy", "widget_destroy", "editor_root", "event_destroy_widget",
		"event_destroy_context", "scroll_then_destroy", "repeated_destroy"
	};
	size_t i;
	int count = 0, failed = 0;
	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		if (argc > 1 && strcmp(argv[1], cases[i]) != 0) continue;
		count++;
		failed += run_case(cases[i]);
	}
	printf("TextEdit lifetime: %d passed, %d failed\n", count - failed, failed);
	return failed != 0 || count == 0;
}
