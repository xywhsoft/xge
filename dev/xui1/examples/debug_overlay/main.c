#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct debug_overlay_demo_t {
	xge_font_t tFont;
	int bFontReady;
	float fTime;
} debug_overlay_demo_t;

static int __debugOverlayLoadFont(debug_overlay_demo_t* pDemo)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/consola.ttf",
		"C:/Windows/Fonts/arial.ttf",
		"C:/Windows/Fonts/msyh.ttc"
	};
	int i;

	if ( pDemo->bFontReady ) {
		return XGE_OK;
	}
	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		if ( xgeFontLoad(&pDemo->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pDemo->bFontReady = 1;
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void __debugOverlayDrawText(debug_overlay_demo_t* pDemo, const char* sText, float fX, float fY, uint32_t iColor)
{
	if ( pDemo->bFontReady ) {
		xgeTextDraw(&pDemo->tFont, sText, fX, fY, iColor);
	}
}

static int DebugOverlayFrame(void* pUser)
{
	debug_overlay_demo_t* pDemo;
	xge_debug_stats_t tStats;
	char arrLine[256];
	float fFrameBar;
	float fAvgBar;

	pDemo = (debug_overlay_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	__debugOverlayLoadFont(pDemo);
	pDemo->fTime += xgeGetDelta();

	xgeClear(XGE_COLOR_RGBA(14, 18, 24, 255));
	xgeShapeRectFillPx((xge_rect_t){ 40.0f, 42.0f, 560.0f, 346.0f }, XGE_COLOR_RGBA(28, 34, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 40.0f, 42.0f, 560.0f, 346.0f }, 2.0f, XGE_COLOR_RGBA(90, 118, 136, 255));
	xgeShapeCircleFillPx(500.0f + 60.0f * sinf(pDemo->fTime), 320.0f, 30.0f, XGE_COLOR_RGBA(80, 170, 255, 220));
	xgeShapeRectFillPx((xge_rect_t){ 88.0f, 274.0f, 120.0f, 72.0f }, XGE_COLOR_RGBA(255, 170, 74, 210));

	memset(&tStats, 0, sizeof(tStats));
	xgeDebugGetStats(&tStats);
	fFrameBar = tStats.tFrame.fFrameTimeMs * 8.0f;
	fAvgBar = tStats.tFrame.fFrameTimeAvgMs * 8.0f;
	if ( fFrameBar > 420.0f ) {
		fFrameBar = 420.0f;
	}
	if ( fAvgBar > 420.0f ) {
		fAvgBar = 420.0f;
	}

	xgeShapeRectFillPx((xge_rect_t){ 68.0f, 70.0f, 500.0f, 150.0f }, XGE_COLOR_RGBA(10, 12, 16, 210));
	snprintf(arrLine, sizeof(arrLine), "FPS %d  frame %.3f ms  avg %.3f ms  max %.3f ms", xgeGetFPS(), tStats.tFrame.fFrameTimeMs, tStats.tFrame.fFrameTimeAvgMs, tStats.tFrame.fFrameTimeMaxMs);
	__debugOverlayDrawText(pDemo, arrLine, 86.0f, 86.0f, XGE_COLOR_RGBA(235, 244, 250, 255));
	snprintf(arrLine, sizeof(arrLine), "draw %d  batch %d  dirty %d", tStats.tFrame.iDrawCallCount, tStats.tFrame.iBatchCount, tStats.tFrame.iDirtyRectCount);
	__debugOverlayDrawText(pDemo, arrLine, 86.0f, 118.0f, XGE_COLOR_RGBA(170, 220, 255, 255));
	snprintf(arrLine, sizeof(arrLine), "texture %d  texture memory %llu  font %d  audio %d", tStats.iTextureCount, (unsigned long long)tStats.iTextureMemoryBytes, tStats.iFontCount, tStats.iAudioCount);
	__debugOverlayDrawText(pDemo, arrLine, 86.0f, 150.0f, XGE_COLOR_RGBA(255, 218, 136, 255));
	snprintf(arrLine, sizeof(arrLine), "last GL error 0x%04X", tStats.iLastGLError);
	__debugOverlayDrawText(pDemo, arrLine, 86.0f, 182.0f, XGE_COLOR_RGBA(180, 255, 180, 255));

	xgeShapeRectFillPx((xge_rect_t){ 86.0f, 236.0f, fFrameBar, 14.0f }, XGE_COLOR_RGBA(110, 190, 255, 255));
	xgeShapeRectFillPx((xge_rect_t){ 86.0f, 260.0f, fAvgBar, 14.0f }, XGE_COLOR_RGBA(255, 190, 92, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 86.0f, 236.0f, 420.0f, 14.0f }, 1.0f, XGE_COLOR_RGBA(230, 238, 244, 140));
	xgeShapeRectStrokePx((xge_rect_t){ 86.0f, 260.0f, 420.0f, 14.0f }, 1.0f, XGE_COLOR_RGBA(230, 238, 244, 140));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	debug_overlay_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Debug Overlay";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(DebugOverlayFrame, &tDemo);
	xgeFontFree(&tDemo.tFont);
	xgeUnit();
	return 0;
}
