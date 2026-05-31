#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_panel_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiPanelRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static xui_style_value_t __xuiPanelStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiPanelStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiPanelStyleProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPanel;
	xui_widget pClient;
	xui_widget pLabel;
	xui_surface pIconSurface;
	xui_surface pCache;
	xui_font pFont;
	xui_panel_desc_t tDesc;
	xui_label_desc_t tLabelDesc;
	xui_style_property_t arrStyle[4];
	xui_rect_t tRect;
	float fBorderWidth;
	uint32_t iBorderColor;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pPanel = NULL;
	pClient = NULL;
	pLabel = NULL;
	pIconSurface = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 260.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Settings";
	tDesc.pFont = pFont;
	tDesc.iTitleColor = XUI_COLOR_RGBA(20, 40, 60, 255);
	tDesc.fHeaderHeight = 30.0f;
	tDesc.fIconSize = 18.0f;
	tDesc.fRadius = 5.0f;
	iRet = xuiPanelCreate(pContext, &pPanel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPanel != NULL, "panel create");
	iRet = xuiWidgetSetRect(pPanel, (xui_rect_t){12.0f, 16.0f, 220.0f, 150.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "panel rect");
	iRet = xuiWidgetAddChild(pRoot, pPanel);
	XUI_TEST_CHECK(iRet == XUI_OK, "panel add");

	XUI_TEST_CHECK(xuiPanelGetHeaderWidget(pPanel) != NULL, "header widget");
	XUI_TEST_CHECK(xuiPanelGetIconWidget(pPanel) != NULL, "icon widget");
	XUI_TEST_CHECK(xuiPanelGetTitleWidget(pPanel) != NULL, "title widget");
	pClient = xuiPanelGetClientWidget(pPanel);
	XUI_TEST_CHECK(pClient != NULL, "client widget");
	XUI_TEST_CHECK(strcmp(xuiPanelGetTitle(pPanel), "Settings") == 0, "title text");
	XUI_TEST_CHECK(xuiPanelGetTitleColor(pPanel) == XUI_COLOR_RGBA(20, 40, 60, 255), "title color");
	XUI_TEST_CHECK(xuiPanelGetHeaderHeight(pPanel) == 30.0f, "header height getter");

	memset(&tLabelDesc, 0, sizeof(tLabelDesc));
	tLabelDesc.iSize = sizeof(tLabelDesc);
	tLabelDesc.pFont = pFont;
	tLabelDesc.sText = "Client label";
	tLabelDesc.iTextColor = XUI_COLOR_RGBA(40, 50, 60, 255);
	iRet = xuiLabelCreate(pContext, &pLabel, &tLabelDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLabel != NULL, "label create");
	iRet = xuiPanelAddChild(pPanel, pLabel);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pLabel) == pClient, "panel add child to client");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetHeaderRect(pPanel), 0.0f, 0.0f, 220.0f, 30.0f), "header rect");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetClientRect(pPanel), 0.0f, 30.0f, 220.0f, 120.0f), "client rect");

	iRet = xuiTestSurfaceCreate(&tState, &pIconSurface, 16, 16, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pIconSurface != NULL, "icon surface");
	iRet = xuiPanelSetIcon(pPanel, pIconSurface, (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f});
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetIconSurface(pPanel) == pIconSurface, "set icon");
	iRet = xuiPanelSetIconSize(pPanel, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetIconSize(pPanel) == 20.0f, "icon size");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout icon");
	tRect = xuiPanelGetIconRect(pPanel);
	XUI_TEST_CHECK(tRect.fW == 20.0f && tRect.fH == 20.0f, "icon rect");

	iRet = xuiPanelSetHeaderColor(pPanel, XUI_COLOR_RGBA(210, 230, 250, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "header color");
	iRet = xuiPanelSetClientColor(pPanel, XUI_COLOR_RGBA(250, 252, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "client color");
	iRet = xuiPanelSetBorder(pPanel, 2.0f, XUI_COLOR_RGBA(70, 120, 180, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "border");
	iRet = xuiPanelGetBorder(pPanel, &fBorderWidth, &iBorderColor);
	XUI_TEST_CHECK(iRet == XUI_OK && fBorderWidth == 2.0f && iBorderColor == XUI_COLOR_RGBA(70, 120, 180, 255), "border getter");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	pCache = xuiWidgetGetCacheSurface(pPanel, xuiWidgetGetStateId(pPanel));
	XUI_TEST_CHECK(pCache != NULL, "panel cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) >= 4, "panel fills");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(70, 120, 180, 255), "border render color");

	arrStyle[0] = __xuiPanelStyleProp("panel.header.height", __xuiPanelStyleFloat(24.0f));
	arrStyle[1] = __xuiPanelStyleProp("panel.icon.size", __xuiPanelStyleFloat(12.0f));
	arrStyle[2] = __xuiPanelStyleProp("panel.border.color", __xuiPanelStyleColor(XUI_COLOR_RGBA(18, 88, 150, 255)));
	arrStyle[3] = __xuiPanelStyleProp("text.color", __xuiPanelStyleColor(XUI_COLOR_RGBA(10, 30, 80, 255)));
	iRet = xuiWidgetSetInlineStyle(pPanel, arrStyle, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style layout");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetHeaderRect(pPanel), 0.0f, 0.0f, 220.0f, 24.0f), "style header height");
	XUI_TEST_CHECK(xuiPanelGetIconRect(pPanel).fW == 12.0f, "style icon size");

	iRet = xuiPanelSetHeaderHeight(pPanel, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide header");
	iRet = xuiWidgetSetInlineStyle(pPanel, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear style");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden layout");
	XUI_TEST_CHECK(!xuiWidgetGetVisible(xuiPanelGetHeaderWidget(pPanel)), "header hidden");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetClientRect(pPanel), 0.0f, 0.0f, 220.0f, 150.0f), "hidden client rect");

	iRet = xuiPanelSetClientClip(pPanel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPanelGetClientClip(pPanel) && xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_VISIBLE, "client clip off");
	iRet = xuiPanelSetClientClip(pPanel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetClientClip(pPanel) && xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_CLIP, "client clip on");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pIconSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pIconSurface);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_panel_test passed\n");
	return 0;
}
