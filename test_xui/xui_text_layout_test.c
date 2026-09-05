#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if ( !(expr) ) { \
	printf("text layout failure at line %d: %s\n", __LINE__, #expr); \
	iFailed = 1; goto cleanup; } } while ( 0 )

typedef struct text_case_t {
	const char* sText;
	float fWidth;
	int iMode;
	int iLineCount;
	const char* arrLines[4];
	int arrOffsets[4];
} text_case_t;

static int textLineEquals(xui_text_layout pLayout, int iLine, const char* sExpected, int iOffset)
{
	xui_text_line_t tLine;
	int iSize = (int)strlen(sExpected);
	return xuiTextLayoutGetLine(pLayout, iLine, &tLine) == XUI_OK &&
		tLine.iTextOffset == iOffset && tLine.iTextSize == iSize &&
		memcmp(xuiTextLayoutGetText(pLayout) + iOffset, sExpected, (size_t)iSize) == 0;
}

static int g_bFailMeasure;
static int (*g_onMeasure)(xui_proxy, xui_font, const char*, xui_vec2_t*);

static int textMeasureMaybeFail(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	return g_bFailMeasure ? XUI_ERROR_BACKEND_FAILED : g_onMeasure(pProxy, pFont, sText, pSize);
}

int main(void)
{
	static const text_case_t arrCases[] = {
		{"", 3, XUI_TEXT_WRAP_WORD, 0, {NULL}, {0}},
		{" \t ", 3, XUI_TEXT_WRAP_WORD, 1, {""}, {0}},
		{"ab cd", 4, XUI_TEXT_WRAP_WORD, 2, {"ab", "cd"}, {0, 3}},
		{"abcdef", 2, XUI_TEXT_WRAP_WORD, 3, {"ab", "cd", "ef"}, {0, 2, 4}},
		{"ab cd", 4, XUI_TEXT_WRAP_CHAR, 2, {"ab c", "d"}, {0, 4}},
		{"ab\t  cd   ", 3, XUI_TEXT_WRAP_WORD, 2, {"ab", "cd"}, {0, 5}},
		{"ab\r\n\nc\r", 0, XUI_TEXT_WRAP_WORD, 4, {"ab", "", "c", ""}, {0, 4, 5, 7}},
		{"abcdef", 1, XUI_TEXT_WRAP_NONE, 1, {"abcdef"}, {0}},
		{"abc  ", 0, XUI_TEXT_WRAP_CHAR, 1, {"abc"}, {0}},
		{"ab", 0.5f, XUI_TEXT_WRAP_CHAR, 3, {"a", "b", ""}, {0, 1, 2}}
	};
	/* Independent expected grapheme spans, in logical order, not a bidi renderer test. */
	static const char* arrClusters[] = {
		"\xe4\xb8\xad", "\xe6\x96\x87", "\xe3\x81\x82", "\xed\x95\x9c",
		"e\xcc\x81", "\xe1\x84\x80\xe1\x85\xa1\xe1\x86\xa8",
		"\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d"
		"\xf0\x9f\x91\xa7\xe2\x80\x8d\xf0\x9f\x91\xa6",
		"\xf0\x9f\x87\xa8\xf0\x9f\x87\xb3", "\xd7\x90\xd6\xb0", "\xd7\x91",
		"\xd8\xa8\xd9\x8e", "\xd8\xaa"
	};
	xui_test_proxy_state_t tState;
	xui_context pContext = NULL;
	xui_font pFont = NULL;
	xui_text_layout pLayout = NULL;
	xui_text_layout_desc_t tDesc;
	xui_text_line_t tLine;
	xui_vec2_t tSize;
	char sMixed[256];
	int i, j, iOffset, iMode;
	int iFailed = 0;
	xuiTestProxyInit(&tState);
	/* Exercise the shared grapheme fallback as well as the proxy's scalar shaping. */
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK);
	CHECK(tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, NULL, 2.0f, 0) == XUI_OK);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "";
	tDesc.iTextSize = -1;
	tDesc.fLineGap = 1;
	tDesc.fParagraphGap = 3;
	CHECK(xuiTextLayoutCreate(pContext, &pLayout, &tDesc) == XUI_OK);
	for ( iMode = 0; iMode < 2; iMode++ ) {
		if ( iMode == 1 ) {
			xuiTextLayoutDestroy(pLayout);
			pLayout = NULL;
			xuiDestroy(pContext);
			pContext = NULL;
			tState.tProxy.textShape = NULL;
			g_onMeasure = tState.tProxy.textMeasure;
			tState.tProxy.textMeasure = textMeasureMaybeFail;
			CHECK(xuiCreate(&pContext) == XUI_OK);
			CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK);
			CHECK(xuiTextLayoutCreate(pContext, &pLayout, &tDesc) == XUI_OK);
		}
		for ( i = 0; i < (int)(sizeof(arrCases) / sizeof(arrCases[0])); i++ ) {
			const text_case_t* pCase = &arrCases[i];
			float fY = 0;
			tDesc.sText = pCase->sText;
			tDesc.fMaxWidth = pCase->fWidth;
			tDesc.iWrapMode = pCase->iMode;
			tDesc.iFlags = i % 2 ? XUI_TEXT_ALIGN_CENTER : XUI_TEXT_ALIGN_RIGHT;
			CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_OK);
			CHECK(xuiTextLayoutGetLineCount(pLayout) == pCase->iLineCount);
			CHECK(!xuiTextLayoutGetTruncated(pLayout));
			tSize = xuiTextLayoutGetSize(pLayout);
			for ( j = 0; j < pCase->iLineCount; j++ ) {
				int iBreak = j == pCase->iLineCount - 1 ? XUI_TEXT_BREAK_END :
					(i == 6 ? XUI_TEXT_BREAK_NEWLINE : XUI_TEXT_BREAK_WRAP);
				CHECK(textLineEquals(pLayout, j, pCase->arrLines[j], pCase->arrOffsets[j]));
				CHECK(xuiTextLayoutGetLine(pLayout, j, &tLine) == XUI_OK);
				CHECK(tLine.iBreakType == iBreak && tLine.fY == fY && tLine.fH == 2.0f);
				CHECK(tLine.fW == (float)strlen(pCase->arrLines[j]));
				CHECK(tLine.fX == (tSize.fX - tLine.fW) * (i % 2 ? 0.5f : 1.0f));
				fY += 2 + (iBreak == XUI_TEXT_BREAK_NEWLINE ? 3 : 1);
			}
			CHECK(tSize.fY == (pCase->iLineCount > 0 ? fY - 1 : 0));
		}
	}
	CHECK(xuiTextLayoutGetLine(pLayout, -1, &tLine) == XUI_ERROR_INVALID_ARGUMENT);
	CHECK(xuiTextLayoutGetLine(pLayout, 3, &tLine) == XUI_ERROR_INVALID_ARGUMENT);
	tDesc.sText = "abcdefgh";
	tDesc.fMaxWidth = 2;
	tDesc.fMaxHeight = 5;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_OK);
	CHECK(xuiTextLayoutGetTruncated(pLayout) && xuiTextLayoutGetLineCount(pLayout) == 2);
	CHECK(textLineEquals(pLayout, 1, "cd", 2));
	tDesc.sText = "abcdefgh";
	tDesc.iTextSize = 3;
	tDesc.fMaxWidth = 10;
	tDesc.fMaxHeight = 0;
	CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_OK);
	CHECK(textLineEquals(pLayout, 0, "abc", 0));
	CHECK(strcmp(xuiTextLayoutGetText(pLayout), "abc") == 0);
	CHECK(!xuiTextLayoutGetTruncated(pLayout));

	iOffset = 0;
	for ( i = 0; i < (int)(sizeof(arrClusters) / sizeof(arrClusters[0])); i++ ) {
		int iLength = (int)strlen(arrClusters[i]);
		CHECK(iOffset + iLength < (int)sizeof(sMixed));
		memcpy(sMixed + iOffset, arrClusters[i], (size_t)iLength);
		iOffset += iLength;
	}
	sMixed[iOffset] = 0;
	tDesc.sText = sMixed;
	tDesc.iTextSize = -1;
	tDesc.fMaxWidth = 0.5f;
	for ( iMode = XUI_TEXT_WRAP_WORD; iMode <= XUI_TEXT_WRAP_CHAR; iMode++ ) {
		tDesc.iWrapMode = iMode;
		CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_OK);
		CHECK(xuiTextLayoutGetLineCount(pLayout) == (int)(sizeof(arrClusters) / sizeof(arrClusters[0])) + 1);
		iOffset = 0;
		for ( i = 0; i < (int)(sizeof(arrClusters) / sizeof(arrClusters[0])); i++ ) {
			CHECK(textLineEquals(pLayout, i, arrClusters[i], iOffset));
			iOffset += (int)strlen(arrClusters[i]);
		}
		CHECK(textLineEquals(pLayout, i, "", iOffset));
	}

	/* A failed shape/reset must not leave the old index or whitespace cache reachable. */
	g_bFailMeasure = 1;
	CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_ERROR_BACKEND_FAILED);
	CHECK(xuiTextLayoutGetLineCount(pLayout) == 0);
	g_bFailMeasure = 0;
	tDesc.sText = "x \t y";
	tDesc.fMaxWidth = 2;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	CHECK(xuiTextLayoutReset(pLayout, &tDesc) == XUI_OK);
	CHECK(xuiTextLayoutGetLineCount(pLayout) == 2);
	CHECK(textLineEquals(pLayout, 1, "y", 4));
	CHECK(xuiTextMeasureLayout(pContext, &tDesc, &tSize) == XUI_OK);
	CHECK(tSize.fX == 1 && tSize.fY == 5);

cleanup:
	if ( pLayout != NULL ) xuiTextLayoutDestroy(pLayout);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( !iFailed ) printf("xui_text_layout_test passed\n");
	return iFailed;
}
