#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"
#include "xui_code_token_internal.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

typedef struct xui_code_token_storage_t {
	size_t refs;
	int capacity;
	xui_code_token_t tokens[];
} xui_code_token_storage_t;

struct xui_code_token_buffer_t {
	xui_code_token_storage_t* storage;
	xui_widget owner;
	const xui_code_token_owner_ops_t* ops;
	uint64_t ownerThread;
	int count;
	uint32_t version;
	int start;
	int end;
};

struct xui_code_token_snapshot_t {
	size_t refs;
	xui_code_token_storage_t* storage;
	int first;
	int count;
	uint32_t version;
	int start;
	int end;
};

static void __xuiCodeTokenStorageRelease(xui_code_token_storage_t* storage)
{
	if ( storage != NULL && --storage->refs == 0 ) xrtFree(storage);
}

static int __xuiCodeTokenBufferStatus(xui_code_token_buffer buffer, bool write)
{
	if ( buffer == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( buffer->owner == NULL ) return XUI_OK;
	if ( buffer->ownerThread != xrtThreadCurrentId() ) return XUI_ERROR_BUSY;
	return buffer->ops->check(buffer->owner, write);
}

static void __xuiCodeTokenBufferNotify(xui_code_token_buffer buffer)
{
	if ( buffer->owner != NULL ) buffer->ops->changed(buffer->owner);
}

int xuiInternalCodeTokenBufferClaimOwner(xui_code_token_buffer buffer, xui_widget editor,
	const xui_code_token_owner_ops_t* ops)
{
	if ( buffer == NULL || editor == NULL || ops == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( buffer->owner != NULL ) return XUI_ERROR_INVALID_STATE;
	buffer->ownerThread = xrtThreadCurrentId();
	buffer->ops = ops;
	buffer->owner = editor;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferEditorLifetimeWatch(xui_code_token_buffer buffer,
	xui_lifetime_link_t* link, xui_lifetime_invalidate_proc invalidate, void* user)
{
	int status = __xuiCodeTokenBufferStatus(buffer, false);
	if ( status != XUI_OK ) return status;
	if ( buffer->owner == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return buffer->ops->watch(buffer->owner, link, invalidate, user);
}

XUI_API int xuiCodeTokenBufferCreate(xui_code_token_buffer* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = xrtCalloc(1, sizeof(**out));
	return *out != NULL ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

void xuiInternalCodeTokenBufferDestroyOwned(xui_code_token_buffer buffer, xui_widget editor)
{
	if ( buffer == NULL || (buffer->owner != NULL && buffer->owner != editor) ) return;
	__xuiCodeTokenStorageRelease(buffer->storage);
	xrtFree(buffer);
}

XUI_API int xuiCodeTokenBufferDestroyChecked(xui_code_token_buffer buffer)
{
	int status = __xuiCodeTokenBufferStatus(buffer, true);
	if ( status != XUI_OK ) return status;
	if ( buffer->owner != NULL ) return XUI_ERROR_INVALID_STATE;
	xuiInternalCodeTokenBufferDestroyOwned(buffer, NULL);
	return XUI_OK;
}

XUI_API void xuiCodeTokenBufferDestroy(xui_code_token_buffer buffer)
{
	if ( buffer != NULL ) (void)xuiCodeTokenBufferDestroyChecked(buffer);
}

void xuiInternalCodeTokenBufferClear(xui_code_token_buffer buffer)
{
	if ( buffer == NULL ) return;
	buffer->count = 0;
	buffer->version = 0;
	buffer->start = 0;
	buffer->end = 0;
}

XUI_API int xuiCodeTokenBufferClearChecked(xui_code_token_buffer buffer)
{
	int status = __xuiCodeTokenBufferStatus(buffer, true);
	if ( status != XUI_OK ) return status;
	if ( buffer->count || buffer->version || buffer->start || buffer->end ) {
		xuiInternalCodeTokenBufferClear(buffer);
		__xuiCodeTokenBufferNotify(buffer);
	}
	return XUI_OK;
}

XUI_API void xuiCodeTokenBufferClear(xui_code_token_buffer buffer)
{
	if ( buffer != NULL ) (void)xuiCodeTokenBufferClearChecked(buffer);
}

static int __xuiCodeTokenBufferWrite(xui_code_token_buffer buffer, const xui_code_token_t* tokens, int count)
{
	xui_code_token_storage_t* old = buffer->storage;
	bool unique = old != NULL && old->refs == 1;
	if ( unique && count <= old->capacity ) {
		memmove(old->tokens, tokens, sizeof(*tokens) * (size_t)count);
		return XUI_OK;
	}
	int capacity = unique ? old->capacity : 16;
	while ( capacity < count ) {
		if ( capacity > INT_MAX / 2 ) {
			capacity = count;
			break;
		}
		capacity *= 2;
	}
	if ( (size_t)capacity > (SIZE_MAX - sizeof(*old)) / sizeof(*tokens) ) return XUI_ERROR_LIMIT_EXCEEDED;
	size_t bytes = sizeof(*old) + (size_t)capacity * sizeof(*tokens);
	xui_code_token_storage_t* next = unique ? xrtRealloc(old, bytes) : xrtMalloc(bytes);
	if ( next == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	next->refs = 1;
	next->capacity = capacity;
	memcpy(next->tokens, tokens, sizeof(*tokens) * (size_t)count);
	buffer->storage = next;
	if ( !unique ) __xuiCodeTokenStorageRelease(old);
	return XUI_OK;
}

int xuiInternalCodeTokenBufferSetRange(xui_code_token_buffer buffer, const xui_code_token_t* tokens,
	int count, uint32_t version, int start, int end)
{
	if ( buffer == NULL || count < 0 || (count > 0 && tokens == NULL) || start < 0 || end < start )
		return XUI_ERROR_INVALID_ARGUMENT;
	for ( int i = 0; i < count; ++i ) {
		if ( tokens[i].iStartOffset < start || tokens[i].iEndOffset <= tokens[i].iStartOffset ||
		     tokens[i].iEndOffset > end ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( i > 0 && (tokens[i].iStartOffset < tokens[i - 1].iStartOffset ||
		     tokens[i].iEndOffset < tokens[i - 1].iEndOffset) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( count > 0 ) {
		int status = __xuiCodeTokenBufferWrite(buffer, tokens, count);
		if ( status != XUI_OK ) return status;
	}
	buffer->count = count;
	buffer->version = version;
	buffer->start = start;
	buffer->end = end;
	return XUI_OK;
}

static int __xuiCodeTokenBufferPublish(xui_code_token_buffer buffer, const xui_code_token_t* tokens,
	int count, uint32_t version, int start, int end, bool full)
{
	int status = __xuiCodeTokenBufferStatus(buffer, true);
	if ( status != XUI_OK ) return status;
	if ( buffer->owner != NULL ) {
		int extent;
		status = buffer->ops->getExtent(buffer->owner, version, &extent);
		if ( status != XUI_OK ) return status;
		if ( full ) end = extent;
		if ( end > extent ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	status = xuiInternalCodeTokenBufferSetRange(buffer, tokens, count, version, start, end);
	if ( status == XUI_OK ) __xuiCodeTokenBufferNotify(buffer);
	return status;
}

XUI_API int xuiCodeTokenBufferSet(xui_code_token_buffer buffer, const xui_code_token_t* tokens,
	int count, uint32_t version)
{
	return __xuiCodeTokenBufferPublish(buffer, tokens, count, version, 0, INT_MAX, true);
}

XUI_API int xuiCodeTokenBufferSetRange(xui_code_token_buffer buffer, const xui_code_token_t* tokens,
	int count, uint32_t version, int start, int end)
{
	return __xuiCodeTokenBufferPublish(buffer, tokens, count, version, start, end, false);
}

XUI_API int xuiCodeTokenBufferGetVersion(xui_code_token_buffer buffer, uint32_t* version)
{
	if ( buffer == NULL || version == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*version = buffer->version;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetRange(xui_code_token_buffer buffer, uint32_t* version, int* start, int* end)
{
	if ( buffer == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( version != NULL ) *version = buffer->version;
	if ( start != NULL ) *start = buffer->start;
	if ( end != NULL ) *end = buffer->end;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetCount(xui_code_token_buffer buffer)
{
	return buffer != NULL ? buffer->count : 0;
}

XUI_API int xuiCodeTokenBufferGetTokens(xui_code_token_buffer buffer, uint32_t version,
	xui_code_token_t* tokens, int capacity, int* count)
{
	if ( buffer == NULL || count == NULL || capacity < 0 || (tokens == NULL && capacity > 0) )
		return XUI_ERROR_INVALID_ARGUMENT;
	if ( version != buffer->version ) {
		*count = 0;
		return XUI_ERROR_UNSUPPORTED;
	}
	int copyCount = buffer->count < capacity ? buffer->count : capacity;
	if ( copyCount > 0 ) memcpy(tokens, buffer->storage->tokens, sizeof(*tokens) * (size_t)copyCount);
	*count = buffer->count;
	return XUI_OK;
}

static int __xuiCodeTokenBufferSelect(xui_code_token_buffer buffer, uint32_t version,
	int start, int end, int* first, int* count)
{
	*first = 0;
	*count = 0;
	if ( version != buffer->version || start < buffer->start || end > buffer->end )
		return XUI_ERROR_UNSUPPORTED;
	if ( start == end || buffer->count == 0 ) return XUI_OK;
	const xui_code_token_t* tokens = buffer->storage->tokens;
	int low = 0, high = buffer->count;
	while ( low < high ) {
		int mid = low + (high - low) / 2;
		if ( tokens[mid].iEndOffset <= start ) low = mid + 1;
		else high = mid;
	}
	*first = low;
	high = buffer->count;
	while ( low < high ) {
		int mid = low + (high - low) / 2;
		if ( tokens[mid].iStartOffset < end ) low = mid + 1;
		else high = mid;
	}
	*count = low - *first;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetTokensInRange(xui_code_token_buffer buffer, uint32_t version,
	int start, int end, xui_code_token_t* tokens, int capacity, int* count)
{
	if ( buffer == NULL || count == NULL || start < 0 || end < start || capacity < 0 ||
	     (tokens == NULL && capacity > 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	int first;
	int status = __xuiCodeTokenBufferSelect(buffer, version, start, end, &first, count);
	if ( status != XUI_OK ) return status;
	int copyCount = *count < capacity ? *count : capacity;
	if ( copyCount > 0 ) memcpy(tokens, buffer->storage->tokens + first, sizeof(*tokens) * (size_t)copyCount);
	return XUI_OK;
}

static int __xuiCodeTokenSnapshotCreate(xui_code_token_buffer buffer, int start, int end,
	int first, int count, xui_code_token_snapshot* out)
{
	xui_code_token_storage_t* storage = count > 0 ? buffer->storage : NULL;
	if ( storage != NULL && storage->refs == SIZE_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	xui_code_token_snapshot snapshot = xrtMalloc(sizeof(*snapshot));
	if ( snapshot == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	*snapshot = (struct xui_code_token_snapshot_t){1, storage, first, count, buffer->version, start, end};
	if ( storage != NULL ) ++storage->refs;
	*out = snapshot;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferCapture(xui_code_token_buffer buffer, uint32_t version, xui_code_token_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	int status = __xuiCodeTokenBufferStatus(buffer, false);
	if ( status != XUI_OK ) return status;
	if ( version != buffer->version ) return XUI_ERROR_UNSUPPORTED;
	return __xuiCodeTokenSnapshotCreate(buffer, buffer->start, buffer->end, 0, buffer->count, out);
}

XUI_API int xuiCodeTokenBufferCaptureInRange(xui_code_token_buffer buffer, uint32_t version,
	int start, int end, xui_code_token_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	int status = __xuiCodeTokenBufferStatus(buffer, false);
	if ( status != XUI_OK ) return status;
	if ( start < 0 || end < start ) return XUI_ERROR_INVALID_ARGUMENT;
	int first, count;
	status = __xuiCodeTokenBufferSelect(buffer, version, start, end, &first, &count);
	if ( status != XUI_OK ) return status;
	return __xuiCodeTokenSnapshotCreate(buffer, start, end, first, count, out);
}

XUI_API int xuiCodeTokenSnapshotAddRef(xui_code_token_snapshot snapshot)
{
	if ( snapshot == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( snapshot->refs == SIZE_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	++snapshot->refs;
	return XUI_OK;
}

XUI_API void xuiCodeTokenSnapshotDestroy(xui_code_token_snapshot snapshot)
{
	if ( snapshot == NULL || --snapshot->refs != 0 ) return;
	__xuiCodeTokenStorageRelease(snapshot->storage);
	xrtFree(snapshot);
}

XUI_API int xuiCodeTokenSnapshotGetRange(const struct xui_code_token_snapshot_t* snapshot,
	uint32_t* version, int* start, int* end)
{
	if ( snapshot == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( version != NULL ) *version = snapshot->version;
	if ( start != NULL ) *start = snapshot->start;
	if ( end != NULL ) *end = snapshot->end;
	return XUI_OK;
}

XUI_API int xuiCodeTokenSnapshotGetTokens(const struct xui_code_token_snapshot_t* snapshot,
	const xui_code_token_t** tokens, size_t* count)
{
	if ( snapshot == NULL || tokens == NULL || count == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*tokens = snapshot->storage != NULL ? snapshot->storage->tokens + snapshot->first : NULL;
	*count = (size_t)snapshot->count;
	return XUI_OK;
}

#endif
