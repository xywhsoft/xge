#ifndef XUI_CODE_LANGUAGE_INTERNAL_H
#define XUI_CODE_LANGUAGE_INTERNAL_H

#include "xui.h"

struct xui_code_fold_output_t {
	xui_code_fold_range_t* pRanges;
	int iCapacity;
	int iCount;
};

typedef struct xui_code_language_owner_ops_t {
	int (*check)(xui_widget editor, bool write);
	int (*watch)(xui_widget editor, xui_lifetime_link_t*, xui_lifetime_invalidate_proc, void*);
	void (*changed)(xui_widget editor, const char* languageId);
} xui_code_language_owner_ops_t;

int xuiInternalCodeLanguageRegistryClaimOwner(xui_code_language_registry registry,
	xui_widget editor, const xui_code_language_owner_ops_t* ops);
int xuiInternalCodeLanguageRegistryDestroyOwned(xui_code_language_registry registry, xui_widget editor);

#endif
