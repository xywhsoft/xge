#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHILD_LABEL_COUNT 5
#define CHILD_BUTTON_COUNT 2
#define EDGE_LABEL_COUNT 2
#define EDGE_BUTTON_COUNT 1
#define EDGE_SEPARATOR_COUNT 2

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tEdgeImageTexture;
	xge_xui_scroll_view_t tScroll;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tChildLabel[CHILD_LABEL_COUNT];
	xge_xui_button_t tChildButton[CHILD_BUTTON_COUNT];
	xge_xui_label_t tEdgeLabel[EDGE_LABEL_COUNT];
	xge_xui_image_t tEdgeImage;
	xge_xui_separator_t tEdgeSeparator[EDGE_SEPARATOR_COUNT];
	xge_xui_progress_t tEdgeProgress;
	xge_xui_button_t tEdgeButton[EDGE_BUTTON_COUNT];
	xge_xui_checkbox_t tEdgeCheck;
	xge_xui_radio_t tEdgeRadio;
	xge_xui_toggle_t tEdgeToggle;
	xge_xui_slider_t tEdgeSlider;
	xge_xui_scrollbar_t tEdgeScrollBar;
	xge_xui_input_t tEdgeInput;
	xge_xui_text_edit_t tEdgeTextEdit;
	xge_xui_numeric_input_t tEdgeNumeric;
	xge_xui_combo_box_t tEdgeCombo;
	xge_xui_color_picker_t tEdgeColorPicker;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pScrollWidget;
	xge_xui_widget pContentWidget;
	xge_xui_widget pTopLayer;
	xge_xui_widget pMiddleLayer;
	xge_xui_widget pLeftLayer;
	xge_xui_widget pCenterLayer;
	xge_xui_widget pRightLayer;
	xge_xui_widget pBottomLayer;
	xge_xui_widget pChildLabelWidget[CHILD_LABEL_COUNT];
	xge_xui_widget pChildButtonWidget[CHILD_BUTTON_COUNT];
	xge_xui_widget pEdgeWidget[16];
	int bFontReady;
	int bEdgeTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iClickCount;
	float fMouseX;
	float fMouseY;
	int bCreateOK;
	int bLayoutOK;
	int bCoordinateOK;
	int bChildOK;
	int bEventOK;
} app_state_t;

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent);

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
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
			printf("xui_scrollview font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateEdgeTexture(app_state_t* pApp)
{
	unsigned char arrPixels[48 * 32 * 4];
	int x;
	int y;
	int i;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( y = 0; y < 32; y++ ) {
		for ( x = 0; x < 48; x++ ) {
			i = (y * 48 + x) * 4;
			arrPixels[i + 0] = (unsigned char)(40 + x * 3);
			arrPixels[i + 1] = (unsigned char)(120 + y * 3);
			arrPixels[i + 2] = (unsigned char)(210 - y * 2);
			arrPixels[i + 3] = 255;
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tEdgeImageTexture, 48, 32, arrPixels) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bEdgeTextureReady = 1;
	return XGE_OK;
}

static xge_xui_widget NewWidget(float fX, float fY, float fW, float fH)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ fX, fY, fW, fH });
	}
	return pWidget;
}

static xge_xui_widget NewPanel(float fW, float fH, int iLayout, uint32_t iBackground, uint32_t iBorder)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(0.0f, 0.0f, fW, fH);
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetLayout(pWidget, iLayout);
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fW), xgeXuiSizePx(fH));
		xgeXuiWidgetSetBackground(pWidget, iBackground);
		xgeXuiWidgetSetBorder(pWidget, 1.0f, iBorder);
		xgeXuiWidgetSetPaddingPx(pWidget, 12.0f, 12.0f, 12.0f, 12.0f);
		xgeXuiWidgetSetGap(pWidget, 10.0f);
	}
	return pWidget;
}

static void OnChildButtonClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iClickCount++;
	}
}

static int AddChildLabel(app_state_t* pApp, xge_xui_widget pParent, int iIndex, float fX, float fY, float fW, float fH, const char* sText)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(fX, fY, fW, fH);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fW), xgeXuiSizePx(fH));
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 230));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetPaddingPx(pWidget, 8.0f, 6.0f, 8.0f, 6.0f);
	if ( xgeXuiLabelInit(&pApp->tChildLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tChildLabel[iIndex], XGE_COLOR_RGBA(42, 55, 72, 255));
	xgeXuiLabelSetAlign(&pApp->tChildLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetAdd(pParent, pWidget);
	pApp->pChildLabelWidget[iIndex] = pWidget;
	return XGE_OK;
}

static int AddChildButton(app_state_t* pApp, xge_xui_widget pParent, int iIndex, float fX, float fY, float fW, const char* sText)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(fX, fY, fW, 36.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fW), xgeXuiSizePx(36.0f));
	if ( xgeXuiButtonInit(&pApp->tChildButton[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pApp->tChildButton[iIndex], pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetClick(&pApp->tChildButton[iIndex], OnChildButtonClick, pApp);
	xgeXuiButtonSetColors(
		&pApp->tChildButton[iIndex],
		XGE_COLOR_RGBA(58, 126, 204, 255),
		XGE_COLOR_RGBA(75, 146, 224, 255),
		XGE_COLOR_RGBA(36, 96, 170, 255),
		XGE_COLOR_RGBA(170, 198, 232, 255),
		XGE_COLOR_RGBA(130, 142, 156, 255));
	xgeXuiWidgetAdd(pParent, pWidget);
	pApp->pChildButtonWidget[iIndex] = pWidget;
	return XGE_OK;
}

static xge_xui_widget AddEdgeWidget(app_state_t* pApp, xge_xui_widget pParent, int iIndex, float fX, float fY, float fW, float fH)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(fX, fY, fW, fH);
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fW), xgeXuiSizePx(fH));
	xgeXuiWidgetAdd(pParent, pWidget);
	if ( (iIndex >= 0) && (iIndex < (int)(sizeof(pApp->pEdgeWidget) / sizeof(pApp->pEdgeWidget[0]))) ) {
		pApp->pEdgeWidget[iIndex] = pWidget;
	}
	return pWidget;
}

static int AddDemoControls(app_state_t* pApp)
{
	static const char* arrComboItems[] = { "Short", "Long text should clip before edge", "Third" };
	xge_xui_widget pWidget;
	xge_font pFont;

	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( AddChildLabel(pApp, pApp->pTopLayer, 0, 0.0f, 0.0f, 230.0f, 42.0f, "top layer uses row layout") != XGE_OK ||
	     AddChildLabel(pApp, pApp->pTopLayer, 1, 0.0f, 0.0f, 300.0f, 42.0f, "children are laid out inside virtual content") != XGE_OK ||
	     AddChildButton(pApp, pApp->pTopLayer, 0, 0.0f, 0.0f, 170.0f, "Top Button") != XGE_OK ||
	     AddChildLabel(pApp, pApp->pCenterLayer, 2, 40.0f, 32.0f, 260.0f, 42.0f, "center panel: absolute x/y") != XGE_OK ||
	     AddChildLabel(pApp, pApp->pCenterLayer, 3, 150.0f, 330.0f, 300.0f, 42.0f, "screen -> viewport -> content") != XGE_OK ||
	     AddChildLabel(pApp, pApp->pCenterLayer, 4, 340.0f, 460.0f, 260.0f, 42.0f, "absolute child near lower edge") != XGE_OK ||
	     AddChildButton(pApp, pApp->pCenterLayer, 1, 300.0f, 218.0f, 170.0f, "Ensure Target") != XGE_OK ) {
		return XGE_ERROR;
	}

	pWidget = AddEdgeWidget(pApp, pApp->pRightLayer, 0, 0.0f, 0.0f, 190.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiLabelInit(&pApp->tEdgeLabel[0], pWidget, pFont, "right column uses layout") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 230));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiLabelSetColor(&pApp->tEdgeLabel[0], XGE_COLOR_RGBA(42, 55, 72, 255));
	xgeXuiLabelSetAlign(&pApp->tEdgeLabel[0], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);

	pWidget = AddEdgeWidget(pApp, pApp->pCenterLayer, 1, 40.0f, 104.0f, 140.0f, 54.0f);
	if ( pWidget == NULL || xgeXuiImageInit(&pApp->tEdgeImage, pWidget, pApp->bEdgeTextureReady ? &pApp->tEdgeImageTexture : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiImageSetMode(&pApp->tEdgeImage, XGE_XUI_IMAGE_STRETCH);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(80, 150, 210, 255));

	pWidget = AddEdgeWidget(pApp, pApp->pCenterLayer, 2, 210.0f, 120.0f, 260.0f, 18.0f);
	if ( pWidget == NULL || xgeXuiSeparatorInit(&pApp->tEdgeSeparator[0], pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSeparatorSetColor(&pApp->tEdgeSeparator[0], XGE_COLOR_RGBA(34, 132, 214, 255));
	xgeXuiSeparatorSetThickness(&pApp->tEdgeSeparator[0], 2.0f);

	pWidget = AddEdgeWidget(pApp, pApp->pCenterLayer, 3, 64.0f, 176.0f, 440.0f, 28.0f);
	if ( pWidget == NULL || xgeXuiProgressInit(&pApp->tEdgeProgress, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiProgressSetValue(&pApp->tEdgeProgress, 66.0f);
	xgeXuiProgressSetText(&pApp->tEdgeProgress, pFont, "%1.0f%% Progress");
	xgeXuiProgressSetTextColor(&pApp->tEdgeProgress, XGE_COLOR_RGBA(36, 52, 70, 255));
	xgeXuiProgressSetFillTextColor(&pApp->tEdgeProgress, XGE_COLOR_RGBA(255, 255, 255, 255));

	pWidget = AddEdgeWidget(pApp, pApp->pRightLayer, 4, 0.0f, 0.0f, 190.0f, 36.0f);
	if ( pWidget == NULL || xgeXuiButtonInit(&pApp->tEdgeButton[0], &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pApp->tEdgeButton[0], pFont, "Right Layout Button");
	xgeXuiButtonSetColors(&pApp->tEdgeButton[0], XGE_COLOR_RGBA(58, 126, 204, 255), XGE_COLOR_RGBA(75, 146, 224, 255), XGE_COLOR_RGBA(36, 96, 170, 255), XGE_COLOR_RGBA(170, 198, 232, 255), XGE_COLOR_RGBA(130, 142, 156, 255));

	pWidget = AddEdgeWidget(pApp, pApp->pLeftLayer, 5, 0.0f, 0.0f, 180.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiCheckBoxInit(&pApp->tEdgeCheck, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiCheckBoxSetText(&pApp->tEdgeCheck, pFont, "CheckBox in left layout");
	xgeXuiCheckBoxSetChecked(&pApp->tEdgeCheck, 1);

	pWidget = AddEdgeWidget(pApp, pApp->pLeftLayer, 6, 0.0f, 0.0f, 180.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiRadioInit(&pApp->tEdgeRadio, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiRadioSetText(&pApp->tEdgeRadio, pFont, "Radio in left layout");
	xgeXuiRadioSetChecked(&pApp->tEdgeRadio, 1);

	pWidget = AddEdgeWidget(pApp, pApp->pLeftLayer, 7, 0.0f, 0.0f, 140.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiToggleInit(&pApp->tEdgeToggle, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiToggleSetInnerText(&pApp->tEdgeToggle, pFont, "OFF", "ON");
	xgeXuiToggleSetChecked(&pApp->tEdgeToggle, 1);

	pWidget = AddEdgeWidget(pApp, pApp->pLeftLayer, 8, 0.0f, 0.0f, 190.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiSliderInit(&pApp->tEdgeSlider, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSliderSetRange(&pApp->tEdgeSlider, 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tEdgeSlider, 58.0f);

	pWidget = AddEdgeWidget(pApp, pApp->pLeftLayer, 9, 0.0f, 0.0f, 190.0f, 28.0f);
	if ( pWidget == NULL || xgeXuiScrollBarInit(&pApp->tEdgeScrollBar, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiScrollBarSetRange(&pApp->tEdgeScrollBar, 0.0f, 100.0f, 20.0f);
	xgeXuiScrollBarSetValue(&pApp->tEdgeScrollBar, 42.0f);

	pWidget = AddEdgeWidget(pApp, pApp->pCenterLayer, 10, 70.0f, 394.0f, 230.0f, 30.0f);
	if ( pWidget == NULL || xgeXuiInputInit(&pApp->tEdgeInput, &pApp->tXui, pWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetText(&pApp->tEdgeInput, "Input text clips at right edge");

	pWidget = AddEdgeWidget(pApp, pApp->pCenterLayer, 11, 326.0f, 394.0f, 220.0f, 70.0f);
	if ( pWidget == NULL || xgeXuiTextEditInit(&pApp->tEdgeTextEdit, &pApp->tXui, pWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTextEditSetText(&pApp->tEdgeTextEdit, "TextEdit clips\nright edge");

	pWidget = AddEdgeWidget(pApp, pApp->pBottomLayer, 12, 0.0f, 0.0f, 170.0f, 30.0f);
	if ( pWidget == NULL || xgeXuiNumericInputInit(&pApp->tEdgeNumeric, &pApp->tXui, pWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiNumericInputSetValue(&pApp->tEdgeNumeric, 42.0f);

	pWidget = AddEdgeWidget(pApp, pApp->pBottomLayer, 13, 0.0f, 0.0f, 220.0f, 30.0f);
	if ( pWidget == NULL || xgeXuiComboBoxInit(&pApp->tEdgeCombo, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiComboBoxSetFont(&pApp->tEdgeCombo, pFont);
	xgeXuiComboBoxSetItems(&pApp->tEdgeCombo, arrComboItems, 3);
	xgeXuiComboBoxSetSelected(&pApp->tEdgeCombo, 1);

	pWidget = AddEdgeWidget(pApp, pApp->pBottomLayer, 14, 0.0f, 0.0f, 230.0f, 30.0f);
	if ( pWidget == NULL || xgeXuiColorPickerInit(&pApp->tEdgeColorPicker, &pApp->tXui, pWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiColorPickerSetColor(&pApp->tEdgeColorPicker, XGE_COLOR_RGBA(130, 183, 55, 255));

	pWidget = AddEdgeWidget(pApp, pApp->pBottomLayer, 15, 0.0f, 0.0f, 320.0f, 34.0f);
	if ( pWidget == NULL || xgeXuiLabelInit(&pApp->tEdgeLabel[1], pWidget, pFont, "bottom layer uses row layout") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 230));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiLabelSetColor(&pApp->tEdgeLabel[1], XGE_COLOR_RGBA(42, 55, 72, 255));
	xgeXuiLabelSetAlign(&pApp->tEdgeLabel[1], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);

	pWidget = AddEdgeWidget(pApp, pApp->pRightLayer, -1, 0.0f, 0.0f, 190.0f, 18.0f);
	if ( pWidget == NULL || xgeXuiSeparatorInit(&pApp->tEdgeSeparator[1], pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSeparatorSetColor(&pApp->tEdgeSeparator[1], XGE_COLOR_RGBA(230, 126, 34, 255));
	xgeXuiSeparatorSetThickness(&pApp->tEdgeSeparator[1], 2.0f);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pContent;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	XgeXuiDemoApplyRootPanel(pRoot);

	pApp->pRootPanel = NewWidget(18.0f, 18.0f, 900.0f, 640.0f);
	pApp->pStatusWidget = NewWidget(24.0f, 22.0f, 840.0f, 32.0f);
	pApp->pScrollWidget = NewWidget(24.0f, 66.0f, 840.0f, 520.0f);
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pScrollWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollWidget);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(246, 249, 252, 255));
	xgeXuiWidgetSetBorder(pApp->pRootPanel, 1.0f, XGE_COLOR_RGBA(88, 160, 220, 255));
	xgeXuiWidgetSetBackground(pApp->pScrollWidget, XGE_COLOR_RGBA(235, 242, 250, 255));
	xgeXuiWidgetSetBorder(pApp->pScrollWidget, 1.0f, XGE_COLOR_RGBA(130, 160, 194, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pScrollWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pApp->bFontReady ? &pApp->tFont : NULL, "ScrollView") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(42, 55, 72, 255));
	xgeXuiLabelSetAlign(&pApp->tStatusLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	if ( xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScrollWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 1120.0f, 820.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 120.0f, 80.0f);
	xgeXuiScrollViewSetWheelAxis(&pApp->tScroll, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollViewSetContentDragEnabled(&pApp->tScroll, 1);
	xgeXuiScrollViewSetColors(&pApp->tScroll, XGE_COLOR_RGBA(235, 242, 250, 255), XGE_COLOR_RGBA(210, 228, 242, 255), XGE_COLOR_RGBA(78, 140, 198, 255));

	pContent = NewWidget(0.0f, 0.0f, 1120.0f, 820.0f);
	pApp->pTopLayer = NewPanel(1088.0f, 78.0f, XGE_XUI_LAYOUT_ROW, XGE_COLOR_RGBA(241, 247, 252, 255), XGE_COLOR_RGBA(172, 196, 220, 255));
	pApp->pMiddleLayer = NewPanel(1088.0f, 554.0f, XGE_XUI_LAYOUT_ROW, XGE_COLOR_RGBA(235, 242, 250, 255), XGE_COLOR_RGBA(172, 196, 220, 255));
	pApp->pLeftLayer = NewPanel(216.0f, 528.0f, XGE_XUI_LAYOUT_COLUMN, XGE_COLOR_RGBA(242, 248, 244, 255), XGE_COLOR_RGBA(172, 196, 220, 255));
	pApp->pCenterLayer = NewPanel(610.0f, 528.0f, XGE_XUI_LAYOUT_ABSOLUTE, XGE_COLOR_RGBA(233, 241, 250, 255), XGE_COLOR_RGBA(130, 160, 194, 255));
	pApp->pRightLayer = NewPanel(216.0f, 528.0f, XGE_XUI_LAYOUT_COLUMN, XGE_COLOR_RGBA(246, 243, 250, 255), XGE_COLOR_RGBA(172, 196, 220, 255));
	pApp->pBottomLayer = NewPanel(1088.0f, 118.0f, XGE_XUI_LAYOUT_ROW, XGE_COLOR_RGBA(241, 247, 252, 255), XGE_COLOR_RGBA(172, 196, 220, 255));
	if ( (pContent == NULL) || (pApp->pTopLayer == NULL) || (pApp->pMiddleLayer == NULL) || (pApp->pLeftLayer == NULL) || (pApp->pCenterLayer == NULL) || (pApp->pRightLayer == NULL) || (pApp->pBottomLayer == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->pContentWidget = pContent;
	xgeXuiWidgetSetLayout(pContent, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pContent, xgeXuiSizePx(1120.0f), xgeXuiSizePx(820.0f));
	xgeXuiWidgetSetPaddingPx(pContent, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiWidgetSetGap(pContent, 12.0f);
	xgeXuiWidgetAdd(pApp->pScrollWidget, pContent);
	xgeXuiWidgetAdd(pContent, pApp->pTopLayer);
	xgeXuiWidgetAdd(pContent, pApp->pMiddleLayer);
	xgeXuiWidgetAdd(pContent, pApp->pBottomLayer);
	xgeXuiWidgetAdd(pApp->pMiddleLayer, pApp->pLeftLayer);
	xgeXuiWidgetAdd(pApp->pMiddleLayer, pApp->pCenterLayer);
	xgeXuiWidgetAdd(pApp->pMiddleLayer, pApp->pRightLayer);
	if ( AddDemoControls(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	int iWidth;
	int iHeight;
	float fRootW;
	float fRootH;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	fRootW = (float)iWidth - 36.0f;
	fRootH = (float)iHeight - 36.0f;
	if ( fRootW < 820.0f ) {
		fRootW = 820.0f;
	}
	if ( fRootH < 600.0f ) {
		fRootH = 600.0f;
	}
	xgeXuiWidgetSetRect(xgeXuiRoot(&pApp->tXui), (xge_rect_t){ 0.0f, 0.0f, (float)iWidth, (float)iHeight });
	xgeXuiWidgetSetRect(pApp->pRootPanel, (xge_rect_t){ 18.0f, 18.0f, fRootW, fRootH });
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 24.0f, 20.0f, fRootW - 48.0f, 34.0f });
	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 24.0f, 66.0f, fRootW - 48.0f, fRootH - 92.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];
	float fScrollX;
	float fScrollY;
	float fViewX;
	float fViewY;
	float fContentX;
	float fContentY;

	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fScrollX, &fScrollY);
	xgeXuiScrollModelScreenToViewport(&pApp->tScroll.tScroll, pApp->fMouseX, pApp->fMouseY, &fViewX, &fViewY);
	xgeXuiScrollModelScreenToContent(&pApp->tScroll.tScroll, pApp->fMouseX, pApp->fMouseY, &fContentX, &fContentY);
	snprintf(
		sText,
		sizeof(sText),
		"offset %.0f, %.0f | mouse viewport %.0f, %.0f | content %.0f, %.0f | clicks %d",
		fScrollX,
		fScrollY,
		fViewX,
		fViewY,
		fContentX,
		fContentY,
		pApp->iClickCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static void DrawScrollContent(app_state_t* pApp)
{
	xge_rect_t tClip;
	xge_rect_t tRect;
	float fX;
	float fY;
	float x;
	float y;
	int i;

	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	tClip = pApp->pScrollWidget->tContentRect;
	xgeClipSet(tClip);
	for ( x = 0.0f; x <= 1120.0f; x += 80.0f ) {
		xgeShapeLinePx(tClip.fX + x - fX, tClip.fY - fY, tClip.fX + x - fX, tClip.fY + 820.0f - fY, 1.0f, XGE_COLOR_RGBA(198, 214, 232, 255));
	}
	for ( y = 0.0f; y <= 820.0f; y += 80.0f ) {
		xgeShapeLinePx(tClip.fX - fX, tClip.fY + y - fY, tClip.fX + 1120.0f - fX, tClip.fY + y - fY, 1.0f, XGE_COLOR_RGBA(198, 214, 232, 255));
	}
	for ( i = 0; i < 6; i++ ) {
		tRect.fX = tClip.fX + 62.0f + (float)(i * 154) - fX;
		tRect.fY = tClip.fY + 166.0f + (float)((i % 3) * 118) - fY;
		tRect.fW = 120.0f;
		tRect.fH = 72.0f;
		xgeShapeRectFillPx(tRect, (i % 2) ? XGE_COLOR_RGBA(218, 236, 248, 255) : XGE_COLOR_RGBA(230, 246, 238, 255));
		xgeShapeRectStrokePx(tRect, 1.0f, XGE_COLOR_RGBA(112, 146, 184, 255));
	}
	tRect = (xge_rect_t){ tClip.fX + 980.0f - fX, tClip.fY + 760.0f - fY, 96.0f, 34.0f };
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(255, 242, 214, 255));
	xgeShapeRectStrokePx(tRect, 1.0f, XGE_COLOR_RGBA(220, 156, 58, 255));
	xgeClipClear();
}

static void RunChecks(app_state_t* pApp)
{
	float fViewX;
	float fViewY;
	float fContentX;
	float fContentY;
	float fMaxX;
	float fMaxY;
	xge_rect_t tViewport;

	tViewport = pApp->pScrollWidget->tContentRect;
	xgeXuiScrollModelGetMaxOffset(&pApp->tScroll.tScroll, &fMaxX, &fMaxY);
	xgeXuiScrollModelScreenToViewport(&pApp->tScroll.tScroll, tViewport.fX, tViewport.fY, &fViewX, &fViewY);
	xgeXuiScrollModelScreenToContent(&pApp->tScroll.tScroll, tViewport.fX, tViewport.fY, &fContentX, &fContentY);
	pApp->bCreateOK =
		(pApp->tScroll.tScroll.pWidget == pApp->pScrollWidget) &&
		(pApp->pScrollWidget->procEvent == xgeXuiScrollViewEventProc) &&
		(pApp->pChildButtonWidget[1] != NULL);
	pApp->bLayoutOK =
		(pApp->pScrollWidget->tContentRect.fW > 500.0f) &&
		(pApp->pScrollWidget->tContentRect.fH > 360.0f) &&
		(pApp->pContentWidget != NULL) &&
		(pApp->pContentWidget->tStyle.iLayout == XGE_XUI_LAYOUT_COLUMN) &&
		(pApp->pTopLayer != NULL) &&
		(pApp->pTopLayer->tStyle.iLayout == XGE_XUI_LAYOUT_ROW) &&
		(pApp->pMiddleLayer != NULL) &&
		(pApp->pMiddleLayer->tStyle.iLayout == XGE_XUI_LAYOUT_ROW) &&
		(pApp->pLeftLayer != NULL) &&
		(pApp->pLeftLayer->tStyle.iLayout == XGE_XUI_LAYOUT_COLUMN) &&
		(pApp->pCenterLayer != NULL) &&
		(pApp->pCenterLayer->tStyle.iLayout == XGE_XUI_LAYOUT_ABSOLUTE) &&
		(pApp->pRightLayer != NULL) &&
		(pApp->pRightLayer->tStyle.iLayout == XGE_XUI_LAYOUT_COLUMN) &&
		(pApp->pBottomLayer != NULL) &&
		(pApp->pBottomLayer->tStyle.iLayout == XGE_XUI_LAYOUT_ROW) &&
		(fMaxX > 0.0f) &&
		(fMaxY > 0.0f);
	pApp->bCoordinateOK =
		FloatNear(fViewX, 0.0f, 0.01f) &&
		FloatNear(fViewY, 0.0f, 0.01f) &&
		FloatNear(fContentX, pApp->tScroll.tScroll.fScrollX, 0.01f) &&
		FloatNear(fContentY, pApp->tScroll.tScroll.fScrollY, 0.01f);
	pApp->bChildOK =
		(pApp->pChildButtonWidget[1]->tRect.fX < pApp->tScroll.tScroll.fContentW) &&
		(pApp->pChildButtonWidget[1]->tRect.fY < pApp->tScroll.tScroll.fContentH);
}

static void RunEventCheck(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fOldX;
	float fOldY;
	float fNewX;
	float fNewY;

	if ( pApp->bEventOK != 0 || pApp->pScrollWidget == NULL ) {
		return;
	}
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fOldX, &fOldY);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = pApp->pScrollWidget->tContentRect.fX + 120.0f;
	tEvent.fY = pApp->pScrollWidget->tContentRect.fY + 120.0f;
	AppEvent(&pApp->tScene, &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX -= 24.0f;
	tEvent.fY -= 20.0f;
	AppEvent(&pApp->tScene, &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	AppEvent(&pApp->tScene, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fNewX, &fNewY);
	pApp->bEventOK = (fNewX > fOldX) && (fNewY > fOldY);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	CreateEdgeTexture(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiColorPickerUnit(&pApp->tEdgeColorPicker);
	xgeXuiComboBoxUnit(&pApp->tEdgeCombo);
	xgeXuiNumericInputUnit(&pApp->tEdgeNumeric);
	xgeXuiTextEditUnit(&pApp->tEdgeTextEdit);
	xgeXuiInputUnit(&pApp->tEdgeInput);
	xgeXuiScrollBarUnit(&pApp->tEdgeScrollBar);
	xgeXuiSliderUnit(&pApp->tEdgeSlider);
	xgeXuiToggleUnit(&pApp->tEdgeToggle);
	xgeXuiRadioUnit(&pApp->tEdgeRadio);
	xgeXuiCheckBoxUnit(&pApp->tEdgeCheck);
	for ( i = 0; i < EDGE_BUTTON_COUNT; i++ ) {
		xgeXuiButtonUnit(&pApp->tEdgeButton[i]);
	}
	xgeXuiProgressUnit(&pApp->tEdgeProgress);
	for ( i = 0; i < EDGE_SEPARATOR_COUNT; i++ ) {
		xgeXuiSeparatorUnit(&pApp->tEdgeSeparator[i]);
	}
	xgeXuiImageUnit(&pApp->tEdgeImage);
	for ( i = 0; i < EDGE_LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tEdgeLabel[i]);
	}
	for ( i = 0; i < CHILD_BUTTON_COUNT; i++ ) {
		xgeXuiButtonUnit(&pApp->tChildButton[i]);
	}
	for ( i = 0; i < CHILD_LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tChildLabel[i]);
	}
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiScrollViewUnit(&pApp->tScroll);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	if ( pApp->bEdgeTextureReady ) {
		xgeTextureFree(&pApp->tEdgeImageTexture);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE || pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		pApp->fMouseX = pEvent->fX;
		pApp->fMouseY = pEvent->fY;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunEventCheck(pApp);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_scrollview final-summary frames=%d create=%d layout=%d coord=%d child=%d event=%d scroll=%.2f,%.2f content=%.2fx%.2f\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bCoordinateOK,
			pApp->bChildOK,
			pApp->bEventOK,
			pApp->tScroll.tScroll.fScrollX,
			pApp->tScroll.tScroll.fScrollY,
			pApp->tScroll.tScroll.fContentW,
			pApp->tScroll.tScroll.fContentH);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
	DrawScrollContent(pApp);
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SCROLLVIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI ScrollView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bCoordinateOK && tApp.bChildOK && tApp.bEventOK) ? 0 : 3;
}
