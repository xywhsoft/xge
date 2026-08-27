#include "xui_internal.h"

#include <string.h>

static xui_widget __xuiEditTarget(xui_widget pWidget)
{
	int iDepth;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	for ( iDepth = 0; iDepth < 8 && pWidget->pEditDelegate != NULL; iDepth++ ) {
		if ( !xuiInternalWidgetIsValid(pWidget->pEditDelegate) || pWidget->pEditDelegate == pWidget ) return NULL;
		pWidget = pWidget->pEditDelegate;
	}
	return (iDepth < 8 && pWidget->pEditAdapter != NULL) ? pWidget : NULL;
}

static int __xuiEditBehaviorValid(const xui_edit_behavior_t* pBehavior)
{
	if ( pBehavior == NULL || pBehavior->iSize < sizeof(*pBehavior) ) return 0;
	if ( pBehavior->iTabBehavior < XUI_EDIT_TAB_DEFAULT || pBehavior->iTabBehavior > XUI_EDIT_TAB_INDENT ) return 0;
	if ( pBehavior->iEnterBehavior < XUI_EDIT_ENTER_DEFAULT || pBehavior->iEnterBehavior > XUI_EDIT_ENTER_NEWLINE ) return 0;
	if ( pBehavior->iEscapeBehavior < XUI_EDIT_ESCAPE_DEFAULT || pBehavior->iEscapeBehavior > XUI_EDIT_ESCAPE_REVERT ) return 0;
	return 1;
}

int xuiInternalEditRegister(xui_widget pWidget, const xui_internal_edit_adapter_t* pAdapter,
	const xui_edit_behavior_t* pBehavior)
{
	if ( !xuiInternalWidgetIsValid(pWidget) || pAdapter == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBehavior != NULL && !__xuiEditBehaviorValid(pBehavior) ) return XUI_ERROR_INVALID_ARGUMENT;
	xuiInternalEditUndelegate(pWidget);
	pWidget->pEditAdapter = pAdapter;
	memset(&pWidget->tEditBehavior, 0, sizeof(pWidget->tEditBehavior));
	pWidget->tEditBehavior.iSize = sizeof(pWidget->tEditBehavior);
	if ( pBehavior != NULL ) {
		pWidget->tEditBehavior = *pBehavior;
		pWidget->tEditBehavior.iSize = sizeof(pWidget->tEditBehavior);
	}
	return XUI_OK;
}

int xuiInternalEditDelegate(xui_widget pWidget, xui_widget pDelegate)
{
	xui_widget pCursor;
	xui_widget pTarget;
	int iDepth;

	if ( !xuiInternalWidgetIsValid(pWidget) || !xuiInternalWidgetIsValid(pDelegate) || pWidget == pDelegate ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pCursor = pDelegate;
	for ( iDepth = 0; iDepth < 8 && pCursor != NULL; iDepth++ ) {
		if ( pCursor == pWidget ) return XUI_ERROR_INVALID_ARGUMENT;
		pCursor = pCursor->pEditDelegate;
	}
	pTarget = __xuiEditTarget(pDelegate);
	if ( pTarget == NULL ) return XUI_ERROR_UNSUPPORTED;
	if ( xuiInternalWidgetIsValid(pWidget->pEditDelegate) &&
	     pWidget->pEditDelegate->pEditOwner == pWidget ) {
		pWidget->pEditDelegate->pEditOwner = NULL;
	}
	if ( xuiInternalWidgetIsValid(pDelegate->pEditOwner) &&
	     pDelegate->pEditOwner != pWidget ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	pWidget->pEditDelegate = pDelegate;
	pDelegate->pEditOwner = pWidget;
	pWidget->tEditBehavior = pDelegate->tEditBehavior;
	pWidget->tEditBehavior.iSize = sizeof(pWidget->tEditBehavior);
	return XUI_OK;
}

void xuiInternalEditUndelegate(xui_widget pWidget)
{
	if ( !xuiInternalWidgetIsValid(pWidget) ) return;
	if ( xuiInternalWidgetIsValid(pWidget->pEditDelegate) &&
	     pWidget->pEditDelegate->pEditOwner == pWidget ) {
		pWidget->pEditDelegate->pEditOwner = NULL;
	}
	pWidget->pEditDelegate = NULL;
}

xui_widget xuiInternalEditHost(xui_widget pWidget)
{
	int iDepth;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	for ( iDepth = 0; iDepth < 8 && xuiInternalWidgetIsValid(pWidget->pEditOwner); iDepth++ ) {
		if ( pWidget->pEditOwner == pWidget ) return NULL;
		pWidget = pWidget->pEditOwner;
	}
	if ( iDepth >= 8 && xuiInternalWidgetIsValid(pWidget->pEditOwner) ) return NULL;
	return pWidget;
}

const xui_edit_behavior_t* xuiInternalEditBehavior(xui_widget pWidget)
{
	pWidget = xuiInternalEditHost(pWidget);
	return (pWidget != NULL) ? &pWidget->tEditBehavior : NULL;
}

int xuiInternalEditEmit(xui_widget pWidget, int iType, const char* sText,
	int iSelectionStart, int iSelectionEnd, int iCompositionStart, int iCompositionEnd, int bValid)
{
	return xuiInternalEditEmitSized(pWidget, iType, sText,
		(sText != NULL) ? (int)strlen(sText) : 0, iSelectionStart, iSelectionEnd,
		iCompositionStart, iCompositionEnd, bValid);
}

int xuiInternalEditEmitSized(xui_widget pWidget, int iType, const char* sText, int iTextSize,
	int iSelectionStart, int iSelectionEnd, int iCompositionStart, int iCompositionEnd, int bValid)
{
	xui_edit_event_t tEvent;
	xui_widget pHost;

	pHost = xuiInternalEditHost(pWidget);
	if ( pHost == NULL || pHost->onEditEvent == NULL ) return XUI_OK;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.sText = sText;
	tEvent.iTextSize = (iTextSize >= 0) ? iTextSize : 0;
	tEvent.iSelectionStart = iSelectionStart;
	tEvent.iSelectionEnd = iSelectionEnd;
	tEvent.iCompositionStart = iCompositionStart;
	tEvent.iCompositionEnd = iCompositionEnd;
	tEvent.bValid = bValid ? 1 : 0;
	return pHost->onEditEvent(pHost, &tEvent, pHost->pEditEventUser);
}

XUI_API uint32_t xuiEditGetCapabilities(xui_widget pWidget)
{
	pWidget = __xuiEditTarget(pWidget);
	return (pWidget != NULL) ? pWidget->pEditAdapter->iCapabilities : 0u;
}

XUI_API int xuiEditSetEvent(xui_widget pWidget, xui_edit_event_proc onEvent, void* pUser)
{
	if ( __xuiEditTarget(pWidget) == NULL ) return XUI_ERROR_UNSUPPORTED;
	pWidget->onEditEvent = onEvent;
	pWidget->pEditEventUser = pUser;
	return XUI_OK;
}

XUI_API int xuiEditSetBehavior(xui_widget pWidget, const xui_edit_behavior_t* pBehavior)
{
	if ( __xuiEditTarget(pWidget) == NULL ) return XUI_ERROR_UNSUPPORTED;
	if ( !__xuiEditBehaviorValid(pBehavior) ) return XUI_ERROR_INVALID_ARGUMENT;
	pWidget->tEditBehavior = *pBehavior;
	pWidget->tEditBehavior.iSize = sizeof(pWidget->tEditBehavior);
	return XUI_OK;
}

XUI_API int xuiEditGetBehavior(xui_widget pWidget, xui_edit_behavior_t* pBehavior)
{
	if ( __xuiEditTarget(pWidget) == NULL || pBehavior == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pBehavior = pWidget->tEditBehavior;
	pBehavior->iSize = sizeof(*pBehavior);
	return XUI_OK;
}

#define XUI_EDIT_CALL0(member) do { \
	xui_widget pTarget = __xuiEditTarget(pWidget); \
	if ( pTarget == NULL || pTarget->pEditAdapter->member == NULL ) return XUI_ERROR_UNSUPPORTED; \
	return pTarget->pEditAdapter->member(pTarget); \
} while (0)

XUI_API int xuiEditSetText(xui_widget pWidget, const char* sText)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget == NULL || pTarget->pEditAdapter->setText == NULL ) return XUI_ERROR_UNSUPPORTED;
	return pTarget->pEditAdapter->setText(pTarget, sText);
}

XUI_API const char* xuiEditGetText(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	return (pTarget != NULL && pTarget->pEditAdapter->getText != NULL) ? pTarget->pEditAdapter->getText(pTarget) : NULL;
}

XUI_API int xuiEditSetSelection(xui_widget pWidget, int iStart, int iEnd)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget == NULL || pTarget->pEditAdapter->setSelection == NULL ) return XUI_ERROR_UNSUPPORTED;
	return pTarget->pEditAdapter->setSelection(pTarget, iStart, iEnd);
}

XUI_API int xuiEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget == NULL || pTarget->pEditAdapter->getSelection == NULL ) return XUI_ERROR_UNSUPPORTED;
	return pTarget->pEditAdapter->getSelection(pTarget, pStart, pEnd);
}

XUI_API int xuiEditHasSelection(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	return (pTarget != NULL && pTarget->pEditAdapter->hasSelection != NULL) ? pTarget->pEditAdapter->hasSelection(pTarget) : 0;
}

XUI_API int xuiEditSelectAll(xui_widget pWidget) { XUI_EDIT_CALL0(selectAll); }
XUI_API int xuiEditCopy(xui_widget pWidget) { XUI_EDIT_CALL0(copy); }
XUI_API int xuiEditCut(xui_widget pWidget) { XUI_EDIT_CALL0(cut); }
XUI_API int xuiEditPaste(xui_widget pWidget) { XUI_EDIT_CALL0(paste); }
XUI_API int xuiEditDeleteSelection(xui_widget pWidget) { XUI_EDIT_CALL0(deleteSelection); }
XUI_API int xuiEditUndo(xui_widget pWidget) { XUI_EDIT_CALL0(undo); }
XUI_API int xuiEditRedo(xui_widget pWidget) { XUI_EDIT_CALL0(redo); }

XUI_API int xuiEditCanUndo(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	return (pTarget != NULL && pTarget->pEditAdapter->canUndo != NULL) ? pTarget->pEditAdapter->canUndo(pTarget) : 0;
}

XUI_API int xuiEditCanRedo(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	return (pTarget != NULL && pTarget->pEditAdapter->canRedo != NULL) ? pTarget->pEditAdapter->canRedo(pTarget) : 0;
}

XUI_API int xuiEditSetReadonly(xui_widget pWidget, int bReadonly)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget == NULL || pTarget->pEditAdapter->setReadonly == NULL ) return XUI_ERROR_UNSUPPORTED;
	return pTarget->pEditAdapter->setReadonly(pTarget, bReadonly);
}

XUI_API int xuiEditIsReadonly(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	return (pTarget != NULL && pTarget->pEditAdapter->isReadonly != NULL) ? pTarget->pEditAdapter->isReadonly(pTarget) : 0;
}

XUI_API xui_rect_t xuiEditGetCaretRect(xui_widget pWidget)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget != NULL && pTarget->pEditAdapter->getCaretRect != NULL ) return pTarget->pEditAdapter->getCaretRect(pTarget);
	return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiEditOpenContextMenu(xui_widget pWidget, float fX, float fY)
{
	xui_widget pTarget = __xuiEditTarget(pWidget);
	if ( pTarget == NULL || pTarget->pEditAdapter->openContextMenu == NULL ) return XUI_ERROR_UNSUPPORTED;
	return pTarget->pEditAdapter->openContextMenu(pTarget, fX, fY);
}

#undef XUI_EDIT_CALL0
