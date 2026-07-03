#include "../../xge.h"

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct touch_demo_t {
	float fMouseX;
	float fMouseY;
	int bMouseDown;
} touch_demo_t;

static uint32_t TouchColor(int iIndex)
{
	static const uint32_t arrColors[XGE_TOUCH_MAX] = {
		0xFF7896FFu,
		0x78C8FFFFu,
		0x78E696FFu,
		0xFFE070FFu,
		0xFF7896FFu,
		0xC896FFFFu,
		0x70E0D0FFu,
		0xF0F0F0FFu
	};

	if ( (iIndex < 0) || (iIndex >= XGE_TOUCH_MAX) ) {
		return XGE_COLOR_RGBA(255, 255, 255, 255);
	}
	return arrColors[iIndex];
}

static void DrawTouchPoint(const xge_touch_point_t* pPoint, int iIndex)
{
	float fRadius;
	uint32_t iColor;

	iColor = TouchColor(iIndex);
	fRadius = pPoint->bChanged ? 34.0f : 24.0f;
	if ( pPoint->iPhase == XGE_TOUCH_BEGIN ) {
		fRadius = 40.0f;
	} else if ( (pPoint->iPhase == XGE_TOUCH_END) || (pPoint->iPhase == XGE_TOUCH_CANCEL) ) {
		fRadius = 18.0f;
	}
	xgeShapeCircleFillPx(pPoint->fX, pPoint->fY, fRadius, iColor);
	xgeShapeCircleStrokePx(pPoint->fX, pPoint->fY, fRadius + 8.0f, 3.0f, XGE_COLOR_RGBA(245, 248, 250, 220));
	xgeShapeLinePx(pPoint->fX, pPoint->fY, pPoint->fX - (pPoint->fDX * 8.0f), pPoint->fY - (pPoint->fDY * 8.0f), 4.0f, XGE_COLOR_RGBA(255, 255, 255, 210));
}

static void DrawMouseFallback(touch_demo_t* pDemo)
{
	float fX;
	float fY;
	float fDX;
	float fDY;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetDelta(&fDX, &fDY);
	pDemo->fMouseX = fX;
	pDemo->fMouseY = fY;
	pDemo->bMouseDown = xgeMouseDown(XGE_MOUSE_LEFT);
	if ( pDemo->bMouseDown == 0 ) {
		xgeShapeCircleStrokePx(fX, fY, 20.0f, 2.0f, XGE_COLOR_RGBA(140, 170, 190, 180));
		return;
	}
	xgeShapeCircleFillPx(fX, fY, 28.0f, XGE_COLOR_RGBA(255, 120, 150, 255));
	xgeShapeCircleStrokePx(fX, fY, 38.0f, 4.0f, XGE_COLOR_RGBA(255, 220, 235, 230));
	xgeShapeLinePx(fX, fY, fX - (fDX * 8.0f), fY - (fDY * 8.0f), 4.0f, XGE_COLOR_RGBA(255, 255, 255, 210));
}

static int TouchFrame(void* pUser)
{
	touch_demo_t* pDemo;
	xge_touch_point_t tPoint;
	xge_rect_t tBar;
	int i;
	int iCount;

	pDemo = (touch_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}

	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));
	xgeShapeLinePx(32.0f, 240.0f, 608.0f, 240.0f, 2.0f, XGE_COLOR_RGBA(80, 120, 150, 255));
	xgeShapeLinePx(320.0f, 32.0f, 320.0f, 448.0f, 2.0f, XGE_COLOR_RGBA(80, 120, 150, 255));

	iCount = xgeTouchGetCount();
	tBar.fX = 40.0f;
	tBar.fY = 40.0f;
	tBar.fW = 24.0f + (float)iCount * 48.0f;
	tBar.fH = 28.0f;
	xgeShapeRectFillPx(tBar, XGE_COLOR_RGBA(42, 96, 160, 255));
	xgeShapeRectStrokePx(tBar, 2.0f, XGE_COLOR_RGBA(180, 225, 255, 255));

	for ( i = 0; i < iCount; i++ ) {
		if ( xgeTouchGet(i, &tPoint) == XGE_OK ) {
			DrawTouchPoint(&tPoint, i);
		}
	}
	if ( iCount == 0 ) {
		DrawMouseFallback(pDemo);
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	touch_demo_t tDemo;

	(void)argc;
	(void)argv;
	tDemo.fMouseX = 0.0f;
	tDemo.fMouseY = 0.0f;
	tDemo.bMouseDown = 0;
	tDesc.iWidth = 640;
	tDesc.iHeight = 480;
	tDesc.sTitle = "XGE Touch";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	tDesc.pNativeWindow = 0;
	tDesc.pUser = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(TouchFrame, &tDemo);
	xgeUnit();
	return 0;
}
