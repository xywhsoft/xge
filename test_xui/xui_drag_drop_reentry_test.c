#include "src/xui_internal.h"

#include <stdio.h>
#include <string.h>

enum {
	ACTION_CANCEL, ACTION_REPLACE, ACTION_DESTROY_NEXT, ACTION_MOVE_NEXT,
	ACTION_DESTROY_SOURCE, ACTION_DESTROY_TARGET, ACTION_DESTROY_ROOT,
	ACTION_DESTROY_CONTEXT, ACTION_DROP, ACTION_MOVE, ACTION_RESTART_DETACHING,
	ACTION_FREE_REPLACE, ACTION_FREE_DESTROY, ACTION_FREE_SHUTDOWN,
	ACTION_ERROR, ACTION_REPLACE_ERROR, ACTION_NONE
};
enum {
	CALL_MOVE, CALL_CANCEL, CALL_DROP, CALL_BEGIN, CALL_REPLACE,
	CALL_EXTERNAL_OVER, CALL_EXTERNAL_DROP, CALL_EXTERNAL_CANCEL, CALL_SHUTDOWN
};

typedef struct test_case_t {
	const char* sName;
	int iEvent;
	int iAction;
	int iCall;
	int bInternal;
} test_case_t;

typedef struct test_state_t {
	const test_case_t* pCase;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pSource;
	xui_widget pTargets[3];
	xui_data_object pOld;
	xui_data_object pNew;
	int iOldFree;
	int iNewFree;
	int iActions;
	int iOldEvents[3][4];
	int iNewEnter;
	int iFailed;
	int bContextDestroyed;
	int bCallerOwnsOld;
} test_state_t;

#define CHECK(state, expr, message) do { \
	if ( !(expr) ) { \
		printf("FAIL %s: %s (line %d)\n", (state)->pCase->sName, message, __LINE__); \
		(state)->iFailed = 1; \
	} \
} while ( 0 )

static int read_payload(const char* sFormat, void* pOutput, size_t iCapacity,
	size_t* pOutputSize, void* pUser)
{
	const char sText[] = "retained payload";
	(void)sFormat;
	(void)pUser;
	*pOutputSize = sizeof(sText);
	if ( pOutput == NULL ) return XUI_OK;
	if ( iCapacity < sizeof(sText) ) return XUI_ERROR_BUFFER_TOO_SMALL;
	memcpy(pOutput, sText, sizeof(sText));
	return XUI_OK;
}

static void free_payload(void* pUser)
{
	(*(int*)pUser)++;
}

static void check_payload(test_state_t* pState, xui_data_object pData)
{
	char sText[32];
	size_t iSize = 0;
	CHECK(pState, pState->iOldFree == 0, "event payload released during callback");
	if ( pState->iOldFree != 0 ) return;
	CHECK(pState, xuiDataObjectGet(pData, XUI_DATA_FORMAT_TEXT_UTF8,
		sText, sizeof(sText), &iSize) == XUI_OK, "read event payload after reentry");
	CHECK(pState, iSize == sizeof("retained payload") &&
		strcmp(sText, "retained payload") == 0, "event payload unchanged");
}

static int start_new(test_state_t* pState)
{
	uint32_t iEffect = 0;
	return xuiDragExternalEvent(pState->pContext, XUI_DRAG_EXTERNAL_ENTER,
		350, 30, 0u, pState->pNew, XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE,
		XUI_DRAG_EFFECT_COPY, &iEffect);
}

static void free_old_payload(void* pUser)
{
	test_state_t* pState = (test_state_t*)pUser;
	pState->iOldFree++;
	if ( pState->pCase->iAction == ACTION_FREE_REPLACE ) {
		pState->iActions++;
		CHECK(pState, start_new(pState) == XUI_OK, "release callback starts replacement");
	} else if ( pState->pCase->iAction == ACTION_FREE_DESTROY ) {
		pState->iActions++;
		CHECK(pState, pState->pContext->iOperationDepth > 0, "release callback has operation barrier");
		xuiDestroy(pState->pContext);
		pState->bContextDestroyed = 1;
	} else if ( pState->pCase->iAction == ACTION_FREE_SHUTDOWN ) {
		pState->iActions++;
		CHECK(pState, start_new(pState) != XUI_OK, "shutdown rejects new external session");
		CHECK(pState, xuiDragBegin(pState->pContext, pState->pSource, pState->pNew,
			XUI_DRAG_EFFECT_COPY, XUI_DRAG_EFFECT_COPY) != XUI_OK,
			"shutdown rejects new internal session");
	}
}

static int target_event(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	test_state_t* pState = (test_state_t*)pUser;
	xui_drag_event_data_t* pDrag;
	int iIndex;
	int iSlot;
	int iRet;

	if ( pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
	switch ( pEvent->iType ) {
	case XUI_EVENT_DRAG_ENTER: iSlot = 0; break;
	case XUI_EVENT_DRAG_OVER: iSlot = 1; break;
	case XUI_EVENT_DRAG_LEAVE: iSlot = 2; break;
	case XUI_EVENT_DROP: iSlot = 3; break;
	default: return XUI_OK;
	}
	pDrag = (xui_drag_event_data_t*)pEvent->pData;
	CHECK(pState, pDrag != NULL && pDrag->pData != NULL, "drag event has data");
	if ( pDrag == NULL || pDrag->pData == NULL ) return XUI_OK;
	for ( iIndex = 0; iIndex < 3 && pState->pTargets[iIndex] != pWidget; iIndex++ ) {}
	if ( pDrag->pData == pState->pNew ) {
		if ( pEvent->iType == XUI_EVENT_DRAG_ENTER ) pState->iNewEnter++;
		if ( pEvent->iType != XUI_EVENT_DRAG_LEAVE ) {
			CHECK(pState, xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_COPY) == XUI_OK,
				"new event can negotiate");
		}
		return XUI_OK;
	}
	CHECK(pState, pDrag->pData == pState->pOld && iIndex < 3, "old event identity");
	if ( iIndex < 3 ) pState->iOldEvents[iIndex][iSlot]++;
	if ( iIndex != 0 || pEvent->iType != pState->pCase->iEvent ||
		(pState->iActions != 0 && pState->pCase->iAction != ACTION_CANCEL) ) {
		if ( pEvent->iType != XUI_EVENT_DRAG_LEAVE ) {
			(void)xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_COPY);
		}
		return XUI_OK;
	}
	/* Bound the old implementation's recursive cancel so failures stay diagnostic. */
	if ( ++pState->iActions > 4 ) {
		CHECK(pState, 0, "recursive cancel dispatches LEAVE repeatedly");
		return XUI_OK;
	}
	switch ( pState->pCase->iAction ) {
	case ACTION_CANCEL:
		CHECK(pState, xuiDragCancel(pState->pContext) == XUI_OK, "cancel from callback");
		break;
	case ACTION_REPLACE:
	case ACTION_REPLACE_ERROR:
		CHECK(pState, xuiDragCancel(pState->pContext) == XUI_OK, "cancel old session");
		CHECK(pState, start_new(pState) == XUI_OK, "start replacement session");
		CHECK(pState, xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_MOVE) ==
			XUI_ERROR_INVALID_STATE, "old callback cannot negotiate new session");
		break;
	case ACTION_DESTROY_NEXT:
		xuiWidgetDestroy(pState->pTargets[1]);
		break;
	case ACTION_MOVE_NEXT:
		CHECK(pState, xuiWidgetSetRect(pState->pTargets[1],
			(xui_rect_t){420, 10, 60, 60}) == XUI_OK, "move candidate during LEAVE");
		break;
	case ACTION_DESTROY_SOURCE:
		xuiWidgetDestroy(pState->pSource);
		break;
	case ACTION_DESTROY_TARGET:
		xuiWidgetDestroy(pWidget);
		break;
	case ACTION_DESTROY_ROOT:
		xuiWidgetDestroy(pState->pRoot);
		break;
	case ACTION_DESTROY_CONTEXT:
		xuiDestroy(pState->pContext);
		pState->bContextDestroyed = 1;
		CHECK(pState, pState->pContext->iOperationDepth > 1,
			"drag operation protects context beyond event dispatch");
		CHECK(pState, xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_COPY) ==
			XUI_ERROR_INVALID_STATE, "destroy-pending context cannot negotiate");
		break;
	case ACTION_DROP:
		CHECK(pState, xuiInternalDragTransferDrop(pState->pContext, 110, 30, 0u) ==
			XUI_OK, "recursive drop returns");
		CHECK(pState, xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_COPY) == XUI_OK,
			"outer drop still negotiates after recursive drop");
		break;
	case ACTION_MOVE:
		CHECK(pState, xuiInternalDragTransferMove(pState->pContext, 350, 30, 0u) ==
			XUI_OK, "nested move returns");
		CHECK(pState, xuiDragAccept(pState->pContext, XUI_DRAG_EFFECT_MOVE) ==
			XUI_ERROR_INVALID_STATE, "superseded event cannot negotiate");
		break;
	case ACTION_RESTART_DETACHING:
		/* This callback runs inside source destruction, before its pending flag. */
		iRet = xuiDragBegin(pState->pContext, pState->pSource, pState->pNew,
			XUI_DRAG_EFFECT_COPY, XUI_DRAG_EFFECT_COPY);
		CHECK(pState, iRet != XUI_OK, "cannot restart from source being detached");
		CHECK(pState, start_new(pState) == XUI_OK, "unrelated session may start during detach");
		break;
	case ACTION_ERROR:
		break;
	}
	check_payload(pState, pDrag->pData);
	if ( pState->pCase->iAction == ACTION_ERROR ||
		pState->pCase->iAction == ACTION_REPLACE_ERROR ) return XUI_ERROR;
	return XUI_OK;
}

static int setup(test_state_t* pState)
{
	int i;
	if ( xuiCreate(&pState->pContext) != XUI_OK ) return 0;
	if ( xuiSetViewportSize(pState->pContext, 500, 100) != XUI_OK ) return 0;
	if ( xuiWidgetCreate(pState->pContext, &pState->pRoot) != XUI_OK ) return 0;
	if ( xuiSetRootWidget(pState->pContext, pState->pRoot) != XUI_OK ) return 0;
	if ( xuiWidgetCreate(pState->pContext, &pState->pSource) != XUI_OK ) return 0;
	if ( xuiWidgetSetRect(pState->pSource, (xui_rect_t){10, 10, 60, 60}) != XUI_OK ) return 0;
	if ( xuiWidgetAddChild(pState->pRoot, pState->pSource) != XUI_OK ) return 0;
	for ( i = 0; i < 3; i++ ) {
		if ( xuiWidgetCreate(pState->pContext, &pState->pTargets[i]) != XUI_OK ) return 0;
		if ( xuiWidgetSetRect(pState->pTargets[i],
			(xui_rect_t){100 + i * 120, 10, 60, 60}) != XUI_OK ) return 0;
		if ( xuiWidgetAddChild(pState->pRoot, pState->pTargets[i]) != XUI_OK ) return 0;
		if ( xuiWidgetSetDropEnabled(pState->pTargets[i], 1) != XUI_OK ) return 0;
		if ( xuiWidgetSetEventCallback(pState->pTargets[i], target_event, pState) != XUI_OK ) return 0;
	}
	if ( xuiDataObjectCreate(&pState->pOld) != XUI_OK ) return 0;
	pState->bCallerOwnsOld = 1;
	if ( xuiDataObjectSetProvider(pState->pOld, XUI_DATA_FORMAT_TEXT_UTF8,
		read_payload, free_old_payload, pState) != XUI_OK ) return 0;
	if ( xuiDataObjectCreate(&pState->pNew) != XUI_OK ) return 0;
	if ( xuiDataObjectSetProvider(pState->pNew, XUI_DATA_FORMAT_TEXT_UTF8,
		read_payload, free_payload, &pState->iNewFree) != XUI_OK ) return 0;
	xuiClearEvents(pState->pContext);
	return 1;
}

static int run_case(const test_case_t* pCase)
{
	test_state_t tState;
	uint32_t iEffect = 0;
	int iRet;
	int bReplacement;

	memset(&tState, 0, sizeof(tState));
	tState.pCase = pCase;
	if ( !setup(&tState) ) {
		CHECK(&tState, 0, "setup");
		goto cleanup;
	}
	if ( pCase->bInternal ) {
		tState.pContext->fPointerX = 110;
		tState.pContext->fPointerY = 30;
		iRet = xuiDragBegin(tState.pContext, tState.pSource, tState.pOld,
			XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE, XUI_DRAG_EFFECT_COPY);
	} else {
		iRet = xuiDragExternalEvent(tState.pContext, XUI_DRAG_EXTERNAL_ENTER,
			110, 30, 0u, tState.pOld, XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE,
			XUI_DRAG_EFFECT_COPY, &iEffect);
	}
	CHECK(&tState, iRet == XUI_OK, "begin returns success");
	xuiDataObjectRelease(tState.pOld);
	tState.bCallerOwnsOld = 0;
	if ( tState.bContextDestroyed ) goto cleanup;
	if ( pCase->iAction == ACTION_RESTART_DETACHING ) {
		xuiWidgetDestroy(tState.pSource);
		iRet = XUI_OK;
	} else if ( pCase->iCall == CALL_MOVE ) {
		iRet = xuiInternalDragTransferMove(tState.pContext,
			pCase->iEvent == XUI_EVENT_DRAG_OVER ? 110 : 230, 30, 0u);
	} else if ( pCase->iCall == CALL_DROP ) {
		iRet = xuiInternalDragTransferDrop(tState.pContext, 110, 30, 0u);
	} else if ( pCase->iCall == CALL_CANCEL ) {
		iRet = xuiDragCancel(tState.pContext);
	} else if ( pCase->iCall == CALL_REPLACE ) {
		iRet = xuiDragExternalEvent(tState.pContext, XUI_DRAG_EXTERNAL_ENTER,
			230, 30, 0u, tState.pOld, XUI_DRAG_EFFECT_COPY,
			XUI_DRAG_EFFECT_COPY, &iEffect);
	} else if ( pCase->iCall == CALL_EXTERNAL_OVER || pCase->iCall == CALL_EXTERNAL_DROP ||
		pCase->iCall == CALL_EXTERNAL_CANCEL ) {
		int iType = pCase->iCall == CALL_EXTERNAL_OVER ? XUI_DRAG_EXTERNAL_OVER :
			(pCase->iCall == CALL_EXTERNAL_DROP ? XUI_DRAG_EXTERNAL_DROP : XUI_DRAG_EXTERNAL_CANCEL);
		iRet = xuiDragExternalEvent(tState.pContext, iType,
			pCase->iCall == CALL_EXTERNAL_OVER ? 230 : 110, 30, 0u, NULL, 0u, 0u, &iEffect);
	} else if ( pCase->iCall == CALL_SHUTDOWN ) {
		xuiDestroy(tState.pContext);
		tState.bContextDestroyed = 1;
		iRet = XUI_OK;
	}
	CHECK(&tState, iRet == ((pCase->iAction == ACTION_ERROR ||
		pCase->iAction == ACTION_REPLACE_ERROR) ? XUI_ERROR : XUI_OK), "outer operation return code");
	CHECK(&tState, tState.iActions == (pCase->iAction == ACTION_NONE ? 0 : 1),
		"callback runs once without recursion");
	if ( tState.bContextDestroyed ) goto cleanup;
	CHECK(&tState, xuiDragAccept(tState.pContext, XUI_DRAG_EFFECT_COPY) ==
		XUI_ERROR_INVALID_STATE, "negotiation does not escape event scope");
	bReplacement = pCase->iAction == ACTION_REPLACE ||
		pCase->iAction == ACTION_RESTART_DETACHING || pCase->iAction == ACTION_FREE_REPLACE ||
		pCase->iAction == ACTION_REPLACE_ERROR;
	if ( bReplacement ) {
		CHECK(&tState, xuiDragIsActive(tState.pContext), "replacement remains active");
		CHECK(&tState, tState.pContext->pTransferData == tState.pNew &&
			tState.pContext->pDropTarget == tState.pTargets[2], "old operation cannot overwrite replacement");
		CHECK(&tState, xuiDragGetEffect(tState.pContext) == XUI_DRAG_EFFECT_COPY,
			"replacement effect retained");
		CHECK(&tState, tState.iNewEnter == 1, "replacement gets exactly one ENTER");
	} else if ( pCase->iAction == ACTION_MOVE ) {
		CHECK(&tState, tState.pContext->pDropTarget == tState.pTargets[2],
			"nested move owns final target");
		CHECK(&tState, xuiDragGetEffect(tState.pContext) == XUI_DRAG_EFFECT_COPY,
			"nested move owns effect");
		CHECK(&tState, xuiInternalDragTransferDrop(tState.pContext, 350, 30, 0u) == XUI_OK &&
			!xuiDragIsActive(tState.pContext), "superseding move does not wedge drop guard");
	} else if ( pCase->iAction == ACTION_ERROR ) {
		CHECK(&tState, xuiDragIsActive(tState.pContext), "error preserves retryable session");
		CHECK(&tState, xuiInternalDragTransferDrop(tState.pContext, 110, 30, 0u) == XUI_OK &&
			!xuiDragIsActive(tState.pContext), "error does not wedge drop guard");
	} else if ( pCase->iAction == ACTION_NONE ) {
		CHECK(&tState, xuiDragIsActive(tState.pContext) && tState.iOldFree == 0 &&
			tState.pContext->pTransferData == tState.pOld, "replacement pins borrowed session payload");
	} else if ( pCase->iAction == ACTION_DESTROY_NEXT || pCase->iAction == ACTION_MOVE_NEXT ) {
		CHECK(&tState, tState.iOldEvents[1][0] == 0 && tState.pContext->pDropTarget == NULL,
			"LEAVE recomputes candidate after destruction or movement");
	} else {
		CHECK(&tState, !xuiDragIsActive(tState.pContext), "old session ended");
	}
	if ( pCase->iEvent == XUI_EVENT_DRAG_LEAVE && pCase->iAction != ACTION_MOVE ) {
		CHECK(&tState, tState.iOldEvents[1][0] == 0, "no stale ENTER after LEAVE");
	}
	if ( pCase->iAction == ACTION_DROP ) {
		CHECK(&tState, tState.iOldEvents[0][3] == 1, "drop delivered exactly once");
		CHECK(&tState, xuiDragGetEffect(tState.pContext) == XUI_DRAG_EFFECT_COPY,
			"drop effect retained");
	}
cleanup:
	if ( tState.pContext != NULL && !tState.bContextDestroyed ) xuiDestroy(tState.pContext);
	if ( tState.bCallerOwnsOld ) xuiDataObjectRelease(tState.pOld);
	if ( tState.pNew != NULL ) xuiDataObjectRelease(tState.pNew);
	CHECK(&tState, tState.iOldFree == 1 && tState.iNewFree == 1, "data freed exactly once");
	if ( !tState.iFailed ) printf("PASS %s\n", pCase->sName);
	return tState.iFailed;
}

int main(int argc, char** argv)
{
	static const test_case_t arrCases[] = {
		{"leave_cancel", XUI_EVENT_DRAG_LEAVE, ACTION_CANCEL, CALL_MOVE, 0},
		{"cancel_leave_cancel", XUI_EVENT_DRAG_LEAVE, ACTION_CANCEL, CALL_CANCEL, 0},
		{"leave_replace", XUI_EVENT_DRAG_LEAVE, ACTION_REPLACE, CALL_MOVE, 0},
		{"cancel_leave_replace", XUI_EVENT_DRAG_LEAVE, ACTION_REPLACE, CALL_CANCEL, 0},
		{"external_replace_reentry", XUI_EVENT_DRAG_LEAVE, ACTION_REPLACE, CALL_REPLACE, 0},
		{"leave_destroy_next", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_NEXT, CALL_MOVE, 0},
		{"leave_move_next", XUI_EVENT_DRAG_LEAVE, ACTION_MOVE_NEXT, CALL_MOVE, 0},
		{"leave_destroy_source", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_SOURCE, CALL_MOVE, 1},
		{"enter_destroy_source", XUI_EVENT_DRAG_ENTER, ACTION_DESTROY_SOURCE, CALL_BEGIN, 1},
		{"over_destroy_target", XUI_EVENT_DRAG_OVER, ACTION_DESTROY_TARGET, CALL_DROP, 0},
		{"leave_destroy_root", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_ROOT, CALL_MOVE, 1},
		{"leave_destroy_context", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_CONTEXT, CALL_MOVE, 0},
		{"cancel_destroy_context", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_CONTEXT, CALL_CANCEL, 0},
		{"begin_destroy_context", XUI_EVENT_DRAG_ENTER, ACTION_DESTROY_CONTEXT, CALL_BEGIN, 1},
		{"external_destroy_context", XUI_EVENT_DRAG_ENTER, ACTION_DESTROY_CONTEXT, CALL_BEGIN, 0},
		{"drop_destroy_context", XUI_EVENT_DROP, ACTION_DESTROY_CONTEXT, CALL_DROP, 0},
		{"drop_cancel", XUI_EVENT_DROP, ACTION_CANCEL, CALL_DROP, 0},
		{"drop_replace", XUI_EVENT_DROP, ACTION_REPLACE, CALL_DROP, 0},
		{"drop_destroy_source", XUI_EVENT_DROP, ACTION_DESTROY_SOURCE, CALL_DROP, 1},
		{"drop_destroy_target", XUI_EVENT_DROP, ACTION_DESTROY_TARGET, CALL_DROP, 0},
		{"drop_recursive", XUI_EVENT_DROP, ACTION_DROP, CALL_DROP, 0},
		{"predrop_recursive", XUI_EVENT_DRAG_OVER, ACTION_DROP, CALL_DROP, 0},
		{"predrop_replace", XUI_EVENT_DRAG_OVER, ACTION_REPLACE, CALL_DROP, 0},
		{"leave_nested_move", XUI_EVENT_DRAG_LEAVE, ACTION_MOVE, CALL_MOVE, 0},
		{"over_nested_move", XUI_EVENT_DRAG_OVER, ACTION_MOVE, CALL_MOVE, 0},
		{"source_detach_restart", XUI_EVENT_DRAG_LEAVE, ACTION_RESTART_DETACHING, CALL_CANCEL, 1},
		{"enter_cancel", XUI_EVENT_DRAG_ENTER, ACTION_CANCEL, CALL_BEGIN, 0},
		{"enter_replace", XUI_EVENT_DRAG_ENTER, ACTION_REPLACE, CALL_BEGIN, 1},
		{"external_leave_cancel", XUI_EVENT_DRAG_LEAVE, ACTION_CANCEL, CALL_EXTERNAL_OVER, 0},
		{"external_cancel_destroy_context", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_CONTEXT, CALL_EXTERNAL_CANCEL, 0},
		{"external_over_destroy_context", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_CONTEXT, CALL_EXTERNAL_OVER, 0},
		{"external_drop_destroy_context", XUI_EVENT_DROP, ACTION_DESTROY_CONTEXT, CALL_EXTERNAL_DROP, 0},
		{"external_drop_replace", XUI_EVENT_DROP, ACTION_REPLACE, CALL_EXTERNAL_DROP, 0},
		{"external_replace_destroy_context", XUI_EVENT_DRAG_LEAVE, ACTION_DESTROY_CONTEXT, CALL_REPLACE, 0},
		{"borrowed_external_replace", 0, ACTION_NONE, CALL_REPLACE, 0},
		{"drop_nested_move", XUI_EVENT_DROP, ACTION_MOVE, CALL_DROP, 0},
		{"predrop_nested_move", XUI_EVENT_DRAG_OVER, ACTION_MOVE, CALL_DROP, 0},
		{"leave_error", XUI_EVENT_DRAG_LEAVE, ACTION_ERROR, CALL_MOVE, 0},
		{"drop_error", XUI_EVENT_DROP, ACTION_ERROR, CALL_DROP, 0},
		{"predrop_error", XUI_EVENT_DRAG_OVER, ACTION_ERROR, CALL_DROP, 0},
		{"leave_replace_error", XUI_EVENT_DRAG_LEAVE, ACTION_REPLACE_ERROR, CALL_MOVE, 0},
		{"drop_replace_error", XUI_EVENT_DROP, ACTION_REPLACE_ERROR, CALL_DROP, 0},
		{"cancel_free_replace", 0, ACTION_FREE_REPLACE, CALL_CANCEL, 0},
		{"drop_free_replace", 0, ACTION_FREE_REPLACE, CALL_DROP, 0},
		{"cancel_free_destroy", 0, ACTION_FREE_DESTROY, CALL_CANCEL, 0},
		{"drop_free_destroy", 0, ACTION_FREE_DESTROY, CALL_DROP, 0},
		{"shutdown_free_reentry", 0, ACTION_FREE_SHUTDOWN, CALL_SHUTDOWN, 0}
	};
	size_t i;
	int iFailed = 0;
	int iRun = 0;
	for ( i = 0; i < sizeof(arrCases) / sizeof(arrCases[0]); i++ ) {
		if ( argc > 1 && strcmp(argv[1], arrCases[i].sName) != 0 ) continue;
		iRun++;
		iFailed += run_case(&arrCases[i]);
	}
	printf("drag/drop reentry: %d passed, %d failed\n", iRun - iFailed, iFailed);
	return iFailed != 0 || iRun == 0;
}
