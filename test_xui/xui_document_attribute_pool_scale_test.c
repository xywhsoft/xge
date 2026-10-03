#include "../src/xui_document_internal.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

static void* scale_alloc(void* user, size_t bytes) { (void)user; return malloc(bytes); }
static void scale_free(void* user, void* pointer) { (void)user; free(pointer); }

static int valid_tree(const doc_attribute* node, size_t* entries, unsigned* height)
{
    unsigned left, right;
    const doc_attribute* collision;
    if (!node) { *height = 0; return 1; }
    if ((node->left && node->left->hash >= node->hash) ||
        (node->right && node->right->hash <= node->hash) ||
        !valid_tree(node->left, entries, &left) ||
        !valid_tree(node->right, entries, &right)) return 0;
    if (left > right + 1 || right > left + 1 ||
        node->height != (left > right ? left : right) + 1) return 0;
    (*entries)++;
    for (collision = node->next; collision; collision = collision->next) {
        if (collision->hash != node->hash || collision->left || collision->right) return 0;
        (*entries)++;
    }
    *height = node->height;
    return 1;
}

static int valid_pool(doc_allocator* allocator, size_t expected)
{
    size_t entries = 0;
    unsigned bucket, height;
    for (bucket = 0; bucket < DOC_ATTR_BUCKETS; bucket++) {
        if (!valid_tree(allocator->attributes[bucket], &entries, &height) || height > 32)
            return 0;
    }
    return entries == expected;
}

static int run(unsigned count)
{
    doc_allocator* allocator = calloc(1, sizeof(*allocator));
    doc_node** nodes = calloc(count, sizeof(*nodes));
    xui_doc_node_desc_t desc = {0};
    clock_t begin, after_insert, after_release;
    unsigned i;
    int complete;
    if (!allocator || !nodes) { free(allocator); free(nodes); return 1; }
    allocator->alloc = scale_alloc; allocator->free = scale_free;
    atomic_init(&allocator->refs, 1);
    atomic_init(&allocator->live, 0); atomic_init(&allocator->peak, 0);
    atomic_init(&allocator->allocations, 0);
    atomic_init(&allocator->attribute_lock, 0);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_TEXT;
    begin = clock();
    for (i = 0; i < count; i++) {
        desc.tAttributes.fFontSize = (float)(i + 1);
        desc.tAttributes.sLanguage = i & 1 ? "TR-tr" : "EN-us";
        nodes[i] = doc_node_new(allocator, i + 2, 1, &desc);
        if (!nodes[i]) break;
        if (strcmp(nodes[i]->attrs->sLanguage, i & 1 ? "tr-tr" : "en-us")) {
            doc_node_release(nodes[i]); nodes[i] = NULL; break;
        }
    }
    complete = i == count;
    after_insert = clock();
    if (complete && !valid_pool(allocator, count)) complete = 0;
    while (i) doc_node_release(nodes[--i]);
    after_release = clock();
    if (!complete || atomic_load(&allocator->live)) {
        fprintf(stderr, "%u nodes left %llu bytes allocated\n", count,
            (unsigned long long)atomic_load(&allocator->live));
        free(nodes); doc_allocator_release(allocator); return 1;
    }
    printf("%u distinct attributes: insert %.3f s, release %.3f s\n",
        count, (double)(after_insert - begin) / CLOCKS_PER_SEC,
        (double)(after_release - after_insert) / CLOCKS_PER_SEC);
    free(nodes); doc_allocator_release(allocator); return 0;
}

static int churn(void)
{
    enum { COUNT = 50000 };
    doc_allocator* allocator = calloc(1, sizeof(*allocator));
    doc_node** nodes = calloc(COUNT, sizeof(*nodes));
    xui_doc_node_desc_t desc = {0};
    unsigned i;
    int okay = 1;
    if (!allocator || !nodes) { free(allocator); free(nodes); return 1; }
    allocator->alloc = scale_alloc; allocator->free = scale_free;
    atomic_init(&allocator->refs, 1);
    atomic_init(&allocator->live, 0); atomic_init(&allocator->peak, 0);
    atomic_init(&allocator->allocations, 0);
    atomic_init(&allocator->attribute_lock, 0);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_TEXT;
    for (i = 0; i < COUNT; i++) {
        desc.tAttributes.fFontSize = (float)(i + 1);
        nodes[i] = doc_node_new(allocator, i + 2, 1, &desc);
        if (!nodes[i]) { okay = 0; break; }
    }
    if (okay) for (i = 0; i < COUNT; i += 2) {
        doc_node_release(nodes[i]); nodes[i] = NULL;
    }
    if (okay && !valid_pool(allocator, COUNT / 2)) okay = 0;
    if (okay) for (i = 0; i < COUNT; i += 2) {
        desc.tAttributes.fFontSize = (float)(i + 1);
        nodes[i] = doc_node_new(allocator, i + 2, 1, &desc);
        if (!nodes[i]) { okay = 0; break; }
    }
    if (okay && !valid_pool(allocator, COUNT)) okay = 0;
    if (okay) for (i = 1; i < COUNT; i += 2) {
        doc_node* duplicate;
        desc.tAttributes.fFontSize = (float)(i + 1);
        duplicate = doc_node_new(allocator, COUNT + i + 2, 1, &desc);
        if (!duplicate) { okay = 0; break; }
        if (duplicate->attrs != nodes[i]->attrs) okay = 0;
        doc_node_release(duplicate);
        if (!okay) break;
    }
    for (i = 0; i < COUNT; i++) {
        unsigned index = (i * 32771u) % COUNT;
        doc_node_release(nodes[index]);
    }
    if (atomic_load(&allocator->live)) okay = 0;
    free(nodes); doc_allocator_release(allocator);
    if (okay) puts("Attribute pool churn: 50000 distinct attributes, alternate deletion/reinsertion, reuse and permuted release passed");
    return !okay;
}

/* FNV-64 collisions are impractical to find in a deterministic test. Move
 * three live entries from distinct buckets into one synthetic equal-hash
 * chain, then exercise lookup and removal of its root, middle and tail. */
static int collision_removal(void)
{
    static const unsigned orders[][3] = {{1, 0, 2}, {0, 1, 2}, {2, 1, 0}};
    unsigned mode;
    for (mode = 0; mode < sizeof(orders) / sizeof(orders[0]); mode++) {
        doc_allocator* allocator = calloc(1, sizeof(*allocator));
        doc_node* nodes[3] = {0};
        doc_attribute* entries[3] = {0};
        unsigned buckets[3] = {0}, found = 0, value, step;
        xui_doc_node_desc_t desc = {0};
        int okay = allocator != NULL;
        if (!okay) return 1;
        allocator->alloc = scale_alloc; allocator->free = scale_free;
        atomic_init(&allocator->refs, 1);
        atomic_init(&allocator->live, 0); atomic_init(&allocator->peak, 0);
        atomic_init(&allocator->allocations, 0);
        atomic_init(&allocator->attribute_lock, 0);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_TEXT;
        for (value = 1; value < 256 && found < 3; value++) {
            doc_node* node; doc_attribute* entry;
            unsigned bucket, i;
            desc.tAttributes.fFontSize = (float)value;
            node = doc_node_new(allocator, value + 1, 1, &desc);
            if (!node) { okay = 0; break; }
            entry = (doc_attribute*)((char*)node->attrs - offsetof(doc_attribute, value));
            bucket = (unsigned)(entry->hash % DOC_ATTR_BUCKETS);
            for (i = 0; i < found; i++) if (buckets[i] == bucket) break;
            if (i != found) { doc_node_release(node); continue; }
            nodes[found] = node; entries[found] = entry; buckets[found] = bucket;
            found++;
        }
        if (found != 3 || !valid_pool(allocator, 3)) okay = 0;
        if (okay) {
            for (step = 0; step < 3; step++) {
                if (allocator->attributes[buckets[step]] != entries[step] ||
                    entries[step]->left || entries[step]->right || entries[step]->next)
                    okay = 0;
            }
        }
        if (okay) {
            uint64_t hash = entries[0]->hash;
            doc_node* duplicate;
            for (step = 0; step < 3; step++) allocator->attributes[buckets[step]] = NULL;
            for (step = 0; step < 3; step++) entries[step]->hash = hash;
            entries[1]->next = entries[0]; entries[0]->next = entries[2];
            allocator->attributes[buckets[0]] = entries[1];
            if (!valid_pool(allocator, 3)) okay = 0;
            desc.tAttributes.fFontSize = nodes[0]->attrs->fFontSize;
            duplicate = doc_node_new(allocator, 999, 1, &desc);
            if (!duplicate || duplicate->attrs != nodes[0]->attrs) okay = 0;
            doc_node_release(duplicate);
            if (!valid_pool(allocator, 3)) okay = 0;
            for (step = 0; step < 3; step++) {
                unsigned index = orders[mode][step];
                doc_node_release(nodes[index]); nodes[index] = NULL;
                if (!valid_pool(allocator, 2 - step)) okay = 0;
            }
        }
        for (step = 0; step < 3; step++) doc_node_release(nodes[step]);
        if (atomic_load(&allocator->live)) okay = 0;
        doc_allocator_release(allocator);
        if (!okay) {
            fprintf(stderr, "Attribute pool synthetic collision removal failed: order %u\n", mode);
            return 1;
        }
    }
    puts("Attribute pool synthetic collision lookup and removal: root, middle and tail passed");
    return 0;
}

int main(int argc, char** argv)
{
    const unsigned sizes[] = {25000, 50000, 100000, 200000};
    unsigned i;
    if (argc == 2 && !strcmp(argv[1], "--million")) return run(1000000);
    if (argc != 1) { fprintf(stderr, "Usage: %s [--million]\n", argv[0]); return 2; }
    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
        if (run(sizes[i])) return 1;
    if (churn()) return 1;
    return collision_removal();
}
