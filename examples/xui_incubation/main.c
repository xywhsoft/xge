#include "../../xge.h"
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pPanelWidget;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pProgressWidget;
	xge_xui_widget pScrollWidget;
	xge_xui_panel_t tPanel;
	xge_xui_button_t tButton;
	xge_xui_progress_t tProgress;
	xge_xui_scroll_view_t tScroll;
	float fProgress;
} app_state_t;

static int AppCreateWidgetTree(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->pButtonWidget = xgeXuiWidgetCreate();
	pApp->pProgressWidget = xgeXuiWidgetCreate();
	pApp->pScrollWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pApp->pPanelWidget == NULL) || (pApp->pButtonWidget == NULL) || (pApp->pProgressWidget == NULL) || (pApp->pScrollWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 32.0f, 32.0f, 360.0f, 220.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanelWidget, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tPanel, NULL, "XUI Incubation");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(34, 42, 52, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(238, 242, 246, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pPanelWidget);

	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 56.0f, 72.0f, 160.0f, 44.0f });
	xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget);
	xgeXuiButtonSetText(&pApp->tButton, NULL, "Button");
	xgeXuiButtonSetColors(&pApp->tButton, XGE_COLOR_RGBA(76, 132, 196, 255), XGE_COLOR_RGBA(92, 152, 216, 255), XGE_COLOR_RGBA(52, 96, 152, 255), XGE_COLOR_RGBA(255, 210, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pButtonWidget);

	xgeXuiWidgetSetRect(pApp->pProgressWidget, (xge_rect_t){ 56.0f, 136.0f, 280.0f, 28.0f });
	xgeXuiProgressInit(&pApp->tProgress, pApp->pProgressWidget);
	xgeXuiProgressSetRange(&pApp->tProgress, 0.0f, 1.0f);
	xgeXuiProgressSetColors(&pApp->tProgress, XGE_COLOR_RGBA(58, 68, 80, 255), XGE_COLOR_RGBA(84, 190, 132, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pProgressWidget);

	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 424.0f, 32.0f, 180.0f, 220.0f });
	xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScrollWidget);
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 260.0f, 420.0f);
	xgeXuiScrollViewSetColors(&pApp->tScroll, XGE_COLOR_RGBA(28, 34, 42, 255), XGE_COLOR_RGBA(70, 82, 96, 190), XGE_COLOR_RGBA(170, 184, 202, 230));
	xgeXuiWidgetAdd(pRoot, pApp->pScrollWidget);

	return XGE_OK;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	if ( xgeKeyDown(256) ) {
		xgeQuit();
		return 1;
	}

	pApp->fProgress += xgeGetDelta() * 0.25f;
	if ( pApp->fProgress > 1.0f ) {
		pApp->fProgress = 0.0f;
	}
	xgeXuiProgressSetValue(&pApp->tProgress, pApp->fProgress);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, pApp->fProgress * 180.0f);
	xgeXuiWidgetMarkPaint(xgeXuiRoot(&pApp->tXui));

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return 0;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE XUI Incubation";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	if ( AppCreateWidgetTree(&tApp) != XGE_OK ) {
		xgeXuiUnit(&tApp.tXui);
		xgeUnit();
		return 3;
	}
	xgeRun(AppFrame, &tApp);
	xgeXuiPanelUnit(&tApp.tPanel);
	xgeXuiButtonUnit(&tApp.tButton);
	xgeXuiProgressUnit(&tApp.tProgress);
	xgeXuiScrollViewUnit(&tApp.tScroll);
	xgeXuiUnit(&tApp.tXui);
	xgeUnit();
	return 0;
}
