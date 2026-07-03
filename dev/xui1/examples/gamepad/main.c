#include "../../xge.h"
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct gamepad_demo_t {
	float fT;
} gamepad_demo_t;

static void __gamepadDrawButton(float fX, float fY, int bDown, uint32_t iColor)
{
	xgeShapeCircleFillPx(fX, fY, bDown ? 18.0f : 12.0f, bDown ? iColor : XGE_COLOR_RGBA(60, 68, 76, 255));
	xgeShapeCircleStrokePx(fX, fY, 20.0f, 2.0f, XGE_COLOR_RGBA(180, 198, 210, 255));
}

static int GamepadFrame(void* pUser)
{
	gamepad_demo_t* pDemo;
	xge_gamepad_state_t tPad;
	float fX;
	float fY;
	float fLX;
	float fLY;
	xge_rect_t tRect;

	pDemo = (gamepad_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	pDemo->fT += xgeGetDelta();
	memset(&tPad, 0, sizeof(tPad));
	xgeGamepadGetState(0, &tPad);

	xgeClear(XGE_COLOR_RGBA(16, 20, 26, 255));
	tRect.fX = 48.0f;
	tRect.fY = 48.0f;
	tRect.fW = 544.0f;
	tRect.fH = 352.0f;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(30, 38, 48, 255));
	xgeShapeRectStrokePx(tRect, 3.0f, xgeGamepadConnected(0) ? XGE_COLOR_RGBA(96, 210, 150, 255) : XGE_COLOR_RGBA(180, 90, 90, 255));

	fLX = xgeGamepadAxis(0, 0);
	fLY = xgeGamepadAxis(0, 1);
	xgeShapeCircleStrokePx(184.0f, 224.0f, 58.0f, 3.0f, XGE_COLOR_RGBA(110, 130, 150, 255));
	xgeShapeCircleFillPx(184.0f + fLX * 42.0f, 224.0f + fLY * 42.0f, 16.0f, XGE_COLOR_RGBA(120, 190, 255, 255));

	__gamepadDrawButton(430.0f, 210.0f, xgeGamepadButtonDown(0, XGE_GAMEPAD_X), XGE_COLOR_RGBA(120, 190, 255, 255));
	__gamepadDrawButton(480.0f, 210.0f, xgeGamepadButtonDown(0, XGE_GAMEPAD_B), XGE_COLOR_RGBA(255, 120, 130, 255));
	__gamepadDrawButton(455.0f, 185.0f, xgeGamepadButtonDown(0, XGE_GAMEPAD_Y), XGE_COLOR_RGBA(255, 220, 120, 255));
	__gamepadDrawButton(455.0f, 235.0f, xgeGamepadButtonDown(0, XGE_GAMEPAD_A), XGE_COLOR_RGBA(120, 225, 150, 255));

	fX = 96.0f + (xgeGamepadConnected(0) ? 0.0f : (float)((int)(pDemo->fT * 4.0f) & 1) * 12.0f);
	fY = 92.0f;
	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = xgeGamepadConnected(0) ? 140.0f : 72.0f;
	tRect.fH = 18.0f;
	xgeShapeRectFillPx(tRect, xgeGamepadConnected(0) ? XGE_COLOR_RGBA(96, 210, 150, 255) : XGE_COLOR_RGBA(180, 90, 90, 255));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	gamepad_demo_t tDemo;

	(void)argc;
	(void)argv;
	memset(&tDemo, 0, sizeof(tDemo));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 448;
	tDesc.sTitle = "XGE Gamepad";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(GamepadFrame, &tDemo);
	xgeUnit();
	return 0;
}
