#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define ZH "\xe4\xb8\xad"
#define WEN "\xe6\x96\x87"
#define ZI "\xe5\xad\x97"
#define OPEN "\xef\xbc\x88"
#define CLOSE "\xef\xbc\x89"
#define COMMA "\xef\xbc\x8c"
#define NBSP "\xc2\xa0"
#define WJ "\xe2\x81\xa0"
#define ZWSP "\xe2\x80\x8b"
#define SHY "\xc2\xad"
#define ACCENT "e\xcc\x81"
#define FAMILY "\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7"
#define HEBREW "\xd7\x90\xd7\x91"
#define ARABIC "\xd8\xa8\xd8\xaa"

typedef struct break_case_t {
	const char* sName;
	const char* sText;
	float fWidth;
	int iMode;
	int iCount;
	const char* sLines[5];
	const char* sDraw[5];
} break_case_t;

static const break_case_t g_tCases[] = {
	{"CJK opening", ZH OPEN WEN CLOSE ZI, 2, XUI_TEXT_WRAP_WORD, 3,
		{ZH, OPEN WEN CLOSE, ZI}, {ZH, OPEN WEN CLOSE, ZI}},
	{"CJK closing", ZH WEN COMMA ZI, 2, XUI_TEXT_WRAP_WORD, 3,
		{ZH, WEN COMMA, ZI}, {ZH, WEN COMMA, ZI}},
	{"Japanese", "\xe3\x81\x82\xe3\x80\x8c\xe3\x81\x84\xe3\x81\x86\xe3\x80\x8d\xe3\x81\x88", 1, XUI_TEXT_WRAP_WORD, 4,
		{"\xe3\x81\x82", "\xe3\x80\x8c\xe3\x81\x84", "\xe3\x81\x86\xe3\x80\x8d", "\xe3\x81\x88"},
		{"\xe3\x81\x82", "\xe3\x80\x8c\xe3\x81\x84", "\xe3\x81\x86\xe3\x80\x8d", "\xe3\x81\x88"}},
	{"Korean", "\xea\xb0\x80\xeb\x82\x98\xeb\x8b\xa4\xeb\x9d\xbc", 2, XUI_TEXT_WRAP_WORD, 2,
		{"\xea\xb0\x80\xeb\x82\x98", "\xeb\x8b\xa4\xeb\x9d\xbc"}, {"\xea\xb0\x80\xeb\x82\x98", "\xeb\x8b\xa4\xeb\x9d\xbc"}},
	{"NBSP", "A" NBSP "B", 1, XUI_TEXT_WRAP_WORD, 1, {"A" NBSP "B"}, {"A" NBSP "B"}},
	{"word joiner", "A" WJ "B", 1, XUI_TEXT_WRAP_WORD, 1, {"A" WJ "B"}, {"AB"}},
	{"ZWSP", "ab" ZWSP "cd", 2, XUI_TEXT_WRAP_WORD, 2, {"ab" ZWSP, "cd"}, {"ab", "cd"}},
	{"soft hyphen", "ab" SHY "cd", 3, XUI_TEXT_WRAP_WORD, 2, {"ab" SHY, "cd"}, {"ab-", "cd"}},
	{"soft hyphen fits", "ab" SHY "cd", 10, XUI_TEXT_WRAP_WORD, 1, {"ab" SHY "cd"}, {"abcd"}},
	{"soft hyphen width", "ab" SHY "cd", 2, XUI_TEXT_WRAP_WORD, 3, {"a", "b" SHY, "cd"}, {"a", "b-", "cd"}},
	{"SHY before spaces", "ab" SHY "   cd", 3, XUI_TEXT_WRAP_WORD, 2, {"ab" SHY, "cd"}, {"ab", "cd"}},
	{"narrow NBSP", "A\xe2\x80\xaf" "B", 1, XUI_TEXT_WRAP_WORD, 1, {"A\xe2\x80\xaf" "B"}, {"A\xe2\x80\xaf" "B"}},
	{"BOM glue", "A\xef\xbb\xbf" "B", 1, XUI_TEXT_WRAP_WORD, 1, {"A\xef\xbb\xbf" "B"}, {"AB"}},
	{"SHY plus WJ", "A" SHY WJ "B", 1, XUI_TEXT_WRAP_WORD, 1, {"A" SHY WJ "B"}, {"AB"}},
	{"space trimming", "a   b", 1, XUI_TEXT_WRAP_WORD, 2, {"a", "b"}, {"a", "b"}},
	{"emergency", "abcdef", 2, XUI_TEXT_WRAP_WORD, 3, {"ab", "cd", "ef"}, {"ab", "cd", "ef"}},
	{"emoji ZWJ", FAMILY "X", 1, XUI_TEXT_WRAP_WORD, 2, {FAMILY, "X"}, {FAMILY, "X"}},
	{"combining", ACCENT "x", 1, XUI_TEXT_WRAP_WORD, 2, {ACCENT, "x"}, {ACCENT, "x"}},
	{"RTL logical", "ab " HEBREW " " ARABIC, 3, XUI_TEXT_WRAP_WORD, 3, {"ab", HEBREW, ARABIC}, {"ab", HEBREW, ARABIC}},
	{"RTL combining", "A \xd7\x90\xd6\xb0\xd7\x91 \xd8\xa8\xd9\x8e\xd8\xaa Z", 3, XUI_TEXT_WRAP_WORD, 4,
		{"A", "\xd7\x90\xd6\xb0\xd7\x91", "\xd8\xa8\xd9\x8e\xd8\xaa", "Z"},
		{"A", "\xd7\x90\xd6\xb0\xd7\x91", "\xd8\xa8\xd9\x8e\xd8\xaa", "Z"}},
	{"Japanese CJ", "\xe3\x81\x82\xe3\x82\x83\xe3\x81\x84", 1, XUI_TEXT_WRAP_WORD, 2,
		{"\xe3\x81\x82\xe3\x82\x83", "\xe3\x81\x84"}, {"\xe3\x81\x82\xe3\x82\x83", "\xe3\x81\x84"}},
	{"CHAR punctuation", ZH OPEN WEN CLOSE, 2, XUI_TEXT_WRAP_CHAR, 2, {ZH OPEN, WEN CLOSE}, {ZH OPEN, WEN CLOSE}},
	{"CHAR glue", "A" WJ "B", 1, XUI_TEXT_WRAP_CHAR, 2, {"A" WJ, "B"}, {"A", "B"}},
	{"CHAR ZWJ", FAMILY "X", 1, XUI_TEXT_WRAP_CHAR, 2, {FAMILY, "X"}, {FAMILY, "X"}},
	{"CHAR SHY", "ab" SHY "cd", 2, XUI_TEXT_WRAP_CHAR, 2, {"ab" SHY, "cd"}, {"ab", "cd"}},
	{"mandatory", "a\xe2\x80\xa8" "b\xc2\x85" "c", 10, XUI_TEXT_WRAP_WORD, 3, {"a", "b", "c"}, {"a", "b", "c"}},
	{"NONE mandatory", "a\v" "b\f" "c\xe2\x80\xa9" "d", 1, XUI_TEXT_WRAP_NONE, 4, {"a", "b", "c", "d"}, {"a", "b", "c", "d"}},
	{"CHAR mandatory", "a\xe2\x80\xa8" "b\xc2\x85" "c", 10, XUI_TEXT_WRAP_CHAR, 3, {"a", "b", "c"}, {"a", "b", "c"}},
	{"NONE SHY", "ab" SHY "cd", 1, XUI_TEXT_WRAP_NONE, 1, {"ab" SHY "cd"}, {"abcd"}}
};

static const break_case_t* g_pCase;
static int g_iDraw, g_iDrawFailed;

static int breakMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	const unsigned char* p = (const unsigned char*)sText;
	pSize->fX = 0;
	pSize->fY = 2;
	for ( ; *p; p++ ) if ( (*p & 0xc0u) != 0x80u ) pSize->fX++;
	return XUI_OK;
}

static int breakDraw(xui_proxy pProxy, xui_surface pTarget, xui_font pFont,
	const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( g_iDraw >= g_pCase->iCount || strcmp(sText, g_pCase->sDraw[g_iDraw]) != 0 ) g_iDrawFailed = 1;
	g_iDraw++;
	return XUI_OK;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext = NULL;
	xui_font pFont = NULL;
	xui_text_layout pLayout = NULL;
	xui_text_layout_desc_t tDesc;
	xui_rect_t tRect = {0, 0, 100, 100};
	int i, j, iBackend, iFailed = 0, iCases = 0;
	xuiTestProxyInit(&tState);
	tState.tProxy.textDraw = breakDraw;
	tState.tProxy.textMeasure = breakMeasure;
	if ( tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, NULL, 2, 0) != XUI_OK ) return 1;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iTextSize = -1;
	for ( iBackend = 0; iBackend < 2; iBackend++ ) {
		if ( iBackend ) tState.tProxy.textShape = NULL;
		if ( xuiCreate(&pContext) != XUI_OK || xuiSetProxy(pContext, &tState.tProxy) != XUI_OK ) return 1;
		for ( i = 0; i < (int)(sizeof(g_tCases) / sizeof(g_tCases[0])); i++ ) {
			int iOk = 1, iAt = 0;
			const char* sOwned;
			xui_vec2_t tBefore, tAfter;
			g_pCase = &g_tCases[i];
			tDesc.sText = g_pCase->sText;
			tDesc.fMaxWidth = g_pCase->fWidth;
			tDesc.iWrapMode = g_pCase->iMode;
			if ( xuiTextLayoutCreate(pContext, &pLayout, &tDesc) != XUI_OK ) return 1;
			sOwned = xuiTextLayoutGetText(pLayout);
			tBefore = xuiTextLayoutGetSize(pLayout);
			if ( xuiTextLayoutGetLineCount(pLayout) != g_pCase->iCount || strcmp(xuiTextLayoutGetText(pLayout), tDesc.sText) != 0 ) iOk = 0;
			for ( j = 0; j < g_pCase->iCount; j++ ) {
				xui_text_line_t tLine;
				xui_vec2_t tExpectedSize;
				int iLen = (int)strlen(g_pCase->sLines[j]);
				const char* sExpected = strstr(tDesc.sText + iAt, g_pCase->sLines[j]);
				if ( !sExpected || xuiTextLayoutGetLine(pLayout, j, &tLine) != XUI_OK ||
				     tLine.iTextOffset != (int)(sExpected - tDesc.sText) || tLine.iTextSize != iLen ||
				     memcmp(xuiTextLayoutGetText(pLayout) + tLine.iTextOffset, g_pCase->sLines[j], (size_t)iLen) != 0 ) iOk = 0;
				breakMeasure(&tState.tProxy, pFont, g_pCase->sDraw[j], &tExpectedSize);
				if ( xuiTextLayoutGetLine(pLayout, j, &tLine) == XUI_OK && tLine.fW != tExpectedSize.fX ) iOk = 0;
				if ( sExpected ) iAt = (int)(sExpected - tDesc.sText) + iLen;
			}
			g_iDraw = g_iDrawFailed = 0;
			if ( xuiTextLayoutDraw(pLayout, (xui_surface)&tState, tRect, 0, 0) != XUI_OK ||
			     g_iDraw != g_pCase->iCount || g_iDrawFailed ) iOk = 0;
			if ( xuiSetVirtualDpi(pContext, i % 2 ? 1.25f : 1.75f) != XUI_OK ) return 1;
			tAfter = xuiTextLayoutGetSize(pLayout);
			if ( xuiTextLayoutGetText(pLayout) != sOwned || tAfter.fX != tBefore.fX || tAfter.fY != tBefore.fY ||
			     xuiTextLayoutGetLineCount(pLayout) != g_pCase->iCount ) iOk = 0;
			g_iDraw = g_iDrawFailed = 0;
			if ( xuiTextLayoutDraw(pLayout, (xui_surface)&tState, tRect, 0, 0) != XUI_OK ||
			     g_iDraw != g_pCase->iCount || g_iDrawFailed ) iOk = 0;
			if ( !iOk ) { printf("FAIL backend=%d %s lines=%d expected=%d\n", iBackend, g_pCase->sName, xuiTextLayoutGetLineCount(pLayout), g_pCase->iCount); iFailed++; }
			iCases++;
			xuiTextLayoutDestroy(pLayout);
			pLayout = NULL;
		}
		xuiDestroy(pContext);
		pContext = NULL;
	}
	tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	printf("text linebreak: %d/%d cases passed (scalar proxy, grapheme fallback, DPI refresh)\n", iCases - iFailed, iCases);
	return iFailed != 0;
}
