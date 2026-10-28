#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_progress_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiProgressRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static xui_style_value_t __xuiProgressStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiProgressStyleInt(int iValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_INT;
	tValue.iInt = iValue;
	return tValue;
}

static xui_style_property_t __xuiProgressStyleProp(const char* sName, xui_style_value_t tValue)
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
	xui_widget pProgress;
	xui_widget pInvalid;
	xui_surface pCache;
	xui_surface pPatchSurface;
	xui_font pFont;
	xui_progress_desc_t tDesc;
	xui_nine_patch_t tPatch;
	xui_nine_patch_t tInvalidPatch;
	xui_accessible_node_t tNode;
	xui_style_property_t arrStyle[3];
	xui_vec2_t tMeasured;
	xui_rect_t tRect;
	xallocationbudget* pBudget;
	xallocationscope tScope;
	uint32_t iAccessibilityRevision;
	float fMin;
	float fMax;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pProgress = NULL;
	pInvalid = NULL;
	pPatchSurface = NULL;
	pFont = NULL;
	pBudget = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 260.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 180.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "%1.0f%%";
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fValue = 25.0f;
	iRet = xuiProgressCreate(pContext, &pProgress, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProgress != NULL, "progress create");
	iRet = xuiWidgetSetRect(pProgress, (xui_rect_t){10.0f, 20.0f, 100.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "progress rect");
	iRet = xuiWidgetAddChild(pRoot, pProgress);
	XUI_TEST_CHECK(iRet == XUI_OK, "progress add");

	iRet = xuiWidgetMeasureContent(pProgress, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 120.0f && tMeasured.fY == 12.0f, "default measure");
	XUI_TEST_CHECK(xuiProgressGetValue(pProgress) == 25.0f, "default value");
	XUI_TEST_CHECK(xuiProgressGetRate(pProgress) == 0.25f, "default rate");
	XUI_TEST_CHECK(strcmp(xuiProgressGetDisplayText(pProgress), "25%") == 0, "dynamic display text");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pProgress, 0, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.iRole == XUI_ACCESSIBLE_ROLE_PROGRESS &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_READONLY) != 0u &&
		tNode.tRange.iSize == sizeof(tNode.tRange) &&
		tNode.tRange.fMin == 0.0 && tNode.tRange.fMax == 100.0 &&
		tNode.tRange.fValue == 25.0 && strcmp(tNode.sValue, "25%") == 0,
		"progress accessibility range");
	iAccessibilityRevision = xuiWidgetGetAccessibilityRevision(pProgress);
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(__xuiProgressRectEq(tRect, 0.0f, 0.0f, 25.0f, 20.0f), "left-to-right fill rect");
	XUI_TEST_CHECK(!xuiProgressGetIndeterminate(pProgress) &&
		xuiProgressGetMarqueeInterval(pProgress) == 1.5f, "marquee defaults");
	XUI_TEST_CHECK(xuiProgressSetMarqueeInterval(pProgress, 0.0f) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiProgressSetMarqueeInterval(pProgress, NAN) == XUI_ERROR_INVALID_ARGUMENT,
		"invalid marquee interval rejected");
	iRet = xuiProgressSetMarqueeInterval(pProgress, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressGetMarqueeInterval(pProgress) == 1.0f,
		"set marquee interval");
	iRet = xuiProgressSetIndeterminate(pProgress, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressGetIndeterminate(pProgress),
		"enable indeterminate progress");
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(tRect.fW > 0 && tRect.fW < 100 && tRect.fH == 20,
		"indeterminate segment rect");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pProgress, 0, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.tRange.iSize == 0,
		"indeterminate accessibility omits numeric range");
	iRet = xuiUpdate(pContext, 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		!__xuiProgressRectEq(xuiProgressGetFillRect(pProgress), tRect.fX, tRect.fY, tRect.fW, tRect.fH),
		"marquee advances during update");
	iRet = xuiProgressSetIndeterminate(pProgress, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiProgressGetIndeterminate(pProgress) &&
		__xuiProgressRectEq(xuiProgressGetFillRect(pProgress), 0.0f, 0.0f, 25.0f, 20.0f),
		"restore determinate progress");

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pProgress, xuiWidgetGetStateId(pProgress));
	XUI_TEST_CHECK(pCache != NULL, "cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) == 2, "track and fill draw count");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCache) == 2,
		"partial fill draws clipped fill text");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pCache) == XUI_COLOR_WHITE &&
		tState.iDrawClipGetCount > 0 && tState.iDrawClipSetCount > 0 &&
		tState.iDrawClipClearCount > 0 && !tState.bLastDrawClipActive,
		"partial fill text uses and restores draw clip");
	XUI_TEST_CHECK(__xuiProgressRectEq(xuiTestSurfaceGetLastTextRect(pCache), 0.0f, 0.0f, 100.0f, 20.0f), "base text rect");

	iRet = xuiProgressSetRange(pProgress, 100.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "swap range");
	iRet = xuiProgressGetRange(pProgress, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 100.0f, "range normalized");
	iRet = xuiProgressSetValue(pProgress, 130.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressGetValue(pProgress) == 100.0f && xuiProgressGetRate(pProgress) == 1.0f, "value clamp high");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render full text");
	pCache = xuiWidgetGetCacheSurface(pProgress, xuiWidgetGetStateId(pProgress));
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCache) == 2, "full fill text count");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pCache) == XUI_COLOR_WHITE, "full fill text color");
	iRet = xuiProgressSetValue(pProgress, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiProgressGetDisplayText(pProgress), "40%") == 0, "dynamic text update");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiWidgetGetAccessibilityRevision(pProgress) > iAccessibilityRevision,
		"value updates accessibility");
	iRet = xuiProgressSetRange(pProgress, -FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide finite range");
	iRet = xuiProgressSetValue(pProgress, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(xuiProgressGetRate(pProgress)) && xuiProgressGetRate(pProgress) == 0.5f, "wide range rate finite");
	iRet = xuiProgressSetRange(pProgress, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "equal maximum range normalized");
	iRet = xuiProgressGetRange(pProgress, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(fMin) && isfinite(fMax) && fMin < fMax, "equal maximum range remains finite");
	iRet = xuiProgressSetRange(pProgress, 0.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore range");
	iRet = xuiProgressSetValue(pProgress, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore value");

	iRet = xuiProgressSetFillDirection(pProgress, XUI_PROGRESS_RIGHT_TO_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "right-to-left");
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(__xuiProgressRectEq(tRect, 60.0f, 0.0f, 40.0f, 20.0f), "right-to-left fill rect");
	iRet = xuiProgressSetFillDirection(pProgress, XUI_PROGRESS_BOTTOM_TO_TOP);
	XUI_TEST_CHECK(iRet == XUI_OK, "bottom-to-top");
	iRet = xuiWidgetSetRect(pProgress, (xui_rect_t){10.0f, 20.0f, 20.0f, 100.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical rect");
	iRet = xuiWidgetMeasureContent(pProgress, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 12.0f && tMeasured.fY == 120.0f, "vertical measure");
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(__xuiProgressRectEq(tRect, 0.0f, 60.0f, 20.0f, 40.0f), "bottom-to-top fill rect");
	iRet = xuiProgressSetFillDirection(pProgress, XUI_PROGRESS_TOP_TO_BOTTOM);
	XUI_TEST_CHECK(iRet == XUI_OK, "top-to-bottom");
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(__xuiProgressRectEq(tRect, 0.0f, 0.0f, 20.0f, 40.0f), "top-to-bottom fill rect");

	iRet = xuiProgressSetTextTemplate(pProgress, "{{{value}}}/{max} {percent}%");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiProgressGetDisplayText(pProgress), "{40}/100 40%") == 0,
		"named template tokens and escaping");
	iRet = xuiProgressSetTextTemplate(pProgress, "Loading");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiProgressGetDisplayText(pProgress), "Loading") == 0, "static text");
	iRet = xuiProgressSetTextTemplate(pProgress, "Progress %s");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiProgressGetDisplayText(pProgress), "Progress %s") == 0, "unsafe format literal");
	memset(&tScope, 0, sizeof(tScope));
	XUI_TEST_CHECK(xrtAllocationBudgetCreate(1u, &pBudget), "create allocation budget");
	XUI_TEST_CHECK(xrtAllocationBudgetEnter(pBudget, &tScope), "enter allocation budget");
	iRet = xuiProgressSetText(pProgress, NULL, "{value} / {max}");
	XUI_TEST_CHECK(xrtAllocationBudgetLeave(&tScope), "leave allocation budget");
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && xuiProgressGetFont(pProgress) == pFont &&
		strcmp(xuiProgressGetTextTemplate(pProgress), "Progress %s") == 0 &&
		strcmp(xuiProgressGetDisplayText(pProgress), "Progress %s") == 0,
		"set text allocation failure is atomic");
	iRet = xuiProgressSetTextTemplate(pProgress, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressGetTextTemplate(pProgress) == NULL && xuiProgressGetDisplayText(pProgress) == NULL, "clear text");

	arrStyle[0] = __xuiProgressStyleProp("progress.fill_direction", __xuiProgressStyleInt(XUI_PROGRESS_RIGHT_TO_LEFT));
	arrStyle[1] = __xuiProgressStyleProp("progress.track_color", __xuiProgressStyleColor(XUI_COLOR_RGBA(20, 40, 60, 255)));
	arrStyle[2] = __xuiProgressStyleProp("progress.fill_color", __xuiProgressStyleColor(XUI_COLOR_RGBA(70, 100, 140, 255)));
	iRet = xuiWidgetSetInlineStyle(pProgress, arrStyle, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	iRet = xuiWidgetSetRect(pProgress, (xui_rect_t){10.0f, 20.0f, 100.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "style rect");
	tRect = xuiProgressGetFillRect(pProgress);
	XUI_TEST_CHECK(__xuiProgressRectEq(tRect, 60.0f, 0.0f, 40.0f, 20.0f), "style direction fill rect");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render style");
	pCache = xuiWidgetGetCacheSurface(pProgress, xuiWidgetGetStateId(pProgress));
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(70, 100, 140, 255), "style fill color");

	iRet = xuiTestSurfaceCreate(&tState, &pPatchSurface, 16, 8, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pPatchSurface != NULL, "patch surface");
	memset(&tPatch, 0, sizeof(tPatch));
	tPatch.iSize = sizeof(tPatch);
	tPatch.pSurface = pPatchSurface;
	tPatch.iColor = XUI_COLOR_WHITE;
	tPatch.iMode = XUI_NINE_PATCH_STRETCH;
	iRet = xuiProgressSetTrackPatch(pProgress, &tPatch);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressHasTrackPatch(pProgress), "track patch");
	iRet = xuiProgressSetFillPatch(pProgress, &tPatch);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressHasFillPatch(pProgress), "fill patch");
	iRet = xuiProgressSetFillPatchMode(pProgress, XUI_PROGRESS_FILL_REVEAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiProgressGetFillPatchMode(pProgress) == XUI_PROGRESS_FILL_REVEAL, "reveal mode");
	iRet = xuiProgressSetValue(pProgress, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "patch value");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render patch");
	pCache = xuiWidgetGetCacheSurface(pProgress, xuiWidgetGetStateId(pProgress));
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) >= 2, "patch draw count");
	XUI_TEST_CHECK(__xuiProgressRectEq(xuiTestSurfaceGetLastSrc(pCache), 8.0f, 0.0f, 8.0f, 8.0f), "reveal patch src");
	XUI_TEST_CHECK(__xuiProgressRectEq(xuiTestSurfaceGetLastDst(pCache), 50.0f, 0.0f, 50.0f, 20.0f), "reveal patch dst");
	{
		int iDrawCount = xuiTestSurfaceGetDrawCount(pCache);
		iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pPatchSurface,
			(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "patch surface update");
		iRet = xuiUpdate(pContext, 0.016f);
		XUI_TEST_CHECK(iRet == XUI_OK, "update after patch surface update");
		iRet = xuiRenderPrepare(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK, "render after patch surface update");
		XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) > iDrawCount, "patch generation invalidates progress cache");
	}
	tPatch.tSlice = (xui_thickness_t){32, 16, 32, 16};
	iRet = xuiProgressSetTrackPatch(pProgress, &tPatch);
	XUI_TEST_CHECK(iRet == XUI_OK, "oversized patch slices accepted");
	iRet = xuiProgressClearFillPatch(pProgress);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear fill patch for slice test");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render oversized patch slices");
	tRect = xuiTestSurfaceGetLastSrc(pCache);
	XUI_TEST_CHECK(tRect.fX >= 0 && tRect.fY >= 0 && tRect.fW >= 0 && tRect.fH >= 0 &&
		tRect.fX + tRect.fW <= 16 && tRect.fY + tRect.fH <= 8,
		"oversized slices clamp source rectangles");
	tPatch.tSlice = (xui_thickness_t){0, 0, 0, 0};
	iRet = xuiProgressSetFillPatch(pProgress, &tPatch);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore fill patch");

	iRet = xuiProgressSetFillDirection(pProgress, 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid direction");
	iRet = xuiProgressSetFillPatchMode(pProgress, 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid patch mode");
	iRet = xuiProgressSetRange(pProgress, NAN, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "nan range rejected");
	iRet = xuiProgressSetValue(pProgress, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiProgressGetValue(pProgress) == 50.0f, "infinite value rejected");
	tInvalidPatch = tPatch;
	tInvalidPatch.tSrc.fW = -1;
	iRet = xuiProgressSetTrackPatch(pProgress, &tInvalidPatch);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "negative patch source rejected");
	XUI_TEST_CHECK(xuiProgressHasTrackPatch(pProgress), "invalid track patch preserves prior state");
	tInvalidPatch = tPatch;
	tInvalidPatch.tSlice.fLeft = -1;
	iRet = xuiProgressSetFillPatch(pProgress, &tInvalidPatch);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "negative patch slice rejected");
	XUI_TEST_CHECK(xuiProgressHasFillPatch(pProgress), "invalid fill patch preserves prior state");
	tDesc.fMin = NAN;
	pInvalid = pRoot;
	iRet = xuiProgressCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "nan descriptor rejected");

cleanup:
	if ( pBudget != NULL ) {
		xrtAllocationBudgetFree(pBudget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pPatchSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pPatchSurface);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_progress_test passed\n");
	return 0;
}
