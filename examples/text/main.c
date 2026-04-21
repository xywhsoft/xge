#include "../../xge.h"
#include <stdio.h>

static xge_font_t g_tFont;
static int g_bFontReady;

static int MainFrame(void* pUser)
{
	(void)pUser;
	if ( xgeKeyDown(256) ) {
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 24, 32, 255));
	if ( g_bFontReady ) {
		xgeTextDraw(&g_tFont, "XGE Text\nASCII + 中文", 40.0f, 48.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeTextDraw(&g_tFont, "0123456789", 40.0f, 140.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
		xgeTextDrawRect(&g_tFont, "Center\n中文居中\nThis line is clipped by rect", (xge_rect_t){ 320.0f, 48.0f, 280.0f, 92.0f }, XGE_COLOR_RGBA(120, 220, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		xgeTextDrawRect(&g_tFont, "Right\n右对齐", (xge_rect_t){ 320.0f, 170.0f, 280.0f, 100.0f }, XGE_COLOR_RGBA(255, 150, 150, 255), XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP);
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	const char* sFont;

	sFont = "C:/Windows/Fonts/msyh.ttc";
	if ( argc >= 2 ) {
		sFont = argv[1];
	}

	tDesc.iWidth = 800;
	tDesc.iHeight = 480;
	tDesc.sTitle = "XGE Text";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	tDesc.pNativeWindow = 0;
	tDesc.pUser = 0;

	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeFontLoad(&g_tFont, sFont, 32.0f) == XGE_OK ) {
		g_bFontReady = 1;
	} else {
		printf("font load failed: %s\n", sFont);
	}
	xgeRun(MainFrame, 0);
	xgeFontFree(&g_tFont);
	xgeUnit();
	return 0;
}
