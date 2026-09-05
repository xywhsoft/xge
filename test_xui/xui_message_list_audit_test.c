#include "../src/xui_internal.h"
#include "../src/xui_text_internal.h"

static struct {
    int LayoutNode, MeasureNode, HitNode, RenderNode, HitLine, RenderLine;
    int CaretSource, CaretDecode, CaretCluster, CaretDisplay, HitCaret;
} g_tSteps;
static int g_iLayoutCreates, g_iLayoutResets, g_iLayoutDestroys;
static int g_iDisplayCalls, g_bFailDisplay, g_bFailCaretAlloc;

static int auditDisplayLine(xui_text_layout pLayout, int iLine, const char** ppText, int* pSize)
{
    g_iDisplayCalls++;
    if (g_bFailDisplay) { *ppText = NULL; *pSize = 0; return XUI_ERROR_OUT_OF_MEMORY; }
    return xuiInternalTextLayoutGetDisplayLine(pLayout, iLine, ppText, pSize);
}

static void* auditMessageRealloc(void* pOld, size_t iSize)
{
    return g_bFailCaretAlloc ? NULL : xrtRealloc(pOld, iSize);
}

static int auditLayoutCreate(xui_context pContext, xui_text_layout* ppLayout, const xui_text_layout_desc_t* pDesc)
{
    g_iLayoutCreates++;
    return xuiTextLayoutCreate(pContext, ppLayout, pDesc);
}

static int auditLayoutReset(xui_text_layout pLayout, const xui_text_layout_desc_t* pDesc)
{
    g_iLayoutResets++;
    return xuiTextLayoutReset(pLayout, pDesc);
}

static void auditLayoutDestroy(xui_text_layout pLayout)
{
    g_iLayoutDestroys++;
    xuiTextLayoutDestroy(pLayout);
}

#define XUI_MESSAGE_LIST_AUDIT_STEP(name) (++g_tSteps.name)
#define xuiTextLayoutCreate auditLayoutCreate
#define xuiTextLayoutReset auditLayoutReset
#define xuiTextLayoutDestroy auditLayoutDestroy
#define xuiInternalTextLayoutGetDisplayLine auditDisplayLine
#define xrtRealloc auditMessageRealloc
#include "../src/xui_message_list.c"
#undef xrtRealloc
#undef xuiInternalTextLayoutGetDisplayLine
#undef xuiTextLayoutCreate
#undef xuiTextLayoutReset
#undef xuiTextLayoutDestroy
#include "xui_test_proxy.h"

static xui_text_shape_proc g_onShape;
static xui_text_measure_proc g_onMeasure;
static int g_iShapeCalls;
static int g_iShapeBytes;
static int g_iMeasureCalls;
static int g_iMeasureBytes;
static int g_bLigature;
static int g_bFailShape;

static int auditShape(xui_proxy pProxy, xui_font pFont, const char* sText,
    int iSize, uint32_t iFlags, xui_text_shape_t* pShape)
{
    int iRet;
    g_iShapeCalls++;
    g_iShapeBytes += iSize < 0 ? (int)strlen(sText) : iSize;
    if (g_bFailShape) return XUI_ERROR_OUT_OF_MEMORY;
    iRet = g_onShape(pProxy, pFont, sText, iSize, iFlags, pShape);
    if (iRet == XUI_OK && g_bLigature && pShape->iTextSize == 5 && memcmp(sText, "AffiB", 5) == 0) {
        pShape->pClusters[1].iTextEnd = 4;
        pShape->pClusters[1].fAdvance = 14;
        pShape->pClusters[2] = pShape->pClusters[4];
        pShape->iClusterCount = 3;
        pShape->fWidth = 30;
    }
    if (iRet == XUI_OK && g_bLigature && pShape->iTextSize == 5 && memcmp(sText, "Ae\xCC\x81" "B", 5) == 0) {
        pShape->pClusters[1].iTextEnd = 4;
        pShape->pClusters[2] = pShape->pClusters[3];
        pShape->iClusterCount = 3;
        pShape->fWidth = 24;
    }
    return iRet;
}

static int auditMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
    g_iMeasureCalls++;
    g_iMeasureBytes += (int)strlen(sText);
    return g_onMeasure(pProxy, pFont, sText, pSize);
}

#define CHECK(expr) do { if (!(expr)) { \
    printf("message audit failed at %d: %s\n", __LINE__, #expr); \
    iFailed = 1; goto cleanup; } } while (0)

static int auditHit(xui_widget pWidget, int iLine, float fX, int* pOffset)
{
    xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
    xui_rect_t tContent = xuiWidgetGetContentRect(pWidget);
    xui_rect_t tWorld = xuiWidgetGetWorldRect(pWidget);
    xui_message_node_data_t* pNode = &pData->arrNodes[0];
    xui_text_layout pLayout = NULL;
    xui_text_line_t tLine;
    int iNode;
    if (__xuiMessageLayoutNodes(pWidget, pData) != XUI_OK) return 0;
    pLayout = pNode->pTextLayout;
    if (xuiTextLayoutGetLine(pLayout, iLine, &tLine) != XUI_OK) return 0;
    return __xuiMessageHitTextOffset(pWidget, pData,
        tWorld.fX + tContent.fX + pNode->tTextRect.fX + fX,
        tWorld.fY + tContent.fY + pNode->tTextRect.fY + tLine.fY + 4 - pData->fScrollY,
        &iNode, pOffset) && iNode == 0;
}

static int auditGraphemes(void)
{
    static const char* const arrText[] = {
        "Ae\xCC\x81" "B",
        "A\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD" "B",
        "A\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB" "B",
        "A\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3" "B",
        "A1\xEF\xB8\x8F\xE2\x83\xA3" "B",
        "A\xE4\xB8\xAD\xE6\x96\x87" "B"
    };
    xui_test_proxy_state_t tState;
    xui_context pContext = NULL;
    xui_widget pWidget = NULL;
    xui_font pFont = NULL;
    xui_message_list_data_t* pData;
    xui_message_node_t tNode = {0};
    char sCopy[128];
    char* sLong = NULL;
    int iFailed = 0, i, j, iOffset, iPrevious, iLength, iLine, iScale;
    xuiTestProxyInit(&tState);
    g_onShape = tState.tProxy.textShape;
    g_onMeasure = tState.tProxy.textMeasure;
    tState.tProxy.textShape = auditShape;
    tState.tProxy.textMeasure = auditMeasure;
    CHECK(xuiCreate(&pContext) == XUI_OK);
    CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK);
    CHECK(tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, NULL, 0, 16, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK);
    CHECK(xuiMessageListCreate(pContext, &pWidget, NULL) == XUI_OK);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 640, 400}) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(pWidget, 0) == XUI_OK);
    tNode.iSize = sizeof(tNode);
    tNode.iType = XUI_MESSAGE_NODE_OTHER;
    tNode.sId = "unicode";
    pData = __xuiMessageListGetData(pWidget);
    for (i = 0; i < (int)(sizeof(arrText) / sizeof(arrText[0])); i++) {
        tNode.sText = arrText[i];
        iLength = (int)strlen(tNode.sText);
        CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
        iPrevious = -1;
        for (j = 0; j <= pData->arrNodes[0].tTextRect.fW; j++) {
            CHECK(auditHit(pWidget, 0, (float)j, &iOffset));
            CHECK(iOffset == xuiInternalTextGraphemeClamp(tNode.sText, iLength, iOffset));
            CHECK(iOffset >= iPrevious && iOffset <= iLength);
            if (iOffset > 0) {
                __xuiMessageSetTextSelection(pData, 0, 0, 0, iOffset);
                CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == iOffset + 1);
                CHECK(memcmp(sCopy, tNode.sText, (size_t)iOffset) == 0);
                CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
                CHECK(strcmp(sCopy, xuiTestProxyGetClipboardText(&tState)) == 0);
            }
            iPrevious = iOffset;
        }
        CHECK(iPrevious == iLength);
    }
    /* Even when the backend splits a grapheme across layout lines, caret offsets must be legal. */
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 156, 400}) == XUI_OK);
    tNode.sText = arrText[2];
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    for (iLine = 0; iLine < xuiTextLayoutGetLineCount(pData->arrNodes[0].pTextLayout); iLine++) {
        for (j = 0; j <= pData->arrNodes[0].tTextRect.fW; j++) {
            CHECK(auditHit(pWidget, iLine, (float)j, &iOffset));
            CHECK(iOffset == xuiInternalTextGraphemeClamp(tNode.sText, (int)strlen(tNode.sText), iOffset));
        }
    }
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 640, 400}) == XUI_OK);
    g_bLigature = 1;
    tNode.sText = "AffiB";
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    CHECK(auditHit(pWidget, 0, 14, &iOffset) && iOffset == 1);
    CHECK(auditHit(pWidget, 0, 16, &iOffset) && iOffset == 4);
    CHECK(auditHit(pWidget, 0, 28, &iOffset) && iOffset == 5);
    g_bLigature = 0;
    tState.tProxy.textShape = NULL;
    tNode.sText = arrText[2];
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    for (j = 0; j <= pData->arrNodes[0].tTextRect.fW; j++) {
        CHECK(auditHit(pWidget, 0, (float)j, &iOffset));
        CHECK(iOffset == xuiInternalTextGraphemeClamp(tNode.sText, (int)strlen(tNode.sText), iOffset));
    }
    tState.tProxy.textShape = auditShape;
    CHECK(xuiMessageListUpdateNodeText(pWidget, "unicode", "Ae") == XUI_OK);
    __xuiMessageSetTextSelection(pData, 0, 0, 0, 2);
    CHECK(xuiMessageListAppendNodeText(pWidget, "unicode", "\xCC\x81") == XUI_OK);
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == 2 && strcmp(sCopy, "A") == 0);
    CHECK(xuiMessageListUpdateNodeText(pWidget, "unicode", "") == XUI_OK);
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == 0);
    sLong = (char*)malloc(4097);
    CHECK(sLong != NULL);
    for (iScale = 512; iScale <= 4096; iScale *= 2) {
        memset(sLong, 'x', (size_t)iScale);
        sLong[iScale] = 0;
        CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){0, 0, iScale * 8 + 140, 400}) == XUI_OK);
        tNode.sText = sLong;
        CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
        g_iShapeCalls = g_iShapeBytes = g_iMeasureCalls = g_iMeasureBytes = 0;
        CHECK(auditHit(pWidget, 0, (float)(iScale * 8 - 1), &iOffset));
        CHECK(iOffset == iScale);
        CHECK(g_iMeasureBytes <= iScale && g_iShapeBytes <= iScale * 2);
        g_iShapeCalls = g_iShapeBytes = g_iMeasureCalls = g_iMeasureBytes = 0;
        for (j = 0; j < 32; j++) CHECK(auditHit(pWidget, 0, (float)(iScale * 4 + j), &iOffset));
        CHECK(g_iMeasureCalls == 0 && g_iShapeCalls == 0);
        printf("P2-4 size=%d: 32 warm hits, shape=%d measure=%d\n", iScale, g_iShapeCalls, g_iMeasureCalls);
    }
cleanup:
    free(sLong);
    if (pContext != NULL) xuiDestroy(pContext);
    if (pFont != NULL) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
    return iFailed;
}

static void auditResetCounts(void)
{
    memset(&g_tSteps, 0, sizeof(g_tSteps));
    g_iLayoutCreates = g_iLayoutResets = g_iLayoutDestroys = 0;
    g_iShapeCalls = g_iShapeBytes = g_iMeasureCalls = g_iMeasureBytes = 0;
}

static char g_sDisplayDraw[16][256];
static xui_rect_t g_tDisplayRects[16], g_tSelectionRects[16];
static int g_iDisplayDraws, g_iSelectionRects;
static float g_fDisplayScale = 1;

static int auditDisplayDraw(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont,
    const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
    (void)pProxy; (void)pDraw; (void)pFont; (void)tRect; (void)iColor; (void)iFlags;
    if (g_iDisplayDraws < 16) {
        snprintf(g_sDisplayDraw[g_iDisplayDraws], sizeof(g_sDisplayDraw[0]), "%s", sText);
        g_tDisplayRects[g_iDisplayDraws] = tRect;
    }
    g_iDisplayDraws++;
    return XUI_OK;
}

static int auditDisplayFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
    (void)pProxy; (void)pDraw;
    if (iColor == XUI_COLOR_RGBA(68, 130, 205, 104) && g_iSelectionRects < 16)
        g_tSelectionRects[g_iSelectionRects++] = tRect;
    return XUI_OK;
}

static int auditDisplayShape(xui_proxy pProxy, xui_font pFont, const char* sText,
    int iSize, uint32_t iFlags, xui_text_shape_t* pShape)
{
    int i, iRet = auditShape(pProxy, pFont, sText, iSize, iFlags, pShape);
    if (iRet == XUI_OK) {
        for (i = 0; i < pShape->iClusterCount; i++) pShape->pClusters[i].fAdvance *= g_fDisplayScale;
        pShape->fWidth *= g_fDisplayScale;
    }
    return iRet;
}

static int auditDisplayMeasure(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize)
{
    int iBytes = (int)strlen(sText), iAt = 0, iScalars = 0;
    int iRet = auditMeasure(pProxy, pFont, sText, pSize);
    while (iAt < iBytes) {
        uint32 iScalar;
        size_t iRead = 0;
        xstrview tView = {sText + iAt, (size_t)(iBytes - iAt)};
        if (xrtUtf8Decode(tView, &iScalar, &iRead) != XUTF_OK) iRead = 1;
        iAt += (int)iRead;
        iScalars++;
    }
    if (iBytes > 0) pSize->fX = pSize->fX / iBytes * iScalars * g_fDisplayScale;
    return iRet;
}

static int auditDisplayPaint(xui_widget pWidget, xui_draw_context pDraw)
{
    g_iDisplayDraws = g_iSelectionRects = 0;
    return __xuiMessageCacheRender(pWidget, pDraw, 0, NULL);
}

static int auditDisplay(void)
{
    static const char sRaw[] = "ab\xC2\xAD\xE2\x80\x8B\xE2\x81\xA0\xEF\xBB\xBF" "cd";
    static const struct {
        const char* sName;
        const char* sRaw;
        float fWidth;
        const char* sFirst;
        const char* sSecond;
    } arrCases[] = {
        {"formats", sRaw, 500, "abcd", NULL},
        {"edges", "\xEF\xBB\xBF" "a\xE2\x81\xA0", 500, "a", NULL},
        {"invisible", "\xC2\xAD\xE2\x80\x8B\xE2\x81\xA0\xEF\xBB\xBF", 500, "", NULL},
        {"shy-wrap", "ab\xC2\xAD" "cd", 24, "ab-", "cd"},
        {"shy-literal-hyphen", "ab\xC2\xAD-cd", 500, "ab-cd", NULL},
        {"shy-space-wrap", "ab\xC2\xAD cd", 24, "ab", "cd"},
        {"zwsp-wrap", "ab\xE2\x80\x8B" "cd", 16, "ab", "cd"},
        {"glue-overflow", "A\xC2\xA0" "B\xE2\x81\xA0" "C", 16, "A\xC2\xA0" "BC", NULL},
        {"shared-utf8-prefix", "A\xE2\x81\xA0\xE2\x82\xAC" "B", 500, "A\xE2\x82\xAC" "B", NULL},
        {"combining", "\xE2\x80\x8B" "Ae\xCC\x81" "B", 500, "Ae\xCC\x81" "B", NULL},
        {"emoji-zwj", "A\xC2\xAD\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB\xEF\xBB\xBF" "B",
            500, "A\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB" "B", NULL},
        {"rtl-logical", "A\xD7\x90\xE2\x81\xA0\xD7\x91" "B", 500, "A\xD7\x90\xD7\x91" "B", NULL},
        {"cjk", "\xE4\xB8\xAD\xE6\x96\x87\xE3\x80\x82", 16, "\xE4\xB8\xAD", "\xE6\x96\x87\xE3\x80\x82"},
        {"crlf", "ab\xC2\xAD\r\ncd", 500, "ab", "cd"}
    };
    xui_test_proxy_state_t tState;
    xui_context pContext = NULL;
    xui_widget pWidget = NULL;
    xui_font pFont = NULL;
    xui_surface pTarget = NULL;
    xui_draw_context pDraw = NULL;
    xui_message_node_t tNode = {0};
    xui_message_list_data_t* pData;
    xui_text_line_t tLine;
    char sCopy[256];
    char* sLong = NULL;
    int iFailed = 0, iOffset, iMode, iCase, iLine, i, j, iLength, iDraw, iScale;
    xuiTestProxyInit(&tState);
    g_onShape = tState.tProxy.textShape;
    g_onMeasure = tState.tProxy.textMeasure;
    tState.tProxy.textShape = auditDisplayShape;
    tState.tProxy.textMeasure = auditDisplayMeasure;
    tState.tProxy.drawText = auditDisplayDraw;
    tState.tProxy.drawRectFill = auditDisplayFill;
    CHECK(xuiCreate(&pContext) == XUI_OK);
    CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK);
    CHECK(tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, NULL, 0, 16, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK);
    CHECK(xuiMessageListCreate(pContext, &pWidget, NULL) == XUI_OK);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 640, 400}) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(pWidget, 0) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 640, 400, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(tState.tProxy.drawBegin(&tState.tProxy, &pDraw, pTarget) == XUI_OK);
    tNode.iSize = sizeof(tNode);
    tNode.iType = XUI_MESSAGE_NODE_OTHER;
    tNode.sId = "display";
    tNode.sText = sRaw;
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    pData = __xuiMessageListGetData(pWidget);
    CHECK(xuiTextLayoutGetLine(pData->arrNodes[0].pTextLayout, 0, &tLine) == XUI_OK);
    CHECK(tLine.fW == 32 && tLine.iTextSize == sizeof(sRaw) - 1);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(g_iDisplayDraws == 1 && strcmp(g_sDisplayDraw[0], "abcd") == 0);
    CHECK(auditHit(pWidget, 0, 25, &iOffset) && iOffset == 14);
    CHECK(__xuiMessageLineCaretX(&pData->arrNodes[0], &tLine, 13) == 16);
    CHECK(__xuiMessageLineCaretX(&pData->arrNodes[0], &tLine, 15) == 32);
    __xuiMessageSetTextSelection(pData, 0, 0, 0, (int)sizeof(sRaw) - 1);
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == sizeof(sRaw));
    CHECK(strcmp(sCopy, sRaw) == 0);
    CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
    CHECK(strcmp(xuiTestProxyGetClipboardText(&tState), sRaw) == 0);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(g_iSelectionRects == 1 && g_tSelectionRects[0].fW == 32);
    __xuiMessageSetTextSelection(pData, 0, 2, 0, 13);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && g_iSelectionRects == 0);
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == 12);
    CHECK(memcmp(sCopy, sRaw + 2, 11) == 0);

    for (iMode = 0; iMode < 3; iMode++) {
        tState.tProxy.textShape = iMode == 1 ? NULL : auditDisplayShape;
        g_fDisplayScale = iMode == 2 ? 1.5f : 1;
        CHECK(xuiSetVirtualDpi(pContext, g_fDisplayScale) == XUI_OK);
        for (iCase = 0; iCase < (int)(sizeof(arrCases) / sizeof(arrCases[0])); iCase++) {
            const char* arrExpected[2] = {arrCases[iCase].sFirst, arrCases[iCase].sSecond};
            int iLines = arrExpected[1] != NULL ? 2 : 1;
            tNode.sText = arrCases[iCase].sRaw;
            iLength = (int)strlen(tNode.sText);
            CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, arrCases[iCase].fWidth * g_fDisplayScale + 140, 400}) == XUI_OK);
            CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
            CHECK(xuiTextLayoutGetLineCount(pData->arrNodes[0].pTextLayout) == iLines);
            CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
            iDraw = 0;
            for (iLine = 0; iLine < iLines; iLine++) {
                xui_vec2_t tWidth;
                xui_message_node_data_t* pNode = &pData->arrNodes[0];
                CHECK(xuiTextLayoutGetLine(pNode->pTextLayout, iLine, &tLine) == XUI_OK);
                CHECK(auditDisplayMeasure(&tState.tProxy, pFont, arrExpected[iLine], &tWidth) == XUI_OK);
                CHECK(tLine.fW == tWidth.fX);
                if (arrExpected[iLine][0] != 0) CHECK(strcmp(g_sDisplayDraw[iDraw++], arrExpected[iLine]) == 0);
                CHECK(__xuiMessageEnsureLineCarets(pWidget, pData, pNode, pNode->pTextLayout, iLine, &tLine) == XUI_OK);
                CHECK(pNode->arrTextCarets[pNode->iTextCaretCount - 1].iOffset == tLine.iTextOffset + tLine.iTextSize);
                CHECK(pNode->arrTextCarets[pNode->iTextCaretCount - 1].fX == tLine.fW);
                for (j = 0; j < pNode->iTextCaretCount; j++) {
                    iOffset = pNode->arrTextCarets[j].iOffset;
                    CHECK(iOffset == xuiInternalTextGraphemeClamp(tNode.sText, iLength, iOffset));
                    if (j > 0) CHECK(iOffset > pNode->arrTextCarets[j - 1].iOffset &&
                        pNode->arrTextCarets[j].fX >= pNode->arrTextCarets[j - 1].fX);
                }
                for (j = 0; j <= tLine.fW && j <= pNode->tTextRect.fW; j++) {
                    CHECK(auditHit(pWidget, iLine, (float)j, &iOffset));
                    CHECK(iOffset == xuiInternalTextGraphemeClamp(tNode.sText, iLength, iOffset));
                }
            }
            CHECK(iDraw == g_iDisplayDraws);
            __xuiMessageSetTextSelection(pData, 0, 0, 0, iLength);
            CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == iLength + 1);
            CHECK(strcmp(sCopy, tNode.sText) == 0);
            CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
            CHECK(strcmp(xuiTestProxyGetClipboardText(&tState), tNode.sText) == 0);
            CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
            CHECK(g_iSelectionRects == iDraw);
            for (j = 0; j < g_iSelectionRects; j++) CHECK(g_tSelectionRects[j].fW > 0);
        }
    }
    printf("MessageList display: %d fixtures x proxy/fallback/DPI passed\n", (int)(sizeof(arrCases) / sizeof(arrCases[0])));
    g_fDisplayScale = 1;
    tState.tProxy.textShape = auditDisplayShape;
    CHECK(xuiSetVirtualDpi(pContext, 1) == XUI_OK);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 164, 400}) == XUI_OK);
    tNode.sText = "ab\xC2\xAD" "cd";
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    CHECK(auditHit(pWidget, 0, 19, &iOffset) && iOffset == 2);
    CHECK(auditHit(pWidget, 0, 22, &iOffset) && iOffset == 4);
    CHECK(auditHit(pWidget, 1, 0, &iOffset) && iOffset == 4);
    __xuiMessageSetTextSelection(pData, 0, 2, 0, 4);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(g_iSelectionRects == 1 && g_tSelectionRects[0].fW == 8);
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == 3 && strcmp(sCopy, "\xC2\xAD") == 0);
    CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
    CHECK(strcmp(xuiTestProxyGetClipboardText(&tState), "\xC2\xAD") == 0);
    /* A retained layout must discard cached source/display carets on DPI change. */
    g_fDisplayScale = 1.5f;
    CHECK(xuiSetVirtualDpi(pContext, 1.5f) == XUI_OK);
    CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK && pData->arrNodes[0].iTextCaretCount == 0);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(g_iDisplayDraws == 3 && strcmp(g_sDisplayDraw[0], "a") == 0 &&
        strcmp(g_sDisplayDraw[1], "b-") == 0 && strcmp(g_sDisplayDraw[2], "cd") == 0);
    CHECK(auditHit(pWidget, 1, 22, &iOffset) && iOffset == 4);
    g_fDisplayScale = 1;
    CHECK(xuiSetVirtualDpi(pContext, 1) == XUI_OK);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 640, 400}) == XUI_OK);
    /* Display shaping can join units across removed formats; geometry must
     * use that same shape, including at the exact wrapping threshold. */
    {
        static const struct {
            const char* sRaw;
            const char* sDisplay;
            float fWidth, fMiddle;
            int iEnd;
        } arrJoined[] = {
            {"Af\xC2\xAD" "fiB", "AffiB", 30, 15, 6},
            {"Af\xE2\x81\xA0" "fiB", "AffiB", 30, 15, 7},
            {"Ae\xC2\xAD\xCC\x81" "B", "Ae\xCC\x81" "B", 24, 12, 6}
        };
        g_bLigature = 1;
        for (iCase = 0; iCase < 3; iCase++) for (iMode = 0; iMode < 2; iMode++) {
            xui_text_shape_t tShape = {0};
            xui_message_node_data_t* pNode = &pData->arrNodes[0];
            float fWidth;
            CHECK(xuiTextShape(pContext, pFont, arrJoined[iCase].sDisplay, -1, XUI_TEXT_SHAPE_DEFAULT, &tShape) == XUI_OK);
            fWidth = tShape.fWidth;
            xuiTextShapeFree(&tShape);
            CHECK(fWidth == arrJoined[iCase].fWidth);
            CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, (iMode ? fWidth : 500) + 140, 400}) == XUI_OK);
            tNode.sText = arrJoined[iCase].sRaw;
            CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
            pNode = &pData->arrNodes[0];
            CHECK(xuiTextLayoutGetLineCount(pNode->pTextLayout) == 1);
            CHECK(xuiTextLayoutGetLine(pNode->pTextLayout, 0, &tLine) == XUI_OK);
            CHECK(tLine.fW == fWidth && pNode->tMeasuredText.fX == fWidth);
            CHECK(pNode->tBubbleRect.fW == __xuiMessageMax(48, fWidth + pData->tMetrics.fBubblePaddingX * 2));
            CHECK(auditHit(pWidget, 0, arrJoined[iCase].fMiddle - 1, &iOffset) && iOffset == 1);
            CHECK(auditHit(pWidget, 0, arrJoined[iCase].fMiddle + 1, &iOffset) && iOffset == arrJoined[iCase].iEnd);
            CHECK(pNode->iTextCaretCount == 4);
            CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && g_iDisplayDraws == 1 &&
                strcmp(g_sDisplayDraw[0], arrJoined[iCase].sDisplay) == 0);
        }
        g_bLigature = 0;
    }
    /* Auxiliary titles use their measured lines; the body can be collapsed. */
    tNode.iType = XUI_MESSAGE_NODE_AUXILIARY;
    tNode.iFlags = XUI_MESSAGE_NODE_FLAG_COLLAPSED;
    tNode.sTitle = "ab\xC2\xAD" "cd";
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 184, 400}) == XUI_OK);
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(g_iDisplayDraws == 3 && strcmp(g_sDisplayDraw[0], ">") == 0 &&
        strcmp(g_sDisplayDraw[1], "ab-") == 0 && strcmp(g_sDisplayDraw[2], "cd") == 0);
    CHECK(g_tDisplayRects[2].fY > g_tDisplayRects[1].fY);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 128, 400}) == XUI_OK);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && g_iDisplayDraws == 1 && strcmp(g_sDisplayDraw[0], ">") == 0);
    tNode.iType = XUI_MESSAGE_NODE_SYSTEM;
    tNode.iFlags = 0;
    tNode.sText = sRaw;
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 640, 400}) == XUI_OK);
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && strcmp(g_sDisplayDraw[0], "abcd") == 0);
    CHECK(g_tDisplayRects[0].fW == 32);
    CHECK(g_tDisplayRects[0].fX == pData->arrNodes[0].tTextRect.fX +
        (pData->arrNodes[0].tTextRect.fW - 32) * 0.5f + xuiWidgetGetContentRect(pWidget).fX);
    tNode.iType = XUI_MESSAGE_NODE_SELF;
    CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && strcmp(g_sDisplayDraw[0], "abcd") == 0);
    CHECK(auditHit(pWidget, 0, 25, &iOffset) && iOffset == 14);

    /* A failed display/shape/allocation must not publish an incomplete caret cache. */
    pData->arrNodes[0].iTextCaretCount = 0;
    g_bFailDisplay = 1;
    CHECK(!auditHit(pWidget, 0, 25, &iOffset));
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(pData->arrNodes[0].iTextCaretCount == 0);
    g_bFailDisplay = 0;
    g_bFailShape = 1;
    CHECK(!auditHit(pWidget, 0, 25, &iOffset) && pData->arrNodes[0].iTextCaretCount == 0);
    g_bFailShape = 0;
    xrtFree(pData->arrNodes[0].arrTextCarets);
    pData->arrNodes[0].arrTextCarets = NULL;
    pData->arrNodes[0].iTextCaretCapacity = 0;
    g_bFailCaretAlloc = 1;
    CHECK(!auditHit(pWidget, 0, 25, &iOffset) && pData->arrNodes[0].iTextCaretCount == 0);
    g_bFailCaretAlloc = 0;
    CHECK(auditHit(pWidget, 0, 25, &iOffset) && iOffset == 14);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
    CHECK(xuiMessageListUpdateNodeText(pWidget, "display", "x\xC2\xADy") == XUI_OK);
    CHECK(auditHit(pWidget, 0, 15, &iOffset) && iOffset == 4);
    CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && strcmp(g_sDisplayDraw[0], "xy") == 0);
    CHECK(xuiMessageListAppendNodeText(pWidget, "display", "z") == XUI_OK);
    CHECK(auditHit(pWidget, 0, 23, &iOffset) && iOffset == 5);

    {
        xui_message_node_t arrNodes[2] = {tNode, tNode};
        static const char sSelected[] = "\xC2\xAD" "cd\nx\xE2\x80\x8B" "y";
        arrNodes[0].sText = "ab\xC2\xAD" "cd";
        arrNodes[1].sId = "second";
        arrNodes[1].sText = "x\xE2\x80\x8B" "y";
        CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 164, 400}) == XUI_OK);
        CHECK(xuiMessageListSetNodes(pWidget, arrNodes, 2) == XUI_OK);
        __xuiMessageSetTextSelection(pData, 0, 2, 1, 5);
        CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == sizeof(sSelected));
        CHECK(strcmp(sCopy, sSelected) == 0);
        CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK && g_iSelectionRects == 3);
        CHECK(g_tSelectionRects[0].fW == 8 && g_tSelectionRects[1].fW == 16 && g_tSelectionRects[2].fW == 16);
        __xuiMessageSetTextSelection(pData, 1, 5, 0, 2);
        CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
        CHECK(strcmp(xuiTestProxyGetClipboardText(&tState), sSelected) == 0);
    }

    sLong = (char*)malloc(65536 * 12 + 1);
    CHECK(sLong != NULL);
    for (iScale = 512; iScale <= 65536; iScale *= 2) {
        int iLog = 0, iWork;
        for (i = 0; i < iScale; i++) memcpy(sLong + i * 12, "x\xC2\xAD\xE2\x81\xA0\xE2\x80\x8B\xEF\xBB\xBF", 12);
        sLong[iScale * 12] = 0;
        tNode.sText = sLong;
        CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){0, 0, iScale * 8 + 140, 400}) == XUI_OK);
        CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
        auditResetCounts();
        g_iDisplayCalls = 0;
        CHECK(auditHit(pWidget, 0, (float)(iScale * 8), &iOffset));
        iWork = g_tSteps.CaretSource + g_tSteps.CaretDecode + g_tSteps.CaretCluster + g_tSteps.CaretDisplay;
        CHECK(g_tSteps.CaretDecode == iScale * 5 && g_tSteps.CaretCluster == iScale && g_tSteps.CaretDisplay == iScale);
        CHECK(iWork <= iScale * 17 && g_iShapeBytes == iScale && g_iDisplayCalls == 1);
        for (i = pData->arrNodes[0].iTextCaretCount; i > 0; i /= 2) iLog++;
        auditResetCounts();
        g_iDisplayCalls = 0;
        for (j = 0; j < 32; j++) CHECK(auditHit(pWidget, 0, (float)(iScale * 4 + j), &iOffset));
        CHECK(g_iShapeCalls == 0 && g_iMeasureCalls == 0 && g_iDisplayCalls == 0 && g_tSteps.CaretDecode == 0);
        CHECK(g_tSteps.HitCaret <= iLog * 32);
        printf("MessageList display size=%d bytes: cold work=%d, 32 warm comparisons=%d, shape/measure/display=0\n",
            iScale * 12, iWork, g_tSteps.HitCaret);
    }
    for (iScale = 512; iScale <= 32768; iScale *= 8) {
        for (i = 0; i < iScale; i++) memcpy(sLong + i * 6, "x\xE2\x81\xA0" "y\n", 6);
        sLong[iScale * 6] = 0;
        tNode.sText = sLong;
        CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){0, 0, 640, 400}) == XUI_OK);
        CHECK(xuiMessageListSetNodes(pWidget, &tNode, 1) == XUI_OK);
        CHECK(xuiMessageListSetScroll(pWidget, pData->fContentHeight) == XUI_OK);
        auditResetCounts();
        g_iDisplayCalls = 0;
        CHECK(auditHit(pWidget, iScale - 1, 8, &iOffset) && iOffset == (iScale - 1) * 6 + 1);
        CHECK(g_tSteps.CaretDecode == 3 && g_tSteps.CaretSource == 6 && g_tSteps.CaretCluster == 2);
        CHECK(g_iShapeBytes == 2 && g_iDisplayCalls == 1);
        CHECK(auditDisplayPaint(pWidget, pDraw) == XUI_OK);
        CHECK(g_tSteps.RenderLine <= 25 && g_iDisplayCalls <= 25);
        printf("MessageList display lines=%d: deep cold hit decode=%d, shape bytes=%d, visible visits=%d\n",
            iScale + 1, g_tSteps.CaretDecode, g_iShapeBytes, g_tSteps.RenderLine);
    }
cleanup:
    g_bFailDisplay = g_bFailShape = g_bFailCaretAlloc = g_bLigature = 0;
    g_fDisplayScale = 1;
    free(sLong);
    if (pDraw != NULL) tState.tProxy.drawEnd(&tState.tProxy, pDraw);
    if (pTarget != NULL) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
    if (pContext != NULL) xuiDestroy(pContext);
    if (pFont != NULL) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
    return iFailed;
}

static int auditCompareRebuild(xui_widget pWidget)
{
    xui_message_list_data_t* pData = __xuiMessageListGetData(pWidget);
    xui_context pContext = xuiWidgetGetContext(pWidget);
    xui_widget pFresh = NULL;
    xui_message_list_data_t* pOther;
    xui_message_node_t* arrNodes = NULL;
    int iFailed = 0, i, j;
    arrNodes = (xui_message_node_t*)calloc((size_t)pData->iNodeCount, sizeof(*arrNodes));
    CHECK(arrNodes != NULL);
    for (i = 0; i < pData->iNodeCount; i++) arrNodes[i] = __xuiMessagePublicNode(&pData->arrNodes[i]);
    CHECK(xuiMessageListCreate(pContext, &pFresh, NULL) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(pFresh, 0) == XUI_OK);
    CHECK(xuiMessageListSetFont(pFresh, __xuiMessageFont(pWidget, pData)) == XUI_OK);
    CHECK(xuiMessageListSetMetrics(pFresh, &pData->tMetrics) == XUI_OK);
    CHECK(xuiWidgetSetRect(pFresh, xuiWidgetGetRect(pWidget)) == XUI_OK);
    CHECK(xuiMessageListSetNodes(pFresh, arrNodes, pData->iNodeCount) == XUI_OK);
    pOther = __xuiMessageListGetData(pFresh);
    CHECK(pOther->fContentHeight == pData->fContentHeight);
    for (i = 0; i < pData->iNodeCount; i++) {
        xui_message_node_data_t* a = &pData->arrNodes[i];
        xui_message_node_data_t* b = &pOther->arrNodes[i];
        CHECK(memcmp(&a->tNodeRect, &b->tNodeRect, sizeof(a->tNodeRect)) == 0);
        CHECK(memcmp(&a->tBubbleRect, &b->tBubbleRect, sizeof(a->tBubbleRect)) == 0);
        CHECK(memcmp(&a->tHeaderRect, &b->tHeaderRect, sizeof(a->tHeaderRect)) == 0);
        CHECK(memcmp(&a->tTextRect, &b->tTextRect, sizeof(a->tTextRect)) == 0);
        CHECK(a->iSelectablePrefix == b->iSelectablePrefix && a->fNextY == b->fNextY);
        if (a->iType == XUI_MESSAGE_NODE_AUXILIARY && (a->iFlags & XUI_MESSAGE_NODE_FLAG_COLLAPSED)) continue;
        CHECK(xuiTextLayoutGetLineCount(a->pTextLayout) == xuiTextLayoutGetLineCount(b->pTextLayout));
        for (j = 0; j < xuiTextLayoutGetLineCount(a->pTextLayout); j++) {
            xui_text_line_t x, y;
            CHECK(xuiTextLayoutGetLine(a->pTextLayout, j, &x) == XUI_OK);
            CHECK(xuiTextLayoutGetLine(b->pTextLayout, j, &y) == XUI_OK);
            CHECK(x.iTextOffset == y.iTextOffset && x.iTextSize == y.iTextSize && x.fY == y.fY && x.fW == y.fW);
        }
    }
cleanup:
    if (pFresh != NULL) xuiWidgetDestroy(pFresh);
    free(arrNodes);
    return iFailed;
}

static void auditLinkClick(xui_widget pWidget, const xui_message_list_event_t* pEvent, void* pUser)
{
    (void)pWidget;
    if (pEvent->iEvent == XUI_MESSAGE_EVENT_CLICK && pEvent->pNode != NULL &&
        strstr(pEvent->pNode->sText, "https://example.test/path") != NULL) (*(int*)pUser)++;
}

static int auditLayouts(int iCount)
{
    const char* sText = "hello e\xCC\x81 https://example.test/path\nsecond wrapped line";
    xui_test_proxy_state_t tState;
    xui_context pContext = NULL;
    xui_widget pWidget = NULL;
    xui_font pFont = NULL, pLargeFont = NULL;
    xui_surface pTarget = NULL;
    xui_draw_context pDraw = NULL;
    xui_message_node_t* arrNodes = NULL;
    char (*arrIds)[24] = NULL;
    xui_message_list_data_t* pData;
    xui_message_list_metrics_t tMetrics;
    xui_text_layout pSavedLayout;
    xui_rect_t tWorld, tContent, tRect, tAnchor;
    xui_event_t tEvent = {0};
    char sCopy[256], sAfter[256];
    char* sLong = NULL;
    int iFailed = 0, i, j, iLog = 1, iMid, iNode, iOffset, iHitSteps, iRenderSteps, iLinkClicks = 0, iLiveLayouts = 0;
    float fScroll, fAnchorScreen;
    xuiTestProxyInit(&tState);
    g_onShape = tState.tProxy.textShape;
    g_onMeasure = tState.tProxy.textMeasure;
    tState.tProxy.textShape = auditShape;
    tState.tProxy.textMeasure = auditMeasure;
    CHECK(xuiCreate(&pContext) == XUI_OK);
    CHECK(xuiSetProxy(pContext, &tState.tProxy) == XUI_OK);
    CHECK(tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, NULL, 0, 16, 0) == XUI_OK);
    CHECK(tState.tProxy.fontLoadMemory(&tState.tProxy, &pLargeFont, NULL, 0, 20, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK);
    CHECK(xuiMessageListCreate(pContext, &pWidget, NULL) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(pWidget, 0) == XUI_OK);
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 420, 300}) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 460, 400, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(tState.tProxy.drawBegin(&tState.tProxy, &pDraw, pTarget) == XUI_OK);
    arrNodes = (xui_message_node_t*)calloc((size_t)iCount + 1u, sizeof(*arrNodes));
    arrIds = (char (*)[24])calloc((size_t)iCount + 1u, sizeof(*arrIds));
    CHECK(arrNodes != NULL && arrIds != NULL);
    for (i = 0; i <= iCount; i++) {
        snprintf(arrIds[i], sizeof(arrIds[i]), "node-%d", i);
        arrNodes[i].iSize = sizeof(arrNodes[i]);
        arrNodes[i].iType = i % 5 >= 3 ? XUI_MESSAGE_NODE_AUXILIARY :
            (i % 5 == 2 ? XUI_MESSAGE_NODE_SYSTEM : (i % 5 == 1 ? XUI_MESSAGE_NODE_SELF : XUI_MESSAGE_NODE_OTHER));
        arrNodes[i].iFlags = XUI_MESSAGE_NODE_FLAG_COLLAPSIBLE | (i % 5 == 4 ? XUI_MESSAGE_NODE_FLAG_COLLAPSED : 0);
        arrNodes[i].sId = arrIds[i];
        arrNodes[i].sParentId = i % 5 >= 3 ? "node-0" : "";
        arrNodes[i].sSender = "sender";
        arrNodes[i].sText = sText;
    }
    pData = __xuiMessageListGetData(pWidget);
    auditResetCounts();
    CHECK(xuiMessageListSetNodes(pWidget, arrNodes, iCount) == XUI_OK);
    CHECK(g_tSteps.LayoutNode == iCount && g_tSteps.MeasureNode == iCount);
    for (i = iCount; i > 1; i /= 2) iLog++;
    iMid = (iCount / 2 / 5) * 5;
    CHECK(xuiMessageListSetScroll(pWidget, pData->arrNodes[iMid].tNodeRect.fY + 7) == XUI_OK);
    tWorld = xuiWidgetGetWorldRect(pWidget);
    tContent = xuiWidgetGetContentRect(pWidget);
    auditResetCounts();
    for (j = 0; j < 3; j++) {
        CHECK(xuiMessageListGetNodeRect(pWidget, iCount - 1).fH > 0);
        CHECK(xuiMessageListGetBubbleRect(pWidget, iCount - 1).fH > 0);
        CHECK(xuiMessageListScrollBy(pWidget, 1) == XUI_OK);
        CHECK(__xuiMessageCacheRender(pWidget, pDraw, 0, NULL) == XUI_OK);
    }
    CHECK(g_tSteps.LayoutNode == 0 && g_tSteps.MeasureNode == 0);
    CHECK(g_iLayoutCreates == 0 && g_iLayoutResets == 0 && g_iShapeCalls == 0 && g_iMeasureCalls == 0);
    CHECK(g_tSteps.RenderNode <= 24);
    iRenderSteps = g_tSteps.RenderNode;
    tRect = pData->arrNodes[iCount - 1].tNodeRect;
    auditResetCounts();
    CHECK(xuiMessageListGetNodeAt(pWidget, tWorld.fX + tContent.fX + 1,
        tWorld.fY + tContent.fY + tRect.fY + 1 - pData->fScrollY) == iCount - 1);
    CHECK(g_tSteps.HitNode <= iLog + 1 && g_tSteps.LayoutNode == 0);
    iHitSteps = g_tSteps.HitNode;
    CHECK(xuiMessageListGetNodeAt(pWidget, tWorld.fX + tContent.fX + 1,
        tWorld.fY + tContent.fY + tRect.fY - 1 - pData->fScrollY) == -1);
    auditResetCounts();
    tRect = pData->arrNodes[iMid + 2].tNodeRect;
    CHECK(__xuiMessageResolveSelectionOffset(pWidget, pData, tWorld.fX - 10,
        tWorld.fY + tContent.fY + tRect.fY + 1 - pData->fScrollY, 0, &iNode, &iOffset));
    CHECK(iNode == iMid + 3 && iOffset == (int)strlen(sText));
    CHECK(g_tSteps.HitNode <= 3 * (iLog + 1));
    CHECK(__xuiMessageResolveSelectionOffset(pWidget, pData, tWorld.fX - 10,
        tWorld.fY + tContent.fY + tRect.fY + 1 - pData->fScrollY, iCount - 1, &iNode, &iOffset));
    CHECK(iNode == iMid + 1 && iOffset == 0);
    __xuiMessageSetTextSelection(pData, iMid, 0, iMid + 1, (int)strlen(sText));
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) == (int)strlen(sText) * 2 + 2);
    CHECK(xuiMessageListCopySelection(pWidget) == XUI_OK);
    CHECK(strcmp(sCopy, xuiTestProxyGetClipboardText(&tState)) == 0);
    CHECK(xuiMessageListSetEvent(pWidget, auditLinkClick, &iLinkClicks) == XUI_OK);
    tRect = pData->arrNodes[iMid].tNodeRect;
    tEvent.iType = XUI_EVENT_POINTER_CLICK;
    tEvent.fX = tWorld.fX + tContent.fX + 1;
    tEvent.fY = tWorld.fY + tContent.fY + tRect.fY + 1 - pData->fScrollY;
    CHECK(__xuiMessageEvent(pWidget, &tEvent, NULL) == XUI_OK && iLinkClicks == 1);
    CHECK(xuiMessageListClearTextSelection(pWidget) == XUI_OK);
    CHECK(xuiMessageListSetScroll(pWidget, pData->arrNodes[iMid].tNodeRect.fY + 7) == XUI_OK);
    fAnchorScreen = pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY;
    auditResetCounts();
    CHECK(xuiMessageListUpdateNodeText(pWidget, arrIds[0], "above\n1\n2\n3\n4\n5\n6\n7\n8\n9") == XUI_OK);
    CHECK(g_tSteps.MeasureNode == 1 && g_iLayoutCreates == 0 && g_iLayoutResets == 1);
    CHECK(pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY == fAnchorScreen);
    pSavedLayout = pData->arrNodes[iCount - 1].pTextLayout;
    auditResetCounts();
    CHECK(xuiMessageListAppendNodeText(pWidget, arrIds[iCount - 1], "\nstreamed text") == XUI_OK);
    CHECK(g_tSteps.LayoutNode == 1 && g_tSteps.MeasureNode == 1);
    CHECK(g_iLayoutCreates == 0 && g_iLayoutResets == 1 && pSavedLayout == pData->arrNodes[iCount - 1].pTextLayout);
    CHECK(pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY == fAnchorScreen);
    arrNodes[iCount].iType = XUI_MESSAGE_NODE_OTHER;
    auditResetCounts();
    CHECK(xuiMessageListAddNode(pWidget, &arrNodes[iCount]) == XUI_OK);
    CHECK(g_tSteps.LayoutNode == 1 && g_tSteps.MeasureNode == 1 && g_iLayoutCreates == 1 && g_iLayoutResets == 0);
    printf("P2-5 nodes=%d: warm layout=0, 3 renders visit=%d, deep hit=%d, append layout=1\n",
        iCount, iRenderSteps, iHitSteps);
    CHECK(pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY == fAnchorScreen);
    pSavedLayout = pData->arrNodes[3].pTitleLayout;
    auditResetCounts();
    CHECK(xuiMessageListSetNodeTitle(pWidget, arrIds[3], "A long auxiliary title that wraps over several lines at this width") == XUI_OK);
    CHECK(g_tSteps.MeasureNode == 1 && g_iLayoutCreates == 0 && g_iLayoutResets == 1);
    CHECK(pSavedLayout == pData->arrNodes[3].pTitleLayout);
    CHECK(xuiMessageListSetNodeCollapsed(pWidget, arrIds[3], 1) == XUI_OK);
    CHECK(xuiMessageListSetNodeCollapsed(pWidget, arrIds[4], 0) == XUI_OK);
    CHECK(auditCompareRebuild(pWidget) == 0);
    __xuiMessageSetTextSelection(pData, iMid, 0, iMid + 1, (int)strlen(sText));
    CHECK(xuiMessageListGetSelectedText(pWidget, sCopy, sizeof(sCopy)) > 0);
    auditResetCounts();
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 300, 300}) == XUI_OK);
    CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
    CHECK(g_tSteps.MeasureNode == iCount + 1 && g_iLayoutCreates == 0 && g_iLayoutResets > 0);
    CHECK(pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY == fAnchorScreen);
    CHECK(xuiMessageListGetSelectedText(pWidget, sAfter, sizeof(sAfter)) > 0 && strcmp(sCopy, sAfter) == 0);
    CHECK(__xuiMessageCacheRender(pWidget, pDraw, 0, NULL) == XUI_OK);
    CHECK(auditCompareRebuild(pWidget) == 0);
    CHECK(xuiMessageListSetFont(pWidget, pLargeFont) == XUI_OK);
    CHECK(auditCompareRebuild(pWidget) == 0);
    CHECK(xuiSetLanguage(pContext, XUI_LANGUAGE_ZH) == XUI_OK);
    CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
    CHECK(auditCompareRebuild(pWidget) == 0);
    CHECK(xuiMessageListGetSelectedText(pWidget, sAfter, sizeof(sAfter)) > 0 && strcmp(sCopy, sAfter) == 0);
    CHECK(xuiMessageListClearTextSelection(pWidget) == XUI_OK);
    tMetrics = pData->tMetrics;
    tMetrics.fNodeGap = 0;
    CHECK(xuiMessageListSetMetrics(pWidget, &tMetrics) == XUI_OK);
    CHECK(__xuiMessageResolveSelectionOffset(pWidget, pData, tWorld.fX - 10,
        tWorld.fY + tContent.fY + pData->arrNodes[4].tNodeRect.fY - pData->fScrollY,
        iCount - 1, &iNode, &iOffset));
    CHECK(iNode == 4 && iOffset == 0);
    tMetrics = pData->tMetrics;
    tMetrics.fNodeGap = 7.5f;
    tMetrics.fPaddingY = 12.5f;
    CHECK(xuiMessageListSetMetrics(pWidget, &tMetrics) == XUI_OK);
    CHECK(xuiMessageListAppendNodeText(pWidget, arrIds[1], "\none\ntwo") == XUI_OK);
    CHECK(auditCompareRebuild(pWidget) == 0);
    CHECK(xuiMessageListSetAutoScroll(pWidget, 1) == XUI_OK);
    CHECK(xuiMessageListScrollToEnd(pWidget) == XUI_OK);
    auditResetCounts();
    CHECK(xuiWidgetSetRect(pWidget, (xui_rect_t){17, 23, 300, 360}) == XUI_OK);
    CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
    CHECK(g_tSteps.LayoutNode == 0 && g_tSteps.MeasureNode == 0 && g_iLayoutResets == 0);
    CHECK(pData->fScrollY == pData->fContentHeight - 360);
    CHECK(xuiMessageListAppendNodeText(pWidget, arrIds[iCount], "\nend") == XUI_OK);
    CHECK(pData->fScrollY == pData->fContentHeight - 360);
    CHECK(xuiMessageListSetAutoScroll(pWidget, 0) == XUI_OK);
    CHECK(xuiMessageListSetScroll(pWidget, pData->arrNodes[iMid].tNodeRect.fY + 7) == XUI_OK);
    tAnchor = pData->arrNodes[iMid].tNodeRect;
    fScroll = pData->fScrollY;
    g_bFailShape = 1;
    CHECK(xuiMessageListUpdateNodeText(pWidget, arrIds[0], "changed before failed layout") == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(pData->iLayoutDirtyFrom == 0 && pData->arrNodes[iMid].tNodeRect.fY == tAnchor.fY && pData->fScrollY == fScroll);
    pData->bSelecting = 1;
    tEvent.iType = XUI_EVENT_POINTER_CAPTURE_LOST;
    CHECK(__xuiMessageEvent(pWidget, &tEvent, NULL) == XUI_EVENT_DISPATCH_STOP && !pData->bSelecting);
    g_bFailShape = 0;
    CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
    CHECK(pData->arrNodes[iMid].tNodeRect.fY - pData->fScrollY == tAnchor.fY - fScroll);
    CHECK(auditCompareRebuild(pWidget) == 0);
    for (i = 1; i < iCount - 1; i++) arrNodes[i].iType = XUI_MESSAGE_NODE_SYSTEM;
    arrNodes[iCount - 1].iType = XUI_MESSAGE_NODE_OTHER;
    CHECK(xuiMessageListSetNodes(pWidget, arrNodes, iCount) == XUI_OK);
    auditResetCounts();
    CHECK(__xuiMessageResolveSelectionOffset(pWidget, pData, tWorld.fX - 10,
        tWorld.fY + tContent.fY + pData->arrNodes[iMid].tNodeRect.fY + 1, 0, &iNode, &iOffset));
    CHECK(iNode == iCount - 1 && iOffset == (int)strlen(sText));
    CHECK(g_tSteps.HitNode <= 3 * (iLog + 1));
    if (iCount == 128) {
        arrNodes[0].iType = XUI_MESSAGE_NODE_OTHER;
        arrNodes[1].iType = XUI_MESSAGE_NODE_AUXILIARY;
        arrNodes[1].sTitle = "DPI title";
        arrNodes[1].iFlags = 0;
        CHECK(xuiMessageListSetNodes(pWidget, arrNodes, 2) == XUI_OK);
        CHECK(auditHit(pWidget, 0, 4, &iOffset));
        CHECK(pData->arrNodes[0].iTextCaretCount > 0);
        auditResetCounts();
        CHECK(xuiSetVirtualDpi(pContext, 1.5f) == XUI_OK);
        CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
        CHECK(g_tSteps.MeasureNode == 2 && g_iLayoutResets == 3 && g_iLayoutCreates == 0);
        CHECK(pData->arrNodes[0].iTextCaretCount == 0);
        auditResetCounts();
        CHECK(xuiSetVirtualDpi(pContext, 1.5f) == XUI_OK);
        CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
        CHECK(g_tSteps.MeasureNode == 0 && g_iLayoutResets == 0);
        CHECK(xuiSetVirtualDpi(pContext, 1.0f) == XUI_OK);
        CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
        CHECK(g_tSteps.MeasureNode == 2 && g_iLayoutResets == 3);
        sLong = (char*)malloc(8193);
        CHECK(sLong != NULL);
        for (i = 0; i < 2048; i++) memcpy(sLong + i * 4, "x y\n", 4);
        sLong[8192] = 0;
        arrNodes[0].sText = sLong;
        CHECK(xuiMessageListSetNodes(pWidget, arrNodes, 1) == XUI_OK);
        CHECK(xuiMessageListSetScroll(pWidget, pData->fContentHeight * 0.7f) == XUI_OK);
        auditResetCounts();
        CHECK(__xuiMessageCacheRender(pWidget, pDraw, 0, NULL) == XUI_OK);
        CHECK(g_tSteps.RenderNode == 1 && g_tSteps.RenderLine <= 22 && g_tSteps.HitLine <= 12);
        CHECK(g_iLayoutCreates == 0 && g_iLayoutResets == 0 && g_iShapeCalls == 0);
        printf("P2-5 long message: 2049 lines, render visits=%d, line search=%d\n", g_tSteps.RenderLine, g_tSteps.HitLine);
        CHECK(xuiMessageListSetFont(pWidget, NULL) == XUI_OK);
        CHECK(xuiSetDefaultFont(pContext, NULL) == XUI_OK);
        arrNodes[0].sText = sText;
        CHECK(xuiMessageListSetNodes(pWidget, arrNodes, 1) == XUI_OK);
        CHECK(__xuiMessageCacheRender(pWidget, pDraw, 0, NULL) == XUI_OK);
        CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK);
        CHECK(__xuiMessageLayoutNodes(pWidget, pData) == XUI_OK);
        CHECK(pData->arrNodes[0].pTextLayoutFont == pFont);
    }
    for (i = 0; i < pData->iNodeCount; i++) {
        if (pData->arrNodes[i].pTextLayout != NULL) iLiveLayouts++;
        if (pData->arrNodes[i].pTitleLayout != NULL) iLiveLayouts++;
    }
    auditResetCounts();
    CHECK(xuiMessageListClear(pWidget) == XUI_OK);
    CHECK(g_iLayoutDestroys == iLiveLayouts && g_iLayoutCreates == 0);
    CHECK(pData->iLaidOutCount == 0 && xuiMessageListGetNodeAt(pWidget, 0, 0) == -1);
cleanup:
    g_bFailShape = 0;
    if (pDraw != NULL) tState.tProxy.drawEnd(&tState.tProxy, pDraw);
    if (pTarget != NULL) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
    if (pContext != NULL) xuiDestroy(pContext);
    if (pFont != NULL) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
    if (pLargeFont != NULL) tState.tProxy.fontDestroy(&tState.tProxy, pLargeFont);
    free(sLong);
    free(arrNodes);
    free(arrIds);
    return iFailed;
}

#define main messageListLegacyMain
#include "xui_message_list_test.c"
#undef main

int main(int argc, char** argv)
{
    if (argc < 2 || strcmp(argv[1], "Display") == 0) {
        if (auditDisplay()) return 1;
        puts("xui_message_list_audit_test display/source mapping passed");
    }
    if (argc < 2 || strcmp(argv[1], "P2-4") == 0) {
        if (auditGraphemes()) return 1;
        puts("xui_message_list_audit_test P2-4 passed");
    }
    if (argc < 2 || strcmp(argv[1], "P2-5") == 0) {
        if (auditLayouts(128) || auditLayouts(1024) || auditLayouts(8192)) return 1;
        puts("xui_message_list_audit_test P2-5 passed");
    }
    if (messageListLegacyMain()) return 1;
    return 0;
}
