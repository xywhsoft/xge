#include "tileset_materials.h"
#include "../../../ui/mapedit_tile_grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#define MAPEDIT_TILE_EDIT_MODE_STATIC 0
#define MAPEDIT_TILE_EDIT_MODE_OTHER 1
#define MAPEDIT_MATERIAL_STATIC_COLS 20
#define MAPEDIT_MATERIAL_INITIAL_ROWS 64
#define MAPEDIT_MATERIAL_EXTEND_ROWS 32
#define MAPEDIT_MATERIAL_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_MATERIAL_STATIC_PANE_WIDTH 356.0f

typedef struct mapedit_material_category_t {
	const char* sTitle;
	const char* sDir;
	const char* sMapFile;
} mapedit_material_category_t;

typedef struct mapedit_material_scan_t {
	mapedit_app_t* pApp;
	xvalue pMap;
	int bChanged;
	int bCollectItems;
	int iDiscoveredCount;
	int iSkippedCount;
} mapedit_material_scan_t;

enum {
	MAPEDIT_MATERIAL_MENU_RENAME = 1,
	MAPEDIT_MATERIAL_MENU_EDIT = 2,
	MAPEDIT_MATERIAL_MENU_VIEW = 3
};

static const mapedit_material_category_t g_arrMaterialCategories[] = {
	{ "静态图块", "tilesheets", "tilesheets.xson" },
	{ "动态图块", "animated_tiles", "animated_tiles.xson" },
	{ "自动图块", "autotiles", "autotiles.xson" },
	{ "多状态图块", "state_tiles", "state_tiles.xson" },
	{ "多状态自动图块", "state_autotiles", "state_autotiles.xson" },
	{ "对象", "objects", "objects.xson" }
};

static const char* g_arrMaterialCategoryNames[] = {
	"静态图块",
	"动态图块",
	"自动图块",
	"多状态图块",
	"多状态自动图块",
	"对象"
};

static int MapEditMaterialTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser);
static xvalue MapEditMaterialLoadMap(const char* sPath);
static int MapEditMaterialIsImageFile(const char* sPath);
static void MapEditMaterialCreateClick(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialMenuSelect(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser);
static int MapEditMaterialListDoubleClick(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void MapEditMaterialRenameSubmit(xge_xui_widget pWidget, const char* sText, void* pUser);
static int MapEditMaterialEditWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void MapEditMaterialEditWindowPaintBefore(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialEditCloseClick(xge_xui_widget pWidget, void* pUser);
static int MapEditMaterialViewWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void MapEditMaterialViewCloseClick(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialViewImagePaint(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialEditLoadSourceClick(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialEditOkClick(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialEditSourceSelectionChange(mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser);
static void MapEditMaterialEditOutputCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser);
static void MapEditMaterialEditScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser);
static void MapEditMaterialEditScrollLayoutProc(xge_xui_widget pWidget, void* pUser);
static void MapEditMaterialEditSyncScrollContent(mapedit_app_t* pApp, int bSource);
static void MapEditMaterialEditUpdateAndSyncScrollContent(mapedit_app_t* pApp, int bSource);
static void MapEditMaterialEditSyncAllScrollContent(mapedit_app_t* pApp);
static int MapEditMaterialEditEnsureOutput(mapedit_app_t* pApp, int iCols, int iRows);

static char* MapEditMaterialCopyText(const char* sText)
{
	size_t iLen;
	char* sCopy;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	sCopy = (char*)malloc(iLen + 1);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen + 1);
	return sCopy;
}

static void MapEditMaterialCopyField(char* sDst, int iDstSize, const char* sSrc)
{
	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	snprintf(sDst, (size_t)iDstSize, "%s", sSrc);
}

static int MapEditMaterialCategoryClamp(int iCategory)
{
	int iCount;

	iCount = (int)(sizeof(g_arrMaterialCategories) / sizeof(g_arrMaterialCategories[0]));
	if ( iCategory < 0 || iCategory >= iCount ) {
		return 0;
	}
	return iCategory;
}

static int MapEditMaterialTileWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

static int MapEditMaterialTileHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

static int MapEditMaterialStaticCols(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return MAPEDIT_MATERIAL_STATIC_COLS;
	}
	return pApp->iSetupTilesPerRow;
}

static int MapEditMaterialRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditMaterialWidgetIsInteractive(xge_xui_widget pWidget)
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

static int MapEditMaterialEditMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static int MapEditMaterialPointEvent(int iType)
{
	return (iType == XGE_EVENT_MOUSE_DOWN) || (iType == XGE_EVENT_MOUSE_UP) || (iType == XGE_EVENT_MOUSE_MOVE) ||
		(iType == XGE_EVENT_MOUSE_WHEEL) || (iType == XGE_EVENT_TOUCH_BEGIN) || (iType == XGE_EVENT_TOUCH_MOVE) ||
		(iType == XGE_EVENT_TOUCH_END) || (iType == XGE_EVENT_TOUCH_CANCEL);
}

static void MapEditMaterialPreviewClear(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMaterialPreviewTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialPreviewTexture);
		pApp->pMaterialPreviewTexture = NULL;
	}
	memset(&pApp->tMaterialPreviewDesc, 0, sizeof(pApp->tMaterialPreviewDesc));
	pApp->iMaterialPreviewIndex = -1;
}

static void MapEditMaterialViewClear(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMaterialViewTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialViewTexture);
		pApp->pMaterialViewTexture = NULL;
	}
	memset(&pApp->tMaterialViewTextureDesc, 0, sizeof(pApp->tMaterialViewTextureDesc));
	if ( pApp->pMaterialViewImageWidget != NULL ) {
		xgeXuiWidgetSetRect(pApp->pMaterialViewImageWidget, (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f });
		xgeXuiWidgetSetSize(pApp->pMaterialViewImageWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
		xgeXuiWidgetMarkPaint(pApp->pMaterialViewImageWidget);
	}
	if ( pApp->pMaterialViewScrollWidget != NULL ) {
		xgeXuiScrollViewSetContentSize(&pApp->tMaterialViewScroll, 1.0f, 1.0f);
		xgeXuiScrollViewSetOffset(&pApp->tMaterialViewScroll, 0.0f, 0.0f);
	}
}

static str MapEditMaterialAssetPath(const char* sSubPath)
{
	return xrtPathJoin(3, xCore.AppPath, (str)"assets", (str)sSubPath);
}

static str MapEditMaterialCurrentMapPath(mapedit_app_t* pApp)
{
	int iCategory;

	if ( pApp == NULL ) {
		return NULL;
	}
	iCategory = MapEditMaterialCategoryClamp(pApp->iMaterialCategory);
	return MapEditMaterialAssetPath(g_arrMaterialCategories[iCategory].sMapFile);
}

static str MapEditMaterialCurrentDirPath(mapedit_app_t* pApp)
{
	int iCategory;

	if ( pApp == NULL ) {
		return NULL;
	}
	iCategory = MapEditMaterialCategoryClamp(pApp->iMaterialCategory);
	return MapEditMaterialAssetPath(g_arrMaterialCategories[iCategory].sDir);
}

static int MapEditMaterialFileHasImageExt(const char* sFile)
{
	return MapEditMaterialIsImageFile(sFile);
}

static int MapEditMaterialBuildOutputFile(char* sDst, int iDstSize, const char* sFile)
{
	const char* s;

	if ( sDst == NULL || iDstSize <= 0 || sFile == NULL || sFile[0] == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( s = sFile; *s != 0; s++ ) {
		if ( *s == '\\' || *s == '/' || *s == ':' ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( MapEditMaterialFileHasImageExt(sFile) ) {
		snprintf(sDst, (size_t)iDstSize, "%s", sFile);
	} else {
		snprintf(sDst, (size_t)iDstSize, "%s.png", sFile);
	}
	return XGE_OK;
}

static void MapEditMaterialBuildBackupName(char* sDst, int iDstSize, const char* sFile, int iIndex)
{
	const char* sExt;
	int iBaseLen;

	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	sDst[0] = 0;
	if ( sFile == NULL || sFile[0] == 0 ) {
		snprintf(sDst, (size_t)iDstSize, "backup_%d.png", iIndex);
		return;
	}
	sExt = strrchr(sFile, '.');
	if ( sExt == NULL || sExt == sFile ) {
		snprintf(sDst, (size_t)iDstSize, "%s_%d.bak", sFile, iIndex);
		return;
	}
	iBaseLen = (int)(sExt - sFile);
	if ( iIndex <= 0 ) {
		snprintf(sDst, (size_t)iDstSize, "%.*s%s", iBaseLen, sFile, sExt);
	} else {
		snprintf(sDst, (size_t)iDstSize, "%.*s_%d%s", iBaseLen, sFile, iIndex, sExt);
	}
}

static int MapEditMaterialBackupFile(mapedit_app_t* pApp, const char* sFile)
{
	str sDir;
	str sBackupDir;
	str sSrcPath;
	str sDstPath;
	char sBackupFile[MAPEDIT_MATERIAL_TEXT_CAPACITY];
	int i;
	int iRet;

	if ( pApp == NULL || sFile == NULL || sFile[0] == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sDir = MapEditMaterialCurrentDirPath(pApp);
	if ( sDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	sSrcPath = xrtPathJoin(2, sDir, (str)sFile);
	if ( sSrcPath == NULL ) {
		xrtFree(sDir);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xrtFileExists(sSrcPath) ) {
		xrtFree(sSrcPath);
		xrtFree(sDir);
		return XGE_OK;
	}
	sBackupDir = xrtPathJoin(2, sDir, (str)"backup");
	xrtFree(sDir);
	if ( sBackupDir == NULL ) {
		xrtFree(sSrcPath);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xrtDirExists(sBackupDir) ) {
		xrtDirCreateAll(sBackupDir);
	}
	sDstPath = NULL;
	for ( i = 0; i < 10000; i++ ) {
		MapEditMaterialBuildBackupName(sBackupFile, sizeof(sBackupFile), sFile, i);
		sDstPath = xrtPathJoin(2, sBackupDir, (str)sBackupFile);
		if ( sDstPath == NULL ) {
			xrtFree(sBackupDir);
			xrtFree(sSrcPath);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( !xrtFileExists(sDstPath) ) {
			break;
		}
		xrtFree(sDstPath);
		sDstPath = NULL;
	}
	if ( sDstPath == NULL ) {
		xrtFree(sBackupDir);
		xrtFree(sSrcPath);
		return XGE_ERROR;
	}
	iRet = xrtFileCopy(sSrcPath, sDstPath, TRUE) ? XGE_OK : XGE_ERROR;
	xrtFree(sDstPath);
	xrtFree(sBackupDir);
	xrtFree(sSrcPath);
	return iRet;
}

static void MapEditMaterialUpdateListText(mapedit_app_t* pApp, int iIndex)
{
	mapedit_material_item_t* pItem;
	char sText[MAPEDIT_MATERIAL_TEXT_CAPACITY * 2];

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) ) {
		return;
	}
	pItem = &pApp->arrMaterialItems[iIndex];
	snprintf(sText, sizeof(sText), "%s    %s", pItem->sName, pItem->sFile);
	MapEditMaterialCopyField(pItem->sText, sizeof(pItem->sText), sText);
	pApp->arrMaterialListItems[iIndex] = pItem->sText;
	xgeXuiListViewSetItems(&pApp->tMaterialList, pApp->arrMaterialListItems, pApp->iMaterialItemCount);
	xgeXuiListViewSetSelected(&pApp->tMaterialList, iIndex);
}

static int MapEditMaterialSaveMapping(mapedit_app_t* pApp, int iIndex, const char* sName)
{
	str sMapPath;
	xvalue pMap;
	mapedit_material_item_t* pItem;

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) || (sName == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = &pApp->arrMaterialItems[iIndex];
	sMapPath = MapEditMaterialCurrentMapPath(pApp);
	if ( sMapPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pMap = MapEditMaterialLoadMap((const char*)sMapPath);
	if ( pMap == NULL ) {
		xrtFree(sMapPath);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xvoTableSetText(pMap, (str)pItem->sFile, (uint32)strlen(pItem->sFile), (str)sName, 0, TRUE);
	xrtStringifyXSON_File(sMapPath, pMap, 1, 0);
	xvoUnref(pMap);
	xrtFree(sMapPath);
	MapEditMaterialCopyField(pItem->sName, sizeof(pItem->sName), sName);
	MapEditMaterialUpdateListText(pApp, iIndex);
	return XGE_OK;
}

static int MapEditMaterialIsImageFile(const char* sPath)
{
	const char* sExt;
	char sLower[16];
	int i;

	if ( sPath == NULL ) {
		return 0;
	}
	sExt = strrchr(sPath, '.');
	if ( sExt == NULL ) {
		return 0;
	}
	for ( i = 0; (i < (int)sizeof(sLower) - 1) && sExt[i] != 0; i++ ) {
		char ch = sExt[i];
		if ( ch >= 'A' && ch <= 'Z' ) {
			ch = (char)(ch - 'A' + 'a');
		}
		sLower[i] = ch;
	}
	sLower[i] = 0;
	return strcmp(sLower, ".png") == 0 ||
		strcmp(sLower, ".jpg") == 0 ||
		strcmp(sLower, ".jpeg") == 0 ||
		strcmp(sLower, ".bmp") == 0 ||
		strcmp(sLower, ".tga") == 0 ||
		strcmp(sLower, ".webp") == 0;
}

static int MapEditMaterialScanProc(str sPath, size_t iSize, int bDir, ptr pData, ptr pParam)
{
	mapedit_material_scan_t* pScan;
	mapedit_material_item_t* pItem;
	str sNameExt;
	xvalue pNameValue;
	const char* sName;
	char sListText[MAPEDIT_MATERIAL_TEXT_CAPACITY * 2];

	(void)pData;
	pScan = (mapedit_material_scan_t*)pParam;
	if ( (pScan == NULL) || (pScan->pApp == NULL) || bDir ) {
		return 0;
	}
	if ( !MapEditMaterialIsImageFile((const char*)sPath) ) {
		return 0;
	}
	sNameExt = xrtPathGetNameExt(sPath, iSize);
	if ( sNameExt == NULL || sNameExt[0] == 0 ) {
		xrtFree(sNameExt);
		return 0;
	}
	pNameValue = xvoTableGetValue(pScan->pMap, sNameExt, (uint32)strlen((const char*)sNameExt));
	if ( xvoType(pNameValue) == XVO_DT_TEXT ) {
		sName = (const char*)xvoGetText(pNameValue);
	} else {
		sName = "未命名图块";
		xvoTableSetText(pScan->pMap, sNameExt, (uint32)strlen((const char*)sNameExt), (str)sName, 0, TRUE);
		pScan->bChanged = 1;
	}
	pScan->iDiscoveredCount++;
	if ( pScan->bCollectItems != 0 ) {
		if ( pScan->pApp->iMaterialItemCount < MAPEDIT_MATERIAL_ITEM_MAX ) {
			pItem = &pScan->pApp->arrMaterialItems[pScan->pApp->iMaterialItemCount];
			snprintf(sListText, sizeof(sListText), "%s    %s", sName, (const char*)sNameExt);
			MapEditMaterialCopyField(pItem->sFile, sizeof(pItem->sFile), (const char*)sNameExt);
			MapEditMaterialCopyField(pItem->sName, sizeof(pItem->sName), sName);
			MapEditMaterialCopyField(pItem->sPath, sizeof(pItem->sPath), (const char*)sPath);
			MapEditMaterialCopyField(pItem->sText, sizeof(pItem->sText), sListText);
			pScan->pApp->arrMaterialListItems[pScan->pApp->iMaterialItemCount] = pItem->sText;
			pScan->pApp->iMaterialItemCount++;
		} else {
			pScan->iSkippedCount++;
		}
	}
	xrtFree(sNameExt);
	return 0;
}

static xvalue MapEditMaterialLoadMap(const char* sPath)
{
	xvalue pMap;

	pMap = NULL;
	if ( (sPath != NULL) && xrtFileExists((str)sPath) ) {
		pMap = xrtParseXSON_File((str)sPath);
	}
	if ( (pMap == NULL) || (xvoType(pMap) != XVO_DT_TABLE) ) {
		if ( pMap != NULL ) {
			xvoUnref(pMap);
		}
		pMap = xvoCreateTable();
	}
	return pMap;
}

static int MapEditMaterialsScanCategory(mapedit_app_t* pApp, int iCategory, int bCollectItems)
{
	const mapedit_material_category_t* pCategory;
	mapedit_material_scan_t tScan;
	str sDir;
	str sMapPath;
	xvalue pMap;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCategory = MapEditMaterialCategoryClamp(iCategory);
	pCategory = &g_arrMaterialCategories[iCategory];
	if ( bCollectItems != 0 ) {
		xgeXuiWidgetClearTooltip(pApp->pMaterialListWidget);
		MapEditMaterialPreviewClear(pApp);
		pApp->iMaterialCategory = iCategory;
		pApp->iMaterialItemCount = 0;
	}
	sDir = MapEditMaterialAssetPath(pCategory->sDir);
	sMapPath = MapEditMaterialAssetPath(pCategory->sMapFile);
	if ( (sDir == NULL) || (sMapPath == NULL) ) {
		xrtFree(sDir);
		xrtFree(sMapPath);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xrtDirExists(sDir) ) {
		xrtDirCreateAll(sDir);
	}
	pMap = MapEditMaterialLoadMap((const char*)sMapPath);
	if ( pMap == NULL ) {
		xrtFree(sDir);
		xrtFree(sMapPath);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(&tScan, 0, sizeof(tScan));
	tScan.pApp = pApp;
	tScan.pMap = pMap;
	tScan.bCollectItems = bCollectItems;
	xrtDirScan(sDir, 0, MapEditMaterialScanProc, &tScan);
	if ( tScan.bChanged || !xrtFileExists(sMapPath) ) {
		xrtStringifyXSON_File(sMapPath, pMap, 1, 0);
	}
	xvoUnref(pMap);
	if ( (bCollectItems != 0) && (pApp->pMaterialListWidget != NULL) ) {
		xgeXuiListViewSetItems(&pApp->tMaterialList, pApp->arrMaterialListItems, pApp->iMaterialItemCount);
		xgeXuiListViewSetSelected(&pApp->tMaterialList, (pApp->iMaterialItemCount > 0) ? 0 : -1);
		xgeXuiListViewSetScroll(&pApp->tMaterialList, 0.0f);
		xgeXuiWidgetSetTooltipResolver(pApp->pMaterialListWidget, MapEditMaterialTooltipResolve, pApp);
		xgeXuiWidgetMarkLayout(pApp->pMaterialListWidget);
		xgeXuiWidgetMarkPaint(pApp->pMaterialListWidget);
	}
	xrtFree(sDir);
	xrtFree(sMapPath);
	return XGE_OK;
}

static int MapEditMaterialsLoadCategory(mapedit_app_t* pApp, int iCategory)
{
	return MapEditMaterialsScanCategory(pApp, iCategory, 1);
}

static void MapEditMaterialsEnsureAllMaps(mapedit_app_t* pApp)
{
	const mapedit_material_category_t* pCategory;
	str sDir;
	str sMapPath;
	xvalue pMap;
	int i;
	int iCount;

	iCount = (int)(sizeof(g_arrMaterialCategories) / sizeof(g_arrMaterialCategories[0]));
	for ( i = 0; i < iCount; i++ ) {
		pCategory = &g_arrMaterialCategories[i];
		sDir = MapEditMaterialAssetPath(pCategory->sDir);
		sMapPath = MapEditMaterialAssetPath(pCategory->sMapFile);
		if ( (sDir == NULL) || (sMapPath == NULL) ) {
			xrtFree(sDir);
			xrtFree(sMapPath);
			continue;
		}
		if ( !xrtDirExists(sDir) ) {
			xrtDirCreateAll(sDir);
		}
		if ( !xrtFileExists(sMapPath) ) {
			pMap = xvoCreateTable();
			if ( pMap != NULL ) {
				xrtStringifyXSON_File(sMapPath, pMap, 1, 0);
				xvoUnref(pMap);
			}
		}
		xrtFree(sDir);
		xrtFree(sMapPath);
		if ( pApp != NULL ) {
			MapEditMaterialsScanCategory(pApp, i, 0);
		}
	}
}

static void MapEditMaterialCategoryChanged(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	MapEditMaterialsLoadCategory(pApp, iIndex);
}

static int MapEditMaterialEnsurePreview(mapedit_app_t* pApp, int iIndex)
{
	mapedit_material_item_t* pItem;

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) ) {
		return 0;
	}
	if ( (pApp->iMaterialPreviewIndex == iIndex) && (pApp->pMaterialPreviewTexture != NULL) ) {
		return 1;
	}
	MapEditMaterialPreviewClear(pApp);
	pItem = &pApp->arrMaterialItems[iIndex];
	if ( xgeXuiTextureCreateFile(&pApp->tXui, pItem->sPath, 0, &pApp->pMaterialPreviewTexture) != XGE_OK ) {
		pApp->pMaterialPreviewTexture = NULL;
		return 0;
	}
	if ( xgeXuiTextureGetDesc(&pApp->tXui, pApp->pMaterialPreviewTexture, &pApp->tMaterialPreviewDesc) != XGE_OK ) {
		MapEditMaterialPreviewClear(pApp);
		return 0;
	}
	pApp->iMaterialPreviewIndex = iIndex;
	return 1;
}

static xge_vec2_t MapEditMaterialTooltipMeasure(xge_xui_context pContext, xge_xui_widget pOwner, void* pUser)
{
	mapedit_app_t* pApp;
	float fW;
	float fH;
	float fScale;

	(void)pContext;
	(void)pOwner;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pApp->pMaterialPreviewTexture == NULL) || (pApp->tMaterialPreviewDesc.iWidth <= 0) || (pApp->tMaterialPreviewDesc.iHeight <= 0) ) {
		return (xge_vec2_t){ 160.0f, 48.0f };
	}
	fW = (float)pApp->tMaterialPreviewDesc.iWidth;
	fH = (float)pApp->tMaterialPreviewDesc.iHeight;
	fScale = 1.0f;
	if ( fW > 320.0f ) {
		fScale = 320.0f / fW;
	}
	if ( fH * fScale > 240.0f ) {
		fScale = 240.0f / fH;
	}
	if ( fScale > 1.0f ) {
		fScale = 1.0f;
	}
	return (xge_vec2_t){ fW * fScale + 16.0f, fH * fScale + 40.0f };
}

static void MapEditMaterialTooltipPaint(xge_xui_context pContext, xge_xui_widget pOwner, xge_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_material_item_t* pItem;
	xge_rect_t tImage;
	xge_draw_t tDraw;
	float fW;
	float fH;
	float fScale;
	int iIndex;

	(void)pContext;
	(void)pOwner;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pApp->pMaterialPreviewTexture == NULL) || (pApp->iMaterialPreviewIndex < 0) || (pApp->iMaterialPreviewIndex >= pApp->iMaterialItemCount) ) {
		xgeTextDrawRect(pApp != NULL && pApp->bFontReady ? &pApp->tFont : NULL, "图片不可用", tRect, XGE_COLOR_RGBA(31, 75, 112, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		return;
	}
	iIndex = pApp->iMaterialPreviewIndex;
	pItem = &pApp->arrMaterialItems[iIndex];
	fW = (float)pApp->tMaterialPreviewDesc.iWidth;
	fH = (float)pApp->tMaterialPreviewDesc.iHeight;
	fScale = 1.0f;
	if ( fW > 320.0f ) {
		fScale = 320.0f / fW;
	}
	if ( fH * fScale > 240.0f ) {
		fScale = 240.0f / fH;
	}
	if ( fScale > 1.0f ) {
		fScale = 1.0f;
	}
	tImage.fW = fW * fScale;
	tImage.fH = fH * fScale;
	tImage.fX = tRect.fX + (tRect.fW - tImage.fW) * 0.5f;
	tImage.fY = tRect.fY + 8.0f;
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pMaterialPreviewTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, fW, fH };
	tDraw.tDst = tImage;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeShapeRectStrokePx(tImage, 1.0f, XGE_COLOR_RGBA(128, 172, 204, 255));
	xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, pItem->sName, (xge_rect_t){ tRect.fX + 8.0f, tImage.fY + tImage.fH + 6.0f, tRect.fW - 16.0f, 20.0f }, XGE_COLOR_RGBA(31, 75, 112, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
}

static int MapEditMaterialTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	mapedit_app_t* pApp;
	int iHover;
	xge_rect_t tViewport;
	xge_rect_t tItem;
	float fScrollY;

	(void)pContext;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) || (pDesc == NULL) || (pWidget != pApp->pMaterialListWidget) ) {
		return 0;
	}
	iHover = pApp->tMaterialList.iHover;
	if ( !MapEditMaterialEnsurePreview(pApp, iHover) ) {
		return 0;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMaterialList.tFrame);
	fScrollY = xgeXuiListViewGetScroll(&pApp->tMaterialList);
	tItem.fX = tViewport.fX;
	tItem.fY = tViewport.fY + (float)iHover * pApp->tMaterialList.fItemHeight - fScrollY;
	tItem.fW = tViewport.fW;
	tItem.fH = pApp->tMaterialList.fItemHeight;
	if ( (tItem.fY + tItem.fH <= tViewport.fY) || (tItem.fY >= tViewport.fY + tViewport.fH) ) {
		return 0;
	}
	if ( tItem.fY < tViewport.fY ) {
		tItem.fH -= (tViewport.fY - tItem.fY);
		tItem.fY = tViewport.fY;
	}
	if ( tItem.fY + tItem.fH > tViewport.fY + tViewport.fH ) {
		tItem.fH = tViewport.fY + tViewport.fH - tItem.fY;
	}
	pDesc->iType = XGE_XUI_TOOLTIP_CUSTOM;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	pDesc->bCustomAnchorRect = 1;
	pDesc->tAnchorRect = tItem;
	pDesc->fOffsetX = 6.0f;
	pDesc->fOffsetY = 0.0f;
	pDesc->fDelay = 0.5f;
	pDesc->bFollowCursor = 0;
	pDesc->procMeasure = MapEditMaterialTooltipMeasure;
	pDesc->procPaint = MapEditMaterialTooltipPaint;
	pDesc->pUser = pApp;
	return 1;
}

static int MapEditMaterialIndexAt(mapedit_app_t* pApp, float fX, float fY)
{
	xge_rect_t tViewport;
	int iIndex;

	if ( (pApp == NULL) || (pApp->pMaterialListWidget == NULL) || (pApp->tMaterialList.fItemHeight <= 0.0f) ||
		!MapEditMaterialWidgetIsInteractive(pApp->pMaterialListWidget) ) {
		return -1;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMaterialList.tFrame);
	if ( !MapEditMaterialRectContains(tViewport, fX, fY) ) {
		return -1;
	}
	iIndex = (int)((fY - tViewport.fY + xgeXuiListViewGetScroll(&pApp->tMaterialList)) / pApp->tMaterialList.fItemHeight);
	if ( (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) ) {
		return -1;
	}
	return iIndex;
}

static void MapEditMaterialEditClose(mapedit_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->bMaterialEditReady == 0) ) {
		return;
	}
	xgeXuiMsgTipClose(&pApp->tMaterialEditMsgTip);
	xgeXuiWindowSetOpen(&pApp->tMaterialEditWindow, 0);
}

static void MapEditMaterialEditCloseClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	MapEditMaterialEditClose((mapedit_app_t*)pUser);
}

static int MapEditMaterialTextIsBlank(const char* sText)
{
	const unsigned char* p;

	if ( sText == NULL ) {
		return 1;
	}
	p = (const unsigned char*)sText;
	while ( *p != 0 ) {
		if ( *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' ) {
			return 0;
		}
		p++;
	}
	return 1;
}

static void MapEditMaterialEditShowError(mapedit_app_t* pApp, xge_xui_widget pFocusWidget, const char* sText)
{
	if ( pApp == NULL || sText == NULL ) {
		return;
	}
	MapEditAppSetStatus(pApp, sText);
	if ( pFocusWidget != NULL ) {
		xgeXuiSetFocus(&pApp->tXui, pFocusWidget);
	}
	if ( pApp->pMaterialEditMsgTipWidget != NULL && pApp->pMaterialEditWindowWidget != NULL ) {
		xgeXuiWidgetSetRect(pApp->pMaterialEditMsgTipWidget, pApp->pMaterialEditWindowWidget->tRect);
		(void)xgeXuiMsgTipShow(&pApp->tMaterialEditMsgTip, XGE_XUI_MSG_TIP_ICON_ERROR, sText, 2.2f);
	}
}

static void MapEditMaterialEditClearSource(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMaterialEditSourceTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialEditSourceTexture);
		pApp->pMaterialEditSourceTexture = NULL;
	}
	xgeImageFree(&pApp->tMaterialEditSourceImage);
	memset(&pApp->tMaterialEditSourceImage, 0, sizeof(pApp->tMaterialEditSourceImage));
	pApp->bMaterialEditSourceReady = 0;
	pApp->iMaterialEditSourceCol = -1;
	pApp->iMaterialEditSourceRow = -1;
	pApp->iMaterialEditSourceCols = 1;
	pApp->iMaterialEditSourceRows = 1;
	pApp->sMaterialEditSourcePath[0] = 0;
	MapEditTileGridSetTexture(&pApp->tMaterialEditSourceGrid, NULL, 0, 0);
	if ( pApp->tMaterialEditSourceScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMaterialEditSourceScrollFrame, 0.0f, 0.0f);
		MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 1);
	}
}

static void MapEditMaterialEditClearOutput(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMaterialEditOutputTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialEditOutputTexture);
		pApp->pMaterialEditOutputTexture = NULL;
	}
	free(pApp->pMaterialEditOutputPixels);
	pApp->pMaterialEditOutputPixels = NULL;
	pApp->iMaterialEditOutputCols = 0;
	pApp->iMaterialEditOutputRows = 0;
	pApp->iMaterialEditUsedCols = 0;
	pApp->iMaterialEditUsedRows = 0;
	MapEditTileGridSetTexture(&pApp->tMaterialEditOriginalGrid, NULL, 0, 0);
	if ( pApp->tMaterialEditOriginalScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMaterialEditOriginalScrollFrame, 0.0f, 0.0f);
		MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 0);
	}
}

static int MapEditMaterialEditRefreshOutputTexture(mapedit_app_t* pApp)
{
	int iWidth;
	int iHeight;

	if ( pApp == NULL || pApp->pMaterialEditOutputPixels == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iWidth = pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth;
	iHeight = pApp->iMaterialEditOutputRows * pApp->iMaterialEditTileHeight;
	if ( pApp->pMaterialEditOutputTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMaterialEditOutputTexture);
		pApp->pMaterialEditOutputTexture = NULL;
	}
	if ( xgeXuiTextureCreateRGBA(&pApp->tXui, iWidth, iHeight, pApp->pMaterialEditOutputPixels, iWidth * (int)sizeof(uint32_t), XGE_IMAGE_STRAIGHT_ALPHA, &pApp->pMaterialEditOutputTexture) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditTileGridSetTexture(&pApp->tMaterialEditOriginalGrid, pApp->pMaterialEditOutputTexture, iWidth, iHeight);
	MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 0);
	return XGE_OK;
}

static float MapEditMaterialGridContentWidth(mapedit_tile_grid_t* pGrid)
{
	float fWidth;

	if ( pGrid == NULL ) {
		return 0.0f;
	}
	fWidth = (float)(pGrid->iColumnCount * pGrid->iCellWidth);
	if ( pGrid->pTexture != NULL && pGrid->iTextureWidth > 0 && fWidth < (float)pGrid->iTextureWidth ) {
		fWidth = (float)pGrid->iTextureWidth;
	}
	return (fWidth > 1.0f) ? fWidth : 1.0f;
}

static float MapEditMaterialGridContentHeight(mapedit_tile_grid_t* pGrid)
{
	float fHeight;

	if ( pGrid == NULL ) {
		return 0.0f;
	}
	fHeight = (float)(pGrid->iRowCount * pGrid->iCellHeight);
	if ( pGrid->pTexture != NULL && pGrid->iTextureHeight > 0 && fHeight < (float)pGrid->iTextureHeight ) {
		fHeight = (float)pGrid->iTextureHeight;
	}
	return (fHeight > 1.0f) ? fHeight : 1.0f;
}

static float MapEditMaterialAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditMaterialRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditMaterialAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditMaterialAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditMaterialAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditMaterialAbsFloat(a.fH - b.fH) < 0.01f;
}

static void MapEditMaterialSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditMaterialRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditMaterialRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditMaterialEditUpdateScrollContentSize(mapedit_app_t* pApp, int bSource)
{
	xge_xui_scroll_frame pFrame;
	xge_xui_scroll_model pModel;
	mapedit_tile_grid_t* pGrid;
	float fContentW;
	float fContentH;

	if ( pApp == NULL ) {
		return;
	}
	pFrame = bSource ? &pApp->tMaterialEditSourceScrollFrame : &pApp->tMaterialEditOriginalScrollFrame;
	pModel = bSource ? &pApp->tMaterialEditSourceScrollModel : &pApp->tMaterialEditOriginalScrollModel;
	pGrid = bSource ? &pApp->tMaterialEditSourceGrid : &pApp->tMaterialEditOriginalGrid;
	if ( pFrame->pWidget == NULL || pModel == NULL ) {
		return;
	}
	fContentW = MapEditMaterialGridContentWidth(pGrid);
	fContentH = MapEditMaterialGridContentHeight(pGrid);
	if ( MapEditMaterialAbsFloat(pModel->fContentW - fContentW) >= 0.01f ||
	     MapEditMaterialAbsFloat(pModel->fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(pFrame, fContentW, fContentH);
	}
}

static void MapEditMaterialEditSyncScrollContent(mapedit_app_t* pApp, int bSource)
{
	xge_xui_scroll_frame pFrame;
	xge_xui_scroll_model pModel;
	xge_xui_widget pGridWidget;
	mapedit_tile_grid_t* pGrid;
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL ) {
		return;
	}
	pFrame = bSource ? &pApp->tMaterialEditSourceScrollFrame : &pApp->tMaterialEditOriginalScrollFrame;
	pModel = bSource ? &pApp->tMaterialEditSourceScrollModel : &pApp->tMaterialEditOriginalScrollModel;
	pGridWidget = bSource ? pApp->pMaterialEditSourceGridWidget : pApp->pMaterialEditOriginalGridWidget;
	pGrid = bSource ? &pApp->tMaterialEditSourceGrid : &pApp->tMaterialEditOriginalGrid;
	if ( pFrame->pWidget == NULL || pModel == NULL || pGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pFrame);
	fContentW = (pModel->fContentW > 1.0f) ? pModel->fContentW : MapEditMaterialGridContentWidth(pGrid);
	fContentH = (pModel->fContentH > 1.0f) ? pModel->fContentH : MapEditMaterialGridContentHeight(pGrid);
	tLocal.fX = -pModel->fScrollX;
	tLocal.fY = -pModel->fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditMaterialSetScrollContentRect(pGridWidget, tLocal, tRect);
}

static void MapEditMaterialEditUpdateAndSyncScrollContent(mapedit_app_t* pApp, int bSource)
{
	MapEditMaterialEditUpdateScrollContentSize(pApp, bSource);
	MapEditMaterialEditSyncScrollContent(pApp, bSource);
}

static void MapEditMaterialEditSyncAllScrollContent(mapedit_app_t* pApp)
{
	MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 0);
	MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 1);
}

static int MapEditMaterialEditExtendOriginalRows(mapedit_app_t* pApp)
{
	int iRows;

	if ( pApp == NULL || pApp->iMaterialEditMode != MAPEDIT_TILE_EDIT_MODE_STATIC ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRows = pApp->iMaterialEditOutputRows + MAPEDIT_MATERIAL_EXTEND_ROWS;
	if ( iRows < MAPEDIT_MATERIAL_INITIAL_ROWS + MAPEDIT_MATERIAL_EXTEND_ROWS ) {
		iRows = MAPEDIT_MATERIAL_INITIAL_ROWS + MAPEDIT_MATERIAL_EXTEND_ROWS;
	}
	return MapEditMaterialEditEnsureOutput(pApp, MapEditMaterialStaticCols(pApp), iRows);
}

static int MapEditMaterialEditOriginalAtBottom(mapedit_app_t* pApp)
{
	xge_xui_scroll_model pModel;
	float fMaxY;

	if ( pApp == NULL ) {
		return 0;
	}
	pModel = &pApp->tMaterialEditOriginalScrollModel;
	fMaxY = pModel->fContentH - pModel->tViewportRect.fH;
	if ( fMaxY <= 0.0f ) {
		return 0;
	}
	return (pModel->fScrollY + 1.0f >= fMaxY);
}

static void MapEditMaterialEditScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pFrame == &pApp->tMaterialEditSourceScrollFrame ) {
		MapEditMaterialEditSyncScrollContent(pApp, 1);
	} else {
		if ( MapEditMaterialEditOriginalAtBottom(pApp) ) {
			(void)MapEditMaterialEditExtendOriginalRows(pApp);
		}
		MapEditMaterialEditSyncScrollContent(pApp, 0);
	}
}

static void MapEditMaterialEditScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pWidget == NULL ) {
		return;
	}
	if ( pWidget == pApp->pMaterialEditSourceCanvasWidget ) {
		xgeXuiScrollFrameLayout(&pApp->tMaterialEditSourceScrollFrame);
		MapEditMaterialEditSyncScrollContent(pApp, 1);
	} else {
		xgeXuiScrollFrameLayout(&pApp->tMaterialEditOriginalScrollFrame);
		MapEditMaterialEditSyncScrollContent(pApp, 0);
	}
}

static int MapEditMaterialEditEnsureOutput(mapedit_app_t* pApp, int iCols, int iRows)
{
	uint32_t* pNewPixels;
	int iOldWidth;
	int iNewWidth;
	int iNewHeight;
	int y;

	if ( pApp == NULL || iCols <= 0 || iRows <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC ) {
		iCols = MapEditMaterialStaticCols(pApp);
	}
	if ( pApp->iMaterialEditOutputCols >= iCols && pApp->iMaterialEditOutputRows >= iRows && pApp->pMaterialEditOutputPixels != NULL ) {
		return XGE_OK;
	}
	if ( iCols < pApp->iMaterialEditOutputCols ) {
		iCols = pApp->iMaterialEditOutputCols;
	}
	if ( iRows < pApp->iMaterialEditOutputRows ) {
		iRows = pApp->iMaterialEditOutputRows;
	}
	iOldWidth = pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth;
	iNewWidth = iCols * pApp->iMaterialEditTileWidth;
	iNewHeight = iRows * pApp->iMaterialEditTileHeight;
	pNewPixels = (uint32_t*)calloc((size_t)iNewWidth * (size_t)iNewHeight, sizeof(uint32_t));
	if ( pNewPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pApp->pMaterialEditOutputPixels != NULL ) {
		int iCopyWidth = iOldWidth;
		int iCopyHeight = pApp->iMaterialEditOutputRows * pApp->iMaterialEditTileHeight;
		for ( y = 0; y < iCopyHeight; y++ ) {
			memcpy(pNewPixels + (size_t)y * (size_t)iNewWidth, pApp->pMaterialEditOutputPixels + (size_t)y * (size_t)iOldWidth, (size_t)iCopyWidth * sizeof(uint32_t));
		}
		free(pApp->pMaterialEditOutputPixels);
	}
	pApp->pMaterialEditOutputPixels = pNewPixels;
	pApp->iMaterialEditOutputCols = iCols;
	pApp->iMaterialEditOutputRows = iRows;
	if ( pApp->bMaterialEditReady != 0 ) {
		MapEditTileGridSetGridSize(&pApp->tMaterialEditOriginalGrid, iCols, iRows);
		MapEditTileGridSetMaxCells(&pApp->tMaterialEditOriginalGrid, 0);
	}
	return MapEditMaterialEditRefreshOutputTexture(pApp);
}

static int MapEditMaterialEditCopyImageToOutput(mapedit_app_t* pApp, const xge_image_t* pImage, int iStaticCols)
{
	const uint8_t* pSrcRow;
	uint8_t* pDstRow;
	int iCols;
	int iRows;
	int iWidth;
	int iHeight;
	int y;

	if ( pApp == NULL || pImage == NULL || pImage->pPixels == NULL || pImage->iWidth <= 0 || pImage->iHeight <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCols = (pImage->iWidth + pApp->iMaterialEditTileWidth - 1) / pApp->iMaterialEditTileWidth;
	iRows = (pImage->iHeight + pApp->iMaterialEditTileHeight - 1) / pApp->iMaterialEditTileHeight;
	if ( iStaticCols > 0 ) {
		iCols = iStaticCols;
	}
	if ( MapEditMaterialEditEnsureOutput(pApp, iCols, iRows) != XGE_OK ) {
		return XGE_ERROR;
	}
	iWidth = pImage->iWidth;
	iHeight = pImage->iHeight;
	if ( iWidth > pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth ) {
		iWidth = pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth;
	}
	if ( iHeight > pApp->iMaterialEditOutputRows * pApp->iMaterialEditTileHeight ) {
		iHeight = pApp->iMaterialEditOutputRows * pApp->iMaterialEditTileHeight;
	}
	for ( y = 0; y < iHeight; y++ ) {
		pSrcRow = (const uint8_t*)pImage->pPixels + (size_t)y * (size_t)pImage->iStride;
		pDstRow = (uint8_t*)pApp->pMaterialEditOutputPixels + (size_t)y * (size_t)(pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth) * sizeof(uint32_t);
		memcpy(pDstRow, pSrcRow, (size_t)iWidth * sizeof(uint32_t));
	}
	pApp->iMaterialEditUsedCols = iCols;
	pApp->iMaterialEditUsedRows = iRows;
	return MapEditMaterialEditRefreshOutputTexture(pApp);
}

static int MapEditMaterialEditLoadExistingOutput(mapedit_app_t* pApp, const char* sPath)
{
	xge_image_t tImage;
	int iRet;

	if ( pApp == NULL || sPath == NULL || sPath[0] == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tImage, 0, sizeof(tImage));
	if ( xgeImageLoad(&tImage, sPath) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = MapEditMaterialEditCopyImageToOutput(pApp, &tImage, (pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC) ? MapEditMaterialStaticCols(pApp) : 0);
	xgeImageFree(&tImage);
	return iRet;
}

static void MapEditMaterialEditConfigureMode(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->bMaterialEditReady == 0 ) {
		return;
	}
	pApp->iMaterialEditMode = (MapEditMaterialCategoryClamp(pApp->iMaterialCategory) == 0) ? MAPEDIT_TILE_EDIT_MODE_STATIC : MAPEDIT_TILE_EDIT_MODE_OTHER;
	pApp->iMaterialEditTileWidth = MapEditMaterialTileWidth(pApp);
	pApp->iMaterialEditTileHeight = MapEditMaterialTileHeight(pApp);
	MapEditTileGridSetCellSize(&pApp->tMaterialEditOriginalGrid, pApp->iMaterialEditTileWidth, pApp->iMaterialEditTileHeight);
	MapEditTileGridSetCellSize(&pApp->tMaterialEditSourceGrid, pApp->iMaterialEditTileWidth, pApp->iMaterialEditTileHeight);
	if ( pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC ) {
		xgeXuiSplitLayoutSetPaneMode(&pApp->tMaterialEditSplit, 0, XGE_XUI_SPLIT_PANE_FIXED);
		xgeXuiSplitLayoutSetPaneFixedSize(&pApp->tMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
		xgeXuiSplitLayoutSetPaneMinSize(&pApp->tMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
		xgeXuiSplitLayoutSetPaneWeight(&pApp->tMaterialEditSplit, 0, 1.0f);
		xgeXuiSplitLayoutSetPaneWeight(&pApp->tMaterialEditSplit, 1, 1.0f);
		MapEditTileGridSetGridSize(&pApp->tMaterialEditOriginalGrid, MapEditMaterialStaticCols(pApp), MAPEDIT_MATERIAL_INITIAL_ROWS);
		MapEditTileGridSetMaxCells(&pApp->tMaterialEditOriginalGrid, 0);
		MapEditTileGridSetExpand(&pApp->tMaterialEditOriginalGrid, 0, 1);
		(void)MapEditMaterialEditEnsureOutput(pApp, MapEditMaterialStaticCols(pApp), MAPEDIT_MATERIAL_INITIAL_ROWS);
	} else {
		xgeXuiSplitLayoutSetPaneMode(&pApp->tMaterialEditSplit, 0, XGE_XUI_SPLIT_PANE_GROW);
		xgeXuiSplitLayoutSetPaneMinSize(&pApp->tMaterialEditSplit, 0, 260.0f);
		xgeXuiSplitLayoutSetPaneWeight(&pApp->tMaterialEditSplit, 0, 1.0f);
		xgeXuiSplitLayoutSetPaneWeight(&pApp->tMaterialEditSplit, 1, 1.0f);
		MapEditTileGridSetGridSize(&pApp->tMaterialEditOriginalGrid, 4, 4);
		MapEditTileGridSetMaxCells(&pApp->tMaterialEditOriginalGrid, 0);
		MapEditTileGridSetExpand(&pApp->tMaterialEditOriginalGrid, 1, 1);
		(void)MapEditMaterialEditEnsureOutput(pApp, 4, 4);
	}
	MapEditMaterialEditSyncAllScrollContent(pApp);
	xgeXuiWidgetMarkLayout(pApp->pMaterialEditEditorAreaWidget);
}

static int MapEditMaterialEditOpenFileDialog(char* sPath, int iPathSize)
{
#ifdef _WIN32
	OPENFILENAMEW tOfn;
	wchar_t arrPath[MAX_PATH * 4];
	int iLen;

	if ( sPath == NULL || iPathSize <= 0 ) {
		return 0;
	}
	memset(sPath, 0, (size_t)iPathSize);
	memset(arrPath, 0, sizeof(arrPath));
	memset(&tOfn, 0, sizeof(tOfn));
	tOfn.lStructSize = sizeof(tOfn);
	tOfn.lpstrFile = arrPath;
	tOfn.nMaxFile = (DWORD)(sizeof(arrPath) / sizeof(arrPath[0]));
	tOfn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.webp\0All Files\0*.*\0";
	tOfn.lpstrTitle = L"打开图片";
	tOfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	if ( !GetOpenFileNameW(&tOfn) ) {
		return 0;
	}
	iLen = WideCharToMultiByte(CP_UTF8, 0, arrPath, -1, sPath, iPathSize, NULL, NULL);
	if ( iLen <= 0 || iLen >= iPathSize ) {
		sPath[0] = 0;
		return 0;
	}
	return 1;
#else
	(void)sPath;
	(void)iPathSize;
	return 0;
#endif
}

static void MapEditMaterialEditLoadSourceClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	char sPath[MAPEDIT_MATERIAL_PATH_CAPACITY];
	xui_texture pTexture;
	xge_image_t tImage;
	int iCols;
	int iRows;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( !MapEditMaterialEditOpenFileDialog(sPath, sizeof(sPath)) ) {
		MapEditAppSetStatus(pApp, "已取消打开图片");
		return;
	}
	memset(&tImage, 0, sizeof(tImage));
	if ( xgeImageLoad(&tImage, sPath) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "打开图片失败");
		return;
	}
	pTexture = NULL;
	if ( xgeXuiTextureCreateFile(&pApp->tXui, sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) != XGE_OK ) {
		xgeImageFree(&tImage);
		MapEditAppSetStatus(pApp, "创建图片纹理失败");
		return;
	}
	MapEditMaterialEditClearSource(pApp);
	pApp->tMaterialEditSourceImage = tImage;
	pApp->pMaterialEditSourceTexture = pTexture;
	pApp->bMaterialEditSourceReady = 1;
	pApp->iMaterialEditSourceCol = -1;
	pApp->iMaterialEditSourceRow = -1;
	pApp->iMaterialEditSourceCols = 1;
	pApp->iMaterialEditSourceRows = 1;
	MapEditMaterialCopyField(pApp->sMaterialEditSourcePath, sizeof(pApp->sMaterialEditSourcePath), sPath);
	iCols = (tImage.iWidth + pApp->iMaterialEditTileWidth - 1) / pApp->iMaterialEditTileWidth;
	iRows = (tImage.iHeight + pApp->iMaterialEditTileHeight - 1) / pApp->iMaterialEditTileHeight;
	MapEditTileGridSetGridSize(&pApp->tMaterialEditSourceGrid, iCols, iRows);
	MapEditTileGridSetMaxCells(&pApp->tMaterialEditSourceGrid, iCols * iRows);
	MapEditTileGridSetTexture(&pApp->tMaterialEditSourceGrid, pTexture, tImage.iWidth, tImage.iHeight);
	MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 1);
	MapEditAppSetStatus(pApp, "已打开外部图片");
}

static void MapEditMaterialEditSourceSelectionChange(mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pApp->bMaterialEditSourceReady == 0 ) {
		return;
	}
	pApp->iMaterialEditSourceCol = iCol;
	pApp->iMaterialEditSourceRow = iRow;
	pApp->iMaterialEditSourceCols = (iCols > 0) ? iCols : 1;
	pApp->iMaterialEditSourceRows = (iRows > 0) ? iRows : 1;
	MapEditAppSetStatus(pApp, "已选择源图块");
}

static void MapEditMaterialEditPasteTile(mapedit_app_t* pApp, int iOutCol, int iOutRow)
{
	const uint8_t* pSrcBase;
	uint8_t* pDstBase;
	int iTileW;
	int iTileH;
	int iSrcX;
	int iSrcY;
	int iDstX;
	int iDstY;
	int iCopyW;
	int iCopyH;
	int iSelW;
	int iSelH;
	int iOutWidth;
	int y;

	if ( pApp == NULL || pApp->bMaterialEditSourceReady == 0 || pApp->tMaterialEditSourceImage.pPixels == NULL ) {
		return;
	}
	if ( pApp->iMaterialEditSourceCol < 0 || pApp->iMaterialEditSourceRow < 0 ) {
		MapEditAppSetStatus(pApp, "请先选择源图块");
		return;
	}
	iTileW = pApp->iMaterialEditTileWidth;
	iTileH = pApp->iMaterialEditTileHeight;
	iSelW = (pApp->iMaterialEditSourceCols > 0) ? pApp->iMaterialEditSourceCols : 1;
	iSelH = (pApp->iMaterialEditSourceRows > 0) ? pApp->iMaterialEditSourceRows : 1;
	if ( pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC ) {
		int iStaticCols = MapEditMaterialStaticCols(pApp);

		if ( iOutCol >= iStaticCols ) {
			return;
		}
		if ( iOutCol + iSelW > iStaticCols ) {
			iSelW = iStaticCols - iOutCol;
		}
	}
	iSrcX = pApp->iMaterialEditSourceCol * iTileW;
	iSrcY = pApp->iMaterialEditSourceRow * iTileH;
	iDstX = iOutCol * iTileW;
	iDstY = iOutRow * iTileH;
	if ( MapEditMaterialEditEnsureOutput(pApp, (pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC) ? MapEditMaterialStaticCols(pApp) : (iOutCol + iSelW), iOutRow + iSelH) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "扩展输出图块失败");
		return;
	}
	iCopyW = iTileW * iSelW;
	iCopyH = iTileH * iSelH;
	if ( iSrcX + iCopyW > pApp->tMaterialEditSourceImage.iWidth ) {
		iCopyW = pApp->tMaterialEditSourceImage.iWidth - iSrcX;
	}
	if ( iSrcY + iCopyH > pApp->tMaterialEditSourceImage.iHeight ) {
		iCopyH = pApp->tMaterialEditSourceImage.iHeight - iSrcY;
	}
	if ( iCopyW <= 0 || iCopyH <= 0 ) {
		return;
	}
	iOutWidth = pApp->iMaterialEditOutputCols * iTileW;
	for ( y = 0; y < iCopyH; y++ ) {
		pSrcBase = (const uint8_t*)pApp->tMaterialEditSourceImage.pPixels + (size_t)(iSrcY + y) * (size_t)pApp->tMaterialEditSourceImage.iStride + (size_t)iSrcX * sizeof(uint32_t);
		pDstBase = (uint8_t*)pApp->pMaterialEditOutputPixels + (size_t)(iDstY + y) * (size_t)iOutWidth * sizeof(uint32_t) + (size_t)iDstX * sizeof(uint32_t);
		memcpy(pDstBase, pSrcBase, (size_t)iCopyW * sizeof(uint32_t));
	}
	if ( pApp->iMaterialEditUsedCols < iOutCol + iSelW ) {
		pApp->iMaterialEditUsedCols = iOutCol + iSelW;
	}
	if ( pApp->iMaterialEditUsedRows < iOutRow + iSelH ) {
		pApp->iMaterialEditUsedRows = iOutRow + iSelH;
	}
	(void)MapEditMaterialEditRefreshOutputTexture(pApp);
	MapEditAppSetStatus(pApp, "已粘贴图块");
}

static void MapEditMaterialEditOutputCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	(void)pGrid;
	MapEditMaterialEditPasteTile((mapedit_app_t*)pUser, iCol, iRow);
}

static int MapEditMaterialEditSaveMappingByFile(mapedit_app_t* pApp, const char* sOldFile, const char* sFile, const char* sName)
{
	str sMapPath;
	xvalue pMap;

	if ( pApp == NULL || sFile == NULL || sName == NULL || sFile[0] == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sMapPath = MapEditMaterialCurrentMapPath(pApp);
	if ( sMapPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pMap = MapEditMaterialLoadMap((const char*)sMapPath);
	if ( pMap == NULL ) {
		xrtFree(sMapPath);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( sOldFile != NULL && sOldFile[0] != 0 && strcmp(sOldFile, sFile) != 0 ) {
		xvoTableRemove(pMap, (str)sOldFile, (uint32)strlen(sOldFile));
	}
	xvoTableSetText(pMap, (str)sFile, (uint32)strlen(sFile), (str)sName, 0, TRUE);
	xrtStringifyXSON_File(sMapPath, pMap, 1, 0);
	xvoUnref(pMap);
	xrtFree(sMapPath);
	return XGE_OK;
}

static int MapEditMaterialEditSaveOutput(mapedit_app_t* pApp, const char* sFile, const char* sName)
{
	char sOutputFile[MAPEDIT_MATERIAL_TEXT_CAPACITY];
	char sOldFile[MAPEDIT_MATERIAL_TEXT_CAPACITY];
	str sDir;
	str sPath;
	uint32_t* pCropped;
	int iSaveCols;
	int iSaveRows;
	int iSaveW;
	int iSaveH;
	int iOutputW;
	int y;
	int iRet;
	int bEdit;

	if ( pApp == NULL || pApp->pMaterialEditOutputPixels == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( MapEditMaterialBuildOutputFile(sOutputFile, sizeof(sOutputFile), sFile) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bEdit = (pApp->iMaterialEditIndex >= 0 && pApp->iMaterialEditIndex < pApp->iMaterialItemCount);
	sOldFile[0] = 0;
	if ( bEdit ) {
		MapEditMaterialCopyField(sOldFile, sizeof(sOldFile), pApp->arrMaterialItems[pApp->iMaterialEditIndex].sFile);
		if ( MapEditMaterialBackupFile(pApp, sOldFile) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	iSaveCols = pApp->iMaterialEditUsedCols;
	iSaveRows = pApp->iMaterialEditUsedRows;
	if ( pApp->iMaterialEditMode == MAPEDIT_TILE_EDIT_MODE_STATIC ) {
		iSaveCols = MapEditMaterialStaticCols(pApp);
	}
	if ( iSaveCols <= 0 ) {
		iSaveCols = 1;
	}
	if ( iSaveRows <= 0 ) {
		iSaveRows = 1;
	}
	if ( iSaveCols > pApp->iMaterialEditOutputCols ) {
		iSaveCols = pApp->iMaterialEditOutputCols;
	}
	if ( iSaveRows > pApp->iMaterialEditOutputRows ) {
		iSaveRows = pApp->iMaterialEditOutputRows;
	}
	iSaveW = iSaveCols * pApp->iMaterialEditTileWidth;
	iSaveH = iSaveRows * pApp->iMaterialEditTileHeight;
	iOutputW = pApp->iMaterialEditOutputCols * pApp->iMaterialEditTileWidth;
	pCropped = (uint32_t*)malloc((size_t)iSaveW * (size_t)iSaveH * sizeof(uint32_t));
	if ( pCropped == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( y = 0; y < iSaveH; y++ ) {
		memcpy(pCropped + (size_t)y * (size_t)iSaveW, pApp->pMaterialEditOutputPixels + (size_t)y * (size_t)iOutputW, (size_t)iSaveW * sizeof(uint32_t));
	}
	sDir = MapEditMaterialCurrentDirPath(pApp);
	if ( sDir == NULL ) {
		free(pCropped);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xrtDirExists(sDir) ) {
		xrtDirCreateAll(sDir);
	}
	sPath = xrtPathJoin(2, sDir, (str)sOutputFile);
	xrtFree(sDir);
	if ( sPath == NULL ) {
		free(pCropped);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (!bEdit && xrtFileExists(sPath)) ||
	     (bEdit && sOldFile[0] != 0 && strcmp(sOldFile, sOutputFile) != 0 && xrtFileExists(sPath)) ) {
		xrtFree(sPath);
		free(pCropped);
		return XGE_ERROR;
	}
	iRet = xgeImageSavePNG((const char*)sPath, iSaveW, iSaveH, pCropped, iSaveW * (int)sizeof(uint32_t));
	free(pCropped);
	if ( iRet == XGE_OK ) {
		iRet = MapEditMaterialEditSaveMappingByFile(pApp, bEdit ? sOldFile : NULL, sOutputFile, sName);
	}
	if ( iRet == XGE_OK && bEdit && sOldFile[0] != 0 && strcmp(sOldFile, sOutputFile) != 0 ) {
		str sOldPath;
		sDir = MapEditMaterialCurrentDirPath(pApp);
		if ( sDir != NULL ) {
			sOldPath = xrtPathJoin(2, sDir, (str)sOldFile);
			if ( sOldPath != NULL ) {
				xrtFileDelete(sOldPath);
				xrtFree(sOldPath);
			}
			xrtFree(sDir);
		}
	}
	xrtFree(sPath);
	return iRet;
}

static void MapEditMaterialEditOkClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	const char* sName;
	const char* sFile;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	sName = xgeXuiInputGetText(&pApp->tMaterialEditNameInput);
	sFile = xgeXuiInputGetText(&pApp->tMaterialEditFileInput);
	if ( MapEditMaterialTextIsBlank(sName) ) {
		MapEditMaterialEditShowError(pApp, pApp->pMaterialEditNameInputWidget, "请输入图块名称");
		return;
	}
	if ( MapEditMaterialTextIsBlank(sFile) ) {
		MapEditMaterialEditShowError(pApp, pApp->pMaterialEditFileInputWidget, "请输入文件名");
		return;
	}
	if ( sName == NULL || sName[0] == 0 ) {
		MapEditAppSetStatus(pApp, "请输入图块名称");
		return;
	}
	if ( sFile == NULL || sFile[0] == 0 ) {
		MapEditAppSetStatus(pApp, "请输入文件名");
		return;
	}
	if ( MapEditMaterialEditSaveOutput(pApp, sFile, sName) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "保存图块失败");
		return;
	}
	MapEditMaterialsLoadCategory(pApp, pApp->iMaterialCategory);
	MapEditAppSetStatus(pApp, "图块已保存");
	MapEditMaterialEditClose(pApp);
}

static void MapEditMaterialEditCanvasPaint(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	xge_rect_t tRect;
	xge_rect_t tGrid;
	const char* sText;
	float fX;
	float fY;
	float fStep;

	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	tRect = pWidget->tContentRect;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(236, 246, 252, 255));
	xgeShapeRectStrokePx(tRect, 1.0f, XGE_COLOR_RGBA(124, 181, 219, 255));
	if ( pWidget == pApp->pMaterialEditSourceCanvasWidget ) {
		tGrid = tRect;
		tGrid.fX += 10.0f;
		tGrid.fY += 10.0f;
		tGrid.fW -= 20.0f;
		tGrid.fH -= 20.0f;
		fStep = 16.0f;
		for ( fX = tGrid.fX; fX <= tGrid.fX + tGrid.fW; fX += fStep ) {
			xgeShapeRectFillPx((xge_rect_t){ fX, tGrid.fY, 1.0f, tGrid.fH }, XGE_COLOR_RGBA(188, 224, 244, 130));
		}
		for ( fY = tGrid.fY; fY <= tGrid.fY + tGrid.fH; fY += fStep ) {
			xgeShapeRectFillPx((xge_rect_t){ tGrid.fX, fY, tGrid.fW, 1.0f }, XGE_COLOR_RGBA(188, 224, 244, 130));
		}
		sText = "加载外部图片后在这里选择图块";
	} else {
		sText = "原图块预览占位";
	}
	xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, sText, tRect, XGE_COLOR_RGBA(104, 128, 148, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
}

static void MapEditMaterialEditWindowPaintBefore(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	xge_rect_t tRoot;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || !xgeXuiWindowIsOpen(&pApp->tMaterialEditWindow) ) {
		return;
	}
	tRoot = xgeXuiRoot(&pApp->tXui)->tRect;
	xgeShapeRectFillPx(tRoot, XGE_COLOR_RGBA(18, 42, 60, 86));
}

static int MapEditMaterialEditWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp;
	int iRet;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pEvent == NULL) || !xgeXuiWindowIsOpen(&pApp->tMaterialEditWindow) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		MapEditMaterialEditClose(pApp);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iRet = xgeXuiWindowEvent(&pApp->tMaterialEditWindow, pEvent);
	if ( iRet != XGE_XUI_EVENT_CONTINUE ) {
		return iRet;
	}
	if ( MapEditMaterialPointEvent(pEvent->iType) && !MapEditMaterialRectContains(pApp->pMaterialEditWindowWidget->tRect, pEvent->fX, pEvent->fY) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) || (pEvent->iType == XGE_EVENT_KEY_UP) || (pEvent->iType == XGE_EVENT_TEXT) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static xge_rect_t MapEditMaterialEditWindowRect(mapedit_app_t* pApp)
{
	xge_rect_t tRoot;
	xge_rect_t tRect;

	tRoot = xgeXuiRoot(&pApp->tXui)->tRect;
	tRect.fW = tRoot.fW * 0.8f;
	tRect.fH = tRoot.fH * 0.8f;
	tRect.fX = tRoot.fX + (tRoot.fW - tRect.fW) * 0.5f;
	tRect.fY = tRoot.fY + (tRoot.fH - tRect.fH) * 0.5f;
	return tRect;
}

static xge_rect_t MapEditMaterialViewWindowRect(mapedit_app_t* pApp)
{
	xge_rect_t tRoot;
	xge_rect_t tRect;

	tRoot = xgeXuiRoot(&pApp->tXui)->tRect;
	tRect.fW = tRoot.fW * 0.8f;
	tRect.fH = tRoot.fH * 0.8f;
	tRect.fX = tRoot.fX + (tRoot.fW - tRect.fW) * 0.5f;
	tRect.fY = tRoot.fY + (tRoot.fH - tRect.fH) * 0.5f;
	return tRect;
}

static void MapEditMaterialViewClose(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->pMaterialViewWindowWidget == NULL ) {
		return;
	}
	xgeXuiWindowSetOpen(&pApp->tMaterialViewWindow, 0);
	MapEditMaterialViewClear(pApp);
}

static void MapEditMaterialViewCloseClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	MapEditMaterialViewClose((mapedit_app_t*)pUser);
}

static int MapEditMaterialViewWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp;
	int iRet;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pEvent == NULL) || !xgeXuiWindowIsOpen(&pApp->tMaterialViewWindow) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		MapEditMaterialViewClose(pApp);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iRet = xgeXuiWindowEvent(&pApp->tMaterialViewWindow, pEvent);
	if ( iRet != XGE_XUI_EVENT_CONTINUE ) {
		return iRet;
	}
	if ( MapEditMaterialPointEvent(pEvent->iType) && !MapEditMaterialRectContains(pApp->pMaterialViewWindowWidget->tRect, pEvent->fX, pEvent->fY) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) || (pEvent->iType == XGE_EVENT_KEY_UP) || (pEvent->iType == XGE_EVENT_TEXT) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void MapEditMaterialViewImagePaint(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	xge_draw_t tDraw;
	xge_rect_t tRect;
	float fW;
	float fH;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pWidget == NULL || pApp->pMaterialViewTexture == NULL ) {
		return;
	}
	fW = (float)pApp->tMaterialViewTextureDesc.iWidth;
	fH = (float)pApp->tMaterialViewTextureDesc.iHeight;
	if ( fW <= 0.0f || fH <= 0.0f ) {
		return;
	}
	tRect = pWidget->tRect;
	tRect.fW = fW;
	tRect.fH = fH;
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pMaterialViewTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, fW, fH };
	tDraw.tDst = tRect;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

static int MapEditMaterialEnsureEditWindow(mapedit_app_t* pApp)
{
	xge_xui_widget pClient;
	xge_xui_widget pOriginalTitleWidget;
	xge_xui_widget pSourceHeaderWidget;
	xge_xui_widget pSourceTitleWidget;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->bMaterialEditReady != 0 ) {
		return XGE_OK;
	}
	pApp->pMaterialEditWindowWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditEditorAreaWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditOriginalCanvasWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditSourceCanvasWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditOriginalGridWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditSourceGridWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditLoadSourceButtonWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditFormWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditNameLabelWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditNameInputWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditFileLabelWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditFileInputWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditSpacerWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditOkWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditCancelWidget = xgeXuiWidgetCreate();
	pApp->pMaterialEditMsgTipWidget = xgeXuiWidgetCreate();
	pOriginalTitleWidget = xgeXuiWidgetCreate();
	pSourceHeaderWidget = xgeXuiWidgetCreate();
	pSourceTitleWidget = xgeXuiWidgetCreate();
	if ( (pApp->pMaterialEditWindowWidget == NULL) || (pApp->pMaterialEditEditorAreaWidget == NULL) ||
	     (pApp->pMaterialEditOriginalCanvasWidget == NULL) || (pApp->pMaterialEditSourceCanvasWidget == NULL) ||
	     (pApp->pMaterialEditOriginalGridWidget == NULL) || (pApp->pMaterialEditSourceGridWidget == NULL) ||
	     (pApp->pMaterialEditLoadSourceButtonWidget == NULL) || (pApp->pMaterialEditFormWidget == NULL) ||
	     (pApp->pMaterialEditNameLabelWidget == NULL) || (pApp->pMaterialEditNameInputWidget == NULL) ||
	     (pApp->pMaterialEditFileLabelWidget == NULL) || (pApp->pMaterialEditFileInputWidget == NULL) ||
	     (pApp->pMaterialEditSpacerWidget == NULL) || (pApp->pMaterialEditOkWidget == NULL) ||
	     (pApp->pMaterialEditCancelWidget == NULL) || (pApp->pMaterialEditMsgTipWidget == NULL) || (pOriginalTitleWidget == NULL) ||
	     (pSourceHeaderWidget == NULL) || (pSourceTitleWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pMaterialEditWindowWidget, MapEditMaterialEditWindowRect(pApp));
	if ( xgeXuiWindowInit(&pApp->tMaterialEditWindow, &pApp->tXui, pApp->pMaterialEditWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pApp->pMaterialEditMsgTipWidget, pApp->pMaterialEditWindowWidget->tRect);
	if ( xgeXuiWidgetAdd(pApp->pMaterialEditWindowWidget, pApp->pMaterialEditMsgTipWidget) != XGE_OK ||
	     xgeXuiMsgTipInit(&pApp->tMaterialEditMsgTip, &pApp->tXui, pApp->pMaterialEditMsgTipWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMsgTipSetMetrics(&pApp->tMaterialEditMsgTip, 120.0f, 320.0f, 38.0f, -48.0f);
	xgeXuiWindowSetTitle(&pApp->tMaterialEditWindow, pApp->bFontReady ? &pApp->tFont : NULL, "图块编辑");
	xgeXuiWindowSetShowCollapse(&pApp->tMaterialEditWindow, 0);
	xgeXuiWindowSetShowMaximize(&pApp->tMaterialEditWindow, 0);
	xgeXuiWindowSetResizable(&pApp->tMaterialEditWindow, 0);
	xgeXuiWindowSetClose(&pApp->tMaterialEditWindow, MapEditMaterialEditCloseClick, pApp);
	xgeXuiWidgetSetPaintBefore(pApp->pMaterialEditWindowWidget, MapEditMaterialEditWindowPaintBefore, pApp);
	xgeXuiWidgetSetEvent(pApp->pMaterialEditWindowWidget, MapEditMaterialEditWindowEventProc, pApp);

	pClient = xgeXuiWindowGetClientWidget(&pApp->tMaterialEditWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetGap(pClient, 10.0f);
	xgeXuiWidgetSetSize(pApp->pMaterialEditEditorAreaWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	if ( xgeXuiSplitLayoutInit(&pApp->tMaterialEditSplit, &pApp->tXui, pApp->pMaterialEditEditorAreaWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSplitLayoutSetOrientation(&pApp->tMaterialEditSplit, XGE_XUI_ORIENTATION_VERTICAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->tMaterialEditSplit, 2);
	xgeXuiSplitLayoutSetDividerSize(&pApp->tMaterialEditSplit, 8.0f);
	xgeXuiSplitLayoutSetDividerVisualSize(&pApp->tMaterialEditSplit, 1.0f);
	xgeXuiSplitLayoutSetDividerHitSize(&pApp->tMaterialEditSplit, 10.0f);
	xgeXuiSplitLayoutSetColors(&pApp->tMaterialEditSplit,
		XGE_COLOR_RGBA(144, 196, 230, 255),
		XGE_COLOR_RGBA(112, 178, 224, 255),
		XGE_COLOR_RGBA(76, 154, 210, 255),
		XGE_COLOR_RGBA(76, 154, 210, 88));
	xgeXuiSplitLayoutSetPaneMode(&pApp->tMaterialEditSplit, 0, XGE_XUI_SPLIT_PANE_FIXED);
	xgeXuiSplitLayoutSetPaneFixedSize(&pApp->tMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tMaterialEditSplit, 1, 360.0f);
	xgeXuiSplitLayoutSetShadowDrag(&pApp->tMaterialEditSplit, 1);
	pApp->pMaterialEditOriginalPaneWidget = xgeXuiSplitLayoutGetPaneWidget(&pApp->tMaterialEditSplit, 0);
	pApp->pMaterialEditSourcePaneWidget = xgeXuiSplitLayoutGetPaneWidget(&pApp->tMaterialEditSplit, 1);
	if ( (pApp->pMaterialEditOriginalPaneWidget == NULL) || (pApp->pMaterialEditSourcePaneWidget == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetGap(pApp->pMaterialEditOriginalPaneWidget, 6.0f);
	xgeXuiWidgetSetGap(pApp->pMaterialEditSourcePaneWidget, 6.0f);
	xgeXuiWidgetSetSize(pOriginalTitleWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetLayout(pSourceHeaderWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pSourceHeaderWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetGap(pSourceHeaderWidget, 8.0f);
	xgeXuiWidgetSetSize(pSourceTitleWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditLoadSourceButtonWidget, xgeXuiSizePx(92.0f), xgeXuiSizePx(26.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditOriginalCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditSourceCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiScrollModelInit(&pApp->tMaterialEditOriginalScrollModel);
	xgeXuiScrollModelInit(&pApp->tMaterialEditSourceScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tMaterialEditOriginalScrollFrame, &pApp->tXui, pApp->pMaterialEditOriginalCanvasWidget, &pApp->tMaterialEditOriginalScrollModel) != XGE_OK ||
	     xgeXuiScrollFrameInit(&pApp->tMaterialEditSourceScrollFrame, &pApp->tXui, pApp->pMaterialEditSourceCanvasWidget, &pApp->tMaterialEditSourceScrollModel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pApp->pMaterialEditOriginalCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMaterialEditOriginalScrollFrame);
	xgeXuiWidgetSetEvent(pApp->pMaterialEditSourceCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMaterialEditSourceScrollFrame);
	xgeXuiWidgetSetLayoutProc(pApp->pMaterialEditOriginalCanvasWidget, MapEditMaterialEditScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayoutProc(pApp->pMaterialEditSourceCanvasWidget, MapEditMaterialEditScrollLayoutProc, pApp);
	xgeXuiScrollFrameSetChange(&pApp->tMaterialEditOriginalScrollFrame, MapEditMaterialEditScrollChanged, pApp);
	xgeXuiScrollFrameSetChange(&pApp->tMaterialEditSourceScrollFrame, MapEditMaterialEditScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMaterialEditOriginalScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMaterialEditSourceScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMaterialEditOriginalScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMaterialEditSourceScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetMetrics(&pApp->tMaterialEditOriginalScrollFrame, MAPEDIT_MATERIAL_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetMetrics(&pApp->tMaterialEditSourceScrollFrame, MAPEDIT_MATERIAL_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tMaterialEditOriginalScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	xgeXuiScrollFrameSetColors(&pApp->tMaterialEditSourceScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMaterialEditOriginalScrollFrame), pApp->pMaterialEditOriginalGridWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMaterialEditSourceScrollFrame), pApp->pMaterialEditSourceGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( MapEditTileGridInit(&pApp->tMaterialEditOriginalGrid, pApp->pMaterialEditOriginalGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tMaterialEditSourceGrid, pApp->pMaterialEditSourceGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditTileGridSetCellSize(&pApp->tMaterialEditOriginalGrid, MapEditMaterialTileWidth(pApp), MapEditMaterialTileHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMaterialEditOriginalGrid, MapEditMaterialStaticCols(pApp), MAPEDIT_MATERIAL_INITIAL_ROWS);
	MapEditTileGridSetMaxCells(&pApp->tMaterialEditOriginalGrid, 0);
	MapEditTileGridSetExpand(&pApp->tMaterialEditOriginalGrid, 0, 1);
	MapEditTileGridSetEmptyText(&pApp->tMaterialEditOriginalGrid, "原图块预览占位");
	MapEditTileGridSetCellSize(&pApp->tMaterialEditSourceGrid, MapEditMaterialTileWidth(pApp), MapEditMaterialTileHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMaterialEditSourceGrid, MapEditMaterialStaticCols(pApp), 16);
	MapEditTileGridSetMaxCells(&pApp->tMaterialEditSourceGrid, 0);
	MapEditTileGridSetExpand(&pApp->tMaterialEditSourceGrid, 1, 1);
	MapEditTileGridSetEmptyText(&pApp->tMaterialEditSourceGrid, "加载外部图片后在这里选择图块");
	MapEditTileGridSetCellClick(&pApp->tMaterialEditOriginalGrid, MapEditMaterialEditOutputCellClick, pApp);
	MapEditTileGridSetSelectionChange(&pApp->tMaterialEditSourceGrid, MapEditMaterialEditSourceSelectionChange, pApp);
	if ( xgeXuiLabelInit(&pApp->tMaterialEditOriginalTitleLabel, pOriginalTitleWidget, pApp->bFontReady ? &pApp->tFont : NULL, "原图块") != XGE_OK ||
	     xgeXuiLabelInit(&pApp->tMaterialEditSourceTitleLabel, pSourceTitleWidget, pApp->bFontReady ? &pApp->tFont : NULL, "外部图片") != XGE_OK ||
	     xgeXuiButtonInit(&pApp->tMaterialEditLoadSourceButton, &pApp->tXui, pApp->pMaterialEditLoadSourceButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tMaterialEditOriginalTitleLabel, XGE_COLOR_RGBA(30, 74, 112, 255));
	xgeXuiLabelSetColor(&pApp->tMaterialEditSourceTitleLabel, XGE_COLOR_RGBA(30, 74, 112, 255));
	xgeXuiLabelSetAlign(&pApp->tMaterialEditOriginalTitleLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiLabelSetAlign(&pApp->tMaterialEditSourceTitleLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiButtonSetText(&pApp->tMaterialEditLoadSourceButton, pApp->bFontReady ? &pApp->tFont : NULL, "打开图片");
	xgeXuiButtonSetClick(&pApp->tMaterialEditLoadSourceButton, MapEditMaterialEditLoadSourceClick, pApp);

	if ( xgeXuiWidgetAdd(pApp->pMaterialEditOriginalPaneWidget, pOriginalTitleWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditOriginalPaneWidget, pApp->pMaterialEditOriginalCanvasWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pSourceHeaderWidget, pSourceTitleWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pSourceHeaderWidget, pApp->pMaterialEditLoadSourceButtonWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditSourcePaneWidget, pSourceHeaderWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditSourcePaneWidget, pApp->pMaterialEditSourceCanvasWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditEditorAreaWidget, pApp->pMaterialEditOriginalPaneWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditEditorAreaWidget, pApp->pMaterialEditSourcePaneWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pClient, pApp->pMaterialEditEditorAreaWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetLayout(pApp->pMaterialEditFormWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pApp->pMaterialEditFormWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(34.0f));
	xgeXuiWidgetSetGap(pApp->pMaterialEditFormWidget, 8.0f);
	xgeXuiWidgetSetSize(pApp->pMaterialEditNameLabelWidget, xgeXuiSizePx(72.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditNameInputWidget, xgeXuiSizePx(210.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditFileLabelWidget, xgeXuiSizePx(60.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditFileInputWidget, xgeXuiSizePx(240.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditSpacerWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditOkWidget, xgeXuiSizePx(72.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pMaterialEditCancelWidget, xgeXuiSizePx(72.0f), xgeXuiSizePx(28.0f));
	if ( xgeXuiLabelInit(&pApp->tMaterialEditNameLabel, pApp->pMaterialEditNameLabelWidget, pApp->bFontReady ? &pApp->tFont : NULL, "图块名称:") != XGE_OK ||
	     xgeXuiLabelInit(&pApp->tMaterialEditFileLabel, pApp->pMaterialEditFileLabelWidget, pApp->bFontReady ? &pApp->tFont : NULL, "文件名:") != XGE_OK ||
	     xgeXuiInputInit(&pApp->tMaterialEditNameInput, &pApp->tXui, pApp->pMaterialEditNameInputWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
	     xgeXuiInputInit(&pApp->tMaterialEditFileInput, &pApp->tXui, pApp->pMaterialEditFileInputWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
	     xgeXuiButtonInit(&pApp->tMaterialEditOkButton, &pApp->tXui, pApp->pMaterialEditOkWidget) != XGE_OK ||
	     xgeXuiButtonInit(&pApp->tMaterialEditCancelButton, &pApp->tXui, pApp->pMaterialEditCancelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetAlign(&pApp->tMaterialEditNameLabel, XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiLabelSetAlign(&pApp->tMaterialEditFileLabel, XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiLabelSetColor(&pApp->tMaterialEditNameLabel, XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiLabelSetColor(&pApp->tMaterialEditFileLabel, XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiInputSetPlaceholder(&pApp->tMaterialEditNameInput, "输入图块名称");
	xgeXuiInputSetPlaceholder(&pApp->tMaterialEditFileInput, "必须输入文件名");
	xgeXuiButtonSetText(&pApp->tMaterialEditOkButton, pApp->bFontReady ? &pApp->tFont : NULL, "确定");
	xgeXuiButtonSetText(&pApp->tMaterialEditCancelButton, pApp->bFontReady ? &pApp->tFont : NULL, "取消");
	xgeXuiButtonSetSemantic(&pApp->tMaterialEditOkButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	xgeXuiButtonSetClick(&pApp->tMaterialEditOkButton, MapEditMaterialEditOkClick, pApp);
	xgeXuiButtonSetClick(&pApp->tMaterialEditCancelButton, MapEditMaterialEditCloseClick, pApp);
	if ( xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditNameLabelWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditNameInputWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditFileLabelWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditFileInputWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditSpacerWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditOkWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pApp->pMaterialEditFormWidget, pApp->pMaterialEditCancelWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pClient, pApp->pMaterialEditFormWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWindowSetOpen(&pApp->tMaterialEditWindow, 0);
	pApp->bMaterialEditReady = 1;
	return XGE_OK;
}

static int MapEditMaterialEnsureViewWindow(mapedit_app_t* pApp)
{
	xge_xui_widget pClient;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->pMaterialViewWindowWidget != NULL ) {
		return XGE_OK;
	}
	pApp->pMaterialViewWindowWidget = xgeXuiWidgetCreate();
	pApp->pMaterialViewScrollWidget = xgeXuiWidgetCreate();
	pApp->pMaterialViewImageWidget = xgeXuiWidgetCreate();
	if ( (pApp->pMaterialViewWindowWidget == NULL) || (pApp->pMaterialViewScrollWidget == NULL) || (pApp->pMaterialViewImageWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pMaterialViewWindowWidget, MapEditMaterialViewWindowRect(pApp));
	if ( xgeXuiWindowInit(&pApp->tMaterialViewWindow, &pApp->tXui, pApp->pMaterialViewWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWindowSetTitle(&pApp->tMaterialViewWindow, pApp->bFontReady ? &pApp->tFont : NULL, "查看素材");
	xgeXuiWindowSetShowCollapse(&pApp->tMaterialViewWindow, 0);
	xgeXuiWindowSetShowMaximize(&pApp->tMaterialViewWindow, 0);
	xgeXuiWindowSetResizable(&pApp->tMaterialViewWindow, 0);
	xgeXuiWindowSetClose(&pApp->tMaterialViewWindow, MapEditMaterialViewCloseClick, pApp);
	xgeXuiWidgetSetEvent(pApp->pMaterialViewWindowWidget, MapEditMaterialViewWindowEventProc, pApp);
	pClient = xgeXuiWindowGetClientWidget(&pApp->tMaterialViewWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pClient, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pClient, 0.0f);
	xgeXuiWidgetSetSize(pApp->pMaterialViewScrollWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	if ( xgeXuiScrollViewInit(&pApp->tMaterialViewScroll, &pApp->tXui, pApp->pMaterialViewScrollWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiScrollViewSetContentSize(&pApp->tMaterialViewScroll, 1.0f, 1.0f);
	xgeXuiScrollViewSetWheelAxis(&pApp->tMaterialViewScroll, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollViewSetContentDragEnabled(&pApp->tMaterialViewScroll, 1);
	xgeXuiScrollViewSetScrollbarMode(&pApp->tMaterialViewScroll, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollViewSetMetrics(&pApp->tMaterialViewScroll, MAPEDIT_MATERIAL_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollViewSetColors(&pApp->tMaterialViewScroll, XGE_COLOR_RGBA(236, 246, 252, 255), XGE_COLOR_RGBA(210, 228, 242, 255), XGE_COLOR_RGBA(78, 140, 198, 255));
	xgeXuiWidgetSetRect(pApp->pMaterialViewImageWidget, (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f });
	xgeXuiWidgetSetSize(pApp->pMaterialViewImageWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiWidgetSetPaint(pApp->pMaterialViewImageWidget, MapEditMaterialViewImagePaint, pApp);
	if ( xgeXuiWidgetAdd(xgeXuiScrollViewGetContentWidget(&pApp->tMaterialViewScroll), pApp->pMaterialViewImageWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pClient, pApp->pMaterialViewScrollWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWindowSetOpen(&pApp->tMaterialViewWindow, 0);
	return XGE_OK;
}

static void MapEditMaterialOpenView(mapedit_app_t* pApp, int iIndex)
{
	mapedit_material_item_t* pItem;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;
	xge_rect_t tRect;

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) ) {
		return;
	}
	if ( MapEditMaterialEnsureViewWindow(pApp) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "素材查看窗口创建失败");
		return;
	}
	pItem = &pApp->arrMaterialItems[iIndex];
	pTexture = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	if ( xgeXuiTextureCreateFile(&pApp->tXui, pItem->sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) != XGE_OK ||
	     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) != XGE_OK ||
	     tDesc.iWidth <= 0 || tDesc.iHeight <= 0 ) {
		if ( pTexture != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pTexture);
		}
		MapEditAppSetStatus(pApp, "打开素材图片失败");
		return;
	}
	MapEditMaterialViewClear(pApp);
	pApp->pMaterialViewTexture = pTexture;
	pApp->tMaterialViewTextureDesc = tDesc;
	xgeXuiWidgetSetRect(pApp->pMaterialViewImageWidget, (xge_rect_t){ 0.0f, 0.0f, (float)tDesc.iWidth, (float)tDesc.iHeight });
	xgeXuiWidgetSetSize(pApp->pMaterialViewImageWidget, xgeXuiSizePx((float)tDesc.iWidth), xgeXuiSizePx((float)tDesc.iHeight));
	xgeXuiScrollViewSetContentSize(&pApp->tMaterialViewScroll, (float)tDesc.iWidth, (float)tDesc.iHeight);
	xgeXuiScrollViewSetOffset(&pApp->tMaterialViewScroll, 0.0f, 0.0f);
	snprintf(pApp->sMaterialViewTitle, sizeof(pApp->sMaterialViewTitle), "查看图块 - %s(%s)", pItem->sName, pItem->sFile);
	xgeXuiWindowSetTitle(&pApp->tMaterialViewWindow, pApp->bFontReady ? &pApp->tFont : NULL, pApp->sMaterialViewTitle);
	tRect = MapEditMaterialViewWindowRect(pApp);
	xgeXuiWidgetSetRect(pApp->pMaterialViewWindowWidget, tRect);
	xgeXuiWindowSetOpen(&pApp->tMaterialViewWindow, 1);
	(void)xgeXuiOverlayAttach(&pApp->tXui, pApp->pMaterialViewWindowWidget, NULL, XGE_XUI_LAYER_MODAL);
	xgeXuiWindowBringToFront(&pApp->tMaterialViewWindow);
	MapEditAppSetStatus(pApp, "已打开素材查看");
}

static void MapEditMaterialOpenEditor(mapedit_app_t* pApp, int iIndex)
{
	char sTitle[256];
	xge_rect_t tRect;

	if ( pApp == NULL ) {
		return;
	}
	if ( MapEditMaterialEnsureEditWindow(pApp) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "图块编辑窗口创建失败");
		return;
	}
	MapEditMaterialEditClearSource(pApp);
	MapEditMaterialEditClearOutput(pApp);
	pApp->iMaterialEditIndex = iIndex;
	MapEditMaterialEditConfigureMode(pApp);
	if ( (iIndex >= 0) && (iIndex < pApp->iMaterialItemCount) ) {
		snprintf(sTitle, sizeof(sTitle), "图块编辑 - %s", pApp->arrMaterialItems[iIndex].sName);
		xgeXuiInputSetText(&pApp->tMaterialEditNameInput, pApp->arrMaterialItems[iIndex].sName);
		xgeXuiInputSetText(&pApp->tMaterialEditFileInput, pApp->arrMaterialItems[iIndex].sFile);
		xgeXuiInputSetReadonly(&pApp->tMaterialEditFileInput, 0);
		(void)MapEditMaterialEditLoadExistingOutput(pApp, pApp->arrMaterialItems[iIndex].sPath);
	} else {
		snprintf(sTitle, sizeof(sTitle), "图块编辑 - 新建图块");
		xgeXuiInputSetText(&pApp->tMaterialEditNameInput, "新建图块");
		xgeXuiInputSetText(&pApp->tMaterialEditFileInput, "");
		xgeXuiInputSetReadonly(&pApp->tMaterialEditFileInput, 0);
	}
	MapEditMaterialCopyField(pApp->sMaterialEditTitle, sizeof(pApp->sMaterialEditTitle), sTitle);
	xgeXuiWindowSetTitle(&pApp->tMaterialEditWindow, pApp->bFontReady ? &pApp->tFont : NULL, pApp->sMaterialEditTitle);
	tRect = MapEditMaterialEditWindowRect(pApp);
	xgeXuiWidgetSetRect(pApp->pMaterialEditWindowWidget, tRect);
	if ( pApp->pMaterialEditMsgTipWidget != NULL ) {
		xgeXuiWidgetSetRect(pApp->pMaterialEditMsgTipWidget, tRect);
		xgeXuiMsgTipClose(&pApp->tMaterialEditMsgTip);
	}
	xgeXuiWindowSetOpen(&pApp->tMaterialEditWindow, 1);
	(void)xgeXuiOverlayAttach(&pApp->tXui, pApp->pMaterialEditWindowWidget, NULL, XGE_XUI_LAYER_MODAL);
	xgeXuiWindowBringToFront(&pApp->tMaterialEditWindow);
}

static void MapEditMaterialCreateClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	MapEditMaterialOpenEditor((mapedit_app_t*)pUser, -1);
}

static void MapEditMaterialOpenRename(mapedit_app_t* pApp, int iIndex)
{
	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iMaterialItemCount) ) {
		return;
	}
	pApp->iMaterialRenameIndex = iIndex;
	xgeXuiInputBoxSetText(&pApp->tMaterialRenameBox, pApp->bFontReady ? &pApp->tFont : NULL, "重命名素材", "映射名称", pApp->arrMaterialItems[iIndex].sName);
	xgeXuiInputBoxSetOpen(&pApp->tMaterialRenameBox, 1);
}

static void MapEditMaterialRenameSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (sText == NULL) || (sText[0] == 0) ) {
		return;
	}
	if ( MapEditMaterialSaveMapping(pApp, pApp->iMaterialRenameIndex, sText) == XGE_OK ) {
		MapEditAppSetStatus(pApp, "素材映射名称已更新");
	} else {
		MapEditAppSetStatus(pApp, "素材映射名称保存失败");
	}
}

static void MapEditMaterialMenuSelect(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pOwner;
	(void)iIndex;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	switch ( iValue ) {
	case MAPEDIT_MATERIAL_MENU_VIEW:
		MapEditMaterialOpenView(pApp, pApp->iMaterialContextIndex);
		break;
	case MAPEDIT_MATERIAL_MENU_RENAME:
		MapEditMaterialOpenRename(pApp, pApp->iMaterialContextIndex);
		break;
	case MAPEDIT_MATERIAL_MENU_EDIT:
		MapEditMaterialOpenEditor(pApp, pApp->iMaterialContextIndex);
		break;
	default:
		break;
	}
}

static int MapEditMaterialListDoubleClick(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp;
	int iIndex;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( (pApp == NULL) || (pEvent == NULL) || (pEvent->iType != XGE_EVENT_XUI_DOUBLE_CLICK) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = MapEditMaterialIndexAt(pApp, pEvent->fX, pEvent->fY);
	if ( iIndex < 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pApp->iMaterialContextIndex = iIndex;
	xgeXuiListViewSetSelected(&pApp->tMaterialList, iIndex);
	MapEditMaterialOpenView(pApp, iIndex);
	return XGE_XUI_EVENT_CONSUMED;
}

static void MapEditMaterialCloseContextMenuOnListClick(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	int bClick;

	if ( pApp == NULL || pEvent == NULL || pApp->pMaterialListWidget == NULL ) {
		return;
	}
	bClick = (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_MOUSE_UP) ||
	         (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) || (pEvent->iType == XGE_EVENT_TOUCH_END);
	if ( bClick == 0 || xgeXuiMenuIsOpen(&pApp->tMaterialContextMenu) == 0 ) {
		return;
	}
	if ( pApp->tMaterialContextMenu.pPopupWidget != NULL &&
	     MapEditMaterialRectContains(pApp->tMaterialContextMenu.pPopupWidget->tRect, pEvent->fX, pEvent->fY) ) {
		return;
	}
	if ( MapEditMaterialRectContains(pApp->pMaterialListWidget->tRect, pEvent->fX, pEvent->fY) ) {
		xgeXuiMenuClose(&pApp->tMaterialContextMenu);
	}
}

static void MapEditMaterialEditExtendOriginalOnWheel(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->bMaterialEditReady == 0 || pApp->pMaterialEditWindowWidget == NULL ) {
		return;
	}
	if ( pApp->iMaterialEditMode != MAPEDIT_TILE_EDIT_MODE_STATIC || pEvent->iType != XGE_EVENT_MOUSE_WHEEL || pEvent->fDY >= 0.0f ) {
		return;
	}
	if ( !xgeXuiWidgetIsVisible(pApp->pMaterialEditWindowWidget) ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMaterialEditOriginalScrollFrame);
	if ( !MapEditMaterialRectContains(tViewport, pEvent->fX, pEvent->fY) ) {
		return;
	}
	if ( !MapEditMaterialEditOriginalAtBottom(pApp) ) {
		return;
	}
	if ( MapEditMaterialEditExtendOriginalRows(pApp) == XGE_OK ) {
		MapEditMaterialEditUpdateAndSyncScrollContent(pApp, 0);
	}
}

static int MapEditMaterialEditRouteGridEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->bMaterialEditReady == 0 || pApp->pMaterialEditWindowWidget == NULL ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !xgeXuiWidgetIsVisible(pApp->pMaterialEditWindowWidget) || !MapEditMaterialEditMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMaterialEditSourceScrollFrame);
	if ( pApp->pMaterialEditSourceGridWidget != NULL &&
	     (pApp->tMaterialEditSourceGrid.bDragging ||
	      (MapEditMaterialRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	       MapEditMaterialRectContains(pApp->pMaterialEditSourceGridWidget->tRect, pEvent->fX, pEvent->fY))) ) {
		return MapEditTileGridEventProc(pApp->pMaterialEditSourceGridWidget, pEvent, &pApp->tMaterialEditSourceGrid);
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMaterialEditOriginalScrollFrame);
	if ( pApp->pMaterialEditOriginalGridWidget != NULL &&
	     (pApp->tMaterialEditOriginalGrid.bDragging ||
	      (MapEditMaterialRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	       MapEditMaterialRectContains(pApp->pMaterialEditOriginalGridWidget->tRect, pEvent->fX, pEvent->fY))) ) {
		return MapEditTileGridEventProc(pApp->pMaterialEditOriginalGridWidget, pEvent, &pApp->tMaterialEditOriginalGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditTilesetMaterialsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	int iIndex;
	int iResult;

	if ( (pApp == NULL) || (pEvent == NULL) || (pApp->pMaterialListWidget == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	MapEditMaterialCloseContextMenuOnListClick(pApp, pEvent);
	MapEditMaterialEditExtendOriginalOnWheel(pApp, pEvent);
	iResult = MapEditMaterialEditRouteGridEvent(pApp, pEvent);
	if ( iResult != XGE_XUI_EVENT_CONTINUE ) {
		return iResult;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_UP) || (pEvent->iParam1 != XGE_MOUSE_RIGHT) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = MapEditMaterialIndexAt(pApp, pEvent->fX, pEvent->fY);
	if ( iIndex < 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pApp->iMaterialContextIndex = iIndex;
	xgeXuiListViewSetSelected(&pApp->tMaterialList, iIndex);
	xgeXuiMenuOpenAt(&pApp->tMaterialContextMenu, pApp->pMaterialListWidget, pEvent->fX, pEvent->fY);
	return XGE_XUI_EVENT_CONSUMED;
}

void MapEditTilesetMaterialsInitDefaults(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->pTileSourcePreviewWidget = NULL;
	pApp->iTileSourceInfoCount = 0;
	pApp->iMaterialPreviewIndex = -1;
	pApp->iMaterialContextIndex = -1;
	pApp->iMaterialRenameIndex = -1;
	pApp->iMaterialEditIndex = -1;
}

void MapEditTilesetMaterialsUpdateInfo(mapedit_app_t* pApp)
{
	(void)pApp;
}

int MapEditTilesetMaterialsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pComboWidget;
	xge_xui_widget pListWidget;
	xge_xui_widget pCreateButtonWidget;
	xge_xui_widget pRenameWidget;
	xge_xui_menu_item_t arrContextItems[3];

	pContent = MapEditWorkspaceNewContent(pApp);
	if ( pContent == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->iTileSourceInfoCount = 0;
	pApp->iMaterialCategory = 0;
	pApp->iMaterialPreviewIndex = -1;
	MapEditMaterialsEnsureAllMaps(pApp);
	pComboWidget = xgeXuiWidgetCreate();
	pListWidget = xgeXuiWidgetCreate();
	pCreateButtonWidget = xgeXuiWidgetCreate();
	pRenameWidget = xgeXuiWidgetCreate();
	if ( (pComboWidget == NULL) || (pListWidget == NULL) || (pCreateButtonWidget == NULL) || (pRenameWidget == NULL) ) {
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pListWidget);
		xgeXuiWidgetFree(pCreateButtonWidget);
		xgeXuiWidgetFree(pRenameWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pComboWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pListWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pCreateButtonWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetBackground(pListWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pListWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiComboBoxInit(&pApp->tMaterialCombo, &pApp->tXui, pComboWidget) != XGE_OK ||
		xgeXuiListViewInit(&pApp->tMaterialList, &pApp->tXui, pListWidget) != XGE_OK ||
		xgeXuiButtonInit(&pApp->tMaterialCreateButton, &pApp->tXui, pCreateButtonWidget) != XGE_OK ||
		xgeXuiMenuInit(&pApp->tMaterialContextMenu, &pApp->tXui) != XGE_OK ||
		xgeXuiInputBoxInit(&pApp->tMaterialRenameBox, &pApp->tXui, pRenameWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiWidgetFree(pComboWidget);
		xgeXuiWidgetFree(pListWidget);
		xgeXuiWidgetFree(pCreateButtonWidget);
		xgeXuiWidgetFree(pRenameWidget);
		return XGE_ERROR;
	}
	memset(arrContextItems, 0, sizeof(arrContextItems));
	arrContextItems[0].sText = "重命名";
	arrContextItems[0].iType = XGE_XUI_MENU_ITEM_NORMAL;
	arrContextItems[0].iState = XGE_XUI_MENU_ITEM_ENABLED;
	arrContextItems[0].iValue = MAPEDIT_MATERIAL_MENU_RENAME;
	arrContextItems[1].sText = "编辑图块";
	arrContextItems[1].iType = XGE_XUI_MENU_ITEM_NORMAL;
	arrContextItems[1].iState = XGE_XUI_MENU_ITEM_ENABLED;
	arrContextItems[1].iValue = MAPEDIT_MATERIAL_MENU_EDIT;
	arrContextItems[0].sText = "查看";
	arrContextItems[0].iValue = MAPEDIT_MATERIAL_MENU_VIEW;
	arrContextItems[1].sText = "重命名";
	arrContextItems[1].iValue = MAPEDIT_MATERIAL_MENU_RENAME;
	arrContextItems[2].sText = "编辑图块";
	arrContextItems[2].iType = XGE_XUI_MENU_ITEM_NORMAL;
	arrContextItems[2].iState = XGE_XUI_MENU_ITEM_ENABLED;
	arrContextItems[2].iValue = MAPEDIT_MATERIAL_MENU_EDIT;
	xgeXuiMenuSetFont(&pApp->tMaterialContextMenu, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiMenuSetItems(&pApp->tMaterialContextMenu, arrContextItems, 3);
	xgeXuiMenuSetSelect(&pApp->tMaterialContextMenu, MapEditMaterialMenuSelect, pApp);
	xgeXuiInputBoxSetResult(&pApp->tMaterialRenameBox, MapEditMaterialRenameSubmit, pApp);
	xgeXuiInputBoxSetModal(&pApp->tMaterialRenameBox, 1);
	xgeXuiComboBoxSetFont(&pApp->tMaterialCombo, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiComboBoxSetItems(&pApp->tMaterialCombo, g_arrMaterialCategoryNames, (int)(sizeof(g_arrMaterialCategoryNames) / sizeof(g_arrMaterialCategoryNames[0])));
	xgeXuiComboBoxSetSelected(&pApp->tMaterialCombo, 0);
	xgeXuiComboBoxSetSelect(&pApp->tMaterialCombo, MapEditMaterialCategoryChanged, pApp);
	xgeXuiComboBoxSetPopupMaxHeight(&pApp->tMaterialCombo, 160.0f);
	xgeXuiListViewSetFont(&pApp->tMaterialList, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItemHeight(&pApp->tMaterialList, 24.0f);
	xgeXuiListViewSetColors(&pApp->tMaterialList, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(230, 240, 247, 255), XGE_COLOR_RGBA(120, 176, 212, 255));
	xgeXuiButtonSetText(&pApp->tMaterialCreateButton, pApp->bFontReady ? &pApp->tFont : NULL, "制作图块");
	xgeXuiButtonSetSemantic(&pApp->tMaterialCreateButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	xgeXuiButtonSetClick(&pApp->tMaterialCreateButton, MapEditMaterialCreateClick, pApp);
	xgeXuiWidgetSetTooltipResolver(pListWidget, MapEditMaterialTooltipResolve, pApp);
	xgeXuiWidgetSetEventHandler(pListWidget, XGE_EVENT_XUI_DOUBLE_CLICK, MapEditMaterialListDoubleClick, pApp);
	pApp->pMaterialComboWidget = pComboWidget;
	pApp->pMaterialListWidget = pListWidget;
	pApp->pMaterialCreateButtonWidget = pCreateButtonWidget;
	pApp->pMaterialRenameWidget = pRenameWidget;
	if ( xgeXuiWidgetAdd(pContent, pComboWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pListWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pCreateButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditMaterialsLoadCategory(pApp, 0);
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}
