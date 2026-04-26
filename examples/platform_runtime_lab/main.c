#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct platform_runtime_lab_t {
	xge_font_t tFont;
	xge_platform_caps_t tCaps;
	xge_gpu_caps_t tGpuCaps;
	xge_platform_runtime_t tRuntime;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bFreed;
	int bFontOK;
	int bCapsOK;
	int bRuntimeOK;
	int bGpuCapsOK;
	int bGamepadSimOK;
	int bCIReady;
	int bGamepadConnected;
	int bGamepadStateSet;
	int bGamepadDisconnected;
	char sFontPath[260];
} platform_runtime_lab_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static double ArgDouble(const char* sText, double fDefault)
{
	double fValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return fDefault;
	}
	fValue = atof(sText);
	return (fValue > 0.0) ? fValue : fDefault;
}

static int ChooseFontPath(char* sPath, int iSize)
{
#if defined(_WIN32)
	static const char* arrCandidates[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/segoeui.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};
#else
	static const char* arrCandidates[] = {
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf"
	};
#endif
	FILE* pFile;
	int i;

	if ( (sPath == NULL) || (iSize <= 0) ) {
		return 0;
	}
	for ( i = 0; i < (int)(sizeof(arrCandidates) / sizeof(arrCandidates[0])); i++ ) {
		pFile = fopen(arrCandidates[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			snprintf(sPath, iSize, "%s", arrCandidates[i]);
			return 1;
		}
	}
	sPath[0] = 0;
	return 0;
}

static int InitLab(platform_runtime_lab_t* pLab)
{
	if ( pLab->bReady ) {
		return XGE_OK;
	}
	pLab->bCapsOK = (xgePlatformCapsGet(&pLab->tCaps) == XGE_OK);
	pLab->bGpuCapsOK = (xgeGpuCapsGet(&pLab->tGpuCaps) == XGE_OK);
	pLab->bRuntimeOK = (xgePlatformRuntimeGet(&pLab->tRuntime) == XGE_OK);
	if ( ChooseFontPath(pLab->sFontPath, (int)sizeof(pLab->sFontPath)) ) {
		pLab->bFontOK = (xgeFontLoad(&pLab->tFont, pLab->sFontPath, 18.0f) == XGE_OK);
	}
	pLab->bCIReady = pLab->bCapsOK && pLab->bGpuCapsOK && pLab->bRuntimeOK;
	if ( !pLab->bCIReady ) {
		fprintf(stderr, "platform-runtime-lab stage failed: caps=%d gpu=%d runtime=%d\n",
			pLab->bCapsOK, pLab->bGpuCapsOK, pLab->bRuntimeOK);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pLab->bReady = 1;
	printf("platform-runtime-lab init caps=%d gpu=%d runtime=%d font=%d highdpi=%d gamepad=%d renderer=%s\n",
		pLab->bCapsOK,
		pLab->bGpuCapsOK,
		pLab->bRuntimeOK,
		pLab->bFontOK,
		pLab->tCaps.bHighDPI,
		pLab->tCaps.bGamepad,
		pLab->tGpuCaps.sRenderer);
	return XGE_OK;
}

static void SimulateGamepad(platform_runtime_lab_t* pLab)
{
	xge_gamepad_state_t tState;

	if ( !pLab->tCaps.bGamepad ) {
		return;
	}
	if ( (pLab->bGamepadConnected == 0) && (pLab->iFrameCount >= 1) ) {
		pLab->bGamepadConnected = (xgeGamepadSetConnected(0, 1) == XGE_OK);
	}
	if ( (pLab->bGamepadConnected != 0) && (pLab->bGamepadStateSet == 0) && (pLab->iFrameCount >= 2) ) {
		memset(&tState, 0, sizeof(tState));
		tState.bConnected = 1;
		tState.iButtons = XGE_GAMEPAD_A | XGE_GAMEPAD_DPAD_RIGHT;
		tState.arrAxes[0] = 0.60f;
		tState.arrAxes[1] = -0.35f;
		pLab->bGamepadStateSet = (xgeGamepadSetState(0, &tState) == XGE_OK);
	}
	if ( (pLab->bGamepadStateSet != 0) && (pLab->bGamepadDisconnected == 0) && (pLab->iFrameCount >= 3) ) {
		pLab->bGamepadDisconnected = (xgeGamepadSetConnected(0, 0) == XGE_OK);
	}
	pLab->bGamepadSimOK = pLab->bGamepadConnected && pLab->bGamepadStateSet && pLab->bGamepadDisconnected;
}

static void DrawTextBlock(xge_font pFont, const char* sText, xge_rect_t tRect)
{
	if ( (pFont == NULL) || (sText == NULL) ) {
		return;
	}
	xgeTextDrawRect(pFont, sText, tRect, XGE_COLOR_RGBA(232, 240, 246, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP);
}

static void DrawRuntimePanels(platform_runtime_lab_t* pLab)
{
	char sRuntime[512];
	char sCounts[512];
	char sCaps[512];

	snprintf(sRuntime, sizeof(sRuntime),
		"running=%d\nwindow=%dx%d\nframebuffer=%dx%d\ndpi=%.2f\nrenderer=%s",
		pLab->tRuntime.bRunning,
		pLab->tRuntime.iWindowWidth, pLab->tRuntime.iWindowHeight,
		pLab->tRuntime.iFramebufferWidth, pLab->tRuntime.iFramebufferHeight,
		pLab->tRuntime.fDpiScale,
		pLab->tGpuCaps.sRenderer);
	snprintf(sCounts, sizeof(sCounts),
		"key=%d\ntext=%d\nmouse=%d\ntouch=%d\ngamepad=%d\nresize=%d\nquit=%d",
		pLab->tRuntime.iKeyEventCount,
		pLab->tRuntime.iTextEventCount,
		pLab->tRuntime.iMouseEventCount,
		pLab->tRuntime.iTouchEventCount,
		pLab->tRuntime.iGamepadEventCount,
		pLab->tRuntime.iResizeEventCount,
		pLab->tRuntime.iQuitEventCount);
	snprintf(sCaps, sizeof(sCaps),
		"window=%d\naudio=%d\nmouse=%d\nkeyboard=%d\ntext=%d\ngamepad=%d\nhighdpi=%d\nmax_texture=%d",
		pLab->tCaps.bWindow,
		pLab->tCaps.bAudio,
		pLab->tCaps.bMouse,
		pLab->tCaps.bKeyboard,
		pLab->tCaps.bTextInput,
		pLab->tCaps.bGamepad,
		pLab->tCaps.bHighDPI,
		pLab->tGpuCaps.iMaxTextureSize);

	xgeShapeRectFillPx((xge_rect_t){ 26.0f, 30.0f, 186.0f, 218.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 228.0f, 30.0f, 186.0f, 218.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectFillPx((xge_rect_t){ 430.0f, 30.0f, 184.0f, 218.0f }, XGE_COLOR_RGBA(30, 36, 42, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 26.0f, 30.0f, 186.0f, 218.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 228.0f, 30.0f, 186.0f, 218.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 430.0f, 30.0f, 184.0f, 218.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));
	xgeShapeRectFillPx((xge_rect_t){ 26.0f, 270.0f, 588.0f, 74.0f }, XGE_COLOR_RGBA(24, 30, 36, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 26.0f, 270.0f, 588.0f, 74.0f }, 2.0f, XGE_COLOR_RGBA(106, 124, 140, 255));

	if ( pLab->bFontOK ) {
		DrawTextBlock(&pLab->tFont, sRuntime, (xge_rect_t){ 40.0f, 42.0f, 156.0f, 190.0f });
		DrawTextBlock(&pLab->tFont, sCounts, (xge_rect_t){ 242.0f, 42.0f, 156.0f, 190.0f });
		DrawTextBlock(&pLab->tFont, sCaps, (xge_rect_t){ 444.0f, 42.0f, 156.0f, 190.0f });
		DrawTextBlock(&pLab->tFont,
			"CI smoke: window + runtime + counters + dpi + gamepad simulation\nmanual: move mouse, type text, resize window, press ESC",
			(xge_rect_t){ 42.0f, 286.0f, 556.0f, 42.0f });
	}

	xgeShapeRectFillPx((xge_rect_t){ 52.0f, 302.0f, (float)(pLab->tRuntime.iMouseEventCount * 6), 10.0f }, XGE_COLOR_RGBA(86, 176, 255, 255));
	xgeShapeRectFillPx((xge_rect_t){ 52.0f, 318.0f, (float)(pLab->tRuntime.iKeyEventCount * 6), 10.0f }, XGE_COLOR_RGBA(255, 156, 86, 255));
	xgeShapeRectFillPx((xge_rect_t){ 52.0f, 334.0f, (float)(pLab->tRuntime.iGamepadEventCount * 6), 10.0f }, XGE_COLOR_RGBA(126, 218, 132, 255));
}

static void CleanupLab(platform_runtime_lab_t* pLab)
{
	if ( pLab->bFreed ) {
		return;
	}
	if ( pLab->bGamepadConnected && !pLab->bGamepadDisconnected ) {
		(void)xgeGamepadSetConnected(0, 0);
	}
	if ( pLab->bFontOK ) {
		xgeFontFree(&pLab->tFont);
	}
	pLab->bFreed = 1;
	pLab->bReady = 0;
}

static void PrintFinalSummary(platform_runtime_lab_t* pLab)
{
	printf("platform-runtime-lab final-summary frames=%d caps=%d gpu=%d runtime=%d ci=%d gamepad_sim=%d running=%d window=%dx%d framebuffer=%dx%d dpi=%.2f counts(key=%d text=%d mouse=%d touch=%d gamepad=%d resize=%d quit=%d)\n",
		pLab->iFrameCount,
		pLab->bCapsOK,
		pLab->bGpuCapsOK,
		pLab->bRuntimeOK,
		pLab->bCIReady,
		pLab->bGamepadSimOK || !pLab->tCaps.bGamepad,
		pLab->tRuntime.bRunning,
		pLab->tRuntime.iWindowWidth, pLab->tRuntime.iWindowHeight,
		pLab->tRuntime.iFramebufferWidth, pLab->tRuntime.iFramebufferHeight,
		pLab->tRuntime.fDpiScale,
		pLab->tRuntime.iKeyEventCount,
		pLab->tRuntime.iTextEventCount,
		pLab->tRuntime.iMouseEventCount,
		pLab->tRuntime.iTouchEventCount,
		pLab->tRuntime.iGamepadEventCount,
		pLab->tRuntime.iResizeEventCount,
		pLab->tRuntime.iQuitEventCount);
}

static int PlatformRuntimeFrame(void* pUser)
{
	platform_runtime_lab_t* pLab;
	int iRet;

	pLab = (platform_runtime_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	SimulateGamepad(pLab);
	(void)xgePlatformRuntimeGet(&pLab->tRuntime);
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	DrawRuntimePanels(pLab);
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	platform_runtime_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_PLATFORM_RUNTIME_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_PLATFORM_RUNTIME_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 372;
	tDesc.sTitle = "XGE Platform Runtime Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(PlatformRuntimeFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("platform-runtime-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
