#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct smoke_state_t {
	int iFrameLimit;
	double fSecondLimit;
	int bFinalDumped;
} smoke_state_t;

static int __smokeArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static double __smokeArgDouble(const char* sText, double fDefault)
{
	double fValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return fDefault;
	}
	fValue = atof(sText);
	return (fValue > 0.0) ? fValue : fDefault;
}

static void __smokePrintRuntime(const char* sPrefix)
{
	xge_platform_runtime_t tRuntime;

	memset(&tRuntime, 0, sizeof(tRuntime));
	if ( xgePlatformRuntimeGet(&tRuntime) == XGE_OK ) {
		printf("%s running=%d window=%dx%d framebuffer=%dx%d dpi=%.2f key=%d text=%d mouse=%d touch=%d gamepad=%d resize=%d quit=%d\n",
			sPrefix,
			tRuntime.bRunning,
			tRuntime.iWindowWidth, tRuntime.iWindowHeight,
			tRuntime.iFramebufferWidth, tRuntime.iFramebufferHeight,
			tRuntime.fDpiScale,
			tRuntime.iKeyEventCount, tRuntime.iTextEventCount,
			tRuntime.iMouseEventCount, tRuntime.iTouchEventCount,
			tRuntime.iGamepadEventCount, tRuntime.iResizeEventCount,
			tRuntime.iQuitEventCount);
	}
}

static int MainFrame(void* pUser)
{
	static int bDumped = 0;
	static int bTextureReady = 0;
	static int iFrameCount = 0;
	static xge_texture_t tTexture;
	static unsigned char arrPixels[64 * 64 * 4];
	smoke_state_t* pState;
	xge_rect_t tRect;
#if XGE_HAS_DEBUGMODE
	char arrCaps[2048];
#endif
	int iX;
	int iY;

	pState = (smoke_state_t*)pUser;
	iFrameCount++;
	if ( bDumped == 0 ) {
#if XGE_HAS_DEBUGMODE
		memset(arrCaps, 0, sizeof(arrCaps));
		(void)xgeDebugDumpCaps(arrCaps, (int)sizeof(arrCaps));
		printf("%s\n", arrCaps);
#endif
		bDumped = 1;
	}
	if ( (iFrameCount == 1) || ((iFrameCount % 120) == 0) ) {
		__smokePrintRuntime("Platform Runtime:");
	}
	if ( bTextureReady == 0 ) {
		for ( iY = 0; iY < 64; iY++ ) {
			for ( iX = 0; iX < 64; iX++ ) {
				int iPos = ((iY * 64) + iX) * 4;
				int bAlt = (((iX / 8) + (iY / 8)) & 1);
				arrPixels[iPos + 0] = bAlt ? 255 : 48;
				arrPixels[iPos + 1] = bAlt ? 220 : 130;
				arrPixels[iPos + 2] = bAlt ? 80 : 230;
				arrPixels[iPos + 3] = 255;
			}
		}
		if ( xgeTextureCreateRGBA(&tTexture, 64, 64, arrPixels) == XGE_OK ) {
			bTextureReady = 1;
		}
	}
	if ( xgeKeyDown(256) ) {
		if ( bTextureReady != 0 ) {
			xgeTextureFree(&tTexture);
			bTextureReady = 0;
		}
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));
	tRect.fX = 24.0f;
	tRect.fY = 24.0f;
	tRect.fW = 180.0f;
	tRect.fH = 96.0f;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(64, 128, 220, 255));
	xgeShapeCircleFill(256.0f, 72.0f, 48.0f, XGE_COLOR_RGBA(240, 96, 72, 255));
	if ( bTextureReady != 0 ) {
		xgeDraw(&tTexture, 344.0f, 40.0f);
	}
	if ( (pState != NULL) && (pState->bFinalDumped == 0) ) {
		if ( ((pState->iFrameLimit > 0) && (iFrameCount >= pState->iFrameLimit)) || ((pState->fSecondLimit > 0.0) && (xgeTimer() >= pState->fSecondLimit)) ) {
			__smokePrintRuntime("Platform Runtime Final:");
			pState->bFinalDumped = 1;
			if ( bTextureReady != 0 ) {
				xgeTextureFree(&tTexture);
				bTextureReady = 0;
			}
			xgeQuit();
			return 1;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	smoke_state_t tState;
	int i;

	memset(&tState, 0, sizeof(tState));
	tState.iFrameLimit = __smokeArgInt(getenv("XGE_SMOKE_FRAMES"), 0);
	tState.fSecondLimit = __smokeArgDouble(getenv("XGE_SMOKE_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tState.iFrameLimit = __smokeArgInt(argv[++i], tState.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tState.fSecondLimit = __smokeArgDouble(argv[++i], tState.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 600;
	tDesc.sTitle = "XGE Platform Smoke";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, &tState);
	xgeUnit();
	return 0;
}
