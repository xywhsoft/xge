#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
	if ( !(expr) ) { \
		fprintf(stderr, "xui_code_bracket_geometry_test:%d: %s\n", __LINE__, #expr); \
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
	xui_widget editor = NULL;
	xui_surface target = NULL;
	xui_surface cache;
	xui_font font = NULL;
	xui_code_edit_desc_t desc;
	xui_style_property_t properties[3];
	xui_rect_t bracket;
	xui_rect_t offset;
	xui_rect_t world;
	xui_rect_t selection;
	xui_rect_t selectionOffset;
	const uint32_t background = XUI_COLOR_RGBA(29, 126, 214, 71);
	const uint32_t border = XUI_COLOR_RGBA(18, 94, 181, 233);
	const uint32_t selectionColor = XUI_COLOR_RGBA(67, 138, 231, 199);
	int failed = 0;
	int status;

	xuiTestProxyInit(&state);
	CHECK(xuiCreate(&context) == XUI_OK);
	CHECK(xuiSetProxy(context, &state.tProxy) == XUI_OK);
	CHECK(state.tProxy.fontLoadFile(&state.tProxy, &font,
		"codeedit.ttf", 13.0f, 0) == XUI_OK && font != NULL);
	CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
	CHECK(xuiInputViewport(context, 320.0f, 160.0f) == XUI_OK);
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK && root != NULL);
	CHECK(xuiWidgetSetRect(root,
		(xui_rect_t){0.0f, 0.0f, 320.0f, 160.0f}) == XUI_OK);
	CHECK(xuiSetRootWidget(context, root) == XUI_OK);

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = font;
	desc.sText = "[ab]";
	CHECK(xuiCodeEditCreate(context, &editor, &desc) == XUI_OK &&
		editor != NULL);
	CHECK(xuiWidgetSetRect(editor,
		(xui_rect_t){20.0f, 20.0f, 240.0f, 100.0f}) == XUI_OK);
	CHECK(xuiWidgetAddChild(root, editor) == XUI_OK);
	properties[0] = color_property(
		"codeedit.bracket_match.background_color", background);
	properties[1] = color_property(
		"codeedit.bracket_match.border_color", border);
	properties[2] = color_property("codeedit.selection.color", selectionColor);
	CHECK(xuiWidgetSetInlineStyle(editor, properties, 3) == XUI_OK);
	CHECK(xuiCodeSelectionSetRange(xuiCodeEditGetSelection(editor),
		xuiCodeEditGetDocument(editor), 1, 1) == XUI_OK);
	CHECK(xuiTestSurfaceCreate(&state, &target, 320, 160,
		XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	CHECK(xuiLayout(context) == XUI_OK);
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK);
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);

	cache = xuiWidgetGetCacheSurface(editor, xuiWidgetGetStateId(editor));
	CHECK(cache != NULL);
	CHECK(xuiTestSurfaceGetRectFillColorCount(cache, background) == 2);
	CHECK(xuiTestSurfaceGetRectFillColorCount(cache, border) == 2);
	bracket = xuiTestSurfaceGetLastRectFillColorRect(cache, border);
	status = xuiCodeEditGetOffsetRect(editor, 3, &offset);
	world = xuiWidgetGetWorldRect(editor);
	CHECK(status == XUI_OK);
	CHECK(bracket.fX == offset.fX - world.fX);

	CHECK(xuiCodeSelectionSetRange(xuiCodeEditGetSelection(editor),
		xuiCodeEditGetDocument(editor), 1, 2) == XUI_OK);
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK);
	CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);
	cache = xuiWidgetGetCacheSurface(editor, xuiWidgetGetStateId(editor));
	CHECK(cache != NULL);
	CHECK(xuiTestSurfaceGetRectFillColorCount(cache, selectionColor) == 1);
	selection = xuiTestSurfaceGetLastRectFillColorRect(cache, selectionColor);
	CHECK(xuiCodeEditGetOffsetRect(editor, 1, &selectionOffset) == XUI_OK);
	CHECK(selection.fX == selectionOffset.fX - world.fX);

cleanup:
	if ( target != NULL ) state.tProxy.surfaceDestroy(&state.tProxy, target);
	if ( context != NULL ) xuiDestroy(context);
	if ( font != NULL ) state.tProxy.fontDestroy(&state.tProxy, font);
	if ( failed ) return 1;
	puts("xui_code_bracket_geometry_test passed");
	return 0;
}
