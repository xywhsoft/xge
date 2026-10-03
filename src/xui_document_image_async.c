#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_internal.h"
#include "xui_document_image_resource_internal.h"
#include "../xge.h"
#include <stdatomic.h>
#include <limits.h>
#include <string.h>

#define DOC_IMAGE_MAX_WORKERS 4u
#define DOC_IMAGE_MAX_REQUESTS 64u
#define DOC_IMAGE_MAX_COPIED_BYTES (UINT64_C(64) * 1024 * 1024)

struct xui_doc_image_request_t {
    xui_context context;
    char *name, *path;
    unsigned char* encoded;
    size_t encoded_bytes;
    uint64_t max_encoded, max_pixels;
    uint32_t base_generation, published_generation;
    atomic_int cancelled;
    xthread* worker;
    xge_image_t image;
    int worker_result, final_result, terminal, started, busy, polling, release_pending;
    struct xui_doc_image_request_t* next;
};
/* All list operations are on the XUI owner thread. Workers only read their
 * immutable job fields and the atomic cancellation flag. */
static xui_doc_image_request doc_image_requests;
static int doc_image_pumping;

static char* doc_image_copy_string(const char* value)
{
    size_t bytes = strlen(value); char* copy;
    if (bytes == SIZE_MAX) return NULL;
    copy = (char*)xrtMalloc(bytes + 1);
    if (copy) memcpy(copy, value, bytes + 1);
    return copy;
}

static int32 doc_image_worker(ptr user)
{
    xui_doc_image_request job = (xui_doc_image_request)user;
    unsigned char* encoded = job->encoded; size_t bytes = job->encoded_bytes;
    int width = 0, height = 0, file_bytes = job->path != NULL;
    uint64_t pixel_limit = job->max_pixels < (uint64_t)INT_MAX / 4 ?
        job->max_pixels : (uint64_t)INT_MAX / 4;
    int result = XUI_OK;
    if (atomic_load_explicit(&job->cancelled, memory_order_acquire)) {
        job->worker_result = XUI_DOC_ERROR_CANCELLED; return 0;
    }
    if (file_bytes) encoded = xrtFileReadAllLimit(job->path, (size_t)job->max_encoded, &bytes);
    if (!encoded) {
        result = xrtErrorFind(xrtGetError(), "xrt.file", XFILE_ERROR_LIMIT) ?
            XUI_DOC_ERROR_LIMIT : XUI_DOC_ERROR_IO;
    } else if (atomic_load_explicit(&job->cancelled, memory_order_acquire)) {
        result = XUI_DOC_ERROR_CANCELLED;
    } else if (!bytes || xgeImageInfoMemory(encoded, (int)bytes, &width, &height) != XGE_OK) {
        result = XUI_DOC_ERROR_FORMAT;
    } else if ((uint64_t)width > pixel_limit / (uint64_t)height) {
        result = XUI_DOC_ERROR_LIMIT;
    } else if (xgeImageLoadMemoryEx(&job->image, encoded, (int)bytes, XGE_IMAGE_PREMULTIPLIED) != XGE_OK) {
        result = XUI_DOC_ERROR_FORMAT;
    } else if (job->image.iWidth != width || job->image.iHeight != height ||
        job->image.iFormat != XGE_PIXEL_RGBA8 || !job->image.pPixels) {
        result = XUI_ERROR_BACKEND_FAILED;
    }
    if (file_bytes) xrtFree(encoded);
    job->worker_result = result;
    return 0;
}

static void doc_image_request_dispose(xui_doc_image_request job)
{
    xui_doc_image_request* link;
    for (link = &doc_image_requests; *link; link = &(*link)->next) {
        if (*link == job) { *link = job->next; break; }
    }
    if (job->worker) {
        atomic_store_explicit(&job->cancelled, 1, memory_order_release);
        (void)xrtThreadWait(job->worker);
        xrtThreadDestroy(job->worker);
    }
    xgeImageFree(&job->image);
    xrtFree(job->encoded); xrtFree(job->path); xrtFree(job->name); xrtFree(job);
}

static void doc_image_start_queued(xui_context context)
{
    xui_doc_image_request scan, oldest; unsigned running = 0;
    for (scan = doc_image_requests; scan; scan = scan->next)
        if (scan->context == context && scan->worker) running++;
    while (running < DOC_IMAGE_MAX_WORKERS) {
        oldest = NULL;
        for (scan = doc_image_requests; scan; scan = scan->next) {
            if (scan->context != context || scan->started || scan->terminal) continue;
            if (atomic_load_explicit(&scan->cancelled, memory_order_acquire)) {
                scan->terminal = 1; scan->final_result = XUI_DOC_ERROR_CANCELLED;
                xrtFree(scan->encoded); scan->encoded = NULL; scan->encoded_bytes = 0;
            } else oldest = scan;
        }
        if (!oldest) break;
        oldest->started = 1;
        oldest->worker = xrtThreadCreate(doc_image_worker, oldest, 0);
        if (!oldest->worker) {
            oldest->terminal = 1; oldest->final_result = XUI_ERROR_OUT_OF_MEMORY;
            xrtFree(oldest->encoded); oldest->encoded = NULL; oldest->encoded_bytes = 0;
        } else running++;
    }
}

static int doc_image_admission(xui_context context, const char* name, uint64_t new_bytes)
{
    xui_doc_image_request previous;
    uint64_t copied = new_bytes; unsigned outstanding = 0;
    if (copied > DOC_IMAGE_MAX_COPIED_BYTES) return XUI_DOC_ERROR_LIMIT;
    for (previous = doc_image_requests; previous; previous = previous->next) {
        if (previous->context != context || previous->terminal) continue;
        /* An unstarted older request for this name can be replaced without
         * consuming another queue slot or copied-input budget. */
        if (!previous->started && !strcmp(previous->name, name)) continue;
        outstanding++;
        if (previous->encoded_bytes > DOC_IMAGE_MAX_COPIED_BYTES - copied) return XUI_DOC_ERROR_LIMIT;
        copied += previous->encoded_bytes;
    }
    if (outstanding >= DOC_IMAGE_MAX_REQUESTS || copied > DOC_IMAGE_MAX_COPIED_BYTES)
        return XUI_DOC_ERROR_LIMIT;
    return XUI_OK;
}

static int doc_image_request_launch(xui_doc_image_request job, xui_doc_image_request* out)
{
    xui_doc_image_request previous; xui_resource existing;
    int result = doc_image_admission(job->context, job->name, job->encoded_bytes);
    if (result != XUI_OK) return result;
    existing = xuiResourceFind(job->context, job->name);
    job->base_generation = existing ? xuiResourceGetGeneration(existing) : 0;
    atomic_init(&job->cancelled, 0);
    for (previous = doc_image_requests; previous; previous = previous->next) {
        if (previous->terminal || previous->context != job->context || strcmp(previous->name, job->name)) continue;
        atomic_store_explicit(&previous->cancelled, 1, memory_order_release);
        if (!previous->started) {
            previous->terminal = 1; previous->final_result = XUI_DOC_ERROR_CANCELLED;
            xrtFree(previous->encoded); previous->encoded = NULL; previous->encoded_bytes = 0;
        }
    }
    job->next = doc_image_requests; doc_image_requests = job;
    if (!doc_image_pumping) doc_image_start_queued(job->context);
    *out = job; return XUI_OK;
}

XUI_API int xuiDocumentImageResourceLoadFileAsync(xui_context context, const char* resource_name,
    const char* authorized_path, const xui_doc_image_load_limits_t* limits, xui_doc_image_request* out)
{
    xui_doc_image_request job; xui_proxy proxy;
    uint64_t max_encoded, max_pixels; int result;
    if (out) *out = NULL;
    if (!out || !resource_name || !*resource_name || !authorized_path || !*authorized_path)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_image_pumping) return XUI_DOC_ERROR_BUSY;
    result = doc_image_limits(limits, &max_encoded, &max_pixels);
    if (result != XUI_OK) return result;
    proxy = xuiInternalContextGetProxy(context);
    if (!proxy || !proxy->surfaceCreateRGBA || !proxy->surfaceDestroy) return XUI_ERROR_NOT_INITIALIZED;
    result = doc_image_admission(context, resource_name, 0);
    if (result != XUI_OK) return result;
    job = (xui_doc_image_request)xrtCalloc(1, sizeof(*job));
    if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    job->name = doc_image_copy_string(resource_name);
    job->path = doc_image_copy_string(authorized_path);
    if (!job->name || !job->path) { result = XUI_ERROR_OUT_OF_MEMORY; goto failed; }
    job->context = context; job->max_encoded = max_encoded; job->max_pixels = max_pixels;
    result = doc_image_request_launch(job, out);
    if (result == XUI_OK) return XUI_OK;
failed:
    xrtFree(job->path); xrtFree(job->name); xrtFree(job);
    return result;
}

XUI_API int xuiDocumentImageResourceLoadMemoryAsync(xui_context context, const char* resource_name,
    const void* encoded, uint64_t encoded_bytes, const xui_doc_image_load_limits_t* limits,
    xui_doc_image_request* out)
{
    xui_doc_image_request job; xui_proxy proxy;
    uint64_t max_encoded, max_pixels; int result;
    if (out) *out = NULL;
    if (!out || !resource_name || !*resource_name || !encoded || !encoded_bytes)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_image_pumping) return XUI_DOC_ERROR_BUSY;
    result = doc_image_limits(limits, &max_encoded, &max_pixels);
    if (result != XUI_OK) return result;
    if (encoded_bytes > max_encoded) return XUI_DOC_ERROR_LIMIT;
    proxy = xuiInternalContextGetProxy(context);
    if (!proxy || !proxy->surfaceCreateRGBA || !proxy->surfaceDestroy) return XUI_ERROR_NOT_INITIALIZED;
    result = doc_image_admission(context, resource_name, encoded_bytes);
    if (result != XUI_OK) return result;
    job = (xui_doc_image_request)xrtCalloc(1, sizeof(*job));
    if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    job->name = doc_image_copy_string(resource_name);
    job->encoded = (unsigned char*)xrtMalloc((size_t)encoded_bytes);
    if (!job->name || !job->encoded) { result = XUI_ERROR_OUT_OF_MEMORY; goto failed; }
    memcpy(job->encoded, encoded, (size_t)encoded_bytes);
    job->encoded_bytes = (size_t)encoded_bytes;
    job->context = context; job->max_encoded = max_encoded; job->max_pixels = max_pixels;
    result = doc_image_request_launch(job, out);
    if (result == XUI_OK) return XUI_OK;
failed:
    xrtFree(job->encoded); xrtFree(job->name); xrtFree(job);
    return result;
}

static void doc_image_finish(xui_doc_image_request job)
{
    xui_proxy proxy = NULL; xui_surface surface = NULL; xui_resource current;
    int result;
    xrtThreadDestroy(job->worker); job->worker = NULL;
    job->busy = 1;
    result = job->worker_result;
    if (atomic_load_explicit(&job->cancelled, memory_order_acquire)) result = XUI_DOC_ERROR_CANCELLED;
    if (result == XUI_OK) {
        current = xuiResourceFind(job->context, job->name);
        if ((current ? xuiResourceGetGeneration(current) : 0) != job->base_generation)
            result = XUI_DOC_ERROR_STALE;
    }
    if (result == XUI_OK) {
        proxy = xuiInternalContextGetProxy(job->context);
        if (!proxy || !proxy->surfaceCreateRGBA) result = XUI_ERROR_NOT_INITIALIZED;
        else result = proxy->surfaceCreateRGBA(proxy, &surface, job->image.iWidth, job->image.iHeight,
            job->image.pPixels, job->image.iStride, XUI_SURFACE_ALPHA_PREMULTIPLIED);
        if (result == XUI_OK) {
            result = doc_image_resource_register_surface(job->context, job->name, surface, NULL);
            surface = NULL;
            if (result == XUI_OK) {
                current = xuiResourceFind(job->context, job->name);
                job->published_generation = current ? xuiResourceGetGeneration(current) : 0;
            }
        }
        if (surface && proxy && proxy->surfaceDestroy) proxy->surfaceDestroy(proxy, surface);
    }
    xgeImageFree(&job->image);
    xrtFree(job->encoded); job->encoded = NULL;
    job->encoded_bytes = 0;
    job->terminal = 1; job->final_result = result;
    job->busy = 0;
}

static void doc_image_pump(xui_context context, xui_doc_image_request protected_job)
{
    xui_doc_image_request scan, completed;
    if (doc_image_pumping) return;
    doc_image_pumping = 1;
    for (;;) {
        completed = NULL;
        for (scan = doc_image_requests; scan; scan = scan->next) {
            if (scan->context == context && scan->worker &&
                xrtThreadWaitFor(scan->worker, 0) == XWAIT_OK) { completed = scan; break; }
        }
        if (!completed) break;
        doc_image_finish(completed);
        if (completed->release_pending && completed != protected_job)
            doc_image_request_dispose(completed);
    }
    doc_image_start_queued(context);
    doc_image_pumping = 0;
}

XUI_API int xuiDocumentImageResourcePoll(xui_doc_image_request job, xui_resource* out)
{
    xui_resource current; int result;
    if (out) *out = NULL;
    if (!job) return XUI_ERROR_INVALID_ARGUMENT;
    if (job->polling) return XUI_DOC_ERROR_BUSY;
    job->polling = 1;
    doc_image_pump(job->context, job);
    result = job->busy || !job->terminal ? XUI_DOC_ERROR_BUSY : job->final_result;
    if (out && result == XUI_OK) {
        current = xuiResourceFind(job->context, job->name);
        if (current && xuiResourceGetGeneration(current) == job->published_generation) *out = current;
    }
    job->polling = 0;
    if (job->release_pending) doc_image_request_dispose(job);
    return result;
}

XUI_API int xuiDocumentImageResourceCancel(xui_doc_image_request job)
{
    if (!job) return XUI_ERROR_INVALID_ARGUMENT;
    if (job->terminal || job->busy) return XUI_ERROR_UNSUPPORTED;
    atomic_store_explicit(&job->cancelled, 1, memory_order_release);
    if (!job->started) {
        job->terminal = 1; job->final_result = XUI_DOC_ERROR_CANCELLED;
        xrtFree(job->encoded); job->encoded = NULL; job->encoded_bytes = 0;
    }
    return XUI_OK;
}

XUI_API void xuiDocumentImageResourceRelease(xui_doc_image_request job)
{
    xui_context context;
    if (!job) return;
    if (job->busy || job->polling) { job->release_pending = 1; return; }
    context = job->context;
    doc_image_request_dispose(job);
    if (!doc_image_pumping) doc_image_start_queued(context);
}

XUI_API int xuiDocumentImageResourceGetAsyncStats(xui_context context, xui_doc_image_async_stats_t* stats)
{
    xui_doc_image_request job; xui_doc_image_async_stats_t value = {0};
    if (!stats || stats->iSize < sizeof(*stats) || !xuiInternalContextGetProxy(context))
        return XUI_ERROR_INVALID_ARGUMENT;
    value.iSize = sizeof(value);
    for (job = doc_image_requests; job; job = job->next) {
        if (job->context != context || job->terminal) continue;
        value.iOutstanding++;
        if (job->worker) value.iRunning++;
        else if (!job->started) value.iQueued++;
        value.iCopiedBytes += job->encoded_bytes;
    }
    *stats = value;
    return XUI_OK;
}

#endif
