#ifndef XUI_EDIT_INTERNAL_H
#define XUI_EDIT_INTERNAL_H

#include "xui.h"

typedef enum xui_edit_event_mode_t {
	XUI_EDIT_EVENT_UNOBSERVED = -1,
	XUI_EDIT_EVENT_LEGACY = 0,
	XUI_EDIT_EVENT_METADATA,
	XUI_EDIT_EVENT_CAPTURE_TEXT
} xui_edit_event_mode_t;

xui_edit_event_mode_t xuiInternalEditEventMode(xui_widget host);

/* The caller keeps counted, terminated text valid across callback reentry. */
int xuiInternalEditEmitStable(xui_widget widget, int type, const char* text, int textSize,
	int first, int last, int compositionStart, int compositionEnd, int valid);

#endif
