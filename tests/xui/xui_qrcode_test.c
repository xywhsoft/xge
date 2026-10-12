#include "xui.h"
#include "xui_test_proxy.h"

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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pQrCode;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_qrcode_desc_t tDesc;
	xui_rect_i_t tFullRect;
	int iVersion;
	int iModuleCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pQrCode = NULL;
	pTarget = NULL;
	pIcon = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

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

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 260, 220};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pQrCode, 0) != NULL, "cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pTarget) > 0 || xuiTestSurfaceGetDrawCount(pTarget) > 0, "draw count");

cleanup:
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
