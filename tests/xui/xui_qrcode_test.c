#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_qrcode_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static xui_draw_rect_fill_proc g_xuiQrCodeDrawRectFill;
static uint32_t g_xuiQrCodeTraceColor;
static xui_rect_t g_xuiQrCodeTraceBounds;
static xui_rect_t g_xuiQrCodeTraceFirstRect;
static int g_xuiQrCodeTraceCount;
static int g_xuiQrCodeTraceIntegral;

static void __xuiQrCodeTraceReset(uint32_t iColor)
{
	g_xuiQrCodeTraceColor = iColor;
	memset(&g_xuiQrCodeTraceBounds, 0, sizeof(g_xuiQrCodeTraceBounds));
	memset(&g_xuiQrCodeTraceFirstRect, 0, sizeof(g_xuiQrCodeTraceFirstRect));
	g_xuiQrCodeTraceCount = 0;
	g_xuiQrCodeTraceIntegral = 1;
}

static int __xuiQrCodeNearInteger(float fValue)
{
	return fabsf(fValue - floorf(fValue + 0.5f)) < 0.001f;
}

static uint32_t __xuiQrCodeMatrixHash(xui_widget pQrCode)
{
	uint32_t iHash;
	int iSize;
	int iX;
	int iY;

	iHash = 2166136261u;
	iSize = xuiQrCodeGetModuleCount(pQrCode);
	for ( iY = 0; iY < iSize; iY++ ) {
		for ( iX = 0; iX < iSize; iX++ ) {
			iHash ^= (uint32_t)xuiQrCodeGetModule(pQrCode, iX, iY);
			iHash *= 16777619u;
		}
	}
	return iHash;
}

static int __xuiQrCodeTraceRectFill(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tRect, uint32_t iColor)
{
	if ( (g_xuiQrCodeTraceColor != 0u) && (iColor == g_xuiQrCodeTraceColor) ) {
		float fRight = tRect.fX + tRect.fW;
		float fBottom = tRect.fY + tRect.fH;
		if ( g_xuiQrCodeTraceCount == 0 ) {
			g_xuiQrCodeTraceBounds = tRect;
			g_xuiQrCodeTraceFirstRect = tRect;
		} else {
			float fBoundsRight = g_xuiQrCodeTraceBounds.fX + g_xuiQrCodeTraceBounds.fW;
			float fBoundsBottom = g_xuiQrCodeTraceBounds.fY + g_xuiQrCodeTraceBounds.fH;
			if ( tRect.fX < g_xuiQrCodeTraceBounds.fX ) g_xuiQrCodeTraceBounds.fX = tRect.fX;
			if ( tRect.fY < g_xuiQrCodeTraceBounds.fY ) g_xuiQrCodeTraceBounds.fY = tRect.fY;
			if ( fRight > fBoundsRight ) fBoundsRight = fRight;
			if ( fBottom > fBoundsBottom ) fBoundsBottom = fBottom;
			g_xuiQrCodeTraceBounds.fW = fBoundsRight - g_xuiQrCodeTraceBounds.fX;
			g_xuiQrCodeTraceBounds.fH = fBoundsBottom - g_xuiQrCodeTraceBounds.fY;
		}
		if ( !__xuiQrCodeNearInteger(tRect.fX) || !__xuiQrCodeNearInteger(tRect.fY) ||
		     !__xuiQrCodeNearInteger(tRect.fW) || !__xuiQrCodeNearInteger(tRect.fH) ) {
			g_xuiQrCodeTraceIntegral = 0;
		}
		g_xuiQrCodeTraceCount++;
	}
	return g_xuiQrCodeDrawRectFill(pProxy, pDraw, tRect, iColor);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pQrCode;
	xui_widget pInvalid;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_surface pCache;
	xui_qrcode_desc_t tDesc;
	xui_qrcode_desc_t tInvalidDesc;
	xui_accessible_node_t tNode;
	xui_rect_i_t tFullRect;
	xui_rect_t tContentRect;
	xallocationbudget* pBudget;
	xallocationscope tScope;
	char sLongValue[512];
	const char* sPreviousValue;
	uint32_t iAccessibilityRevision;
	uint32_t iDirtyFlags;
	int iChangeCount;
	int iDarkModules;
	int iX;
	int iY;
	int iVersion;
	int iModuleCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pQrCode = NULL;
	pInvalid = NULL;
	pTarget = NULL;
	pIcon = NULL;
	pCache = NULL;
	pBudget = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	g_xuiQrCodeDrawRectFill = tState.tProxy.drawRectFill;
	tState.tProxy.drawRectFill = __xuiQrCodeTraceRectFill;
	__xuiQrCodeTraceReset(0u);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 260.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 220.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fPadding = NAN;
	pInvalid = pRoot;
	iRet = xuiQrCodeCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject NaN descriptor");
	tInvalidDesc.fPadding = 0.0f;
	tInvalidDesc.tIconSrc.fX = (int)XUI_LAYOUT_UNBOUNDED + 1024;
	pInvalid = pRoot;
	iRet = xuiQrCodeCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject out-of-range icon source");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sValue = "https://example.com/xui2";
	tDesc.iForegroundColor = XUI_COLOR_RGBA(0, 0, 0, 255);
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.fPadding = 10.0f;
	iRet = xuiQrCodeCreate(pContext, &pQrCode, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pQrCode != NULL, "qrcode create");
	xuiWidgetSetRect(pQrCode, (xui_rect_t){20.0f, 20.0f, 180.0f, 180.0f});
	iRet = xuiWidgetAddChild(pRoot, pQrCode);
	XUI_TEST_CHECK(iRet == XUI_OK, "add qrcode");

	iVersion = xuiQrCodeGetVersion(pQrCode);
	iModuleCount = xuiQrCodeGetModuleCount(pQrCode);
	XUI_TEST_CHECK(iVersion >= 1 && iVersion <= XUI_QRCODE_MAX_VERSION, "version range");
	XUI_TEST_CHECK(iModuleCount == 21 + (iVersion - 1) * 4, "module count");
	XUI_TEST_CHECK(xuiQrCodeGetModule(pQrCode, 0, 0) == 1, "finder outer dark");
	XUI_TEST_CHECK(xuiQrCodeGetModule(pQrCode, 1, 1) == 0, "finder ring light");
	XUI_TEST_CHECK(xuiQrCodeGetModule(pQrCode, 3, 3) == 1, "finder center dark");
	XUI_TEST_CHECK(xuiQrCodeGetModule(pQrCode, -1, 0) == -1, "invalid module");

	iRet = xuiQrCodeSetColors(pQrCode, XUI_COLOR_RGBA(18, 112, 214, 255), XUI_COLOR_RGBA(244, 250, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	XUI_TEST_CHECK(xuiQrCodeGetForegroundColor(pQrCode) == XUI_COLOR_RGBA(18, 112, 214, 255), "foreground");
	XUI_TEST_CHECK(xuiQrCodeGetBackgroundColor(pQrCode) == XUI_COLOR_RGBA(244, 250, 255, 255), "background");
	iRet = xuiQrCodeSetPadding(pQrCode, 6.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetPadding(pQrCode) == 6.0f, "padding");

	iRet = xuiTestSurfaceCreate(&tState, &pIcon, 16, 16, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon create");
	iRet = xuiQrCodeSetIcon(pQrCode, pIcon, (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f}, 28.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set icon");
	XUI_TEST_CHECK(xuiQrCodeGetIcon(pQrCode) == pIcon && xuiQrCodeGetIconSize(pQrCode) == 28.0f, "icon getter");

	iRet = xuiQrCodeSetValue(pQrCode, "hello");
	XUI_TEST_CHECK(iRet == XUI_OK, "set short value");
	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 1, "short value version");
	XUI_TEST_CHECK(strcmp(xuiQrCodeGetValue(pQrCode), "hello") == 0, "value getter");
	XUI_TEST_CHECK(xuiQrCodeGetChangeCount(pQrCode) >= 2, "change count");
	iRet = xuiQrCodeSetValue(pQrCode, "12345678901234");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 1,
		"version 1-M accepts fourteen byte payload");
	iChangeCount = xuiQrCodeGetChangeCount(pQrCode);
	iRet = xuiQrCodeSetValue(pQrCode, "123456789012345");
	XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL &&
		xuiQrCodeGetChangeCount(pQrCode) == iChangeCount &&
		strcmp(xuiQrCodeGetValue(pQrCode), "12345678901234") == 0,
		"version 1-M rejects fifteen byte payload without mutation");
	iRet = xuiQrCodeSetValue(pQrCode, "\xe4\xb8\xad\xe6\x96\x87\xf0\x9f\x99\x82");
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strcmp(xuiQrCodeGetValue(pQrCode), "\xe4\xb8\xad\xe6\x96\x87\xf0\x9f\x99\x82") == 0,
		"UTF-8 payload is preserved as byte-mode data");
	iRet = xuiQrCodeSetValue(pQrCode, "hello");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore value after payload boundaries");
	/* Byte-mode ECC-M matrices independently matched against qrcodegen. */
	iRet = xuiQrCodeSetVersionRange(pQrCode, 6, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 6 &&
		__xuiQrCodeMatrixHash(pQrCode) == 0x31864ee6u,
		"version 6 mask penalty matrix");
	iRet = xuiQrCodeSetVersionRange(pQrCode, 7, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 7 &&
		xuiQrCodeGetModule(pQrCode, 6, 22) == 1 &&
		xuiQrCodeGetModule(pQrCode, 7, 22) == 0 &&
		xuiQrCodeGetModule(pQrCode, 8, 22) == 1 &&
		xuiQrCodeGetModule(pQrCode, 22, 6) == 1 &&
		xuiQrCodeGetModule(pQrCode, 22, 7) == 0 &&
		xuiQrCodeGetModule(pQrCode, 22, 8) == 1 &&
		__xuiQrCodeMatrixHash(pQrCode) == 0x1b1f9643u,
		"version 7 intermediate alignment patterns and matrix");
	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 1,
		"restore version after conformance matrices");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pQrCode, 0, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.iRole == XUI_ACCESSIBLE_ROLE_IMAGE &&
		strcmp(tNode.sName, "hello") == 0 && strcmp(tNode.sValue, "hello") == 0,
		"qrcode accessibility");
	iAccessibilityRevision = xuiWidgetGetAccessibilityRevision(pQrCode);

	XUI_TEST_CHECK(xuiQrCodeSetPadding(pQrCode, NAN) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN padding");
	XUI_TEST_CHECK(xuiQrCodeSetPadding(pQrCode, INFINITY) == XUI_ERROR_INVALID_ARGUMENT,
		"reject infinite padding");
	XUI_TEST_CHECK(xuiQrCodeSetIcon(pQrCode, pIcon,
		(xui_rect_t){(int)XUI_LAYOUT_UNBOUNDED + 1024, 0, 16, 16}, 16.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"reject out-of-range icon source");
	XUI_TEST_CHECK(xuiQrCodeSetIcon(pQrCode, pIcon,
		(xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f}, INFINITY) == XUI_ERROR_INVALID_ARGUMENT,
		"reject infinite icon size");

	memset(sLongValue, 'A', sizeof(sLongValue));
	sLongValue[sizeof(sLongValue) - 1] = '\0';
	iChangeCount = xuiQrCodeGetChangeCount(pQrCode);
	iVersion = xuiQrCodeGetVersion(pQrCode);
	sPreviousValue = xuiQrCodeGetValue(pQrCode);
	iRet = xuiQrCodeSetValue(pQrCode, sLongValue);
	XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL &&
		xuiQrCodeGetChangeCount(pQrCode) == iChangeCount &&
		xuiQrCodeGetVersion(pQrCode) == iVersion &&
		strcmp(xuiQrCodeGetValue(pQrCode), sPreviousValue) == 0,
		"oversized value rollback");

	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, XUI_QRCODE_MAX_VERSION);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand version range");
	sLongValue[30] = '\0';
	iChangeCount = xuiQrCodeGetChangeCount(pQrCode);
	iVersion = xuiQrCodeGetVersion(pQrCode);
	sPreviousValue = xuiQrCodeGetValue(pQrCode);
	memset(&tScope, 0, sizeof(tScope));
	XUI_TEST_CHECK(xrtAllocationBudgetCreate(1u, &pBudget), "create allocation budget");
	XUI_TEST_CHECK(xrtAllocationBudgetEnter(pBudget, &tScope), "enter allocation budget");
	iRet = xuiQrCodeSetValue(pQrCode, sLongValue);
	XUI_TEST_CHECK(xrtAllocationBudgetLeave(&tScope), "leave allocation budget");
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY &&
		xuiQrCodeGetChangeCount(pQrCode) == iChangeCount &&
		xuiQrCodeGetVersion(pQrCode) == iVersion &&
		strcmp(xuiQrCodeGetValue(pQrCode), sPreviousValue) == 0,
		"allocation failure preserves value and matrix");
	iRet = xuiQrCodeSetValue(pQrCode, sLongValue);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) > 1, "larger value version");
	iChangeCount = xuiQrCodeGetChangeCount(pQrCode);
	iVersion = xuiQrCodeGetVersion(pQrCode);
	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL &&
		xuiQrCodeGetChangeCount(pQrCode) == iChangeCount && xuiQrCodeGetVersion(pQrCode) == iVersion,
		"version range rollback");
	iRet = xuiQrCodeSetValue(pQrCode, "hello");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore short value");
	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetVersion(pQrCode) == 1, "restore version range");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiWidgetGetAccessibilityRevision(pQrCode) > iAccessibilityRevision,
		"value updates accessibility");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 260, 220};
	__xuiQrCodeTraceReset(XUI_COLOR_RGBA(18, 112, 214, 255));
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	pCache = xuiWidgetGetCacheSurface(pQrCode, 0);
	XUI_TEST_CHECK(pCache != NULL, "cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pTarget) > 0 || xuiTestSurfaceGetDrawCount(pTarget) > 0, "draw count");
	iDarkModules = 0;
	for ( iY = 0; iY < xuiQrCodeGetModuleCount(pQrCode); iY++ ) {
		for ( iX = 0; iX < xuiQrCodeGetModuleCount(pQrCode); iX++ ) {
			if ( xuiQrCodeGetModule(pQrCode, iX, iY) == 1 ) iDarkModules++;
		}
	}
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) < iDarkModules,
		"dark module runs are batched");
	tContentRect = xuiWidgetGetContentRect(pQrCode);
	XUI_TEST_CHECK(g_xuiQrCodeTraceCount > 0 && g_xuiQrCodeTraceIntegral,
		"modules render on integral pixel boundaries");
	XUI_TEST_CHECK(g_xuiQrCodeTraceFirstRect.fX == tContentRect.fX + 27.0f &&
		g_xuiQrCodeTraceFirstRect.fY == tContentRect.fY + 27.0f &&
		g_xuiQrCodeTraceFirstRect.fW == 42.0f && g_xuiQrCodeTraceFirstRect.fH == 6.0f,
		"configured padding preserves four-module quiet zone");
	XUI_TEST_CHECK(g_xuiQrCodeTraceBounds.fX >= tContentRect.fX + 24.0f &&
		g_xuiQrCodeTraceBounds.fY >= tContentRect.fY + 24.0f &&
		g_xuiQrCodeTraceBounds.fX + g_xuiQrCodeTraceBounds.fW <=
			tContentRect.fX + tContentRect.fW - 24.0f &&
		g_xuiQrCodeTraceBounds.fY + g_xuiQrCodeTraceBounds.fH <=
			tContentRect.fY + tContentRect.fH - 24.0f,
		"all modules remain inside quiet zone");

	iRet = xuiQrCodeSetPadding(pQrCode, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear padding for minimum-size render");
	iRet = xuiWidgetSetRect(pQrCode, (xui_rect_t){20.0f, 20.0f, 28.0f, 28.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set undersized qrcode rect");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout undersized qrcode");
	__xuiQrCodeTraceReset(XUI_COLOR_RGBA(18, 112, 214, 255));
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && g_xuiQrCodeTraceCount == 0,
		"undersized qrcode does not render subpixel modules");
	iRet = xuiWidgetSetRect(pQrCode, (xui_rect_t){20.0f, 20.0f, 29.0f, 29.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set minimum qrcode rect");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout minimum qrcode");
	__xuiQrCodeTraceReset(XUI_COLOR_RGBA(18, 112, 214, 255));
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && g_xuiQrCodeTraceCount > 0 &&
		g_xuiQrCodeTraceFirstRect.fX == 4.0f && g_xuiQrCodeTraceFirstRect.fY == 4.0f &&
		g_xuiQrCodeTraceFirstRect.fW == 7.0f && g_xuiQrCodeTraceFirstRect.fH == 1.0f,
		"minimum qrcode uses one-pixel modules and four-pixel quiet zone");
	iRet = xuiQrCodeSetPadding(pQrCode, 6.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore qrcode padding");
	iRet = xuiWidgetSetRect(pQrCode, (xui_rect_t){20.0f, 20.0f, 180.0f, 180.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore qrcode rect");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout restored qrcode");
	__xuiQrCodeTraceReset(0u);
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render restored qrcode");
	pCache = xuiWidgetGetCacheSurface(pQrCode, 0);
	XUI_TEST_CHECK(pCache != NULL, "restored cache surface");
	iDirtyFlags = xuiWidgetGetDirtyFlags(pQrCode);
	iChangeCount = xuiQrCodeGetChangeCount(pQrCode);
	iRet = xuiQrCodeSetValue(pQrCode, "hello");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetChangeCount(pQrCode) == iChangeCount &&
		xuiWidgetGetDirtyFlags(pQrCode) == iDirtyFlags, "same value is idempotent");
	iRet = xuiQrCodeSetVersionRange(pQrCode, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQrCodeGetChangeCount(pQrCode) == iChangeCount &&
		xuiWidgetGetDirtyFlags(pQrCode) == iDirtyFlags, "same version range is idempotent");
	iRet = xuiQrCodeSetColors(pQrCode, xuiQrCodeGetForegroundColor(pQrCode),
		xuiQrCodeGetBackgroundColor(pQrCode));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pQrCode) == iDirtyFlags,
		"same colors are idempotent");
	xuiTestSurfaceReset(pCache);
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pIcon,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "icon surface update");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update after icon surface change");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) > 0,
		"icon generation invalidates qrcode cache");

cleanup:
	if ( pBudget != NULL ) {
		xrtAllocationBudgetFree(pBudget);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pIcon);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_qrcode_test passed\n");
	return 0;
}
