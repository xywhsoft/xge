#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
	if ( !(expr) ) { \
		fprintf(stderr, "xui_text_pixel_geometry_test:%d: %s\n", __LINE__, #expr); \
		failed = 1; \
		goto cleanup; \
	} \
} while ( 0 )

static xui_style_property_t color_property(const char* name, uint32_t color)
{
	xui_style_property_t property;
	memset(&property, 0, sizeof(property));
	property.iSize = sizeof(property);
	property.sName = name;
	property.iDirtyFlags = XUI_WIDGET_DIRTY_STYLE |
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	property.tValue.iSize = sizeof(property.tValue);
	property.tValue.iType = XUI_STYLE_VALUE_COLOR;
	property.tValue.iColor = color;
	return property;
}

int main(void)
{
	xui_test_proxy_state_t state;
	xui_context context = NULL;
	xui_widget root = NULL;
	xui_widget input = NULL;
	xui_widget textEdit = NULL;
	xui_surface target = NULL;
	xui_surface cache;
	xui_font font = NULL;
	xui_input_desc_t inputDesc;
	xui_text_edit_desc_t textEditDesc;
	xui_style_property_t inputStyle;
	xui_style_property_t textEditStyle;
	xui_rect_t inputCursor;
	xui_rect_t inputSelection;
	xui_rect_t textEditCursor;
	xui_rect_t textEditSelection;
	const uint32_t inputSelectionColor = XUI_COLOR_RGBA(61, 127, 211, 197);
	const uint32_t textEditSelectionColor = XUI_COLOR_RGBA(73, 139, 223, 197);
	int failed = 0;

	xuiTestProxyInit(&state);
	CHECK(xuiCreate(&context) == XUI_OK);
	CHECK(xuiSetProxy(context, &state.tProxy) == XUI_OK);
	CHECK(state.tProxy.fontLoadFile(&state.tProxy, &font,
		"text-pixel.ttf", 13.0f, 0) == XUI_OK && font != NULL);
	CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
	CHECK(xuiInputViewport(context, 400.0f, 200.0f) == XUI_OK);
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK && root != NULL);
	CHECK(xuiWidgetSetRect(root,
		(xui_rect_t){0.0f, 0.0f, 400.0f, 200.0f}) == XUI_OK);
	CHECK(xuiSetRootWidget(context, root) == XUI_OK);

	memset(&inputDesc, 0, sizeof(inputDesc));
	inputDesc.iSize = sizeof(inputDesc);
	inputDesc.pFont = font;
	inputDesc.sText = "abc";
	CHECK(xuiInputCreate(context, &input, &inputDesc) == XUI_OK && input != NULL);
	CHECK(xuiWidgetSetRect(input,
		(xui_rect_t){20.0f, 20.0f, 220.0f, 30.0f}) == XUI_OK);
	CHECK(xuiWidgetAddChild(root, input) == XUI_OK);
	inputStyle = color_property("input.selection.color", inputSelectionColor);
	CHECK(xuiWidgetSetInlineStyle(input, &inputStyle, 1) == XUI_OK);

	memset(&textEditDesc, 0, sizeof(textEditDesc));
	textEditDesc.iSize = sizeof(textEditDesc);
	textEditDesc.pFont = font;
	textEditDesc.sText = "abc";
	CHECK(xuiTextEditCreate(context, &textEdit, &textEditDesc) == XUI_OK &&
		textEdit != NULL);
	CHECK(xuiWidgetSetRect(textEdit,
		(xui_rect_t){20.0f, 70.0f, 260.0f, 90.0f}) == XUI_OK);
	CHECK(xuiWidgetAddChild(root, textEdit) == XUI_OK);
	textEditStyle = color_property("textedit.selection.color", textEditSelectionColor);
	CHECK(xuiWidgetSetInlineStyle(textEdit, &textEditStyle, 1) == XUI_OK);

	CHECK(xuiLayout(context) == XUI_OK);
	CHECK(xuiInputSetSelection(input, 1, 1) == XUI_OK);
	CHECK(xuiTextEditSetSelection(textEdit, 1, 1) == XUI_OK);
	CHECK(xuiTestSurfaceCreate(&state, &target, 400, 200,
		XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK);
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);
	inputCursor = xuiInputGetCursorRect(input);
	textEditCursor = xuiTextEditGetCursorRect(textEdit);
	CHECK(xuiInputSetSelection(input, 1, 2) == XUI_OK);
	CHECK(xuiTextEditSetSelection(textEdit, 1, 2) == XUI_OK);
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK);
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);

	cache = xuiWidgetGetCacheSurface(input, xuiWidgetGetStateId(input));
	CHECK(cache != NULL);
	inputSelection = xuiTestSurfaceGetLastRectFillColorRect(cache,
		inputSelectionColor);
	CHECK(inputSelection.fX == inputCursor.fX);

	cache = xuiWidgetGetCacheSurface(textEdit, xuiWidgetGetStateId(textEdit));
	CHECK(cache != NULL);
	textEditSelection = xuiTestSurfaceGetLastRectFillColorRect(cache,
		textEditSelectionColor);
	CHECK(textEditSelection.fX == textEditCursor.fX);

cleanup:
	if ( target != NULL ) state.tProxy.surfaceDestroy(&state.tProxy, target);
	if ( context != NULL ) xuiDestroy(context);
	if ( font != NULL ) state.tProxy.fontDestroy(&state.tProxy, font);
	if ( failed ) return 1;
	puts("xui_text_pixel_geometry_test passed");
	return 0;
}
