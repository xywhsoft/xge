#include "mapedit_app.h"
#include "mapedit_layout.h"
#include "mapedit_workspace.h"
#include "../workspaces/tileset/tabs/tileset_materials.h"
#include "../workspaces/tileset/tabs/tileset_arrange.h"
#include "../workspaces/tileset/tabs/tileset_sets.h"
#include "../workspaces/tileset/tabs/tileset_passage.h"
#include "../workspaces/tileset/tabs/tileset_actor_overlay.h"
#include "../workspaces/tileset/tabs/tileset_tags.h"
#include "../workspaces/map/tabs/map_maps.h"
#include "../workspaces/map/tabs/map_tile_select.h"
#include "../workspaces/map/tabs/map_edit.h"
#include "../workspaces/map/tabs/map_passage.h"
#include "../workspaces/map/tabs/map_tags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_MAP_PREVIEW_FRAME_SECONDS 0.25f

static xge_xui_menu_item_t g_arrFileItems[] = {
	{ "打开地图", "Ctrl+O", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_OPEN_MAP, 0, NULL, NULL },
	{ "保存地图", "Ctrl+S", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_SAVE_MAP, 0, NULL, NULL },
	{ "另存为", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_SAVE_MAP_AS, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "退出", "Alt+F4", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_EXIT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrViewItems[] = {
	{ "重置布局", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_RESET_LAYOUT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrHelpItems[] = {
	{ "关于", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_ABOUT, 0, NULL, NULL }
};

static int MapEditLoadFont(mapedit_app_t* pApp)
{
	str sXrfPath;

	sXrfPath = xrtPathJoin(2, xCore.AppPath, (str)"res/fonts/simsun12.xrf");
	if ( sXrfPath != NULL ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoadXRF(&pApp->tFont, (const char*)sXrfPath) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("mapedit font loaded: %s\n", sXrfPath);
			xrtFree(sXrfPath);
			return XGE_OK;
		}
		printf("mapedit font load failed: %s\n", sXrfPath);
		xrtFree(sXrfPath);
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xui_texture MapEditLoadXuiTexture(mapedit_app_t* pApp, const char* sFileName)
{
	str sPath;
	xui_texture pTexture;

	if ( (pApp == NULL) || (sFileName == NULL) ) {
		return NULL;
	}
	sPath = xrtPathJoin(2, xCore.AppPath, (str)sFileName);
	if ( sPath == NULL ) {
		return NULL;
	}
	pTexture = NULL;
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, 0, &pTexture) != XGE_OK ) {
		printf("mapedit icon load failed: %s\n", sPath);
		pTexture = NULL;
	} else {
		printf("mapedit icon loaded: %s\n", sPath);
	}
	xrtFree(sPath);
	return pTexture;
}

static int MapEditSetupReadInt(xvalue pTable, const char* sKey, int iDefault)
{
	xvalue pValue;
	int iType;

	if ( (pTable == NULL) || (sKey == NULL) || (xvoType(pTable) != XVO_DT_TABLE) ) {
		return iDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	iType = xvoType(pValue);
	if ( iType == XVO_DT_INT ) {
		return (int)xvoGetInt(pValue);
	}
	if ( iType == XVO_DT_FLOAT ) {
		return (int)xvoGetFloat(pValue);
	}
	return iDefault;
}

static int MapEditSetupReadBool(xvalue pTable, const char* sKey, int bDefault)
{
	xvalue pValue;
	int iType;
	const char* sText;

	if ( (pTable == NULL) || (sKey == NULL) || (xvoType(pTable) != XVO_DT_TABLE) ) {
		return bDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	iType = xvoType(pValue);
	if ( iType == XVO_DT_BOOL ) {
		return xvoGetBool(pValue) ? 1 : 0;
	}
	if ( iType == XVO_DT_INT ) {
		return xvoGetInt(pValue) != 0;
	}
	if ( iType == XVO_DT_FLOAT ) {
		return xvoGetFloat(pValue) != 0.0;
	}
	if ( iType == XVO_DT_TEXT ) {
		sText = (const char*)xvoGetText(pValue);
		if ( sText != NULL ) {
			if ( strcmp(sText, "true") == 0 || strcmp(sText, "1") == 0 || strcmp(sText, "yes") == 0 ) {
				return 1;
			}
			if ( strcmp(sText, "false") == 0 || strcmp(sText, "0") == 0 || strcmp(sText, "no") == 0 ) {
				return 0;
			}
		}
	}
	return bDefault;
}

static const char* MapEditSetupReadText(xvalue pTable, const char* sKey, const char* sDefault)
{
	xvalue pValue;

	if ( (pTable == NULL) || (sKey == NULL) || (xvoType(pTable) != XVO_DT_TABLE) ) {
		return sDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	if ( xvoType(pValue) == XVO_DT_TEXT ) {
		return (const char*)xvoGetText(pValue);
	}
	return sDefault;
}

static void MapEditSetupCopyText(char* sDst, int iDstSize, const char* sSrc)
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

static void MapEditSetupSetDefaultLayers(mapedit_app_t* pApp)
{
	static const char* arrDefaultNames[] = { "地表", "装饰", "上层" };
	int i;

	if ( pApp == NULL ) {
		return;
	}
	pApp->iSetupLayerCount = MAPEDIT_MAP_LAYER_DEFAULT;
	if ( pApp->iSetupLayerCount > MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		pApp->iSetupLayerCount = MAPEDIT_MAP_LAYER_OPTION_MAX;
	}
	for ( i = 0; i < MAPEDIT_MAP_LAYER_OPTION_MAX; i++ ) {
		memset(&pApp->arrSetupLayers[i], 0, sizeof(pApp->arrSetupLayers[i]));
		pApp->arrSetupLayers[i].iId = i;
		pApp->arrSetupLayers[i].bVisible = 1;
		pApp->arrSetupLayers[i].bEditable = 1;
		pApp->arrSetupLayers[i].bAboveActor = 0;
		if ( i < (int)(sizeof(arrDefaultNames) / sizeof(arrDefaultNames[0])) ) {
			MapEditSetupCopyText(pApp->arrSetupLayers[i].sName, sizeof(pApp->arrSetupLayers[i].sName), arrDefaultNames[i]);
		} else {
			snprintf(pApp->arrSetupLayers[i].sName, sizeof(pApp->arrSetupLayers[i].sName), "图层 %d", i + 1);
		}
	}
	if ( pApp->iSetupLayerCount > 2 ) {
		pApp->arrSetupLayers[2].bAboveActor = 1;
	}
}

static void MapEditSetupValueToText(xvalue pValue, char* sDst, int iDstSize, const char* sDefault)
{
	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	if ( pValue == NULL || xvoType(pValue) == XVO_DT_NULL ) {
		MapEditSetupCopyText(sDst, iDstSize, sDefault);
		return;
	}
	switch ( xvoType(pValue) ) {
	case XVO_DT_BOOL:
		MapEditSetupCopyText(sDst, iDstSize, xvoGetBool(pValue) ? "true" : "false");
		break;
	case XVO_DT_INT:
		snprintf(sDst, (size_t)iDstSize, "%lld", (long long)xvoGetInt(pValue));
		break;
	case XVO_DT_FLOAT:
		snprintf(sDst, (size_t)iDstSize, "%.6g", xvoGetFloat(pValue));
		break;
	case XVO_DT_TEXT:
		MapEditSetupCopyText(sDst, iDstSize, (const char*)xvoGetText(pValue));
		break;
	default:
		MapEditSetupCopyText(sDst, iDstSize, sDefault);
		break;
	}
}

static void MapEditSetupReadCustomOptions(mapedit_custom_channel_def_t* pDef, xvalue pOptions, int bFlags)
{
	xvalue pItem;
	xvalue pValue;
	const char* sName;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int iCount;
	int i;
	int iBit;

	if ( pDef == NULL || xvoType(pOptions) != XVO_DT_ARRAY ) {
		return;
	}
	iCount = (int)xvoArrayItemCount(pOptions);
	if ( iCount > MAPEDIT_CUSTOM_OPTION_MAX ) {
		iCount = MAPEDIT_CUSTOM_OPTION_MAX;
	}
	for ( i = 0; i < iCount; i++ ) {
		pItem = xvoArrayGetValue(pOptions, (uint32)i);
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			continue;
		}
		sName = MapEditSetupReadText(pItem, "name", "");
		if ( bFlags ) {
			iBit = MapEditSetupReadInt(pItem, "bit", i);
			snprintf(sValue, sizeof(sValue), "%d", iBit);
			pDef->arrOptions[pDef->iOptionCount].iBit = iBit;
			pDef->arrOptions[pDef->iOptionCount].bHasBit = 1;
		} else {
			pValue = xvoTableGetValue(pItem, "value", 5);
			MapEditSetupValueToText(pValue, sValue, sizeof(sValue), "0");
		}
		if ( sName == NULL || sName[0] == 0 ) {
			sName = sValue;
		}
		MapEditSetupCopyText(pDef->arrOptions[pDef->iOptionCount].sText, sizeof(pDef->arrOptions[pDef->iOptionCount].sText), sName);
		MapEditSetupCopyText(pDef->arrOptions[pDef->iOptionCount].sValue, sizeof(pDef->arrOptions[pDef->iOptionCount].sValue), sValue);
		pDef->arrOptionItems[pDef->iOptionCount] = pDef->arrOptions[pDef->iOptionCount].sText;
		pDef->iOptionCount++;
	}
}

static void MapEditSetupLoadLayers(mapedit_app_t* pApp, xvalue pSetup)
{
	xvalue pLayers;
	xvalue pDefaults;
	xvalue pLayer;
	const char* sName;
	int iCount;
	int i;

	if ( pApp == NULL || pSetup == NULL ) {
		return;
	}
	MapEditSetupSetDefaultLayers(pApp);
	pLayers = xvoTableGetValue(pSetup, "layers", 6);
	if ( xvoType(pLayers) != XVO_DT_TABLE ) {
		return;
	}
	iCount = MapEditSetupReadInt(pLayers, "count", pApp->iSetupLayerCount);
	if ( iCount < 1 ) {
		iCount = 1;
	} else if ( iCount > MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		iCount = MAPEDIT_MAP_LAYER_OPTION_MAX;
	}
	pApp->iSetupLayerCount = iCount;
	pDefaults = xvoTableGetValue(pLayers, "defaults", 8);
	if ( xvoType(pDefaults) != XVO_DT_ARRAY ) {
		return;
	}
	for ( i = 0; i < iCount; i++ ) {
		pLayer = xvoArrayGetValue(pDefaults, (uint32)i);
		if ( xvoType(pLayer) != XVO_DT_TABLE ) {
			continue;
		}
		pApp->arrSetupLayers[i].iId = MapEditSetupReadInt(pLayer, "id", i);
		sName = MapEditSetupReadText(pLayer, "name", pApp->arrSetupLayers[i].sName);
		MapEditSetupCopyText(pApp->arrSetupLayers[i].sName, sizeof(pApp->arrSetupLayers[i].sName), sName);
		pApp->arrSetupLayers[i].bVisible = MapEditSetupReadBool(pLayer, "visible", pApp->arrSetupLayers[i].bVisible);
		pApp->arrSetupLayers[i].bEditable = MapEditSetupReadBool(pLayer, "editable", pApp->arrSetupLayers[i].bEditable);
		pApp->arrSetupLayers[i].bAboveActor = MapEditSetupReadBool(pLayer, "aboveActor", pApp->arrSetupLayers[i].bAboveActor);
	}
}

static void MapEditSetupLoadCustomData(mapedit_app_t* pApp, xvalue pSetup)
{
	xvalue pCustomData;
	xvalue pChannels;
	xvalue pChannel;
	xvalue pDefault;
	mapedit_custom_channel_def_t* pDef;
	const char* sId;
	const char* sName;
	int iCount;
	int i;

	if ( pApp == NULL || pSetup == NULL ) {
		return;
	}
	pApp->iCustomChannelCount = 0;
	pCustomData = xvoTableGetValue(pSetup, "customData", 10);
	if ( xvoType(pCustomData) != XVO_DT_TABLE ) {
		return;
	}
	pChannels = xvoTableGetValue(pCustomData, "channels", 8);
	if ( xvoType(pChannels) != XVO_DT_ARRAY ) {
		return;
	}
	iCount = (int)xvoArrayItemCount(pChannels);
	if ( iCount > MAPEDIT_CUSTOM_CHANNEL_MAX ) {
		iCount = MAPEDIT_CUSTOM_CHANNEL_MAX;
	}
	for ( i = 0; i < iCount; i++ ) {
		pChannel = xvoArrayGetValue(pChannels, (uint32)i);
		if ( xvoType(pChannel) != XVO_DT_TABLE ) {
			continue;
		}
		sId = MapEditSetupReadText(pChannel, "id", "");
		if ( sId == NULL || sId[0] == 0 ) {
			continue;
		}
		pDef = &pApp->arrCustomChannels[pApp->iCustomChannelCount];
		memset(pDef, 0, sizeof(*pDef));
		MapEditSetupCopyText(pDef->sId, sizeof(pDef->sId), sId);
		sName = MapEditSetupReadText(pChannel, "name", sId);
		MapEditSetupCopyText(pDef->sName, sizeof(pDef->sName), sName);
		MapEditSetupCopyText(pDef->sScope, sizeof(pDef->sScope), MapEditSetupReadText(pChannel, "scope", "tile"));
		MapEditSetupCopyText(pDef->sDataType, sizeof(pDef->sDataType), MapEditSetupReadText(pChannel, "dataType", "int"));
		MapEditSetupCopyText(pDef->sMarkMode, sizeof(pDef->sMarkMode), MapEditSetupReadText(pChannel, "markMode", "paint"));
		pDefault = xvoTableGetValue(pChannel, "defaultValue", 12);
		MapEditSetupValueToText(pDefault, pDef->sDefaultValue, sizeof(pDef->sDefaultValue), "");
		pDef->iMinValue = MapEditSetupReadInt(pChannel, "minValue", 0);
		pDef->iMaxValue = MapEditSetupReadInt(pChannel, "maxValue", 0);
		pDef->bHasMin = xvoType(xvoTableGetValue(pChannel, "minValue", 8)) != XVO_DT_NULL;
		pDef->bHasMax = xvoType(xvoTableGetValue(pChannel, "maxValue", 8)) != XVO_DT_NULL;
		MapEditSetupReadCustomOptions(pDef, xvoTableGetValue(pChannel, "values", 6), 0);
		MapEditSetupReadCustomOptions(pDef, xvoTableGetValue(pChannel, "flags", 5), 1);
		pApp->iCustomChannelCount++;
	}
}

static int MapEditLoadSetup(mapedit_app_t* pApp)
{
	str sPath;
	xvalue pSetup;
	xvalue pTile;
	xvalue pState;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPath = xrtPathJoin(2, xCore.AppPath, (str)"option/setup.xson");
	if ( sPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSetup = xrtParseXSON_File(sPath);
	if ( pSetup == NULL ) {
		printf("mapedit setup load failed: %s\n", sPath);
		xrtFree(sPath);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pApp->bSetupLoaded = 1;
	pTile = xvoTableGetValue(pSetup, "tile", 4);
	pApp->iSetupTileWidth = MapEditSetupReadInt(pTile, "width", pApp->iSetupTileWidth);
	pApp->iSetupTileHeight = MapEditSetupReadInt(pTile, "height", pApp->iSetupTileHeight);
	pApp->iSetupTilesPerRow = MapEditSetupReadInt(pTile, "tilesPerRow", pApp->iSetupTilesPerRow);
	if ( pApp->iSetupTileWidth <= 0 ) {
		pApp->iSetupTileWidth = 16;
	}
	if ( pApp->iSetupTileHeight <= 0 ) {
		pApp->iSetupTileHeight = 16;
	}
	if ( pApp->iSetupTilesPerRow <= 0 ) {
		pApp->iSetupTilesPerRow = 20;
	}
	pState = xvoTableGetValue(pSetup, "state", 5);
	pApp->iSetupStateMin = MapEditSetupReadInt(pState, "min", pApp->iSetupStateMin);
	pApp->iSetupStateMax = MapEditSetupReadInt(pState, "max", pApp->iSetupStateMax);
	if ( pApp->iSetupStateMax < pApp->iSetupStateMin ) {
		pApp->iSetupStateMax = pApp->iSetupStateMin;
	}
	MapEditSetupLoadLayers(pApp, pSetup);
	MapEditSetupLoadCustomData(pApp, pSetup);
	printf("mapedit setup loaded: %s tile=%dx%d tilesPerRow=%d layers=%d state=%d..%d\n", sPath, pApp->iSetupTileWidth, pApp->iSetupTileHeight, pApp->iSetupTilesPerRow, pApp->iSetupLayerCount, pApp->iSetupStateMin, pApp->iSetupStateMax);
	xvoUnref(pSetup);
	xrtFree(sPath);
	return XGE_OK;
}

static void MapEditStyleWorkspaceButton(xge_xui_button pButton, int bSelected);

static void MapEditSelectWorkspace(mapedit_app_t* pApp, int iWorkspace)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditStyleWorkspaceButton(&pApp->tTilesetEditorButton, iWorkspace == MAPEDIT_WORKSPACE_TILESET);
	MapEditStyleWorkspaceButton(&pApp->tMapEditorButton, iWorkspace == MAPEDIT_WORKSPACE_MAP);
	if ( pApp->iWorkspace == iWorkspace ) {
		return;
	}
	pApp->iWorkspace = iWorkspace;
	MapEditWorkspacesSelect(pApp, iWorkspace);
	if ( iWorkspace == MAPEDIT_WORKSPACE_TILESET ) {
		MapEditAppSetStatus(pApp, "当前工作区: 图块编辑器");
	} else {
		MapEditAppSetStatus(pApp, "当前工作区: 地图编辑器");
	}
}

static void MapEditStyleWorkspaceButton(xge_xui_button pButton, int bSelected)
{
	int iVisualState;

	if ( pButton == NULL ) {
		return;
	}
	xgeXuiButtonSetColors(pButton,
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(255, 255, 255, 0),
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(224, 243, 253, 255),
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(224, 243, 253, 255),
		XGE_COLOR_RGBA(54, 118, 178, 220),
		XGE_COLOR_RGBA(255, 255, 255, 0));
	xgeXuiWidgetSetBorder(pButton->pWidget, bSelected ? 1.0f : 0.0f, bSelected ? XGE_COLOR_RGBA(54, 118, 178, 220) : XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_HOVER, 1.0f, XGE_COLOR_RGBA(54, 118, 178, 220));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_ACTIVE, 1.0f, XGE_COLOR_RGBA(54, 118, 178, 220));
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_CHECKED,
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(255, 255, 255, 0));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_CHECKED, 0.0f, XGE_COLOR_RGBA(0, 0, 0, 0));
	pButton->iColorChecked = bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(255, 255, 255, 0);
	pButton->bSelected = bSelected ? 1 : 0;
	pButton->bSelectable = 0;
	iVisualState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED);
	if ( bSelected ) {
		iVisualState |= XGE_XUI_STATE_CHECKED;
	}
	xgeXuiWidgetSetVisualState(pButton->pWidget, iVisualState);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

static int MapEditInitWorkspaceBar(mapedit_app_t* pApp)
{
	xge_rect_t tIconSrc;

	if ( (pApp == NULL) || (pApp->pTilesetEditorButtonWidget == NULL) || (pApp->pMapEditorButtonWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiButtonInit(&pApp->tTilesetEditorButton, &pApp->tXui, pApp->pTilesetEditorButtonWidget) != XGE_OK ||
		xgeXuiButtonInit(&pApp->tMapEditorButton, &pApp->tXui, pApp->pMapEditorButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pTilesetEditorIcon = MapEditLoadXuiTexture(pApp, "res/icon_tileset_editor.png");
	pApp->pMapEditorIcon = MapEditLoadXuiTexture(pApp, "res/icon_map_editor.png");
	tIconSrc = (xge_rect_t){ 0.0f, 0.0f, 24.0f, 24.0f };
	xgeXuiButtonSetText(&pApp->tTilesetEditorButton, pApp->bFontReady ? &pApp->tFont : NULL, "图块编辑器");
	xgeXuiButtonSetText(&pApp->tMapEditorButton, pApp->bFontReady ? &pApp->tFont : NULL, "地图编辑器");
	MapEditStyleWorkspaceButton(&pApp->tTilesetEditorButton, 0);
	MapEditStyleWorkspaceButton(&pApp->tMapEditorButton, 0);
	xgeXuiButtonSetIcon(&pApp->tTilesetEditorButton, pApp->pTilesetEditorIcon, tIconSrc);
	xgeXuiButtonSetIcon(&pApp->tMapEditorButton, pApp->pMapEditorIcon, tIconSrc);
	xgeXuiButtonSetIconLayout(&pApp->tTilesetEditorButton, XGE_XUI_BUTTON_ICON_TOP, 24.0f, 3.0f);
	xgeXuiButtonSetIconLayout(&pApp->tMapEditorButton, XGE_XUI_BUTTON_ICON_TOP, 24.0f, 3.0f);
	xgeXuiButtonSetTextColor(&pApp->tTilesetEditorButton, XGE_COLOR_RGBA(34, 74, 102, 255));
	xgeXuiButtonSetTextColor(&pApp->tMapEditorButton, XGE_COLOR_RGBA(34, 74, 102, 255));
	xgeXuiButtonSetIconColor(&pApp->tTilesetEditorButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetIconColor(&pApp->tMapEditorButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetSelectable(&pApp->tTilesetEditorButton, 0);
	xgeXuiButtonSetSelectable(&pApp->tMapEditorButton, 0);
	xgeXuiButtonSetClick(&pApp->tTilesetEditorButton, MapEditAppWorkspaceClick, pApp);
	xgeXuiButtonSetClick(&pApp->tMapEditorButton, MapEditAppWorkspaceClick, pApp);
	MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	return XGE_OK;
}

static int MapEditInitMenus(mapedit_app_t* pApp)
{
	if ( xgeXuiMenuInit(&pApp->tFileMenu, &pApp->tXui) != XGE_OK ||
		xgeXuiMenuInit(&pApp->tViewMenu, &pApp->tXui) != XGE_OK ||
		xgeXuiMenuInit(&pApp->tHelpMenu, &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuSetItems(&pApp->tFileMenu, g_arrFileItems, (int)(sizeof(g_arrFileItems) / sizeof(g_arrFileItems[0])));
	xgeXuiMenuSetItems(&pApp->tViewMenu, g_arrViewItems, (int)(sizeof(g_arrViewItems) / sizeof(g_arrViewItems[0])));
	xgeXuiMenuSetItems(&pApp->tHelpMenu, g_arrHelpItems, (int)(sizeof(g_arrHelpItems) / sizeof(g_arrHelpItems[0])));
	if ( xgeXuiMenuBarInit(&pApp->tMenuBar, &pApp->tXui, pApp->pMenuWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuBarSetFont(&pApp->tMenuBar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiMenuBarSetSelect(&pApp->tMenuBar, MapEditAppMenuSelect, pApp);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "文件", &pApp->tFileMenu, 1);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "视图", &pApp->tViewMenu, 2);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "帮助", &pApp->tHelpMenu, 3);
	return XGE_OK;
}

static int MapEditInitStatusBar(mapedit_app_t* pApp)
{
	if ( xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "坐标: 0,0", 148.0f, 0);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "地图: 100 x 100", 132.0f, 0);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "图块: tileset 0 / tile 0 / layer 0", 230.0f, 0);
	xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "位置: passable / region 0 / event none", 280.0f, 0);
	MapEditAppSetStatus(pApp, "就绪");
	return XGE_OK;
}

int MapEditAppInit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pApp, 0, sizeof(*pApp));
	pApp->iWorkspace = -1;
	pApp->iSetupTileWidth = 16;
	pApp->iSetupTileHeight = 16;
	pApp->iSetupTilesPerRow = 20;
	pApp->iSetupStateMin = 0;
	pApp->iSetupStateMax = 3;
	MapEditSetupSetDefaultLayers(pApp);
	pApp->iTilesetArrangeSelectedTile = -1;
	pApp->iTilesetPassageSelectedTile = -1;
	pApp->iTilesetActorOverlaySelectedTile = -1;
	pApp->iTilesetTagsSelectedTile = -1;
	pApp->iTilesetTagChannel = -1;
	MapEditSetupCopyText(pApp->sTilesetTagValue, sizeof(pApp->sTilesetTagValue), "");
	pApp->iMapTagChannel = -1;
	pApp->iMapPassageSelectedCell = -1;
	pApp->iMapTagsSelectedCell = -1;
	MapEditSetupCopyText(pApp->sMapTagValue, sizeof(pApp->sMapTagValue), "");
	pApp->iMapSelected = -1;
	pApp->iMapBrushTileId = -1;
	pApp->iMapBrushCols = 1;
	pApp->iMapBrushRows = 1;
	pApp->iMapSelectedTool = MAPEDIT_MAP_TOOL_BRUSH;
	pApp->iMapActiveLayer = 0;
	pApp->bMapShowGrid = 1;
	pApp->bMapPreviewMode = 0;
	pApp->fMapPreviewAnimTime = 0.0f;
	pApp->iMapPreviewAnimFrame = 0;
	xgeMapSetDefault(&pApp->tMap);
	return XGE_OK;
}

void MapEditAppUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	xgeMapUnit(&pApp->tMap);
}

int MapEditAppEnter(xge_scene pScene)
{
	mapedit_app_t* pApp;
	xge_xui_widget pRoot;

	pApp = (mapedit_app_t*)pScene->pUser;
	MapEditLoadSetup(pApp);
	MapEditLoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		printf("mapedit enter failed: xgeXuiInit\n");
		return XGE_ERROR;
	}
	if ( pApp->bFontReady ) {
		xgeXuiSetDefaultFont(&pApp->tXui, &pApp->tFont);
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(226, 235, 242, 255));
	if ( MapEditLayoutBuild(pApp) != XGE_OK ) {
		printf("mapedit enter failed: MapEditLayoutBuild\n");
		return XGE_ERROR;
	}
	if ( MapEditInitMenus(pApp) != XGE_OK || MapEditInitWorkspaceBar(pApp) != XGE_OK || MapEditInitStatusBar(pApp) != XGE_OK ) {
		printf("mapedit enter failed: menu/workspace/status init\n");
		return XGE_ERROR;
	}
	if ( MapEditLayoutLoadUser(pApp) == XGE_OK ) {
		pApp->bLayoutLoaded = 1;
		MapEditAppSetStatus(pApp, "已加载工作区 DockPanel 布局");
	} else if ( MapEditLayoutLoadDefault(pApp) == XGE_OK ) {
		pApp->bLayoutLoaded = 1;
		MapEditAppSetStatus(pApp, "已加载默认 DockPanel 布局");
	} else {
		MapEditAppSetStatus(pApp, "默认布局不可用，使用内置布局");
	}
	pApp->bCreateOK = 1;
	return XGE_OK;
}

int MapEditAppLeave(xge_scene pScene)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	if ( pApp != NULL ) {
		MapEditLayoutSaveUser(pApp);
		xgeXuiButtonUnit(&pApp->tTilesetEditorButton);
		xgeXuiButtonUnit(&pApp->tMapEditorButton);
		if ( pApp->pTilesetEditorIcon != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetEditorIcon);
			pApp->pTilesetEditorIcon = NULL;
		}
		if ( pApp->pMapEditorIcon != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->pMapEditorIcon);
			pApp->pMapEditorIcon = NULL;
		}
		if ( pApp->pMaterialPreviewTexture != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialPreviewTexture);
			pApp->pMaterialPreviewTexture = NULL;
		}
		if ( pApp->pMaterialListWidget != NULL ) {
			xgeXuiListViewUnit(&pApp->tMaterialList);
			pApp->pMaterialListWidget = NULL;
		}
		if ( pApp->pMaterialCreateButtonWidget != NULL ) {
			xgeXuiButtonUnit(&pApp->tMaterialCreateButton);
			pApp->pMaterialCreateButtonWidget = NULL;
		}
		if ( pApp->pMaterialRenameWidget != NULL ) {
			xgeXuiInputBoxUnit(&pApp->tMaterialRenameBox);
			xgeXuiWidgetFree(pApp->pMaterialRenameWidget);
			pApp->pMaterialRenameWidget = NULL;
		}
		if ( pApp->pMaterialViewWindowWidget != NULL ) {
			if ( pApp->pMaterialViewTexture != NULL ) {
				xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialViewTexture);
				pApp->pMaterialViewTexture = NULL;
			}
			xgeXuiScrollViewUnit(&pApp->tMaterialViewScroll);
			xgeXuiWindowUnit(&pApp->tMaterialViewWindow);
			xgeXuiWidgetFree(pApp->pMaterialViewWindowWidget);
			pApp->pMaterialViewWindowWidget = NULL;
			pApp->pMaterialViewScrollWidget = NULL;
			pApp->pMaterialViewImageWidget = NULL;
		}
		if ( pApp->pMaterialEditWindowWidget != NULL ) {
			xgeXuiMsgTipUnit(&pApp->tMaterialEditMsgTip);
			xgeXuiButtonUnit(&pApp->tMaterialEditCancelButton);
			xgeXuiButtonUnit(&pApp->tMaterialEditOkButton);
			xgeXuiButtonUnit(&pApp->tMaterialEditLoadSourceButton);
			xgeXuiInputUnit(&pApp->tMaterialEditFileInput);
			xgeXuiInputUnit(&pApp->tMaterialEditNameInput);
			xgeXuiLabelUnit(&pApp->tMaterialEditFileLabel);
			xgeXuiLabelUnit(&pApp->tMaterialEditNameLabel);
			xgeXuiLabelUnit(&pApp->tMaterialEditSourceTitleLabel);
			xgeXuiLabelUnit(&pApp->tMaterialEditOriginalTitleLabel);
			if ( pApp->pMaterialEditSourceTexture != NULL ) {
				xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialEditSourceTexture);
				pApp->pMaterialEditSourceTexture = NULL;
			}
			if ( pApp->pMaterialEditOutputTexture != NULL ) {
				xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialEditOutputTexture);
				pApp->pMaterialEditOutputTexture = NULL;
			}
			xgeImageFree(&pApp->tMaterialEditSourceImage);
			free(pApp->pMaterialEditOutputPixels);
			pApp->pMaterialEditOutputPixels = NULL;
			MapEditTileGridUnit(&pApp->tMaterialEditSourceGrid);
			MapEditTileGridUnit(&pApp->tMaterialEditOriginalGrid);
			xgeXuiScrollFrameUnit(&pApp->tMaterialEditSourceScrollFrame);
			xgeXuiScrollFrameUnit(&pApp->tMaterialEditOriginalScrollFrame);
			xgeXuiSplitLayoutUnit(&pApp->tMaterialEditSplit);
			xgeXuiWindowUnit(&pApp->tMaterialEditWindow);
			xgeXuiWidgetFree(pApp->pMaterialEditWindowWidget);
			pApp->pMaterialEditWindowWidget = NULL;
			pApp->pMaterialEditMsgTipWidget = NULL;
			pApp->bMaterialEditReady = 0;
		}
		if ( pApp->pMaterialComboWidget != NULL ) {
			xgeXuiComboBoxUnit(&pApp->tMaterialCombo);
			pApp->pMaterialComboWidget = NULL;
		}
		MapEditTilesetArrangeUnit(pApp);
		MapEditTilesetPassageUnit(pApp);
		MapEditTilesetActorOverlayUnit(pApp);
		MapEditTilesetTagsUnit(pApp);
		MapEditTilesetSetsUnit(pApp);
		MapEditMapEditUnit(pApp);
		MapEditMapPassageUnit(pApp);
		MapEditMapTileSelectUnit(pApp);
		MapEditMapTagsUnit(pApp);
		MapEditMapListUnit(pApp);
		xgeXuiMenuUnit(&pApp->tMaterialContextMenu);
		MapEditWorkspacesUnit(pApp);
		xgeXuiStatusBarUnit(&pApp->tStatusBar);
		xgeXuiMenuBarUnit(&pApp->tMenuBar);
		xgeXuiMenuUnit(&pApp->tFileMenu);
		xgeXuiMenuUnit(&pApp->tViewMenu);
		xgeXuiMenuUnit(&pApp->tHelpMenu);
		xgeXuiUnit(&pApp->tXui);
		if ( pApp->bFontReady ) {
			xgeFontFree(&pApp->tFont);
		}
	}
	return XGE_OK;
}

void MapEditAppWorkspaceClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pWidget == pApp->pTilesetEditorButtonWidget ) {
		MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	} else if ( pWidget == pApp->pMapEditorButtonWidget ) {
		MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_MAP);
	}
}

int MapEditAppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	mapedit_app_t* pApp;
	int iResult;

	pApp = (mapedit_app_t*)pScene->pUser;
	if ( MapEditTilesetMaterialsHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditTilesetArrangeHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditTilesetPassageHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditTilesetActorOverlayHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditTilesetTagsHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditMapTileSelectHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditMapEditHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditMapPassageHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( MapEditMapTagsHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	iResult = xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	return XGE_OK;
}

int MapEditAppUpdate(xge_scene pScene, float fDelta)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	if ( pApp->bMapPreviewMode && pApp->iWorkspace == MAPEDIT_WORKSPACE_MAP ) {
		pApp->fMapPreviewAnimTime += fDelta;
		if ( pApp->fMapPreviewAnimTime >= MAPEDIT_MAP_PREVIEW_FRAME_SECONDS ) {
			int iStep;

			iStep = (int)(pApp->fMapPreviewAnimTime / MAPEDIT_MAP_PREVIEW_FRAME_SECONDS);
			if ( iStep < 1 ) {
				iStep = 1;
			}
			pApp->fMapPreviewAnimTime -= (float)iStep * MAPEDIT_MAP_PREVIEW_FRAME_SECONDS;
			if ( pApp->fMapPreviewAnimTime < 0.0f ) {
				pApp->fMapPreviewAnimTime = 0.0f;
			}
			pApp->iMapPreviewAnimFrame = (pApp->iMapPreviewAnimFrame + iStep) & 0x3fffffff;
			if ( pApp->pMapEditGridWidget != NULL ) {
				xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
			}
		}
		xgeRenderRequest();
	}
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		MapEditLayoutSaveUser(pApp);
		printf("mapedit final-summary frames=%d create=%d setupLoaded=%d layoutLoaded=%d layoutSaved=%d dockReady=%d\n",
			pApp->iFrameCount, pApp->bCreateOK, pApp->bSetupLoaded, pApp->bLayoutLoaded, pApp->bLayoutSaved, pApp->bDockReady);
		xgeQuit();
	}
	return XGE_OK;
}

int MapEditAppDraw(xge_scene pScene)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(226, 235, 242, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

void MapEditAppSetStatus(mapedit_app_t* pApp, const char* sText)
{
	if ( (pApp == NULL) || (sText == NULL) ) {
		return;
	}
	snprintf(pApp->sStatus, sizeof(pApp->sStatus), "%s", sText);
	if ( pApp->tStatusBar.iItemCount > 0 ) {
		xgeXuiStatusBarSetItemText(&pApp->tStatusBar, 0, pApp->sStatus);
	}
}

void MapEditAppMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	switch ( iValue ) {
	case MAPEDIT_CMD_OPEN_MAP:
		MapEditAppSetStatus(pApp, "打开地图: 占位命令");
		break;
	case MAPEDIT_CMD_SAVE_MAP:
		MapEditLayoutSaveUser(pApp);
		MapEditAppSetStatus(pApp, "保存地图: 占位命令，DockPanel 布局已保存");
		break;
	case MAPEDIT_CMD_SAVE_MAP_AS:
		MapEditAppSetStatus(pApp, "另存为: 占位命令");
		break;
	case MAPEDIT_CMD_RESET_LAYOUT:
		if ( MapEditLayoutReset(pApp) == XGE_OK ) {
			MapEditLayoutSaveUser(pApp);
			MapEditAppSetStatus(pApp, "已重置并保存默认 DockPanel 布局");
		} else {
			MapEditAppSetStatus(pApp, "重置 DockPanel 布局失败");
		}
		break;
	case MAPEDIT_CMD_ABOUT:
		MapEditAppSetStatus(pApp, "XGE MapEdit - 通用地图编辑器框架");
		break;
	case MAPEDIT_CMD_EXIT:
		xgeQuit();
		break;
	default:
		MapEditAppSetStatus(pApp, "未知菜单命令");
		break;
	}
}
