#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_separator_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiSeparatorRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static xui_style_value_t __xuiSeparatorStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiSeparatorStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_value_t __xuiSeparatorStyleInt(int iValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_INT;
	tValue.iInt = iValue;
	return tValue;
}

static xui_style_property_t __xuiSeparatorStyleProp(const char* sName, xui_style_value_t tValue)
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
	xui_widget pSeparator;
	xui_surface pCache;
	xui_separator_desc_t tDesc;
	xui_style_property_t arrStyle[4];
	xui_vec2_t tMeasured;
	xui_rect_t tRect;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pSeparator = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 240.0f, 160.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 160.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiSeparatorCreate(pContext, &pSeparator, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSeparator != NULL, "separator create");
	iRet = xuiWidgetSetRect(pSeparator, (xui_rect_t){20.0f, 20.0f, 100.0f, 14.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "separator rect");
	iRet = xuiWidgetAddChild(pRoot, pSeparator);
	XUI_TEST_CHECK(iRet == XUI_OK, "separator add");

	iRet = xuiWidgetMeasureContent(pSeparator, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 0.0f && tMeasured.fY == 1.0f, "default measure");
	XUI_TEST_CHECK(xuiSeparatorGetOrientation(pSeparator) == XUI_SEPARATOR_HORIZONTAL, "default orientation");
	XUI_TEST_CHECK(xuiSeparatorGetAlign(pSeparator) == XUI_ALIGN_CENTER, "default align");
	XUI_TEST_CHECK(xuiSeparatorGetLineStyle(pSeparator) == XUI_SEPARATOR_SOLID, "default line style");
	tRect = xuiSeparatorGetLineRect(pSeparator);
	XUI_TEST_CHECK(__xuiSeparatorRectEq(tRect, 0.0f, 7.0f, 100.0f, 1.0f), "default line rect");

	iRet = xuiSeparatorSetThickness(pSeparator, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set thickness");
	iRet = xuiSeparatorSetColor(pSeparator, XUI_COLOR_RGBA(80, 102, 130, 190));
	XUI_TEST_CHECK(iRet == XUI_OK, "set color");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare solid");
	pCache = xuiWidgetGetCacheSurface(pSeparator, xuiWidgetGetStateId(pSeparator));
	XUI_TEST_CHECK(pCache != NULL, "cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) == 1, "solid draw count");
	XUI_TEST_CHECK(__xuiSeparatorRectEq(xuiTestSurfaceGetLastRect(pCache), 0.0f, 6.0f, 100.0f, 2.0f), "solid draw rect");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(80, 102, 130, 190), "solid color");

	iRet = xuiSeparatorSetLineStyle(pSeparator, XUI_SEPARATOR_DOT);
	XUI_TEST_CHECK(iRet == XUI_OK, "dot style");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare dot");
	pCache = xuiWidgetGetCacheSurface(pSeparator, xuiWidgetGetStateId(pSeparator));
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) > 1, "dot segments");

	iRet = xuiSeparatorSetOrientation(pSeparator, XUI_SEPARATOR_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set vertical");
	iRet = xuiSeparatorSetAlign(pSeparator, XUI_ALIGN_END);
	XUI_TEST_CHECK(iRet == XUI_OK, "set align end");
	iRet = xuiSeparatorSetLineStyle(pSeparator, XUI_SEPARATOR_SOLID);
	XUI_TEST_CHECK(iRet == XUI_OK, "set solid");
	iRet = xuiSeparatorSetThickness(pSeparator, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set thickness 3");
	iRet = xuiWidgetSetRect(pSeparator, (xui_rect_t){20.0f, 20.0f, 14.0f, 80.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical rect");
	iRet = xuiWidgetMeasureContent(pSeparator, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 3.0f && tMeasured.fY == 0.0f, "vertical measure");
	tRect = xuiSeparatorGetLineRect(pSeparator);
	XUI_TEST_CHECK(__xuiSeparatorRectEq(tRect, 11.0f, 0.0f, 3.0f, 80.0f), "vertical line rect");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare vertical");
	pCache = xuiWidgetGetCacheSurface(pSeparator, xuiWidgetGetStateId(pSeparator));
	XUI_TEST_CHECK(__xuiSeparatorRectEq(xuiTestSurfaceGetLastRect(pCache), 11.0f, 0.0f, 3.0f, 80.0f), "vertical draw rect");

	arrStyle[0] = __xuiSeparatorStyleProp("separator.orientation", __xuiSeparatorStyleInt(XUI_SEPARATOR_HORIZONTAL));
	arrStyle[1] = __xuiSeparatorStyleProp("separator.align", __xuiSeparatorStyleInt(XUI_ALIGN_END));
	arrStyle[2] = __xuiSeparatorStyleProp("separator.thickness", __xuiSeparatorStyleFloat(4.0f));
	arrStyle[3] = __xuiSeparatorStyleProp("separator.color", __xuiSeparatorStyleColor(XUI_COLOR_RGBA(40, 90, 180, 255)));
	iRet = xuiWidgetSetInlineStyle(pSeparator, arrStyle, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	tRect = xuiSeparatorGetLineRect(pSeparator);
	XUI_TEST_CHECK(__xuiSeparatorRectEq(tRect, 0.0f, 76.0f, 14.0f, 4.0f), "style line rect");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare style");
	pCache = xuiWidgetGetCacheSurface(pSeparator, xuiWidgetGetStateId(pSeparator));
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(40, 90, 180, 255), "style color");

	iRet = xuiSeparatorSetThickness(pSeparator, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid thickness");
	iRet = xuiSeparatorSetLineStyle(pSeparator, 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid line style");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_separator_test passed\n");
	return 0;
}
