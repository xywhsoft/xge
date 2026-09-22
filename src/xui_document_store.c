#include "xui_document_internal.h"
#include <math.h>

typedef union doc_allocation {
    struct { doc_allocator* allocator; size_t bytes; } value;
    max_align_t alignment;
} doc_allocation;

void* doc_alloc(doc_allocator* a, size_t size)
{
    doc_allocation* p;
    uint64_t live, peak;
    if (!a || size > SIZE_MAX - sizeof(*p)) return NULL;
    p = a->alloc(a->user, sizeof(*p) + size);
    if (!p) return NULL;
    p->value.allocator = a;
    p->value.bytes = sizeof(*p) + size;
    live = atomic_fetch_add(&a->live, p->value.bytes) + p->value.bytes;
    peak = atomic_load(&a->peak);
    while (live > peak && !atomic_compare_exchange_weak(&a->peak, &peak, live)) { }
    atomic_fetch_add(&a->allocations, 1);
    memset(p + 1, 0, size);
    return p + 1;
}

void doc_free(void* pointer)
{
    doc_allocation* p;
    doc_allocator* a;
    if (!pointer) return;
    p = (doc_allocation*)pointer - 1;
    a = p->value.allocator;
    atomic_fetch_sub(&a->live, p->value.bytes);
    a->free(a->user, p);
}
void* doc_realloc(doc_allocator* a, void* pointer, size_t bytes)
{
    void* next;
    size_t previous = pointer ? ((doc_allocation*)pointer - 1)->value.bytes - sizeof(doc_allocation) : 0;
    if (!bytes) { doc_free(pointer); return NULL; }
    next = doc_alloc(a, bytes);
    if (!next) return NULL;
    if (previous) memcpy(next, pointer, previous < bytes ? previous : bytes);
    doc_free(pointer); return next;
}

void doc_allocator_retain(doc_allocator* a) { if (a) atomic_fetch_add(&a->refs, 1); }
void doc_allocator_release(doc_allocator* a)
{
    if (a && atomic_fetch_sub(&a->refs, 1) == 1) free(a);
}

doc_blob* doc_blob_new(doc_allocator* a, const char* text, uint64_t bytes)
{
    doc_blob* p;
    if ((!text && bytes) || bytes > SIZE_MAX - sizeof(*p) - 1) return NULL;
    p = doc_alloc(a, sizeof(*p) + (size_t)bytes + 1);
    if (!p) return NULL;
    atomic_init(&p->refs, 1);
    p->size = bytes;
    if (bytes) memcpy(p->data, text, (size_t)bytes);
    return p;
}
void doc_blob_retain(doc_blob* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_blob_release(doc_blob* p)
{
    if (p && atomic_fetch_sub(&p->refs, 1) == 1) doc_free(p);
}
const char* doc_string(doc_blob* p) { return p ? p->data : ""; }

int doc_utf8(const char* str, uint64_t bytes)
{
    uint64_t i = 0;
    if (!str && bytes) return 0;
    while (i < bytes) {
        uint32_t c = (unsigned char)str[i++], value, min;
        unsigned n;
        if (c < 0x80) continue;
        if (c >= 0xc2 && c <= 0xdf) { n = 1; value = c & 0x1f; min = 0x80; }
        else if (c >= 0xe0 && c <= 0xef) { n = 2; value = c & 0xf; min = 0x800; }
        else if (c >= 0xf0 && c <= 0xf4) { n = 3; value = c & 7; min = 0x10000; }
        else return 0;
        if (bytes - i < n) return 0;
        while (n--) {
            c = (unsigned char)str[i++];
            if ((c & 0xc0) != 0x80) return 0;
            value = (value << 6) | (c & 0x3f);
        }
        if (value < min || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) return 0;
    }
    return 1;
}

uint64_t doc_seq_size(doc_sequence* p) { return p ? p->total : 0; }
void doc_seq_retain(doc_sequence* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_seq_release(doc_sequence* p)
{
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    doc_seq_release(p->left);
    doc_seq_release(p->right);
    doc_blob_release(p->blob);
    doc_free(p);
}
static uint64_t doc_mix(uint64_t x)
{
    x += UINT64_C(0x9e3779b97f4a7c15);
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31);
}
static void doc_seq_measure(doc_sequence* p)
{
    p->total = doc_seq_size(p->left) + p->length + doc_seq_size(p->right);
}
static doc_sequence* doc_seq_new(doc_allocator* a)
{
    doc_sequence* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    atomic_init(&p->refs, 1);
    p->priority = doc_mix(atomic_fetch_add(&a->sequence, 1));
    return p;
}
static doc_sequence* doc_seq_clone(doc_allocator* a, const doc_sequence* src)
{
    doc_sequence* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    /* Refcounts may change on snapshot readers. Copy only immutable fields. */
    memcpy(&p->priority, &src->priority, sizeof(*p) - offsetof(doc_sequence, priority));
    atomic_init(&p->refs, 1);
    doc_seq_retain(p->left); doc_seq_retain(p->right); doc_blob_retain(p->blob);
    return p;
}
doc_sequence* doc_seq_text(doc_allocator* a, const char* text, uint64_t bytes)
{
    doc_sequence* p;
    if (!bytes) return NULL;
    p = doc_seq_new(a);
    if (!p) return NULL;
    p->blob = doc_blob_new(a, text, bytes);
    if (!p->blob) { doc_seq_release(p); return NULL; }
    p->length = p->total = bytes;
    return p;
}
doc_sequence* doc_seq_id(doc_allocator* a, uint64_t id)
{
    doc_sequence* p = doc_seq_new(a);
    if (p) { p->id = id; p->length = p->total = 1; }
    return p;
}
static int doc_seq_join(doc_allocator* a, doc_sequence* l, doc_sequence* r, doc_sequence** out)
{
    doc_sequence *p, *child = NULL;
    int result;
    *out = NULL;
    if (!l || !r) { *out = l ? l : r; doc_seq_retain(*out); return XUI_OK; }
    p = doc_seq_clone(a, l->priority <= r->priority ? l : r);
    if (!p) return XUI_ERROR_OUT_OF_MEMORY;
    if (l->priority <= r->priority) {
        result = doc_seq_join(a, l->right, r, &child);
        if (result == XUI_OK) { doc_seq_release(p->right); p->right = child; }
    } else {
        result = doc_seq_join(a, l, r->left, &child);
        if (result == XUI_OK) { doc_seq_release(p->left); p->left = child; }
    }
    if (result != XUI_OK) { doc_seq_release(p); return result; }
    doc_seq_measure(p); *out = p; return XUI_OK;
}
static int doc_seq_split(doc_allocator* a, doc_sequence* p, uint64_t at,
    doc_sequence** left, doc_sequence** right)
{
    doc_sequence *copy = NULL, *l = NULL, *r = NULL;
    uint64_t size;
    int result;
    *left = *right = NULL;
    if (!p) return at ? XUI_ERROR_INVALID_ARGUMENT : XUI_OK;
    if (at > p->total) return XUI_ERROR_INVALID_ARGUMENT;
    if (!at) { doc_seq_retain(p); *right = p; return XUI_OK; }
    if (at == p->total) { doc_seq_retain(p); *left = p; return XUI_OK; }
    size = doc_seq_size(p->left);
    copy = doc_seq_clone(a, p);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    if (at < size) {
        result = doc_seq_split(a, p->left, at, &l, &r);
        if (result != XUI_OK) { doc_seq_release(copy); return result; }
        doc_seq_release(copy->left); copy->left = r; doc_seq_measure(copy);
        *left = l; *right = copy;
    } else if (at > size + p->length) {
        result = doc_seq_split(a, p->right, at - size - p->length, &l, &r);
        if (result != XUI_OK) { doc_seq_release(copy); return result; }
        doc_seq_release(copy->right); copy->right = l; doc_seq_measure(copy);
        *left = copy; *right = r;
    } else if (at == size) {
        doc_seq_retain(p->left); *left = p->left;
        doc_seq_release(copy->left); copy->left = NULL; doc_seq_measure(copy);
        *right = copy;
    } else if (at == size + p->length) {
        doc_seq_retain(p->right); *right = p->right;
        doc_seq_release(copy->right); copy->right = NULL; doc_seq_measure(copy);
        *left = copy;
    } else {
        r = doc_seq_clone(a, p);
        if (!r) { doc_seq_release(copy); return XUI_ERROR_OUT_OF_MEMORY; }
        copy->length = at - size;
        doc_seq_release(copy->right); copy->right = NULL; doc_seq_measure(copy);
        r->offset += at - size; r->length -= at - size;
        doc_seq_release(r->left); r->left = NULL; doc_seq_measure(r);
        *left = copy; *right = r;
    }
    return XUI_OK;
}
int doc_seq_replace(doc_allocator* a, doc_sequence* base, uint64_t start,
    uint64_t end, doc_sequence* insert, doc_sequence** out)
{
    doc_sequence *l = NULL, *tail = NULL, *removed = NULL, *r = NULL, *joined = NULL;
    int result;
    *out = NULL;
    if (start > end || end > doc_seq_size(base) ||
        doc_seq_size(insert) > UINT64_MAX - (doc_seq_size(base) - (end - start))) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_seq_split(a, base, start, &l, &tail);
    if (result == XUI_OK) result = doc_seq_split(a, tail, end - start, &removed, &r);
    if (result == XUI_OK) result = doc_seq_join(a, l, insert, &joined);
    if (result == XUI_OK) result = doc_seq_join(a, joined, r, out);
    doc_seq_release(l); doc_seq_release(tail); doc_seq_release(removed);
    doc_seq_release(r); doc_seq_release(joined);
    return result;
}
int doc_seq_read(doc_sequence* p, uint64_t offset, void* buffer, uint64_t bytes)
{
    uint64_t left, n;
    char* out = buffer;
    if (offset > doc_seq_size(p) || bytes > doc_seq_size(p) - offset || (!out && bytes)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!bytes) return XUI_OK;
    left = doc_seq_size(p->left);
    if (offset < left) {
        n = bytes < left - offset ? bytes : left - offset;
        if (doc_seq_read(p->left, offset, out, n) != XUI_OK) return XUI_ERROR_INVALID_ARGUMENT;
        offset += n; out += n; bytes -= n;
    }
    if (bytes && offset < left + p->length) {
        uint64_t in_piece = offset - left;
        n = bytes < p->length - in_piece ? bytes : p->length - in_piece;
        if (!p->blob) return XUI_ERROR_INVALID_ARGUMENT;
        memcpy(out, p->blob->data + p->offset + in_piece, (size_t)n);
        offset += n; out += n; bytes -= n;
    }
    return bytes ? doc_seq_read(p->right, offset - left - p->length, out, bytes) : XUI_OK;
}
uint64_t doc_seq_get_id(doc_sequence* p, uint64_t index)
{
    while (p) {
        uint64_t left = doc_seq_size(p->left);
        if (index < left) p = p->left;
        else if (index == left) return p->id;
        else { index -= left + 1; p = p->right; }
    }
    return 0;
}
int doc_seq_boundary(doc_sequence* p, uint64_t offset)
{
    unsigned char c;
    if (offset > doc_seq_size(p)) return 0;
    if (!offset || offset == doc_seq_size(p)) return 1;
    return doc_seq_read(p, offset, &c, 1) == XUI_OK && (c & 0xc0) != 0x80;
}
char* doc_seq_string(doc_allocator* a, doc_sequence* p)
{
    uint64_t n = doc_seq_size(p);
    char* out;
    if (n >= SIZE_MAX) return NULL;
    out = doc_alloc(a, (size_t)n + 1);
    if (!out) return NULL;
    if (doc_seq_read(p, 0, out, n) != XUI_OK) { doc_free(out); return NULL; }
    return out;
}
int doc_seq_equal_bytes(doc_sequence* p, uint64_t start, const char* text, uint64_t bytes)
{
    char block[512];
    uint64_t at = 0;
    if (start > doc_seq_size(p) || bytes > doc_seq_size(p) - start || (!text && bytes)) return 0;
    while (at < bytes) {
        uint64_t n = bytes - at < sizeof(block) ? bytes - at : sizeof(block);
        if (doc_seq_read(p, start + at, block, n) != XUI_OK || memcmp(block, text + at, (size_t)n)) return 0;
        at += n;
    }
    return 1;
}

doc_node* doc_node_new(doc_allocator* a, uint64_t id, uint64_t parent, const xui_doc_node_desc_t* d)
{
    doc_node* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    atomic_init(&p->refs, 1);
    p->id = id; p->parent = parent; p->kind = d->iKind; p->attrs = d->tAttributes;
    p->source_start = p->source_end = DOC_NONE;
    if (!p->attrs.iRowSpan) p->attrs.iRowSpan = 1;
    if (!p->attrs.iColumnSpan) p->attrs.iColumnSpan = 1;
    if (!p->attrs.iHeadingLevel && p->kind == XUI_DOC_HEADING) p->attrs.iHeadingLevel = 1;
    p->text = doc_seq_text(a, d->sText, d->iTextBytes);
    if (d->iTextBytes && !p->text) goto failed;
    if (d->sResource && *d->sResource) { p->resource = doc_blob_new(a, d->sResource, strlen(d->sResource)); if (!p->resource) goto failed; }
    if (d->sInfo && *d->sInfo) { p->info = doc_blob_new(a, d->sInfo, strlen(d->sInfo)); if (!p->info) goto failed; }
    if (d->sTitle && *d->sTitle) { p->title = doc_blob_new(a, d->sTitle, strlen(d->sTitle)); if (!p->title) goto failed; }
    return p;
failed:
    doc_node_release(p); return NULL;
}
doc_node* doc_node_clone(doc_allocator* a, const doc_node* src)
{
    doc_node* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    memcpy(&p->id, &src->id, sizeof(*p) - offsetof(doc_node, id)); atomic_init(&p->refs, 1);
    doc_seq_retain(p->text); doc_seq_retain(p->children);
    doc_blob_retain(p->resource); doc_blob_retain(p->info); doc_blob_retain(p->title);
    return p;
}
void doc_node_retain(doc_node* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_node_release(doc_node* p)
{
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    doc_seq_release(p->text); doc_seq_release(p->children);
    doc_blob_release(p->resource); doc_blob_release(p->info); doc_blob_release(p->title);
    doc_free(p);
}

void doc_index_retain(doc_index* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_index_release(doc_index* p)
{
    unsigned i;
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    for (i = 0; i < DOC_INDEX_SIZE; i++) {
        if (p->level) doc_index_release(p->slots[i]);
        else doc_node_release(p->slots[i]);
    }
    doc_free(p);
}
doc_node* doc_index_get(doc_index* p, uint64_t id)
{
    if (p && p->level < 15 && (id >> ((p->level + 1) * DOC_INDEX_BITS))) return NULL;
    while (p) {
        unsigned slot = (unsigned)((id >> (p->level * DOC_INDEX_BITS)) & 15u);
        if (!p->level) return p->slots[slot];
        p = p->slots[slot];
    }
    return NULL;
}
static int doc_index_put(doc_allocator* a, doc_index* base, unsigned level,
    uint64_t id, doc_node* node, doc_index** out)
{
    doc_index *p = doc_alloc(a, sizeof(*p)), *child = NULL;
    unsigned i, slot = (unsigned)((id >> (level * DOC_INDEX_BITS)) & 15u);
    int result;
    *out = NULL;
    if (!p) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&p->refs, 1); p->level = level;
    if (base) for (i = 0; i < DOC_INDEX_SIZE; i++) {
        p->slots[i] = base->slots[i];
        if (level) doc_index_retain(p->slots[i]); else doc_node_retain(p->slots[i]);
    }
    if (!level) {
        doc_node_release(p->slots[slot]); p->slots[slot] = node; doc_node_retain(node);
    } else {
        result = doc_index_put(a, p->slots[slot], level - 1, id, node, &child);
        if (result != XUI_OK) { doc_index_release(p); return result; }
        doc_index_release(p->slots[slot]); p->slots[slot] = child;
    }
    *out = p; return XUI_OK;
}
int doc_index_set(doc_allocator* a, doc_index* base, uint64_t id, doc_node* node, doc_index** out)
{
    unsigned level = base ? base->level : 0;
    doc_index *root = base, *next;
    int result;
    doc_index_retain(root);
    while (level < 15 && (id >> ((level + 1) * DOC_INDEX_BITS))) {
        next = doc_alloc(a, sizeof(*next));
        if (!next) { doc_index_release(root); *out = NULL; return XUI_ERROR_OUT_OF_MEMORY; }
        atomic_init(&next->refs, 1); next->level = ++level; next->slots[0] = root; root = next;
    }
    result = doc_index_put(a, root, level, id, node, out);
    doc_index_release(root); return result;
}
doc_state* doc_state_new(doc_allocator* a, uint32_t profile)
{
    doc_state* p = doc_alloc(a, sizeof(*p));
    doc_node* root;
    xui_doc_node_desc_t desc = {0};
    if (!p) return NULL;
    atomic_init(&p->refs, 1); p->allocator = a; p->profile = profile;
    doc_allocator_retain(a);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_ROOT;
    root = doc_node_new(a, DOC_ROOT, 0, &desc);
    if (!root || doc_state_set(p, root) != XUI_OK) { doc_node_release(root); doc_state_release(p); return NULL; }
    doc_node_release(root); return p;
}
doc_state* doc_state_clone(doc_state* src)
{
    doc_state* p = doc_alloc(src->allocator, sizeof(*p));
    if (!p) return NULL;
    memcpy(&p->allocator, &src->allocator, sizeof(*p) - offsetof(doc_state, allocator)); atomic_init(&p->refs, 1);
    doc_allocator_retain(p->allocator); doc_index_retain(p->index); doc_seq_retain(p->source);
    return p;
}
void doc_state_retain(doc_state* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_state_release(doc_state* p)
{
    doc_allocator* a;
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    a = p->allocator;
    doc_index_release(p->index); doc_seq_release(p->source); doc_free(p); doc_allocator_release(a);
}
int doc_state_set(doc_state* s, doc_node* node)
{
    doc_index* next = NULL;
    doc_node* previous = doc_index_get(s->index, node->id);
    int existed = previous != NULL;
    uint64_t old_bytes = previous ? doc_seq_size(previous->text) : 0;
    int result = doc_index_set(s->allocator, s->index, node->id, node, &next);
    if (result != XUI_OK) return result;
    doc_index_release(s->index); s->index = next;
    s->text_bytes = s->text_bytes - old_bytes + doc_seq_size(node->text);
    if (!existed) s->node_count++;
    return XUI_OK;
}
int doc_state_remove(doc_state* s, uint64_t id)
{
    doc_index* next = NULL;
    int result;
    doc_node* previous = doc_index_get(s->index, id);
    uint64_t bytes;
    if (!previous) return XUI_ERROR_NOT_FOUND;
    bytes = doc_seq_size(previous->text);
    result = doc_index_set(s->allocator, s->index, id, NULL, &next);
    if (result != XUI_OK) return result;
    doc_index_release(s->index); s->index = next; s->node_count--; s->text_bytes -= bytes;
    return XUI_OK;
}
int doc_node_info(const doc_node* p, xui_doc_node_info_t* out)
{
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!p) return XUI_ERROR_NOT_FOUND;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iKind = p->kind; out->iId = p->id; out->iParentId = p->parent;
    out->iChildCount = doc_seq_size(p->children); out->iTextBytes = doc_seq_size(p->text);
    out->tAttributes = p->attrs; out->sResource = doc_string(p->resource);
    out->sInfo = doc_string(p->info); out->sTitle = doc_string(p->title);
    out->iSourceStart = p->source_start; out->iSourceEnd = p->source_end; out->bSourceExact = p->source_exact;
    return XUI_OK;
}
int doc_text_kind(uint32_t k)
{
    return k == XUI_DOC_TEXT || k == XUI_DOC_CODE_BLOCK || k == XUI_DOC_HTML ||
        k == XUI_DOC_MATH || k == XUI_DOC_DIAGRAM || k == XUI_DOC_IMAGE ||
        k == XUI_DOC_FRONT_MATTER || k == XUI_DOC_EXTENSION || k == XUI_DOC_FOOTNOTE_REF;
}
int doc_inline_kind(uint32_t k)
{
    return k == XUI_DOC_TEXT || k == XUI_DOC_IMAGE || k == XUI_DOC_SOFT_BREAK ||
        k == XUI_DOC_HARD_BREAK || k == XUI_DOC_HTML || k == XUI_DOC_MATH ||
        k == XUI_DOC_FOOTNOTE_REF || k == XUI_DOC_EXTENSION;
}
int doc_schema_child(uint32_t parent, uint32_t child)
{
    if (child < XUI_DOC_PARAGRAPH || child > XUI_DOC_EXTENSION) return 0;
    if (parent == XUI_DOC_PARAGRAPH || parent == XUI_DOC_HEADING) return doc_inline_kind(child);
    if (parent == XUI_DOC_LIST) return child == XUI_DOC_LIST_ITEM;
    if (parent == XUI_DOC_TABLE) return child == XUI_DOC_ROW;
    if (parent == XUI_DOC_ROW) return child == XUI_DOC_CELL;
    if (parent == XUI_DOC_ROOT || parent == XUI_DOC_QUOTE || parent == XUI_DOC_LIST_ITEM ||
        parent == XUI_DOC_CELL || parent == XUI_DOC_FOOTNOTE) {
        return child != XUI_DOC_TEXT && child != XUI_DOC_ROW && child != XUI_DOC_CELL &&
            child != XUI_DOC_LIST_ITEM && child != XUI_DOC_SOFT_BREAK && child != XUI_DOC_HARD_BREAK &&
            child != XUI_DOC_FOOTNOTE_REF;
    }
    return 0;
}
int doc_schema_attrs(uint32_t kind, const xui_doc_attributes_t* a)
{
    if (!a || a->iAlignment > 3 || a->iHeadingLevel > 6 ||
        a->iRowSpan > 1024 || a->iColumnSpan > 1024 ||
        a->iMarks & ~(XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_UNDERLINE | XUI_DOC_STRIKE |
            XUI_DOC_CODE | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT | XUI_DOC_LINK | XUI_DOC_HIGHLIGHT) ||
        !isfinite(a->fFontSize) || !isfinite(a->fWidth) || !isfinite(a->fHeight) || !isfinite(a->fParagraphSpacing) ||
        a->fFontSize < 0 || a->fWidth < 0 || a->fHeight < 0 || a->fParagraphSpacing < 0 || !memchr(a->sFontFamily, 0, sizeof(a->sFontFamily))) return 0;
    if (!doc_utf8(a->sFontFamily, strlen(a->sFontFamily))) return 0;
    if ((a->iMarks & XUI_DOC_SUBSCRIPT) && (a->iMarks & XUI_DOC_SUPERSCRIPT)) return 0;
    return kind >= XUI_DOC_ROOT && kind <= XUI_DOC_EXTENSION;
}
uint64_t doc_child_index(const doc_node* parent, uint64_t id)
{
    uint64_t i, n = doc_seq_size(parent->children);
    for (i = 0; i < n; i++) if (doc_seq_get_id(parent->children, i) == id) return i;
    return DOC_NONE;
}
