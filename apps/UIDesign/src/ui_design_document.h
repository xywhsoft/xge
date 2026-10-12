#ifndef UI_DESIGN_DOCUMENT_H
#define UI_DESIGN_DOCUMENT_H

#include "ui_design_model.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_DESIGN_DOCUMENT_FORMAT "xui.uidesign"
#define UI_DESIGN_DOCUMENT_VERSION 1
#define UI_DESIGN_DOCUMENT_MAX_BYTES (16u * 1024u * 1024u)

int uiDesignDocumentSaveModel(const ui_design_model_t* pModel, char** ppSnapshot);
/* Persistent content only: selection belongs to the editing session. */
int uiDesignDocumentSaveContent(const ui_design_model_t* pModel, char** ppSnapshot);
int uiDesignDocumentLoadModel(const char* sSnapshot, ui_design_model_t** ppModel);

#ifdef __cplusplus
}
#endif

#endif
