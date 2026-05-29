#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_context_test failed: %s\n", (msg)); \
			return 1; \
		} \
	} while ( 0 )

static int __xuiTestRectEquals(xui_rect_i_t tRect, int iX, int iY, int iW, int iH)
{
	return (tRect.iX == iX) && (tRect.iY == iY) && (tRect.iW == iW) && (tRect.iH == iH);
}

int main(void)
{
	xge_desc_t tXgeDesc;
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_proxy_t tBadProxy;
	xui_proxy_caps_t tCaps;
	xui_vec2_t tSize;
	xui_surface_desc_t tSurfaceDesc;
	xui_surface pTarget;
	xui_rect_i_t arrRects[4];
	int iRet;
	int iCount;

	pContext = NULL;
	XUI_TEST_CHECK(xuiCreate(NULL) == XUI_ERROR_INVALID_ARGUMENT, "xuiCreate should reject null out parameter");
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 0, "new context should not have damage");
	XUI_TEST_CHECK(xuiGetVirtualDpi(pContext) == 1.0f, "default virtual dpi should be 1.0");
	tSize = xuiGetViewportSize(pContext);
	XUI_TEST_CHECK((tSize.fX == 0.0f) && (tSize.fY == 0.0f), "default viewport should be empty");
	XUI_TEST_CHECK(xuiGetProxy(pContext, &tProxy) == XUI_ERROR_NOT_INITIALIZED, "proxy should not be initialized by default");

	tProxy = xuiProxyXge();
	tBadProxy = tProxy;
	tBadProxy.drawText = NULL;
	XUI_TEST_CHECK(xuiSetProxy(pContext, &tBadProxy) == XUI_ERROR_UNSUPPORTED, "proxy validation should reject missing required callback");
	iRet = xuiSetProxy(pContext, &tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetProxy failed");
	memset(&tCaps, 0, sizeof(tCaps));
	iRet = xuiGetProxyCaps(pContext, &tCaps);
	XUI_TEST_CHECK((iRet == XUI_OK) && ((tCaps.iCaps & XUI_PROXY_CAP_DRAW_CONTEXT) != 0), "xuiGetProxyCaps failed");
	XUI_TEST_CHECK(xuiSetProxy(pContext, &tProxy) == XUI_ERROR_ALREADY_INITIALIZED, "proxy should only be bound once");

	iRet = xuiSetViewportSize(pContext, 100.2f, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetViewportSize failed");
	iCount = xuiGetDamageRects(pContext, arrRects, 4);
	XUI_TEST_CHECK((iCount == 1) && __xuiTestRectEquals(arrRects[0], 0, 0, 101, 60), "viewport resize should invalidate full viewport");

	xuiClearDamage(pContext);
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 0, "xuiClearDamage failed");
	iRet = xuiInvalidateRect(pContext, (xui_rect_i_t){10, 10, 10, 10});
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiInvalidateRect failed");
	iRet = xuiInvalidateRect(pContext, (xui_rect_i_t){20, 10, 10, 10});
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiInvalidateRect merge failed");
	iCount = xuiGetDamageRects(pContext, arrRects, 4);
	XUI_TEST_CHECK((iCount == 1) && __xuiTestRectEquals(arrRects[0], 10, 10, 20, 10), "touching damage rects should merge");

	xuiClearDamage(pContext);
	iRet = xuiInvalidateRect(pContext, (xui_rect_i_t){-5, -5, 10, 10});
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiInvalidateRect clip failed");
	iCount = xuiGetDamageRects(pContext, arrRects, 4);
	XUI_TEST_CHECK((iCount == 1) && __xuiTestRectEquals(arrRects[0], 0, 0, 5, 5), "damage rect should clip to viewport");

	iRet = xuiSetVirtualDpi(pContext, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiSetVirtualDpi failed");
	iCount = xuiGetDamageRects(pContext, arrRects, 4);
	XUI_TEST_CHECK((iCount == 1) && __xuiTestRectEquals(arrRects[0], 0, 0, 101, 60), "dpi change should invalidate full viewport");
	xuiClearDamage(pContext);

	memset(&tXgeDesc, 0, sizeof(tXgeDesc));
	tXgeDesc.iWidth = 64;
	tXgeDesc.iHeight = 64;
	tXgeDesc.sTitle = "xui context test";
	tXgeDesc.iFlags = XGE_INIT_OFFSCREEN;
	tXgeDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tXgeDesc);
	XUI_TEST_CHECK(iRet == XGE_OK, "xgeInit failed");

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 64;
	tSurfaceDesc.iHeight = 64;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	pTarget = NULL;
	iRet = tProxy.surfaceCreate(&tProxy, &pTarget, &tSurfaceDesc);
	if ( (iRet != XUI_OK) || (pTarget == NULL) ) {
		xgeUnit();
		xuiDestroy(pContext);
		printf("xui_context_test failed: target surface create ret=%d\n", iRet);
		return 1;
	}

	iRet = xuiInvalidateRect(pContext, (xui_rect_i_t){1, 1, 4, 4});
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate before render failed");
	arrRects[0] = (xui_rect_i_t){0, 0, 0, 1};
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, arrRects, 1) == XUI_ERROR_INVALID_ARGUMENT, "xuiRender should reject invalid explicit damage");
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 1, "failed render should not consume damage");
	arrRects[0] = (xui_rect_i_t){0, 0, 16, 16};
	iRet = xuiRender(pContext, pTarget, arrRects, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "xuiRender failed");
	XUI_TEST_CHECK(xuiHasDamage(pContext) == 0, "xuiRender should consume damage");

	tProxy.surfaceDestroy(&tProxy, pTarget);
	xgeUnit();
	xuiDestroy(pContext);
	printf("xui_context_test passed\n");
	return 0;
}
