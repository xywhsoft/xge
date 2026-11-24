#ifndef XUI_WIDGET_CALLBACK_SNAPSHOT_INTERNAL_H
#define XUI_WIDGET_CALLBACK_SNAPSHOT_INTERNAL_H

#include "xui.h"

typedef void (*xui_internal_snapshot_callback)(void);
typedef int (*xui_internal_snapshot_install)(xui_widget widget,
    xui_internal_snapshot_callback callback, void* user, const xui_callback_owner_t* owner);

int xuiInternalWidgetCallbackCaptureBegin(xui_widget widget,
    xui_widget_callback_snapshot_t** output, xui_call_scope_t* scope);
int xuiInternalWidgetCallbackCaptureEnd(xui_call_scope_t* scope, xui_widget widget,
    const void* slot, xui_internal_snapshot_callback callback, void* user,
    xui_internal_snapshot_install install, xui_widget_callback_snapshot_t** output, int status);

/* Keep each setter call typed. The stored function pointer is never invoked
 * before conversion back to its original type. */
#define XUI_DEFINE_CALLBACK_CAPTURE(api, callback_type, getter, setter, slot_address) \
static int api##Install(xui_widget widget, xui_internal_snapshot_callback callback, \
    void* user, const xui_callback_owner_t* owner) \
{ \
    return setter(widget, (callback_type)callback, user, owner); \
} \
XUI_API int api(xui_widget widget, xui_widget_callback_snapshot_t** output) \
{ \
    xui_call_scope_t scope = {0}; \
    int status = xuiInternalWidgetCallbackCaptureBegin(widget, output, &scope); \
    if (status != XUI_OK) return status; \
    callback_type callback = NULL; \
    void* user = NULL; \
    status = getter(widget, &callback, &user); \
    const void* slot = status == XUI_OK && callback ? (slot_address) : NULL; \
    return xuiInternalWidgetCallbackCaptureEnd(&scope, widget, slot, \
        (xui_internal_snapshot_callback)callback, user, api##Install, output, status); \
}

#endif
