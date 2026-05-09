static void __xgeXuiTooltipDefaultDesc(xge_xui_tooltip_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iType = XGE_XUI_TOOLTIP_NONE;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	pDesc->fOffsetX = 0.0f;
	pDesc->fOffsetY = 6.0f;
	pDesc->fDelay = 0.35f;
	pDesc->bFollowCursor = 0;
}

static int __xgeXuiTooltipDescEnabled(const xge_xui_tooltip_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 0;
	}
	if ( pDesc->iType == XGE_XUI_TOOLTIP_TEXT ) {
		return (pDesc->sText != NULL) && (pDesc->sText[0] != 0);
	}
	if ( pDesc->iType == XGE_XUI_TOOLTIP_CUSTOM ) {
		return (pDesc->procMeasure != NULL) && (pDesc->procPaint != NULL);
	}
	return 0;
}

static void __xgeXuiTooltipNormalizeDesc(xge_xui_tooltip_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return;
	}
	if ( (pDesc->iType != XGE_XUI_TOOLTIP_NONE) && (pDesc->iType != XGE_XUI_TOOLTIP_TEXT) && (pDesc->iType != XGE_XUI_TOOLTIP_CUSTOM) ) {
		pDesc->iType = XGE_XUI_TOOLTIP_NONE;
	}
	if ( pDesc->iAnchor < XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM || pDesc->iAnchor > XGE_XUI_TOOLTIP_ANCHOR_CURSOR ) {
		pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	}
	if ( pDesc->fDelay < 0.0f ) {
		pDesc->fDelay = 0.0f;
	}
	if ( (pDesc->iType == XGE_XUI_TOOLTIP_TEXT) && ((pDesc->sText == NULL) || (pDesc->sText[0] == 0)) ) {
		pDesc->iType = XGE_XUI_TOOLTIP_NONE;
	}
	if ( (pDesc->iType == XGE_XUI_TOOLTIP_CUSTOM) && ((pDesc->procMeasure == NULL) || (pDesc->procPaint == NULL)) ) {
		pDesc->iType = XGE_XUI_TOOLTIP_NONE;
	}
}

static int __xgeXuiTooltipTextSame(const char* sLeft, const char* sRight)
{
	if ( sLeft == sRight ) {
		return 1;
	}
	if ( sLeft == NULL ) {
		sLeft = "";
	}
	if ( sRight == NULL ) {
		sRight = "";
	}
	return strcmp(sLeft, sRight) == 0;
}

static int __xgeXuiTooltipDescSame(const xge_xui_tooltip_desc_t* pLeft, const xge_xui_tooltip_desc_t* pRight)
{
	if ( pLeft == pRight ) {
		return 1;
	}
	if ( (pLeft == NULL) || (pRight == NULL) ) {
		return 0;
	}
	return (pLeft->iType == pRight->iType) &&
		__xgeXuiTooltipTextSame(pLeft->sText, pRight->sText) &&
		(pLeft->iAnchor == pRight->iAnchor) &&
		(pLeft->fOffsetX == pRight->fOffsetX) &&
		(pLeft->fOffsetY == pRight->fOffsetY) &&
		(pLeft->fDelay == pRight->fDelay) &&
		(pLeft->bFollowCursor == pRight->bFollowCursor) &&
		(pLeft->procMeasure == pRight->procMeasure) &&
		(pLeft->procPaint == pRight->procPaint) &&
		(pLeft->pUser == pRight->pUser);
}

static int __xgeXuiTooltipResolveWidget(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 0;
	}
	__xgeXuiTooltipDefaultDesc(pDesc);
	if ( pWidget == NULL ) {
		return 0;
	}
	if ( pWidget->procTooltipResolve != NULL ) {
		if ( pWidget->procTooltipResolve(pContext, pWidget, pDesc, pWidget->pTooltipUser) == 0 ) {
			__xgeXuiTooltipDefaultDesc(pDesc);
			return 0;
		}
		__xgeXuiTooltipNormalizeDesc(pDesc);
		if ( __xgeXuiTooltipDescEnabled(pDesc) ) {
			return 1;
		}
		__xgeXuiTooltipDefaultDesc(pDesc);
		return 0;
	}
	*pDesc = pWidget->tTooltip;
	__xgeXuiTooltipNormalizeDesc(pDesc);
	if ( __xgeXuiTooltipDescEnabled(pDesc) ) {
		return 1;
	}
	__xgeXuiTooltipDefaultDesc(pDesc);
	return 0;
}

static xge_xui_widget __xgeXuiTooltipFindOwner(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc_t* pDesc)
{
	xge_xui_tooltip_desc_t tDesc;

	for ( ; pWidget != NULL; pWidget = pWidget->pParent ) {
		if ( __xgeXuiTooltipResolveWidget(pContext, pWidget, &tDesc) ) {
			if ( pDesc != NULL ) {
				*pDesc = tDesc;
			}
			return pWidget;
		}
	}
	if ( pDesc != NULL ) {
		__xgeXuiTooltipDefaultDesc(pDesc);
	}
	return NULL;
}

static void __xgeXuiTooltipClose(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	if ( pContext->bTooltipOpen != 0 ) {
		pContext->bTooltipOpen = 0;
		xgeXuiWidgetSetVisible(pContext->pTooltipPopupWidget, 0);
		pContext->pTooltipPopupWidget->pOverlayOwner = NULL;
		xgeXuiWidgetMarkPaint(pContext->pTooltipPopupWidget);
	}
}

static xge_vec2_t __xgeXuiTooltipMeasure(xge_xui_context pContext, xge_xui_widget pOwner, const xge_xui_tooltip_desc_t* pDesc)
{
	xge_vec2_t tSize;
	xge_font pFont;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pContext == NULL) || (pDesc == NULL) ) {
		return tSize;
	}
	if ( pDesc->iType == XGE_XUI_TOOLTIP_CUSTOM ) {
		tSize = pDesc->procMeasure(pContext, pOwner, pDesc->pUser);
	} else if ( pDesc->iType == XGE_XUI_TOOLTIP_TEXT ) {
		pFont = (pContext->tTheme.pFont != NULL) ? pContext->tTheme.pFont : (pContext->bDefaultFontReady ? &pContext->tDefaultFont : NULL);
		tSize = __xgeXuiHostMeasureText(pFont, pDesc->sText);
		tSize.fX += 12.0f;
		tSize.fY += 8.0f;
	}
	if ( tSize.fX < 12.0f ) {
		tSize.fX = 12.0f;
	}
	if ( tSize.fY < 18.0f ) {
		tSize.fY = 18.0f;
	}
	return tSize;
}

static float __xgeXuiTooltipClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) {
		return fMin;
	}
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static xge_rect_t __xgeXuiTooltipResolveRect(xge_xui_context pContext, xge_xui_widget pOwner, const xge_xui_tooltip_desc_t* pDesc, xge_vec2_t tSize)
{
	xge_rect_t tRect;
	xge_rect_t tRoot;
	xge_rect_t tOwner;
	float fMargin;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pContext == NULL) || (pOwner == NULL) || (pDesc == NULL) ) {
		return tRect;
	}
	tRoot = pContext->pRoot != NULL ? pContext->pRoot->tRect : (xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() };
	tOwner = pOwner->tRect;
	tRect.fW = tSize.fX;
	tRect.fH = tSize.fY;
	fMargin = 2.0f;
	switch ( pDesc->iAnchor ) {
		case XGE_XUI_TOOLTIP_ANCHOR_CURSOR:
			tRect.fX = pContext->fTooltipMouseX + pDesc->fOffsetX;
			tRect.fY = pContext->fTooltipMouseY + pDesc->fOffsetY;
			break;
		case XGE_XUI_TOOLTIP_ANCHOR_WIDGET_TOP:
			tRect.fX = tOwner.fX + pDesc->fOffsetX;
			tRect.fY = tOwner.fY - tRect.fH - pDesc->fOffsetY;
			break;
		case XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT:
			tRect.fX = tOwner.fX + tOwner.fW + pDesc->fOffsetX;
			tRect.fY = tOwner.fY + pDesc->fOffsetY;
			break;
		case XGE_XUI_TOOLTIP_ANCHOR_WIDGET_LEFT:
			tRect.fX = tOwner.fX - tRect.fW - pDesc->fOffsetX;
			tRect.fY = tOwner.fY + pDesc->fOffsetY;
			break;
		case XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM:
		default:
			tRect.fX = tOwner.fX + pDesc->fOffsetX;
			tRect.fY = tOwner.fY + tOwner.fH + pDesc->fOffsetY;
			break;
	}
	if ( (pDesc->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM) && (tRect.fY + tRect.fH > tRoot.fY + tRoot.fH - fMargin) ) {
		tRect.fY = tOwner.fY - tRect.fH - pDesc->fOffsetY;
	} else if ( (pDesc->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_TOP) && (tRect.fY < tRoot.fY + fMargin) ) {
		tRect.fY = tOwner.fY + tOwner.fH + pDesc->fOffsetY;
	}
	if ( (pDesc->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT) && (tRect.fX + tRect.fW > tRoot.fX + tRoot.fW - fMargin) ) {
		tRect.fX = tOwner.fX - tRect.fW - pDesc->fOffsetX;
	} else if ( (pDesc->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_LEFT) && (tRect.fX < tRoot.fX + fMargin) ) {
		tRect.fX = tOwner.fX + tOwner.fW + pDesc->fOffsetX;
	}
	tRect.fX = __xgeXuiTooltipClamp(tRect.fX, tRoot.fX + fMargin, tRoot.fX + tRoot.fW - tRect.fW - fMargin);
	tRect.fY = __xgeXuiTooltipClamp(tRect.fY, tRoot.fY + fMargin, tRoot.fY + tRoot.fH - tRect.fH - fMargin);
	return tRect;
}

static void __xgeXuiTooltipOpen(xge_xui_context pContext)
{
	xge_vec2_t tSize;
	xge_rect_t tRect;
	xge_xui_tooltip_desc_t* pDesc;

	if ( (pContext == NULL) || (pContext->pTooltipOwner == NULL) || (pContext->pTooltipPopupWidget == NULL) ) {
		return;
	}
	pDesc = &pContext->tActiveTooltip;
	if ( !__xgeXuiTooltipDescEnabled(pDesc) || !xgeXuiWidgetIsVisible(pContext->pTooltipOwner) || !xgeXuiWidgetIsEnabled(pContext->pTooltipOwner) ) {
		__xgeXuiTooltipClose(pContext);
		return;
	}
	tSize = __xgeXuiTooltipMeasure(pContext, pContext->pTooltipOwner, pDesc);
	tRect = __xgeXuiTooltipResolveRect(pContext, pContext->pTooltipOwner, pDesc, tSize);
	pContext->tTooltipRect = tRect;
	xgeXuiWidgetSetRect(pContext->pTooltipPopupWidget, tRect);
	xgeXuiWidgetSetLayer(pContext->pTooltipPopupWidget, XGE_XUI_LAYER_TOOLTIP);
	pContext->pTooltipPopupWidget->pOverlayOwner = pContext->pTooltipOwner;
	xgeXuiOverlayBringToFront(pContext, pContext->pTooltipPopupWidget);
	xgeXuiWidgetSetVisible(pContext->pTooltipPopupWidget, 1);
	xgeXuiWidgetSetEnabled(pContext->pTooltipPopupWidget, 0);
	xgeXuiWidgetMarkPaint(pContext->pTooltipPopupWidget);
	pContext->bTooltipOpen = 1;
}

void xgeXuiWidgetSetTooltipText(xge_xui_widget pWidget, const char* sText)
{
	xge_xui_tooltip_desc_t tDesc;

	if ( pWidget == NULL ) {
		return;
	}
	__xgeXuiTooltipDefaultDesc(&tDesc);
	if ( (sText != NULL) && (sText[0] != 0) ) {
		tDesc.iType = XGE_XUI_TOOLTIP_TEXT;
		tDesc.sText = sText;
	}
	xgeXuiWidgetSetTooltip(pWidget, &tDesc);
}

void xgeXuiWidgetSetTooltip(xge_xui_widget pWidget, const xge_xui_tooltip_desc_t* pDesc)
{
	xge_xui_tooltip_desc_t tDesc;

	if ( pWidget == NULL ) {
		return;
	}
	if ( pDesc == NULL ) {
		xgeXuiWidgetClearTooltip(pWidget);
		return;
	}
	tDesc = *pDesc;
	if ( tDesc.iType == XGE_XUI_TOOLTIP_NONE ) {
		xgeXuiWidgetClearTooltip(pWidget);
		return;
	}
	__xgeXuiTooltipNormalizeDesc(&tDesc);
	if ( !__xgeXuiTooltipDescEnabled(&tDesc) ) {
		xgeXuiWidgetClearTooltip(pWidget);
		return;
	}
	pWidget->tTooltip = tDesc;
	pWidget->procTooltipResolve = NULL;
	pWidget->pTooltipUser = NULL;
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_TOOLTIP, 1);
}

void xgeXuiWidgetSetTooltipResolver(xge_xui_widget pWidget, xge_xui_tooltip_resolve_proc procResolve, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	__xgeXuiTooltipDefaultDesc(&pWidget->tTooltip);
	pWidget->procTooltipResolve = procResolve;
	pWidget->pTooltipUser = pUser;
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_TOOLTIP, procResolve != NULL);
}

void xgeXuiWidgetClearTooltip(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	__xgeXuiTooltipDefaultDesc(&pWidget->tTooltip);
	pWidget->procTooltipResolve = NULL;
	pWidget->pTooltipUser = NULL;
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_TOOLTIP, 0);
}

const xge_xui_tooltip_desc_t* xgeXuiWidgetGetTooltip(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return &pWidget->tTooltip;
}

int xgeXuiTooltipIsOpen(xge_xui_context pContext)
{
	return (pContext != NULL) && (pContext->bTooltipOpen != 0);
}

xge_xui_widget xgeXuiTooltipGetOwner(xge_xui_context pContext)
{
	return (pContext != NULL) ? pContext->pTooltipOwner : NULL;
}

xge_rect_t xgeXuiTooltipGetRect(xge_xui_context pContext)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pContext == NULL ) {
		return tRect;
	}
	return pContext->tTooltipRect;
}

void xgeXuiTooltipHandleEvent(xge_xui_context pContext, xge_xui_widget pHit, const xge_event_t* pEvent)
{
	xge_xui_widget pOwner;
	xge_xui_tooltip_desc_t tDesc;
	int bWasOpen;

	if ( (pContext == NULL) || (pEvent == NULL) ) {
		return;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pContext->fTooltipMouseX = pEvent->fX;
			pContext->fTooltipMouseY = pEvent->fY;
			if ( xgeXuiHasCapture(pContext) ) {
				pOwner = NULL;
				__xgeXuiTooltipDefaultDesc(&tDesc);
			} else {
				pOwner = __xgeXuiTooltipFindOwner(pContext, pHit, &tDesc);
			}
			if ( pOwner != pContext->pTooltipOwner ) {
				__xgeXuiTooltipClose(pContext);
				pContext->pTooltipOwner = pOwner;
				pContext->fTooltipHoverTime = 0.0f;
				if ( pOwner != NULL ) {
					pContext->tActiveTooltip = tDesc;
				} else {
					__xgeXuiTooltipDefaultDesc(&pContext->tActiveTooltip);
				}
			} else if ( pOwner != NULL && !__xgeXuiTooltipDescSame(&pContext->tActiveTooltip, &tDesc) ) {
				bWasOpen = pContext->bTooltipOpen;
				pContext->tActiveTooltip = tDesc;
				if ( bWasOpen != 0 ) {
					__xgeXuiTooltipOpen(pContext);
				} else {
					pContext->fTooltipHoverTime = 0.0f;
				}
			} else if ( (pContext->bTooltipOpen != 0) && (pContext->tActiveTooltip.bFollowCursor != 0) ) {
				__xgeXuiTooltipOpen(pContext);
			}
			break;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_MOUSE_WHEEL:
		case XGE_EVENT_TOUCH_BEGIN:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			pContext->fTooltipMouseX = pEvent->fX;
			pContext->fTooltipMouseY = pEvent->fY;
			pContext->pTooltipOwner = NULL;
			pContext->fTooltipHoverTime = 0.0f;
			__xgeXuiTooltipClose(pContext);
			break;
		default:
			break;
	}
}

void xgeXuiTooltipUpdate(xge_xui_context pContext, float fDelta)
{
	xge_xui_tooltip_desc_t tDesc;

	if ( (pContext == NULL) || (pContext->pTooltipOwner == NULL) ) {
		return;
	}
	if ( xgeXuiHasCapture(pContext) ) {
		pContext->pTooltipOwner = NULL;
		__xgeXuiTooltipClose(pContext);
		return;
	}
	if ( !__xgeXuiTooltipResolveWidget(pContext, pContext->pTooltipOwner, &tDesc) || !xgeXuiWidgetIsVisible(pContext->pTooltipOwner) || !xgeXuiWidgetIsEnabled(pContext->pTooltipOwner) ) {
		pContext->pTooltipOwner = NULL;
		pContext->fTooltipHoverTime = 0.0f;
		__xgeXuiTooltipDefaultDesc(&pContext->tActiveTooltip);
		__xgeXuiTooltipClose(pContext);
		return;
	}
	if ( !__xgeXuiTooltipDescSame(&pContext->tActiveTooltip, &tDesc) ) {
		pContext->tActiveTooltip = tDesc;
		if ( pContext->bTooltipOpen != 0 ) {
			__xgeXuiTooltipOpen(pContext);
		} else {
			pContext->fTooltipHoverTime = 0.0f;
		}
	}
	pContext->fTooltipHoverTime += fDelta;
	if ( pContext->fTooltipHoverTime >= pContext->tActiveTooltip.fDelay ) {
		__xgeXuiTooltipOpen(pContext);
	}
}

void xgeXuiTooltipPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_context pContext;
	xge_rect_t tRect;
	xge_rect_t tText;
	xge_font pFont;

	(void)pWidget;
	pContext = (xge_xui_context)pUser;
	if ( (pContext == NULL) || (pContext->bTooltipOpen == 0) ) {
		return;
	}
	tRect = pContext->tTooltipRect;
	__xgeXuiHostDrawRoundedRect(tRect, XGE_COLOR_RGBA(247, 252, 255, 255), 2.0f);
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(118, 180, 224, 255));
	if ( pContext->tActiveTooltip.iType == XGE_XUI_TOOLTIP_CUSTOM ) {
		pContext->tActiveTooltip.procPaint(pContext, pContext->pTooltipOwner, tRect, pContext->tActiveTooltip.pUser);
	} else if ( pContext->tActiveTooltip.iType == XGE_XUI_TOOLTIP_TEXT ) {
		pFont = (pContext->tTheme.pFont != NULL) ? pContext->tTheme.pFont : (pContext->bDefaultFontReady ? &pContext->tDefaultFont : NULL);
		tText = tRect;
		tText.fX += 6.0f;
		tText.fY += 3.0f;
		tText.fW -= 12.0f;
		tText.fH -= 6.0f;
		__xgeXuiHostDrawTextRect(pFont, pContext->tActiveTooltip.sText, tText, XGE_COLOR_RGBA(31, 75, 112, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
}
