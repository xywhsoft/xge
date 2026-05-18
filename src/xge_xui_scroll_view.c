static float __xgeXuiScrollViewContentSize(float fContent, float fViewport)
{
	if ( fContent > 0.0f ) {
		return (fContent > fViewport) ? fContent : fViewport;
	}
	return (fViewport > 0.0f) ? fViewport : 0.0f;
}

static void __xgeXuiScrollViewSyncContent(xge_xui_scroll_view pScroll)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( (pScroll == NULL) || (pScroll->pContentWidget == NULL) ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pScroll->tFrame);
	fContentW = __xgeXuiScrollViewContentSize(pScroll->tModel.fContentW, tViewport.fW);
	fContentH = __xgeXuiScrollViewContentSize(pScroll->tModel.fContentH, tViewport.fH);
	tLocal.fX = -pScroll->tModel.fScrollX;
	tLocal.fY = -pScroll->tModel.fScrollY;
	tLocal.fW = fContentW;
	tLocal.fH = fContentH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	pScroll->pContentWidget->tLocalRect = tLocal;
	__xgeXuiWidgetArrangeRect(pScroll->pContentWidget, tRect);
	xgeXuiWidgetMarkPaint(pScroll->pContentWidget);
	xgeXuiWidgetMarkPaint(pScroll->tFrame.pViewportWidget);
}

static void __xgeXuiScrollViewFrameChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	__xgeXuiScrollViewSyncContent((xge_xui_scroll_view)pUser);
}

int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_rect_t tZero;

	if ( (pScroll == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pScroll, 0, sizeof(*pScroll));
	memset(&tZero, 0, sizeof(tZero));
	pScroll->pContentWidget = xgeXuiWidgetCreate();
	if ( pScroll->pContentWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pScroll->pContext = pContext;
	pScroll->pWidget = pWidget;
	xgeXuiScrollModelInit(&pScroll->tModel);
	__xgeXuiViewportWidgetInit(pWidget, 0);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiScrollViewEventProc, pScroll);
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiScrollViewLayoutProc, pScroll);
	pWidget->pUser = pScroll;
	if ( xgeXuiScrollFrameInit(&pScroll->tFrame, pContext, pWidget, &pScroll->tModel) != XGE_OK ) {
		xgeXuiWidgetFree(pScroll->pContentWidget);
		if ( pWidget->pUser == pScroll ) {
			pWidget->pUser = NULL;
			xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
			xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
		}
		memset(pScroll, 0, sizeof(*pScroll));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiScrollFrameSetChange(&pScroll->tFrame, __xgeXuiScrollViewFrameChanged, pScroll);
	xgeXuiWidgetSetRole(pScroll->pContentWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pScroll->pContentWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetName(pScroll->pContentWidget, "scrollview-content");
	xgeXuiWidgetSetBackground(pScroll->pContentWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBorder(pScroll->pContentWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pScroll->pContentWidget, 0);
	xgeXuiWidgetSetRect(pScroll->pContentWidget, tZero);
	if ( xgeXuiWidgetAddInternal(xgeXuiScrollFrameGetViewportWidget(&pScroll->tFrame), pScroll->pContentWidget) != XGE_OK ) {
		xgeXuiScrollViewUnit(pScroll);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiScrollViewLayout(pScroll);
	return XGE_OK;
}

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( pScroll->pWidget != NULL && pScroll->pWidget->pUser == pScroll ) {
		pScroll->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pScroll->pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pScroll->pWidget, NULL, NULL);
		pScroll->pWidget->procPaint = NULL;
		pScroll->pWidget->procPaintAfter = NULL;
	}
	xgeXuiScrollFrameUnit(&pScroll->tFrame);
	memset(pScroll, 0, sizeof(*pScroll));
}

xge_xui_widget xgeXuiScrollViewGetContentWidget(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->pContentWidget : NULL;
}

xge_xui_widget xgeXuiScrollViewGetViewportWidget(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? xgeXuiScrollFrameGetViewportWidget(&pScroll->tFrame) : NULL;
}

xge_xui_scroll_model xgeXuiScrollViewGetModel(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? &pScroll->tModel : NULL;
}

xge_xui_scroll_frame xgeXuiScrollViewGetFrame(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? &pScroll->tFrame : NULL;
}

void xgeXuiScrollViewLayout(xge_xui_scroll_view pScroll)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pScroll->tFrame);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetContentSize(&pScroll->tFrame, fWidth, fHeight);
	__xgeXuiScrollViewSyncContent(pScroll);
	xgeXuiWidgetMarkLayout(pScroll->pWidget);
}

void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetOffset(&pScroll->tFrame, fX, fY);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewScrollBy(xge_xui_scroll_view pScroll, float fDX, float fDY)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameScrollBy(&pScroll->tFrame, fDX, fDY);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY)
{
	xgeXuiScrollFrameGetOffset((pScroll != NULL) ? &pScroll->tFrame : NULL, pX, pY);
}

void xgeXuiScrollViewEnsureRectVisible(xge_xui_scroll_view pScroll, xge_rect_t tRect)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( xgeXuiScrollModelEnsureRectVisible(&pScroll->tModel, tRect) ) {
		__xgeXuiScrollViewSyncContent(pScroll);
		xgeXuiScrollFrameLayout(&pScroll->tFrame);
	}
}

void xgeXuiScrollViewEnsureChildVisible(xge_xui_scroll_view pScroll, xge_xui_widget pChild)
{
	xge_rect_t tRect;
	float fX;
	float fY;

	if ( (pScroll == NULL) || (pChild == NULL) ) {
		return;
	}
	tRect = pChild->tBorderRect;
	xgeXuiScrollModelScreenToContent(&pScroll->tModel, tRect.fX, tRect.fY, &fX, &fY);
	tRect.fX = fX;
	tRect.fY = fY;
	xgeXuiScrollViewEnsureRectVisible(pScroll, tRect);
}

void xgeXuiScrollViewSetScrollbarPolicy(xge_xui_scroll_view pScroll, int iPolicy)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetScrollbarPolicy(&pScroll->tFrame, iPolicy, iPolicy);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewSetScrollbarPolicyXY(xge_xui_scroll_view pScroll, int iPolicyX, int iPolicyY)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetScrollbarPolicy(&pScroll->tFrame, iPolicyX, iPolicyY);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewSetScrollbarMode(xge_xui_scroll_view pScroll, int iMode)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetScrollbarMode(&pScroll->tFrame, iMode);
}

int xgeXuiScrollViewGetScrollbarMode(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? xgeXuiScrollFrameGetScrollbarMode(&pScroll->tFrame) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiScrollViewSetWheelAxis(xge_xui_scroll_view pScroll, int iAxis)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetWheelAxis(&pScroll->tFrame, iAxis);
}

int xgeXuiScrollViewGetWheelAxis(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? xgeXuiScrollFrameGetWheelAxis(&pScroll->tFrame) : XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollViewSetWheelStep(xge_xui_scroll_view pScroll, float fStep)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetWheelStep(&pScroll->tFrame, fStep);
}

void xgeXuiScrollViewSetContentDragEnabled(xge_xui_scroll_view pScroll, int bEnabled)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetContentDragEnabled(&pScroll->tFrame, bEnabled);
}

int xgeXuiScrollViewIsContentDragEnabled(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? xgeXuiScrollFrameIsContentDragEnabled(&pScroll->tFrame) : 0;
}

void xgeXuiScrollViewSetMetrics(xge_xui_scroll_view pScroll, float fScrollbarSize, float fMinThumbSize, float fThumbRadius, float fButtonSize)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetMetrics(&pScroll->tFrame, fScrollbarSize, fMinThumbSize, fThumbRadius, fButtonSize);
	__xgeXuiScrollViewSyncContent(pScroll);
}

void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pScroll->pWidget, iBackground);
	xgeXuiScrollFrameSetColors(&pScroll->tFrame, iBar, iThumb, iThumb, iThumb, iThumb, iBar);
	xgeXuiScrollFrameSetButtonColors(&pScroll->tFrame, iBar, iThumb);
	xgeXuiScrollFrameSetCornerColors(&pScroll->tFrame, iBar, iThumb);
}

int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	return xgeXuiScrollFrameEvent((pScroll != NULL) ? &pScroll->tFrame : NULL, pEvent);
}

int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiScrollViewEvent((xge_xui_scroll_view)pUser, pEvent);
}

void xgeXuiScrollViewLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pLayoutUser;
	}
	xgeXuiScrollViewLayout((xge_xui_scroll_view)pUser);
}
