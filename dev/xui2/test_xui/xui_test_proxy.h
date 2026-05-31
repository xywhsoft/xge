#ifndef XUI_TEST_PROXY_H
#define XUI_TEST_PROXY_H

#include "xui.h"

typedef struct xui_test_proxy_state_t {
	xui_proxy_t tProxy;
	char sClipboard[4096];
	int iMeshDrawCount;
	int iLastMeshVertexCount;
	int iLastMeshIndexCount;
} xui_test_proxy_state_t;

void xuiTestProxyInit(xui_test_proxy_state_t* pState);
int xuiTestProxySetClipboardText(xui_test_proxy_state_t* pState, const char* sText);
const char* xuiTestProxyGetClipboardText(xui_test_proxy_state_t* pState);
int xuiTestSurfaceCreate(xui_test_proxy_state_t* pState, xui_surface* ppSurface, int iWidth, int iHeight, uint32_t iFlags);
void xuiTestSurfaceReset(xui_surface pSurface);
int xuiTestSurfaceGetDrawCount(xui_surface pSurface);
int xuiTestSurfaceGetRectFillCount(xui_surface pSurface);
int xuiTestSurfaceGetRectFillColorCount(xui_surface pSurface, uint32_t iColor);
int xuiTestSurfaceGetTextDrawCount(xui_surface pSurface);
xui_rect_t xuiTestSurfaceGetLastRect(xui_surface pSurface);
xui_rect_t xuiTestSurfaceGetLastSrc(xui_surface pSurface);
xui_rect_t xuiTestSurfaceGetLastDst(xui_surface pSurface);
xui_rect_t xuiTestSurfaceGetLastTextRect(xui_surface pSurface);
uint32_t xuiTestSurfaceGetLastColor(xui_surface pSurface);
uint32_t xuiTestSurfaceGetLastTextColor(xui_surface pSurface);
int xuiTestProxyGetMeshDrawCount(xui_test_proxy_state_t* pState);
int xuiTestProxyGetLastMeshVertexCount(xui_test_proxy_state_t* pState);
int xuiTestProxyGetLastMeshIndexCount(xui_test_proxy_state_t* pState);

#endif /* XUI_TEST_PROXY_H */
