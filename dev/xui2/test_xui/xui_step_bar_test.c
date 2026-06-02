#include "xui.h"
#include "xui_test_proxy.h"

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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pArrow;
	xui_widget pDot;
	xui_widget pVertical;
	xui_surface pTarget;
	xui_font pFont;
	xui_step_bar_desc_t tDesc;
	const char* arrTitles[3];
	xui_rect_t tRect;
	xui_rect_i_t tFullRect;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pArrow = NULL;
	pDot = NULL;
	pVertical = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
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

	tDesc.iStyle = XUI_STEP_BAR_STYLE_DOT;
	iRet = xuiStepBarCreate(pContext, &pDot, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDot != NULL, "dot create");
	xuiWidgetSetRect(pDot, (xui_rect_t){20.0f, 80.0f, 460.0f, 76.0f});
	iRet = xuiWidgetAddChild(pRoot, pDot);
	XUI_TEST_CHECK(iRet == XUI_OK, "add dot");

	tDesc.iStyle = XUI_STEP_BAR_STYLE_VERTICAL;
	iRet = xuiStepBarCreate(pContext, &pVertical, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pVertical != NULL, "vertical create");
	xuiWidgetSetRect(pVertical, (xui_rect_t){20.0f, 170.0f, 180.0f, 130.0f});
	iRet = xuiWidgetAddChild(pRoot, pVertical);
	XUI_TEST_CHECK(iRet == XUI_OK, "add vertical");

	iRet = xuiStepBarSetCurrent(pArrow, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetCurrent(pArrow) == 2, "set current");
	iRet = xuiStepBarSetCurrent(pArrow, 3);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid current");
	iRet = xuiStepBarSetStyle(pArrow, XUI_STEP_BAR_STYLE_DOT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStepBarGetStyle(pArrow) == XUI_STEP_BAR_STYLE_DOT, "set style");
	iRet = xuiStepBarSetTitle(pArrow, 0, "Start");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiStepBarGetTitle(pArrow, 0), "Start") == 0, "set title");
	iRet = xuiStepBarSetColors(pArrow, XUI_COLOR_RGBA(100, 190, 120, 255), XUI_COLOR_RGBA(50, 160, 90, 255), XUI_COLOR_RGBA(220, 220, 220, 255), XUI_COLOR_RGBA(200, 200, 200, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiStepBarSetMetrics(pArrow, 32.0f, 10.0f, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	XUI_TEST_CHECK(xuiStepBarGetChangeCount(pArrow) >= 3, "change count");

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
	tRect = xuiStepBarGetStepRect(pDot, 1);
	XUI_TEST_CHECK(tRect.fW > 0.0f && tRect.fH > 0.0f, "dot step rect");
	tRect = xuiStepBarGetIndicatorRect(pVertical, 1);
	XUI_TEST_CHECK(tRect.fW > 0.0f && tRect.fH > 0.0f, "vertical indicator rect");

cleanup:
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
