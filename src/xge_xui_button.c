static int __xgeXuiButtonStateStyleIndex(int iState)
{
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return 4;
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return 2;
	}
	if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return 1;
	}
	if ( (iState & XGE_XUI_STATE_CHECKED) != 0 ) {
		return 5;
	}
	return 0;
}

static int __xgeXuiButtonPatchIndex(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return 0;
	}
	if ( ((pButton->iState & XGE_XUI_STATE_DISABLED) != 0) && pButton->arrHasPatch[4] ) {
		return 4;
	}
	if ( ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0) && pButton->arrHasPatch[2] ) {
		return 2;
	}
	if ( ((pButton->iState & XGE_XUI_STATE_CHECKED) != 0) && pButton->arrHasPatch[5] ) {
		return 5;
	}
	if ( ((pButton->iState & XGE_XUI_STATE_HOVER) != 0) && pButton->arrHasPatch[1] ) {
		return 1;
	}
	if ( ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0) && pButton->arrHasPatch[1] ) {
		return 1;
	}
	return pButton->arrHasPatch[0] ? 0 : -1;
}

typedef struct xge_xui_button_cache_paint_t {
	xge_xui_button pButton;
	int iPatch;
	xge_rect_t tBorderRect;
	xge_rect_t tContentRect;
} xge_xui_button_cache_paint_t;

static uint32_t __xgeXuiButtonTextColor(xge_xui_button pButton);
static xge_texture __xgeXuiButtonBadgeTexture(void);
static void __xgeXuiButtonLayoutContent(xge_xui_button pButton, xge_rect_t tContent);
static void __xgeXuiButtonLayoutBadge(xge_xui_button pButton, xge_rect_t tContent);

static int __xgeXuiButtonCacheIndexFromState(int iState)
{
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return 4;
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return 3;
	}
	if ( (iState & XGE_XUI_STATE_CHECKED) != 0 ) {
		return 1;
	}
	if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return 2;
	}
	return 0;
}

static void __xgeXuiButtonCacheInit(xge_xui_button pButton)
{
	int i;

	if ( pButton == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_BUTTON_CACHE_STATE_COUNT; i++ ) {
		__xgeXuiRenderCacheInit(&pButton->arrCache[i]);
	}
}

static void __xgeXuiButtonCacheUnit(xge_xui_button pButton)
{
	int i;

	if ( pButton == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_BUTTON_CACHE_STATE_COUNT; i++ ) {
		__xgeXuiRenderCacheUnit(&pButton->arrCache[i]);
	}
}

static void __xgeXuiButtonCacheInvalidate(xge_xui_button pButton)
{
	int i;

	if ( pButton == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_BUTTON_CACHE_STATE_COUNT; i++ ) {
		__xgeXuiRenderCacheInvalidate(&pButton->arrCache[i]);
	}
}

static void __xgeXuiButtonNinePatchDrawProc(const xge_draw_t* pDraw, void* pUser)
{
	(void)pUser;
	__xgeXuiHostDrawImage(pDraw);
}

static void __xgeXuiButtonDrawPatch(const xge_nine_patch_t* pPatch, xge_rect_t tDst)
{
	__xgeNinePatchDrawInternal(pPatch, tDst, XGE_DRAW_SCREEN_SPACE, XGE_COLOR_RGBA(255, 255, 255, 255), __xgeXuiButtonNinePatchDrawProc, NULL);
}

static void __xgeXuiButtonPaintCacheContent(xge_rect_t tRect, void* pUser)
{
	xge_xui_button_cache_paint_t* pPaint;
	xge_xui_button pButton;
	xge_draw_t tDraw;
	xge_texture pBadgeTexture;
	xge_rect_t tBadgeSrc;
	xge_rect_t tLocalContent;
	xge_rect_t tOldIconRect;
	xge_rect_t tOldTextRect;
	xge_rect_t tOldContentGroupRect;
	xge_rect_t tOldBadgeRect;
	int bHasIcon;
	int bHasText;

	pPaint = (xge_xui_button_cache_paint_t*)pUser;
	if ( (pPaint == NULL) || (pPaint->pButton == NULL) ) {
		return;
	}
	pButton = pPaint->pButton;
	tOldIconRect = pButton->tIconRect;
	tOldTextRect = pButton->tTextRect;
	tOldContentGroupRect = pButton->tContentGroupRect;
	tOldBadgeRect = pButton->tBadgeRect;
	if ( (pPaint->iPatch >= 0) && (pPaint->iPatch < XGE_XUI_WIDGET_STATE_STYLE_COUNT) ) {
		xgeNinePatchDraw(&pButton->arrPatch[pPaint->iPatch], tRect, XGE_DRAW_SCREEN_SPACE);
	}
	tLocalContent = pPaint->tContentRect;
	tLocalContent.fX -= pPaint->tBorderRect.fX;
	tLocalContent.fY -= pPaint->tBorderRect.fY;
	if ( (tLocalContent.fW <= 0.0f) || (tLocalContent.fH <= 0.0f) ) {
		tLocalContent = tRect;
	}
	__xgeXuiButtonLayoutContent(pButton, tLocalContent);
	bHasIcon = (pButton->pIconTexture != NULL) && (pButton->fIconSize > 0.0f);
	bHasText = (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0);
	if ( bHasIcon && (XGE_COLOR_GET_A(pButton->iIconColor) != 0) ) {
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = pButton->pIconTexture;
		tDraw.tSrc = pButton->tIconSrc;
		tDraw.tDst = pButton->tIconRect;
		tDraw.iColor = pButton->iIconColor;
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
	}
	if ( bHasText ) {
		xgeTextDrawRect(
			pButton->pFont,
			pButton->sText,
			bHasIcon ? pButton->tTextRect : tLocalContent,
			__xgeXuiButtonTextColor(pButton),
			(bHasIcon ? (XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP) : pButton->iTextFlags) | XGE_TEXT_SCREEN_SPACE);
	}
	if ( pButton->bBadgeVisible != 0 ) {
		__xgeXuiButtonLayoutBadge(pButton, tLocalContent);
		if ( pButton->iBadgeAnchor == XGE_XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT ) {
			pButton->tBadgeRect.fX -= pPaint->tBorderRect.fX;
			pButton->tBadgeRect.fY -= pPaint->tBorderRect.fY;
		}
		if ( pButton->pBadgeTexture != NULL ) {
			pBadgeTexture = pButton->pBadgeTexture;
			tBadgeSrc = pButton->tBadgeSrc;
		} else {
			pBadgeTexture = __xgeXuiButtonBadgeTexture();
			tBadgeSrc = __xgeXuiBuiltinAssetSrc(XGE_XUI_ASSET_BUTTON_BADGE);
		}
		if ( pBadgeTexture != NULL ) {
			memset(&tDraw, 0, sizeof(tDraw));
			tDraw.pTexture = pBadgeTexture;
			tDraw.tSrc = tBadgeSrc;
			tDraw.tDst = pButton->tBadgeRect;
			tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
			xgeDrawEx(&tDraw);
		}
	}
	pButton->tIconRect = tOldIconRect;
	pButton->tTextRect = tOldTextRect;
	pButton->tContentGroupRect = tOldContentGroupRect;
	pButton->tBadgeRect = tOldBadgeRect;
}

static int __xgeXuiButtonDrawStateCache(xge_xui_button pButton, xge_xui_widget pWidget, int iPatch)
{
	xge_xui_button_cache_paint_t tPaint;
	xge_draw_t tDraw;
	xge_texture pTexture;
	float fDipScale;
	int iCache;

	if ( (pButton == NULL) || (pWidget == NULL) || (pButton->iCacheMode == XGE_XUI_CACHE_OFF) || (pWidget->tBorderRect.fW <= 0.0f) || (pWidget->tBorderRect.fH <= 0.0f) ) {
		return 0;
	}
	iCache = __xgeXuiButtonCacheIndexFromState(pButton->iState);
	if ( (iCache < 0) || (iCache >= XGE_XUI_BUTTON_CACHE_STATE_COUNT) ) {
		return 0;
	}
	fDipScale = (pButton->pContext != NULL && pButton->pContext->fDipScale > 0.0f) ? pButton->pContext->fDipScale : 1.0f;
	memset(&tPaint, 0, sizeof(tPaint));
	tPaint.pButton = pButton;
	tPaint.iPatch = iPatch;
	tPaint.tBorderRect = pWidget->tBorderRect;
	tPaint.tContentRect = pWidget->tContentRect;
	pTexture = __xgeXuiRenderCacheEnsure(&pButton->arrCache[iCache], pWidget->tBorderRect, fDipScale, __xgeXuiButtonPaintCacheContent, &tPaint);
	if ( pTexture == NULL ) {
		return 0;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst = pWidget->tBorderRect;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE | XGE_DRAW_FLIP_Y;
	__xgeXuiHostDrawImage(&tDraw);
	return 1;
}

static uint32_t __xgeXuiButtonTextColor(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pButton->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return XGE_COLOR_RGBA(XGE_COLOR_GET_R(pButton->iTextColor), XGE_COLOR_GET_G(pButton->iTextColor), XGE_COLOR_GET_B(pButton->iTextColor), 128);
	}
	return pButton->iTextColor;
}

static xge_texture __xgeXuiButtonBadgeTexture(void)
{
	return __xgeXuiBuiltinAtlasTexture();
}

int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pButton == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pButton, 0, sizeof(*pButton));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pButton->pContext = pContext;
	pButton->pWidget = pWidget;
	pButton->pFont = pTheme->pFont;
	pButton->sText = "";
	pButton->iTextColor = pTheme->iTextColor;
	pButton->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pButton->iIconColor = pTheme->iTextColor;
	pButton->fIconSize = 16.0f;
	pButton->fIconGap = 6.0f;
	pButton->iIconPlacement = XGE_XUI_BUTTON_ICON_LEFT;
	pButton->iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
	pButton->iColorNormal = pTheme->iStateNormal;
	pButton->iColorHover = pTheme->iStateHover;
	pButton->iColorActive = pTheme->iStateActive;
	pButton->iColorFocus = pTheme->iStateFocus;
	pButton->iColorDisabled = pTheme->iStateDisabled;
	pButton->iColorChecked = pTheme->iAccentColor;
	pButton->iCacheMode = XGE_XUI_CACHE_AUTO;
	pButton->iBadgeAnchor = XGE_XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT;
	pButton->fBadgeSize = 12.0f;
	__xgeXuiButtonCacheInit(pButton);
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetBackground(pWidget, pButton->iColorNormal);
	xgeXuiWidgetSetBorder(pWidget, pTheme->fBorderWidth, pTheme->iBorderColor);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_HOVER, pButton->iColorHover);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_ACTIVE, pButton->iColorActive);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_DISABLED, pButton->iColorDisabled);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_CHECKED, pButton->iColorChecked);
	xgeXuiWidgetSetFocusRing(pWidget, 1.0f, pTheme->iStateFocus);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiButtonEventProc, pButton);
	pWidget->procPaint = xgeXuiButtonPaintProc;
	pWidget->pUser = pButton;
	__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiButtonUnit(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( pButton->pWidget != NULL && pButton->pWidget->pUser == pButton ) {
		pButton->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pButton->pWidget, NULL, NULL);
		pButton->pWidget->procPaint = NULL;
	}
	__xgeXuiButtonCacheUnit(pButton);
	memset(pButton, 0, sizeof(*pButton));
}

void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->procClick = procClick;
	pButton->pUser = pUser;
}

void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pFont = pFont;
	pButton->sText = (sText != NULL) ? sText : "";
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkLayout(pButton->pWidget);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iTextColor = iColor;
	pButton->iIconColor = iColor;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetSelectable(xge_xui_button pButton, int bSelectable)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bSelectable = (bSelectable != 0);
	if ( pButton->bSelectable == 0 ) {
		pButton->bSelected = 0;
	}
	__xgeXuiButtonSetState(pButton, pButton->iState);
}

void xgeXuiButtonSetSelected(xge_xui_button pButton, int bSelected)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bSelectable = 1;
	pButton->bSelected = (bSelected != 0);
	__xgeXuiButtonSetState(pButton, pButton->iState);
}

int xgeXuiButtonIsSelected(xge_xui_button pButton)
{
	return (pButton != NULL) ? pButton->bSelected : 0;
}

void xgeXuiButtonSetSemantic(xge_xui_button pButton, int iSemantic)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( iSemantic == XGE_XUI_BUTTON_SEMANTIC_PRIMARY ) {
		xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(35, 132, 214, 255), XGE_COLOR_RGBA(56, 151, 228, 255), XGE_COLOR_RGBA(22, 104, 176, 255), pButton->iColorFocus, XGE_COLOR_RGBA(188, 210, 228, 255));
		xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	} else if ( iSemantic == XGE_XUI_BUTTON_SEMANTIC_DANGER ) {
		xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(214, 72, 86, 255), XGE_COLOR_RGBA(228, 92, 104, 255), XGE_COLOR_RGBA(178, 52, 66, 255), pButton->iColorFocus, XGE_COLOR_RGBA(230, 200, 204, 255));
		xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(255, 250, 250, 255));
	} else {
		const xge_xui_theme_t* pTheme = xgeXuiGetTheme(pButton->pContext);
		iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
		xgeXuiButtonSetColors(pButton, pTheme->iStateNormal, pTheme->iStateHover, pTheme->iStateActive, pTheme->iStateFocus, pTheme->iStateDisabled);
		xgeXuiButtonSetTextColor(pButton, pTheme->iTextColor);
	}
	pButton->iSemantic = iSemantic;
}

int xgeXuiButtonGetSemantic(xge_xui_button pButton)
{
	return (pButton != NULL) ? pButton->iSemantic : XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
}

void xgeXuiButtonSetIcon(xge_xui_button pButton, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pIconTexture = pTexture;
	pButton->tIconSrc = tSrc;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetIconColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iIconColor = iColor;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetIconLayout(xge_xui_button pButton, int iPlacement, float fIconSize, float fGap)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( (iPlacement < XGE_XUI_BUTTON_ICON_LEFT) || (iPlacement > XGE_XUI_BUTTON_ICON_BOTTOM) ) {
		iPlacement = XGE_XUI_BUTTON_ICON_LEFT;
	}
	pButton->iIconPlacement = iPlacement;
	pButton->fIconSize = (fIconSize > 0.0f) ? fIconSize : 16.0f;
	pButton->fIconGap = (fGap > 0.0f) ? fGap : 0.0f;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iColorNormal = iNormal;
	pButton->iColorHover = iHover;
	pButton->iColorActive = iActive;
	pButton->iColorFocus = iFocus;
	pButton->iColorDisabled = iDisabled;
	pButton->iColorChecked = iActive;
	xgeXuiWidgetSetBackground(pButton->pWidget, iNormal);
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_HOVER, iHover);
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_ACTIVE, iActive);
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_DISABLED, iDisabled);
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_CHECKED, iActive);
	xgeXuiWidgetSetFocusRing(pButton->pWidget, 1.0f, iFocus);
	pButton->iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetPatch(xge_xui_button pButton, int iState, const xge_nine_patch_t* pPatch)
{
	int iIndex;

	if ( pButton == NULL ) {
		return;
	}
	iIndex = __xgeXuiButtonStateStyleIndex(iState);
	if ( pPatch != NULL ) {
		pButton->arrPatch[iIndex] = *pPatch;
		pButton->arrHasPatch[iIndex] = 1;
	} else {
		xgeXuiButtonClearPatch(pButton, iState);
		return;
	}
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonClearPatch(xge_xui_button pButton, int iState)
{
	int iIndex;

	if ( pButton == NULL ) {
		return;
	}
	iIndex = __xgeXuiButtonStateStyleIndex(iState);
	memset(&pButton->arrPatch[iIndex], 0, sizeof(pButton->arrPatch[iIndex]));
	pButton->arrHasPatch[iIndex] = 0;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetCacheMode(xge_xui_button pButton, int iMode)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( (iMode != XGE_XUI_CACHE_AUTO) && (iMode != XGE_XUI_CACHE_OFF) && (iMode != XGE_XUI_CACHE_FORCE) ) {
		iMode = XGE_XUI_CACHE_AUTO;
	}
	if ( pButton->iCacheMode == iMode ) {
		return;
	}
	pButton->iCacheMode = iMode;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetBadgeVisible(xge_xui_button pButton, int bVisible)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bBadgeVisible = (bVisible != 0);
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetBadgeAnchor(xge_xui_button pButton, int iAnchor)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( (iAnchor < XGE_XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT) || (iAnchor > XGE_XUI_BUTTON_BADGE_TEXT_TOP_RIGHT) ) {
		iAnchor = XGE_XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT;
	}
	pButton->iBadgeAnchor = iAnchor;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetBadgeOffset(xge_xui_button pButton, float fX, float fY)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->fBadgeOffsetX = fX;
	pButton->fBadgeOffsetY = fY;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetBadgeSize(xge_xui_button pButton, float fSize)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->fBadgeSize = (fSize > 0.0f) ? fSize : 12.0f;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetBadgeTexture(xge_xui_button pButton, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pBadgeTexture = pTexture;
	pButton->tBadgeSrc = tSrc;
	__xgeXuiButtonCacheInvalidate(pButton);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetState(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiButtonSetState(pButton, pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pButton->iState;
}

int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pButton == NULL) || (pButton->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pButton->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pButton->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiButtonSetState(pButton, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiButtonSetState(pButton, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiButtonSetState(pButton, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiButtonSetState(pButton, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, pButton->pWidget);
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiButtonSetState(pButton, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && xgeXuiGetPointerCapture(pButton->pContext, pEvent->iPointerId) == pButton->pWidget ) {
				xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, NULL);
			}
			if ( bWasActive && iInside ) {
				if ( pButton->bSelectable != 0 ) {
					pButton->bSelected = (pButton->bSelected == 0);
					__xgeXuiButtonSetState(pButton, pButton->iState);
				}
				pButton->iClickCount++;
				if ( pButton->procClick != NULL ) {
					pButton->procClick(pButton->pWidget, pButton->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && xgeXuiGetPointerCapture(pButton->pContext, pEvent->iPointerId) == pButton->pWidget ) {
				xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pButton->pContext == NULL) || (pButton->pContext->pFocus != pButton->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pButton->bSelectable != 0 ) {
				pButton->bSelected = (pButton->bSelected == 0);
				__xgeXuiButtonSetState(pButton, pButton->iState);
			}
			pButton->iClickCount++;
			if ( pButton->procClick != NULL ) {
				pButton->procClick(pButton->pWidget, pButton->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiButtonEvent((xge_xui_button)pUser, pEvent);
}

static void __xgeXuiButtonLayoutContent(xge_xui_button pButton, xge_rect_t tContent)
{
	xge_vec2_t tTextSize;
	float fIconSize;
	float fTextW;
	float fTextH;
	float fGap;
	float fGroupW;
	float fGroupH;
	float fX;
	float fY;
	int bHasText;
	int bHasIcon;
	int bVertical;

	pButton->tIconRect = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	pButton->tTextRect = tContent;
	pButton->tContentGroupRect = tContent;
	bHasText = (pButton->sText != NULL) && (pButton->sText[0] != 0) && (pButton->pFont != NULL);
	bHasIcon = (pButton->pIconTexture != NULL) && (pButton->fIconSize > 0.0f);
	if ( !bHasText && !bHasIcon ) {
		pButton->tContentGroupRect = (xge_rect_t){ tContent.fX + tContent.fW * 0.5f, tContent.fY + tContent.fH * 0.5f, 0.0f, 0.0f };
		return;
	}
	fIconSize = pButton->fIconSize;
	if ( fIconSize > tContent.fW ) {
		fIconSize = tContent.fW;
	}
	if ( fIconSize > tContent.fH ) {
		fIconSize = tContent.fH;
	}
	fTextW = 0.0f;
	fTextH = 0.0f;
	if ( bHasText ) {
		tTextSize = __xgeXuiHostMeasureText(pButton->pFont, pButton->sText);
		fTextW = (tTextSize.fX > 0.0f) ? tTextSize.fX : (float)strlen(pButton->sText) * 6.0f;
		fTextH = (tTextSize.fY > 0.0f) ? tTextSize.fY : tContent.fH;
	}
	if ( !bHasIcon ) {
		pButton->tTextRect = tContent;
		pButton->tContentGroupRect = tContent;
		return;
	}
	if ( !bHasText ) {
		pButton->tIconRect = (xge_rect_t){ tContent.fX + (tContent.fW - fIconSize) * 0.5f, tContent.fY + (tContent.fH - fIconSize) * 0.5f, fIconSize, fIconSize };
		pButton->tContentGroupRect = pButton->tIconRect;
		return;
	}
	bVertical = (pButton->iIconPlacement == XGE_XUI_BUTTON_ICON_TOP) || (pButton->iIconPlacement == XGE_XUI_BUTTON_ICON_BOTTOM);
	fGap = pButton->fIconGap;
	if ( bVertical ) {
		if ( fTextW > tContent.fW ) {
			fTextW = tContent.fW;
		}
		fGroupW = (fTextW > fIconSize) ? fTextW : fIconSize;
		fGroupH = fIconSize + fGap + fTextH;
		if ( fGroupH > tContent.fH ) {
			fGroupH = tContent.fH;
			fTextH = tContent.fH - fIconSize - fGap;
			if ( fTextH < 0.0f ) {
				fTextH = 0.0f;
			}
		}
		fX = tContent.fX + (tContent.fW - fGroupW) * 0.5f;
		fY = tContent.fY + (tContent.fH - fGroupH) * 0.5f;
		if ( pButton->iIconPlacement == XGE_XUI_BUTTON_ICON_BOTTOM ) {
			pButton->tTextRect = (xge_rect_t){ fX, fY, fGroupW, fTextH };
			pButton->tIconRect = (xge_rect_t){ fX + (fGroupW - fIconSize) * 0.5f, fY + fTextH + fGap, fIconSize, fIconSize };
		} else {
			pButton->tIconRect = (xge_rect_t){ fX + (fGroupW - fIconSize) * 0.5f, fY, fIconSize, fIconSize };
			pButton->tTextRect = (xge_rect_t){ fX, fY + fIconSize + fGap, fGroupW, fTextH };
		}
		pButton->tContentGroupRect = (xge_rect_t){ fX, fY, fGroupW, fGroupH };
	} else {
		if ( fTextW > tContent.fW - fIconSize - fGap ) {
			fTextW = tContent.fW - fIconSize - fGap;
		}
		if ( fTextW < 0.0f ) {
			fTextW = 0.0f;
		}
		fGroupW = fIconSize + fGap + fTextW;
		fGroupH = (fTextH > fIconSize) ? fTextH : fIconSize;
		if ( fGroupH > tContent.fH ) {
			fGroupH = tContent.fH;
		}
		fX = tContent.fX + (tContent.fW - fGroupW) * 0.5f;
		fY = tContent.fY + (tContent.fH - fGroupH) * 0.5f;
		if ( pButton->iIconPlacement == XGE_XUI_BUTTON_ICON_RIGHT ) {
			pButton->tTextRect = (xge_rect_t){ fX, fY, fTextW, fGroupH };
			pButton->tIconRect = (xge_rect_t){ fX + fTextW + fGap, fY + (fGroupH - fIconSize) * 0.5f, fIconSize, fIconSize };
		} else {
			pButton->tIconRect = (xge_rect_t){ fX, fY + (fGroupH - fIconSize) * 0.5f, fIconSize, fIconSize };
			pButton->tTextRect = (xge_rect_t){ fX + fIconSize + fGap, fY, fTextW, fGroupH };
		}
		pButton->tContentGroupRect = (xge_rect_t){ fX, fY, fGroupW, fGroupH };
	}
}

static void __xgeXuiButtonLayoutBadge(xge_xui_button pButton, xge_rect_t tContent)
{
	xge_rect_t tAnchorRect;
	float fSize;
	float fX;
	float fY;

	if ( pButton == NULL ) {
		return;
	}
	tAnchorRect = pButton->tContentGroupRect;
	if ( pButton->iBadgeAnchor == XGE_XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT ) {
		tAnchorRect = pButton->pWidget->tBorderRect;
	} else if ( (pButton->iBadgeAnchor == XGE_XUI_BUTTON_BADGE_ICON_TOP_RIGHT) && (pButton->tIconRect.fW > 0.0f) ) {
		tAnchorRect = pButton->tIconRect;
	} else if ( (pButton->iBadgeAnchor == XGE_XUI_BUTTON_BADGE_TEXT_TOP_RIGHT) && (pButton->tTextRect.fW > 0.0f) ) {
		tAnchorRect = pButton->tTextRect;
	}
	if ( (tAnchorRect.fW <= 0.0f) && (tAnchorRect.fH <= 0.0f) ) {
		tAnchorRect = tContent;
	}
	fSize = (pButton->fBadgeSize > 0.0f) ? pButton->fBadgeSize : 12.0f;
	fX = tAnchorRect.fX + tAnchorRect.fW - fSize * 0.5f + pButton->fBadgeOffsetX;
	fY = tAnchorRect.fY + fSize * 0.5f + pButton->fBadgeOffsetY;
	pButton->tBadgeRect = (xge_rect_t){ fX - fSize * 0.5f, fY - fSize * 0.5f, fSize, fSize };
}

void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	xge_draw_t tDraw;
	xge_rect_t tContent;
	xge_texture pBadgeTexture;
	xge_rect_t tBadgeSrc;
	int iPatch;
	int bHasIcon;
	int bHasText;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		tContent = pWidget->tRect;
	}
	iPatch = __xgeXuiButtonPatchIndex(pButton);
	if ( __xgeXuiButtonDrawStateCache(pButton, pWidget, iPatch) ) {
		return;
	}
	if ( iPatch >= 0 ) {
		__xgeXuiButtonDrawPatch(&pButton->arrPatch[iPatch], pWidget->tBorderRect);
	}
	__xgeXuiButtonLayoutContent(pButton, tContent);
	bHasIcon = (pButton->pIconTexture != NULL) && (pButton->fIconSize > 0.0f);
	bHasText = (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0);
	if ( bHasIcon && (XGE_COLOR_GET_A(pButton->iIconColor) != 0) ) {
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = pButton->pIconTexture;
		tDraw.tSrc = pButton->tIconSrc;
		tDraw.tDst = pButton->tIconRect;
		tDraw.iColor = pButton->iIconColor;
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		__xgeXuiHostDrawImage(&tDraw);
	}
	if ( bHasText ) {
		__xgeXuiHostDrawTextRect(pButton->pFont, pButton->sText, bHasIcon ? pButton->tTextRect : tContent, __xgeXuiButtonTextColor(pButton), bHasIcon ? (XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP) : pButton->iTextFlags);
	}
	if ( pButton->bBadgeVisible != 0 ) {
		__xgeXuiButtonLayoutBadge(pButton, tContent);
		if ( pButton->pBadgeTexture != NULL ) {
			pBadgeTexture = pButton->pBadgeTexture;
			tBadgeSrc = pButton->tBadgeSrc;
		} else {
			pBadgeTexture = __xgeXuiButtonBadgeTexture();
			tBadgeSrc = __xgeXuiBuiltinAssetSrc(XGE_XUI_ASSET_BUTTON_BADGE);
		}
		if ( pBadgeTexture != NULL ) {
			memset(&tDraw, 0, sizeof(tDraw));
			tDraw.pTexture = pBadgeTexture;
			tDraw.tSrc = tBadgeSrc;
			tDraw.tDst = pButton->tBadgeRect;
			tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
			__xgeXuiHostDrawImage(&tDraw);
		}
	}
}
