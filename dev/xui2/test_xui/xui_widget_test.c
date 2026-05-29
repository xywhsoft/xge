#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_widget_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiTestRectEquals(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static int __xuiTestStatusAllowed(int iRet)
{
	return (iRet == XUI_OK) || (iRet == XUI_ERROR_NOT_INITIALIZED) || (iRet == XUI_ERROR_UNSUPPORTED);
}

int main(void)
{
	xge_desc_t tXgeDesc;
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_widget pRoot;
	xui_widget pChildA;
	xui_widget pChildB;
	xui_draw_context pDraw;
	xui_surface_desc_t tSurfaceDesc;
	xui_surface pTarget;
	xui_rect_i_t arrDamage[4];
	xui_rect_t tRect;
	unsigned char arrReadback[64 * 64 * 4];
	int iRet;
	int iFailed;
	int iXgeReady;

	pContext = NULL;
	pRoot = NULL;
	pChildA = NULL;
	pChildB = NULL;
	pDraw = NULL;
	pTarget = NULL;
	iFailed = 0;
	iXgeReady = 0;

	tProxy = xuiProxyXge();
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetProxy(pContext, &tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetProxy failed");
	iRet = xuiSetViewportSize(pContext, 64.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetViewportSize failed");
	xuiClearDamage(pContext);

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pChildA);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChildA != NULL), "child A create failed");
	iRet = xuiWidgetCreate(pContext, &pChildB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChildB != NULL), "child B create failed");

	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){1.0f, 2.0f, 20.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "unattached root rect set failed");
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 0, "unattached widget should not invalidate context damage");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root failed");
	XUI_TEST_CHECK(xuiGetRootWidget(pContext) == pRoot, "root getter failed");
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 1, "set root should invalidate context");
	xuiClearDamage(pContext);

	xuiWidgetSetUserData(pRoot, (void*)0x1234);
	XUI_TEST_CHECK(xuiWidgetGetUserData(pRoot) == (void*)0x1234, "user data failed");
	iRet = xuiWidgetSetRect(pChildA, (xui_rect_t){3.0f, 4.0f, 5.0f, 6.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "child A rect set failed");
	iRet = xuiWidgetAddChild(pRoot, pChildA);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child A failed");
	XUI_TEST_CHECK((xuiWidgetGetParent(pChildA) == pRoot) &&
	               (xuiWidgetGetFirstChild(pRoot) == pChildA) &&
	               (xuiWidgetGetLastChild(pRoot) == pChildA) &&
	               (xuiWidgetGetChildCount(pRoot) == 1), "child A tree links failed");
	tRect = xuiWidgetGetWorldRect(pChildA);
	XUI_TEST_CHECK(__xuiTestRectEquals(tRect, 4.0f, 6.0f, 5.0f, 6.0f), "child A world rect failed");
	XUI_TEST_CHECK(xuiGetDamageRects(pContext, arrDamage, 4) > 0, "add child should invalidate attached tree");
	xuiClearDamage(pContext);

	iRet = xuiWidgetSetRect(pChildB, (xui_rect_t){7.0f, 8.0f, 9.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "child B rect set failed");
	iRet = xuiWidgetInsertBefore(pRoot, pChildB, pChildA);
	XUI_TEST_CHECK(iRet == XUI_OK, "insert child B failed");
	XUI_TEST_CHECK((xuiWidgetGetFirstChild(pRoot) == pChildB) &&
	               (xuiWidgetGetLastChild(pRoot) == pChildA) &&
	               (xuiWidgetGetNextSibling(pChildB) == pChildA) &&
	               (xuiWidgetGetPrevSibling(pChildA) == pChildB) &&
	               (xuiWidgetGetChildCount(pRoot) == 2), "inserted child order failed");
	iRet = xuiWidgetRemoveFromParent(pChildA);
	XUI_TEST_CHECK(iRet == XUI_OK, "remove child A failed");
	XUI_TEST_CHECK((xuiWidgetGetParent(pChildA) == NULL) &&
	               (xuiWidgetGetChildCount(pRoot) == 1) &&
	               (xuiWidgetGetFirstChild(pRoot) == pChildB), "remove child A links failed");

	xuiClearDamage(pContext);
	iRet = xuiWidgetSetVisible(pChildB, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "set visible failed");
	XUI_TEST_CHECK((xuiWidgetGetVisible(pChildB) == 0) && (xuiHasDamage(pContext) == 1), "visibility should invalidate");
	iRet = xuiWidgetSetEnabled(pChildB, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "set enabled failed");
	XUI_TEST_CHECK(xuiWidgetGetEnabled(pChildB) == 0, "enabled getter failed");

	memset(&tXgeDesc, 0, sizeof(tXgeDesc));
	tXgeDesc.iWidth = 64;
	tXgeDesc.iHeight = 64;
	tXgeDesc.sTitle = "xui widget test";
	tXgeDesc.iFlags = XGE_INIT_OFFSCREEN;
	tXgeDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tXgeDesc);
	XUI_TEST_CHECK(iRet == XGE_OK, "xgeInit failed");
	iXgeReady = 1;

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 64;
	tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = tProxy.surfaceCreate(&tProxy, &pTarget, &tSurfaceDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pTarget != NULL), "target create failed");

	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "cache root rect set failed");
	iRet = xuiWidgetSetCacheStateCount(pRoot, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "set cache count failed");
	iRet = xuiWidgetSetCacheStateId(pRoot, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "set cache state 0 failed");
	iRet = xuiWidgetSetCacheStateId(pRoot, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set cache state 1 failed");
	XUI_TEST_CHECK((xuiWidgetGetCacheStateCount(pRoot) == 2) &&
	               (xuiWidgetGetCacheStateId(pRoot, 0) == 0) &&
	               (xuiWidgetGetCacheStateId(pRoot, 1) == 1), "cache state getters failed");
	iRet = xuiWidgetUpdateBegin(pRoot, 0, XUI_WIDGET_UPDATE_CLEAR, XUI_COLOR_RGBA(255, 0, 0, 255), &pDraw);
	if ( iRet == XUI_OK ) {
		XUI_TEST_CHECK(pDraw != NULL, "widget update begin should return draw context");
		iRet = xuiWidgetUpdateEnd(pRoot, 0, pDraw);
		pDraw = NULL;
		XUI_TEST_CHECK(iRet == XUI_OK, "widget update end failed");
		XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pRoot, 0) != NULL, "cache surface not created");
		iRet = xuiWidgetSetStateId(pRoot, 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "set state id failed");
		iRet = tProxy.surfaceClear(&tProxy, pTarget, XUI_COLOR_RGBA(0, 0, 0, 0));
		XUI_TEST_CHECK(iRet == XUI_OK, "target clear failed");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		if ( iRet == XUI_OK ) {
			memset(arrReadback, 0, sizeof(arrReadback));
			iRet = tProxy.surfaceReadRGBA(&tProxy, pTarget, arrReadback, 64 * 4);
			XUI_TEST_CHECK(iRet == XUI_OK, "target readback failed");
			XUI_TEST_CHECK((arrReadback[0] == 255) && (arrReadback[1] == 0) &&
			               (arrReadback[2] == 0) && (arrReadback[3] == 255), "rendered cache pixel mismatch");
			XUI_TEST_CHECK(xuiHasDamage(pContext) == 0, "xuiRender should consume damage");
		} else {
			XUI_TEST_CHECK(__xuiTestStatusAllowed(iRet), "xuiRender returned unexpected status");
		}
	} else {
		XUI_TEST_CHECK(__xuiTestStatusAllowed(iRet), "widget update begin returned unexpected status");
	}

cleanup:
	if ( pDraw != NULL ) {
		(void)xuiWidgetUpdateEnd(pRoot, 0, pDraw);
	}
	if ( pTarget != NULL ) {
		tProxy.surfaceDestroy(&tProxy, pTarget);
	}
	if ( iXgeReady ) {
		xgeUnit();
	}
	if ( pContext != NULL ) {
		if ( (pChildA != NULL) && (xuiWidgetGetParent(pChildA) == NULL) ) {
			xuiWidgetDestroy(pChildA);
		}
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_widget_test passed\n");
	return 0;
}
