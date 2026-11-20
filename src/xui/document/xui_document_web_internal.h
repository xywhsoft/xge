#ifndef XUI_DOCUMENT_WEB_INTERNAL_H
#define XUI_DOCUMENT_WEB_INTERNAL_H

#include "xui_document_ui.h"

#if XUI_ENABLE_WEBVIEW
int doc_web_watch_renderer(xui_doc_web_provider provider, xui_context context,
    xui_lifetime_link_t* link, xui_lifetime_invalidate_proc invalidate, void* user);
uint64_t doc_web_object_generation(xui_doc_web_provider provider);
#endif

#endif
