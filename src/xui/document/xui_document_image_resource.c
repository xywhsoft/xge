#include "xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_internal.h"
#include "xui_document_image_resource_internal.h"
#include "xge.h"
#include <limits.h>

#define DOC_IMAGE_DEFAULT_ENCODED_BYTES (UINT64_C(16) * 1024 * 1024)
#define DOC_IMAGE_DEFAULT_PIXELS UINT64_C(16000000)
static void doc_image_context_gone(void* raw){*(bool*)raw=false;}

static xui_proxy doc_image_proxy(xui_context context)
{
    if(!xuiInternalContextIsValid(context))return NULL;
    return context->pOwnedProxy ? context->pOwnedProxy:xuiInternalContextGetProxy(context);
}
static int doc_image_surface_release(xui_context context,xui_surface surface)
{
    if(!surface)return XUI_OK;
    if(context->pOwnedProxy){
        bool consumed=false;
        /* Same-cap creator outputs, outside Ownership Freeze, are valid
         * Surface owners. Kind3 release consumes that owner even with pins or
         * other owners; negative semantic cleanup remains a checked result. */
        return xuiSurfaceReleaseOwnerChecked(surface,&consumed);
    }
    xui_proxy proxy=doc_image_proxy(context);proxy->surfaceDestroy(proxy,surface);
    return xrtGetError() ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
}

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
    xui_proxy proxy = doc_image_proxy(context);
    (void)user;
    if (proxy && proxy->surfaceDestroy && handle) proxy->surfaceDestroy(proxy, (xui_surface)handle);
}

int doc_image_resource_register_surface(xui_context context, const char* name,
    xui_surface surface, xui_resource* out)
{
    xui_proxy proxy = doc_image_proxy(context);
    xui_resource_desc_t resource_desc = {0}; int result;
    if (out) *out = NULL;
    if (!proxy || !proxy->surfaceDestroy || !surface || !name || !*name || context->iOperationDepth<=0)
        return XUI_ERROR_INVALID_ARGUMENT;
    resource_desc.iSize = sizeof(resource_desc);
    resource_desc.sName = name;
    resource_desc.iKind = XUI_RESOURCE_SURFACE;
    resource_desc.pHandle = surface;
    bool managed=context->pOwnedProxy!=NULL;
    /* Invalid caller inputs retain their original owner. */
    if(managed && (!xuiInternalProxyCreator(proxy) || xuiSurfaceGetProxy(surface)!=proxy))return XUI_ERROR_INVALID_ARGUMENT;
    resource_desc.onDestroy=managed ? NULL:doc_image_surface_destroy;
    result=xuiResourceSet(context,out,&resource_desc);
    xerror* first=xrtTakeError();if(result==XUI_OK && first)result=XUI_ERROR_CALLBACK_FAILED;
    xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(context,&prefix,result,first);
    /* Managed registry retains its own reference. The producer's original
     * owner is consumed on both success and rollback, exactly once. */
    if(managed)xuiInternalContextCleanupOutcome(context,doc_image_surface_release(context,surface),&result,&first);
    else if(result!=XUI_OK){proxy->surfaceDestroy(proxy,surface);xuiInternalContextCleanupOutcome(context,XUI_OK,&result,&first);}
    xuiInternalCleanupPrefixEnd(&prefix);xrtSetErrorTake(first);return result;
}

int doc_image_resource_export_png(xui_context context,const char* name,void** out,size_t* bytes)
{
    if(!out || !bytes)return XUI_ERROR_INVALID_ARGUMENT;
    *out=NULL;*bytes=0;
    xui_call_scope_t scope={0};int result=xuiCallScopeBegin(context,&scope);if(result!=XUI_OK)return result;
    xui_proxy proxy=doc_image_proxy(context);xui_surface surface=NULL;bool held=false;
    unsigned char* pixels=NULL;xui_surface_desc_t desc={0};size_t stride,pixel_bytes;
    if(!name || !*name || !proxy || !proxy->surfaceGetDesc || !proxy->surfaceReadRGBA){result=XUI_ERROR_NOT_FOUND;goto end;}
    xui_resource resource=xuiResourceFind(context,name);
    if(!resource || xuiResourceGetKind(resource)!=XUI_RESOURCE_SURFACE){result=XUI_ERROR_NOT_FOUND;goto end;}
    surface=(xui_surface)xuiResourceGetHandle(resource);
    if(!surface){result=XUI_ERROR_NOT_FOUND;goto end;}
    if(context->pOwnedProxy){result=xuiSurfaceAddRef(surface);if(result!=XUI_OK)goto end;held=true;}
    result=proxy->surfaceGetDesc(proxy,surface,&desc);if(result!=XUI_OK)goto end;
    if(desc.iWidth<=0 || desc.iHeight<=0){result=XUI_ERROR_NOT_FOUND;goto end;}
    if((uint64_t)desc.iWidth>DOC_IMAGE_DEFAULT_PIXELS/(uint64_t)desc.iHeight || desc.iWidth>(INT_MAX-1)/4){result=XUI_DOC_ERROR_LIMIT;goto end;}
    stride=(size_t)desc.iWidth*4u;
    if(stride>SIZE_MAX/(size_t)desc.iHeight){result=XUI_DOC_ERROR_LIMIT;goto end;}
    pixel_bytes=stride*(size_t)desc.iHeight;pixels=xrtMalloc(pixel_bytes);
    if(!pixels){result=XUI_ERROR_OUT_OF_MEMORY;goto end;}
    result=proxy->surfaceReadRGBA(proxy,surface,pixels,(int)stride);
    if(result==XUI_OK)result=xgeImageEncodePNGEx(desc.iWidth,desc.iHeight,pixels,(int)stride,
        desc.iFlags&XUI_SURFACE_ALPHA_PREMULTIPLIED ? XGE_IMAGE_PREMULTIPLIED:XGE_IMAGE_STRAIGHT_ALPHA,out,bytes);
    if(result==XUI_OK && *bytes>DOC_IMAGE_DEFAULT_ENCODED_BYTES)result=XUI_DOC_ERROR_LIMIT;
    if(result==XUI_OK && (context->bCloseRequested || context->bDestroyPending || context->bDestroying))result=XUI_ERROR_INVALID_STATE;
end: ;
    xerror* first=xrtTakeError();if(result==XUI_OK && first)result=XUI_ERROR_CALLBACK_FAILED;
    xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(context,&prefix,result,first);
    xrtFree(pixels);
    if(held)xuiInternalContextCleanupOutcome(context,doc_image_surface_release(context,surface),&result,&first);
    xuiInternalCleanupPrefixEnd(&prefix);
    result=xuiInternalCallScopeEndOutcome(&scope,result,&first);
    if(result!=XUI_OK){xrtFree(*out);*out=NULL;*bytes=0;}
    xrtSetErrorTake(first);return result;
}

static int doc_image_load_memory(xui_context context, const char* resource_name,
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
    proxy = doc_image_proxy(context);
    if (!proxy || !proxy->surfaceLoadMemory || !proxy->surfaceGetDesc || !proxy->surfaceDestroy)
        return XUI_ERROR_NOT_INITIALIZED;
    result = proxy->surfaceLoadMemory(proxy, &surface, encoded, (int)encoded_bytes, 0);
    if (result != XUI_OK) goto failed;
    if(!surface){result=XUI_ERROR_BACKEND_FAILED;goto failed;}
    if(context->pOwnedProxy && xuiSurfaceGetProxy(surface)!=proxy){result=XUI_ERROR_INVALID_ARGUMENT;goto failed;}
    result=proxy->surfaceGetDesc(proxy,surface,&surface_desc);if(result!=XUI_OK)goto failed;
    if(surface_desc.iWidth<=0 || surface_desc.iHeight<=0){result=XUI_ERROR_BACKEND_FAILED;goto failed;}
    if ((uint64_t)surface_desc.iWidth > max_pixels / (uint64_t)surface_desc.iHeight) {
        result = XUI_DOC_ERROR_LIMIT; goto failed;
    }
    return doc_image_resource_register_surface(context, resource_name, surface, out);
failed:
    {
        xerror* first=xrtTakeError();
        xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(context,&prefix,result,first);
        if(surface){
            if(context->pOwnedProxy)xuiInternalContextCleanupOutcome(context,doc_image_surface_release(context,surface),&result,&first);
            else {proxy->surfaceDestroy(proxy,surface);xuiInternalContextCleanupOutcome(context,XUI_OK,&result,&first);}
        }
        xuiInternalCleanupPrefixEnd(&prefix);xrtSetErrorTake(first);return result;
    }
}

XUI_API int xuiDocumentImageResourceLoadMemory(xui_context context,const char* resource_name,
    const void* encoded,uint64_t encoded_bytes,const xui_doc_image_load_limits_t* limits,xui_resource* out)
{
    if(out)*out=NULL;
    xui_call_scope_t scope={0};int result=xuiCallScopeBegin(context,&scope);if(result!=XUI_OK)return result;
    bool alive=true;xui_lifetime_link_t watch={0};
    result=xuiContextLifetimeWatch(context,&watch,doc_image_context_gone,&alive);
    if(result!=XUI_OK){xerror* first=xrtTakeError();result=xuiInternalCallScopeEndOutcome(&scope,result,&first);xrtSetErrorTake(first);return result;}
    result=doc_image_load_memory(context,resource_name,encoded,encoded_bytes,limits,out);
    if(result==XUI_OK && (context->bCloseRequested || context->bDestroyPending || context->bDestroying)){if(out)*out=NULL;result=XUI_ERROR_INVALID_STATE;}
    xerror* first=xrtTakeError();result=xuiInternalCallScopeEndOutcome(&scope,result,&first);xuiLifetimeUnwatch(&watch);
    if(result==XUI_OK && (!alive || context->bCloseRequested || context->bDestroyPending || context->bDestroying))result=XUI_ERROR_INVALID_STATE;
    if(result!=XUI_OK && out)*out=NULL;
    xrtSetErrorTake(first);return result;
}

static int doc_image_load_file(xui_context context, const char* resource_name,
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

XUI_API int xuiDocumentImageResourceLoadFile(xui_context context,const char* name,const char* path,const xui_doc_image_load_limits_t* limits,xui_resource* out)
{
    if(out)*out=NULL;
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(context,&scope);if(status!=XUI_OK)return status;
    bool alive=true;xui_lifetime_link_t watch={0};
    status=xuiContextLifetimeWatch(context,&watch,doc_image_context_gone,&alive);
    if(status!=XUI_OK){xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;}
    status=doc_image_load_file(context,name,path,limits,out);
    if(status==XUI_OK && (context->bCloseRequested || context->bDestroyPending || context->bDestroying)){if(out)*out=NULL;status=XUI_ERROR_INVALID_STATE;}
    xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xuiLifetimeUnwatch(&watch);
    if(status==XUI_OK && (!alive || context->bCloseRequested || context->bDestroyPending || context->bDestroying))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK && out)*out=NULL;
    xrtSetErrorTake(first);return status;
}

#endif
