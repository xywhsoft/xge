#include "xge.h"
#include "xui.h"
#include "mapedit_xrt.h"
#include "mapedit_history.h"
#include "mapedit_theme.h"
#include "mapedit_document.h"
#include "mapedit_files.h"
#include "mapedit_view.h"
#include "map_sdk/xge_map.h"

#include <limits.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <shlobj.h>

#if defined(MAPEDIT_FORCE_DISCRETE_GPU)
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_LEFT_ALT 342
#define XGE_KEY_LEFT_SUPER 343
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#define XGE_KEY_RIGHT_ALT 346
#define XGE_KEY_RIGHT_SUPER 347
#endif

#define MAPEDIT_W 1400
#define MAPEDIT_H 900
#define MAPEDIT_MENU_H 34.0f
#define MAPEDIT_SWITCH_H 46.0f
#define MAPEDIT_STATUS_H 32.0f
#define MAPEDIT_TILE_W gMapeditTileWidth
#define MAPEDIT_TILE_H gMapeditTileHeight
#define MAPEDIT_MAP_PASSAGE_CELL 32
#define MAPEDIT_TILES_PER_ROW gMapeditTilesPerRow
#define MAPEDIT_MAP_LAYER_MAX 8
#define MAPEDIT_PATH_MAX 512
#define MAPEDIT_NAME_MAX 128
#define MAPEDIT_FILE_MAX 256
#define MAPEDIT_LIST_TEXT_MAX 512
#define MAPEDIT_FILE_LIST_HARD_MAX 1048576
#define MAPEDIT_SPECIAL_MAX 512
/* History storage and limits live in mapedit_history.h. */
#define MAPEDIT_TOOL_COUNT 6
#define MAPEDIT_CUSTOM_CHANNEL_MAX 16
#define MAPEDIT_CUSTOM_OPTION_MAX 32
#define MAPEDIT_CUSTOM_VALUE_MAX 64
#define MAPEDIT_TAG_FORM_H 148.0f
#define MAPEDIT_BLOB47_COLS 8
#define MAPEDIT_BLOB47_ROWS 6
#define MAPEDIT_BLOB47_COUNT 47
#define MAPEDIT_MAP_PREVIEW_FRAME_SECONDS 0.25f
#define MAPEDIT_BLOB47_N 0x01
#define MAPEDIT_BLOB47_E 0x02
#define MAPEDIT_BLOB47_S 0x04
#define MAPEDIT_BLOB47_W 0x08
#define MAPEDIT_BLOB47_NE 0x10
#define MAPEDIT_BLOB47_SE 0x20
#define MAPEDIT_BLOB47_SW 0x40
#define MAPEDIT_BLOB47_NW 0x80
#define MAPEDIT_MATERIAL_MODE_STATIC 0
#define MAPEDIT_MATERIAL_MODE_OTHER 1
#define MAPEDIT_MATERIAL_STATIC_COLS 20
#define MAPEDIT_MATERIAL_INITIAL_ROWS 64
#define MAPEDIT_MATERIAL_EXTEND_ROWS 32
#define MAPEDIT_TILE_DIM_MAX 1024
#define MAPEDIT_TILES_PER_ROW_MAX 4096
#define MAPEDIT_MATERIAL_OUTPUT_BYTE_MAX ((size_t)128u * 1024u * 1024u)
#define MAPEDIT_MATERIAL_STATIC_PANE_WIDTH 356.0f
#define MAPEDIT_TILE_SELECT_PLACEHOLDER_ROWS 8
#define MAPEDIT_MAP_EDIT_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_TILEGRID_BG XUI_COLOR_RGBA(236, 246, 252, 255)
#define MAPEDIT_TILEGRID_BORDER XUI_COLOR_RGBA(124, 181, 219, 255)
#define MAPEDIT_TILEGRID_GRID XUI_COLOR_RGBA(188, 224, 244, 150)
#define MAPEDIT_TILEGRID_GRID_TAGS XUI_COLOR_RGBA(188, 224, 244, 130)
#define MAPEDIT_TILESET_PASSAGE_DISPLAY_CELL 32
#define MAPEDIT_TILESET_PROPERTY_MODE_SET 0
#define MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL 1
#define MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM 2
#define MAPEDIT_MAP_PROPERTY_MODE_MAP 0
#define MAPEDIT_MAP_PROPERTY_MODE_CELL_CUSTOM 1

static int gMapeditTileWidth = 16;
static int gMapeditTileHeight = 16;
static int gMapeditTilesPerRow = 20;
static char gMapeditMapIoError[256];

#define MAP_KEY_NAME "地图名称"
#define MAP_KEY_TILESET "图集"
#define MAP_KEY_STATE "当前状态"
#define MAP_KEY_WIDTH "地图宽度"
#define MAP_KEY_HEIGHT "地图高度"
#define MAP_KEY_CUSTOM "自定义数据"
#define MAP_KEY_TILES "图块数据"
#define MAP_KEY_PASSAGE "通行修正"
#define MAP_KEY_CELL_DATA "cellCustomData"
#define MAP_KEY_CELL "cell"
#define MAP_KEY_VALUE "value"

#define SET_KEY_STATIC "静态图块集"
#define SET_KEY_FORMAT "format"
#define SET_KEY_VERSION "version"
#define SET_KEY_NAME "图集名称"
#define SET_KEY_SPECIAL_COUNT "特殊图块数量"
#define SET_KEY_SPECIAL_TILES "特殊图块"
#define SET_KEY_SPECIAL_TYPE "特殊图块类型"
#define SET_KEY_SPECIAL_FILE "特殊图块文件名"
#define SET_KEY_PASSAGE "通行数据"
#define SET_KEY_ACTOR "角色覆盖数据"
#define SET_KEY_CUSTOM "自定义数据"
#define SET_KEY_TILE_DATA "tileCustomData"
#define SET_KEY_TILE "tile"
#define MAPEDIT_TILESET_FORMAT_NAME "xge.tileset"
#define MAPEDIT_TILESET_FORMAT_VERSION 2

enum {
	MAPEDIT_WORKSPACE_TILESET = 0,
	MAPEDIT_WORKSPACE_MAP = 1
};

enum {
	MAPEDIT_TOOL_BRUSH = 0,
	MAPEDIT_TOOL_ERASER,
	MAPEDIT_TOOL_LINE,
	MAPEDIT_TOOL_RECT,
	MAPEDIT_TOOL_CIRCLE,
	MAPEDIT_TOOL_BUCKET
};

enum {
	CMD_FILE_OPEN = 100,
	CMD_FILE_SAVE,
	CMD_FILE_SAVE_AS,
	CMD_FILE_RECOVER,
	CMD_FILE_EXIT,
	CMD_VIEW_RESET_LAYOUT,
	CMD_HELP_ABOUT,
	CMD_VIEW_ACTUAL_SIZE,
	CMD_VIEW_FIT,
	CMD_WS_TILESET = 200,
	CMD_WS_MAP,
	CMD_TOOL_BRUSH = 300,
	CMD_TOOL_ERASER,
	CMD_TOOL_LINE,
	CMD_TOOL_RECT,
	CMD_TOOL_CIRCLE,
	CMD_TOOL_BUCKET,
	CMD_PREVIEW,
	CMD_GRID,
	CMD_UNDO,
	CMD_REDO,
	CMD_MATERIAL_VIEW = 500,
	CMD_MATERIAL_RENAME,
	CMD_MATERIAL_EDIT,
	CMD_MATERIAL_SOURCE_IMAGE
};

typedef struct mapedit_file_list_t {
	char (*arrNames)[MAPEDIT_FILE_MAX];
	char (*arrText)[MAPEDIT_LIST_TEXT_MAX];
	const char** arrPtrs;
	const char** arrTextPtrs;
	int iCount;
	int iSelected;
	int iCapacity;
} mapedit_file_list_t;

typedef struct mapedit_material_category_t {
	const char* sTitle;
	const char* sDir;
	const char* sMapFile;
} mapedit_material_category_t;

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

static const char* g_arrTilesetSpecialTypeNames[] = {
	"动态图块",
	"自动图块",
	"多状态图块",
	"多状态自动图块",
	"对象"
};

typedef struct mapedit_setup_layer_t {
	int iId;
	char sName[MAPEDIT_NAME_MAX];
	int bVisible;
	int bEditable;
	int bAboveActor;
} mapedit_setup_layer_t;

typedef struct mapedit_custom_option_t {
	char sText[MAPEDIT_NAME_MAX];
	char sValue[MAPEDIT_CUSTOM_VALUE_MAX];
	int iBit;
	int bHasBit;
} mapedit_custom_option_t;

typedef struct mapedit_custom_channel_def_t {
	char sId[MAPEDIT_NAME_MAX];
	char sName[MAPEDIT_NAME_MAX];
	char sScope[32];
	char sDataType[32];
	char sMarkMode[32];
	char sDefaultValue[MAPEDIT_CUSTOM_VALUE_MAX];
	mapedit_custom_option_t arrOptions[MAPEDIT_CUSTOM_OPTION_MAX];
	const char* arrOptionItems[MAPEDIT_CUSTOM_OPTION_MAX];
	int iOptionCount;
	int bHasMin;
	int bHasMax;
	int iMinValue;
	int iMaxValue;
} mapedit_custom_channel_def_t;

typedef xge_map_passage_override_t mapedit_map_passage_override_t;
typedef xge_map_t mapedit_map_doc_t;

typedef struct mapedit_app_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	int iTargetWidth;
	int iTargetHeight;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pSwitchBar;
	xui_widget pFileMenu;
	xui_widget pViewMenu;
	xui_widget pHelpMenu;
	xui_widget pTileButton;
	xui_widget pMapButton;
	xui_widget pTilesetDock;
	xui_widget pMapDock;
	xui_widget pStatus;
	xui_widget pMapToolbar;
	xui_widget pMapCommandToolbar;
	xui_widget pLayerCombo;
	xui_widget pMapScrollFrame;
	xui_widget pMapCanvas;
	xui_widget pTileSelectScrollFrame;
	xui_widget pTileSelectCanvas;
	xui_widget pTilesetArrangeScrollFrame;
	xui_widget pTilesetArrangeCanvas;
	xui_widget pTilesetPassageScrollFrame;
	xui_widget pTilesetPassageCanvas;
	xui_widget pTilesetActorScrollFrame;
	xui_widget pTilesetActorCanvas;
	xui_widget pTilesetTagsScrollFrame;
	xui_widget pTilesetTagsCanvas;
	xui_widget pTilesetTagsPanel;
	xui_widget pTilesetTagsChannelCombo;
	xui_widget pTilesetTagsInspectCheck;
	xui_widget pTilesetTagsFormGrid;
	xui_widget pMapPassageScrollFrame;
	xui_widget pMapPassageCanvas;
	xui_widget pMapTagsScrollFrame;
	xui_widget pMapTagsCanvas;
	xui_widget pMapTagsPanel;
	xui_widget pMapTagsChannelCombo;
	xui_widget pMapTagsInspectCheck;
	xui_widget pMapTagsFormGrid;
	xui_widget pMapListView;
	xui_widget pTilesetListView;
	xui_widget pMaterialListView;
	xui_widget pMaterialCategoryCombo;
	xui_widget pMapListPanel;
	xui_widget pTilesetListPanel;
	xui_widget pMaterialListPanel;
	xui_widget pMapAddButton;
	xui_widget pMapSaveButton;
	xui_widget pTilesetAddButton;
	xui_widget pTilesetSaveButton;
	xui_widget pMaterialCreateButton;
	xui_widget pMaterialRefreshButton;
	xui_widget pMaterialPreviewCanvas;
	xui_widget pMaterialContextMenu;
	xui_widget pMaterialViewWindow;
	xui_widget pMaterialViewScroll;
	xui_widget pMaterialViewCanvas;
	xui_widget pMaterialRenameWindow;
	xui_widget pMaterialRenameInput;
	xui_widget pMaterialRenameOkButton;
	xui_widget pMaterialRenameCancelButton;
	xui_widget pMaterialEditWindow;
	xui_widget pMaterialEditSplit;
	xui_widget pMaterialEditOutputScroll;
	xui_widget pMaterialEditSourceScroll;
	xui_widget pMaterialEditOutputCanvas;
	xui_widget pMaterialEditSourceCanvas;
	xui_widget pMaterialEditNameInput;
	xui_widget pMaterialEditFileInput;
	xui_widget pMaterialEditLoadButton;
	xui_widget pMaterialEditOkButton;
	xui_widget pMaterialEditCancelButton;
	xui_msgtip pMaterialEditMsgTip;
	xui_widget pMapPropertyGrid;
	xui_widget pTilesetPropertyGrid;
	mapedit_file_list_t tMapFiles;
	mapedit_file_list_t tTilesetFiles;
	mapedit_file_list_t tMaterialFiles;
	mapedit_file_list_t tMaterialCategoryFiles;
	mapedit_file_list_t tMapTilesetOptions;
	mapedit_file_list_t tTilesetSpecialFileOptions;
	mapedit_setup_layer_t arrSetupLayers[MAPEDIT_MAP_LAYER_MAX];
	const char* arrLayerNames[MAPEDIT_MAP_LAYER_MAX];
	int iSetupLayerCount;
	int iSetupTileWidth;
	int iSetupTileHeight;
	int iSetupTilesPerRow;
	int iSetupStateMin;
	int iSetupStateMax;
	mapedit_custom_channel_def_t arrCustomChannels[MAPEDIT_CUSTOM_CHANNEL_MAX];
	int iCustomChannelCount;
	int iTilesetTagChannel;
	int iMapTagChannel;
	int iTilesetTagsSelectedTile;
	int iMapTagsSelectedCell;
	int iTilesetPropertyMode;
	int iTilesetPropertyTile;
	int iMapPropertyMode;
	int iMapPropertyCell;
	xui_combobox_item_t arrTilesetTagChannelItems[MAPEDIT_CUSTOM_CHANNEL_MAX];
	xui_combobox_item_t arrMapTagChannelItems[MAPEDIT_CUSTOM_CHANNEL_MAX];
	char arrTilesetTagChannelText[MAPEDIT_CUSTOM_CHANNEL_MAX][MAPEDIT_NAME_MAX + MAPEDIT_CUSTOM_VALUE_MAX];
	char arrMapTagChannelText[MAPEDIT_CUSTOM_CHANNEL_MAX][MAPEDIT_NAME_MAX + MAPEDIT_CUSTOM_VALUE_MAX];
	char sTilesetTagValue[MAPEDIT_CUSTOM_VALUE_MAX];
	char sMapTagValue[MAPEDIT_CUSTOM_VALUE_MAX];
	int iTilesetTagChannelOptionCount;
	int iMapTagChannelOptionCount;
	int bTilesetTagsInspectMode;
	int bMapTagsInspectMode;
	int bTilesetTagsFormUpdating;
	int bMapTagsFormUpdating;
	int bTilesetPropertyUpdating;
	int bMapPropertyUpdating;
	mapedit_map_doc_t tMap;
	mapedit_tileset_t tTileset;
	xui_surface pMaterialPreviewSurface;
	xui_surface_desc_t tMaterialPreviewDesc;
	xui_surface pMaterialTooltipSurface;
	xui_surface_desc_t tMaterialTooltipDesc;
	xui_surface pMaterialViewSurface;
	xui_surface_desc_t tMaterialViewDesc;
	char sMaterialViewTitle[MAPEDIT_LIST_TEXT_MAX];
	xui_surface pMaterialEditSourceSurface;
	xui_surface_desc_t tMaterialEditSourceDesc;
	xui_surface pMaterialEditOutputSurface;
	xui_surface_desc_t tMaterialEditOutputDesc;
	xge_image_t tMaterialEditSourceImage;
	unsigned char* pMaterialEditOutputPixels;
	int iMaterialEditMode;
	int iMaterialEditIndex;
	int iMaterialEditOutputCols;
	int iMaterialEditOutputRows;
	int iMaterialEditUsedCols;
	int iMaterialEditUsedRows;
	int iMaterialEditSourceCols;
	int iMaterialEditSourceRows;
	int iMaterialEditSelCol;
	int iMaterialEditSelRow;
	int iMaterialEditSelCols;
	int iMaterialEditSelRows;
	int iMaterialEditDragCol;
	int iMaterialEditDragRow;
	int bMaterialEditSelecting;
	char sMaterialEditOriginalFile[MAPEDIT_FILE_MAX];
	int iMaterialCategory;
	int iMaterialContextIndex;
	int iMaterialRenameIndex;
	int iMaterialTooltipIndex;
	char sAppDir[MAPEDIT_PATH_MAX];
	char sProjectDir[MAPEDIT_PATH_MAX];
	char sUserDir[MAPEDIT_PATH_MAX];
	xui_file_dialog pFileDialog;
	int iFileDialogCommand;
	int iFileDialogWorkspace;
	xui_msgbox pConfirmBox;
	xui_msgbox pExternalBox;
	int iExternalDocument;
	int iPendingOperation, iPendingDiscardMask, iPendingDocument;
	char sPendingPath[MAPEDIT_PATH_MAX];
	mapedit_file_stamp_t tMapStamp, tTilesetStamp;
	double fNextFileCheck;
	int bMapExternalConflict, bTilesetExternalConflict;
	float fDpiScale;
	int iMapLayoutRevision, iTilesetLayoutRevision;
	double fLayoutDirtyAt;
	double fMapDirtySince;
	double fLastRecoveryWrite;
	char sStatusText[256];
	char sStatusMapText[64];
	char sStatusTileText[160];
	char sStatusPositionText[160];
	HANDLE hSingleInstance;
	FILE* pStartupLog;
	int iFrame;
	int iFrameLimit;
	int iStartupWorkspace;
	int iLargeMapWidth;
	int iLargeMapHeight;
	int bSmokeWorkbench, bSmokeWorkbenchOK, iSmokeWorkbenchStage;
	int bSmokeCustomDefault;
	int bSmokeCustomDefaultOK;
	float arrViewZoom[8];
	int iLastView, iPanView, bViewPanning, iPanX, iPanY;
	xui_widget pPanWidget;
	char sCapturePath[MAPEDIT_PATH_MAX];
	int iActiveWorkspace;
	int iActiveTool;
	int iSelectedTile;
	int iBrushW;
	int iBrushH;
	int iMapHoverX;
	int iMapHoverY;
	int iMapTagsHoverX;
	int iMapTagsHoverY;
	int iTileSelectHoverCol;
	int iTileSelectHoverRow;
	int iTileSelectAnchorCol;
	int iTileSelectAnchorRow;
	int bTileSelectDragging;
	int iTilesetArrangeHoverTile;
	int iTilesetTagsHoverTile;
	int iActiveLayer;
	int iTilesetArrangeSelectedTile;
	int iTilesetPassageSelectedTile;
	int iTilesetActorSelectedTile;
	int iMapPassageSelectedCell;
	int bPreview;
	int bGrid;
	float fPreviewAnimTime;
	int iPreviewAnimFrame;
	int bPainting;
	int bBatchEdit;
	int bHistoryFailed;
	int bGestureDirty;
	int iDragStartX;
	int iDragStartY;
	int iDragCurrentX;
	int iDragCurrentY;
	int iLastPaintX;
	int iLastPaintY;
	float fMapScrollX;
	float fMapScrollY;
	float fMapTagsScrollX;
	float fMapTagsScrollY;
	float fMapPassageScrollX;
	float fMapPassageScrollY;
	float fTileScrollX;
	float fTileScrollY;
	float fTilesetArrangeScrollX;
	float fTilesetArrangeScrollY;
	float fTilesetPassageScrollX;
	float fTilesetPassageScrollY;
	float fTilesetActorScrollX;
	float fTilesetActorScrollY;
	float fTilesetTagsScrollX;
	float fTilesetTagsScrollY;
	mapedit_history_t tMapHistory;
	mapedit_history_t tTilesetHistory;
	int bCreateOK;
	int bLayoutOK;
	int bDataOK;
	int bMapOK;
	int bTilesetOK;
	int bRenderOK;
	int bEditOK;
	int iLastVisibleCells;
} mapedit_app_t;

static int mapedit_min_i(int a, int b) { return a < b ? a : b; }
static int mapedit_max_i(int a, int b) { return a > b ? a : b; }
static float mapedit_clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static void mapedit_copy_text(char* sDst, int iCap, const char* sSrc);
static int mapedit_ensure_dir_utf8(const char* sPath);
static void mapedit_status(mapedit_app_t* pApp, const char* sText);
static void mapedit_update_window_title(mapedit_app_t* pApp);
static int mapedit_material_view_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
static int mapedit_material_edit_output_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
static int mapedit_material_edit_source_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
static int mapedit_file_exists_utf8(const char* sPath);
static void mapedit_material_preview_load(mapedit_app_t* pApp);
static void mapedit_material_tooltip_clear(mapedit_app_t* pApp);
static void mapedit_material_display_name(mapedit_app_t* pApp, int iIndex, char* sOut, int iCap);
static int mapedit_material_open_editor(mapedit_app_t* pApp, int iIndex);
static void mapedit_material_edit_close(mapedit_app_t* pApp);
static void mapedit_refresh_toolbar_state(mapedit_app_t* pApp);
static void mapedit_activate_loaded_map(mapedit_app_t* pApp, const char* sStatus);
static void mapedit_invalidate_map_views(mapedit_app_t* pApp);
static void mapedit_invalidate_tileset_views(mapedit_app_t* pApp);
static int mapedit_show_material_source_dialog(mapedit_app_t* pApp);
static int mapedit_show_map_file_dialog(mapedit_app_t* pApp, int iCommand);
static void mapedit_pending_continue(mapedit_app_t* pApp);
static int mapedit_save_current(mapedit_app_t* pApp);
static void mapedit_zoom_command(mapedit_app_t* pApp, int fit);
static int mapedit_view_interact(mapedit_app_t* pApp, xui_widget widget, const xui_event_t* event);
static void mapedit_request_operation(mapedit_app_t* pApp, int operation, const char* path);
static void mapedit_clear_recovery_for(mapedit_app_t* pApp, int workspace, const char* path);
static void mapedit_tileset_selected(xui_widget pWidget, int iIndex, void* pUser);

static int mapedit_arg_int(const char* sText, int iDefault)
{
	int v;
	if ( sText == NULL || sText[0] == 0 ) return iDefault;
	v = atoi(sText);
	return v > 0 ? v : iDefault;
}

static int mapedit_parse_size_arg(const char* sText, int* pW, int* pH)
{
	int w = 0;
	int h = 0;
	if ( sText == NULL || pW == NULL || pH == NULL ) return 0;
	if ( sscanf(sText, "%dx%d", &w, &h) != 2 && sscanf(sText, "%d,%d", &w, &h) != 2 ) return 0;
	if ( w <= 0 || h <= 0 ) return 0;
	*pW = w;
	*pH = h;
	return 1;
}

static int mapedit_has_ext(const char* sName, const char* sExt)
{
	size_t nName;
	size_t nExt;
	if ( sName == NULL || sExt == NULL ) return 0;
	nName = strlen(sName);
	nExt = strlen(sExt);
	return nName >= nExt && _stricmp(sName + nName - nExt, sExt) == 0;
}

static void mapedit_path_join(char* sOut, int iCap, const char* sA, const char* sB)
{
	size_t n;
	if ( sOut == NULL || iCap <= 0 ) return;
	if ( sA == NULL ) sA = "";
	if ( sB == NULL ) sB = "";
	n = strlen(sA);
	if ( n > 0 && (sA[n - 1] == '\\' || sA[n - 1] == '/') ) {
		snprintf(sOut, (size_t)iCap, "%s%s", sA, sB);
	} else {
		snprintf(sOut, (size_t)iCap, "%s\\%s", sA, sB);
	}
}

static void mapedit_app_path(mapedit_app_t* pApp, char* sOut, int iCap, const char* sRel)
{
	const char* sBase;
	const char* sUseRel;
	if ( sOut == NULL || iCap <= 0 ) return;
	if ( pApp == NULL ) {
		sOut[0] = 0;
		return;
	}
	sUseRel = sRel != NULL ? sRel : "";
	sBase = pApp->sAppDir;
	if ( strncmp(sUseRel, "assets\\", 7) == 0 || strcmp(sUseRel, "assets") == 0 ||
	     strcmp(sUseRel, "option\\setup.xson") == 0 ) {
		sBase = pApp->sProjectDir[0] != 0 ? pApp->sProjectDir : pApp->sAppDir;
	} else if ( strncmp(sUseRel, "option\\layout_", 14) == 0 ) {
		sBase = pApp->sUserDir[0] != 0 ? pApp->sUserDir : pApp->sAppDir;
		sUseRel += 7;
	}
	mapedit_path_join(sOut, iCap, sBase, sUseRel);
}

static int mapedit_utf8_to_wide(const char* sText, wchar_t* sOut, int iCap)
{
	int n;
	if ( sOut == NULL || iCap <= 0 ) return 0;
	sOut[0] = 0;
	if ( sText == NULL ) sText = "";
	n = MultiByteToWideChar(CP_UTF8, 0, sText, -1, sOut, iCap);
	return n > 0;
}

static int mapedit_wide_to_utf8(const wchar_t* sText, char* sOut, int iCap)
{
	int n;
	if ( sOut == NULL || iCap <= 0 ) return 0;
	sOut[0] = 0;
	if ( sText == NULL ) sText = L"";
	n = WideCharToMultiByte(CP_UTF8, 0, sText, -1, sOut, iCap, NULL, NULL);
	return n > 0;
}

static void mapedit_get_user_dir(mapedit_app_t* pApp)
{
	wchar_t wBase[MAPEDIT_PATH_MAX];
	char base[MAPEDIT_PATH_MAX];
	DWORD n;
	if ( pApp == NULL ) return;
	n = GetEnvironmentVariableW(L"LOCALAPPDATA", wBase, (DWORD)(sizeof(wBase) / sizeof(wBase[0])));
	if ( n == 0 || n >= (DWORD)(sizeof(wBase) / sizeof(wBase[0])) ||
	     !mapedit_wide_to_utf8(wBase, base, (int)sizeof(base)) ) {
		mapedit_copy_text(base, sizeof(base), pApp->sAppDir);
	}
	mapedit_path_join(pApp->sUserDir, sizeof(pApp->sUserDir), base, "XGE\\MapEdit");
}

static void mapedit_set_project_dir(mapedit_app_t* pApp, const char* sPath)
{
	wchar_t wInput[MAPEDIT_PATH_MAX];
	wchar_t wFull[MAPEDIT_PATH_MAX];
	DWORD n;
	if ( pApp == NULL ) return;
	if ( sPath == NULL || sPath[0] == 0 ||
	     !mapedit_utf8_to_wide(sPath, wInput, (int)(sizeof(wInput) / sizeof(wInput[0]))) ) {
		mapedit_copy_text(pApp->sProjectDir, sizeof(pApp->sProjectDir), pApp->sAppDir);
		return;
	}
	n = GetFullPathNameW(wInput, (DWORD)(sizeof(wFull) / sizeof(wFull[0])), wFull, NULL);
	if ( n == 0 || n >= (DWORD)(sizeof(wFull) / sizeof(wFull[0])) ||
	     !mapedit_wide_to_utf8(wFull, pApp->sProjectDir, (int)sizeof(pApp->sProjectDir)) ) {
		mapedit_copy_text(pApp->sProjectDir, sizeof(pApp->sProjectDir), pApp->sAppDir);
	}
}

static void mapedit_get_app_dir(char* sOut, int iCap)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	DWORD n;
	char* p;
	if ( sOut == NULL || iCap <= 0 ) return;
	n = GetModuleFileNameW(NULL, wPath, (DWORD)(sizeof(wPath) / sizeof(wPath[0])));
	if ( n == 0 || n >= (DWORD)(sizeof(wPath) / sizeof(wPath[0])) || !mapedit_wide_to_utf8(wPath, sOut, iCap) ) {
		snprintf(sOut, (size_t)iCap, ".");
		return;
	}
	p = strrchr(sOut, '\\');
	if ( p == NULL ) p = strrchr(sOut, '/');
	if ( p != NULL ) *p = 0;
}

static void mapedit_configure_process_startup(const char* sAppDir)
{
	wchar_t wDir[MAPEDIT_PATH_MAX];
	SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
	SetEnvironmentVariableA("SHIM_MCCOMPAT", "0x800000001");
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	if ( mapedit_utf8_to_wide(sAppDir, wDir, (int)(sizeof(wDir) / sizeof(wDir[0]))) ) {
		SetDllDirectoryW(wDir);
	}
}

static void mapedit_startup_log_open(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX + 64];
	if ( pApp == NULL || pApp->sUserDir[0] == 0 ) return;
	snprintf(path, sizeof(path), "%s\\mapedit_startup.log", pApp->sUserDir);
	pApp->pStartupLog = fopen(path, "ab");
}

static void mapedit_startup_log(mapedit_app_t* pApp, const char* sFmt, ...)
{
	va_list args;
	if ( pApp == NULL || pApp->pStartupLog == NULL || sFmt == NULL ) return;
	fprintf(pApp->pStartupLog, "pid=%lu tick=%lu ", (unsigned long)GetCurrentProcessId(), (unsigned long)GetTickCount());
	va_start(args, sFmt);
	vfprintf(pApp->pStartupLog, sFmt, args);
	va_end(args);
	fputc('\n', pApp->pStartupLog);
	fflush(pApp->pStartupLog);
}

static BOOL CALLBACK mapedit_find_window_proc(HWND hWnd, LPARAM lParam)
{
	wchar_t title[64];
	HWND* pFound = (HWND*)lParam;
	if ( pFound == NULL || !IsWindowVisible(hWnd) ) return TRUE;
	if ( GetWindowTextW(hWnd, title, (int)(sizeof(title) / sizeof(title[0]))) <= 0 ) return TRUE;
	if ( wcsncmp(title, L"MapEdit", 7u) != 0 ) return TRUE;
	*pFound = hWnd;
	return FALSE;
}

static HWND mapedit_find_existing_window(void)
{
	HWND hWnd = NULL;
	(void)EnumWindows(mapedit_find_window_proc, (LPARAM)&hWnd);
	return hWnd;
}

static int mapedit_acquire_single_instance(mapedit_app_t* pApp)
{
	DWORD err;
	HWND hWnd;
	if ( pApp == NULL ) return 0;
	pApp->hSingleInstance = CreateMutexW(NULL, TRUE, L"Local\\xge_xui2_mapedit_single_instance");
	if ( pApp->hSingleInstance == NULL ) {
		mapedit_startup_log(pApp, "single-instance mutex create failed err=%lu", (unsigned long)GetLastError());
		return 1;
	}
	err = GetLastError();
	if ( err != ERROR_ALREADY_EXISTS ) {
		mapedit_startup_log(pApp, "single-instance acquired");
		return 1;
	}
	mapedit_startup_log(pApp, "single-instance already running");
	hWnd = mapedit_find_existing_window();
	if ( hWnd != NULL ) {
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
	}
	CloseHandle(pApp->hSingleInstance);
	pApp->hSingleInstance = NULL;
	return 0;
}

static void mapedit_release_single_instance(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->hSingleInstance != NULL ) {
		ReleaseMutex(pApp->hSingleInstance);
		CloseHandle(pApp->hSingleInstance);
		pApp->hSingleInstance = NULL;
	}
	if ( pApp->pStartupLog != NULL ) {
		mapedit_startup_log(pApp, "shutdown");
		fclose(pApp->pStartupLog);
		pApp->pStartupLog = NULL;
	}
}

typedef struct mapedit_file_row_t {
	char sName[MAPEDIT_FILE_MAX];
	char sText[MAPEDIT_LIST_TEXT_MAX];
} mapedit_file_row_t;

static void mapedit_file_list_rebuild_ptrs(mapedit_file_list_t* pList)
{
	int i;
	if ( pList == NULL ) return;
	for ( i = 0; i < pList->iCount; ++i ) {
		pList->arrPtrs[i] = pList->arrNames[i];
		pList->arrTextPtrs[i] = pList->arrText[i];
	}
}

static void mapedit_file_list_clear(mapedit_file_list_t* pList)
{
	if ( pList == NULL ) return;
	free(pList->arrNames);
	free(pList->arrText);
	free(pList->arrPtrs);
	free(pList->arrTextPtrs);
	memset(pList, 0, sizeof(*pList));
	pList->iSelected = -1;
}

static int mapedit_file_list_reserve(mapedit_file_list_t* pList, int iNeed)
{
	char (*arrNames)[MAPEDIT_FILE_MAX];
	char (*arrText)[MAPEDIT_LIST_TEXT_MAX];
	const char** arrPtrs;
	const char** arrTextPtrs;
	int capacity;
	if ( pList == NULL || iNeed < 0 || iNeed > MAPEDIT_FILE_LIST_HARD_MAX ) return 0;
	if ( iNeed <= pList->iCapacity ) return 1;
	capacity = pList->iCapacity > 0 ? pList->iCapacity : 32;
	while ( capacity < iNeed ) {
		if ( capacity > MAPEDIT_FILE_LIST_HARD_MAX / 2 ) {
			capacity = MAPEDIT_FILE_LIST_HARD_MAX;
			break;
		}
		capacity *= 2;
	}
	arrNames = (char (*)[MAPEDIT_FILE_MAX])calloc((size_t)capacity, sizeof(*arrNames));
	arrText = (char (*)[MAPEDIT_LIST_TEXT_MAX])calloc((size_t)capacity, sizeof(*arrText));
	arrPtrs = (const char**)calloc((size_t)capacity, sizeof(*arrPtrs));
	arrTextPtrs = (const char**)calloc((size_t)capacity, sizeof(*arrTextPtrs));
	if ( arrNames == NULL || arrText == NULL || arrPtrs == NULL || arrTextPtrs == NULL ) {
		free(arrNames);
		free(arrText);
		free(arrPtrs);
		free(arrTextPtrs);
		return 0;
	}
	if ( pList->iCount > 0 ) {
		memcpy(arrNames, pList->arrNames, (size_t)pList->iCount * sizeof(*arrNames));
		memcpy(arrText, pList->arrText, (size_t)pList->iCount * sizeof(*arrText));
	}
	free(pList->arrNames);
	free(pList->arrText);
	free(pList->arrPtrs);
	free(pList->arrTextPtrs);
	pList->arrNames = arrNames;
	pList->arrText = arrText;
	pList->arrPtrs = arrPtrs;
	pList->arrTextPtrs = arrTextPtrs;
	pList->iCapacity = capacity;
	mapedit_file_list_rebuild_ptrs(pList);
	return 1;
}

static int mapedit_file_list_append(mapedit_file_list_t* pList, const char* sName, const char* sText)
{
	int i;
	if ( pList == NULL || sName == NULL || !mapedit_file_list_reserve(pList, pList->iCount + 1) ) return 0;
	i = pList->iCount++;
	mapedit_copy_text(pList->arrNames[i], MAPEDIT_FILE_MAX, sName);
	mapedit_copy_text(pList->arrText[i], MAPEDIT_LIST_TEXT_MAX, sText != NULL ? sText : sName);
	pList->arrPtrs[i] = pList->arrNames[i];
	pList->arrTextPtrs[i] = pList->arrText[i];
	return 1;
}

static int mapedit_file_row_compare(const void* pLeft, const void* pRight)
{
	const mapedit_file_row_t* pA = (const mapedit_file_row_t*)pLeft;
	const mapedit_file_row_t* pB = (const mapedit_file_row_t*)pRight;
	return _stricmp(pA->sName, pB->sName);
}

static void mapedit_file_list_sort(mapedit_file_list_t* pList)
{
	mapedit_file_row_t* arrRows;
	int i;
	if ( pList == NULL || pList->iCount < 2 ) return;
	arrRows = (mapedit_file_row_t*)malloc((size_t)pList->iCount * sizeof(*arrRows));
	if ( arrRows == NULL ) return;
	for ( i = 0; i < pList->iCount; ++i ) {
		mapedit_copy_text(arrRows[i].sName, sizeof(arrRows[i].sName), pList->arrNames[i]);
		mapedit_copy_text(arrRows[i].sText, sizeof(arrRows[i].sText), pList->arrText[i]);
	}
	qsort(arrRows, (size_t)pList->iCount, sizeof(*arrRows), mapedit_file_row_compare);
	for ( i = 0; i < pList->iCount; ++i ) {
		mapedit_copy_text(pList->arrNames[i], MAPEDIT_FILE_MAX, arrRows[i].sName);
		mapedit_copy_text(pList->arrText[i], MAPEDIT_LIST_TEXT_MAX, arrRows[i].sText);
	}
	free(arrRows);
	mapedit_file_list_rebuild_ptrs(pList);
}

static void mapedit_scan_files(const char* sPath, const char* sExt, mapedit_file_list_t* pList)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	wchar_t wPattern[MAPEDIT_PATH_MAX];
	WIN32_FIND_DATAW tFind;
	HANDLE hFind;
	if ( pList == NULL ) return;
	mapedit_file_list_clear(pList);
	if ( !mapedit_utf8_to_wide(sPath, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) return;
	_snwprintf(wPattern, sizeof(wPattern) / sizeof(wPattern[0]), L"%ls\\*", wPath);
	wPattern[(sizeof(wPattern) / sizeof(wPattern[0])) - 1] = 0;
	hFind = FindFirstFileW(wPattern, &tFind);
	if ( hFind == INVALID_HANDLE_VALUE ) return;
	do {
		char sName[MAPEDIT_FILE_MAX];
		if ( tFind.cFileName[0] == L'.' || (tFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) continue;
		if ( !mapedit_wide_to_utf8(tFind.cFileName, sName, (int)sizeof(sName)) ) continue;
		if ( !mapedit_has_ext(sName, sExt) ) continue;
		if ( !mapedit_file_list_append(pList, sName, sName) ) break;
	} while ( FindNextFileW(hFind, &tFind) );
	FindClose(hFind);
	mapedit_file_list_sort(pList);
	if ( pList->iCount > 0 ) pList->iSelected = 0;
}

static int mapedit_dir_exists_utf8(const char* sPath)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	DWORD attr;
	if ( !mapedit_utf8_to_wide(sPath, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) return 0;
	attr = GetFileAttributesW(wPath);
	return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int mapedit_ensure_dir_utf8(const char* sPath)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	int result;
	if ( sPath == NULL || sPath[0] == 0 ) return 0;
	if ( mapedit_dir_exists_utf8(sPath) ) return 1;
	if ( !mapedit_utf8_to_wide(sPath, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) return 0;
	result = SHCreateDirectoryExW(NULL, wPath, NULL);
	if ( result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS || result == ERROR_FILE_EXISTS ) return 1;
	return mapedit_dir_exists_utf8(sPath);
}

static int mapedit_prepare_project_dirs(mapedit_app_t* pApp)
{
	static const char* const arrDirs[] = {
		"assets",
		"assets\\maps",
		"assets\\图块集",
		"option"
	};
	char path[MAPEDIT_PATH_MAX];
	int i;
	if ( pApp == NULL || pApp->sProjectDir[0] == 0 ) return 0;
	if ( !mapedit_ensure_dir_utf8(pApp->sProjectDir) ) return 0;
	for ( i = 0; i < (int)(sizeof(arrDirs) / sizeof(arrDirs[0])); ++i ) {
		mapedit_path_join(path, sizeof(path), pApp->sProjectDir, arrDirs[i]);
		if ( !mapedit_ensure_dir_utf8(path) ) return 0;
	}
	return 1;
}

static int mapedit_is_image_file(const char* sName)
{
	const char* ext;
	char lower[16];
	int i;
	if ( sName == NULL ) return 0;
	ext = strrchr(sName, '.');
	if ( ext == NULL ) return 0;
	for ( i = 0; i < (int)sizeof(lower) - 1 && ext[i] != 0; i++ ) {
		char ch = ext[i];
		if ( ch >= 'A' && ch <= 'Z' ) ch = (char)(ch - 'A' + 'a');
		lower[i] = ch;
	}
	lower[i] = 0;
	return strcmp(lower, ".png") == 0 ||
	       strcmp(lower, ".jpg") == 0 ||
	       strcmp(lower, ".jpeg") == 0 ||
	       strcmp(lower, ".bmp") == 0 ||
	       strcmp(lower, ".tga") == 0 ||
	       strcmp(lower, ".webp") == 0;
}

static int mapedit_material_category_count(void)
{
	return (int)(sizeof(g_arrMaterialCategories) / sizeof(g_arrMaterialCategories[0]));
}

static int mapedit_material_category_clamp(int iCategory)
{
	int count = mapedit_material_category_count();
	if ( iCategory < 0 || iCategory >= count ) return 0;
	return iCategory;
}

static xvalue* mapedit_material_load_map(const char* sPath)
{
	xvalue* map = NULL;
	if ( sPath != NULL && mapedit_file_exists_utf8(sPath) ) map = xrtXsonParseFile(sPath);
	if ( map == NULL || xuiXrtValueType(map) != XVALUE_OBJECT ) {
		if ( map != NULL ) xrtValueRelease(map);
		map = xrtValueObject();
	}
	return map;
}

static int mapedit_material_scan_category(mapedit_app_t* pApp, int iCategory, int bCollectItems)
{
	const mapedit_material_category_t* pCategory;
	mapedit_file_list_t* pList;
	wchar_t wPath[MAPEDIT_PATH_MAX];
	wchar_t wPattern[MAPEDIT_PATH_MAX];
	WIN32_FIND_DATAW tFind;
	HANDLE hFind;
	char dir[MAPEDIT_PATH_MAX];
	char mapPath[MAPEDIT_PATH_MAX];
	char rel[MAPEDIT_PATH_MAX];
	xvalue* pMap;
	int changed;
	iCategory = mapedit_material_category_clamp(iCategory);
	pCategory = &g_arrMaterialCategories[iCategory];
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sDir);
	mapedit_app_path(pApp, dir, sizeof(dir), rel);
	mapedit_app_path(pApp, mapPath, sizeof(mapPath), "assets");
	(void)mapedit_ensure_dir_utf8(mapPath);
	(void)mapedit_ensure_dir_utf8(dir);
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sMapFile);
	mapedit_app_path(pApp, mapPath, sizeof(mapPath), rel);
	pMap = mapedit_material_load_map(mapPath);
	if ( pMap == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pList = &pApp->tMaterialCategoryFiles;
	if ( bCollectItems ) {
		mapedit_material_tooltip_clear(pApp);
		mapedit_file_list_clear(pList);
		pApp->iMaterialCategory = iCategory;
	}
	changed = !mapedit_file_exists_utf8(mapPath);
	if ( mapedit_utf8_to_wide(dir, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) {
		_snwprintf(wPattern, sizeof(wPattern) / sizeof(wPattern[0]), L"%ls\\*", wPath);
		wPattern[(sizeof(wPattern) / sizeof(wPattern[0])) - 1] = 0;
		hFind = FindFirstFileW(wPattern, &tFind);
		if ( hFind != INVALID_HANDLE_VALUE ) {
			do {
				char sName[MAPEDIT_FILE_MAX];
				char sText[MAPEDIT_LIST_TEXT_MAX];
				const char* sDisplay;
				xvalue* pNameValue;
				if ( tFind.cFileName[0] == L'.' || (tFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) continue;
				if ( !mapedit_wide_to_utf8(tFind.cFileName, sName, (int)sizeof(sName)) ) continue;
				if ( !mapedit_is_image_file(sName) ) continue;
				pNameValue = xuiXrtValueObjectGet(pMap, sName, (uint32)strlen(sName));
				if ( pNameValue != NULL && xuiXrtValueType(pNameValue) == XVALUE_STRING ) {
					sDisplay = (const char*)xuiXrtValueGetText(pNameValue);
				} else {
					sDisplay = "未命名图块";
					if ( xuiXrtValueObjectSetText(pMap, sName, (uint32)strlen(sName), sDisplay, 0, TRUE) ) changed = 1;
				}
				if ( bCollectItems ) {
					snprintf(sText, sizeof(sText), "%s    %s", sDisplay, sName);
					if ( !mapedit_file_list_append(pList, sName, sText) ) break;
				}
			} while ( FindNextFileW(hFind, &tFind) );
			FindClose(hFind);
		}
	}
	if ( changed ) (void)xrtXsonStringifyFile(mapPath, pMap, true);
	xrtValueRelease(pMap);
	if ( bCollectItems ) {
		mapedit_file_list_sort(pList);
		if ( pList->iCount > 0 ) pList->iSelected = 0;
		if ( pApp->pMaterialListView != NULL ) {
			(void)xuiListViewSetItems(pApp->pMaterialListView, pList->arrTextPtrs, pList->iCount);
			(void)xuiListViewSetSelected(pApp->pMaterialListView, pList->iSelected);
			(void)xuiListViewSetScroll(pApp->pMaterialListView, 0.0f);
		}
		if ( pApp->pMaterialPreviewCanvas != NULL ) mapedit_material_preview_load(pApp);
	}
	return XUI_OK;
}

static void mapedit_material_ensure_all_maps(mapedit_app_t* pApp)
{
	int i;
	if ( pApp == NULL ) return;
	for ( i = 0; i < mapedit_material_category_count(); i++ ) (void)mapedit_material_scan_category(pApp, i, 0);
}

static int mapedit_file_list_find(mapedit_file_list_t* pList, const char* sName)
{
	int i;
	if ( pList == NULL || sName == NULL ) return -1;
	for ( i = 0; i < pList->iCount; i++ ) {
		if ( strcmp(pList->arrNames[i], sName) == 0 ) return i;
	}
	return -1;
}

static int mapedit_file_exists_utf8(const char* sPath)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	DWORD attr;
	if ( !mapedit_utf8_to_wide(sPath, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) return 0;
	attr = GetFileAttributesW(wPath);
	return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int mapedit_load_dock_layout(mapedit_app_t* pApp, xui_widget pDock, const char* sUserRel, const char* sDefaultRel)
{
	char path[MAPEDIT_PATH_MAX];
	if ( pApp == NULL || pDock == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	mapedit_app_path(pApp, path, sizeof(path), sUserRel);
	if ( mapedit_file_exists_utf8(path) && xuiDockPanelLoadXSONFile(pDock, path) == XUI_OK ) return XUI_OK;
	mapedit_app_path(pApp, path, sizeof(path), sDefaultRel);
	if ( mapedit_file_exists_utf8(path) && xuiDockPanelLoadXSONFile(pDock, path) == XUI_OK ) return XUI_OK;
	return XUI_ERROR_FILE_NOT_FOUND;
}

static void mapedit_load_layouts(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	(void)mapedit_load_dock_layout(pApp, pApp->pTilesetDock, "option\\layout_tileset.xson", "option\\default_layout_tileset.xson");
	(void)mapedit_load_dock_layout(pApp, pApp->pMapDock, "option\\layout_map.xson", "option\\default_layout_map.xson");
}

static int mapedit_load_default_layouts(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	int ret;
	if ( pApp == NULL || pApp->pTilesetDock == NULL || pApp->pMapDock == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = XUI_OK;
	mapedit_app_path(pApp, path, sizeof(path), "option\\default_layout_tileset.xson");
	if ( !mapedit_file_exists_utf8(path) || xuiDockPanelLoadXSONFile(pApp->pTilesetDock, path) != XUI_OK ) ret = XUI_ERROR_FILE_NOT_FOUND;
	mapedit_app_path(pApp, path, sizeof(path), "option\\default_layout_map.xson");
	if ( !mapedit_file_exists_utf8(path) || xuiDockPanelLoadXSONFile(pApp->pMapDock, path) != XUI_OK ) ret = XUI_ERROR_FILE_NOT_FOUND;
	return ret;
}

static void mapedit_save_layouts(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	if ( pApp == NULL ) return;
	mapedit_app_path(pApp, path, sizeof(path), "option\\layout_tileset.xson");
	if ( pApp->pTilesetDock != NULL ) (void)xuiDockPanelSaveXSONFile(pApp->pTilesetDock, path);
	mapedit_app_path(pApp, path, sizeof(path), "option\\layout_map.xson");
	if ( pApp->pMapDock != NULL ) (void)xuiDockPanelSaveXSONFile(pApp->pMapDock, path);
}

static void mapedit_poll_layout(mapedit_app_t* pApp)
{
	int mapRevision = xuiDockPanelGetLayoutChangeCount(pApp->pMapDock);
	int tilesetRevision = xuiDockPanelGetLayoutChangeCount(pApp->pTilesetDock);
	double now = xgeTimer();
	if (mapRevision != pApp->iMapLayoutRevision || tilesetRevision != pApp->iTilesetLayoutRevision) {
		pApp->iMapLayoutRevision = mapRevision;
		pApp->iTilesetLayoutRevision = tilesetRevision;
		pApp->fLayoutDirtyAt = now;
	}
	if (pApp->fLayoutDirtyAt > 0.0) {
		if (now - pApp->fLayoutDirtyAt >= 0.75) {
			mapedit_save_layouts(pApp);
			pApp->fLayoutDirtyAt = 0.0;
		} else xgeRenderRequestAfter((float)(0.75 - (now - pApp->fLayoutDirtyAt)));
	}
}

static xvalue* mapedit_table_get(xvalue* pRoot, const char* sKey)
{
	if ( pRoot == NULL || sKey == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) return NULL;
	return xuiXrtValueObjectGet(pRoot, sKey, (uint32)strlen(sKey));
}

static int mapedit_table_set_take(xvalue* pRoot, const char* sKey, xvalue** ppValue)
{
	xstrview key;
	if ( pRoot == NULL || sKey == NULL || ppValue == NULL || *ppValue == NULL ) return 0;
	key.Data = sKey;
	key.Size = strlen(sKey);
	return xrtValueObjectSetTake(pRoot, key, ppValue) ? 1 : 0;
}

static int mapedit_table_int(xvalue* pRoot, const char* sKey, int iDefault)
{
	xvalue* pValue = mapedit_table_get(pRoot, sKey);
	if ( pValue == NULL ) return iDefault;
	if ( xuiXrtValueType(pValue) == XVALUE_INT ) return (int)xuiXrtValueGetInt(pValue);
	if ( xuiXrtValueType(pValue) == XVALUE_FLOAT ) return (int)xuiXrtValueGetFloat(pValue);
	return iDefault;
}

static int mapedit_table_read_int_strict(xvalue* pRoot, const char* sKey, int iDefault, int* pOut)
{
	xvalue* pValue;
	long long value;
	if ( pOut == NULL ) return 0;
	pValue = mapedit_table_get(pRoot, sKey);
	if ( pValue == NULL ) {
		*pOut = iDefault;
		return 1;
	}
	if ( xuiXrtValueType(pValue) != XVALUE_INT ) return 0;
	value = (long long)xuiXrtValueGetInt(pValue);
	if ( value < INT_MIN || value > INT_MAX ) return 0;
	*pOut = (int)value;
	return 1;
}

static int mapedit_table_bool(xvalue* pRoot, const char* sKey, int iDefault)
{
	xvalue* pValue = mapedit_table_get(pRoot, sKey);
	if ( pValue == NULL ) return iDefault;
	if ( xuiXrtValueType(pValue) == XVALUE_BOOL ) return xuiXrtValueGetBool(pValue) ? 1 : 0;
	if ( xuiXrtValueType(pValue) == XVALUE_INT ) return xuiXrtValueGetInt(pValue) != 0;
	return iDefault;
}

static const char* mapedit_table_text(xvalue* pRoot, const char* sKey, const char* sDefault)
{
	xvalue* pValue = mapedit_table_get(pRoot, sKey);
	if ( pValue != NULL && xuiXrtValueType(pValue) == XVALUE_STRING ) return (const char*)xuiXrtValueGetText(pValue);
	return sDefault;
}

static int mapedit_table_read_text_strict(xvalue* pRoot, const char* sKey, const char* sDefault, const char** ppText)
{
	xvalue* pValue;
	if ( ppText == NULL ) return 0;
	pValue = mapedit_table_get(pRoot, sKey);
	if ( pValue == NULL ) {
		*ppText = sDefault != NULL ? sDefault : "";
		return 1;
	}
	if ( xuiXrtValueType(pValue) != XVALUE_STRING ) return 0;
	*ppText = (const char*)xuiXrtValueGetText(pValue);
	return 1;
}

static void mapedit_value_text(xvalue* pValue, char* sOut, int iCap, const char* sDefault)
{
	if ( sOut == NULL || iCap <= 0 ) return;
	if ( sDefault == NULL ) sDefault = "";
	sOut[0] = 0;
	if ( pValue == NULL ) {
		snprintf(sOut, (size_t)iCap, "%s", sDefault);
		return;
	}
	switch ( xuiXrtValueType(pValue) ) {
	case XVALUE_BOOL:
		snprintf(sOut, (size_t)iCap, "%s", xuiXrtValueGetBool(pValue) ? "true" : "false");
		break;
	case XVALUE_INT:
		snprintf(sOut, (size_t)iCap, "%lld", (long long)xuiXrtValueGetInt(pValue));
		break;
	case XVALUE_FLOAT:
		snprintf(sOut, (size_t)iCap, "%.6g", xuiXrtValueGetFloat(pValue));
		break;
	case XVALUE_STRING:
		snprintf(sOut, (size_t)iCap, "%s", (const char*)xuiXrtValueGetText(pValue));
		break;
	default:
		snprintf(sOut, (size_t)iCap, "%s", sDefault);
		break;
	}
}

static void mapedit_copy_text(char* sDst, int iCap, const char* sSrc)
{
	size_t n;
	if ( sDst == NULL || iCap <= 0 ) return;
	if ( sSrc == NULL ) sSrc = "";
	n = strlen(sSrc);
	if ( n >= (size_t)iCap ) n = (size_t)iCap - 1;
	memmove(sDst, sSrc, n);
	sDst[n] = 0;
}

static void mapedit_file_list_set_text(mapedit_file_list_t* pList, int iIndex, const char* sText)
{
	if ( pList == NULL || iIndex < 0 || iIndex >= pList->iCount ) return;
	mapedit_copy_text(pList->arrText[iIndex], MAPEDIT_LIST_TEXT_MAX, sText);
	pList->arrTextPtrs[iIndex] = pList->arrText[iIndex];
}

static void mapedit_refresh_map_list_texts(mapedit_app_t* pApp, mapedit_file_list_t* pList)
{
	char rel[MAPEDIT_PATH_MAX];
	char path[MAPEDIT_PATH_MAX];
	char text[MAPEDIT_LIST_TEXT_MAX];
	xvalue* pRoot;
	const char* sName;
	int i;
	if ( pApp == NULL || pList == NULL ) return;
	for ( i = 0; i < pList->iCount; i++ ) {
		snprintf(rel, sizeof(rel), "assets\\maps\\%s", pList->arrNames[i]);
		mapedit_app_path(pApp, path, sizeof(path), rel);
		pRoot = xrtXsonParseFile(path);
		sName = (pRoot != NULL && xuiXrtValueType(pRoot) == XVALUE_OBJECT) ? mapedit_table_text(pRoot, MAP_KEY_NAME, "未命名地图") : pList->arrNames[i];
		snprintf(text, sizeof(text), "%s    %s", sName, pList->arrNames[i]);
		mapedit_file_list_set_text(pList, i, text);
		xrtValueRelease(pRoot);
	}
}

static void mapedit_refresh_tileset_list_texts(mapedit_app_t* pApp, mapedit_file_list_t* pList)
{
	char rel[MAPEDIT_PATH_MAX];
	char path[MAPEDIT_PATH_MAX];
	char text[MAPEDIT_LIST_TEXT_MAX];
	xvalue* pRoot;
	const char* sName;
	const char* sStatic;
	int i;
	if ( pApp == NULL || pList == NULL ) return;
	for ( i = 0; i < pList->iCount; i++ ) {
		snprintf(rel, sizeof(rel), "assets\\图块集\\%s", pList->arrNames[i]);
		mapedit_app_path(pApp, path, sizeof(path), rel);
		pRoot = xrtXsonParseFile(path);
		if ( pRoot != NULL && xuiXrtValueType(pRoot) == XVALUE_OBJECT ) {
			sName = mapedit_table_text(pRoot, SET_KEY_NAME, pList->arrNames[i]);
			sStatic = mapedit_table_text(pRoot, SET_KEY_STATIC, "未设置");
			if ( sStatic != NULL && strcmp(sStatic, "未设置") != 0 ) snprintf(text, sizeof(text), "%s    %s", sName, sStatic);
			else snprintf(text, sizeof(text), "%s", sName);
		} else {
			snprintf(text, sizeof(text), "%s", pList->arrNames[i]);
		}
		mapedit_file_list_set_text(pList, i, text);
		xrtValueRelease(pRoot);
	}
}

static void mapedit_map_tileset_option_add(mapedit_app_t* pApp, const char* sText, const char* sValue)
{
	if ( pApp == NULL || sText == NULL ) return;
	(void)mapedit_file_list_append(&pApp->tMapTilesetOptions, sValue != NULL ? sValue : "", sText);
}

static void mapedit_map_tileset_options_build(mapedit_app_t* pApp, const char* sCurrentFile)
{
	char rel[MAPEDIT_PATH_MAX];
	char path[MAPEDIT_PATH_MAX];
	char text[MAPEDIT_LIST_TEXT_MAX];
	xvalue* pRoot;
	const char* sName;
	int i;
	int found;
	if ( pApp == NULL ) return;
	mapedit_file_list_clear(&pApp->tMapTilesetOptions);
	mapedit_map_tileset_option_add(pApp, "未设置", "");
	found = (sCurrentFile == NULL || sCurrentFile[0] == 0);
	for ( i = 0; i < pApp->tTilesetFiles.iCount; i++ ) {
		snprintf(rel, sizeof(rel), "assets\\图块集\\%s", pApp->tTilesetFiles.arrNames[i]);
		mapedit_app_path(pApp, path, sizeof(path), rel);
		pRoot = xrtXsonParseFile(path);
		sName = (pRoot != NULL && xuiXrtValueType(pRoot) == XVALUE_OBJECT) ? mapedit_table_text(pRoot, SET_KEY_NAME, pApp->tTilesetFiles.arrNames[i]) : pApp->tTilesetFiles.arrNames[i];
		snprintf(text, sizeof(text), "%s (%s)", sName, pApp->tTilesetFiles.arrNames[i]);
		mapedit_map_tileset_option_add(pApp, text, pApp->tTilesetFiles.arrNames[i]);
		if ( sCurrentFile != NULL && strcmp(sCurrentFile, pApp->tTilesetFiles.arrNames[i]) == 0 ) found = 1;
		xrtValueRelease(pRoot);
	}
	if ( !found && sCurrentFile != NULL && sCurrentFile[0] != 0 ) {
		snprintf(text, sizeof(text), "已丢失(%s)", sCurrentFile);
		mapedit_map_tileset_option_add(pApp, text, sCurrentFile);
	}
	mapedit_file_list_sort(&pApp->tMapTilesetOptions);
}

static int mapedit_map_tileset_option_find_value(mapedit_app_t* pApp, const char* sValue)
{
	if ( pApp == NULL ) return -1;
	if ( sValue == NULL ) sValue = "";
	return mapedit_file_list_find(&pApp->tMapTilesetOptions, sValue);
}

static int mapedit_map_tileset_option_find_text(mapedit_app_t* pApp, const char* sText)
{
	int i;
	if ( pApp == NULL || sText == NULL ) return -1;
	for ( i = 0; i < pApp->tMapTilesetOptions.iCount; i++ ) {
		if ( strcmp(pApp->tMapTilesetOptions.arrText[i], sText) == 0 ) return i;
	}
	return -1;
}

static const char* mapedit_map_tileset_display_for_value(mapedit_app_t* pApp, const char* sValue)
{
	int i = mapedit_map_tileset_option_find_value(pApp, sValue);
	return i >= 0 ? pApp->tMapTilesetOptions.arrText[i] : "未设置";
}

static const char* mapedit_map_tileset_value_for_display(mapedit_app_t* pApp, const char* sText)
{
	int i = mapedit_map_tileset_option_find_text(pApp, sText);
	if ( i >= 0 ) return pApp->tMapTilesetOptions.arrNames[i];
	i = mapedit_map_tileset_option_find_value(pApp, sText);
	return i >= 0 ? pApp->tMapTilesetOptions.arrNames[i] : "";
}

static int mapedit_tileset_special_type_count(void)
{
	return (int)(sizeof(g_arrTilesetSpecialTypeNames) / sizeof(g_arrTilesetSpecialTypeNames[0]));
}

static int mapedit_tileset_special_type_find(const char* sType)
{
	int i;
	if ( sType != NULL ) {
		for ( i = 0; i < mapedit_tileset_special_type_count(); i++ ) {
			if ( strcmp(sType, g_arrTilesetSpecialTypeNames[i]) == 0 ) return i;
		}
	}
	return 0;
}

static const char* mapedit_tileset_special_type_or_default(const char* sType)
{
	return g_arrTilesetSpecialTypeNames[mapedit_tileset_special_type_find(sType)];
}

static const mapedit_material_category_t* mapedit_tileset_special_category_for_type(const char* sType)
{
	const char* sNormalized;
	int i;
	sNormalized = mapedit_tileset_special_type_or_default(sType);
	for ( i = 0; i < mapedit_material_category_count(); i++ ) {
		if ( strcmp(g_arrMaterialCategories[i].sTitle, sNormalized) == 0 ) return &g_arrMaterialCategories[i];
	}
	return &g_arrMaterialCategories[1];
}

static void mapedit_tileset_special_file_option_add(mapedit_app_t* pApp, const char* sText, const char* sValue)
{
	if ( pApp == NULL || sText == NULL ) return;
	(void)mapedit_file_list_append(&pApp->tTilesetSpecialFileOptions, sValue != NULL ? sValue : "", sText);
}

static int mapedit_tileset_special_file_find_value(mapedit_app_t* pApp, const char* sValue)
{
	if ( pApp == NULL ) return -1;
	if ( sValue == NULL ) sValue = "";
	return mapedit_file_list_find(&pApp->tTilesetSpecialFileOptions, sValue);
}

static int mapedit_tileset_special_file_find_text(mapedit_app_t* pApp, const char* sText)
{
	int i;
	if ( pApp == NULL || sText == NULL ) return -1;
	for ( i = 0; i < pApp->tTilesetSpecialFileOptions.iCount; i++ ) {
		if ( strcmp(pApp->tTilesetSpecialFileOptions.arrText[i], sText) == 0 ) return i;
	}
	return -1;
}

static const char* mapedit_tileset_special_file_display_for_value(mapedit_app_t* pApp, const char* sValue)
{
	int i = mapedit_tileset_special_file_find_value(pApp, sValue);
	return i >= 0 ? pApp->tTilesetSpecialFileOptions.arrText[i] : "未设置";
}

static const char* mapedit_tileset_special_file_value_for_display(mapedit_app_t* pApp, const char* sText)
{
	int i = mapedit_tileset_special_file_find_text(pApp, sText);
	if ( i >= 0 ) return pApp->tTilesetSpecialFileOptions.arrNames[i];
	i = mapedit_tileset_special_file_find_value(pApp, sText);
	return i >= 0 ? pApp->tTilesetSpecialFileOptions.arrNames[i] : "";
}

static void mapedit_tileset_special_file_options_build(mapedit_app_t* pApp, const char* sType, const char* sCurrentFile)
{
	const mapedit_material_category_t* pCategory;
	wchar_t wPath[MAPEDIT_PATH_MAX];
	wchar_t wPattern[MAPEDIT_PATH_MAX];
	WIN32_FIND_DATAW tFind;
	HANDLE hFind;
	char dir[MAPEDIT_PATH_MAX];
	char mapPath[MAPEDIT_PATH_MAX];
	char rel[MAPEDIT_PATH_MAX];
	char text[MAPEDIT_LIST_TEXT_MAX];
	xvalue* pMap;
	int found;
	int changed;
	if ( pApp == NULL ) return;
	mapedit_file_list_clear(&pApp->tTilesetSpecialFileOptions);
	mapedit_tileset_special_file_option_add(pApp, "未设置", "");
	pCategory = mapedit_tileset_special_category_for_type(sType);
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sDir);
	mapedit_app_path(pApp, dir, sizeof(dir), rel);
	(void)mapedit_ensure_dir_utf8(dir);
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sMapFile);
	mapedit_app_path(pApp, mapPath, sizeof(mapPath), rel);
	pMap = mapedit_material_load_map(mapPath);
	if ( pMap == NULL ) return;
	found = (sCurrentFile == NULL || sCurrentFile[0] == 0);
	changed = !mapedit_file_exists_utf8(mapPath);
	if ( mapedit_utf8_to_wide(dir, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) {
		_snwprintf(wPattern, sizeof(wPattern) / sizeof(wPattern[0]), L"%ls\\*", wPath);
		wPattern[(sizeof(wPattern) / sizeof(wPattern[0])) - 1] = 0;
		hFind = FindFirstFileW(wPattern, &tFind);
		if ( hFind != INVALID_HANDLE_VALUE ) {
			do {
				char sName[MAPEDIT_FILE_MAX];
				const char* sDisplay;
				xvalue* pNameValue;
				if ( tFind.cFileName[0] == L'.' || (tFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) continue;
				if ( !mapedit_wide_to_utf8(tFind.cFileName, sName, (int)sizeof(sName)) ) continue;
				if ( !mapedit_is_image_file(sName) ) continue;
				pNameValue = xuiXrtValueObjectGet(pMap, sName, (uint32)strlen(sName));
				if ( pNameValue != NULL && xuiXrtValueType(pNameValue) == XVALUE_STRING ) {
					sDisplay = (const char*)xuiXrtValueGetText(pNameValue);
				} else {
					sDisplay = "未命名图块";
					if ( xuiXrtValueObjectSetText(pMap, sName, (uint32)strlen(sName), sDisplay, 0, TRUE) ) changed = 1;
				}
				snprintf(text, sizeof(text), "%s (%s)", sDisplay, sName);
				mapedit_tileset_special_file_option_add(pApp, text, sName);
				if ( sCurrentFile != NULL && strcmp(sCurrentFile, sName) == 0 ) found = 1;
			} while ( FindNextFileW(hFind, &tFind) );
			FindClose(hFind);
		}
	}
	if ( changed ) (void)xrtXsonStringifyFile(mapPath, pMap, true);
	xrtValueRelease(pMap);
	if ( !found && sCurrentFile != NULL && sCurrentFile[0] != 0 ) {
		snprintf(text, sizeof(text), "已丢失 (%s)", sCurrentFile);
		mapedit_tileset_special_file_option_add(pApp, text, sCurrentFile);
	}
	mapedit_file_list_sort(&pApp->tTilesetSpecialFileOptions);
}

static void mapedit_setup_default_layers(mapedit_app_t* pApp)
{
	static const char* defaults[3] = {"地表", "装饰", "上层"};
	int i;
	if ( pApp == NULL ) return;
	pApp->iSetupLayerCount = 3;
	for ( i = 0; i < MAPEDIT_MAP_LAYER_MAX; i++ ) {
		memset(&pApp->arrSetupLayers[i], 0, sizeof(pApp->arrSetupLayers[i]));
		pApp->arrSetupLayers[i].iId = i;
		pApp->arrSetupLayers[i].bVisible = 1;
		pApp->arrSetupLayers[i].bEditable = 1;
		pApp->arrSetupLayers[i].bAboveActor = i == 2;
		mapedit_copy_text(pApp->arrSetupLayers[i].sName, MAPEDIT_NAME_MAX, i < 3 ? defaults[i] : "图层");
		pApp->arrLayerNames[i] = pApp->arrSetupLayers[i].sName;
	}
}

static void mapedit_setup_load_custom_options(mapedit_custom_channel_def_t* pDef, xvalue* pArray, int bFlags)
{
	int i;
	int count;
	if ( pDef == NULL || pArray == NULL || xuiXrtValueType(pArray) != XVALUE_ARRAY ) return;
	count = mapedit_min_i((int)xrtValueCount(pArray), MAPEDIT_CUSTOM_OPTION_MAX);
	for ( i = 0; i < count; i++ ) {
		xvalue* pItem = xuiXrtValueArrayGet(pArray, (uint32)i);
		mapedit_custom_option_t* pOpt;
		if ( pItem == NULL || xuiXrtValueType(pItem) != XVALUE_OBJECT || pDef->iOptionCount >= MAPEDIT_CUSTOM_OPTION_MAX ) continue;
		pOpt = &pDef->arrOptions[pDef->iOptionCount++];
		memset(pOpt, 0, sizeof(*pOpt));
		if ( bFlags ) {
			pOpt->iBit = mapedit_table_int(pItem, "bit", i);
			pOpt->bHasBit = 1;
			snprintf(pOpt->sValue, sizeof(pOpt->sValue), "%d", pOpt->iBit);
		} else {
			mapedit_value_text(mapedit_table_get(pItem, "value"), pOpt->sValue, sizeof(pOpt->sValue), "0");
		}
		mapedit_copy_text(pOpt->sText, sizeof(pOpt->sText), mapedit_table_text(pItem, "name", pOpt->sValue));
		pDef->arrOptionItems[pDef->iOptionCount - 1] = pOpt->sText;
	}
}

static void mapedit_setup_load_custom_data(mapedit_app_t* pApp, xvalue* pRoot)
{
	xvalue* pCustom;
	xvalue* pChannels;
	int i;
	int count;
	if ( pApp == NULL || pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) return;
	pApp->iCustomChannelCount = 0;
	pApp->iTilesetTagChannel = -1;
	pApp->iMapTagChannel = -1;
	pCustom = mapedit_table_get(pRoot, "customData");
	if ( pCustom == NULL || xuiXrtValueType(pCustom) != XVALUE_OBJECT ) return;
	pChannels = mapedit_table_get(pCustom, "channels");
	if ( pChannels == NULL || xuiXrtValueType(pChannels) != XVALUE_ARRAY ) return;
	count = mapedit_min_i((int)xrtValueCount(pChannels), MAPEDIT_CUSTOM_CHANNEL_MAX);
	for ( i = 0; i < count; i++ ) {
		xvalue* pChannel = xuiXrtValueArrayGet(pChannels, (uint32)i);
		mapedit_custom_channel_def_t* pDef;
		const char* sId;
		if ( pChannel == NULL || xuiXrtValueType(pChannel) != XVALUE_OBJECT ) continue;
		sId = mapedit_table_text(pChannel, "id", "");
		if ( sId == NULL || sId[0] == 0 ) continue;
		pDef = &pApp->arrCustomChannels[pApp->iCustomChannelCount];
		memset(pDef, 0, sizeof(*pDef));
		mapedit_copy_text(pDef->sId, sizeof(pDef->sId), sId);
		mapedit_copy_text(pDef->sName, sizeof(pDef->sName), mapedit_table_text(pChannel, "name", sId));
		mapedit_copy_text(pDef->sScope, sizeof(pDef->sScope), mapedit_table_text(pChannel, "scope", "tile"));
		mapedit_copy_text(pDef->sDataType, sizeof(pDef->sDataType), mapedit_table_text(pChannel, "dataType", "int"));
		mapedit_copy_text(pDef->sMarkMode, sizeof(pDef->sMarkMode), mapedit_table_text(pChannel, "markMode", "paint"));
		mapedit_value_text(mapedit_table_get(pChannel, "defaultValue"), pDef->sDefaultValue, sizeof(pDef->sDefaultValue), "");
		pDef->bHasMin = mapedit_table_get(pChannel, "minValue") != NULL;
		pDef->bHasMax = mapedit_table_get(pChannel, "maxValue") != NULL;
		pDef->iMinValue = mapedit_table_int(pChannel, "minValue", 0);
		pDef->iMaxValue = mapedit_table_int(pChannel, "maxValue", 0);
		mapedit_setup_load_custom_options(pDef, mapedit_table_get(pChannel, "values"), 0);
		mapedit_setup_load_custom_options(pDef, mapedit_table_get(pChannel, "flags"), 1);
		if ( pApp->iTilesetTagChannel < 0 && strcmp(pDef->sScope, "tile") == 0 ) pApp->iTilesetTagChannel = pApp->iCustomChannelCount;
		if ( pApp->iMapTagChannel < 0 && (strcmp(pDef->sScope, "cell") == 0 || strcmp(pDef->sScope, "tile") == 0) ) pApp->iMapTagChannel = pApp->iCustomChannelCount;
		pApp->iCustomChannelCount++;
	}
}

static void mapedit_load_setup(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	xvalue* pRoot;
	xvalue* pTile;
	xvalue* pState;
	xvalue* pLayers;
	xvalue* pDefaults;
	int count;
	int i;
	if ( pApp == NULL ) return;
	pApp->iSetupTileWidth = 16;
	pApp->iSetupTileHeight = 16;
	pApp->iSetupTilesPerRow = 20;
	pApp->iSetupStateMin = 0;
	pApp->iSetupStateMax = 3;
	gMapeditTileWidth = pApp->iSetupTileWidth;
	gMapeditTileHeight = pApp->iSetupTileHeight;
	gMapeditTilesPerRow = pApp->iSetupTilesPerRow;
	mapedit_setup_default_layers(pApp);
	mapedit_app_path(pApp, path, sizeof(path), "option\\setup.xson");
	pRoot = xrtXsonParseFile(path);
	if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		return;
	}
	mapedit_setup_load_custom_data(pApp, pRoot);
	pTile = mapedit_table_get(pRoot, "tile");
	pApp->iSetupTileWidth = mapedit_table_int(pTile, "width", pApp->iSetupTileWidth);
	pApp->iSetupTileHeight = mapedit_table_int(pTile, "height", pApp->iSetupTileHeight);
	pApp->iSetupTilesPerRow = mapedit_table_int(pTile, "tilesPerRow", pApp->iSetupTilesPerRow);
	if ( pApp->iSetupTileWidth <= 0 || pApp->iSetupTileWidth > MAPEDIT_TILE_DIM_MAX ) pApp->iSetupTileWidth = 16;
	if ( pApp->iSetupTileHeight <= 0 || pApp->iSetupTileHeight > MAPEDIT_TILE_DIM_MAX ) pApp->iSetupTileHeight = 16;
	if ( pApp->iSetupTilesPerRow <= 0 || pApp->iSetupTilesPerRow > MAPEDIT_TILES_PER_ROW_MAX ) pApp->iSetupTilesPerRow = 20;
	gMapeditTileWidth = pApp->iSetupTileWidth;
	gMapeditTileHeight = pApp->iSetupTileHeight;
	gMapeditTilesPerRow = pApp->iSetupTilesPerRow;
	pState = mapedit_table_get(pRoot, "state");
	pApp->iSetupStateMin = mapedit_table_int(pState, "min", pApp->iSetupStateMin);
	pApp->iSetupStateMax = mapedit_table_int(pState, "max", pApp->iSetupStateMax);
	if ( pApp->iSetupStateMax < pApp->iSetupStateMin ) pApp->iSetupStateMax = pApp->iSetupStateMin;
	pLayers = mapedit_table_get(pRoot, "layers");
	if ( pLayers == NULL || xuiXrtValueType(pLayers) != XVALUE_OBJECT ) {
		xrtValueRelease(pRoot);
		return;
	}
	count = mapedit_table_int(pLayers, "count", pApp->iSetupLayerCount);
	if ( count < 1 ) count = 1;
	if ( count > MAPEDIT_MAP_LAYER_MAX ) count = MAPEDIT_MAP_LAYER_MAX;
	pApp->iSetupLayerCount = count;
	pDefaults = mapedit_table_get(pLayers, "defaults");
	if ( pDefaults != NULL && xuiXrtValueType(pDefaults) == XVALUE_ARRAY ) {
		int n = mapedit_min_i((int)xrtValueCount(pDefaults), count);
		for ( i = 0; i < n; i++ ) {
			xvalue* pLayer = xuiXrtValueArrayGet(pDefaults, (uint32)i);
			if ( pLayer == NULL || xuiXrtValueType(pLayer) != XVALUE_OBJECT ) continue;
			pApp->arrSetupLayers[i].iId = mapedit_table_int(pLayer, "id", i);
			mapedit_copy_text(pApp->arrSetupLayers[i].sName, MAPEDIT_NAME_MAX, mapedit_table_text(pLayer, "name", pApp->arrSetupLayers[i].sName));
			pApp->arrSetupLayers[i].bVisible = mapedit_table_bool(pLayer, "visible", pApp->arrSetupLayers[i].bVisible);
			pApp->arrSetupLayers[i].bEditable = mapedit_table_bool(pLayer, "editable", pApp->arrSetupLayers[i].bEditable);
			pApp->arrSetupLayers[i].bAboveActor = mapedit_table_bool(pLayer, "aboveActor", pApp->arrSetupLayers[i].bAboveActor);
			pApp->arrLayerNames[i] = pApp->arrSetupLayers[i].sName;
		}
	}
	xrtValueRelease(pRoot);
}

static mapedit_custom_channel_def_t* mapedit_custom_channel(mapedit_app_t* pApp, int iIndex)
{
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->iCustomChannelCount ) return NULL;
	return &pApp->arrCustomChannels[iIndex];
}

static mapedit_custom_channel_def_t* mapedit_custom_channel_by_id(mapedit_app_t* pApp, const char* sId)
{
	int i;
	if ( pApp == NULL || sId == NULL ) return NULL;
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		if ( strcmp(pApp->arrCustomChannels[i].sId, sId) == 0 ) return &pApp->arrCustomChannels[i];
	}
	return NULL;
}

static int mapedit_setup_clamp_state(mapedit_app_t* pApp, int iState)
{
	int minState;
	int maxState;
	if ( pApp == NULL ) return iState;
	minState = pApp->iSetupStateMin;
	maxState = pApp->iSetupStateMax;
	if ( maxState < minState ) maxState = minState;
	if ( iState < minState ) iState = minState;
	if ( iState > maxState ) iState = maxState;
	return iState;
}

static int mapedit_custom_is_tileset_channel(const mapedit_custom_channel_def_t* pDef)
{
	return pDef != NULL && pDef->sId[0] != 0 && (pDef->sScope[0] == 0 || strcmp(pDef->sScope, "tile") == 0);
}

static int mapedit_custom_is_map_channel(const mapedit_custom_channel_def_t* pDef)
{
	return pDef != NULL && pDef->sId[0] != 0 &&
		(pDef->sScope[0] == 0 || strcmp(pDef->sScope, "cell") == 0 || strcmp(pDef->sScope, "tile") == 0);
}

static int mapedit_custom_value_as_int(const char* sValue)
{
	if ( sValue == NULL || sValue[0] == 0 ) return 0;
	if ( strcmp(sValue, "true") == 0 ) return 1;
	if ( strcmp(sValue, "false") == 0 ) return 0;
	return atoi(sValue);
}

static void mapedit_custom_clamp_value(const mapedit_custom_channel_def_t* pDef, char* sValue, int iCap)
{
	int v;
	if ( pDef == NULL || sValue == NULL || iCap <= 0 ) return;
	if ( strcmp(pDef->sDataType, "string") == 0 || strcmp(pDef->sDataType, "text") == 0 ) return;
	if ( strcmp(pDef->sDataType, "float") == 0 || strcmp(pDef->sDataType, "double") == 0 ) return;
	if ( !pDef->bHasMin && !pDef->bHasMax ) return;
	v = mapedit_custom_value_as_int(sValue);
	if ( pDef->bHasMin && v < pDef->iMinValue ) v = pDef->iMinValue;
	if ( pDef->bHasMax && v > pDef->iMaxValue ) v = pDef->iMaxValue;
	snprintf(sValue, (size_t)iCap, "%d", v);
}

static int mapedit_custom_enum_find_value(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int i;
	if ( pDef == NULL || sValue == NULL ) return -1;
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		if ( strcmp(pDef->arrOptions[i].sValue, sValue) == 0 ) return i;
	}
	return -1;
}

static int mapedit_custom_enum_find_text(const mapedit_custom_channel_def_t* pDef, const char* sText)
{
	int i;
	if ( pDef == NULL || sText == NULL ) return -1;
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		if ( strcmp(pDef->arrOptions[i].sText, sText) == 0 ) return i;
	}
	return -1;
}

static const char* mapedit_custom_enum_display(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int i = mapedit_custom_enum_find_value(pDef, sValue);
	return i >= 0 ? pDef->arrOptions[i].sText : (sValue != NULL ? sValue : "");
}

static const char* mapedit_custom_enum_value(const mapedit_custom_channel_def_t* pDef, const char* sText)
{
	int i = mapedit_custom_enum_find_text(pDef, sText);
	if ( i >= 0 ) return pDef->arrOptions[i].sValue;
	i = mapedit_custom_enum_find_value(pDef, sText);
	return i >= 0 ? pDef->arrOptions[i].sValue : (sText != NULL ? sText : "");
}

static int mapedit_property_add(xui_widget pGrid, int iCategory, const char* sId, const char* sName, const char* sDesc, int iType, const char* sValue, const char* sDefault, int iFlags)
{
	xui_property_desc_t p;
	int existing = xuiPropertyGridFindProperty(pGrid, sId);
	if (existing >= 0) {
		const char* current = xuiPropertyGridGetValue(pGrid, existing);
		if (!current || strcmp(current, sValue ? sValue : "") != 0) (void)xuiPropertyGridSetValue(pGrid, existing, sValue);
		(void)xuiPropertyGridSetPropertyFlags(pGrid, existing, iFlags);
		return existing;
	}
	memset(&p, 0, sizeof(p));
	p.sId = sId;
	p.sName = sName;
	p.sDescription = sDesc;
	p.iType = iType;
	p.sValue = sValue;
	p.sDefaultValue = sDefault;
	p.iFlags = iFlags;
	return xuiPropertyGridAddProperty(pGrid, iCategory, &p);
}

static int mapedit_property_category(xui_widget grid, const char* id, const char* name, int expanded)
{
	int category = xuiPropertyGridFindCategory(grid, id);
	return category >= 0 ? category : xuiPropertyGridAddCategory(grid, id, name, expanded);
}

static mapedit_custom_channel_def_t* mapedit_tags_current_channel(mapedit_app_t* pApp, int bMap)
{
	return mapedit_custom_channel(pApp, bMap ? pApp->iMapTagChannel : pApp->iTilesetTagChannel);
}

static char* mapedit_tags_value_buffer(mapedit_app_t* pApp, int bMap)
{
	return bMap ? pApp->sMapTagValue : pApp->sTilesetTagValue;
}

static void mapedit_tags_use_default_value(mapedit_app_t* pApp, int bMap)
{
	mapedit_custom_channel_def_t* pDef;
	char* sValue;
	if ( pApp == NULL ) return;
	pDef = mapedit_tags_current_channel(pApp, bMap);
	sValue = mapedit_tags_value_buffer(pApp, bMap);
	mapedit_copy_text(sValue, MAPEDIT_CUSTOM_VALUE_MAX, pDef != NULL ? pDef->sDefaultValue : "");
}

static void mapedit_tags_sync_flag_properties(mapedit_app_t* pApp, int bMap, mapedit_custom_channel_def_t* pDef)
{
	xui_widget pGrid;
	char* sValue;
	char sId[48];
	int iValue;
	int iProp;
	int i;
	if ( pApp == NULL || pDef == NULL ) return;
	if ( bMap && pApp->bMapTagsFormUpdating ) return;
	if ( !bMap && pApp->bTilesetTagsFormUpdating ) return;
	pGrid = bMap ? pApp->pMapTagsFormGrid : pApp->pTilesetTagsFormGrid;
	if ( pGrid == NULL ) return;
	if ( bMap ) pApp->bMapTagsFormUpdating = 1;
	else pApp->bTilesetTagsFormUpdating = 1;
	sValue = mapedit_tags_value_buffer(pApp, bMap);
	iValue = mapedit_custom_value_as_int(sValue);
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sId, sizeof(sId), bMap ? "map.tag.flag.%d" : "tag.flag.%d", i);
		iProp = xuiPropertyGridFindProperty(pGrid, sId);
		if ( iProp >= 0 ) {
			(void)xuiPropertyGridSetBool(pGrid, iProp, (iValue & (1 << pDef->arrOptions[i].iBit)) != 0);
		}
	}
	if ( bMap ) pApp->bMapTagsFormUpdating = 0;
	else pApp->bTilesetTagsFormUpdating = 0;
}

static void mapedit_tags_build_form(mapedit_app_t* pApp, int bMap)
{
	mapedit_custom_channel_def_t* pDef;
	xui_widget pGrid;
	xui_table_grid_editor_config_t cfg;
	char* sValue;
	char sValueDisplay[MAPEDIT_CUSTOM_VALUE_MAX];
	char sFlagId[48];
	int cat;
	int type;
	int valueProp;
	int i;
	if ( pApp == NULL ) return;
	pGrid = bMap ? pApp->pMapTagsFormGrid : pApp->pTilesetTagsFormGrid;
	if ( pGrid == NULL ) return;
	if ( bMap ) pApp->bMapTagsFormUpdating = 1;
	else pApp->bTilesetTagsFormUpdating = 1;
	(void)xuiPropertyGridClear(pGrid);
	pDef = mapedit_tags_current_channel(pApp, bMap);
	if ( pDef == NULL ) {
		cat = xuiPropertyGridAddCategory(pGrid, bMap ? "map.tag.empty" : "tag.empty", bMap ? "地图打标" : "图块打标", 1);
		if ( cat >= 0 ) {
			(void)mapedit_property_add(pGrid, cat, bMap ? "map.tag.empty.hint" : "tag.empty.hint", "当前通道", bMap ? "setup.xson 中没有 scope=tile/cell 的 customData 通道" : "setup.xson 中没有 scope=tile 的 customData 通道", XUI_TABLE_CELL_TYPE_TEXT, "未配置", "未配置", XUI_PROPERTY_FLAG_READONLY);
		}
		if ( bMap ) pApp->bMapTagsFormUpdating = 0;
		else pApp->bTilesetTagsFormUpdating = 0;
		(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return;
	}
	sValue = mapedit_tags_value_buffer(pApp, bMap);
	if ( sValue[0] == 0 && pDef->sDefaultValue[0] != 0 ) mapedit_copy_text(sValue, MAPEDIT_CUSTOM_VALUE_MAX, pDef->sDefaultValue);
	mapedit_custom_clamp_value(pDef, sValue, MAPEDIT_CUSTOM_VALUE_MAX);
	cat = xuiPropertyGridAddCategory(pGrid, bMap ? "map.tag" : "tag", bMap ? "地图打标" : "图块打标", 1);
	if ( cat < 0 ) {
		if ( bMap ) pApp->bMapTagsFormUpdating = 0;
		else pApp->bTilesetTagsFormUpdating = 0;
		return;
	}
	(void)mapedit_property_add(pGrid, cat, bMap ? "map.tag.channel" : "tag.channel", "通道", pDef->sId, XUI_TABLE_CELL_TYPE_TEXT, pDef->sName[0] ? pDef->sName : pDef->sId, "", XUI_PROPERTY_FLAG_READONLY);
	(void)mapedit_property_add(pGrid, cat, bMap ? "map.tag.type" : "tag.type", "数据类型", pDef->sMarkMode, XUI_TABLE_CELL_TYPE_TEXT, pDef->sDataType, "", XUI_PROPERTY_FLAG_READONLY);
	type = XUI_TABLE_CELL_TYPE_INT;
	if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		type = XUI_TABLE_CELL_TYPE_ENUM;
		mapedit_copy_text(sValueDisplay, sizeof(sValueDisplay), mapedit_custom_enum_display(pDef, sValue));
	} else if ( strcmp(pDef->sDataType, "string") == 0 || strcmp(pDef->sDataType, "text") == 0 || strcmp(pDef->sMarkMode, "text") == 0 ) {
		type = XUI_TABLE_CELL_TYPE_TEXT;
		mapedit_copy_text(sValueDisplay, sizeof(sValueDisplay), sValue);
	} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
		type = XUI_TABLE_CELL_TYPE_BOOL;
		mapedit_copy_text(sValueDisplay, sizeof(sValueDisplay), mapedit_custom_value_as_int(sValue) ? "true" : "false");
	} else if ( strcmp(pDef->sDataType, "float") == 0 || strcmp(pDef->sDataType, "double") == 0 ) {
		type = XUI_TABLE_CELL_TYPE_FLOAT;
		mapedit_copy_text(sValueDisplay, sizeof(sValueDisplay), sValue);
	} else {
		mapedit_copy_text(sValueDisplay, sizeof(sValueDisplay), sValue);
	}
	valueProp = mapedit_property_add(pGrid, cat, bMap ? "map.tag.value" : "tag.value", pDef->sName[0] ? pDef->sName : "值", pDef->sId, type, sValueDisplay, pDef->sDefaultValue, 0);
	if ( valueProp >= 0 && type == XUI_TABLE_CELL_TYPE_ENUM ) {
		memset(&cfg, 0, sizeof(cfg));
		cfg.arrEnumItems = pDef->arrOptionItems;
		cfg.iEnumItemCount = pDef->iOptionCount;
		cfg.iEnumSelected = mapedit_custom_enum_find_value(pDef, sValue);
		(void)xuiPropertyGridSetEditorConfig(pGrid, valueProp, &cfg);
	}
	if ( strcmp(pDef->sMarkMode, "bitmask") == 0 && pDef->iOptionCount > 0 ) {
		int iValue = mapedit_custom_value_as_int(sValue);
		for ( i = 0; i < pDef->iOptionCount; i++ ) {
			snprintf(sFlagId, sizeof(sFlagId), bMap ? "map.tag.flag.%d" : "tag.flag.%d", i);
			(void)mapedit_property_add(pGrid, cat, sFlagId, pDef->arrOptions[i].sText, "bitmask", XUI_TABLE_CELL_TYPE_BOOL,
				(iValue & (1 << pDef->arrOptions[i].iBit)) ? "true" : "false", "false", 0);
		}
	}
	if ( valueProp >= 0 ) (void)xuiPropertyGridSetSelected(pGrid, valueProp);
	if ( bMap ) pApp->bMapTagsFormUpdating = 0;
	else pApp->bTilesetTagsFormUpdating = 0;
	(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tags_form_change(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	mapedit_custom_channel_def_t* pDef;
	char* sCurrent;
	char sFlagId[48];
	int bMap;
	int iValue;
	int i;
	(void)iType;
	if ( pApp == NULL || pWidget == NULL || sId == NULL ) return;
	bMap = pWidget == pApp->pMapTagsFormGrid;
	if ( bMap && pApp->bMapTagsFormUpdating ) return;
	if ( !bMap && pApp->bTilesetTagsFormUpdating ) return;
	pDef = mapedit_tags_current_channel(pApp, bMap);
	if ( pDef == NULL ) return;
	sCurrent = mapedit_tags_value_buffer(pApp, bMap);
	if ( strcmp(sId, bMap ? "map.tag.value" : "tag.value") == 0 ) {
		if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
			mapedit_copy_text(sCurrent, MAPEDIT_CUSTOM_VALUE_MAX, mapedit_custom_enum_value(pDef, sValue));
		} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
			mapedit_copy_text(sCurrent, MAPEDIT_CUSTOM_VALUE_MAX, mapedit_custom_value_as_int(sValue) ? "true" : "false");
		} else {
			mapedit_copy_text(sCurrent, MAPEDIT_CUSTOM_VALUE_MAX, sValue);
		}
		mapedit_custom_clamp_value(pDef, sCurrent, MAPEDIT_CUSTOM_VALUE_MAX);
		if ( strcmp(pDef->sMarkMode, "bitmask") == 0 ) mapedit_tags_sync_flag_properties(pApp, bMap, pDef);
		mapedit_status(pApp, bMap ? "地图打标值已修改" : "已更新打标值");
		return;
	}
	for ( i = 0; i < pDef->iOptionCount; i++ ) {
		snprintf(sFlagId, sizeof(sFlagId), bMap ? "map.tag.flag.%d" : "tag.flag.%d", i);
		if ( strcmp(sId, sFlagId) == 0 ) {
			iValue = mapedit_custom_value_as_int(sCurrent);
			if ( mapedit_custom_value_as_int(sValue) ) iValue |= (1 << pDef->arrOptions[i].iBit);
			else iValue &= ~(1 << pDef->arrOptions[i].iBit);
			snprintf(sCurrent, MAPEDIT_CUSTOM_VALUE_MAX, "%d", iValue);
			iProperty = xuiPropertyGridFindProperty(pWidget, bMap ? "map.tag.value" : "tag.value");
			if ( iProperty >= 0 ) {
				if ( bMap ) pApp->bMapTagsFormUpdating = 1;
				else pApp->bTilesetTagsFormUpdating = 1;
				(void)xuiPropertyGridSetValue(pWidget, iProperty, sCurrent);
				if ( bMap ) pApp->bMapTagsFormUpdating = 0;
				else pApp->bTilesetTagsFormUpdating = 0;
			}
			mapedit_status(pApp, bMap ? "地图打标位已修改" : "已更新打标位");
			return;
		}
	}
}

static void mapedit_tags_update_channel_combo(mapedit_app_t* pApp, int bMap)
{
	xui_widget pCombo;
	xui_combobox_item_t* pItems;
	char (*arrText)[MAPEDIT_NAME_MAX + MAPEDIT_CUSTOM_VALUE_MAX];
	mapedit_custom_channel_def_t* pDef;
	int i;
	int out = 0;
	int selected = -1;
	if ( pApp == NULL ) return;
	pCombo = bMap ? pApp->pMapTagsChannelCombo : pApp->pTilesetTagsChannelCombo;
	if ( pCombo == NULL ) return;
	pItems = bMap ? pApp->arrMapTagChannelItems : pApp->arrTilesetTagChannelItems;
	arrText = bMap ? pApp->arrMapTagChannelText : pApp->arrTilesetTagChannelText;
	for ( i = 0; i < pApp->iCustomChannelCount && out < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( bMap ? !mapedit_custom_is_map_channel(pDef) : !mapedit_custom_is_tileset_channel(pDef) ) continue;
		snprintf(arrText[out], MAPEDIT_NAME_MAX + MAPEDIT_CUSTOM_VALUE_MAX, "%s (%s)", pDef->sName[0] ? pDef->sName : pDef->sId, pDef->sId);
		pItems[out].sText = arrText[out];
		pItems[out].iValue = i;
		pItems[out].bEnabled = 1;
		pItems[out].bSeparator = 0;
		pItems[out].iIcon = 0;
		pItems[out].pUser = NULL;
		if ( (bMap ? pApp->iMapTagChannel : pApp->iTilesetTagChannel) == i ) selected = out;
		out++;
	}
	if ( bMap ) pApp->iMapTagChannelOptionCount = out;
	else pApp->iTilesetTagChannelOptionCount = out;
	if ( out <= 0 ) {
		if ( bMap ) pApp->iMapTagChannel = -1;
		else pApp->iTilesetTagChannel = -1;
		(void)xuiComboBoxSetItemData(pCombo, NULL, 0);
		mapedit_tags_build_form(pApp, bMap);
		return;
	}
	if ( selected < 0 ) {
		selected = 0;
		if ( bMap ) pApp->iMapTagChannel = pItems[0].iValue;
		else pApp->iTilesetTagChannel = pItems[0].iValue;
		mapedit_tags_use_default_value(pApp, bMap);
	}
	(void)xuiComboBoxSetItemData(pCombo, pItems, out);
	(void)xuiComboBoxSetSelected(pCombo, selected);
	mapedit_tags_build_form(pApp, bMap);
}

static void mapedit_tags_channel_select(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int bMap;
	(void)iIndex;
	if ( pApp == NULL || pWidget == NULL ) return;
	bMap = pWidget == pApp->pMapTagsChannelCombo;
	if ( iValue < 0 || iValue >= pApp->iCustomChannelCount ) return;
	if ( bMap ) pApp->iMapTagChannel = iValue;
	else pApp->iTilesetTagChannel = iValue;
	mapedit_tags_use_default_value(pApp, bMap);
	mapedit_tags_build_form(pApp, bMap);
	if ( bMap && pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( !bMap && pApp->pTilesetTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_status(pApp, bMap ? "地图打标通道已切换" : "已切换打标通道");
}

static void mapedit_tags_inspect_change(xui_widget pWidget, int bChecked, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int bMap;
	if ( pApp == NULL || pWidget == NULL ) return;
	bMap = pWidget == pApp->pMapTagsInspectCheck;
	if ( bMap ) pApp->bMapTagsInspectMode = bChecked ? 1 : 0;
	else pApp->bTilesetTagsInspectMode = bChecked ? 1 : 0;
	mapedit_status(pApp, bMap ? (pApp->bMapTagsInspectMode ? "地图打标：检查模式" : "地图打标：写入模式") : (pApp->bTilesetTagsInspectMode ? "图块打标：检查模式" : "图块打标：写入模式"));
}

static int mapedit_custom_get_value(xvalue* pRoot, const char* sChannel, const char* sIndexKey, int iIndex, char* sOut, int iCap)
{
	xvalue* pArray;
	int i;
	int count;
	if ( sOut != NULL && iCap > 0 ) sOut[0] = 0;
	if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT || sChannel == NULL || sIndexKey == NULL ) return 0;
	pArray = mapedit_table_get(pRoot, sChannel);
	if ( pArray == NULL || xuiXrtValueType(pArray) != XVALUE_ARRAY ) return 0;
	count = (int)xrtValueCount(pArray);
	for ( i = 0; i < count; i++ ) {
		xvalue* pEntry = xuiXrtValueArrayGet(pArray, (uint32)i);
		if ( pEntry == NULL || xuiXrtValueType(pEntry) != XVALUE_OBJECT ) continue;
		if ( mapedit_table_int(pEntry, sIndexKey, -1) == iIndex ) {
			mapedit_value_text(mapedit_table_get(pEntry, MAP_KEY_VALUE), sOut, iCap, "");
			return 1;
		}
	}
	return 0;
}

static int mapedit_custom_remove_value(xvalue* pRoot, const char* sChannel, const char* sIndexKey, int iIndex)
{
	xvalue* pArray;
	int i;
	int count;
	if ( sChannel == NULL || sChannel[0] == 0 || sIndexKey == NULL || iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) return XUI_OK;
	pArray = mapedit_table_get(pRoot, sChannel);
	if ( pArray == NULL || xuiXrtValueType(pArray) != XVALUE_ARRAY ) return XUI_OK;
	count = (int)xrtValueCount(pArray);
	for ( i = 0; i < count; i++ ) {
		xvalue* pEntry = xuiXrtValueArrayGet(pArray, (uint32)i);
		if ( pEntry == NULL || xuiXrtValueType(pEntry) != XVALUE_OBJECT ) continue;
		if ( mapedit_table_int(pEntry, sIndexKey, -1) == iIndex ) {
			return xrtValueArrayRemove(pArray, (uint32)i, 1) ? XUI_OK : XUI_ERROR;
		}
	}
	return XUI_OK;
}

static int mapedit_custom_set_value(xvalue** ppRoot, const char* sChannel, const char* sIndexKey, int iIndex, const char* sValue)
{
	xvalue* pRoot;
	xvalue* pArray;
	xvalue* pEntry;
	int i;
	int count;
	if ( ppRoot == NULL || sChannel == NULL || sChannel[0] == 0 || sIndexKey == NULL || iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sValue == NULL ) sValue = "";
	pRoot = *ppRoot;
	if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		pRoot = xrtValueObject();
		if ( pRoot == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		*ppRoot = pRoot;
	}
	pArray = mapedit_table_get(pRoot, sChannel);
	if ( pArray != NULL && xuiXrtValueType(pArray) == XVALUE_ARRAY ) {
		count = (int)xrtValueCount(pArray);
		for ( i = 0; i < count; i++ ) {
			pEntry = xuiXrtValueArrayGet(pArray, (uint32)i);
			if ( pEntry == NULL || xuiXrtValueType(pEntry) != XVALUE_OBJECT ) continue;
			if ( mapedit_table_int(pEntry, sIndexKey, -1) == iIndex ) {
				return xuiXrtValueObjectSetText(pEntry, MAP_KEY_VALUE, (uint32)strlen(MAP_KEY_VALUE), sValue, 0, FALSE) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
			}
		}
		pEntry = xrtValueObject();
		if ( pEntry == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		if ( !xuiXrtValueObjectSetInt(pEntry, sIndexKey, (uint32)strlen(sIndexKey), iIndex) ||
		     !xuiXrtValueObjectSetText(pEntry, MAP_KEY_VALUE, (uint32)strlen(MAP_KEY_VALUE), sValue, 0, FALSE) ||
		     !xuiXrtValueArrayAppendTake(pArray, pEntry, TRUE) ) {
			xrtValueRelease(pEntry);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		return XUI_OK;
	}
	pArray = xrtValueArray();
	pEntry = xrtValueObject();
	if ( pArray == NULL || pEntry == NULL ) {
		if ( pArray != NULL ) xrtValueRelease(pArray);
		if ( pEntry != NULL ) xrtValueRelease(pEntry);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetInt(pEntry, sIndexKey, (uint32)strlen(sIndexKey), iIndex) ||
	     !xuiXrtValueObjectSetText(pEntry, MAP_KEY_VALUE, (uint32)strlen(MAP_KEY_VALUE), sValue, 0, FALSE) ) {
		xrtValueRelease(pEntry);
		xrtValueRelease(pArray);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueArrayAppendTake(pArray, pEntry, TRUE) ) {
		xrtValueRelease(pEntry);
		xrtValueRelease(pArray);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetTake(pRoot, sChannel, (uint32)strlen(sChannel), pArray, TRUE) ) {
		xrtValueRelease(pArray);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int mapedit_map_cell_id(const mapedit_map_doc_t* pMap, int x, int y)
{
	if ( pMap == NULL || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) return -1;
	return y * pMap->iWidth + x;
}

static int mapedit_map_find_passage_override(mapedit_map_doc_t* pMap, int iCellId)
{
	return xgeMapFindPassageOverride(pMap, iCellId);
}

static void mapedit_map_remove_passage_override(mapedit_map_doc_t* pMap, int iIndex)
{
	(void)xgeMapRemovePassageOverride(pMap, iIndex);
}

static int mapedit_map_set_passage_override(mapedit_map_doc_t* pMap, int iCellId, unsigned char iValue)
{
	int ret = xgeMapSetPassageOverride(pMap, iCellId, iValue);
	if ( ret == XGE_MAP_OK ) return XUI_OK;
	return ret == XGE_MAP_ERROR_OUT_OF_MEMORY ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR_INVALID_ARGUMENT;
}

static void mapedit_map_clear(mapedit_map_doc_t* pMap)
{
	if ( pMap == NULL ) return;
	xgeMapSetDefault(pMap);
}

static int mapedit_map_alloc(mapedit_map_doc_t* pMap)
{
	int ret;
	if ( pMap == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pMap->iTileWidth = MAPEDIT_TILE_W;
	pMap->iTileHeight = MAPEDIT_TILE_H;
	ret = xgeMapAllocate(pMap);
	if ( ret == XGE_MAP_OK ) return XUI_OK;
	return ret == XGE_MAP_ERROR_OUT_OF_MEMORY ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR_INVALID_ARGUMENT;
}

static int mapedit_map_load(mapedit_map_doc_t* pMap, const char* sPath)
{
	int ret;
	gMapeditMapIoError[0] = 0;
	ret = xgeMapLoadXson(pMap, sPath, gMapeditMapIoError, sizeof(gMapeditMapIoError));
	if ( ret == XGE_MAP_OK ) return XUI_OK;
	return ret == XGE_MAP_ERROR_OUT_OF_MEMORY ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR;
}

static int mapedit_map_save(mapedit_map_doc_t* pMap, const char* sPath)
{
	int ret;
	if ( pMap == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pMap->iTileWidth = MAPEDIT_TILE_W;
	pMap->iTileHeight = MAPEDIT_TILE_H;
	gMapeditMapIoError[0] = 0;
	ret = xgeMapSaveXson(pMap, sPath, gMapeditMapIoError, sizeof(gMapeditMapIoError));
	if ( ret == XGE_MAP_OK ) return XUI_OK;
	return ret == XGE_MAP_ERROR_OUT_OF_MEMORY ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR;
}

static void mapedit_tileset_clear(mapedit_app_t* pApp)
{
	int i;
	xvalue* pTileCustomRaw;
	if ( pApp == NULL ) return;
	pTileCustomRaw = pApp->tTileset.pTileCustomRaw;
	if ( pApp->tTileset.pStaticSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tTileset.pStaticSurface);
	}
	for ( i = 0; i < pApp->tTileset.iSpecialCount && i < MAPEDIT_SPECIAL_MAX; i++ ) {
		if ( pApp->tTileset.arrSpecial[i].pSurface != NULL ) {
			pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tTileset.arrSpecial[i].pSurface);
		}
	}
	if ( pTileCustomRaw != NULL ) xrtValueRelease(pTileCustomRaw);
	memset(&pApp->tTileset, 0, sizeof(pApp->tTileset));
	for ( i = 0; i < (int)sizeof(pApp->tTileset.arrPassage); i++ ) pApp->tTileset.arrPassage[i] = 255;
}

static int mapedit_surface_load(mapedit_app_t* pApp, const char* sPath, xui_surface* ppSurface, xui_surface_desc_t* pDesc)
{
	int ret;
	if ( pApp == NULL || sPath == NULL || ppSurface == NULL || pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppSurface = NULL;
	memset(pDesc, 0, sizeof(*pDesc));
	ret = pApp->tProxy.surfaceLoadFile(&pApp->tProxy, ppSurface, sPath, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( ret != XUI_OK ) return ret;
	(void)pApp->tProxy.surfaceGetDesc(&pApp->tProxy, *ppSurface, pDesc);
	return XUI_OK;
}

static int mapedit_tileset_special_surface_load_dir(mapedit_app_t* pApp, const char* sDir, const char* sFile, xui_surface* ppSurface, xui_surface_desc_t* pDesc)
{
	char sRel[MAPEDIT_PATH_MAX];
	char sPath[MAPEDIT_PATH_MAX];
	if ( pApp == NULL || sDir == NULL || sFile == NULL || sFile[0] == 0 || ppSurface == NULL || pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	snprintf(sRel, sizeof(sRel), "assets\\%s\\%s", sDir, sFile);
	mapedit_app_path(pApp, sPath, sizeof(sPath), sRel);
	if ( !mapedit_file_exists_utf8(sPath) ) return XUI_ERROR_FILE_NOT_FOUND;
	return mapedit_surface_load(pApp, sPath, ppSurface, pDesc);
}

static int mapedit_tileset_load_special_surface(mapedit_app_t* pApp, const char* sType, const char* sFile, xui_surface* ppSurface, xui_surface_desc_t* pDesc)
{
	const mapedit_material_category_t* pCategory;
	const char* sTriedDir;
	int i;
	if ( pApp == NULL || sFile == NULL || sFile[0] == 0 || ppSurface == NULL || pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pCategory = mapedit_tileset_special_category_for_type(sType);
	sTriedDir = pCategory != NULL ? pCategory->sDir : NULL;
	if ( sTriedDir != NULL && mapedit_tileset_special_surface_load_dir(pApp, sTriedDir, sFile, ppSurface, pDesc) == XUI_OK ) return XUI_OK;
	for ( i = 0; i < mapedit_tileset_special_type_count(); i++ ) {
		pCategory = mapedit_tileset_special_category_for_type(g_arrTilesetSpecialTypeNames[i]);
		if ( pCategory == NULL || pCategory->sDir == NULL || (sTriedDir != NULL && strcmp(pCategory->sDir, sTriedDir) == 0) ) continue;
		if ( mapedit_tileset_special_surface_load_dir(pApp, pCategory->sDir, sFile, ppSurface, pDesc) == XUI_OK ) return XUI_OK;
	}
	return XUI_ERROR_FILE_NOT_FOUND;
}

static void mapedit_material_preview_clear(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialPreviewSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialPreviewSurface);
		pApp->pMaterialPreviewSurface = NULL;
	}
	memset(&pApp->tMaterialPreviewDesc, 0, sizeof(pApp->tMaterialPreviewDesc));
}

static void mapedit_material_tooltip_clear(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialTooltipSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialTooltipSurface);
		pApp->pMaterialTooltipSurface = NULL;
	}
	memset(&pApp->tMaterialTooltipDesc, 0, sizeof(pApp->tMaterialTooltipDesc));
	pApp->iMaterialTooltipIndex = -1;
}

static void mapedit_material_view_clear(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialViewSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialViewSurface);
		pApp->pMaterialViewSurface = NULL;
	}
	memset(&pApp->tMaterialViewDesc, 0, sizeof(pApp->tMaterialViewDesc));
}

static int mapedit_material_build_path(mapedit_app_t* pApp, int iIndex, char* sPath, int iCap)
{
	const mapedit_material_category_t* pCategory;
	mapedit_file_list_t* pList;
	char rel[MAPEDIT_PATH_MAX];
	if ( pApp == NULL || sPath == NULL || iCap <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pList = &pApp->tMaterialCategoryFiles;
	if ( iIndex < 0 || iIndex >= pList->iCount ) return XUI_ERROR_INVALID_ARGUMENT;
	pCategory = &g_arrMaterialCategories[mapedit_material_category_clamp(pApp->iMaterialCategory)];
	snprintf(rel, sizeof(rel), "assets\\%s\\%s", pCategory->sDir, pList->arrNames[iIndex]);
	mapedit_app_path(pApp, sPath, iCap, rel);
	return XUI_OK;
}

static void mapedit_material_preview_load(mapedit_app_t* pApp)
{
	mapedit_file_list_t* pList;
	char path[MAPEDIT_PATH_MAX];
	if ( pApp == NULL ) return;
	mapedit_material_preview_clear(pApp);
	pList = &pApp->tMaterialCategoryFiles;
	if ( pList->iSelected < 0 || pList->iSelected >= pList->iCount ) return;
	if ( mapedit_material_build_path(pApp, pList->iSelected, path, sizeof(path)) != XUI_OK ) return;
	(void)mapedit_surface_load(pApp, path, &pApp->pMaterialPreviewSurface, &pApp->tMaterialPreviewDesc);
	if ( pApp->pMaterialPreviewCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMaterialPreviewCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_material_tooltip_ensure(mapedit_app_t* pApp, int iIndex)
{
	char path[MAPEDIT_PATH_MAX];
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->tMaterialCategoryFiles.iCount ) return 0;
	if ( pApp->iMaterialTooltipIndex == iIndex && pApp->pMaterialTooltipSurface != NULL &&
	     pApp->tMaterialTooltipDesc.iWidth > 0 && pApp->tMaterialTooltipDesc.iHeight > 0 ) return 1;
	mapedit_material_tooltip_clear(pApp);
	if ( mapedit_material_build_path(pApp, iIndex, path, sizeof(path)) != XUI_OK ) return 0;
	if ( mapedit_surface_load(pApp, path, &pApp->pMaterialTooltipSurface, &pApp->tMaterialTooltipDesc) != XUI_OK ) {
		mapedit_material_tooltip_clear(pApp);
		return 0;
	}
	pApp->iMaterialTooltipIndex = iIndex;
	return 1;
}

static int mapedit_material_view_load(mapedit_app_t* pApp, int iIndex)
{
	mapedit_file_list_t* pList;
	char path[MAPEDIT_PATH_MAX];
	char name[MAPEDIT_NAME_MAX];
	const char* sDisplay;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pList = &pApp->tMaterialCategoryFiles;
	if ( iIndex < 0 || iIndex >= pList->iCount ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_material_build_path(pApp, iIndex, path, sizeof(path)) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	mapedit_material_view_clear(pApp);
	if ( mapedit_surface_load(pApp, path, &pApp->pMaterialViewSurface, &pApp->tMaterialViewDesc) != XUI_OK ) return XUI_ERROR;
	sDisplay = pList->arrText[iIndex][0] ? pList->arrText[iIndex] : pList->arrNames[iIndex];
	mapedit_material_display_name(pApp, iIndex, name, sizeof(name));
	snprintf(pApp->sMaterialViewTitle, sizeof(pApp->sMaterialViewTitle), "查看图块 - %s(%s)", name[0] ? name : sDisplay, pList->arrNames[iIndex]);
	if ( pApp->pMaterialViewWindow != NULL ) (void)xuiWindowSetTitle(pApp->pMaterialViewWindow, pApp->sMaterialViewTitle);
	if ( pApp->pMaterialViewScroll != NULL ) (void)xuiScrollViewSetContentSize(pApp->pMaterialViewScroll, (float)pApp->tMaterialViewDesc.iWidth, (float)pApp->tMaterialViewDesc.iHeight);
	if ( pApp->pMaterialViewCanvas != NULL ) {
		(void)xuiWidgetSetRect(pApp->pMaterialViewCanvas, (xui_rect_t){0, 0, pApp->tMaterialViewDesc.iWidth, pApp->tMaterialViewDesc.iHeight});
		(void)xuiWidgetInvalidate(pApp->pMaterialViewCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static void mapedit_material_mapping_path(mapedit_app_t* pApp, char* sPath, int iCap)
{
	const mapedit_material_category_t* pCategory;
	char rel[MAPEDIT_PATH_MAX];
	if ( sPath == NULL || iCap <= 0 ) return;
	sPath[0] = 0;
	if ( pApp == NULL ) return;
	pCategory = &g_arrMaterialCategories[mapedit_material_category_clamp(pApp->iMaterialCategory)];
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sMapFile);
	mapedit_app_path(pApp, sPath, iCap, rel);
}

static void mapedit_material_display_name(mapedit_app_t* pApp, int iIndex, char* sOut, int iCap)
{
	mapedit_file_list_t* pList;
	const char* sText;
	const char* sSep;
	int len;
	if ( sOut == NULL || iCap <= 0 ) return;
	sOut[0] = 0;
	if ( pApp == NULL ) return;
	pList = &pApp->tMaterialCategoryFiles;
	if ( iIndex < 0 || iIndex >= pList->iCount ) return;
	sText = pList->arrText[iIndex];
	sSep = strstr(sText, "    ");
	if ( sSep == NULL ) {
		mapedit_copy_text(sOut, iCap, sText);
		return;
	}
	len = (int)(sSep - sText);
	if ( len >= iCap ) len = iCap - 1;
	if ( len > 0 ) memcpy(sOut, sText, (size_t)len);
	sOut[len] = 0;
}

static int mapedit_material_save_mapping(mapedit_app_t* pApp, int iIndex, const char* sName)
{
	mapedit_file_list_t* pList;
	char mapPath[MAPEDIT_PATH_MAX];
	char sText[MAPEDIT_LIST_TEXT_MAX];
	xvalue* pMap;
	int ok;
	if ( pApp == NULL || sName == NULL || sName[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pList = &pApp->tMaterialCategoryFiles;
	if ( iIndex < 0 || iIndex >= pList->iCount ) return XUI_ERROR_INVALID_ARGUMENT;
	mapedit_material_mapping_path(pApp, mapPath, sizeof(mapPath));
	pMap = mapedit_material_load_map(mapPath);
	if ( pMap == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	ok = xuiXrtValueObjectSetText(pMap, pList->arrNames[iIndex], (uint32)strlen(pList->arrNames[iIndex]), sName, 0, TRUE) &&
	     xrtXsonStringifyFile(mapPath, pMap, true);
	xrtValueRelease(pMap);
	if ( !ok ) return XUI_ERROR;
	snprintf(sText, sizeof(sText), "%s    %s", sName, pList->arrNames[iIndex]);
	mapedit_copy_text(pList->arrText[iIndex], MAPEDIT_LIST_TEXT_MAX, sText);
	pList->arrTextPtrs[iIndex] = pList->arrText[iIndex];
	if ( pApp->pMaterialListView != NULL ) {
		(void)xuiListViewSetItems(pApp->pMaterialListView, pList->arrTextPtrs, pList->iCount);
		(void)xuiListViewSetSelected(pApp->pMaterialListView, iIndex);
	}
	pList->iSelected = iIndex;
	return XUI_OK;
}

static int mapedit_has_image_ext(const char* sName)
{
	return mapedit_has_ext(sName, ".png") ||
	       mapedit_has_ext(sName, ".jpg") ||
	       mapedit_has_ext(sName, ".jpeg") ||
	       mapedit_has_ext(sName, ".bmp") ||
	       mapedit_has_ext(sName, ".tga") ||
	       mapedit_has_ext(sName, ".webp");
}

static int mapedit_delete_file_utf8(const char* sPath)
{
	wchar_t wPath[MAPEDIT_PATH_MAX];
	if ( sPath == NULL || sPath[0] == 0 ) return 0;
	if ( !mapedit_file_exists_utf8(sPath) ) return 1;
	if ( !mapedit_utf8_to_wide(sPath, wPath, (int)(sizeof(wPath) / sizeof(wPath[0]))) ) return 0;
	return DeleteFileW(wPath) != 0;
}

static int mapedit_copy_file_utf8(const char* sSrc, const char* sDst, int bFailIfExists)
{
	wchar_t wSrc[MAPEDIT_PATH_MAX];
	wchar_t wDst[MAPEDIT_PATH_MAX];
	if ( !mapedit_utf8_to_wide(sSrc, wSrc, (int)(sizeof(wSrc) / sizeof(wSrc[0]))) ) return 0;
	if ( !mapedit_utf8_to_wide(sDst, wDst, (int)(sizeof(wDst) / sizeof(wDst[0]))) ) return 0;
	return CopyFileW(wSrc, wDst, bFailIfExists ? TRUE : FALSE) != 0;
}

static void mapedit_material_category_dir(mapedit_app_t* pApp, char* sPath, int iCap)
{
	const mapedit_material_category_t* pCategory;
	char rel[MAPEDIT_PATH_MAX];
	if ( sPath == NULL || iCap <= 0 ) return;
	sPath[0] = 0;
	if ( pApp == NULL ) return;
	pCategory = &g_arrMaterialCategories[mapedit_material_category_clamp(pApp->iMaterialCategory)];
	snprintf(rel, sizeof(rel), "assets\\%s", pCategory->sDir);
	mapedit_app_path(pApp, sPath, iCap, rel);
}

static void mapedit_material_edit_clear_source(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialEditSourceSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialEditSourceSurface);
		pApp->pMaterialEditSourceSurface = NULL;
	}
	if ( pApp->tMaterialEditSourceImage.pPixels != NULL ) xgeImageFree(&pApp->tMaterialEditSourceImage);
	memset(&pApp->tMaterialEditSourceImage, 0, sizeof(pApp->tMaterialEditSourceImage));
	memset(&pApp->tMaterialEditSourceDesc, 0, sizeof(pApp->tMaterialEditSourceDesc));
	pApp->iMaterialEditSourceCols = 0;
	pApp->iMaterialEditSourceRows = 0;
	pApp->iMaterialEditSelCol = 0;
	pApp->iMaterialEditSelRow = 0;
	pApp->iMaterialEditSelCols = 1;
	pApp->iMaterialEditSelRows = 1;
	pApp->bMaterialEditSelecting = 0;
}

static void mapedit_material_edit_clear_output(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialEditOutputSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialEditOutputSurface);
		pApp->pMaterialEditOutputSurface = NULL;
	}
	free(pApp->pMaterialEditOutputPixels);
	pApp->pMaterialEditOutputPixels = NULL;
	memset(&pApp->tMaterialEditOutputDesc, 0, sizeof(pApp->tMaterialEditOutputDesc));
	pApp->iMaterialEditOutputCols = 0;
	pApp->iMaterialEditOutputRows = 0;
	pApp->iMaterialEditUsedCols = 0;
	pApp->iMaterialEditUsedRows = 0;
}

static int mapedit_material_edit_output_dimensions(int iCols, int iRows, int* pWidth, int* pHeight, size_t* pBytes)
{
	size_t width;
	size_t height;
	size_t pixels;
	size_t bytes;
	if ( iCols <= 0 || iRows <= 0 || MAPEDIT_TILE_W <= 0 || MAPEDIT_TILE_H <= 0 ) return 0;
	if ( (size_t)iCols > (size_t)INT_MAX / (size_t)MAPEDIT_TILE_W ) return 0;
	if ( (size_t)iRows > (size_t)INT_MAX / (size_t)MAPEDIT_TILE_H ) return 0;
	width = (size_t)iCols * (size_t)MAPEDIT_TILE_W;
	height = (size_t)iRows * (size_t)MAPEDIT_TILE_H;
	if ( width > (size_t)INT_MAX / 4u || height > (size_t)INT_MAX ) return 0;
	if ( width > (size_t)-1 / height ) return 0;
	pixels = width * height;
	if ( pixels > (size_t)-1 / 4u ) return 0;
	bytes = pixels * 4u;
	if ( bytes > MAPEDIT_MATERIAL_OUTPUT_BYTE_MAX ) return 0;
	if ( pWidth != NULL ) *pWidth = (int)width;
	if ( pHeight != NULL ) *pHeight = (int)height;
	if ( pBytes != NULL ) *pBytes = bytes;
	return 1;
}

static int mapedit_material_edit_refresh_output_surface(mapedit_app_t* pApp)
{
	int w;
	int h;
	if ( pApp == NULL || pApp->pMaterialEditOutputPixels == NULL || pApp->iMaterialEditOutputCols <= 0 || pApp->iMaterialEditOutputRows <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pMaterialEditOutputSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pMaterialEditOutputSurface);
		pApp->pMaterialEditOutputSurface = NULL;
	}
	if ( !mapedit_material_edit_output_dimensions(pApp->iMaterialEditOutputCols, pApp->iMaterialEditOutputRows, &w, &h, NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->tProxy.surfaceCreateRGBA(&pApp->tProxy, &pApp->pMaterialEditOutputSurface, w, h, pApp->pMaterialEditOutputPixels, w * 4, XUI_SURFACE_ALPHA_STRAIGHT) != XUI_OK ) return XUI_ERROR;
	(void)pApp->tProxy.surfaceGetDesc(&pApp->tProxy, pApp->pMaterialEditOutputSurface, &pApp->tMaterialEditOutputDesc);
	if ( pApp->pMaterialEditOutputScroll != NULL ) (void)xuiScrollViewSetContentSize(pApp->pMaterialEditOutputScroll, (float)w, (float)h);
	if ( pApp->pMaterialEditOutputCanvas != NULL ) {
		(void)xuiWidgetSetRect(pApp->pMaterialEditOutputCanvas, (xui_rect_t){0, 0, w, h});
		(void)xuiWidgetInvalidate(pApp->pMaterialEditOutputCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int mapedit_material_edit_set_output_size(mapedit_app_t* pApp, int iCols, int iRows)
{
	unsigned char* pNew;
	int oldW;
	int oldH;
	int newW;
	int newH;
	int copyW;
	int copyH;
	int y;
	size_t newBytes;
	if ( pApp == NULL || iCols <= 0 || iRows <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCols == pApp->iMaterialEditOutputCols && iRows == pApp->iMaterialEditOutputRows && pApp->pMaterialEditOutputPixels != NULL ) return XUI_OK;
	if ( !mapedit_material_edit_output_dimensions(iCols, iRows, &newW, &newH, &newBytes) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !mapedit_material_edit_output_dimensions(pApp->iMaterialEditOutputCols, pApp->iMaterialEditOutputRows, &oldW, &oldH, NULL) ) {
		oldW = 0;
		oldH = 0;
	}
	pNew = (unsigned char*)calloc(newBytes, 1u);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pApp->pMaterialEditOutputPixels != NULL && oldW > 0 && oldH > 0 ) {
		copyW = mapedit_min_i(oldW, newW);
		copyH = mapedit_min_i(oldH, newH);
		for ( y = 0; y < copyH; y++ ) {
			memcpy(pNew + (size_t)y * (size_t)newW * 4u, pApp->pMaterialEditOutputPixels + (size_t)y * (size_t)oldW * 4u, (size_t)copyW * 4u);
		}
	}
	free(pApp->pMaterialEditOutputPixels);
	pApp->pMaterialEditOutputPixels = pNew;
	pApp->iMaterialEditOutputCols = iCols;
	pApp->iMaterialEditOutputRows = iRows;
	if ( pApp->iMaterialEditUsedCols > iCols ) pApp->iMaterialEditUsedCols = iCols;
	if ( pApp->iMaterialEditUsedRows > iRows ) pApp->iMaterialEditUsedRows = iRows;
	return mapedit_material_edit_refresh_output_surface(pApp);
}

static int mapedit_material_edit_ensure_output(mapedit_app_t* pApp, int iCols, int iRows)
{
	int cols;
	int rows;
	if ( pApp == NULL || iCols <= 0 || iRows <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	cols = pApp->iMaterialEditOutputCols;
	rows = pApp->iMaterialEditOutputRows;
	if ( cols <= 0 ) cols = (pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC) ? MAPEDIT_MATERIAL_STATIC_COLS : 4;
	if ( rows <= 0 ) rows = (pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC) ? MAPEDIT_MATERIAL_INITIAL_ROWS : 4;
	if ( pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ) {
		cols = MAPEDIT_MATERIAL_STATIC_COLS;
		if ( iRows > rows ) {
			if ( iRows > INT_MAX - (MAPEDIT_MATERIAL_EXTEND_ROWS - 1) ) return XUI_ERROR_INVALID_ARGUMENT;
			rows = ((iRows + MAPEDIT_MATERIAL_EXTEND_ROWS - 1) / MAPEDIT_MATERIAL_EXTEND_ROWS) * MAPEDIT_MATERIAL_EXTEND_ROWS;
		}
	} else {
		while ( iCols > cols ) {
			if ( cols > INT_MAX / 2 ) return XUI_ERROR_INVALID_ARGUMENT;
			cols *= 2;
		}
		while ( iRows > rows ) {
			if ( rows > INT_MAX / 2 ) return XUI_ERROR_INVALID_ARGUMENT;
			rows *= 2;
		}
	}
	if ( cols != pApp->iMaterialEditOutputCols || rows != pApp->iMaterialEditOutputRows || pApp->pMaterialEditOutputPixels == NULL ) {
		return mapedit_material_edit_set_output_size(pApp, cols, rows);
	}
	return XUI_OK;
}

static int mapedit_material_edit_output_at_bottom(mapedit_app_t* pApp)
{
	xui_rect_t viewport;
	float offsetY;
	float contentH;
	if ( pApp == NULL || pApp->pMaterialEditOutputScroll == NULL ) return 0;
	if ( xuiScrollViewGetOffset(pApp->pMaterialEditOutputScroll, NULL, &offsetY) != XUI_OK ) return 0;
	if ( xuiScrollViewGetContentSize(pApp->pMaterialEditOutputScroll, NULL, &contentH) != XUI_OK ) return 0;
	viewport = xuiScrollViewGetViewportRect(pApp->pMaterialEditOutputScroll);
	if ( contentH <= (float)viewport.fH ) return 0;
	return (offsetY + (float)viewport.fH + 1.0f >= contentH);
}

static int mapedit_material_edit_extend_output_rows(mapedit_app_t* pApp)
{
	int rows;
	if ( pApp == NULL || pApp->iMaterialEditMode != MAPEDIT_MATERIAL_MODE_STATIC ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->iMaterialEditOutputRows > INT_MAX - MAPEDIT_MATERIAL_EXTEND_ROWS ) return XUI_ERROR_INVALID_ARGUMENT;
	rows = pApp->iMaterialEditOutputRows + MAPEDIT_MATERIAL_EXTEND_ROWS;
	if ( rows < MAPEDIT_MATERIAL_INITIAL_ROWS + MAPEDIT_MATERIAL_EXTEND_ROWS ) rows = MAPEDIT_MATERIAL_INITIAL_ROWS + MAPEDIT_MATERIAL_EXTEND_ROWS;
	return mapedit_material_edit_ensure_output(pApp, MAPEDIT_MATERIAL_STATIC_COLS, rows);
}

static int mapedit_material_edit_extend_output_if_bottom(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iMaterialEditMode != MAPEDIT_MATERIAL_MODE_STATIC ) return 0;
	if ( !mapedit_material_edit_output_at_bottom(pApp) ) return 0;
	return mapedit_material_edit_extend_output_rows(pApp) == XUI_OK;
}

static void mapedit_material_edit_output_scroll_change(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	(void)pWidget;
	(void)fOffsetX;
	(void)fOffsetY;
	(void)mapedit_material_edit_extend_output_if_bottom((mapedit_app_t*)pUser);
}

static void mapedit_trim_copy(char* sOut, int iCap, const char* sText)
{
	const char* a;
	const char* b;
	int n;
	if ( sOut == NULL || iCap <= 0 ) return;
	sOut[0] = 0;
	if ( sText == NULL ) return;
	a = sText;
	while ( *a == ' ' || *a == '\t' || *a == '\r' || *a == '\n' ) a++;
	b = a + strlen(a);
	while ( b > a && (b[-1] == ' ' || b[-1] == '\t' || b[-1] == '\r' || b[-1] == '\n') ) b--;
	n = (int)(b - a);
	if ( n >= iCap ) n = iCap - 1;
	if ( n > 0 ) memcpy(sOut, a, (size_t)n);
	sOut[n] = 0;
}

static int mapedit_material_edit_file_name(const char* sInput, char* sOut, int iCap)
{
	char text[MAPEDIT_FILE_MAX];
	if ( sOut == NULL || iCap <= 0 ) return 0;
	sOut[0] = 0;
	mapedit_trim_copy(text, sizeof(text), sInput);
	if ( text[0] == 0 ) return 0;
	if ( strchr(text, '\\') != NULL || strchr(text, '/') != NULL || strchr(text, ':') != NULL ) return 0;
	if ( mapedit_has_image_ext(text) ) snprintf(sOut, (size_t)iCap, "%s", text);
	else snprintf(sOut, (size_t)iCap, "%s.png", text);
	return sOut[0] != 0;
}

static void mapedit_material_edit_close(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialEditMsgTip != NULL ) (void)xuiMsgTipClose(pApp->pMaterialEditMsgTip);
	if ( pApp->pMaterialEditWindow != NULL ) (void)xuiWindowSetOpen(pApp->pMaterialEditWindow, 0);
}

static void mapedit_material_edit_show_error(mapedit_app_t* pApp, xui_widget pFocusWidget, const char* sText)
{
	if ( pApp == NULL || sText == NULL ) return;
	mapedit_status(pApp, sText);
	if ( pFocusWidget != NULL ) {
		(void)xuiSetFocusWidget(pApp->pContext, pFocusWidget);
		(void)xuiInputSelectAll(pFocusWidget);
	}
	if ( pApp->pMaterialEditMsgTip != NULL ) {
		(void)xuiMsgTipShow(pApp->pMaterialEditMsgTip, XUI_MSGTIP_ICON_ERROR, sText, 2.2f);
	}
}

static int mapedit_material_backup_file(mapedit_app_t* pApp, const char* sFile)
{
	char dir[MAPEDIT_PATH_MAX];
	char oldPath[MAPEDIT_PATH_MAX];
	char backupDir[MAPEDIT_PATH_MAX];
	char backupPath[MAPEDIT_PATH_MAX];
	char base[MAPEDIT_FILE_MAX];
	char ext[32];
	char candidate[MAPEDIT_FILE_MAX];
	const char* dot;
	int maxBase;
	int n;
	if ( pApp == NULL || sFile == NULL || sFile[0] == 0 ) return 1;
	mapedit_material_category_dir(pApp, dir, sizeof(dir));
	mapedit_path_join(oldPath, sizeof(oldPath), dir, sFile);
	if ( !mapedit_file_exists_utf8(oldPath) ) return 1;
	mapedit_path_join(backupDir, sizeof(backupDir), dir, "backup");
	if ( !mapedit_ensure_dir_utf8(backupDir) ) return 0;
	dot = strrchr(sFile, '.');
	if ( dot != NULL && dot > sFile ) {
		int len = (int)(dot - sFile);
		if ( len >= (int)sizeof(base) ) len = (int)sizeof(base) - 1;
		memcpy(base, sFile, (size_t)len);
		base[len] = 0;
		snprintf(ext, sizeof(ext), "%s", dot);
	} else {
		snprintf(base, sizeof(base), "%s", sFile[0] ? sFile : "backup");
		snprintf(ext, sizeof(ext), ".png");
	}
	maxBase = (int)sizeof(candidate) - (int)strlen(ext) - 16;
	if ( maxBase < 1 ) maxBase = 1;
	for ( n = 0; n < 10000; n++ ) {
		if ( n == 0 ) snprintf(candidate, sizeof(candidate), "%.*s%s", maxBase, base, ext);
		else snprintf(candidate, sizeof(candidate), "%.*s_%d%s", maxBase, base, n, ext);
		mapedit_path_join(backupPath, sizeof(backupPath), backupDir, candidate);
		if ( !mapedit_file_exists_utf8(backupPath) ) return mapedit_copy_file_utf8(oldPath, backupPath, 1);
	}
	return 0;
}

static int mapedit_material_edit_load_existing_output(mapedit_app_t* pApp, int iIndex)
{
	xge_image_t img;
	char path[MAPEDIT_PATH_MAX];
	int cols;
	int rows;
	int outW;
	int copyW;
	int copyH;
	int y;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_material_build_path(pApp, iIndex, path, sizeof(path)) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&img, 0, sizeof(img));
	if ( xgeImageLoadEx(&img, path, XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) return XUI_ERROR_FILE_NOT_FOUND;
	if ( img.iWidth <= 0 || img.iHeight <= 0 ) {
		xgeImageFree(&img);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	cols = 1 + (img.iWidth - 1) / MAPEDIT_TILE_W;
	rows = 1 + (img.iHeight - 1) / MAPEDIT_TILE_H;
	if ( pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ) {
		cols = MAPEDIT_MATERIAL_STATIC_COLS;
		if ( rows < MAPEDIT_MATERIAL_INITIAL_ROWS ) rows = MAPEDIT_MATERIAL_INITIAL_ROWS;
	} else {
		if ( cols < 4 ) cols = 4;
		if ( rows < 4 ) rows = 4;
	}
	if ( mapedit_material_edit_set_output_size(pApp, cols, rows) != XUI_OK ) {
		xgeImageFree(&img);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	outW = pApp->iMaterialEditOutputCols * MAPEDIT_TILE_W;
	copyW = mapedit_min_i(img.iWidth, outW);
	copyH = mapedit_min_i(img.iHeight, pApp->iMaterialEditOutputRows * MAPEDIT_TILE_H);
	for ( y = 0; y < copyH; y++ ) {
		memcpy(pApp->pMaterialEditOutputPixels + (size_t)y * (size_t)outW * 4u, (const unsigned char*)img.pPixels + (size_t)y * (size_t)img.iStride, (size_t)copyW * 4u);
	}
	pApp->iMaterialEditUsedCols = pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ? MAPEDIT_MATERIAL_STATIC_COLS : (copyW + MAPEDIT_TILE_W - 1) / MAPEDIT_TILE_W;
	pApp->iMaterialEditUsedRows = (copyH + MAPEDIT_TILE_H - 1) / MAPEDIT_TILE_H;
	xgeImageFree(&img);
	return mapedit_material_edit_refresh_output_surface(pApp);
}

static int mapedit_material_edit_load_source(mapedit_app_t* pApp, const char* sPath)
{
	xge_image_t img;
	int w;
	int h;
	if ( pApp == NULL || sPath == NULL || sPath[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&img, 0, sizeof(img));
	if ( xgeImageLoadEx(&img, sPath, XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) {
		mapedit_status(pApp, "外部图片加载失败");
		return XUI_ERROR;
	}
	if ( img.iWidth <= 0 || img.iHeight <= 0 ) {
		xgeImageFree(&img);
		mapedit_status(pApp, "外部图片尺寸无效");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	mapedit_material_edit_clear_source(pApp);
	pApp->tMaterialEditSourceImage = img;
	w = img.iWidth;
	h = img.iHeight;
	if ( pApp->tProxy.surfaceCreateRGBA(&pApp->tProxy, &pApp->pMaterialEditSourceSurface, w, h, img.pPixels, img.iStride, XUI_SURFACE_ALPHA_STRAIGHT) != XUI_OK ) {
		mapedit_material_edit_clear_source(pApp);
		mapedit_status(pApp, "外部图片创建纹理失败");
		return XUI_ERROR;
	}
	(void)pApp->tProxy.surfaceGetDesc(&pApp->tProxy, pApp->pMaterialEditSourceSurface, &pApp->tMaterialEditSourceDesc);
	pApp->iMaterialEditSourceCols = 1 + (w - 1) / MAPEDIT_TILE_W;
	pApp->iMaterialEditSourceRows = 1 + (h - 1) / MAPEDIT_TILE_H;
	pApp->iMaterialEditSelCol = 0;
	pApp->iMaterialEditSelRow = 0;
	pApp->iMaterialEditSelCols = pApp->iMaterialEditSourceCols > 0 ? 1 : 0;
	pApp->iMaterialEditSelRows = pApp->iMaterialEditSourceRows > 0 ? 1 : 0;
	if ( pApp->pMaterialEditSourceScroll != NULL ) (void)xuiScrollViewSetContentSize(pApp->pMaterialEditSourceScroll, (float)w, (float)h);
	if ( pApp->pMaterialEditSourceCanvas != NULL ) {
		(void)xuiWidgetSetRect(pApp->pMaterialEditSourceCanvas, (xui_rect_t){0, 0, w, h});
		(void)xuiWidgetInvalidate(pApp->pMaterialEditSourceCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	mapedit_status(pApp, "外部图片已加载");
	return XUI_OK;
}

static int mapedit_material_edit_open_source_dialog(mapedit_app_t* pApp)
{
	return mapedit_show_material_source_dialog(pApp);
}

static int mapedit_material_edit_cell_from_event(xui_widget pWidget, const xui_event_t* pEvent, int* pCol, int* pRow)
{
	xui_rect_t wr;
	int col;
	int row;
	if ( pWidget == NULL || pEvent == NULL ) return 0;
	wr = xuiWidgetGetWorldRect(pWidget);
	col = (pEvent->fX - wr.fX) / MAPEDIT_TILE_W;
	row = (pEvent->fY - wr.fY) / MAPEDIT_TILE_H;
	if ( col < 0 || row < 0 ) return 0;
	if ( pCol != NULL ) *pCol = col;
	if ( pRow != NULL ) *pRow = row;
	return 1;
}

static void mapedit_material_edit_set_selection(mapedit_app_t* pApp, int iCol0, int iRow0, int iCol1, int iRow1)
{
	int x0;
	int y0;
	int x1;
	int y1;
	if ( pApp == NULL || pApp->iMaterialEditSourceCols <= 0 || pApp->iMaterialEditSourceRows <= 0 ) return;
	x0 = mapedit_min_i(iCol0, iCol1);
	y0 = mapedit_min_i(iRow0, iRow1);
	x1 = mapedit_max_i(iCol0, iCol1);
	y1 = mapedit_max_i(iRow0, iRow1);
	x0 = mapedit_max_i(0, mapedit_min_i(x0, pApp->iMaterialEditSourceCols - 1));
	y0 = mapedit_max_i(0, mapedit_min_i(y0, pApp->iMaterialEditSourceRows - 1));
	x1 = mapedit_max_i(0, mapedit_min_i(x1, pApp->iMaterialEditSourceCols - 1));
	y1 = mapedit_max_i(0, mapedit_min_i(y1, pApp->iMaterialEditSourceRows - 1));
	pApp->iMaterialEditSelCol = x0;
	pApp->iMaterialEditSelRow = y0;
	pApp->iMaterialEditSelCols = x1 - x0 + 1;
	pApp->iMaterialEditSelRows = y1 - y0 + 1;
	if ( pApp->pMaterialEditSourceCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMaterialEditSourceCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_status(pApp, "已选择源图块");
}

static int mapedit_material_edit_paste(mapedit_app_t* pApp, int iDstCol, int iDstRow)
{
	const unsigned char* src;
	unsigned char* dst;
	int outW;
	int srcX0;
	int srcY0;
	int copyCols;
	int copyRows;
	int totalCols;
	int totalRows;
	int c;
	int r;
	int px;
	int py;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->tMaterialEditSourceImage.pPixels == NULL ) {
		mapedit_status(pApp, "请先选择源图块");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->pMaterialEditOutputPixels == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iDstCol < 0 || iDstRow < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	copyCols = pApp->iMaterialEditSelCols > 0 ? pApp->iMaterialEditSelCols : 1;
	copyRows = pApp->iMaterialEditSelRows > 0 ? pApp->iMaterialEditSelRows : 1;
	if ( copyCols > INT_MAX - iDstCol || copyRows > INT_MAX - iDstRow ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC && iDstCol + copyCols > MAPEDIT_MATERIAL_STATIC_COLS ) copyCols = MAPEDIT_MATERIAL_STATIC_COLS - iDstCol;
	if ( copyCols <= 0 || copyRows <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	totalCols = iDstCol + copyCols;
	totalRows = iDstRow + copyRows;
	if ( mapedit_material_edit_ensure_output(pApp, totalCols, totalRows) != XUI_OK ) {
		mapedit_status(pApp, "扩展输出图块失败");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	outW = pApp->iMaterialEditOutputCols * MAPEDIT_TILE_W;
	for ( r = 0; r < copyRows; r++ ) {
		for ( c = 0; c < copyCols; c++ ) {
			srcX0 = (pApp->iMaterialEditSelCol + c) * MAPEDIT_TILE_W;
			srcY0 = (pApp->iMaterialEditSelRow + r) * MAPEDIT_TILE_H;
			for ( py = 0; py < MAPEDIT_TILE_H; py++ ) {
				int srcY = srcY0 + py;
				int dstY = (iDstRow + r) * MAPEDIT_TILE_H + py;
				if ( dstY < 0 || dstY >= pApp->iMaterialEditOutputRows * MAPEDIT_TILE_H ) continue;
				for ( px = 0; px < MAPEDIT_TILE_W; px++ ) {
					int srcX = srcX0 + px;
					int dstX = (iDstCol + c) * MAPEDIT_TILE_W + px;
					if ( dstX < 0 || dstX >= outW ) continue;
					dst = pApp->pMaterialEditOutputPixels + ((size_t)dstY * (size_t)outW + (size_t)dstX) * 4u;
					dst[0] = dst[1] = dst[2] = dst[3] = 0;
					if ( srcX >= 0 && srcY >= 0 && srcX < pApp->tMaterialEditSourceImage.iWidth && srcY < pApp->tMaterialEditSourceImage.iHeight ) {
						src = (const unsigned char*)pApp->tMaterialEditSourceImage.pPixels + (size_t)srcY * (size_t)pApp->tMaterialEditSourceImage.iStride + (size_t)srcX * 4u;
						memcpy(dst, src, 4);
					}
				}
			}
		}
	}
	pApp->iMaterialEditUsedCols = pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ? MAPEDIT_MATERIAL_STATIC_COLS : mapedit_max_i(pApp->iMaterialEditUsedCols, totalCols);
	pApp->iMaterialEditUsedRows = mapedit_max_i(pApp->iMaterialEditUsedRows, totalRows);
	(void)mapedit_material_edit_refresh_output_surface(pApp);
	mapedit_status(pApp, "已粘贴图块");
	return XUI_OK;
}

static int mapedit_material_edit_update_mapping(mapedit_app_t* pApp, const char* sOldFile, const char* sNewFile, const char* sName)
{
	char mapPath[MAPEDIT_PATH_MAX];
	xvalue* pMap;
	int ok;
	if ( pApp == NULL || sNewFile == NULL || sName == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	mapedit_material_mapping_path(pApp, mapPath, sizeof(mapPath));
	pMap = mapedit_material_load_map(mapPath);
	if ( pMap == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( sOldFile != NULL && sOldFile[0] != 0 && strcmp(sOldFile, sNewFile) != 0 ) {
		(void)xuiXrtValueObjectRemove(pMap, sOldFile, (uint32)strlen(sOldFile));
	}
	ok = xuiXrtValueObjectSetText(pMap, sNewFile, (uint32)strlen(sNewFile), sName, 0, TRUE) &&
	     xrtXsonStringifyFile(mapPath, pMap, true);
	xrtValueRelease(pMap);
	return ok ? XUI_OK : XUI_ERROR;
}

/*
 * Copy-on-write reference update: the new image is written before this runs,
 * every XSON file is replaced atomically, and the old image is retained unless
 * all references succeed. A mid-operation failure therefore cannot leave a
 * tileset pointing at a missing file.
 */
static int mapedit_material_update_tileset_references(mapedit_app_t* pApp,
	const char* sOldFile, const char* sNewFile)
{
	const mapedit_material_category_t* pCategory;
	int i;
	int allOk = 1;
	if ( pApp == NULL || sOldFile == NULL || sNewFile == NULL || strcmp(sOldFile, sNewFile) == 0 ) return XUI_OK;
	pCategory = &g_arrMaterialCategories[mapedit_material_category_clamp(pApp->iMaterialCategory)];
	for ( i = 0; i < pApp->tTilesetFiles.iCount; ++i ) {
		char rel[MAPEDIT_PATH_MAX];
		char path[MAPEDIT_PATH_MAX];
		xvalue* pRoot;
		int changed = 0;
		snprintf(rel, sizeof(rel), "assets\\图块集\\%s", pApp->tTilesetFiles.arrNames[i]);
		mapedit_app_path(pApp, path, sizeof(path), rel);
		pRoot = xrtXsonParseFile(path);
		if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) {
			if ( pRoot != NULL ) xrtValueRelease(pRoot);
			allOk = 0;
			continue;
		}
		if ( pApp->iMaterialCategory == MAPEDIT_MATERIAL_MODE_STATIC ) {
			const char* sStatic = mapedit_table_text(pRoot, SET_KEY_STATIC, "");
			if ( strcmp(sStatic, sOldFile) == 0 ) {
				changed = xuiXrtValueObjectSetText(pRoot, SET_KEY_STATIC, strlen(SET_KEY_STATIC), sNewFile, 0u, FALSE) ? 1 : -1;
			}
		} else {
			xvalue* pItems = mapedit_table_get(pRoot, SET_KEY_SPECIAL_TILES);
			if ( pItems != NULL && xuiXrtValueType(pItems) == XVALUE_ARRAY ) {
				size_t j;
				for ( j = 0u; j < xrtValueCount(pItems); ++j ) {
					xvalue* pItem = xuiXrtValueArrayGet(pItems, j);
					if ( pItem == NULL || xuiXrtValueType(pItem) != XVALUE_OBJECT ) continue;
					if ( strcmp(mapedit_table_text(pItem, SET_KEY_SPECIAL_TYPE, ""), pCategory->sTitle) == 0 &&
					     strcmp(mapedit_table_text(pItem, SET_KEY_SPECIAL_FILE, ""), sOldFile) == 0 ) {
						if ( !xuiXrtValueObjectSetText(pItem, SET_KEY_SPECIAL_FILE, strlen(SET_KEY_SPECIAL_FILE), sNewFile, 0u, FALSE) ) {
							changed = -1;
							break;
						}
						changed = 1;
					}
				}
			}
		}
		if ( changed > 0 ) {
			if ( !xuiXrtValueObjectSetText(pRoot, SET_KEY_FORMAT, strlen(SET_KEY_FORMAT), MAPEDIT_TILESET_FORMAT_NAME, 0u, FALSE) ||
			     !xuiXrtValueObjectSetInt(pRoot, SET_KEY_VERSION, strlen(SET_KEY_VERSION), MAPEDIT_TILESET_FORMAT_VERSION) ||
			     !xrtXsonStringifyFile(path, pRoot, true) ) allOk = 0;
		} else if ( changed < 0 ) {
			allOk = 0;
		}
		xrtValueRelease(pRoot);
	}
	if ( pApp->iMaterialCategory == MAPEDIT_MATERIAL_MODE_STATIC ) {
		if ( strcmp(pApp->tTileset.sStaticFile, sOldFile) == 0 ) {
			mapedit_copy_text(pApp->tTileset.sStaticFile, sizeof(pApp->tTileset.sStaticFile), sNewFile);
		}
	} else {
		for ( i = 0; i < pApp->tTileset.iSpecialCount; ++i ) {
			if ( strcmp(pApp->tTileset.arrSpecial[i].sType, pCategory->sTitle) == 0 &&
			     strcmp(pApp->tTileset.arrSpecial[i].sFile, sOldFile) == 0 ) {
				mapedit_copy_text(pApp->tTileset.arrSpecial[i].sFile,
					sizeof(pApp->tTileset.arrSpecial[i].sFile), sNewFile);
			}
		}
	}
	return allOk ? XUI_OK : XUI_ERROR;
}

static int mapedit_material_edit_submit(mapedit_app_t* pApp)
{
	const char* sNameIn;
	const char* sFileIn;
	char name[MAPEDIT_NAME_MAX];
	char file[MAPEDIT_FILE_MAX];
	char dir[MAPEDIT_PATH_MAX];
	char target[MAPEDIT_PATH_MAX];
	char oldPath[MAPEDIT_PATH_MAX];
	int saveCols;
	int saveRows;
	int saveW;
	int saveH;
	int editing;
	int sameFile;
	int index;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sNameIn = xuiInputGetText(pApp->pMaterialEditNameInput);
	sFileIn = xuiInputGetText(pApp->pMaterialEditFileInput);
	mapedit_trim_copy(name, sizeof(name), sNameIn);
	if ( name[0] == 0 ) {
		mapedit_material_edit_show_error(pApp, pApp->pMaterialEditNameInput, "请输入图块名称");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !mapedit_material_edit_file_name(sFileIn, file, sizeof(file)) ) {
		mapedit_material_edit_show_error(pApp, pApp->pMaterialEditFileInput, "请输入文件名");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->pMaterialEditOutputPixels == NULL ) {
		mapedit_status(pApp, "保存图块失败");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	editing = pApp->iMaterialEditIndex >= 0 && pApp->iMaterialEditIndex < pApp->tMaterialCategoryFiles.iCount;
	sameFile = editing && strcmp(pApp->sMaterialEditOriginalFile, file) == 0;
	mapedit_material_category_dir(pApp, dir, sizeof(dir));
	if ( !mapedit_ensure_dir_utf8(dir) ) {
		mapedit_status(pApp, "保存图块失败");
		return XUI_ERROR;
	}
	mapedit_path_join(target, sizeof(target), dir, file);
	if ( mapedit_file_exists_utf8(target) && !sameFile ) {
		mapedit_status(pApp, "保存图块失败");
		return XUI_ERROR;
	}
	if ( editing ) {
		mapedit_path_join(oldPath, sizeof(oldPath), dir, pApp->sMaterialEditOriginalFile);
		if ( mapedit_file_exists_utf8(oldPath) && !mapedit_material_backup_file(pApp, pApp->sMaterialEditOriginalFile) ) {
			mapedit_status(pApp, "保存图块失败");
			return XUI_ERROR;
		}
	}
	saveCols = pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ? MAPEDIT_MATERIAL_STATIC_COLS : pApp->iMaterialEditUsedCols;
	saveRows = pApp->iMaterialEditUsedRows;
	if ( saveCols <= 0 ) saveCols = 1;
	if ( saveRows <= 0 ) saveRows = 1;
	saveCols = mapedit_min_i(saveCols, pApp->iMaterialEditOutputCols);
	saveRows = mapedit_min_i(saveRows, pApp->iMaterialEditOutputRows);
	saveW = saveCols * MAPEDIT_TILE_W;
	saveH = saveRows * MAPEDIT_TILE_H;
	if ( xgeImageSavePNG(target, saveW, saveH, pApp->pMaterialEditOutputPixels, pApp->iMaterialEditOutputCols * MAPEDIT_TILE_W * 4) != XGE_OK ) {
		mapedit_status(pApp, "保存图块失败");
		return XUI_ERROR;
	}
	if ( mapedit_material_edit_update_mapping(pApp, editing ? pApp->sMaterialEditOriginalFile : NULL, file, name) != XUI_OK ) {
		mapedit_status(pApp, "保存图块失败");
		return XUI_ERROR;
	}
	if ( editing && !sameFile ) {
		if ( mapedit_material_update_tileset_references(pApp, pApp->sMaterialEditOriginalFile, file) != XUI_OK ) {
			mapedit_status(pApp, "新图块已保存，但引用更新未完成；旧文件已保留，可安全重试");
			return XUI_ERROR;
		}
		mapedit_path_join(oldPath, sizeof(oldPath), dir, pApp->sMaterialEditOriginalFile);
		if ( !mapedit_delete_file_utf8(oldPath) ) {
			mapedit_status(pApp, "引用已更新，但旧图块文件删除失败");
			return XUI_ERROR;
		}
	}
	if ( mapedit_material_scan_category(pApp, pApp->iMaterialCategory, 1) == XUI_OK ) {
		index = mapedit_file_list_find(&pApp->tMaterialCategoryFiles, file);
		if ( index >= 0 ) {
			pApp->tMaterialCategoryFiles.iSelected = index;
			if ( pApp->pMaterialListView != NULL ) (void)xuiListViewSetSelected(pApp->pMaterialListView, index);
		}
	}
	mapedit_material_edit_close(pApp);
	mapedit_status(pApp, "图块已保存");
	return XUI_OK;
}

static void mapedit_tileset_refresh_surfaces(mapedit_app_t* pApp)
{
	char sPath[MAPEDIT_PATH_MAX], sAssetPath[MAPEDIT_PATH_MAX];
	int i;
	if (pApp->tTileset.pStaticSurface) pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tTileset.pStaticSurface);
	pApp->tTileset.pStaticSurface = NULL;
	memset(&pApp->tTileset.tStaticDesc, 0, sizeof(pApp->tTileset.tStaticDesc));
	for (i = 0; i < MAPEDIT_SPECIAL_MAX; ++i) {
		if (pApp->tTileset.arrSpecial[i].pSurface) pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tTileset.arrSpecial[i].pSurface);
		pApp->tTileset.arrSpecial[i].pSurface = NULL;
		memset(&pApp->tTileset.arrSpecial[i].tDesc, 0, sizeof(pApp->tTileset.arrSpecial[i].tDesc));
	}
	if ( pApp->tTileset.sStaticFile[0] != 0 ) {
		snprintf(sAssetPath, sizeof(sAssetPath), "assets\\tilesheets\\%s", pApp->tTileset.sStaticFile);
		mapedit_app_path(pApp, sPath, sizeof(sPath), sAssetPath);
		if ( mapedit_file_exists_utf8(sPath) ) {
			(void)mapedit_surface_load(pApp, sPath, &pApp->tTileset.pStaticSurface, &pApp->tTileset.tStaticDesc);
		}
	}
	for ( i = 0; i < pApp->tTileset.iSpecialCount; i++ ) {
		(void)mapedit_tileset_load_special_surface(pApp,
			pApp->tTileset.arrSpecial[i].sType, pApp->tTileset.arrSpecial[i].sFile,
			&pApp->tTileset.arrSpecial[i].pSurface, &pApp->tTileset.arrSpecial[i].tDesc);
	}

}

static int mapedit_tileset_load(mapedit_app_t* pApp, const char* sFile)
{
	char sPath[MAPEDIT_PATH_MAX];
	char sAssetPath[MAPEDIT_PATH_MAX];
	mapedit_tileset_t* pLoaded;
	xvalue* pRoot;
	xvalue* pArray;
	xvalue* pValue;
	const char* sFormat;
	const char* sStatic;
	const char* sText;
	size_t iArrayCount;
	long long iWideValue;
	int iVersion;
	int iValue;
	int i;
	if ( pApp == NULL || sFile == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if (strchr(sFile, ':') || sFile[0] == '/' || sFile[0] == '\\') mapedit_copy_text(sAssetPath, sizeof(sAssetPath), sFile);
	else {
		snprintf(sPath, sizeof(sPath), "assets\\图块集\\%s", sFile);
		mapedit_app_path(pApp, sAssetPath, sizeof(sAssetPath), sPath);
	}
	pRoot = xrtXsonParseFile(sAssetPath);
	if ( pRoot == NULL || xuiXrtValueType(pRoot) != XVALUE_OBJECT ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	pLoaded = (mapedit_tileset_t*)calloc(1u, sizeof(*pLoaded));
	if ( pLoaded == NULL ) {
		xrtValueRelease(pRoot);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pLoaded->arrPassage, 255, sizeof(pLoaded->arrPassage));
	mapedit_copy_text(pLoaded->sPath, sizeof(pLoaded->sPath), sAssetPath);
	mapedit_copy_text(pLoaded->sFile, sizeof(pLoaded->sFile), sFile);
	if ( !mapedit_table_read_text_strict(pRoot, SET_KEY_FORMAT, "", &sFormat) ||
	     !mapedit_table_read_int_strict(pRoot, SET_KEY_VERSION, 1, &iVersion) ) goto format_error;
	if ( (sFormat[0] != 0 && strcmp(sFormat, MAPEDIT_TILESET_FORMAT_NAME) != 0) ||
	     iVersion <= 0 || iVersion > MAPEDIT_TILESET_FORMAT_VERSION ) {
		goto format_error;
	}
	if ( !mapedit_table_read_text_strict(pRoot, SET_KEY_NAME, sFile, &sText) ) goto format_error;
	mapedit_copy_text(pLoaded->sName, sizeof(pLoaded->sName), sText);
	if ( !mapedit_table_read_text_strict(pRoot, SET_KEY_CUSTOM, "", &sText) ) goto format_error;
	mapedit_copy_text(pLoaded->sCustomData, sizeof(pLoaded->sCustomData), sText);
	if ( !mapedit_table_read_text_strict(pRoot, SET_KEY_STATIC, "", &sStatic) ) goto format_error;
	if ( strcmp(sStatic, "未设置") == 0 ) sStatic = "";
	mapedit_copy_text(pLoaded->sStaticFile, sizeof(pLoaded->sStaticFile), sStatic);
	pLoaded->iSchemaVersion = iVersion;
	pArray = mapedit_table_get(pRoot, SET_KEY_SPECIAL_TILES);
	if ( pArray != NULL && xuiXrtValueType(pArray) != XVALUE_ARRAY ) goto format_error;
	iArrayCount = pArray != NULL ? xrtValueCount(pArray) : 0u;
	if ( iArrayCount > MAPEDIT_SPECIAL_MAX ||
	     !mapedit_table_read_int_strict(pRoot, SET_KEY_SPECIAL_COUNT, (int)iArrayCount, &pLoaded->iSpecialCount) ||
	     pLoaded->iSpecialCount < 0 || pLoaded->iSpecialCount > MAPEDIT_SPECIAL_MAX ||
	     (size_t)pLoaded->iSpecialCount > iArrayCount ||
	     (iVersion >= MAPEDIT_TILESET_FORMAT_VERSION && (size_t)pLoaded->iSpecialCount != iArrayCount) ) goto format_error;
	if ( pArray != NULL ) {
		for ( i = 0; i < pLoaded->iSpecialCount; i++ ) {
			xvalue* pItem = xuiXrtValueArrayGet(pArray, (uint32)i);
			const char* sType;
			const char* sName;
			if ( pItem == NULL || xuiXrtValueType(pItem) != XVALUE_OBJECT ||
			     !mapedit_table_read_text_strict(pItem, SET_KEY_SPECIAL_TYPE, "", &sType) ||
			     !mapedit_table_read_text_strict(pItem, SET_KEY_SPECIAL_FILE, "", &sName) ) goto format_error;
			mapedit_copy_text(pLoaded->arrSpecial[i].sType, sizeof(pLoaded->arrSpecial[i].sType), sType);
			mapedit_copy_text(pLoaded->arrSpecial[i].sFile, sizeof(pLoaded->arrSpecial[i].sFile), sName);
		}
	}
	pArray = mapedit_table_get(pRoot, SET_KEY_PASSAGE);
	if ( pArray != NULL ) {
		if ( xuiXrtValueType(pArray) != XVALUE_ARRAY || xrtValueCount(pArray) > sizeof(pLoaded->arrPassage) ) goto format_error;
		pLoaded->iPassageCount = (int)xrtValueCount(pArray);
		for ( i = 0; i < pLoaded->iPassageCount; i++ ) {
			pValue = xuiXrtValueArrayGet(pArray, (uint32)i);
			if ( pValue == NULL || xuiXrtValueType(pValue) != XVALUE_INT ) goto format_error;
			iWideValue = (long long)xuiXrtValueGetInt(pValue);
			if ( iWideValue < 0 || iWideValue > 255 ) goto format_error;
			iValue = (int)iWideValue;
			pLoaded->arrPassage[i] = (unsigned char)iValue;
		}
	}
	pArray = mapedit_table_get(pRoot, SET_KEY_ACTOR);
	if ( pArray != NULL ) {
		if ( xuiXrtValueType(pArray) != XVALUE_ARRAY || xrtValueCount(pArray) > sizeof(pLoaded->arrActorOverlay) ) goto format_error;
		pLoaded->iActorOverlayCount = (int)xrtValueCount(pArray);
		for ( i = 0; i < pLoaded->iActorOverlayCount; i++ ) {
			pValue = xuiXrtValueArrayGet(pArray, (uint32)i);
			if ( pValue == NULL || xuiXrtValueType(pValue) != XVALUE_INT ) goto format_error;
			iWideValue = (long long)xuiXrtValueGetInt(pValue);
			if ( iWideValue != 0 && iWideValue != 1 ) goto format_error;
			iValue = (int)iWideValue;
			pLoaded->arrActorOverlay[i] = (unsigned char)iValue;
		}
	}
	pValue = mapedit_table_get(pRoot, SET_KEY_TILE_DATA);
	if ( pValue != NULL && xuiXrtValueType(pValue) != XVALUE_OBJECT ) goto format_error;
	pLoaded->pTileCustomRaw = pValue != NULL ? xrtValueClone(pValue) : xrtValueObject();
	if ( pLoaded->pTileCustomRaw == NULL ) {
		free(pLoaded);
		xrtValueRelease(pRoot);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pLoaded->bLoaded = 1;
	pLoaded->bDirty = 0;
	xrtValueRelease(pRoot);
	mapedit_tileset_clear(pApp);
	pApp->tTileset = *pLoaded;
	free(pLoaded);
	mapeditHistoryClear(&pApp->tTilesetHistory);
	mapedit_tileset_refresh_surfaces(pApp);
	pApp->tTilesetStamp = mapeditFileStamp(pApp->tTileset.sPath);
	pApp->bTilesetExternalConflict = 0;
	return XUI_OK;

format_error:
	if ( pLoaded->pTileCustomRaw != NULL ) xrtValueRelease(pLoaded->pTileCustomRaw);
	free(pLoaded);
	xrtValueRelease(pRoot);
	return XUI_ERROR;
}

static int mapedit_tileset_tile_count(mapedit_app_t* pApp)
{
	int staticRows = 0;
	int staticStart;
	int count;
	if ( pApp == NULL ) return 0;
	if ( pApp->tTileset.tStaticDesc.iWidth > 0 && pApp->tTileset.tStaticDesc.iHeight > 0 ) {
		staticRows = (pApp->tTileset.tStaticDesc.iHeight + MAPEDIT_TILE_H - 1) / MAPEDIT_TILE_H;
	}
	count = pApp->tTileset.iSpecialCount;
	if ( staticRows > 0 ) {
		staticStart = ((1 + pApp->tTileset.iSpecialCount + MAPEDIT_TILES_PER_ROW - 1) / MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILES_PER_ROW;
		count = staticStart + staticRows * MAPEDIT_TILES_PER_ROW;
	}
	count = mapedit_max_i(count, pApp->tTileset.iPassageCount);
	count = mapedit_max_i(count, pApp->tTileset.iActorOverlayCount);
	if ( count > (int)sizeof(pApp->tTileset.arrPassage) ) count = (int)sizeof(pApp->tTileset.arrPassage);
	return count;
}

static int mapedit_tileset_save_to(mapedit_app_t* pApp, const char* path, int commit)
{
	xvalue* pRoot;
	xvalue* pSpecialTiles;
	xvalue* pPassage;
	xvalue* pActorOverlay;
	xvalue* pTileCustom;
	int i;
	int count;
	int ok;
	if ( pApp == NULL || !pApp->tTileset.bLoaded || path == NULL || path[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pRoot = xrtValueObject();
	pSpecialTiles = xrtValueArray();
	pPassage = xrtValueArray();
	pActorOverlay = xrtValueArray();
	pTileCustom = (pApp->tTileset.pTileCustomRaw != NULL) ? xrtValueClone(pApp->tTileset.pTileCustomRaw) : xrtValueObject();
	if ( pRoot == NULL || pSpecialTiles == NULL || pPassage == NULL || pActorOverlay == NULL || pTileCustom == NULL ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		if ( pSpecialTiles != NULL ) xrtValueRelease(pSpecialTiles);
		if ( pPassage != NULL ) xrtValueRelease(pPassage);
		if ( pActorOverlay != NULL ) xrtValueRelease(pActorOverlay);
		if ( pTileCustom != NULL ) xrtValueRelease(pTileCustom);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < pApp->tTileset.iSpecialCount; i++ ) {
		xvalue* pItem = xrtValueObject();
		if ( pItem == NULL ||
		     !xuiXrtValueObjectSetText(pItem, SET_KEY_SPECIAL_TYPE, (uint32)strlen(SET_KEY_SPECIAL_TYPE), pApp->tTileset.arrSpecial[i].sType, 0, FALSE) ||
		     !xuiXrtValueObjectSetText(pItem, SET_KEY_SPECIAL_FILE, (uint32)strlen(SET_KEY_SPECIAL_FILE), pApp->tTileset.arrSpecial[i].sFile, 0, FALSE) ||
		     !xuiXrtValueArrayAppendTake(pSpecialTiles, pItem, TRUE) ) {
			if ( pItem != NULL ) xrtValueRelease(pItem);
			xrtValueRelease(pRoot);
			xrtValueRelease(pSpecialTiles);
			xrtValueRelease(pPassage);
			xrtValueRelease(pActorOverlay);
			xrtValueRelease(pTileCustom);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	count = mapedit_tileset_tile_count(pApp);
	if ( count > 0 && count < (int)sizeof(pApp->tTileset.arrPassage) ) count++;
	for ( i = 0; i < count; i++ ) {
		if ( !xuiXrtValueArrayAppendInt(pPassage, (int)pApp->tTileset.arrPassage[i]) ||
		     !xuiXrtValueArrayAppendInt(pActorOverlay, (int)(pApp->tTileset.arrActorOverlay[i] ? 1 : 0)) ) {
			xrtValueRelease(pRoot);
			xrtValueRelease(pSpecialTiles);
			xrtValueRelease(pPassage);
			xrtValueRelease(pActorOverlay);
			xrtValueRelease(pTileCustom);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xuiXrtValueObjectSetText(pRoot, SET_KEY_FORMAT, (uint32)strlen(SET_KEY_FORMAT), MAPEDIT_TILESET_FORMAT_NAME, 0, FALSE) ||
	     !xuiXrtValueObjectSetInt(pRoot, SET_KEY_VERSION, (uint32)strlen(SET_KEY_VERSION), MAPEDIT_TILESET_FORMAT_VERSION) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_NAME, (uint32)strlen(SET_KEY_NAME), (pApp->tTileset.sName[0] ? pApp->tTileset.sName : pApp->tTileset.sFile), 0, FALSE) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_STATIC, (uint32)strlen(SET_KEY_STATIC), pApp->tTileset.sStaticFile, 0, FALSE) ||
	     !xuiXrtValueObjectSetInt(pRoot, SET_KEY_SPECIAL_COUNT, (uint32)strlen(SET_KEY_SPECIAL_COUNT), pApp->tTileset.iSpecialCount) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_CUSTOM, (uint32)strlen(SET_KEY_CUSTOM), pApp->tTileset.sCustomData, 0, FALSE) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_SPECIAL_TILES, &pSpecialTiles) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_PASSAGE, &pPassage) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_ACTOR, &pActorOverlay) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_TILE_DATA, &pTileCustom) ) goto save_out_of_memory;
	ok = xrtXsonStringifyFile(path, pRoot, true) ? XUI_OK : XUI_ERROR;
	xrtValueRelease(pRoot);
	if ( ok == XUI_OK && commit ) {
		pApp->tTileset.iSchemaVersion = MAPEDIT_TILESET_FORMAT_VERSION;
		pApp->tTileset.iPassageCount = count;
		pApp->tTileset.iActorOverlayCount = count;
		mapedit_clear_recovery_for(pApp, MAPEDIT_WORKSPACE_TILESET, pApp->tTileset.sPath);
		mapedit_copy_text(pApp->tTileset.sPath, sizeof(pApp->tTileset.sPath), path);
		mapeditHistorySaved(&pApp->tTilesetHistory);
		pApp->tTilesetStamp = mapeditFileStamp(path);
		pApp->bTilesetExternalConflict = 0;
		pApp->tTileset.bDirty = 0;
		mapedit_update_window_title(pApp);
	}
	return ok;

save_out_of_memory:
	if ( pSpecialTiles != NULL ) xrtValueRelease(pSpecialTiles);
	if ( pPassage != NULL ) xrtValueRelease(pPassage);
	if ( pActorOverlay != NULL ) xrtValueRelease(pActorOverlay);
	if ( pTileCustom != NULL ) xrtValueRelease(pTileCustom);
	xrtValueRelease(pRoot);
	return XUI_ERROR_OUT_OF_MEMORY;
}

static void mapedit_status(mapedit_app_t* pApp, const char* sText)
{
	if ( pApp == NULL || pApp->pStatus == NULL || sText == NULL ) return;
	mapedit_copy_text(pApp->sStatusText, sizeof(pApp->sStatusText), sText);
	(void)xuiStatusBarSetItemText(pApp->pStatus, 0, pApp->sStatusText);
}

static void mapedit_update_window_title(mapedit_app_t* pApp)
{
	char title[MAPEDIT_NAME_MAX + 32];
	const char* sName;
	if ( pApp == NULL ) return;
	if ( pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_TILESET ) {
		sName = pApp->tTileset.sName[0] != 0 ? pApp->tTileset.sName : "未命名图集";
	} else {
		sName = pApp->tMap.sName[0] != 0 ? pApp->tMap.sName : "未命名地图";
	}
	snprintf(title, sizeof(title), "MapEdit - %s%s", sName,
		(pApp->tMap.bDirty || pApp->tTileset.bDirty) ? " *" : "");
	xgeSetTitle(title);
}

static int mapedit_capture_map(mapedit_app_t* pApp, int flags)
{
	if (mapeditDocumentCaptureMap(&pApp->tMapHistory, &pApp->tMap, flags)) return 1;
	mapedit_status(pApp, "无法分配撤销记录，文档未修改");
	return 0;
}

static int mapedit_capture_tileset(mapedit_app_t* pApp)
{
	if (mapeditDocumentCaptureTileset(&pApp->tTilesetHistory, &pApp->tTileset)) return 1;
	mapedit_status(pApp, "无法分配撤销记录，图集未修改");
	return 0;
}

static void mapedit_mark_map_dirty(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if (pApp->tMapHistory.tCurrentCommand.pPayload) {
		if (mapeditDocumentUnchanged(&pApp->tMapHistory.tCurrentCommand, &pApp->tMap, &pApp->tTileset)) mapeditHistoryBegin(&pApp->tMapHistory);
		else (void)mapeditHistoryCommit(&pApp->tMapHistory);
	}
	pApp->tMap.bDirty = mapeditHistoryDirty(&pApp->tMapHistory);
	mapedit_refresh_toolbar_state(pApp);
	if ( pApp->fMapDirtySince <= 0.0 ) pApp->fMapDirtySince = xgeTimer();
	mapedit_update_window_title(pApp);
	xgeRenderRequest();
	xgeRenderRequestAfter(2.0f);
}

static void mapedit_mark_tileset_dirty(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if (pApp->tTilesetHistory.tCurrentCommand.pPayload) {
		if (mapeditDocumentUnchanged(&pApp->tTilesetHistory.tCurrentCommand, &pApp->tMap, &pApp->tTileset)) mapeditHistoryBegin(&pApp->tTilesetHistory);
		else (void)mapeditHistoryCommit(&pApp->tTilesetHistory);
	}
	pApp->tTileset.bDirty = mapeditHistoryDirty(&pApp->tTilesetHistory);
	mapedit_refresh_toolbar_state(pApp);
	mapedit_update_window_title(pApp);
	xgeRenderRequest();
	xgeRenderRequestAfter(2.0f);
}

static void mapedit_update_status_details(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->pStatus == NULL ) return;
	snprintf(pApp->sStatusMapText, sizeof(pApp->sStatusMapText), "地图: %d x %d", pApp->tMap.iWidth > 0 ? pApp->tMap.iWidth : 100, pApp->tMap.iHeight > 0 ? pApp->tMap.iHeight : 100);
	(void)xuiStatusBarSetItemText(pApp->pStatus, 1, pApp->sStatusMapText);
	snprintf(pApp->sStatusTileText, sizeof(pApp->sStatusTileText), "图块: tileset %d / tile %d / layer %d",
		pApp->tTilesetFiles.iSelected >= 0 ? pApp->tTilesetFiles.iSelected : 0,
		pApp->iSelectedTile,
		pApp->iActiveLayer);
	(void)xuiStatusBarSetItemText(pApp->pStatus, 2, pApp->sStatusTileText);
	snprintf(pApp->sStatusPositionText, sizeof(pApp->sStatusPositionText), "位置: passable / region 0 / event none");
	(void)xuiStatusBarSetItemText(pApp->pStatus, 4, pApp->sStatusPositionText);
}

static int mapedit_tile_static_start(mapedit_app_t* pApp)
{
	int reserved;
	reserved = 1 + ((pApp != NULL) ? pApp->tTileset.iSpecialCount : 0);
	if ( reserved < 1 ) reserved = 1;
	return ((reserved + MAPEDIT_TILES_PER_ROW - 1) / MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILES_PER_ROW;
}

static int mapedit_tile_static_cols(mapedit_app_t* pApp)
{
	int cols;
	if ( pApp == NULL || pApp->tTileset.tStaticDesc.iWidth <= 0 ) return 0;
	cols = (pApp->tTileset.tStaticDesc.iWidth + MAPEDIT_TILE_W - 1) / MAPEDIT_TILE_W;
	if ( cols > MAPEDIT_TILES_PER_ROW ) cols = MAPEDIT_TILES_PER_ROW;
	return cols > 0 ? cols : 0;
}

static int mapedit_tile_static_rows(mapedit_app_t* pApp)
{
	int rows;
	if ( pApp == NULL || pApp->tTileset.tStaticDesc.iHeight <= 0 ) return MAPEDIT_TILE_SELECT_PLACEHOLDER_ROWS;
	rows = (pApp->tTileset.tStaticDesc.iHeight + MAPEDIT_TILE_H - 1) / MAPEDIT_TILE_H;
	return rows > 0 ? rows : 1;
}

static int mapedit_tile_select_row_count(mapedit_app_t* pApp)
{
	int rows;
	if ( pApp == NULL ) return MAPEDIT_TILE_SELECT_PLACEHOLDER_ROWS;
	rows = mapedit_tile_static_start(pApp) / MAPEDIT_TILES_PER_ROW + mapedit_tile_static_rows(pApp);
	if ( rows < MAPEDIT_TILE_SELECT_PLACEHOLDER_ROWS ) rows = MAPEDIT_TILE_SELECT_PLACEHOLDER_ROWS;
	return rows;
}

static void mapedit_tileset_panel_scroll_ptrs(mapedit_app_t* pApp, xui_widget pWidget, float** ppScrollX, float** ppScrollY)
{
	if ( ppScrollX != NULL ) *ppScrollX = NULL;
	if ( ppScrollY != NULL ) *ppScrollY = NULL;
	if ( pApp == NULL || pWidget == NULL ) return;
	if ( pWidget == pApp->pTilesetArrangeCanvas ) {
		if ( ppScrollX != NULL ) *ppScrollX = &pApp->fTilesetArrangeScrollX;
		if ( ppScrollY != NULL ) *ppScrollY = &pApp->fTilesetArrangeScrollY;
	} else if ( pWidget == pApp->pTilesetPassageCanvas ) {
		if ( ppScrollX != NULL ) *ppScrollX = &pApp->fTilesetPassageScrollX;
		if ( ppScrollY != NULL ) *ppScrollY = &pApp->fTilesetPassageScrollY;
	} else if ( pWidget == pApp->pTilesetActorCanvas ) {
		if ( ppScrollX != NULL ) *ppScrollX = &pApp->fTilesetActorScrollX;
		if ( ppScrollY != NULL ) *ppScrollY = &pApp->fTilesetActorScrollY;
	} else if ( pWidget == pApp->pTilesetTagsCanvas ) {
		if ( ppScrollX != NULL ) *ppScrollX = &pApp->fTilesetTagsScrollX;
		if ( ppScrollY != NULL ) *ppScrollY = &pApp->fTilesetTagsScrollY;
	}
}

static int* mapedit_tileset_panel_hover_ptr(mapedit_app_t* pApp, xui_widget pWidget)
{
	if ( pApp == NULL || pWidget == NULL ) return NULL;
	if ( pWidget == pApp->pTilesetArrangeCanvas ) return &pApp->iTilesetArrangeHoverTile;
	if ( pWidget == pApp->pTilesetTagsCanvas ) return &pApp->iTilesetTagsHoverTile;
	return NULL;
}

static void mapedit_tileset_panel_set_hover(mapedit_app_t* pApp, xui_widget pWidget, int iTile)
{
	int* pHover;
	if ( pApp == NULL || pWidget == NULL ) return;
	pHover = mapedit_tileset_panel_hover_ptr(pApp, pWidget);
	if ( pHover == NULL || *pHover == iTile ) return;
	*pHover = iTile;
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_view_index(mapedit_app_t* pApp, xui_widget widget)
{
	if (widget == pApp->pMapCanvas) return 0;
	if (widget == pApp->pMapPassageCanvas) return 1;
	if (widget == pApp->pMapTagsCanvas) return 2;
	if (widget == pApp->pTileSelectCanvas) return 3;
	if (widget == pApp->pTilesetArrangeCanvas) return 4;
	if (widget == pApp->pTilesetPassageCanvas) return 5;
	if (widget == pApp->pTilesetActorCanvas) return 6;
	return 7;
}

static int mapedit_view_cell(mapedit_app_t* pApp, xui_widget widget, int axis)
{
	int index = mapedit_view_index(pApp, widget);
	int base = index == 1 || index == 5 ? 32 : (axis ? MAPEDIT_TILE_H : MAPEDIT_TILE_W);
	return mapeditViewCell(base, pApp->arrViewZoom[index]);
}

static int mapedit_tileset_panel_cell_width(mapedit_app_t* pApp, xui_widget pWidget)
{
	return mapedit_view_cell(pApp, pWidget, 0);
}

static int mapedit_tileset_panel_cell_height(mapedit_app_t* pApp, xui_widget pWidget)
{
	return mapedit_view_cell(pApp, pWidget, 1);
}

static float mapedit_tileset_panel_content_width(mapedit_app_t* pApp, xui_widget pWidget)
{
	return (float)(MAPEDIT_TILES_PER_ROW * mapedit_tileset_panel_cell_width(pApp, pWidget));
}

static float mapedit_tileset_panel_content_height(mapedit_app_t* pApp, xui_widget pWidget)
{
	return (float)(mapedit_tile_select_row_count(pApp) * mapedit_tileset_panel_cell_height(pApp, pWidget));
}

static void mapedit_tileset_workspace_scroll_sync_content(mapedit_app_t* pApp);

static void mapedit_tileset_panel_max_scroll(mapedit_app_t* pApp, xui_widget pWidget, float* pMaxX, float* pMaxY)
{
	xui_rect_t wr;
	float maxX;
	float maxY;
	if ( pMaxX != NULL ) *pMaxX = 0.0f;
	if ( pMaxY != NULL ) *pMaxY = 0.0f;
	if ( pApp == NULL || pWidget == NULL ) return;
	wr = xuiWidgetGetWorldRect(pWidget);
	maxX = mapedit_tileset_panel_content_width(pApp, pWidget) - (float)wr.fW;
	maxY = mapedit_tileset_panel_content_height(pApp, pWidget) - (float)wr.fH;
	if ( maxX < 0.0f ) maxX = 0.0f;
	if ( maxY < 0.0f ) maxY = 0.0f;
	if ( pMaxX != NULL ) *pMaxX = maxX;
	if ( pMaxY != NULL ) *pMaxY = maxY;
}

static void mapedit_tileset_panel_clamp_scroll(mapedit_app_t* pApp, xui_widget pWidget)
{
	float* pScrollX;
	float* pScrollY;
	float maxX;
	float maxY;
	mapedit_tileset_panel_scroll_ptrs(pApp, pWidget, &pScrollX, &pScrollY);
	if ( pScrollX == NULL || pScrollY == NULL ) return;
	mapedit_tileset_panel_max_scroll(pApp, pWidget, &maxX, &maxY);
	*pScrollX = mapedit_clampf(*pScrollX, 0.0f, maxX);
	*pScrollY = mapedit_clampf(*pScrollY, 0.0f, maxY);
}

static void mapedit_reset_tileset_panel_scrolls(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	pApp->fTilesetArrangeScrollX = 0.0f;
	pApp->fTilesetArrangeScrollY = 0.0f;
	pApp->fTilesetPassageScrollX = 0.0f;
	pApp->fTilesetPassageScrollY = 0.0f;
	pApp->fTilesetActorScrollX = 0.0f;
	pApp->fTilesetActorScrollY = 0.0f;
	pApp->fTilesetTagsScrollX = 0.0f;
	pApp->fTilesetTagsScrollY = 0.0f;
	mapedit_tileset_workspace_scroll_sync_content(pApp);
	if ( pApp->pTilesetArrangeCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetArrangeCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetActorCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetActorCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_tile_select_tile_selectable(mapedit_app_t* pApp, int iTile)
{
	int staticStart;
	int local;
	int row;
	int col;
	if ( pApp == NULL || iTile < 0 ) return 0;
	if ( iTile == 0 ) return 1;
	if ( iTile >= 1 && iTile <= pApp->tTileset.iSpecialCount ) return 1;
	staticStart = mapedit_tile_static_start(pApp);
	if ( iTile < staticStart || pApp->tTileset.pStaticSurface == NULL ) return 0;
	local = iTile - staticStart;
	row = local / MAPEDIT_TILES_PER_ROW;
	col = local % MAPEDIT_TILES_PER_ROW;
	return row >= 0 && row < mapedit_tile_static_rows(pApp) && col >= 0 && col < mapedit_tile_static_cols(pApp);
}

static int mapedit_tileset_static_tile_editable(mapedit_app_t* pApp, int iTile)
{
	int staticStart;
	int local;
	int row;
	int col;
	if ( pApp == NULL || iTile < 0 || pApp->tTileset.pStaticSurface == NULL ) return 0;
	staticStart = mapedit_tile_static_start(pApp);
	if ( iTile < staticStart ) return 0;
	local = iTile - staticStart;
	row = local / MAPEDIT_TILES_PER_ROW;
	col = local % MAPEDIT_TILES_PER_ROW;
	return row >= 0 && row < mapedit_tile_static_rows(pApp) && col >= 0 && col < mapedit_tile_static_cols(pApp);
}

static int mapedit_tileset_panel_tile_editable(mapedit_app_t* pApp, xui_widget pWidget, int iTile)
{
	if ( pApp == NULL || pWidget == NULL || iTile < 0 ) return 0;
	if ( pWidget == pApp->pTilesetPassageCanvas || pWidget == pApp->pTilesetActorCanvas ) {
		if ( iTile <= 0 ) return 0;
		if ( iTile <= pApp->tTileset.iSpecialCount ) return 1;
		return mapedit_tileset_static_tile_editable(pApp, iTile);
	}
	if ( pWidget == pApp->pTilesetTagsCanvas ) {
		if ( iTile <= pApp->tTileset.iSpecialCount ) return 1;
		return mapedit_tileset_static_tile_editable(pApp, iTile);
	}
	return mapedit_tile_select_tile_selectable(pApp, iTile);
}

static int mapedit_commit_history(mapedit_app_t* pApp);
static void mapedit_refresh_map_properties(mapedit_app_t* pApp);
static void mapedit_refresh_tileset_properties(mapedit_app_t* pApp);

static const char* mapedit_layer_name(mapedit_app_t* pApp, int iLayer)
{
	if ( pApp == NULL || iLayer < 0 || iLayer >= pApp->iSetupLayerCount ) return NULL;
	return pApp->arrSetupLayers[iLayer].sName;
}

static const char* mapedit_tool_name(int iTool)
{
	switch ( iTool ) {
	case MAPEDIT_TOOL_BRUSH: return "画笔";
	case MAPEDIT_TOOL_ERASER: return "橡皮";
	case MAPEDIT_TOOL_LINE: return "线条";
	case MAPEDIT_TOOL_RECT: return "矩形填充";
	case MAPEDIT_TOOL_CIRCLE: return "圆形填充";
	case MAPEDIT_TOOL_BUCKET: return "填充模式";
	default: return "未知工具";
	}
}

static void mapedit_finish_map_edit(mapedit_app_t* pApp, const char* sAction)
{
	const char* sLayer;
	char status[160];
	if ( pApp == NULL ) return;
	pApp->bBatchEdit = 0;
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if (pApp->bHistoryFailed) {
		int i;
		mapedit_history_cmd_t* command = &pApp->tMapHistory.tCurrentCommand;
		for (i = 0; i < command->iChangeCount; ++i) pApp->tMap.pTiles[command->arrChanges[i].iIndex] = command->arrChanges[i].iOldTile;
		mapeditHistoryBegin(&pApp->tMapHistory);
		pApp->bGestureDirty = 0; pApp->bHistoryFailed = 0;
		pApp->tMap.bDirty = mapeditHistoryDirty(&pApp->tMapHistory);
		mapedit_update_window_title(pApp);
		mapedit_status(pApp, "历史内存不足，本次编辑已完整回滚");
		return;
	}
	if ( !pApp->bGestureDirty ) {
		mapedit_status(pApp, "地图没有变化");
		return;
	}
	if (!mapedit_commit_history(pApp)) { pApp->bGestureDirty = 0; mapedit_update_window_title(pApp); return; }
	if ( sAction == NULL ) sAction = "地图编辑";
	mapedit_mark_map_dirty(pApp);
	sLayer = mapedit_layer_name(pApp, pApp->iActiveLayer);
	if ( sLayer != NULL && sLayer[0] != 0 ) snprintf(status, sizeof(status), "%s已修改图层 %s（未保存）", sAction, sLayer);
	else snprintf(status, sizeof(status), "%s已修改图层 %d（未保存）", sAction, pApp->iActiveLayer + 1);
	mapedit_status(pApp, status);
	pApp->bGestureDirty = 0;
}





static void mapedit_clear_history(mapedit_app_t* pApp)
{
	if (pApp == NULL) return;
	mapeditHistoryClear(&pApp->tMapHistory);
	if (pApp->tMap.bDirty) pApp->tMapHistory.iSavedRevision = UINT64_MAX;
	mapedit_refresh_toolbar_state(pApp);
}

static void mapedit_begin_history(mapedit_app_t* pApp)
{
	if (pApp) { mapeditHistoryBegin(&pApp->tMapHistory); pApp->bHistoryFailed = 0; }
}

static int mapedit_record_history(mapedit_app_t* pApp, int iIndex, int iOld, int iNew)
{
	return mapeditHistoryRecord(&pApp->tMapHistory, iIndex, iOld, iNew) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static int mapedit_commit_history(mapedit_app_t* pApp)
{
	int changed = mapeditHistoryCommit(&pApp->tMapHistory);
	pApp->tMap.bDirty = mapeditHistoryDirty(&pApp->tMapHistory);
	mapedit_refresh_toolbar_state(pApp);
	return changed;
}

static void mapedit_set_tile(mapedit_app_t* pApp, int iX, int iY, int iLayer, int iTile)
{
	int idx;
	int oldTile;
	if ( pApp == NULL || pApp->tMap.pTiles == NULL || pApp->bHistoryFailed ) return;
	if ( iX < 0 || iY < 0 || iX >= pApp->tMap.iWidth || iY >= pApp->tMap.iHeight ) return;
	if ( iLayer < 0 || iLayer >= pApp->tMap.iLayers ) return;
	idx = (iLayer * pApp->tMap.iHeight + iY) * pApp->tMap.iWidth + iX;
	oldTile = pApp->tMap.pTiles[idx];
	if ( oldTile == iTile ) return;
	if (mapedit_record_history(pApp, idx, oldTile, iTile) != XUI_OK) {
		pApp->bHistoryFailed = 1;
		mapedit_status(pApp, "编辑历史内存不足；此图块未被修改");
		return;
	}
	pApp->tMap.pTiles[idx] = iTile;
	pApp->tMap.bDirty = 1;
	pApp->bGestureDirty = 1;
	pApp->bEditOK = 1;
	if ( !pApp->bBatchEdit ) {
		if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
}

static int mapedit_get_tile(mapedit_app_t* pApp, int iX, int iY, int iLayer)
{
	int idx;
	if ( pApp == NULL || pApp->tMap.pTiles == NULL ) return 0;
	if ( iX < 0 || iY < 0 || iX >= pApp->tMap.iWidth || iY >= pApp->tMap.iHeight ) return 0;
	if ( iLayer < 0 || iLayer >= pApp->tMap.iLayers ) return 0;
	idx = (iLayer * pApp->tMap.iHeight + iY) * pApp->tMap.iWidth + iX;
	return pApp->tMap.pTiles[idx];
}

static int mapedit_map_top_tile(mapedit_app_t* pApp, int iX, int iY)
{
	int l;
	if ( pApp == NULL ) return 0;
	for ( l = pApp->tMap.iLayers - 1; l >= 0; l-- ) {
		int tile = mapedit_get_tile(pApp, iX, iY, l);
		if ( tile > 0 ) return tile;
	}
	return 0;
}

static unsigned char mapedit_map_default_passage(mapedit_app_t* pApp, int iX, int iY)
{
	int tile;
	if ( pApp == NULL ) return 255;
	tile = mapedit_map_top_tile(pApp, iX, iY);
	if ( tile <= 0 || tile >= (int)sizeof(pApp->tTileset.arrPassage) || tile >= pApp->tTileset.iPassageCount ) return 255;
	return pApp->tTileset.arrPassage[tile];
}

static void mapedit_map_cell_custom_fallback(mapedit_app_t* pApp, mapedit_custom_channel_def_t* pDef, int iX, int iY, char* sOut, int iCap)
{
	int tile;
	if ( sOut != NULL && iCap > 0 ) mapedit_copy_text(sOut, iCap, pDef != NULL ? pDef->sDefaultValue : "");
	if ( pApp == NULL || pDef == NULL || sOut == NULL || iCap <= 0 ) return;
	if ( pDef->sScope[0] != 0 && strcmp(pDef->sScope, "tile") != 0 ) return;
	tile = mapedit_map_top_tile(pApp, iX, iY);
	if ( tile < 0 ) tile = 0;
	if ( !mapedit_custom_get_value(pApp->tTileset.pTileCustomRaw, pDef->sId, SET_KEY_TILE, tile, sOut, iCap) ) {
		mapedit_copy_text(sOut, iCap, pDef->sDefaultValue);
	}
}

static int mapedit_map_set_cell_custom_value(mapedit_app_t* pApp, mapedit_custom_channel_def_t* pDef, int iX, int iY, const char* sValue)
{
	char fallback[MAPEDIT_CUSTOM_VALUE_MAX];
	int cell;
	int ret;
	if ( pApp == NULL || pDef == NULL || pDef->sId[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	cell = mapedit_map_cell_id(&pApp->tMap, iX, iY);
	if ( cell < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sValue == NULL ) sValue = "";
	mapedit_map_cell_custom_fallback(pApp, pDef, iX, iY, fallback, sizeof(fallback));
	if ( strcmp(sValue, fallback) == 0 ) ret = mapedit_custom_remove_value(pApp->tMap.pCellDataRaw, pDef->sId, MAP_KEY_CELL, cell);
	else ret = mapedit_custom_set_value(&pApp->tMap.pCellDataRaw, pDef->sId, MAP_KEY_CELL, cell, sValue);
	if ( ret != XUI_OK ) return ret;
	pApp->tMap.bDirty = 1;
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

static unsigned char mapedit_map_get_cell_passage(mapedit_app_t* pApp, int iX, int iY)
{
	int cell;
	int idx;
	if ( pApp == NULL ) return 255;
	cell = mapedit_map_cell_id(&pApp->tMap, iX, iY);
	if ( cell < 0 ) return 255;
	idx = mapedit_map_find_passage_override(&pApp->tMap, cell);
	if ( idx >= 0 ) return pApp->tMap.arrPassageOverrides[idx].iValue;
	return mapedit_map_default_passage(pApp, iX, iY);
}

static int mapedit_map_set_cell_passage(mapedit_app_t* pApp, int iX, int iY, unsigned char iValue)
{
	int cell;
	int idx;
	unsigned char def;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	cell = mapedit_map_cell_id(&pApp->tMap, iX, iY);
	if ( cell < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	def = mapedit_map_default_passage(pApp, iX, iY);
	idx = mapedit_map_find_passage_override(&pApp->tMap, cell);
	if ( iValue == def ) {
		if ( idx >= 0 ) mapedit_map_remove_passage_override(&pApp->tMap, idx);
	} else if ( mapedit_map_set_passage_override(&pApp->tMap, cell, iValue) != XUI_OK ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pApp->tMap.bDirty = 1;
	pApp->iMapPassageSelectedCell = cell;
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

static int mapedit_paint_tile(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return 0;
	if ( pApp->iActiveTool == MAPEDIT_TOOL_ERASER ) return 0;
	return mapedit_tile_select_tile_selectable(pApp, pApp->iSelectedTile) ? pApp->iSelectedTile : -1;
}

static int mapedit_need_brush(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return 0;
	if ( pApp->iActiveTool == MAPEDIT_TOOL_ERASER ) return 1;
	if ( mapedit_tile_select_tile_selectable(pApp, pApp->iSelectedTile) ) return 1;
	mapedit_status(pApp, "请先在图块选择中选择图块");
	return 0;
}

static int mapedit_brush_tile_at(mapedit_app_t* pApp, int dx, int dy)
{
	if ( pApp == NULL || pApp->iActiveTool == MAPEDIT_TOOL_ERASER ) return 0;
	return pApp->iSelectedTile + dy * MAPEDIT_TILES_PER_ROW + dx;
}

static void mapedit_apply_tile_at(mapedit_app_t* pApp, int iX, int iY)
{
	int tile;
	if ( pApp == NULL ) return;
	if ( !mapedit_need_brush(pApp) ) return;
	tile = mapedit_paint_tile(pApp);
	if ( tile < 0 ) return;
	mapedit_set_tile(pApp, iX, iY, pApp->iActiveLayer, tile);
}

static int mapedit_apply_brush_at(mapedit_app_t* pApp, int iX, int iY)
{
	int before;
	int cols;
	int rows;
	int dx;
	int dy;
	if ( pApp == NULL ) return -1;
	if ( !mapedit_need_brush(pApp) ) return -1;
	before = pApp->tMapHistory.tCurrentCommand.iChangeCount;
	cols = (pApp->iActiveTool == MAPEDIT_TOOL_ERASER) ? 1 : (pApp->iBrushW > 0 ? pApp->iBrushW : 1);
	rows = (pApp->iActiveTool == MAPEDIT_TOOL_ERASER) ? 1 : (pApp->iBrushH > 0 ? pApp->iBrushH : 1);
	for ( dy = 0; dy < rows; dy++ ) {
		for ( dx = 0; dx < cols; dx++ ) {
			mapedit_set_tile(pApp, iX + dx, iY + dy, pApp->iActiveLayer, mapedit_brush_tile_at(pApp, dx, dy));
		}
	}
	return pApp->tMapHistory.tCurrentCommand.iChangeCount > before ? 1 : 0;
}

static void mapedit_apply_line(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	int dx;
	int sx;
	int dy;
	int sy;
	int err;
	int e2;
	if ( pApp == NULL ) return;
	dx = abs(x1 - x0);
	sx = x0 < x1 ? 1 : -1;
	dy = -abs(y1 - y0);
	sy = y0 < y1 ? 1 : -1;
	err = dx + dy;
	for (;;) {
		mapedit_apply_tile_at(pApp, x0, y0);
		if ( x0 == x1 && y0 == y1 ) break;
		e2 = err * 2;
		if ( e2 >= dy ) {
			err += dy;
			x0 += sx;
		}
		if ( e2 <= dx ) {
			err += dx;
			y0 += sy;
		}
	}
	mapedit_status(pApp, "线条");
}

static void mapedit_apply_rect(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	int minX;
	int maxX;
	int minY;
	int maxY;
	int x;
	int y;
	if ( pApp == NULL ) return;
	minX = mapedit_min_i(x0, x1);
	maxX = mapedit_max_i(x0, x1);
	minY = mapedit_min_i(y0, y1);
	maxY = mapedit_max_i(y0, y1);
	for ( y = minY; y <= maxY; y++ ) {
		for ( x = minX; x <= maxX; x++ ) {
			mapedit_apply_tile_at(pApp, x, y);
		}
	}
	mapedit_status(pApp, "矩形填充");
}

static void mapedit_apply_circle(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	int minX;
	int maxX;
	int minY;
	int maxY;
	int x;
	int y;
	float cx;
	float cy;
	float rx;
	float ry;
	if ( pApp == NULL ) return;
	minX = mapedit_min_i(x0, x1);
	maxX = mapedit_max_i(x0, x1);
	minY = mapedit_min_i(y0, y1);
	maxY = mapedit_max_i(y0, y1);
	rx = (float)(maxX - minX + 1) * 0.5f;
	ry = (float)(maxY - minY + 1) * 0.5f;
	if ( rx <= 0.0f || ry <= 0.0f ) return;
	cx = (float)minX + rx - 0.5f;
	cy = (float)minY + ry - 0.5f;
	for ( y = minY; y <= maxY; y++ ) {
		for ( x = minX; x <= maxX; x++ ) {
			float nx = ((float)x - cx) / rx;
			float ny = ((float)y - cy) / ry;
			if ( nx * nx + ny * ny <= 1.0f ) mapedit_apply_tile_at(pApp, x, y);
		}
	}
	mapedit_status(pApp, "圆形填充");
}

static int mapedit_apply_bucket(mapedit_app_t* pApp, int iX, int iY)
{
	int layer;
	int target;
	int tile;
	int cellCount;
	int head = 0;
	int tail = 0;
	int* queue;
	unsigned char* seen;
	if ( pApp == NULL || pApp->tMap.pTiles == NULL ) return 0;
	if ( iX < 0 || iY < 0 || iX >= pApp->tMap.iWidth || iY >= pApp->tMap.iHeight ) return 0;
	layer = pApp->iActiveLayer;
	target = mapedit_get_tile(pApp, iX, iY, layer);
	tile = mapedit_paint_tile(pApp);
	if ( tile < 0 ) {
		mapedit_status(pApp, "请先在图块选择中选择图块");
		return 0;
	}
	if ( target == tile ) {
		mapedit_status(pApp, "填充区域没有变化");
		return 0;
	}
	cellCount = pApp->tMap.iWidth * pApp->tMap.iHeight;
	queue = (int*)malloc((size_t)cellCount * sizeof(int));
	seen = (unsigned char*)calloc((size_t)cellCount, 1);
	if ( queue == NULL || seen == NULL ) {
		free(queue);
		free(seen);
		mapedit_status(pApp, "填充队列创建失败");
		return 0;
	}
	queue[tail++] = iY * pApp->tMap.iWidth + iX;
	seen[iY * pApp->tMap.iWidth + iX] = 1;
	while ( head < tail ) {
		int cell = queue[head++];
		int x = cell % pApp->tMap.iWidth;
		int y = cell / pApp->tMap.iWidth;
		int next[4];
		int i;
		if ( mapedit_get_tile(pApp, x, y, layer) != target ) continue;
		mapedit_set_tile(pApp, x, y, layer, tile);
		next[0] = (y > 0) ? cell - pApp->tMap.iWidth : -1;
		next[1] = (x + 1 < pApp->tMap.iWidth) ? cell + 1 : -1;
		next[2] = (y + 1 < pApp->tMap.iHeight) ? cell + pApp->tMap.iWidth : -1;
		next[3] = (x > 0) ? cell - 1 : -1;
		for ( i = 0; i < 4; i++ ) {
			if ( next[i] >= 0 && !seen[next[i]] ) {
				seen[next[i]] = 1;
				queue[tail++] = next[i];
			}
		}
	}
	free(queue);
	free(seen);
	mapedit_status(pApp, "填充");
	return 1;
}

static void mapedit_apply_shape(mapedit_app_t* pApp, int x0, int y0, int x1, int y1)
{
	if ( pApp == NULL ) return;
	if ( pApp->iActiveTool == MAPEDIT_TOOL_LINE ) mapedit_apply_line(pApp, x0, y0, x1, y1);
	else if ( pApp->iActiveTool == MAPEDIT_TOOL_RECT ) mapedit_apply_rect(pApp, x0, y0, x1, y1);
	else if ( pApp->iActiveTool == MAPEDIT_TOOL_CIRCLE ) mapedit_apply_circle(pApp, x0, y0, x1, y1);
}

static void mapedit_history_move(mapedit_app_t* pApp, int redo)
{
	mapedit_history_t* history;
	mapedit_history_cmd_t* cmd;
	int i, tileset;
	char oldTileset[MAPEDIT_FILE_MAX];
	if (!pApp || pApp->bPainting) return;
	tileset = pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_TILESET;
	history = tileset ? &pApp->tTilesetHistory : &pApp->tMapHistory;
	mapeditHistoryBegin(history);
	cmd = mapeditHistoryPeek(history, redo);
	if (!cmd) return;
	mapedit_copy_text(oldTileset, sizeof(oldTileset), pApp->tMap.sTileset);
	mapeditDocumentSwap(cmd, &pApp->tMap, &pApp->tTileset);
	for (i = 0; i < cmd->iChangeCount; ++i) {
		int index = cmd->arrChanges[i].iIndex;
		if (index >= 0 && index < pApp->tMap.iTileCount)
			pApp->tMap.pTiles[index] = redo ? cmd->arrChanges[i].iNewTile : cmd->arrChanges[i].iOldTile;
	}
	mapeditHistoryMoved(history, redo);
	if (tileset) {
		pApp->tTileset.bDirty = mapeditHistoryDirty(history);
		mapedit_tileset_refresh_surfaces(pApp);
	} else {
		pApp->tMap.bDirty = mapeditHistoryDirty(history);
		if (strcmp(oldTileset, pApp->tMap.sTileset) != 0 && !pApp->tTileset.bDirty)
			(void)mapedit_tileset_load(pApp, pApp->tMap.sTileset);
	}
	mapedit_invalidate_map_views(pApp);
	mapedit_invalidate_tileset_views(pApp);
	mapedit_refresh_map_properties(pApp);
	mapedit_refresh_tileset_properties(pApp);
	mapedit_refresh_toolbar_state(pApp);
	mapedit_update_window_title(pApp);
	mapedit_update_status_details(pApp);
	mapedit_status(pApp, redo ? "已重做" : "已撤销");
	xgeRenderRequestAfter(2.0f);
}

static void mapedit_undo(mapedit_app_t* pApp) { mapedit_history_move(pApp, 0); }

static void mapedit_redo(mapedit_app_t* pApp) { mapedit_history_move(pApp, 1); }

static unsigned char mapedit_blob47_normalize_mask(unsigned char iMask)
{
	if ( (iMask & (MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E)) != (MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E) ) iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_NE);
	if ( (iMask & (MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S)) != (MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S) ) iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_SE);
	if ( (iMask & (MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W)) != (MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W) ) iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_SW);
	if ( (iMask & (MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_N)) != (MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_N) ) iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_NW);
	return iMask;
}

static int mapedit_blob47_index_from_mask(unsigned char iMask)
{
	static const unsigned char arrMasks[MAPEDIT_BLOB47_COUNT] = {
		0,
		MAPEDIT_BLOB47_E,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W
	};
	int i;
	iMask = mapedit_blob47_normalize_mask(iMask);
	for ( i = 0; i < MAPEDIT_BLOB47_COUNT; i++ ) {
		if ( arrMasks[i] == iMask ) return i;
	}
	return 0;
}

static unsigned char mapedit_blob47_mask_for_cell(mapedit_app_t* pApp, int iLayer, int x, int y, int iTile)
{
	unsigned char mask = 0;
	if ( pApp == NULL ) return 0;
	if ( mapedit_get_tile(pApp, x, y - 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_N);
	if ( mapedit_get_tile(pApp, x + 1, y, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_E);
	if ( mapedit_get_tile(pApp, x, y + 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_S);
	if ( mapedit_get_tile(pApp, x - 1, y, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_W);
	if ( mapedit_get_tile(pApp, x + 1, y - 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_NE);
	if ( mapedit_get_tile(pApp, x + 1, y + 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_SE);
	if ( mapedit_get_tile(pApp, x - 1, y + 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_SW);
	if ( mapedit_get_tile(pApp, x - 1, y - 1, iLayer) == iTile ) mask = (unsigned char)(mask | MAPEDIT_BLOB47_NW);
	return mapedit_blob47_normalize_mask(mask);
}

static int mapedit_special_is_auto(const char* sType)
{
	return sType != NULL && strstr(sType, "自动") != NULL;
}

static int mapedit_special_has_state(const char* sType)
{
	return sType != NULL && strstr(sType, "状态") != NULL;
}

static int mapedit_special_frame_block_width(const char* sType)
{
	return mapedit_special_is_auto(sType) ? (MAPEDIT_TILE_W * MAPEDIT_BLOB47_COLS) : MAPEDIT_TILE_W;
}

static void mapedit_draw_special_missing(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t dst, int iTile, int bCompact, int bConfigured)
{
	char sText[32];
	if ( pApp == NULL || pDraw == NULL ) return;
	if ( bCompact ) {
		(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, dst, bConfigured ? XUI_COLOR_RGBA(246, 218, 158, 205) : XUI_COLOR_RGBA(204, 232, 250, 205));
		(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, dst, 1.0f, XUI_COLOR_RGBA(72, 150, 208, 230));
		if ( bConfigured ) {
			(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "!", dst, XUI_COLOR_RGBA(168, 82, 42, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
		return;
	}
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, dst, XUI_COLOR_RGBA(246, 218, 158, 205));
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, dst, 1.0f, XUI_COLOR_RGBA(190, 140, 42, 220));
	if ( dst.fW >= 18 && dst.fH >= 14 ) {
		snprintf(sText, sizeof(sText), "%d", iTile);
		(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, dst, XUI_COLOR_RGBA(96, 72, 24, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
}

static int mapedit_draw_tile_rect(mapedit_app_t* pApp, xui_draw_context pDraw, int iTile, xui_rect_t dst)
{
	int special;
	int local;
	xui_surface pSurface;
	xui_surface_desc_t* pDesc;
	xui_rect_t src;
	if ( pApp == NULL || pDraw == NULL || iTile <= 0 ) return XUI_OK;
	special = pApp->tTileset.iSpecialCount;
	if ( iTile <= special ) {
		int idx = iTile - 1;
		if ( idx < 0 || idx >= MAPEDIT_SPECIAL_MAX ) return XUI_OK;
		pSurface = pApp->tTileset.arrSpecial[idx].pSurface;
		pDesc = &pApp->tTileset.arrSpecial[idx].tDesc;
		if ( pSurface == NULL || pDesc->iWidth <= 0 || pDesc->iHeight <= 0 ) {
			mapedit_draw_special_missing(pApp, pDraw, dst, iTile, 1, pApp->tTileset.arrSpecial[idx].sFile[0] != 0 || pApp->tTileset.arrSpecial[idx].sType[0] != 0);
			return XUI_OK;
		}
		src = (xui_rect_t){0, 0, MAPEDIT_TILE_W, MAPEDIT_TILE_H};
		if ( src.fW > pDesc->iWidth ) src.fW = pDesc->iWidth;
		if ( src.fH > pDesc->iHeight ) src.fH = pDesc->iHeight;
		return pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pSurface, src, dst, XUI_COLOR_WHITE, 0);
	}
	if ( pApp->tTileset.pStaticSurface == NULL ) return XUI_OK;
	local = iTile - mapedit_tile_static_start(pApp);
	if ( local < 0 ) return XUI_OK;
	src = (xui_rect_t){
		(local % MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILE_W,
		(local / MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILE_H,
		MAPEDIT_TILE_W,
		MAPEDIT_TILE_H
	};
	if ( src.fX + src.fW > pApp->tTileset.tStaticDesc.iWidth || src.fY + src.fH > pApp->tTileset.tStaticDesc.iHeight ) {
		return XUI_OK;
	}
	return pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->tTileset.pStaticSurface, src, dst, XUI_COLOR_WHITE, 0);
}

static int mapedit_draw_map_tile_rect(mapedit_app_t* pApp, xui_draw_context pDraw, int iTile, int iLayer, int iX, int iY, xui_rect_t dst)
{
	int special;
	int local;
	xui_surface pSurface;
	xui_surface_desc_t* pDesc;
	xui_rect_t src;
	if ( pApp == NULL || pDraw == NULL || iTile <= 0 ) return XUI_OK;
	special = pApp->tTileset.iSpecialCount;
	if ( iTile <= special ) {
		int idx = iTile - 1;
		int state = pApp->tMap.iState;
		int autoTile;
		int stateBlockH;
		int availableStates;
		int frameBlockW;
		int frameCount;
		int frameIndex;
		if ( idx < 0 || idx >= MAPEDIT_SPECIAL_MAX ) return XUI_OK;
		pSurface = pApp->tTileset.arrSpecial[idx].pSurface;
		pDesc = &pApp->tTileset.arrSpecial[idx].tDesc;
		if ( pSurface == NULL || pDesc->iWidth <= 0 || pDesc->iHeight <= 0 ) {
			mapedit_draw_special_missing(pApp, pDraw, dst, iTile, 0, 1);
			return XUI_OK;
		}
		autoTile = mapedit_special_is_auto(pApp->tTileset.arrSpecial[idx].sType);
		src = (xui_rect_t){0, 0, MAPEDIT_TILE_W, MAPEDIT_TILE_H};
		if ( autoTile && pDesc->iWidth >= MAPEDIT_TILE_W * MAPEDIT_BLOB47_COLS && pDesc->iHeight >= MAPEDIT_TILE_H * MAPEDIT_BLOB47_ROWS ) {
			int blob = mapedit_blob47_index_from_mask(mapedit_blob47_mask_for_cell(pApp, iLayer, iX, iY, iTile));
			src.fX += (blob % MAPEDIT_BLOB47_COLS) * MAPEDIT_TILE_W;
			src.fY = (blob / MAPEDIT_BLOB47_COLS) * MAPEDIT_TILE_H;
		}
		if ( mapedit_special_has_state(pApp->tTileset.arrSpecial[idx].sType) ) {
			if ( state < 0 ) state = 0;
			stateBlockH = autoTile ? (MAPEDIT_TILE_H * MAPEDIT_BLOB47_ROWS) : MAPEDIT_TILE_H;
			availableStates = stateBlockH > 0 ? (pDesc->iHeight / stateBlockH) : 0;
			if ( availableStates > 0 ) {
				if ( state >= availableStates ) state = availableStates - 1;
				src.fY += state * stateBlockH;
			}
		}
		if ( pApp->bPreview ) {
			frameBlockW = mapedit_special_frame_block_width(pApp->tTileset.arrSpecial[idx].sType);
			frameCount = frameBlockW > 0 ? (pDesc->iWidth / frameBlockW) : 0;
			if ( frameCount > 1 ) {
				frameIndex = pApp->iPreviewAnimFrame % frameCount;
				if ( frameIndex < 0 ) frameIndex = 0;
				src.fX += frameIndex * frameBlockW;
			}
		}
		if ( src.fX >= pDesc->iWidth || src.fY >= pDesc->iHeight ) return XUI_OK;
		if ( src.fX + src.fW > pDesc->iWidth ) src.fW = pDesc->iWidth - src.fX;
		if ( src.fY + src.fH > pDesc->iHeight ) src.fH = pDesc->iHeight - src.fY;
		if ( src.fW <= 0 || src.fH <= 0 ) {
			return XUI_OK;
		}
		return pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pSurface, src, dst, XUI_COLOR_WHITE, 0);
	}
	if ( pApp->tTileset.pStaticSurface == NULL ) return XUI_OK;
	local = iTile - mapedit_tile_static_start(pApp);
	if ( local < 0 ) return XUI_OK;
	src = (xui_rect_t){
		(local % MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILE_W,
		(local / MAPEDIT_TILES_PER_ROW) * MAPEDIT_TILE_H,
		MAPEDIT_TILE_W,
		MAPEDIT_TILE_H
	};
	if ( src.fX + src.fW > pApp->tTileset.tStaticDesc.iWidth || src.fY + src.fH > pApp->tTileset.tStaticDesc.iHeight ) {
		return XUI_OK;
	}
	return pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->tTileset.pStaticSurface, src, dst, XUI_COLOR_WHITE, 0);
}



static void mapedit_draw_tag_marker(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell, const char* sValue)
{
	xui_rect_t dot;
	xui_rect_t textRect;
	int dotSize;
	if ( pApp == NULL || pDraw == NULL ) return;
	if ( sValue == NULL || sValue[0] == 0 ) return;
	dotSize = ((cell.fW < cell.fH ? cell.fW : cell.fH) * 35 + 50) / 100;
	if ( dotSize < 5 ) dotSize = 5;
	if ( dotSize > 12 ) dotSize = 12;
	dot = (xui_rect_t){cell.fX + 2, cell.fY + 2, dotSize, dotSize};
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, dot, XUI_COLOR_RGBA(30, 135, 210, 210));
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, dot, 1.0f, XUI_COLOR_RGBA(245, 252, 255, 230));
	if ( cell.fW >= 28 && cell.fH >= 20 ) {
		textRect = (xui_rect_t){cell.fX + 2, cell.fY + cell.fH - 15, cell.fW - 4, 13};
		(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, textRect, XUI_COLOR_RGBA(245, 252, 255, 190));
		(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sValue, textRect,
			XUI_COLOR_RGBA(31, 75, 112, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
}

static void mapedit_draw_zero_tile_cell(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell)
{
	if ( pApp == NULL || pDraw == NULL ) return;
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, cell, XUI_COLOR_RGBA(242, 248, 252, 235));
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, cell, 1.0f, XUI_COLOR_RGBA(98, 158, 108, 220));
	(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "0", cell,
		XUI_COLOR_RGBA(48, 112, 62, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static void mapedit_draw_special_placeholder_cell(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell)
{
	if ( pApp == NULL || pDraw == NULL ) return;
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, cell, XUI_COLOR_RGBA(204, 232, 250, 168));
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, cell, 1.0f, XUI_COLOR_RGBA(72, 150, 208, 210));
}

static void mapedit_draw_tilegrid_lines(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t viewport, float scrollX, float scrollY, int iCellW, int iCellH, int iCols, int iRows, uint32_t iGridColor)
{
	int scrollPixelX;
	int scrollPixelY;
	int originX;
	int originY;
	int contentW;
	int contentH;
	int viewRight;
	int viewBottom;
	int y0;
	int y1;
	int x0;
	int x1;
	int firstCol;
	int lastCol;
	int firstRow;
	int lastRow;
	int i;
	if ( pApp == NULL || pDraw == NULL || iCellW <= 0 || iCellH <= 0 || iCols <= 0 || iRows <= 0 ) return;
	scrollPixelX = (int)floorf(scrollX);
	scrollPixelY = (int)floorf(scrollY);
	originX = viewport.fX - scrollPixelX;
	originY = viewport.fY - scrollPixelY;
	contentW = iCols * iCellW;
	contentH = iRows * iCellH;
	viewRight = viewport.fX + viewport.fW;
	viewBottom = viewport.fY + viewport.fH;
	y0 = (viewport.fY > originY) ? viewport.fY : originY;
	y1 = (viewBottom < originY + contentH) ? viewBottom : originY + contentH;
	x0 = (viewport.fX > originX) ? viewport.fX : originX;
	x1 = (viewRight < originX + contentW) ? viewRight : originX + contentW;
	if ( y1 > y0 ) {
		firstCol = mapedit_max_i(0, scrollPixelX / iCellW);
		lastCol = mapedit_min_i(iCols, (scrollPixelX + viewport.fW + iCellW - 1) / iCellW + 1);
		for ( i = firstCol; i <= lastCol; i++ ) {
			int x = originX + i * iCellW;
			if ( x < viewport.fX - 1 || x > viewRight ) continue;
			(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, (xui_rect_t){x, y0, 1, y1 - y0}, iGridColor);
		}
	}
	if ( x1 > x0 ) {
		firstRow = mapedit_max_i(0, scrollPixelY / iCellH);
		lastRow = mapedit_min_i(iRows, (scrollPixelY + viewport.fH + iCellH - 1) / iCellH + 1);
		for ( i = firstRow; i <= lastRow; i++ ) {
			int y = originY + i * iCellH;
			if ( y < viewport.fY - 1 || y > viewBottom ) continue;
			(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, (xui_rect_t){x0, y, x1 - x0, 1}, iGridColor);
		}
	}
}

static void mapedit_draw_tilegrid_border(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t viewport, float scrollX, float scrollY, int iCellW, int iCellH, int iCols, int iRows)
{
	if ( pApp == NULL || pDraw == NULL || iCellW <= 0 || iCellH <= 0 || iCols <= 0 || iRows <= 0 ) return;
	(void)viewport;
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw,
		(xui_rect_t){viewport.fX - (int)floorf(scrollX), viewport.fY - (int)floorf(scrollY), iCols * iCellW, iRows * iCellH},
		1.0f, MAPEDIT_TILEGRID_BORDER);
}

static void mapedit_draw_actor_overlay_marker(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell)
{
	int shadow;
	if ( pApp == NULL || pDraw == NULL ) return;
	shadow = (cell.fW < 20 || cell.fH < 20) ? 0 : 1;
	if ( shadow > 0 ) {
		(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "≈",
			(xui_rect_t){cell.fX + shadow, cell.fY + shadow, cell.fW, cell.fH},
			XUI_COLOR_RGBA(248, 252, 255, 210), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "≈", cell,
		XUI_COLOR_RGBA(28, 118, 78, 245), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static void mapedit_map_widget_scroll(mapedit_app_t* pApp, xui_widget pWidget, float* pScrollX, float* pScrollY)
{
	float sx = 0.0f;
	float sy = 0.0f;
	if ( pApp != NULL ) {
		if ( pWidget == pApp->pMapTagsCanvas ) {
			sx = pApp->fMapTagsScrollX;
			sy = pApp->fMapTagsScrollY;
		} else {
			sx = pApp->fMapScrollX;
			sy = pApp->fMapScrollY;
		}
	}
	if ( pScrollX != NULL ) *pScrollX = sx;
	if ( pScrollY != NULL ) *pScrollY = sy;
}

static int mapedit_map_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	float scrollX;
	float scrollY;
	int scrollPixelX;
	int scrollPixelY;
	int firstX;
	int firstY;
	int lastX;
	int lastY;
	int x;
	int y;
	int l;
	int visibleCells = 0;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(238, 243, 248, 255));
	if ( pApp->tMap.pTiles == NULL ) {
		return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "未加载地图", r, XUI_COLOR_RGBA(64, 80, 96, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE);
	}
	mapedit_map_widget_scroll(pApp, pWidget, &scrollX, &scrollY);
	scrollPixelX = (int)floorf(scrollX);
	scrollPixelY = (int)floorf(scrollY);
	firstX = mapedit_max_i(0, scrollPixelX / mapedit_view_cell(pApp, pWidget, 0));
	firstY = mapedit_max_i(0, scrollPixelY / mapedit_view_cell(pApp, pWidget, 1));
	lastX = mapedit_min_i(pApp->tMap.iWidth - 1, (scrollPixelX + r.fW + mapedit_view_cell(pApp, pWidget, 0) - 1) / mapedit_view_cell(pApp, pWidget, 0));
	lastY = mapedit_min_i(pApp->tMap.iHeight - 1, (scrollPixelY + r.fH + mapedit_view_cell(pApp, pWidget, 1) - 1) / mapedit_view_cell(pApp, pWidget, 1));
	for ( y = firstY; y <= lastY; y++ ) {
		for ( x = firstX; x <= lastX; x++ ) {
			int dx = r.fX + x * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX;
			int dy = r.fY + y * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY;
			visibleCells++;
			for ( l = 0; l < pApp->tMap.iLayers; l++ ) {
				int idx = (l * pApp->tMap.iHeight + y) * pApp->tMap.iWidth + x;
				int tile = pApp->tMap.pTiles[idx];
				if ( tile > 0 ) (void)mapedit_draw_map_tile_rect(pApp, pDraw, tile, l, x, y, (xui_rect_t){dx, dy, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1)});
			}
			if ( pApp->bGrid && !(pWidget == pApp->pMapCanvas && pApp->bPreview) ) {
				(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, (xui_rect_t){dx, dy, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1)}, 1.0f, XUI_COLOR_RGBA(46, 72, 98, 56));
			}
			if ( pWidget == pApp->pMapTagsCanvas ) {
				mapedit_custom_channel_def_t* pDef = mapedit_custom_channel(pApp, pApp->iMapTagChannel);
				char sValue[MAPEDIT_CUSTOM_VALUE_MAX];
				if ( pDef != NULL ) {
					if ( !mapedit_custom_get_value(pApp->tMap.pCellDataRaw, pDef->sId, MAP_KEY_CELL, y * pApp->tMap.iWidth + x, sValue, sizeof(sValue)) ) {
						mapedit_map_cell_custom_fallback(pApp, pDef, x, y, sValue, sizeof(sValue));
					}
				}
				if ( pDef != NULL && sValue[0] != 0 && strcmp(sValue, pDef->sDefaultValue) != 0 ) {
					mapedit_draw_tag_marker(pApp, pDraw, (xui_rect_t){dx, dy, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1)}, sValue);
				}
			}
		}
	}
	if ( pWidget == pApp->pMapCanvas && !pApp->bPreview && pApp->iMapHoverX >= 0 && pApp->iMapHoverY >= 0 &&
	     pApp->iMapHoverX < pApp->tMap.iWidth && pApp->iMapHoverY < pApp->tMap.iHeight ) {
		xui_rect_t hover = {
			r.fX + pApp->iMapHoverX * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
			r.fY + pApp->iMapHoverY * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
			mapedit_view_cell(pApp, pWidget, 0) + 1,
			mapedit_view_cell(pApp, pWidget, 1) + 1
		};
		(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, hover, 1.0f, XUI_COLOR_RGBA(42, 140, 210, 180));
	}
	if ( pWidget == pApp->pMapTagsCanvas ) {
		if ( pApp->iMapTagsHoverX >= 0 && pApp->iMapTagsHoverY >= 0 &&
		     pApp->iMapTagsHoverX < pApp->tMap.iWidth && pApp->iMapTagsHoverY < pApp->tMap.iHeight ) {
			xui_rect_t hover = {
				r.fX + pApp->iMapTagsHoverX * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
				r.fY + pApp->iMapTagsHoverY * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
				mapedit_view_cell(pApp, pWidget, 0) + 1,
				mapedit_view_cell(pApp, pWidget, 1) + 1
			};
			(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, hover, 1.0f, XUI_COLOR_RGBA(42, 140, 210, 180));
		}
		if ( pApp->iMapTagsSelectedCell >= 0 && pApp->tMap.iWidth > 0 ) {
			int sx = pApp->iMapTagsSelectedCell % pApp->tMap.iWidth;
			int sy = pApp->iMapTagsSelectedCell / pApp->tMap.iWidth;
			if ( sx >= 0 && sy >= 0 && sx < pApp->tMap.iWidth && sy < pApp->tMap.iHeight ) {
				xui_rect_t sel = {
					r.fX + sx * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
					r.fY + sy * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
					mapedit_view_cell(pApp, pWidget, 0) + 1,
					mapedit_view_cell(pApp, pWidget, 1) + 1
				};
				(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, sel, 1.0f, XUI_COLOR_RGBA(28, 122, 196, 230));
			}
		}
	}
	if ( pWidget == pApp->pMapCanvas && !pApp->bPreview && pApp->bPainting &&
	     (pApp->iActiveTool == MAPEDIT_TOOL_LINE || pApp->iActiveTool == MAPEDIT_TOOL_RECT || pApp->iActiveTool == MAPEDIT_TOOL_CIRCLE) &&
	     pApp->iDragStartX >= 0 && pApp->iDragStartY >= 0 && pApp->iDragCurrentX >= 0 && pApp->iDragCurrentY >= 0 ) {
		int minX = mapedit_min_i(pApp->iDragStartX, pApp->iDragCurrentX);
		int maxX = mapedit_max_i(pApp->iDragStartX, pApp->iDragCurrentX);
		int minY = mapedit_min_i(pApp->iDragStartY, pApp->iDragCurrentY);
		int maxY = mapedit_max_i(pApp->iDragStartY, pApp->iDragCurrentY);
		xui_rect_t sel = {
			r.fX + minX * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
			r.fY + minY * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
			(maxX - minX + 1) * mapedit_view_cell(pApp, pWidget, 0) + 1,
			(maxY - minY + 1) * mapedit_view_cell(pApp, pWidget, 1) + 1
		};
		(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, sel, 1.0f, XUI_COLOR_RGBA(28, 122, 196, 230));
	}
	pApp->iLastVisibleCells = visibleCells;
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, r, 1.0f, XUI_COLOR_RGBA(120, 150, 180, 255));
	pApp->bRenderOK = 1;
	return XUI_OK;
}

static int mapedit_tile_select_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	int rowCount;
	int visibleRows;
	int firstRow;
	int lastRow;
	int row;
	int col;
	int special;
	int staticStart;
	int selCol;
	int selRow;
	int scrollPixelX;
	int scrollPixelY;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, MAPEDIT_TILEGRID_BG);
	rowCount = mapedit_tile_select_row_count(pApp);
	scrollPixelX = (int)floorf(pApp->fTileScrollX);
	scrollPixelY = (int)floorf(pApp->fTileScrollY);
	visibleRows = r.fH / mapedit_view_cell(pApp, pWidget, 1) + 2;
	firstRow = mapedit_max_i(0, scrollPixelY / mapedit_view_cell(pApp, pWidget, 1));
	lastRow = mapedit_min_i(rowCount, firstRow + visibleRows);
	special = pApp->tTileset.iSpecialCount;
	staticStart = mapedit_tile_static_start(pApp);
	selCol = (pApp->iSelectedTile >= 0) ? (pApp->iSelectedTile % MAPEDIT_TILES_PER_ROW) : -1;
	selRow = (pApp->iSelectedTile >= 0) ? (pApp->iSelectedTile / MAPEDIT_TILES_PER_ROW) : -1;
	if ( pApp->tTileset.pStaticSurface == NULL ) {
		int staticRow = staticStart / MAPEDIT_TILES_PER_ROW;
		xui_rect_t staticRect = {
			r.fX - scrollPixelX,
			r.fY + staticRow * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
			MAPEDIT_TILES_PER_ROW * mapedit_view_cell(pApp, pWidget, 0),
			mapedit_tile_static_rows(pApp) * mapedit_view_cell(pApp, pWidget, 1)
		};
		(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, staticRect, XUI_COLOR_RGBA(235, 244, 250, 255));
		(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "当前地图未设置可用静态图块集", staticRect, XUI_COLOR_RGBA(104, 128, 148, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	for ( row = firstRow; row < lastRow; row++ ) {
		for ( col = 0; col < MAPEDIT_TILES_PER_ROW; col++ ) {
		int tile = row * MAPEDIT_TILES_PER_ROW + col;
		int dx = r.fX + col * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX;
		int dy = r.fY + row * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY;
		xui_rect_t cell = (xui_rect_t){dx, dy, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1)};
		if ( tile == 0 ) {
			(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, cell, XUI_COLOR_RGBA(242, 248, 252, 235));
			(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "0", cell, XUI_COLOR_RGBA(48, 112, 62, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		} else if ( mapedit_tile_select_tile_selectable(pApp, tile) ) {
			(void)mapedit_draw_tile_rect(pApp, pDraw, tile, cell);
		} else if ( tile > special && tile < staticStart ) {
			(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, cell, XUI_COLOR_RGBA(224, 232, 238, 180));
		}
		}
	}
	mapedit_draw_tilegrid_lines(pApp, pDraw, r, pApp->fTileScrollX, pApp->fTileScrollY, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1), MAPEDIT_TILES_PER_ROW, rowCount, MAPEDIT_TILEGRID_GRID);
	if ( pApp->iTileSelectHoverCol >= 0 && pApp->iTileSelectHoverRow >= 0 &&
	     pApp->iTileSelectHoverCol < MAPEDIT_TILES_PER_ROW && pApp->iTileSelectHoverRow < rowCount ) {
		xui_rect_t hover = {
			r.fX + pApp->iTileSelectHoverCol * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
			r.fY + pApp->iTileSelectHoverRow * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
			mapedit_view_cell(pApp, pWidget, 0) + 1,
			mapedit_view_cell(pApp, pWidget, 1) + 1
		};
		if ( hover.fX + hover.fW >= r.fX && hover.fY + hover.fH >= r.fY && hover.fX <= r.fX + r.fW && hover.fY <= r.fY + r.fH ) {
			(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, hover, 1.0f, XUI_COLOR_RGBA(42, 140, 210, 180));
		}
	}
	if ( selCol >= 0 && selRow >= 0 ) {
		xui_rect_t sel = {
			r.fX + selCol * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX,
			r.fY + selRow * mapedit_view_cell(pApp, pWidget, 1) - scrollPixelY,
			mapedit_max_i(1, pApp->iBrushW) * mapedit_view_cell(pApp, pWidget, 0) + 1,
			mapedit_max_i(1, pApp->iBrushH) * mapedit_view_cell(pApp, pWidget, 1) + 1
		};
		if ( sel.fX + sel.fW >= r.fX && sel.fY + sel.fH >= r.fY && sel.fX <= r.fX + r.fW && sel.fY <= r.fY + r.fH ) {
			(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, sel, 1.0f, XUI_COLOR_RGBA(28, 122, 196, 230));
		}
	}
	mapedit_draw_tilegrid_border(pApp, pDraw, r, pApp->fTileScrollX, pApp->fTileScrollY, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 1), MAPEDIT_TILES_PER_ROW, rowCount);
	return XUI_OK;
}

static int mapedit_simple_panel_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	const char* sText = (const char*)pUser;
	xui_proxy_t proxy;
	xui_rect_t r;
	(void)iStateId;
	if ( pWidget == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	proxy = xuiProxyXge();
	r = xuiWidgetGetContentRect(pWidget);
	(void)proxy.drawRectFill(&proxy, pDraw, r, XUI_COLOR_RGBA(250, 253, 255, 255));
	return proxy.drawText(&proxy, pDraw, xuiGetDefaultFont(xuiWidgetGetContext(pWidget)), sText != NULL ? sText : "", (xui_rect_t){r.fX + 12, r.fY + 12, r.fW - 24, r.fH - 24}, XUI_COLOR_RGBA(54, 74, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
}

static void mapedit_fit_dimensions(int iSourceW, int iSourceH, int iMaxW, int iMaxH, int* pWidth, int* pHeight)
{
	int w = iSourceW;
	int h = iSourceH;
	if ( iMaxW < 1 ) iMaxW = 1;
	if ( iMaxH < 1 ) iMaxH = 1;
	if ( w > iMaxW || h > iMaxH ) {
		if ( (long long)w * (long long)iMaxH > (long long)h * (long long)iMaxW ) {
			h = mapedit_max_i(1, (int)((long long)h * (long long)iMaxW / (long long)w));
			w = iMaxW;
		} else {
			w = mapedit_max_i(1, (int)((long long)w * (long long)iMaxH / (long long)h));
			h = iMaxH;
		}
	}
	if ( pWidth != NULL ) *pWidth = w;
	if ( pHeight != NULL ) *pHeight = h;
}

static xui_vec2_t mapedit_material_tooltip_measure(xui_context pContext, xui_widget pOwner, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int w;
	int h;
	(void)pContext;
	(void)pOwner;
	if ( pApp == NULL || pApp->pMaterialTooltipSurface == NULL ||
	     pApp->tMaterialTooltipDesc.iWidth <= 0 || pApp->tMaterialTooltipDesc.iHeight <= 0 ) {
		return (xui_vec2_t){160.0f, 48.0f};
	}
	mapedit_fit_dimensions(pApp->tMaterialTooltipDesc.iWidth, pApp->tMaterialTooltipDesc.iHeight, 320, 240, &w, &h);
	return (xui_vec2_t){(float)(w + 16), (float)(h + 40)};
}

static int mapedit_material_tooltip_paint(xui_context pContext, xui_widget pOwner, xui_draw_context pDraw, xui_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t src;
	xui_rect_t dst;
	xui_rect_t textRect;
	char name[MAPEDIT_NAME_MAX];
	int drawW;
	int drawH;
	(void)pContext;
	(void)pOwner;
	if ( pApp == NULL || pDraw == NULL || pApp->pMaterialTooltipSurface == NULL ||
	     pApp->tMaterialTooltipDesc.iWidth <= 0 || pApp->tMaterialTooltipDesc.iHeight <= 0 ) {
		return XUI_OK;
	}
	src = (xui_rect_t){0, 0, pApp->tMaterialTooltipDesc.iWidth, pApp->tMaterialTooltipDesc.iHeight};
	mapedit_fit_dimensions(src.fW, src.fH, 320, 240, &drawW, &drawH);
	dst = (xui_rect_t){tRect.fX + (tRect.fW - drawW) / 2, tRect.fY + 8, drawW, drawH};
	(void)pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pMaterialTooltipSurface, src, dst, XUI_COLOR_WHITE, 0);
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, dst, 1.0f, XUI_COLOR_RGBA(128, 172, 204, 255));
	name[0] = 0;
	if ( pApp->iMaterialTooltipIndex >= 0 ) mapedit_material_display_name(pApp, pApp->iMaterialTooltipIndex, name, sizeof(name));
	textRect = (xui_rect_t){tRect.fX + 8, dst.fY + dst.fH + 6, tRect.fW - 16, 20};
	return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, name[0] ? name : "", textRect,
		XUI_COLOR_RGBA(31, 75, 112, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static int mapedit_material_tooltip_resolve(xui_context pContext, xui_widget pWidget, xui_tooltip_desc_t* pDesc, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t wr;
	xui_rect_t viewport;
	xui_rect_t item;
	int hover;
	(void)pContext;
	if ( pApp == NULL || pWidget == NULL || pDesc == NULL || pWidget != pApp->pMaterialListView ) return 0;
	hover = xuiListViewGetHoverIndex(pWidget);
	if ( hover < 0 || hover >= pApp->tMaterialCategoryFiles.iCount ) return 0;
	if ( !mapedit_material_tooltip_ensure(pApp, hover) ) return 0;
	viewport = xuiListViewGetViewportRect(pWidget);
	item = xuiListViewGetItemRect(pWidget, hover);
	if ( item.fY + item.fH <= viewport.fY || item.fY >= viewport.fY + viewport.fH ) return 0;
	if ( item.fY < viewport.fY ) {
		item.fH -= viewport.fY - item.fY;
		item.fY = viewport.fY;
	}
	if ( item.fY + item.fH > viewport.fY + viewport.fH ) {
		item.fH = viewport.fY + viewport.fH - item.fY;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iType = XUI_TOOLTIP_CUSTOM;
	pDesc->iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	pDesc->fOffsetX = 6.0f;
	pDesc->fOffsetY = 0.0f;
	pDesc->fDelay = 0.5f;
	pDesc->bFollowCursor = 0;
	pDesc->onMeasure = mapedit_material_tooltip_measure;
	pDesc->onPaint = mapedit_material_tooltip_paint;
	pDesc->pUser = pApp;
	wr = xuiWidgetGetWorldRect(pWidget);
	pDesc->bCustomAnchorRect = 1;
	pDesc->tAnchorRect = (xui_rect_t){wr.fX + item.fX, wr.fY + item.fY, item.fW, item.fH};
	return 1;
}

static int mapedit_material_view_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	xui_rect_t src;
	xui_rect_t dst;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(250, 253, 255, 255));
	if ( pApp->pMaterialViewSurface == NULL || pApp->tMaterialViewDesc.iWidth <= 0 || pApp->tMaterialViewDesc.iHeight <= 0 ) {
		return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "选择素材预览", r, XUI_COLOR_RGBA(64, 80, 96, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	src = (xui_rect_t){0, 0, pApp->tMaterialViewDesc.iWidth, pApp->tMaterialViewDesc.iHeight};
	dst = (xui_rect_t){r.fX, r.fY, src.fW, src.fH};
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, (xui_rect_t){dst.fX - 1, dst.fY - 1, dst.fW + 2, dst.fH + 2}, 1.0f, XUI_COLOR_RGBA(152, 174, 198, 255));
	return pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pMaterialViewSurface, src, dst, XUI_COLOR_WHITE, 0);
}

static void mapedit_material_edit_draw_grid(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t r, int iCols, int iRows, uint32_t iColor)
{
	int x;
	int y;
	if ( pApp == NULL || pDraw == NULL || iCols <= 0 || iRows <= 0 ) return;
	for ( y = 0; y <= iRows; y++ ) {
		float py = (float)(r.fY + y * MAPEDIT_TILE_H);
		(void)pApp->tProxy.drawLine(&pApp->tProxy, pDraw, (float)r.fX, py, (float)(r.fX + iCols * MAPEDIT_TILE_W), py, 1.0f, iColor);
	}
	for ( x = 0; x <= iCols; x++ ) {
		float px = (float)(r.fX + x * MAPEDIT_TILE_W);
		(void)pApp->tProxy.drawLine(&pApp->tProxy, pDraw, px, (float)r.fY, px, (float)(r.fY + iRows * MAPEDIT_TILE_H), 1.0f, iColor);
	}
}

static int mapedit_material_edit_output_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	xui_rect_t src;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(245, 248, 252, 255));
	if ( pApp->pMaterialEditOutputSurface != NULL && pApp->tMaterialEditOutputDesc.iWidth > 0 && pApp->tMaterialEditOutputDesc.iHeight > 0 ) {
		src = (xui_rect_t){0, 0, pApp->tMaterialEditOutputDesc.iWidth, pApp->tMaterialEditOutputDesc.iHeight};
		(void)pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pMaterialEditOutputSurface, src, r, XUI_COLOR_WHITE, 0);
	}
	mapedit_material_edit_draw_grid(pApp, pDraw, r, pApp->iMaterialEditOutputCols, pApp->iMaterialEditOutputRows, XUI_COLOR_RGBA(56, 78, 104, 70));
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, r, 1.0f, XUI_COLOR_RGBA(104, 132, 160, 180));
	return XUI_OK;
}

static int mapedit_material_edit_source_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	xui_rect_t src;
	xui_rect_t sel;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(250, 253, 255, 255));
	if ( pApp->pMaterialEditSourceSurface == NULL || pApp->tMaterialEditSourceDesc.iWidth <= 0 || pApp->tMaterialEditSourceDesc.iHeight <= 0 ) {
		return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "打开图片后选择图块", r, XUI_COLOR_RGBA(64, 80, 96, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	src = (xui_rect_t){0, 0, pApp->tMaterialEditSourceDesc.iWidth, pApp->tMaterialEditSourceDesc.iHeight};
	(void)pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pMaterialEditSourceSurface, src, r, XUI_COLOR_WHITE, 0);
	mapedit_material_edit_draw_grid(pApp, pDraw, r, pApp->iMaterialEditSourceCols, pApp->iMaterialEditSourceRows, XUI_COLOR_RGBA(56, 78, 104, 70));
	if ( pApp->iMaterialEditSelCols > 0 && pApp->iMaterialEditSelRows > 0 ) {
		sel = (xui_rect_t){
			r.fX + pApp->iMaterialEditSelCol * MAPEDIT_TILE_W,
			r.fY + pApp->iMaterialEditSelRow * MAPEDIT_TILE_H,
			pApp->iMaterialEditSelCols * MAPEDIT_TILE_W,
			pApp->iMaterialEditSelRows * MAPEDIT_TILE_H
		};
		(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, sel, XUI_COLOR_RGBA(32, 126, 212, 64));
		(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, sel, 2.0f, XUI_COLOR_RGBA(32, 126, 212, 230));
	}
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, r, 1.0f, XUI_COLOR_RGBA(104, 132, 160, 180));
	return XUI_OK;
}

static unsigned char mapedit_tileset_passage_bit_for_region(int iRegionCol, int iRegionRow)
{
	if ( iRegionRow == 0 && iRegionCol == 0 ) return 0x01;
	if ( iRegionRow == 0 && iRegionCol == 1 ) return 0x02;
	if ( iRegionRow == 0 && iRegionCol == 2 ) return 0x04;
	if ( iRegionRow == 1 && iRegionCol == 0 ) return 0x08;
	if ( iRegionRow == 1 && iRegionCol == 2 ) return 0x10;
	if ( iRegionRow == 2 && iRegionCol == 0 ) return 0x20;
	if ( iRegionRow == 2 && iRegionCol == 1 ) return 0x40;
	if ( iRegionRow == 2 && iRegionCol == 2 ) return 0x80;
	return 0;
}

static float mapedit_min_f(float a, float b)
{
	return a < b ? a : b;
}

static void mapedit_draw_forbidden_triangle(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell, int iRegionCol, int iRegionRow)
{
	float fSize;
	float fPad;
	float fTri;
	float fX;
	float fY;
	float fW;
	float fH;
	xui_vec2_t a;
	xui_vec2_t b;
	xui_vec2_t c;
	if ( pApp == NULL || pDraw == NULL || pApp->tProxy.drawTriangleStroke == NULL ) return;
	fX = (float)cell.fX;
	fY = (float)cell.fY;
	fW = (float)cell.fW;
	fH = (float)cell.fH;
	fSize = mapedit_min_f(fW, fH);
	fPad = fSize <= 16.0f ? 2.0f : 3.0f;
	fTri = fSize * 0.36f;
	if ( fTri < 5.0f ) fTri = 5.0f;
	if ( iRegionRow == 0 && iRegionCol == 0 ) {
		a = (xui_vec2_t){fX + fPad, fY + fPad};
		b = (xui_vec2_t){fX + fPad + fTri, fY + fPad};
		c = (xui_vec2_t){fX + fPad, fY + fPad + fTri};
	} else if ( iRegionRow == 0 && iRegionCol == 1 ) {
		a = (xui_vec2_t){fX + fW * 0.5f, fY + fPad + fTri};
		b = (xui_vec2_t){fX + fW * 0.5f - fTri * 0.5f, fY + fPad};
		c = (xui_vec2_t){fX + fW * 0.5f + fTri * 0.5f, fY + fPad};
	} else if ( iRegionRow == 0 && iRegionCol == 2 ) {
		a = (xui_vec2_t){fX + fW - fPad, fY + fPad};
		b = (xui_vec2_t){fX + fW - fPad - fTri, fY + fPad};
		c = (xui_vec2_t){fX + fW - fPad, fY + fPad + fTri};
	} else if ( iRegionRow == 1 && iRegionCol == 0 ) {
		a = (xui_vec2_t){fX + fPad + fTri, fY + fH * 0.5f};
		b = (xui_vec2_t){fX + fPad, fY + fH * 0.5f - fTri * 0.5f};
		c = (xui_vec2_t){fX + fPad, fY + fH * 0.5f + fTri * 0.5f};
	} else if ( iRegionRow == 1 && iRegionCol == 2 ) {
		a = (xui_vec2_t){fX + fW - fPad - fTri, fY + fH * 0.5f};
		b = (xui_vec2_t){fX + fW - fPad, fY + fH * 0.5f - fTri * 0.5f};
		c = (xui_vec2_t){fX + fW - fPad, fY + fH * 0.5f + fTri * 0.5f};
	} else if ( iRegionRow == 2 && iRegionCol == 0 ) {
		a = (xui_vec2_t){fX + fPad, fY + fH - fPad};
		b = (xui_vec2_t){fX + fPad + fTri, fY + fH - fPad};
		c = (xui_vec2_t){fX + fPad, fY + fH - fPad - fTri};
	} else if ( iRegionRow == 2 && iRegionCol == 1 ) {
		a = (xui_vec2_t){fX + fW * 0.5f, fY + fH - fPad - fTri};
		b = (xui_vec2_t){fX + fW * 0.5f - fTri * 0.5f, fY + fH - fPad};
		c = (xui_vec2_t){fX + fW * 0.5f + fTri * 0.5f, fY + fH - fPad};
	} else if ( iRegionRow == 2 && iRegionCol == 2 ) {
		a = (xui_vec2_t){fX + fW - fPad, fY + fH - fPad};
		b = (xui_vec2_t){fX + fW - fPad - fTri, fY + fH - fPad};
		c = (xui_vec2_t){fX + fW - fPad, fY + fH - fPad - fTri};
	} else {
		return;
	}
	(void)pApp->tProxy.drawTriangleStroke(&pApp->tProxy, pDraw, a, b, c, 2.0f, XUI_COLOR_RGBA(232, 26, 36, 225));
}

static void mapedit_draw_passage_overlay_rect(mapedit_app_t* pApp, xui_draw_context pDraw, xui_rect_t cell, unsigned char value)
{
	int rx;
	int ry;
	if ( pApp == NULL || pDraw == NULL ) return;
	if ( value == 255 ) {
		float fSize = mapedit_min_f((float)cell.fW, (float)cell.fH);
		float centerX = (float)cell.fX + (float)cell.fW * 0.5f;
		float centerY = (float)cell.fY + (float)cell.fH * 0.5f;
		float r = fSize * 0.12f;
		if ( r < 2.0f ) r = 2.0f;
		if ( r > 4.0f ) r = 4.0f;
		if ( pApp->tProxy.drawCircleFill != NULL ) {
			(void)pApp->tProxy.drawCircleFill(&pApp->tProxy, pDraw, centerX, centerY, r + 1.0f, XUI_COLOR_RGBA(245, 252, 246, 210));
			(void)pApp->tProxy.drawCircleFill(&pApp->tProxy, pDraw, centerX, centerY, r, XUI_COLOR_RGBA(26, 166, 62, 230));
		}
		return;
	}
	if ( value == 0 ) {
		(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, (xui_rect_t){cell.fX + 2, cell.fY + 2, cell.fW - 4, cell.fH - 4}, 2.0f, XUI_COLOR_RGBA(232, 26, 36, 225));
		(void)pApp->tProxy.drawLine(&pApp->tProxy, pDraw, (float)(cell.fX + 3), (float)(cell.fY + cell.fH - 3), (float)(cell.fX + cell.fW - 3), (float)(cell.fY + 3), 2.0f, XUI_COLOR_RGBA(232, 26, 36, 225));
		return;
	}
	for ( ry = 0; ry < 3; ry++ ) {
		for ( rx = 0; rx < 3; rx++ ) {
			unsigned char bit;
			if ( rx == 1 && ry == 1 ) continue;
			bit = mapedit_tileset_passage_bit_for_region(rx, ry);
			if ( (value & bit) == 0 ) mapedit_draw_forbidden_triangle(pApp, pDraw, cell, rx, ry);
		}
	}
}

static int mapedit_map_passage_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	int firstX;
	int firstY;
	int lastX;
	int lastY;
	int x;
	int y;
	int l;
	int scrollPixelX;
	int scrollPixelY;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(248, 252, 255, 255));
	if ( pApp->tMap.pTiles == NULL ) {
		return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "未加载地图", r, XUI_COLOR_RGBA(64, 80, 96, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE);
	}
	scrollPixelX = (int)floorf(pApp->fMapPassageScrollX);
	scrollPixelY = (int)floorf(pApp->fMapPassageScrollY);
	firstX = mapedit_max_i(0, scrollPixelX / mapedit_view_cell(pApp, pWidget, 0));
	firstY = mapedit_max_i(0, scrollPixelY / mapedit_view_cell(pApp, pWidget, 0));
	lastX = mapedit_min_i(pApp->tMap.iWidth - 1, (scrollPixelX + r.fW + mapedit_view_cell(pApp, pWidget, 0) - 1) / mapedit_view_cell(pApp, pWidget, 0));
	lastY = mapedit_min_i(pApp->tMap.iHeight - 1, (scrollPixelY + r.fH + mapedit_view_cell(pApp, pWidget, 0) - 1) / mapedit_view_cell(pApp, pWidget, 0));
	for ( y = firstY; y <= lastY; y++ ) {
		for ( x = firstX; x <= lastX; x++ ) {
			int dx = r.fX + x * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelX;
			int dy = r.fY + y * mapedit_view_cell(pApp, pWidget, 0) - scrollPixelY;
			xui_rect_t cell = {dx, dy, mapedit_view_cell(pApp, pWidget, 0), mapedit_view_cell(pApp, pWidget, 0)};
			for ( l = 0; l < pApp->tMap.iLayers; l++ ) {
				int tile = mapedit_get_tile(pApp, x, y, l);
				if ( tile > 0 ) (void)mapedit_draw_map_tile_rect(pApp, pDraw, tile, l, x, y, cell);
			}
			mapedit_draw_passage_overlay_rect(pApp, pDraw, cell, mapedit_map_get_cell_passage(pApp, x, y));
			(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, cell, 1.0f, XUI_COLOR_RGBA(124, 181, 219, 130));
			if ( mapedit_map_cell_id(&pApp->tMap, x, y) == pApp->iMapPassageSelectedCell ) {
				(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, (xui_rect_t){cell.fX + 1, cell.fY + 1, cell.fW - 2, cell.fH - 2}, 2.0f, XUI_COLOR_RGBA(255, 144, 0, 255));
			}
		}
	}
	(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, r, 1.0f, XUI_COLOR_RGBA(164, 206, 236, 255));
	return XUI_OK;
}

static int mapedit_tile_panel_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	int rowCount;
	int firstRow;
	int lastRow;
	int row;
	int col;
	int special;
	int staticStart;
	float* pScrollX;
	float* pScrollY;
	float scrollX;
	float scrollY;
	int scrollPixelX;
	int scrollPixelY;
	int cellW;
	int cellH;
	uint32_t gridColor;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, MAPEDIT_TILEGRID_BG);
	mapedit_tileset_panel_clamp_scroll(pApp, pWidget);
	mapedit_tileset_panel_scroll_ptrs(pApp, pWidget, &pScrollX, &pScrollY);
	scrollX = pScrollX != NULL ? *pScrollX : 0.0f;
	scrollY = pScrollY != NULL ? *pScrollY : 0.0f;
	scrollPixelX = (int)floorf(scrollX);
	scrollPixelY = (int)floorf(scrollY);
	cellW = mapedit_tileset_panel_cell_width(pApp, pWidget);
	cellH = mapedit_tileset_panel_cell_height(pApp, pWidget);
	gridColor = (pWidget == pApp->pTilesetTagsCanvas) ? MAPEDIT_TILEGRID_GRID_TAGS : MAPEDIT_TILEGRID_GRID;
	rowCount = mapedit_tile_select_row_count(pApp);
	special = pApp->tTileset.iSpecialCount;
	staticStart = mapedit_tile_static_start(pApp);
	firstRow = mapedit_max_i(0, (int)(scrollY / (float)cellH));
	lastRow = mapedit_min_i(rowCount, firstRow + r.fH / cellH + 3);
	if ( pApp->tTileset.pStaticSurface == NULL ) {
		int staticRow = staticStart / MAPEDIT_TILES_PER_ROW;
		xui_rect_t staticRect = {
			r.fX - scrollPixelX,
			r.fY + staticRow * cellH - scrollPixelY,
			MAPEDIT_TILES_PER_ROW * cellW,
			mapedit_tile_static_rows(pApp) * cellH
		};
		(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, staticRect, XUI_COLOR_RGBA(235, 244, 250, 255));
	}
	for ( row = firstRow; row < lastRow; row++ ) {
		for ( col = 0; col < MAPEDIT_TILES_PER_ROW; col++ ) {
		int tile = row * MAPEDIT_TILES_PER_ROW + col;
		int dx = r.fX + col * cellW - scrollPixelX;
		int dy = r.fY + row * cellH - scrollPixelY;
		xui_rect_t cell = (xui_rect_t){dx, dy, cellW, cellH};
		int editable;
		int isSpecial;
		if ( dx + cellW < r.fX || dx > r.fX + r.fW ||
		     dy + cellH < r.fY || dy > r.fY + r.fH ) continue;
		editable = mapedit_tileset_panel_tile_editable(pApp, pWidget, tile);
		isSpecial = tile >= 1 && tile <= special;
		if ( tile == 0 ) {
			mapedit_draw_zero_tile_cell(pApp, pDraw, cell);
		} else if ( isSpecial && (pWidget == pApp->pTilesetPassageCanvas || pWidget == pApp->pTilesetActorCanvas) ) {
			mapedit_draw_special_placeholder_cell(pApp, pDraw, cell);
		} else if ( isSpecial && pWidget == pApp->pTilesetTagsCanvas ) {
			/* Tags page only paints custom markers over special slots, matching the XUI1 grid. */
		} else if ( mapedit_tile_select_tile_selectable(pApp, tile) ) {
			(void)mapedit_draw_tile_rect(pApp, pDraw, tile, cell);
		} else if ( tile > special && tile < staticStart ) {
			(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, cell, XUI_COLOR_RGBA(224, 232, 238, 180));
			if ( cellW >= 20 && cellH >= 16 ) {
				(void)pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "-", cell, XUI_COLOR_RGBA(120, 136, 148, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			}
		}
		if ( pWidget == pApp->pTilesetPassageCanvas && tile > 0 && editable ) {
			if ( tile < (int)sizeof(pApp->tTileset.arrPassage) ) {
				mapedit_draw_passage_overlay_rect(pApp, pDraw, cell, pApp->tTileset.arrPassage[tile]);
			}
		} else if ( pWidget == pApp->pTilesetActorCanvas && tile > 0 && editable &&
		            tile < (int)sizeof(pApp->tTileset.arrActorOverlay) && pApp->tTileset.arrActorOverlay[tile] ) {
			mapedit_draw_actor_overlay_marker(pApp, pDraw, cell);
		} else if ( pWidget == pApp->pTilesetTagsCanvas ) {
			mapedit_custom_channel_def_t* pDef = mapedit_custom_channel(pApp, pApp->iTilesetTagChannel);
			char sValue[MAPEDIT_CUSTOM_VALUE_MAX];
			if ( editable && pDef != NULL &&
			     mapedit_custom_get_value(pApp->tTileset.pTileCustomRaw, pDef->sId, SET_KEY_TILE, tile, sValue, sizeof(sValue)) &&
			     strcmp(sValue, pDef->sDefaultValue) != 0 ) {
				mapedit_draw_tag_marker(pApp, pDraw, cell, sValue);
			}
		}
		}
	}
	mapedit_draw_tilegrid_lines(pApp, pDraw, r, scrollX, scrollY, cellW, cellH, MAPEDIT_TILES_PER_ROW, rowCount, gridColor);
	if ( pWidget == pApp->pTilesetArrangeCanvas || pWidget == pApp->pTilesetTagsCanvas ) {
		int hoverTile = (pWidget == pApp->pTilesetArrangeCanvas) ? pApp->iTilesetArrangeHoverTile : pApp->iTilesetTagsHoverTile;
		int selectTile = (pWidget == pApp->pTilesetArrangeCanvas) ? pApp->iTilesetArrangeSelectedTile : pApp->iTilesetTagsSelectedTile;
		if ( hoverTile >= 0 && hoverTile < rowCount * MAPEDIT_TILES_PER_ROW ) {
			int hoverCol = hoverTile % MAPEDIT_TILES_PER_ROW;
			int hoverRow = hoverTile / MAPEDIT_TILES_PER_ROW;
			xui_rect_t hover = {
				r.fX + hoverCol * cellW - scrollPixelX,
				r.fY + hoverRow * cellH - scrollPixelY,
				cellW + 1,
				cellH + 1
			};
			if ( hover.fX + hover.fW >= r.fX && hover.fY + hover.fH >= r.fY && hover.fX <= r.fX + r.fW && hover.fY <= r.fY + r.fH ) {
				(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, hover, 1.0f, XUI_COLOR_RGBA(42, 140, 210, 180));
			}
		}
		if ( selectTile >= 0 && selectTile < rowCount * MAPEDIT_TILES_PER_ROW ) {
			int selectCol = selectTile % MAPEDIT_TILES_PER_ROW;
			int selectRow = selectTile / MAPEDIT_TILES_PER_ROW;
			xui_rect_t sel = {
				r.fX + selectCol * cellW - scrollPixelX,
				r.fY + selectRow * cellH - scrollPixelY,
				cellW + 1,
				cellH + 1
			};
			if ( sel.fX + sel.fW >= r.fX && sel.fY + sel.fH >= r.fY && sel.fX <= r.fX + r.fW && sel.fY <= r.fY + r.fH ) {
				(void)pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, sel, 1.0f, XUI_COLOR_RGBA(28, 122, 196, 230));
			}
		}
	}
	mapedit_draw_tilegrid_border(pApp, pDraw, r, scrollX, scrollY, cellW, cellH, MAPEDIT_TILES_PER_ROW, rowCount);
	return XUI_OK;
}

static int mapedit_pointer_to_map_tile_on_widget(mapedit_app_t* pApp, xui_widget pWidget, int iWorldX, int iWorldY, int* pTileX, int* pTileY)
{
	xui_rect_t wr;
	float scrollX;
	float scrollY;
	int localX;
	int localY;
	int tx;
	int ty;
	if ( pApp == NULL || pWidget == NULL ) return 0;
	wr = xuiWidgetGetWorldRect(pWidget);
	mapedit_map_widget_scroll(pApp, pWidget, &scrollX, &scrollY);
	localX = iWorldX - wr.fX + (int)floorf(scrollX);
	localY = iWorldY - wr.fY + (int)floorf(scrollY);
	if ( localX < 0 || localY < 0 ) return 0;
	tx = localX / mapedit_view_cell(pApp, pWidget, 0);
	ty = localY / mapedit_view_cell(pApp, pWidget, 1);
	if ( tx < 0 || ty < 0 || tx >= pApp->tMap.iWidth || ty >= pApp->tMap.iHeight ) return 0;
	if ( pTileX != NULL ) *pTileX = tx;
	if ( pTileY != NULL ) *pTileY = ty;
	return 1;
}

static int mapedit_pointer_to_tile(mapedit_app_t* pApp, int iWorldX, int iWorldY, int* pTileX, int* pTileY)
{
	return mapedit_pointer_to_map_tile_on_widget(pApp, pApp != NULL ? pApp->pMapCanvas : NULL, iWorldX, iWorldY, pTileX, pTileY);
}

static void mapedit_set_map_hover(mapedit_app_t* pApp, int x, int y)
{
	if ( pApp == NULL ) return;
	if ( pApp->iMapHoverX == x && pApp->iMapHoverY == y ) return;
	pApp->iMapHoverX = x;
	pApp->iMapHoverY = y;
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_set_map_tags_hover(mapedit_app_t* pApp, int x, int y)
{
	if ( pApp == NULL ) return;
	if ( pApp->iMapTagsHoverX == x && pApp->iMapTagsHoverY == y ) return;
	pApp->iMapTagsHoverX = x;
	pApp->iMapTagsHoverY = y;
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_rect_almost_equal(xui_rect_t a, xui_rect_t b)
{
	return a.fX == b.fX && a.fY == b.fY && a.fW == b.fW && a.fH == b.fH;
}

static void mapedit_map_scroll_content_size(mapedit_app_t* pApp, float* pWidth, float* pHeight)
{
	float w;
	float h;
	w = 1.0f;
	h = 1.0f;
	if ( pApp != NULL && pApp->tMap.iWidth > 0 && pApp->tMap.iHeight > 0 ) {
		w = (float)(pApp->tMap.iWidth * mapedit_view_cell(pApp, pApp->pMapCanvas, 0));
		h = (float)(pApp->tMap.iHeight * mapedit_view_cell(pApp, pApp->pMapCanvas, 1));
	}
	if ( pWidth != NULL ) *pWidth = w;
	if ( pHeight != NULL ) *pHeight = h;
}

static void mapedit_map_scroll_invalidate(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_scroll_sync_from_frame(mapedit_app_t* pApp)
{
	float sx;
	float sy;
	if ( pApp == NULL || pApp->pMapScrollFrame == NULL ) return;
	if ( xuiScrollFrameGetOffset(pApp->pMapScrollFrame, &sx, &sy) != XUI_OK ) return;
	if ( fabsf(pApp->fMapScrollX - sx) < 0.01f && fabsf(pApp->fMapScrollY - sy) < 0.01f ) return;
	pApp->fMapScrollX = sx;
	pApp->fMapScrollY = sy;
	mapedit_map_scroll_invalidate(pApp);
}

static void mapedit_map_scroll_changed(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	mapedit_app_t* pApp;
	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) return;
	if ( fabsf(pApp->fMapScrollX - fOffsetX) < 0.01f && fabsf(pApp->fMapScrollY - fOffsetY) < 0.01f ) return;
	pApp->fMapScrollX = fOffsetX;
	pApp->fMapScrollY = fOffsetY;
	mapedit_map_scroll_invalidate(pApp);
}

static void mapedit_map_scroll_sync_content(mapedit_app_t* pApp)
{
	xui_widget viewport;
	xui_rect_t viewportRect;
	xui_rect_t canvasRect;
	xui_rect_t targetRect;
	float contentW;
	float contentH;
	float currentW;
	float currentH;

	if ( pApp == NULL || pApp->pMapScrollFrame == NULL || pApp->pMapCanvas == NULL ) return;
	mapedit_map_scroll_content_size(pApp, &contentW, &contentH);
	currentW = 0.0f;
	currentH = 0.0f;
	if ( xuiScrollFrameGetContentSize(pApp->pMapScrollFrame, &currentW, &currentH) == XUI_OK &&
	     (fabsf(currentW - contentW) >= 0.01f || fabsf(currentH - contentH) >= 0.01f) ) {
		(void)xuiScrollFrameSetContentSize(pApp->pMapScrollFrame, contentW, contentH);
	}
	(void)xuiScrollFrameSetOffset(pApp->pMapScrollFrame, pApp->fMapScrollX, pApp->fMapScrollY);
	mapedit_map_scroll_sync_from_frame(pApp);
	viewport = xuiScrollFrameGetViewportWidget(pApp->pMapScrollFrame);
	if ( viewport == NULL ) return;
	viewportRect = xuiWidgetGetRect(viewport);
	targetRect = (xui_rect_t){0.0f, 0.0f, viewportRect.fW, viewportRect.fH};
	canvasRect = xuiWidgetGetRect(pApp->pMapCanvas);
	if ( !mapedit_rect_almost_equal(canvasRect, targetRect) ) {
		(void)xuiWidgetArrangeChild(viewport, pApp->pMapCanvas, targetRect);
	}
}

static void mapedit_map_scroll_reset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	pApp->fMapScrollX = 0.0f;
	pApp->fMapScrollY = 0.0f;
	if ( pApp->pMapScrollFrame != NULL ) (void)xuiScrollFrameSetOffset(pApp->pMapScrollFrame, 0.0f, 0.0f);
	mapedit_map_scroll_sync_content(pApp);
	mapedit_map_scroll_invalidate(pApp);
}

static void mapedit_map_scroll_by_wheel(mapedit_app_t* pApp, const xui_event_t* pEvent)
{
	if ( pApp == NULL || pEvent == NULL || pApp->pMapScrollFrame == NULL ) return;
	(void)xuiScrollFrameScrollBy(pApp->pMapScrollFrame, -pEvent->fWheelX * 40.0f, -pEvent->fWheelY * 40.0f);
	mapedit_map_scroll_sync_from_frame(pApp);
}

static void mapedit_canvas_scroll_arrange(xui_widget pFrame, xui_widget pCanvas)
{
	xui_widget viewport;
	xui_rect_t viewportRect;
	xui_rect_t canvasRect;
	xui_rect_t targetRect;
	if ( pFrame == NULL || pCanvas == NULL ) return;
	viewport = xuiScrollFrameGetViewportWidget(pFrame);
	if ( viewport == NULL ) return;
	viewportRect = xuiWidgetGetRect(viewport);
	targetRect = (xui_rect_t){0.0f, 0.0f, viewportRect.fW, viewportRect.fH};
	canvasRect = xuiWidgetGetRect(pCanvas);
	if ( !mapedit_rect_almost_equal(canvasRect, targetRect) ) {
		(void)xuiWidgetArrangeChild(viewport, pCanvas, targetRect);
	}
}

static void mapedit_scroll_frame_set_content_if_changed(xui_widget pFrame, float fContentW, float fContentH)
{
	float currentW;
	float currentH;
	if ( pFrame == NULL ) return;
	currentW = 0.0f;
	currentH = 0.0f;
	if ( xuiScrollFrameGetContentSize(pFrame, &currentW, &currentH) == XUI_OK &&
	     (fabsf(currentW - fContentW) >= 0.01f || fabsf(currentH - fContentH) >= 0.01f) ) {
		(void)xuiScrollFrameSetContentSize(pFrame, fContentW, fContentH);
	}
}

static xui_widget mapedit_tileset_panel_scroll_frame(mapedit_app_t* pApp, xui_widget pCanvas)
{
	if ( pApp == NULL || pCanvas == NULL ) return NULL;
	if ( pCanvas == pApp->pTilesetArrangeCanvas ) return pApp->pTilesetArrangeScrollFrame;
	if ( pCanvas == pApp->pTilesetPassageCanvas ) return pApp->pTilesetPassageScrollFrame;
	if ( pCanvas == pApp->pTilesetActorCanvas ) return pApp->pTilesetActorScrollFrame;
	if ( pCanvas == pApp->pTilesetTagsCanvas ) return pApp->pTilesetTagsScrollFrame;
	return NULL;
}

static xui_widget mapedit_tileset_panel_canvas_for_frame(mapedit_app_t* pApp, xui_widget pFrame)
{
	if ( pApp == NULL || pFrame == NULL ) return NULL;
	if ( pFrame == pApp->pTilesetArrangeScrollFrame ) return pApp->pTilesetArrangeCanvas;
	if ( pFrame == pApp->pTilesetPassageScrollFrame ) return pApp->pTilesetPassageCanvas;
	if ( pFrame == pApp->pTilesetActorScrollFrame ) return pApp->pTilesetActorCanvas;
	if ( pFrame == pApp->pTilesetTagsScrollFrame ) return pApp->pTilesetTagsCanvas;
	return NULL;
}

static void mapedit_tileset_panel_scroll_invalidate(xui_widget pCanvas)
{
	if ( pCanvas != NULL ) (void)xuiWidgetInvalidate(pCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tileset_panel_scroll_sync_from_frame(mapedit_app_t* pApp, xui_widget pCanvas)
{
	xui_widget frame;
	float* pScrollX;
	float* pScrollY;
	float sx;
	float sy;
	if ( pApp == NULL || pCanvas == NULL ) return;
	frame = mapedit_tileset_panel_scroll_frame(pApp, pCanvas);
	if ( frame == NULL ) return;
	mapedit_tileset_panel_scroll_ptrs(pApp, pCanvas, &pScrollX, &pScrollY);
	if ( pScrollX == NULL || pScrollY == NULL ) return;
	if ( xuiScrollFrameGetOffset(frame, &sx, &sy) != XUI_OK ) return;
	if ( fabsf(*pScrollX - sx) < 0.01f && fabsf(*pScrollY - sy) < 0.01f ) return;
	*pScrollX = sx;
	*pScrollY = sy;
	mapedit_tileset_panel_scroll_invalidate(pCanvas);
}

static void mapedit_tileset_panel_scroll_changed(xui_widget pFrame, float fOffsetX, float fOffsetY, void* pUser)
{
	mapedit_app_t* pApp;
	xui_widget canvas;
	float* pScrollX;
	float* pScrollY;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) return;
	canvas = mapedit_tileset_panel_canvas_for_frame(pApp, pFrame);
	if ( canvas == NULL ) return;
	mapedit_tileset_panel_scroll_ptrs(pApp, canvas, &pScrollX, &pScrollY);
	if ( pScrollX == NULL || pScrollY == NULL ) return;
	if ( fabsf(*pScrollX - fOffsetX) < 0.01f && fabsf(*pScrollY - fOffsetY) < 0.01f ) return;
	*pScrollX = fOffsetX;
	*pScrollY = fOffsetY;
	mapedit_tileset_panel_scroll_invalidate(canvas);
}

static void mapedit_tileset_panel_scroll_sync_content(mapedit_app_t* pApp, xui_widget pCanvas)
{
	xui_widget frame;
	float* pScrollX;
	float* pScrollY;
	float contentW;
	float contentH;
	if ( pApp == NULL || pCanvas == NULL ) return;
	frame = mapedit_tileset_panel_scroll_frame(pApp, pCanvas);
	if ( frame == NULL ) return;
	mapedit_tileset_panel_scroll_ptrs(pApp, pCanvas, &pScrollX, &pScrollY);
	if ( pScrollX == NULL || pScrollY == NULL ) return;
	contentW = mapedit_tileset_panel_content_width(pApp, pCanvas);
	contentH = mapedit_tileset_panel_content_height(pApp, pCanvas);
	mapedit_scroll_frame_set_content_if_changed(frame, contentW, contentH);
	(void)xuiScrollFrameSetOffset(frame, *pScrollX, *pScrollY);
	mapedit_tileset_panel_scroll_sync_from_frame(pApp, pCanvas);
	mapedit_canvas_scroll_arrange(frame, pCanvas);
}

static void mapedit_tileset_workspace_scroll_sync_content(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	mapedit_tileset_panel_scroll_sync_content(pApp, pApp->pTilesetArrangeCanvas);
	mapedit_tileset_panel_scroll_sync_content(pApp, pApp->pTilesetPassageCanvas);
	mapedit_tileset_panel_scroll_sync_content(pApp, pApp->pTilesetActorCanvas);
	mapedit_tileset_panel_scroll_sync_content(pApp, pApp->pTilesetTagsCanvas);
}

static void mapedit_tile_select_scroll_content_size(mapedit_app_t* pApp, float* pWidth, float* pHeight)
{
	if ( pWidth != NULL ) *pWidth = (float)(MAPEDIT_TILES_PER_ROW * mapedit_view_cell(pApp, pApp->pTileSelectCanvas, 0));
	if ( pHeight != NULL ) *pHeight = (float)(mapedit_tile_select_row_count(pApp) * mapedit_view_cell(pApp, pApp->pTileSelectCanvas, 1));
}

static void mapedit_tile_select_scroll_sync_from_frame(mapedit_app_t* pApp)
{
	float sx;
	float sy;
	if ( pApp == NULL || pApp->pTileSelectScrollFrame == NULL ) return;
	if ( xuiScrollFrameGetOffset(pApp->pTileSelectScrollFrame, &sx, &sy) != XUI_OK ) return;
	if ( fabsf(pApp->fTileScrollX - sx) < 0.01f && fabsf(pApp->fTileScrollY - sy) < 0.01f ) return;
	pApp->fTileScrollX = sx;
	pApp->fTileScrollY = sy;
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tile_select_scroll_changed(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL ) return;
	if ( fabsf(pApp->fTileScrollX - fOffsetX) < 0.01f && fabsf(pApp->fTileScrollY - fOffsetY) < 0.01f ) return;
	pApp->fTileScrollX = fOffsetX;
	pApp->fTileScrollY = fOffsetY;
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tile_select_scroll_sync_content(mapedit_app_t* pApp)
{
	float contentW;
	float contentH;
	if ( pApp == NULL || pApp->pTileSelectScrollFrame == NULL || pApp->pTileSelectCanvas == NULL ) return;
	mapedit_tile_select_scroll_content_size(pApp, &contentW, &contentH);
	mapedit_scroll_frame_set_content_if_changed(pApp->pTileSelectScrollFrame, contentW, contentH);
	(void)xuiScrollFrameSetOffset(pApp->pTileSelectScrollFrame, pApp->fTileScrollX, pApp->fTileScrollY);
	mapedit_tile_select_scroll_sync_from_frame(pApp);
	mapedit_canvas_scroll_arrange(pApp->pTileSelectScrollFrame, pApp->pTileSelectCanvas);
}

static void mapedit_tile_select_scroll_reset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	pApp->fTileScrollX = 0.0f;
	pApp->fTileScrollY = 0.0f;
	if ( pApp->pTileSelectScrollFrame != NULL ) (void)xuiScrollFrameSetOffset(pApp->pTileSelectScrollFrame, 0.0f, 0.0f);
	mapedit_tile_select_scroll_sync_content(pApp);
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tile_select_scroll_by_wheel(mapedit_app_t* pApp, const xui_event_t* pEvent)
{
	if ( pApp == NULL || pEvent == NULL || pApp->pTileSelectScrollFrame == NULL ) return;
	(void)xuiScrollFrameScrollBy(pApp->pTileSelectScrollFrame, -pEvent->fWheelX * 40.0f, -pEvent->fWheelY * 40.0f);
	mapedit_tile_select_scroll_sync_from_frame(pApp);
}

static void mapedit_map_passage_scroll_content_size(mapedit_app_t* pApp, float* pWidth, float* pHeight)
{
	float w = 1.0f;
	float h = 1.0f;
	if ( pApp != NULL && pApp->tMap.iWidth > 0 && pApp->tMap.iHeight > 0 ) {
		w = (float)(pApp->tMap.iWidth * mapedit_view_cell(pApp, pApp->pMapPassageCanvas, 0));
		h = (float)(pApp->tMap.iHeight * mapedit_view_cell(pApp, pApp->pMapPassageCanvas, 0));
	}
	if ( pWidth != NULL ) *pWidth = w;
	if ( pHeight != NULL ) *pHeight = h;
}

static void mapedit_map_passage_scroll_sync_from_frame(mapedit_app_t* pApp)
{
	float sx;
	float sy;
	if ( pApp == NULL || pApp->pMapPassageScrollFrame == NULL ) return;
	if ( xuiScrollFrameGetOffset(pApp->pMapPassageScrollFrame, &sx, &sy) != XUI_OK ) return;
	if ( fabsf(pApp->fMapPassageScrollX - sx) < 0.01f && fabsf(pApp->fMapPassageScrollY - sy) < 0.01f ) return;
	pApp->fMapPassageScrollX = sx;
	pApp->fMapPassageScrollY = sy;
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_passage_scroll_changed(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL ) return;
	if ( fabsf(pApp->fMapPassageScrollX - fOffsetX) < 0.01f && fabsf(pApp->fMapPassageScrollY - fOffsetY) < 0.01f ) return;
	pApp->fMapPassageScrollX = fOffsetX;
	pApp->fMapPassageScrollY = fOffsetY;
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_passage_scroll_sync_content(mapedit_app_t* pApp)
{
	float contentW;
	float contentH;
	if ( pApp == NULL || pApp->pMapPassageScrollFrame == NULL || pApp->pMapPassageCanvas == NULL ) return;
	mapedit_map_passage_scroll_content_size(pApp, &contentW, &contentH);
	mapedit_scroll_frame_set_content_if_changed(pApp->pMapPassageScrollFrame, contentW, contentH);
	(void)xuiScrollFrameSetOffset(pApp->pMapPassageScrollFrame, pApp->fMapPassageScrollX, pApp->fMapPassageScrollY);
	mapedit_map_passage_scroll_sync_from_frame(pApp);
	mapedit_canvas_scroll_arrange(pApp->pMapPassageScrollFrame, pApp->pMapPassageCanvas);
}

static void mapedit_map_passage_scroll_reset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	pApp->fMapPassageScrollX = 0.0f;
	pApp->fMapPassageScrollY = 0.0f;
	if ( pApp->pMapPassageScrollFrame != NULL ) (void)xuiScrollFrameSetOffset(pApp->pMapPassageScrollFrame, 0.0f, 0.0f);
	mapedit_map_passage_scroll_sync_content(pApp);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_passage_scroll_by_wheel(mapedit_app_t* pApp, const xui_event_t* pEvent)
{
	if ( pApp == NULL || pEvent == NULL || pApp->pMapPassageScrollFrame == NULL ) return;
	(void)xuiScrollFrameScrollBy(pApp->pMapPassageScrollFrame, -pEvent->fWheelX * 40.0f, -pEvent->fWheelY * 40.0f);
	mapedit_map_passage_scroll_sync_from_frame(pApp);
}

static void mapedit_map_tags_scroll_content_size(mapedit_app_t* pApp, float* pWidth, float* pHeight)
{
	float w = 1.0f;
	float h = 1.0f;
	if ( pApp != NULL && pApp->tMap.iWidth > 0 && pApp->tMap.iHeight > 0 ) {
		w = (float)(pApp->tMap.iWidth * mapedit_view_cell(pApp, pApp->pMapTagsCanvas, 0));
		h = (float)(pApp->tMap.iHeight * mapedit_view_cell(pApp, pApp->pMapTagsCanvas, 1));
	}
	if ( pWidth != NULL ) *pWidth = w;
	if ( pHeight != NULL ) *pHeight = h;
}

static void mapedit_map_tags_scroll_sync_from_frame(mapedit_app_t* pApp)
{
	float sx;
	float sy;
	if ( pApp == NULL || pApp->pMapTagsScrollFrame == NULL ) return;
	if ( xuiScrollFrameGetOffset(pApp->pMapTagsScrollFrame, &sx, &sy) != XUI_OK ) return;
	if ( fabsf(pApp->fMapTagsScrollX - sx) < 0.01f && fabsf(pApp->fMapTagsScrollY - sy) < 0.01f ) return;
	pApp->fMapTagsScrollX = sx;
	pApp->fMapTagsScrollY = sy;
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_tags_scroll_changed(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL ) return;
	if ( fabsf(pApp->fMapTagsScrollX - fOffsetX) < 0.01f && fabsf(pApp->fMapTagsScrollY - fOffsetY) < 0.01f ) return;
	pApp->fMapTagsScrollX = fOffsetX;
	pApp->fMapTagsScrollY = fOffsetY;
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_tags_scroll_sync_content(mapedit_app_t* pApp)
{
	float contentW;
	float contentH;
	if ( pApp == NULL || pApp->pMapTagsScrollFrame == NULL || pApp->pMapTagsCanvas == NULL ) return;
	mapedit_map_tags_scroll_content_size(pApp, &contentW, &contentH);
	mapedit_scroll_frame_set_content_if_changed(pApp->pMapTagsScrollFrame, contentW, contentH);
	(void)xuiScrollFrameSetOffset(pApp->pMapTagsScrollFrame, pApp->fMapTagsScrollX, pApp->fMapTagsScrollY);
	mapedit_map_tags_scroll_sync_from_frame(pApp);
	mapedit_canvas_scroll_arrange(pApp->pMapTagsScrollFrame, pApp->pMapTagsCanvas);
}

static void mapedit_map_tags_scroll_reset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	pApp->fMapTagsScrollX = 0.0f;
	pApp->fMapTagsScrollY = 0.0f;
	if ( pApp->pMapTagsScrollFrame != NULL ) (void)xuiScrollFrameSetOffset(pApp->pMapTagsScrollFrame, 0.0f, 0.0f);
	mapedit_map_tags_scroll_sync_content(pApp);
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_map_tags_scroll_by_wheel(mapedit_app_t* pApp, const xui_event_t* pEvent)
{
	if ( pApp == NULL || pEvent == NULL || pApp->pMapTagsScrollFrame == NULL ) return;
	(void)xuiScrollFrameScrollBy(pApp->pMapTagsScrollFrame, -pEvent->fWheelX * 40.0f, -pEvent->fWheelY * 40.0f);
	mapedit_map_tags_scroll_sync_from_frame(pApp);
}

static void mapedit_map_workspace_scroll_sync_content(mapedit_app_t* pApp)
{
	mapedit_map_scroll_sync_content(pApp);
	mapedit_tile_select_scroll_sync_content(pApp);
	mapedit_map_passage_scroll_sync_content(pApp);
	mapedit_map_tags_scroll_sync_content(pApp);
}

static int mapedit_apply_pointer_brush(mapedit_app_t* pApp, int iWorldX, int iWorldY)
{
	int tx;
	int ty;
	int changed;
	if ( !mapedit_pointer_to_tile(pApp, iWorldX, iWorldY, &tx, &ty) ) return 0;
	if ( tx == pApp->iLastPaintX && ty == pApp->iLastPaintY ) return 0;
	changed = mapedit_apply_brush_at(pApp, tx, ty);
	if ( changed >= 0 ) {
		pApp->iLastPaintX = tx;
		pApp->iLastPaintY = ty;
	}
	return changed;
}

static int mapedit_map_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t wr;
	int tx;
	int ty;
	(void)pWidget;
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	wr = xuiWidgetGetWorldRect(pApp->pMapCanvas);
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( pApp->pMapScrollFrame != NULL ) {
			mapedit_map_scroll_by_wheel(pApp, pEvent);
		} else {
			float maxX = (float)mapedit_max_i(0, pApp->tMap.iWidth * MAPEDIT_TILE_W - wr.fW);
			float maxY = (float)mapedit_max_i(0, pApp->tMap.iHeight * MAPEDIT_TILE_H - wr.fH);
			pApp->fMapScrollX = mapedit_clampf(pApp->fMapScrollX - pEvent->fWheelX * 40.0f, 0.0f, maxX);
			pApp->fMapScrollY = mapedit_clampf(pApp->fMapScrollY - pEvent->fWheelY * 40.0f, 0.0f, maxY);
			mapedit_map_scroll_invalidate(pApp);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE && !pApp->bPainting ) {
		mapedit_set_map_hover(pApp, -1, -1);
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( pApp->bPainting && (pApp->iActiveTool == MAPEDIT_TOOL_BRUSH || pApp->iActiveTool == MAPEDIT_TOOL_ERASER) ) {
			mapedit_finish_map_edit(pApp, pApp->iActiveTool == MAPEDIT_TOOL_ERASER ? "橡皮" : "画笔");
		} else {
			mapeditHistoryFreeCommand(&pApp->tMapHistory.tCurrentCommand);
			pApp->bGestureDirty = 0;
			pApp->bBatchEdit = 0;
		}
		pApp->bPainting = 0;
		pApp->iDragCurrentX = -1;
		pApp->iDragCurrentY = -1;
		mapedit_set_map_hover(pApp, -1, -1);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pApp->bPreview ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		if ( mapedit_pointer_to_tile(pApp, pEvent->fX, pEvent->fY, &tx, &ty) ) mapedit_set_map_hover(pApp, tx, ty);
		else mapedit_set_map_hover(pApp, -1, -1);
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( !mapedit_pointer_to_tile(pApp, pEvent->fX, pEvent->fY, &tx, &ty) ) return XUI_EVENT_DISPATCH_STOP;
		mapedit_set_map_hover(pApp, tx, ty);
		pApp->bPainting = 1;
		pApp->bGestureDirty = 0;
		pApp->bBatchEdit = 0;
		mapedit_begin_history(pApp);
		pApp->iDragStartX = tx;
		pApp->iDragStartY = ty;
		pApp->iDragCurrentX = tx;
		pApp->iDragCurrentY = ty;
		pApp->iLastPaintX = -999999;
		pApp->iLastPaintY = -999999;
		(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		if ( pApp->iActiveTool == MAPEDIT_TOOL_BRUSH || pApp->iActiveTool == MAPEDIT_TOOL_ERASER ) {
			if ( mapedit_apply_pointer_brush(pApp, pEvent->fX, pEvent->fY) < 0 ) {
				mapeditHistoryFreeCommand(&pApp->tMapHistory.tCurrentCommand);
				pApp->bGestureDirty = 0;
				pApp->bPainting = 0;
				if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) (void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			}
		} else if ( pApp->iActiveTool == MAPEDIT_TOOL_BUCKET ) {
			pApp->bBatchEdit = 1;
			if ( mapedit_apply_bucket(pApp, tx, ty) ) {
				mapedit_finish_map_edit(pApp, "填充");
			} else {
				mapeditHistoryFreeCommand(&pApp->tMapHistory.tCurrentCommand);
				pApp->bGestureDirty = 0;
				pApp->bBatchEdit = 0;
			}
			pApp->bPainting = 0;
			pApp->iDragCurrentX = -1;
			pApp->iDragCurrentY = -1;
			if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) (void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		} else if ( pApp->iActiveTool == MAPEDIT_TOOL_LINE || pApp->iActiveTool == MAPEDIT_TOOL_RECT || pApp->iActiveTool == MAPEDIT_TOOL_CIRCLE ) {
			(void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE && pApp->bPainting && (pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) ) {
		if ( pApp->iActiveTool == MAPEDIT_TOOL_BRUSH || pApp->iActiveTool == MAPEDIT_TOOL_ERASER ) {
			(void)mapedit_apply_pointer_brush(pApp, pEvent->fX, pEvent->fY);
		} else if ( (pApp->iActiveTool == MAPEDIT_TOOL_LINE || pApp->iActiveTool == MAPEDIT_TOOL_RECT || pApp->iActiveTool == MAPEDIT_TOOL_CIRCLE) &&
		            mapedit_pointer_to_tile(pApp, pEvent->fX, pEvent->fY, &tx, &ty) ) {
			pApp->iDragCurrentX = tx;
			pApp->iDragCurrentY = ty;
			(void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_UP && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( pApp->bPainting &&
		     (pApp->iActiveTool == MAPEDIT_TOOL_LINE || pApp->iActiveTool == MAPEDIT_TOOL_RECT || pApp->iActiveTool == MAPEDIT_TOOL_CIRCLE) ) {
			if ( !mapedit_pointer_to_tile(pApp, pEvent->fX, pEvent->fY, &tx, &ty) ) {
				tx = pApp->iDragCurrentX >= 0 ? pApp->iDragCurrentX : pApp->iDragStartX;
				ty = pApp->iDragCurrentY >= 0 ? pApp->iDragCurrentY : pApp->iDragStartY;
			}
			pApp->bBatchEdit = 1;
			mapedit_apply_shape(pApp, pApp->iDragStartX, pApp->iDragStartY, tx, ty);
			if ( pApp->iActiveTool == MAPEDIT_TOOL_LINE ) mapedit_finish_map_edit(pApp, "线条");
			else if ( pApp->iActiveTool == MAPEDIT_TOOL_RECT ) mapedit_finish_map_edit(pApp, "矩形填充");
			else mapedit_finish_map_edit(pApp, "圆形填充");
		} else if ( pApp->bPainting && (pApp->iActiveTool == MAPEDIT_TOOL_BRUSH || pApp->iActiveTool == MAPEDIT_TOOL_ERASER) ) {
			mapedit_finish_map_edit(pApp, pApp->iActiveTool == MAPEDIT_TOOL_ERASER ? "橡皮" : "画笔");
		}
		pApp->bPainting = 0;
		pApp->iDragCurrentX = -1;
		pApp->iDragCurrentY = -1;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) (void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		(void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_tile_select_hit(mapedit_app_t* pApp, xui_widget pWidget, const xui_event_t* pEvent, int* pCol, int* pRow)
{
	xui_rect_t wr;
	int rx;
	int ry;
	int col;
	int row;
	if ( pApp == NULL || pWidget == NULL || pEvent == NULL || pCol == NULL || pRow == NULL ) return 0;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->fX < wr.fX || pEvent->fY < wr.fY || pEvent->fX >= wr.fX + wr.fW || pEvent->fY >= wr.fY + wr.fH ) return 0;
	rx = pEvent->fX - wr.fX + (int)floorf(pApp->fTileScrollX);
	ry = pEvent->fY - wr.fY + (int)floorf(pApp->fTileScrollY);
	if ( rx < 0 || ry < 0 || rx >= MAPEDIT_TILES_PER_ROW * mapedit_view_cell(pApp, pWidget, 0) ) return 0;
	col = rx / mapedit_view_cell(pApp, pWidget, 0);
	row = ry / mapedit_view_cell(pApp, pWidget, 1);
	if ( col < 0 || col >= MAPEDIT_TILES_PER_ROW || row < 0 || row >= mapedit_tile_select_row_count(pApp) ) return 0;
	*pCol = col;
	*pRow = row;
	return 1;
}

static void mapedit_set_tile_select_hover(mapedit_app_t* pApp, int col, int row)
{
	if ( pApp == NULL ) return;
	if ( pApp->iTileSelectHoverCol == col && pApp->iTileSelectHoverRow == row ) return;
	pApp->iTileSelectHoverCol = col;
	pApp->iTileSelectHoverRow = row;
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_tile_select_set_selection(mapedit_app_t* pApp, int iCol0, int iRow0, int iCol1, int iRow1)
{
	int minCol;
	int maxCol;
	int minRow;
	int maxRow;
	int rowCount;
	int tile;
	char status[128];
	if ( pApp == NULL ) return;
	rowCount = mapedit_tile_select_row_count(pApp);
	minCol = mapedit_min_i(iCol0, iCol1);
	maxCol = mapedit_max_i(iCol0, iCol1);
	minRow = mapedit_min_i(iRow0, iRow1);
	maxRow = mapedit_max_i(iRow0, iRow1);
	minCol = mapedit_max_i(0, mapedit_min_i(minCol, MAPEDIT_TILES_PER_ROW - 1));
	maxCol = mapedit_max_i(0, mapedit_min_i(maxCol, MAPEDIT_TILES_PER_ROW - 1));
	minRow = mapedit_max_i(0, mapedit_min_i(minRow, rowCount - 1));
	maxRow = mapedit_max_i(0, mapedit_min_i(maxRow, rowCount - 1));
	tile = minRow * MAPEDIT_TILES_PER_ROW + minCol;
	if ( !mapedit_tile_select_tile_selectable(pApp, tile) ) {
		pApp->iSelectedTile = -1;
		pApp->iBrushW = 1;
		pApp->iBrushH = 1;
		mapedit_status(pApp, "当前图块位置不可作为地图画笔");
	} else {
		pApp->iSelectedTile = tile;
		pApp->iBrushW = maxCol - minCol + 1;
		pApp->iBrushH = maxRow - minRow + 1;
		snprintf(status, sizeof(status), "已选择图块: ID %d / %d x %d", pApp->iSelectedTile, pApp->iBrushW, pApp->iBrushH);
		mapedit_status(pApp, status);
	}
	mapedit_update_status_details(pApp);
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_tile_select_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t wr;
	int col;
	int row;
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	if ( pWidget == NULL ) return XUI_OK;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( pApp->pTileSelectScrollFrame != NULL ) {
			mapedit_tile_select_scroll_by_wheel(pApp, pEvent);
		} else {
			float maxX = (float)mapedit_max_i(0, MAPEDIT_TILES_PER_ROW * MAPEDIT_TILE_W - wr.fW);
			float maxY = (float)mapedit_max_i(0, mapedit_tile_select_row_count(pApp) * MAPEDIT_TILE_H - wr.fH);
			pApp->fTileScrollX = mapedit_clampf(pApp->fTileScrollX - pEvent->fWheelX * 40.0f, 0.0f, maxX);
			pApp->fTileScrollY = mapedit_clampf(pApp->fTileScrollY - pEvent->fWheelY * 40.0f, 0.0f, maxY);
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE && !pApp->bTileSelectDragging ) {
		mapedit_set_tile_select_hover(pApp, -1, -1);
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		pApp->bTileSelectDragging = 0;
		mapedit_set_tile_select_hover(pApp, -1, -1);
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		if ( mapedit_tile_select_hit(pApp, pWidget, pEvent, &col, &row) ) mapedit_set_tile_select_hover(pApp, col, row);
		else mapedit_set_tile_select_hover(pApp, -1, -1);
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( mapedit_tile_select_hit(pApp, pWidget, pEvent, &col, &row) ) {
			pApp->iTileSelectAnchorCol = col;
			pApp->iTileSelectAnchorRow = row;
			pApp->bTileSelectDragging = 1;
			(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			mapedit_set_tile_select_hover(pApp, col, row);
			mapedit_tile_select_set_selection(pApp, col, row, col, row);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE && pApp->bTileSelectDragging ) {
		if ( !(pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) ) {
			pApp->bTileSelectDragging = 0;
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( mapedit_tile_select_hit(pApp, pWidget, pEvent, &col, &row) ) {
			mapedit_tile_select_set_selection(pApp, pApp->iTileSelectAnchorCol, pApp->iTileSelectAnchorRow, col, row);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_UP && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( pApp->bTileSelectDragging && mapedit_tile_select_hit(pApp, pWidget, pEvent, &col, &row) ) {
			mapedit_tile_select_set_selection(pApp, pApp->iTileSelectAnchorCol, pApp->iTileSelectAnchorRow, col, row);
		}
		pApp->bTileSelectDragging = 0;
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_map_passage_hit(mapedit_app_t* pApp, xui_widget pWidget, const xui_event_t* pEvent, int* pX, int* pY, int* pRegionCol, int* pRegionRow)
{
	xui_rect_t wr;
	int rx;
	int ry;
	int x;
	int y;
	int inCellX;
	int inCellY;
	if ( pApp == NULL || pWidget == NULL || pEvent == NULL || pX == NULL || pY == NULL ) return 0;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->fX < wr.fX || pEvent->fY < wr.fY || pEvent->fX >= wr.fX + wr.fW || pEvent->fY >= wr.fY + wr.fH ) return 0;
	rx = pEvent->fX - wr.fX + (int)floorf(pApp->fMapPassageScrollX);
	ry = pEvent->fY - wr.fY + (int)floorf(pApp->fMapPassageScrollY);
	if ( rx < 0 || ry < 0 ) return 0;
	x = rx / mapedit_view_cell(pApp, pWidget, 0);
	y = ry / mapedit_view_cell(pApp, pWidget, 0);
	if ( x < 0 || y < 0 || x >= pApp->tMap.iWidth || y >= pApp->tMap.iHeight ) return 0;
	inCellX = rx - x * mapedit_view_cell(pApp, pWidget, 0);
	inCellY = ry - y * mapedit_view_cell(pApp, pWidget, 0);
	*pX = x;
	*pY = y;
	if ( pRegionCol != NULL ) *pRegionCol = mapedit_max_i(0, mapedit_min_i(2, inCellX * 3 / mapedit_view_cell(pApp, pWidget, 0)));
	if ( pRegionRow != NULL ) *pRegionRow = mapedit_max_i(0, mapedit_min_i(2, inCellY * 3 / mapedit_view_cell(pApp, pWidget, 0)));
	return 1;
}

static int mapedit_map_passage_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t wr;
	int x;
	int y;
	int regionCol;
	int regionRow;
	unsigned char value;
	unsigned char bit;
	char status[160];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( pApp->pMapPassageScrollFrame != NULL ) {
			mapedit_map_passage_scroll_by_wheel(pApp, pEvent);
		} else {
			float maxX = (float)mapedit_max_i(0, pApp->tMap.iWidth * MAPEDIT_MAP_PASSAGE_CELL - wr.fW);
			float maxY = (float)mapedit_max_i(0, pApp->tMap.iHeight * MAPEDIT_MAP_PASSAGE_CELL - wr.fH);
			pApp->fMapPassageScrollX = mapedit_clampf(pApp->fMapPassageScrollX - pEvent->fWheelX * 40.0f, 0.0f, maxX);
			pApp->fMapPassageScrollY = mapedit_clampf(pApp->fMapPassageScrollY - pEvent->fWheelY * 40.0f, 0.0f, maxY);
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_map_passage_hit(pApp, pWidget, pEvent, &x, &y, &regionCol, &regionRow) ) return XUI_EVENT_DISPATCH_STOP;
	value = mapedit_map_get_cell_passage(pApp, x, y);
	bit = mapedit_tileset_passage_bit_for_region(regionCol, regionRow);
	if ( bit == 0 ) value = (value == 0) ? 255 : 0;
	else value = (unsigned char)(value ^ bit);
	if (!mapedit_capture_map(pApp, MAPEDIT_SNAPSHOT_PASSAGE)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_map_set_cell_passage(pApp, x, y, value) != XUI_OK ) {
		mapedit_status(pApp, "地图通行修正写入失败");
		return XUI_EVENT_DISPATCH_STOP;
	}
	mapedit_mark_map_dirty(pApp);
	snprintf(status, sizeof(status), "地图通行修正已修改（未保存）: (%d,%d) = %u", x, y, (unsigned int)value);
	mapedit_status(pApp, status);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_map_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static int mapedit_tileset_panel_hit(mapedit_app_t* pApp, xui_widget pWidget, const xui_event_t* pEvent, int* pTile, int* pRegionCol, int* pRegionRow)
{
	xui_rect_t wr;
	int rx;
	int ry;
	int col;
	int row;
	int inCellX;
	int inCellY;
	float* pScrollX;
	float* pScrollY;
	float scrollX;
	float scrollY;
	int cellW;
	int cellH;
	if ( pApp == NULL || pWidget == NULL || pEvent == NULL || pTile == NULL ) return 0;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->fX < wr.fX || pEvent->fY < wr.fY || pEvent->fX >= wr.fX + wr.fW || pEvent->fY >= wr.fY + wr.fH ) return 0;
	mapedit_tileset_panel_scroll_ptrs(pApp, pWidget, &pScrollX, &pScrollY);
	scrollX = pScrollX != NULL ? *pScrollX : 0.0f;
	scrollY = pScrollY != NULL ? *pScrollY : 0.0f;
	cellW = mapedit_tileset_panel_cell_width(pApp, pWidget);
	cellH = mapedit_tileset_panel_cell_height(pApp, pWidget);
	rx = pEvent->fX - wr.fX + (int)floorf(scrollX);
	ry = pEvent->fY - wr.fY + (int)floorf(scrollY);
	if ( rx < 0 || ry < 0 ) return 0;
	col = rx / cellW;
	row = ry / cellH;
	inCellX = rx - col * cellW;
	inCellY = ry - row * cellH;
	if ( col < 0 || row < 0 || col >= MAPEDIT_TILES_PER_ROW || row >= mapedit_tile_select_row_count(pApp) ||
	     inCellX < 0 || inCellY < 0 || inCellX >= cellW || inCellY >= cellH ) return 0;
	*pTile = row * MAPEDIT_TILES_PER_ROW + col;
	if ( pRegionCol != NULL ) *pRegionCol = mapedit_max_i(0, mapedit_min_i(2, inCellX * 3 / cellW));
	if ( pRegionRow != NULL ) *pRegionRow = mapedit_max_i(0, mapedit_min_i(2, inCellY * 3 / cellH));
	return 1;
}

static int mapedit_tileset_panel_handle_wheel(mapedit_app_t* pApp, xui_widget pWidget, const xui_event_t* pEvent)
{
	xui_widget frame;
	float* pScrollX;
	float* pScrollY;
	float maxX;
	float maxY;
	if ( pApp == NULL || pWidget == NULL || pEvent == NULL || pEvent->iType != XUI_EVENT_POINTER_WHEEL ) return 0;
	frame = mapedit_tileset_panel_scroll_frame(pApp, pWidget);
	if ( frame != NULL ) {
		(void)xuiScrollFrameScrollBy(frame, -pEvent->fWheelX * 40.0f, -pEvent->fWheelY * 40.0f);
		mapedit_tileset_panel_scroll_sync_from_frame(pApp, pWidget);
		return 1;
	}
	mapedit_tileset_panel_scroll_ptrs(pApp, pWidget, &pScrollX, &pScrollY);
	if ( pScrollX == NULL || pScrollY == NULL ) return 0;
	mapedit_tileset_panel_max_scroll(pApp, pWidget, &maxX, &maxY);
	*pScrollX = mapedit_clampf(*pScrollX - pEvent->fWheelX * 40.0f, 0.0f, maxX);
	*pScrollY = mapedit_clampf(*pScrollY - pEvent->fWheelY * 40.0f, 0.0f, maxY);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return 1;
}

static void mapedit_tileset_panel_handle_hover(mapedit_app_t* pApp, xui_widget pWidget, const xui_event_t* pEvent)
{
	int tile;
	if ( pApp == NULL || pWidget == NULL || pEvent == NULL || mapedit_tileset_panel_hover_ptr(pApp, pWidget) == NULL ) return;
	if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE || pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		mapedit_tileset_panel_set_hover(pApp, pWidget, -1);
		return;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		if ( mapedit_tileset_panel_hit(pApp, pWidget, pEvent, &tile, NULL, NULL) ) mapedit_tileset_panel_set_hover(pApp, pWidget, tile);
		else mapedit_tileset_panel_set_hover(pApp, pWidget, -1);
	}
}

static int mapedit_tileset_arrange_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int tile;
	char status[256];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_tileset_panel_handle_wheel(pApp, pWidget, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	mapedit_tileset_panel_handle_hover(pApp, pWidget, pEvent);
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_tileset_panel_hit(pApp, pWidget, pEvent, &tile, NULL, NULL) ) return XUI_EVENT_DISPATCH_STOP;
	pApp->iTilesetArrangeSelectedTile = tile;
	pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
	pApp->iTilesetPropertyTile = -1;
	if ( tile == 0 ) {
		mapedit_status(pApp, "0 号橡皮图块固定，不参与编辑");
	} else if ( tile >= 1 && tile <= pApp->tTileset.iSpecialCount ) {
		snprintf(status, sizeof(status), "已选择特殊图块槽位: ID %d / 特殊槽 %d", tile, tile);
		pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL;
		pApp->iTilesetPropertyTile = tile;
		mapedit_status(pApp, status);
	} else if ( tile > pApp->tTileset.iSpecialCount && tile < mapedit_tile_static_start(pApp) ) {
		snprintf(status, sizeof(status), "ID %d 是对齐占位，不写入图集数据", tile);
		mapedit_status(pApp, status);
	} else if ( mapedit_tile_select_tile_selectable(pApp, tile) ) {
		snprintf(status, sizeof(status), "静态图块 ID %d，来自 %.200s", tile, pApp->tTileset.sStaticFile[0] ? pApp->tTileset.sStaticFile : "未设置");
		mapedit_status(pApp, status);
	} else {
		snprintf(status, sizeof(status), "ID %d 不是有效图块", tile);
		mapedit_status(pApp, status);
	}
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_tileset_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static int mapedit_tileset_passage_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int tile;
	int regionCol;
	int regionRow;
	unsigned char bit;
	unsigned char value;
	char status[128];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_tileset_panel_handle_wheel(pApp, pWidget, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_tileset_panel_hit(pApp, pWidget, pEvent, &tile, &regionCol, &regionRow) ) return XUI_EVENT_DISPATCH_STOP;
	if ( tile <= 0 || tile >= (int)sizeof(pApp->tTileset.arrPassage) || !mapedit_tile_select_tile_selectable(pApp, tile) ) {
		snprintf(status, sizeof(status), "ID %d 不是可编辑通行图块", tile);
		mapedit_status(pApp, status);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (!mapedit_capture_tileset(pApp)) return XUI_EVENT_DISPATCH_STOP;
	value = pApp->tTileset.arrPassage[tile];
	bit = mapedit_tileset_passage_bit_for_region(regionCol, regionRow);
	if ( bit == 0 ) pApp->tTileset.arrPassage[tile] = (value == 0) ? 255 : 0;
	else pApp->tTileset.arrPassage[tile] = (unsigned char)(value ^ bit);
	if ( pApp->tTileset.iPassageCount <= tile ) pApp->tTileset.iPassageCount = tile + 1;
	pApp->iTilesetPassageSelectedTile = tile;
	mapedit_mark_tileset_dirty(pApp);
	snprintf(status, sizeof(status), "通行数据已修改（未保存）: ID %d = %u", tile, (unsigned int)pApp->tTileset.arrPassage[tile]);
	mapedit_status(pApp, status);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_tileset_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static int mapedit_tileset_actor_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int tile;
	char status[128];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_tileset_panel_handle_wheel(pApp, pWidget, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_tileset_panel_hit(pApp, pWidget, pEvent, &tile, NULL, NULL) ) return XUI_EVENT_DISPATCH_STOP;
	if ( tile <= 0 || tile >= (int)sizeof(pApp->tTileset.arrActorOverlay) || !mapedit_tile_select_tile_selectable(pApp, tile) ) {
		snprintf(status, sizeof(status), "ID %d 不是可编辑角色覆盖图块", tile);
		mapedit_status(pApp, status);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if (!mapedit_capture_tileset(pApp)) return XUI_EVENT_DISPATCH_STOP;
	pApp->tTileset.arrActorOverlay[tile] = (unsigned char)(pApp->tTileset.arrActorOverlay[tile] ? 0 : 1);
	if ( pApp->tTileset.iActorOverlayCount <= tile ) pApp->tTileset.iActorOverlayCount = tile + 1;
	pApp->iTilesetActorSelectedTile = tile;
	mapedit_mark_tileset_dirty(pApp);
	snprintf(status, sizeof(status), "角色覆盖数据已修改（未保存）: ID %d = %s", tile, pApp->tTileset.arrActorOverlay[tile] ? "覆盖" : "不覆盖");
	mapedit_status(pApp, status);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_tileset_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static int mapedit_tileset_tags_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	mapedit_custom_channel_def_t* pDef;
	int tile;
	char status[192];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_tileset_panel_handle_wheel(pApp, pWidget, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	mapedit_tileset_panel_handle_hover(pApp, pWidget, pEvent);
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_tileset_panel_hit(pApp, pWidget, pEvent, &tile, NULL, NULL) ) return XUI_EVENT_DISPATCH_STOP;
	pApp->iTilesetTagsSelectedTile = tile;
	pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM;
	pApp->iTilesetPropertyTile = tile;
	if ( !mapedit_tile_select_tile_selectable(pApp, tile) ) {
		mapedit_status(pApp, "该位置没有可打标图块");
		mapedit_refresh_tileset_properties(pApp);
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pApp->bTilesetTagsInspectMode ) {
		snprintf(status, sizeof(status), "正在检查图块自定义数据: ID %d", tile);
		mapedit_status(pApp, status);
		mapedit_refresh_tileset_properties(pApp);
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	pDef = mapedit_custom_channel(pApp, pApp->iTilesetTagChannel);
	if ( pDef == NULL ) {
		mapedit_status(pApp, "setup.xson 中没有可写入的图块 customData 通道");
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pApp->sTilesetTagValue[0] == 0 && pDef->sDefaultValue[0] != 0 ) mapedit_tags_use_default_value(pApp, 0);
	if (!mapedit_capture_tileset(pApp)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_custom_set_value(&pApp->tTileset.pTileCustomRaw, pDef->sId, SET_KEY_TILE, tile, pApp->sTilesetTagValue) != XUI_OK ) {
		mapedit_status(pApp, "图块打标失败");
		return XUI_EVENT_DISPATCH_STOP;
	}
	mapedit_mark_tileset_dirty(pApp);
	snprintf(status, sizeof(status), "图块打标已修改（未保存）: ID %d / %s = %s", tile, pDef->sId, pApp->sTilesetTagValue);
	mapedit_status(pApp, status);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_tileset_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static int mapedit_map_tags_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	mapedit_custom_channel_def_t* pDef;
	xui_rect_t wr;
	int x;
	int y;
	int cell;
	char status[192];
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	if (mapedit_view_interact(pApp, pWidget, pEvent)) return XUI_EVENT_DISPATCH_STOP;
	wr = xuiWidgetGetWorldRect(pWidget);
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( pApp->pMapTagsScrollFrame != NULL ) {
			mapedit_map_tags_scroll_by_wheel(pApp, pEvent);
		} else {
			float maxX = (float)mapedit_max_i(0, pApp->tMap.iWidth * MAPEDIT_TILE_W - wr.fW);
			float maxY = (float)mapedit_max_i(0, pApp->tMap.iHeight * MAPEDIT_TILE_H - wr.fH);
			pApp->fMapTagsScrollX = mapedit_clampf(pApp->fMapTagsScrollX - pEvent->fWheelX * 40.0f, 0.0f, maxX);
			pApp->fMapTagsScrollY = mapedit_clampf(pApp->fMapTagsScrollY - pEvent->fWheelY * 40.0f, 0.0f, maxY);
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_LEAVE ) {
		mapedit_set_map_tags_hover(pApp, -1, -1);
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE ) {
		if ( mapedit_pointer_to_map_tile_on_widget(pApp, pWidget, pEvent->fX, pEvent->fY, &x, &y) ) mapedit_set_map_tags_hover(pApp, x, y);
		else mapedit_set_map_tags_hover(pApp, -1, -1);
		return XUI_OK;
	}
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !mapedit_pointer_to_map_tile_on_widget(pApp, pWidget, pEvent->fX, pEvent->fY, &x, &y) ) return XUI_EVENT_DISPATCH_STOP;
	mapedit_set_map_tags_hover(pApp, x, y);
	cell = mapedit_map_cell_id(&pApp->tMap, x, y);
	if ( cell < 0 ) {
		mapedit_status(pApp, "该位置不在地图范围内");
		return XUI_EVENT_DISPATCH_STOP;
	}
	pApp->iMapTagsSelectedCell = cell;
	if ( pApp->bMapTagsInspectMode ) {
		pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_CELL_CUSTOM;
		pApp->iMapPropertyCell = cell;
		snprintf(status, sizeof(status), "正在检查地图位置自定义数据: (%d,%d)", x, y);
		mapedit_status(pApp, status);
		mapedit_refresh_map_properties(pApp);
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	pDef = mapedit_custom_channel(pApp, pApp->iMapTagChannel);
	if ( pDef == NULL ) {
		mapedit_status(pApp, "setup.xson 中没有可写入的地图 customData 通道");
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pApp->sMapTagValue[0] == 0 && pDef->sDefaultValue[0] != 0 ) mapedit_tags_use_default_value(pApp, 1);
	if (!mapedit_capture_map(pApp, MAPEDIT_SNAPSHOT_CUSTOM)) return XUI_EVENT_DISPATCH_STOP;
	if ( mapedit_map_set_cell_custom_value(pApp, pDef, x, y, pApp->sMapTagValue) != XUI_OK ) {
		mapedit_status(pApp, "地图打标写入失败");
		return XUI_EVENT_DISPATCH_STOP;
	}
	mapedit_mark_map_dirty(pApp);
	snprintf(status, sizeof(status), "地图打标已修改（未保存）: (%d,%d) / %s = %s", x, y, pDef->sId, pApp->sMapTagValue);
	pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_MAP;
	pApp->iMapPropertyCell = -1;
	mapedit_status(pApp, status);
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_refresh_map_properties(pApp);
	return XUI_EVENT_DISPATCH_STOP;
}

static xui_widget mapedit_view_widget(mapedit_app_t* pApp, int index, xui_widget* frame)
{
	xui_widget widgets[] = {pApp->pMapCanvas, pApp->pMapPassageCanvas, pApp->pMapTagsCanvas, pApp->pTileSelectCanvas,
		pApp->pTilesetArrangeCanvas, pApp->pTilesetPassageCanvas, pApp->pTilesetActorCanvas, pApp->pTilesetTagsCanvas};
	xui_widget frames[] = {pApp->pMapScrollFrame, pApp->pMapPassageScrollFrame, pApp->pMapTagsScrollFrame, pApp->pTileSelectScrollFrame,
		pApp->pTilesetArrangeScrollFrame, pApp->pTilesetPassageScrollFrame, pApp->pTilesetActorScrollFrame, pApp->pTilesetTagsScrollFrame};
	if (index < 0 || index >= 8) index = 0;
	if (frame) *frame = frames[index];
	return widgets[index];
}

static void mapedit_view_zoom_at(mapedit_app_t* pApp, xui_widget widget, float zoom, float x, float y)
{
	int index = mapedit_view_index(pApp, widget);
	int oldW = mapedit_view_cell(pApp, widget, 0), oldH = mapedit_view_cell(pApp, widget, 1);
	float sx = 0.0f, sy = 0.0f;
	xui_widget frame;
	char status[128];
	(void)mapedit_view_widget(pApp, index, &frame);
	if (pApp->bPainting || !frame) return;
	(void)xuiScrollFrameGetOffset(frame, &sx, &sy);
	pApp->arrViewZoom[index] = mapeditViewZoom(zoom);
	mapedit_map_workspace_scroll_sync_content(pApp);
	mapedit_tileset_workspace_scroll_sync_content(pApp);
	(void)xuiScrollFrameSetOffset(frame,
		mapeditViewAnchor(sx, x, oldW, mapedit_view_cell(pApp, widget, 0)),
		mapeditViewAnchor(sy, y, oldH, mapedit_view_cell(pApp, widget, 1)));
	pApp->iLastView = index;
	(void)xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	snprintf(status, sizeof(status), "缩放 %.0f%% · Ctrl+滚轮缩放 / 中键拖动画布", (double)pApp->arrViewZoom[index] * 100.0);
	mapedit_status(pApp, status);
}

static int mapedit_view_interact(mapedit_app_t* pApp, xui_widget widget, const xui_event_t* event)
{
	xui_widget frame;
	int index = mapedit_view_index(pApp, widget);
	if (event->iType == XUI_EVENT_POINTER_DOWN || event->iType == XUI_EVENT_POINTER_WHEEL) pApp->iLastView = index;
	if (event->iType == XUI_EVENT_POINTER_WHEEL && (event->iModifiers & XUI_MOD_CTRL)) {
		xui_rect_t rect = xuiWidgetGetWorldRect(widget);
		float factor = event->fWheelY > 0.0f ? 1.25f : 0.8f;
		mapedit_view_zoom_at(pApp, widget, mapeditViewZoom(pApp->arrViewZoom[index]) * factor,
			(float)(event->fX - rect.fX), (float)(event->fY - rect.fY));
		return 1;
	}
	if (event->iType == XUI_EVENT_POINTER_DOWN && event->iButton == XUI_POINTER_BUTTON_MIDDLE && !pApp->bPainting) {
		pApp->bViewPanning = 1; pApp->pPanWidget = widget; pApp->iPanView = index;
		pApp->iPanX = event->fX; pApp->iPanY = event->fY;
		(void)xuiSetPointerCapture(pApp->pContext, widget);
		return 1;
	}
	if (!pApp->bViewPanning || widget != pApp->pPanWidget) return 0;
	(void)mapedit_view_widget(pApp, index, &frame);
	if (event->iType == XUI_EVENT_POINTER_MOVE) {
		(void)xuiScrollFrameScrollBy(frame, (float)(pApp->iPanX - event->fX), (float)(pApp->iPanY - event->fY));
		pApp->iPanX = event->fX; pApp->iPanY = event->fY;
	} else if (event->iType == XUI_EVENT_POINTER_UP || event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) {
		pApp->bViewPanning = 0; pApp->pPanWidget = NULL;
		if (xuiGetPointerCapture(pApp->pContext) == widget) (void)xuiReleasePointerCapture(pApp->pContext, widget);
	}
	return 1;
}

static void mapedit_zoom_command(mapedit_app_t* pApp, int fit)
{
	xui_widget widget;
	xui_rect_t rect;
	int index = pApp->iLastView;
	int width, height, baseW, baseH;
	float zoom = 1.0f;
	if (pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_MAP && index >= 4) index = 0;
	if (pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_TILESET && index < 4) index = 4;
	widget = mapedit_view_widget(pApp, index, NULL);
	rect = xuiWidgetGetWorldRect(widget);
	if (fit) {
		width = index < 3 ? pApp->tMap.iWidth : MAPEDIT_TILES_PER_ROW;
		height = index < 3 ? pApp->tMap.iHeight : mapedit_tile_select_row_count(pApp);
		baseW = index == 1 || index == 5 ? 32 : MAPEDIT_TILE_W;
		baseH = index == 1 || index == 5 ? 32 : MAPEDIT_TILE_H;
		if (width > 0 && height > 0)
			zoom = fminf((float)rect.fW / (float)(width * baseW), (float)rect.fH / (float)(height * baseH));
	}
	mapedit_view_zoom_at(pApp, widget, zoom, (float)rect.fW * 0.5f, (float)rect.fH * 0.5f);
}

static void mapedit_register_view_gestures(mapedit_app_t* pApp)
{
	xui_widget_event_proc handlers[] = {mapedit_map_event, mapedit_map_passage_event, mapedit_map_tags_event, mapedit_tile_select_event,
		mapedit_tileset_arrange_event, mapedit_tileset_passage_event, mapedit_tileset_actor_event, mapedit_tileset_tags_event};
	int events[] = {XUI_EVENT_POINTER_DOWN, XUI_EVENT_POINTER_UP, XUI_EVENT_POINTER_MOVE, XUI_EVENT_POINTER_WHEEL, XUI_EVENT_POINTER_CAPTURE_LOST};
	int i, j;
	for (i = 0; i < 8; ++i) {
		xui_widget widget = mapedit_view_widget(pApp, i, NULL);
		for (j = 0; j < 5; ++j) (void)xuiWidgetSetEventHandler(widget, events[j], handlers[i], pApp);
	}
}

static void mapedit_select_workspace(mapedit_app_t* pApp, int iWorkspace)
{
	if ( pApp == NULL ) return;
	pApp->iActiveWorkspace = iWorkspace;
	mapedit_refresh_toolbar_state(pApp);
	if ( pApp->pTilesetDock != NULL ) (void)xuiWidgetSetVisible(pApp->pTilesetDock, iWorkspace == MAPEDIT_WORKSPACE_TILESET);
	if ( pApp->pMapDock != NULL ) (void)xuiWidgetSetVisible(pApp->pMapDock, iWorkspace == MAPEDIT_WORKSPACE_MAP);
	if ( pApp->pTileButton != NULL ) (void)xuiButtonSetSelected(pApp->pTileButton, iWorkspace == MAPEDIT_WORKSPACE_TILESET);
	if ( pApp->pMapButton != NULL ) (void)xuiButtonSetSelected(pApp->pMapButton, iWorkspace == MAPEDIT_WORKSPACE_MAP);
	mapedit_status(pApp, iWorkspace == MAPEDIT_WORKSPACE_TILESET ? "当前工作区: 图块编辑器" : "当前工作区: 地图编辑器");
	mapedit_update_window_title(pApp);
}

static void mapedit_workspace_click(xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int target;
	if ( pApp == NULL ) return;
	target = pWidget == pApp->pTileButton ? MAPEDIT_WORKSPACE_TILESET : MAPEDIT_WORKSPACE_MAP;
	mapedit_select_workspace(pApp, target);
}

static void mapedit_toolbar_select(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	(void)iIndex;
	if ( pApp == NULL ) return;
	if ( iValue >= CMD_TOOL_BRUSH && iValue <= CMD_TOOL_BUCKET ) {
		char status[96];
		pApp->iActiveTool = iValue - CMD_TOOL_BRUSH;
		snprintf(status, sizeof(status), "地图编辑工具: %s", mapedit_tool_name(pApp->iActiveTool));
		mapedit_status(pApp, status);
	}
	if ( iValue == CMD_GRID ) {
		pApp->bGrid = !pApp->bGrid;
		mapedit_status(pApp, pApp->bGrid ? "已显示地图网格" : "已隐藏地图网格");
	}
	if ( iValue == CMD_PREVIEW ) {
		pApp->bPreview = !pApp->bPreview;
		pApp->fPreviewAnimTime = 0.0f;
		pApp->iPreviewAnimFrame = 0;
		mapedit_status(pApp, pApp->bPreview ? "预览模式已开启" : "预览模式已关闭");
	}
	if ( iValue == CMD_UNDO ) mapedit_undo(pApp);
	if ( iValue == CMD_REDO ) mapedit_redo(pApp);
	mapedit_refresh_toolbar_state(pApp);
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_layer_select(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	char status[128];
	(void)pWidget;
	(void)iValue;
	if ( pApp == NULL ) return;
	if ( iIndex < 0 || iIndex >= pApp->iSetupLayerCount || iIndex >= pApp->tMap.iLayers ) return;
	pApp->iActiveLayer = iIndex;
	if ( pApp->arrSetupLayers[iIndex].sName[0] != 0 ) snprintf(status, sizeof(status), "当前地图图层: %s", pApp->arrSetupLayers[iIndex].sName);
	else snprintf(status, sizeof(status), "当前地图图层: %d", pApp->iActiveLayer + 1);
	mapedit_status(pApp, status);
	mapedit_update_status_details(pApp);
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static const char* mapedit_path_file_name(const char* sPath)
{
	const char* pBack;
	const char* pSlash;
	if ( sPath == NULL ) return "";
	pBack = strrchr(sPath, '\\');
	pSlash = strrchr(sPath, '/');
	if ( pSlash != NULL && (pBack == NULL || pSlash > pBack) ) pBack = pSlash;
	return pBack != NULL ? pBack + 1 : sPath;
}

static int mapedit_paths_equal(const char* sLeft, const char* sRight)
{
	wchar_t left[MAPEDIT_PATH_MAX];
	wchar_t right[MAPEDIT_PATH_MAX];
	wchar_t leftFull[MAPEDIT_PATH_MAX];
	wchar_t rightFull[MAPEDIT_PATH_MAX];
	DWORD leftSize;
	DWORD rightSize;
	if ( sLeft == NULL || sRight == NULL ||
	     !mapedit_utf8_to_wide(sLeft, left, (int)(sizeof(left) / sizeof(left[0]))) ||
	     !mapedit_utf8_to_wide(sRight, right, (int)(sizeof(right) / sizeof(right[0]))) ) return 0;
	leftSize = GetFullPathNameW(left, (DWORD)(sizeof(leftFull) / sizeof(leftFull[0])), leftFull, NULL);
	rightSize = GetFullPathNameW(right, (DWORD)(sizeof(rightFull) / sizeof(rightFull[0])), rightFull, NULL);
	if ( leftSize == 0 || leftSize >= (DWORD)(sizeof(leftFull) / sizeof(leftFull[0])) ||
	     rightSize == 0 || rightSize >= (DWORD)(sizeof(rightFull) / sizeof(rightFull[0])) ) return 0;
	return _wcsicmp(leftFull, rightFull) == 0;
}

static void mapedit_rescan_map_list(mapedit_app_t* pApp, const char* sSelectedPath)
{
	char dir[MAPEDIT_PATH_MAX];
	char candidate[MAPEDIT_PATH_MAX];
	const char* sFile;
	if ( pApp == NULL ) return;
	mapedit_app_path(pApp, dir, sizeof(dir), "assets\\maps");
	mapedit_scan_files(dir, ".xson", &pApp->tMapFiles);
	mapedit_refresh_map_list_texts(pApp, &pApp->tMapFiles);
	pApp->tMapFiles.iSelected = -1;
	if ( sSelectedPath != NULL && sSelectedPath[0] != 0 ) {
		sFile = mapedit_path_file_name(sSelectedPath);
		mapedit_path_join(candidate, sizeof(candidate), dir, sFile);
		if ( mapedit_paths_equal(candidate, sSelectedPath) ) {
			pApp->tMapFiles.iSelected = mapedit_file_list_find(&pApp->tMapFiles, sFile);
		}
	}
	if ( pApp->pMapListView != NULL ) {
		(void)xuiListViewSetItems(pApp->pMapListView, pApp->tMapFiles.arrTextPtrs, pApp->tMapFiles.iCount);
		(void)xuiListViewSetSelected(pApp->pMapListView, pApp->tMapFiles.iSelected);
	}
}

static void mapedit_recovery_path_for(mapedit_app_t* pApp, int workspace, const char* documentPath, char* path, int capacity)
{
	char name[96];
	snprintf(name, sizeof(name), "recovery-%016llx-%s-%016llx.xson",
		(unsigned long long)mapeditPathKey(pApp->sProjectDir),
		workspace == MAPEDIT_WORKSPACE_MAP ? "map" : "tileset",
		(unsigned long long)mapeditPathKey(documentPath && *documentPath ? documentPath :
			(workspace == MAPEDIT_WORKSPACE_MAP ? "untitled-map" : "untitled-tileset")));
	mapedit_path_join(path, capacity, pApp->sUserDir, name);
}

static void mapedit_recovery_path(mapedit_app_t* pApp, char* path, int capacity)
{
	int workspace = pApp->iActiveWorkspace;
	mapedit_recovery_path_for(pApp, workspace, workspace == MAPEDIT_WORKSPACE_MAP ? pApp->tMap.sPath : pApp->tTileset.sPath, path, capacity);
}

static void mapedit_clear_recovery_for(mapedit_app_t* pApp, int workspace, const char* sourcePath)
{
	char path[MAPEDIT_PATH_MAX];
	mapedit_recovery_path_for(pApp, workspace, sourcePath, path, sizeof(path));
	(void)mapedit_delete_file_utf8(path);
}

static int mapedit_write_recovery_now(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	int ret = XUI_OK;
	if (pApp->tMap.bDirty && pApp->tMap.pTiles) {
		/* Save changes identity/schema flags only: preserve the live document. */
		xge_map_t copy = pApp->tMap;
		mapedit_recovery_path_for(pApp, MAPEDIT_WORKSPACE_MAP, copy.sPath, path, sizeof(path));
		ret = mapedit_map_save(&copy, path);
	}
	if (pApp->tTileset.bDirty && pApp->tTileset.bLoaded) {
		mapedit_recovery_path_for(pApp, MAPEDIT_WORKSPACE_TILESET, pApp->tTileset.sPath, path, sizeof(path));
		if (mapedit_tileset_save_to(pApp, path, 0) != XUI_OK) ret = XUI_ERROR;
	}
	if (ret != XUI_OK) mapedit_status(pApp, "自动恢复文件写入失败，请立即手动保存");
	return ret;
}

static void mapedit_maybe_write_recovery(mapedit_app_t* pApp)
{
	double now;
	double delay;
	if ( pApp == NULL || (!pApp->tMap.bDirty && !pApp->tTileset.bDirty) ) return;
	now = xgeTimer();
	if ( pApp->fMapDirtySince <= 0.0 ) pApp->fMapDirtySince = now;
	delay = (pApp->fLastRecoveryWrite > 0.0 ? 30.0 : 2.0) -
		(now - (pApp->fLastRecoveryWrite > 0.0 ? pApp->fLastRecoveryWrite : pApp->fMapDirtySince));
	if ( delay > 0.0 ) {
		xgeRenderRequestAfter((float)delay);
		return;
	}
	pApp->fLastRecoveryWrite = now;
	(void)mapedit_write_recovery_now(pApp);
	xgeRenderRequestAfter(30.0f);
}

static void mapedit_recover_map(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	mapedit_recovery_path(pApp, path, sizeof(path));
	if (!mapedit_file_exists_utf8(path)) { mapedit_status(pApp, "当前文档没有可恢复的自动保存文件"); return; }
	mapedit_request_operation(pApp, pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_MAP ? 4 : 5, path);
}

static int mapedit_save_map_to(mapedit_app_t* pApp, const char* sPath)
{
	char status[sizeof(gMapeditMapIoError) + 48];
	if ( pApp == NULL || sPath == NULL || sPath[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	char oldPath[MAPEDIT_PATH_MAX];
	mapedit_copy_text(oldPath, sizeof(oldPath), pApp->tMap.sPath);
	if ( mapedit_map_save(&pApp->tMap, sPath) != XUI_OK ) {
		snprintf(status, sizeof(status), "地图保存失败: %s", gMapeditMapIoError[0] != 0 ? gMapeditMapIoError : "未知错误");
		mapedit_status(pApp, status);
		return XUI_ERROR;
	}
	pApp->fMapDirtySince = 0.0;
	pApp->fLastRecoveryWrite = 0.0;
	mapedit_clear_recovery_for(pApp, MAPEDIT_WORKSPACE_MAP, oldPath);
	mapeditHistorySaved(&pApp->tMapHistory);
	pApp->tMapStamp = mapeditFileStamp(pApp->tMap.sPath);
	pApp->bMapExternalConflict = 0;
	mapedit_rescan_map_list(pApp, sPath);
	mapedit_update_window_title(pApp);
	mapedit_status(pApp, "地图已保存");
	return XUI_OK;
}

static void mapedit_file_dialog_result(xui_file_dialog_t* pDialog, int iResult, const char* sPath, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int command = pApp->iFileDialogCommand;
	int workspace = pApp->iFileDialogWorkspace;
	int ret = XUI_ERROR;
	(void)pDialog;
	pApp->iFileDialogCommand = 0;
	if (iResult != XUI_FILE_DIALOG_RESULT_OK || !sPath || !*sPath) {
		pApp->iPendingOperation = 0;
		mapedit_status(pApp, "文件操作已取消，文档保持不变");
		return;
	}
	if (command == CMD_FILE_OPEN) {
		mapedit_request_operation(pApp, workspace == MAPEDIT_WORKSPACE_MAP ? 2 : 3, sPath);
	} else if (command == CMD_FILE_SAVE_AS) {
		ret = workspace == MAPEDIT_WORKSPACE_MAP ? mapedit_save_map_to(pApp, sPath) : mapedit_tileset_save_to(pApp, sPath, 1);
		if (ret == XUI_OK) {
			mapedit_status(pApp, "文档已另存");
			mapedit_pending_continue(pApp);
		} else {
			pApp->iPendingOperation = 0;
			mapedit_status(pApp, "保存失败，未关闭或替换当前文档");
		}
	} else if (command == CMD_MATERIAL_SOURCE_IMAGE) (void)mapedit_material_edit_load_source(pApp, sPath);
}

static int mapedit_show_file_dialog(mapedit_app_t* pApp, int iCommand, const char* sTitle,
	const char* sInitialDir, const char* sFileName, const char* sFilter, int bSave)
{
	xui_file_dialog_desc_t desc;
	int ret;
	if ( pApp == NULL || pApp->pContext == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pFileDialog != NULL ) {
		if ( xuiFileDialogIsOpen(pApp->pFileDialog) ) return XUI_OK;
		xuiFileDialogDestroy(pApp->pFileDialog);
		pApp->pFileDialog = NULL;
	}
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.sTitle = sTitle;
	desc.sInitialDir = sInitialDir;
	desc.sFileName = sFileName;
	desc.sFilter = sFilter;
	desc.pFont = pApp->pFont;
	desc.onResult = mapedit_file_dialog_result;
	desc.pResultUser = pApp;
	desc.bModal = 1;
	pApp->iFileDialogCommand = iCommand;
	pApp->iFileDialogWorkspace = pApp->iActiveWorkspace;
	if ( bSave ) ret = xuiSaveFileDialog(pApp->pContext, &pApp->pFileDialog, &desc);
	else ret = xuiOpenFileDialog(pApp->pContext, &pApp->pFileDialog, &desc);
	if ( ret != XUI_OK ) pApp->iFileDialogCommand = 0;
	return ret;
}

static int mapedit_show_map_file_dialog(mapedit_app_t* pApp, int iCommand)
{
	char dir[MAPEDIT_PATH_MAX];
	int map = pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_MAP;
	const char* path = map ? pApp->tMap.sPath : pApp->tTileset.sPath;
	mapedit_app_path(pApp, dir, sizeof(dir), map ? "assets\\maps" : "assets\\图块集");
	return mapedit_show_file_dialog(pApp, iCommand,
		iCommand == CMD_FILE_OPEN ? (map ? "打开地图" : "打开图集") : (map ? "地图另存为" : "图集另存为"),
		dir, iCommand == CMD_FILE_SAVE_AS ? mapedit_path_file_name(path) : "",
		map ? "XGE 地图 (*.xson)|*.xson" : "XGE 图集 (*.xson)|*.xson", iCommand == CMD_FILE_SAVE_AS);
}

static int mapedit_show_material_source_dialog(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return mapedit_show_file_dialog(pApp, CMD_MATERIAL_SOURCE_IMAGE, "打开外部图片",
		pApp->sProjectDir, "", "图片文件|*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.webp|所有文件 (*.*)|*.*", 0);
}

static void mapedit_execute_command(mapedit_app_t* pApp, int iValue)
{
	if ( pApp == NULL ) return;
	if ( iValue == CMD_FILE_OPEN ) {
		if (mapedit_show_map_file_dialog(pApp, CMD_FILE_OPEN) != XUI_OK) mapedit_status(pApp, "无法打开文件选择器");
	} else if ( iValue == CMD_FILE_SAVE ) {
		(void)mapedit_save_current(pApp);
	} else if ( iValue == CMD_FILE_SAVE_AS ) {
		if ( mapedit_show_map_file_dialog(pApp, CMD_FILE_SAVE_AS) != XUI_OK ) mapedit_status(pApp, "无法打开另存为对话框");
	} else if ( iValue == CMD_FILE_RECOVER ) {
		mapedit_recover_map(pApp);
	} else if ( iValue == CMD_FILE_EXIT ) {
		mapedit_request_operation(pApp, 1, NULL);
	} else if (iValue == CMD_VIEW_ACTUAL_SIZE || iValue == CMD_VIEW_FIT) {
		mapedit_zoom_command(pApp, iValue == CMD_VIEW_FIT);
	} else if ( iValue == CMD_VIEW_RESET_LAYOUT ) {
		mapedit_map_scroll_reset(pApp);
		mapedit_map_passage_scroll_reset(pApp);
		mapedit_map_tags_scroll_reset(pApp);
		mapedit_tile_select_scroll_reset(pApp);
		mapedit_reset_tileset_panel_scrolls(pApp);
		if ( mapedit_load_default_layouts(pApp) == XUI_OK ) {
			mapedit_save_layouts(pApp);
			mapedit_status(pApp, "已重置并保存默认 DockPanel 布局");
		} else mapedit_status(pApp, "重置 DockPanel 布局失败");
	} else if ( iValue == CMD_HELP_ABOUT ) {
		mapedit_status(pApp, "XGE MapEdit 2 - V2 地图编辑器");
	} else if ( iValue == CMD_UNDO || iValue == CMD_REDO ) {
		if ( iValue == CMD_UNDO ) mapedit_undo(pApp);
		else mapedit_redo(pApp);
	} else {
		mapedit_status(pApp, "未知菜单命令");
	}
}

static void mapedit_menu_select(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	(void)iIndex;
	mapedit_execute_command(pApp, iValue);
}

static void mapedit_activate_loaded_map(mapedit_app_t* pApp, const char* sStatus)
{
	int tilesetRet;
	if ( pApp == NULL ) return;
	pApp->tMap.iState = mapedit_setup_clamp_state(pApp, pApp->tMap.iState);
	pApp->iActiveLayer = mapedit_max_i(0, mapedit_min_i(pApp->iActiveLayer, pApp->tMap.iLayers - 1));
	if ( pApp->pLayerCombo != NULL ) (void)xuiComboBoxSetSelected(pApp->pLayerCombo, pApp->iActiveLayer);
	mapedit_clear_history(pApp);
	if ( pApp->tMap.sTileset[0] != 0 ) {
		tilesetRet = mapedit_tileset_load(pApp, pApp->tMap.sTileset);
		if ( tilesetRet != XUI_OK ) mapedit_tileset_clear(pApp);
	} else {
		mapedit_tileset_clear(pApp);
		tilesetRet = XUI_OK;
	}
	mapedit_map_scroll_reset(pApp);
	mapedit_map_passage_scroll_reset(pApp);
	mapedit_map_tags_scroll_reset(pApp);
	mapedit_tile_select_scroll_reset(pApp);
	mapedit_reset_tileset_panel_scrolls(pApp);
	pApp->iMapPassageSelectedCell = -1;
	pApp->iMapTagsSelectedCell = -1;
	pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_MAP;
	pApp->iMapPropertyCell = -1;
	pApp->iMapHoverX = -1;
	pApp->iMapHoverY = -1;
	pApp->iMapTagsHoverX = -1;
	pApp->iMapTagsHoverY = -1;
	pApp->iTilesetArrangeHoverTile = -1;
	pApp->iTilesetTagsHoverTile = -1;
	pApp->iDragCurrentX = -1;
	pApp->iDragCurrentY = -1;
	pApp->fMapDirtySince = 0.0;
	pApp->tMapStamp = mapeditFileStamp(pApp->tMap.sPath);
	pApp->bMapExternalConflict = 0;
	pApp->bMapOK = 1;
	if ( tilesetRet == XUI_OK ) mapedit_status(pApp, sStatus != NULL ? sStatus : "已切换地图");
	else mapedit_status(pApp, "地图已载入，但引用的图集无法读取");
	mapedit_update_window_title(pApp);
	mapedit_update_status_details(pApp);
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_tags_update_channel_combo(pApp, 1);
	mapedit_refresh_map_properties(pApp);
}

static void mapedit_map_selected(xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	char rel[MAPEDIT_PATH_MAX], path[MAPEDIT_PATH_MAX];
	(void)pWidget;
	if (!pApp || iIndex < 0 || iIndex >= pApp->tMapFiles.iCount) return;
	snprintf(rel, sizeof(rel), "assets\\maps\\%s", pApp->tMapFiles.arrNames[iIndex]);
	mapedit_app_path(pApp, path, sizeof(path), rel);
	if (mapedit_paths_equal(path, pApp->tMap.sPath)) return;
	if (pApp->pMapListView) (void)xuiListViewSetSelected(pApp->pMapListView, pApp->tMapFiles.iSelected);
	mapedit_request_operation(pApp, 2, path);
}

static int mapedit_open_tileset(mapedit_app_t* pApp, const char* path)
{
	if (mapedit_tileset_load(pApp, path) != XUI_OK) {
		mapedit_status(pApp, "图集文件格式无效，已保留当前图集");
		return XUI_ERROR;
	}
	mapedit_copy_text(pApp->tTileset.sFile, sizeof(pApp->tTileset.sFile), mapedit_path_file_name(path));
	pApp->tTilesetFiles.iSelected = mapedit_file_list_find(&pApp->tTilesetFiles, pApp->tTileset.sFile);
	if (pApp->pTilesetListView) (void)xuiListViewSetSelected(pApp->pTilesetListView, pApp->tTilesetFiles.iSelected);
		pApp->iTilesetArrangeSelectedTile = -1;
		pApp->iTilesetPassageSelectedTile = -1;
		pApp->iTilesetActorSelectedTile = -1;
		pApp->iTilesetTagsSelectedTile = -1;
		pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
		pApp->iTilesetPropertyTile = -1;
		pApp->iSelectedTile = -1;
		pApp->iBrushW = 1;
		pApp->iBrushH = 1;
		pApp->iTileSelectHoverCol = -1;
		pApp->iTileSelectHoverRow = -1;
		pApp->iTilesetArrangeHoverTile = -1;
		pApp->iTilesetTagsHoverTile = -1;
		mapedit_tile_select_scroll_reset(pApp);
		mapedit_reset_tileset_panel_scrolls(pApp);
		mapedit_status(pApp, "已选择图集");
		pApp->bTilesetOK = 1;
		mapedit_update_status_details(pApp);
		if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( pApp->pTilesetArrangeCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetArrangeCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( pApp->pTilesetPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( pApp->pTilesetActorCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetActorCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( pApp->pTilesetTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		mapedit_tags_update_channel_combo(pApp, 0);
		mapedit_refresh_tileset_properties(pApp);
	mapedit_invalidate_map_views(pApp);
	mapedit_update_window_title(pApp);
	return XUI_OK;
}

static void mapedit_tileset_selected(xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	char rel[MAPEDIT_PATH_MAX], path[MAPEDIT_PATH_MAX];
	(void)pWidget;
	if (!pApp || iIndex < 0 || iIndex >= pApp->tTilesetFiles.iCount) return;
	snprintf(rel, sizeof(rel), "assets\\图块集\\%s", pApp->tTilesetFiles.arrNames[iIndex]);
	mapedit_app_path(pApp, path, sizeof(path), rel);
	if (mapedit_paths_equal(path, pApp->tTileset.sPath)) return;
	if (pApp->pTilesetListView) (void)xuiListViewSetSelected(pApp->pTilesetListView, pApp->tTilesetFiles.iSelected);
	mapedit_request_operation(pApp, 3, path);
}

static int mapedit_save_current(mapedit_app_t* pApp)
{
	int map = pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_MAP;
	const char* path = map ? pApp->tMap.sPath : pApp->tTileset.sPath;
	mapedit_file_stamp_t baseline = map ? pApp->tMapStamp : pApp->tTilesetStamp;
	int ret;
	if (!*path) return mapedit_show_map_file_dialog(pApp, CMD_FILE_SAVE_AS) == XUI_OK ? 1 : XUI_ERROR;
	if (!mapeditFileStampEqual(baseline, mapeditFileStamp(path)) ||
		(map ? pApp->bMapExternalConflict : pApp->bTilesetExternalConflict)) {
		mapedit_status(pApp, "磁盘文件已变化；请重新打开或另存为，避免覆盖外部修改");
		return XUI_ERROR;
	}
	ret = map ? mapedit_save_map_to(pApp, path) : mapedit_tileset_save_to(pApp, path, 1);
	mapedit_status(pApp, ret == XUI_OK ? "当前文档已保存" : "保存失败，文档保持打开");
	return ret;
}

static void mapedit_confirm_result(xui_msgbox_t* box, int result, void* user)
{
	mapedit_app_t* pApp = (mapedit_app_t*)user;
	int ret;
	(void)box;
	if (!pApp->iPendingOperation) return;
	if (result == XUI_MSGBOX_RESULT_CANCEL || result == XUI_MSGBOX_RESULT_CLOSE) {
		pApp->iPendingOperation = 0;
		mapedit_status(pApp, "已取消；所有修改仍保留");
		return;
	}
	if (result == XUI_MSGBOX_RESULT_NO) {
		pApp->iPendingDiscardMask |= 1 << pApp->iPendingDocument;
		mapedit_pending_continue(pApp);
		return;
	}
	mapedit_select_workspace(pApp, pApp->iPendingDocument);
	ret = mapedit_save_current(pApp);
	if (ret == XUI_OK) mapedit_pending_continue(pApp);
	else if (ret < 0) pApp->iPendingOperation = 0;
}

static void mapedit_pending_continue(mapedit_app_t* pApp)
{
	int operation = pApp->iPendingOperation;
	int document = -1, result = XUI_OK;
	char message[512], oldMapPath[MAPEDIT_PATH_MAX], oldTilesetPath[MAPEDIT_PATH_MAX];
	char path[MAPEDIT_PATH_MAX];
	if (!operation) return;
	if ((operation == 1 || operation == 2 || operation == 4) && pApp->tMap.bDirty &&
		!(pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_MAP))) document = MAPEDIT_WORKSPACE_MAP;
	else if (pApp->tTileset.bDirty && !(pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_TILESET))) document = MAPEDIT_WORKSPACE_TILESET;
	if (document >= 0) {
		const xui_msgbox_button_t buttons[] = {
			{"保存", XUI_MSGBOX_RESULT_YES, 0}, {"不保存", XUI_MSGBOX_RESULT_NO, 0}, {"取消", XUI_MSGBOX_RESULT_CANCEL, 0}
		};
		pApp->iPendingDocument = document;
		snprintf(message, sizeof(message), "%s“%s”有未保存修改。是否保存后继续？",
			document == MAPEDIT_WORKSPACE_MAP ? "地图" : "图集",
			document == MAPEDIT_WORKSPACE_MAP ? pApp->tMap.sName : pApp->tTileset.sName);
		if (!pApp->pConfirmBox) {
			xui_msgbox_desc_t desc;
			memset(&desc, 0, sizeof(desc));
			desc.iSize = sizeof(desc); desc.pFont = pApp->pFont;
			desc.sTitle = "未保存的修改"; desc.sMessage = message;
			desc.iButtons = XUI_MSGBOX_BUTTON_CUSTOM;
			desc.pCustomButtons = buttons; desc.iCustomButtonCount = 3;
			desc.bModal = 1; desc.bHasModal = 1;
			if (xuiMsgBoxCreate(pApp->pContext, &pApp->pConfirmBox, &desc) != XUI_OK) {
				pApp->iPendingOperation = 0; mapedit_status(pApp, "无法创建确认窗口，操作已取消"); return;
			}
			(void)xuiMsgBoxSetResult(pApp->pConfirmBox, mapedit_confirm_result, pApp);
		}
		(void)xuiMsgBoxSetText(pApp->pConfirmBox, "未保存的修改", message);
		(void)xuiMsgBoxSetOpen(pApp->pConfirmBox, 1);
		xgeRenderRequest();
		return;
	}
	mapedit_copy_text(path, sizeof(path), pApp->sPendingPath);
	mapedit_copy_text(oldMapPath, sizeof(oldMapPath), pApp->tMap.sPath);
	mapedit_copy_text(oldTilesetPath, sizeof(oldTilesetPath), pApp->tTileset.sPath);
	pApp->iPendingOperation = 0;
	if (operation == 1) {
		if (pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_MAP)) pApp->tMap.bDirty = 0;
		if (pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_TILESET)) pApp->tTileset.bDirty = 0;
		mapedit_save_layouts(pApp);
		xgeQuit();
	} else if (operation == 2 || operation == 4) {
		result = mapedit_map_load(&pApp->tMap, path);
		if (result == XUI_OK) {
			if (operation == 4) { mapedit_copy_text(pApp->tMap.sPath, sizeof(pApp->tMap.sPath), oldMapPath); pApp->tMap.bDirty = 1; }
			mapedit_activate_loaded_map(pApp, operation == 4 ? "恢复版本已载入，请检查后保存" : "地图已打开");
			mapedit_rescan_map_list(pApp, pApp->tMap.sPath);
			mapedit_select_workspace(pApp, MAPEDIT_WORKSPACE_MAP);
		} else mapedit_status(pApp, gMapeditMapIoError);
	} else {
		result = mapedit_open_tileset(pApp, path);
		if (result == XUI_OK) {
			if (operation == 5) {
				mapedit_copy_text(pApp->tTileset.sPath, sizeof(pApp->tTileset.sPath), oldTilesetPath);
				mapedit_copy_text(pApp->tTileset.sFile, sizeof(pApp->tTileset.sFile), mapedit_path_file_name(oldTilesetPath));
				pApp->tTileset.bDirty = 1; pApp->tTilesetHistory.iSavedRevision = UINT64_MAX;
				pApp->tTilesetStamp = mapeditFileStamp(oldTilesetPath);
			}
			mapedit_select_workspace(pApp, MAPEDIT_WORKSPACE_TILESET);
		}
	}
	if (result == XUI_OK) {
		if (pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_MAP)) mapedit_clear_recovery_for(pApp, MAPEDIT_WORKSPACE_MAP, oldMapPath);
		if (pApp->iPendingDiscardMask & (1 << MAPEDIT_WORKSPACE_TILESET)) mapedit_clear_recovery_for(pApp, MAPEDIT_WORKSPACE_TILESET, oldTilesetPath);
	}
	pApp->iPendingDiscardMask = 0;
	mapedit_update_window_title(pApp);
}

static void mapedit_request_operation(mapedit_app_t* pApp, int operation, const char* path)
{
	if (!pApp || pApp->iPendingOperation || pApp->bPainting) return;
	if (pApp->pFileDialog && xuiFileDialogIsOpen(pApp->pFileDialog)) return;
	pApp->iPendingOperation = operation;
	pApp->iPendingDiscardMask = 0;
	mapedit_copy_text(pApp->sPendingPath, sizeof(pApp->sPendingPath), path ? path : "");
	mapedit_pending_continue(pApp);
}

static int mapedit_native_close(void* user)
{
	mapedit_app_t* pApp = (mapedit_app_t*)user;
	if (!pApp->tMap.bDirty && !pApp->tTileset.bDirty && !pApp->iPendingOperation) {
		mapedit_save_layouts(pApp);
		return 1;
	}
	mapedit_request_operation(pApp, 1, NULL);
	return 0;
}

static void mapedit_external_result(xui_msgbox_t* box, int result, void* user)
{
	mapedit_app_t* pApp = (mapedit_app_t*)user;
	int map = pApp->iExternalDocument == MAPEDIT_WORKSPACE_MAP;
	(void)box;
	if (result == XUI_MSGBOX_RESULT_YES)
		mapedit_request_operation(pApp, map ? 2 : 3, map ? pApp->tMap.sPath : pApp->tTileset.sPath);
	else mapedit_status(pApp, "保留内存版本；如需保存请另存为，避免覆盖外部修改");
}

static void mapedit_poll_external_files(mapedit_app_t* pApp)
{
	int mapChanged, setChanged, document;
	const char* path;
	char message[640];
	double now = xgeTimer();
	mapedit_file_stamp_t mapStamp, setStamp;
	if (now < pApp->fNextFileCheck) { xgeRenderRequestAfter((float)(pApp->fNextFileCheck - now)); return; }
	pApp->fNextFileCheck = now + 2.0;
	xgeRenderRequestAfter(2.0f);
	if (pApp->iPendingOperation || (pApp->pFileDialog && xuiFileDialogIsOpen(pApp->pFileDialog)) ||
		(pApp->pExternalBox && xuiMsgBoxIsOpen(pApp->pExternalBox))) return;
	mapStamp = mapeditFileStamp(pApp->tMap.sPath);
	setStamp = mapeditFileStamp(pApp->tTileset.sPath);
	mapChanged = pApp->tMap.sPath[0] && !pApp->bMapExternalConflict && !mapeditFileStampEqual(mapStamp, pApp->tMapStamp);
	setChanged = pApp->tTileset.sPath[0] && !pApp->bTilesetExternalConflict && !mapeditFileStampEqual(setStamp, pApp->tTilesetStamp);
	if (!mapChanged && !setChanged) return;
	document = mapChanged ? MAPEDIT_WORKSPACE_MAP : MAPEDIT_WORKSPACE_TILESET;
	path = mapChanged ? pApp->tMap.sPath : pApp->tTileset.sPath;
	if (mapChanged) pApp->bMapExternalConflict = 1;
	else pApp->bTilesetExternalConflict = 1;
	pApp->iExternalDocument = document;
	snprintf(message, sizeof(message), "磁盘上的文件已被修改或删除：\n%s\n\n重新加载磁盘版本，还是保留当前内存版本？", path);
	if (!pApp->pExternalBox) {
		const xui_msgbox_button_t buttons[] = {{"重新加载", XUI_MSGBOX_RESULT_YES, 0}, {"保留内存版本", XUI_MSGBOX_RESULT_NO, 0}};
		xui_msgbox_desc_t desc;
		memset(&desc, 0, sizeof(desc));
		desc.iSize = sizeof(desc); desc.pFont = pApp->pFont;
		desc.sTitle = "外部文件变化"; desc.sMessage = message;
		desc.iButtons = XUI_MSGBOX_BUTTON_CUSTOM; desc.pCustomButtons = buttons; desc.iCustomButtonCount = 2;
		desc.bHasModal = 1; desc.bModal = 1;
		if (xuiMsgBoxCreate(pApp->pContext, &pApp->pExternalBox, &desc) != XUI_OK) {
			mapedit_status(pApp, "检测到外部文件变化；保存已保护，请重新打开或另存为");
			return;
		}
		(void)xuiMsgBoxSetResult(pApp->pExternalBox, mapedit_external_result, pApp);
	}
	(void)xuiMsgBoxSetText(pApp->pExternalBox, "外部文件变化", message);
	(void)xuiMsgBoxSetOpen(pApp->pExternalBox, 1);
}

static int mapedit_create_render_widget(mapedit_app_t* pApp, xui_widget* ppWidget, xui_widget_cache_render_proc onRender, void* pUser)
{
	xui_widget w;
	xui_cache_policy_t policy;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &w);
	if ( ret != XUI_OK ) return ret;
	memset(&policy, 0, sizeof(policy));
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	policy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(w, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(w, &policy);
	(void)xuiWidgetSetCacheRenderCallback(w, onRender, pUser);
	*ppWidget = w;
	return XUI_OK;
}

static void mapedit_widget_fill(xui_widget pWidget)
{
	if ( pWidget == NULL ) return;
	(void)xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	(void)xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	(void)xuiWidgetSetFlex(pWidget, 1.0f, 1.0f);
}

static void mapedit_widget_fixed_height(xui_widget pWidget, float fHeight)
{
	if ( pWidget == NULL ) return;
	(void)xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	(void)xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
}

static void mapedit_widget_fixed_size(xui_widget pWidget, float fWidth, float fHeight)
{
	if ( pWidget == NULL ) return;
	(void)xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){fWidth, fHeight});
	(void)xuiWidgetSetAlign(pWidget, XUI_ALIGN_START, XUI_ALIGN_START);
}

static void mapedit_scroll_frame_desc_default(xui_scroll_frame_desc_t* pDesc)
{
	if ( pDesc == NULL ) return;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->fContentWidth = 1.0f;
	pDesc->fContentHeight = 1.0f;
	pDesc->iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	pDesc->iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	pDesc->iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	pDesc->iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	pDesc->iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
	pDesc->bContentDragEnabled = 0;
	pDesc->fScrollbarSize = MAPEDIT_MAP_EDIT_SCROLLBAR_SIZE;
	pDesc->fMinThumbSize = 28.0f;
	pDesc->fButtonSize = 0.0f;
	pDesc->fWheelStep = 40.0f;
	pDesc->iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pDesc->iTrackColor = XUI_COLOR_RGBA(226, 240, 249, 255);
	pDesc->iThumbColor = XUI_COLOR_RGBA(90, 164, 214, 255);
	pDesc->iHoverColor = XUI_COLOR_RGBA(108, 180, 226, 255);
	pDesc->iActiveColor = XUI_COLOR_RGBA(68, 142, 204, 255);
	pDesc->iFocusColor = XUI_COLOR_RGBA(108, 180, 226, 255);
	pDesc->iDisabledColor = XUI_COLOR_RGBA(180, 204, 218, 255);
	pDesc->iButtonColor = XUI_COLOR_RGBA(226, 240, 249, 255);
	pDesc->iButtonIconColor = XUI_COLOR_RGBA(90, 164, 214, 255);
	pDesc->iCornerColor = XUI_COLOR_RGBA(226, 240, 249, 255);
	pDesc->iGripColor = XUI_COLOR_RGBA(90, 164, 214, 255);
}

static int mapedit_create_canvas_scroll_frame(mapedit_app_t* pApp, xui_widget* ppFrame, xui_widget pCanvas, xui_scroll_frame_change_proc onChange)
{
	xui_scroll_frame_desc_t d;
	xui_widget viewport;
	int ret;
	if ( pApp == NULL || ppFrame == NULL || pCanvas == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	mapedit_scroll_frame_desc_default(&d);
	ret = xuiScrollFrameCreate(pApp->pContext, ppFrame, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(*ppFrame);
	(void)xuiScrollFrameSetChange(*ppFrame, onChange, pApp);
	viewport = xuiScrollFrameGetViewportWidget(*ppFrame);
	if ( viewport == NULL ) {
		xuiWidgetDestroy(*ppFrame);
		*ppFrame = NULL;
		return XUI_ERROR;
	}
	(void)xuiWidgetSetRect(pCanvas, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f});
	ret = xuiWidgetAddChild(viewport, pCanvas);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(*ppFrame);
		*ppFrame = NULL;
		return ret;
	}
	return XUI_OK;
}

static int mapedit_create_tags_panel(mapedit_app_t* pApp, int bMap, xui_widget* ppPanel)
{
	xui_widget panel;
	xui_widget row;
	xui_widget combo;
	xui_widget check;
	xui_widget form;
	xui_widget canvas;
	xui_widget scrollFrame;
	xui_combobox_desc_t cd;
	xui_checkbox_desc_t xd;
	xui_property_grid_desc_t pd;
	int ret;
	if ( pApp == NULL || ppPanel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	canvas = bMap ? pApp->pMapTagsCanvas : pApp->pTilesetTagsCanvas;
	scrollFrame = bMap ? pApp->pMapTagsScrollFrame : pApp->pTilesetTagsScrollFrame;
	if ( canvas == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( scrollFrame == NULL ) scrollFrame = canvas;
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &row);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){4.0f, 4.0f, 4.0f, 4.0f});
	(void)xuiWidgetSetGap(panel, 4.0f);
	mapedit_widget_fill(panel);
	(void)xuiWidgetSetLayoutType(row, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(row, 8.0f);
	mapedit_widget_fixed_height(row, 28.0f);
	memset(&cd, 0, sizeof(cd));
	cd.iSize = sizeof(cd);
	cd.pFont = pApp->pFont;
	cd.fItemHeight = MAPEDIT_ROW_HEIGHT;
	cd.fPopupMaxHeight = 220.0f;
	cd.iTextColor = XUI_COLOR_RGBA(40, 58, 76, 255);
	cd.iBackgroundColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	cd.iHoverBackgroundColor = XUI_COLOR_RGBA(232, 242, 252, 255);
	cd.iOpenBackgroundColor = XUI_COLOR_RGBA(242, 248, 254, 255);
	cd.iBorderColor = XUI_COLOR_RGBA(148, 174, 198, 255);
	cd.iHoverBorderColor = XUI_COLOR_RGBA(86, 156, 214, 255);
	cd.iFocusBorderColor = XUI_COLOR_RGBA(60, 132, 200, 255);
	cd.iArrowColor = XUI_COLOR_RGBA(58, 76, 96, 255);
	ret = xuiComboBoxCreate(pApp->pContext, &combo, &cd);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		xuiWidgetDestroy(row);
		return ret;
	}
	(void)xuiComboBoxSetSelect(combo, mapedit_tags_channel_select, pApp);
	mapedit_widget_fixed_size(combo, 260.0f, 28.0f);
	memset(&xd, 0, sizeof(xd));
	xd.iSize = sizeof(xd);
	xd.sText = bMap ? "检查地图位置所有字段" : "检查图块所有字段";
	xd.pFont = pApp->pFont;
	xd.bUseBuiltinAtlas = 1;
	xd.iTextColor = XUI_COLOR_RGBA(40, 58, 76, 255);
	ret = xuiCheckBoxCreate(pApp->pContext, &check, &xd);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(combo);
		xuiWidgetDestroy(panel);
		xuiWidgetDestroy(row);
		return ret;
	}
	(void)xuiCheckBoxSetChange(check, mapedit_tags_inspect_change, pApp);
	mapedit_widget_fixed_size(check, bMap ? 190.0f : 160.0f, 28.0f);
	memset(&pd, 0, sizeof(pd));
	pd.iSize = sizeof(pd);
	pd.pFont = pApp->pFont;
	pd.fNameWidth = 120.0f;
	pd.fRowHeight = MAPEDIT_ROW_HEIGHT;
	pd.fCategoryHeight = 25.0f;
	pd.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	pd.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	pd.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	ret = xuiPropertyGridCreate(pApp->pContext, &form, &pd);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(check);
		xuiWidgetDestroy(combo);
		xuiWidgetDestroy(panel);
		xuiWidgetDestroy(row);
		return ret;
	}
	(void)xuiPropertyGridSetChange(form, mapedit_tags_form_change, pApp);
	mapedit_widget_fixed_height(form, MAPEDIT_TAG_FORM_H);
	mapedit_widget_fill(scrollFrame);
	ret = xuiWidgetAddChild(row, combo);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(row, check);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, row);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, form);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, scrollFrame);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(form);
		xuiWidgetDestroy(check);
		xuiWidgetDestroy(combo);
		xuiWidgetDestroy(row);
		xuiWidgetDestroy(panel);
		return ret;
	}
	if ( bMap ) {
		pApp->pMapTagsPanel = panel;
		pApp->pMapTagsChannelCombo = combo;
		pApp->pMapTagsInspectCheck = check;
		pApp->pMapTagsFormGrid = form;
	} else {
		pApp->pTilesetTagsPanel = panel;
		pApp->pTilesetTagsChannelCombo = combo;
		pApp->pTilesetTagsInspectCheck = check;
		pApp->pTilesetTagsFormGrid = form;
	}
	*ppPanel = panel;
	mapedit_tags_update_channel_combo(pApp, bMap);
	return XUI_OK;
}

static int mapedit_create_list(mapedit_app_t* pApp, xui_widget* ppList, mapedit_file_list_t* pFiles, xui_list_view_select_proc onSelect)
{
	xui_list_view_desc_t d;
	xui_widget w;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.arrItems = pFiles->arrTextPtrs;
	d.iItemCount = pFiles->iCount;
	d.pFont = pApp->pFont;
	d.fItemHeight = MAPEDIT_ROW_HEIGHT;
	d.fPadding = 4.0f;
	d.iSelected = pFiles->iSelected;
	d.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	d.iBackgroundColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	d.iBorderColor = XUI_COLOR_RGBA(152, 174, 198, 255);
	d.iRowColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	d.iHoverColor = XUI_COLOR_RGBA(232, 242, 252, 255);
	d.iSelectedColor = XUI_COLOR_RGBA(38, 117, 178, 255);
	d.iTextColor = XUI_COLOR_RGBA(40, 58, 76, 255);
	d.iDisabledTextColor = XUI_COLOR_RGBA(130, 142, 154, 255);
	ret = xuiListViewCreate(pApp->pContext, &w, &d);
	if ( ret != XUI_OK ) return ret;
	(void)xuiListViewSetSelect(w, onSelect, pApp);
	*ppList = w;
	return XUI_OK;
}

static void mapedit_refresh_file_list(mapedit_app_t* pApp, mapedit_file_list_t* pFiles, xui_widget pList, const char* sRelDir, const char* sExt, const char* sSelectName)
{
	char path[MAPEDIT_PATH_MAX];
	int selected;
	if ( pApp == NULL || pFiles == NULL || sRelDir == NULL || sExt == NULL ) return;
	mapedit_app_path(pApp, path, sizeof(path), sRelDir);
	mapedit_scan_files(path, sExt, pFiles);
	if ( strcmp(sRelDir, "assets\\maps") == 0 ) mapedit_refresh_map_list_texts(pApp, pFiles);
	else if ( strcmp(sRelDir, "assets\\图块集") == 0 ) mapedit_refresh_tileset_list_texts(pApp, pFiles);
	selected = mapedit_file_list_find(pFiles, sSelectName);
	if ( selected >= 0 ) pFiles->iSelected = selected;
	if ( pList != NULL ) {
		(void)xuiListViewSetItems(pList, pFiles->arrTextPtrs, pFiles->iCount);
		if ( pFiles->iSelected >= 0 ) (void)xuiListViewSetSelected(pList, pFiles->iSelected);
	}
}

static int mapedit_make_unique_asset_file(mapedit_app_t* pApp, const char* sRelDir, const char* sPrefix, const char* sExt, char* sOutName, int iCap, char* sOutPath, int iPathCap)
{
	char rel[MAPEDIT_PATH_MAX];
	int i;
	if ( pApp == NULL || sRelDir == NULL || sPrefix == NULL || sExt == NULL || sOutName == NULL || iCap <= 0 || sOutPath == NULL || iPathCap <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 1; i < 10000; i++ ) {
		snprintf(sOutName, (size_t)iCap, "%s_%03d%s", sPrefix, i, sExt);
		snprintf(rel, sizeof(rel), "%s\\%s", sRelDir, sOutName);
		mapedit_app_path(pApp, sOutPath, iPathCap, rel);
		if ( !mapedit_file_exists_utf8(sOutPath) ) return XUI_OK;
	}
	return XUI_ERROR;
}

static int mapedit_create_default_map_file(mapedit_app_t* pApp, char* sOutName, int iCap)
{
	mapedit_map_doc_t doc;
	char path[MAPEDIT_PATH_MAX];
	int ret;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_make_unique_asset_file(pApp, "assets\\maps", "map", ".xson", sOutName, iCap, path, sizeof(path)) != XUI_OK ) return XUI_ERROR;
	memset(&doc, 0, sizeof(doc));
	mapedit_map_clear(&doc);
	mapedit_copy_text(doc.sPath, sizeof(doc.sPath), path);
	snprintf(doc.sName, sizeof(doc.sName), "新建地图%d", pApp->tMapFiles.iCount + 1);
	mapedit_copy_text(doc.sTileset, sizeof(doc.sTileset), pApp->tTileset.sFile[0] ? pApp->tTileset.sFile : "default.xson");
	doc.iWidth = 100;
	doc.iHeight = 100;
	doc.iLayers = pApp->iSetupLayerCount > 0 ? pApp->iSetupLayerCount : XGE_MAP_LEGACY_DEFAULT_LAYERS;
	doc.iState = mapedit_setup_clamp_state(pApp, pApp != NULL ? pApp->iSetupStateMin : 0);
	doc.pPassageRaw = xrtValueArray();
	doc.pCellDataRaw = xrtValueObject();
	ret = mapedit_map_alloc(&doc);
	if ( ret == XUI_OK ) ret = mapedit_map_save(&doc, path);
	mapedit_map_clear(&doc);
	return ret;
}

static int mapedit_create_default_tileset_file(mapedit_app_t* pApp, char* sOutName, int iCap)
{
	char path[MAPEDIT_PATH_MAX];
	char displayName[MAPEDIT_NAME_MAX];
	const char* sStatic;
	xvalue* pRoot;
	xvalue* pSpecial;
	xvalue* pPassage;
	xvalue* pActor;
	xvalue* pCustom;
	int ok;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_make_unique_asset_file(pApp, "assets\\图块集", "tileset", ".xson", sOutName, iCap, path, sizeof(path)) != XUI_OK ) return XUI_ERROR;
	sStatic = pApp->tMaterialFiles.iCount > 0 ? pApp->tMaterialFiles.arrNames[0] : "";
	pRoot = xrtValueObject();
	pSpecial = xrtValueArray();
	pPassage = xrtValueArray();
	pActor = xrtValueArray();
	pCustom = xrtValueObject();
	if ( pRoot == NULL || pSpecial == NULL || pPassage == NULL || pActor == NULL || pCustom == NULL ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		if ( pSpecial != NULL ) xrtValueRelease(pSpecial);
		if ( pPassage != NULL ) xrtValueRelease(pPassage);
		if ( pActor != NULL ) xrtValueRelease(pActor);
		if ( pCustom != NULL ) xrtValueRelease(pCustom);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	snprintf(displayName, sizeof(displayName), "新建图集%d", pApp->tTilesetFiles.iCount + 1);
	if ( !xuiXrtValueObjectSetText(pRoot, SET_KEY_FORMAT, (uint32)strlen(SET_KEY_FORMAT), MAPEDIT_TILESET_FORMAT_NAME, 0, FALSE) ||
	     !xuiXrtValueObjectSetInt(pRoot, SET_KEY_VERSION, (uint32)strlen(SET_KEY_VERSION), MAPEDIT_TILESET_FORMAT_VERSION) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_NAME, (uint32)strlen(SET_KEY_NAME), displayName, 0, FALSE) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_STATIC, (uint32)strlen(SET_KEY_STATIC), sStatic, 0, FALSE) ||
	     !xuiXrtValueObjectSetInt(pRoot, SET_KEY_SPECIAL_COUNT, (uint32)strlen(SET_KEY_SPECIAL_COUNT), 0) ||
	     !xuiXrtValueObjectSetText(pRoot, SET_KEY_CUSTOM, (uint32)strlen(SET_KEY_CUSTOM), "", 0, FALSE) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_SPECIAL_TILES, &pSpecial) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_PASSAGE, &pPassage) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_ACTOR, &pActor) ||
	     !mapedit_table_set_take(pRoot, SET_KEY_TILE_DATA, &pCustom) ) goto create_out_of_memory;
	ok = xrtXsonStringifyFile(path, pRoot, true) ? 1 : 0;
	xrtValueRelease(pRoot);
	return ok ? XUI_OK : XUI_ERROR;

create_out_of_memory:
	if ( pSpecial != NULL ) xrtValueRelease(pSpecial);
	if ( pPassage != NULL ) xrtValueRelease(pPassage);
	if ( pActor != NULL ) xrtValueRelease(pActor);
	if ( pCustom != NULL ) xrtValueRelease(pCustom);
	xrtValueRelease(pRoot);
	return XUI_ERROR_OUT_OF_MEMORY;
}

static int mapedit_material_select_for_action(mapedit_app_t* pApp, int iIndex)
{
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->tMaterialCategoryFiles.iCount ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->tMaterialCategoryFiles.iSelected = iIndex;
	if ( pApp->pMaterialListView != NULL ) (void)xuiListViewSetSelected(pApp->pMaterialListView, iIndex);
	return XUI_OK;
}

static void mapedit_material_view_close(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMaterialViewWindow != NULL ) (void)xuiWindowSetOpen(pApp->pMaterialViewWindow, 0);
	mapedit_material_view_clear(pApp);
}

static void mapedit_material_view_close_cb(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	mapedit_material_view_close((mapedit_app_t*)pUser);
}

static int mapedit_material_view_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	if ( pEvent == NULL ) return XUI_OK;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iKey == XGE_KEY_ESCAPE ) {
		mapedit_material_view_close((mapedit_app_t*)pUser);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_material_modal_pointer_event(int iType)
{
	return iType == XUI_EVENT_POINTER_DOWN ||
	       iType == XUI_EVENT_POINTER_UP ||
	       iType == XUI_EVENT_POINTER_MOVE ||
	       iType == XUI_EVENT_POINTER_CLICK ||
	       iType == XUI_EVENT_POINTER_WHEEL ||
	       iType == XUI_EVENT_POINTER_DOUBLE_CLICK ||
	       iType == XUI_EVENT_CONTEXT_MENU;
}

static int mapedit_material_modal_keyboard_event(int iType)
{
	return iType == XUI_EVENT_KEY_DOWN ||
	       iType == XUI_EVENT_KEY_UP ||
	       iType == XUI_EVENT_TEXT ||
	       iType == XUI_EVENT_HOTKEY ||
	       iType == XUI_EVENT_COMMAND;
}

static xui_widget mapedit_material_top_modal(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return NULL;
	if ( pApp->pMaterialEditWindow != NULL && xuiWindowIsOpen(pApp->pMaterialEditWindow) ) return pApp->pMaterialEditWindow;
	if ( pApp->pMaterialViewWindow != NULL && xuiWindowIsOpen(pApp->pMaterialViewWindow) ) return pApp->pMaterialViewWindow;
	if ( pApp->pMaterialRenameWindow != NULL && xuiWindowIsOpen(pApp->pMaterialRenameWindow) ) return pApp->pMaterialRenameWindow;
	return NULL;
}

static void mapedit_material_close_modal(mapedit_app_t* pApp, xui_widget pWindow)
{
	if ( pApp == NULL || pWindow == NULL ) return;
	if ( pWindow == pApp->pMaterialEditWindow ) {
		mapedit_material_edit_close(pApp);
	} else if ( pWindow == pApp->pMaterialViewWindow ) {
		mapedit_material_view_close(pApp);
	} else if ( pWindow == pApp->pMaterialRenameWindow ) {
		(void)xuiWindowSetOpen(pApp->pMaterialRenameWindow, 0);
		pApp->iMaterialRenameIndex = -1;
	}
}

static int mapedit_point_in_rect(xui_rect_t r, int x, int y)
{
	return x >= r.fX && y >= r.fY && x < r.fX + r.fW && y < r.fY + r.fH;
}

static void mapedit_material_close_context_menu_on_list_click(mapedit_app_t* pApp, const xui_event_t* pEvent)
{
	xui_widget popup;
	xui_rect_t r;
	if ( pApp == NULL || pEvent == NULL || pApp->pMaterialListView == NULL || pApp->pMaterialContextMenu == NULL ) return;
	if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) return;
	if ( pEvent->iType != XUI_EVENT_POINTER_DOWN && pEvent->iType != XUI_EVENT_POINTER_UP ) return;
	if ( !xuiMenuIsOpen(pApp->pMaterialContextMenu) ) return;
	popup = xuiMenuGetPopupWidget(pApp->pMaterialContextMenu);
	if ( popup != NULL ) {
		r = xuiWidgetGetWorldRect(popup);
		if ( mapedit_point_in_rect(r, pEvent->fX, pEvent->fY) ) return;
	}
	r = xuiWidgetGetWorldRect(pApp->pMaterialListView);
	if ( mapedit_point_in_rect(r, pEvent->fX, pEvent->fY) ) {
		(void)xuiMenuClose(pApp->pMaterialContextMenu);
	}
}

static int mapedit_root_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_widget pModal;
	(void)pWidget;
	if ( pApp == NULL || pEvent == NULL ) return XUI_OK;
	mapedit_material_close_context_menu_on_list_click(pApp, pEvent);
	pModal = mapedit_material_top_modal(pApp);
	if ( pModal == NULL && pEvent->iType == XUI_EVENT_COMMAND && pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) {
		mapedit_execute_command(pApp, pEvent->iCommand);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pModal == NULL ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iKey == XUI_KEY_ESCAPE ) {
		mapedit_material_close_modal(pApp, pModal);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( mapedit_material_modal_pointer_event(pEvent->iType) ||
	     mapedit_material_modal_keyboard_event(pEvent->iType) ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_resize_target(mapedit_app_t* pApp, int iWidth, int iHeight)
{
	xui_surface_desc_t sd;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iWidth <= 0 ) iWidth = MAPEDIT_W;
	if ( iHeight <= 0 ) iHeight = MAPEDIT_H;
	if ( pApp->pTarget != NULL && pApp->iTargetWidth == iWidth && pApp->iTargetHeight == iHeight ) return XUI_OK;
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
	memset(&sd, 0, sizeof(sd));
	sd.iKind = XUI_SURFACE_KIND_TEXTURE;
	sd.iWidth = iWidth;
	sd.iHeight = iHeight;
	sd.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	sd.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	if ( pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &sd) != XUI_OK ) return XUI_ERROR;
	pApp->iTargetWidth = iWidth;
	pApp->iTargetHeight = iHeight;
	return XUI_OK;
}

static int mapedit_sync_viewport(mapedit_app_t* pApp)
{
	xge_platform_runtime_t runtime;
	int w;
	int h;
	float dpi = 1.0f;
	if ( pApp == NULL || pApp->pContext == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	w = xgeGetWidth();
	h = xgeGetHeight();
	if ( w <= 0 ) w = MAPEDIT_W;
	if ( h <= 0 ) h = MAPEDIT_H;
	memset(&runtime, 0, sizeof(runtime));
	if ( xgePlatformRuntimeGet(&runtime) == XGE_OK && runtime.fDpiScale > 0.0f ) dpi = runtime.fDpiScale;
	if ( pApp->iTargetWidth == w && pApp->iTargetHeight == h && fabsf(pApp->fDpiScale - dpi) < 0.001f ) return XUI_OK;
	if ( mapedit_resize_target(pApp, w, h) != XUI_OK ) return XUI_ERROR;
	pApp->fDpiScale = dpi;
	(void)xuiInputViewport(pApp->pContext, w, h);
	(void)xuiInputDpi(pApp->pContext, dpi);
	if ( pApp->pRoot != NULL ) (void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0, 0, w, h});
	if ( pApp->pRoot != NULL ) (void)xuiWidgetInvalidate(pApp->pRoot, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

static xui_rect_t mapedit_centered_root_rect(mapedit_app_t* pApp, float fScaleW, float fScaleH)
{
	xui_rect_t root;
	xui_rect_t rect;
	if ( pApp != NULL && pApp->pRoot != NULL ) root = xuiWidgetGetRect(pApp->pRoot);
	else root = (xui_rect_t){0, 0, MAPEDIT_W, MAPEDIT_H};
	if ( fScaleW <= 0.0f ) fScaleW = 0.8f;
	if ( fScaleH <= 0.0f ) fScaleH = 0.8f;
	rect.fW = (int)lroundf((float)root.fW * fScaleW);
	rect.fH = (int)lroundf((float)root.fH * fScaleH);
	rect.fX = root.fX + (root.fW - rect.fW) / 2;
	rect.fY = root.fY + (root.fH - rect.fH) / 2;
	return rect;
}

static int mapedit_material_open_view(mapedit_app_t* pApp, int iIndex)
{
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex < 0 ) iIndex = pApp->tMaterialCategoryFiles.iSelected;
	if ( mapedit_material_select_for_action(pApp, iIndex) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_material_view_load(pApp, iIndex) != XUI_OK ) {
		mapedit_status(pApp, "打开素材图片失败");
		return XUI_ERROR;
	}
	if ( pApp->pMaterialViewWindow != NULL ) {
		(void)xuiWidgetSetRect(pApp->pMaterialViewWindow, mapedit_centered_root_rect(pApp, 0.8f, 0.8f));
		(void)xuiWindowSetOpen(pApp->pMaterialViewWindow, 1);
		(void)xuiOverlayAttach(pApp->pContext, NULL, pApp->pMaterialViewWindow, XUI_LAYER_MODAL, 1);
		(void)xuiOverlayBringToFront(pApp->pMaterialViewWindow);
	}
	mapedit_status(pApp, "已打开素材查看");
	return XUI_OK;
}

static int mapedit_material_submit_rename(mapedit_app_t* pApp)
{
	const char* sText;
	if ( pApp == NULL || pApp->pMaterialRenameInput == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sText = xuiInputGetText(pApp->pMaterialRenameInput);
	if ( sText == NULL || sText[0] == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( mapedit_material_save_mapping(pApp, pApp->iMaterialRenameIndex, sText) == XUI_OK ) {
		if ( pApp->pMaterialRenameWindow != NULL ) (void)xuiWindowSetOpen(pApp->pMaterialRenameWindow, 0);
		pApp->iMaterialRenameIndex = -1;
		mapedit_status(pApp, "素材映射名称已更新");
		return XUI_OK;
	}
	mapedit_status(pApp, "素材映射名称保存失败");
	return XUI_ERROR;
}

static void mapedit_material_rename_button_click(xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) return;
	if ( pWidget == pApp->pMaterialRenameOkButton ) {
		(void)mapedit_material_submit_rename(pApp);
	} else if ( pWidget == pApp->pMaterialRenameCancelButton ) {
		if ( pApp->pMaterialRenameWindow != NULL ) (void)xuiWindowSetOpen(pApp->pMaterialRenameWindow, 0);
		pApp->iMaterialRenameIndex = -1;
	}
}

static int mapedit_material_rename_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE || pEvent->iType != XUI_EVENT_KEY_DOWN ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_ENTER ) {
		(void)mapedit_material_submit_rename(pApp);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		if ( pApp->pMaterialRenameWindow != NULL ) (void)xuiWindowSetOpen(pApp->pMaterialRenameWindow, 0);
		pApp->iMaterialRenameIndex = -1;
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static void mapedit_material_open_rename(mapedit_app_t* pApp, int iIndex)
{
	char sName[MAPEDIT_NAME_MAX];
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->tMaterialCategoryFiles.iCount ) return;
	if ( pApp->pMaterialRenameWindow == NULL || pApp->pMaterialRenameInput == NULL ) {
		mapedit_status(pApp, "素材重命名窗口创建失败");
		return;
	}
	mapedit_material_select_for_action(pApp, iIndex);
	pApp->iMaterialRenameIndex = iIndex;
	mapedit_material_display_name(pApp, iIndex, sName, sizeof(sName));
	(void)xuiInputSetText(pApp->pMaterialRenameInput, sName);
	(void)xuiInputSetSelection(pApp->pMaterialRenameInput, 0, (int)strlen(sName));
	(void)xuiWindowSetOpen(pApp->pMaterialRenameWindow, 1);
	(void)xuiOverlayAttach(pApp->pContext, NULL, pApp->pMaterialRenameWindow, XUI_LAYER_MODAL, 1);
	(void)xuiOverlayBringToFront(pApp->pMaterialRenameWindow);
	(void)xuiSetFocusWidget(pApp->pContext, pApp->pMaterialRenameInput);
}

static int mapedit_material_edit_source_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int col;
	int row;
	if ( pApp == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		pApp->bMaterialEditSelecting = 0;
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pApp->iMaterialEditSourceCols <= 0 || pApp->iMaterialEditSourceRows <= 0 ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( !mapedit_material_edit_cell_from_event(pWidget, pEvent, &col, &row) ) return XUI_EVENT_DISPATCH_STOP;
		pApp->bMaterialEditSelecting = 1;
		pApp->iMaterialEditDragCol = col;
		pApp->iMaterialEditDragRow = row;
		(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		mapedit_material_edit_set_selection(pApp, col, row, col, row);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_MOVE && pApp->bMaterialEditSelecting && (pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) ) {
		if ( mapedit_material_edit_cell_from_event(pWidget, pEvent, &col, &row) ) {
			mapedit_material_edit_set_selection(pApp, pApp->iMaterialEditDragCol, pApp->iMaterialEditDragRow, col, row);
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_UP && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		pApp->bMaterialEditSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) (void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_material_edit_output_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int col;
	int row;
	if ( pApp == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_WHEEL ) {
		if ( pEvent->fWheelY < 0.0f ) (void)mapedit_material_edit_extend_output_if_bottom(pApp);
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN && pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
		if ( mapedit_material_edit_cell_from_event(pWidget, pEvent, &col, &row) ) (void)mapedit_material_edit_paste(pApp, col, row);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static void mapedit_material_edit_button_click(xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) return;
	if ( pWidget == pApp->pMaterialEditLoadButton ) {
		(void)mapedit_material_edit_open_source_dialog(pApp);
	} else if ( pWidget == pApp->pMaterialEditOkButton ) {
		(void)mapedit_material_edit_submit(pApp);
	} else if ( pWidget == pApp->pMaterialEditCancelButton ) {
		mapedit_material_edit_close(pApp);
	}
}

static int mapedit_material_edit_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE || pEvent->iType != XUI_EVENT_KEY_DOWN ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		mapedit_material_edit_close(pApp);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static void mapedit_material_edit_close_cb(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	mapedit_material_edit_close((mapedit_app_t*)pUser);
}

static int mapedit_material_open_editor(mapedit_app_t* pApp, int iIndex)
{
	char title[MAPEDIT_LIST_TEXT_MAX];
	char name[MAPEDIT_NAME_MAX];
	const char* sFile;
	int ret;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pMaterialEditWindow == NULL || pApp->pMaterialEditNameInput == NULL || pApp->pMaterialEditFileInput == NULL ) {
		mapedit_status(pApp, "图块编辑窗口创建失败");
		return XUI_ERROR;
	}
	if ( pApp->pMaterialEditMsgTip != NULL ) (void)xuiMsgTipClose(pApp->pMaterialEditMsgTip);
	if ( iIndex >= pApp->tMaterialCategoryFiles.iCount ) iIndex = -1;
	pApp->iMaterialEditMode = pApp->iMaterialCategory == 0 ? MAPEDIT_MATERIAL_MODE_STATIC : MAPEDIT_MATERIAL_MODE_OTHER;
	pApp->iMaterialEditIndex = iIndex;
	pApp->sMaterialEditOriginalFile[0] = 0;
	mapedit_material_edit_clear_source(pApp);
	mapedit_material_edit_clear_output(pApp);
	if ( pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ) ret = mapedit_material_edit_set_output_size(pApp, MAPEDIT_MATERIAL_STATIC_COLS, MAPEDIT_MATERIAL_INITIAL_ROWS);
	else ret = mapedit_material_edit_set_output_size(pApp, 4, 4);
	if ( ret != XUI_OK ) {
		mapedit_status(pApp, "制作图块输出画布创建失败");
		return ret;
	}
	if ( pApp->pMaterialEditSplit != NULL ) {
		(void)xuiSplitLayoutSetPaneMode(pApp->pMaterialEditSplit, 0, pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ? XUI_SPLIT_PANE_FIXED : XUI_SPLIT_PANE_GROW);
		if ( pApp->iMaterialEditMode == MAPEDIT_MATERIAL_MODE_STATIC ) {
			(void)xuiSplitLayoutSetPaneFixedSize(pApp->pMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
			(void)xuiSplitLayoutSetPaneMinSize(pApp->pMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
		} else {
			(void)xuiSplitLayoutSetPaneMinSize(pApp->pMaterialEditSplit, 0, 260.0f);
		}
		(void)xuiSplitLayoutSetPaneWeight(pApp->pMaterialEditSplit, 0, 1.0f);
		(void)xuiSplitLayoutSetPaneWeight(pApp->pMaterialEditSplit, 1, 1.0f);
	}
	if ( iIndex >= 0 ) {
		mapedit_material_select_for_action(pApp, iIndex);
		sFile = pApp->tMaterialCategoryFiles.arrNames[iIndex];
		mapedit_copy_text(pApp->sMaterialEditOriginalFile, sizeof(pApp->sMaterialEditOriginalFile), sFile);
		mapedit_material_display_name(pApp, iIndex, name, sizeof(name));
		(void)xuiInputSetText(pApp->pMaterialEditNameInput, name);
		(void)xuiInputSetText(pApp->pMaterialEditFileInput, sFile);
		(void)mapedit_material_edit_load_existing_output(pApp, iIndex);
		snprintf(title, sizeof(title), "图块编辑 - %s", name[0] ? name : sFile);
	} else {
		(void)xuiInputSetText(pApp->pMaterialEditNameInput, "新建图块");
		(void)xuiInputSetText(pApp->pMaterialEditFileInput, "");
		snprintf(title, sizeof(title), "图块编辑 - 新建图块");
	}
	(void)xuiWindowSetTitle(pApp->pMaterialEditWindow, title);
	(void)xuiWidgetSetRect(pApp->pMaterialEditWindow, mapedit_centered_root_rect(pApp, 0.8f, 0.8f));
	(void)xuiWindowSetOpen(pApp->pMaterialEditWindow, 1);
	(void)xuiOverlayAttach(pApp->pContext, NULL, pApp->pMaterialEditWindow, XUI_LAYER_MODAL, 2);
	(void)xuiOverlayBringToFront(pApp->pMaterialEditWindow);
	(void)xuiSetFocusWidget(pApp->pContext, pApp->pMaterialEditNameInput);
	return XUI_OK;
}

static int mapedit_material_hit_list_item(xui_widget pWidget, const xui_event_t* pEvent)
{
	xui_rect_t wr;
	if ( pWidget == NULL || pEvent == NULL ) return -1;
	wr = xuiWidgetGetWorldRect(pWidget);
	return xuiListViewGetItemAt(pWidget, (float)(pEvent->fX - wr.fX), (float)(pEvent->fY - wr.fY));
}

static int mapedit_material_list_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int index;
	if ( pApp == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	index = mapedit_material_hit_list_item(pWidget, pEvent);
	if ( pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) {
		if ( index >= 0 ) (void)mapedit_material_open_view(pApp, index);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int mapedit_material_list_context(xui_widget pWidget, int iIndex, float fX, float fY, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iIndex >= 0 ) {
		pApp->iMaterialContextIndex = iIndex;
		(void)mapedit_material_select_for_action(pApp, iIndex);
	} else {
		pApp->iMaterialContextIndex = pApp->tMaterialCategoryFiles.iSelected;
	}
	if ( pApp->iMaterialContextIndex >= 0 && pApp->pMaterialContextMenu != NULL ) {
		(void)xuiMenuOpenAt(pApp->pMaterialContextMenu, pWidget, fX, fY);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static void mapedit_material_context_select(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	(void)iIndex;
	if ( pApp == NULL ) return;
	switch ( iValue ) {
	case CMD_MATERIAL_VIEW:
		(void)mapedit_material_open_view(pApp, pApp->iMaterialContextIndex);
		break;
	case CMD_MATERIAL_RENAME:
		mapedit_material_open_rename(pApp, pApp->iMaterialContextIndex);
		break;
	case CMD_MATERIAL_EDIT:
		(void)mapedit_material_open_editor(pApp, pApp->iMaterialContextIndex);
		break;
	default:
		break;
	}
}

static void mapedit_material_selected(xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->tMaterialCategoryFiles.iCount ) return;
	pApp->tMaterialCategoryFiles.iSelected = iIndex;
	mapedit_status(pApp, "素材已选择");
}

static void mapedit_material_category_selected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	(void)pWidget;
	(void)iValue;
	if ( pApp == NULL ) return;
	if ( mapedit_material_scan_category(pApp, iIndex, 1) == XUI_OK ) mapedit_status(pApp, "素材分类已切换");
	else mapedit_status(pApp, "素材分类加载失败");
}

static void mapedit_manager_button_click(xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	char name[MAPEDIT_FILE_MAX];
	int idx;
	if ( pApp == NULL || pWidget == NULL ) return;
	if ( pWidget == pApp->pMapAddButton ) {
		if ( mapedit_create_default_map_file(pApp, name, sizeof(name)) == XUI_OK ) {
			mapedit_refresh_file_list(pApp, &pApp->tMapFiles, pApp->pMapListView, "assets\\maps", ".xson", name);
			idx = mapedit_file_list_find(&pApp->tMapFiles, name);
			if ( idx >= 0 ) mapedit_map_selected(pApp->pMapListView, idx, pApp);
			mapedit_status(pApp, "已添加地图");
		} else {
			mapedit_status(pApp, "添加地图失败");
		}
	} else if ( pWidget == pApp->pTilesetAddButton ) {
		if ( mapedit_create_default_tileset_file(pApp, name, sizeof(name)) == XUI_OK ) {
			mapedit_refresh_file_list(pApp, &pApp->tTilesetFiles, pApp->pTilesetListView, "assets\\图块集", ".xson", name);
			idx = mapedit_file_list_find(&pApp->tTilesetFiles, name);
			if ( idx >= 0 ) mapedit_tileset_selected(pApp->pTilesetListView, idx, pApp);
			mapedit_status(pApp, "已添加图集");
		} else {
			mapedit_status(pApp, "添加图集失败");
		}
	} else if ( pWidget == pApp->pMaterialCreateButton ) {
		(void)mapedit_material_open_editor(pApp, -1);
	}
}

static int mapedit_create_manager_button_widget(mapedit_app_t* pApp, const char* sText, xui_widget* ppButton)
{
	xui_button_desc_t d;
	xui_widget b;
	int ret;
	if ( pApp == NULL || ppButton == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.sText = sText;
	d.pFont = pApp->pFont;
	d.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	d.iTextColor = XUI_COLOR_RGBA(34, 62, 90, 255);
	d.iNormalColor = XUI_COLOR_RGBA(244, 249, 253, 255);
	d.iHoverColor = XUI_COLOR_RGBA(228, 240, 251, 255);
	d.iActiveColor = XUI_COLOR_RGBA(210, 230, 248, 255);
	d.iBorderColor = XUI_COLOR_RGBA(136, 166, 196, 255);
	d.fBorderWidth = 1.0f;
	ret = xuiButtonCreate(pApp->pContext, &b, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_size(b, 92.0f, MAPEDIT_BUTTON_HEIGHT);
	(void)xuiButtonSetClick(b, mapedit_manager_button_click, pApp);
	*ppButton = b;
	return XUI_OK;
}

static int mapedit_create_list_manager_panel(mapedit_app_t* pApp, xui_widget pList, xui_widget* ppPanel, xui_widget* ppAdd, const char* sAddText)
{
	xui_widget panel;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){4.0f, 4.0f, 4.0f, 4.0f});
	(void)xuiWidgetSetGap(panel, 4.0f);
	mapedit_widget_fill(panel);
	if ( mapedit_create_manager_button_widget(pApp, sAddText, ppAdd) != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return XUI_ERROR;
	}
	(void)xuiButtonSetSemantic(*ppAdd, XUI_BUTTON_SEMANTIC_PRIMARY);
	mapedit_widget_fixed_height(*ppAdd, 30.0f);
	mapedit_widget_fill(pList);
	ret = xuiWidgetAddChild(panel, pList);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, *ppAdd);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	*ppPanel = panel;
	return XUI_OK;
}

static int mapedit_create_material_context_menu(mapedit_app_t* pApp, xui_widget pOwner)
{
	xui_menu_desc_t d;
	xui_menu_item_t items[3];
	int ret;
	if ( pApp == NULL || pOwner == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pOwner = pOwner;
	d.pFont = pApp->pFont;
	ret = xuiMenuCreate(pApp->pContext, &pApp->pMaterialContextMenu, &d);
	if ( ret != XUI_OK ) return ret;
	memset(items, 0, sizeof(items));
	items[0] = (xui_menu_item_t){"查看", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, CMD_MATERIAL_VIEW, 0, NULL, NULL};
	items[1] = (xui_menu_item_t){"重命名", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_MATERIAL_RENAME, 0, NULL, NULL};
	items[2] = (xui_menu_item_t){"编辑图块", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_MATERIAL_EDIT, 0, NULL, NULL};
	ret = xuiMenuSetItems(pApp->pMaterialContextMenu, items, 3);
	if ( ret == XUI_OK ) ret = xuiMenuSetSelect(pApp->pMaterialContextMenu, mapedit_material_context_select, pApp);
	return ret;
}

static int mapedit_create_material_view_window(mapedit_app_t* pApp)
{
	xui_window_desc_t d;
	xui_scroll_view_desc_t sd;
	xui_widget content;
	int ret;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.sTitle = "查看素材";
	d.pFont = pApp->pFont;
	d.bClosed = 1;
	d.bHideCollapse = 1;
	d.bHideMaximize = 1;
	d.bNotResizable = 1;
	d.fMinWidth = 240.0f;
	d.fMinHeight = 180.0f;
	ret = xuiWindowCreate(pApp->pContext, &pApp->pMaterialViewWindow, &d);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWindowSetClose(pApp->pMaterialViewWindow, mapedit_material_view_close_cb, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMaterialViewWindow, XUI_EVENT_KEY_DOWN, mapedit_material_view_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMaterialViewWindow, XUI_EVENT_MASK_KEY_DOWN, 1);
	(void)xuiWidgetSetRect(pApp->pMaterialViewWindow, mapedit_centered_root_rect(pApp, 0.8f, 0.8f));
	memset(&sd, 0, sizeof(sd));
	sd.iSize = sizeof(sd);
	sd.fContentWidth = 320.0f;
	sd.fContentHeight = 240.0f;
	sd.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	sd.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	sd.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	sd.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	sd.iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
	sd.bContentDragEnabled = 1;
	sd.fScrollbarSize = 12.0f;
	sd.fMinThumbSize = 28.0f;
	sd.fWheelStep = 40.0f;
	sd.iBackgroundColor = XUI_COLOR_RGBA(236, 246, 252, 255);
	sd.iTrackColor = XUI_COLOR_RGBA(210, 228, 242, 255);
	sd.iThumbColor = XUI_COLOR_RGBA(78, 140, 198, 255);
	sd.iHoverColor = XUI_COLOR_RGBA(78, 140, 198, 255);
	sd.iActiveColor = XUI_COLOR_RGBA(78, 140, 198, 255);
	sd.iFocusColor = XUI_COLOR_RGBA(78, 140, 198, 255);
	sd.iDisabledColor = XUI_COLOR_RGBA(180, 204, 218, 255);
	ret = xuiScrollViewCreate(pApp->pContext, &pApp->pMaterialViewScroll, &sd);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(pApp->pMaterialViewScroll);
	ret = mapedit_create_render_widget(pApp, &pApp->pMaterialViewCanvas, mapedit_material_view_render, pApp);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pMaterialViewCanvas, (xui_rect_t){0.0f, 0.0f, 320.0f, 240.0f});
	content = xuiScrollViewGetContentWidget(pApp->pMaterialViewScroll);
	ret = xuiWidgetAddChild(content, pApp->pMaterialViewCanvas);
	if ( ret == XUI_OK ) ret = xuiWindowAddChild(pApp->pMaterialViewWindow, pApp->pMaterialViewScroll);
	return ret;
}

static int mapedit_create_material_dialog_button(mapedit_app_t* pApp, xui_widget pParent, const char* sText, xui_widget* ppButton)
{
	xui_button_desc_t d;
	xui_widget b;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.sText = sText;
	d.pFont = pApp->pFont;
	d.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	d.iTextColor = XUI_COLOR_RGBA(34, 62, 90, 255);
	d.iNormalColor = XUI_COLOR_RGBA(244, 249, 253, 255);
	d.iHoverColor = XUI_COLOR_RGBA(228, 240, 251, 255);
	d.iActiveColor = XUI_COLOR_RGBA(210, 230, 248, 255);
	d.iBorderColor = XUI_COLOR_RGBA(136, 166, 196, 255);
	d.fBorderWidth = 1.0f;
	ret = xuiButtonCreate(pApp->pContext, &b, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_size(b, 72.0f, 26.0f);
	(void)xuiButtonSetClick(b, mapedit_material_rename_button_click, pApp);
	ret = xuiWidgetAddChild(pParent, b);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(b);
		return ret;
	}
	*ppButton = b;
	return XUI_OK;
}

static int mapedit_create_material_rename_window(mapedit_app_t* pApp)
{
	xui_window_desc_t wd;
	xui_label_desc_t ld;
	xui_input_desc_t id;
	xui_widget panel;
	xui_widget row;
	xui_widget label;
	xui_widget spacer;
	int ret;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&wd, 0, sizeof(wd));
	wd.iSize = sizeof(wd);
	wd.sTitle = "重命名素材";
	wd.pFont = pApp->pFont;
	wd.bClosed = 1;
	wd.bHideCollapse = 1;
	wd.bHideMaximize = 1;
	wd.fMinWidth = 320.0f;
	wd.fMinHeight = 140.0f;
	ret = xuiWindowCreate(pApp->pContext, &pApp->pMaterialRenameWindow, &wd);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pMaterialRenameWindow, (xui_rect_t){520.0f, 310.0f, 360.0f, 154.0f});
	(void)xuiWidgetSetEventHandler(pApp->pMaterialRenameWindow, XUI_EVENT_KEY_DOWN, mapedit_material_rename_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMaterialRenameWindow, XUI_EVENT_MASK_KEY_DOWN, 1);
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &row);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &spacer);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){12.0f, 12.0f, 12.0f, 12.0f});
	(void)xuiWidgetSetGap(panel, 8.0f);
	mapedit_widget_fill(panel);
	memset(&ld, 0, sizeof(ld));
	ld.iSize = sizeof(ld);
	ld.sText = "映射名称";
	ld.pFont = pApp->pFont;
	ld.iTextColor = XUI_COLOR_RGBA(40, 58, 76, 255);
	ld.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	ret = xuiLabelCreate(pApp->pContext, &label, &ld);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_height(label, 22.0f);
	memset(&id, 0, sizeof(id));
	id.iSize = sizeof(id);
	id.sPlaceholder = "素材映射名称";
	id.pFont = pApp->pFont;
	id.iMaxLength = MAPEDIT_NAME_MAX - 1;
	id.iTextColor = XUI_COLOR_RGBA(34, 52, 70, 255);
	id.iBackgroundColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	id.iBorderColor = XUI_COLOR_RGBA(148, 174, 198, 255);
	id.iFocusBorderColor = XUI_COLOR_RGBA(60, 132, 200, 255);
	id.fBorderWidth = 1.0f;
	ret = xuiInputCreate(pApp->pContext, &pApp->pMaterialRenameInput, &id);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_height(pApp->pMaterialRenameInput, 30.0f);
	(void)xuiWidgetSetLayoutType(row, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(row, 6.0f);
	mapedit_widget_fixed_height(row, 28.0f);
	mapedit_widget_fill(spacer);
	ret = xuiWidgetAddChild(row, spacer);
	if ( ret == XUI_OK ) ret = mapedit_create_material_dialog_button(pApp, row, "确定", &pApp->pMaterialRenameOkButton);
	if ( ret == XUI_OK ) ret = mapedit_create_material_dialog_button(pApp, row, "取消", &pApp->pMaterialRenameCancelButton);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, label);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pApp->pMaterialRenameInput);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, row);
	if ( ret == XUI_OK ) ret = xuiWindowAddChild(pApp->pMaterialRenameWindow, panel);
	return ret;
}

static int mapedit_create_material_edit_button(mapedit_app_t* pApp, xui_widget pParent, const char* sText, xui_widget* ppButton, float fWidth)
{
	xui_button_desc_t d;
	xui_widget b;
	int ret;
	if ( pApp == NULL || pParent == NULL || ppButton == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.sText = sText;
	d.pFont = pApp->pFont;
	d.iNormalColor = XUI_COLOR_RGBA(236, 243, 250, 255);
	d.iHoverColor = XUI_COLOR_RGBA(224, 237, 250, 255);
	d.iActiveColor = XUI_COLOR_RGBA(205, 226, 248, 255);
	d.iBorderColor = XUI_COLOR_RGBA(150, 170, 190, 255);
	d.iTextColor = XUI_COLOR_RGBA(36, 54, 72, 255);
	d.fBorderWidth = 1.0f;
	ret = xuiButtonCreate(pApp->pContext, &b, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_size(b, fWidth, 26.0f);
	(void)xuiButtonSetClick(b, mapedit_material_edit_button_click, pApp);
	ret = xuiWidgetAddChild(pParent, b);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(b);
		return ret;
	}
	*ppButton = b;
	return XUI_OK;
}

static int mapedit_create_material_edit_label(mapedit_app_t* pApp, const char* sText, xui_widget* ppLabel, float fWidth)
{
	xui_label_desc_t d;
	xui_widget label;
	int ret;
	if ( pApp == NULL || ppLabel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.sText = sText;
	d.pFont = pApp->pFont;
	d.iTextColor = XUI_COLOR_RGBA(30, 74, 112, 255);
	d.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	ret = xuiLabelCreate(pApp->pContext, &label, &d);
	if ( ret != XUI_OK ) return ret;
	if ( fWidth > 0.0f ) mapedit_widget_fixed_size(label, fWidth, 26.0f);
	else mapedit_widget_fixed_height(label, 26.0f);
	*ppLabel = label;
	return XUI_OK;
}

static int mapedit_create_material_edit_input(mapedit_app_t* pApp, xui_widget* ppInput, int iMaxLength)
{
	xui_input_desc_t d;
	int ret;
	if ( pApp == NULL || ppInput == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	d.iMaxLength = iMaxLength;
	d.iTextColor = XUI_COLOR_RGBA(35, 52, 70, 255);
	d.iPlaceholderColor = XUI_COLOR_RGBA(118, 132, 148, 255);
	d.iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	d.iBorderColor = XUI_COLOR_RGBA(150, 170, 190, 255);
	d.iFocusBorderColor = XUI_COLOR_RGBA(60, 132, 200, 255);
	d.iSelectionColor = XUI_COLOR_RGBA(174, 214, 255, 255);
	d.iCursorColor = XUI_COLOR_RGBA(35, 52, 70, 255);
	d.fBorderWidth = 1.0f;
	ret = xuiInputCreate(pApp->pContext, ppInput, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_size(*ppInput, 220.0f, 28.0f);
	return XUI_OK;
}

static int mapedit_create_material_edit_scroll(mapedit_app_t* pApp, xui_widget* ppScroll, xui_widget* ppCanvas, xui_widget_cache_render_proc onRender, xui_widget_event_proc onEvent, int iWidth, int iHeight)
{
	xui_scroll_view_desc_t d;
	xui_widget scroll;
	xui_widget content;
	xui_widget canvas;
	int ret;
	if ( pApp == NULL || ppScroll == NULL || ppCanvas == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.fContentWidth = (float)iWidth;
	d.fContentHeight = (float)iHeight;
	d.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	d.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	d.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	d.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	d.iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
	d.bContentDragEnabled = 0;
	d.fScrollbarSize = 12.0f;
	d.fWheelStep = 40.0f;
	d.iBackgroundColor = XUI_COLOR_RGBA(246, 250, 254, 255);
	d.iTrackColor = XUI_COLOR_RGBA(228, 237, 247, 255);
	d.iThumbColor = XUI_COLOR_RGBA(158, 184, 210, 255);
	d.iHoverColor = XUI_COLOR_RGBA(138, 170, 202, 255);
	d.iActiveColor = XUI_COLOR_RGBA(98, 144, 190, 255);
	d.iFocusColor = XUI_COLOR_RGBA(60, 132, 200, 255);
	d.iDisabledColor = XUI_COLOR_RGBA(198, 208, 218, 255);
	ret = xuiScrollViewCreate(pApp->pContext, &scroll, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(scroll);
	content = xuiScrollViewGetContentWidget(scroll);
	ret = mapedit_create_render_widget(pApp, &canvas, onRender, pApp);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(scroll);
		return ret;
	}
	(void)xuiWidgetSetRect(canvas, (xui_rect_t){0, 0, iWidth, iHeight});
	if ( onEvent != NULL ) {
		(void)xuiWidgetSetEventHandler(canvas, XUI_EVENT_POINTER_DOWN, onEvent, pApp);
		(void)xuiWidgetSetEventHandler(canvas, XUI_EVENT_POINTER_MOVE, onEvent, pApp);
		(void)xuiWidgetSetEventHandler(canvas, XUI_EVENT_POINTER_UP, onEvent, pApp);
		(void)xuiWidgetSetEventHandler(canvas, XUI_EVENT_POINTER_WHEEL, onEvent, pApp);
		(void)xuiWidgetSetEventHandler(canvas, XUI_EVENT_POINTER_CAPTURE_LOST, onEvent, pApp);
		(void)xuiWidgetSetEventInterest(canvas, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	}
	ret = xuiWidgetAddChild(content, canvas);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(scroll);
		return ret;
	}
	*ppScroll = scroll;
	*ppCanvas = canvas;
	return XUI_OK;
}

static int mapedit_create_material_edit_pane(mapedit_app_t* pApp, xui_widget* ppPanel, int bSource)
{
	xui_widget panel = NULL;
	xui_widget header = NULL;
	xui_widget label = NULL;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &header);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiWidgetSetGap(panel, 6.0f);
	mapedit_widget_fill(panel);
	(void)xuiWidgetSetLayoutType(header, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(header, 8.0f);
	mapedit_widget_fixed_height(header, 28.0f);
	ret = mapedit_create_material_edit_label(pApp, bSource ? "外部图片" : "原图块", &label, 0.0f);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	mapedit_widget_fill(label);
	ret = xuiWidgetAddChild(header, label);
	if ( ret == XUI_OK && bSource ) ret = mapedit_create_material_edit_button(pApp, header, "打开图片", &pApp->pMaterialEditLoadButton, 92.0f);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, header);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	if ( bSource ) {
		ret = mapedit_create_material_edit_scroll(pApp, &pApp->pMaterialEditSourceScroll, &pApp->pMaterialEditSourceCanvas, mapedit_material_edit_source_render, mapedit_material_edit_source_event, 320, 240);
		if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pApp->pMaterialEditSourceScroll);
	} else {
		ret = mapedit_create_material_edit_scroll(pApp, &pApp->pMaterialEditOutputScroll, &pApp->pMaterialEditOutputCanvas, mapedit_material_edit_output_render, mapedit_material_edit_output_event, MAPEDIT_MATERIAL_STATIC_COLS * MAPEDIT_TILE_W, MAPEDIT_MATERIAL_INITIAL_ROWS * MAPEDIT_TILE_H);
		if ( ret == XUI_OK ) ret = xuiScrollViewSetChange(pApp->pMaterialEditOutputScroll, mapedit_material_edit_output_scroll_change, pApp);
		if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pApp->pMaterialEditOutputScroll);
	}
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	*ppPanel = panel;
	return XUI_OK;
}

static int mapedit_create_material_edit_form(mapedit_app_t* pApp, xui_widget* ppForm)
{
	xui_widget form;
	xui_widget labelName;
	xui_widget labelFile;
	xui_widget spacer;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &form);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &spacer);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(form, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetPadding(form, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiWidgetSetGap(form, 8.0f);
	mapedit_widget_fixed_height(form, 34.0f);
	ret = mapedit_create_material_edit_label(pApp, "图块名称:", &labelName, 72.0f);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_input(pApp, &pApp->pMaterialEditNameInput, MAPEDIT_NAME_MAX - 1);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_label(pApp, "文件名:", &labelFile, 60.0f);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_input(pApp, &pApp->pMaterialEditFileInput, MAPEDIT_FILE_MAX - 1);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(form);
		return ret;
	}
	(void)xuiLabelSetTextColor(labelName, XUI_COLOR_RGBA(31, 75, 112, 255));
	(void)xuiLabelSetTextColor(labelFile, XUI_COLOR_RGBA(31, 75, 112, 255));
	(void)xuiLabelSetTextFlags(labelName, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	(void)xuiLabelSetTextFlags(labelFile, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	(void)xuiInputSetPlaceholder(pApp->pMaterialEditNameInput, "输入图块名称");
	(void)xuiInputSetPlaceholder(pApp->pMaterialEditFileInput, "必须输入文件名");
	mapedit_widget_fixed_size(pApp->pMaterialEditNameInput, 210.0f, 28.0f);
	mapedit_widget_fixed_size(pApp->pMaterialEditFileInput, 240.0f, 28.0f);
	mapedit_widget_fill(spacer);
	ret = xuiWidgetAddChild(form, labelName);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(form, pApp->pMaterialEditNameInput);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(form, labelFile);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(form, pApp->pMaterialEditFileInput);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(form, spacer);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_button(pApp, form, "确定", &pApp->pMaterialEditOkButton, 72.0f);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_button(pApp, form, "取消", &pApp->pMaterialEditCancelButton, 72.0f);
	if ( ret == XUI_OK ) {
		(void)xuiButtonSetSemantic(pApp->pMaterialEditOkButton, XUI_BUTTON_SEMANTIC_PRIMARY);
		mapedit_widget_fixed_size(pApp->pMaterialEditOkButton, 72.0f, 28.0f);
		mapedit_widget_fixed_size(pApp->pMaterialEditCancelButton, 72.0f, 28.0f);
	}
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(form);
		return ret;
	}
	*ppForm = form;
	return XUI_OK;
}

static int mapedit_create_material_edit_window(mapedit_app_t* pApp)
{
	xui_window_desc_t wd;
	xui_split_layout_desc_t sd;
	xui_msgtip_desc_t td;
	xui_widget root;
	xui_widget outputPane;
	xui_widget sourcePane;
	xui_widget form;
	int ret;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&wd, 0, sizeof(wd));
	wd.iSize = sizeof(wd);
	wd.sTitle = "图块编辑";
	wd.pFont = pApp->pFont;
	wd.bClosed = 1;
	wd.bHideCollapse = 1;
	wd.bHideMaximize = 1;
	wd.bNotResizable = 1;
	wd.fTitleBarHeight = 28.0f;
	wd.fBorderWidth = 1.0f;
	wd.fMinWidth = 720.0f;
	wd.fMinHeight = 480.0f;
	wd.iBackgroundColor = XUI_COLOR_RGBA(238, 244, 250, 255);
	wd.iClientColor = XUI_COLOR_RGBA(238, 244, 250, 255);
	wd.iTitleBarColor = XUI_COLOR_RGBA(32, 126, 212, 255);
	wd.iTitleTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	wd.iBorderColor = XUI_COLOR_RGBA(84, 134, 184, 255);
	ret = xuiWindowCreate(pApp->pContext, &pApp->pMaterialEditWindow, &wd);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pMaterialEditWindow, mapedit_centered_root_rect(pApp, 0.8f, 0.8f));
	(void)xuiWindowSetClose(pApp->pMaterialEditWindow, mapedit_material_edit_close_cb, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMaterialEditWindow, XUI_EVENT_KEY_DOWN, mapedit_material_edit_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMaterialEditWindow, XUI_EVENT_MASK_KEY_DOWN, 1);
	memset(&td, 0, sizeof(td));
	td.iSize = sizeof(td);
	td.pFont = pApp->pFont;
	td.iType = XUI_MSGTIP_ICON_ERROR;
	td.sText = "";
	td.fDuration = 2.2f;
	ret = xuiMsgTipCreate(pApp->pContext, &pApp->pMaterialEditMsgTip, &td);
	if ( ret != XUI_OK ) return ret;
	ret = xuiWidgetCreate(pApp->pContext, &root);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(root, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(root, (xui_thickness_t){12.0f, 10.0f, 12.0f, 10.0f});
	(void)xuiWidgetSetGap(root, 10.0f);
	mapedit_widget_fill(root);
	memset(&sd, 0, sizeof(sd));
	sd.iSize = sizeof(sd);
	sd.iOrientation = XUI_ORIENTATION_VERTICAL;
	sd.iPaneCount = 2;
	sd.fDividerSize = 3.0f;
	sd.fDividerVisualSize = 3.0f;
	sd.fDividerHitSize = 12.0f;
	sd.iDividerColor = XUI_COLOR_RGBA(144, 196, 230, 255);
	sd.iDividerHoverColor = XUI_COLOR_RGBA(112, 178, 224, 255);
	sd.iDividerActiveColor = XUI_COLOR_RGBA(76, 154, 210, 255);
	sd.iShadowColor = XUI_COLOR_RGBA(76, 154, 210, 88);
	ret = xuiSplitLayoutCreate(pApp->pContext, &pApp->pMaterialEditSplit, &sd);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(pApp->pMaterialEditSplit);
	(void)xuiSplitLayoutSetPaneMode(pApp->pMaterialEditSplit, 0, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pApp->pMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
	(void)xuiSplitLayoutSetPaneMinSize(pApp->pMaterialEditSplit, 0, MAPEDIT_MATERIAL_STATIC_PANE_WIDTH);
	(void)xuiSplitLayoutSetPaneMinSize(pApp->pMaterialEditSplit, 1, 360.0f);
	ret = mapedit_create_material_edit_pane(pApp, &outputPane, 0);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_pane(pApp, &sourcePane, 1);
	if ( ret == XUI_OK ) ret = mapedit_create_material_edit_form(pApp, &form);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSplitLayoutAddPaneChild(pApp->pMaterialEditSplit, 0, outputPane);
	if ( ret == XUI_OK ) ret = xuiSplitLayoutAddPaneChild(pApp->pMaterialEditSplit, 1, sourcePane);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(root, pApp->pMaterialEditSplit);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(root, form);
	if ( ret == XUI_OK ) ret = xuiWindowAddChild(pApp->pMaterialEditWindow, root);
	return ret;
}

static int mapedit_create_material_manager_panel(mapedit_app_t* pApp, xui_widget pList, xui_widget* ppPanel)
{
	xui_combobox_desc_t cd;
	xui_widget panel;
	int ret;
	memset(&cd, 0, sizeof(cd));
	cd.iSize = sizeof(cd);
	cd.arrItems = g_arrMaterialCategoryNames;
	cd.iItemCount = mapedit_material_category_count();
	cd.iSelected = mapedit_material_category_clamp(pApp->iMaterialCategory);
	cd.pFont = pApp->pFont;
	cd.fItemHeight = MAPEDIT_ROW_HEIGHT;
	cd.fPopupMaxHeight = 160.0f;
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret == XUI_OK ) ret = xuiComboBoxCreate(pApp->pContext, &pApp->pMaterialCategoryCombo, &cd);
	if ( ret != XUI_OK ) return ret;
	(void)xuiComboBoxSetSelect(pApp->pMaterialCategoryCombo, mapedit_material_category_selected, pApp);
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){4.0f, 4.0f, 4.0f, 4.0f});
	(void)xuiWidgetSetGap(panel, 4.0f);
	mapedit_widget_fill(panel);
	mapedit_widget_fixed_height(pApp->pMaterialCategoryCombo, 28.0f);
	if ( mapedit_create_manager_button_widget(pApp, "制作图块", &pApp->pMaterialCreateButton) != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return XUI_ERROR;
	}
	(void)xuiButtonSetSemantic(pApp->pMaterialCreateButton, XUI_BUTTON_SEMANTIC_PRIMARY);
	if ( mapedit_create_material_context_menu(pApp, pList) != XUI_OK ||
	     mapedit_create_material_view_window(pApp) != XUI_OK ||
	     mapedit_create_material_rename_window(pApp) != XUI_OK ||
	     mapedit_create_material_edit_window(pApp) != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return XUI_ERROR;
	}
	(void)xuiWidgetSetEventHandler(pList, XUI_EVENT_POINTER_DOUBLE_CLICK, mapedit_material_list_event, pApp);
	(void)xuiListViewSetContextMenu(pList, mapedit_material_list_context, pApp);
	(void)xuiWidgetSetEventInterest(pList, XUI_EVENT_MASK_DOUBLE_CLICK | XUI_EVENT_MASK_CONTEXT_MENU, 1);
	(void)xuiWidgetSetTooltipResolver(pList, mapedit_material_tooltip_resolve, pApp);
	mapedit_widget_fixed_height(pApp->pMaterialCreateButton, 30.0f);
	mapedit_widget_fill(pList);
	ret = xuiWidgetAddChild(panel, pApp->pMaterialCategoryCombo);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pList);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pApp->pMaterialCreateButton);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	*ppPanel = panel;
	return XUI_OK;
}

static void mapedit_property_set_enum(xui_widget pGrid, int iProperty, const char** arrItems, int iCount, int iSelected)
{
	xui_table_grid_editor_config_t cfg;
	if ( pGrid == NULL || iProperty < 0 || arrItems == NULL || iCount <= 0 ) return;
	memset(&cfg, 0, sizeof(cfg));
	cfg.arrEnumItems = arrItems;
	cfg.iEnumItemCount = iCount;
	cfg.iEnumSelected = iSelected;
	(void)xuiPropertyGridSetEditorConfig(pGrid, iProperty, &cfg);
}

static void mapedit_custom_display_value(const mapedit_custom_channel_def_t* pDef, const char* sValue, char* sOut, int iCap)
{
	if ( sOut == NULL || iCap <= 0 ) return;
	if ( pDef == NULL ) {
		mapedit_copy_text(sOut, iCap, sValue);
		return;
	}
	if ( sValue == NULL || sValue[0] == 0 ) sValue = pDef->sDefaultValue;
	if ( strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		mapedit_copy_text(sOut, iCap, mapedit_custom_enum_display(pDef, sValue));
	} else if ( strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0 ) {
		mapedit_copy_text(sOut, iCap, mapedit_custom_value_as_int(sValue) ? "true" : "false");
	} else {
		mapedit_copy_text(sOut, iCap, sValue);
	}
}

static void mapedit_custom_value_from_property(const mapedit_custom_channel_def_t* pDef, const char* sPropertyValue, char* sOut, int iCap)
{
	if ( sOut == NULL || iCap <= 0 ) return;
	if ( pDef != NULL && strcmp(pDef->sMarkMode, "enum") == 0 && pDef->iOptionCount > 0 ) {
		mapedit_copy_text(sOut, iCap, mapedit_custom_enum_value(pDef, sPropertyValue));
	} else if ( pDef != NULL && (strcmp(pDef->sDataType, "bool") == 0 || strcmp(pDef->sDataType, "boolean") == 0) ) {
		mapedit_copy_text(sOut, iCap, mapedit_custom_value_as_int(sPropertyValue) ? "true" : "false");
	} else {
		mapedit_copy_text(sOut, iCap, sPropertyValue);
	}
	if ( pDef != NULL ) mapedit_custom_clamp_value(pDef, sOut, iCap);
}

static void mapedit_invalidate_map_views(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pMapTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_invalidate_tileset_views(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pTilesetArrangeCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetArrangeCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetPassageCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetPassageCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetActorCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetActorCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTilesetTagsCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTilesetTagsCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pTileSelectCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pTileSelectCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}



static int mapedit_map_resize_doc(mapedit_map_doc_t* pMap, int iWidth, int iHeight)
{
	return mapeditDocumentResize(pMap, iWidth, iHeight) == XGE_MAP_OK ? XUI_OK : XUI_ERROR;
}

static int mapedit_tileset_property_tile(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return -1;
	if ( pApp->iTilesetPropertyTile >= 0 ) return pApp->iTilesetPropertyTile;
	if ( pApp->iTilesetArrangeSelectedTile > 0 && pApp->iTilesetArrangeSelectedTile <= pApp->tTileset.iSpecialCount ) return pApp->iTilesetArrangeSelectedTile;
	if ( pApp->iTilesetTagsSelectedTile > 0 ) return pApp->iTilesetTagsSelectedTile;
	if ( pApp->iTilesetPassageSelectedTile > 0 ) return pApp->iTilesetPassageSelectedTile;
	if ( pApp->iTilesetActorSelectedTile > 0 ) return pApp->iTilesetActorSelectedTile;
	if ( pApp->iSelectedTile > 0 ) return pApp->iSelectedTile;
	return -1;
}

static int mapedit_map_property_cell(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return -1;
	if ( pApp->iMapPropertyMode == MAPEDIT_MAP_PROPERTY_MODE_CELL_CUSTOM && pApp->iMapPropertyCell >= 0 ) return pApp->iMapPropertyCell;
	if ( pApp->iMapTagsSelectedCell >= 0 ) return pApp->iMapTagsSelectedCell;
	if ( pApp->iMapPassageSelectedCell >= 0 ) return pApp->iMapPassageSelectedCell;
	return -1;
}

static int mapedit_tileset_custom_property_channel(const char* sId, const char* sPrefix)
{
	size_t n;
	if ( sId == NULL || sPrefix == NULL ) return 0;
	n = strlen(sPrefix);
	return strncmp(sId, sPrefix, n) == 0 && sId[n] != 0;
}

static void mapedit_refresh_tileset_properties(mapedit_app_t* pApp)
{
	xui_widget pGrid;
	xui_table_grid_editor_config_t cfg;
	char text[128];
	char value[MAPEDIT_CUSTOM_VALUE_MAX];
	int cat;
	int prop;
	int tile;
	int i;
	if ( pApp == NULL || pApp->pTilesetPropertyGrid == NULL ) return;
	pGrid = pApp->pTilesetPropertyGrid;
	pApp->bTilesetPropertyUpdating = 1;
	{
		const char* schema = !pApp->tTileset.bLoaded ? "empty" :
			pApp->iTilesetPropertyMode == MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL &&
				pApp->iTilesetPropertyTile > 0 && pApp->iTilesetPropertyTile <= pApp->tTileset.iSpecialCount ? "tileset.special" :
			pApp->iTilesetPropertyMode == MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM ? "tileset.tile_custom" : "tileset.set";
		if (xuiPropertyGridFindCategory(pGrid, schema) < 0) (void)xuiPropertyGridClear(pGrid);
	}
	if ( !pApp->tTileset.bLoaded ) {
		cat = mapedit_property_category(pGrid, "empty", "图块属性", 1);
		if ( cat >= 0 ) {
			(void)mapedit_property_add(pGrid, cat, "empty.hint", "当前对象", "选择图集后显示图集属性", XUI_TABLE_CELL_TYPE_TEXT, "未选择", "未选择", XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bTilesetPropertyUpdating = 0;
		(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return;
	}
	tile = pApp->iTilesetPropertyTile;
	if ( pApp->iTilesetPropertyMode == MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL ) {
		if ( tile < 1 || tile > pApp->tTileset.iSpecialCount || tile > MAPEDIT_SPECIAL_MAX ) {
			pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
			pApp->iTilesetPropertyTile = -1;
		} else {
			int slot = tile - 1;
			const char* sType = mapedit_tileset_special_type_or_default(pApp->tTileset.arrSpecial[slot].sType);
			const char* sFileDisplay;
			int typeProp;
			int fileProp;
			cat = mapedit_property_category(pGrid, "tileset.special", "特殊图块", 1);
			if ( cat >= 0 ) {
				snprintf(text, sizeof(text), "%d", tile);
				(void)mapedit_property_add(pGrid, cat, "special.tile_id", "图块ID", "图集中的图块 ID", XUI_TABLE_CELL_TYPE_INT, text, text, XUI_PROPERTY_FLAG_READONLY);
				snprintf(text, sizeof(text), "%d", slot);
				(void)mapedit_property_add(pGrid, cat, "special.slot_index", "槽位序号", "特殊图块数组中的序号，从 0 开始", XUI_TABLE_CELL_TYPE_INT, text, text, XUI_PROPERTY_FLAG_READONLY);
				mapedit_tileset_special_file_options_build(pApp, sType, pApp->tTileset.arrSpecial[slot].sFile);
				sFileDisplay = mapedit_tileset_special_file_display_for_value(pApp, pApp->tTileset.arrSpecial[slot].sFile);
				typeProp = mapedit_property_add(pGrid, cat, "special.type", "特殊图块类型", "选择这个特殊槽位使用的素材类型", XUI_TABLE_CELL_TYPE_ENUM, sType, "动态图块", 0);
				if ( typeProp >= 0 ) {
					memset(&cfg, 0, sizeof(cfg));
					cfg.arrEnumItems = g_arrTilesetSpecialTypeNames;
					cfg.iEnumItemCount = mapedit_tileset_special_type_count();
					cfg.iEnumSelected = mapedit_tileset_special_type_find(sType);
					(void)xuiPropertyGridSetEditorConfig(pGrid, typeProp, &cfg);
					if (xuiPropertyGridGetSelected(pGrid) < 0) (void)xuiPropertyGridSetSelected(pGrid, typeProp);
				}
				fileProp = mapedit_property_add(pGrid, cat, "special.file", "特殊图块文件", "选择对应素材目录下的图块文件，显示为映射名和文件名", XUI_TABLE_CELL_TYPE_ENUM, sFileDisplay, "未设置", 0);
				if ( fileProp >= 0 ) {
					memset(&cfg, 0, sizeof(cfg));
					cfg.arrEnumItems = pApp->tTilesetSpecialFileOptions.arrTextPtrs;
					cfg.iEnumItemCount = pApp->tTilesetSpecialFileOptions.iCount;
					cfg.iEnumSelected = mapedit_tileset_special_file_find_value(pApp, pApp->tTileset.arrSpecial[slot].sFile);
					(void)xuiPropertyGridSetEditorConfig(pGrid, fileProp, &cfg);
				}
			}
			pApp->bTilesetPropertyUpdating = 0;
			(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return;
		}
	}
	if ( pApp->iTilesetPropertyMode == MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM ) {
		tile = pApp->iTilesetPropertyTile;
		if ( tile < 0 ) {
			cat = mapedit_property_category(pGrid, "empty", "图块属性", 1);
			if ( cat >= 0 ) {
				(void)mapedit_property_add(pGrid, cat, "empty.hint", "当前对象", "选择图块后显示图块属性", XUI_TABLE_CELL_TYPE_TEXT, "未选择", "未选择", XUI_PROPERTY_FLAG_READONLY);
			}
			pApp->bTilesetPropertyUpdating = 0;
			(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return;
		}
		cat = mapedit_property_category(pGrid, "tileset.tile_custom", "图块自定义数据", 1);
		if ( cat >= 0 ) {
			snprintf(text, sizeof(text), "%d", tile);
			prop = mapedit_property_add(pGrid, cat, "tile_custom.tile_id", "图块ID", "当前查看的图块 ID", XUI_TABLE_CELL_TYPE_INT, text, text, XUI_PROPERTY_FLAG_READONLY);
			if (prop >= 0 && xuiPropertyGridGetSelected(pGrid) < 0) (void)xuiPropertyGridSetSelected(pGrid, prop);
			for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
				mapedit_custom_channel_def_t* pDef = &pApp->arrCustomChannels[i];
				char propId[MAPEDIT_NAME_MAX + 32];
				if ( !mapedit_custom_is_tileset_channel(pDef) ) continue;
				if ( !mapedit_custom_get_value(pApp->tTileset.pTileCustomRaw, pDef->sId, SET_KEY_TILE, tile, value, sizeof(value)) ) mapedit_copy_text(value, sizeof(value), pDef->sDefaultValue);
				mapedit_custom_display_value(pDef, value, value, sizeof(value));
				snprintf(propId, sizeof(propId), "tile_custom.%s", pDef->sId);
				(void)mapedit_property_add(pGrid, cat, propId, pDef->sName[0] ? pDef->sName : pDef->sId, pDef->sId, XUI_TABLE_CELL_TYPE_TEXT, value, "", XUI_PROPERTY_FLAG_READONLY);
			}
		}
		pApp->bTilesetPropertyUpdating = 0;
		(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return;
	}
	cat = mapedit_property_category(pGrid, "tileset.set", "图集", 1);
	if ( cat >= 0 ) {
		prop = mapedit_property_add(pGrid, cat, "tileset.name", "图集名称", "当前图集的显示名称", XUI_TABLE_CELL_TYPE_TEXT, pApp->tTileset.sName, "默认图集", 0);
		if (prop >= 0 && xuiPropertyGridGetSelected(pGrid) < 0) (void)xuiPropertyGridSetSelected(pGrid, prop);
		snprintf(text, sizeof(text), "%d", pApp->tTileset.iSpecialCount);
		(void)mapedit_property_add(pGrid, cat, "tileset.special_tile_count", "特殊图块数量", "独立素材图块槽位数量，0 号图块仍保留为橡皮", XUI_TABLE_CELL_TYPE_INT, text, "0", 0);
		prop = mapedit_property_add(pGrid, cat, "tileset.static_tilesheet", "静态图块集", "从 assets/tilesheets 中选择静态图块集素材", XUI_TABLE_CELL_TYPE_ENUM, pApp->tTileset.sStaticFile, "未设置", 0);
		if ( prop >= 0 && pApp->tMaterialFiles.iCount > 0 ) mapedit_property_set_enum(pGrid, prop, pApp->tMaterialFiles.arrPtrs, pApp->tMaterialFiles.iCount, mapedit_file_list_find(&pApp->tMaterialFiles, pApp->tTileset.sStaticFile));
		(void)mapedit_property_add(pGrid, cat, "tileset.custom_data", "自定义数据", "图集级自定义数据，后续可用于通道默认值或扩展配置", XUI_TABLE_CELL_TYPE_TEXTAREA, pApp->tTileset.sCustomData, "", 0);
	}
	pApp->bTilesetPropertyUpdating = 0;
	(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void mapedit_refresh_map_properties(mapedit_app_t* pApp)
{
	xui_widget pGrid;
	char text[128];
	char value[MAPEDIT_CUSTOM_VALUE_MAX];
	const char* sTilesetDisplay;
	int cat;
	int prop;
	int cell;
	int x;
	int y;
	int i;
	if ( pApp == NULL || pApp->pMapPropertyGrid == NULL ) return;
	pGrid = pApp->pMapPropertyGrid;
	pApp->bMapPropertyUpdating = 1;
	{
		const char* schema = !pApp->tMap.pTiles ? "empty" :
			pApp->iMapPropertyMode == MAPEDIT_MAP_PROPERTY_MODE_CELL_CUSTOM &&
				pApp->iMapPropertyCell >= 0 && pApp->iMapPropertyCell < pApp->tMap.iWidth * pApp->tMap.iHeight ? "map.cell_custom" : "map";
		if (xuiPropertyGridFindCategory(pGrid, schema) < 0) (void)xuiPropertyGridClear(pGrid);
	}
	if ( pApp->tMap.pTiles == NULL ) {
		cat = mapedit_property_category(pGrid, "empty", "地图属性", 1);
		if ( cat >= 0 ) {
			(void)mapedit_property_add(pGrid, cat, "empty.hint", "当前对象", "选择地图后显示地图属性", XUI_TABLE_CELL_TYPE_TEXT, "未选择", "未选择", XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bMapPropertyUpdating = 0;
		(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return;
	}
	cell = (pApp->iMapPropertyMode == MAPEDIT_MAP_PROPERTY_MODE_CELL_CUSTOM) ? pApp->iMapPropertyCell : -1;
	if ( cell >= 0 && pApp->tMap.iWidth > 0 && pApp->tMap.iHeight > 0 ) {
		x = cell % pApp->tMap.iWidth;
		y = cell / pApp->tMap.iWidth;
		if ( y >= 0 && y < pApp->tMap.iHeight ) {
			cat = mapedit_property_category(pGrid, "map.cell_custom", "地图位置自定义数据", 1);
			if ( cat >= 0 ) {
				snprintf(text, sizeof(text), "%d, %d", x, y);
				prop = mapedit_property_add(pGrid, cat, "cell.position", "地图位置", "当前检查的地图位置", XUI_TABLE_CELL_TYPE_TEXT, text, text, XUI_PROPERTY_FLAG_READONLY);
				if (prop >= 0 && xuiPropertyGridGetSelected(pGrid) < 0) (void)xuiPropertyGridSetSelected(pGrid, prop);
				snprintf(text, sizeof(text), "%d", mapedit_map_top_tile(pApp, x, y));
				(void)mapedit_property_add(pGrid, cat, "cell.tile", "参考图块", "当前位置最上层非空图块", XUI_TABLE_CELL_TYPE_INT, text, text, XUI_PROPERTY_FLAG_READONLY);
				snprintf(text, sizeof(text), "%u", (unsigned int)mapedit_map_get_cell_passage(pApp, x, y));
				(void)mapedit_property_add(pGrid, cat, "cell.passage", "通行数据", "当前位置最终通行修正值", XUI_TABLE_CELL_TYPE_INT, text, text, XUI_PROPERTY_FLAG_READONLY);
				for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
					mapedit_custom_channel_def_t* pDef = &pApp->arrCustomChannels[i];
					char propId[MAPEDIT_NAME_MAX + 32];
					if ( !mapedit_custom_is_map_channel(pDef) ) continue;
					if ( !mapedit_custom_get_value(pApp->tMap.pCellDataRaw, pDef->sId, MAP_KEY_CELL, cell, value, sizeof(value)) ) mapedit_copy_text(value, sizeof(value), pDef->sDefaultValue);
					mapedit_custom_display_value(pDef, value, value, sizeof(value));
					snprintf(propId, sizeof(propId), "cell_custom.%s", pDef->sId);
					(void)mapedit_property_add(pGrid, cat, propId, pDef->sName[0] ? pDef->sName : pDef->sId, pDef->sId, XUI_TABLE_CELL_TYPE_TEXT, value, "", XUI_PROPERTY_FLAG_READONLY);
				}
			}
			pApp->bMapPropertyUpdating = 0;
			(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return;
		}
		pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_MAP;
		pApp->iMapPropertyCell = -1;
	}
	cat = mapedit_property_category(pGrid, "map", "地图", 1);
	if ( cat >= 0 ) {
		prop = mapedit_property_add(pGrid, cat, "map.name", "地图名称", "当前地图的显示名称", XUI_TABLE_CELL_TYPE_TEXT, pApp->tMap.sName, "默认地图", 0);
		if (prop >= 0 && xuiPropertyGridGetSelected(pGrid) < 0) (void)xuiPropertyGridSetSelected(pGrid, prop);
		mapedit_map_tileset_options_build(pApp, pApp->tMap.sTileset);
		sTilesetDisplay = mapedit_map_tileset_display_for_value(pApp, pApp->tMap.sTileset);
		prop = mapedit_property_add(pGrid, cat, "map.tileset", "图集", "地图使用的图集", XUI_TABLE_CELL_TYPE_ENUM, sTilesetDisplay, "未设置", 0);
		if ( prop >= 0 && pApp->tMapTilesetOptions.iCount > 0 ) mapedit_property_set_enum(pGrid, prop, pApp->tMapTilesetOptions.arrTextPtrs, pApp->tMapTilesetOptions.iCount, mapedit_map_tileset_option_find_value(pApp, pApp->tMap.sTileset));
		snprintf(text, sizeof(text), "%d", pApp->tMap.iState);
		(void)mapedit_property_add(pGrid, cat, "map.state", "当前状态", "地图当前状态，用于多状态图块显示", XUI_TABLE_CELL_TYPE_INT, text, "0", 0);
		snprintf(text, sizeof(text), "%d", pApp->tMap.iWidth);
		(void)mapedit_property_add(pGrid, cat, "map.width", "地图宽度", "横向图块数量", XUI_TABLE_CELL_TYPE_INT, text, "100", 0);
		snprintf(text, sizeof(text), "%d", pApp->tMap.iHeight);
		(void)mapedit_property_add(pGrid, cat, "map.height", "地图高度", "纵向图块数量", XUI_TABLE_CELL_TYPE_INT, text, "100", 0);
		(void)mapedit_property_add(pGrid, cat, "map.custom_data", "自定义数据", "地图级自定义数据", XUI_TABLE_CELL_TYPE_TEXTAREA, pApp->tMap.sCustomData, "", 0);
	}
	pApp->bMapPropertyUpdating = 0;
	(void)xuiWidgetInvalidate(pGrid, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int mapedit_property_integer(const char* text, int* number)
{
	char* end = NULL;
	long long value;
	errno = 0;
	value = strtoll(text, &end, 10);
	if (errno == ERANGE || end == text || value < INT_MIN || value > INT_MAX) return 0;
	while (*end == ' ' || *end == '\t') ++end;
	if (*end) return 0;
	*number = (int)value;
	return 1;
}

static void mapedit_tileset_property_change(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int tile;
	int reload = 0;
	int save = 0;
	int number = 0;
	(void)pWidget;
	(void)iProperty;
	(void)iType;
	if ( pApp == NULL || pApp->bTilesetPropertyUpdating || sId == NULL || sValue == NULL ) return;
	if (strcmp(sId, "tileset.special_tile_count") == 0 || strcmp(sId, "tileset.special_count") == 0 || strcmp(sId, "tile.passage") == 0) {
		if (!mapedit_property_integer(sValue, &number)) { mapedit_status(pApp, "请输入有效整数"); mapedit_refresh_tileset_properties(pApp); return; }
	}
	if (!mapedit_capture_tileset(pApp)) { mapedit_refresh_tileset_properties(pApp); return; }
	tile = mapedit_tileset_property_tile(pApp);
	if ( strcmp(sId, "tileset.name") == 0 ) {
		mapedit_copy_text(pApp->tTileset.sName, sizeof(pApp->tTileset.sName), sValue);
		save = 1;
	} else if ( strcmp(sId, "tileset.static_tilesheet") == 0 || strcmp(sId, "tileset.static") == 0 ) {
		mapedit_copy_text(pApp->tTileset.sStaticFile, sizeof(pApp->tTileset.sStaticFile), sValue);
		save = 1;
		reload = 1;
	} else if ( strcmp(sId, "tileset.special_tile_count") == 0 || strcmp(sId, "tileset.special_count") == 0 ) {
		int count = number;
		if ( count < 0 ) count = 0;
		if ( count > MAPEDIT_SPECIAL_MAX ) count = MAPEDIT_SPECIAL_MAX;
		pApp->tTileset.iSpecialCount = count;
		if ( pApp->iTilesetPropertyMode == MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL && pApp->iTilesetPropertyTile > count ) {
			pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
			pApp->iTilesetPropertyTile = -1;
		}
		save = 1;
		reload = 1;
	} else if ( strcmp(sId, "tileset.custom_data") == 0 || strcmp(sId, "tileset.custom") == 0 ) {
		mapedit_copy_text(pApp->tTileset.sCustomData, sizeof(pApp->tTileset.sCustomData), sValue);
		save = 1;
	} else if ( strcmp(sId, "tile.passage") == 0 && tile > 0 && tile < (int)sizeof(pApp->tTileset.arrPassage) ) {
		int v = number;
		if ( v < 0 ) v = 0;
		if ( v > 255 ) v = 255;
		pApp->tTileset.arrPassage[tile] = (unsigned char)v;
		if ( pApp->tTileset.iPassageCount <= tile ) pApp->tTileset.iPassageCount = tile + 1;
		pApp->iTilesetPassageSelectedTile = tile;
		save = 1;
	} else if ( strcmp(sId, "tile.actor") == 0 && tile > 0 && tile < (int)sizeof(pApp->tTileset.arrActorOverlay) ) {
		pApp->tTileset.arrActorOverlay[tile] = (unsigned char)(mapedit_custom_value_as_int(sValue) ? 1 : 0);
		if ( pApp->tTileset.iActorOverlayCount <= tile ) pApp->tTileset.iActorOverlayCount = tile + 1;
		pApp->iTilesetActorSelectedTile = tile;
		save = 1;
	} else if ( strcmp(sId, "special.type") == 0 && tile > 0 && tile <= pApp->tTileset.iSpecialCount ) {
		mapedit_copy_text(pApp->tTileset.arrSpecial[tile - 1].sType, sizeof(pApp->tTileset.arrSpecial[tile - 1].sType), mapedit_tileset_special_type_or_default(sValue));
		pApp->tTileset.arrSpecial[tile - 1].sFile[0] = 0;
		save = 1;
		reload = 1;
	} else if ( strcmp(sId, "special.file") == 0 && tile > 0 && tile <= pApp->tTileset.iSpecialCount ) {
		mapedit_copy_text(pApp->tTileset.arrSpecial[tile - 1].sFile, sizeof(pApp->tTileset.arrSpecial[tile - 1].sFile), mapedit_tileset_special_file_value_for_display(pApp, sValue));
		save = 1;
		reload = 1;
	} else if ( tile > 0 && mapedit_tileset_custom_property_channel(sId, "tile_custom.") ) {
		mapedit_custom_channel_def_t* pDef = mapedit_custom_channel_by_id(pApp, sId + strlen("tile_custom."));
		char value[MAPEDIT_CUSTOM_VALUE_MAX];
		mapedit_custom_value_from_property(pDef, sValue, value, sizeof(value));
		if ( pDef != NULL && mapedit_custom_set_value(&pApp->tTileset.pTileCustomRaw, pDef->sId, SET_KEY_TILE, tile, value) == XUI_OK ) save = 1;
	}
	if ( save ) {
		mapedit_mark_tileset_dirty(pApp);
		if (reload) mapedit_tileset_refresh_surfaces(pApp);
		mapedit_status(pApp, "图集属性已修改（未保存，可撤销）");
		mapedit_refresh_tileset_list_texts(pApp, &pApp->tTilesetFiles);
		if ( pApp->pTilesetListView != NULL ) {
			(void)xuiListViewSetItems(pApp->pTilesetListView, pApp->tTilesetFiles.arrTextPtrs, pApp->tTilesetFiles.iCount);
			if ( pApp->tTilesetFiles.iSelected >= 0 ) (void)xuiListViewSetSelected(pApp->pTilesetListView, pApp->tTilesetFiles.iSelected);
		}
	}
	if (!save) mapeditHistoryBegin(&pApp->tTilesetHistory);
	mapedit_invalidate_tileset_views(pApp);
	mapedit_invalidate_map_views(pApp);
	mapedit_refresh_tileset_properties(pApp);
}

static void mapedit_map_property_change(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	int save = 0;
	int number = 0;
	int reloadTileset = 0;
	int resized = 0;
	int cell;
	(void)pWidget;
	(void)iProperty;
	(void)iType;
	if ( pApp == NULL || pApp->bMapPropertyUpdating || sId == NULL || sValue == NULL ) return;
	if (strcmp(sId, "map.width") == 0 || strcmp(sId, "map.height") == 0 || strcmp(sId, "map.state") == 0 || strcmp(sId, "cell.passage") == 0) {
		int width, height;
		if (!mapedit_property_integer(sValue, &number)) { mapedit_status(pApp, "请输入有效整数"); mapedit_refresh_map_properties(pApp); return; }
		width = strcmp(sId, "map.width") == 0 ? number : pApp->tMap.iWidth;
		height = strcmp(sId, "map.height") == 0 ? number : pApp->tMap.iHeight;
		if (xgeMapValidateSize(width, height, pApp->tMap.iLayers, NULL, NULL) != XGE_MAP_OK) {
			mapedit_status(pApp, "尺寸无效：单边最多 2048，总格数最多 1048576；原地图未改动");
			mapedit_refresh_map_properties(pApp); return;
		}
	}
	if (strcmp(sId, "map.tileset") == 0 && pApp->tTileset.bDirty) {
		mapedit_status(pApp, "请先保存或撤销当前图集的修改，再更换地图引用");
		mapedit_refresh_map_properties(pApp);
		return;
	}
	{
		int flags = MAPEDIT_SNAPSHOT_META;
		if (strcmp(sId, "map.width") == 0 || strcmp(sId, "map.height") == 0)
			flags |= MAPEDIT_SNAPSHOT_SIZE | MAPEDIT_SNAPSHOT_PASSAGE | MAPEDIT_SNAPSHOT_CUSTOM;
		else if (strcmp(sId, "cell.passage") == 0) flags = MAPEDIT_SNAPSHOT_PASSAGE;
		else if (strncmp(sId, "cell_custom.", 12) == 0) flags = MAPEDIT_SNAPSHOT_CUSTOM;
		if (!mapedit_capture_map(pApp, flags)) { mapedit_refresh_map_properties(pApp); return; }
	}
	if ( strcmp(sId, "map.name") == 0 ) {
		mapedit_copy_text(pApp->tMap.sName, sizeof(pApp->tMap.sName), sValue);
		save = 1;
	} else if ( strcmp(sId, "map.width") == 0 || strcmp(sId, "map.height") == 0 ) {
		int w = strcmp(sId, "map.width") == 0 ? number : pApp->tMap.iWidth;
		int h = strcmp(sId, "map.height") == 0 ? number : pApp->tMap.iHeight;
		if ( mapedit_map_resize_doc(&pApp->tMap, w, h) == XUI_OK ) {
			mapedit_map_scroll_reset(pApp);
			mapedit_map_passage_scroll_reset(pApp);
			mapedit_map_tags_scroll_reset(pApp);
			pApp->iMapTagsSelectedCell = -1;
			pApp->iMapPassageSelectedCell = -1;
			pApp->iMapHoverX = -1;
			pApp->iMapHoverY = -1;
			pApp->iMapTagsHoverX = -1;
			pApp->iMapTagsHoverY = -1;
			resized = 1;
			save = 1;
		} else {
			mapedit_status(pApp, "地图尺寸已修改，但图块数据重建失败");
		}
	} else if ( strcmp(sId, "map.state") == 0 ) {
		pApp->tMap.iState = mapedit_setup_clamp_state(pApp, number);
		save = 1;
	} else if ( strcmp(sId, "map.tileset") == 0 ) {
		mapedit_copy_text(pApp->tMap.sTileset, sizeof(pApp->tMap.sTileset), mapedit_map_tileset_value_for_display(pApp, sValue));
		pApp->iSelectedTile = -1;
		pApp->iBrushW = 1;
		pApp->iBrushH = 1;
		pApp->iTileSelectHoverCol = -1;
		pApp->iTileSelectHoverRow = -1;
		mapedit_tile_select_scroll_reset(pApp);
		mapedit_reset_tileset_panel_scrolls(pApp);
		save = 1;
		reloadTileset = 1;
	} else if ( strcmp(sId, "map.custom_data") == 0 || strcmp(sId, "map.custom") == 0 ) {
		mapedit_copy_text(pApp->tMap.sCustomData, sizeof(pApp->tMap.sCustomData), sValue);
		save = 1;
	} else if ( strcmp(sId, "cell.passage") == 0 ) {
		cell = mapedit_map_property_cell(pApp);
		if ( cell >= 0 && pApp->tMap.iWidth > 0 ) {
			int x = cell % pApp->tMap.iWidth;
			int y = cell / pApp->tMap.iWidth;
			int v = number;
			if ( v < 0 ) v = 0;
			if ( v > 255 ) v = 255;
			if ( mapedit_map_set_cell_passage(pApp, x, y, (unsigned char)v) == XUI_OK ) save = 1;
		}
	} else if ( mapedit_tileset_custom_property_channel(sId, "cell_custom.") ) {
		cell = mapedit_map_property_cell(pApp);
		if ( cell >= 0 ) {
			mapedit_custom_channel_def_t* pDef = mapedit_custom_channel_by_id(pApp, sId + strlen("cell_custom."));
			char value[MAPEDIT_CUSTOM_VALUE_MAX];
			int x = pApp->tMap.iWidth > 0 ? cell % pApp->tMap.iWidth : -1;
			int y = pApp->tMap.iWidth > 0 ? cell / pApp->tMap.iWidth : -1;
			mapedit_custom_value_from_property(pDef, sValue, value, sizeof(value));
			if ( pDef != NULL && x >= 0 && y >= 0 && mapedit_map_set_cell_custom_value(pApp, pDef, x, y, value) == XUI_OK ) save = 1;
		}
	}
	if ( save ) {
		mapedit_mark_map_dirty(pApp);
		mapedit_status(pApp, "地图属性已修改（未保存）");
		if ( reloadTileset ) (void)mapedit_tileset_load(pApp, pApp->tMap.sTileset);
	}
	if (!save) mapeditHistoryBegin(&pApp->tMapHistory);
	mapedit_invalidate_map_views(pApp);
	mapedit_invalidate_tileset_views(pApp);
	if ( resized ) mapedit_update_status_details(pApp);
	mapedit_refresh_map_properties(pApp);
}

static int mapedit_create_property_grid(mapedit_app_t* pApp, xui_widget* ppGrid, int bMap)
{
	xui_property_grid_desc_t d;
	xui_widget w;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	d.fNameWidth = 148.0f;
	d.fRowHeight = MAPEDIT_ROW_HEIGHT;
	d.fCategoryHeight = 32.0f;
	d.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	d.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	d.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	ret = xuiPropertyGridCreate(pApp->pContext, &w, &d);
	if ( ret != XUI_OK ) return ret;
	(void)xuiPropertyGridSetChange(w, bMap ? mapedit_map_property_change : mapedit_tileset_property_change, pApp);
	*ppGrid = w;
	return XUI_OK;
}

static void mapedit_configure_dock_metrics(xui_widget dock)
{
	xui_dock_panel_metrics_t metrics;
	memset(&metrics, 0, sizeof(metrics));
	metrics.iSize = sizeof(metrics);
	if (xuiDockPanelGetMetrics(dock, &metrics) != XUI_OK) return;
	metrics.fCaptionHeight = 32.0f;
	metrics.fTabStripHeight = 34.0f;
	metrics.fFloatTitleHeight = 34.0f;
	metrics.fAutoHideStripSize = 30.0f;
	metrics.fTabMinWidth = 100.0f;
	(void)xuiDockPanelSetMetrics(dock, &metrics);
}

static int mapedit_add_dock_window(xui_widget pDock, const char* sId, const char* sTitle, xui_widget pClient, int* pWindow, int iRegion, int iSide, float fRatio, int* pPane)
{
	int ret;
	ret = xuiDockPanelAddWindow(pDock, sTitle, pClient, pWindow);
	if ( ret != XUI_OK ) return ret;
	(void)xuiDockPanelSetWindowUserData(pDock, *pWindow, (void*)sId);
	return xuiDockPanelDockWindow(pDock, *pWindow, iRegion, iSide, fRatio, pPane);
}

static int mapedit_create_placeholder(mapedit_app_t* pApp, xui_widget* ppWidget, const char* sText)
{
	return mapedit_create_render_widget(pApp, ppWidget, mapedit_simple_panel_render, (void*)sText);
}

static int mapedit_create_tileset_workspace(mapedit_app_t* pApp)
{
	xui_dock_panel_desc_t d;
	xui_widget materials;
	xui_widget sets;
	xui_widget props;
	xui_widget tagsPanel;
	xui_widget materialsPanel;
	xui_widget setsPanel;
	int paneDoc;
	int paneLeft;
	int win;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	ret = xuiDockPanelCreate(pApp->pContext, &pApp->pTilesetDock, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(pApp->pTilesetDock);
	(void)xuiWidgetSetDock(pApp->pTilesetDock, XUI_DOCK_FILL);
	mapedit_configure_dock_metrics(pApp->pTilesetDock);
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pTilesetDock);
	if ( mapedit_create_list(pApp, &materials, &pApp->tMaterialCategoryFiles, mapedit_material_selected) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_list(pApp, &sets, &pApp->tTilesetFiles, mapedit_tileset_selected) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_material_manager_panel(pApp, materials, &materialsPanel) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_list_manager_panel(pApp, sets, &setsPanel, &pApp->pTilesetAddButton, "添加图集") != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_property_grid(pApp, &props, 0) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pTilesetArrangeCanvas, mapedit_tile_panel_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pTilesetPassageCanvas, mapedit_tile_panel_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pTilesetActorCanvas, mapedit_tile_panel_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pTilesetTagsCanvas, mapedit_tile_panel_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pTilesetArrangeScrollFrame, pApp->pTilesetArrangeCanvas, mapedit_tileset_panel_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pTilesetPassageScrollFrame, pApp->pTilesetPassageCanvas, mapedit_tileset_panel_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pTilesetActorScrollFrame, pApp->pTilesetActorCanvas, mapedit_tileset_panel_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pTilesetTagsScrollFrame, pApp->pTilesetTagsCanvas, mapedit_tileset_panel_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_tags_panel(pApp, 0, &tagsPanel) != XUI_OK ) return XUI_ERROR;
	(void)xuiWidgetSetEventHandler(pApp->pTilesetArrangeCanvas, XUI_EVENT_POINTER_DOWN, mapedit_tileset_arrange_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetArrangeCanvas, XUI_EVENT_POINTER_MOVE, mapedit_tileset_arrange_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetArrangeCanvas, XUI_EVENT_POINTER_LEAVE, mapedit_tileset_arrange_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetArrangeCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_tileset_arrange_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pTilesetArrangeCanvas, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetPassageCanvas, XUI_EVENT_POINTER_DOWN, mapedit_tileset_passage_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetPassageCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_tileset_passage_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pTilesetPassageCanvas, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetActorCanvas, XUI_EVENT_POINTER_DOWN, mapedit_tileset_actor_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetActorCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_tileset_actor_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pTilesetActorCanvas, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetTagsCanvas, XUI_EVENT_POINTER_DOWN, mapedit_tileset_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetTagsCanvas, XUI_EVENT_POINTER_MOVE, mapedit_tileset_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetTagsCanvas, XUI_EVENT_POINTER_LEAVE, mapedit_tileset_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTilesetTagsCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_tileset_tags_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pTilesetTagsCanvas, XUI_EVENT_MASK_POINTER, 1);
	ret = mapedit_add_dock_window(pApp->pTilesetDock, "tileset.set_layout", "图集编排", pApp->pTilesetArrangeScrollFrame, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pTilesetDock, "通行编辑", pApp->pTilesetPassageScrollFrame, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pTilesetDock, win, (void*)"tileset.passage");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pTilesetDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pTilesetDock, "角色覆盖", pApp->pTilesetActorScrollFrame, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pTilesetDock, win, (void*)"tileset.actor_overlay");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pTilesetDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pTilesetDock, "图块打标", tagsPanel, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pTilesetDock, win, (void*)"tileset.tags");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pTilesetDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = mapedit_add_dock_window(pApp->pTilesetDock, "tileset.materials", "素材管理", materialsPanel, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.22f, &paneLeft);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pTilesetDock, "图集管理", setsPanel, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pTilesetDock, win, (void*)"tileset.set");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pTilesetDock, win, paneLeft);
	if ( ret == XUI_OK ) ret = mapedit_add_dock_window(pApp->pTilesetDock, "tileset.properties", "图块属性", props, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.22f, NULL);
	pApp->pTilesetListView = sets;
	pApp->pMaterialListView = materials;
	pApp->pTilesetListPanel = setsPanel;
	pApp->pMaterialListPanel = materialsPanel;
	pApp->pTilesetPropertyGrid = props;
	mapedit_refresh_tileset_properties(pApp);
	return ret;
}

static int mapedit_create_map_edit_panel(mapedit_app_t* pApp, xui_widget* ppPanel)
{
	xui_widget panel = NULL;
	xui_widget row = NULL;
	xui_widget commandRow = NULL;
	xui_widget spacer = NULL;
	xui_widget viewport = NULL;
	xui_scroll_frame_desc_t scrollDesc;
	int ret;
	if ( pApp == NULL || ppPanel == NULL || pApp->pMapToolbar == NULL || pApp->pLayerCombo == NULL || pApp->pMapCommandToolbar == NULL || pApp->pMapCanvas == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = xuiWidgetCreate(pApp->pContext, &panel);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &row);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &spacer);
	if ( ret == XUI_OK ) ret = xuiWidgetCreate(pApp->pContext, &commandRow);
	if ( ret == XUI_OK ) {
		memset(&scrollDesc, 0, sizeof(scrollDesc));
		scrollDesc.iSize = sizeof(scrollDesc);
		scrollDesc.fContentWidth = 1.0f;
		scrollDesc.fContentHeight = 1.0f;
		scrollDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
		scrollDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
		scrollDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
		scrollDesc.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
		scrollDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
		scrollDesc.bContentDragEnabled = 0;
		scrollDesc.fScrollbarSize = MAPEDIT_MAP_EDIT_SCROLLBAR_SIZE;
		scrollDesc.fMinThumbSize = 28.0f;
		scrollDesc.fButtonSize = 0.0f;
		scrollDesc.fWheelStep = 40.0f;
		scrollDesc.iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
		scrollDesc.iTrackColor = XUI_COLOR_RGBA(226, 240, 249, 255);
		scrollDesc.iThumbColor = XUI_COLOR_RGBA(90, 164, 214, 255);
		scrollDesc.iHoverColor = XUI_COLOR_RGBA(108, 180, 226, 255);
		scrollDesc.iActiveColor = XUI_COLOR_RGBA(68, 142, 204, 255);
		scrollDesc.iFocusColor = XUI_COLOR_RGBA(108, 180, 226, 255);
		scrollDesc.iDisabledColor = XUI_COLOR_RGBA(180, 204, 218, 255);
		scrollDesc.iButtonColor = XUI_COLOR_RGBA(226, 240, 249, 255);
		scrollDesc.iButtonIconColor = XUI_COLOR_RGBA(90, 164, 214, 255);
		scrollDesc.iCornerColor = XUI_COLOR_RGBA(226, 240, 249, 255);
		scrollDesc.iGripColor = XUI_COLOR_RGBA(90, 164, 214, 255);
		ret = xuiScrollFrameCreate(pApp->pContext, &pApp->pMapScrollFrame, &scrollDesc);
	}
	if ( ret != XUI_OK ) {
		if ( panel != NULL ) xuiWidgetDestroy(panel);
		return ret;
	}
	(void)xuiWidgetSetLayoutType(panel, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(panel, (xui_thickness_t){4.0f, 4.0f, 4.0f, 4.0f});
	(void)xuiWidgetSetGap(panel, 4.0f);
	mapedit_widget_fill(panel);
	(void)xuiWidgetSetLayoutType(row, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(row, 6.0f);
	mapedit_widget_fixed_height(row, 38.0f);
	(void)xuiWidgetSetLayoutType(commandRow, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(commandRow, 6.0f);
	mapedit_widget_fixed_height(commandRow, 38.0f);
	mapedit_widget_fill(spacer);
	mapedit_widget_fill(pApp->pMapScrollFrame);
	(void)xuiScrollFrameSetChange(pApp->pMapScrollFrame, mapedit_map_scroll_changed, pApp);
	viewport = xuiScrollFrameGetViewportWidget(pApp->pMapScrollFrame);
	if ( viewport == NULL ) {
		xuiWidgetDestroy(panel);
		return XUI_ERROR;
	}
	(void)xuiWidgetSetRect(pApp->pMapCanvas, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f});
	ret = xuiWidgetAddChild(row, pApp->pMapToolbar);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(commandRow, pApp->pLayerCombo);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(commandRow, pApp->pMapCommandToolbar);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(commandRow, spacer);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(viewport, pApp->pMapCanvas);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, row);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, commandRow);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(panel, pApp->pMapScrollFrame);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(panel);
		return ret;
	}
	mapedit_map_scroll_sync_content(pApp);
	*ppPanel = panel;
	return XUI_OK;
}

static int mapedit_create_map_workspace(mapedit_app_t* pApp)
{
	xui_dock_panel_desc_t d;
	xui_widget placeholderObj;
	xui_widget placeholderEvt;
	xui_widget props;
	xui_widget tagsPanel;
	xui_widget mapPanel;
	xui_widget mapEditPanel;
	int paneDoc;
	int paneLeft;
	int win;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	ret = xuiDockPanelCreate(pApp->pContext, &pApp->pMapDock, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fill(pApp->pMapDock);
	(void)xuiWidgetSetDock(pApp->pMapDock, XUI_DOCK_FILL);
	mapedit_configure_dock_metrics(pApp->pMapDock);
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pMapDock);
	if ( mapedit_create_list(pApp, &pApp->pMapListView, &pApp->tMapFiles, mapedit_map_selected) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_list_manager_panel(pApp, pApp->pMapListView, &mapPanel, &pApp->pMapAddButton, "添加地图") != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pTileSelectCanvas, mapedit_tile_select_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pMapCanvas, mapedit_map_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pMapPassageCanvas, mapedit_map_passage_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_render_widget(pApp, &pApp->pMapTagsCanvas, mapedit_map_render, pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pTileSelectScrollFrame, pApp->pTileSelectCanvas, mapedit_tile_select_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pMapPassageScrollFrame, pApp->pMapPassageCanvas, mapedit_map_passage_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_canvas_scroll_frame(pApp, &pApp->pMapTagsScrollFrame, pApp->pMapTagsCanvas, mapedit_map_tags_scroll_changed) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_map_edit_panel(pApp, &mapEditPanel) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_tags_panel(pApp, 1, &tagsPanel) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_placeholder(pApp, &placeholderObj, "对象编辑\n窗口ID: map.object_edit") != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_placeholder(pApp, &placeholderEvt, "事件编辑\n窗口ID: map.event_edit") != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_property_grid(pApp, &props, 1) != XUI_OK ) return XUI_ERROR;
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_DOWN, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_MOVE, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_LEAVE, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_UP, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapCanvas, XUI_EVENT_POINTER_CAPTURE_LOST, mapedit_map_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMapCanvas, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_DOWN, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_MOVE, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_LEAVE, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_UP, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pTileSelectCanvas, XUI_EVENT_POINTER_CAPTURE_LOST, mapedit_tile_select_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pTileSelectCanvas, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	(void)xuiWidgetSetEventHandler(pApp->pMapPassageCanvas, XUI_EVENT_POINTER_DOWN, mapedit_map_passage_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapPassageCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_map_passage_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMapPassageCanvas, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiWidgetSetEventHandler(pApp->pMapTagsCanvas, XUI_EVENT_POINTER_DOWN, mapedit_map_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapTagsCanvas, XUI_EVENT_POINTER_MOVE, mapedit_map_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapTagsCanvas, XUI_EVENT_POINTER_LEAVE, mapedit_map_tags_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pMapTagsCanvas, XUI_EVENT_POINTER_WHEEL, mapedit_map_tags_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pMapTagsCanvas, XUI_EVENT_MASK_POINTER, 1);
	ret = mapedit_add_dock_window(pApp->pMapDock, "map.edit", "地图编辑", mapEditPanel, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pMapDock, "通行调整", pApp->pMapPassageScrollFrame, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pMapDock, win, (void*)"map.passage_adjust");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pMapDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pMapDock, "对象编辑", placeholderObj, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pMapDock, win, (void*)"map.object_edit");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pMapDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pMapDock, "事件编辑", placeholderEvt, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pMapDock, win, (void*)"map.event_edit");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pMapDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pMapDock, "地图打标", tagsPanel, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pMapDock, win, (void*)"map.tags");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pMapDock, win, paneDoc);
	if ( ret == XUI_OK ) ret = mapedit_add_dock_window(pApp->pMapDock, "map.list", "地图管理", mapPanel, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.24f, &paneLeft);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pMapDock, "图块选择", pApp->pTileSelectScrollFrame, &win);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetWindowUserData(pApp->pMapDock, win, (void*)"map.tile_select");
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pMapDock, win, paneLeft);
	if ( ret == XUI_OK ) ret = mapedit_add_dock_window(pApp->pMapDock, "map.properties", "地图属性", props, &win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.22f, NULL);
	pApp->pMapListPanel = mapPanel;
	pApp->pMapPropertyGrid = props;
	mapedit_refresh_map_properties(pApp);
	return ret;
}

static int mapedit_create_menu(mapedit_app_t* pApp)
{
	xui_menubar_desc_t mb;
	xui_menu_desc_t md;
	xui_menubar_item_t items[3];
	xui_menu_item_t fileItems[6];
	xui_menu_item_t viewItems[3];
	xui_menu_item_t helpItems[1];
	int ret;
	memset(&mb, 0, sizeof(mb));
	mb.iSize = sizeof(mb);
	mb.pFont = pApp->pFont;
	ret = xuiMenuBarCreate(pApp->pContext, &pApp->pMenuBar, &mb);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_height(pApp->pMenuBar, MAPEDIT_MENU_H);
	(void)xuiWidgetSetDock(pApp->pMenuBar, XUI_DOCK_TOP);
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pMenuBar);
	memset(&md, 0, sizeof(md));
	md.iSize = sizeof(md);
	md.pOwner = pApp->pMenuBar;
	md.pFont = pApp->pFont;
	ret = xuiMenuCreate(pApp->pContext, &pApp->pFileMenu, &md);
	if ( ret == XUI_OK ) ret = xuiMenuCreate(pApp->pContext, &pApp->pViewMenu, &md);
	if ( ret == XUI_OK ) ret = xuiMenuCreate(pApp->pContext, &pApp->pHelpMenu, &md);
	if ( ret != XUI_OK ) return ret;
	memset(fileItems, 0, sizeof(fileItems));
	fileItems[0] = (xui_menu_item_t){"打开当前类型文档", "Ctrl+O", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_FILE_OPEN, 0, NULL, NULL};
	fileItems[1] = (xui_menu_item_t){"保存当前文档", "Ctrl+S", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_FILE_SAVE, 0, NULL, NULL};
	fileItems[2] = (xui_menu_item_t){"当前文档另存为", "Ctrl+Shift+S", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_FILE_SAVE_AS, 0, NULL, NULL};
	fileItems[3] = (xui_menu_item_t){"恢复自动保存", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_FILE_RECOVER, 0, NULL, NULL};
	fileItems[4] = (xui_menu_item_t){NULL, NULL, XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL};
	fileItems[5] = (xui_menu_item_t){"退出", "Alt+F4", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_FILE_EXIT, 0, NULL, NULL};
	viewItems[0] = (xui_menu_item_t){"重置布局", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_VIEW_RESET_LAYOUT, 0, NULL, NULL};
	helpItems[0] = (xui_menu_item_t){"关于", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_HELP_ABOUT, 0, NULL, NULL};
	(void)xuiMenuSetItems(pApp->pFileMenu, fileItems, 6);
	viewItems[1] = (xui_menu_item_t){"原始大小 (100%)", "Ctrl+0", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_VIEW_ACTUAL_SIZE, 0, NULL, NULL};
	viewItems[2] = (xui_menu_item_t){"适应视图", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, CMD_VIEW_FIT, 0, NULL, NULL};
	(void)xuiMenuSetItems(pApp->pViewMenu, viewItems, 3);
	(void)xuiMenuSetItems(pApp->pHelpMenu, helpItems, 1);
	(void)xuiMenuSetSelect(pApp->pFileMenu, mapedit_menu_select, pApp);
	(void)xuiMenuSetSelect(pApp->pViewMenu, mapedit_menu_select, pApp);
	(void)xuiMenuSetSelect(pApp->pHelpMenu, mapedit_menu_select, pApp);
	items[0] = (xui_menubar_item_t){"文件", XUI_MENUBAR_ITEM_ENABLED, 0, 'F', pApp->pFileMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	items[1] = (xui_menubar_item_t){"视图", XUI_MENUBAR_ITEM_ENABLED, 0, 'V', pApp->pViewMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	items[2] = (xui_menubar_item_t){"帮助", XUI_MENUBAR_ITEM_ENABLED, 0, 'H', pApp->pHelpMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	ret = xuiMenuBarSetItems(pApp->pMenuBar, items, 3);
	if ( ret == XUI_OK ) ret = xuiMenuBarSetSelect(pApp->pMenuBar, mapedit_menu_select, pApp);
	return ret;
}

static int mapedit_create_switch(mapedit_app_t* pApp)
{
	xui_button_desc_t d;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	d.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE;
	d.iTextColor = XUI_COLOR_RGBA(30, 74, 112, 255);
	d.iNormalColor = XUI_COLOR_RGBA(235, 243, 252, 255);
	d.iHoverColor = XUI_COLOR_RGBA(221, 236, 250, 255);
	d.iActiveColor = XUI_COLOR_RGBA(205, 226, 246, 255);
	d.iCheckedColor = XUI_COLOR_RGBA(210, 232, 252, 255);
	d.iBorderColor = XUI_COLOR_RGBA(136, 166, 196, 255);
	d.fBorderWidth = 1.0f;
	d.sText = "图块编辑器";
	if ( xuiButtonCreate(pApp->pContext, &pApp->pTileButton, &d) != XUI_OK ) return XUI_ERROR;
	d.sText = "地图编辑器";
	if ( xuiButtonCreate(pApp->pContext, &pApp->pMapButton, &d) != XUI_OK ) return XUI_ERROR;
	(void)xuiButtonSetSelectable(pApp->pTileButton, 1);
	(void)xuiButtonSetSelectable(pApp->pMapButton, 1);
	(void)xuiButtonSetClick(pApp->pTileButton, mapedit_workspace_click, pApp);
	(void)xuiButtonSetClick(pApp->pMapButton, mapedit_workspace_click, pApp);
	if (xuiWidgetCreate(pApp->pContext, &pApp->pSwitchBar) != XUI_OK) return XUI_ERROR;
	(void)xuiWidgetSetLayoutType(pApp->pSwitchBar, XUI_LAYOUT_ROW);
	(void)xuiWidgetSetDock(pApp->pSwitchBar, XUI_DOCK_TOP);
	(void)xuiWidgetSetPadding(pApp->pSwitchBar, (xui_thickness_t){8, 6, 8, 6});
	(void)xuiWidgetSetGap(pApp->pSwitchBar, 8.0f);
	mapedit_widget_fixed_height(pApp->pSwitchBar, MAPEDIT_SWITCH_H);
	mapedit_widget_fixed_size(pApp->pTileButton, 120.0f, MAPEDIT_BUTTON_HEIGHT);
	mapedit_widget_fixed_size(pApp->pMapButton, 120.0f, MAPEDIT_BUTTON_HEIGHT);
	(void)xuiWidgetAddChild(pApp->pSwitchBar, pApp->pTileButton);
	(void)xuiWidgetAddChild(pApp->pSwitchBar, pApp->pMapButton);
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pSwitchBar);
	return XUI_OK;
}

static int mapedit_create_status(mapedit_app_t* pApp)
{
	xui_statusbar_desc_t d;
	int ret;
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	ret = xuiStatusBarCreate(pApp->pContext, &pApp->pStatus, &d);
	if ( ret != XUI_OK ) return ret;
	mapedit_widget_fixed_height(pApp->pStatus, MAPEDIT_STATUS_H);
	(void)xuiWidgetSetDock(pApp->pStatus, XUI_DOCK_BOTTOM);
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pStatus);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "坐标: 0,0", 420.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "地图: 100 x 100", 170.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "图块: tileset 0 / tile 0 / layer 0", 310.0f, 0, 0);
	(void)xuiStatusBarAddFlexibleSpacer(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, 1.0f);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_RIGHT, "位置: passable / region 0 / event none", 330.0f, 0, 0);
	mapedit_status(pApp, "就绪");
	return XUI_OK;
}

static xui_toolbar_item_t mapedit_toolbar_item(const char* sText, const char* sTooltip, int iType, uint32_t iState, int iValue, int iGroup)
{
	xui_toolbar_item_t item;
	memset(&item, 0, sizeof(item));
	item.sText = sText;
	item.sTooltip = sTooltip;
	item.iType = iType;
	item.iState = iState;
	item.iValue = iValue;
	item.iGroup = iGroup;
	return item;
}

static void mapedit_refresh_toolbar_state(mapedit_app_t* pApp)
{
	int i;
	if ( pApp == NULL ) return;
	if ( pApp->pMapToolbar != NULL ) {
		for ( i = 0; i < MAPEDIT_TOOL_COUNT; i++ ) {
			(void)xuiToolbarSetItemChecked(pApp->pMapToolbar, i, i == pApp->iActiveTool);
		}
	}
	if ( pApp->pMapCommandToolbar != NULL ) {
		(void)xuiToolbarSetItemChecked(pApp->pMapCommandToolbar, 0, pApp->bPreview);
		(void)xuiToolbarSetItemChecked(pApp->pMapCommandToolbar, 1, pApp->bGrid);
		(void)xuiToolbarSetItemChecked(pApp->pMapCommandToolbar, 2, 0);
		(void)xuiToolbarSetItemChecked(pApp->pMapCommandToolbar, 3, 0);
		(void)xuiToolbarSetItemEnabled(pApp->pMapCommandToolbar, 2, (pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_TILESET ? pApp->tTilesetHistory.iUndoCount : pApp->tMapHistory.iUndoCount) > 0);
		(void)xuiToolbarSetItemEnabled(pApp->pMapCommandToolbar, 3, (pApp->iActiveWorkspace == MAPEDIT_WORKSPACE_TILESET ? pApp->tTilesetHistory.iRedoCount : pApp->tMapHistory.iRedoCount) > 0);
	}
}

static int mapedit_create_toolbar(mapedit_app_t* pApp)
{
	xui_toolbar_desc_t d;
	xui_combobox_desc_t cd;
	xui_toolbar_item_t toolItems[MAPEDIT_TOOL_COUNT];
	xui_toolbar_item_t commandItems[4];
	memset(&d, 0, sizeof(d));
	d.iSize = sizeof(d);
	d.pFont = pApp->pFont;
	if ( xuiToolbarCreate(pApp->pContext, &pApp->pMapToolbar, &d) != XUI_OK ) return XUI_ERROR;
	if ( xuiToolbarCreate(pApp->pContext, &pApp->pMapCommandToolbar, &d) != XUI_OK ) return XUI_ERROR;
	memset(toolItems, 0, sizeof(toolItems));
	toolItems[0] = mapedit_toolbar_item("画笔", "画笔", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED | XUI_TOOLBAR_ITEM_CHECKED, CMD_TOOL_BRUSH, 1);
	toolItems[1] = mapedit_toolbar_item("橡皮", "橡皮", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_TOOL_ERASER, 1);
	toolItems[2] = mapedit_toolbar_item("线条", "线条", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_TOOL_LINE, 1);
	toolItems[3] = mapedit_toolbar_item("矩形", "矩形填充", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_TOOL_RECT, 1);
	toolItems[4] = mapedit_toolbar_item("圆形", "圆形填充", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_TOOL_CIRCLE, 1);
	toolItems[5] = mapedit_toolbar_item("填充", "填充模式", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_TOOL_BUCKET, 1);
	memset(commandItems, 0, sizeof(commandItems));
	commandItems[0] = mapedit_toolbar_item("预览", "预览", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, CMD_PREVIEW, 2);
	commandItems[1] = mapedit_toolbar_item("网格", "网格", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED | XUI_TOOLBAR_ITEM_CHECKED, CMD_GRID, 2);
	commandItems[2] = mapedit_toolbar_item("撤销", "撤销", XUI_TOOLBAR_ITEM_BUTTON, 0, CMD_UNDO, 3);
	commandItems[3] = mapedit_toolbar_item("重做", "重做", XUI_TOOLBAR_ITEM_BUTTON, 0, CMD_REDO, 3);
	(void)xuiToolbarSetItems(pApp->pMapToolbar, toolItems, MAPEDIT_TOOL_COUNT);
	(void)xuiToolbarSetItemSize(pApp->pMapToolbar, 58.0f, 34.0f, 4.0f);
	(void)xuiToolbarSetSelect(pApp->pMapToolbar, mapedit_toolbar_select, pApp);
	mapedit_widget_fixed_size(pApp->pMapToolbar, 374.0f, 38.0f);
	(void)xuiToolbarSetItems(pApp->pMapCommandToolbar, commandItems, 4);
	(void)xuiToolbarSetItemSize(pApp->pMapCommandToolbar, 58.0f, 34.0f, 4.0f);
	(void)xuiToolbarSetSelect(pApp->pMapCommandToolbar, mapedit_toolbar_select, pApp);
	mapedit_widget_fixed_size(pApp->pMapCommandToolbar, 252.0f, 38.0f);
	mapedit_refresh_toolbar_state(pApp);
	memset(&cd, 0, sizeof(cd));
	cd.iSize = sizeof(cd);
	cd.arrItems = pApp->arrLayerNames;
	cd.iItemCount = pApp->iSetupLayerCount > 0 ? pApp->iSetupLayerCount : 1;
	cd.iSelected = 0;
	cd.pFont = pApp->pFont;
	cd.fItemHeight = MAPEDIT_ROW_HEIGHT;
	cd.fPopupMaxHeight = 220.0f;
	cd.iTextColor = XUI_COLOR_RGBA(31, 75, 112, 255);
	cd.iDisabledTextColor = XUI_COLOR_RGBA(132, 142, 152, 255);
	cd.iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	cd.iHoverBackgroundColor = XUI_COLOR_RGBA(226, 242, 252, 255);
	cd.iOpenBackgroundColor = XUI_COLOR_RGBA(204, 232, 250, 255);
	cd.iDisabledBackgroundColor = XUI_COLOR_RGBA(230, 236, 240, 255);
	cd.iBorderColor = XUI_COLOR_RGBA(124, 181, 219, 255);
	cd.iHoverBorderColor = XUI_COLOR_RGBA(124, 181, 219, 255);
	cd.iFocusBorderColor = XUI_COLOR_RGBA(124, 181, 219, 255);
	cd.iArrowColor = XUI_COLOR_RGBA(31, 75, 112, 255);
	cd.iDisabledArrowColor = XUI_COLOR_RGBA(132, 142, 152, 255);
	cd.iButtonColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	cd.iButtonHoverColor = XUI_COLOR_RGBA(226, 242, 252, 255);
	cd.iButtonOpenColor = XUI_COLOR_RGBA(204, 232, 250, 255);
	cd.iPopupPanelColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	cd.iPopupBorderColor = XUI_COLOR_RGBA(124, 181, 219, 255);
	cd.iPopupHoverColor = XUI_COLOR_RGBA(226, 242, 252, 255);
	cd.iPopupTextColor = XUI_COLOR_RGBA(31, 75, 112, 255);
	cd.iPopupHoverTextColor = XUI_COLOR_RGBA(31, 75, 112, 255);
	cd.iPopupDisabledTextColor = XUI_COLOR_RGBA(132, 142, 152, 255);
	cd.iPopupSeparatorColor = XUI_COLOR_RGBA(230, 236, 240, 255);
	if ( xuiComboBoxCreate(pApp->pContext, &pApp->pLayerCombo, &cd) != XUI_OK ) return XUI_ERROR;
	(void)xuiComboBoxSetSelect(pApp->pLayerCombo, mapedit_layer_select, pApp);
	mapedit_widget_fixed_size(pApp->pLayerCombo, 136.0f, 34.0f);
	return XUI_OK;
}

static int mapedit_root_render(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_t r;
	(void)iStateId;
	if ( pApp == NULL || pDraw == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	(void)pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(232, 241, 250, 255));
	return XUI_OK;
}

static int mapedit_register_hotkeys(mapedit_app_t* pApp)
{
	int ret;
	if ( pApp == NULL || pApp->pContext == NULL || pApp->pRoot == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, 'O', XUI_MOD_CTRL, CMD_FILE_OPEN, "file.open", pApp);
	if ( ret == XUI_OK ) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, 'S', XUI_MOD_CTRL, CMD_FILE_SAVE, "file.save", pApp);
	if ( ret == XUI_OK ) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, 'S', XUI_MOD_CTRL | XUI_MOD_SHIFT, CMD_FILE_SAVE_AS, "file.save_as", pApp);
	if ( ret == XUI_OK ) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, 'Z', XUI_MOD_CTRL, CMD_UNDO, "edit.undo", pApp);
	if ( ret == XUI_OK ) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, 'Y', XUI_MOD_CTRL, CMD_REDO, "edit.redo", pApp);
	if ( ret == XUI_OK ) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, XUI_KEY_F4, XUI_MOD_ALT, CMD_FILE_EXIT, "file.exit", pApp);
	if (ret == XUI_OK) ret = xuiHotKeyRegisterCommand(pApp->pContext, pApp->pRoot, '0', XUI_MOD_CTRL, CMD_VIEW_ACTUAL_SIZE, "view.actual", pApp);
	return ret;
}

static int mapedit_create_ui(mapedit_app_t* pApp)
{
	xui_cache_policy_t policy;
	int ret;
	pApp->tProxy = xuiProxyXge();
	ret = xuiCreate(&pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputViewport(pApp->pContext, MAPEDIT_W, MAPEDIT_H);
	if ( ret != XUI_OK ) return ret;
	ret = mapedit_resize_target(pApp, MAPEDIT_W, MAPEDIT_H);
	if ( ret != XUI_OK ) return ret;
	ret = mapeditThemeInit(pApp->pContext, &pApp->tProxy, &pApp->pFont, pApp->sAppDir);
	if (ret != XUI_OK) return ret;
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( ret != XUI_OK ) return ret;
	memset(&policy, 0, sizeof(policy));
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	policy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)MAPEDIT_W, (float)MAPEDIT_H});
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_DOCK);
	(void)xuiWidgetSetCachePolicy(pApp->pRoot, &policy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, mapedit_root_render, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_DOWN, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_UP, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_MOVE, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_CLICK, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_WHEEL, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_POINTER_DOUBLE_CLICK, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_CONTEXT_MENU, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_KEY_DOWN, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_KEY_UP, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_TEXT, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_HOTKEY, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pRoot, XUI_EVENT_COMMAND, mapedit_root_event, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pRoot, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_KEYBOARD, 1);
	ret = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if ( ret != XUI_OK ) return ret;
	ret = mapedit_register_hotkeys(pApp);
	if ( ret != XUI_OK ) return ret;
	if ( mapedit_create_menu(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_switch(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_toolbar(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_status(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_tileset_workspace(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_create_map_workspace(pApp) != XUI_OK ) return XUI_ERROR;
	if ( mapedit_sync_viewport(pApp) != XUI_OK ) return XUI_ERROR;
	mapedit_register_view_gestures(pApp);
	mapedit_load_layouts(pApp);
	mapedit_select_workspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	pApp->bCreateOK = 1;
	return XUI_OK;
}

static void mapedit_scan_data(mapedit_app_t* pApp)
{
	char path[MAPEDIT_PATH_MAX];
	mapedit_load_setup(pApp);
	mapedit_app_path(pApp, path, sizeof(path), "assets\\maps");
	mapedit_scan_files(path, ".xson", &pApp->tMapFiles);
	mapedit_refresh_map_list_texts(pApp, &pApp->tMapFiles);
	mapedit_app_path(pApp, path, sizeof(path), "assets\\图块集");
	mapedit_scan_files(path, ".xson", &pApp->tTilesetFiles);
	mapedit_refresh_tileset_list_texts(pApp, &pApp->tTilesetFiles);
	mapedit_app_path(pApp, path, sizeof(path), "assets\\tilesheets");
	mapedit_scan_files(path, ".png", &pApp->tMaterialFiles);
	pApp->iMaterialCategory = 0;
	pApp->iMaterialContextIndex = -1;
	pApp->iMaterialRenameIndex = -1;
	pApp->iMaterialTooltipIndex = -1;
	mapedit_material_ensure_all_maps(pApp);
	(void)mapedit_material_scan_category(pApp, 0, 1);
	pApp->bDataOK = (pApp->tMapFiles.iCount > 0) && (pApp->tTilesetFiles.iCount > 0);
}

static void mapedit_load_initial_data(mapedit_app_t* pApp)
{
	pApp->iMapHoverX = -1;
	pApp->iMapHoverY = -1;
	pApp->iMapTagsHoverX = -1;
	pApp->iMapTagsHoverY = -1;
	pApp->iTileSelectHoverCol = -1;
	pApp->iTileSelectHoverRow = -1;
	pApp->iTilesetArrangeHoverTile = -1;
	pApp->iTilesetTagsHoverTile = -1;
	if ( pApp->tMapFiles.iCount > 0 ) mapedit_map_selected(NULL, 0, pApp);
	else mapedit_map_clear(&pApp->tMap);
	if ( pApp->tTileset.bLoaded == 0 && pApp->tTilesetFiles.iCount > 0 ) mapedit_tileset_selected(NULL, 0, pApp);
	pApp->iSelectedTile = -1;
	pApp->iBrushW = 1;
	pApp->iBrushH = 1;
	pApp->iTilesetArrangeSelectedTile = -1;
	pApp->iTilesetPassageSelectedTile = -1;
	pApp->iTilesetActorSelectedTile = -1;
	pApp->iTilesetTagsSelectedTile = -1;
	pApp->iMapPassageSelectedCell = -1;
	pApp->iMapTagsSelectedCell = -1;
	pApp->iTilesetPropertyMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
	pApp->iTilesetPropertyTile = -1;
	pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_MAP;
	pApp->iMapPropertyCell = -1;
	pApp->iDragStartX = -1;
	pApp->iDragStartY = -1;
	pApp->iDragCurrentX = -1;
	pApp->iDragCurrentY = -1;
	pApp->iActiveTool = MAPEDIT_TOOL_BRUSH;
	pApp->iActiveLayer = 0;
	if ( pApp->pLayerCombo != NULL ) (void)xuiComboBoxSetSelected(pApp->pLayerCombo, 0);
	pApp->bGrid = 1;
	mapedit_refresh_toolbar_state(pApp);
	mapedit_update_status_details(pApp);
	mapedit_tags_update_channel_combo(pApp, 0);
	mapedit_tags_update_channel_combo(pApp, 1);
	mapedit_refresh_tileset_properties(pApp);
	mapedit_refresh_map_properties(pApp);
	mapedit_map_workspace_scroll_sync_content(pApp);
	mapedit_tileset_workspace_scroll_sync_content(pApp);
}

static void mapedit_make_large_map(mapedit_app_t* pApp, int iWidth, int iHeight)
{
	int x;
	int y;
	if ( pApp == NULL || iWidth <= 0 || iHeight <= 0 ) return;
	if ( xgeMapValidateSize(iWidth, iHeight, XGE_MAP_LEGACY_DEFAULT_LAYERS, NULL, NULL) != XGE_MAP_OK ) {
		iWidth = 1024;
		iHeight = 1024;
	}
	mapedit_map_clear(&pApp->tMap);
	mapedit_clear_history(pApp);
	mapedit_copy_text(pApp->tMap.sName, MAPEDIT_NAME_MAX, "大地图性能测试");
	mapedit_copy_text(pApp->tMap.sTileset, MAPEDIT_FILE_MAX, pApp->tTileset.sFile[0] ? pApp->tTileset.sFile : "default.xson");
	pApp->tMap.iWidth = iWidth;
	pApp->tMap.iHeight = iHeight;
	pApp->tMap.iLayers = XGE_MAP_LEGACY_DEFAULT_LAYERS;
	pApp->tMap.iState = mapedit_setup_clamp_state(pApp, pApp->iSetupStateMin);
	pApp->tMap.pPassageRaw = xrtValueArray();
	pApp->tMap.pCellDataRaw = xrtValueObject();
	if ( mapedit_map_alloc(&pApp->tMap) != XUI_OK ) return;
	for ( y = 0; y < iHeight; y += 8 ) {
		for ( x = 0; x < iWidth; x += 8 ) {
			int idx = y * iWidth + x;
			if ( idx >= 0 && idx < pApp->tMap.iTileCount ) pApp->tMap.pTiles[idx] = 1 + ((x / 8 + y / 8) % 4);
		}
	}
	pApp->tMap.bDirty = 0;
	pApp->bMapOK = 1;
	pApp->iMapHoverX = -1;
	pApp->iMapHoverY = -1;
	pApp->iMapTagsHoverX = -1;
	pApp->iMapTagsHoverY = -1;
	pApp->iMapPassageSelectedCell = -1;
	pApp->iMapTagsSelectedCell = -1;
	pApp->iMapPropertyMode = MAPEDIT_MAP_PROPERTY_MODE_MAP;
	pApp->iMapPropertyCell = -1;
	mapedit_map_scroll_reset(pApp);
	mapedit_map_passage_scroll_reset(pApp);
	mapedit_map_tags_scroll_reset(pApp);
	mapedit_update_status_details(pApp);
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	mapedit_map_workspace_scroll_sync_content(pApp);
	mapedit_refresh_map_properties(pApp);
}

static int mapedit_run_custom_default_smoke(mapedit_app_t* pApp)
{
	mapedit_custom_channel_def_t def;
	char got[MAPEDIT_CUSTOM_VALUE_MAX];
	xvalue* pSavedTileCustom;
	xvalue* pSavedCellData;
	int cell;
	int oldTile;
	int bRestoreTile;
	int ok;
	if ( pApp == NULL ) return 0;
	pSavedTileCustom = (pApp->tTileset.pTileCustomRaw != NULL) ? xrtValueClone(pApp->tTileset.pTileCustomRaw) : NULL;
	pSavedCellData = (pApp->tMap.pCellDataRaw != NULL) ? xrtValueClone(pApp->tMap.pCellDataRaw) : NULL;
	if ( (pApp->tTileset.pTileCustomRaw != NULL && pSavedTileCustom == NULL) ||
	     (pApp->tMap.pCellDataRaw != NULL && pSavedCellData == NULL) ) {
		if ( pSavedTileCustom != NULL ) xrtValueRelease(pSavedTileCustom);
		if ( pSavedCellData != NULL ) xrtValueRelease(pSavedCellData);
		return 0;
	}
	oldTile = 0;
	bRestoreTile = 0;
	ok = 0;
	if ( pApp->tMap.pTiles == NULL ) {
		pApp->tMap.iWidth = 2;
		pApp->tMap.iHeight = 2;
		pApp->tMap.iLayers = XGE_MAP_LEGACY_DEFAULT_LAYERS;
		if ( mapedit_map_alloc(&pApp->tMap) != XUI_OK ) goto cleanup;
	}
	if ( pApp->tMap.iWidth <= 0 || pApp->tMap.iHeight <= 0 ) goto cleanup;
	cell = mapedit_map_cell_id(&pApp->tMap, 0, 0);
	if ( cell < 0 ) goto cleanup;
	if ( pApp->tMap.pTiles != NULL && pApp->tMap.iTileCount > 0 ) {
		oldTile = pApp->tMap.pTiles[0];
		bRestoreTile = 1;
		pApp->tMap.pTiles[0] = 1;
	}
	memset(&def, 0, sizeof(def));
	mapedit_copy_text(def.sId, sizeof(def.sId), "__smoke_custom_default");
	mapedit_copy_text(def.sScope, sizeof(def.sScope), "tile");
	mapedit_copy_text(def.sDataType, sizeof(def.sDataType), "int");
	mapedit_copy_text(def.sDefaultValue, sizeof(def.sDefaultValue), "7");
	if ( pApp->tTileset.pTileCustomRaw == NULL || xuiXrtValueType(pApp->tTileset.pTileCustomRaw) != XVALUE_OBJECT ) {
		if ( pApp->tTileset.pTileCustomRaw != NULL ) xrtValueRelease(pApp->tTileset.pTileCustomRaw);
		pApp->tTileset.pTileCustomRaw = xrtValueObject();
		if ( pApp->tTileset.pTileCustomRaw == NULL ) goto cleanup;
	}
	if ( mapedit_custom_set_value(&pApp->tTileset.pTileCustomRaw, def.sId, SET_KEY_TILE, 1, "42") != XUI_OK ) goto cleanup;
	if ( mapedit_map_set_cell_custom_value(pApp, &def, 0, 0, "99") != XUI_OK ) goto cleanup;
	if ( !mapedit_custom_get_value(pApp->tMap.pCellDataRaw, def.sId, MAP_KEY_CELL, cell, got, sizeof(got)) || strcmp(got, "99") != 0 ) goto cleanup;
	if ( mapedit_map_set_cell_custom_value(pApp, &def, 0, 0, "42") != XUI_OK ) goto cleanup;
	if ( mapedit_custom_get_value(pApp->tMap.pCellDataRaw, def.sId, MAP_KEY_CELL, cell, got, sizeof(got)) ) goto cleanup;
	(void)mapedit_custom_remove_value(pApp->tTileset.pTileCustomRaw, def.sId, SET_KEY_TILE, 1);
	ok = 1;

cleanup:
	if ( bRestoreTile && pApp->tMap.pTiles != NULL && pApp->tMap.iTileCount > 0 ) pApp->tMap.pTiles[0] = oldTile;
	if ( pApp->tMap.pCellDataRaw != NULL ) xrtValueRelease(pApp->tMap.pCellDataRaw);
	pApp->tMap.pCellDataRaw = pSavedCellData;
	if ( pApp->tTileset.pTileCustomRaw != NULL ) xrtValueRelease(pApp->tTileset.pTileCustomRaw);
	pApp->tTileset.pTileCustomRaw = pSavedTileCustom;
	return ok;
}

static int mapedit_run_edit_history_smoke(mapedit_app_t* pApp)
{
	double savedDirtySince;
	double savedRecoveryWrite;
	int oldTile;
	int newTile;
	int savedDirty;
	int savedGestureDirty;
	int savedBatchEdit;
	int ok;
	int workspace = pApp->iActiveWorkspace;
	if ( pApp == NULL || pApp->tMap.pTiles == NULL || pApp->tMap.iTileCount <= 0 || pApp->tMap.iLayers <= 0 ) return 0;
	pApp->iActiveWorkspace = MAPEDIT_WORKSPACE_MAP;
	oldTile = pApp->tMap.pTiles[0];
	newTile = oldTile == 0 ? 1 : 0;
	savedDirty = pApp->tMap.bDirty;
	savedGestureDirty = pApp->bGestureDirty;
	savedBatchEdit = pApp->bBatchEdit;
	savedDirtySince = pApp->fMapDirtySince;
	savedRecoveryWrite = pApp->fLastRecoveryWrite;
	mapedit_clear_history(pApp);
	mapedit_begin_history(pApp);
	pApp->bBatchEdit = 1;
	pApp->bGestureDirty = 0;
	mapedit_set_tile(pApp, 0, 0, 0, newTile);
	ok = pApp->tMap.pTiles[0] == newTile && mapedit_commit_history(pApp) && pApp->tMapHistory.iUndoCount == 1;
	if ( ok ) {
		mapedit_undo(pApp);
		ok = pApp->tMap.pTiles[0] == oldTile && pApp->tMapHistory.iRedoCount == 1;
	}
	if ( ok ) {
		mapedit_redo(pApp);
		ok = pApp->tMap.pTiles[0] == newTile && pApp->tMapHistory.iUndoCount == 1;
	}
	pApp->tMap.pTiles[0] = oldTile;
	mapedit_clear_history(pApp);
	pApp->tMap.bDirty = savedDirty;
	pApp->bGestureDirty = savedGestureDirty;
	pApp->bBatchEdit = savedBatchEdit;
	pApp->fMapDirtySince = savedDirtySince;
	pApp->fLastRecoveryWrite = savedRecoveryWrite;
	pApp->bEditOK = ok;
	pApp->iActiveWorkspace = workspace;
	mapedit_update_window_title(pApp);
	return ok;
}

static void mapedit_destroy(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) return;
	xgeSetQuitRequestCallback(NULL, NULL);
	if (pApp->pExternalBox) { xuiMsgBoxDestroy(pApp->pExternalBox); pApp->pExternalBox = NULL; }
	if (pApp->pConfirmBox) { xuiMsgBoxDestroy(pApp->pConfirmBox); pApp->pConfirmBox = NULL; }
	mapedit_clear_history(pApp);
	mapeditHistoryClear(&pApp->tTilesetHistory);
	mapedit_material_preview_clear(pApp);
	mapedit_material_tooltip_clear(pApp);
	mapedit_material_view_clear(pApp);
	mapedit_material_edit_clear_source(pApp);
	mapedit_material_edit_clear_output(pApp);
	if ( pApp->pMaterialEditMsgTip != NULL ) {
		xuiMsgTipDestroy(pApp->pMaterialEditMsgTip);
		pApp->pMaterialEditMsgTip = NULL;
	}
	mapedit_tileset_clear(pApp);
	mapedit_map_clear(&pApp->tMap);
	if ( pApp->pFileDialog != NULL ) {
		xuiFileDialogDestroy(pApp->pFileDialog);
		pApp->pFileDialog = NULL;
	}
	if ( pApp->pContext != NULL ) {
		xuiDestroy(pApp->pContext);
		pApp->pContext = NULL;
	}
	mapedit_file_list_clear(&pApp->tMapFiles);
	mapedit_file_list_clear(&pApp->tTilesetFiles);
	mapedit_file_list_clear(&pApp->tMaterialFiles);
	mapedit_file_list_clear(&pApp->tMaterialCategoryFiles);
	mapedit_file_list_clear(&pApp->tMapTilesetOptions);
	mapedit_file_list_clear(&pApp->tTilesetSpecialFileOptions);
	if ( pApp->pFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont);
		pApp->pFont = NULL;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
}

static void mapedit_update_summary_checks(mapedit_app_t* pApp)
{
	xui_dock_pane_info_t pane;
	pApp->bLayoutOK = (pApp->pTilesetDock != NULL) && (pApp->pMapDock != NULL) &&
	                  (xuiDockPanelGetPaneCount(pApp->pTilesetDock) >= 3) &&
	                  (xuiDockPanelGetPaneCount(pApp->pMapDock) >= 3) &&
	                  (xuiDockPanelGetPaneInfo(pApp->pMapDock, 0, &pane) == XUI_OK || xuiDockPanelGetPaneCount(pApp->pMapDock) > 0);
	pApp->bMapOK = pApp->bMapOK || (pApp->tMap.pTiles != NULL && pApp->tMap.iWidth > 0 && pApp->tMap.iHeight > 0);
	pApp->bTilesetOK = pApp->bTilesetOK || pApp->tTileset.bLoaded;
}

static void mapedit_update_preview(mapedit_app_t* pApp, float fDelta)
{
	int step;
	if ( pApp == NULL || !pApp->bPreview || pApp->iActiveWorkspace != MAPEDIT_WORKSPACE_MAP ) return;
	if ( fDelta < 0.0f ) fDelta = 0.0f;
	pApp->fPreviewAnimTime += fDelta;
	if ( pApp->fPreviewAnimTime < MAPEDIT_MAP_PREVIEW_FRAME_SECONDS ) {
		xgeRenderRequestAfter(MAPEDIT_MAP_PREVIEW_FRAME_SECONDS - pApp->fPreviewAnimTime);
		return;
	}
	step = (int)(pApp->fPreviewAnimTime / MAPEDIT_MAP_PREVIEW_FRAME_SECONDS);
	if ( step < 1 ) step = 1;
	pApp->fPreviewAnimTime -= (float)step * MAPEDIT_MAP_PREVIEW_FRAME_SECONDS;
	if ( pApp->fPreviewAnimTime < 0.0f ) pApp->fPreviewAnimTime = 0.0f;
	pApp->iPreviewAnimFrame = (pApp->iPreviewAnimFrame + step) & 0x3fffffff;
	if ( pApp->pMapCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pMapCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	xgeRenderRequestAfter(MAPEDIT_MAP_PREVIEW_FRAME_SECONDS - pApp->fPreviewAnimTime);
}

static void mapedit_smoke_activate_view(mapedit_app_t* pApp, const char* id)
{
	int window;
	for (window = 0; window < xuiDockPanelGetWindowCount(pApp->pMapDock); ++window) {
		const char* current = (const char*)xuiDockPanelGetWindowUserData(pApp->pMapDock, window);
		xui_dock_window_info_t info;
		memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
		if (current && !strcmp(id, current) && xuiDockPanelGetWindowInfo(pApp->pMapDock, window, &info) == XUI_OK)
			(void)xuiDockPanelSetPaneActiveWindow(pApp->pMapDock, info.iPane, window);
	}
}

static int mapedit_smoke_workbench(mapedit_app_t* pApp)
{
	char mapPath[MAPEDIT_PATH_MAX], setPath[MAPEDIT_PATH_MAX], recovery[MAPEDIT_PATH_MAX];
	char name[MAPEDIT_NAME_MAX], oldMapPath[MAPEDIT_PATH_MAX], oldSetPath[MAPEDIT_PATH_MAX];
	int oldDirty, oldTile, tx, ty, ok = 1, category, selected, oldWidth;
	xui_rect_t rect;
	xui_event_t event;
	if (pApp->iSmokeWorkbenchStage == 0) {
		mapedit_smoke_activate_view(pApp, "map.tags");
		/* Execute a real native WM_CLOSE while dirty; the next frame must survive. */
		mapedit_map_property_change(NULL, 0, "map.name", "native-close-test", 0, pApp);
		PostMessageW((HWND)xgePlatformNativeHandle(), WM_CLOSE, 0, 0);
		pApp->iSmokeWorkbenchStage = 1;
		xgeRenderRequest();
		return 1;
	}
	if (pApp->iSmokeWorkbenchStage != 1) {
		xui_rect_t canvas = xuiWidgetGetRect(pApp->pMapTagsCanvas);
		if (canvas.fW < 100 || canvas.fH < 100 || pApp->iLastVisibleCells < 100) {
			pApp->bSmokeWorkbenchOK = 0;
			fprintf(stderr, "workbench FAIL: canvas %dx%d, visible cells %d\n", canvas.fW, canvas.fH, pApp->iLastVisibleCells);
		}
		if (pApp->iSmokeWorkbenchStage == 2) {
			mapedit_smoke_activate_view(pApp, "map.edit");
			pApp->iSmokeWorkbenchStage = 3;
		}
		return pApp->bSmokeWorkbenchOK;
	}
#define WORKBENCH_CHECK(expr) do { if (!(expr)) { fprintf(stderr, "workbench FAIL line %d: %s\n", __LINE__, #expr); ok = 0; } } while (0)
	WORKBENCH_CHECK(pApp->pConfirmBox && xuiMsgBoxIsOpen(pApp->pConfirmBox) && pApp->iPendingOperation == 1);
	if (pApp->pConfirmBox) (void)xuiMsgBoxSetOpen(pApp->pConfirmBox, 0);
	mapedit_confirm_result(pApp->pConfirmBox, XUI_MSGBOX_RESULT_CANCEL, pApp);
	WORKBENCH_CHECK(pApp->tMap.bDirty && !pApp->iPendingOperation);
	mapedit_undo(pApp);
	mapedit_copy_text(name, sizeof(name), pApp->tMap.sName);
	mapedit_copy_text(oldMapPath, sizeof(oldMapPath), pApp->tMap.sPath);
	mapedit_copy_text(oldSetPath, sizeof(oldSetPath), pApp->tTileset.sPath);
	oldDirty = pApp->tMap.bDirty;
	mapedit_path_join(mapPath, sizeof(mapPath), pApp->sAppDir, "workbench-test-map.xson");
	mapedit_path_join(setPath, sizeof(setPath), pApp->sAppDir, "workbench-test-tileset.xson");
	selected = xuiPropertyGridFindProperty(pApp->pMapPropertyGrid, "map.width");
	category = xuiPropertyGridFindCategory(pApp->pMapPropertyGrid, "map");
	(void)xuiPropertyGridSetSelected(pApp->pMapPropertyGrid, selected);
	(void)xuiPropertyGridSetCategoryExpanded(pApp->pMapPropertyGrid, category, 0);
	mapedit_map_property_change(NULL, 0, "map.name", "history-test", 0, pApp);
	WORKBENCH_CHECK(xuiPropertyGridGetSelected(pApp->pMapPropertyGrid) == selected);
	WORKBENCH_CHECK(!xuiPropertyGridGetCategoryExpanded(pApp->pMapPropertyGrid, category));
	WORKBENCH_CHECK(mapedit_save_map_to(pApp, mapPath) == XUI_OK && !pApp->tMap.bDirty);
	mapedit_undo(pApp); WORKBENCH_CHECK(pApp->tMap.bDirty && strcmp(pApp->tMap.sName, name) == 0);
	mapedit_redo(pApp); WORKBENCH_CHECK(!pApp->tMap.bDirty && !strcmp(pApp->tMap.sPath, mapPath));
	{
		int undoCount = pApp->tMapHistory.iUndoCount;
		mapedit_map_property_change(NULL, 0, "map.name", "history-test", 0, pApp);
		WORKBENCH_CHECK(!pApp->tMap.bDirty && pApp->tMapHistory.iUndoCount == undoCount);
		mapedit_map_property_change(NULL, 0, "map.width", "9999999999999999999", 0, pApp);
		WORKBENCH_CHECK(!pApp->tMap.bDirty && pApp->tMapHistory.iUndoCount == undoCount);
	}
	{
		xge_map_t diskCopy = pApp->tMap;
		mapedit_copy_text(diskCopy.sName, sizeof(diskCopy.sName), "externally-edited");
		WORKBENCH_CHECK(mapedit_map_save(&diskCopy, mapPath) == XUI_OK);
		pApp->fNextFileCheck = 0;
		mapedit_poll_external_files(pApp);
		WORKBENCH_CHECK(pApp->bMapExternalConflict && pApp->pExternalBox && xuiMsgBoxIsOpen(pApp->pExternalBox));
		if (pApp->pExternalBox) (void)xuiMsgBoxSetOpen(pApp->pExternalBox, 0);
		mapedit_external_result(pApp->pExternalBox, XUI_MSGBOX_RESULT_NO, pApp);
		WORKBENCH_CHECK(mapedit_save_current(pApp) == XUI_ERROR);
		pApp->bMapExternalConflict = 0;
		pApp->tMapStamp = mapeditFileStamp(mapPath);
	}
	oldWidth = pApp->tMap.iWidth;
	mapedit_map_property_change(NULL, 0, "map.width", "37", 0, pApp);
	WORKBENCH_CHECK(pApp->tMap.iWidth == 37);
	mapedit_undo(pApp); WORKBENCH_CHECK(pApp->tMap.iWidth == oldWidth && !pApp->tMap.bDirty);
	oldTile = pApp->tMap.pTiles[0];
	mapedit_begin_history(pApp); pApp->bBatchEdit = 1; pApp->bGestureDirty = 0;
	mapedit_set_tile(pApp, 0, 0, 0, oldTile == 0 ? 1 : 0);
	pApp->bHistoryFailed = 1; mapedit_finish_map_edit(pApp, "allocation failure");
	WORKBENCH_CHECK(pApp->tMap.pTiles[0] == oldTile && !pApp->tMap.bDirty);
	mapedit_map_property_change(NULL, 0, "map.name", "save-failure-test", 0, pApp);
	/* An existing directory cannot be atomically replaced with a document. */
	mapedit_request_operation(pApp, 2, mapPath);
	WORKBENCH_CHECK(pApp->iPendingOperation == 2);
	(void)xuiMsgBoxSetOpen(pApp->pConfirmBox, 0);
	mapedit_copy_text(pApp->tMap.sPath, sizeof(pApp->tMap.sPath), pApp->sAppDir);
	pApp->tMapStamp = mapeditFileStamp(pApp->sAppDir);
	mapedit_confirm_result(pApp->pConfirmBox, XUI_MSGBOX_RESULT_YES, pApp);
	WORKBENCH_CHECK(!pApp->iPendingOperation && pApp->tMap.bDirty && !strcmp(pApp->tMap.sName, "save-failure-test"));
	mapedit_copy_text(pApp->tMap.sPath, sizeof(pApp->tMap.sPath), mapPath);
	pApp->tMapStamp = mapeditFileStamp(mapPath);
	WORKBENCH_CHECK(mapedit_write_recovery_now(pApp) == XUI_OK);
	mapedit_recovery_path_for(pApp, MAPEDIT_WORKSPACE_MAP, mapPath, recovery, sizeof(recovery));
	WORKBENCH_CHECK(mapedit_file_exists_utf8(recovery));
	mapedit_clear_recovery_for(pApp, MAPEDIT_WORKSPACE_MAP, mapPath);
	mapedit_undo(pApp);
	mapedit_select_workspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	mapedit_tileset_property_change(NULL, 0, "tileset.name", "tileset-save-as-test", 0, pApp);
	pApp->iFileDialogCommand = CMD_FILE_SAVE_AS; pApp->iFileDialogWorkspace = MAPEDIT_WORKSPACE_TILESET;
	mapedit_file_dialog_result(NULL, XUI_FILE_DIALOG_RESULT_OK, setPath, pApp);
	WORKBENCH_CHECK(!pApp->tTileset.bDirty && !strcmp(pApp->tTileset.sPath, setPath));
	WORKBENCH_CHECK(!strcmp(pApp->tMap.sPath, mapPath));
	mapedit_undo(pApp); WORKBENCH_CHECK(pApp->tTileset.bDirty);
	mapedit_redo(pApp); WORKBENCH_CHECK(!pApp->tTileset.bDirty);
	mapedit_undo(pApp);
	mapedit_copy_text(pApp->tTileset.sPath, sizeof(pApp->tTileset.sPath), oldSetPath);
	pApp->tTileset.bDirty = 0; mapeditHistoryClear(&pApp->tTilesetHistory);
	pApp->tTilesetStamp = mapeditFileStamp(oldSetPath);
	mapedit_select_workspace(pApp, MAPEDIT_WORKSPACE_MAP);
	mapedit_copy_text(pApp->tMap.sName, sizeof(pApp->tMap.sName), name);
	mapedit_copy_text(pApp->tMap.sPath, sizeof(pApp->tMap.sPath), oldMapPath);
	pApp->tMap.bDirty = oldDirty; mapedit_clear_history(pApp);
	pApp->tMapStamp = mapeditFileStamp(oldMapPath);
	(void)xuiPropertyGridSetCategoryExpanded(pApp->pMapPropertyGrid, category, 1);
	rect = xuiWidgetGetWorldRect(pApp->pMapTagsCanvas);
	mapedit_view_zoom_at(pApp, pApp->pMapTagsCanvas, 2.0f, 0.0f, 0.0f);
	(void)xuiScrollFrameSetOffset(pApp->pMapTagsScrollFrame, 0, 0);
	WORKBENCH_CHECK(mapedit_pointer_to_map_tile_on_widget(pApp, pApp->pMapTagsCanvas, rect.fX + 33, rect.fY + 33, &tx, &ty) && tx == 1 && ty == 1);
	memset(&event, 0, sizeof(event)); event.iType = XUI_EVENT_POINTER_DOWN; event.iButton = XUI_POINTER_BUTTON_MIDDLE;
	event.fX = rect.fX + 60; event.fY = rect.fY + 60;
	WORKBENCH_CHECK(mapedit_view_interact(pApp, pApp->pMapTagsCanvas, &event) && pApp->bViewPanning);
	event.iType = XUI_EVENT_POINTER_CAPTURE_LOST;
	WORKBENCH_CHECK(mapedit_view_interact(pApp, pApp->pMapTagsCanvas, &event) && !pApp->bViewPanning);
	mapedit_view_zoom_at(pApp, pApp->pMapTagsCanvas, 1.0f, 0.0f, 0.0f);
	mapedit_map_tags_scroll_reset(pApp);
	mapedit_refresh_map_properties(pApp); mapedit_refresh_tileset_properties(pApp);
	mapedit_update_window_title(pApp);
	mapedit_update_status_details(pApp);
	(void)mapedit_delete_file_utf8(mapPath); (void)mapedit_delete_file_utf8(setPath);
	pApp->iSmokeWorkbenchStage = 2; pApp->bSmokeWorkbenchOK = ok;
	printf("workbench: native-close/cancel, save-failure, save-as routing, undo/savepoint, property state, external conflict, recovery, zoom/pan %s\n", ok ? "OK" : "FAILED");
#undef WORKBENCH_CHECK
	return ok;
}

static int mapedit_frame(void* pUser)
{
	mapedit_app_t* pApp = (mapedit_app_t*)pUser;
	xui_rect_i_t full;
	xui_rect_t src;
	xui_rect_t dst;
	xui_render_stats_t stats;
	int targetW;
	int targetH;
	int ret;
	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	ret = mapedit_sync_viewport(pApp);
	if ( ret != XUI_OK ) goto frame_end;
	ret = xuiProxyXgePumpInput(pApp->pContext);
	if ( ret != XUI_OK ) goto frame_end;
	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret != XUI_OK ) goto frame_end;
	mapedit_update_preview(pApp, xgeGetDelta());
	ret = xuiLayout(pApp->pContext);
	if ( ret != XUI_OK ) goto frame_end;
	mapedit_map_workspace_scroll_sync_content(pApp);
	mapedit_tileset_workspace_scroll_sync_content(pApp);
	ret = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( ret != XUI_OK ) goto frame_end;
	mapedit_maybe_write_recovery(pApp);
	mapedit_poll_layout(pApp);
	mapedit_poll_external_files(pApp);
	mapedit_update_summary_checks(pApp);
	if (pApp->bSmokeWorkbench && pApp->iFrame > 0) (void)mapedit_smoke_workbench(pApp);
	ret = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(232, 241, 250, 255));
	if ( ret != XUI_OK ) goto frame_end;
	targetW = pApp->iTargetWidth > 0 ? pApp->iTargetWidth : MAPEDIT_W;
	targetH = pApp->iTargetHeight > 0 ? pApp->iTargetHeight : MAPEDIT_H;
	full = (xui_rect_i_t){0, 0, targetW, targetH};
	ret = xuiRender(pApp->pContext, pApp->pTarget, &full, 1);
	if ( ret != XUI_OK ) goto frame_end;
	pApp->bRenderOK = 1;
	if (pApp->sCapturePath[0] && pApp->iFrameLimit > 0 && pApp->iFrame + 1 >= pApp->iFrameLimit) {
		size_t bytes = (size_t)targetW * (size_t)targetH * 4u;
		void* pixels = malloc(bytes);
		if (pixels && pApp->tProxy.surfaceReadRGBA(&pApp->tProxy, pApp->pTarget, pixels, targetW * 4) == XUI_OK)
			(void)xgeImageSavePNG(pApp->sCapturePath, targetW, targetH, pixels, targetW * 4);
		free(pixels);
	}
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	src = (xui_rect_t){0, 0, targetW, targetH};
	dst = src;
	ret = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, src, dst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	frame_end:
	{
		int endResult = xgeEnd();
		if (ret == XUI_OK) ret = endResult;
	}
	if ( ret != XGE_OK ) return ret;
	pApp->iFrame++;
	if ( pApp->iFrameLimit > 0 && pApp->iFrame >= pApp->iFrameLimit ) {
		mapedit_save_layouts(pApp);
		memset(&stats, 0, sizeof(stats));
		stats.iSize = sizeof(stats);
		(void)xuiGetRenderStats(pApp->pContext, &stats);
		printf("mapedit_xui2 final-summary frames=%d create=%d layout=%d data=%d map=%d tileset=%d render=%d edit=%d customDefault=%d maps=%d tilesets=%d materials=%d updatedCaches=%d drawnCaches=%d visibleTiles=%d\n",
			pApp->iFrame,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bDataOK,
			pApp->bMapOK,
			pApp->bTilesetOK,
			pApp->bRenderOK,
			pApp->bEditOK,
			pApp->bSmokeCustomDefault ? pApp->bSmokeCustomDefaultOK : 1,
			pApp->tMapFiles.iCount,
			pApp->tTilesetFiles.iCount,
			pApp->tMaterialFiles.iCount,
			stats.iUpdatedCaches,
			stats.iDrawnCaches,
			pApp->iLastVisibleCells);
		xgeQuit();
	} else if ( pApp->iFrameLimit > 0 ) {
		xgeRenderRequest();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	static mapedit_app_t app;
	xge_desc_t desc;
	char recoveryPath[MAPEDIT_PATH_MAX];
	int bWrapperChild;
	int i;
	int ret;
	memset(&app, 0, sizeof(app));
	app.iSetupTileWidth = 16;
	app.iSetupTileHeight = 16;
	app.iSetupTilesPerRow = 20;
	app.iSetupStateMin = 0;
	app.iSetupStateMax = 3;
	gMapeditTileWidth = app.iSetupTileWidth;
	gMapeditTileHeight = app.iSetupTileHeight;
	gMapeditTilesPerRow = app.iSetupTilesPerRow;
	mapedit_get_app_dir(app.sAppDir, sizeof(app.sAppDir));
	mapedit_configure_process_startup(app.sAppDir);
	mapedit_get_user_dir(&app);
	(void)mapedit_ensure_dir_utf8(app.sUserDir);
	mapedit_set_project_dir(&app, getenv("XGE_MAPEDIT_PROJECT_DIR"));
	mapedit_startup_log_open(&app);
	mapedit_startup_log(&app, "start appDir=%s userDir=%s projectDir=%s argc=%d",
		app.sAppDir, app.sUserDir, app.sProjectDir, argc);
	bWrapperChild = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--mapedit-wrapper-child") == 0 ) {
			bWrapperChild = 1;
			break;
		}
	}
	if ( bWrapperChild ) {
		mapedit_startup_log(&app, "single-instance managed by launcher");
	} else if ( !mapedit_acquire_single_instance(&app) ) {
		mapedit_release_single_instance(&app);
		return 0;
	}
	app.iStartupWorkspace = MAPEDIT_WORKSPACE_TILESET;
	app.iFrameLimit = mapedit_arg_int(getenv("XGE_MAPEDIT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--mapedit-wrapper-child") == 0 ) {
			continue;
		} else if ( strcmp(argv[i], "--frames") == 0 && i + 1 < argc ) {
			app.iFrameLimit = mapedit_arg_int(argv[++i], app.iFrameLimit);
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			app.iFrameLimit = mapedit_arg_int(argv[i] + 9, app.iFrameLimit);
		} else if ( strcmp(argv[i], "--project") == 0 && i + 1 < argc ) {
			mapedit_set_project_dir(&app, argv[++i]);
		} else if ( strncmp(argv[i], "--project=", 10) == 0 ) {
			mapedit_set_project_dir(&app, argv[i] + 10);
		} else if ( strcmp(argv[i], "--capture") == 0 && i + 1 < argc ) {
			mapedit_copy_text(app.sCapturePath, sizeof(app.sCapturePath), argv[++i]);
		} else if ( strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "--workspace=map") == 0 ) {
			app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		} else if ( strcmp(argv[i], "--tileset") == 0 || strcmp(argv[i], "--workspace=tileset") == 0 ) {
			app.iStartupWorkspace = MAPEDIT_WORKSPACE_TILESET;
		} else if ( strcmp(argv[i], "--preview") == 0 ) {
			app.bPreview = 1;
			app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		} else if ( strcmp(argv[i], "--large-map") == 0 && i + 1 < argc ) {
			if ( mapedit_parse_size_arg(argv[++i], &app.iLargeMapWidth, &app.iLargeMapHeight) ) app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		} else if ( strncmp(argv[i], "--large-map=", 12) == 0 ) {
			if ( mapedit_parse_size_arg(argv[i] + 12, &app.iLargeMapWidth, &app.iLargeMapHeight) ) app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		} else if ( strcmp(argv[i], "--smoke-workbench") == 0 ) {
			app.bSmokeWorkbench = 1;
			app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		} else if ( strcmp(argv[i], "--smoke-custom-default") == 0 ) {
			app.bSmokeCustomDefault = 1;
			app.iStartupWorkspace = MAPEDIT_WORKSPACE_MAP;
		}
	}
	if (app.iFrameLimit > 0) {
		/* Automated runs must not overwrite the user's interactive layout/recovery. */
		mapedit_path_join(app.sUserDir, sizeof(app.sUserDir), app.sAppDir, "test-user");
		(void)mapedit_ensure_dir_utf8(app.sUserDir);
		(void)mapedit_ensure_dir_utf8(app.sAppDir);
	}
	if ( !mapedit_prepare_project_dirs(&app) ) {
		mapedit_startup_log(&app, "project directory unavailable: %s", app.sProjectDir);
		printf("mapedit_xui2: project directory unavailable: %s\n", app.sProjectDir);
		mapedit_release_single_instance(&app);
		return 1;
	}
	mapedit_startup_log(&app, "using projectDir=%s", app.sProjectDir);
	memset(&desc, 0, sizeof(desc));
	desc.sTitle = "MapEdit";
	desc.iWidth = MAPEDIT_W;
	desc.iHeight = MAPEDIT_H;
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_RESIZABLE | XGE_INIT_HIGHDPI |
		XGE_INIT_VSYNC | XGE_INIT_ON_DEMAND;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;
	mapedit_startup_log(&app, "before xgeInit");
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) {
		mapedit_startup_log(&app, "xgeInit failed ret=%d", ret);
		printf("mapedit_xui2: xgeInit failed: %d\n", ret);
		mapedit_release_single_instance(&app);
		return 1;
	}
	mapedit_startup_log(&app, "after xgeInit");
	mapedit_startup_log(&app, "before scan data");
	mapedit_scan_data(&app);
	mapedit_startup_log(&app, "after scan data maps=%d tilesets=%d materials=%d", app.tMapFiles.iCount, app.tTilesetFiles.iCount, app.tMaterialFiles.iCount);
	mapedit_startup_log(&app, "before create ui");
	ret = mapedit_create_ui(&app);
	if ( ret != XUI_OK ) {
		mapedit_startup_log(&app, "create ui failed ret=%d", ret);
		printf("mapedit_xui2: create ui failed: %d\n", ret);
		mapedit_destroy(&app);
		xgeUnit();
		mapedit_release_single_instance(&app);
		return 1;
	}
	mapedit_startup_log(&app, "after create ui");
	mapedit_load_initial_data(&app);
	mapedit_startup_log(&app, "after load initial data");
	if ( app.iLargeMapWidth > 0 && app.iLargeMapHeight > 0 ) mapedit_make_large_map(&app, app.iLargeMapWidth, app.iLargeMapHeight);
	if ( app.iFrameLimit > 0 ) app.bEditOK = mapedit_run_edit_history_smoke(&app);
	if ( app.bSmokeCustomDefault ) app.bSmokeCustomDefaultOK = mapedit_run_custom_default_smoke(&app);
	mapedit_select_workspace(&app, app.iStartupWorkspace);
	mapedit_recovery_path(&app, recoveryPath, sizeof(recoveryPath));
	if ( mapedit_file_exists_utf8(recoveryPath) ) {
		mapedit_status(&app, "检测到自动保存：可从“文件 → 恢复自动保存”载入");
	}
	mapedit_startup_log(&app, "before xgeRun");
	xgeSetQuitRequestCallback(mapedit_native_close, &app);
	ret = xgeRun(mapedit_frame, &app);
	mapedit_startup_log(&app, "after xgeRun ret=%d", ret);
	mapedit_save_layouts(&app);
	if (app.tMap.bDirty || app.tTileset.bDirty) (void)mapedit_write_recovery_now(&app);
	mapedit_destroy(&app);
	xgeUnit();
	i = (ret == XGE_OK && app.bCreateOK && app.bLayoutOK && app.bDataOK && app.bMapOK && app.bTilesetOK && app.bRenderOK &&
		(app.iFrameLimit <= 0 || app.bEditOK) &&
		(!app.bSmokeCustomDefault || app.bSmokeCustomDefaultOK) &&
		(!app.bSmokeWorkbench || app.bSmokeWorkbenchOK)) ? 0 : 1;
	mapedit_release_single_instance(&app);
	return i;
}
