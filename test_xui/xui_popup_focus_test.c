#include "src/xui_internal.h"

#include <stdio.h>
#include <string.h>

typedef struct focus_test_t {
	const char* sName;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pParent;
	xui_widget pOwner;
	xui_widget pBackground;
	xui_widget pPopup;
	xui_widget pChild[2];
	xui_widget pNested;
	int iFailed;
	int iCallbacks;
	int iBackgroundKeys;
	int bDestroyed;
} focus_test_t;

#define CHECK(s, expr, message) do { \
	if ( !(expr) ) { \
		printf("FAIL %s: %s (line %d)\n", (s)->sName, message, __LINE__); \
		(s)->iFailed = 1; \
	} \
} while ( 0 )

static xui_widget focus_child(xui_context pContext, xui_widget pParent, int iX)
{
	xui_widget pWidget = NULL;
	if ( xuiWidgetCreate(pContext, &pWidget) != XUI_OK ) return NULL;
	(void)xuiWidgetSetRect(pWidget, (xui_rect_t){iX, 10, 40, 20});
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetTabStop(pWidget, 1);
	(void)xuiWidgetAddChild(pParent, pWidget);
	return pWidget;
}

static void clear_tab_stops(xui_widget pWidget)
{
	xui_widget pChild;
	(void)xuiWidgetSetTabStop(pWidget, 0);
	for ( pChild = xuiWidgetGetFirstChild(pWidget); pChild != NULL;
		pChild = xuiWidgetGetNextSibling(pChild) ) clear_tab_stops(pChild);
}

static xui_widget popup_create(focus_test_t* pState, xui_widget pOwner, int bModal)
{
	xui_popup_desc_t tDesc;
	xui_widget pPopup = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	tDesc.bModal = bModal;
	tDesc.iFocusPolicy = XUI_POPUP_FOCUS_NONE;
	tDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_IGNORE;
	tDesc.fContentWidth = 160;
	tDesc.fContentHeight = 100;
	if ( xuiPopupCreate(pState->pContext, &pPopup, &tDesc) != XUI_OK ) return NULL;
	clear_tab_stops(pPopup);
	/* Let context teardown own even the initially closed test popups. */
	if ( strcmp(pState->sName, "initial_unattached") != 0 ) {
		(void)xuiOverlayAttach(pState->pContext, pOwner, pPopup, XUI_LAYER_POPUP, 0);
	}
	return pPopup;
}

static int setup(focus_test_t* pState, const char* sName)
{
	memset(pState, 0, sizeof(*pState));
	pState->sName = sName;
	if ( xuiCreate(&pState->pContext) != XUI_OK ) return 0;
	(void)xuiSetViewportSize(pState->pContext, 500, 300);
	if ( xuiWidgetCreate(pState->pContext, &pState->pRoot) != XUI_OK ) return 0;
	(void)xuiSetRootWidget(pState->pContext, pState->pRoot);
	pState->pParent = focus_child(pState->pContext, pState->pRoot, 0);
	pState->pOwner = focus_child(pState->pContext, pState->pParent, 0);
	pState->pBackground = focus_child(pState->pContext, pState->pRoot, 100);
	pState->pPopup = popup_create(pState, pState->pOwner, 1);
	if ( pState->pPopup == NULL ) return 0;
	pState->pChild[0] = focus_child(pState->pContext, xuiPopupGetContentWidget(pState->pPopup), 10);
	pState->pChild[1] = focus_child(pState->pContext, xuiPopupGetContentWidget(pState->pPopup), 70);
	(void)xuiSetFocusWidget(pState->pContext, pState->pOwner);
	xuiClearEvents(pState->pContext);
	return pState->pChild[0] != NULL && pState->pChild[1] != NULL;
}

static int inside(xui_widget pRoot, xui_widget pWidget)
{
	for ( ; pWidget != NULL; pWidget = xuiWidgetGetParent(pWidget) ) {
		if ( pWidget == pRoot ) return 1;
	}
	return 0;
}

static void check_modal_focus(focus_test_t* pState, xui_widget pPopup)
{
	CHECK(pState, inside(pPopup, xuiGetFocusWidget(pState->pContext)), "focus stays inside modal");
}

static int blur_reentry(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	if ( pEvent->iType != XUI_EVENT_BLUR || pState->iCallbacks++ != 0 ) return XUI_OK;
	if ( strcmp(pState->sName, "close_blur_destroy_context") == 0 ) {
		xuiDestroy(pState->pContext);
		pState->bDestroyed = 1;
	} else if ( strcmp(pState->sName, "close_blur_destroy_popup") == 0 ||
		strcmp(pState->sName, "close_blur_destroy_owner") == 0 ) {
		xuiWidgetDestroy(pState->pPopup);
		if ( strcmp(pState->sName, "close_blur_destroy_owner") == 0 ) xuiWidgetDestroy(pState->pOwner);
	} else if ( strcmp(pState->sName, "close_blur_open_new") == 0 ) {
		CHECK(pState, xuiPopupSetOpen(pState->pNested, 1) == XUI_OK, "open new modal during close BLUR");
	} else {
		CHECK(pState, xuiPopupSetOpen(pState->pPopup, 1) == XUI_OK, "reopen during close BLUR");
	}
	return XUI_OK;
}

static int key_open_modal(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_TARGET ) {
		pState->iCallbacks++;
		(void)xuiPopupSetOpen(pState->pPopup, 1);
	}
	return XUI_OK;
}

static int background_key(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	(void)pEvent;
	pState->iBackgroundKeys++;
	return XUI_OK;
}

static xui_rect_t ime_reentry(xui_widget pWidget, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	if ( pState->iCallbacks++ == 0 ) {
		CHECK(pState, pState->pContext->iOperationDepth > 0, "focus callback has operation barrier");
		if ( strstr(pState->sName, "destroy_context") != NULL ) {
			xuiDestroy(pState->pContext);
			pState->bDestroyed = 1;
		} else if ( strcmp(pState->sName, "ime_destroy_target") == 0 ) {
			xuiWidgetDestroy(pWidget);
		} else {
			(void)xuiPopupSetOpen(pState->pNested, 1);
		}
	}
	return (xui_rect_t){0, 0, 1, 1};
}

static int layout_destroy_context(xui_widget pWidget, xui_rect_t tRect, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	(void)tRect;
	if ( pState->iCallbacks++ == 0 ) {
		xuiDestroy(pState->pContext);
		pState->bDestroyed = 1;
	}
	return XUI_OK;
}

static void change_destroy_context(xui_widget pWidget, int bOpen, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	(void)bOpen;
	pState->iCallbacks++;
	CHECK(pState, pState->pContext->iOperationDepth > 0, "change callback has popup operation barrier");
	xuiDestroy(pState->pContext);
	pState->bDestroyed = 1;
}

static void close_rebind_unavailable(xui_widget pWidget, int bOpen, void* pUser)
{
	focus_test_t* pState = (focus_test_t*)pUser;
	(void)pWidget;
	if ( bOpen || pState->iCallbacks++ != 0 ) return;
	CHECK(pState, xuiPopupSetOwner(pState->pNested, pState->pOwner) != XUI_OK, "cannot rebind cancelling owner");
	CHECK(pState, xuiPopupSetFocusRestore(pState->pNested, pState->pOwner) != XUI_OK, "cannot restore to cancelling subtree");
	CHECK(pState, xuiPopupSetFocusPolicy(pState->pNested, XUI_POPUP_FOCUS_CUSTOM, pState->pOwner) != XUI_OK,
		"cannot configure cancelling focus target");
	xuiWidgetDestroy(pState->pParent);
}

static int run_case(const char* sName)
{
	focus_test_t tState;
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pOther;
	xui_widget pNestedChild;
	int i;
	int iRet;

	if ( !setup(&tState, sName) ) return 1;
	pContext = tState.pContext;
	pPopup = tState.pPopup;
	if ( strcmp(sName, "empty_fallback") == 0 ) {
		(void)xuiWidgetSetFocusable(tState.pChild[0], 0);
		(void)xuiWidgetSetFocusable(tState.pChild[1], 0);
	}
	if ( strcmp(sName, "custom_background") == 0 ) {
		(void)xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, tState.pBackground);
	}
	if ( strcmp(sName, "set_modal_open") == 0 || strcmp(sName, "nonmodal_none") == 0 ) {
		(void)xuiPopupSetModal(pPopup, 0);
	}
	if ( strcmp(sName, "destroy_custom_closed") == 0 ) {
		(void)xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, tState.pBackground);
		xuiWidgetDestroy(tState.pBackground);
	}
	if ( strcmp(sName, "destroy_restore_closed") == 0 ) {
		(void)xuiPopupSetFocusRestore(pPopup, tState.pBackground);
		xuiWidgetDestroy(tState.pBackground);
	}
	if ( strcmp(sName, "change_destroy_context") == 0 ) {
		(void)xuiPopupSetChange(pPopup, change_destroy_context, &tState);
	}
	if ( strncmp(sName, "ime_", 4) == 0 ) {
		tState.pNested = popup_create(&tState, tState.pBackground, 1);
		(void)xuiWidgetSetImeMode(tState.pChild[0], XUI_IME_ENABLED);
		(void)xuiWidgetSetImeCandidateRect(tState.pChild[0], ime_reentry, &tState);
	}
	CHECK(&tState, xuiPopupSetOpen(pPopup, 1) == XUI_OK, "open popup");
	if ( strncmp(sName, "ime_", 4) == 0 ) CHECK(&tState, tState.iCallbacks > 0, "initial IME callback executed");
	if ( tState.bDestroyed ) goto cleanup;
	if ( strcmp(sName, "ime_open_new") == 0 ) {
		check_modal_focus(&tState, tState.pNested);
		CHECK(&tState, (xuiWidgetGetStateId(tState.pChild[0]) & XUI_WIDGET_STATE_FOCUS) == 0, "stale focus commit cannot mark old target focused");
	} else if ( strcmp(sName, "ime_destroy_target") == 0 ) {
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pChild[1], "destroyed initial target falls back to next child");
	} else if ( strcmp(sName, "colors_ime_destroy_context") == 0 ) {
		(void)xuiSetFocusWidget(pContext, NULL);
		(void)xuiWidgetSetImeMode(tState.pChild[0], XUI_IME_ENABLED);
		(void)xuiWidgetSetImeCandidateRect(tState.pChild[0], ime_reentry, &tState);
		(void)xuiPopupSetColors(pPopup, 0, 0, 0, 0);
		CHECK(&tState, tState.bDestroyed, "color placement focus callback destroys context");
	} else if ( strcmp(sName, "content_layout_destroy_context") == 0 ) {
		(void)xuiWidgetSetLayoutCompleteCallback(xuiPopupGetContentWidget(pPopup), layout_destroy_context, &tState);
		(void)xuiPopupSetContentSize(pPopup, 180, 120);
		CHECK(&tState, tState.bDestroyed, "content placement callback destroys context");
	} else if ( strcmp(sName, "restore_ime_open_new") == 0 ) {
		tState.pNested = popup_create(&tState, tState.pBackground, 1);
		(void)xuiWidgetSetImeMode(tState.pOwner, XUI_IME_ENABLED);
		(void)xuiWidgetSetImeCandidateRect(tState.pOwner, ime_reentry, &tState);
		(void)xuiPopupSetOpen(pPopup, 0);
		check_modal_focus(&tState, tState.pNested);
		CHECK(&tState, (xuiWidgetGetStateId(tState.pOwner) & XUI_WIDGET_STATE_FOCUS) == 0, "stale restore cannot focus background owner");
	} else if ( strcmp(sName, "none_initial") == 0 || strcmp(sName, "custom_background") == 0 ||
		strcmp(sName, "initial_unattached") == 0 || strcmp(sName, "destroy_custom_closed") == 0 ) {
		check_modal_focus(&tState, pPopup);
	} else if ( strcmp(sName, "nonmodal_none") == 0 ) {
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pOwner, "nonmodal NONE preserves focus");
	} else if ( strcmp(sName, "destroy_restore_closed") == 0 ) {
		(void)xuiPopupSetOpen(pPopup, 0);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pOwner, "destroyed explicit restore falls back to owner");
	} else if ( strcmp(sName, "set_modal_open") == 0 ) {
		CHECK(&tState, xuiPopupSetModal(pPopup, 1) == XUI_OK, "promote open popup to modal");
		check_modal_focus(&tState, pPopup);
	} else if ( strcmp(sName, "tab_no_focus") == 0 || strcmp(sName, "shift_tab_no_focus") == 0 ) {
		int bReverse = strcmp(sName, "shift_tab_no_focus") == 0;
		(void)xuiSetFocusWidget(pContext, NULL);
		CHECK(&tState, xuiInputKeyDown(pContext, XUI_KEY_TAB, bReverse ? XUI_MOD_SHIFT : 0u) == XUI_OK,
			"Tab without focus");
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pChild[bReverse ? 1 : 0],
			"Tab starts at modal boundary");
		for ( i = 0; i < 5; i++ ) {
			(void)xuiFocusNext(pContext, !bReverse);
			check_modal_focus(&tState, pPopup);
		}
	} else if ( strcmp(sName, "explicit_background") == 0 ) {
		(void)xuiSetFocusWidget(pContext, tState.pChild[0]);
		CHECK(&tState, xuiSetFocusWidget(pContext, tState.pBackground) != XUI_OK,
			"background focus rejected");
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pChild[0], "rejected focus leaves current unchanged");
	} else if ( strcmp(sName, "empty_fallback") == 0 ) {
		(void)xuiSetFocusWidget(pContext, NULL);
		(void)xuiFocusNext(pContext, 1);
		CHECK(&tState, xuiGetFocusWidget(pContext) == pPopup, "empty modal focuses shell");
		(void)xuiFocusNext(pContext, 0);
		CHECK(&tState, xuiGetFocusWidget(pContext) == pPopup, "empty modal reverse fallback");
	} else if ( strcmp(sName, "hidden_target") == 0 || strcmp(sName, "disabled_target") == 0 ) {
		(void)xuiSetFocusWidget(pContext, tState.pChild[0]);
		if ( strcmp(sName, "hidden_target") == 0 ) (void)xuiWidgetSetVisible(tState.pChild[0], 0);
		else (void)xuiWidgetSetEnabled(tState.pChild[0], 0);
		CHECK(&tState, xuiSetFocusWidget(pContext, tState.pBackground) != XUI_OK, "invalidating child cannot release modal constraint");
		(void)xuiFocusNext(pContext, 1);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pChild[1], "Tab skips unavailable child");
	} else if ( strcmp(sName, "nested_modal") == 0 || strcmp(sName, "owned_nonmodal") == 0 ) {
		int bModal = strcmp(sName, "nested_modal") == 0;
		(void)xuiSetFocusWidget(pContext, tState.pChild[0]);
		tState.pNested = popup_create(&tState, tState.pChild[0], bModal);
		pNestedChild = focus_child(pContext, xuiPopupGetContentWidget(tState.pNested), 0);
		(void)xuiPopupSetFocusPolicy(tState.pNested, XUI_POPUP_FOCUS_FIRST_CHILD, NULL);
		CHECK(&tState, xuiPopupSetOpen(tState.pNested, 1) == XUI_OK, "open nested popup");
		CHECK(&tState, xuiSetFocusWidget(pContext, pNestedChild) == XUI_OK, "owned popup belongs to focus domain");
		(void)xuiFocusNext(pContext, 1);
		CHECK(&tState, xuiGetFocusWidget(pContext) == pNestedChild, "nested popup Tab scope");
		if ( bModal ) {
			CHECK(&tState, xuiSetFocusWidget(pContext, tState.pChild[1]) != XUI_OK, "outer modal blocked by inner modal");
			(void)xuiSetFocusWidget(pContext, NULL);
			(void)xuiFocusNext(pContext, 0);
			CHECK(&tState, xuiGetFocusWidget(pContext) == pNestedChild, "null focus stays in innermost modal");
		}
		(void)xuiPopupSetOpen(tState.pNested, 0);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pChild[0], "nested close restores modal owner");
		CHECK(&tState, xuiSetFocusWidget(pContext, tState.pBackground) != XUI_OK, "outer modal remains constrained");
	} else if ( strcmp(sName, "unrelated_nonmodal") == 0 ) {
		tState.pNested = popup_create(&tState, tState.pBackground, 0);
		pNestedChild = focus_child(pContext, xuiPopupGetContentWidget(tState.pNested), 0);
		(void)xuiPopupSetOpen(tState.pNested, 1);
		CHECK(&tState, xuiSetFocusWidget(pContext, pNestedChild) != XUI_OK, "unrelated overlay cannot bypass modal");
	} else if ( strcmp(sName, "deep_owned_nonmodal") == 0 || strcmp(sName, "close_outer_nested") == 0 ) {
		tState.pNested = popup_create(&tState, tState.pChild[0], 1);
		pNestedChild = focus_child(pContext, xuiPopupGetContentWidget(tState.pNested), 0);
		(void)xuiPopupSetOpen(tState.pNested, 1);
		pOther = popup_create(&tState, pNestedChild, 0);
		pNestedChild = focus_child(pContext, xuiPopupGetContentWidget(pOther), 0);
		(void)xuiPopupSetOpen(pOther, 1);
		CHECK(&tState, xuiSetFocusWidget(pContext, pNestedChild) == XUI_OK, "transitive owned popup is allowed");
		CHECK(&tState, xuiSetFocusWidget(pContext, tState.pChild[1]) != XUI_OK, "innermost modal blocks outer focus");
		if ( strcmp(sName, "close_outer_nested") == 0 ) {
			(void)xuiPopupSetOpen(pPopup, 0);
			CHECK(&tState, !xuiPopupIsOpen(tState.pNested) && !xuiPopupIsOpen(pOther), "closing owner closes owned popup chain");
			CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pOwner, "outer close restores background owner");
		} else {
			(void)xuiWidgetSetEnabled(tState.pNested, 0);
			CHECK(&tState, xuiSetFocusWidget(pContext, pNestedChild) != XUI_OK, "disabled logical ancestor cannot be bypassed");
			CHECK(&tState, xuiSetFocusWidget(pContext, tState.pBackground) != XUI_OK, "disabled modal still blocks background");
		}
	} else if ( strcmp(sName, "disabled_modal") == 0 ) {
		(void)xuiWidgetSetEnabled(pPopup, 0);
		CHECK(&tState, xuiSetFocusWidget(pContext, tState.pBackground) != XUI_OK, "disabled visible modal stays a barrier");
		(void)xuiFocusNext(pContext, 1);
		CHECK(&tState, xuiGetFocusWidget(pContext) == NULL, "fully disabled modal has no unsafe fallback");
		xuiWidgetDestroy(pPopup);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pOwner, "destroying disabled modal restores owner");
	} else if ( strcmp(sName, "restore_unfocusable_owner") == 0 ) {
		(void)xuiWidgetSetFocusable(tState.pOwner, 0);
		(void)xuiPopupSetOpen(pPopup, 0);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pParent, "unfocusable owner falls back to parent");
	} else if ( strcmp(sName, "restore_hidden_ancestor") == 0 ) {
		(void)xuiWidgetSetFocusable(tState.pRoot, 1);
		(void)xuiWidgetSetVisible(tState.pParent, 0);
		(void)xuiPopupSetOpen(pPopup, 0);
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pRoot, "restore skips hidden ancestor subtree");
	} else if ( strcmp(sName, "modal_null_escape") == 0 ) {
		(void)xuiSetFocusWidget(pContext, NULL);
		CHECK(&tState, xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0) == XUI_OK, "Escape without focus returns safely");
		CHECK(&tState, !xuiPopupIsOpen(pPopup), "Escape reaches modal fallback");
	} else if ( strcmp(sName, "key_opens_modal") == 0 || strcmp(sName, "escape_opens_modal") == 0 ) {
		(void)xuiPopupSetOpen(pPopup, 0);
		(void)xuiWidgetSetEventHandler(tState.pOwner, XUI_EVENT_KEY_DOWN, key_open_modal, &tState);
		(void)xuiWidgetSetEventHandler(tState.pParent, XUI_EVENT_KEY_DOWN, background_key, &tState);
		CHECK(&tState, xuiInputKeyDown(pContext, strcmp(sName, "escape_opens_modal") == 0 ? XUI_KEY_ESCAPE : 'K', 0) == XUI_OK, "key callback opens modal");
		CHECK(&tState, tState.iCallbacks == 1 && tState.iBackgroundKeys == 0, "old key cannot bubble into background after modal opens");
		check_modal_focus(&tState, pPopup);
	} else if ( strcmp(sName, "close_rebind_unavailable") == 0 ) {
		tState.pNested = popup_create(&tState, tState.pBackground, 0);
		(void)xuiPopupSetFocusRestore(tState.pNested, tState.pOwner);
		(void)xuiPopupSetChange(pPopup, close_rebind_unavailable, &tState);
		xuiWidgetDestroy(tState.pOwner);
		CHECK(&tState, tState.iCallbacks == 1, "owner removal sends close notification once");
		(void)xuiPopupSetOpen(tState.pNested, 1);
		(void)xuiPopupSetOpen(tState.pNested, 0);
	} else if ( strcmp(sName, "restore_owner_ancestor") == 0 || strcmp(sName, "restore_destroyed") == 0 ) {
		(void)xuiSetFocusWidget(pContext, tState.pChild[0]);
		if ( strcmp(sName, "restore_destroyed") == 0 ) xuiWidgetDestroy(tState.pOwner);
		else (void)xuiWidgetSetEnabled(tState.pOwner, 0);
		CHECK(&tState, xuiPopupSetOpen(pPopup, 0) == XUI_OK, "close with unavailable restore target");
		CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pParent, "restore nearest available owner ancestor");
	} else if ( strcmp(sName, "owner_self_cycle") == 0 ) {
		CHECK(&tState, xuiPopupSetOwner(pPopup, pPopup) != XUI_OK, "reject self owner cycle");
		(void)xuiPopupSetOwner(pPopup, tState.pOwner);
	} else if ( strcmp(sName, "owner_child_cycle") == 0 ) {
		CHECK(&tState, xuiPopupSetOwner(pPopup, tState.pChild[0]) != XUI_OK, "reject physical descendant owner cycle");
		(void)xuiPopupSetOwner(pPopup, tState.pOwner);
	} else if ( strcmp(sName, "owner_popup_cycle") == 0 ) {
		tState.pNested = popup_create(&tState, tState.pChild[0], 0);
		pOther = focus_child(pContext, xuiPopupGetContentWidget(tState.pNested), 0);
		CHECK(&tState, xuiPopupSetOwner(pPopup, pOther) != XUI_OK, "reject transitive popup owner cycle");
		(void)xuiPopupSetOwner(pPopup, tState.pOwner);
	} else if ( strncmp(sName, "close_blur_", 11) == 0 ) {
		if ( strcmp(sName, "close_blur_open_new") == 0 ) {
			tState.pNested = popup_create(&tState, tState.pBackground, 1);
		}
		(void)xuiSetFocusWidget(pContext, tState.pChild[0]);
		xuiClearEvents(pContext);
		(void)xuiWidgetSetEventHandler(tState.pChild[0], XUI_EVENT_BLUR, blur_reentry, &tState);
		iRet = xuiPopupSetOpen(pPopup, 0);
		CHECK(&tState, iRet == XUI_OK, "close returns safely after BLUR reentry");
		CHECK(&tState, tState.iCallbacks == 1, "BLUR callback once");
		if ( !tState.bDestroyed && strcmp(sName, "close_blur_reopen") == 0 ) {
			CHECK(&tState, xuiPopupIsOpen(pPopup), "old close cannot overwrite reopen");
			check_modal_focus(&tState, pPopup);
		} else if ( strcmp(sName, "close_blur_destroy_popup") == 0 ) {
			CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pOwner, "destroy during close still restores owner");
		} else if ( strcmp(sName, "close_blur_destroy_owner") == 0 ) {
			CHECK(&tState, xuiGetFocusWidget(pContext) == tState.pParent, "pending popup destruction tracks destroyed owner");
		} else if ( strcmp(sName, "close_blur_open_new") == 0 ) {
			check_modal_focus(&tState, tState.pNested);
		}
	}
cleanup:
	if ( !tState.bDestroyed ) xuiDestroy(pContext);
	if ( !tState.iFailed ) printf("PASS %s\n", sName);
	return tState.iFailed;
}

int main(int argc, char** argv)
{
	static const char* const arrNames[] = {
		"none_initial", "tab_no_focus", "shift_tab_no_focus", "explicit_background", "empty_fallback",
		"custom_background", "set_modal_open", "hidden_target", "disabled_target", "nested_modal",
		"owned_nonmodal", "unrelated_nonmodal", "restore_owner_ancestor", "restore_destroyed",
		"owner_self_cycle", "owner_child_cycle", "owner_popup_cycle", "close_blur_reopen",
		"close_blur_destroy_popup", "close_blur_destroy_context", "change_destroy_context",
		"initial_unattached", "nonmodal_none", "destroy_custom_closed", "destroy_restore_closed",
		"deep_owned_nonmodal", "close_outer_nested", "disabled_modal", "restore_unfocusable_owner",
		"restore_hidden_ancestor", "modal_null_escape", "key_opens_modal", "close_blur_destroy_owner",
		"close_blur_open_new", "ime_open_new", "ime_destroy_target", "ime_destroy_context",
		"colors_ime_destroy_context", "content_layout_destroy_context", "restore_ime_open_new",
		"escape_opens_modal", "close_rebind_unavailable"
	};
	size_t i;
	int iRun = 0;
	int iFailed = 0;
	for ( i = 0; i < sizeof(arrNames) / sizeof(arrNames[0]); i++ ) {
		if ( argc > 1 && strcmp(argv[1], arrNames[i]) != 0 ) continue;
		iRun++;
		iFailed += run_case(arrNames[i]);
	}
	printf("popup focus: %d passed, %d failed\n", iRun - iFailed, iFailed);
	return iFailed != 0 || iRun == 0;
}
