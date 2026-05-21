static int __xgeXuiProgressTemplateIsDynamic(const char* sTextTemplate)
{
	int i;

	if ( sTextTemplate == NULL ) {
		return 0;
	}
	for ( i = 0; sTextTemplate[i] != 0; i++ ) {
		if ( sTextTemplate[i] != '%' ) {
			continue;
		}
		if ( sTextTemplate[i + 1] == '%' ) {
			i++;
			continue;
		}
		return 1;
	}
	return 0;
}

static char* __xgeXuiProgressCopyText(const char* sText)
{
	char* sCopy;
	size_t iNeed;

	if ( sText == NULL ) {
		return NULL;
	}
	iNeed = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iNeed);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iNeed);
	return sCopy;
}

static int __xgeXuiProgressSetDisplayText(xge_xui_progress pProgress)
{
	char* sNew;
	double fPercent;

	if ( pProgress == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pProgress->sDisplayText != NULL ) {
		xrtFree(pProgress->sDisplayText);
		pProgress->sDisplayText = NULL;
	}
	if ( pProgress->sTextTemplate == NULL ) {
		return XGE_OK;
	}
	if ( pProgress->bTemplateString ) {
		fPercent = (double)__xgeXuiProgressRate(pProgress) * 100.0;
		sNew = (char*)xrtFormat(pProgress->sTextTemplate, fPercent);
	} else {
		sNew = __xgeXuiProgressCopyText(pProgress->sTextTemplate);
	}
	if ( sNew == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pProgress->sDisplayText = sNew;
	return XGE_OK;
}

static void __xgeXuiProgressInvalidateText(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pProgress->bTemplateString ) {
		(void)__xgeXuiProgressSetDisplayText(pProgress);
	}
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget)
{
	if ( (pProgress == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pProgress, 0, sizeof(*pProgress));
	__xgeXuiControlWidgetInit(pWidget, 0);
	pProgress->pWidget = pWidget;
	pProgress->fMin = 0.0f;
	pProgress->fMax = 1.0f;
	pProgress->fValue = 0.0f;
	pProgress->iColorTrack = XGE_COLOR_RGBA(216, 236, 248, 255);
	pProgress->iColorFill = XGE_COLOR_RGBA(46, 124, 214, 255);
	pProgress->iTextColor = XGE_COLOR_RGBA(36, 52, 70, 255);
	pProgress->iFillTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pProgress->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pProgress->iFillDirection = XGE_XUI_PROGRESS_LEFT_TO_RIGHT;
	pProgress->iFillPatchMode = XGE_XUI_PROGRESS_FILL_STRETCH;
	pWidget->procMeasure = xgeXuiProgressMeasureProc;
	pWidget->procPaint = xgeXuiProgressPaintProc;
	pWidget->pUser = pProgress;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiProgressUnit(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pProgress->sTextTemplate != NULL ) {
		xrtFree(pProgress->sTextTemplate);
	}
	if ( pProgress->sDisplayText != NULL ) {
		xrtFree(pProgress->sDisplayText);
	}
	if ( pProgress->pWidget != NULL && pProgress->pWidget->pUser == pProgress ) {
		pProgress->pWidget->pUser = NULL;
		pProgress->pWidget->procMeasure = NULL;
		pProgress->pWidget->procPaint = NULL;
	}
	memset(pProgress, 0, sizeof(*pProgress));
}

void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		float fSwap;
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pProgress->fMin = fMin;
	pProgress->fMax = fMax;
	__xgeXuiProgressSetValueInternal(pProgress, pProgress->fValue);
	__xgeXuiProgressInvalidateText(pProgress);
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue)
{
	float fOldValue;

	fOldValue = (pProgress != NULL) ? pProgress->fValue : 0.0f;
	__xgeXuiProgressSetValueInternal(pProgress, fValue);
	if ( (pProgress != NULL) && (pProgress->fValue != fOldValue) ) {
		__xgeXuiProgressInvalidateText(pProgress);
	}
}

float xgeXuiProgressGetValue(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return 0.0f;
	}
	return pProgress->fValue;
}

void xgeXuiProgressSetText(xge_xui_progress pProgress, xui_font pFont, const char* sText)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->pFont = pFont;
	xgeXuiProgressSetTextTemplate(pProgress, sText);
}

void xgeXuiProgressSetTextTemplate(xge_xui_progress pProgress, const char* sTextTemplate)
{
	char* sNewTemplate;

	if ( pProgress == NULL ) {
		return;
	}
	sNewTemplate = __xgeXuiProgressCopyText(sTextTemplate);
	if ( (sTextTemplate != NULL) && (sNewTemplate == NULL) ) {
		return;
	}
	if ( pProgress->sTextTemplate != NULL ) {
		xrtFree(pProgress->sTextTemplate);
	}
	pProgress->sTextTemplate = sNewTemplate;
	pProgress->bTemplateString = __xgeXuiProgressTemplateIsDynamic(pProgress->sTextTemplate);
	(void)__xgeXuiProgressSetDisplayText(pProgress);
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetFillTextColor(xge_xui_progress pProgress, uint32_t iColor)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iFillTextColor = iColor;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iColorTrack = iTrack;
	pProgress->iColorFill = iFill;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetFillDirection(xge_xui_progress pProgress, int iFillDirection)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( (iFillDirection < XGE_XUI_PROGRESS_LEFT_TO_RIGHT) || (iFillDirection > XGE_XUI_PROGRESS_TOP_TO_BOTTOM) ) {
		iFillDirection = XGE_XUI_PROGRESS_LEFT_TO_RIGHT;
	}
	if ( pProgress->iFillDirection != iFillDirection ) {
		pProgress->iFillDirection = iFillDirection;
		xgeXuiWidgetMarkLayout(pProgress->pWidget);
		xgeXuiWidgetMarkPaint(pProgress->pWidget);
	}
}

void xgeXuiProgressSetTrackPatch(xge_xui_progress pProgress, const xge_nine_patch_t* pPatch)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pPatch != NULL ) {
		pProgress->tTrackPatch = *pPatch;
		pProgress->bHasTrackPatch = 1;
	} else {
		memset(&pProgress->tTrackPatch, 0, sizeof(pProgress->tTrackPatch));
		pProgress->bHasTrackPatch = 0;
	}
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetFillPatch(xge_xui_progress pProgress, const xge_nine_patch_t* pPatch)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pPatch != NULL ) {
		pProgress->tFillPatch = *pPatch;
		pProgress->bHasFillPatch = 1;
	} else {
		memset(&pProgress->tFillPatch, 0, sizeof(pProgress->tFillPatch));
		pProgress->bHasFillPatch = 0;
	}
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetFillPatchMode(xge_xui_progress pProgress, int iMode)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( iMode != XGE_XUI_PROGRESS_FILL_REVEAL ) {
		iMode = XGE_XUI_PROGRESS_FILL_STRETCH;
	}
	if ( pProgress->iFillPatchMode != iMode ) {
		pProgress->iFillPatchMode = iMode;
		xgeXuiWidgetMarkPaint(pProgress->pWidget);
	}
}

xge_vec2_t xgeXuiProgressMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_progress pProgress;
	xge_vec2_t tSize;

	(void)pWidget;
	pProgress = (xge_xui_progress)pUser;
	if ( (pProgress != NULL) && ((pProgress->iFillDirection == XGE_XUI_PROGRESS_BOTTOM_TO_TOP) || (pProgress->iFillDirection == XGE_XUI_PROGRESS_TOP_TO_BOTTOM)) ) {
		tSize.fX = 12.0f;
		tSize.fY = 120.0f;
	} else {
		tSize.fX = 120.0f;
		tSize.fY = 12.0f;
	}
	return tSize;
}

static void __xgeXuiProgressNinePatchDrawProc(const xge_draw_t* pDraw, void* pUser)
{
	(void)pUser;
	__xgeXuiHostDrawImage(pDraw);
}

static void __xgeXuiProgressDrawPatch(const xge_nine_patch_t* pPatch, xge_rect_t tDst)
{
	__xgeNinePatchDrawInternal(pPatch, tDst, XGE_DRAW_SCREEN_SPACE, XGE_COLOR_RGBA(255, 255, 255, 255), __xgeXuiProgressNinePatchDrawProc, NULL);
}

static void __xgeXuiProgressDrawRevealPatch(const xge_nine_patch_t* pPatch, xge_rect_t tContent, xge_rect_t tFill, int iDirection, float fRate)
{
	xge_nine_patch_t tRevealPatch;
	xge_rect_t tSrc;
	xui_texture_desc_t tDesc;

	if ( (pPatch == NULL) || (pPatch->pTexture == NULL) || (tFill.fW <= 0.0f) || (tFill.fH <= 0.0f) || (fRate <= 0.0f) ) {
		return;
	}
	tRevealPatch = *pPatch;
	tSrc = tRevealPatch.tSrc;
	memset(&tDesc, 0, sizeof(tDesc));
	(void)__xgeXuiHostTextureGetDesc(NULL, (xui_texture)tRevealPatch.pTexture, &tDesc);
	if ( tSrc.fW == 0.0f ) {
		tSrc.fW = (float)tDesc.iWidth;
	}
	if ( tSrc.fH == 0.0f ) {
		tSrc.fH = (float)tDesc.iHeight;
	}
	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ) {
		return;
	}
	if ( iDirection == XGE_XUI_PROGRESS_RIGHT_TO_LEFT ) {
		tSrc.fX += tSrc.fW * (1.0f - fRate);
		tSrc.fW *= fRate;
	} else if ( iDirection == XGE_XUI_PROGRESS_BOTTOM_TO_TOP ) {
		tSrc.fY += tSrc.fH * (1.0f - fRate);
		tSrc.fH *= fRate;
	} else if ( iDirection == XGE_XUI_PROGRESS_TOP_TO_BOTTOM ) {
		tSrc.fH *= fRate;
	} else {
		tSrc.fW *= fRate;
	}
	tRevealPatch.tSrc = tSrc;
	__xgeNinePatchDrawInternal(&tRevealPatch, tFill, XGE_DRAW_SCREEN_SPACE, XGE_COLOR_RGBA(255, 255, 255, 255), __xgeXuiProgressNinePatchDrawProc, NULL);
	(void)tContent;
}

static void __xgeXuiProgressDrawText(xge_xui_widget pWidget, xge_xui_progress pProgress, xge_rect_t tFill)
{
	xge_rect_t tOldClip;
	int bOldClip;

	if ( (pWidget == NULL) || (pProgress == NULL) || (pProgress->pFont == NULL) || (pProgress->sDisplayText == NULL) || (pProgress->sDisplayText[0] == 0) ) {
		return;
	}
	__xgeXuiHostDrawTextRect(pProgress->pFont, pProgress->sDisplayText, pWidget->tContentRect, pProgress->iTextColor, pProgress->iTextFlags);
	if ( (tFill.fW <= 0.0f) || (tFill.fH <= 0.0f) || ((pProgress->bHasFillPatch == 0) && (XGE_COLOR_GET_A(pProgress->iColorFill) == 0)) || (XGE_COLOR_GET_A(pProgress->iFillTextColor) == 0) ) {
		return;
	}
	memset(&tOldClip, 0, sizeof(tOldClip));
	bOldClip = 0;
	(void)__xgeXuiHostClipGet(&tOldClip, &bOldClip);
	__xgeXuiHostFlush(__xgeXuiWidgetContext(pWidget));
	__xgeXuiHostClipSet(tFill);
	__xgeXuiHostDrawTextRect(pProgress->pFont, pProgress->sDisplayText, pWidget->tContentRect, pProgress->iFillTextColor, pProgress->iTextFlags);
	__xgeXuiHostFlush(__xgeXuiWidgetContext(pWidget));
	if ( bOldClip ) {
		__xgeXuiHostClipSet(tOldClip);
	} else {
		__xgeXuiHostClipClear();
	}
}

void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_progress pProgress;
	xge_rect_t tFill;
	float fRate;

	pProgress = (xge_xui_progress)pUser;
	if ( (pWidget == NULL) || (pProgress == NULL) ) {
		return;
	}
	if ( pProgress->bHasTrackPatch ) {
		__xgeXuiProgressDrawPatch(&pProgress->tTrackPatch, pWidget->tContentRect);
	} else if ( XGE_COLOR_GET_A(pProgress->iColorTrack) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tContentRect, pProgress->iColorTrack);
	}
	tFill = pWidget->tContentRect;
	fRate = __xgeXuiProgressRate(pProgress);
	if ( pProgress->iFillDirection == XGE_XUI_PROGRESS_RIGHT_TO_LEFT ) {
		tFill.fW *= fRate;
		tFill.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tFill.fW;
	} else if ( pProgress->iFillDirection == XGE_XUI_PROGRESS_BOTTOM_TO_TOP ) {
		tFill.fH *= fRate;
		tFill.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH - tFill.fH;
	} else if ( pProgress->iFillDirection == XGE_XUI_PROGRESS_TOP_TO_BOTTOM ) {
		tFill.fH *= fRate;
	} else {
		tFill.fW *= fRate;
	}
	if ( pProgress->bHasFillPatch && (tFill.fW > 0.0f) && (tFill.fH > 0.0f) ) {
		if ( pProgress->iFillPatchMode == XGE_XUI_PROGRESS_FILL_REVEAL && pProgress->tFillPatch.bEasyMode ) {
			__xgeXuiProgressDrawRevealPatch(&pProgress->tFillPatch, pWidget->tContentRect, tFill, pProgress->iFillDirection, fRate);
		} else {
			__xgeXuiProgressDrawPatch(&pProgress->tFillPatch, tFill);
		}
	} else if ( (tFill.fW > 0.0f) && (tFill.fH > 0.0f) && (XGE_COLOR_GET_A(pProgress->iColorFill) != 0) ) {
		__xgeXuiHostDrawRect(tFill, pProgress->iColorFill);
	}
	__xgeXuiProgressDrawText(pWidget, pProgress, tFill);
}
