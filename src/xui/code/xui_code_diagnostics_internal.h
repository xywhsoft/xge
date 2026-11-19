#ifndef XUI_CODE_DIAGNOSTICS_INTERNAL_H
#define XUI_CODE_DIAGNOSTICS_INTERNAL_H

#include "xui.h"

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
