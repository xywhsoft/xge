/*
 * af_shell.c - DockPanel / MenuBar / Toolbar / StatusBar setup
 *
 * Creates the main application shell: dockable panel layout,
 * menu bar with all menus, tool toolbar, and status bar.
 */

#include "af_shell.h"
#include "af_tools.h"
#include "af_dialogs.h"
#include "../core/af_app.h"
#include "../core/af_file.h"
#include "../core/af_import.h"
#include "../core/af_export.h"
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* File dialog callbacks                                              */
/* ------------------------------------------------------------------ */

#define AF_FILEDLG_OPEN    1
#define AF_FILEDLG_SAVE_AS 2
#define AF_FILEDLG_IMPORT  3
#define AF_FILEDLG_EXPORT  4

static int s_iFileDialogMode = 0;

static void __afFileDialogResult(xui_file_dialog_t* pDialog, int iResult, const char* sPath, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	(void)pDialog;
	if ( pApp == NULL || iResult != 1 || sPath == NULL || sPath[0] == '\0' ) return;

	switch ( s_iFileDialogMode ) {
	case AF_FILEDLG_OPEN:
		afAppOpenDocument(pApp, sPath);
		break;
	case AF_FILEDLG_SAVE_AS:
		afAppSaveDocument(pApp, sPath);
		break;
	case AF_FILEDLG_IMPORT:
		{
			uint32_t iImportId = 0;
			int iRet = afImportFile(&pApp->tDoc, sPath, &iImportId);
			if ( iRet == 0 ) {
				pApp->tDoc.bModified = 1;
				xgeRenderRequest();
			}
		}
		break;
	case AF_FILEDLG_EXPORT:
		{
			int iRet = afExport(&pApp->tDoc, AF_EXPORT_TYPE_PNG_SEQ, sPath,
			                    0, (int)pApp->tDoc.iFrameCount - 1, 1.0f);
			(void)iRet;
		}
		break;
	default:
		break;
	}
}

static void __afOpenFileDialog(af_app_t* pApp, int iMode)
{
	xui_file_dialog_desc_t desc;
	xui_file_dialog pDlg = NULL;

	s_iFileDialogMode = iMode;
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = pApp->pFont;
	desc.onResult = __afFileDialogResult;
	desc.pResultUser = pApp;
	desc.bModal = 1;

	switch ( iMode ) {
	case AF_FILEDLG_OPEN:
		desc.sTitle = "Open Animation";
		desc.sFilter = "AnimForge (*.xanim)\0*.xanim\0All Files\0*.*\0";
		xuiOpenFileDialog(pApp->pContext, &pDlg, &desc);
		break;
	case AF_FILEDLG_SAVE_AS:
		desc.sTitle = "Save Animation As";
		desc.sFilter = "AnimForge (*.xanim)\0*.xanim\0All Files\0*.*\0";
		desc.sFileName = "untitled.xanim";
		xuiSaveFileDialog(pApp->pContext, &pDlg, &desc);
		break;
	case AF_FILEDLG_IMPORT:
		desc.sTitle = "Import Asset";
		desc.sFilter = "SVG (*.svg)\0*.svg\0PNG (*.png)\0*.png\0All Files\0*.*\0";
		xuiOpenFileDialog(pApp->pContext, &pDlg, &desc);
		break;
	case AF_FILEDLG_EXPORT:
		desc.sTitle = "Export";
		desc.sFilter = "PNG Sequence (*.png)\0*.png\0All Files\0*.*\0";
		xuiSaveFileDialog(pApp->pContext, &pDlg, &desc);
		break;
	default:
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Menu IDs                                                           */
/* ------------------------------------------------------------------ */

#define AF_MENU_FILE_NEW        101
#define AF_MENU_FILE_OPEN       102
#define AF_MENU_FILE_SAVE       103
#define AF_MENU_FILE_SAVE_AS    104
#define AF_MENU_FILE_IMPORT     105
#define AF_MENU_FILE_EXPORT     106
#define AF_MENU_FILE_EXIT       107

#define AF_MENU_EDIT_UNDO       201
#define AF_MENU_EDIT_REDO       202
#define AF_MENU_EDIT_CUT        203
#define AF_MENU_EDIT_COPY       204
#define AF_MENU_EDIT_PASTE      205
#define AF_MENU_EDIT_DELETE     206
#define AF_MENU_EDIT_SELECT_ALL 207

#define AF_MENU_VIEW_GRID       301
#define AF_MENU_VIEW_RULERS     302
#define AF_MENU_VIEW_ONION      303
#define AF_MENU_VIEW_ZOOM_IN    304
#define AF_MENU_VIEW_ZOOM_OUT   305
#define AF_MENU_VIEW_FIT        306

#define AF_MENU_INSERT_KEYFRAME 401
#define AF_MENU_INSERT_BLANK_KF 402
#define AF_MENU_INSERT_LAYER    403
#define AF_MENU_INSERT_SYMBOL   404

#define AF_MENU_MODIFY_TWEEN    501
#define AF_MENU_MODIFY_GROUP    502
#define AF_MENU_MODIFY_UNGROUP  503
#define AF_MENU_MODIFY_ALIGN    504

#define AF_MENU_CONTROL_PLAY    601
#define AF_MENU_CONTROL_STOP    602
#define AF_MENU_CONTROL_LOOP    603
#define AF_MENU_CONTROL_STEP_F  604
#define AF_MENU_CONTROL_STEP_B  605

#define AF_MENU_HELP_ABOUT      701

/* ------------------------------------------------------------------ */
/* Menu bar callback                                                  */
/* ------------------------------------------------------------------ */

static void __afMenuBarSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	(void)pMenu; (void)iIndex;
	if ( pApp == NULL ) return;
	afShellHandleMenuCommand(pApp, iValue);
	xgeRenderRequest();
}

/* ------------------------------------------------------------------ */
/* Toolbar callback                                                   */
/* ------------------------------------------------------------------ */

static void __afToolbarSelected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	(void)pWidget; (void)iIndex;
	if ( pApp == NULL ) return;
	if ( iValue >= 0 && iValue < AF_TOOL_COUNT ) {
		afAppSetTool(pApp, iValue);
		if ( pApp->pToolMgr != NULL ) afToolManagerSetActive(pApp->pToolMgr, iValue);
		afShellRefreshToolbar(pApp);
	}
}

/* ------------------------------------------------------------------ */
/* Toolbar refresh - update checked state for radio group             */
/* ------------------------------------------------------------------ */

void afShellRefreshToolbar(af_app_t* pApp)
{
	xui_toolbar_item_t tbItems[16];
	int i;

	if ( pApp == NULL || pApp->pToolbar == NULL ) return;

	memset(tbItems, 0, sizeof(tbItems));
	tbItems[0]  = (xui_toolbar_item_t){"V", "Select (V)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_SELECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[1]  = (xui_toolbar_item_t){"A", "Sub-Select (A)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_SUBSELECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[2]  = (xui_toolbar_item_t){"P", "Pen (P)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_PEN, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[3]  = (xui_toolbar_item_t){"N", "Line (N)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_LINE, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[4]  = (xui_toolbar_item_t){"R", "Rectangle (R)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_RECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[5]  = (xui_toolbar_item_t){"O", "Ellipse (O)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ELLIPSE, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[6].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
	tbItems[7]  = (xui_toolbar_item_t){"Y", "Pencil (Y)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_PENCIL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[8]  = (xui_toolbar_item_t){"B", "Brush (B)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_BRUSH, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[9]  = (xui_toolbar_item_t){"T", "Text (T)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_TEXT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[10] = (xui_toolbar_item_t){"K", "Fill (K)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_FILL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[11] = (xui_toolbar_item_t){"G", "Gradient (G)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_GRADIENT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[12].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
	tbItems[13] = (xui_toolbar_item_t){"H", "Hand (H)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_HAND, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[14] = (xui_toolbar_item_t){"Z", "Zoom (Z)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ZOOM, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[15] = (xui_toolbar_item_t){"E", "Eraser (E)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ERASER, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};

	/* Set checked on the active tool */
	for ( i = 0; i < 16; i++ ) {
		if ( tbItems[i].iType == XUI_TOOLBAR_ITEM_SEPARATOR ) continue;
		if ( tbItems[i].iValue == pApp->iActiveTool ) {
			tbItems[i].iState |= XUI_TOOLBAR_ITEM_CHECKED;
		}
	}

	(void)xuiToolbarSetItems(pApp->pToolbar, tbItems, 16);
}

/* ------------------------------------------------------------------ */
/* Shell creation - builds menubar, toolbar, statusbar                */
/* ------------------------------------------------------------------ */

int afShellCreate(af_app_t* pApp)
{
	xui_menubar_desc_t mbDesc;
	xui_menubar_item_t mbItems[7];
	xui_menu_desc_t menuDesc;
	xui_menu_item_t fileItems[8];
	xui_menu_item_t editItems[7];
	xui_menu_item_t viewItems[7];
	xui_menu_item_t insertItems[4];
	xui_menu_item_t modifyItems[4];
	xui_menu_item_t controlItems[5];
	xui_menu_item_t helpItems[1];
	xui_toolbar_desc_t tbDesc;
	xui_toolbar_item_t tbItems[16];
	xui_statusbar_desc_t sbDesc;
	xui_widget menuFile, menuEdit, menuView, menuInsert, menuModify, menuControl, menuHelp;
	float fW = (float)AF_APP_WIDTH;
	int ret;

	/* ---- Menu Bar ---- */
	memset(&mbDesc, 0, sizeof(mbDesc));
	mbDesc.iSize = sizeof(mbDesc);
	mbDesc.pFont = pApp->pFont;
	ret = xuiMenuBarCreate(pApp->pContext, &pApp->pMenuBar, &mbDesc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pMenuBar, (xui_rect_t){0.0f, 0.0f, fW, 24.0f});
	ret = xuiWidgetAddChild(pApp->pRoot, pApp->pMenuBar);
	if ( ret != XUI_OK ) return ret;

	/* Create menus */
	memset(&menuDesc, 0, sizeof(menuDesc));
	menuDesc.iSize = sizeof(menuDesc);
	menuDesc.pOwner = pApp->pMenuBar;
	menuDesc.pFont = pApp->pFont;

	/* File menu */
	ret = xuiMenuCreate(pApp->pContext, &menuFile, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(fileItems, 0, sizeof(fileItems));
	fileItems[0] = (xui_menu_item_t){"New", "Ctrl+N", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_NEW, 0, NULL, NULL};
	fileItems[1] = (xui_menu_item_t){"Open...", "Ctrl+O", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_OPEN, 0, NULL, NULL};
	fileItems[2] = (xui_menu_item_t){"Save", "Ctrl+S", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_SAVE, 0, NULL, NULL};
	fileItems[3] = (xui_menu_item_t){"Save As...", "Ctrl+Shift+S", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_SAVE_AS, 0, NULL, NULL};
	fileItems[4].iType = XUI_MENU_ITEM_SEPARATOR;
	fileItems[5] = (xui_menu_item_t){"Import...", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_IMPORT, 0, NULL, NULL};
	fileItems[6] = (xui_menu_item_t){"Export...", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_FILE_EXPORT, 0, NULL, NULL};
	fileItems[7].iType = XUI_MENU_ITEM_SEPARATOR;
	(void)xuiMenuSetItems(menuFile, fileItems, 8);

	/* Edit menu */
	ret = xuiMenuCreate(pApp->pContext, &menuEdit, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(editItems, 0, sizeof(editItems));
	editItems[0] = (xui_menu_item_t){"Undo", "Ctrl+Z", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_UNDO, 0, NULL, NULL};
	editItems[1] = (xui_menu_item_t){"Redo", "Ctrl+Y", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_REDO, 0, NULL, NULL};
	editItems[2].iType = XUI_MENU_ITEM_SEPARATOR;
	editItems[3] = (xui_menu_item_t){"Cut", "Ctrl+X", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_CUT, 0, NULL, NULL};
	editItems[4] = (xui_menu_item_t){"Copy", "Ctrl+C", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_COPY, 0, NULL, NULL};
	editItems[5] = (xui_menu_item_t){"Paste", "Ctrl+V", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_PASTE, 0, NULL, NULL};
	editItems[6] = (xui_menu_item_t){"Delete", "Del", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_EDIT_DELETE, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuEdit, editItems, 7);

	/* View menu */
	ret = xuiMenuCreate(pApp->pContext, &menuView, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(viewItems, 0, sizeof(viewItems));
	viewItems[0] = (xui_menu_item_t){"Grid", "Ctrl+'", XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED, AF_MENU_VIEW_GRID, 0, NULL, NULL};
	viewItems[1] = (xui_menu_item_t){"Rulers", "Ctrl+R", XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, AF_MENU_VIEW_RULERS, 0, NULL, NULL};
	viewItems[2] = (xui_menu_item_t){"Onion Skin", "Ctrl+;", XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED, AF_MENU_VIEW_ONION, 0, NULL, NULL};
	viewItems[3].iType = XUI_MENU_ITEM_SEPARATOR;
	viewItems[4] = (xui_menu_item_t){"Zoom In", "Ctrl+=", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_VIEW_ZOOM_IN, 0, NULL, NULL};
	viewItems[5] = (xui_menu_item_t){"Zoom Out", "Ctrl+-", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_VIEW_ZOOM_OUT, 0, NULL, NULL};
	viewItems[6] = (xui_menu_item_t){"Fit Stage", "Ctrl+0", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_VIEW_FIT, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuView, viewItems, 7);

	/* Insert menu */
	ret = xuiMenuCreate(pApp->pContext, &menuInsert, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(insertItems, 0, sizeof(insertItems));
	insertItems[0] = (xui_menu_item_t){"Keyframe", "F6", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_INSERT_KEYFRAME, 0, NULL, NULL};
	insertItems[1] = (xui_menu_item_t){"Blank Keyframe", "F7", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_INSERT_BLANK_KF, 0, NULL, NULL};
	insertItems[2] = (xui_menu_item_t){"Layer", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_INSERT_LAYER, 0, NULL, NULL};
	insertItems[3] = (xui_menu_item_t){"Symbol...", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_INSERT_SYMBOL, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuInsert, insertItems, 4);

	/* Modify menu */
	ret = xuiMenuCreate(pApp->pContext, &menuModify, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(modifyItems, 0, sizeof(modifyItems));
	modifyItems[0] = (xui_menu_item_t){"Create Motion Tween", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_MODIFY_TWEEN, 0, NULL, NULL};
	modifyItems[1] = (xui_menu_item_t){"Group", "Ctrl+G", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_MODIFY_GROUP, 0, NULL, NULL};
	modifyItems[2] = (xui_menu_item_t){"Ungroup", "Ctrl+Shift+G", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_MODIFY_UNGROUP, 0, NULL, NULL};
	modifyItems[3] = (xui_menu_item_t){"Align...", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_MODIFY_ALIGN, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuModify, modifyItems, 4);

	/* Control menu */
	ret = xuiMenuCreate(pApp->pContext, &menuControl, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(controlItems, 0, sizeof(controlItems));
	controlItems[0] = (xui_menu_item_t){"Play / Stop", "Enter", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_CONTROL_PLAY, 0, NULL, NULL};
	controlItems[1] = (xui_menu_item_t){"Stop", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_CONTROL_STOP, 0, NULL, NULL};
	controlItems[2] = (xui_menu_item_t){"Loop", "L", XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, AF_MENU_CONTROL_LOOP, 0, NULL, NULL};
	controlItems[3] = (xui_menu_item_t){"Step Forward", ".", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_CONTROL_STEP_F, 0, NULL, NULL};
	controlItems[4] = (xui_menu_item_t){"Step Backward", ",", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_CONTROL_STEP_B, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuControl, controlItems, 5);

	/* Help menu */
	ret = xuiMenuCreate(pApp->pContext, &menuHelp, &menuDesc);
	if ( ret != XUI_OK ) return ret;
	memset(helpItems, 0, sizeof(helpItems));
	helpItems[0] = (xui_menu_item_t){"About AnimForge", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, AF_MENU_HELP_ABOUT, 0, NULL, NULL};
	(void)xuiMenuSetItems(menuHelp, helpItems, 1);

	/* Set menubar items */
	memset(mbItems, 0, sizeof(mbItems));
	mbItems[0] = (xui_menubar_item_t){"&File", XUI_MENUBAR_ITEM_ENABLED, 1, 0, menuFile, {0,0,0,0}, NULL};
	mbItems[1] = (xui_menubar_item_t){"&Edit", XUI_MENUBAR_ITEM_ENABLED, 2, 0, menuEdit, {0,0,0,0}, NULL};
	mbItems[2] = (xui_menubar_item_t){"&View", XUI_MENUBAR_ITEM_ENABLED, 3, 0, menuView, {0,0,0,0}, NULL};
	mbItems[3] = (xui_menubar_item_t){"&Insert", XUI_MENUBAR_ITEM_ENABLED, 4, 0, menuInsert, {0,0,0,0}, NULL};
	mbItems[4] = (xui_menubar_item_t){"&Modify", XUI_MENUBAR_ITEM_ENABLED, 5, 0, menuModify, {0,0,0,0}, NULL};
	mbItems[5] = (xui_menubar_item_t){"&Control", XUI_MENUBAR_ITEM_ENABLED, 6, 0, menuControl, {0,0,0,0}, NULL};
	mbItems[6] = (xui_menubar_item_t){"&Help", XUI_MENUBAR_ITEM_ENABLED, 7, 0, menuHelp, {0,0,0,0}, NULL};
	ret = xuiMenuBarSetItems(pApp->pMenuBar, mbItems, 7);
	if ( ret != XUI_OK ) return ret;
	(void)xuiMenuBarSetSelect(pApp->pMenuBar, __afMenuBarSelected, pApp);

	/* ---- Toolbar ---- */
	memset(&tbDesc, 0, sizeof(tbDesc));
	tbDesc.iSize = sizeof(tbDesc);
	tbDesc.pFont = pApp->pFont;
	ret = xuiToolbarCreate(pApp->pContext, &pApp->pToolbar, &tbDesc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0.0f, 24.0f, fW, 32.0f});
	ret = xuiWidgetAddChild(pApp->pRoot, pApp->pToolbar);
	if ( ret != XUI_OK ) return ret;

	memset(tbItems, 0, sizeof(tbItems));
	tbItems[0]  = (xui_toolbar_item_t){"V", "Select (V)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED | XUI_TOOLBAR_ITEM_CHECKED, AF_TOOL_SELECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[1]  = (xui_toolbar_item_t){"A", "Sub-Select (A)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_SUBSELECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[2]  = (xui_toolbar_item_t){"P", "Pen (P)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_PEN, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[3]  = (xui_toolbar_item_t){"N", "Line (N)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_LINE, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[4]  = (xui_toolbar_item_t){"R", "Rectangle (R)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_RECT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[5]  = (xui_toolbar_item_t){"O", "Ellipse (O)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ELLIPSE, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[6].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
	tbItems[7]  = (xui_toolbar_item_t){"Y", "Pencil (Y)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_PENCIL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[8]  = (xui_toolbar_item_t){"B", "Brush (B)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_BRUSH, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[9]  = (xui_toolbar_item_t){"T", "Text (T)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_TEXT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[10] = (xui_toolbar_item_t){"K", "Fill (K)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_FILL, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[11] = (xui_toolbar_item_t){"G", "Gradient (G)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_GRADIENT, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[12].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
	tbItems[13] = (xui_toolbar_item_t){"H", "Hand (H)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_HAND, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[14] = (xui_toolbar_item_t){"Z", "Zoom (Z)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ZOOM, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	tbItems[15] = (xui_toolbar_item_t){"E", "Eraser (E)", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, AF_TOOL_ERASER, 1, NULL, {0,0,0,0}, {0,0,0,0}, NULL};
	(void)xuiToolbarSetItems(pApp->pToolbar, tbItems, 16);
	(void)xuiToolbarSetSelect(pApp->pToolbar, __afToolbarSelected, pApp);

	/* ---- Status Bar ---- */
	memset(&sbDesc, 0, sizeof(sbDesc));
	sbDesc.iSize = sizeof(sbDesc);
	sbDesc.pFont = pApp->pFont;
	ret = xuiStatusBarCreate(pApp->pContext, &pApp->pStatusBar, &sbDesc);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pStatusBar, (xui_rect_t){0.0f, (float)AF_APP_HEIGHT - 22.0f, fW, 22.0f});
	ret = xuiWidgetAddChild(pApp->pRoot, pApp->pStatusBar);
	if ( ret != XUI_OK ) return ret;
	(void)xuiStatusBarAddText(pApp->pStatusBar, 0, "Frame: 0/100", 120.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, 1, "FPS: 24", 70.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, 2, "Zoom: 100%", 90.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, 3, "Tool: Select", 100.0f, 0, 0);
	(void)xuiStatusBarAddText(pApp->pStatusBar, 4, "", 200.0f, 0, 0);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Status bar update                                                  */
/* ------------------------------------------------------------------ */

void afShellUpdateStatusBar(af_app_t* pApp, char* sBuffer, int iBufSize)
{
	float fZoomPct = pApp->tView.fZoom * 100.0f;
	char sFrame[64], sFps[32], sZoom[32], sTool[64], sInfo[128];

	sprintf(sFrame, "Frame: %d/%d", pApp->iCurrentFrame + 1, (int)pApp->tDoc.iFrameCount);
	sprintf(sFps, "FPS: %.0f", pApp->tDoc.fFrameRate);
	sprintf(sZoom, "Zoom: %.0f%%", fZoomPct);
	sprintf(sTool, "Tool: %s", pApp->pToolMgr ? afToolManagerGetActiveName(pApp->pToolMgr) : "None");
	sprintf(sInfo, "%s | %s", pApp->bPlaying ? "Playing" : "Stopped",
	        pApp->tDoc.bModified ? "[Modified]" : "");

	if ( pApp->pStatusBar != NULL ) {
		xuiStatusBarSetItemText(pApp->pStatusBar, 0, sFrame);
		xuiStatusBarSetItemText(pApp->pStatusBar, 1, sFps);
		xuiStatusBarSetItemText(pApp->pStatusBar, 2, sZoom);
		xuiStatusBarSetItemText(pApp->pStatusBar, 3, sTool);
		xuiStatusBarSetItemText(pApp->pStatusBar, 4, sInfo);
	}

	if ( sBuffer != NULL && iBufSize > 0 ) {
		snprintf(sBuffer, iBufSize, "%s | %s | %s | %s | %s", sFrame, sFps, sZoom, sTool, sInfo);
	}
}

/* ------------------------------------------------------------------ */
/* Menu command dispatch                                              */
/* ------------------------------------------------------------------ */

int afShellHandleMenuCommand(af_app_t* pApp, int iCmdId)
{
	switch ( iCmdId ) {
	case AF_MENU_FILE_NEW:
		afDialogOpenNewDocument(pApp);
		return 1;

	case AF_MENU_FILE_SAVE:
		if ( pApp->tDoc.sFilePath[0] )
			afAppSaveDocument(pApp, pApp->tDoc.sFilePath);
		return 1;

	case AF_MENU_VIEW_GRID:
		pApp->tView.bShowGrid = !pApp->tView.bShowGrid;
		return 1;

	case AF_MENU_VIEW_RULERS:
		pApp->tView.bShowRulers = !pApp->tView.bShowRulers;
		return 1;

	case AF_MENU_VIEW_ZOOM_IN:
		pApp->tView.fZoom *= 1.25f;
		if ( pApp->tView.fZoom > 32.0f ) pApp->tView.fZoom = 32.0f;
		return 1;

	case AF_MENU_VIEW_ZOOM_OUT:
		pApp->tView.fZoom *= 0.8f;
		if ( pApp->tView.fZoom < 0.05f ) pApp->tView.fZoom = 0.05f;
		return 1;

	case AF_MENU_CONTROL_PLAY:
		if ( pApp->bPlaying )
			afAppStop(pApp);
		else
			afAppPlay(pApp);
		return 1;

	case AF_MENU_CONTROL_STOP:
		afAppStop(pApp);
		return 1;

	case AF_MENU_CONTROL_LOOP:
		pApp->bLoop = !pApp->bLoop;
		return 1;

	case AF_MENU_CONTROL_STEP_F:
		afAppStepFrame(pApp, 1);
		return 1;

	case AF_MENU_CONTROL_STEP_B:
		afAppStepFrame(pApp, -1);
		return 1;

	case AF_MENU_EDIT_UNDO:
		afUndoUndo(&pApp->tUndo);
		return 1;

	case AF_MENU_EDIT_REDO:
		afUndoRedo(&pApp->tUndo);
		return 1;

	case AF_MENU_INSERT_KEYFRAME:
		if ( pApp->iActiveLayerId != 0 ) {
			afDocSetKeyframe(&pApp->tDoc, pApp->iActiveLayerId, (uint32_t)pApp->iCurrentFrame, 0);
			pApp->tDoc.bModified = 1;
		}
		return 1;

	case AF_MENU_INSERT_BLANK_KF:
		if ( pApp->iActiveLayerId != 0 ) {
			afDocSetKeyframe(&pApp->tDoc, pApp->iActiveLayerId, (uint32_t)pApp->iCurrentFrame, AF_KF_BLANK);
			pApp->tDoc.bModified = 1;
		}
		return 1;

	case AF_MENU_INSERT_LAYER:
		{
			char sName[64];
			snprintf(sName, sizeof(sName), "Layer %d", pApp->tDoc.tTimeline.iLayerCount + 1);
			afDocAddLayer(&pApp->tDoc, sName, AF_LAYER_NORMAL, NULL);
			pApp->tDoc.bModified = 1;
		}
		return 1;

	case AF_MENU_INSERT_SYMBOL:
		{
			uint32_t iNewId = 0;
			char sName[64];
			snprintf(sName, sizeof(sName), "Symbol %d", pApp->tDoc.iSymbolCount + 1);
			afDocAddSymbol(&pApp->tDoc, AF_SYMBOL_GRAPHIC, sName, &iNewId);
			pApp->tDoc.bModified = 1;
		}
		return 1;

	case AF_MENU_MODIFY_TWEEN:
		/* TODO: create motion tween on active layer/frame */
		return 1;

	case AF_MENU_FILE_OPEN:
		__afOpenFileDialog(pApp, AF_FILEDLG_OPEN);
		return 1;

	case AF_MENU_FILE_SAVE_AS:
		__afOpenFileDialog(pApp, AF_FILEDLG_SAVE_AS);
		return 1;

	case AF_MENU_FILE_IMPORT:
		__afOpenFileDialog(pApp, AF_FILEDLG_IMPORT);
		return 1;

	case AF_MENU_FILE_EXPORT:
		__afOpenFileDialog(pApp, AF_FILEDLG_EXPORT);
		return 1;

	case AF_MENU_EDIT_CUT:
	case AF_MENU_EDIT_COPY:
	case AF_MENU_EDIT_PASTE:
	case AF_MENU_EDIT_DELETE:
		/* TODO: clipboard operations */
		return 1;

	case AF_MENU_MODIFY_GROUP:
	case AF_MENU_MODIFY_UNGROUP:
	case AF_MENU_MODIFY_ALIGN:
		/* TODO: modify operations */
		return 1;

	case AF_MENU_VIEW_ONION:
		pApp->tView.bShowGrid = !pApp->tView.bShowGrid; /* reuse grid toggle for onion visual */
		return 1;

	case AF_MENU_VIEW_FIT:
		pApp->tView.fZoom = 1.0f;
		pApp->tView.fOffsetX = 0.0f;
		pApp->tView.fOffsetY = 0.0f;
		return 1;

	case AF_MENU_HELP_ABOUT:
		afDialogOpenAbout(pApp);
		return 1;

	default:
		break;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* Keyboard shortcut mapping                                          */
/* ------------------------------------------------------------------ */

int afShellHandleShortcut(af_app_t* pApp, int iKey, uint32_t iMods)
{
	int ctrl = (iMods & 0x0002) != 0;  /* XGE_KEY_MOD_CTRL */
	int shift = (iMods & 0x0001) != 0; /* XGE_KEY_MOD_SHIFT */

	/* Tool shortcuts (no modifiers) */
	if ( !ctrl && !shift ) {
		int iTool = -1;
		switch ( iKey ) {
		case 'V': iTool = AF_TOOL_SELECT; break;
		case 'A': iTool = AF_TOOL_SUBSELECT; break;
		case 'P': iTool = AF_TOOL_PEN; break;
		case 'N': iTool = AF_TOOL_LINE; break;
		case 'R': iTool = AF_TOOL_RECT; break;
		case 'O': iTool = AF_TOOL_ELLIPSE; break;
		case 'Y': iTool = AF_TOOL_PENCIL; break;
		case 'B': iTool = AF_TOOL_BRUSH; break;
		case 'T': iTool = AF_TOOL_TEXT; break;
		case 'E': iTool = AF_TOOL_ERASER; break;
		case 'K': iTool = AF_TOOL_FILL; break;
		case 'I': iTool = AF_TOOL_EYEDROPPER; break;
		case 'G': iTool = AF_TOOL_GRADIENT; break;
		case 'H': iTool = AF_TOOL_HAND; break;
		case 'Z': iTool = AF_TOOL_ZOOM; break;
		default: break;
		}
		if ( iTool >= 0 ) {
			afAppSetTool(pApp, iTool);
			if ( pApp->pToolMgr != NULL ) afToolManagerSetActive(pApp->pToolMgr, iTool);
			afShellRefreshToolbar(pApp);
			return 1;
		}
	}

	/* Ctrl shortcuts */
	if ( ctrl && !shift ) {
		switch ( iKey ) {
		case 'N': return afShellHandleMenuCommand(pApp, AF_MENU_FILE_NEW);
		case 'S': return afShellHandleMenuCommand(pApp, AF_MENU_FILE_SAVE);
		case 'Z': return afShellHandleMenuCommand(pApp, AF_MENU_EDIT_UNDO);
		case 'Y': return afShellHandleMenuCommand(pApp, AF_MENU_EDIT_REDO);
		case 'G': return afShellHandleMenuCommand(pApp, AF_MENU_MODIFY_GROUP);
		default: break;
		}
	}

	return 0;
}
