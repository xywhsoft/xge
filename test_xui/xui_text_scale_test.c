#include "xui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct text_work_t {
	unsigned long long iRangeProbes;
	unsigned long long iNextProbes;
	unsigned long long iWhitespaceVisits;
	unsigned long long iWrapSteps;
	unsigned long long iIndexBuildSteps;
} text_work_t;

static text_work_t g_tWork;
static int g_bFailMalloc;
static void* textMalloc(size_t iSize)
{
	return g_bFailMalloc ? NULL : xrtMalloc(iSize);
}
#define xrtMalloc textMalloc
#define XUI_TEXT_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#include "../src/xui_text.c"
#undef xrtMalloc

#define CHECK(expr) do { if ( !(expr) ) { \
	printf("text scale failure at line %d: %s\n", __LINE__, #expr); \
	iFailed = 1; goto cleanup; } } while ( 0 )

static int textScale(int iCount, int iPattern, int bBaseline)
{
	struct xui_text_layout_t tLayout;
	int i;
	int iLog = 0;
	int iFailed = 0;
	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.sText = (char*)xrtMalloc((size_t)iCount + 1u);
	tLayout.tShape.pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)iCount, sizeof(xui_text_cluster_t));
	CHECK(tLayout.sText != NULL && tLayout.tShape.pClusters != NULL);
	tLayout.iTextSize = iCount;
	tLayout.tShape.iClusterCount = iCount;
	tLayout.tMetrics.fLineHeight = 12.0f;
	tLayout.tMetrics.fAscent = 9.0f;
	tLayout.tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tLayout.tDesc.fMaxWidth = iPattern < 2 ? (float)iCount / 2.0f : 8.0f;
	for ( i = 0; i < iCount; i++ ) {
		tLayout.sText[i] = iPattern == 1 ? ' ' : 'a';
		if ( iPattern == 2 ) tLayout.sText[i] = "ab cdef "[i % 8];
		if ( iPattern == 3 && i % 4 == 3 ) tLayout.sText[i] = '\n';
		if ( iPattern == 4 && i % 256 != 0 ) tLayout.sText[i] = ' ';
		tLayout.tShape.pClusters[i].iTextStart = i;
		tLayout.tShape.pClusters[i].iTextEnd = i + 1;
		tLayout.tShape.pClusters[i].fAdvance = 1.0f;
	}
	tLayout.sText[iCount] = 0;
	memset(&g_tWork, 0, sizeof(g_tWork));
	CHECK(__xuiTextLayoutBuild(&tLayout) == XUI_OK);
	if ( iPattern < 2 ) {
		CHECK(tLayout.iLineCount == (iPattern == 1 ? 1 : 2));
		CHECK(tLayout.pLines[0].iTextSize == (iPattern == 1 ? 0 : iCount / 2));
	} else if ( iPattern == 2 ) {
		CHECK(tLayout.iLineCount == iCount / 8);
		CHECK(tLayout.pLines[0].iTextSize == 7);
	} else if ( iPattern == 3 ) {
		CHECK(tLayout.iLineCount == iCount / 4 + 1);
		CHECK(tLayout.pLines[0].iTextSize == 3);
	} else {
		CHECK(tLayout.iLineCount == iCount / 256);
		CHECK(tLayout.pLines[0].iTextSize == 1);
	}
	for ( i = iCount; i > 0; i /= 2 ) iLog++;
	printf("%s N=%d range=%llu next=%llu whitespace=%llu wrap=%llu index=%llu\n",
		(const char*[]){"unbroken", "spaces", "words", "paragraphs", "space-runs"}[iPattern],
		iCount, g_tWork.iRangeProbes, g_tWork.iNextProbes,
		g_tWork.iWhitespaceVisits, g_tWork.iWrapSteps, g_tWork.iIndexBuildSteps);
	if ( !bBaseline ) {
		CHECK(g_tWork.iRangeProbes + g_tWork.iNextProbes <= (unsigned long long)16 * iCount * iLog);
		CHECK(g_tWork.iWhitespaceVisits <= (unsigned long long)8 * iCount);
		CHECK(g_tWork.iWrapSteps <= (unsigned long long)2 * iCount);
		CHECK(g_tWork.iIndexBuildSteps == (unsigned long long)2 * iCount);
	}
cleanup:
	__xuiTextLayoutClear(&tLayout);
	return !iFailed;
}

static float textLegacyMeasure(const xui_text_shape_t* pShape, int iStart, int iEnd)
{
	float fWidth = 0.0f;
	int i;
	if ( iEnd <= iStart ) return 0.0f;
	for ( i = 0; i < pShape->iClusterCount; i++ ) {
		if ( pShape->pClusters[i].iTextEnd <= iStart ) continue;
		if ( pShape->pClusters[i].iTextStart >= iEnd ) break;
		fWidth += pShape->pClusters[i].fAdvance;
	}
	return fWidth;
}

static int textLegacyNext(const xui_text_shape_t* pShape, int iAt, int iEnd)
{
	int i;
	for ( i = 0; i < pShape->iClusterCount; i++ ) {
		if ( pShape->pClusters[i].iTextEnd <= iAt ) continue;
		if ( pShape->pClusters[i].iTextStart >= iEnd ) break;
		return pShape->pClusters[i].iTextEnd < iEnd ? pShape->pClusters[i].iTextEnd : iEnd;
	}
	return iAt < iEnd ? iAt + 1 : iEnd;
}

static int textRangeRegression(void)
{
	struct xui_text_layout_t tLayout;
	xui_vec2_t tSize;
	int iCase, i, iStart, iEnd;
	int iFailed = 0;
	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.sText = (char*)xrtCalloc(33, 1);
	tLayout.tShape.pClusters = (xui_text_cluster_t*)xrtCalloc(8, sizeof(xui_text_cluster_t));
	CHECK(tLayout.sText != NULL && tLayout.tShape.pClusters != NULL);
	tLayout.iTextSize = 32;
	tLayout.tMetrics.fLineHeight = 12.0f;
	/* Gaps, duplicate and overlapping intervals, zero advances, reversed custom shapes. */
	for ( iCase = 0; iCase < 6; iCase++ ) {
		tLayout.tShape.iClusterCount = iCase == 4 ? 0 : 8;
		for ( i = 0; i < 8; i++ ) {
			int iAt = iCase == 3 ? 7 - i : i;
			tLayout.tShape.pClusters[i].iTextStart = iCase == 2 ? (iAt / 2) * 4 : iAt * 4;
			tLayout.tShape.pClusters[i].iTextEnd = tLayout.tShape.pClusters[i].iTextStart + (iCase == 1 ? 6 : 2);
			tLayout.tShape.pClusters[i].fAdvance = (float)i * 0.25f;
		}
		if ( iCase == 5 ) tLayout.tShape.pClusters[2].fAdvance = -0.5f;
		CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_OK);
		CHECK((tLayout.pClusterAdvances != NULL) == (iCase < 3));
		for ( iStart = 0; iStart <= 32; iStart++ ) {
			for ( iEnd = iStart; iEnd <= 32; iEnd++ ) {
				CHECK(__xuiTextMeasureRange(&tLayout, tLayout.sText + iStart, tLayout.sText + iEnd, &tSize) == XUI_OK);
				CHECK(tSize.fX == textLegacyMeasure(&tLayout.tShape, iStart, iEnd));
				CHECK(tSize.fY == 12.0f);
				CHECK(__xuiTextNextCluster(&tLayout, tLayout.sText + iStart, tLayout.sText + iEnd) - tLayout.sText ==
					textLegacyNext(&tLayout.tShape, iStart, iEnd));
			}
		}
		xrtFree(tLayout.pClusterAdvances);
		tLayout.pClusterAdvances = NULL;
	}
	/* Late small ranges must survive a large prefix; float prefixes lose these quarters. */
	tLayout.tShape.pClusters[0].fAdvance = 16777216.0f;
	tLayout.tShape.pClusters[2].fAdvance = 0.25f;
	CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_OK);
	CHECK(__xuiTextMeasureRange(&tLayout, tLayout.sText + 8, tLayout.sText + 10, &tSize) == XUI_OK);
	CHECK(tSize.fX == 0.25f);
	xrtFree(tLayout.pClusterAdvances);
	tLayout.pClusterAdvances = NULL;
	tLayout.tShape.pClusters[0].fAdvance = 0.000000001f;
	tLayout.tShape.pClusters[1].fAdvance = XUI_LAYOUT_UNBOUNDED;
	CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_OK);
	CHECK(tLayout.pClusterAdvances == NULL);
	CHECK(__xuiTextMeasureRange(&tLayout, tLayout.sText, tLayout.sText + 2, &tSize) == XUI_OK);
	CHECK(tSize.fX == 0.000000001f);
	tLayout.tShape.pClusters[0].fAdvance = XUI_LAYOUT_UNBOUNDED;
	tLayout.tShape.pClusters[1].fAdvance = 0.000000001f;
	CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_OK);
	CHECK(tLayout.pClusterAdvances == NULL);
	CHECK(__xuiTextMeasureRange(&tLayout, tLayout.sText + 4, tLayout.sText + 6, &tSize) == XUI_OK);
	CHECK(tSize.fX == 0.000000001f);
	tLayout.tShape.pClusters[0].fAdvance = 0.25f;
	tLayout.tShape.pClusters[1].fAdvance = 0.25f;
	g_bFailMalloc = 1;
	CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_ERROR_OUT_OF_MEMORY);
	CHECK(tLayout.pClusterAdvances == NULL);
	g_bFailMalloc = 0;
	CHECK(__xuiTextBuildClusterIndex(&tLayout) == XUI_OK);
cleanup:
	g_bFailMalloc = 0;
	__xuiTextLayoutClear(&tLayout);
	__xuiTextLayoutClear(&tLayout);
	return !iFailed;
}

static const char* textLegacyTrim(const char* sBegin, const char* sEnd)
{
	while ( sEnd > sBegin ) {
		const unsigned char* pPrev = (const unsigned char*)sEnd - 1;
		while ( (const char*)pPrev > sBegin && (*pPrev & 0xc0u) == 0x80u ) pPrev--;
		if ( *pPrev != ' ' && *pPrev != '\t' ) break;
		sEnd = (const char*)pPrev;
	}
	return sEnd;
}

static int textWhitespaceRegression(void)
{
	static const char* arrText[] = {"\t  abc \t def    ", "   ", "a\r\n \t b\n  ",
		"\xe4\xb8\xad \xcc\x81\t x", " \x80\x80\t \x80 x ", "a\xc2\xa0" "b \xe3\x80\x80"};
	struct xui_text_layout_t tLayout;
	int iCase, iPass, iStart, iEnd;
	int iFailed = 0;
	memset(&tLayout, 0, sizeof(tLayout));
	for ( iCase = 0; iCase < (int)(sizeof(arrText) / sizeof(arrText[0])); iCase++ ) {
		tLayout.sText = (char*)arrText[iCase];
		tLayout.iTextSize = (int)strlen(tLayout.sText);
		tLayout.iTrimStart = tLayout.iTrimEnd = 0;
		tLayout.iSkipStart = tLayout.iSkipEnd = 0;
		for ( iPass = 0; iPass < 2; iPass++ ) {
			for ( iStart = 0; iStart <= tLayout.iTextSize; iStart++ ) {
				for ( iEnd = iStart; iEnd <= tLayout.iTextSize; iEnd++ ) {
					int iLimit = iPass == 0 ? iEnd : tLayout.iTextSize + iStart - iEnd;
					const char* sStart = tLayout.sText + iStart;
					const char* sEnd = tLayout.sText + iLimit;
					const char* sSkip = sStart;
					while ( sSkip < sEnd && (*sSkip == ' ' || *sSkip == '\t') ) sSkip++;
					CHECK(__xuiTextTrimEndSpaces(&tLayout, sStart, sEnd) == textLegacyTrim(sStart, sEnd));
					CHECK(__xuiTextSkipSpaces(&tLayout, sStart, sEnd) == sSkip);
				}
			}
		}
	}
cleanup:
	return !iFailed;
}

static unsigned int textRandom(unsigned int* pSeed)
{
	*pSeed = *pSeed * 1664525u + 1013904223u;
	return *pSeed;
}

static int textLayoutDifferential(void)
{
	struct xui_text_layout_t tIndexed, tLegacy;
	unsigned int iSeed = 7381;
	int iCase, i, iCount;
	int iFailed = 0;
	memset(&tIndexed, 0, sizeof(tIndexed));
	memset(&tLegacy, 0, sizeof(tLegacy));
	for ( iCase = 0; iCase < 256; iCase++ ) {
		iCount = 1 + (int)(textRandom(&iSeed) % 256u);
		tIndexed.sText = (char*)xrtCalloc((size_t)iCount + 1u, 1);
		tLegacy.sText = (char*)xrtCalloc((size_t)iCount + 1u, 1);
		tIndexed.tShape.pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)iCount, sizeof(xui_text_cluster_t));
		tLegacy.tShape.pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)iCount, sizeof(xui_text_cluster_t));
		CHECK(tIndexed.sText && tLegacy.sText && tIndexed.tShape.pClusters && tLegacy.tShape.pClusters);
		tIndexed.iTextSize = tLegacy.iTextSize = iCount;
		tIndexed.tShape.iClusterCount = tLegacy.tShape.iClusterCount = iCount;
		tIndexed.tMetrics.fLineHeight = 2;
		tIndexed.tDesc.fMaxWidth = (float)(textRandom(&iSeed) % 48u) * 0.25f;
		tIndexed.tDesc.fMaxHeight = iCase % 2 ? 0 : 11;
		tIndexed.tDesc.iWrapMode = (int)(textRandom(&iSeed) % 3u);
		tIndexed.tDesc.fLineGap = 1;
		tIndexed.tDesc.iFlags = iCase % 2 ? XUI_TEXT_ALIGN_CENTER : XUI_TEXT_ALIGN_RIGHT;
		tLegacy.tMetrics = tIndexed.tMetrics;
		tLegacy.tDesc = tIndexed.tDesc;
		for ( i = 0; i < iCount; i++ ) {
			tIndexed.sText[i] = "ab  c\tdef"[(textRandom(&iSeed) >> 16) % 9u];
			tIndexed.tShape.pClusters[i].iTextStart = i;
			tIndexed.tShape.pClusters[i].iTextEnd = i + 1;
			tIndexed.tShape.pClusters[i].fAdvance = (float)((textRandom(&iSeed) >> 16) % 13u) * 0.25f;
		}
		memcpy(tLegacy.sText, tIndexed.sText, (size_t)iCount);
		memcpy(tLegacy.tShape.pClusters, tIndexed.tShape.pClusters, (size_t)iCount * sizeof(xui_text_cluster_t));
		CHECK(__xuiTextLayoutBuild(&tIndexed) == XUI_OK);
		/* The unindexed path is the original linear range/next-cluster implementation. */
		CHECK(__xuiTextLayoutParagraph(&tLegacy, tLegacy.sText, tLegacy.sText + iCount, XUI_TEXT_BREAK_END) == XUI_OK);
		__xuiTextLayoutAlignLines(&tLegacy);
		CHECK(tIndexed.iLineCount == tLegacy.iLineCount && tIndexed.bTruncated == tLegacy.bTruncated);
		CHECK(tIndexed.tSize.fX == tLegacy.tSize.fX && tIndexed.tSize.fY == tLegacy.tSize.fY);
		CHECK(memcmp(tIndexed.pLines, tLegacy.pLines, (size_t)tIndexed.iLineCount * sizeof(xui_text_line_t)) == 0);
		__xuiTextLayoutClear(&tIndexed);
		__xuiTextLayoutClear(&tLegacy);
	}
cleanup:
	__xuiTextLayoutClear(&tIndexed);
	__xuiTextLayoutClear(&tLegacy);
	return !iFailed;
}

int main(int argc, char** argv)
{
	int arrSizes[] = {256, 1024, 4096, 16384, 65536};
	int bBaseline = argc > 1 && strcmp(argv[1], "--baseline") == 0;
	int i, iPattern;
	if ( !textRangeRegression() || !textWhitespaceRegression() || !textLayoutDifferential() ) return 1;
	for ( i = 0; i < (bBaseline ? 3 : 5); i++ ) {
		for ( iPattern = 0; iPattern < 5; iPattern++ ) {
			if ( !textScale(arrSizes[i], iPattern, bBaseline) ) return 1;
		}
	}
	printf("xui_text_scale_test %s\n", bBaseline ? "baseline recorded" : "passed");
	return 0;
}
