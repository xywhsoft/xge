#include "xui_document_internal.h"

XUI_API int xuiDocumentSnapshotExportFile(xui_document_snapshot snapshot, const char* path, uint32_t format)
{
    char* data = NULL; uint64_t length = 0; int result;
    if (!snapshot || !path || !*path || !doc_utf8(path, strlen(path))) return XUI_ERROR_INVALID_ARGUMENT;
    switch (format) {
    case XUI_DOC_FILE_NATIVE: result = xuiDocumentSerialize(snapshot, &data, &length); break;
    case XUI_DOC_FILE_TEXT: result = xuiDocumentSnapshotCopyPlainText(snapshot, &data, &length); break;
    case XUI_DOC_FILE_HTML: result = xuiDocumentExportHtml(snapshot, &data, &length); break;
    case XUI_DOC_FILE_MARKDOWN:
        result = xuiDocumentSnapshotCopySource(snapshot, NULL, 0, &length);
        if (result != XUI_OK) break;
        if (length >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
        data = malloc((size_t)length + 1); if (!data) return XUI_ERROR_OUT_OF_MEMORY;
        result = xuiDocumentSnapshotCopySource(snapshot, data, length + 1, &length); break;
    default: return XUI_ERROR_INVALID_ARGUMENT;
    }
    if (result == XUI_OK) {
        xbytesview bytes = {(const unsigned char*)data, (size_t)length};
        if (length > SIZE_MAX) result = XUI_DOC_ERROR_LIMIT;
        else if (!xrtFileWriteAtomic(path, bytes)) result = XUI_DOC_ERROR_IO;
    }
    xuiDocumentFreeBuffer(data); return result;
}
XUI_API int xuiDocumentSaveFile(xui_document document, const char* path, uint32_t format)
{
    xui_document_snapshot snapshot; int result;
    if (!document || (format != XUI_DOC_FILE_NATIVE && format != XUI_DOC_FILE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentAcquireSnapshot(document, &snapshot); if (result != XUI_OK) return result;
    result = xuiDocumentSnapshotExportFile(snapshot, path, format);
    if (result == XUI_OK) result = xuiDocumentMarkSaved(document, snapshot);
    xuiDocumentSnapshotRelease(snapshot); return result;
}
XUI_API int xuiDocumentOpenFile(const char* path, uint32_t format, const xui_doc_desc_t* desc, uint64_t max_bytes, xui_document* out)
{
    unsigned char* data; size_t length; xui_document document = NULL; xui_doc_desc_t options = {0}; int result;
    if (out) *out = NULL;
    if (!out || !path || !*path || !doc_utf8(path, strlen(path)) || (desc && desc->iSize != sizeof(*desc)) ||
        (format != XUI_DOC_FILE_NATIVE && format != XUI_DOC_FILE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!max_bytes) max_bytes = UINT64_C(256) * 1024 * 1024;
    if (max_bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    data = xrtFileReadAllLimit(path, (size_t)max_bytes, &length); if (!data) return XUI_DOC_ERROR_IO;
    if (format == XUI_DOC_FILE_NATIVE) result = xuiDocumentDeserialize(desc, (const char*)data, length, &document);
    else {
        if (desc) options = *desc;
        options.iSize = sizeof(options); options.iProfile = XUI_DOCUMENT_MARKDOWN; options.bDisableHistory = 1;
        result = xuiDocumentCreate(&options, &document);
        if (result == XUI_OK) result = xuiDocumentLoadMarkdown(document, (const char*)data, length);
        if (result == XUI_OK) { document->disable_history = desc ? desc->bDisableHistory : 0; document->saved_state = document->state->content_id; }
    }
    xrtFree(data);
    if (result != XUI_OK) xuiDocumentRelease(document); else *out = document;
    return result;
}
