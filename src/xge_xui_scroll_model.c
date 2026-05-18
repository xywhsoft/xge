static float __xgeXuiScrollModelNormalizeSize(float fValue)
{
	return (fValue > 0.0f) ? fValue : 0.0f;
}

static int __xgeXuiScrollModelRectSame(xge_rect_t tA, xge_rect_t tB)
{
	return (tA.fX == tB.fX) && (tA.fY == tB.fY) && (tA.fW == tB.fW) && (tA.fH == tB.fH);
}

static void __xgeXuiScrollModelMaxOffset(xge_xui_scroll_model pModel, float* pX, float* pY)
{
	float fMaxX;
	float fMaxY;

	fMaxX = 0.0f;
	fMaxY = 0.0f;
	if ( pModel != NULL ) {
		fMaxX = pModel->fContentW - pModel->tViewportRect.fW;
		fMaxY = pModel->fContentH - pModel->tViewportRect.fH;
		if ( fMaxX < 0.0f ) {
			fMaxX = 0.0f;
		}
		if ( fMaxY < 0.0f ) {
			fMaxY = 0.0f;
		}
	}
	if ( pX != NULL ) {
		*pX = fMaxX;
	}
	if ( pY != NULL ) {
		*pY = fMaxY;
	}
}

static int __xgeXuiScrollModelClampOffset(xge_xui_scroll_model pModel)
{
	float fMaxX;
	float fMaxY;
	float fOldX;
	float fOldY;

	if ( pModel == NULL ) {
		return 0;
	}
	fOldX = pModel->fScrollX;
	fOldY = pModel->fScrollY;
	__xgeXuiScrollModelMaxOffset(pModel, &fMaxX, &fMaxY);
	pModel->fScrollX = __xgeXuiClampFloat(pModel->fScrollX, 0.0f, fMaxX);
	pModel->fScrollY = __xgeXuiClampFloat(pModel->fScrollY, 0.0f, fMaxY);
	return (pModel->fScrollX != fOldX) || (pModel->fScrollY != fOldY);
}

void xgeXuiScrollModelInit(xge_xui_scroll_model pModel)
{
	if ( pModel == NULL ) {
		return;
	}
	memset(pModel, 0, sizeof(*pModel));
}

int xgeXuiScrollModelSetViewport(xge_xui_scroll_model pModel, xge_rect_t tViewport)
{
	xge_rect_t tOld;
	int bChanged;

	if ( pModel == NULL ) {
		return 0;
	}
	tViewport.fW = __xgeXuiScrollModelNormalizeSize(tViewport.fW);
	tViewport.fH = __xgeXuiScrollModelNormalizeSize(tViewport.fH);
	tOld = pModel->tViewportRect;
	pModel->tViewportRect = tViewport;
	bChanged = !__xgeXuiScrollModelRectSame(tOld, tViewport);
	if ( __xgeXuiScrollModelClampOffset(pModel) ) {
		bChanged = 1;
	}
	return bChanged;
}

xge_rect_t xgeXuiScrollModelGetViewport(xge_xui_scroll_model pModel)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pModel != NULL ) {
		tRect = pModel->tViewportRect;
	}
	return tRect;
}

int xgeXuiScrollModelSetContentSize(xge_xui_scroll_model pModel, float fWidth, float fHeight)
{
	float fOldW;
	float fOldH;
	int bChanged;

	if ( pModel == NULL ) {
		return 0;
	}
	fWidth = __xgeXuiScrollModelNormalizeSize(fWidth);
	fHeight = __xgeXuiScrollModelNormalizeSize(fHeight);
	fOldW = pModel->fContentW;
	fOldH = pModel->fContentH;
	pModel->fContentW = fWidth;
	pModel->fContentH = fHeight;
	bChanged = (fOldW != fWidth) || (fOldH != fHeight);
	if ( __xgeXuiScrollModelClampOffset(pModel) ) {
		bChanged = 1;
	}
	return bChanged;
}

int xgeXuiScrollModelSetOffset(xge_xui_scroll_model pModel, float fX, float fY)
{
	float fMaxX;
	float fMaxY;
	float fOldX;
	float fOldY;

	if ( pModel == NULL ) {
		return 0;
	}
	__xgeXuiScrollModelMaxOffset(pModel, &fMaxX, &fMaxY);
	fOldX = pModel->fScrollX;
	fOldY = pModel->fScrollY;
	pModel->fScrollX = __xgeXuiClampFloat(fX, 0.0f, fMaxX);
	pModel->fScrollY = __xgeXuiClampFloat(fY, 0.0f, fMaxY);
	return (pModel->fScrollX != fOldX) || (pModel->fScrollY != fOldY);
}

int xgeXuiScrollModelScrollBy(xge_xui_scroll_model pModel, float fDX, float fDY)
{
	if ( pModel == NULL ) {
		return 0;
	}
	return xgeXuiScrollModelSetOffset(pModel, pModel->fScrollX + fDX, pModel->fScrollY + fDY);
}

void xgeXuiScrollModelGetOffset(xge_xui_scroll_model pModel, float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = (pModel != NULL) ? pModel->fScrollX : 0.0f;
	}
	if ( pY != NULL ) {
		*pY = (pModel != NULL) ? pModel->fScrollY : 0.0f;
	}
}

void xgeXuiScrollModelGetMaxOffset(xge_xui_scroll_model pModel, float* pX, float* pY)
{
	__xgeXuiScrollModelMaxOffset(pModel, pX, pY);
}

int xgeXuiScrollModelEnsureRectVisible(xge_xui_scroll_model pModel, xge_rect_t tContentRect)
{
	float fX;
	float fY;
	float fRight;
	float fBottom;

	if ( pModel == NULL ) {
		return 0;
	}
	fX = pModel->fScrollX;
	fY = pModel->fScrollY;
	fRight = tContentRect.fX + tContentRect.fW;
	fBottom = tContentRect.fY + tContentRect.fH;
	if ( tContentRect.fX < fX ) {
		fX = tContentRect.fX;
	} else if ( fRight > fX + pModel->tViewportRect.fW ) {
		fX = fRight - pModel->tViewportRect.fW;
	}
	if ( tContentRect.fY < fY ) {
		fY = tContentRect.fY;
	} else if ( fBottom > fY + pModel->tViewportRect.fH ) {
		fY = fBottom - pModel->tViewportRect.fH;
	}
	return xgeXuiScrollModelSetOffset(pModel, fX, fY);
}

void xgeXuiScrollModelScreenToViewport(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pViewportX, float* pViewportY)
{
	if ( pViewportX != NULL ) {
		*pViewportX = (pModel != NULL) ? (fScreenX - pModel->tViewportRect.fX) : fScreenX;
	}
	if ( pViewportY != NULL ) {
		*pViewportY = (pModel != NULL) ? (fScreenY - pModel->tViewportRect.fY) : fScreenY;
	}
}

void xgeXuiScrollModelViewportToContent(xge_xui_scroll_model pModel, float fViewportX, float fViewportY, float* pContentX, float* pContentY)
{
	if ( pContentX != NULL ) {
		*pContentX = (pModel != NULL) ? (fViewportX + pModel->fScrollX) : fViewportX;
	}
	if ( pContentY != NULL ) {
		*pContentY = (pModel != NULL) ? (fViewportY + pModel->fScrollY) : fViewportY;
	}
}

void xgeXuiScrollModelScreenToContent(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pContentX, float* pContentY)
{
	float fViewportX;
	float fViewportY;

	xgeXuiScrollModelScreenToViewport(pModel, fScreenX, fScreenY, &fViewportX, &fViewportY);
	xgeXuiScrollModelViewportToContent(pModel, fViewportX, fViewportY, pContentX, pContentY);
}

void xgeXuiScrollModelContentToViewport(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pViewportX, float* pViewportY)
{
	if ( pViewportX != NULL ) {
		*pViewportX = (pModel != NULL) ? (fContentX - pModel->fScrollX) : fContentX;
	}
	if ( pViewportY != NULL ) {
		*pViewportY = (pModel != NULL) ? (fContentY - pModel->fScrollY) : fContentY;
	}
}

void xgeXuiScrollModelContentToScreen(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pScreenX, float* pScreenY)
{
	float fViewportX;
	float fViewportY;

	xgeXuiScrollModelContentToViewport(pModel, fContentX, fContentY, &fViewportX, &fViewportY);
	if ( pScreenX != NULL ) {
		*pScreenX = (pModel != NULL) ? (pModel->tViewportRect.fX + fViewportX) : fViewportX;
	}
	if ( pScreenY != NULL ) {
		*pScreenY = (pModel != NULL) ? (pModel->tViewportRect.fY + fViewportY) : fViewportY;
	}
}
