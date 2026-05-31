#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 900
#define DEMO_TARGET_H 560
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DOCK_CONTENT_COUNT 11

typedef struct xui_dock_demo_t xui_dock_demo_t;

typedef struct xui_dock_content_t {
	xui_dock_demo_t* pDemo;
	const char* sTitle;
	const char* sText;
	uint32_t iAccent;
} xui_dock_content_t;

struct xui_dock_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pDock;
	xui_widget pStatus;
	xui_widget arrContentWidget[DOCK_CONTENT_COUNT];
	xui_dock_content_t arrContent[DOCK_CONTENT_COUNT];
	int iDoc;
	int iPreview;
	int iScene;
	int iReadme;
	int iNotes;
	int iShader;
	int iConfig;
	int iTasks;
	int iToolbox;
	int iProperties;
	int iOutput;
	int iDocPane;
	int iToolboxPane;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iStateChanged;
	int iActiveChanged;
	int iClosed;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bTabOK;
	int bFloatOK;
	int bDragDockOK;
	int bAutoHideOK;
	int bMenuOK;
	int bDockableOK;
	int bPaintOK;
};

static void __xuiDockUsage(void)
{
	printf("usage: xui_dockpanel [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiDockParseArgs(xui_dock_demo_t* pDemo, int argc, char** argv)
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
			__xuiDockUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiDockFindTtf(void)
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

static uint32_t __xuiDockColorMix(uint32_t c, int add)
{
	int r = (int)((c >> 24) & 0xffu) + add;
	int g = (int)((c >> 16) & 0xffu) + add;
	int b = (int)((c >> 8) & 0xffu) + add;
	if ( r < 0 ) r = 0;
	if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0;
	if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0;
	if ( b > 255 ) b = 255;
	return XUI_COLOR_RGBA(r, g, b, 255);
}

static int __xuiDockRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_dock_demo_t* pDemo = (xui_dock_demo_t*)pUser;
	xui_rect_t r;
	(void)iStateId;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	r = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, r, XUI_COLOR_RGBA(232, 241, 250, 255));
	}
	return XUI_OK;
}

static int __xuiDockContentRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_dock_content_t* pContent = (xui_dock_content_t*)pUser;
	xui_dock_demo_t* pDemo;
	xui_rect_t r;
	xui_rect_t accent;
	int ret;
	(void)iStateId;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pContent == NULL) || (pContent->pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pDemo = pContent->pDemo;
	r = xuiWidgetGetContentRect(pWidget);
	if ( r.fW <= 0.0f || r.fH <= 0.0f ) return XUI_OK;
	ret = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, r, XUI_COLOR_RGBA(250, 253, 255, 255));
	if ( ret != XUI_OK ) return ret;
	accent = (xui_rect_t){r.fX, r.fY, 5.0f, r.fH};
	ret = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, accent, pContent->iAccent);
	if ( ret != XUI_OK ) return ret;
	ret = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, (xui_rect_t){r.fX + 18.0f, r.fY + 18.0f, 120.0f, 8.0f}, __xuiDockColorMix(pContent->iAccent, 42));
	if ( ret != XUI_OK ) return ret;
	if ( r.fW > 36.0f ) {
		ret = pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, pContent->sTitle, (xui_rect_t){r.fX + 18.0f, r.fY + 34.0f, r.fW - 36.0f, 24.0f}, XUI_COLOR_RGBA(32, 48, 68, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( ret != XUI_OK ) return ret;
		if ( r.fH > 80.0f ) {
			return pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, pContent->sText, (xui_rect_t){r.fX + 18.0f, r.fY + 64.0f, r.fW - 36.0f, r.fH - 80.0f}, XUI_COLOR_RGBA(84, 104, 126, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		}
	}
	return XUI_OK;
}

static int __xuiDockAddLabel(xui_dock_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget* pOut)
{
	xui_label_desc_t desc;
	xui_widget label;
	int ret;
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = pDemo->pFont;
	desc.sText = sText;
	desc.iTextColor = XUI_COLOR_RGBA(42, 60, 82, 255);
	desc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	ret = xuiLabelCreate(pDemo->pContext, &label, &desc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(label, tRect);
	ret = xuiWidgetAddChild(pDemo->pRoot, label);
	if ( ret != XUI_OK ) {
		xuiWidgetDestroy(label);
		return ret;
	}
	if ( pOut != NULL ) *pOut = label;
	return XUI_OK;
}

static int __xuiDockCreateContent(xui_dock_demo_t* pDemo, int iIndex, const char* sTitle, const char* sText, uint32_t iAccent)
{
	xui_cache_policy_t policy;
	xui_widget widget;
	int ret;
	if ( iIndex < 0 || iIndex >= DOCK_CONTENT_COUNT ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = xuiWidgetCreate(pDemo->pContext, &widget);
	if ( ret != XUI_OK ) return ret;
	memset(&policy, 0, sizeof(policy));
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	policy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pDemo->arrContent[iIndex].pDemo = pDemo;
	pDemo->arrContent[iIndex].sTitle = sTitle;
	pDemo->arrContent[iIndex].sText = sText;
	pDemo->arrContent[iIndex].iAccent = iAccent;
	(void)xuiWidgetSetLayoutType(widget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(widget, &policy);
	(void)xuiWidgetSetCacheRenderCallback(widget, __xuiDockContentRender, &pDemo->arrContent[iIndex]);
	pDemo->arrContentWidget[iIndex] = widget;
	return XUI_OK;
}

static void __xuiDockStateChanged(xui_widget pWidget, int iWindow, int iOldState, int iNewState, void* pUser)
{
	xui_dock_demo_t* pDemo = (xui_dock_demo_t*)pUser;
	(void)pWidget;
	(void)iWindow;
	(void)iOldState;
	(void)iNewState;
	if ( pDemo != NULL ) pDemo->iStateChanged++;
}

static void __xuiDockActiveChanged(xui_widget pWidget, int iPane, int iOldWindow, int iNewWindow, void* pUser)
{
	xui_dock_demo_t* pDemo = (xui_dock_demo_t*)pUser;
	(void)pWidget;
	(void)iPane;
	(void)iOldWindow;
	(void)iNewWindow;
	if ( pDemo != NULL ) pDemo->iActiveChanged++;
}

static void __xuiDockClosed(xui_widget pWidget, int iWindow, void* pUser)
{
	xui_dock_demo_t* pDemo = (xui_dock_demo_t*)pUser;
	(void)pWidget;
	(void)iWindow;
	if ( pDemo != NULL ) pDemo->iClosed++;
}

static void __xuiDockUpdateStatus(xui_dock_demo_t* pDemo)
{
	char sText[256];
	if ( pDemo->pStatus == NULL ) return;
	snprintf(sText, sizeof(sText), "windows=%d panes=%d active=%d changes=%d state=%d activeChanges=%d",
		xuiDockPanelGetWindowCount(pDemo->pDock),
		xuiDockPanelGetPaneCount(pDemo->pDock),
		xuiDockPanelGetPaneActiveWindow(pDemo->pDock, pDemo->iDocPane),
		xuiDockPanelGetChangeCount(pDemo->pDock),
		pDemo->iStateChanged,
		pDemo->iActiveChanged);
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiDockCreateUi(xui_dock_demo_t* pDemo)
{
	xui_dock_panel_desc_t desc;
	int ret;
	ret = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( ret != XUI_OK ) return ret;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiDockRootRender, pDemo);
	ret = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( ret != XUI_OK ) return ret;
	if ( __xuiDockAddLabel(pDemo, "XUI DockPanel", (xui_rect_t){28.0f, 18.0f, 220.0f, 24.0f}, NULL) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockAddLabel(pDemo, "windows=0 panes=0 active=0 changes=0 state=0 activeChanges=0", (xui_rect_t){28.0f, 520.0f, 780.0f, 24.0f}, &pDemo->pStatus) != XUI_OK ) return XUI_ERROR;
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = pDemo->pFont;
	ret = xuiDockPanelCreate(pDemo->pContext, &pDemo->pDock, &desc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pDemo->pDock, (xui_rect_t){28.0f, 52.0f, 844.0f, 452.0f});
	ret = xuiWidgetAddChild(pDemo->pRoot, pDemo->pDock);
	if ( ret != XUI_OK ) return ret;
	(void)xuiDockPanelSetWindowStateChanged(pDemo->pDock, __xuiDockStateChanged, pDemo);
	(void)xuiDockPanelSetActiveChanged(pDemo->pDock, __xuiDockActiveChanged, pDemo);
	(void)xuiDockPanelSetWindowClose(pDemo->pDock, __xuiDockClosed, pDemo);
	if ( __xuiDockCreateContent(pDemo, 0, "Document.c", "Primary document surface hosted as a dock window.", XUI_COLOR_RGBA(47, 125, 214, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 1, "Preview.h", "Second document in the same DockPane tab group.", XUI_COLOR_RGBA(74, 151, 118, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 2, "Scene.json", "Additional document used to exercise the overflow tab menu.", XUI_COLOR_RGBA(120, 98, 206, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 3, "Readme.md", "Document tab with a longer label.", XUI_COLOR_RGBA(214, 132, 70, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 4, "Notes.txt", "Another document tab for overflow selection.", XUI_COLOR_RGBA(83, 142, 188, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 5, "Shader.fx", "Overflow tab target with syntax content.", XUI_COLOR_RGBA(47, 137, 186, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 6, "Config.ini", "Overflow tab target for configuration.", XUI_COLOR_RGBA(169, 116, 192, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 7, "Tasks.todo", "Overflow tab target for task notes.", XUI_COLOR_RGBA(96, 153, 92, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 8, "Toolbox", "Tool window docked to the left split.", XUI_COLOR_RGBA(120, 98, 206, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 9, "Properties", "Tool window that can float inside the XUI root.", XUI_COLOR_RGBA(214, 132, 70, 255)) != XUI_OK ) return XUI_ERROR;
	if ( __xuiDockCreateContent(pDemo, 10, "Output", "Bottom tool window for logs and build messages.", XUI_COLOR_RGBA(83, 142, 188, 255)) != XUI_OK ) return XUI_ERROR;
	ret = xuiDockPanelAddWindow(pDemo->pDock, "Document.c", pDemo->arrContentWidget[0], &pDemo->iDoc);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Preview.h", pDemo->arrContentWidget[1], &pDemo->iPreview);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Scene.json", pDemo->arrContentWidget[2], &pDemo->iScene);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Readme.md", pDemo->arrContentWidget[3], &pDemo->iReadme);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Notes.txt", pDemo->arrContentWidget[4], &pDemo->iNotes);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Shader.fx", pDemo->arrContentWidget[5], &pDemo->iShader);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Config.ini", pDemo->arrContentWidget[6], &pDemo->iConfig);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Tasks.todo", pDemo->arrContentWidget[7], &pDemo->iTasks);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Toolbox", pDemo->arrContentWidget[8], &pDemo->iToolbox);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Properties", pDemo->arrContentWidget[9], &pDemo->iProperties);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pDemo->pDock, "Output", pDemo->arrContentWidget[10], &pDemo->iOutput);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDockPanelDockWindow(pDemo->pDock, pDemo->iDoc, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iPreview, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iScene, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iReadme, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iNotes, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iShader, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iConfig, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pDemo->pDock, pDemo->iTasks, pDemo->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelSetPaneActiveWindow(pDemo->pDock, pDemo->iDocPane, pDemo->iPreview);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pDemo->pDock, pDemo->iToolbox, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.22f, &pDemo->iToolboxPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pDemo->pDock, pDemo->iProperties, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.24f, NULL);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pDemo->pDock, pDemo->iOutput, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.24f, NULL);
	__xuiDockUpdateStatus(pDemo);
	return ret;
}

static int __xuiDockCreateAssets(xui_dock_demo_t* pDemo)
{
	xui_surface_desc_t surfaceDesc;
	const char* sFontPath;
	int ret;
	pDemo->tProxy = xuiProxyXge();
	ret = xuiCreate(&pDemo->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( ret != XUI_OK ) return ret;
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	surfaceDesc.iWidth = DEMO_TARGET_W;
	surfaceDesc.iHeight = DEMO_TARGET_H;
	surfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	surfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	ret = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &surfaceDesc);
	if ( ret != XUI_OK ) return ret;
	sFontPath = __xuiDockFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	ret = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( ret != XUI_OK ) return ret;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiDockCreateUi(pDemo);
}

static void __xuiDockDestroyAssets(xui_dock_demo_t* pDemo)
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

static uint32_t __xuiDockReadButtons(void)
{
	uint32_t buttons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) buttons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) buttons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) buttons |= XUI_POINTER_BUTTON_MIDDLE;
	return buttons;
}

static int __xuiDockFrameFail(const char* sStage, int iRet)
{
	printf("xui_dockpanel frame failed at %s: %d\n", sStage != NULL ? sStage : "unknown", iRet);
	return iRet;
}

static int __xuiDockHandleInput(xui_dock_demo_t* pDemo)
{
	float x;
	float y;
	float wheelX;
	float wheelY;
	uint32_t buttons;
	uint32_t pressed;
	uint32_t released;
	int ret;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	xgeMouseGet(&x, &y);
	xgeMouseGetWheel(&wheelX, &wheelY);
	x -= DEMO_OFFSET_X;
	y -= DEMO_OFFSET_Y;
	buttons = __xuiDockReadButtons();
	if ( !pDemo->bHasMouse || x != pDemo->fLastMouseX || y != pDemo->fLastMouseY || buttons != pDemo->iLastButtons ) {
		ret = xuiInputPointerMove(pDemo->pContext, x, y, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( wheelX != 0.0f || wheelY != 0.0f ) {
		ret = xuiInputPointerWheel(pDemo->pContext, x, y, wheelX, wheelY, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	pressed = buttons & ~pDemo->iLastButtons;
	released = pDemo->iLastButtons & ~buttons;
	if ( (pressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerDown(pDemo->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (released & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerUp(pDemo->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = x;
	pDemo->fLastMouseY = y;
	pDemo->iLastButtons = buttons;
	return XUI_OK;
}

static void __xuiDockRunChecks(xui_dock_demo_t* pDemo, int bExercise)
{
	xui_dock_window_info_t info;
	xui_dock_pane_info_t pane;
	xui_dock_drop_info_t drop;
	xui_widget menu;
	pDemo->bCreateOK = (pDemo->pDock != NULL) && (xuiDockPanelGetWindowCount(pDemo->pDock) == 11);
	pDemo->bLayoutOK = (xuiDockPanelGetPaneInfo(pDemo->pDock, pDemo->iDocPane, &pane) == XUI_OK) && (pane.tClientRect.fW > 100.0f) && (pane.tClientRect.fH > 100.0f);
	if ( bExercise && !pDemo->bExerciseDone ) {
		(void)xuiDockPanelSetPaneActiveWindow(pDemo->pDock, pDemo->iDocPane, pDemo->iDoc);
		pDemo->bTabOK = (xuiDockPanelGetPaneActiveWindow(pDemo->pDock, pDemo->iDocPane) == pDemo->iDoc);
		(void)xuiDockPanelOpenOverflowMenu(pDemo->pDock, pDemo->iDocPane);
		menu = xuiDockPanelGetOverflowMenu(pDemo->pDock);
		pDemo->bMenuOK = (menu != NULL) && xuiMenuIsOpen(menu) && (xuiMenuGetItemCount(menu) == xuiDockPanelGetPaneWindowCount(pDemo->pDock, pDemo->iDocPane));
		(void)xuiMenuClose(menu);
		(void)xuiDockPanelOpenPaneMenu(pDemo->pDock, pDemo->iToolboxPane);
		menu = xuiDockPanelGetOptionMenu(pDemo->pDock);
		pDemo->bMenuOK = pDemo->bMenuOK && (menu != NULL) && xuiMenuIsOpen(menu) && (xuiMenuGetItemCount(menu) >= 4);
		(void)xuiMenuClose(menu);
		(void)xuiDockPanelSetWindowFlags(pDemo->pDock, pDemo->iToolbox, 1, 0);
		memset(&drop, 0, sizeof(drop));
		(void)xuiDockPanelGetPaneInfo(pDemo->pDock, pDemo->iDocPane, &pane);
		(void)xuiDockPanelFindDropTarget(pDemo->pDock, pDemo->iToolbox, pane.tRect.fX + pane.tRect.fW * 0.5f, pane.tRect.fY + pane.tRect.fH * 0.5f, &drop);
		pDemo->bDockableOK = !drop.bValid &&
			(xuiDockPanelFloatWindow(pDemo->pDock, pDemo->iToolbox, (xui_rect_t){96.0f, 72.0f, 190.0f, 135.0f}) == XUI_ERROR_INVALID_ARGUMENT) &&
			(xuiDockPanelAutoHideWindow(pDemo->pDock, pDemo->iToolbox) == XUI_ERROR_INVALID_ARGUMENT);
		(void)xuiDockPanelSetWindowFlags(pDemo->pDock, pDemo->iToolbox, 1, 1);
		(void)xuiDockPanelFloatWindow(pDemo->pDock, pDemo->iProperties, (xui_rect_t){548.0f, 92.0f, 260.0f, 170.0f});
		(void)xuiDockPanelGetWindowInfo(pDemo->pDock, pDemo->iProperties, &info);
		pDemo->bFloatOK = (info.iState == XUI_DOCK_PANEL_WINDOW_FLOATING);
		(void)xuiDockPanelGetPaneInfo(pDemo->pDock, pDemo->iDocPane, &pane);
		memset(&drop, 0, sizeof(drop));
		(void)xuiDockPanelFindDropTarget(pDemo->pDock, pDemo->iProperties, pane.tRect.fX + pane.tRect.fW - 8.0f, pane.tRect.fY + pane.tRect.fH * 0.5f, &drop);
		pDemo->bDragDockOK = drop.bValid && drop.iPane == pDemo->iDocPane && drop.iSide == XUI_DOCK_PANEL_SIDE_RIGHT;
		if ( pDemo->bDragDockOK ) {
			(void)xuiDockPanelDockWindowToPaneSide(pDemo->pDock, pDemo->iProperties, drop.iPane, drop.iSide, 0.25f, NULL);
			(void)xuiLayout(pDemo->pContext);
			(void)xuiDockPanelGetWindowInfo(pDemo->pDock, pDemo->iProperties, &info);
			pDemo->bDragDockOK = (info.iState == XUI_DOCK_PANEL_WINDOW_DOCKED) && (info.iPane >= 0) && (info.iPane != pDemo->iDocPane);
		}
		(void)xuiDockPanelAutoHideWindow(pDemo->pDock, pDemo->iToolbox);
		(void)xuiLayout(pDemo->pContext);
		(void)xuiDockPanelExpandAutoHideWindow(pDemo->pDock, pDemo->iToolbox);
		(void)xuiLayout(pDemo->pContext);
		(void)xuiDockPanelGetWindowInfo(pDemo->pDock, pDemo->iToolbox, &info);
		pane.tRect = xuiDockPanelGetAutoHideExpandRect(pDemo->pDock);
		pDemo->bAutoHideOK = (info.iState == XUI_DOCK_PANEL_WINDOW_AUTO_HIDE) &&
			(xuiDockPanelGetAutoHideExpandedWindow(pDemo->pDock) == pDemo->iToolbox) &&
			(pane.tRect.fW > 0.0f) && xuiWidgetGetVisible(info.pHostWidget);
		(void)xuiDockPanelDockAutoHideWindow(pDemo->pDock, pDemo->iToolbox);
		(void)xuiLayout(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	} else if ( !bExercise ) {
		pDemo->bTabOK = 1;
		pDemo->bFloatOK = 1;
		pDemo->bDragDockOK = 1;
		pDemo->bAutoHideOK = 1;
		pDemo->bMenuOK = 1;
		pDemo->bDockableOK = 1;
	}
	pDemo->bPaintOK = (xuiDockPanelGetChangeCount(pDemo->pDock) > 0);
	__xuiDockUpdateStatus(pDemo);
}

static int __xuiDockFrame(void* pUser)
{
	xui_dock_demo_t* pDemo = (xui_dock_demo_t*)pUser;
	xui_render_stats_t stats;
	xui_rect_i_t fullRect;
	xui_rect_t src;
	xui_rect_t dst;
	int autoRun;
	int ret;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	autoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	ret = xgeBegin();
	if ( ret != XGE_OK ) return __xuiDockFrameFail("xgeBegin", ret);
	ret = __xuiDockHandleInput(pDemo);
	if ( ret != XUI_OK ) return __xuiDockFrameFail("handleInput", ret);
	ret = xuiDispatchPendingEvents(pDemo->pContext);
	if ( ret != XUI_OK ) return __xuiDockFrameFail("dispatchEvents", ret);
	ret = xuiLayout(pDemo->pContext);
	if ( ret != XUI_OK ) return __xuiDockFrameFail("layout", ret);
	ret = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( ret != XUI_OK ) return __xuiDockFrameFail("update", ret);
	__xuiDockRunChecks(pDemo, autoRun);
	ret = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(232, 241, 250, 255));
	if ( ret != XUI_OK ) return __xuiDockFrameFail("surfaceClear", ret);
	fullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	ret = xuiRender(pDemo->pContext, pDemo->pTarget, &fullRect, 1);
	if ( ret != XUI_OK ) return __xuiDockFrameFail("render", ret);
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	src = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	dst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	ret = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, src, dst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( ret == XUI_OK ) ret = xgeEnd();
	if ( ret != XGE_OK ) return __xuiDockFrameFail("present", ret);
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&stats, 0, sizeof(stats));
		(void)xuiGetRenderStats(pDemo->pContext, &stats);
		printf("xui_dockpanel final-summary frames=%d create=%d layout=%d tab=%d menu=%d dockable=%d float=%d dragdock=%d autohide=%d paint=%d panes=%d changes=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bTabOK, pDemo->bMenuOK, pDemo->bDockableOK,
			pDemo->bFloatOK, pDemo->bDragDockOK, pDemo->bAutoHideOK, pDemo->bPaintOK, xuiDockPanelGetPaneCount(pDemo->pDock),
			xuiDockPanelGetChangeCount(pDemo->pDock), stats.iUpdatedCaches, stats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_dock_demo_t demo;
	xge_desc_t desc;
	int ret;
	memset(&demo, 0, sizeof(demo));
	ret = __xuiDockParseArgs(&demo, argc, argv);
	if ( ret == 1 ) return 0;
	if ( ret != XGE_OK ) {
		__xuiDockUsage();
		return 1;
	}
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_TARGET_W + 20;
	desc.iHeight = DEMO_TARGET_H + 50;
	desc.sTitle = "XUI DockPanel";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) {
		printf("xui_dockpanel: xgeInit failed: %d\n", ret);
		return 1;
	}
	ret = __xuiDockCreateAssets(&demo);
	if ( ret != XUI_OK ) {
		printf("xui_dockpanel: create assets failed: %d\n", ret);
		__xuiDockDestroyAssets(&demo);
		xgeUnit();
		return 1;
	}
	ret = xgeRun(__xuiDockFrame, &demo);
	__xuiDockDestroyAssets(&demo);
	xgeUnit();
	return (ret == XGE_OK && demo.bCreateOK && demo.bLayoutOK && demo.bTabOK && demo.bMenuOK && demo.bDockableOK && demo.bFloatOK && demo.bDragDockOK && demo.bAutoHideOK && demo.bPaintOK) ? 0 : 1;
}
