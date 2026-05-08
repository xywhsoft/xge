int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture)
{
	if ( (pImage == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pImage, 0, sizeof(*pImage));
	__xgeXuiControlWidgetInit(pWidget, 0);
	pImage->pWidget = pWidget;
	pImage->pTexture = pTexture;
	pImage->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pImage->iMode = XGE_XUI_IMAGE_STRETCH;
	pWidget->procMeasure = xgeXuiImageMeasureProc;
	pWidget->procPaint = xgeXuiImagePaintProc;
	pWidget->pUser = pImage;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiImageUnit(xge_xui_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pWidget != NULL && pImage->pWidget->pUser == pImage ) {
		pImage->pWidget->pUser = NULL;
		pImage->pWidget->procMeasure = NULL;
		pImage->pWidget->procPaint = NULL;
	}
	memset(pImage, 0, sizeof(*pImage));
}

void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->pTexture = pTexture;
	xgeXuiWidgetMarkLayout(pImage->pWidget);
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->tSrc = tSrc;
	xgeXuiWidgetMarkLayout(pImage->pWidget);
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iColor = iColor;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetMode(xge_xui_image pImage, int iMode)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iMode = iMode;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_image pImage;
	xge_vec2_t tSize;

	(void)pWidget;
	pImage = (xge_xui_image)pUser;
	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pImage == NULL ) {
		return tSize;
	}
	if ( pImage->tSrc.fW > 0.0f && pImage->tSrc.fH > 0.0f ) {
		tSize.fX = pImage->tSrc.fW;
		tSize.fY = pImage->tSrc.fH;
		return tSize;
	}
	if ( pImage->pTexture != NULL ) {
		tSize.fX = (float)pImage->pTexture->iWidth;
		tSize.fY = (float)pImage->pTexture->iHeight;
	}
	return tSize;
}

static xge_rect_t __xgeXuiImageDestRect(xge_xui_image pImage, xge_rect_t tContent)
{
	xge_rect_t tDst;
	float fSrcW;
	float fSrcH;
	float fScale;

	tDst = tContent;
	if ( (pImage == NULL) || (pImage->pTexture == NULL) ) {
		return tDst;
	}
	fSrcW = (pImage->tSrc.fW > 0.0f) ? pImage->tSrc.fW : (float)pImage->pTexture->iWidth;
	fSrcH = (pImage->tSrc.fH > 0.0f) ? pImage->tSrc.fH : (float)pImage->pTexture->iHeight;
	if ( (fSrcW <= 0.0f) || (fSrcH <= 0.0f) ) {
		return tDst;
	}
	if ( pImage->iMode == XGE_XUI_IMAGE_CENTER ) {
		tDst.fW = fSrcW;
		tDst.fH = fSrcH;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	} else if ( pImage->iMode == XGE_XUI_IMAGE_FIT ) {
		fScale = tContent.fW / fSrcW;
		if ( (fSrcH * fScale) > tContent.fH ) {
			fScale = tContent.fH / fSrcH;
		}
		tDst.fW = fSrcW * fScale;
		tDst.fH = fSrcH * fScale;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	}
	return tDst;
}

void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_image pImage;
	xge_draw_t tDraw;

	pImage = (xge_xui_image)pUser;
	if ( (pWidget == NULL) || (pImage == NULL) || (pImage->pTexture == NULL) || (XGE_COLOR_GET_A(pImage->iColor) == 0) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pImage->pTexture;
	tDraw.tSrc = pImage->tSrc;
	tDraw.tDst = __xgeXuiImageDestRect(pImage, pWidget->tContentRect);
	tDraw.iColor = pImage->iColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	__xgeXuiHostDrawImage(&tDraw);
}
