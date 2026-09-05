#include "../src/xui_message_list.c"
#include "xui_test_proxy.h"

static xui_text_shape_proc g_onShape;
static xui_text_measure_proc g_onMeasure;
static int g_iShapeCalls;
static int g_iShapeBytes;
static int g_iMeasureCalls;
static int g_iMeasureBytes;
static int g_bLigature;

static int auditShape(xui_proxy pProxy, xui_font pFont, const char* sText,
    int iSize, uint32_t iFlags, xui_text_shape_t* pShape)
{
    int iRet;
    g_iShapeCalls++;
    g_iShapeBytes += iSize < 0 ? (int)strlen(sText) : iSize;
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
    if (__xuiMessageEnsureNodeTextLayout(pWidget, pData, pNode, pNode->tTextRect.fW, &pLayout) != XUI_OK ||
        xuiTextLayoutGetLine(pLayout, iLine, &tLine) != XUI_OK) return 0;
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

#define main messageListLegacyMain
#include "xui_message_list_test.c"
#undef main

int main(void)
{
    if (auditGraphemes()) return 1;
    if (messageListLegacyMain()) return 1;
    puts("xui_message_list_audit_test P2-4 passed");
    return 0;
}
