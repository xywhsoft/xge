#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if ( !(expr) ) { \
	printf("text dpi failure at line %d: %s\n", __LINE__, #expr); \
	iFailed = 1; goto cleanup; } } while ( 0 )

static xui_context g_pContext;
static xui_proxy_t g_tBase;
static int g_iMetrics, g_iShapes, g_iFailure;
static xui_text_layout g_pReentry;
static xui_text_layout_desc_t g_tDesc;
static int g_iNestedResult;
static float g_fChangeDpi;

static float textLineHeight(void)
{
	return 10.0f + 4.0f * (xuiGetVirtualDpi(g_pContext) - 1.0f);
}

static int textMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
	int iRet;
	g_iMetrics++;
	if ( g_iFailure == 1 ) return XUI_ERROR_BACKEND_FAILED;
	if ( g_pReentry != NULL ) {
		xui_vec2_t tNested = xuiTextLayoutGetSize(g_pReentry);
		if ( tNested.fX != 0 || tNested.fY != 0 ) return XUI_ERROR_INVALID_STATE;
		g_iNestedResult = xuiTextLayoutReset(g_pReentry, &g_tDesc);
		g_pReentry = NULL;
	}
	iRet = g_tBase.fontGetMetrics(pProxy, pFont, pMetrics);
	if ( iRet != XUI_OK ) return iRet;
	/* A DPI-sensitive proxy may change hinting metrics, but font pixels stay four. */
	if ( pMetrics->fSize != 4.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	pMetrics->fLineHeight = textLineHeight();
	pMetrics->fAscent = pMetrics->fLineHeight * 0.8f;
	pMetrics->fDescent = pMetrics->fLineHeight - pMetrics->fAscent;
	return XUI_OK;
}

static int textShape(xui_proxy pProxy, xui_font pFont, const char* sText,
	int iTextSize, uint32_t iFlags, xui_text_shape_t* pShape)
{
	int i, iRet;
	g_iShapes++;
	iRet = g_tBase.textShape(pProxy, pFont, sText, iTextSize, iFlags, pShape);
	if ( iRet != XUI_OK ) return iRet;
	if ( g_iFailure == 2 ) return XUI_ERROR_BACKEND_FAILED;
	pShape->fWidth = 0;
	for ( i = 0; i < pShape->iClusterCount; i++ ) {
		pShape->pClusters[i].fAdvance += 4.0f * (xuiGetVirtualDpi(g_pContext) - 1.0f);
		pShape->fWidth += pShape->pClusters[i].fAdvance;
	}
	pShape->fHeight = pShape->fLineHeight = textLineHeight();
	pShape->fAscent = pShape->fLineHeight * 0.8f;
	pShape->fDescent = pShape->fLineHeight - pShape->fAscent;
	if ( g_fChangeDpi > 0 ) {
		float fDpi = g_fChangeDpi;
		g_fChangeDpi = 0;
		return xuiSetVirtualDpi(g_pContext, fDpi);
	}
	return XUI_OK;
}

static int textMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
	int iRet = g_tBase.textMeasure(pProxy, pFont, sText, pSize);
	if ( iRet == XUI_OK ) pSize->fX += (float)strlen(sText) * 4 * (xuiGetVirtualDpi(g_pContext) - 1);
	return iRet;
}

int main(void)
{
	const float arrDpi[] = {1.25f, 1.5f, 2.0f, 1.0f, 1.0f, 2.0f};
	xui_test_proxy_state_t tState;
	xui_text_layout arrLayouts[6] = {0};
	const char* arrText[6] = {0};
	xui_font pFont = NULL;
	xui_surface pTarget = NULL;
	xui_text_line_t tLine;
	xui_vec2_t tSize;
	xui_rect_t tRect = {0, 0, 12, 256};
	char sSource[] = "abcdefghijkl";
	float fPreviousDpi = 1;
	int i, j, k, iMetrics, iShapes, iDraws, iLines;
	int iFailed = 0;
	xuiTestProxyInit(&tState);
	g_tBase = tState.tProxy;
	tState.tProxy.fontGetMetrics = textMetrics;
	tState.tProxy.textShape = textShape;
	CHECK(xuiCreate(&g_pContext) == XUI_OK);
	CHECK(xuiSetProxy(g_pContext, &tState.tProxy) == XUI_OK);
	CHECK(tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, NULL, 4.0f, 0) == XUI_OK);
	CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 32, 256, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	memset(&g_tDesc, 0, sizeof(g_tDesc));
	g_tDesc.iSize = sizeof(g_tDesc);
	g_tDesc.sText = sSource;
	g_tDesc.iTextSize = -1;
	g_tDesc.pFont = pFont;
	g_tDesc.fMaxWidth = 12;
	g_tDesc.fLineGap = 1;
	g_tDesc.iWrapMode = XUI_TEXT_WRAP_CHAR;
	for ( j = 0; j < 6; j++ ) {
		g_tDesc.fMaxHeight = j == 3 ? 25 : 0;
		CHECK(xuiTextLayoutCreate(g_pContext, &arrLayouts[j], &g_tDesc) == XUI_OK);
		arrText[j] = xuiTextLayoutGetText(arrLayouts[j]);
	}
	CHECK(g_iMetrics == 6 && g_iShapes == 6);
	memset(sSource, 'z', 12);
	for ( i = 0; i < (int)(sizeof(arrDpi) / sizeof(arrDpi[0])); i++ ) {
		float fAdvance = 2 + 4 * (arrDpi[i] - 1);
		float fHeight = 10 + 4 * (arrDpi[i] - 1);
		int iColumns = (int)(12 / fAdvance);
		int bChanged = arrDpi[i] != fPreviousDpi;
		iMetrics = g_iMetrics;
		CHECK(xuiSetVirtualDpi(g_pContext, arrDpi[i]) == XUI_OK);
		CHECK(g_iMetrics == iMetrics);
		for ( j = 0; j < 6; j++ ) {
			iLines = (12 + iColumns - 1) / iColumns;
			if ( j == 3 && iLines * (fHeight + 1) - 1 > 25 ) iLines = (int)(26 / (fHeight + 1));
			iMetrics = g_iMetrics;
			iShapes = g_iShapes;
			switch ( j ) {
			case 0: tSize = xuiTextLayoutGetSize(arrLayouts[j]);
				CHECK(tSize.fX == iColumns * fAdvance && tSize.fY == iLines * (fHeight + 1) - 1); break;
			case 1: CHECK(xuiTextLayoutGetLineCount(arrLayouts[j]) == iLines); break;
			case 2: CHECK(xuiTextLayoutGetLine(arrLayouts[j], iLines - 1, &tLine) == XUI_OK); break;
			case 3: CHECK(xuiTextLayoutGetTruncated(arrLayouts[j]) == (iLines < (12 + iColumns - 1) / iColumns)); break;
			case 4: CHECK(xuiTextLayoutDraw(arrLayouts[j], pTarget, tRect, XUI_COLOR_WHITE, 0) == XUI_OK); break;
			case 5: CHECK(xuiTextLayoutGetText(arrLayouts[j]) == arrText[j]); break;
			}
			CHECK(g_iMetrics == iMetrics + bChanged && g_iShapes == iShapes + bChanged);
			CHECK(xuiTextLayoutGetLineCount(arrLayouts[j]) == iLines);
			CHECK(xuiTextLayoutGetLine(arrLayouts[j], iLines - 1, &tLine) == XUI_OK);
			CHECK(tLine.iTextOffset == (iLines - 1) * iColumns && tLine.fH == fHeight);
			CHECK(xuiTextLayoutGetText(arrLayouts[j]) == arrText[j]);
			CHECK(strcmp(arrText[j], "abcdefghijkl") == 0);
			for ( k = 0; k < 100; k++ ) (void)xuiTextLayoutGetSize(arrLayouts[j]);
			CHECK(g_iMetrics == iMetrics + bChanged && g_iShapes == iShapes + bChanged);
		}
		CHECK(xuiTestSurfaceGetLastTextRect(pTarget).fW == 12);
		CHECK(xuiTestSurfaceGetLastTextRect(pTarget).fH == fHeight);
		fPreviousDpi = arrDpi[i];
	}
	/* Returning to the same DPI after unobserved changes still advances the generation. */
	CHECK(xuiSetVirtualDpi(g_pContext, 1) == XUI_OK);
	CHECK(xuiSetVirtualDpi(g_pContext, 2) == XUI_OK);
	iMetrics = g_iMetrics;
	CHECK(xuiTextLayoutGetLineCount(arrLayouts[0]) == 6 && g_iMetrics == iMetrics + 1);

	CHECK(xuiSetVirtualDpi(g_pContext, 1.5f) == XUI_OK);
	g_iFailure = 1;
	tSize = xuiTextLayoutGetSize(arrLayouts[0]);
	CHECK(tSize.fX == 0 && tSize.fY == 0);
	CHECK(xuiTextLayoutGetLineCount(arrLayouts[0]) == 0);
	memset(&tLine, 0, sizeof(tLine));
	tLine.iTextOffset = -19;
	CHECK(xuiTextLayoutGetLine(arrLayouts[0], 0, &tLine) == XUI_ERROR_BACKEND_FAILED && tLine.iTextOffset == -19);
	iDraws = xuiTestSurfaceGetTextDrawCount(pTarget);
	CHECK(xuiTextLayoutDraw(arrLayouts[0], pTarget, tRect, XUI_COLOR_WHITE, 0) == XUI_ERROR_BACKEND_FAILED);
	CHECK(xuiTestSurfaceGetTextDrawCount(pTarget) == iDraws);
	CHECK(xuiTextLayoutGetText(arrLayouts[0]) == arrText[0]);
	g_iFailure = 2;
	CHECK(xuiTextLayoutGetLine(arrLayouts[0], 0, &tLine) == XUI_ERROR_BACKEND_FAILED);
	g_iFailure = 0;
	CHECK(xuiTextLayoutGetLineCount(arrLayouts[0]) == 4);
	CHECK(xuiTextLayoutGetText(arrLayouts[0]) == arrText[0]);

	CHECK(xuiSetVirtualDpi(g_pContext, 1.25f) == XUI_OK);
	g_pReentry = arrLayouts[0];
	g_iNestedResult = XUI_OK;
	CHECK(xuiTextLayoutGetLineCount(arrLayouts[0]) == 3);
	CHECK(g_iNestedResult == XUI_ERROR_INVALID_STATE && g_pReentry == NULL);
	CHECK(xuiSetVirtualDpi(g_pContext, 1) == XUI_OK);
	g_fChangeDpi = 2;
	CHECK(xuiTextLayoutGetLine(arrLayouts[0], 0, &tLine) == XUI_ERROR_INVALID_STATE);
	CHECK(xuiTextLayoutGetLineCount(arrLayouts[0]) == 6);
	CHECK(xuiTextLayoutGetText(arrLayouts[0]) == arrText[0]);

	for ( j = 0; j < 6; j++ ) {
		xuiTextLayoutDestroy(arrLayouts[j]);
		arrLayouts[j] = NULL;
	}
	xuiDestroy(g_pContext);
	g_pContext = NULL;
	tState.tProxy.textShape = NULL;
	tState.tProxy.textMeasure = textMeasure;
	CHECK(xuiCreate(&g_pContext) == XUI_OK);
	CHECK(xuiSetProxy(g_pContext, &tState.tProxy) == XUI_OK);
	g_tDesc.sText = "ab";
	g_tDesc.fMaxHeight = 0;
	CHECK(xuiTextLayoutCreate(g_pContext, &arrLayouts[0], &g_tDesc) == XUI_OK);
	iMetrics = g_iMetrics;
	iShapes = g_iShapes;
	CHECK(xuiSetVirtualDpi(g_pContext, 2) == XUI_OK);
	tSize = xuiTextLayoutGetSize(arrLayouts[0]);
	CHECK(tSize.fX == 12 && tSize.fY == 14);
	CHECK(g_iMetrics == iMetrics + 2 && g_iShapes == iShapes);
	CHECK(xuiTextLayoutDraw(arrLayouts[0], pTarget, tRect, XUI_COLOR_WHITE, 0) == XUI_OK);
	CHECK(g_iMetrics == iMetrics + 2);
	xuiTextLayoutDestroy(arrLayouts[0]);
	arrLayouts[0] = NULL;
	xuiDestroy(g_pContext);
	g_pContext = NULL;

	/* DPI-independent proxy pixels must not acquire a second scale factor. */
	CHECK(xuiCreate(&g_pContext) == XUI_OK);
	CHECK(xuiSetProxy(g_pContext, &g_tBase) == XUI_OK);
	CHECK(xuiTextLayoutCreate(g_pContext, &arrLayouts[0], &g_tDesc) == XUI_OK);
	CHECK(xuiSetVirtualDpi(g_pContext, 2) == XUI_OK);
	tSize = xuiTextLayoutGetSize(arrLayouts[0]);
	CHECK(tSize.fX == 4 && tSize.fY == 4);

cleanup:
	for ( j = 0; j < 6; j++ ) if ( arrLayouts[j] != NULL ) xuiTextLayoutDestroy(arrLayouts[j]);
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( g_pContext != NULL ) xuiDestroy(g_pContext);
	if ( !iFailed ) printf("xui_text_dpi_test passed: metrics=%d shapes=%d\n", g_iMetrics, g_iShapes);
	return iFailed;
}
