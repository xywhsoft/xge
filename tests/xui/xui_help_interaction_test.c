#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
	fprintf(stderr, "help interaction:%d: %s\n", __LINE__, #expr); failed = 1; goto cleanup; } } while (0)

static xui_text_shape_proc original_shape;
static xui_context callback_context;
static xui_widget callback_label;
static int shape_action;

static int controlled_shape(xui_proxy proxy, const xui_text_item_t* item, xui_text_shape_t* shape)
{
	int action = shape_action;
	shape_action = 0;
	if (action == 1) return XUI_ERROR_OUT_OF_MEMORY;
	if (action == 2) (void)xuiLabelSetText(callback_label, "replacement text with a different size");
	if (action == 3) (void)xuiSetVirtualDpi(callback_context, 1.5f);
	if (action == 4) { (void)xuiWidgetDestroy(callback_label); callback_label = NULL; }
	return original_shape(proxy, item, shape);
}

static int cache_test(void)
{
	xui_test_proxy_state_t state;
	xui_context context = NULL;
	xui_widget label = NULL;
	xui_font font = NULL, other_font = NULL;
	xui_label_desc_t desc = {0};
	xui_vec2_t sizes[4], size;
	const xui_vec2_t constraints[4] = {{1448, XUI_LAYOUT_UNBOUNDED},
		{1896, XUI_LAYOUT_UNBOUNDED}, {64, 32}, {0, 0}};
	int failed = 0, i, j, before;
	xuiTestProxyInit(&state);
	original_shape = state.tProxy.textShape;
	state.tProxy.textShape = controlled_shape;
	CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &state.tProxy) == XUI_OK);
	CHECK(state.tProxy.fontLoadFile(&state.tProxy, &font, NULL, 16, 0) == XUI_OK);
	CHECK(state.tProxy.fontLoadFile(&state.tProxy, &other_font, NULL, 20, 0) == XUI_OK);
	desc.iSize = sizeof(desc); desc.pFont = font;
	desc.sText = "A long paragraph with UTF-8: \xe4\xb8\xad\xe6\x96\x87, e\xcc\x81 and words that wrap.";
	desc.iWrapMode = XUI_TEXT_WRAP_WORD;
	CHECK(xuiLabelCreate(context, &label, &desc) == XUI_OK);
	callback_context = context; callback_label = label;
	for (i = 0; i < 4; ++i) CHECK(xuiWidgetMeasureContent(label, constraints[i], &sizes[i]) == XUI_OK);
	CHECK(sizes[0].fX > 0 && sizes[0].fY > 0 && sizes[3].fX == 0 && sizes[3].fY == 0);
	xuiTestProxyResetTextCounters(&state);
	for (j = 0; j < 100; ++j) for (i = 0; i < 4; ++i) {
		CHECK(xuiWidgetMeasureContent(label, constraints[i], &size) == XUI_OK);
		CHECK(size.fX == sizes[i].fX && size.fY == sizes[i].fY);
	}
	CHECK(xuiTestProxyGetTextShapeCount(&state) == 0 && xuiTestProxyGetTextMeasureCount(&state) == 0);
	CHECK(xuiLabelSetTextColor(label, XUI_COLOR_RGBA(1, 2, 3, 255)) == XUI_OK);
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK);
	CHECK(xuiTestProxyGetTextShapeCount(&state) == 0);
	/* An explicit content invalidation covers mutable proxy/font metrics too. */
	CHECK(xuiWidgetInvalidate(label, XUI_WIDGET_DIRTY_LAYOUT) == XUI_OK);
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK);
	CHECK(xuiTestProxyGetTextShapeCount(&state) > 0);
	CHECK(xuiLabelSetFont(label, other_font) == XUI_OK);
	xuiTestProxyResetTextCounters(&state);
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && state.iTextShapeCount > 0);
	CHECK(xuiLabelSetLineGap(label, 2) == XUI_OK && xuiLabelSetWrapMode(label, XUI_TEXT_WRAP_CHAR) == XUI_OK);
	xuiTestProxyResetTextCounters(&state);
	CHECK(xuiWidgetMeasureContent(label, constraints[2], &size) == XUI_OK && state.iTextShapeCount > 0);
	CHECK(xuiSetVirtualDpi(context, 2) == XUI_OK);
	xuiTestProxyResetTextCounters(&state);
	CHECK(xuiWidgetMeasureContent(label, constraints[2], &size) == XUI_OK && state.iTextShapeCount > 0);
	/* A failed measurement must not turn into a cached zero-size success. */
	CHECK(xuiLabelSetText(label, "new contents after a failure") == XUI_OK);
	shape_action = 1;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && size.fX == 0 && size.fY == 0);
	xuiTestProxyResetTextCounters(&state);
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && size.fX > 0 && state.iTextShapeCount > 0);
	/* Reentrant text/DPI mutations cannot publish an entry for old inputs. */
	CHECK(xuiLabelSetText(label, "before reentrant edit") == XUI_OK);
	shape_action = 2;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK);
	before = state.iTextShapeCount;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && state.iTextShapeCount > before);
	CHECK(strstr(xuiLabelGetText(label), "replacement") != NULL);
	CHECK(xuiLabelSetText(label, "before reentrant DPI") == XUI_OK);
	shape_action = 3;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK);
	before = state.iTextShapeCount;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && size.fX > 0 && state.iTextShapeCount > before);
	CHECK(xuiLabelSetText(label, "retire while measuring") == XUI_OK);
	shape_action = 4;
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && callback_label == NULL);
	label = NULL;
	CHECK(xuiLabelCreate(context, &label, &desc) == XUI_OK);
	CHECK(xuiWidgetMeasureContent(label, constraints[0], &size) == XUI_OK && size.fX > 0);
	printf("help cache: 400 warm alternating-constraint queries, zero text shaping; font/DPI/style, failure and reentry passed\n");
cleanup:
	shape_action = 0; callback_label = NULL; callback_context = NULL;
	if (context) xuiDestroy(context);
	if (font) state.tProxy.fontDestroy(&state.tProxy, font);
	if (other_font) state.tProxy.fontDestroy(&state.tProxy, other_font);
	return failed;
}

static int interaction_test(void)
{
	xui_test_proxy_state_t state;
	xui_context context = NULL;
	xui_widget root = NULL, view = NULL, content = NULL, code = NULL, label = NULL;
	xui_font font = NULL;
	xui_scroll_view_desc_t scroll = {0};
	xui_code_edit_desc_t editor = {0};
	xui_label_desc_t paragraph = {0};
	xui_layout_stats_t before = {0}, after = {0};
	xui_rect_t initial_code, moved_code, initial_ime, next_ime;
	int failed = 0, i;
	xuiTestProxyInit(&state);
	CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &state.tProxy) == XUI_OK);
	CHECK(state.tProxy.fontLoadFile(&state.tProxy, &font, NULL, 16, 0) == XUI_OK);
	CHECK(xuiSetDefaultFont(context, font) == XUI_OK && xuiSetViewportSize(context, 640, 480) == XUI_OK);
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK && xuiSetRootWidget(context, root) == XUI_OK);
	scroll.iSize = sizeof(scroll); scroll.fContentWidth = 400; scroll.fContentHeight = 5000;
	scroll.iPolicyX = XUI_SCROLLBAR_POLICY_HIDDEN; scroll.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	CHECK(xuiScrollViewCreate(context, &view, &scroll) == XUI_OK && xuiWidgetAddChild(root, view) == XUI_OK);
	CHECK(xuiWidgetSetRect(view, (xui_rect_t){10, 10, 420, 240}) == XUI_OK);
	content = xuiScrollViewGetContentWidget(view);
	CHECK(xuiWidgetSetLayoutType(content, XUI_LAYOUT_COLUMN) == XUI_OK);
	editor.iSize = sizeof(editor); editor.pFont = font; editor.sText = "alpha beta gamma\nsecond line";
	CHECK(xuiCodeEditCreate(context, &code, &editor) == XUI_OK && xuiWidgetAddChild(content, code) == XUI_OK);
	CHECK(xuiWidgetSetSizeMode(code, XUI_SIZE_FILL, XUI_SIZE_FIXED) == XUI_OK &&
		xuiWidgetSetPreferredSize(code, (xui_vec2_t){0, 80}) == XUI_OK);
	paragraph.iSize = sizeof(paragraph); paragraph.pFont = font;
	paragraph.sText = "A stable paragraph with enough words to measure and wrap in the help viewport.";
	paragraph.iWrapMode = XUI_TEXT_WRAP_WORD;
	for (i = 0; i < 128; ++i) {
		CHECK(xuiLabelCreate(context, &label, &paragraph) == XUI_OK && xuiWidgetAddChild(content, label) == XUI_OK);
		CHECK(xuiWidgetSetSizeMode(label, XUI_SIZE_FILL, XUI_SIZE_FIXED) == XUI_OK &&
			xuiWidgetSetPreferredSize(label, (xui_vec2_t){0, 32}) == XUI_OK);
	}
	CHECK(xuiLayout(context) == XUI_OK && xuiUpdate(context, 0.016f) == XUI_OK && xuiLayout(context) == XUI_OK);
	initial_code = xuiWidgetGetWorldRect(code);
	/* Moving the scroll span changes world geometry, not its text metrics. */
	CHECK(xuiScrollViewSetOffset(view, 0, 10) == XUI_OK && xuiLayout(context) == XUI_OK);
	moved_code = xuiWidgetGetWorldRect(code);
	CHECK(moved_code.fY == initial_code.fY - 10 && moved_code.fW == initial_code.fW);
	xuiTestProxyResetTextCounters(&state);
	for (i = 0; i < 100; ++i) CHECK(xuiScrollViewSetOffset(view, 0, 10 + i) == XUI_OK && xuiLayout(context) == XUI_OK);
	CHECK(state.iTextShapeCount == 0 && state.iTextMeasureCount == 0);
	CHECK(xuiScrollViewSetOffset(view, 0, 0) == XUI_OK && xuiLayout(context) == XUI_OK);
	CHECK(xuiSetFocusWidget(context, code) == XUI_OK);
	CHECK(xuiCodeSelectionSetRange(xuiCodeEditGetSelection(code), xuiCodeEditGetDocument(code), 0, 0) == XUI_OK);
	CHECK(xuiCodeEditEnsureCaretVisible(code) == XUI_OK && xuiLayout(context) == XUI_OK);
	initial_ime = xuiGetImeCandidateRect(context);
	CHECK(xuiGetLayoutStats(context, &before) == XUI_OK);
	for (i = 0; i < 100; ++i) {
		CHECK(xuiCodeSelectionSetRange(xuiCodeEditGetSelection(code), xuiCodeEditGetDocument(code), 0, 1 + i % 12) == XUI_OK);
		CHECK(xuiCodeEditEnsureCaretVisible(code) == XUI_OK && xuiRenderPrepare(context) == XUI_OK);
	}
	CHECK(xuiGetLayoutStats(context, &after) == XUI_OK && after.iGeneration == before.iGeneration);
	next_ime = xuiGetImeCandidateRect(context);
	CHECK(next_ime.fX > initial_ime.fX);
	CHECK(strcmp(xuiCodeEditGetText(code), editor.sText) == 0);
	/* A real size change still propagates to the paragraph widths. */
	CHECK(xuiWidgetSetRect(view, (xui_rect_t){10, 10, 240, 240}) == XUI_OK && xuiLayout(context) == XUI_OK);
	CHECK(xuiWidgetGetWorldRect(code).fW < initial_code.fW);
	printf("help interaction: 128 paragraphs, 100 scrolls with zero shaping, 100 caret moves without layout, IME and width reflow passed\n");
cleanup:
	if (context) xuiDestroy(context);
	if (font) state.tProxy.fontDestroy(&state.tProxy, font);
	return failed;
}

int main(void)
{
	if (cache_test() || interaction_test()) return 1;
	puts("xui_help_interaction_test passed");
	return 0;
}
