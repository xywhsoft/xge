#ifndef XUI_CODE_DOCUMENT_LIFETIME_INTERNAL_H
#define XUI_CODE_DOCUMENT_LIFETIME_INTERNAL_H
#include "xui_internal.h"
/* A single owning CodeEdit observes its unique Document. GetDocument is a
 * borrow; this protocol adds no shared/Attach ownership semantics. */
typedef int (*xui_internal_code_document_prepare_proc)(xui_code_document,int,int,const char*,int,bool,void*);
int xuiInternalCodeDocumentSetEditObserverChecked(xui_code_document,xui_context,
    xui_internal_code_document_prepare_proc,xui_internal_code_document_edit_proc,void*);
int xuiInternalCodeDocumentClaimOwner(xui_code_document,xui_widget);
int xuiInternalCodeDocumentDestroyOwned(xui_widget,xui_code_document);
/* Counted internal edit over exact UTF-8 byte boundaries (not grapheme-clamped).
 * Native transformations share public validation, history, observers and the
 * checked write lifetime; public C-string edits keep their grapheme behavior. */
int xuiInternalCodeDocumentReplaceLength(xui_code_document,int,int,const char*,int);
/* An editor owns its selection and binds it to exactly one document. */
int xuiInternalCodeSelectionClaimOwner(xui_code_selection_model selection, xui_widget editor, xui_code_document document);
void xuiInternalCodeSelectionDestroyOwned(xui_widget editor, xui_code_selection_model selection);
int xuiInternalCodeSelectionPrepare(xui_code_selection_model selection, xui_code_document document);
void xuiInternalCodeSelectionClampLength(xui_code_selection_model selection, int length);

#endif
