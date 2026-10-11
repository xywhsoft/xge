#ifndef XUI_ACCESSIBILITY_TEXT_INTERNAL_H
#define XUI_ACCESSIBILITY_TEXT_INTERNAL_H
#include "../xui.h"

/* Platform-neutral text ranges. Offsets count UTF-8 bytes, never UTF-16
 * code units. Geometry uses XUI world coordinates and is viewport clipped.
 * Query/action methods belong to the UI thread. Retain/release may run on
 * another thread, including after the widget/context has been destroyed. */
typedef struct xui_internal_accessible_text_range xui_internal_accessible_text_range;
typedef struct xui_internal_accessible_text_ops {
    void (*retain)(xui_internal_accessible_text_range*);
    void (*release)(xui_internal_accessible_text_range*);
    int (*clone)(xui_internal_accessible_text_range*, xui_internal_accessible_text_range**);
    int (*getOffsets)(xui_internal_accessible_text_range*, uint64_t*, uint64_t*, uint64_t*);
    int (*setOffsets)(xui_internal_accessible_text_range*, uint64_t, uint64_t);
    /* bytes excludes the terminator. NULL/0 queries size. A non-NULL buffer
     * must hold bytes+1; insufficient capacity leaves it unchanged. */
    int (*getText)(xui_internal_accessible_text_range*, char*, uint64_t, uint64_t*);
    int (*getRects)(xui_internal_accessible_text_range*, xui_rect_t*, uint64_t, uint64_t*);
    int (*select)(xui_internal_accessible_text_range*);
    int (*scrollIntoView)(xui_internal_accessible_text_range*, int);
    /* Peers must belong to the same View/provider. Ordered endpoints are
     * independent of the direction in which the selection was made. */
    int (*compare)(xui_internal_accessible_text_range*, xui_internal_accessible_text_range*, int*);
    int (*compareEndpoints)(xui_internal_accessible_text_range*, int,
        xui_internal_accessible_text_range*, int, int*);
    int (*setEndpoint)(xui_internal_accessible_text_range*, int,
        xui_internal_accessible_text_range*, int);
    /* Literal UTF-8 search; no match returns OK and NULL. Case insensitive
     * search uses the same Unicode regex engine as Document FindEx. */
    int (*findText)(xui_internal_accessible_text_range*, const char*, uint64_t,
        int, int, xui_internal_accessible_text_range**);
    int (*expand)(xui_internal_accessible_text_range*, int);
    int (*move)(xui_internal_accessible_text_range*, int, int, int*);
    int (*moveEndpoint)(xui_internal_accessible_text_range*, int, int, int, int*);
} xui_internal_accessible_text_ops;
struct xui_internal_accessible_text_range {
    const xui_internal_accessible_text_ops* ops;
};
enum { XUI_INTERNAL_TEXT_START = 0, XUI_INTERNAL_TEXT_END = 1 };
enum {
    XUI_INTERNAL_TEXT_GRAPHEME = 0,
    XUI_INTERNAL_TEXT_FORMAT,
    XUI_INTERNAL_TEXT_WORD,
    XUI_INTERNAL_TEXT_LINE,
    XUI_INTERNAL_TEXT_PARAGRAPH,
    XUI_INTERNAL_TEXT_PAGE,
    XUI_INTERNAL_TEXT_ALL
};
/* GRAPHEME/FORMAT/WORD/LINE/PARAGRAPH/ALL are currently implemented. Other units return
 * UNSUPPORTED atomically until supplied by semantic/layout queries. This
 * protocol does not advertise an incomplete UIA TextPattern to the OS. */
enum {
    XUI_INTERNAL_TEXT_DOCUMENT = 1,
    XUI_INTERNAL_TEXT_SELECTION,
    XUI_INTERNAL_TEXT_CARET,
    XUI_INTERNAL_TEXT_CHILD,
    XUI_INTERNAL_TEXT_POINT
};
typedef struct xui_internal_accessible_text_request {
    int kind;
    uint64_t child;
    double x, y;
} xui_internal_accessible_text_request;
XUI_API int xuiInternalAccessibilityCreateTextRange(xui_widget, uint64_t,
    const xui_internal_accessible_text_request*, xui_internal_accessible_text_range**);
#endif
