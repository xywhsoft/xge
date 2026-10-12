#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XRF_MAGIC 0x32465258u
#define XRF_VERSION 1
#define XRF_PAGE_A8 1
#define PAGE_W 512
#define PAGE_H 512

typedef struct xrf_header_t {
	uint32_t iMagic;
	uint16_t iVersion;
	uint16_t iHeaderSize;
	uint32_t iFlags;
	uint32_t iGlyphCount;
	uint32_t iPageCount;
	uint32_t iRangeCount;
	uint32_t iKerningCount;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
	uint32_t iGlyphOffset;
	uint32_t iPageOffset;
	uint32_t iRangeOffset;
	uint32_t iKerningOffset;
	uint32_t iPixelOffset;
} xrf_header_t;

typedef struct xrf_glyph_t {
	uint32_t iCodepoint;
	uint16_t iPage;
	uint16_t iGlyph;
	uint16_t iX;
	uint16_t iY;
	uint16_t iWidth;
	uint16_t iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xrf_glyph_t;

typedef struct xrf_page_t {
	uint16_t iWidth;
	uint16_t iHeight;
	uint16_t iFormat;
	uint16_t iReserved;
	uint32_t iPixelOffset;
	uint32_t iPixelSize;
} xrf_page_t;

typedef struct page_t {
	unsigned char* pPixels;
	int iCursorX;
	int iCursorY;
	int iRowHeight;
} page_t;

typedef struct codepoint_list_t {
	uint32_t* pItems;
	int iCount;
	int iCapacity;
} codepoint_list_t;

static int CodepointCompare(const void* pA, const void* pB)
{
	uint32_t a = *(const uint32_t*)pA;
	uint32_t b = *(const uint32_t*)pB;
	return (a > b) - (a < b);
}

static int CodepointAdd(codepoint_list_t* pList, uint32_t iCodepoint)
{
	uint32_t* pNewItems;
	int iNewCapacity;

	if ( pList == NULL || iCodepoint == 0 || iCodepoint > 0xFFFFu ) {
		return 0;
	}
	if ( pList->iCount >= pList->iCapacity ) {
		iNewCapacity = (pList->iCapacity > 0) ? (pList->iCapacity * 2) : 512;
		pNewItems = (uint32_t*)realloc(pList->pItems, (size_t)iNewCapacity * sizeof(uint32_t));
		if ( pNewItems == NULL ) {
			return 0;
		}
		pList->pItems = pNewItems;
		pList->iCapacity = iNewCapacity;
	}
	pList->pItems[pList->iCount++] = iCodepoint;
	return 1;
}

static void CodepointListUniqueSort(codepoint_list_t* pList)
{
	int i;
	int iWrite;

	if ( pList == NULL || pList->iCount <= 1 ) {
		return;
	}
	qsort(pList->pItems, (size_t)pList->iCount, sizeof(uint32_t), CodepointCompare);
	iWrite = 1;
	for ( i = 1; i < pList->iCount; i++ ) {
		if ( pList->pItems[i] != pList->pItems[iWrite - 1] ) {
			pList->pItems[iWrite++] = pList->pItems[i];
		}
	}
	pList->iCount = iWrite;
}

static int AddDefaultCodepoints(codepoint_list_t* pList)
{
	unsigned char arrBytes[2];
	wchar_t arrWide[2];
	int iLead;
	int iTrail;
	int iWide;

	for ( iWide = 32; iWide <= 126; iWide++ ) {
		if ( !CodepointAdd(pList, (uint32_t)iWide) ) {
			return 0;
		}
	}
	for ( iWide = 0x3000; iWide <= 0x303F; iWide++ ) {
		if ( !CodepointAdd(pList, (uint32_t)iWide) ) {
			return 0;
		}
	}
	for ( iWide = 0xFF01; iWide <= 0xFF5E; iWide++ ) {
		if ( !CodepointAdd(pList, (uint32_t)iWide) ) {
			return 0;
		}
	}
	for ( iLead = 0xA1; iLead <= 0xF7; iLead++ ) {
		for ( iTrail = 0xA1; iTrail <= 0xFE; iTrail++ ) {
			arrBytes[0] = (unsigned char)iLead;
			arrBytes[1] = (unsigned char)iTrail;
			if ( MultiByteToWideChar(936, MB_ERR_INVALID_CHARS, (LPCCH)arrBytes, 2, arrWide, 1) == 1 ) {
				if ( arrWide[0] != 0 && !CodepointAdd(pList, (uint32_t)arrWide[0]) ) {
					return 0;
				}
			}
		}
	}
	CodepointListUniqueSort(pList);
	return 1;
}

static page_t* AddPage(page_t** ppPages, int* pPageCount)
{
	page_t* pPages;
	page_t* pPage;
	int iNewCount;

	iNewCount = *pPageCount + 1;
	pPages = (page_t*)realloc(*ppPages, (size_t)iNewCount * sizeof(page_t));
	if ( pPages == NULL ) {
		return NULL;
	}
	*ppPages = pPages;
	pPage = &pPages[iNewCount - 1];
	memset(pPage, 0, sizeof(*pPage));
	pPage->pPixels = (unsigned char*)calloc((size_t)PAGE_W * PAGE_H, 1);
	if ( pPage->pPixels == NULL ) {
		return NULL;
	}
	*pPageCount = iNewCount;
	return pPage;
}

static page_t* PlaceGlyph(page_t** ppPages, int* pPageCount, int iWidth, int iHeight, int* pPageIndex, int* pX, int* pY)
{
	page_t* pPage;
	int i;

	if ( iWidth <= 0 || iHeight <= 0 || iWidth > PAGE_W || iHeight > PAGE_H ) {
		return NULL;
	}
	for ( i = 0; i < *pPageCount; i++ ) {
		pPage = &(*ppPages)[i];
		if ( pPage->iCursorX + iWidth > PAGE_W ) {
			pPage->iCursorX = 0;
			pPage->iCursorY += pPage->iRowHeight + 1;
			pPage->iRowHeight = 0;
		}
		if ( pPage->iCursorY + iHeight <= PAGE_H ) {
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
	pPage = AddPage(ppPages, pPageCount);
	if ( pPage == NULL ) {
		return NULL;
	}
	*pPageIndex = *pPageCount - 1;
	*pX = 0;
	*pY = 0;
	pPage->iCursorX = iWidth + 1;
	pPage->iRowHeight = iHeight;
	return pPage;
}

static void BlitMonoGlyph(page_t* pPage, int iDstX, int iDstY, const unsigned char* pBits, int iWidth, int iHeight)
{
	int iStride;
	int x;
	int y;
	int iByte;
	int iBit;

	iStride = ((iWidth + 31) / 32) * 4;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			iByte = y * iStride + (x / 8);
			iBit = 7 - (x % 8);
			if ( (pBits[iByte] & (1u << iBit)) != 0 ) {
				pPage->pPixels[(iDstY + y) * PAGE_W + (iDstX + x)] = 255;
			}
		}
	}
}

static int WriteXrf(const char* sPath, const xrf_header_t* pHeader, const xrf_glyph_t* pGlyphs, const xrf_page_t* pPageHeaders, const page_t* pPages)
{
	FILE* fp;
	uint32_t i;

	fp = fopen(sPath, "wb");
	if ( fp == NULL ) {
		return 0;
	}
	fwrite(pHeader, 1, sizeof(*pHeader), fp);
	fwrite(pGlyphs, sizeof(xrf_glyph_t), pHeader->iGlyphCount, fp);
	fwrite(pPageHeaders, sizeof(xrf_page_t), pHeader->iPageCount, fp);
	for ( i = 0; i < pHeader->iPageCount; i++ ) {
		fwrite(pPages[i].pPixels, 1, (size_t)PAGE_W * PAGE_H, fp);
	}
	fclose(fp);
	return 1;
}

static int BuildFont(const wchar_t* sFaceName, int iPixelSize, const char* sOutput)
{
	codepoint_list_t tCodes;
	xrf_glyph_t* pGlyphs;
	xrf_page_t* pPageHeaders;
	page_t* pPages;
	HDC hdc;
	HFONT hFont;
	HGDIOBJ hOldFont;
	TEXTMETRICW tMetric;
	MAT2 tMat;
	GLYPHMETRICS tGm;
	DWORD iBytes;
	unsigned char* pBitmap;
	SIZE tSize;
	xrf_header_t tHeader;
	page_t* pPage;
	int iPageCount;
	int iGlyphCount;
	int iPage;
	int iX;
	int iY;
	int i;

	memset(&tCodes, 0, sizeof(tCodes));
	if ( !AddDefaultCodepoints(&tCodes) ) {
		fprintf(stderr, "fontbuild: failed to build codepoint list\n");
		return 1;
	}
	hdc = CreateCompatibleDC(NULL);
	if ( hdc == NULL ) {
		fprintf(stderr, "fontbuild: CreateCompatibleDC failed\n");
		return 1;
	}
	hFont = CreateFontW(-iPixelSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, GB2312_CHARSET,
		OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FIXED_PITCH | FF_ROMAN, sFaceName);
	if ( hFont == NULL ) {
		fprintf(stderr, "fontbuild: CreateFontW failed\n");
		DeleteDC(hdc);
		return 1;
	}
	hOldFont = SelectObject(hdc, hFont);
	GetTextMetricsW(hdc, &tMetric);
	memset(&tMat, 0, sizeof(tMat));
	tMat.eM11.value = 1;
	tMat.eM22.value = 1;
	pGlyphs = (xrf_glyph_t*)calloc((size_t)tCodes.iCount, sizeof(xrf_glyph_t));
	pPages = NULL;
	iPageCount = 0;
	iGlyphCount = 0;
	for ( i = 0; i < tCodes.iCount; i++ ) {
		wchar_t wc;
		memset(&tGm, 0, sizeof(tGm));
		wc = (wchar_t)tCodes.pItems[i];
		iBytes = GetGlyphOutlineW(hdc, wc, GGO_BITMAP, &tGm, 0, NULL, &tMat);
		if ( iBytes == GDI_ERROR ) {
			continue;
		}
		pGlyphs[iGlyphCount].iCodepoint = tCodes.pItems[i];
		pGlyphs[iGlyphCount].iGlyph = (uint16_t)tCodes.pItems[i];
		pGlyphs[iGlyphCount].fOffsetX = (float)tGm.gmptGlyphOrigin.x;
		pGlyphs[iGlyphCount].fOffsetY = (float)-tGm.gmptGlyphOrigin.y;
		pGlyphs[iGlyphCount].fAdvanceX = (float)tGm.gmCellIncX;
		pGlyphs[iGlyphCount].iPage = 0;
		if ( tGm.gmBlackBoxX == 0 || tGm.gmBlackBoxY == 0 || iBytes == 0 ) {
			if ( GetTextExtentPoint32W(hdc, &wc, 1, &tSize) ) {
				pGlyphs[iGlyphCount].fAdvanceX = (float)tSize.cx;
			}
			iGlyphCount++;
			continue;
		}
		pBitmap = (unsigned char*)malloc((size_t)iBytes);
		if ( pBitmap == NULL ) {
			fprintf(stderr, "fontbuild: out of memory\n");
			return 1;
		}
		if ( GetGlyphOutlineW(hdc, wc, GGO_BITMAP, &tGm, iBytes, pBitmap, &tMat) == GDI_ERROR ) {
			free(pBitmap);
			continue;
		}
		pPage = PlaceGlyph(&pPages, &iPageCount, (int)tGm.gmBlackBoxX, (int)tGm.gmBlackBoxY, &iPage, &iX, &iY);
		if ( pPage == NULL ) {
			free(pBitmap);
			fprintf(stderr, "fontbuild: failed to place glyph U+%04X\n", (unsigned)tCodes.pItems[i]);
			return 1;
		}
		BlitMonoGlyph(pPage, iX, iY, pBitmap, (int)tGm.gmBlackBoxX, (int)tGm.gmBlackBoxY);
		pGlyphs[iGlyphCount].iPage = (uint16_t)iPage;
		pGlyphs[iGlyphCount].iX = (uint16_t)iX;
		pGlyphs[iGlyphCount].iY = (uint16_t)iY;
		pGlyphs[iGlyphCount].iWidth = (uint16_t)tGm.gmBlackBoxX;
		pGlyphs[iGlyphCount].iHeight = (uint16_t)tGm.gmBlackBoxY;
		iGlyphCount++;
		free(pBitmap);
	}
	pPageHeaders = (xrf_page_t*)calloc((size_t)iPageCount, sizeof(xrf_page_t));
	if ( pPageHeaders == NULL ) {
		fprintf(stderr, "fontbuild: out of memory\n");
		return 1;
	}
	memset(&tHeader, 0, sizeof(tHeader));
	tHeader.iMagic = XRF_MAGIC;
	tHeader.iVersion = XRF_VERSION;
	tHeader.iHeaderSize = (uint16_t)sizeof(tHeader);
	tHeader.iGlyphCount = (uint32_t)iGlyphCount;
	tHeader.iPageCount = (uint32_t)iPageCount;
	tHeader.fAscent = (float)tMetric.tmAscent;
	tHeader.fDescent = (float)-tMetric.tmDescent;
	tHeader.fLineGap = (float)tMetric.tmExternalLeading;
	tHeader.fLineHeight = (float)(tMetric.tmAscent + tMetric.tmDescent + tMetric.tmExternalLeading);
	tHeader.iGlyphOffset = (uint32_t)sizeof(tHeader);
	tHeader.iPageOffset = tHeader.iGlyphOffset + (uint32_t)(iGlyphCount * sizeof(xrf_glyph_t));
	tHeader.iPixelOffset = tHeader.iPageOffset + (uint32_t)(iPageCount * sizeof(xrf_page_t));
	for ( i = 0; i < iPageCount; i++ ) {
		pPageHeaders[i].iWidth = PAGE_W;
		pPageHeaders[i].iHeight = PAGE_H;
		pPageHeaders[i].iFormat = XRF_PAGE_A8;
		pPageHeaders[i].iPixelOffset = (uint32_t)(i * PAGE_W * PAGE_H);
		pPageHeaders[i].iPixelSize = PAGE_W * PAGE_H;
	}
	if ( !WriteXrf(sOutput, &tHeader, pGlyphs, pPageHeaders, pPages) ) {
		fprintf(stderr, "fontbuild: failed to write %s\n", sOutput);
		return 1;
	}
	printf("fontbuild: %s size=%d glyphs=%d pages=%d lineHeight=%.1f\n", sOutput, iPixelSize, iGlyphCount, iPageCount, tHeader.fLineHeight);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
	DeleteDC(hdc);
	for ( i = 0; i < iPageCount; i++ ) {
		free(pPages[i].pPixels);
	}
	free(pPages);
	free(pPageHeaders);
	free(pGlyphs);
	free(tCodes.pItems);
	return 0;
}

int main(int argc, char** argv)
{
	const char* sOut12;
	const char* sOut16;

	sOut12 = (argc > 1) ? argv[1] : "simsun12.xrf";
	sOut16 = (argc > 2) ? argv[2] : "simsun16.xrf";
	if ( BuildFont(L"SimSun", 12, sOut12) != 0 ) {
		return 1;
	}
	if ( BuildFont(L"SimSun", 16, sOut16) != 0 ) {
		return 1;
	}
	return 0;
}
