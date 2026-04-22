#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct bridge_demo_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_widget pRootPanel;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pInputWidget;
	xge_xui_panel_t tPanel;
	xge_xui_button_t tButton;
	xge_xui_input_t tInput;
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

static int BridgeCreateUI(bridge_demo_t* pDemo)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pDemo->tXui);
	pDemo->pRootPanel = xgeXuiWidgetCreate();
	pDemo->pButtonWidget = xgeXuiWidgetCreate();
	pDemo->pInputWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pDemo->pRootPanel == NULL) || (pDemo->pButtonWidget == NULL) || (pDemo->pInputWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pDemo->pRootPanel, (xge_rect_t){ 40.0f, 36.0f, 360.0f, 210.0f });
	xgeXuiWidgetSetPaddingPx(pDemo->pRootPanel, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiPanelInit(&pDemo->tPanel, pDemo->pRootPanel);
	xgeXuiPanelSetTitle(&pDemo->tPanel, NULL, "XUI Bridge");
	xgeXuiPanelSetBackground(&pDemo->tPanel, XGE_COLOR_RGBA(34, 42, 54, 245));
	xgeXuiPanelSetTitleColor(&pDemo->tPanel, XGE_COLOR_RGBA(240, 245, 250, 255));
	xgeXuiWidgetAdd(pRoot, pDemo->pRootPanel);

	xgeXuiWidgetSetRect(pDemo->pButtonWidget, (xge_rect_t){ 64.0f, 86.0f, 160.0f, 42.0f });
	xgeXuiButtonInit(&pDemo->tButton, &pDemo->tXui, pDemo->pButtonWidget);
	xgeXuiButtonSetText(&pDemo->tButton, NULL, "Click UI");
	xgeXuiButtonSetClick(&pDemo->tButton, BridgeButtonClick, pDemo);
	xgeXuiWidgetAdd(pDemo->pRootPanel, pDemo->pButtonWidget);

	xgeXuiWidgetSetRect(pDemo->pInputWidget, (xge_rect_t){ 64.0f, 148.0f, 260.0f, 38.0f });
	xgeXuiInputInit(&pDemo->tInput, &pDemo->tXui, pDemo->pInputWidget, NULL);
	xgeXuiInputSetText(&pDemo->tInput, "system IME text path");
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
	return BridgeCreateUI(pDemo);
}

static int BridgeSceneLeave(xge_scene pScene)
{
	bridge_demo_t* pDemo;

	pDemo = (bridge_demo_t*)pScene->pUser;
	xgeXuiInputUnit(&pDemo->tInput);
	xgeXuiButtonUnit(&pDemo->tButton);
	xgeXuiPanelUnit(&pDemo->tPanel);
	xgeXuiUnit(&pDemo->tXui);
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

	pDemo = (bridge_demo_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(17, 21, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 430.0f, 48.0f, 124.0f, 32.0f }, XGE_COLOR_RGBA(44, 54, 68, 255));
	fBar = (float)(pDemo->iClickCount % 10) * 12.0f;
	xgeShapeRectFillPx((xge_rect_t){ 432.0f, 50.0f, fBar, 28.0f }, XGE_COLOR_RGBA(96, 190, 132, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 430.0f, 48.0f, 124.0f, 32.0f }, 2.0f, XGE_COLOR_RGBA(210, 220, 232, 255));
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
