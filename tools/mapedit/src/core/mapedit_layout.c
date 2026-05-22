#include "mapedit_layout.h"
#include "mapedit_workspace.h"
#include <stdio.h>

static int MapEditAddChild(xge_xui_widget pParent, xge_xui_widget pChild)
{
	if ( (pParent == NULL) || (pChild == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeXuiWidgetAdd(pParent, pChild);
}

static int MapEditLayoutCreateShell(mapedit_app_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pApp->pMenuWidget = xgeXuiWidgetCreate();
	pApp->pWorkspaceWidget = xgeXuiWidgetCreate();
	pApp->pClientWidget = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pTilesetEditorButtonWidget = xgeXuiWidgetCreate();
	pApp->pMapEditorButtonWidget = xgeXuiWidgetCreate();
	if ( (pApp->pMenuWidget == NULL) || (pApp->pWorkspaceWidget == NULL) || (pApp->pClientWidget == NULL) || (pApp->pStatusWidget == NULL) ||
		(pApp->pTilesetEditorButtonWidget == NULL) || (pApp->pMapEditorButtonWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pApp->pMenuWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(26.0f));
	xgeXuiWidgetSetSize(pApp->pWorkspaceWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(68.0f));
	xgeXuiWidgetSetSize(pApp->pClientWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetMinSize(pApp->pClientWidget, xgeXuiSizePx(760.0f), xgeXuiSizePx(420.0f));
	xgeXuiWidgetSetSize(pApp->pStatusWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pTilesetEditorButtonWidget, xgeXuiSizePx(86.0f), xgeXuiSizePx(58.0f));
	xgeXuiWidgetSetSize(pApp->pMapEditorButtonWidget, xgeXuiSizePx(86.0f), xgeXuiSizePx(58.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pMenuWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pWorkspaceWidget, 6.0f, 5.0f, 6.0f, 5.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pClientWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 0.0f, 1.0f, 0.0f, 1.0f);
	xgeXuiWidgetSetLayout(pApp->pWorkspaceWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetLayout(pApp->pClientWidget, XGE_XUI_LAYOUT_STACK);
	xgeXuiWidgetSetGap(pApp->pWorkspaceWidget, 6.0f);
	xgeXuiWidgetSetAlign(pApp->pWorkspaceWidget, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetBackground(pApp->pWorkspaceWidget, XGE_COLOR_RGBA(239, 248, 253, 255));
	xgeXuiWidgetSetBorder(pApp->pWorkspaceWidget, 0.0f, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( MapEditAddChild(pRoot, pApp->pMenuWidget) != XGE_OK ||
		MapEditAddChild(pRoot, pApp->pWorkspaceWidget) != XGE_OK ||
		MapEditAddChild(pRoot, pApp->pClientWidget) != XGE_OK ||
		MapEditAddChild(pRoot, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MapEditAddChild(pApp->pWorkspaceWidget, pApp->pTilesetEditorButtonWidget) != XGE_OK ||
		MapEditAddChild(pApp->pWorkspaceWidget, pApp->pMapEditorButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

int MapEditLayoutBuild(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( MapEditLayoutCreateShell(pApp) != XGE_OK ) {
		printf("mapedit layout build failed: shell\n");
		return XGE_ERROR;
	}
	if ( MapEditWorkspacesCreate(pApp, pApp->pClientWidget) != XGE_OK ) {
		printf("mapedit layout build failed: workspaces\n");
		return XGE_ERROR;
	}
	return XGE_OK;
}

int MapEditLayoutLoadUser(mapedit_app_t* pApp)
{
	return MapEditWorkspacesLoadUser(pApp);
}

int MapEditLayoutLoadDefault(mapedit_app_t* pApp)
{
	return MapEditWorkspacesLoadDefault(pApp);
}

int MapEditLayoutSaveUser(mapedit_app_t* pApp)
{
	return MapEditWorkspacesSaveUser(pApp);
}

int MapEditLayoutReset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return MapEditWorkspacesReset(pApp);
}
