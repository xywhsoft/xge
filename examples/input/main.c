#include "../../xge.h"

#define XGE_EXAMPLE_KEY_ESCAPE	256
#define XGE_EXAMPLE_KEY_SPACE	32

typedef struct input_demo_t {
	float fPulse;
	float fWheelPulse;
	int iClickCount;
} input_demo_t;

static int InputFrame(void* pUser)
{
	input_demo_t* pDemo;
	xge_rect_t tRect;
	float fMouseX;
	float fMouseY;
	float fDX;
	float fDY;
	float fWheelX;
	float fWheelY;
	float fRadius;

	pDemo = (input_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}

	xgeMouseGet(&fMouseX, &fMouseY);
	xgeMouseGetDelta(&fDX, &fDY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	if ( xgeKeyPressed(XGE_EXAMPLE_KEY_SPACE) ) {
		pDemo->fPulse = 1.0f;
	}
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
		pDemo->iClickCount++;
		pDemo->fPulse = 1.0f;
	}
	if ( (fWheelX != 0.0f) || (fWheelY != 0.0f) ) {
		pDemo->fWheelPulse = 1.0f;
	}

	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));
	xgeShapeLinePx(32.0f, 240.0f, 608.0f, 240.0f, 2.0f, XGE_COLOR_RGBA(80, 120, 150, 255));
	xgeShapeLinePx(320.0f, 32.0f, 320.0f, 448.0f, 2.0f, XGE_COLOR_RGBA(80, 120, 150, 255));

	tRect.fX = 48.0f;
	tRect.fY = 52.0f;
	tRect.fW = 156.0f + (pDemo->fPulse * 52.0f);
	tRect.fH = 56.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(42, 96, 160, 255));
	xgeShapeRectStrokePx(tRect, 3.0f, XGE_COLOR_RGBA(180, 225, 255, 255));

	tRect.fX = 48.0f;
	tRect.fY = 136.0f;
	tRect.fW = 120.0f + ((float)(pDemo->iClickCount % 8) * 18.0f);
	tRect.fH = 44.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(96, 170, 120, 255));

	fRadius = 16.0f + (pDemo->fWheelPulse * 26.0f);
	xgeShapeCircleFillPx(500.0f, 104.0f, fRadius, XGE_COLOR_RGBA(255, 210, 96, 255));
	xgeShapeCircleStrokePx(500.0f, 104.0f, fRadius + 8.0f, 3.0f, XGE_COLOR_RGBA(255, 240, 180, 255));

	xgeShapeLinePx(fMouseX - 16.0f, fMouseY, fMouseX + 16.0f, fMouseY, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	xgeShapeLinePx(fMouseX, fMouseY - 16.0f, fMouseX, fMouseY + 16.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	xgeShapeCircleStrokePx(fMouseX, fMouseY, 24.0f, xgeMouseDown(XGE_MOUSE_LEFT) ? 6.0f : 3.0f, XGE_COLOR_RGBA(255, 120, 150, 255));
	xgeShapeLinePx(fMouseX, fMouseY, fMouseX + (fDX * 6.0f), fMouseY + (fDY * 6.0f), 4.0f, XGE_COLOR_RGBA(120, 200, 255, 255));

	if ( pDemo->fPulse > 0.0f ) {
		pDemo->fPulse -= xgeGetDelta() * 2.5f;
		if ( pDemo->fPulse < 0.0f ) {
			pDemo->fPulse = 0.0f;
		}
	}
	if ( pDemo->fWheelPulse > 0.0f ) {
		pDemo->fWheelPulse -= xgeGetDelta() * 3.0f;
		if ( pDemo->fWheelPulse < 0.0f ) {
			pDemo->fWheelPulse = 0.0f;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	input_demo_t tDemo;

	(void)argc;
	(void)argv;
	tDemo.fPulse = 0.0f;
	tDemo.fWheelPulse = 0.0f;
	tDemo.iClickCount = 0;
	tDesc.iWidth = 640;
	tDesc.iHeight = 480;
	tDesc.sTitle = "XGE Input";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	tDesc.pNativeWindow = 0;
	tDesc.pUser = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(InputFrame, &tDemo);
	xgeUnit();
	return 0;
}
