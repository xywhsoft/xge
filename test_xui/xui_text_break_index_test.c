#include "xui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct break_work_t {
	unsigned long long iDecode, iMap, iProperties;
	unsigned long long iRangeProbes, iNextProbes, iWhitespaceVisits, iWrapSteps, iIndexBuildSteps;
} break_work_t;

static break_work_t g_tWork;
static int g_iFailAfter = -1;
static void* g_pLive[32];
static int g_iLive;

static int breakFail(void)
{
	return g_iFailAfter >= 0 && g_iFailAfter-- == 0;
}

static void* breakMalloc(size_t n)
{
	void* p = breakFail() ? NULL : xrtMalloc(n);
	if ( p ) { if ( g_iLive == 32 ) abort(); g_pLive[g_iLive++] = p; }
	return p;
}

static void* breakCalloc(size_t n, size_t s)
{
	void* p = breakFail() ? NULL : xrtCalloc(n, s);
	if ( p ) { if ( g_iLive == 32 ) abort(); g_pLive[g_iLive++] = p; }
	return p;
}

static void* breakRealloc(void* p, size_t n)
{
	int i;
	void* q;
	if ( breakFail() ) return NULL;
	for ( i = 0; i < g_iLive && g_pLive[i] != p; i++ ) {}
	q = xrtRealloc(p, n);
	if ( q ) {
		if ( i == g_iLive ) { if ( g_iLive == 32 ) abort(); g_iLive++; }
		g_pLive[i] = q;
	}
	return q;
}

static void breakFree(void* p)
{
	int i;
	for ( i = 0; i < g_iLive; i++ ) if ( g_pLive[i] == p ) { g_pLive[i] = g_pLive[--g_iLive]; break; }
	xrtFree(p);
}

#define xrtMalloc breakMalloc
#define xrtCalloc breakCalloc
#define xrtRealloc breakRealloc
#define xrtFree breakFree
#define XUI_TEXT_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#define XUI_TEXT_BREAK_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#include "../src/xui_text.c"
#undef xrtMalloc
#undef xrtCalloc
#undef xrtRealloc
#undef xrtFree

#define CHECK(expr) do { if ( !(expr) ) { printf("break index failure %d: %s\n", __LINE__, #expr); return 0; } } while (0)

static int breakConformance(const char* sPath, int bGrapheme)
{
	FILE* pFile = fopen(sPath, "rb");
	char sLine[16384], sText[16384], sBreaks[16384];
	int iEnds[4096], iExpected[4097];
	uint32 iScalars[4096];
	int iTotal = 0, iKnown = 0, iLine = 0;
	CHECK(pFile != NULL);
	while ( fgets(sLine, sizeof(sLine), pFile) != NULL ) {
		char* p = sLine;
		int iCount = 0, iBytes = 0, i, iMismatch = 0;
		iLine++;
		if ( *p == '#' || *p == '\n' || *p == '\r' ) continue;
		while ( *p && *p != '#' && *p != '\n' && *p != '\r' ) {
			if ( *p == ' ' || *p == '\t' ) { p++; continue; }
			if ( (unsigned char)p[0] == 0xc3u && ((unsigned char)p[1] == 0xb7u || (unsigned char)p[1] == 0x97u) ) {
				iExpected[iCount] = (unsigned char)p[1] == 0xb7u;
				p += 2;
			} else {
				char* sNext;
				uint32 iScalar = (uint32)strtoul(p, &sNext, 16);
				CHECK(sNext != p && iCount < 4096 && iBytes + 4 <= (int)sizeof(sText));
				iBytes += (int)xrtUtf8Encode(iScalar, sText + iBytes);
				iScalars[iCount] = iScalar;
				iEnds[iCount++] = iBytes;
				p = sNext;
			}
		}
		CHECK(iCount > 0);
		if ( bGrapheme ) set_graphemebreaks(sText, (size_t)iBytes, sBreaks, __xuiTextBreakDecode);
		else __xuiUbLineBreaks(sText, (size_t)iBytes, "-strict", LBOT_PER_CODE_UNIT, sBreaks, __xuiTextBreakDecode);
		CHECK(iExpected[0] == bGrapheme);
		for ( i = 0; i < iCount; i++ ) {
			int iActual = bGrapheme ? sBreaks[iEnds[i] - 1] == GRAPHEMEBREAK_BREAK :
				sBreaks[iEnds[i] - 1] != LINEBREAK_NOBREAK;
			int j, iStart = i ? iEnds[i - 1] : 0;
			if ( iActual != iExpected[i + 1] ) iMismatch++;
			for ( j = iStart; j + 1 < iEnds[i]; j++ ) CHECK(sBreaks[j] ==
				(bGrapheme ? GRAPHEMEBREAK_INSIDEACHAR : LINEBREAK_INSIDEACHAR));
		}
		/* Upstream 7.0's one documented XX -> AL exception. Assert it exactly,
		 * including its sequence and mismatch count, rather than skipping a row. */
		if ( !bGrapheme && iCount == 2 && iScalars[0] == 0x1f02cu && iScalars[1] == 0x1f3ffu ) {
			CHECK(iMismatch == 1 && iExpected[1] == 0 && sBreaks[iEnds[0] - 1] == LINEBREAK_ALLOWBREAK);
			iKnown++;
		} else if ( iMismatch ) {
			printf("conformance mismatch %s:%d count=%d\n", sPath, iLine, iMismatch);
			fclose(pFile);
			return 0;
		}
		iTotal++;
	}
	fclose(pFile);
	CHECK(iTotal == (bGrapheme ? 1187 : 7654) && iKnown == (bGrapheme ? 0 : 1));
	printf("%s: %d passed, %d explicitly verified upstream exception\n", bGrapheme ? "UAX29 15.1" : "UAX14 15.0", iTotal - iKnown, iKnown);
	return 1;
}

static int breakPrepare(struct xui_text_layout_t* pLayout, const char* sPattern, int iRepeats)
{
	int i, iLength = (int)strlen(sPattern), iBytes = iLength * iRepeats, iCount = 0;
	size_t iAt = 0;
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->sText = (char*)xrtMalloc((size_t)iBytes + 1u);
	pLayout->tShape.pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)iBytes, sizeof(xui_text_cluster_t));
	CHECK(pLayout->sText && pLayout->tShape.pClusters);
	for ( i = 0; i < iRepeats; i++ ) memcpy(pLayout->sText + i * iLength, sPattern, (size_t)iLength);
	pLayout->sText[iBytes] = 0;
	pLayout->iTextSize = iBytes;
	while ( iAt < (size_t)iBytes ) {
		xui_text_cluster_t* p = &pLayout->tShape.pClusters[iCount++];
		utf32_t iScalar;
		p->iTextStart = (int)iAt;
		iScalar = __xuiTextBreakDecode(pLayout->sText, (size_t)iBytes, &iAt);
		p->iTextEnd = (int)iAt;
		/* Supply already projected advances to the isolated wrapping benchmark. */
		p->fAdvance = iScalar == 0xadu || iScalar == 0x200bu || iScalar == 0x2060u || iScalar == 0xfeffu ? 0 : 1;
		pLayout->tShape.fWidth += p->fAdvance;
	}
	pLayout->tShape.iClusterCount = iCount;
	pLayout->tMetrics.fLineHeight = 2;
	pLayout->fHyphenWidth = 1;
	pLayout->tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	pLayout->tDesc.fMaxWidth = 7;
	return 1;
}

static int breakScale(void)
{
	static const char* const arrPatterns[] = {
		"abcdef", "\xe4\xb8\xad\xef\xbc\x88\xe6\x96\x87\xef\xbc\x89",
		"A\xc2\xa0" "B\xe2\x81\xa0", "\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9",
		"e\xcc\x81", "\xcc\x81", "\xf0\x9f\x87\xa8", "(               ",
		"ab\xc2\xad" "cd\xe2\x80\x8b", "\xd7\x90\xd7\x91 \xd8\xa8\xd8\xaa ",
		"\xc2\xad\xe2\x81\xa0", "x\xe2\x80\xa8"
	};
	int i, n;
	for ( n = 256; n <= 65536; n *= 4 ) {
		for ( i = 0; i < (int)(sizeof(arrPatterns) / sizeof(arrPatterns[0])); i++ ) {
			struct xui_text_layout_t tLayout;
			unsigned long long iWork, iLimit;
			int iLog = 1, j, iAt = 0;
			CHECK(breakPrepare(&tLayout, arrPatterns[i], n));
			memset(&g_tWork, 0, sizeof(g_tWork));
			CHECK(__xuiTextLayoutBuild(&tLayout) == XUI_OK);
			for ( j = tLayout.tShape.iClusterCount; j > 1; j /= 2 ) iLog++;
			CHECK(g_tWork.iDecode <= (unsigned long long)4 * tLayout.tShape.iClusterCount + 2u);
			CHECK(g_tWork.iMap <= (unsigned long long)2 * tLayout.iTextSize);
			CHECK(g_tWork.iProperties == (unsigned long long)tLayout.tShape.iClusterCount);
			CHECK(g_tWork.iWrapSteps <= (unsigned long long)3 * tLayout.tShape.iClusterCount);
			iWork = g_tWork.iRangeProbes + g_tWork.iNextProbes;
			iLimit = (unsigned long long)20 * tLayout.tShape.iClusterCount * iLog;
			CHECK(iWork <= iLimit);
			for ( j = 0; j < tLayout.iLineCount; j++ ) {
				xui_text_line_t* p = &tLayout.pLines[j];
				int iEnd = p->iTextOffset + p->iTextSize;
				CHECK(p->iTextOffset >= iAt && iEnd <= tLayout.iTextSize);
				if ( p->iTextOffset > 0 ) CHECK(tLayout.pBreaks[p->iTextOffset] & XUI_LB_GRAPHEME);
				if ( iEnd > 0 ) CHECK(tLayout.pBreaks[iEnd] & XUI_LB_GRAPHEME);
				iAt = iEnd;
			}
			printf("break scale pattern=%d repeats=%d bytes=%d clusters=%d decode=%llu map=%llu wrap=%llu probes=%llu\n",
				i, n, tLayout.iTextSize, tLayout.tShape.iClusterCount, g_tWork.iDecode, g_tWork.iMap, g_tWork.iWrapSteps, iWork);
			__xuiTextLayoutClear(&tLayout);
			CHECK(g_iLive == 0);
		}
	}
	return 1;
}

static int breakFailures(void)
{
	int i, iFailed = 0, iSucceeded = 0;
	for ( i = 0; i < 24; i++ ) {
		struct xui_text_layout_t tLayout;
		int iRet;
		CHECK(breakPrepare(&tLayout, "abcd \xe4\xb8\xad\xc2\xad", 256));
		g_iFailAfter = i;
		iRet = __xuiTextLayoutBuild(&tLayout);
		g_iFailAfter = -1;
		if ( iRet == XUI_OK ) iSucceeded++;
		else { CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY); iFailed++; }
		__xuiTextLayoutClearGeometry(&tLayout);
		CHECK(g_iLive == 0 && tLayout.sText != NULL);
		CHECK(__xuiTextLayoutBuild(&tLayout) == XUI_OK);
		__xuiTextLayoutClear(&tLayout);
		CHECK(g_iLive == 0);
	}
	CHECK(iFailed > 4 && iSucceeded > 0);
	printf("break allocation: %d injected failures, %d successes, every rebuild retried, no outstanding allocations\n", iFailed, iSucceeded);
	return 1;
}

static int breakMalformed(void)
{
	static const char arrText[] = {'a', (char)0xf0, (char)0x9f, 0, (char)0x80, (char)0xc0, (char)0xaf};
	unsigned char arrMap[sizeof(arrText) + 1] = {0};
	int i;
	CHECK(__xuiTextBreakMap(arrText, sizeof(arrText), arrMap) == XUI_OK);
	for ( i = 1; i <= (int)sizeof(arrText); i++ ) CHECK(arrMap[i] & XUI_LB_GRAPHEME);
	CHECK(g_iLive == 0);
	return 1;
}

static int breakDisplay(void)
{
	struct xui_text_layout_t tLayout;
	xui_text_line_t tTooLarge = {0};
	const char* sDisplay = NULL;
	int iSize = 0;
	tTooLarge.iTextSize = INT_MAX;
	CHECK(__xuiTextLayoutDisplayLine(&tLayout, &tTooLarge, &sDisplay, &iSize) == XUI_ERROR_OUT_OF_MEMORY);
	CHECK(g_iLive == 0);
	CHECK(breakPrepare(&tLayout, "ab\xc2\xad" "cd", 1));
	tLayout.tDesc.fMaxWidth = 3;
	CHECK(__xuiTextLayoutBuild(&tLayout) == XUI_OK && tLayout.iLineCount == 2);
	g_iFailAfter = 0;
	CHECK(__xuiTextLayoutDisplayLine(&tLayout, &tLayout.pLines[0], &sDisplay, &iSize) == XUI_ERROR_OUT_OF_MEMORY);
	g_iFailAfter = -1;
	CHECK(__xuiTextLayoutDisplayLine(&tLayout, &tLayout.pLines[0], &sDisplay, &iSize) == XUI_OK);
	CHECK(iSize == 3 && strcmp(sDisplay, "ab-") == 0);
	CHECK(__xuiTextLayoutDisplayLine(&tLayout, &tLayout.pLines[1], &sDisplay, &iSize) == XUI_OK);
	CHECK(iSize == 2 && strcmp(sDisplay, "cd") == 0);
	CHECK(strcmp(tLayout.sText, "ab\xc2\xad" "cd") == 0);
	__xuiTextLayoutClear(&tLayout);
	breakFree(tLayout.pScratch);
	CHECK(g_iLive == 0);

	/* Full projection (including invisible-only proxy input) has its own test. */
	return 1;
}

int main(void)
{
	if ( !breakConformance("test_xui/data/libunibreak/LineBreakTest.txt", 0) ||
	     !breakConformance("test_xui/data/libunibreak/GraphemeBreakTest.txt", 1) ||
	     !breakMalformed() || !breakDisplay() || !breakFailures() || !breakScale() ) return 1;
	puts("xui_text_break_index_test passed");
	return 0;
}
