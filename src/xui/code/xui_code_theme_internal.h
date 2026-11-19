#ifndef XUI_CODE_THEME_INTERNAL_H
#define XUI_CODE_THEME_INTERNAL_H

#include "xui.h"

typedef struct xui_code_theme_owner_ops_t {
	int (*check)(xui_widget editor, bool write);
	int (*watch)(xui_widget editor, xui_lifetime_link_t*, xui_lifetime_invalidate_proc, void*);
	int (*changed)(xui_widget editor);
} xui_code_theme_owner_ops_t;

int xuiInternalCodeThemeClaimOwner(xui_code_theme theme, xui_widget editor, const xui_code_theme_owner_ops_t* ops);
void xuiInternalCodeThemeDestroyOwned(xui_code_theme theme, xui_widget editor);

#endif
