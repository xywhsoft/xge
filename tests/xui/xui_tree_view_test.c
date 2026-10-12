#include "xui.h"
#include "xui_test_proxy.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef struct xui_tree_view_event_state_t {
	int iDoubleClickCount;
	int iContextMenuCount;
	int iContextNode;
	int iContextVisible;
	float fContextX;
	float fContextY;
} xui_tree_view_event_state_t;

static int __xuiTreeViewEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_tree_view_event_state_t* pState = (xui_tree_view_event_state_t*)pUser;
	(void)pWidget;
	if ( (pState == NULL) || (pEvent == NULL) ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) pState->iDoubleClickCount++;
	return XUI_OK;
}

static int __xuiTreeViewContext(xui_widget pWidget, int iNodeId, int iVisibleIndex, float fX, float fY, void* pUser)
{
	xui_tree_view_event_state_t* pState = (xui_tree_view_event_state_t*)pUser;
	(void)pWidget;
	if ( pState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pState->iContextMenuCount++;
	pState->iContextNode = iNodeId;
	pState->iContextVisible = iVisibleIndex;
	pState->fContextX = fX;
	pState->fContextY = fY;
	return XUI_EVENT_DISPATCH_STOP;
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

static xui_tree_view_node_t __xuiTreeViewTestNode(int iId, int iParent)
{
	xui_tree_view_node_t tNode;
	memset(&tNode, 0, sizeof(tNode));
	tNode.iId = iId;
	tNode.iParent = iParent;
	tNode.sText = "indexed node";
	tNode.bEnabled = 1;
	tNode.bExpanded = 1;
	return tNode;
}

static int __xuiTreeViewExpectOrder(xui_widget pTree, const int* pIds, int iCount)
{
	int i;
	if ( xuiTreeViewGetVisibleCount(pTree) != iCount ) return 0;
	for ( i = 0; i < iCount; i++ ) {
		if ( xuiTreeViewGetVisibleNodeId(pTree, i) != pIds[i] ||
		     xuiTreeViewGetVisibleIndexOfId(pTree, pIds[i]) != i ||
		     xuiTreeViewGetVisibleNode(pTree, i) != xuiTreeViewGetNodeById(pTree, pIds[i]) ) return 0;
	}
	return 1;
}

typedef struct xui_tree_view_adapter_test_t {
	xui_tree_view_node_t arrNodes[8];
	int iCount;
	int iFailedIndex;
} xui_tree_view_adapter_test_t;

static int __xuiTreeViewAdapterCount(xui_widget pTree, void* pUser)
{
	(void)pTree;
	return ((xui_tree_view_adapter_test_t*)pUser)->iCount;
}

static int __xuiTreeViewAdapterNode(xui_widget pTree, int iIndex, xui_tree_view_node_t* pNode, void* pUser)
{
	xui_tree_view_adapter_test_t* pAdapter = (xui_tree_view_adapter_test_t*)pUser;
	(void)pTree;
	if ( iIndex == pAdapter->iFailedIndex ) return XUI_ERROR_INVALID_ARGUMENT;
	*pNode = pAdapter->arrNodes[iIndex];
	return XUI_OK;
}

static int __xuiTreeViewIndexRegression(xui_widget pTree)
{
	xui_tree_view_node_t arrNodes[7];
	xui_tree_view_node_t arrInvalid[2];
	xui_tree_view_node_t tSwap;
	xui_tree_view_adapter_test_t tAdapter;
	int iFailed = 0;
	int i;
	int iChange;
	arrNodes[0] = __xuiTreeViewTestNode(0, -9);
	arrNodes[1] = __xuiTreeViewTestNode(INT_MAX, 0);
	arrNodes[2] = __xuiTreeViewTestNode(7, INT_MAX);
	arrNodes[3] = __xuiTreeViewTestNode(900, -1);
	arrNodes[4] = __xuiTreeViewTestNode(3, 0);
	arrNodes[5] = __xuiTreeViewTestNode(11, 3);
	arrNodes[6] = __xuiTreeViewTestNode(25, 900);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrNodes, 7) == XUI_OK, "set interleaved hierarchy");
	XUI_TEST_CHECK(__xuiTreeViewExpectOrder(pTree, (int[]){0, INT_MAX, 7, 3, 11, 900, 25}, 7), "preorder preserves sibling insertion order");
	for ( i = 0; i < 7; i++ ) {
		XUI_TEST_CHECK(xuiTreeViewFindNode(pTree, arrNodes[i].iId) == i, "ID maps to storage index, not visible index");
	}
	XUI_TEST_CHECK(xuiTreeViewGetNodeById(pTree, 11)->iDepth == 2, "visible depth");
	XUI_TEST_CHECK(xuiTreeViewGetNodeById(pTree, 0)->bHasChildren && !xuiTreeViewGetNodeById(pTree, 7)->bHasChildren, "children flags");
	XUI_TEST_CHECK(xuiTreeViewSetSelected(pTree, 7) == XUI_OK, "select descendant");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, INT_MAX, 0) == XUI_OK, "collapse middle subtree");
	XUI_TEST_CHECK(__xuiTreeViewExpectOrder(pTree, (int[]){0, INT_MAX, 3, 11, 900, 25}, 6), "collapse preserves suffix inverse indices");
	XUI_TEST_CHECK(xuiTreeViewGetVisibleIndexOfId(pTree, 7) == -1 && xuiTreeViewGetSelected(pTree) == INT_MAX && xuiTreeViewGetFocusIndex(pTree) == 1, "hidden selection falls back to collapsed parent");
	XUI_TEST_CHECK(xuiTreeViewSetNodeEnabled(pTree, INT_MAX, 0) == XUI_OK && xuiTreeViewGetSelected(pTree) == -1, "disable selected clears selection");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, INT_MAX, 0) == XUI_OK && xuiTreeViewGetSelected(pTree) == -1, "disabled collapse fallback is rejected");
	XUI_TEST_CHECK(xuiTreeViewSetNodeEnabled(pTree, INT_MAX, 1) == XUI_OK, "enable parent");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 0) == XUI_OK, "collapse ancestor");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, INT_MAX, 1) == XUI_OK && xuiTreeViewSetNodeExpanded(pTree, 3, 0) == XUI_OK, "change hidden expansion state");
	XUI_TEST_CHECK(__xuiTreeViewExpectOrder(pTree, (int[]){0, 900, 25}, 3), "hidden expansion does not insert rows");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 1) == XUI_OK, "restore ancestor");
	XUI_TEST_CHECK(__xuiTreeViewExpectOrder(pTree, (int[]){0, INT_MAX, 7, 3, 900, 25}, 6), "restore nested expansion state");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, 8, INT_MAX, "new sibling") == XUI_OK, "append to nonlast parent");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, 12, 3, "hidden sibling") == XUI_OK && xuiTreeViewGetVisibleIndexOfId(pTree, 12) == -1, "append hidden child");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 3, 1) == XUI_OK, "show appended hidden child");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, 14, 0, "after expanded sibling") == XUI_OK, "append after previous sibling subtree");
	XUI_TEST_CHECK(__xuiTreeViewExpectOrder(pTree, (int[]){0, INT_MAX, 7, 8, 3, 11, 12, 14, 900, 25}, 10), "incremental inserts retain DFS order");
	iChange = xuiTreeViewGetChangeCount(pTree);
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 1) == XUI_OK && xuiTreeViewGetChangeCount(pTree) == iChange + 1, "no-op expansion keeps change counter contract");
	XUI_TEST_CHECK(xuiTreeViewSetNodeEnabled(pTree, 0, 0) == XUI_OK && xuiTreeViewSetSelected(pTree, 12) == XUI_OK && xuiTreeViewGetSelected(pTree) == 12, "disabled parent does not disable children");
	arrInvalid[0] = __xuiTreeViewTestNode(123, -1);
	arrInvalid[1] = __xuiTreeViewTestNode(123, -1);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "duplicate replacement rejected");
	arrInvalid[1] = __xuiTreeViewTestNode(124, 999);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "missing parent rejected");
	arrInvalid[0] = __xuiTreeViewTestNode(123, 124);
	arrInvalid[1] = __xuiTreeViewTestNode(124, 123);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "cycle and forward parent rejected");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, 15, 15, "self") == XUI_ERROR_INVALID_ARGUMENT && xuiTreeViewAddNode(pTree, -1, -1, "negative") == XUI_ERROR_INVALID_ARGUMENT, "self parent and negative ID rejected");
	XUI_TEST_CHECK(xuiTreeViewGetNodeCount(pTree) == 10 && xuiTreeViewGetSelected(pTree) == 12 && xuiTreeViewFindNode(pTree, 123) == -1, "rejected edits preserve indices and selection");
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, xuiTreeViewGetNode(pTree, 0), 10) == XUI_OK && xuiTreeViewGetSelected(pTree) == -1, "self-backed replacement owns its storage and resets selection");
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrNodes, 7) == XUI_OK && xuiTreeViewFindNode(pTree, 12) == -1 && xuiTreeViewGetNodeById(pTree, 14) == NULL, "replacement deletion removes old IDs");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, 12, 900, "reinserted") == XUI_OK && xuiTreeViewGetNodeById(pTree, 12)->iParent == 900, "deleted ID reinserts under a different parent");
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, NULL, 0) == XUI_OK && xuiTreeViewFindNode(pTree, 0) == -1, "empty replacement clears indices");
	for ( i = 0; i < 300; i++ ) {
		XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, INT_MAX - i * 7919, -1, "sparse root") == XUI_OK, "grow sparse ID and row storage");
	}
	for ( i = 0; i < 300; i++ ) {
		XUI_TEST_CHECK(xuiTreeViewFindNode(pTree, INT_MAX - i * 7919) == i && xuiTreeViewGetVisibleIndexOfId(pTree, INT_MAX - i * 7919) == i, "IDs survive realloc and hash rehash");
	}
	XUI_TEST_CHECK(xuiTreeViewClear(pTree) == XUI_OK && xuiTreeViewFindNode(pTree, INT_MAX) == -1, "clear removes retained hash entries");
	XUI_TEST_CHECK(xuiTreeViewAddNode(pTree, INT_MAX, -1, "reused root") == XUI_OK && xuiTreeViewGetVisibleIndexOfId(pTree, INT_MAX) == 0, "clear then reuse ID");

	memset(&tAdapter, 0, sizeof(tAdapter));
	tAdapter.iFailedIndex = -1;
	tAdapter.iCount = 3;
	tAdapter.arrNodes[0] = __xuiTreeViewTestNode(100, -1);
	tAdapter.arrNodes[1] = __xuiTreeViewTestNode(300, 100);
	tAdapter.arrNodes[2] = __xuiTreeViewTestNode(200, 100);
	XUI_TEST_CHECK(xuiTreeViewSetAdapter(pTree, __xuiTreeViewAdapterCount, __xuiTreeViewAdapterNode, &tAdapter) == XUI_OK, "adapter initial refresh");
	XUI_TEST_CHECK(xuiTreeViewSetSelected(pTree, 300) == XUI_OK, "adapter select stable ID");
	tSwap = tAdapter.arrNodes[1];
	tAdapter.arrNodes[1] = tAdapter.arrNodes[2];
	tAdapter.arrNodes[2] = tSwap;
	XUI_TEST_CHECK(xuiTreeViewRefreshAdapter(pTree) == XUI_OK && __xuiTreeViewExpectOrder(pTree, (int[]){100, 200, 300}, 3), "adapter sibling sorting");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 300 && xuiTreeViewGetFocusIndex(pTree) == 2, "adapter sorting retains selected ID at new index");
	tAdapter.iCount = 2;
	XUI_TEST_CHECK(xuiTreeViewRefreshAdapter(pTree) == XUI_OK && xuiTreeViewFindNode(pTree, 300) == -1 && xuiTreeViewGetSelected(pTree) == -1, "adapter filtering deletes ID and hidden selection");
	tAdapter.arrNodes[0] = __xuiTreeViewTestNode(400, -1);
	tAdapter.arrNodes[1] = __xuiTreeViewTestNode(300, 400);
	tAdapter.arrNodes[2] = __xuiTreeViewTestNode(300, -1);
	tAdapter.arrNodes[3] = __xuiTreeViewTestNode(800, 999);
	tAdapter.arrNodes[4] = __xuiTreeViewTestNode(700, -1);
	tAdapter.iCount = 5;
	tAdapter.iFailedIndex = 4;
	XUI_TEST_CHECK(xuiTreeViewRefreshAdapter(pTree) == XUI_OK && __xuiTreeViewExpectOrder(pTree, (int[]){400, 300}, 2), "adapter ID reuse, duplicate/orphan/callback errors skipped");
	XUI_TEST_CHECK(xuiTreeViewGetNodeById(pTree, 300)->iParent == 400 && xuiTreeViewFindNode(pTree, 100) == -1, "adapter indices have no stale parent or ID");
	tAdapter.iCount = -1;
	XUI_TEST_CHECK(xuiTreeViewRefreshAdapter(pTree) == XUI_OK && xuiTreeViewGetNodeCount(pTree) == 0, "negative adapter count is empty");
	tAdapter.iCount = 2;
	XUI_TEST_CHECK(xuiTreeViewRefreshAdapter(pTree) == XUI_OK && xuiTreeViewGetNodeById(pTree, 300) != NULL, "adapter repopulates empty tree");
cleanup:
	(void)xuiTreeViewSetAdapter(pTree, NULL, NULL, NULL);
	return !iFailed;
}

static int __xuiTreeViewLargeRegression(xui_context pContext, xui_widget pTree, xui_surface pTarget, int* pRenderCount)
{
	const int iCount = 32768;
	xui_tree_view_node_t* pNodes = (xui_tree_view_node_t*)malloc((size_t)iCount * sizeof(*pNodes));
	int iFailed = 0;
	int i;
	int bChain;
	int iBefore;
	XUI_TEST_CHECK(pNodes != NULL, "large descriptors allocated");
	for ( bChain = 0; bChain <= 1; bChain++ ) {
		for ( i = 0; i < iCount; i++ ) pNodes[i] = __xuiTreeViewTestNode(i, (i == 0) ? -1 : (bChain ? i - 1 : 0));
		XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, pNodes, iCount) == XUI_OK, "large public replacement");
		XUI_TEST_CHECK(xuiTreeViewGetVisibleCount(pTree) == iCount && xuiTreeViewGetVisibleIndexOfId(pTree, iCount - 1) == iCount - 1, "large public ID/visible lookup");
		XUI_TEST_CHECK(xuiTreeViewGetNodeById(pTree, iCount - 1)->iDepth == (bChain ? iCount - 1 : 1), "large public depth");
		XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK && xuiUpdate(pContext, 0.016f) == XUI_OK, "large layout and update");
		XUI_TEST_CHECK(xuiTreeViewSetSelected(pTree, iCount - 1) == XUI_OK && xuiTreeViewEnsureVisible(pTree, iCount - 1) == XUI_OK, "large selected ID and scroll");
		iBefore = *pRenderCount;
		XUI_TEST_CHECK(__xuiTreeViewRender(pContext, pTarget) == XUI_OK, "large tree renders");
		XUI_TEST_CHECK(*pRenderCount > iBefore && *pRenderCount - iBefore <= 16, "large tree draws only viewport rows");
		XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 0) == XUI_OK && xuiTreeViewGetVisibleCount(pTree) == 1, "large public collapse");
		XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) == 0 && xuiTreeViewGetScroll(pTree) == 0.0f, "collapse clamps scroll and selection");
		XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 1) == XUI_OK && xuiTreeViewGetVisibleCount(pTree) == iCount, "large public expand");
	}
cleanup:
	free(pNodes);
	return !iFailed;
}

static int __xuiTreeViewPressedIndexRegression(xui_context pContext, xui_widget pTree)
{
	xui_tree_view_node_t arrNodes[4];
	xui_rect_t tItem;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iFailed = 0;
	int i;
	for ( i = 0; i < 4; i++ ) arrNodes[i] = __xuiTreeViewTestNode(i, (i == 1) ? 0 : -1);
	XUI_TEST_CHECK(xuiTreeViewSetNodes(pTree, arrNodes, 4) == XUI_OK && xuiLayout(pContext) == XUI_OK, "pressed-index tree");
	tWorld = xuiWidgetGetWorldRect(pTree);
	tItem = xuiTreeViewGetItemRect(pTree, 2);
	fX = tWorld.fX + tItem.fX + 90.0f;
	fY = tWorld.fY + tItem.fY + 10.0f;
	XUI_TEST_CHECK(xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "press suffix row");
	XUI_TEST_CHECK(xuiGetPointerCapture(pContext) == pTree, "pressed row captures");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 0) == XUI_OK, "splice above pressed row");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "release after suffix moves");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) != 3 && xuiGetPointerCapture(pContext) == NULL, "stale row index cannot activate the next ID");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 1) == XUI_OK, "restore pressed-index tree");
	tItem = xuiTreeViewGetItemRect(pTree, 1);
	fY = tWorld.fY + tItem.fY + 10.0f;
	XUI_TEST_CHECK(xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "press descendant");
	XUI_TEST_CHECK(xuiTreeViewSetNodeExpanded(pTree, 0, 0) == XUI_OK, "hide pressed descendant");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "release hidden descendant");
	XUI_TEST_CHECK(xuiTreeViewGetSelected(pTree) != 2 && xuiGetPointerCapture(pContext) == NULL, "hidden press is canceled and capture released");
cleanup:
	return !iFailed;
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
	xui_tree_view_event_state_t tEventState;
	int iFailed;
	int iRet;
	int i;
	char arrText[48][32];
	xui_tree_view_node_t arrInvalidNodes[3];

	pContext = NULL;
	pRoot = NULL;
	pTree = NULL;
	pTarget = NULL;
	pCache = NULL;
	iBorderColor = XUI_COLOR_RGBA(96, 148, 204, 255);
	iSelectCount = 0;
	iRenderCount = 0;
	memset(&tEventState, 0, sizeof(tEventState));
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
	iRet = xuiWidgetSetEventCallback(pTree, __xuiTreeViewEvent, &tEventState);
	XUI_TEST_CHECK(iRet == XUI_OK, "event callback");
	iRet = xuiTreeViewSetContextMenu(pTree, __xuiTreeViewContext, &tEventState);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
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
	memset(arrInvalidNodes, 0, sizeof(arrInvalidNodes));
	arrInvalidNodes[0].iId = 800;
	arrInvalidNodes[0].iParent = -1;
	arrInvalidNodes[0].sText = "candidate root";
	arrInvalidNodes[0].bEnabled = 1;
	arrInvalidNodes[1].iId = 801;
	arrInvalidNodes[1].iParent = 800;
	arrInvalidNodes[1].sText = "candidate child";
	arrInvalidNodes[1].bEnabled = 1;
	arrInvalidNodes[2].iId = 800;
	arrInvalidNodes[2].iParent = -1;
	arrInvalidNodes[2].sText = "duplicate";
	arrInvalidNodes[2].bEnabled = 1;
	iRet = xuiTreeViewSetNodes(pTree, arrInvalidNodes, 3);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject invalid replacement tree");
	XUI_TEST_CHECK(xuiTreeViewGetNodeCount(pTree) == 7 && xuiTreeViewGetNodeById(pTree, 30) != NULL &&
		xuiTreeViewGetSelected(pTree) == 30, "failed replacement preserves tree state");
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

	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 70));
	iRet = xuiInputPointerDown(pContext, tTreeWorld.fX + tItem.fX + 72.0f, tTreeWorld.fY + tItem.fY + 10.0f, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, tTreeWorld.fX + tItem.fX + 72.0f, tTreeWorld.fY + tItem.fY + 10.0f, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetSelected(pTree) == 70, "right click selects");
	XUI_TEST_CHECK(tEventState.iContextMenuCount == 1 && tEventState.iContextNode == 70,
		"right click semantic callback");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_CONTEXT_MENU, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEventState.iContextMenuCount == 2 &&
		tEventState.iContextNode == 70 && tEventState.iContextVisible >= 0 &&
		tEventState.fContextX >= tTreeWorld.fX && tEventState.fContextY >= tTreeWorld.fY,
		"menu key selected node anchor");

	tItem = xuiTreeViewGetItemRect(pTree, xuiTreeViewGetVisibleIndexOfId(pTree, 50));
	iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 84.0f, tTreeWorld.fY + tItem.fY + 10.0f);
	if ( iRet == XUI_OK ) iRet = __xuiTreeViewClick(pContext, tTreeWorld.fX + tItem.fX + 84.0f, tTreeWorld.fY + tItem.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTreeViewGetNodeExpanded(pTree, 50) == 1 && tEventState.iDoubleClickCount >= 1, "double click expands and notifies");

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
	XUI_TEST_CHECK(__xuiTreeViewIndexRegression(pTree), "indexed tree public API regressions");
	XUI_TEST_CHECK(__xuiTreeViewLargeRegression(pContext, pTree, pTarget, &iRenderCount), "large public tree regressions");
	XUI_TEST_CHECK(__xuiTreeViewPressedIndexRegression(pContext, pTree), "pressed row ID regressions");

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
