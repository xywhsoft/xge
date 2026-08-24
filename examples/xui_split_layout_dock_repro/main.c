#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 1160
#define DEMO_H 760
#define PANEL_STYLE_COUNT 8

typedef struct repro_panel_style_t {
	xui_proxy_t* pProxy;
	uint32_t iFill;
	uint32_t iBorder;
} repro_panel_style_t;

typedef struct repro_app_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pFileMenu;
	xui_widget pViewMenu;
	xui_widget pToolbar;
	xui_widget pDock;
	xui_widget pStatusBar;
	xui_widget pSplitHost;
	xui_widget pSplit;
	repro_panel_style_t arrStyle[PANEL_STYLE_COUNT];
	int iStyleCount;
	int iDocumentPane;
	int iStatusMain;
	int iSplitChanges;
	int iFrames;
	int iMaxFrames;
	int bHighDpi;
	int bCreated;
	int bLayoutOK;
	int bRenderOK;
} repro_app_t;

static void __reproUsage(void)
{
	printf("usage: xui_split_layout_dock_repro [--frames N] [--high-dpi]\n");
	printf("       interactive mode runs on demand until Escape or window close.\n");
}

static int __reproParseArgs(repro_app_t* pApp, int argc, char** argv)
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
		} else if ( strcmp(argv[i], "--high-dpi") == 0 ) {
			pApp->bHighDpi = 1;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__reproUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __reproFindFont(void)
{
	static const char* arrPath[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc"
	};
	FILE* pFile;
	int i;
	for ( i = 0; i < (int)(sizeof(arrPath) / sizeof(arrPath[0])); i++ ) {
		pFile = fopen(arrPath[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPath[i];
		}
	}
	return NULL;
}

static int __reproRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iState, void* pUser)
{
	repro_app_t* pApp = (repro_app_t*)pUser;
	xui_rect_t tRect;
	(void)iState;
	if ( pWidget == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(235, 241, 248, 255));
}

static int __reproPanelRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iState, void* pUser)
{
	repro_panel_style_t* pStyle = (repro_panel_style_t*)pUser;
	xui_rect_t tRect;
	int iRet;
	(void)iState;
	if ( pWidget == NULL || pStyle == NULL || pStyle->pProxy == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = pStyle->pProxy->drawRectFill(pStyle->pProxy, pDraw, tRect, pStyle->iFill);
	if ( iRet != XUI_OK ) return iRet;
	return pStyle->pProxy->drawRectStroke(pStyle->pProxy, pDraw, tRect, 1.0f, pStyle->iBorder);
}

static int __reproStyleWidget(repro_app_t* pApp, xui_widget pWidget, uint32_t iFill, uint32_t iBorder)
{
	repro_panel_style_t* pStyle;
	if ( pApp->iStyleCount >= PANEL_STYLE_COUNT ) return XUI_ERROR_OUT_OF_MEMORY;
	pStyle = &pApp->arrStyle[pApp->iStyleCount++];
	pStyle->pProxy = &pApp->tProxy;
	pStyle->iFill = iFill;
	pStyle->iBorder = iBorder;
	return xuiWidgetSetCacheRenderCallback(pWidget, __reproPanelRender, pStyle);
}

static int __reproAddLabel(repro_app_t* pApp, xui_widget pParent, const char* sText, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pApp->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pApp->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pLabel, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	(void)xuiWidgetSetAlign(pLabel, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) xuiWidgetDestroy(pLabel);
	return iRet;
}

static int __reproAddButton(repro_app_t* pApp, xui_widget pParent, const char* sText)
{
	xui_button_desc_t tDesc;
	xui_widget pButton;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pApp->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(45, 63, 84, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.iNormalColor = XUI_COLOR_RGBA(248, 251, 255, 255);
	tDesc.iHoverColor = XUI_COLOR_RGBA(225, 238, 253, 255);
	tDesc.iActiveColor = XUI_COLOR_RGBA(205, 224, 248, 255);
	tDesc.iBorderColor = XUI_COLOR_RGBA(155, 178, 205, 255);
	tDesc.fBorderWidth = 1.0f;
	iRet = xuiButtonCreate(pApp->pContext, &pButton, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetPreferredSize(pButton, (xui_vec2_t){112.0f, 28.0f});
	(void)xuiWidgetSetAlign(pButton, XUI_ALIGN_START, XUI_ALIGN_START);
	iRet = xuiWidgetAddChild(pParent, pButton);
	if ( iRet != XUI_OK ) xuiWidgetDestroy(pButton);
	return iRet;
}

static int __reproCreateContent(repro_app_t* pApp, xui_widget* ppWidget, const char* sTitle,
	const char* sText, uint32_t iFill, uint32_t iBorder)
{
	xui_widget pWidget;
	int iRet;
	iRet = xuiWidgetCreate(pApp->pContext, &pWidget);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(pWidget, (xui_thickness_t){12.0f, 10.0f, 12.0f, 10.0f});
	(void)xuiWidgetSetGap(pWidget, 8.0f);
	iRet = __reproStyleWidget(pApp, pWidget, iFill, iBorder);
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pWidget, sTitle, XUI_COLOR_RGBA(42, 64, 91, 255));
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pWidget, sText, XUI_COLOR_RGBA(80, 99, 122, 255));
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pWidget);
		return iRet;
	}
	*ppWidget = pWidget;
	return XUI_OK;
}

static void __reproCommand(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	repro_app_t* pApp = (repro_app_t*)pUser;
	char sText[128];
	(void)pWidget;
	(void)iIndex;
	if ( pApp == NULL || pApp->pStatusBar == NULL ) return;
	snprintf(sText, sizeof(sText), "Command %d selected", iValue);
	(void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusMain, sText);
}

static void __reproSplitChanged(xui_widget pWidget, int iDivider, void* pUser)
{
	repro_app_t* pApp = (repro_app_t*)pUser;
	char sText[160];
	(void)pWidget;
	if ( pApp == NULL || pApp->pStatusBar == NULL ) return;
	pApp->iSplitChanges++;
	snprintf(sText, sizeof(sText), "Split divider %d committed, changes=%d", iDivider, pApp->iSplitChanges);
	(void)xuiStatusBarSetItemText(pApp->pStatusBar, pApp->iStatusMain, sText);
}

static int __reproCreateMenu(repro_app_t* pApp)
{
	xui_menubar_desc_t tBarDesc;
	xui_menu_desc_t tMenuDesc;
	xui_menubar_item_t arrBar[4];
	xui_menu_item_t arrFile[4];
	xui_menu_item_t arrView[3];
	int iRet;
	memset(&tBarDesc, 0, sizeof(tBarDesc));
	tBarDesc.iSize = sizeof(tBarDesc);
	tBarDesc.pFont = pApp->pFont;
	iRet = xuiMenuBarCreate(pApp->pContext, &pApp->pMenuBar, &tBarDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pMenuBar);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tMenuDesc, 0, sizeof(tMenuDesc));
	tMenuDesc.iSize = sizeof(tMenuDesc);
	tMenuDesc.pOwner = pApp->pMenuBar;
	tMenuDesc.pFont = pApp->pFont;
	iRet = xuiMenuCreate(pApp->pContext, &pApp->pFileMenu, &tMenuDesc);
	if ( iRet == XUI_OK ) iRet = xuiMenuCreate(pApp->pContext, &pApp->pViewMenu, &tMenuDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrFile, 0, sizeof(arrFile));
	arrFile[0] = (xui_menu_item_t){"New", "Ctrl+N", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 101, 0, NULL, NULL};
	arrFile[1] = (xui_menu_item_t){"Open...", "Ctrl+O", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 102, 0, NULL, NULL};
	arrFile[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrFile[3] = (xui_menu_item_t){"Exit", "Alt+F4", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 103, 0, NULL, NULL};
	iRet = xuiMenuSetItems(pApp->pFileMenu, arrFile, 4);
	if ( iRet == XUI_OK ) iRet = xuiMenuSetSelect(pApp->pFileMenu, __reproCommand, pApp);
	memset(arrView, 0, sizeof(arrView));
	arrView[0] = (xui_menu_item_t){"Toolbox", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 201, 0, NULL, NULL};
	arrView[1] = (xui_menu_item_t){"Inspector", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 202, 0, NULL, NULL};
	arrView[2] = (xui_menu_item_t){"Output", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 203, 0, NULL, NULL};
	if ( iRet == XUI_OK ) iRet = xuiMenuSetItems(pApp->pViewMenu, arrView, 3);
	if ( iRet == XUI_OK ) iRet = xuiMenuSetSelect(pApp->pViewMenu, __reproCommand, pApp);
	memset(arrBar, 0, sizeof(arrBar));
	arrBar[0] = (xui_menubar_item_t){"&File", XUI_MENUBAR_ITEM_ENABLED, 1, 0, pApp->pFileMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrBar[1] = (xui_menubar_item_t){"&Edit", XUI_MENUBAR_ITEM_ENABLED, 2, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrBar[2] = (xui_menubar_item_t){"&View", XUI_MENUBAR_ITEM_ENABLED, 3, 0, pApp->pViewMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrBar[3] = (xui_menubar_item_t){"&Help", XUI_MENUBAR_ITEM_ENABLED, 4, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	if ( iRet == XUI_OK ) iRet = xuiMenuBarSetItems(pApp->pMenuBar, arrBar, 4);
	if ( iRet == XUI_OK ) iRet = xuiMenuBarSetSelect(pApp->pMenuBar, __reproCommand, pApp);
	return iRet;
}

static int __reproCreateToolbar(repro_app_t* pApp)
{
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_item_t arrItem[7];
	int iRet;
	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tMetrics.fItemWidth = 66.0f;
	tMetrics.fItemHeight = 28.0f;
	tMetrics.fSeparatorSize = 10.0f;
	tMetrics.fGroupGap = 6.0f;
	tMetrics.fPaddingX = 5.0f;
	tMetrics.fPaddingY = 4.0f;
	tMetrics.fOverflowSize = 28.0f;
	tMetrics.fBorderWidth = 1.0f;
	memset(arrItem, 0, sizeof(arrItem));
	arrItem[0] = (xui_toolbar_item_t){"New", "New", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 301, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[1] = (xui_toolbar_item_t){"Open", "Open", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 302, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[2] = (xui_toolbar_item_t){"Save", "Save", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 303, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[3] = (xui_toolbar_item_t){"", "", XUI_TOOLBAR_ITEM_SEPARATOR, 0, 0, 0, NULL,
		{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[4] = (xui_toolbar_item_t){"Undo", "Undo", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 304, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[5] = (xui_toolbar_item_t){"Redo", "Redo", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 305, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItem[6] = (xui_toolbar_item_t){"Run", "Run", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 306, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItem;
	tDesc.iItemCount = 7;
	tDesc.pFont = pApp->pFont;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	tDesc.bOverflowEnabled = 1;
	iRet = xuiToolbarCreate(pApp->pContext, &pApp->pToolbar, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiToolbarSetSelect(pApp->pToolbar, __reproCommand, pApp);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pToolbar);
	return iRet;
}

static int __reproCreateStatusBar(repro_app_t* pApp)
{
	xui_statusbar_desc_t tDesc;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pApp->pFont;
	if ( xuiStatusBarCreate(pApp->pContext, &pApp->pStatusBar, &tDesc) != XUI_OK ) return XUI_ERROR;
	if ( xuiWidgetAddChild(pApp->pRoot, pApp->pStatusBar) != XUI_OK ) return XUI_ERROR;
	pApp->iStatusMain = xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_LEFT,
		"Ready - drag the divider in Split Repro", 320.0f, 0, 0);
	(void)xuiStatusBarAddFlexibleSpacer(pApp->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, 1.0f);
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_CENTER, "APP mode", 92.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "Line 1, Col 1", 110.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "UTF-8", 58.0f, 0, 0);
	return pApp->iStatusMain >= 0 ? XUI_OK : XUI_ERROR;
}

static int __reproCreateSplit(repro_app_t* pApp)
{
	xui_split_layout_desc_t tDesc;
	xui_widget pLeft;
	xui_widget pRight;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 2;
	tDesc.fDividerSize = 3.0f;
	tDesc.fDividerVisualSize = 3.0f;
	tDesc.fDividerHitSize = 14.0f;
	tDesc.iDividerColor = XUI_COLOR_RGBA(75, 135, 208, 255);
	tDesc.iDividerHoverColor = XUI_COLOR_RGBA(34, 112, 210, 255);
	tDesc.iDividerActiveColor = XUI_COLOR_RGBA(20, 90, 190, 255);
	tDesc.iShadowColor = XUI_COLOR_RGBA(55, 205, 205, 150);
	iRet = xuiSplitLayoutCreate(pApp->pContext, &pApp->pSplit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pApp->pSplitHost, pApp->pSplit);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSplitLayoutSetChange(pApp->pSplit, __reproSplitChanged, pApp);
	(void)xuiSplitLayoutSetPaneMode(pApp->pSplit, 0, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pApp->pSplit, 0, 280.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pApp->pSplit, 0, 150.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pApp->pSplit, 1, 220.0f);
	pLeft = xuiSplitLayoutGetPaneWidget(pApp->pSplit, 0);
	pRight = xuiSplitLayoutGetPaneWidget(pApp->pSplit, 1);
	(void)xuiWidgetSetLayoutType(pLeft, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetLayoutType(pRight, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetPadding(pLeft, (xui_thickness_t){14.0f, 12.0f, 14.0f, 12.0f});
	(void)xuiWidgetSetPadding(pRight, (xui_thickness_t){14.0f, 12.0f, 14.0f, 12.0f});
	(void)xuiWidgetSetGap(pLeft, 10.0f);
	(void)xuiWidgetSetGap(pRight, 10.0f);
	iRet = __reproStyleWidget(pApp, pLeft, XUI_COLOR_RGBA(248, 251, 255, 255), XUI_COLOR_RGBA(174, 196, 222, 255));
	if ( iRet == XUI_OK ) iRet = __reproStyleWidget(pApp, pRight, XUI_COLOR_RGBA(250, 252, 249, 255), XUI_COLOR_RGBA(183, 209, 190, 255));
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pLeft, "Search options", XUI_COLOR_RGBA(42, 68, 98, 255));
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pLeft, "This pane is fixed width. Drag its right divider.", XUI_COLOR_RGBA(83, 103, 126, 255));
	if ( iRet == XUI_OK ) iRet = __reproAddButton(pApp, pLeft, "Select source");
	if ( iRet == XUI_OK ) iRet = __reproAddButton(pApp, pLeft, "Run search");
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pRight, "Result preview", XUI_COLOR_RGBA(42, 82, 58, 255));
	if ( iRet == XUI_OK ) iRet = __reproAddLabel(pApp, pRight, "The cyan 3 px shadow must track the pointer exactly.", XUI_COLOR_RGBA(82, 109, 90, 255));
	return iRet;
}

static int __reproCreateDock(repro_app_t* pApp)
{
	xui_dock_panel_desc_t tDesc;
	xui_widget pReadme;
	xui_widget pToolbox;
	xui_widget pInspector;
	xui_widget pOutput;
	int iSplitWindow;
	int iReadmeWindow;
	int iToolboxWindow;
	int iInspectorWindow;
	int iOutputWindow;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pApp->pFont;
	iRet = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pDock);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pSplitHost);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pApp->pSplitHost, XUI_LAYOUT_MANUAL);
	iRet = __reproCreateContent(pApp, &pReadme, "Readme", "Second document tab placeholder.",
		XUI_COLOR_RGBA(249, 251, 254, 255), XUI_COLOR_RGBA(190, 204, 220, 255));
	if ( iRet == XUI_OK ) iRet = __reproCreateContent(pApp, &pToolbox, "Toolbox", "Pointer\nLabel\nButton\nSplitLayout",
		XUI_COLOR_RGBA(246, 249, 253, 255), XUI_COLOR_RGBA(181, 199, 219, 255));
	if ( iRet == XUI_OK ) iRet = __reproCreateContent(pApp, &pInspector, "Inspector", "Name: split_repro\nOrientation: Vertical\nDivider: 3 px",
		XUI_COLOR_RGBA(250, 252, 249, 255), XUI_COLOR_RGBA(190, 210, 194, 255));
	if ( iRet == XUI_OK ) iRet = __reproCreateContent(pApp, &pOutput, "Output", "Drag commits are reported in the status bar.",
		XUI_COLOR_RGBA(253, 250, 246, 255), XUI_COLOR_RGBA(222, 204, 181, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDockPanelAddWindow(pApp->pDock, "Split Repro", pApp->pSplitHost, &iSplitWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Readme", pReadme, &iReadmeWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Toolbox", pToolbox, &iToolboxWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Inspector", pInspector, &iInspectorWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Output", pOutput, &iOutputWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, iSplitWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pApp->iDocumentPane);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindowToPane(pApp->pDock, iReadmeWindow, pApp->iDocumentPane);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelSetPaneActiveWindow(pApp->pDock, pApp->iDocumentPane, iSplitWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, iToolboxWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.19f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, iInspectorWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.22f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, iOutputWindow,
		XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.22f, NULL);
	if ( iRet == XUI_OK ) iRet = __reproCreateSplit(pApp);
	return iRet;
}

static int __reproCreateUi(repro_app_t* pApp)
{
	int iRet;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( iRet != XUI_OK ) {
		printf("repro create stage=root: %d\n", iRet);
		return iRet;
	}
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __reproRootRender, pApp);
	iRet = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if ( iRet == XUI_OK ) {
		iRet = __reproCreateMenu(pApp);
		if ( iRet != XUI_OK ) printf("repro create stage=menu: %d\n", iRet);
	}
	if ( iRet == XUI_OK ) {
		iRet = __reproCreateToolbar(pApp);
		if ( iRet != XUI_OK ) printf("repro create stage=toolbar: %d\n", iRet);
	}
	if ( iRet == XUI_OK ) {
		iRet = __reproCreateStatusBar(pApp);
		if ( iRet != XUI_OK ) printf("repro create stage=statusbar: %d\n", iRet);
	}
	if ( iRet == XUI_OK ) {
		iRet = __reproCreateDock(pApp);
		if ( iRet != XUI_OK ) printf("repro create stage=dock: %d\n", iRet);
	}
	pApp->bCreated = (iRet == XUI_OK);
	return iRet;
}

static int __reproCreateAssets(repro_app_t* pApp)
{
	xui_surface_desc_t tSurface;
	const char* sFont;
	int iRet;
	pApp->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pApp->pContext);
	if ( iRet == XUI_OK ) iRet = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( iRet == XUI_OK ) iRet = xuiInputViewport(pApp->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurface, 0, sizeof(tSurface));
	tSurface.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurface.iWidth = DEMO_W;
	tSurface.iHeight = DEMO_H;
	tSurface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurface.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &tSurface);
	if ( iRet != XUI_OK ) return iRet;
	sFont = __reproFindFont();
	if ( sFont == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFont, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet == XUI_OK ) iRet = xuiSetDefaultFont(pApp->pContext, pApp->pFont);
	if ( iRet == XUI_OK ) iRet = __reproCreateUi(pApp);
	return iRet;
}

static void __reproDestroy(repro_app_t* pApp)
{
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

static void __reproLayout(repro_app_t* pApp)
{
	xui_rect_t tHost;
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	(void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, 28.0f});
	(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0.0f, 28.0f, (float)DEMO_W, 38.0f});
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){8.0f, 72.0f, (float)DEMO_W - 16.0f, (float)DEMO_H - 106.0f});
	(void)xuiWidgetSetRect(pApp->pStatusBar, (xui_rect_t){0.0f, (float)DEMO_H - 28.0f, (float)DEMO_W, 28.0f});
	(void)xuiLayout(pApp->pContext);
	tHost = xuiWidgetGetRect(pApp->pSplitHost);
	(void)xuiWidgetSetRect(pApp->pSplit, (xui_rect_t){0.0f, 0.0f, tHost.fW, tHost.fH});
	(void)xuiLayout(pApp->pContext);
}

static void __reproCheckLayout(repro_app_t* pApp)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pApp->pSplit);
	xui_rect_t tDivider = xuiSplitLayoutGetDividerVisualRect(pApp->pSplit, 0);
	pApp->bLayoutOK = xuiDockPanelGetPaneCount(pApp->pDock) >= 4 &&
		tWorld.fX > 100.0f && tWorld.fY > 90.0f && tWorld.fW > 300.0f && tWorld.fH > 180.0f &&
		tDivider.fW == 3.0f && tDivider.fH > 100.0f;
}

static int __reproFrame(void* pUser)
{
	repro_app_t* pApp = (repro_app_t*)pUser;
	xui_rect_i_t tClip;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pApp->pContext == NULL ) {
		iRet = __reproCreateAssets(pApp);
		if ( iRet != XUI_OK ) {
			printf("xui_split_layout_dock_repro: create failed: %d\n", iRet);
			return iRet;
		}
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiProxyXgePumpInput(pApp->pContext);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__reproLayout(pApp);
	iRet = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__reproCheckLayout(pApp);
	iRet = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(235, 241, 248, 255));
	if ( iRet != XUI_OK ) return iRet;
	tClip = (xui_rect_i_t){0, 0, DEMO_W, DEMO_H};
	iRet = xuiRender(pApp->pContext, pApp->pTarget, &tClip, 1);
	if ( iRet != XUI_OK ) return iRet;
	pApp->bRenderOK = 1;
	xgeClear(XUI_COLOR_RGBA(20, 25, 33, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H};
	tDst = (xui_rect_t){0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight()};
	iRet = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, tSrc, tDst,
		XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pApp->iFrames++;
	if ( pApp->iMaxFrames > 0 && pApp->iFrames >= pApp->iMaxFrames ) {
		xui_rect_t tWorld = xuiWidgetGetWorldRect(pApp->pSplit);
		printf("xui_split_layout_dock_repro final-summary frames=%d create=%d layout=%d render=%d panes=%d splitWorld=%d,%d,%d,%d changes=%d\n",
			pApp->iFrames, pApp->bCreated, pApp->bLayoutOK, pApp->bRenderOK,
			xuiDockPanelGetPaneCount(pApp->pDock), tWorld.fX, tWorld.fY, tWorld.fW, tWorld.fH, pApp->iSplitChanges);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	repro_app_t tApp;
	xge_desc_t tDesc;
	int iRet;
	memset(&tApp, 0, sizeof(tApp));
	iRet = __reproParseArgs(&tApp, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__reproUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W;
	tDesc.iHeight = DEMO_H;
	tDesc.sTitle = "XUI SplitLayout Dock Repro";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	if ( tApp.bHighDpi ) tDesc.iFlags |= XGE_INIT_HIGHDPI;
	if ( tApp.iMaxFrames <= 0 ) tDesc.iFlags |= XGE_INIT_ON_DEMAND;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_split_layout_dock_repro: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = xgeRun(__reproFrame, &tApp);
	__reproDestroy(&tApp);
	xgeUnit();
	return (iRet == XGE_OK && tApp.bCreated && tApp.bLayoutOK && tApp.bRenderOK) ? 0 : 1;
}
