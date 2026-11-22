#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_accessibility_text_internal.h"
#include "xui_native_accessibility_internal.h"

#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define XUI_A11Y_BIT(type) (1u << ((type) - 1))

typedef struct xui_accessible_provider_state_t {
	xui_accessible_node_owned_proc onNode;
	xui_accessible_action_owned_proc onAction;
	char* sNodeTexts;
} xui_accessible_provider_state_t;

struct xui_accessible_node_output_t {
	xui_accessible_node_t* pNode;
	char* sTexts;
	bool bSet;
};

static void __xuiAccessibleProviderFree(xui_accessible_provider_state_t* pState)
{
	if ( pState == NULL ) return;
	xrtFree(pState->sNodeTexts);
	xrtFree(pState);
}

void xuiInternalAccessibilityClearOwnedProvider(xui_widget pWidget)
{
	__xuiAccessibleProviderFree(pWidget->pAccessibleOwned);
	pWidget->pAccessibleOwned = NULL;
}

static int __xuiAccessibleValid(xui_widget pWidget)
{
	return pWidget != NULL && pWidget->pContext != NULL && xuiInternalWidgetIsValid(pWidget) &&
		!xuiInternalContextDestroyPending(pWidget->pContext) && !pWidget->pContext->bDestroying;
}

static int __xuiAccessibleCanAct(xui_widget pWidget)
{
	xui_widget pScan;
	xui_widget pSlow = pWidget, pFast = pWidget, pModal;
	if ( !__xuiAccessibleValid(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ||
		!xuiWidgetGetEffectiveEnabled(pWidget) ) return 0;
	for ( pScan = pWidget; pScan != NULL; pScan = xuiInternalInputFocusParent(pScan) ) {
		if ( !__xuiAccessibleValid(pScan) || pScan->bInteractionCancelling ||
			!xuiWidgetGetEffectiveVisible(pScan) || !xuiWidgetGetEffectiveEnabled(pScan) ) return 0;
		pSlow = xuiInternalInputFocusParent(pSlow);
		pFast = xuiInternalInputFocusParent(xuiInternalInputFocusParent(pFast));
		if ( pSlow != NULL && pSlow == pFast ) return 0;
	}
	pModal = xuiInternalInputModalRoot(pWidget->pContext);
	return pModal == NULL || xuiInternalInputFocusContains(pModal, pWidget);
}

static const xui_internal_accessibility_adapter_t* __xuiAccessibleAdapter(xui_widget pWidget)
{
	xui_widget_type pType;
	for ( pType = pWidget->pType; pType != NULL; pType = pType->pParent ) {
		if ( pType->pAccessibleAdapter != NULL ) return pType->pAccessibleAdapter;
	}
	return NULL;
}

static xui_widget __xuiAccessibleEditTarget(xui_widget pWidget)
{
	int iDepth;
	for ( iDepth = 0; iDepth < 8 && __xuiAccessibleValid(pWidget); iDepth++ ) {
		if ( pWidget->pEditDelegate == NULL ) return pWidget;
		pWidget = pWidget->pEditDelegate;
	}
	return NULL;
}

static int __xuiAccessibleRole(xui_widget pWidget)
{
	static const struct { const char* sType; int iRole; } roles[] = {
		{"label", XUI_ACCESSIBLE_ROLE_TEXT}, {"hyperlink", XUI_ACCESSIBLE_ROLE_LINK},
		{"image", XUI_ACCESSIBLE_ROLE_IMAGE}, {"separator", XUI_ACCESSIBLE_ROLE_SEPARATOR},
		{"webview", XUI_ACCESSIBLE_ROLE_EMBEDDED_OBJECT},
		{"panel", XUI_ACCESSIBLE_ROLE_GROUP}, {"radiogroup", XUI_ACCESSIBLE_ROLE_RADIO_GROUP},
		{"listview", XUI_ACCESSIBLE_ROLE_LIST}, {"messagelist", XUI_ACCESSIBLE_ROLE_LIST},
		{"inventorygrid", XUI_ACCESSIBLE_ROLE_LIST},
		{"treeview", XUI_ACCESSIBLE_ROLE_TREE}, {"menu", XUI_ACCESSIBLE_ROLE_MENU},
		{"menubar", XUI_ACCESSIBLE_ROLE_MENU}, {"toolbar", XUI_ACCESSIBLE_ROLE_TOOLBAR},
		{"tableview", XUI_ACCESSIBLE_ROLE_TABLE}, {"tablegrid", XUI_ACCESSIBLE_ROLE_TABLE},
		{"propertygrid", XUI_ACCESSIBLE_ROLE_TABLE}, {"tabs", XUI_ACCESSIBLE_ROLE_TAB_LIST},
		{"scrollbar", XUI_ACCESSIBLE_ROLE_SCROLLBAR}, {"progress", XUI_ACCESSIBLE_ROLE_PROGRESS}
	};
	xui_widget_type pType;
	size_t i;
	if ( pWidget == pWidget->pContext->pRoot ) return XUI_ACCESSIBLE_ROLE_ROOT;
	for ( pType = pWidget->pType; pType != NULL; pType = pType->pParent ) {
		for ( i = 0; i < sizeof(roles) / sizeof(roles[0]); i++ ) {
			if ( strcmp(pType->sName, roles[i].sType) == 0 ) return roles[i].iRole;
		}
	}
	return XUI_ACCESSIBLE_ROLE_GENERIC;
}

/* Drain before publishing borrowed strings, while destruction is still deferred.
 * A provider that mutates during a query cannot return a pre-mutation snapshot. */
static void __xuiAccessibleDrain(xui_context pContext)
{
	if ( !xuiInternalContextDestroyPending(pContext) && pContext->iOperationDepth == 1 &&
		!pContext->bAccessibilityFlushing ) xuiInternalStateChangeFlush(pContext);
}

static bool __xuiAccessibleHasNodeProvider(xui_widget widget)
{
	return widget->onAccessibleGet || (widget->pAccessibleOwned && widget->pAccessibleOwned->onNode);
}

static bool __xuiAccessibleHasActionProvider(xui_widget widget)
{
	return widget->onAccessibleAction || (widget->pAccessibleOwned && widget->pAccessibleOwned->onAction);
}

static int __xuiAccessiblePublishProvider(xui_widget pWidget,
	xui_accessible_count_proc onCount, xui_accessible_get_proc onGet,
	xui_accessible_action_proc onAction, void* pUser,
	xui_accessible_provider_state_t* pState, const xui_callback_owner_t* pOwner)
{
	xui_callback_update_t update = {0};
	if ( pOwner || xuiInternalCallbackSlotOwned(pWidget, &pWidget->pAccessibleUser) ) {
		int status = xuiInternalCallbackUpdateBegin(pWidget, &pWidget->pAccessibleUser,
			onCount != NULL || onAction != NULL || pState != NULL, pUser, pOwner, &update);
		if ( status != XUI_OK ) {
			__xuiAccessibleProviderFree(pState);
			return status;
		}
	}
	xui_accessible_provider_state_t* previous = pWidget->pAccessibleOwned;
	pWidget->pAccessibleOwned = pState;
	pWidget->onAccessibleCount = onCount;
	pWidget->onAccessibleGet = onGet;
	pWidget->onAccessibleAction = onAction;
	pWidget->onAccessibleFocus = NULL;
	pWidget->pAccessibleUser = (onCount != NULL || onAction != NULL || pState != NULL) ? pUser : NULL;
	pWidget->iAccessibleProviderGeneration++;
	if ( update.pContext ) xuiInternalCallbackUpdatePublish(&update);
	__xuiAccessibleProviderFree(previous);
	int status = xuiWidgetNotifyAccessibility(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED, 0);
	return update.pContext ? xuiInternalCallbackUpdateEnd(&update, status) : status;
}

XUI_API int xuiWidgetSetAccessibilityProvider(xui_widget pWidget,
	xui_accessible_count_proc onCount, xui_accessible_get_proc onGet,
	xui_accessible_action_proc onAction, void* pUser)
{
	if ( !__xuiAccessibleValid(pWidget) || ((onCount == NULL) != (onGet == NULL)) )
		return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiAccessiblePublishProvider(pWidget, onCount, onGet, onAction, pUser, NULL, NULL);
}

XUI_API int xuiWidgetSetAccessibilityProviderOwned(xui_widget pWidget,
	const xui_accessible_provider_desc_t* pDesc, const xui_callback_owner_t* pOwner)
{
	if ( !__xuiAccessibleValid(pWidget) ||
	     (pDesc && (pDesc->iSize < sizeof(*pDesc) || (pDesc->onCount == NULL) != (pDesc->onNode == NULL))) )
		return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pDesc || (!pDesc->onNode && !pDesc->onAction) ) {
		if ( pOwner || (pDesc && pDesc->pUser) ) return XUI_ERROR_INVALID_ARGUMENT;
		return __xuiAccessiblePublishProvider(pWidget, NULL, NULL, NULL, NULL, NULL, NULL);
	}
	if ( !pOwner ) return XUI_ERROR_INVALID_ARGUMENT;
	xui_accessible_provider_state_t* state = xrtCalloc(1, sizeof(*state));
	if ( !state ) return XUI_ERROR_OUT_OF_MEMORY;
	state->onNode = pDesc->onNode;
	state->onAction = pDesc->onAction;
	return __xuiAccessiblePublishProvider(pWidget, pDesc->onCount, NULL, NULL, pDesc->pUser, state, pOwner);
}

XUI_API int xuiAccessibleNodeOutputSet(xui_accessible_node_output_t* pOutput,
	const xui_accessible_node_values_t* pValues, const xui_accessible_node_text_t* pText)
{
	if ( !pOutput || !pOutput->pNode || !pValues ) return XUI_ERROR_INVALID_ARGUMENT;
	const char* inputs[3] = {0};
	if ( pText ) {
		inputs[0] = pText->sName;
		inputs[1] = pText->sValue;
		inputs[2] = pText->sDescription;
	}
	const char* stored[3] = {0};
	size_t lengths[3] = {0};
	size_t total = 0;
	for ( size_t i = 0; i < 3; ++i ) {
		if ( !inputs[i] ) continue;
		size_t size = strlen(inputs[i]);
		if ( size >= SIZE_MAX - total ) return XUI_ERROR_OUT_OF_MEMORY;
		lengths[i] = size + 1;
		total += lengths[i];
	}
	char* texts = total ? xrtMalloc(total) : NULL;
	if ( total && !texts ) return XUI_ERROR_OUT_OF_MEMORY;
	char* next = texts;
	for ( size_t i = 0; i < 3; ++i ) {
		if ( !lengths[i] ) continue;
		memcpy(next, inputs[i], lengths[i]);
		stored[i] = next;
		next += lengths[i];
	}
	*pOutput->pNode = (xui_accessible_node_t){
		.iSize = sizeof(*pOutput->pNode), .iId = pValues->iId, .iParentId = pValues->iParentId,
		.iRole = pValues->iRole, .iState = pValues->iState,
		.sName = stored[0], .sValue = stored[1], .sDescription = stored[2], .tBounds = pValues->tBounds,
		.iTextStart = pValues->iTextStart, .iTextEnd = pValues->iTextEnd, .iLevel = pValues->iLevel,
		.iRow = pValues->iRow, .iColumn = pValues->iColumn, .iRowCount = pValues->iRowCount,
		.iColumnCount = pValues->iColumnCount, .iActions = pValues->iActions, .tRange = pValues->tRange
	};
	xrtFree(pOutput->sTexts);
	pOutput->sTexts = texts;
	pOutput->bSet = true;
	return XUI_OK;
}

XUI_API int xuiInternalAccessibilitySetFocusResolver(xui_widget pWidget,
	xui_internal_accessible_focus_proc onFocus)
{
	if ( !__xuiAccessibleValid(pWidget) ||
	     (onFocus != NULL && !__xuiAccessibleHasNodeProvider(pWidget)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pWidget->onAccessibleFocus = onFocus;
	pWidget->iAccessibleProviderGeneration++;
	return XUI_OK;
}

XUI_API int xuiInternalAccessibilityGetFocusedNode(xui_widget pWidget,
	uint64_t* pNodeId)
{
	xui_context pContext;
	uint32_t iGeneration;
	uint32_t iMutation;
	int iRet;

	if ( pNodeId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pNodeId = 0;
	if ( !__xuiAccessibleValid(pWidget) || pWidget->bAccessibleQueryActive ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pWidget->onAccessibleFocus == NULL ) return XUI_ERROR_UNSUPPORTED;
	pContext = pWidget->pContext;
	xuiInternalOperationEnter(pContext);
	pWidget->bAccessibleQueryActive = 1;
	__xuiAccessibleDrain(pContext);
	iGeneration = pWidget->iAccessibleProviderGeneration;
	iMutation = pWidget->iAccessibleMutationGeneration;
	iRet = __xuiAccessibleValid(pWidget) && pWidget->onAccessibleFocus != NULL ?
		pWidget->onAccessibleFocus(pWidget, pNodeId, pWidget->pAccessibleUser) :
		XUI_ERROR_INVALID_ARGUMENT;
	__xuiAccessibleDrain(pContext);
	if ( !__xuiAccessibleValid(pWidget) ||
	     iGeneration != pWidget->iAccessibleProviderGeneration ||
	     iMutation != pWidget->iAccessibleMutationGeneration ) {
		iRet = XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iRet != XUI_OK ) *pNodeId = 0;
	pWidget->bAccessibleQueryActive = 0;
	xuiInternalOperationLeave(pContext);
	return iRet;
}

static int __xuiAccessibleSetText(xui_widget pWidget, char** psTarget, const char* sText)
{
	char* sCopy = NULL;
	if ( !__xuiAccessibleValid(pWidget) || psTarget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sText != NULL && sText[0] != 0 ) {
		size_t iLength = strlen(sText) + 1;
		sCopy = (char*)xrtMalloc(iLength);
		if ( sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		memcpy(sCopy, sText, iLength);
	}
	xrtFree(*psTarget);
	*psTarget = sCopy;
	return xuiWidgetNotifyAccessibility(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED, 1);
}

XUI_API int xuiWidgetSetAccessibleName(xui_widget pWidget, const char* sName)
{
	return __xuiAccessibleSetText(pWidget, pWidget != NULL ? &pWidget->sAccessibleName : NULL, sName);
}

XUI_API const char* xuiWidgetGetAccessibleName(xui_widget pWidget)
{
	return __xuiAccessibleValid(pWidget) ? pWidget->sAccessibleName : NULL;
}

XUI_API int xuiWidgetSetAccessibleDescription(xui_widget pWidget, const char* sDescription)
{
	return __xuiAccessibleSetText(pWidget,
		pWidget != NULL ? &pWidget->sAccessibleDescription : NULL, sDescription);
}

XUI_API const char* xuiWidgetGetAccessibleDescription(xui_widget pWidget)
{
	return __xuiAccessibleValid(pWidget) ? pWidget->sAccessibleDescription : NULL;
}

XUI_API int xuiWidgetGetAccessibleNodeCountChecked(xui_widget pWidget)
{
	xui_context pContext;
	uint32_t iGeneration, iMutation;
	int iCount = XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiAccessibleValid(pWidget) || pWidget->bAccessibleQueryActive ) return iCount;
	pContext = pWidget->pContext;
	xuiInternalOperationEnter(pContext);
	pWidget->bAccessibleQueryActive = 1;
	__xuiAccessibleDrain(pContext);
	iGeneration = pWidget->iAccessibleProviderGeneration;
	iMutation = pWidget->iAccessibleMutationGeneration;
	if ( __xuiAccessibleValid(pWidget) ) {
		xui_accessible_count_proc count = pWidget->onAccessibleCount;
		void* user = pWidget->pAccessibleUser;
		if ( count && pWidget->pAccessibleOwned ) {
			xui_callback_snapshot_t snapshot = {0};
			iCount = xuiInternalCallbackSnapshotBeginReturned(pWidget, &pWidget->pAccessibleUser, &user, &snapshot);
			if ( iCount == XUI_OK ) {
				iCount = count(pWidget, user);
				int status = xuiInternalCallbackSnapshotEnd(&snapshot, iCount < 0 ? iCount : XUI_OK);
				if ( status != XUI_OK ) iCount = status;
			}
		} else iCount = count ? count(pWidget, user) : 1;
	}
	__xuiAccessibleDrain(pContext);
	if ( !__xuiAccessibleValid(pWidget) || iGeneration != pWidget->iAccessibleProviderGeneration ||
		iMutation != pWidget->iAccessibleMutationGeneration ) iCount = XUI_ERROR_INVALID_ARGUMENT;
	pWidget->bAccessibleQueryActive = 0;
	xuiInternalOperationLeave(pContext);
	return iCount;
}

XUI_API int xuiWidgetGetAccessibleNodeCount(xui_widget pWidget)
{
	int count = xuiWidgetGetAccessibleNodeCountChecked(pWidget);
	return count > 0 ? count : 0;
}

static int __xuiAccessibleDefaultNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	const xui_internal_accessibility_adapter_t* pAdapter = __xuiAccessibleAdapter(pWidget);
	int iRet = XUI_OK;
	pNode->iId = 1;
	pNode->iRole = __xuiAccessibleRole(pWidget);
	pNode->tBounds = xuiWidgetGetWorldRect(pWidget);
	if ( pAdapter != NULL && pAdapter->getNode != NULL ) iRet = pAdapter->getNode(pWidget, pNode);
	if ( pWidget->onDefaultAction == NULL && (pAdapter == NULL || pAdapter->performAction == NULL) )
		pNode->iActions &= ~(XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE) |
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE));
	if ( pWidget->sAccessibleName != NULL ) pNode->sName = pWidget->sAccessibleName;
	return iRet;
}

XUI_API int xuiWidgetGetAccessibleNode(xui_widget pWidget, int iIndex, xui_accessible_node_t* pNode)
{
	xui_context pContext;
	xui_accessible_node_t tNode;
	xui_accessible_node_output_t output = {.pNode = &tNode};
	uint32_t iGeneration, iRevision, iMutation;
	size_t iSize;
	int iRet = XUI_ERROR_INVALID_ARGUMENT;
	if ( pNode == NULL ) return iRet;
	iSize = pNode->iSize != 0 ? pNode->iSize : sizeof(*pNode);
	if ( iSize < offsetof(xui_accessible_node_t, iActions) ) return iRet;
	if ( iSize > sizeof(*pNode) ) iSize = sizeof(*pNode);
	memset(pNode, 0, iSize);
	pNode->iSize = (uint32_t)iSize;
	if ( !__xuiAccessibleValid(pWidget) || iIndex < 0 || pWidget->bAccessibleQueryActive ) return iRet;
	pContext = pWidget->pContext;
	xuiInternalOperationEnter(pContext);
	pWidget->bAccessibleQueryActive = 1;
	__xuiAccessibleDrain(pContext);
	iGeneration = pWidget->iAccessibleProviderGeneration;
	iRevision = pWidget->iAccessibilityRevision;
	iMutation = pWidget->iAccessibleMutationGeneration;
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	if ( __xuiAccessibleValid(pWidget) ) {
		if ( pWidget->onAccessibleGet != NULL ) {
			iRet = pWidget->onAccessibleGet(pWidget, iIndex, &tNode, pWidget->pAccessibleUser);
		} else if ( pWidget->pAccessibleOwned && pWidget->pAccessibleOwned->onNode ) {
			xui_accessible_node_owned_proc node = pWidget->pAccessibleOwned->onNode;
			void* user = pWidget->pAccessibleUser;
			xui_callback_snapshot_t snapshot = {0};
			iRet = xuiInternalCallbackSnapshotBeginReturned(pWidget, &pWidget->pAccessibleUser, &user, &snapshot);
			if ( iRet == XUI_OK ) {
				iRet = node(pWidget, iIndex, &output, user);
				iRet = xuiInternalCallbackSnapshotEnd(&snapshot, iRet);
				if ( iRet == XUI_OK && !output.bSet ) iRet = XUI_ERROR_INVALID_STATE;
			}
		} else if ( iIndex == 0 ) iRet = __xuiAccessibleDefaultNode(pWidget, &tNode);
	}
	__xuiAccessibleDrain(pContext);
	if ( !__xuiAccessibleValid(pWidget) || iGeneration != pWidget->iAccessibleProviderGeneration ||
		iRevision != pWidget->iAccessibilityRevision ||
		iMutation != pWidget->iAccessibleMutationGeneration ) iRet = XUI_ERROR_INVALID_ARGUMENT;
	if ( iRet == XUI_OK ) {
		xui_widget pFocusTarget = __xuiAccessibleEditTarget(pWidget);
		bool hasProvider = __xuiAccessibleHasNodeProvider(pWidget);
		int bRootNode = !hasProvider || iIndex == 0;
		if ( tNode.sName == NULL ) tNode.sName = pWidget->sAccessibleName;
		if ( tNode.sDescription == NULL ) tNode.sDescription = pWidget->sAccessibleDescription;
		if ( (tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) == 0 &&
		     xuiInternalInputFocusAllowed(pContext, pFocusTarget) ) {
			if ( bRootNode ||
			     (tNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS)) != 0 )
				tNode.iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
			if ( bRootNode &&
			     (!hasProvider || __xuiAccessibleHasActionProvider(pWidget) || tNode.iId <= 1) )
				tNode.iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		}
		if ( (tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) == 0 &&
		     bRootNode &&
		     (pContext->pFocusWidget == pWidget ||
		      (pFocusTarget != NULL && pContext->pFocusWidget == pFocusTarget)) )
			tNode.iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		if ( (tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0 ) {
			tNode.iActions = 0;
			tNode.iState &= ~(XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_FOCUSED);
		}
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) tNode.iState |= XUI_ACCESSIBLE_STATE_DISABLED;
		if ( !xuiWidgetGetEffectiveVisible(pWidget) ) tNode.iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
		if ( !__xuiAccessibleCanAct(pWidget) ) {
			tNode.iActions = 0;
			tNode.iState &= ~(XUI_ACCESSIBLE_STATE_FOCUSABLE | XUI_ACCESSIBLE_STATE_FOCUSED);
		}
		tNode.iSize = (uint32_t)iSize;
		if ( output.bSet ) {
			xrtFree(pWidget->pAccessibleOwned->sNodeTexts);
			pWidget->pAccessibleOwned->sNodeTexts = output.sTexts;
			output.sTexts = NULL;
		}
		memcpy(pNode, &tNode, iSize);
	}
	xrtFree(output.sTexts);
	pWidget->bAccessibleQueryActive = 0;
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiInternalAccessibilityCreateTextRange(xui_widget widget, uint64_t id,
	const xui_internal_accessible_text_request* request,
	xui_internal_accessible_text_range** out)
{
	xui_context context;
	const xui_internal_accessibility_adapter_t* adapter;
	xui_internal_accessible_text_range* range = NULL;
	uint32_t generation, mutation;
	int result = XUI_ERROR_UNSUPPORTED;
	if (!out) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (!request || !__xuiAccessibleValid(widget) || widget->bAccessibleQueryActive ||
		widget->bAccessibleActionActive) return XUI_ERROR_INVALID_ARGUMENT;
	context = widget->pContext;
	xuiInternalOperationEnter(context);
	widget->bAccessibleQueryActive = 1;
	__xuiAccessibleDrain(context);
	generation = widget->iAccessibleProviderGeneration;
	mutation = widget->iAccessibleMutationGeneration;
	if (__xuiAccessibleValid(widget)) {
		adapter = __xuiAccessibleAdapter(widget);
		if (adapter && adapter->createTextRange)
			result = adapter->createTextRange(widget, id, request, &range);
	}
	__xuiAccessibleDrain(context);
	if (!__xuiAccessibleValid(widget) || generation != widget->iAccessibleProviderGeneration ||
		mutation != widget->iAccessibleMutationGeneration) result = XUI_ERROR_INVALID_STATE;
	if (result == XUI_OK && !range) result = XUI_ERROR_INVALID_STATE;
	if (result == XUI_OK && range) *out = range;
	else if (range) range->ops->release(range);
	widget->bAccessibleQueryActive = 0;
	xuiInternalOperationLeave(context);
	return result;
}

static int __xuiAccessibleActionData(int action, const void* raw, xui_accessible_action_data_t* data)
{
	if ( !raw ) return XUI_OK;
	if ( action == XUI_ACCESSIBLE_ACTION_SET_SELECTION ) {
		const xui_accessible_selection_t* selection = raw;
		if ( selection->iSize < sizeof(*selection) ) return XUI_ERROR_INVALID_ARGUMENT;
		data->iKind = XUI_ACCESSIBLE_PAYLOAD_SELECTION;
		data->iAnchor = selection->iAnchor;
		data->iCaret = selection->iCaret;
	} else if ( action == XUI_ACCESSIBLE_ACTION_SET_VALUE ) {
		const xui_accessible_value_t* value = raw;
		if ( value->iSize < sizeof(*value) ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( value->iType == XUI_ACCESSIBLE_VALUE_NUMBER ) {
			data->iKind = XUI_ACCESSIBLE_PAYLOAD_NUMBER;
			data->fNumber = value->fNumber;
		} else if ( value->iType == XUI_ACCESSIBLE_VALUE_TEXT && value->sText ) {
			data->iKind = XUI_ACCESSIBLE_PAYLOAD_TEXT;
			data->sText = value->sText;
		} else return XUI_ERROR_INVALID_ARGUMENT;
	} else return XUI_ERROR_INVALID_ARGUMENT;
	return XUI_OK;
}

XUI_API int xuiWidgetPerformAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pData)
{
	xui_context pContext;
	const xui_internal_accessibility_adapter_t* pAdapter;
	xui_accessible_node_t tNode;
	int iRet = XUI_ERROR_UNSUPPORTED;
	if ( !__xuiAccessibleCanAct(pWidget) || pWidget->bAccessibleActionActive ||
		pWidget->bAccessibleQueryActive )
		return XUI_ERROR_INVALID_ARGUMENT;
	pContext = pWidget->pContext;
	xuiInternalOperationEnter(pContext);
	pWidget->bAccessibleActionActive = 1;
	if ( pWidget->onAccessibleAction != NULL ) {
		iRet = pWidget->onAccessibleAction(pWidget, iNodeId, iAction, pData, pWidget->pAccessibleUser);
	} else if ( pWidget->pAccessibleOwned && pWidget->pAccessibleOwned->onAction ) {
		xui_accessible_action_data_t data = {0};
		iRet = __xuiAccessibleActionData(iAction, pData, &data);
		if ( iRet == XUI_OK ) {
			xui_accessible_action_owned_proc action = pWidget->pAccessibleOwned->onAction;
			void* user = pWidget->pAccessibleUser;
			xui_callback_snapshot_t snapshot = {0};
			iRet = xuiInternalCallbackSnapshotBeginReturned(pWidget, &pWidget->pAccessibleUser, &user, &snapshot);
			if ( iRet == XUI_OK ) {
				iRet = action(pWidget, iNodeId, iAction, &data, user);
				iRet = xuiInternalCallbackSnapshotEnd(&snapshot, iRet);
			}
		}
	} else if ( iAction == XUI_ACCESSIBLE_ACTION_FOCUS && (iNodeId == 0 || iNodeId == 1) ) {
		xui_widget pTarget = __xuiAccessibleEditTarget(pWidget);
		iRet = pTarget != NULL ? xuiSetFocusWidget(pContext, pTarget) : XUI_ERROR_INVALID_ARGUMENT;
	} else if ( !__xuiAccessibleHasNodeProvider(pWidget) && (iNodeId == 0 || iNodeId == 1) &&
		iAction >= 1 && iAction <= XUI_ACCESSIBLE_ACTION_CLOSE ) {
		memset(&tNode, 0, sizeof(tNode));
		tNode.iSize = sizeof(tNode);
		pAdapter = __xuiAccessibleAdapter(pWidget);
		if ( __xuiAccessibleDefaultNode(pWidget, &tNode) == XUI_OK &&
			(tNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(iAction)) != 0 ) {
			if ( (iAction == XUI_ACCESSIBLE_ACTION_ACTIVATE || iAction == XUI_ACCESSIBLE_ACTION_TOGGLE) &&
				pWidget->onDefaultAction != NULL ) {
				iRet=xuiInternalInvokeDefaultAction(pWidget);
			} else if ( pAdapter != NULL && pAdapter->performAction != NULL ) {
				iRet = pAdapter->performAction(pWidget, iAction, pData);
			}
		}
	}
	/* Successful commands may intentionally destroy their widget or context. */
	pWidget->bAccessibleActionActive = 0;
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiWidgetPerformAccessibleSelection(xui_widget widget, uint64_t nodeId, int anchor, int caret)
{
	const xui_accessible_selection_t selection = {sizeof(selection), anchor, caret};
	return xuiWidgetPerformAccessibleAction(widget, nodeId, XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection);
}

XUI_API int xuiWidgetSetAccessibleTextValue(xui_widget widget, uint64_t nodeId, const char* text)
{
	if ( !text ) return XUI_ERROR_INVALID_ARGUMENT;
	const xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_TEXT, text, 0};
	return xuiWidgetPerformAccessibleAction(widget, nodeId, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value);
}

XUI_API int xuiWidgetSetAccessibleNumberValue(xui_widget widget, uint64_t nodeId, double number)
{
	if ( !isfinite(number) ) return XUI_ERROR_INVALID_ARGUMENT;
	const xui_accessible_value_t value = {sizeof(value), XUI_ACCESSIBLE_VALUE_NUMBER, NULL, number};
	return xuiWidgetPerformAccessibleAction(widget, nodeId, XUI_ACCESSIBLE_ACTION_SET_VALUE, &value);
}

XUI_API uint32_t xuiWidgetGetAccessibilityRevision(xui_widget pWidget)
{
	return __xuiAccessibleValid(pWidget) ? pWidget->iAccessibilityRevision : 0;
}

XUI_API int xuiWidgetNotifyAccessibility(xui_widget widget,int type,uint64_t nodeId)
{
    if(!__xuiAccessibleValid(widget) || type<XUI_ACCESSIBLE_EVENT_TREE_CHANGED ||
       type>XUI_ACCESSIBLE_EVENT_STATE_CHANGED)return XUI_ERROR_INVALID_ARGUMENT;
    xui_context context=widget->pContext;xui_call_scope_t scope={0};
    int result=xuiCallScopeBegin(context,&scope);if(result!=XUI_OK)return result;
    uint32_t bit=XUI_A11Y_BIT(type);++widget->iAccessibleMutationGeneration;
    if(widget->iActiveAccessibilityEvents&bit)return xuiCallScopeEnd(&scope);
    widget->iActiveAccessibilityEvents|=bit;
    if(++widget->iAccessibilityRevision==0)widget->iAccessibilityRevision=1;
    const xui_accessibility_event_t event={sizeof(event),type,nodeId,widget->iAccessibilityRevision};
    xuiInternalNativeAccessibilityNotify(context,widget,&event);
    xerror* first=xrtTakeError();if(first)result=XUI_ERROR_CALLBACK_FAILED;
    xuiInternalCallScopeRecordOutcome(&scope,result,first);
    xui_accessibility_event_proc callback=NULL;void* user=NULL;xui_callback_record_t* record=NULL;
    if(!xuiInternalContextDestroyPending(context) && xuiInternalWidgetIsValid(widget) && context->onAccessibilityEvent){
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation)){
            if(result==XUI_OK)result=XUI_ERROR_INVALID_STATE;
        }else{
            callback=context->onAccessibilityEvent;user=context->pAccessibilityEventUser;
            record=context->pAccessibilityOwner;
            int pin=xuiInternalCallbackRecordRetain(record);
            if(pin!=XUI_OK){record=NULL;callback=NULL;if(result==XUI_OK)result=pin;}
            if(!xrtOwnershipScopeEnd(&mutation))abort();
        }
    }
    if(callback)callback(context,widget,&event,user);
    xerror* current=xrtTakeError();
    if(current){if(!first){first=current;if(result==XUI_OK)result=XUI_ERROR_CALLBACK_FAILED;}else xrtErrorFree(current);}
    widget->iActiveAccessibilityEvents&=~bit;
    xuiInternalCallScopeRecordOutcome(&scope,result,first);
    int next=xuiInternalCallbackRecordRelease(context,record,result,first);
    xuiInternalContextCleanupOutcome(context,next,&result,&first);
    result=xuiInternalCallScopeEndOutcome(&scope,result,&first);xrtSetErrorTake(first);return result;
}

void xuiInternalAccessibilityQueue(xui_widget pWidget, int iEventType)
{
	xui_context pContext;
	uint32_t iBit;
	if ( !__xuiAccessibleValid(pWidget) || iEventType < XUI_ACCESSIBLE_EVENT_TREE_CHANGED ||
		iEventType > XUI_ACCESSIBLE_EVENT_STATE_CHANGED ) return;
	pContext = pWidget->pContext;
	pWidget->iAccessibleMutationGeneration++;
	iBit = XUI_A11Y_BIT(iEventType);
	if ( (pWidget->iActiveAccessibilityEvents & iBit) != 0 ||
		(pContext->bAccessibilityFlushing && pWidget->iAccessibilitySeenEpoch == pContext->iAccessibilityFlushEpoch &&
		(pWidget->iAccessibilitySeenEvents & iBit) != 0) ) return;
	if ( pWidget->iPendingAccessibilityEvents == 0 ) {
		pWidget->pAccessibilityPrev = pContext->pAccessibilityTail;
		pWidget->pAccessibilityNext = NULL;
		if ( pContext->pAccessibilityTail != NULL ) pContext->pAccessibilityTail->pAccessibilityNext = pWidget;
		else pContext->pAccessibilityHead = pWidget;
		pContext->pAccessibilityTail = pWidget;
	}
	pWidget->iPendingAccessibilityEvents |= iBit;
	if ( iEventType == XUI_ACCESSIBLE_EVENT_FOCUS_CHANGED ) {
		xui_widget pHost = xuiInternalEditHost(pWidget);
		if ( pHost != pWidget ) xuiInternalAccessibilityQueue(pHost, iEventType);
	}
}

void xuiInternalAccessibilityDetach(xui_widget pWidget)
{
	xui_context pContext = pWidget->pContext;
	if ( pWidget->iPendingAccessibilityEvents == 0 ) return;
	if ( pWidget->pAccessibilityPrev != NULL ) pWidget->pAccessibilityPrev->pAccessibilityNext = pWidget->pAccessibilityNext;
	else pContext->pAccessibilityHead = pWidget->pAccessibilityNext;
	if ( pWidget->pAccessibilityNext != NULL ) pWidget->pAccessibilityNext->pAccessibilityPrev = pWidget->pAccessibilityPrev;
	else pContext->pAccessibilityTail = pWidget->pAccessibilityPrev;
	pWidget->pAccessibilityPrev = pWidget->pAccessibilityNext = NULL;
	pWidget->iPendingAccessibilityEvents = 0;
}

/* Only publish at the outer update barrier: a control setter can call nested
 * Widget operations while it still owns pointers needed after those calls. */
void xuiInternalAccessibilityFlush(xui_context pContext)
{
	xui_widget pWidget;
	if ( !xuiInternalContextIsValid(pContext) || xuiInternalContextDestroyPending(pContext) ||
		pContext->bAccessibilityFlushing ) return;
	pContext->bAccessibilityFlushing = 1;
	pContext->iAccessibilityFlushEpoch++;
	if ( pContext->iAccessibilityFlushEpoch == 0 ) pContext->iAccessibilityFlushEpoch = 1;
	while ( !xuiInternalContextDestroyPending(pContext) && (pWidget = pContext->pAccessibilityHead) != NULL ) {
		uint32_t iEvents = pWidget->iPendingAccessibilityEvents;
		int iType;
		xuiInternalAccessibilityDetach(pWidget);
		if ( !__xuiAccessibleValid(pWidget) ) continue;
		if ( pWidget->iAccessibilitySeenEpoch != pContext->iAccessibilityFlushEpoch ) {
			pWidget->iAccessibilitySeenEpoch = pContext->iAccessibilityFlushEpoch;
			pWidget->iAccessibilitySeenEvents = 0;
		}
		/* Bound cross-widget callback cycles to one event of each kind per drain. */
		iEvents &= ~pWidget->iAccessibilitySeenEvents;
		pWidget->iAccessibilitySeenEvents |= iEvents;
		for ( iType = XUI_ACCESSIBLE_EVENT_TREE_CHANGED; iType <= XUI_ACCESSIBLE_EVENT_STATE_CHANGED; iType++ ) {
			if ( !__xuiAccessibleValid(pWidget) ) break;
			if ( (iEvents & XUI_A11Y_BIT(iType)) != 0 )
				(void)xuiWidgetNotifyAccessibility(pWidget, iType, 1);
		}
	}
	pContext->bAccessibilityFlushing = 0;
}

int xuiInternalAccessibilityEditNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_widget pTarget = __xuiAccessibleEditTarget(pWidget);
	if ( pTarget != NULL && pTarget != pWidget ) {
		const xui_internal_accessibility_adapter_t* pAdapter = __xuiAccessibleAdapter(pTarget);
		if ( pAdapter != NULL && pAdapter->getNode != NULL ) return pAdapter->getNode(pTarget, pNode);
	}
	pNode->iRole = XUI_ACCESSIBLE_ROLE_TEXTBOX;
	pNode->sValue = xuiEditGetText(pWidget);
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
	if ( xuiEditIsReadonly(pWidget) ) pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	else {
		pNode->iState |= XUI_ACCESSIBLE_STATE_EDITABLE;
		pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE);
	}
	(void)xuiEditGetSelection(pWidget, &pNode->iTextStart, &pNode->iTextEnd);
	return XUI_OK;
}

int xuiInternalAccessibilityEditAction(xui_widget pWidget, int iAction, const void* pData)
{
	if ( !__xuiAccessibleCanAct(__xuiAccessibleEditTarget(pWidget)) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_SELECTION ) {
		const xui_accessible_selection_t* pSelection = (const xui_accessible_selection_t*)pData;
		if ( pSelection == NULL || pSelection->iSize < sizeof(*pSelection) ||
			pSelection->iAnchor < 0 || pSelection->iCaret < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
		return xuiEditSetSelection(pWidget, pSelection->iAnchor, pSelection->iCaret);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_VALUE && !xuiEditIsReadonly(pWidget) ) {
		const xui_accessible_value_t* pValue = (const xui_accessible_value_t*)pData;
		if ( pValue == NULL || pValue->iSize < sizeof(*pValue) ||
			pValue->iType != XUI_ACCESSIBLE_VALUE_TEXT || pValue->sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		return xuiEditSetText(pWidget, pValue->sText);
	}
	return XUI_ERROR_UNSUPPORTED;
}

#endif
