#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_message_list_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiMessageListEvent(xui_widget pWidget, const xui_message_list_event_t* pEvent, void* pUser)
{
	int* pCounts;
	(void)pWidget;
	pCounts = (int*)pUser;
	if ( pEvent == NULL || pCounts == NULL ) return;
	if ( pEvent->iEvent == XUI_MESSAGE_EVENT_SELECT ) pCounts[0]++;
	if ( pEvent->iEvent == XUI_MESSAGE_EVENT_CLICK ) pCounts[1]++;
	if ( pEvent->iEvent == XUI_MESSAGE_EVENT_SCROLL ) pCounts[2]++;
}

static int __xuiMessageListRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;
	tFullRect = (xui_rect_i_t){0, 0, 460, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiMessageListClick(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	xui_message_node_t arrNodes[4];
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pDialog;
	xui_surface pTarget;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	const xui_message_node_t* pNode;
	char sBuffer[2048];
	int arrEvents[3];
	int iNeed;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pDialog = NULL;
	pTarget = NULL;
	iFailed = 0;
	memset(arrEvents, 0, sizeof(arrEvents));
	memset(arrNodes, 0, sizeof(arrNodes));
	xuiTestProxyInit(&tState);

	arrNodes[0].iSize = sizeof(arrNodes[0]);
	arrNodes[0].iType = XUI_MESSAGE_NODE_OTHER;
	arrNodes[0].sId = "m1";
	arrNodes[0].sSender = "Anubis";
	arrNodes[0].sTime = "10:53";
	arrNodes[0].sText = "need to adapt avatar size and screen width limits";
	arrNodes[1].iSize = sizeof(arrNodes[1]);
	arrNodes[1].iType = XUI_MESSAGE_NODE_SYSTEM;
	arrNodes[1].sId = "s1";
	arrNodes[1].sTime = "yesterday 10:56";
	arrNodes[1].sText = "yesterday 10:56";
	arrNodes[2].iSize = sizeof(arrNodes[2]);
	arrNodes[2].iType = XUI_MESSAGE_NODE_SELF;
	arrNodes[2].sId = "m2";
	arrNodes[2].sSender = "Leaf";
	arrNodes[2].sTime = "10:57";
	arrNodes[2].sText = "mobile layout must support finger-first hit targets";

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 460.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 460.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");
	iRet = xuiMessageListCreate(pContext, &pDialog, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "dialog create");
	iRet = xuiWidgetAddChild(pRoot, pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK, "add dialog");
	xuiWidgetSetRect(pDialog, (xui_rect_t){20.0f, 18.0f, 420.0f, 300.0f});
	iRet = xuiMessageListSetEvent(pDialog, __xuiMessageListEvent, arrEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "event callback");
	iRet = xuiMessageListSetNodes(pDialog, arrNodes, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "set nodes");
	XUI_TEST_CHECK(xuiMessageListGetNodeCount(pDialog) == 3, "node count");
	XUI_TEST_CHECK(xuiMessageListGetNode(pDialog, 0)->iType == XUI_MESSAGE_NODE_OTHER, "other node");
	XUI_TEST_CHECK(xuiMessageListGetNode(pDialog, 1)->iType == XUI_MESSAGE_NODE_SYSTEM, "system node");
	XUI_TEST_CHECK(xuiMessageListGetNode(pDialog, 2)->iType == XUI_MESSAGE_NODE_SELF, "self node");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 460, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiMessageListRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	tWorld = xuiWidgetGetWorldRect(pDialog);
	tRect = xuiMessageListGetNodeRect(pDialog, 0);
	XUI_TEST_CHECK(tRect.fH > 0.0f, "node rect");
	iRet = __xuiMessageListClick(pContext, tWorld.fX + tRect.fX + 12.0f, tWorld.fY + tRect.fY + 12.0f - xuiMessageListGetScroll(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK, "click node");
	XUI_TEST_CHECK(xuiMessageListGetSelected(pDialog) == 0 && arrEvents[0] > 0, "select event");

	iNeed = xuiMessageListExportText(pDialog, sBuffer, sizeof(sBuffer));
	XUI_TEST_CHECK(iNeed > 0 && strstr(sBuffer, "MESSAGELIST1") != NULL && strstr(sBuffer, "mobile layout") != NULL, "export");
	iRet = xuiMessageListClear(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMessageListGetNodeCount(pDialog) == 0, "clear");
	iRet = xuiMessageListImportText(pDialog, sBuffer);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMessageListGetNodeCount(pDialog) == 3, "import");
	pNode = xuiMessageListGetNode(pDialog, 2);
	XUI_TEST_CHECK(pNode != NULL && strcmp(pNode->sId, "m2") == 0 && pNode->iType == XUI_MESSAGE_NODE_SELF, "imported self");

	arrNodes[3].iSize = sizeof(arrNodes[3]);
	arrNodes[3].iType = XUI_MESSAGE_NODE_OTHER;
	arrNodes[3].sId = "m3";
	arrNodes[3].sSender = "Anubis";
	arrNodes[3].sText = "new incoming message";
	iRet = xuiMessageListAddNode(pDialog, &arrNodes[3]);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMessageListGetNodeCount(pDialog) == 4, "add node");
	iRet = xuiMessageListEnsureVisible(pDialog, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_message_list_test passed\n");
	return 0;
}
