#ifndef MAPEDIT_APP_H
#define MAPEDIT_APP_H

#include "xge.h"
#include "map_sdk/xge_map.h"
#include "../ui/mapedit_tile_grid.h"

#define MAPEDIT_WORKSPACE_COUNT 2
#define MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX 16
#define MAPEDIT_TILESET_DOCK_WINDOW_COUNT 7
#define MAPEDIT_MAP_DOCK_WINDOW_COUNT 8
#define MAPEDIT_CONTENT_COUNT 32
#define MAPEDIT_LABEL_COUNT 128
#define MAPEDIT_TILE_SOURCE_INFO_LABEL_COUNT 18
#define MAPEDIT_MATERIAL_ITEM_MAX 2048
#define MAPEDIT_MATERIAL_TEXT_CAPACITY 128
#define MAPEDIT_MATERIAL_PATH_CAPACITY 512
#define MAPEDIT_TILESET_SET_MAX 128
#define MAPEDIT_TILESET_SET_TEXT_CAPACITY 128
#define MAPEDIT_TILESET_SET_FILE_CAPACITY 128
#define MAPEDIT_TILESET_STATIC_TILE_OPTION_MAX 256
#define MAPEDIT_TILESET_SPECIAL_TYPE_OPTION_MAX 5
#define MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX 512
#define MAPEDIT_TILESET_CUSTOM_DATA_CAPACITY 1024
#define MAPEDIT_TILESET_SPECIAL_TILE_MAX 512
#define MAPEDIT_TILESET_SPECIAL_TILE_TYPE_CAPACITY 64
#define MAPEDIT_TILESET_PASSAGE_MAX 65536
#define MAPEDIT_TILESET_PASSAGE_DEFAULT 255
#define MAPEDIT_TILESET_ACTOR_OVERLAY_MAX 65536
#define MAPEDIT_TILESET_ACTOR_OVERLAY_DEFAULT 0
#define MAPEDIT_CUSTOM_CHANNEL_MAX 32
#define MAPEDIT_CUSTOM_OPTION_MAX 64
#define MAPEDIT_CUSTOM_ID_CAPACITY 64
#define MAPEDIT_CUSTOM_NAME_CAPACITY 96
#define MAPEDIT_CUSTOM_TEXT_CAPACITY 128
#define MAPEDIT_CUSTOM_VALUE_CAPACITY 128
#define MAPEDIT_TILESET_CUSTOM_TILE_MAX 65536
#define MAPEDIT_MAP_ITEM_MAX 256
#define MAPEDIT_MAP_TEXT_CAPACITY 128
#define MAPEDIT_MAP_FILE_CAPACITY 128
#define MAPEDIT_MAP_CUSTOM_DATA_CAPACITY 1024
#define MAPEDIT_MAP_TILESET_OPTION_MAX 256
#define MAPEDIT_MAP_WIDTH_DEFAULT 100
#define MAPEDIT_MAP_HEIGHT_DEFAULT 100
#define MAPEDIT_MAP_LAYER_DEFAULT 3
#define MAPEDIT_MAP_SIZE_MAX 1048576
#define MAPEDIT_MAP_LAYER_OPTION_MAX 32
#define MAPEDIT_MAP_LAYER_NAME_CAPACITY 64
#define MAPEDIT_MAP_HISTORY_LIMIT 128

#define MAPEDIT_WORKSPACE_TILESET 0
#define MAPEDIT_WORKSPACE_MAP 1

#define MAPEDIT_TILESET_PROPERTY_MODE_SET 0
#define MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL 1
#define MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM 2

#define MAPEDIT_MAP_TOOL_BRUSH 0
#define MAPEDIT_MAP_TOOL_ERASER 1
#define MAPEDIT_MAP_TOOL_LINE 2
#define MAPEDIT_MAP_TOOL_RECT 3
#define MAPEDIT_MAP_TOOL_CIRCLE 4
#define MAPEDIT_MAP_TOOL_BUCKET 5
#define MAPEDIT_MAP_TOOL_COUNT 6

#define MAPEDIT_MAP_COMMAND_PREVIEW 0
#define MAPEDIT_MAP_COMMAND_GRID 1
#define MAPEDIT_MAP_COMMAND_UNDO 2
#define MAPEDIT_MAP_COMMAND_REDO 3
#define MAPEDIT_MAP_COMMAND_COUNT 4

typedef struct mapedit_workspace_def_t mapedit_workspace_def_t;

enum {
	MAPEDIT_CMD_OPEN_MAP = 100,
	MAPEDIT_CMD_SAVE_MAP,
	MAPEDIT_CMD_SAVE_MAP_AS,
	MAPEDIT_CMD_EXIT,
	MAPEDIT_CMD_RESET_LAYOUT,
	MAPEDIT_CMD_ABOUT
};

typedef struct mapedit_workspace_t {
	int iType;
	const char* sName;
	const char* sLayoutFile;
	const char* sDefaultLayoutFile;
	const mapedit_workspace_def_t* pDef;
	xge_xui_widget pPageWidget;
	xge_xui_widget pDockWidget;
	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t arrDockWindow[MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX];
	int iWindowCount;
	int bDockReady;
	int bLayoutLoaded;
	int bLayoutSaved;
} mapedit_workspace_t;

typedef struct mapedit_tile_source_t {
	int iImageWidth;
	int iImageHeight;
	int iTileWidth;
	int iTileHeight;
	int iTilesPerRow;
	int iOffsetX;
	int iOffsetY;
	int iGapX;
	int iGapY;
	int iHoverCol;
	int iHoverRow;
	int iSelectCol;
	int iSelectRow;
	int bHasSelection;
	int iLastSentTileId;
	int bBrushReady;
} mapedit_tile_source_t;

typedef struct mapedit_material_item_t {
	char sFile[MAPEDIT_MATERIAL_TEXT_CAPACITY];
	char sName[MAPEDIT_MATERIAL_TEXT_CAPACITY];
	char sText[MAPEDIT_MATERIAL_TEXT_CAPACITY * 2];
	char sPath[MAPEDIT_MATERIAL_PATH_CAPACITY];
} mapedit_material_item_t;

typedef struct mapedit_tileset_special_tile_t {
	char sType[MAPEDIT_TILESET_SPECIAL_TILE_TYPE_CAPACITY];
	char sFile[MAPEDIT_TILESET_SET_FILE_CAPACITY];
} mapedit_tileset_special_tile_t;

typedef struct mapedit_custom_option_t {
	char sText[MAPEDIT_CUSTOM_TEXT_CAPACITY];
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int iBit;
	int bHasBit;
} mapedit_custom_option_t;

typedef struct mapedit_custom_channel_def_t {
	char sId[MAPEDIT_CUSTOM_ID_CAPACITY];
	char sName[MAPEDIT_CUSTOM_NAME_CAPACITY];
	char sScope[MAPEDIT_CUSTOM_ID_CAPACITY];
	char sDataType[MAPEDIT_CUSTOM_ID_CAPACITY];
	char sMarkMode[MAPEDIT_CUSTOM_ID_CAPACITY];
	char sDefaultValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int bHasMin;
	int bHasMax;
	int iMinValue;
	int iMaxValue;
	int iOptionCount;
	mapedit_custom_option_t arrOptions[MAPEDIT_CUSTOM_OPTION_MAX];
	const char* arrOptionItems[MAPEDIT_CUSTOM_OPTION_MAX];
} mapedit_custom_channel_def_t;

typedef struct mapedit_tileset_custom_value_t {
	int iTileId;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
} mapedit_tileset_custom_value_t;

typedef struct mapedit_tileset_custom_channel_data_t {
	char sChannelId[MAPEDIT_CUSTOM_ID_CAPACITY];
	mapedit_tileset_custom_value_t* arrValues;
	int iValueCount;
	int iValueCapacity;
} mapedit_tileset_custom_channel_data_t;

typedef struct mapedit_tileset_set_item_t {
	char sFile[MAPEDIT_TILESET_SET_FILE_CAPACITY];
	char sName[MAPEDIT_TILESET_SET_TEXT_CAPACITY];
	char sStaticTiles[MAPEDIT_TILESET_SET_TEXT_CAPACITY];
	char sText[MAPEDIT_TILESET_SET_TEXT_CAPACITY * 3];
	char sCustomData[MAPEDIT_TILESET_CUSTOM_DATA_CAPACITY];
	int iSpecialTileCount;
	int iPassageCount;
	int iActorOverlayCount;
	unsigned char arrPassage[MAPEDIT_TILESET_PASSAGE_MAX];
	unsigned char arrActorOverlay[MAPEDIT_TILESET_ACTOR_OVERLAY_MAX];
	mapedit_tileset_special_tile_t arrSpecialTiles[MAPEDIT_TILESET_SPECIAL_TILE_MAX];
	mapedit_tileset_custom_channel_data_t arrCustomTileData[MAPEDIT_CUSTOM_CHANNEL_MAX];
	int iCustomTileDataCount;
} mapedit_tileset_set_item_t;

typedef struct mapedit_map_passage_override_t {
	int iCellId;
	unsigned char iValue;
} mapedit_map_passage_override_t;

typedef struct mapedit_map_item_t {
	char sFile[MAPEDIT_MAP_FILE_CAPACITY];
	char sName[MAPEDIT_MAP_TEXT_CAPACITY];
	char sTileset[MAPEDIT_TILESET_SET_FILE_CAPACITY];
	char sText[MAPEDIT_MAP_TEXT_CAPACITY * 3];
	char sCustomData[MAPEDIT_MAP_CUSTOM_DATA_CAPACITY];
	int iState;
	int iWidth;
	int iHeight;
	int iLayerCount;
	mapedit_map_passage_override_t* arrPassageOverrides;
	int iPassageOverrideCount;
	int iPassageOverrideCapacity;
	mapedit_tileset_custom_channel_data_t arrCellCustomData[MAPEDIT_CUSTOM_CHANNEL_MAX];
	int iCellCustomDataCount;
} mapedit_map_item_t;

typedef struct mapedit_setup_layer_t {
	int iId;
	char sName[MAPEDIT_MAP_LAYER_NAME_CAPACITY];
	int bVisible;
	int bEditable;
	int bAboveActor;
} mapedit_setup_layer_t;

typedef struct mapedit_map_cell_change_t {
	int iLayer;
	int x;
	int y;
	int iBeforeTile;
	int iAfterTile;
} mapedit_map_cell_change_t;

typedef struct mapedit_map_command_t {
	int iTool;
	int iLayer;
	int iChangeCount;
	int iChangeCapacity;
	mapedit_map_cell_change_t* arrChanges;
} mapedit_map_command_t;

typedef struct mapedit_map_history_t {
	mapedit_map_command_t arrUndo[MAPEDIT_MAP_HISTORY_LIMIT];
	mapedit_map_command_t arrRedo[MAPEDIT_MAP_HISTORY_LIMIT];
	mapedit_map_command_t tCurrent;
	int iUndoCount;
	int iRedoCount;
	int bRecording;
	int bReplaying;
} mapedit_map_history_t;

typedef struct mapedit_app_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_map_t tMap;
	xge_xui_widget pMenuWidget;
	xge_xui_widget pWorkspaceWidget;
	xge_xui_widget pTilesetEditorButtonWidget;
	xge_xui_widget pMapEditorButtonWidget;
	xge_xui_widget pClientWidget;
	xge_xui_widget pStatusWidget;
	xge_xui_menubar_t tMenuBar;
	xge_xui_button_t tTilesetEditorButton;
	xge_xui_button_t tMapEditorButton;
	xge_xui_menu_t tFileMenu;
	xge_xui_menu_t tViewMenu;
	xge_xui_menu_t tHelpMenu;
	xge_xui_status_bar_t tStatusBar;
	mapedit_workspace_t arrWorkspace[MAPEDIT_WORKSPACE_COUNT];
	xge_xui_widget arrContent[MAPEDIT_CONTENT_COUNT];
	xge_xui_label_t arrLabel[MAPEDIT_LABEL_COUNT];
	xge_xui_widget pTileSourcePreviewWidget;
	xge_xui_widget pMaterialComboWidget;
	xge_xui_widget pMaterialListWidget;
	xge_xui_widget pMaterialCreateButtonWidget;
	xge_xui_widget pMaterialRenameWidget;
	xge_xui_widget pMaterialEditWindowWidget;
	xge_xui_widget pMaterialEditContentWidget;
	xge_xui_widget pMaterialEditEditorAreaWidget;
	xge_xui_widget pMaterialEditOriginalPaneWidget;
	xge_xui_widget pMaterialEditSourcePaneWidget;
	xge_xui_widget pMaterialEditOriginalCanvasWidget;
	xge_xui_widget pMaterialEditSourceCanvasWidget;
	xge_xui_widget pMaterialEditOriginalGridWidget;
	xge_xui_widget pMaterialEditSourceGridWidget;
	xge_xui_widget pMaterialEditLoadSourceButtonWidget;
	xge_xui_widget pMaterialEditFormWidget;
	xge_xui_widget pMaterialEditNameLabelWidget;
	xge_xui_widget pMaterialEditNameInputWidget;
	xge_xui_widget pMaterialEditFileLabelWidget;
	xge_xui_widget pMaterialEditFileInputWidget;
	xge_xui_widget pMaterialEditSpacerWidget;
	xge_xui_widget pMaterialEditOkWidget;
	xge_xui_widget pMaterialEditCancelWidget;
	xge_xui_widget pMaterialEditMsgTipWidget;
	xge_xui_widget pMaterialViewWindowWidget;
	xge_xui_widget pMaterialViewScrollWidget;
	xge_xui_widget pMaterialViewImageWidget;
	xge_xui_widget pTilesetSetListWidget;
	xge_xui_widget pTilesetSetAddButtonWidget;
	xge_xui_widget pTilesetArrangeCanvasWidget;
	xge_xui_widget pTilesetArrangeGridWidget;
	xge_xui_widget pTilesetPassageCanvasWidget;
	xge_xui_widget pTilesetPassageGridWidget;
	xge_xui_widget pTilesetActorOverlayCanvasWidget;
	xge_xui_widget pTilesetActorOverlayGridWidget;
	xge_xui_widget pTilesetTagsControlRowWidget;
	xge_xui_widget pTilesetTagsChannelComboWidget;
	xge_xui_widget pTilesetTagsInspectCheckWidget;
	xge_xui_widget pTilesetTagsFormGridWidget;
	xge_xui_widget pTilesetTagsCanvasWidget;
	xge_xui_widget pTilesetTagsGridWidget;
	xge_xui_widget pTilesetPropertiesGridWidget;
	xge_xui_widget pMapListWidget;
	xge_xui_widget pMapAddButtonWidget;
	xge_xui_widget pMapPropertiesGridWidget;
	xge_xui_widget pMapEditToolbarRowWidget;
	xge_xui_widget pMapEditToolToolbarWidget;
	xge_xui_widget pMapEditLayerComboWidget;
	xge_xui_widget pMapEditCommandToolbarWidget;
	xge_xui_widget pMapTileSelectCanvasWidget;
	xge_xui_widget pMapTileSelectGridWidget;
	xge_xui_widget pMapEditCanvasWidget;
	xge_xui_widget pMapEditGridWidget;
	xge_xui_widget pMapPassageCanvasWidget;
	xge_xui_widget pMapPassageGridWidget;
	xge_xui_widget pMapTagsControlRowWidget;
	xge_xui_widget pMapTagsChannelComboWidget;
	xge_xui_widget pMapTagsInspectCheckWidget;
	xge_xui_widget pMapTagsFormGridWidget;
	xge_xui_widget pMapTagsCanvasWidget;
	xge_xui_widget pMapTagsGridWidget;
	xge_xui_combo_box_t tMaterialCombo;
	xge_xui_list_view_t tMaterialList;
	xge_xui_button_t tMaterialCreateButton;
	xge_xui_menu_t tMaterialContextMenu;
	xge_xui_input_box_t tMaterialRenameBox;
	xge_xui_window_t tMaterialEditWindow;
	xge_xui_split_layout_t tMaterialEditSplit;
	xge_xui_scroll_model_t tMaterialEditOriginalScrollModel;
	xge_xui_scroll_model_t tMaterialEditSourceScrollModel;
	xge_xui_scroll_frame_t tMaterialEditOriginalScrollFrame;
	xge_xui_scroll_frame_t tMaterialEditSourceScrollFrame;
	xge_xui_button_t tMaterialEditOkButton;
	xge_xui_button_t tMaterialEditCancelButton;
	xge_xui_button_t tMaterialEditLoadSourceButton;
	xge_xui_combo_box_t tTilesetTagsChannelCombo;
	xge_xui_checkbox_t tTilesetTagsInspectCheck;
	xge_xui_label_t tMaterialEditOriginalTitleLabel;
	xge_xui_label_t tMaterialEditSourceTitleLabel;
	xge_xui_label_t tMaterialEditOriginalHintLabel;
	xge_xui_label_t tMaterialEditSourceHintLabel;
	xge_xui_label_t tMaterialEditNameLabel;
	xge_xui_label_t tMaterialEditFileLabel;
	xge_xui_input_t tMaterialEditNameInput;
	xge_xui_input_t tMaterialEditFileInput;
	xge_xui_msg_tip_t tMaterialEditMsgTip;
	xge_xui_window_t tMaterialViewWindow;
	xge_xui_scroll_view_t tMaterialViewScroll;
	xge_xui_list_view_t tTilesetSetList;
	xge_xui_button_t tTilesetSetAddButton;
	xge_xui_list_view_t tMapList;
	xge_xui_button_t tMapAddButton;
	xge_xui_toolbar_t tMapEditToolToolbar;
	xge_xui_combo_box_t tMapEditLayerCombo;
	xge_xui_toolbar_t tMapEditCommandToolbar;
	xge_xui_combo_box_t tMapTagsChannelCombo;
	xge_xui_checkbox_t tMapTagsInspectCheck;
	xge_xui_scroll_model_t tTilesetArrangeScrollModel;
	xge_xui_scroll_model_t tTilesetPassageScrollModel;
	xge_xui_scroll_model_t tTilesetActorOverlayScrollModel;
	xge_xui_scroll_model_t tTilesetTagsScrollModel;
	xge_xui_scroll_model_t tMapTileSelectScrollModel;
	xge_xui_scroll_model_t tMapEditScrollModel;
	xge_xui_scroll_model_t tMapPassageScrollModel;
	xge_xui_scroll_model_t tMapTagsScrollModel;
	xge_xui_scroll_frame_t tTilesetArrangeScrollFrame;
	xge_xui_scroll_frame_t tTilesetPassageScrollFrame;
	xge_xui_scroll_frame_t tTilesetActorOverlayScrollFrame;
	xge_xui_scroll_frame_t tTilesetTagsScrollFrame;
	xge_xui_scroll_frame_t tMapTileSelectScrollFrame;
	xge_xui_scroll_frame_t tMapEditScrollFrame;
	xge_xui_scroll_frame_t tMapPassageScrollFrame;
	xge_xui_scroll_frame_t tMapTagsScrollFrame;
	xge_xui_property_grid_t tTilesetPropertiesGrid;
	xge_xui_property_grid_t tTilesetTagsFormGrid;
	xge_xui_property_grid_t tMapPropertiesGrid;
	xge_xui_property_grid_t tMapTagsFormGrid;
	mapedit_tile_grid_t tTilesetArrangeGrid;
	mapedit_tile_grid_t tTilesetPassageGrid;
	mapedit_tile_grid_t tTilesetActorOverlayGrid;
	mapedit_tile_grid_t tTilesetTagsGrid;
	mapedit_tile_grid_t tMapTileSelectGrid;
	mapedit_tile_grid_t tMapEditGrid;
	mapedit_tile_grid_t tMapPassageGrid;
	mapedit_tile_grid_t tMapTagsGrid;
	mapedit_tile_grid_t tMaterialEditOriginalGrid;
	mapedit_tile_grid_t tMaterialEditSourceGrid;
	xge_image_t tMaterialEditSourceImage;
	xui_texture pMaterialEditSourceTexture;
	xui_texture pMaterialEditOutputTexture;
	uint32_t* pMaterialEditOutputPixels;
	int iMaterialEditMode;
	int iMaterialEditTileWidth;
	int iMaterialEditTileHeight;
	int iMaterialEditOutputCols;
	int iMaterialEditOutputRows;
	int iMaterialEditUsedCols;
	int iMaterialEditUsedRows;
	int iMaterialEditSourceCol;
	int iMaterialEditSourceRow;
	int iMaterialEditSourceCols;
	int iMaterialEditSourceRows;
	int bMaterialEditSourceReady;
	int bMaterialEditOutputDirty;
	char sMaterialEditSourcePath[MAPEDIT_MATERIAL_PATH_CAPACITY];
	mapedit_material_item_t arrMaterialItems[MAPEDIT_MATERIAL_ITEM_MAX];
	const char* arrMaterialListItems[MAPEDIT_MATERIAL_ITEM_MAX];
	mapedit_tileset_set_item_t arrTilesetSetItems[MAPEDIT_TILESET_SET_MAX];
	const char* arrTilesetSetListItems[MAPEDIT_TILESET_SET_MAX];
	mapedit_map_item_t arrMapItems[MAPEDIT_MAP_ITEM_MAX];
	const char* arrMapListItems[MAPEDIT_MAP_ITEM_MAX];
	char arrTilesetStaticTileOptionText[MAPEDIT_TILESET_STATIC_TILE_OPTION_MAX][MAPEDIT_TILESET_SET_TEXT_CAPACITY];
	const char* arrTilesetStaticTileOptions[MAPEDIT_TILESET_STATIC_TILE_OPTION_MAX];
	char arrTilesetSpecialFileOptionText[MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX][MAPEDIT_TILESET_SET_TEXT_CAPACITY * 2];
	char arrTilesetSpecialFileOptionValue[MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX][MAPEDIT_TILESET_SET_FILE_CAPACITY];
	const char* arrTilesetSpecialFileOptions[MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX];
	char arrMapTilesetOptionText[MAPEDIT_MAP_TILESET_OPTION_MAX][MAPEDIT_MAP_TEXT_CAPACITY * 2];
	char arrMapTilesetOptionValue[MAPEDIT_MAP_TILESET_OPTION_MAX][MAPEDIT_TILESET_SET_FILE_CAPACITY];
	const char* arrMapTilesetOptions[MAPEDIT_MAP_TILESET_OPTION_MAX];
	mapedit_setup_layer_t arrSetupLayers[MAPEDIT_MAP_LAYER_OPTION_MAX];
	char arrMapLayerOptionText[MAPEDIT_MAP_LAYER_OPTION_MAX][MAPEDIT_MAP_LAYER_NAME_CAPACITY];
	xge_xui_combo_box_item_t arrMapLayerOptionItems[MAPEDIT_MAP_LAYER_OPTION_MAX];
	mapedit_custom_channel_def_t arrCustomChannels[MAPEDIT_CUSTOM_CHANNEL_MAX];
	char arrTilesetTagChannelText[MAPEDIT_CUSTOM_CHANNEL_MAX][MAPEDIT_CUSTOM_TEXT_CAPACITY];
	xge_xui_combo_box_item_t arrTilesetTagChannelItems[MAPEDIT_CUSTOM_CHANNEL_MAX];
	char arrMapTagChannelText[MAPEDIT_CUSTOM_CHANNEL_MAX][MAPEDIT_CUSTOM_TEXT_CAPACITY];
	xge_xui_combo_box_item_t arrMapTagChannelItems[MAPEDIT_CUSTOM_CHANNEL_MAX];
	int iMaterialCategory;
	int iMaterialItemCount;
	int iMaterialPreviewIndex;
	int iMaterialContextIndex;
	int iMaterialRenameIndex;
	int iMaterialEditIndex;
	int bMaterialEditReady;
	int iTilesetSetCount;
	int iTilesetSetSelected;
	int iMapCount;
	int iMapSelected;
	int iMapTilesetOptionCount;
	int iMapTileDataCount;
	int iMapBrushTileId;
	int iMapBrushCols;
	int iMapBrushRows;
	int iMapSelectedTool;
	int iMapActiveLayer;
	int iMapLayerOptionCount;
	int bMapShowGrid;
	int bMapPreviewMode;
	float fMapPreviewAnimTime;
	int iMapPreviewAnimFrame;
	int iTilesetStaticTileOptionCount;
	int iTilesetPropertiesMode;
	int iTilesetSelectedSpecialTileId;
	int iTilesetSpecialFileOptionCount;
	int iTilesetArrangeSelectedTile;
	int iTilesetPassageSelectedTile;
	int iTilesetActorOverlaySelectedTile;
	int iTilesetTagsSelectedTile;
	int iMapPassageSelectedCell;
	int iMapTagsSelectedCell;
	int iTilesetTagChannel;
	int iTilesetTagChannelOptionCount;
	int bTilesetTagsInspectMode;
	int bTilesetTagsFormUpdating;
	int iMapTagChannel;
	int iMapTagChannelOptionCount;
	int bMapTagsInspectMode;
	int bMapTagsFormUpdating;
	float fTilesetPassageLastMouseX;
	float fTilesetPassageLastMouseY;
	float fMapPassageLastMouseX;
	float fMapPassageLastMouseY;
	xui_texture pMaterialPreviewTexture;
	xui_texture pMaterialViewTexture;
	xui_texture pTilesetArrangeStaticTexture;
	xui_texture arrTilesetArrangeSpecialTexture[MAPEDIT_TILESET_SPECIAL_TILE_MAX];
	xui_texture pTilesetPassageStaticTexture;
	xui_texture pTilesetActorOverlayStaticTexture;
	xui_texture pTilesetTagsStaticTexture;
	xui_texture pMapTilesetStaticTexture;
	xui_texture arrMapTilesetSpecialTexture[MAPEDIT_TILESET_SPECIAL_TILE_MAX];
	xui_texture arrTilesetPassageOverlayTexture[256];
	xui_texture_desc_t tMaterialViewTextureDesc;
	xui_texture_desc_t tMaterialPreviewDesc;
	xui_texture_desc_t tTilesetArrangeStaticDesc;
	xui_texture_desc_t arrTilesetArrangeSpecialDesc[MAPEDIT_TILESET_SPECIAL_TILE_MAX];
	xui_texture_desc_t tTilesetPassageStaticDesc;
	xui_texture_desc_t tTilesetActorOverlayStaticDesc;
	xui_texture_desc_t tTilesetTagsStaticDesc;
	xui_texture_desc_t tMapTilesetStaticDesc;
	xui_texture_desc_t arrMapTilesetSpecialDesc[MAPEDIT_TILESET_SPECIAL_TILE_MAX];
	int iTilesetPassageOverlayCellWidth;
	int iTilesetPassageOverlayCellHeight;
	mapedit_tile_source_t tTileSource;
	xge_xui_label_t arrTileSourceInfo[MAPEDIT_TILE_SOURCE_INFO_LABEL_COUNT];
	int* pMapTileData;
	mapedit_map_history_t tMapHistory;
	int iTileSourceInfoCount;
	int iContentCount;
	int iLabelCount;
	int iCustomChannelCount;
	int iSetupLayerCount;
	int bFontReady;
	int bDockReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bLayoutLoaded;
	int bLayoutSaved;
	int bSetupLoaded;
	int iWorkspace;
	int iSetupTileWidth;
	int iSetupTileHeight;
	int iSetupTilesPerRow;
	int iSetupStateMin;
	int iSetupStateMax;
	char sTilesetTagValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	char sMapTagValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	xui_texture pTilesetEditorIcon;
	xui_texture pMapEditorIcon;
	char sMaterialEditTitle[256];
	char sMaterialViewTitle[512];
	char sStatus[256];
} mapedit_app_t;

int MapEditAppInit(mapedit_app_t* pApp);
void MapEditAppUnit(mapedit_app_t* pApp);
int MapEditAppEnter(xge_scene pScene);
int MapEditAppLeave(xge_scene pScene);
int MapEditAppEvent(xge_scene pScene, const xge_event_t* pEvent);
int MapEditAppUpdate(xge_scene pScene, float fDelta);
int MapEditAppDraw(xge_scene pScene);
void MapEditAppSetStatus(mapedit_app_t* pApp, const char* sText);
void MapEditAppMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser);
void MapEditAppWorkspaceClick(xge_xui_widget pWidget, void* pUser);

#endif
