#include "ui_design_workbench.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#endif

static xstrview __view(const char* text) { return (xstrview){text, strlen(text)}; }
static xvalue* __get(const xvalue* value, const char* key) { return xrtValueObjectGet(value, __view(key)); }

int uiDesignWorkbenchLoadFont(ui_design_app_t* app)
{
	int ret;
	if ( app->fFontSize <= 0 ) app->fFontSize = 18.0f;
	if ( app->sFontPath[0] == 0 ) {
#if defined(_WIN32)
		wchar_t wide[UI_DESIGN_PATH_CAPACITY];
		char directory[UI_DESIGN_PATH_CAPACITY];
		char* slash;
		DWORD n = GetModuleFileNameW(NULL, wide, UI_DESIGN_PATH_CAPACITY);
		if ( n > 0 && n < UI_DESIGN_PATH_CAPACITY && WideCharToMultiByte(CP_UTF8, 0, wide, -1, directory, sizeof(directory), NULL, NULL) ) {
			slash = strrchr(directory, '\\');
			if ( slash != NULL ) {
				*slash = 0;
				if ( strlen(directory) + 20 < sizeof(app->sFontPath) ) snprintf(app->sFontPath, sizeof(app->sFontPath), "%s/font/UIDesign.ttf", directory);
			}
		}
		if ( !uiDesignFileStamp(app->sFontPath).bPresent ) {
			app->sFontPath[0] = 0;
			n = GetWindowsDirectoryW(wide, UI_DESIGN_PATH_CAPACITY);
			if ( n > 0 && n < UI_DESIGN_PATH_CAPACITY && WideCharToMultiByte(CP_UTF8, 0, wide, -1, directory, sizeof(directory), NULL, NULL) && strlen(directory) + 20 < sizeof(app->sFontPath) ) {
				snprintf(app->sFontPath, sizeof(app->sFontPath), "%s/Fonts/msyh.ttc", directory);
				if ( !uiDesignFileStamp(app->sFontPath).bPresent ) snprintf(app->sFontPath, sizeof(app->sFontPath), "%s/Fonts/simhei.ttf", directory);
			}
		}
#endif
	}
	ret = app->tProxy.fontLoadFile(&app->tProxy, &app->pFont, app->sFontPath, app->fFontSize, XUI_FONT_FORMAT_TTF);
	if ( ret == XUI_OK ) {
		(void)xuiSetDefaultFont(app->pContext, app->pFont);
		printf("xui_uidesign font=%s size=%.0f\n", app->sFontPath, app->fFontSize);
	}
	return ret;
}

static int __designerLayout(xui_widget widget, xui_rect_t rect, void* user)
{
	ui_design_app_t* app = (ui_design_app_t*)user;
	int height = rect.fH < 36 ? rect.fH : 36;
	int ret = xuiLayoutArrangeChild(widget, app->pDocumentTabs, (xui_rect_t){rect.fX, rect.fY, rect.fW, height});
	if ( ret == XUI_OK ) ret = xuiLayoutArrangeChild(widget, app->pCanvas, (xui_rect_t){rect.fX, rect.fY + height, rect.fW, rect.fH - height});
	return ret;
}

static void __tabSelect(xui_widget widget, int index, void* user)
{
	ui_design_app_t* app = (ui_design_app_t*)user;
	(void)widget;
	if ( !app->bSyncingTabs ) { app->iPendingTab = index; xgeRenderRequest(); }
}

static void __tabClose(xui_widget widget, int index, void* user)
{
	ui_design_app_t* app = (ui_design_app_t*)user;
	(void)widget;
	if ( !app->bSyncingTabs ) { app->iPendingCloseTab = index; xgeRenderRequest(); }
}

void uiDesignWorkbenchRefreshTabs(ui_design_app_t* app)
{
	char labels[UI_DESIGN_SESSION_LIMIT][UI_DESIGN_PATH_CAPACITY + 32];
	const char* items[UI_DESIGN_SESSION_LIMIT];
	int dirty[UI_DESIGN_SESSION_LIMIT], i, selected = 0, changed;
	if ( app->pDocumentTabs == NULL ) return;
	changed = xuiTabsGetItemCount(app->pDocumentTabs) != app->iSessionCount;
	for ( i = 0; i < app->iSessionCount; ++i ) {
		ui_design_session_t* session = app->arrSessions[i];
		const char* name = session->sDocumentPath;
		const char* slash;
		for ( slash = name; *slash; ++slash ) if ( *slash == '/' || *slash == '\\' ) name = slash + 1;
		if ( *name ) snprintf(labels[i], sizeof(labels[i]), "%s%s", name, session->bExternalChange ? " [disk!]" : "");
		else snprintf(labels[i], sizeof(labels[i]), "Untitled %llu", (unsigned long long)session->iId);
		items[i] = labels[i];
		dirty[i] = session->bDocumentDirty;
		if ( !changed && strcmp(xuiTabsGetItemText(app->pDocumentTabs, i), labels[i]) != 0 ) changed = 1;
		if ( session == app->pSession ) selected = i;
	}
	app->bSyncingTabs = 1;
	if ( changed ) (void)xuiTabsSetItems(app->pDocumentTabs, items, app->iSessionCount);
	if ( changed ) for ( i = 0; i < app->iSessionCount; ++i ) {
		(void)xuiWidgetSetTooltipText(xuiTabsGetButtonWidget(app->pDocumentTabs, i), app->arrSessions[i]->sDocumentPath);
	}
	(void)xuiTabsSetDirtyItems(app->pDocumentTabs, dirty, app->iSessionCount);
	(void)xuiTabsSetSelected(app->pDocumentTabs, selected);
	app->bSyncingTabs = 0;
}

int uiDesignWorkbenchCreateTabs(ui_design_app_t* app)
{
	xui_tabs_desc_t desc = {0};
	int ret = xuiWidgetCreate(app->pContext, &app->pDesignerHost);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(app->pDesignerHost, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetLayoutChildrenCallback(app->pDesignerHost, __designerLayout, app);
	desc.iSize = sizeof(desc);
	desc.pFont = app->pFont;
	desc.bCloseButtons = desc.bScrollable = 1;
	desc.fTabHeight = 34;
	desc.fTabWidth = 196;
	ret = xuiTabsCreate(app->pContext, &app->pDocumentTabs, &desc);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(app->pDesignerHost, app->pDocumentTabs);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(app->pDesignerHost, app->pCanvas);
	if ( ret != XUI_OK ) return ret;
	(void)xuiTabsSetSelect(app->pDocumentTabs, __tabSelect, app);
	(void)xuiTabsSetClose(app->pDocumentTabs, __tabClose, 1, app);
	uiDesignWorkbenchRefreshTabs(app);
	return XUI_OK;
}

/* Translate only window references, never pane indexes or other numeric values.
 * The persisted format uses stable names rather than registration-order IDs. */
static xvalue* __panelRef(ui_design_app_t* app, const xvalue* value, int encode)
{
	static const char* names[] = {"toolbox", "designer", "inspector"};
	int ids[] = {app->iToolboxWindow, app->iCanvasWindow, app->iInspectorWindow};
	int i;
	int64 id;
	xstrview name;
	for ( i = 0; i < 3; ++i ) {
		if ( encode && xrtValueGetInt(value, &id) && id == ids[i] ) return xrtValueString(__view(names[i]));
		if ( !encode && xrtValueGetString(value, &name) && name.Size == strlen(names[i]) && memcmp(name.Data, names[i], name.Size) == 0 ) return xrtValueInt(ids[i]);
	}
	return NULL;
}

static int __mapLayout(ui_design_app_t* app, xvalue* value, int encode, int depth)
{
	size_t i, j;
	xstrview key;
	if ( depth > 32 ) return 0;
	if ( xrtValueType(value) == XVALUE_OBJECT ) {
		for ( i = 0; i < xrtValueCount(value); ++i ) {
			xvalue* child = xrtValueObjectAt(value, i, &key);
			if ( key.Size == 2 && memcmp(key.Data, "id", 2) == 0 ) {
				xvalue* replacement = __panelRef(app, child, encode);
				if ( replacement == NULL || !xrtValueObjectSetNew(value, key, replacement) ) return 0;
			} else if ( key.Size == 4 && memcmp(key.Data, "tabs", 4) == 0 ) {
				if ( xrtValueType(child) != XVALUE_ARRAY ) return 0;
				for ( j = 0; j < xrtValueCount(child); ++j ) {
					xvalue* replacement = __panelRef(app, xrtValueArrayGet(child, j), encode);
					if ( replacement == NULL || !xrtValueArraySetNew(child, j, replacement) ) return 0;
				}
			} else if ( !__mapLayout(app, child, encode, depth + 1) ) return 0;
		}
	} else if ( xrtValueType(value) == XVALUE_ARRAY ) {
		for ( i = 0; i < xrtValueCount(value); ++i ) if ( !__mapLayout(app, xrtValueArrayGet(value, i), encode, depth + 1) ) return 0;
	}
	return 1;
}

int uiDesignWorkbenchSaveLayout(ui_design_app_t* app)
{
	xvalue* state = NULL;
	char* text = NULL;
	int ret;
	if ( !app->sWorkspacePath[0] || app->bWorkspaceLoadFailed ) return XUI_OK;
	ret = xuiDockPanelSaveState(app->pDock, &state);
	if ( ret != XUI_OK ) return ret;
	if ( !__mapLayout(app, state, 1, 0) || !xrtValueObjectSetNew(state, __view("uidesignVersion"), xrtValueInt(1)) ) ret = XUI_ERROR_INVALID_ARGUMENT;
	if ( ret == XUI_OK ) text = xrtXsonStringify(state, TRUE, NULL);
	xuiDockPanelStateFree(state);
	if ( ret != XUI_OK || text == NULL ) return XUI_ERROR_RESOURCE_FAILED;
	if ( app->sLastLayout == NULL || strcmp(app->sLastLayout, text) != 0 ) {
		ret = uiDesignFileWrite(app->sWorkspacePath, text);
		if ( ret == XUI_OK ) { xrtFree(app->sLastLayout); app->sLastLayout = text; text = NULL; }
		else uiDesignAppSetStatus(app, "Workspace save failed; will retry. Documents are unaffected.");
	}
	xrtFree(text);
	return ret;
}

int uiDesignWorkbenchLoadLayout(ui_design_app_t* app)
{
	char* text;
	xvalue* state;
	int64 version;
	int ret = XUI_ERROR_INVALID_ARGUMENT;
	xui_dock_window_info_t info = {0};
	if ( !app->sWorkspacePath[0] || !uiDesignFileStamp(app->sWorkspacePath).bPresent ) return XUI_OK;
	text = uiDesignFileRead(app->sWorkspacePath, 1024u * 1024u);
	state = text ? xrtXsonParse(__view(text)) : NULL;
	free(text);
	if ( state && xrtValueGetInt(__get(state, "uidesignVersion"), &version) && version == 1 && __mapLayout(app, state, 0, 0) ) ret = xuiDockPanelLoadState(app->pDock, state);
	xrtValueRelease(state);
	if ( ret == XUI_OK ) {
		info.iSize = sizeof(info);
		ret = xuiDockPanelGetWindowInfo(app->pDock, app->iCanvasWindow, &info);
		if ( ret == XUI_OK && info.iState == XUI_DOCK_PANEL_WINDOW_HIDDEN ) ret = XUI_ERROR_INVALID_ARGUMENT;
		if ( ret == XUI_OK ) app->iCanvasPane = info.iPane;
	}
	if ( ret != XUI_OK ) {
		app->bWorkspaceLoadFailed = 1;
		(void)xuiDockPanelLoadState(app->pDock, app->pDefaultLayout);
		uiDesignAppSetStatus(app, "Invalid workspace kept on disk. View > Reset Layout to replace it.");
	}
	info.iSize = sizeof(info);
	if ( xuiDockPanelGetWindowInfo(app->pDock, app->iCanvasWindow, &info) == XUI_OK ) app->iCanvasPane = info.iPane;
	(void)xuiDockPanelSetWindowFlags(app->pDock, app->iCanvasWindow, 0, 1);
	return ret;
}

int uiDesignWorkbenchResetLayout(ui_design_app_t* app)
{
	xui_dock_window_info_t info = {0};
	int ret = xuiDockPanelLoadState(app->pDock, app->pDefaultLayout);
	if ( ret != XUI_OK ) return ret;
	info.iSize = sizeof(info);
	if ( xuiDockPanelGetWindowInfo(app->pDock, app->iCanvasWindow, &info) == XUI_OK ) app->iCanvasPane = info.iPane;
	app->bWorkspaceLoadFailed = 0;
	xrtFree(app->sLastLayout); app->sLastLayout = NULL;
	ret = uiDesignWorkbenchSaveLayout(app);
	uiDesignAppUpdateCommandUI(app);
	uiDesignAppInvalidate(app);
	return ret;
}

static int __hotkey(xui_widget widget, const xui_event_t* event, void* user)
{
	ui_design_app_t* app = (ui_design_app_t*)user;
	ui_design_command_t command = UI_DESIGN_COMMAND_NONE;
	int design = xuiGetFocusWidget(app->pContext) == app->pOverlay;
	(void)widget;
	if ( event->iKey == XUI_KEY_ESCAPE ) {
		if ( design ) { uiDesignAppCancelGesture(app); return XUI_EVENT_DISPATCH_STOP; }
		return XUI_OK;
	}
	if ( event->iModifiers & XUI_MOD_CTRL ) {
		switch ( event->iKey ) {
		case 'N': command = UI_DESIGN_COMMAND_FILE_NEW; break;
		case 'O': command = UI_DESIGN_COMMAND_FILE_OPEN; break;
		case 'S': command = event->iModifiers & XUI_MOD_SHIFT ? UI_DESIGN_COMMAND_FILE_SAVE_AS : UI_DESIGN_COMMAND_FILE_SAVE; break;
		case 'W': command = UI_DESIGN_COMMAND_FILE_CLOSE; break;
		case 'Q': command = UI_DESIGN_COMMAND_FILE_EXIT; break;
		default: break;
		}
		if ( design ) switch ( event->iKey ) {
		case 'Z': command = event->iModifiers & XUI_MOD_SHIFT ? UI_DESIGN_COMMAND_EDIT_REDO : UI_DESIGN_COMMAND_EDIT_UNDO; break;
		case 'Y': command = UI_DESIGN_COMMAND_EDIT_REDO; break;
		case 'A': command = UI_DESIGN_COMMAND_EDIT_SELECT_ALL; break;
		case 'X': command = UI_DESIGN_COMMAND_EDIT_CUT; break;
		case 'C': command = UI_DESIGN_COMMAND_EDIT_COPY; break;
		case 'V': command = UI_DESIGN_COMMAND_EDIT_PASTE; break;
		case 'D': command = UI_DESIGN_COMMAND_EDIT_DUPLICATE; break;
		default: break;
		}
	} else if ( design && event->iKey == XUI_KEY_DELETE ) command = UI_DESIGN_COMMAND_EDIT_DELETE;
	if ( command == UI_DESIGN_COMMAND_NONE || !uiDesignAppCanExecuteCommand(app, command) ) return XUI_OK;
	if ( uiDesignAppExecuteCommand(app, command) != XUI_OK && !app->pSession->bExternalChange ) uiDesignAppSetStatus(app, "Command failed; document retained");
	return XUI_EVENT_DISPATCH_STOP;
}

#if defined(_WIN32)
static LRESULT CALLBACK __windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR data)
{
	ui_design_app_t* app = (ui_design_app_t*)data;
	(void)id;
	if ( message == WM_CLOSE ) { app->bNativeCloseRequested = 1; xgeRenderRequest(); return 0; }
	return DefSubclassProc(window, message, wParam, lParam);
}
#endif

int uiDesignWorkbenchInit(ui_design_app_t* app)
{
	const char* keys = "NOSWQZYAXCVD";
	int ret;
	app->iPendingTab = app->iPendingCloseTab = -1;
	ret = xuiDockPanelSaveState(app->pDock, &app->pDefaultLayout);
	if ( ret != XUI_OK ) return ret;
#if defined(_WIN32)
	if ( !app->bWorkspaceExplicit && app->iMaxFrames <= 0 && app->fMaxSeconds <= 0 ) {
		wchar_t path[UI_DESIGN_PATH_CAPACITY];
		DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", path, UI_DESIGN_PATH_CAPACITY);
		if ( n > 0 && n + 40 < UI_DESIGN_PATH_CAPACITY ) {
			wcscat(path, L"\\XUI"); (void)CreateDirectoryW(path, NULL);
			wcscat(path, L"\\UIDesign"); (void)CreateDirectoryW(path, NULL);
			wcscat(path, L"\\workspace.xson");
			(void)WideCharToMultiByte(CP_UTF8, 0, path, -1, app->sWorkspacePath, sizeof(app->sWorkspacePath), NULL, NULL);
		}
	}
#endif
	(void)uiDesignWorkbenchLoadLayout(app);
	app->iLastLayoutChange = xuiDockPanelGetLayoutChangeCount(app->pDock);
	for ( ; *keys; ++keys ) {
		ret = xuiHotKeyRegister(app->pContext, app->pRoot, *keys, XUI_MOD_CTRL, __hotkey, app);
		if ( ret != XUI_OK ) return ret;
	}
	(void)xuiHotKeyRegister(app->pContext, app->pRoot, 'S', XUI_MOD_CTRL | XUI_MOD_SHIFT, __hotkey, app);
	(void)xuiHotKeyRegister(app->pContext, app->pRoot, 'Z', XUI_MOD_CTRL | XUI_MOD_SHIFT, __hotkey, app);
	(void)xuiHotKeyRegister(app->pContext, app->pRoot, XUI_KEY_DELETE, 0, __hotkey, app);
	(void)xuiHotKeyRegister(app->pContext, app->pRoot, XUI_KEY_ESCAPE, 0, __hotkey, app);
	return XUI_OK;
}

int uiDesignWorkbenchTick(ui_design_app_t* app)
{
	double now = xgeTimer();
	int count = xuiDockPanelGetLayoutChangeCount(app->pDock);
#if defined(_WIN32)
	/* Sokol creates the native window in xgeRun, after asset initialization. */
	if ( !app->pNativeWindow && xgePlatformNativeHandle() ) {
		app->pNativeWindow = xgePlatformNativeHandle();
		if ( !SetWindowSubclass((HWND)app->pNativeWindow, __windowProc, 1, (DWORD_PTR)app) ) return XUI_ERROR_RESOURCE_FAILED;
	}
#endif
	uiDesignAppPollSessions(app);
	if ( count != app->iLastLayoutChange ) {
		app->iLastLayoutChange = count;
		app->fNextWorkspaceCheck = now + 0.5;
	}
	if ( app->fNextWorkspaceCheck > 0 && now >= app->fNextWorkspaceCheck ) {
		app->fNextWorkspaceCheck = uiDesignWorkbenchSaveLayout(app) == XUI_OK ? 0 : now + 3.0;
	}
	if ( app->fNextWorkspaceCheck > now ) xgeRenderRequestAfter((float)(app->fNextWorkspaceCheck - now));
	if ( now >= app->fNextDiskCheck ) {
		int i, changed = 0;
		for ( i = 0; i < app->iSessionCount; ++i ) {
			ui_design_session_t* session = app->arrSessions[i];
			if ( session->bDiskKnown && !session->bExternalChange && !uiDesignFileStampEqual(session->tDisk, uiDesignFileStamp(session->sDocumentPath)) ) {
				session->bExternalChange = 1;
				changed = 1;
			}
		}
		if ( changed ) {
			uiDesignWorkbenchRefreshTabs(app);
			uiDesignAppSetStatus(app, "File changed/deleted on disk. Use File > Reload or Save As.");
		}
		app->fNextDiskCheck = now + 1.0;
	}
	xgeRenderRequestAfter((float)(app->fNextDiskCheck - now));
	return XUI_OK;
}

void uiDesignWorkbenchShutdown(ui_design_app_t* app)
{
#if defined(_WIN32)
	if ( app->pNativeWindow ) (void)RemoveWindowSubclass((HWND)app->pNativeWindow, __windowProc, 1);
#endif
	if ( app->pDock && app->pDefaultLayout ) (void)uiDesignWorkbenchSaveLayout(app);
	xuiDockPanelStateFree(app->pDefaultLayout); app->pDefaultLayout = NULL;
	xrtFree(app->sLastLayout); app->sLastLayout = NULL;
}

int uiDesignWorkbenchScreenshot(ui_design_app_t* app)
{
	int ret;
	unsigned char* pixels;
	if ( app->sScreenshotPath[0] == 0 ) return XUI_OK;
	pixels = (unsigned char*)malloc((size_t)app->iSurfaceWidth * app->iSurfaceHeight * 4);
	if ( pixels == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	ret = app->tProxy.surfaceReadRGBA(&app->tProxy, app->pTarget, pixels, app->iSurfaceWidth * 4);
	if ( ret == XUI_OK ) ret = xgeImageSavePNG(app->sScreenshotPath, app->iSurfaceWidth, app->iSurfaceHeight, pixels, app->iSurfaceWidth * 4);
	free(pixels);
	return ret;
}
