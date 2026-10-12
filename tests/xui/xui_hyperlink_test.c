#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_hyperlink_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiHyperlinkClicked(xui_widget pWidget, void* pUser)
{
	int* pCount;

	(void)pWidget;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static xui_style_value_t __xuiHyperlinkTestColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_property_t __xuiHyperlinkTestProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static xui_style_desc_t __xuiHyperlinkTestStyle(const xui_style_property_t* pProps, int iCount)
{
	xui_style_desc_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.pProperties = pProps;
	tStyle.iPropertyCount = iCount;
	return tStyle;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pLink;
	xui_font pFont;
	xui_hyperlink_desc_t tDesc;
	xui_style_property_t arrProps[1];
	xui_style_desc_t tStyle;
	xui_vec2_t tMeasure;
	xui_surface pNormalCache;
	xui_surface pHoverCache;
	xui_surface pActiveCache;
	int iClickCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pLink = NULL;
	pFont = NULL;
	iClickCount = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 240.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "input viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "font", 4, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 120.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Open document";
	tDesc.pFont = pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(20, 92, 210, 255);
	tDesc.iHoverTextColor = XUI_COLOR_RGBA(8, 128, 230, 255);
	tDesc.iActiveTextColor = XUI_COLOR_RGBA(6, 60, 160, 255);
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(120, 130, 140, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.bHoverUnderline = 1;
	tDesc.bActiveUnderline = 1;
	iRet = xuiHyperlinkCreate(pContext, &pLink, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLink != NULL, "hyperlink create");
	XUI_TEST_CHECK(strcmp(xuiHyperlinkGetText(pLink), "Open document") == 0, "text getter");
	XUI_TEST_CHECK(xuiHyperlinkGetTextColor(pLink) == XUI_COLOR_RGBA(20, 92, 210, 255), "text color getter");
	XUI_TEST_CHECK(xuiHyperlinkGetHoverUnderline(pLink) == 1 && xuiHyperlinkGetActiveUnderline(pLink) == 1, "underline defaults");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateCount(pLink) == 3, "three cache states");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 0) == 0, "normal cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 1) == XUI_WIDGET_STATE_HOVER, "hover cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 2) == XUI_WIDGET_STATE_ACTIVE, "active cache id");
	iRet = xuiHyperlinkSetClick(pLink, __xuiHyperlinkClicked, &iClickCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "click callback");
	iRet = xuiWidgetSetRect(pLink, (xui_rect_t){20.0f, 20.0f, 160.0f, 24.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "link rect");
	iRet = xuiWidgetAddChild(pRoot, pLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "add link");

	iRet = xuiWidgetMeasureContent(pLink, (xui_vec2_t){300.0f, 100.0f}, &tMeasure);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasure.fX > 20.0f && tMeasure.fY == 16.0f, "measure content");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	pHoverCache = xuiWidgetGetCacheSurface(pLink, XUI_WIDGET_STATE_HOVER);
	pActiveCache = xuiWidgetGetCacheSurface(pLink, XUI_WIDGET_STATE_ACTIVE);
	XUI_TEST_CHECK(pNormalCache != NULL && pHoverCache != NULL && pActiveCache != NULL, "cache surfaces");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(20, 92, 210, 255), "normal color cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pHoverCache) == XUI_COLOR_RGBA(8, 128, 230, 255), "hover color cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pActiveCache) == XUI_COLOR_RGBA(6, 60, 160, 255), "active color cache");

	iRet = xuiInputPointerMove(pContext, 30.0f, 30.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_HOVER) != 0, "hover state");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pLink) == XUI_WIDGET_STATE_HOVER, "hover visual state");
	iRet = xuiInputPointerDown(pContext, 30.0f, 30.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch down");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0, "active state");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pLink) == XUI_WIDGET_STATE_ACTIVE, "active visual state");
	iRet = xuiInputPointerUp(pContext, 30.0f, 30.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch up");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 1 && iClickCount == 1, "pointer click");

	iRet = xuiSetFocusWidget(pContext, pLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space down");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0, "keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space up");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 2 && iClickCount == 2, "keyboard click");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "enter default");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch enter default");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 3 && iClickCount == 3, "enter default click");

	iRet = xuiWidgetSetEnabled(pLink, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(120, 130, 140, 255), "disabled color");
	iRet = xuiWidgetSetEnabled(pLink, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable");

	arrProps[0] = __xuiHyperlinkTestProp("hyperlink.text.color", __xuiHyperlinkTestColor(XUI_COLOR_RGBA(30, 170, 100, 255)));
	tStyle = __xuiHyperlinkTestStyle(arrProps, 1);
	iRet = xuiStyleSetClass(pContext, "green-link", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "style set");
	iRet = xuiWidgetAddStyleClass(pLink, "green-link");
	XUI_TEST_CHECK(iRet == XUI_OK, "style add");
	iRet = xuiInputPointerMove(pContext, 220.0f, 90.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move out");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move out");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style render");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(30, 170, 100, 255), "style color");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_hyperlink_test passed\n");
	return 0;
}
