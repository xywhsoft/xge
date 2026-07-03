#include "map_tags.h"
#include "map_maps.h"
#include "../../tileset/tabs/tileset_sets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_MAP_TAGS_FORM_HEIGHT 148.0f
#define MAPEDIT_MAP_TAGS_SCROLLBAR_SIZE 12.0f

static float MapEditMapTagsAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditMapTagsRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditMapTagsAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditMapTagsAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditMapTagsAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditMapTagsAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditMapTagsRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditMapTagsWidgetInteractive(xge_xui_widget pWidget)
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

static int MapEditMapTagsMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static void MapEditMapTagsCopyText(char* sDst, int iDstSize, const char* sSrc)
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

static int MapEditMapTagsIsCellChannel(const mapedit_custom_channel_def_t* pDef)
{
	return pDef != NULL && pDef->sId[0] != 0 &&
		(pDef->sScope[0] == 0 || strcmp(pDef->sScope, "tile") == 0 || strcmp(pDef->sScope, "cell") == 0);
}

static const mapedit_custom_channel_def_t* MapEditMapTagsCurrentChannel(mapedit_app_t* pApp)
{
	return MapEditCustomChannelGet(pApp, (pApp != NULL) ? pApp->iMapTagChannel : -1);
}

static void MapEditMapTagsUseDefaultValue(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;

	if ( pApp == NULL ) {
		return;
	}
	pDef = MapEditMapTagsCurrentChannel(pApp);
	MapEditMapTagsCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), (pDef != NULL) ? pDef->sDefaultValue : "");
}

static int MapEditMapTagsValueAsInt(const char* sValue)
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

static void MapEditMapTagsClampValue(mapedit_app_t* pApp, const mapedit_custom_channel_def_t* pDef)
{
	int iValue;

	if ( pApp == NULL || pDef == NULL ) {
		return;
	}
	if ( !pDef->bHasMin && !pDef->bHasMax ) {
		return;
	}
	iValue = MapEditMapTagsValueAsInt(pApp->sMapTagValue);
	if ( pDef->bHasMin && iValue < pDef->iMinValue ) {
		iValue = pDef->iMinValue;
	}
	if ( pDef->bHasMax && iValue > pDef->iMaxValue ) {
		iValue = pDef->iMaxValue;
	}
	snprintf(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), "%d", iValue);
}

static int MapEditMapTagsEnumFindByValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
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

static int MapEditMapTagsEnumFindByText(const mapedit_custom_channel_def_t* pDef, const char* sText)
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

static const char* MapEditMapTagsEnumDisplayForValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int iIndex;

	iIndex = MapEditMapTagsEnumFindByValue(pDef, sValue);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sText;
	}
	return (sValue != NULL) ? sValue : "";
}

static const char* MapEditMapTagsEnumValueForDisplay(const mapedit_custom_channel_def_t* pDef, const char* sText)
{
	int iIndex;

	iIndex = MapEditMapTagsEnumFindByText(pDef, sText);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sValue;
	}
	iIndex = MapEditMapTagsEnumFindByValue(pDef, sText);
	if ( iIndex >= 0 ) {
		return pDef->arrOptions[iIndex].sValue;
	}
	return (sText != NULL) ? sText : "";
}

static int MapEditMapTagsAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
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

static void MapEditMapTagsSyncFlagProperties(mapedit_app_t* pApp, const mapedit_custom_channel_def_t* pDef)
{
	char sId[32];
	int iValue;
	int iProp;
	int i;

	if ( pApp == NULL || pDef == NULL || pApp->bMapTagsFormUpdating ) {
		return;
	}
	pApp->bMapTagsFormUpdating = 1;
	iValue = MapEditMapTagsValueAsInt(pApp->sMapTagValue);
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sId, sizeof(sId), "map.tag.flag.%d", i);
		iProp = xgeXuiPropertyGridFindProperty(&pApp->tMapTagsFormGrid, sId);
		if ( iProp >= 0 ) {
			xgeXuiPropertyGridSetBool(&pApp->tMapTagsFormGrid, iProp, (iValue & (1 << pDef->arrOptions[i].iBit)) != 0);
		}
	}
	pApp->bMapTagsFormUpdating = 0;
}

static void MapEditMapTagsBuildForm(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;
	xge_xui_property_grid pGrid;
	xge_xui_table_grid_editor_config_t tEnum;
	char sValueDisplay[MAPEDIT_CUSTOM_TEXT_CAPACITY];
	char sFlagId[32];
	int iCategory;
	int iType;
	int iValueProp;
	int i;

	if ( pApp == NULL || pApp->pMapTagsFormGridWidget == NULL ) {
		return;
	}
	pGrid = &pApp->tMapTagsFormGrid;
	pApp->bMapTagsFormUpdating = 1;
	xgeXuiPropertyGridClear(pGrid);
	pDef = MapEditMapTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		iCategory = xgeXuiPropertyGridAddCategory(pGrid, "map.tag.empty", "地图打标", 1);
		if ( iCategory >= 0 ) {
			MapEditMapTagsAddProperty(pGrid, iCategory, "map.tag.empty.hint", "当前通道", "setup.xson 中没有 scope=tile/cell 的 customData 通道", XGE_XUI_TABLE_CELL_TYPE_TEXT, "未配置", "未配置", XGE_XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bMapTagsFormUpdating = 0;
		return;
	}
	if ( pApp->sMapTagValue[0] == 0 && pDef->sDefaultValue[0] != 0 ) {
		MapEditMapTagsCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), pDef->sDefaultValue);
	}
	MapEditMapTagsClampValue(pApp, pDef);
	if ( strcmp(pDef->sMarkMode, "bitmask") == 0 ) {
		MapEditMapTagsSyncFlagProperties(pApp, pDef);
	}
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "map.tag", "地图打标", 1);
	if ( iCategory < 0 ) {
		pApp->bMapTagsFormUpdating = 0;
		return;
	}
	MapEditMapTagsAddProperty(pGrid, iCategory, "map.tag.channel", "通道", pDef->sId, XGE_XUI_TABLE_CELL_TYPE_TEXT, pDef->sName[0] ? pDef->sName : pDef->sId, "", XGE_XUI_PROPERTY_FLAG_READONLY);
	MapEditMapTagsAddProperty(pGrid, iCategory, "map.tag.type", "数据类型", pDef->sMarkMode, XGE_XUI_TABLE_CELL_TYPE_TEXT, pDef->sDataType, "", XGE_XUI_PROPERTY_FLAG_READONLY);
	iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_ENUM;
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), MapEditMapTagsEnumDisplayForValue(pDef, pApp->sMapTagValue));
	} else if ( strcmp(pDef->sDataType, "string") == 0 || strcmp(pDef->sDataType, "text") == 0 || strcmp(pDef->sMarkMode, "text") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sMapTagValue);
	} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_BOOL;
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), MapEditMapTagsValueAsInt(pApp->sMapTagValue) ? "true" : "false");
	} else if ( strcmp(pDef->sDataType, "float") == 0 || strcmp(pDef->sDataType, "double") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_FLOAT;
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sMapTagValue);
	} else if ( strstr(pDef->sDataType, "int") != NULL || strcmp(pDef->sMarkMode, "paint") == 0 ) {
		iType = XGE_XUI_TABLE_CELL_TYPE_INT;
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sMapTagValue);
	} else {
		MapEditMapTagsCopyText(sValueDisplay, sizeof(sValueDisplay), pApp->sMapTagValue);
	}
	iValueProp = MapEditMapTagsAddProperty(pGrid, iCategory, "map.tag.value", pDef->sName[0] ? pDef->sName : "值", pDef->sId, iType, sValueDisplay, pDef->sDefaultValue, 0);
	if ( iValueProp >= 0 && iType == XGE_XUI_TABLE_CELL_TYPE_ENUM ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = (const char**)pDef->arrOptionItems;
		tEnum.iEnumItemCount = pDef->iOptionCount;
		tEnum.iEnumSelected = MapEditMapTagsEnumFindByValue(pDef, pApp->sMapTagValue);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iValueProp, &tEnum);
	}
	if ( strcmp(pDef->sMarkMode, "bitmask") == 0 && pDef->iOptionCount > 0 ) {
		for ( i = 0; i < pDef->iOptionCount; i++ ) {
			snprintf(sFlagId, sizeof(sFlagId), "map.tag.flag.%d", i);
			MapEditMapTagsAddProperty(pGrid, iCategory, sFlagId, pDef->arrOptions[i].sText, "bitmask flag", XGE_XUI_TABLE_CELL_TYPE_BOOL,
				(MapEditMapTagsValueAsInt(pApp->sMapTagValue) & (1 << pDef->arrOptions[i].iBit)) ? "true" : "false", "false", 0);
		}
	}
	xgeXuiPropertyGridSetSelected(pGrid, iValueProp);
	pApp->bMapTagsFormUpdating = 0;
}

static void MapEditMapTagsFormChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp;
	const mapedit_custom_channel_def_t* pDef;
	char sFlagId[32];
	int iValue;
	int i;

	(void)pWidget;
	(void)iType;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pApp->bMapTagsFormUpdating || sId == NULL ) {
		return;
	}
	pDef = MapEditMapTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		return;
	}
	if ( strcmp(sId, "map.tag.value") == 0 ) {
	if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		MapEditMapTagsCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), MapEditMapTagsEnumValueForDisplay(pDef, sValue));
	} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
		MapEditMapTagsCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), MapEditMapTagsValueAsInt(sValue) ? "true" : "false");
	} else {
		MapEditMapTagsCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), sValue);
	}
	MapEditMapTagsClampValue(pApp, pDef);
	if ( strcmp(pDef->sMarkMode, "bitmask") == 0 ) {
		MapEditMapTagsSyncFlagProperties(pApp, pDef);
	}
	MapEditAppSetStatus(pApp, "地图打标值已修改");
	return;
	}
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sFlagId, sizeof(sFlagId), "map.tag.flag.%d", i);
		if ( strcmp(sId, sFlagId) == 0 ) {
			iValue = MapEditMapTagsValueAsInt(pApp->sMapTagValue);
			if ( MapEditMapTagsValueAsInt(sValue) ) {
				iValue |= (1 << pDef->arrOptions[i].iBit);
			} else {
				iValue &= ~(1 << pDef->arrOptions[i].iBit);
			}
			snprintf(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), "%d", iValue);
			iProperty = xgeXuiPropertyGridFindProperty(&pApp->tMapTagsFormGrid, "map.tag.value");
			if ( iProperty >= 0 ) {
				pApp->bMapTagsFormUpdating = 1;
				xgeXuiPropertyGridSetValue(&pApp->tMapTagsFormGrid, iProperty, pApp->sMapTagValue);
				pApp->bMapTagsFormUpdating = 0;
			}
			MapEditAppSetStatus(pApp, "鍦板浘鎵撴爣浣嶅凡淇敼");
			return;
		}
	}
}

static void MapEditMapTagsUpdateChannelCombo(mapedit_app_t* pApp)
{
	const mapedit_custom_channel_def_t* pDef;
	int i;
	int iOut;
	int iSelected;

	if ( pApp == NULL || pApp->pMapTagsChannelComboWidget == NULL ) {
		return;
	}
	iOut = 0;
	iSelected = -1;
	for ( i = 0; i < pApp->iCustomChannelCount && iOut < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( !MapEditMapTagsIsCellChannel(pDef) ) {
			continue;
		}
		snprintf(pApp->arrMapTagChannelText[iOut], sizeof(pApp->arrMapTagChannelText[iOut]), "%s (%s)", pDef->sName[0] ? pDef->sName : pDef->sId, pDef->sId);
		pApp->arrMapTagChannelItems[iOut].sText = pApp->arrMapTagChannelText[iOut];
		pApp->arrMapTagChannelItems[iOut].iValue = i;
		pApp->arrMapTagChannelItems[iOut].bEnabled = 1;
		pApp->arrMapTagChannelItems[iOut].bSeparator = 0;
		pApp->arrMapTagChannelItems[iOut].iIcon = 0;
		pApp->arrMapTagChannelItems[iOut].pUser = NULL;
		if ( pApp->iMapTagChannel == i ) {
			iSelected = iOut;
		}
		iOut++;
	}
	pApp->iMapTagChannelOptionCount = iOut;
	if ( iOut <= 0 ) {
		pApp->iMapTagChannel = -1;
		xgeXuiComboBoxSetItemData(&pApp->tMapTagsChannelCombo, NULL, 0);
		MapEditMapTagsBuildForm(pApp);
		return;
	}
	if ( iSelected < 0 ) {
		iSelected = 0;
		pApp->iMapTagChannel = pApp->arrMapTagChannelItems[0].iValue;
		MapEditMapTagsUseDefaultValue(pApp);
	}
	xgeXuiComboBoxSetItemData(&pApp->tMapTagsChannelCombo, pApp->arrMapTagChannelItems, iOut);
	xgeXuiComboBoxSetSelected(&pApp->tMapTagsChannelCombo, iSelected);
	MapEditMapTagsBuildForm(pApp);
}

static void MapEditMapTagsChannelSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->iMapTagChannelOptionCount ) {
		return;
	}
	pApp->iMapTagChannel = xgeXuiComboBoxGetSelectedValue(&pApp->tMapTagsChannelCombo);
	MapEditMapTagsUseDefaultValue(pApp);
	MapEditMapTagsBuildForm(pApp);
	if ( pApp->pMapTagsGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapTagsGridWidget);
	}
	MapEditAppSetStatus(pApp, "地图打标通道已切换");
}

static void MapEditMapTagsInspectChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->bMapTagsInspectMode = bChecked ? 1 : 0;
	MapEditAppSetStatus(pApp, pApp->bMapTagsInspectMode ? "地图打标：检查模式" : "地图打标：写入模式");
}

static float MapEditMapTagsContentWidth(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iWidth * MapEditMapTileWidth(pApp));
}

static float MapEditMapTagsContentHeight(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iHeight * MapEditMapTileHeight(pApp));
}

static void MapEditMapTagsSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditMapTagsRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditMapTagsRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditMapTagsAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditMapTagsAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditMapTagsUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapTagsScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditMapTagsContentWidth(pApp);
	fContentH = MapEditMapTagsContentHeight(pApp);
	if ( MapEditMapTagsAbsFloat(pApp->tMapTagsScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditMapTagsAbsFloat(pApp->tMapTagsScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tMapTagsScrollFrame, fContentW, fContentH);
	}
}

static void MapEditMapTagsSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapTagsScrollFrame.pWidget == NULL || pApp->pMapTagsGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapTagsScrollFrame);
	fContentW = (pApp->tMapTagsScrollModel.fContentW > 1.0f) ? pApp->tMapTagsScrollModel.fContentW : MapEditMapTagsContentWidth(pApp);
	fContentH = (pApp->tMapTagsScrollModel.fContentH > 1.0f) ? pApp->tMapTagsScrollModel.fContentH : MapEditMapTagsContentHeight(pApp);
	tLocal.fX = -pApp->tMapTagsScrollModel.fScrollX;
	tLocal.fY = -pApp->tMapTagsScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditMapTagsSetScrollContentRect(pApp->pMapTagsGridWidget, tLocal, tRect);
}

static void MapEditMapTagsUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditMapTagsUpdateScrollContentSize(pApp);
	MapEditMapTagsSyncScrollContent(pApp);
}

static void MapEditMapTagsScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditMapTagsSyncScrollContent(pApp);
}

static void MapEditMapTagsScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tMapTagsScrollFrame);
	MapEditMapTagsSyncScrollContent(pApp);
}

static void MapEditMapTagsDrawMarker(mapedit_tile_grid_t* pGrid, xge_rect_t tCell, const char* sValue)
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

static void MapEditMapTagsCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	const mapedit_custom_channel_def_t* pDef;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pGrid == NULL ) {
		return;
	}
	MapEditMapDrawCell(pApp, pGrid, iCol, iRow, tCell);
	pDef = MapEditMapTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		return;
	}
	MapEditMapGetCellCustomValue(pApp, pDef->sId, iCol, iRow, sValue, sizeof(sValue));
	if ( sValue[0] != 0 && strcmp(sValue, pDef->sDefaultValue) != 0 ) {
		MapEditMapTagsDrawMarker(pGrid, tCell, sValue);
	}
}

static void MapEditMapTagsCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	const mapedit_custom_channel_def_t* pDef;
	char sStatus[320];
	int iCellId;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	iCellId = MapEditMapCellId(MapEditMapSelectedItem(pApp), iCol, iRow);
	if ( iCellId < 0 ) {
		MapEditAppSetStatus(pApp, "该位置不在地图范围内");
		return;
	}
	pApp->iMapTagsSelectedCell = iCellId;
	if ( pApp->bMapTagsInspectMode ) {
		MapEditMapPropertiesShowCellCustomData(pApp, iCol, iRow);
		snprintf(sStatus, sizeof(sStatus), "正在检查地图位置自定义数据: (%d,%d)", iCol, iRow);
		MapEditAppSetStatus(pApp, sStatus);
		return;
	}
	pDef = MapEditMapTagsCurrentChannel(pApp);
	if ( pDef == NULL ) {
		MapEditAppSetStatus(pApp, "setup.xson 中没有可写入的地图 customData 通道");
		return;
	}
	if ( MapEditMapSetCellCustomValue(pApp, pDef->sId, iCol, iRow, pApp->sMapTagValue) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "地图打标写入失败");
		return;
	}
	if ( MapEditMapSaveSelected(pApp) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "地图打标已修改，但保存失败");
		return;
	}
	snprintf(sStatus, sizeof(sStatus), "地图打标已保存: (%d,%d) / %s = %s", iCol, iRow, pDef->sId, pApp->sMapTagValue);
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pMapTagsGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapTagsGridWidget);
	}
}

void MapEditMapTagsRefresh(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	int iCols;
	int iRows;

	MapEditMapTagsUpdateChannelCombo(pApp);
	if ( pApp == NULL || pApp->pMapTagsGridWidget == NULL ) {
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
	MapEditTileGridSetCellSize(&pApp->tMapTagsGrid, MapEditMapTileWidth(pApp), MapEditMapTileHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMapTagsGrid, iCols, iRows);
	MapEditTileGridSetMaxCells(&pApp->tMapTagsGrid, iCols * iRows);
	pApp->tMapTagsGrid.iHoverCol = -1;
	pApp->tMapTagsGrid.iHoverRow = -1;
	pApp->tMapTagsGrid.bHasSelection = 0;
	pApp->iMapTagsSelectedCell = -1;
	if ( pApp->tMapTagsScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMapTagsScrollFrame, 0.0f, 0.0f);
		MapEditMapTagsUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pMapTagsCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pMapTagsGridWidget);
}

int MapEditMapTagsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pMapTagsGridWidget == NULL || !MapEditMapTagsMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditMapTagsWidgetInteractive(pApp->pMapTagsGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapTagsScrollFrame);
	if ( pApp->tMapTagsGrid.bDragging ||
	     (MapEditMapTagsRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditMapTagsRectContains(pApp->pMapTagsGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return MapEditTileGridEventProc(pApp->pMapTagsGridWidget, pEvent, &pApp->tMapTagsGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditMapTagsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
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
	xgeXuiWidgetSetSize(pRowWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetPaddingPx(pRowWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pRowWidget, 8.0f);
	xgeXuiWidgetSetSize(pComboWidget, xgeXuiSizePx(260.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetSize(pCheckWidget, xgeXuiSizePx(190.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetSize(pFormWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(MAPEDIT_MAP_TAGS_FORM_HEIGHT));
	xgeXuiWidgetSetSize(pCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiWidgetSetBackground(pFormWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pFormWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	xgeXuiWidgetSetBackground(pCanvasWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pCanvasWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiComboBoxInit(&pApp->tMapTagsChannelCombo, &pApp->tXui, pComboWidget) != XGE_OK ||
	     xgeXuiCheckBoxInit(&pApp->tMapTagsInspectCheck, &pApp->tXui, pCheckWidget) != XGE_OK ||
	     xgeXuiPropertyGridInit(&pApp->tMapTagsFormGrid, &pApp->tXui, pFormWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiComboBoxSetFont(&pApp->tMapTagsChannelCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetSelect(&pApp->tMapTagsChannelCombo, MapEditMapTagsChannelSelect, pApp);
	xgeXuiComboBoxSetPopupMaxHeight(&pApp->tMapTagsChannelCombo, 220.0f);
	xgeXuiComboBoxSetMetrics(&pApp->tMapTagsChannelCombo, 24.0f);
	xgeXuiComboBoxSetColors(&pApp->tMapTagsChannelCombo, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(236, 240, 244, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiCheckBoxSetText(&pApp->tMapTagsInspectCheck, pApp->bFontReady ? &pApp->tFont : NULL, "检查地图位置所有字段");
	xgeXuiCheckBoxSetChange(&pApp->tMapTagsInspectCheck, MapEditMapTagsInspectChange, pApp);
	xgeXuiCheckBoxSetTextColor(&pApp->tMapTagsInspectCheck, XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetFont(&pApp->tMapTagsFormGrid, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPropertyGridSetMetrics(&pApp->tMapTagsFormGrid, 120.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tMapTagsFormGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tMapTagsFormGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tMapTagsFormGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tMapTagsFormGrid, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(164, 206, 236, 255), XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetChange(&pApp->tMapTagsFormGrid, MapEditMapTagsFormChange, pApp);
	xgeXuiScrollModelInit(&pApp->tMapTagsScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tMapTagsScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tMapTagsScrollModel) != XGE_OK ) {
		xgeXuiComboBoxUnit(&pApp->tMapTagsChannelCombo);
		xgeXuiCheckBoxUnit(&pApp->tMapTagsInspectCheck);
		xgeXuiPropertyGridUnit(&pApp->tMapTagsFormGrid);
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMapTagsScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditMapTagsScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapTagsScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tMapTagsScrollFrame, MapEditMapTagsScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMapTagsScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMapTagsScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tMapTagsScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tMapTagsScrollFrame, MAPEDIT_MAP_TAGS_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tMapTagsScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapTagsScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tMapTagsGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tMapTagsScrollFrame);
		xgeXuiComboBoxUnit(&pApp->tMapTagsChannelCombo);
		xgeXuiCheckBoxUnit(&pApp->tMapTagsInspectCheck);
		xgeXuiPropertyGridUnit(&pApp->tMapTagsFormGrid);
		xgeXuiWidgetFree(pRowWidget);
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pCheckWidget);
		xgeXuiWidgetFree(pFormWidget);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tMapTagsGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tMapTagsGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 130),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 80),
		XGE_COLOR_RGBA(31, 75, 112, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tMapTagsGrid, 1, 1);
	MapEditTileGridSetEmptyText(&pApp->tMapTagsGrid, "未选择地图");
	MapEditTileGridSetCellPaint(&pApp->tMapTagsGrid, MapEditMapTagsCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tMapTagsGrid, MapEditMapTagsCellClick, pApp);
	pApp->pMapTagsControlRowWidget = pRowWidget;
	pApp->pMapTagsChannelComboWidget = pComboWidget;
	pApp->pMapTagsInspectCheckWidget = pCheckWidget;
	pApp->pMapTagsFormGridWidget = pFormWidget;
	pApp->pMapTagsCanvasWidget = pCanvasWidget;
	pApp->pMapTagsGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pRowWidget, pComboWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pRowWidget, pCheckWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pRowWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pFormWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditMapTagsRefresh(pApp);
	return XGE_OK;
}

void MapEditMapTagsUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMapTagsGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tMapTagsGrid);
		pApp->pMapTagsGridWidget = NULL;
	}
	if ( pApp->pMapTagsCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tMapTagsScrollFrame);
		pApp->pMapTagsCanvasWidget = NULL;
	}
	if ( pApp->pMapTagsFormGridWidget != NULL ) {
		xgeXuiPropertyGridUnit(&pApp->tMapTagsFormGrid);
		pApp->pMapTagsFormGridWidget = NULL;
	}
	if ( pApp->pMapTagsInspectCheckWidget != NULL ) {
		xgeXuiCheckBoxUnit(&pApp->tMapTagsInspectCheck);
		pApp->pMapTagsInspectCheckWidget = NULL;
	}
	if ( pApp->pMapTagsChannelComboWidget != NULL ) {
		xgeXuiComboBoxUnit(&pApp->tMapTagsChannelCombo);
		pApp->pMapTagsChannelComboWidget = NULL;
	}
	pApp->pMapTagsControlRowWidget = NULL;
	pApp->iMapTagChannel = -1;
	pApp->iMapTagsSelectedCell = -1;
}
