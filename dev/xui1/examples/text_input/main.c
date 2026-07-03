#include "../../xge.h"
#include <stdio.h>
#include <string.h>

#define XGE_EXAMPLE_KEY_ESCAPE	256

typedef struct text_input_demo_t {
	xge_xui_context_t tXui;
	xge_xui_widget pInputWidget;
	xge_xui_input_t tInput;
	xge_font_t tFont;
	int bFontReady;
	int bMouseDown;
	int bCompositionDemo;
} text_input_demo_t;

static void DispatchPointer(text_input_demo_t* pDemo)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	float fDX;
	float fDY;
	int bDown;

	memset(&tEvent, 0, sizeof(tEvent));
	xgeMouseGet(&fX, &fY);
	xgeMouseGetDelta(&fDX, &fDY);
	bDown = xgeMouseDown(XGE_MOUSE_LEFT);
	if ( (fDX != 0.0f) || (fDY != 0.0f) ) {
		tEvent.iType = XGE_EVENT_MOUSE_MOVE;
		tEvent.fX = fX;
		tEvent.fY = fY;
		tEvent.fDX = fDX;
		tEvent.fDY = fDY;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
	}
	if ( (bDown != 0) && (pDemo->bMouseDown == 0) ) {
		tEvent.iType = XGE_EVENT_MOUSE_DOWN;
		tEvent.iParam1 = XGE_MOUSE_LEFT;
		tEvent.fX = fX;
		tEvent.fY = fY;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
	} else if ( (bDown == 0) && (pDemo->bMouseDown != 0) ) {
		tEvent.iType = XGE_EVENT_MOUSE_UP;
		tEvent.iParam1 = XGE_MOUSE_LEFT;
		tEvent.fX = fX;
		tEvent.fY = fY;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
	}
	pDemo->bMouseDown = bDown;
}

static void DispatchKey(text_input_demo_t* pDemo, int iKey)
{
	xge_event_t tEvent;

	if ( xgeKeyPressed(iKey) == 0 ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = iKey;
	xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
}

static void DispatchTextAndIme(text_input_demo_t* pDemo)
{
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	uint32_t iCodepoint;

	memset(&tEvent, 0, sizeof(tEvent));
	iCodepoint = xgeTextGet();
	if ( iCodepoint != 0 ) {
		pDemo->bCompositionDemo = 0;
		tEvent.iType = XGE_EVENT_TEXT;
		tEvent.iCodepoint = iCodepoint;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
	}

	if ( xgeKeyPressed(XGE_KEY_TAB) ) {
		memset(&tIme, 0, sizeof(tIme));
		tIme.sText = "pin yin";
		tEvent.iType = XGE_EVENT_IME_UPDATE;
		tEvent.pData = &tIme;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
		pDemo->bCompositionDemo = 1;
	}
	if ( xgeKeyPressed(XGE_KEY_ENTER) && (pDemo->bCompositionDemo != 0) ) {
		tEvent.iType = XGE_EVENT_IME_END;
		tEvent.pData = NULL;
		xgeXuiDispatchEvent(&pDemo->tXui, &tEvent);
		pDemo->bCompositionDemo = 0;
	}
}

static int TextInputFrame(void* pUser)
{
	text_input_demo_t* pDemo;
	xge_rect_t tCandidate;

	pDemo = (text_input_demo_t*)pUser;
	if ( xgeKeyDown(XGE_EXAMPLE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}

	DispatchPointer(pDemo);
	DispatchKey(pDemo, XGE_KEY_BACKSPACE);
	DispatchKey(pDemo, XGE_KEY_DELETE);
	DispatchKey(pDemo, XGE_KEY_LEFT);
	DispatchKey(pDemo, XGE_KEY_RIGHT);
	DispatchTextAndIme(pDemo);

	xgeClear(XGE_COLOR_RGBA(18, 24, 30, 255));
	xgeXuiUpdate(&pDemo->tXui, xgeGetDelta());
	xgeXuiPaint(&pDemo->tXui);
	tCandidate = xgeXuiInputGetCandidateRect(&pDemo->tInput);
	xgeShapeRectStrokePx(tCandidate, 2.0f, XGE_COLOR_RGBA(255, 220, 96, 255));
	xgeShapeLinePx(tCandidate.fX, tCandidate.fY + tCandidate.fH + 6.0f, tCandidate.fX + 80.0f, tCandidate.fY + tCandidate.fH + 6.0f, 3.0f, XGE_COLOR_RGBA(255, 220, 96, 220));
	return 0;
}

static int TextInputDemoInit(text_input_demo_t* pDemo, const char* sFont)
{
	xge_rect_t tRect;
	xge_xui_widget pRoot;

	memset(pDemo, 0, sizeof(*pDemo));
	if ( xgeXuiInit(&pDemo->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	pRoot = xgeXuiRoot(&pDemo->tXui);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	pDemo->pInputWidget = xgeXuiWidgetCreate();
	if ( pDemo->pInputWidget == NULL ) {
		xgeXuiUnit(&pDemo->tXui);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	tRect.fX = 56.0f;
	tRect.fY = 96.0f;
	tRect.fW = 520.0f;
	tRect.fH = 48.0f;
	xgeXuiWidgetSetRect(pDemo->pInputWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pDemo->pInputWidget, 12.0f, 6.0f, 12.0f, 6.0f);
	xgeXuiWidgetAdd(pRoot, pDemo->pInputWidget);
	if ( xgeFontLoad(&pDemo->tFont, sFont, 24.0f) == XGE_OK ) {
		pDemo->bFontReady = 1;
	}
	if ( xgeXuiInputInit(&pDemo->tInput, &pDemo->tXui, pDemo->pInputWidget, pDemo->bFontReady ? &pDemo->tFont : NULL) != XGE_OK ) {
		xgeXuiUnit(&pDemo->tXui);
		if ( pDemo->bFontReady ) {
			xgeFontFree(&pDemo->tFont);
		}
		return XGE_ERROR;
	}
	xgeXuiInputSetText(&pDemo->tInput, "XGE input: ");
	xgeXuiInputSetColors(&pDemo->tInput, XGE_COLOR_RGBA(245, 248, 250, 255), XGE_COLOR_RGBA(42, 96, 160, 255), XGE_COLOR_RGBA(52, 116, 190, 255), XGE_COLOR_RGBA(255, 240, 180, 255));
	xgeXuiSetFocus(&pDemo->tXui, pDemo->pInputWidget);
	return XGE_OK;
}

static void TextInputDemoUnit(text_input_demo_t* pDemo)
{
	xgeXuiInputUnit(&pDemo->tInput);
	if ( pDemo->bFontReady ) {
		xgeFontFree(&pDemo->tFont);
	}
	xgeXuiUnit(&pDemo->tXui);
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	text_input_demo_t tDemo;
	const char* sFont;

	sFont = "C:/Windows/Fonts/msyh.ttc";
	if ( argc >= 2 ) {
		sFont = argv[1];
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE Text Input";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( TextInputDemoInit(&tDemo, sFont) != XGE_OK ) {
		printf("text input demo init failed: %s\n", sFont);
		xgeUnit();
		return 2;
	}
	xgeRun(TextInputFrame, &tDemo);
	TextInputDemoUnit(&tDemo);
	xgeUnit();
	return 0;
}
