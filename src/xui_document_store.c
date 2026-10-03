#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <math.h>
#include <assert.h>

void* doc_alloc(doc_allocator* a, size_t size)
{
    doc_allocation* p;
    uint64_t live, peak;
    if (!a || size > SIZE_MAX - sizeof(*p)) return NULL;
    p = a->alloc(a->user, sizeof(*p) + size);
    if (!p) return NULL;
    memset(p, 0, sizeof(*p));
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
    assert(!p->value.owners[0] && !p->value.owners[1] && !p->value.owners[2]);
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
    if (a && atomic_fetch_sub(&a->refs, 1) == 1) {
        unsigned i;
        for (i = 0; i < DOC_ATTR_BUCKETS; i++) assert(!a->attributes[i]);
        free(a);
    }
}

static void doc_attribute_lock(doc_allocator* a)
{
    while (atomic_exchange_explicit(&a->attribute_lock, 1, memory_order_acquire)) { }
}
static void doc_attribute_unlock(doc_allocator* a)
{
    atomic_store_explicit(&a->attribute_lock, 0, memory_order_release);
}
static uint64_t doc_attribute_hash(const xui_doc_attributes_t* value)
{
    const unsigned char* p = (const unsigned char*)value->sFontFamily;
    uint64_t h = UINT64_C(14695981039346656037);
    uint32_t floats[4];
    const uint64_t fields[] = {
        value->iMarks, value->iFlags, value->iHeadingLevel,
        value->iAlignment, value->iRowSpan, value->iColumnSpan,
        value->iListStart, value->iTextColor, value->iBackgroundColor
    };
    unsigned i, j;
    float values[] = { value->fFontSize, value->fWidth, value->fHeight, value->fParagraphSpacing };
    for (i = 0; i < sizeof(fields) / sizeof(fields[0]); i++) {
        uint64_t field = fields[i];
        for (j = 0; j < 8; j++) { h = (h ^ (unsigned char)field) * UINT64_C(1099511628211); field >>= 8; }
    }
    /* +0 and -0 compare equal in the schema and must hash to one bucket. */
    for (i = 0; i < 4; i++) {
        floats[i] = 0;
        if (values[i] != 0) memcpy(&floats[i], &values[i], sizeof(floats[i]));
        for (j = 0; j < 4; j++) {
            h = (h ^ (unsigned char)(floats[i] >> (j * 8))) * UINT64_C(1099511628211);
        }
    }
    for (i = 0; i < sizeof(value->sFontFamily) && p[i]; i++)
        h = (h ^ p[i]) * UINT64_C(1099511628211);
    h = (h ^ 0u) * UINT64_C(1099511628211);
    p = (const unsigned char*)doc_language(value->sLanguage);
    for (i = 0; i < 255 && p[i]; i++)
        h = (h ^ __xgeLanguageLower(p[i])) * UINT64_C(1099511628211);
    return h;
}
static int doc_attribute_comparable(const xui_doc_attributes_t* value)
{
    return memchr(value->sFontFamily, 0, sizeof(value->sFontFamily)) != NULL &&
        doc_language_valid(value->sLanguage);
}
static unsigned doc_attribute_height(const doc_attribute* p) { return p ? p->height : 0; }
static void doc_attribute_measure(doc_attribute* p)
{
    unsigned left = doc_attribute_height(p->left), right = doc_attribute_height(p->right);
    p->height = (left > right ? left : right) + 1;
}
static doc_attribute* doc_attribute_rotate_left(doc_attribute* p)
{
    doc_attribute* top = p->right;
    p->right = top->left; top->left = p;
    doc_attribute_measure(p); doc_attribute_measure(top); return top;
}
static doc_attribute* doc_attribute_rotate_right(doc_attribute* p)
{
    doc_attribute* top = p->left;
    p->left = top->right; top->right = p;
    doc_attribute_measure(p); doc_attribute_measure(top); return top;
}
static doc_attribute* doc_attribute_balance(doc_attribute* p)
{
    int difference;
    doc_attribute_measure(p);
    difference = (int)doc_attribute_height(p->left) - (int)doc_attribute_height(p->right);
    if (difference > 1) {
        if (doc_attribute_height(p->left->left) < doc_attribute_height(p->left->right))
            p->left = doc_attribute_rotate_left(p->left);
        return doc_attribute_rotate_right(p);
    }
    if (difference < -1) {
        if (doc_attribute_height(p->right->right) < doc_attribute_height(p->right->left))
            p->right = doc_attribute_rotate_right(p->right);
        return doc_attribute_rotate_left(p);
    }
    return p;
}
static doc_attribute* doc_attribute_insert(doc_attribute* root, doc_attribute* entry)
{
    if (!root) return entry;
    if (entry->hash < root->hash) root->left = doc_attribute_insert(root->left, entry);
    else if (entry->hash > root->hash) root->right = doc_attribute_insert(root->right, entry);
    else { entry->next = root->next; root->next = entry; return root; }
    return doc_attribute_balance(root);
}
static doc_attribute* doc_attribute_pop_min(doc_attribute* root, doc_attribute** minimum)
{
    if (!root->left) { *minimum = root; return root->right; }
    root->left = doc_attribute_pop_min(root->left, minimum);
    return doc_attribute_balance(root);
}
static doc_attribute* doc_attribute_remove(doc_attribute* root, doc_attribute* entry)
{
    assert(root);
    if (entry->hash < root->hash) root->left = doc_attribute_remove(root->left, entry);
    else if (entry->hash > root->hash) root->right = doc_attribute_remove(root->right, entry);
    else if (root != entry) {
        doc_attribute** link = &root->next;
        while (*link && *link != entry) link = &(*link)->next;
        assert(*link == entry); *link = entry->next;
        return root;
    } else if (root->next) {
        doc_attribute* replacement = root->next;
        replacement->left = root->left; replacement->right = root->right;
        replacement->height = root->height;
        return replacement;
    } else if (!root->left) return root->right;
    else if (!root->right) return root->left;
    else {
        doc_attribute* replacement;
        doc_attribute* right = doc_attribute_pop_min(root->right, &replacement);
        replacement->left = root->left; replacement->right = right;
        return doc_attribute_balance(replacement);
    }
    return doc_attribute_balance(root);
}
static const xui_doc_attributes_t* doc_attribute_intern(doc_allocator* a,
    const xui_doc_attributes_t* value)
{
    uint64_t hash = doc_attribute_hash(value);
    unsigned bucket = (unsigned)(hash % DOC_ATTR_BUCKETS);
    doc_attribute *current, *candidate, *created;
    doc_attribute_lock(a);
    current = a->attributes[bucket];
    while (current && current->hash != hash)
        current = hash < current->hash ? current->left : current->right;
    for (candidate = current; candidate; candidate = candidate->next) {
        if (doc_attribute_comparable(value) &&
            doc_attribute_comparable(&candidate->value) &&
            doc_attributes_equal(&candidate->value, value)) {
            assert(atomic_load(&candidate->refs) < UINT_MAX);
            atomic_fetch_add(&candidate->refs, 1);
            doc_attribute_unlock(a);
            return &candidate->value;
        }
    }
    if (!doc_language_valid(value->sLanguage)) {
        doc_attribute_unlock(a); return NULL;
    }
    size_t language_bytes = strlen(doc_language(value->sLanguage));
    created = doc_alloc(a, sizeof(*created) + (language_bytes ? language_bytes + 1 : 0));
    if (created) {
        doc_memory_tag(created, DOC_MEMORY_ATTRIBUTE);
        atomic_init(&created->refs, 1);
        created->hash = hash;
        created->height = 1;
        created->value = *value;
        created->value.sLanguage = NULL;
        if (language_bytes) {
            size_t i;
            for (i = 0; i < language_bytes; i++)
                created->language[i] = (char)__xgeLanguageLower((unsigned char)value->sLanguage[i]);
            created->language[language_bytes] = 0;
            created->value.sLanguage = created->language;
        }
        a->attributes[bucket] = doc_attribute_insert(a->attributes[bucket], created);
    }
    doc_attribute_unlock(a);
    return created ? &created->value : NULL;
}
static void doc_attribute_retain(const xui_doc_attributes_t* value)
{
    if (value) {
        doc_attribute* entry = doc_attribute_owner(value);
        assert(atomic_load(&entry->refs) < UINT_MAX);
        atomic_fetch_add(&entry->refs, 1);
    }
}
static void doc_attribute_release(const xui_doc_attributes_t* value)
{
    doc_attribute* entry;
    doc_allocator* a;
    unsigned bucket;
    if (!value) return;
    entry = doc_attribute_owner(value);
    a = ((doc_allocation*)entry - 1)->value.allocator;
    doc_attribute_lock(a);
    if (atomic_fetch_sub(&entry->refs, 1) != 1) {
        doc_attribute_unlock(a); return;
    }
    bucket = (unsigned)(entry->hash % DOC_ATTR_BUCKETS);
    a->attributes[bucket] = doc_attribute_remove(a->attributes[bucket], entry);
    doc_attribute_unlock(a);
    doc_free(entry);
}
int doc_node_set_attrs(doc_allocator* a, doc_node* node,
    const xui_doc_attributes_t* value)
{
    const xui_doc_attributes_t* next;
    if (node->attrs && doc_attribute_comparable(value) &&
        doc_attribute_comparable(node->attrs) &&
        doc_attributes_equal(node->attrs, value)) return XUI_OK;
    next = doc_attribute_intern(a, value);
    if (!next) return XUI_ERROR_OUT_OF_MEMORY;
    doc_attribute_release(node->attrs);
    node->attrs = next;
    return XUI_OK;
}
uint64_t doc_node_id_next(doc_allocator* a)
{
    uint_fast64_t id = atomic_load(&a->next_node_id);
    while (id != UINT64_MAX) {
        if (atomic_compare_exchange_weak(&a->next_node_id, &id, id + 1)) return id;
    }
    return 0;
}

doc_blob* doc_blob_allocate(doc_allocator* a, uint64_t bytes)
{
    doc_blob* p;
    if (bytes > SIZE_MAX - sizeof(*p) - 1) return NULL;
    p = doc_alloc(a, sizeof(*p) + (size_t)bytes + 1);
    if (!p) return NULL;
    doc_memory_tag(p, DOC_MEMORY_BLOB);
    atomic_init(&p->refs, 1);
    p->size = bytes;
    return p;
}
doc_blob* doc_blob_new(doc_allocator* a, const char* text, uint64_t bytes)
{
    doc_blob* p;
    if (!text && bytes) return NULL;
    p = doc_blob_allocate(a, bytes);
    if (p && bytes) memcpy(p->data, text, (size_t)bytes);
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
uint64_t doc_source_open_bracket_count(const char* text, uint64_t bytes)
{
    uint64_t i, count = 0;
    for (i = 0; i < bytes; i++) count += text[i] == '[';
    return count;
}
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
    doc_memory_tag(p, DOC_MEMORY_SEQUENCE);
    atomic_init(&p->refs, 1);
    p->priority = doc_mix(atomic_fetch_add(&a->sequence, 1));
    return p;
}
static doc_sequence* doc_seq_clone(doc_allocator* a, const doc_sequence* src)
{
    doc_sequence* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    doc_memory_tag(p, DOC_MEMORY_SEQUENCE);
    /* Refcounts may change on snapshot readers. Copy only immutable fields. */
    memcpy(&p->priority, &src->priority, sizeof(*p) - offsetof(doc_sequence, priority));
    atomic_init(&p->refs, 1);
    doc_seq_retain(p->left); doc_seq_retain(p->right); doc_blob_retain(p->blob);
    return p;
}
static void doc_seq_syntax_add(doc_sequence* p, int64_t source, int64_t index)
{
    p->syntax_self_source += source; p->syntax_self_index += index;
    p->syntax_child_source += source; p->syntax_child_index += index;
}
/* A join or split replaces a child of a cloned node. Push its pending syntax
 * displacement into cloned children first, so unrelated joined pieces do not
 * inherit it. The original tree and every published snapshot stay immutable. */
static int doc_seq_syntax_push(doc_allocator* a, doc_sequence* p)
{
    doc_sequence *left = NULL, *right = NULL;
    if (!p->syntax_child_source && !p->syntax_child_index) return XUI_OK;
    if (p->left) {
        left = doc_seq_clone(a, p->left);
        if (!left) return XUI_ERROR_OUT_OF_MEMORY;
        doc_seq_syntax_add(left, p->syntax_child_source, p->syntax_child_index);
    }
    if (p->right) {
        right = doc_seq_clone(a, p->right);
        if (!right) { doc_seq_release(left); return XUI_ERROR_OUT_OF_MEMORY; }
        doc_seq_syntax_add(right, p->syntax_child_source, p->syntax_child_index);
    }
    doc_seq_release(p->left); doc_seq_release(p->right);
    p->left = left; p->right = right;
    p->syntax_child_source = p->syntax_child_index = 0;
    return XUI_OK;
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
doc_sequence* doc_seq_blob_item(doc_allocator* a, uint64_t kind, doc_blob* blob)
{
    doc_sequence* p = doc_seq_id(a, kind);
    if (p) { p->blob = blob; doc_blob_retain(blob); }
    return p;
}
doc_blob* doc_seq_get_blob_item(doc_sequence* p, uint64_t index)
{
    while (p) {
        uint64_t left = doc_seq_size(p->left);
        if (index < left) p = p->left;
        else if (index - left < p->length) return p->length == 1 ? p->blob : NULL;
        else { index -= left + p->length; p = p->right; }
    }
    return NULL;
}
static int doc_seq_join(doc_allocator* a, doc_sequence* l, doc_sequence* r, doc_sequence** out)
{
    doc_sequence *p, *child = NULL;
    int result;
    *out = NULL;
    if (!l || !r) { *out = l ? l : r; doc_seq_retain(*out); return XUI_OK; }
    p = doc_seq_clone(a, l->priority <= r->priority ? l : r);
    if (!p) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_syntax_push(a, p);
    if (result != XUI_OK) { doc_seq_release(p); return result; }
    if (l->priority <= r->priority) {
        result = doc_seq_join(a, p->right, r, &child);
        if (result == XUI_OK) { doc_seq_release(p->right); p->right = child; }
    } else {
        result = doc_seq_join(a, l, p->left, &child);
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
    result = doc_seq_syntax_push(a, copy);
    if (result != XUI_OK) { doc_seq_release(copy); return result; }
    if (at < size) {
        result = doc_seq_split(a, copy->left, at, &l, &r);
        if (result != XUI_OK) { doc_seq_release(copy); return result; }
        doc_seq_release(copy->left); copy->left = r; doc_seq_measure(copy);
        *left = l; *right = copy;
    } else if (at > size + p->length) {
        result = doc_seq_split(a, copy->right, at - size - p->length, &l, &r);
        if (result != XUI_OK) { doc_seq_release(copy); return result; }
        doc_seq_release(copy->right); copy->right = l; doc_seq_measure(copy);
        *left = copy; *right = r;
    } else if (at == size) {
        doc_seq_retain(copy->left); *left = copy->left;
        doc_seq_release(copy->left); copy->left = NULL; doc_seq_measure(copy);
        *right = copy;
    } else if (at == size + p->length) {
        doc_seq_retain(copy->right); *right = copy->right;
        doc_seq_release(copy->right); copy->right = NULL; doc_seq_measure(copy);
        *left = copy;
    } else {
        r = doc_seq_clone(a, p);
        if (!r) { doc_seq_release(copy); return XUI_ERROR_OUT_OF_MEMORY; }
        result = doc_seq_syntax_push(a, r);
        if (result != XUI_OK) { doc_seq_release(copy); doc_seq_release(r); return result; }
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
static uint64_t doc_seq_syntax_offset(uint64_t value, int64_t delta)
{
    return value == DOC_NONE ? DOC_NONE : (uint64_t)((int64_t)value + delta);
}
int doc_seq_read_syntax(doc_sequence* p, uint64_t index, xui_doc_inline_syntax_t* out)
{
    const uint64_t unit = sizeof(*out); uint64_t offset, left;
    int64_t source = 0, parent = 0;
    if (!out || index >= doc_seq_size(p) / unit || doc_seq_size(p) % unit) return XUI_ERROR_NOT_FOUND;
    offset = index * unit;
    while (p) {
        left = doc_seq_size(p->left);
        if (offset < left) {
            source += p->syntax_child_source; parent += p->syntax_child_index;
            p = p->left; continue;
        }
        if (offset < left + p->length) {
            if (!p->blob || unit > left + p->length - offset) return XUI_DOC_ERROR_SCHEMA;
            memcpy(out, p->blob->data + p->offset + offset - left, sizeof(*out));
            source += p->syntax_self_source; parent += p->syntax_self_index;
            out->iSourceStart = doc_seq_syntax_offset(out->iSourceStart, source);
            out->iSourceEnd = doc_seq_syntax_offset(out->iSourceEnd, source);
            out->iContentStart = doc_seq_syntax_offset(out->iContentStart, source);
            out->iContentEnd = doc_seq_syntax_offset(out->iContentEnd, source);
            out->iParentIndex = doc_seq_syntax_offset(out->iParentIndex, parent);
            return XUI_OK;
        }
        offset -= left + p->length;
        source += p->syntax_child_source; parent += p->syntax_child_index;
        p = p->right;
    }
    return XUI_DOC_ERROR_SCHEMA;
}
static int doc_seq_syntax_shift(doc_allocator* a, doc_sequence* base,
    int64_t source, int64_t index, doc_sequence** out)
{
    *out = NULL;
    if (!base) return XUI_OK;
    if (!source && !index) { doc_seq_retain(base); *out = base; return XUI_OK; }
    *out = doc_seq_clone(a, base);
    if (!*out) return XUI_ERROR_OUT_OF_MEMORY;
    doc_seq_syntax_add(*out, source, index); return XUI_OK;
}
int doc_seq_syntax_replace(doc_allocator* a, doc_sequence* base, uint64_t lo, uint64_t hi,
    doc_sequence* insert, uint64_t source_start, uint64_t old_end, uint64_t end,
    doc_sequence** out)
{
    const uint64_t unit = sizeof(xui_doc_inline_syntax_t);
    doc_sequence *prefix = NULL, *tail = NULL, *removed = NULL, *suffix = NULL;
    doc_sequence *adjusted_insert = NULL, *adjusted_suffix = NULL, *joined = NULL;
    uint64_t count = doc_seq_size(base) / unit, added = doc_seq_size(insert) / unit;
    int result;
    *out = NULL;
    if (doc_seq_size(base) % unit || doc_seq_size(insert) % unit || lo > hi || hi > count ||
        source_start > INT64_MAX || old_end > INT64_MAX || end > INT64_MAX ||
        lo > INT64_MAX || hi - lo > INT64_MAX || added > INT64_MAX) return XUI_DOC_ERROR_LIMIT;
    result = doc_seq_split(a, base, lo * unit, &prefix, &tail);
    if (result == XUI_OK) result = doc_seq_split(a, tail, (hi - lo) * unit, &removed, &suffix);
    if (result == XUI_OK) result = doc_seq_syntax_shift(a, insert, (int64_t)source_start, (int64_t)lo, &adjusted_insert);
    if (result == XUI_OK) result = doc_seq_syntax_shift(a, suffix,
        (int64_t)end - (int64_t)old_end, (int64_t)added - (int64_t)(hi - lo), &adjusted_suffix);
    if (result == XUI_OK) result = doc_seq_join(a, prefix, adjusted_insert, &joined);
    if (result == XUI_OK) result = doc_seq_join(a, joined, adjusted_suffix, out);
    doc_seq_release(prefix); doc_seq_release(tail); doc_seq_release(removed); doc_seq_release(suffix);
    doc_seq_release(adjusted_insert); doc_seq_release(adjusted_suffix); doc_seq_release(joined);
    return result;
}
int doc_seq_syntax_shift_source_range(doc_allocator* a, doc_sequence* base,
    uint64_t lo, uint64_t hi, int64_t source, doc_sequence** out)
{
    const uint64_t unit = sizeof(xui_doc_inline_syntax_t);
    doc_sequence *prefix = NULL, *tail = NULL, *middle = NULL, *suffix = NULL;
    doc_sequence *shifted = NULL, *joined = NULL;
    uint64_t count = doc_seq_size(base) / unit;
    int result;
    *out = NULL;
    if (doc_seq_size(base) % unit || lo > hi || hi > count)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (!source || lo == hi) {
        doc_seq_retain(base); *out = base; return XUI_OK;
    }
    result = doc_seq_split(a, base, lo * unit, &prefix, &tail);
    if (result == XUI_OK)
        result = doc_seq_split(a, tail, (hi - lo) * unit, &middle, &suffix);
    if (result == XUI_OK)
        result = doc_seq_syntax_shift(a, middle, source, 0, &shifted);
    if (result == XUI_OK) result = doc_seq_join(a, prefix, shifted, &joined);
    if (result == XUI_OK) result = doc_seq_join(a, joined, suffix, out);
    doc_seq_release(prefix); doc_seq_release(tail); doc_seq_release(middle);
    doc_seq_release(suffix); doc_seq_release(shifted); doc_seq_release(joined);
    return result;
}
int64_t doc_seq_source_shift(doc_sequence* p, uint64_t index)
{
    int64_t shift = 0;
    if (index >= doc_seq_size(p)) return 0;
    while (p) {
        uint64_t left = doc_seq_size(p->left);
        if (index < left) {
            shift += p->syntax_child_source; p = p->left;
        } else if (index < left + p->length) {
            return shift + p->syntax_self_source;
        } else {
            index -= left + p->length;
            shift += p->syntax_child_source; p = p->right;
        }
    }
    return 0;
}
int doc_seq_shift_source_suffix(doc_allocator* a, doc_sequence* base, uint64_t first,
    int64_t source, doc_sequence** out)
{
    doc_sequence *prefix = NULL, *suffix = NULL, *shifted = NULL;
    int result;
    *out = NULL;
    if (first > doc_seq_size(base)) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_seq_split(a, base, first, &prefix, &suffix);
    if (result == XUI_OK) result = doc_seq_syntax_shift(a, suffix, source, 0, &shifted);
    if (result == XUI_OK) result = doc_seq_join(a, prefix, shifted, out);
    doc_seq_release(prefix); doc_seq_release(suffix); doc_seq_release(shifted);
    return result;
}
int doc_seq_clear_source_shifts(doc_allocator* a, doc_sequence* base, doc_sequence** out)
{
    doc_sequence *copy, *left = NULL, *right = NULL;
    int result;
    *out = NULL;
    if (!base) return XUI_OK;
    copy = doc_seq_clone(a, base);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_clear_source_shifts(a, base->left, &left);
    if (result == XUI_OK) result = doc_seq_clear_source_shifts(a, base->right, &right);
    if (result == XUI_OK) {
        doc_seq_release(copy->left); doc_seq_release(copy->right);
        copy->left = left; copy->right = right;
        copy->syntax_self_source = copy->syntax_self_index = 0;
        copy->syntax_child_source = copy->syntax_child_index = 0;
        *out = copy;
    } else { doc_seq_release(left); doc_seq_release(right); doc_seq_release(copy); }
    return result;
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
    xui_doc_attributes_t attrs = d->tAttributes;
    if (!p) return NULL;
    doc_memory_tag(p, DOC_MEMORY_NODE);
    atomic_init(&p->refs, 1);
    p->id = id; p->parent = parent; p->kind = d->iKind;
    p->source_start = p->source_end = DOC_NONE;
    p->syntax_start = p->syntax_end = DOC_NONE;
    p->provenance_base = DOC_NONE;
    p->block_ordinal = DOC_NONE;
    if (!attrs.iRowSpan) attrs.iRowSpan = 1;
    if (!attrs.iColumnSpan) attrs.iColumnSpan = 1;
    if (!attrs.iHeadingLevel && p->kind == XUI_DOC_HEADING) attrs.iHeadingLevel = 1;
    if (doc_node_set_attrs(a, p, &attrs) != XUI_OK) goto failed;
    p->text = doc_seq_text(a, d->sText, d->iTextBytes);
    if (d->iTextBytes && !p->text) goto failed;
    if (d->sResource && *d->sResource) { p->resource = doc_blob_new(a, d->sResource, strlen(d->sResource)); if (!p->resource) goto failed; }
    if (d->sInfo && *d->sInfo) { p->info = doc_blob_new(a, d->sInfo, strlen(d->sInfo)); if (!p->info) goto failed; }
    if (d->sTitle && *d->sTitle) { p->title = doc_blob_new(a, d->sTitle, strlen(d->sTitle)); if (!p->title) goto failed; }
    if (d->sLinkTarget && *d->sLinkTarget) { p->link_target = doc_blob_new(a, d->sLinkTarget, strlen(d->sLinkTarget)); if (!p->link_target) goto failed; }
    if (d->sLinkTitle && *d->sLinkTitle) { p->link_title = doc_blob_new(a, d->sLinkTitle, strlen(d->sLinkTitle)); if (!p->link_title) goto failed; }
    if (p->kind == XUI_DOC_EXTENSION) {
        p->extension_version = d->iExtensionVersion ? d->iExtensionVersion : 1;
        p->extension_required = d->bExtensionRequired;
        if (d->iExtensionPayloadBytes) {
            p->extension_payload = doc_blob_new(a, d->pExtensionPayload, d->iExtensionPayloadBytes);
            if (!p->extension_payload) goto failed;
        }
    }
    return p;
failed:
    doc_node_release(p); return NULL;
}
doc_node* doc_node_clone(doc_allocator* a, const doc_node* src)
{
    doc_node* p = doc_alloc(a, sizeof(*p));
    if (!p) return NULL;
    doc_memory_tag(p, DOC_MEMORY_NODE);
    memcpy(&p->id, &src->id, sizeof(*p) - offsetof(doc_node, id)); atomic_init(&p->refs, 1);
    doc_attribute_retain(p->attrs);
    doc_seq_retain(p->text); doc_seq_retain(p->children); doc_seq_retain(p->provenance);
    doc_blob_retain(p->resource); doc_blob_retain(p->info); doc_blob_retain(p->title);
    doc_blob_retain(p->link_target); doc_blob_retain(p->link_title);
    doc_blob_retain(p->column_widths); doc_blob_retain(p->extension_payload);
    doc_blob_retain(p->quote_prefixes); doc_blob_retain(p->syntax_aux); doc_blob_retain(p->list_indents);
    return p;
}
void doc_node_retain(doc_node* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_node_release(doc_node* p)
{
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    doc_seq_release(p->text); doc_seq_release(p->children); doc_seq_release(p->provenance);
    doc_blob_release(p->resource); doc_blob_release(p->info); doc_blob_release(p->title);
    doc_blob_release(p->link_target); doc_blob_release(p->link_title);
    doc_blob_release(p->column_widths); doc_blob_release(p->extension_payload);
    doc_blob_release(p->quote_prefixes); doc_blob_release(p->syntax_aux); doc_blob_release(p->list_indents);
    doc_attribute_release(p->attrs);
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
    doc_memory_tag(p, DOC_MEMORY_INDEX);
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
        doc_memory_tag(next, DOC_MEMORY_INDEX);
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
    doc_memory_tag(p, DOC_MEMORY_STATE);
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
    doc_memory_tag(p, DOC_MEMORY_STATE);
    memcpy(&p->allocator, &src->allocator, sizeof(*p) - offsetof(doc_state, allocator)); atomic_init(&p->refs, 1);
    doc_allocator_retain(p->allocator); doc_index_retain(p->index); doc_seq_retain(p->source);
    doc_seq_retain(p->references); doc_seq_retain(p->reference_values); doc_seq_retain(p->inline_syntax); doc_seq_retain(p->reference_candidates);
    return p;
}
void doc_state_retain(doc_state* p) { if (p) atomic_fetch_add(&p->refs, 1); }
void doc_state_release(doc_state* p)
{
    doc_allocator* a;
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    a = p->allocator;
    doc_index_release(p->index); doc_seq_release(p->source); doc_seq_release(p->references); doc_seq_release(p->reference_values);
    doc_seq_release(p->inline_syntax); doc_seq_release(p->reference_candidates); doc_free(p); doc_allocator_release(a);
}
int doc_state_set(doc_state* s, doc_node* node)
{
    doc_index* next = NULL;
    doc_node* previous = doc_index_get(s->index, node->id);
    int existed = previous != NULL;
    uint64_t old_bytes = previous ? doc_seq_size(previous->text) : 0;
    uint64_t old_payload = previous && previous->extension_payload ? previous->extension_payload->size : 0;
    uint64_t new_payload = node->extension_payload ? node->extension_payload->size : 0;
    if (new_payload > s->allocator->max_bytes - (s->payload_bytes - old_payload)) return XUI_DOC_ERROR_LIMIT;
    int result = doc_index_set(s->allocator, s->index, node->id, node, &next);
    if (result != XUI_OK) return result;
    doc_index_release(s->index); s->index = next;
    s->text_bytes = s->text_bytes - old_bytes + doc_seq_size(node->text);
    s->payload_bytes = s->payload_bytes - old_payload + new_payload;
    if (!existed) s->node_count++;
    s->source_blocks_indexed = 0;
    return XUI_OK;
}
int doc_state_remove(doc_state* s, uint64_t id)
{
    doc_index* next = NULL;
    int result;
    doc_node* previous = doc_index_get(s->index, id);
    uint64_t bytes, payload_bytes;
    if (!previous) return XUI_ERROR_NOT_FOUND;
    bytes = doc_seq_size(previous->text);
    payload_bytes = previous->extension_payload ? previous->extension_payload->size : 0;
    result = doc_index_set(s->allocator, s->index, id, NULL, &next);
    if (result != XUI_OK) return result;
    doc_index_release(s->index); s->index = next; s->node_count--; s->text_bytes -= bytes;
    s->payload_bytes -= payload_bytes;
    s->source_blocks_indexed = 0;
    return XUI_OK;
}
static int64_t doc_node_child_shift(doc_sequence* children, const doc_node* p)
{
    return !children || p->block_ordinal == DOC_NONE ? 0 :
        doc_seq_source_shift(children, p->block_ordinal) - p->block_shift_base;
}
int64_t doc_node_block_shift(const doc_state* s, const doc_node* p)
{
    doc_node* root;
    if (!s || !p || p->block_ordinal == DOC_NONE) return 0;
    root = doc_index_get(s->index, DOC_ROOT);
    return doc_node_child_shift(root ? root->children : NULL, p);
}
void doc_node_source_range_with_children(doc_sequence* children, const doc_node* p,
    doc_node_source_range* out)
{
    int64_t shift = doc_node_child_shift(children, p);
    out->source_start = doc_seq_syntax_offset(p->source_start, shift);
    out->source_end = doc_seq_syntax_offset(p->source_end, shift);
    out->syntax_start = doc_seq_syntax_offset(p->syntax_start, shift);
    out->syntax_end = doc_seq_syntax_offset(p->syntax_end, shift);
}
void doc_node_source_range_get(const doc_state* s, const doc_node* p, doc_node_source_range* out)
{
    doc_node* root = doc_index_get(s->index, DOC_ROOT);
    doc_node_source_range_with_children(root ? root->children : NULL, p, out);
}
int doc_node_info(const doc_state* s, const doc_node* p, xui_doc_node_info_t* out)
{
    doc_node_source_range range;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!p) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(s, p, &range);
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iKind = p->kind; out->iId = p->id; out->iParentId = p->parent;
    out->iChildCount = doc_seq_size(p->children); out->iTextBytes = doc_seq_size(p->text);
    out->iSourceSegmentCount = doc_seq_size(p->provenance) / sizeof(xui_doc_source_segment_t);
    out->tAttributes = *p->attrs; out->sResource = doc_string(p->resource);
    out->sInfo = doc_string(p->info); out->sTitle = doc_string(p->title);
    out->sLinkTarget = doc_string(p->link_target); out->sLinkTitle = doc_string(p->link_title);
    out->iExtensionPayloadBytes = p->extension_payload ? p->extension_payload->size : 0;
    out->iExtensionVersion = p->extension_version;
    out->bExtensionRequired = p->extension_required;
    out->iSourceStart = range.source_start; out->iSourceEnd = range.source_end; out->bSourceExact = p->source_exact;
    out->iSyntaxStart = range.syntax_start; out->iSyntaxEnd = range.syntax_end;
    return XUI_OK;
}
int doc_text_kind(uint32_t k)
{
    return k == XUI_DOC_TEXT || k == XUI_DOC_CODE_BLOCK || k == XUI_DOC_HTML ||
        k == XUI_DOC_MATH || k == XUI_DOC_DIAGRAM || k == XUI_DOC_IMAGE ||
        k == XUI_DOC_FRONT_MATTER || k == XUI_DOC_EXTENSION || k == XUI_DOC_FOOTNOTE_REF;
}
int doc_selectable_object_kind(uint32_t k)
{
    return k == XUI_DOC_IMAGE || k == XUI_DOC_MATH || k == XUI_DOC_DIAGRAM ||
        k == XUI_DOC_HTML || k == XUI_DOC_EXTENSION;
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
        a->fFontSize < 0 || a->fFontSize > DOC_MAX_LAYOUT_VALUE ||
        a->fWidth < 0 || a->fWidth > DOC_MAX_LAYOUT_VALUE ||
        a->fHeight < 0 || a->fHeight > DOC_MAX_LAYOUT_VALUE ||
        a->fParagraphSpacing < 0 || a->fParagraphSpacing > DOC_MAX_LAYOUT_VALUE ||
        !memchr(a->sFontFamily, 0, sizeof(a->sFontFamily)) ||
        ((a->iFlags & XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO) && a->iTextColor) ||
        ((a->iFlags & XUI_DOC_TEXT_COLOR_CURRENT) &&
            (a->iTextColor || (a->iFlags & XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO))) ||
        ((a->iFlags & XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO) && a->iBackgroundColor) ||
        ((a->iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT) &&
            (a->iBackgroundColor || (a->iFlags & XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO))) ||
        ((a->iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT) && a->iAlignment)) return 0;
    if (!doc_utf8(a->sFontFamily, strlen(a->sFontFamily)) ||
        !doc_language_valid(a->sLanguage)) return 0;
    if ((a->iMarks & XUI_DOC_SUBSCRIPT) && (a->iMarks & XUI_DOC_SUPERSCRIPT)) return 0;
    return kind >= XUI_DOC_ROOT && kind <= XUI_DOC_EXTENSION;
}
uint64_t doc_child_index(const doc_node* parent, uint64_t id)
{
    uint64_t i, n = doc_seq_size(parent->children);
    for (i = 0; i < n; i++) if (doc_seq_get_id(parent->children, i) == id) return i;
    return DOC_NONE;
}

#endif
