#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

typedef struct proxy_drop_log_t {
	xui_context pContext;
	int iEnter;
	int iDrop;
	char sText[64];
} proxy_drop_log_t;

static int __dropEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	proxy_drop_log_t* pLog = (proxy_drop_log_t*)pUser;
	xui_drag_event_data_t* pDrag = (xui_drag_event_data_t*)pEvent->pData;
	size_t iSize = 0u;

	(void)pWidget;
	if ( pDrag == NULL || !pDrag->bExternal ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType == XUI_EVENT_DRAG_ENTER ) pLog->iEnter++;
	if ( pEvent->iType == XUI_EVENT_DROP ) {
		pLog->iDrop++;
		if ( xuiDataObjectGet(pDrag->pData, XUI_DATA_FORMAT_TEXT_UTF8,
			pLog->sText, sizeof(pLog->sText), &iSize) != XUI_OK ) {
			return XUI_ERROR;
		}
	}
	if ( pEvent->iType == XUI_EVENT_DRAG_ENTER ||
		pEvent->iType == XUI_EVENT_DRAG_OVER ||
		pEvent->iType == XUI_EVENT_DROP ) {
		return xuiDragAccept(pLog->pContext, XUI_DRAG_EFFECT_COPY);
	}
	return XUI_OK;
}

int main(void)
{
	xge_desc_t tDesc;
	xge_drag_event_t tDrag;
	xge_data_object pXgeData = NULL;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pTarget = NULL;
	proxy_drop_log_t tLog;
	const char sText[] = "native bridge payload";
	uint32_t iEffect;
	int iFailed = 0;

#define CHECK(expr, message) do { if (!(expr)) { \
	printf("xui_proxy_drag_drop_test failed: %s\n", message); \
	iFailed = 1; goto cleanup; } } while (0)

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 200;
	tDesc.iHeight = 100;
	tDesc.iFlags = XGE_INIT_OFFSCREEN;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	CHECK(xgeInit(&tDesc) == XGE_OK, "xge init");
	CHECK(xuiCreate(&pContext) == XUI_OK, "xui create");
	memset(&tLog, 0, sizeof(tLog));
	tLog.pContext = pContext;
	CHECK(xuiInputViewport(pContext, 200, 100) == XUI_OK, "viewport");
	CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "root");
	CHECK(xuiWidgetCreate(pContext, &pTarget) == XUI_OK, "target");
	CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "set root");
	CHECK(xuiWidgetSetRect(pTarget, (xui_rect_t){100, 10, 90, 80}) == XUI_OK,
		"target rect");
	CHECK(xuiWidgetAddChild(pRoot, pTarget) == XUI_OK, "add target");
	CHECK(xuiWidgetSetDropEnabled(pTarget, 1) == XUI_OK, "drop enabled");
	CHECK(xuiWidgetSetEventCallback(pTarget, __dropEvent, &tLog) == XUI_OK,
		"drop handler");
	CHECK(xuiProxyXgePumpInputRect(pContext,
		(xui_rect_t){0, 0, 200, 100}) == XUI_OK, "attach bridge");
	CHECK(xgeDataObjectCreate(&pXgeData) == XGE_OK, "xge data create");
	CHECK(xgeDataObjectSet(pXgeData, XGE_DATA_FORMAT_TEXT_UTF8,
		sText, sizeof(sText)) == XGE_OK, "xge data set");
	memset(&tDrag, 0, sizeof(tDrag));
	tDrag.iSize = sizeof(tDrag);
	tDrag.iType = XGE_DRAG_EVENT_ENTER;
	tDrag.fX = 140;
	tDrag.fY = 30;
	tDrag.iAllowedEffects = XGE_DRAG_EFFECT_COPY;
	tDrag.iSuggestedEffect = XGE_DRAG_EFFECT_COPY;
	tDrag.pData = pXgeData;
	iEffect = xgeDragEventDispatch(&tDrag);
	CHECK(iEffect == XGE_DRAG_EFFECT_COPY && tLog.iEnter == 1,
		"native enter bridge");
	tDrag.iType = XGE_DRAG_EVENT_DROP;
	iEffect = xgeDragEventDispatch(&tDrag);
	CHECK(iEffect == XGE_DRAG_EFFECT_COPY && tLog.iDrop == 1,
		"native drop bridge");
	CHECK(strcmp(tLog.sText, sText) == 0, "native payload bridge");

cleanup:
	if ( pXgeData != NULL ) xgeDataObjectRelease(pXgeData);
	if ( pContext != NULL ) xuiDestroy(pContext);
	xgeUnit();
	if ( !iFailed ) printf("xui_proxy_drag_drop_test passed\n");
	return iFailed ? 1 : 0;
}
