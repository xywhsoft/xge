#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_WIDTH 780
#define DEMO_HEIGHT 500
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_PICKER_COUNT 3
#define DEMO_LABEL_COUNT 7
#define DEMO_ICON_COUNT 24

typedef struct xui_iconpicker_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabels[DEMO_LABEL_COUNT];
	xui_widget pPickers[DEMO_PICKER_COUNT];
	xui_widget pStatus;
	xui_icon_category pCategory;
	xui_icon_id arrIds[DEMO_ICON_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int bCreateOK;
	int bGeometryOK;
	int bScrollOK;
	int bSelectionOK;
	int bExerciseDone;
} xui_iconpicker_demo_t;

static const char* g_arrIconPaths[] = {
	"M12 2 L15 9 L22 9 L17 14 L19 22 L12 17 L5 22 L7 14 L2 9 L9 9 Z",
	"M3 12 L12 4 L21 12 V21 H15 V15 H9 V21 H3 Z",
	"M4 4 H20 V20 H4 Z M8 8 H16 V16 H8 Z",
	"M12 2 C6 2 2 6 2 12 C2 18 6 22 12 22 C18 22 22 18 22 12 C22 6 18 2 12 2 Z",
	"M4 6 H20 V18 H4 Z M7 9 H17 M7 13 H17",
	"M5 3 H15 L20 8 V21 H5 Z M15 3 V8 H20",
	"M4 12 L9 17 L20 6 L22 8 L9 21 L2 14 Z",
	"M12 2 L22 12 L12 22 L2 12 Z",
	"M3 5 H21 V19 H3 Z M3 9 H21",
	"M12 3 L21 20 H3 Z"
};

static void __xuiIconPickerUsage(void)
{
	printf("usage: xui_iconpicker [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiIconPickerParseArgs(xui_iconpicker_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiIconPickerUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( pDemo->iMaxFrames < 0 || pDemo->fMaxSeconds < 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
	return XGE_OK;
}

static const char* __xuiIconPickerFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc"
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

static int __xuiIconPickerRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_iconpicker_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_iconpicker_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
		tPanel = (xui_rect_t){24.0f, 22.0f, tRect.fW - 48.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(250, 252, 255, 255));
	}
	return XUI_OK;
}

static int __xuiIconPickerAddLabel(
	xui_iconpicker_demo_t* pDemo,
	int iIndex,
	const char* sText,
	xui_rect_t tRect,
	uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( iIndex < 0 || iIndex >= DEMO_LABEL_COUNT ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabels[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiIconPickerCreateIcons(xui_iconpicker_demo_t* pDemo)
{
	static const uint32_t arrColors[] = {
		XUI_COLOR_RGBA(49, 132, 224, 255),
		XUI_COLOR_RGBA(36, 167, 123, 255),
		XUI_COLOR_RGBA(229, 156, 48, 255),
		XUI_COLOR_RGBA(218, 83, 111, 255),
		XUI_COLOR_RGBA(125, 91, 201, 255),
		XUI_COLOR_RGBA(44, 164, 188, 255)
	};
	xui_icon_category_desc_t tCategoryDesc;
	xui_icon_desc_t tIconDesc;
	xui_path_style_t tPathStyle;
	xui_icon pIcon;
	char sName[32];
	char sDisplayName[48];
	int iRet;
	int i;

	xuiIconCategoryDescDefault(&tCategoryDesc);
	tCategoryDesc.iSizeMode = XUI_ICON_SIZE_UNRESTRICTED;
	tCategoryDesc.fWidth = 0.0f;
	tCategoryDesc.fHeight = 0.0f;
	iRet = xuiIconCategoryCreate(pDemo->pContext, "iconpicker_demo", &tCategoryDesc, &pDemo->pCategory);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tPathStyle, 0, sizeof(tPathStyle));
	tPathStyle.iSize = sizeof(tPathStyle);
	tPathStyle.iFillRule = XUI_PATH_FILL_NON_ZERO;
	tPathStyle.iLineJoin = XUI_PATH_JOIN_ROUND;
	tPathStyle.iLineCap = XUI_PATH_CAP_ROUND;
	xuiIconDescDefault(&tIconDesc);
	iRet = xuiIconCategoryBeginUpdate(pDemo->pCategory);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < DEMO_ICON_COUNT; i++ ) {
		snprintf(sName, sizeof(sName), "tool_%02d", i + 1);
		snprintf(sDisplayName, sizeof(sDisplayName), "Tool Icon %02d", i + 1);
		tIconDesc.sDisplayName = sDisplayName;
		tPathStyle.iFillColor = arrColors[i % (int)(sizeof(arrColors) / sizeof(arrColors[0]))];
		tPathStyle.iStrokeColor = tPathStyle.iFillColor;
		tPathStyle.fStrokeWidth = 0.0f;
		iRet = xuiIconAddSvgPath(
			pDemo->pCategory,
			sName,
			g_arrIconPaths[i % (int)(sizeof(g_arrIconPaths) / sizeof(g_arrIconPaths[0]))],
			(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
			&tPathStyle,
			&tIconDesc,
			&pIcon);
		if ( iRet != XUI_OK ) break;
		pDemo->arrIds[i] = xuiIconGetId(pIcon);
	}
	if ( xuiIconCategoryEndUpdate(pDemo->pCategory) != XUI_OK && iRet == XUI_OK ) iRet = XUI_ERROR;
	return iRet;
}

static void __xuiIconPickerChanged(
	xui_widget pWidget,
	xui_icon_id iOldId,
	xui_icon_id iNewId,
	void* pUser)
{
	xui_iconpicker_demo_t* pDemo;
	xui_icon pIcon;
	char sText[160];

	pDemo = (xui_iconpicker_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	pIcon = xuiIconPickerGetSelectedIcon(pWidget);
	snprintf(
		sText,
		sizeof(sText),
		"changes=%d  old=%u  new=%u  name=%s",
		pDemo->iChangeCount,
		(unsigned)iOldId,
		(unsigned)iNewId,
		pIcon != NULL ? xuiIconGetName(pIcon) : "none");
	if ( pDemo->pStatus != NULL ) (void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiIconPickerCreatePickers(xui_iconpicker_demo_t* pDemo)
{
	xui_icon_picker_desc_t tDesc;
	int iRet;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pCategory = pDemo->pCategory;
	tDesc.iSelectedId = pDemo->arrIds[1];
	tDesc.iTextMode = XUI_ICON_PICKER_TEXT_ID;
	tDesc.iVisibleRows = 3;
	tDesc.iVisibleColumns = 6;
	tDesc.fCellWidth = 48.0f;
	tDesc.fCellHeight = 48.0f;
	tDesc.fGapX = 4.0f;
	tDesc.fGapY = 4.0f;
	tDesc.fIconPadding = 9.0f;
	tDesc.sPlaceholder = "Choose an icon";
	tDesc.pFont = pDemo->pFont;
	iRet = xuiIconPickerCreate(pDemo->pContext, &pDemo->pPickers[0], &tDesc);
	if ( iRet != XUI_OK ) return iRet;

	tDesc.iSelectedId = pDemo->arrIds[5];
	tDesc.iTextMode = XUI_ICON_PICKER_TEXT_DISPLAY_NAME;
	tDesc.iVisibleRows = 2;
	tDesc.iVisibleColumns = 5;
	tDesc.fCellWidth = 52.0f;
	tDesc.fCellHeight = 48.0f;
	iRet = xuiIconPickerCreate(pDemo->pContext, &pDemo->pPickers[1], &tDesc);
	if ( iRet != XUI_OK ) return iRet;

	tDesc.iSelectedId = pDemo->arrIds[9];
	tDesc.iTextMode = XUI_ICON_PICKER_TEXT_NONE;
	tDesc.iVisibleRows = 2;
	tDesc.iVisibleColumns = 4;
	iRet = xuiIconPickerCreate(pDemo->pContext, &pDemo->pPickers[2], &tDesc);
	if ( iRet != XUI_OK ) return iRet;

	xuiWidgetSetRect(pDemo->pPickers[0], (xui_rect_t){214.0f, 92.0f, 250.0f, 36.0f});
	xuiWidgetSetRect(pDemo->pPickers[1], (xui_rect_t){214.0f, 150.0f, 250.0f, 36.0f});
	xuiWidgetSetRect(pDemo->pPickers[2], (xui_rect_t){214.0f, 208.0f, 84.0f, 42.0f});
	for ( i = 0; i < DEMO_PICKER_COUNT; i++ ) {
		(void)xuiIconPickerSetChange(pDemo->pPickers[i], __xuiIconPickerChanged, pDemo);
		iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPickers[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiIconPickerCreateUi(xui_iconpicker_demo_t* pDemo)
{
	uint32_t iTextColor;
	uint32_t iMutedColor;
	int iRet;

	iTextColor = XUI_COLOR_RGBA(50, 65, 84, 255);
	iMutedColor = XUI_COLOR_RGBA(101, 118, 140, 255);
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_WIDTH, (float)DEMO_HEIGHT});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiIconPickerRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiIconPickerAddLabel(pDemo, 0, "XUI IconPicker", (xui_rect_t){48.0f, 34.0f, 260.0f, 28.0f}, iTextColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 1, "Icon + stable ID", (xui_rect_t){48.0f, 92.0f, 150.0f, 36.0f}, iTextColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 2, "Icon + display name", (xui_rect_t){48.0f, 150.0f, 150.0f, 36.0f}, iTextColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 3, "Icon only", (xui_rect_t){48.0f, 208.0f, 150.0f, 42.0f}, iTextColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 4, "The popup grid keeps its configured column width when the vertical scrollbar appears.", (xui_rect_t){48.0f, 302.0f, 670.0f, 28.0f}, iMutedColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 5, "Keyboard: arrows, Home/End, Page Up/Page Down, Enter and Escape.", (xui_rect_t){48.0f, 334.0f, 670.0f, 28.0f}, iMutedColor) != XUI_OK ||
	     __xuiIconPickerAddLabel(pDemo, 6, "changes=0", (xui_rect_t){48.0f, 392.0f, 670.0f, 30.0f}, XUI_COLOR_RGBA(39, 117, 194, 255)) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabels[6];
	return __xuiIconPickerCreatePickers(pDemo);
}

static void __xuiIconPickerRunChecks(xui_iconpicker_demo_t* pDemo, int bExercise)
{
	xui_widget pPopup;
	xui_widget pFrame;
	xui_rect_t tPopup;
	xui_rect_t tViewport;
	float fOffsetY;

	pDemo->bCreateOK = pDemo->pRoot != NULL && pDemo->pCategory != NULL &&
		pDemo->pPickers[0] != NULL && xuiIconPickerGetPopupWidget(pDemo->pPickers[0]) != NULL;
	if ( !bExercise || pDemo->bExerciseDone ) return;
	(void)xuiIconPickerOpen(pDemo->pPickers[0]);
	(void)xuiLayout(pDemo->pContext);
	pPopup = xuiIconPickerGetPopupWidget(pDemo->pPickers[0]);
	pFrame = xuiIconPickerGetFrameWidget(pDemo->pPickers[0]);
	tPopup = xuiPopupGetPopupRect(pPopup);
	tViewport = xuiScrollFrameGetViewportRect(pFrame);
	pDemo->bGeometryOK = tPopup.fW > tViewport.fW && tViewport.fW >= 308.0f && tViewport.fW <= 313.0f;
	(void)xuiIconPickerEnsureVisible(pDemo->pPickers[0], pDemo->arrIds[DEMO_ICON_COUNT - 1]);
	fOffsetY = 0.0f;
	(void)xuiScrollFrameGetOffset(pFrame, NULL, &fOffsetY);
	pDemo->bScrollOK = fOffsetY > 0.0f;
	(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bSelectionOK = xuiIconPickerGetSelectedId(pDemo->pPickers[0]) == pDemo->arrIds[0] &&
		pDemo->iChangeCount > 0 && !xuiIconPickerIsOpen(pDemo->pPickers[0]);
	pDemo->bExerciseDone = 1;
}

static int __xuiIconPickerCreateAssets(xui_iconpicker_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_WIDTH, (float)DEMO_HEIGHT);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_WIDTH;
	tSurfaceDesc.iHeight = DEMO_HEIGHT;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiIconPickerFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiIconPickerCreateIcons(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiIconPickerCreateUi(pDemo);
}

static void __xuiIconPickerDestroyAssets(xui_iconpicker_demo_t* pDemo)
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

static int __xuiIconPickerFrame(void* pUser)
{
	xui_iconpicker_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int bAutoRun;
	int iRet;

	pDemo = (xui_iconpicker_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = pDemo->iMaxFrames > 0 || pDemo->fMaxSeconds > 0.0;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiProxyXgePumpInputRect(pDemo->pContext,
		(xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_WIDTH, (float)DEMO_HEIGHT});
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiIconPickerRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_WIDTH, DEMO_HEIGHT};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_WIDTH, (float)DEMO_HEIGHT};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_WIDTH, (float)DEMO_HEIGHT};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		printf(
			"xui_iconpicker final-summary frames=%d create=%d geometry=%d scroll=%d selection=%d changes=%d selected=%u\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bGeometryOK,
			pDemo->bScrollOK,
			pDemo->bSelectionOK,
			pDemo->iChangeCount,
			(unsigned)xuiIconPickerGetSelectedId(pDemo->pPickers[0]));
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_iconpicker_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiIconPickerParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiIconPickerUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_WIDTH + 20;
	tDesc.iHeight = DEMO_HEIGHT + 50;
	tDesc.sTitle = "XUI IconPicker";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_iconpicker: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiIconPickerCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_iconpicker: create assets failed: %d\n", iRet);
		__xuiIconPickerDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiIconPickerFrame, &tDemo);
	__xuiIconPickerDestroyAssets(&tDemo);
	xgeUnit();
	return iRet == XGE_OK && tDemo.bCreateOK &&
		((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) ||
		 (tDemo.bGeometryOK && tDemo.bScrollOK && tDemo.bSelectionOK)) ? 0 : 1;
}
