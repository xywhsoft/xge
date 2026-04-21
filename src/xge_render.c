int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels)
{
	GLuint iTexture;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	memset(pTexture, 0, sizeof(*pTexture));
	glGenTextures(1, &iTexture);
	glBindTexture(GL_TEXTURE_2D, iTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	pTexture->iWidth = iWidth;
	pTexture->iHeight = iHeight;
	pTexture->iFormat = XGE_PIXEL_RGBA8;
	pTexture->iRefCount = 1;
	pTexture->iBackendId = iTexture;
	return XGE_OK;
}

int xgeImageLoad(xge_image pImage, const char* sPath)
{
	return xgeImageLoadEx(pImage, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pImage == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeImageLoadMemoryEx(pImage, tResource.pData, tResource.iSize, iFlags);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize)
{
	return xgeImageLoadMemoryEx(pImage, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags)
{
	int iWidth;
	int iHeight;
	int iChannels;
	unsigned char* pPixels;

	if ( (pImage == NULL) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	pPixels = stbi_load_from_memory((const stbi_uc*)pData, iSize, &iWidth, &iHeight, &iChannels, 4);
	if ( pPixels == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	memset(pImage, 0, sizeof(*pImage));
	pImage->iWidth = iWidth;
	pImage->iHeight = iHeight;
	pImage->iFormat = XGE_PIXEL_RGBA8;
	pImage->iStride = iWidth * 4;
	pImage->pPixels = pPixels;
	pImage->iFlags = ((iFlags & XGE_IMAGE_STRAIGHT_ALPHA) != 0) ? XGE_IMAGE_STRAIGHT_ALPHA : XGE_IMAGE_PREMULTIPLIED;
	if ( (pImage->iFlags & XGE_IMAGE_PREMULTIPLIED) != 0 ) {
		xgeImagePremultiply(pImage);
	}
	return XGE_OK;
}

void* xgeImageGetPixels(xge_image pImage)
{
	if ( pImage == NULL ) {
		return NULL;
	}
	return pImage->pPixels;
}

void xgeImagePremultiply(xge_image pImage)
{
	unsigned char* pPixels;
	int i;
	int iCount;
	unsigned int iA;

	if ( (pImage == NULL) || (pImage->pPixels == NULL) || (pImage->iFormat != XGE_PIXEL_RGBA8) ) {
		return;
	}
	pPixels = (unsigned char*)pImage->pPixels;
	iCount = pImage->iWidth * pImage->iHeight;
	for ( i = 0; i < iCount; i++ ) {
		iA = (unsigned int)pPixels[(i * 4) + 3];
		pPixels[(i * 4) + 0] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 0] * iA + 127u) / 255u);
		pPixels[(i * 4) + 1] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 1] * iA + 127u) / 255u);
		pPixels[(i * 4) + 2] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 2] * iA + 127u) / 255u);
	}
	pImage->iFlags &= ~XGE_IMAGE_STRAIGHT_ALPHA;
	pImage->iFlags |= XGE_IMAGE_PREMULTIPLIED;
}

void xgeImageFree(xge_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pPixels != NULL ) {
		stbi_image_free(pImage->pPixels);
	}
	memset(pImage, 0, sizeof(*pImage));
}

int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage)
{
	if ( (pTexture == NULL) || (pImage == NULL) || (pImage->pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeTextureCreateRGBA(pTexture, pImage->iWidth, pImage->iHeight, pImage->pPixels);
}

int xgeTextureLoad(xge_texture pTexture, const char* sPath)
{
	return xgeTextureLoadEx(pTexture, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	iRet = xgeImageLoadEx(&objImage, sPath, iFlags);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateFromImage(pTexture, &objImage);
	xgeImageFree(&objImage);
	return iRet;
}

int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize)
{
	return xgeTextureLoadMemoryEx(pTexture, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	iRet = xgeImageLoadMemoryEx(&objImage, pData, iSize, iFlags);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateFromImage(pTexture, &objImage);
	xgeImageFree(&objImage);
	return iRet;
}

int xgeTextureAddRef(xge_texture pTexture)
{
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iRefCount < INT32_MAX ) {
		pTexture->iRefCount++;
	}
	return pTexture->iRefCount;
}

void xgeTextureFree(xge_texture pTexture)
{
	GLuint iTexture;

	if ( pTexture == NULL ) {
		return;
	}
	if ( pTexture->iRefCount > 1 ) {
		pTexture->iRefCount--;
		return;
	}
	iTexture = (GLuint)pTexture->iBackendId;
	if ( iTexture != 0 ) {
		glDeleteTextures(1, &iTexture);
	}
	memset(pTexture, 0, sizeof(*pTexture));
}

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
}

static void __xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor, int bScreenSpace)
{
	float fR;
	float fG;
	float fB;
	float fA;
	float arrVertices[8];
	xge_vec2_t tPoint;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		return;
	}

	if ( bScreenSpace ) {
		arrVertices[0] = tRect.fX;
		arrVertices[1] = tRect.fY;
		arrVertices[2] = tRect.fX + tRect.fW;
		arrVertices[3] = tRect.fY;
		arrVertices[4] = tRect.fX;
		arrVertices[5] = tRect.fY + tRect.fH;
		arrVertices[6] = tRect.fX + tRect.fW;
		arrVertices[7] = tRect.fY + tRect.fH;
	} else {
		tPoint.fX = tRect.fX;
		tPoint.fY = tRect.fY;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[0] = tPoint.fX;
		arrVertices[1] = tPoint.fY;
		tPoint.fX = tRect.fX + tRect.fW;
		tPoint.fY = tRect.fY;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[2] = tPoint.fX;
		arrVertices[3] = tPoint.fY;
		tPoint.fX = tRect.fX;
		tPoint.fY = tRect.fY + tRect.fH;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[4] = tPoint.fX;
		arrVertices[5] = tPoint.fY;
		tPoint.fX = tRect.fX + tRect.fW;
		tPoint.fY = tRect.fY + tRect.fH;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[6] = tPoint.fX;
		arrVertices[7] = tPoint.fY;
	}

	__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, fR, fG, fB, fA);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor)
{
	__xgeShapeRectFill(tRect, iColor, 0);
}

void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor)
{
	__xgeShapeRectFill(tRect, iColor, 1);
}

void xgeClipSet(xge_rect_t tRect)
{
	GLint iX;
	GLint iY;
	GLsizei iW;
	GLsizei iH;

	if ( (g_xge.bSokolRunning == 0) || (glScissor == NULL) ) {
		return;
	}
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	g_xge.bClipEnabled = 1;
	g_xge.tClipRect = tRect;
	iX = (GLint)tRect.fX;
	iY = (GLint)((float)g_xge.iHeight - tRect.fY - tRect.fH);
	iW = (GLsizei)tRect.fW;
	iH = (GLsizei)tRect.fH;
	glEnable(GL_SCISSOR_TEST);
	glScissor(iX, iY, iW, iH);
}

void xgeClipClear(void)
{
	if ( (g_xge.bSokolRunning == 0) || (glDisable == NULL) ) {
		return;
	}
	g_xge.bClipEnabled = 0;
	memset(&g_xge.tClipRect, 0, sizeof(g_xge.tClipRect));
	glDisable(GL_SCISSOR_TEST);
}

