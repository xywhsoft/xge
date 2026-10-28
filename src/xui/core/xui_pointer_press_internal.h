#ifndef XUI_POINTER_PRESS_INTERNAL_H
#define XUI_POINTER_PRESS_INTERNAL_H

#include "xui.h"

#include <string.h>

typedef struct xui_pointer_owner_t {
	int bActive;
	uint64_t iPointerId;
	int iPointerType;
} xui_pointer_owner_t;

typedef struct xui_pointer_press_t {
	xui_pointer_owner_t tOwner;
	int bClickPending;
	uint64_t iClickPointerId;
	int iClickPointerType;
} xui_pointer_press_t;

static int xuiInternalPointerOwnerMatches(const xui_pointer_owner_t* pOwner,
	const xui_event_t* pEvent)
{
	return (pOwner != NULL) && (pEvent != NULL) && pOwner->bActive &&
		(pOwner->iPointerId == pEvent->iPointerId) &&
		(pOwner->iPointerType == pEvent->iPointerType);
}

static int xuiInternalPointerOwnerBegin(xui_context pContext, xui_widget pCapture,
	const xui_event_t* pEvent, xui_pointer_owner_t* pOwner)
{
	int iRet;

	if ( (pContext == NULL) || (pCapture == NULL) || (pEvent == NULL) ||
	     (pOwner == NULL) || pOwner->bActive ) {
		return XUI_ERROR_INVALID_STATE;
	}
	iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId,
		pEvent->iPointerType, pCapture);
	if ( iRet != XUI_OK ) return iRet;
	pOwner->bActive = 1;
	pOwner->iPointerId = pEvent->iPointerId;
	pOwner->iPointerType = pEvent->iPointerType;
	return XUI_OK;
}

static int xuiInternalPointerOwnerEnd(xui_context pContext, xui_widget pCapture,
	const xui_event_t* pEvent, xui_pointer_owner_t* pOwner)
{
	uint64_t iPointerId;
	int iPointerType;

	if ( !xuiInternalPointerOwnerMatches(pOwner, pEvent) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	iPointerId = pOwner->iPointerId;
	iPointerType = pOwner->iPointerType;
	pOwner->bActive = 0;
	pOwner->iPointerId = 0;
	pOwner->iPointerType = XUI_POINTER_TYPE_MOUSE;
	if ( xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pCapture ) {
		return xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pCapture);
	}
	return XUI_OK;
}

static int xuiInternalPointerOwnerCancel(xui_context pContext, xui_widget pCapture,
	xui_pointer_owner_t* pOwner)
{
	uint64_t iPointerId;
	int iPointerType;
	int bActive;

	if ( pOwner == NULL ) return XUI_OK;
	iPointerId = pOwner->iPointerId;
	iPointerType = pOwner->iPointerType;
	bActive = pOwner->bActive;
	memset(pOwner, 0, sizeof(*pOwner));
	pOwner->iPointerType = XUI_POINTER_TYPE_MOUSE;
	if ( bActive && (pContext != NULL) && (pCapture != NULL) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pCapture ) {
		return xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pCapture);
	}
	return XUI_OK;
}

static int xuiInternalPointerOwnerCaptureLost(xui_pointer_owner_t* pOwner,
	const xui_event_t* pEvent)
{
	if ( !xuiInternalPointerOwnerMatches(pOwner, pEvent) ) return 0;
	pOwner->bActive = 0;
	pOwner->iPointerId = 0;
	pOwner->iPointerType = XUI_POINTER_TYPE_MOUSE;
	return 1;
}

static int xuiInternalPointerPressMatches(const xui_pointer_press_t* pPress,
	const xui_event_t* pEvent)
{
	return (pPress != NULL) &&
		xuiInternalPointerOwnerMatches(&pPress->tOwner, pEvent);
}

static int xuiInternalPointerPressBegin(xui_context pContext, xui_widget pWidget,
	const xui_event_t* pEvent, xui_pointer_press_t* pPress)
{
	int iRet;

	if ( (pContext == NULL) || (pWidget == NULL) || (pEvent == NULL) ||
	     (pPress == NULL) || pPress->tOwner.bActive ) {
		return XUI_ERROR_INVALID_STATE;
	}
	iRet = xuiInternalPointerOwnerBegin(pContext, pWidget, pEvent,
		&pPress->tOwner);
	if ( iRet != XUI_OK ) return iRet;
	pPress->bClickPending = 0;
	return XUI_OK;
}

static int xuiInternalPointerPressEnd(xui_context pContext, xui_widget pWidget,
	const xui_event_t* pEvent, xui_pointer_press_t* pPress)
{
	uint64_t iPointerId;
	int iPointerType;
	int iRet;

	if ( !xuiInternalPointerPressMatches(pPress, pEvent) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	iPointerId = pPress->tOwner.iPointerId;
	iPointerType = pPress->tOwner.iPointerType;
	iRet = xuiInternalPointerOwnerEnd(pContext, pWidget, pEvent,
		&pPress->tOwner);
	if ( iRet != XUI_OK ) return iRet;
	pPress->bClickPending = 1;
	pPress->iClickPointerId = iPointerId;
	pPress->iClickPointerType = iPointerType;
	return XUI_OK;
}

static int xuiInternalPointerPressCancel(xui_context pContext, xui_widget pWidget,
	xui_pointer_press_t* pPress)
{
	int iRet;

	if ( pPress == NULL ) return XUI_OK;
	iRet = xuiInternalPointerOwnerCancel(pContext, pWidget, &pPress->tOwner);
	pPress->bClickPending = 0;
	pPress->iClickPointerId = 0;
	pPress->iClickPointerType = XUI_POINTER_TYPE_MOUSE;
	return iRet;
}

static int xuiInternalPointerPressCaptureLost(xui_pointer_press_t* pPress,
	const xui_event_t* pEvent)
{
	if ( (pPress == NULL) ||
	     !xuiInternalPointerOwnerCaptureLost(&pPress->tOwner, pEvent) ) return 0;
	pPress->bClickPending = 0;
	return 1;
}

static int xuiInternalPointerPressAcceptClick(xui_pointer_press_t* pPress,
	const xui_event_t* pEvent)
{
	if ( (pPress == NULL) || (pEvent == NULL) ) return 0;
	if ( pPress->tOwner.bActive ) return 0;
	if ( !pPress->bClickPending ) return 1;
	if ( (pPress->iClickPointerId != pEvent->iPointerId) ||
	     (pPress->iClickPointerType != pEvent->iPointerType) ) {
		return 0;
	}
	pPress->bClickPending = 0;
	return 1;
}

#endif
