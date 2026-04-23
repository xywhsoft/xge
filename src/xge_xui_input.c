static float __xgeXuiInputDisplayPrefixWidth(xge_xui_input pInput, int iCursor);
static void __xgeXuiInputEnsureCursorVisible(xge_xui_input pInput);
static void __xgeXuiInputResetCursorBlink(xge_xui_input pInput);

enum {
	XGE_XUI_INPUT_MENU_SELECT_ALL = 0,
	XGE_XUI_INPUT_MENU_CUT,
	XGE_XUI_INPUT_MENU_COPY,
	XGE_XUI_INPUT_MENU_PASTE,
	XGE_XUI_INPUT_MENU_DELETE,
	XGE_XUI_INPUT_MENU_COUNT
};

static const char* g_arrXgeXuiInputMenuItems[XGE_XUI_INPUT_MENU_COUNT] = {
	"Select All",
	"Cut",
	"Copy",
	"Paste",
	"Delete"
};

static void __xgeXuiInputMenuSelect(xge_xui_widget pWidget, int iIndex, void* pUser);
static void __xgeXuiInputOpenDefaultMenu(xge_xui_input pInput, float fX, float fY);

int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pInput == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInput, 0, sizeof(*pInput));
	iRet = xgeXuiTextInit(&pInput->tText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTheme = xgeXuiGetTheme(pContext);
	pInput->pContext = pContext;
	pInput->pWidget = pWidget;
	pInput->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pInput->iTextColor = pTheme->iTextColor;
	pInput->iPlaceholderColor = XGE_COLOR_RGBA(150, 160, 174, 255);
	pInput->iBackgroundColor = pTheme->iBackgroundColor;
	pInput->iFocusColor = pTheme->iStateFocus;
	pInput->iCursorColor = pTheme->iTextColor;
	pInput->iSelectionColor = pTheme->iSelectionColor;
	pInput->iDisabledTextColor = XGE_COLOR_RGBA(132, 142, 156, 255);
	pInput->iDisabledBackgroundColor = pTheme->iStateDisabled;
	pInput->bCursorVisible = 1;
	pInput->bInitialized = 1;
	pInput->pDefaultMenu = (xge_xui_menu)xrtMalloc(sizeof(xge_xui_menu_t));
	if ( pInput->pDefaultMenu == NULL ) {
		xgeXuiTextUnit(&pInput->tText);
		memset(pInput, 0, sizeof(*pInput));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeXuiMenuInit(pInput->pDefaultMenu, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		xrtFree(pInput->pDefaultMenu);
		xgeXuiTextUnit(&pInput->tText);
		memset(pInput, 0, sizeof(*pInput));
		return iRet;
	}
	xgeXuiMenuSetFont(pInput->pDefaultMenu, pInput->pFont);
	xgeXuiMenuSetItems(pInput->pDefaultMenu, g_arrXgeXuiInputMenuItems, XGE_XUI_INPUT_MENU_COUNT);
	xgeXuiMenuSetSelect(pInput->pDefaultMenu, __xgeXuiInputMenuSelect, pInput);
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiInputEventProc;
	pWidget->procUpdate = xgeXuiInputUpdateProc;
	pWidget->procPaint = xgeXuiInputPaintProc;
	pWidget->pUser = pInput;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiInputUnit(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return;
	}
	if ( pInput->pWidget != NULL && pInput->pWidget->pUser == pInput ) {
		pInput->pWidget->pUser = NULL;
		pInput->pWidget->procEvent = NULL;
		pInput->pWidget->procUpdate = NULL;
		pInput->pWidget->procPaint = NULL;
	}
	if ( pInput->pDefaultMenu != NULL ) {
		xgeXuiMenuUnit(pInput->pDefaultMenu);
		xrtFree(pInput->pDefaultMenu);
	}
	xgeXuiTextUnit(&pInput->tText);
	memset(pInput, 0, sizeof(*pInput));
}

void xgeXuiInputSetText(xge_xui_input pInput, const char* sText)
{
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	if ( xgeXuiTextSet(&pInput->tText, sText) == XGE_OK ) {
		pInput->fScrollX = 0.0f;
		__xgeXuiInputEnsureCursorVisible(pInput);
		xgeXuiWidgetMarkLayout(pInput->pWidget);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
}

const char* xgeXuiInputGetText(xge_xui_input pInput)
{
	if ( (pInput == NULL) || (pInput->tText.sText == NULL) ) {
		return "";
	}
	return pInput->tText.sText;
}

void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->pFont = pFont;
	if ( pInput->pDefaultMenu != NULL ) {
		xgeXuiMenuSetFont(pInput->pDefaultMenu, pFont);
	}
	xgeXuiWidgetMarkLayout(pInput->pWidget);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iTextColor = iText;
	pInput->iBackgroundColor = iBackground;
	pInput->iFocusColor = iFocus;
	pInput->iCursorColor = iCursor;
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetPlaceholder(xge_xui_input pInput, const char* sText)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->sPlaceholder = sText;
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetPassword(xge_xui_input pInput, int bPassword)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bPassword = (bPassword != 0);
	__xgeXuiInputEnsureCursorVisible(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetReadonly(xge_xui_input pInput, int bReadonly)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bReadonly = (bReadonly != 0);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetDisabled(xge_xui_input pInput, int bDisabled)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bDisabled = (bDisabled != 0);
	if ( pInput->pWidget != NULL ) {
		xgeXuiWidgetSetEnabled(pInput->pWidget, pInput->bDisabled == 0);
	}
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd)
{
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	xgeXuiTextSetSelection(&pInput->tText, iStart, iEnd);
	__xgeXuiInputEnsureCursorVisible(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd)
{
	if ( pInput == NULL ) {
		if ( pStart != NULL ) {
			*pStart = 0;
		}
		if ( pEnd != NULL ) {
			*pEnd = 0;
		}
		return;
	}
	xgeXuiTextGetSelection(&pInput->tText, pStart, pEnd);
}

xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput)
{
	xge_rect_t tRect;
	float fCursorX;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return tRect;
	}
	fCursorX = pInput->pWidget->tContentRect.fX + __xgeXuiInputDisplayPrefixWidth(pInput, xgeXuiTextGetCursor(&pInput->tText)) - pInput->fScrollX;
	if ( fCursorX > (pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW) ) {
		fCursorX = pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW;
	}
	tRect.fX = fCursorX;
	tRect.fY = pInput->pWidget->tContentRect.fY;
	tRect.fW = 1.0f;
	tRect.fH = pInput->pWidget->tContentRect.fH;
	return tRect;
}

static int __xgeXuiInputIsCtrl(const xge_event_t* pEvent)
{
	return (pEvent != NULL) && ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0);
}

static int __xgeXuiInputCharClass(const char* sText, int iPos)
{
	unsigned char c;

	if ( (sText == NULL) || (iPos < 0) || (sText[iPos] == 0) ) {
		return 0;
	}
	c = (unsigned char)sText[iPos];
	if ( c <= 32 ) {
		return 0;
	}
	if ( c >= 128 ) {
		return 1;
	}
	if ( ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '_') ) {
		return 1;
	}
	return 2;
}

static int __xgeXuiInputPrevWord(xge_xui_input pInput)
{
	const char* sText;
	int iPos;
	int iPrev;
	int iClass;

	if ( pInput == NULL ) {
		return 0;
	}
	sText = xgeXuiInputGetText(pInput);
	iPos = __xgeXuiTextPrevCursor(&pInput->tText, pInput->tText.iCursor);
	while ( iPos > 0 && __xgeXuiInputCharClass(sText, iPos) == 0 ) {
		iPos = __xgeXuiTextPrevCursor(&pInput->tText, iPos);
	}
	iClass = __xgeXuiInputCharClass(sText, iPos);
	while ( iPos > 0 ) {
		iPrev = __xgeXuiTextPrevCursor(&pInput->tText, iPos);
		if ( __xgeXuiInputCharClass(sText, iPrev) != iClass ) {
			break;
		}
		iPos = iPrev;
	}
	return iPos;
}

static int __xgeXuiInputNextWord(xge_xui_input pInput)
{
	const char* sText;
	int iPos;
	int iNext;
	int iClass;

	if ( pInput == NULL ) {
		return 0;
	}
	sText = xgeXuiInputGetText(pInput);
	iPos = __xgeXuiTextClampCursor(&pInput->tText, pInput->tText.iCursor);
	if ( iPos >= pInput->tText.iSize ) {
		return pInput->tText.iSize;
	}
	iClass = __xgeXuiInputCharClass(sText, iPos);
	while ( iPos < pInput->tText.iSize ) {
		iNext = __xgeXuiTextNextCursor(&pInput->tText, iPos);
		if ( iNext <= iPos ) {
			break;
		}
		if ( __xgeXuiInputCharClass(sText, iNext) != iClass ) {
			iPos = iNext;
			break;
		}
		iPos = iNext;
	}
	while ( iPos < pInput->tText.iSize && __xgeXuiInputCharClass(sText, iPos) == 0 ) {
		iNext = __xgeXuiTextNextCursor(&pInput->tText, iPos);
		if ( iNext <= iPos ) {
			break;
		}
		iPos = iNext;
	}
	return iPos;
}

static void __xgeXuiInputSelectWordAt(xge_xui_input pInput, int iCursor)
{
	const char* sText;
	int iStart;
	int iEnd;
	int iPrev;
	int iNext;
	int iClass;

	if ( pInput == NULL || pInput->tText.iSize <= 0 ) {
		return;
	}
	sText = xgeXuiInputGetText(pInput);
	iStart = __xgeXuiTextClampCursor(&pInput->tText, iCursor);
	if ( iStart >= pInput->tText.iSize && iStart > 0 ) {
		iStart = __xgeXuiTextPrevCursor(&pInput->tText, iStart);
	}
	iClass = __xgeXuiInputCharClass(sText, iStart);
	if ( iClass == 0 ) {
		xgeXuiTextSetCursor(&pInput->tText, iCursor);
		return;
	}
	while ( iStart > 0 ) {
		iPrev = __xgeXuiTextPrevCursor(&pInput->tText, iStart);
		if ( __xgeXuiInputCharClass(sText, iPrev) != iClass ) {
			break;
		}
		iStart = iPrev;
	}
	iEnd = iStart;
	while ( iEnd < pInput->tText.iSize && __xgeXuiInputCharClass(sText, iEnd) == iClass ) {
		iNext = __xgeXuiTextNextCursor(&pInput->tText, iEnd);
		if ( iNext <= iEnd ) {
			break;
		}
		iEnd = iNext;
	}
	xgeXuiTextSetSelection(&pInput->tText, iStart, iEnd);
}

static char* __xgeXuiInputSelectionText(xge_xui_input pInput)
{
	char* sOut;
	int iStart;
	int iEnd;
	int iSize;

	if ( pInput == NULL ) {
		return NULL;
	}
	xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		return NULL;
	}
	iSize = iEnd - iStart;
	sOut = (char*)xrtMalloc((size_t)iSize + 1);
	if ( sOut == NULL ) {
		return NULL;
	}
	memcpy(sOut, xgeXuiInputGetText(pInput) + iStart, (size_t)iSize);
	sOut[iSize] = 0;
	return sOut;
}

static void __xgeXuiInputCopySelection(xge_xui_input pInput)
{
	char* sSelection;

	if ( (pInput == NULL) || (pInput->bPassword != 0) ) {
		return;
	}
	sSelection = __xgeXuiInputSelectionText(pInput);
	if ( sSelection != NULL ) {
		xgeClipboardSetText(sSelection);
		xrtFree(sSelection);
	}
}

static void __xgeXuiInputCutSelection(xge_xui_input pInput)
{
	if ( (pInput == NULL) || (pInput->bPassword != 0) || (pInput->bReadonly != 0) ) {
		return;
	}
	__xgeXuiInputCopySelection(pInput);
	__xgeXuiTextSelectionDelete(&pInput->tText);
	__xgeXuiInputEnsureCursorVisible(pInput);
	__xgeXuiInputResetCursorBlink(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

static void __xgeXuiInputPasteClipboard(xge_xui_input pInput)
{
	const char* sClipboard;

	if ( (pInput == NULL) || (pInput->bReadonly != 0) ) {
		return;
	}
	sClipboard = xgeClipboardGetText();
	if ( (sClipboard != NULL) && (sClipboard[0] != 0) && (xgeXuiTextInsert(&pInput->tText, sClipboard) == XGE_OK) ) {
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
}

static void __xgeXuiInputDeleteSelection(xge_xui_input pInput)
{
	if ( (pInput == NULL) || (pInput->bReadonly != 0) ) {
		return;
	}
	if ( __xgeXuiTextSelectionDelete(&pInput->tText) == XGE_OK ) {
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
}

static void __xgeXuiInputMenuSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_input pInput;

	(void)pWidget;
	pInput = (xge_xui_input)pUser;
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	switch ( iIndex ) {
		case XGE_XUI_INPUT_MENU_SELECT_ALL:
			xgeXuiTextSetSelection(&pInput->tText, 0, pInput->tText.iSize);
			__xgeXuiInputEnsureCursorVisible(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			break;
		case XGE_XUI_INPUT_MENU_CUT:
			__xgeXuiInputCutSelection(pInput);
			break;
		case XGE_XUI_INPUT_MENU_COPY:
			__xgeXuiInputCopySelection(pInput);
			break;
		case XGE_XUI_INPUT_MENU_PASTE:
			__xgeXuiInputPasteClipboard(pInput);
			break;
		case XGE_XUI_INPUT_MENU_DELETE:
			__xgeXuiInputDeleteSelection(pInput);
			break;
		default:
			break;
	}
}

static void __xgeXuiInputOpenDefaultMenu(xge_xui_input pInput, float fX, float fY)
{
	const char* sClipboard;
	int iStart;
	int iEnd;
	int bSelection;
	int bClipboard;

	if ( (pInput == NULL) || (pInput->pDefaultMenu == NULL) || (pInput->bPassword != 0) ) {
		return;
	}
	xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
	sClipboard = xgeClipboardGetText();
	bSelection = (iStart != iEnd);
	bClipboard = (sClipboard != NULL) && (sClipboard[0] != 0);
	pInput->arrDefaultMenuEnabled[XGE_XUI_INPUT_MENU_SELECT_ALL] = (pInput->tText.iSize > 0);
	pInput->arrDefaultMenuEnabled[XGE_XUI_INPUT_MENU_CUT] = bSelection && (pInput->bReadonly == 0);
	pInput->arrDefaultMenuEnabled[XGE_XUI_INPUT_MENU_COPY] = bSelection;
	pInput->arrDefaultMenuEnabled[XGE_XUI_INPUT_MENU_PASTE] = bClipboard && (pInput->bReadonly == 0);
	pInput->arrDefaultMenuEnabled[XGE_XUI_INPUT_MENU_DELETE] = bSelection && (pInput->bReadonly == 0);
	xgeXuiMenuSetEnabledItems(pInput->pDefaultMenu, pInput->arrDefaultMenuEnabled, XGE_XUI_INPUT_MENU_COUNT);
	xgeXuiMenuOpen(pInput->pDefaultMenu, fX, fY);
}

static char* __xgeXuiInputMakePasswordText(const char* sText, int iLimitBytes)
{
	char* sOut;
	int i;
	int iCount;

	if ( sText == NULL ) {
		return NULL;
	}
	if ( iLimitBytes < 0 ) {
		iLimitBytes = (int)strlen(sText);
	}
	iCount = 0;
	for ( i = 0; sText[i] != 0 && i < iLimitBytes; i++ ) {
		if ( ((unsigned char)sText[i] & 0xC0) != 0x80 ) {
			iCount++;
		}
	}
	sOut = (char*)xrtMalloc((size_t)iCount + 1);
	if ( sOut == NULL ) {
		return NULL;
	}
	for ( i = 0; i < iCount; i++ ) {
		sOut[i] = '*';
	}
	sOut[iCount] = 0;
	return sOut;
}

static float __xgeXuiInputDisplayPrefixWidth(xge_xui_input pInput, int iCursor)
{
	char* sPassword;
	float fWidth;

	if ( pInput == NULL ) {
		return 0.0f;
	}
	if ( pInput->bPassword == 0 ) {
		return __xgeXuiTextPrefixWidth(pInput->pFont, xgeXuiInputGetText(pInput), iCursor);
	}
	sPassword = __xgeXuiInputMakePasswordText(xgeXuiInputGetText(pInput), iCursor);
	if ( sPassword == NULL ) {
		return 0.0f;
	}
	fWidth = __xgeXuiHostMeasureText(pInput->pFont, sPassword).fX;
	xrtFree(sPassword);
	return fWidth;
}

static void __xgeXuiInputEnsureCursorVisible(xge_xui_input pInput)
{
	float fCursorX;
	float fTextW;
	float fContentW;
	float fMaxScroll;

	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return;
	}
	fContentW = pInput->pWidget->tContentRect.fW;
	if ( fContentW <= 0.0f ) {
		pInput->fScrollX = 0.0f;
		return;
	}
	fCursorX = __xgeXuiInputDisplayPrefixWidth(pInput, xgeXuiTextGetCursor(&pInput->tText));
	fTextW = __xgeXuiInputDisplayPrefixWidth(pInput, pInput->tText.iSize);
	fMaxScroll = fTextW - fContentW + 2.0f;
	if ( fMaxScroll < 0.0f ) {
		fMaxScroll = 0.0f;
	}
	if ( fCursorX - pInput->fScrollX > fContentW - 2.0f ) {
		pInput->fScrollX = fCursorX - fContentW + 2.0f;
	}
	if ( fCursorX - pInput->fScrollX < 0.0f ) {
		pInput->fScrollX = fCursorX;
	}
	if ( pInput->fScrollX < 0.0f ) {
		pInput->fScrollX = 0.0f;
	}
	if ( pInput->fScrollX > fMaxScroll ) {
		pInput->fScrollX = fMaxScroll;
	}
}

static void __xgeXuiInputResetCursorBlink(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->fCursorBlinkTime = 0.0f;
	if ( pInput->bCursorVisible == 0 ) {
		pInput->bCursorVisible = 1;
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	} else {
		pInput->bCursorVisible = 1;
	}
}

int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent)
{
	int iResult;
	int iCursor;
	int iStart;
	int iEnd;

	if ( (pInput == NULL) || (pInput->bInitialized == 0) || (pInput->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		double fNow;
		int bDoubleClick;

		if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
			xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
		fNow = xgeTimer();
		bDoubleClick = ((fNow - pInput->fLastClickTime) <= 0.35) && ((pEvent->fX - pInput->fLastClickX) < 4.0f) && ((pInput->fLastClickX - pEvent->fX) < 4.0f) && ((pEvent->fY - pInput->fLastClickY) < 4.0f) && ((pInput->fLastClickY - pEvent->fY) < 4.0f);
		xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		if ( bDoubleClick != 0 ) {
			pInput->fLastClickTime = fNow;
			pInput->fLastClickX = pEvent->fX;
			pInput->fLastClickY = pEvent->fY;
			__xgeXuiInputSelectWordAt(pInput, iCursor);
			pInput->bSelecting = 0;
			pInput->bPressPending = 0;
		} else {
			pInput->iPressCursor = iCursor;
			pInput->fPressX = pEvent->fX;
			pInput->fPressY = pEvent->fY;
			xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
			pInput->bPressInsideSelection = (iStart != iEnd && iCursor >= iStart && iCursor <= iEnd) ? 1 : 0;
			pInput->bPressPending = 1;
			pInput->bSelecting = 0;
			xgeXuiSetCapture(pInput->pContext, pInput->pWidget);
		}
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pInput->bPressPending != 0) ) {
		float fDX = pEvent->fX - pInput->fPressX;
		float fDY = pEvent->fY - pInput->fPressY;
		if ( (fDX * fDX + fDY * fDY) <= 36.0f ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		pInput->bPressPending = 0;
		if ( pInput->bPressInsideSelection != 0 ) {
			pInput->bSelecting = 2;
			return XGE_XUI_EVENT_CONSUMED;
		}
		pInput->bSelecting = 1;
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		xgeXuiTextSetSelection(&pInput->tText, pInput->iPressCursor, iCursor);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pInput->bSelecting == 1) ) {
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		xgeXuiTextSetSelection(&pInput->tText, pInput->iPressCursor, iCursor);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) || (pEvent->iType == XGE_EVENT_TOUCH_CANCEL)) && ((pInput->bSelecting != 0) || (pInput->bPressPending != 0)) ) {
		if ( pInput->bPressPending != 0 && pEvent->iType != XGE_EVENT_TOUCH_CANCEL ) {
			pInput->fLastClickTime = xgeTimer();
			pInput->fLastClickX = pEvent->fX;
			pInput->fLastClickY = pEvent->fY;
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiInputCursorFromX(pInput, pEvent->fX));
			__xgeXuiInputEnsureCursorVisible(pInput);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
		}
		pInput->bPressPending = 0;
		pInput->bPressInsideSelection = 0;
		pInput->bSelecting = 0;
		if ( pInput->pContext != NULL && pInput->pContext->pCapture == pInput->pWidget ) {
			xgeXuiSetCapture(pInput->pContext, NULL);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_XUI_CONTEXT_BEGIN) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_UPDATE) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_END) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_CANCEL) ) {
		pInput->bPressPending = 0;
		pInput->bPressInsideSelection = 0;
		pInput->bSelecting = 0;
		if ( pInput->pContext != NULL && pInput->pContext->pCapture == pInput->pWidget ) {
			xgeXuiSetCapture(pInput->pContext, NULL);
		}
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		if ( pEvent->iType == XGE_EVENT_XUI_CONTEXT_BEGIN ) {
			__xgeXuiInputOpenDefaultMenu(pInput, pEvent->fX, pEvent->fY);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus != pInput->pWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_TEXT) || (pEvent->iType == XGE_EVENT_IME_START) || (pEvent->iType == XGE_EVENT_IME_UPDATE) || (pEvent->iType == XGE_EVENT_IME_END) ) {
		if ( pInput->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		iResult = xgeXuiTextInputEvent(&pInput->tText, pEvent);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiInputEnsureCursorVisible(pInput);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
		}
		return iResult;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && __xgeXuiInputIsCtrl(pEvent) && (pEvent->iParam1 == 'A' || pEvent->iParam1 == 'a') ) {
		xgeXuiTextSetSelection(&pInput->tText, 0, pInput->tText.iSize);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && __xgeXuiInputIsCtrl(pEvent) && (pEvent->iParam1 == 'C' || pEvent->iParam1 == 'c' || pEvent->iParam1 == 'X' || pEvent->iParam1 == 'x') ) {
		if ( pInput->bPassword != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( (pEvent->iParam1 == 'X' || pEvent->iParam1 == 'x') && (pInput->bReadonly == 0) ) {
			__xgeXuiInputCutSelection(pInput);
		} else {
			__xgeXuiInputCopySelection(pInput);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && __xgeXuiInputIsCtrl(pEvent) && (pEvent->iParam1 == 'V' || pEvent->iParam1 == 'v') ) {
		if ( pInput->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		__xgeXuiInputPasteClipboard(pInput);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_HOME) ) {
		xgeXuiTextSetCursor(&pInput->tText, 0);
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_END) ) {
		xgeXuiTextSetCursor(&pInput->tText, pInput->tText.iSize);
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pInput->bReadonly != 0) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && ((pEvent->iParam1 == XGE_KEY_BACKSPACE) || (pEvent->iParam1 == XGE_KEY_DELETE)) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_BACKSPACE) ) {
		if ( xgeXuiTextDeleteBack(&pInput->tText) == XGE_OK ) {
			__xgeXuiInputEnsureCursorVisible(pInput);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_DELETE) ) {
		if ( xgeXuiTextDeleteForward(&pInput->tText) == XGE_OK ) {
			__xgeXuiInputEnsureCursorVisible(pInput);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_LEFT) ) {
		xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
		if ( iStart != iEnd ) {
			xgeXuiTextSetCursor(&pInput->tText, iStart);
		} else if ( __xgeXuiInputIsCtrl(pEvent) ) {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiInputPrevWord(pInput));
		} else {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiTextPrevCursor(&pInput->tText, pInput->tText.iCursor));
		}
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_RIGHT) ) {
		xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
		if ( iStart != iEnd ) {
			xgeXuiTextSetCursor(&pInput->tText, iEnd);
		} else if ( __xgeXuiInputIsCtrl(pEvent) ) {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiInputNextWord(pInput));
		} else {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiTextNextCursor(&pInput->tText, pInput->tText.iCursor));
		}
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiInputEvent((xge_xui_input)pUser, pEvent);
}

void xgeXuiInputUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_input pInput;
	int bVisible;

	pInput = (xge_xui_input)pUser;
	if ( (pWidget == NULL) || (pInput == NULL) ) {
		return;
	}
	if ( (pInput->pContext == NULL) || (pInput->pContext->pFocus != pWidget) || (pInput->bSelecting != 0) ) {
		__xgeXuiInputResetCursorBlink(pInput);
		return;
	}
	if ( fDelta < 0.0f ) {
		fDelta = 0.0f;
	}
	pInput->fCursorBlinkTime += fDelta;
	if ( pInput->fCursorBlinkTime >= 0.5f ) {
		pInput->fCursorBlinkTime = 0.0f;
		bVisible = (pInput->bCursorVisible == 0);
		if ( pInput->bCursorVisible != bVisible ) {
			pInput->bCursorVisible = bVisible;
			xgeXuiWidgetMarkPaint(pWidget);
		}
	}
}

void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_input pInput;
	xge_rect_t tCursor;
	xge_rect_t tSelection;
	xge_rect_t tComposition;
	xge_rect_t tTextRect;
	xge_vec2_t tSize;
	char* sPassword;
	const char* sDrawText;
	uint32_t iTextColor;
	uint32_t iBackground;
	float fStartX;
	float fEndX;
	int iStart;
	int iEnd;

	pInput = (xge_xui_input)pUser;
	if ( (pWidget == NULL) || (pInput == NULL) ) {
		return;
	}
	iBackground = (pInput->bDisabled != 0) ? pInput->iDisabledBackgroundColor : ((pInput->pContext != NULL && pInput->pContext->pFocus == pWidget) ? pInput->iFocusColor : pInput->iBackgroundColor);
	if ( XGE_COLOR_GET_A(iBackground) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iBackground);
	}
	xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
	if ( (iStart != iEnd) && (XGE_COLOR_GET_A(pInput->iSelectionColor) != 0) ) {
		fStartX = pWidget->tContentRect.fX + __xgeXuiInputDisplayPrefixWidth(pInput, iStart) - pInput->fScrollX;
		fEndX = pWidget->tContentRect.fX + __xgeXuiInputDisplayPrefixWidth(pInput, iEnd) - pInput->fScrollX;
		if ( fStartX < pWidget->tContentRect.fX ) {
			fStartX = pWidget->tContentRect.fX;
		}
		if ( fEndX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			fEndX = pWidget->tContentRect.fX + pWidget->tContentRect.fW;
		}
		if ( fEndX > fStartX ) {
			tSelection.fX = fStartX;
			tSelection.fY = pWidget->tContentRect.fY + 2.0f;
			tSelection.fW = fEndX - fStartX;
			tSelection.fH = pWidget->tContentRect.fH - 4.0f;
			if ( tSelection.fH < 1.0f ) {
				tSelection.fH = 1.0f;
			}
			__xgeXuiHostDrawRect(tSelection, pInput->iSelectionColor);
		}
	}
	sPassword = NULL;
	sDrawText = xgeXuiInputGetText(pInput);
	iTextColor = (pInput->bDisabled != 0) ? pInput->iDisabledTextColor : pInput->iTextColor;
	if ( (sDrawText[0] == 0) && (pInput->sPlaceholder != NULL) && (pInput->sPlaceholder[0] != 0) && ((pInput->tText.sComposition == NULL) || (pInput->tText.sComposition[0] == 0)) ) {
		sDrawText = pInput->sPlaceholder;
		iTextColor = pInput->iPlaceholderColor;
	} else if ( pInput->bPassword != 0 ) {
		sPassword = __xgeXuiInputMakePasswordText(sDrawText, -1);
		if ( sPassword != NULL ) {
			sDrawText = sPassword;
		}
	}
	if ( (pInput->pFont != NULL) && (sDrawText != NULL) ) {
		tTextRect = pWidget->tContentRect;
		if ( sDrawText != pInput->sPlaceholder ) {
			tTextRect.fX -= pInput->fScrollX;
			tTextRect.fW += pInput->fScrollX;
		}
		__xgeXuiHostDrawTextRect(pInput->pFont, sDrawText, tTextRect, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	}
	if ( sPassword != NULL ) {
		xrtFree(sPassword);
	}
	if ( (pInput->pFont != NULL) && (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
		tComposition = xgeXuiInputGetCandidateRect(pInput);
		tComposition.fX += 1.0f;
		tComposition.fW = (pWidget->tContentRect.fX + pWidget->tContentRect.fW) - tComposition.fX;
		if ( tComposition.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pInput->pFont, pInput->tText.sComposition, tComposition, pInput->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
		}
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus == pWidget) && (pInput->bCursorVisible != 0) && (XGE_COLOR_GET_A(pInput->iCursorColor) != 0) ) {
		tSize.fX = __xgeXuiInputDisplayPrefixWidth(pInput, xgeXuiTextGetCursor(&pInput->tText));
		if ( (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
			tSize.fX += __xgeXuiTextPrefixWidth(pInput->pFont, pInput->tText.sComposition, pInput->tText.iCompositionSize);
		}
		tSize.fY = 0.0f;
		tCursor.fX = pWidget->tContentRect.fX + tSize.fX - pInput->fScrollX + 1.0f;
		if ( tCursor.fX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f) ) {
			tCursor.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
		}
		tCursor.fY = pWidget->tContentRect.fY + 2.0f;
		tCursor.fW = 1.0f;
		tCursor.fH = pWidget->tContentRect.fH - 4.0f;
		if ( tCursor.fH < 1.0f ) {
			tCursor.fH = 1.0f;
		}
		__xgeXuiHostDrawRect(tCursor, pInput->iCursorColor);
	}
}
