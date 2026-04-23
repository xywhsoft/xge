#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct bridge_demo_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pInputWidget;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatusLabel;
	xge_xui_button_t tButton;
	xge_xui_input_t tInput;
	int bFontReady;
	int iClickCount;
	int iGameEventCount;
} bridge_demo_t;

static void BridgeButtonClick(xge_xui_widget pWidget, void* pUser)
{
	bridge_demo_t* pDemo;

	(void)pWidget;
	pDemo = (bridge_demo_t*)pUser;
	pDemo->iClickCount++;
}

static int BridgeLoadFont(bridge_demo_t* pDemo)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/NotoSansSC-VF.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pDemo->tFont, 0, sizeof(pDemo->tFont));
		if ( xgeFontLoad(&pDemo->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pDemo->bFontReady = 1;
			printf("xui bridge font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui bridge font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int BridgeCreateUI(bridge_demo_t* pDemo)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pDemo->tXui);
	pFont = pDemo->bFontReady ? &pDemo->tFont : NULL;
	pDemo->pRootPanel = xgeXuiWidgetCreate();
	pDemo->pStatusWidget = xgeXuiWidgetCreate();
	pDemo->pButtonWidget = xgeXuiWidgetCreate();
	pDemo->pInputWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pDemo->pRootPanel == NULL) || (pDemo->pStatusWidget == NULL) || (pDemo->pButtonWidget == NULL) || (pDemo->pInputWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pDemo->pRootPanel, (xge_rect_t){ 40.0f, 36.0f, 360.0f, 230.0f });
	xgeXuiWidgetSetPaddingPx(pDemo->pRootPanel, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiPanelInit(&pDemo->tPanel, pDemo->pRootPanel);
	xgeXuiPanelSetTitle(&pDemo->tPanel, pFont, "XUI Bridge");
	xgeXuiPanelSetBackground(&pDemo->tPanel, XGE_COLOR_RGBA(34, 42, 54, 245));
	xgeXuiPanelSetTitleColor(&pDemo->tPanel, XGE_COLOR_RGBA(240, 245, 250, 255));
	xgeXuiWidgetAdd(pRoot, pDemo->pRootPanel);

	xgeXuiWidgetSetRect(pDemo->pStatusWidget, (xge_rect_t){ 64.0f, 76.0f, 300.0f, 34.0f });
	xgeXuiLabelInit(&pDemo->tStatusLabel, pDemo->pStatusWidget, pFont, "clicks=0 game-events=0");
	xgeXuiLabelSetColor(&pDemo->tStatusLabel, XGE_COLOR_RGBA(220, 232, 244, 255));
	xgeXuiWidgetAdd(pDemo->pRootPanel, pDemo->pStatusWidget);

	xgeXuiWidgetSetRect(pDemo->pButtonWidget, (xge_rect_t){ 64.0f, 122.0f, 160.0f, 42.0f });
	xgeXuiButtonInit(&pDemo->tButton, &pDemo->tXui, pDemo->pButtonWidget);
	xgeXuiButtonSetText(&pDemo->tButton, pFont, "Click UI");
	xgeXuiButtonSetTextColor(&pDemo->tButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetColors(&pDemo->tButton, XGE_COLOR_RGBA(42, 128, 232, 255), XGE_COLOR_RGBA(66, 154, 255, 255), XGE_COLOR_RGBA(24, 86, 168, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiButtonSetClick(&pDemo->tButton, BridgeButtonClick, pDemo);
	xgeXuiWidgetAdd(pDemo->pRootPanel, pDemo->pButtonWidget);

	xgeXuiWidgetSetRect(pDemo->pInputWidget, (xge_rect_t){ 64.0f, 184.0f, 280.0f, 38.0f });
	xgeXuiInputInit(&pDemo->tInput, &pDemo->tXui, pDemo->pInputWidget, pFont);
	xgeXuiInputSetText(&pDemo->tInput, "system IME text path");
	xgeXuiInputSetColors(&pDemo->tInput, XGE_COLOR_RGBA(235, 240, 248, 255), XGE_COLOR_RGBA(38, 48, 62, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(72, 214, 128, 255));
	xgeXuiWidgetAdd(pDemo->pRootPanel, pDemo->pInputWidget);
	return XGE_OK;
}

static int BridgeSceneEnter(xge_scene pScene)
{
	bridge_demo_t* pDemo;

	pDemo = (bridge_demo_t*)pScene->pUser;
	if ( xgeXuiInit(&pDemo->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)BridgeLoadFont(pDemo);
	return BridgeCreateUI(pDemo);
}

static int BridgeSceneLeave(xge_scene pScene)
{
	bridge_demo_t* pDemo;

	pDemo = (bridge_demo_t*)pScene->pUser;
	xgeXuiInputUnit(&pDemo->tInput);
	xgeXuiButtonUnit(&pDemo->tButton);
	xgeXuiLabelUnit(&pDemo->tStatusLabel);
	xgeXuiPanelUnit(&pDemo->tPanel);
	xgeXuiUnit(&pDemo->tXui);
	if ( pDemo->bFontReady != 0 ) {
		xgeFontFree(&pDemo->tFont);
		pDemo->bFontReady = 0;
	}
	return XGE_OK;
}

static int BridgeSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	bridge_demo_t* pDemo;
	int iRet;

	pDemo = (bridge_demo_t*)pScene->pUser;
	iRet = xgeXuiDispatchEvent(&pDemo->tXui, pEvent);
	if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	pDemo->iGameEventCount++;
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == 256) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int BridgeSceneUpdate(xge_scene pScene, float fDelta)
{
	bridge_demo_t* pDemo;

	pDemo = (bridge_demo_t*)pScene->pUser;
	return xgeXuiUpdate(&pDemo->tXui, fDelta);
}

static int BridgeSceneDraw(xge_scene pScene)
{
	bridge_demo_t* pDemo;
	float fBar;
	char sStatus[128];

	pDemo = (bridge_demo_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(17, 21, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 430.0f, 48.0f, 124.0f, 32.0f }, XGE_COLOR_RGBA(44, 54, 68, 255));
	fBar = (float)(pDemo->iClickCount % 10) * 12.0f;
	xgeShapeRectFillPx((xge_rect_t){ 432.0f, 50.0f, fBar, 28.0f }, XGE_COLOR_RGBA(96, 190, 132, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 430.0f, 48.0f, 124.0f, 32.0f }, 2.0f, XGE_COLOR_RGBA(210, 220, 232, 255));
	snprintf(sStatus, sizeof(sStatus), "clicks=%d game-events=%d", pDemo->iClickCount, pDemo->iGameEventCount);
	xgeXuiLabelSetText(&pDemo->tStatusLabel, sStatus);
	xgeXuiWidgetMarkPaint(xgeXuiRoot(&pDemo->tXui));
	xgeXuiPaint(&pDemo->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void BridgeDemoInit(bridge_demo_t* pDemo)
{
	memset(pDemo, 0, sizeof(*pDemo));
	pDemo->tScene.pUser = pDemo;
	pDemo->tScene.onEnter = BridgeSceneEnter;
	pDemo->tScene.onLeave = BridgeSceneLeave;
	pDemo->tScene.onEvent = BridgeSceneEvent;
	pDemo->tScene.onUpdate = BridgeSceneUpdate;
	pDemo->tScene.onDraw = BridgeSceneDraw;
}

int main(int argc, char** argv)
{
	bridge_demo_t tDemo;
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	BridgeDemoInit(&tDemo);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE XUI Bridge";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeSceneSet(&tDemo.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	printf("xui clicks=%d game-events=%d\n", tDemo.iClickCount, tDemo.iGameEventCount);
	xgeUnit();
	return 0;
}
