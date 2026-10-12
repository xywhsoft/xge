#include "xui.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr, message) do { \
	if ( !(expr) ) { \
		printf("xui_drag_drop_test failed: %s\n", message); \
		iFailed = 1; \
		goto cleanup; \
	} \
} while ( 0 )

typedef struct drag_log_t {
	xui_context pContext;
	xui_data_object pData;
	int iEnter;
	int iOver;
	int iLeave;
	int iDrop;
	int iSourceBegin;
	int iSourceEnd;
	char sText[32];
} drag_log_t;

static int __sourceEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	drag_log_t* pLog = (drag_log_t*)pUser;

	if ( pEvent->iType == XUI_EVENT_DRAG_BEGIN ) {
		pLog->iSourceBegin++;
		return xuiDragBegin(pLog->pContext, pWidget, pLog->pData,
			XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE,
			XUI_DRAG_EFFECT_COPY);
	}
	if ( pEvent->iType == XUI_EVENT_DRAG_END ) {
		pLog->iSourceEnd++;
	}
	return XUI_OK;
}

static int __targetEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	drag_log_t* pLog = (drag_log_t*)pUser;
	xui_drag_event_data_t* pDrag;
	size_t iSize = 0u;

	(void)pWidget;
	pDrag = (xui_drag_event_data_t*)pEvent->pData;
	if ( pDrag == NULL || pDrag->iSize < sizeof(*pDrag) ||
		pDrag->pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType == XUI_EVENT_DRAG_ENTER ) pLog->iEnter++;
	if ( pEvent->iType == XUI_EVENT_DRAG_OVER ) pLog->iOver++;
	if ( pEvent->iType == XUI_EVENT_DRAG_LEAVE ) {
		pLog->iLeave++;
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_DROP ) {
		pLog->iDrop++;
		if ( xuiDataObjectGet(pDrag->pData, XUI_DATA_FORMAT_TEXT_UTF8,
			pLog->sText, sizeof(pLog->sText), &iSize) != XUI_OK ) {
			return XUI_ERROR;
		}
	}
	return xuiDragAccept(pLog->pContext, XUI_DRAG_EFFECT_COPY);
}

static int __dispatch(xui_context pContext)
{
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	drag_log_t tLog;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pSource = NULL;
	xui_widget pTarget = NULL;
	xui_data_object pExternal = NULL;
	uint32_t iEffect = 0u;
	const char sPayload[] = "drag payload";
	int iFailed = 0;
	int iRet;

	memset(&tLog, 0, sizeof(tLog));
	CHECK(xuiCreate(&pContext) == XUI_OK, "create context");
	tLog.pContext = pContext;
	CHECK(xuiInputViewport(pContext, 240, 120) == XUI_OK, "viewport");
	CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "create root");
	CHECK(xuiWidgetCreate(pContext, &pSource) == XUI_OK, "create source");
	CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "create target");
	CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "set root");
	CHECK(xuiWidgetSetRect(pSource, (xui_rect_t){10, 10, 60, 50}) == XUI_OK,
		"source rect");
	CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){130, 10, 80, 70}) == XUI_OK,
		"target rect");
	CHECK(xuiWidgetAddChild(pRoot, pSource) == XUI_OK, "add source");
	CHECK(xuiWidgetAddChild(pRoot, pTarget) == XUI_OK, "add target");
	CHECK(xuiWidgetSetDragEnabled(pSource, 1) == XUI_OK, "enable drag");
	CHECK(xuiWidgetSetDropEnabled(pTarget, 1) == XUI_OK, "enable drop");
	CHECK(xuiWidgetSetEventCallback(pSource, __sourceEvent, &tLog) == XUI_OK,
		"source handler");
	CHECK(xuiWidgetSetEventCallback(pTarget, __targetEvent, &tLog) == XUI_OK,
		"target handler");
	CHECK(xuiDataObjectCreate(&tLog.pData) == XUI_OK, "create data");
	CHECK(xuiDataObjectSet(tLog.pData, XUI_DATA_FORMAT_TEXT_UTF8,
		sPayload, sizeof(sPayload)) == XUI_OK, "set data");
	xuiClearEvents(pContext);

	CHECK(xuiInputPointerDown(pContext, 20, 20, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK,
		"pointer down");
	CHECK(__dispatch(pContext) == XUI_OK, "dispatch down");
	CHECK(xuiInputPointerMove(pContext, 145, 25, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
		"drag move begin");
	CHECK(__dispatch(pContext) == XUI_OK, "dispatch drag begin");
	CHECK(tLog.iSourceBegin == 1 && tLog.iEnter == 1,
		"source begin and target enter");
	CHECK(xuiInputPointerMove(pContext, 160, 30, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
		"drag over");
	CHECK(__dispatch(pContext) == XUI_OK, "dispatch over");
	CHECK(tLog.iOver >= 1, "target over");
	CHECK(xuiInputPointerUp(pContext, 160, 30, XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT) == XUI_OK, "pointer up");
	CHECK(__dispatch(pContext) == XUI_OK, "dispatch drop");
	CHECK(tLog.iDrop == 1 && tLog.iSourceEnd == 1, "drop and source end");
	CHECK(strcmp(tLog.sText, sPayload) == 0, "drop payload");
	CHECK(xuiDragGetEffect(pContext) == XUI_DRAG_EFFECT_COPY, "drop effect");
	CHECK(!xuiDragIsActive(pContext), "drag ended");

	CHECK(xuiDataObjectCreate(&pExternal) == XUI_OK, "external data");
	CHECK(xuiDataObjectSet(pExternal, XUI_DATA_FORMAT_TEXT_UTF8,
		sPayload, sizeof(sPayload)) == XUI_OK, "external payload");
	iRet = xuiDragExternalEvent(pContext, XUI_DRAG_EXTERNAL_ENTER,
		150, 30, 0u, pExternal, XUI_DRAG_EFFECT_COPY,
		XUI_DRAG_EFFECT_COPY, &iEffect);
	CHECK(iRet == XUI_OK && iEffect == XUI_DRAG_EFFECT_COPY,
		"external enter acceptance");
	iRet = xuiDragExternalEvent(pContext, XUI_DRAG_EXTERNAL_DROP,
		150, 30, 0u, NULL, 0u, 0u, &iEffect);
	CHECK(iRet == XUI_OK && iEffect == XUI_DRAG_EFFECT_COPY,
		"external drop acceptance");
	CHECK(tLog.iDrop == 2, "external drop delivered");

cleanup:
	if ( pExternal != NULL ) xuiDataObjectRelease(pExternal);
	if ( tLog.pData != NULL ) xuiDataObjectRelease(tLog.pData);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( !iFailed ) printf("xui_drag_drop_test passed\n");
	return iFailed ? 1 : 0;
}
