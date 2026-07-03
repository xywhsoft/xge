#include "tileset_tags.h"
#include "tileset_sets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_TILESET_TAGS_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_TILESET_TAGS_PLACEHOLDER_ROWS 8
#define MAPEDIT_TILESET_TAGS_FORM_HEIGHT 148.0f

static float MapEditTilesetTagsAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditTilesetTagsRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditTilesetTagsAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditTilesetTagsAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditTilesetTagsAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditTilesetTagsAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditTilesetTagsRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditTilesetTagsWidgetInteractive(xge_xui_widget pWidget)
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

static int MapEditTilesetTagsMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static void MapEditTilesetTagsCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	size_t iLen;

	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	iLen = strlen(sSrc);
	if ( iLen >= (size_t)iDstSize ) {
		iLen = (size_t)iDstSize - 1u;
	}
	memmove(sDst, sSrc, iLen);
	sDst[iLen] = 0;
}

static int MapEditTilesetTagsIsTileChannel(const mapedit_custom_channel_def_t* pDef)
{
	return pDef != NULL && pDef->sId[0] != 0 && (pDef->sScope[0] == 0 || strcmp(pDef->sScope, "tile") == 0);
}

static mapedit_tileset_set_item_t* MapEditTilesetTagsSelectedItem(mapedit_app_t* pApp)
{
	return MapEditTilesetSetSelectedItem(pApp);
}

static int MapEditTilesetTagsTilesPerRow(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return 20;
	}
	return pApp->iSetupTilesPerRow;
}

static int MapEditTilesetTagsCellWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

static int MapEditTilesetTagsCellHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

static int MapEditTilesetTagsStaticStart(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iReserved;

	pItem = MapEditTilesetTagsSelectedItem(pApp);
	iPerRow = MapEditTilesetTagsTilesPerRow(pApp);
	iReserved = 1 + ((pItem != NULL) ? pItem->iSpecialTileCount : 0);
	if ( iReserved < 1 ) {
		iReserved = 1;
	}
	return ((iReserved + iPerRow - 1) / iPerRow) * iPerRow;
}

static int MapEditTilesetTagsStaticRows(mapedit_app_t* pApp)
{
	int iCellH;
	int iRows;

	if ( pApp == NULL ) {
		return MAPEDIT_TILESET_TAGS_PLACEHOLDER_ROWS;
	}
	iCellH = MapEditTilesetTagsCellHeight(pApp);
	if ( pApp->tTilesetTagsStaticDesc.iHeight > 0 ) {
		iRows = (pApp->tTilesetTagsStaticDesc.iHeight + iCellH - 1) / iCellH;
		return (iRows > 0) ? iRows : 1;
	}
	return MAPEDIT_TILESET_TAGS_PLACEHOLDER_ROWS;
}

static int MapEditTilesetTagsStaticCols(mapedit_app_t* pApp)
{
	int iCellW;
	int iPerRow;
	int iCols;

	if ( pApp == NULL || pApp->tTilesetTagsStaticDesc.iWidth <= 0 ) {
		return 0;
	}
	iCellW = MapEditTilesetTagsCellWidth(pApp);
	iPerRow = MapEditTilesetTagsTilesPerRow(pApp);
	iCols = (pApp->tTilesetTagsStaticDesc.iWidth + iCellW - 1) / iCellW;
	if ( iCols > iPerRow ) {
		iCols = iPerRow;
	}
	return (iCols > 0) ? iCols : 0;
}

static float MapEditTilesetTagsGridContentWidth(mapedit_app_t* pApp)
{
	return (float)(MapEditTilesetTagsTilesPerRow(pApp) * MapEditTilesetTagsCellWidth(pApp));
}

static float MapEditTilesetTagsGridContentHeight(mapedit_app_t* pApp)
{
	int iRows;

	if ( pApp == NULL ) {
		return 1.0f;
	}
	iRows = pApp->tTilesetTagsGrid.iRowCount;
	if ( iRows <= 0 ) {
		iRows = 1;
	}
	return (float)(iRows * MapEditTilesetTagsCellHeight(pApp));
}

static void MapEditTilesetTagsSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditTilesetTagsRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditTilesetTagsRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditTilesetTagsAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditTilesetTagsAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditTilesetTagsClearStaticTexture(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetTagsStaticTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetTagsStaticTexture);
		pApp->pTilesetTagsStaticTexture = NULL;
	}
	memset(&pApp->tTilesetTagsStaticDesc, 0, sizeof(pApp->tTilesetTagsStaticDesc));
}

static void MapEditTilesetTagsLoadStaticTexture(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem)
{
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	MapEditTilesetTagsClearStaticTexture(pApp);
	if ( pApp == NULL || pItem == NULL || pItem->sStaticTiles[0] == 0 || strcmp(pItem->sStaticTiles, "未设置") == 0 ) {
		return;
	}
	sPath = xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)"tilesheets", (str)pItem->sStaticTiles);
	if ( sPath == NULL ) {
		return;
	}
	if ( !xrtFileExists(sPath) ) {
		xrtFree(sPath);
		return;
	}
	pTexture = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) == XGE_OK &&
	     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) == XGE_OK &&
	     tDesc.iWidth > 0 && tDesc.iHeight > 0 ) {
		pApp->pTilesetTagsStaticTexture = pTexture;
		pApp->tTilesetTagsStaticDesc = tDesc;
	} else if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
}

static void MapEditTilesetTagsUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetTagsScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditTilesetTagsGridContentWidth(pApp);
	fContentH = MapEditTilesetTagsGridContentHeight(pApp);
	if ( MapEditTilesetTagsAbsFloat(pApp->tTilesetTagsScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditTilesetTagsAbsFloat(pApp->tTilesetTagsScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tTilesetTagsScrollFrame, fContentW, fContentH);
	}
}

static void MapEditTilesetTagsSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetTagsScrollFrame.pWidget == NULL || pApp->pTilesetTagsGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetTagsScrollFrame);
	fContentW = (pApp->tTilesetTagsScrollModel.fContentW > 1.0f) ? pApp->tTilesetTagsScrollModel.fContentW : MapEditTilesetTagsGridContentWidth(pApp);
	fContentH = (pApp->tTilesetTagsScrollModel.fContentH > 1.0f) ? pApp->tTilesetTagsScrollModel.fContentH : MapEditTilesetTagsGridContentHeight(pApp);
	tLocal.fX = -pApp->tTilesetTagsScrollModel.fScrollX;
	tLocal.fY = -pApp->tTilesetTagsScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditTilesetTagsSetScrollContentRect(pApp->pTilesetTagsGridWidget, tLocal, tRect);
}

static void MapEditTilesetTagsUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditTilesetTagsUpdateScrollContentSize(pApp);
	MapEditTilesetTagsSyncScrollContent(pApp);
}

static void MapEditTilesetTagsScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditTilesetTagsSyncScrollContent(pApp);
}

static void MapEditTilesetTagsScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tTilesetTagsScrollFrame);
	MapEditTilesetTagsSyncScrollContent(pApp);
}

static int MapEditTilesetTagsTileEditable(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem, int iTileId)
{
	int iStaticStart;
	int iLocal;
	int iRow;
	int iCol;

	if ( pApp == NULL || pItem == NULL || iTileId < 0 ) {
		return 0;
	}
	if ( iTileId <= pItem->iSpecialTileCount ) {
		return 1;
	}
	iStaticStart = MapEditTilesetTagsStaticStart(pApp);
	if ( iTileId < iStaticStart || pApp->pTilesetTagsStaticTexture == NULL ) {
		return 0;
	}
	iLocal = iTileId - iStaticStart;
	iRow = iLocal / MapEditTilesetTagsTilesPerRow(pApp);
	iCol = iLocal % MapEditTilesetTagsTilesPerRow(pApp);
	if ( iRow < 0 || iRow >= MapEditTilesetTagsStaticRows(pApp) ) {
		return 0;
	}
	return iCol < MapEditTilesetTagsStaticCols(pApp);
}

static const mapedit_custom_channel_def_t* MapEditTilesetTagsCurrentChannel(mapedit_app_t* pApp)
{
	return MapEditCustomChannelGet(pApp, (pApp != NULL) ? pApp->iTilesetTagChannel : -1);
}

static void MapEditTilesetTagsUseDefaultValue(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;

	if ( pApp == NULL ) {
		return;
	}
	pDef = MapEditTilesetTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), "");
		return;
	}
	MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), pDef->sDefaultValue);
}

static int MapEditTilesetTagsEnumFindByValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int i;

	if ( pDef == NULL || sValue == NULL ) {
		return -1;
	}
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		if ( strcmp(pDef->arrOptions[i].sValue, sValue) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int MapEditTilesetTagsEnumFindByText(const mapedit_custom_channel_def_t* pDef, const char* sText)
{
	int i;

	if ( pDef == NULL || sText == NULL ) {
		return -1;
	}
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		if ( strcmp(pDef->arrOptions[i].sText, sText) == 0 ) {
			return i;
		}
	}
	return -1;
}

static const char* MapEditTilesetTagsEnumDisplayForValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int iIndex;

	iIndex = MapEditTilesetTagsEnumFindByValue(pDef, sValue);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sText;
	}
	return (sValue != NULL) ? sValue : "";
}

static const char* MapEditTilesetTagsEnumValueForDisplay(const mapedit_custom_channel_def_t* pDef, const char* sText)
{
	int iIndex;

	iIndex = MapEditTilesetTagsEnumFindByText(pDef, sText);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sValue;
	}
	iIndex = MapEditTilesetTagsEnumFindByValue(pDef, sText);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sValue;
	}
	return (sText != NULL) ? sText : "";
}

static int MapEditTilesetTagsValueAsInt(const char* sValue)
{
	if ( sValue == NULL || sValue[0] == 0 ) {
		return 0;
	}
	if ( strcmp(sValue, "true") == 0 ) {
		return 1;
	}
	if ( strcmp(sValue, "false") == 0 ) {
		return 0;
	}
	return atoi(sValue);
}

static void MapEditTilesetTagsClampValue(mapedit_app_t* pApp, const mapedit_custom_channel_def_t* pDef)
{
	int iValue;

	if ( pApp == NULL || pDef == NULL ) {
		return;
	}
	if ( strcmp(pDef->sDataType, "string") == 0 || strcmp(pDef->sDataType, "text") == 0 ) {
		return;
	}
	if ( strcmp(pDef->sDataType, "float") == 0 || strcmp(pDef->sDataType, "double") == 0 ) {
		return;
	}
	iValue = MapEditTilesetTagsValueAsInt(pApp->sTilesetTagValue);
	if ( pDef->bHasMin && iValue < pDef->iMinValue ) {
		iValue = pDef->iMinValue;
	}
	if ( pDef->bHasMax && iValue > pDef->iMaxValue ) {
		iValue = pDef->iMaxValue;
	}
	snprintf(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), "%d", iValue);
}

static int MapEditTilesetTagsAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
{
	xge_xui_property_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sId = sId;
	tDesc.sName = sName;
	tDesc.sDescription = sDescription;
	tDesc.iType = iType;
	tDesc.sValue = sValue;
	tDesc.sDefaultValue = sDefault;
	tDesc.iFlags = iFlags;
	return xgeXuiPropertyGridAddProperty(pGrid, iCategory, &tDesc);
}

static void MapEditTilesetTagsSyncFlagProperties(mapedit_app_t* pApp, const mapedit_custom_channel_def_t* pDef)
{
	char sId[32];
	int iValue;
	int iProp;
	int i;

	if ( pApp == NULL || pDef == NULL || pApp->bTilesetTagsFormUpdating ) {
		return;
	}
	pApp->bTilesetTagsFormUpdating = 1;
	iValue = MapEditTilesetTagsValueAsInt(pApp->sTilesetTagValue);
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sId, sizeof(sId), "tag.flag.%d", i);
		iProp = xgeXuiPropertyGridFindProperty(&pApp->tTilesetTagsFormGrid, sId);
		if ( iProp >= 0 ) {
			xgeXuiPropertyGridSetBool(&pApp->tTilesetTagsFormGrid, iProp, (iValue & (1 << pDef->arrOptions[i].iBit)) != 0);
		}
	}
	pApp->bTilesetTagsFormUpdating = 0;
}

static void MapEditTilesetTagsBuildForm(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;
	xge_xui_property_grid pGrid;
	xge_xui_table_grid_editor_config_t tEnum;
	char sValueDisplay[MAPEDIT_CUSTOM_TEXT_CAPACITY];
	char sFlagId[32];
	int iCategory;
	int iValueProp;
	int iType;
	int i;

	if ( pApp == NULL || pApp->pTilesetTagsFormGridWidget == NULL ) {
		return;
	}
	pGrid = &pApp->tTilesetTagsFormGrid;
	pApp->bTilesetTagsFormUpdating = 1;
	xgeXuiPropertyGridClear(pGrid);
	pDef = MapEditTilesetTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		iCategory = xgeXuiPropertyGridAddCategory(pGrid, "tag.empty", "打标数据", 1);
		if ( iCategory >= 0 ) {
			MapEditTilesetTagsAddProperty(pGrid, iCategory, "tag.empty.hint", "当前通道", "setup.xson 中没有 scope=tile 的 customData 通道", XGE_XUI_TABLE_CELL_TYPE_TEXT, "未配置", "未配置", XGE_XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bTilesetTagsFormUpdating = 0;
		return;
	}
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "tag.form", "打标值", 1);
	if ( iCategory < 0 ) {
		pApp->bTilesetTagsFormUpdating = 0;
		return;
	}
	if ( pApp->sTilesetTagValue[0] == 0 && pDef->sDefaultValue[0] != 0 ) {
		MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), pDef->sDefaultValue);
	}
	MapEditTilesetTagsClampValue(pApp, pDef);
	iType = XGE_XUI_TABLE_CELL_TYPE_INT;
	if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_ENUM;
		MapEditTilesetTagsCopyText(sValueDisplay, sizeof(sValueDisplay), MapEditTilesetTagsEnumDisplayForValue(pDef, pApp->sTilesetTagValue));
	} else if ( strcmp(pDef->sDataType, "string") == 0 || strcmp(pDef->sDataType, "text") == 0 || strcmp(pDef->sMarkMode, "text") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
		MapEditTilesetTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sTilesetTagValue);
	} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_BOOL;
		MapEditTilesetTagsCopyText(sValueDisplay, sizeof(sValueDisplay), MapEditTilesetTagsValueAsInt(pApp->sTilesetTagValue) ? "true" : "false");
	} else if ( strcmp(pDef->sDataType, "float") == 0 || strcmp(pDef->sDataType, "double") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_FLOAT;
		MapEditTilesetTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sTilesetTagValue);
	} else {
		MapEditTilesetTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sTilesetTagValue);
	}
	iValueProp = MapEditTilesetTagsAddProperty(pGrid, iCategory, "tag.value", pDef->sName[0] ? pDef->sName : "值", pDef->sId, iType, sValueDisplay, pDef->sDefaultValue, 0);
	if ( iValueProp >= 0 && iType == XGE_XUI_TABLE_CELL_TYPE_ENUM ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = (const char**)pDef->arrOptionItems;
		tEnum.iEnumItemCount = pDef->iOptionCount;
		tEnum.iEnumSelected = MapEditTilesetTagsEnumFindByValue(pDef, pApp->sTilesetTagValue);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iValueProp, &tEnum);
	}
	if ( strcmp(pDef->sMarkMode, "bitmask") == 0 && pDef->iOptionCount > 0 ) {
		for ( i = 0; i < pDef->iOptionCount; i++ ) {
			snprintf(sFlagId, sizeof(sFlagId), "tag.flag.%d", i);
			MapEditTilesetTagsAddProperty(pGrid, iCategory, sFlagId, pDef->arrOptions[i].sText, "bitmask 标记位", XGE_XUI_TABLE_CELL_TYPE_BOOL,
				(MapEditTilesetTagsValueAsInt(pApp->sTilesetTagValue) & (1 << pDef->arrOptions[i].iBit)) ? "true" : "false", "false", 0);
		}
	}
	xgeXuiPropertyGridSetSelected(pGrid, iValueProp);
	pApp->bTilesetTagsFormUpdating = 0;
}

static void MapEditTilesetTagsFormChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp;
	const mapedit_custom_channel_def_t* pDef;
	char sFlagId[32];
	int iValue;
	int i;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pApp->bTilesetTagsFormUpdating || sId == NULL ) {
		return;
	}
	pDef = MapEditTilesetTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		return;
	}
	if ( strcmp(sId, "tag.value") == 0 ) {
		if ( strcmp(pDef->sMarkMode, "enum") == 0 ) {
			MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), MapEditTilesetTagsEnumValueForDisplay(pDef, sValue));
		} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
			MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), MapEditTilesetTagsValueAsInt(sValue) ? "true" : "false");
		} else {
			MapEditTilesetTagsCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), sValue);
		}
		MapEditTilesetTagsClampValue(pApp, pDef);
		if ( strcmp(pDef->sMarkMode, "bitmask") == 0 ) {
			MapEditTilesetTagsSyncFlagProperties(pApp, pDef);
		}
		MapEditAppSetStatus(pApp, "已更新打标值");
		return;
	}
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sFlagId, sizeof(sFlagId), "tag.flag.%d", i);
		if ( strcmp(sId, sFlagId) == 0 ) {
			iValue = MapEditTilesetTagsValueAsInt(pApp->sTilesetTagValue);
			if ( MapEditTilesetTagsValueAsInt(sValue) ) {
				iValue |= (1 << pDef->arrOptions[i].iBit);
			} else {
				iValue &= ~(1 << pDef->arrOptions[i].iBit);
			}
			snprintf(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), "%d", iValue);
			iProperty = xgeXuiPropertyGridFindProperty(&pApp->tTilesetTagsFormGrid, "tag.value");
			if ( iProperty >= 0 ) {
				pApp->bTilesetTagsFormUpdating = 1;
				xgeXuiPropertyGridSetValue(&pApp->tTilesetTagsFormGrid, iProperty, pApp->sTilesetTagValue);
				pApp->bTilesetTagsFormUpdating = 0;
			}
			MapEditAppSetStatus(pApp, "已更新打标位");
			return;
		}
	}
}

static void MapEditTilesetTagsUpdateChannelCombo(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;
	int i;
	int iOut;
	int iSelected;

	if ( pApp == NULL || pApp->pTilesetTagsChannelComboWidget == NULL ) {
		return;
	}
	iOut = 0;
	iSelected = -1;
	for ( i = 0; i < pApp->iCustomChannelCount && iOut < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( !MapEditTilesetTagsIsTileChannel(pDef) ) {
			continue;
		}
		snprintf(pApp->arrTilesetTagChannelText[iOut], sizeof(pApp->arrTilesetTagChannelText[iOut]), "%s (%s)", pDef->sName[0] ? pDef->sName : pDef->sId, pDef->sId);
		pApp->arrTilesetTagChannelItems[iOut].sText = pApp->arrTilesetTagChannelText[iOut];
		pApp->arrTilesetTagChannelItems[iOut].iValue = i;
		pApp->arrTilesetTagChannelItems[iOut].bEnabled = 1;
		pApp->arrTilesetTagChannelItems[iOut].bSeparator = 0;
		pApp->arrTilesetTagChannelItems[iOut].iIcon = 0;
		pApp->arrTilesetTagChannelItems[iOut].pUser = NULL;
		if ( pApp->iTilesetTagChannel == i ) {
			iSelected = iOut;
		}
		iOut++;
	}
	pApp->iTilesetTagChannelOptionCount = iOut;
	if ( iOut <= 0 ) {
		pApp->iTilesetTagChannel = -1;
		xgeXuiComboBoxSetItemData(&pApp->tTilesetTagsChannelCombo, NULL, 0);
		MapEditTilesetTagsBuildForm(pApp);
		return;
	}
	if ( iSelected < 0 ) {
		iSelected = 0;
		pApp->iTilesetTagChannel = pApp->arrTilesetTagChannelItems[0].iValue;
		MapEditTilesetTagsUseDefaultValue(pApp);
	}
	xgeXuiComboBoxSetItemData(&pApp->tTilesetTagsChannelCombo, pApp->arrTilesetTagChannelItems, iOut);
	xgeXuiComboBoxSetSelected(&pApp->tTilesetTagsChannelCombo, iSelected);
	MapEditTilesetTagsBuildForm(pApp);
}

static void MapEditTilesetTagsChannelSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( iIndex < 0 || iIndex >= pApp->iTilesetTagChannelOptionCount ) {
		return;
	}
	pApp->iTilesetTagChannel = xgeXuiComboBoxGetSelectedValue(&pApp->tTilesetTagsChannelCombo);
	MapEditTilesetTagsUseDefaultValue(pApp);
	MapEditTilesetTagsBuildForm(pApp);
	if ( pApp->pTilesetTagsGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetTagsGridWidget);
	}
	MapEditAppSetStatus(pApp, "已切换打标通道");
}

static void MapEditTilesetTagsInspectChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->bTilesetTagsInspectMode = bChecked ? 1 : 0;
	MapEditAppSetStatus(pApp, pApp->bTilesetTagsInspectMode ? "图块打标：检查模式" : "图块打标：写入模式");
}

static void MapEditTilesetTagsDrawStaticTexture(mapedit_app_t* pApp, xge_rect_t tRect)
{
	xge_draw_t tDraw;
	int iStaticStartRow;

	if ( pApp == NULL || pApp->pTilesetTagsStaticTexture == NULL ||
	     pApp->tTilesetTagsStaticDesc.iWidth <= 0 || pApp->tTilesetTagsStaticDesc.iHeight <= 0 ) {
		return;
	}
	iStaticStartRow = MapEditTilesetTagsStaticStart(pApp) / MapEditTilesetTagsTilesPerRow(pApp);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pTilesetTagsStaticTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->tTilesetTagsStaticDesc.iWidth, (float)pApp->tTilesetTagsStaticDesc.iHeight };
	tDraw.tDst = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * MapEditTilesetTagsCellHeight(pApp)),
		(float)pApp->tTilesetTagsStaticDesc.iWidth,
		(float)pApp->tTilesetTagsStaticDesc.iHeight
	};
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
}

static void MapEditTilesetTagsContentPaint(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp;
	int iCellH;
	int iStaticStartRow;
	xge_rect_t tStaticRect;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetTagsStaticTexture != NULL ) {
		MapEditTilesetTagsDrawStaticTexture(pApp, tRect);
		return;
	}
	iCellH = MapEditTilesetTagsCellHeight(pApp);
	iStaticStartRow = MapEditTilesetTagsStaticStart(pApp) / MapEditTilesetTagsTilesPerRow(pApp);
	tStaticRect = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * iCellH),
		(float)(MapEditTilesetTagsTilesPerRow(pApp) * MapEditTilesetTagsCellWidth(pApp)),
		(float)(MapEditTilesetTagsStaticRows(pApp) * iCellH)
	};
	xgeShapeRectFillPx(tStaticRect, XGE_COLOR_RGBA(232, 242, 248, 160));
}

static int MapEditTilesetTagsTileIdFromCell(mapedit_app_t* pApp, int iCol, int iRow)
{
	return iRow * MapEditTilesetTagsTilesPerRow(pApp) + iCol;
}

static void MapEditTilesetTagsDrawMarker(mapedit_tile_grid_t* pGrid, xge_rect_t tCell, const char* sValue)
{
	xge_rect_t tDot;
	xge_rect_t tText;
	float fDot;

	if ( pGrid == NULL || sValue == NULL || sValue[0] == 0 ) {
		return;
	}
	fDot = (tCell.fW < tCell.fH ? tCell.fW : tCell.fH) * 0.35f;
	if ( fDot < 5.0f ) {
		fDot = 5.0f;
	}
	if ( fDot > 12.0f ) {
		fDot = 12.0f;
	}
	tDot = (xge_rect_t){ tCell.fX + 2.0f, tCell.fY + 2.0f, fDot, fDot };
	xgeShapeRectFillPx(tDot, XGE_COLOR_RGBA(30, 135, 210, 210));
	xgeShapeRectStrokePx(tDot, 1.0f, XGE_COLOR_RGBA(245, 252, 255, 230));
	if ( tCell.fW >= 28.0f && tCell.fH >= 20.0f ) {
		tText = (xge_rect_t){ tCell.fX + 2.0f, tCell.fY + tCell.fH - 15.0f, tCell.fW - 4.0f, 13.0f };
		xgeShapeRectFillPx(tText, XGE_COLOR_RGBA(245, 252, 255, 190));
		xgeTextDrawRect(pGrid->pFont, sValue, tText, XGE_COLOR_RGBA(31, 75, 112, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	}
}

static void MapEditTilesetTagsCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	const mapedit_custom_channel_def_t* pDef;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int iTileId;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pGrid == NULL ) {
		return;
	}
	pItem = MapEditTilesetTagsSelectedItem(pApp);
	pDef = MapEditTilesetTagsCurrentChannel(pApp);
	iTileId = MapEditTilesetTagsTileIdFromCell(pApp, iCol, iRow);
	if ( !MapEditTilesetTagsTileEditable(pApp, pItem, iTileId) || pDef == NULL ) {
		return;
	}
	if ( MapEditTilesetSetGetTileCustomValue(pItem, pDef->sId, iTileId, sValue, sizeof(sValue)) ) {
		MapEditTilesetTagsDrawMarker(pGrid, tCell, sValue);
	}
}

static void MapEditTilesetTagsCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	const mapedit_custom_channel_def_t* pDef;
	char sStatus[320];
	int iTileId;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetTagsSelectedItem(pApp);
	pDef = MapEditTilesetTagsCurrentChannel(pApp);
	iTileId = MapEditTilesetTagsTileIdFromCell(pApp, iCol, iRow);
	pApp->iTilesetTagsSelectedTile = iTileId;
	if ( !MapEditTilesetTagsTileEditable(pApp, pItem, iTileId) ) {
		MapEditAppSetStatus(pApp, "该位置没有可打标图块");
		return;
	}
	if ( pApp->bTilesetTagsInspectMode ) {
		MapEditTilesetPropertiesShowTileCustomData(pApp, iTileId);
		snprintf(sStatus, sizeof(sStatus), "正在检查图块自定义数据: ID %d", iTileId);
		MapEditAppSetStatus(pApp, sStatus);
		return;
	}
	if ( pDef == NULL ) {
		MapEditAppSetStatus(pApp, "setup.xson 中没有可写入的图块 customData 通道");
		return;
	}
	if ( MapEditTilesetSetSetTileCustomValue(pItem, pDef->sId, iTileId, pApp->sTilesetTagValue) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "图块打标失败");
		return;
	}
	if ( MapEditTilesetSetSaveSelected(pApp) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "图块打标已修改，但保存失败");
		return;
	}
	snprintf(sStatus, sizeof(sStatus), "图块打标已保存: ID %d / %s = %s", iTileId, pDef->sId, pApp->sTilesetTagValue);
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pTilesetTagsGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetTagsGridWidget);
	}
	if ( pApp->iTilesetPropertiesMode == MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM ) {
		MapEditTilesetPropertiesShowTileCustomData(pApp, iTileId);
	}
}

void MapEditTilesetTagsRefresh(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iCellW;
	int iCellH;
	int iStaticStart;
	int iStaticRows;
	int iRows;

	if ( pApp == NULL || pApp->pTilesetTagsGridWidget == NULL ) {
		return;
	}
	MapEditTilesetTagsUpdateChannelCombo(pApp);
	pItem = MapEditTilesetTagsSelectedItem(pApp);
	iPerRow = MapEditTilesetTagsTilesPerRow(pApp);
	iCellW = MapEditTilesetTagsCellWidth(pApp);
	iCellH = MapEditTilesetTagsCellHeight(pApp);
	MapEditTilesetTagsLoadStaticTexture(pApp, pItem);
	iStaticStart = MapEditTilesetTagsStaticStart(pApp);
	iStaticRows = MapEditTilesetTagsStaticRows(pApp);
	iRows = iStaticStart / iPerRow + iStaticRows;
	if ( iRows < MAPEDIT_TILESET_TAGS_PLACEHOLDER_ROWS ) {
		iRows = MAPEDIT_TILESET_TAGS_PLACEHOLDER_ROWS;
	}
	MapEditTileGridSetCellSize(&pApp->tTilesetTagsGrid, iCellW, iCellH);
	MapEditTileGridSetGridSize(&pApp->tTilesetTagsGrid, iPerRow, iRows);
	MapEditTileGridSetMaxCells(&pApp->tTilesetTagsGrid, iPerRow * iRows);
	pApp->tTilesetTagsGrid.bHasSelection = 0;
	pApp->tTilesetTagsGrid.iHoverCol = -1;
	pApp->tTilesetTagsGrid.iHoverRow = -1;
	pApp->iTilesetTagsSelectedTile = -1;
	if ( pApp->tTilesetTagsScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tTilesetTagsScrollFrame, 0.0f, 0.0f);
		MapEditTilesetTagsUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pTilesetTagsCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pTilesetTagsGridWidget);
}

int MapEditTilesetTagsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pTilesetTagsGridWidget == NULL || !MapEditTilesetTagsMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditTilesetTagsWidgetInteractive(pApp->pTilesetTagsGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetTagsScrollFrame);
	if ( pApp->tTilesetTagsGrid.bDragging ||
	     (MapEditTilesetTagsRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditTilesetTagsRectContains(pApp->pTilesetTagsGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return MapEditTileGridEventProc(pApp->pTilesetTagsGridWidget, pEvent, &pApp->tTilesetTagsGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditTilesetTagsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pRowWidget;
	xge_xui_widget pComboWidget;
	xge_xui_widget pCheckWidget;
	xge_xui_widget pFormWidget;
	xge_xui_widget pCanvasWidget;
	xge_xui_widget pGridWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pRowWidget = xgeXuiWidgetCreate();
	pComboWidget = xgeXuiWidgetCreate();
	pCheckWidget = xgeXuiWidgetCreate();
	pFormWidget = xgeXuiWidgetCreate();
	pCanvasWidget = xgeXuiWidgetCreate();
	pGridWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pRowWidget == NULL || pComboWidget == NULL || pCheckWidget == NULL || pFormWidget == NULL || pCanvasWidget == NULL || pGridWidget == NULL ) {
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pRowWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pRowWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetPaddingPx(pRowWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pRowWidget, 8.0f);
	xgeXuiWidgetSetSize(pComboWidget, xgeXuiSizePx(260.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetSize(pCheckWidget, xgeXuiSizePx(190.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetSize(pFormWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(MAPEDIT_TILESET_TAGS_FORM_HEIGHT));
	xgeXuiWidgetSetSize(pCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetBackground(pCanvasWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pCanvasWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiWidgetSetBackground(pFormWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pFormWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiComboBoxInit(&pApp->tTilesetTagsChannelCombo, &pApp->tXui, pComboWidget) != XGE_OK ||
	     xgeXuiCheckBoxInit(&pApp->tTilesetTagsInspectCheck, &pApp->tXui, pCheckWidget) != XGE_OK ||
	     xgeXuiPropertyGridInit(&pApp->tTilesetTagsFormGrid, &pApp->tXui, pFormWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiComboBoxSetFont(&pApp->tTilesetTagsChannelCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetSelect(&pApp->tTilesetTagsChannelCombo, MapEditTilesetTagsChannelSelect, pApp);
	xgeXuiComboBoxSetPopupMaxHeight(&pApp->tTilesetTagsChannelCombo, 220.0f);
	xgeXuiComboBoxSetMetrics(&pApp->tTilesetTagsChannelCombo, 24.0f);
	xgeXuiComboBoxSetColors(&pApp->tTilesetTagsChannelCombo, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(236, 240, 244, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiCheckBoxSetText(&pApp->tTilesetTagsInspectCheck, pApp->bFontReady ? &pApp->tFont : NULL, "检查图块所有字段");
	xgeXuiCheckBoxSetChange(&pApp->tTilesetTagsInspectCheck, MapEditTilesetTagsInspectChange, pApp);
	xgeXuiCheckBoxSetTextColor(&pApp->tTilesetTagsInspectCheck, XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetFont(&pApp->tTilesetTagsFormGrid, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPropertyGridSetMetrics(&pApp->tTilesetTagsFormGrid, 120.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tTilesetTagsFormGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tTilesetTagsFormGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tTilesetTagsFormGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tTilesetTagsFormGrid, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(164, 206, 236, 255), XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetChange(&pApp->tTilesetTagsFormGrid, MapEditTilesetTagsFormChange, pApp);
	xgeXuiScrollModelInit(&pApp->tTilesetTagsScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tTilesetTagsScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tTilesetTagsScrollModel) != XGE_OK ) {
		xgeXuiComboBoxUnit(&pApp->tTilesetTagsChannelCombo);
		xgeXuiCheckBoxUnit(&pApp->tTilesetTagsInspectCheck);
		xgeXuiPropertyGridUnit(&pApp->tTilesetTagsFormGrid);
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tTilesetTagsScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditTilesetTagsScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetTagsScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tTilesetTagsScrollFrame, MapEditTilesetTagsScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tTilesetTagsScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tTilesetTagsScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tTilesetTagsScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tTilesetTagsScrollFrame, MAPEDIT_TILESET_TAGS_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tTilesetTagsScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetTagsScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tTilesetTagsGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetTagsScrollFrame);
		xgeXuiComboBoxUnit(&pApp->tTilesetTagsChannelCombo);
		xgeXuiCheckBoxUnit(&pApp->tTilesetTagsInspectCheck);
		xgeXuiPropertyGridUnit(&pApp->tTilesetTagsFormGrid);
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tTilesetTagsGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tTilesetTagsGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 130),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 80),
		XGE_COLOR_RGBA(31, 75, 112, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tTilesetTagsGrid, 1, 1);
	MapEditTileGridSetEmptyText(&pApp->tTilesetTagsGrid, "");
	MapEditTileGridSetContentPaint(&pApp->tTilesetTagsGrid, MapEditTilesetTagsContentPaint, pApp);
	MapEditTileGridSetCellPaint(&pApp->tTilesetTagsGrid, MapEditTilesetTagsCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tTilesetTagsGrid, MapEditTilesetTagsCellClick, pApp);
	pApp->pTilesetTagsControlRowWidget = pRowWidget;
	pApp->pTilesetTagsChannelComboWidget = pComboWidget;
	pApp->pTilesetTagsInspectCheckWidget = pCheckWidget;
	pApp->pTilesetTagsFormGridWidget = pFormWidget;
	pApp->pTilesetTagsCanvasWidget = pCanvasWidget;
	pApp->pTilesetTagsGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pRowWidget, pComboWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pRowWidget, pCheckWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pRowWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pFormWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditTilesetTagsRefresh(pApp);
	return XGE_OK;
}

void MapEditTilesetTagsUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditTilesetTagsClearStaticTexture(pApp);
	if ( pApp->pTilesetTagsGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tTilesetTagsGrid);
		pApp->pTilesetTagsGridWidget = NULL;
	}
	if ( pApp->pTilesetTagsCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetTagsScrollFrame);
		pApp->pTilesetTagsCanvasWidget = NULL;
	}
	if ( pApp->pTilesetTagsFormGridWidget != NULL ) {
		xgeXuiPropertyGridUnit(&pApp->tTilesetTagsFormGrid);
		pApp->pTilesetTagsFormGridWidget = NULL;
	}
	if ( pApp->pTilesetTagsInspectCheckWidget != NULL ) {
		xgeXuiCheckBoxUnit(&pApp->tTilesetTagsInspectCheck);
		pApp->pTilesetTagsInspectCheckWidget = NULL;
	}
	if ( pApp->pTilesetTagsChannelComboWidget != NULL ) {
		xgeXuiComboBoxUnit(&pApp->tTilesetTagsChannelCombo);
		pApp->pTilesetTagsChannelComboWidget = NULL;
	}
	pApp->pTilesetTagsControlRowWidget = NULL;
	pApp->iTilesetTagsSelectedTile = -1;
}
