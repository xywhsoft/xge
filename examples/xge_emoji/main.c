#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_W 980
#define WINDOW_H 640

typedef struct emoji_demo_t {
	xge_font_t tFont16;
	xge_font_t tFont24;
	xge_font_t tFont40;
	xge_font_t tFont72;
	xge_render_target_t tCaptureTarget;
	char sCapturePath[512];
	int bCaptureDone;
} emoji_demo_t;

static const char* find_font(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
	};
	FILE* file;
	int i;

	for ( i = 0; i < (int)(sizeof(paths) / sizeof(paths[0])); i++ ) {
		file = fopen(paths[i], "rb");
		if ( file != NULL ) {
			fclose(file);
			return paths[i];
		}
	}
	return NULL;
}

static void draw_mixed_line(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor)
{
	xge_text_shape_desc_t tDesc;
	xge_glyph_run_t tRun;
	xge_vec2_t tSize;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tRun, 0, sizeof(tRun));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = sText;
	tDesc.iTextSize = -1;
	tDesc.iFlags = XGE_TEXT_SHAPE_DEFAULT;
	tDesc.iEmojiLinePolicy = XGE_EMOJI_LINE_STABLE;
	if ( xgeTextShape(&tDesc, &tRun) != XGE_OK ) return;
	tSize = xgeGlyphRunMeasure(&tRun);
	xgeShapeRectStroke((xge_rect_t){fX, fY, tSize.fX, tSize.fY}, 1.0f, XGE_COLOR_RGBA(76, 104, 136, 180));
	xgeShapeLine(fX, fY + tRun.fAscent, fX + tSize.fX, fY + tRun.fAscent, 1.0f, XGE_COLOR_RGBA(245, 158, 11, 150));
	xgeGlyphRunDraw(&tRun, fX, fY, iColor, XGE_DRAW_SCREEN_SPACE);
	xgeGlyphRunFree(&tRun);
}

static void draw_scene(emoji_demo_t* pDemo)
{
	static const char line1[] =
		"16 px  Hello " "\xF0\x9F\x98\x80" "  Rocket " "\xF0\x9F\x9A\x80";
	static const char line2[] =
		"24 px  Success " "\xE2\x9C\x85" "  Fire " "\xF0\x9F\x94\xA5" "  Party " "\xF0\x9F\x8E\x89";
	static const char line3[] =
		"40 px  " "\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD" "  "
		"\xE2\x9D\xA4\xEF\xB8\x8F" "  "
		"\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3";
	static const char line4[] =
		"\xF0\x9F\x98\x82" "  "
		"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D"
		"\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6";
	xgeTextDraw(&pDemo->tFont24, "XGE SVG Emoji - stable line height and baseline", 42.0f, 32.0f, XGE_COLOR_RGBA(225, 232, 240, 255));
	draw_mixed_line(&pDemo->tFont16, line1, 42.0f, 92.0f, XGE_COLOR_RGBA(219, 228, 240, 255));
	draw_mixed_line(&pDemo->tFont24, line2, 42.0f, 152.0f, XGE_COLOR_RGBA(219, 228, 240, 255));
	draw_mixed_line(&pDemo->tFont40, line3, 42.0f, 232.0f, XGE_COLOR_RGBA(219, 228, 240, 255));
	draw_mixed_line(&pDemo->tFont72, line4, 42.0f, 340.0f, XGE_COLOR_RGBA(219, 228, 240, 255));
	xgeTextDraw(&pDemo->tFont16, "Orange: baseline   Blue: measured layout box", 42.0f, 540.0f, XGE_COLOR_RGBA(148, 163, 184, 255));
}

static int capture_frame(emoji_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	iStride = WINDOW_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * WINDOW_H);
	if ( pPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	iRet = xgeRenderTargetReadPixels(&pDemo->tCaptureTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(pDemo->sCapturePath, WINDOW_W, WINDOW_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		pDemo->bCaptureDone = 1;
		printf("capture saved: %s\n", pDemo->sCapturePath);
	}
	return iRet;
}

static int frame(void* pUser)
{
	emoji_demo_t* pDemo;
	xge_pass_t tPass;
	int iRet;

	pDemo = (emoji_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( pDemo->sCapturePath[0] != 0 ) {
		xgePassInit(&tPass, &pDemo->tCaptureTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(18, 24, 32, 255));
		iRet = xgePassBegin(&tPass);
		if ( iRet != XGE_OK ) return iRet;
		draw_scene(pDemo);
		iRet = xgePassEnd(&tPass);
		if ( iRet != XGE_OK ) return iRet;
		iRet = capture_frame(pDemo);
		if ( iRet != XGE_OK ) return iRet;
	} else {
		xgeClear(XGE_COLOR_RGBA(18, 24, 32, 255));
		draw_scene(pDemo);
	}
	xgeEnd();
	if ( pDemo->bCaptureDone ) xgeQuit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	emoji_demo_t tDemo;
	xge_desc_t tDesc;
	const char* sFont;
	int iRet;
	int i;

	sFont = find_font();
	if ( sFont == NULL ) return 1;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = WINDOW_W;
	tDesc.iHeight = WINDOW_H;
	tDesc.sTitle = "XGE SVG Emoji";
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iFlags = XGE_INIT_RESIZABLE | XGE_INIT_HIGHDPI;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--capture") == 0) && (i + 1 < argc) ) {
			snprintf(tDemo.sCapturePath, sizeof(tDemo.sCapturePath), "%s", argv[++i]);
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(tDemo.sCapturePath, sizeof(tDemo.sCapturePath), "%s", argv[i] + 10);
		}
	}
	if ( tDemo.sCapturePath[0] != 0 ) tDesc.iFlags = XGE_INIT_OFFSCREEN;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) return 1;
	if ( (tDemo.sCapturePath[0] != 0) &&
	     (xgeRenderTargetCreate(&tDemo.tCaptureTarget, WINDOW_W, WINDOW_H) != XGE_OK) ) {
		xgeUnit();
		return 1;
	}
	if ( xgeFontLoad(&tDemo.tFont16, sFont, 16.0f) != XGE_OK ||
	     xgeFontLoad(&tDemo.tFont24, sFont, 24.0f) != XGE_OK ||
	     xgeFontLoad(&tDemo.tFont40, sFont, 40.0f) != XGE_OK ||
	     xgeFontLoad(&tDemo.tFont72, sFont, 72.0f) != XGE_OK ) {
		xgeRenderTargetFree(&tDemo.tCaptureTarget);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(frame, &tDemo);
	xgeFontFree(&tDemo.tFont72);
	xgeFontFree(&tDemo.tFont40);
	xgeFontFree(&tDemo.tFont24);
	xgeFontFree(&tDemo.tFont16);
	xgeRenderTargetFree(&tDemo.tCaptureTarget);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
