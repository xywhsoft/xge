#ifndef XUI_DOCUMENT_UI_H
#define XUI_DOCUMENT_UI_H
#include "xui_document.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xui_document_renderer_t* xui_document_renderer;
enum xui_doc_display_mode { XUI_DOC_VISUAL = 1, XUI_DOC_SOURCE_TEXT = 2, XUI_DOC_LIVE_MARKDOWN = 3 };
typedef struct xui_doc_rect_t { double x, y, width, height; } xui_doc_rect_t;
typedef struct xui_doc_font_set_t {
    xui_font normal, bold, italic, boldItalic, monospace;
} xui_doc_font_set_t;
typedef xui_font (*xui_doc_font_proc)(xui_context, const char* family, uint32_t marks, float size, void* user);
typedef int (*xui_doc_object_measure_proc)(xui_document_snapshot, xui_doc_node_id, float width, float zoom,
    xui_vec2_t* size, float* baseline, void* user);
typedef int (*xui_doc_object_draw_proc)(xui_document_snapshot, xui_doc_node_id, xui_proxy, xui_draw_context,
    xui_rect_t bounds, void* user);
typedef struct xui_doc_renderer_desc_t {
    uint32_t iSize;
    xui_doc_font_set_t tFonts;
    float fZoom;
    float fParagraphGap;
    float fLineGap;
    float fIndent;
    uint32_t iTextColor;
    uint32_t iBorderColor;
    uint32_t iCodeBackground;
    xui_doc_font_proc onFont;
    xui_doc_object_measure_proc onObjectMeasure;
    xui_doc_object_draw_proc onObjectDraw;
    void* pUser;
} xui_doc_renderer_desc_t;
typedef struct xui_doc_renderer_stats_t {
    uint32_t iSize;
    uint64_t iBlocks;
    uint64_t iMeasuredBlocks;
    uint64_t iShapedBytes;
    uint64_t iFragments;
    uint64_t iLayoutPasses;
    uint64_t iSourceBlocks;
    int bLiveSourceFallback;
} xui_doc_renderer_stats_t;

/* The renderer borrows its context/fonts and retains its immutable snapshot.
 * It owns no widget, document writer, selection history, or window. */
XUI_API int xuiDocumentRendererCreate(xui_context context, const xui_doc_renderer_desc_t* desc, xui_document_renderer* out);
XUI_API void xuiDocumentRendererRelease(xui_document_renderer renderer);
XUI_API int xuiDocumentRendererSetSnapshot(xui_document_renderer renderer, xui_document_snapshot snapshot, xui_document_change_set changes);
XUI_API int xuiDocumentRendererSetMode(xui_document_renderer renderer, uint32_t mode);
/* LIVE_MARKDOWN exposes the active top-level Markdown container as source.
 * Hit tests/selections in this mode use SOURCE positions. Unknown source
 * boundaries conservatively expose the whole source instead of hiding bytes. */
XUI_API int xuiDocumentRendererSetActivePosition(xui_document_renderer renderer, const xui_doc_position_t* position);
XUI_API int xuiDocumentRendererGetActiveSourceRange(xui_document_renderer renderer, uint64_t* start, uint64_t* end);
XUI_API int xuiDocumentRendererLayout(xui_document_renderer renderer, double width, double viewport_top, double viewport_height);
XUI_API int xuiDocumentRendererGetSize(xui_document_renderer renderer, xui_doc_rect_t* size, int* exact);
XUI_API int xuiDocumentRendererHitTest(xui_document_renderer renderer, double x, double y, xui_doc_position_t* position);
XUI_API int xuiDocumentRendererGetCaretRect(xui_document_renderer renderer, const xui_doc_position_t* position, xui_doc_rect_t* rect);
XUI_API int xuiDocumentRendererDraw(xui_document_renderer renderer, xui_draw_context draw, double origin_x, double origin_y,
    xui_rect_t clip, const xui_doc_range_t* selection, uint32_t selection_color);
XUI_API int xuiDocumentRendererGetStats(xui_document_renderer renderer, xui_doc_renderer_stats_t* stats);

typedef void (*xui_doc_view_event_proc)(xui_widget view, xui_doc_node_id node, const char* resource, void* user);
typedef struct xui_doc_view_desc_t {
    uint32_t iSize;
    xui_document pDocument;
    xui_doc_renderer_desc_t tRenderer;
    int bAutoHeight;
    int bDisableSelection;
    uint32_t iSelectionColor;
    xui_doc_view_event_proc onActivate;
    void* pUser;
} xui_doc_view_desc_t;
XUI_API xui_widget_type xuiDocumentViewGetType(xui_context context);
XUI_API int xuiDocumentViewCreate(xui_context context, const xui_doc_view_desc_t* desc, xui_widget* out);
XUI_API int xuiDocumentViewSetDocument(xui_widget view, xui_document document);
XUI_API xui_document xuiDocumentViewGetDocument(xui_widget view);
XUI_API int xuiDocumentViewSetSelection(xui_widget view, const xui_doc_range_t* range);
XUI_API int xuiDocumentViewGetSelection(xui_widget view, xui_doc_range_t* range);
XUI_API int xuiDocumentViewSetScroll(xui_widget view, double x, double y);
XUI_API int xuiDocumentViewGetScroll(xui_widget view, double* x, double* y);
XUI_API int xuiDocumentViewSetZoom(xui_widget view, float zoom);
XUI_API int xuiDocumentViewSetMode(xui_widget view, uint32_t mode);
XUI_API uint32_t xuiDocumentViewGetMode(xui_widget view);
XUI_API int xuiDocumentViewGetContentSize(xui_widget view, xui_doc_rect_t* size, int* exact);
XUI_API int xuiDocumentViewHitTest(xui_widget view, double x, double y, xui_doc_position_t* position);
XUI_API int xuiDocumentViewFind(xui_widget view, const char* pattern, uint64_t bytes, int backward, int wrap, xui_doc_range_t* match);

/* A DocumentEditor is a DocumentView subtype; every View API also accepts it.
 * tView is first so the base type can initialize the same renderer/subscription. */
typedef void (*xui_doc_editor_error_proc)(xui_widget editor, int error, void* user);
typedef struct xui_doc_editor_desc_t {
    xui_doc_view_desc_t tView;
    uint32_t iSize;
    uint32_t iMode;
    int bReadOnly;
    uint32_t iCaretColor;
    xui_doc_editor_error_proc onError;
    void* pUser;
} xui_doc_editor_desc_t;
enum xui_doc_editor_command {
    XUI_DOC_EDIT_UNDO = 1, XUI_DOC_EDIT_REDO, XUI_DOC_EDIT_SELECT_ALL,
    XUI_DOC_EDIT_COPY, XUI_DOC_EDIT_CUT, XUI_DOC_EDIT_PASTE,
    XUI_DOC_EDIT_BACKSPACE, XUI_DOC_EDIT_DELETE, XUI_DOC_EDIT_ENTER,
    XUI_DOC_EDIT_BOLD, XUI_DOC_EDIT_ITALIC, XUI_DOC_EDIT_UNDERLINE,
    XUI_DOC_EDIT_STRIKE
};
typedef struct xui_doc_command_state_t {
    uint32_t iSize;
    int bEnabled, bActive, bMixed;
    int iDisabledReason;
} xui_doc_command_state_t;
XUI_API xui_widget_type xuiDocumentEditorGetType(xui_context context);
XUI_API int xuiDocumentEditorCreate(xui_context context, const xui_doc_editor_desc_t* desc, xui_widget* out);
XUI_API int xuiDocumentEditorInsertText(xui_widget editor, const char* text, uint64_t bytes);
XUI_API int xuiDocumentEditorExecute(xui_widget editor, uint32_t command);
/* Availability checks the current mode, profile, selection, backend and history.
 * A semantic Markdown command still validates representability before commit. */
XUI_API int xuiDocumentEditorQueryCommand(xui_widget editor, uint32_t command, xui_doc_command_state_t* state);
XUI_API int xuiDocumentEditorCanExecute(xui_widget editor, uint32_t command);
XUI_API int xuiDocumentEditorSetReadOnly(xui_widget editor, int read_only);
XUI_API int xuiDocumentEditorGetReadOnly(xui_widget editor);
XUI_API int xuiDocumentEditorSetMarks(xui_widget editor, uint32_t set, uint32_t clear);
XUI_API int xuiDocumentEditorReplaceAll(xui_widget editor, const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes, const xui_doc_range_t* scope, uint64_t* replaced);
XUI_API int xuiDocumentEditorCancelComposition(xui_widget editor);
XUI_API int xuiDocumentEditorIsComposing(xui_widget editor);

#ifdef __cplusplus
}
#endif
#endif
