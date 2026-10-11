#ifndef XUI_NATIVE_ACCESSIBILITY_INTERNAL_H
#define XUI_NATIVE_ACCESSIBILITY_INTERNAL_H
#include "../xui.h"
/* Platform bridge; separate from the application's accessibility callback. */
XUI_API int xuiInternalNativeAccessibilityBind(xui_context, void*, xui_rect_t);
XUI_API void xuiInternalNativeAccessibilityDetach(xui_context);
XUI_API void xuiInternalNativeAccessibilityResetWidget(xui_widget);
XUI_API void xuiInternalNativeAccessibilityNotify(xui_context, xui_widget,
    const xui_accessibility_event_t*);
#endif
