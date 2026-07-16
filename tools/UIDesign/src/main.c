#include "ui_design_app.h"
#include "ui_design_canvas.h"
#include "ui_design_document.h"
#include "ui_design_inspector.h"
#include "ui_design_registry.h"
#include "ui_design_toolbox.h"

#if defined(_WIN32) && !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

#define UI_DESIGN_KEY_LEFT_SHIFT 340
#define UI_DESIGN_KEY_LEFT_CTRL 341
#define UI_DESIGN_KEY_LEFT_ALT 342
#define UI_DESIGN_KEY_RIGHT_SHIFT 344
#define UI_DESIGN_KEY_RIGHT_CTRL 345
#define UI_DESIGN_KEY_RIGHT_ALT 346
#define UI_DESIGN_MENUBAR_HEIGHT 28.0f
#define UI_DESIGN_TOOLBAR_HEIGHT 36.0f
#define UI_DESIGN_STATUSBAR_HEIGHT 24.0f
#define UI_DESIGN_CHROME_GAP 4.0f
#define UI_DESIGN_PREVIEW_W 900
#define UI_DESIGN_PREVIEW_H 600

static void __uiDesignUsage(void)
{
	printf("usage: xui_uidesign [--frames N] [--seconds N] [--exercise] [--preview FILE]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __uiDesignAppDefaultSurfaceWidth(const ui_design_app_t* pApp)
{
	return (pApp != NULL && pApp->bPreviewRunner) ? UI_DESIGN_PREVIEW_W : UI_DESIGN_TARGET_W;
}

static int __uiDesignAppDefaultSurfaceHeight(const ui_design_app_t* pApp)
{
	return (pApp != NULL && pApp->bPreviewRunner) ? UI_DESIGN_PREVIEW_H : UI_DESIGN_TARGET_H;
}

static int __uiDesignAppSurfaceWidth(const ui_design_app_t* pApp)
{
	if ( pApp != NULL && pApp->iSurfaceWidth > 0 ) return pApp->iSurfaceWidth;
	return __uiDesignAppDefaultSurfaceWidth(pApp);
}

static int __uiDesignAppSurfaceHeight(const ui_design_app_t* pApp)
{
	if ( pApp != NULL && pApp->iSurfaceHeight > 0 ) return pApp->iSurfaceHeight;
	return __uiDesignAppDefaultSurfaceHeight(pApp);
}

static float __uiDesignAppAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int __uiDesignAppRectAlmostEqual(xui_rect_t tA, xui_rect_t tB, float fEpsilon)
{
	return (__uiDesignAppAbsFloat(tA.fX - tB.fX) <= fEpsilon) &&
		(__uiDesignAppAbsFloat(tA.fY - tB.fY) <= fEpsilon) &&
		(__uiDesignAppAbsFloat(tA.fW - tB.fW) <= fEpsilon) &&
		(__uiDesignAppAbsFloat(tA.fH - tB.fH) <= fEpsilon);
}

static int __uiDesignAppApplyViewportLayout(ui_design_app_t* pApp, int iWidth, int iHeight)
{
	float fDockY;
	float fDockW;
	float fDockH;
	float fStatusY;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iWidth < 1 ) iWidth = 1;
	if ( iHeight < 1 ) iHeight = 1;
	if ( pApp->pRoot != NULL ) {
		(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight});
	}
	if ( pApp->bPreviewRunner ) {
		pApp->pArtboard = pApp->pRoot;
		uiDesignAppInvalidate(pApp);
		return XUI_OK;
	}
	if ( pApp->pMenuBar != NULL ) {
		(void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0.0f, 0.0f, (float)iWidth, UI_DESIGN_MENUBAR_HEIGHT});
	}
	if ( pApp->pToolbar != NULL ) {
		(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0.0f, UI_DESIGN_MENUBAR_HEIGHT, (float)iWidth, UI_DESIGN_TOOLBAR_HEIGHT});
	}
	if ( pApp->pStatusBar != NULL ) {
		fStatusY = (float)iHeight - UI_DESIGN_STATUSBAR_HEIGHT;
		if ( fStatusY < 0.0f ) fStatusY = 0.0f;
		(void)xuiWidgetSetRect(pApp->pStatusBar, (xui_rect_t){0.0f, fStatusY, (float)iWidth, UI_DESIGN_STATUSBAR_HEIGHT});
	}
	if ( pApp->pDock != NULL ) {
		fDockY = UI_DESIGN_MENUBAR_HEIGHT + UI_DESIGN_TOOLBAR_HEIGHT + UI_DESIGN_CHROME_GAP;
		fDockW = (float)iWidth - 16.0f;
		fDockH = (float)iHeight - UI_DESIGN_MENUBAR_HEIGHT - UI_DESIGN_TOOLBAR_HEIGHT - UI_DESIGN_STATUSBAR_HEIGHT - UI_DESIGN_CHROME_GAP * 2.0f;
		if ( fDockW < 0.0f ) fDockW = 0.0f;
		if ( fDockH < 0.0f ) fDockH = 0.0f;
		(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){8.0f, fDockY, fDockW, fDockH});
	}
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignAppResizeTarget(ui_design_app_t* pApp, int iWidth, int iHeight)
{
	xui_surface_desc_t tSurface;
	xui_surface pNewTarget;
	int iRet;

	if ( pApp == NULL || pApp->pContext == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iWidth <= 0 ) iWidth = __uiDesignAppDefaultSurfaceWidth(pApp);
	if ( iHeight <= 0 ) iHeight = __uiDesignAppDefaultSurfaceHeight(pApp);
	if ( iWidth < 1 ) iWidth = 1;
	if ( iHeight < 1 ) iHeight = 1;
	if ( pApp->pTarget != NULL && pApp->iSurfaceWidth == iWidth && pApp->iSurfaceHeight == iHeight ) return XUI_OK;
	memset(&tSurface, 0, sizeof(tSurface));
	tSurface.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurface.iWidth = iWidth;
	tSurface.iHeight = iHeight;
	tSurface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurface.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	pNewTarget = NULL;
	iRet = pApp->tProxy.surfaceCreate(&pApp->tProxy, &pNewTarget, &tSurface);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetViewportSize(pApp->pContext, (float)iWidth, (float)iHeight);
	if ( iRet == XUI_OK ) iRet = xuiInputViewport(pApp->pContext, (float)iWidth, (float)iHeight);
	if ( iRet != XUI_OK ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pNewTarget);
		return iRet;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
	}
	pApp->pTarget = pNewTarget;
	pApp->iSurfaceWidth = iWidth;
	pApp->iSurfaceHeight = iHeight;
	return __uiDesignAppApplyViewportLayout(pApp, iWidth, iHeight);
}

static int __uiDesignAppRefreshWindowSize(ui_design_app_t* pApp)
{
	int iWidth;
	int iHeight;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth <= 0 || iHeight <= 0 ) return XUI_OK;
	if ( pApp->iSurfaceWidth == iWidth && pApp->iSurfaceHeight == iHeight ) return XUI_OK;
	return __uiDesignAppResizeTarget(pApp, iWidth, iHeight);
}

static int __uiDesignParseArgs(ui_design_app_t* pApp, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pApp->iMaxFrames = atoi(argv[++i]);
			if ( pApp->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pApp->iMaxFrames = atoi(argv[i] + 9);
			if ( pApp->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pApp->fMaxSeconds = atof(argv[++i]);
			if ( pApp->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pApp->fMaxSeconds = atof(argv[i] + 10);
			if ( pApp->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--exercise") == 0 ) {
			pApp->bExercise = 1;
		} else if ( strcmp(argv[i], "--preview") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pApp->bPreviewRunner = 1;
			snprintf(pApp->sPreviewPath, sizeof(pApp->sPreviewPath), "%s", argv[++i]);
		} else if ( strncmp(argv[i], "--preview=", 10) == 0 ) {
			pApp->bPreviewRunner = 1;
			snprintf(pApp->sPreviewPath, sizeof(pApp->sPreviewPath), "%s", argv[i] + 10);
		} else if ( strcmp(argv[i], "--preview-delete") == 0 ) {
			pApp->bPreviewDeleteFile = 1;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__uiDesignUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __uiDesignFindTtf(void)
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

void uiDesignAppInvalidate(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pToolbox != NULL ) (void)xuiWidgetInvalidate(pApp->pToolbox, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pArtboard != NULL ) (void)xuiWidgetInvalidate(pApp->pArtboard, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pOverlay != NULL ) (void)xuiWidgetInvalidate(pApp->pOverlay, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pInspector != NULL ) (void)xuiWidgetInvalidate(pApp->pInspector, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pContext != NULL ) (void)xuiInvalidateAll(pApp->pContext);
}

static int __uiDesignAppRefreshInspector(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->bSyncingInspector || pApp->bSyncingTree || pApp->pInspector == NULL ) return XUI_OK;
	return uiDesignInspectorRefresh(pApp);
}

int uiDesignAppSelectNode(ui_design_app_t* pApp, int iId)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	pApp->sEditBuffer[0] = '\0';
	pApp->iEditLength = 0;
	iRet = uiDesignModelSetSelected(&pApp->tModel, iId);
	if ( iRet != XUI_OK ) return iRet;
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppSetStatus(pApp, (iId > 0) ? "Selected control" : "Ready");
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppAddNodeSelection(ui_design_app_t* pApp, int iId)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	pApp->sEditBuffer[0] = '\0';
	pApp->iEditLength = 0;
	iRet = uiDesignModelAddSelection(&pApp->tModel, iId);
	if ( iRet != XUI_OK ) return iRet;
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppSetStatus(pApp, "Selection changed");
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppToggleNodeSelection(ui_design_app_t* pApp, int iId)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	pApp->sEditBuffer[0] = '\0';
	pApp->iEditLength = 0;
	iRet = uiDesignModelToggleSelection(&pApp->tModel, iId);
	if ( iRet != XUI_OK ) return iRet;
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppSetStatus(pApp, "Selection changed");
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignAppCarouselPage(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iPage;
	int iPageCount;

	if ( pParentNode == NULL ) return 0;
	iPageCount = uiDesignNodeGetPropertyInt(pParentNode, "data.pageCount", 1);
	if ( iPageCount < 1 ) iPageCount = 1;
	iPage = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.carouselPage", -1) : -1;
	if ( iPage < 0 ) iPage = uiDesignNodeGetPropertyInt(pParentNode, "data.current", 0);
	if ( iPage < 0 ) iPage = 0;
	if ( iPage >= iPageCount ) iPage = iPageCount - 1;
	return iPage;
}

static int __uiDesignAppSplitPane(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode, float fDesignX, float fDesignY)
{
	int iPane;
	int iPaneCount;
	int iOrientation;
	float fRatio;

	if ( pParentNode == NULL ) return 0;
	iPaneCount = uiDesignNodeGetPropertyInt(pParentNode, "data.paneCount", 2);
	if ( iPaneCount < 1 ) iPaneCount = 1;
	if ( iPaneCount > XUI_SPLIT_LAYOUT_MAX_PANES ) iPaneCount = XUI_SPLIT_LAYOUT_MAX_PANES;
	iPane = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.splitPane", -1) : -1;
	if ( iPane < 0 ) {
		iOrientation = uiDesignNodeGetPropertyInt(pParentNode, "behavior.orientation", XUI_ORIENTATION_VERTICAL);
		if ( iOrientation == XUI_ORIENTATION_VERTICAL ) {
			fRatio = (pParentNode->tRect.fW > 1.0f) ? (fDesignX / pParentNode->tRect.fW) : 0.0f;
		} else {
			fRatio = (pParentNode->tRect.fH > 1.0f) ? (fDesignY / pParentNode->tRect.fH) : 0.0f;
		}
		iPane = (int)(fRatio * (float)iPaneCount);
	}
	if ( iPane < 0 ) iPane = 0;
	if ( iPane >= iPaneCount ) iPane = iPaneCount - 1;
	return iPane;
}

static int __uiDesignAppTabsPage(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iPage;
	int iPageCount;

	if ( pParentNode == NULL ) return 0;
	iPageCount = uiDesignNodeGetPropertyInt(pParentNode, "data.pageCount", -1);
	if ( iPageCount < 1 ) {
		const char* sCursor = uiDesignNodeGetProperty(pParentNode, "data.items", "");
		char sLine[256];
		iPageCount = 0;
		while ( sCursor != NULL && *sCursor != 0 && iPageCount < XUI_TABS_PAGE_CAPACITY ) {
			int i = 0;
			while ( sCursor[i] != 0 && sCursor[i] != '\n' && sCursor[i] != '\r' && i < (int)sizeof(sLine) - 1 ) {
				sLine[i] = sCursor[i];
				i++;
			}
			sLine[i] = 0;
			if ( sLine[0] != 0 ) iPageCount++;
			while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) sCursor++;
			while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
		}
	}
	if ( iPageCount < 1 ) iPageCount = 1;
	if ( iPageCount > XUI_TABS_PAGE_CAPACITY ) iPageCount = XUI_TABS_PAGE_CAPACITY;
	iPage = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.tabPage", -1) : -1;
	if ( iPage < 0 ) iPage = uiDesignNodeGetPropertyInt(pParentNode, "data.selected", 0);
	if ( iPage < 0 ) iPage = 0;
	if ( iPage >= iPageCount ) iPage = iPageCount - 1;
	return iPage;
}

static int __uiDesignAppAccordionSection(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iSection;
	int iSectionCount;
	const char* sCursor;
	char sLine[256];

	if ( pParentNode == NULL ) return 0;
	iSection = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.accordionSection", -1) : -1;
	if ( iSection < 0 ) {
		iSection = 0;
	}
	sCursor = uiDesignNodeGetProperty(pParentNode, "data.sections", "");
	iSectionCount = 0;
	while ( sCursor != NULL && *sCursor != 0 && iSectionCount < XUI_ACCORDION_SECTION_CAPACITY ) {
		int i = 0;
		while ( sCursor[i] != 0 && sCursor[i] != '\n' && sCursor[i] != '\r' && i < (int)sizeof(sLine) - 1 ) {
			sLine[i] = sCursor[i];
			i++;
		}
		sLine[i] = 0;
		if ( sLine[0] != 0 ) iSectionCount++;
		while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) sCursor++;
		while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
	}
	if ( iSectionCount < 1 ) iSectionCount = 1;
	if ( iSection < 0 ) iSection = 0;
	if ( iSection >= iSectionCount ) iSection = iSectionCount - 1;
	return iSection;
}

static int __uiDesignAppDockWindow(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iWindow;
	int iWindowCount;
	const char* sCursor;
	char sLine[256];

	if ( pParentNode == NULL ) return 0;
	iWindow = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.dockWindow", -1) : -1;
	if ( iWindow < 0 ) iWindow = 0;
	sCursor = uiDesignNodeGetProperty(pParentNode, "data.windows", "");
	iWindowCount = 0;
	while ( sCursor != NULL && *sCursor != 0 && iWindowCount < XUI_DOCK_PANEL_WINDOW_CAPACITY ) {
		int i = 0;
		while ( sCursor[i] != 0 && sCursor[i] != '\n' && sCursor[i] != '\r' && i < (int)sizeof(sLine) - 1 ) {
			sLine[i] = sCursor[i];
			i++;
		}
		sLine[i] = 0;
		if ( sLine[0] != 0 ) iWindowCount++;
		while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) sCursor++;
		while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
	}
	if ( iWindowCount < 1 ) iWindowCount = 1;
	if ( iWindow >= iWindowCount ) iWindow = iWindowCount - 1;
	return iWindow;
}

static int __uiDesignAppLower(int c)
{
	if ( c >= 'A' && c <= 'Z' ) return c - 'A' + 'a';
	return c;
}

static int __uiDesignAppTextEqualsNoCase(const char* sA, const char* sB)
{
	int a;
	int b;

	if ( (sA == NULL) || (sB == NULL) ) return 0;
	while ( sA[0] != 0 && sB[0] != 0 ) {
		a = __uiDesignAppLower((unsigned char)sA[0]);
		b = __uiDesignAppLower((unsigned char)sB[0]);
		if ( a != b ) return 0;
		++sA;
		++sB;
	}
	return (sA[0] == 0 && sB[0] == 0);
}

static void __uiDesignAppTrim(char* sText)
{
	char* sStart;
	size_t iLen;

	if ( sText == NULL ) return;
	sStart = sText;
	while ( *sStart == ' ' || *sStart == '\t' || *sStart == '\r' || *sStart == '\n' ) ++sStart;
	if ( sStart != sText ) memmove(sText, sStart, strlen(sStart) + 1u);
	iLen = strlen(sText);
	while ( iLen > 0u && (sText[iLen - 1u] == ' ' || sText[iLen - 1u] == '\t' || sText[iLen - 1u] == '\r' || sText[iLen - 1u] == '\n') ) {
		sText[--iLen] = 0;
	}
}

static int __uiDesignAppNextLine(const char** ppText, char* sLine, int iCapacity)
{
	const char* sText;
	int i;

	if ( (ppText == NULL) || (sLine == NULL) || (iCapacity <= 0) ) return 0;
	sText = *ppText;
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	while ( sText[0] != 0 && sText[0] != '\n' ) {
		if ( i < iCapacity - 1 ) sLine[i++] = sText[0];
		++sText;
	}
	if ( sText[0] == '\n' ) ++sText;
	sLine[i] = 0;
	__uiDesignAppTrim(sLine);
	*ppText = sText;
	return 1;
}

static int __uiDesignAppSplitFields(char* sLine, char** arrFields, int iCapacity)
{
	char* sCursor;
	int iCount;
	int i;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	arrFields[iCount++] = sLine;
	sCursor = sLine;
	while ( sCursor[0] != 0 ) {
		if ( sCursor[0] == '|' ) {
			sCursor[0] = 0;
			if ( iCount < iCapacity ) arrFields[iCount++] = sCursor + 1;
		}
		++sCursor;
	}
	for ( i = 0; i < iCount; ++i ) __uiDesignAppTrim(arrFields[i]);
	return iCount;
}

static int __uiDesignAppLooksIntText(const char* sText)
{
	int i;

	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	if ( sText[i] == '-' || sText[i] == '+' ) ++i;
	if ( sText[i] == 0 ) return 0;
	for ( ; sText[i] != 0; ++i ) {
		if ( sText[i] < '0' || sText[i] > '9' ) return 0;
	}
	return 1;
}

static int __uiDesignAppLayoutSizeModeFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( __uiDesignAppTextEqualsNoCase(sText, "fixed") || __uiDesignAppTextEqualsNoCase(sText, "pixel") ||
	     __uiDesignAppTextEqualsNoCase(sText, "pixels") || __uiDesignAppTextEqualsNoCase(sText, "px") ) return XUI_SIZE_FIXED;
	if ( __uiDesignAppTextEqualsNoCase(sText, "content") || __uiDesignAppTextEqualsNoCase(sText, "auto") ) return XUI_SIZE_CONTENT;
	if ( __uiDesignAppTextEqualsNoCase(sText, "fill") || __uiDesignAppTextEqualsNoCase(sText, "stretch") ||
	     __uiDesignAppTextEqualsNoCase(sText, "star") || __uiDesignAppTextEqualsNoCase(sText, "grow") ) return XUI_SIZE_FILL;
	if ( __uiDesignAppLooksIntText(sText) ) {
		iValue = atoi(sText);
		if ( iValue == XUI_SIZE_FIXED || iValue == XUI_SIZE_CONTENT || iValue == XUI_SIZE_FILL ) return iValue;
	}
	return iDefault;
}

static xui_table_track_t __uiDesignAppTableTrackFromValues(int iSizeMode, float fValue,
	float fMin, int bHasMin, float fMax, int bHasMax, float fWeight, int bHasWeight)
{
	xui_table_track_t tTrack;

	tTrack.iSizeMode = iSizeMode;
	tTrack.fValue = 0.0f;
	tTrack.fMin = 0.0f;
	tTrack.fMax = XUI_LAYOUT_UNBOUNDED;
	tTrack.fWeight = 1.0f;
	if ( bHasMin ) tTrack.fMin = (fMin > 0.0f) ? fMin : 0.0f;
	if ( bHasMax ) tTrack.fMax = (fMax > 0.0f) ? fMax : 0.0f;
	if ( tTrack.fMax < tTrack.fMin ) tTrack.fMax = tTrack.fMin;
	if ( bHasWeight ) tTrack.fWeight = (fWeight > 0.0f) ? fWeight : 1.0f;
	if ( tTrack.iSizeMode == XUI_SIZE_FIXED ) {
		tTrack.fValue = (fValue > 0.0f) ? fValue : 0.0f;
	} else if ( tTrack.iSizeMode == XUI_SIZE_FILL ) {
		tTrack.fWeight = bHasWeight ? tTrack.fWeight : ((fValue > 0.0f) ? fValue : 1.0f);
	}
	return tTrack;
}

static xui_table_track_t __uiDesignAppTableTrackFromValue(int iSizeMode, float fValue)
{
	return __uiDesignAppTableTrackFromValues(iSizeMode, fValue, 0.0f, 0, 0.0f, 0, 1.0f, 0);
}

static void __uiDesignAppApplyTableTrackConfig(ui_design_node_t* pNode, xui_widget pHost, const char* sPropertyId, int bRows)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[6];
	xui_table_track_t tTrack;
	int iFieldCount;
	int iTrack;
	int iSizeMode;
	float fValue;

	if ( (pNode == NULL) || (pHost == NULL) || (sPropertyId == NULL) ) return;
	sCursor = uiDesignNodeGetProperty(pNode, sPropertyId, "");
	if ( (sCursor == NULL) || (sCursor[0] == 0) ) return;
	while ( __uiDesignAppNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignAppSplitFields(sLine, arrFields, 6);
		if ( iFieldCount < 2 || !__uiDesignAppLooksIntText(arrFields[0]) ) continue;
		iTrack = atoi(arrFields[0]);
		if ( iTrack < 0 ) continue;
		iSizeMode = __uiDesignAppLayoutSizeModeFromText((iFieldCount > 1) ? arrFields[1] : "content", XUI_SIZE_CONTENT);
		fValue = (float)atof((iFieldCount > 2) ? arrFields[2] : "0");
		tTrack = __uiDesignAppTableTrackFromValues(iSizeMode, fValue,
			(float)atof((iFieldCount > 3) ? arrFields[3] : "0"), iFieldCount > 3 && arrFields[3][0] != 0,
			(float)atof((iFieldCount > 4) ? arrFields[4] : "0"), iFieldCount > 4 && arrFields[4][0] != 0,
			(float)atof((iFieldCount > 5) ? arrFields[5] : "1"), iFieldCount > 5 && arrFields[5][0] != 0);
		if ( bRows ) {
			(void)xuiWidgetSetTableRow(pHost, iTrack, &tTrack);
		} else {
			(void)xuiWidgetSetTableColumn(pHost, iTrack, &tTrack);
		}
	}
}

static void __uiDesignAppApplyContainerHostLayout(ui_design_node_t* pNode, xui_widget pHost, int bApplyOverflow)
{
	xui_thickness_t tPadding;
	xui_table_track_t tTrack;

	if ( (pNode == NULL) || (pHost == NULL) ) return;
	(void)xuiWidgetSetLayoutType(pHost, uiDesignNodeGetPropertyInt(pNode, "layout.type", XUI_LAYOUT_MANUAL));
	if ( bApplyOverflow ) {
		(void)xuiWidgetSetOverflow(pHost, uiDesignNodeGetPropertyInt(pNode, "layout.overflow", XUI_OVERFLOW_VISIBLE));
	}
	(void)xuiWidgetSetGap(pHost, uiDesignNodeGetPropertyFloat(pNode, "layout.gap", 0.0f));
	tPadding = (xui_thickness_t){
		uiDesignNodeGetPropertyFloat(pNode, "layout.paddingLeft", 0.0f),
		uiDesignNodeGetPropertyFloat(pNode, "layout.paddingTop", 0.0f),
		uiDesignNodeGetPropertyFloat(pNode, "layout.paddingRight", 0.0f),
		uiDesignNodeGetPropertyFloat(pNode, "layout.paddingBottom", 0.0f)
	};
	(void)xuiWidgetSetPadding(pHost, tPadding);
	(void)xuiWidgetSetTableSize(pHost,
		uiDesignNodeGetPropertyInt(pNode, "layout.tableRows", 1),
		uiDesignNodeGetPropertyInt(pNode, "layout.tableColumns", 1));
	(void)xuiWidgetSetGridMetrics(pHost,
		uiDesignNodeGetPropertyInt(pNode, "layout.gridColumns", 1),
		uiDesignNodeGetPropertyFloat(pNode, "layout.gridItemWidth", 0.0f),
		uiDesignNodeGetPropertyFloat(pNode, "layout.gridItemHeight", 0.0f));
	if ( uiDesignNodeGetProperty(pNode, "layout.tableRowSizeMode", NULL)[0] != '\0' ||
	     uiDesignNodeGetProperty(pNode, "layout.tableRowValue", NULL)[0] != '\0' ) {
		tTrack = __uiDesignAppTableTrackFromValue(
			uiDesignNodeGetPropertyInt(pNode, "layout.tableRowSizeMode", XUI_SIZE_FIXED),
			uiDesignNodeGetPropertyFloat(pNode, "layout.tableRowValue", 0.0f));
		(void)xuiWidgetSetTableRow(pHost, uiDesignNodeGetPropertyInt(pNode, "layout.tableTrackRow", 0), &tTrack);
	}
	if ( uiDesignNodeGetProperty(pNode, "layout.tableColumnSizeMode", NULL)[0] != '\0' ||
	     uiDesignNodeGetProperty(pNode, "layout.tableColumnValue", NULL)[0] != '\0' ) {
		tTrack = __uiDesignAppTableTrackFromValue(
			uiDesignNodeGetPropertyInt(pNode, "layout.tableColumnSizeMode", XUI_SIZE_FIXED),
			uiDesignNodeGetPropertyFloat(pNode, "layout.tableColumnValue", 0.0f));
		(void)xuiWidgetSetTableColumn(pHost, uiDesignNodeGetPropertyInt(pNode, "layout.tableTrackColumn", 0), &tTrack);
	}
	__uiDesignAppApplyTableTrackConfig(pNode, pHost, "layout.tableRowsConfig", 1);
	__uiDesignAppApplyTableTrackConfig(pNode, pHost, "layout.tableColumnsConfig", 0);
}

static int __uiDesignAppAttachNodeWidget(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode, xui_widget pParent, xui_widget pChild)
{
	xui_widget pContent;
	int iRet;
	int iPage;
	int iPane;
	int iSection;
	int iWindow;

	if ( (pParent == NULL) || (pChild == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( xuiWidgetGetParent(pChild) != NULL ) {
		iRet = xuiWidgetRemoveFromParent(pChild);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pParentNode == NULL ) return xuiWidgetAddChild(pParent, pChild);
	switch ( pParentNode->iType ) {
	case UI_DESIGN_NODE_PANEL:
		pContent = xuiPanelGetClientWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 0);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_WINDOW:
		pContent = xuiWindowGetClientWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 1);
		return xuiWindowAddChild(pParent, pChild);
	case UI_DESIGN_NODE_SCROLL_VIEW:
		pContent = xuiScrollViewGetContentWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 1);
		iRet = xuiWidgetAddChild(pContent, pChild);
		return iRet;
	case UI_DESIGN_NODE_SCROLL_FRAME:
		pContent = xuiScrollFrameGetViewportWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 0);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_POPUP:
		pContent = xuiPopupGetContentWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_CAROUSEL:
		iPage = __uiDesignAppCarouselPage(pParentNode, pChildNode);
		return xuiCarouselAddPageChild(pParent, iPage, pChild);
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		iPane = __uiDesignAppSplitPane(pParentNode, pChildNode, 0.0f, 0.0f);
		return xuiSplitLayoutAddPaneChild(pParent, iPane, pChild);
	case UI_DESIGN_NODE_TABS:
		iPage = __uiDesignAppTabsPage(pParentNode, pChildNode);
		pContent = xuiTabsGetPageWidget(pParent, iPage);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_ACCORDION:
		iSection = __uiDesignAppAccordionSection(pParentNode, pChildNode);
		return xuiAccordionAddSectionChild(pParent, iSection, pChild);
	case UI_DESIGN_NODE_DOCK_PANEL:
		iWindow = __uiDesignAppDockWindow(pParentNode, pChildNode);
		pContent = xuiDockPanelGetWindowClient(pParent, iWindow);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignAppApplyContainerHostLayout(pParentNode, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_RADIO_GROUP:
		if ( pChildNode->iType == UI_DESIGN_NODE_RADIO ) return xuiRadioGroupAddRadio(pParent, pChild);
		if ( pChildNode->iType == UI_DESIGN_NODE_CHECK_CARD ) return xuiRadioGroupAddCheckCard(pParent, pChild);
		return xuiWidgetAddChild(pParent, pChild);
	default:
		return xuiWidgetAddChild(pParent, pChild);
	}
}

int uiDesignAppCreateNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;
	ui_design_node_t* pParentNode;
	xui_widget pParent;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pNode->pWidget != NULL ) return uiDesignAppSyncNodeWidget(pApp, pNode);
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( (pDesc == NULL) || (pDesc->onCreate == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pParentNode = NULL;
	if ( pNode->iParentId == 0 ) {
		pParent = pApp->pArtboard;
	} else {
		pParentNode = uiDesignModelGetNode(&pApp->tModel, pNode->iParentId);
		pParent = (pParentNode != NULL) ? pParentNode->pWidget : NULL;
	}
	if ( pParent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = pDesc->onCreate(pApp, pNode, &pNode->pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAppAttachNodeWidget(pParentNode, pNode, pParent, pNode->pWidget);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pNode->pWidget);
		pNode->pWidget = NULL;
		return iRet;
	}
	return uiDesignAppSyncNodeWidget(pApp, pNode);
}

int uiDesignAppSyncNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;

	if ( (pApp == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pNode->pWidget == NULL ) return uiDesignAppCreateNodeWidget(pApp, pNode);
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( (pDesc == NULL) || (pDesc->onApply == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return pDesc->onApply(pApp, pNode);
}

static int __uiDesignAppDropDockSide(float fW, float fH, float fX, float fY)
{
	float fLeft;
	float fRight;
	float fTop;
	float fBottom;
	float fMin;
	int iDock;

	if ( (fW <= 1.0f) || (fH <= 1.0f) ) return XUI_DOCK_FILL;
	if ( (fX > fW * 0.25f) && (fX < fW * 0.75f) && (fY > fH * 0.25f) && (fY < fH * 0.75f) ) return XUI_DOCK_FILL;
	fLeft = fX;
	fRight = fW - fX;
	fTop = fY;
	fBottom = fH - fY;
	fMin = fLeft;
	iDock = XUI_DOCK_LEFT;
	if ( fTop < fMin ) {
		fMin = fTop;
		iDock = XUI_DOCK_TOP;
	}
	if ( fRight < fMin ) {
		fMin = fRight;
		iDock = XUI_DOCK_RIGHT;
	}
	if ( fBottom < fMin ) iDock = XUI_DOCK_BOTTOM;
	return iDock;
}

static int __uiDesignAppCountSubtree(const ui_design_model_t* pModel, int iRootId)
{
	const ui_design_node_t* pNode;
	int iCount;
	int i;

	pNode = uiDesignModelGetNodeConst(pModel, iRootId);
	if ( pNode == NULL ) return 0;
	iCount = 1;
	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		if ( pModel->arrNodes[i].iParentId == iRootId ) {
			iCount += __uiDesignAppCountSubtree(pModel, pModel->arrNodes[i].iId);
		}
	}
	return iCount;
}

static void __uiDesignAppCopyPersistentNode(ui_design_clipboard_node_t* pDst, const ui_design_node_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) ) return;
	memset(pDst, 0, sizeof(*pDst));
	pDst->iOriginalId = pSrc->iId;
	pDst->iOriginalParentId = pSrc->iParentId;
	pDst->iType = pSrc->iType;
	pDst->tRect = pSrc->tRect;
	snprintf(pDst->sText, sizeof(pDst->sText), "%s", pSrc->sText);
	pDst->bChecked = pSrc->bChecked;
	pDst->bVisible = pSrc->bVisible;
	pDst->bEnabled = pSrc->bEnabled;
	pDst->iPropertyCount = pSrc->iPropertyCount;
	if ( pDst->iPropertyCount > UI_DESIGN_MAX_NODE_PROPERTIES ) pDst->iPropertyCount = UI_DESIGN_MAX_NODE_PROPERTIES;
	if ( pDst->iPropertyCount > 0 ) {
		memcpy(pDst->arrProperties, pSrc->arrProperties, sizeof(pDst->arrProperties[0]) * (size_t)pDst->iPropertyCount);
	}
}

static int __uiDesignAppFillClipboardSubtree(ui_design_app_t* pApp, int iRootId, int* pCount)
{
	const ui_design_node_t* pNode;
	int i;
	int iRet;

	if ( (pApp == NULL) || (pCount == NULL) || (pApp->pClipboardNodes == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetNodeConst(&pApp->tModel, iRootId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( *pCount >= pApp->iClipboardCapacity ) return XUI_ERROR_OUT_OF_MEMORY;
	__uiDesignAppCopyPersistentNode(&pApp->pClipboardNodes[*pCount], pNode);
	(*pCount)++;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		if ( pApp->tModel.arrNodes[i].iParentId == iRootId ) {
			iRet = __uiDesignAppFillClipboardSubtree(pApp, pApp->tModel.arrNodes[i].iId, pCount);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __uiDesignAppNodeIsDescendantOf(const ui_design_model_t* pModel, int iNodeId, int iRootId);

static int __uiDesignAppSelectionRootIds(ui_design_app_t* pApp, int* arrRootIds, int iCapacity)
{
	int i;
	int j;
	int iId;
	int iCount;
	int bDescendantSelected;

	if ( (pApp == NULL) || (arrRootIds == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	for ( i = 0; i < pApp->tModel.iSelectedCount; ++i ) {
		iId = pApp->tModel.arrSelectedIds[i];
		if ( iId <= 0 || uiDesignModelGetNode(&pApp->tModel, iId) == NULL ) continue;
		bDescendantSelected = 0;
		for ( j = 0; j < pApp->tModel.iSelectedCount; ++j ) {
			if ( i == j ) continue;
			if ( __uiDesignAppNodeIsDescendantOf(&pApp->tModel, iId, pApp->tModel.arrSelectedIds[j]) ) {
				bDescendantSelected = 1;
				break;
			}
		}
		if ( bDescendantSelected ) continue;
		if ( iCount < iCapacity ) arrRootIds[iCount++] = iId;
	}
	return iCount;
}

static int __uiDesignAppClipboardHasOriginalId(ui_design_app_t* pApp, int iOriginalId)
{
	int i;

	if ( (pApp == NULL) || (iOriginalId <= 0) ) return 0;
	for ( i = 0; i < pApp->iClipboardNodeCount; ++i ) {
		if ( pApp->pClipboardNodes[i].iOriginalId == iOriginalId ) return 1;
	}
	return 0;
}

static int __uiDesignAppCopyNodeList(ui_design_app_t* pApp, const int* arrRootIds, int iRootCount)
{
	const ui_design_node_t* pNode;
	ui_design_clipboard_node_t* pNewClipboard;
	xui_rect_t tAbs;
	float fMinX;
	float fMinY;
	int i;
	int iStart;
	int iTotal;
	int iRet;

	if ( (pApp == NULL) || (arrRootIds == NULL) || (iRootCount <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iTotal = 0;
	fMinX = 0.0f;
	fMinY = 0.0f;
	for ( i = 0; i < iRootCount; ++i ) {
		pNode = uiDesignModelGetNodeConst(&pApp->tModel, arrRootIds[i]);
		if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		iTotal += __uiDesignAppCountSubtree(&pApp->tModel, arrRootIds[i]);
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, arrRootIds[i], &tAbs) == XUI_OK ) {
			if ( i == 0 || tAbs.fX < fMinX ) fMinX = tAbs.fX;
			if ( i == 0 || tAbs.fY < fMinY ) fMinY = tAbs.fY;
		}
	}
	if ( iTotal <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pNewClipboard = (ui_design_clipboard_node_t*)calloc((size_t)iTotal, sizeof(ui_design_clipboard_node_t));
	if ( pNewClipboard == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	free(pApp->pClipboardNodes);
	pApp->pClipboardNodes = pNewClipboard;
	pApp->iClipboardCapacity = iTotal;
	pApp->iClipboardNodeCount = 0;
	pApp->iClipboardRootOriginalId = arrRootIds[0];
	pApp->fClipboardRootX = fMinX;
	pApp->fClipboardRootY = fMinY;
	for ( i = 0; i < iRootCount; ++i ) {
		iStart = pApp->iClipboardNodeCount;
		iRet = __uiDesignAppFillClipboardSubtree(pApp, arrRootIds[i], &pApp->iClipboardNodeCount);
		if ( iRet != XUI_OK ) {
			free(pApp->pClipboardNodes);
			pApp->pClipboardNodes = NULL;
			pApp->iClipboardNodeCount = 0;
			pApp->iClipboardCapacity = 0;
			pApp->iClipboardRootOriginalId = 0;
			return iRet;
		}
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, arrRootIds[i], &tAbs) == XUI_OK ) {
			pApp->pClipboardNodes[iStart].iOriginalParentId = 0;
			pApp->pClipboardNodes[iStart].tRect = tAbs;
		}
	}
	return XUI_OK;
}

static int __uiDesignAppNodeIndexById(const ui_design_model_t* pModel, int iId)
{
	int i;

	if ( pModel == NULL || iId <= 0 ) return -1;
	for ( i = 0; i < pModel->iNodeCount; ++i ) {
		if ( pModel->arrNodes[i].iId == iId ) return i;
	}
	return -1;
}

static int __uiDesignAppNodeMarkedById(const ui_design_model_t* pModel, const unsigned char* arrDelete, int iId)
{
	int iIndex;

	iIndex = __uiDesignAppNodeIndexById(pModel, iId);
	return (iIndex >= 0 && arrDelete != NULL && arrDelete[iIndex] != 0);
}

static int __uiDesignAppNodeIsDescendantOf(const ui_design_model_t* pModel, int iNodeId, int iRootId)
{
	const ui_design_node_t* pNode;

	if ( (pModel == NULL) || (iNodeId <= 0) || (iRootId <= 0) ) return 0;
	if ( iNodeId == iRootId ) return 1;
	pNode = uiDesignModelGetNodeConst(pModel, iNodeId);
	while ( pNode != NULL && pNode->iParentId != 0 ) {
		if ( pNode->iParentId == iRootId ) return 1;
		pNode = uiDesignModelGetNodeConst(pModel, pNode->iParentId);
	}
	return 0;
}

static void __uiDesignAppReleaseNodeRuntime(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	int j;

	if ( (pApp == NULL) || (pNode == NULL) ) return;
	if ( pNode->pRuntimeFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pNode->pRuntimeFont);
		pNode->pRuntimeFont = NULL;
		pNode->sRuntimeFontSource[0] = '\0';
		pNode->fRuntimeFontSize = 0.0f;
		pNode->iRuntimeFontFlags = 0u;
	}
	if ( pNode->pRuntimeSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pNode->pRuntimeSurface);
		pNode->pRuntimeSurface = NULL;
		pNode->sRuntimeSurfaceSource[0] = '\0';
	}
	for ( j = 0; j < UI_DESIGN_RUNTIME_SURFACE_COUNT; j++ ) {
		if ( pNode->arrRuntimeSurface[j] != NULL ) {
			pApp->tProxy.surfaceDestroy(&pApp->tProxy, pNode->arrRuntimeSurface[j]);
			pNode->arrRuntimeSurface[j] = NULL;
			pNode->arrRuntimeSurfaceSource[j][0] = '\0';
		}
	}
	for ( j = 0; j < UI_DESIGN_RUNTIME_MENUBAR_MENUS; j++ ) {
		if ( pNode->arrRuntimeMenuPopup[j] != NULL ) {
			xuiWidgetDestroy(pNode->arrRuntimeMenuPopup[j]);
			pNode->arrRuntimeMenuPopup[j] = NULL;
		}
	}
}

static void __uiDesignAppApplyDropLayoutProperties(ui_design_app_t* pApp, ui_design_node_t* pNode, int iParentId, xui_rect_t tParentRect, int bHasParentHost, float fLocalDropX, float fLocalDropY)
{
	ui_design_node_t* pParentNode;
	char sValue[16];
	int iParentLayout;
	int iRows;
	int iColumns;
	int iRow;
	int iColumn;
	int iDock;

	if ( (pApp == NULL) || (pNode == NULL) || (iParentId == 0) ) return;
	pParentNode = uiDesignModelGetNode(&pApp->tModel, iParentId);
	if ( pParentNode == NULL ) return;
	iParentLayout = uiDesignNodeGetPropertyInt(pParentNode, "layout.type", XUI_LAYOUT_MANUAL);
	if ( iParentLayout == XUI_LAYOUT_OVERLAY ) {
		snprintf(sValue, sizeof(sValue), "%d", XUI_FLOW_ABSOLUTE);
		(void)uiDesignNodeSetProperty(pNode, "layout.flowMode", sValue);
	} else if ( (iParentLayout == XUI_LAYOUT_TABLE) && bHasParentHost ) {
		iRows = uiDesignNodeGetPropertyInt(pParentNode, "layout.tableRows", 1);
		iColumns = uiDesignNodeGetPropertyInt(pParentNode, "layout.tableColumns", 1);
		if ( iRows < 1 ) iRows = 1;
		if ( iColumns < 1 ) iColumns = 1;
		iRow = (tParentRect.fH > 1.0f) ? (int)(fLocalDropY / (tParentRect.fH / (float)iRows)) : 0;
		iColumn = (tParentRect.fW > 1.0f) ? (int)(fLocalDropX / (tParentRect.fW / (float)iColumns)) : 0;
		if ( iRow < 0 ) iRow = 0;
		if ( iColumn < 0 ) iColumn = 0;
		if ( iRow >= iRows ) iRow = iRows - 1;
		if ( iColumn >= iColumns ) iColumn = iColumns - 1;
		snprintf(sValue, sizeof(sValue), "%d", iRow);
		(void)uiDesignNodeSetProperty(pNode, "layout.tableCellRow", sValue);
		snprintf(sValue, sizeof(sValue), "%d", iColumn);
		(void)uiDesignNodeSetProperty(pNode, "layout.tableCellColumn", sValue);
	} else if ( (iParentLayout == XUI_LAYOUT_DOCK) && bHasParentHost ) {
		iDock = __uiDesignAppDropDockSide(tParentRect.fW, tParentRect.fH, fLocalDropX, fLocalDropY);
		snprintf(sValue, sizeof(sValue), "%d", iDock);
		(void)uiDesignNodeSetProperty(pNode, "layout.dock", sValue);
	}
	if ( pParentNode->iType == UI_DESIGN_NODE_CAROUSEL ) {
		snprintf(sValue, sizeof(sValue), "%d", __uiDesignAppCarouselPage(pParentNode, pNode));
		(void)uiDesignNodeSetProperty(pNode, "layout.carouselPage", sValue);
	} else if ( pParentNode->iType == UI_DESIGN_NODE_SPLIT_LAYOUT ) {
		snprintf(sValue, sizeof(sValue), "%d", __uiDesignAppSplitPane(pParentNode, pNode, pNode->tRect.fX + pNode->tRect.fW * 0.5f, pNode->tRect.fY + pNode->tRect.fH * 0.5f));
		(void)uiDesignNodeSetProperty(pNode, "layout.splitPane", sValue);
	} else if ( pParentNode->iType == UI_DESIGN_NODE_TABS ) {
		snprintf(sValue, sizeof(sValue), "%d", __uiDesignAppTabsPage(pParentNode, pNode));
		(void)uiDesignNodeSetProperty(pNode, "layout.tabPage", sValue);
	} else if ( pParentNode->iType == UI_DESIGN_NODE_ACCORDION ) {
		snprintf(sValue, sizeof(sValue), "%d", __uiDesignAppAccordionSection(pParentNode, pNode));
		(void)uiDesignNodeSetProperty(pNode, "layout.accordionSection", sValue);
	} else if ( pParentNode->iType == UI_DESIGN_NODE_DOCK_PANEL ) {
		snprintf(sValue, sizeof(sValue), "%d", __uiDesignAppDockWindow(pParentNode, pNode));
		(void)uiDesignNodeSetProperty(pNode, "layout.dockWindow", sValue);
	}
}

int uiDesignAppCopyNode(ui_design_app_t* pApp, int iId)
{
	int arrRootIds[1];

	if ( (pApp == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	arrRootIds[0] = iId;
	return __uiDesignAppCopyNodeList(pApp, arrRootIds, 1);
}

int uiDesignAppCopySelection(ui_design_app_t* pApp)
{
	int arrRootIds[UI_DESIGN_MAX_NODES];
	int iRootCount;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRootCount = __uiDesignAppSelectionRootIds(pApp, arrRootIds, UI_DESIGN_MAX_NODES);
	if ( iRootCount <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	return __uiDesignAppCopyNodeList(pApp, arrRootIds, iRootCount);
}

int uiDesignAppDeleteNode(ui_design_app_t* pApp, int iId)
{
	ui_design_model_t* pModel;
	ui_design_node_t* pNode;
	ui_design_node_t* pParentNode;
	unsigned char arrDelete[UI_DESIGN_MAX_NODES];
	int iParentId;
	int iNewSelected;
	int bContextDeleted;
	int iRead;
	int iWrite;

	if ( (pApp == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pModel = &pApp->tModel;
	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrDelete, 0, sizeof(arrDelete));
	for ( iRead = 0; iRead < pModel->iNodeCount; ++iRead ) {
		if ( __uiDesignAppNodeIsDescendantOf(pModel, pModel->arrNodes[iRead].iId, iId) ) {
			arrDelete[iRead] = 1u;
		}
	}
	iParentId = pNode->iParentId;
	iNewSelected = __uiDesignAppNodeIsDescendantOf(pModel, pModel->iSelectedId, iId) ? iParentId : pModel->iSelectedId;
	bContextDeleted = __uiDesignAppNodeIsDescendantOf(pModel, pApp->iContextMenuNodeId, iId);
	for ( iRead = 0; iRead < pModel->iNodeCount; ++iRead ) {
		if ( !arrDelete[iRead] ) continue;
		if ( pModel->arrNodes[iRead].pWidget != NULL &&
		     !__uiDesignAppNodeMarkedById(pModel, arrDelete, pModel->arrNodes[iRead].iParentId) ) {
			xuiWidgetDestroy(pModel->arrNodes[iRead].pWidget);
		}
		pModel->arrNodes[iRead].pWidget = NULL;
		__uiDesignAppReleaseNodeRuntime(pApp, &pModel->arrNodes[iRead]);
	}
	iWrite = 0;
	for ( iRead = 0; iRead < pModel->iNodeCount; ++iRead ) {
		if ( arrDelete[iRead] ) continue;
		if ( iWrite != iRead ) pModel->arrNodes[iWrite] = pModel->arrNodes[iRead];
		iWrite++;
	}
	pModel->iNodeCount = iWrite;
	if ( iNewSelected != 0 && uiDesignModelGetNode(pModel, iNewSelected) == NULL ) iNewSelected = 0;
	(void)uiDesignModelSetSelected(pModel, iNewSelected);
	pModel->iRevision++;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	if ( bContextDeleted ) pApp->iContextMenuNodeId = 0;
	if ( iParentId != 0 ) {
		pParentNode = uiDesignModelGetNode(pModel, iParentId);
		if ( pParentNode != NULL && pParentNode->pWidget != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pParentNode);
	}
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppDeleteSelection(ui_design_app_t* pApp)
{
	int arrRootIds[UI_DESIGN_MAX_NODES];
	int iRootCount;
	int i;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRootCount = __uiDesignAppSelectionRootIds(pApp, arrRootIds, UI_DESIGN_MAX_NODES);
	if ( iRootCount <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iRootCount; ++i ) {
		if ( uiDesignModelGetNode(&pApp->tModel, arrRootIds[i]) == NULL ) continue;
		iRet = uiDesignAppDeleteNode(pApp, arrRootIds[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

int uiDesignAppCutNode(ui_design_app_t* pApp, int iId)
{
	int iRet;

	iRet = uiDesignAppCopyNode(pApp, iId);
	if ( iRet != XUI_OK ) return iRet;
	return uiDesignAppDeleteNode(pApp, iId);
}

static int __uiDesignAppPasteClipboardAt(ui_design_app_t* pApp, float fDesignX, float fDesignY, int iForcedParentId, int* pNewRootId)
{
	ui_design_clipboard_node_t* pClip;
	ui_design_node_t* pNode;
	ui_design_node_t* pParentNode;
	xui_rect_t tParentRect;
	xui_rect_t tRect;
	int arrOriginalIds[UI_DESIGN_MAX_NODES];
	int arrNewIds[UI_DESIGN_MAX_NODES];
	int iParentId;
	int iMappedParent;
	int iNewId;
	int iRootNewId;
	int i;
	int j;
	int iRet;
	int bPasteRoot;
	int bHasParentHost;
	float fLocalX;
	float fLocalY;

	if ( (pApp == NULL) || (pApp->pClipboardNodes == NULL) || (pApp->iClipboardNodeCount <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->iClipboardNodeCount > UI_DESIGN_MAX_NODES ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( iForcedParentId >= 0 ) {
		if ( iForcedParentId != 0 ) {
			pParentNode = uiDesignModelGetNode(&pApp->tModel, iForcedParentId);
			if ( pParentNode == NULL || !uiDesignNodeTypeIsContainer(pParentNode->iType) ) return XUI_ERROR_INVALID_ARGUMENT;
		}
		iParentId = iForcedParentId;
	} else {
		iParentId = uiDesignModelFindDropParent(&pApp->tModel, fDesignX, fDesignY);
	}
	fLocalX = fDesignX;
	fLocalY = fDesignY;
	bHasParentHost = 0;
	if ( iParentId != 0 && uiDesignModelGetChildHostRect(&pApp->tModel, iParentId, &tParentRect) == XUI_OK ) {
		bHasParentHost = 1;
		fLocalX = fDesignX - tParentRect.fX;
		fLocalY = fDesignY - tParentRect.fY;
	}
	if ( fLocalX < 0.0f ) fLocalX = 0.0f;
	if ( fLocalY < 0.0f ) fLocalY = 0.0f;
	memset(arrOriginalIds, 0, sizeof(arrOriginalIds));
	memset(arrNewIds, 0, sizeof(arrNewIds));
	iRootNewId = 0;
	for ( i = 0; i < pApp->iClipboardNodeCount; ++i ) {
		pClip = &pApp->pClipboardNodes[i];
		iMappedParent = iParentId;
		if ( pClip->iOriginalParentId != 0 && __uiDesignAppClipboardHasOriginalId(pApp, pClip->iOriginalParentId) ) {
			iMappedParent = 0;
			for ( j = 0; j < i; ++j ) {
				if ( arrOriginalIds[j] == pClip->iOriginalParentId ) {
					iMappedParent = arrNewIds[j];
					break;
				}
			}
			if ( iMappedParent == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
		}
		tRect = pClip->tRect;
		if ( iMappedParent == iParentId ) {
			tRect.fX = fLocalX + (pClip->tRect.fX - pApp->fClipboardRootX);
			tRect.fY = fLocalY + (pClip->tRect.fY - pApp->fClipboardRootY);
		}
		iRet = uiDesignModelAddNode(&pApp->tModel, pClip->iType, iMappedParent, tRect.fX, tRect.fY, &iNewId);
		if ( iRet != XUI_OK ) return iRet;
		pNode = uiDesignModelGetNode(&pApp->tModel, iNewId);
		if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		pNode->tRect = tRect;
		snprintf(pNode->sText, sizeof(pNode->sText), "%s", pClip->sText);
		pNode->bChecked = pClip->bChecked;
		pNode->bVisible = pClip->bVisible;
		pNode->bEnabled = pClip->bEnabled;
		pNode->iPropertyCount = pClip->iPropertyCount;
		if ( pNode->iPropertyCount > UI_DESIGN_MAX_NODE_PROPERTIES ) pNode->iPropertyCount = UI_DESIGN_MAX_NODE_PROPERTIES;
		if ( pNode->iPropertyCount > 0 ) {
			memcpy(pNode->arrProperties, pClip->arrProperties, sizeof(pNode->arrProperties[0]) * (size_t)pNode->iPropertyCount);
		}
		if ( i == 0 ) {
			__uiDesignAppApplyDropLayoutProperties(pApp, pNode, iMappedParent, tParentRect, bHasParentHost, fLocalX, fLocalY);
		}
		iRet = uiDesignAppCreateNodeWidget(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
		arrOriginalIds[i] = pClip->iOriginalId;
		arrNewIds[i] = iNewId;
		bPasteRoot = (pClip->iOriginalParentId == 0) || !__uiDesignAppClipboardHasOriginalId(pApp, pClip->iOriginalParentId);
		if ( bPasteRoot && iRootNewId == 0 ) iRootNewId = iNewId;
	}
	if ( iParentId != 0 ) {
		pParentNode = uiDesignModelGetNode(&pApp->tModel, iParentId);
		if ( pParentNode != NULL && pParentNode->pWidget != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pParentNode);
	}
	if ( iRootNewId == 0 ) iRootNewId = arrNewIds[0];
	(void)uiDesignModelClearSelection(&pApp->tModel);
	for ( i = 0; i < pApp->iClipboardNodeCount; ++i ) {
		pClip = &pApp->pClipboardNodes[i];
		bPasteRoot = (pClip->iOriginalParentId == 0) || !__uiDesignAppClipboardHasOriginalId(pApp, pClip->iOriginalParentId);
		if ( bPasteRoot && arrNewIds[i] > 0 ) (void)uiDesignModelAddSelection(&pApp->tModel, arrNewIds[i]);
	}
	if ( pApp->tModel.iSelectedCount <= 0 ) (void)uiDesignModelSetSelected(&pApp->tModel, iRootNewId);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	if ( pNewRootId != NULL ) *pNewRootId = iRootNewId;
	return XUI_OK;
}

int uiDesignAppPasteClipboard(ui_design_app_t* pApp, float fDesignX, float fDesignY, int* pNewRootId)
{
	return __uiDesignAppPasteClipboardAt(pApp, fDesignX, fDesignY, -1, pNewRootId);
}

int uiDesignAppPasteClipboardAsChild(ui_design_app_t* pApp, int iParentId, int* pNewRootId)
{
	xui_rect_t tParentHost;
	float fX;
	float fY;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iParentId != 0 ) {
		ui_design_node_t* pParent = uiDesignModelGetNode(&pApp->tModel, iParentId);
		if ( pParent == NULL || !uiDesignNodeTypeIsContainer(pParent->iType) ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( uiDesignModelGetChildHostRect(&pApp->tModel, iParentId, &tParentHost) == XUI_OK ) {
			fX = tParentHost.fX + 16.0f;
			fY = tParentHost.fY + 16.0f;
		} else if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, iParentId, &tParentHost) == XUI_OK ) {
			fX = tParentHost.fX + 16.0f;
			fY = tParentHost.fY + 16.0f;
		} else {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	} else {
		fX = 48.0f;
		fY = 48.0f;
	}
	return __uiDesignAppPasteClipboardAt(pApp, fX, fY, iParentId, pNewRootId);
}

int uiDesignAppAddNodeAt(ui_design_app_t* pApp, ui_design_node_type_t iType, float fDesignX, float fDesignY, int* pId)
{
	const ui_design_control_desc_t* pDesc;
	ui_design_node_t* pParentNode;
	ui_design_node_t* pNode;
	xui_rect_t tParentRect;
	float fW;
	float fH;
	float fX;
	float fY;
	float fLocalDropX;
	float fLocalDropY;
	char sPage[16];
	char sPane[16];
	char sSection[16];
	char sDockWindow[16];
	char sLayoutValue[16];
	int iParentId;
	int iNewId;
	int iParentLayout;
	int iRows;
	int iColumns;
	int iRow;
	int iColumn;
	int iDock;
	int bHasParentHost;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = uiDesignRegistryFind(iType);
	if ( pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fW = pDesc->fDefaultW;
	fH = pDesc->fDefaultH;
	iParentId = uiDesignModelFindDropParent(&pApp->tModel, fDesignX, fDesignY);
	fX = fDesignX - fW * 0.5f;
	fY = fDesignY - fH * 0.5f;
	fLocalDropX = fDesignX;
	fLocalDropY = fDesignY;
	bHasParentHost = 0;
	if ( iParentId != 0 && uiDesignModelGetChildHostRect(&pApp->tModel, iParentId, &tParentRect) == XUI_OK ) {
		bHasParentHost = 1;
		fLocalDropX = fDesignX - tParentRect.fX;
		fLocalDropY = fDesignY - tParentRect.fY;
		fX = fDesignX - tParentRect.fX - fW * 0.5f;
		fY = fDesignY - tParentRect.fY - fH * 0.5f;
	}
	if ( fX < 0.0f ) fX = 0.0f;
	if ( fY < 0.0f ) fY = 0.0f;
	iNewId = 0;
	iRet = uiDesignModelAddNode(&pApp->tModel, iType, iParentId, fX, fY, &iNewId);
	if ( iRet == XUI_OK ) {
		pNode = uiDesignModelGetNode(&pApp->tModel, iNewId);
		if ( pNode != NULL ) {
			pNode->tRect.fW = fW;
			pNode->tRect.fH = fH;
			iRet = uiDesignRegistryInitNodeProperties(pDesc, pNode);
			if ( iRet != XUI_OK ) return iRet;
			pParentNode = (iParentId != 0) ? uiDesignModelGetNode(&pApp->tModel, iParentId) : NULL;
			if ( pParentNode != NULL ) {
				iParentLayout = uiDesignNodeGetPropertyInt(pParentNode, "layout.type", XUI_LAYOUT_MANUAL);
				if ( iParentLayout == XUI_LAYOUT_OVERLAY ) {
					snprintf(sLayoutValue, sizeof(sLayoutValue), "%d", XUI_FLOW_ABSOLUTE);
					iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.flowMode", sLayoutValue);
					if ( iRet != XUI_OK ) return iRet;
				} else if ( (iParentLayout == XUI_LAYOUT_TABLE) && bHasParentHost ) {
					iRows = uiDesignNodeGetPropertyInt(pParentNode, "layout.tableRows", 1);
					iColumns = uiDesignNodeGetPropertyInt(pParentNode, "layout.tableColumns", 1);
					if ( iRows < 1 ) iRows = 1;
					if ( iColumns < 1 ) iColumns = 1;
					iRow = (tParentRect.fH > 1.0f) ? (int)(fLocalDropY / (tParentRect.fH / (float)iRows)) : 0;
					iColumn = (tParentRect.fW > 1.0f) ? (int)(fLocalDropX / (tParentRect.fW / (float)iColumns)) : 0;
					if ( iRow < 0 ) iRow = 0;
					if ( iColumn < 0 ) iColumn = 0;
					if ( iRow >= iRows ) iRow = iRows - 1;
					if ( iColumn >= iColumns ) iColumn = iColumns - 1;
					snprintf(sLayoutValue, sizeof(sLayoutValue), "%d", iRow);
					iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.tableCellRow", sLayoutValue);
					if ( iRet != XUI_OK ) return iRet;
					snprintf(sLayoutValue, sizeof(sLayoutValue), "%d", iColumn);
					iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.tableCellColumn", sLayoutValue);
					if ( iRet != XUI_OK ) return iRet;
				} else if ( (iParentLayout == XUI_LAYOUT_DOCK) && bHasParentHost ) {
					iDock = __uiDesignAppDropDockSide(tParentRect.fW, tParentRect.fH, fLocalDropX, fLocalDropY);
					snprintf(sLayoutValue, sizeof(sLayoutValue), "%d", iDock);
					iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.dock", sLayoutValue);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
			if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_CAROUSEL) ) {
				snprintf(sPage, sizeof(sPage), "%d", __uiDesignAppCarouselPage(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.carouselPage", sPage);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_SPLIT_LAYOUT) ) {
				snprintf(sPane, sizeof(sPane), "%d", __uiDesignAppSplitPane(pParentNode, pNode, fX + fW * 0.5f, fY + fH * 0.5f));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.splitPane", sPane);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_TABS) ) {
				snprintf(sPage, sizeof(sPage), "%d", __uiDesignAppTabsPage(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.tabPage", sPage);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_ACCORDION) ) {
				snprintf(sSection, sizeof(sSection), "%d", __uiDesignAppAccordionSection(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.accordionSection", sSection);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_DOCK_PANEL) ) {
				snprintf(sDockWindow, sizeof(sDockWindow), "%d", __uiDesignAppDockWindow(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.dockWindow", sDockWindow);
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSyncNodeWidget(pApp, pParentNode);
				if ( iRet != XUI_OK ) return iRet;
			}
			iRet = uiDesignAppCreateNodeWidget(pApp, pNode);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pId != NULL ) *pId = iNewId;
		pApp->iActiveTool = UI_DESIGN_NODE_NONE;
		pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
		(void)__uiDesignAppRefreshInspector(pApp);
		uiDesignAppInvalidate(pApp);
	}
	return iRet;
}

int uiDesignAppSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetRect(&pApp->tModel, iId, tRect);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeText(ui_design_app_t* pApp, int iId, const char* sText)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetText(&pApp->tModel, iId, sText);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetChecked(&pApp->tModel, iId, bChecked);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetVisible(&pApp->tModel, iId, bVisible);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetEnabled(&pApp->tModel, iId, bEnabled);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue)
{
	ui_design_node_t* pParentNode;
	ui_design_node_t* pNode;
	int iRet;
	int bChecked;

	if ( (pApp == NULL) || (sPropertyId == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetProperty(&pApp->tModel, iId, sPropertyId, sValue);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( strcmp(sPropertyId, "checked") == 0 ) {
		bChecked = uiDesignNodeGetPropertyBool(pNode, "checked", pNode->bChecked);
		(void)uiDesignModelSetChecked(&pApp->tModel, iId, bChecked);
	}
	if ( (strcmp(sPropertyId, "layout.carouselPage") == 0 ||
	      strcmp(sPropertyId, "layout.splitPane") == 0 ||
	      strcmp(sPropertyId, "layout.tabPage") == 0 ||
	      strcmp(sPropertyId, "layout.accordionSection") == 0) &&
	     pNode->pWidget != NULL && pNode->iParentId != 0 ) {
		pParentNode = uiDesignModelGetNode(&pApp->tModel, pNode->iParentId);
		if ( (pParentNode != NULL) && (pParentNode->pWidget != NULL) ) {
			iRet = __uiDesignAppAttachNodeWidget(pParentNode, pNode, pParentNode->pWidget, pNode->pWidget);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignCaptureSnapshot(ui_design_app_t* pApp, char** ppSnapshot)
{
	if ( (pApp == NULL) || (ppSnapshot == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return uiDesignDocumentSaveModel(&pApp->tModel, ppSnapshot);
}

static int __uiDesignParseSnapshot(const char* sSnapshot, ui_design_model_t** ppModel)
{
	return uiDesignDocumentLoadModel(sSnapshot, ppModel);
}

static void __uiDesignAppClearNodeRuntime(ui_design_app_t* pApp)
{
	ui_design_node_t* pNode;
	int i;

	if ( pApp == NULL ) return;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pNode = &pApp->tModel.arrNodes[i];
		if ( pNode->pWidget != NULL && (pNode->iParentId == 0 || uiDesignModelGetNode(&pApp->tModel, pNode->iParentId) == NULL) ) {
			xuiWidgetDestroy(pNode->pWidget);
		}
		pNode->pWidget = NULL;
		__uiDesignAppReleaseNodeRuntime(pApp, pNode);
	}
}

static int __uiDesignAppRebuildNodeRuntime(ui_design_app_t* pApp)
{
	int i;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pApp->tModel.arrNodes[i].pWidget = NULL;
		iRet = uiDesignAppCreateNodeWidget(pApp, &pApp->tModel.arrNodes[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignAppRestoreSnapshot(ui_design_app_t* pApp, const char* sSnapshot)
{
	ui_design_model_t* pNewModel;
	int iRet;

	if ( (pApp == NULL) || (sSnapshot == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNewModel = NULL;
	iRet = __uiDesignParseSnapshot(sSnapshot, &pNewModel);
	if ( iRet != XUI_OK ) return iRet;
	__uiDesignAppClearNodeRuntime(pApp);
	pApp->tModel = *pNewModel;
	free(pNewModel);
	iRet = __uiDesignAppRebuildNodeRuntime(pApp);
	if ( iRet != XUI_OK ) return iRet;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignExercisePreviewRuntime(ui_design_app_t* pApp)
{
	ui_design_node_t* pNode;
	xui_widget pSplit;
	xui_rect_t tWorld;
	xui_rect_t tHit;
	float fX;
	float fY;
	float fMoveX;
	float fMoveY;
	float fBeforeSize;
	float fAfterSize;
	int iBeforeChange;
	int iAfterChange;
	int iOrientation;
	int iRet;
	int i;

	if ( (pApp == NULL) || !pApp->bPreviewRunner ) return XUI_OK;
	pNode = NULL;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		if ( pApp->tModel.arrNodes[i].iType == UI_DESIGN_NODE_SPLIT_LAYOUT ) {
			pNode = &pApp->tModel.arrNodes[i];
			break;
		}
	}
	if ( pNode == NULL ) return XUI_OK;
	pSplit = pNode->pWidget;
	if ( pSplit == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !xuiWidgetGetHitTestVisible(pSplit) ) {
		printf("xui_uidesign preview-split-hit-test-failed id=%d\n", pNode->iId);
		return XUI_ERROR;
	}
	if ( xuiSplitLayoutGetPaneCount(pSplit) < 2 ) return XUI_OK;
	iRet = xuiLayout(pApp->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=layout ret=%d\n", iRet);
		return iRet;
	}
	iRet = xuiUpdate(pApp->pContext, 0.016f);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=update ret=%d\n", iRet);
		return iRet;
	}
	tWorld = xuiWidgetGetWorldRect(pSplit);
	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, 0);
	if ( tHit.fW <= 0.0f || tHit.fH <= 0.0f ) {
		printf("xui_uidesign preview-split-hit-rect-failed id=%d hit=%.1f/%.1f/%.1f/%.1f\n",
			pNode->iId, tHit.fX, tHit.fY, tHit.fW, tHit.fH);
		return XUI_ERROR;
	}
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	fMoveX = fX;
	fMoveY = fY;
	iOrientation = xuiSplitLayoutGetOrientation(pSplit);
	if ( iOrientation == XUI_ORIENTATION_VERTICAL ) {
		fMoveX += 32.0f;
	} else {
		fMoveY += 32.0f;
	}
	iBeforeChange = xuiSplitLayoutGetChangeCount(pSplit);
	fBeforeSize = xuiSplitLayoutGetPaneSize(pSplit, 0);
	iRet = xuiInputPointerMove(pApp->pContext, fX, fY, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=hover ret=%d\n", iRet);
		return iRet;
	}
	iRet = xuiInputPointerDown(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=down-input ret=%d\n", iRet);
		return iRet;
	}
	iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=down-dispatch ret=%d\n", iRet);
		return iRet;
	}
	if ( xuiSplitLayoutGetActiveDivider(pSplit) != 0 ) {
		printf("xui_uidesign preview-split-down-failed id=%d active=%d hit=%.1f/%.1f/%.1f/%.1f world=%.1f/%.1f/%.1f/%.1f\n",
			pNode->iId,
			xuiSplitLayoutGetActiveDivider(pSplit),
			tHit.fX, tHit.fY, tHit.fW, tHit.fH,
			tWorld.fX, tWorld.fY, tWorld.fW, tWorld.fH);
		(void)xuiInputPointerUp(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pApp->pContext);
		return XUI_ERROR;
	}
	iRet = xuiInputPointerMove(pApp->pContext, fMoveX, fMoveY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=drag ret=%d\n", iRet);
		return iRet;
	}
	iRet = xuiInputPointerUp(pApp->pContext, fMoveX, fMoveY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pApp->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign preview-split-stage-failed stage=release ret=%d\n", iRet);
		return iRet;
	}
	iAfterChange = xuiSplitLayoutGetChangeCount(pSplit);
	fAfterSize = xuiSplitLayoutGetPaneSize(pSplit, 0);
	if ( (iAfterChange <= iBeforeChange) && (__uiDesignAppAbsFloat(fAfterSize - fBeforeSize) < 0.5f) ) {
		printf("xui_uidesign preview-split-drag-failed id=%d changes=%d/%d size=%.1f/%.1f\n",
			pNode->iId, iBeforeChange, iAfterChange, fBeforeSize, fAfterSize);
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __uiDesignHistoryEntryFree(ui_design_history_entry_t* pEntry)
{
	if ( pEntry == NULL ) return;
	free(pEntry->sBefore);
	free(pEntry->sAfter);
	memset(pEntry, 0, sizeof(*pEntry));
}

static void __uiDesignHistoryClear(ui_design_history_entry_t* pEntries, int* pCount)
{
	int i;

	if ( (pEntries == NULL) || (pCount == NULL) ) return;
	for ( i = 0; i < *pCount; ++i ) {
		__uiDesignHistoryEntryFree(&pEntries[i]);
	}
	*pCount = 0;
}

static void __uiDesignHistoryPushRaw(ui_design_history_entry_t* pEntries, int* pCount, ui_design_history_entry_t* pEntry)
{
	if ( (pEntries == NULL) || (pCount == NULL) || (pEntry == NULL) ) return;
	if ( *pCount >= UI_DESIGN_HISTORY_LIMIT ) {
		__uiDesignHistoryEntryFree(&pEntries[0]);
		memmove(&pEntries[0], &pEntries[1], sizeof(pEntries[0]) * (UI_DESIGN_HISTORY_LIMIT - 1));
		*pCount = UI_DESIGN_HISTORY_LIMIT - 1;
	}
	pEntries[*pCount] = *pEntry;
	memset(pEntry, 0, sizeof(*pEntry));
	(*pCount)++;
}

static int __uiDesignHistoryPushUndo(ui_design_app_t* pApp, ui_design_command_t iCommand, const char* sName, char* sBefore, char* sAfter)
{
	ui_design_history_entry_t tEntry;

	if ( (pApp == NULL) || (sBefore == NULL) || (sAfter == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( strcmp(sBefore, sAfter) == 0 ) {
		free(sBefore);
		free(sAfter);
		return XUI_OK;
	}
	memset(&tEntry, 0, sizeof(tEntry));
	tEntry.iCommand = iCommand;
	snprintf(tEntry.sName, sizeof(tEntry.sName), "%s", (sName != NULL && sName[0] != '\0') ? sName : "Edit");
	tEntry.sBefore = sBefore;
	tEntry.sAfter = sAfter;
	__uiDesignHistoryClear(pApp->arrRedo, &pApp->iRedoCount);
	__uiDesignHistoryPushRaw(pApp->arrUndo, &pApp->iUndoCount, &tEntry);
	pApp->bDocumentDirty = 1;
	uiDesignAppUpdateCommandUI(pApp);
	return XUI_OK;
}

int uiDesignAppBeginHistoryTransaction(ui_design_app_t* pApp, ui_design_command_t iCommand, const char* sName)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	uiDesignAppCancelHistoryTransaction(pApp);
	iRet = __uiDesignCaptureSnapshot(pApp, &pApp->sHistoryTransactionBefore);
	if ( iRet != XUI_OK ) return iRet;
	pApp->iHistoryTransactionCommand = iCommand;
	snprintf(pApp->sHistoryTransactionName, sizeof(pApp->sHistoryTransactionName), "%s", (sName != NULL && sName[0] != '\0') ? sName : "Edit");
	return XUI_OK;
}

int uiDesignAppCommitHistoryTransaction(ui_design_app_t* pApp)
{
	char* sAfter;
	char* sBefore;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->sHistoryTransactionBefore == NULL ) return XUI_OK;
	sAfter = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sAfter);
	if ( iRet != XUI_OK ) {
		uiDesignAppCancelHistoryTransaction(pApp);
		return iRet;
	}
	sBefore = pApp->sHistoryTransactionBefore;
	pApp->sHistoryTransactionBefore = NULL;
	iRet = __uiDesignHistoryPushUndo(pApp, pApp->iHistoryTransactionCommand, pApp->sHistoryTransactionName, sBefore, sAfter);
	pApp->iHistoryTransactionCommand = UI_DESIGN_COMMAND_NONE;
	pApp->sHistoryTransactionName[0] = '\0';
	return iRet;
}

void uiDesignAppCancelHistoryTransaction(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return;
	free(pApp->sHistoryTransactionBefore);
	pApp->sHistoryTransactionBefore = NULL;
	pApp->iHistoryTransactionCommand = UI_DESIGN_COMMAND_NONE;
	pApp->sHistoryTransactionName[0] = '\0';
}

static int __uiDesignAppBeginImmediateUndo(ui_design_app_t* pApp, char** ppBefore)
{
	if ( (pApp == NULL) || (ppBefore == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppBefore = NULL;
	return __uiDesignCaptureSnapshot(pApp, ppBefore);
}

static int __uiDesignAppFinishImmediateUndo(ui_design_app_t* pApp, ui_design_command_t iCommand, const char* sName, char* sBefore, int iOperationRet)
{
	char* sAfter;
	int iRet;

	if ( pApp == NULL ) {
		free(sBefore);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iOperationRet != XUI_OK ) {
		free(sBefore);
		return iOperationRet;
	}
	sAfter = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sAfter);
	if ( iRet != XUI_OK ) {
		free(sBefore);
		return iRet;
	}
	iRet = __uiDesignHistoryPushUndo(pApp, iCommand, sName, sBefore, sAfter);
	uiDesignAppUpdateCommandUI(pApp);
	return iRet;
}

int uiDesignAppCommandSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect, const char* sName)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeRect(pApp, iId, tRect);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, (sName != NULL) ? sName : "Resize", sBefore, iRet);
}

int uiDesignAppCommandSetNodeText(ui_design_app_t* pApp, int iId, const char* sText)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeText(pApp, iId, sText);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, "Edit Text", sBefore, iRet);
}

int uiDesignAppCommandSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeChecked(pApp, iId, bChecked);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, "Edit Checked", sBefore, iRet);
}

int uiDesignAppCommandSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeVisible(pApp, iId, bVisible);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, "Edit Visible", sBefore, iRet);
}

int uiDesignAppCommandSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeEnabled(pApp, iId, bEnabled);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, "Edit Enabled", sBefore, iRet);
}

int uiDesignAppCommandSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue)
{
	char* sBefore;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignAppSetNodeProperty(pApp, iId, sPropertyId, sValue);
	return __uiDesignAppFinishImmediateUndo(pApp, UI_DESIGN_COMMAND_NONE, "Edit Property", sBefore, iRet);
}

void uiDesignAppSetStatus(ui_design_app_t* pApp, const char* sStatus)
{
	if ( pApp == NULL ) return;
	if ( sStatus == NULL ) sStatus = "";
	snprintf(pApp->sStatus, sizeof(pApp->sStatus), "%s", sStatus);
	if ( pApp->pStatusBar != NULL && pApp->iStatusMessageItem >= 0 ) {
		(void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusMessageItem, pApp->sStatus);
	}
}

static void __uiDesignAppUpdateStatusBar(ui_design_app_t* pApp)
{
	char sSelection[80];
	char sZoom[48];

	if ( (pApp == NULL) || (pApp->pStatusBar == NULL) ) return;
	if ( pApp->sStatus[0] == '\0' ) snprintf(pApp->sStatus, sizeof(pApp->sStatus), "Ready");
	snprintf(sSelection, sizeof(sSelection), "%d selected", pApp->tModel.iSelectedCount);
	snprintf(sZoom, sizeof(sZoom), "%.0f%%", (pApp->fZoom > 0.01f ? pApp->fZoom : 1.0f) * 100.0f);
	if ( pApp->iStatusMessageItem >= 0 ) (void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusMessageItem, pApp->sStatus);
	if ( pApp->iStatusSelectionItem >= 0 ) (void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusSelectionItem, sSelection);
	if ( pApp->iStatusZoomItem >= 0 ) (void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusZoomItem, sZoom);
}

static int __uiDesignAppSelectionTransformIds(ui_design_app_t* pApp, int* arrIds, int iCapacity)
{
	ui_design_node_t* pNode;
	int i;
	int iId;
	int iCount;

	if ( (pApp == NULL) || (arrIds == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	for ( i = 0; i < pApp->tModel.iSelectedCount; ++i ) {
		iId = pApp->tModel.arrSelectedIds[i];
		pNode = uiDesignModelGetNode(&pApp->tModel, iId);
		if ( pNode == NULL ) continue;
		if ( !uiDesignModelCanFreeTransformNode(&pApp->tModel, pNode) ) continue;
		if ( iCount < iCapacity ) arrIds[iCount++] = iId;
	}
	return iCount;
}

static int __uiDesignAppSelectionTransformCount(const ui_design_app_t* pApp)
{
	const ui_design_node_t* pNode;
	int i;
	int iId;
	int iCount;

	if ( pApp == NULL ) return 0;
	iCount = 0;
	for ( i = 0; i < pApp->tModel.iSelectedCount; ++i ) {
		iId = pApp->tModel.arrSelectedIds[i];
		pNode = uiDesignModelGetNodeConst(&pApp->tModel, iId);
		if ( pNode == NULL ) continue;
		if ( !uiDesignModelCanFreeTransformNode(&pApp->tModel, pNode) ) continue;
		++iCount;
	}
	return iCount;
}

static xui_rect_t __uiDesignAppAbsoluteToLocalRect(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_rect_t tAbs)
{
	xui_rect_t tParent;

	if ( (pApp != NULL) && (pNode != NULL) && pNode->iParentId != 0 &&
	     uiDesignModelGetChildHostRect(&pApp->tModel, pNode->iParentId, &tParent) == XUI_OK ) {
		tAbs.fX -= tParent.fX;
		tAbs.fY -= tParent.fY;
	}
	return tAbs;
}

static int __uiDesignAppSetNodeAbsoluteRect(ui_design_app_t* pApp, int iId, xui_rect_t tAbs)
{
	ui_design_node_t* pNode;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return uiDesignAppSetNodeRect(pApp, iId, __uiDesignAppAbsoluteToLocalRect(pApp, pNode, tAbs));
}

static int __uiDesignAppAlignSelection(ui_design_app_t* pApp, ui_design_command_t iCommand)
{
	int arrIds[UI_DESIGN_MAX_NODES];
	xui_rect_t tPrimary;
	xui_rect_t tRect;
	float fTarget;
	int i;
	int iCount;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = __uiDesignAppSelectionTransformIds(pApp, arrIds, UI_DESIGN_MAX_NODES);
	if ( iCount < 2 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, pApp->tModel.iSelectedId, &tPrimary) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iCount; ++i ) {
		if ( arrIds[i] == pApp->tModel.iSelectedId ) continue;
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, arrIds[i], &tRect) != XUI_OK ) continue;
		switch ( iCommand ) {
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT:
			tRect.fX = tPrimary.fX;
			break;
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER:
			fTarget = tPrimary.fX + tPrimary.fW * 0.5f;
			tRect.fX = fTarget - tRect.fW * 0.5f;
			break;
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT:
			tRect.fX = tPrimary.fX + tPrimary.fW - tRect.fW;
			break;
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP:
			tRect.fY = tPrimary.fY;
			break;
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE:
			fTarget = tPrimary.fY + tPrimary.fH * 0.5f;
			tRect.fY = fTarget - tRect.fH * 0.5f;
			break;
		case UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM:
			tRect.fY = tPrimary.fY + tPrimary.fH - tRect.fH;
			break;
		default:
			break;
		}
		iRet = __uiDesignAppSetNodeAbsoluteRect(pApp, arrIds[i], tRect);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

typedef struct ui_design_sort_item_t {
	int iId;
	xui_rect_t tRect;
} ui_design_sort_item_t;

static void __uiDesignSortItems(ui_design_sort_item_t* pItems, int iCount, int bVertical)
{
	ui_design_sort_item_t tTmp;
	int i;
	int j;
	float a;
	float b;

	for ( i = 0; i < iCount; ++i ) {
		for ( j = i + 1; j < iCount; ++j ) {
			a = bVertical ? pItems[i].tRect.fY : pItems[i].tRect.fX;
			b = bVertical ? pItems[j].tRect.fY : pItems[j].tRect.fX;
			if ( b < a ) {
				tTmp = pItems[i];
				pItems[i] = pItems[j];
				pItems[j] = tTmp;
			}
		}
	}
}

static int __uiDesignAppDistributeSelection(ui_design_app_t* pApp, int bVertical)
{
	int arrIds[UI_DESIGN_MAX_NODES];
	ui_design_sort_item_t arrItems[UI_DESIGN_MAX_NODES];
	float fStart;
	float fEnd;
	float fTotal;
	float fGap;
	float fCursor;
	int i;
	int iCount;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = __uiDesignAppSelectionTransformIds(pApp, arrIds, UI_DESIGN_MAX_NODES);
	if ( iCount < 3 ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iCount; ++i ) {
		arrItems[i].iId = arrIds[i];
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, arrIds[i], &arrItems[i].tRect) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	__uiDesignSortItems(arrItems, iCount, bVertical);
	fStart = bVertical ? arrItems[0].tRect.fY : arrItems[0].tRect.fX;
	fEnd = bVertical ? (arrItems[iCount - 1].tRect.fY + arrItems[iCount - 1].tRect.fH) : (arrItems[iCount - 1].tRect.fX + arrItems[iCount - 1].tRect.fW);
	fTotal = 0.0f;
	for ( i = 0; i < iCount; ++i ) fTotal += bVertical ? arrItems[i].tRect.fH : arrItems[i].tRect.fW;
	fGap = (fEnd - fStart - fTotal) / (float)(iCount - 1);
	fCursor = fStart;
	for ( i = 0; i < iCount; ++i ) {
		if ( bVertical ) {
			arrItems[i].tRect.fY = fCursor;
			fCursor += arrItems[i].tRect.fH + fGap;
		} else {
			arrItems[i].tRect.fX = fCursor;
			fCursor += arrItems[i].tRect.fW + fGap;
		}
		iRet = __uiDesignAppSetNodeAbsoluteRect(pApp, arrItems[i].iId, arrItems[i].tRect);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignSubtreeEndIndex(const ui_design_model_t* pModel, int iStart)
{
	int iRootId;
	int i;

	if ( (pModel == NULL) || (iStart < 0) || (iStart >= pModel->iNodeCount) ) return iStart;
	iRootId = pModel->arrNodes[iStart].iId;
	for ( i = iStart + 1; i < pModel->iNodeCount; ++i ) {
		if ( !__uiDesignAppNodeIsDescendantOf(pModel, pModel->arrNodes[i].iId, iRootId) ) break;
	}
	return i;
}

static int __uiDesignMoveNodeBlock(ui_design_model_t* pModel, int iStart, int iEnd, int iInsert)
{
	ui_design_node_t* pTemp;
	int iCount;
	int iMove;

	if ( (pModel == NULL) || (iStart < 0) || (iEnd <= iStart) || (iEnd > pModel->iNodeCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = iEnd - iStart;
	pTemp = (ui_design_node_t*)malloc(sizeof(ui_design_node_t) * (size_t)iCount);
	if ( pTemp == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memcpy(pTemp, &pModel->arrNodes[iStart], sizeof(ui_design_node_t) * (size_t)iCount);
	if ( iInsert > iStart ) {
		iInsert -= iCount;
		iMove = iInsert - iStart;
		if ( iMove > 0 ) memmove(&pModel->arrNodes[iStart], &pModel->arrNodes[iEnd], sizeof(ui_design_node_t) * (size_t)iMove);
		memcpy(&pModel->arrNodes[iStart + iMove], pTemp, sizeof(ui_design_node_t) * (size_t)iCount);
	} else {
		iMove = iStart - iInsert;
		if ( iMove > 0 ) memmove(&pModel->arrNodes[iInsert + iCount], &pModel->arrNodes[iInsert], sizeof(ui_design_node_t) * (size_t)iMove);
		memcpy(&pModel->arrNodes[iInsert], pTemp, sizeof(ui_design_node_t) * (size_t)iCount);
	}
	free(pTemp);
	pModel->iRevision++;
	return XUI_OK;
}

static int __uiDesignAppReorderPrimary(ui_design_app_t* pApp, int bForward)
{
	ui_design_model_t* pModel;
	ui_design_node_t* pNode;
	int iIndex;
	int iStart;
	int iEnd;
	int iSiblingStart;
	int iSiblingEnd;
	int iParent;
	int i;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pModel = &pApp->tModel;
	pNode = uiDesignModelGetNode(pModel, pModel->iSelectedId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __uiDesignAppNodeIndexById(pModel, pNode->iId);
	if ( iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iParent = pNode->iParentId;
	iStart = iIndex;
	iEnd = __uiDesignSubtreeEndIndex(pModel, iStart);
	__uiDesignAppClearNodeRuntime(pApp);
	if ( bForward ) {
		iSiblingStart = -1;
		for ( i = iEnd; i < pModel->iNodeCount; ++i ) {
			if ( pModel->arrNodes[i].iParentId == iParent ) {
				iSiblingStart = i;
				break;
			}
		}
		if ( iSiblingStart < 0 ) {
			iRet = XUI_OK;
		} else {
			iSiblingEnd = __uiDesignSubtreeEndIndex(pModel, iSiblingStart);
			iRet = __uiDesignMoveNodeBlock(pModel, iStart, iEnd, iSiblingEnd);
		}
	} else {
		iSiblingStart = -1;
		for ( i = 0; i < iStart; ++i ) {
			if ( pModel->arrNodes[i].iParentId == iParent ) iSiblingStart = i;
		}
		if ( iSiblingStart < 0 ) {
			iRet = XUI_OK;
		} else {
			iRet = __uiDesignMoveNodeBlock(pModel, iStart, iEnd, iSiblingStart);
		}
	}
	if ( iRet == XUI_OK ) iRet = __uiDesignAppRebuildNodeRuntime(pApp);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return iRet;
}

int uiDesignAppPromoteNode(ui_design_app_t* pApp, int iId)
{
	ui_design_node_t* pNode;
	ui_design_node_t* pParentNode;
	ui_design_node_t* pNewParentNode;
	xui_rect_t tAbs;
	xui_rect_t tNewParentHost;
	xui_rect_t tLocal;
	int iOldParentId;
	int iNewParentId;
	int iRet;
	int bHasNewParentHost;

	if ( (pApp == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL || pNode->iParentId == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iOldParentId = pNode->iParentId;
	pParentNode = uiDesignModelGetNode(&pApp->tModel, iOldParentId);
	if ( pParentNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNewParentId = pParentNode->iParentId;
	if ( iNewParentId != 0 ) {
		pNewParentNode = uiDesignModelGetNode(&pApp->tModel, iNewParentId);
		if ( pNewParentNode == NULL || !uiDesignNodeTypeIsContainer(pNewParentNode->iType) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, iId, &tAbs) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	tLocal = tAbs;
	tNewParentHost = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	bHasNewParentHost = 0;
	if ( iNewParentId != 0 && uiDesignModelGetChildHostRect(&pApp->tModel, iNewParentId, &tNewParentHost) == XUI_OK ) {
		bHasNewParentHost = 1;
		tLocal.fX -= tNewParentHost.fX;
		tLocal.fY -= tNewParentHost.fY;
	}
	if ( tLocal.fX < 0.0f ) tLocal.fX = 0.0f;
	if ( tLocal.fY < 0.0f ) tLocal.fY = 0.0f;
	__uiDesignAppClearNodeRuntime(pApp);
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iParentId = iNewParentId;
	pNode->tRect = tLocal;
	__uiDesignAppApplyDropLayoutProperties(pApp, pNode, iNewParentId, tNewParentHost, bHasNewParentHost, tLocal.fX + tLocal.fW * 0.5f, tLocal.fY + tLocal.fH * 0.5f);
	pApp->tModel.iRevision++;
	iRet = __uiDesignAppRebuildNodeRuntime(pApp);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return iRet;
}

static void __uiDesignAppDefaultPastePoint(ui_design_app_t* pApp, float* pX, float* pY)
{
	xui_rect_t tBounds;

	if ( (pApp == NULL) || (pX == NULL) || (pY == NULL) ) return;
	if ( uiDesignModelGetSelectionBounds(&pApp->tModel, &tBounds) == XUI_OK ) {
		*pX = tBounds.fX + 18.0f;
		*pY = tBounds.fY + 18.0f;
		return;
	}
	if ( pApp->bHasMouse && uiDesignCanvasWorldToDesign(pApp, pApp->fLastMouseX, pApp->fLastMouseY, pX, pY) == XUI_OK ) return;
	*pX = 48.0f;
	*pY = 48.0f;
}

int uiDesignAppCanExecuteCommand(const ui_design_app_t* pApp, ui_design_command_t iCommand)
{
	int iSelectionCount;
	int iTransformCount;

	if ( pApp == NULL ) return 0;
	iSelectionCount = pApp->tModel.iSelectedCount;
	iTransformCount = __uiDesignAppSelectionTransformCount(pApp);
	switch ( iCommand ) {
	case UI_DESIGN_COMMAND_EDIT_UNDO: return pApp->iUndoCount > 0;
	case UI_DESIGN_COMMAND_EDIT_REDO: return pApp->iRedoCount > 0;
	case UI_DESIGN_COMMAND_EDIT_COPY:
	case UI_DESIGN_COMMAND_EDIT_CUT:
	case UI_DESIGN_COMMAND_EDIT_DUPLICATE:
	case UI_DESIGN_COMMAND_EDIT_DELETE:
		return iSelectionCount > 0;
	case UI_DESIGN_COMMAND_EDIT_PASTE:
		return pApp->iClipboardNodeCount > 0;
	case UI_DESIGN_COMMAND_EDIT_SELECT_ALL:
		return pApp->tModel.iNodeCount > 0;
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM:
		return iTransformCount >= 2;
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H:
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V:
		return iTransformCount >= 3;
	case UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD:
	case UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD:
		return pApp->tModel.iSelectedId > 0;
	default:
		return 1;
	}
}

static int __uiDesignAppExecuteUndo(ui_design_app_t* pApp)
{
	ui_design_history_entry_t tEntry;
	int iRet;

	if ( (pApp == NULL) || (pApp->iUndoCount <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iUndoCount--;
	tEntry = pApp->arrUndo[pApp->iUndoCount];
	memset(&pApp->arrUndo[pApp->iUndoCount], 0, sizeof(pApp->arrUndo[pApp->iUndoCount]));
	iRet = __uiDesignAppRestoreSnapshot(pApp, tEntry.sBefore);
	if ( iRet != XUI_OK ) {
		__uiDesignHistoryEntryFree(&tEntry);
		return iRet;
	}
	__uiDesignHistoryPushRaw(pApp->arrRedo, &pApp->iRedoCount, &tEntry);
	uiDesignAppSetStatus(pApp, "Undo");
	uiDesignAppUpdateCommandUI(pApp);
	return XUI_OK;
}

static int __uiDesignAppExecuteRedo(ui_design_app_t* pApp)
{
	ui_design_history_entry_t tEntry;
	int iRet;

	if ( (pApp == NULL) || (pApp->iRedoCount <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iRedoCount--;
	tEntry = pApp->arrRedo[pApp->iRedoCount];
	memset(&pApp->arrRedo[pApp->iRedoCount], 0, sizeof(pApp->arrRedo[pApp->iRedoCount]));
	iRet = __uiDesignAppRestoreSnapshot(pApp, tEntry.sAfter);
	if ( iRet != XUI_OK ) {
		__uiDesignHistoryEntryFree(&tEntry);
		return iRet;
	}
	__uiDesignHistoryPushRaw(pApp->arrUndo, &pApp->iUndoCount, &tEntry);
	uiDesignAppSetStatus(pApp, "Redo");
	uiDesignAppUpdateCommandUI(pApp);
	return XUI_OK;
}

static int __uiDesignAppExecuteUndoableCommand(ui_design_app_t* pApp, ui_design_command_t iCommand, const char* sName)
{
	char* sBefore;
	float fX;
	float fY;
	int iRet;

	iRet = __uiDesignAppBeginImmediateUndo(pApp, &sBefore);
	if ( iRet != XUI_OK ) return iRet;
	switch ( iCommand ) {
	case UI_DESIGN_COMMAND_FILE_NEW:
		__uiDesignAppClearNodeRuntime(pApp);
		uiDesignModelInit(&pApp->tModel);
		(void)__uiDesignAppRefreshInspector(pApp);
		uiDesignAppInvalidate(pApp);
		iRet = XUI_OK;
		break;
	case UI_DESIGN_COMMAND_EDIT_CUT:
		iRet = uiDesignAppCopySelection(pApp);
		if ( iRet == XUI_OK ) iRet = uiDesignAppDeleteSelection(pApp);
		break;
	case UI_DESIGN_COMMAND_EDIT_PASTE:
		__uiDesignAppDefaultPastePoint(pApp, &fX, &fY);
		iRet = uiDesignAppPasteClipboard(pApp, fX, fY, NULL);
		break;
	case UI_DESIGN_COMMAND_EDIT_DUPLICATE:
		iRet = uiDesignAppCopySelection(pApp);
		if ( iRet == XUI_OK ) {
			__uiDesignAppDefaultPastePoint(pApp, &fX, &fY);
			iRet = uiDesignAppPasteClipboard(pApp, fX, fY, NULL);
		}
		break;
	case UI_DESIGN_COMMAND_EDIT_DELETE:
		iRet = uiDesignAppDeleteSelection(pApp);
		break;
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM:
		iRet = __uiDesignAppAlignSelection(pApp, iCommand);
		break;
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H:
		iRet = __uiDesignAppDistributeSelection(pApp, 0);
		break;
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V:
		iRet = __uiDesignAppDistributeSelection(pApp, 1);
		break;
	case UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD:
		iRet = __uiDesignAppReorderPrimary(pApp, 1);
		break;
	case UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD:
		iRet = __uiDesignAppReorderPrimary(pApp, 0);
		break;
	default:
		iRet = XUI_ERROR_INVALID_ARGUMENT;
		break;
	}
	return __uiDesignAppFinishImmediateUndo(pApp, iCommand, sName, sBefore, iRet);
}

static int __uiDesignAppWriteTextFile(const char* sPath, const char* sText)
{
	FILE* pFile;

	if ( (sPath == NULL) || (sText == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) return XUI_ERROR;
	if ( fputs(sText, pFile) < 0 ) {
		fclose(pFile);
		return XUI_ERROR;
	}
	fclose(pFile);
	return XUI_OK;
}

static char* __uiDesignAppReadTextFile(const char* sPath)
{
	FILE* pFile;
	char* sData;
	long iSize;

	if ( sPath == NULL ) return NULL;
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) return NULL;
	if ( fseek(pFile, 0, SEEK_END) != 0 ) {
		fclose(pFile);
		return NULL;
	}
	iSize = ftell(pFile);
	if ( iSize < 0 ) {
		fclose(pFile);
		return NULL;
	}
	rewind(pFile);
	sData = (char*)malloc((size_t)iSize + 1u);
	if ( sData == NULL ) {
		fclose(pFile);
		return NULL;
	}
	if ( fread(sData, 1u, (size_t)iSize, pFile) != (size_t)iSize ) {
		free(sData);
		fclose(pFile);
		return NULL;
	}
	sData[iSize] = '\0';
	fclose(pFile);
	return sData;
}

static int __uiDesignAppSaveDocument(ui_design_app_t* pApp, const char* sPath)
{
	char* sSnapshot;
	int iRet;

	if ( (pApp == NULL) || (sPath == NULL) || (sPath[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	sSnapshot = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAppWriteTextFile(sPath, sSnapshot);
	free(sSnapshot);
	if ( iRet == XUI_OK ) {
		snprintf(pApp->sDocumentPath, sizeof(pApp->sDocumentPath), "%s", sPath);
		pApp->bDocumentDirty = 0;
		uiDesignAppSetStatus(pApp, "Saved");
		uiDesignAppUpdateCommandUI(pApp);
	}
	return iRet;
}

static int __uiDesignAppOpenDocument(ui_design_app_t* pApp, const char* sPath)
{
	char* sSnapshot;
	int iRet;

	if ( (pApp == NULL) || (sPath == NULL) || (sPath[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	sSnapshot = __uiDesignAppReadTextFile(sPath);
	if ( sSnapshot == NULL ) return XUI_ERROR;
	iRet = __uiDesignAppRestoreSnapshot(pApp, sSnapshot);
	free(sSnapshot);
	if ( iRet == XUI_OK ) {
		snprintf(pApp->sDocumentPath, sizeof(pApp->sDocumentPath), "%s", sPath);
		__uiDesignHistoryClear(pApp->arrUndo, &pApp->iUndoCount);
		__uiDesignHistoryClear(pApp->arrRedo, &pApp->iRedoCount);
		pApp->bDocumentDirty = 0;
		uiDesignAppSetStatus(pApp, "Opened");
		uiDesignAppUpdateCommandUI(pApp);
	}
	return iRet;
}

static int __uiDesignAppExportDocument(ui_design_app_t* pApp, const char* sPath)
{
	char* sSnapshot;
	int iRet;

	if ( (pApp == NULL) || (sPath == NULL) || (sPath[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	sSnapshot = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAppWriteTextFile(sPath, sSnapshot);
	free(sSnapshot);
	if ( iRet == XUI_OK ) {
		uiDesignAppSetStatus(pApp, "Exported");
		uiDesignAppUpdateCommandUI(pApp);
	}
	return iRet;
}

static int __uiDesignAppLaunchPreview(ui_design_app_t* pApp)
{
#if defined(_WIN32)
	char* sSnapshot;
	char sTempDir[UI_DESIGN_PATH_CAPACITY];
	char sTempFile[UI_DESIGN_PATH_CAPACITY];
	char sExe[UI_DESIGN_PATH_CAPACITY];
	char sParams[UI_DESIGN_PATH_CAPACITY * 2];
	DWORD iLen;
	HINSTANCE hExec;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sSnapshot = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	iLen = GetTempPathA((DWORD)sizeof(sTempDir), sTempDir);
	if ( iLen == 0 || iLen >= sizeof(sTempDir) ) {
		free(sSnapshot);
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	if ( GetTempFileNameA(sTempDir, "xui", 0, sTempFile) == 0 ) {
		free(sSnapshot);
		return XUI_ERROR;
	}
	iRet = __uiDesignAppWriteTextFile(sTempFile, sSnapshot);
	free(sSnapshot);
	if ( iRet != XUI_OK ) {
		(void)remove(sTempFile);
		return iRet;
	}
	iLen = GetModuleFileNameA(NULL, sExe, (DWORD)sizeof(sExe));
	if ( iLen == 0 || iLen >= sizeof(sExe) ) {
		(void)remove(sTempFile);
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	snprintf(sParams, sizeof(sParams), "--preview-delete --preview \"%s\"", sTempFile);
	hExec = ShellExecuteA(NULL, "open", sExe, sParams, NULL, SW_SHOWNORMAL);
	if ( (INT_PTR)hExec <= 32 ) {
		(void)remove(sTempFile);
		return XUI_ERROR;
	}
	pApp->bPreviewMode = 0;
	uiDesignAppSetStatus(pApp, "Preview window opened");
	return XUI_OK;
#else
	(void)pApp;
	return XUI_ERROR_UNSUPPORTED;
#endif
}

static void __uiDesignFileDialogResult(xui_file_dialog pDialog, int iResult, const char* sPath, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_command_t iCommand;
	int iRet;

	(void)pDialog;
	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return;
	iCommand = (ui_design_command_t)pApp->iPendingFileCommand;
	pApp->iPendingFileCommand = UI_DESIGN_COMMAND_NONE;
	if ( iResult != XUI_FILE_DIALOG_RESULT_OK || sPath == NULL || sPath[0] == '\0' ) {
		uiDesignAppSetStatus(pApp, "File command cancelled");
		uiDesignAppUpdateCommandUI(pApp);
		return;
	}
	iRet = XUI_ERROR_INVALID_ARGUMENT;
	switch ( iCommand ) {
	case UI_DESIGN_COMMAND_FILE_OPEN:
		iRet = __uiDesignAppOpenDocument(pApp, sPath);
		break;
	case UI_DESIGN_COMMAND_FILE_SAVE:
	case UI_DESIGN_COMMAND_FILE_SAVE_AS:
		iRet = __uiDesignAppSaveDocument(pApp, sPath);
		break;
	case UI_DESIGN_COMMAND_FILE_EXPORT:
		iRet = __uiDesignAppExportDocument(pApp, sPath);
		break;
	default:
		break;
	}
	if ( iRet != XUI_OK ) {
		uiDesignAppSetStatus(pApp, "File command failed");
		uiDesignAppUpdateCommandUI(pApp);
	}
}

static int __uiDesignAppShowFileDialog(ui_design_app_t* pApp, ui_design_command_t iCommand)
{
	xui_file_dialog_desc_t tDesc;
	const char* sFilter;
	const char* sTitle;
	const char* sFileName;
	int iRet;

	if ( (pApp == NULL) || (pApp->pContext == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pFileDialog != NULL ) {
		xuiFileDialogDestroy(pApp->pFileDialog);
		pApp->pFileDialog = NULL;
	}
	sTitle = "Open XUI Design";
	sFileName = "uidesign.json";
	sFilter = "JSON Files (*.json)|*.json|All Files (*.*)|*.*";
	if ( iCommand == UI_DESIGN_COMMAND_FILE_SAVE || iCommand == UI_DESIGN_COMMAND_FILE_SAVE_AS ) {
		sTitle = "Save XUI Design";
	} else if ( iCommand == UI_DESIGN_COMMAND_FILE_EXPORT ) {
		sTitle = "Export XUI Design";
		sFileName = "uidesign_export.json";
		sFilter = "JSON Files (*.json)|*.json|All Files (*.*)|*.*";
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = sTitle;
	tDesc.sInitialDir = "";
	tDesc.sFileName = sFileName;
	tDesc.sFilter = sFilter;
	tDesc.pFont = pApp->pFont;
	tDesc.onResult = __uiDesignFileDialogResult;
	tDesc.pResultUser = pApp;
	tDesc.bModal = 1;
	tDesc.fWidth = 760.0f;
	tDesc.fHeight = 540.0f;
	pApp->iPendingFileCommand = iCommand;
	if ( iCommand == UI_DESIGN_COMMAND_FILE_OPEN ) {
		iRet = xuiOpenFileDialog(pApp->pContext, &pApp->pFileDialog, &tDesc);
	} else {
		iRet = xuiSaveFileDialog(pApp->pContext, &pApp->pFileDialog, &tDesc);
	}
	if ( iRet != XUI_OK ) pApp->iPendingFileCommand = UI_DESIGN_COMMAND_NONE;
	return iRet;
}

static int __uiDesignAppDockWindowVisible(ui_design_app_t* pApp, int iWindow)
{
	xui_dock_window_info_t tInfo;

	if ( (pApp == NULL) || (pApp->pDock == NULL) || (iWindow < 0) ) return 0;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	if ( xuiDockPanelGetWindowInfo(pApp->pDock, iWindow, &tInfo) != XUI_OK ) return 0;
	return tInfo.iState != XUI_DOCK_PANEL_WINDOW_HIDDEN;
}

static int __uiDesignAppSetDockWindowVisible(ui_design_app_t* pApp, int iWindow, int bVisible, int iSide, float fRatio)
{
	xui_dock_window_info_t tInfo;
	int iRet;

	if ( (pApp == NULL) || (pApp->pDock == NULL) || (iWindow < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	iRet = xuiDockPanelGetWindowInfo(pApp->pDock, iWindow, &tInfo);
	if ( iRet != XUI_OK ) return iRet;
	if ( bVisible ) {
		if ( tInfo.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN ) {
			iRet = xuiDockPanelDockWindow(pApp->pDock, iWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, iSide, fRatio, NULL);
		}
	} else if ( tInfo.iState != XUI_DOCK_PANEL_WINDOW_HIDDEN ) {
		iRet = xuiDockPanelHideWindow(pApp->pDock, iWindow);
	}
	if ( iRet == XUI_OK ) uiDesignAppInvalidate(pApp);
	return iRet;
}

static int __uiDesignAppToggleDockWindow(ui_design_app_t* pApp, int iWindow, int iSide, float fRatio, const char* sName)
{
	int bShow;
	int iRet;
	char sStatus[96];

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bShow = !__uiDesignAppDockWindowVisible(pApp, iWindow);
	iRet = __uiDesignAppSetDockWindowVisible(pApp, iWindow, bShow, iSide, fRatio);
	if ( iRet == XUI_OK ) {
		snprintf(sStatus, sizeof(sStatus), "%s %s", (sName != NULL) ? sName : "Window", bShow ? "shown" : "hidden");
		uiDesignAppSetStatus(pApp, sStatus);
	}
	return iRet;
}

static void __uiDesignAppRefreshMenus(ui_design_app_t* pApp);

int uiDesignAppExecuteCommand(ui_design_app_t* pApp, ui_design_command_t iCommand)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !uiDesignAppCanExecuteCommand(pApp, iCommand) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = XUI_OK;
	switch ( iCommand ) {
	case UI_DESIGN_COMMAND_EDIT_UNDO:
		return __uiDesignAppExecuteUndo(pApp);
	case UI_DESIGN_COMMAND_EDIT_REDO:
		return __uiDesignAppExecuteRedo(pApp);
	case UI_DESIGN_COMMAND_EDIT_COPY:
		iRet = uiDesignAppCopySelection(pApp);
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Copied");
		break;
	case UI_DESIGN_COMMAND_FILE_NEW:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "New Document");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "New document");
		break;
	case UI_DESIGN_COMMAND_EDIT_CUT:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "Cut");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Cut");
		break;
	case UI_DESIGN_COMMAND_EDIT_PASTE:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "Paste");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Pasted");
		break;
	case UI_DESIGN_COMMAND_EDIT_DUPLICATE:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "Duplicate");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Duplicated");
		break;
	case UI_DESIGN_COMMAND_EDIT_DELETE:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "Delete");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Deleted");
		break;
	case UI_DESIGN_COMMAND_EDIT_SELECT_ALL:
		(void)uiDesignModelClearSelection(&pApp->tModel);
		for ( iRet = 0; iRet < pApp->tModel.iNodeCount; ++iRet ) (void)uiDesignModelAddSelection(&pApp->tModel, pApp->tModel.arrNodes[iRet].iId);
		(void)__uiDesignAppRefreshInspector(pApp);
		uiDesignAppSetStatus(pApp, "Selected all");
		iRet = XUI_OK;
		break;
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE:
	case UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM:
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H:
	case UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V:
	case UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD:
	case UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD:
		iRet = __uiDesignAppExecuteUndoableCommand(pApp, iCommand, "Arrange");
		if ( iRet == XUI_OK ) uiDesignAppSetStatus(pApp, "Arranged");
		break;
	case UI_DESIGN_COMMAND_FILE_SAVE:
		if ( pApp->sDocumentPath[0] == '\0' ) {
			iRet = __uiDesignAppShowFileDialog(pApp, UI_DESIGN_COMMAND_FILE_SAVE);
		} else {
			iRet = __uiDesignAppSaveDocument(pApp, pApp->sDocumentPath);
		}
		break;
	case UI_DESIGN_COMMAND_FILE_SAVE_AS:
		iRet = __uiDesignAppShowFileDialog(pApp, UI_DESIGN_COMMAND_FILE_SAVE_AS);
		break;
	case UI_DESIGN_COMMAND_FILE_OPEN:
		iRet = __uiDesignAppShowFileDialog(pApp, UI_DESIGN_COMMAND_FILE_OPEN);
		break;
	case UI_DESIGN_COMMAND_FILE_EXPORT:
		iRet = __uiDesignAppShowFileDialog(pApp, UI_DESIGN_COMMAND_FILE_EXPORT);
		break;
	case UI_DESIGN_COMMAND_FILE_EXIT:
		xgeQuit();
		break;
	case UI_DESIGN_COMMAND_VIEW_GRID:
		pApp->bGridVisible = !pApp->bGridVisible;
		uiDesignAppSetStatus(pApp, pApp->bGridVisible ? "Grid on" : "Grid off");
		break;
	case UI_DESIGN_COMMAND_VIEW_SNAP:
		pApp->bSnapEnabled = !pApp->bSnapEnabled;
		uiDesignAppSetStatus(pApp, pApp->bSnapEnabled ? "Snap on" : "Snap off");
		break;
	case UI_DESIGN_COMMAND_VIEW_MARQUEE_CONTAIN:
		pApp->bMarqueeSelectContain = 1;
		uiDesignAppSetStatus(pApp, "Marquee: enclosed");
		break;
	case UI_DESIGN_COMMAND_VIEW_MARQUEE_TOUCH:
		pApp->bMarqueeSelectContain = 0;
		uiDesignAppSetStatus(pApp, "Marquee: touching");
		break;
	case UI_DESIGN_COMMAND_VIEW_ZOOM_IN:
		pApp->fZoom *= 1.10f;
		uiDesignAppSetStatus(pApp, "Zoom in");
		break;
	case UI_DESIGN_COMMAND_VIEW_ZOOM_OUT:
		pApp->fZoom /= 1.10f;
		if ( pApp->fZoom < 0.25f ) pApp->fZoom = 0.25f;
		uiDesignAppSetStatus(pApp, "Zoom out");
		break;
	case UI_DESIGN_COMMAND_VIEW_ZOOM_FIT:
	case UI_DESIGN_COMMAND_VIEW_ZOOM_100:
		pApp->fZoom = 1.0f;
		uiDesignAppSetStatus(pApp, "Zoom 100%");
		break;
	case UI_DESIGN_COMMAND_TOOL_POINTER:
		pApp->iActiveTool = UI_DESIGN_NODE_NONE;
		uiDesignAppSetStatus(pApp, "Pointer");
		break;
	case UI_DESIGN_COMMAND_TOOL_WIDGET: pApp->iActiveTool = UI_DESIGN_NODE_WIDGET; break;
	case UI_DESIGN_COMMAND_TOOL_LABEL: pApp->iActiveTool = UI_DESIGN_NODE_LABEL; break;
	case UI_DESIGN_COMMAND_TOOL_BUTTON: pApp->iActiveTool = UI_DESIGN_NODE_BUTTON; break;
	case UI_DESIGN_COMMAND_TOOL_CHECKBOX: pApp->iActiveTool = UI_DESIGN_NODE_CHECKBOX; break;
	case UI_DESIGN_COMMAND_TOOL_RADIO: pApp->iActiveTool = UI_DESIGN_NODE_RADIO; break;
	case UI_DESIGN_COMMAND_TOOL_INPUT: pApp->iActiveTool = UI_DESIGN_NODE_INPUT; break;
	case UI_DESIGN_COMMAND_TOOL_TEXT_EDIT: pApp->iActiveTool = UI_DESIGN_NODE_TEXT_EDIT; break;
	case UI_DESIGN_COMMAND_PREVIEW:
		iRet = __uiDesignAppLaunchPreview(pApp);
		if ( iRet != XUI_OK ) uiDesignAppSetStatus(pApp, "Preview failed");
		break;
	case UI_DESIGN_COMMAND_VIEW_TOOLBOX:
		iRet = __uiDesignAppToggleDockWindow(pApp, pApp->iToolboxWindow, XUI_DOCK_PANEL_SIDE_LEFT, 0.20f, "Toolbox");
		break;
	case UI_DESIGN_COMMAND_VIEW_INSPECTOR:
		iRet = __uiDesignAppToggleDockWindow(pApp, pApp->iInspectorWindow, XUI_DOCK_PANEL_SIDE_RIGHT, 0.28f, "Inspector");
		break;
	case UI_DESIGN_COMMAND_HELP_ABOUT:
		uiDesignAppSetStatus(pApp, "XUI UI Design");
		break;
	case UI_DESIGN_COMMAND_HELP_SHORTCUTS:
		uiDesignAppSetStatus(pApp, "Ctrl+Z/Y, Ctrl+C/X/V, Del, Ctrl+A");
		break;
	default:
		break;
	}
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return iRet;
}

void uiDesignAppUpdateCommandUI(ui_design_app_t* pApp)
{
	int i;
	const xui_toolbar_item_t* pItem;
	ui_design_command_t iCommand;

	if ( pApp == NULL ) return;
	__uiDesignAppRefreshMenus(pApp);
	if ( pApp->pToolbar != NULL ) {
		for ( i = 0; i < xuiToolbarGetItemCount(pApp->pToolbar); ++i ) {
			pItem = xuiToolbarGetItem(pApp->pToolbar, i);
			if ( pItem == NULL || pItem->iType == XUI_TOOLBAR_ITEM_SEPARATOR ) continue;
			iCommand = (ui_design_command_t)pItem->iValue;
			(void)xuiToolbarSetItemEnabled(pApp->pToolbar, i, uiDesignAppCanExecuteCommand(pApp, iCommand));
			if ( iCommand == UI_DESIGN_COMMAND_VIEW_GRID ) (void)xuiToolbarSetItemChecked(pApp->pToolbar, i, pApp->bGridVisible);
			if ( iCommand == UI_DESIGN_COMMAND_VIEW_SNAP ) (void)xuiToolbarSetItemChecked(pApp->pToolbar, i, pApp->bSnapEnabled);
			if ( iCommand == UI_DESIGN_COMMAND_PREVIEW ) (void)xuiToolbarSetItemChecked(pApp->pToolbar, i, pApp->bPreviewMode);
			if ( iCommand == UI_DESIGN_COMMAND_TOOL_POINTER ) (void)xuiToolbarSetItemChecked(pApp->pToolbar, i, pApp->iActiveTool == UI_DESIGN_NODE_NONE);
		}
	}
	__uiDesignAppUpdateStatusBar(pApp);
}

static uint32_t __uiDesignCommandMenuState(ui_design_app_t* pApp, ui_design_command_t iCommand)
{
	uint32_t iState;

	iState = uiDesignAppCanExecuteCommand(pApp, iCommand) ? XUI_MENU_ITEM_ENABLED : 0u;
	switch ( iCommand ) {
	case UI_DESIGN_COMMAND_VIEW_GRID:
		if ( pApp != NULL && pApp->bGridVisible ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_VIEW_SNAP:
		if ( pApp != NULL && pApp->bSnapEnabled ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_VIEW_MARQUEE_CONTAIN:
		if ( pApp != NULL && pApp->bMarqueeSelectContain ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_VIEW_MARQUEE_TOUCH:
		if ( pApp != NULL && !pApp->bMarqueeSelectContain ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_VIEW_TOOLBOX:
		if ( pApp != NULL && __uiDesignAppDockWindowVisible(pApp, pApp->iToolboxWindow) ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_VIEW_INSPECTOR:
		if ( pApp != NULL && __uiDesignAppDockWindowVisible(pApp, pApp->iInspectorWindow) ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	case UI_DESIGN_COMMAND_PREVIEW:
		if ( pApp != NULL && pApp->bPreviewMode ) iState |= XUI_MENU_ITEM_CHECKED;
		break;
	default:
		break;
	}
	return iState;
}

static void __uiDesignMenuItem(ui_design_app_t* pApp, xui_menu_item_t* pItem, const char* sText, const char* sShortcut, int iType, ui_design_command_t iCommand)
{
	if ( pItem == NULL ) return;
	memset(pItem, 0, sizeof(*pItem));
	pItem->sText = sText;
	pItem->sShortcut = sShortcut;
	pItem->iType = iType;
	pItem->iValue = iCommand;
	if ( iType == XUI_MENU_ITEM_SEPARATOR ) {
		pItem->iState = 0u;
	} else {
		pItem->iState = __uiDesignCommandMenuState(pApp, iCommand);
	}
}

static void __uiDesignAppRefreshMenus(ui_design_app_t* pApp)
{
	xui_menu_item_t arrItems[16];
	int n;

	if ( pApp == NULL ) return;
	if ( pApp->pFileMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "New", "Ctrl+N", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_NEW);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Open", "Ctrl+O", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_OPEN);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Save", "Ctrl+S", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_SAVE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Save As", "Ctrl+Shift+S", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_SAVE_AS);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Export", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_EXPORT);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Exit", "Esc", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_FILE_EXIT);
		(void)xuiMenuSetItems(pApp->pFileMenu, arrItems, n);
	}
	if ( pApp->pEditMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "Undo", "Ctrl+Z", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_UNDO);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Redo", "Ctrl+Y", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_REDO);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Cut", "Ctrl+X", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_CUT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Copy", "Ctrl+C", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_COPY);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Paste", "Ctrl+V", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_PASTE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Duplicate", "Ctrl+D", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_DUPLICATE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Delete", "Del", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_DELETE);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Select All", "Ctrl+A", XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_EDIT_SELECT_ALL);
		(void)xuiMenuSetItems(pApp->pEditMenu, arrItems, n);
	}
	if ( pApp->pArrangeMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Left", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Center", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Right", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Top", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Middle", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Align Bottom", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Distribute H", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Distribute V", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Bring Forward", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Send Backward", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD);
		(void)xuiMenuSetItems(pApp->pArrangeMenu, arrItems, n);
	}
	if ( pApp->pViewMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "Toolbox", NULL, XUI_MENU_ITEM_CHECK, UI_DESIGN_COMMAND_VIEW_TOOLBOX);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Inspector", NULL, XUI_MENU_ITEM_CHECK, UI_DESIGN_COMMAND_VIEW_INSPECTOR);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Grid", NULL, XUI_MENU_ITEM_CHECK, UI_DESIGN_COMMAND_VIEW_GRID);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Snap", NULL, XUI_MENU_ITEM_CHECK, UI_DESIGN_COMMAND_VIEW_SNAP);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Marquee: Enclosed", NULL, XUI_MENU_ITEM_RADIO, UI_DESIGN_COMMAND_VIEW_MARQUEE_CONTAIN);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Marquee: Touching", NULL, XUI_MENU_ITEM_RADIO, UI_DESIGN_COMMAND_VIEW_MARQUEE_TOUCH);
		__uiDesignMenuItem(pApp, &arrItems[n++], NULL, NULL, XUI_MENU_ITEM_SEPARATOR, UI_DESIGN_COMMAND_NONE);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Zoom In", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_VIEW_ZOOM_IN);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Zoom Out", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_VIEW_ZOOM_OUT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Zoom Fit", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_VIEW_ZOOM_FIT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "100%", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_VIEW_ZOOM_100);
		(void)xuiMenuSetItems(pApp->pViewMenu, arrItems, n);
	}
	if ( pApp->pToolsMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "Pointer", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_POINTER);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Widget", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_WIDGET);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Label", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_LABEL);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Button", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_BUTTON);
		__uiDesignMenuItem(pApp, &arrItems[n++], "CheckBox", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_CHECKBOX);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Radio", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_RADIO);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Input", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_INPUT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Text Edit", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_TOOL_TEXT_EDIT);
		(void)xuiMenuSetItems(pApp->pToolsMenu, arrItems, n);
	}
	if ( pApp->pHelpMenu != NULL ) {
		n = 0;
		__uiDesignMenuItem(pApp, &arrItems[n++], "About", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_HELP_ABOUT);
		__uiDesignMenuItem(pApp, &arrItems[n++], "Shortcuts", NULL, XUI_MENU_ITEM_NORMAL, UI_DESIGN_COMMAND_HELP_SHORTCUTS);
		(void)xuiMenuSetItems(pApp->pHelpMenu, arrItems, n);
	}
}

static void __uiDesignCommandMenuSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	if ( iValue > 0 ) (void)uiDesignAppExecuteCommand((ui_design_app_t*)pUser, (ui_design_command_t)iValue);
}

static void __uiDesignCommandToolbarSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	if ( iValue > 0 ) (void)uiDesignAppExecuteCommand((ui_design_app_t*)pUser, (ui_design_command_t)iValue);
}

static int __uiDesignCreateMenu(ui_design_app_t* pApp, xui_widget* ppMenu)
{
	xui_menu_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (ppMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pApp->pMenuBar;
	tDesc.pFont = pApp->pFont;
	iRet = xuiMenuCreate(pApp->pContext, ppMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	return xuiMenuSetSelect(*ppMenu, __uiDesignCommandMenuSelect, pApp);
}

static int __uiDesignCreateChrome(ui_design_app_t* pApp)
{
	xui_menubar_desc_t tMenuBarDesc;
	xui_toolbar_desc_t tToolbarDesc;
	xui_statusbar_desc_t tStatusDesc;
	int iWidth;
	int iHeight;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iWidth = __uiDesignAppSurfaceWidth(pApp);
	iHeight = __uiDesignAppSurfaceHeight(pApp);
	memset(&tMenuBarDesc, 0, sizeof(tMenuBarDesc));
	tMenuBarDesc.iSize = sizeof(tMenuBarDesc);
	tMenuBarDesc.pFont = pApp->pFont;
	iRet = xuiMenuBarCreate(pApp->pContext, &pApp->pMenuBar, &tMenuBarDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0.0f, 0.0f, (float)iWidth, UI_DESIGN_MENUBAR_HEIGHT});
	(void)xuiMenuBarSetSelect(pApp->pMenuBar, __uiDesignCommandMenuSelect, pApp);
	iRet = __uiDesignCreateMenu(pApp, &pApp->pFileMenu);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateMenu(pApp, &pApp->pEditMenu);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateMenu(pApp, &pApp->pArrangeMenu);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateMenu(pApp, &pApp->pViewMenu);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateMenu(pApp, &pApp->pToolsMenu);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateMenu(pApp, &pApp->pHelpMenu);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "File", pApp->pFileMenu, 0);
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "Edit", pApp->pEditMenu, 0);
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "Arrange", pApp->pArrangeMenu, 0);
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "View", pApp->pViewMenu, 0);
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "Tools", pApp->pToolsMenu, 0);
	(void)xuiMenuBarAddItem(pApp->pMenuBar, "Help", pApp->pHelpMenu, 0);
	memset(&tToolbarDesc, 0, sizeof(tToolbarDesc));
	tToolbarDesc.iSize = sizeof(tToolbarDesc);
	tToolbarDesc.pFont = pApp->pFont;
	iRet = xuiToolbarCreate(pApp->pContext, &pApp->pToolbar, &tToolbarDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0.0f, UI_DESIGN_MENUBAR_HEIGHT, (float)iWidth, UI_DESIGN_TOOLBAR_HEIGHT});
	(void)xuiToolbarSetSelect(pApp->pToolbar, __uiDesignCommandToolbarSelect, pApp);
	(void)xuiToolbarAddItem(pApp->pToolbar, "New", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_FILE_NEW);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Open", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_FILE_OPEN);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Save", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_FILE_SAVE);
	(void)xuiToolbarAddSeparator(pApp->pToolbar);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Undo", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_EDIT_UNDO);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Redo", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_EDIT_REDO);
	(void)xuiToolbarAddSeparator(pApp->pToolbar);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Pointer", XUI_TOOLBAR_ITEM_TOGGLE, UI_DESIGN_COMMAND_TOOL_POINTER);
	(void)xuiToolbarAddSeparator(pApp->pToolbar);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Left", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Center", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Right", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Top", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Middle", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Bottom", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Dist H", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Dist V", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V);
	(void)xuiToolbarAddSeparator(pApp->pToolbar);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Forward", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Backward", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD);
	(void)xuiToolbarAddSeparator(pApp->pToolbar);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Zoom+", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_VIEW_ZOOM_IN);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Zoom-", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_VIEW_ZOOM_OUT);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Grid", XUI_TOOLBAR_ITEM_TOGGLE, UI_DESIGN_COMMAND_VIEW_GRID);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Snap", XUI_TOOLBAR_ITEM_TOGGLE, UI_DESIGN_COMMAND_VIEW_SNAP);
	(void)xuiToolbarAddItem(pApp->pToolbar, "Preview", XUI_TOOLBAR_ITEM_BUTTON, UI_DESIGN_COMMAND_PREVIEW);
	memset(&tStatusDesc, 0, sizeof(tStatusDesc));
	tStatusDesc.iSize = sizeof(tStatusDesc);
	tStatusDesc.pFont = pApp->pFont;
	iRet = xuiStatusBarCreate(pApp->pContext, &pApp->pStatusBar, &tStatusDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pApp->pStatusBar, (xui_rect_t){0.0f, (float)iHeight - UI_DESIGN_STATUSBAR_HEIGHT, (float)iWidth, UI_DESIGN_STATUSBAR_HEIGHT});
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, "Ready", 420.0f, 0, 0);
	(void)xuiStatusBarAddFlexibleSpacer(pApp->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, 1.0f);
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "0 selected", 112.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "100%", 64.0f, 0, 0);
	pApp->iStatusMessageItem = 0;
	pApp->iStatusSelectionItem = 2;
	pApp->iStatusZoomItem = 3;
	iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pMenuBar);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pToolbar);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pStatusBar);
	uiDesignAppSetStatus(pApp, "Ready");
	uiDesignAppUpdateCommandUI(pApp);
	return iRet;
}

static int __uiDesignRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tRect;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(229, 236, 245, 255));
}

static int __uiDesignPreviewRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tRect;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_WHITE);
}

static int __uiDesignCreateRoot(ui_design_app_t* pApp)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)__uiDesignAppSurfaceWidth(pApp), (float)__uiDesignAppSurfaceHeight(pApp)});
	(void)xuiWidgetSetCachePolicy(pApp->pRoot, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __uiDesignRootRender, pApp);
	return xuiSetRootWidget(pApp->pContext, pApp->pRoot);
}

static int __uiDesignCreatePreviewRoot(ui_design_app_t* pApp)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)__uiDesignAppSurfaceWidth(pApp), (float)__uiDesignAppSurfaceHeight(pApp)});
	(void)xuiWidgetSetCachePolicy(pApp->pRoot, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __uiDesignPreviewRootRender, pApp);
	iRet = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	pApp->pArtboard = pApp->pRoot;
	return XUI_OK;
}

static int __uiDesignCreateDock(ui_design_app_t* pApp)
{
	xui_dock_panel_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pApp->pFont;
	iRet = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){8.0f, UI_DESIGN_MENUBAR_HEIGHT + UI_DESIGN_TOOLBAR_HEIGHT + UI_DESIGN_CHROME_GAP, (float)__uiDesignAppSurfaceWidth(pApp) - 16.0f, (float)__uiDesignAppSurfaceHeight(pApp) - UI_DESIGN_MENUBAR_HEIGHT - UI_DESIGN_TOOLBAR_HEIGHT - UI_DESIGN_STATUSBAR_HEIGHT - UI_DESIGN_CHROME_GAP * 2.0f});
	iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pDock);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignToolboxCreate(pApp);
	if ( iRet == XUI_OK ) iRet = uiDesignCanvasCreate(pApp);
	if ( iRet == XUI_OK ) iRet = uiDesignInspectorCreate(pApp);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDockPanelAddWindow(pApp->pDock, "Toolbox", pApp->pToolbox, &pApp->iToolboxWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Designer", pApp->pCanvas, &pApp->iCanvasWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Inspector", pApp->pInspector, &pApp->iInspectorWindow);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iCanvasWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pApp->iCanvasPane);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iToolboxWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.20f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iInspectorWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.28f, NULL);
	if ( iRet == XUI_OK ) iRet = __uiDesignAppApplyViewportLayout(pApp, __uiDesignAppSurfaceWidth(pApp), __uiDesignAppSurfaceHeight(pApp));
	return iRet;
}

static int __uiDesignExerciseResizeLayout(ui_design_app_t* pApp)
{
	xui_rect_t tRoot;
	xui_rect_t tMenu;
	xui_rect_t tToolbar;
	xui_rect_t tStatus;
	xui_rect_t tDock;
	xui_rect_t tExpectedRoot;
	xui_rect_t tExpectedMenu;
	xui_rect_t tExpectedToolbar;
	xui_rect_t tExpectedStatus;
	xui_rect_t tExpectedDock;
	int iOldWidth;
	int iOldHeight;
	int iNewWidth;
	int iNewHeight;
	int iRet;
	int iRestore;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pRoot == NULL || pApp->pMenuBar == NULL || pApp->pToolbar == NULL || pApp->pStatusBar == NULL || pApp->pDock == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iOldWidth = __uiDesignAppSurfaceWidth(pApp);
	iOldHeight = __uiDesignAppSurfaceHeight(pApp);
	iNewWidth = iOldWidth + 160;
	iNewHeight = iOldHeight + 90;
	iRet = __uiDesignAppResizeTarget(pApp, iNewWidth, iNewHeight);
	if ( iRet != XUI_OK ) return iRet;
	tRoot = xuiWidgetGetRect(pApp->pRoot);
	tMenu = xuiWidgetGetRect(pApp->pMenuBar);
	tToolbar = xuiWidgetGetRect(pApp->pToolbar);
	tStatus = xuiWidgetGetRect(pApp->pStatusBar);
	tDock = xuiWidgetGetRect(pApp->pDock);
	tExpectedRoot = (xui_rect_t){0.0f, 0.0f, (float)iNewWidth, (float)iNewHeight};
	tExpectedMenu = (xui_rect_t){0.0f, 0.0f, (float)iNewWidth, UI_DESIGN_MENUBAR_HEIGHT};
	tExpectedToolbar = (xui_rect_t){0.0f, UI_DESIGN_MENUBAR_HEIGHT, (float)iNewWidth, UI_DESIGN_TOOLBAR_HEIGHT};
	tExpectedStatus = (xui_rect_t){0.0f, (float)iNewHeight - UI_DESIGN_STATUSBAR_HEIGHT, (float)iNewWidth, UI_DESIGN_STATUSBAR_HEIGHT};
	tExpectedDock = (xui_rect_t){8.0f, UI_DESIGN_MENUBAR_HEIGHT + UI_DESIGN_TOOLBAR_HEIGHT + UI_DESIGN_CHROME_GAP, (float)iNewWidth - 16.0f, (float)iNewHeight - UI_DESIGN_MENUBAR_HEIGHT - UI_DESIGN_TOOLBAR_HEIGHT - UI_DESIGN_STATUSBAR_HEIGHT - UI_DESIGN_CHROME_GAP * 2.0f};
	if ( !__uiDesignAppRectAlmostEqual(tRoot, tExpectedRoot, 0.01f) ||
	     !__uiDesignAppRectAlmostEqual(tMenu, tExpectedMenu, 0.01f) ||
	     !__uiDesignAppRectAlmostEqual(tToolbar, tExpectedToolbar, 0.01f) ||
	     !__uiDesignAppRectAlmostEqual(tStatus, tExpectedStatus, 0.01f) ||
	     !__uiDesignAppRectAlmostEqual(tDock, tExpectedDock, 0.01f) ||
	     pApp->iSurfaceWidth != iNewWidth ||
	     pApp->iSurfaceHeight != iNewHeight ) {
		printf("xui_uidesign exercise-resize-layout-failed surface=%d/%d root=%.1f/%.1f/%.1f/%.1f menu=%.1f/%.1f/%.1f/%.1f toolbar=%.1f/%.1f/%.1f/%.1f status=%.1f/%.1f/%.1f/%.1f dock=%.1f/%.1f/%.1f/%.1f\n",
			pApp->iSurfaceWidth, pApp->iSurfaceHeight,
			tRoot.fX, tRoot.fY, tRoot.fW, tRoot.fH,
			tMenu.fX, tMenu.fY, tMenu.fW, tMenu.fH,
			tToolbar.fX, tToolbar.fY, tToolbar.fW, tToolbar.fH,
			tStatus.fX, tStatus.fY, tStatus.fW, tStatus.fH,
			tDock.fX, tDock.fY, tDock.fW, tDock.fH);
		iRet = XUI_ERROR;
	}
	iRestore = __uiDesignAppResizeTarget(pApp, iOldWidth, iOldHeight);
	if ( iRet == XUI_OK ) iRet = iRestore;
	return iRet;
}

static const char* __uiDesignExerciseRuntimeTextAt(const ui_design_node_t* pNode, int iIndex)
{
	if ( (pNode == NULL) || (iIndex <= 0) || (iIndex > pNode->iRuntimeTextCount) ) return "";
	return pNode->arrRuntimeText[iIndex - 1];
}

static xui_widget __uiDesignExerciseChildAt(xui_widget pParent, int iIndex)
{
	xui_widget pChild;
	int i;

	if ( (pParent == NULL) || (iIndex < 0) ) return NULL;
	pChild = xuiWidgetGetFirstChild(pParent);
	for ( i = 0; (pChild != NULL) && (i < iIndex); ++i ) {
		pChild = xuiWidgetGetNextSibling(pChild);
	}
	return pChild;
}

static const char* __uiDesignExerciseWidgetText(ui_design_app_t* pApp, xui_widget pWidget)
{
	xui_widget_type pType;

	if ( (pApp == NULL) || (pApp->pContext == NULL) || (pWidget == NULL) ) return NULL;
	pType = xuiWidgetGetType(pWidget);
	if ( pType == xuiLabelGetType(pApp->pContext) ) return xuiLabelGetText(pWidget);
	if ( pType == xuiButtonGetType(pApp->pContext) ) return xuiButtonGetText(pWidget);
	return NULL;
}

static xui_widget __uiDesignExerciseFindChildText(ui_design_app_t* pApp, xui_widget pParent, xui_widget_type pType, const char* sText)
{
	xui_widget pChild;
	const char* sChildText;

	if ( (pApp == NULL) || (pParent == NULL) || (sText == NULL) ) return NULL;
	for ( pChild = xuiWidgetGetFirstChild(pParent); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		if ( (pType != NULL) && (xuiWidgetGetType(pChild) != pType) ) continue;
		sChildText = __uiDesignExerciseWidgetText(pApp, pChild);
		if ( (sChildText != NULL) && (strcmp(sChildText, sText) == 0) ) return pChild;
	}
	return NULL;
}

static xui_widget __uiDesignExerciseFindPanelColor(ui_design_app_t* pApp, xui_widget pParent, uint32_t iColor)
{
	xui_widget pChild;
	xui_widget_type pPanelType;

	if ( (pApp == NULL) || (pParent == NULL) || (pApp->pContext == NULL) ) return NULL;
	pPanelType = xuiPanelGetType(pApp->pContext);
	for ( pChild = xuiWidgetGetFirstChild(pParent); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		if ( (xuiWidgetGetType(pChild) == pPanelType) && (xuiPanelGetBackgroundColor(pChild) == iColor) ) return pChild;
	}
	return NULL;
}

static int __uiDesignExerciseMarqueeSelection(ui_design_app_t* pApp)
{
	int iA;
	int iB;
	int iOldContain;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iA = 0;
	iB = 0;
	iOldContain = pApp->bMarqueeSelectContain;
	iRet = uiDesignCanvasPlaceToolRect(pApp, UI_DESIGN_NODE_LABEL, (xui_rect_t){2220.0f, 1300.0f, 40.0f, 40.0f}, &iA);
	if ( iRet != XUI_OK ) goto done;
	iRet = uiDesignCanvasPlaceToolRect(pApp, UI_DESIGN_NODE_LABEL, (xui_rect_t){2280.0f, 1300.0f, 40.0f, 40.0f}, &iB);
	if ( iRet != XUI_OK ) goto done;
	pApp->bMarqueeSelectContain = 1;
	iRet = uiDesignCanvasSelectRect(pApp, (xui_rect_t){2218.0f, 1298.0f, 44.0f, 44.0f});
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetSelectionCount(&pApp->tModel) != 1 ||
	     !uiDesignModelIsSelected(&pApp->tModel, iA) ||
	     uiDesignModelIsSelected(&pApp->tModel, iB) ) {
		printf("xui_uidesign exercise-marquee-contain-failed a=%d b=%d count=%d\n",
			iA, iB, uiDesignModelGetSelectionCount(&pApp->tModel));
		iRet = XUI_ERROR;
		goto done;
	}
	pApp->bMarqueeSelectContain = 0;
	iRet = uiDesignCanvasSelectRect(pApp, (xui_rect_t){2258.0f, 1298.0f, 24.0f, 44.0f});
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetSelectionCount(&pApp->tModel) != 2 ||
	     !uiDesignModelIsSelected(&pApp->tModel, iA) ||
	     !uiDesignModelIsSelected(&pApp->tModel, iB) ) {
		printf("xui_uidesign exercise-marquee-touch-failed a=%d b=%d count=%d\n",
			iA, iB, uiDesignModelGetSelectionCount(&pApp->tModel));
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_VIEW_MARQUEE_CONTAIN);
	if ( iRet != XUI_OK || !pApp->bMarqueeSelectContain ) {
		printf("xui_uidesign exercise-marquee-command-contain-failed ret=%d contain=%d\n", iRet, pApp->bMarqueeSelectContain);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_VIEW_MARQUEE_TOUCH);
	if ( iRet != XUI_OK || pApp->bMarqueeSelectContain ) {
		printf("xui_uidesign exercise-marquee-command-touch-failed ret=%d contain=%d\n", iRet, pApp->bMarqueeSelectContain);
		iRet = XUI_ERROR;
		goto done;
	}
done:
	pApp->bMarqueeSelectContain = iOldContain;
	if ( iA > 0 ) (void)uiDesignAppDeleteNode(pApp, iA);
	if ( iB > 0 ) (void)uiDesignAppDeleteNode(pApp, iB);
	return iRet;
}

static int __uiDesignExerciseContextCommands(ui_design_app_t* pApp, int iSourceId)
{
	ui_design_node_t* pSource;
	ui_design_node_t* pPaste;
	ui_design_node_t* pTemp;
	xui_rect_t tRect;
	int iBefore;
	int iPasteId;
	int iTempId;
	int iRet;

	if ( (pApp == NULL) || (iSourceId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pSource = uiDesignModelGetNode(&pApp->tModel, iSourceId);
	if ( pSource == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iBefore = pApp->tModel.iNodeCount;
	iRet = uiDesignAppCopyNode(pApp, iSourceId);
	if ( iRet != XUI_OK ) return iRet;
	if ( pApp->iClipboardNodeCount != 1 ) {
		printf("xui_uidesign exercise-context-copy-failed source=%d clip=%d\n", iSourceId, pApp->iClipboardNodeCount);
		return XUI_ERROR;
	}
	iPasteId = 0;
	iRet = uiDesignAppPasteClipboard(pApp, 1900.0f, 1040.0f, &iPasteId);
	if ( iRet != XUI_OK ) return iRet;
	pPaste = uiDesignModelGetNode(&pApp->tModel, iPasteId);
	if ( (pPaste == NULL) || (pPaste->pWidget == NULL) ||
	     (pApp->tModel.iNodeCount != iBefore + 1) ||
	     (pPaste->iType != pSource->iType) ||
	     (strcmp(pPaste->sText, pSource->sText) != 0) ) {
		printf("xui_uidesign exercise-context-paste-failed source=%d paste=%d before=%d now=%d\n",
			iSourceId, iPasteId, iBefore, pApp->tModel.iNodeCount);
		return XUI_ERROR;
	}
	iRet = uiDesignAppDeleteNode(pApp, iPasteId);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pApp->tModel.iNodeCount != iBefore) || (uiDesignModelGetNode(&pApp->tModel, iPasteId) != NULL) ) {
		printf("xui_uidesign exercise-context-delete-failed paste=%d before=%d now=%d\n", iPasteId, iBefore, pApp->tModel.iNodeCount);
		return XUI_ERROR;
	}
	iTempId = 0;
	iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_BUTTON, 1900.0f, 1100.0f, &iTempId);
	if ( iRet != XUI_OK ) return iRet;
	iBefore = pApp->tModel.iNodeCount;
	iRet = uiDesignAppCutNode(pApp, iTempId);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pApp->tModel.iNodeCount != iBefore - 1) ||
	     (uiDesignModelGetNode(&pApp->tModel, iTempId) != NULL) ||
	     (pApp->iClipboardNodeCount != 1) ) {
		printf("xui_uidesign exercise-context-cut-failed temp=%d before=%d now=%d clip=%d\n",
			iTempId, iBefore, pApp->tModel.iNodeCount, pApp->iClipboardNodeCount);
		return XUI_ERROR;
	}
	iBefore = pApp->tModel.iNodeCount;
	iTempId = 0;
	iRet = uiDesignCanvasPlaceToolRect(pApp, UI_DESIGN_NODE_LABEL, (xui_rect_t){1920.0f, 1160.0f, 136.0f, 48.0f}, &iTempId);
	if ( iRet != XUI_OK ) return iRet;
	pTemp = uiDesignModelGetNode(&pApp->tModel, iTempId);
	tRect = (pTemp != NULL && pTemp->pWidget != NULL) ? xuiWidgetGetRect(pTemp->pWidget) : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( (pTemp == NULL) || (pTemp->pWidget == NULL) ||
	     (pApp->tModel.iNodeCount != iBefore + 1) ||
	     (pTemp->tRect.fX != 1920.0f) || (pTemp->tRect.fY != 1160.0f) ||
	     (pTemp->tRect.fW != 136.0f) || (pTemp->tRect.fH != 48.0f) ||
	     (tRect.fX != 1920.0f) || (tRect.fY != 1160.0f) ||
	     (tRect.fW != 136.0f) || (tRect.fH != 48.0f) ) {
		printf("xui_uidesign exercise-canvas-place-rect-failed temp=%d before=%d now=%d model=%.1f/%.1f/%.1f/%.1f widget=%.1f/%.1f/%.1f/%.1f\n",
			iTempId, iBefore, pApp->tModel.iNodeCount,
			pTemp != NULL ? pTemp->tRect.fX : 0.0f,
			pTemp != NULL ? pTemp->tRect.fY : 0.0f,
			pTemp != NULL ? pTemp->tRect.fW : 0.0f,
			pTemp != NULL ? pTemp->tRect.fH : 0.0f,
			tRect.fX, tRect.fY, tRect.fW, tRect.fH);
		return XUI_ERROR;
	}
	iRet = uiDesignAppDeleteNode(pApp, iTempId);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pApp->tModel.iNodeCount != iBefore) || (uiDesignModelGetNode(&pApp->tModel, iTempId) != NULL) ) {
		printf("xui_uidesign exercise-canvas-place-delete-failed temp=%d before=%d now=%d\n", iTempId, iBefore, pApp->tModel.iNodeCount);
		return XUI_ERROR;
	}
	iBefore = pApp->tModel.iNodeCount;
	iTempId = 0;
	iRet = uiDesignCanvasPlaceToolRect(pApp, UI_DESIGN_NODE_WINDOW, (xui_rect_t){1960.0f, 1220.0f, 200.0f, 120.0f}, &iTempId);
	if ( iRet != XUI_OK ) return iRet;
	pTemp = uiDesignModelGetNode(&pApp->tModel, iTempId);
	tRect = (pTemp != NULL && pTemp->pWidget != NULL) ? xuiWidgetGetRect(pTemp->pWidget) : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( (pTemp == NULL) || (pTemp->pWidget == NULL) ||
	     (pApp->tModel.iNodeCount != iBefore + 1) ||
	     (xuiWidgetGetParent(pTemp->pWidget) != pApp->pArtboard) ||
	     (xuiWidgetGetVisible(pTemp->pWidget) == 0) ||
	     (pTemp->tRect.fX != 1960.0f) || (pTemp->tRect.fY != 1220.0f) ||
	     (pTemp->tRect.fW != 200.0f) || (pTemp->tRect.fH != 120.0f) ||
	     (tRect.fX != 1960.0f) || (tRect.fY != 1220.0f) ||
	     (tRect.fW != 200.0f) || (tRect.fH != 120.0f) ) {
		printf("xui_uidesign exercise-canvas-place-window-failed temp=%d before=%d now=%d parent=%p artboard=%p model=%.1f/%.1f/%.1f/%.1f widget=%.1f/%.1f/%.1f/%.1f visible=%d\n",
			iTempId, iBefore, pApp->tModel.iNodeCount,
			(void*)((pTemp != NULL && pTemp->pWidget != NULL) ? xuiWidgetGetParent(pTemp->pWidget) : NULL),
			(void*)pApp->pArtboard,
			pTemp != NULL ? pTemp->tRect.fX : 0.0f,
			pTemp != NULL ? pTemp->tRect.fY : 0.0f,
			pTemp != NULL ? pTemp->tRect.fW : 0.0f,
			pTemp != NULL ? pTemp->tRect.fH : 0.0f,
			tRect.fX, tRect.fY, tRect.fW, tRect.fH,
			(pTemp != NULL && pTemp->pWidget != NULL) ? xuiWidgetGetVisible(pTemp->pWidget) : -1);
		return XUI_ERROR;
	}
	iRet = uiDesignAppDeleteNode(pApp, iTempId);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pApp->tModel.iNodeCount != iBefore) || (uiDesignModelGetNode(&pApp->tModel, iTempId) != NULL) ) {
		printf("xui_uidesign exercise-canvas-place-window-delete-failed temp=%d before=%d now=%d\n", iTempId, iBefore, pApp->tModel.iNodeCount);
		return XUI_ERROR;
	}
	return __uiDesignExerciseMarqueeSelection(pApp);
}

static int __uiDesignExerciseCommandSystem(ui_design_app_t* pApp)
{
	ui_design_node_t* pA;
	ui_design_node_t* pB;
	ui_design_node_t* pC;
	xui_rect_t tA;
	xui_rect_t tB;
	xui_rect_t tC;
	char* sSnapshot;
	int iA;
	int iB;
	int iC;
	int iContainer;
	int iChild;
	int iPasted;
	int iCountBefore;
	int iUndoBefore;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sSnapshot = NULL;
	iRet = __uiDesignCaptureSnapshot(pApp, &sSnapshot);
	if ( iRet != XUI_OK ) return iRet;
	iA = iB = iC = iContainer = iChild = iPasted = 0;
	iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_BUTTON, 2160.0f, 1020.0f, &iA);
	if ( iRet == XUI_OK ) iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_BUTTON, 2280.0f, 1080.0f, &iB);
	if ( iRet == XUI_OK ) iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_BUTTON, 2420.0f, 1160.0f, &iC);
	if ( iRet != XUI_OK ) goto done;
	pA = uiDesignModelGetNode(&pApp->tModel, iA);
	pB = uiDesignModelGetNode(&pApp->tModel, iB);
	pC = uiDesignModelGetNode(&pApp->tModel, iC);
	if ( pA == NULL || pB == NULL || pC == NULL ) {
		iRet = XUI_ERROR;
		goto done;
	}
	tA = (xui_rect_t){2100.0f, 1000.0f, 90.0f, 28.0f};
	tB = (xui_rect_t){2240.0f, 1060.0f, 90.0f, 28.0f};
	tC = (xui_rect_t){2400.0f, 1140.0f, 90.0f, 28.0f};
	(void)uiDesignAppSetNodeRect(pApp, iA, tA);
	(void)uiDesignAppSetNodeRect(pApp, iB, tB);
	(void)uiDesignAppSetNodeRect(pApp, iC, tC);
	(void)uiDesignAppSelectNode(pApp, iA);
	(void)uiDesignAppAddNodeSelection(pApp, iB);
	(void)uiDesignAppAddNodeSelection(pApp, iC);
	iUndoBefore = pApp->iUndoCount;
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT);
	if ( iRet != XUI_OK ) goto done;
	pA = uiDesignModelGetNode(&pApp->tModel, iA);
	pB = uiDesignModelGetNode(&pApp->tModel, iB);
	pC = uiDesignModelGetNode(&pApp->tModel, iC);
	if ( pA == NULL || pB == NULL || pC == NULL ||
	     pB->tRect.fX != pA->tRect.fX ||
	     pC->tRect.fX != pA->tRect.fX ||
	     pApp->iUndoCount != iUndoBefore + 1 ||
	     !uiDesignAppCanExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_UNDO) ) {
		printf("xui_uidesign exercise-command-align-failed undo=%d/%d ax=%.1f bx=%.1f cx=%.1f\n",
			iUndoBefore, pApp->iUndoCount,
			pA != NULL ? pA->tRect.fX : -1.0f,
			pB != NULL ? pB->tRect.fX : -1.0f,
			pC != NULL ? pC->tRect.fX : -1.0f);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_UNDO);
	if ( iRet != XUI_OK ) goto done;
	pB = uiDesignModelGetNode(&pApp->tModel, iB);
	pC = uiDesignModelGetNode(&pApp->tModel, iC);
	if ( pB == NULL || pC == NULL || pB->tRect.fX != tB.fX || pC->tRect.fX != tC.fX ) {
		printf("xui_uidesign exercise-command-undo-failed b=%.1f c=%.1f\n",
			pB != NULL ? pB->tRect.fX : -1.0f,
			pC != NULL ? pC->tRect.fX : -1.0f);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_REDO);
	if ( iRet != XUI_OK ) goto done;
	iCountBefore = pApp->tModel.iNodeCount;
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_DUPLICATE);
	if ( iRet != XUI_OK ) goto done;
	if ( pApp->tModel.iNodeCount <= iCountBefore ) {
		printf("xui_uidesign exercise-command-duplicate-failed before=%d now=%d\n", iCountBefore, pApp->tModel.iNodeCount);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_UNDO);
	if ( iRet != XUI_OK ) goto done;
	if ( pApp->tModel.iNodeCount != iCountBefore ) {
		printf("xui_uidesign exercise-command-duplicate-undo-failed before=%d now=%d\n", iCountBefore, pApp->tModel.iNodeCount);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_DELETE);
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetNode(&pApp->tModel, iA) != NULL || uiDesignModelGetNode(&pApp->tModel, iB) != NULL || uiDesignModelGetNode(&pApp->tModel, iC) != NULL ) {
		printf("xui_uidesign exercise-command-delete-failed\n");
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_UNDO);
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetNode(&pApp->tModel, iA) == NULL || uiDesignModelGetNode(&pApp->tModel, iB) == NULL || uiDesignModelGetNode(&pApp->tModel, iC) == NULL ) {
		printf("xui_uidesign exercise-command-delete-undo-failed\n");
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_WIDGET, 2600.0f, 1000.0f, &iContainer);
	if ( iRet == XUI_OK ) iRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_LABEL, 2620.0f, 1020.0f, &iChild);
	if ( iRet != XUI_OK ) goto done;
	iRet = uiDesignAppCopyNode(pApp, iChild);
	if ( iRet != XUI_OK ) goto done;
	iUndoBefore = pApp->iUndoCount;
	iRet = uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_EDIT_PASTE, "Paste Child");
	if ( iRet == XUI_OK ) {
		iRet = uiDesignAppPasteClipboardAsChild(pApp, iContainer, &iPasted);
		if ( iRet == XUI_OK ) iRet = uiDesignAppCommitHistoryTransaction(pApp);
		else uiDesignAppCancelHistoryTransaction(pApp);
	}
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetNode(&pApp->tModel, iPasted) == NULL ||
	     uiDesignModelGetNode(&pApp->tModel, iPasted)->iParentId != iContainer ||
	     pApp->iUndoCount != iUndoBefore + 1 ) {
		printf("xui_uidesign exercise-tree-paste-child-failed container=%d pasted=%d undo=%d/%d\n",
			iContainer, iPasted, iUndoBefore, pApp->iUndoCount);
		iRet = XUI_ERROR;
		goto done;
	}
	iUndoBefore = pApp->iUndoCount;
	iRet = uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_NONE, "Promote");
	if ( iRet == XUI_OK ) {
		iRet = uiDesignAppPromoteNode(pApp, iPasted);
		if ( iRet == XUI_OK ) iRet = uiDesignAppCommitHistoryTransaction(pApp);
		else uiDesignAppCancelHistoryTransaction(pApp);
	}
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetNode(&pApp->tModel, iPasted) == NULL ||
	     uiDesignModelGetNode(&pApp->tModel, iPasted)->iParentId != 0 ||
	     pApp->iUndoCount != iUndoBefore + 1 ) {
		printf("xui_uidesign exercise-tree-promote-failed pasted=%d undo=%d/%d\n",
			iPasted, iUndoBefore, pApp->iUndoCount);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_UNDO);
	if ( iRet != XUI_OK ) goto done;
	if ( uiDesignModelGetNode(&pApp->tModel, iPasted) == NULL ||
	     uiDesignModelGetNode(&pApp->tModel, iPasted)->iParentId != iContainer ) {
		printf("xui_uidesign exercise-tree-promote-undo-failed pasted=%d container=%d\n", iPasted, iContainer);
		iRet = XUI_ERROR;
		goto done;
	}
	iRet = XUI_OK;
done:
	if ( sSnapshot != NULL ) {
		int iRestore = __uiDesignAppRestoreSnapshot(pApp, sSnapshot);
		free(sSnapshot);
		if ( iRet == XUI_OK ) iRet = iRestore;
	}
	__uiDesignHistoryClear(pApp->arrUndo, &pApp->iUndoCount);
	__uiDesignHistoryClear(pApp->arrRedo, &pApp->iRedoCount);
	return iRet;
}

static const char* __uiDesignExerciseResourcePath(const char* sName)
{
	static char sPath[260];
	static const char* arrPrefixes[] = {
		"res\\",
		"..\\res\\",
		"..\\..\\res\\",
		"..\\..\\..\\res\\"
	};
	FILE* pFile;
	int i;

	if ( (sName == NULL) || (sName[0] == '\0') ) return "";
	for ( i = 0; i < (int)(sizeof(arrPrefixes) / sizeof(arrPrefixes[0])); ++i ) {
		snprintf(sPath, sizeof(sPath), "%s%s", arrPrefixes[i], sName);
		pFile = fopen(sPath, "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return sPath;
		}
	}
	return sName;
}

static int __uiDesignSeedExercise(ui_design_app_t* pApp)
{
	const ui_design_control_desc_t* pDesc;
	int iWidget;
	int iLabel;
	int iButton;
	int iCheck;
	int iHyperlink;
	int iRadio;
	int iToggle;
	int iScrollBar;
	int iSlider;
	int iRangeSlider;
	int iInput;
	int iNumericInput;
	int iTagInput;
	int iTextEdit;
	int iPanel;
	int iCarousel;
	int iSplit;
	int iTabs;
	int iAccordion;
	int iDockPanel;
	int iPopup;
	int iRadioGroup;
	int iRadioGroupGenerated;
	int iScrollFrame;
	int iScrollView;
	int iListView;
	int iTreeView;
	int iSeparator;
	int iProgress;
	int iVirtualJoystick;
	int iCanvas;
	int iComboBox;
	int iPage;
	int iStepBar;
	int iBreadcrumb;
	int iCheckCard;
	int iChart;
	int iInventoryGrid;
	int iMessageList;
	int iTimelineView;
	int iTerminal;
	int iPropertyGrid;
	int iWindow;
	int iTableView;
	int iTableGrid;
	int iQrCode;
	int iImage;
	int iCascader;
	int iColorPicker;
	int iDatePicker;
	int iCodeEdit;
	int iMenuBar;
	int iToolbar;
	int iStatusBar;
	int iMenu;
	int iMsgBox;
	int iFileDialog;
	int iMsgTip;
	int iToast;
	int iFlowGraph;
	int iWorkflow;
	int iOverlayPanel;
	int iTablePanel;
	int iDockLayoutPanel;
	int iNested;
	int iPanelLayoutChild;
	int iCarouselChild;
	int iSplitChild;
	int iTabsChild;
	int iAccordionChild;
	int iDockPanelChild;
	int iPopupChild;
	int iRadioGroupRadioChild;
	int iRadioGroupCardChild;
	int iOverlayDropChild;
	int iTableDropChild;
	int iDockLayoutDropChild;
	int iId;
	int i;
	int iCol;
	int iRow;
	int iCreateRet;
	char sLayout[16];
	float fX;
	float fY;
	float fA;
	float fB;
	xtime tStart;
	xtime tEnd;
	xtime tMin;
	xtime tMax;
	int bHasMin;
	int bHasMax;

	if ( (pApp == NULL) || pApp->bExerciseSeeded ) return XUI_OK;
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_WIDGET, 0, 120.0f, 90.0f, &iWidget);
	(void)uiDesignModelSetRect(&pApp->tModel, iWidget, (xui_rect_t){120.0f, 90.0f, 260.0f, 170.0f});
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iWidget, 28.0f, 44.0f, &iButton);
	(void)uiDesignModelSetText(&pApp->tModel, iButton, "Run");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipText", "Run script");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipAnchor", "4");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipFollowCursor", "true");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipDelay", "0.1");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipOffsetX", "10");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipOffsetY", "14");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tabStop", "true");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tabIndex", "2");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.minWidth", "80");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.maxWidth", "240");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.tableCellRow", "0");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.tableCellColumn", "0");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "style.classes", "primary accent");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "style.inlineProperties", "button.normal_color|color|#D9EAFE|render");
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_CHECKBOX, iWidget, 28.0f, 92.0f, &iCheck);
	(void)uiDesignModelSetText(&pApp->tModel, iCheck, "Enabled");
	(void)uiDesignModelSetChecked(&pApp->tModel, iCheck, 1);
	iLabel = 0;
	iHyperlink = 0;
	iRadio = 0;
	iToggle = 0;
	iScrollBar = 0;
	iSlider = 0;
	iRangeSlider = 0;
	iInput = 0;
	iNumericInput = 0;
	iTagInput = 0;
	iTextEdit = 0;
	iPanel = 0;
	iCarousel = 0;
	iSplit = 0;
	iTabs = 0;
	iAccordion = 0;
	iDockPanel = 0;
	iPopup = 0;
	iRadioGroup = 0;
	iRadioGroupGenerated = 0;
	iScrollFrame = 0;
	iScrollView = 0;
	iListView = 0;
	iTreeView = 0;
	iSeparator = 0;
	iProgress = 0;
	iVirtualJoystick = 0;
	iCanvas = 0;
	iComboBox = 0;
	iPage = 0;
	iStepBar = 0;
	iBreadcrumb = 0;
	iCheckCard = 0;
	iChart = 0;
	iInventoryGrid = 0;
	iMessageList = 0;
	iTimelineView = 0;
	iTerminal = 0;
	iPropertyGrid = 0;
	iWindow = 0;
	iTableView = 0;
	iTableGrid = 0;
	iQrCode = 0;
	iImage = 0;
	iCascader = 0;
	iColorPicker = 0;
	iDatePicker = 0;
	iCodeEdit = 0;
	iMenuBar = 0;
	iToolbar = 0;
	iStatusBar = 0;
	iMenu = 0;
	iMsgBox = 0;
	iFileDialog = 0;
	iMsgTip = 0;
	iToast = 0;
	iFlowGraph = 0;
	iWorkflow = 0;
	iOverlayPanel = 0;
	iTablePanel = 0;
	iDockLayoutPanel = 0;
	iPanelLayoutChild = 0;
	iCarouselChild = 0;
	iSplitChild = 0;
	iTabsChild = 0;
	iAccordionChild = 0;
	iDockPanelChild = 0;
	iPopupChild = 0;
	iRadioGroupRadioChild = 0;
	iRadioGroupCardChild = 0;
	iOverlayDropChild = 0;
	iTableDropChild = 0;
	iDockLayoutDropChild = 0;
	iCol = 0;
	iRow = 0;
	for ( i = 0; i < uiDesignRegistryGetCount(); i++ ) {
		pDesc = uiDesignRegistryGetAt(i);
		if ( pDesc == NULL ) continue;
		if ( pDesc->iType == UI_DESIGN_NODE_WIDGET || pDesc->iType == UI_DESIGN_NODE_BUTTON || pDesc->iType == UI_DESIGN_NODE_CHECKBOX ) continue;
		fX = 420.0f + (float)iCol * 170.0f;
		fY = 36.0f + (float)iRow * 112.0f;
		if ( uiDesignModelAddNode(&pApp->tModel, pDesc->iType, 0, fX, fY, &iId) == XUI_OK ) {
			ui_design_node_t* pNode = uiDesignModelGetNode(&pApp->tModel, iId);
			if ( pNode != NULL ) {
				pNode->tRect.fW = pDesc->fDefaultW;
				pNode->tRect.fH = pDesc->fDefaultH;
				(void)uiDesignRegistryInitNodeProperties(pDesc, pNode);
				if ( pDesc->iType == UI_DESIGN_NODE_LABEL ) iLabel = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_HYPERLINK ) iHyperlink = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_RADIO ) iRadio = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TOGGLE ) iToggle = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SCROLLBAR ) iScrollBar = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SLIDER ) iSlider = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_RANGE_SLIDER ) iRangeSlider = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_INPUT ) iInput = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_NUMERIC_INPUT ) iNumericInput = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TAG_INPUT ) iTagInput = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TEXT_EDIT ) iTextEdit = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_PANEL ) iPanel = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CAROUSEL ) iCarousel = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SPLIT_LAYOUT ) iSplit = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TABS ) iTabs = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_ACCORDION ) iAccordion = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_DOCK_PANEL ) iDockPanel = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_POPUP ) iPopup = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_RADIO_GROUP ) iRadioGroup = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SCROLL_FRAME ) iScrollFrame = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SCROLL_VIEW ) iScrollView = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_LISTVIEW ) iListView = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TREEVIEW ) iTreeView = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SEPARATOR ) iSeparator = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_PROGRESS ) iProgress = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_VIRTUAL_JOYSTICK ) iVirtualJoystick = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CANVAS ) iCanvas = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_COMBOBOX ) iComboBox = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_PAGE ) iPage = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_STEP_BAR ) iStepBar = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_BREADCRUMB ) iBreadcrumb = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CHECK_CARD ) iCheckCard = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CHART ) iChart = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_INVENTORY_GRID ) iInventoryGrid = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_MESSAGE_LIST ) iMessageList = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TIMELINE_VIEW ) iTimelineView = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TERMINAL ) iTerminal = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_PROPERTY_GRID ) iPropertyGrid = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_WINDOW ) iWindow = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TABLEVIEW ) iTableView = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TABLEGRID ) iTableGrid = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_QRCODE ) iQrCode = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_IMAGE ) iImage = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CASCADER ) iCascader = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_COLOR_PICKER ) iColorPicker = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_DATE_PICKER ) iDatePicker = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_CODE_EDIT ) iCodeEdit = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_MENU_BAR ) iMenuBar = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TOOLBAR ) iToolbar = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_STATUS_BAR ) iStatusBar = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_MENU ) iMenu = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_MSG_BOX ) iMsgBox = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_FILE_DIALOG ) iFileDialog = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_MSG_TIP ) iMsgTip = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TOAST ) iToast = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_FLOW_GRAPH ) iFlowGraph = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_WORKFLOW ) iWorkflow = iId;
			}
		}
		iCol++;
		if ( iCol >= 3 ) {
			iCol = 0;
			iRow++;
		}
	}
	if ( iInput != 0 ) {
		(void)uiDesignModelSetText(&pApp->tModel, iInput, "Filter text");
		(void)uiDesignModelSetProperty(&pApp->tModel, iInput, "value.selectionStart", "0");
		(void)uiDesignModelSetProperty(&pApp->tModel, iInput, "value.selectionEnd", "6");
	}
	if ( iNumericInput != 0 ) {
		(void)uiDesignModelSetProperty(&pApp->tModel, iNumericInput, "behavior.textOverride", "true");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNumericInput, "text.rawText", "42.5?");
	}
	if ( iFlowGraph != 0 ) {
		(void)uiDesignModelSetProperty(&pApp->tModel, iFlowGraph, "behavior.commandHistoryLimit", "7");
	}
	if ( iWorkflow != 0 ) {
		(void)uiDesignModelSetProperty(&pApp->tModel, iWorkflow, "behavior.commandHistoryLimit", "9");
	}
	if ( iHyperlink != 0 ) {
		(void)uiDesignModelSetText(&pApp->tModel, iHyperlink, "Open Docs");
	}
	if ( iToggle != 0 ) {
		(void)uiDesignModelSetText(&pApp->tModel, iToggle, "Power");
		(void)uiDesignModelSetChecked(&pApp->tModel, iToggle, 1);
	}
	if ( iPanel != 0 ) {
		snprintf(sLayout, sizeof(sLayout), "%d", XUI_LAYOUT_ROW);
		(void)uiDesignModelSetProperty(&pApp->tModel, iPanel, "layout.type", sLayout);
		(void)uiDesignModelSetProperty(&pApp->tModel, iPanel, "layout.gap", "8");
		if ( uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_LABEL, iPanel, 8.0f, 8.0f, &iNested) == XUI_OK ) {
			iPanelLayoutChild = iNested;
			(void)uiDesignModelSetText(&pApp->tModel, iNested, "Panel layout child");
		}
	}
	if ( iCarousel != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_LABEL, iCarousel, 14.0f, 18.0f, &iNested) == XUI_OK ) {
		iCarouselChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Carousel child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.carouselPage", "0");
	}
	if ( iSplit != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_LABEL, iSplit, 12.0f, 12.0f, &iNested) == XUI_OK ) {
		iSplitChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Split child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.splitPane", "1");
	}
	if ( iTabs != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iTabs, 18.0f, 18.0f, &iNested) == XUI_OK ) {
		iTabsChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Tab child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.tabPage", "1");
	}
	if ( iAccordion != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_CHECKBOX, iAccordion, 12.0f, 8.0f, &iNested) == XUI_OK ) {
		iAccordionChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Section child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.accordionSection", "1");
	}
	if ( iDockPanel != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iDockPanel, 18.0f, 18.0f, &iNested) == XUI_OK ) {
		iDockPanelChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Dock child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.dockWindow", "0");
	}
	if ( iPopup != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iPopup, 18.0f, 18.0f, &iNested) == XUI_OK ) {
		iPopupChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Popup child");
	}
	if ( iRadioGroup != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_RADIO, iRadioGroup, 0.0f, 0.0f, &iNested) == XUI_OK ) {
		iRadioGroupRadioChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Grouped radio");
		(void)uiDesignModelSetChecked(&pApp->tModel, iNested, 1);
	}
	if ( iRadioGroup != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_CHECK_CARD, iRadioGroup, 0.0f, 34.0f, &iNested) == XUI_OK ) {
		iRadioGroupCardChild = iNested;
		(void)uiDesignModelSetChecked(&pApp->tModel, iNested, 0);
	}
	if ( uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_RADIO_GROUP, 0, 1280.0f, 520.0f, &iRadioGroupGenerated) == XUI_OK ) {
		ui_design_node_t* pGeneratedGroup = uiDesignModelGetNode(&pApp->tModel, iRadioGroupGenerated);
		const ui_design_control_desc_t* pRadioGroupDesc = uiDesignRegistryFind(UI_DESIGN_NODE_RADIO_GROUP);
		if ( pGeneratedGroup != NULL ) {
			pGeneratedGroup->tRect.fW = (pRadioGroupDesc != NULL) ? pRadioGroupDesc->fDefaultW : 180.0f;
			pGeneratedGroup->tRect.fH = (pRadioGroupDesc != NULL) ? pRadioGroupDesc->fDefaultH : 96.0f;
			if ( pRadioGroupDesc != NULL ) (void)uiDesignRegistryInitNodeProperties(pRadioGroupDesc, pGeneratedGroup);
		}
	} else {
		iRadioGroupGenerated = 0;
	}
	if ( uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_PANEL, 0, 1080.0f, 36.0f, &iOverlayPanel) == XUI_OK ) {
		ui_design_node_t* pOverlay = uiDesignModelGetNode(&pApp->tModel, iOverlayPanel);
		const ui_design_control_desc_t* pPanelDesc = uiDesignRegistryFind(UI_DESIGN_NODE_PANEL);
		if ( pOverlay != NULL ) {
			pOverlay->tRect.fW = 220.0f;
			pOverlay->tRect.fH = 130.0f;
			if ( pPanelDesc != NULL ) (void)uiDesignRegistryInitNodeProperties(pPanelDesc, pOverlay);
			snprintf(sLayout, sizeof(sLayout), "%d", XUI_LAYOUT_OVERLAY);
			(void)uiDesignModelSetProperty(&pApp->tModel, iOverlayPanel, "layout.type", sLayout);
		}
	}
	if ( uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_PANEL, 0, 1080.0f, 200.0f, &iTablePanel) == XUI_OK ) {
		ui_design_node_t* pTable = uiDesignModelGetNode(&pApp->tModel, iTablePanel);
		const ui_design_control_desc_t* pPanelDesc = uiDesignRegistryFind(UI_DESIGN_NODE_PANEL);
		if ( pTable != NULL ) {
			pTable->tRect.fW = 240.0f;
			pTable->tRect.fH = 150.0f;
			if ( pPanelDesc != NULL ) (void)uiDesignRegistryInitNodeProperties(pPanelDesc, pTable);
			snprintf(sLayout, sizeof(sLayout), "%d", XUI_LAYOUT_TABLE);
			(void)uiDesignModelSetProperty(&pApp->tModel, iTablePanel, "layout.type", sLayout);
			(void)uiDesignModelSetProperty(&pApp->tModel, iTablePanel, "layout.tableRows", "2");
			(void)uiDesignModelSetProperty(&pApp->tModel, iTablePanel, "layout.tableColumns", "3");
		}
	}
	if ( uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_PANEL, 0, 1080.0f, 380.0f, &iDockLayoutPanel) == XUI_OK ) {
		ui_design_node_t* pDock = uiDesignModelGetNode(&pApp->tModel, iDockLayoutPanel);
		const ui_design_control_desc_t* pPanelDesc = uiDesignRegistryFind(UI_DESIGN_NODE_PANEL);
		if ( pDock != NULL ) {
			pDock->tRect.fW = 240.0f;
			pDock->tRect.fH = 150.0f;
			if ( pPanelDesc != NULL ) (void)uiDesignRegistryInitNodeProperties(pPanelDesc, pDock);
			snprintf(sLayout, sizeof(sLayout), "%d", XUI_LAYOUT_DOCK);
			(void)uiDesignModelSetProperty(&pApp->tModel, iDockLayoutPanel, "layout.type", sLayout);
		}
	}
	(void)uiDesignModelSetSelected(&pApp->tModel, iButton);
	for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
		pDesc = uiDesignRegistryFind(pApp->tModel.arrNodes[i].iType);
		if ( pDesc != NULL ) {
			(void)uiDesignRegistryInitNodeProperties(pDesc, &pApp->tModel.arrNodes[i]);
		}
		iCreateRet = uiDesignAppCreateNodeWidget(pApp, &pApp->tModel.arrNodes[i]);
		if ( iCreateRet != XUI_OK ) {
			printf("xui_uidesign exercise-create-failed type=%s id=%d ret=%d\n",
				uiDesignNodeTypeName(pApp->tModel.arrNodes[i].iType),
				pApp->tModel.arrNodes[i].iId,
				iCreateRet);
		}
	}
	if ( iSplit != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.paneCount", "3");
	if ( iTabs != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iTabs, "data.selected", "1");
	if ( iAccordion != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iAccordion, "behavior.mode", "1");
	if ( iWidget != 0 ) {
		ui_design_node_t* pWidgetNode = uiDesignModelGetNode(&pApp->tModel, iWidget);
		ui_design_node_t* pButtonNode = uiDesignModelGetNode(&pApp->tModel, iButton);
		ui_design_node_t* pCheckNode = uiDesignModelGetNode(&pApp->tModel, iCheck);
		xui_widget_cache_render_proc onRender = NULL;
		xui_cache_policy_t tPolicy;
		void* pRenderUser = NULL;
		xui_rect_t tWidgetRect;
		memset(&tPolicy, 0, sizeof(tPolicy));
		memset(&tWidgetRect, 0, sizeof(tWidgetRect));
		(void)uiDesignAppSetNodeProperty(pApp, iWidget, "appearance.backgroundColor", "#112233");
		(void)uiDesignAppSetNodeProperty(pApp, iWidget, "appearance.borderColor", "#445566");
		if ( pWidgetNode != NULL && pWidgetNode->pWidget != NULL ) {
			tPolicy = xuiWidgetGetCachePolicy(pWidgetNode->pWidget);
			(void)xuiWidgetGetCacheRenderCallback(pWidgetNode->pWidget, &onRender, &pRenderUser);
			tWidgetRect = xuiWidgetGetRect(pWidgetNode->pWidget);
		}
		if ( (pWidgetNode == NULL) || (pWidgetNode->pWidget == NULL) ||
		     (pButtonNode == NULL) || (pButtonNode->pWidget == NULL) ||
		     (pCheckNode == NULL) || (pCheckNode->pWidget == NULL) ||
		     (uiDesignNodeGetPropertyColor(pWidgetNode, "appearance.backgroundColor", 0u) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (uiDesignNodeGetPropertyColor(pWidgetNode, "appearance.borderColor", 0u) != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (tWidgetRect.fX != 120.0f) || (tWidgetRect.fY != 90.0f) ||
		     (tWidgetRect.fW != 260.0f) || (tWidgetRect.fH != 170.0f) ||
		     (xuiWidgetGetLayoutType(pWidgetNode->pWidget) != XUI_LAYOUT_MANUAL) ||
		     (xuiWidgetGetChildCount(pWidgetNode->pWidget) < 2) ||
		     (xuiWidgetGetParent(pButtonNode->pWidget) != pWidgetNode->pWidget) ||
		     (xuiWidgetGetParent(pCheckNode->pWidget) != pWidgetNode->pWidget) ||
		     (tPolicy.iPolicy != XUI_CACHE_POLICY_SELF) ||
		     ((tPolicy.iFlags & XUI_CACHE_CLEAR_ON_UPDATE) == 0u) ||
		     (onRender == NULL) ||
		     (pRenderUser != pWidgetNode) ) {
			printf("xui_uidesign exercise-widget-properties-failed id=%d children=%d layout=%d cache=%d flags=%u\n",
				iWidget,
				(pWidgetNode != NULL && pWidgetNode->pWidget != NULL) ? xuiWidgetGetChildCount(pWidgetNode->pWidget) : -1,
				(pWidgetNode != NULL && pWidgetNode->pWidget != NULL) ? xuiWidgetGetLayoutType(pWidgetNode->pWidget) : -1,
				tPolicy.iPolicy,
				tPolicy.iFlags);
			return XUI_ERROR;
		}
	}
	if ( iScrollFrame != 0 ) {
		ui_design_node_t* pScrollNode = uiDesignModelGetNode(&pApp->tModel, iScrollFrame);
		float fContentWidth = 0.0f;
		float fContentHeight = 0.0f;
		float fOffsetX = 0.0f;
		float fOffsetY = 0.0f;
		int iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
		int iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
		float fScrollbarSize = 0.0f;
		float fMinThumbSize = 0.0f;
		float fButtonSize = 0.0f;
		uint32_t iTrackColor = 0u;
		uint32_t iThumbColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iActiveColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonIconColor = 0u;
		uint32_t iCornerColor = 0u;
		uint32_t iGripColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.contentWidth", "640");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.contentHeight", "420");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "value.offsetX", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "value.offsetY", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.policyX", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.policyY", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.wheelAxis", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.cornerMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "behavior.contentDrag", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.wheelStep", "61");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.scrollbarSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.minThumbSize", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "metrics.buttonSize", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.backgroundColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.trackColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.thumbColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.hoverColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.activeColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.focusColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.disabledColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.buttonColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.buttonIconColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.cornerColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollFrame, "appearance.gripColor", "#A1A2A3");
		if ( (pScrollNode != NULL) && (pScrollNode->pWidget != NULL) ) {
			(void)xuiScrollFrameGetContentSize(pScrollNode->pWidget, &fContentWidth, &fContentHeight);
			(void)xuiScrollFrameGetOffset(pScrollNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiScrollFrameGetScrollbarPolicy(pScrollNode->pWidget, &iPolicyX, &iPolicyY);
			(void)xuiScrollFrameGetMetrics(pScrollNode->pWidget, &fScrollbarSize, &fMinThumbSize, &fButtonSize);
			(void)xuiScrollFrameGetColors(pScrollNode->pWidget, &iTrackColor, &iThumbColor, &iHoverColor,
				&iActiveColor, &iFocusColor, &iDisabledColor);
			(void)xuiScrollFrameGetButtonColors(pScrollNode->pWidget, &iButtonColor, &iButtonIconColor);
			(void)xuiScrollFrameGetCornerColors(pScrollNode->pWidget, &iCornerColor, &iGripColor);
		}
		if ( (pScrollNode == NULL) || (pScrollNode->pWidget == NULL) ||
		     (fContentWidth != 640.0f) || (fContentHeight != 420.0f) ||
		     (fOffsetX != 17.0f) || (fOffsetY != 23.0f) ||
		     (iPolicyX != XUI_SCROLLBAR_POLICY_ALWAYS) || (iPolicyY != XUI_SCROLLBAR_POLICY_HIDDEN) ||
		     (xuiScrollFrameGetScrollbarMode(pScrollNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (xuiScrollFrameGetWheelAxis(pScrollNode->pWidget) != XUI_WHEEL_AXIS_BOTH) ||
		     (xuiScrollFrameGetWheelStep(pScrollNode->pWidget) != 61.0f) ||
		     (xuiScrollFrameIsContentDragEnabled(pScrollNode->pWidget) == 0) ||
		     (xuiScrollFrameGetCornerMode(pScrollNode->pWidget) != XUI_SCROLL_FRAME_CORNER_GRIP) ||
		     (xuiScrollFrameGetBackgroundColor(pScrollNode->pWidget) != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (fScrollbarSize != 18.0f) || (fMinThumbSize != 31.0f) || (fButtonSize != 21.0f) ||
		     (iTrackColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iButtonIconColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (iCornerColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (iGripColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ) {
			printf("xui_uidesign exercise-scroll-properties-failed type=scroll_frame id=%d size=%.1f/%.1f offset=%.1f/%.1f policy=%d/%d\n",
				iScrollFrame,
				fContentWidth,
				fContentHeight,
				fOffsetX,
				fOffsetY,
				iPolicyX,
				iPolicyY);
			return XUI_ERROR;
		}
	}
	if ( iScrollView != 0 ) {
		ui_design_node_t* pScrollNode = uiDesignModelGetNode(&pApp->tModel, iScrollView);
		xui_widget pFrameWidget = NULL;
		float fContentWidth = 0.0f;
		float fContentHeight = 0.0f;
		float fOffsetX = 0.0f;
		float fOffsetY = 0.0f;
		int iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
		int iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
		float fScrollbarSize = 0.0f;
		float fMinThumbSize = 0.0f;
		float fButtonSize = 0.0f;
		uint32_t iTrackColor = 0u;
		uint32_t iThumbColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iActiveColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonIconColor = 0u;
		uint32_t iCornerColor = 0u;
		uint32_t iGripColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.contentWidth", "660");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.contentHeight", "440");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "value.offsetX", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "value.offsetY", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.policyX", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.policyY", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.scrollbarMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.wheelAxis", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.cornerMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "behavior.contentDrag", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.wheelStep", "73");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.scrollbarSize", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.minThumbSize", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "metrics.buttonSize", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.backgroundColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.trackColor", "#141516");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.thumbColor", "#242526");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.hoverColor", "#343536");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.activeColor", "#444546");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.focusColor", "#545556");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.disabledColor", "#646566");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.buttonColor", "#747576");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.buttonIconColor", "#848586");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.cornerColor", "#949596");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollView, "appearance.gripColor", "#A4A5A6");
		if ( (pScrollNode != NULL) && (pScrollNode->pWidget != NULL) ) {
			pFrameWidget = xuiScrollViewGetFrameWidget(pScrollNode->pWidget);
			(void)xuiScrollViewGetContentSize(pScrollNode->pWidget, &fContentWidth, &fContentHeight);
			(void)xuiScrollViewGetOffset(pScrollNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiScrollViewGetScrollbarPolicy(pScrollNode->pWidget, &iPolicyX, &iPolicyY);
			(void)xuiScrollViewGetMetrics(pScrollNode->pWidget, &fScrollbarSize, &fMinThumbSize, &fButtonSize);
			(void)xuiScrollViewGetColors(pScrollNode->pWidget, &iTrackColor, &iThumbColor, &iHoverColor,
				&iActiveColor, &iFocusColor, &iDisabledColor);
			(void)xuiScrollViewGetButtonColors(pScrollNode->pWidget, &iButtonColor, &iButtonIconColor);
			(void)xuiScrollViewGetCornerColors(pScrollNode->pWidget, &iCornerColor, &iGripColor);
		}
		if ( (pScrollNode == NULL) || (pScrollNode->pWidget == NULL) || (pFrameWidget == NULL) ||
		     (fContentWidth != 660.0f) || (fContentHeight != 440.0f) ||
		     (fOffsetX != 19.0f) || (fOffsetY != 29.0f) ||
		     (iPolicyX != XUI_SCROLLBAR_POLICY_HIDDEN) || (iPolicyY != XUI_SCROLLBAR_POLICY_ALWAYS) ||
		     (xuiScrollViewGetScrollbarMode(pScrollNode->pWidget) != XUI_SCROLLBAR_MODE_FULL) ||
		     (xuiScrollViewGetWheelAxis(pScrollNode->pWidget) != XUI_WHEEL_AXIS_HORIZONTAL) ||
		     (xuiScrollViewGetWheelStep(pScrollNode->pWidget) != 73.0f) ||
		     (xuiScrollViewIsContentDragEnabled(pScrollNode->pWidget) != 0) ||
		     (xuiScrollViewGetCornerMode(pScrollNode->pWidget) != XUI_SCROLL_FRAME_CORNER_GRIP) ||
		     (xuiScrollViewGetBackgroundColor(pScrollNode->pWidget) != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (fScrollbarSize != 19.0f) || (fMinThumbSize != 33.0f) || (fButtonSize != 22.0f) ||
		     (iTrackColor != XUI_COLOR_RGBA(20, 21, 22, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(36, 37, 38, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(52, 53, 54, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(68, 69, 70, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(84, 85, 86, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(100, 101, 102, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(116, 117, 118, 255)) ||
		     (iButtonIconColor != XUI_COLOR_RGBA(132, 133, 134, 255)) ||
		     (iCornerColor != XUI_COLOR_RGBA(148, 149, 150, 255)) ||
		     (iGripColor != XUI_COLOR_RGBA(164, 165, 166, 255)) ) {
			printf("xui_uidesign exercise-scroll-properties-failed type=scroll_view id=%d size=%.1f/%.1f offset=%.1f/%.1f policy=%d/%d\n",
				iScrollView,
				fContentWidth,
				fContentHeight,
				fOffsetX,
				fOffsetY,
				iPolicyX,
				iPolicyY);
			return XUI_ERROR;
		}
	}
	if ( iButton != 0 ) {
		ui_design_node_t* pButtonNode = uiDesignModelGetNode(&pApp->tModel, iButton);
		char sButtonPatches[512];
		xui_surface pIconSurface = NULL;
		xui_surface pBadgeSurface = NULL;
		xui_rect_t tIconSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tBadgeSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_nine_patch_t tPatch;
		uint32_t arrFill[6] = {0u, 0u, 0u, 0u, 0u, 0u};
		uint32_t arrBorder[6] = {0u, 0u, 0u, 0u, 0u, 0u};
		float arrBorderWidth[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		int iIconPlacement = -1;
		float fIconSize = 0.0f;
		float fIconGap = 0.0f;
		float fBadgeX = 0.0f;
		float fBadgeY = 0.0f;
		memset(&tPatch, 0, sizeof(tPatch));
		snprintf(sButtonPatches, sizeof(sButtonPatches), "normal|%s|9|10|29|30|1|2|3|4|#A1B2C3D4|tile", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "behavior.selectable", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "behavior.selected", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "behavior.semantic", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "data.iconSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "data.statePatches", sButtonPatches);
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.textColor", "#112233");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.disabledTextColor", "#445566");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.normalColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.hoverColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.activeColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.focusColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.disabledColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.checkedColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.borderColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.hoverBorderColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.activeBorderColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.disabledBorderColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.checkedBorderColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.hoverBorderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.activeBorderWidth", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.focusBorderWidth", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.disabledBorderWidth", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.checkedBorderWidth", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "appearance.iconColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "behavior.iconPlacement", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "metrics.iconSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "metrics.iconGap", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "icon.x", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "icon.y", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "icon.w", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "icon.h", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.visible", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.anchor", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.offsetX", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.offsetY", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "metrics.badgeSize", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "data.badgeSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.sourceX", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.sourceY", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.sourceW", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iButton, "badge.sourceH", "28");
		if ( pButtonNode != NULL && pButtonNode->pWidget != NULL ) {
			(void)xuiButtonGetIcon(pButtonNode->pWidget, &pIconSurface, &tIconSrc);
			(void)xuiButtonGetBadgeSurface(pButtonNode->pWidget, &pBadgeSurface, &tBadgeSrc);
			(void)xuiButtonGetIconLayout(pButtonNode->pWidget, &iIconPlacement, &fIconSize, &fIconGap);
			(void)xuiButtonGetBadgeOffset(pButtonNode->pWidget, &fBadgeX, &fBadgeY);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, 0u, &arrFill[0], &arrBorderWidth[0], &arrBorder[0]);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, XUI_WIDGET_STATE_HOVER, &arrFill[1], &arrBorderWidth[1], &arrBorder[1]);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, XUI_WIDGET_STATE_ACTIVE, &arrFill[2], &arrBorderWidth[2], &arrBorder[2]);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, XUI_WIDGET_STATE_FOCUS, &arrFill[3], &arrBorderWidth[3], &arrBorder[3]);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, XUI_WIDGET_STATE_DISABLED, &arrFill[4], &arrBorderWidth[4], &arrBorder[4]);
			(void)xuiButtonGetStateVisual(pButtonNode->pWidget, XUI_BUTTON_STATE_CHECKED, &arrFill[5], &arrBorderWidth[5], &arrBorder[5]);
			(void)xuiButtonGetPatch(pButtonNode->pWidget, 0u, &tPatch);
		}
		if ( (pButtonNode == NULL) || (pButtonNode->pWidget == NULL) ||
		     (xuiButtonGetText(pButtonNode->pWidget) == NULL) ||
		     (strcmp(xuiButtonGetText(pButtonNode->pWidget), "Run") != 0) ||
		     (xuiButtonGetSelectable(pButtonNode->pWidget) == 0) ||
		     (xuiButtonIsSelected(pButtonNode->pWidget) == 0) ||
		     (xuiButtonGetSemantic(pButtonNode->pWidget) != XUI_BUTTON_SEMANTIC_PRIMARY) ||
		     (xuiButtonGetTextColor(pButtonNode->pWidget) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (xuiButtonGetDisabledTextColor(pButtonNode->pWidget) != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (xuiButtonGetTextFlags(pButtonNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiButtonGetIconColor(pButtonNode->pWidget) != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iIconPlacement != XUI_BUTTON_ICON_RIGHT) || (fIconSize != 18.0f) || (fIconGap != 7.0f) ||
		     (pIconSurface == NULL) ||
		     (tIconSrc.fX != 5.0f) || (tIconSrc.fY != 6.0f) || (tIconSrc.fW != 25.0f) || (tIconSrc.fH != 26.0f) ||
		     (arrFill[0] != XUI_COLOR_RGBA(16, 17, 18, 255)) || (arrBorderWidth[0] != 2.0f) || (arrBorder[0] != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (arrFill[1] != XUI_COLOR_RGBA(32, 33, 34, 255)) || (arrBorderWidth[1] != 3.0f) || (arrBorder[1] != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (arrFill[2] != XUI_COLOR_RGBA(48, 49, 50, 255)) || (arrBorderWidth[2] != 4.0f) || (arrBorder[2] != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (arrFill[3] != XUI_COLOR_RGBA(16, 17, 18, 255)) || (arrBorderWidth[3] != 5.0f) || (arrBorder[3] != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (arrFill[4] != XUI_COLOR_RGBA(80, 81, 82, 255)) || (arrBorderWidth[4] != 6.0f) || (arrBorder[4] != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (arrFill[5] != XUI_COLOR_RGBA(96, 97, 98, 255)) || (arrBorderWidth[5] != 7.0f) || (arrBorder[5] != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (xuiButtonGetBadgeVisible(pButtonNode->pWidget) == 0) ||
		     (xuiButtonGetBadgeAnchor(pButtonNode->pWidget) != XUI_BUTTON_BADGE_ICON_TOP_RIGHT) ||
		     (fBadgeX != 4.0f) || (fBadgeY != 5.0f) ||
		     (xuiButtonGetBadgeSize(pButtonNode->pWidget) != 13.0f) ||
		     (pBadgeSurface == NULL) ||
		     (tBadgeSrc.fX != 7.0f) || (tBadgeSrc.fY != 8.0f) || (tBadgeSrc.fW != 27.0f) || (tBadgeSrc.fH != 28.0f) ||
		     (xuiButtonHasPatch(pButtonNode->pWidget, 0u) == 0) ||
		     (tPatch.pSurface == NULL) ||
		     (tPatch.tSrc.fX != 9.0f) || (tPatch.tSrc.fY != 10.0f) || (tPatch.tSrc.fW != 29.0f) || (tPatch.tSrc.fH != 30.0f) ||
		     (tPatch.tSlice.fLeft != 1.0f) || (tPatch.tSlice.fTop != 2.0f) || (tPatch.tSlice.fRight != 3.0f) || (tPatch.tSlice.fBottom != 4.0f) ||
		     (tPatch.iColor != XUI_COLOR_RGBA(161, 178, 195, 212)) ||
		     (tPatch.iMode != XUI_NINE_PATCH_TILE) ) {
			printf("xui_uidesign exercise-button-properties-failed id=%d text=%s selected=%d semantic=%d primary=%d textColor=%08X disabledText=%08X badge=%d\n",
				iButton,
				(pButtonNode != NULL && pButtonNode->pWidget != NULL && xuiButtonGetText(pButtonNode->pWidget) != NULL) ? xuiButtonGetText(pButtonNode->pWidget) : "(null)",
				(pButtonNode != NULL && pButtonNode->pWidget != NULL) ? xuiButtonIsSelected(pButtonNode->pWidget) : -1,
				(pButtonNode != NULL && pButtonNode->pWidget != NULL) ? xuiButtonGetSemantic(pButtonNode->pWidget) : -1,
				XUI_BUTTON_SEMANTIC_PRIMARY,
				(pButtonNode != NULL && pButtonNode->pWidget != NULL) ? xuiButtonGetTextColor(pButtonNode->pWidget) : 0u,
				(pButtonNode != NULL && pButtonNode->pWidget != NULL) ? xuiButtonGetDisabledTextColor(pButtonNode->pWidget) : 0u,
				(pButtonNode != NULL && pButtonNode->pWidget != NULL) ? xuiButtonGetBadgeVisible(pButtonNode->pWidget) : -1);
			return XUI_ERROR;
		}
	}
	if ( iInput != 0 ) {
		ui_design_node_t* pInputNode = uiDesignModelGetNode(&pApp->tModel, iInput);
		xui_input_decoration pLeadingDecoration = NULL;
		xui_input_decoration pTrailingDecoration = NULL;
		xui_input_decoration_desc_t tLeadingDecoration;
		xui_input_decoration_desc_t tTrailingDecoration;
		int iSelectionStart = -1;
		int iSelectionEnd = -1;
		uint32_t iBackgroundColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iErrorBackgroundColor = 0u;
		uint32_t iErrorBorderColor = 0u;
		uint32_t iPlaceholderColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iSelectionColor = 0u;
		uint32_t iCursorColor = 0u;
		memset(&tLeadingDecoration, 0, sizeof(tLeadingDecoration));
		memset(&tTrailingDecoration, 0, sizeof(tTrailingDecoration));
		(void)uiDesignAppSetNodeText(pApp, iInput, "Search value");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "text.placeholder", "Type filter");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "text.maxLength", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "text.align", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "value.selectionStart", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "value.selectionEnd", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "behavior.password", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "behavior.readonly", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "behavior.error", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "metrics.borderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.backgroundColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.hoverBackgroundColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.disabledBackgroundColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.textColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.placeholderColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.disabledTextColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.borderColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.hoverBorderColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.focusBorderColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.errorBackgroundColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.errorBorderColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.selectionColor", "#C1C2C3A4");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "appearance.cursorColor", "#D1D2D3");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "data.decorations",
			"leading|icon|always|28|3|search|||||||#101112|#202122|#303132|#404142\n"
			"trailing|text|notEmpty|34|4||Go||||||#505152|#606162|#707172|#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iInput, "data.menuTitles", "copy|Copy Text\nselectAll|Select Everything");
		if ( pInputNode != NULL && pInputNode->pWidget != NULL ) {
			(void)xuiInputGetSelection(pInputNode->pWidget, &iSelectionStart, &iSelectionEnd);
			(void)xuiInputGetColors(pInputNode->pWidget, &iBackgroundColor, &iTextColor, &iBorderColor, &iFocusBorderColor);
			(void)xuiInputGetErrorColors(pInputNode->pWidget, &iErrorBackgroundColor, &iErrorBorderColor);
			(void)xuiInputGetExtendedColors(pInputNode->pWidget, &iPlaceholderColor, &iDisabledTextColor, &iHoverBackgroundColor,
				&iDisabledBackgroundColor, &iHoverBorderColor, &iSelectionColor, &iCursorColor);
			pLeadingDecoration = xuiInputDecorationGetAt(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_LEADING, 0);
			pTrailingDecoration = xuiInputDecorationGetAt(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_TRAILING, 0);
			if ( pLeadingDecoration != NULL ) {
				(void)xuiInputDecorationGetDesc(pInputNode->pWidget, pLeadingDecoration, &tLeadingDecoration);
			}
			if ( pTrailingDecoration != NULL ) {
				(void)xuiInputDecorationGetDesc(pInputNode->pWidget, pTrailingDecoration, &tTrailingDecoration);
			}
		}
		if ( (pInputNode == NULL) || (pInputNode->pWidget == NULL) ||
		     (strcmp(xuiInputGetText(pInputNode->pWidget), "Search value") != 0) ||
		     (strcmp(xuiInputGetPlaceholder(pInputNode->pWidget), "Type filter") != 0) ||
		     (xuiInputGetMaxLength(pInputNode->pWidget) != 24) ||
		     (xuiInputGetTextAlign(pInputNode->pWidget) != XUI_INPUT_ALIGN_RIGHT) ||
		     (iSelectionStart != 1) || (iSelectionEnd != 6) ||
		     (xuiInputIsPassword(pInputNode->pWidget) == 0) ||
		     (xuiInputIsReadonly(pInputNode->pWidget) == 0) ||
		     (xuiInputGetError(pInputNode->pWidget) == 0) ||
		     (xuiInputGetBorderWidth(pInputNode->pWidget) != 3.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (iErrorBackgroundColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (iErrorBorderColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ||
		     (iPlaceholderColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (iSelectionColor != XUI_COLOR_RGBA(193, 194, 195, 164)) ||
		     (iCursorColor != XUI_COLOR_RGBA(209, 210, 211, 255)) ||
		     (xuiInputDecorationGetCount(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_LEADING) != 1) ||
		     (xuiInputDecorationGetCount(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_TRAILING) != 1) ||
		     (pLeadingDecoration == NULL) ||
		     (pTrailingDecoration == NULL) ||
		     (tLeadingDecoration.iKind != XUI_INPUT_DECORATION_ICON) ||
		     (tLeadingDecoration.iVisibleMode != XUI_INPUT_DECORATION_VISIBLE_ALWAYS) ||
		     (tLeadingDecoration.fWidth != 28.0f) ||
		     (tLeadingDecoration.fPadding != 3.0f) ||
		     (tLeadingDecoration.iIcon != XUI_INPUT_ICON_SEARCH) ||
		     (tLeadingDecoration.iColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tLeadingDecoration.iHoverColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tLeadingDecoration.iActiveColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (tLeadingDecoration.iDisabledColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (tTrailingDecoration.iKind != XUI_INPUT_DECORATION_TEXT) ||
		     (tTrailingDecoration.iVisibleMode != XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY) ||
		     (tTrailingDecoration.fWidth != 34.0f) ||
		     (tTrailingDecoration.fPadding != 4.0f) ||
		     (tTrailingDecoration.sText == NULL) ||
		     (strcmp(tTrailingDecoration.sText, "Go") != 0) ||
		     (tTrailingDecoration.iColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (tTrailingDecoration.iHoverColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (tTrailingDecoration.iActiveColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (tTrailingDecoration.iDisabledColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (strcmp(xuiInputGetMenuTitle(pInputNode->pWidget, XUI_INPUT_MENU_COPY), "Copy Text") != 0) ||
		     (strcmp(xuiInputGetMenuTitle(pInputNode->pWidget, XUI_INPUT_MENU_SELECT_ALL), "Select Everything") != 0) ) {
			printf("xui_uidesign exercise-input-properties-failed id=%d selection=%d/%d decorations=%d/%d\n",
				iInput,
				iSelectionStart,
				iSelectionEnd,
				(pInputNode != NULL && pInputNode->pWidget != NULL) ? xuiInputDecorationGetCount(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_LEADING) : -1,
				(pInputNode != NULL && pInputNode->pWidget != NULL) ? xuiInputDecorationGetCount(pInputNode->pWidget, XUI_INPUT_DECORATION_SIDE_TRAILING) : -1);
			return XUI_ERROR;
		}
	}
	if ( iNumericInput != 0 ) {
		ui_design_node_t* pNumericNode = uiDesignModelGetNode(&pApp->tModel, iNumericInput);
		xui_widget pInnerInput = NULL;
		float fMin = 0.0f;
		float fMax = 0.0f;
		uint32_t iBackgroundColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iPlaceholderColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iErrorBackgroundColor = 0u;
		uint32_t iErrorBorderColor = 0u;
		uint32_t iSelectionColor = 0u;
		uint32_t iCursorColor = 0u;
		uint32_t iSpinnerColor = 0u;
		uint32_t iSpinnerHoverColor = 0u;
		uint32_t iSpinnerActiveColor = 0u;
		uint32_t iSpinnerBorderColor = 0u;
		uint32_t iSpinnerIconColor = 0u;
		uint32_t iSpinnerDisabledIconColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "text.placeholder", "Amount");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "text.rawText", "custom-number");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.min", "-10");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.max", "90");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.value", "42.5");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.step", "2.5");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.integer", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "value.precision", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "behavior.readonly", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "behavior.spinnerVisible", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "behavior.textOverride", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "metrics.spinnerWidth", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "metrics.borderWidth", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.backgroundColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.hoverBackgroundColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.disabledBackgroundColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.textColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.placeholderColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.disabledTextColor", "#626364");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.borderColor", "#727374");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.hoverBorderColor", "#828384");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.focusBorderColor", "#929394");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.errorBackgroundColor", "#A2A3A4");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.errorBorderColor", "#B2B3B4");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.selectionColor", "#C2C3C4A5");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.cursorColor", "#D2D3D4");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerColor", "#132435");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerHoverColor", "#243546");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerActiveColor", "#354657");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerBorderColor", "#465768");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerIconColor", "#576879");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "appearance.spinnerDisabledIconColor", "#68798A");
		(void)uiDesignAppSetNodeProperty(pApp, iNumericInput, "data.menuTitles", "copy|Copy Number\nselectAll|Select Number");
		if ( pNumericNode != NULL && pNumericNode->pWidget != NULL ) {
			(void)xuiNumericInputGetRange(pNumericNode->pWidget, &fMin, &fMax);
			(void)xuiNumericInputGetColors(pNumericNode->pWidget, &iBackgroundColor, &iTextColor, &iBorderColor, &iFocusBorderColor);
			(void)xuiNumericInputGetExtendedColors(pNumericNode->pWidget, &iPlaceholderColor, &iDisabledTextColor,
				&iHoverBackgroundColor, &iDisabledBackgroundColor, &iHoverBorderColor, &iErrorBackgroundColor,
				&iErrorBorderColor, &iSelectionColor, &iCursorColor);
			(void)xuiNumericInputGetSpinnerColors(pNumericNode->pWidget, &iSpinnerColor, &iSpinnerHoverColor,
				&iSpinnerActiveColor, &iSpinnerBorderColor, &iSpinnerIconColor, &iSpinnerDisabledIconColor);
			pInnerInput = xuiNumericInputGetInputWidget(pNumericNode->pWidget);
		}
		if ( (pNumericNode == NULL) || (pNumericNode->pWidget == NULL) ||
		     (fMin != -10.0f) || (fMax != 90.0f) ||
		     (xuiNumericInputGetValue(pNumericNode->pWidget) != 42.5f) ||
		     (xuiNumericInputGetStep(pNumericNode->pWidget) != 2.5f) ||
		     (xuiNumericInputIsInteger(pNumericNode->pWidget) != 0) ||
		     (xuiNumericInputGetPrecision(pNumericNode->pWidget) != 3) ||
		     (xuiNumericInputIsReadonly(pNumericNode->pWidget) == 0) ||
		     (xuiNumericInputGetSpinnerVisible(pNumericNode->pWidget) != 0) ||
		     (xuiNumericInputGetSpinnerWidth(pNumericNode->pWidget) != 27.0f) ||
		     (xuiNumericInputGetBorderWidth(pNumericNode->pWidget) != 4.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(114, 115, 116, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(146, 147, 148, 255)) ||
		     (iPlaceholderColor != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(98, 99, 100, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(130, 131, 132, 255)) ||
		     (iErrorBackgroundColor != XUI_COLOR_RGBA(162, 163, 164, 255)) ||
		     (iErrorBorderColor != XUI_COLOR_RGBA(178, 179, 180, 255)) ||
		     (iSelectionColor != XUI_COLOR_RGBA(194, 195, 196, 165)) ||
		     (iCursorColor != XUI_COLOR_RGBA(210, 211, 212, 255)) ||
		     (iSpinnerColor != XUI_COLOR_RGBA(19, 36, 53, 255)) ||
		     (iSpinnerHoverColor != XUI_COLOR_RGBA(36, 53, 70, 255)) ||
		     (iSpinnerActiveColor != XUI_COLOR_RGBA(53, 70, 87, 255)) ||
		     (iSpinnerBorderColor != XUI_COLOR_RGBA(70, 87, 104, 255)) ||
		     (iSpinnerIconColor != XUI_COLOR_RGBA(87, 104, 121, 255)) ||
		     (iSpinnerDisabledIconColor != XUI_COLOR_RGBA(104, 121, 138, 255)) ||
		     (strcmp(xuiNumericInputGetText(pNumericNode->pWidget), "custom-number") != 0) ||
		     (strcmp(xuiNumericInputGetMenuTitle(pNumericNode->pWidget, XUI_INPUT_MENU_COPY), "Copy Number") != 0) ||
		     (strcmp(xuiNumericInputGetMenuTitle(pNumericNode->pWidget, XUI_INPUT_MENU_SELECT_ALL), "Select Number") != 0) ||
		     (pInnerInput == NULL) ||
		     (strcmp(xuiInputGetPlaceholder(pInnerInput), "Amount") != 0) ) {
			printf("xui_uidesign exercise-numeric-input-properties-failed id=%d range=%.1f/%.1f value=%.1f text=%s\n",
				iNumericInput,
				fMin,
				fMax,
				(pNumericNode != NULL && pNumericNode->pWidget != NULL) ? xuiNumericInputGetValue(pNumericNode->pWidget) : 0.0f,
				(pNumericNode != NULL && pNumericNode->pWidget != NULL) ? xuiNumericInputGetText(pNumericNode->pWidget) : "(null)");
			return XUI_ERROR;
		}
	}
	if ( iPanel != 0 ) {
		ui_design_node_t* pPanelNode = uiDesignModelGetNode(&pApp->tModel, iPanel);
		xui_surface pIconSurface = NULL;
		xui_rect_t tIconSrc;
		float fBorderWidth = 0.0f;
		uint32_t iBorderColor = 0u;
		memset(&tIconSrc, 0, sizeof(tIconSrc));
		(void)uiDesignAppSetNodeText(pApp, iPanel, "Options Panel");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.titleColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.disabledTitleColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.backgroundColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.headerColor", "#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.clientColor", "#506070");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.borderColor", "#607080");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "appearance.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "metrics.headerHeight", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "metrics.headerGap", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "metrics.iconSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "data.iconSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "icon.x", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "icon.y", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "icon.w", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "icon.h", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iPanel, "behavior.clientClip", "false");
		if ( pPanelNode != NULL && pPanelNode->pWidget != NULL ) {
			(void)xuiPanelGetBorder(pPanelNode->pWidget, &fBorderWidth, &iBorderColor);
			pIconSurface = xuiPanelGetIconSurface(pPanelNode->pWidget);
			tIconSrc = xuiPanelGetIconSource(pPanelNode->pWidget);
		}
		if ( (pPanelNode == NULL) || (pPanelNode->pWidget == NULL) ||
		     (strcmp(xuiPanelGetTitle(pPanelNode->pWidget), "Options Panel") != 0) ||
		     (xuiPanelGetTitleColor(pPanelNode->pWidget) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiPanelGetDisabledTitleColor(pPanelNode->pWidget) != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (xuiPanelGetBackgroundColor(pPanelNode->pWidget) != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (xuiPanelGetHeaderColor(pPanelNode->pWidget) != XUI_COLOR_RGBA(64, 80, 96, 255)) ||
		     (xuiPanelGetClientColor(pPanelNode->pWidget) != XUI_COLOR_RGBA(80, 96, 112, 255)) ||
		     (fBorderWidth != 2.0f) ||
		     (iBorderColor != XUI_COLOR_RGBA(96, 112, 128, 255)) ||
		     (xuiPanelGetTitleAlign(pPanelNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiPanelGetHeaderHeight(pPanelNode->pWidget) != 33.0f) ||
		     (xuiPanelGetHeaderGap(pPanelNode->pWidget) != 4.0f) ||
		     (xuiPanelGetIconSize(pPanelNode->pWidget) != 18.0f) ||
		     (pIconSurface == NULL) ||
		     (tIconSrc.fX != 4.0f) || (tIconSrc.fY != 5.0f) || (tIconSrc.fW != 20.0f) || (tIconSrc.fH != 21.0f) ||
		     (xuiPanelGetClientClip(pPanelNode->pWidget) != 0) ) {
			printf("xui_uidesign exercise-panel-properties-failed id=%d border=%.1f/%08X\n",
				iPanel,
				fBorderWidth,
				iBorderColor);
			return XUI_ERROR;
		}
	}
	if ( iCheck != 0 ) {
		ui_design_node_t* pCheckNode = uiDesignModelGetNode(&pApp->tModel, iCheck);
		uint32_t iAccentColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		xui_surface pUncheckedSurface = NULL;
		xui_surface pCheckedSurface = NULL;
		xui_rect_t tUncheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tCheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "checked", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "data.indicatorSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.textColor", "#122334");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.disabledTextColor", "#455667");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.accentColor", "#18293A");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.borderColor", "#293A4B");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.hoverBorderColor", "#3A4B5C");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "appearance.focusColor", "#4B5C6D");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "metrics.indicatorSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "metrics.gap", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "unchecked.x", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "unchecked.y", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "unchecked.w", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "unchecked.h", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "checked.x", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "checked.y", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "checked.w", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iCheck, "checked.h", "24");
		if ( pCheckNode != NULL && pCheckNode->pWidget != NULL ) {
			(void)xuiCheckBoxGetColors(pCheckNode->pWidget, &iAccentColor, &iBorderColor, &iHoverBorderColor, &iFocusColor);
			(void)xuiCheckBoxGetIndicatorSurface(pCheckNode->pWidget, &pUncheckedSurface, &tUncheckedSrc, &pCheckedSurface, &tCheckedSrc);
		}
		if ( (pCheckNode == NULL) || (pCheckNode->pWidget == NULL) ||
		     (xuiCheckBoxGetText(pCheckNode->pWidget) == NULL) ||
		     (strcmp(xuiCheckBoxGetText(pCheckNode->pWidget), "Enabled") != 0) ||
		     (xuiCheckBoxGetChecked(pCheckNode->pWidget) != 0) ||
		     (xuiCheckBoxGetUseBuiltinAtlas(pCheckNode->pWidget) != 0) ||
		     (xuiCheckBoxGetTextColor(pCheckNode->pWidget) != XUI_COLOR_RGBA(18, 35, 52, 255)) ||
		     (xuiCheckBoxGetDisabledTextColor(pCheckNode->pWidget) != XUI_COLOR_RGBA(69, 86, 103, 255)) ||
		     (xuiCheckBoxGetTextFlags(pCheckNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (iAccentColor != XUI_COLOR_RGBA(24, 41, 58, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(41, 58, 75, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(58, 75, 92, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(75, 92, 109, 255)) ||
		     (xuiCheckBoxGetIndicatorSize(pCheckNode->pWidget) != 18.0f) ||
		     (xuiCheckBoxGetGap(pCheckNode->pWidget) != 9.0f) ||
		     (pUncheckedSurface == NULL) || (pCheckedSurface == NULL) || (pUncheckedSurface != pCheckedSurface) ||
		     (tUncheckedSrc.fX != 1.0f) || (tUncheckedSrc.fY != 2.0f) || (tUncheckedSrc.fW != 21.0f) || (tUncheckedSrc.fH != 22.0f) ||
		     (tCheckedSrc.fX != 3.0f) || (tCheckedSrc.fY != 4.0f) || (tCheckedSrc.fW != 23.0f) || (tCheckedSrc.fH != 24.0f) ) {
			printf("xui_uidesign exercise-checkbox-properties-failed id=%d\n", iCheck);
			return XUI_ERROR;
		}
	}
	if ( iLabel != 0 ) {
		ui_design_node_t* pLabelNode = uiDesignModelGetNode(&pApp->tModel, iLabel);
		(void)uiDesignAppSetNodeText(pApp, iLabel, "Status Label");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "appearance.textColor", "#213243");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "appearance.disabledTextColor", "#546576");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "text.wrapMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "text.underline", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "text.lineGap", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iLabel, "text.paragraphGap", "4");
		if ( (pLabelNode == NULL) || (pLabelNode->pWidget == NULL) ||
		     (xuiLabelGetText(pLabelNode->pWidget) == NULL) ||
		     (strcmp(xuiLabelGetText(pLabelNode->pWidget), "Status Label") != 0) ||
		     (xuiLabelGetTextColor(pLabelNode->pWidget) != XUI_COLOR_RGBA(33, 50, 67, 255)) ||
		     (xuiLabelGetDisabledTextColor(pLabelNode->pWidget) != XUI_COLOR_RGBA(84, 101, 118, 255)) ||
		     (xuiLabelGetTextFlags(pLabelNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiLabelGetWrapMode(pLabelNode->pWidget) != XUI_TEXT_WRAP_WORD) ||
		     (xuiLabelGetUnderline(pLabelNode->pWidget) == 0) ||
		     (xuiLabelGetLineGap(pLabelNode->pWidget) != 2.0f) ||
		     (xuiLabelGetParagraphGap(pLabelNode->pWidget) != 4.0f) ) {
			printf("xui_uidesign exercise-label-properties-failed id=%d\n", iLabel);
			return XUI_ERROR;
		}
	}
	if ( iRadio != 0 ) {
		ui_design_node_t* pRadioNode = uiDesignModelGetNode(&pApp->tModel, iRadio);
		uint32_t iAccentColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		xui_surface pUncheckedSurface = NULL;
		xui_surface pCheckedSurface = NULL;
		xui_rect_t tUncheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tCheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		(void)uiDesignAppSetNodeText(pApp, iRadio, "Choice");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "checked", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "data.indicatorSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.textColor", "#233445");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.disabledTextColor", "#566778");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "text.flags", "273");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.accentColor", "#112233");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.borderColor", "#223344");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.hoverBorderColor", "#334455");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "appearance.focusColor", "#445566");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "metrics.indicatorSize", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "metrics.gap", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "unchecked.x", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "unchecked.y", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "unchecked.w", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "unchecked.h", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "checked.x", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "checked.y", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "checked.w", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iRadio, "checked.h", "28");
		if ( pRadioNode != NULL && pRadioNode->pWidget != NULL ) {
			(void)xuiRadioGetColors(pRadioNode->pWidget, &iAccentColor, &iBorderColor, &iHoverBorderColor, &iFocusColor);
			(void)xuiRadioGetIndicatorSurface(pRadioNode->pWidget, &pUncheckedSurface, &tUncheckedSrc, &pCheckedSurface, &tCheckedSrc);
		}
		if ( (pRadioNode == NULL) || (pRadioNode->pWidget == NULL) ||
		     (xuiRadioGetText(pRadioNode->pWidget) == NULL) ||
		     (strcmp(xuiRadioGetText(pRadioNode->pWidget), "Choice") != 0) ||
		     (xuiRadioGetChecked(pRadioNode->pWidget) == 0) ||
		     (xuiRadioGetUseBuiltinAtlas(pRadioNode->pWidget) != 0) ||
		     (xuiRadioGetTextColor(pRadioNode->pWidget) != XUI_COLOR_RGBA(35, 52, 69, 255)) ||
		     (xuiRadioGetDisabledTextColor(pRadioNode->pWidget) != XUI_COLOR_RGBA(86, 103, 120, 255)) ||
		     (xuiRadioGetTextFlags(pRadioNode->pWidget) != (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (iAccentColor != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(34, 51, 68, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(51, 68, 85, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (xuiRadioGetIndicatorSize(pRadioNode->pWidget) != 19.0f) ||
		     (xuiRadioGetGap(pRadioNode->pWidget) != 10.0f) ||
		     (pUncheckedSurface == NULL) || (pCheckedSurface == NULL) || (pUncheckedSurface != pCheckedSurface) ||
		     (tUncheckedSrc.fX != 5.0f) || (tUncheckedSrc.fY != 6.0f) || (tUncheckedSrc.fW != 25.0f) || (tUncheckedSrc.fH != 26.0f) ||
		     (tCheckedSrc.fX != 7.0f) || (tCheckedSrc.fY != 8.0f) || (tCheckedSrc.fW != 27.0f) || (tCheckedSrc.fH != 28.0f) ) {
			printf("xui_uidesign exercise-radio-properties-failed id=%d\n", iRadio);
			return XUI_ERROR;
		}
	}
	if ( iCheckCard != 0 ) {
		ui_design_node_t* pCardNode = uiDesignModelGetNode(&pApp->tModel, iCheckCard);
		xui_thickness_t tPadding;
		xui_vec2_t tMinSize;
		float fBorderWidth;
		float fCheckedBorderWidth;
		float fCornerSize;
		float fFocusWidth;
		uint32_t iBackgroundColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iActiveBackgroundColor = 0u;
		uint32_t iCheckedBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iCheckedBorderColor = 0u;
		uint32_t iCornerColor = 0u;
		uint32_t iCheckColor = 0u;
		uint32_t iDisabledBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		tPadding = (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f};
		tMinSize = (xui_vec2_t){0.0f, 0.0f};
		fBorderWidth = fCheckedBorderWidth = fCornerSize = fFocusWidth = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "checked", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.minWidth", "123");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.minHeight", "67");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.paddingLeft", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.paddingTop", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.paddingRight", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.paddingBottom", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.checkedBorderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.cornerSize", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "metrics.focusWidth", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.hoverBackgroundColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.activeBackgroundColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.checkedBackgroundColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.borderColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.hoverBorderColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.checkedBorderColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.disabledBorderColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.cornerColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.checkColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iCheckCard, "appearance.focusColor", "#B0B1B2");
		if ( pCardNode != NULL && pCardNode->pWidget != NULL ) {
			tPadding = xuiWidgetGetPadding(pCardNode->pWidget);
			tMinSize = xuiWidgetGetMinSize(pCardNode->pWidget);
			(void)xuiCheckCardGetMetrics(pCardNode->pWidget, &fBorderWidth, &fCheckedBorderWidth, &fCornerSize, &fFocusWidth);
			(void)xuiCheckCardGetColors(pCardNode->pWidget, &iBackgroundColor, &iHoverBackgroundColor, &iActiveBackgroundColor, &iCheckedBackgroundColor, &iBorderColor, &iHoverBorderColor, &iCheckedBorderColor, &iCornerColor, &iCheckColor);
			(void)xuiCheckCardGetStateColors(pCardNode->pWidget, &iDisabledBorderColor, &iFocusColor);
		}
		if ( (pCardNode == NULL) || (pCardNode->pWidget == NULL) ||
		     (xuiCheckCardGetChecked(pCardNode->pWidget) == 0) ||
		     (tMinSize.fX != 123.0f) || (tMinSize.fY != 67.0f) ||
		     (tPadding.fLeft != 11.0f) || (tPadding.fTop != 12.0f) ||
		     (tPadding.fRight != 13.0f) || (tPadding.fBottom != 14.0f) ||
		     (fBorderWidth != 2.0f) || (fCheckedBorderWidth != 3.0f) ||
		     (fCornerSize != 19.0f) || (fFocusWidth != 4.0f) ) {
			printf("xui_uidesign exercise-check-card-properties-failed id=%d\n", iCheckCard);
			return XUI_ERROR;
		}
		if ( (iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iActiveBackgroundColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iCheckedBackgroundColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iCheckedBorderColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iDisabledBorderColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iCornerColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iCheckColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ) {
			printf("xui_uidesign exercise-check-card-colors-failed id=%d\n", iCheckCard);
			return XUI_ERROR;
		}
	}
	if ( iHyperlink != 0 ) {
		ui_design_node_t* pHyperlinkNode = uiDesignModelGetNode(&pApp->tModel, iHyperlink);
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "appearance.textColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "appearance.hoverTextColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "appearance.activeTextColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "appearance.disabledTextColor", "#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.wrapMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.underline", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.hoverUnderline", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.activeUnderline", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.lineGap", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iHyperlink, "text.paragraphGap", "5");
		if ( (pHyperlinkNode == NULL) || (pHyperlinkNode->pWidget == NULL) ||
		     (xuiHyperlinkGetText(pHyperlinkNode->pWidget) == NULL) ||
		     (strcmp(xuiHyperlinkGetText(pHyperlinkNode->pWidget), "Open Docs") != 0) ||
		     (xuiHyperlinkGetTextColor(pHyperlinkNode->pWidget) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiHyperlinkGetHoverTextColor(pHyperlinkNode->pWidget) != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (xuiHyperlinkGetActiveTextColor(pHyperlinkNode->pWidget) != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (xuiHyperlinkGetDisabledTextColor(pHyperlinkNode->pWidget) != XUI_COLOR_RGBA(64, 80, 96, 255)) ||
		     (xuiHyperlinkGetTextFlags(pHyperlinkNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiHyperlinkGetWrapMode(pHyperlinkNode->pWidget) != XUI_TEXT_WRAP_WORD) ||
		     (xuiHyperlinkGetUnderline(pHyperlinkNode->pWidget) != 0) ||
		     (xuiHyperlinkGetHoverUnderline(pHyperlinkNode->pWidget) == 0) ||
		     (xuiHyperlinkGetActiveUnderline(pHyperlinkNode->pWidget) != 0) ||
		     (xuiHyperlinkGetLineGap(pHyperlinkNode->pWidget) != 3.0f) ||
		     (xuiHyperlinkGetParagraphGap(pHyperlinkNode->pWidget) != 5.0f) ) {
			printf("xui_uidesign exercise-hyperlink-properties-failed id=%d\n", iHyperlink);
			return XUI_ERROR;
		}
	}
	if ( iToggle != 0 ) {
		ui_design_node_t* pToggleNode = uiDesignModelGetNode(&pApp->tModel, iToggle);
		uint32_t iAccentColor = 0u;
		uint32_t iTrackColor = 0u;
		uint32_t iHoverTrackColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iUncheckedTextColor = 0u;
		uint32_t iCheckedTextColor = 0u;
		float fInnerPadding = 0.0f;
		float fInnerGap = 0.0f;
		xui_surface pUncheckedSurface = NULL;
		xui_surface pCheckedSurface = NULL;
		xui_rect_t tUncheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tCheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "checked", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "text.uncheckedText", "No");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "text.checkedText", "Yes");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.textColor", "#112233");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.disabledTextColor", "#445566");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.accentColor", "#556677");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.trackColor", "#667788");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.hoverTrackColor", "#778899");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.focusColor", "#8899AA");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.uncheckedTextColor", "#99AABB");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "appearance.checkedTextColor", "#AABBCC");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.trackWidth", "54");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.trackHeight", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.thumbSize", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.gap", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.innerTextPadding", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "metrics.innerTextGap", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "data.indicatorSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "unchecked.x", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "unchecked.y", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "unchecked.w", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "unchecked.h", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "checked.x", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "checked.y", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "checked.w", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iToggle, "checked.h", "25");
		if ( pToggleNode != NULL && pToggleNode->pWidget != NULL ) {
			(void)xuiToggleGetColors(pToggleNode->pWidget, &iAccentColor, &iTrackColor, &iHoverTrackColor, &iFocusColor);
			(void)xuiToggleGetInnerTextColor(pToggleNode->pWidget, &iUncheckedTextColor, &iCheckedTextColor);
			(void)xuiToggleGetInnerTextMetrics(pToggleNode->pWidget, &fInnerPadding, &fInnerGap);
			(void)xuiToggleGetIndicatorSurface(pToggleNode->pWidget, &pUncheckedSurface, &tUncheckedSrc, &pCheckedSurface, &tCheckedSrc);
		}
		if ( (pToggleNode == NULL) || (pToggleNode->pWidget == NULL) ||
		     (xuiToggleGetText(pToggleNode->pWidget) == NULL) ||
		     (strcmp(xuiToggleGetText(pToggleNode->pWidget), "Power") != 0) ||
		     (xuiToggleGetChecked(pToggleNode->pWidget) != 0) ||
		     (xuiToggleGetUseBuiltinAtlas(pToggleNode->pWidget) != 0) ||
		     (xuiToggleGetUncheckedText(pToggleNode->pWidget) == NULL) ||
		     (strcmp(xuiToggleGetUncheckedText(pToggleNode->pWidget), "No") != 0) ||
		     (xuiToggleGetCheckedText(pToggleNode->pWidget) == NULL) ||
		     (strcmp(xuiToggleGetCheckedText(pToggleNode->pWidget), "Yes") != 0) ||
		     (xuiToggleGetTextColor(pToggleNode->pWidget) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (xuiToggleGetDisabledTextColor(pToggleNode->pWidget) != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (xuiToggleGetTrackWidth(pToggleNode->pWidget) != 54.0f) ||
		     (xuiToggleGetTrackHeight(pToggleNode->pWidget) != 24.0f) ||
		     (xuiToggleGetThumbSize(pToggleNode->pWidget) != 20.0f) ||
		     (xuiToggleGetGap(pToggleNode->pWidget) != 9.0f) ||
		     (iAccentColor != XUI_COLOR_RGBA(85, 102, 119, 255)) ||
		     (iTrackColor != XUI_COLOR_RGBA(102, 119, 136, 255)) ||
		     (iHoverTrackColor != XUI_COLOR_RGBA(119, 136, 153, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(136, 153, 170, 255)) ||
		     (iUncheckedTextColor != XUI_COLOR_RGBA(153, 170, 187, 255)) ||
		     (iCheckedTextColor != XUI_COLOR_RGBA(170, 187, 204, 255)) ||
		     (fInnerPadding != 7.0f) || (fInnerGap != 4.0f) ||
		     (pUncheckedSurface == NULL) || (pCheckedSurface == NULL) || (pUncheckedSurface != pCheckedSurface) ||
		     (tUncheckedSrc.fX != 2.0f) || (tUncheckedSrc.fY != 3.0f) || (tUncheckedSrc.fW != 22.0f) || (tUncheckedSrc.fH != 23.0f) ||
		     (tCheckedSrc.fX != 4.0f) || (tCheckedSrc.fY != 5.0f) || (tCheckedSrc.fW != 24.0f) || (tCheckedSrc.fH != 25.0f) ) {
			printf("xui_uidesign exercise-toggle-properties-failed id=%d\n", iToggle);
			return XUI_ERROR;
		}
	}
	if ( iScrollBar != 0 ) {
		ui_design_node_t* pScrollBarNode = uiDesignModelGetNode(&pApp->tModel, iScrollBar);
		float fMin;
		float fMax;
		float fPage;
		float fSmallStep;
		float fLargeStep;
		float fThickness;
		float fMinThumbSize;
		float fButtonSize;
		uint32_t iTrackColor = 0u;
		uint32_t iThumbColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iActiveColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonIconColor = 0u;
		fMin = fMax = fPage = 0.0f;
		fSmallStep = fLargeStep = 0.0f;
		fThickness = fMinThumbSize = fButtonSize = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.min", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.max", "210");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.page", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.value", "50");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.smallStep", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "value.largeStep", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "behavior.mode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "behavior.buttonMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "metrics.thickness", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "metrics.minThumbSize", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "metrics.buttonSize", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.trackColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.thumbColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.hoverColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.activeColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.focusColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.disabledColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.buttonColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iScrollBar, "appearance.buttonIconColor", "#808182");
		if ( pScrollBarNode != NULL && pScrollBarNode->pWidget != NULL ) {
			(void)xuiScrollBarGetRange(pScrollBarNode->pWidget, &fMin, &fMax, &fPage);
			(void)xuiScrollBarGetSteps(pScrollBarNode->pWidget, &fSmallStep, &fLargeStep);
			(void)xuiScrollBarGetMetrics(pScrollBarNode->pWidget, &fThickness, &fMinThumbSize, &fButtonSize);
			(void)xuiScrollBarGetColors(pScrollBarNode->pWidget, &iTrackColor, &iThumbColor, &iHoverColor, &iActiveColor, &iFocusColor, &iDisabledColor);
			(void)xuiScrollBarGetButtonColors(pScrollBarNode->pWidget, &iButtonColor, &iButtonIconColor);
		}
		if ( (pScrollBarNode == NULL) || (pScrollBarNode->pWidget == NULL) ||
		     (fMin != 10.0f) || (fMax != 210.0f) || (fPage != 20.0f) ||
		     (xuiScrollBarGetValue(pScrollBarNode->pWidget) != 50.0f) ||
		     (fSmallStep != 2.0f) || (fLargeStep != 15.0f) ||
		     (xuiScrollBarGetOrientation(pScrollBarNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (xuiScrollBarGetMode(pScrollBarNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (xuiScrollBarGetButtonMode(pScrollBarNode->pWidget) != XUI_SCROLLBAR_BUTTONS_OFF) ||
		     (fThickness != 18.0f) || (fMinThumbSize != 30.0f) || (fButtonSize != 21.0f) ) {
			printf("xui_uidesign exercise-scrollbar-properties-failed id=%d\n", iScrollBar);
			return XUI_ERROR;
		}
		if ( (iTrackColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iButtonIconColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ) {
			printf("xui_uidesign exercise-scrollbar-colors-failed id=%d\n", iScrollBar);
			return XUI_ERROR;
		}
	}
	if ( iSlider != 0 ) {
		ui_design_node_t* pSliderNode = uiDesignModelGetNode(&pApp->tModel, iSlider);
		float fMin;
		float fMax;
		float fStep;
		float fPageStep;
		float fTrackSize;
		float fKnobSize;
		float fTrackRadius;
		uint32_t iTrackColor = 0u;
		uint32_t iFillColor = 0u;
		uint32_t iKnobColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		fMin = fMax = fStep = fPageStep = 0.0f;
		fTrackSize = fKnobSize = fTrackRadius = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "value.min", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "value.max", "105");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "value.value", "55");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "value.step", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "value.pageStep", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "metrics.trackSize", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "metrics.knobSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "metrics.trackRadius", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.trackColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.fillColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.knobColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.knobBorderColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.focusColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iSlider, "appearance.disabledColor", "#616263");
		if ( pSliderNode != NULL && pSliderNode->pWidget != NULL ) {
			(void)xuiSliderGetRange(pSliderNode->pWidget, &fMin, &fMax);
			(void)xuiSliderGetStep(pSliderNode->pWidget, &fStep, &fPageStep);
			(void)xuiSliderGetMetrics(pSliderNode->pWidget, &fTrackSize, &fKnobSize, &fTrackRadius);
			(void)xuiSliderGetColors(pSliderNode->pWidget, &iTrackColor, &iFillColor, &iKnobColor, &iFocusColor, &iDisabledColor);
		}
		if ( (pSliderNode == NULL) || (pSliderNode->pWidget == NULL) ||
		     (fMin != 5.0f) || (fMax != 105.0f) ||
		     (xuiSliderGetValue(pSliderNode->pWidget) != 55.0f) ||
		     (fStep != 2.0f) || (fPageStep != 20.0f) ||
		     (xuiSliderGetOrientation(pSliderNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (fTrackSize != 6.0f) || (fKnobSize != 18.0f) || (fTrackRadius != 3.0f) ||
		     (iTrackColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iFillColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iKnobColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (xuiSliderGetKnobBorderColor(pSliderNode->pWidget) != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ) {
			printf("xui_uidesign exercise-slider-properties-failed id=%d\n", iSlider);
			return XUI_ERROR;
		}
	}
	if ( iRangeSlider != 0 ) {
		ui_design_node_t* pRangeNode = uiDesignModelGetNode(&pApp->tModel, iRangeSlider);
		float fMin;
		float fMax;
		float fStart;
		float fEnd;
		float fStep;
		float fPageStep;
		float fMinInterval;
		float fMaxInterval;
		float fTrackSize;
		float fKnobSize;
		uint32_t iTrackColor = 0u;
		uint32_t iFillColor = 0u;
		uint32_t iKnobColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		fMin = fMax = fStart = fEnd = 0.0f;
		fStep = fPageStep = fMinInterval = fMaxInterval = 0.0f;
		fTrackSize = fKnobSize = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.min", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.max", "200");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.start", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.end", "170");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.step", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.pageStep", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.minInterval", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "value.maxInterval", "160");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "metrics.trackSize", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "metrics.knobSize", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.trackColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.fillColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.knobColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.knobBorderColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.focusColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iRangeSlider, "appearance.disabledColor", "#626364");
		if ( pRangeNode != NULL && pRangeNode->pWidget != NULL ) {
			(void)xuiRangeSliderGetRange(pRangeNode->pWidget, &fMin, &fMax);
			(void)xuiRangeSliderGetValues(pRangeNode->pWidget, &fStart, &fEnd);
			(void)xuiRangeSliderGetStep(pRangeNode->pWidget, &fStep, &fPageStep);
			(void)xuiRangeSliderGetIntervalLimits(pRangeNode->pWidget, &fMinInterval, &fMaxInterval);
			(void)xuiRangeSliderGetMetrics(pRangeNode->pWidget, &fTrackSize, &fKnobSize);
			(void)xuiRangeSliderGetColors(pRangeNode->pWidget, &iTrackColor, &iFillColor, &iKnobColor, &iFocusColor, &iDisabledColor);
		}
		if ( (pRangeNode == NULL) || (pRangeNode->pWidget == NULL) ||
		     (fMin != 0.0f) || (fMax != 200.0f) ||
		     (fStart != 30.0f) || (fEnd != 170.0f) ||
		     (fStep != 5.0f) || (fPageStep != 25.0f) ||
		     (fMinInterval != 20.0f) || (fMaxInterval != 160.0f) ||
		     (xuiRangeSliderGetOrientation(pRangeNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (fTrackSize != 7.0f) || (fKnobSize != 19.0f) ||
		     (iTrackColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (iFillColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (iKnobColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (xuiRangeSliderGetKnobBorderColor(pRangeNode->pWidget) != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(98, 99, 100, 255)) ) {
			printf("xui_uidesign exercise-range-slider-properties-failed id=%d\n", iRangeSlider);
			return XUI_ERROR;
		}
	}
	if ( iSeparator != 0 ) {
		ui_design_node_t* pSeparatorNode = uiDesignModelGetNode(&pApp->tModel, iSeparator);
		(void)uiDesignAppSetNodeProperty(pApp, iSeparator, "appearance.color", "#334455");
		(void)uiDesignAppSetNodeProperty(pApp, iSeparator, "metrics.thickness", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iSeparator, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iSeparator, "behavior.align", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iSeparator, "behavior.lineStyle", "2");
		if ( (pSeparatorNode == NULL) || (pSeparatorNode->pWidget == NULL) ||
		     (xuiSeparatorGetColor(pSeparatorNode->pWidget) != XUI_COLOR_RGBA(51, 68, 85, 255)) ||
		     (xuiSeparatorGetThickness(pSeparatorNode->pWidget) != 3.0f) ||
		     (xuiSeparatorGetOrientation(pSeparatorNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (xuiSeparatorGetAlign(pSeparatorNode->pWidget) != XUI_ALIGN_END) ||
		     (xuiSeparatorGetLineStyle(pSeparatorNode->pWidget) != XUI_SEPARATOR_DASH) ) {
			printf("xui_uidesign exercise-separator-properties-failed id=%d\n", iSeparator);
			return XUI_ERROR;
		}
	}
	if ( iProgress != 0 ) {
		ui_design_node_t* pProgressNode = uiDesignModelGetNode(&pApp->tModel, iProgress);
		float fMin;
		float fMax;
		int bHasTrackPatch;
		int bHasFillPatch;
		xui_nine_patch_t tTrackPatch;
		xui_nine_patch_t tFillPatch;
		fMin = fMax = 0.0f;
		bHasTrackPatch = bHasFillPatch = 0;
		memset(&tTrackPatch, 0, sizeof(tTrackPatch));
		memset(&tFillPatch, 0, sizeof(tFillPatch));
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "value.min", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "value.max", "110");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "value.value", "60");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "text.template", "{value}/{max}");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "appearance.trackColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "appearance.fillColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "appearance.textColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "appearance.fillTextColor", "#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "behavior.fillDirection", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "behavior.fillPatchMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "data.trackPatchSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.x", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.y", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.w", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.h", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.sliceLeft", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.sliceTop", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.sliceRight", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.sliceBottom", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.color", "#50607080");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "trackPatch.mode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "data.fillPatchSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_knob.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.x", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.y", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.w", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.h", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.sliceLeft", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.sliceTop", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.sliceRight", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.sliceBottom", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.color", "#90A0B0C0");
		(void)uiDesignAppSetNodeProperty(pApp, iProgress, "fillPatch.mode", "0");
		if ( pProgressNode != NULL && pProgressNode->pWidget != NULL ) {
			(void)xuiProgressGetRange(pProgressNode->pWidget, &fMin, &fMax);
			bHasTrackPatch = xuiProgressHasTrackPatch(pProgressNode->pWidget);
			bHasFillPatch = xuiProgressHasFillPatch(pProgressNode->pWidget);
			(void)xuiProgressGetTrackPatch(pProgressNode->pWidget, &tTrackPatch);
			(void)xuiProgressGetFillPatch(pProgressNode->pWidget, &tFillPatch);
		}
		if ( (pProgressNode == NULL) || (pProgressNode->pWidget == NULL) ||
		     (fMin != 10.0f) || (fMax != 110.0f) ||
		     (xuiProgressGetValue(pProgressNode->pWidget) != 60.0f) ||
		     (xuiProgressGetRate(pProgressNode->pWidget) != 0.5f) ||
		     (xuiProgressGetTextTemplate(pProgressNode->pWidget) == NULL) ||
		     (strcmp(xuiProgressGetTextTemplate(pProgressNode->pWidget), "{value}/{max}") != 0) ||
		     (xuiProgressGetTextFlags(pProgressNode->pWidget) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiProgressGetTrackColor(pProgressNode->pWidget) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiProgressGetFillColor(pProgressNode->pWidget) != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (xuiProgressGetTextColor(pProgressNode->pWidget) != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (xuiProgressGetFillTextColor(pProgressNode->pWidget) != XUI_COLOR_RGBA(64, 80, 96, 255)) ||
		     (xuiProgressGetFillDirection(pProgressNode->pWidget) != XUI_PROGRESS_BOTTOM_TO_TOP) ||
		     (xuiProgressGetFillPatchMode(pProgressNode->pWidget) != XUI_PROGRESS_FILL_REVEAL) ||
		     !bHasTrackPatch || !bHasFillPatch ||
		     (tTrackPatch.pSurface == NULL) || (tFillPatch.pSurface == NULL) ||
		     (tTrackPatch.tSrc.fX != 1.0f) || (tTrackPatch.tSrc.fY != 2.0f) ||
		     (tTrackPatch.tSrc.fW != 21.0f) || (tTrackPatch.tSrc.fH != 22.0f) ||
		     (tTrackPatch.tSlice.fLeft != 3.0f) || (tTrackPatch.tSlice.fTop != 4.0f) ||
		     (tTrackPatch.tSlice.fRight != 5.0f) || (tTrackPatch.tSlice.fBottom != 6.0f) ||
		     (tTrackPatch.iColor != XUI_COLOR_RGBA(80, 96, 112, 128)) ||
		     (tTrackPatch.iMode != XUI_NINE_PATCH_TILE) ||
		     (tFillPatch.tSrc.fX != 7.0f) || (tFillPatch.tSrc.fY != 8.0f) ||
		     (tFillPatch.tSrc.fW != 23.0f) || (tFillPatch.tSrc.fH != 24.0f) ||
		     (tFillPatch.tSlice.fLeft != 9.0f) || (tFillPatch.tSlice.fTop != 10.0f) ||
		     (tFillPatch.tSlice.fRight != 11.0f) || (tFillPatch.tSlice.fBottom != 12.0f) ||
		     (tFillPatch.iColor != XUI_COLOR_RGBA(144, 160, 176, 192)) ||
		     (tFillPatch.iMode != XUI_NINE_PATCH_STRETCH) ) {
			printf("xui_uidesign exercise-progress-properties-failed id=%d props=%d has=%d/%d source='%s'/'%s'\n",
				iProgress, pProgressNode->iPropertyCount,
				bHasTrackPatch, bHasFillPatch,
				uiDesignNodeGetProperty(pProgressNode, "data.trackPatchSource", "<missing>"),
				uiDesignNodeGetProperty(pProgressNode, "data.fillPatchSource", "<missing>"));
			return XUI_ERROR;
		}
	}
	if ( iVirtualJoystick != 0 ) {
		ui_design_node_t* pJoystickNode = uiDesignModelGetNode(&pApp->tModel, iVirtualJoystick);
		int bRightPressed;
		int bLeftPressed;
		float fRightValue;
		float fLeftValue;
		float fRadius;
		float fKnobSize;
		float fDeadZone;
		uint32_t iBaseColor = 0u;
		uint32_t iBaseActiveColor = 0u;
		uint32_t iKnobColor = 0u;
		uint32_t iKnobActiveColor = 0u;
		uint32_t iRippleColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iDisabledColor = 0u;
		xui_surface pBaseSurface = NULL;
		xui_surface pBaseActiveSurface = NULL;
		xui_surface pKnobSurface = NULL;
		xui_surface pKnobActiveSurface = NULL;
		xui_surface pRippleSurface = NULL;
		xui_rect_t tBaseSrc;
		xui_rect_t tBaseActiveSrc;
		xui_rect_t tKnobSrc;
		xui_rect_t tKnobActiveSrc;
		xui_rect_t tRippleSrc;
		bRightPressed = bLeftPressed = 0;
		fRightValue = fLeftValue = 0.0f;
		fRadius = fKnobSize = fDeadZone = 0.0f;
		memset(&tBaseSrc, 0, sizeof(tBaseSrc));
		memset(&tBaseActiveSrc, 0, sizeof(tBaseActiveSrc));
		memset(&tKnobSrc, 0, sizeof(tKnobSrc));
		memset(&tKnobActiveSrc, 0, sizeof(tKnobActiveSrc));
		memset(&tRippleSrc, 0, sizeof(tRippleSrc));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.channels", "right|true|0.75\nup|true|0.5\nleft|false|0.25");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "metrics.radius", "66");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "metrics.knobSize", "36");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "metrics.deadZone", "0.15");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.baseColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.baseActiveColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.knobColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.knobActiveColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.rippleColor", "#50515260");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.focusColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "appearance.disabledColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.baseSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.baseActiveSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base_active.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.knobSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_knob.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.knobActiveSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_knob_active.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.rippleSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_ripple.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "base.x", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "base.y", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "base.w", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "base.h", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "baseActive.x", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "baseActive.y", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "baseActive.w", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "baseActive.h", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knob.x", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knob.y", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knob.w", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knob.h", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knobActive.x", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knobActive.y", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knobActive.w", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "knobActive.h", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "ripple.x", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "ripple.y", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "ripple.w", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "ripple.h", "30");
		if ( pJoystickNode != NULL && pJoystickNode->pWidget != NULL ) {
			(void)xuiVirtualJoystickGetChannel(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, &bRightPressed, &fRightValue);
			(void)xuiVirtualJoystickGetChannel(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT, &bLeftPressed, &fLeftValue);
			(void)xuiVirtualJoystickGetMetrics(pJoystickNode->pWidget, &fRadius, &fKnobSize, &fDeadZone);
			(void)xuiVirtualJoystickGetColors(pJoystickNode->pWidget, &iBaseColor, &iBaseActiveColor, &iKnobColor, &iKnobActiveColor, &iRippleColor, &iFocusColor, &iDisabledColor);
			(void)xuiVirtualJoystickGetSurface(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_BASE, &pBaseSurface, &tBaseSrc);
			(void)xuiVirtualJoystickGetSurface(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_BASE_ACTIVE, &pBaseActiveSurface, &tBaseActiveSrc);
			(void)xuiVirtualJoystickGetSurface(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_KNOB, &pKnobSurface, &tKnobSrc);
			(void)xuiVirtualJoystickGetSurface(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_KNOB_ACTIVE, &pKnobActiveSurface, &tKnobActiveSrc);
			(void)xuiVirtualJoystickGetSurface(pJoystickNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_RIPPLE, &pRippleSurface, &tRippleSrc);
		}
		if ( (pJoystickNode == NULL) || (pJoystickNode->pWidget == NULL) ||
		     (bRightPressed == 0) || (fRightValue != 0.75f) ||
		     (bLeftPressed != 0) ||
		     (fRadius != 66.0f) || (fKnobSize != 36.0f) || (fDeadZone != 0.15f) ||
		     (xuiVirtualJoystickGetUseBuiltinAtlas(pJoystickNode->pWidget) != 0) ||
		     (iBaseColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iBaseActiveColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iKnobColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iKnobActiveColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iRippleColor != XUI_COLOR_RGBA(80, 81, 82, 96)) ||
		     (iFocusColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     ((pBaseSurface != NULL) && ((tBaseSrc.fX != 1.0f) || (tBaseSrc.fY != 2.0f) || (tBaseSrc.fW != 21.0f) || (tBaseSrc.fH != 22.0f))) ||
		     ((pBaseActiveSurface != NULL) && ((tBaseActiveSrc.fX != 3.0f) || (tBaseActiveSrc.fY != 4.0f) || (tBaseActiveSrc.fW != 23.0f) || (tBaseActiveSrc.fH != 24.0f))) ||
		     ((pKnobSurface != NULL) && ((tKnobSrc.fX != 5.0f) || (tKnobSrc.fY != 6.0f) || (tKnobSrc.fW != 25.0f) || (tKnobSrc.fH != 26.0f))) ||
		     ((pKnobActiveSurface != NULL) && ((tKnobActiveSrc.fX != 7.0f) || (tKnobActiveSrc.fY != 8.0f) || (tKnobActiveSrc.fW != 27.0f) || (tKnobActiveSrc.fH != 28.0f))) ||
		     ((pRippleSurface != NULL) && ((tRippleSrc.fX != 9.0f) || (tRippleSrc.fY != 10.0f) || (tRippleSrc.fW != 29.0f) || (tRippleSrc.fH != 30.0f))) ) {
			printf("xui_uidesign exercise-virtual-joystick-properties-failed id=%d right=%d/%.3f left=%d/%.3f metrics=%.3f/%.3f/%.3f atlas=%d colors=%08X/%08X/%08X/%08X/%08X/%08X/%08X surfaces=%p/%p/%p/%p/%p base=%.1f/%.1f/%.1f/%.1f baseActive=%.1f/%.1f/%.1f/%.1f knob=%.1f/%.1f/%.1f/%.1f knobActive=%.1f/%.1f/%.1f/%.1f ripple=%.1f/%.1f/%.1f/%.1f\n",
				iVirtualJoystick,
				bRightPressed,
				fRightValue,
				bLeftPressed,
				fLeftValue,
				fRadius,
				fKnobSize,
				fDeadZone,
				(pJoystickNode != NULL && pJoystickNode->pWidget != NULL) ? xuiVirtualJoystickGetUseBuiltinAtlas(pJoystickNode->pWidget) : -1,
				iBaseColor,
				iBaseActiveColor,
				iKnobColor,
				iKnobActiveColor,
				iRippleColor,
				iFocusColor,
				iDisabledColor,
				(void*)pBaseSurface,
				(void*)pBaseActiveSurface,
				(void*)pKnobSurface,
				(void*)pKnobActiveSurface,
				(void*)pRippleSurface,
				tBaseSrc.fX,
				tBaseSrc.fY,
				tBaseSrc.fW,
				tBaseSrc.fH,
				tBaseActiveSrc.fX,
				tBaseActiveSrc.fY,
				tBaseActiveSrc.fW,
				tBaseActiveSrc.fH,
				tKnobSrc.fX,
				tKnobSrc.fY,
				tKnobSrc.fW,
				tKnobSrc.fH,
				tKnobActiveSrc.fX,
				tKnobActiveSrc.fY,
				tKnobActiveSrc.fW,
				tKnobActiveSrc.fH,
				tRippleSrc.fX,
				tRippleSrc.fY,
				tRippleSrc.fW,
				tRippleSrc.fH);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "value.x", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "value.y", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.channels", "");
		if ( (xuiVirtualJoystickGetX(pJoystickNode->pWidget) != 1.0f) ||
		     (xuiVirtualJoystickGetY(pJoystickNode->pWidget) != 0.0f) ) {
			printf("xui_uidesign exercise-virtual-joystick-value-failed id=%d value=%.3f/%.3f\n",
				iVirtualJoystick,
				xuiVirtualJoystickGetX(pJoystickNode->pWidget),
				xuiVirtualJoystickGetY(pJoystickNode->pWidget));
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iVirtualJoystick, "data.channels", "right|true|0.75\nup|true|0.5\nleft|false|0.25");
	}
	if ( iListView != 0 ) {
		ui_design_node_t* pListNode = uiDesignModelGetNode(&pApp->tModel, iListView);
		float fItemHeight;
		float fPadding;
		float fBorderWidth;
		uint32_t iBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iRowColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iSelectedColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iTrackColor = 0u;
		uint32_t iThumbColor = 0u;
		uint32_t iScrollbarHoverColor = 0u;
		uint32_t iScrollbarActiveColor = 0u;
		uint32_t iScrollbarFocusColor = 0u;
		uint32_t iScrollbarDisabledColor = 0u;
		fItemHeight = fPadding = fBorderWidth = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "data.items", "Alpha\nBeta\nGamma");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "behavior.selectionMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "data.selected", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "value.scrollY", "0");
		if ( (pListNode == NULL) || (pListNode->pWidget == NULL) ||
		     (xuiListViewGetSelected(pListNode->pWidget) != 2) ||
		     (xuiListViewGetScroll(pListNode->pWidget) != 0.0f) ) {
			printf("xui_uidesign exercise-listview-selected-scroll-failed id=%d\n", iListView);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "data.items", "Alpha|true|true\nBeta|false|false\nGamma|true|true");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "behavior.selectionMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "behavior.repeatSelect", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "metrics.itemHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "metrics.padding", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.borderColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.focusColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.rowColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.hoverColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.selectedColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.textColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.disabledTextColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.trackColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.thumbColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.scrollbarHoverColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.scrollbarActiveColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.scrollbarFocusColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iListView, "appearance.scrollbarDisabledColor", "#E0E1E2");
		if ( pListNode != NULL && pListNode->pWidget != NULL ) {
			(void)xuiListViewGetMetrics(pListNode->pWidget, &fItemHeight, &fPadding, &fBorderWidth);
			(void)xuiListViewGetColors(pListNode->pWidget, &iBackgroundColor, &iBorderColor, &iFocusColor, &iRowColor, &iHoverColor, &iSelectedColor, &iTextColor, &iDisabledTextColor);
			(void)xuiListViewGetScrollbarColors(pListNode->pWidget, &iTrackColor, &iThumbColor, &iScrollbarHoverColor, &iScrollbarActiveColor, &iScrollbarFocusColor, &iScrollbarDisabledColor);
		}
		if ( (pListNode == NULL) || (pListNode->pWidget == NULL) ||
		     (xuiListViewGetItemCount(pListNode->pWidget) != 3) ||
		     (xuiListViewGetItemText(pListNode->pWidget, 2) == NULL) ||
		     (strcmp(xuiListViewGetItemText(pListNode->pWidget, 2), "Gamma") != 0) ||
		     (xuiListViewIsItemEnabled(pListNode->pWidget, 1) != 0) ||
		     (xuiListViewGetSelectionMode(pListNode->pWidget) != XUI_SELECTION_MULTI) ||
		     (xuiListViewIsItemSelected(pListNode->pWidget, 0) == 0) ||
		     (xuiListViewIsItemSelected(pListNode->pWidget, 2) == 0) ||
		     (xuiListViewGetNotifyRepeatSelect(pListNode->pWidget) == 0) ||
		     (xuiListViewGetScrollbarMode(pListNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (fItemHeight != 29.0f) || (fPadding != 11.0f) || (fBorderWidth != 2.0f) ) {
			printf("xui_uidesign exercise-listview-properties-failed id=%d\n", iListView);
			return XUI_ERROR;
		}
		if ( (iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iRowColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iSelectedColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iTrackColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (iScrollbarHoverColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (iScrollbarActiveColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (iScrollbarFocusColor != XUI_COLOR_RGBA(208, 209, 210, 255)) ||
		     (iScrollbarDisabledColor != XUI_COLOR_RGBA(224, 225, 226, 255)) ) {
			printf("xui_uidesign exercise-listview-colors-failed id=%d\n", iListView);
			return XUI_ERROR;
		}
	}
	if ( iTreeView != 0 ) {
		ui_design_node_t* pTreeNode = uiDesignModelGetNode(&pApp->tModel, iTreeView);
		const xui_tree_view_node_t* pTreeItem;
		float fItemHeight;
		float fIndent;
		float fPadding;
		float fBorderWidth;
		uint32_t iBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		uint32_t iRowColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iSelectedColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iExpanderColor = 0u;
		uint32_t iIconColor = 0u;
		uint32_t iCheckColor = 0u;
		uint32_t iTrackColor = 0u;
		uint32_t iThumbColor = 0u;
		uint32_t iScrollbarHoverColor = 0u;
		uint32_t iScrollbarActiveColor = 0u;
		uint32_t iScrollbarFocusColor = 0u;
		uint32_t iScrollbarDisabledColor = 0u;
		pTreeItem = NULL;
		fItemHeight = fIndent = fPadding = fBorderWidth = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "data.nodes", "10|-1|0|Root|true|true|true|false|false\n11|10|1|Child|false|true|true|true|true\n12|10|1|Disabled|true|false|false|true|false");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "data.selected", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "value.scrollY", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "metrics.itemHeight", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "metrics.indent", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "metrics.padding", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.backgroundColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.borderColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.focusColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.rowColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.hoverColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.selectedColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.textColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.disabledTextColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.expanderColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.iconColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.checkColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.trackColor", "#C1C2C3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.thumbColor", "#D1D2D3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.scrollbarHoverColor", "#E1E2E3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.scrollbarActiveColor", "#F1F2F3");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.scrollbarFocusColor", "#AABBCC");
		(void)uiDesignAppSetNodeProperty(pApp, iTreeView, "appearance.scrollbarDisabledColor", "#BBCCDD");
		if ( pTreeNode != NULL && pTreeNode->pWidget != NULL ) {
			pTreeItem = xuiTreeViewGetNodeById(pTreeNode->pWidget, 11);
			(void)xuiTreeViewGetMetrics(pTreeNode->pWidget, &fItemHeight, &fIndent, &fPadding, &fBorderWidth);
			(void)xuiTreeViewGetColors(pTreeNode->pWidget, &iBackgroundColor, &iBorderColor, &iFocusColor, &iRowColor, &iHoverColor, &iSelectedColor, &iTextColor, &iDisabledTextColor);
			(void)xuiTreeViewGetDecorationColors(pTreeNode->pWidget, &iExpanderColor, &iIconColor, &iCheckColor);
			(void)xuiTreeViewGetScrollbarColors(pTreeNode->pWidget, &iTrackColor, &iThumbColor, &iScrollbarHoverColor, &iScrollbarActiveColor, &iScrollbarFocusColor, &iScrollbarDisabledColor);
		}
		if ( (pTreeNode == NULL) || (pTreeNode->pWidget == NULL) ||
		     (xuiTreeViewGetNodeCount(pTreeNode->pWidget) != 3) ||
		     (pTreeItem == NULL) ||
		     (pTreeItem->sText == NULL) ||
		     (strcmp(pTreeItem->sText, "Child") != 0) ||
		     (pTreeItem->iParent != 10) || (pTreeItem->iDepth != 1) ||
		     (xuiTreeViewGetSelected(pTreeNode->pWidget) != 11) ||
		     (xuiTreeViewGetNodeExpanded(pTreeNode->pWidget, 10) == 0) ||
		     (xuiTreeViewGetNodeEnabled(pTreeNode->pWidget, 12) != 0) ||
		     (xuiTreeViewGetNodeChecked(pTreeNode->pWidget, 11) == 0) ||
		     (xuiTreeViewGetVisibleCount(pTreeNode->pWidget) != 3) ||
		     (xuiTreeViewGetVisibleNodeId(pTreeNode->pWidget, 1) != 11) ||
		     (xuiTreeViewGetScroll(pTreeNode->pWidget) != 0.0f) ||
		     (xuiTreeViewGetScrollbarMode(pTreeNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (fItemHeight != 28.0f) || (fIndent != 21.0f) || (fPadding != 10.0f) || (fBorderWidth != 2.0f) ) {
			printf("xui_uidesign exercise-treeview-properties-failed id=%d\n", iTreeView);
			return XUI_ERROR;
		}
		if ( (iBackgroundColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iRowColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iSelectedColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (iExpanderColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (iIconColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (iCheckColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ||
		     (iTrackColor != XUI_COLOR_RGBA(193, 194, 195, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(209, 210, 211, 255)) ||
		     (iScrollbarHoverColor != XUI_COLOR_RGBA(225, 226, 227, 255)) ||
		     (iScrollbarActiveColor != XUI_COLOR_RGBA(241, 242, 243, 255)) ||
		     (iScrollbarFocusColor != XUI_COLOR_RGBA(170, 187, 204, 255)) ||
		     (iScrollbarDisabledColor != XUI_COLOR_RGBA(187, 204, 221, 255)) ) {
			printf("xui_uidesign exercise-treeview-colors-failed id=%d\n", iTreeView);
			return XUI_ERROR;
		}
	}
	if ( iCanvas != 0 ) {
		ui_design_node_t* pCanvasNode = uiDesignModelGetNode(&pApp->tModel, iCanvas);
		xui_widget pFrameWidget;
		float fCanvasWidth;
		float fCanvasHeight;
		float fOffsetX;
		float fOffsetY;
		float fScrollbarSize;
		float fMinThumbSize;
		float fButtonSize;
		int bPenEnabled;
		float fPenWidth;
		uint32_t iBackgroundColor;
		uint32_t iBorderColor;
		uint32_t iPenColor;
		uint32_t iTrackColor;
		uint32_t iThumbColor;
		uint32_t iHoverColor;
		uint32_t iActiveColor;
		uint32_t iFocusColor;
		uint32_t iDisabledColor;
		uint32_t iButtonColor;
		uint32_t iButtonIconColor;
		uint32_t iCornerColor;
		uint32_t iGripColor;
		int iPolicyX;
		int iPolicyY;
		int iDrawBefore;
		int iDrawAfter;
		pFrameWidget = NULL;
		fCanvasWidth = fCanvasHeight = 0.0f;
		fOffsetX = fOffsetY = 0.0f;
		fScrollbarSize = fMinThumbSize = fButtonSize = 0.0f;
		bPenEnabled = 0;
		fPenWidth = 0.0f;
		iBackgroundColor = iBorderColor = iPenColor = 0u;
		iTrackColor = iThumbColor = iHoverColor = iActiveColor = iFocusColor = iDisabledColor = 0u;
		iButtonColor = iButtonIconColor = iCornerColor = iGripColor = 0u;
		iPolicyX = iPolicyY = -1;
		iDrawBefore = 0;
		iDrawAfter = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.canvasWidth", "640");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.canvasHeight", "480");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "value.offsetX", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "value.offsetY", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.policyX", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.policyY", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.scrollbarMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.wheelAxis", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.cornerMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.contentDrag", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "behavior.penEnabled", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.scrollbarSize", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.minThumbSize", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.buttonSize", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.wheelStep", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "metrics.penWidth", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.clearColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.backgroundColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.borderColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.penColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.trackColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.thumbColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.hoverColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.activeColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.focusColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.disabledColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.buttonColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.buttonIconColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.cornerColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "appearance.gripColor", "#E0E1E2");
		if ( pCanvasNode != NULL && pCanvasNode->pWidget != NULL ) {
			iDrawBefore = xuiCanvasGetDrawCount(pCanvasNode->pWidget);
		}
		(void)uiDesignAppSetNodeProperty(pApp, iCanvas, "data.commands", "clear|#101112\npoint|12|13|5|#A0A1A2\nline|20|22|88|92|3|#B0B1B2\nrectfill|32|34|44|46|#C0C1C2\ntext|42|44|120|24|Canvas OK|#D0D1D2|272");
		if ( pCanvasNode != NULL && pCanvasNode->pWidget != NULL ) {
			pFrameWidget = xuiCanvasGetFrameWidget(pCanvasNode->pWidget);
			(void)xuiCanvasGetCanvasSize(pCanvasNode->pWidget, &fCanvasWidth, &fCanvasHeight);
			(void)xuiCanvasGetOffset(pCanvasNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiCanvasGetPen(pCanvasNode->pWidget, &bPenEnabled, &fPenWidth, &iPenColor);
			(void)xuiCanvasGetColors(pCanvasNode->pWidget, &iBackgroundColor, &iBorderColor, NULL);
			if ( pFrameWidget != NULL ) (void)xuiScrollFrameGetScrollbarPolicy(pFrameWidget, &iPolicyX, &iPolicyY);
			if ( pFrameWidget != NULL ) (void)xuiScrollFrameGetMetrics(pFrameWidget, &fScrollbarSize, &fMinThumbSize, &fButtonSize);
			if ( pFrameWidget != NULL ) (void)xuiScrollFrameGetColors(pFrameWidget, &iTrackColor, &iThumbColor, &iHoverColor, &iActiveColor, &iFocusColor, &iDisabledColor);
			if ( pFrameWidget != NULL ) (void)xuiScrollFrameGetButtonColors(pFrameWidget, &iButtonColor, &iButtonIconColor);
			if ( pFrameWidget != NULL ) (void)xuiScrollFrameGetCornerColors(pFrameWidget, &iCornerColor, &iGripColor);
			iDrawAfter = xuiCanvasGetDrawCount(pCanvasNode->pWidget);
		}
		if ( (pCanvasNode == NULL) || (pCanvasNode->pWidget == NULL) || (pFrameWidget == NULL) ||
		     (fCanvasWidth != 640.0f) || (fCanvasHeight != 480.0f) ||
		     (fOffsetX != 17.0f) || (fOffsetY != 23.0f) ||
		     (bPenEnabled == 0) || (fPenWidth != 4.0f) ||
		     (iPenColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iPolicyX != XUI_SCROLLBAR_POLICY_ALWAYS) || (iPolicyY != XUI_SCROLLBAR_POLICY_HIDDEN) ||
		     (xuiScrollFrameGetScrollbarMode(pFrameWidget) != XUI_SCROLLBAR_MODE_FULL) ||
		     (xuiScrollFrameGetWheelAxis(pFrameWidget) != XUI_WHEEL_AXIS_HORIZONTAL) ||
		     (xuiScrollFrameGetWheelStep(pFrameWidget) != 33.0f) ||
		     (xuiScrollFrameIsContentDragEnabled(pFrameWidget) != 0) ||
		     (xuiScrollFrameGetCornerMode(pFrameWidget) != XUI_SCROLL_FRAME_CORNER_GRIP) ||
		     (fScrollbarSize != 15.0f) || (fMinThumbSize != 26.0f) || (fButtonSize != 17.0f) ||
		     (iTrackColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (iButtonIconColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (iCornerColor != XUI_COLOR_RGBA(208, 209, 210, 255)) ||
		     (iGripColor != XUI_COLOR_RGBA(224, 225, 226, 255)) ) {
			printf("xui_uidesign exercise-canvas-properties-failed id=%d size=%.1f/%.1f offset=%.1f/%.1f pen=%d/%.1f/%08X colors=%08X/%08X policy=%d/%d mode=%d wheel=%d/%.1f drag=%d corner=%d metrics=%.1f/%.1f/%.1f scroll=%08X/%08X/%08X/%08X/%08X/%08X button=%08X/%08X cornerColor=%08X/%08X draw=%d/%d\n",
				iCanvas,
				fCanvasWidth,
				fCanvasHeight,
				fOffsetX,
				fOffsetY,
				bPenEnabled,
				fPenWidth,
				iPenColor,
				iBackgroundColor,
				iBorderColor,
				iPolicyX,
				iPolicyY,
				xuiScrollFrameGetScrollbarMode(pFrameWidget),
				xuiScrollFrameGetWheelAxis(pFrameWidget),
				xuiScrollFrameGetWheelStep(pFrameWidget),
				xuiScrollFrameIsContentDragEnabled(pFrameWidget),
				xuiScrollFrameGetCornerMode(pFrameWidget),
				fScrollbarSize,
				fMinThumbSize,
				fButtonSize,
				iTrackColor,
				iThumbColor,
				iHoverColor,
				iActiveColor,
				iFocusColor,
				iDisabledColor,
				iButtonColor,
				iButtonIconColor,
				iCornerColor,
				iGripColor,
				iDrawBefore,
				iDrawAfter);
			return XUI_ERROR;
		}
	}
	if ( iTextEdit != 0 ) {
		ui_design_node_t* pTextNode = uiDesignModelGetNode(&pApp->tModel, iTextEdit);
		const char* sText;
		const char* sPlaceholder;
		const char* sCopyTitle;
		const char* sAllTitle;
		int iSelectionStart;
		int iSelectionEnd;
		float fBorderWidth = 0.0f;
		float fLineGap = 0.0f;
		uint32_t iBackgroundColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iPlaceholderColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iSelectionColor = 0u;
		uint32_t iCursorColor = 0u;
		uint32_t iFindResultColor = 0u;
		uint32_t iFindActiveColor = 0u;
		uint32_t iLineNumberColor = 0u;
		uint32_t iLineNumberBackgroundColor = 0u;
		uint32_t iLineNumberBorderColor = 0u;
		float fScrollX = -1.0f;
		float fScrollY = -1.0f;
		iSelectionStart = iSelectionEnd = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "data.text", "Alpha\nBeta\nAlpha");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "text.placeholder", "Type here");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "text.maxLength", "80");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "behavior.readonly", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "behavior.wordWrap", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "behavior.lineNumbers", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "behavior.findWindow", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "metrics.lineNumberWidth", "52");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "metrics.borderWidth", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "metrics.lineGap", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "value.selectionStart", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "value.selectionEnd", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "value.scrollX", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "value.scrollY", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.pattern", "Alpha");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.replacement", "Omega");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.caseSensitive", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.wholeWord", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.regex", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.escape", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.selection", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.wrap", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.backward", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.startOffset", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.rangeStart", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.rangeEnd", "16");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "find.previewMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.backgroundColor", "#151617");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.hoverBackgroundColor", "#252627");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.disabledBackgroundColor", "#353637");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.textColor", "#454647");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.placeholderColor", "#555657");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.disabledTextColor", "#656667");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.borderColor", "#757677");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.hoverBorderColor", "#858687");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.focusBorderColor", "#959697");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.selectionColor", "#A5A6A7A8");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.cursorColor", "#B5B6B7");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.findResultColor", "#C5C6C7A9");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.findActiveColor", "#D5D6D7AA");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.lineNumberColor", "#E5E6E7");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.lineNumberBackgroundColor", "#F5F6F7");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "appearance.lineNumberBorderColor", "#C8C9CA");
		(void)uiDesignAppSetNodeProperty(pApp, iTextEdit, "data.menuTitles", "copy|Copy Text\nselectAll|Select Everything");
		sText = (pTextNode != NULL && pTextNode->pWidget != NULL) ? xuiTextEditGetText(pTextNode->pWidget) : NULL;
		sPlaceholder = (pTextNode != NULL && pTextNode->pWidget != NULL) ? xuiTextEditGetPlaceholder(pTextNode->pWidget) : NULL;
		sCopyTitle = (pTextNode != NULL && pTextNode->pWidget != NULL) ? xuiTextEditGetMenuTitle(pTextNode->pWidget, XUI_INPUT_MENU_COPY) : NULL;
		sAllTitle = (pTextNode != NULL && pTextNode->pWidget != NULL) ? xuiTextEditGetMenuTitle(pTextNode->pWidget, XUI_INPUT_MENU_SELECT_ALL) : NULL;
		if ( pTextNode != NULL && pTextNode->pWidget != NULL ) {
			(void)xuiTextEditGetSelection(pTextNode->pWidget, &iSelectionStart, &iSelectionEnd);
			(void)xuiTextEditGetScroll(pTextNode->pWidget, &fScrollX, &fScrollY);
			(void)xuiTextEditGetVisualMetrics(pTextNode->pWidget, &fBorderWidth, &fLineGap);
			(void)xuiTextEditGetColors(pTextNode->pWidget, &iBackgroundColor, &iTextColor, &iBorderColor, &iFocusBorderColor);
			(void)xuiTextEditGetExtendedColors(pTextNode->pWidget, &iPlaceholderColor, &iDisabledTextColor,
				&iHoverBackgroundColor, &iDisabledBackgroundColor, &iHoverBorderColor, &iSelectionColor,
				&iCursorColor, &iFindResultColor, &iFindActiveColor);
			(void)xuiTextEditGetLineNumberColors(pTextNode->pWidget, &iLineNumberColor,
				&iLineNumberBackgroundColor, &iLineNumberBorderColor);
		}
		if ( (pTextNode == NULL) || (pTextNode->pWidget == NULL) ||
		     (sText == NULL) || (strcmp(sText, "Alpha\nBeta\nAlpha") != 0) ||
		     (sPlaceholder == NULL) || (strcmp(sPlaceholder, "Type here") != 0) ||
		     (xuiTextEditGetMaxLength(pTextNode->pWidget) != 80) ||
		     (xuiTextEditIsReadonly(pTextNode->pWidget) == 0) ||
		     (xuiTextEditGetWordWrap(pTextNode->pWidget) != 0) ||
		     (xuiTextEditGetLineNumbers(pTextNode->pWidget) == 0) ||
		     (xuiTextEditGetLineNumberWidth(pTextNode->pWidget) != 52.0f) ||
		     (fBorderWidth != 5.0f) ||
		     (fLineGap != 4.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(21, 22, 23, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(69, 70, 71, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(117, 118, 119, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(149, 150, 151, 255)) ||
		     (iPlaceholderColor != XUI_COLOR_RGBA(85, 86, 87, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(101, 102, 103, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(37, 38, 39, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(53, 54, 55, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(133, 134, 135, 255)) ||
		     (iSelectionColor != XUI_COLOR_RGBA(165, 166, 167, 168)) ||
		     (iCursorColor != XUI_COLOR_RGBA(181, 182, 183, 255)) ||
		     (iFindResultColor != XUI_COLOR_RGBA(197, 198, 199, 169)) ||
		     (iFindActiveColor != XUI_COLOR_RGBA(213, 214, 215, 170)) ||
		     (iLineNumberColor != XUI_COLOR_RGBA(229, 230, 231, 255)) ||
		     (iLineNumberBackgroundColor != XUI_COLOR_RGBA(245, 246, 247, 255)) ||
		     (iLineNumberBorderColor != XUI_COLOR_RGBA(200, 201, 202, 255)) ||
		     (xuiTextEditGetLineCount(pTextNode->pWidget) != 3) ||
		     (iSelectionStart != 11) || (iSelectionEnd != 16) ||
		     (fScrollX != 0.0f) || (fScrollY != 0.0f) ||
		     (sCopyTitle == NULL) || (strcmp(sCopyTitle, "Copy Text") != 0) ||
		     (sAllTitle == NULL) || (strcmp(sAllTitle, "Select Everything") != 0) ||
		     (xuiTextEditGetFindWindow(pTextNode->pWidget) == NULL) ) {
			printf("xui_uidesign exercise-text-edit-properties-failed id=%d selection=%d/%d scroll=%.2f/%.2f\n",
				iTextEdit,
				iSelectionStart,
				iSelectionEnd,
				(double)fScrollX,
				(double)fScrollY);
			return XUI_ERROR;
		}
	}
	if ( iCodeEdit != 0 ) {
		ui_design_node_t* pCodeNode = uiDesignModelGetNode(&pApp->tModel, iCodeEdit);
		xui_code_annotation_store pAnnotations;
		xui_code_fold_state pFoldState;
		xui_code_margin_model pMargins;
		xui_code_selection_model pSelection;
		xui_code_marker_t arrMarkers[4];
		xui_code_indicator_t arrIndicators[4];
		xui_code_diagnostic_t tDiagnostic;
		xui_code_fold_range_t tFold;
		xui_code_margin_info_t tMargin;
		xui_code_selection_t tSelection;
		xui_code_style_t tStyle;
		xui_code_style_t tTokenStyle;
		xui_code_style_t tDefaultStyle;
		xui_code_style_t tTypeStyle;
		xui_code_style_t tSelectionStyle;
		xui_code_style_t tSearchStyle;
		xui_code_style_t tDiagnosticInfoStyle;
		xui_code_find_result_t tFind;
		const char* sText;
		const char* sLanguage;
		const char* sFindTitle;
		const char* sReplaceTitle;
		uint32_t iOptions;
		int iMarkerCount;
		int iIndicatorCount;
		int bFoundIndicator;
		int iRangeStart;
		int iRangeEnd;
		float fScrollX;
		float fScrollY;
		int bDefaultLineNumbers;
		int bDefaultMarkerMargin;
		int bDefaultFoldMargin;
		int bDefaultDiagnosticMargin;
		memset(arrMarkers, 0, sizeof(arrMarkers));
		memset(arrIndicators, 0, sizeof(arrIndicators));
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		memset(&tFold, 0, sizeof(tFold));
		memset(&tMargin, 0, sizeof(tMargin));
		memset(&tSelection, 0, sizeof(tSelection));
		memset(&tStyle, 0, sizeof(tStyle));
		memset(&tTokenStyle, 0, sizeof(tTokenStyle));
		memset(&tDefaultStyle, 0, sizeof(tDefaultStyle));
		memset(&tTypeStyle, 0, sizeof(tTypeStyle));
		memset(&tSelectionStyle, 0, sizeof(tSelectionStyle));
		memset(&tSearchStyle, 0, sizeof(tSearchStyle));
		memset(&tDiagnosticInfoStyle, 0, sizeof(tDiagnosticInfoStyle));
		memset(&tFind, 0, sizeof(tFind));
		for ( i = 0; i < 4; ++i ) {
			arrMarkers[i].iSize = sizeof(arrMarkers[i]);
			arrIndicators[i].iSize = sizeof(arrIndicators[i]);
		}
		tDiagnostic.iSize = sizeof(tDiagnostic);
		tFold.iSize = sizeof(tFold);
		tMargin.iSize = sizeof(tMargin);
		tSelection.iSize = sizeof(tSelection);
		tStyle.iSize = sizeof(tStyle);
		tTokenStyle.iSize = sizeof(tTokenStyle);
		tDefaultStyle.iSize = sizeof(tDefaultStyle);
		tTypeStyle.iSize = sizeof(tTypeStyle);
		tSelectionStyle.iSize = sizeof(tSelectionStyle);
		tSearchStyle.iSize = sizeof(tSearchStyle);
		tDiagnosticInfoStyle.iSize = sizeof(tDiagnosticInfoStyle);
		tFind.iSize = sizeof(tFind);
		iMarkerCount = 0;
		iIndicatorCount = 0;
		bFoundIndicator = 0;
		iRangeStart = iRangeEnd = -1;
		fScrollX = fScrollY = -1.0f;
		bDefaultLineNumbers = bDefaultMarkerMargin = bDefaultFoldMargin = bDefaultDiagnosticMargin = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.text", "int alpha = 1;\nint beta = alpha;\n");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "text.language", "c");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.readonly", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.wordWrap", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.eolMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.showWhitespace", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.showEol", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.indentGuides", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.expandTabs", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.lineNumbers", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.markerMargin", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.foldMargin", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.diagnosticMargin", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.findWindow", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "metrics.tabColumns", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "metrics.indentColumns", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "value.scrollX", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "value.scrollY", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.margins", "11|lineNumber|42|true|false|false\n12|marker|18|true|true|false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.markers", "1|breakpoint|7|Here");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.indicators", "2|searchResult|4|9|3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.diagnostics", "warning|4|9|demo.warning|Warn text|UIDesign");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.folds", "1|2|0|header+collapsed");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.selections", "4|9|-1|");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.menuTitles", "find|Find Text\nreplace|Replace Text");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.styles", "keyword|#010203|#040506|true|false|true|false\ncomment|#111213|#00000000|false|true|false|false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.tokenStyles", "keyword|comment");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.pattern", "alpha");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.replacement", "ALPHA");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.caseSensitive", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.wholeWord", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.regex", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.escape", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.selection", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.wrap", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.backward", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.startOffset", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.rangeStart", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.rangeEnd", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "find.previewMode", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.backgroundColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.textColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.keywordColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.typeColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.identifierColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.numberColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.stringColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.charColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.commentColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.preprocessorColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.operatorColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.braceColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.errorColor", "#E0E1E2");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.selectionTextColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.selectionColor", "#11121314");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.currentLineColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.searchTextColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.searchColor", "#41424344");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.matchedBraceColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.matchedBraceBackground", "#61626364");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.marginColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.lineNumberColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.foldIconColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.markerColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.diagnosticErrorColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.diagnosticWarningColor", "#C1C2C3");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "theme.diagnosticInfoColor", "#D1D2D3");
		sText = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetText(pCodeNode->pWidget) : NULL;
		sLanguage = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetLanguage(pCodeNode->pWidget) : NULL;
		sFindTitle = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetMenuTitle(pCodeNode->pWidget, XUI_CODE_COMMAND_OPEN_FIND) : NULL;
		sReplaceTitle = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetMenuTitle(pCodeNode->pWidget, XUI_CODE_COMMAND_OPEN_REPLACE) : NULL;
		iOptions = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetDisplayOptions(pCodeNode->pWidget) : 0u;
		pAnnotations = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetAnnotations(pCodeNode->pWidget) : NULL;
		pFoldState = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetFoldState(pCodeNode->pWidget) : NULL;
		pMargins = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetMargins(pCodeNode->pWidget) : NULL;
		pSelection = (pCodeNode != NULL && pCodeNode->pWidget != NULL) ? xuiCodeEditGetSelection(pCodeNode->pWidget) : NULL;
		if ( pCodeNode != NULL && pCodeNode->pWidget != NULL ) {
			(void)xuiCodeEditGetScroll(pCodeNode->pWidget, &fScrollX, &fScrollY);
		}
		if ( pAnnotations != NULL ) {
			(void)xuiCodeAnnotationGetMarkers(pAnnotations, 0, arrMarkers, 4, &iMarkerCount);
			(void)xuiCodeAnnotationGetIndicatorsAt(pAnnotations, 5, arrIndicators, 4, &iIndicatorCount);
			(void)xuiCodeAnnotationGetDiagnostic(pAnnotations, 0, &tDiagnostic);
		}
		for ( i = 0; i < iIndicatorCount && i < 4; ++i ) {
			if ( (arrIndicators[i].iIndicator == 2) &&
			     (arrIndicators[i].iStyle == XUI_CODE_INDICATOR_SEARCH_RESULT) &&
			     (arrIndicators[i].tRange.iStart == 4) &&
			     (arrIndicators[i].tRange.iEnd == 9) &&
			     (arrIndicators[i].iFlags == 3u) ) {
				bFoundIndicator = 1;
			}
		}
		if ( pFoldState != NULL ) (void)xuiCodeFoldStateGetRange(pFoldState, 0, &tFold);
		if ( pMargins != NULL ) (void)xuiCodeMarginModelGet(pMargins, 0, &tMargin);
		if ( pSelection != NULL ) {
			(void)xuiCodeSelectionGetRange(pSelection, &iRangeStart, &iRangeEnd);
			(void)xuiCodeSelectionGetState(pSelection, &tSelection);
		}
		if ( pCodeNode != NULL && pCodeNode->pWidget != NULL ) {
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_KEYWORD, &tStyle);
			(void)xuiCodeThemeGetTokenStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_TOKEN_KEYWORD, &tTokenStyle);
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_DEFAULT, &tDefaultStyle);
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_TYPE, &tTypeStyle);
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_SELECTION, &tSelectionStyle);
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_SEARCH_RESULT, &tSearchStyle);
			(void)xuiCodeThemeGetStyle(xuiCodeEditGetTheme(pCodeNode->pWidget), XUI_CODE_STYLE_DIAGNOSTIC_INFO, &tDiagnosticInfoStyle);
			(void)xuiCodeEditGetFindResult(pCodeNode->pWidget, 0, &tFind);
		}
		if ( (pCodeNode == NULL) || (pCodeNode->pWidget == NULL) ||
		     (sText == NULL) || (strcmp(sText, "int alpha = 1;\nint beta = alpha;\n") != 0) ||
		     (sLanguage == NULL) || (strcmp(sLanguage, "c") != 0) ||
		     (xuiCodeEditIsReadonly(pCodeNode->pWidget) == 0) ||
		     (xuiCodeEditGetWordWrap(pCodeNode->pWidget) == 0) ||
		     (xuiCodeEditGetEolMode(pCodeNode->pWidget) != XUI_CODE_EOL_CRLF) ||
		     ((iOptions & (XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL)) != (XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL)) ||
		     ((iOptions & (XUI_CODE_EDIT_SHOW_INDENT_GUIDES | XUI_CODE_EDIT_EXPAND_TABS)) != 0u) ||
		     (xuiCodeEditGetTabColumns(pCodeNode->pWidget) != 2) ||
		     (xuiCodeEditGetIndentColumns(pCodeNode->pWidget) != 3) ||
		     (xuiCodeEditGetExpandTabs(pCodeNode->pWidget) != 0) ||
		     (fScrollX != 0.0f) || (fScrollY != 0.0f) ||
		     (sFindTitle == NULL) || (strcmp(sFindTitle, "Find Text") != 0) ||
		     (sReplaceTitle == NULL) || (strcmp(sReplaceTitle, "Replace Text") != 0) ||
		     (xuiCodeEditGetFindWindow(pCodeNode->pWidget) == NULL) ) {
			printf("xui_uidesign exercise-code-edit-core-failed id=%d options=%u\n", iCodeEdit, iOptions);
			return XUI_ERROR;
		}
		if ( (pMargins == NULL) || (xuiCodeMarginModelGetCount(pMargins) != 2) ||
		     (tMargin.iId != 11) || (tMargin.iKind != XUI_CODE_MARGIN_LINE_NUMBER) ||
		     (tMargin.fWidth != 42.0f) ||
		     ((tMargin.iFlags & XUI_CODE_MARGIN_VISIBLE) == 0u) ||
		     ((tMargin.iFlags & (XUI_CODE_MARGIN_CLICKABLE | XUI_CODE_MARGIN_RESIZABLE)) != 0u) ) {
			printf("xui_uidesign exercise-code-edit-margins-failed id=%d\n", iCodeEdit);
			return XUI_ERROR;
		}
		if ( (pAnnotations == NULL) ||
		     (iMarkerCount != 1) ||
		     (arrMarkers[0].iLine != 0) ||
		     (arrMarkers[0].iMarker != XUI_CODE_MARKER_BREAKPOINT) ||
		     (arrMarkers[0].iFlags != 7u) ||
		     (arrMarkers[0].sTooltip == NULL) || (strcmp(arrMarkers[0].sTooltip, "Here") != 0) ||
		     (iIndicatorCount < 1) ||
		     (bFoundIndicator == 0) ||
		     (xuiCodeAnnotationGetDiagnosticCount(pAnnotations) != 1) ||
		     (tDiagnostic.iSeverity != XUI_CODE_DIAGNOSTIC_WARNING) ||
		     (tDiagnostic.tRange.iStart != 4) || (tDiagnostic.tRange.iEnd != 9) ||
		     (tDiagnostic.sCode == NULL) || (strcmp(tDiagnostic.sCode, "demo.warning") != 0) ||
		     (tDiagnostic.sMessage == NULL) || (strcmp(tDiagnostic.sMessage, "Warn text") != 0) ||
		     (tDiagnostic.sSource == NULL) || (strcmp(tDiagnostic.sSource, "UIDesign") != 0) ) {
			printf("xui_uidesign exercise-code-edit-annotations-failed id=%d markers=%d indicators=%d diagnostics=%d\n",
				iCodeEdit,
				iMarkerCount,
				iIndicatorCount,
				(pAnnotations != NULL) ? xuiCodeAnnotationGetDiagnosticCount(pAnnotations) : -1);
			return XUI_ERROR;
		}
		if ( (pFoldState == NULL) || (xuiCodeFoldStateGetCount(pFoldState) != 1) ||
		     (tFold.iStartLine != 0) || (tFold.iEndLine != 1) || (tFold.iLevel != 0) ||
		     ((tFold.iFlags & (XUI_CODE_FOLD_HEADER | XUI_CODE_FOLD_COLLAPSED)) != (XUI_CODE_FOLD_HEADER | XUI_CODE_FOLD_COLLAPSED)) ||
		     (pSelection == NULL) || (xuiCodeSelectionGetCount(pSelection) != 1) ||
		     (iRangeStart != 4) || (iRangeEnd != 9) ||
		     (tSelection.iAnchorOffset != 4) || (tSelection.iCaretOffset != 9) ) {
			printf("xui_uidesign exercise-code-edit-state-failed id=%d range=%d/%d folds=%d selections=%d\n",
				iCodeEdit,
				iRangeStart,
				iRangeEnd,
				(pFoldState != NULL) ? xuiCodeFoldStateGetCount(pFoldState) : -1,
				(pSelection != NULL) ? xuiCodeSelectionGetCount(pSelection) : -1);
			return XUI_ERROR;
		}
		if ( (tStyle.iForeground != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (tStyle.iBackground != XUI_COLOR_RGBA(4, 5, 6, 255)) ||
		     ((tStyle.iFlags & (XUI_CODE_STYLE_BOLD | XUI_CODE_STYLE_UNDERLINE)) != (XUI_CODE_STYLE_BOLD | XUI_CODE_STYLE_UNDERLINE)) ||
		     (tTokenStyle.iForeground != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     ((tTokenStyle.iFlags & XUI_CODE_STYLE_ITALIC) == 0u) ||
		     (tDefaultStyle.iForeground != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (tDefaultStyle.iBackground != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tTypeStyle.iForeground != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (tSelectionStyle.iForeground != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (tSelectionStyle.iBackground != XUI_COLOR_RGBA(17, 18, 19, 20)) ||
		     (tSearchStyle.iForeground != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (tSearchStyle.iBackground != XUI_COLOR_RGBA(65, 66, 67, 68)) ||
		     (tDiagnosticInfoStyle.iForeground != XUI_COLOR_RGBA(209, 210, 211, 255)) ) {
			printf("xui_uidesign exercise-code-edit-theme-failed id=%d\n", iCodeEdit);
			return XUI_ERROR;
		}
		if ( (xuiCodeEditGetFindResultCount(pCodeNode->pWidget) != 2) ||
		     (tFind.iStart != 4) || (tFind.iEnd != 9) || (tFind.iLine != 0) || (tFind.iColumn != 4) ) {
			printf("xui_uidesign exercise-code-edit-find-failed id=%d count=%d first=%d/%d line=%d col=%d\n",
				iCodeEdit,
				xuiCodeEditGetFindResultCount(pCodeNode->pWidget),
				tFind.iStart,
				tFind.iEnd,
				tFind.iLine,
				tFind.iColumn);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "data.margins", "");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.lineNumbers", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.markerMargin", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.foldMargin", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCodeEdit, "behavior.diagnosticMargin", "true");
		if ( pCodeNode != NULL && pCodeNode->pWidget != NULL ) {
			(void)xuiCodeEditGetDefaultMargins(pCodeNode->pWidget, &bDefaultLineNumbers,
				&bDefaultMarkerMargin, &bDefaultFoldMargin, &bDefaultDiagnosticMargin);
		}
		if ( (bDefaultLineNumbers != 0) || (bDefaultMarkerMargin == 0) ||
		     (bDefaultFoldMargin != 0) || (bDefaultDiagnosticMargin == 0) ) {
			printf("xui_uidesign exercise-code-edit-default-margins-failed id=%d margins=%d/%d/%d/%d\n",
				iCodeEdit,
				bDefaultLineNumbers,
				bDefaultMarkerMargin,
				bDefaultFoldMargin,
				bDefaultDiagnosticMargin);
			return XUI_ERROR;
		}
	}
	if ( iWindow != 0 ) {
		ui_design_node_t* pWindowNode = uiDesignModelGetNode(&pApp->tModel, iWindow);
		float fTitleBarHeight;
		float fBorderWidth;
		float fResizeGrip;
		float fButtonSize;
		float fMinWidth;
		float fMinHeight;
		uint32_t iBackground;
		uint32_t iClient;
		uint32_t iTitleBar;
		uint32_t iTitleText;
		uint32_t iBorder;
		uint32_t iButtonNormal;
		uint32_t iButtonHover;
		uint32_t iButtonActive;
		uint32_t iInactiveTitleBar;
		uint32_t iInactiveTitleText;
		uint32_t iActiveBorder;
		uint32_t iCloseHover;
		uint32_t iCloseActive;
		xui_surface pWindowIconSurface;
		xui_rect_t tWindowIconSrc;
		int bCollapsedApplied;
		fTitleBarHeight = fBorderWidth = fResizeGrip = fButtonSize = 0.0f;
		fMinWidth = fMinHeight = 0.0f;
		iBackground = iClient = iTitleBar = iTitleText = iBorder = 0u;
		iButtonNormal = iButtonHover = iButtonActive = 0u;
		iInactiveTitleBar = iInactiveTitleText = iActiveBorder = iCloseHover = iCloseActive = 0u;
		pWindowIconSurface = NULL;
		memset(&tWindowIconSrc, 0, sizeof(tWindowIconSrc));
		bCollapsedApplied = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "text.title", "Inspector Window");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.topMost", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.showTitleBar", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.movable", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.dragAnywhere", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.resizable", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.resizeEdges", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.showCollapse", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.showMaximize", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.showClose", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.collapsed", "true");
		if ( pWindowNode != NULL && pWindowNode->pWidget != NULL ) bCollapsedApplied = xuiWindowIsCollapsed(pWindowNode->pWidget);
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "behavior.maximized", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "data.iconSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "icon.x", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "icon.y", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "icon.w", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "icon.h", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.titleBarHeight", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.resizeGrip", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.buttonSize", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.iconSize", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.minWidth", "140");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "metrics.minHeight", "90");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.clientColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.titleBarColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.titleTextColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.borderColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.buttonColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.buttonHoverColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.buttonActiveColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.inactiveTitleBarColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.inactiveTitleTextColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.activeBorderColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.closeHoverColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iWindow, "appearance.closeActiveColor", "#D0D1D2");
		if ( pWindowNode != NULL && pWindowNode->pWidget != NULL ) {
			(void)xuiWindowGetChrome(pWindowNode->pWidget, &fTitleBarHeight, &fBorderWidth, &fResizeGrip, &fButtonSize);
			(void)xuiWindowGetMinSize(pWindowNode->pWidget, &fMinWidth, &fMinHeight);
			(void)xuiWindowGetColors(pWindowNode->pWidget, &iBackground, &iClient, &iTitleBar, &iTitleText, &iBorder, &iButtonNormal, &iButtonHover, &iButtonActive);
			(void)xuiWindowGetStateColors(pWindowNode->pWidget, &iInactiveTitleBar, &iInactiveTitleText, &iActiveBorder, &iCloseHover, &iCloseActive);
			pWindowIconSurface = xuiWindowGetIconSurface(pWindowNode->pWidget);
			tWindowIconSrc = xuiWindowGetIconSource(pWindowNode->pWidget);
		}
		if ( (pWindowNode == NULL) || (pWindowNode->pWidget == NULL) ||
		     (xuiWindowGetTitle(pWindowNode->pWidget) == NULL) ||
		     (strcmp(xuiWindowGetTitle(pWindowNode->pWidget), "Inspector Window") != 0) ||
		     (xuiWidgetGetVisible(pWindowNode->pWidget) == 0) ||
		     (xuiWindowIsTopMost(pWindowNode->pWidget) == 0) ||
		     (bCollapsedApplied == 0) ||
		     (xuiWindowIsCollapsed(pWindowNode->pWidget) != 0) ||
		     (xuiWindowIsMaximized(pWindowNode->pWidget) == 0) ||
		     (xuiWindowGetShowTitleBar(pWindowNode->pWidget) != 0) ||
		     (xuiWindowIsMovable(pWindowNode->pWidget) != 0) ||
		     (xuiWindowIsDragAnywhere(pWindowNode->pWidget) == 0) ||
		     (xuiWindowIsResizable(pWindowNode->pWidget) != 0) ||
		     (xuiWindowGetResizeEdges(pWindowNode->pWidget) != 5u) ||
		     (xuiWindowGetShowCollapse(pWindowNode->pWidget) != 0) ||
		     (xuiWindowGetShowMaximize(pWindowNode->pWidget) != 0) ||
		     (xuiWindowGetShowClose(pWindowNode->pWidget) == 0) ||
		     (fTitleBarHeight != 31.0f) || (fBorderWidth != 2.0f) ||
		     (fResizeGrip != 7.0f) || (fButtonSize != 23.0f) ||
		     (xuiWindowGetIconSize(pWindowNode->pWidget) != 15.0f) ||
		     (pWindowIconSurface == NULL) ||
		     (tWindowIconSrc.fX != 3.0f) || (tWindowIconSrc.fY != 4.0f) ||
		     (tWindowIconSrc.fW != 25.0f) || (tWindowIconSrc.fH != 26.0f) ||
		     (fMinWidth != 140.0f) || (fMinHeight != 90.0f) ||
		     (iBackground != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iClient != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iTitleBar != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iTitleText != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iBorder != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iButtonNormal != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iButtonHover != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iButtonActive != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iInactiveTitleBar != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iInactiveTitleText != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (iActiveBorder != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (iCloseHover != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (iCloseActive != XUI_COLOR_RGBA(208, 209, 210, 255)) ) {
			printf("xui_uidesign exercise-window-properties-failed id=%d collapsed=%d/%d maximized=%d icon=%p src=%.1f/%.1f/%.1f/%.1f chrome=%.1f/%.1f/%.1f/%.1f\n",
				iWindow,
				bCollapsedApplied,
				(pWindowNode != NULL && pWindowNode->pWidget != NULL) ? xuiWindowIsCollapsed(pWindowNode->pWidget) : -1,
				(pWindowNode != NULL && pWindowNode->pWidget != NULL) ? xuiWindowIsMaximized(pWindowNode->pWidget) : -1,
				(void*)pWindowIconSurface,
				tWindowIconSrc.fX,
				tWindowIconSrc.fY,
				tWindowIconSrc.fW,
				tWindowIconSrc.fH,
				fTitleBarHeight,
				fBorderWidth,
				fResizeGrip,
				fButtonSize);
			return XUI_ERROR;
		}
	}
	if ( iPopup != 0 ) {
		ui_design_node_t* pPopupNode = uiDesignModelGetNode(&pApp->tModel, iPopup);
		xui_widget pContentWidget;
		xui_widget pScrollViewWidget;
		xui_widget pPopupChildWidget;
		const char* sPopupChildText;
		float fContentWidth;
		float fContentHeight;
		float fMaxWidth;
		float fMaxHeight;
		float fOffsetX;
		float fOffsetY;
		float fScrollX;
		float fScrollY;
		float fPadding;
		float fBorderWidth;
		float fShadowSize;
		float fScrollbarSize;
		int iOutsidePolicy;
		int iOwnerPolicy;
		int iEscapePolicy;
		int iScrollbarMode;
		uint32_t iPanelColor;
		uint32_t iBorderColor;
		uint32_t iShadowColor;
		uint32_t iBackdropColor;
		uint32_t iTrackColor;
		uint32_t iThumbColor;
		uint32_t iHoverColor;
		uint32_t iActiveColor;
		uint32_t iFocusColor;
		uint32_t iDisabledColor;
		fContentWidth = fContentHeight = 0.0f;
		fMaxWidth = fMaxHeight = 0.0f;
		fOffsetX = fOffsetY = 0.0f;
		fScrollX = fScrollY = 0.0f;
		fPadding = fBorderWidth = fShadowSize = 0.0f;
		fScrollbarSize = 0.0f;
		iOutsidePolicy = iOwnerPolicy = iEscapePolicy = -1;
		iScrollbarMode = -1;
		iPanelColor = iBorderColor = iShadowColor = iBackdropColor = 0u;
		iTrackColor = iThumbColor = iHoverColor = iActiveColor = iFocusColor = iDisabledColor = 0u;
		pPopupChildWidget = NULL;
		sPopupChildText = NULL;
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "data.content", "Alpha row|14|16|120|22|#A1B2C3|left\nBeta row|18|44|132|24|#B2C3D4|right");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.modal", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.consumeInside", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.matchOwnerWidth", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.anchor", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.direction", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.outsidePolicy", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.ownerPolicy", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.escapePolicy", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.focusPolicy", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "behavior.scrollbarMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "value.scrollX", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "value.scrollY", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.contentWidth", "520");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.contentHeight", "340");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.maxWidth", "260");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.maxHeight", "190");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.gap", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.offsetX", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.offsetY", "-4");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.margin", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.padding", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.shadowSize", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "metrics.scrollbarSize", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.panelColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.borderColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.shadowColor", "#30405060");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.backdropColor", "#40506070");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.trackColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.thumbColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.hoverColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.activeColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.focusColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iPopup, "appearance.disabledColor", "#A0A1A2");
		pContentWidget = (pPopupNode != NULL && pPopupNode->pWidget != NULL) ? xuiPopupGetContentWidget(pPopupNode->pWidget) : NULL;
		pScrollViewWidget = (pPopupNode != NULL && pPopupNode->pWidget != NULL) ? xuiPopupGetScrollViewWidget(pPopupNode->pWidget) : NULL;
		if ( pContentWidget != NULL ) {
			pPopupChildWidget = xuiWidgetGetFirstChild(pContentWidget);
			sPopupChildText = __uiDesignExerciseWidgetText(pApp, pPopupChildWidget);
		}
		if ( pPopupNode != NULL && pPopupNode->pWidget != NULL ) {
			(void)xuiPopupGetContentSize(pPopupNode->pWidget, &fContentWidth, &fContentHeight);
			(void)xuiPopupGetMaxSize(pPopupNode->pWidget, &fMaxWidth, &fMaxHeight);
			(void)xuiPopupGetOffset(pPopupNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiPopupGetScroll(pPopupNode->pWidget, &fScrollX, &fScrollY);
			(void)xuiPopupGetClosePolicy(pPopupNode->pWidget, &iOutsidePolicy, &iOwnerPolicy, &iEscapePolicy);
			(void)xuiPopupGetMetrics(pPopupNode->pWidget, &fPadding, &fBorderWidth, &fShadowSize);
			(void)xuiPopupGetScrollbarStyle(pPopupNode->pWidget, &iScrollbarMode, &fScrollbarSize);
			(void)xuiPopupGetColors(pPopupNode->pWidget, &iPanelColor, &iBorderColor, &iShadowColor, &iBackdropColor);
		}
		if ( pScrollViewWidget != NULL ) {
			(void)xuiScrollViewGetColors(pScrollViewWidget, &iTrackColor, &iThumbColor, &iHoverColor, &iActiveColor, &iFocusColor, &iDisabledColor);
		}
		if ( (pPopupNode == NULL) || (pPopupNode->pWidget == NULL) ||
		     (xuiPopupIsOpen(pPopupNode->pWidget) == 0) ||
		     (xuiPopupIsModal(pPopupNode->pWidget) == 0) ||
		     (xuiPopupGetConsumeInside(pPopupNode->pWidget) == 0) ||
		     (xuiPopupGetMatchOwnerWidth(pPopupNode->pWidget) == 0) ||
		     (xuiPopupGetAnchor(pPopupNode->pWidget) != XUI_POPUP_ANCHOR_TOP_RIGHT) ||
		     (xuiPopupGetDirection(pPopupNode->pWidget) != XUI_POPUP_DIRECTION_LEFT_DOWN) ||
		     (xuiPopupGetGap(pPopupNode->pWidget) != 7.0f) ||
		     (xuiPopupGetMargin(pPopupNode->pWidget) != 9.0f) ||
		     (xuiPopupGetFocusPolicy(pPopupNode->pWidget) != XUI_POPUP_FOCUS_FIRST_CHILD) ||
		     (pContentWidget == NULL) ||
		     (pScrollViewWidget == NULL) ||
		     (pPopupChildWidget == NULL) || (sPopupChildText == NULL) ||
		     (strcmp(sPopupChildText, "Popup child") != 0) ||
		     (fContentWidth != 520.0f) || (fContentHeight != 340.0f) ||
		     (fMaxWidth != 260.0f) || (fMaxHeight != 190.0f) ||
		     (fOffsetX != 5.0f) || (fOffsetY != -4.0f) ||
		     (fScrollX != 11.0f) || (fScrollY != 13.0f) ||
		     (iOutsidePolicy != XUI_POPUP_OUTSIDE_CONSUME) || (iOwnerPolicy != XUI_POPUP_OWNER_CONSUME) ||
		     (iEscapePolicy != XUI_POPUP_ESCAPE_IGNORE) ||
		     (fPadding != 6.0f) || (fBorderWidth != 2.0f) || (fShadowSize != 10.0f) ||
		     (iScrollbarMode != XUI_SCROLLBAR_MODE_FULL) || (fScrollbarSize != 15.0f) ||
		     (xuiScrollViewGetScrollbarMode(pScrollViewWidget) != XUI_SCROLLBAR_MODE_FULL) ||
		     (iPanelColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iShadowColor != XUI_COLOR_RGBA(48, 64, 80, 96)) ||
		     (iBackdropColor != XUI_COLOR_RGBA(64, 80, 96, 112)) ||
		     (iTrackColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ) {
			printf("xui_uidesign exercise-popup-properties-failed id=%d size=%.1f/%.1f max=%.1f/%.1f scroll=%.1f/%.1f policy=%d/%d/%d scrollbar=%d/%.1f\n",
				iPopup,
				fContentWidth,
				fContentHeight,
				fMaxWidth,
				fMaxHeight,
				fScrollX,
				fScrollY,
				iOutsidePolicy,
				iOwnerPolicy,
				iEscapePolicy,
				iScrollbarMode,
				fScrollbarSize);
			return XUI_ERROR;
		}
	}
	{
		int iPopupContent;
		ui_design_node_t* pPopupContentNode;
		xui_widget pContentWidget;
		xui_widget pFirstPopupLabel;
		xui_widget pSecondPopupLabel;
		xui_rect_t tFirstPopupRect;
		xui_rect_t tSecondPopupRect;
		iPopupContent = 0;
		pPopupContentNode = NULL;
		pContentWidget = NULL;
		pFirstPopupLabel = pSecondPopupLabel = NULL;
		memset(&tFirstPopupRect, 0, sizeof(tFirstPopupRect));
		memset(&tSecondPopupRect, 0, sizeof(tSecondPopupRect));
		iCreateRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_POPUP, 1800.0f, 760.0f, &iPopupContent);
		if ( iCreateRet != XUI_OK ) return iCreateRet;
		(void)uiDesignAppSetNodeProperty(pApp, iPopupContent, "data.content", "Alpha row|14|16|120|22|#A1B2C3|left\nBeta row|18|44|132|24|#B2C3D4|right");
		(void)uiDesignAppSetNodeProperty(pApp, iPopupContent, "behavior.open", "true");
		pPopupContentNode = uiDesignModelGetNode(&pApp->tModel, iPopupContent);
		pContentWidget = (pPopupContentNode != NULL && pPopupContentNode->pWidget != NULL) ? xuiPopupGetContentWidget(pPopupContentNode->pWidget) : NULL;
		if ( pContentWidget != NULL ) {
			pFirstPopupLabel = xuiWidgetGetFirstChild(pContentWidget);
			pSecondPopupLabel = (pFirstPopupLabel != NULL) ? xuiWidgetGetNextSibling(pFirstPopupLabel) : NULL;
			if ( pFirstPopupLabel != NULL ) tFirstPopupRect = xuiWidgetGetRect(pFirstPopupLabel);
			if ( pSecondPopupLabel != NULL ) tSecondPopupRect = xuiWidgetGetRect(pSecondPopupLabel);
		}
		if ( (pPopupContentNode == NULL) || (pPopupContentNode->pWidget == NULL) ||
		     (pContentWidget == NULL) ||
		     (pFirstPopupLabel == NULL) || (pSecondPopupLabel == NULL) ||
		     (xuiLabelGetText(pFirstPopupLabel) == NULL) ||
		     (strcmp(xuiLabelGetText(pFirstPopupLabel), "Alpha row") != 0) ||
		     (xuiLabelGetText(pSecondPopupLabel) == NULL) ||
		     (strcmp(xuiLabelGetText(pSecondPopupLabel), "Beta row") != 0) ||
		     (xuiLabelGetTextColor(pFirstPopupLabel) != XUI_COLOR_RGBA(161, 178, 195, 255)) ||
		     (xuiLabelGetTextColor(pSecondPopupLabel) != XUI_COLOR_RGBA(178, 195, 212, 255)) ||
		     (tFirstPopupRect.fX != 14.0f) || (tFirstPopupRect.fY != 16.0f) ||
		     (tFirstPopupRect.fW != 120.0f) || (tFirstPopupRect.fH != 22.0f) ||
		     (tSecondPopupRect.fX != 18.0f) || (tSecondPopupRect.fY != 44.0f) ||
		     (tSecondPopupRect.fW != 132.0f) || (tSecondPopupRect.fH != 24.0f) ) {
			printf("xui_uidesign exercise-popup-content-failed id=%d\n", iPopupContent);
			return XUI_ERROR;
		}
	}
	if ( iRadioGroup != 0 ) {
		ui_design_node_t* pGroupNode = uiDesignModelGetNode(&pApp->tModel, iRadioGroup);
		ui_design_node_t* pGroupCardNode = uiDesignModelGetNode(&pApp->tModel, iRadioGroupCardChild);
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "behavior.orientation", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "data.indicatorSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "metrics.gap", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "metrics.indicatorSize", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "metrics.indicatorTextGap", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "data.selected", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "data.options", "One|true|false\nTwo|true|true");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.textColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.disabledTextColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.accentColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.borderColor", "#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.hoverBorderColor", "#506070");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "appearance.focusColor", "#607080");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "unchecked.x", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "unchecked.y", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "unchecked.w", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "unchecked.h", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "checked.x", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "checked.y", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "checked.w", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroup, "checked.h", "32");
		if ( (pGroupNode == NULL) || (pGroupNode->pWidget == NULL) ||
		     (pGroupCardNode == NULL) || (pGroupCardNode->pWidget == NULL) ||
		     (xuiRadioGroupGetOrientation(pGroupNode->pWidget) != XUI_RADIO_GROUP_HORIZONTAL) ||
		     (xuiRadioGroupGetGap(pGroupNode->pWidget) != 13.0f) ||
		     (xuiRadioGroupGetSelectedIndex(pGroupNode->pWidget) != 1) ||
		     (xuiRadioGroupGetSelectedWidget(pGroupNode->pWidget) != pGroupCardNode->pWidget) ) {
			printf("xui_uidesign exercise-radio-group-properties-failed id=%d orientation=%d gap=%.1f selected=%d selectedWidget=%p expected=%p\n",
				iRadioGroup,
				(pGroupNode != NULL && pGroupNode->pWidget != NULL) ? xuiRadioGroupGetOrientation(pGroupNode->pWidget) : -1,
				(pGroupNode != NULL && pGroupNode->pWidget != NULL) ? xuiRadioGroupGetGap(pGroupNode->pWidget) : -1.0f,
				(pGroupNode != NULL && pGroupNode->pWidget != NULL) ? xuiRadioGroupGetSelectedIndex(pGroupNode->pWidget) : -1,
				(void*)((pGroupNode != NULL && pGroupNode->pWidget != NULL) ? xuiRadioGroupGetSelectedWidget(pGroupNode->pWidget) : NULL),
				(void*)((pGroupCardNode != NULL) ? pGroupCardNode->pWidget : NULL));
			return XUI_ERROR;
		}
	}
	if ( iRadioGroupGenerated != 0 ) {
		ui_design_node_t* pGeneratedNode = uiDesignModelGetNode(&pApp->tModel, iRadioGroupGenerated);
		xui_widget pFirstRadio = NULL;
		xui_widget pSecondRadio = NULL;
		xui_widget pThirdRadio = NULL;
		xui_widget pAfterThird = NULL;
		uint32_t iAccentColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusColor = 0u;
		xui_surface pUncheckedSurface = NULL;
		xui_surface pCheckedSurface = NULL;
		xui_rect_t tUncheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tCheckedSrc = {0.0f, 0.0f, 0.0f, 0.0f};
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "behavior.orientation", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "behavior.useBuiltinAtlas", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "data.indicatorSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "data.options", "Alpha|true|false\nBeta|true|true\nGamma|false|false");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "data.selected", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "text.flags", "274");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "metrics.gap", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "metrics.indicatorSize", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "metrics.indicatorTextGap", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.textColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.disabledTextColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.accentColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.borderColor", "#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.hoverBorderColor", "#506070");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "appearance.focusColor", "#607080");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "unchecked.x", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "unchecked.y", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "unchecked.w", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "unchecked.h", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "checked.x", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "checked.y", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "checked.w", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iRadioGroupGenerated, "checked.h", "32");
		if ( pGeneratedNode != NULL && pGeneratedNode->pWidget != NULL ) {
			pFirstRadio = xuiWidgetGetFirstChild(pGeneratedNode->pWidget);
			pSecondRadio = (pFirstRadio != NULL) ? xuiWidgetGetNextSibling(pFirstRadio) : NULL;
			pThirdRadio = (pSecondRadio != NULL) ? xuiWidgetGetNextSibling(pSecondRadio) : NULL;
			pAfterThird = (pThirdRadio != NULL) ? xuiWidgetGetNextSibling(pThirdRadio) : NULL;
			if ( pSecondRadio != NULL ) {
				(void)xuiRadioGetColors(pSecondRadio, &iAccentColor, &iBorderColor, &iHoverBorderColor, &iFocusColor);
				(void)xuiRadioGetIndicatorSurface(pSecondRadio, &pUncheckedSurface, &tUncheckedSrc, &pCheckedSurface, &tCheckedSrc);
			}
		}
		if ( (pGeneratedNode == NULL) || (pGeneratedNode->pWidget == NULL) ||
		     (xuiRadioGroupGetOrientation(pGeneratedNode->pWidget) != XUI_RADIO_GROUP_HORIZONTAL) ||
		     (xuiRadioGroupGetGap(pGeneratedNode->pWidget) != 14.0f) ||
		     (xuiWidgetGetChildCount(pGeneratedNode->pWidget) != 3) ||
		     (pFirstRadio == NULL) || (pSecondRadio == NULL) || (pThirdRadio == NULL) || (pAfterThird != NULL) ||
		     (strcmp(xuiRadioGetText(pFirstRadio), "Alpha") != 0) ||
		     (strcmp(xuiRadioGetText(pSecondRadio), "Beta") != 0) ||
		     (strcmp(xuiRadioGetText(pThirdRadio), "Gamma") != 0) ||
		     (xuiWidgetGetEnabled(pThirdRadio) != 0) ||
		     (xuiRadioGroupGetSelectedIndex(pGeneratedNode->pWidget) != 1) ||
		     (xuiRadioGroupGetSelectedWidget(pGeneratedNode->pWidget) != pSecondRadio) ||
		     (xuiRadioGetChecked(pSecondRadio) == 0) ||
		     (xuiRadioGetUseBuiltinAtlas(pSecondRadio) != 0) ||
		     (xuiRadioGetTextColor(pSecondRadio) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiRadioGetDisabledTextColor(pSecondRadio) != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (xuiRadioGetTextFlags(pSecondRadio) != (XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ||
		     (xuiRadioGetIndicatorSize(pSecondRadio) != 20.0f) ||
		     (xuiRadioGetGap(pSecondRadio) != 11.0f) ||
		     (iAccentColor != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(64, 80, 96, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(80, 96, 112, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(96, 112, 128, 255)) ||
		     (pUncheckedSurface == NULL) || (pCheckedSurface == NULL) || (pUncheckedSurface != pCheckedSurface) ||
		     (tUncheckedSrc.fX != 9.0f) || (tUncheckedSrc.fY != 10.0f) || (tUncheckedSrc.fW != 29.0f) || (tUncheckedSrc.fH != 30.0f) ||
		     (tCheckedSrc.fX != 11.0f) || (tCheckedSrc.fY != 12.0f) || (tCheckedSrc.fW != 31.0f) || (tCheckedSrc.fH != 32.0f) ) {
			printf("xui_uidesign exercise-radio-group-generated-failed id=%d children=%d selected=%d\n",
				iRadioGroupGenerated,
				(pGeneratedNode != NULL && pGeneratedNode->pWidget != NULL) ? xuiWidgetGetChildCount(pGeneratedNode->pWidget) : -1,
				(pGeneratedNode != NULL && pGeneratedNode->pWidget != NULL) ? xuiRadioGroupGetSelectedIndex(pGeneratedNode->pWidget) : -1);
			return XUI_ERROR;
		}
	}
	if ( iDockPanel != 0 ) {
		ui_design_node_t* pDockNode = uiDesignModelGetNode(&pApp->tModel, iDockPanel);
		ui_design_node_t* pDockChildNode = uiDesignModelGetNode(&pApp->tModel, iDockPanelChild);
		xui_dock_panel_metrics_t tDockMetrics;
		xui_dock_panel_colors_t tDockColors;
		xui_dock_window_info_t tDockWindow;
		xui_widget pWindowClient;
		int iRegionMode;
		float fRegionValue;
		memset(&tDockMetrics, 0, sizeof(tDockMetrics));
		memset(&tDockColors, 0, sizeof(tDockColors));
		memset(&tDockWindow, 0, sizeof(tDockWindow));
		iRegionMode = -1;
		fRegionValue = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "data.windows",
			"Main|fill|0|false|true|Main|doc|true|false|document|false|false|80|70|260|180\nTools|left|0.30|true|true|Tools|left|true|false|document|false|false|80|70|260|180");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "data.regions", "left|pixel|180");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "data.menuTitles", "float|Float Window\nclose|Close Window");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "data.tooltips", "close|Close active\noptions|Window menu");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.captionHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.tabStripHeight", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.buttonSize", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.buttonGap", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.splitterSize", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.splitterHitSize", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.minPaneWidth", "111");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.minPaneHeight", "77");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.tabMinWidth", "69");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.tabMaxWidth", "151");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.tabPaddingX", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.floatTitleHeight", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.floatBorderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.autoHideStripSize", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.leftRatio", "0.31");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.rightRatio", "0.27");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "metrics.bottomRatio", "0.23");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.backgroundColor", "#0A0B0C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.paneColor", "#1A1B1C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.clientColor", "#2A2B2C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.captionColor", "#3A3B3C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.activeCaptionColor", "#4A4B4C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.captionTextColor", "#5A5B5C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.activeCaptionTextColor", "#6A6B6C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.tabColor", "#7A7B7C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.tabHoverColor", "#8A8B8C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.activeTabColor", "#9A9B9C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.tabTextColor", "#AAABAC");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.activeTabTextColor", "#BABBBC");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.borderColor", "#5A5B5C");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.focusColor", "#CACBCC");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.splitterColor", "#0B0C0D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.splitterHoverColor", "#1B1C1D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.splitterActiveColor", "#2B2C2D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.buttonColor", "#3B3C3D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.buttonHoverColor", "#4B4C4D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.buttonActiveColor", "#5B5C5D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.autoHideColor", "#6B6C6D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.autoHideHoverColor", "#7B7C7D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.floatTitleColor", "#8B8C8D");
		(void)uiDesignAppSetNodeProperty(pApp, iDockPanel, "appearance.floatBorderColor", "#9B9C9D");
		pWindowClient = (pDockNode != NULL && pDockNode->pWidget != NULL) ? xuiDockPanelGetWindowClient(pDockNode->pWidget, 0) : NULL;
		if ( pDockNode != NULL && pDockNode->pWidget != NULL ) {
			(void)xuiDockPanelGetMetrics(pDockNode->pWidget, &tDockMetrics);
			(void)xuiDockPanelGetColors(pDockNode->pWidget, &tDockColors);
			(void)xuiDockPanelGetWindowInfo(pDockNode->pWidget, 0, &tDockWindow);
			(void)xuiDockPanelGetRegionSize(pDockNode->pWidget, XUI_DOCK_PANEL_REGION_LEFT, &iRegionMode, &fRegionValue);
		}
		if ( (pDockNode == NULL) || (pDockNode->pWidget == NULL) ||
		     (xuiDockPanelGetWindowCount(pDockNode->pWidget) != 2) ||
		     (xuiDockPanelGetWindowTitle(pDockNode->pWidget, 0) == NULL) ||
		     (strcmp(xuiDockPanelGetWindowTitle(pDockNode->pWidget, 0), "Main") != 0) ||
		     (tDockWindow.bClosable != 0) ||
		     (tDockWindow.bDockable == 0) ||
		     (tDockMetrics.fCaptionHeight != 29.0f) ||
		     (tDockMetrics.fTabStripHeight != 31.0f) ||
		     (tDockMetrics.fButtonSize != 17.0f) ||
		     (tDockMetrics.fButtonGap != 3.0f) ||
		     (tDockMetrics.fBorderWidth != 2.0f) ||
		     (tDockMetrics.fSplitterSize != 6.0f) ||
		     (tDockMetrics.fSplitterHitSize != 8.0f) ||
		     (tDockMetrics.fMinPaneWidth != 111.0f) ||
		     (tDockMetrics.fMinPaneHeight != 77.0f) ||
		     (tDockMetrics.fTabMinWidth != 69.0f) ||
		     (tDockMetrics.fTabMaxWidth != 151.0f) ||
		     (tDockMetrics.fTabPaddingX != 13.0f) ||
		     (tDockMetrics.fFloatTitleHeight != 28.0f) ||
		     (tDockMetrics.fFloatBorderWidth != 3.0f) ||
		     (tDockMetrics.fAutoHideStripSize != 25.0f) ||
		     (tDockColors.iBackgroundColor != XUI_COLOR_RGBA(10, 11, 12, 255)) ||
		     (tDockColors.iPaneColor != XUI_COLOR_RGBA(26, 27, 28, 255)) ||
		     (tDockColors.iClientColor != XUI_COLOR_RGBA(42, 43, 44, 255)) ||
		     (tDockColors.iCaptionColor != XUI_COLOR_RGBA(58, 59, 60, 255)) ||
		     (tDockColors.iActiveCaptionColor != XUI_COLOR_RGBA(74, 75, 76, 255)) ||
		     (tDockColors.iCaptionTextColor != XUI_COLOR_RGBA(90, 91, 92, 255)) ||
		     (tDockColors.iActiveCaptionTextColor != XUI_COLOR_RGBA(106, 107, 108, 255)) ||
		     (tDockColors.iTabColor != XUI_COLOR_RGBA(122, 123, 124, 255)) ||
		     (tDockColors.iTabHoverColor != XUI_COLOR_RGBA(138, 139, 140, 255)) ||
		     (tDockColors.iActiveTabColor != XUI_COLOR_RGBA(154, 155, 156, 255)) ||
		     (tDockColors.iTabTextColor != XUI_COLOR_RGBA(170, 171, 172, 255)) ||
		     (tDockColors.iActiveTabTextColor != XUI_COLOR_RGBA(186, 187, 188, 255)) ||
		     (tDockColors.iBorderColor != XUI_COLOR_RGBA(90, 91, 92, 255)) ||
		     (tDockColors.iFocusColor != XUI_COLOR_RGBA(202, 203, 204, 255)) ||
		     (tDockColors.iSplitterColor != XUI_COLOR_RGBA(11, 12, 13, 255)) ||
		     (tDockColors.iSplitterHoverColor != XUI_COLOR_RGBA(27, 28, 29, 255)) ||
		     (tDockColors.iSplitterActiveColor != XUI_COLOR_RGBA(43, 44, 45, 255)) ||
		     (tDockColors.iButtonColor != XUI_COLOR_RGBA(59, 60, 61, 255)) ||
		     (tDockColors.iButtonHoverColor != XUI_COLOR_RGBA(75, 76, 77, 255)) ||
		     (tDockColors.iButtonActiveColor != XUI_COLOR_RGBA(91, 92, 93, 255)) ||
		     (tDockColors.iAutoHideColor != XUI_COLOR_RGBA(107, 108, 109, 255)) ||
		     (tDockColors.iAutoHideHoverColor != XUI_COLOR_RGBA(123, 124, 125, 255)) ||
		     (tDockColors.iFloatTitleColor != XUI_COLOR_RGBA(139, 140, 141, 255)) ||
		     (tDockColors.iFloatBorderColor != XUI_COLOR_RGBA(155, 156, 157, 255)) ||
		     (iRegionMode != XUI_DOCK_PANEL_SIZE_PIXEL) ||
		     (fRegionValue != 180.0f) ||
		     (xuiDockPanelGetMenuTitle(pDockNode->pWidget, XUI_DOCK_PANEL_MENU_FLOAT) == NULL) ||
		     (strcmp(xuiDockPanelGetMenuTitle(pDockNode->pWidget, XUI_DOCK_PANEL_MENU_FLOAT), "Float Window") != 0) ||
		     (xuiDockPanelGetTooltipText(pDockNode->pWidget, XUI_DOCK_PANEL_TOOLTIP_CLOSE) == NULL) ||
		     (strcmp(xuiDockPanelGetTooltipText(pDockNode->pWidget, XUI_DOCK_PANEL_TOOLTIP_CLOSE), "Close active") != 0) ||
		     (pDockChildNode == NULL) || (pDockChildNode->pWidget == NULL) ||
		     (pWindowClient == NULL) ||
		     (xuiWidgetGetParent(pDockChildNode->pWidget) != pWindowClient) ) {
			printf("xui_uidesign exercise-dock-panel-properties-failed id=%d windows=%d region=%d/%.1f\n",
				iDockPanel,
				(pDockNode != NULL && pDockNode->pWidget != NULL) ? xuiDockPanelGetWindowCount(pDockNode->pWidget) : -1,
				iRegionMode,
				fRegionValue);
			return XUI_ERROR;
		}
	}
	if ( iMenu != 0 ) {
		ui_design_node_t* pMenuNode = uiDesignModelGetNode(&pApp->tModel, iMenu);
		xui_widget pMenuContent;
		xui_widget pMenuWidget;
		const xui_menu_item_t* pRunItem;
		const xui_menu_item_t* pItem;
		const xui_menu_item_t* pMoreItem;
		const xui_menu_item_t* pSubItem;
		xui_menu_metrics_t tMenuMetrics;
		xui_menu_colors_t tMenuColors;
		memset(&tMenuMetrics, 0, sizeof(tMenuMetrics));
		memset(&tMenuColors, 0, sizeof(tMenuColors));
		tMenuMetrics.iSize = sizeof(tMenuMetrics);
		tMenuColors.iSize = sizeof(tMenuColors);
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "data.items", "Run|normal|true|false|true|false|F5|10|4\nStop|check|false|true|false|true||20|5\nMore|submenu|true|false|false|false||30|6");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "data.submenus", "More|Child|normal|true|false|false|false|Ctrl+M|31|7");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "data.hoverIndex", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "behavior.previewOpen", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.itemHeight", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.separatorHeight", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.paddingX", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.paddingY", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.markWidth", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.iconWidth", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.shortcutGap", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.arrowWidth", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.minWidth", "180");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "metrics.maxHeight", "240");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.panelColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.borderColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.shadowColor", "#30313240");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.hoverColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.textColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.hoverTextColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.disabledTextColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.shortcutColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.dangerTextColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.markColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.separatorColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iMenu, "appearance.focusColor", "#C0C1C2");
		pMenuContent = (pMenuNode != NULL && pMenuNode->pWidget != NULL) ? xuiPopupGetContentWidget(pMenuNode->pWidget) : NULL;
		pMenuWidget = (pMenuContent != NULL) ? xuiWidgetGetFirstChild(pMenuContent) : NULL;
		pRunItem = (pMenuWidget != NULL) ? xuiMenuGetItem(pMenuWidget, 0) : NULL;
		pItem = (pMenuWidget != NULL) ? xuiMenuGetItem(pMenuWidget, 1) : NULL;
		pMoreItem = (pMenuWidget != NULL) ? xuiMenuGetItem(pMenuWidget, 2) : NULL;
		pSubItem = (pMoreItem != NULL && pMoreItem->pSubmenu != NULL) ? xuiMenuGetItem(pMoreItem->pSubmenu, 0) : NULL;
		if ( pMenuWidget != NULL ) {
			(void)xuiMenuGetMetrics(pMenuWidget, &tMenuMetrics);
			(void)xuiMenuGetColors(pMenuWidget, &tMenuColors);
		}
		if ( (pMenuNode == NULL) || (pMenuNode->pWidget == NULL) ||
		     (xuiPopupIsOpen(pMenuNode->pWidget) == 0) ||
		     (pMenuWidget == NULL) ||
		     (xuiMenuGetItemCount(pMenuWidget) != 3) ||
		     (xuiMenuGetHoverIndex(pMenuWidget) != 0) ||
		     (pRunItem == NULL) ||
		     ((pRunItem->iState & XUI_MENU_ITEM_DEFAULT) == 0u) ||
		     (pRunItem->sShortcut == NULL) || (strcmp(pRunItem->sShortcut, "F5") != 0) ||
		     (pRunItem->iIcon != 4) ||
		     (pItem == NULL) ||
		     (pItem->sText == NULL) || (strcmp(pItem->sText, "Stop") != 0) ||
		     (pItem->iType != XUI_MENU_ITEM_CHECK) ||
		     ((pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u) ||
		     ((pItem->iState & XUI_MENU_ITEM_CHECKED) == 0u) ||
		     ((pItem->iState & XUI_MENU_ITEM_DANGER) == 0u) ||
		     (pItem->iValue != 20) ||
		     (pItem->iIcon != 5) ||
		     (pMoreItem == NULL) ||
		     (pMoreItem->iType != XUI_MENU_ITEM_SUBMENU) ||
		     (pMoreItem->pSubmenu == NULL) ||
		     (xuiMenuGetItemCount(pMoreItem->pSubmenu) != 1) ||
		     (pSubItem == NULL) ||
		     (pSubItem->sText == NULL) || (strcmp(pSubItem->sText, "Child") != 0) ||
		     (pSubItem->sShortcut == NULL) || (strcmp(pSubItem->sShortcut, "Ctrl+M") != 0) ||
		     (pSubItem->iValue != 31) ||
		     (pSubItem->iIcon != 7) ||
		     (tMenuMetrics.fItemHeight != 27.0f) ||
		     (tMenuMetrics.fSeparatorHeight != 9.0f) ||
		     (tMenuMetrics.fPaddingX != 6.0f) ||
		     (tMenuMetrics.fPaddingY != 7.0f) ||
		     (tMenuMetrics.fMarkWidth != 23.0f) ||
		     (tMenuMetrics.fIconWidth != 15.0f) ||
		     (tMenuMetrics.fShortcutGap != 25.0f) ||
		     (tMenuMetrics.fArrowWidth != 19.0f) ||
		     (tMenuMetrics.fMinWidth != 180.0f) ||
		     (tMenuMetrics.fMaxHeight != 240.0f) ||
		     (tMenuColors.iPanelColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tMenuColors.iBorderColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tMenuColors.iShadowColor != XUI_COLOR_RGBA(48, 49, 50, 64)) ||
		     (tMenuColors.iHoverColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (tMenuColors.iTextColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (tMenuColors.iHoverTextColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (tMenuColors.iDisabledTextColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (tMenuColors.iShortcutColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (tMenuColors.iDangerTextColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (tMenuColors.iMarkColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (tMenuColors.iSeparatorColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (tMenuColors.iFocusColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ) {
			printf("xui_uidesign exercise-menu-properties-failed id=%d items=%d submenu=%d width=%.1f panel=%08X\n",
				iMenu,
				(pMenuWidget != NULL) ? xuiMenuGetItemCount(pMenuWidget) : -1,
				(pMoreItem != NULL && pMoreItem->pSubmenu != NULL) ? xuiMenuGetItemCount(pMoreItem->pSubmenu) : -1,
				tMenuMetrics.fMinWidth,
				tMenuColors.iPanelColor);
			return XUI_ERROR;
		}
	}
	if ( iMsgBox != 0 ) {
		ui_design_node_t* pMsgNode = uiDesignModelGetNode(&pApp->tModel, iMsgBox);
		xui_widget pClient;
		xui_widget pIconChild;
		xui_widget pMessageChild;
		xui_widget pModeChild;
		xui_widget pProceedButton;
		xui_widget pDeclineButton;
		xui_widget pKeepButton;
		xui_widget pAbortButton;
		xui_widget_type pLabelType;
		xui_widget_type pButtonType;
		xui_rect_t tKeepRect;
		float fBorderWidth;
		uint32_t iBorderColor;
		fBorderWidth = 0.0f;
		iBorderColor = 0u;
		tKeepRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		pLabelType = (pApp->pContext != NULL) ? xuiLabelGetType(pApp->pContext) : NULL;
		pButtonType = (pApp->pContext != NULL) ? xuiButtonGetType(pApp->pContext) : NULL;
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "text.title", "Confirm");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "text.message", "Delete selected rows?");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.open", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.type", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.buttons", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "data.buttonTitles", "yes|Proceed\nno|Decline\nok|Apply\ncancel|Cancel");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "data.iconSource", "");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "icon.x", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "icon.y", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "icon.w", "16");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "icon.h", "16");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.modal", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.paddingX", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.paddingY", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.iconSize", "32");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.iconGap", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.buttonWidth", "84");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.buttonHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.buttonGap", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.footerHeight", "48");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.minMessageHeight", "44");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.minWidth", "280");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.maxWidth", "360");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "metrics.maxWidthRatio", "0.7");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.backdropColor", "#01020304");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.clientColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.messageColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.mutedTextColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.iconColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.buttonColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.buttonHoverColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.buttonActiveColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.buttonFocusColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.buttonDisabledColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "appearance.borderColor", "#313233");
		pClient = (pMsgNode != NULL && pMsgNode->pWidget != NULL) ? xuiPanelGetClientWidget(pMsgNode->pWidget) : NULL;
		pProceedButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Proceed");
		pDeclineButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Decline");
		if ( (pProceedButton == NULL) || (pDeclineButton == NULL) ) {
			printf("xui_uidesign exercise-msgbox-button-title-failed id=%d\n", iMsgBox);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.buttons", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "data.customButtons", "Keep|77|primary\nAbort|78|danger");
		pClient = (pMsgNode != NULL && pMsgNode->pWidget != NULL) ? xuiPanelGetClientWidget(pMsgNode->pWidget) : NULL;
		pIconChild = __uiDesignExerciseChildAt(pClient, 0);
		pMessageChild = __uiDesignExerciseChildAt(pClient, 1);
		pModeChild = __uiDesignExerciseChildAt(pClient, 2);
		pKeepButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Keep");
		pAbortButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Abort");
		if ( pKeepButton != NULL ) tKeepRect = xuiWidgetGetRect(pKeepButton);
		if ( pMsgNode != NULL && pMsgNode->pWidget != NULL ) {
			(void)xuiPanelGetBorder(pMsgNode->pWidget, &fBorderWidth, &iBorderColor);
		}
		if ( (pMsgNode == NULL) || (pMsgNode->pWidget == NULL) ||
		     (xuiPanelGetTitle(pMsgNode->pWidget) == NULL) ||
		     (strcmp(xuiPanelGetTitle(pMsgNode->pWidget), "Confirm") != 0) ||
		     (xuiWidgetGetVisible(pMsgNode->pWidget) != 0) ||
		     (xuiPanelGetBackgroundColor(pMsgNode->pWidget) != XUI_COLOR_RGBA(1, 2, 3, 4)) ||
		     (xuiPanelGetClientColor(pMsgNode->pWidget) != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (fBorderWidth != 1.0f) ||
		     (iBorderColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (pClient == NULL) ||
		     (pIconChild == NULL) ||
		     (xuiWidgetGetType(pIconChild) != pLabelType) ||
		     (xuiLabelGetTextColor(pIconChild) != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (pMessageChild == NULL) ||
		     (xuiWidgetGetType(pMessageChild) != pLabelType) ||
		     (xuiLabelGetText(pMessageChild) == NULL) ||
		     (strcmp(xuiLabelGetText(pMessageChild), "Delete selected rows?") != 0) ||
		     (xuiLabelGetTextColor(pMessageChild) != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (pModeChild == NULL) ||
		     (xuiWidgetGetType(pModeChild) != pLabelType) ||
		     (xuiLabelGetText(pModeChild) == NULL) ||
		     (strcmp(xuiLabelGetText(pModeChild), "Modeless dialog") != 0) ||
		     (xuiLabelGetTextColor(pModeChild) != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (pKeepButton == NULL) ||
		     (pAbortButton == NULL) ||
		     (xuiButtonGetTextColor(pKeepButton) != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tKeepRect.fW != 84.0f) ||
		     (tKeepRect.fH != 29.0f) ) {
			printf("xui_uidesign exercise-msgbox-preview-failed id=%d icon=%p/%08X msg=%p/%s/%08X mode=%p/%s/%08X keep=%p/%d/%08X/%.1f/%.1f abort=%p/%d\n",
				iMsgBox,
				(void*)pIconChild,
				(pIconChild != NULL && xuiWidgetGetType(pIconChild) == pLabelType) ? xuiLabelGetTextColor(pIconChild) : 0u,
				(void*)pMessageChild,
				(pMessageChild != NULL && xuiWidgetGetType(pMessageChild) == pLabelType && xuiLabelGetText(pMessageChild) != NULL) ? xuiLabelGetText(pMessageChild) : "",
				(pMessageChild != NULL && xuiWidgetGetType(pMessageChild) == pLabelType) ? xuiLabelGetTextColor(pMessageChild) : 0u,
				(void*)pModeChild,
				(pModeChild != NULL && xuiWidgetGetType(pModeChild) == pLabelType && xuiLabelGetText(pModeChild) != NULL) ? xuiLabelGetText(pModeChild) : "",
				(pModeChild != NULL && xuiWidgetGetType(pModeChild) == pLabelType) ? xuiLabelGetTextColor(pModeChild) : 0u,
				(void*)pKeepButton,
				(pKeepButton != NULL) ? xuiButtonGetSemantic(pKeepButton) : -1,
				(pKeepButton != NULL) ? xuiButtonGetTextColor(pKeepButton) : 0u,
				tKeepRect.fW,
				tKeepRect.fH,
				(void*)pAbortButton,
				(pAbortButton != NULL) ? xuiButtonGetSemantic(pAbortButton) : -1);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iMsgBox, "behavior.open", "true");
		if ( xuiWidgetGetVisible(pMsgNode->pWidget) == 0 ) {
			printf("xui_uidesign exercise-msgbox-open-failed id=%d\n", iMsgBox);
			return XUI_ERROR;
		}
	}
	if ( iFileDialog != 0 ) {
		ui_design_node_t* pFileNode = uiDesignModelGetNode(&pApp->tModel, iFileDialog);
		xui_widget pClient;
		xui_widget pFirstChild;
		xui_widget pSelectedFill;
		xui_widget pHoverFill;
		xui_widget pProjectLabel;
		xui_widget pDisabledEntry;
		xui_widget pParentButton;
		xui_widget pReloadButton;
		xui_widget pChooseButton;
		xui_widget pDismissButton;
		xui_widget pPathLabel;
		xui_widget pFileCaption;
		xui_widget pFileNameLabel;
		xui_widget pFilterLabel;
		xui_widget pModeLabel;
		xui_widget_type pLabelType;
		xui_widget_type pButtonType;
		xui_rect_t tFirstRect;
		xui_rect_t tPanelRect;
		xui_rect_t tChooseRect;
		float fBorderWidth;
		uint32_t iBorderColor;
		fBorderWidth = 0.0f;
		iBorderColor = 0u;
		tFirstRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tPanelRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tChooseRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		pLabelType = (pApp->pContext != NULL) ? xuiLabelGetType(pApp->pContext) : NULL;
		pButtonType = (pApp->pContext != NULL) ? xuiButtonGetType(pApp->pContext) : NULL;
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "text.title", "Select Script");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "text.initialDir", "D:\\GIT\\xge");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "text.fileName", "manual.lua");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.filter", "Python (*.py)|*.py\nText (*.txt)|*.txt\nAll (*.*)|*.*");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.roots", "Project|D:\\GIT\\xge|true|true\nTemp|C:\\Temp|false|false");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.entries", "main.c|file|12 KB|today|true|true\nhelper.py|file|4 KB|yesterday|false|false\nsrc|folder|Folder|today|false|true");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.buttonTitles", "ok|Choose\ncancel|Dismiss\nup|Parent\nrefresh|Reload\npath|Location\nfile|Script\ntype|Kind");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.selectedFilter", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "data.hoverEntry", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "behavior.mode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "behavior.modal", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "behavior.showRoots", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "behavior.pathEditMode", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.dialogWidth", "420");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.dialogHeight", "300");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.padding", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.sidebarWidth", "104");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.toolbarHeight", "34");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.footerHeight", "82");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.rowHeight", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.buttonWidth", "88");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.buttonHeight", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "metrics.buttonGap", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.backgroundColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.headerColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.clientColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.sidebarColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.listColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.inputColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.buttonColor", "#C1C2C3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.buttonHoverColor", "#D1D2D3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.buttonActiveColor", "#E1E2E3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.buttonFocusColor", "#F1F2F3");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.buttonDisabledColor", "#A4A5A6");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.selectedColor", "#C4C5C6");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.hoverColor", "#D4D5D6");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.disabledTextColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.textColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.mutedTextColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iFileDialog, "appearance.borderColor", "#515253");
		pClient = (pFileNode != NULL && pFileNode->pWidget != NULL) ? xuiPanelGetClientWidget(pFileNode->pWidget) : NULL;
		pFirstChild = (pClient != NULL) ? xuiWidgetGetFirstChild(pClient) : NULL;
		if ( pFirstChild != NULL ) tFirstRect = xuiWidgetGetRect(pFirstChild);
		if ( pFileNode != NULL && pFileNode->pWidget != NULL ) {
			(void)xuiPanelGetBorder(pFileNode->pWidget, &fBorderWidth, &iBorderColor);
			tPanelRect = xuiWidgetGetRect(pFileNode->pWidget);
		}
		pSelectedFill = __uiDesignExerciseFindPanelColor(pApp, pClient, XUI_COLOR_RGBA(196, 197, 198, 255));
		pHoverFill = __uiDesignExerciseFindPanelColor(pApp, pClient, XUI_COLOR_RGBA(212, 213, 214, 255));
		pProjectLabel = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "Project");
		pDisabledEntry = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "helper.py");
		pParentButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Parent");
		pReloadButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Reload");
		pChooseButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Choose");
		pDismissButton = __uiDesignExerciseFindChildText(pApp, pClient, pButtonType, "Dismiss");
		pPathLabel = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "Location: D:\\GIT\\xge");
		pFileCaption = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "Script");
		pFileNameLabel = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "manual.lua");
		pFilterLabel = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "Text (*.txt)");
		pModeLabel = __uiDesignExerciseFindChildText(pApp, pClient, pLabelType, "Modeless, path edit");
		if ( pChooseButton != NULL ) tChooseRect = xuiWidgetGetRect(pChooseButton);
		if ( (pFileNode == NULL) || (pFileNode->pWidget == NULL) ||
		     (xuiPanelGetTitle(pFileNode->pWidget) == NULL) ||
		     (strcmp(xuiPanelGetTitle(pFileNode->pWidget), "Select Script") != 0) ||
		     (xuiWidgetGetVisible(pFileNode->pWidget) == 0) ||
		     (xuiPanelGetBackgroundColor(pFileNode->pWidget) != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (xuiPanelGetHeaderColor(pFileNode->pWidget) != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (xuiPanelGetClientColor(pFileNode->pWidget) != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (fBorderWidth != 1.0f) ||
		     (iBorderColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (pFirstChild == NULL) ||
		     (tFirstRect.fX != 15.0f) ||
		     (tFirstRect.fY != 57.0f) ||
		     (tFirstRect.fW != 104.0f) ||
		     (tFirstRect.fH != 121.0f) ||
		     (xuiPanelGetBackgroundColor(pFirstChild) != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (tPanelRect.fW != 420.0f) ||
		     (tPanelRect.fH != 300.0f) ||
		     (pSelectedFill == NULL) ||
		     (pHoverFill == NULL) ||
		     (pProjectLabel == NULL) ||
		     (pDisabledEntry == NULL) ||
		     (xuiLabelGetTextColor(pDisabledEntry) != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (pParentButton == NULL) ||
		     (pReloadButton == NULL) ||
		     (pChooseButton == NULL) ||
		     (pDismissButton == NULL) ||
		     (xuiButtonGetTextColor(pChooseButton) != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (tChooseRect.fW != 88.0f) ||
		     (tChooseRect.fH != 30.0f) ||
		     (pPathLabel == NULL) ||
		     (pFileCaption == NULL) ||
		     (pFileNameLabel == NULL) ||
		     (pFilterLabel == NULL) ||
		     (pModeLabel == NULL) ) {
			printf("xui_uidesign exercise-file-dialog-preview-failed id=%d first=%.1f/%.1f/%.1f/%.1f\n",
				iFileDialog,
				tFirstRect.fX,
				tFirstRect.fY,
				tFirstRect.fW,
				tFirstRect.fH);
			return XUI_ERROR;
		}
	}
	if ( iMsgTip != 0 ) {
		ui_design_node_t* pTipNode = uiDesignModelGetNode(&pApp->tModel, iMsgTip);
		xui_widget pClient;
		xui_widget pShadowChild;
		xui_widget pIconChild;
		xui_widget pTextChild;
		xui_widget pMetaChild;
		xui_widget_type pPanelType;
		xui_widget_type pImageType;
		xui_widget_type pLabelType;
		xui_rect_t tPanelRect;
		xui_rect_t tShadowRect;
		xui_rect_t tIconRect;
		xui_rect_t tIconSource;
		xui_rect_t tTextRect;
		xui_rect_t tMetaRect;
		float fBorderWidth;
		uint32_t iBorderColor;
		const char* sIconPath;
		fBorderWidth = 0.0f;
		iBorderColor = 0u;
		pShadowChild = NULL;
		pIconChild = NULL;
		pTextChild = NULL;
		pMetaChild = NULL;
		pPanelType = (pApp->pContext != NULL) ? xuiPanelGetType(pApp->pContext) : NULL;
		pImageType = (pApp->pContext != NULL) ? xuiImageGetType(pApp->pContext) : NULL;
		pLabelType = (pApp->pContext != NULL) ? xuiLabelGetType(pApp->pContext) : NULL;
		tPanelRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tShadowRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tIconRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tIconSource = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tTextRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tMetaRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		sIconPath = __uiDesignExerciseResourcePath("xui_virtual_joystick_base.png");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "text.text", "Saved as draft");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "behavior.type", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "behavior.duration", "3.7");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "data.iconSource", sIconPath);
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "icon.x", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "icon.y", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "icon.w", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "icon.h", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.minWidth", "180");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.maxWidth", "190");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.minHeight", "64");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.paddingX", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.paddingY", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.iconSize", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.iconGap", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "metrics.offsetY", "-33");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "appearance.backgroundColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "appearance.borderColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "appearance.textColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "appearance.iconColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iMsgTip, "appearance.shadowColor", "#62636466");
		pClient = (pTipNode != NULL && pTipNode->pWidget != NULL) ? xuiPanelGetClientWidget(pTipNode->pWidget) : NULL;
		pShadowChild = __uiDesignExerciseChildAt(pClient, 0);
		pIconChild = __uiDesignExerciseChildAt(pClient, 1);
		pTextChild = __uiDesignExerciseChildAt(pClient, 2);
		pMetaChild = __uiDesignExerciseChildAt(pClient, 3);
		if ( pTipNode != NULL && pTipNode->pWidget != NULL ) tPanelRect = xuiWidgetGetRect(pTipNode->pWidget);
		if ( pShadowChild != NULL ) tShadowRect = xuiWidgetGetRect(pShadowChild);
		if ( pIconChild != NULL ) {
			tIconRect = xuiWidgetGetRect(pIconChild);
			if ( xuiWidgetGetType(pIconChild) == pImageType ) tIconSource = xuiImageGetSource(pIconChild);
		}
		if ( pTextChild != NULL ) tTextRect = xuiWidgetGetRect(pTextChild);
		if ( pMetaChild != NULL ) tMetaRect = xuiWidgetGetRect(pMetaChild);
		if ( pTipNode != NULL && pTipNode->pWidget != NULL ) {
			(void)xuiPanelGetBorder(pTipNode->pWidget, &fBorderWidth, &iBorderColor);
		}
		if ( (pTipNode == NULL) || (pTipNode->pWidget == NULL) ||
		     (xuiWidgetGetVisible(pTipNode->pWidget) == 0) ||
		     (xuiPanelGetHeaderHeight(pTipNode->pWidget) != 0.0f) ||
		     (xuiPanelGetBackgroundColor(pTipNode->pWidget) != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (xuiPanelGetClientColor(pTipNode->pWidget) != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (fBorderWidth != 1.0f) ||
		     (iBorderColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (tPanelRect.fW != 190.0f) ||
		     (tPanelRect.fH != 64.0f) ||
		     (pShadowChild == NULL) ||
		     (xuiWidgetGetType(pShadowChild) != pPanelType) ||
		     (xuiPanelGetBackgroundColor(pShadowChild) != XUI_COLOR_RGBA(98, 99, 100, 102)) ||
		     (tShadowRect.fX != 2.0f) ||
		     (tShadowRect.fY != 2.0f) ||
		     (tShadowRect.fW != 190.0f) ||
		     (tShadowRect.fH != 64.0f) ||
		     (pIconChild == NULL) ||
		     (xuiWidgetGetType(pIconChild) != pImageType) ||
		     (xuiImageGetSurface(pIconChild) == NULL) ||
		     (xuiImageGetColor(pIconChild) != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (tIconSource.fX != 4.0f) ||
		     (tIconSource.fY != 5.0f) ||
		     (tIconSource.fW != 30.0f) ||
		     (tIconSource.fH != 31.0f) ||
		     (tIconRect.fX != 21.0f) ||
		     (tIconRect.fY != 7.0f) ||
		     (tIconRect.fW != 19.0f) ||
		     (tIconRect.fH != 19.0f) ||
		     (pTextChild == NULL) ||
		     (xuiWidgetGetType(pTextChild) != pLabelType) ||
		     (xuiLabelGetText(pTextChild) == NULL) ||
		     (strcmp(xuiLabelGetText(pTextChild), "Saved as draft") != 0) ||
		     (xuiLabelGetTextColor(pTextChild) != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (tTextRect.fX != 51.0f) ||
		     (tTextRect.fY != 7.0f) ||
		     (tTextRect.fW != 118.0f) ||
		     (tTextRect.fH != 32.0f) ||
		     (pMetaChild == NULL) ||
		     (xuiWidgetGetType(pMetaChild) != pLabelType) ||
		     (xuiLabelGetText(pMetaChild) == NULL) ||
		     (strcmp(xuiLabelGetText(pMetaChild), "3.7s, offset -33, width 180-190") != 0) ||
		     (xuiLabelGetTextColor(pMetaChild) != XUI_COLOR_RGBA(210, 220, 232, 255)) ||
		     (tMetaRect.fX != 51.0f) ||
		     (tMetaRect.fY != 41.0f) ||
		     (tMetaRect.fW != 118.0f) ||
		     (tMetaRect.fH != 16.0f) ) {
			printf("xui_uidesign exercise-msgtip-preview-failed id=%d panel=%.1f/%.1f shadow=%.1f/%.1f/%.1f/%.1f icon=%.1f/%.1f/%.1f/%.1f src=%.1f/%.1f/%.1f/%.1f text=%.1f/%.1f/%.1f/%.1f meta='%s'\n",
				iMsgTip,
				tPanelRect.fW,
				tPanelRect.fH,
				tShadowRect.fX,
				tShadowRect.fY,
				tShadowRect.fW,
				tShadowRect.fH,
				tIconRect.fX,
				tIconRect.fY,
				tIconRect.fW,
				tIconRect.fH,
				tIconSource.fX,
				tIconSource.fY,
				tIconSource.fW,
				tIconSource.fH,
				tTextRect.fX,
				tTextRect.fY,
				tTextRect.fW,
				tTextRect.fH,
				(pMetaChild != NULL && xuiWidgetGetType(pMetaChild) == pLabelType && xuiLabelGetText(pMetaChild) != NULL) ? xuiLabelGetText(pMetaChild) : "");
			return XUI_ERROR;
		}
	}
	if ( iToast != 0 ) {
		ui_design_node_t* pToastNode = uiDesignModelGetNode(&pApp->tModel, iToast);
		xui_widget pClient;
		xui_widget pShadowChild;
		xui_widget pFirstCard;
		xui_widget pIconChild;
		xui_widget pTitleChild;
		xui_widget pMessageChild;
		xui_widget pCloseChild;
		xui_widget pProgressChild;
		xui_widget pSecondShadow;
		xui_widget pSecondCard;
		xui_widget pExtraChild;
		xui_widget_type pLabelType;
		xui_rect_t tShadowRect;
		xui_rect_t tFirstRect;
		xui_rect_t tIconRect;
		xui_rect_t tCloseRect;
		xui_rect_t tProgressRect;
		xui_rect_t tSecondRect;
		float fBorderWidth;
		uint32_t iBorderColor;
		fBorderWidth = 0.0f;
		iBorderColor = 0u;
		pShadowChild = NULL;
		pFirstCard = NULL;
		pIconChild = NULL;
		pTitleChild = NULL;
		pMessageChild = NULL;
		pCloseChild = NULL;
		pProgressChild = NULL;
		pSecondShadow = NULL;
		pSecondCard = NULL;
		pExtraChild = NULL;
		pLabelType = (pApp->pContext != NULL) ? xuiLabelGetType(pApp->pContext) : NULL;
		tShadowRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tFirstRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tIconRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tCloseRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tProgressRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		tSecondRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		if ( pToastNode != NULL ) {
			(void)uiDesignAppSetNodeRect(pApp, iToast, (xui_rect_t){pToastNode->tRect.fX, pToastNode->tRect.fY, 320.0f, 220.0f});
		}
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "text.title", "Fallback Title");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "text.message", "Fallback message");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "behavior.type", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "behavior.placement", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "behavior.direction", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "behavior.duration", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.width", "210");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.minWidth", "120");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.minHeight", "60");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.margin", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.gap", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.paddingX", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.paddingY", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.iconSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.iconGap", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.closeSize", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.progressHeight", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "metrics.maxVisible", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "data.toasts",
			"warning|Ready|Queue ready.|3|hover|0.25\n"
			"success|Done|Complete.|4|true|0.50\n"
			"error|Hidden|Should not show.|5|false|0.75");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.backgroundColor", "#232425");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.borderColor", "#333435");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.textColor", "#434445");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.mutedTextColor", "#535455");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.shadowColor", "#07080940");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.infoColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.successColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.warningColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.errorColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.closeColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iToast, "appearance.closeHoverColor", "#B1B2B3");
		pClient = (pToastNode != NULL && pToastNode->pWidget != NULL) ? xuiPanelGetClientWidget(pToastNode->pWidget) : NULL;
		pShadowChild = (pClient != NULL) ? xuiWidgetGetFirstChild(pClient) : NULL;
		pFirstCard = (pShadowChild != NULL) ? xuiWidgetGetNextSibling(pShadowChild) : NULL;
		pIconChild = (pFirstCard != NULL) ? xuiWidgetGetNextSibling(pFirstCard) : NULL;
		pTitleChild = (pIconChild != NULL) ? xuiWidgetGetNextSibling(pIconChild) : NULL;
		pMessageChild = (pTitleChild != NULL) ? xuiWidgetGetNextSibling(pTitleChild) : NULL;
		pCloseChild = (pMessageChild != NULL) ? xuiWidgetGetNextSibling(pMessageChild) : NULL;
		pProgressChild = (pCloseChild != NULL) ? xuiWidgetGetNextSibling(pCloseChild) : NULL;
		pSecondShadow = (pProgressChild != NULL) ? xuiWidgetGetNextSibling(pProgressChild) : NULL;
		pSecondCard = (pSecondShadow != NULL) ? xuiWidgetGetNextSibling(pSecondShadow) : NULL;
		pExtraChild = pSecondCard;
		if ( pExtraChild != NULL ) {
			for ( i = 0; (i < 6) && (pExtraChild != NULL); ++i ) pExtraChild = xuiWidgetGetNextSibling(pExtraChild);
		}
		if ( pShadowChild != NULL ) tShadowRect = xuiWidgetGetRect(pShadowChild);
		if ( pFirstCard != NULL ) tFirstRect = xuiWidgetGetRect(pFirstCard);
		if ( pIconChild != NULL ) tIconRect = xuiWidgetGetRect(pIconChild);
		if ( pCloseChild != NULL ) tCloseRect = xuiWidgetGetRect(pCloseChild);
		if ( pProgressChild != NULL ) tProgressRect = xuiWidgetGetRect(pProgressChild);
		if ( pSecondCard != NULL ) tSecondRect = xuiWidgetGetRect(pSecondCard);
		if ( pToastNode != NULL && pToastNode->pWidget != NULL ) {
			(void)xuiPanelGetBorder(pToastNode->pWidget, &fBorderWidth, &iBorderColor);
		}
		if ( (pToastNode == NULL) || (pToastNode->pWidget == NULL) ||
		     (xuiWidgetGetVisible(pToastNode->pWidget) == 0) ||
		     (xuiPanelGetClientColor(pToastNode->pWidget) != XUI_COLOR_RGBA(35, 36, 37, 255)) ||
		     (fBorderWidth != 1.0f) ||
		     (iBorderColor != XUI_COLOR_RGBA(51, 52, 53, 255)) ||
		     (pShadowChild == NULL) ||
		     (xuiPanelGetClientColor(pShadowChild) != XUI_COLOR_RGBA(7, 8, 9, 64)) ||
		     (tShadowRect.fX != 22.0f) ||
		     (tShadowRect.fY != 22.0f) ||
		     (pFirstCard == NULL) ||
		     (xuiPanelGetClientColor(pFirstCard) != XUI_COLOR_RGBA(35, 36, 37, 255)) ||
		     (tFirstRect.fX != 20.0f) ||
		     (tFirstRect.fY != 20.0f) ||
		     (tFirstRect.fW != 210.0f) ||
		     (tFirstRect.fH != 60.0f) ||
		     (pIconChild == NULL) ||
		     (xuiWidgetGetType(pIconChild) != pLabelType) ||
		     (xuiLabelGetTextColor(pIconChild) != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (tIconRect.fX != 31.0f) ||
		     (tIconRect.fY != 29.0f) ||
		     (tIconRect.fW != 18.0f) ||
		     (tIconRect.fH != 18.0f) ||
		     (pTitleChild == NULL) ||
		     (xuiWidgetGetType(pTitleChild) != pLabelType) ||
		     (strcmp(xuiLabelGetText(pTitleChild), "Ready") != 0) ||
		     (xuiLabelGetTextColor(pTitleChild) != XUI_COLOR_RGBA(67, 68, 69, 255)) ||
		     (pMessageChild == NULL) ||
		     (xuiWidgetGetType(pMessageChild) != pLabelType) ||
		     (strcmp(xuiLabelGetText(pMessageChild), "Queue ready.") != 0) ||
		     (xuiLabelGetTextColor(pMessageChild) != XUI_COLOR_RGBA(83, 84, 85, 255)) ||
		     (pCloseChild == NULL) ||
		     (xuiWidgetGetType(pCloseChild) != pLabelType) ||
		     (xuiLabelGetTextColor(pCloseChild) != XUI_COLOR_RGBA(177, 178, 179, 255)) ||
		     (tCloseRect.fX != 206.0f) ||
		     (tCloseRect.fY != 29.0f) ||
		     (tCloseRect.fW != 13.0f) ||
		     (tCloseRect.fH != 13.0f) ||
		     (pProgressChild == NULL) ||
		     (xuiPanelGetClientColor(pProgressChild) != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (tProgressRect.fX != 20.0f) ||
		     (tProgressRect.fY != 77.0f) ||
		     (tProgressRect.fW != 53.0f) ||
		     (tProgressRect.fH != 3.0f) ||
		     (pSecondCard == NULL) ||
		     (tSecondRect.fX != 20.0f) ||
		     (tSecondRect.fY != 85.0f) ||
		     (tSecondRect.fW != 210.0f) ||
		     (tSecondRect.fH != 60.0f) ||
		     (pExtraChild != NULL) ) {
			printf("xui_uidesign exercise-toast-preview-failed id=%d first=%.1f/%.1f/%.1f/%.1f second=%.1f/%.1f/%.1f/%.1f progress=%.1f/%.1f/%.1f/%.1f\n",
				iToast,
				tFirstRect.fX,
				tFirstRect.fY,
				tFirstRect.fW,
				tFirstRect.fH,
				tSecondRect.fX,
				tSecondRect.fY,
				tSecondRect.fW,
				tSecondRect.fH,
				tProgressRect.fX,
				tProgressRect.fY,
				tProgressRect.fW,
				tProgressRect.fH);
			return XUI_ERROR;
		}
	}
	if ( iTableView != 0 ) {
		ui_design_node_t* pTableNode = uiDesignModelGetNode(&pApp->tModel, iTableView);
		const xui_table_view_column_t* pColumn;
		xui_table_view_colors_t tColors;
		xui_rect_t tMerged;
		xui_rect_t tRow2;
		uint32_t iTrackColor;
		uint32_t iThumbColor;
		uint32_t iScrollbarHoverColor;
		uint32_t iScrollbarActiveColor;
		uint32_t iScrollbarFocusColor;
		uint32_t iScrollbarDisabledColor;
		float fColumnWidth;
		float fRowHeight;
		float fHeaderHeight;
		float fOffsetX;
		float fOffsetY;
		int iSelectedRow;
		int iSelectedColumn;
		memset(&tColors, 0, sizeof(tColors));
		memset(&tMerged, 0, sizeof(tMerged));
		memset(&tRow2, 0, sizeof(tRow2));
		iTrackColor = iThumbColor = iScrollbarHoverColor = iScrollbarActiveColor = iScrollbarFocusColor = iScrollbarDisabledColor = 0u;
		fColumnWidth = fRowHeight = fHeaderHeight = 0.0f;
		fOffsetX = fOffsetY = 0.0f;
		iSelectedRow = iSelectedColumn = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.columns",
			"Name|140|text|name|true|true|left|70|220|#101112|#202122|#303132\n"
			"Qty|66|float|qty|true|false|right|50|120|#111213|#212223|#313233\n"
			"Mode|260|enum|mode|true|true|center|40|320|#121314|#222324|#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.rows",
			"Alpha,12.5,Small\nBeta,18.75,Medium\nGamma,21.25,Large\nDelta,25.5,Small\n"
			"Epsilon,28.5,Medium\nZeta,30.0,Large\nEta,31.5,Small\nTheta,33.5,Medium");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.rowStyles", "1|44|true|false|#414243|#515253|#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.cellStyles", "1|1|float|Quantity value|1|1|false|true|false|true|#717273|#818283|#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.merges", "0|1|1|2|true");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "behavior.selectionMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.selectedRow", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "data.selectedColumn", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "value.offsetX", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "value.offsetY", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "metrics.columnWidth", "123");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "metrics.rowHeight", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "metrics.headerHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.headerColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.headerTextColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.rowColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.altRowColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.hoverColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.selectedColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.disabledColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.gridColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.textColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.disabledTextColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.focusColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.trackColor", "#C1C2C3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.thumbColor", "#D1D2D3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.scrollbarHoverColor", "#E1E2E3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.scrollbarActiveColor", "#F1F2F3");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.scrollbarFocusColor", "#123456");
		(void)uiDesignAppSetNodeProperty(pApp, iTableView, "appearance.scrollbarDisabledColor", "#654321");
		pColumn = (pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableViewGetColumn(pTableNode->pWidget, 1) : NULL;
		if ( pTableNode != NULL && pTableNode->pWidget != NULL ) {
			(void)xuiTableViewGetDefaultMetrics(pTableNode->pWidget, &fColumnWidth, &fRowHeight, &fHeaderHeight);
			(void)xuiTableViewGetSelectedCell(pTableNode->pWidget, &iSelectedRow, &iSelectedColumn);
			(void)xuiTableViewGetOffset(pTableNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiTableViewGetColors(pTableNode->pWidget, &tColors);
			(void)xuiTableViewGetScrollbarColors(pTableNode->pWidget, &iTrackColor, &iThumbColor, &iScrollbarHoverColor,
				&iScrollbarActiveColor, &iScrollbarFocusColor, &iScrollbarDisabledColor);
			(void)xuiTableViewGetCellContentRect(pTableNode->pWidget, 0, 1, &tMerged);
			(void)xuiTableViewGetCellContentRect(pTableNode->pWidget, 2, 0, &tRow2);
		}
		if ( (pTableNode == NULL) || (pTableNode->pWidget == NULL) ||
		     (xuiTableViewGetColumnCount(pTableNode->pWidget) != 3) ||
		     (pColumn == NULL) ||
		     (pColumn->sTitle == NULL) || (strcmp(pColumn->sTitle, "Qty") != 0) ||
		     (pColumn->sId == NULL) || (strcmp(pColumn->sId, "qty") != 0) ||
		     (pColumn->fWidth != 66.0f) ||
		     (pColumn->bVisible == 0) ||
		     (pColumn->bResizable != 0) ||
		     (pColumn->iAlign != XUI_TEXT_ALIGN_RIGHT) ||
		     (pColumn->iType != XUI_TABLE_CELL_TYPE_FLOAT) ||
		     (pColumn->bHasStyle == 0) ||
		     (pColumn->iBackgroundColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (xuiTableViewGetRowCount(pTableNode->pWidget) != 8) ||
		     (xuiTableViewGetSelectionMode(pTableNode->pWidget) != XUI_TABLE_VIEW_SELECTION_CELL) ||
		     (iSelectedRow != 1) || (iSelectedColumn != 1) ||
		     (xuiTableViewGetScrollbarMode(pTableNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (fOffsetX != 9.0f) || (fOffsetY != 7.0f) ||
		     (fColumnWidth != 123.0f) || (fRowHeight != 31.0f) || (fHeaderHeight != 29.0f) ||
		     (pTableNode->iRuntimeTableColumnCount != 3) ||
		     (pTableNode->iRuntimeTableRowCount != 8) ||
		     (strcmp(__uiDesignExerciseRuntimeTextAt(pTableNode, pTableNode->arrRuntimeTableText[1][0]), "Beta") != 0) ||
		     (pTableNode->arrRuntimeTableCellTypeSet[1][1] == 0) ||
		     (pTableNode->arrRuntimeTableCells[1][1].iType != XUI_TABLE_CELL_TYPE_FLOAT) ||
		     (pTableNode->arrRuntimeTableCells[1][1].sTooltip == NULL) ||
		     (strcmp(pTableNode->arrRuntimeTableCells[1][1].sTooltip, "Quantity value") != 0) ||
		     (pTableNode->arrRuntimeTableCells[1][1].bEditing == 0) ||
		     (pTableNode->arrRuntimeTableCells[1][1].bDirty == 0) ||
		     (pTableNode->arrRuntimeTableCells[1][1].bHasStyle == 0) ||
		     (pTableNode->arrRuntimeTableCells[1][1].iBackgroundColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (tMerged.fW != 326.0f) ||
		     (tRow2.fY != 75.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (tColors.iHeaderColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (tColors.iHeaderTextColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tColors.iRowColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (tColors.iAltRowColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (tColors.iSelectedColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (tColors.iDisabledColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (tColors.iGridColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (tColors.iDisabledTextColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (tColors.iFocusRingColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ||
		     (tColors.iBarColor != XUI_COLOR_RGBA(193, 194, 195, 255)) ||
		     (tColors.iThumbColor != XUI_COLOR_RGBA(209, 210, 211, 255)) ||
		     (iTrackColor != XUI_COLOR_RGBA(193, 194, 195, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(209, 210, 211, 255)) ||
		     (iScrollbarHoverColor != XUI_COLOR_RGBA(225, 226, 227, 255)) ||
		     (iScrollbarActiveColor != XUI_COLOR_RGBA(241, 242, 243, 255)) ||
		     (iScrollbarFocusColor != XUI_COLOR_RGBA(18, 52, 86, 255)) ||
		     (iScrollbarDisabledColor != XUI_COLOR_RGBA(101, 67, 33, 255)) ) {
			printf("xui_uidesign exercise-tableview-properties-failed id=%d cols=%d rows=%d offset=%.1f/%.1f merged=%.1f row2y=%.1f\n",
				iTableView,
				(pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableViewGetColumnCount(pTableNode->pWidget) : -1,
				(pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableViewGetRowCount(pTableNode->pWidget) : -1,
				fOffsetX,
				fOffsetY,
				tMerged.fW,
				tRow2.fY);
			return XUI_ERROR;
		}
	}
	if ( iTableGrid != 0 ) {
		ui_design_node_t* pTableNode = uiDesignModelGetNode(&pApp->tModel, iTableGrid);
		xui_widget pTableWidget = (pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableGridGetTableView(pTableNode->pWidget) : NULL;
		const xui_table_view_column_t* pColumn;
		xui_table_view_colors_t tColors;
		xui_table_grid_editor_config_t tFloatConfig;
		xui_table_grid_editor_config_t tEnumConfig;
		uint32_t iTrackColor;
		uint32_t iThumbColor;
		uint32_t iScrollbarHoverColor;
		uint32_t iScrollbarActiveColor;
		uint32_t iScrollbarFocusColor;
		uint32_t iScrollbarDisabledColor;
		float fColumnWidth;
		float fRowHeight;
		float fHeaderHeight;
		float fOffsetX;
		float fOffsetY;
		int iSelectedRow;
		int iSelectedColumn;
		memset(&tColors, 0, sizeof(tColors));
		memset(&tFloatConfig, 0, sizeof(tFloatConfig));
		memset(&tEnumConfig, 0, sizeof(tEnumConfig));
		iTrackColor = iThumbColor = iScrollbarHoverColor = iScrollbarActiveColor = iScrollbarFocusColor = iScrollbarDisabledColor = 0u;
		fColumnWidth = fRowHeight = fHeaderHeight = 0.0f;
		fOffsetX = fOffsetY = 0.0f;
		iSelectedRow = iSelectedColumn = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.columns",
			"Name|120|text|name|true|true|left|70|220|#141516|#242526|#343536\n"
			"Value|80|float|value|true|true|right|60|140|#151617|#252627|#353637\n"
			"State|260|enum|state|true|false|center|80|320|#161718|#262728|#363738");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.rows",
			"Alpha,12.5,Small\nBeta,18.75,Medium\nGamma,33.5,Large\nDelta,25.5,Small\n"
			"Epsilon,28.5,Medium\nZeta,30.0,Large\nEta,31.5,Small\nTheta,34.5,Medium");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.rowStyles", "2|42|false|false|#444546|#545556|#646566");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.cellStyles", "2|1|float|Editable value|1|1|false|true|true|true|#747576|#848586|#949596");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.merges", "0|1|1|2|true");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.editorConfigs",
			"-1|1|float||5|50|0.5|2|false\n"
			"-1|2|enum|Small,Medium,Large|||||||||||||||||10,20,30|true,false,true|2|true|30");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "behavior.selectionMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "behavior.editMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.selectedRow", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "data.selectedColumn", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "value.offsetX", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "value.offsetY", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "metrics.columnWidth", "111");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "metrics.rowHeight", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "metrics.headerHeight", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.backgroundColor", "#020304");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.headerColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.headerTextColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.rowColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.altRowColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.hoverColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.selectedColor", "#626364");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.disabledColor", "#727374");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.gridColor", "#828384");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.textColor", "#929394");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.disabledTextColor", "#A2A3A4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.focusColor", "#B2B3B4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.trackColor", "#C2C3C4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.thumbColor", "#D2D3D4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.scrollbarHoverColor", "#E2E3E4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.scrollbarActiveColor", "#F2F3F4");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.scrollbarFocusColor", "#223344");
		(void)uiDesignAppSetNodeProperty(pApp, iTableGrid, "appearance.scrollbarDisabledColor", "#665544");
		pTableWidget = (pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableGridGetTableView(pTableNode->pWidget) : NULL;
		pColumn = (pTableWidget != NULL) ? xuiTableViewGetColumn(pTableWidget, 2) : NULL;
		if ( pTableWidget != NULL ) {
			(void)xuiTableViewGetDefaultMetrics(pTableWidget, &fColumnWidth, &fRowHeight, &fHeaderHeight);
			(void)xuiTableViewGetSelectedCell(pTableWidget, &iSelectedRow, &iSelectedColumn);
			(void)xuiTableViewGetOffset(pTableWidget, &fOffsetX, &fOffsetY);
			(void)xuiTableViewGetColors(pTableWidget, &tColors);
			(void)xuiTableViewGetScrollbarColors(pTableWidget, &iTrackColor, &iThumbColor, &iScrollbarHoverColor,
				&iScrollbarActiveColor, &iScrollbarFocusColor, &iScrollbarDisabledColor);
		}
		if ( (pTableNode == NULL) || (pTableNode->pWidget == NULL) || (pTableWidget == NULL) ||
		     (xuiTableViewGetColumnCount(pTableWidget) != 3) ||
		     (pColumn == NULL) ||
		     (pColumn->sTitle == NULL) || (strcmp(pColumn->sTitle, "State") != 0) ||
		     (pColumn->sId == NULL) || (strcmp(pColumn->sId, "state") != 0) ||
		     (pColumn->fWidth != 260.0f) ||
		     (pColumn->bResizable != 0) ||
		     (pColumn->iAlign != XUI_TEXT_ALIGN_CENTER) ||
		     (pColumn->iType != XUI_TABLE_CELL_TYPE_ENUM) ||
		     (xuiTableViewGetRowCount(pTableWidget) != 8) ||
		     (xuiTableViewGetSelectionMode(pTableWidget) != XUI_TABLE_VIEW_SELECTION_CELL) ||
		     (iSelectedRow != 2) || (iSelectedColumn != 1) ||
		     (xuiTableViewGetScrollbarMode(pTableWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (xuiTableGridGetEditMode(pTableNode->pWidget) != XUI_TABLE_GRID_EDIT_IMMEDIATE) ||
		     (fOffsetX != 11.0f) || (fOffsetY != 13.0f) ||
		     (fColumnWidth != 111.0f) || (fRowHeight != 30.0f) || (fHeaderHeight != 28.0f) ||
		     (pTableNode->iRuntimeTableColumnCount != 3) ||
		     (pTableNode->iRuntimeTableRowCount != 8) ||
		     (strcmp(__uiDesignExerciseRuntimeTextAt(pTableNode, pTableNode->arrRuntimeTableText[2][1]), "33.5") != 0) ||
		     (pTableNode->arrRuntimeTableCellTypeSet[2][1] == 0) ||
		     (pTableNode->arrRuntimeTableCells[2][1].iType != XUI_TABLE_CELL_TYPE_FLOAT) ||
		     (pTableNode->arrRuntimeTableCells[2][1].bEditing == 0) ||
		     (pTableNode->arrRuntimeTableCells[2][1].bInvalid == 0) ||
		     (pTableNode->arrRuntimeTableCells[2][1].bDirty == 0) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(2, 3, 4, 255)) ||
		     (tColors.iHeaderColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (tColors.iHeaderTextColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (tColors.iRowColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (tColors.iAltRowColor != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (tColors.iSelectedColor != XUI_COLOR_RGBA(98, 99, 100, 255)) ||
		     (tColors.iDisabledColor != XUI_COLOR_RGBA(114, 115, 116, 255)) ||
		     (tColors.iGridColor != XUI_COLOR_RGBA(130, 131, 132, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(146, 147, 148, 255)) ||
		     (tColors.iDisabledTextColor != XUI_COLOR_RGBA(162, 163, 164, 255)) ||
		     (tColors.iFocusRingColor != XUI_COLOR_RGBA(178, 179, 180, 255)) ||
		     (tColors.iBarColor != XUI_COLOR_RGBA(194, 195, 196, 255)) ||
		     (tColors.iThumbColor != XUI_COLOR_RGBA(210, 211, 212, 255)) ||
		     (iTrackColor != XUI_COLOR_RGBA(194, 195, 196, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(210, 211, 212, 255)) ||
		     (iScrollbarHoverColor != XUI_COLOR_RGBA(226, 227, 228, 255)) ||
		     (iScrollbarActiveColor != XUI_COLOR_RGBA(242, 243, 244, 255)) ||
		     (iScrollbarFocusColor != XUI_COLOR_RGBA(34, 51, 68, 255)) ||
		     (iScrollbarDisabledColor != XUI_COLOR_RGBA(102, 85, 68, 255)) ||
		     (uiDesignRegistryResolveTableGridEditorConfig(pTableNode, pTableNode->pWidget, 2, 1, XUI_TABLE_CELL_TYPE_FLOAT, &tFloatConfig) != XUI_OK) ||
		     (tFloatConfig.fMin != 5.0f) || (tFloatConfig.fMax != 50.0f) ||
		     (tFloatConfig.fStep != 0.5f) || (tFloatConfig.iPrecision != 2) ||
		     (tFloatConfig.bNullable != 0) ||
		     (uiDesignRegistryResolveTableGridEditorConfig(pTableNode, pTableNode->pWidget, 2, 2, XUI_TABLE_CELL_TYPE_ENUM, &tEnumConfig) != XUI_OK) ||
		     (tEnumConfig.iEnumItemCount != 3) ||
		     (tEnumConfig.arrEnumItemData == NULL) ||
		     (tEnumConfig.arrEnumEnabled == NULL) ||
		     (tEnumConfig.arrEnumItemData[1].iValue != 20) ||
		     (tEnumConfig.arrEnumEnabled[1] != 0) ||
		     (tEnumConfig.iEnumSelected != 2) ||
		     (tEnumConfig.bEnumUseValue == 0) ||
		     (tEnumConfig.iEnumSelectedValue != 30) ) {
			printf("xui_uidesign exercise-tablegrid-properties-failed id=%d cols=%d rows=%d offset=%.1f/%.1f edit=%d enum=%d\n",
				iTableGrid,
				(pTableWidget != NULL) ? xuiTableViewGetColumnCount(pTableWidget) : -1,
				(pTableWidget != NULL) ? xuiTableViewGetRowCount(pTableWidget) : -1,
				fOffsetX,
				fOffsetY,
				(pTableNode != NULL && pTableNode->pWidget != NULL) ? xuiTableGridGetEditMode(pTableNode->pWidget) : -1,
				tEnumConfig.iEnumItemCount);
			return XUI_ERROR;
		}
	}
	if ( iQrCode != 0 ) {
		ui_design_node_t* pQrNode = uiDesignModelGetNode(&pApp->tModel, iQrCode);
		xui_surface pIconSurface = NULL;
		xui_rect_t tIconSrc;
		memset(&tIconSrc, 0, sizeof(tIconSrc));
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "data.value", "XUI QR");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "appearance.foregroundColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "appearance.backgroundColor", "#F0E0D0");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "metrics.padding", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "data.iconSource", __uiDesignExerciseResourcePath("xui_virtual_joystick_knob.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "metrics.iconSize", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "icon.x", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "icon.y", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "icon.w", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "icon.h", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "behavior.minVersion", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iQrCode, "behavior.maxVersion", "2");
		if ( pQrNode != NULL && pQrNode->pWidget != NULL ) {
			pIconSurface = xuiQrCodeGetIcon(pQrNode->pWidget);
			tIconSrc = xuiQrCodeGetIconSource(pQrNode->pWidget);
		}
		if ( (pQrNode == NULL) || (pQrNode->pWidget == NULL) ||
		     (xuiQrCodeGetValue(pQrNode->pWidget) == NULL) ||
		     (strcmp(xuiQrCodeGetValue(pQrNode->pWidget), "XUI QR") != 0) ||
		     (xuiQrCodeGetForegroundColor(pQrNode->pWidget) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiQrCodeGetBackgroundColor(pQrNode->pWidget) != XUI_COLOR_RGBA(240, 224, 208, 255)) ||
		     (xuiQrCodeGetPadding(pQrNode->pWidget) != 11.0f) ||
		     (pIconSurface == NULL) ||
		     (xuiQrCodeGetIconSize(pQrNode->pWidget) != 23.0f) ||
		     (tIconSrc.fX != 2.0f) || (tIconSrc.fY != 3.0f) || (tIconSrc.fW != 18.0f) || (tIconSrc.fH != 19.0f) ||
		     (xuiQrCodeGetVersion(pQrNode->pWidget) != 2) ||
		     (xuiQrCodeGetModuleCount(pQrNode->pWidget) != 25) ) {
			printf("xui_uidesign exercise-qrcode-properties-failed id=%d version=%d modules=%d\n",
				iQrCode,
				(pQrNode != NULL && pQrNode->pWidget != NULL) ? xuiQrCodeGetVersion(pQrNode->pWidget) : -1,
				(pQrNode != NULL && pQrNode->pWidget != NULL) ? xuiQrCodeGetModuleCount(pQrNode->pWidget) : -1);
			return XUI_ERROR;
		}
	}
	if ( iImage != 0 ) {
		ui_design_node_t* pImageNode = uiDesignModelGetNode(&pApp->tModel, iImage);
		xui_surface pImageSurface = NULL;
		xui_rect_t tSource;
		xui_rect_t tCustom;
		int iAlignX = -1;
		int iAlignY = -1;
		memset(&tSource, 0, sizeof(tSource));
		memset(&tCustom, 0, sizeof(tCustom));
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "data.source", __uiDesignExerciseResourcePath("xui_menu_icon.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "appearance.tintColor", "#8899AA");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "behavior.mode", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "behavior.alignX", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "behavior.alignY", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "source.x", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "source.y", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "source.w", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "source.h", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "custom.x", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "custom.y", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "custom.w", "41");
		(void)uiDesignAppSetNodeProperty(pApp, iImage, "custom.h", "43");
		if ( pImageNode != NULL && pImageNode->pWidget != NULL ) {
			pImageSurface = xuiImageGetSurface(pImageNode->pWidget);
			(void)xuiImageGetAlign(pImageNode->pWidget, &iAlignX, &iAlignY);
			tSource = xuiImageGetSource(pImageNode->pWidget);
			tCustom = xuiImageGetCustomRect(pImageNode->pWidget);
		}
		if ( (pImageNode == NULL) || (pImageNode->pWidget == NULL) ||
		     (pImageSurface == NULL) ||
		     (xuiImageGetColor(pImageNode->pWidget) != XUI_COLOR_RGBA(136, 153, 170, 255)) ||
		     (xuiImageGetMode(pImageNode->pWidget) != XUI_IMAGE_CUSTOM) ||
		     (iAlignX != XUI_ALIGN_END) ||
		     (iAlignY != XUI_ALIGN_START) ||
		     (tSource.fX != 3.0f) || (tSource.fY != 5.0f) ||
		     (tSource.fW != 17.0f) || (tSource.fH != 19.0f) ||
		     (tCustom.fX != 7.0f) || (tCustom.fY != 11.0f) ||
		     (tCustom.fW != 41.0f) || (tCustom.fH != 43.0f) ) {
			printf("xui_uidesign exercise-image-properties-failed id=%d\n", iImage);
			return XUI_ERROR;
		}
	}
	if ( iMenuBar != 0 ) {
		ui_design_node_t* pMenuNode = uiDesignModelGetNode(&pApp->tModel, iMenuBar);
		const xui_menubar_item_t* pItem;
		const xui_menu_item_t* pToolsItem;
		const xui_menu_item_t* pProfilerItem;
		xui_menubar_metrics_t tMetrics;
		xui_menubar_colors_t tColors;
		xui_widget pOpenMenu = NULL;
		pToolsItem = NULL;
		pProfilerItem = NULL;
		memset(&tMetrics, 0, sizeof(tMetrics));
		memset(&tColors, 0, sizeof(tColors));
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "data.items", "File|false|F|101\nRun|true|R|202");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "data.menus", "Run|Execute|normal|true|false|true|false|Ctrl+R|301|0\nRun|Tools|submenu|true|false|false|false||302|0");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "data.submenus", "Run|Tools|Profiler|normal|true|false|false|false||401|0");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "data.hoverIndex", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "data.openIndex", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.height", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.paddingX", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.paddingY", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.itemPaddingX", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.itemGap", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.borderColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.itemColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.hoverColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.activeColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.textColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.disabledTextColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iMenuBar, "appearance.focusColor", "#717273");
		pItem = (pMenuNode != NULL && pMenuNode->pWidget != NULL) ? xuiMenuBarGetItem(pMenuNode->pWidget, 0) : NULL;
		if ( pMenuNode != NULL && pMenuNode->pWidget != NULL ) {
			(void)xuiMenuBarGetMetrics(pMenuNode->pWidget, &tMetrics);
			(void)xuiMenuBarGetColors(pMenuNode->pWidget, &tColors);
			pOpenMenu = xuiMenuBarGetOpenMenu(pMenuNode->pWidget);
			if ( pOpenMenu != NULL ) {
				pToolsItem = xuiMenuGetItem(pOpenMenu, 1);
				if ( pToolsItem != NULL && pToolsItem->pSubmenu != NULL ) {
					pProfilerItem = xuiMenuGetItem(pToolsItem->pSubmenu, 0);
				}
			}
		}
		if ( (pItem == NULL) || ((pItem->iState & XUI_MENUBAR_ITEM_ENABLED) != 0u) || (pItem->iMnemonic != 'F') || (pItem->iValue != 101) ||
		     (xuiMenuBarGetItemCount(pMenuNode->pWidget) != 2) ||
		     (xuiMenuBarGetHoverIndex(pMenuNode->pWidget) != 1) ||
		     (xuiMenuBarGetOpenIndex(pMenuNode->pWidget) != 1) ||
		     (xuiMenuBarIsOpen(pMenuNode->pWidget) == 0) ||
		     (pOpenMenu == NULL) ||
		     (xuiMenuGetItemCount(pOpenMenu) != 2) ||
		     (pToolsItem == NULL) ||
		     (pToolsItem->iType != XUI_MENU_ITEM_SUBMENU) ||
		     (pToolsItem->pSubmenu == NULL) ||
		     (xuiMenuGetItemCount(pToolsItem->pSubmenu) != 1) ||
		     (pProfilerItem == NULL) ||
		     (pProfilerItem->sText == NULL) ||
		     (strcmp(pProfilerItem->sText, "Profiler") != 0) ||
		     (pProfilerItem->iValue != 401) ||
		     (tMetrics.fHeight != 31.0f) ||
		     (tMetrics.fPaddingX != 5.0f) ||
		     (tMetrics.fPaddingY != 3.0f) ||
		     (tMetrics.fItemPaddingX != 14.0f) ||
		     (tMetrics.fItemGap != 4.0f) ||
		     (tMetrics.fBorderWidth != 2.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (tColors.iItemColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (tColors.iActiveColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (tColors.iDisabledTextColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (tColors.iFocusColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ) {
			printf("xui_uidesign exercise-menubar-properties-failed id=%d metrics=%.1f/%.1f/%.1f items=%d open=%d\n",
				iMenuBar,
				tMetrics.fHeight,
				tMetrics.fPaddingX,
				tMetrics.fPaddingY,
				(pMenuNode != NULL && pMenuNode->pWidget != NULL) ? xuiMenuBarGetItemCount(pMenuNode->pWidget) : -1,
				(pMenuNode != NULL && pMenuNode->pWidget != NULL) ? xuiMenuBarGetOpenIndex(pMenuNode->pWidget) : -1);
			return XUI_ERROR;
		}
	}
	if ( iToolbar != 0 ) {
		ui_design_node_t* pToolbarNode = uiDesignModelGetNode(&pApp->tModel, iToolbar);
		const xui_toolbar_item_t* pItem;
		xui_toolbar_metrics_t tMetrics;
		xui_toolbar_colors_t tColors;
		memset(&tMetrics, 0, sizeof(tMetrics));
		memset(&tColors, 0, sizeof(tColors));
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "data.items", "Build|toggle|true|true|Build all|7|701|||||\n-|separator|false|false||0|0|||||\nRun|button|false|false|Run|0|702|||||");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "data.hoverIndex", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "behavior.overflow", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.itemWidth", "71");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.itemHeight", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.separatorSize", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.groupGap", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.overflowButtonSize", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.paddingX", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.paddingY", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.iconSize", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "metrics.iconGap", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.borderColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.itemColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.hoverColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.activeColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.checkedColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.focusColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.disabledColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.separatorColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.textColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.disabledTextColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iToolbar, "appearance.iconColor", "#B1B2B3");
		pItem = (pToolbarNode != NULL && pToolbarNode->pWidget != NULL) ? xuiToolbarGetItem(pToolbarNode->pWidget, 0) : NULL;
		if ( pToolbarNode != NULL && pToolbarNode->pWidget != NULL ) {
			(void)xuiToolbarGetMetrics(pToolbarNode->pWidget, &tMetrics);
			(void)xuiToolbarGetColors(pToolbarNode->pWidget, &tColors);
		}
		if ( (pItem == NULL) || (pItem->iType != XUI_TOOLBAR_ITEM_TOGGLE) ||
		     ((pItem->iState & XUI_TOOLBAR_ITEM_ENABLED) == 0u) || ((pItem->iState & XUI_TOOLBAR_ITEM_CHECKED) == 0u) ||
		     (pItem->iGroup != 7) || (pItem->iValue != 701) ||
		     (pItem->sTooltip == NULL) || (strcmp(pItem->sTooltip, "Build all") != 0) ||
		     (xuiToolbarGetItemCount(pToolbarNode->pWidget) != 3) ||
		     (xuiToolbarGetOrientation(pToolbarNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (xuiToolbarIsOverflowEnabled(pToolbarNode->pWidget) != 0) ||
		     (xuiToolbarGetHoverIndex(pToolbarNode->pWidget) != 0) ||
		     (xuiToolbarGetHoverTooltip(pToolbarNode->pWidget) == NULL) ||
		     (strcmp(xuiToolbarGetHoverTooltip(pToolbarNode->pWidget), "Build all") != 0) ||
		     (tMetrics.iOrientation != XUI_ORIENTATION_VERTICAL) || (tMetrics.fItemWidth != 71.0f) ||
		     (tMetrics.fItemHeight != 33.0f) || (tMetrics.fSeparatorSize != 9.0f) ||
		     (tMetrics.fGroupGap != 12.0f) || (tMetrics.fOverflowSize != 29.0f) ||
		     (tMetrics.fPaddingX != 7.0f) || (tMetrics.fPaddingY != 5.0f) ||
		     (tMetrics.fBorderWidth != 2.0f) || (tMetrics.fIconSize != 18.0f) ||
		     (tMetrics.fIconGap != 8.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (tColors.iItemColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (tColors.iActiveColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (tColors.iCheckedColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (tColors.iFocusColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (tColors.iDisabledColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (tColors.iSeparatorColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (tColors.iDisabledTextColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (tColors.iIconColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ) {
			printf("xui_uidesign exercise-toolbar-properties-failed id=%d items=%d hover=%d width=%.1f bg=%08X\n",
				iToolbar,
				(pToolbarNode != NULL && pToolbarNode->pWidget != NULL) ? xuiToolbarGetItemCount(pToolbarNode->pWidget) : -1,
				(pToolbarNode != NULL && pToolbarNode->pWidget != NULL) ? xuiToolbarGetHoverIndex(pToolbarNode->pWidget) : -1,
				tMetrics.fItemWidth,
				tColors.iBackgroundColor);
			return XUI_ERROR;
		}
		pItem = xuiToolbarGetItem(pToolbarNode->pWidget, 2);
		if ( (pItem == NULL) || ((pItem->iState & XUI_TOOLBAR_ITEM_ENABLED) != 0u) ) {
			printf("xui_uidesign exercise-toolbar-disabled-failed id=%d\n", iToolbar);
			return XUI_ERROR;
		}
	}
	if ( iStatusBar != 0 ) {
		ui_design_node_t* pStatusNode = uiDesignModelGetNode(&pApp->tModel, iStatusBar);
		const xui_statusbar_item_t* pItem;
		xui_statusbar_metrics_t tMetrics;
		xui_statusbar_colors_t tColors;
		memset(&tMetrics, 0, sizeof(tMetrics));
		memset(&tColors, 0, sizeof(tColors));
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "data.items", "Ready|text|left|true|true|88|0|0|1|0|11\n|spacer|center|true|false|0|2|0|1|0|12\nLoad|progress|right|true|false|120|0|0|10|6|13");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "data.hoverIndex", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.height", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.paddingX", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.paddingY", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.gap", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.itemPaddingX", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.itemPaddingY", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.progressHeight", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "metrics.topBorderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.backgroundColor", "#020304");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.borderColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.highlightColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.itemColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.hoverColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.activeColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.focusColor", "#626364");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.textColor", "#727374");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.disabledTextColor", "#828384");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.progressTrackColor", "#929394");
		(void)uiDesignAppSetNodeProperty(pApp, iStatusBar, "appearance.progressFillColor", "#A2A3A4");
		pItem = (pStatusNode != NULL && pStatusNode->pWidget != NULL) ? xuiStatusBarGetItem(pStatusNode->pWidget, 2) : NULL;
		if ( pStatusNode != NULL && pStatusNode->pWidget != NULL ) {
			(void)xuiStatusBarGetMetrics(pStatusNode->pWidget, &tMetrics);
			(void)xuiStatusBarGetColors(pStatusNode->pWidget, &tColors);
		}
		if ( (pItem == NULL) || (pItem->iType != XUI_STATUSBAR_ITEM_PROGRESS) ||
		     (pItem->iSection != XUI_STATUSBAR_SECTION_RIGHT) || (pItem->fWidth != 120.0f) ||
		     (pItem->fMin != 0.0f) || (pItem->fMax != 10.0f) || (pItem->fValue != 6.0f) ||
		     (pItem->iValue != 13) || (xuiStatusBarGetItemCount(pStatusNode->pWidget) != 3) ||
		     (xuiStatusBarGetHoverIndex(pStatusNode->pWidget) != 0) ||
		     (tMetrics.fHeight != 27.0f) || (tMetrics.fPaddingX != 9.0f) ||
		     (tMetrics.fPaddingY != 4.0f) || (tMetrics.fGap != 11.0f) ||
		     (tMetrics.fItemPaddingX != 7.0f) || (tMetrics.fItemPaddingY != 3.0f) ||
		     (tMetrics.fProgressHeight != 8.0f) || (tMetrics.fBorderWidth != 2.0f) ||
		     (tMetrics.fTopBorderWidth != 3.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(2, 3, 4, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (tColors.iHighlightColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (tColors.iItemColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (tColors.iActiveColor != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (tColors.iFocusColor != XUI_COLOR_RGBA(98, 99, 100, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(114, 115, 116, 255)) ||
		     (tColors.iDisabledTextColor != XUI_COLOR_RGBA(130, 131, 132, 255)) ||
		     (tColors.iProgressTrackColor != XUI_COLOR_RGBA(146, 147, 148, 255)) ||
		     (tColors.iProgressFillColor != XUI_COLOR_RGBA(162, 163, 164, 255)) ) {
			printf("xui_uidesign exercise-statusbar-properties-failed id=%d items=%d hover=%d firstState=%08X firstClickable=%d height=%.1f bg=%08X\n",
				iStatusBar,
				(pStatusNode != NULL && pStatusNode->pWidget != NULL) ? xuiStatusBarGetItemCount(pStatusNode->pWidget) : -1,
				(pStatusNode != NULL && pStatusNode->pWidget != NULL) ? xuiStatusBarGetHoverIndex(pStatusNode->pWidget) : -1,
				(pStatusNode != NULL && pStatusNode->pWidget != NULL && xuiStatusBarGetItem(pStatusNode->pWidget, 0) != NULL) ? xuiStatusBarGetItem(pStatusNode->pWidget, 0)->iState : 0u,
				(pStatusNode != NULL && pStatusNode->pWidget != NULL) ? xuiStatusBarIsItemClickable(pStatusNode->pWidget, 0) : 0,
				tMetrics.fHeight,
				tColors.iBackgroundColor);
			return XUI_ERROR;
		}
		pItem = xuiStatusBarGetItem(pStatusNode->pWidget, 0);
		if ( (pItem == NULL) || ((pItem->iState & XUI_STATUSBAR_ITEM_CLICKABLE) == 0u) ) {
			printf("xui_uidesign exercise-statusbar-clickable-failed id=%d\n", iStatusBar);
			return XUI_ERROR;
		}
	}
	if ( iChart != 0 ) {
		ui_design_node_t* pChartNode = uiDesignModelGetNode(&pApp->tModel, iChart);
		xui_thickness_t tPadding;
		double fMinX;
		double fMaxX;
		double fMinY;
		double fMaxY;
		float arrDash[4];
		float fDuration;
		float fProgress;
		uint32_t iBackground;
		uint32_t iPlot;
		uint32_t iGrid;
		uint32_t iAxis;
		uint32_t iText;
		uint32_t iTooltip;
		uint32_t iTooltipText;
		uint32_t iColorA;
		uint32_t iColorB;
		int bAnimation;
		(void)uiDesignModelSetText(&pApp->tModel, iChart, "Default Series");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.seriesColor", "#ABCDEF");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "data.seriesType", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "data.seriesList", "base||Base|true|");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "data.series", "base|A|0|1|1|");
		if ( (pChartNode == NULL) || (pChartNode->pWidget == NULL) ||
		     (xuiChartGetSeriesCount(pChartNode->pWidget) != 1) ||
		     (xuiChartGetSeriesType(pChartNode->pWidget, 0) != XUI_CHART_SERIES_SCATTER) ||
		     (xuiChartGetSeriesColor(pChartNode->pWidget, 0) != XUI_COLOR_RGBA(171, 205, 239, 255)) ) {
			printf("xui_uidesign exercise-chart-default-series-color-failed id=%d\n", iChart);
			return XUI_ERROR;
		}
		(void)uiDesignModelSetText(&pApp->tModel, iChart, "Revenue");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "data.seriesList", "s1|line|Trend|true|#102030|true|#203040|true|4,2|diamond|7|0|0||\ns2|scatter|Points|false|#405060|false||false||rect|9|3|11|#010203|#040506");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "data.series", "s1|A|0|1|1|#102030\ns1|B|1|2|2|#102030\ns2|P|2|3|5|#405060");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.xAxis", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.yAxis", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.barMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.barDirection", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.pieMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.legendVisible", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.tooltipVisible", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.animation", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.animationDuration", "0.5");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "behavior.lodThreshold", "123");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.useViewRange", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.minX", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.maxX", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.minY", "-2");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.maxY", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.useBrushRange", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.brushMinX", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.brushMaxX", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.brushMinY", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "viewport.brushMaxY", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "metrics.paddingLeft", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "metrics.paddingTop", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "metrics.paddingRight", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "metrics.paddingBottom", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "metrics.pieInnerRadius", "0.35");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.backgroundColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.plotColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.gridColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.axisColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.textColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.tooltipColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iChart, "appearance.tooltipTextColor", "#717273");
		memset(arrDash, 0, sizeof(arrDash));
		fMinX = fMaxX = fMinY = fMaxY = 0.0;
		fDuration = fProgress = 0.0f;
		bAnimation = 0;
		iColorA = iColorB = 0u;
		tPadding = (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f};
		iBackground = iPlot = iGrid = iAxis = iText = iTooltip = iTooltipText = 0u;
		if ( pChartNode != NULL && pChartNode->pWidget != NULL ) {
			tPadding = xuiChartGetPadding(pChartNode->pWidget);
			(void)xuiChartGetColors(pChartNode->pWidget, &iBackground, &iPlot, &iGrid, &iAxis, &iText, &iTooltip, &iTooltipText);
			(void)xuiChartGetAnimation(pChartNode->pWidget, &bAnimation, &fDuration, &fProgress);
		}
		if ( (pChartNode == NULL) || (pChartNode->pWidget == NULL) ||
		     (strcmp(xuiChartGetTitle(pChartNode->pWidget), "Revenue") != 0) ||
		     (xuiChartGetSeriesCount(pChartNode->pWidget) != 2) ||
		     (xuiChartGetXAxis(pChartNode->pWidget) != XUI_CHART_AXIS_CATEGORY) ||
		     (xuiChartGetYAxis(pChartNode->pWidget) != XUI_CHART_AXIS_VALUE) ||
		     (xuiChartGetBarMode(pChartNode->pWidget) != XUI_CHART_BAR_STACKED) ||
		     (xuiChartGetBarDirection(pChartNode->pWidget) != XUI_CHART_BAR_HORIZONTAL) ||
		     (xuiChartGetPieMode(pChartNode->pWidget) != XUI_CHART_PIE_ROSE) ||
		     (xuiChartGetLegendVisible(pChartNode->pWidget) != 0) ||
		     (xuiChartGetTooltipVisible(pChartNode->pWidget) != 0) ||
		     (bAnimation == 0) || (fDuration != 0.5f) ||
		     (xuiChartGetLodThreshold(pChartNode->pWidget) != 123) ||
		     (tPadding.fLeft != 11.0f) || (tPadding.fTop != 12.0f) || (tPadding.fRight != 13.0f) || (tPadding.fBottom != 14.0f) ||
		     (xuiChartGetPieInnerRadius(pChartNode->pWidget) != 0.35f) ||
		     (iBackground != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iPlot != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iGrid != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iAxis != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iText != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iTooltip != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iTooltipText != XUI_COLOR_RGBA(113, 114, 115, 255)) ) {
			printf("xui_uidesign exercise-chart-basic-properties-failed id=%d\n", iChart);
			return XUI_ERROR;
		}
		if ( (xuiChartGetViewRange(pChartNode->pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) != 1) ||
		     (fMinX != 1.0) || (fMaxX != 9.0) || (fMinY != -2.0) || (fMaxY != 12.0) ||
		     (xuiChartGetBrushRange(pChartNode->pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) != 1) ||
		     (fMinX != 2.0) || (fMaxX != 4.0) || (fMinY != 3.0) || (fMaxY != 6.0) ) {
			printf("xui_uidesign exercise-chart-viewport-failed id=%d\n", iChart);
			return XUI_ERROR;
		}
		if ( (xuiChartGetSeriesColor(pChartNode->pWidget, 0) != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (xuiChartGetSeriesVisible(pChartNode->pWidget, 0) == 0) ||
		     (xuiChartGetSeriesAreaFill(pChartNode->pWidget, 0, &iColorA) != 1) ||
		     (iColorA != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (xuiChartGetSeriesSmooth(pChartNode->pWidget, 0) == 0) ||
		     (xuiChartGetSeriesDash(pChartNode->pWidget, 0, arrDash, 4) != 2) ||
		     (arrDash[0] != 4.0f) || (arrDash[1] != 2.0f) ||
		     (xuiChartGetSeriesSymbol(pChartNode->pWidget, 0) != XUI_CHART_SYMBOL_DIAMOND) ||
		     (xuiChartGetSeriesSymbolSize(pChartNode->pWidget, 0) != 7.0f) ) {
			printf("xui_uidesign exercise-chart-line-series-failed id=%d\n", iChart);
			return XUI_ERROR;
		}
		if ( (xuiChartGetSeriesColor(pChartNode->pWidget, 1) != XUI_COLOR_RGBA(64, 80, 96, 255)) ||
		     (xuiChartGetSeriesVisible(pChartNode->pWidget, 1) != 0) ||
		     (xuiChartGetSeriesSymbol(pChartNode->pWidget, 1) != XUI_CHART_SYMBOL_RECT) ||
		     (xuiChartGetSeriesSymbolSize(pChartNode->pWidget, 1) != 9.0f) ||
		     (xuiChartGetSeriesValueRadius(pChartNode->pWidget, 1, &fA, &fB) != 1) ||
		     (fA != 3.0f) || (fB != 11.0f) ||
		     (xuiChartGetSeriesValueColor(pChartNode->pWidget, 1, &iColorA, &iColorB) != 1) ||
		     (iColorA != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iColorB != XUI_COLOR_RGBA(4, 5, 6, 255)) ) {
			printf("xui_uidesign exercise-chart-scatter-series-failed id=%d\n", iChart);
			return XUI_ERROR;
		}
	}
	if ( iInventoryGrid != 0 ) {
		ui_design_node_t* pInventoryNode = uiDesignModelGetNode(&pApp->tModel, iInventoryGrid);
		xui_inventory_slot_t tSlot;
		xui_inventory_grid_layout_t tLayout;
		xui_inventory_grid_colors_t tColors;
		xui_inventory_gamepad_profile_t tProfile;
		uint32_t iAnimFlags;
		uint32_t iAnimTint;
		float fAnimScale;
		memset(&tSlot, 0, sizeof(tSlot));
		memset(&tLayout, 0, sizeof(tLayout));
		memset(&tColors, 0, sizeof(tColors));
		memset(&tProfile, 0, sizeof(tProfile));
		iAnimFlags = 0u;
		iAnimTint = 0u;
		fAnimScale = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "data.slots", "Gem|4|G|selected+cooldown+durability+radial|#112233|2001|10|7|0.25|0.8|2|3|#AABBCC||||||5|1.5|#445566\nKey|1|K|locked|#223344|2002|1|8|0|1|0|0|#FFFFFF||||||0|1|#FFFFFF");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "data.slotCount", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "data.current", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "data.selectedSlots", "0|true\n1|true");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.columns", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.slotSize", "50");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.slotGap", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.padding", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.iconPadding", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.wheelStep", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "metrics.dragThreshold", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.slotColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.emptyColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.hoverColor", "#131415");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.activeColor", "#141516");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.selectedColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.disabledColor", "#151617");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.lockedColor", "#161718");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.borderColor", "#171819");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.qualityColor", "#18191A");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.focusColor", "#191A1B");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.textColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.mutedTextColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.countColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.hotkeyColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.cooldownColor", "#E0E1E280");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.durabilityColor", "#E1E2E3");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.dragColor", "#E2E3E480");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "appearance.dropColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "behavior.selectionMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "behavior.tooltipVisible", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.wrapRows", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.wrapColumns", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.skipDisabled", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.selectOnMove", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.multiSelectModifiers", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.acceptButton", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.cancelButton", "16");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.contextButton", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.leftButton", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.rightButton", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.upButton", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.downButton", "23");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.pageUpButton", "24");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.pageDownButton", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.homeButton", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "gamepad.endButton", "19");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "value.previewSplitSlot", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "value.previewSplitX", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "value.previewSplitY", "44");
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "behavior.previewSplitOpen", "true");
		if ( pInventoryNode != NULL && pInventoryNode->pWidget != NULL ) {
			(void)xuiInventoryGridGetSlot(pInventoryNode->pWidget, 0, &tSlot);
			(void)xuiInventoryGridGetLayout(pInventoryNode->pWidget, &tLayout);
			(void)xuiInventoryGridGetColors(pInventoryNode->pWidget, &tColors);
			(void)xuiInventoryGridGetGamepadProfile(pInventoryNode->pWidget, &tProfile);
			(void)xuiInventoryGridGetSlotAnimation(pInventoryNode->pWidget, 0, &iAnimFlags, &fAnimScale, &iAnimTint);
		}
		if ( (pInventoryNode == NULL) || (pInventoryNode->pWidget == NULL) ||
		     (xuiInventoryGridGetSlotCount(pInventoryNode->pWidget) != 6) ||
		     (xuiInventoryGridGetCurrent(pInventoryNode->pWidget) != 0) ||
		     (xuiInventoryGridGetSelected(pInventoryNode->pWidget, 0) == 0) ||
		     (xuiInventoryGridGetSelected(pInventoryNode->pWidget, 1) == 0) ||
		     (xuiInventoryGridGetTooltipVisible(pInventoryNode->pWidget) != 0) ||
		     (strcmp(tSlot.sText, "Gem") != 0) ||
		     (strcmp(tSlot.sHotkey, "G") != 0) ||
		     (tSlot.iItemId != 2001) || (tSlot.iSlotId != 7) || (tSlot.iCount != 4) || (tSlot.iMaxCount != 10) ||
		     (tSlot.iSlotType != 2) || (tSlot.iItemType != 3) ||
		     ((tSlot.iFlags & (XUI_INVENTORY_SLOT_SELECTED | XUI_INVENTORY_SLOT_COOLDOWN | XUI_INVENTORY_SLOT_DURABILITY | XUI_INVENTORY_SLOT_COOLDOWN_RADIAL | XUI_INVENTORY_SLOT_ANIMATION)) != (XUI_INVENTORY_SLOT_SELECTED | XUI_INVENTORY_SLOT_COOLDOWN | XUI_INVENTORY_SLOT_DURABILITY | XUI_INVENTORY_SLOT_COOLDOWN_RADIAL | XUI_INVENTORY_SLOT_ANIMATION)) ||
		     (tSlot.iQualityColor != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (tSlot.iIconTint != XUI_COLOR_RGBA(170, 187, 204, 255)) ||
		     (tSlot.fCooldownRate != 0.25f) || (tSlot.fDurabilityRate != 0.8f) ||
		     (iAnimFlags != 5u) || (fAnimScale != 1.5f) || (iAnimTint != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (tLayout.iColumns != 3) || (tLayout.iSelectionMode != XUI_INVENTORY_SELECTION_MULTI) ||
		     (tLayout.fSlotSize != 50.0f) || (tLayout.fSlotGap != 4.0f) || (tLayout.fPadding != 6.0f) ||
		     (tLayout.fIconPadding != 3.0f) || (tLayout.fBorderWidth != 2.0f) ||
		     (tLayout.fWheelStep != 30.0f) || (tLayout.fDragThreshold != 7.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tColors.iSlotColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (tColors.iEmptyColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(19, 20, 21, 255)) ||
		     (tColors.iActiveColor != XUI_COLOR_RGBA(20, 21, 22, 255)) ||
		     (tColors.iSelectedColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tColors.iDisabledColor != XUI_COLOR_RGBA(21, 22, 23, 255)) ||
		     (tColors.iLockedColor != XUI_COLOR_RGBA(22, 23, 24, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(23, 24, 25, 255)) ||
		     (tColors.iQualityColor != XUI_COLOR_RGBA(24, 25, 26, 255)) ||
		     (tColors.iFocusColor != XUI_COLOR_RGBA(25, 26, 27, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (tColors.iMutedTextColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (tColors.iCountColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (tColors.iHotkeyColor != XUI_COLOR_RGBA(208, 209, 210, 255)) ||
		     (tColors.iCooldownColor != XUI_COLOR_RGBA(224, 225, 226, 128)) ||
		     (tColors.iDurabilityColor != XUI_COLOR_RGBA(225, 226, 227, 255)) ||
		     (tColors.iDragColor != XUI_COLOR_RGBA(226, 227, 228, 128)) ||
		     (tColors.iDropColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (xuiInventoryGridIsSplitPopupOpen(pInventoryNode->pWidget) == 0) ||
		     (xuiInventoryGridGetSplitSlot(pInventoryNode->pWidget) != 0) ||
		     ((tProfile.iFlags & XUI_INVENTORY_GAMEPAD_WRAP_ROWS) == 0u) ||
		     ((tProfile.iFlags & XUI_INVENTORY_GAMEPAD_WRAP_COLUMNS) == 0u) ||
		     ((tProfile.iFlags & XUI_INVENTORY_GAMEPAD_SKIP_DISABLED) != 0u) ||
		     ((tProfile.iFlags & XUI_INVENTORY_GAMEPAD_SELECT_ON_MOVE) != 0u) ||
		     ((tProfile.iFlags & XUI_INVENTORY_GAMEPAD_MULTI_SELECT_MODIFIERS) == 0u) ||
		     (tProfile.iAcceptButton != 15) ||
		     (tProfile.iCancelButton != 16) ||
		     (tProfile.iContextButton != 17) ||
		     (tProfile.iLeftButton != 20) ||
		     (tProfile.iRightButton != 21) ||
		     (tProfile.iUpButton != 22) ||
		     (tProfile.iDownButton != 23) ||
		     (tProfile.iPageUpButton != 24) ||
		     (tProfile.iPageDownButton != 25) ||
		     (tProfile.iHomeButton != 18) ||
		     (tProfile.iEndButton != 19) ) {
			printf("xui_uidesign exercise-inventory-grid-properties-failed id=%d\n", iInventoryGrid);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iInventoryGrid, "behavior.previewSplitOpen", "false");
	}
	if ( iMessageList != 0 ) {
		ui_design_node_t* pMessageNode = uiDesignModelGetNode(&pApp->tModel, iMessageList);
		const xui_message_node_t* pMessage;
		const xui_message_node_t* pImportMessage;
		const xui_message_node_t* pFileMessage;
		xui_message_list_metrics_t tMetrics;
		xui_message_list_colors_t tColors;
		char sMessagePath[260];
		FILE* pMessageFile;
		memset(&tMetrics, 0, sizeof(tMetrics));
		memset(&tColors, 0, sizeof(tColors));
		pImportMessage = NULL;
		pFileMessage = NULL;
		sMessagePath[0] = 0;
		pMessageFile = NULL;
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.sourceMode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.messages", "Dev|10:00|Hello|other|m10|3\nYou|10:01|Reply|self|m11|0\nSystem|10:02|Done|system|m12|0");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.selected", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "behavior.autoScroll", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "value.scrollY", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.paddingX", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.paddingY", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.nodeGap", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.avatarSize", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.avatarGap", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.bubbleMaxWidth", "180");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.bubblePaddingX", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.bubblePaddingY", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.systemPaddingX", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.systemPaddingY", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.metaHeight", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.minBubbleHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "metrics.wheelStep", "41");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.selfBubbleColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.otherBubbleColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.systemBubbleColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.selfTextColor", "#333435");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.otherTextColor", "#343536");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.systemTextColor", "#353637");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.metaTextColor", "#363738");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.avatarSelfColor", "#373839");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.avatarOtherColor", "#38393A");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.hoverColor", "#393A3B");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.selectedColor", "#3A3B3C");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "appearance.borderColor", "#303132");
		pMessage = (pMessageNode != NULL && pMessageNode->pWidget != NULL) ? xuiMessageListGetNode(pMessageNode->pWidget, 0) : NULL;
		if ( pMessageNode != NULL && pMessageNode->pWidget != NULL ) {
			(void)xuiMessageListGetMetrics(pMessageNode->pWidget, &tMetrics);
			(void)xuiMessageListGetColors(pMessageNode->pWidget, &tColors);
		}
		if ( (pMessageNode == NULL) || (pMessageNode->pWidget == NULL) ||
		     (xuiMessageListGetNodeCount(pMessageNode->pWidget) != 3) ||
		     (pMessage == NULL) ||
		     (strcmp(pMessage->sSender, "Dev") != 0) ||
		     (strcmp(pMessage->sTime, "10:00") != 0) ||
		     (strcmp(pMessage->sText, "Hello") != 0) ||
		     (strcmp(pMessage->sId, "m10") != 0) ||
		     (pMessage->iType != XUI_MESSAGE_NODE_OTHER) ||
		     (pMessage->iFlags != 3) ||
		     (xuiMessageListGetSelected(pMessageNode->pWidget) != 2) ||
		     (xuiMessageListGetAutoScroll(pMessageNode->pWidget) != 0) ||
		     (tMetrics.fPaddingX != 11.0f) || (tMetrics.fPaddingY != 12.0f) ||
		     (tMetrics.fNodeGap != 13.0f) || (tMetrics.fAvatarSize != 31.0f) ||
		     (tMetrics.fAvatarGap != 7.0f) || (tMetrics.fBubbleMaxWidth != 180.0f) ||
		     (tMetrics.fBubblePaddingX != 8.0f) || (tMetrics.fBubblePaddingY != 6.0f) ||
		     (tMetrics.fSystemPaddingX != 9.0f) || (tMetrics.fSystemPaddingY != 5.0f) ||
		     (tMetrics.fMetaHeight != 17.0f) || (tMetrics.fMinBubbleHeight != 29.0f) ||
		     (tMetrics.fWheelStep != 41.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tColors.iSelfBubbleColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tColors.iOtherBubbleColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tColors.iSystemBubbleColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (tColors.iSelfTextColor != XUI_COLOR_RGBA(51, 52, 53, 255)) ||
		     (tColors.iOtherTextColor != XUI_COLOR_RGBA(52, 53, 54, 255)) ||
		     (tColors.iSystemTextColor != XUI_COLOR_RGBA(53, 54, 55, 255)) ||
		     (tColors.iMetaTextColor != XUI_COLOR_RGBA(54, 55, 56, 255)) ||
		     (tColors.iAvatarSelfColor != XUI_COLOR_RGBA(55, 56, 57, 255)) ||
		     (tColors.iAvatarOtherColor != XUI_COLOR_RGBA(56, 57, 58, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(57, 58, 59, 255)) ||
		     (tColors.iSelectedColor != XUI_COLOR_RGBA(58, 59, 60, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ) {
			printf("xui_uidesign exercise-message-list-properties-failed id=%d\n", iMessageList);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.importText",
			"MESSAGELIST1\n"
			"N\t0\t4\timp-1\tImportUser\t10:10\tImported hello\n"
			"N\t2\t0\timp-2\tSystem\t10:11\tImported notice\n");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.selected", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.sourceMode", "1");
		pImportMessage = (pMessageNode != NULL && pMessageNode->pWidget != NULL) ? xuiMessageListGetNode(pMessageNode->pWidget, 0) : NULL;
		if ( (pMessageNode == NULL) || (pMessageNode->pWidget == NULL) ||
		     (xuiMessageListGetNodeCount(pMessageNode->pWidget) != 2) ||
		     (pImportMessage == NULL) ||
		     (strcmp(pImportMessage->sSender, "ImportUser") != 0) ||
		     (strcmp(pImportMessage->sTime, "10:10") != 0) ||
		     (strcmp(pImportMessage->sText, "Imported hello") != 0) ||
		     (strcmp(pImportMessage->sId, "imp-1") != 0) ||
		     (pImportMessage->iType != XUI_MESSAGE_NODE_SELF) ||
		     (pImportMessage->iFlags != 4) ) {
			printf("xui_uidesign exercise-message-list-import-failed id=%d\n", iMessageList);
			return XUI_ERROR;
		}
		snprintf(sMessagePath, sizeof(sMessagePath), "uidesign_message_list_source.txt");
		pMessageFile = fopen(sMessagePath, "wb");
		if ( pMessageFile == NULL ) {
			printf("xui_uidesign exercise-message-list-file-create-failed id=%d path=%s\n", iMessageList, sMessagePath);
			return XUI_ERROR;
		}
		fputs("MESSAGELIST1\n"
			"N\t0\t5\tfile-1\tFileUser\t11:10\tLoaded from file\n"
			"N\t2\t0\tfile-2\tSystem\t11:11\tFile notice\n", pMessageFile);
		fclose(pMessageFile);
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.sourcePath", sMessagePath);
		(void)uiDesignAppSetNodeProperty(pApp, iMessageList, "data.sourceMode", "2");
		pFileMessage = (pMessageNode != NULL && pMessageNode->pWidget != NULL) ? xuiMessageListGetNode(pMessageNode->pWidget, 0) : NULL;
		remove(sMessagePath);
		if ( (pMessageNode == NULL) || (pMessageNode->pWidget == NULL) ||
		     (xuiMessageListGetNodeCount(pMessageNode->pWidget) != 2) ||
		     (pFileMessage == NULL) ||
		     (strcmp(pFileMessage->sSender, "FileUser") != 0) ||
		     (strcmp(pFileMessage->sTime, "11:10") != 0) ||
		     (strcmp(pFileMessage->sText, "Loaded from file") != 0) ||
		     (strcmp(pFileMessage->sId, "file-1") != 0) ||
		     (pFileMessage->iType != XUI_MESSAGE_NODE_SELF) ||
		     (pFileMessage->iFlags != 5) ) {
			printf("xui_uidesign exercise-message-list-file-failed id=%d\n", iMessageList);
			return XUI_ERROR;
		}
	}
	if ( iTimelineView != 0 ) {
		ui_design_node_t* pTimelineNode = uiDesignModelGetNode(&pApp->tModel, iTimelineView);
		xui_timeline_layer_t tLayer;
		xui_timeline_frame_t tFrame;
		xui_timeline_span_t tSpan;
		xui_timeline_selection_t tSelection;
		xui_timeline_view_colors_t tColors;
		const char* sMenuTitle;
		float fLayerHeader;
		float fFrameWidth;
		float fMinFrameWidth;
		float fMaxFrameWidth;
		float fRowHeight;
		float fRulerHeight;
		float fOffsetX;
		float fOffsetY;
		int bVisibilityFeature;
		int bLockFeature;
		memset(&tLayer, 0, sizeof(tLayer));
		memset(&tFrame, 0, sizeof(tFrame));
		memset(&tSpan, 0, sizeof(tSpan));
		memset(&tSelection, 0, sizeof(tSelection));
		memset(&tColors, 0, sizeof(tColors));
		fLayerHeader = fFrameWidth = fMinFrameWidth = fMaxFrameWidth = fRowHeight = fRulerHeight = 0.0f;
		fOffsetX = fOffsetY = 0.0f;
		bVisibilityFeature = bLockFeature = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.timelineLayers",
			"Base|true|false|30|#112233\nFX|false|true|26|#445566\n"
			"Camera|true|false|24|#667788\nAudio|true|false|24|#778899\n"
			"Event|true|false|24|#8899AA\nLight|true|false|24|#99AABB\n"
			"Mask|true|false|24|#AABBCC\nGuide|true|false|24|#BBCCDD\n"
			"Notes|true|false|24|#CCDDEE\nTemp|true|false|24|#DDEEFF");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.timelineFrames", "Base|5|key\nFX|8|blank");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.timelineSpans", "Base|2|10|motion|Move|#778899|\nFX|8|16|custom|Glow|#AABBCC|glow");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.selection", "Base|5|Base|5|true\nFX|8|FX|8|true");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.menuTitles", "rename|Rename Layer\ninsertKeyframe|Insert Key");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.frameCount", "80");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "data.currentFrame", "12");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "value.frameRate", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "value.offsetX", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "value.offsetY", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.layerHeaderWidth", "120");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.frameWidth", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.rowHeight", "25");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.rulerHeight", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.minFrameWidth", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "metrics.maxFrameWidth", "72");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "behavior.visibilityFeature", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "behavior.lockFeature", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.cornerColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.rulerColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.layerColor", "#131415");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.layerAltColor", "#141516");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.gridColor", "#151617");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.gridStrongColor", "#161718");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.textColor", "#171819");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.mutedTextColor", "#18191A");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.selectedColor", "#191A1B");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.hoverColor", "#1A1B1C");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.currentColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.frameColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.keyFrameColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.blankKeyFrameColor", "#232425");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.spanColor", "#242526");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.spanTextColor", "#252627");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.iconColor", "#262728");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.hiddenIconColor", "#272829");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.lockedIconColor", "#28292A");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.borderColor", "#292A2B");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.focusColor", "#2A2B2C");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.disabledColor", "#2B2C2D");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.trackColor", "#2C2D2E");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.thumbColor", "#2D2E2F");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.scrollbarHoverColor", "#2E2F30");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.scrollbarActiveColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.scrollbarFocusColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iTimelineView, "appearance.scrollbarDisabledColor", "#323334");
		sMenuTitle = NULL;
		if ( pTimelineNode != NULL && pTimelineNode->pWidget != NULL ) {
			(void)xuiTimeLineViewGetLayer(pTimelineNode->pWidget, 1, &tLayer);
			(void)xuiTimeLineViewGetFrame(pTimelineNode->pWidget, 1, 8, &tFrame);
			(void)xuiTimeLineViewGetSpan(pTimelineNode->pWidget, 2, &tSpan);
			(void)xuiTimeLineViewGetSelection(pTimelineNode->pWidget, 1, &tSelection);
			(void)xuiTimeLineViewGetMetrics(pTimelineNode->pWidget, &fLayerHeader, &fFrameWidth, &fRowHeight, &fRulerHeight);
			(void)xuiTimeLineViewGetFrameWidthRange(pTimelineNode->pWidget, &fMinFrameWidth, &fMaxFrameWidth);
			(void)xuiTimeLineViewGetFeatureFlags(pTimelineNode->pWidget, &bVisibilityFeature, &bLockFeature);
			(void)xuiTimeLineViewGetOffset(pTimelineNode->pWidget, &fOffsetX, &fOffsetY);
			(void)xuiTimeLineViewGetColors(pTimelineNode->pWidget, &tColors);
			sMenuTitle = xuiTimeLineViewGetMenuTitle(pTimelineNode->pWidget, XUI_TIMELINE_MENU_FRAME_KEY);
		}
		if ( (pTimelineNode == NULL) || (pTimelineNode->pWidget == NULL) ||
		     (xuiTimeLineViewGetLayerCount(pTimelineNode->pWidget) != 10) ||
		     (strcmp(tLayer.sName, "FX") != 0) ||
		     (tLayer.bVisible != 0) || (tLayer.bLocked == 0) ||
		     (tLayer.fHeight != 26.0f) ||
		     (tLayer.iColor != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (tFrame.iLayer != 1) || (tFrame.iFrame != 8) ||
		     (tFrame.iType != XUI_TIMELINE_FRAME_BLANK_KEY) ||
		     (tSpan.iLayer != 1) || (tSpan.iStartFrame != 8) || (tSpan.iEndFrame != 16) ||
		     (tSpan.iType != XUI_TIMELINE_SPAN_CUSTOM) ||
		     (strcmp(tSpan.sLabel, "Glow") != 0) ||
		     (strcmp(tSpan.sCustomType, "glow") != 0) ||
		     (tSpan.iColor != XUI_COLOR_RGBA(170, 187, 204, 255)) ||
		     (xuiTimeLineViewGetSelectionCount(pTimelineNode->pWidget) != 2) ||
		     (tSelection.iLayer != 1) || (tSelection.iFrame != 8) ||
		     (xuiTimeLineViewGetFrameCount(pTimelineNode->pWidget) != 80) ||
		     (xuiTimeLineViewGetCurrentFrame(pTimelineNode->pWidget) != 12) ||
		     (xuiTimeLineViewGetFrameRate(pTimelineNode->pWidget) != 30.0f) ||
		     (xuiTimeLineViewGetScrollbarMode(pTimelineNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (fLayerHeader != 120.0f) || (fFrameWidth != 20.0f) || (fRowHeight != 25.0f) || (fRulerHeight != 27.0f) ||
		     (fMinFrameWidth != 10.0f) || (fMaxFrameWidth != 72.0f) ||
		     (bVisibilityFeature != 0) || (bLockFeature != 0) ||
		     (xuiTimeLineViewGetFrameWidth(pTimelineNode->pWidget) != 20.0f) ||
		     (fOffsetX != 14.0f) ||
		     (fOffsetY != 18.0f) ||
		     (tColors.iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tColors.iCornerColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (tColors.iRulerColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (tColors.iLayerColor != XUI_COLOR_RGBA(19, 20, 21, 255)) ||
		     (tColors.iLayerAltColor != XUI_COLOR_RGBA(20, 21, 22, 255)) ||
		     (tColors.iGridColor != XUI_COLOR_RGBA(21, 22, 23, 255)) ||
		     (tColors.iGridStrongColor != XUI_COLOR_RGBA(22, 23, 24, 255)) ||
		     (tColors.iTextColor != XUI_COLOR_RGBA(23, 24, 25, 255)) ||
		     (tColors.iMutedTextColor != XUI_COLOR_RGBA(24, 25, 26, 255)) ||
		     (tColors.iSelectedColor != XUI_COLOR_RGBA(25, 26, 27, 255)) ||
		     (tColors.iHoverColor != XUI_COLOR_RGBA(26, 27, 28, 255)) ||
		     (tColors.iCurrentColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tColors.iFrameColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (tColors.iKeyFrameColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (tColors.iBlankKeyFrameColor != XUI_COLOR_RGBA(35, 36, 37, 255)) ||
		     (tColors.iSpanColor != XUI_COLOR_RGBA(36, 37, 38, 255)) ||
		     (tColors.iSpanTextColor != XUI_COLOR_RGBA(37, 38, 39, 255)) ||
		     (tColors.iIconColor != XUI_COLOR_RGBA(38, 39, 40, 255)) ||
		     (tColors.iHiddenIconColor != XUI_COLOR_RGBA(39, 40, 41, 255)) ||
		     (tColors.iLockedIconColor != XUI_COLOR_RGBA(40, 41, 42, 255)) ||
		     (tColors.iBorderColor != XUI_COLOR_RGBA(41, 42, 43, 255)) ||
		     (tColors.iFocusColor != XUI_COLOR_RGBA(42, 43, 44, 255)) ||
		     (tColors.iDisabledColor != XUI_COLOR_RGBA(43, 44, 45, 255)) ||
		     (tColors.iTrackColor != XUI_COLOR_RGBA(44, 45, 46, 255)) ||
		     (tColors.iThumbColor != XUI_COLOR_RGBA(45, 46, 47, 255)) ||
		     (tColors.iScrollbarHoverColor != XUI_COLOR_RGBA(46, 47, 48, 255)) ||
		     (tColors.iScrollbarActiveColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (tColors.iScrollbarFocusColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (tColors.iScrollbarDisabledColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (sMenuTitle == NULL) || (strcmp(sMenuTitle, "Insert Key") != 0) ) {
			printf("xui_uidesign exercise-timeline-view-properties-failed id=%d layers=%d frame=%d scrollbar=%d metrics=%.1f/%.1f/%.1f/%.1f range=%.1f/%.1f features=%d/%d offset=%.1f/%.1f bg=%08X current=%08X scrollActive=%08X title=%s\n",
				iTimelineView,
				(pTimelineNode != NULL && pTimelineNode->pWidget != NULL) ? xuiTimeLineViewGetLayerCount(pTimelineNode->pWidget) : -1,
				(pTimelineNode != NULL && pTimelineNode->pWidget != NULL) ? xuiTimeLineViewGetCurrentFrame(pTimelineNode->pWidget) : -1,
				(pTimelineNode != NULL && pTimelineNode->pWidget != NULL) ? xuiTimeLineViewGetScrollbarMode(pTimelineNode->pWidget) : -1,
				fLayerHeader, fFrameWidth, fRowHeight, fRulerHeight,
				fMinFrameWidth, fMaxFrameWidth,
				bVisibilityFeature, bLockFeature,
				fOffsetX, fOffsetY,
				tColors.iBackgroundColor, tColors.iCurrentColor, tColors.iScrollbarActiveColor,
				(sMenuTitle != NULL) ? sMenuTitle : "(null)");
			return XUI_ERROR;
		}
	}
	if ( iTerminal != 0 ) {
		ui_design_node_t* pTerminalNode = uiDesignModelGetNode(&pApp->tModel, iTerminal);
		float fCellWidth;
		float fCellHeight;
		float fPadding;
		uint32_t iBackgroundColor;
		uint32_t iForegroundColor;
		uint32_t iCursorColor;
		uint32_t iSelectionColor;
		uint32_t iSelectionTextColor;
		uint32_t iSearchColor;
		uint32_t iFocusColor;
		uint32_t iLinkColor;
		int iAnchorLine;
		int iAnchorColumn;
		int iEndLine;
		int iEndColumn;
		int iFindLine;
		int iFindColumn;
		int iFindLength;
		int iSelectionRet;
		int iFindRet;
		const char* sCopyTitle;
		const char* sFindTitle;
		xui_scroll_model_t* pTerminalScroll;
		float fScrollX;
		float fScrollY;
		fCellWidth = fCellHeight = fPadding = 0.0f;
		iBackgroundColor = iForegroundColor = iCursorColor = iSelectionColor = 0u;
		iSelectionTextColor = iSearchColor = iFocusColor = iLinkColor = 0u;
		iAnchorLine = iAnchorColumn = iEndLine = iEndColumn = -1;
		iFindLine = iFindColumn = iFindLength = -1;
		iSelectionRet = XUI_ERROR;
		iFindRet = XUI_ERROR;
		sCopyTitle = NULL;
		sFindTitle = NULL;
		pTerminalScroll = NULL;
		fScrollX = fScrollY = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "data.text", "alpha beta\nbeta gamma");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "data.palette", "3|#010203\n4|#A0B0C0");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "data.selection", "1|2|1|6");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "data.menuTitles", "copy|Copy Text\nfind|Find Text");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "data.findText", "beta");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.columns", "40");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.rows", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.scrollback", "222");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.parseBudget", "333");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.cellWidth", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.cellHeight", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "metrics.padding", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "behavior.useScroll", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "value.scrollX", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "value.scrollY", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.foregroundColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.cursorColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.selectionColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.selectionTextColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.searchHighlightColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.focusColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "appearance.linkHoverColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "behavior.bracketedPaste", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "behavior.ligaturesEnabled", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "behavior.findCaseSensitive", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iTerminal, "behavior.findBackward", "false");
		if ( pTerminalNode != NULL && pTerminalNode->pWidget != NULL ) {
			(void)xuiTerminalGetMetrics(pTerminalNode->pWidget, &fCellWidth, &fCellHeight, &fPadding);
			(void)xuiTerminalGetColors(pTerminalNode->pWidget, &iBackgroundColor, &iForegroundColor, &iCursorColor,
				&iSelectionColor, &iSelectionTextColor, &iSearchColor, &iFocusColor, &iLinkColor);
			iSelectionRet = xuiTerminalGetSelectionRange(pTerminalNode->pWidget, &iAnchorLine, &iAnchorColumn, &iEndLine, &iEndColumn);
			iFindRet = xuiTerminalGetFindMatch(pTerminalNode->pWidget, &iFindLine, &iFindColumn, &iFindLength);
			sCopyTitle = xuiTerminalGetMenuTitle(pTerminalNode->pWidget, XUI_TERMINAL_MENU_COPY);
			sFindTitle = xuiTerminalGetMenuTitle(pTerminalNode->pWidget, XUI_TERMINAL_MENU_FIND);
			pTerminalScroll = xuiTerminalGetScrollModel(pTerminalNode->pWidget);
			if ( pTerminalScroll != NULL ) {
				(void)xuiScrollModelGetOffset(pTerminalScroll, &fScrollX, &fScrollY);
			}
		}
		if ( (pTerminalNode == NULL) || (pTerminalNode->pWidget == NULL) ||
		     (xuiTerminalGetColumns(pTerminalNode->pWidget) != 40) ||
		     (xuiTerminalGetRows(pTerminalNode->pWidget) != 8) ||
		     (xuiTerminalGetScrollbackLimit(pTerminalNode->pWidget) != 222) ||
		     (xuiTerminalGetParseBudget(pTerminalNode->pWidget) != 333) ||
		     (fCellWidth != 9.0f) || (fCellHeight != 17.0f) || (fPadding != 5.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iForegroundColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iCursorColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iSelectionColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iSelectionTextColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iSearchColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iLinkColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (xuiTerminalGetPalette(pTerminalNode->pWidget, 3) != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (xuiTerminalGetPalette(pTerminalNode->pWidget, 4) != XUI_COLOR_RGBA(160, 176, 192, 255)) ||
		     (pTerminalScroll == NULL) ||
		     (fScrollX != 0.0f) || (fScrollY != 0.0f) ||
		     (xuiTerminalGetBracketedPaste(pTerminalNode->pWidget) == 0) ||
		     (xuiTerminalGetLigaturesEnabled(pTerminalNode->pWidget) == 0) ||
		     (iSelectionRet != XUI_OK) || (iAnchorLine != 0) || (iAnchorColumn != 1) || (iEndLine != 0) || (iEndColumn != 5) ||
		     (iFindRet != 1) || (iFindLine != 0) || (iFindColumn != 6) || (iFindLength != 4) ||
		     (sCopyTitle == NULL) || (strcmp(sCopyTitle, "Copy Text") != 0) ||
		     (sFindTitle == NULL) || (strcmp(sFindTitle, "Find Text") != 0) ) {
			printf("xui_uidesign exercise-terminal-properties-failed id=%d find=%d/%d/%d ret=%d\n", iTerminal, iFindLine, iFindColumn, iFindLength, iFindRet);
			return XUI_ERROR;
		}
	}
	if ( iPropertyGrid != 0 ) {
		ui_design_node_t* pPropertyNode = uiDesignModelGetNode(&pApp->tModel, iPropertyGrid);
		xui_widget pPropertyTableView;
		xui_property_grid_style_t tStyle;
		float fNameWidth;
		float fRowHeight;
		float fCategoryHeight;
		uint32_t iTrackColor;
		uint32_t iThumbColor;
		uint32_t iScrollbarHoverColor;
		uint32_t iScrollbarActiveColor;
		uint32_t iScrollbarFocusColor;
		uint32_t iScrollbarDisabledColor;
		int iGeneralCategory;
		int iAppearanceCategory;
		int iRuntimeCategory;
		int iEnabledProperty;
		int iCountProperty;
		int iTintProperty;
		int iRatioProperty;
		pPropertyTableView = NULL;
		memset(&tStyle, 0, sizeof(tStyle));
		fNameWidth = fRowHeight = fCategoryHeight = 0.0f;
		iTrackColor = iThumbColor = iScrollbarHoverColor = iScrollbarActiveColor = iScrollbarFocusColor = iScrollbarDisabledColor = 0u;
		iGeneralCategory = iAppearanceCategory = iRuntimeCategory = -1;
		iEnabledProperty = iCountProperty = iTintProperty = iRatioProperty = -1;
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "data.properties", "General|Enabled|true|bool|Enabled state|false||general.enabled|false\nGeneral|Count|42|int|Count value|0|dirty+invalid|general.count|false\nAppearance|Tint|#112233|color|Tint color|#000000||appearance.tint|true\nRuntime|Ratio|1.5|float|Ratio value|1.0|readonly|runtime.ratio|true");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "data.selected", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "value.scrollY", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "metrics.nameWidth", "140");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "metrics.rowHeight", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "metrics.categoryHeight", "29");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "metrics.descriptionHeight", "88");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "behavior.descriptionMode", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "behavior.editMode", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "behavior.scrollbarMode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.categoryColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.categoryHoverColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.categoryTextColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.categoryIconColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.rowColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.nameBackgroundColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.nameTextColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.nameHoverColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.valueBackgroundColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.valueTextColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.selectedColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.gridColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.textColor", "#E0E1E2");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.readonlyTextColor", "#AABBCC");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.invalidColor", "#BBCCDD");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.dirtyColor", "#CCDDEE");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.trackColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.thumbColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.scrollbarHoverColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.scrollbarActiveColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.scrollbarFocusColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iPropertyGrid, "appearance.scrollbarDisabledColor", "#515253");
		if ( pPropertyNode != NULL && pPropertyNode->pWidget != NULL ) {
			(void)xuiPropertyGridGetMetrics(pPropertyNode->pWidget, &fNameWidth, &fRowHeight, &fCategoryHeight);
			(void)xuiPropertyGridGetStyle(pPropertyNode->pWidget, &tStyle);
			pPropertyTableView = xuiPropertyGridGetTableView(pPropertyNode->pWidget);
			if ( pPropertyTableView != NULL ) {
				(void)xuiTableViewGetScrollbarColors(pPropertyTableView, &iTrackColor, &iThumbColor, &iScrollbarHoverColor,
					&iScrollbarActiveColor, &iScrollbarFocusColor, &iScrollbarDisabledColor);
			}
			iGeneralCategory = xuiPropertyGridFindCategory(pPropertyNode->pWidget, "general");
			iAppearanceCategory = xuiPropertyGridFindCategory(pPropertyNode->pWidget, "appearance");
			iRuntimeCategory = xuiPropertyGridFindCategory(pPropertyNode->pWidget, "runtime");
			iEnabledProperty = xuiPropertyGridFindProperty(pPropertyNode->pWidget, "general.enabled");
			iCountProperty = xuiPropertyGridFindProperty(pPropertyNode->pWidget, "general.count");
			iTintProperty = xuiPropertyGridFindProperty(pPropertyNode->pWidget, "appearance.tint");
			iRatioProperty = xuiPropertyGridFindProperty(pPropertyNode->pWidget, "runtime.ratio");
		}
		if ( (pPropertyNode == NULL) || (pPropertyNode->pWidget == NULL) ||
		     (xuiPropertyGridGetCategoryCount(pPropertyNode->pWidget) != 3) ||
		     (xuiPropertyGridGetPropertyCount(pPropertyNode->pWidget) != 4) ||
		     (iGeneralCategory < 0) || (iAppearanceCategory < 0) || (iRuntimeCategory < 0) ||
		     (xuiPropertyGridGetCategoryExpanded(pPropertyNode->pWidget, iGeneralCategory) != 0) ||
		     (xuiPropertyGridGetCategoryExpanded(pPropertyNode->pWidget, iAppearanceCategory) == 0) ||
		     (iEnabledProperty < 0) || (iCountProperty < 0) || (iTintProperty < 0) || (iRatioProperty < 0) ||
		     (xuiPropertyGridGetBool(pPropertyNode->pWidget, iEnabledProperty, 0) == 0) ||
		     (xuiPropertyGridGetInt(pPropertyNode->pWidget, iCountProperty, 0) != 42) ||
		     (xuiPropertyGridGetColor(pPropertyNode->pWidget, iTintProperty, 0u) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (xuiPropertyGridGetFloat(pPropertyNode->pWidget, iRatioProperty, 0.0f) != 1.5f) ||
		     ((xuiPropertyGridGetPropertyFlags(pPropertyNode->pWidget, iCountProperty) & (XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID)) != (XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID)) ||
		     ((xuiPropertyGridGetPropertyFlags(pPropertyNode->pWidget, iRatioProperty) & XUI_PROPERTY_FLAG_READONLY) == 0) ||
		     (xuiPropertyGridGetSelected(pPropertyNode->pWidget) != 2) ||
		     (xuiPropertyGridGetDescriptionMode(pPropertyNode->pWidget) != XUI_PROPERTY_GRID_DESCRIPTION_BOTH) ||
		     (xuiPropertyGridGetEditMode(pPropertyNode->pWidget) != XUI_TABLE_GRID_EDIT_IMMEDIATE) ||
		     (xuiPropertyGridGetScrollbarMode(pPropertyNode->pWidget) != XUI_SCROLLBAR_MODE_COMPACT) ||
		     (fNameWidth != 140.0f) || (fRowHeight != 27.0f) || (fCategoryHeight != 29.0f) ||
		     (tStyle.iBackgroundColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (tStyle.iCategoryBackgroundColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (tStyle.iCategoryHoverColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (tStyle.iCategoryTextColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (tStyle.iCategoryIconColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (tStyle.iNameBackgroundColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (tStyle.iNameTextColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (tStyle.iNameHoverColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (tStyle.iValueBackgroundColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (tStyle.iValueTextColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (tStyle.iSelectedColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (tStyle.iGridColor != XUI_COLOR_RGBA(208, 209, 210, 255)) ||
		     (tStyle.iReadonlyTextColor != XUI_COLOR_RGBA(170, 187, 204, 255)) ||
		     (tStyle.iInvalidColor != XUI_COLOR_RGBA(187, 204, 221, 255)) ||
		     (tStyle.iDirtyColor != XUI_COLOR_RGBA(204, 221, 238, 255)) ||
		     (pPropertyTableView == NULL) ||
		     (iTrackColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iThumbColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iScrollbarHoverColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iScrollbarActiveColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iScrollbarFocusColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iScrollbarDisabledColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ) {
			printf("xui_uidesign exercise-property-grid-properties-failed id=%d props=%d cats=%d\n",
				iPropertyGrid,
				(pPropertyNode != NULL && pPropertyNode->pWidget != NULL) ? xuiPropertyGridGetPropertyCount(pPropertyNode->pWidget) : -1,
				(pPropertyNode != NULL && pPropertyNode->pWidget != NULL) ? xuiPropertyGridGetCategoryCount(pPropertyNode->pWidget) : -1);
			return XUI_ERROR;
		}
	}
	if ( iFlowGraph != 0 ) {
		ui_design_node_t* pFlowNode = uiDesignModelGetNode(&pApp->tModel, iFlowGraph);
		xui_flow_graph pGraph;
		xui_flow_node_info_t tNodeInfo;
		xui_flow_port_info_t tPortInfo;
		xui_flow_edge_info_t tEdgeInfo;
		xui_flow_edge_info_t tDefaultEdgeInfo;
		xui_flow_viewport_t tViewport;
		xui_style_property_t tStyleProp;
		xvalue pConfig;
		const char* sModel;
		const char* arrFlowStyleNames[8];
		uint32_t arrFlowStyleColors[8];
		int iProcNode;
		int iEdge;
		int iDefaultEdge;
		int iPort;
		int iPortCount;
		int iStyle;
		int bFoundDataPort;
		int bConfigOK;
		memset(&tNodeInfo, 0, sizeof(tNodeInfo));
		memset(&tPortInfo, 0, sizeof(tPortInfo));
		memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
		memset(&tDefaultEdgeInfo, 0, sizeof(tDefaultEdgeInfo));
		memset(&tViewport, 0, sizeof(tViewport));
		memset(&tStyleProp, 0, sizeof(tStyleProp));
		memset(arrFlowStyleColors, 0, sizeof(arrFlowStyleColors));
		tViewport.iSize = sizeof(tViewport);
		pGraph = NULL;
		pConfig = NULL;
		sModel = NULL;
		iProcNode = -1;
		iEdge = -1;
		iDefaultEdge = -1;
		iPortCount = 0;
		iStyle = 0;
		bFoundDataPort = 0;
		bConfigOK = 0;
		arrFlowStyleNames[0] = "flowgraph.background.color";
		arrFlowStyleNames[1] = "flowgraph.grid.color";
		arrFlowStyleNames[2] = "flowgraph.node.color";
		arrFlowStyleNames[3] = "flowgraph.node.selected_color";
		arrFlowStyleNames[4] = "flowgraph.node.border_color";
		arrFlowStyleNames[5] = "flowgraph.port.color";
		arrFlowStyleNames[6] = "flowgraph.edge.color";
		arrFlowStyleNames[7] = "flowgraph.edge.selected_color";
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.nodes", "src|event.start|Source|10|20|100|50|Start node|running|Now\nproc|action.proc|Process|180|40|120|60|Process node|success|Done\nsink|event.end|Sink|360|80|100|50|End node|idle|");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.ports", "src|out|Out|output|control|false|false|false|\nproc|in|In|input|control|true|false|false|\nproc|out|Out|output|control|false|false|false|\nproc|data|Data|input|data|false|true|true|string\nsink|in|In|input|control|true|false|false|");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.nodeConfigs", "proc|model|string|alpha\nproc|enabled|bool|true\nproc|ratio|float|0.25");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.edges", "e_src_proc|src|out|proc|in|bezier|control|active|Flowing|0.25|11|22\ne_proc_sink|proc|out|sink|in||control|taken|Done|0.75|33|44");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.diagnostics", "error|E001|Bad node|proc||nodes.proc\nwarning|W002|Edge check||e_src_proc|edges.e_src_proc");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.selectedNode", "proc");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "data.selectedEdge", "e_src_proc");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "value.panX", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "value.panY", "22");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "value.zoom", "1.25");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "behavior.commandHistoryLimit", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "behavior.defaultRoute", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.backgroundColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.gridColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.nodeColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.selectedNodeColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.nodeBorderColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.portColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.edgeColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iFlowGraph, "appearance.selectedEdgeColor", "#808182");
		if ( pFlowNode != NULL && pFlowNode->pWidget != NULL ) {
			pGraph = xuiFlowGraphWidgetGetGraph(pFlowNode->pWidget);
			if ( pGraph != NULL ) {
				iProcNode = xuiFlowGraphFindNode(pGraph, "proc");
				if ( iProcNode >= 0 ) {
					(void)xuiFlowGraphGetNode(pGraph, iProcNode, &tNodeInfo);
					iPortCount = xuiFlowGraphGetNodePortCount(pGraph, iProcNode);
					for ( iPort = 0; iPort < iPortCount; ++iPort ) {
						memset(&tPortInfo, 0, sizeof(tPortInfo));
						if ( xuiFlowGraphGetPort(pGraph, iProcNode, iPort, &tPortInfo) == XUI_OK &&
						     tPortInfo.sId != NULL && strcmp(tPortInfo.sId, "data") == 0 ) {
							bFoundDataPort = (tPortInfo.iDirection == XUI_FLOW_PORT_INPUT) &&
								(tPortInfo.iKind == XUI_FLOW_PORT_DATA) &&
								(tPortInfo.bMulti != 0) &&
								(tPortInfo.bDynamic != 0) &&
								(tPortInfo.sDataType != NULL) &&
								(strcmp(tPortInfo.sDataType, "string") == 0);
						}
					}
				}
				iEdge = xuiFlowGraphFindEdge(pGraph, "e_src_proc");
				if ( iEdge >= 0 ) (void)xuiFlowGraphGetEdge(pGraph, iEdge, &tEdgeInfo);
				iDefaultEdge = xuiFlowGraphFindEdge(pGraph, "e_proc_sink");
				if ( iDefaultEdge >= 0 ) (void)xuiFlowGraphGetEdge(pGraph, iDefaultEdge, &tDefaultEdgeInfo);
				(void)xuiFlowGraphGetViewport(pGraph, &tViewport);
				if ( xuiFlowGraphGetNodeConfig(pGraph, "proc", &pConfig) == XUI_OK && pConfig != NULL ) {
					sModel = (const char*)(const void*)xvoTableGetText(pConfig, "model", 5);
					bConfigOK = (sModel != NULL) && (strcmp(sModel, "alpha") == 0) &&
						(xvoTableGetBool(pConfig, "enabled", 7) != 0) &&
						(xvoTableGetFloat(pConfig, "ratio", 5) == 0.25);
					xvoUnref(pConfig);
					pConfig = NULL;
				}
			}
			for ( iStyle = 0; iStyle < 8; ++iStyle ) {
				memset(&tStyleProp, 0, sizeof(tStyleProp));
				if ( xuiWidgetGetInlineStyleProperty(pFlowNode->pWidget, arrFlowStyleNames[iStyle], &tStyleProp) == XUI_OK ) {
					arrFlowStyleColors[iStyle] = tStyleProp.tValue.iColor;
				}
			}
		}
		if ( (pFlowNode == NULL) || (pFlowNode->pWidget == NULL) || (pGraph == NULL) ||
		     (xuiFlowGraphGetNodeCount(pGraph) != 3) ||
		     (xuiFlowGraphGetEdgeCount(pGraph) != 2) ||
		     (iProcNode < 0) ||
		     (strcmp(tNodeInfo.sId, "proc") != 0) ||
		     (strcmp(tNodeInfo.sType, "action.proc") != 0) ||
		     (strcmp(tNodeInfo.sTitle, "Process") != 0) ||
		     (strcmp(tNodeInfo.sSummary, "Process node") != 0) ||
		     (tNodeInfo.fX != 180.0f) || (tNodeInfo.fY != 40.0f) ||
		     (tNodeInfo.fW != 120.0f) || (tNodeInfo.fH != 60.0f) ||
		     (tNodeInfo.iRunState != XUI_WORKFLOW_NODE_RUN_SUCCESS) ||
		     (tNodeInfo.sRunPreview == NULL) || (strcmp(tNodeInfo.sRunPreview, "Done") != 0) ||
		     (iPortCount != 3) || (bFoundDataPort == 0) ||
		     (iEdge < 0) ||
		     (strcmp(tEdgeInfo.sId, "e_src_proc") != 0) ||
		     (strcmp(tEdgeInfo.sFromNode, "src") != 0) ||
		     (strcmp(tEdgeInfo.sToNode, "proc") != 0) ||
		     (tEdgeInfo.iRouteStyle != XUI_FLOW_ROUTE_BEZIER) ||
		     (tEdgeInfo.iRunState != XUI_WORKFLOW_EDGE_RUN_ACTIVE) ||
		     (tEdgeInfo.sRunPreview == NULL) || (strcmp(tEdgeInfo.sRunPreview, "Flowing") != 0) ||
		     (tEdgeInfo.fRouteBias != 0.25f) || (tEdgeInfo.fRouteSourceOffset != 11.0f) || (tEdgeInfo.fRouteTargetOffset != 22.0f) ||
		     (iDefaultEdge < 0) ||
		     (tDefaultEdgeInfo.iRouteStyle != XUI_FLOW_ROUTE_ORTHOGONAL) ||
		     (xuiFlowGraphGetSelectedNodeCount(pGraph) != 1) ||
		     (xuiFlowGraphGetSelectedEdgeCount(pGraph) != 1) ||
		     (xuiFlowGraphIsNodeSelected(pGraph, "proc") == 0) ||
		     (xuiFlowGraphIsEdgeSelected(pGraph, "e_src_proc") == 0) ||
		     (xuiFlowGraphGetCommandHistoryLimit(pGraph) != 13) ||
		     (xuiFlowGraphGetDiagnosticCount(pGraph) != 2) ||
		     (xuiFlowGraphGetNodeDiagnosticCount(pGraph, "proc") != 1) ||
		     (xuiFlowGraphGetEdgeDiagnosticCount(pGraph, "e_src_proc") != 1) ||
		     (tViewport.fPanX != 11.0f) || (tViewport.fPanY != 22.0f) || (tViewport.fZoom != 1.25f) ||
		     (bConfigOK == 0) ||
		     (arrFlowStyleColors[0] != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (arrFlowStyleColors[1] != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (arrFlowStyleColors[2] != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (arrFlowStyleColors[3] != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (arrFlowStyleColors[4] != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (arrFlowStyleColors[5] != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (arrFlowStyleColors[6] != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (arrFlowStyleColors[7] != XUI_COLOR_RGBA(128, 129, 130, 255)) ) {
			printf("xui_uidesign exercise-flow-graph-properties-failed id=%d nodes=%d edges=%d proc=%d edge=%d\n",
				iFlowGraph,
				(pGraph != NULL) ? xuiFlowGraphGetNodeCount(pGraph) : -1,
				(pGraph != NULL) ? xuiFlowGraphGetEdgeCount(pGraph) : -1,
				iProcNode,
				iEdge);
			return XUI_ERROR;
		}
	}
	if ( iWorkflow != 0 ) {
		ui_design_node_t* pWorkflowNode = uiDesignModelGetNode(&pApp->tModel, iWorkflow);
		xui_workflow pWorkflow;
		xui_flow_graph pWorkflowGraph;
		xui_flow_node_info_t tSelectedNode;
		xui_flow_node_info_t tTaskNode;
		xui_flow_edge_info_t tWorkflowEdge;
		xui_flow_edge_info_t tWorkflowDefaultEdge;
		xui_flow_viewport_t tWorkflowViewport;
		xui_workflow_node_run_state_t tTaskState;
		xui_workflow_edge_run_state_t tWorkflowEdgeState;
		xui_workflow_variable_desc_t tVariable;
		xui_style_property_t tWorkflowStyle;
		xvalue pWorkflowConfig;
		const char* sLabel;
		const char* arrWorkflowStyleNames[8];
		uint32_t arrWorkflowStyleColors[8];
		int iTaskNode;
		int iWorkflowEdge;
		int iWorkflowDefaultEdge;
		int iWorkflowStyle;
		int bWorkflowConfigOK;
		memset(&tSelectedNode, 0, sizeof(tSelectedNode));
		memset(&tTaskNode, 0, sizeof(tTaskNode));
		memset(&tWorkflowEdge, 0, sizeof(tWorkflowEdge));
		memset(&tWorkflowDefaultEdge, 0, sizeof(tWorkflowDefaultEdge));
		memset(&tWorkflowViewport, 0, sizeof(tWorkflowViewport));
		memset(&tTaskState, 0, sizeof(tTaskState));
		memset(&tWorkflowEdgeState, 0, sizeof(tWorkflowEdgeState));
		memset(&tVariable, 0, sizeof(tVariable));
		memset(&tWorkflowStyle, 0, sizeof(tWorkflowStyle));
		memset(arrWorkflowStyleColors, 0, sizeof(arrWorkflowStyleColors));
		tWorkflowViewport.iSize = sizeof(tWorkflowViewport);
		tTaskState.iSize = sizeof(tTaskState);
		tWorkflowEdgeState.iSize = sizeof(tWorkflowEdgeState);
		tVariable.iSize = sizeof(tVariable);
		pWorkflow = NULL;
		pWorkflowGraph = NULL;
		pWorkflowConfig = NULL;
		sLabel = NULL;
		iTaskNode = -1;
		iWorkflowEdge = -1;
		iWorkflowDefaultEdge = -1;
		iWorkflowStyle = 0;
		bWorkflowConfigOK = 0;
		arrWorkflowStyleNames[0] = "flowgraph.background.color";
		arrWorkflowStyleNames[1] = "flowgraph.grid.color";
		arrWorkflowStyleNames[2] = "flowgraph.node.color";
		arrWorkflowStyleNames[3] = "flowgraph.node.selected_color";
		arrWorkflowStyleNames[4] = "flowgraph.node.border_color";
		arrWorkflowStyleNames[5] = "flowgraph.port.color";
		arrWorkflowStyleNames[6] = "flowgraph.edge.color";
		arrWorkflowStyleNames[7] = "flowgraph.edge.selected_color";
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.nodeTypes", "start|Start|Flow||out|1|Entry|\ntask.mock|Task Mock|Action|in|out|2|Mock task|\nend|End|Flow|in||1|Exit|");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.typePorts", "start|out|Out|output|control|false|false|false|\ntask.mock|in|In|input|control|true|false|false|\ntask.mock|out|Out|output|control|false|false|false|\nend|in|In|input|control|true|false|false|");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.configFields", "task.mock|label|Label|string|true|demo|||||||||\ntask.mock|enabled|Enabled|bool|false|true|||||||||\ntask.mock|threshold|Threshold|float|false|0.5|0|1|||||||");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.nodes", "start|start|Start|20|100|120|60|Entry|idle|\ntask.mock|task|Task|180|120|140|70|Task summary|running|Running\nend|end|End|380|120|120|60|Exit|idle|");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.nodeConfigs", "task|label|string|demo\ntask|enabled|bool|true\ntask|threshold|float|0.8");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.edges", "e_start_task|start|out|task|in|straight|control|active|Active|0.2|10|12\ne_task_end|task|out|end|in||control|taken|Done|0.8|20|24");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.variables", "var_prompt|Prompt|string|workflow|hello\nvar_count|Count|int|session|3");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.diagnostics", "warning|W001|Config warning|task||nodes.task");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "data.selectedNode", "task");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "value.panX", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "value.panY", "8");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "value.zoom", "0.9");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "behavior.commandHistoryLimit", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "behavior.defaultRoute", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.backgroundColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.gridColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.nodeColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.selectedNodeColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.nodeBorderColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.portColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.edgeColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iWorkflow, "appearance.selectedEdgeColor", "#909192");
		if ( pWorkflowNode != NULL && pWorkflowNode->pWidget != NULL ) {
			pWorkflow = xuiWorkflowWidgetGetWorkflow(pWorkflowNode->pWidget);
			pWorkflowGraph = (pWorkflow != NULL) ? xuiWorkflowGetGraph(pWorkflow) : NULL;
			if ( pWorkflow != NULL && pWorkflowGraph != NULL ) {
				iTaskNode = xuiFlowGraphFindNode(pWorkflowGraph, "task");
				if ( iTaskNode >= 0 ) (void)xuiFlowGraphGetNode(pWorkflowGraph, iTaskNode, &tTaskNode);
				iWorkflowEdge = xuiFlowGraphFindEdge(pWorkflowGraph, "e_start_task");
				if ( iWorkflowEdge >= 0 ) (void)xuiFlowGraphGetEdge(pWorkflowGraph, iWorkflowEdge, &tWorkflowEdge);
				iWorkflowDefaultEdge = xuiFlowGraphFindEdge(pWorkflowGraph, "e_task_end");
				if ( iWorkflowDefaultEdge >= 0 ) (void)xuiFlowGraphGetEdge(pWorkflowGraph, iWorkflowDefaultEdge, &tWorkflowDefaultEdge);
				(void)xuiFlowGraphGetViewport(pWorkflowGraph, &tWorkflowViewport);
				(void)xuiWorkflowGetSelectedNode(pWorkflow, &tSelectedNode);
				(void)xuiWorkflowGetNodeRunState(pWorkflow, "task", &tTaskState);
				(void)xuiWorkflowGetEdgeRunState(pWorkflow, "e_start_task", &tWorkflowEdgeState);
				(void)xuiWorkflowGetVariable(pWorkflow, 1, &tVariable);
				if ( xuiWorkflowGetNodeConfig(pWorkflow, "task", &pWorkflowConfig) == XUI_OK && pWorkflowConfig != NULL ) {
					double fThreshold;
					sLabel = (const char*)(const void*)xvoTableGetText(pWorkflowConfig, "label", 5);
					fThreshold = xvoTableGetFloat(pWorkflowConfig, "threshold", 9);
					bWorkflowConfigOK = (sLabel != NULL) && (strcmp(sLabel, "demo") == 0) &&
						(xvoTableGetBool(pWorkflowConfig, "enabled", 7) != 0) &&
						(fThreshold > 0.79) && (fThreshold < 0.81);
					xvoUnref(pWorkflowConfig);
					pWorkflowConfig = NULL;
				}
			}
			if ( xuiWorkflowWidgetGetCanvas(pWorkflowNode->pWidget) != NULL ) {
				for ( iWorkflowStyle = 0; iWorkflowStyle < 8; ++iWorkflowStyle ) {
					memset(&tWorkflowStyle, 0, sizeof(tWorkflowStyle));
					if ( xuiWidgetGetInlineStyleProperty(xuiWorkflowWidgetGetCanvas(pWorkflowNode->pWidget), arrWorkflowStyleNames[iWorkflowStyle], &tWorkflowStyle) == XUI_OK ) {
						arrWorkflowStyleColors[iWorkflowStyle] = tWorkflowStyle.tValue.iColor;
					}
				}
			}
		}
		if ( (pWorkflowNode == NULL) || (pWorkflowNode->pWidget == NULL) || (pWorkflow == NULL) || (pWorkflowGraph == NULL) ||
		     (xuiWorkflowGetNodeTypeCount(pWorkflow) != 3) ||
		     (xuiWorkflowFindNodeType(pWorkflow, "task.mock") < 0) ||
		     (xuiWorkflowGetNodeLibraryCount(pWorkflow) != 3) ||
		     (xuiWorkflowGetNodeCount(pWorkflow) != 3) ||
		     (xuiFlowGraphGetEdgeCount(pWorkflowGraph) != 2) ||
		     (iTaskNode < 0) ||
		     (strcmp(tTaskNode.sId, "task") != 0) ||
		     (strcmp(tTaskNode.sType, "task.mock") != 0) ||
		     (strcmp(tTaskNode.sTitle, "Task") != 0) ||
		     (strcmp(tTaskNode.sSummary, "label=demo, enabled=true, threshold=0.80") != 0) ||
		     (tTaskNode.fX != 180.0f) || (tTaskNode.fY != 120.0f) ||
		     (tTaskNode.fW != 140.0f) || (tTaskNode.fH != 70.0f) ||
		     (strcmp(tSelectedNode.sId, "task") != 0) ||
		     (tTaskState.iState != XUI_WORKFLOW_NODE_RUN_RUNNING) ||
		     (tTaskState.sPreview == NULL) || (strcmp(tTaskState.sPreview, "Running") != 0) ||
		     (iWorkflowEdge < 0) ||
		     (strcmp(tWorkflowEdge.sId, "e_start_task") != 0) ||
		     (tWorkflowEdge.iRouteStyle != XUI_FLOW_ROUTE_STRAIGHT) ||
		     (tWorkflowEdge.fRouteBias < 0.19f) || (tWorkflowEdge.fRouteBias > 0.21f) ||
		     (tWorkflowEdge.fRouteSourceOffset != 10.0f) || (tWorkflowEdge.fRouteTargetOffset != 12.0f) ||
		     (iWorkflowDefaultEdge < 0) ||
		     (tWorkflowDefaultEdge.iRouteStyle != XUI_FLOW_ROUTE_ORTHOGONAL) ||
		     (tWorkflowEdgeState.iState != XUI_WORKFLOW_EDGE_RUN_ACTIVE) ||
		     (tWorkflowEdgeState.sPreview == NULL) || (strcmp(tWorkflowEdgeState.sPreview, "Active") != 0) ||
		     (xuiWorkflowGetVariableCount(pWorkflow) != 2) ||
		     (strcmp(tVariable.sId, "var_count") != 0) ||
		     (strcmp(tVariable.sType, "int") != 0) ||
		     (xuiFlowGraphGetCommandHistoryLimit(pWorkflowGraph) != 17) ||
		     (xuiFlowGraphGetDiagnosticCount(pWorkflowGraph) != 1) ||
		     (xuiFlowGraphGetNodeDiagnosticCount(pWorkflowGraph, "task") != 1) ||
		     (tWorkflowViewport.fPanX != 7.0f) || (tWorkflowViewport.fPanY != 8.0f) ||
		     (tWorkflowViewport.fZoom < 0.89f) || (tWorkflowViewport.fZoom > 0.91f) ||
		     (bWorkflowConfigOK == 0) ||
		     (arrWorkflowStyleColors[0] != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (arrWorkflowStyleColors[1] != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (arrWorkflowStyleColors[2] != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (arrWorkflowStyleColors[3] != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (arrWorkflowStyleColors[4] != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (arrWorkflowStyleColors[5] != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (arrWorkflowStyleColors[6] != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (arrWorkflowStyleColors[7] != XUI_COLOR_RGBA(144, 145, 146, 255)) ) {
			printf("xui_uidesign exercise-workflow-properties-failed id=%d nodes=%d edges=%d types=%d task=%d edge=%d\n",
				iWorkflow,
				(pWorkflow != NULL) ? xuiWorkflowGetNodeCount(pWorkflow) : -1,
				(pWorkflowGraph != NULL) ? xuiFlowGraphGetEdgeCount(pWorkflowGraph) : -1,
				(pWorkflow != NULL) ? xuiWorkflowGetNodeTypeCount(pWorkflow) : -1,
				iTaskNode,
				iWorkflowEdge);
			printf("workflow-detail selected=%s taskState=%d/%s edgeRoute=%d edgeState=%d/%s vars=%d var=%s/%s history=%d diag=%d nodeDiag=%d viewport=%.2f/%.2f/%.2f config=%d style=%08X\n",
				(tSelectedNode.sId != NULL) ? tSelectedNode.sId : "",
				tTaskState.iState,
				(tTaskState.sPreview != NULL) ? tTaskState.sPreview : "",
				tWorkflowEdge.iRouteStyle,
				tWorkflowEdgeState.iState,
				(tWorkflowEdgeState.sPreview != NULL) ? tWorkflowEdgeState.sPreview : "",
				(pWorkflow != NULL) ? xuiWorkflowGetVariableCount(pWorkflow) : -1,
				(tVariable.sId != NULL) ? tVariable.sId : "",
				(tVariable.sType != NULL) ? tVariable.sType : "",
				(pWorkflowGraph != NULL) ? xuiFlowGraphGetCommandHistoryLimit(pWorkflowGraph) : -1,
				(pWorkflowGraph != NULL) ? xuiFlowGraphGetDiagnosticCount(pWorkflowGraph) : -1,
				(pWorkflowGraph != NULL) ? xuiFlowGraphGetNodeDiagnosticCount(pWorkflowGraph, "task") : -1,
				tWorkflowViewport.fPanX,
				tWorkflowViewport.fPanY,
				tWorkflowViewport.fZoom,
				bWorkflowConfigOK,
				arrWorkflowStyleColors[0]);
			printf("workflow-node-detail task=%s/%s/%s/%s rect=%.2f/%.2f/%.2f/%.2f edge=%s %.6f/%.2f/%.2f\n",
				(tTaskNode.sId != NULL) ? tTaskNode.sId : "",
				(tTaskNode.sType != NULL) ? tTaskNode.sType : "",
				(tTaskNode.sTitle != NULL) ? tTaskNode.sTitle : "",
				(tTaskNode.sSummary != NULL) ? tTaskNode.sSummary : "",
				tTaskNode.fX,
				tTaskNode.fY,
				tTaskNode.fW,
				tTaskNode.fH,
				(tWorkflowEdge.sId != NULL) ? tWorkflowEdge.sId : "",
				tWorkflowEdge.fRouteBias,
				tWorkflowEdge.fRouteSourceOffset,
				tWorkflowEdge.fRouteTargetOffset);
			return XUI_ERROR;
		}
	}
	if ( iTagInput != 0 ) {
		ui_design_node_t* pTagNode = uiDesignModelGetNode(&pApp->tModel, iTagInput);
		float fBorderWidth = 0.0f;
		float fTagHeight = 0.0f;
		uint32_t iBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iTagBackgroundColor = 0u;
		uint32_t iTagTextColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iPlaceholderColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iFocusBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iTagHoverBackgroundColor = 0u;
		uint32_t iTagCloseColor = 0u;
		uint32_t iTagCloseHoverColor = 0u;
		xui_widget pTagInputWidget = NULL;
		const char* sTagCopyTitle = NULL;
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "data.tags", "red\ngreen\nblue");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "data.menuTitles", "copy|Copy Tag Text\nselectAll|Select All Tags");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "text.inputText", "draft");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "text.placeholder", "Add item");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "text.maxLength", "18");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "behavior.maxTags", "5");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "metrics.tagHeight", "26");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.backgroundColor", "#141516");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.hoverBackgroundColor", "#242526");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.focusBackgroundColor", "#343536");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.disabledBackgroundColor", "#444546");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.textColor", "#545556");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.placeholderColor", "#646566");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.disabledTextColor", "#747576");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.borderColor", "#848586");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.hoverBorderColor", "#949596");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.focusBorderColor", "#A4A5A6");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.tagBackgroundColor", "#B4B5B6");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.tagHoverBackgroundColor", "#C4C5C6");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.tagTextColor", "#D4D5D6");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.tagCloseColor", "#E4E5E6");
		(void)uiDesignAppSetNodeProperty(pApp, iTagInput, "appearance.tagCloseHoverColor", "#F4F5F6");
		if ( pTagNode != NULL && pTagNode->pWidget != NULL ) {
			(void)xuiTagInputGetVisualMetrics(pTagNode->pWidget, &fBorderWidth, &fTagHeight);
			(void)xuiTagInputGetColors(pTagNode->pWidget, &iBackgroundColor, &iBorderColor, &iFocusBorderColor,
				&iTagBackgroundColor, &iTagTextColor);
			(void)xuiTagInputGetExtendedColors(pTagNode->pWidget, &iTextColor, &iPlaceholderColor, &iDisabledTextColor,
				&iHoverBackgroundColor, &iFocusBackgroundColor, &iDisabledBackgroundColor, &iHoverBorderColor,
				&iTagHoverBackgroundColor, &iTagCloseColor, &iTagCloseHoverColor);
			pTagInputWidget = xuiTagInputGetInputWidget(pTagNode->pWidget);
			if ( pTagInputWidget != NULL ) {
				sTagCopyTitle = xuiInputGetMenuTitle(pTagInputWidget, XUI_INPUT_MENU_COPY);
			}
		}
		if ( (pTagNode == NULL) || (pTagNode->pWidget == NULL) ||
		     (xuiTagInputGetTagCount(pTagNode->pWidget) != 3) ||
		     (xuiTagInputGetTag(pTagNode->pWidget, 1) == NULL) ||
		     (strcmp(xuiTagInputGetTag(pTagNode->pWidget, 1), "green") != 0) ||
		     (strcmp(xuiTagInputGetText(pTagNode->pWidget), "draft") != 0) ||
		     (strcmp(xuiTagInputGetPlaceholder(pTagNode->pWidget), "Add item") != 0) ||
		     (xuiTagInputGetMaxLength(pTagNode->pWidget) != 18) ||
		     (xuiTagInputGetMaxTags(pTagNode->pWidget) != 5) ||
		     (sTagCopyTitle == NULL) ||
		     (strcmp(sTagCopyTitle, "Copy Tag Text") != 0) ||
		     (fBorderWidth != 2.0f) ||
		     (fTagHeight != 26.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(20, 21, 22, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(132, 133, 134, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(164, 165, 166, 255)) ||
		     (iTagBackgroundColor != XUI_COLOR_RGBA(180, 181, 182, 255)) ||
		     (iTagTextColor != XUI_COLOR_RGBA(212, 213, 214, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(84, 85, 86, 255)) ||
		     (iPlaceholderColor != XUI_COLOR_RGBA(100, 101, 102, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(116, 117, 118, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(36, 37, 38, 255)) ||
		     (iFocusBackgroundColor != XUI_COLOR_RGBA(52, 53, 54, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(68, 69, 70, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(148, 149, 150, 255)) ||
		     (iTagHoverBackgroundColor != XUI_COLOR_RGBA(196, 197, 198, 255)) ||
		     (iTagCloseColor != XUI_COLOR_RGBA(228, 229, 230, 255)) ||
		     (iTagCloseHoverColor != XUI_COLOR_RGBA(244, 245, 246, 255)) ) {
			printf("xui_uidesign exercise-tag-input-properties-failed id=%d\n", iTagInput);
			return XUI_ERROR;
		}
	}
	if ( iComboBox != 0 ) {
		ui_design_node_t* pComboNode = uiDesignModelGetNode(&pApp->tModel, iComboBox);
		const xui_combobox_item_t* pItem;
		uint32_t iTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iBackgroundColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iOpenBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iArrowColor = 0u;
		uint32_t iDisabledArrowColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonHoverColor = 0u;
		uint32_t iButtonOpenColor = 0u;
		uint32_t iPopupPanelColor = 0u;
		uint32_t iPopupBorderColor = 0u;
		uint32_t iPopupShadowColor = 0u;
		uint32_t iPopupHoverColor = 0u;
		uint32_t iPopupTextColor = 0u;
		uint32_t iPopupHoverTextColor = 0u;
		uint32_t iPopupDisabledTextColor = 0u;
		uint32_t iPopupSeparatorColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "data.items", "Alpha|true|false|10|0|false\nBeta|false|false|20|0|false\nGamma|true|false|30|0|false");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "data.menuTitles", "copy|Copy Text\nselectAll|Select All Text");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "data.selected", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "behavior.useValue", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "data.selectedValue", "30");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "behavior.mode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "text.inputText", "Gamma");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "text.placeholder", "Pick one");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "text.maxLength", "32");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "metrics.itemHeight", "28");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "metrics.popupHeight", "92");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "metrics.popupMaxHeight", "180");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "behavior.popupPlacement", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.textColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.disabledTextColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.backgroundColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.hoverBackgroundColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.openBackgroundColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.disabledBackgroundColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.borderColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.hoverBorderColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.focusBorderColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.arrowColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.disabledArrowColor", "#B1B2B3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.buttonColor", "#C1C2C3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.buttonHoverColor", "#D1D2D3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.buttonOpenColor", "#E1E2E3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupPanelColor", "#F1F2F3");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupBorderColor", "#A4A5A6");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupShadowColor", "#01020304");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupHoverColor", "#B4B5B6");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupTextColor", "#C4C5C6");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupHoverTextColor", "#D4D5D6");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupDisabledTextColor", "#E4E5E6E7");
		(void)uiDesignAppSetNodeProperty(pApp, iComboBox, "appearance.popupSeparatorColor", "#F4F5F6");
		pItem = (pComboNode != NULL && pComboNode->pWidget != NULL) ? xuiComboBoxGetItem(pComboNode->pWidget, 2) : NULL;
		fA = 0.0f;
		fB = 0.0f;
		if ( pComboNode != NULL && pComboNode->pWidget != NULL ) {
			(void)xuiComboBoxGetMetrics(pComboNode->pWidget, &fA, &fB);
			(void)xuiComboBoxGetColors(pComboNode->pWidget, &iTextColor, &iDisabledTextColor, &iBackgroundColor, &iHoverBackgroundColor, &iOpenBackgroundColor, &iDisabledBackgroundColor);
			(void)xuiComboBoxGetBorderColors(pComboNode->pWidget, &iBorderColor, &iHoverBorderColor, &iFocusBorderColor);
			(void)xuiComboBoxGetArrowColors(pComboNode->pWidget, &iArrowColor, &iDisabledArrowColor);
			(void)xuiComboBoxGetButtonColors(pComboNode->pWidget, &iButtonColor, &iButtonHoverColor, &iButtonOpenColor);
			(void)xuiComboBoxGetPopupColors(pComboNode->pWidget, &iPopupPanelColor, &iPopupBorderColor, &iPopupShadowColor, &iPopupHoverColor, &iPopupTextColor, &iPopupHoverTextColor, &iPopupDisabledTextColor, &iPopupSeparatorColor);
		}
		if ( (pComboNode == NULL) || (pComboNode->pWidget == NULL) ||
		     (xuiComboBoxGetItemCount(pComboNode->pWidget) != 3) ||
		     (xuiComboBoxIsItemEnabled(pComboNode->pWidget, 1) != 0) ||
		     (pItem == NULL) || (pItem->iValue != 30) ||
		     (xuiComboBoxGetSelected(pComboNode->pWidget) != 2) ||
		     (xuiComboBoxGetSelectedValue(pComboNode->pWidget) != 30) ||
		     (xuiComboBoxGetMode(pComboNode->pWidget) != XUI_COMBOBOX_MODE_EDIT) ||
		     (xuiComboBoxIsOpen(pComboNode->pWidget) == 0) ||
		     ((xuiComboBoxGetState(pComboNode->pWidget) & XUI_COMBOBOX_STATE_OPEN) == 0u) ||
		     (strcmp(xuiComboBoxGetText(pComboNode->pWidget), "Gamma") != 0) ||
		     (strcmp(xuiComboBoxGetPlaceholder(pComboNode->pWidget), "Pick one") != 0) ||
		     (xuiComboBoxGetMaxLength(pComboNode->pWidget) != 32) ||
		     (xuiComboBoxGetInputMenuTitle(pComboNode->pWidget, XUI_INPUT_MENU_COPY) == NULL) ||
		     (strcmp(xuiComboBoxGetInputMenuTitle(pComboNode->pWidget, XUI_INPUT_MENU_COPY), "Copy Text") != 0) ||
		     (xuiComboBoxGetInputMenuTitle(pComboNode->pWidget, XUI_INPUT_MENU_SELECT_ALL) == NULL) ||
		     (strcmp(xuiComboBoxGetInputMenuTitle(pComboNode->pWidget, XUI_INPUT_MENU_SELECT_ALL), "Select All Text") != 0) ||
		     (fA != 28.0f) || (fB != 2.0f) ||
		     (xuiComboBoxGetPopupHeight(pComboNode->pWidget) != 92.0f) ||
		     (xuiComboBoxGetPopupMaxHeight(pComboNode->pWidget) != 180.0f) ||
		     (xuiComboBoxGetPopupPlacement(pComboNode->pWidget) != XUI_COMBOBOX_POPUP_TOP) ||
		     (iTextColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iOpenBackgroundColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (iArrowColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (iDisabledArrowColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(193, 194, 195, 255)) ||
		     (iButtonHoverColor != XUI_COLOR_RGBA(209, 210, 211, 255)) ||
		     (iButtonOpenColor != XUI_COLOR_RGBA(225, 226, 227, 255)) ||
		     (iPopupPanelColor != XUI_COLOR_RGBA(241, 242, 243, 255)) ||
		     (iPopupBorderColor != XUI_COLOR_RGBA(164, 165, 166, 255)) ||
		     (iPopupShadowColor != XUI_COLOR_RGBA(1, 2, 3, 4)) ||
		     (iPopupHoverColor != XUI_COLOR_RGBA(180, 181, 182, 255)) ||
		     (iPopupTextColor != XUI_COLOR_RGBA(196, 197, 198, 255)) ||
		     (iPopupHoverTextColor != XUI_COLOR_RGBA(212, 213, 214, 255)) ||
		     (iPopupDisabledTextColor != XUI_COLOR_RGBA(228, 229, 230, 231)) ||
		     (iPopupSeparatorColor != XUI_COLOR_RGBA(244, 245, 246, 255)) ) {
			printf("xui_uidesign exercise-combobox-properties-failed id=%d\n", iComboBox);
			return XUI_ERROR;
		}
	}
	if ( iColorPicker != 0 ) {
		ui_design_node_t* pColorNode = uiDesignModelGetNode(&pApp->tModel, iColorPicker);
		uint32_t iTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iBackgroundColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iOpenBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iArrowColor = 0u;
		uint32_t iDisabledArrowColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonHoverColor = 0u;
		uint32_t iButtonOpenColor = 0u;
		uint32_t iPopupPanelColor = 0u;
		uint32_t iPopupBorderColor = 0u;
		uint32_t iPopupShadowColor = 0u;
		uint32_t iPopupTextColor = 0u;
		uint32_t iPopupMutedTextColor = 0u;
		uint32_t iPopupAccentColor = 0u;
		uint32_t iPopupFieldColor = 0u;
		uint32_t iPopupFieldBorderColor = 0u;
		uint32_t iPopupSeparatorColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "value.useHex", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "value.color", "#224466");
		if ( (pColorNode == NULL) || (pColorNode->pWidget == NULL) ||
		     (xuiColorPickerGetColor(pColorNode->pWidget) != XUI_COLOR_RGBA(34, 68, 102, 255)) ) {
			printf("xui_uidesign exercise-color-picker-value-color-failed id=%d\n", iColorPicker);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "behavior.alphaEnabled", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "value.useHex", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "value.hex", "#11223344");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "data.palette", "#010203\n#A0B0C0\n#FFEEDD");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "metrics.popupWidth", "320");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "metrics.popupHeight", "260");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "metrics.borderWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "behavior.popupPlacement", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.textColor", "#121314");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.disabledTextColor", "#222324");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.backgroundColor", "#323334");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.hoverBackgroundColor", "#424344");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.openBackgroundColor", "#525354");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.disabledBackgroundColor", "#626364");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.borderColor", "#727374");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.hoverBorderColor", "#828384");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.focusBorderColor", "#929394");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.arrowColor", "#A2A3A4");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.disabledArrowColor", "#B2B3B4");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.buttonColor", "#C2C3C4");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.buttonHoverColor", "#D2D3D4");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.buttonOpenColor", "#E2E3E4");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupPanelColor", "#131415");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupBorderColor", "#232425");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupShadowColor", "#33343540");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupTextColor", "#434445");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupMutedTextColor", "#535455");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupAccentColor", "#636465");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupFieldColor", "#737475");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupFieldBorderColor", "#838485");
		(void)uiDesignAppSetNodeProperty(pApp, iColorPicker, "appearance.popupSeparatorColor", "#939495");
		fA = 0.0f;
		fB = 0.0f;
		if ( pColorNode != NULL && pColorNode->pWidget != NULL ) {
			(void)xuiColorPickerGetPopupSize(pColorNode->pWidget, &fA, &fB);
			(void)xuiColorPickerGetColors(pColorNode->pWidget, &iTextColor, &iDisabledTextColor, &iBackgroundColor, &iHoverBackgroundColor, &iOpenBackgroundColor, &iDisabledBackgroundColor);
			(void)xuiColorPickerGetBorderColors(pColorNode->pWidget, &iBorderColor, &iHoverBorderColor, &iFocusBorderColor);
			(void)xuiColorPickerGetArrowColors(pColorNode->pWidget, &iArrowColor, &iDisabledArrowColor);
			(void)xuiColorPickerGetButtonColors(pColorNode->pWidget, &iButtonColor, &iButtonHoverColor, &iButtonOpenColor);
			(void)xuiColorPickerGetPopupColors(pColorNode->pWidget, &iPopupPanelColor, &iPopupBorderColor, &iPopupShadowColor, &iPopupTextColor, &iPopupMutedTextColor, &iPopupAccentColor, &iPopupFieldColor, &iPopupFieldBorderColor, &iPopupSeparatorColor);
		}
		if ( (pColorNode == NULL) || (pColorNode->pWidget == NULL) ||
		     (xuiColorPickerGetAlphaEnabled(pColorNode->pWidget) != 0) ||
		     (xuiColorPickerIsOpen(pColorNode->pWidget) == 0) ||
		     (xuiColorPickerGetColor(pColorNode->pWidget) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (xuiColorPickerGetPaletteCount(pColorNode->pWidget) != 3) ||
		     (xuiColorPickerGetPaletteColor(pColorNode->pWidget, 1) != XUI_COLOR_RGBA(160, 176, 192, 255)) ||
		     (fA != 320.0f) || (fB != 260.0f) ||
		     (xuiColorPickerGetPopupPlacement(pColorNode->pWidget) != XUI_COLOR_PICKER_POPUP_BOTTOM) ) {
			printf("xui_uidesign exercise-color-picker-properties-failed id=%d\n", iColorPicker);
			return XUI_ERROR;
		}
		fA = 0.0f;
		(void)xuiColorPickerGetMetrics(pColorNode->pWidget, &fA);
		if ( fA != 3.0f ) {
			printf("xui_uidesign exercise-color-picker-metrics-failed id=%d\n", iColorPicker);
			return XUI_ERROR;
		}
		if ( (iTextColor != XUI_COLOR_RGBA(18, 19, 20, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(34, 35, 36, 255)) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(50, 51, 52, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(66, 67, 68, 255)) ||
		     (iOpenBackgroundColor != XUI_COLOR_RGBA(82, 83, 84, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(98, 99, 100, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(114, 115, 116, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(130, 131, 132, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(146, 147, 148, 255)) ||
		     (iArrowColor != XUI_COLOR_RGBA(162, 163, 164, 255)) ||
		     (iDisabledArrowColor != XUI_COLOR_RGBA(178, 179, 180, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(194, 195, 196, 255)) ||
		     (iButtonHoverColor != XUI_COLOR_RGBA(210, 211, 212, 255)) ||
		     (iButtonOpenColor != XUI_COLOR_RGBA(226, 227, 228, 255)) ||
		     (iPopupPanelColor != XUI_COLOR_RGBA(19, 20, 21, 255)) ||
		     (iPopupBorderColor != XUI_COLOR_RGBA(35, 36, 37, 255)) ||
		     (iPopupShadowColor != XUI_COLOR_RGBA(51, 52, 53, 64)) ||
		     (iPopupTextColor != XUI_COLOR_RGBA(67, 68, 69, 255)) ||
		     (iPopupMutedTextColor != XUI_COLOR_RGBA(83, 84, 85, 255)) ||
		     (iPopupAccentColor != XUI_COLOR_RGBA(99, 100, 101, 255)) ||
		     (iPopupFieldColor != XUI_COLOR_RGBA(115, 116, 117, 255)) ||
		     (iPopupFieldBorderColor != XUI_COLOR_RGBA(131, 132, 133, 255)) ||
		     (iPopupSeparatorColor != XUI_COLOR_RGBA(147, 148, 149, 255)) ) {
			printf("xui_uidesign exercise-color-picker-colors-failed id=%d\n", iColorPicker);
			return XUI_ERROR;
		}
	}
	if ( iDatePicker != 0 ) {
		ui_design_node_t* pDateNode = uiDesignModelGetNode(&pApp->tModel, iDatePicker);
		uint32_t iTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iBackgroundColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iOpenBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iArrowColor = 0u;
		uint32_t iDisabledArrowColor = 0u;
		uint32_t iPopupPanelColor = 0u;
		uint32_t iPopupBorderColor = 0u;
		uint32_t iPopupShadowColor = 0u;
		uint32_t iPopupTextColor = 0u;
		uint32_t iPopupMutedTextColor = 0u;
		uint32_t iPopupAccentColor = 0u;
		uint32_t iPopupFieldColor = 0u;
		uint32_t iPopupFieldBorderColor = 0u;
		uint32_t iPopupSelectedTextColor = 0u;
		uint32_t iPopupDisabledDayColor = 0u;
		uint32_t iPopupSeparatorColor = 0u;
		xtime tSingle = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.mode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "value.hasValue", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "value.value", "2026-02-03");
		if ( pDateNode != NULL && pDateNode->pWidget != NULL ) {
			tSingle = xuiDatePickerGetValue(pDateNode->pWidget);
		}
		if ( (pDateNode == NULL) || (pDateNode->pWidget == NULL) ||
		     (xuiDatePickerHasValue(pDateNode->pWidget) == 0) ||
		     (tSingle == 0) ) {
			printf("xui_uidesign exercise-date-picker-single-value-failed id=%d\n", iDatePicker);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.mode", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "value.rangeStart", "2026-02-01");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "value.rangeEnd", "2026-02-04");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "value.defaultRangeSpan", "2d");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "limits.hasMin", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "limits.min", "2026-01-31");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "limits.hasMax", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "limits.max", "2026-02-10");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.nullable", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.showSecond", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.firstDayOfWeek", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "text.format", "yyyy/mm/dd");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "text.rangeSeparator", " to ");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "metrics.popupWidth", "640");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "metrics.popupHeight", "340");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.popupPlacement", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.textColor", "#141516");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.disabledTextColor", "#242526");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.backgroundColor", "#343536");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.hoverBackgroundColor", "#444546");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.openBackgroundColor", "#545556");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.disabledBackgroundColor", "#646566");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.borderColor", "#747576");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.hoverBorderColor", "#848586");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.focusBorderColor", "#949596");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.arrowColor", "#A4A5A6");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.disabledArrowColor", "#B4B5B6");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupPanelColor", "#151617");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupBorderColor", "#252627");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupShadowColor", "#35363740");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupTextColor", "#454647");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupMutedTextColor", "#555657");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupAccentColor", "#656667");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupFieldColor", "#757677");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupFieldBorderColor", "#858687");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupSelectedTextColor", "#959697");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupDisabledDayColor", "#A5A6A7");
		(void)uiDesignAppSetNodeProperty(pApp, iDatePicker, "appearance.popupSeparatorColor", "#B5B6B7");
		tStart = 0;
		tEnd = 0;
		tMin = 0;
		tMax = 0;
		bHasMin = 0;
		bHasMax = 0;
		fA = 0.0f;
		fB = 0.0f;
		if ( pDateNode != NULL && pDateNode->pWidget != NULL ) {
			(void)xuiDatePickerGetRangeValue(pDateNode->pWidget, &tStart, &tEnd);
			(void)xuiDatePickerGetLimitRange(pDateNode->pWidget, &bHasMin, &tMin, &bHasMax, &tMax);
			(void)xuiDatePickerGetPopupSize(pDateNode->pWidget, &fA, &fB);
			(void)xuiDatePickerGetColors(pDateNode->pWidget, &iTextColor, &iDisabledTextColor, &iBackgroundColor, &iHoverBackgroundColor, &iOpenBackgroundColor, &iDisabledBackgroundColor);
			(void)xuiDatePickerGetBorderColors(pDateNode->pWidget, &iBorderColor, &iHoverBorderColor, &iFocusBorderColor);
			(void)xuiDatePickerGetArrowColors(pDateNode->pWidget, &iArrowColor, &iDisabledArrowColor);
			(void)xuiDatePickerGetPopupColors(pDateNode->pWidget, &iPopupPanelColor, &iPopupBorderColor, &iPopupShadowColor, &iPopupTextColor, &iPopupMutedTextColor, &iPopupAccentColor, &iPopupFieldColor, &iPopupFieldBorderColor, &iPopupSelectedTextColor, &iPopupDisabledDayColor, &iPopupSeparatorColor);
		}
		if ( (pDateNode == NULL) || (pDateNode->pWidget == NULL) ||
		     (xuiDatePickerGetMode(pDateNode->pWidget) != XUI_DATE_PICKER_MODE_DATE_RANGE) ||
		     (xuiDatePickerHasRangeValue(pDateNode->pWidget) == 0) ||
		     (xuiDatePickerIsOpen(pDateNode->pWidget) == 0) ||
		     ((tSingle - tStart) != (2 * XRT_TIME_DAY)) ||
		     ((tEnd - tStart) != (3 * XRT_TIME_DAY)) ||
		     (bHasMin == 0) || (bHasMax == 0) ||
		     ((tStart - tMin) != XRT_TIME_DAY) ||
		     ((tMax - tEnd) != (6 * XRT_TIME_DAY)) ||
		     (xuiDatePickerGetDefaultRangeSpan(pDateNode->pWidget) != (2 * XRT_TIME_DAY)) ||
		     (xuiDatePickerGetNullable(pDateNode->pWidget) != 0) ||
		     (xuiDatePickerGetShowSecond(pDateNode->pWidget) == 0) ||
		     (xuiDatePickerGetFirstDayOfWeek(pDateNode->pWidget) != 0) ||
		     (strcmp(xuiDatePickerGetFormat(pDateNode->pWidget), "yyyy/mm/dd") != 0) ||
		     (strcmp(xuiDatePickerGetRangeSeparator(pDateNode->pWidget), " to ") != 0) ||
		     (fA != 640.0f) || (fB != 340.0f) ||
		     (xuiDatePickerGetPopupPlacement(pDateNode->pWidget) != XUI_DATE_PICKER_POPUP_TOP) ) {
			printf("xui_uidesign exercise-date-picker-properties-failed id=%d\n", iDatePicker);
			return XUI_ERROR;
		}
		fA = 0.0f;
		(void)xuiDatePickerGetMetrics(pDateNode->pWidget, &fA);
		if ( fA != 2.0f ) {
			printf("xui_uidesign exercise-date-picker-metrics-failed id=%d\n", iDatePicker);
			return XUI_ERROR;
		}
		if ( (iTextColor != XUI_COLOR_RGBA(20, 21, 22, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(36, 37, 38, 255)) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(52, 53, 54, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(68, 69, 70, 255)) ||
		     (iOpenBackgroundColor != XUI_COLOR_RGBA(84, 85, 86, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(100, 101, 102, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(116, 117, 118, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(132, 133, 134, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(148, 149, 150, 255)) ||
		     (iArrowColor != XUI_COLOR_RGBA(164, 165, 166, 255)) ||
		     (iDisabledArrowColor != XUI_COLOR_RGBA(180, 181, 182, 255)) ||
		     (iPopupPanelColor != XUI_COLOR_RGBA(21, 22, 23, 255)) ||
		     (iPopupBorderColor != XUI_COLOR_RGBA(37, 38, 39, 255)) ||
		     (iPopupShadowColor != XUI_COLOR_RGBA(53, 54, 55, 64)) ||
		     (iPopupTextColor != XUI_COLOR_RGBA(69, 70, 71, 255)) ||
		     (iPopupMutedTextColor != XUI_COLOR_RGBA(85, 86, 87, 255)) ||
		     (iPopupAccentColor != XUI_COLOR_RGBA(101, 102, 103, 255)) ||
		     (iPopupFieldColor != XUI_COLOR_RGBA(117, 118, 119, 255)) ||
		     (iPopupFieldBorderColor != XUI_COLOR_RGBA(133, 134, 135, 255)) ||
		     (iPopupSelectedTextColor != XUI_COLOR_RGBA(149, 150, 151, 255)) ||
		     (iPopupDisabledDayColor != XUI_COLOR_RGBA(165, 166, 167, 255)) ||
		     (iPopupSeparatorColor != XUI_COLOR_RGBA(181, 182, 183, 255)) ) {
			printf("xui_uidesign exercise-date-picker-colors-failed id=%d\n", iDatePicker);
			return XUI_ERROR;
		}
	}
	if ( iStepBar != 0 ) {
		ui_design_node_t* pStepNode = uiDesignModelGetNode(&pApp->tModel, iStepBar);
		uint32_t iDoneColor = 0u;
		uint32_t iActiveColor = 0u;
		uint32_t iPendingColor = 0u;
		uint32_t iLineColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iActiveTextColor = 0u;
		uint32_t iPendingTextColor = 0u;
		float fBarHeight = 0.0f;
		float fDotRadius = 0.0f;
		float fLineWidth = 0.0f;
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "data.steps", "Open\nEdit\nShip\nReview");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "data.current", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "behavior.style", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "metrics.barHeight", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "metrics.dotRadius", "10");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "metrics.lineWidth", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.doneColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.activeColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.pendingColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.lineColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.backgroundColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.textColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.activeTextColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iStepBar, "appearance.pendingTextColor", "#808182");
		if ( pStepNode != NULL && pStepNode->pWidget != NULL ) {
			(void)xuiStepBarGetMetrics(pStepNode->pWidget, &fBarHeight, &fDotRadius, &fLineWidth);
			(void)xuiStepBarGetColors(pStepNode->pWidget, &iDoneColor, &iActiveColor, &iPendingColor, &iLineColor);
			(void)xuiStepBarGetTextColors(pStepNode->pWidget, &iTextColor, &iActiveTextColor, &iPendingTextColor);
		}
		if ( (pStepNode == NULL) || (pStepNode->pWidget == NULL) ||
		     (xuiStepBarGetStepCount(pStepNode->pWidget) != 4) ||
		     (xuiStepBarGetTitle(pStepNode->pWidget, 2) == NULL) ||
		     (strcmp(xuiStepBarGetTitle(pStepNode->pWidget, 2), "Ship") != 0) ||
		     (xuiStepBarGetCurrent(pStepNode->pWidget) != 2) ||
		     (xuiStepBarGetStyle(pStepNode->pWidget) != XUI_STEP_BAR_STYLE_ARROW) ||
		     (fBarHeight != 6.0f) || (fDotRadius != 10.0f) || (fLineWidth != 3.0f) ||
		     (iDoneColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iPendingColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iLineColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (xuiStepBarGetBackgroundColor(pStepNode->pWidget) != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iActiveTextColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iPendingTextColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ) {
			printf("xui_uidesign exercise-stepbar-properties-failed id=%d metrics=%.1f/%.1f/%.1f\n",
				iStepBar,
				fBarHeight,
				fDotRadius,
				fLineWidth);
			return XUI_ERROR;
		}
	}
	if ( iPage != 0 ) {
		ui_design_node_t* pPageNode = uiDesignModelGetNode(&pApp->tModel, iPage);
		const char* sFirst = NULL;
		const char* sLast = NULL;
		const char* sPrev = NULL;
		const char* sNext = NULL;
		uint32_t iBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iTextColor = 0u;
		uint32_t iHoverColor = 0u;
		uint32_t iActiveColor = 0u;
		uint32_t iCurrentColor = 0u;
		uint32_t iCurrentTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "behavior.useTotal", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "data.pageCount", "9");
		if ( (pPageNode == NULL) || (pPageNode->pWidget == NULL) || (xuiPageGetPageCount(pPageNode->pWidget) != 9) ) {
			printf("xui_uidesign exercise-page-count-failed id=%d\n", iPage);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "behavior.useTotal", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "data.totalCount", "95");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "data.pageSize", "20");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "data.current", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "data.windowSize", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "text.first", "First");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "text.last", "Last");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "text.prev", "Prev");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "text.next", "Next");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "metrics.itemHeight", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "metrics.pageWidth", "33");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "metrics.textWidth", "45");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "metrics.navWidth", "41");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "metrics.ellipsisWidth", "21");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.borderColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.textColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.hoverColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.activeColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.currentColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.currentTextColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.disabledTextColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iPage, "appearance.focusColor", "#818283");
		fA = fB = 0.0f;
		if ( pPageNode != NULL && pPageNode->pWidget != NULL ) {
			float fC = 0.0f;
			float fD = 0.0f;
			float fE = 0.0f;
			(void)xuiPageGetText(pPageNode->pWidget, &sFirst, &sLast, &sPrev, &sNext);
			(void)xuiPageGetMetrics(pPageNode->pWidget, &fA, &fB, &fC, &fD, &fE);
			(void)xuiPageGetColors(pPageNode->pWidget,
				&iBackgroundColor,
				&iBorderColor,
				&iTextColor,
				&iHoverColor,
				&iActiveColor,
				&iCurrentColor,
				&iCurrentTextColor,
				&iDisabledTextColor);
			if ( (fC != 45.0f) || (fD != 41.0f) || (fE != 21.0f) ) pPageNode = NULL;
		}
		if ( (pPageNode == NULL) || (pPageNode->pWidget == NULL) ||
		     (xuiPageGetPageCount(pPageNode->pWidget) != 5) ||
		     (xuiPageGetCurrent(pPageNode->pWidget) != 4) ||
		     (xuiPageGetWindowSize(pPageNode->pWidget) != 7) ||
		     (sFirst == NULL) || (strcmp(sFirst, "First") != 0) ||
		     (sLast == NULL) || (strcmp(sLast, "Last") != 0) ||
		     (sPrev == NULL) || (strcmp(sPrev, "Prev") != 0) ||
		     (sNext == NULL) || (strcmp(sNext, "Next") != 0) ||
		     (fA != 31.0f) || (fB != 33.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iCurrentColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iCurrentTextColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (xuiPageGetFocusColor(pPageNode->pWidget) != XUI_COLOR_RGBA(129, 130, 131, 255)) ) {
			printf("xui_uidesign exercise-page-properties-failed id=%d\n", iPage);
			return XUI_ERROR;
		}
	}
	if ( iCarousel != 0 ) {
		ui_design_node_t* pCarouselNode = uiDesignModelGetNode(&pApp->tModel, iCarousel);
		ui_design_node_t* pChildNode = (iCarouselChild != 0) ? uiDesignModelGetNode(&pApp->tModel, iCarouselChild) : NULL;
		xui_widget pPageWidget = NULL;
		float fArrowSize = 0.0f;
		float fIndicatorSize = 0.0f;
		float fIndicatorGap = 0.0f;
		float fIndicatorBottom = 0.0f;
		float fAutoInterval = 0.0f;
		uint32_t iBackgroundColor = 0u;
		uint32_t iArrowColor = 0u;
		uint32_t iArrowHoverColor = 0u;
		uint32_t iArrowTextColor = 0u;
		uint32_t iIndicatorColor = 0u;
		uint32_t iIndicatorActiveColor = 0u;
		uint32_t iIndicatorHoverColor = 0u;
		int bAutoPlay = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "data.pageCount", "4");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "data.current", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "behavior.loop", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "behavior.autoPlay", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "behavior.autoInterval", "1.25");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "behavior.showIndicators", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "behavior.arrowsOnHover", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "metrics.arrowSize", "37");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "metrics.indicatorSize", "11");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "metrics.indicatorGap", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "metrics.indicatorBottom", "17");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.arrowColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.arrowHoverColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.arrowTextColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.indicatorColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.indicatorActiveColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.indicatorHoverColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iCarousel, "appearance.focusColor", "#717273");
		if ( pCarouselNode != NULL && pCarouselNode->pWidget != NULL ) {
			(void)xuiCarouselGetAutoPlay(pCarouselNode->pWidget, &bAutoPlay, &fAutoInterval);
			(void)xuiCarouselGetMetrics(pCarouselNode->pWidget, &fArrowSize, &fIndicatorSize, &fIndicatorGap, &fIndicatorBottom);
			(void)xuiCarouselGetColors(pCarouselNode->pWidget,
				&iBackgroundColor,
				&iArrowColor,
				&iArrowHoverColor,
				&iArrowTextColor,
				&iIndicatorColor,
				&iIndicatorActiveColor,
				&iIndicatorHoverColor);
			pPageWidget = xuiCarouselGetPageWidget(pCarouselNode->pWidget, 0);
		}
		if ( (pCarouselNode == NULL) || (pCarouselNode->pWidget == NULL) ||
		     (xuiCarouselGetPageCount(pCarouselNode->pWidget) != 4) ||
		     (xuiCarouselGetCurrent(pCarouselNode->pWidget) != 2) ||
		     (xuiCarouselGetLoop(pCarouselNode->pWidget) != 0) ||
		     (bAutoPlay == 0) ||
		     (fAutoInterval != 1.25f) ||
		     (xuiCarouselGetIndicatorsVisible(pCarouselNode->pWidget) != 0) ||
		     (xuiCarouselGetArrowsOnHover(pCarouselNode->pWidget) != 0) ||
		     (fArrowSize != 37.0f) ||
		     (fIndicatorSize != 11.0f) ||
		     (fIndicatorGap != 13.0f) ||
		     (fIndicatorBottom != 17.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iArrowColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iArrowHoverColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iArrowTextColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iIndicatorColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iIndicatorActiveColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iIndicatorHoverColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (xuiCarouselGetFocusColor(pCarouselNode->pWidget) != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (pChildNode == NULL) ||
		     (pChildNode->pWidget == NULL) ||
		     (pPageWidget == NULL) ||
		     (xuiWidgetGetParent(pChildNode->pWidget) != pPageWidget) ) {
			printf("xui_uidesign exercise-carousel-properties-failed id=%d pages=%d current=%d auto=%d/%.2f metrics=%.1f/%.1f/%.1f/%.1f\n",
				iCarousel,
				(pCarouselNode != NULL && pCarouselNode->pWidget != NULL) ? xuiCarouselGetPageCount(pCarouselNode->pWidget) : -1,
				(pCarouselNode != NULL && pCarouselNode->pWidget != NULL) ? xuiCarouselGetCurrent(pCarouselNode->pWidget) : -1,
				bAutoPlay,
				fAutoInterval,
				fArrowSize,
				fIndicatorSize,
				fIndicatorGap,
				fIndicatorBottom);
			return XUI_ERROR;
		}
	}
	if ( iBreadcrumb != 0 ) {
		ui_design_node_t* pBreadcrumbNode = uiDesignModelGetNode(&pApp->tModel, iBreadcrumb);
		xui_surface pSeparatorIcon = NULL;
		xui_rect_t tSeparatorIconSrc;
		uint32_t iNormalTextColor = 0u;
		uint32_t iHoverTextColor = 0u;
		uint32_t iActiveTextColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		float fGap = 0.0f;
		float fPaddingX = 0.0f;
		float fPaddingY = 0.0f;
		memset(&tSeparatorIconSrc, 0, sizeof(tSeparatorIconSrc));
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "data.items", "Root|true|11\nChild|false|22");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "text.separator", ">");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "data.separatorIconSource", __uiDesignExerciseResourcePath("xui_menu_icon.png"));
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "separatorIcon.x", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "separatorIcon.y", "7");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "separatorIcon.w", "14");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "separatorIcon.h", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.textColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.hoverTextColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.activeTextColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.disabledTextColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.separatorColor", "#112233");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "appearance.backgroundColor", "#445566");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "metrics.separatorIconSize", "15");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "metrics.gap", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "metrics.paddingX", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iBreadcrumb, "metrics.paddingY", "7");
		if ( pBreadcrumbNode != NULL && pBreadcrumbNode->pWidget != NULL ) {
			(void)xuiBreadcrumbGetTextColors(pBreadcrumbNode->pWidget, &iNormalTextColor, &iHoverTextColor, &iActiveTextColor, &iDisabledTextColor);
			(void)xuiBreadcrumbGetMetrics(pBreadcrumbNode->pWidget, &fGap, &fPaddingX, &fPaddingY);
			pSeparatorIcon = xuiBreadcrumbGetSeparatorIcon(pBreadcrumbNode->pWidget);
			tSeparatorIconSrc = xuiBreadcrumbGetSeparatorIconSource(pBreadcrumbNode->pWidget);
		}
		if ( (pBreadcrumbNode == NULL) || (pBreadcrumbNode->pWidget == NULL) ||
		     (xuiBreadcrumbGetItemCount(pBreadcrumbNode->pWidget) != 2) ||
		     (strcmp(xuiBreadcrumbGetItemText(pBreadcrumbNode->pWidget, 0), "Root") != 0) ||
		     (xuiBreadcrumbGetItemClickable(pBreadcrumbNode->pWidget, 1) != 0) ||
		     (xuiBreadcrumbGetItemValue(pBreadcrumbNode->pWidget, 1) != 22) ||
		     (strcmp(xuiBreadcrumbGetSeparator(pBreadcrumbNode->pWidget), ">") != 0) ||
		     (iNormalTextColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iHoverTextColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iActiveTextColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (xuiBreadcrumbGetSeparatorColor(pBreadcrumbNode->pWidget) != XUI_COLOR_RGBA(17, 34, 51, 255)) ||
		     (xuiBreadcrumbGetBackgroundColor(pBreadcrumbNode->pWidget) != XUI_COLOR_RGBA(68, 85, 102, 255)) ||
		     (xuiBreadcrumbGetSeparatorIconSize(pBreadcrumbNode->pWidget) != 15.0f) ||
		     (pSeparatorIcon == NULL) ||
		     (tSeparatorIconSrc.fX != 6.0f) || (tSeparatorIconSrc.fY != 7.0f) || (tSeparatorIconSrc.fW != 14.0f) || (tSeparatorIconSrc.fH != 15.0f) ||
		     (fGap != 9.0f) || (fPaddingX != 13.0f) || (fPaddingY != 7.0f) ) {
			printf("xui_uidesign exercise-breadcrumb-properties-failed id=%d metrics=%.1f/%.1f/%.1f\n",
				iBreadcrumb,
				fGap,
				fPaddingX,
				fPaddingY);
			return XUI_ERROR;
		}
	}
	if ( iCascader != 0 ) {
		ui_design_node_t* pCascaderNode = uiDesignModelGetNode(&pApp->tModel, iCascader);
		uint32_t iTextColor = 0u;
		uint32_t iPlaceholderColor = 0u;
		uint32_t iDisabledTextColor = 0u;
		uint32_t iBackgroundColor = 0u;
		uint32_t iHoverBackgroundColor = 0u;
		uint32_t iOpenBackgroundColor = 0u;
		uint32_t iDisabledBackgroundColor = 0u;
		uint32_t iBorderColor = 0u;
		uint32_t iHoverBorderColor = 0u;
		uint32_t iFocusBorderColor = 0u;
		uint32_t iArrowColor = 0u;
		uint32_t iDisabledArrowColor = 0u;
		uint32_t iButtonColor = 0u;
		uint32_t iButtonHoverColor = 0u;
		uint32_t iButtonOpenColor = 0u;
		uint32_t iPopupPanelColor = 0u;
		uint32_t iPopupBorderColor = 0u;
		uint32_t iPopupShadowColor = 0u;
		uint32_t iPopupTextColor = 0u;
		uint32_t iPopupMutedTextColor = 0u;
		uint32_t iPopupHoverColor = 0u;
		uint32_t iPopupActiveColor = 0u;
		uint32_t iPopupSelectedColor = 0u;
		uint32_t iPopupActiveTextColor = 0u;
		uint32_t iPopupDisabledTextColor = 0u;
		uint32_t iPopupSeparatorColor = 0u;
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "data.items", "Root/A/Leaf\nRoot/B/Other");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "data.selectedPath", "Root/B/Other");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "text.placeholder", "Choose path");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "text.separator", " > ");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.showAllLevels", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.clearable", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.selectAnyLevel", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.expandTrigger", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.popupPlacement", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "metrics.itemHeight", "27");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "metrics.columnWidth", "180");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "metrics.popupHeight", "120");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "metrics.popupMaxHeight", "280");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "metrics.borderWidth", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "behavior.open", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.textColor", "#101112");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.placeholderColor", "#202122");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.disabledTextColor", "#303132");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.backgroundColor", "#404142");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.hoverBackgroundColor", "#505152");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.openBackgroundColor", "#606162");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.disabledBackgroundColor", "#707172");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.borderColor", "#808182");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.hoverBorderColor", "#909192");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.focusBorderColor", "#A0A1A2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.arrowColor", "#B0B1B2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.disabledArrowColor", "#C0C1C2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.buttonColor", "#D0D1D2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.buttonHoverColor", "#E0E1E2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.buttonOpenColor", "#F0F1F2");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupPanelColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupBorderColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupShadowColor", "#31323340");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupTextColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupMutedTextColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupHoverColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupActiveColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupSelectedColor", "#818283");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupActiveTextColor", "#919293");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupDisabledTextColor", "#A1A2A3");
		(void)uiDesignAppSetNodeProperty(pApp, iCascader, "appearance.popupSeparatorColor", "#B1B2B3");
		fA = fB = 0.0f;
		if ( pCascaderNode != NULL && pCascaderNode->pWidget != NULL ) {
			float fC = 0.0f;
			(void)xuiCascaderGetPopupSize(pCascaderNode->pWidget, &fA, &fB, &fC);
			(void)xuiCascaderGetColors(pCascaderNode->pWidget, &iTextColor, &iPlaceholderColor, &iDisabledTextColor, &iBackgroundColor, &iHoverBackgroundColor, &iOpenBackgroundColor, &iDisabledBackgroundColor);
			(void)xuiCascaderGetBorderColors(pCascaderNode->pWidget, &iBorderColor, &iHoverBorderColor, &iFocusBorderColor);
			(void)xuiCascaderGetArrowColors(pCascaderNode->pWidget, &iArrowColor, &iDisabledArrowColor);
			(void)xuiCascaderGetButtonColors(pCascaderNode->pWidget, &iButtonColor, &iButtonHoverColor, &iButtonOpenColor);
			(void)xuiCascaderGetPopupColors(pCascaderNode->pWidget, &iPopupPanelColor, &iPopupBorderColor, &iPopupShadowColor, &iPopupTextColor, &iPopupMutedTextColor, &iPopupHoverColor, &iPopupActiveColor, &iPopupSelectedColor, &iPopupActiveTextColor, &iPopupDisabledTextColor, &iPopupSeparatorColor);
			if ( fC != 280.0f ) pCascaderNode = NULL;
		}
		if ( (pCascaderNode == NULL) || (pCascaderNode->pWidget == NULL) ||
		     (xuiCascaderGetItemCount(pCascaderNode->pWidget) != 5) ||
		     (xuiCascaderGetSelectedDepth(pCascaderNode->pWidget) != 3) ||
		     (xuiCascaderIsOpen(pCascaderNode->pWidget) == 0) ||
		     (strcmp(xuiCascaderGetSelectedText(pCascaderNode->pWidget), "Root > B > Other") != 0) ||
		     (strcmp(xuiCascaderGetPlaceholder(pCascaderNode->pWidget), "Choose path") != 0) ||
		     (strcmp(xuiCascaderGetSeparator(pCascaderNode->pWidget), " > ") != 0) ||
		     (xuiCascaderGetShowAllLevels(pCascaderNode->pWidget) == 0) ||
		     (xuiCascaderGetClearable(pCascaderNode->pWidget) != 0) ||
		     (xuiCascaderGetSelectAnyLevel(pCascaderNode->pWidget) == 0) ||
		     (xuiCascaderGetExpandTrigger(pCascaderNode->pWidget) != XUI_CASCADER_EXPAND_HOVER) ||
		     (xuiCascaderGetPopupPlacement(pCascaderNode->pWidget) != XUI_CASCADER_POPUP_TOP) ||
		     (fA != 180.0f) || (fB != 120.0f) ) {
			printf("xui_uidesign exercise-cascader-properties-failed id=%d\n", iCascader);
			return XUI_ERROR;
		}
		fA = fB = 0.0f;
		(void)xuiCascaderGetMetrics(pCascaderNode->pWidget, &fA, &fB);
		if ( (fA != 27.0f) || (fB != 2.0f) ) {
			printf("xui_uidesign exercise-cascader-metrics-failed id=%d\n", iCascader);
			return XUI_ERROR;
		}
		if ( (iTextColor != XUI_COLOR_RGBA(16, 17, 18, 255)) ||
		     (iPlaceholderColor != XUI_COLOR_RGBA(32, 33, 34, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(48, 49, 50, 255)) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(64, 65, 66, 255)) ||
		     (iHoverBackgroundColor != XUI_COLOR_RGBA(80, 81, 82, 255)) ||
		     (iOpenBackgroundColor != XUI_COLOR_RGBA(96, 97, 98, 255)) ||
		     (iDisabledBackgroundColor != XUI_COLOR_RGBA(112, 113, 114, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(128, 129, 130, 255)) ||
		     (iHoverBorderColor != XUI_COLOR_RGBA(144, 145, 146, 255)) ||
		     (iFocusBorderColor != XUI_COLOR_RGBA(160, 161, 162, 255)) ||
		     (iArrowColor != XUI_COLOR_RGBA(176, 177, 178, 255)) ||
		     (iDisabledArrowColor != XUI_COLOR_RGBA(192, 193, 194, 255)) ||
		     (iButtonColor != XUI_COLOR_RGBA(208, 209, 210, 255)) ||
		     (iButtonHoverColor != XUI_COLOR_RGBA(224, 225, 226, 255)) ||
		     (iButtonOpenColor != XUI_COLOR_RGBA(240, 241, 242, 255)) ||
		     (iPopupPanelColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iPopupBorderColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iPopupShadowColor != XUI_COLOR_RGBA(49, 50, 51, 64)) ||
		     (iPopupTextColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iPopupMutedTextColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iPopupHoverColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iPopupActiveColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iPopupSelectedColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ||
		     (iPopupActiveTextColor != XUI_COLOR_RGBA(145, 146, 147, 255)) ||
		     (iPopupDisabledTextColor != XUI_COLOR_RGBA(161, 162, 163, 255)) ||
		     (iPopupSeparatorColor != XUI_COLOR_RGBA(177, 178, 179, 255)) ) {
			printf("xui_uidesign exercise-cascader-colors-failed id=%d\n", iCascader);
			return XUI_ERROR;
		}
	}
	if ( iTabs != 0 ) {
		ui_design_node_t* pTabsNode = uiDesignModelGetNode(&pApp->tModel, iTabs);
		uint32_t iBorder = 0;
		uint32_t iClient = 0;
		uint32_t iBackgroundColor = 0;
		uint32_t iTabColor = 0;
		uint32_t iHoverColor = 0;
		uint32_t iActiveColor = 0;
		uint32_t iFocusColor = 0;
		uint32_t iDisabledColor = 0;
		uint32_t iTextColor = 0;
		uint32_t iActiveTextColor = 0;
		float fScrollBeforeLayout = 0.0f;
		xui_rect_t tCloseRect = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tTabsChildModelRect = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tTabsChildWorldRect = {0.0f, 0.0f, 0.0f, 0.0f};
		xui_rect_t tArtboardWorldRect = {0.0f, 0.0f, 0.0f, 0.0f};
		int bTabsChildRectMatched = (iTabsChild == 0) ? 1 : 0;
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "data.items", "One|true|false|||||\nTwo|true|true|||||\nThree|false|false|||||\nFour|true|false|||||\nFive|true|false|||||\nSix|true|false|||||");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "data.selected", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "behavior.placement", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "behavior.scrollable", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "behavior.closeButtons", "true");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "metrics.tabWidth", "110");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "metrics.tabHeight", "34");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.tabColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.hoverColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.activeColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.focusColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.disabledColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.textColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.activeTextColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.borderColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "appearance.clientColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iTabs, "value.scrollX", "24");
		fA = fB = 0.0f;
		if ( pTabsNode != NULL && pTabsNode->pWidget != NULL ) {
			fScrollBeforeLayout = xuiTabsGetScroll(pTabsNode->pWidget);
			(void)xuiWidgetArrange(pTabsNode->pWidget, pTabsNode->tRect);
			if ( iTabsChild != 0 ) {
				ui_design_node_t* pTabsChildNode = uiDesignModelGetNode(&pApp->tModel, iTabsChild);
				if ( pTabsChildNode != NULL && pTabsChildNode->pWidget != NULL &&
				     uiDesignModelGetAbsoluteRect(&pApp->tModel, iTabsChild, &tTabsChildModelRect) == XUI_OK ) {
					tTabsChildWorldRect = xuiWidgetGetWorldRect(pTabsChildNode->pWidget);
					if ( pApp->pArtboard != NULL ) {
						tArtboardWorldRect = xuiWidgetGetWorldRect(pApp->pArtboard);
						tTabsChildWorldRect.fX -= tArtboardWorldRect.fX;
						tTabsChildWorldRect.fY -= tArtboardWorldRect.fY;
					}
					bTabsChildRectMatched = __uiDesignAppRectAlmostEqual(tTabsChildModelRect, tTabsChildWorldRect, 0.5f);
				}
			}
			(void)xuiTabsGetTabSize(pTabsNode->pWidget, &fA, &fB);
			(void)xuiTabsGetColors(pTabsNode->pWidget, &iBackgroundColor, &iTabColor, &iHoverColor, &iActiveColor, &iFocusColor, &iDisabledColor, &iTextColor, &iActiveTextColor);
			(void)xuiTabsGetFrameColors(pTabsNode->pWidget, &iBorder, &iClient);
			tCloseRect = xuiTabsGetCloseRect(pTabsNode->pWidget, 1);
		}
		if ( (pTabsNode == NULL) || (pTabsNode->pWidget == NULL) ||
		     (xuiTabsGetItemCount(pTabsNode->pWidget) != 6) ||
		     (strcmp(xuiTabsGetItemText(pTabsNode->pWidget, 1), "Two") != 0) ||
		     (xuiTabsGetSelected(pTabsNode->pWidget) != 1) ||
		     (xuiTabsGetPlacement(pTabsNode->pWidget) != XUI_TABS_PLACEMENT_BOTTOM) ||
		     (xuiTabsIsScrollable(pTabsNode->pWidget) != 1) ||
		     (fScrollBeforeLayout != 24.0f) ||
		     (tCloseRect.fW <= 0.0f) || (tCloseRect.fH <= 0.0f) ||
		     (fA != 110.0f) || (fB != 34.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iTabColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iActiveColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iFocusColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iDisabledColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iActiveTextColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iBorder != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (iClient != XUI_COLOR_RGBA(32, 48, 64, 255)) ) {
			printf("xui_uidesign exercise-tabs-properties-failed id=%d scroll=%.1f close=%.1f/%.1f colors=%08x/%08x/%08x/%08x/%08x/%08x/%08x/%08x frame=%08x/%08x\n",
				iTabs,
				fScrollBeforeLayout,
				tCloseRect.fW,
				tCloseRect.fH,
				iBackgroundColor,
				iTabColor,
				iHoverColor,
				iActiveColor,
				iFocusColor,
				iDisabledColor,
				iTextColor,
				iActiveTextColor,
				iBorder,
				iClient);
			return XUI_ERROR;
		}
		if ( !bTabsChildRectMatched ) {
			printf("xui_uidesign exercise-tabs-child-rect-failed child=%d model=%.1f,%.1f,%.1f,%.1f widget=%.1f,%.1f,%.1f,%.1f\n",
				iTabsChild,
				tTabsChildModelRect.fX,
				tTabsChildModelRect.fY,
				tTabsChildModelRect.fW,
				tTabsChildModelRect.fH,
				tTabsChildWorldRect.fX,
				tTabsChildWorldRect.fY,
				tTabsChildWorldRect.fW,
				tTabsChildWorldRect.fH);
			return XUI_ERROR;
		}
	}
	if ( iAccordion != 0 ) {
		ui_design_node_t* pAccordionNode = uiDesignModelGetNode(&pApp->tModel, iAccordion);
		uint32_t iBackgroundColor = 0;
		uint32_t iHeaderColor = 0;
		uint32_t iHoverColor = 0;
		uint32_t iExpandedColor = 0;
		uint32_t iContentColor = 0;
		uint32_t iBorderColor = 0;
		uint32_t iTextColor = 0;
		uint32_t iActiveTextColor = 0;
		uint32_t iDisabledTextColor = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "data.sections", "Main|true|false|10\nDisabled|false|true|20\nExtra|true|false|30");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "behavior.mode", "0");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "metrics.headerHeight", "31");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "metrics.spacing", "6");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "metrics.contentPadding", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.backgroundColor", "#010203");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.headerColor", "#111213");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.hoverColor", "#212223");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.expandedColor", "#313233");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.contentColor", "#414243");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.borderColor", "#515253");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.textColor", "#616263");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.activeTextColor", "#717273");
		(void)uiDesignAppSetNodeProperty(pApp, iAccordion, "appearance.disabledTextColor", "#818283");
		fA = fB = 0.0f;
		if ( pAccordionNode != NULL && pAccordionNode->pWidget != NULL ) {
			float fC = 0.0f;
			(void)xuiAccordionGetMetrics(pAccordionNode->pWidget, &fA, &fB, &fC);
			(void)xuiAccordionGetColors(pAccordionNode->pWidget, &iBackgroundColor, &iHeaderColor, &iHoverColor, &iExpandedColor, &iContentColor, &iBorderColor, &iTextColor, &iActiveTextColor, &iDisabledTextColor);
			if ( fC != 9.0f ) pAccordionNode = NULL;
		}
		if ( (pAccordionNode == NULL) || (pAccordionNode->pWidget == NULL) ||
		     (xuiAccordionGetSectionCount(pAccordionNode->pWidget) != 3) ||
		     (strcmp(xuiAccordionGetSectionTitle(pAccordionNode->pWidget, 1), "Disabled") != 0) ||
		     (xuiAccordionGetSectionId(pAccordionNode->pWidget, 2) != 30) ||
		     (xuiAccordionIsExpanded(pAccordionNode->pWidget, 0) == 0) ||
		     (xuiAccordionIsExpanded(pAccordionNode->pWidget, 2) == 0) ||
		     (xuiAccordionIsSectionEnabled(pAccordionNode->pWidget, 1) != 0) ||
		     (xuiAccordionGetMode(pAccordionNode->pWidget) != XUI_ACCORDION_MODE_MULTIPLE) ||
		     (fA != 31.0f) || (fB != 6.0f) ||
		     (iBackgroundColor != XUI_COLOR_RGBA(1, 2, 3, 255)) ||
		     (iHeaderColor != XUI_COLOR_RGBA(17, 18, 19, 255)) ||
		     (iHoverColor != XUI_COLOR_RGBA(33, 34, 35, 255)) ||
		     (iExpandedColor != XUI_COLOR_RGBA(49, 50, 51, 255)) ||
		     (iContentColor != XUI_COLOR_RGBA(65, 66, 67, 255)) ||
		     (iBorderColor != XUI_COLOR_RGBA(81, 82, 83, 255)) ||
		     (iTextColor != XUI_COLOR_RGBA(97, 98, 99, 255)) ||
		     (iActiveTextColor != XUI_COLOR_RGBA(113, 114, 115, 255)) ||
		     (iDisabledTextColor != XUI_COLOR_RGBA(129, 130, 131, 255)) ) {
			printf("xui_uidesign exercise-accordion-properties-failed id=%d colors=%08x/%08x/%08x/%08x/%08x/%08x/%08x/%08x/%08x metrics=%.1f/%.1f\n",
				iAccordion,
				iBackgroundColor,
				iHeaderColor,
				iHoverColor,
				iExpandedColor,
				iContentColor,
				iBorderColor,
				iTextColor,
				iActiveTextColor,
				iDisabledTextColor,
				fA,
				fB);
			return XUI_ERROR;
		}
	}
	if ( iSplit != 0 ) {
		ui_design_node_t* pSplitNode = uiDesignModelGetNode(&pApp->tModel, iSplit);
		xui_rect_t tSplitChildRect = {0.0f, 0.0f, 0.0f, 0.0f};
		uint32_t iDivider = 0;
		uint32_t iHover = 0;
		uint32_t iActive = 0;
		uint32_t iShadow = 0;
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "behavior.orientation", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.paneCount", "2");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.panes", "First||1|0|40|0\nSecond||1|0|40|0");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "behavior.pane0Mode", "1");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "behavior.pane1Mode", "0");
		if ( (pSplitNode == NULL) || (pSplitNode->pWidget == NULL) ||
		     (xuiSplitLayoutGetPaneCount(pSplitNode->pWidget) != 2) ||
		     (xuiSplitLayoutGetPaneMode(pSplitNode->pWidget, 0) != XUI_SPLIT_PANE_FIXED) ||
		     (xuiSplitLayoutGetPaneMode(pSplitNode->pWidget, 1) != XUI_SPLIT_PANE_GROW) ) {
			printf("xui_uidesign exercise-split-layout-pane-mode-properties-failed id=%d\n", iSplit);
			return XUI_ERROR;
		}
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.paneCount", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.panes", "Left|fixed|1|120|60|220\nMiddle|grow|2|0|80|0\nRight|grow|1|0|70|0");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "metrics.dividerSize", "9");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "metrics.dividerVisualSize", "3");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "metrics.dividerHitSize", "13");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "behavior.shadowDrag", "false");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "appearance.dividerColor", "#102030");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "appearance.hoverColor", "#203040");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "appearance.activeColor", "#304050");
		(void)uiDesignAppSetNodeProperty(pApp, iSplit, "appearance.shadowColor", "#405060");
		fA = fB = 0.0f;
		if ( pSplitNode != NULL && pSplitNode->pWidget != NULL ) {
			float fC = 0.0f;
			(void)xuiSplitLayoutGetDividerMetrics(pSplitNode->pWidget, &fA, &fB, &fC);
			(void)xuiSplitLayoutGetColors(pSplitNode->pWidget, &iDivider, &iHover, &iActive, &iShadow);
			if ( fC != 13.0f ) pSplitNode = NULL;
		}
		if ( (pSplitNode == NULL) || (pSplitNode->pWidget == NULL) ||
		     (xuiSplitLayoutGetOrientation(pSplitNode->pWidget) != XUI_ORIENTATION_VERTICAL) ||
		     (xuiSplitLayoutGetPaneCount(pSplitNode->pWidget) != 3) ||
		     (xuiSplitLayoutGetPaneMode(pSplitNode->pWidget, 0) != XUI_SPLIT_PANE_FIXED) ||
		     (xuiSplitLayoutGetPaneFixedSize(pSplitNode->pWidget, 0) != 120.0f) ||
		     (xuiSplitLayoutGetPaneMinSize(pSplitNode->pWidget, 0) != 60.0f) ||
		     (xuiSplitLayoutGetPaneMaxSize(pSplitNode->pWidget, 0) != 220.0f) ||
		     (xuiSplitLayoutGetPaneWeight(pSplitNode->pWidget, 1) != 2.0f) ||
		     (xuiSplitLayoutGetShadowDrag(pSplitNode->pWidget) != 0) ||
		     (fA != 9.0f) || (fB != 3.0f) ||
		     (iDivider != XUI_COLOR_RGBA(16, 32, 48, 255)) ||
		     (iHover != XUI_COLOR_RGBA(32, 48, 64, 255)) ||
		     (iActive != XUI_COLOR_RGBA(48, 64, 80, 255)) ||
		     (iShadow != XUI_COLOR_RGBA(64, 80, 96, 255)) ) {
			printf("xui_uidesign exercise-split-layout-properties-failed id=%d\n", iSplit);
			return XUI_ERROR;
		}
		if ( (iSplitChild != 0) &&
		     (uiDesignModelGetAbsoluteRect(&pApp->tModel, iSplitChild, &tSplitChildRect) != XUI_OK ||
		      tSplitChildRect.fX <= pSplitNode->tRect.fX + 40.0f ||
		      tSplitChildRect.fY >= pSplitNode->tRect.fY + 40.0f) ) {
			printf("xui_uidesign exercise-split-layout-child-axis-failed id=%d child=%d child=%.1f/%.1f split=%.1f/%.1f\n",
				iSplit, iSplitChild, tSplitChildRect.fX, tSplitChildRect.fY, pSplitNode->tRect.fX, pSplitNode->tRect.fY);
			return XUI_ERROR;
		}
	}
	if ( iOverlayPanel != 0 ) {
		(void)uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_LABEL, 1160.0f, 96.0f, &iOverlayDropChild);
	}
	if ( iTablePanel != 0 ) {
		iCreateRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_LABEL, 1240.0f, 300.0f, &iTableDropChild);
		if ( iCreateRet != XUI_OK ) return iCreateRet;
	}
	if ( iDockLayoutPanel != 0 ) {
		iCreateRet = uiDesignAppAddNodeAt(pApp, UI_DESIGN_NODE_LABEL, 1095.0f, 440.0f, &iDockLayoutDropChild);
		if ( iCreateRet != XUI_OK ) return iCreateRet;
	}
	if ( (iPanelLayoutChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iPanelLayoutChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=panel_row child=%d\n", iPanelLayoutChild);
		return XUI_ERROR;
	}
	if ( (iCarouselChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iCarouselChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=carousel child=%d\n", iCarouselChild);
		return XUI_ERROR;
	}
	if ( (iSplitChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iSplitChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=split_layout child=%d\n", iSplitChild);
		return XUI_ERROR;
	}
	if ( (iTabsChild != 0) && !uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iTabsChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=tabs child=%d\n", iTabsChild);
		return XUI_ERROR;
	}
	if ( (iAccordionChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iAccordionChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=accordion child=%d\n", iAccordionChild);
		return XUI_ERROR;
	}
	if ( (iDockPanelChild != 0) && !uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iDockPanelChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=dock_panel child=%d\n", iDockPanelChild);
		return XUI_ERROR;
	}
	if ( (iPopupChild != 0) && !uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iPopupChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=popup child=%d\n", iPopupChild);
		return XUI_ERROR;
	}
	if ( (iRadioGroupRadioChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iRadioGroupRadioChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=radio_group radio_child=%d\n", iRadioGroupRadioChild);
		return XUI_ERROR;
	}
	if ( (iRadioGroupCardChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iRadioGroupCardChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=radio_group card_child=%d\n", iRadioGroupCardChild);
		return XUI_ERROR;
	}
	if ( (iOverlayDropChild != 0) && !uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iOverlayDropChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=overlay child=%d\n", iOverlayDropChild);
		return XUI_ERROR;
	}
	if ( (iOverlayDropChild != 0) &&
	     (uiDesignNodeGetPropertyInt(uiDesignModelGetNode(&pApp->tModel, iOverlayDropChild), "layout.flowMode", XUI_FLOW_BLOCK) != XUI_FLOW_ABSOLUTE) ) {
		printf("xui_uidesign exercise-layout-flow-failed type=overlay child=%d\n", iOverlayDropChild);
		return XUI_ERROR;
	}
	if ( (iTableDropChild != 0) &&
	     (uiDesignNodeGetPropertyInt(uiDesignModelGetNode(&pApp->tModel, iTableDropChild), "layout.tableCellRow", -1) != 1 ||
	      uiDesignNodeGetPropertyInt(uiDesignModelGetNode(&pApp->tModel, iTableDropChild), "layout.tableCellColumn", -1) != 2) ) {
		printf("xui_uidesign exercise-layout-table-drop-failed child=%d\n", iTableDropChild);
		return XUI_ERROR;
	}
	if ( (iTableDropChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iTableDropChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=table child=%d\n", iTableDropChild);
		return XUI_ERROR;
	}
	if ( (iDockLayoutDropChild != 0) &&
	     (uiDesignNodeGetPropertyInt(uiDesignModelGetNode(&pApp->tModel, iDockLayoutDropChild), "layout.dock", 0) != XUI_DOCK_LEFT) ) {
		printf("xui_uidesign exercise-layout-dock-drop-failed child=%d\n", iDockLayoutDropChild);
		return XUI_ERROR;
	}
	if ( (iDockLayoutDropChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iDockLayoutDropChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=dock child=%d\n", iDockLayoutDropChild);
		return XUI_ERROR;
	}
	iCreateRet = uiDesignInspectorExerciseComplexEditors(pApp);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-complex-editor-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	iCreateRet = uiDesignInspectorExercisePropertyCoverage(pApp);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-property-coverage-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	iCreateRet = __uiDesignExerciseContextCommands(pApp, iButton);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-context-command-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	iCreateRet = __uiDesignExerciseCommandSystem(pApp);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-command-system-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	iCreateRet = __uiDesignExerciseResizeLayout(pApp);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-resize-layout-command-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	(void)uiDesignModelSetSelected(&pApp->tModel, iButton);
	(void)uiDesignInspectorRefresh(pApp);
	pApp->bExerciseSeeded = 1;
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignCreateAssets(ui_design_app_t* pApp)
{
	const char* sFontPath;
	char* sSnapshot;
	int iWidth;
	int iHeight;
	int iRet;

	pApp->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pApp->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iWidth = __uiDesignAppSurfaceWidth(pApp);
	iHeight = __uiDesignAppSurfaceHeight(pApp);
	iRet = __uiDesignAppResizeTarget(pApp, iWidth, iHeight);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __uiDesignFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pApp->pContext, pApp->pFont);
	uiDesignModelInit(&pApp->tModel);
	if ( pApp->bPreviewRunner ) {
		if ( pApp->sPreviewPath[0] == '\0' ) return XUI_ERROR_INVALID_ARGUMENT;
		iRet = __uiDesignCreatePreviewRoot(pApp);
		if ( iRet != XUI_OK ) return iRet;
		sSnapshot = __uiDesignAppReadTextFile(pApp->sPreviewPath);
		if ( pApp->bPreviewDeleteFile ) (void)remove(pApp->sPreviewPath);
		if ( sSnapshot == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
		iRet = __uiDesignAppRestoreSnapshot(pApp, sSnapshot);
		free(sSnapshot);
		if ( iRet != XUI_OK ) return iRet;
		if ( pApp->bExercise ) {
			iRet = __uiDesignExercisePreviewRuntime(pApp);
			if ( iRet != XUI_OK ) return iRet;
		}
		pApp->bCreateOK = 1;
		pApp->bLayoutOK = 1;
		pApp->bModelOK = 1;
		pApp->bPaintOK = 1;
		return XUI_OK;
	}
	pApp->iActiveTool = UI_DESIGN_NODE_NONE;
	pApp->iDraggingTool = UI_DESIGN_NODE_NONE;
	pApp->bGridVisible = 1;
	pApp->bSnapEnabled = 0;
	pApp->bMarqueeSelectContain = 1;
	pApp->fZoom = 1.0f;
	pApp->iStatusMessageItem = -1;
	pApp->iStatusSelectionItem = -1;
	pApp->iStatusZoomItem = -1;
	iRet = __uiDesignCreateRoot(pApp);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateChrome(pApp);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateDock(pApp);
	if ( iRet == XUI_OK && pApp->bExercise ) iRet = __uiDesignSeedExercise(pApp);
	return iRet;
}

static void __uiDesignDestroyAssets(ui_design_app_t* pApp)
{
	int i;

	if ( pApp != NULL ) {
		for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
			__uiDesignAppReleaseNodeRuntime(pApp, &pApp->tModel.arrNodes[i]);
		}
		free(pApp->pClipboardNodes);
		pApp->pClipboardNodes = NULL;
		pApp->iClipboardNodeCount = 0;
		pApp->iClipboardCapacity = 0;
		__uiDesignHistoryClear(pApp->arrUndo, &pApp->iUndoCount);
		__uiDesignHistoryClear(pApp->arrRedo, &pApp->iRedoCount);
		uiDesignAppCancelHistoryTransaction(pApp);
		if ( pApp->pFileDialog != NULL ) {
			xuiFileDialogDestroy(pApp->pFileDialog);
			pApp->pFileDialog = NULL;
		}
	}
	if ( pApp->pContext != NULL ) {
		xuiDestroy(pApp->pContext);
		pApp->pContext = NULL;
	}
	if ( pApp->pFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont);
		pApp->pFont = NULL;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
}

static uint32_t __uiDesignReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static uint32_t __uiDesignReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_SHIFT) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_CTRL) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_ALT) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	return iModifiers;
}

static int __uiDesignMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_SPACE: return XUI_KEY_SPACE;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_PAGE_UP: return XUI_KEY_PAGE_UP;
	case XGE_KEY_PAGE_DOWN: return XUI_KEY_PAGE_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	default: return 0;
	}
}

static int __uiDesignCanHandleDesignShortcut(ui_design_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pContext == NULL) || (pApp->pOverlay == NULL) ) return 0;
	return xuiGetFocusWidget(pApp->pContext) == pApp->pOverlay;
}

static void __uiDesignShortcutPastePoint(ui_design_app_t* pApp, float* pX, float* pY)
{
	xui_rect_t tRect;

	if ( pX != NULL ) *pX = 48.0f;
	if ( pY != NULL ) *pY = 48.0f;
	if ( (pApp == NULL) || (pX == NULL) || (pY == NULL) ) return;
	if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, pApp->tModel.iSelectedId, &tRect) == XUI_OK ) {
		*pX = tRect.fX + 16.0f;
		*pY = tRect.fY + 16.0f;
	}
}

static int __uiDesignHandleDesignShortcut(ui_design_app_t* pApp, int iKey, uint32_t iModifiers, uint32_t* pResult)
{
	int iRet;
	ui_design_command_t iCommand;

	if ( !__uiDesignCanHandleDesignShortcut(pApp) ) return XUI_OK;
	iRet = XUI_OK;
	iCommand = UI_DESIGN_COMMAND_NONE;
	if ( (iModifiers & XUI_MOD_CTRL) != 0u && (iModifiers & XUI_MOD_ALT) == 0u ) {
		switch ( iKey ) {
		case 'A':
			iCommand = UI_DESIGN_COMMAND_EDIT_SELECT_ALL;
			break;
		case 'C':
			iCommand = UI_DESIGN_COMMAND_EDIT_COPY;
			break;
		case 'D':
			iCommand = UI_DESIGN_COMMAND_EDIT_DUPLICATE;
			break;
		case 'X':
			iCommand = UI_DESIGN_COMMAND_EDIT_CUT;
			break;
		case 'V':
			iCommand = UI_DESIGN_COMMAND_EDIT_PASTE;
			break;
		case 'Y':
			iCommand = UI_DESIGN_COMMAND_EDIT_REDO;
			break;
		case 'Z':
			iCommand = UI_DESIGN_COMMAND_EDIT_UNDO;
			break;
		case 'N':
			iCommand = UI_DESIGN_COMMAND_FILE_NEW;
			break;
		case 'O':
			iCommand = UI_DESIGN_COMMAND_FILE_OPEN;
			break;
		case 'S':
			iCommand = ((iModifiers & XUI_MOD_SHIFT) != 0u) ? UI_DESIGN_COMMAND_FILE_SAVE_AS : UI_DESIGN_COMMAND_FILE_SAVE;
			break;
		default:
			return XUI_OK;
		}
		if ( iCommand != UI_DESIGN_COMMAND_NONE && uiDesignAppCanExecuteCommand(pApp, iCommand) ) iRet = uiDesignAppExecuteCommand(pApp, iCommand);
		if ( iRet == XUI_OK && pResult != NULL ) *pResult |= XUI_INPUT_RESULT_CONSUMED;
		return iRet;
	}
	if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u && iKey == XUI_KEY_DELETE && uiDesignAppCanExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_DELETE) ) {
		iRet = uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_DELETE);
		if ( iRet == XUI_OK && pResult != NULL ) *pResult |= XUI_INPUT_RESULT_CONSUMED;
	}
	return iRet;
}

static int __uiDesignSendKeys(ui_design_app_t* pApp)
{
	static const int arrKeys[] = {
		'A',
		'C',
		'D',
		'N',
		'O',
		'S',
		'V',
		'X',
		'Y',
		'Z',
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_SPACE,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_PAGE_UP,
		XGE_KEY_PAGE_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_ESCAPE
	};
	uint32_t iModifiers;
	uint32_t iResult;
	uint32_t iText;
	int iKey;
	int iRet;
	int i;

	iModifiers = __uiDesignReadModifiers();
	iRet = xuiInputSetModifiers(pApp->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __uiDesignMapKey(arrKeys[i]);
		if ( iKey == 0 ) iKey = arrKeys[i];
		if ( xgeKeyPressed(arrKeys[i]) ) {
			iResult = 0u;
			iRet = xuiInputKeyDownEx(pApp->pContext, iKey, iModifiers, &iResult);
			if ( iRet != XUI_OK ) return iRet;
			if ( !pApp->bPreviewRunner && (iResult & XUI_INPUT_RESULT_CONSUMED) == 0u ) {
				iRet = __uiDesignHandleDesignShortcut(pApp, iKey, iModifiers, &iResult);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( (iResult & XUI_INPUT_RESULT_CONSUMED) != 0u ) xgeInputConsumeKey(arrKeys[i]);
			if ( arrKeys[i] == XGE_KEY_ESCAPE && (iResult & XUI_INPUT_RESULT_CONSUMED) == 0u && pApp->iMaxFrames <= 0 ) xgeQuit();
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iResult = 0u;
			iRet = xuiInputKeyUpEx(pApp->pContext, iKey, iModifiers, &iResult);
			if ( iRet != XUI_OK ) return iRet;
			if ( (iResult & XUI_INPUT_RESULT_CONSUMED) != 0u ) xgeInputConsumeKey(arrKeys[i]);
		}
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u ) {
			iRet = xuiInputTextEx(pApp->pContext, iText, NULL);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __uiDesignHandleInput(ui_design_app_t* pApp)
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
	iButtons = __uiDesignReadButtons();
	if ( !pApp->bHasMouse || pApp->fLastMouseX != fX || pApp->fLastMouseY != fY || pApp->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pApp->pContext, fX, fY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pApp->pContext, fX, fY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pApp->iLastButtons;
	iReleased = pApp->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __uiDesignSendKeys(pApp);
	if ( iRet != XUI_OK ) return iRet;
	pApp->bHasMouse = 1;
	pApp->fLastMouseX = fX;
	pApp->fLastMouseY = fY;
	pApp->iLastButtons = iButtons;
	return XUI_OK;
}

static int __uiDesignFrameFail(const char* sStage, int iRet)
{
	printf("xui_uidesign frame failed at %s: %d\n", sStage != NULL ? sStage : "unknown", iRet);
	return iRet;
}

static void __uiDesignRunChecks(ui_design_app_t* pApp)
{
	xui_dock_pane_info_t tPane;
	float fScrollMin;
	float fScrollMax;
	float fScrollPage;
	float fScrollOld;
	float fScrollTest;
	float fScrollRead;
	int i;
	int bWidgetsOK;
	int bToolboxScrollOK;

	pApp->bCreateOK = (pApp->pDock != NULL) && (xuiDockPanelGetWindowCount(pApp->pDock) == 3);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pTree != NULL) && (pApp->pPropertyGrid != NULL) && (pApp->pArtboard != NULL) && (pApp->pOverlay != NULL);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pToolbox != NULL) && (pApp->pToolboxScrollBar != NULL);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pCanvasContextMenu != NULL);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pMenuBar != NULL) && (xuiMenuBarGetItemCount(pApp->pMenuBar) >= 6);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pFileMenu != NULL) && (xuiMenuGetItemCount(pApp->pFileMenu) >= 7);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pEditMenu != NULL) && (xuiMenuGetItemCount(pApp->pEditMenu) >= 10);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pArrangeMenu != NULL) && (xuiMenuGetItemCount(pApp->pArrangeMenu) >= 11);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pViewMenu != NULL) && (xuiMenuGetItemCount(pApp->pViewMenu) >= 13);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pToolsMenu != NULL) && (pApp->pHelpMenu != NULL);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pToolbar != NULL) && (xuiToolbarGetItemCount(pApp->pToolbar) >= 18);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pStatusBar != NULL) && (xuiStatusBarGetItemCount(pApp->pStatusBar) >= 3);
	pApp->bLayoutOK = (xuiDockPanelGetPaneInfo(pApp->pDock, pApp->iCanvasPane, &tPane) == XUI_OK) && (tPane.tClientRect.fW > 200.0f) && (tPane.tClientRect.fH > 200.0f);
	bToolboxScrollOK = 1;
	if ( pApp->bExercise ) {
		bToolboxScrollOK = 0;
		if ( pApp->pToolboxScrollBar != NULL &&
		     xuiWidgetGetVisible(pApp->pToolboxScrollBar) &&
		     xuiScrollBarGetRange(pApp->pToolboxScrollBar, &fScrollMin, &fScrollMax, &fScrollPage) == XUI_OK &&
		     fScrollMin == 0.0f &&
		     fScrollMax > 0.0f &&
		     fScrollPage > 0.0f ) {
			fScrollOld = xuiScrollBarGetValue(pApp->pToolboxScrollBar);
			fScrollTest = (fScrollMax > 24.0f) ? 24.0f : fScrollMax;
			if ( xuiScrollBarSetValue(pApp->pToolboxScrollBar, fScrollTest) == XUI_OK ) {
				fScrollRead = xuiScrollBarGetValue(pApp->pToolboxScrollBar);
				bToolboxScrollOK = (fScrollRead >= fScrollTest - 0.01f) && (fScrollRead <= fScrollTest + 0.01f);
				(void)xuiScrollBarSetValue(pApp->pToolboxScrollBar, fScrollOld);
			}
		}
	}
	pApp->bLayoutOK = pApp->bLayoutOK && bToolboxScrollOK;
	bWidgetsOK = 1;
	for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
		if ( pApp->tModel.arrNodes[i].pWidget == NULL ) {
			bWidgetsOK = 0;
			break;
		}
	}
	pApp->bModelOK = pApp->bExercise ? (pApp->tModel.iNodeCount >= 3 && pApp->tModel.iSelectedId > 0 && bWidgetsOK) : 1;
	pApp->bPaintOK = pApp->tModel.iRevision > 0u;
}

static int __uiDesignPreviewFrame(ui_design_app_t* pApp)
{
	xui_render_stats_t tStats;
	xui_rect_i_t tFull;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iWidth;
	int iHeight;
	int iRet;

	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignAppRefreshWindowSize(pApp);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("resize", iRet);
	iWidth = __uiDesignAppSurfaceWidth(pApp);
	iHeight = __uiDesignAppSurfaceHeight(pApp);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("xgeBegin", iRet);
	iRet = __uiDesignHandleInput(pApp);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("handleInput", iRet);
	iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("dispatchEvents", iRet);
	iRet = xuiLayout(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("layout", iRet);
	iRet = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("update", iRet);
	iRet = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_WHITE);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("surfaceClear", iRet);
	tFull = (xui_rect_i_t){0, 0, iWidth, iHeight};
	iRet = xuiRender(pApp->pContext, pApp->pTarget, &tFull, 1);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("render", iRet);
	xgeClear(XUI_COLOR_WHITE);
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
	tDst = tSrc;
	iRet = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("present", iRet);
	pApp->iFrame++;
	if ( (pApp->iMaxFrames > 0 && pApp->iFrame >= pApp->iMaxFrames) ||
	     (pApp->fMaxSeconds > 0.0 && xgeTimer() >= pApp->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pApp->pContext, &tStats);
		printf("xui_uidesign preview-summary frames=%d nodes=%d caches=%d/%d\n",
			pApp->iFrame,
			pApp->tModel.iNodeCount,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

static int __uiDesignFrame(void* pUser)
{
	ui_design_app_t* pApp;
	xui_render_stats_t tStats;
	xui_rect_i_t tFull;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iWidth;
	int iHeight;
	int iRet;

	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pApp->bPreviewRunner ) return __uiDesignPreviewFrame(pApp);
	iRet = __uiDesignAppRefreshWindowSize(pApp);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("resize", iRet);
	iWidth = __uiDesignAppSurfaceWidth(pApp);
	iHeight = __uiDesignAppSurfaceHeight(pApp);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("xgeBegin", iRet);
	iRet = __uiDesignHandleInput(pApp);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("handleInput", iRet);
	iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("dispatchEvents", iRet);
	iRet = xuiLayout(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("layout", iRet);
	iRet = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("update", iRet);
	__uiDesignRunChecks(pApp);
	iRet = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(229, 236, 245, 255));
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("surfaceClear", iRet);
	tFull = (xui_rect_i_t){0, 0, iWidth, iHeight};
	iRet = xuiRender(pApp->pContext, pApp->pTarget, &tFull, 1);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("render", iRet);
	xgeClear(XUI_COLOR_RGBA(22, 28, 36, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
	tDst = tSrc;
	iRet = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("present", iRet);
	pApp->iFrame++;
	if ( (pApp->iMaxFrames > 0 && pApp->iFrame >= pApp->iMaxFrames) ||
	     (pApp->fMaxSeconds > 0.0 && xgeTimer() >= pApp->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pApp->pContext, &tStats);
		printf("xui_uidesign final-summary frames=%d create=%d layout=%d model=%d paint=%d nodes=%d selected=%d caches=%d/%d\n",
			pApp->iFrame,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bModelOK,
			pApp->bPaintOK,
			pApp->tModel.iNodeCount,
			pApp->tModel.iSelectedId,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	ui_design_app_t* pApp;
	xge_desc_t tDesc;
	int iRet;
	int iExit;

	pApp = (ui_design_app_t*)calloc(1, sizeof(*pApp));
	if ( pApp == NULL ) {
		printf("xui_uidesign: out of memory\n");
		return 1;
	}
	iRet = __uiDesignParseArgs(pApp, argc, argv);
	if ( iRet == 1 ) {
		free(pApp);
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__uiDesignUsage();
		free(pApp);
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = __uiDesignAppSurfaceWidth(pApp);
	tDesc.iHeight = __uiDesignAppSurfaceHeight(pApp);
	tDesc.sTitle = pApp->bPreviewRunner ? "XUI UI Preview" : "XUI UI Design";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_uidesign: xgeInit failed: %d\n", iRet);
		free(pApp);
		return 1;
	}
	iRet = __uiDesignCreateAssets(pApp);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign: create assets failed: %d\n", iRet);
		__uiDesignDestroyAssets(pApp);
		xgeUnit();
		free(pApp);
		return 1;
	}
	iRet = xgeRun(__uiDesignFrame, pApp);
	iExit = (iRet == XGE_OK && pApp->bCreateOK && pApp->bLayoutOK && pApp->bModelOK && pApp->bPaintOK) ? 0 : 1;
	__uiDesignDestroyAssets(pApp);
	xgeUnit();
	free(pApp);
	return iExit;
}
