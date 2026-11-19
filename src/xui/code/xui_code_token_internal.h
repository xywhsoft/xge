#ifndef XUI_CODE_TOKEN_INTERNAL_H
#define XUI_CODE_TOKEN_INTERNAL_H

#include "xui.h"

typedef struct xui_code_token_owner_ops_t {
    int (*check)(xui_widget editor, bool write);
    int (*watch)(xui_widget editor, xui_lifetime_link_t*, xui_lifetime_invalidate_proc, void*);
    int (*getExtent)(xui_widget editor, uint32_t textVersion, int* length);
    void (*changed)(xui_widget editor);
} xui_code_token_owner_ops_t;

int xuiInternalCodeTokenBufferClaimOwner(xui_code_token_buffer buffer, xui_widget editor,
    const xui_code_token_owner_ops_t* ops);
void xuiInternalCodeTokenBufferDestroyOwned(xui_code_token_buffer buffer, xui_widget editor);
void xuiInternalCodeTokenBufferClear(xui_code_token_buffer buffer);
int xuiInternalCodeTokenBufferSetRange(xui_code_token_buffer buffer, const xui_code_token_t* tokens,
    int count, uint32_t textVersion, int start, int end);

#endif
