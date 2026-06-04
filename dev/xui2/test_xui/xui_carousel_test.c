#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_carousel_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_carousel_test_state_t {
	int iChanges;
	int iOld;
	int iNew;
} xui_carousel_test_state_t;

static void __xuiCarouselTestChanged(xui_widget pWidget, int iOldIndex, int iNewIndex, void* pUser)
{
	xui_carousel_test_state_t* pState;

	(void)pWidget;
	pState = (xui_carousel_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iChanges++;
		pState->iOld = iOldIndex;
		pState->iNew = iNewIndex;
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_carousel_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCarousel;
	xui_widget pChild;
	xui_surface pTarget;
	xui_font pFont;
	xui_carousel_desc_t tDesc;
	xui_rect_i_t tFullRect;
	int bAuto;
	float fInterval;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCarousel = NULL;
	pChild = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	xuiTestProxyInit(&tProxyState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "carousel", 8, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPageCount = 4;
	tDesc.iCurrent = 2;
	tDesc.pFont = pFont;
	tDesc.fAutoInterval = 0.1f;
	iRet = xuiCarouselCreate(pContext, &pCarousel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCarousel != NULL, "carousel create");
	(void)xuiWidgetSetRect(pCarousel, (xui_rect_t){20.0f, 20.0f, 460.0f, 260.0f});
	iRet = xuiCarouselSetChange(pCarousel, __xuiCarouselTestChanged, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	iRet = xuiWidgetAddChild(pRoot, pCarousel);
	XUI_TEST_CHECK(iRet == XUI_OK, "add carousel");

	XUI_TEST_CHECK(xuiCarouselGetPageCount(pCarousel) == 4, "page count");
	XUI_TEST_CHECK(xuiCarouselGetCurrent(pCarousel) == 2, "initial current");
	XUI_TEST_CHECK(xuiCarouselGetPageWidget(pCarousel, 0) != NULL, "page widget 0");
	XUI_TEST_CHECK(xuiCarouselGetPageWidget(pCarousel, 3) != NULL, "page widget 3");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiCarouselGetPageWidget(pCarousel, 2)) != 0, "current page visible");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiCarouselGetPageWidget(pCarousel, 0)) == 0, "inactive page hidden");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	iRet = xuiCarouselAddPageChild(pCarousel, 1, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pChild) == xuiCarouselGetPageWidget(pCarousel, 1), "add page child");
	pChild = NULL;

	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "next");
	XUI_TEST_CHECK(tState.iChanges == 1 && tState.iOld == 2 && tState.iNew == 3, "next callback");
	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "loop next");
	iRet = xuiCarouselPrev(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "loop prev");
	iRet = xuiCarouselSetLoop(pCarousel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetLoop(pCarousel) == 0, "set loop false");
	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "next at end without loop");
	iRet = xuiCarouselSetLoop(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set loop true");
	iRet = xuiCarouselSetCurrent(pCarousel, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "set current");
	iRet = xuiCarouselSetAutoPlay(pCarousel, 1, 0.1f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set autoplay");
	iRet = xuiCarouselGetAutoPlay(pCarousel, &bAuto, &fInterval);
	XUI_TEST_CHECK(iRet == XUI_OK && bAuto && fInterval >= 0.09f, "get autoplay");
	iRet = xuiUpdate(pContext, 0.11f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 1, "auto advance");
	iRet = xuiCarouselSetMetrics(pCarousel, 34.0f, 8.0f, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiCarouselSetColors(pCarousel,
		XUI_COLOR_RGBA(255, 82, 36, 255),
		XUI_COLOR_RGBA(20, 20, 20, 120),
		XUI_COLOR_RGBA(20, 20, 20, 180),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(255, 255, 255, 140),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(255, 255, 255, 210));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	XUI_TEST_CHECK(xuiCarouselGetChangeCount(pCarousel) >= 3, "change count");

	iRet = xuiTestSurfaceCreate(&tProxyState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pCarousel, 0) != NULL, "carousel cache");

cleanup:
	if ( pTarget != NULL ) {
		tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_carousel_test passed\n");
	return 0;
}
