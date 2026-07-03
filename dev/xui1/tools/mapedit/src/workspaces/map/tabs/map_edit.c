#include "map_edit.h"
#include "map_maps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_MAP_EDIT_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_MAP_EDIT_TOOLBAR_HEIGHT 34.0f

static const char* g_arrMapEditToolText[MAPEDIT_MAP_TOOL_COUNT] = {
	"画笔",
	"橡皮",
	"线条",
	"矩形填充",
	"圆形填充",
	"填充模式"
};

static const int g_arrMapEditToolType[MAPEDIT_MAP_TOOL_COUNT] = {
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE
};

static const char* g_arrMapEditCommandText[MAPEDIT_MAP_COMMAND_COUNT] = {
	"预览",
	"网格",
	"撤销",
	"重做"
};

static const int g_arrMapEditCommandType[MAPEDIT_MAP_COMMAND_COUNT] = {
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_TOGGLE,
	XGE_XUI_TOOLBAR_ITEM_BUTTON,
	XGE_XUI_TOOLBAR_ITEM_BUTTON
};

static float MapEditMapEditAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditMapEditRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditMapEditAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditMapEditAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditMapEditAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditMapEditAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditMapEditRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditMapEditWidgetInteractive(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( pWidget == NULL ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( !xgeXuiWidgetIsVisible(pIt) || !xgeXuiWidgetIsEnabled(pIt) ) {
			return 0;
		}
	}
	return 1;
}

static int MapEditMapEditMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static const char* MapEditMapEditToolName(int iTool)
{
	switch ( iTool ) {
	case MAPEDIT_MAP_TOOL_BRUSH: return "画笔";
	case MAPEDIT_MAP_TOOL_ERASER: return "橡皮";
	case MAPEDIT_MAP_TOOL_LINE: return "线条";
	case MAPEDIT_MAP_TOOL_RECT: return "矩形填充";
	case MAPEDIT_MAP_TOOL_CIRCLE: return "圆形填充";
	case MAPEDIT_MAP_TOOL_BUCKET: return "填充模式";
	default: return "未知工具";
	}
}

static int MapEditMapEditClampActiveLayer(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || pMap->iLayerCount <= 0 ) {
		if ( pApp != NULL ) {
			pApp->iMapActiveLayer = 0;
		}
		return 0;
	}
	if ( pApp->iMapActiveLayer >= pMap->iLayerCount ) {
		pApp->iMapActiveLayer = pMap->iLayerCount - 1;
	}
	if ( pApp->iMapActiveLayer < 0 ) {
		pApp->iMapActiveLayer = 0;
	}
	return pApp->iMapActiveLayer;
}

static int MapEditMapEditTileIndex(mapedit_map_item_t* pMap, int iLayer, int x, int y)
{
	if ( pMap == NULL || iLayer < 0 || iLayer >= pMap->iLayerCount || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
		return -1;
	}
	return (iLayer * pMap->iHeight + y) * pMap->iWidth + x;
}

static int MapEditMapEditGetTile(mapedit_app_t* pApp, int iLayer, int x, int y)
{
	mapedit_map_item_t* pMap;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || pApp->pMapTileData == NULL ) {
		return 0;
	}
	iIndex = MapEditMapEditTileIndex(pMap, iLayer, x, y);
	if ( iIndex < 0 || iIndex >= pApp->iMapTileDataCount ) {
		return 0;
	}
	return pApp->pMapTileData[iIndex];
}

static void MapEditMapEditFreeCommand(mapedit_map_command_t* pCommand)
{
	if ( pCommand == NULL ) {
		return;
	}
	free(pCommand->arrChanges);
	memset(pCommand, 0, sizeof(*pCommand));
}

static void MapEditMapEditClearCommandStack(mapedit_map_command_t* arrCommands, int* pCount)
{
	int i;

	if ( arrCommands == NULL || pCount == NULL ) {
		return;
	}
	for ( i = 0; i < *pCount; i++ ) {
		MapEditMapEditFreeCommand(&arrCommands[i]);
	}
	*pCount = 0;
}

void MapEditMapEditClearHistory(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditMapEditClearCommandStack(pApp->tMapHistory.arrUndo, &pApp->tMapHistory.iUndoCount);
	MapEditMapEditClearCommandStack(pApp->tMapHistory.arrRedo, &pApp->tMapHistory.iRedoCount);
	MapEditMapEditFreeCommand(&pApp->tMapHistory.tCurrent);
	pApp->tMapHistory.bRecording = 0;
	pApp->tMapHistory.bReplaying = 0;
	if ( pApp->pMapEditCommandToolbarWidget != NULL ) {
		xgeXuiToolbarSetItemEnabled(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_UNDO, 0);
		xgeXuiToolbarSetItemEnabled(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_REDO, 0);
	}
}

static int MapEditMapEditEnsureCommandCapacity(mapedit_map_command_t* pCommand, int iNeed)
{
	mapedit_map_cell_change_t* pNext;
	int iCapacity;

	if ( pCommand == NULL || iNeed <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pCommand->iChangeCapacity >= iNeed ) {
		return XGE_OK;
	}
	iCapacity = (pCommand->iChangeCapacity > 0) ? pCommand->iChangeCapacity : 64;
	while ( iCapacity < iNeed ) {
		iCapacity *= 2;
	}
	pNext = (mapedit_map_cell_change_t*)realloc(pCommand->arrChanges, (size_t)iCapacity * sizeof(mapedit_map_cell_change_t));
	if ( pNext == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pCommand->arrChanges = pNext;
	pCommand->iChangeCapacity = iCapacity;
	return XGE_OK;
}

static void MapEditMapEditRefreshToolbarState(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMapEditToolToolbarWidget != NULL ) {
		for ( i = 0; i < MAPEDIT_MAP_TOOL_COUNT; i++ ) {
			xgeXuiToolbarSetItemChecked(&pApp->tMapEditToolToolbar, i, i == pApp->iMapSelectedTool);
		}
	}
	if ( pApp->pMapEditCommandToolbarWidget != NULL ) {
		xgeXuiToolbarSetItemChecked(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_PREVIEW, pApp->bMapPreviewMode);
		xgeXuiToolbarSetItemChecked(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_GRID, pApp->bMapShowGrid);
		xgeXuiToolbarSetItemChecked(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_UNDO, 0);
		xgeXuiToolbarSetItemChecked(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_REDO, 0);
		xgeXuiToolbarSetItemEnabled(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_UNDO, pApp->tMapHistory.iUndoCount > 0);
		xgeXuiToolbarSetItemEnabled(&pApp->tMapEditCommandToolbar, MAPEDIT_MAP_COMMAND_REDO, pApp->tMapHistory.iRedoCount > 0);
	}
}

static void MapEditMapEditSetTileRaw(mapedit_app_t* pApp, int iLayer, int x, int y, int iTile)
{
	mapedit_map_item_t* pMap;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || pApp->pMapTileData == NULL ) {
		return;
	}
	iIndex = MapEditMapEditTileIndex(pMap, iLayer, x, y);
	if ( iIndex < 0 || iIndex >= pApp->iMapTileDataCount ) {
		return;
	}
	pApp->pMapTileData[iIndex] = iTile;
	if ( pApp->pMapEditGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
	}
}

static void MapEditMapEditRecordChange(mapedit_app_t* pApp, int iLayer, int x, int y, int iBeforeTile, int iAfterTile)
{
	mapedit_map_command_t* pCommand;
	int i;

	if ( pApp == NULL || !pApp->tMapHistory.bRecording || pApp->tMapHistory.bReplaying || iBeforeTile == iAfterTile ) {
		return;
	}
	pCommand = &pApp->tMapHistory.tCurrent;
	for ( i = 0; i < pCommand->iChangeCount; i++ ) {
		if ( pCommand->arrChanges[i].iLayer == iLayer && pCommand->arrChanges[i].x == x && pCommand->arrChanges[i].y == y ) {
			pCommand->arrChanges[i].iAfterTile = iAfterTile;
			return;
		}
	}
	if ( MapEditMapEditEnsureCommandCapacity(pCommand, pCommand->iChangeCount + 1) != XGE_OK ) {
		return;
	}
	pCommand->arrChanges[pCommand->iChangeCount].iLayer = iLayer;
	pCommand->arrChanges[pCommand->iChangeCount].x = x;
	pCommand->arrChanges[pCommand->iChangeCount].y = y;
	pCommand->arrChanges[pCommand->iChangeCount].iBeforeTile = iBeforeTile;
	pCommand->arrChanges[pCommand->iChangeCount].iAfterTile = iAfterTile;
	pCommand->iChangeCount++;
}

static void MapEditMapEditSetTile(mapedit_app_t* pApp, int iLayer, int x, int y, int iTile)
{
	int iBeforeTile;

	iBeforeTile = MapEditMapEditGetTile(pApp, iLayer, x, y);
	if ( iBeforeTile == iTile ) {
		return;
	}
	MapEditMapEditRecordChange(pApp, iLayer, x, y, iBeforeTile, iTile);
	MapEditMapEditSetTileRaw(pApp, iLayer, x, y, iTile);
}

static void MapEditMapEditPushCommand(mapedit_map_command_t* arrCommands, int* pCount, mapedit_map_command_t* pCommand)
{
	int i;

	if ( arrCommands == NULL || pCount == NULL || pCommand == NULL || pCommand->iChangeCount <= 0 ) {
		return;
	}
	if ( *pCount >= MAPEDIT_MAP_HISTORY_LIMIT ) {
		MapEditMapEditFreeCommand(&arrCommands[0]);
		for ( i = 1; i < *pCount; i++ ) {
			arrCommands[i - 1] = arrCommands[i];
		}
		*pCount = MAPEDIT_MAP_HISTORY_LIMIT - 1;
	}
	arrCommands[*pCount] = *pCommand;
	(*pCount)++;
	memset(pCommand, 0, sizeof(*pCommand));
}

static int MapEditMapEditCompactCommand(mapedit_map_command_t* pCommand)
{
	int i;
	int n;

	if ( pCommand == NULL ) {
		return 0;
	}
	n = 0;
	for ( i = 0; i < pCommand->iChangeCount; i++ ) {
		if ( pCommand->arrChanges[i].iBeforeTile == pCommand->arrChanges[i].iAfterTile ) {
			continue;
		}
		if ( n != i ) {
			pCommand->arrChanges[n] = pCommand->arrChanges[i];
		}
		n++;
	}
	pCommand->iChangeCount = n;
	return n;
}

static void MapEditMapEditBeginCommand(mapedit_app_t* pApp, int iTool)
{
	if ( pApp == NULL || pApp->tMapHistory.bReplaying || pApp->tMapHistory.bRecording ) {
		return;
	}
	MapEditMapEditFreeCommand(&pApp->tMapHistory.tCurrent);
	pApp->tMapHistory.tCurrent.iTool = iTool;
	pApp->tMapHistory.tCurrent.iLayer = MapEditMapEditClampActiveLayer(pApp);
	pApp->tMapHistory.bRecording = 1;
}

static int MapEditMapEditCommitCommand(mapedit_app_t* pApp)
{
	if ( pApp == NULL || !pApp->tMapHistory.bRecording ) {
		return 0;
	}
	pApp->tMapHistory.bRecording = 0;
	if ( MapEditMapEditCompactCommand(&pApp->tMapHistory.tCurrent) <= 0 ) {
		MapEditMapEditFreeCommand(&pApp->tMapHistory.tCurrent);
		MapEditMapEditRefreshToolbarState(pApp);
		return 0;
	}
	MapEditMapEditClearCommandStack(pApp->tMapHistory.arrRedo, &pApp->tMapHistory.iRedoCount);
	MapEditMapEditPushCommand(pApp->tMapHistory.arrUndo, &pApp->tMapHistory.iUndoCount, &pApp->tMapHistory.tCurrent);
	MapEditMapEditRefreshToolbarState(pApp);
	return 1;
}

static void MapEditMapEditCancelCommand(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->tMapHistory.bRecording = 0;
	MapEditMapEditFreeCommand(&pApp->tMapHistory.tCurrent);
}

static void MapEditMapEditUndo(mapedit_app_t* pApp)
{
	mapedit_map_command_t tCommand;
	int i;

	if ( pApp == NULL || pApp->tMapHistory.iUndoCount <= 0 ) {
		return;
	}
	MapEditMapEditCancelCommand(pApp);
	pApp->tMapHistory.iUndoCount--;
	tCommand = pApp->tMapHistory.arrUndo[pApp->tMapHistory.iUndoCount];
	memset(&pApp->tMapHistory.arrUndo[pApp->tMapHistory.iUndoCount], 0, sizeof(mapedit_map_command_t));
	pApp->tMapHistory.bReplaying = 1;
	for ( i = tCommand.iChangeCount - 1; i >= 0; i-- ) {
		MapEditMapEditSetTileRaw(pApp, tCommand.arrChanges[i].iLayer, tCommand.arrChanges[i].x, tCommand.arrChanges[i].y, tCommand.arrChanges[i].iBeforeTile);
	}
	pApp->tMapHistory.bReplaying = 0;
	MapEditMapEditPushCommand(pApp->tMapHistory.arrRedo, &pApp->tMapHistory.iRedoCount, &tCommand);
	MapEditMapSaveSelected(pApp);
	MapEditAppSetStatus(pApp, "已撤销地图编辑操作");
	MapEditMapEditRefreshToolbarState(pApp);
}

static void MapEditMapEditRedo(mapedit_app_t* pApp)
{
	mapedit_map_command_t tCommand;
	int i;

	if ( pApp == NULL || pApp->tMapHistory.iRedoCount <= 0 ) {
		return;
	}
	MapEditMapEditCancelCommand(pApp);
	pApp->tMapHistory.iRedoCount--;
	tCommand = pApp->tMapHistory.arrRedo[pApp->tMapHistory.iRedoCount];
	memset(&pApp->tMapHistory.arrRedo[pApp->tMapHistory.iRedoCount], 0, sizeof(mapedit_map_command_t));
	pApp->tMapHistory.bReplaying = 1;
	for ( i = 0; i < tCommand.iChangeCount; i++ ) {
		MapEditMapEditSetTileRaw(pApp, tCommand.arrChanges[i].iLayer, tCommand.arrChanges[i].x, tCommand.arrChanges[i].y, tCommand.arrChanges[i].iAfterTile);
	}
	pApp->tMapHistory.bReplaying = 0;
	MapEditMapEditPushCommand(pApp->tMapHistory.arrUndo, &pApp->tMapHistory.iUndoCount, &tCommand);
	MapEditMapSaveSelected(pApp);
	MapEditAppSetStatus(pApp, "已重做地图编辑操作");
	MapEditMapEditRefreshToolbarState(pApp);
}

static void MapEditMapEditApplyGridPaintState(mapedit_app_t* pApp)
{
	int bShowGrid;

	if ( pApp == NULL || pApp->pMapEditGridWidget == NULL ) {
		return;
	}
	bShowGrid = pApp->bMapShowGrid && !pApp->bMapPreviewMode;
	MapEditTileGridSetColors(&pApp->tMapEditGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		bShowGrid ? XGE_COLOR_RGBA(188, 224, 244, 130) : XGE_COLOR_RGBA(188, 224, 244, 0),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 80),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tMapEditGrid, !pApp->bMapPreviewMode, !pApp->bMapPreviewMode);
}

static void MapEditMapEditUpdateLayerCombo(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	const char* sName;
	int i;
	int iCount;

	if ( pApp == NULL || pApp->pMapEditLayerComboWidget == NULL ) {
		return;
	}
	pMap = MapEditMapSelectedItem(pApp);
	iCount = (pMap != NULL && pMap->iLayerCount > 0) ? pMap->iLayerCount : MapEditMapSetupLayerCount(pApp);
	if ( iCount > MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		iCount = MAPEDIT_MAP_LAYER_OPTION_MAX;
	}
	pApp->iMapLayerOptionCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		sName = MapEditMapSetupLayerName(pApp, i);
		if ( sName != NULL && sName[0] != 0 ) {
			snprintf(pApp->arrMapLayerOptionText[i], sizeof(pApp->arrMapLayerOptionText[i]), "%s", sName);
		} else {
			snprintf(pApp->arrMapLayerOptionText[i], sizeof(pApp->arrMapLayerOptionText[i]), "图层 %d", i + 1);
		}
		pApp->arrMapLayerOptionItems[i].sText = pApp->arrMapLayerOptionText[i];
		pApp->arrMapLayerOptionItems[i].iValue = i;
		pApp->arrMapLayerOptionItems[i].bEnabled = 1;
		pApp->arrMapLayerOptionItems[i].bSeparator = 0;
		pApp->arrMapLayerOptionItems[i].iIcon = 0;
		pApp->arrMapLayerOptionItems[i].pUser = NULL;
	}
	MapEditMapEditClampActiveLayer(pApp);
	xgeXuiComboBoxSetItemData(&pApp->tMapEditLayerCombo, pApp->arrMapLayerOptionItems, iCount);
	xgeXuiComboBoxSetSelectedValue(&pApp->tMapEditLayerCombo, pApp->iMapActiveLayer);
}

static void MapEditMapEditLayerSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;
	const char* sName;
	char sStatus[64];

	(void)pWidget;
	(void)iIndex;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iMapActiveLayer = xgeXuiComboBoxGetSelectedValue(&pApp->tMapEditLayerCombo);
	MapEditMapEditClampActiveLayer(pApp);
	sName = MapEditMapSetupLayerName(pApp, pApp->iMapActiveLayer);
	if ( sName != NULL && sName[0] != 0 ) {
		snprintf(sStatus, sizeof(sStatus), "当前地图图层: %s", sName);
	} else {
		snprintf(sStatus, sizeof(sStatus), "当前地图图层: %d", pApp->iMapActiveLayer + 1);
	}
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pMapEditGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
	}
}

static void MapEditMapEditToolSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;
	char sStatus[96];

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || iIndex < 0 || iIndex >= MAPEDIT_MAP_TOOL_COUNT ) {
		return;
	}
	pApp->iMapSelectedTool = iIndex;
	MapEditMapEditRefreshToolbarState(pApp);
	snprintf(sStatus, sizeof(sStatus), "地图编辑工具: %s", MapEditMapEditToolName(pApp->iMapSelectedTool));
	MapEditAppSetStatus(pApp, sStatus);
}

static void MapEditMapEditCommandSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( iIndex == MAPEDIT_MAP_COMMAND_PREVIEW ) {
		pApp->bMapPreviewMode = !pApp->bMapPreviewMode;
		pApp->fMapPreviewAnimTime = 0.0f;
		pApp->iMapPreviewAnimFrame = 0;
		MapEditMapEditApplyGridPaintState(pApp);
		MapEditMapEditRefreshToolbarState(pApp);
		if ( pApp->pMapEditGridWidget != NULL ) {
			xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
		}
		xgeRenderRequest();
		MapEditAppSetStatus(pApp, pApp->bMapPreviewMode ? "预览模式已开启" : "预览模式已关闭");
		return;
	}
	if ( iIndex == MAPEDIT_MAP_COMMAND_GRID ) {
		pApp->bMapShowGrid = !pApp->bMapShowGrid;
		MapEditMapEditApplyGridPaintState(pApp);
		MapEditMapEditRefreshToolbarState(pApp);
		MapEditAppSetStatus(pApp, pApp->bMapShowGrid ? "已显示地图网格" : "已隐藏地图网格");
		return;
	}
	if ( iIndex == MAPEDIT_MAP_COMMAND_UNDO ) {
		MapEditMapEditUndo(pApp);
		return;
	}
	if ( iIndex == MAPEDIT_MAP_COMMAND_REDO ) {
		MapEditMapEditRedo(pApp);
		return;
	}
}

static float MapEditMapEditContentWidth(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iWidth * MapEditMapTileWidth(pApp));
}

static float MapEditMapEditContentHeight(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iHeight * MapEditMapTileHeight(pApp));
}

static void MapEditMapEditSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditMapEditRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditMapEditRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditMapEditAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditMapEditAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditMapEditUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapEditScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditMapEditContentWidth(pApp);
	fContentH = MapEditMapEditContentHeight(pApp);
	if ( MapEditMapEditAbsFloat(pApp->tMapEditScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditMapEditAbsFloat(pApp->tMapEditScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tMapEditScrollFrame, fContentW, fContentH);
	}
}

static void MapEditMapEditSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapEditScrollFrame.pWidget == NULL || pApp->pMapEditGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapEditScrollFrame);
	fContentW = (pApp->tMapEditScrollModel.fContentW > 1.0f) ? pApp->tMapEditScrollModel.fContentW : MapEditMapEditContentWidth(pApp);
	fContentH = (pApp->tMapEditScrollModel.fContentH > 1.0f) ? pApp->tMapEditScrollModel.fContentH : MapEditMapEditContentHeight(pApp);
	tLocal.fX = -pApp->tMapEditScrollModel.fScrollX;
	tLocal.fY = -pApp->tMapEditScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditMapEditSetScrollContentRect(pApp->pMapEditGridWidget, tLocal, tRect);
}

static void MapEditMapEditUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditMapEditUpdateScrollContentSize(pApp);
	MapEditMapEditSyncScrollContent(pApp);
}

static void MapEditMapEditScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditMapEditSyncScrollContent(pApp);
}

static void MapEditMapEditScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tMapEditScrollFrame);
	MapEditMapEditSyncScrollContent(pApp);
}

static void MapEditMapEditCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	MapEditMapDrawCell(pApp, pGrid, iCol, iRow, tCell);
}

static int MapEditMapEditNeedBrush(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return 0;
	}
	if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER ) {
		return 1;
	}
	if ( pApp->iMapBrushTileId < 0 ) {
		MapEditAppSetStatus(pApp, "请先在图块选择中选择图块");
		return 0;
	}
	return 1;
}

static int MapEditMapEditBrushTileAt(mapedit_app_t* pApp, int dx, int dy)
{
	if ( pApp == NULL || pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER ) {
		return 0;
	}
	return pApp->iMapBrushTileId + dy * MapEditMapTilesPerRow(pApp) + dx;
}

static void MapEditMapEditFinishEdit(mapedit_app_t* pApp, const char* sAction)
{
	const char* sName;
	char sStatus[160];

	if ( pApp == NULL ) {
		return;
	}
	if ( MapEditMapEditCommitCommand(pApp) ) {
		if ( MapEditMapSaveSelected(pApp) == XGE_OK ) {
			sName = MapEditMapSetupLayerName(pApp, pApp->iMapActiveLayer);
			if ( sName != NULL && sName[0] != 0 ) {
				snprintf(sStatus, sizeof(sStatus), "%s已写入图层 %s", sAction, sName);
			} else {
				snprintf(sStatus, sizeof(sStatus), "%s已写入图层 %d", sAction, pApp->iMapActiveLayer + 1);
			}
		} else {
			snprintf(sStatus, sizeof(sStatus), "%s已修改，但地图保存失败", sAction);
		}
		MapEditAppSetStatus(pApp, sStatus);
	} else {
		MapEditAppSetStatus(pApp, "地图没有变化");
	}
}

static void MapEditMapEditApplyBrush(mapedit_app_t* pApp, int iCol, int iRow)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iCols;
	int iRows;
	int dx;
	int dy;
	int iTileId;
	int iDstX;
	int iDstY;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || !MapEditMapEditNeedBrush(pApp) || MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return;
	}
	iLayer = MapEditMapEditClampActiveLayer(pApp);
	iCols = (pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER) ? 1 : ((pApp->iMapBrushCols > 0) ? pApp->iMapBrushCols : 1);
	iRows = (pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER) ? 1 : ((pApp->iMapBrushRows > 0) ? pApp->iMapBrushRows : 1);
	MapEditMapEditBeginCommand(pApp, pApp->iMapSelectedTool);
	for ( dy = 0; dy < iRows; dy++ ) {
		for ( dx = 0; dx < iCols; dx++ ) {
			iDstX = iCol + dx;
			iDstY = iRow + dy;
			if ( iDstX < 0 || iDstY < 0 || iDstX >= pMap->iWidth || iDstY >= pMap->iHeight ) {
				continue;
			}
			iTileId = MapEditMapEditBrushTileAt(pApp, dx, dy);
			MapEditMapEditSetTile(pApp, iLayer, iDstX, iDstY, iTileId);
		}
	}
	MapEditMapEditFinishEdit(pApp, (pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER) ? "橡皮" : "画笔");
}

static void MapEditMapEditNormalizeRect(int x0, int y0, int x1, int y1, int* pX, int* pY, int* pW, int* pH)
{
	int iMinX;
	int iMaxX;
	int iMinY;
	int iMaxY;

	iMinX = (x0 < x1) ? x0 : x1;
	iMaxX = (x0 > x1) ? x0 : x1;
	iMinY = (y0 < y1) ? y0 : y1;
	iMaxY = (y0 > y1) ? y0 : y1;
	*pX = iMinX;
	*pY = iMinY;
	*pW = iMaxX - iMinX + 1;
	*pH = iMaxY - iMinY + 1;
}

static void MapEditMapEditApplyRect(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iTileId;
	int x;
	int y;
	int rx;
	int ry;
	int rw;
	int rh;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || !MapEditMapEditNeedBrush(pApp) || MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return;
	}
	MapEditMapEditNormalizeRect(x0, y0, x1, y1, &rx, &ry, &rw, &rh);
	iLayer = MapEditMapEditClampActiveLayer(pApp);
	iTileId = MapEditMapEditBrushTileAt(pApp, 0, 0);
	MapEditMapEditBeginCommand(pApp, pApp->iMapSelectedTool);
	for ( y = ry; y < ry + rh; y++ ) {
		for ( x = rx; x < rx + rw; x++ ) {
			if ( x >= 0 && y >= 0 && x < pMap->iWidth && y < pMap->iHeight ) {
				MapEditMapEditSetTile(pApp, iLayer, x, y, iTileId);
			}
		}
	}
	MapEditMapEditFinishEdit(pApp, "矩形填充");
}

static void MapEditMapEditApplyCircle(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iTileId;
	int x;
	int y;
	int rx;
	int ry;
	int rw;
	int rh;
	float fCx;
	float fCy;
	float fRx;
	float fRy;
	float fNx;
	float fNy;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || !MapEditMapEditNeedBrush(pApp) || MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return;
	}
	MapEditMapEditNormalizeRect(x0, y0, x1, y1, &rx, &ry, &rw, &rh);
	iLayer = MapEditMapEditClampActiveLayer(pApp);
	iTileId = MapEditMapEditBrushTileAt(pApp, 0, 0);
	fCx = (float)rx + ((float)rw - 1.0f) * 0.5f;
	fCy = (float)ry + ((float)rh - 1.0f) * 0.5f;
	fRx = ((float)rw > 1.0f) ? ((float)rw * 0.5f) : 0.5f;
	fRy = ((float)rh > 1.0f) ? ((float)rh * 0.5f) : 0.5f;
	MapEditMapEditBeginCommand(pApp, pApp->iMapSelectedTool);
	for ( y = ry; y < ry + rh; y++ ) {
		for ( x = rx; x < rx + rw; x++ ) {
			if ( x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
				continue;
			}
			fNx = ((float)x - fCx) / fRx;
			fNy = ((float)y - fCy) / fRy;
			if ( fNx * fNx + fNy * fNy <= 1.0f ) {
				MapEditMapEditSetTile(pApp, iLayer, x, y, iTileId);
			}
		}
	}
	MapEditMapEditFinishEdit(pApp, "圆形填充");
}

static void MapEditMapEditApplyLine(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iTileId;
	int dx;
	int dy;
	int sx;
	int sy;
	int err;
	int e2;
	int x;
	int y;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || !MapEditMapEditNeedBrush(pApp) || MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return;
	}
	iLayer = MapEditMapEditClampActiveLayer(pApp);
	iTileId = MapEditMapEditBrushTileAt(pApp, 0, 0);
	dx = abs(x1 - x0);
	dy = -abs(y1 - y0);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = dx + dy;
	x = x0;
	y = y0;
	MapEditMapEditBeginCommand(pApp, pApp->iMapSelectedTool);
	for (;;) {
		if ( x >= 0 && y >= 0 && x < pMap->iWidth && y < pMap->iHeight ) {
			MapEditMapEditSetTile(pApp, iLayer, x, y, iTileId);
		}
		if ( x == x1 && y == y1 ) {
			break;
		}
		e2 = 2 * err;
		if ( e2 >= dy ) {
			err += dy;
			x += sx;
		}
		if ( e2 <= dx ) {
			err += dx;
			y += sy;
		}
	}
	MapEditMapEditFinishEdit(pApp, "线条");
}

static void MapEditMapEditApplyBucket(mapedit_app_t* pApp, int iCol, int iRow)
{
	mapedit_map_item_t* pMap;
	int* arrQueue;
	unsigned char* arrVisited;
	int iLayer;
	int iTargetTile;
	int iNewTile;
	int iHead;
	int iTail;
	int iIndex;
	int x;
	int y;
	int iMapSize;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || !MapEditMapEditNeedBrush(pApp) || MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return;
	}
	if ( iCol < 0 || iRow < 0 || iCol >= pMap->iWidth || iRow >= pMap->iHeight ) {
		return;
	}
	iLayer = MapEditMapEditClampActiveLayer(pApp);
	iTargetTile = MapEditMapEditGetTile(pApp, iLayer, iCol, iRow);
	iNewTile = MapEditMapEditBrushTileAt(pApp, 0, 0);
	if ( iTargetTile == iNewTile ) {
		MapEditAppSetStatus(pApp, "填充区域没有变化");
		return;
	}
	iMapSize = pMap->iWidth * pMap->iHeight;
	arrQueue = (int*)malloc((size_t)iMapSize * sizeof(int));
	arrVisited = (unsigned char*)calloc((size_t)iMapSize, sizeof(unsigned char));
	if ( arrQueue == NULL || arrVisited == NULL ) {
		free(arrQueue);
		free(arrVisited);
		MapEditAppSetStatus(pApp, "填充队列创建失败");
		return;
	}
	iHead = 0;
	iTail = 0;
	arrQueue[iTail++] = iRow * pMap->iWidth + iCol;
	arrVisited[iRow * pMap->iWidth + iCol] = 1;
	MapEditMapEditBeginCommand(pApp, pApp->iMapSelectedTool);
	while ( iHead < iTail ) {
		iIndex = arrQueue[iHead++];
		x = iIndex % pMap->iWidth;
		y = iIndex / pMap->iWidth;
		if ( MapEditMapEditGetTile(pApp, iLayer, x, y) != iTargetTile ) {
			continue;
		}
		MapEditMapEditSetTile(pApp, iLayer, x, y, iNewTile);
		if ( x > 0 && !arrVisited[iIndex - 1] ) {
			arrVisited[iIndex - 1] = 1;
			arrQueue[iTail++] = iIndex - 1;
		}
		if ( x + 1 < pMap->iWidth && !arrVisited[iIndex + 1] ) {
			arrVisited[iIndex + 1] = 1;
			arrQueue[iTail++] = iIndex + 1;
		}
		if ( y > 0 && !arrVisited[iIndex - pMap->iWidth] ) {
			arrVisited[iIndex - pMap->iWidth] = 1;
			arrQueue[iTail++] = iIndex - pMap->iWidth;
		}
		if ( y + 1 < pMap->iHeight && !arrVisited[iIndex + pMap->iWidth] ) {
			arrVisited[iIndex + pMap->iWidth] = 1;
			arrQueue[iTail++] = iIndex + pMap->iWidth;
		}
	}
	free(arrQueue);
	free(arrVisited);
	MapEditMapEditFinishEdit(pApp, "填充");
}

static void MapEditMapEditApplyShape(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_LINE ) {
		MapEditMapEditApplyLine(pApp, x0, y0, x1, y1);
	} else if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_RECT ) {
		MapEditMapEditApplyRect(pApp, x0, y0, x1, y1);
	} else if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_CIRCLE ) {
		MapEditMapEditApplyCircle(pApp, x0, y0, x1, y1);
	}
}

static void MapEditMapEditCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_BRUSH || pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_ERASER ) {
		MapEditMapEditApplyBrush(pApp, iCol, iRow);
		return;
	}
	if ( pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_BUCKET ) {
		MapEditMapEditApplyBucket(pApp, iCol, iRow);
	}
}

void MapEditMapEditRefresh(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	int iCols;
	int iRows;

	if ( pApp == NULL || pApp->pMapEditGridWidget == NULL ) {
		return;
	}
	MapEditMapEnsureDefaults(pApp);
	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap != NULL ) {
		MapEditMapEnsureTileData(pApp);
		iCols = pMap->iWidth;
		iRows = pMap->iHeight;
	} else {
		iCols = 1;
		iRows = 1;
	}
	MapEditMapEditClampActiveLayer(pApp);
	MapEditMapEditUpdateLayerCombo(pApp);
	MapEditTileGridSetCellSize(&pApp->tMapEditGrid, MapEditMapTileWidth(pApp), MapEditMapTileHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMapEditGrid, iCols, iRows);
	MapEditTileGridSetMaxCells(&pApp->tMapEditGrid, iCols * iRows);
	MapEditMapEditApplyGridPaintState(pApp);
	MapEditMapEditRefreshToolbarState(pApp);
	pApp->tMapEditGrid.iHoverCol = -1;
	pApp->tMapEditGrid.iHoverRow = -1;
	pApp->tMapEditGrid.bHasSelection = 0;
	if ( pApp->tMapEditScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMapEditScrollFrame, 0.0f, 0.0f);
		MapEditMapEditUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pMapEditCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
}

int MapEditMapEditHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;
	int iResult;
	int bWasDragging;
	int iStartCol;
	int iStartRow;
	int iEndCol;
	int iEndRow;

	if ( pApp == NULL || pEvent == NULL || pApp->pMapEditGridWidget == NULL || !MapEditMapEditMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pApp->bMapPreviewMode ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditMapEditWidgetInteractive(pApp->pMapEditGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapEditScrollFrame);
	if ( !pApp->tMapEditGrid.bDragging &&
	     (!MapEditMapEditRectContains(tViewport, pEvent->fX, pEvent->fY) ||
	      !MapEditMapEditRectContains(pApp->pMapEditGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	bWasDragging = pApp->tMapEditGrid.bDragging;
	iStartCol = pApp->tMapEditGrid.iDragAnchorCol;
	iStartRow = pApp->tMapEditGrid.iDragAnchorRow;
	iEndCol = pApp->tMapEditGrid.iHoverCol;
	iEndRow = pApp->tMapEditGrid.iHoverRow;
	iResult = MapEditTileGridEventProc(pApp->pMapEditGridWidget, pEvent, &pApp->tMapEditGrid);
	if ( iResult == XGE_XUI_EVENT_CONSUMED &&
	     pEvent->iType == XGE_EVENT_MOUSE_UP &&
	     bWasDragging &&
	     (pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_LINE ||
	      pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_RECT ||
	      pApp->iMapSelectedTool == MAPEDIT_MAP_TOOL_CIRCLE) ) {
		if ( iEndCol < 0 || iEndRow < 0 ) {
			iEndCol = iStartCol;
			iEndRow = iStartRow;
		}
		MapEditMapEditApplyShape(pApp, iStartCol, iStartRow, iEndCol, iEndRow);
	}
	return iResult;
}

static int MapEditMapEditCreateToolbar(mapedit_app_t* pApp, xge_xui_widget pContent)
{
	xge_xui_widget pRowWidget;
	xge_xui_widget pToolWidget;
	xge_xui_widget pLayerWidget;
	xge_xui_widget pCommandWidget;

	pRowWidget = xgeXuiWidgetCreate();
	pToolWidget = xgeXuiWidgetCreate();
	pLayerWidget = xgeXuiWidgetCreate();
	pCommandWidget = xgeXuiWidgetCreate();
	if ( pRowWidget == NULL || pToolWidget == NULL || pLayerWidget == NULL || pCommandWidget == NULL ) {
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pToolWidget);
		xgeXuiWidgetFree(pLayerWidget);
		xgeXuiWidgetFree(pCommandWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pRowWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pRowWidget, 6.0f);
	xgeXuiWidgetSetSize(pRowWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(MAPEDIT_MAP_EDIT_TOOLBAR_HEIGHT));
	xgeXuiWidgetSetPaddingPx(pRowWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetBackground(pRowWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetSize(pToolWidget, xgeXuiSizePx(438.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetSize(pLayerWidget, xgeXuiSizePx(112.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pCommandWidget, xgeXuiSizePx(236.0f), xgeXuiSizePx(30.0f));
	if ( xgeXuiToolbarInit(&pApp->tMapEditToolToolbar, &pApp->tXui, pToolWidget) != XGE_OK ||
	     xgeXuiComboBoxInit(&pApp->tMapEditLayerCombo, &pApp->tXui, pLayerWidget) != XGE_OK ||
	     xgeXuiToolbarInit(&pApp->tMapEditCommandToolbar, &pApp->tXui, pCommandWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pToolWidget);
		xgeXuiWidgetFree(pLayerWidget);
		xgeXuiWidgetFree(pCommandWidget);
		return XGE_ERROR;
	}
	xgeXuiToolbarSetItems(&pApp->tMapEditToolToolbar, g_arrMapEditToolText, g_arrMapEditToolType, MAPEDIT_MAP_TOOL_COUNT);
	xgeXuiToolbarSetFont(&pApp->tMapEditToolToolbar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiToolbarSetItemSize(&pApp->tMapEditToolToolbar, 70.0f, 26.0f, 6.0f);
	xgeXuiToolbarSetSelect(&pApp->tMapEditToolToolbar, MapEditMapEditToolSelect, pApp);
	xgeXuiToolbarSetItems(&pApp->tMapEditCommandToolbar, g_arrMapEditCommandText, g_arrMapEditCommandType, MAPEDIT_MAP_COMMAND_COUNT);
	xgeXuiToolbarSetFont(&pApp->tMapEditCommandToolbar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiToolbarSetItemSize(&pApp->tMapEditCommandToolbar, 54.0f, 26.0f, 6.0f);
	xgeXuiToolbarSetSelect(&pApp->tMapEditCommandToolbar, MapEditMapEditCommandSelect, pApp);
	xgeXuiComboBoxSetFont(&pApp->tMapEditLayerCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetSelect(&pApp->tMapEditLayerCombo, MapEditMapEditLayerSelect, pApp);
	xgeXuiComboBoxSetMetrics(&pApp->tMapEditLayerCombo, 24.0f);
	xgeXuiComboBoxSetPopupMaxHeight(&pApp->tMapEditLayerCombo, 220.0f);
	xgeXuiComboBoxSetColors(&pApp->tMapEditLayerCombo, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(230, 236, 240, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiComboBoxSetItemColors(&pApp->tMapEditLayerCombo, XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(230, 236, 240, 255), XGE_COLOR_RGBA(132, 142, 152, 255));
	xgeXuiWidgetAdd(pRowWidget, pToolWidget);
	xgeXuiWidgetAdd(pRowWidget, pLayerWidget);
	xgeXuiWidgetAdd(pRowWidget, pCommandWidget);
	if ( xgeXuiWidgetAdd(pContent, pRowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pMapEditToolbarRowWidget = pRowWidget;
	pApp->pMapEditToolToolbarWidget = pToolWidget;
	pApp->pMapEditLayerComboWidget = pLayerWidget;
	pApp->pMapEditCommandToolbarWidget = pCommandWidget;
	MapEditMapEditUpdateLayerCombo(pApp);
	MapEditMapEditRefreshToolbarState(pApp);
	return XGE_OK;
}

int MapEditMapEditCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pCanvasWidget;
	xge_xui_widget pGridWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pCanvasWidget = xgeXuiWidgetCreate();
	pGridWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pCanvasWidget == NULL || pGridWidget == NULL ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( MapEditMapEditCreateToolbar(pApp, pContent) != XGE_OK ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetSize(pCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetBackground(pCanvasWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pCanvasWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiScrollModelInit(&pApp->tMapEditScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tMapEditScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tMapEditScrollModel) != XGE_OK ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMapEditScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditMapEditScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapEditScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tMapEditScrollFrame, MapEditMapEditScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMapEditScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMapEditScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tMapEditScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tMapEditScrollFrame, MAPEDIT_MAP_EDIT_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tMapEditScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapEditScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tMapEditGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tMapEditScrollFrame);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tMapEditGrid, 0, 0);
	MapEditMapEditApplyGridPaintState(pApp);
	MapEditTileGridSetEmptyText(&pApp->tMapEditGrid, "未选择地图");
	MapEditTileGridSetCellPaint(&pApp->tMapEditGrid, MapEditMapEditCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tMapEditGrid, MapEditMapEditCellClick, pApp);
	pApp->pMapEditCanvasWidget = pCanvasWidget;
	pApp->pMapEditGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditMapEnsureDefaults(pApp);
	MapEditMapEditRefresh(pApp);
	return XGE_OK;
}

void MapEditMapEditUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditMapEditClearHistory(pApp);
	if ( pApp->pMapEditGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tMapEditGrid);
		pApp->pMapEditGridWidget = NULL;
	}
	if ( pApp->pMapEditCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tMapEditScrollFrame);
		pApp->pMapEditCanvasWidget = NULL;
	}
	if ( pApp->pMapEditCommandToolbarWidget != NULL ) {
		xgeXuiToolbarUnit(&pApp->tMapEditCommandToolbar);
		pApp->pMapEditCommandToolbarWidget = NULL;
	}
	if ( pApp->pMapEditLayerComboWidget != NULL ) {
		xgeXuiComboBoxUnit(&pApp->tMapEditLayerCombo);
		pApp->pMapEditLayerComboWidget = NULL;
	}
	if ( pApp->pMapEditToolToolbarWidget != NULL ) {
		xgeXuiToolbarUnit(&pApp->tMapEditToolToolbar);
		pApp->pMapEditToolToolbarWidget = NULL;
	}
	pApp->pMapEditToolbarRowWidget = NULL;
}
