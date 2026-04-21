#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_host_t tHost;
	xge_xui_widget pPanel;
	int iRefreshCount;
	int iPaintCount;
} app_state_t;

static void AppRequestRefresh(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iRefreshCount++;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	if ( xgeXuiRefreshNeeded(&pApp->tXui) == 0 ) {
		return 0;
	}
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	pApp->iPaintCount += xgeXuiPaint(&pApp->tXui);
	xgeRender();
	xgePresent();
	return 0;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;
	xge_rect_t tRect;

	(void)argc;
	(void)argv;
	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 480;
	tDesc.iHeight = 320;
	tDesc.sTitle = "XGE App Mode";
	tDesc.iFlags = XGE_INIT_WINDOW;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	tApp.tHost.request_refresh = AppRequestRefresh;
	tApp.tHost.pUser = &tApp;
	xgeXuiSetHost(&tApp.tXui, &tApp.tHost);
	tApp.pPanel = xgeXuiWidgetCreate();
	if ( tApp.pPanel == NULL ) {
		xgeXuiUnit(&tApp.tXui);
		xgeUnit();
		return 3;
	}
	tRect.fX = 24.0f;
	tRect.fY = 24.0f;
	tRect.fW = 240.0f;
	tRect.fH = 96.0f;
	xgeXuiWidgetSetRect(tApp.pPanel, tRect);
	xgeXuiWidgetSetBackground(tApp.pPanel, XGE_COLOR_RGBA(42, 96, 160, 255));
	xgeXuiWidgetAdd(xgeXuiRoot(&tApp.tXui), tApp.pPanel);
	xgeRun(AppFrame, &tApp);
	while ( xgeXuiRefreshNeeded(&tApp.tXui) ) {
		xgeFrame();
	}
	printf("app-mode refresh=%d paint=%d dirty=%d\n", tApp.iRefreshCount, tApp.iPaintCount, xgeDirtyRectCount());
	xgeXuiUnit(&tApp.tXui);
	xgeUnit();
	return 0;
}
