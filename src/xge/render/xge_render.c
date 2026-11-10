#if XGE_ENABLE_2D
void xgeDraw(xge_texture pTexture, float fX, float fY)
{
	xge_draw_t tDraw;

	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = fX;
	tDraw.tDst.fY = fY;
	tDraw.tDst.fW = (float)pTexture->iWidth;
	tDraw.tDst.fH = (float)pTexture->iHeight;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeDrawEx(&tDraw);
}

void xgeDrawPx(xge_texture pTexture, int iX, int iY)
{
	xge_draw_t tDraw;

	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = (float)iX;
	tDraw.tDst.fY = (float)iY;
	tDraw.tDst.fW = (float)pTexture->iWidth;
	tDraw.tDst.fH = (float)pTexture->iHeight;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

void xgeDrawEx(const xge_draw_t* pDraw)
{
	(void)__xgeRenderCommandDraw(pDraw);
}

typedef void (*xge_nine_patch_draw_proc)(const xge_draw_t* pDraw, void* pUser);

static float __xgeNinePatchClamp01(float fValue)
{
	if ( fValue < 0.0f ) {
		return 0.0f;
	}
	if ( fValue > 1.0f ) {
		return 1.0f;
	}
	return fValue;
}

static void __xgeNinePatchNormalize(xge_nine_patch pPatch)
{
	float fSwap;

	if ( pPatch == NULL ) {
		return;
	}
	pPatch->fX1 = __xgeNinePatchClamp01(pPatch->fX1);
	pPatch->fY1 = __xgeNinePatchClamp01(pPatch->fY1);
	pPatch->fX2 = __xgeNinePatchClamp01(pPatch->fX2);
	pPatch->fY2 = __xgeNinePatchClamp01(pPatch->fY2);
	if ( pPatch->fX2 < pPatch->fX1 ) {
		fSwap = pPatch->fX1;
		pPatch->fX1 = pPatch->fX2;
		pPatch->fX2 = fSwap;
	}
	if ( pPatch->fY2 < pPatch->fY1 ) {
		fSwap = pPatch->fY1;
		pPatch->fY1 = pPatch->fY2;
		pPatch->fY2 = fSwap;
	}
	if ( pPatch->iMode != XGE_NINE_PATCH_TILE ) {
		pPatch->iMode = XGE_NINE_PATCH_STRETCH;
	}
	pPatch->bEasyMode = (pPatch->fX1 == 0.0f && pPatch->fY1 == 0.0f && pPatch->fX2 == 1.0f && pPatch->fY2 == 1.0f) ? 1 : 0;
}

void xgeNinePatchInitSimple(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pPatch == NULL ) {
		return;
	}
	memset(pPatch, 0, sizeof(*pPatch));
	pPatch->pTexture = pTexture;
	pPatch->tSrc = tSrc;
	pPatch->fX1 = 0.0f;
	pPatch->fY1 = 0.0f;
	pPatch->fX2 = 1.0f;
	pPatch->fY2 = 1.0f;
	pPatch->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPatch->iMode = XGE_NINE_PATCH_STRETCH;
	pPatch->bEasyMode = 1;
}

void xgeNinePatchInit(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc, float fX1, float fY1, float fX2, float fY2)
{
	if ( pPatch == NULL ) {
		return;
	}
	memset(pPatch, 0, sizeof(*pPatch));
	pPatch->pTexture = pTexture;
	pPatch->tSrc = tSrc;
	pPatch->fX1 = fX1;
	pPatch->fY1 = fY1;
	pPatch->fX2 = fX2;
	pPatch->fY2 = fY2;
	pPatch->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPatch->iMode = XGE_NINE_PATCH_STRETCH;
	__xgeNinePatchNormalize(pPatch);
}

void xgeNinePatchSetMode(xge_nine_patch pPatch, int iMode)
{
	if ( pPatch == NULL ) {
		return;
	}
	pPatch->iMode = (iMode == XGE_NINE_PATCH_TILE) ? XGE_NINE_PATCH_TILE : XGE_NINE_PATCH_STRETCH;
	__xgeNinePatchNormalize(pPatch);
}

void xgeNinePatchSetColor(xge_nine_patch pPatch, uint32_t iColor)
{
	if ( pPatch == NULL ) {
		return;
	}
	pPatch->iColor = iColor;
}

static uint32_t __xgeNinePatchMulColor(uint32_t iA, uint32_t iB)
{
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t a;

	r = (uint32_t)XGE_COLOR_GET_R(iA) * (uint32_t)XGE_COLOR_GET_R(iB) / 255u;
	g = (uint32_t)XGE_COLOR_GET_G(iA) * (uint32_t)XGE_COLOR_GET_G(iB) / 255u;
	b = (uint32_t)XGE_COLOR_GET_B(iA) * (uint32_t)XGE_COLOR_GET_B(iB) / 255u;
	a = (uint32_t)XGE_COLOR_GET_A(iA) * (uint32_t)XGE_COLOR_GET_A(iB) / 255u;
	return XGE_COLOR_RGBA(r, g, b, a);
}

static void __xgeNinePatchDrawStretchPiece(const xge_nine_patch_t* pPatch, xge_rect_t tSrc, xge_rect_t tDst, uint32_t iColor, uint32_t iFlags, xge_nine_patch_draw_proc procDraw, void* pUser)
{
	xge_draw_t tDraw;

	if ( (pPatch == NULL) || (procDraw == NULL) || (pPatch->pTexture == NULL) || (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pPatch->pTexture;
	tDraw.tSrc = tSrc;
	tDraw.tDst = tDst;
	tDraw.iColor = iColor;
	tDraw.iFlags = iFlags;
	procDraw(&tDraw, pUser);
}

static void __xgeNinePatchDrawTilePiece(const xge_nine_patch_t* pPatch, xge_rect_t tSrc, xge_rect_t tDst, uint32_t iColor, uint32_t iFlags, xge_nine_patch_draw_proc procDraw, void* pUser)
{
	xge_rect_t tTileSrc;
	xge_rect_t tTileDst;
	float fY;
	float fX;
	float fTileW;
	float fTileH;
	int iTileCount;
	const int iTileLimit = 16384;

	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return;
	}
	iTileCount = 0;
	for ( fY = 0.0f; fY < tDst.fH; fY += tSrc.fH ) {
		fTileH = tSrc.fH;
		if ( fY + fTileH > tDst.fH ) {
			fTileH = tDst.fH - fY;
		}
		for ( fX = 0.0f; fX < tDst.fW; fX += tSrc.fW ) {
			if ( iTileCount++ >= iTileLimit ) {
				/* Keep pathological source dimensions bounded while preserving the
				 * remaining destination coverage. */
				tTileSrc = tSrc;
				tTileDst = (xge_rect_t){tDst.fX + fX, tDst.fY + fY, tDst.fW - fX, tDst.fH - fY};
				__xgeNinePatchDrawStretchPiece(pPatch, tTileSrc, tTileDst, iColor, iFlags, procDraw, pUser);
				return;
			}
			fTileW = tSrc.fW;
			if ( fX + fTileW > tDst.fW ) {
				fTileW = tDst.fW - fX;
			}
			tTileSrc = tSrc;
			tTileSrc.fW = fTileW;
			tTileSrc.fH = fTileH;
			tTileDst.fX = tDst.fX + fX;
			tTileDst.fY = tDst.fY + fY;
			tTileDst.fW = fTileW;
			tTileDst.fH = fTileH;
			__xgeNinePatchDrawStretchPiece(pPatch, tTileSrc, tTileDst, iColor, iFlags, procDraw, pUser);
		}
	}
}

static void __xgeNinePatchDrawPiece(const xge_nine_patch_t* pPatch, xge_rect_t tSrc, xge_rect_t tDst, uint32_t iColor, uint32_t iFlags, int bTile, xge_nine_patch_draw_proc procDraw, void* pUser)
{
	if ( bTile ) {
		__xgeNinePatchDrawTilePiece(pPatch, tSrc, tDst, iColor, iFlags, procDraw, pUser);
	} else {
		__xgeNinePatchDrawStretchPiece(pPatch, tSrc, tDst, iColor, iFlags, procDraw, pUser);
	}
}

static void __xgeNinePatchScaleFixed(float fTotal, float* pA, float* pB, float* pMid)
{
	float fFixed;
	float fScale;

	fFixed = *pA + *pB;
	if ( (fTotal < fFixed) && (fFixed > 0.0f) ) {
		fScale = fTotal / fFixed;
		*pA *= fScale;
		*pB *= fScale;
		*pMid = 0.0f;
	} else {
		*pMid = fTotal - fFixed;
	}
	if ( *pMid < 0.0f ) {
		*pMid = 0.0f;
	}
}

static void __xgeNinePatchDrawInternal(const xge_nine_patch_t* pPatch, xge_rect_t tDst, uint32_t iFlags, uint32_t iDrawColor, xge_nine_patch_draw_proc procDraw, void* pUser)
{
	xge_nine_patch_t tPatch;
	xge_rect_t tSrc;
	xge_rect_t arrSrc[3][3];
	xge_rect_t arrDst[3][3];
	float arrSrcX[4];
	float arrSrcY[4];
	float arrDstX[4];
	float arrDstY[4];
	float fLeftW;
	float fRightW;
	float fMidW;
	float fTopH;
	float fBottomH;
	float fMidH;
	uint32_t iColor;
	int i;
	int j;

	if ( (pPatch == NULL) || (pPatch->pTexture == NULL) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) || (procDraw == NULL) ) {
		return;
	}
	tPatch = *pPatch;
	__xgeNinePatchNormalize(&tPatch);
	if ( XGE_COLOR_GET_A(tPatch.iColor) == 0 ) {
		return;
	}
	iColor = __xgeNinePatchMulColor(tPatch.iColor, iDrawColor);
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	tSrc = tPatch.tSrc;
	if ( tSrc.fW == 0.0f ) {
		tSrc.fW = (float)tPatch.pTexture->iWidth;
	}
	if ( tSrc.fH == 0.0f ) {
		tSrc.fH = (float)tPatch.pTexture->iHeight;
	}
	if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ) {
		return;
	}
	if ( tPatch.bEasyMode ) {
		__xgeNinePatchDrawPiece(&tPatch, tSrc, tDst, iColor, iFlags, tPatch.iMode == XGE_NINE_PATCH_TILE, procDraw, pUser);
		return;
	}
	arrSrcX[0] = tSrc.fX;
	arrSrcX[1] = tSrc.fX + tSrc.fW * tPatch.fX1;
	arrSrcX[2] = tSrc.fX + tSrc.fW * tPatch.fX2;
	arrSrcX[3] = tSrc.fX + tSrc.fW;
	arrSrcY[0] = tSrc.fY;
	arrSrcY[1] = tSrc.fY + tSrc.fH * tPatch.fY1;
	arrSrcY[2] = tSrc.fY + tSrc.fH * tPatch.fY2;
	arrSrcY[3] = tSrc.fY + tSrc.fH;
	fLeftW = arrSrcX[1] - arrSrcX[0];
	fRightW = arrSrcX[3] - arrSrcX[2];
	fTopH = arrSrcY[1] - arrSrcY[0];
	fBottomH = arrSrcY[3] - arrSrcY[2];
	__xgeNinePatchScaleFixed(tDst.fW, &fLeftW, &fRightW, &fMidW);
	__xgeNinePatchScaleFixed(tDst.fH, &fTopH, &fBottomH, &fMidH);
	arrDstX[0] = tDst.fX;
	arrDstX[1] = tDst.fX + fLeftW;
	arrDstX[2] = arrDstX[1] + fMidW;
	arrDstX[3] = tDst.fX + tDst.fW;
	arrDstY[0] = tDst.fY;
	arrDstY[1] = tDst.fY + fTopH;
	arrDstY[2] = arrDstY[1] + fMidH;
	arrDstY[3] = tDst.fY + tDst.fH;
	for ( j = 0; j < 3; j++ ) {
		for ( i = 0; i < 3; i++ ) {
			arrSrc[j][i].fX = arrSrcX[i];
			arrSrc[j][i].fY = arrSrcY[j];
			arrSrc[j][i].fW = arrSrcX[i + 1] - arrSrcX[i];
			arrSrc[j][i].fH = arrSrcY[j + 1] - arrSrcY[j];
			arrDst[j][i].fX = arrDstX[i];
			arrDst[j][i].fY = arrDstY[j];
			arrDst[j][i].fW = arrDstX[i + 1] - arrDstX[i];
			arrDst[j][i].fH = arrDstY[j + 1] - arrDstY[j];
			__xgeNinePatchDrawPiece(&tPatch, arrSrc[j][i], arrDst[j][i], iColor, iFlags, (tPatch.iMode == XGE_NINE_PATCH_TILE) && !((i == 0 || i == 2) && (j == 0 || j == 2)), procDraw, pUser);
		}
	}
}

static void __xgeNinePatchDrawEngineProc(const xge_draw_t* pDraw, void* pUser)
{
	(void)pUser;
	xgeDrawEx(pDraw);
}

void xgeNinePatchDraw(const xge_nine_patch_t* pPatch, xge_rect_t tDst, uint32_t iFlags)
{
	__xgeNinePatchDrawInternal(pPatch, tDst, iFlags, XGE_COLOR_RGBA(255, 255, 255, 255), __xgeNinePatchDrawEngineProc, NULL);
}

static void __xgeDrawExImmediate(const xge_draw_t* pDraw)
{
	float fU0;
	float fV0;
	float fU1;
	float fV1;
	float fLocalX[4];
	float fLocalY[4];
	float fCos;
	float fSin;
	float fR;
	float fG;
	float fB;
	float fA;
	float arrVertices[16];
	int i;
	xge_texture pTexture;
	xge_vec2_t tScreen;
	xge_texture_yuv420p_t* pYUV;

	if ( (pDraw == NULL) || (pDraw->pTexture == NULL) || (pDraw->pTexture->iBackendId == 0) ) {
		return;
	}
	pTexture = pDraw->pTexture;
	if ( __xgeTextureRendererInit() != XGE_OK ) {
		return;
	}

	__xgeTextureDrawRectUV(pDraw, pTexture, &fU0, &fV0, &fU1, &fV1);

	fLocalX[0] = -pDraw->tOrigin.fX;
	fLocalY[0] = -pDraw->tOrigin.fY;
	fLocalX[1] = pDraw->tDst.fW - pDraw->tOrigin.fX;
	fLocalY[1] = -pDraw->tOrigin.fY;
	fLocalX[2] = -pDraw->tOrigin.fX;
	fLocalY[2] = pDraw->tDst.fH - pDraw->tOrigin.fY;
	fLocalX[3] = pDraw->tDst.fW - pDraw->tOrigin.fX;
	fLocalY[3] = pDraw->tDst.fH - pDraw->tOrigin.fY;
	fCos = cosf(pDraw->fRotation);
	fSin = sinf(pDraw->fRotation);

	for ( i = 0; i < 4; i++ ) {
		xge_vec2_t tWorld;
		tWorld.fX = pDraw->tDst.fX + (fLocalX[i] * fCos) - (fLocalY[i] * fSin);
		tWorld.fY = pDraw->tDst.fY + (fLocalX[i] * fSin) + (fLocalY[i] * fCos);
		if ( (pDraw->iFlags & XGE_DRAW_SCREEN_SPACE) != 0 ) {
			tScreen = tWorld;
		} else {
			tScreen = xgeWorldToScreen(tWorld);
		}
		arrVertices[(i * 4) + 0] = tScreen.fX;
		arrVertices[(i * 4) + 1] = tScreen.fY;
	}
	arrVertices[2] = fU0; arrVertices[3] = fV0;
	arrVertices[6] = fU1; arrVertices[7] = fV0;
	arrVertices[10] = fU0; arrVertices[11] = fV1;
	arrVertices[14] = fU1; arrVertices[15] = fV1;

	__xgeColorToFloat(pDraw->iColor, &fR, &fG, &fB, &fA);

	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		if ( !__xgeTextureHasYUV420P(pTexture) ) {
			return;
		}
		pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
		xge_glUseProgram(g_xgeTextureRenderer.iYUVProgram);
		xge_glUniform2f(g_xgeTextureRenderer.iYUVLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
		xge_glUniform1i(g_xgeTextureRenderer.iYUVLocTexY, 0);
		xge_glUniform1i(g_xgeTextureRenderer.iYUVLocTexU, 1);
		xge_glUniform1i(g_xgeTextureRenderer.iYUVLocTexV, 2);
		xge_glUniform4f(g_xgeTextureRenderer.iYUVLocColor, fR, fG, fB, fA);
		xge_glActiveTexture(GL_TEXTURE0);
		xge_glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[0]);
		xge_glActiveTexture(GL_TEXTURE0 + 1);
		xge_glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[1]);
		xge_glActiveTexture(GL_TEXTURE0 + 2);
		xge_glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[2]);
	} else {
		xge_glUseProgram(g_xgeTextureRenderer.iProgram);
		xge_glUniform2f(g_xgeTextureRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
		xge_glUniform1i(g_xgeTextureRenderer.iLocTexture, 0);
		xge_glUniform4f(g_xgeTextureRenderer.iLocColor, fR, fG, fB, fA);
		xge_glActiveTexture(GL_TEXTURE0);
		xge_glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
	}
	xge_glBindVertexArray(g_xgeTextureRenderer.iVAO);
	xge_glBindBuffer(GL_ARRAY_BUFFER, g_xgeTextureRenderer.iVBO);
	xge_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	xge_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	xge_glBindBuffer(GL_ARRAY_BUFFER, 0);
	xge_glBindVertexArray(0);
	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		xge_glActiveTexture(GL_TEXTURE0 + 2);
		xge_glBindTexture(GL_TEXTURE_2D, 0);
		xge_glActiveTexture(GL_TEXTURE0 + 1);
		xge_glBindTexture(GL_TEXTURE_2D, 0);
		xge_glActiveTexture(GL_TEXTURE0);
	}
	xge_glBindTexture(GL_TEXTURE_2D, 0);
	xge_glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}

#endif

static int __xgeRenderRectFinite(xge_rect_t tRect)
{
	return isfinite(tRect.fX) && isfinite(tRect.fY) &&
		isfinite(tRect.fW) && isfinite(tRect.fH);
}

static int __xgeRenderFloatToInt(double fValue)
{
	if ( !isfinite(fValue) ) return 0;
	if ( fValue >= (double)INT_MAX ) return INT_MAX;
	if ( fValue <= (double)INT_MIN ) return INT_MIN;
	return (int)fValue;
}

/* Apply already-published CPU state. Pass restoration must not recursively
 * Flush while its ending reservation owns the target transition. */
static void __xgeViewportApplyCurrent(void)
{
    xge_rect_t tRect=g_xge.bViewportEnabled ? g_xge.tViewportRect :
        (xge_rect_t){0,0,(float)g_xge.iWidth,(float)g_xge.iHeight};
	GLint iX;
	GLint iY;
	GLsizei iW;
	GLsizei iH;

	if ( (g_xge.bGraphicsRunning == 0) || (xge_glViewport == NULL) ) {
		return;
	}
	iX = (GLint)__xgeRenderFloatToInt((double)tRect.fX);
	iY = (GLint)__xgeRenderFloatToInt((double)g_xge.iHeight -
		(double)tRect.fY - (double)tRect.fH);
	iW = (GLsizei)__xgeRenderFloatToInt((double)tRect.fW);
	iH = (GLsizei)__xgeRenderFloatToInt((double)tRect.fH);
	xge_glViewport(iX, iY, iW, iH);
 }
void xgeViewportSet(xge_rect_t tRect)
{
    (void)xgeFlush();
    if(!__xgeRenderRectFinite(tRect))memset(&tRect,0,sizeof(tRect));
    if(tRect.fW<0)tRect.fW=0;
    if(tRect.fH<0)tRect.fH=0;
    g_xge.bViewportEnabled=1;g_xge.tViewportRect=tRect;__xgeViewportApplyCurrent();
}

xge_rect_t xgeViewportGet(void)
{
	xge_rect_t tRect;

	if ( g_xge.bViewportEnabled ) {
		return g_xge.tViewportRect;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)g_xge.iWidth;
	tRect.fH = (float)g_xge.iHeight;
	return tRect;
}

void xgeViewportClear(void)
{
    (void)xgeFlush();g_xge.bViewportEnabled=0;
    memset(&g_xge.tViewportRect,0,sizeof(g_xge.tViewportRect));__xgeViewportApplyCurrent();
}

static int __xgePixelRound(double fValue)
{
	if ( !isfinite(fValue) ) return 0;
	return __xgeRenderFloatToInt(floor(fValue + 0.5));
}

static int __xgePixelFloor(double fValue)
{
	if ( !isfinite(fValue) ) return 0;
	return __xgeRenderFloatToInt(floor(fValue));
}

static int __xgePixelCeil(double fValue)
{
	if ( !isfinite(fValue) ) return 0;
	return __xgeRenderFloatToInt(ceil(fValue));
}

static int __xgePixelExtent(int iStart, int iEnd)
{
	int64_t iExtent = (int64_t)iEnd - (int64_t)iStart;
	if ( iExtent <= 0 ) return 0;
	return iExtent > (int64_t)INT_MAX ? INT_MAX : (int)iExtent;
}

xge_rect_i_t xgeRectToPixelsNearest(xge_rect_t tRect)
{
	xge_rect_i_t tPixels;
	int iRight;
	int iBottom;

	if ( !__xgeRenderRectFinite(tRect) ) return (xge_rect_i_t){0, 0, 0, 0};
	tPixels.iX = __xgePixelRound(tRect.fX);
	tPixels.iY = __xgePixelRound(tRect.fY);
	iRight = __xgePixelRound((double)tRect.fX + (double)tRect.fW);
	iBottom = __xgePixelRound((double)tRect.fY + (double)tRect.fH);
	tPixels.iW = __xgePixelExtent(tPixels.iX, iRight);
	tPixels.iH = __xgePixelExtent(tPixels.iY, iBottom);
	return tPixels;
}

xge_rect_i_t xgeRectToPixelsOutward(xge_rect_t tRect)
{
	xge_rect_i_t tPixels;
	int iRight;
	int iBottom;

	if ( !__xgeRenderRectFinite(tRect) ) return (xge_rect_i_t){0, 0, 0, 0};
	tPixels.iX = __xgePixelFloor(tRect.fX);
	tPixels.iY = __xgePixelFloor(tRect.fY);
	iRight = __xgePixelCeil((double)tRect.fX + (double)tRect.fW);
	iBottom = __xgePixelCeil((double)tRect.fY + (double)tRect.fH);
	tPixels.iW = __xgePixelExtent(tPixels.iX, iRight);
	tPixels.iH = __xgePixelExtent(tPixels.iY, iBottom);
	return tPixels;
}

xge_rect_i_t xgeRectToPixelsInward(xge_rect_t tRect)
{
	xge_rect_i_t tPixels;
	int iRight;
	int iBottom;

	if ( !__xgeRenderRectFinite(tRect) ) return (xge_rect_i_t){0, 0, 0, 0};
	tPixels.iX = __xgePixelCeil(tRect.fX);
	tPixels.iY = __xgePixelCeil(tRect.fY);
	iRight = __xgePixelFloor((double)tRect.fX + (double)tRect.fW);
	iBottom = __xgePixelFloor((double)tRect.fY + (double)tRect.fH);
	tPixels.iW = __xgePixelExtent(tPixels.iX, iRight);
	tPixels.iH = __xgePixelExtent(tPixels.iY, iBottom);
	return tPixels;
}

void xgeViewportSetPixels(xge_rect_i_t tRect)
{
	if ( tRect.iW < 0 ) tRect.iW = 0;
	if ( tRect.iH < 0 ) tRect.iH = 0;
	xgeViewportSet((xge_rect_t){(float)tRect.iX, (float)tRect.iY,
		(float)tRect.iW, (float)tRect.iH});
}

xge_rect_i_t xgeViewportGetPixels(void)
{
	return xgeRectToPixelsNearest(xgeViewportGet());
}

static void __xgeClipApplyCurrent(void)
{
    if(!g_xge.bClipEnabled){
        if(g_xge.bGraphicsRunning && xge_glDisable)xge_glDisable(GL_SCISSOR_TEST);
        return;
    }
    xge_rect_t tRect=g_xge.tClipRect;
	GLint iX;
	GLint iY;
	GLsizei iW;
	GLsizei iH;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fScissorBottom;

	if ( (g_xge.bGraphicsRunning == 0) || (xge_glScissor == NULL) ) {
		return;
	}
	fLeft = floorf(tRect.fX);
	fTop = floorf(tRect.fY);
	fRight = ceilf(tRect.fX + tRect.fW);
	fBottom = ceilf(tRect.fY + tRect.fH);
	if ( fRight < fLeft ) {
		fRight = fLeft;
	}
	if ( fBottom < fTop ) {
		fBottom = fTop;
	}
	if ( fLeft < 0.0f ) {
		fLeft = 0.0f;
	}
	if ( fTop < 0.0f ) {
		fTop = 0.0f;
	}
	if ( fLeft > (float)g_xge.iWidth ) {
		fLeft = (float)g_xge.iWidth;
	}
	if ( fTop > (float)g_xge.iHeight ) {
		fTop = (float)g_xge.iHeight;
	}
	if ( fRight < 0.0f ) {
		fRight = 0.0f;
	}
	if ( fBottom < 0.0f ) {
		fBottom = 0.0f;
	}
	if ( fRight > (float)g_xge.iWidth ) {
		fRight = (float)g_xge.iWidth;
	}
	if ( fBottom > (float)g_xge.iHeight ) {
		fBottom = (float)g_xge.iHeight;
	}
	if ( fRight < fLeft ) {
		fRight = fLeft;
	}
	if ( fBottom < fTop ) {
		fBottom = fTop;
	}
	fScissorBottom = floorf((float)g_xge.iHeight - fBottom);
	iX = (GLint)__xgeRenderFloatToInt(fLeft);
	iY = (GLint)__xgeRenderFloatToInt(fScissorBottom);
	iW = (GLsizei)__xgeRenderFloatToInt((double)fRight - fLeft);
	iH = (GLsizei)__xgeRenderFloatToInt((double)fBottom - fTop);
	xge_glEnable(GL_SCISSOR_TEST);
	xge_glScissor(iX, iY, iW, iH);
 }
void xgeClipSet(xge_rect_t tRect)
{
    (void)xgeFlush();
    if(!__xgeRenderRectFinite(tRect))memset(&tRect,0,sizeof(tRect));
    if(tRect.fW<0)tRect.fW=0;
    if(tRect.fH<0)tRect.fH=0;
    g_xge.bClipEnabled=1;g_xge.tClipRect=tRect;__xgeClipApplyCurrent();
}

void xgeClipSetPixels(xge_rect_i_t tRect)
{
	if ( tRect.iW < 0 ) tRect.iW = 0;
	if ( tRect.iH < 0 ) tRect.iH = 0;
	xgeClipSet((xge_rect_t){(float)tRect.iX, (float)tRect.iY,
		(float)tRect.iW, (float)tRect.iH});
}

xge_rect_t xgeClipGet(void)
{
	xge_rect_t tRect;

	if ( g_xge.bClipEnabled ) {
		return g_xge.tClipRect;
	}
	memset(&tRect, 0, sizeof(tRect));
	return tRect;
}

xge_rect_i_t xgeClipGetPixels(void)
{
	if ( !g_xge.bClipEnabled ) {
		return (xge_rect_i_t){0, 0, 0, 0};
	}
	return xgeRectToPixelsNearest(g_xge.tClipRect);
}

void xgeClipClear(void)
{
    (void)xgeFlush();g_xge.bClipEnabled=0;
    memset(&g_xge.tClipRect,0,sizeof(g_xge.tClipRect));__xgeClipApplyCurrent();
}

#if defined(_WIN32) || defined(_WIN64)
#define XGE_WIN32_CLIPBOARD_RETRY_COUNT 30
#define XGE_WIN32_CLIPBOARD_RETRY_DELAY_MS 2

static int __xgeWin32ClipboardCacheReserve(size_t iCapacity)
{
	char* sNew;

	if ( iCapacity <= g_xge.iClipboardTextCapacity ) {
		return 1;
	}
	sNew = (char*)xrtRealloc(g_xge.sClipboardText, iCapacity);
	if ( sNew == NULL ) {
		return 0;
	}
	g_xge.sClipboardText = sNew;
	g_xge.iClipboardTextCapacity = iCapacity;
	return 1;
}

static HWND g_xgeWin32ClipboardOwnerWindow = NULL;
static struct {
	unsigned char* png;
	size_t size;
	DWORD sequence;
} g_xgeWin32ClipboardDibPngCache = {0};

static void __xgeWin32ClipboardDibPngCacheClear(void)
{
	xrtFree(g_xgeWin32ClipboardDibPngCache.png);
	memset(&g_xgeWin32ClipboardDibPngCache, 0,
		sizeof(g_xgeWin32ClipboardDibPngCache));
}

static int __xgeWin32ClipboardDibPngCacheRead(void* data, size_t capacity, int* result)
{
	DWORD sequence;
	if ( g_xgeWin32ClipboardDibPngCache.png == NULL ) return 0;
	sequence = GetClipboardSequenceNumber();
	if ( sequence == 0u || sequence != g_xgeWin32ClipboardDibPngCache.sequence ) {
		__xgeWin32ClipboardDibPngCacheClear();
		return 0;
	}
	*result = (int)g_xgeWin32ClipboardDibPngCache.size;
	if ( data != NULL && capacity > 0u ) {
		memcpy(data, g_xgeWin32ClipboardDibPngCache.png,
			capacity < g_xgeWin32ClipboardDibPngCache.size ?
			capacity : g_xgeWin32ClipboardDibPngCache.size);
		if ( capacity >= g_xgeWin32ClipboardDibPngCache.size )
			__xgeWin32ClipboardDibPngCacheClear();
	}
	return 1;
}

static void __xgeWin32ClipboardOwnerRelease(void)
{
	__xgeWin32ClipboardDibPngCacheClear();
	if ( g_xgeWin32ClipboardOwnerWindow != NULL ) {
		DestroyWindow(g_xgeWin32ClipboardOwnerWindow);
		g_xgeWin32ClipboardOwnerWindow = NULL;
	}
}

static HWND __xgeWin32ClipboardOwner(void)
{
	if ( g_xge.bSokolRunning ) {
		HWND window = (HWND)sapp_win32_get_hwnd();
		if ( window != NULL ) return window;
	}
	if ( g_xgeWin32ClipboardOwnerWindow == NULL )
		g_xgeWin32ClipboardOwnerWindow = CreateWindowExA(0, "STATIC", "XGE Clipboard",
			WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);
	return g_xgeWin32ClipboardOwnerWindow;
}

static int __xgeWin32ClipboardOpen(void)
{
	HWND owner = __xgeWin32ClipboardOwner();
	int iAttempt;
	if ( owner == NULL ) return 0;

	for ( iAttempt = 0; iAttempt < XGE_WIN32_CLIPBOARD_RETRY_COUNT; iAttempt++ ) {
		if ( OpenClipboard(owner) ) {
			return 1;
		}
		if ( iAttempt + 1 < XGE_WIN32_CLIPBOARD_RETRY_COUNT ) {
			Sleep(XGE_WIN32_CLIPBOARD_RETRY_DELAY_MS);
		}
	}
	return 0;
}

static const char* __xgeWin32ClipboardCacheEmpty(DWORD iSequence)
{
	if ( __xgeWin32ClipboardCacheReserve(1u) ) {
		g_xge.sClipboardText[0] = '\0';
		g_xge.iClipboardSequence = (uint32_t)iSequence;
		g_xge.iClipboardCacheValid = 1;
		return g_xge.sClipboardText;
	}
	g_xge.iClipboardSequence = 0u;
	g_xge.iClipboardCacheValid = 0;
	return "";
}

static const char* __xgeWin32ClipboardCacheInvalidate(void)
{
	if ( __xgeWin32ClipboardCacheReserve(1u) ) g_xge.sClipboardText[0] = '\0';
	g_xge.iClipboardSequence = 0u;
	g_xge.iClipboardCacheValid = 0;
	return NULL;
}

static int __xgeWin32ClipboardSetText(const char* sText)
{
	HGLOBAL hMemory;
	WCHAR* sWide;
	int iWideCount;
	int bSuccess;
	size_t iTextSize;
	DWORD iSequence;

	iWideCount = MultiByteToWideChar(CP_UTF8, 0, sText, -1, NULL, 0);
	if ( iWideCount <= 0 ) {
		return 0;
	}
	hMemory = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)iWideCount * sizeof(WCHAR));
	if ( hMemory == NULL ) {
		return 0;
	}
	sWide = (WCHAR*)GlobalLock(hMemory);
	if ( sWide == NULL ) {
		GlobalFree(hMemory);
		return 0;
	}
	if ( MultiByteToWideChar(CP_UTF8, 0, sText, -1, sWide, iWideCount) <= 0 ) {
		GlobalUnlock(hMemory);
		GlobalFree(hMemory);
		return 0;
	}
	GlobalUnlock(hMemory);
	if ( !__xgeWin32ClipboardOpen() ) {
		GlobalFree(hMemory);
		return 0;
	}
	bSuccess = 0;
	iSequence = 0u;
	if ( EmptyClipboard() && SetClipboardData(CF_UNICODETEXT, hMemory) != NULL ) {
		bSuccess = 1;
		hMemory = NULL;
		iSequence = GetClipboardSequenceNumber();
	}
	CloseClipboard();
	if ( hMemory != NULL ) {
		GlobalFree(hMemory);
	}
	if ( !bSuccess ) {
		return 0;
	}
	iTextSize = strlen(sText) + 1u;
	if ( __xgeWin32ClipboardCacheReserve(iTextSize) ) {
		memcpy(g_xge.sClipboardText, sText, iTextSize);
		g_xge.iClipboardSequence = (uint32_t)iSequence;
		g_xge.iClipboardCacheValid = 1;
	} else {
		g_xge.iClipboardSequence = 0u;
		g_xge.iClipboardCacheValid = 0;
	}
	return 1;
}

static const char* __xgeWin32ClipboardGetText(void)
{
	HANDLE hData;
	const WCHAR* sWide;
	SIZE_T iWideCapacity;
	SIZE_T iWideLength;
	DWORD iSequence;
	DWORD iReadSequence;
	int iUtf8Size;
	int iConverted;
	int iAttempt;

	iSequence = GetClipboardSequenceNumber();
	if ( g_xge.iClipboardCacheValid && iSequence != 0u && g_xge.sClipboardText != NULL && g_xge.iClipboardSequence == (uint32_t)iSequence ) {
		return g_xge.sClipboardText;
	}
	if ( !IsClipboardFormatAvailable(CF_UNICODETEXT) ) {
		return __xgeWin32ClipboardCacheEmpty(iSequence);
	}
	hData = NULL;
	iReadSequence = 0u;
	for ( iAttempt = 0; iAttempt < XGE_WIN32_CLIPBOARD_RETRY_COUNT; iAttempt++ ) {
		if ( OpenClipboard(__xgeWin32ClipboardOwner()) ) {
			iReadSequence = GetClipboardSequenceNumber();
			hData = GetClipboardData(CF_UNICODETEXT);
			if ( hData != NULL ) {
				break;
			}
			CloseClipboard();
		}
		if ( iAttempt + 1 < XGE_WIN32_CLIPBOARD_RETRY_COUNT ) {
			Sleep(XGE_WIN32_CLIPBOARD_RETRY_DELAY_MS);
		}
	}
	if ( hData == NULL ) {
		return __xgeWin32ClipboardCacheInvalidate();
	}
	sWide = (const WCHAR*)GlobalLock(hData);
	if ( sWide == NULL ) {
		CloseClipboard();
		return __xgeWin32ClipboardCacheInvalidate();
	}
	iWideCapacity = GlobalSize(hData) / sizeof(WCHAR);
	iWideLength = 0u;
	while ( iWideLength < iWideCapacity && sWide[iWideLength] != 0 ) iWideLength++;
	if ( (iWideLength == iWideCapacity) || (iWideLength >= (SIZE_T)INT_MAX) ) {
		GlobalUnlock(hData);
		CloseClipboard();
		return __xgeWin32ClipboardCacheInvalidate();
	}
	/* Include the validated terminator; never ask Win32 to scan clipboard memory. */
	iWideLength++;
	iUtf8Size = WideCharToMultiByte(CP_UTF8, 0, sWide, (int)iWideLength, NULL, 0, NULL, NULL);
	if ( iUtf8Size <= 0 || !__xgeWin32ClipboardCacheReserve((size_t)iUtf8Size) ) {
		GlobalUnlock(hData);
		CloseClipboard();
		return __xgeWin32ClipboardCacheInvalidate();
	}
	iConverted = WideCharToMultiByte(CP_UTF8, 0, sWide, (int)iWideLength,
		g_xge.sClipboardText, iUtf8Size, NULL, NULL);
	GlobalUnlock(hData);
	CloseClipboard();
	if ( iConverted <= 0 ) {
		return __xgeWin32ClipboardCacheInvalidate();
	}
	g_xge.iClipboardSequence = (uint32_t)iReadSequence;
	g_xge.iClipboardCacheValid = 1;
	return g_xge.sClipboardText;
}

typedef struct xge_win32_clipboard_item_t {
	UINT iFormat;
	HGLOBAL hMemory;
} xge_win32_clipboard_item_t;

static HGLOBAL __xgeWin32ClipboardMemory(const void* pData, size_t iSize, int bTerminate)
{
	HGLOBAL hMemory;
	unsigned char* pTarget;
	size_t iTotal = iSize + (bTerminate ? 1u : 0u);
	if ( iTotal < iSize ) return NULL;
	hMemory = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)iTotal);
	if ( hMemory == NULL ) return NULL;
	if(iTotal==0u)return hMemory; /* Valid discarded handle: exact empty bytes. */
	pTarget = (unsigned char*)GlobalLock(hMemory);
	if ( pTarget == NULL ) { GlobalFree(hMemory); return NULL; }
	if ( iSize > 0u ) memcpy(pTarget, pData, iSize);
	if ( bTerminate ) pTarget[iSize] = 0;
	GlobalUnlock(hMemory);
	return hMemory;
}

static const unsigned char* __xgeWin32ClipboardFind(const unsigned char* pData,
	size_t iSize, const char* sNeedle)
{
	size_t iNeedleSize = strlen(sNeedle);
	size_t i;
	if ( (pData == NULL) || (iNeedleSize == 0u) || (iNeedleSize > iSize) ) return NULL;
	for ( i = 0u; i <= iSize - iNeedleSize; i++ ) {
		if ( memcmp(pData + i, sNeedle, iNeedleSize) == 0 ) return pData + i;
	}
	return NULL;
}

static int __xgeWin32ClipboardParseOffset(const unsigned char* pText,
	const unsigned char* pEnd, size_t* pOffset)
{
	size_t iValue = 0u;
	int bDigit = 0;
	if ( (pText == NULL) || (pEnd == NULL) || (pOffset == NULL) ) return 0;
	while ( pText < pEnd && *pText >= (unsigned char)'0' && *pText <= (unsigned char)'9' ) {
		unsigned int iDigit = (unsigned int)(*pText - (unsigned char)'0');
		if ( iValue > (SIZE_MAX - (size_t)iDigit) / 10u ) return 0;
		iValue = iValue * 10u + (size_t)iDigit;
		bDigit = 1;
		pText++;
	}
	if ( !bDigit ) return 0;
	*pOffset = iValue;
	return 1;
}

static HGLOBAL __xgeWin32ClipboardUtf8Text(const void* pData, size_t iSize)
{
	HGLOBAL hMemory;
	WCHAR* sWide;
	int iWideCount;
	if ( iSize > (size_t)INT_MAX ) return NULL;
	iWideCount = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char*)pData, (int)iSize, NULL, 0);
	if ( iWideCount <= 0 && iSize != 0u ) return NULL;
	hMemory = GlobalAlloc(GMEM_MOVEABLE, ((SIZE_T)iWideCount + 1u) * sizeof(WCHAR));
	if ( hMemory == NULL ) return NULL;
	sWide = (WCHAR*)GlobalLock(hMemory);
	if ( sWide == NULL ) { GlobalFree(hMemory); return NULL; }
	if ( iWideCount > 0 && MultiByteToWideChar(CP_UTF8, 0, (const char*)pData,
		(int)iSize, sWide, iWideCount) != iWideCount ) {
		GlobalUnlock(hMemory); GlobalFree(hMemory); return NULL;
	}
	sWide[iWideCount] = 0;
	GlobalUnlock(hMemory);
	return hMemory;
}

static HGLOBAL __xgeWin32ClipboardHtml(const void* pData, size_t iSize)
{
	static const char sHeader[] = "Version:0.9\r\nStartHTML:%010u\r\nEndHTML:%010u\r\nStartFragment:%010u\r\nEndFragment:%010u\r\n";
	static const char sPrefix[] = "<html><body><!--StartFragment-->";
	static const char sSuffix[] = "<!--EndFragment--></body></html>";
	char arrHeader[160];
	char* sOutput;
	size_t iHeaderSize;
	size_t iTotal;
	unsigned int iStartHtml;
	unsigned int iEndHtml;
	unsigned int iStartFragment;
	unsigned int iEndFragment;
	HGLOBAL hMemory;
	int iWritten = snprintf(arrHeader, sizeof(arrHeader), sHeader, 0u, 0u, 0u, 0u);
	if ( iWritten <= 0 ) return NULL;
	iHeaderSize = (size_t)iWritten;
	if ( iSize > (size_t)UINT_MAX || iHeaderSize + sizeof(sPrefix) + sizeof(sSuffix) + iSize > (size_t)UINT_MAX ) return NULL;
	iStartHtml = (unsigned int)iHeaderSize;
	iStartFragment = iStartHtml + (unsigned int)(sizeof(sPrefix) - 1u);
	iEndFragment = iStartFragment + (unsigned int)iSize;
	iEndHtml = iEndFragment + (unsigned int)(sizeof(sSuffix) - 1u);
	iWritten = snprintf(arrHeader, sizeof(arrHeader), sHeader, iStartHtml, iEndHtml,
		iStartFragment, iEndFragment);
	if ( iWritten <= 0 || (size_t)iWritten != iHeaderSize ) return NULL;
	iTotal = (size_t)iEndHtml;
	sOutput = (char*)xrtMalloc(iTotal);
	if ( sOutput == NULL ) return NULL;
	memcpy(sOutput, arrHeader, iHeaderSize);
	memcpy(sOutput + iStartHtml, sPrefix, sizeof(sPrefix) - 1u);
	if ( iSize > 0u ) memcpy(sOutput + iStartFragment, pData, iSize);
	memcpy(sOutput + iEndFragment, sSuffix, sizeof(sSuffix) - 1u);
	hMemory = __xgeWin32ClipboardMemory(sOutput, iTotal, 1);
	xrtFree(sOutput);
	return hMemory;
}

/* A PNG remains the lossless clipboard format. Add a packed 32-bit DIBV5
 * for Windows applications that only understand standard bitmap formats. */
static HGLOBAL __xgeWin32ClipboardPngDibV5(const void* data, size_t size)
{
	static const unsigned char signature[8] = {137u, 80u, 78u, 71u, 13u, 10u, 26u, 10u};
	xge_image_t image = {0};
	BITMAPV5HEADER header;
	HGLOBAL memory;
	unsigned char* target;
	size_t pixels_size;
	int width = 0, height = 0, x, y;
	if ( data == NULL || size < sizeof(signature) || size > (size_t)INT_MAX ||
	     memcmp(data, signature, sizeof(signature)) != 0 ||
	     xgeImageInfoMemory(data, (int)size, &width, &height) != XGE_OK ||
	     width <= 0 || height <= 0 ||
	     (uint64_t)width * (uint64_t)height > UINT64_C(16000000) ||
	     xgeImageLoadMemoryEx(&image, data, (int)size,
		XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) return NULL;
	if ( image.iWidth != width || image.iHeight != height ) {
		xgeImageFree(&image);
		return NULL;
	}
	pixels_size = (size_t)width * (size_t)height * 4u;
	memory = GlobalAlloc(GMEM_MOVEABLE, sizeof(header) + pixels_size);
	if ( memory == NULL ) { xgeImageFree(&image); return NULL; }
	target = (unsigned char*)GlobalLock(memory);
	if ( target == NULL ) {
		GlobalFree(memory); xgeImageFree(&image); return NULL;
	}
	memset(&header, 0, sizeof(header));
	header.bV5Size = sizeof(header);
	header.bV5Width = width;
	header.bV5Height = -height;
	header.bV5Planes = 1;
	header.bV5BitCount = 32;
	header.bV5Compression = BI_BITFIELDS;
	header.bV5SizeImage = (DWORD)pixels_size;
	header.bV5RedMask = 0x00ff0000u;
	header.bV5GreenMask = 0x0000ff00u;
	header.bV5BlueMask = 0x000000ffu;
	header.bV5AlphaMask = 0xff000000u;
	header.bV5CSType = 0x73524742u; /* LCS_sRGB, without a multi-character literal. */
	memcpy(target, &header, sizeof(header));
	for ( y = 0; y < height; y++ ) {
		const unsigned char* source = (const unsigned char*)image.pPixels +
			(size_t)y * (size_t)image.iStride;
		unsigned char* row = target + sizeof(header) + (size_t)y * (size_t)width * 4u;
		for ( x = 0; x < width; x++ ) {
			row[4u * x] = source[4u * x + 2u];
			row[4u * x + 1u] = source[4u * x + 1u];
			row[4u * x + 2u] = source[4u * x];
			row[4u * x + 3u] = source[4u * x + 3u];
		}
	}
	GlobalUnlock(memory);
	xgeImageFree(&image);
	return memory;
}

static UINT __xgeWin32ClipboardFormat(const char* sFormat)
{
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_TEXT_UTF8) == 0 ) return CF_UNICODETEXT;
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_HTML) == 0 ) return RegisterClipboardFormatA("HTML Format");
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_IMAGE_PNG) == 0 ) return RegisterClipboardFormatA("PNG");
	return RegisterClipboardFormatA(sFormat);
}

/* All representation allocation precedes the one OLE publication. A prepared
 * object never calls a lazy provider, and GetData returns only independent
 * HGLOBAL media (pUnkForRelease=NULL), as required by transfer protocol v1. */
static int __xgeWin32ClipboardSetItems(const xge_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    size_t slots=(size_t)count+1u;
    xge_win32_data_object_t* native=xrtCalloc(1,sizeof(*native)+slots*(sizeof(FORMATETC)+sizeof(HGLOBAL)));
    if(!native)return XGE_ERROR_OUT_OF_MEMORY;
    native->iface.lpVtbl=(IDataObjectVtbl*)&g_xgeWin32DataVtbl;native->iRefs=1;native->iCreator=xrtThreadCurrentId();
    native->pFormats=(FORMATETC*)(native+1);native->pPrepared=(HGLOBAL*)(native->pFormats+slots);
    int status=XGE_OK,png=-1;bool localOle=false;IDataObject* data=&native->iface;
    for(int i=0;status==XGE_OK && i<count;++i){
        UINT format=__xgeWin32ClipboardFormat(items[i].sFormat);
        if(!format){status=XGE_ERROR_BACKEND_FAILED;break;}
        for(int j=0;j<i;++j)if(native->pFormats[j].cfFormat==format)status=XGE_ERROR_INVALID_ARGUMENT;
        if(status!=XGE_OK)break;
        HGLOBAL memory;
        if(strcmp(items[i].sFormat,XGE_CLIPBOARD_FORMAT_TEXT_UTF8)==0){
            if((items[i].iDataSize && memchr(items[i].pData,0,items[i].iDataSize)) || items[i].iDataSize>INT_MAX){status=XGE_ERROR_INVALID_ARGUMENT;break;}
            memory=__xgeWin32ClipboardUtf8Text(items[i].pData,items[i].iDataSize);
        }else if(strcmp(items[i].sFormat,XGE_CLIPBOARD_FORMAT_HTML)==0)
            memory=__xgeWin32ClipboardHtml(items[i].pData,items[i].iDataSize);
        else memory=__xgeWin32ClipboardMemory(items[i].pData,items[i].iDataSize,0);
        if(!memory){status=XGE_ERROR_OUT_OF_MEMORY;break;}
        FORMATETC* target=&native->pFormats[native->iCount];target->cfFormat=(CLIPFORMAT)format;
        target->dwAspect=DVASPECT_CONTENT;target->lindex=-1;target->tymed=TYMED_HGLOBAL;
        native->pPrepared[native->iCount++]=memory;
        if(strcmp(items[i].sFormat,XGE_CLIPBOARD_FORMAT_IMAGE_PNG)==0)png=i;
    }
    if(status==XGE_OK && png>=0){
        HGLOBAL dib=__xgeWin32ClipboardPngDibV5(items[png].pData,items[png].iDataSize);
        if(dib){FORMATETC* target=&native->pFormats[native->iCount];target->cfFormat=CF_DIBV5;
            target->dwAspect=DVASPECT_CONTENT;target->lindex=-1;target->tymed=TYMED_HGLOBAL;
            native->pPrepared[native->iCount++]=dib;}
    }
    if(status==XGE_OK && !g_xgeWin32OleInitialized){
        HRESULT hr=OleInitialize(NULL);if(SUCCEEDED(hr))localOle=true;
        else status=XGE_ERROR_BACKEND_FAILED;
    }
    if(status==XGE_OK && services){
        xn_transfer_native_owner_v1 owner=__xgeWin32DataOwner(native);void* wrapped=NULL;
        int wrappedStatus=services->wrapDataObject(services,data,&owner,&wrapped);
        if(wrappedStatus==0 && wrapped)data=(IDataObject*)wrapped;
        if(wrappedStatus!=0 || !wrapped || xrtGetError())status=XGE_ERROR_BACKEND_FAILED;
    }
    if(status==XGE_OK){
        if(FAILED(OleSetClipboard(data)))status=XGE_ERROR_BACKEND_FAILED;
        else {
            __xgeWin32ClipboardDibPngCacheClear();(void)__xgeWin32ClipboardCacheInvalidate();
            if(localOle && FAILED(OleFlushClipboard()))status=XGE_ERROR_BACKEND_FAILED;
        }
    }
    xerror* first=status!=XGE_OK ? xrtTakeError():NULL;int outcome=status;
    data->lpVtbl->Release(data);__xgeCallbackOutcome(XGE_OK,&outcome,&first);
    if(localOle){OleUninitialize();__xgeCallbackOutcome(XGE_OK,&outcome,&first);}
    xrtSetErrorTake(first);return outcome;
}

typedef struct xge_win32_dib_channel_t {
	DWORD mask;
	DWORD scale;
	unsigned shift;
} xge_win32_dib_channel_t;

static int __xgeWin32DibPrepareChannel(DWORD mask, xge_win32_dib_channel_t* channel)
{
	unsigned shift = 0;
	if ( mask == 0u ) return 0;
	channel->mask = mask;
	while ( (mask & 1u) == 0u ) { mask >>= 1; shift++; }
	if ( (mask & (mask + 1u)) != 0u ) return 0;
	channel->scale = mask;
	channel->shift = shift;
	return 1;
}

static unsigned char __xgeWin32DibChannel(DWORD pixel, const xge_win32_dib_channel_t* channel)
{
	return (unsigned char)((((uint64_t)((pixel & channel->mask) >> channel->shift) * 255u) +
		(uint64_t)channel->scale / 2u) / (uint64_t)channel->scale);
}

/* Clipboard DIBs have no BMP file header. Decode the common uncompressed
 * 24/32-bit layouts here so XUI can continue to consume portable PNG bytes. */
static int __xgeWin32DibToPng(const unsigned char* dib, size_t dib_size,
	void** png, size_t* png_size)
{
	BITMAPINFOHEADER header;
	DWORD red = 0x00ff0000u, green = 0x0000ff00u, blue = 0x000000ffu, alpha = 0u;
	xge_win32_dib_channel_t channels[4] = {0};
	uint64_t height, pixels_count, stride, raster_bytes;
	size_t offset, extra_masks = 0u;
	unsigned char* rgba;
	int x, y, result;
	if ( dib_size < sizeof(header) ) return XGE_ERROR_RESOURCE_FAILED;
	memcpy(&header, dib, sizeof(header));
	if ( header.biSize < sizeof(header) || header.biSize > dib_size ||
	     header.biWidth <= 0 || header.biHeight == 0 || header.biHeight == LONG_MIN ||
	     header.biPlanes != 1 ||
	     (header.biBitCount != 24 && header.biBitCount != 32) ||
	     (header.biCompression != BI_RGB && header.biCompression != BI_BITFIELDS) )
		return XGE_ERROR_RESOURCE_FAILED;
	if ( header.biBitCount == 24 && header.biCompression != BI_RGB )
		return XGE_ERROR_RESOURCE_FAILED;
	height = header.biHeight < 0 ? (uint64_t)-(int64_t)header.biHeight :
		(uint64_t)header.biHeight;
	pixels_count = (uint64_t)header.biWidth * height;
	if ( pixels_count > UINT64_C(16000000) ||
	     (uint64_t)header.biWidth > (uint64_t)INT_MAX / 4u )
		return XGE_ERROR_BUFFER_TOO_SMALL;
	stride = (((uint64_t)header.biWidth * header.biBitCount + 31u) / 32u) * 4u;
	raster_bytes = stride * height;
	offset = (size_t)header.biSize;
	if ( header.biCompression == BI_BITFIELDS ) {
		if ( header.biSize >= 52u ) {
			memcpy(&red, dib + 40, sizeof(red));
			memcpy(&green, dib + 44, sizeof(green));
			memcpy(&blue, dib + 48, sizeof(blue));
			if ( header.biSize >= 56u ) memcpy(&alpha, dib + 52, sizeof(alpha));
		} else {
			extra_masks = 3u * sizeof(DWORD);
			if ( dib_size - offset < extra_masks ) return XGE_ERROR_RESOURCE_FAILED;
			memcpy(&red, dib + offset, sizeof(red));
			memcpy(&green, dib + offset + 4u, sizeof(green));
			memcpy(&blue, dib + offset + 8u, sizeof(blue));
		}
		if ( !red || !green || !blue || ((red & green) | (red & blue) |
		     (green & blue) | (alpha & (red | green | blue))) != 0u )
			return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( header.biBitCount == 32 &&
	     (!__xgeWin32DibPrepareChannel(red, &channels[0]) ||
	      !__xgeWin32DibPrepareChannel(green, &channels[1]) ||
	      !__xgeWin32DibPrepareChannel(blue, &channels[2]) ||
	      (alpha && !__xgeWin32DibPrepareChannel(alpha, &channels[3]))) )
		return XGE_ERROR_RESOURCE_FAILED;
	if ( header.biClrUsed > 256u || extra_masks > dib_size - offset )
		return XGE_ERROR_RESOURCE_FAILED;
	offset += extra_masks;
	if ( (uint64_t)header.biClrUsed * sizeof(RGBQUAD) > dib_size - offset )
		return XGE_ERROR_RESOURCE_FAILED;
	offset += (size_t)header.biClrUsed * sizeof(RGBQUAD);
	if ( raster_bytes > dib_size - offset ||
	     (header.biSizeImage != 0u && header.biSizeImage < raster_bytes) )
		return XGE_ERROR_RESOURCE_FAILED;
	rgba = (unsigned char*)xrtMalloc((size_t)pixels_count * 4u);
	if ( rgba == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	for ( y = 0; y < (int)height; y++ ) {
		const unsigned char* row = dib + offset + (size_t)(header.biHeight < 0 ?
			y : (int)height - 1 - y) * (size_t)stride;
		unsigned char* target = rgba + (size_t)y * (size_t)header.biWidth * 4u;
		for ( x = 0; x < header.biWidth; x++ ) {
			if ( header.biBitCount == 24 ) {
				target[4u * x] = row[3u * x + 2u];
				target[4u * x + 1u] = row[3u * x + 1u];
				target[4u * x + 2u] = row[3u * x];
				target[4u * x + 3u] = 255u;
			} else {
				DWORD value;
				memcpy(&value, row + 4u * x, sizeof(value));
				target[4u * x] = __xgeWin32DibChannel(value, &channels[0]);
				target[4u * x + 1u] = __xgeWin32DibChannel(value, &channels[1]);
				target[4u * x + 2u] = __xgeWin32DibChannel(value, &channels[2]);
				target[4u * x + 3u] = alpha ?
					__xgeWin32DibChannel(value, &channels[3]) : 255u;
			}
		}
	}
	result = xgeImageEncodePNG(header.biWidth, (int)height, rgba,
		header.biWidth * 4, png, png_size);
	xrtFree(rgba);
	return result;
}

static int __xgeWin32ClipboardGetPngFromDib(void* data, size_t capacity)
{
	UINT format;
	HANDLE native;
	const unsigned char* source;
	unsigned char* copy;
	SIZE_T bytes;
	void* png = NULL;
	size_t png_size = 0u;
	DWORD sequence = GetClipboardSequenceNumber();
	int result;
	if ( __xgeWin32ClipboardDibPngCacheRead(data, capacity, &result) ) return result;
	format = IsClipboardFormatAvailable(CF_DIBV5) ? CF_DIBV5 :
		(IsClipboardFormatAvailable(CF_DIB) ? CF_DIB : 0u);
	if ( format == 0u ) return XGE_ERROR_FILE_NOT_FOUND;
	if ( !__xgeWin32ClipboardOpen() ) return XGE_ERROR_BACKEND_FAILED;
	sequence = GetClipboardSequenceNumber();
	native = GetClipboardData(format);
	if ( native == NULL && format == CF_DIBV5 && IsClipboardFormatAvailable(CF_DIB) )
		native = GetClipboardData(CF_DIB);
	if ( native == NULL ) { CloseClipboard(); return XGE_ERROR_FILE_NOT_FOUND; }
	bytes = GlobalSize(native);
	if ( bytes < sizeof(BITMAPINFOHEADER) || bytes > (SIZE_T)128u * 1024u * 1024u ) {
		CloseClipboard(); return XGE_ERROR_RESOURCE_FAILED;
	}
	source = (const unsigned char*)GlobalLock(native);
	if ( source == NULL ) { CloseClipboard(); return XGE_ERROR_BACKEND_FAILED; }
	copy = (unsigned char*)xrtMalloc((size_t)bytes);
	if ( copy != NULL ) memcpy(copy, source, (size_t)bytes);
	GlobalUnlock(native); CloseClipboard();
	if ( copy == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	result = __xgeWin32DibToPng(copy, (size_t)bytes, &png, &png_size);
	xrtFree(copy);
	if ( result != XGE_OK ) return result;
	if ( png_size > (size_t)INT_MAX ) result = XGE_ERROR_BUFFER_TOO_SMALL;
	else {
		result = (int)png_size;
		if ( data != NULL && capacity > 0u ) memcpy(data, png,
			capacity < png_size ? capacity : png_size);
		if ( (data == NULL || capacity < png_size) && sequence != 0u &&
		     png_size <= (size_t)16u * 1024u * 1024u &&
		     GetClipboardSequenceNumber() == sequence ) {
			g_xgeWin32ClipboardDibPngCache.png = (unsigned char*)png;
			g_xgeWin32ClipboardDibPngCache.size = png_size;
			g_xgeWin32ClipboardDibPngCache.sequence = sequence;
			png = NULL;
		}
	}
	xrtFree(png);
	return result;
}

static int __xgeWin32ClipboardGetData(const char* sFormat, void* pData, size_t iCapacity)
{
	UINT iFormat;
	HANDLE hData;
	const unsigned char* pSource;
	SIZE_T iSize;
	size_t iOffset = 0u;
	size_t iEnd;
	int iResult;
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_TEXT_UTF8) == 0 ) {
		const char* sText = __xgeWin32ClipboardGetText();
		size_t iTextSize;
		/* The result-returning API must preserve read failure. The legacy
		 * pointer API still exposes an empty string for its callers. */
		if ( sText == NULL ) return XGE_ERROR_BACKEND_FAILED;
		iTextSize = strlen(sText);
		if ( iTextSize > (size_t)INT_MAX ) return XGE_ERROR_BUFFER_TOO_SMALL;
		if ( pData != NULL && iCapacity > 0u ) memcpy(pData, sText, iCapacity < iTextSize ? iCapacity : iTextSize);
		return (int)iTextSize;
	}
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_IMAGE_PNG) == 0 &&
	     __xgeWin32ClipboardDibPngCacheRead(pData, iCapacity, &iResult) ) return iResult;
	iFormat = __xgeWin32ClipboardFormat(sFormat);
	if ( iFormat == 0u ) return XGE_ERROR_FILE_NOT_FOUND;
	if ( !IsClipboardFormatAvailable(iFormat) ) {
		if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_IMAGE_PNG) == 0 )
			return __xgeWin32ClipboardGetPngFromDib(pData, iCapacity);
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( !__xgeWin32ClipboardOpen() ) return XGE_ERROR_BACKEND_FAILED;
	hData = GetClipboardData(iFormat);
	if ( hData == NULL ) {
		CloseClipboard();
		return strcmp(sFormat, XGE_CLIPBOARD_FORMAT_IMAGE_PNG) == 0 ?
			__xgeWin32ClipboardGetPngFromDib(pData, iCapacity) : XGE_ERROR_FILE_NOT_FOUND;
	}
	iSize = GlobalSize(hData);
	if(iSize==0u){CloseClipboard();return 0;}
	pSource = (const unsigned char*)GlobalLock(hData);
	if ( pSource == NULL ) { CloseClipboard(); return XGE_ERROR_BACKEND_FAILED; }
	iEnd = (size_t)iSize;
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_HTML) == 0 ) {
		static const char sStartLabel[] = "StartFragment:";
		static const char sEndLabel[] = "EndFragment:";
		const unsigned char* pStart = __xgeWin32ClipboardFind(pSource, (size_t)iSize, sStartLabel);
		const unsigned char* pFinish = __xgeWin32ClipboardFind(pSource, (size_t)iSize, sEndLabel);
		size_t iParsedStart;
		size_t iParsedEnd;
		if ( pStart == NULL || pFinish == NULL ||
		     !__xgeWin32ClipboardParseOffset(pStart + sizeof(sStartLabel) - 1u, pSource + iSize, &iParsedStart) ||
		     !__xgeWin32ClipboardParseOffset(pFinish + sizeof(sEndLabel) - 1u, pSource + iSize, &iParsedEnd) ||
		     iParsedStart > iParsedEnd || iParsedEnd > (size_t)iSize ) {
			GlobalUnlock(hData); CloseClipboard(); return XGE_ERROR_RESOURCE_FAILED;
		}
		iOffset = iParsedStart;
		iEnd = iParsedEnd;
	}
	if ( iEnd - iOffset > (size_t)INT_MAX ) iResult = XGE_ERROR_BUFFER_TOO_SMALL;
	else {
		iResult = (int)(iEnd - iOffset);
		if ( pData != NULL && iCapacity > 0u ) memcpy(pData, pSource + iOffset,
			iCapacity < (size_t)iResult ? iCapacity : (size_t)iResult);
	}
	GlobalUnlock(hData);
	CloseClipboard();
	return iResult;
}
#endif

#if defined(_SAPP_LINUX)
/* Keep Sokol's selection owner/event loop, but grow its text storage instead
 * of accepting a successful write after the fixed buffer truncated it. */
static int __xgeX11ClipboardReserve(size_t bytes)
{
	char* buffer;
	if ( bytes >= (size_t)INT_MAX ) return XGE_ERROR_BUFFER_TOO_SMALL;
	if ( !_sapp.clipboard.enabled || _sapp.clipboard.buffer == NULL ) return XGE_ERROR_UNSUPPORTED;
	if ( bytes < (size_t)_sapp.clipboard.buf_size ) return XGE_OK;
	buffer = (char*)xrtMalloc(bytes + 1u);
	if ( buffer == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	memcpy(buffer, _sapp.clipboard.buffer, (size_t)_sapp.clipboard.buf_size);
	_sapp_free(_sapp.clipboard.buffer);
	_sapp.clipboard.buffer = buffer;
	_sapp.clipboard.buf_size = (int)bytes + 1;
	return XGE_OK;
}

typedef struct xge_x11_clipboard_item_t {Atom target;const unsigned char* data;size_t bytes;} xge_x11_clipboard_item_t;
typedef struct xge_x11_clipboard_snapshot_t {int count;xge_x11_clipboard_item_t items[65];} xge_x11_clipboard_snapshot_t;
static xge_x11_clipboard_snapshot_t* g_xgeX11Clipboard;
static bool __xgeX11ClipboardSelectionRequest(void* raw)
{
    XSelectionRequestEvent* request=&((XEvent*)raw)->xselectionrequest;
    xge_x11_clipboard_snapshot_t* snapshot=g_xgeX11Clipboard;
    if(!snapshot || request->selection!=_sapp.x11.CLIPBOARD || !_sapp.clipboard.enabled)return false;
    XSelectionEvent reply={0};reply.type=SelectionNotify;reply.display=request->display;
    reply.requestor=request->requestor;reply.selection=request->selection;reply.target=request->target;reply.time=request->time;
    Atom property=request->property!=None ? request->property:request->target;
    if(request->owner==_sapp.x11.window && XGetSelectionOwner(_sapp.x11.display,_sapp.x11.CLIPBOARD)==_sapp.x11.window){
        if(request->target==_sapp.x11.TARGETS){
            Atom targets[66];targets[0]=_sapp.x11.TARGETS;
            for(int i=0;i<snapshot->count;++i)targets[i+1]=snapshot->items[i].target;
            XChangeProperty(_sapp.x11.display,request->requestor,property,XA_ATOM,32,PropModeReplace,(unsigned char*)targets,snapshot->count+1);
            reply.property=property;
        }else for(int i=0;i<snapshot->count;++i)if(snapshot->items[i].target==request->target){
            XChangeProperty(_sapp.x11.display,request->requestor,property,request->target,8,PropModeReplace,snapshot->items[i].data,(int)snapshot->items[i].bytes);
            reply.property=property;break;
        }
    }
    XSendEvent(_sapp.x11.display,request->requestor,False,0,(XEvent*)&reply);XFlush(_sapp.x11.display);return true;
}
static int __xgeX11ClipboardSetItems(const xge_clipboard_item_t* items,int count)
{
    if(!_sapp.clipboard.enabled || !_sapp.clipboard.buffer || !_sapp.x11.display)return XGE_ERROR_UNSUPPORTED;
    long words=XExtendedMaxRequestSize(_sapp.x11.display);if(!words)words=XMaxRequestSize(_sapp.x11.display);
    if(words<=8)return XGE_ERROR_BUFFER_TOO_SMALL;
    size_t total=sizeof(xge_x11_clipboard_snapshot_t);int text=-1;
    for(int i=0;i<count;++i){
        if(items[i].iDataSize>INT_MAX || items[i].iDataSize>(size_t)(words-8)*4u || items[i].iDataSize>SIZE_MAX-total)return XGE_ERROR_BUFFER_TOO_SMALL;
        total+=items[i].iDataSize;
        if(strcmp(items[i].sFormat,XGE_CLIPBOARD_FORMAT_TEXT_UTF8)==0){
            text=i;if(items[i].iDataSize && memchr(items[i].pData,0,items[i].iDataSize))return XGE_ERROR_INVALID_ARGUMENT;
        }
    }
    xge_x11_clipboard_snapshot_t* next=xrtCalloc(1,total);if(!next)return XGE_ERROR_OUT_OF_MEMORY;
    unsigned char* output=(unsigned char*)(next+1);int status=XGE_OK;
    for(int i=0;i<count;++i){
        Atom atom=XInternAtom(_sapp.x11.display,items[i].sFormat,False);
        if(atom==None){status=XGE_ERROR_BACKEND_FAILED;break;}
        for(int j=0;j<next->count;++j)if(next->items[j].target==atom)status=XGE_ERROR_INVALID_ARGUMENT;
        if(status!=XGE_OK)break;
        next->items[next->count++]=(xge_x11_clipboard_item_t){atom,output,items[i].iDataSize};
        if(items[i].iDataSize)memcpy(output,items[i].pData,items[i].iDataSize);
        if(i==text && atom!=_sapp.x11.UTF8_STRING)next->items[next->count++]=(xge_x11_clipboard_item_t){_sapp.x11.UTF8_STRING,output,items[i].iDataSize};
        output+=items[i].iDataSize;
    }
    if(status==XGE_OK)status=__xgeX11ClipboardReserve(text>=0 ? items[text].iDataSize:0u);
    if(status==XGE_OK){
        XSetSelectionOwner(_sapp.x11.display,_sapp.x11.CLIPBOARD,_sapp.x11.window,CurrentTime);
        if(XGetSelectionOwner(_sapp.x11.display,_sapp.x11.CLIPBOARD)!=_sapp.x11.window)status=XGE_ERROR_BACKEND_FAILED;
    }
    if(status!=XGE_OK){xrtFree(next);return status;}
    xge_x11_clipboard_snapshot_t* old=g_xgeX11Clipboard;g_xgeX11Clipboard=next;
    if(text>=0 && items[text].iDataSize)memcpy(_sapp.clipboard.buffer,items[text].pData,items[text].iDataSize);
    _sapp.clipboard.buffer[text>=0 ? items[text].iDataSize:0u]=0;xrtFree(old);return XGE_OK;
}
static int __xgeX11ClipboardGetData(const char* format,void* output,size_t capacity)
{
    if(!_sapp.clipboard.enabled || !_sapp.x11.display)return XGE_ERROR_UNSUPPORTED;
    Atom target=XInternAtom(_sapp.x11.display,format,False);
    Window owner=XGetSelectionOwner(_sapp.x11.display,_sapp.x11.CLIPBOARD);
    if(owner==None)return XGE_ERROR_NOT_FOUND;
    if(owner==_sapp.x11.window && g_xgeX11Clipboard){
        for(int i=0;i<g_xgeX11Clipboard->count;++i)if(g_xgeX11Clipboard->items[i].target==target){
            const xge_x11_clipboard_item_t* item=&g_xgeX11Clipboard->items[i];
            if(output && capacity)memcpy(output,item->data,capacity<item->bytes ? capacity:item->bytes);
            return (int)item->bytes;
        }
        return XGE_ERROR_NOT_FOUND;
    }
    Atom property=XInternAtom(_sapp.x11.display,"XGE_SELECTION",False),actual;XEvent event;
    XConvertSelection(_sapp.x11.display,_sapp.x11.CLIPBOARD,target,property,_sapp.x11.window,CurrentTime);
    if(!_sapp_x11_wait_for_event(SelectionNotify,0.1,&event) || event.xselection.requestor!=_sapp.x11.window || event.xselection.selection!=_sapp.x11.CLIPBOARD || event.xselection.target!=target || event.xselection.property!=property)return XGE_ERROR_BACKEND_FAILED;
    int bits;unsigned long count,remaining;unsigned char* data=NULL;
    if(XGetWindowProperty(_sapp.x11.display,_sapp.x11.window,property,0,LONG_MAX,True,AnyPropertyType,&actual,&bits,&count,&remaining,&data)!=Success){if(data)XFree(data);return XGE_ERROR_BACKEND_FAILED;}
    int result=XGE_ERROR_BACKEND_FAILED;
    if(actual==target && bits==8 && !remaining && count<=INT_MAX && (!count || data)){
        if(output && capacity)memcpy(output,data,capacity<(size_t)count ? capacity:(size_t)count);
        result=(int)count;
    }
    if(data)XFree(data);
    return result;
}

static int __xgeX11ClipboardGetText(const char** text)
{
	Window owner;
	Atom property, actual_type;
	XEvent event;
	int actual_format, result;
	unsigned long bytes, remaining;
	unsigned char* data = NULL;
	if ( !_sapp.clipboard.enabled || _sapp.clipboard.buffer == NULL ) return XGE_ERROR_UNSUPPORTED;
	owner = XGetSelectionOwner(_sapp.x11.display, _sapp.x11.CLIPBOARD);
	if ( owner == None ) { *text = ""; return 0; }
	if ( owner == _sapp.x11.window ) {
		*text = _sapp.clipboard.buffer;
		return (int)strlen(*text);
	}
	property = XInternAtom(_sapp.x11.display, "SAPP_SELECTION", False);
	XConvertSelection(_sapp.x11.display, _sapp.x11.CLIPBOARD, _sapp.x11.UTF8_STRING,
		property, _sapp.x11.window, CurrentTime);
	if ( !_sapp_x11_wait_for_event(SelectionNotify, 0.1, &event) ||
		event.xselection.requestor != _sapp.x11.window ||
		event.xselection.selection != _sapp.x11.CLIPBOARD ||
		event.xselection.target != _sapp.x11.UTF8_STRING || event.xselection.property != property )
		return XGE_ERROR_BACKEND_FAILED;
	if ( XGetWindowProperty(_sapp.x11.display, _sapp.x11.window, property, 0, LONG_MAX,
		True, AnyPropertyType, &actual_type, &actual_format, &bytes, &remaining, &data) != Success ) {
		if ( data != NULL ) XFree(data);
		return XGE_ERROR_BACKEND_FAILED;
	}
	/* Incremental transfers and unsupported types must fail before a caller
 * changes its document; they must not be mistaken for cached/empty text. */
	if ( actual_type != _sapp.x11.UTF8_STRING || actual_format != 8 || remaining ||
		(bytes && (data == NULL || memchr(data, 0, (size_t)bytes))) ) {
		if ( data != NULL ) XFree(data);
		return XGE_ERROR_BACKEND_FAILED;
	}
	result = __xgeX11ClipboardReserve((size_t)bytes);
	if ( result == XGE_OK ) {
		if ( bytes ) memcpy(_sapp.clipboard.buffer, data, (size_t)bytes);
		_sapp.clipboard.buffer[bytes] = 0;
		*text = _sapp.clipboard.buffer;
		result = (int)bytes;
	}
	if ( data != NULL ) XFree(data);
	return result;
}
#endif

static void __xgeClipboardPlatformCleanup(void)
{
#if defined(_SAPP_LINUX)
    xrtFree(g_xgeX11Clipboard);g_xgeX11Clipboard=NULL;
#endif
}

static int __xgeClipboardSetItems(const xge_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    if(!items || count<=0 || count>64)return XGE_ERROR_INVALID_ARGUMENT;
    if(services){int status=__xgeTransferValidate(services);if(status!=XGE_OK)return status;}
    for(int i=0;i<count;++i){
        if(!items[i].sFormat || !items[i].sFormat[0] || (!items[i].pData && items[i].iDataSize) || items[i].iDataSize==SIZE_MAX)return XGE_ERROR_INVALID_ARGUMENT;
        for(int j=0;j<i;++j)if(strcmp(items[i].sFormat,items[j].sFormat)==0)return XGE_ERROR_INVALID_ARGUMENT;
    }
#if defined(_WIN32) || defined(_WIN64)
    if(g_xge.bInitialized || services)return __xgeWin32ClipboardSetItems(items,count,services);
#endif
#if defined(_SAPP_LINUX)
    if(g_xge.bSokolRunning)return __xgeX11ClipboardSetItems(items,count);
#else
    /* Other backends must reject the entire set before changing their old
     * clipboard. A UTF8-only backend cannot silently discard rich formats. */
    if(count!=1 || strcmp(items[0].sFormat,XGE_CLIPBOARD_FORMAT_TEXT_UTF8)!=0)return XGE_ERROR_UNSUPPORTED;
    if(g_xge.bSokolRunning){
        if(items[0].iDataSize>=INT_MAX || (items[0].iDataSize && memchr(items[0].pData,0,items[0].iDataSize)))return XGE_ERROR_INVALID_ARGUMENT;
        char* text=xrtMalloc(items[0].iDataSize+1u);if(!text)return XGE_ERROR_OUT_OF_MEMORY;
        if(items[0].iDataSize)memcpy(text,items[0].pData,items[0].iDataSize);
        text[items[0].iDataSize]=0;
        int status=XGE_ERROR_UNSUPPORTED;
#if defined(_SAPP_MACOS) || defined(_SAPP_WIN32) || defined(_SAPP_EMSCRIPTEN)
        if(!_sapp.clipboard.enabled)status=XGE_ERROR_UNSUPPORTED;
        else if(items[0].iDataSize>=(size_t)_sapp.clipboard.buf_size)status=XGE_ERROR_BUFFER_TOO_SMALL;
        else {sapp_set_clipboard_string(text);status=XGE_OK;}
#endif
        xrtFree(text);return status;
    }
#endif
    return XGE_ERROR_UNSUPPORTED;
}
int xgeClipboardSetItems(const xge_clipboard_item_t* items,int count)
{
    xerror* ambient=xrtTakeError();xn_transfer_services_v1 services;
    int status=__xgeClipboardSetItems(items,count,__xgeTransferCurrent(&services) ? &services:NULL);
    xge_data_outcome_t outcome={0};__xgeDataOwnedOutcomeBegin(&outcome,status);
    return __xgeDataOwnedOutcomeEnd(&outcome,ambient);
}
int xgeClipboardSetItemsWithTransfer(const xge_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    xerror* ambient=xrtTakeError();int status=__xgeTransferValidate(services);
    if(status==XGE_OK)status=__xgeClipboardSetItems(items,count,services);
    xge_data_outcome_t outcome={0};__xgeDataOwnedOutcomeBegin(&outcome,status);
    return __xgeDataOwnedOutcomeEnd(&outcome,ambient);
}
static int __xgeDataClipboardConsume(const xge_clipboard_item_t* items,int count,void* services)
{return xgeClipboardSetItemsWithTransfer(items,count,services);}
int xgeDataObjectCopyToClipboardWithTransfer(xge_data_object data,const xn_transfer_services_v1* services)
{int status=__xgeTransferValidate(services);return status==XGE_OK ? __xgeDataSnapshotCopy(data,__xgeDataClipboardConsume,(void*)services):status;}

int xgeClipboardGetData(const char* sFormat, void* pData, size_t iCapacity)
{
	if ( sFormat == NULL || sFormat[0] == 0 || (pData == NULL && iCapacity != 0u) ) return XGE_ERROR_INVALID_ARGUMENT;
	#if defined(_WIN32) || defined(_WIN64)
		if ( g_xge.bInitialized ) return __xgeWin32ClipboardGetData(sFormat, pData, iCapacity);
	#endif
#if defined(_SAPP_LINUX)
    if(g_xge.bSokolRunning)return __xgeX11ClipboardGetData(sFormat,pData,iCapacity);
#endif
	if ( strcmp(sFormat, XGE_CLIPBOARD_FORMAT_TEXT_UTF8) == 0 && g_xge.bSokolRunning ) {
		const char* sText;
		size_t iSize;
		#if defined(_SAPP_LINUX)
			int result = __xgeX11ClipboardGetText(&sText);
			if ( result < 0 ) return result;
			iSize = (size_t)result;
		#else
			sText = sapp_get_clipboard_string();
			if ( sText == NULL ) return XGE_ERROR_BACKEND_FAILED;
			iSize = strlen(sText);
		#endif
		if ( iSize > (size_t)INT_MAX ) return XGE_ERROR_BUFFER_TOO_SMALL;
		if ( pData != NULL && iCapacity > 0u ) memcpy(pData, sText, iCapacity < iSize ? iCapacity : iSize);
		return (int)iSize;
	}
	return XGE_ERROR_UNSUPPORTED;
}

void xgeClipboardSetText(const char* sText)
{
	xge_clipboard_item_t tItem;
	if ( sText == NULL ) {
		sText = "";
	}
	tItem.sFormat = XGE_CLIPBOARD_FORMAT_TEXT_UTF8;
	tItem.pData = sText;
	tItem.iDataSize = strlen(sText);
	if ( xgeClipboardSetItems(&tItem, 1) == XGE_OK ) return;
	#if defined(_WIN32) || defined(_WIN64)
		if ( g_xge.bInitialized ) {
			(void)__xgeWin32ClipboardSetText(sText);
			return;
		}
	#endif
	/* The result API already attempted the supported backend. Retrying through
	 * Sokol's void API would hide failure and can truncate the old clipboard. */
}

const char* xgeClipboardGetText(void)
{
	#if defined(_WIN32) || defined(_WIN64)
		const char* sText;
		if ( g_xge.bInitialized ) {
			sText = __xgeWin32ClipboardGetText();
			return (sText != NULL) ? sText : "";
		}
	#endif
	if ( g_xge.bSokolRunning == 0 ) {
		return "";
	}
	#if defined(_SAPP_LINUX)
		const char* text;
		return __xgeX11ClipboardGetText(&text) >= 0 ? text : "";
	#else
		const char* text = sapp_get_clipboard_string();
		return text != NULL ? text : "";
	#endif
}
