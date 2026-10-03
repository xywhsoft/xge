#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <assert.h>

static atomic_uint_fast64_t doc_identity = 1;
static void* doc_default_alloc(void* user, size_t bytes) { (void)user; return malloc(bytes); }
static void doc_default_free(void* user, void* pointer) { (void)user; free(pointer); }

void doc_history_retain(doc_history* h) { if (h) atomic_fetch_add(&h->refs, 1); }
void doc_history_release(doc_history* h)
{
    if (!h || atomic_fetch_sub(&h->refs, 1) != 1) return;
    assert(!h->accounted);
    doc_state_release(h->before); doc_state_release(h->after);
    doc_free(h->ops); doc_free(h);
}
static void doc_history_free(doc_history* h)
{
    if (!h) return;
    if (h->accounted) doc_memory_history(h, 0);
    doc_history_release(h);
}
static void doc_history_clear(doc_history** head)
{
    while (*head) { doc_history* p = *head; *head = p->next; doc_history_free(p); }
}
static void doc_history_trim(xui_document d)
{
    while ((uint64_t)d->undo_count + d->redo_count > d->history_limit ||
        doc_memory_history_bytes(d->allocator) > d->history_max_bytes) {
        doc_history **tail, *dead; unsigned* count;
        if (d->undo_count >= d->redo_count && d->undo_count) { tail = &d->undo; count = &d->undo_count; }
        else { tail = &d->redo; count = &d->redo_count; }
        if (!*tail) break;
        while ((*tail)->next) tail = &(*tail)->next;
        if (!d->disable_history && !d->standby_history && d->undo_count + d->redo_count == 1) {
            /* Transfer the last real owner's current root to the empty-history
             * standby before releasing it. Shared children need no walk. */
            doc_memory_standby(d->state, 1); d->standby_history = 1;
        }
        dead = *tail; *tail = NULL; (*count)--; doc_history_free(dead);
    }
}
XUI_API int xuiDocumentSetHistoryLimits(xui_document d, uint32_t max_steps, uint64_t max_bytes)
{
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    d->history_limit = max_steps ? max_steps : 256;
    d->history_max_bytes = max_bytes ? max_bytes : DOC_DEFAULT_HISTORY_BYTES;
    doc_history_trim(d);
    if (!d->disable_history && !d->standby_history && !d->undo && !d->redo) {
        doc_memory_standby(d->state, 1); d->standby_history = 1;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentClearHistory(xui_document d)
{
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    if (!d->disable_history && !d->standby_history) {
        doc_memory_standby(d->state, 1); d->standby_history = 1;
    }
    doc_history_clear(&d->undo); doc_history_clear(&d->redo); d->undo_count = d->redo_count = 0;
    return XUI_OK;
}

XUI_API int xuiDocumentCreate(const xui_doc_desc_t* desc, xui_document* out)
{
    xui_document d;
    doc_allocator* a;
    uint32_t profile = desc ? desc->iProfile : XUI_DOCUMENT_RICH;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if ((desc && desc->iSize != sizeof(*desc)) ||
        (profile != XUI_DOCUMENT_RICH && profile != XUI_DOCUMENT_MARKDOWN) ||
        (desc && desc->iMarkdownDialect > XUI_MD_EXTENDED) ||
        (desc && (!!desc->onAlloc != !!desc->onFree))) return XUI_ERROR_INVALID_ARGUMENT;
    a = calloc(1, sizeof(*a));
    if (!a) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&a->refs, 1); atomic_init(&a->live, 0); atomic_init(&a->peak, 0);
    atomic_init(&a->allocations, 0); atomic_init(&a->sequence, 1);
    atomic_init(&a->markdown_parses, 0); atomic_init(&a->markdown_parsed_bytes, 0);
    atomic_init(&a->markdown_incremental_parses, 0);
    atomic_init(&a->prepared_publishes, 0); atomic_init(&a->prepared_storage_updates, 0);
    atomic_init(&a->prepared_accounting_visits, 0);
    atomic_init(&a->next_node_id, 2);
    atomic_init(&a->memory_lock, 0); atomic_init(&a->attribute_lock, 0);
    a->alloc = desc && desc->onAlloc ? desc->onAlloc : doc_default_alloc;
    a->free = desc && desc->onFree ? desc->onFree : doc_default_free;
    a->user = desc ? desc->pAllocatorUser : NULL;
    d = doc_alloc(a, sizeof(*d));
    if (!d) { doc_allocator_release(a); return XUI_ERROR_OUT_OF_MEMORY; }
    atomic_init(&d->refs, 1); d->allocator = a;
    d->identity = atomic_fetch_add(&doc_identity, 1);
    d->revision = d->next_state = d->saved_state = 1;
    d->history_limit = desc && desc->iHistoryLimit ? desc->iHistoryLimit : 256;
    d->history_max_bytes = desc && desc->iHistoryMaxBytes ? desc->iHistoryMaxBytes : DOC_DEFAULT_HISTORY_BYTES;
    d->disable_history = desc ? desc->bDisableHistory : 0;
    d->max_bytes = desc && desc->iMaxTextBytes ? desc->iMaxTextBytes : UINT64_C(256) * 1024 * 1024;
    d->max_nodes = desc && desc->iMaxNodes ? desc->iMaxNodes : 1000000;
    a->max_bytes = d->max_bytes; a->max_nodes = d->max_nodes;
    d->state = doc_state_new(a, profile);
    if (d->state && profile == XUI_DOCUMENT_MARKDOWN) d->state->dialect = desc && desc->iMarkdownDialect ? desc->iMarkdownDialect : XUI_MD_EXTENDED;
    if (!d->state) { xuiDocumentRelease(d); return XUI_ERROR_OUT_OF_MEMORY; }
    d->state->content_id = 1; doc_memory_current(NULL, d->state);
    if (!d->disable_history) { doc_memory_standby(d->state, 1); d->standby_history = 1; }
    *out = d; return XUI_OK;
}
XUI_API void xuiDocumentRetain(xui_document d) { if (d) atomic_fetch_add(&d->refs, 1); }
XUI_API void xuiDocumentRelease(xui_document d)
{
    doc_allocator* a;
    doc_observer* o;
    if (!d || atomic_fetch_sub(&d->refs, 1) != 1) return;
    a = d->allocator;
    doc_prepare_invalidate(d, NULL, XUI_DOC_ERROR_CANCELLED);
    if (d->standby_history) doc_memory_standby(d->state, 0);
    doc_history_clear(&d->undo); doc_history_clear(&d->redo);
    doc_memory_current(d->state, NULL);
    doc_state_release(d->state);
    while ((o = d->observers) != NULL) { d->observers = o->next; doc_free(o); }
    doc_free(d); doc_allocator_release(a);
}
XUI_API uint64_t xuiDocumentGetRevision(xui_document d) { return d ? d->revision : 0; }
XUI_API uint64_t xuiDocumentGetIdentity(xui_document d) { return d ? d->identity : 0; }
XUI_API int xuiDocumentGetProfile(xui_document d) { return d ? (int)d->state->profile : 0; }
XUI_API int xuiDocumentGetMarkdownDialect(xui_document d) { return d ? (int)d->state->dialect : 0; }
XUI_API int xuiDocumentAcquireSnapshot(xui_document d, xui_document_snapshot* out)
{
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_snapshot_create(d->state, d->identity, d->revision, out);
}
XUI_API void xuiDocumentSnapshotRetain(xui_document_snapshot s) { if (s) atomic_fetch_add(&s->refs, 1); }
XUI_API void xuiDocumentSnapshotRelease(xui_document_snapshot s)
{
    doc_state* state;
    if (!s || atomic_fetch_sub(&s->refs, 1) != 1) return;
    doc_snapshot_unregister(s);
    state = s->state; doc_free(s); doc_state_release(state);
}
XUI_API uint64_t xuiDocumentSnapshotGetRevision(xui_document_snapshot s) { return s ? s->revision : 0; }
XUI_API uint64_t xuiDocumentSnapshotGetIdentity(xui_document_snapshot s) { return s ? s->identity : 0; }
XUI_API int xuiDocumentSnapshotGetNode(xui_document_snapshot s, uint64_t id, xui_doc_node_info_t* out)
{
    return s ? doc_node_info(s->state, doc_index_get(s->state->index, id), out) : XUI_ERROR_INVALID_ARGUMENT;
}
XUI_API int xuiDocumentSnapshotGetChild(xui_document_snapshot s, uint64_t parent, uint64_t index, uint64_t* out)
{
    doc_node* p;
    if (!s || !out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = 0; p = doc_index_get(s->state->index, parent);
    if (!p) return XUI_ERROR_NOT_FOUND;
    *out = doc_seq_get_id(p->children, index);
    return *out ? XUI_OK : XUI_ERROR_NOT_FOUND;
}
static int doc_copy_sequence(doc_sequence* p, char* buffer, uint64_t capacity, uint64_t* length)
{
    uint64_t size = doc_seq_size(p);
    if (!length) return XUI_ERROR_INVALID_ARGUMENT;
    *length = size;
    if (!buffer) return XUI_OK;
    if (capacity <= size || capacity > SIZE_MAX) return XUI_ERROR_BUFFER_TOO_SMALL;
    if (doc_seq_read(p, 0, buffer, size) != XUI_OK) return XUI_ERROR_INVALID_ARGUMENT;
    buffer[size] = 0; return XUI_OK;
}
XUI_API int xuiDocumentSnapshotCopyText(xui_document_snapshot s, uint64_t id, char* buffer, uint64_t capacity, uint64_t* length)
{
    doc_node* node;
    if (!s) return XUI_ERROR_INVALID_ARGUMENT;
    node = doc_index_get(s->state->index, id);
    return node ? doc_copy_sequence(node->text, buffer, capacity, length) : XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentSnapshotCopyExtensionPayload(xui_document_snapshot s,
    uint64_t id, void* buffer, uint64_t capacity, uint64_t* length)
{
    doc_node* node;
    uint64_t bytes;
    if (!s || !length || (!buffer && capacity)) return XUI_ERROR_INVALID_ARGUMENT;
    node = doc_index_get(s->state->index, id);
    if (!node) return XUI_ERROR_NOT_FOUND;
    if (node->kind != XUI_DOC_EXTENSION) return XUI_ERROR_INVALID_ARGUMENT;
    bytes = node->extension_payload ? node->extension_payload->size : 0;
    *length = bytes;
    if (!buffer) return XUI_OK;
    if (capacity < bytes) return XUI_ERROR_BUFFER_TOO_SMALL;
    if (bytes) memcpy(buffer, node->extension_payload->data, (size_t)bytes);
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotCopySource(xui_document_snapshot s, char* buffer, uint64_t capacity, uint64_t* length)
{
    if (!s) return XUI_ERROR_INVALID_ARGUMENT;
    if (s->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    return doc_copy_sequence(s->state->source, buffer, capacity, length);
}
XUI_API int xuiDocumentSnapshotReadText(xui_document_snapshot s, uint64_t id, uint64_t offset, void* buffer, uint64_t bytes)
{
    doc_node* n;
    if (!s) return XUI_ERROR_INVALID_ARGUMENT;
    n = doc_index_get(s->state->index, id);
    return n ? doc_seq_read(n->text, offset, buffer, bytes) : XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentSnapshotReadSource(xui_document_snapshot s, uint64_t offset, void* buffer, uint64_t bytes)
{
    if (!s || s->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_seq_read(s->state->source, offset, buffer, bytes);
}
XUI_API int xuiDocumentSubscribe(xui_document d, xui_doc_change_proc callback, void* user, uint64_t* token)
{
    doc_observer* o;
    if (!d || !callback || !token) return XUI_ERROR_INVALID_ARGUMENT;
    *token = 0;
    if (d->notifying) return XUI_DOC_ERROR_BUSY;
    o = doc_alloc(d->allocator, sizeof(*o));
    if (!o) return XUI_ERROR_OUT_OF_MEMORY;
    o->token = ++d->next_observer; o->callback = callback; o->user = user;
    o->next = d->observers; d->observers = o; *token = o->token;
    return XUI_OK;
}
static void doc_observers_collect(xui_document d)
{
    doc_observer** it = &d->observers;
    while (*it) {
        if ((*it)->removed) { doc_observer* dead = *it; *it = dead->next; doc_free(dead); }
        else it = &(*it)->next;
    }
}
XUI_API void xuiDocumentUnsubscribe(xui_document d, uint64_t token)
{
    doc_observer* o;
    if (!d) return;
    for (o = d->observers; o; o = o->next) if (o->token == token) o->removed = 1;
    if (!d->notifying) doc_observers_collect(d);
}
static void doc_notify(xui_document d, xui_document_change_set change)
{
    doc_observer* o;
    xuiDocumentRetain(d); xuiDocumentChangeSetRetain(change);
    d->notifying = 1;
    for (o = d->observers; o; o = o->next) if (!o->removed) o->callback(d, change, o->user);
    d->notifying = 0; doc_observers_collect(d);
    xuiDocumentChangeSetRelease(change); xuiDocumentRelease(d);
}
XUI_API int xuiDocumentGetStats(xui_document d, xui_doc_stats_t* out)
{
    if (!d || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    out->iLiveBytes = atomic_load(&d->allocator->live);
    out->iPeakBytes = atomic_load(&d->allocator->peak);
    out->iAllocations = atomic_load(&d->allocator->allocations);
    out->iNodes = d->state->node_count; out->iUndoCount = d->undo_count; out->iRedoCount = d->redo_count;
    out->iMarkdownParses = atomic_load(&d->allocator->markdown_parses);
    out->iMarkdownParsedBytes = atomic_load(&d->allocator->markdown_parsed_bytes);
    out->iMarkdownIncrementalParses = atomic_load(&d->allocator->markdown_incremental_parses);
    out->iPreparedAccountingVisits = atomic_load(&d->allocator->prepared_accounting_visits);
    out->iPreparedPublishes = atomic_load(&d->allocator->prepared_publishes);
    out->iPreparedStorageUpdates = atomic_load(&d->allocator->prepared_storage_updates);
    out->iHistoryMaxBytes = d->history_max_bytes; doc_memory_stats(d->allocator, out);
    return XUI_OK;
}
XUI_API int xuiDocumentIsDirty(xui_document d) { return d && d->state->content_id != d->saved_state; }
XUI_API int xuiDocumentMarkSaved(xui_document d, xui_document_snapshot saved)
{
    if (!d || !saved || d->identity != saved->identity) return XUI_ERROR_INVALID_ARGUMENT;
    d->saved_state = saved->state->content_id; return XUI_OK;
}

int doc_txn_fail(xui_document_transaction t, int error)
{
    if (t && !t->closed && !t->error && error != XUI_OK) t->error = error;
    return error;
}
int doc_txn_check(xui_document_transaction t, int domain)
{
    int cancelled;
    if (!t || t->closed) return XUI_ERROR_INVALID_STATE;
    if (t->error) return t->error;
    if (t->cancellation && (cancelled = atomic_load(t->cancellation)) != 0) return doc_txn_fail(t, cancelled);
    if (domain && t->domain != (uint32_t)domain && !t->parsing) return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    return XUI_OK;
}
int doc_txn_op(xui_document_transaction t, const xui_doc_operation_t* op)
{
    if (t->parsing == DOC_BUILD_PARSE) return XUI_OK;
    if (t->count == t->capacity) {
        uint64_t capacity = t->capacity ? t->capacity * 2 : 16;
        xui_doc_operation_t* ops;
        if (capacity < t->capacity || capacity > SIZE_MAX / sizeof(*ops)) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        ops = doc_alloc(t->draft->allocator, (size_t)capacity * sizeof(*ops));
        if (!ops) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
        if (t->count) memcpy(ops, t->ops, (size_t)t->count * sizeof(*ops));
        doc_free(t->ops); t->ops = ops; t->capacity = capacity;
    }
    t->ops[t->count++] = *op; t->flags |= op->iFlags; return XUI_OK;
}
XUI_API int xuiDocumentBeginTransaction(xui_document d, const xui_doc_txn_desc_t* desc, xui_document_transaction* out)
{
    xui_document_transaction t;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!d || (desc && (desc->iSize != sizeof(*desc) ||
        (desc->iDomain != XUI_DOC_SOURCE && desc->iDomain != XUI_DOC_SEMANTIC)))) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    if (desc && desc->iBaseRevision && desc->iBaseRevision != d->revision) return XUI_DOC_ERROR_STALE;
    if (desc && desc->iDomain == XUI_DOC_SOURCE && d->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    t = doc_alloc(d->allocator, sizeof(*t));
    if (!t) return XUI_ERROR_OUT_OF_MEMORY;
    t->draft = doc_state_clone(d->state);
    if (!t->draft) { doc_free(t); return XUI_ERROR_OUT_OF_MEMORY; }
    t->document = d; xuiDocumentRetain(d);
    t->base = d->state; doc_state_retain(t->base); t->base_revision = d->revision;
    t->domain = desc ? desc->iDomain : XUI_DOC_SEMANTIC;
    t->origin = desc ? desc->iOrigin : 0; t->group = desc ? desc->iGroup : 0;
    d->writer = t; *out = t; return XUI_OK;
}
XUI_API int xuiDocumentTxnGetNode(xui_document_transaction t, uint64_t id, xui_doc_node_info_t* out)
{
    int result = doc_txn_check(t, 0);
    return result == XUI_OK ? doc_node_info(t->draft, doc_index_get(t->draft->index, id), out) : result;
}
static int doc_parent_replace(xui_document_transaction t, uint64_t parent_id, uint64_t start, uint64_t end, uint64_t child)
{
    doc_node *parent = doc_index_get(t->draft->index, parent_id), *copy;
    doc_sequence *insert = NULL, *children = NULL;
    int result;
    if (!parent) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    copy = doc_node_clone(t->draft->allocator, parent);
    if (!copy) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    if (child) {
        insert = doc_seq_id(t->draft->allocator, child);
        if (!insert) { doc_node_release(copy); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    }
    result = doc_seq_replace(t->draft->allocator, parent->children, start, end, insert, &children);
    doc_seq_release(insert);
    if (result == XUI_OK) {
        doc_seq_release(copy->children); copy->children = children;
        result = doc_state_set(t->draft, copy);
    }
    doc_node_release(copy); return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
int doc_txn_insert(xui_document_transaction t, uint64_t parent_id, uint64_t index, const xui_doc_node_desc_t* desc, uint64_t* out)
{
    doc_node *parent, *node, *ancestor;
    unsigned depth = 0;
    uint64_t id;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    xui_doc_operation_t op = {0};
    if (out) *out = 0;
    if (result != XUI_OK) return result;
    if (!desc || desc->iSize != sizeof(*desc) || !out) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    parent = doc_index_get(t->draft->index, parent_id);
    if (!parent) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    if (index == DOC_NONE) index = doc_seq_size(parent->children);
    if (index > doc_seq_size(parent->children)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_schema_child(parent->kind, desc->iKind) || !doc_schema_attrs(desc->iKind, &desc->tAttributes) ||
        (!doc_text_kind(desc->iKind) && desc->iTextBytes)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (desc->iTextBytes > t->draft->allocator->max_bytes || t->draft->text_bytes > t->draft->allocator->max_bytes - desc->iTextBytes || t->draft->node_count >= t->draft->allocator->max_nodes) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (!doc_utf8(desc->sText, desc->iTextBytes)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if ((desc->sResource && !doc_utf8(desc->sResource, strlen(desc->sResource))) ||
        (desc->sInfo && !doc_utf8(desc->sInfo, strlen(desc->sInfo))) ||
        (desc->sTitle && !doc_utf8(desc->sTitle, strlen(desc->sTitle))) ||
        (desc->sLinkTarget && !doc_utf8(desc->sLinkTarget, strlen(desc->sLinkTarget))) ||
        (desc->sLinkTitle && !doc_utf8(desc->sLinkTitle, strlen(desc->sLinkTitle))))
        return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if ((desc->sLinkTarget || desc->sLinkTitle) &&
        (desc->iKind != XUI_DOC_IMAGE || !(desc->tAttributes.iMarks & XUI_DOC_LINK)))
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (desc->iKind == XUI_DOC_EXTENSION) {
        if ((desc->iExtensionPayloadBytes && !desc->pExtensionPayload) ||
            (desc->bExtensionRequired != 0 && desc->bExtensionRequired != 1) ||
            ((desc->iExtensionPayloadBytes || desc->bExtensionRequired) &&
                (!desc->sInfo || !*desc->sInfo)))
            return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        if (desc->iExtensionPayloadBytes > DOC_EXTENSION_MAX_PAYLOAD_BYTES)
            return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    } else if (desc->pExtensionPayload || desc->iExtensionPayloadBytes ||
        desc->iExtensionVersion || desc->bExtensionRequired)
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    for (ancestor = parent; ancestor; ancestor = doc_index_get(t->draft->index, ancestor->parent))
        if (++depth >= DOC_MAX_DEPTH) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    id = doc_node_id_next(t->draft->allocator);
    if (!id) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    node = doc_node_new(t->draft->allocator, id, parent_id, desc);
    if (!node) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = doc_state_set(t->draft, node); doc_node_release(node);
    if (result == XUI_OK) result = doc_parent_replace(t, parent_id, index, index, id);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_INSERT; op.iFlags = XUI_DOC_CHANGE_STRUCTURE;
    op.iNodeId = id; op.iParentId = parent_id; op.iOffset = index; op.iNewLength = 1;
    result = doc_txn_op(t, &op);
    if (result == XUI_OK) *out = id;
    return result;
}
XUI_API int xuiDocumentTxnInsertNode(xui_document_transaction t, uint64_t parent, uint64_t index, const xui_doc_node_desc_t* desc, uint64_t* out)
{
    return doc_txn_insert(t, parent, index, desc, out);
}
static int doc_remove_subtree(xui_document_transaction t, uint64_t id)
{
    doc_node* node = doc_index_get(t->draft->index, id);
    uint64_t i, n;
    int result = XUI_OK;
    if (!node) return XUI_ERROR_NOT_FOUND;
    doc_node_retain(node); n = doc_seq_size(node->children);
    for (i = 0; i < n && result == XUI_OK; i++) result = doc_remove_subtree(t, doc_seq_get_id(node->children, i));
    if (result == XUI_OK) result = doc_state_remove(t->draft, id);
    doc_node_release(node); return result;
}
int doc_txn_delete(xui_document_transaction t, uint64_t id)
{
    doc_node *node, *parent;
    uint64_t index, parent_id;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    xui_doc_operation_t op = {0};
    if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = doc_txn_delete(&shadow, id);
        return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    node = doc_index_get(t->draft->index, id);
    if (!node || id == DOC_ROOT) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    parent_id = node->parent; parent = doc_index_get(t->draft->index, parent_id);
    index = doc_child_index(parent, id);
    result = doc_parent_replace(t, parent_id, index, index + 1, 0);
    if (result == XUI_OK) result = doc_remove_subtree(t, id);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_DELETE; op.iFlags = XUI_DOC_CHANGE_STRUCTURE;
    op.iNodeId = id; op.iParentId = parent_id; op.iOffset = index; op.iOldLength = 1;
    return doc_txn_op(t, &op);
}
XUI_API int xuiDocumentTxnDeleteNode(xui_document_transaction t, uint64_t id) { return doc_txn_delete(t, id); }
XUI_API int xuiDocumentTxnMoveNode(xui_document_transaction t, uint64_t id, uint64_t parent_id, uint64_t index)
{
    doc_node *node, *parent, *old_parent, *ancestor, *copy;
    uint64_t old_index, old_parent_id;
    unsigned depth = 0;
    xui_doc_operation_t op = {0};
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnMoveNode(&shadow, id, parent_id, index);
        return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    node = doc_index_get(t->draft->index, id); parent = doc_index_get(t->draft->index, parent_id);
    if (!node || !parent || id == DOC_ROOT) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_schema_child(parent->kind, node->kind)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    for (ancestor = parent; ancestor; ancestor = doc_index_get(t->draft->index, ancestor->parent)) {
        if (ancestor->id == id) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        if (++depth >= DOC_MAX_DEPTH) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    }
    if (index == DOC_NONE) index = doc_seq_size(parent->children);
    if (index > doc_seq_size(parent->children)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    old_parent_id = node->parent; old_parent = doc_index_get(t->draft->index, old_parent_id);
    result = doc_schema_depth(t->draft, id, depth + 1);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    old_index = doc_child_index(old_parent, id);
    if (parent_id == old_parent_id && (index == old_index || index == old_index + 1)) return XUI_OK;
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    copy->parent = parent_id;
    result = doc_parent_replace(t, old_parent_id, old_index, old_index + 1, 0);
    if (parent_id == old_parent_id && index > old_index) index--;
    if (result == XUI_OK) result = doc_parent_replace(t, parent_id, index, index, id);
    if (result == XUI_OK) result = doc_state_set(t->draft, copy);
    doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_MOVE; op.iFlags = XUI_DOC_CHANGE_STRUCTURE; op.iNodeId = id;
    op.iParentId = old_parent_id; op.iOtherNodeId = parent_id; op.iOffset = old_index; op.iNewLength = index;
    return doc_txn_op(t, &op);
}
int doc_txn_text(xui_document_transaction t, uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    doc_node *node, *copy;
    doc_sequence *insert = NULL, *result_text = NULL;
    xui_doc_operation_t op = {0};
    uint64_t size;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, id);
    if (!node || !doc_text_kind(node->kind)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    size = doc_seq_size(node->text);
    if (start > end || end > size) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_utf8(text, bytes) || !doc_seq_boundary(node->text, start) || !doc_seq_boundary(node->text, end)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (bytes > t->draft->allocator->max_bytes || t->draft->text_bytes - (end - start) > t->draft->allocator->max_bytes - bytes) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (end - start == bytes && doc_seq_equal_bytes(node->text, start, text, bytes)) return XUI_OK;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_markdown_text(t, id, start, end, text, bytes, NULL);
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    insert = doc_seq_text(t->draft->allocator, text, bytes);
    if (bytes && !insert) { doc_node_release(copy); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    result = doc_seq_replace(t->draft->allocator, node->text, start, end, insert, &result_text);
    doc_seq_release(insert);
    if (result == XUI_OK) {
        doc_seq_release(copy->text); copy->text = result_text;
        result = doc_state_set(t->draft, copy);
    }
    doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_TEXT; op.iFlags = XUI_DOC_CHANGE_TEXT;
    op.iNodeId = id; op.iOffset = start; op.iOldLength = end - start; op.iNewLength = bytes;
    return doc_txn_op(t, &op);
}
XUI_API int xuiDocumentTxnReplaceText(xui_document_transaction t, uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    return doc_txn_text(t, id, start, end, text, bytes);
}
XUI_API int xuiDocumentTxnSetAttributes(xui_document_transaction t, uint64_t id, const xui_doc_attributes_t* attrs)
{
    doc_node *node, *copy;
    xui_doc_attributes_t normalized;
    xui_doc_operation_t op = {0};
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, id);
    if (!node || !attrs) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_schema_attrs(node->kind, attrs)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (doc_attributes_equal(node->attrs, attrs)) return XUI_OK;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        /* The default language belongs to the document, independently of
         * Markdown source syntax. Local styles must still be representable. */
        normalized = *attrs; normalized.sLanguage = node->attrs->sLanguage;
        if (id != DOC_ROOT || !doc_attributes_equal(node->attrs, &normalized))
            return doc_markdown_attributes(t, id, attrs);
    }
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    normalized = *attrs;
    if (!normalized.iRowSpan) normalized.iRowSpan = 1;
    if (!normalized.iColumnSpan) normalized.iColumnSpan = 1;
    if (!normalized.iHeadingLevel && copy->kind == XUI_DOC_HEADING) normalized.iHeadingLevel = 1;
    result = doc_node_set_attrs(t->draft->allocator, copy, &normalized);
    if (result != XUI_OK) { doc_node_release(copy); return doc_txn_fail(t, result); }
    if (copy->kind == XUI_DOC_IMAGE && !(attrs->iMarks & XUI_DOC_LINK) &&
        (copy->link_target || copy->link_title)) {
        doc_blob_release(copy->link_target); copy->link_target = NULL;
        doc_blob_release(copy->link_title); copy->link_title = NULL;
        op.iFlags |= XUI_DOC_CHANGE_RESOURCE;
    }
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags |= XUI_DOC_CHANGE_STYLE; op.iNodeId = id;
    return doc_txn_op(t, &op);
}
XUI_API int xuiDocumentTxnSetResource(xui_document_transaction t, uint64_t id, const char* resource, const char* info, const char* title)
{
    doc_node *node, *copy;
    doc_blob *r = NULL, *i = NULL, *l = NULL;
    xui_doc_operation_t op = {0};
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, id);
    if (!node) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    if ((resource && !doc_utf8(resource, strlen(resource))) || (info && !doc_utf8(info, strlen(info))) ||
        (title && !doc_utf8(title, strlen(title)))) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (node->kind == XUI_DOC_EXTENSION &&
        (node->extension_payload || node->extension_required) && (!info || !*info))
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (!strcmp(doc_string(node->resource), resource ? resource : "") && !strcmp(doc_string(node->info), info ? info : "") &&
        !strcmp(doc_string(node->title), title ? title : "")) return XUI_OK;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetResource(&shadow, id, resource, info, title);
        return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (resource && !(r = doc_blob_new(t->draft->allocator, resource, strlen(resource)))) goto failed;
    if (info && !(i = doc_blob_new(t->draft->allocator, info, strlen(info)))) goto failed;
    if (title && !(l = doc_blob_new(t->draft->allocator, title, strlen(title)))) goto failed;
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) goto failed;
    doc_blob_release(copy->resource); copy->resource = r;
    doc_blob_release(copy->info); copy->info = i;
    doc_blob_release(copy->title); copy->title = l;
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags = XUI_DOC_CHANGE_RESOURCE; op.iNodeId = id;
    return doc_txn_op(t, &op);
failed:
    doc_blob_release(r); doc_blob_release(i); doc_blob_release(l);
    return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
}
XUI_API int xuiDocumentTxnSetExtensionPayload(xui_document_transaction t,
    uint64_t id, const void* payload, uint64_t bytes, uint32_t version, int required)
{
    doc_node *node, *copy;
    doc_blob* next = NULL;
    xui_doc_operation_t op = {0};
    uint64_t previous, remaining;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, id);
    if (!node) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    if (node->kind != XUI_DOC_EXTENSION || (bytes && !payload) ||
        (required != 0 && required != 1))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if ((bytes || required) && (!node->info || !node->info->size))
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (bytes > DOC_EXTENSION_MAX_PAYLOAD_BYTES) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (t->draft->profile != XUI_DOCUMENT_RICH) return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    previous = node->extension_payload ? node->extension_payload->size : 0;
    remaining = t->draft->payload_bytes - previous;
    if (bytes > t->draft->allocator->max_bytes - remaining) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (!version) version = 1;
    if (node->extension_version == version && node->extension_required == required &&
        previous == bytes && (!bytes || !memcmp(node->extension_payload->data, payload, (size_t)bytes)))
        return XUI_OK;
    if (bytes && !(next = doc_blob_new(t->draft->allocator, payload, bytes)))
        return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) { doc_blob_release(next); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    doc_blob_release(copy->extension_payload); copy->extension_payload = next;
    copy->extension_version = version; copy->extension_required = required;
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags = XUI_DOC_CHANGE_RESOURCE; op.iNodeId = id;
    return doc_txn_op(t, &op);
}

xui_document_change_set doc_change_create(doc_allocator* allocator, uint64_t identity, uint64_t revision, doc_state* before, doc_state* after,
    const xui_doc_operation_t* ops, uint64_t count, uint32_t flags, uint32_t domain, uint64_t origin, int undo)
{
    xui_document_change_set c = doc_alloc(allocator, sizeof(*c));
    if (!c) return NULL;
    atomic_init(&c->refs, 1); c->before = before; c->after = after;
    doc_state_retain(before); doc_state_retain(after);
    c->identity = identity; c->before_revision = revision; c->after_revision = revision + 1;
    c->origin = origin; c->flags = flags; c->domain = domain; c->undo = undo; c->count = count;
    if (count) {
        c->ops = doc_alloc(allocator, (size_t)count * sizeof(*ops));
        if (!c->ops) { xuiDocumentChangeSetRelease(c); return NULL; }
        memcpy(c->ops, ops, (size_t)count * sizeof(*ops));
    }
    return c;
}
XUI_API void xuiDocumentChangeSetRetain(xui_document_change_set c) { if (c) atomic_fetch_add(&c->refs, 1); }
XUI_API void xuiDocumentChangeSetRelease(xui_document_change_set c)
{
    doc_state *before, *after;
    if (!c || atomic_fetch_sub(&c->refs, 1) != 1) return;
    before = c->before; after = c->after; doc_free(c->ops); doc_free(c);
    doc_state_release(before); doc_state_release(after);
}
XUI_API int xuiDocumentChangeSetGetInfo(xui_document_change_set c, xui_doc_change_info_t* out)
{
    if (!c || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    out->iFlags = c->flags; out->iBeforeRevision = c->before_revision; out->iAfterRevision = c->after_revision;
    out->iOrigin = c->origin; out->iOperationCount = c->count; out->pOperations = c->ops; out->bUndo = c->undo;
    out->iDomain = c->domain;
    return XUI_OK;
}
doc_history* doc_history_prepare(xui_document_transaction t, doc_history* old)
{
    doc_history* h = doc_alloc(t->draft->allocator, sizeof(*h));
    uint64_t previous = t->group && old && old->group == t->group && old->origin == t->origin &&
        old->domain == t->domain && old->after == t->base ? old->count : 0;
    if (!h) return NULL;
    atomic_init(&h->refs, 1);
    if (t->count > SIZE_MAX / sizeof(*h->ops) || previous > SIZE_MAX / sizeof(*h->ops) - t->count) {
        doc_free(h); doc_txn_fail(t, XUI_DOC_ERROR_LIMIT); return NULL;
    }
    h->before = previous ? old->before : t->base; h->after = t->draft;
    doc_state_retain(h->before); doc_state_retain(h->after);
    h->count = previous + t->count; h->flags = t->flags | (previous ? old->flags : 0);
    h->origin = t->origin; h->group = t->group; h->domain = t->domain;
    h->ops = doc_alloc(t->draft->allocator, (size_t)h->count * sizeof(*h->ops));
    if (!h->ops) { doc_history_free(h); return NULL; }
    if (previous) memcpy(h->ops, old->ops, (size_t)previous * sizeof(*h->ops));
    memcpy(h->ops + previous, t->ops, (size_t)t->count * sizeof(*h->ops));
    return h;
}
XUI_API int xuiDocumentTxnCommit(xui_document_transaction t, xui_document_change_set* out)
{
    xui_document d;
    xui_document_change_set change;
    doc_history* h = NULL;
    doc_publish_plan* plan;
    int result = doc_txn_check(t, 0);
    if (out) *out = NULL;
    if (result != XUI_OK) return result;
    d = t->document;
    if (d->revision != t->base_revision) return doc_txn_fail(t, XUI_DOC_ERROR_STALE);
    if (!t->count) { xuiDocumentTxnAbort(t); return XUI_OK; }
    result = doc_schema_validate_transaction(t);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (d->revision == UINT64_MAX || d->next_state == UINT64_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    plan = doc_prepare_publish_plan(t->preparation, d);
    change = plan ? doc_publish_plan_change(plan) : doc_change_create(d->allocator, d->identity, d->revision,
        t->base, t->draft, t->ops, t->count, t->flags, t->domain, t->origin, 0);
    if (!change) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    if (!d->disable_history) {
        h = plan ? doc_publish_plan_take_history(plan) : doc_history_prepare(t, d->undo);
        if (!h) { xuiDocumentChangeSetRelease(change); return doc_txn_fail(t, t->error ? t->error : XUI_ERROR_OUT_OF_MEMORY); }
    }
    /* Publish point: every allocation required for this commit is complete. */
    t->draft->content_id = ++d->next_state;
    if (plan) {
        doc_publish_plan_apply(plan);
        d->standby_history = !d->disable_history && !doc_publish_plan_undo_count(plan);
    }
    else doc_memory_current(d->state, t->draft);
    doc_state_release(d->state); d->state = t->draft; doc_state_retain(d->state);
    d->revision++; t->closed = 1; d->writer = NULL;
    doc_prepare_invalidate(d, t->preparation, XUI_DOC_ERROR_STALE);
    doc_history_clear(&d->redo); d->redo_count = 0;
    if (h) {
        if (!plan) doc_memory_history(h, 1);
        if (!plan && d->standby_history) { doc_memory_standby(t->base, 0); d->standby_history = 0; }
        if (d->undo && h->before == d->undo->before && h->group &&
            h->group == d->undo->group && h->origin == d->undo->origin) {
            doc_history* old = d->undo; d->undo = old->next; doc_history_free(old); d->undo_count--;
        }
        h->next = d->undo; d->undo = h; d->undo_count++;
    }
    if (plan) {
        doc_history **tail = &d->undo, *discard, *live;
        uint64_t keep = doc_publish_plan_undo_count(plan), i;
        for (i = 0; i < keep; i++) { assert(*tail); tail = &(*tail)->next; }
        discard = *tail; *tail = NULL; doc_history_clear(&discard);
        d->undo_count = (unsigned)keep;
        for (live = d->undo; live; live = live->next) live->accounted = 1;
    } else doc_history_trim(d);
    if (out) { *out = change; xuiDocumentChangeSetRetain(change); }
    doc_notify(d, change); xuiDocumentChangeSetRelease(change);
    return XUI_OK;
}
XUI_API void xuiDocumentTxnAbort(xui_document_transaction t)
{
    if (!t || t->closed) return;
    t->closed = 1;
    if (t->document->writer == t) t->document->writer = NULL;
}
XUI_API void xuiDocumentTxnRelease(xui_document_transaction t)
{
    xui_document d;
    if (!t) return;
    xuiDocumentTxnAbort(t); d = t->document;
    doc_state_release(t->base); doc_state_release(t->draft); doc_free(t->ops); doc_free(t);
    xuiDocumentRelease(d);
}
static int doc_history_apply(xui_document d, int undo, xui_document_change_set* out)
{
    doc_history *h, **from, **to;
    doc_state* state;
    xui_document_change_set c;
    if (out) *out = NULL;
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    from = undo ? &d->undo : &d->redo; to = undo ? &d->redo : &d->undo;
    h = *from;
    if (!h) return XUI_ERROR_NOT_FOUND;
    if (d->revision == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    state = undo ? h->before : h->after;
    c = doc_change_create(d->allocator, d->identity, d->revision, d->state, state, h->ops, h->count, h->flags, h->domain, h->origin, undo);
    if (!c) return XUI_ERROR_OUT_OF_MEMORY;
    doc_memory_current(d->state, state);
    doc_state_retain(state); doc_state_release(d->state); d->state = state; d->revision++;
    doc_prepare_invalidate(d, NULL, XUI_DOC_ERROR_STALE);
    *from = h->next; h->next = *to; *to = h;
    if (undo) { d->undo_count--; d->redo_count++; } else { d->redo_count--; d->undo_count++; }
    doc_history_trim(d);
    if (out) { *out = c; xuiDocumentChangeSetRetain(c); }
    doc_notify(d, c); xuiDocumentChangeSetRelease(c); return XUI_OK;
}
XUI_API int xuiDocumentUndo(xui_document d, xui_document_change_set* out) { return doc_history_apply(d, 1, out); }
XUI_API int xuiDocumentRedo(xui_document d, xui_document_change_set* out) { return doc_history_apply(d, 0, out); }
XUI_API int xuiDocumentCanUndo(xui_document d) { return d && !d->writer && !d->notifying && d->undo != NULL; }
XUI_API int xuiDocumentCanRedo(xui_document d) { return d && !d->writer && !d->notifying && d->redo != NULL; }
XUI_API void xuiDocumentFreeBuffer(void* p) { free(p); }

int doc_split_text(xui_document_transaction t, uint64_t id, uint64_t at, uint64_t* right_id)
{
    doc_node *node = doc_index_get(t->draft->index, id), *left = NULL, *right = NULL;
    doc_sequence *left_text = NULL, *right_text = NULL;
    xui_doc_node_desc_t desc = {0};
    xui_doc_operation_t op = {0};
    uint64_t index, parent;
    int result;
    *right_id = 0;
    if (!node || node->kind != XUI_DOC_TEXT || !doc_seq_boundary(node->text, at)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    doc_node_retain(node); parent = node->parent;
    index = doc_child_index(doc_index_get(t->draft->index, parent), id);
    result = doc_seq_replace(t->draft->allocator, node->text, at, doc_seq_size(node->text), NULL, &left_text);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator, node->text, 0, at, NULL, &right_text);
    if (result != XUI_OK) goto done;
    desc.iSize = sizeof(desc); desc.iKind = node->kind; desc.tAttributes = *node->attrs;
    desc.sResource = doc_string(node->resource); desc.sInfo = doc_string(node->info); desc.sTitle = doc_string(node->title);
    result = doc_txn_insert(t, parent, index + 1, &desc, right_id);
    if (result != XUI_OK) goto done;
    left = doc_node_clone(t->draft->allocator, node);
    right = doc_node_clone(t->draft->allocator, doc_index_get(t->draft->index, *right_id));
    if (!left || !right) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    doc_seq_release(left->text); left->text = left_text; left_text = NULL;
    doc_seq_release(right->text); right->text = right_text; right_text = NULL;
    result = doc_state_set(t->draft, left);
    if (result == XUI_OK) result = doc_state_set(t->draft, right);
    if (result == XUI_OK) {
        op.iKind = XUI_DOC_OP_SPLIT; op.iFlags = XUI_DOC_CHANGE_STRUCTURE;
        op.iNodeId = id; op.iOtherNodeId = *right_id; op.iOffset = at;
        result = doc_txn_op(t, &op);
    }
done:
    doc_node_release(node); doc_node_release(left); doc_node_release(right);
    doc_seq_release(left_text); doc_seq_release(right_text);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnSetMarks(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    uint64_t id, start, end, tail;
    doc_node* node;
    xui_doc_attributes_t attrs;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if ((set | clear) & XUI_DOC_LINK) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (!range || range->tAnchor.iSize != sizeof(range->tAnchor) || range->tCaret.iSize != sizeof(range->tCaret) ||
        range->tAnchor.iDocumentId != t->document->identity || range->tCaret.iDocumentId != t->document->identity ||
        range->tAnchor.iRevision != t->base_revision || range->tCaret.iRevision != t->base_revision) return doc_txn_fail(t, XUI_DOC_ERROR_STALE);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE || range->tCaret.iKind == XUI_DOC_POSITION_SOURCE) return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    { int order;
      result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
      if (result != XUI_OK) return doc_txn_fail(t, result);
      if (!order) return XUI_OK; }
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_markdown_marks(t, range, set, clear);
    if (range->tAnchor.iNodeId != range->tCaret.iNodeId || range->tAnchor.iKind != XUI_DOC_POSITION_TEXT) return doc_marks_range(t, range, set, clear);
    id = range->tAnchor.iNodeId; start = range->tAnchor.iOffset; end = range->tCaret.iOffset;
    if (start > end) { uint64_t temp = start; start = end; end = temp; }
    node = doc_index_get(t->draft->index, id);
    if (!node || node->kind != XUI_DOC_TEXT || end > doc_seq_size(node->text)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_seq_boundary(node->text, start) || !doc_seq_boundary(node->text, end)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    if (start == end) return XUI_OK;
    attrs = *node->attrs; attrs.iMarks = (attrs.iMarks | set) & ~clear;
    if (!doc_schema_attrs(node->kind, &attrs)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (attrs.iMarks == node->attrs->iMarks) return XUI_OK;
    if (end < doc_seq_size(node->text)) {
        result = doc_split_text(t, id, end, &tail);
        if (result != XUI_OK) return result;
    }
    if (start) {
        result = doc_split_text(t, id, start, &tail);
        if (result != XUI_OK) return result;
        id = tail;
    }
    return xuiDocumentTxnSetAttributes(t, id, &attrs);
}

#endif
