#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 860
#define DEMO_TARGET_H 470
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define LABEL_COUNT 5

typedef struct xui_propertygrid_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pStatus;
	xui_widget pGrid;
	xui_widget pTableGrid;
	xui_widget pTableView;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iSelectCount;
	int iValidateCount;
	int iChangeCount;
	int iActionCount;
	int iRenderCount;
	int iRejectCount;
	char sLastId[64];
	char sLastValue[128];
	int pTitle;
	int pVisible;
	int pOpacity;
	int pAccent;
	int pTheme;
	int pMaxPlayers;
	int pSpawnMode;
	int pPhysics;
	int pTimeout;
	int pMaterial;
	int pPreview;
	int pNotes;
	int pReadonly;
	int pDisabled;
	int pHidden;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bEditOK;
	int bValidateOK;
	int bActionOK;
	int bToggleOK;
	int bScrollOK;
	int bRenderOK;
} xui_propertygrid_demo_t;

static const char* g_arrThemeItems[] = {"Default", "Focused", "High contrast"};
static const char* g_arrSpawnItems[] = {"Manual", "Balanced", "Burst"};
static const uint32_t g_arrPalette[] = {
	XUI_COLOR_RGBA(47, 128, 237, 255),
	XUI_COLOR_RGBA(67, 167, 124, 255),
	XUI_COLOR_RGBA(219, 72, 99, 255),
	XUI_COLOR_RGBA(237, 161, 47, 255)
};

static void __xuiPropertyGridUsage(void)
{
	printf("usage: xui_propertygrid [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiPropertyGridParseArgs(xui_propertygrid_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiPropertyGridUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiPropertyGridFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiPropertyGridRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;
	xui_rect_t tInfo;

	(void)iStateId;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 22.0f, tRect.fW - 48.0f, tRect.fH - 44.0f};
		tInfo = (xui_rect_t){540.0f, 84.0f, 272.0f, 224.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tInfo, XUI_COLOR_RGBA(240, 247, 253, 255));
	}
	return XUI_OK;
}

static void __xuiPropertyGridUpdateStatus(xui_propertygrid_demo_t* pDemo)
{
	char sText[256];

	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	snprintf(sText, sizeof(sText), "selected=%s selects=%d validates=%d changes=%d actions=%d rejects=%d",
		pDemo->sLastId[0] != '\0' ? pDemo->sLastId : "none",
		pDemo->iSelectCount,
		pDemo->iValidateCount,
		pDemo->iChangeCount,
		pDemo->iActionCount,
		pDemo->iRejectCount);
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiPropertyGridStrictIntInRange(const char* sValue, int iMin, int iMax)
{
	const char* p;
	int iValue;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) return 0;
	p = sValue;
	while ( *p != '\0' ) {
		if ( (*p < '0') || (*p > '9') ) return 0;
		p++;
	}
	iValue = atoi(sValue);
	return (iValue >= iMin) && (iValue <= iMax);
}

static int __xuiPropertyGridFloatInRange(const char* sValue, float fMin, float fMax)
{
	float fValue;
	char* pEnd;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) return 0;
	fValue = (float)strtod(sValue, &pEnd);
	return (pEnd != NULL) && (*pEnd == '\0') && (fValue >= fMin) && (fValue <= fMax);
}

static void __xuiPropertyGridSelect(xui_widget pWidget, int iProperty, const char* sId, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;

	(void)pWidget;
	(void)iProperty;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	snprintf(pDemo->sLastId, sizeof(pDemo->sLastId), "%s", sId != NULL ? sId : "");
	__xuiPropertyGridUpdateStatus(pDemo);
}

static int __xuiPropertyGridValidate(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;
	int bValid;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iValidateCount++;
	}
	bValid = 1;
	if ( (sId != NULL) && (strcmp(sId, "maxPlayers") == 0) ) {
		bValid = __xuiPropertyGridStrictIntInRange(sValue, 1, 64);
	} else if ( (sId != NULL) && (strcmp(sId, "opacity") == 0) ) {
		bValid = __xuiPropertyGridFloatInRange(sValue, 0.0f, 1.0f);
	}
	if ( !bValid && pDemo != NULL ) {
		pDemo->iRejectCount++;
	}
	if ( pDemo != NULL ) {
		__xuiPropertyGridUpdateStatus(pDemo);
	}
	return bValid;
}

static void __xuiPropertyGridChange(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	snprintf(pDemo->sLastId, sizeof(pDemo->sLastId), "%s", sId != NULL ? sId : "");
	snprintf(pDemo->sLastValue, sizeof(pDemo->sLastValue), "%s", sValue != NULL ? sValue : "");
	__xuiPropertyGridUpdateStatus(pDemo);
}

static int __xuiPropertyGridAction(xui_widget pWidget, int iProperty, const char* sId, xui_rect_t tWorldRect, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;

	(void)tWorldRect;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iActionCount++;
		snprintf(pDemo->sLastId, sizeof(pDemo->sLastId), "%s", sId != NULL ? sId : "");
	}
	if ( (sId != NULL) && (strcmp(sId, "preview") == 0) ) {
		(void)xuiPropertyGridSetValue(pWidget, iProperty, "assets/preview.png");
	} else {
		(void)xuiPropertyGridSetValue(pWidget, iProperty, "assets/player.material");
	}
	if ( pDemo != NULL ) {
		__xuiPropertyGridUpdateStatus(pDemo);
	}
	return 1;
}

static int __xuiPropertyGridRender(xui_widget pWidget, int iProperty, int iColumn, const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	xui_propertygrid_demo_t* pDemo;

	(void)pWidget;
	(void)iProperty;
	(void)iColumn;
	(void)pCell;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iRenderCount++;
	}
	return 0;
}

static int __xuiPropertyGridAddLabel(xui_propertygrid_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiPropertyGridAddProperty(xui_propertygrid_demo_t* pDemo, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefaultValue, int iFlags)
{
	xui_property_desc_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.sId = sId;
	tProp.sName = sName;
	tProp.sDescription = sDescription;
	tProp.iType = iType;
	tProp.sValue = sValue;
	tProp.sDefaultValue = sDefaultValue;
	tProp.iFlags = iFlags;
	return xuiPropertyGridAddProperty(pDemo->pGrid, iCategory, &tProp);
}

static int __xuiPropertyGridConfigureEditors(xui_propertygrid_demo_t* pDemo)
{
	xui_table_grid_editor_config_t tConfig;
	int iRet;

	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = g_arrThemeItems;
	tConfig.iEnumItemCount = 3;
	tConfig.iEnumSelected = 1;
	iRet = xuiPropertyGridSetEditorConfig(pDemo->pGrid, pDemo->pTheme, &tConfig);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = g_arrSpawnItems;
	tConfig.iEnumItemCount = 3;
	tConfig.iEnumSelected = 1;
	iRet = xuiPropertyGridSetEditorConfig(pDemo->pGrid, pDemo->pSpawnMode, &tConfig);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fMin = 1.0f;
	tConfig.fMax = 64.0f;
	tConfig.fStep = 1.0f;
	tConfig.iPrecision = 0;
	iRet = xuiPropertyGridSetEditorConfig(pDemo->pGrid, pDemo->pMaxPlayers, &tConfig);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fMin = 0.0f;
	tConfig.fMax = 1.0f;
	tConfig.fStep = 0.05f;
	tConfig.iPrecision = 2;
	iRet = xuiPropertyGridSetEditorConfig(pDemo->pGrid, pDemo->pOpacity, &tConfig);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrPalette = g_arrPalette;
	tConfig.iPaletteCount = (int)(sizeof(g_arrPalette) / sizeof(g_arrPalette[0]));
	iRet = xuiPropertyGridSetEditorConfig(pDemo->pGrid, pDemo->pAccent, &tConfig);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPropertyGridSetAction(pDemo->pGrid, pDemo->pMaterial, __xuiPropertyGridAction, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPropertyGridSetAction(pDemo->pGrid, pDemo->pPreview, __xuiPropertyGridAction, pDemo);
	return iRet;
}

static int __xuiPropertyGridCreateGrid(xui_propertygrid_demo_t* pDemo)
{
	xui_property_grid_desc_t tDesc;
	xui_property_grid_style_t tStyle;
	int iAppearance;
	int iBehavior;
	int iAssets;
	int iAdvanced;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.fNameWidth = 156.0f;
	tDesc.fRowHeight = 26.0f;
	tDesc.fCategoryHeight = 28.0f;
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_BOTH;
	tDesc.fDescriptionPanelHeight = 56.0f;
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	iRet = xuiPropertyGridCreate(pDemo->pContext, &pDemo->pGrid, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pGrid, (xui_rect_t){48.0f, 84.0f, 456.0f, 300.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pGrid);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pDemo->pGrid);
		pDemo->pGrid = NULL;
		return iRet;
	}
	pDemo->pTableGrid = xuiPropertyGridGetTableGrid(pDemo->pGrid);
	pDemo->pTableView = xuiPropertyGridGetTableView(pDemo->pGrid);
	(void)xuiPropertyGridSetSelect(pDemo->pGrid, __xuiPropertyGridSelect, pDemo);
	(void)xuiPropertyGridSetValidate(pDemo->pGrid, __xuiPropertyGridValidate, pDemo);
	(void)xuiPropertyGridSetChange(pDemo->pGrid, __xuiPropertyGridChange, pDemo);
	(void)xuiPropertyGridSetGlobalRenderer(pDemo->pGrid, __xuiPropertyGridRender, pDemo);
	if ( xuiPropertyGridGetStyle(pDemo->pGrid, &tStyle) == XUI_OK ) {
		tStyle.iSelectedColor = XUI_COLOR_RGBA(216, 235, 250, 255);
		tStyle.iDirtyColor = XUI_COLOR_RGBA(238, 161, 47, 255);
		tStyle.iInvalidColor = XUI_COLOR_RGBA(218, 77, 99, 255);
		(void)xuiPropertyGridSetStyle(pDemo->pGrid, &tStyle);
	}
	iAppearance = xuiPropertyGridAddCategory(pDemo->pGrid, "appearance", "Appearance", 1);
	iBehavior = xuiPropertyGridAddCategory(pDemo->pGrid, "behavior", "Behavior", 1);
	iAssets = xuiPropertyGridAddCategory(pDemo->pGrid, "assets", "Assets", 1);
	iAdvanced = xuiPropertyGridAddCategory(pDemo->pGrid, "advanced", "Advanced", 1);
	if ( iAppearance < 0 || iBehavior < 0 || iAssets < 0 || iAdvanced < 0 ) return XUI_ERROR;
	pDemo->pTitle = __xuiPropertyGridAddProperty(pDemo, iAppearance, "title", "Title", "Displayed name in the inspector title area.", XUI_TABLE_CELL_TYPE_TEXT, "Player Controller", "Player Controller", 0);
	pDemo->pVisible = __xuiPropertyGridAddProperty(pDemo, iAppearance, "visible", "Visible", "Controls whether this object is drawn.", XUI_TABLE_CELL_TYPE_BOOL, "true", "true", 0);
	pDemo->pOpacity = __xuiPropertyGridAddProperty(pDemo, iAppearance, "opacity", "Opacity", "Alpha multiplier from 0 to 1.", XUI_TABLE_CELL_TYPE_FLOAT, "0.85", "1.0", 0);
	pDemo->pAccent = __xuiPropertyGridAddProperty(pDemo, iAppearance, "accent", "Accent", "Preview color used by child tools.", XUI_TABLE_CELL_TYPE_COLOR, "#2F80ED", "#2F80ED", 0);
	pDemo->pTheme = __xuiPropertyGridAddProperty(pDemo, iAppearance, "theme", "Theme", "Visual theme override for this node.", XUI_TABLE_CELL_TYPE_ENUM, "Default", "Default", 0);
	pDemo->pMaxPlayers = __xuiPropertyGridAddProperty(pDemo, iBehavior, "maxPlayers", "Max players", "Integer validation accepts values from 1 to 64.", XUI_TABLE_CELL_TYPE_INT, "8", "4", 0);
	pDemo->pSpawnMode = __xuiPropertyGridAddProperty(pDemo, iBehavior, "spawnMode", "Spawn mode", "Enum editor supplied by per-property config.", XUI_TABLE_CELL_TYPE_ENUM, "Manual", "Manual", 0);
	pDemo->pPhysics = __xuiPropertyGridAddProperty(pDemo, iBehavior, "lockPhysics", "Lock physics", "Boolean values toggle without keeping an inline editor.", XUI_TABLE_CELL_TYPE_BOOL, "false", "false", 0);
	pDemo->pTimeout = __xuiPropertyGridAddProperty(pDemo, iBehavior, "timeout", "Timeout", "Floating point editor using numeric constraints.", XUI_TABLE_CELL_TYPE_FLOAT, "1.25", "1.25", 0);
	pDemo->pMaterial = __xuiPropertyGridAddProperty(pDemo, iAssets, "material", "Material", "External picker callback fills this path.", XUI_TABLE_CELL_TYPE_PICKER, "...", "", 0);
	pDemo->pPreview = __xuiPropertyGridAddProperty(pDemo, iAssets, "preview", "Preview image", "Image picker callback follows the same external editor contract.", XUI_TABLE_CELL_TYPE_IMAGE, "...", "", 0);
	pDemo->pNotes = __xuiPropertyGridAddProperty(pDemo, iAssets, "notes", "Notes", "Textarea values use the TableGrid popup text editor.", XUI_TABLE_CELL_TYPE_TEXTAREA, "Editable memo", "Editable memo", 0);
	pDemo->pReadonly = __xuiPropertyGridAddProperty(pDemo, iAdvanced, "readonly", "Readonly", "Readonly values still render but reject editing.", XUI_TABLE_CELL_TYPE_TEXT, "locked", "locked", XUI_PROPERTY_FLAG_READONLY);
	pDemo->pDisabled = __xuiPropertyGridAddProperty(pDemo, iAdvanced, "disabled", "Disabled", "Disabled values use disabled visual state.", XUI_TABLE_CELL_TYPE_TEXT, "offline", "offline", XUI_PROPERTY_FLAG_DISABLED);
	pDemo->pHidden = __xuiPropertyGridAddProperty(pDemo, iAdvanced, "hidden", "Hidden", "Hidden properties remain in the model but not in visible rows.", XUI_TABLE_CELL_TYPE_TEXT, "secret", "secret", XUI_PROPERTY_FLAG_HIDDEN);
	if ( pDemo->pTitle < 0 || pDemo->pVisible < 0 || pDemo->pOpacity < 0 || pDemo->pAccent < 0 || pDemo->pTheme < 0 ||
	     pDemo->pMaxPlayers < 0 || pDemo->pSpawnMode < 0 || pDemo->pPhysics < 0 || pDemo->pTimeout < 0 ||
	     pDemo->pMaterial < 0 || pDemo->pPreview < 0 || pDemo->pNotes < 0 || pDemo->pReadonly < 0 || pDemo->pDisabled < 0 || pDemo->pHidden < 0 ) {
		return XUI_ERROR;
	}
	return __xuiPropertyGridConfigureEditors(pDemo);
}

static int __xuiPropertyGridCreateUi(xui_propertygrid_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiPropertyGridRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiPropertyGridAddLabel(pDemo, 0, "XUI PropertyGrid", (xui_rect_t){48.0f, 30.0f, 180.0f, 24.0f}) != XUI_OK ||
	     __xuiPropertyGridAddLabel(pDemo, 1, "Scene object inspector", (xui_rect_t){48.0f, 58.0f, 260.0f, 22.0f}) != XUI_OK ||
	     __xuiPropertyGridAddLabel(pDemo, 2, "Selection and edit state", (xui_rect_t){560.0f, 104.0f, 220.0f, 24.0f}) != XUI_OK ||
	     __xuiPropertyGridAddLabel(pDemo, 3, "selected=none selects=0 validates=0 changes=0 actions=0 rejects=0", (xui_rect_t){560.0f, 138.0f, 230.0f, 84.0f}) != XUI_OK ||
	     __xuiPropertyGridAddLabel(pDemo, 4, "Runtime object: Player Controller   Tags: gameplay, input, camera", (xui_rect_t){48.0f, 404.0f, 720.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[3];
	return __xuiPropertyGridCreateGrid(pDemo);
}

static uint32_t __xuiPropertyGridReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiPropertyGridXgeKeyToXui(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_SPACE: return XUI_KEY_SPACE;
	case XGE_KEY_BACKSPACE: return 8;
	case XGE_KEY_DELETE: return 46;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	default: return 0;
	}
}

static int __xuiPropertyGridHandleKeys(xui_propertygrid_demo_t* pDemo)
{
	static const int arrKeys[] = {
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_SPACE,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_ESCAPE
	};
	uint32_t iText;
	int i;
	int iKey;
	int iRet;

	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		if ( xgeKeyPressed(arrKeys[i]) ) {
			if ( arrKeys[i] == XGE_KEY_ESCAPE && !xuiPropertyGridIsEditing(pDemo->pGrid) ) {
				xgeQuit();
				continue;
			}
			iKey = __xuiPropertyGridXgeKeyToXui(arrKeys[i]);
			if ( iKey != 0 ) {
				iRet = xuiInputKeyDown(pDemo->pContext, iKey, 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iKey = __xuiPropertyGridXgeKeyToXui(arrKeys[i]);
			if ( iKey != 0 ) {
				iRet = xuiInputKeyUp(pDemo->pContext, iKey, 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		iRet = xuiInputText(pDemo->pContext, iText);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiPropertyGridHandleInput(xui_propertygrid_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiPropertyGridReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiPropertyGridHandleKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiPropertyGridTypeText(xui_propertygrid_demo_t* pDemo, const char* sText)
{
	int iRet;

	if ( sText == NULL ) return XUI_OK;
	while ( *sText != '\0' ) {
		iRet = xuiInputText(pDemo->pContext, (uint32_t)(unsigned char)*sText++);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pDemo->pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiPropertyGridClickVisibleCell(xui_propertygrid_demo_t* pDemo, int iRow, int iColumn)
{
	xui_rect_t tWorld;
	xui_rect_t tCell;
	float fX;
	float fY;
	int iRet;

	if ( xuiTableViewGetCellRect(pDemo->pTableView, iRow, iColumn, &tCell) != XUI_OK ) return XUI_ERROR;
	tWorld = xuiWidgetGetWorldRect(pDemo->pTableView);
	fX = tWorld.fX + tCell.fX + 10.0f;
	fY = tWorld.fY + tCell.fY + tCell.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiPropertyGridRunChecks(xui_propertygrid_demo_t* pDemo, int bExerciseInput)
{
	float fScroll;
	int iVisibleBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pGrid != NULL) && (pDemo->pTableGrid != NULL) && (pDemo->pTableView != NULL);
	pDemo->bLayoutOK = pDemo->bCreateOK &&
		(xuiPropertyGridGetCategoryCount(pDemo->pGrid) == 4) &&
		(xuiPropertyGridGetPropertyCount(pDemo->pGrid) == 15) &&
		(xuiTableViewGetColumnCount(pDemo->pTableView) == 2) &&
		(xuiPropertyGridFindProperty(pDemo->pGrid, "hidden") == pDemo->pHidden) &&
		(xuiPropertyGridGetVisibleProperty(pDemo->pGrid, xuiPropertyGridGetVisibleCount(pDemo->pGrid) - 1) != pDemo->pHidden);
	pDemo->bRenderOK = pDemo->iRenderCount > 0;
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pTitle);
		(void)__xuiPropertyGridTypeText(pDemo, "Runtime Node");
		(void)xuiPropertyGridEndEdit(pDemo->pGrid, 1);
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pMaxPlayers);
		(void)__xuiPropertyGridTypeText(pDemo, "bad");
		pDemo->bValidateOK = (xuiPropertyGridEndEdit(pDemo->pGrid, 1) == 0) &&
			(xuiTableGridGetRejectCount(pDemo->pTableGrid) > 0) &&
			(pDemo->iRejectCount > 0);
		(void)xuiPropertyGridEndEdit(pDemo->pGrid, 0);
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pMaxPlayers);
		(void)__xuiPropertyGridTypeText(pDemo, "32");
		(void)xuiPropertyGridEndEdit(pDemo->pGrid, 1);
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pVisible);
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pTheme);
		(void)xuiPropertyGridEndEdit(pDemo->pGrid, 1);
		(void)xuiPropertyGridBeginEdit(pDemo->pGrid, pDemo->pMaterial);
		iVisibleBefore = xuiPropertyGridGetVisibleCount(pDemo->pGrid);
		(void)xuiPropertyGridSetCategoryExpanded(pDemo->pGrid, 0, 0);
		pDemo->bToggleOK = (xuiPropertyGridGetToggleCount(pDemo->pGrid) > 0) &&
			(xuiPropertyGridGetVisibleCount(pDemo->pGrid) < iVisibleBefore);
		(void)xuiPropertyGridSetCategoryExpanded(pDemo->pGrid, 0, 1);
		(void)xuiTableViewEnsureCellVisible(pDemo->pTableView, xuiPropertyGridGetVisibleCount(pDemo->pGrid) - 1, 1);
		fScroll = xuiPropertyGridGetScroll(pDemo->pGrid);
		pDemo->bScrollOK = fScroll > 0.0f;
		pDemo->bEditOK = (strcmp(xuiPropertyGridGetValue(pDemo->pGrid, pDemo->pTitle), "Runtime Node") == 0) &&
			(xuiPropertyGridGetInt(pDemo->pGrid, pDemo->pMaxPlayers, 0) == 32) &&
			(xuiPropertyGridGetBool(pDemo->pGrid, pDemo->pVisible, 1) == 0) &&
			(strcmp(xuiPropertyGridGetValue(pDemo->pGrid, pDemo->pTheme), "Focused") == 0) &&
			(xuiTableGridGetCommitCount(pDemo->pTableGrid) >= 3);
		pDemo->bActionOK = (pDemo->iActionCount > 0) &&
			(strcmp(xuiPropertyGridGetValue(pDemo->pGrid, pDemo->pMaterial), "assets/player.material") == 0);
		(void)xuiPropertyGridSetSelected(pDemo->pGrid, pDemo->pAccent);
		(void)xuiPropertyGridSetColor(pDemo->pGrid, pDemo->pAccent, XUI_COLOR_RGBA(67, 167, 124, 255));
		(void)xuiPropertyGridSetPropertyFlags(pDemo->pGrid, pDemo->pOpacity, XUI_PROPERTY_FLAG_DIRTY);
		pDemo->bExerciseDone = 1;
		__xuiPropertyGridUpdateStatus(pDemo);
	} else if ( !bExerciseInput ) {
		pDemo->bEditOK = 1;
		pDemo->bValidateOK = 1;
		pDemo->bActionOK = 1;
		pDemo->bToggleOK = 1;
		pDemo->bScrollOK = 1;
	}
}

static int __xuiPropertyGridCreateAssets(xui_propertygrid_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiPropertyGridFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiPropertyGridCreateUi(pDemo);
}

static void __xuiPropertyGridDestroyAssets(xui_propertygrid_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiPropertyGridFrame(void* pUser)
{
	xui_propertygrid_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_propertygrid_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiPropertyGridHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiPropertyGridRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_propertygrid final-summary frames=%d create=%d layout=%d edit=%d validate=%d action=%d toggle=%d scroll=%d render=%d selects=%d changes=%d rejects=%d commits=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bEditOK,
			pDemo->bValidateOK,
			pDemo->bActionOK,
			pDemo->bToggleOK,
			pDemo->bScrollOK,
			pDemo->bRenderOK,
			pDemo->iSelectCount,
			pDemo->iChangeCount,
			xuiTableGridGetRejectCount(pDemo->pTableGrid),
			xuiTableGridGetCommitCount(pDemo->pTableGrid),
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_propertygrid_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiPropertyGridParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiPropertyGridUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI PropertyGrid";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_propertygrid: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiPropertyGridCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_propertygrid: create assets failed: %d\n", iRet);
		__xuiPropertyGridDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiPropertyGridFrame, &tDemo);
	__xuiPropertyGridDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bEditOK && tDemo.bValidateOK &&
		tDemo.bActionOK && tDemo.bToggleOK && tDemo.bScrollOK && tDemo.bRenderOK) ? 0 : 1;
}
