#ifndef XUI_CODE_MARGIN_LIFETIME_INTERNAL_H
#define XUI_CODE_MARGIN_LIFETIME_INTERNAL_H
#include "xui_internal.h"
int xuiInternalCodeMarginClaimOwner(xui_code_margin_model,xui_widget);
int xuiInternalCodeMarginDestroyOwned(xui_widget,xui_code_margin_model);
typedef struct xui_code_margin_slot_t xui_code_margin_slot_t;
typedef struct xui_code_margin_entry_t {
    xui_code_margin_info_t info;
    xui_code_margin_slot_t* render;
} xui_code_margin_entry_t;
/* Private caller-owned frame, never copy/move while live. Storage belongs to
 * CodeEdit and is reused after End; no borrowed frame escapes into xlang. */
typedef struct xui_code_margin_snapshot_t {
    struct xui_code_margin_snapshot_t* self;
    xui_code_margin_scope_t scope;
    xui_code_margin_entry_t* entries;
    int count;
    bool invoking,ending;
} xui_code_margin_snapshot_t;
int xuiInternalCodeMarginSnapshotBegin(xui_code_margin_model,xui_rect_t,
    xui_code_margin_entry_t*,int,xui_code_margin_snapshot_t*,xui_rect_t*);
int xuiInternalCodeMarginSnapshotEnd(xui_code_margin_snapshot_t*,int);
int xuiInternalCodeMarginSnapshotRender(xui_code_margin_snapshot_t*,int,
    xui_widget,int,xui_draw_context,xui_rect_t,bool*);
int xuiInternalCodeMarginInvokeEvent(xui_code_margin_model,int,xui_widget,int,
    int,xui_rect_t,bool*);
#endif
