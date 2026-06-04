#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_tree_view_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiTreeViewSelected(xui_widget pWidget, int iNodeId, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iNodeId;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiTreeViewRenderItem(xui_widget pWidget, int iNodeId, int iVisible, const xui_tree_view_node_t* pNode, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iNodeId;
	(void)iVisible;
	(void)pNode;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
	return 0;
}

static int g_iTreeViewTriangleCount = 0;
static int g_iTreeViewWhiteTriangleCount = 0;

static int __xuiTreeViewTestDrawTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	(void)tA;
	(void)tB;
	(void)tC;
	g_iTreeViewTriangleCount++;
	if ( iColor == XUI_COLOR_RGBA(255, 255, 255, 255) ) {
		g_iTreeViewWhiteTriangleCount++;
	}
	if ( (pProxy != NULL) && (pProxy->drawRectFill != NULL) ) {
		return pProxy->drawRectFill(pProxy, pDraw, (xui_rect_t){0.0f, 0.0f, 1.0f, 1.0f}, iColor);
	}
	return XUI_OK;
}

static int __xuiTreeViewRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 520, 340};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTreeViewClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTree;
	xui_surface pTarget;
	xui_surface pCache;
	xui_tree_view_desc_t tDesc;
	xui_rect_t tItem;
	xui_rect_t tTreeWorld;
	uint32_t iBorderColor;
	float fScroll;
	int iSelectCount;
	int iRenderCount;
	int iFailed;
	int iRet;
	int i;
	char arrText[48][32];

	pContext = NULL;
	pRoot = NULL;
	pTree = NULL;
	pTarget = NULL;
	pCache = NULL;
	iBorderColor = XUI_COLOR_RGBA(96, 148, 204, 255);
	iSelectCount = 0;
	iRenderCount = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	tState.tProxy.drawTriangleFill = __xuiTreeViewTestDrawTriangleFill;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 520.0f, 340.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 340.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fItemHeight = 22.0f;
	iRet = xuiTreeViewCreate(pContext, &pTree, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTree != NULL, "treeview create");
	iRet = xuiWidgetAddChild(pRoot, pTree);
	XUI_TEST_CHECK(iRet == XUI_OK, "add treeview");
	xuiWidgetSetRect(pTree, (xui_rect_t){24.0f, 20.0f, 240.0f, 132.0f});
	iRet = xuiTreeViewSetSelect(pTree, __xuiTreeViewSelected, &iSelectCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiTreeViewSetItemRenderer(pTree, __xuiTreeViewRenderItem, &iRenderCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "renderer callback");
	iRet = xuiTreeViewSetColors(pTree,
		XUI_COLOR_RGBA(247, 251, 255, 255),
		iBorderColor,
		XUI_COLOR_RGBA(0, 0, 0, 0),
		XUI_COLOR_RGBA(255, 255, 255, 0),
		XUI_COLOR_RGBA(228, 240, 252, 255),
		XUI_COLOR_RGBA(47, 128, 237, 255),
		XUI_COLOR_RGBA(31, 49, 68, 255),
		XUI_COLOR_RGBA(150, 162, 176, 180));
	XUI_TEST_CHECK(iRet == XUI_OK, "transparent focus color setup");

	iRet = xuiTreeViewAddNode(pTree, 10, -1, "Project");
	XUI_TEST_CHECK(iRet == XUI_OK, "add root");
	iRet = xuiTreeViewAddNode(pTree, 20, 10, "src");
	XUI_TEST_CHECK(iRet == XUI_OK, "add src");
	iRet = xuiTreeViewAddNode(pTree, 30, 20, "xui_tree_view.c");
	XUI_TEST_CHECK(iRet == XUI_OK, "add source file");
	iRet = xuiTreeViewAddNode(pTree, 40, 20, "disabled file");
	XUI_TEST_CHECK(iRet == XUI_OK, "add disabled file");
	iRet = xuiTreeViewAddNode(pTree, 50, 10, "examples");
	XUI_TEST_CHECK(iRet == XUI_OK, "add examples");
	iRet = xuiTreeViewAddNode(pTree, 60, 50, "xui_treeview");
	XUI_TEST_CHECK(iRet == XUI_OK, "add hidden child");
	iRet = xuiTreeViewAddNode(pTree, 70, 10, "docs");
	XUI_TEST_CHECK(iRet == XUI_OK, "add docs");
	iRet = xuiTreeViewSetNodeExpanded(pTree, 10, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand root");
	iRet = xuiTreeViewSetNodeExpanded(pTree, 20, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand src");
	iRet = xuiTreeViewSetNodeEnabled(pTree, 40, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable node");
	iRet = xuiTreeViewSetNodeChecked(pTree, 30, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "check node");
	iRet = xuiTreeViewSetNodeDecorations(pTree, 70, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "decorations");
	iRet = xuiTreeViewSetSelected(pTree, 30);
	XUI_TEST_CHECK(iRet == XUI_OK, "set selected");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 340, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiTreeViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && iRenderCount > 0, "render rows");

	XUI_TEST_CHECK(xuiTreeViewGetNodeCount(pTree) == 7, "node count");
	XUI_TEST_CHECK(xuiTreeViewGetVisibleCount(pTree) == 6, "visible count");
	XUI_TEST_CHECK(xuiTreeViewGetVisibleNodeId(pTree, 2) == 30, "visible node id");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 30, "initial selected");
	XUI_TEST_CHECK(xuiTreeViewGetNodeEnabled(pTree, 40) == 0, "disabled node state");
	XUI_TEST_CHECK(xuiTreeViewGetNodeChecked(pTree, 30) == 1, "checked state");
	XUI_TEST_CHECK(xuiTreeViewGetFrameWidget(pTree) != NULL, "frame exists");
	XUI_TEST_CHECK(xuiTreeViewGetViewportWidget(pTree) != NULL, "viewport exists");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(xuiTreeViewGetFrameWidget(pTree)), "vertical bar visible");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(xuiTreeViewGetFrameWidget(pTree)), "horizontal bar hidden");
	g_iTreeViewTriangleCount = 0;
	g_iTreeViewWhiteTriangleCount = 0;
	iRet = xuiTreeViewSetSelected(pTree, 20);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetSelected(pTree) == 20, "select expandable row");
	iRet = __xuiTreeViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && g_iTreeViewTriangleCount > 0, "expander rendered");
	XUI_TEST_CHECK(g_iTreeViewWhiteTriangleCount > 0, "selected expander contrast");
	iRet = xuiTreeViewSetSelected(pTree, 30);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetSelected(pTree) == 30, "restore selected row");

	tTreeWorld = xuiWidgetGetWorldRect(pTree);
	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 40));
	iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 88.0f, tTreeWorld.fY + tItem.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click disabled");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 30 && iSelectCount == 0, "disabled not selected");

	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 50));
	iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 86.0f, tTreeWorld.fY + tItem.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click row");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 50 && iSelectCount == 1, "row selects");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pTree, "click focus");
	iRet = __xuiTreeViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render focused tree");
	pCache = xuiWidgetGetCacheSurface(pTree, xuiWidgetGetStateId(pTree));
	XUI_TEST_CHECK((pCache != NULL) && (xuiTestSurfaceGetLastColor(pCache) == iBorderColor), "focused tree keeps border");

	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 30));
	iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 66.0f, tTreeWorld.fY + tItem.fY + 11.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click checkbox");
	XUI_TEST_CHECK(xuiTreeViewGetNodeChecked(pTree, 30) == 0, "checkbox toggles");

	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 10));
	iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 12.0f, tTreeWorld.fY + tItem.fY + 11.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click expander");
	XUI_TEST_CHECK(xuiTreeViewGetNodeExpanded(pTree, 10) == 0 && xuiTreeViewGetVisibleCount(pTree) == 1, "expander collapses");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 10, "collapse fallback selected");

	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetNodeExpanded(pTree, 10) == 1, "keyboard right expands");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetSelected(pTree) == 20, "keyboard down selects child");

	iRet = xuiTreeViewSetNodeExpanded(pTree, 50, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand examples");
	for ( i = 0; i < 36; i++ ) {
		snprintf(arrText[i], sizeof(arrText[i]), "Generated item %02d", i + 1);
		iRet = xuiTreeViewAddNode(pTree, 1000 + i, 50, arrText[i]);
		XUI_TEST_CHECK(iRet == XUI_OK, "add scroll item");
	}
	iRet = xuiTreeViewEnsureVisible(pTree, 1035);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	fScroll = xuiTreeViewGetScroll(pTree);
	XUI_TEST_CHECK(fScroll > 0.0f, "ensure scrolls");
	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 1035));
	XUI_TEST_CHECK(tItem.fY + tItem.fH <= xuiTreeViewGetViewportRect(pTree).fY + xuiTreeViewGetViewportRect(pTree).fH + 0.01f, "item visible");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_tree_view_test passed\n");
	return 0;
}
