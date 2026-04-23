#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct viewport_state_t {
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bPrintedFirst;
} viewport_state_t;

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

static xge_rect_t Rect(float fX, float fY, float fW, float fH)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	return tRect;
}

static xge_vec2_t Vec2(float fX, float fY)
{
	xge_vec2_t tPoint;

	tPoint.fX = fX;
	tPoint.fY = fY;
	return tPoint;
}

static void PrintRect(const char* sName, xge_rect_t tRect)
{
	printf("%s=(%.1f,%.1f %.1fx%.1f) ", sName, tRect.fX, tRect.fY, tRect.fW, tRect.fH);
}

static void PrintCameraSummary(const char* sPrefix, xge_vec2_t tWorld)
{
	xge_camera_t tCamera;
	xge_vec2_t tScreen;
	xge_vec2_t tRoundTrip;
	xge_rect_t tViewport;
	xge_rect_t tClip;

	tCamera = xgeCameraGet();
	tViewport = xgeViewportGet();
	tClip = xgeClipGet();
	tScreen = xgeWorldToScreen(tWorld);
	tRoundTrip = xgeScreenToWorld(tScreen);
	printf("%s frame=%d size=%dx%d camera_pos=(%.1f,%.1f) scale=(%.2f,%.2f) world=(%.1f,%.1f) screen=(%.1f,%.1f) roundtrip=(%.1f,%.1f) ",
		sPrefix,
		(int)xgeFrameStatsGet().iFrameCount,
		xgeGetWidth(),
		xgeGetHeight(),
		tCamera.tPosition.fX,
		tCamera.tPosition.fY,
		tCamera.tScale.fX,
		tCamera.tScale.fY,
		tWorld.fX,
		tWorld.fY,
		tScreen.fX,
		tScreen.fY,
		tRoundTrip.fX,
		tRoundTrip.fY);
	PrintRect("viewport", tViewport);
	PrintRect("clip", tClip);
	printf("\n");
}

static void DrawViewportFrame(xge_rect_t tRect, uint32_t iFill, uint32_t iStroke)
{
	xgeShapeRectFillPx(tRect, iFill);
	xgeShapeRectStrokePx(tRect, 2.0f, iStroke);
}

static void DrawWorldGrid(float fPhase)
{
	int i;

	for ( i = -4; i <= 4; i++ ) {
		uint32_t iColor;

		iColor = (i == 0) ? XGE_COLOR_RGBA(240, 240, 245, 230) : XGE_COLOR_RGBA(130, 160, 190, 130);
		xgeShapeLine((float)i * 40.0f, -160.0f, (float)i * 40.0f, 160.0f, (i == 0) ? 3.0f : 1.5f, iColor);
		xgeShapeLine(-180.0f, (float)i * 40.0f, 180.0f, (float)i * 40.0f, (i == 0) ? 3.0f : 1.5f, iColor);
	}
	xgeShapeCircleFill(0.0f, 0.0f, 18.0f + fPhase, XGE_COLOR_RGBA(255, 196, 72, 230));
	xgeShapeRectStroke(Rect(-84.0f, -52.0f, 168.0f, 104.0f), 3.0f, XGE_COLOR_RGBA(84, 180, 240, 240));
	xgeShapeLine(-120.0f, -92.0f, 120.0f, 92.0f, 4.0f, XGE_COLOR_RGBA(244, 116, 100, 230));
}

static void DrawClippedScene(xge_rect_t tViewport, xge_rect_t tClip, float fPhase)
{
	int i;

	xgeViewportSet(tViewport);
	xgeClipSet(tClip);
	xgeShapeRectFillPx(Rect(tViewport.fX - 70.0f + fPhase, tViewport.fY + 34.0f, tViewport.fW + 140.0f, 58.0f), XGE_COLOR_RGBA(250, 106, 86, 220));
	xgeShapeCircleFillPx(tViewport.fX + tViewport.fW * 0.5f, tViewport.fY + tViewport.fH * 0.5f, 120.0f, XGE_COLOR_RGBA(90, 190, 120, 180));
	for ( i = 0; i < 8; i++ ) {
		float fX;

		fX = tViewport.fX - 40.0f + ((float)i * 70.0f);
		xgeShapeLinePx(fX, tViewport.fY - 20.0f, fX + 48.0f, tViewport.fY + tViewport.fH + 20.0f, 5.0f, XGE_COLOR_RGBA(245, 220, 96, 210));
	}
	xgeClipClear();
	xgeViewportClear();
	xgeShapeRectStrokePx(tClip, 3.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
}

static int ViewportFrame(void* pUser)
{
	viewport_state_t* pState;
	xge_rect_t tLeft;
	xge_rect_t tRight;
	xge_rect_t tClip;
	xge_camera_t tDefault;
	xge_camera_t tCamera;
	xge_camera_t tPerspective;
	xge_vec2_t tProbe;
	float fPhase;
	int iWidth;
	int iHeight;

	pState = (viewport_state_t*)pUser;
	pState->iFrameCount++;
	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	fPhase = (float)(pState->iFrameCount % 90) / 90.0f;
	tLeft = Rect(24.0f, 30.0f, ((float)iWidth * 0.5f) - 42.0f, (float)iHeight - 68.0f);
	tRight = Rect(((float)iWidth * 0.5f) + 18.0f, 30.0f, ((float)iWidth * 0.5f) - 42.0f, (float)iHeight - 68.0f);
	tClip = Rect(tRight.fX + 42.0f, tRight.fY + 46.0f, tRight.fW - 84.0f, tRight.fH - 92.0f);
	tProbe = Vec2(72.0f, -48.0f);
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintCameraSummary("viewport-clip-camera esc-summary", tProbe);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	DrawViewportFrame(tLeft, XGE_COLOR_RGBA(32, 44, 64, 255), XGE_COLOR_RGBA(94, 150, 220, 230));
	DrawViewportFrame(tRight, XGE_COLOR_RGBA(35, 46, 48, 255), XGE_COLOR_RGBA(100, 210, 160, 230));

	xgeViewportSet(tLeft);
	tCamera = xgeCameraDefault(tLeft.fW, tLeft.fH);
	tCamera.iCoordinateMode = XGE_COORD_CENTER;
	tCamera.tViewport = tLeft;
	tCamera.tPosition.fX = 20.0f + (fPhase * 40.0f);
	tCamera.tPosition.fY = -10.0f;
	tCamera.tScale.fX = 1.2f;
	tCamera.tScale.fY = 1.2f;
	xgeCameraSet(&tCamera);
	DrawWorldGrid(fPhase * 18.0f);
	tPerspective = xgeCameraPerspective(tLeft.fW, tLeft.fH, 55.0f, 0.1f, 500.0f);
	tPerspective.iCoordinateMode = XGE_COORD_CENTER;
	tPerspective.tViewport = Rect(tLeft.fX + tLeft.fW - 118.0f, tLeft.fY + 22.0f, 96.0f, 80.0f);
	xgeCameraSet(&tPerspective);
	xgeShapeRectStroke(Rect(-32.0f, -24.0f, 64.0f, 48.0f), 2.0f, XGE_COLOR_RGBA(255, 255, 255, 210));
	xgeViewportClear();

	tDefault = xgeCameraDefault((float)iWidth, (float)iHeight);
	xgeCameraSet(&tDefault);
	DrawClippedScene(tRight, tClip, fPhase * 96.0f);
	if ( (pState->bPrintedFirst == 0) || ((pState->iFrameCount % 60) == 0) ) {
		xgeCameraSet(&tCamera);
		xgeViewportSet(tLeft);
		xgeClipSet(tClip);
		PrintCameraSummary((pState->bPrintedFirst == 0) ? "viewport-clip-camera first" : "viewport-clip-camera frame", tProbe);
		xgeClipClear();
		xgeViewportClear();
		xgeCameraSet(&tDefault);
		pState->bPrintedFirst = 1;
	}
	if ( ((pState->iFrameLimit > 0) && (pState->iFrameCount >= pState->iFrameLimit)) || ((pState->fSecondLimit > 0.0) && (xgeTimer() >= pState->fSecondLimit)) ) {
		xgeCameraSet(&tCamera);
		xgeViewportSet(tLeft);
		xgeClipSet(tClip);
		PrintCameraSummary("viewport-clip-camera final-summary", tProbe);
		xgeClipClear();
		xgeViewportClear();
		xgeCameraSet(&tDefault);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	viewport_state_t tState;
	xge_desc_t tDesc;
	int i;

	memset(&tState, 0, sizeof(tState));
	tState.iFrameLimit = ArgInt(getenv("XGE_VIEWPORT_FRAMES"), 180);
	tState.fSecondLimit = ArgDouble(getenv("XGE_VIEWPORT_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tState.iFrameLimit = ArgInt(argv[++i], tState.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tState.fSecondLimit = ArgDouble(argv[++i], tState.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 900;
	tDesc.iHeight = 520;
	tDesc.sTitle = "XGE Viewport Clip Camera";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeFrameStatsReset();
	xgeRun(ViewportFrame, &tState);
	xgeUnit();
	printf("viewport-clip-camera summary frames=%d\n", tState.iFrameCount);
	return 0;
}
