#include "../src/xui_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct xui_context_t g_tContext;
static xui_proxy_t g_tProxy;
static int g_iShapes, g_iShapeBytes, g_iShapedFormats;
static int g_bScalarOnly;
static int g_iFailAfter = -1, g_iAllocations, g_iLive;
static void* g_pLive[64];
static struct {
    unsigned long long iDecode, iMap, iProperties, iRangeProbes, iNextProbes;
    unsigned long long iWhitespaceVisits, iWrapSteps, iIndexBuildSteps;
} g_tWork;

static int projectionFail(void) { g_iAllocations++; return g_iFailAfter >= 0 && g_iFailAfter-- == 0; }
static void* projectionMalloc(size_t n)
{
    void* p = projectionFail() ? NULL : xrtMalloc(n);
    if (p) { if (g_iLive == 64) abort(); g_pLive[g_iLive++] = p; }
    return p;
}
static void* projectionCalloc(size_t n, size_t s)
{
    void* p = projectionFail() ? NULL : xrtCalloc(n, s);
    if (p) { if (g_iLive == 64) abort(); g_pLive[g_iLive++] = p; }
    return p;
}
static void* projectionRealloc(void* p, size_t n)
{
    void* q;
    int i;
    if (projectionFail()) return NULL;
    for (i = 0; i < g_iLive && g_pLive[i] != p; i++) {}
    q = xrtRealloc(p, n);
    if (q) { if (i == g_iLive) { if (g_iLive == 64) abort(); g_iLive++; } g_pLive[i] = q; }
    return q;
}
static void projectionFree(void* p)
{
    int i;
    for (i = 0; i < g_iLive; i++) if (g_pLive[i] == p) { g_pLive[i] = g_pLive[--g_iLive]; break; }
    xrtFree(p);
}

int xuiInternalContextIsValid(xui_context pContext) { return pContext == &g_tContext; }
xui_proxy xuiInternalContextGetProxy(xui_context pContext) { return &g_tProxy; }

#define xrtMalloc projectionMalloc
#define xrtCalloc projectionCalloc
#define xrtRealloc projectionRealloc
#define xrtFree projectionFree
#define XUI_TEXT_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#define XUI_TEXT_BREAK_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#include "../src/xui_text.c"
#undef xrtMalloc
#undef xrtCalloc
#undef xrtRealloc
#undef xrtFree

#define CHECK(expr) do { if (!(expr)) { printf("projection failed at %d: %s\n", __LINE__, #expr); return 1; } } while (0)

static int projectionMetrics(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics)
{
    memset(pMetrics, 0, sizeof(*pMetrics));
    pMetrics->fAscent = 12;
    pMetrics->fDescent = 4;
    pMetrics->fLineHeight = 16;
    return XUI_OK;
}

static int projectionShape(xui_proxy pProxy, xui_font pFont, const char* sText,
    int iSize, uint32_t iFlags, xui_text_shape_t* pShape)
{
    int iAt = 0, iCount = 0;
    g_iShapes++;
    g_iShapeBytes += iSize;
    memset(pShape, 0, sizeof(*pShape));
    pShape->iTextSize = iSize;
    pShape->fLineHeight = 16;
    pShape->fAscent = 12;
    pShape->fDescent = 4;
    if (iSize > 0) {
        pShape->pClusters = (xui_text_cluster_t*)projectionCalloc((size_t)iSize, sizeof(*pShape->pClusters));
        if (pShape->pClusters == NULL) return XUI_ERROR_OUT_OF_MEMORY;
    }
    while (iAt < iSize) {
        uint32 iScalar = 0;
        size_t iRead = 0;
        float fWidth = 8;
        xstrview tView = {sText + iAt, (size_t)(iSize - iAt)};
        if (!g_bScalarOnly && iSize - iAt >= 3 && memcmp(sText + iAt, "ffi", 3) == 0) { iRead = 3; fWidth = 14; }
        else if (!g_bScalarOnly && iSize - iAt >= 3 && memcmp(sText + iAt, "e\xCC\x81", 3) == 0) iRead = 3;
        else if (xrtUtf8Decode(tView, &iScalar, &iRead) != XUTF_OK) iRead = 1;
        if (iScalar == 0xADu || iScalar == 0x200Bu || iScalar == 0x2060u || iScalar == 0xFEFFu) g_iShapedFormats++;
        pShape->pClusters[iCount].iTextStart = iAt;
        pShape->pClusters[iCount].iTextEnd = iAt + (int)iRead;
        pShape->pClusters[iCount++].fAdvance = fWidth;
        pShape->fWidth += fWidth;
        iAt += (int)iRead;
    }
    pShape->iClusterCount = iCount;
    return XUI_OK;
}

static int projectionCases(void)
{
    static const char* const arrRaw[] = {"Af\xC2\xAD" "fiB", "Ae\xC2\xAD\xCC\x81" "B", "Af\xE2\x81\xA0" "fiB"};
    int i, iMode, iFailed = 0;
    for (i = 0; i < 3; i++) {
        xui_text_layout pLayout = NULL;
        xui_text_layout_desc_t tDesc = {0};
        xui_text_line_t tLine;
        xui_text_shape_t tShape = {0};
        const char* sDisplay = NULL;
        int iSize = 0;
        tDesc.iSize = sizeof(tDesc);
        tDesc.sText = arrRaw[i];
        tDesc.iTextSize = -1;
        tDesc.pFont = (xui_font)&g_tContext;
        tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
        tDesc.fMaxWidth = 500;
        if (xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) != XUI_OK ||
            xuiTextLayoutGetLine(pLayout, 0, &tLine) != XUI_OK ||
            xuiInternalTextLayoutGetDisplayLine(pLayout, 0, &sDisplay, &iSize) != XUI_OK ||
            xuiTextShape(&g_tContext, tDesc.pFont, sDisplay, iSize, XUI_TEXT_SHAPE_DEFAULT, &tShape) != XUI_OK) return 1;
        printf("projection fixture=%d line=%g display=%g\n", i, tLine.fW, tShape.fWidth);
        if (tLine.fW != tShape.fWidth || xuiTextLayoutGetSize(pLayout).fX != tShape.fWidth) iFailed = 1;
        tDesc.fMaxWidth = tShape.fWidth;
        for (iMode = XUI_TEXT_WRAP_NONE; iMode <= XUI_TEXT_WRAP_CHAR; iMode++) {
            tDesc.iWrapMode = iMode;
            if (xuiTextLayoutReset(pLayout, &tDesc) != XUI_OK || xuiTextLayoutGetLineCount(pLayout) != 1 ||
                xuiTextLayoutGetSize(pLayout).fX != tShape.fWidth || strcmp(xuiTextLayoutGetText(pLayout), arrRaw[i]) != 0) iFailed = 1;
        }
        xuiTextShapeFree(&tShape);
        xuiTextLayoutDestroy(pLayout);
        CHECK(g_iLive == 0);
    }
    return iFailed;
}

static int projectionScale(void)
{
    static const char* const arrPatterns[] = {
        "Af\xC2\xAD" "fiB ", "Ae\xC2\xAD\xCC\x81" "B ", "Af\xE2\x81\xA0" "fiB ",
        "ab\xC2\xAD" "cd\xE2\x80\x8B", "\xC2\xAD\xE2\x81\xA0"
    };
    int n, i;
    for (n = 256; n <= 65536; n *= 4) for (i = 0; i < 5; i++) {
        int j, iBytes = (int)strlen(arrPatterns[i]) * n, iPrevious = 0, iLog = 1;
        unsigned long long iBuildBytes, iVisits, iProbes;
        char* sText = (char*)malloc((size_t)iBytes + 1);
        xui_text_layout pLayout = NULL;
        xui_text_layout_desc_t tDesc = {0};
        CHECK(sText != NULL);
        for (j = 0; j < n; j++) memcpy(sText + (size_t)j * strlen(arrPatterns[i]), arrPatterns[i], strlen(arrPatterns[i]));
        sText[iBytes] = 0;
        tDesc.iSize = sizeof(tDesc);
        tDesc.sText = sText;
        tDesc.iTextSize = iBytes;
        tDesc.pFont = (xui_font)&g_tContext;
        tDesc.fMaxWidth = i == 0 || i == 2 ? 30 : 24;
        tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
        memset(&g_tWork, 0, sizeof(g_tWork));
        g_iShapes = g_iShapeBytes = g_iShapedFormats = 0;
        CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_OK);
        iBuildBytes = (unsigned long long)g_iShapeBytes;
        iVisits = g_tWork.iDecode + g_tWork.iMap + g_tWork.iIndexBuildSteps;
        iProbes = g_tWork.iRangeProbes + g_tWork.iNextProbes;
        for (j = pLayout->tShape.iClusterCount; j > 1; j /= 2) iLog++;
        CHECK(iBuildBytes <= (unsigned long long)iBytes + 1 && g_iShapes <= 2);
        CHECK(g_iShapedFormats == 0);
        CHECK(iVisits <= (unsigned long long)iBytes * 16 && g_tWork.iWrapSteps <= (unsigned long long)iBytes * 3);
        CHECK(iProbes <= (unsigned long long)20 * (pLayout->tShape.iClusterCount + 1) * iLog);
        for (j = 0; j < xuiTextLayoutGetLineCount(pLayout); j++) {
            xui_text_line_t tLine;
            xui_text_shape_t tShape = {0};
            const char* sDisplay;
            int iSize;
            CHECK(xuiTextLayoutGetLine(pLayout, j, &tLine) == XUI_OK);
            CHECK(tLine.iTextOffset >= iPrevious && tLine.iTextOffset + tLine.iTextSize <= iBytes);
            iPrevious = tLine.iTextOffset + tLine.iTextSize;
            CHECK(xuiInternalTextLayoutGetDisplayLine(pLayout, j, &sDisplay, &iSize) == XUI_OK);
            CHECK(xuiTextShape(&g_tContext, tDesc.pFont, sDisplay, iSize, XUI_TEXT_SHAPE_DEFAULT, &tShape) == XUI_OK);
            CHECK(tLine.fW == tShape.fWidth);
            xuiTextShapeFree(&tShape);
        }
        g_iShapes = 0;
        for (j = 0; j < 32; j++) CHECK(xuiTextLayoutGetSize(pLayout).fY > 0);
        CHECK(g_iShapes == 0 && strcmp(xuiTextLayoutGetText(pLayout), sText) == 0);
        printf("projection pattern=%d bytes=%d build-shape-bytes=%llu visits=%llu probes=%llu\n", i, iBytes, iBuildBytes, iVisits, iProbes);
        xuiTextLayoutDestroy(pLayout);
        free(sText);
        CHECK(g_iLive == 0);
    }
    return 0;
}

static int projectionScalarGraphemes(void)
{
    static const char* const arrText[] = {
        "e\xC2\xAD\xCC\x81",
        "\xF0\x9F\x91\xA9\xE2\x81\xA0\xE2\x80\x8D\xF0\x9F\x92\xBB"
    };
    int i;
    g_bScalarOnly = 1;
    for (i = 0; i < 2; i++) {
        xui_text_layout pLayout = NULL;
        xui_text_layout_desc_t tDesc = {0};
        xui_text_line_t tLine;
        tDesc.iSize = sizeof(tDesc);
        tDesc.sText = arrText[i];
        tDesc.iTextSize = -1;
        tDesc.pFont = (xui_font)&g_tContext;
        tDesc.fMaxWidth = 1;
        tDesc.iWrapMode = XUI_TEXT_WRAP_CHAR;
        CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_OK);
        CHECK(xuiTextLayoutGetLine(pLayout, 0, &tLine) == XUI_OK);
        CHECK(tLine.iTextSize == (int)strlen(arrText[i]));
        CHECK(tLine.fW == (i ? 24 : 16));
        /* CHAR retains its historical empty line after an oversized unit. */
        CHECK(xuiTextLayoutGetLineCount(pLayout) == 2);
        xuiTextLayoutDestroy(pLayout);
        CHECK(g_iLive == 0);
    }
    g_bScalarOnly = 0;
    puts("projection scalar-only joined graphemes passed");
    return 0;
}

static int projectionFailures(void)
{
    xui_text_layout_desc_t tDesc = {0};
    xui_text_layout pLayout = NULL;
    int i, iCount;
    tDesc.iSize = sizeof(tDesc);
    tDesc.sText = "Af\xC2\xAD" "fiB Ae\xE2\x81\xA0\xCC\x81" "B";
    tDesc.iTextSize = -1;
    tDesc.pFont = (xui_font)&g_tContext;
    tDesc.fMaxWidth = 30;
    tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
    g_iAllocations = 0;
    CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_OK);
    iCount = g_iAllocations;
    xuiTextLayoutDestroy(pLayout);
    for (i = 0; i < iCount; i++) {
        g_iFailAfter = i;
        CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_ERROR_OUT_OF_MEMORY && pLayout == NULL);
        g_iFailAfter = -1;
        CHECK(g_iLive == 0);
        CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_OK);
        xuiTextLayoutDestroy(pLayout);
        CHECK(g_iLive == 0);
    }
    printf("projection construction: %d allocation failures retried\n", iCount);
    CHECK(xuiTextLayoutCreate(&g_tContext, &pLayout, &tDesc) == XUI_OK);
    for (i = 0; i < iCount - 2; i++) {
        xui_text_line_t tLine, *pSaved = pLayout->pLines;
        const char* sSaved = xuiTextLayoutGetText(pLayout);
        g_tContext.iDpiGeneration++;
        g_iFailAfter = i;
        CHECK(xuiTextLayoutGetLine(pLayout, 0, &tLine) == XUI_ERROR_OUT_OF_MEMORY);
        g_iFailAfter = -1;
        CHECK(pLayout->pLines == pSaved && pLayout->sText == sSaved);
        CHECK(xuiTextLayoutGetLine(pLayout, 0, &tLine) == XUI_OK && pLayout->sText == sSaved && tLine.fW == 30);
    }
    xuiTextLayoutDestroy(pLayout);
    CHECK(g_iLive == 0);
    printf("projection DPI: %d allocation failures retained original geometry and retried\n", iCount - 2);
    return 0;
}

int main(void)
{
    g_tContext.iDpiGeneration = 1;
    g_tProxy.fontGetMetrics = projectionMetrics;
    g_tProxy.textShape = projectionShape;
    if (projectionCases() || projectionScalarGraphemes() || projectionScale() || projectionFailures()) return 1;
    puts("xui_text_projection_test passed");
    return 0;
}
