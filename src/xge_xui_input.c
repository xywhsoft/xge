static float __xgeXuiInputDisplayPrefixWidth(xge_xui_input pInput, int iCursor);
static void __xgeXuiInputEnsureCursorVisible(xge_xui_input pInput);
static void __xgeXuiInputResetCursorBlink(xge_xui_input pInput);
static char* __xgeXuiInputTextDup(const char* sText);
static void __xgeXuiInputAfterTextMutation(xge_xui_input pInput, const char* sBefore, int bLayout);
static void __xgeXuiInputDecorationLayout(xge_xui_input pInput);
static void __xgeXuiInputUpdatePadding(xge_xui_input pInput);
static void __xgeXuiInputSyncWidgetStyle(xge_xui_input pInput);
static int __xgeXuiInputBeginOuterPaint(xge_xui_context pContext, xge_rect_t tClipRect, xge_rect_t* pOldClip, int* pOldClipEnabled);
static void __xgeXuiInputEndOuterPaint(const xge_rect_t* pOldClip, int bOldClipEnabled);

enum {
	XGE_XUI_INPUT_MENU_SELECT_ALL = 0,
	XGE_XUI_INPUT_MENU_CUT,
	XGE_XUI_INPUT_MENU_COPY,
	XGE_XUI_INPUT_MENU_PASTE,
	XGE_XUI_INPUT_MENU_DELETE,
	XGE_XUI_INPUT_MENU_COUNT
};

static const xge_xui_menu_item_t g_arrXgeXuiInputMenuItems[XGE_XUI_INPUT_MENU_COUNT] = {
	{ "Select All", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_INPUT_MENU_SELECT_ALL, 0, NULL, NULL },
	{ "Cut", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_INPUT_MENU_CUT, 0, NULL, NULL },
	{ "Copy", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_INPUT_MENU_COPY, 0, NULL, NULL },
	{ "Paste", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_INPUT_MENU_PASTE, 0, NULL, NULL },
	{ "Delete", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_INPUT_MENU_DELETE, 0, NULL, NULL }
};

static void __xgeXuiInputMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser);
static void __xgeXuiInputOpenDefaultMenu(xge_xui_input pInput, float fX, float fY);

static void __xgeXuiInputSyncWidgetStyle(xge_xui_input pInput)
{
	uint32_t iBackground;
	uint32_t iHoverBackground;
	uint32_t iBorder;
	uint32_t iHoverBorder;
	uint32_t iFocusBorder;

	if ( pInput == NULL ) {
		return;
	}
	iBackground = pInput->iNormalBackgroundColor;
	iHoverBackground = pInput->iHoverBackgroundColor;
	iBorder = pInput->iBorderColor;
	iHoverBorder = pInput->iHoverBorderColor;
	iFocusBorder = pInput->iFocusBorderColor;
	if ( (pInput->bError != 0) && (pInput->bDisabled == 0) ) {
		iBackground = pInput->iErrorBackgroundColor;
		iHoverBackground = pInput->iErrorBackgroundColor;
		iBorder = pInput->iErrorBorderColor;
		iHoverBorder = pInput->iErrorBorderColor;
		iFocusBorder = pInput->iErrorBorderColor;
	}
	xgeXuiWidgetSetBackground(pInput->pWidget, iBackground);
	xgeXuiWidgetSetBorder(pInput->pWidget, 1.0f, iBorder);
	xgeXuiWidgetSetStateBackground(pInput->pWidget, XGE_XUI_STATE_HOVER, iHoverBackground);
	xgeXuiWidgetSetStateBorder(pInput->pWidget, XGE_XUI_STATE_HOVER, 1.0f, iHoverBorder);
	xgeXuiWidgetSetStateBorder(pInput->pWidget, XGE_XUI_STATE_FOCUS, 1.0f, iFocusBorder);
	xgeXuiWidgetSetStateBackground(pInput->pWidget, XGE_XUI_STATE_DISABLED, pInput->iDisabledBackgroundColor);
	xgeXuiWidgetSetStateBorder(pInput->pWidget, XGE_XUI_STATE_DISABLED, 1.0f, pInput->iDisabledBorderColor);
}

static int __xgeXuiInputDecorationSideClamp(int iSide)
{
	return (iSide == XGE_XUI_INPUT_DECORATION_SIDE_TRAILING) ? XGE_XUI_INPUT_DECORATION_SIDE_TRAILING : XGE_XUI_INPUT_DECORATION_SIDE_LEADING;
}

static int __xgeXuiInputDecorationKindClamp(int iKind)
{
	if ( (iKind < XGE_XUI_INPUT_DECORATION_NONE) || (iKind > XGE_XUI_INPUT_DECORATION_CUSTOM_PAINT) ) {
		return XGE_XUI_INPUT_DECORATION_NONE;
	}
	return iKind;
}

static int __xgeXuiInputDecorationVisibleModeClamp(int iMode)
{
	if ( (iMode < XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS) || (iMode > XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED_NOT_EMPTY) ) {
		return XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	}
	return iMode;
}

static xge_xui_input_decoration* __xgeXuiInputDecorationHead(xge_xui_input pInput, int iSide)
{
	if ( pInput == NULL ) {
		return NULL;
	}
	return (__xgeXuiInputDecorationSideClamp(iSide) == XGE_XUI_INPUT_DECORATION_SIDE_TRAILING) ? &pInput->pTrailingDecoration : &pInput->pLeadingDecoration;
}

static void __xgeXuiInputDecorationDirty(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bDecorationDirty = 1;
	__xgeXuiInputUpdatePadding(pInput);
	xgeXuiWidgetMarkLayout(pInput->pWidget);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

static int __xgeXuiInputDecorationIsVisible(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	int bFocused;
	int bNotEmpty;

	if ( (pInput == NULL) || (pDecoration == NULL) || (pDecoration->iKind == XGE_XUI_INPUT_DECORATION_NONE) ) {
		return 0;
	}
	bFocused = (pInput->pContext != NULL) && (pInput->pContext->pFocus == pInput->pWidget);
	bNotEmpty = (xgeXuiInputGetText(pInput)[0] != 0);
	switch ( pDecoration->iVisibleMode ) {
		case XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY:
			return bNotEmpty;
		case XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED:
			return bFocused;
		case XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED_NOT_EMPTY:
			return bFocused && bNotEmpty;
		case XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS:
		default:
			return 1;
	}
}

static float __xgeXuiInputDecorationDefaultWidth(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	xge_vec2_t tSize;

	if ( pDecoration == NULL ) {
		return 0.0f;
	}
	if ( pDecoration->fWidth > 0.0f ) {
		return pDecoration->fWidth;
	}
	if ( pDecoration->iKind == XGE_XUI_INPUT_DECORATION_TEXT && pDecoration->sText != NULL && pInput != NULL && pInput->pFont != NULL ) {
		tSize = __xgeXuiHostMeasureText(pInput->pFont, pDecoration->sText);
		return tSize.fX + pDecoration->fPadding * 2.0f;
	}
	return 22.0f;
}

static void __xgeXuiInputDecorationCopy(xge_xui_input_decoration pDecoration, int iSide, const xge_xui_input_decoration_desc_t* pDesc)
{
	if ( (pDecoration == NULL) || (pDesc == NULL) ) {
		return;
	}
	pDecoration->iSide = __xgeXuiInputDecorationSideClamp(iSide);
	pDecoration->iKind = __xgeXuiInputDecorationKindClamp(pDesc->iKind);
	pDecoration->iVisibleMode = __xgeXuiInputDecorationVisibleModeClamp(pDesc->iVisibleMode);
	pDecoration->fWidth = pDesc->fWidth;
	pDecoration->fPadding = (pDesc->fPadding >= 0.0f) ? pDesc->fPadding : 0.0f;
	pDecoration->iIcon = pDesc->iIcon;
	pDecoration->sText = pDesc->sText;
	pDecoration->pTexture = pDesc->pTexture;
	pDecoration->tSrc = pDesc->tSrc;
	pDecoration->iColor = pDesc->iColor;
	pDecoration->iHoverColor = pDesc->iHoverColor;
	pDecoration->iActiveColor = pDesc->iActiveColor;
	pDecoration->iDisabledColor = pDesc->iDisabledColor;
	pDecoration->procClick = pDesc->procClick;
	pDecoration->procPaint = pDesc->procPaint;
	pDecoration->pUser = pDesc->pUser;
}

static void __xgeXuiInputDecorationFreeList(xge_xui_input_decoration pDecoration)
{
	xge_xui_input_decoration pNext;

	while ( pDecoration != NULL ) {
		pNext = pDecoration->pNext;
		xrtFree(pDecoration);
		pDecoration = pNext;
	}
}

static xge_rect_t __xgeXuiInputImeCandidateRect(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	return xgeXuiInputGetCandidateRect((xge_xui_input)pUser);
}

int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pInput == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInput, 0, sizeof(*pInput));
	__xgeXuiTextControlWidgetInit(pWidget);
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
	pInput->iNormalBackgroundColor = pTheme->iBackgroundColor;
	pInput->iHoverBackgroundColor = XGE_COLOR_RGBA(250, 253, 255, 255);
	pInput->iFocusColor = pTheme->iStateFocus;
	pInput->iBorderColor = XGE_COLOR_RGBA(184, 210, 230, 255);
	pInput->iHoverBorderColor = XGE_COLOR_RGBA(143, 188, 220, 255);
	pInput->iFocusBorderColor = XGE_COLOR_RGBA(53, 147, 218, 255);
	pInput->iErrorBackgroundColor = XGE_COLOR_RGBA(255, 244, 246, 255);
	pInput->iErrorBorderColor = XGE_COLOR_RGBA(220, 74, 84, 255);
	pInput->iClearColor = XGE_COLOR_RGBA(96, 126, 148, 255);
	pInput->iClearHoverColor = XGE_COLOR_RGBA(72, 102, 124, 255);
	pInput->iIconColor = XGE_COLOR_RGBA(68, 126, 166, 255);
	pInput->iCursorColor = pTheme->iTextColor;
	pInput->iSelectionColor = pTheme->iSelectionColor;
	pInput->iDisabledTextColor = XGE_COLOR_RGBA(132, 142, 156, 255);
	pInput->iDisabledBackgroundColor = pTheme->iStateDisabled;
	pInput->iDisabledBorderColor = XGE_COLOR_RGBA(196, 206, 216, 255);
	__xgeXuiInputSyncWidgetStyle(pInput);
	pInput->bCursorVisible = 1;
	pInput->bInitialized = 1;
	pInput->pDefaultMenu = (xge_xui_menu)xrtMalloc(sizeof(xge_xui_menu_t));
	if ( pInput->pDefaultMenu == NULL ) {
		xgeXuiTextUnit(&pInput->tText);
		memset(pInput, 0, sizeof(*pInput));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeXuiMenuInit(pInput->pDefaultMenu, pContext);
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
	xgeXuiWidgetSetImeCandidateRect(pWidget, __xgeXuiInputImeCandidateRect, pInput);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiInputEventProc, NULL);
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
		xgeXuiWidgetSetEvent(pInput->pWidget, NULL, NULL);
		pInput->pWidget->procUpdate = NULL;
		pInput->pWidget->procPaint = NULL;
	}
	if ( (pInput->pWidget != NULL) && (pInput->pWidget->procImeCandidateRect == __xgeXuiInputImeCandidateRect) && (pInput->pWidget->pImeCandidateRectUser == pInput) ) {
		xgeXuiWidgetSetImeCandidateRect(pInput->pWidget, NULL, NULL);
	}
	if ( pInput->pDefaultMenu != NULL ) {
		xgeXuiMenuUnit(pInput->pDefaultMenu);
		xrtFree(pInput->pDefaultMenu);
	}
	__xgeXuiInputDecorationFreeList(pInput->pLeadingDecoration);
	__xgeXuiInputDecorationFreeList(pInput->pTrailingDecoration);
	xgeXuiTextUnit(&pInput->tText);
	memset(pInput, 0, sizeof(*pInput));
}

static char* __xgeXuiInputTextDup(const char* sText)
{
	char* sCopy;
	size_t n;

	if ( sText == NULL ) {
		sText = "";
	}
	n = strlen(sText);
	sCopy = (char*)xrtMalloc(n + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, n + 1u);
	return sCopy;
}

static void __xgeXuiInputEnforceMaxLength(xge_xui_input pInput)
{
	int iLimit;

	if ( (pInput == NULL) || (pInput->iMaxLength <= 0) || (pInput->tText.sText == NULL) || (pInput->tText.iSize <= pInput->iMaxLength) ) {
		return;
	}
	iLimit = pInput->iMaxLength;
	if ( iLimit < 0 ) {
		iLimit = 0;
	}
	if ( iLimit > pInput->tText.iSize ) {
		iLimit = pInput->tText.iSize;
	}
	while ( (iLimit > 0) && (((unsigned char)pInput->tText.sText[iLimit] & 0xC0) == 0x80) ) {
		iLimit--;
	}
	pInput->tText.sText[iLimit] = 0;
	pInput->tText.iSize = iLimit;
	pInput->tText.iCursor = __xgeXuiTextClampCursor(&pInput->tText, pInput->tText.iCursor);
	pInput->tText.iSelectStart = __xgeXuiTextClampCursor(&pInput->tText, pInput->tText.iSelectStart);
	pInput->tText.iSelectEnd = __xgeXuiTextClampCursor(&pInput->tText, pInput->tText.iSelectEnd);
	xgeXuiTextClearComposition(&pInput->tText);
}

static int __xgeXuiInputApplyFilter(xge_xui_input pInput, const char* sBefore)
{
	const char* sNow;

	if ( (pInput == NULL) || (pInput->procFilter == NULL) || (sBefore == NULL) ) {
		return 1;
	}
	sNow = xgeXuiInputGetText(pInput);
	if ( strcmp(sBefore, sNow) == 0 ) {
		return 1;
	}
	if ( pInput->procFilter(pInput->pWidget, sBefore, sNow, pInput->pFilterUser) != 0 ) {
		return 1;
	}
	(void)xgeXuiTextSet(&pInput->tText, sBefore);
	pInput->iFilterRejectCount++;
	return 0;
}

static void __xgeXuiInputAfterTextMutation(xge_xui_input pInput, const char* sBefore, int bLayout)
{
	const char* sNow;
	int bChanged;

	if ( pInput == NULL ) {
		if ( sBefore != NULL ) {
			xrtFree((void*)sBefore);
		}
		return;
	}
	__xgeXuiInputEnforceMaxLength(pInput);
	(void)__xgeXuiInputApplyFilter(pInput, sBefore);
	__xgeXuiInputUpdatePadding(pInput);
	__xgeXuiInputEnsureCursorVisible(pInput);
	__xgeXuiInputResetCursorBlink(pInput);
	if ( bLayout != 0 ) {
		xgeXuiWidgetMarkLayout(pInput->pWidget);
	}
	xgeXuiWidgetMarkPaint(pInput->pWidget);
	sNow = xgeXuiInputGetText(pInput);
	bChanged = (sBefore == NULL) || (strcmp(sBefore, sNow) != 0);
	if ( sBefore != NULL ) {
		xrtFree((void*)sBefore);
	}
	if ( bChanged != 0 ) {
		pInput->iChangeCount++;
		if ( pInput->procChange != NULL ) {
			pInput->procChange(pInput->pWidget, sNow, pInput->pChangeUser);
		}
	}
}

static int __xgeXuiInputIconClamp(int iIcon)
{
	if ( (iIcon < XGE_XUI_INPUT_ICON_NONE) || (iIcon > XGE_XUI_INPUT_ICON_EYE) ) {
		return XGE_XUI_INPUT_ICON_NONE;
	}
	return iIcon;
}

static void __xgeXuiInputDecorationLayoutSide(xge_xui_input pInput, xge_xui_input_decoration pDecoration, float fStartX, float fY, float fH, float* pTotal)
{
	float fX;
	float fWidth;

	fX = fStartX;
	if ( pTotal != NULL ) {
		*pTotal = 0.0f;
	}
	while ( pDecoration != NULL ) {
		memset(&pDecoration->tRect, 0, sizeof(pDecoration->tRect));
		if ( __xgeXuiInputDecorationIsVisible(pInput, pDecoration) ) {
			fWidth = __xgeXuiInputDecorationDefaultWidth(pInput, pDecoration);
			pDecoration->tRect.fX = fX;
			pDecoration->tRect.fY = fY;
			pDecoration->tRect.fW = fWidth;
			pDecoration->tRect.fH = fH;
			fX += fWidth;
			if ( pTotal != NULL ) {
				*pTotal += fWidth;
			}
		}
		pDecoration = pDecoration->pNext;
	}
}

static float __xgeXuiInputDecorationMeasureSide(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	float fTotal;

	fTotal = 0.0f;
	while ( pDecoration != NULL ) {
		if ( __xgeXuiInputDecorationIsVisible(pInput, pDecoration) ) {
			fTotal += __xgeXuiInputDecorationDefaultWidth(pInput, pDecoration);
		}
		pDecoration = pDecoration->pNext;
	}
	return fTotal;
}

static void __xgeXuiInputDecorationLayout(xge_xui_input pInput)
{
	xge_rect_t tRect;
	float fTrailing;
	float fTrailingX;

	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return;
	}
	tRect = pInput->pWidget->tPaddingRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		tRect = pInput->pWidget->tContentRect;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		tRect = pInput->pWidget->tRect;
	}
	fTrailing = __xgeXuiInputDecorationMeasureSide(pInput, pInput->pTrailingDecoration);
	fTrailingX = tRect.fX + tRect.fW - fTrailing;
	__xgeXuiInputDecorationLayoutSide(pInput, pInput->pLeadingDecoration, tRect.fX, tRect.fY, tRect.fH, &pInput->fLeadingDecorationWidth);
	__xgeXuiInputDecorationLayoutSide(pInput, pInput->pTrailingDecoration, fTrailingX, tRect.fY, tRect.fH, &pInput->fTrailingDecorationWidth);
	pInput->bDecorationDirty = 0;
}

static void __xgeXuiInputUpdatePadding(xge_xui_input pInput)
{
	float fLeft;
	float fRight;

	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return;
	}
	__xgeXuiInputDecorationLayout(pInput);
	fLeft = 4.0f + pInput->fLeadingDecorationWidth;
	fRight = 4.0f + pInput->fTrailingDecorationWidth;
	xgeXuiWidgetSetPaddingPx(pInput->pWidget, fLeft, 4.0f, fRight, 4.0f);
}

static const uint16_t* __xgeXuiInputIconMask(int iIcon, int* pWidth, int* pHeight)
{
	static const uint16_t arrSearch12[12] = {
		0x000, 0x1e0, 0x210, 0x408,
		0x408, 0x408, 0x210, 0x1e0,
		0x060, 0x030, 0x018, 0x000
	};
	static const uint16_t arrUser12[12] = {
		0x000, 0x1e0, 0x330, 0x330,
		0x330, 0x1e0, 0x000, 0x3f0,
		0x7f8, 0xc0c, 0xc0c, 0x000
	};
	static const uint16_t arrLock12[12] = {
		0x000, 0x1e0, 0x330, 0x330,
		0x330, 0xffc, 0xffc, 0xe1c,
		0xe1c, 0xffc, 0xffc, 0x000
	};
	static const uint16_t arrEye12[12] = {
		0x000, 0x000, 0x1e0, 0x618,
		0xc0c, 0xdac, 0xdac, 0xc0c,
		0x618, 0x1e0, 0x000, 0x000
	};

	if ( pWidth != NULL ) {
		*pWidth = 12;
	}
	if ( pHeight != NULL ) {
		*pHeight = 12;
	}
	switch ( iIcon ) {
		case XGE_XUI_INPUT_ICON_SEARCH:
			return arrSearch12;
		case XGE_XUI_INPUT_ICON_USER:
			return arrUser12;
		case XGE_XUI_INPUT_ICON_LOCK:
			return arrLock12;
		case XGE_XUI_INPUT_ICON_EYE:
			return arrEye12;
		default:
			return NULL;
	}
}

static int __xgeXuiInputBeginOuterPaint(xge_xui_context pContext, xge_rect_t tClipRect, xge_rect_t* pOldClip, int* pOldClipEnabled)
{
	xge_rect_t tParentClip;
	int bParentClip;

	if ( (tClipRect.fW <= 0.0f) || (tClipRect.fH <= 0.0f) || (pOldClip == NULL) || (pOldClipEnabled == NULL) ) {
		return 0;
	}
	*pOldClip = g_xge.tClipRect;
	*pOldClipEnabled = g_xge.bClipEnabled;
	bParentClip = 0;
	memset(&tParentClip, 0, sizeof(tParentClip));
	if ( (pContext != NULL) && (pContext->iPaintClipStackCount > 1) ) {
		tParentClip = pContext->arrPaintClipStack[pContext->iPaintClipStackCount - 2];
		bParentClip = 1;
	} else if ( (pContext != NULL) && (pContext->bPaintClipBaseEnabled != 0) ) {
		tParentClip = pContext->tPaintClipBaseRect;
		bParentClip = 1;
	}
	if ( bParentClip != 0 ) {
		tClipRect = __xgeXuiRectIntersection(tParentClip, tClipRect);
		if ( (tClipRect.fW <= 0.0f) || (tClipRect.fH <= 0.0f) ) {
			return 0;
		}
	}
	(void)xgeFlush();
	__xgeXuiHostClipSet(tClipRect);
	return 1;
}

static void __xgeXuiInputEndOuterPaint(const xge_rect_t* pOldClip, int bOldClipEnabled)
{
	if ( pOldClip == NULL ) {
		return;
	}
	(void)xgeFlush();
	if ( bOldClipEnabled != 0 ) {
		__xgeXuiHostClipSet(*pOldClip);
	} else {
		__xgeXuiHostClipClear();
	}
}

void xgeXuiInputSetText(xge_xui_input pInput, const char* sText)
{
	char* sBefore;

	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
	if ( xgeXuiTextSet(&pInput->tText, sText) == XGE_OK ) {
		pInput->fScrollX = 0.0f;
		__xgeXuiInputAfterTextMutation(pInput, sBefore, 1);
	} else if ( sBefore != NULL ) {
		xrtFree(sBefore);
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
	pInput->iNormalBackgroundColor = iBackground;
	pInput->iHoverBackgroundColor = iBackground;
	pInput->iFocusColor = iFocus;
	pInput->iFocusBorderColor = iFocus;
	pInput->iCursorColor = iCursor;
	__xgeXuiInputSyncWidgetStyle(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetFrameColors(xge_xui_input pInput, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iNormalBackgroundColor = iBackground;
	pInput->iHoverBackgroundColor = iHoverBackground;
	pInput->iBorderColor = iBorder;
	pInput->iHoverBorderColor = iHoverBorder;
	pInput->iFocusBorderColor = iFocusBorder;
	__xgeXuiInputSyncWidgetStyle(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetDisabledColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iBorder)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iDisabledTextColor = iText;
	pInput->iDisabledBackgroundColor = iBackground;
	pInput->iDisabledBorderColor = iBorder;
	__xgeXuiInputSyncWidgetStyle(pInput);
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

void xgeXuiInputSetChange(xge_xui_input pInput, xge_xui_text_submit_proc procChange, void* pUser)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->procChange = procChange;
	pInput->pChangeUser = pUser;
}

void xgeXuiInputSetSubmit(xge_xui_input pInput, xge_xui_text_submit_proc procSubmit, void* pUser)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->procSubmit = procSubmit;
	pInput->pSubmitUser = pUser;
}

void xgeXuiInputSetFilter(xge_xui_input pInput, xge_xui_input_filter_proc procFilter, void* pUser)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->procFilter = procFilter;
	pInput->pFilterUser = pUser;
}

void xgeXuiInputSetErrorChange(xge_xui_input pInput, xge_xui_input_error_proc procError, void* pUser)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->procError = procError;
	pInput->pErrorUser = pUser;
}

void xgeXuiInputSetMaxLength(xge_xui_input pInput, int iMaxLength)
{
	char* sBefore;

	if ( pInput == NULL ) {
		return;
	}
	if ( iMaxLength < 0 ) {
		iMaxLength = 0;
	}
	sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
	pInput->iMaxLength = iMaxLength;
	__xgeXuiInputAfterTextMutation(pInput, sBefore, 1);
}

int xgeXuiInputGetMaxLength(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return 0;
	}
	return pInput->iMaxLength;
}

void xgeXuiInputSetTextAlign(xge_xui_input pInput, int iAlign)
{
	if ( pInput == NULL ) {
		return;
	}
	if ( (iAlign < XGE_XUI_INPUT_TEXT_ALIGN_LEFT) || (iAlign > XGE_XUI_INPUT_TEXT_ALIGN_RIGHT) ) {
		iAlign = XGE_XUI_INPUT_TEXT_ALIGN_LEFT;
	}
	if ( pInput->iTextAlign == iAlign ) {
		return;
	}
	pInput->iTextAlign = iAlign;
	__xgeXuiInputEnsureCursorVisible(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

int xgeXuiInputGetTextAlign(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return XGE_XUI_INPUT_TEXT_ALIGN_LEFT;
	}
	return pInput->iTextAlign;
}

void xgeXuiInputSetError(xge_xui_input pInput, int bError)
{
	int bNewError;
	int bChanged;

	if ( pInput == NULL ) {
		return;
	}
	bNewError = (bError != 0);
	bChanged = (pInput->bError != bNewError);
	pInput->bError = bNewError;
	__xgeXuiInputSyncWidgetStyle(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
	if ( ((bChanged != 0) || (pInput->bError != 0)) && (pInput->procError != NULL) ) {
		pInput->procError(pInput->pWidget, pInput->bError, pInput->pErrorUser);
	}
}

int xgeXuiInputGetError(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return 0;
	}
	return pInput->bError;
}

void xgeXuiInputSetErrorColors(xge_xui_input pInput, uint32_t iBackground, uint32_t iBorder)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iErrorBackgroundColor = iBackground;
	pInput->iErrorBorderColor = iBorder;
	__xgeXuiInputSyncWidgetStyle(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

xge_xui_input_decoration xgeXuiInputDecorationAdd(xge_xui_input pInput, int iSide, const xge_xui_input_decoration_desc_t* pDesc)
{
	xge_xui_input_decoration* ppHead;
	xge_xui_input_decoration pItem;
	xge_xui_input_decoration pLast;

	if ( (pInput == NULL) || (pDesc == NULL) ) {
		return NULL;
	}
	ppHead = __xgeXuiInputDecorationHead(pInput, iSide);
	if ( ppHead == NULL ) {
		return NULL;
	}
	pItem = (xge_xui_input_decoration)xrtMalloc(sizeof(*pItem));
	if ( pItem == NULL ) {
		return NULL;
	}
	memset(pItem, 0, sizeof(*pItem));
	__xgeXuiInputDecorationCopy(pItem, iSide, pDesc);
	if ( *ppHead == NULL ) {
		*ppHead = pItem;
	} else {
		pLast = *ppHead;
		while ( pLast->pNext != NULL ) {
			pLast = pLast->pNext;
		}
		pLast->pNext = pItem;
	}
	__xgeXuiInputDecorationDirty(pInput);
	return pItem;
}

void xgeXuiInputDecorationSet(xge_xui_input pInput, xge_xui_input_decoration pDecoration, const xge_xui_input_decoration_desc_t* pDesc)
{
	if ( (pInput == NULL) || (pDecoration == NULL) || (pDesc == NULL) ) {
		return;
	}
	__xgeXuiInputDecorationCopy(pDecoration, pDecoration->iSide, pDesc);
	__xgeXuiInputDecorationDirty(pInput);
}

void xgeXuiInputDecorationRemove(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	xge_xui_input_decoration* ppHead;
	xge_xui_input_decoration pPrev;
	xge_xui_input_decoration pItem;

	if ( (pInput == NULL) || (pDecoration == NULL) ) {
		return;
	}
	ppHead = __xgeXuiInputDecorationHead(pInput, pDecoration->iSide);
	if ( ppHead == NULL ) {
		return;
	}
	pPrev = NULL;
	pItem = *ppHead;
	while ( pItem != NULL ) {
		if ( pItem == pDecoration ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pItem->pNext;
			} else {
				*ppHead = pItem->pNext;
			}
			if ( pInput->pHoverDecoration == pItem ) {
				pInput->pHoverDecoration = NULL;
			}
			if ( pInput->pActiveDecoration == pItem ) {
				pInput->pActiveDecoration = NULL;
			}
			xrtFree(pItem);
			__xgeXuiInputDecorationDirty(pInput);
			return;
		}
		pPrev = pItem;
		pItem = pItem->pNext;
	}
}

void xgeXuiInputDecorationClear(xge_xui_input pInput, int iSide)
{
	xge_xui_input_decoration* ppHead;

	if ( pInput == NULL ) {
		return;
	}
	ppHead = __xgeXuiInputDecorationHead(pInput, iSide);
	if ( ppHead == NULL ) {
		return;
	}
	__xgeXuiInputDecorationFreeList(*ppHead);
	*ppHead = NULL;
	pInput->pHoverDecoration = NULL;
	pInput->pActiveDecoration = NULL;
	__xgeXuiInputDecorationDirty(pInput);
}

xge_rect_t xgeXuiInputDecorationGetRect(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pInput == NULL) || (pDecoration == NULL) ) {
		return tRect;
	}
	__xgeXuiInputDecorationLayout(pInput);
	return pDecoration->tRect;
}

static xge_xui_input_decoration __xgeXuiInputDecorationFindKind(xge_xui_input pInput, int iSide, int iKind)
{
	xge_xui_input_decoration* ppHead;
	xge_xui_input_decoration pItem;

	ppHead = __xgeXuiInputDecorationHead(pInput, iSide);
	if ( ppHead == NULL ) {
		return NULL;
	}
	pItem = *ppHead;
	while ( pItem != NULL ) {
		if ( pItem->iKind == iKind ) {
			return pItem;
		}
		pItem = pItem->pNext;
	}
	return NULL;
}

void xgeXuiInputSetClearButton(xge_xui_input pInput, int bEnabled)
{
	xge_xui_input_decoration_desc_t tDesc;
	xge_xui_input_decoration pOld;

	if ( pInput == NULL ) {
		return;
	}
	pOld = __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_CLEAR);
	if ( bEnabled == 0 ) {
		xgeXuiInputDecorationRemove(pInput, pOld);
		return;
	}
	if ( pOld != NULL ) {
		return;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XGE_XUI_INPUT_DECORATION_CLEAR;
	tDesc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY;
	tDesc.fWidth = 22.0f;
	tDesc.fPadding = 3.0f;
	tDesc.iColor = pInput->iClearColor;
	tDesc.iHoverColor = pInput->iClearHoverColor;
	tDesc.iActiveColor = pInput->iClearHoverColor;
	tDesc.iDisabledColor = pInput->iDisabledTextColor;
	(void)xgeXuiInputDecorationAdd(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tDesc);
}

int xgeXuiInputGetClearButton(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return 0;
	}
	return __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_CLEAR) != NULL;
}

xge_rect_t xgeXuiInputGetClearRect(xge_xui_input pInput)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pInput != NULL ) {
		tRect = xgeXuiInputDecorationGetRect(pInput, __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_CLEAR));
	}
	return tRect;
}

void xgeXuiInputSetClearColors(xge_xui_input pInput, uint32_t iColor, uint32_t iHoverColor)
{
	xge_xui_input_decoration pClear;

	if ( pInput == NULL ) {
		return;
	}
	pInput->iClearColor = iColor;
	pInput->iClearHoverColor = iHoverColor;
	pClear = __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_CLEAR);
	if ( pClear != NULL ) {
		pClear->iColor = iColor;
		pClear->iHoverColor = iHoverColor;
		pClear->iActiveColor = iHoverColor;
	}
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetIcons(xge_xui_input pInput, int iPrefixIcon, int iSuffixIcon)
{
	xge_xui_input_decoration_desc_t tDesc;
	xge_xui_input_decoration pIcon;

	if ( pInput == NULL ) {
		return;
	}
	while ( (pIcon = __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_LEADING, XGE_XUI_INPUT_DECORATION_ICON)) != NULL ) {
		xgeXuiInputDecorationRemove(pInput, pIcon);
	}
	while ( (pIcon = __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_ICON)) != NULL ) {
		xgeXuiInputDecorationRemove(pInput, pIcon);
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XGE_XUI_INPUT_DECORATION_ICON;
	tDesc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDesc.fWidth = 22.0f;
	tDesc.fPadding = 4.0f;
	tDesc.iColor = pInput->iIconColor;
	tDesc.iHoverColor = pInput->iIconColor;
	tDesc.iActiveColor = pInput->iIconColor;
	tDesc.iDisabledColor = pInput->iDisabledTextColor;
	tDesc.iIcon = __xgeXuiInputIconClamp(iPrefixIcon);
	if ( tDesc.iIcon != XGE_XUI_INPUT_ICON_NONE ) {
		(void)xgeXuiInputDecorationAdd(pInput, XGE_XUI_INPUT_DECORATION_SIDE_LEADING, &tDesc);
	}
	tDesc.iIcon = __xgeXuiInputIconClamp(iSuffixIcon);
	if ( tDesc.iIcon != XGE_XUI_INPUT_ICON_NONE ) {
		(void)xgeXuiInputDecorationAdd(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tDesc);
	}
}

void xgeXuiInputSetIconColor(xge_xui_input pInput, uint32_t iColor)
{
	xge_xui_input_decoration pItem;

	if ( pInput == NULL ) {
		return;
	}
	pInput->iIconColor = iColor;
	for ( pItem = pInput->pLeadingDecoration; pItem != NULL; pItem = pItem->pNext ) {
		if ( pItem->iKind == XGE_XUI_INPUT_DECORATION_ICON ) {
			pItem->iColor = iColor;
			pItem->iHoverColor = iColor;
			pItem->iActiveColor = iColor;
		}
	}
	for ( pItem = pInput->pTrailingDecoration; pItem != NULL; pItem = pItem->pNext ) {
		if ( pItem->iKind == XGE_XUI_INPUT_DECORATION_ICON ) {
			pItem->iColor = iColor;
			pItem->iHoverColor = iColor;
			pItem->iActiveColor = iColor;
		}
	}
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

xge_rect_t xgeXuiInputGetPrefixIconRect(xge_xui_input pInput)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pInput != NULL ) {
		tRect = xgeXuiInputDecorationGetRect(pInput, __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_LEADING, XGE_XUI_INPUT_DECORATION_ICON));
	}
	return tRect;
}

xge_rect_t xgeXuiInputGetSuffixIconRect(xge_xui_input pInput)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pInput != NULL ) {
		tRect = xgeXuiInputDecorationGetRect(pInput, __xgeXuiInputDecorationFindKind(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_ICON));
	}
	return tRect;
}

void xgeXuiInputSetPassword(xge_xui_input pInput, int bPassword)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bPassword = (bPassword != 0);
	xgeXuiWidgetSetImeMode(pInput->pWidget, (pInput->bPassword || pInput->bReadonly) ? XGE_XUI_IME_DISABLED : XGE_XUI_IME_ENABLED);
	if ( pInput->bPassword != 0 ) {
		xgeXuiTextClearComposition(&pInput->tText);
	}
	__xgeXuiInputEnsureCursorVisible(pInput);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetReadonly(xge_xui_input pInput, int bReadonly)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->bReadonly = (bReadonly != 0);
	xgeXuiWidgetSetImeMode(pInput->pWidget, (pInput->bPassword || pInput->bReadonly) ? XGE_XUI_IME_DISABLED : XGE_XUI_IME_ENABLED);
	if ( pInput->bReadonly != 0 ) {
		xgeXuiTextClearComposition(&pInput->tText);
	}
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
	__xgeXuiInputSyncWidgetStyle(pInput);
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
	float fOriginX;
	float fTextW;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return tRect;
	}
	fTextW = __xgeXuiInputDisplayTextWidthForAlign(pInput);
	fOriginX = pInput->pWidget->tContentRect.fX + __xgeXuiInputTextAlignOffset(pInput, fTextW) - pInput->fScrollX;
	fCursorX = fOriginX + __xgeXuiInputDisplayPrefixWidth(pInput, xgeXuiTextGetCursor(&pInput->tText));
	if ( fCursorX > (pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW) ) {
		fCursorX = pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW;
	}
	if ( fCursorX < pInput->pWidget->tContentRect.fX ) {
		fCursorX = pInput->pWidget->tContentRect.fX;
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
	char* sBefore;

	if ( (pInput == NULL) || (pInput->bPassword != 0) || (pInput->bReadonly != 0) ) {
		return;
	}
	sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
	__xgeXuiInputCopySelection(pInput);
	__xgeXuiTextSelectionDelete(&pInput->tText);
	__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
}

static void __xgeXuiInputPasteClipboard(xge_xui_input pInput)
{
	const char* sClipboard;
	char* sBefore;

	if ( (pInput == NULL) || (pInput->bReadonly != 0) ) {
		return;
	}
	sClipboard = xgeClipboardGetText();
	if ( (sClipboard != NULL) && (sClipboard[0] != 0) ) {
		sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
		if ( xgeXuiTextInsert(&pInput->tText, sClipboard) == XGE_OK ) {
			__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
		} else if ( sBefore != NULL ) {
			xrtFree(sBefore);
		}
	}
}

static void __xgeXuiInputDeleteSelection(xge_xui_input pInput)
{
	char* sBefore;

	if ( (pInput == NULL) || (pInput->bReadonly != 0) ) {
		return;
	}
	sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
	if ( __xgeXuiTextSelectionDelete(&pInput->tText) == XGE_OK ) {
		__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
	} else if ( sBefore != NULL ) {
		xrtFree(sBefore);
	}
}

static void __xgeXuiInputMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xge_xui_input pInput;

	(void)pWidget;
	(void)iIndex;
	pInput = (xge_xui_input)pUser;
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	switch ( iValue ) {
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
	for ( iStart = 0; iStart < XGE_XUI_INPUT_MENU_COUNT; iStart++ ) {
		xgeXuiMenuSetItemState(pInput->pDefaultMenu, iStart, pInput->arrDefaultMenuEnabled[iStart] ? XGE_XUI_MENU_ITEM_ENABLED : 0);
	}
	xgeXuiMenuOpenAt(pInput->pDefaultMenu, pInput->pWidget, fX, fY);
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

static xge_xui_input_decoration __xgeXuiInputDecorationHitList(xge_xui_input pInput, xge_xui_input_decoration pDecoration, float fX, float fY)
{
	while ( pDecoration != NULL ) {
		if ( __xgeXuiInputDecorationIsVisible(pInput, pDecoration) && __xgeXuiRectContains(pDecoration->tRect, fX, fY) ) {
			return pDecoration;
		}
		pDecoration = pDecoration->pNext;
	}
	return NULL;
}

static xge_xui_input_decoration __xgeXuiInputDecorationHit(xge_xui_input pInput, float fX, float fY)
{
	xge_xui_input_decoration pHit;

	if ( pInput == NULL || (pInput->pLeadingDecoration == NULL && pInput->pTrailingDecoration == NULL) ) {
		return NULL;
	}
	__xgeXuiInputDecorationLayout(pInput);
	pHit = __xgeXuiInputDecorationHitList(pInput, pInput->pLeadingDecoration, fX, fY);
	if ( pHit != NULL ) {
		return pHit;
	}
	return __xgeXuiInputDecorationHitList(pInput, pInput->pTrailingDecoration, fX, fY);
}

static int __xgeXuiInputDecorationCanClick(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	if ( (pInput == NULL) || (pDecoration == NULL) || (pInput->bDisabled != 0) ) {
		return 0;
	}
	if ( pInput->bReadonly != 0 ) {
		return 0;
	}
	return (pDecoration->iKind == XGE_XUI_INPUT_DECORATION_CLEAR) || (pDecoration->procClick != NULL);
}

static void __xgeXuiInputDecorationClick(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	if ( !__xgeXuiInputDecorationCanClick(pInput, pDecoration) ) {
		return;
	}
	if ( pDecoration->iKind == XGE_XUI_INPUT_DECORATION_CLEAR ) {
		xgeXuiInputSetText(pInput, "");
		pInput->iClearCount++;
	}
	if ( pDecoration->procClick != NULL ) {
		pDecoration->procClick(pInput->pWidget, pDecoration->pUser);
	}
}

int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent)
{
	int iResult;
	int iCursor;
	int iStart;
	int iEnd;
	char* sBefore;
	xge_xui_input_decoration pDecoration;

	if ( (pInput == NULL) || (pInput->bInitialized == 0) || (pInput->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_IN ) {
		if ( pInput->bPassword != 0 ) {
			xgeXuiTextClearComposition(&pInput->tText);
		}
		__xgeXuiInputUpdatePadding(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		xgeXuiTextClearComposition(&pInput->tText);
		pInput->pHoverDecoration = NULL;
		pInput->pActiveDecoration = NULL;
		__xgeXuiInputUpdatePadding(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiInputDecorationLayout(pInput);
	if ( (pEvent->iType == XGE_EVENT_XUI_POINTER_ENTER) || ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		xgeXuiWidgetSetVisualState(pInput->pWidget, xgeXuiWidgetGetVisualState(pInput->pWidget) | XGE_XUI_STATE_HOVER);
	}
	if ( pEvent->iType == XGE_EVENT_XUI_POINTER_LEAVE ) {
		xgeXuiWidgetSetVisualState(pInput->pWidget, xgeXuiWidgetGetVisualState(pInput->pWidget) & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
		pInput->pHoverDecoration = NULL;
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_MOVE) && (pInput->pLeadingDecoration != NULL || pInput->pTrailingDecoration != NULL) ) {
		pDecoration = __xgeXuiInputDecorationHit(pInput, pEvent->fX, pEvent->fY);
		if ( pInput->pHoverDecoration != pDecoration ) {
			pInput->pHoverDecoration = pDecoration;
			xgeXuiWidgetMarkPaint(pInput->pWidget);
		}
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT)) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) ) {
		pDecoration = __xgeXuiInputDecorationHit(pInput, pEvent->fX, pEvent->fY);
		if ( __xgeXuiInputDecorationCanClick(pInput, pDecoration) ) {
			pInput->pActiveDecoration = pDecoration;
			pInput->pHoverDecoration = pDecoration;
			xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
			xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, pInput->pWidget);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END)) && (pInput->pActiveDecoration != NULL) ) {
		if ( xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) != pInput->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		pDecoration = __xgeXuiInputDecorationHit(pInput, pEvent->fX, pEvent->fY);
		if ( pDecoration == pInput->pActiveDecoration ) {
			__xgeXuiInputDecorationClick(pInput, pInput->pActiveDecoration);
		}
		pInput->pActiveDecoration = NULL;
		pInput->pHoverDecoration = pDecoration;
		xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, NULL);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_TOUCH_CANCEL) && (pInput->pActiveDecoration != NULL) ) {
		pInput->pActiveDecoration = NULL;
		xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, NULL);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (((pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT)) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		if ( (pInput->pDefaultMenu != NULL) && xgeXuiMenuIsOpen(pInput->pDefaultMenu) ) {
			xgeXuiMenuClose(pInput->pDefaultMenu);
		}
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		double fNow;
		int bDoubleClick;

		xgeXuiWidgetSetVisualState(pInput->pWidget, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
		if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
			xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
			__xgeXuiInputResetCursorBlink(pInput);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
		fNow = xgeTimer();
		bDoubleClick = ((fNow - pInput->fLastClickTime) <= 0.35) && ((pEvent->fX - pInput->fLastClickX) < 4.0f) && ((pInput->fLastClickX - pEvent->fX) < 4.0f) && ((pEvent->fY - pInput->fLastClickY) < 4.0f) && ((pInput->fLastClickY - pEvent->fY) < 4.0f);
		xgeXuiTextClearComposition(&pInput->tText);
		xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		if ( bDoubleClick != 0 ) {
			pInput->fLastClickTime = fNow;
			pInput->fLastClickX = pEvent->fX;
			pInput->fLastClickY = pEvent->fY;
			__xgeXuiInputSelectWordAt(pInput, iCursor);
			__xgeXuiInputEnsureCursorVisible(pInput);
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
			xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, pInput->pWidget);
		}
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pInput->bPressPending != 0) ) {
		float fDX = pEvent->fX - pInput->fPressX;
		float fDY = pEvent->fY - pInput->fPressY;
		if ( xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) != pInput->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
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
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pInput->bSelecting == 1) ) {
		if ( xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) != pInput->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		xgeXuiTextSetSelection(&pInput->tText, pInput->iPressCursor, iCursor);
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) || (pEvent->iType == XGE_EVENT_TOUCH_CANCEL)) && ((pInput->bSelecting != 0) || (pInput->bPressPending != 0)) ) {
		if ( xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) != pInput->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
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
		xgeXuiWidgetSetVisualState(pInput->pWidget, __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
		if ( pInput->pContext != NULL && xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) == pInput->pWidget ) {
			xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, NULL);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_CAPTURE_LOST || pEvent->iType == XGE_EVENT_XUI_CAPTURE_CANCEL ) {
		pInput->bPressPending = 0;
		pInput->bPressInsideSelection = 0;
		pInput->bSelecting = 0;
		pInput->pActiveDecoration = NULL;
		__xgeXuiInputResetCursorBlink(pInput);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_XUI_CONTEXT_BEGIN) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_UPDATE) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_END) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_CANCEL) ) {
		pInput->bPressPending = 0;
		pInput->bPressInsideSelection = 0;
		pInput->bSelecting = 0;
		xgeXuiWidgetSetVisualState(pInput->pWidget, XGE_XUI_STATE_NORMAL);
		if ( pInput->pContext != NULL && xgeXuiGetPointerCapture(pInput->pContext, pEvent->iPointerId) == pInput->pWidget ) {
			xgeXuiSetPointerCapture(pInput->pContext, pEvent->iPointerId, NULL);
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ENTER) ) {
		pInput->iSubmitCount++;
		if ( pInput->procSubmit != NULL ) {
			pInput->procSubmit(pInput->pWidget, xgeXuiInputGetText(pInput), pInput->pSubmitUser);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_IME_START) || (pEvent->iType == XGE_EVENT_IME_UPDATE) || (pEvent->iType == XGE_EVENT_IME_END) ) {
		if ( pInput->bPassword != 0 ) {
			xgeXuiTextClearComposition(&pInput->tText);
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pInput->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
		iResult = xgeXuiTextInputEvent(&pInput->tText, pEvent);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			if ( pEvent->iType == XGE_EVENT_IME_END ) {
				__xgeXuiInputEnsureCursorVisible(pInput);
				__xgeXuiInputResetCursorBlink(pInput);
				xgeXuiWidgetMarkPaint(pInput->pWidget);
				if ( sBefore != NULL ) {
					xrtFree(sBefore);
				}
			} else {
				__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
			}
		} else if ( sBefore != NULL ) {
			xrtFree(sBefore);
		}
		return iResult;
	}
	if ( pEvent->iType == XGE_EVENT_TEXT ) {
		if ( (pInput->bPassword != 0) && (pEvent->iCodepoint >= 0x80) ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pInput->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
		iResult = xgeXuiTextInputEvent(&pInput->tText, pEvent);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
		} else if ( sBefore != NULL ) {
			xrtFree(sBefore);
		}
		return iResult;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && __xgeXuiInputIsCtrl(pEvent) && (pEvent->iParam1 == 'A' || pEvent->iParam1 == 'a') ) {
		xgeXuiTextSetSelection(&pInput->tText, 0, pInput->tText.iSize);
		__xgeXuiInputEnsureCursorVisible(pInput);
		__xgeXuiInputResetCursorBlink(pInput);
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
		sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
		if ( xgeXuiTextDeleteBack(&pInput->tText) == XGE_OK ) {
			__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
			return XGE_XUI_EVENT_CONSUMED;
		} else if ( sBefore != NULL ) {
			xrtFree(sBefore);
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_DELETE) ) {
		sBefore = __xgeXuiInputTextDup(xgeXuiInputGetText(pInput));
		if ( xgeXuiTextDeleteForward(&pInput->tText) == XGE_OK ) {
			__xgeXuiInputAfterTextMutation(pInput, sBefore, 0);
			return XGE_XUI_EVENT_CONSUMED;
		} else if ( sBefore != NULL ) {
			xrtFree(sBefore);
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

static uint32_t __xgeXuiInputDecorationColor(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	if ( (pInput == NULL) || (pDecoration == NULL) ) {
		return 0;
	}
	if ( pInput->bDisabled != 0 && XGE_COLOR_GET_A(pDecoration->iDisabledColor) != 0 ) {
		return pDecoration->iDisabledColor;
	}
	if ( pInput->pActiveDecoration == pDecoration && XGE_COLOR_GET_A(pDecoration->iActiveColor) != 0 ) {
		return pDecoration->iActiveColor;
	}
	if ( pInput->pHoverDecoration == pDecoration && XGE_COLOR_GET_A(pDecoration->iHoverColor) != 0 ) {
		return pDecoration->iHoverColor;
	}
	return pDecoration->iColor;
}

static void __xgeXuiInputDecorationPaintItem(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	static const uint16_t arrClear10[10] = {
		0x201, 0x102, 0x084, 0x048, 0x030,
		0x030, 0x048, 0x084, 0x102, 0x201
	};
	xge_draw_t tDraw;
	xge_rect_t tRect;
	const uint16_t* arrIcon;
	uint32_t iColor;
	int iIconW;
	int iIconH;

	if ( (pInput == NULL) || (pDecoration == NULL) || (!__xgeXuiInputDecorationIsVisible(pInput, pDecoration)) ) {
		return;
	}
	iColor = __xgeXuiInputDecorationColor(pInput, pDecoration);
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	tRect = pDecoration->tRect;
	tRect.fX += pDecoration->fPadding;
	tRect.fY += pDecoration->fPadding;
	tRect.fW -= pDecoration->fPadding * 2.0f;
	tRect.fH -= pDecoration->fPadding * 2.0f;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	switch ( pDecoration->iKind ) {
		case XGE_XUI_INPUT_DECORATION_ICON:
			arrIcon = __xgeXuiInputIconMask(__xgeXuiInputIconClamp(pDecoration->iIcon), &iIconW, &iIconH);
			if ( arrIcon != NULL ) {
				tRect.fW = 12.0f;
				tRect.fH = 12.0f;
				tRect.fX = pDecoration->tRect.fX + (pDecoration->tRect.fW - tRect.fW) * 0.5f;
				tRect.fY = pDecoration->tRect.fY + (pDecoration->tRect.fH - tRect.fH) * 0.5f;
				__xgeXuiHostDrawBitmapMask(tRect, arrIcon, iIconW, iIconH, iColor);
			}
			break;
		case XGE_XUI_INPUT_DECORATION_TEXT:
			if ( (pInput->pFont != NULL) && (pDecoration->sText != NULL) ) {
				__xgeXuiHostDrawTextRect(pInput->pFont, pDecoration->sText, tRect, iColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
			break;
		case XGE_XUI_INPUT_DECORATION_TEXTURE:
			if ( pDecoration->pTexture != NULL ) {
				memset(&tDraw, 0, sizeof(tDraw));
				tDraw.pTexture = pDecoration->pTexture;
				tDraw.tDst = tRect;
				tDraw.tSrc = pDecoration->tSrc;
				tDraw.iColor = iColor;
				__xgeXuiHostDrawImage(&tDraw);
			}
			break;
		case XGE_XUI_INPUT_DECORATION_CLEAR:
			tRect.fW = 10.0f;
			tRect.fH = 10.0f;
			tRect.fX = pDecoration->tRect.fX + (pDecoration->tRect.fW - tRect.fW) * 0.5f;
			tRect.fY = pDecoration->tRect.fY + (pDecoration->tRect.fH - tRect.fH) * 0.5f;
			__xgeXuiHostDrawBitmapMask(tRect, arrClear10, 10, 10, iColor);
			break;
		case XGE_XUI_INPUT_DECORATION_CUSTOM_PAINT:
			if ( pDecoration->procPaint != NULL ) {
				pDecoration->procPaint(pInput, pDecoration, pDecoration->tRect, pDecoration->iState, pDecoration->pUser);
			}
			break;
		default:
			break;
	}
}

static void __xgeXuiInputDecorationPaintList(xge_xui_input pInput, xge_xui_input_decoration pDecoration)
{
	while ( pDecoration != NULL ) {
		__xgeXuiInputDecorationPaintItem(pInput, pDecoration);
		pDecoration = pDecoration->pNext;
	}
}

static void __xgeXuiInputDecorationPaint(xge_xui_input pInput)
{
	xge_rect_t tOldOuterClip;
	int bOldOuterClipEnabled;
	int bOuterClip;

	if ( (pInput == NULL) || (pInput->pWidget == NULL) || (pInput->pLeadingDecoration == NULL && pInput->pTrailingDecoration == NULL) ) {
		return;
	}
	__xgeXuiInputDecorationLayout(pInput);
	bOuterClip = __xgeXuiInputBeginOuterPaint(pInput->pContext, pInput->pWidget->tBorderRect, &tOldOuterClip, &bOldOuterClipEnabled);
	__xgeXuiInputDecorationPaintList(pInput, pInput->pLeadingDecoration);
	__xgeXuiInputDecorationPaintList(pInput, pInput->pTrailingDecoration);
	if ( bOuterClip != 0 ) {
		__xgeXuiInputEndOuterPaint(&tOldOuterClip, bOldOuterClipEnabled);
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
	float fOriginX;
	float fTextW;
	float fDrawTextW;
	float fStartX;
	float fEndX;
	int iStart;
	int iEnd;

	pInput = (xge_xui_input)pUser;
	if ( (pWidget == NULL) || (pInput == NULL) ) {
		return;
	}
	__xgeXuiInputDecorationLayout(pInput);
	__xgeXuiInputDecorationPaint(pInput);
	fTextW = __xgeXuiInputDisplayTextWidthForAlign(pInput);
	fOriginX = pWidget->tContentRect.fX + __xgeXuiInputTextAlignOffset(pInput, fTextW) - pInput->fScrollX;
	xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
	if ( (iStart != iEnd) && (XGE_COLOR_GET_A(pInput->iSelectionColor) != 0) ) {
		fStartX = fOriginX + __xgeXuiInputDisplayPrefixWidth(pInput, iStart);
		fEndX = fOriginX + __xgeXuiInputDisplayPrefixWidth(pInput, iEnd);
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
		fDrawTextW = __xgeXuiHostMeasureText(pInput->pFont, sDrawText).fX;
		tTextRect.fX += __xgeXuiInputTextAlignOffset(pInput, fDrawTextW);
		if ( sDrawText != pInput->sPlaceholder ) {
			tTextRect.fX -= pInput->fScrollX;
			tTextRect.fW += pInput->fScrollX;
		}
		__xgeXuiHostDrawTextRect(pInput->pFont, sDrawText, tTextRect, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( sPassword != NULL ) {
		xrtFree(sPassword);
	}
	if ( (pInput->pFont != NULL) && (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
		tComposition = xgeXuiInputGetCandidateRect(pInput);
		tComposition.fX += 1.0f;
		tComposition.fW = (pWidget->tContentRect.fX + pWidget->tContentRect.fW) - tComposition.fX;
		if ( tComposition.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pInput->pFont, pInput->tText.sComposition, tComposition, pInput->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus == pWidget) && (pInput->bCursorVisible != 0) && (XGE_COLOR_GET_A(pInput->iCursorColor) != 0) ) {
		tSize.fX = __xgeXuiInputDisplayPrefixWidth(pInput, xgeXuiTextGetCursor(&pInput->tText));
		if ( (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
			tSize.fX += __xgeXuiTextPrefixWidth(pInput->pFont, pInput->tText.sComposition, pInput->tText.iCompositionSize);
		}
		tSize.fY = 0.0f;
		tCursor.fX = fOriginX + tSize.fX + 1.0f;
		if ( tCursor.fX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f) ) {
			tCursor.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
		}
		if ( tCursor.fX < pWidget->tContentRect.fX ) {
			tCursor.fX = pWidget->tContentRect.fX;
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
