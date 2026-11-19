#ifndef XUI_CODE_DIAGNOSTICS_INTERNAL_H
#define XUI_CODE_DIAGNOSTICS_INTERNAL_H

#include "xui.h"

/* A changed candidate becomes visible only after the optional editor is marked
 * dirty. Equal candidates and validation failures skip the invalidation. */
int xuiInternalCodeAnnotationApplyDiagnostics(xui_code_annotation_store store,
    const xui_code_diagnostic_t* diagnostics, int count, xui_widget editor);

#endif
