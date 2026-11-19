#ifndef XUI_CODE_DIAGNOSTICS_INTERNAL_H
#define XUI_CODE_DIAGNOSTICS_INTERNAL_H

#include "xui.h"

typedef struct xui_code_annotation_owner_ops_t {
    int (*check)(xui_widget editor, bool write);
    int (*watch)(xui_widget editor, xui_lifetime_link_t*, xui_lifetime_invalidate_proc, void*);
    void (*changed)(xui_widget editor, bool diagnostics);
} xui_code_annotation_owner_ops_t;

int xuiInternalCodeAnnotationClaimOwner(xui_code_annotation_store store, xui_widget editor,
    xui_code_document document, const xui_code_annotation_owner_ops_t* ops);
void xuiInternalCodeAnnotationDestroyOwned(xui_code_annotation_store store, xui_widget editor);
int xuiInternalCodeAnnotationCheckDocument(xui_code_annotation_store store, xui_code_document document);
void xuiInternalCodeAnnotationNotify(xui_code_annotation_store store, bool diagnostics);

/* Inside a checked native operation, batch indicator changes and invalidate once. */
int xuiInternalCodeAnnotationSetIndicator(xui_code_annotation_store store, int indicator, int style,
    int start, int end, uint32_t flags, uintptr_t userData);
int xuiInternalCodeAnnotationClearIndicators(xui_code_annotation_store store, int indicator);

/* A changed candidate becomes visible only after the optional editor is marked
 * dirty. Equal candidates and validation failures skip the invalidation. */
int xuiInternalCodeAnnotationApplyDiagnostics(xui_code_annotation_store store,
    const xui_code_diagnostic_t* diagnostics, int count, xui_widget editor);

/* A hover hit copies numeric fields. The immutable string storage can outlive
 * a reentrant annotation update while the selected callback reads the hit. */
typedef struct xui_code_diagnostic_text_t xui_code_diagnostic_text_t;
int xuiInternalCodeAnnotationPinDiagnosticText(xui_code_annotation_store store,
    xui_code_diagnostic_text_t** text);
void xuiInternalCodeAnnotationUnpinDiagnosticText(xui_code_diagnostic_text_t* text);

#endif
