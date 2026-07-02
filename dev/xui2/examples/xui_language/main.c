#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 820
#define DEMO_TARGET_H 560
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

#define XUI_DEMO_KEY_LEFT_SHIFT 340
#define XUI_DEMO_KEY_LEFT_CTRL 341
#define XUI_DEMO_KEY_LEFT_ALT 342
#define XUI_DEMO_KEY_LEFT_SUPER 343
#define XUI_DEMO_KEY_RIGHT_SHIFT 344
#define XUI_DEMO_KEY_RIGHT_CTRL 345
#define XUI_DEMO_KEY_RIGHT_ALT 346
#define XUI_DEMO_KEY_RIGHT_SUPER 347

enum {
	XUI_LANGUAGE_DEMO_EN = 0,
	XUI_LANGUAGE_DEMO_ZH,
	XUI_LANGUAGE_DEMO_RU,
	XUI_LANGUAGE_DEMO_JA,
	XUI_LANGUAGE_DEMO_FR,
	XUI_LANGUAGE_DEMO_ES,
	XUI_LANGUAGE_DEMO_CUSTOM,
	XUI_LANGUAGE_DEMO_BUTTON_COUNT
};

typedef struct xui_language_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_language pCustomLanguage;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pStatus;
	xui_widget pTranslated;
	xui_widget pInput;
	xui_widget pTextEdit;
	xui_widget arrLanguageButtons[XUI_LANGUAGE_DEMO_BUTTON_COUNT];
	xui_widget pFindButton;
	xui_widget pReplaceButton;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLanguageOK;
	int bFindOK;
} xui_language_demo_t;

static void __xuiLanguageDemoUsage(void)
{
	printf("usage: xui_language [--frames N] [--seconds N]\n");
}

static int __xuiLanguageDemoParseArgs(xui_language_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiLanguageDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiLanguageDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
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

static int __xuiLanguageDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_language_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_language_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(238, 242, 247, 255));
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, (xui_rect_t){24.0f, 20.0f, tRect.fW - 48.0f, tRect.fH - 40.0f}, XUI_COLOR_RGBA(249, 251, 254, 255));
	}
	return XUI_OK;
}

static int __xuiLanguageDemoAddLabel(xui_language_demo_t* pDemo, xui_widget* ppLabel, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = iFlags;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiLanguageDemoCreateButton(xui_language_demo_t* pDemo, xui_widget* ppButton, const char* sText, xui_rect_t tRect)
{
	xui_button_desc_t tDesc;
	xui_widget pButton;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.fBorderWidth = 1.0f;
	iRet = xuiButtonCreate(pDemo->pContext, &pButton, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiButtonSetTextColor(pButton, XUI_COLOR_RGBA(31, 41, 55, 255));
	(void)xuiButtonSetColors(pButton,
		XUI_COLOR_RGBA(237, 242, 247, 255),
		XUI_COLOR_RGBA(224, 232, 242, 255),
		XUI_COLOR_RGBA(210, 222, 236, 255),
		XUI_COLOR_RGBA(220, 230, 244, 255),
		XUI_COLOR_RGBA(210, 214, 220, 255));
	(void)xuiWidgetSetRect(pButton, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pButton);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pButton);
		return iRet;
	}
	*ppButton = pButton;
	return XUI_OK;
}

static int __xuiLanguageDemoCustomLanguage(xui_language_demo_t* pDemo)
{
	xui_language_text_t* pText;
	xarray pArray;
	int iRet;

	pDemo->pCustomLanguage = xuiCreateLanguage(pDemo->pContext, "demo-custom", "Demo Custom", XUI_LANGUAGE_ZH);
	if ( pDemo->pCustomLanguage == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_EDIT_COPY, "Copy [custom]");
	if ( iRet == XUI_OK ) iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_EDIT_PASTE, "Paste [custom]");
	if ( iRet == XUI_OK ) iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_FIND_TITLE, "Find [custom]");
	if ( iRet == XUI_OK ) iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_REPLACE_TITLE, "Replace [custom]");
	if ( iRet == XUI_OK ) iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_FIND_ALL, "Find All [custom]");
	if ( iRet == XUI_OK ) iRet = xuiLanguageSetText(pDemo->pContext, pDemo->pCustomLanguage, XUI_TR_REPLACE_ALL, "Replace All [custom]");
	if ( iRet != XUI_OK ) return iRet;
	pArray = xuiGetLanguageTextArray(pDemo->pCustomLanguage);
	if ( pArray == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pText = (xui_language_text_t*)xrtArrayGet_Unsafe(pArray, XUI_TR_FIND_NEXT);
	if ( pText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pText->sText = "Next [xarray]";
	pText->bOwned = 0;
	xuiLanguageTouch(pDemo->pContext, pDemo->pCustomLanguage);
	return XUI_OK;
}

static int __xuiLanguageDemoLanguageForButton(xui_language_demo_t* pDemo, xui_widget pButton)
{
	int i;

	for ( i = 0; i < XUI_LANGUAGE_DEMO_BUTTON_COUNT; i++ ) {
		if ( pDemo->arrLanguageButtons[i] == pButton ) {
			switch ( i ) {
			case XUI_LANGUAGE_DEMO_EN: return XUI_LANGUAGE_EN;
			case XUI_LANGUAGE_DEMO_ZH: return XUI_LANGUAGE_ZH;
			case XUI_LANGUAGE_DEMO_RU: return XUI_LANGUAGE_RU;
			case XUI_LANGUAGE_DEMO_JA: return XUI_LANGUAGE_JA;
			case XUI_LANGUAGE_DEMO_FR: return XUI_LANGUAGE_FR;
			case XUI_LANGUAGE_DEMO_ES: return XUI_LANGUAGE_ES;
			case XUI_LANGUAGE_DEMO_CUSTOM: return xuiGetLanguageId(pDemo->pCustomLanguage);
			default: return XUI_LANGUAGE_EN;
			}
		}
	}
	return -1;
}

static void __xuiLanguageDemoApplyText(xui_language_demo_t* pDemo)
{
	xui_language pLanguage;
	char sStatus[256];
	char sTranslated[512];
	int iLanguageId;

	if ( pDemo == NULL || pDemo->pContext == NULL ) return;
	iLanguageId = xuiGetLanguage(pDemo->pContext);
	pLanguage = xuiGetLanguagePack(pDemo->pContext, iLanguageId);
	snprintf(sStatus, sizeof(sStatus), "Language: %s (%s)  id=%d  revision=%u",
		xuiGetLanguageName(pLanguage), xuiGetLanguageCode(pLanguage), iLanguageId, xuiGetLanguageRevision(pDemo->pContext));
	snprintf(sTranslated, sizeof(sTranslated), "Copy: %s    Paste: %s    Find: %s    Next: %s    Replace All: %s",
		xuiTranslate(pDemo->pContext, XUI_TR_EDIT_COPY),
		xuiTranslate(pDemo->pContext, XUI_TR_EDIT_PASTE),
		xuiTranslate(pDemo->pContext, XUI_TR_FIND_TITLE),
		xuiTranslate(pDemo->pContext, XUI_TR_FIND_NEXT),
		xuiTranslate(pDemo->pContext, XUI_TR_REPLACE_ALL));
	if ( pDemo->pStatus != NULL ) (void)xuiLabelSetText(pDemo->pStatus, sStatus);
	if ( pDemo->pTranslated != NULL ) (void)xuiLabelSetText(pDemo->pTranslated, sTranslated);
}

static void __xuiLanguageDemoButtonClick(xui_widget pWidget, void* pUser)
{
	xui_language_demo_t* pDemo;
	int iLanguageId;

	pDemo = (xui_language_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	if ( pWidget == pDemo->pFindButton ) {
		(void)xuiTextEditOpenFind(pDemo->pTextEdit);
		return;
	}
	if ( pWidget == pDemo->pReplaceButton ) {
		(void)xuiTextEditOpenReplace(pDemo->pTextEdit);
		return;
	}
	iLanguageId = __xuiLanguageDemoLanguageForButton(pDemo, pWidget);
	if ( iLanguageId >= 0 && xuiSetLanguage(pDemo->pContext, iLanguageId) == XUI_OK ) {
		__xuiLanguageDemoApplyText(pDemo);
	}
}

static int __xuiLanguageDemoCreateUi(xui_language_demo_t* pDemo)
{
	static const char* arrButtonText[XUI_LANGUAGE_DEMO_BUTTON_COUNT] = {
		"English", "Chinese", "Russian", "Japanese", "French", "Spanish", "Custom"
	};
	xui_input_desc_t tInput;
	xui_text_edit_desc_t tEdit;
	float fX;
	int i;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetFlowMode(pDemo->pRoot, XUI_FLOW_ABSOLUTE);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiLanguageDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLanguageDemoAddLabel(pDemo, &pDemo->pTitle, "XUI2 Language", (xui_rect_t){38.0f, 28.0f, 300.0f, 28.0f}, XUI_COLOR_RGBA(31, 41, 55, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLanguageDemoAddLabel(pDemo, &pDemo->pStatus, "", (xui_rect_t){38.0f, 62.0f, 720.0f, 24.0f}, XUI_COLOR_RGBA(64, 79, 101, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLanguageDemoAddLabel(pDemo, &pDemo->pTranslated, "", (xui_rect_t){38.0f, 90.0f, 744.0f, 24.0f}, XUI_COLOR_RGBA(64, 79, 101, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	fX = 38.0f;
	for ( i = 0; i < XUI_LANGUAGE_DEMO_BUTTON_COUNT; i++ ) {
		iRet = __xuiLanguageDemoCreateButton(pDemo, &pDemo->arrLanguageButtons[i], arrButtonText[i], (xui_rect_t){fX, 128.0f, (i == XUI_LANGUAGE_DEMO_CUSTOM) ? 90.0f : 86.0f, 30.0f});
		if ( iRet != XUI_OK ) return iRet;
		(void)xuiButtonSetClick(pDemo->arrLanguageButtons[i], __xuiLanguageDemoButtonClick, pDemo);
		fX += (i == XUI_LANGUAGE_DEMO_CUSTOM) ? 98.0f : 94.0f;
	}
	iRet = __xuiLanguageDemoCreateButton(pDemo, &pDemo->pFindButton, "Find", (xui_rect_t){596.0f, 178.0f, 86.0f, 30.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLanguageDemoCreateButton(pDemo, &pDemo->pReplaceButton, "Replace", (xui_rect_t){692.0f, 178.0f, 86.0f, 30.0f});
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiButtonSetClick(pDemo->pFindButton, __xuiLanguageDemoButtonClick, pDemo);
	(void)xuiButtonSetClick(pDemo->pReplaceButton, __xuiLanguageDemoButtonClick, pDemo);
	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.sText = "right click here";
	tInput.sPlaceholder = "Input menu uses the active XUI language";
	tInput.pFont = pDemo->pFont;
	tInput.iMaxLength = 256;
	tInput.iTextAlign = XUI_INPUT_ALIGN_LEFT;
	iRet = xuiInputCreate(pDemo->pContext, &pDemo->pInput, &tInput);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pInput, (xui_rect_t){38.0f, 178.0f, 530.0f, 30.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pInput);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tEdit, 0, sizeof(tEdit));
	tEdit.iSize = sizeof(tEdit);
	tEdit.sText =
		"XUI2 language switching sample\n"
		"\n"
		"alpha beta gamma\n"
		"copy paste find replace\n"
		"custom language uses Chinese fallback and patches Find Next through xarray.\n"
		"\n"
		"Use the buttons above to switch the context language.";
	tEdit.sPlaceholder = "TextEdit";
	tEdit.pFont = pDemo->pFont;
	tEdit.iMaxLength = 4096;
	tEdit.bWordWrap = 1;
	tEdit.bLineNumbers = 1;
	tEdit.fLineNumberWidth = 46.0f;
	iRet = xuiTextEditCreate(pDemo->pContext, &pDemo->pTextEdit, &tEdit);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pTextEdit, (xui_rect_t){38.0f, 230.0f, 740.0f, 285.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTextEdit);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pTextEdit);
	__xuiLanguageDemoApplyText(pDemo);
	pDemo->bCreateOK = 1;
	return XUI_OK;
}

static void __xuiLanguageDemoLayout(xui_language_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiLanguageDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiLanguageDemoButtonTransitions(xui_language_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiLanguageDemoMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
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
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static uint32_t __xuiLanguageDemoReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static uint32_t __xuiLanguageDemoAsciiFromKey(int iKey, uint32_t iModifiers)
{
	int bShift;

	if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SUPER)) != 0u ) return 0u;
	bShift = ((iModifiers & XUI_MOD_SHIFT) != 0u);
	if ( iKey >= 'A' && iKey <= 'Z' ) return (uint32_t)(bShift ? iKey : (iKey + ('a' - 'A')));
	if ( iKey >= '0' && iKey <= '9' ) return (uint32_t)iKey;
	switch ( iKey ) {
	case XGE_KEY_SPACE: return ' ';
	case '-': return '-';
	case '=': return '=';
	case '[': return '[';
	case ']': return ']';
	case '\\': return '\\';
	case ';': return ';';
	case '\'': return '\'';
	case ',': return ',';
	case '.': return '.';
	case '/': return '/';
	default: return 0u;
	}
}

static int __xuiLanguageDemoSendKeys(xui_language_demo_t* pDemo)
{
	static const int arrKeys[] = {
		'A', 'C', 'F', 'H', 'V', 'X', 'Z',
		XGE_KEY_ENTER, XGE_KEY_TAB, XGE_KEY_BACKSPACE, XGE_KEY_DELETE,
		XGE_KEY_LEFT, XGE_KEY_RIGHT, XGE_KEY_UP, XGE_KEY_DOWN,
		XGE_KEY_PAGE_UP, XGE_KEY_PAGE_DOWN, XGE_KEY_HOME, XGE_KEY_END, XGE_KEY_MENU
	};
	static const int arrPrintable[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		XGE_KEY_SPACE, '-', '=', '[', ']', '\\', ';', '\'', ',', '.', '/'
	};
	uint32_t iText;
	uint32_t iModifiers;
	int iKey;
	int iRet;
	int bTextDelivered;
	int i;

	iModifiers = __xuiLanguageDemoReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiLanguageDemoMapKey(arrKeys[i]);
		if ( iKey == 0 ) iKey = arrKeys[i];
		if ( xgeKeyPressed(arrKeys[i]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, iKey, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iRet = xuiInputKeyUp(pDemo->pContext, iKey, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	bTextDelivered = 0;
	while ( (iText = xgeTextGet()) != 0u ) {
		if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u ) {
			iRet = xuiInputText(pDemo->pContext, iText);
			if ( iRet != XUI_OK ) return iRet;
			bTextDelivered = 1;
		}
	}
	if ( !bTextDelivered ) {
		for ( i = 0; i < (int)(sizeof(arrPrintable) / sizeof(arrPrintable[0])); i++ ) {
			if ( xgeKeyPressed(arrPrintable[i]) ) {
				iText = __xuiLanguageDemoAsciiFromKey(arrPrintable[i], iModifiers);
				if ( iText != 0u ) {
					iRet = xuiInputText(pDemo->pContext, iText);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
		}
	}
	return XUI_OK;
}

static int __xuiLanguageDemoHandleInput(xui_language_demo_t* pDemo)
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
	iButtons = __xuiLanguageDemoReadButtons();
	if ( !pDemo->bHasMouse || pDemo->fLastMouseX != fX || pDemo->fLastMouseY != fY || pDemo->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiLanguageDemoButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiLanguageDemoSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiLanguageDemoExercise(xui_language_demo_t* pDemo, int bAutoRun)
{
	xui_widget pFindWindow;

	pDemo->bLanguageOK = (pDemo->pCustomLanguage != NULL) &&
		(xuiGetLanguageId(pDemo->pCustomLanguage) >= XUI_LANGUAGE_CUSTOM_BASE) &&
		(xuiGetLanguagePack(pDemo->pContext, XUI_LANGUAGE_EN) != NULL) &&
		(xuiGetLanguagePack(pDemo->pContext, XUI_LANGUAGE_ZH) != NULL) &&
		(xuiGetLanguageTextArray(pDemo->pCustomLanguage) != NULL);
	if ( !bAutoRun || pDemo->bExerciseDone ) return;
	if ( xuiSetLanguage(pDemo->pContext, xuiGetLanguageId(pDemo->pCustomLanguage)) == XUI_OK ) {
		__xuiLanguageDemoApplyText(pDemo);
		(void)xuiTextEditOpenFind(pDemo->pTextEdit);
		pFindWindow = xuiTextEditGetFindWindow(pDemo->pTextEdit);
		pDemo->bFindOK = (pFindWindow != NULL) && (strcmp(xuiWindowGetTitle(pFindWindow), "Find [custom]") == 0);
	}
	pDemo->bExerciseDone = 1;
}

static int __xuiLanguageDemoCreateAssets(xui_language_demo_t* pDemo)
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
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiLanguageDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiLanguageDemoCustomLanguage(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiLanguageDemoCreateUi(pDemo);
}

static void __xuiLanguageDemoDestroyAssets(xui_language_demo_t* pDemo)
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

static int __xuiLanguageDemoFrame(void* pUser)
{
	xui_language_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_language_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiLanguageDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiLanguageDemoLayout(pDemo);
	__xuiLanguageDemoExercise(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		printf("xui_language final-summary frames=%d create=%d language=%d find=%d current=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLanguageOK, pDemo->bFindOK, xuiGetLanguage(pDemo->pContext));
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_language_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiLanguageDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiLanguageDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Language";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_language: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiLanguageDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_language: create assets failed: %d\n", iRet);
		__xuiLanguageDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiLanguageDemoFrame, &tDemo);
	__xuiLanguageDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLanguageOK) ? 0 : 1;
}
