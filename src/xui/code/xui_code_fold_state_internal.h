#ifndef XUI_CODE_FOLD_STATE_INTERNAL_H
#define XUI_CODE_FOLD_STATE_INTERNAL_H

#include "xui.h"

/* The independent fold model must link without the editor or widget runtime. */
typedef struct xui_code_fold_owner_ops_t {
	int (*check)(xui_widget editor, bool write);
	int (*watch)(xui_widget editor, xui_lifetime_link_t*, xui_lifetime_invalidate_proc, void*);
	void (*changed)(xui_widget editor);
	int (*build)(xui_widget editor, xui_code_fold_state, xui_code_document, xui_code_fold_proc, void*);
} xui_code_fold_owner_ops_t;

int xuiInternalCodeFoldStateClaimOwner(xui_code_fold_state state, xui_widget editor,
	const xui_code_fold_owner_ops_t* ops);
void xuiInternalCodeFoldStateDestroyOwned(xui_code_fold_state state, xui_widget editor);
int xuiInternalCodeFoldStateBuild(xui_code_fold_state state, xui_code_document document,
	xui_code_fold_proc callback, void* user);

#endif
