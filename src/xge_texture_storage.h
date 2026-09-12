/* Private texture storage helpers, included once by xge_texture.c.
 * CPU pixels always remain canonical RGBA8. No entropy codec, residency cache,
 * palette lookup, shader rewrite, or periodic recompression is involved. */
typedef struct xge_texture_choice_t {
	int iStorage;
	int iSwizzle;
} xge_texture_choice_t;

enum {
	XGE_SWIZZLE_IDENTITY,
	XGE_SWIZZLE_COVERAGE,
	XGE_SWIZZLE_GRAY,
	XGE_SWIZZLE_WHITE_ALPHA,
	XGE_SWIZZLE_BLACK_ALPHA,
	XGE_SWIZZLE_GRAY_ALPHA
};

static int __xgeTextureNativeSwizzle(void)
{
#if defined(__EMSCRIPTEN__)
	return 0;
#else
	int iBackend = xgeGraphicsBackendGet().iType;
	/* WebGL2 explicitly removes TEXTURE_SWIZZLE_*. Unknown/custom backends
	 * also keep the original sampler2D contract without assuming extensions. */
	return iBackend == XGE_GPU_BACKEND_OPENGL33 || iBackend == XGE_GPU_BACKEND_GLES30;
#endif
}

static int __xgeTextureChoices(xge_texture pTexture, xge_texture_choice_t* pChoices)
{
	xge_texture_shadow_t* pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	uint32_t iMode = pTexture->iFlags & XGE_TEXTURE_COMPRESS_MASK;
	size_t i, iCount = (size_t)pTexture->iWidth * (size_t)pTexture->iHeight;
	int bGray = 1, bCoverage = 1, bOpaque = 1, bBinary = 1, bWhite = 1, bBlack = 1;
	int iChoiceCount = 0;
	int bSwizzle = __xgeTextureNativeSwizzle();
	if ( iMode == XGE_TEXTURE_COMPRESS_NONE || pShadow->bPackingDisabled ||
	     iCount < 64u || glGetError == NULL || glGetIntegerv == NULL ||
	     glPixelStorei == NULL || (!bSwizzle && iMode != XGE_TEXTURE_COMPRESS_LOSSY) ) goto fallback;
	for ( i = 0; i < iCount; i++ ) {
		const unsigned char* p = pShadow->pPixels + i * 4u;
		bGray &= p[0] == p[1] && p[0] == p[2];
		bCoverage &= p[0] == p[3];
		bOpaque &= p[3] == 255;
		bBinary &= p[3] == 0 || p[3] == 255;
		bWhite &= p[0] == 255;
		bBlack &= p[0] == 0;
		/* An exact candidate must pass every pixel. Color images reject early. */
		if ( !bGray && (iMode != XGE_TEXTURE_COMPRESS_LOSSY || (!bOpaque && !bBinary)) ) break;
	}
	if ( iMode == XGE_TEXTURE_COMPRESS_LOSSY ) {
		if ( bOpaque ) pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RGB565, 0};
		if ( bBinary ) pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RGB5_A1, 0};
		pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RGBA4, 0};
	}
	if ( bSwizzle && bGray ) {
		int iSwizzle = bCoverage ? XGE_SWIZZLE_COVERAGE : bOpaque ? XGE_SWIZZLE_GRAY :
		               bWhite ? XGE_SWIZZLE_WHITE_ALPHA : bBlack ? XGE_SWIZZLE_BLACK_ALPHA : 0;
		if ( iSwizzle ) pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_R8, iSwizzle};
		pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RG8, XGE_SWIZZLE_GRAY_ALPHA};
	}
fallback:
	pChoices[iChoiceCount++] = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RGBA8, 0};
	return iChoiceCount;
}

static void __xgeTextureStorageCommit(xge_texture pTexture, xge_texture_choice_t tChoice)
{
	xge_texture_shadow_t* pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	uint64_t iBefore = __xgeTextureMemoryBytes(pTexture), iAfter;
	pShadow->iStorage = tChoice.iStorage;
	pShadow->iSwizzle = tChoice.iSwizzle;
	iAfter = __xgeTextureMemoryBytes(pTexture);
	if ( iAfter >= iBefore ) g_xge.iTextureMemoryBytes += iAfter - iBefore;
	else g_xge.iTextureMemoryBytes -= iBefore - iAfter;
}

static void* __xgeTexturePack(xge_texture_choice_t tChoice, const unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	int x, y, iBytes = tChoice.iStorage == XGE_TEXTURE_STORAGE_R8 ? 1 :
	                  tChoice.iStorage == XGE_TEXTURE_STORAGE_RGBA8 ? 4 : 2;
	unsigned char* pPacked = (unsigned char*)xrtMalloc((size_t)iWidth * (size_t)iHeight * (size_t)iBytes);
	if ( pPacked == NULL ) return NULL;
	for ( y = 0; y < iHeight; y++ ) {
		const unsigned char* p = pPixels + (size_t)y * (size_t)iStride;
		unsigned char* q = pPacked + (size_t)y * (size_t)iWidth * (size_t)iBytes;
		if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_RGBA8 ) {
			memcpy(q, p, (size_t)iWidth * 4u);
			continue;
		}
		for ( x = 0; x < iWidth; x++, p += 4, q += iBytes ) {
			uint16_t v;
			if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_R8 ) {
				q[0] = (tChoice.iSwizzle == XGE_SWIZZLE_WHITE_ALPHA || tChoice.iSwizzle == XGE_SWIZZLE_BLACK_ALPHA) ? p[3] : p[0];
			} else if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_RG8 ) {
				q[0] = p[0]; q[1] = p[3];
			} else {
				unsigned int r = p[0], g = p[1], b = p[2], a = p[3];
				if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_RGB565 )
					v = (uint16_t)((((r * 31u + 127u) / 255u) << 11) | (((g * 63u + 127u) / 255u) << 5) | ((b * 31u + 127u) / 255u));
				else if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_RGB5_A1 )
					v = (uint16_t)((((r * 31u + 127u) / 255u) << 11) | (((g * 31u + 127u) / 255u) << 6) | (((b * 31u + 127u) / 255u) << 1) | (a == 255u));
				else
					v = (uint16_t)((((r + 8u) / 17u) << 12) | (((g + 8u) / 17u) << 8) | (((b + 8u) / 17u) << 4) | ((a + 8u) / 17u));
				memcpy(q, &v, sizeof(v));
			}
		}
	}
	return pPacked;
}

static void __xgeTextureGLFormat(int iStorage, GLint* pInternal, GLenum* pFormat, GLenum* pType)
{
	*pInternal = GL_RGBA8; *pFormat = GL_RGBA; *pType = GL_UNSIGNED_BYTE;
	switch ( iStorage ) {
	case XGE_TEXTURE_STORAGE_R8: *pInternal = GL_R8; *pFormat = GL_RED; break;
	case XGE_TEXTURE_STORAGE_RG8: *pInternal = GL_RG8; *pFormat = GL_RG; break;
	case XGE_TEXTURE_STORAGE_RGB565: *pInternal = GL_RGB565; *pFormat = GL_RGB; *pType = GL_UNSIGNED_SHORT_5_6_5; break;
	case XGE_TEXTURE_STORAGE_RGB5_A1: *pInternal = GL_RGB5_A1; *pType = GL_UNSIGNED_SHORT_5_5_5_1; break;
	case XGE_TEXTURE_STORAGE_RGBA4: *pInternal = GL_RGBA4; *pType = GL_UNSIGNED_SHORT_4_4_4_4; break;
	default: break;
	}
}

static void __xgeTextureSetSwizzle(int iSwizzle)
{
	GLint r = GL_RED, g = GL_RED, b = GL_RED, a = GL_RED;
	if ( iSwizzle == 0 ) return; /* Also avoids unsupported WebGL2 tokens. */
	if ( iSwizzle == XGE_SWIZZLE_GRAY ) a = GL_ONE;
	else if ( iSwizzle == XGE_SWIZZLE_GRAY_ALPHA ) a = GL_GREEN;
	else if ( iSwizzle == XGE_SWIZZLE_WHITE_ALPHA ) r = g = b = GL_ONE;
	else if ( iSwizzle == XGE_SWIZZLE_BLACK_ALPHA ) r = g = b = GL_ZERO;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, g);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, b);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, a);
}

static void __xgeTextureClearGLErrors(void)
{
	int i;
	if ( glGetError != NULL ) for ( i = 0; i < 16 && glGetError() != GL_NO_ERROR; i++ ) {}
}

static int __xgeTextureGLError(GLenum iError)
{
	if ( iError == GL_NO_ERROR ) return XGE_OK;
	if ( iError == GL_INVALID_ENUM || iError == GL_INVALID_VALUE || iError == GL_INVALID_OPERATION ) return XGE_ERROR_UNSUPPORTED;
	if ( iError == GL_OUT_OF_MEMORY ) return XGE_ERROR_OUT_OF_MEMORY;
	return XGE_ERROR_GPU_FAILED;
}

/* Every attempt uses a fresh object: a rejected format/swizzle cannot pollute
 * the RGBA fallback. Existing textures are replaced only after success. */
static int __xgeTextureNewGPU(xge_texture pTexture, const unsigned char* pPixels, xge_texture_choice_t tChoice, GLuint* pId)
{
	void* pPacked = NULL;
	const void* pUpload = pPixels;
	GLuint iTexture = 0;
	GLint iBinding = 0, iAlignment = 4, iInternal;
	GLenum iFormat, iType, iError = GL_NO_ERROR;
	xge_sampler_t tSampler = pTexture->tSampler;
	int iRet;
	*pId = 0;
	if ( glGenTextures == NULL || glDeleteTextures == NULL || glBindTexture == NULL || glTexImage2D == NULL || glTexParameteri == NULL ) return XGE_ERROR_GPU_FAILED;
	iRet = __xgeSamplerNormalize(&tSampler);
	if ( iRet != XGE_OK ) return iRet;
	if ( tChoice.iStorage != XGE_TEXTURE_STORAGE_RGBA8 ) {
		pPacked = __xgeTexturePack(tChoice, pPixels, pTexture->iWidth, pTexture->iHeight, pTexture->iWidth * 4);
		if ( pPacked == NULL ) return XGE_ERROR_UNSUPPORTED; /* Optional scratch only. */
		pUpload = pPacked;
	}
	__xgeTextureGLFormat(tChoice.iStorage, &iInternal, &iFormat, &iType);
	if ( glGetIntegerv != NULL ) {
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &iBinding);
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &iAlignment);
	}
	__xgeTextureClearGLErrors();
	glGenTextures(1, &iTexture);
	if ( iTexture != 0 ) {
		glBindTexture(GL_TEXTURE_2D, iTexture);
		if ( glPixelStorei != NULL ) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, iInternal, pTexture->iWidth, pTexture->iHeight, 0, iFormat, iType, pUpload);
		if ( glGetError != NULL ) iError = glGetError();
		if ( iError == GL_NO_ERROR ) {
			__xgeTextureSetSwizzle(tChoice.iSwizzle);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, __xgeSamplerFilterToGL(tSampler.iMinFilter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, __xgeSamplerFilterToGL(tSampler.iMagFilter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, __xgeSamplerWrapToGL(tSampler.iWrapS));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, __xgeSamplerWrapToGL(tSampler.iWrapT));
			if ( glGetError != NULL ) iError = glGetError();
		}
	} else iError = glGetError != NULL ? glGetError() : GL_OUT_OF_MEMORY;
	if ( glPixelStorei != NULL ) glPixelStorei(GL_UNPACK_ALIGNMENT, iAlignment);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iBinding);
	xrtFree(pPacked);
	iRet = iTexture == 0 && iError == GL_NO_ERROR ? XGE_ERROR_GPU_FAILED : __xgeTextureGLError(iError);
	if ( iRet != XGE_OK ) {
		if ( iTexture != 0 ) glDeleteTextures(1, &iTexture);
		__xgeTextureClearGLErrors();
		return iRet;
	}
	*pId = iTexture;
	return XGE_OK;
}

static int __xgeTexturePatchFits(xge_texture_choice_t tChoice, const unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	int x, y;
	if ( tChoice.iStorage == XGE_TEXTURE_STORAGE_RGBA8 ) return 1;
	/* Mutable textures leave lossy storage on the first update, permanently. */
	if ( tChoice.iStorage >= XGE_TEXTURE_STORAGE_RGB565 ) return 0;
	for ( y = 0; y < iHeight; y++ ) {
		const unsigned char* p = pPixels + (size_t)y * (size_t)iStride;
		for ( x = 0; x < iWidth; x++, p += 4 ) {
			if ( p[0] != p[1] || p[0] != p[2] ) return 0;
			if ( tChoice.iSwizzle == XGE_SWIZZLE_COVERAGE && p[0] != p[3] ) return 0;
			if ( tChoice.iSwizzle == XGE_SWIZZLE_GRAY && p[3] != 255 ) return 0;
			if ( tChoice.iSwizzle == XGE_SWIZZLE_WHITE_ALPHA && p[0] != 255 ) return 0;
			if ( tChoice.iSwizzle == XGE_SWIZZLE_BLACK_ALPHA && p[0] != 0 ) return 0;
		}
	}
	return 1;
}

int xgeTextureGetStorageInfo(xge_texture pTexture, xge_texture_storage_info_t* pInfo)
{
	if ( pTexture == NULL || pInfo == NULL || pInfo->iSize < sizeof(*pInfo) || pTexture->iRefCount <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->iCompression = pTexture->iFlags & XGE_TEXTURE_COMPRESS_MASK;
	pInfo->bResident = pTexture->iBackendId != 0;
	if ( pTexture->iFormat == XGE_PIXEL_RGBA8 && __xgeTextureHasShadow(pTexture) ) {
		pInfo->iStorage = ((xge_texture_shadow_t*)pTexture->pBackend)->iStorage;
		pInfo->iCpuBytes = (uint64_t)pTexture->iWidth * (uint64_t)pTexture->iHeight * 4u;
	} else if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		pInfo->iStorage = XGE_TEXTURE_STORAGE_YUV420P;
		pInfo->iCompression = XGE_TEXTURE_COMPRESS_NONE;
		pInfo->iCpuBytes = __xgeTextureMemoryBytes(pTexture);
	} else return XGE_ERROR_INVALID_ARGUMENT;
	pInfo->iGpuBytes = pInfo->bResident ? __xgeTextureMemoryBytes(pTexture) : 0;
	return XGE_OK;
}
