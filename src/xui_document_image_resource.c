#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_internal.h"
#include "xui_document_image_resource_internal.h"
#include "../xge.h"
#include <limits.h>

#define DOC_IMAGE_DEFAULT_ENCODED_BYTES (UINT64_C(16) * 1024 * 1024)
#define DOC_IMAGE_DEFAULT_PIXELS UINT64_C(16000000)

int doc_image_limits(const xui_doc_image_load_limits_t* limits,
    uint64_t* max_encoded, uint64_t* max_pixels)
{
    if (limits && limits->iSize < sizeof(*limits)) return XUI_ERROR_INVALID_ARGUMENT;
    *max_encoded = limits && limits->iMaxEncodedBytes ? limits->iMaxEncodedBytes : DOC_IMAGE_DEFAULT_ENCODED_BYTES;
    *max_pixels = limits && limits->iMaxPixels ? limits->iMaxPixels : DOC_IMAGE_DEFAULT_PIXELS;
    if (*max_encoded > INT_MAX) *max_encoded = INT_MAX;
    return XUI_OK;
}

static void doc_image_surface_destroy(xui_context context, void* handle, void* user)
{
    xui_proxy proxy = xuiInternalContextGetProxy(context);
    (void)user;
    if (proxy && proxy->surfaceDestroy && handle) proxy->surfaceDestroy(proxy, (xui_surface)handle);
}

int doc_image_resource_register_surface(xui_context context, const char* name,
    xui_surface surface, xui_resource* out)
{
    xui_proxy proxy = xuiInternalContextGetProxy(context);
    xui_resource_desc_t resource_desc = {0}; int result;
    if (out) *out = NULL;
    if (!proxy || !proxy->surfaceDestroy || !surface || !name || !*name) return XUI_ERROR_INVALID_ARGUMENT;
    resource_desc.iSize = sizeof(resource_desc);
    resource_desc.sName = name;
    resource_desc.iKind = XUI_RESOURCE_SURFACE;
    resource_desc.pHandle = surface;
    resource_desc.onDestroy = doc_image_surface_destroy;
    result = xuiResourceSet(context, out, &resource_desc);
    if (result != XUI_OK) proxy->surfaceDestroy(proxy, surface);
    return result;
}

int doc_image_resource_export_png(xui_context context, const char* name,
    void** out, size_t* bytes)
{
    xui_proxy proxy = xuiInternalContextGetProxy(context);
    xui_resource resource;
    xui_surface surface;
    xui_surface_desc_t desc = {0};
    unsigned char* pixels;
    size_t stride, pixel_bytes;
    int result;
    if (!out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL; *bytes = 0;
    if (!name || !*name || !proxy || !proxy->surfaceGetDesc || !proxy->surfaceReadRGBA)
        return XUI_ERROR_NOT_FOUND;
    resource = xuiResourceFind(context, name);
    if (!resource || xuiResourceGetKind(resource) != XUI_RESOURCE_SURFACE)
        return XUI_ERROR_NOT_FOUND;
    surface = (xui_surface)xuiResourceGetHandle(resource);
    if (!surface || proxy->surfaceGetDesc(proxy, surface, &desc) != XUI_OK ||
        desc.iWidth <= 0 || desc.iHeight <= 0) return XUI_ERROR_NOT_FOUND;
    if ((uint64_t)desc.iWidth > DOC_IMAGE_DEFAULT_PIXELS / (uint64_t)desc.iHeight ||
        desc.iWidth > (INT_MAX - 1) / 4) return XUI_DOC_ERROR_LIMIT;
    stride = (size_t)desc.iWidth * 4u;
    if (stride > SIZE_MAX / (size_t)desc.iHeight) return XUI_DOC_ERROR_LIMIT;
    pixel_bytes = stride * (size_t)desc.iHeight;
    pixels = (unsigned char*)xrtMalloc(pixel_bytes);
    if (!pixels) return XUI_ERROR_OUT_OF_MEMORY;
    result = proxy->surfaceReadRGBA(proxy, surface, pixels, (int)stride);
    if (result == XUI_OK)
        result = xgeImageEncodePNGEx(desc.iWidth, desc.iHeight, pixels,
            (int)stride, (desc.iFlags & XUI_SURFACE_ALPHA_PREMULTIPLIED) ?
            XGE_IMAGE_PREMULTIPLIED : XGE_IMAGE_STRAIGHT_ALPHA, out, bytes);
    xrtFree(pixels);
    if (result == XUI_OK && *bytes > DOC_IMAGE_DEFAULT_ENCODED_BYTES) {
        xrtFree(*out); *out = NULL; *bytes = 0;
        return XUI_DOC_ERROR_LIMIT;
    }
    return result;
}

XUI_API int xuiDocumentImageResourceLoadMemory(xui_context context, const char* resource_name,
    const void* encoded, uint64_t encoded_bytes, const xui_doc_image_load_limits_t* limits,
    xui_resource* out)
{
    xui_proxy proxy; xui_surface surface = NULL; xui_surface_desc_t surface_desc = {0};
    uint64_t max_encoded, max_pixels; int result;
    if (out) *out = NULL;
    if (!resource_name || !*resource_name || !encoded || !encoded_bytes) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_image_limits(limits, &max_encoded, &max_pixels);
    if (result != XUI_OK) return result;
    if (encoded_bytes > max_encoded) return XUI_DOC_ERROR_LIMIT;
    proxy = xuiInternalContextGetProxy(context);
    if (!proxy || !proxy->surfaceLoadMemory || !proxy->surfaceGetDesc || !proxy->surfaceDestroy)
        return XUI_ERROR_NOT_INITIALIZED;
    result = proxy->surfaceLoadMemory(proxy, &surface, encoded, (int)encoded_bytes, 0);
    if (result != XUI_OK) goto failed;
    if (!surface || proxy->surfaceGetDesc(proxy, surface, &surface_desc) != XUI_OK ||
        surface_desc.iWidth <= 0 || surface_desc.iHeight <= 0) {
        result = XUI_ERROR_BACKEND_FAILED; goto failed;
    }
    if ((uint64_t)surface_desc.iWidth > max_pixels / (uint64_t)surface_desc.iHeight) {
        result = XUI_DOC_ERROR_LIMIT; goto failed;
    }
    return doc_image_resource_register_surface(context, resource_name, surface, out);
failed:
    if (surface) proxy->surfaceDestroy(proxy, surface);
    return result;
}

XUI_API int xuiDocumentImageResourceLoadFile(xui_context context, const char* resource_name,
    const char* authorized_path, const xui_doc_image_load_limits_t* limits, xui_resource* out)
{
    uint64_t max_encoded, max_pixels; unsigned char* encoded;
    size_t bytes = 0; int result;
    if (out) *out = NULL;
    if (!resource_name || !*resource_name || !authorized_path || !*authorized_path)
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_image_limits(limits, &max_encoded, &max_pixels);
    if (result != XUI_OK) return result;
    if (!xuiInternalContextGetProxy(context)) return XUI_ERROR_NOT_INITIALIZED;
    encoded = xrtFileReadAllLimit(authorized_path, (size_t)max_encoded, &bytes);
    if (!encoded) return xrtErrorFind(xrtGetError(), "xrt.file", XFILE_ERROR_LIMIT) ?
        XUI_DOC_ERROR_LIMIT : XUI_DOC_ERROR_IO;
    (void)max_pixels;
    result = xuiDocumentImageResourceLoadMemory(context, resource_name, encoded, bytes, limits, out);
    xrtFree(encoded);
    return result;
}

#endif
