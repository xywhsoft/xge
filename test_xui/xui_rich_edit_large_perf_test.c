#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define PERF_CHECK(e, m) do { if (!(e)) { printf("xui_rich_edit_large_perf_test failed: %s\n", m); failed = 1; goto cleanup; } } while (0)

static double now_ms(void)
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER now;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&now);
	return (double)now.QuadPart * 1000.0 / (double)frequency.QuadPart;
}

static char* make_text(int paragraphs, int columns, int* length)
{
	static const char pattern[] = "Structured rich text benchmark paragraph with styled content and UTF-8-safe editing. ";
	char* text;
	int capacity;
	int at = 0;
	int p;
	int c;
	if ( paragraphs < 1 ) paragraphs = 1000;
	if ( columns < 32 ) columns = 96;
	capacity = paragraphs * (columns + 1) + 1;
	text = (char*)malloc((size_t)capacity);
	if ( text == NULL ) return NULL;
	for ( p = 0; p < paragraphs; p++ ) {
		for ( c = 0; c < columns; c++ ) text[at++] = pattern[c % ((int)sizeof(pattern) - 1)];
		if ( p + 1 < paragraphs ) text[at++] = '\n';
	}
	text[at] = 0;
	if ( length != NULL ) *length = at;
	return text;
}

int main(int argc, char** argv)
{
	xui_test_proxy_state_t proxy;
	xui_context context = NULL;
	xui_surface target = NULL;
	xui_font font = NULL;
	xui_widget root = NULL;
	xui_widget edit = NULL;
	xui_rich_document document = NULL;
	xui_rich_edit_desc_t desc;
	xui_rich_text_style_t style;
	char* text = NULL;
	double begin;
	double buildMs;
	double createMs;
	double frameMs;
	double insertMs;
	double replaceMs;
	double structuralReplaceMs;
	double undoMs;
	double redoMs;
	int paragraphs = argc > 1 ? atoi(argv[1]) : 10000;
	int columns = argc > 2 ? atoi(argv[2]) : 96;
	int textLength;
	int middle;
	int failed = 0;
	int ret;

	xuiTestProxyInit(&proxy);
	text = make_text(paragraphs, columns, &textLength);
	PERF_CHECK(text != NULL, "build input");
	memset(&style, 0, sizeof(style)); style.iSize = sizeof(style);
	begin = now_ms();
	PERF_CHECK(xuiRichDocumentCreate(&document) == XUI_OK, "document create");
	PERF_CHECK(xuiRichDocumentReplace(document, 0, 0, text, &style) == XUI_OK, "initial document");
	buildMs = now_ms() - begin;
	PERF_CHECK(xuiCreate(&context) == XUI_OK, "context");
	PERF_CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK, "proxy");
	PERF_CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "rich_perf.ttf", 14.0f, 0) == XUI_OK, "font");
	PERF_CHECK(xuiSetDefaultFont(context, font) == XUI_OK, "default font");
	PERF_CHECK(xuiInputViewport(context, 960, 640) == XUI_OK, "viewport");
	PERF_CHECK(xuiWidgetCreate(context, &root) == XUI_OK, "root");
	PERF_CHECK(xuiWidgetSetRect(root, (xui_rect_t){0,0,960,640}) == XUI_OK, "root rect");
	PERF_CHECK(xuiSetRootWidget(context, root) == XUI_OK, "root set");
	memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc); desc.pDocument = document;
	desc.bOwnDocument = 1; desc.pFont = font; desc.bWordWrap = 1;
	begin = now_ms();
	ret = xuiRichEditCreate(context, &edit, &desc); document = NULL;
	PERF_CHECK(ret == XUI_OK, "editor");
	PERF_CHECK(xuiWidgetSetRect(edit, (xui_rect_t){12,12,936,616}) == XUI_OK, "editor rect");
	PERF_CHECK(xuiWidgetAddChild(root, edit) == XUI_OK, "editor attach");
	createMs = now_ms() - begin;
	PERF_CHECK(xuiTestSurfaceCreate(&proxy, &target, 960, 640, XUI_SURFACE_USAGE_TARGET) == XUI_OK, "target");
	begin = now_ms();
	PERF_CHECK(xuiLayout(context) == XUI_OK, "layout");
	PERF_CHECK(xuiUpdate(context, 0.016f) == XUI_OK, "update");
	PERF_CHECK(xuiRender(context, target, NULL, 0) == XUI_OK, "render");
	frameMs = now_ms() - begin;
	middle = (paragraphs / 2) * (columns + 1) + columns / 2;
	PERF_CHECK(xuiRichEditSetSelection(edit, middle, middle) == XUI_OK, "middle caret");
	begin = now_ms();
	PERF_CHECK(xuiRichEditInsertText(edit, "x") == XUI_OK, "single insert");
	insertMs = now_ms() - begin;
	PERF_CHECK(xuiRichEditSetSelection(edit, middle - 8, middle + 8) == XUI_OK, "replace selection");
	begin = now_ms();
	PERF_CHECK(xuiRichEditInsertText(edit, "replacement") == XUI_OK, "middle replace");
	replaceMs = now_ms() - begin;
	begin = now_ms(); PERF_CHECK(xuiRichEditUndo(edit) == XUI_OK, "undo"); undoMs = now_ms() - begin;
	begin = now_ms(); PERF_CHECK(xuiRichEditRedo(edit) == XUI_OK, "redo"); redoMs = now_ms() - begin;
	PERF_CHECK(xuiRichEditSetSelection(edit, middle + columns / 2 - 4, middle + columns / 2 + 4) == XUI_OK,
		"cross paragraph selection");
	begin = now_ms();
	PERF_CHECK(xuiRichEditInsertText(edit, "structure") == XUI_OK, "cross paragraph replace");
	structuralReplaceMs = now_ms() - begin;
	printf("RichEdit baseline: paragraphs=%d bytes=%d build=%.2fms create=%.2fms first_frame=%.2fms local_insert=%.2fms local_replace=%.2fms undo=%.2fms redo=%.2fms structural_replace=%.2fms\n",
		paragraphs, textLength, buildMs, createMs, frameMs, insertMs, replaceMs, undoMs, redoMs, structuralReplaceMs);

cleanup:
	free(text);
	if ( context != NULL ) xuiDestroy(context);
	if ( target != NULL ) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
	if ( document != NULL ) xuiRichDocumentDestroy(document);
	return failed ? 1 : 0;
}
