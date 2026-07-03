#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANUAL_KEY_ESCAPE 256
#define MANUAL_KEY_SPACE 32
#define MANUAL_KEY_G 'G'

typedef struct manual_state_t {
	xge_texture_t tChecker;
	xge_font_t tFont;
	int bCheckerReady;
	int bFontReady;
	int iFrameCount;
	int iLeftClicks;
	int iRightClicks;
	int iSpacePresses;
	int iWheelEvents;
	int iTextEvents;
	int iLastCodepoint;
	float fPulse;
	float fWheelPulse;
	float fAnim;
	int bGamepadSimulated;
	int bLeftWasDown;
	int bRightWasDown;
	int bCapsDumped;
	const char* sFontPath;
	int bBenchmark;
	double fBenchmarkSeconds;
	double fBenchmarkStart;
	int iBenchmarkStartFrame;
	float fBenchmarkMinDt;
	float fBenchmarkMaxDt;
	int iStressCopies;
	int bStressTextureOnly;
	int bStressShapeOnly;
	int iLastDrawCallCount;
	int iLastBatchCount;
} manual_state_t;

static void __manualMakeChecker(manual_state_t* pState)
{
	unsigned char arrPixels[96 * 96 * 4];
	int iX;
	int iY;

	memset(&pState->tChecker, 0, sizeof(pState->tChecker));
	for ( iY = 0; iY < 96; iY++ ) {
		for ( iX = 0; iX < 96; iX++ ) {
			int iPos;
			int bGrid;
			int bDiag;

			iPos = ((iY * 96) + iX) * 4;
			bGrid = (((iX / 12) + (iY / 12)) & 1);
			bDiag = ((iX + iY) % 24) < 12;
			arrPixels[iPos + 0] = (unsigned char)(bGrid ? 245 : 40);
			arrPixels[iPos + 1] = (unsigned char)(bDiag ? 210 : 95);
			arrPixels[iPos + 2] = (unsigned char)(bGrid ? 75 : 220);
			arrPixels[iPos + 3] = 255;
		}
	}
	pState->bCheckerReady = (xgeTextureCreateRGBA(&pState->tChecker, 96, 96, arrPixels) == XGE_OK) ? 1 : 0;
}

static int __manualLoadFont(manual_state_t* pState, const char* sPreferred)
{
	const char* arrFonts[] = {
		NULL,
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/NotoSansSC-VF.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	arrFonts[0] = sPreferred;
	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		if ( (arrFonts[i] != NULL) && (arrFonts[i][0] != 0) ) {
			memset(&pState->tFont, 0, sizeof(pState->tFont));
			if ( xgeFontLoad(&pState->tFont, arrFonts[i], 20.0f) == XGE_OK ) {
				pState->bFontReady = 1;
				pState->sFontPath = arrFonts[i];
				printf("font loaded: %s\n", arrFonts[i]);
				return XGE_OK;
			}
		}
	}
	printf("font load failed: tried simhei/NotoSansSC/Deng/msyh/simsun/arial\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void __manualText(manual_state_t* pState, const char* sText, float fX, float fY, uint32_t iColor)
{
	if ( pState->bFontReady != 0 ) {
		xgeTextDraw(&pState->tFont, sText, fX, fY, iColor);
	}
}

static void __manualTextRect(manual_state_t* pState, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( pState->bFontReady != 0 ) {
		xgeTextDrawRect(&pState->tFont, sText, tRect, iColor, iFlags);
	}
}

static void __manualDrawPanel(float fX, float fY, float fW, float fH, uint32_t iFill, uint32_t iStroke)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	xgeShapeRectFillPx(tRect, iFill);
	xgeShapeRectStrokePx(tRect, 2.0f, iStroke);
}

static void __manualDrawRuntime(manual_state_t* pState)
{
	xge_platform_runtime_t tRuntime;
	xge_gamepad_state_t tPad;
	char sLine[256];
	float fMouseX;
	float fMouseY;
	float fMouseDX;
	float fMouseDY;
	float fWheelX;
	float fWheelY;
	int iTouchCount;
	int i;

	memset(&tRuntime, 0, sizeof(tRuntime));
	memset(&tPad, 0, sizeof(tPad));
	xgePlatformRuntimeGet(&tRuntime);
	xgeMouseGet(&fMouseX, &fMouseY);
	xgeMouseGetDelta(&fMouseDX, &fMouseDY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	iTouchCount = xgeTouchGetCount();

	xgeShapeRectFillPx((xge_rect_t){ 20.0f, 18.0f, 760.0f, 224.0f }, XGE_COLOR_RGBA(8, 12, 18, 218));
	xgeShapeRectStrokePx((xge_rect_t){ 20.0f, 18.0f, 760.0f, 224.0f }, 2.0f, XGE_COLOR_RGBA(82, 100, 126, 220));
	snprintf(sLine, sizeof(sLine), "FPS %d  dt %.3f  frame %d  draw %d  batch %d  stress %d", xgeGetFPS(), xgeGetDelta(), pState->iFrameCount, pState->iLastDrawCallCount, pState->iLastBatchCount, pState->iStressCopies);
	__manualText(pState, sLine, 32.0f, 26.0f, XGE_COLOR_RGBA(245, 248, 250, 255));
	snprintf(sLine, sizeof(sLine), "window %dx%d  framebuffer %dx%d  dpi %.2f", tRuntime.iWindowWidth, tRuntime.iWindowHeight, tRuntime.iFramebufferWidth, tRuntime.iFramebufferHeight, tRuntime.fDpiScale);
	__manualText(pState, sLine, 32.0f, 58.0f, XGE_COLOR_RGBA(185, 220, 255, 255));
	snprintf(sLine, sizeof(sLine), "mouse %.0f,%.0f  delta %.1f,%.1f  wheel %.1f,%.1f", fMouseX, fMouseY, fMouseDX, fMouseDY, fWheelX, fWheelY);
	__manualText(pState, sLine, 32.0f, 90.0f, XGE_COLOR_RGBA(220, 230, 240, 255));
	snprintf(sLine, sizeof(sLine), "left %d  right %d  space %d  wheel events %d  text events %d  last U+%04X", pState->iLeftClicks, pState->iRightClicks, pState->iSpacePresses, pState->iWheelEvents, pState->iTextEvents, pState->iLastCodepoint);
	__manualText(pState, sLine, 32.0f, 122.0f, XGE_COLOR_RGBA(255, 232, 160, 255));
	snprintf(sLine, sizeof(sLine), "runtime key=%d text=%d mouse=%d touch=%d gamepad=%d resize=%d quit=%d", tRuntime.iKeyEventCount, tRuntime.iTextEventCount, tRuntime.iMouseEventCount, tRuntime.iTouchEventCount, tRuntime.iGamepadEventCount, tRuntime.iResizeEventCount, tRuntime.iQuitEventCount);
	__manualText(pState, sLine, 32.0f, 154.0f, XGE_COLOR_RGBA(174, 230, 184, 255));
	snprintf(sLine, sizeof(sLine), "touch count %d", iTouchCount);
	__manualText(pState, sLine, 32.0f, 186.0f, XGE_COLOR_RGBA(220, 210, 255, 255));

	if ( xgeGamepadGetState(0, &tPad) == XGE_OK ) {
		snprintf(sLine, sizeof(sLine), "gamepad0 connected=%d buttons=0x%08X axis0=%.2f axis1=%.2f", tPad.bConnected, tPad.iButtons, tPad.arrAxes[0], tPad.arrAxes[1]);
		__manualText(pState, sLine, 32.0f, 218.0f, tPad.bConnected ? XGE_COLOR_RGBA(120, 255, 170, 255) : XGE_COLOR_RGBA(255, 210, 120, 255));
	}
	for ( i = 0; i < iTouchCount && i < 4; i++ ) {
		xge_touch_point_t tTouch;
		if ( xgeTouchGet(i, &tTouch) == XGE_OK ) {
			xgeShapeCircleStrokePx(tTouch.fX, tTouch.fY, 28.0f, 4.0f, XGE_COLOR_RGBA(255, 210, 80, 255));
		}
	}
}

static void __manualDrawVisuals(manual_state_t* pState)
{
	xge_rect_t tRect;
	float fMouseX;
	float fMouseY;
	float fPulseW;
	float fWheelR;
	float fMoveX;
	int i;

	xgeMouseGet(&fMouseX, &fMouseY);
	fPulseW = 120.0f + (pState->fPulse * 80.0f);
	fWheelR = 24.0f + (pState->fWheelPulse * 28.0f);
	fMoveX = 470.0f + (float)((int)(pState->fAnim * 80.0f) % 220);

	for ( i = 0; i < pState->iStressCopies; i++ ) {
		float fX;
		float fY;
		xge_draw_t tStressDraw;

		fX = 16.0f + (float)((i * 37) % 920);
		fY = 232.0f + (float)((i * 53) % 300);
		if ( pState->bStressTextureOnly == 0 ) {
			xgeShapeCircleFillPx(fX, fY, 5.0f + (float)(i % 7), XGE_COLOR_RGBA((i * 47) & 255, (80 + i * 19) & 255, (160 + i * 31) & 255, 128));
		}
		if ( (pState->bStressShapeOnly == 0) && (pState->bCheckerReady != 0) ) {
			memset(&tStressDraw, 0, sizeof(tStressDraw));
			tStressDraw.pTexture = &pState->tChecker;
			tStressDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, 48.0f, 48.0f };
			tStressDraw.tDst = (xge_rect_t){ fX + 8.0f, fY + 8.0f, 20.0f, 20.0f };
			tStressDraw.tOrigin = (xge_vec2_t){ 10.0f, 10.0f };
			tStressDraw.fRotation = pState->fAnim * 2.0f + (float)i * 0.013f;
			tStressDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 110);
			xgeDrawEx(&tStressDraw);
		}
	}
	(void)xgeFlush();

	__manualDrawPanel(28.0f, 260.0f, 240.0f, 122.0f, XGE_COLOR_RGBA(34, 44, 58, 235), XGE_COLOR_RGBA(105, 130, 165, 255));
	__manualDrawPanel(292.0f, 260.0f, 240.0f, 122.0f, XGE_COLOR_RGBA(42, 36, 54, 235), XGE_COLOR_RGBA(140, 108, 175, 255));
	__manualDrawPanel(556.0f, 260.0f, 240.0f, 122.0f, XGE_COLOR_RGBA(32, 50, 42, 235), XGE_COLOR_RGBA(100, 165, 120, 255));

	tRect.fX = 52.0f;
	tRect.fY = 306.0f;
	tRect.fW = fPulseW;
	tRect.fH = 34.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(66, 138, 230, 255));
	xgeShapeRectStrokePx(tRect, 3.0f, XGE_COLOR_RGBA(220, 238, 255, 255));
	xgeShapeCircleFillPx(204.0f, 322.0f, fWheelR, XGE_COLOR_RGBA(255, 200, 84, 230));
	xgeShapeLinePx(52.0f, 358.0f, 228.0f, 286.0f, 3.0f, XGE_COLOR_RGBA(255, 114, 126, 255));

	if ( pState->bCheckerReady != 0 ) {
		xge_draw_t tDraw;

		xgeDraw(&pState->tChecker, 324.0f, 280.0f);
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = &pState->tChecker;
		tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, 48.0f, 48.0f };
		tDraw.tDst = (xge_rect_t){ 430.0f, 292.0f, 72.0f, 72.0f };
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 180);
		xgeDrawEx(&tDraw);
	}

	xgeShapeCircleFillPx(fMoveX, 322.0f, 26.0f, XGE_COLOR_RGBA(88, 210, 150, 240));
	xgeShapeCircleStrokePx(fMoveX, 322.0f, 34.0f, 3.0f, XGE_COLOR_RGBA(210, 255, 228, 255));
	tRect.fX = 604.0f;
	tRect.fY = 288.0f;
	tRect.fW = 140.0f;
	tRect.fH = 68.0f;
	xgeShapeRectStrokePx(tRect, 3.0f, XGE_COLOR_RGBA(230, 245, 255, 255));
	xgeShapeLinePx(604.0f, 322.0f, 744.0f, 322.0f, 2.0f, XGE_COLOR_RGBA(130, 180, 255, 255));

}

static void __manualDrawInstructions(manual_state_t* pState)
{
	__manualTextRect(pState,
		"XGE Manual Validation\n"
		"ESC quit | SPACE pulse | mouse move/click/wheel | type English/Chinese IME | resize window | G toggles simulated gamepad",
		(xge_rect_t){ 32.0f, 420.0f, 740.0f, 92.0f },
		XGE_COLOR_RGBA(235, 240, 245, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP);
	if ( pState->bFontReady == 0 ) {
		xgeShapeRectFillPx((xge_rect_t){ 32.0f, 420.0f, 560.0f, 42.0f }, XGE_COLOR_RGBA(120, 50, 50, 255));
	}
}

static void __manualDrawCursor(void)
{
	float fMouseX;
	float fMouseY;

	xgeMouseGet(&fMouseX, &fMouseY);
	xgeShapeLinePx(fMouseX - 18.0f, fMouseY, fMouseX + 18.0f, fMouseY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	xgeShapeLinePx(fMouseX, fMouseY - 18.0f, fMouseX, fMouseY + 18.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	xgeShapeCircleStrokePx(fMouseX, fMouseY, xgeMouseDown(XGE_MOUSE_LEFT) ? 34.0f : 24.0f, xgeMouseDown(XGE_MOUSE_RIGHT) ? 6.0f : 3.0f, XGE_COLOR_RGBA(255, 128, 160, 255));
}

static int ManualFrame(void* pUser)
{
	manual_state_t* pState;
	float fWheelX;
	float fWheelY;
	uint32_t iCodepoint;
	xge_gamepad_state_t tPad;
	int bLeftDown;
	int bRightDown;

	pState = (manual_state_t*)pUser;
	pState->iFrameCount++;
	if ( (pState->bBenchmark != 0) && (pState->fBenchmarkStart <= 0.0) && (pState->iFrameCount > 5) ) {
		pState->fBenchmarkStart = xgeTimer();
		pState->iBenchmarkStartFrame = pState->iFrameCount;
		pState->fBenchmarkMinDt = 9999.0f;
		pState->fBenchmarkMaxDt = 0.0f;
	}
	if ( pState->fBenchmarkStart > 0.0 ) {
		float fDt;

		fDt = xgeGetDelta();
		if ( fDt < pState->fBenchmarkMinDt ) {
			pState->fBenchmarkMinDt = fDt;
		}
		if ( fDt > pState->fBenchmarkMaxDt ) {
			pState->fBenchmarkMaxDt = fDt;
		}
	}
	if ( pState->bCapsDumped == 0 ) {
#if XGE_HAS_DEBUGMODE
		char sCaps[2048];

		memset(sCaps, 0, sizeof(sCaps));
		(void)xgeDebugDumpCaps(sCaps, (int)sizeof(sCaps));
		printf("%s\n", sCaps);
#endif
		pState->bCapsDumped = 1;
	}
	pState->fAnim += xgeGetDelta();
	if ( xgeKeyDown(MANUAL_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	if ( (pState->bBenchmark != 0) && (pState->fBenchmarkStart > 0.0) && ((xgeTimer() - pState->fBenchmarkStart) >= pState->fBenchmarkSeconds) ) {
		xgeQuit();
		return 1;
	}
	if ( xgeKeyPressed(MANUAL_KEY_SPACE) ) {
		pState->iSpacePresses++;
		pState->fPulse = 1.0f;
	}
	bLeftDown = xgeMouseDown(XGE_MOUSE_LEFT);
	bRightDown = xgeMouseDown(XGE_MOUSE_RIGHT);
	if ( (bLeftDown != 0) && (pState->bLeftWasDown == 0) ) {
		pState->iLeftClicks++;
		pState->fPulse = 1.0f;
	}
	if ( (bRightDown != 0) && (pState->bRightWasDown == 0) ) {
		pState->iRightClicks++;
	}
	pState->bLeftWasDown = bLeftDown;
	pState->bRightWasDown = bRightDown;
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	if ( (fWheelX != 0.0f) || (fWheelY != 0.0f) ) {
		pState->iWheelEvents++;
		pState->fWheelPulse = 1.0f;
	}
	iCodepoint = xgeTextGet();
	if ( iCodepoint != 0 ) {
		pState->iTextEvents++;
		pState->iLastCodepoint = (int)iCodepoint;
	}
	if ( xgeKeyPressed(MANUAL_KEY_G) ) {
		memset(&tPad, 0, sizeof(tPad));
		pState->bGamepadSimulated = pState->bGamepadSimulated ? 0 : 1;
		if ( pState->bGamepadSimulated ) {
			tPad.bConnected = 1;
			tPad.iButtons = XGE_GAMEPAD_A | XGE_GAMEPAD_DPAD_RIGHT;
			tPad.arrAxes[0] = 0.65f;
			tPad.arrAxes[1] = -0.35f;
			xgeGamepadSetState(0, &tPad);
		} else {
			xgeGamepadSetConnected(0, 0);
		}
	}

	xgeClear(XGE_COLOR_RGBA(16, 21, 29, 255));
	__manualDrawVisuals(pState);
	(void)xgeFlush();
	__manualDrawRuntime(pState);
	__manualDrawInstructions(pState);
	(void)xgeFlush();
	__manualDrawCursor();
	(void)xgeFlush();
	{
		xge_frame_stats_t tStats;

		tStats = xgeFrameStatsGet();
		pState->iLastDrawCallCount = tStats.iDrawCallCount;
		pState->iLastBatchCount = tStats.iBatchCount;
	}

	if ( pState->fPulse > 0.0f ) {
		pState->fPulse -= xgeGetDelta() * 2.8f;
		if ( pState->fPulse < 0.0f ) {
			pState->fPulse = 0.0f;
		}
	}
	if ( pState->fWheelPulse > 0.0f ) {
		pState->fWheelPulse -= xgeGetDelta() * 3.0f;
		if ( pState->fWheelPulse < 0.0f ) {
			pState->fWheelPulse = 0.0f;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	manual_state_t tState;
	const char* sFont;
	int bNoVSync;
	int i;

	sFont = NULL;
	bNoVSync = 0;
	memset(&tState, 0, sizeof(tState));
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--bench") == 0 ) {
			tState.bBenchmark = 1;
			tState.fBenchmarkSeconds = 10.0;
			bNoVSync = 1;
			if ( (i + 1) < argc ) {
				double fSeconds;

				fSeconds = atof(argv[i + 1]);
				if ( fSeconds > 0.0 ) {
					tState.fBenchmarkSeconds = fSeconds;
					i++;
				}
			}
		} else if ( strcmp(argv[i], "--novsync") == 0 ) {
			bNoVSync = 1;
		} else if ( strcmp(argv[i], "--stress") == 0 ) {
			if ( (i + 1) < argc ) {
				tState.iStressCopies = atoi(argv[++i]);
				if ( tState.iStressCopies < 0 ) {
					tState.iStressCopies = 0;
				}
			}
		} else if ( strcmp(argv[i], "--stress-texture-only") == 0 ) {
			tState.bStressTextureOnly = 1;
			tState.bStressShapeOnly = 0;
		} else if ( strcmp(argv[i], "--stress-shape-only") == 0 ) {
			tState.bStressShapeOnly = 1;
			tState.bStressTextureOnly = 0;
		} else {
			sFont = argv[i];
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 960;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XGE Manual Validation";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_RESIZABLE;
	if ( bNoVSync == 0 ) {
		tDesc.iFlags |= XGE_INIT_VSYNC;
	}
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	__manualMakeChecker(&tState);
	(void)__manualLoadFont(&tState, sFont);
	xgeRun(ManualFrame, &tState);
	if ( tState.bCheckerReady != 0 ) {
		xgeTextureFree(&tState.tChecker);
	}
	if ( tState.bFontReady != 0 ) {
		xgeFontFree(&tState.tFont);
	}
	printf("manual summary: frames=%d left=%d right=%d space=%d wheel=%d text=%d last=U+%04X draw=%d batch=%d\n",
		tState.iFrameCount,
		tState.iLeftClicks,
		tState.iRightClicks,
		tState.iSpacePresses,
		tState.iWheelEvents,
		tState.iTextEvents,
		tState.iLastCodepoint,
		tState.iLastDrawCallCount,
		tState.iLastBatchCount);
	if ( (tState.bBenchmark != 0) && (tState.fBenchmarkStart > 0.0) ) {
		double fElapsed;
		int iFrames;

		fElapsed = xgeTimer() - tState.fBenchmarkStart;
		iFrames = tState.iFrameCount - tState.iBenchmarkStartFrame;
		printf("benchmark: seconds=%.3f frames=%d avg_fps=%.1f min_frame_ms=%.3f max_frame_ms=%.3f vsync=off stress=%d draw=%d batch=%d\n",
			fElapsed,
			iFrames,
			(fElapsed > 0.0) ? ((double)iFrames / fElapsed) : 0.0,
			tState.fBenchmarkMinDt * 1000.0f,
			tState.fBenchmarkMaxDt * 1000.0f,
			tState.iStressCopies,
			tState.iLastDrawCallCount,
			tState.iLastBatchCount);
	}
	xgeUnit();
	return 0;
}
