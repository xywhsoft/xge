#ifndef XUI_DOCUMENT_IMAGE_RESOURCE_INTERNAL_H
#define XUI_DOCUMENT_IMAGE_RESOURCE_INTERNAL_H

#include "../xui_document_ui.h"

int doc_image_limits(const xui_doc_image_load_limits_t* limits,
    uint64_t* max_encoded, uint64_t* max_pixels);
/* With valid context/name/surface inputs, takes ownership on success and failure. */
int doc_image_resource_register_surface(xui_context context, const char* name,
    xui_surface surface, xui_resource* out);
/* Returns an xrtFree-owned PNG buffer, or NOT_FOUND for an unresolved name. */
int doc_image_resource_export_png(xui_context context, const char* name,
    void** out, size_t* bytes);

#endif
