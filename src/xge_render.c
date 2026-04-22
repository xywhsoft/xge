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

static void __xgeDrawExImmediate(const xge_draw_t* pDraw)
{
	float fSrcX;
	float fSrcY;
	float fSrcW;
	float fSrcH;
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

	if ( (pDraw == NULL) || (pDraw->pTexture == NULL) || (pDraw->pTexture->iBackendId == 0) ) {
		return;
	}
	pTexture = pDraw->pTexture;
	if ( __xgeTextureRendererInit() != XGE_OK ) {
		return;
	}

	fSrcX = pDraw->tSrc.fX;
	fSrcY = pDraw->tSrc.fY;
	fSrcW = pDraw->tSrc.fW;
	fSrcH = pDraw->tSrc.fH;
	if ( fSrcW == 0.0f ) {
		fSrcW = (float)pTexture->iWidth;
	}
	if ( fSrcH == 0.0f ) {
		fSrcH = (float)pTexture->iHeight;
	}
	fU0 = fSrcX / (float)pTexture->iWidth;
	fV0 = fSrcY / (float)pTexture->iHeight;
	fU1 = (fSrcX + fSrcW) / (float)pTexture->iWidth;
	fV1 = (fSrcY + fSrcH) / (float)pTexture->iHeight;
	if ( (pDraw->iFlags & XGE_DRAW_FLIP_X) != 0 ) {
		float fTmp = fU0;
		fU0 = fU1;
		fU1 = fTmp;
	}
	if ( (pDraw->iFlags & XGE_DRAW_FLIP_Y) != 0 ) {
		float fTmp = fV0;
		fV0 = fV1;
		fV1 = fTmp;
	}

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

	glUseProgram(g_xgeTextureRenderer.iProgram);
	glUniform2f(g_xgeTextureRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform1i(g_xgeTextureRenderer.iLocTexture, 0);
	glUniform4f(g_xgeTextureRenderer.iLocColor, fR, fG, fB, fA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
	glBindVertexArray(g_xgeTextureRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeTextureRenderer.iVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}

void xgeViewportSet(xge_rect_t tRect)
{
	GLint iX;
	GLint iY;
	GLsizei iW;
	GLsizei iH;

	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	g_xge.bViewportEnabled = 1;
	g_xge.tViewportRect = tRect;
	if ( (g_xge.bSokolRunning == 0) || (glViewport == NULL) ) {
		return;
	}
	iX = (GLint)tRect.fX;
	iY = (GLint)((float)g_xge.iHeight - tRect.fY - tRect.fH);
	iW = (GLsizei)tRect.fW;
	iH = (GLsizei)tRect.fH;
	glViewport(iX, iY, iW, iH);
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
	xge_rect_t tRect;

	g_xge.bViewportEnabled = 0;
	memset(&g_xge.tViewportRect, 0, sizeof(g_xge.tViewportRect));
	tRect = xgeViewportGet();
	if ( (g_xge.bSokolRunning == 0) || (glViewport == NULL) ) {
		return;
	}
	glViewport((GLint)tRect.fX, (GLint)tRect.fY, (GLsizei)tRect.fW, (GLsizei)tRect.fH);
}

void xgeClipSet(xge_rect_t tRect)
{
	GLint iX;
	GLint iY;
	GLsizei iW;
	GLsizei iH;

	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	g_xge.bClipEnabled = 1;
	g_xge.tClipRect = tRect;
	if ( (g_xge.bSokolRunning == 0) || (glScissor == NULL) ) {
		return;
	}
	iX = (GLint)tRect.fX;
	iY = (GLint)((float)g_xge.iHeight - tRect.fY - tRect.fH);
	iW = (GLsizei)tRect.fW;
	iH = (GLsizei)tRect.fH;
	glEnable(GL_SCISSOR_TEST);
	glScissor(iX, iY, iW, iH);
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

void xgeClipClear(void)
{
	g_xge.bClipEnabled = 0;
	memset(&g_xge.tClipRect, 0, sizeof(g_xge.tClipRect));
	if ( (g_xge.bSokolRunning == 0) || (glDisable == NULL) ) {
		return;
	}
	glDisable(GL_SCISSOR_TEST);
}

