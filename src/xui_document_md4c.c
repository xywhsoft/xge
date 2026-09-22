#include "xui_document_md4c.h"

/* Keep upstream unchanged while routing its complete allocation lifetime
 * through the document allocator. TLS isolates independent parser workers;
 * save/restore also makes nested calls safe. */
static _Thread_local doc_allocator* doc_md4c_allocator;
static _Thread_local int doc_md4c_oom;
static void* doc_md4c_malloc(size_t bytes)
{
    void* p = doc_alloc(doc_md4c_allocator, bytes); if (!p) doc_md4c_oom = 1; return p;
}
static void* doc_md4c_realloc(void* previous, size_t bytes)
{
    void* p = doc_realloc(doc_md4c_allocator, previous, bytes); if (!p && bytes) doc_md4c_oom = 1; return p;
}
#define malloc doc_md4c_malloc
#define realloc doc_md4c_realloc
#define free doc_free
#define md_parse doc_md4c_upstream_parse
#include "../lib/md4c/md4c.c"
#undef md_parse
#undef free
#undef realloc
#undef malloc

int doc_md4c_parse(doc_allocator* allocator, const char* text, MD_SIZE size, const MD_PARSER* parser, void* user)
{
    doc_allocator* previous = doc_md4c_allocator;
    int previous_oom = doc_md4c_oom, result;
    doc_md4c_allocator = allocator; doc_md4c_oom = 0;
    result = doc_md4c_upstream_parse(text, size, parser, user);
    if (doc_md4c_oom) result = XUI_ERROR_OUT_OF_MEMORY;
    doc_md4c_allocator = previous; doc_md4c_oom = previous_oom;
    return result;
}
