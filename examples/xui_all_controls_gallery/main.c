#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIcon;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pBasics;
	xge_xui_widget pInputs;
	xge_xui_widget pValues;
	xge_xui_widget pData;
	xge_xui_widget pLayout;
	xge_xui_widget pOverlayOwner;
	xge_xui_widget pPopup;
	xge_xui_widget pPopupLabel;
	xge_xui_widget pDialog;
	xge_xui_widget pMessage;
	xge_xui_widget pToast;
	xge_xui_widget pSplit;
	xge_xui_widget pScroll;
	xge_xui_widget pMenuOwner;
	xge_xui_widget pTooltipOwner;
	xge_xui_widget pPopupOwner;
	xge_xui_widget pDialogOwner;
	xge_xui_widget pMessageOwner;
	xge_xui_widget pToastOwner;
	xge_xui_widget pWindow;
	xge_xui_widget pBasicWidgets[10];
	xge_xui_widget pInputWidgets[5];
	xge_xui_widget pValueWidgets[11];
	xge_xui_widget pDataWidgets[9];
	xge_xui_widget pLabelWidgets[64];
	xge_xui_label_t tStatusLabel;
	xge_xui_panel_t tBasicsPanel;
	xge_xui_panel_t tInputsPanel;
	xge_xui_panel_t tValuesPanel;
	xge_xui_panel_t tDataPanel;
	xge_xui_panel_t tLayoutPanel;
	xge_xui_label_t tLabels[64];
	xge_xui_button_t tButton;
	xge_xui_button_t tIconTextButton;
	xge_xui_button_t tMenuButton;
	xge_xui_button_t tPopupButton;
	xge_xui_button_t tDialogButton;
	xge_xui_button_t tMessageButton;
	xge_xui_button_t tToastButton;
	xge_xui_button_t tIconAction;
	xge_xui_image_t tImage;
	xge_xui_separator_t tSeparator;
	xge_xui_input_t tInput;
	xge_xui_input_t tSearchInput;
	xge_xui_numeric_input_t tNumeric;
	xge_xui_text_edit_t tTextEdit;
	xge_xui_color_picker_t tColorPicker;
	xge_xui_button_t tChoiceButton;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_toggle_t tToggle;
	xge_xui_slider_t tSlider;
	xge_xui_progress_t tProgress;
	xge_xui_scrollbar_t tScrollbar;
	xge_xui_toolbar_t tToolbar;
	xge_xui_status_bar_t tStatusBar;
	xge_xui_tabs_t tTabs;
	xge_xui_list_view_t tList;
	xge_xui_virtual_list_t tVirtualList;
	xge_xui_tree_view_t tTree;
	xge_xui_table_view_t tTable;
	xge_xui_property_grid_t tPropertyGrid;
	xge_xui_breadcrumb_t tBreadcrumb;
	xge_xui_accordion_t tAccordion;
	xge_xui_combo_box_t tCombo;
	xge_xui_split_layout_t tSplit;
	xge_xui_scroll_view_t tScroll;
	xge_xui_window_t tWindow;
	xge_xui_popup_t tPopupCtl;
	xge_xui_dialog_t tDialogCtl;
	xge_xui_message_box_t tMessageBox;
	xge_xui_toast_t tToastCtl;
	xge_xui_menu_t tMenu;
	int bFontReady;
	int bIconReady;
	int iFrameLimit;
	int iFrameCount;
	int iLabelCount;
	int iButtonClicks;
	int iSelectCount;
	int iToggleCount;
	int iSliderCount;
	int iTableRows;
	int iTableCells;
	int iVirtualCreates;
	int iVirtualBinds;
	int bCoreOK;
	int bDataOK;
	int bOverlayOK;
	int bLayoutOK;
} app_state_t;

static const char* g_arrListItems[] = { "List item A", "List item B", "List item C", "Disabled row" };
static const int g_arrListEnabled[] = { 1, 1, 1, 0 };
static const char* g_arrComboItems[] = { "Default", "Compact", "Detailed" };
static const xge_xui_menu_item_t g_arrMenuItems[] = {
	{ "Open", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 0, 0, NULL, NULL },
	{ "Rename", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 1, 0, NULL, NULL },
	{ "Disabled", NULL, XGE_XUI_MENU_ITEM_NORMAL, 0, 2, 0, NULL, NULL },
	{ "Delete", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER, 3, 0, NULL, NULL }
};

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-all-controls-gallery font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-all-controls-gallery font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateIcon(app_state_t* pApp)
{
	uint32_t arrPixels[16 * 16];
	int x;
	int y;

	for ( y = 0; y < 16; y++ ) {
		for ( x = 0; x < 16; x++ ) {
			uint32_t iColor = XGE_COLOR_RGBA(46, 124, 214, 255);
			if ( x < 2 || y < 2 || x > 13 || y > 13 ) {
				iColor = XGE_COLOR_RGBA(162, 174, 190, 255);
			}
			if ( (x >= 5 && x <= 10) || (y >= 5 && y <= 10) ) {
				iColor = XGE_COLOR_RGBA(248, 250, 253, 255);
			}
			arrPixels[y * 16 + x] = iColor;
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIcon, 16, 16, arrPixels) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bIconReady = 1;
	return XGE_OK;
}

static xge_font Font(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static xge_xui_widget NewWidget(xge_xui_widget pParent, xge_rect_t tRect)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetRect(pWidget, tRect);
		xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
		if ( pParent != NULL ) {
			xgeXuiWidgetAdd(pParent, pWidget);
		}
	}
	return pWidget;
}

static xge_xui_label_t* AddLabel(app_state_t* pApp, xge_xui_widget pParent, xge_rect_t tRect, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_label_t* pLabel;

	if ( pApp->iLabelCount >= (int)(sizeof(pApp->tLabels) / sizeof(pApp->tLabels[0])) ) {
		return NULL;
	}
	pWidget = NewWidget(pParent, tRect);
	if ( pWidget == NULL ) {
		return NULL;
	}
	pLabel = &pApp->tLabels[pApp->iLabelCount];
	pApp->pLabelWidgets[pApp->iLabelCount] = pWidget;
	pApp->iLabelCount++;
	xgeXuiLabelInit(pLabel, pWidget, Font(pApp), sText);
	xgeXuiLabelSetColor(pLabel, XGE_COLOR_RGBA(22, 64, 118, 255));
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return pLabel;
}

static void ClickProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iButtonClicks++;
	}
}

static void SelectProc(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
	}
}

static void MenuSelectProc(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	(void)iValue;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
	}
}

static void SliderProc(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSliderCount++;
	}
}

static void OpenMenuProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiMenuOpenForOwner(&pApp->tMenu, pApp->pMenuOwner);
	}
}

static void OpenPopupProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiPopupSetAnchorRect(&pApp->tPopupCtl, pApp->pPopupOwner->tRect);
		xgeXuiPopupSetPlacement(&pApp->tPopupCtl, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
		xgeXuiPopupSetOpen(&pApp->tPopupCtl, 1);
	}
}

static void OpenDialogProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiDialogSetOpen(&pApp->tDialogCtl, 1);
	}
}

static void OpenMessageProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiMessageBoxSetOpen(&pApp->tMessageBox, 1);
	}
}

static void ShowToastProc(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiToastShow(&pApp->tToastCtl, XGE_XUI_TOAST_TYPE_SUCCESS, "Toast", "Button triggered", 3.0f);
	}
}

static int TableCount(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	return (pApp != NULL) ? pApp->iTableRows : 0;
}

static int TableCell(xge_xui_widget pWidget, int iRow, int iColumn, char* sBuffer, int iBufferSize, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iTableCells++;
	}
	if ( iColumn == 0 ) {
		snprintf(sBuffer, (size_t)iBufferSize, "Asset %02d", iRow + 1);
	} else if ( iColumn == 1 ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%s", (iRow & 1) ? "Dirty" : "Ready");
	} else {
		snprintf(sBuffer, (size_t)iBufferSize, "%d", 12 + iRow * 4);
	}
	return XGE_OK;
}

static int VirtualCount(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	return 80;
}

static xge_xui_widget VirtualCreate(xge_xui_widget pListWidget, int iSlot, void* pUser)
{
	app_state_t* pApp;
	xge_xui_widget pWidget;

	(void)pListWidget;
	(void)iSlot;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iVirtualCreates++;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	}
	return pWidget;
}

static void VirtualBind(xge_xui_widget pItemWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pItemWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iVirtualBinds++;
	}
	(void)iIndex;
}

static void AddBasics(app_state_t* pApp)
{
	xge_xui_widget pPanel;

	pPanel = pApp->pBasics;
	AddLabel(pApp, pPanel, (xge_rect_t){ 14.0f, 36.0f, 170.0f, 22.0f }, "Label / Image / Button");

	pApp->pBasicWidgets[0] = NewWidget(pPanel, (xge_rect_t){ 14.0f, 66.0f, 54.0f, 42.0f });
	xgeXuiImageInit(&pApp->tImage, pApp->pBasicWidgets[0], pApp->bIconReady ? &pApp->tIcon : NULL);
	xgeXuiImageSetMode(&pApp->tImage, XGE_XUI_IMAGE_FIT);

	pApp->pBasicWidgets[1] = NewWidget(pPanel, (xge_rect_t){ 82.0f, 72.0f, 116.0f, 30.0f });
	xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pBasicWidgets[1]);
	xgeXuiButtonSetText(&pApp->tButton, Font(pApp), "Button");
	xgeXuiButtonSetClick(&pApp->tButton, ClickProc, pApp);

	pApp->pBasicWidgets[2] = NewWidget(pPanel, (xge_rect_t){ 210.0f, 72.0f, 34.0f, 30.0f });
	xgeXuiButtonInit(&pApp->tIconAction, &pApp->tXui, pApp->pBasicWidgets[2]);
	xgeXuiButtonSetText(&pApp->tIconAction, NULL, "");
	xgeXuiButtonSetIcon(&pApp->tIconAction, pApp->bIconReady ? &pApp->tIcon : NULL, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	xgeXuiButtonSetIconLayout(&pApp->tIconAction, XGE_XUI_BUTTON_ICON_LEFT, 16.0f, 0.0f);
	xgeXuiButtonSetClick(&pApp->tIconAction, ClickProc, pApp);

	pApp->pBasicWidgets[3] = NewWidget(pPanel, (xge_rect_t){ 14.0f, 120.0f, 230.0f, 1.0f });
	xgeXuiSeparatorInit(&pApp->tSeparator, pApp->pBasicWidgets[3]);
	xgeXuiSeparatorSetThickness(&pApp->tSeparator, 1.0f);

	pApp->pBasicWidgets[4] = NewWidget(pPanel, (xge_rect_t){ 14.0f, 136.0f, 150.0f, 30.0f });
	xgeXuiButtonInit(&pApp->tIconTextButton, &pApp->tXui, pApp->pBasicWidgets[4]);
	xgeXuiButtonSetText(&pApp->tIconTextButton, Font(pApp), "icon action");
	xgeXuiButtonSetIcon(&pApp->tIconTextButton, pApp->bIconReady ? &pApp->tIcon : NULL, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	xgeXuiButtonSetIconLayout(&pApp->tIconTextButton, XGE_XUI_BUTTON_ICON_LEFT, 14.0f, 6.0f);
	xgeXuiButtonSetClick(&pApp->tIconTextButton, ClickProc, pApp);
}

static void AddInputs(app_state_t* pApp)
{
	uint32_t arrPalette[5];
	xge_xui_input_decoration_desc_t tClearDecoration;

	pApp->pInputWidgets[0] = NewWidget(pApp->pInputs, (xge_rect_t){ 14.0f, 38.0f, 194.0f, 28.0f });
	xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidgets[0], Font(pApp));
	xgeXuiInputSetText(&pApp->tInput, "input text");
	xgeXuiInputSetPlaceholder(&pApp->tInput, "Input");
	memset(&tClearDecoration, 0, sizeof(tClearDecoration));
	tClearDecoration.iKind = XGE_XUI_INPUT_DECORATION_CLEAR;
	tClearDecoration.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY;
	tClearDecoration.fWidth = 22.0f;
	tClearDecoration.fPadding = 6.0f;
	tClearDecoration.iColor = XGE_COLOR_RGBA(80, 112, 140, 255);
	tClearDecoration.iHoverColor = XGE_COLOR_RGBA(30, 112, 190, 255);
	tClearDecoration.iDisabledColor = XGE_COLOR_RGBA(156, 168, 180, 255);
	(void)xgeXuiInputDecorationAdd(&pApp->tInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tClearDecoration);

	pApp->pInputWidgets[1] = NewWidget(pApp->pInputs, (xge_rect_t){ 222.0f, 38.0f, 194.0f, 28.0f });
	xgeXuiInputInit(&pApp->tSearchInput, &pApp->tXui, pApp->pInputWidgets[1], Font(pApp));
	xgeXuiInputSetText(&pApp->tSearchInput, "assets");
	xgeXuiInputSetPlaceholder(&pApp->tSearchInput, "Search");
	xgeXuiInputDecorationAdd(&pApp->tSearchInput, XGE_XUI_INPUT_DECORATION_SIDE_LEADING, &(xge_xui_input_decoration_desc_t){
		.iKind = XGE_XUI_INPUT_DECORATION_ICON,
		.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS,
		.fWidth = 24.0f,
		.fPadding = 6.0f,
		.iIcon = XGE_XUI_INPUT_ICON_SEARCH,
		.iColor = XGE_COLOR_RGBA(80, 112, 140, 255),
		.iHoverColor = XGE_COLOR_RGBA(30, 112, 190, 255),
		.iDisabledColor = XGE_COLOR_RGBA(156, 168, 180, 255)
	});
	xgeXuiInputDecorationAdd(&pApp->tSearchInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tClearDecoration);

	pApp->pInputWidgets[2] = NewWidget(pApp->pInputs, (xge_rect_t){ 14.0f, 80.0f, 120.0f, 28.0f });
	xgeXuiNumericInputInit(&pApp->tNumeric, &pApp->tXui, pApp->pInputWidgets[2], Font(pApp));
	xgeXuiNumericInputSetRange(&pApp->tNumeric, 0.0f, 100.0f);
	xgeXuiNumericInputSetStep(&pApp->tNumeric, 0.5f);
	xgeXuiNumericInputSetValue(&pApp->tNumeric, 42.5f);

	pApp->pInputWidgets[3] = NewWidget(pApp->pInputs, (xge_rect_t){ 150.0f, 80.0f, 266.0f, 78.0f });
	xgeXuiTextEditInit(&pApp->tTextEdit, &pApp->tXui, pApp->pInputWidgets[3], Font(pApp));
	xgeXuiTextEditSetText(&pApp->tTextEdit, "TextEdit line 1\nline 2 with wrap and cursor");
	xgeXuiTextEditSetLineNumbers(&pApp->tTextEdit, 1, 30.0f);

	pApp->pInputWidgets[4] = NewWidget(pApp->pInputs, (xge_rect_t){ 14.0f, 120.0f, 120.0f, 30.0f });
	xgeXuiColorPickerInit(&pApp->tColorPicker, &pApp->tXui, pApp->pInputWidgets[4], Font(pApp));
	arrPalette[0] = XGE_COLOR_RGBA(46, 124, 214, 255);
	arrPalette[1] = XGE_COLOR_RGBA(43, 184, 150, 255);
	arrPalette[2] = XGE_COLOR_RGBA(244, 187, 68, 255);
	arrPalette[3] = XGE_COLOR_RGBA(224, 92, 92, 255);
	arrPalette[4] = XGE_COLOR_RGBA(142, 116, 220, 255);
	xgeXuiColorPickerSetPalette(&pApp->tColorPicker, arrPalette, 5);
	xgeXuiColorPickerSetHex(&pApp->tColorPicker, "#2E7CD6FF");
}

static void AddValues(app_state_t* pApp)
{
	const char* arrTool[] = { "New", "Pin", "", "Run" };
	int arrToolType[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_BUTTON };
	const char* arrTabs[] = { "Scene", "Assets", "Output" };

	pApp->pValueWidgets[0] = NewWidget(pApp->pValues, (xge_rect_t){ 14.0f, 36.0f, 112.0f, 24.0f });
	xgeXuiButtonInit(&pApp->tChoiceButton, &pApp->tXui, pApp->pValueWidgets[0]);
	xgeXuiButtonSetText(&pApp->tChoiceButton, Font(pApp), "Selectable");
	xgeXuiButtonSetSelectable(&pApp->tChoiceButton, 1);
	xgeXuiButtonSetSelected(&pApp->tChoiceButton, 1);

	pApp->pValueWidgets[1] = NewWidget(pApp->pValues, (xge_rect_t){ 140.0f, 36.0f, 130.0f, 24.0f });
	xgeXuiCheckBoxInit(&pApp->tCheckBox, &pApp->tXui, pApp->pValueWidgets[1]);
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, Font(pApp), "CheckBox");
	xgeXuiCheckBoxSetChecked(&pApp->tCheckBox, 1);

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	pApp->pValueWidgets[2] = NewWidget(pApp->pValues, (xge_rect_t){ 284.0f, 36.0f, 92.0f, 24.0f });
	xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pValueWidgets[2]);
	xgeXuiRadioSetText(&pApp->tRadioA, Font(pApp), "Radio A");
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 1);
	pApp->pValueWidgets[3] = NewWidget(pApp->pValues, (xge_rect_t){ 384.0f, 36.0f, 92.0f, 24.0f });
	xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pValueWidgets[3]);
	xgeXuiRadioSetText(&pApp->tRadioB, Font(pApp), "Radio B");
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 2);
	xgeXuiRadioGroupSetSelected(&pApp->tRadioGroup, 2);

	pApp->pValueWidgets[4] = NewWidget(pApp->pValues, (xge_rect_t){ 14.0f, 72.0f, 130.0f, 26.0f });
	xgeXuiToggleInit(&pApp->tToggle, &pApp->tXui, pApp->pValueWidgets[4]);
	xgeXuiToggleSetInnerText(&pApp->tToggle, Font(pApp), "OFF", "ON");
	xgeXuiToggleSetChecked(&pApp->tToggle, 1);

	pApp->pValueWidgets[5] = NewWidget(pApp->pValues, (xge_rect_t){ 160.0f, 74.0f, 150.0f, 22.0f });
	xgeXuiSliderInit(&pApp->tSlider, &pApp->tXui, pApp->pValueWidgets[5]);
	xgeXuiSliderSetRange(&pApp->tSlider, 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tSlider, 64.0f);
	xgeXuiSliderSetChange(&pApp->tSlider, SliderProc, pApp);

	pApp->pValueWidgets[6] = NewWidget(pApp->pValues, (xge_rect_t){ 328.0f, 74.0f, 150.0f, 22.0f });
	xgeXuiProgressInit(&pApp->tProgress, pApp->pValueWidgets[6]);
	xgeXuiProgressSetRange(&pApp->tProgress, 0.0f, 100.0f);
	xgeXuiProgressSetValue(&pApp->tProgress, 72.0f);
	xgeXuiProgressSetText(&pApp->tProgress, Font(pApp), "72%");

	pApp->pValueWidgets[7] = NewWidget(pApp->pValues, (xge_rect_t){ 14.0f, 112.0f, 226.0f, 18.0f });
	xgeXuiScrollBarInit(&pApp->tScrollbar, &pApp->tXui, pApp->pValueWidgets[7]);
	xgeXuiScrollBarSetOrientation(&pApp->tScrollbar, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiScrollBarSetRange(&pApp->tScrollbar, 0.0f, 100.0f, 24.0f);
	xgeXuiScrollBarSetValue(&pApp->tScrollbar, 32.0f);

	pApp->pValueWidgets[8] = NewWidget(pApp->pValues, (xge_rect_t){ 260.0f, 108.0f, 220.0f, 30.0f });
	xgeXuiToolbarInit(&pApp->tToolbar, &pApp->tXui, pApp->pValueWidgets[8]);
	xgeXuiToolbarSetFont(&pApp->tToolbar, Font(pApp));
	xgeXuiToolbarSetItems(&pApp->tToolbar, arrTool, arrToolType, 4);
	xgeXuiToolbarSetItemSize(&pApp->tToolbar, 50.0f, 24.0f, 10.0f);
	xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 1, 1);
	xgeXuiToolbarSetSelect(&pApp->tToolbar, SelectProc, pApp);

	pApp->pValueWidgets[9] = NewWidget(pApp->pValues, (xge_rect_t){ 14.0f, 146.0f, 210.0f, 28.0f });
	xgeXuiTabsInit(&pApp->tTabs, &pApp->tXui, pApp->pValueWidgets[9]);
	xgeXuiTabsSetFont(&pApp->tTabs, Font(pApp));
	xgeXuiTabsSetItems(&pApp->tTabs, arrTabs, 3);
	xgeXuiTabsSetSelected(&pApp->tTabs, 1);

	pApp->pValueWidgets[10] = NewWidget(pApp->pValues, (xge_rect_t){ 238.0f, 146.0f, 242.0f, 26.0f });
	xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pValueWidgets[10]);
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, Font(pApp));
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 56.0f, 1);
	xgeXuiStatusBarAddProgress(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_CENTER, 0.0f, 100.0f, 64.0f, 88.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "XUI", 44.0f, 0);
}

static void AddData(app_state_t* pApp)
{
	xge_xui_table_view_column_t arrColumns[3];
	int iCat;

	pApp->pDataWidgets[0] = NewWidget(pApp->pData, (xge_rect_t){ 14.0f, 36.0f, 150.0f, 96.0f });
	xgeXuiListViewInit(&pApp->tList, &pApp->tXui, pApp->pDataWidgets[0]);
	xgeXuiListViewSetFont(&pApp->tList, Font(pApp));
	xgeXuiListViewSetItems(&pApp->tList, g_arrListItems, 4);
	xgeXuiListViewSetEnabledItems(&pApp->tList, g_arrListEnabled, 4);
	xgeXuiListViewSetSelected(&pApp->tList, 1);

	pApp->pDataWidgets[1] = NewWidget(pApp->pData, (xge_rect_t){ 178.0f, 36.0f, 134.0f, 96.0f });
	xgeXuiVirtualListInit(&pApp->tVirtualList, &pApp->tXui, pApp->pDataWidgets[1]);
	xgeXuiVirtualListSetItemHeight(&pApp->tVirtualList, 20.0f);
	xgeXuiVirtualListSetAdapter(&pApp->tVirtualList, VirtualCount, VirtualCreate, VirtualBind, pApp);
	xgeXuiVirtualListSetSelected(&pApp->tVirtualList, 4);

	pApp->pDataWidgets[2] = NewWidget(pApp->pData, (xge_rect_t){ 326.0f, 36.0f, 154.0f, 96.0f });
	xgeXuiTreeViewInit(&pApp->tTree, &pApp->tXui, pApp->pDataWidgets[2]);
	xgeXuiTreeViewSetFont(&pApp->tTree, Font(pApp));
	xgeXuiTreeViewSetMetrics(&pApp->tTree, 20.0f, 12.0f);
	xgeXuiTreeViewAddNode(&pApp->tTree, 1, -1, "Project");
	xgeXuiTreeViewAddNode(&pApp->tTree, 2, 1, "src");
	xgeXuiTreeViewAddNode(&pApp->tTree, 3, 1, "examples");
	xgeXuiTreeViewSetNodeExpanded(&pApp->tTree, 1, 1);
	xgeXuiTreeViewSetSelected(&pApp->tTree, 3);

	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 86.0f;
	arrColumns[0].fMinWidth = 50.0f;
	arrColumns[0].iAlign = XGE_TEXT_ALIGN_LEFT;
	arrColumns[1].sTitle = "State";
	arrColumns[1].fWidth = 62.0f;
	arrColumns[1].fMinWidth = 40.0f;
	arrColumns[1].iAlign = XGE_TEXT_ALIGN_CENTER;
	arrColumns[2].sTitle = "Size";
	arrColumns[2].fWidth = 48.0f;
	arrColumns[2].fMinWidth = 36.0f;
	arrColumns[2].iAlign = XGE_TEXT_ALIGN_RIGHT;
	pApp->pDataWidgets[3] = NewWidget(pApp->pData, (xge_rect_t){ 14.0f, 150.0f, 214.0f, 112.0f });
	xgeXuiTableViewInit(&pApp->tTable, &pApp->tXui, pApp->pDataWidgets[3]);
	xgeXuiTableViewSetFont(&pApp->tTable, Font(pApp));
	xgeXuiTableViewSetColumns(&pApp->tTable, arrColumns, 3);
	xgeXuiTableViewSetAdapter(&pApp->tTable, TableCount, TableCell, pApp);
	xgeXuiTableViewSetMetrics(&pApp->tTable, 22.0f, 20.0f);
	xgeXuiTableViewSetSelected(&pApp->tTable, 2);

	pApp->pDataWidgets[4] = NewWidget(pApp->pData, (xge_rect_t){ 244.0f, 150.0f, 236.0f, 112.0f });
	xgeXuiPropertyGridInit(&pApp->tPropertyGrid, &pApp->tXui, pApp->pDataWidgets[4]);
	xgeXuiPropertyGridSetFont(&pApp->tPropertyGrid, Font(pApp));
	xgeXuiPropertyGridSetMetrics(&pApp->tPropertyGrid, 20.0f, 86.0f);
	iCat = xgeXuiPropertyGridAddCategory(&pApp->tPropertyGrid, "Style", 1);
	xgeXuiPropertyGridAddProperty(&pApp->tPropertyGrid, iCat, "Font", "Simsun 12", XGE_XUI_PROPERTY_GRID_EDITOR_TEXT);
	xgeXuiPropertyGridAddProperty(&pApp->tPropertyGrid, iCat, "Radius", "4", XGE_XUI_PROPERTY_GRID_EDITOR_NUMBER);
	xgeXuiPropertyGridAddProperty(&pApp->tPropertyGrid, iCat, "Accent", "#2E7CD6", XGE_XUI_PROPERTY_GRID_EDITOR_COLOR);
	xgeXuiPropertyGridSetSelected(&pApp->tPropertyGrid, 1);

	pApp->pDataWidgets[5] = NewWidget(pApp->pData, (xge_rect_t){ 14.0f, 278.0f, 238.0f, 26.0f });
	xgeXuiBreadcrumbInit(&pApp->tBreadcrumb, &pApp->tXui, pApp->pDataWidgets[5]);
	xgeXuiBreadcrumbSetFont(&pApp->tBreadcrumb, Font(pApp));
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "examples", 1);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "xui", 2);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "gallery", 3);
	xgeXuiBreadcrumbSetSelected(&pApp->tBreadcrumb, 2);

	pApp->pDataWidgets[6] = NewWidget(pApp->pData, (xge_rect_t){ 268.0f, 278.0f, 212.0f, 96.0f });
	xgeXuiAccordionInit(&pApp->tAccordion, &pApp->tXui, pApp->pDataWidgets[6]);
	xgeXuiAccordionSetFont(&pApp->tAccordion, Font(pApp));
	xgeXuiAccordionAddSection(&pApp->tAccordion, "Inputs", "Input decorations, NumericInput, TextEdit", 34.0f, 1, 1);
	xgeXuiAccordionAddSection(&pApp->tAccordion, "Data", "List, Tree, Table, PropertyGrid", 34.0f, 0, 2);

	pApp->pDataWidgets[7] = NewWidget(pApp->pData, (xge_rect_t){ 14.0f, 322.0f, 140.0f, 28.0f });
	xgeXuiComboBoxInit(&pApp->tCombo, &pApp->tXui, pApp->pDataWidgets[7]);
	xgeXuiComboBoxSetFont(&pApp->tCombo, Font(pApp));
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrComboItems, 3);
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);
	xgeXuiComboBoxSetPopupHeight(&pApp->tCombo, 86.0f);
}

static void AddLayoutAndOverlay(app_state_t* pApp)
{
	xge_xui_widget pPane;
	xge_xui_widget pOverlayRoot;

	pOverlayRoot = xgeXuiOverlayRoot(&pApp->tXui);

	pApp->pSplit = NewWidget(pApp->pLayout, (xge_rect_t){ 14.0f, 36.0f, 240.0f, 120.0f });
	xgeXuiSplitLayoutInit(&pApp->tSplit, &pApp->tXui, pApp->pSplit);
	xgeXuiSplitLayoutSetOrientation(&pApp->tSplit, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->tSplit, 2);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tSplit, 0, 0.9f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tSplit, 1, 1.1f);
	xgeXuiSplitLayoutSetDividerSize(&pApp->tSplit, 3.0f);
	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tSplit, 0);
	xgeXuiWidgetSetBackground(pPane, XGE_COLOR_RGBA(236, 240, 246, 255));
	AddLabel(pApp, pPane, (xge_rect_t){ 8.0f, 8.0f, 82.0f, 22.0f }, "Pane A");
	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tSplit, 1);
	xgeXuiWidgetSetBackground(pPane, XGE_COLOR_RGBA(248, 250, 253, 255));
	AddLabel(pApp, pPane, (xge_rect_t){ 8.0f, 8.0f, 82.0f, 22.0f }, "Pane B");

	pApp->pScroll = NewWidget(pApp->pLayout, (xge_rect_t){ 270.0f, 36.0f, 210.0f, 120.0f });
	xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScroll);
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 380.0f, 260.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 36.0f, 28.0f);
	xgeXuiScrollViewSetColors(&pApp->tScroll, XGE_COLOR_RGBA(248, 250, 253, 255), XGE_COLOR_RGBA(226, 236, 246, 220), XGE_COLOR_RGBA(158, 176, 196, 235));
	AddLabel(pApp, pApp->pLayout, (xge_rect_t){ 284.0f, 52.0f, 140.0f, 22.0f }, "ScrollView");

	pApp->pWindow = NewWidget(pOverlayRoot, (xge_rect_t){ 554.0f, 444.0f, 226.0f, 132.0f });
	xgeXuiWidgetSetZ(pApp->pWindow, 1500);
	xgeXuiWindowInit(&pApp->tWindow, &pApp->tXui, pApp->pWindow);
	xgeXuiWindowSetTitle(&pApp->tWindow, Font(pApp), "Window");
	xgeXuiWindowSetIcon(&pApp->tWindow, pApp->bIconReady ? &pApp->tIcon : NULL, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	xgeXuiWindowSetShowCollapse(&pApp->tWindow, 1);
	xgeXuiWindowSetShowMaximize(&pApp->tWindow, 1);
	AddLabel(pApp, xgeXuiWindowGetClientWidget(&pApp->tWindow), (xge_rect_t){ 12.0f, 10.0f, 170.0f, 42.0f }, "Movable and resizable window");

	pApp->pMenuOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 270.0f, 176.0f, 98.0f, 28.0f });
	xgeXuiButtonInit(&pApp->tMenuButton, &pApp->tXui, pApp->pMenuOwner);
	xgeXuiButtonSetText(&pApp->tMenuButton, Font(pApp), "Menu");
	xgeXuiButtonSetClick(&pApp->tMenuButton, OpenMenuProc, pApp);
	xgeXuiMenuInit(&pApp->tMenu, &pApp->tXui);
	xgeXuiMenuSetItems(&pApp->tMenu, g_arrMenuItems, 4);
	xgeXuiMenuSetFont(&pApp->tMenu, Font(pApp));
	xgeXuiMenuSetSelect(&pApp->tMenu, MenuSelectProc, pApp);

	pApp->pTooltipOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 382.0f, 176.0f, 98.0f, 28.0f });
	AddLabel(pApp, pApp->pTooltipOwner, (xge_rect_t){ 8.0f, 3.0f, 82.0f, 22.0f }, "Tooltip");
	xgeXuiWidgetSetBackground(pApp->pTooltipOwner, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetTooltipText(pApp->pTooltipOwner, "Tooltip");

	pApp->pPopupOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 270.0f, 214.0f, 98.0f, 28.0f });
	xgeXuiButtonInit(&pApp->tPopupButton, &pApp->tXui, pApp->pPopupOwner);
	xgeXuiButtonSetText(&pApp->tPopupButton, Font(pApp), "Popup");
	xgeXuiButtonSetClick(&pApp->tPopupButton, OpenPopupProc, pApp);

	pApp->pDialogOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 382.0f, 214.0f, 98.0f, 28.0f });
	xgeXuiButtonInit(&pApp->tDialogButton, &pApp->tXui, pApp->pDialogOwner);
	xgeXuiButtonSetText(&pApp->tDialogButton, Font(pApp), "Dialog");
	xgeXuiButtonSetClick(&pApp->tDialogButton, OpenDialogProc, pApp);

	pApp->pMessageOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 270.0f, 252.0f, 98.0f, 28.0f });
	xgeXuiButtonInit(&pApp->tMessageButton, &pApp->tXui, pApp->pMessageOwner);
	xgeXuiButtonSetText(&pApp->tMessageButton, Font(pApp), "Message");
	xgeXuiButtonSetClick(&pApp->tMessageButton, OpenMessageProc, pApp);

	pApp->pToastOwner = NewWidget(pApp->pLayout, (xge_rect_t){ 382.0f, 252.0f, 98.0f, 28.0f });
	xgeXuiButtonInit(&pApp->tToastButton, &pApp->tXui, pApp->pToastOwner);
	xgeXuiButtonSetText(&pApp->tToastButton, Font(pApp), "Toast");
	xgeXuiButtonSetClick(&pApp->tToastButton, ShowToastProc, pApp);

	pApp->pPopup = NewWidget(pOverlayRoot, (xge_rect_t){ 792.0f, 506.0f, 150.0f, 64.0f });
	xgeXuiPopupInit(&pApp->tPopupCtl, &pApp->tXui, pApp->pPopup);
	xgeXuiPopupSetOwner(&pApp->tPopupCtl, pApp->pPopupOwner);
	xgeXuiPopupSetAutoClose(&pApp->tPopupCtl, 1, 1);
	xgeXuiPopupSetBackground(&pApp->tPopupCtl, XGE_COLOR_RGBA(248, 250, 253, 245));
	pApp->pPopupLabel = NewWidget(pApp->pPopup, (xge_rect_t){ 10.0f, 10.0f, 120.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tLabels[pApp->iLabelCount++], pApp->pPopupLabel, Font(pApp), "Popup");

	pApp->pDialog = NewWidget(pOverlayRoot, (xge_rect_t){ 1010.0f, 382.0f, 196.0f, 112.0f });
	xgeXuiDialogInit(&pApp->tDialogCtl, &pApp->tXui, pApp->pDialog);
	xgeXuiDialogSetTitle(&pApp->tDialogCtl, Font(pApp), "Dialog");
	xgeXuiDialogSetModal(&pApp->tDialogCtl, 0);

	pApp->pMessage = NewWidget(pOverlayRoot, (xge_rect_t){ 1010.0f, 514.0f, 196.0f, 128.0f });
	xgeXuiMessageBoxInit(&pApp->tMessageBox, &pApp->tXui, pApp->pMessage);
	xgeXuiMessageBoxSetText(&pApp->tMessageBox, Font(pApp), "MessageBox", "Buttons and message styling");
	xgeXuiMessageBoxSetType(&pApp->tMessageBox, XGE_XUI_MESSAGE_BOX_INFO);
	xgeXuiMessageBoxSetButtons(&pApp->tMessageBox, XGE_XUI_MESSAGE_BOX_OK_CANCEL);

	pApp->pToast = NewWidget(pOverlayRoot, (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f });
	xgeXuiToastInit(&pApp->tToastCtl, &pApp->tXui, pApp->pToast, Font(pApp));
	xgeXuiToastSetPlacement(&pApp->tToastCtl, XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT);
	xgeXuiToastSetMetrics(&pApp->tToastCtl, 210.0f, 58.0f, 8.0f);
	xgeXuiToastShow(&pApp->tToastCtl, XGE_XUI_TOAST_TYPE_INFO, "Toast", "All controls gallery", 9999.0f);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, Font(pApp));
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(236, 240, 246, 255));

	pApp->pRootPanel = NewWidget(pRoot, (xge_rect_t){ 18.0f, 16.0f, 1210.0f, 680.0f });
	pApp->pStatus = NewWidget(pApp->pRootPanel, (xge_rect_t){ 10.0f, 8.0f, 1188.0f, 28.0f });
	xgeXuiWidgetSetBackground(pApp->pStatus, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatus, Font(pApp), "");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(22, 64, 118, 255));

	pApp->pBasics = NewWidget(pApp->pRootPanel, (xge_rect_t){ 10.0f, 48.0f, 270.0f, 190.0f });
	pApp->pInputs = NewWidget(pApp->pRootPanel, (xge_rect_t){ 292.0f, 48.0f, 438.0f, 220.0f });
	pApp->pValues = NewWidget(pApp->pRootPanel, (xge_rect_t){ 742.0f, 48.0f, 500.0f, 190.0f });
	pApp->pData = NewWidget(pApp->pRootPanel, (xge_rect_t){ 10.0f, 252.0f, 500.0f, 390.0f });
	pApp->pLayout = NewWidget(pApp->pRootPanel, (xge_rect_t){ 522.0f, 252.0f, 500.0f, 390.0f });
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatus == NULL) || (pApp->pBasics == NULL) || (pApp->pInputs == NULL) ||
	     (pApp->pValues == NULL) || (pApp->pData == NULL) || (pApp->pLayout == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	XgeXuiDemoApplyRootPanel(pApp->pRootPanel);
	XgeXuiDemoApplyStatus(pApp->pStatus);
	XgeXuiDemoApplyPanel(pApp->pBasics);
	XgeXuiDemoApplyPanel(pApp->pInputs);
	XgeXuiDemoApplyPanel(pApp->pValues);
	XgeXuiDemoApplyPanel(pApp->pData);
	XgeXuiDemoApplyPanel(pApp->pLayout);
	xgeXuiPanelInit(&pApp->tBasicsPanel, pApp->pBasics);
	xgeXuiPanelSetTitle(&pApp->tBasicsPanel, Font(pApp), "Basic");
	xgeXuiPanelInit(&pApp->tInputsPanel, pApp->pInputs);
	xgeXuiPanelSetTitle(&pApp->tInputsPanel, Font(pApp), "Input");
	xgeXuiPanelInit(&pApp->tValuesPanel, pApp->pValues);
	xgeXuiPanelSetTitle(&pApp->tValuesPanel, Font(pApp), "Choice / Value / Navigation");
	xgeXuiPanelInit(&pApp->tDataPanel, pApp->pData);
	xgeXuiPanelSetTitle(&pApp->tDataPanel, Font(pApp), "Data");
	xgeXuiPanelInit(&pApp->tLayoutPanel, pApp->pLayout);
	xgeXuiPanelSetTitle(&pApp->tLayoutPanel, Font(pApp), "Layout / Overlay");

	AddBasics(pApp);
	AddInputs(pApp);
	AddValues(pApp);
	AddData(pApp);
	AddLayoutAndOverlay(pApp);
	return XGE_OK;
}

static void DrawScrollContent(app_state_t* pApp)
{
	xge_rect_t tRect;
	xge_rect_t tText;
	float fX;
	float fY;
	int i;

	if ( pApp->bFontReady == 0 ) {
		return;
	}
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	xgeClipSet(pApp->pScroll->tContentRect);
	for ( i = 0; i < 8; i++ ) {
		tRect.fX = pApp->pScroll->tContentRect.fX + 12.0f - fX;
		tRect.fY = pApp->pScroll->tContentRect.fY + 12.0f + (float)i * 30.0f - fY;
		tRect.fW = 170.0f;
		tRect.fH = 22.0f;
		xgeShapeRectFillPx(tRect, (i & 1) ? XGE_COLOR_RGBA(248, 250, 253, 255) : XGE_COLOR_RGBA(236, 240, 246, 255));
		xgeShapeRectStrokePx(tRect, 1.0f, XGE_COLOR_RGBA(162, 174, 190, 255));
		tText = tRect;
		tText.fX += 8.0f;
		tText.fW -= 16.0f;
		xgeTextDrawRect(&pApp->tFont, "ScrollView content", tText, XGE_COLOR_RGBA(36, 42, 52, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	xgeClipClear();
}

static void RunChecks(app_state_t* pApp)
{
	pApp->iTableCells = 0;
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCoreOK =
		(pApp->tButton.pWidget != NULL) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "input text") == 0) &&
		(xgeXuiButtonIsSelected(&pApp->tChoiceButton) == 1) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 2) &&
		(xgeXuiSliderGetValue(&pApp->tSlider) == 64.0f) &&
		(pApp->tStatusBar.iItemCount == 3);
	pApp->bDataOK =
		(xgeXuiListViewGetSelected(&pApp->tList) == 1) &&
		(xgeXuiTreeViewGetVisibleCount(&pApp->tTree) == 3) &&
		(xgeXuiTableViewGetRowCount(&pApp->tTable) == pApp->iTableRows) &&
		(xgeXuiPropertyGridGetVisibleCount(&pApp->tPropertyGrid) == 4) &&
		(xgeXuiBreadcrumbGetSegmentCount(&pApp->tBreadcrumb) == 3) &&
		(xgeXuiAccordionGetSectionCount(&pApp->tAccordion) == 2) &&
		(pApp->iVirtualCreates > 0) &&
		(pApp->iVirtualBinds > 0);
	pApp->bOverlayOK =
		(pApp->tPopupCtl.pWidget != NULL) &&
		(xgeXuiWidgetGetTooltip(pApp->pTooltipOwner)->iType == XGE_XUI_TOOLTIP_TEXT) &&
		(pApp->tDialogCtl.pWidget != NULL) &&
		(pApp->tMessageBox.pWidget != NULL) &&
		(pApp->tMenu.iItemCount == 4) &&
		(xgeXuiToastGetCount(&pApp->tToastCtl) == 1);
	pApp->bLayoutOK =
		(xgeXuiSplitLayoutGetPaneCount(&pApp->tSplit) == 2) &&
		(xgeXuiWindowIsOpen(&pApp->tWindow) != 0);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"XUI all controls gallery  core=%d data=%d overlay=%d layout=%d  clicks=%d select=%d toggle=%d slider=%d tableCells=%d virtual=%d/%d",
		pApp->bCoreOK,
		pApp->bDataOK,
		pApp->bOverlayOK,
		pApp->bLayoutOK,
		pApp->iButtonClicks,
		pApp->iSelectCount,
		pApp->iToggleCount,
		pApp->iSliderCount,
		pApp->iTableCells,
		pApp->iVirtualCreates,
		pApp->iVirtualBinds);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int SceneEnter(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static int SceneLeave(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static int SceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	RunChecks(pApp);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int SceneUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	UpdateStatus(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xgeQuit();
	}
	return XGE_OK;
}

static int SceneDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiMenuUnit(&pApp->tMenu);
	xgeXuiToastUnit(&pApp->tToastCtl);
	xgeXuiMessageBoxUnit(&pApp->tMessageBox);
	xgeXuiDialogUnit(&pApp->tDialogCtl);
	xgeXuiPopupUnit(&pApp->tPopupCtl);
	xgeXuiWindowUnit(&pApp->tWindow);
	xgeXuiScrollViewUnit(&pApp->tScroll);
	xgeXuiSplitLayoutUnit(&pApp->tSplit);
	xgeXuiComboBoxUnit(&pApp->tCombo);
	xgeXuiAccordionUnit(&pApp->tAccordion);
	xgeXuiBreadcrumbUnit(&pApp->tBreadcrumb);
	xgeXuiPropertyGridUnit(&pApp->tPropertyGrid);
	xgeXuiTableViewUnit(&pApp->tTable);
	xgeXuiTreeViewUnit(&pApp->tTree);
	xgeXuiVirtualListUnit(&pApp->tVirtualList);
	xgeXuiListViewUnit(&pApp->tList);
	xgeXuiTabsUnit(&pApp->tTabs);
	xgeXuiStatusBarUnit(&pApp->tStatusBar);
	xgeXuiToolbarUnit(&pApp->tToolbar);
	xgeXuiColorPickerUnit(&pApp->tColorPicker);
	xgeXuiTextEditUnit(&pApp->tTextEdit);
	xgeXuiNumericInputUnit(&pApp->tNumeric);
	xgeXuiInputUnit(&pApp->tSearchInput);
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bIconReady ) {
		xgeTextureFree(&pApp->tIcon);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;
	int i;
	int iExitCode;

	memset(&tApp, 0, sizeof(tApp));
	tApp.iTableRows = 20;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], 0);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 1248;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XGE XUI All Controls Gallery";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC | XGE_INIT_RESIZABLE;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( (LoadFont(&tApp) != XGE_OK) || (CreateIcon(&tApp) != XGE_OK) || (xgeXuiInit(&tApp.tXui) != XGE_OK) || (CreateUI(&tApp) != XGE_OK) ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	RunChecks(&tApp);
	UpdateStatus(&tApp);
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = SceneEnter;
	tApp.tScene.onLeave = SceneLeave;
	tApp.tScene.onEvent = SceneEvent;
	tApp.tScene.onUpdate = SceneUpdate;
	tApp.tScene.onDraw = SceneDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	printf(
		"xui-all-controls-gallery final-summary frames=%d core=%d data=%d overlay=%d layout=%d clicks=%d select=%d toggle=%d slider=%d tableCells=%d virtual=%d/%d\n",
		tApp.iFrameCount,
		tApp.bCoreOK,
		tApp.bDataOK,
		tApp.bOverlayOK,
		tApp.bLayoutOK,
		tApp.iButtonClicks,
		tApp.iSelectCount,
		tApp.iToggleCount,
		tApp.iSliderCount,
		tApp.iTableCells,
		tApp.iVirtualCreates,
		tApp.iVirtualBinds);
	xgeSceneSet(NULL);
	AppUnit(&tApp);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCoreOK && tApp.bDataOK && tApp.bOverlayOK && tApp.bLayoutOK) ? 0 : 3;
}
