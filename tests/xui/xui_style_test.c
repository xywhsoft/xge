#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_style_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static xui_style_value_t __xuiTestColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiTestFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_value_t __xuiTestInt(int iValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_INT;
	tValue.iInt = iValue;
	return tValue;
}

static xui_style_value_t __xuiTestString(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_STRING;
	tValue.sText = sText;
	return tValue;
}

static xui_style_value_t __xuiTestToken(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_TOKEN;
	tValue.sText = sText;
	return tValue;
}

static xui_style_property_t __xuiTestProp(const char* sName, uint32_t iDirtyFlags, xui_style_value_t tValue)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.iDirtyFlags = iDirtyFlags;
	tProperty.tValue = tValue;
	return tProperty;
}

static xui_style_desc_t __xuiTestStyle(const char* sName, const char* sParent, const xui_style_property_t* pProperties, int iPropertyCount)
{
	xui_style_desc_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.sName = sName;
	tStyle.sParent = sParent;
	tStyle.pProperties = pProperties;
	tStyle.iPropertyCount = iPropertyCount;
	return tStyle;
}

int main(void)
{
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pChild;
	xui_style_property_info_t tInfo;
	xui_style_property_t arrTypeProps[2];
	xui_style_property_t arrBaseProps[1];
	xui_style_property_t arrHeadlineProps[1];
	xui_style_property_t arrClassProps[1];
	xui_style_property_t arrAccentProps[1];
	xui_style_property_t arrInlineProps[2];
	xui_style_property_t arrStateProps[1];
	xui_style_property_t arrDefaultProps[1];
	xui_style_property_t tProp;
	xui_style_value_t tValue;
	xui_style_desc_t tStyle;
	xui_state_style_desc_t tStateStyle;
	xui_theme_t tTheme;
	xui_chrome_style_t tChrome;
	xui_font pFakeFont;
	uint32_t iAccentProperty;
	uint32_t iGenerationBefore;
	uint32_t iTokenGenerationBefore;
	uint32_t iHashBefore;
	uint32_t iHashAfter;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pChild = NULL;
	iFailed = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetViewportSize(pContext, 200.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport set failed");

	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = "theme.accent";
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_STYLE_DIRTY_DEFAULT;
	tInfo.iFlags = XUI_STYLE_PROPERTY_INHERITED;
	tInfo.tDefaultValue = __xuiTestColor(XUI_COLOR_RGBA(12, 34, 56, 255));
	iRet = xuiStyleRegisterProperty(pContext, &tInfo, &iAccentProperty);
	XUI_TEST_CHECK((iRet == XUI_OK) && (iAccentProperty != 0), "style property register failed");
	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "theme.accent") == iAccentProperty, "style property lookup failed");
	memset(&tInfo, 0, sizeof(tInfo));
	iRet = xuiStyleGetPropertyInfo(pContext, iAccentProperty, &tInfo);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tInfo.iValueType == XUI_STYLE_VALUE_COLOR) &&
	               ((tInfo.iFlags & XUI_STYLE_PROPERTY_INHERITED) != 0), "style property info failed");
	iTokenGenerationBefore = xuiStyleGetTokenGeneration(pContext);
	tValue = __xuiTestColor(XUI_COLOR_RGBA(8, 120, 220, 255));
	iRet = xuiStyleSetToken(pContext, "accent.primary", &tValue);
	XUI_TEST_CHECK(iRet == XUI_OK, "style token set failed");
	XUI_TEST_CHECK(xuiStyleGetTokenGeneration(pContext) != iTokenGenerationBefore, "style token generation failed");
	memset(&tValue, 0, sizeof(tValue));
	iRet = xuiStyleGetToken(pContext, "accent.primary", &tValue);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tValue.iType == XUI_STYLE_VALUE_COLOR) &&
	               (tValue.iColor == XUI_COLOR_RGBA(8, 120, 220, 255)), "style token getter failed");

	arrTypeProps[0] = __xuiTestProp("type.priority", XUI_WIDGET_DIRTY_STYLE, __xuiTestInt(10));
	arrTypeProps[1] = __xuiTestProp("type.source", XUI_WIDGET_DIRTY_STYLE, __xuiTestString("base-widget"));
	tStyle = __xuiTestStyle(NULL, NULL, arrTypeProps, 2);
	iRet = xuiStyleSetType(pContext, xuiWidgetGetBaseType(), &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "type style set failed");

	arrBaseProps[0] = __xuiTestProp("text.color", XUI_STYLE_DIRTY_DEFAULT, __xuiTestColor(XUI_COLOR_RGBA(255, 0, 0, 255)));
	tStyle = __xuiTestStyle("base.text", NULL, arrBaseProps, 1);
	iRet = xuiStyleSetNamed(pContext, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "base named style set failed");

	arrHeadlineProps[0] = __xuiTestProp("text.size", XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, __xuiTestFloat(18.0f));
	tStyle = __xuiTestStyle("headline", "base.text", arrHeadlineProps, 1);
	iRet = xuiStyleSetNamed(pContext, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "headline named style set failed");

	arrClassProps[0] = __xuiTestProp("text.color", XUI_STYLE_DIRTY_DEFAULT, __xuiTestColor(XUI_COLOR_RGBA(0, 180, 80, 255)));
	tStyle = __xuiTestStyle(NULL, NULL, arrClassProps, 1);
	iRet = xuiStyleSetClass(pContext, "primary", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "class style set failed");

	arrAccentProps[0] = __xuiTestProp("theme.accent", XUI_STYLE_DIRTY_DEFAULT, __xuiTestToken("accent.primary"));
	tStyle = __xuiTestStyle(NULL, NULL, arrAccentProps, 1);
	iRet = xuiStyleSetClass(pContext, "accented", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "accent class style set failed");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChild != NULL), "child create failed");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set failed");
	iRet = xuiWidgetAddChild(pRoot, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "child add failed");
	iRet = xuiWidgetAddStyleClass(pRoot, "accented");
	XUI_TEST_CHECK(iRet == XUI_OK, "root inherited class add failed");

	iRet = xuiWidgetSetStyleName(pChild, "headline");
	XUI_TEST_CHECK(iRet == XUI_OK, "style name set failed");
	XUI_TEST_CHECK(strcmp(xuiWidgetGetStyleName(pChild), "headline") == 0, "style name getter failed");
	iRet = xuiWidgetAddStyleClass(pChild, "primary");
	XUI_TEST_CHECK(iRet == XUI_OK, "style class add failed");
	XUI_TEST_CHECK((xuiWidgetGetStyleClassCount(pChild) == 1) &&
	               (strcmp(xuiWidgetGetStyleClass(pChild, 0), "primary") == 0) &&
	               xuiWidgetHasStyleClass(pChild, "primary"), "style class getter failed");

	arrInlineProps[0] = __xuiTestProp("text.color", XUI_STYLE_DIRTY_DEFAULT, __xuiTestColor(XUI_COLOR_RGBA(40, 90, 255, 255)));
	arrInlineProps[1] = __xuiTestProp("inline.note", XUI_WIDGET_DIRTY_STYLE, __xuiTestString("inline"));
	iRet = xuiWidgetSetInlineStyle(pChild, arrInlineProps, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style set failed");
	iRet = xuiWidgetGetInlineStyleProperty(pChild, "inline.note", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	               (strcmp(tProp.tValue.sText, "inline") == 0), "inline property getter failed");

	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(40, 90, 255, 255)), "inline should override class color");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.size", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iType == XUI_STYLE_VALUE_FLOAT) && (tProp.tValue.fFloat == 18.0f), "named parent style failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "type.source", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	               (strcmp(tProp.tValue.sText, "base-widget") == 0), "type style failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "theme.accent", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.iPropertyId == iAccentProperty) &&
	               (tProp.tValue.iType == XUI_STYLE_VALUE_COLOR) &&
	               (tProp.tValue.iColor == XUI_COLOR_RGBA(8, 120, 220, 255)), "inherited token style failed");
	XUI_TEST_CHECK(xuiWidgetGetResolvedStylePropertyCount(pChild) == 6, "resolved property count failed");

	iHashBefore = xuiWidgetGetStyleHash(pChild);
	iRet = xuiWidgetSetInlineStyle(pChild, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline clear failed");
	iHashAfter = xuiWidgetGetStyleHash(pChild);
	XUI_TEST_CHECK(iHashAfter != iHashBefore, "style hash should change after inline clear");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(0, 180, 80, 255)), "class should override named color");
	XUI_TEST_CHECK(xuiWidgetGetResolvedStyleProperty(pChild, "inline.note", &tProp) == XUI_ERROR_FILE_NOT_FOUND, "cleared inline property should be missing");

	xuiWidgetClearDirty(pChild, 0);
	iGenerationBefore = xuiStyleGetGeneration(pContext);
	iHashBefore = xuiWidgetGetStyleHash(pChild);
	iRet = xuiStyleBeginUpdate(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style update begin failed");
	arrClassProps[0] = __xuiTestProp("text.color", XUI_STYLE_DIRTY_DEFAULT, __xuiTestColor(XUI_COLOR_RGBA(240, 220, 40, 255)));
	tStyle = __xuiTestStyle(NULL, NULL, arrClassProps, 1);
	iRet = xuiStyleSetClass(pContext, "primary", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "batched class set failed");
	XUI_TEST_CHECK(xuiStyleGetGeneration(pContext) == iGenerationBefore, "style generation should not change inside batch");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(0, 180, 80, 255)), "batch should defer resolved cache refresh");
	iRet = xuiStyleEndUpdate(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style update end failed");
	XUI_TEST_CHECK(xuiStyleGetGeneration(pContext) != iGenerationBefore, "style generation should change after batch");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(240, 220, 40, 255)), "batch refresh failed");
	XUI_TEST_CHECK(xuiWidgetGetStyleHash(pChild) != iHashBefore, "batch refresh hash failed");
	XUI_TEST_CHECK((xuiWidgetGetDirtyFlags(pChild) & XUI_STYLE_DIRTY_DEFAULT) == XUI_STYLE_DIRTY_DEFAULT, "style refresh dirty flags failed");

	arrStateProps[0] = __xuiTestProp("text.color", XUI_STYLE_DIRTY_DEFAULT, __xuiTestColor(XUI_COLOR_RGBA(20, 120, 250, 255)));
	memset(&tStateStyle, 0, sizeof(tStateStyle));
	tStateStyle.iSize = sizeof(tStateStyle);
	tStateStyle.sClass = "primary";
	tStateStyle.iStateMask = XUI_WIDGET_STATE_DISABLED;
	tStateStyle.pProperties = arrStateProps;
	tStateStyle.iPropertyCount = 1;
	iRet = xuiStyleSetStateClass(pContext, &tStateStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "state class style set failed");
	iRet = xuiWidgetSetEnabled(pChild, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "child disable failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(20, 120, 250, 255)), "disabled state class should override normal class");
	iRet = xuiStyleRemoveStateClass(pContext, "primary", XUI_WIDGET_STATE_DISABLED);
	XUI_TEST_CHECK(iRet == XUI_OK, "state class remove failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(240, 220, 40, 255)), "state class remove should restore normal class");
	iRet = xuiWidgetSetEnabled(pChild, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "child reenable failed");

	iRet = xuiWidgetRemoveStyleClass(pChild, "primary");
	XUI_TEST_CHECK(iRet == XUI_OK, "class remove failed");
	XUI_TEST_CHECK(!xuiWidgetHasStyleClass(pChild, "primary"), "class remove getter failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "text.color", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iColor == XUI_COLOR_RGBA(255, 0, 0, 255)), "named color should remain after class remove");
	xuiWidgetClearStyleClasses(pChild);
	XUI_TEST_CHECK(xuiWidgetGetStyleClassCount(pChild) == 0, "class clear failed");

	arrDefaultProps[0] = __xuiTestProp("ua.padding", XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_RENDER, __xuiTestFloat(6.0f));
	iRet = xuiStyleSetDefault(pContext, arrDefaultProps, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "default style set failed");
	iRet = xuiWidgetGetResolvedStyleProperty(pChild, "ua.padding", &tProp);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tProp.tValue.iType == XUI_STYLE_VALUE_FLOAT) && (tProp.tValue.fFloat == 6.0f), "default style property failed");
	iRet = xuiStyleClearDefault(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "default style clear failed");
	XUI_TEST_CHECK(xuiWidgetGetResolvedStyleProperty(pChild, "ua.padding", &tProp) == XUI_ERROR_FILE_NOT_FOUND, "default style clear property failed");

	xuiThemeDefault(&tTheme);
	tTheme.iAccentColor = XUI_COLOR_RGBA(1, 2, 3, 255);
	iRet = xuiSetTheme(pContext, &tTheme);
	XUI_TEST_CHECK(iRet == XUI_OK, "theme set failed");
	memset(&tTheme, 0, sizeof(tTheme));
	iRet = xuiGetTheme(pContext, &tTheme);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tTheme.iAccentColor == XUI_COLOR_RGBA(1, 2, 3, 255)), "theme get failed");
	iRet = xuiStyleGetToken(pContext, "theme.accent", &tValue);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tValue.iType == XUI_STYLE_VALUE_COLOR) &&
	               (tValue.iColor == XUI_COLOR_RGBA(1, 2, 3, 255)), "theme token failed");

	memset(&tChrome, 0, sizeof(tChrome));
	tChrome.iSize = sizeof(tChrome);
	tChrome.iPopupColor = XUI_COLOR_RGBA(4, 5, 6, 255);
	tChrome.iTooltipTextColor = XUI_COLOR_RGBA(7, 8, 9, 255);
	iRet = xuiSetChromeStyle(pContext, &tChrome);
	XUI_TEST_CHECK(iRet == XUI_OK, "chrome style set failed");
	memset(&tChrome, 0, sizeof(tChrome));
	iRet = xuiGetChromeStyle(pContext, &tChrome);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tChrome.iPopupColor == XUI_COLOR_RGBA(4, 5, 6, 255)), "chrome style get failed");
	iRet = xuiStyleGetToken(pContext, "chrome.popup", &tValue);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tValue.iType == XUI_STYLE_VALUE_COLOR) &&
	               (tValue.iColor == XUI_COLOR_RGBA(4, 5, 6, 255)), "chrome token failed");

	pFakeFont = (xui_font)(uintptr_t)0x1234;
	iRet = xuiRegisterFont(pContext, "ui", pFakeFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "font register failed");
	XUI_TEST_CHECK(xuiFindFont(pContext, "ui") == pFakeFont, "font find failed");
	iRet = xuiSetDefaultFont(pContext, pFakeFont);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiGetDefaultFont(pContext) == pFakeFont), "default font failed");
	xuiClearFonts(pContext);
	XUI_TEST_CHECK((xuiFindFont(pContext, "ui") == NULL) && (xuiGetDefaultFont(pContext) == NULL), "font clear failed");

cleanup:
	if ( pChild != NULL ) {
		xuiWidgetDestroy(pChild);
	}
	if ( pRoot != NULL ) {
		xuiWidgetDestroy(pRoot);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_style_test passed\n");
	return 0;
}
