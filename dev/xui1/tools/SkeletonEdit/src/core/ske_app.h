#ifndef SKE_APP_H
#define SKE_APP_H

#include "xge.h"

#define SKE_DOCK_WINDOW_COUNT 3
#define SKE_LABEL_CAPACITY 64
#define SKE_BONE_CAPACITY 64
#define SKE_SLOT_CAPACITY 64
#define SKE_ATTACHMENT_CAPACITY 64
#define SKE_ANIMATION_CAPACITY 32
#define SKE_KEYFRAME_CAPACITY 256
#define SKE_NAME_CAPACITY 128
#define SKE_HISTORY_CAPACITY 32
#define SKE_IMAGE_CAPACITY 64
#define SKE_IMAGE_NODE_BASE 1000

enum {
	SKE_DOCK_CENTER = 0,
	SKE_DOCK_ASSETS,
	SKE_DOCK_PROPERTIES
};

enum {
	SKE_MODE_SELECT = 0,
	SKE_MODE_BONE,
	SKE_MODE_SLOT,
	SKE_MODE_ATTACHMENT,
	SKE_MODE_KEY,
	SKE_MODE_PLAY
};

enum {
	SKE_CMD_NEW = 100,
	SKE_CMD_OPEN,
	SKE_CMD_SAVE,
	SKE_CMD_SAVE_AS,
	SKE_CMD_EXIT,
	SKE_CMD_UNDO,
	SKE_CMD_REDO,
	SKE_CMD_DELETE,
	SKE_CMD_RESET_LAYOUT,
	SKE_CMD_FRAME_ALL,
	SKE_CMD_ADD_BONE,
	SKE_CMD_ADD_SLOT,
	SKE_CMD_ADD_ATTACHMENT,
	SKE_CMD_IMPORT_IMAGE,
	SKE_CMD_ADD_ANIMATION,
	SKE_CMD_ADD_KEY,
	SKE_CMD_PLAY,
	SKE_CMD_STOP,
	SKE_CMD_ABOUT,
	SKE_CMD_MODE_SELECT,
	SKE_CMD_MODE_BONE,
	SKE_CMD_MODE_SLOT,
	SKE_CMD_MODE_ATTACHMENT,
	SKE_CMD_MODE_KEY
};

enum {
	SKE_SELECTION_NONE = 0,
	SKE_SELECTION_BONE,
	SKE_SELECTION_SLOT,
	SKE_SELECTION_ATTACHMENT,
	SKE_SELECTION_IMAGE,
	SKE_SELECTION_ANIMATION,
	SKE_SELECTION_KEYFRAME
};

typedef struct ske_label_slot_t {
	xge_xui_widget pWidget;
	xge_xui_label_t tLabel;
	int bReady;
} ske_label_slot_t;

typedef struct ske_image_texture_t {
	char sName[SKE_NAME_CAPACITY];
	xge_texture_t tTexture;
	int bLoaded;
	int bFailed;
} ske_image_texture_t;

typedef struct ske_bone_t {
	int iId;
	int iParent;
	char sName[SKE_NAME_CAPACITY];
	float fLocalX;
	float fLocalY;
	float fRotation;
	float fLength;
	int bConnected;
	int bVisible;
} ske_bone_t;

typedef struct ske_slot_t {
	int iId;
	int iBone;
	char sName[SKE_NAME_CAPACITY];
	int iAttachment;
	int iDrawOrder;
	uint32_t iColor;
	int bVisible;
} ske_slot_t;

typedef struct ske_attachment_t {
	int iId;
	int iBone;
	char sName[SKE_NAME_CAPACITY];
	char sImage[SKE_NAME_CAPACITY];
	float fOffsetX;
	float fOffsetY;
	float fRotation;
	float fScaleX;
	float fScaleY;
	float fPivotX;
	float fPivotY;
	int iDrawOrder;
} ske_attachment_t;

typedef struct ske_animation_t {
	int iId;
	char sName[SKE_NAME_CAPACITY];
	int iFrameStart;
	int iFrameEnd;
	int bLoop;
} ske_animation_t;

typedef struct ske_keyframe_t {
	int iId;
	int iAnimation;
	int iBone;
	int iFrame;
	float fLocalX;
	float fLocalY;
	float fRotation;
	float fLength;
} ske_keyframe_t;

typedef struct ske_document_t {
	char sName[SKE_NAME_CAPACITY];
	int iFrameRate;
	int bDirty;
	ske_bone_t arrBones[SKE_BONE_CAPACITY];
	int iBoneCount;
	ske_slot_t arrSlots[SKE_SLOT_CAPACITY];
	int iSlotCount;
	ske_attachment_t arrAttachments[SKE_ATTACHMENT_CAPACITY];
	int iAttachmentCount;
	ske_animation_t arrAnimations[SKE_ANIMATION_CAPACITY];
	int iAnimationCount;
	ske_keyframe_t arrKeyframes[SKE_KEYFRAME_CAPACITY];
	int iKeyframeCount;
	int iSelectedType;
	int iSelectedIndex;
	int iSelectedImageNode;
	int iActiveBoneIndex;
	int iActiveSlotIndex;
	int iActiveAnimationIndex;
	int iNextBoneId;
	int iNextSlotId;
	int iNextAttachmentId;
	int iNextAnimationId;
	int iNextKeyframeId;
} ske_document_t;

typedef struct ske_app_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bSmoke;
	int bRoundTripOK;
	int iMode;
	int iCurrentFrame;
	float fPlaybackAccum;
	int iSelectedTreeNode;
	int bCanvasCreateBone;
	int bCanvasDragBoneEnd;
	int bCanvasDragAttachment;
	int bCanvasDragRoot;
	int bCanvasPan;
	int iDragBoneIndex;
	int iDragAttachmentIndex;
	float fCreateStartX;
	float fCreateStartY;
	float fCreateEndX;
	float fCreateEndY;
	int iCreateParent;
	float fDragStartDocX;
	float fDragStartDocY;
	float fDragStartOffsetX;
	float fDragStartOffsetY;
	float fDragStartRootX;
	float fDragStartRootY;
	float fCanvasPanX;
	float fCanvasPanY;
	float fCanvasZoom;
	float fPanStartMouseX;
	float fPanStartMouseY;
	float fPanStartX;
	float fPanStartY;

	ske_document_t tDocument;
	ske_document_t arrUndo[SKE_HISTORY_CAPACITY];
	ske_document_t arrRedo[SKE_HISTORY_CAPACITY];
	int iUndoCount;
	int iRedoCount;

	xge_xui_widget pRoot;
	xge_xui_widget pMenuBar;
	xge_xui_widget pToolbar;
	xge_xui_widget pDockHost;
	xge_xui_widget pStatusHost;
	xge_xui_widget pAssetPanel;
	xge_xui_widget pAssetTreeWidget;
	xge_xui_widget pPropertyPanel;
	xge_xui_widget pPropertyGridWidget;
	xge_xui_widget pCenterPanel;
	xge_xui_widget pTimelineWidget;
	xge_xui_widget pCanvasWidget;

	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t arrDockWindow[SKE_DOCK_WINDOW_COUNT];
	xge_xui_dock_pane pCenterPane;
	xge_xui_dock_pane pAssetPane;
	xge_xui_dock_pane pPropertyPane;
	int bDockLayoutReady;
	int iDockWindowReadyCount;

	xge_xui_menubar_t tMenuBar;
	xge_xui_menu_t tFileMenu;
	xge_xui_menu_t tEditMenu;
	xge_xui_menu_t tViewMenu;
	xge_xui_menu_t tSkeletonMenu;
	xge_xui_menu_t tAnimationMenu;
	xge_xui_menu_t tHelpMenu;
	xge_xui_toolbar_t tToolbar;
	xge_xui_status_bar_t tStatusBar;
	xge_xui_tree_view_t tAssetTree;
	xge_xui_property_grid_t tPropertyGrid;
	xge_xui_timeline_view_t tTimeline;
	int bMenuReady;
	int bToolbarReady;
	int bStatusReady;
	int bAssetTreeReady;
	int bPropertyGridReady;
	int bPropertyRefreshPending;
	int bPropertyUndoOpen;
	char sPropertyUndoId[SKE_NAME_CAPACITY];
	int bTimelineReady;
	int iStatusMain;
	int iStatusProject;
	int iStatusMode;
	int iStatusSelection;
	int iStatusFrame;

	ske_label_slot_t arrLabels[SKE_LABEL_CAPACITY];
	int iLabelCount;
	char arrImageNames[SKE_IMAGE_CAPACITY][SKE_NAME_CAPACITY];
	int iImageCount;
	ske_image_texture_t arrImageTextures[SKE_IMAGE_CAPACITY];
	int iImageTextureCount;
	char sDocumentPath[512];
	char sProjectDir[512];
} ske_app_t;

int SkeletonEditAppInit(ske_app_t* pApp);
void SkeletonEditAppUnit(ske_app_t* pApp);
int SkeletonEditAppEnter(xge_scene pScene);
int SkeletonEditAppLeave(xge_scene pScene);
int SkeletonEditAppEvent(xge_scene pScene, const xge_event_t* pEvent);
int SkeletonEditAppUpdate(xge_scene pScene, float fDelta);
int SkeletonEditAppDraw(xge_scene pScene);
void SkeletonEditDocumentResetDefault(ske_document_t* pDoc);
int SkeletonEditDocumentSave(const ske_document_t* pDoc, const char* sPath);
int SkeletonEditDocumentLoad(ske_document_t* pDoc, const char* sPath);
int SkeletonEditDocumentEquals(const ske_document_t* pA, const ske_document_t* pB);

int SkeletonEditShellBuild(ske_app_t* pApp);
void SkeletonEditShellUnit(ske_app_t* pApp);
int SkeletonEditCommandExecute(ske_app_t* pApp, int iCommand);
int SkeletonEditCanvasHandleEvent(ske_app_t* pApp, const xge_event_t* pEvent);
void SkeletonEditPropertiesRequestRefresh(ske_app_t* pApp);
void SkeletonEditPropertiesFlushRefresh(ske_app_t* pApp);
void SkeletonEditStatusRefresh(ske_app_t* pApp);
void SkeletonEditStatusSet(ske_app_t* pApp, const char* sText);

#endif
