#include "pxe_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
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

/* ─── Default palette (PICO-8 inspired) ─── */
static const uint32_t g_arrDefaultPalette[] = {
	XUI_COLOR_RGBA(0,0,0,255), XUI_COLOR_RGBA(29,43,83,255), XUI_COLOR_RGBA(126,37,83,255), XUI_COLOR_RGBA(0,135,81,255),
	XUI_COLOR_RGBA(171,82,54,255), XUI_COLOR_RGBA(95,87,79,255), XUI_COLOR_RGBA(194,195,199,255), XUI_COLOR_RGBA(255,241,232,255),
	XUI_COLOR_RGBA(255,0,77,255), XUI_COLOR_RGBA(255,163,0,255), XUI_COLOR_RGBA(255,236,39,255), XUI_COLOR_RGBA(0,228,54,255),
	XUI_COLOR_RGBA(41,173,255,255), XUI_COLOR_RGBA(131,118,156,255), XUI_COLOR_RGBA(255,119,168,255), XUI_COLOR_RGBA(255,204,170,255)
};
#define PXE_DEFAULT_PALETTE_COUNT 16

/* ─── Helpers ─── */
static int pxe_arg_int(const char* s, int def) { int v; if (!s || !s[0]) return def; v = atoi(s); return v > 0 ? v : def; }

static void pxe_set_working_dir(void)
{
#ifdef _WIN32
	char path[MAX_PATH]; char* sl;
	if (!GetModuleFileNameA(NULL, path, sizeof(path))) return;
	path[sizeof(path)-1] = 0;
	sl = strrchr(path, '\\'); if (!sl) sl = strrchr(path, '/');
	if (sl) { *sl = 0; SetCurrentDirectoryA(path); }
#endif
}

static uint32_t pxe_read_buttons(void)
{
	uint32_t b = 0;
	if (xgeMouseDown(XGE_MOUSE_LEFT)) b |= XUI_POINTER_BUTTON_LEFT;
	if (xgeMouseDown(XGE_MOUSE_RIGHT)) b |= XUI_POINTER_BUTTON_RIGHT;
	if (xgeMouseDown(XGE_MOUSE_MIDDLE)) b |= XUI_POINTER_BUTTON_MIDDLE;
	return b;
}

static uint32_t pxe_read_modifiers(void)
{
	uint32_t m = 0;
	if (xgeKeyDown(XGE_KEY_LEFT_SHIFT) || xgeKeyDown(XGE_KEY_RIGHT_SHIFT)) m |= XUI_MOD_SHIFT;
	if (xgeKeyDown(XGE_KEY_LEFT_CONTROL) || xgeKeyDown(XGE_KEY_RIGHT_CONTROL)) m |= XUI_MOD_CTRL;
	if (xgeKeyDown(XGE_KEY_LEFT_ALT) || xgeKeyDown(XGE_KEY_RIGHT_ALT)) m |= XUI_MOD_ALT;
	return m;
}

/* ─── Resize target ─── */
static int pxe_resize_target(pxe_app_t* pApp, int w, int h)
{
	xui_surface_desc_t sd;
	if (w <= 0) w = PXE_W;
	if (h <= 0) h = PXE_H;
	if (pApp->pTarget && pApp->iTargetW == w && pApp->iTargetH == h) return XUI_OK;
	if (pApp->pTarget) { pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget); pApp->pTarget = NULL; }
	memset(&sd, 0, sizeof(sd));
	sd.iKind = XUI_SURFACE_KIND_TEXTURE;
	sd.iWidth = w; sd.iHeight = h;
	sd.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	sd.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	if (pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &sd) != XUI_OK) return XUI_ERROR;
	pApp->iTargetW = w; pApp->iTargetH = h;
	return XUI_OK;
}

/* ─── Sync viewport ─── */
int PixelEditSyncViewport(pxe_app_t* pApp)
{
	int w = xgeGetWidth(), h = xgeGetHeight();
	float fw, fh, contentY, contentH;
	if (w <= 0) w = PXE_W;
	if (h <= 0) h = PXE_H;
	fw = (float)w; fh = (float)h;
	if (pxe_resize_target(pApp, w, h) != XUI_OK) return XUI_ERROR;
	(void)xuiInputViewport(pApp->pContext, fw, fh);
	(void)xuiSetViewportSize(pApp->pContext, fw, fh);
	if (pApp->pRoot) (void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0,0,fw,fh});
	if (pApp->pMenuBar) (void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0,0,fw,PXE_MENU_H});
	contentY = PXE_MENU_H + PXE_TOOLBAR_H;
	contentH = fh - contentY - PXE_STATUS_H;
	if (contentH < 1.0f) contentH = 1.0f;
	if (pApp->pToolbar) (void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0,PXE_MENU_H,fw,PXE_TOOLBAR_H});
	if (pApp->pDock) (void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){0,contentY,fw,contentH});
	if (pApp->pStatus) (void)xuiWidgetSetRect(pApp->pStatus, (xui_rect_t){0,fh-PXE_STATUS_H,fw,PXE_STATUS_H});
	return XUI_OK;
}

/* ─── Input ─── */
int PixelEditHandleInput(pxe_app_t* pApp)
{
	static const int keys[] = { XGE_KEY_ENTER, XGE_KEY_TAB, XGE_KEY_SPACE, XGE_KEY_BACKSPACE, XGE_KEY_DELETE, XGE_KEY_LEFT, XGE_KEY_RIGHT, XGE_KEY_UP, XGE_KEY_DOWN, XGE_KEY_ESCAPE };
	float x, y, wx, wy;
	uint32_t buttons, pressed, released, mods;
	int i;

	mods = pxe_read_modifiers();
	(void)xuiInputSetModifiers(pApp->pContext, mods);
	if (xgeKeyPressed(XGE_KEY_ESCAPE)) xgeQuit();
	for (i = 0; i < (int)(sizeof(keys)/sizeof(keys[0])); i++) {
		if (xgeKeyPressed(keys[i])) { (void)xuiInputKeyDown(pApp->pContext, keys[i], mods); }
		if (xgeKeyReleased(keys[i])) { (void)xuiInputKeyUp(pApp->pContext, keys[i], mods); }
	}
	while ((mods = xgeTextGet()) != 0) { (void)xuiInputText(pApp->pContext, mods); }

	xgeMouseGet(&x, &y);
	xgeMouseGetWheel(&wx, &wy);
	buttons = pxe_read_buttons();
	if (!pApp->bMouse || x != pApp->fMouseX || y != pApp->fMouseY || buttons != pApp->iButtons)
		(void)xuiInputPointerMove(pApp->pContext, x, y, buttons);
	if (wx != 0.0f || wy != 0.0f)
		(void)xuiInputPointerWheel(pApp->pContext, x, y, wx, wy, buttons);
	pressed = buttons & ~pApp->iButtons;
	released = pApp->iButtons & ~buttons;
	if (pressed & XUI_POINTER_BUTTON_LEFT) (void)xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
	if (released & XUI_POINTER_BUTTON_LEFT) (void)xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
	if (pressed & XUI_POINTER_BUTTON_RIGHT) (void)xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_RIGHT, buttons);
	if (released & XUI_POINTER_BUTTON_RIGHT) (void)xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_RIGHT, buttons);
	pApp->bMouse = 1; pApp->fMouseX = x; pApp->fMouseY = y; pApp->iButtons = buttons;
	return XUI_OK;
}

/* ─── Menu select ─── */
static void pxe_menu_select(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	(void)pWidget; (void)iIndex;
	if (!pApp) return;
	switch (iValue) {
	case PXE_CMD_EXIT: xgeQuit(); break;
	case PXE_CMD_EXPORT_PNG: PixelEditExportPNG(pApp, "output/pixel.png"); break;
	case PXE_CMD_ABOUT: (void)xuiStatusBarSetItemText(pApp->pStatus, 0, "PixelEdit v1.0 - XUI2"); break;
	default: break;
	}
}

/* ─── Toolbar select ─── */
static void pxe_toolbar_select(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	(void)pWidget; (void)iIndex;
	if (!pApp) return;
	if (iValue >= 0 && iValue < PXE_TOOL_COUNT) {
		pApp->iTool = iValue;
		if (pApp->pCanvasWidget) (void)xuiWidgetInvalidate(pApp->pCanvasWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
}

/* ─── Create UI ─── */
int PixelEditCreateUI(pxe_app_t* pApp)
{
	xui_cache_policy_t policy;
	xui_menubar_desc_t mbd;
	xui_menu_desc_t md;
	xui_menubar_item_t mbItems[3];
	xui_menu_item_t fileItems[4];
	xui_menu_item_t editItems[3];
	xui_menu_item_t helpItems[1];
	xui_toolbar_desc_t tbd;
	xui_toolbar_item_t tbItems[PXE_TOOL_COUNT];
	xui_statusbar_desc_t sbd;
	xui_dock_panel_desc_t dd;
	const char* sFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
	int ret, win, pane;

	pApp->tProxy = xuiProxyXge();
	ret = xuiCreate(&pApp->pContext);
	if (ret != XUI_OK) return ret;
	ret = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if (ret != XUI_OK) return ret;
	(void)xuiInputViewport(pApp->pContext, (float)PXE_W, (float)PXE_H);
	ret = pxe_resize_target(pApp, PXE_W, PXE_H);
	if (ret != XUI_OK) return ret;

	if (pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) {
		sFontPath = "C:\\Windows\\Fonts\\arial.ttf";
		if (pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return XUI_ERROR;
	}
	(void)xuiSetDefaultFont(pApp->pContext, pApp->pFont);

	/* Root */
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if (ret != XUI_OK) return ret;
	memset(&policy, 0, sizeof(policy));
	policy.iSize = sizeof(policy);
	policy.iPolicy = XUI_CACHE_POLICY_SELF;
	policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	policy.iClearColor = XUI_COLOR_RGBA(0,0,0,0);
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0,0,(float)PXE_W,(float)PXE_H});
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(pApp->pRoot, &policy);
	ret = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if (ret != XUI_OK) return ret;

	/* Menu */
	memset(&mbd, 0, sizeof(mbd)); mbd.iSize = sizeof(mbd); mbd.pFont = pApp->pFont;
	ret = xuiMenuBarCreate(pApp->pContext, &pApp->pMenuBar, &mbd);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0,0,(float)PXE_W,PXE_MENU_H});
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pMenuBar);
	memset(&md, 0, sizeof(md)); md.iSize = sizeof(md); md.pOwner = pApp->pMenuBar; md.pFont = pApp->pFont;
	(void)xuiMenuCreate(pApp->pContext, &pApp->pFileMenu, &md);
	(void)xuiMenuCreate(pApp->pContext, &pApp->pEditMenu, &md);
	(void)xuiMenuCreate(pApp->pContext, &pApp->pHelpMenu, &md);
	memset(fileItems, 0, sizeof(fileItems));
	fileItems[0] = (xui_menu_item_t){"导出 PNG", "Ctrl+E", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_EXPORT_PNG, 0, NULL, NULL};
	fileItems[1] = (xui_menu_item_t){"导出精灵条", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_EXPORT_STRIP, 0, NULL, NULL};
	fileItems[2] = (xui_menu_item_t){NULL, NULL, XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL};
	fileItems[3] = (xui_menu_item_t){"退出", "Alt+F4", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_EXIT, 0, NULL, NULL};
	memset(editItems, 0, sizeof(editItems));
	editItems[0] = (xui_menu_item_t){"清空画布", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_CLEAR, 0, NULL, NULL};
	editItems[1] = (xui_menu_item_t){NULL, NULL, XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL};
	editItems[2] = (xui_menu_item_t){"撤销", "Ctrl+Z", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_UNDO, 0, NULL, NULL};
	helpItems[0] = (xui_menu_item_t){"关于", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, PXE_CMD_ABOUT, 0, NULL, NULL};
	(void)xuiMenuSetItems(pApp->pFileMenu, fileItems, 4);
	(void)xuiMenuSetItems(pApp->pEditMenu, editItems, 3);
	(void)xuiMenuSetItems(pApp->pHelpMenu, helpItems, 1);
	(void)xuiMenuSetSelect(pApp->pFileMenu, pxe_menu_select, pApp);
	(void)xuiMenuSetSelect(pApp->pEditMenu, pxe_menu_select, pApp);
	(void)xuiMenuSetSelect(pApp->pHelpMenu, pxe_menu_select, pApp);
	mbItems[0] = (xui_menubar_item_t){"文件", XUI_MENUBAR_ITEM_ENABLED, 0, 'F', pApp->pFileMenu, {0,0,0,0}, NULL};
	mbItems[1] = (xui_menubar_item_t){"编辑", XUI_MENUBAR_ITEM_ENABLED, 0, 'E', pApp->pEditMenu, {0,0,0,0}, NULL};
	mbItems[2] = (xui_menubar_item_t){"帮助", XUI_MENUBAR_ITEM_ENABLED, 0, 'H', pApp->pHelpMenu, {0,0,0,0}, NULL};
	(void)xuiMenuBarSetItems(pApp->pMenuBar, mbItems, 3);
	(void)xuiMenuBarSetSelect(pApp->pMenuBar, pxe_menu_select, pApp);

	/* Toolbar */
	memset(&tbd, 0, sizeof(tbd)); tbd.iSize = sizeof(tbd); tbd.pFont = pApp->pFont;
	memset(tbItems, 0, sizeof(tbItems));
	tbItems[0] = (xui_toolbar_item_t){"铅笔", "Pencil (P)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_PENCIL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[1] = (xui_toolbar_item_t){"橡皮", "Eraser (E)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_ERASER, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[2] = (xui_toolbar_item_t){"填充", "Fill (F)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_FILL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[3] = (xui_toolbar_item_t){"取色", "Picker (I)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_PICKER, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[4] = (xui_toolbar_item_t){"直线", "Line (L)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_LINE, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[5] = (xui_toolbar_item_t){"矩形", "Rect (R)", XUI_TOOLBAR_ITEM_TOGGLE, 0, PXE_TOOL_RECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbd.pItems = tbItems; tbd.iItemCount = PXE_TOOL_COUNT;
	ret = xuiToolbarCreate(pApp->pContext, &pApp->pToolbar, &tbd);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0,PXE_MENU_H,(float)PXE_W,PXE_TOOLBAR_H});
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pToolbar);
	(void)xuiToolbarSetSelect(pApp->pToolbar, pxe_toolbar_select, pApp);

	/* Status */
	memset(&sbd, 0, sizeof(sbd)); sbd.iSize = sizeof(sbd); sbd.pFont = pApp->pFont;
	ret = xuiStatusBarCreate(pApp->pContext, &pApp->pStatus, &sbd);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(pApp->pStatus, (xui_rect_t){0,(float)PXE_H-PXE_STATUS_H,(float)PXE_W,PXE_STATUS_H});
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pStatus);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "就绪", 200.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "32x32", 80.0f, 0, 0);
	(void)xuiStatusBarAddFlexibleSpacer(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, 1.0f);

	/* Dock */
	memset(&dd, 0, sizeof(dd)); dd.iSize = sizeof(dd); dd.pFont = pApp->pFont;
	ret = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &dd);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){0,PXE_MENU_H+PXE_TOOLBAR_H,(float)PXE_W,(float)PXE_H-PXE_MENU_H-PXE_TOOLBAR_H-PXE_STATUS_H});
	(void)xuiWidgetAddChild(pApp->pRoot, pApp->pDock);

	/* Canvas widget (center) */
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pCanvasWidget);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetCacheRenderCallback(pApp->pCanvasWidget, PixelEditCanvasRender, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pCanvasWidget, XUI_EVENT_POINTER_DOWN, PixelEditCanvasEvent, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pCanvasWidget, XUI_EVENT_POINTER_MOVE, PixelEditCanvasEvent, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pCanvasWidget, XUI_EVENT_POINTER_UP, PixelEditCanvasEvent, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pCanvasWidget, XUI_EVENT_POINTER_WHEEL, PixelEditCanvasEvent, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pCanvasWidget, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiDockPanelAddWindow(pApp->pDock, "画布", pApp->pCanvasWidget, &win);
	(void)xuiDockPanelDockWindow(pApp->pDock, win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pane);

	/* Palette widget (right) */
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pPaletteWidget);
	if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetCacheRenderCallback(pApp->pPaletteWidget, PixelEditPaletteRender, pApp);
	(void)xuiWidgetSetEventHandler(pApp->pPaletteWidget, XUI_EVENT_POINTER_DOWN, PixelEditPaletteEvent, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pPaletteWidget, XUI_EVENT_MASK_POINTER, 1);
	(void)xuiDockPanelAddWindow(pApp->pDock, "调色板", pApp->pPaletteWidget, &win);
	(void)xuiDockPanelDockWindow(pApp->pDock, win, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.15f, NULL);

	pApp->bCreateOK = 1;
	return XUI_OK;
}

/* ─── Destroy ─── */
void PixelEditDestroy(pxe_app_t* pApp)
{
	if (!pApp) return;
	if (pApp->pTarget) { pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget); pApp->pTarget = NULL; }
	if (pApp->pFont) { pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont); pApp->pFont = NULL; }
	if (pApp->pContext) { xuiDestroy(pApp->pContext); pApp->pContext = NULL; }
}

/* ─── Frame ─── */
int PixelEditFrame(void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	xui_rect_i_t full;
	xui_rect_t src, dst;
	int ret;
	if (!pApp) return XGE_ERROR;
	ret = xgeBegin();
	if (ret != XGE_OK) return ret;
	(void)PixelEditSyncViewport(pApp);
	(void)PixelEditHandleInput(pApp);
	(void)xuiDispatchPendingEvents(pApp->pContext);
	(void)xuiLayout(pApp->pContext);
	(void)xuiUpdate(pApp->pContext, xgeGetDelta());
	(void)pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(240,244,248,255));
	full = (xui_rect_i_t){0, 0, pApp->iTargetW, pApp->iTargetH};
	(void)xuiRender(pApp->pContext, pApp->pTarget, &full, 1);
	pApp->bRenderOK = 1;
	xgeClear(XUI_COLOR_RGBA(18,23,32,255));
	src = (xui_rect_t){0,0,(float)pApp->iTargetW,(float)pApp->iTargetH};
	dst = src;
	ret = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, src, dst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if (ret == XUI_OK) ret = xgeEnd();
	if (ret != XGE_OK) return ret;
	pApp->iFrame++;
	if (pApp->iFrameLimit > 0 && pApp->iFrame >= pApp->iFrameLimit) {
		printf("pixeledit_xui2 final-summary frames=%d create=%d render=%d canvas=%dx%d layers=%d tool=%d\n",
			pApp->iFrame, pApp->bCreateOK, pApp->bRenderOK, pApp->iCanvasW, pApp->iCanvasH, pApp->iLayerCount, pApp->iTool);
		xgeQuit();
	}
	return XGE_OK;
}

/* ─── Main ─── */
int main(int argc, char** argv)
{
	static pxe_app_t app;
	xge_desc_t desc;
	int i, ret;

	pxe_set_working_dir();
	memset(&app, 0, sizeof(app));
	app.iCanvasW = 32; app.iCanvasH = 32;
	app.iLayerCount = 1; app.iActiveLayer = 0;
	app.arrLayers[0].bVisible = 1; app.arrLayers[0].iAlpha = 255;
	strcpy(app.arrLayers[0].sName, "Layer 1");
	app.iTool = PXE_TOOL_PENCIL;
	app.iColor = XUI_COLOR_RGBA(0,0,0,255);
	app.iZoom = 16;
	app.iFrameCount = 1; app.iActiveFrame = 0;
	memcpy(app.arrPalette, g_arrDefaultPalette, sizeof(g_arrDefaultPalette));
	app.iPaletteCount = PXE_DEFAULT_PALETTE_COUNT;
	app.iFrameLimit = pxe_arg_int(getenv("XGE_PIXELEDIT_FRAMES"), 0);
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--frames") == 0 && i+1 < argc) app.iFrameLimit = pxe_arg_int(argv[++i], app.iFrameLimit);
		else if (strncmp(argv[i], "--frames=", 9) == 0) app.iFrameLimit = pxe_arg_int(argv[i]+9, app.iFrameLimit);
	}

	memset(&desc, 0, sizeof(desc));
	desc.sTitle = "PixelEdit";
	desc.iWidth = PXE_W; desc.iHeight = PXE_H;
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;
	ret = xgeInit(&desc);
	if (ret != XGE_OK) { printf("pixeledit_xui2: xgeInit failed: %d\n", ret); return 1; }
	ret = PixelEditCreateUI(&app);
	if (ret != XUI_OK) { printf("pixeledit_xui2: create ui failed: %d\n", ret); PixelEditDestroy(&app); xgeUnit(); return 1; }
	ret = xgeRun(PixelEditFrame, &app);
	PixelEditDestroy(&app);
	xgeUnit();
	return (ret == XGE_OK && app.bCreateOK && app.bRenderOK) ? 0 : 1;
}
