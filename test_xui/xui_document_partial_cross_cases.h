#ifndef XUI_DOCUMENT_PARTIAL_CROSS_CASES_H
#define XUI_DOCUMENT_PARTIAL_CROSS_CASES_H
#include "xui_document_nonempty_seam_cases.h"
static const struct partial_sample {
    const char *name, *before, *normal, *ticks; unsigned marks;
} partial_samples[] = {
    {"strong", "ab &amp; __keep__\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"italic", "ab &amp; *keep*\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; *keZ\\*\\[\\]*!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; *keZ\\`\\`*!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_ITALIC},
    {"nested", "ab &amp; __*keep*__\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __*keZ\\*\\[\\]*__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __*keZ\\`\\`*__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD | XUI_DOC_ITALIC},
    {"link", "ab &amp; [keep][r]\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; [keZ\\*\\[\\]][r]!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; [keZ\\`\\`][r]!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_LINK},
    {"code", "ab &amp; ``keep``\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; ``keZ*[]``!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; ``` keZ`` ```!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_CODE},
    {"nested-code", "ab &amp; __``keep``__\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __``keZ*[]``__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __``` keZ`` ```__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD | XUI_DOC_CODE},
    {"right-strong", "ab &amp; keep\n\n**xx!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n",
        "ab &amp; keZ\\*\\[\\]**!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n",
        "ab &amp; keZ\\`\\`**!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n", 0},
    {"mixed-formats", "ab &amp; __keep__\n\n*xx!cd* &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\*\\[\\]__*!cd* &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\`\\`__*!cd* &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"quote", "> ab &amp; __keep__\n>\n> xx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "> ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "> ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"footnote", "use[^n]\n\n[^n]: ab &amp; __keep__\n    \n    xx!cd &lt; **tail**\n    \n    [r]: <raw> 'T'\n",
        "use[^n]\n\n[^n]: ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n    \n    [r]: <raw> 'T'\n",
        "use[^n]\n\n[^n]: ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n    \n    [r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"atx-left", "## ab &amp; __keep__ ##\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "## ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail** ##\n\n[r]: <raw> 'T'\n",
        "## ab &amp; __keZ\\`\\`__!cd &lt; **tail** ##\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"setext-left", "ab &amp; __keep__\n===\n\nxx!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n===\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n===\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"atx-right", "ab &amp; __keep__\n\n### xx!cd &lt; **tail** ###\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"setext-both", "ab &amp; __keep__\n===\n\nxx!cd &lt; **tail**\n---\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\*\\[\\]__!cd &lt; **tail**\n===\n\n[r]: <raw> 'T'\n",
        "ab &amp; __keZ\\`\\`__!cd &lt; **tail**\n===\n\n[r]: <raw> 'T'\n", XUI_DOC_BOLD},
    {"hidden-quote", "> ab &amp; [keep][r]\n>\n> [r]: <raw> 'T'\n>\n> xx!cd &lt; **tail**\n",
        "> ab &amp; [keZ\\*\\[\\]][r]!cd &lt; **tail**\n>\n> [r]: <raw> 'T'\n>\n",
        "> ab &amp; [keZ\\`\\`][r]!cd &lt; **tail**\n>\n> [r]: <raw> 'T'\n>\n", XUI_DOC_LINK}
};
static void partial_cross_sources(unsigned fixture, unsigned payload, const char* internal,
    const char* final, unsigned bom, char** before, char** after)
{
    const struct partial_sample* c = partial_samples + fixture; size_t width = strlen(internal);
    *before = delete_ending(c->before, internal, bom);
    const char* desired = payload ? c->ticks : c->normal;
    char empty[4096];
    if (payload == 2) {
        snprintf(empty, sizeof(empty), "%s", c->normal);
        char* insertion = strstr(empty, "keZ"); TABLE_CHECK(insertion);
        const char* raw_payload = (c->marks & XUI_DOC_CODE) ? "Z*[]" : "Z\\*\\[\\]";
        memmove(insertion + 2, insertion + 2 + strlen(raw_payload), strlen(insertion + 2 + strlen(raw_payload)) + 1);
        if (fixture == 6) snprintf(empty, sizeof(empty), "ab &amp; k&#101;**!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n");
        desired = empty;
    }
    *after = delete_ending(desired, internal, bom);
    heading_edge_replace_ending(*before + strlen(*before) - width, width, final);
    if (fixture == 14) {
        char* boundary = strpbrk(*after, "\r\n"); TABLE_CHECK(boundary);
        heading_edge_replace_ending(boundary, width, *final ? final : "\n");
    } else heading_edge_replace_ending(*after + strlen(*after) - width, width, final);
}
static xui_doc_range_t partial_cross_range(xui_document_snapshot old, const char* before, unsigned reverse,
    xui_doc_position_t* tail)
{
    xui_doc_range_t range = {0}; int quality;
    const char *left = strstr(before, "keep"), *right = strstr(before, "!cd"); TABLE_CHECK(left && right);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(left - before) + 2, &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        xuiDocumentSourceToPosition(old, (uint64_t)(right - before), &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        xuiDocumentSourceToPosition(old, (uint64_t)(right - before) + 1, tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret.iAffinity = XUI_DOC_BEFORE;
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}
static int partial_cross_execute(xui_document d, const xui_doc_range_t* range, const char* text,
    xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(t, range, text, strlen(text), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static int partial_strict_nonempty(void)
{
    const char* payloads[] = {"Z*[]", "Z``"}; unsigned total = 0, differences = 0, rejected = 0;
    for (unsigned fixture = 0; fixture < sizeof(partial_samples) / sizeof(*partial_samples); fixture++)
    for (unsigned payload = 0; payload < 2; payload++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        const struct partial_sample* c = partial_samples + fixture; const char* after = payload ? c->ticks : c->normal;
        table_allocator allocation = {-1, 0}; xui_document d = table_open(c->before, &allocation), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_document_change_set change = NULL;
        xui_doc_range_t range = {0}; xui_doc_position_t caret, tail, mapped; xui_doc_node_info_t typed = {0}; uint64_t at, bytes; int quality, result;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        const char *left = strstr(c->before, "keep"), *right = strstr(c->before, "!cd"); TABLE_CHECK(left && right);
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(left - c->before) + 2, &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            xuiDocumentSourceToPosition(old, (uint64_t)(right - c->before), &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            xuiDocumentSourceToPosition(old, (uint64_t)(right - c->before) + 1, &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
        range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret.iAffinity = XUI_DOC_BEFORE;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        xui_document_transaction t; TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnReplaceRange(t, &range, payloads[payload], strlen(payloads[payload]), &caret);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, &change);
        if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
        xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        char raw[4096]; TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, sizeof(raw), &bytes) == XUI_OK && bytes < sizeof(raw)); raw[bytes] = 0;
        int equal = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes);
        printf("Partial cross fixture=%u name=%s payload=%u reverse=%u result=%d exact_source=%d source_hex=", fixture, c->name, payload, reverse, result, equal);
        for (uint64_t i = 0; i < bytes; i++) printf("%02x", (unsigned char)raw[i]);
        puts(""); fflush(stdout);
        if (result == XUI_OK) {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            integration_equivalent(current, raw); boundary_caret(d, current, &caret); differences += !equal;
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == c->marks);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - 2);
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); xuiDocumentChangeSetRelease(change);
        } else {
            TABLE_CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE); rejected++;
            table_source(current, c->before); table_equivalent(current, old); TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
        }
        table_source(old, c->before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d); xuiDocumentRelease(oracle); TABLE_CHECK(!allocation.live); total++;
    }
    TABLE_CHECK(total == 60);
    printf("Partial cross diagnostic: %u checks; differences=%u rejected=%u; independent source/full semantics/index/inherited styles/caret/stable tail/history/zero leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}
static int partial_strict_empty(void)
{
    const char* payloads[] = {"", ""}; unsigned total = 0, differences = 0, rejected = 0;
    for (unsigned fixture = 0; fixture < sizeof(partial_samples) / sizeof(*partial_samples); fixture++)
    for (unsigned payload = 0; payload < 1; payload++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        const struct partial_sample* c = partial_samples + fixture;
        char wanted[4096]; snprintf(wanted, sizeof(wanted), "%s", c->normal);
        char* insertion = strstr(wanted, "keZ"); TABLE_CHECK(insertion);
        const char* raw_payload = (c->marks & XUI_DOC_CODE) ? "Z*[]" : "Z\\*\\[\\]";
        memmove(insertion + 2, insertion + 2 + strlen(raw_payload), strlen(insertion + 2 + strlen(raw_payload)) + 1);
        /* The literal ke**!cd** does not open strong under delimiter flanking.
         * Encode the surviving boundary e, retaining its text and node marks. */
        if (fixture == 6) snprintf(wanted, sizeof(wanted), "ab &amp; k&#101;**!cd** &lt; __tail__\n\n[r]: <raw> 'T'\n");
        const char* after = wanted;
        table_allocator allocation = {-1, 0}; xui_document d = table_open(c->before, &allocation), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_document_change_set change = NULL;
        xui_doc_range_t range = {0}; xui_doc_position_t caret, tail, mapped; xui_doc_node_info_t typed = {0}; uint64_t at, bytes; int quality, result;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        const char *left = strstr(c->before, "keep"), *right = strstr(c->before, "!cd"); TABLE_CHECK(left && right);
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(left - c->before) + 2, &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            xuiDocumentSourceToPosition(old, (uint64_t)(right - c->before), &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            xuiDocumentSourceToPosition(old, (uint64_t)(right - c->before) + 1, &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
        range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret.iAffinity = XUI_DOC_BEFORE;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        xui_document_transaction t; TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnReplaceRange(t, &range, payloads[payload], strlen(payloads[payload]), &caret);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, &change);
        if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
        xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        char raw[4096]; TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, sizeof(raw), &bytes) == XUI_OK && bytes < sizeof(raw)); raw[bytes] = 0;
        int equal = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes);
        printf("Partial cross empty fixture=%u name=%s payload=%u reverse=%u result=%d exact_source=%d source_hex=", fixture, c->name, payload, reverse, result, equal);
        for (uint64_t i = 0; i < bytes; i++) printf("%02x", (unsigned char)raw[i]);
        puts(""); fflush(stdout);
        if (result == XUI_OK) {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            integration_equivalent(current, raw); boundary_caret(d, current, &caret); differences += !equal;
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == c->marks);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - 2);
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); xuiDocumentChangeSetRelease(change);
        } else {
            TABLE_CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE); rejected++;
            table_source(current, c->before); table_equivalent(current, old); TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
        }
        table_source(old, c->before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d); xuiDocumentRelease(oracle); TABLE_CHECK(!allocation.live); total++;
    }
    TABLE_CHECK(total == 30);
    printf("Partial cross empty diagnostic: %u checks; differences=%u rejected=%u; independent source/full semantics/index/inherited styles/caret/stable tail/history/zero leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}
static int partial_cross_goldens(void)
{
    const char* endings[] = {"\n", "\r\n", "\r", ""}, *payloads[] = {"Z*[]", "Z``", ""}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < 15; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned internal = 0; internal < 3; internal++) for (unsigned final = 0; final < 4; final++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        char *before, *after; partial_cross_sources(fixture, payload, endings[internal], endings[final], bom, &before, &after);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change;
        xui_doc_position_t tail, mapped, caret; xui_doc_node_info_t typed = {0}; uint64_t at; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        xui_doc_range_t range = partial_cross_range(old, before, reverse, &tail);
        printf("Partial cross golden fixture=%u payload=%u internal=%u final=%u bom=%u reverse=%u\n", fixture, payload, internal, final, bom, reverse); fflush(stdout);
        TABLE_CHECK(partial_cross_execute(d, &range, payloads[payload], &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
        typed.iSize = sizeof(typed);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == partial_samples[fixture].marks);
        const char* raw_payload = payload == 2 ? "" : (typed.tAttributes.iMarks & XUI_DOC_CODE) ? payloads[payload] : payload ? "Z\\`\\`" : "Z\\*\\[\\]";
        char* word = strstr(after, payload == 2 ? "ke" : "keZ");
        uint64_t caret_at;
        if (payload == 2 && fixture == 6) {
            word = strstr(after, "&#101;"); TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + strlen("&#101;");
        } else { TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + 2 + strlen(raw_payload); }
        TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
        TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - 2 &&
            xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == (uint64_t)(strstr(after, "!cd") - after) + 1);
        xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 2160); puts("Partial cross matrix: 2160 independent source/format/index/precise-caret/stable-tail/EOF-mixed-EOL/BOM/history/zero-leak goldens passed"); return 0;
}

typedef struct partial_public_fault_allocator {
    table_allocator base;
} partial_public_fault_allocator;
static void* partial_public_fault_alloc(void* user, size_t bytes)
{
    partial_public_fault_allocator* a = user;
    return table_alloc(&a->base, bytes);
}
static void partial_public_fault_free(void* user, void* p)
{ partial_public_fault_allocator* a = user; table_free(&a->base, p); }
static void partial_public_fault_changed(xui_document d, xui_document_change_set change, void* user)
{ (void)d; TABLE_CHECK(change); (*(unsigned*)user)++; }
static int partial_public_fault_attempt(xui_document d, const xui_doc_range_t* range, const char* text,
    partial_public_fault_allocator* a, long point, xui_doc_position_t* caret, xui_document_change_set* change)
{
    a->base.remaining = point;
    return partial_cross_execute(d, range, text, caret, change);
}
static int partial_public_faults(void)
{
    const char* payloads[] = {"Z*[]", "Z``", ""}; unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 15; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        char *before, *after; long point;
        partial_cross_sources(fixture, payload, "\r\n", "", 1, &before, &after);
        for (point = 0; point < 4096; point++) {
            partial_public_fault_allocator a = {0}; a.base.remaining = -1;
            xui_doc_desc_t desc = {0}; xui_document d;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = partial_public_fault_alloc; desc.onFree = partial_public_fault_free; desc.pAllocatorUser = &a;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK);
            xui_document_snapshot old, current, restored; xui_document_change_set change = NULL;
            xui_doc_position_t tail, mapped, caret; xui_doc_node_info_t typed = {0};
            uint64_t revision, token, at; unsigned notifications = 0; int quality, result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, partial_public_fault_changed, &notifications, &token) == XUI_OK);
            xui_doc_range_t range = partial_cross_range(old, before, reverse, &tail);
            revision = xuiDocumentGetRevision(d);
            result = partial_public_fault_attempt(d, &range, payloads[payload], &a, point, &caret, &change);
            a.base.remaining = -1;
            const int wanted_error = XUI_ERROR_OUT_OF_MEMORY;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == wanted_error && !change && xuiDocumentGetRevision(d) == revision &&
                    !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(partial_public_fault_attempt(d, &range, payloads[payload], &a, -1, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(change && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) &&
                xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1);
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == partial_samples[fixture].marks);
            const char* raw_payload = payload == 2 ? "" : (typed.tAttributes.iMarks & XUI_DOC_CODE) ? payloads[payload] : payload ? "Z\\`\\`" : "Z\\*\\[\\]";
            char* word = strstr(after, payload == 2 ? "ke" : "keZ");
            uint64_t caret_at;
            if (payload == 2 && fixture == 6) {
                word = strstr(after, "&#101;"); TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + strlen("&#101;");
            } else { TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + 2 + strlen(raw_payload); }
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - 2 &&
                xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                at == (uint64_t)(strstr(after, "!cd") - after) + 1);
            xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) &&
                !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
            xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point > 0 && point < 4096);
        printf("Partial cross fault fixture=%u payload=%u reverse=%u: %ld points atomic/no-ghost-notifications/poisoned-commit/same-document-retry/source/precise-caret/stable-tail/history/zero-leaks passed\n", fixture, payload, reverse, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 90);
    puts("Partial cross public faults: 90 OOM scans passed");
    return 0;
}

#endif
