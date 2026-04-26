#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct input_state_lab_t {
	xge_font_t tFont;
	xge_gamepad_state_t tPadState;
	xge_touch_point_t tTouch;
	xge_touch_point_t tTouchFound;
	char sFontPath[512];
	char sClipboard[128];
	float fMouseX;
	float fMouseY;
	float fMouseDX;
	float fMouseDY;
	float fWheelX;
	float fWheelY;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bDone;
	int bFontReady;
	int bClipboardOK;
	int bTouchOK;
	int bTouchFindOK;
	int bGamepadConnected;
	int bGamepadStateOK;
	int bGamepadDownOK;
	int bGamepadPressedOK;
	int bGamepadReleasedOK;
	int bGamepadAxisOK;
	int bKeyDown;
	int bKeyPressed;
	int bKeyReleased;
	int bMouseLeft;
	int iTouchCount;
} input_state_lab_t;

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

static int FileExists(const char* sPath)
{
	FILE* pFile;

	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) {
		return 0;
	}
	fclose(pFile);
	return 1;
}

static const char* FindFontPath(int argc, char** argv)
{
	static const char* arrCandidates[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf",
		"C:/Windows/Fonts/segoeui.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
		NULL
	};
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--font") == 0) && ((i + 1) < argc) ) {
			return argv[i + 1];
		}
	}
	for ( i = 0; arrCandidates[i] != NULL; i++ ) {
		if ( FileExists(arrCandidates[i]) ) {
			return arrCandidates[i];
		}
	}
	return NULL;
}

static void DrawPanel(float fX, float fY, float fW, float fH, uint32_t iColor)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeShapeRectStrokePx(tRect, 2.0f, iColor);
}

static void DrawTextLine(input_state_lab_t* pLab, const char* sText, float fX, float fY, uint32_t iColor)
{
	if ( pLab->bFontReady ) {
		xgeTextDraw(&pLab->tFont, sText, fX, fY, iColor);
	}
}

static int SetupSimulatedTouch(input_state_lab_t* pLab)
{
	if ( xgeMiniProgramTouchOne(XGE_TOUCH_BEGIN, 77, 614.0f, 94.0f, 1.0f) != XGE_OK ) {
		return 0;
	}
	if ( xgeMiniProgramTouchOne(XGE_TOUCH_MOVE, 77, 634.0f, 124.0f, 1.0f) != XGE_OK ) {
		return 0;
	}
	pLab->iTouchCount = xgeTouchGetCount();
	pLab->bTouchOK = (pLab->iTouchCount > 0) && (xgeTouchGet(0, &pLab->tTouch) == XGE_OK);
	pLab->bTouchFindOK = (xgeTouchFind(77, &pLab->tTouchFound) == XGE_OK);
	return pLab->bTouchOK && pLab->bTouchFindOK;
}

static int SetupSimulatedGamepad(input_state_lab_t* pLab)
{
	xge_gamepad_state_t tState;

	memset(&tState, 0, sizeof(tState));
	if ( xgeGamepadSetConnected(0, 1) != XGE_OK ) {
		return 0;
	}
	tState.bConnected = 1;
	tState.iButtons = XGE_GAMEPAD_A | XGE_GAMEPAD_DPAD_RIGHT;
	tState.arrAxes[0] = 0.65f;
	tState.arrAxes[1] = -0.35f;
	if ( xgeGamepadSetState(0, &tState) != XGE_OK ) {
		return 0;
	}
	tState.iButtons = XGE_GAMEPAD_B | XGE_GAMEPAD_DPAD_UP;
	tState.arrAxes[0] = -0.45f;
	tState.arrAxes[1] = 0.25f;
	if ( xgeGamepadSetState(0, &tState) != XGE_OK ) {
		return 0;
	}
	pLab->bGamepadConnected = xgeGamepadConnected(0);
	pLab->bGamepadStateOK = (xgeGamepadGetState(0, &pLab->tPadState) == XGE_OK) && (pLab->tPadState.bConnected != 0);
	pLab->bGamepadDownOK = xgeGamepadButtonDown(0, XGE_GAMEPAD_B);
	pLab->bGamepadPressedOK = xgeGamepadButtonPressed(0, XGE_GAMEPAD_A) || xgeGamepadButtonPressed(0, XGE_GAMEPAD_B);
	pLab->bGamepadReleasedOK = xgeGamepadButtonReleased(0, XGE_GAMEPAD_A);
	pLab->bGamepadAxisOK = (xgeGamepadAxis(0, 0) < -0.4f) && (xgeGamepadAxis(0, 1) > 0.2f);
	return pLab->bGamepadConnected && pLab->bGamepadStateOK && pLab->bGamepadDownOK && pLab->bGamepadPressedOK && pLab->bGamepadReleasedOK && pLab->bGamepadAxisOK;
}

static int InitLab(input_state_lab_t* pLab)
{
	const char* sClip;

	if ( pLab->bReady || pLab->bDone ) {
		return XGE_OK;
	}
	if ( pLab->sFontPath[0] != 0 ) {
		if ( xgeFontLoad(&pLab->tFont, pLab->sFontPath, 19.0f) == XGE_OK ) {
			pLab->bFontReady = 1;
		}
	}
	xgeClipboardSetText("xge-input-state-lab clipboard");
	sClip = xgeClipboardGetText();
	snprintf(pLab->sClipboard, sizeof(pLab->sClipboard), "%s", (sClip != NULL) ? sClip : "");
	pLab->bClipboardOK = strcmp(pLab->sClipboard, "xge-input-state-lab clipboard") == 0;
	if ( SetupSimulatedTouch(pLab) == 0 ) {
		fprintf(stderr, "input-state-lab stage failed: simulated touch\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( SetupSimulatedGamepad(pLab) == 0 ) {
		fprintf(stderr, "input-state-lab stage failed: simulated gamepad\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( pLab->bClipboardOK == 0 ) {
		fprintf(stderr, "input-state-lab stage failed: clipboard text='%s'\n", pLab->sClipboard);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bReady = 1;
	printf("input-state-lab init clipboard=%d touch(count=%d get=%d find=%d id=%llu pos=%.1f,%.1f) gamepad(conn=%d state=%d down=%d pressed=%d released=%d axis=%d axes=%.2f,%.2f)\n",
		pLab->bClipboardOK,
		pLab->iTouchCount,
		pLab->bTouchOK,
		pLab->bTouchFindOK,
		(unsigned long long)pLab->tTouchFound.iId,
		pLab->tTouchFound.fX,
		pLab->tTouchFound.fY,
		pLab->bGamepadConnected,
		pLab->bGamepadStateOK,
		pLab->bGamepadDownOK,
		pLab->bGamepadPressedOK,
		pLab->bGamepadReleasedOK,
		pLab->bGamepadAxisOK,
		pLab->tPadState.arrAxes[0],
		pLab->tPadState.arrAxes[1]);
	return XGE_OK;
}

static void CleanupLab(input_state_lab_t* pLab)
{
	(void)xgeMiniProgramTouchOne(XGE_TOUCH_END, 77, 634.0f, 124.0f, 0.0f);
	(void)xgeGamepadSetConnected(0, 0);
	if ( pLab->bFontReady ) {
		xgeFontFree(&pLab->tFont);
		pLab->bFontReady = 0;
	}
	pLab->bReady = 0;
}

static void UpdateLiveInput(input_state_lab_t* pLab)
{
	xgeMouseGet(&pLab->fMouseX, &pLab->fMouseY);
	xgeMouseGetDelta(&pLab->fMouseDX, &pLab->fMouseDY);
	xgeMouseGetWheel(&pLab->fWheelX, &pLab->fWheelY);
	pLab->bMouseLeft = xgeMouseDown(XGE_MOUSE_LEFT);
	pLab->bKeyDown = xgeKeyDown(XGE_KEY_SPACE);
	pLab->bKeyPressed = xgeKeyPressed(XGE_KEY_SPACE);
	pLab->bKeyReleased = xgeKeyReleased(XGE_KEY_SPACE);
}

static void DrawInputState(input_state_lab_t* pLab)
{
	char sLine[256];
	float fStickX;
	float fStickY;

	DrawPanel(24.0f, 28.0f, 236.0f, 146.0f, XGE_COLOR_RGBA(100, 186, 255, 255));
	DrawPanel(282.0f, 28.0f, 236.0f, 146.0f, XGE_COLOR_RGBA(116, 220, 156, 255));
	DrawPanel(540.0f, 28.0f, 236.0f, 146.0f, XGE_COLOR_RGBA(246, 196, 88, 255));
	DrawPanel(24.0f, 206.0f, 752.0f, 170.0f, XGE_COLOR_RGBA(235, 128, 160, 255));

	snprintf(sLine, sizeof(sLine), "key space down=%d pressed=%d released=%d", pLab->bKeyDown, pLab->bKeyPressed, pLab->bKeyReleased);
	DrawTextLine(pLab, sLine, 42.0f, 48.0f, XGE_COLOR_RGBA(232, 242, 255, 255));
	snprintf(sLine, sizeof(sLine), "mouse %.1f,%.1f d=%.1f,%.1f", pLab->fMouseX, pLab->fMouseY, pLab->fMouseDX, pLab->fMouseDY);
	DrawTextLine(pLab, sLine, 42.0f, 82.0f, XGE_COLOR_RGBA(232, 242, 255, 255));
	snprintf(sLine, sizeof(sLine), "wheel %.1f,%.1f left=%d", pLab->fWheelX, pLab->fWheelY, pLab->bMouseLeft);
	DrawTextLine(pLab, sLine, 42.0f, 116.0f, XGE_COLOR_RGBA(232, 242, 255, 255));
	xgeShapeLinePx(pLab->fMouseX - 12.0f, pLab->fMouseY, pLab->fMouseX + 12.0f, pLab->fMouseY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	xgeShapeLinePx(pLab->fMouseX, pLab->fMouseY - 12.0f, pLab->fMouseX, pLab->fMouseY + 12.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));

	snprintf(sLine, sizeof(sLine), "clipboard ok=%d", pLab->bClipboardOK);
	DrawTextLine(pLab, sLine, 300.0f, 48.0f, XGE_COLOR_RGBA(222, 250, 230, 255));
	snprintf(sLine, sizeof(sLine), "%.40s", pLab->sClipboard);
	DrawTextLine(pLab, sLine, 300.0f, 82.0f, XGE_COLOR_RGBA(222, 250, 230, 255));

	snprintf(sLine, sizeof(sLine), "touch count=%d get=%d find=%d", pLab->iTouchCount, pLab->bTouchOK, pLab->bTouchFindOK);
	DrawTextLine(pLab, sLine, 558.0f, 48.0f, XGE_COLOR_RGBA(255, 238, 190, 255));
	snprintf(sLine, sizeof(sLine), "id=%llu pos=%.1f,%.1f", (unsigned long long)pLab->tTouchFound.iId, pLab->tTouchFound.fX, pLab->tTouchFound.fY);
	DrawTextLine(pLab, sLine, 558.0f, 82.0f, XGE_COLOR_RGBA(255, 238, 190, 255));
	xgeShapeCircleStrokePx(pLab->tTouchFound.fX, pLab->tTouchFound.fY, 22.0f, 3.0f, XGE_COLOR_RGBA(255, 224, 120, 255));

	snprintf(sLine, sizeof(sLine), "gamepad connected=%d state=%d down=%d pressed=%d released=%d axis=%d", pLab->bGamepadConnected, pLab->bGamepadStateOK, pLab->bGamepadDownOK, pLab->bGamepadPressedOK, pLab->bGamepadReleasedOK, pLab->bGamepadAxisOK);
	DrawTextLine(pLab, sLine, 48.0f, 230.0f, XGE_COLOR_RGBA(255, 220, 232, 255));
	snprintf(sLine, sizeof(sLine), "buttons=0x%08X pressed=0x%08X released=0x%08X axes=%.2f,%.2f", pLab->tPadState.iButtons, pLab->tPadState.iButtonsPressed, pLab->tPadState.iButtonsReleased, pLab->tPadState.arrAxes[0], pLab->tPadState.arrAxes[1]);
	DrawTextLine(pLab, sLine, 48.0f, 264.0f, XGE_COLOR_RGBA(255, 220, 232, 255));
	fStickX = 156.0f + (xgeGamepadAxis(0, 0) * 44.0f);
	fStickY = 332.0f + (xgeGamepadAxis(0, 1) * 44.0f);
	xgeShapeCircleStrokePx(156.0f, 332.0f, 54.0f, 3.0f, XGE_COLOR_RGBA(180, 160, 180, 255));
	xgeShapeCircleFillPx(fStickX, fStickY, 15.0f, XGE_COLOR_RGBA(245, 128, 176, 255));
	xgeShapeCircleFillPx(364.0f, 332.0f, xgeGamepadButtonDown(0, XGE_GAMEPAD_B) ? 20.0f : 12.0f, XGE_COLOR_RGBA(245, 128, 176, 255));
	xgeShapeCircleStrokePx(364.0f, 332.0f, 22.0f, 2.0f, XGE_COLOR_RGBA(255, 225, 236, 255));
}

static int InputStateFrame(void* pUser)
{
	input_state_lab_t* pLab;
	int iRet;

	pLab = (input_state_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "input-state-lab init failed: %d\n", iRet);
		xgeQuit();
		return 1;
	}
	UpdateLiveInput(pLab);
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("input-state-lab esc-summary frames=%d clipboard=%d touch=%d gamepad=%d\n", pLab->iFrameCount, pLab->bClipboardOK, pLab->bTouchFindOK, pLab->bGamepadStateOK);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(13, 17, 23, 255));
	DrawInputState(pLab);
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("input-state-lab final-summary frames=%d key(sampled down=%d pressed=%d released=%d) mouse=(%.1f,%.1f d=%.1f,%.1f wheel=%.1f,%.1f left=%d) clipboard=%d touch(count=%d get=%d find=%d) gamepad(conn=%d state=%d down=%d pressed=%d released=%d axis=%d axes=%.2f,%.2f)\n",
			pLab->iFrameCount,
			pLab->bKeyDown,
			pLab->bKeyPressed,
			pLab->bKeyReleased,
			pLab->fMouseX,
			pLab->fMouseY,
			pLab->fMouseDX,
			pLab->fMouseDY,
			pLab->fWheelX,
			pLab->fWheelY,
			pLab->bMouseLeft,
			pLab->bClipboardOK,
			pLab->iTouchCount,
			pLab->bTouchOK,
			pLab->bTouchFindOK,
			pLab->bGamepadConnected,
			pLab->bGamepadStateOK,
			pLab->bGamepadDownOK,
			pLab->bGamepadPressedOK,
			pLab->bGamepadReleasedOK,
			pLab->bGamepadAxisOK,
			pLab->tPadState.arrAxes[0],
			pLab->tPadState.arrAxes[1]);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	input_state_lab_t tLab;
	xge_desc_t tDesc;
	const char* sFontPath;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_INPUT_STATE_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_INPUT_STATE_SECONDS"), 0.0);
	sFontPath = FindFontPath(argc, argv);
	if ( sFontPath != NULL ) {
		snprintf(tLab.sFontPath, sizeof(tLab.sFontPath), "%s", sFontPath);
	}
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		} else if ( (strcmp(argv[i], "--font") == 0) && ((i + 1) < argc) ) {
			i++;
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE Input State Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(InputStateFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("input-state-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
