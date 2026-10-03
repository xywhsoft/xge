#ifndef XUI_DOCUMENT_FIND_UI_H
#define XUI_DOCUMENT_FIND_UI_H
#include "../xui_document_ui.h"

typedef struct doc_find_ui doc_find_ui;
int doc_find_ui_open(xui_widget editor, doc_find_ui** slot, int replace);
void doc_find_ui_destroy(doc_find_ui* ui);
void doc_find_ui_update(doc_find_ui* ui);
xui_widget doc_find_ui_window(doc_find_ui* ui);

#endif
