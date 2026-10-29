#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_MSGTIP_TEST_FRACTIONAL_TEXT "ADB \xE8\xBF\x9E\xE6\x8E\xA5\xE5\xA4\xB1\xE8\xB4\xA5"

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_msgtip_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_msgtip_test_close_t {
	int iCount;
	int iExpired;
} xui_msgtip_test_close_t;

typedef struct xui_msgtip_test_destroy_t {
	xui_msgtip* ppTip;
	int iCount;
} xui_msgtip_test_destroy_t;

static int __xuiMsgTipDispatchMove(xui_context pContext, float fX, float fY, uint32_t iButtons)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMsgTipDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMsgTipDispatchClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiMsgTipDispatchMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMsgTipDispatchDown(pContext, fX, fY);
}

static void __xuiMsgTipClose(xui_msgtip pTip, int bExpired, void* pUser)
{
	xui_msgtip_test_close_t* pClose;

	(void)pTip;
	pClose = (xui_msgtip_test_close_t*)pUser;
	if ( pClose != NULL ) {
		pClose->iCount++;
		pClose->iExpired = bExpired ? 1 : 0;
	}
}

static void __xuiMsgTipDestroyClose(xui_msgtip pTip, int bExpired, void* pUser)
{
	xui_msgtip_test_destroy_t* pDestroy = (xui_msgtip_test_destroy_t*)pUser;
	(void)bExpired;
	if ( pDestroy == NULL ) return;
	pDestroy->iCount++;
	xuiMsgTipDestroy(pTip);
	if ( pDestroy->ppTip != NULL ) *pDestroy->ppTip = NULL;
}

static int __xuiMsgTipTestUtf8Count(const char* sText)
{
	const unsigned char* p;
	int iCount;

	if ( sText == NULL ) {
		return 0;
	}
	p = (const unsigned char*)sText;
	iCount = 0;
	while ( *p != 0 ) {
		if ( (*p & 0xc0u) != 0x80u ) {
			iCount++;
		}
		p++;
	}
	return iCount;
}

static int __xuiMsgTipTestPrefixOfFractionalText(const char* sText)
{
	size_t iLen;

	if ( sText == NULL ) {
		return 0;
	}
	iLen = strlen(sText);
	if ( iLen > strlen(XUI_MSGTIP_TEST_FRACTIONAL_TEXT) ) {
		return 0;
	}
	return memcmp(XUI_MSGTIP_TEST_FRACTIONAL_TEXT, sText, iLen) == 0;
}

static int __xuiMsgTipTestFractionalMeasure(xui_proxy pProxy, const xui_text_item_t* pTextItem, xui_vec2_t* pSize)
{
    xui_font pFont = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* sText = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

	static const float arrPrefixW[] = {
		0.0f, 7.498f, 15.498f, 21.997f, 29.993f, 43.993f, 57.632f, 71.270f, 84.908f
	};
	int iCount;

	(void)pProxy;
	(void)pFont;
	if ( pSize == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 18.0f;
	if ( __xuiMsgTipTestPrefixOfFractionalText(sText) ) {
		iCount = __xuiMsgTipTestUtf8Count(sText);
		if ( iCount >= 0 && iCount < (int)(sizeof(arrPrefixW) / sizeof(arrPrefixW[0])) ) {
			pSize->fX = arrPrefixW[iCount];
			return XUI_OK;
		}
	}
	pSize->fX = (sText != NULL) ? ((float)strlen(sText) * 7.0f) : 0.0f;
	return XUI_OK;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_msgtip_test_close_t tClose;
	xui_msgtip_test_destroy_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_msgtip pTip;
	xui_msgtip pInvalidTip;
	xui_surface pCache;
	xui_surface pCustomIcon;
	xui_font pFont;
	xui_msgtip_desc_t tDesc;
	xui_msgtip_desc_t tInvalidDesc;
	xui_msgtip_metrics_t tMetrics;
	xui_msgtip_colors_t tColors;
	xui_rect_t tTip;
	xui_rect_t tIcon;
	xui_rect_t tText;
	int iLayer;
	int iZ;
	int iChangeBefore;
	int iFailed;
	int iRet;

	memset(&tClose, 0, sizeof(tClose));
	memset(&tDestroy, 0, sizeof(tDestroy));
	pContext = NULL;
	pRoot = NULL;
	pTip = NULL;
	pCache = NULL;
	pCustomIcon = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Saved successfully. This intentionally long message wraps to validate the compact tip layout.";
	tDesc.pFont = pFont;
	tDesc.iType = XUI_MSGTIP_ICON_INFO;
	tDesc.fDuration = 0.0f;
	tInvalidDesc = tDesc;
	tInvalidDesc.iType = XUI_MSGTIP_ICON_ERROR + 1;
	pInvalidTip = (xui_msgtip)&tState;
	iRet = xuiMsgTipCreate(pContext, &pInvalidTip, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidTip == NULL, "invalid create clears output");
	tInvalidDesc = tDesc;
	tInvalidDesc.bHasCustomIcon = 1;
	tInvalidDesc.pIconSurface = (xui_surface)&tState;
	tInvalidDesc.tIconSrc = (xui_rect_t){0, 0, 0, 16};
	pInvalidTip = (xui_msgtip)&tState;
	iRet = xuiMsgTipCreate(pContext, &pInvalidTip, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidTip == NULL,
		"invalid custom icon create clears output");
	tInvalidDesc = tDesc;
	tInvalidDesc.tIconSrc.fX = INFINITY;
	pInvalidTip = (xui_msgtip)&tState;
	iRet = xuiMsgTipCreate(pContext, &pInvalidTip, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidTip == NULL,
		"nonfinite icon source is rejected at creation");
	iRet = xuiMsgTipCreate(pContext, &pTip, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTip != NULL, "msgtip create");
	iRet = xuiMsgTipSetClose(pTip, __xuiMsgTipClose, &tClose);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	XUI_TEST_CHECK(!xuiMsgTipIsOpen(pTip), "closed initially");
	XUI_TEST_CHECK(strcmp(xuiMsgTipGetText(pTip), tDesc.sText) == 0, "text getter");
	XUI_TEST_CHECK(xuiMsgTipGetDuration(pTip) == 0.0f, "zero descriptor duration stays sticky");
	iRet = xuiMsgTipGetColors(pTip, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iShadowColor == 0, "default shadow disabled");
	iRet = xuiMsgTipGetMetrics(pTip, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "default metrics get");
	{
		xui_msgtip_metrics_t tInvalidMetrics = tMetrics;
		tInvalidMetrics.fMaxWidth = tInvalidMetrics.fMinWidth - 1.0f;
		iRet = xuiMsgTipSetMetrics(pTip, &tInvalidMetrics);
		XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject inverted metrics");
	}
	iRet = xuiMsgTipSetDuration(pTip, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject negative duration");
	iRet = xuiMsgTipSetDuration(pTip, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite duration");
	iRet = xuiMsgTipSetType(pTip, XUI_MSGTIP_ICON_ERROR + 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject invalid type");
	iRet = xuiMsgTipSetIconSurface(pTip, NULL, (xui_rect_t){0, 0, -1, 1});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject negative icon source");
	iRet = xuiMsgTipSetIconSurface(pTip, NULL, (xui_rect_t){NAN, 0, 1, 1});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nonfinite icon source");
	iChangeBefore = xuiMsgTipGetChangeCount(pTip);
	iRet = xuiMsgTipSetText(pTip, xuiMsgTipGetText(pTip));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipGetChangeCount(pTip) == iChangeBefore, "same text is no-op");

	iRet = xuiMsgTipShow(pTip, XUI_MSGTIP_ICON_INFO, tDesc.sText, 0.1f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipIsOpen(pTip), "show");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tTip = xuiMsgTipGetTipRect(pTip);
	tIcon = xuiMsgTipGetIconRect(pTip);
	tText = xuiMsgTipGetTextRect(pTip);
	XUI_TEST_CHECK(tTip.fW >= 92.0f && tTip.fW <= 360.0f && tTip.fH >= 40.0f, "tip rect");
	XUI_TEST_CHECK(tIcon.fW > 0.0f && tIcon.fH > 0.0f, "icon rect");
	XUI_TEST_CHECK(tText.fW > 40.0f && tText.fH > 12.0f, "text rect");
	XUI_TEST_CHECK(xuiMsgTipGetWrapLineCount(pTip) >= 1, "wrap line count");
	iRet = xuiInputViewport(pContext, 16.0f, 16.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny viewport");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 16, 16});
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root sync");
	tTip = xuiMsgTipGetTipRect(pTip);
	tIcon = xuiMsgTipGetIconRect(pTip);
	tText = xuiMsgTipGetTextRect(pTip);
	XUI_TEST_CHECK(tTip.fX >= 0 && tTip.fY >= 0 &&
		tTip.fX + tTip.fW <= 16 && tTip.fY + tTip.fH <= 16,
		"tiny tip remains inside root");
	XUI_TEST_CHECK(tIcon.fX >= 0 && tIcon.fY >= 0 &&
		tIcon.fX + tIcon.fW <= tTip.fW && tIcon.fY + tIcon.fH <= tTip.fH &&
		tText.fX >= 0 && tText.fY >= 0 &&
		tText.fX + tText.fW <= tTip.fW && tText.fY + tText.fH <= tTip.fH,
		"tiny tip content remains bounded");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny tip render");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore viewport");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 640, 420});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root sync");
	tTip = xuiMsgTipGetTipRect(pTip);
	XUI_TEST_CHECK(tTip.fW >= 92 && tTip.fW <= 360,
		"restored tip uses normal width");
	iRet = xuiWidgetGetLayer(xuiMsgTipGetWidget(pTip), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_TOOLTIP && iZ >= 100, "tooltip layer");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(xuiMsgTipGetWidget(pTip), xuiWidgetGetStateId(xuiMsgTipGetWidget(pTip)));
	XUI_TEST_CHECK(pCache != NULL, "cache exists");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) > 0, "tip rendered");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCache) > 0, "text rendered");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiWidgetInvalidate(xuiMsgTipGetWidget(pTip), XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate tip text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLastTextDrawSize > 0, "tip lines use bounded text lengths");

	iRet = xuiUpdate(pContext, 0.05f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipIsOpen(pTip), "duration stays open");
	iRet = xuiUpdate(pContext, 0.06f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMsgTipIsOpen(pTip), "duration expires");
	XUI_TEST_CHECK(xuiMsgTipGetExpireCount(pTip) == 1 && tClose.iCount == 1 && tClose.iExpired == 1, "expire count");

	iRet = xuiMsgTipShow(pTip, XUI_MSGTIP_ICON_WAR, "Manual close", 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipIsOpen(pTip), "show sticky");
	iRet = xuiUpdate(pContext, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipIsOpen(pTip), "sticky remains");
	tTip = xuiMsgTipGetTipRect(pTip);
	iRet = __xuiMsgTipDispatchClick(pContext, tTip.fX + tTip.fW * 0.5f, tTip.fY + tTip.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMsgTipIsOpen(pTip), "click closes");
	XUI_TEST_CHECK(xuiMsgTipGetCloseCount(pTip) == 1 && tClose.iCount == 2 && tClose.iExpired == 0, "close count");

	iRet = xuiTestSurfaceCreate(&tState, &pCustomIcon, 16, 16, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pCustomIcon != NULL, "custom icon create");
	iRet = xuiMsgTipSetIconSurface(pTip, pCustomIcon, (xui_rect_t){0, 0, 0, 16});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"custom icon requires nonempty source");
	iRet = xuiMsgTipSetIconSurface(pTip, pCustomIcon, (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "custom icon set");
	iRet = xuiMsgTipGetMetrics(pTip, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "metrics get");
	tMetrics.fMinWidth = 160.0f;
	tMetrics.fMaxWidth = 280.0f;
	tMetrics.fOffsetY = -40.0f;
	iRet = xuiMsgTipSetMetrics(pTip, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "metrics set");
	iRet = xuiMsgTipGetColors(pTip, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "colors get");
	tColors.iBackgroundColor = XUI_COLOR_RGBA(28, 92, 70, 225);
	tColors.iIconColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	iRet = xuiMsgTipSetColors(pTip, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "colors set");
	iRet = xuiMsgTipShow(pTip, XUI_MSGTIP_ICON_NONE, "Custom icon", 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipGetType(pTip) == XUI_MSGTIP_ICON_NONE, "custom show");
	tTip = xuiMsgTipGetTipRect(pTip);
	tIcon = xuiMsgTipGetIconRect(pTip);
	XUI_TEST_CHECK(tTip.fW >= 160.0f && tTip.fW <= 280.0f && tIcon.fW > 0.0f, "custom layout");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom icon render");
	pCache = xuiWidgetGetCacheSurface(xuiMsgTipGetWidget(pTip),
		xuiWidgetGetStateId(xuiMsgTipGetWidget(pTip)));
	XUI_TEST_CHECK(pCache != NULL, "custom icon cache");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) == 0,
		"unchanged borrowed msgtip icon reuses cache");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pCustomIcon,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) > 0,
		"borrowed msgtip icon generation rebuilds cache");
	iRet = xuiMsgTipSetDuration(pTip, 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipGetDuration(pTip) > 0.2f, "duration setter");
	iRet = xuiMsgTipUseBuiltinIcon(pTip);
	XUI_TEST_CHECK(iRet == XUI_OK, "builtin icon");
	XUI_TEST_CHECK(xuiMsgTipGetShowCount(pTip) == 3, "show count");
	XUI_TEST_CHECK(xuiMsgTipGetChangeCount(pTip) > 0, "change count");

	tState.tProxy.textMeasure = __xuiMsgTipTestFractionalMeasure;
	iRet = xuiMsgTipShow(pTip, XUI_MSGTIP_ICON_ERROR, XUI_MSGTIP_TEST_FRACTIONAL_TEXT, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgTipIsOpen(pTip), "fractional show");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "fractional layout");
	tTip = xuiMsgTipGetTipRect(pTip);
	tText = xuiMsgTipGetTextRect(pTip);
	XUI_TEST_CHECK(tTip.fW >= 145.0f, "fractional tip width");
	XUI_TEST_CHECK(tText.fW >= 85.0f, "fractional text width");
	XUI_TEST_CHECK(xuiMsgTipGetWrapLineCount(pTip) == 1, "fractional no wrap");

	tDestroy.ppTip = &pTip;
	iRet = xuiMsgTipSetClose(pTip, __xuiMsgTipDestroyClose, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying close callback set");
	iRet = xuiMsgTipClose(pTip);
	XUI_TEST_CHECK(iRet == XUI_OK && pTip == NULL && tDestroy.iCount == 1,
		"close callback may destroy msgtip");

cleanup:
	if ( pCustomIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pCustomIcon);
	}
	if ( pTip != NULL ) {
		xuiMsgTipDestroy(pTip);
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
	printf("xui_msgtip_test passed\n");
	return 0;
}
