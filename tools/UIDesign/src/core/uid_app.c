#include "uid_app.h"
#include <stdio.h>
#include <string.h>

static void UIDesignRootResize(uid_app_t* pApp)
{
	xge_rect_t tRect;

	if ( (pApp == NULL) || (pApp->pRoot == NULL) ) {
		return;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)xgeGetWidth();
	tRect.fH = (float)xgeGetHeight();
	xgeXuiWidgetSetRect(pApp->pRoot, tRect);
}

int UIDesignAppInit(uid_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pApp, 0, sizeof(*pApp));
	UIDesignDocumentInit(&pApp->tDocument);
	UIDesignProjectInit(&pApp->tProject);
	UIDesignControlRegistryInit(&pApp->tRegistry);
	UIDesignUndoInit(&pApp->tUndo);
	UIDesignRefreshInit(&pApp->tRefresh);
	return XGE_OK;
}

void UIDesignAppUnit(uid_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	UIDesignUndoUnit(&pApp->tUndo);
	UIDesignControlRegistryUnit(&pApp->tRegistry);
	UIDesignProjectUnit(&pApp->tProject);
	UIDesignDocumentUnit(&pApp->tDocument);
}

int UIDesignAppEnter(xge_scene pScene)
{
	uid_app_t* pApp;

	pApp = (uid_app_t*)pScene->pUser;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		printf("uidesign enter failed: xgeXuiInit\n");
		return XGE_ERROR;
	}
	pApp->pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pApp->pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetBackground(pApp->pRoot, XGE_COLOR_RGBA(226, 232, 238, 255));
	UIDesignRootResize(pApp);
	UIDesignControlRegistryRegisterBuiltins(&pApp->tRegistry);
	if ( UIDesignShellBuild(pApp) != XGE_OK ) {
		printf("uidesign enter failed: shell build\n");
		return XGE_ERROR;
	}
	pApp->bCreateOK = 1;
	UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_BOOT);
	return XGE_OK;
}

int UIDesignAppLeave(xge_scene pScene)
{
	uid_app_t* pApp;

	pApp = (uid_app_t*)pScene->pUser;
	if ( pApp != NULL ) {
		UIDesignShellUnit(pApp);
		xgeXuiUnit(&pApp->tXui);
	}
	return XGE_OK;
}

int UIDesignAppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	uid_app_t* pApp;
	int iResult;

	pApp = (uid_app_t*)pScene->pUser;
	if ( (pApp == NULL) || (pEvent == NULL) ) {
		return XGE_OK;
	}
	iResult = xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_RESIZE ) {
		UIDesignRootResize(pApp);
		UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_RESIZE);
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	return XGE_OK;
}

int UIDesignAppUpdate(xge_scene pScene, float fDelta)
{
	uid_app_t* pApp;

	pApp = (uid_app_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	UIDesignRefreshAfterUpdate(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("uidesign final-summary frames=%d create=%d controls=%d\n",
			pApp->iFrameCount, pApp->bCreateOK, pApp->tRegistry.iCount);
		xgeQuit();
	}
	return XGE_OK;
}

int UIDesignAppDraw(xge_scene pScene)
{
	uid_app_t* pApp;

	pApp = (uid_app_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(226, 232, 238, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}
