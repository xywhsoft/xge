#include "../src/xui_internal.h"

static struct {
    int LayoutNode, MeasureNode, HitNode, RenderNode, HitLine, RenderLine;
} g_tSteps;
static int g_iLayoutCreates, g_iLayoutResets, g_iLayoutDestroys;

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
#include "../src/xui_message_list.c"
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
