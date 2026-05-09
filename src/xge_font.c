#ifndef XGE_NO_TEXT
typedef struct xge_glyph_cache_t {
	xge_glyph_t tGlyph;
	struct xge_glyph_cache_t* pNext;
} xge_glyph_cache_t;

typedef struct xge_glyph_atlas_page_t {
	int iCursorX;
	int iCursorY;
	int iRowHeight;
	int bDirty;
	unsigned char* pPixels;
	xge_texture_t tTexture;
} xge_glyph_atlas_page_t;

static stbtt_fontinfo* __xgeFontInfo(xge_font pFont)
{
	if ( pFont == NULL ) {
		return NULL;
	}
	return (stbtt_fontinfo*)pFont->pBackend;
}

static int __xgeFontFallbackUse(xge_font pFont, float fSize, int iOriginalError)
{
	if ( (pFont == NULL) || (g_xge.tFallbackFont.iRefCount <= 0) ) {
		return iOriginalError;
	}
	if ( xgeFontFallbackGet(pFont, fSize) != XGE_OK ) {
		return iOriginalError;
	}
	return XGE_OK;
}

static void __xgeFontAtlasFree(xge_font pFont)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_cache_t* pGlyph;
	xge_glyph_cache_t* pNext;
	int i;

	if ( pFont == NULL ) {
		return;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	if ( pPages != NULL ) {
		for ( i = 0; i < pFont->tAtlas.iPageCount; i++ ) {
			xgeTextureFree(&pPages[i].tTexture);
			xrtFree(pPages[i].pPixels);
		}
		xrtFree(pPages);
	}
	pGlyph = (xge_glyph_cache_t*)pFont->pGlyphs;
	while ( pGlyph != NULL ) {
		pNext = pGlyph->pNext;
		xrtFree(pGlyph);
		pGlyph = pNext;
	}
	pFont->tAtlas.pPages = NULL;
	pFont->tAtlas.iPageCount = 0;
	pFont->pGlyphs = NULL;
}

static xge_glyph_cache_t* __xgeFontGlyphFind(xge_font pFont, uint32_t iCodepoint)
{
	xge_glyph_cache_t* pGlyph;

	pGlyph = (xge_glyph_cache_t*)pFont->pGlyphs;
	while ( pGlyph != NULL ) {
		if ( pGlyph->tGlyph.iCodepoint == iCodepoint ) {
			return pGlyph;
		}
		pGlyph = pGlyph->pNext;
	}
	return NULL;
}

static int __xgeRangeInside(int iOffset, int iSize, int iTotal)
{
	if ( (iOffset < 0) || (iSize < 0) || (iTotal < 0) ) {
		return 0;
	}
	if ( iOffset > iTotal ) {
		return 0;
	}
	return iSize <= (iTotal - iOffset);
}

static xge_glyph_atlas_page_t* __xgeFontAtlasAddPage(xge_font pFont)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;
	int iNewCount;
	int iPixelSize;

	iNewCount = pFont->tAtlas.iPageCount + 1;
	pPages = (xge_glyph_atlas_page_t*)xrtRealloc(pFont->tAtlas.pPages, sizeof(*pPages) * (size_t)iNewCount);
	if ( pPages == NULL ) {
		return NULL;
	}
	pFont->tAtlas.pPages = pPages;
	pPage = &pPages[iNewCount - 1];
	memset(pPage, 0, sizeof(*pPage));
	iPixelSize = pFont->tAtlas.iPageWidth * pFont->tAtlas.iPageHeight * 4;
	pPage->pPixels = (unsigned char*)xrtMalloc((size_t)iPixelSize);
	if ( pPage->pPixels == NULL ) {
		return NULL;
	}
	memset(pPage->pPixels, 0, (size_t)iPixelSize);
	pPage->bDirty = 1;
	pFont->tAtlas.iPageCount = iNewCount;
	return pPage;
}

static xge_glyph_atlas_page_t* __xgeFontAtlasPlace(xge_font pFont, int iWidth, int iHeight, int* pPageIndex, int* pX, int* pY)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;
	int i;

	if ( (iWidth > pFont->tAtlas.iPageWidth) || (iHeight > pFont->tAtlas.iPageHeight) ) {
		return NULL;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	for ( i = 0; i < pFont->tAtlas.iPageCount; i++ ) {
		pPage = &pPages[i];
		if ( pPage->iCursorX + iWidth > pFont->tAtlas.iPageWidth ) {
			pPage->iCursorX = 0;
			pPage->iCursorY += pPage->iRowHeight + 1;
			pPage->iRowHeight = 0;
		}
		if ( pPage->iCursorY + iHeight <= pFont->tAtlas.iPageHeight ) {
			*pPageIndex = i;
			*pX = pPage->iCursorX;
			*pY = pPage->iCursorY;
			pPage->iCursorX += iWidth + 1;
			if ( iHeight > pPage->iRowHeight ) {
				pPage->iRowHeight = iHeight;
			}
			return pPage;
		}
	}
	pPage = __xgeFontAtlasAddPage(pFont);
	if ( pPage == NULL ) {
		return NULL;
	}
	*pPageIndex = pFont->tAtlas.iPageCount - 1;
	*pX = 0;
	*pY = 0;
	pPage->iCursorX = iWidth + 1;
	pPage->iCursorY = 0;
	pPage->iRowHeight = iHeight;
	return pPage;
}

static void __xgeFontAtlasBlit(xge_font pFont, xge_glyph_atlas_page_t* pPage, int iX, int iY, const xge_glyph_bitmap_t* pBitmap)
{
	unsigned char* pDst;
	unsigned char* pSrc;
	int iRow;
	int iCol;
	int iDstPos;

	for ( iRow = 0; iRow < pBitmap->iHeight; iRow++ ) {
		for ( iCol = 0; iCol < pBitmap->iWidth; iCol++ ) {
			pSrc = ((unsigned char*)pBitmap->pPixels) + iRow * pBitmap->iStride + iCol;
			iDstPos = (((iY + iRow) * pFont->tAtlas.iPageWidth) + (iX + iCol)) * 4;
			pDst = pPage->pPixels + iDstPos;
			pDst[0] = *pSrc;
			pDst[1] = *pSrc;
			pDst[2] = *pSrc;
			pDst[3] = *pSrc;
		}
	}
	pPage->bDirty = 1;
}

static int __xgeFontAtlasPageSetPixels(xge_font pFont, int iPage, int iWidth, int iHeight, int iFormat, const unsigned char* pPixels, int iPixelSize)
{
	xge_glyph_atlas_page_t* pPage;
	xge_glyph_atlas_page_t* pPages;
	int i;
	int iPixelCount;
	int iRGBABytes;

	while ( pFont->tAtlas.iPageCount <= iPage ) {
		if ( __xgeFontAtlasAddPage(pFont) == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	pPage = &pPages[iPage];
	if ( (iWidth != pFont->tAtlas.iPageWidth) || (iHeight != pFont->tAtlas.iPageHeight) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iPixelCount = iWidth * iHeight;
	iRGBABytes = iPixelCount * 4;
	if ( iFormat == XGE_XRF_PAGE_A8 ) {
		if ( iPixelSize < iPixelCount ) {
			return XGE_ERROR_RESOURCE_FAILED;
		}
		memset(pPage->pPixels, 0, (size_t)iRGBABytes);
		for ( i = 0; i < iPixelCount; i++ ) {
			pPage->pPixels[i * 4 + 0] = pPixels[i];
			pPage->pPixels[i * 4 + 1] = pPixels[i];
			pPage->pPixels[i * 4 + 2] = pPixels[i];
			pPage->pPixels[i * 4 + 3] = pPixels[i];
		}
	} else if ( iFormat == XGE_XRF_PAGE_RGBA8 ) {
		if ( iPixelSize < iRGBABytes ) {
			return XGE_ERROR_RESOURCE_FAILED;
		}
		memcpy(pPage->pPixels, pPixels, (size_t)iRGBABytes);
	} else {
		return XGE_ERROR_UNSUPPORTED;
	}
	pPage->bDirty = 1;
	return XGE_OK;
}

static int __xgeFontAtlasUploadPage(xge_font pFont, int iPage)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;

	if ( (pFont == NULL) || (iPage < 0) || (iPage >= pFont->tAtlas.iPageCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	pPage = &pPages[iPage];
	if ( pPage->tTexture.iBackendId == 0 ) {
		if ( xgeTextureCreateRGBA(&pPage->tTexture, pFont->tAtlas.iPageWidth, pFont->tAtlas.iPageHeight, pPage->pPixels) != XGE_OK ) {
			return XGE_ERROR_GPU_FAILED;
		}
		pPage->bDirty = 0;
		return XGE_OK;
	}
	if ( pPage->bDirty ) {
		xgeTextureFree(&pPage->tTexture);
		if ( xgeTextureCreateRGBA(&pPage->tTexture, pFont->tAtlas.iPageWidth, pFont->tAtlas.iPageHeight, pPage->pPixels) != XGE_OK ) {
			return XGE_ERROR_GPU_FAILED;
		}
		pPage->bDirty = 0;
	}
	return XGE_OK;
}

int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint)
{
	const unsigned char* pText;
	uint32_t iCodepoint;
	int iCount;

	if ( (psText == NULL) || (*psText == NULL) || (pCodepoint == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pText = (const unsigned char*)*psText;
	if ( pText[0] == 0 ) {
		return XGE_ERROR;
	}
	if ( pText[0] < 0x80 ) {
		iCodepoint = pText[0];
		iCount = 1;
	} else if ( (pText[0] & 0xE0) == 0xC0 ) {
		if ( (pText[1] & 0xC0) != 0x80 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		iCodepoint = ((uint32_t)(pText[0] & 0x1F) << 6) | (uint32_t)(pText[1] & 0x3F);
		iCount = 2;
		if ( iCodepoint < 0x80 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
	} else if ( (pText[0] & 0xF0) == 0xE0 ) {
		if ( ((pText[1] & 0xC0) != 0x80) || ((pText[2] & 0xC0) != 0x80) ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		iCodepoint = ((uint32_t)(pText[0] & 0x0F) << 12) | ((uint32_t)(pText[1] & 0x3F) << 6) | (uint32_t)(pText[2] & 0x3F);
		iCount = 3;
		if ( iCodepoint < 0x800 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
	} else {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( (iCodepoint >= 0xD800 && iCodepoint <= 0xDFFF) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	*pCodepoint = iCodepoint;
	*psText += iCount;
	return XGE_OK;
}

int xgeFontLoad(xge_font pFont, const char* sPath, float fSize)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pFont == NULL) || (sPath == NULL) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return __xgeFontFallbackUse(pFont, fSize, iRet);
	}
	iRet = xgeFontLoadMemory(pFont, tResource.pData, tResource.iSize, fSize);
	xgeResourceFree(&tResource);
	if ( iRet != XGE_OK ) {
		return __xgeFontFallbackUse(pFont, fSize, iRet);
	}
	return iRet;
}

int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize)
{
	stbtt_fontinfo* pInfo;
	void* pCopy;
	int iOffset;
	int iAscent;
	int iDescent;
	int iLineGap;

	if ( (pFont == NULL) || (pData == NULL) || (iSize <= 0) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pFont, 0, sizeof(*pFont));
	pCopy = xrtMalloc((size_t)iSize);
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pData, (size_t)iSize);
	pInfo = (stbtt_fontinfo*)xrtMalloc(sizeof(*pInfo));
	if ( pInfo == NULL ) {
		xrtFree(pCopy);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iOffset = stbtt_GetFontOffsetForIndex((const unsigned char*)pCopy, 0);
	if ( (iOffset < 0) || (stbtt_InitFont(pInfo, (const unsigned char*)pCopy, iOffset) == 0) ) {
		xrtFree(pInfo);
		xrtFree(pCopy);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	stbtt_GetFontVMetrics(pInfo, &iAscent, &iDescent, &iLineGap);
	pFont->iRefCount = 1;
	g_xge.iFontCount++;
	pFont->fSize = fSize;
	pFont->fScale = stbtt_ScaleForPixelHeight(pInfo, fSize);
	pFont->fAscent = (float)iAscent * pFont->fScale;
	pFont->fDescent = (float)iDescent * pFont->fScale;
	pFont->fLineGap = (float)iLineGap * pFont->fScale;
	pFont->fLineHeight = pFont->fAscent - pFont->fDescent + pFont->fLineGap;
	pFont->pData = pCopy;
	pFont->iDataSize = iSize;
	pFont->pBackend = pInfo;
	pFont->tAtlas.iPageWidth = 512;
	pFont->tAtlas.iPageHeight = 512;
	pFont->tAtlas.iFormat = XGE_PIXEL_RGBA8;
	return XGE_OK;
}

int xgeFontLoadXRF(xge_font pFont, const char* sPath)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pFont == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontLoadXRFMemory(pFont, tResource.pData, tResource.iSize);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize)
{
	const unsigned char* pBytes;
	const xge_xrf_header_t* pHeader;
	const xge_xrf_glyph_t* pXGlyphs;
	const xge_xrf_page_t* pXPages;
	const xge_xrf_page_t* pXPage;
	xge_glyph_cache_t* pCache;
	uint32_t i;
	int iRet;

	if ( (pFont == NULL) || (pData == NULL) || (iSize < (int)sizeof(xge_xrf_header_t)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pBytes = (const unsigned char*)pData;
	pHeader = (const xge_xrf_header_t*)pBytes;
	if ( (pHeader->iMagic != XGE_XRF_MAGIC) || (pHeader->iVersion != XGE_XRF_VERSION) || (pHeader->iHeaderSize < sizeof(xge_xrf_header_t)) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( !__xgeRangeInside((int)pHeader->iGlyphOffset, (int)(pHeader->iGlyphCount * sizeof(xge_xrf_glyph_t)), iSize) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( !__xgeRangeInside((int)pHeader->iPageOffset, (int)(pHeader->iPageCount * sizeof(xge_xrf_page_t)), iSize) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (pHeader->iRangeCount > 0) && !__xgeRangeInside((int)pHeader->iRangeOffset, (int)(pHeader->iRangeCount * sizeof(xge_xrf_range_t)), iSize) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (pHeader->iKerningCount > 0) && !__xgeRangeInside((int)pHeader->iKerningOffset, (int)(pHeader->iKerningCount * sizeof(xge_xrf_kerning_t)), iSize) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	memset(pFont, 0, sizeof(*pFont));
	pFont->iRefCount = 1;
	pFont->fAscent = pHeader->fAscent;
	pFont->fDescent = pHeader->fDescent;
	pFont->fLineGap = pHeader->fLineGap;
	pFont->fLineHeight = pHeader->fLineHeight;
	pFont->fSize = pHeader->fLineHeight;
	pFont->fScale = 1.0f;
	pXPages = (const xge_xrf_page_t*)(pBytes + pHeader->iPageOffset);
	if ( pHeader->iPageCount > 0 ) {
		pFont->tAtlas.iPageWidth = pXPages[0].iWidth;
		pFont->tAtlas.iPageHeight = pXPages[0].iHeight;
		pFont->tAtlas.iFormat = XGE_PIXEL_RGBA8;
	}
	for ( i = 0; i < pHeader->iPageCount; i++ ) {
		pXPage = &pXPages[i];
		if ( (pXPage->iWidth <= 0) || (pXPage->iHeight <= 0) || !__xgeRangeInside((int)(pHeader->iPixelOffset + pXPage->iPixelOffset), (int)pXPage->iPixelSize, iSize) ) {
			__xgeFontAtlasFree(pFont);
			memset(pFont, 0, sizeof(*pFont));
			return XGE_ERROR_RESOURCE_FAILED;
		}
		if ( (pXPage->iWidth != pFont->tAtlas.iPageWidth) || (pXPage->iHeight != pFont->tAtlas.iPageHeight) ) {
			__xgeFontAtlasFree(pFont);
			memset(pFont, 0, sizeof(*pFont));
			return XGE_ERROR_UNSUPPORTED;
		}
		iRet = __xgeFontAtlasPageSetPixels(pFont, (int)i, pXPage->iWidth, pXPage->iHeight, pXPage->iFormat, pBytes + pHeader->iPixelOffset + pXPage->iPixelOffset, (int)pXPage->iPixelSize);
		if ( iRet != XGE_OK ) {
			__xgeFontAtlasFree(pFont);
			memset(pFont, 0, sizeof(*pFont));
			return iRet;
		}
	}
	pXGlyphs = (const xge_xrf_glyph_t*)(pBytes + pHeader->iGlyphOffset);
	for ( i = 0; i < pHeader->iGlyphCount; i++ ) {
		if ( pXGlyphs[i].iCodepoint > 0xFFFF || pXGlyphs[i].iPage >= pHeader->iPageCount ) {
			__xgeFontAtlasFree(pFont);
			memset(pFont, 0, sizeof(*pFont));
			return XGE_ERROR_RESOURCE_FAILED;
		}
		pCache = (xge_glyph_cache_t*)xrtMalloc(sizeof(*pCache));
		if ( pCache == NULL ) {
			__xgeFontAtlasFree(pFont);
			memset(pFont, 0, sizeof(*pFont));
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pCache, 0, sizeof(*pCache));
		pCache->tGlyph.iCodepoint = pXGlyphs[i].iCodepoint;
		pCache->tGlyph.iGlyph = pXGlyphs[i].iGlyph;
		pCache->tGlyph.iPage = pXGlyphs[i].iPage;
		pCache->tGlyph.iX = pXGlyphs[i].iX;
		pCache->tGlyph.iY = pXGlyphs[i].iY;
		pCache->tGlyph.iWidth = pXGlyphs[i].iWidth;
		pCache->tGlyph.iHeight = pXGlyphs[i].iHeight;
		pCache->tGlyph.fOffsetX = pXGlyphs[i].fOffsetX;
		pCache->tGlyph.fOffsetY = pXGlyphs[i].fOffsetY;
		pCache->tGlyph.fAdvanceX = pXGlyphs[i].fAdvanceX;
		pCache->pNext = (xge_glyph_cache_t*)pFont->pGlyphs;
		pFont->pGlyphs = pCache;
	}
	g_xge.iFontCount++;
	return XGE_OK;
}

int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize)
{
	xge_xrf_header_t* pHeader;
	xge_xrf_range_t* pRange;
	xge_xrf_glyph_t* pGlyphs;
	xge_xrf_page_t* pPagesOut;
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_cache_t* pCache;
	xge_glyph_t tGlyph;
	unsigned char* pBytes;
	unsigned char* pDstPixels;
	uint32_t i;
	uint32_t iGlyphCount;
	uint32_t iPageCount;
	uint32_t iPixelBytes;
	uint32_t iGlyphOffset;
	uint32_t iPageOffset;
	uint32_t iRangeOffset;
	uint32_t iPixelOffset;
	uint32_t iTotalSize;

	if ( (pFont == NULL) || (ppData == NULL) || (pSize == NULL) || (iCount == 0) || (iCount > 0x10000u) || (iFirstCodepoint > 0xFFFFu) || ((iFirstCodepoint + iCount) > 0x10000u) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		(void)xgeFontGlyphAtlasGet(pFont, iFirstCodepoint + i, &tGlyph);
	}
	iGlyphCount = 0;
	for ( i = 0; i < iCount; i++ ) {
		pCache = __xgeFontGlyphFind(pFont, iFirstCodepoint + i);
		if ( (pCache != NULL) && (pCache->tGlyph.iPage >= 0) ) {
			iGlyphCount++;
		}
	}
	iPageCount = (uint32_t)pFont->tAtlas.iPageCount;
	if ( (iGlyphCount == 0) || (iPageCount == 0) || (pFont->tAtlas.pPages == NULL) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iPixelBytes = (uint32_t)(pFont->tAtlas.iPageWidth * pFont->tAtlas.iPageHeight * 4);
	if ( (iPixelBytes == 0) || (iPageCount > ((UINT32_MAX - sizeof(xge_xrf_header_t)) / iPixelBytes)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRangeOffset = (uint32_t)sizeof(xge_xrf_header_t);
	iGlyphOffset = iRangeOffset + (uint32_t)sizeof(xge_xrf_range_t);
	iPageOffset = iGlyphOffset + (iGlyphCount * (uint32_t)sizeof(xge_xrf_glyph_t));
	iPixelOffset = iPageOffset + (iPageCount * (uint32_t)sizeof(xge_xrf_page_t));
	iTotalSize = iPixelOffset + (iPageCount * iPixelBytes);
	if ( iTotalSize < iPixelOffset ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBytes = (unsigned char*)xrtMalloc(iTotalSize);
	if ( pBytes == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pBytes, 0, iTotalSize);
	pHeader = (xge_xrf_header_t*)pBytes;
	pRange = (xge_xrf_range_t*)(pBytes + iRangeOffset);
	pGlyphs = (xge_xrf_glyph_t*)(pBytes + iGlyphOffset);
	pPagesOut = (xge_xrf_page_t*)(pBytes + iPageOffset);
	pHeader->iMagic = XGE_XRF_MAGIC;
	pHeader->iVersion = XGE_XRF_VERSION;
	pHeader->iHeaderSize = (uint16_t)sizeof(xge_xrf_header_t);
	pHeader->iGlyphCount = iGlyphCount;
	pHeader->iPageCount = iPageCount;
	pHeader->iRangeCount = 1;
	pHeader->fAscent = pFont->fAscent;
	pHeader->fDescent = pFont->fDescent;
	pHeader->fLineGap = pFont->fLineGap;
	pHeader->fLineHeight = pFont->fLineHeight;
	pHeader->iGlyphOffset = iGlyphOffset;
	pHeader->iPageOffset = iPageOffset;
	pHeader->iRangeOffset = iRangeOffset;
	pHeader->iPixelOffset = iPixelOffset;
	pRange->iFirstCodepoint = iFirstCodepoint;
	pRange->iCount = iCount;
	iGlyphCount = 0;
	for ( i = 0; i < iCount; i++ ) {
		pCache = __xgeFontGlyphFind(pFont, iFirstCodepoint + i);
		if ( (pCache != NULL) && (pCache->tGlyph.iPage >= 0) ) {
			pGlyphs[iGlyphCount].iCodepoint = pCache->tGlyph.iCodepoint;
			pGlyphs[iGlyphCount].iGlyph = (uint16_t)pCache->tGlyph.iGlyph;
			pGlyphs[iGlyphCount].iPage = (uint16_t)pCache->tGlyph.iPage;
			pGlyphs[iGlyphCount].iX = (uint16_t)pCache->tGlyph.iX;
			pGlyphs[iGlyphCount].iY = (uint16_t)pCache->tGlyph.iY;
			pGlyphs[iGlyphCount].iWidth = (uint16_t)pCache->tGlyph.iWidth;
			pGlyphs[iGlyphCount].iHeight = (uint16_t)pCache->tGlyph.iHeight;
			pGlyphs[iGlyphCount].fOffsetX = pCache->tGlyph.fOffsetX;
			pGlyphs[iGlyphCount].fOffsetY = pCache->tGlyph.fOffsetY;
			pGlyphs[iGlyphCount].fAdvanceX = pCache->tGlyph.fAdvanceX;
			iGlyphCount++;
		}
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	for ( i = 0; i < iPageCount; i++ ) {
		pPagesOut[i].iWidth = (uint16_t)pFont->tAtlas.iPageWidth;
		pPagesOut[i].iHeight = (uint16_t)pFont->tAtlas.iPageHeight;
		pPagesOut[i].iFormat = XGE_XRF_PAGE_RGBA8;
		pPagesOut[i].iPixelOffset = i * iPixelBytes;
		pPagesOut[i].iPixelSize = iPixelBytes;
		pDstPixels = pBytes + iPixelOffset + pPagesOut[i].iPixelOffset;
		if ( pPages[i].pPixels != NULL ) {
			memcpy(pDstPixels, pPages[i].pPixels, iPixelBytes);
		}
	}
	*ppData = pBytes;
	*pSize = (int)iTotalSize;
	return XGE_OK;
}

int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount)
{
	FILE* pFile;
	char* sFullPath;
	void* pData;
	int iSize;
	int iRet;

	if ( (pFont == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pData = NULL;
	iSize = 0;
	iRet = xgeFontBuildXRFMemory(pFont, iFirstCodepoint, iCount, &pData, &iSize);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	sFullPath = __xgePathResolve(sPath);
	if ( sFullPath == NULL ) {
		xrtFree(pData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pFile = fopen(sFullPath, "wb");
	xrtFree(sFullPath);
	if ( pFile == NULL ) {
		xrtFree(pData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( fwrite(pData, 1, (size_t)iSize, pFile) != (size_t)iSize ) {
		fclose(pFile);
		xrtFree(pData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	fclose(pFile);
	xrtFree(pData);
	return XGE_OK;
}

int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount)
{
	int iRet;

	if ( (pFont == NULL) || (sTTFPath == NULL) || (sXRFPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeFontLoadXRF(pFont, sXRFPath);
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	iRet = xgeFontLoad(pFont, sTTFPath, fSize);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	(void)xgeFontSaveXRF(pFont, sXRFPath, iFirstCodepoint, iCount);
	return XGE_OK;
}

int xgeFontAddRef(xge_font pFont)
{
	if ( pFont == NULL ) {
		return 0;
	}
	if ( pFont->iRefCount < INT32_MAX ) {
		pFont->iRefCount++;
	}
	return pFont->iRefCount;
}

void xgeFontFree(xge_font pFont)
{
	if ( pFont == NULL ) {
		return;
	}
	if ( pFont->iRefCount > 1 ) {
		pFont->iRefCount--;
		return;
	}
	if ( pFont->iRefCount > 0 && g_xge.iFontCount > 0 ) {
		g_xge.iFontCount--;
	}
	if ( pFont->pBackend != NULL ) {
		xrtFree(pFont->pBackend);
	}
	if ( pFont->pData != NULL ) {
		xrtFree(pFont->pData);
	}
	__xgeFontAtlasFree(pFont);
	memset(pFont, 0, sizeof(*pFont));
}

void xgeFontSetFallback(xge_font pFont, xge_font pFallback)
{
	if ( pFont != NULL ) {
		pFont->pFallback = pFallback;
	}
}

int xgeFontFallbackSet(const char* sPath, float fSize)
{
	xge_resource_t tResource;
	int iRet;

	if ( (sPath == NULL) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontFallbackSetMemory(tResource.pData, tResource.iSize, fSize);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize)
{
	int iRet;

	if ( (pData == NULL) || (iSize <= 0) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeFontFallbackClear();
	iRet = xgeFontLoadMemory(&g_xge.tFallbackFont, pData, iSize, fSize);
	if ( iRet != XGE_OK ) {
		memset(&g_xge.tFallbackFont, 0, sizeof(g_xge.tFallbackFont));
		return iRet;
	}
	g_xge.tFallbackFont.iFlags |= XGE_FONT_FALLBACK;
	return XGE_OK;
}

int xgeFontFallbackGet(xge_font pFont, float fSize)
{
	int iRet;

	if ( (pFont == NULL) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (g_xge.tFallbackFont.iRefCount <= 0) || (g_xge.tFallbackFont.pData == NULL) || (g_xge.tFallbackFont.iDataSize <= 0) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = xgeFontLoadMemory(pFont, g_xge.tFallbackFont.pData, g_xge.tFallbackFont.iDataSize, fSize);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pFont->iFlags |= XGE_FONT_FALLBACK;
	return XGE_OK;
}

void xgeFontFallbackClear(void)
{
	if ( g_xge.tFallbackFont.iRefCount > 0 ) {
		xgeFontFree(&g_xge.tFallbackFont);
	}
	memset(&g_xge.tFallbackFont, 0, sizeof(g_xge.tFallbackFont));
}

int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics)
{
	stbtt_fontinfo* pInfo;
	xge_glyph_cache_t* pCache;
	int iGlyph;
	int iAdvance;
	int iBearing;
	int iX0;
	int iY0;
	int iX1;
	int iY1;

	if ( (pFont == NULL) || (pMetrics == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pCache = __xgeFontGlyphFind(pFont, iCodepoint);
	if ( pCache != NULL ) {
		memset(pMetrics, 0, sizeof(*pMetrics));
		pMetrics->iCodepoint = pCache->tGlyph.iCodepoint;
		pMetrics->iGlyph = pCache->tGlyph.iGlyph;
		pMetrics->fAdvanceX = pCache->tGlyph.fAdvanceX;
		pMetrics->fX0 = pCache->tGlyph.fOffsetX;
		pMetrics->fY0 = pCache->tGlyph.fOffsetY;
		pMetrics->fX1 = pCache->tGlyph.fOffsetX + (float)pCache->tGlyph.iWidth;
		pMetrics->fY1 = pCache->tGlyph.fOffsetY + (float)pCache->tGlyph.iHeight;
		return XGE_OK;
	}
	pInfo = __xgeFontInfo(pFont);
	if ( pInfo == NULL ) {
		if ( pFont->pFallback != NULL ) {
			return xgeFontGlyphGet(pFont->pFallback, iCodepoint, pMetrics);
		}
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iGlyph = stbtt_FindGlyphIndex(pInfo, (int)iCodepoint);
	if ( (iGlyph == 0) && (pFont->pFallback != NULL) ) {
		return xgeFontGlyphGet(pFont->pFallback, iCodepoint, pMetrics);
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iCodepoint = iCodepoint;
	pMetrics->iGlyph = iGlyph;
	stbtt_GetGlyphHMetrics(pInfo, iGlyph, &iAdvance, &iBearing);
	stbtt_GetGlyphBitmapBox(pInfo, iGlyph, pFont->fScale, pFont->fScale, &iX0, &iY0, &iX1, &iY1);
	pMetrics->fAdvanceX = (float)iAdvance * pFont->fScale;
	pMetrics->fLeftSideBearing = (float)iBearing * pFont->fScale;
	pMetrics->fX0 = (float)iX0;
	pMetrics->fY0 = (float)iY0;
	pMetrics->fX1 = (float)iX1;
	pMetrics->fY1 = (float)iY1;
	return XGE_OK;
}

int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap)
{
	stbtt_fontinfo* pInfo;
	unsigned char* pPixels;
	int iGlyph;
	int iWidth;
	int iHeight;
	int iOffsetX;
	int iOffsetY;

	if ( (pFont == NULL) || (pBitmap == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pInfo = __xgeFontInfo(pFont);
	if ( pInfo == NULL ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iGlyph = stbtt_FindGlyphIndex(pInfo, (int)iCodepoint);
	if ( (iGlyph == 0) && (pFont->pFallback != NULL) ) {
		return xgeFontGlyphRasterize(pFont->pFallback, iCodepoint, pBitmap);
	}
	memset(pBitmap, 0, sizeof(*pBitmap));
	pPixels = stbtt_GetCodepointBitmap(pInfo, pFont->fScale, pFont->fScale, (int)iCodepoint, &iWidth, &iHeight, &iOffsetX, &iOffsetY);
	if ( (pPixels == NULL) && (iWidth > 0) && (iHeight > 0) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBitmap->iCodepoint = iCodepoint;
	pBitmap->iWidth = iWidth;
	pBitmap->iHeight = iHeight;
	pBitmap->iStride = iWidth;
	pBitmap->iOffsetX = iOffsetX;
	pBitmap->iOffsetY = iOffsetY;
	pBitmap->iFormat = XGE_PIXEL_A8;
	pBitmap->pPixels = pPixels;
	return XGE_OK;
}

int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph)
{
	xge_glyph_cache_t* pCache;
	xge_glyph_bitmap_t tBitmap;
	xge_glyph_metrics_t tMetrics;
	xge_glyph_atlas_page_t* pPage;
	int iPage;
	int iX;
	int iY;
	int iRet;

	if ( (pFont == NULL) || (pGlyph == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pCache = __xgeFontGlyphFind(pFont, iCodepoint);
	if ( pCache != NULL ) {
		*pGlyph = pCache->tGlyph;
		return XGE_OK;
	}
	if ( __xgeFontInfo(pFont) == NULL ) {
		if ( pFont->pFallback != NULL ) {
			return xgeFontGlyphAtlasGet(pFont->pFallback, iCodepoint, pGlyph);
		}
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iRet = xgeFontGlyphGet(pFont, iCodepoint, &tMetrics);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontGlyphRasterize(pFont, iCodepoint, &tBitmap);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pCache = (xge_glyph_cache_t*)xrtMalloc(sizeof(*pCache));
	if ( pCache == NULL ) {
		xgeGlyphBitmapFree(&tBitmap);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pCache, 0, sizeof(*pCache));
	pCache->tGlyph.iCodepoint = iCodepoint;
	pCache->tGlyph.iGlyph = tMetrics.iGlyph;
	pCache->tGlyph.fAdvanceX = tMetrics.fAdvanceX;
	pCache->tGlyph.fOffsetX = (float)tBitmap.iOffsetX;
	pCache->tGlyph.fOffsetY = (float)tBitmap.iOffsetY;
	pCache->tGlyph.iWidth = tBitmap.iWidth;
	pCache->tGlyph.iHeight = tBitmap.iHeight;
	pCache->tGlyph.iPage = -1;
	if ( (tBitmap.iWidth > 0) && (tBitmap.iHeight > 0) ) {
		pPage = __xgeFontAtlasPlace(pFont, tBitmap.iWidth, tBitmap.iHeight, &iPage, &iX, &iY);
		if ( pPage == NULL ) {
			xrtFree(pCache);
			xgeGlyphBitmapFree(&tBitmap);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		__xgeFontAtlasBlit(pFont, pPage, iX, iY, &tBitmap);
		pCache->tGlyph.iPage = iPage;
		pCache->tGlyph.iX = iX;
		pCache->tGlyph.iY = iY;
	}
	pCache->pNext = (xge_glyph_cache_t*)pFont->pGlyphs;
	pFont->pGlyphs = pCache;
	*pGlyph = pCache->tGlyph;
	xgeGlyphBitmapFree(&tBitmap);
	return XGE_OK;
}

void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap)
{
	if ( pBitmap == NULL ) {
		return;
	}
	if ( pBitmap->pPixels != NULL ) {
		xrtFree(pBitmap->pPixels);
	}
	memset(pBitmap, 0, sizeof(*pBitmap));
}

xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText)
{
	xge_vec2_t tSize;
	xge_glyph_metrics_t tMetrics;
	const char* sScan;
	uint32_t iCodepoint;
	float fLineWidth;
	int iRet;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pFont == NULL) || (sText == NULL) ) {
		return tSize;
	}
	sScan = sText;
	fLineWidth = 0.0f;
	tSize.fY = pFont->fLineHeight;
	while ( *sScan != 0 ) {
		iRet = xgeTextUTF8Next(&sScan, &iCodepoint);
		if ( iRet != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			if ( fLineWidth > tSize.fX ) {
				tSize.fX = fLineWidth;
			}
			fLineWidth = 0.0f;
			tSize.fY += pFont->fLineHeight;
			continue;
		}
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fLineWidth += tMetrics.fAdvanceX;
		}
	}
	if ( fLineWidth > tSize.fX ) {
		tSize.fX = fLineWidth;
	}
	return tSize;
}

static float __xgeTextLineMeasure(xge_font pFont, const char* sText, int iSize)
{
	xge_glyph_metrics_t tMetrics;
	const char* sScan;
	const char* sEnd;
	uint32_t iCodepoint;
	float fWidth;

	if ( (pFont == NULL) || (sText == NULL) || (iSize <= 0) ) {
		return 0.0f;
	}
	sScan = sText;
	sEnd = sText + iSize;
	fWidth = 0.0f;
	while ( sScan < sEnd ) {
		if ( xgeTextUTF8Next(&sScan, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fWidth += tMetrics.fAdvanceX;
		}
	}
	return fWidth;
}

static const char* __xgeTextLineEnd(const char* sText)
{
	const char* sScan;

	sScan = sText;
	while ( (*sScan != 0) && (*sScan != '\n') ) {
		sScan++;
	}
	return sScan;
}

static void __xgeTextDrawRange(xge_font pFont, const char* sText, int iSize, float fX, float fY, uint32_t iColor)
{
	const char* sScan;
	const char* sEnd;
	uint32_t iCodepoint;
	xge_glyph_t tGlyph;
	xge_draw_t tDraw;
	xge_glyph_atlas_page_t* pPages;
	float fPenX;
	float fPenY;
	int iRet;

	if ( (pFont == NULL) || (sText == NULL) || (iSize <= 0) ) {
		return;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	sScan = sText;
	sEnd = sText + iSize;
	fPenX = fX;
	fPenY = fY + pFont->fAscent;
	while ( sScan < sEnd ) {
		iRet = xgeTextUTF8Next(&sScan, &iCodepoint);
		if ( iRet != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			fPenX = fX;
			fPenY += pFont->fLineHeight;
			continue;
		}
		if ( xgeFontGlyphAtlasGet(pFont, iCodepoint, &tGlyph) != XGE_OK ) {
			continue;
		}
		if ( (tGlyph.iPage >= 0) && (tGlyph.iWidth > 0) && (tGlyph.iHeight > 0) ) {
			pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
			if ( __xgeFontAtlasUploadPage(pFont, tGlyph.iPage) == XGE_OK ) {
				memset(&tDraw, 0, sizeof(tDraw));
				tDraw.pTexture = &pPages[tGlyph.iPage].tTexture;
				tDraw.tSrc.fX = (float)tGlyph.iX;
				tDraw.tSrc.fY = (float)tGlyph.iY;
				tDraw.tSrc.fW = (float)tGlyph.iWidth;
				tDraw.tSrc.fH = (float)tGlyph.iHeight;
				tDraw.tDst.fX = fPenX + tGlyph.fOffsetX;
				tDraw.tDst.fY = fPenY + tGlyph.fOffsetY;
				tDraw.tDst.fW = (float)tGlyph.iWidth;
				tDraw.tDst.fH = (float)tGlyph.iHeight;
				tDraw.iColor = iColor;
				xgeDrawEx(&tDraw);
			}
		}
		fPenX += tGlyph.fAdvanceX;
	}
}

void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor)
{
	const char* sLine;
	const char* sEnd;
	float fPenY;

	if ( (pFont == NULL) || (sText == NULL) ) {
		return;
	}
	sLine = sText;
	fPenY = fY;
	while ( *sLine != 0 ) {
		sEnd = __xgeTextLineEnd(sLine);
		__xgeTextDrawRange(pFont, sLine, (int)(sEnd - sLine), fX, fPenY, iColor);
		if ( *sEnd == '\n' ) {
			sLine = sEnd + 1;
			fPenY += pFont->fLineHeight;
		} else {
			break;
		}
	}
}

void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	const char* sLine;
	const char* sEnd;
	xge_vec2_t tSize;
	xge_rect_t tOldClip;
	xge_rect_t tClip;
	float fPenY;
	float fLineX;
	float fLineWidth;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	int iLineSize;
	int bClip;
	int bOldClip;

	if ( (pFont == NULL) || (sText == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	memset(&tOldClip, 0, sizeof(tOldClip));
	memset(&tClip, 0, sizeof(tClip));
	bOldClip = 0;
	tSize = xgeTextMeasure(pFont, sText);
	fPenY = tRect.fY;
	if ( (iFlags & XGE_TEXT_ALIGN_BOTTOM) == XGE_TEXT_ALIGN_BOTTOM ) {
		fPenY = tRect.fY + tRect.fH - tSize.fY;
	} else if ( (iFlags & XGE_TEXT_ALIGN_MIDDLE) == XGE_TEXT_ALIGN_MIDDLE ) {
		fPenY = tRect.fY + (tRect.fH - tSize.fY) * 0.5f;
	}
	bClip = ((iFlags & XGE_TEXT_CLIP) != 0);
	if ( bClip ) {
		tOldClip = xgeClipGet();
		bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
		tClip = tRect;
		if ( bOldClip ) {
			fLeft = (tClip.fX > tOldClip.fX) ? tClip.fX : tOldClip.fX;
			fTop = (tClip.fY > tOldClip.fY) ? tClip.fY : tOldClip.fY;
			fRight = ((tClip.fX + tClip.fW) < (tOldClip.fX + tOldClip.fW)) ? (tClip.fX + tClip.fW) : (tOldClip.fX + tOldClip.fW);
			fBottom = ((tClip.fY + tClip.fH) < (tOldClip.fY + tOldClip.fH)) ? (tClip.fY + tClip.fH) : (tOldClip.fY + tOldClip.fH);
			tClip.fX = fLeft;
			tClip.fY = fTop;
			tClip.fW = fRight - fLeft;
			tClip.fH = fBottom - fTop;
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			return;
		}
		xgeClipSet(tClip);
	}
	sLine = sText;
	while ( *sLine != 0 ) {
		sEnd = __xgeTextLineEnd(sLine);
		iLineSize = (int)(sEnd - sLine);
		fLineWidth = __xgeTextLineMeasure(pFont, sLine, iLineSize);
		fLineX = tRect.fX;
		if ( (iFlags & XGE_TEXT_ALIGN_RIGHT) == XGE_TEXT_ALIGN_RIGHT ) {
			fLineX = tRect.fX + tRect.fW - fLineWidth;
		} else if ( (iFlags & XGE_TEXT_ALIGN_CENTER) == XGE_TEXT_ALIGN_CENTER ) {
			fLineX = tRect.fX + (tRect.fW - fLineWidth) * 0.5f;
		}
		__xgeTextDrawRange(pFont, sLine, iLineSize, fLineX, fPenY, iColor);
		if ( (iFlags & XGE_TEXT_UNDERLINE) != 0 ) {
			xge_rect_t tUnderline;
			tUnderline.fX = fLineX;
			tUnderline.fY = fPenY + pFont->fAscent + 1.0f;
			tUnderline.fW = fLineWidth;
			tUnderline.fH = (pFont->fLineHeight >= 18.0f) ? 2.0f : 1.0f;
			xgeShapeRectFillPx(tUnderline, iColor);
		}
		if ( *sEnd == '\n' ) {
			sLine = sEnd + 1;
			fPenY += pFont->fLineHeight;
		} else {
			break;
		}
	}
	if ( bClip ) {
		if ( bOldClip ) {
			xgeClipSet(tOldClip);
		} else {
			xgeClipClear();
		}
	}
}
#else
int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint) { (void)psText; (void)pCodepoint; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoad(xge_font pFont, const char* sPath, float fSize) { (void)pFont; (void)sPath; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize) { (void)pFont; (void)pData; (void)iSize; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoadXRF(xge_font pFont, const char* sPath) { (void)pFont; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize) { (void)pFont; (void)pData; (void)iSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontAddRef(xge_font pFont) { (void)pFont; return 0; }
void xgeFontFree(xge_font pFont) { (void)pFont; }
void xgeFontSetFallback(xge_font pFont, xge_font pFallback) { (void)pFont; (void)pFallback; }
int xgeFontFallbackSet(const char* sPath, float fSize) { (void)sPath; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize) { (void)pData; (void)iSize; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontFallbackGet(xge_font pFont, float fSize) { (void)pFont; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
void xgeFontFallbackClear(void) {}
int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics) { (void)pFont; (void)iCodepoint; (void)pMetrics; return XGE_ERROR_UNSUPPORTED; }
int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap) { (void)pFont; (void)iCodepoint; (void)pBitmap; return XGE_ERROR_UNSUPPORTED; }
int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph) { (void)pFont; (void)iCodepoint; (void)pGlyph; return XGE_ERROR_UNSUPPORTED; }
void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap) { (void)pBitmap; }
xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText) { xge_vec2_t tSize; (void)pFont; (void)sText; tSize.fX = 0.0f; tSize.fY = 0.0f; return tSize; }
void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor) { (void)pFont; (void)sText; (void)fX; (void)fY; (void)iColor; }
void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags) { (void)pFont; (void)sText; (void)tRect; (void)iColor; (void)iFlags; }
#endif
