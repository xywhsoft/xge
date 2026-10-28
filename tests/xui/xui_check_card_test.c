#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_check_card_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiCheckCardChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
}

static void __xuiCheckCardDestroyChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;

	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
}

static void __xuiCheckCardGroupChanged(xui_widget pGroup, xui_widget pSelected, int iSelectedIndex, void* pUser)
{
	int* pIndex;

	(void)pGroup;
	(void)pSelected;
	pIndex = (int*)pUser;
	if ( pIndex != NULL ) *pIndex = iSelectedIndex;
}

static int __xuiCheckCardClick(xui_context pContext, xui_widget pWidget)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pWidget);
	fX = tWorld.fX + tWorld.fW * 0.5f;
	fY = tWorld.fY + tWorld.fH * 0.5f;
	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCheckCardRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 420, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pStandalone;
	xui_widget pReparentCard;
	xui_widget pDestroyCard;
	xui_widget pInvalidCard;
	xui_widget pGroup;
	xui_widget pCard[3];
	xui_widget pChild;
	xui_widget pLabel;
	xui_surface pTarget;
	xui_check_card_desc_t tDesc;
	xui_check_card_desc_t tInvalidDesc;
	xui_label_desc_t tLabelDesc;
	xui_radio_group_desc_t tGroupDesc;
	xui_accessible_node_t tAccessible;
	xui_rect_t tCorner;
	xui_rect_t tCardRect;
	xui_rect_t tLabelWorld;
	float fBorder;
	float fCheckedBorder;
	float fCorner;
	float fFocus;
	int iChanged;
	int iDestroyedChanges;
	int iGroupIndex;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pStandalone = NULL;
	pReparentCard = NULL;
	pDestroyCard = NULL;
	pInvalidCard = NULL;
	pGroup = NULL;
	pLabel = NULL;
	pTarget = NULL;
	memset(pCard, 0, sizeof(pCard));
	iChanged = 0;
	iDestroyedChanges = 0;
	iGroupIndex = -2;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 420.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 420.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMinWidth = 180.0f;
	tDesc.fMinHeight = 56.0f;
	iRet = xuiCheckCardCreate(pContext, &pStandalone, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pStandalone != NULL, "standalone create");
	iRet = xuiCheckCardSetChange(pStandalone, __xuiCheckCardChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change");
	iRet = xuiWidgetAddChild(pRoot, pStandalone);
	XUI_TEST_CHECK(iRet == XUI_OK, "add standalone");
	xuiWidgetSetRect(pStandalone, (xui_rect_t){12.0f, 12.0f, 180.0f, 56.0f});
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pStandalone) == 1, "focusable");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pStandalone, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetAccessibleNodeCount(pStandalone) == 1 &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)) != 0u,
		"standalone accessibility");

	memset(&tGroupDesc, 0, sizeof(tGroupDesc));
	tGroupDesc.iSize = sizeof(tGroupDesc);
	tGroupDesc.iOrientation = XUI_RADIO_GROUP_VERTICAL;
	tGroupDesc.fGap = 8.0f;
	iRet = xuiRadioGroupCreate(pContext, &pGroup, &tGroupDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGroup != NULL, "group create");
	iRet = xuiRadioGroupSetChange(pGroup, __xuiCheckCardGroupChanged, &iGroupIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "group change");
	iRet = xuiWidgetAddChild(pRoot, pGroup);
	XUI_TEST_CHECK(iRet == XUI_OK, "add group");
	xuiWidgetSetRect(pGroup, (xui_rect_t){210.0f, 12.0f, 190.0f, 200.0f});

	for ( iRet = 0; iRet < 3; iRet++ ) {
		memset(&tDesc, 0, sizeof(tDesc));
		tDesc.iSize = sizeof(tDesc);
		tDesc.bChecked = (iRet == 1);
		tDesc.fMinWidth = 160.0f;
		tDesc.fMinHeight = 48.0f;
		XUI_TEST_CHECK(xuiCheckCardCreate(pContext, &pCard[iRet], &tDesc) == XUI_OK, "card create");
		XUI_TEST_CHECK(xuiRadioGroupAddCheckCard(pGroup, pCard[iRet]) == XUI_OK, "add card to group");
	}
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 1 && xuiRadioGroupGetSelectedWidget(pGroup) == pCard[1], "initial group selection");
	XUI_TEST_CHECK(xuiCheckCardGetChecked(pCard[1]) && !xuiCheckCardGetChecked(pCard[0]), "initial checked");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pCard[0]) == 0 && xuiWidgetGetTabStop(pCard[1]) == 1 &&
		xuiWidgetGetTabStop(pCard[2]) == 0, "selected card is the only group tab stop");
	iRet = xuiWidgetSetEnabled(pCard[1], 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetTabStop(pCard[0]) == 1 &&
		xuiWidgetGetTabStop(pCard[1]) == 0 && xuiWidgetGetTabStop(pCard[2]) == 0,
		"disabled card selection falls back to first eligible tab stop");
	iRet = xuiWidgetSetEnabled(pCard[1], 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetTabStop(pCard[0]) == 0 &&
		xuiWidgetGetTabStop(pCard[1]) == 1 && xuiWidgetGetTabStop(pCard[2]) == 0,
		"reenabled selected card regains the group tab stop");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pCard[1], 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_RADIO_BUTTON &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_CHECKED) != 0u &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)) == 0u,
		"grouped accessibility");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiCheckCardCreate(pContext, &pReparentCard, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pReparentCard != NULL, "reparent card create");
	iRet = xuiWidgetAddChild(pRoot, pReparentCard);
	XUI_TEST_CHECK(iRet == XUI_OK, "reparent card initial parent");
	iRet = xuiCheckCardSetRadioGroup(pReparentCard, pGroup);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pReparentCard) == pGroup &&
		xuiCheckCardGetRadioGroup(pReparentCard) == pGroup, "radio group reparents attached card");
	iRet = xuiRadioGroupSetSelectedWidget(pGroup, pReparentCard);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckCardGetChecked(pReparentCard),
		"reparent card selected");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pCard[0]) == 0 && xuiWidgetGetTabStop(pCard[1]) == 0 &&
		xuiWidgetGetTabStop(pCard[2]) == 0 && xuiWidgetGetTabStop(pReparentCard) == 1,
		"selected reparented card owns the group tab stop");
	iRet = xuiCheckCardSetRadioGroup(pReparentCard, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pReparentCard) == NULL &&
		xuiCheckCardGetRadioGroup(pReparentCard) == NULL &&
		xuiRadioGroupGetSelectedWidget(pGroup) == NULL && xuiCheckCardGetChecked(pReparentCard) &&
		iGroupIndex == -1,
		"radio group detach reconciles selection");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pReparentCard) == 1 && xuiWidgetGetTabStop(pCard[0]) == 1 &&
		xuiWidgetGetTabStop(pCard[1]) == 0 && xuiWidgetGetTabStop(pCard[2]) == 0,
		"detached card restores standalone tab stop and group fallback");
	iRet = xuiWidgetAddChild(pRoot, pReparentCard);
	XUI_TEST_CHECK(iRet == XUI_OK, "reattach ungrouped card");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	iRet = xuiWidgetAddChild(pStandalone, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pChild) == pStandalone, "content child");
	xuiWidgetSetRect(pChild, (xui_rect_t){10.0f, 10.0f, 40.0f, 20.0f});
	memset(&tLabelDesc, 0, sizeof(tLabelDesc));
	tLabelDesc.iSize = sizeof(tLabelDesc);
	tLabelDesc.sText = "label";
	iRet = xuiLabelCreate(pContext, &pLabel, &tLabelDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLabel != NULL, "label child create");
	iRet = xuiWidgetAddChild(pStandalone, pLabel);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pLabel) == pStandalone, "label child add");
	xuiWidgetSetRect(pLabel, (xui_rect_t){70.0f, 14.0f, 80.0f, 20.0f});

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 420, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiCheckCardRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	tLabelWorld = xuiWidgetGetWorldRect(pLabel);
	XUI_TEST_CHECK(xuiWidgetGetHitTestVisible(pLabel) == 0, "label hit transparent by default");
	XUI_TEST_CHECK(xuiHitTest(pContext, tLabelWorld.fX + 4.0f, tLabelWorld.fY + 4.0f, XUI_WIDGET_HIT_DEFAULT) == pStandalone, "label passes hit to parent");
	tCorner = xuiCheckCardGetCornerRect(pCard[1]);
	XUI_TEST_CHECK(tCorner.fW > 0.0f && tCorner.fH > 0.0f, "corner rect");
	tCardRect = xuiWidgetGetContentRect(pCard[1]);
	XUI_TEST_CHECK(tCorner.fY >= tCardRect.fY + 1.0f && tCorner.fX + tCorner.fW <= tCardRect.fX + tCardRect.fW - 1.0f, "corner inside border");

	iRet = __xuiCheckCardClick(pContext, pStandalone);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckCardGetChecked(pStandalone) && iChanged == 1, "standalone toggles on");
	iRet = __xuiCheckCardClick(pContext, pStandalone);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCheckCardGetChecked(pStandalone) && iChanged == 2, "standalone toggles off");
	tLabelWorld = xuiWidgetGetWorldRect(pStandalone);
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tLabelWorld.fX + tLabelWorld.fW * 0.5f),
		(int)(tLabelWorld.fY + tLabelWorld.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pStandalone,
		"first touch owns check card press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tLabelWorld.fX + tLabelWorld.fW * 0.5f),
		(int)(tLabelWorld.fY + tLabelWorld.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tLabelWorld.fX + tLabelWorld.fW * 0.5f),
		(int)(tLabelWorld.fY + tLabelWorld.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCheckCardGetChecked(pStandalone) &&
		iChanged == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pStandalone &&
		(xuiCheckCardGetState(pStandalone) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot click or release check card press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tLabelWorld.fX + tLabelWorld.fW * 0.5f),
		(int)(tLabelWorld.fY + tLabelWorld.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckCardGetChecked(pStandalone) &&
		iChanged == 3 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch commits check card click once");
	iRet = xuiCheckCardSetChecked(pStandalone, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset check card before keyboard blur");
	iRet = xuiSetFocusWidget(pContext, pStandalone);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus standalone");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down before blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiCheckCardGetState(pStandalone) & XUI_WIDGET_STATE_ACTIVE) != 0,
	               "active before blur");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during keyboard press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiCheckCardGetState(pStandalone) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up after blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCheckCardGetChecked(pStandalone) && iChanged == 3,
	               "blurred keyboard press does not toggle");

	iRet = __xuiCheckCardClick(pContext, pCard[0]);
	XUI_TEST_CHECK(iRet == XUI_OK, "group click");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 0 && iGroupIndex == 0, "group selected index");
	XUI_TEST_CHECK(xuiCheckCardGetChecked(pCard[0]) && !xuiCheckCardGetChecked(pCard[1]), "group unchecks sibling");

	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckCardGetChecked(pCard[2]) && !xuiCheckCardGetChecked(pCard[0]), "program selected index");
	iRet = xuiWidgetSetEnabled(pCard[1], 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable middle card");
	iRet = xuiSetFocusWidget(pContext, pCard[2]);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus grouped card");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "grouped card left key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedWidget(pGroup) == pCard[0] &&
		xuiGetFocusWidget(pContext) == pCard[0], "grouped card arrow navigation skips disabled option");
	iRet = xuiWidgetSetEnabled(pCard[1], 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable middle card");
	iRet = xuiCheckCardSetMetrics(pStandalone, 1.0f, 2.0f, 18.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiCheckCardGetMetrics(pStandalone, &fBorder, &fCheckedBorder, &fCorner, &fFocus);
	XUI_TEST_CHECK(iRet == XUI_OK && fBorder == 1.0f && fCheckedBorder == 2.0f && fCorner == 18.0f && fFocus == 3.0f, "get metrics");
	iRet = xuiCheckCardSetMetrics(pStandalone, NAN, 2.0f, 18.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan metrics");
	iRet = xuiCheckCardGetMetrics(pStandalone, &fBorder, &fCheckedBorder, &fCorner, &fFocus);
	XUI_TEST_CHECK(iRet == XUI_OK && fBorder == 1.0f && fCheckedBorder == 2.0f && fCorner == 18.0f && fFocus == 3.0f,
	               "invalid metrics preserve state");
	iRet = xuiCheckCardSetColors(pStandalone, XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(250, 252, 252, 255), XUI_COLOR_RGBA(244, 248, 248, 255), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(220, 220, 220, 255), XUI_COLOR_RGBA(120, 210, 205, 255), XUI_COLOR_RGBA(30, 190, 180, 255), XUI_COLOR_RGBA(30, 190, 180, 255), XUI_COLOR_RGBA(255, 255, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiCheckCardCreate(pContext, &pDestroyCard, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyCard != NULL, "destroy callback card create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyCard);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback card add");
	iRet = xuiCheckCardSetChange(pDestroyCard, __xuiCheckCardDestroyChanged, &iDestroyedChanges);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback setup");
	iRet = xuiCheckCardSetCheckedFromGroup(pDestroyCard, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyedChanges == 1,
		"change callback may destroy card without stale access");
	pDestroyCard = NULL;

	tInvalidDesc = tDesc;
	tInvalidDesc.fBorderWidth = NAN;
	pInvalidCard = pRoot;
	iRet = xuiCheckCardCreate(pContext, &pInvalidCard, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidCard == NULL,
		"invalid create clears output");

cleanup:
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_check_card_test passed\n");
	return 0;
}
