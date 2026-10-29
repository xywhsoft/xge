#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_step_bar_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static xui_draw_circle_stroke_proc g_onCircleStroke;
static int g_bFailCircleStroke;
static int g_iErrorCount;
static int g_iLastError;
static int g_iLastErrorStage;
static xui_widget g_pLastErrorWidget;

static void __xuiStepBarTestError(xui_context pContext, const xui_error_info_t* pError, void* pUser)
{
	(void)pContext;
	(void)pUser;
	if ( pError == NULL ) return;
	g_iErrorCount++;
	g_iLastError = pError->iCode;
	g_iLastErrorStage = pError->iStage;
	g_pLastErrorWidget = pError->pWidget;
}

static int __xuiStepBarTestCircleStroke(xui_proxy pProxy, xui_draw_context pDraw,
	float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	if ( g_bFailCircleStroke ) {
		g_bFailCircleStroke = 0;
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return g_onCircleStroke(pProxy, pDraw, fX, fY, fRadius, fWidth, iColor);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pArrow;
	xui_widget pDot;
	xui_widget pVertical;
	xui_widget pInvalid;
	xui_surface pTarget;
	xui_font pFont;
	xui_step_bar_desc_t tDesc;
	xui_step_bar_desc_t tInvalidDesc;
	const char* arrTitles[3];
	const char* arrLongTitle[1];
	const char* sLongTitle;
	const char* sRenderedText;
	xui_rect_t tRect;
	xui_rect_t tNextRect;
	xui_rect_t tLastRect;
	xui_rect_i_t tFullRect;
	xui_vec2_t tMeasured;
	xui_accessible_node_t tAccessible;
	xallocationbudget* pBudget;
	xallocationscope tScope;
	uint32_t iAccessibilityRevision;
	float fBarHeight;
	float fDotRadius;
	float fLineWidth;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pArrow = NULL;
	pDot = NULL;
	pVertical = NULL;
	pInvalid = NULL;
	pTarget = NULL;
	pFont = NULL;
	pBudget = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	g_onCircleStroke = tState.tProxy.drawCircleStroke;
	tState.tProxy.drawCircleStroke = __xuiStepBarTestCircleStroke;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetErrorCallback(pContext, __xuiStepBarTestError, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "error callback set");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "step", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	arrTitles[0] = "First";
	arrTitles[1] = "Second";
	arrTitles[2] = "Third";
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTitles = arrTitles;
	tDesc.iStepCount = 3;
	tDesc.iCurrent = 1;
	tDesc.iStyle = XUI_STEP_BAR_STYLE_ARROW;
	tDesc.pFont = pFont;
	iRet = xuiStepBarCreate(pContext, &pArrow, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pArrow != NULL, "arrow create");
	xuiWidgetSetRect(pArrow, (xui_rect_t){20.0f, 20.0f, 460.0f, 36.0f});
	iRet = xuiWidgetAddChild(pRoot, pArrow);
	XUI_TEST_CHECK(iRet == XUI_OK, "add arrow");
	XUI_TEST_CHECK(xuiStepBarGetStepCount(pArrow) == 3, "step count");
	XUI_TEST_CHECK(strcmp(xuiStepBarGetTitle(pArrow, 1), "Second") == 0, "title copy");
	XUI_TEST_CHECK(xuiStepBarGetCurrent(pArrow) == 1, "current");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pArrow, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_PROGRESS &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_READONLY) != 0u &&
		tAccessible.tRange.iSize == sizeof(tAccessible.tRange) &&
		tAccessible.tRange.fMin == 1.0 && tAccessible.tRange.fMax == 3.0 &&
		tAccessible.tRange.fValue == 2.0 && strcmp(tAccessible.sValue, "2/3 Second") == 0,
		"step bar accessibility state");
	iAccessibilityRevision = xuiWidgetGetAccessibilityRevision(pArrow);
	iRet = xuiWidgetMeasureContent(pArrow, (xui_vec2_t){1000.0f, 1000.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 420.0f && tMeasured.fY == 30.0f,
		"large constraint preserves arrow preferred size");
	iRet = xuiWidgetMeasureContent(pArrow, (xui_vec2_t){100.0f, 20.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 100.0f && tMeasured.fY == 20.0f,
		"small constraint clamps arrow preferred size");

	tDesc.iStyle = XUI_STEP_BAR_STYLE_DOT;
	iRet = xuiStepBarCreate(pContext, &pDot, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDot != NULL, "dot create");
	xuiWidgetSetRect(pDot, (xui_rect_t){20.0f, 80.0f, 460.0f, 76.0f});
	iRet = xuiWidgetAddChild(pRoot, pDot);
	XUI_TEST_CHECK(iRet == XUI_OK, "add dot");
	iRet = xuiWidgetMeasureContent(pDot, (xui_vec2_t){1000.0f, 1000.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 420.0f && tMeasured.fY == 72.0f,
		"large constraint preserves dot preferred size");

	tDesc.iStyle = XUI_STEP_BAR_STYLE_VERTICAL;
	iRet = xuiStepBarCreate(pContext, &pVertical, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pVertical != NULL, "vertical create");
	xuiWidgetSetRect(pVertical, (xui_rect_t){20.0f, 170.0f, 180.0f, 130.0f});
	iRet = xuiWidgetAddChild(pRoot, pVertical);
	XUI_TEST_CHECK(iRet == XUI_OK, "add vertical");
	iRet = xuiWidgetMeasureContent(pVertical, (xui_vec2_t){1000.0f, 1000.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 180.0f && tMeasured.fY == 216.0f,
		"large constraint preserves vertical preferred size");

	iRet = xuiStepBarSetCurrent(pArrow, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetCurrent(pArrow) == 2, "set current");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiWidgetGetAccessibilityRevision(pArrow) > iAccessibilityRevision,
		"current change queues accessibility event");
	iRet = xuiStepBarSetCurrent(pArrow, 3);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid current");
	iRet = xuiStepBarSetStyle(pArrow, XUI_STEP_BAR_STYLE_DOT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetStyle(pArrow) == XUI_STEP_BAR_STYLE_DOT, "set style");
	iRet = xuiStepBarSetTitle(pArrow, 0, "Start");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiStepBarGetTitle(pArrow, 0), "Start") == 0, "set title");
	{
		int iChangeCount = xuiStepBarGetChangeCount(pArrow);
		iRet = xuiStepBarSetTitle(pArrow, 0, "Start");
		XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetChangeCount(pArrow) == iChangeCount,
			"same title is idempotent");
	}
	iRet = xuiStepBarSetColors(pArrow, XUI_COLOR_RGBA(100, 190, 120, 255), XUI_COLOR_RGBA(50, 160, 90, 255), XUI_COLOR_RGBA(220, 220, 220, 255), XUI_COLOR_RGBA(200, 200, 200, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiStepBarSetMetrics(pArrow, 32.0f, 10.0f, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiStepBarGetMetrics(pArrow, &fBarHeight, &fDotRadius, &fLineWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fBarHeight == 32.0f && fDotRadius == 10.0f && fLineWidth == 2.0f, "get metrics");
	iRet = xuiStepBarSetMetrics(pArrow, NAN, 11.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan metrics");
	iRet = xuiStepBarSetMetrics(pArrow, 33.0f, INFINITY, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metrics");
	iRet = xuiStepBarSetMetrics(pArrow, 33.0f, FLT_MAX, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject out of layout range metrics");
	iRet = xuiStepBarGetMetrics(pArrow, &fBarHeight, &fDotRadius, &fLineWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fBarHeight == 32.0f && fDotRadius == 10.0f && fLineWidth == 2.0f, "invalid metrics preserve state");
	tInvalidDesc = tDesc;
	tInvalidDesc.fDotRadius = NAN;
	pInvalid = pRoot;
	iRet = xuiStepBarCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nan descriptor");
	tInvalidDesc = tDesc;
	tInvalidDesc.fLineWidth = FLT_MAX;
	pInvalid = pRoot;
	iRet = xuiStepBarCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject out of layout range descriptor metric");
	tInvalidDesc = tDesc;
	tInvalidDesc.iCurrent = tInvalidDesc.iStepCount;
	pInvalid = pRoot;
	iRet = xuiStepBarCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject out of range descriptor current");
	memset(&tScope, 0, sizeof(tScope));
	XUI_TEST_CHECK(xrtAllocationBudgetCreate(1u, &pBudget), "create allocation budget");
	XUI_TEST_CHECK(xrtAllocationBudgetEnter(pBudget, &tScope), "enter allocation budget");
	{
		const char* arrReplacement[2] = {"Replacement", "Other"};
		iRet = xuiStepBarSetSteps(pArrow, arrReplacement, 2);
	}
	XUI_TEST_CHECK(xrtAllocationBudgetLeave(&tScope), "leave allocation budget");
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && xuiStepBarGetStepCount(pArrow) == 3 &&
		strcmp(xuiStepBarGetTitle(pArrow, 0), "Start") == 0 && xuiStepBarGetCurrent(pArrow) == 2,
		"step replacement allocation failure is atomic");
	memset(&tScope, 0, sizeof(tScope));
	XUI_TEST_CHECK(xrtAllocationBudgetCreate(1u, &pBudget), "create empty title allocation budget");
	XUI_TEST_CHECK(xrtAllocationBudgetEnter(pBudget, &tScope), "enter empty title allocation budget");
	iRet = xuiStepBarSetSteps(pArrow, NULL, XUI_STEP_BAR_MAX_STEPS);
	XUI_TEST_CHECK(xrtAllocationBudgetLeave(&tScope), "leave empty title allocation budget");
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetStepCount(pArrow) == XUI_STEP_BAR_MAX_STEPS &&
		xuiStepBarGetTitle(pArrow, 0)[0] == '\0' && xuiStepBarGetCurrent(pArrow) == 2,
		"empty titles require no per-step allocation");
	iRet = xuiStepBarSetSteps(pArrow, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetCurrent(pArrow) == 0, "clear steps resets current");
	iRet = xuiStepBarSetCurrent(pArrow, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "empty step bar rejects nonzero current");
	iRet = xuiStepBarSetCurrent(pArrow, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "empty step bar accepts zero current");
	iRet = xuiStepBarSetSteps(pArrow, arrTitles, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore steps");
	XUI_TEST_CHECK(xuiStepBarGetChangeCount(pArrow) >= 3, "change count");
	iRet = xuiStepBarSetStyle(pArrow, XUI_STEP_BAR_STYLE_ARROW);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore arrow style");
	iRet = xuiWidgetSetRect(pArrow, (xui_rect_t){20, 20, 101, 36});
	XUI_TEST_CHECK(iRet == XUI_OK, "set non-divisible arrow width");
	tRect = xuiStepBarGetStepRect(pArrow, 0);
	tNextRect = xuiStepBarGetStepRect(pArrow, 1);
	tLastRect = xuiStepBarGetStepRect(pArrow, 2);
	XUI_TEST_CHECK(tRect.fX == 0 && tRect.fX + tRect.fW == tNextRect.fX &&
		tNextRect.fX + tNextRect.fW == tLastRect.fX &&
		tLastRect.fX + tLastRect.fW == 101,
		"arrow steps partition every content pixel without gaps");
	sLongTitle =
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyz";
	arrLongTitle[0] = sLongTitle;
	iRet = xuiStepBarSetSteps(pArrow, arrLongTitle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiStepBarGetTitle(pArrow, 0), sLongTitle) == 0,
		"long arrow title remains available without truncation");
	iRet = xuiWidgetSetRect(pArrow, (xui_rect_t){20, 20, 460, 36});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore arrow rect");
	iRet = xuiStepBarSetMetrics(pDot, 32.0f, 100.0f, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set oversized horizontal dot radius");
	iRet = xuiStepBarSetColors(pDot, XUI_COLOR_RGBA(30, 140, 70, 255),
		XUI_COLOR_RGBA(30, 100, 200, 255), XUI_COLOR_RGBA(210, 120, 40, 255),
		XUI_COLOR_RGBA(90, 90, 90, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set distinct dot colors");
	iRet = xuiStepBarSetTextColors(pDot, XUI_COLOR_RGBA(20, 80, 40, 255),
		XUI_COLOR_RGBA(20, 60, 140, 255), XUI_COLOR_RGBA(110, 40, 150, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set distinct dot text colors");
	xuiWidgetSetRect(pDot, (xui_rect_t){20.0f, 80.0f, 20.0f, 20.0f});
	iRet = xuiStepBarSetMetrics(pVertical, 32.0f, 100.0f, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set oversized vertical dot radius");
	xuiWidgetSetRect(pVertical, (xui_rect_t){20.0f, 170.0f, 20.0f, 20.0f});

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pArrow, 0) != NULL, "arrow cache");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pDot, 0) != NULL, "dot cache");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pVertical, 0) != NULL, "vertical cache");
	sRenderedText = xuiTestSurfaceGetLastText(xuiWidgetGetCacheSurface(pArrow, 0));
	XUI_TEST_CHECK(sRenderedText != NULL && strncmp(sRenderedText, "1.", 2u) == 0 &&
		strcmp(sRenderedText + 2, sLongTitle) == 0,
		"arrow rendering does not truncate long copied titles");
	tRect = xuiStepBarGetStepRect(pDot, 1);
	XUI_TEST_CHECK(tRect.fW > 0.0f && tRect.fH > 0.0f, "dot step rect");
	tRect = xuiStepBarGetIndicatorRect(pDot, 0);
	XUI_TEST_CHECK(tRect.fX >= 0.0f && tRect.fY >= 0.0f &&
	               tRect.fX + tRect.fW <= 20.0f && tRect.fY + tRect.fH <= 20.0f,
	               "horizontal indicator stays inside tiny content");
	tNextRect = xuiStepBarGetIndicatorRect(pDot, 1);
	XUI_TEST_CHECK(tRect.fX + tRect.fW <= tNextRect.fX,
		"horizontal indicators do not overlap in tiny content");
	tRect = xuiStepBarGetIndicatorRect(pVertical, 1);
	XUI_TEST_CHECK(tRect.fX >= 0.0f && tRect.fY >= 0.0f &&
	               tRect.fX + tRect.fW <= 20.0f && tRect.fY + tRect.fH <= 20.0f,
	               "vertical indicator stays inside tiny content");
	tRect = xuiStepBarGetIndicatorRect(pVertical, 0);
	tNextRect = xuiStepBarGetIndicatorRect(pVertical, 1);
	XUI_TEST_CHECK(tRect.fY + tRect.fH <= tNextRect.fY,
		"vertical indicators do not overlap in tiny content");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(xuiWidgetGetCacheSurface(pDot, 0),
		XUI_COLOR_RGBA(210, 120, 40, 255)) > 0,
		"pending indicator uses pending color");

	xuiWidgetSetRect(pDot, (xui_rect_t){20.0f, 80.0f, 60.0f, 20.0f});
	tRect = xuiStepBarGetIndicatorRect(pDot, 2);
	XUI_TEST_CHECK(tRect.fX > 40.0f && tRect.fX + tRect.fW <= 60.0f,
		"geometry query reflects current widget size before render");

	g_bFailCircleStroke = 1;
	g_iErrorCount = 0;
	g_iLastError = XUI_OK;
	g_iLastErrorStage = XUI_ERROR_STAGE_GENERAL;
	g_pLastErrorWidget = NULL;
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "relayout before draw failure test");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && g_bFailCircleStroke == 0 && g_iErrorCount == 1 &&
		g_iLastError == XUI_ERROR_OUT_OF_MEMORY && g_iLastErrorStage == XUI_ERROR_STAGE_CACHE &&
		g_pLastErrorWidget == pDot,
		"circle draw failure reaches recoverable error channel");

cleanup:
	if ( pBudget != NULL ) {
		xrtAllocationBudgetFree(pBudget);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_step_bar_test passed\n");
	return 0;
}
