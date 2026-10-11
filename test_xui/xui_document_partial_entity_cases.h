#ifndef XUI_DOCUMENT_PARTIAL_ENTITY_CASES_H
#define XUI_DOCUMENT_PARTIAL_ENTITY_CASES_H
#include "xui_document_partial_cross_cases.h"
static const struct entity_sample {
    const char *name, *before, *after; unsigned left_entity, right_entity, marks;
} entity_samples[] = {
    {"left-strong", "ab &amp; __ke&NotEqualTilde;ep__\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __ke&#8770;%s__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", 1, 0, XUI_DOC_BOLD},
    {"right-strong", "ab &amp; __keep__\n\n**xx&NotEqualTilde;!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n",
        "ab &amp; __ke%s__**&#824;!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n", 0, 1, XUI_DOC_BOLD},
    {"both-marks", "ab &amp; __ke&NotEqualTilde;ep__\n\n*xx&NotEqualTilde;!cd* &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __ke&#8770;%s__*&#824;!cd* &lt; **tail**\n\n[r]: <raw> 'T'\n", 1, 1, XUI_DOC_BOLD},
    {"left-link", "ab &amp; [__ke&NotEqualTilde;ep__][r]\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; [__ke&#8770;%s__][r]!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", 1, 0, XUI_DOC_BOLD | XUI_DOC_LINK},
    {"hidden-quote", "> ab &amp; [__ke&NotEqualTilde;ep__][r]\n>\n> [r]: <raw> 'T'\n>\n> *xx&NotEqualTilde;!cd* &lt; **tail**\n",
        "> ab &amp; [__ke&#8770;%s__][r]*&#824;!cd* &lt; **tail**\n>\n> [r]: <raw> 'T'\n>\n", 1, 1, XUI_DOC_BOLD | XUI_DOC_LINK},
    {"footnote", "use[^n]\n\n[^n]: ab &amp; __ke&NotEqualTilde;ep__\n    \n    xx!cd &lt; **tail**\n    \n    [r]: <raw> 'T'\n",
        "use[^n]\n\n[^n]: ab &amp; __ke&#8770;%s__!cd &lt; **tail**\n    \n    [r]: <raw> 'T'\n", 1, 0, XUI_DOC_BOLD},
    {"atx-left", "## ab &amp; __ke&NotEqualTilde;ep__ ##\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "## ab &amp; __ke&#8770;%s__!cd &lt; **tail** ##\n\n[r]: <raw> 'T'\n", 1, 0, XUI_DOC_BOLD},
    {"setext-both", "ab &amp; __ke&NotEqualTilde;ep__\n===\n\n*xx&NotEqualTilde;!cd* &lt; **tail**\n---\n\n[r]: <raw> 'T'\n",
        "ab &amp; __ke&#8770;%s__*&#824;!cd* &lt; **tail**\n===\n\n[r]: <raw> 'T'\n", 1, 1, XUI_DOC_BOLD},
    {"right-link", "ab &amp; ke&NotEqualTilde;ep\n\n[xx&NotEqualTilde;!cd][r] &lt; __tail__\n\n[r]: <raw> 'T'\n",
        "ab &amp; ke&#8770;%s[&#824;!cd][r] &lt; __tail__\n\n[r]: <raw> 'T'\n", 1, 1, 0}
};
static int entity_find_text(xui_document_snapshot snapshot, uint64_t id, const char* needle, xui_doc_position_t* position)
{
    xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK);
    if (info.iKind == XUI_DOC_TEXT) {
        char text[1024]; uint64_t bytes;
        TABLE_CHECK(xuiDocumentSnapshotCopyText(snapshot, id, text, sizeof(text), &bytes) == XUI_OK && bytes < sizeof(text)); text[bytes] = 0;
        const char* word = strstr(text, needle);
        if (word) { position->iNodeId = id; position->iOffset = (uint64_t)(word - text); return 1; }
    }
    for (uint64_t i = 0; i < info.iChildCount; i++) {
        uint64_t child; TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        if (entity_find_text(snapshot, child, needle, position)) return 1;
    }
    return 0;
}
static xui_doc_range_t entity_range(xui_document d, xui_document_snapshot old, const struct entity_sample* c,
    unsigned reverse, xui_doc_position_t* tail)
{
    xui_doc_range_t range = {0}; uint64_t raw; int quality;
    const char *entity = strstr(c->before, "&NotEqualTilde;"), *last_entity = entity, *next;
    TABLE_CHECK(entity);
    while ((next = strstr(last_entity + 1, "&NotEqualTilde;")) != NULL) last_entity = next;
    if (c->left_entity) {
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(entity - c->before) + 1, &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_COLLAPSED);
        range.tAnchor.iOffset = 3;
    } else TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(strstr(c->before, "keep") - c->before) + 2, &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    if (c->right_entity) {
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(last_entity - c->before) + 1, &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_COLLAPSED);
        range.tCaret.iOffset = 3;
    } else TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(strstr(c->before, "!cd") - c->before), &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret.iAffinity = XUI_DOC_BEFORE;
    TABLE_CHECK(range.tAnchor.iDocumentId == xuiDocumentGetIdentity(d));
    TABLE_CHECK(xuiDocumentPositionToSource(old, &range.tAnchor, &raw, &quality) == XUI_OK && quality == (c->left_entity ? XUI_DOC_MAP_COLLAPSED : XUI_DOC_MAP_EXACT));
    TABLE_CHECK(xuiDocumentPositionToSource(old, &range.tCaret, &raw, &quality) == XUI_OK && quality == (c->right_entity ? XUI_DOC_MAP_COLLAPSED : XUI_DOC_MAP_EXACT));
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(strstr(c->before, "!cd") - c->before) + 1, tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}
static void entity_sources(unsigned fixture, unsigned payload, const char* internal, const char* final,
    unsigned bom, struct entity_sample* sample, char** before, char** after)
{
    const char* raw_texts[] = {"Z\\*\\[\\]", "Z\\`\\`", ""}; char wanted[4096]; size_t width = strlen(internal);
    *sample = entity_samples[fixture]; snprintf(wanted, sizeof(wanted), sample->after, raw_texts[payload]);
    *before = delete_ending(sample->before, internal, bom); *after = delete_ending(wanted, internal, bom);
    heading_edge_replace_ending(*before + strlen(*before) - width, width, final);
    if (fixture == 4) {
        char* ending = strpbrk(*after, "\r\n"); TABLE_CHECK(ending); heading_edge_replace_ending(ending, width, *final ? final : "\n");
    } else heading_edge_replace_ending(*after + strlen(*after) - width, width, final);
    sample->before = *before; sample->after = *after;
}
static void entity_verify(xui_document d, xui_document_snapshot current, const struct entity_sample* sample,
    const char* after, unsigned payload, const xui_doc_range_t* range, unsigned reverse,
    const xui_doc_position_t* caret, const xui_doc_position_t* tail, xui_document_change_set change)
{
    const char* raw_texts[] = {"Z\\*\\[\\]", "Z\\`\\`", ""}; xui_doc_node_info_t typed = {0};
    xui_doc_position_t mapped; uint64_t at, removed = sample->right_entity ? 0 : (reverse ? range->tAnchor : range->tCaret).iOffset; int quality;
    xui_doc_position_t checked_caret = *caret;
    table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &checked_caret);
    typed.iSize = sizeof(typed);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret->iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == sample->marks);
    TABLE_CHECK(xuiDocumentMapPosition(change, tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        mapped.iNodeId == tail->iNodeId && mapped.iOffset == tail->iOffset - removed);
    const char* word = strstr(after, sample->left_entity ? "ke&#8770;" : "ke"); TABLE_CHECK(word);
    uint64_t caret_at = (uint64_t)(word - after) + strlen(sample->left_entity ? "ke&#8770;" : "ke") + strlen(raw_texts[payload]);
    TABLE_CHECK(xuiDocumentPositionToSource(current, &checked_caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
    TABLE_CHECK(xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == (uint64_t)(strstr(after, "!cd") - after) + 1);
}
static int entity_strict(void)
{
    const char *texts[] = {"Z*[]", "Z``", ""}, *raw_texts[] = {"Z\\*\\[\\]", "Z\\`\\`", ""};
    unsigned total = 0, differences = 0, rejected = 0;
    for (unsigned fixture = 0; fixture < 9; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        const struct entity_sample* c = entity_samples + fixture; char after[4096];
        snprintf(after, sizeof(after), c->after, raw_texts[payload]);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(c->before, &allocation), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_document_change_set change = NULL;
        xui_doc_position_t caret, tail, mapped; xui_doc_node_info_t typed = {0}; uint64_t at, bytes; int quality, result;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        xui_doc_range_t range = entity_range(d, old, c, reverse, &tail);
        uint64_t removed = c->right_entity ? 0 : (reverse ? range.tAnchor : range.tCaret).iOffset;
        result = partial_cross_execute(d, &range, texts[payload], &caret, &change);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        char raw[4096]; TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, sizeof(raw), &bytes) == XUI_OK && bytes < sizeof(raw)); raw[bytes] = 0;
        int equal = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes);
        printf("Partial entity fixture=%u payload=%u reverse=%u result=%d exact_source=%d\n", fixture, payload, reverse, result, equal); fflush(stdout);
        if (result == XUI_OK) {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            integration_equivalent(current, raw); boundary_caret(d, current, &caret); differences += !equal;
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == c->marks);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - removed);
            if (equal) {
                const char* word = strstr(after, c->left_entity ? "ke&#8770;" : "ke"); TABLE_CHECK(word);
                uint64_t caret_at = (uint64_t)(word - after) + strlen(c->left_entity ? "ke&#8770;" : "ke") + strlen(raw_texts[payload]);
                TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
                TABLE_CHECK(xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == (uint64_t)(strstr(after, "!cd") - after) + 1);
            }
            xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        } else {
            TABLE_CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && !change && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
            table_source(current, c->before); table_equivalent(current, old); rejected++;
        }
        table_source(old, c->before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d); xuiDocumentRelease(oracle); TABLE_CHECK(!allocation.live); total++;
    }
    TABLE_CHECK(total == 54);
    printf("Partial entity diagnostic: %u checks; differences=%u rejected=%u; independent entity slices/full semantics/source/index/inherited marks/exact caret/stable-tail/history/zero leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}
static int entity_goldens(void)
{
    const char *endings[] = {"\n", "\r\n", "\r", ""}, *texts[] = {"Z*[]", "Z``", ""}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < 9; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned internal = 0; internal < 3; internal++) for (unsigned final = 0; final < 4; final++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        struct entity_sample sample; char *before, *after;
        entity_sources(fixture, payload, endings[internal], endings[final], bom, &sample, &before, &after);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change; xui_doc_position_t caret, tail;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        xui_doc_range_t range = entity_range(d, old, &sample, reverse, &tail);
        printf("Partial entity golden fixture=%u payload=%u internal=%u final=%u bom=%u reverse=%u\n", fixture, payload, internal, final, bom, reverse); fflush(stdout);
        TABLE_CHECK(partial_cross_execute(d, &range, texts[payload], &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        entity_verify(d, current, &sample, after, payload, &range, reverse, &caret, &tail, change); xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 1296); puts("Partial entity matrix: 1296 independent source/scalar-fragments/marks/index/exact-caret/stable-tail/mixed-EOL/BOM/history/zero-leak goldens passed"); return 0;
}
typedef struct entity_public_fault_allocator {
    table_allocator base;
} entity_public_fault_allocator;
static void* entity_public_fault_alloc(void* user, size_t bytes)
{
    entity_public_fault_allocator* a = user;
    return table_alloc(&a->base, bytes);
}
static void entity_public_fault_free(void* user, void* p)
{ entity_public_fault_allocator* a = user; table_free(&a->base, p); }
static void entity_public_fault_changed(xui_document d, xui_document_change_set change, void* user)
{ (void)d; TABLE_CHECK(change); (*(unsigned*)user)++; }
static int entity_public_fault_attempt(xui_document d, const xui_doc_range_t* range, const char* text,
    entity_public_fault_allocator* a, long point, xui_doc_position_t* caret, xui_document_change_set* change)
{
    a->base.remaining = point; return partial_cross_execute(d, range, text, caret, change);
}
static int entity_public_faults(void)
{
    const char* texts[] = {"Z*[]", "Z``", ""}; unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 9; fixture++) for (unsigned payload = 0; payload < 3; payload++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        struct entity_sample sample; char *before, *after; long point;
        entity_sources(fixture, payload, "\r\n", "", 1, &sample, &before, &after);
        for (point = 0; point < 4096; point++) {
            entity_public_fault_allocator a = {0}; a.base.remaining = -1;
            xui_doc_desc_t desc = {0}; xui_document d;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = entity_public_fault_alloc; desc.onFree = entity_public_fault_free; desc.pAllocatorUser = &a;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK);
            xui_document_snapshot old, current, restored; xui_document_change_set change = NULL; xui_doc_position_t tail, caret;
            uint64_t revision, token; unsigned notifications = 0; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, entity_public_fault_changed, &notifications, &token) == XUI_OK);
            xui_doc_range_t range = entity_range(d, old, &sample, reverse, &tail); revision = xuiDocumentGetRevision(d);
            result = entity_public_fault_attempt(d, &range, texts[payload], &a, point, &caret, &change); a.base.remaining = -1;
            const int wanted_error = XUI_ERROR_OUT_OF_MEMORY;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == wanted_error && !change && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(entity_public_fault_attempt(d, &range, texts[payload], &a, -1, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(change && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1);
            entity_verify(d, current, &sample, after, payload, &range, reverse, &caret, &tail, change); xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point > 0 && point < 4096); printf("Partial entity fault fixture=%u payload=%u reverse=%u: %ld points atomic/poisoned-commit/no-ghost-notifications/same-document-retry/source/exact-caret/stable-tail/history/zero-leaks passed\n", fixture, payload, reverse, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 54);
    puts("Partial entity public faults: 54 OOM scans passed");
    return 0;
}

#endif
