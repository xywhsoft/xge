#include "xui.h"
#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 128
#define HEIGHT 48

static const char* font_path(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc"
	};
	int i;
	for ( i = 0; i < (int)(sizeof(paths) / sizeof(paths[0])); i++ ) {
		FILE* file = fopen(paths[i], "rb");
		if ( file != NULL ) {
			fclose(file);
			return paths[i];
		}
	}
	return NULL;
}

static int render(xui_proxy proxy, xui_font font, const xui_text_item_t* item,
	int width, uint32_t align, unsigned char* pixels)
{
	xui_surface_desc_t desc = {0};
	xui_surface surface = NULL;
	xui_draw_context draw = NULL;
	int result;
	desc.iKind = XUI_SURFACE_KIND_TEXTURE;
	desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	desc.iWidth = WIDTH;
	desc.iHeight = HEIGHT;
	desc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	result = proxy->surfaceCreate(proxy, &surface, &desc);
	if ( result == XGE_OK ) result = proxy->surfaceClear(proxy, surface, 0);
	if ( result == XGE_OK ) result = proxy->drawBegin(proxy, &draw, surface);
	if ( result == XGE_OK ) result = proxy->drawText(proxy, draw, item,
		(xui_rect_t){8, 2, width, 32}, XUI_COLOR_WHITE,
		XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP | align);
	if ( draw != NULL ) {
		int end_result = proxy->drawEnd(proxy, draw);
		if ( result == XGE_OK ) result = end_result;
	}
	if ( result == XGE_OK ) result = proxy->surfaceReadRGBA(proxy, surface,
		pixels, WIDTH * 4);
	if ( surface != NULL ) proxy->surfaceDestroy(proxy, surface);
	(void)font;
	return result;
}

static int compare_alignment(xui_proxy proxy, xui_font font,
	const xui_text_item_t* item, int width, const char* label)
{
	unsigned char left[WIDTH * HEIGHT * 4];
	unsigned char right[WIDTH * HEIGHT * 4];
	int result = render(proxy, font, item, width, XUI_TEXT_ALIGN_LEFT, left);
	if ( result == XGE_OK ) result = render(proxy, font, item, width,
		XUI_TEXT_ALIGN_RIGHT, right);
	if ( result != XGE_OK || memcmp(left, right, sizeof(left)) != 0 ) {
		printf("FAIL %s tight clipped left/right pixels differ: %d\n", label, result);
		return 1;
	}
	return 0;
}

static int run_test(void* user)
{
	static const char* candidates[] = {
		"A", "B", "M", "W", "AV", "xui", "Hello", "right edge"
	};
	xui_proxy_t proxy;
	xui_font font = NULL;
	xui_text_shape_t shape = {0};
	xui_text_item_t item = {0};
	const char* path = (const char*)user;
	const char* text = NULL;
	int width = 0;
	int result;
	int i;

	proxy = xuiProxyXge();
	result = proxy.fontLoadFile(&proxy, &font, path, 12.0f, XUI_FONT_FORMAT_TTF);
	for ( i = 0; result == XGE_OK &&
	     i < (int)(sizeof(candidates) / sizeof(candidates[0])); i++ ) {
		item = (xui_text_item_t){sizeof(item), font, candidates[i], -1,
			XUI_TEXT_SHAPE_DEFAULT};
		result = proxy.textShape(&proxy, &item, &shape);
		if ( result != XGE_OK ) break;
		width = (int)ceilf(shape.fWidth);
		if ( shape.fWidth > 0.0f && (float)width - shape.fWidth >= 0.5f )
			text = candidates[i];
		xuiTextShapeFree(&shape);
		if ( text != NULL ) break;
	}
	if ( result != XGE_OK || text == NULL || width <= 0 || width > WIDTH - 16 ) {
		printf("FAIL no fractional alignment sample: result=%d width=%d\n", result, width);
		result = XGE_ERROR_RESOURCE_FAILED;
		goto done;
	}
	item = (xui_text_item_t){sizeof(item), font, text, -1,
		XUI_TEXT_SHAPE_DEFAULT};
	if ( compare_alignment(&proxy, font, &item, width, "direct") != 0 ) {
		result = XGE_ERROR_RESOURCE_FAILED;
		goto done;
	}
	item.iFlags |= XUI_TEXT_SHAPE_RETAIN_PAINT;
	result = proxy.textShape(&proxy, &item, &shape);
	if ( result != XGE_OK ) goto done;
	item.iFlags = XUI_TEXT_SHAPE_DEFAULT | XUI_TEXT_SHAPE_RANGE;
	item.iRangeStart = 0;
	item.iRangeEnd = (int)strlen(text);
	item.pShape = &shape;
	if ( compare_alignment(&proxy, font, &item, width, "retained range") != 0 )
		result = XGE_ERROR_RESOURCE_FAILED;

done:
	xuiTextShapeFree(&shape);
	if ( font != NULL ) proxy.fontDestroy(&proxy, font);
	xgeQuit();
	if ( result == XGE_OK )
		printf("PASS pixel-aligned clipped text: %s at 12 px\n", text);
	return result;
}

int main(void)
{
	xge_desc_t engine = {0};
	const char* path = font_path();
	int result;
	if ( path == NULL ) {
		puts("SKIP no system font");
		return 0;
	}
	engine.iWidth = WIDTH;
	engine.iHeight = HEIGHT;
	engine.sTitle = "XUI text pixel alignment";
	engine.iFlags = XGE_INIT_OFFSCREEN;
	engine.iRunMode = XGE_RUN_GAME_LOOP;
	result = xgeInit(&engine);
	if ( result == XGE_OK ) result = xgeRun(run_test, (void*)path);
	xgeUnit();
	if ( result != XGE_OK ) {
		printf("FAIL text pixel alignment run: %d\n", result);
		return 1;
	}
	return 0;
}
