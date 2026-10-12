#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stddef.h>

typedef struct test_t {
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pWidget;
	xui_widget pOther;
	xui_test_proxy_state_t tProxy;
	const char* sCase;
	int iFailures;
	int iActions;
	int iFocusEvents;
	int iNodeEvents;
	int iBoundsEvents;
	int iStateEvents;
	int iValueEvents;
	int iTreeEvents;
	int iSelectionEvents;
	int iCallbacks;
	int bDestroyed;
	int bInsideSetter;
} test_t;

static int is_family(const char* sName, const char* sFamily)
{
	size_t n = strlen(sFamily);
	return strncmp(sName, sFamily, n) == 0 && (sName[n] == 0 || sName[n] == '_');
}

#define CHECK(s, e, message) do { if (!(e)) { \
	printf("FAIL %s: %s (%d)\n", (s)->sCase, message, __LINE__); (s)->iFailures++; \
} } while (0)

static void clicked(xui_widget pWidget, void* pUser)
{
	test_t* s = (test_t*)pUser;
	(void)pWidget;
	s->iActions++;
	if (strcmp(s->sCase, "action_reentry") == 0) {
		CHECK(s, xuiWidgetPerformAccessibleAction(pWidget, 1, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) != XUI_OK,
			"recursive activation blocked");
	}
	if (strstr(s->sCase, "action_destroy_context") != NULL) {
		xuiDestroy(s->pContext);
		s->bDestroyed = 1;
	}
}

static void text_changed(xui_widget pWidget, const char* sText, void* pUser)
{
	(void)sText;
	clicked(pWidget, pUser);
}

static void number_changed(xui_widget pWidget, float fValue, void* pUser)
{
	(void)fValue;
	clicked(pWidget, pUser);
}

static void combo_selected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	(void)iIndex;
	(void)iValue;
	clicked(pWidget, pUser);
}

static void popup_changed(xui_widget pWidget, int bOpen, void* pUser)
{
	if (!bOpen) clicked(pWidget, pUser);
}

static int batch_bounds(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	(void)pUser;
	if (pEvent->iType != XUI_EVENT_COMMAND) return XUI_OK;
	(void)xuiWidgetSetRect(pWidget, (xui_rect_t){0, 0, 80, 20});
	(void)xuiWidgetSetRect(pWidget, (xui_rect_t){10, 10, 90, 30});
	if (strcmp(((test_t*)pUser)->sCase, "effective_state_notify") == 0) {
		(void)xuiWidgetSetVisible(pWidget, 0);
		(void)xuiWidgetSetEnabled(pWidget, 0);
	}
	return XUI_OK;
}

static int provider_count(xui_widget pWidget, void* pUser)
{
	test_t* s = (test_t*)pUser;
	if (strcmp(s->sCase, "provider_count_reentry") == 0)
		CHECK(s, xuiWidgetGetAccessibleNodeCount(pWidget) == 0, "recursive provider count blocked");
	if (strcmp(s->sCase, "provider_count_destroy_context") == 0) {
		xuiDestroy(s->pContext);
		s->bDestroyed = 1;
	}
	return 1;
}

static int provider_get(xui_widget pWidget, int iIndex, xui_accessible_node_t* pNode, void* pUser)
{
	test_t* s = (test_t*)pUser;
	(void)pWidget;
	(void)iIndex;
	pNode->iId = 77;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_HEADING;
	pNode->sName = "Custom provider";
	pNode->sValue = "Custom value";
	if (strcmp(s->sCase, "provider_get_reentry") == 0) {
		xui_accessible_node_t other = {0};
		other.iSize = sizeof(other);
		CHECK(s, xuiWidgetGetAccessibleNode(pWidget, 0, &other) != XUI_OK && other.sName == NULL,
			"recursive provider query blocked");
	}
	if (strcmp(s->sCase, "provider_replace") == 0)
		(void)xuiWidgetSetAccessibilityProvider(pWidget, NULL, NULL, NULL, NULL);
	if (strcmp(s->sCase, "provider_mutate") == 0) {
		pNode->sName = xuiButtonGetText(pWidget);
		(void)xuiButtonSetText(pWidget, "Changed during query");
	}
	if (strcmp(s->sCase, "provider_mutate_explicit") == 0) {
		pNode->sName = xuiWidgetGetAccessibleName(pWidget);
		(void)xuiWidgetSetAccessibleName(pWidget, "Changed during notification");
	}
	if (strcmp(s->sCase, "provider_widget_destroy") == 0) xuiWidgetDestroy(pWidget);
	if (strcmp(s->sCase, "provider_destroy_context") == 0) {
		xuiDestroy(s->pContext);
		s->bDestroyed = 1;
	}
	if (strcmp(s->sCase, "provider_fail") == 0) return XUI_ERROR_UNSUPPORTED;
	return XUI_OK;
}

static int provider_action(xui_widget pWidget, uint64_t iId, int iAction, const void* pData, void* pUser)
{
	test_t* s = (test_t*)pUser;
	(void)pWidget;
	(void)iId;
	(void)iAction;
	(void)pData;
	s->iActions++;
	if (strcmp(s->sCase, "provider_action_reentry") == 0)
		CHECK(s, xuiWidgetPerformAccessibleAction(pWidget, 77, iAction, pData) != XUI_OK,
			"recursive provider action blocked");
	if (strcmp(s->sCase, "provider_action_destroy_context") == 0) {
		xuiDestroy(s->pContext);
		s->bDestroyed = 1;
	}
	return XUI_OK;
}

static void accessibility_event(xui_context pContext, xui_widget pWidget,
	const xui_accessibility_event_t* pEvent, void* pUser)
{
	test_t* s = (test_t*)pUser;
	(void)pContext;
	if (strcmp(s->sCase, "notify_ping_pong") == 0 && pEvent->iType == XUI_ACCESSIBLE_EVENT_NODE_CHANGED) {
		s->iCallbacks++;
		if (s->iCallbacks < 10)
			(void)xuiButtonSetText(pWidget == s->pWidget ? s->pOther : s->pWidget,
				(s->iCallbacks & 2) ? "Even" : "Odd");
		return;
	}
	if (pWidget != s->pWidget) return;
	if (strcmp(s->sCase, "provider_mutate_explicit") == 0 && pEvent->iType == XUI_ACCESSIBLE_EVENT_NODE_CHANGED) {
		xui_accessible_node_t node = {0};
		node.iSize = sizeof(node);
		s->iCallbacks++;
		CHECK(s, xuiWidgetGetAccessibleNode(pWidget, 0, &node) != XUI_OK && node.sName == NULL,
			"mutation during suppressed explicit notification invalidates borrowed query result");
	}
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_FOCUS_CHANGED) s->iFocusEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_NODE_CHANGED) s->iNodeEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED) s->iBoundsEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_STATE_CHANGED) s->iStateEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_VALUE_CHANGED) s->iValueEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED) s->iSelectionEvents++;
	if (pEvent->iType == XUI_ACCESSIBLE_EVENT_TREE_CHANGED) s->iTreeEvents++;
	if (strcmp(s->sCase, "notify_destroy_context") == 0 ||
		strcmp(s->sCase, "focus_destroy_context") == 0 ||
		strcmp(s->sCase, "state_destroy_context") == 0 ||
		strcmp(s->sCase, "window_state_destroy_context") == 0 ||
		strcmp(s->sCase, "combobox_state_destroy_context") == 0 ||
		strcmp(s->sCase, "query_drain_destroy_context") == 0) {
		CHECK(s, !s->bInsideSetter, "automatic notification cannot run inside a control setter");
		if (s->bInsideSetter) return;
		xuiDestroy(s->pContext);
		s->bDestroyed = 1;
		return;
	}
	if (strcmp(s->sCase, "notify_reentry") == 0 && s->iCallbacks++ == 0) {
		(void)xuiWidgetNotifyAccessibility(pWidget, pEvent->iType, pEvent->iNodeId);
	}
	if (strcmp(s->sCase, "notify_widget_destroy") == 0) {
		xuiWidgetDestroy(pWidget);
		s->pWidget = NULL;
	}
	if (strcmp(s->sCase, "notify_update_reentry") == 0 && pEvent->iType == XUI_ACCESSIBLE_EVENT_NODE_CHANGED) {
		(void)xuiButtonSetText(pWidget, "Reentered");
		(void)xuiUpdate(pContext, 0);
	}
}

static int run_case(const char* sName)
{
	test_t s;
	xui_accessible_node_t node;
	int iRet;
	memset(&s, 0, sizeof(s));
	s.sCase = sName;
	xuiTestProxyInit(&s.tProxy);
	if (xuiCreate(&s.pContext) != XUI_OK) return 1;
	(void)xuiSetProxy(s.pContext, &s.tProxy.tProxy);
	(void)xuiSetViewportSize(s.pContext, 600, 400);
	(void)xuiWidgetCreate(s.pContext, &s.pRoot);
	(void)xuiSetRootWidget(s.pContext, s.pRoot);
	if (is_family(sName, "widget")) (void)xuiWidgetCreate(s.pContext, &s.pWidget);
	else if (is_family(sName, "toggle")) {
		(void)xuiToggleCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiToggleSetText(s.pWidget, "Power");
		(void)xuiToggleSetChecked(s.pWidget, 1);
	} else if (is_family(sName, "checkbox")) {
		(void)xuiCheckBoxCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiCheckBoxSetText(s.pWidget, "Remember");
		(void)xuiCheckBoxSetChecked(s.pWidget, 1);
	} else if (is_family(sName, "radio")) {
		(void)xuiRadioCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiRadioSetText(s.pWidget, "Choice");
		(void)xuiRadioSetChecked(s.pWidget, 1);
	} else if (is_family(sName, "input") || strcmp(sName, "password") == 0) {
		(void)xuiInputCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiInputSetText(s.pWidget, "Sample text");
		(void)xuiInputSetPlaceholder(s.pWidget, "Account");
		if (strcmp(sName, "password") == 0) (void)xuiInputSetPassword(s.pWidget, 1);
		(void)xuiInputSetChange(s.pWidget, text_changed, &s);
	} else if (is_family(sName, "textedit")) {
		(void)xuiTextEditCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiTextEditSetText(s.pWidget, "First\nSecond");
		(void)xuiTextEditSetChange(s.pWidget, text_changed, &s);
	} else if (strcmp(sName, "label") == 0) {
		(void)xuiLabelCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiLabelSetText(s.pWidget, "Status");
	} else if (is_family(sName, "scrollbar")) {
		(void)xuiScrollBarCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiScrollBarSetRange(s.pWidget, 0, 100, 20);
		(void)xuiScrollBarSetSteps(s.pWidget, 2, 10);
		(void)xuiScrollBarSetValue(s.pWidget, 30);
		(void)xuiScrollBarSetChange(s.pWidget, number_changed, &s);
	} else if (is_family(sName, "slider")) {
		(void)xuiSliderCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiSliderSetRange(s.pWidget, -10, 10);
		(void)xuiSliderSetValue(s.pWidget, 3);
		(void)xuiSliderSetStep(s.pWidget, 2, 4);
		(void)xuiSliderSetChange(s.pWidget, number_changed, &s);
	} else if (is_family(sName, "combobox")) {
		const char* items[] = {"First", "Second"};
		(void)xuiComboBoxCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiComboBoxSetItems(s.pWidget, items, 2);
		(void)xuiComboBoxSetSelected(s.pWidget, 1);
		(void)xuiComboBoxSetSelect(s.pWidget, combo_selected, &s);
		if (strstr(sName, "_edit") != NULL) (void)xuiComboBoxSetMode(s.pWidget, XUI_COMBOBOX_MODE_EDIT);
	} else if (is_family(sName, "window")) {
		(void)xuiWindowCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiWindowSetTitle(s.pWidget, "Preferences");
		(void)xuiWindowSetClose(s.pWidget, clicked, &s);
	} else if (is_family(sName, "popup")) {
		(void)xuiPopupCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiPopupSetModal(s.pWidget, 1);
		(void)xuiPopupSetChange(s.pWidget, popup_changed, &s);
	} else {
		(void)xuiButtonCreate(s.pContext, &s.pWidget, NULL);
		(void)xuiButtonSetText(s.pWidget, "Apply");
		(void)xuiButtonSetClick(s.pWidget, clicked, &s);
	}
	CHECK(&s, s.pWidget != NULL, "create control");
	if (s.pWidget == NULL) goto cleanup;
	(void)xuiWidgetAddChild(s.pRoot, s.pWidget);
	if (strncmp(sName, "provider_", 9) == 0) {
		if (strcmp(sName, "provider_mutate_explicit") == 0) (void)xuiWidgetSetAccessibleName(s.pWidget, "Original");
		(void)xuiWidgetSetAccessibilityProvider(s.pWidget, provider_count, provider_get, provider_action, &s);
	}
	(void)xuiUpdate(s.pContext, 0);
	(void)xuiSetAccessibilityEventCallback(s.pContext, accessibility_event, &s);
	memset(&node, 0, sizeof(node));
	node.iSize = sizeof(node);
	if (strcmp(sName, "provider_count_destroy_context") == 0) {
		CHECK(&s, xuiWidgetGetAccessibleNodeCount(s.pWidget) == 0, "destroyed count provider returns zero");
	} else if (strcmp(sName, "provider_mutate_explicit") == 0) {
		(void)xuiWidgetNotifyAccessibility(s.pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED, 1);
		CHECK(&s, s.iCallbacks == 1, "explicit provider mutation notification is bounded");
	} else if (strcmp(sName, "button_selected_notify") == 0) {
		(void)xuiButtonSetSelected(s.pWidget, 1);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iStateEvents == 1 && s.iValueEvents == 1, "selectable button value and state notifications");
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK,
			"selectable button toggles");
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iStateEvents == 2 && s.iValueEvents == 2, "toggle publishes changed button value and state");
	} else if (strcmp(sName, "scrollbar_orientation_notify") == 0) {
		(void)xuiScrollBarSetOrientation(s.pWidget, XUI_ORIENTATION_HORIZONTAL);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iStateEvents == 1 && xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK &&
			!(node.iState & XUI_ACCESSIBLE_STATE_VERTICAL), "scrollbar orientation semantics and notification");
	} else if (strcmp(sName, "window_close_metadata") == 0) {
		(void)xuiWindowSetShowClose(s.pWidget, 0);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iNodeEvents == 1 && xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK &&
			!(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_CLOSE)), "window close action availability notification");
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_CLOSE, NULL) != XUI_OK,
			"hidden window close command cannot be invoked");
	} else if (strcmp(sName, "provider_disabled") == 0 || strcmp(sName, "provider_hidden") == 0) {
		if (strcmp(sName, "provider_disabled") == 0) (void)xuiWidgetSetEnabled(s.pRoot, 0);
		else (void)xuiWidgetSetVisible(s.pRoot, 0);
		iRet = xuiWidgetPerformAccessibleAction(s.pWidget, 77, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
		CHECK(&s, iRet != XUI_OK && s.iActions == 0, "effective state blocks custom action");
		CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK && node.iActions == 0,
			"unavailable provider advertises no actions");
	} else if (strcmp(sName, "action_disabled") == 0 || strcmp(sName, "action_hidden") == 0) {
		if (strcmp(sName, "action_disabled") == 0) (void)xuiWidgetSetEnabled(s.pRoot, 0);
		else (void)xuiWidgetSetVisible(s.pRoot, 0);
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) != XUI_OK &&
			s.iActions == 0, "effective ancestor state blocks default action");
	} else if (is_family(sName, "provider_action")) {
		int action = strcmp(sName, "provider_action_extension") == 0 ? 1009 : XUI_ACCESSIBLE_ACTION_ACTIVATE;
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 77, action, NULL) == XUI_OK,
			"custom action dispatched");
		CHECK(&s, s.iActions == 1, "custom action runs once");
	} else if (is_family(sName, "action")) {
		iRet = xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
		CHECK(&s, iRet == XUI_OK && s.iActions == 1, "built-in activation invokes click");
	} else if (strcmp(sName, "focus_destroy_context") == 0) {
		(void)xuiSetFocusWidget(s.pContext, s.pWidget);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.bDestroyed && s.iFocusEvents == 1, "focus callback may destroy context");
	} else if (strcmp(sName, "state_destroy_context") == 0) {
		(void)xuiWidgetSetEnabled(s.pRoot, 0);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.bDestroyed && s.iStateEvents == 1, "state callback may destroy context");
	} else if (strcmp(sName, "window_state_destroy_context") == 0 ||
		strcmp(sName, "combobox_state_destroy_context") == 0) {
		s.bInsideSetter = 1;
		if (is_family(sName, "window")) (void)xuiWindowSetOpen(s.pWidget, 0);
		else (void)xuiComboBoxSetMode(s.pWidget, XUI_COMBOBOX_MODE_EDIT);
		s.bInsideSetter = 0;
		CHECK(&s, !s.bDestroyed && s.iStateEvents == 0 && s.iNodeEvents == 0,
			"control setter completes before automatic notification");
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.bDestroyed, "automatic callback destroys context at update barrier");
	} else if (strcmp(sName, "focus_notify") == 0) {
		(void)xuiSetFocusWidget(s.pContext, s.pWidget);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iFocusEvents == 1, "focus gain notification");
		(void)xuiWidgetSetVisible(s.pRoot, 0);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iFocusEvents == 2, "focus cancellation notification");
	} else if (strcmp(sName, "state_notify") == 0 || strcmp(sName, "effective_state_notify") == 0) {
		xui_event_t event = {0};
		event.iSize = sizeof(event);
		event.iType = XUI_EVENT_COMMAND;
		event.pTarget = s.pWidget;
		(void)xuiWidgetSetEventHandler(s.pWidget, XUI_EVENT_COMMAND, batch_bounds, &s);
		(void)xuiDispatchEvent(s.pContext, &event);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iBoundsEvents == 1, "coalesced bounds notification");
		if (strcmp(sName, "effective_state_notify") == 0) {
			CHECK(&s, s.iStateEvents == 1, "visible and enabled changes share one state notification");
			CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK &&
				(node.iState & (XUI_ACCESSIBLE_STATE_DISABLED | XUI_ACCESSIBLE_STATE_OFFSCREEN)) ==
				(XUI_ACCESSIBLE_STATE_DISABLED | XUI_ACCESSIBLE_STATE_OFFSCREEN), "query observes merged effective state");
		}
	} else if (strcmp(sName, "notify_reentry") == 0) {
		(void)xuiWidgetNotifyAccessibility(s.pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED, 1);
		CHECK(&s, s.iCallbacks == 1, "same notification reentry is suppressed");
	} else if (strcmp(sName, "notify_destroy_context") == 0) {
		(void)xuiWidgetNotifyAccessibility(s.pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED, 1);
		CHECK(&s, s.bDestroyed, "explicit notification may destroy context");
	} else if (strcmp(sName, "query_drain_destroy_context") == 0) {
		(void)xuiButtonSetText(s.pWidget, "Changed");
		CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK && node.sName &&
			strcmp(node.sName, "Changed") == 0, "query observes pending semantics without publishing notifications");
		CHECK(&s, !s.bDestroyed, "query cannot publish automatic callbacks");
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.bDestroyed, "queued callback runs at update barrier");
	} else if (strcmp(sName, "notify_widget_destroy") == 0 || strcmp(sName, "notify_update_reentry") == 0) {
		(void)xuiButtonSetText(s.pWidget, "Changed");
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iNodeEvents == 1, "automatic notification tolerates widget destruction or nested update");
		if (strcmp(sName, "notify_widget_destroy") == 0) CHECK(&s, s.pWidget == NULL, "widget destroyed in notification");
	} else if (strcmp(sName, "notify_queued_destroy") == 0) {
		(void)xuiButtonCreate(s.pContext, &s.pOther, NULL);
		(void)xuiWidgetAddChild(s.pRoot, s.pOther);
		(void)xuiButtonSetText(s.pOther, "Removed");
		(void)xuiButtonSetText(s.pWidget, "Kept");
		xuiWidgetDestroy(s.pOther);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iNodeEvents == 1, "destroyed queued widget is unlinked without losing live events");
	} else if (strcmp(sName, "notify_ping_pong") == 0) {
		(void)xuiButtonCreate(s.pContext, &s.pOther, NULL);
		(void)xuiWidgetAddChild(s.pRoot, s.pOther);
		(void)xuiButtonSetText(s.pWidget, "Start");
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iCallbacks == 2, "cross-widget queued notification loop is bounded");
	} else if (strcmp(sName, "widget_tree_notify") == 0) {
		(void)xuiWidgetCreate(s.pContext, &s.pOther);
		(void)xuiWidgetAddChild(s.pWidget, s.pOther);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iTreeEvents == 1, "child attach emits tree notification");
		CHECK(&s, xuiWidgetGetFirstChild(s.pWidget) == s.pOther &&
			xuiWidgetGetAccessibleNodeCount(s.pOther) == 1, "physical child node is queryable");
		(void)xuiWidgetRemoveFromParent(s.pOther);
		(void)xuiUpdate(s.pContext, 0);
		CHECK(&s, s.iTreeEvents == 2, "child detach emits tree notification");
		xuiWidgetDestroy(s.pOther);
	} else if (strcmp(sName, "modal_action") == 0) {
		(void)xuiPopupCreate(s.pContext, &s.pOther, NULL);
		(void)xuiPopupSetModal(s.pOther, 1);
		(void)xuiPopupSetOpen(s.pOther, 1);
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_FOCUS, NULL) != XUI_OK,
			"focus action respects modal trap");
		CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) != XUI_OK && s.iActions == 0,
			"activation cannot bypass modal scope");
	} else if (strcmp(sName, "legacy_node_size") == 0) {
		xui_accessible_node_t legacy;
		unsigned char* bytes = (unsigned char*)&legacy;
		size_t oldSize = offsetof(xui_accessible_node_t, tRange);
		size_t i;
		memset(&legacy, 0xa5, sizeof(legacy));
		legacy.iSize = (uint32_t)oldSize;
		CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &legacy) == XUI_OK && strcmp(legacy.sName, "Apply") == 0,
			"legacy node prefix remains queryable");
		CHECK(&s, legacy.iSize == oldSize && xuiWidgetGetAccessibleNode(s.pWidget, 0, &legacy) == XUI_OK,
			"reusing a legacy node cannot enlarge its output buffer");
		CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 99, &legacy) != XUI_OK && legacy.iSize == oldSize && legacy.sName == NULL,
			"failed query preserves legacy capacity and clears semantic output");
		CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &legacy) == XUI_OK, "legacy buffer can be retried after failure");
		for (i = oldSize; i < sizeof(legacy); i++) CHECK(&s, bytes[i] == 0xa5, "query does not overwrite legacy buffer tail");
	} else {
		CHECK(&s, xuiWidgetGetAccessibleNodeCount(s.pWidget) == 1, "default node exists without custom name");
		iRet = xuiWidgetGetAccessibleNode(s.pWidget, 0, &node);
		if (strcmp(sName, "provider_destroy_context") == 0 || strcmp(sName, "provider_widget_destroy") == 0 ||
			strcmp(sName, "provider_replace") == 0 || strcmp(sName, "provider_fail") == 0 ||
			strcmp(sName, "provider_mutate") == 0) {
			CHECK(&s, iRet != XUI_OK && node.sName == NULL, "destroyed provider result is discarded");
			goto cleanup;
		}
		CHECK(&s, iRet == XUI_OK, "query control node");
		if (strcmp(sName, "widget_tree") == 0) {
			CHECK(&s, xuiWidgetGetAccessibleNodeCount(s.pRoot) == 1, "root is queryable");
		} else CHECK(&s, node.iRole != XUI_ACCESSIBLE_ROLE_GENERIC, "built-in semantic role");
		if (strcmp(sName, "button") == 0) CHECK(&s, node.sName && strcmp(node.sName, "Apply") == 0, "button text supplies name");
		if (strcmp(sName, "toggle") == 0 || strcmp(sName, "checkbox") == 0 || strcmp(sName, "radio") == 0)
			CHECK(&s, (node.iState & XUI_ACCESSIBLE_STATE_CHECKED) != 0, "checked state");
		if (strcmp(sName, "input") == 0) CHECK(&s, node.sValue && strcmp(node.sValue, "Sample text") == 0, "input value");
		if (strcmp(sName, "password") == 0) CHECK(&s, node.sValue == NULL || node.sValue[0] == 0, "password value not exposed");
		if (strcmp(sName, "provider_priority") == 0) CHECK(&s, node.iId == 77 && strcmp(node.sName, "Custom provider") == 0, "custom provider takes priority");
		if (strcmp(sName, "label") == 0) CHECK(&s, node.sName && strcmp(node.sName, "Status") == 0, "label default name");
		if (is_family(sName, "scrollbar")) {
			xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_NUMBER, NULL, 40};
			CHECK(&s, node.tRange.fMin == 0 && node.tRange.fMax == 100 && node.tRange.fValue == 30 &&
				node.tRange.fStep == 2 && node.tRange.fPageStep == 10, "scrollbar range and steps");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) == XUI_OK,
				"scrollbar value action");
			CHECK(&s, s.iActions == 1, "scrollbar value callback");
			if (s.bDestroyed) goto cleanup;
			(void)xuiUpdate(s.pContext, 0);
			CHECK(&s, xuiScrollBarGetValue(s.pWidget) == 40 && s.iValueEvents == 1, "scrollbar value notification");
		}
		if (strcmp(sName, "button") == 0) {
			CHECK(&s, (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)) != 0, "button action metadata");
			(void)xuiWidgetSetAccessibleName(s.pWidget, "Explicit label");
			node.iSize = sizeof(node);
			CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK && strcmp(node.sName, "Explicit label") == 0,
				"explicit name overrides built-in fallback");
		}
		if (strcmp(sName, "toggle") == 0 || strcmp(sName, "checkbox") == 0 || strcmp(sName, "radio") == 0) {
			int action = strcmp(sName, "radio") == 0 ? XUI_ACCESSIBLE_ACTION_ACTIVATE : XUI_ACCESSIBLE_ACTION_TOGGLE;
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, action, NULL) == XUI_OK, "checkable default action");
			node.iSize = sizeof(node);
			CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK, "updated checkable node");
			CHECK(&s, !!(node.iState & XUI_ACCESSIBLE_STATE_CHECKED) == (action == XUI_ACCESSIBLE_ACTION_ACTIVATE),
				"radio stays selected; checkbox and toggle invert");
		}
		if (is_family(sName, "slider")) {
			xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_NUMBER, NULL, 5};
			CHECK(&s, node.tRange.iSize == sizeof(node.tRange) && node.tRange.fMin == -10 && node.tRange.fMax == 10 &&
				node.tRange.fValue == 3 && node.tRange.fStep == 2 && node.tRange.fPageStep == 4, "slider numeric range");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) == XUI_OK,
				"slider set value action");
			CHECK(&s, s.iActions == 1, "slider action invokes change callback");
			if (s.bDestroyed) goto cleanup;
			(void)xuiUpdate(s.pContext, 0);
			CHECK(&s, xuiSliderGetValue(s.pWidget) == 5 && s.iValueEvents == 1, "slider changed value and notification");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_INCREMENT, NULL) == XUI_OK &&
				xuiSliderGetValue(s.pWidget) == 7, "slider increment uses configured step");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_DECREMENT, NULL) == XUI_OK &&
				xuiSliderGetValue(s.pWidget) == 5, "slider decrement uses configured step");
			value.fNumber = NAN;
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) != XUI_OK,
				"NaN value rejected");
			value.fNumber = 11;
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) != XUI_OK,
				"out of range value rejected");
		}
		if (is_family(sName, "input") || is_family(sName, "textedit")) {
			xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_TEXT, "Edited", 0};
			xui_accessible_selection_t selection = {sizeof(selection), 1, 3};
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) == XUI_OK,
				"editor set value action");
			CHECK(&s, s.iActions == 1, "editor action invokes text change callback");
			if (s.bDestroyed) goto cleanup;
			(void)xuiUpdate(s.pContext, 0);
			CHECK(&s, strcmp(xuiEditGetText(s.pWidget), "Edited") == 0 && s.iValueEvents == 1, "editor value notification");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK,
				"editor selection action");
			(void)xuiEditSetReadonly(s.pWidget, 1);
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) != XUI_OK,
				"readonly editor rejects value action");
			node.iSize = sizeof(node);
			CHECK(&s, xuiWidgetGetAccessibleNode(s.pWidget, 0, &node) == XUI_OK &&
				(node.iState & XUI_ACCESSIBLE_STATE_READONLY) && !(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE)),
				"readonly editor metadata matches action policy");
		}
		if (is_family(sName, "combobox")) {
			xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_NUMBER, NULL, 0};
			CHECK(&s, node.sValue && strcmp(node.sValue, "Second") == 0, "combobox selected text");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) == XUI_OK,
				"combobox selection action");
			CHECK(&s, s.iActions == 1, "combobox action invokes selection callback");
			if (s.bDestroyed) goto cleanup;
			(void)xuiUpdate(s.pContext, 0);
			CHECK(&s, xuiComboBoxGetSelected(s.pWidget) == 0 && s.iValueEvents >= 1, "combobox selection updated");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_EXPAND, NULL) == XUI_OK &&
				xuiComboBoxIsOpen(s.pWidget), "combobox expands");
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL) == XUI_OK &&
				!xuiComboBoxIsOpen(s.pWidget), "combobox collapses");
			if (strcmp(sName, "combobox_edit") == 0) {
				value.iType = XUI_ACCESSIBLE_VALUE_TEXT;
				value.sText = "Typed";
				CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) == XUI_OK &&
					strcmp(xuiComboBoxGetText(s.pWidget), "Typed") == 0, "editable combobox delegates text action");
				CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_FOCUS, NULL) == XUI_OK &&
					xuiGetFocusWidget(s.pContext) == xuiComboBoxGetInputWidget(s.pWidget), "editable combobox delegates focus action");
				(void)xuiWidgetSetEnabled(s.pWidget, 0);
				CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value) != XUI_OK,
					"disabled combo and edit delegate block text action");
			}
		}
		if (is_family(sName, "window") || is_family(sName, "popup")) {
			if (is_family(sName, "popup")) (void)xuiPopupSetOpen(s.pWidget, 1);
			CHECK(&s, xuiWidgetPerformAccessibleAction(s.pWidget, 1, XUI_ACCESSIBLE_ACTION_CLOSE, NULL) == XUI_OK,
				"window and popup close action");
			CHECK(&s, s.iActions == 1, "close action invokes control callback");
		}
	}
cleanup:
	if (!s.bDestroyed) xuiDestroy(s.pContext);
	if (!s.iFailures) printf("PASS %s\n", sName);
	return s.iFailures != 0;
}

int main(int argc, char** argv)
{
	static const char* const cases[] = {
		"widget_tree", "button", "toggle", "checkbox", "radio", "input", "password", "textedit",
		"slider", "combobox", "window", "popup", "action", "action_destroy_context", "focus_notify",
		"state_notify", "notify_reentry", "provider_priority", "provider_disabled", "provider_hidden",
		"provider_destroy_context", "widget_tree_notify", "effective_state_notify", "action_reentry",
		"provider_count_reentry", "provider_get_reentry", "provider_action_reentry", "provider_replace",
		"provider_fail", "provider_widget_destroy", "provider_count_destroy_context", "provider_action_destroy_context",
		"notify_destroy_context", "focus_destroy_context", "state_destroy_context", "query_drain_destroy_context",
		"notify_ping_pong", "input_action_destroy_context", "textedit_action_destroy_context",
		"slider_action_destroy_context", "combobox_action_destroy_context", "combobox_edit",
		"window_action_destroy_context", "popup_action_destroy_context",
		"label", "scrollbar", "scrollbar_action_destroy_context", "legacy_node_size", "provider_action_extension",
		"action_disabled", "action_hidden", "modal_action", "provider_mutate",
		"window_state_destroy_context", "combobox_state_destroy_context",
		"notify_widget_destroy", "notify_queued_destroy", "notify_update_reentry", "provider_mutate_explicit",
		"button_selected_notify", "scrollbar_orientation_notify", "window_close_metadata"
	};
	size_t i;
	int count = 0, failed = 0;
	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		if (argc > 1 && strcmp(argv[1], cases[i]) != 0) continue;
		count++;
		failed += run_case(cases[i]);
	}
	printf("accessibility: %d passed, %d failed\n", count - failed, failed);
	return failed != 0 || count == 0;
}
