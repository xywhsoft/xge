#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

static xui_style_value_t __xuiPanelStyleInt(int iValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_INT;
	tValue.iInt = iValue;
	return tValue;
}

static xui_style_value_t __xuiPanelStyleBool(int bValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_BOOL;
	tValue.iInt = bValue ? 1 : 0;
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

static int __xuiPanelDestroyOnVisible(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pPanel = (xui_widget)pUser;

	(void)pWidget;
	if ( (pEvent != NULL) && (pEvent->iPhase == XUI_EVENT_PHASE_TARGET) ) {
		xuiWidgetDestroy(pPanel);
	}
	return XUI_OK;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPanel;
	xui_widget pLifecyclePanel;
	xui_widget pClient;
	xui_widget pLabel;
	xui_widget pTitle;
	xui_surface pIconSurface;
	xui_surface pCache;
	xui_surface pTitleCache;
	xui_font pFont;
	xui_panel_desc_t tDesc;
	xui_panel_desc_t tInvalidDesc;
	xui_label_desc_t tLabelDesc;
	xui_style_property_t arrStyle[8];
	xui_accessible_node_t tNode;
	xui_rect_t tRect;
	xui_vec2_t tContentSize;
	uint32_t iAccessibilityRevision;
	uint32_t iDirtyFlags;
	float fBorderWidth;
	uint32_t iBorderColor;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pPanel = NULL;
	pLifecyclePanel = NULL;
	pClient = NULL;
	pLabel = NULL;
	pTitle = NULL;
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

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fHeaderHeight = NAN;
	pLifecyclePanel = pRoot;
	iRet = xuiPanelCreate(pContext, &pLifecyclePanel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecyclePanel == NULL, "reject NaN descriptor");
	tInvalidDesc.fHeaderHeight = 0.0f;
	tInvalidDesc.tIconSrc.fW = -1;
	pLifecyclePanel = pRoot;
	iRet = xuiPanelCreate(pContext, &pLifecyclePanel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecyclePanel == NULL, "reject negative icon source");
	tInvalidDesc.tIconSrc.fW = 0;
	tInvalidDesc.tIconSrc.fX = (int)XUI_LAYOUT_UNBOUNDED + 1024;
	pLifecyclePanel = pRoot;
	iRet = xuiPanelCreate(pContext, &pLifecyclePanel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecyclePanel == NULL,
		"reject out-of-range icon source descriptor");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Settings";
	tDesc.pFont = pFont;
	tDesc.iTitleColor = XUI_COLOR_RGBA(20, 40, 60, 255);
	tDesc.fHeaderHeight = 30.0f;
	tDesc.fIconSize = 18.0f;
	iRet = xuiPanelCreate(pContext, &pPanel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPanel != NULL, "panel create");
	iRet = xuiWidgetMeasureContent(pPanel,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tContentSize);
	XUI_TEST_CHECK(iRet == XUI_OK && tContentSize.fX == 180.0f && tContentSize.fY == 94.0f,
		"panel intrinsic content measure");
	iRet = xuiWidgetMeasureContent(pPanel, (xui_vec2_t){72.0f, 40.0f}, &tContentSize);
	XUI_TEST_CHECK(iRet == XUI_OK && tContentSize.fX == 72.0f && tContentSize.fY == 40.0f,
		"panel content measure respects tight constraints");
	iRet = xuiWidgetSetRect(pPanel, (xui_rect_t){12.0f, 16.0f, 220.0f, 150.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "panel rect");
	iRet = xuiWidgetAddChild(pRoot, pPanel);
	XUI_TEST_CHECK(iRet == XUI_OK, "panel add");

	XUI_TEST_CHECK(xuiPanelGetHeaderWidget(pPanel) != NULL, "header widget");
	XUI_TEST_CHECK(xuiPanelGetIconWidget(pPanel) != NULL, "icon widget");
	pTitle = xuiPanelGetTitleWidget(pPanel);
	XUI_TEST_CHECK(pTitle != NULL, "title widget");
	pClient = xuiPanelGetClientWidget(pPanel);
	XUI_TEST_CHECK(pClient != NULL, "client widget");
	XUI_TEST_CHECK(strcmp(xuiPanelGetTitle(pPanel), "Settings") == 0, "title text");
	XUI_TEST_CHECK(xuiPanelGetTitleColor(pPanel) == XUI_COLOR_RGBA(20, 40, 60, 255), "title color");
	XUI_TEST_CHECK(xuiPanelGetHeaderHeight(pPanel) == 30.0f, "header height getter");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pPanel, 0, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		strcmp(tNode.sName, "Settings") == 0, "panel accessibility title");
	iAccessibilityRevision = xuiWidgetGetAccessibilityRevision(pPanel);
	iRet = xuiPanelSetTitle(pPanel, "Preferences");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiPanelGetTitle(pPanel), "Preferences") == 0,
		"title update");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush title accessibility");
	XUI_TEST_CHECK(xuiWidgetGetAccessibilityRevision(pPanel) > iAccessibilityRevision,
		"title updates panel accessibility");
	iRet = xuiPanelSetTitle(pPanel, "Settings");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore title");
	iRet = xuiWidgetSetAccessibleName(pPanel, "Custom panel");
	XUI_TEST_CHECK(iRet == XUI_OK, "set explicit accessible name");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pPanel, 0, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tNode.sName, "Custom panel") == 0,
		"explicit accessible name overrides title");
	iRet = xuiWidgetSetAccessibleName(pPanel, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear explicit accessible name");

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
	iRet = xuiPanelSetIcon(pPanel, NULL,
		(xui_rect_t){(int)XUI_LAYOUT_UNBOUNDED + 1024, 0, 16, 16});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiPanelGetIconSurface(pPanel) == pIconSurface &&
		__xuiPanelRectEq(xuiPanelGetIconSource(pPanel), 0.0f, 0.0f, 16.0f, 16.0f) &&
		xuiWidgetGetVisible(xuiPanelGetIconWidget(pPanel)),
		"invalid icon update preserves surface, source, and visibility");
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
	iRet = xuiWidgetSetEnabled(pPanel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWidgetGetEffectiveEnabled(pTitle) &&
		(xuiWidgetGetDirtyFlags(pTitle) & XUI_WIDGET_DIRTY_CACHE) != 0,
		"disable panel invalidates title cache");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render");
	pTitleCache = xuiWidgetGetCacheSurface(pTitle, xuiWidgetGetStateId(pTitle));
	XUI_TEST_CHECK(pTitleCache != NULL, "disabled title cache");
	iRet = xuiWidgetSetEnabled(pPanel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable panel");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "enabled render");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetEnabled(pPanel) &&
		!xuiWidgetGetEffectiveEnabled(pPanel) &&
		(xuiPanelGetState(pPanel) & XUI_WIDGET_STATE_DISABLED) != 0,
		"panel state reflects inherited disable");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiPanelGetState(pPanel) & XUI_WIDGET_STATE_DISABLED) == 0,
		"panel state clears inherited disable");
	iDirtyFlags = xuiWidgetGetDirtyFlags(pPanel);
	iRet = xuiPanelSetFont(pPanel, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetFont(pPanel) == NULL &&
		xuiLabelGetFont(pTitle) == pFont && xuiWidgetGetDirtyFlags(pPanel) == iDirtyFlags,
		"default font fallback remains resolved without redundant invalidation");
	iRet = xuiPanelSetFont(pPanel, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiLabelGetFont(pTitle) == pFont &&
		xuiWidgetGetDirtyFlags(pPanel) == iDirtyFlags,
		"repeated default font setter is idempotent");
	iRet = xuiPanelSetHeaderColor(pPanel, xuiPanelGetHeaderColor(pPanel));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pPanel) == iDirtyFlags,
		"idempotent color keeps dirty state");
	iRet = xuiPanelSetHeaderHeight(pPanel, xuiPanelGetHeaderHeight(pPanel));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pPanel) == iDirtyFlags,
		"idempotent metric keeps dirty state");
	iRet = xuiWidgetSetGap(xuiPanelGetHeaderWidget(pPanel), 17.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "customize header gap");
	iRet = xuiPanelSetHeaderGap(pPanel, xuiPanelGetHeaderGap(pPanel));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetGap(xuiPanelGetHeaderWidget(pPanel)) ==
		xuiPanelGetHeaderGap(pPanel), "panel setter repairs customized child");
	XUI_TEST_CHECK(xuiPanelSetBorder(pPanel, NAN, 0) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN border");
	XUI_TEST_CHECK(xuiPanelSetHeaderGap(pPanel, NAN) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN gap");
	XUI_TEST_CHECK(xuiPanelSetIconSize(pPanel, NAN) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN icon size");

	arrStyle[0] = __xuiPanelStyleProp("panel.header.height", __xuiPanelStyleFloat(24.0f));
	arrStyle[1] = __xuiPanelStyleProp("panel.header.gap", __xuiPanelStyleFloat(3.0f));
	arrStyle[2] = __xuiPanelStyleProp("panel.icon.size", __xuiPanelStyleFloat(12.0f));
	arrStyle[3] = __xuiPanelStyleProp("panel.border.color", __xuiPanelStyleColor(XUI_COLOR_RGBA(18, 88, 150, 255)));
	arrStyle[4] = __xuiPanelStyleProp("text.color", __xuiPanelStyleColor(XUI_COLOR_RGBA(10, 30, 80, 255)));
	arrStyle[5] = __xuiPanelStyleProp("text.disabled_color", __xuiPanelStyleColor(XUI_COLOR_RGBA(90, 100, 110, 255)));
	arrStyle[6] = __xuiPanelStyleProp("text.flags", __xuiPanelStyleInt(XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE));
	arrStyle[7] = __xuiPanelStyleProp("panel.client.clip", __xuiPanelStyleBool(0));
	iRet = xuiWidgetSetInlineStyle(pPanel, arrStyle, 8);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style layout");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetHeaderRect(pPanel), 0.0f, 0.0f, 220.0f, 24.0f), "style header height");
	XUI_TEST_CHECK(xuiPanelGetIconRect(pPanel).fW == 12.0f, "style icon size");
	XUI_TEST_CHECK(xuiWidgetGetGap(xuiPanelGetHeaderWidget(pPanel)) == 3.0f &&
		xuiLabelGetTextColor(pTitle) == XUI_COLOR_RGBA(10, 30, 80, 255) &&
		xuiLabelGetDisabledTextColor(pTitle) == XUI_COLOR_RGBA(90, 100, 110, 255) &&
		xuiLabelGetTextFlags(pTitle) ==
			(XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) &&
		xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_VISIBLE,
		"style values synchronize internal widgets");

	iRet = xuiPanelSetTitleColor(pPanel, XUI_COLOR_RGBA(30, 60, 90, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set base title color under style");
	iRet = xuiPanelSetDisabledTitleColor(pPanel, XUI_COLOR_RGBA(120, 130, 140, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set base disabled title color under style");
	iRet = xuiPanelSetTitleAlign(pPanel, XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE);
	XUI_TEST_CHECK(iRet == XUI_OK, "set base title flags under style");
	iRet = xuiPanelSetHeaderHeight(pPanel, 36.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set base header height under style");
	iRet = xuiPanelSetHeaderGap(pPanel, 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set base header gap under style");
	iRet = xuiPanelSetIconSize(pPanel, 22.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set base icon size under style");
	iRet = xuiPanelSetClientClip(pPanel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set base client clip under style");
	XUI_TEST_CHECK(xuiWidgetGetPreferredSize(xuiPanelGetHeaderWidget(pPanel)).fY == 24.0f &&
		xuiWidgetGetGap(xuiPanelGetHeaderWidget(pPanel)) == 3.0f &&
		xuiWidgetGetPreferredSize(xuiPanelGetIconWidget(pPanel)).fX == 12.0f &&
		xuiLabelGetTextColor(pTitle) == XUI_COLOR_RGBA(10, 30, 80, 255) &&
		xuiLabelGetDisabledTextColor(pTitle) == XUI_COLOR_RGBA(90, 100, 110, 255) &&
		xuiLabelGetTextFlags(pTitle) ==
			(XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) &&
		xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_VISIBLE,
		"base setters preserve active resolved style");

	iRet = xuiWidgetSetInlineStyle(pPanel, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear style");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "base layout after clearing style");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetHeaderRect(pPanel), 0.0f, 0.0f, 220.0f, 36.0f) &&
		xuiPanelGetIconRect(pPanel).fW == 22.0f &&
		xuiWidgetGetGap(xuiPanelGetHeaderWidget(pPanel)) == 9.0f &&
		xuiLabelGetTextColor(pTitle) == XUI_COLOR_RGBA(30, 60, 90, 255) &&
		xuiLabelGetDisabledTextColor(pTitle) == XUI_COLOR_RGBA(120, 130, 140, 255) &&
		xuiLabelGetTextFlags(pTitle) ==
			(XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) &&
		xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_CLIP,
		"clearing style restores updated base values");

	iRet = xuiPanelSetHeaderHeight(pPanel, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide header");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden layout");
	XUI_TEST_CHECK(!xuiWidgetGetVisible(xuiPanelGetHeaderWidget(pPanel)), "header hidden");
	XUI_TEST_CHECK(__xuiPanelRectEq(xuiPanelGetClientRect(pPanel), 0.0f, 0.0f, 220.0f, 150.0f), "hidden client rect");

	iRet = xuiPanelSetClientClip(pPanel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPanelGetClientClip(pPanel) && xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_VISIBLE, "client clip off");
	iRet = xuiPanelSetClientClip(pPanel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetClientClip(pPanel) && xuiWidgetGetOverflow(pClient) == XUI_OVERFLOW_CLIP, "client clip on");

	iRet = xuiPanelCreate(pContext, &pLifecyclePanel, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pLifecyclePanel != NULL, "lifecycle panel create");
	iRet = xuiWidgetAddChild(pRoot, pLifecyclePanel);
	XUI_TEST_CHECK(iRet == XUI_OK, "lifecycle panel add");
	iRet = xuiWidgetSetEventHandler(xuiPanelGetHeaderWidget(pLifecyclePanel),
		XUI_EVENT_VISIBLE_CHANGED, __xuiPanelDestroyOnVisible, pLifecyclePanel);
	XUI_TEST_CHECK(iRet == XUI_OK, "lifecycle handler");
	iRet = xuiPanelSetHeaderHeight(pLifecyclePanel, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPanelGetClientWidget(pLifecyclePanel) == NULL,
		"visible callback may destroy panel");
	pLifecyclePanel = NULL;

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
