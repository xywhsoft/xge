#ifndef XUI_DOCUMENT_WEB_INTERNAL_H
#define XUI_DOCUMENT_WEB_INTERNAL_H

#include "xui_document_ui.h"

#if XUI_ENABLE_WEBVIEW
int doc_web_watch_renderer(xui_doc_web_provider provider, xui_context context,
    xui_lifetime_link_t* lifetime, xui_lifetime_link_t* changes,
    xui_lifetime_invalidate_proc gone, xui_lifetime_invalidate_proc changed, void* user);
uint64_t doc_web_object_generation(xui_doc_web_provider provider);
/* Internal observers may only mark native layout or paint state dirty. */
int doc_render_set_web_provider(xui_document_renderer renderer, xui_doc_web_provider provider,
    xui_lifetime_invalidate_proc changed, void* user);
bool doc_render_web_objects_pending(xui_document_renderer renderer);
#endif

#endif
