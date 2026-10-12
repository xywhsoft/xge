#ifndef XUI_DOCUMENT_NONEMPTY_SEAM_CASES_H
#define XUI_DOCUMENT_NONEMPTY_SEAM_CASES_H
#include "tests/xui/xui_document_join_heading_cases.h"
static int heading_replace_execute(xui_document d, xui_document_snapshot old,
    uint64_t left, uint64_t right, unsigned mode, const char* text,
    xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; xui_doc_range_t range = {0};
    xui_doc_node_info_t n = {0}; uint64_t child; int result;
    n.iSize = sizeof(n); TABLE_CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK && n.iChildCount);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iAffinity = XUI_DOC_AFTER;
    range.tAnchor.iNodeId = left; range.tAnchor.iOffset = n.iChildCount; range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
    if (mode >= 2) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, left, n.iChildCount - 1, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(old, child, &n) == XUI_OK && n.iKind == XUI_DOC_TEXT);
        range.tAnchor.iNodeId = child; range.tAnchor.iOffset = n.iTextBytes; range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    }
    range.tCaret = range.tAnchor; range.tCaret.iNodeId = right; range.tCaret.iOffset = 0;
    range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    if (mode >= 2) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, right, 0, &child) == XUI_OK);
        range.tCaret.iNodeId = child; range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    }
    if (mode & 1) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    result = xuiDocumentBeginTransaction(d, NULL, &t); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(t, &range, text, strlen(text), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}


static const struct heading_replace_payload { const char *text, *raw; } heading_replace_payloads[] = {
    {"Z", "Z"}, {"中🙂", "中🙂"}, {"Z*[]", "Z\\*\\[\\]"}, {" Z ", "&#32;Z&#32;"}
};
static unsigned heading_replace_marks(unsigned fixture)
{
    return (fixture < 12 && fixture != 7 && fixture != 8) || fixture == 12 ? XUI_DOC_BOLD : 0;
}
static void heading_replace_sources(unsigned fixture, const char* internal,
    const char* final, unsigned bom, const char* raw, char** before, char** after, uint64_t* insertion)
{
    char* joined;
    if (fixture < 12) heading_edge_sources(fixture, internal, final, bom, before, &joined);
    else {
        unsigned sample = fixture - 12; size_t width = strlen(internal);
        *before = delete_ending(join_samples[sample].before, internal, bom);
        joined = delete_ending(join_samples[sample].after, internal, bom);
        heading_edge_replace_ending(*before + strlen(*before) - width, width, final);
        if (sample == 5) {
            char* boundary = strpbrk(joined, "\r\n"); TABLE_CHECK(boundary);
            heading_edge_replace_ending(boundary, width, *final ? final : "\n");
        } else heading_edge_replace_ending(joined + strlen(joined) - width, width, final);
    }
    char* junction = strstr(joined, heading_replace_marks(fixture) ? "__!cd" : "!cd"); TABLE_CHECK(junction);
    size_t at = (size_t)(junction - joined), bytes = strlen(raw); *insertion = at;
    *after = malloc(strlen(joined) + bytes + 1); TABLE_CHECK(*after);
    memcpy(*after, joined, at); memcpy(*after + at, raw, bytes); strcpy(*after + at + bytes, joined + at);
    free(joined);
}
static void heading_replace_goldens(void)
{
    const char* endings[] = {"\n", "\r\n", "\r", ""}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < 18; fixture++) for (unsigned internal = 0; internal < 3; internal++)
    for (unsigned final = 0; final < 4; final++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned mode = 0; mode < 4; mode++) for (unsigned payload = 0; payload < 4; payload++) {
        const struct heading_replace_payload* p = heading_replace_payloads + payload;
        char *before, *after; uint64_t insertion; heading_replace_sources(fixture, endings[internal], endings[final], bom, p->raw, &before, &after, &insertion);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change;
        xui_doc_position_t tail, mapped, caret; xui_doc_node_info_t typed = {0}; uint64_t left, right, at; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        printf("Nonempty seam golden fixture=%u internal=%u final=%u bom=%u mode=%u payload=%u\n", fixture, internal, final, bom, mode, payload); fflush(stdout);
        TABLE_CHECK(heading_replace_execute(d, old, left, right, mode, p->text, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
        typed.iSize = sizeof(typed);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.iKind == XUI_DOC_TEXT && typed.tAttributes.iMarks == heading_replace_marks(fixture));
        TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == insertion + strlen(p->raw));
        TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
            xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == (uint64_t)(strstr(after, "!cd") - after) + 1);
        xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 6912);
    printf("Nonempty seam matrix: %u independent raw/semantic/index/inherited-marks/exact-caret/stable-tail/common-history/zero-leak goldens passed\n", total);
}

static const struct seam_format_sample {
    const char *name, *before, *prefix, *suffix; unsigned marks;
} seam_format_samples[] = {
    {"code", "ab &amp; ``keep``\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "ab &amp; ", "", XUI_DOC_CODE},
    {"atx-code", "## ab &amp; ``keep`` ##\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "## ab &amp; ", " ##", XUI_DOC_CODE},
    {"setext-code", "ab &amp; ``keep``\n===\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "ab &amp; ", "\n===", XUI_DOC_CODE},
    {"link", "ab &amp; [keep][r]\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "ab &amp; ", "", XUI_DOC_LINK},
    {"strong-link", "ab &amp; __[keep][r]__\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "ab &amp; ", "", XUI_DOC_BOLD | XUI_DOC_LINK},
    {"strong-code", "ab &amp; __``keep``__\n\n!cd &lt; __tail__\n\n[r]: <raw> 'T'\n", "ab &amp; ", "", XUI_DOC_BOLD | XUI_DOC_CODE}
};
static int nonempty_format_goldens(void)
{
    const char* payloads[] = {"Z", "中🙂", "Z*[]", " Z ", "Z``"};
    const char* escaped[] = {"Z", "中🙂", "Z\\*\\[\\]", "&#32;Z&#32;", "Z\\`\\`"};
    unsigned total = 0, differences = 0, rejected = 0;
    for (unsigned fixture = 0; fixture < 6; fixture++) for (unsigned payload = 0; payload < 5; payload++)
    for (unsigned mode = 0; mode < 4; mode++) {
        const struct seam_format_sample* c = seam_format_samples + fixture;
        char wanted[4096], word[2048], actual[4096]; int code = !!(c->marks & XUI_DOC_CODE), bold = !!(c->marks & XUI_DOC_BOLD);
        if (code) snprintf(word, sizeof(word), payload == 4 ? "``` keep%s ```" : "``keep%s``", payloads[payload]);
        else snprintf(word, sizeof(word), "[keep%s][r]", escaped[payload]);
        if (fixture == 2)
            snprintf(wanted, sizeof(wanted), "%s%s%s!cd &lt; __tail__%s\n\n[r]: <raw> 'T'\n", c->prefix, bold ? "__" : "", word, c->suffix);
        else snprintf(wanted, sizeof(wanted), "%s%s%s%s!cd &lt; __tail__%s\n\n[r]: <raw> 'T'\n", c->prefix, bold ? "__" : "", word, bold ? "__" : "", c->suffix);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(c->before, &allocation), oracle = table_open(wanted, NULL);
        xui_document_snapshot old, current, expected, restored; xui_doc_position_t tail, caret;
        xui_doc_node_info_t typed = {0}; uint64_t left, right, bytes; int result;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        join_positions(old, c->before, &left, &right, &tail);
        result = heading_replace_execute(d, old, left, right, mode, payloads[payload], &caret, NULL);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK && xuiDocumentSnapshotCopySource(current, actual, sizeof(actual), &bytes) == XUI_OK && bytes < sizeof(actual)); actual[bytes] = 0;
        int equal = bytes == strlen(wanted) && !memcmp(actual, wanted, (size_t)bytes);
        printf("Seam format fixture=%u name=%s payload=%u mode=%u result=%d exact_source=%d source_hex=", fixture, c->name, payload, mode, result, equal);
        for (uint64_t i = 0; i < bytes; i++) printf("%02x", (unsigned char)actual[i]);
        puts(""); fflush(stdout);
        if (result == XUI_OK) {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            integration_equivalent(current, actual); boundary_caret(d, current, &caret); differences += !equal;
            typed.iSize = sizeof(typed); TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == c->marks);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, actual); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        } else {
            TABLE_CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE); rejected++; table_source(current, c->before); table_equivalent(current, old);
            TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
        }
        table_source(old, c->before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d); xuiDocumentRelease(oracle); TABLE_CHECK(!allocation.live); total++;
    }
    TABLE_CHECK(total == 120);
    printf("Seam format diagnostic: %u checks; differences=%u rejected=%u; independent styles/link-code/whole semantics/reparse/history/zero leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}
static void seam_fault_changed(xui_document d, xui_document_change_set change, void* user)
{ (void)d; TABLE_CHECK(change); (*(unsigned*)user)++; }
static xui_doc_range_t seam_fault_range(xui_document d, xui_document_snapshot old,
    uint64_t left, uint64_t right, unsigned mode)
{
    xui_doc_range_t range = {0}; xui_doc_node_info_t n = {0}; uint64_t child;
    n.iSize = sizeof(n); TABLE_CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK && n.iChildCount);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iAffinity = XUI_DOC_AFTER;
    range.tAnchor.iNodeId = left; range.tAnchor.iOffset = n.iChildCount;
    range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
    if (mode >= 2) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, left, n.iChildCount - 1, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(old, child, &n) == XUI_OK && n.iKind == XUI_DOC_TEXT);
        range.tAnchor.iNodeId = child; range.tAnchor.iOffset = n.iTextBytes;
        range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    }
    range.tCaret = range.tAnchor; range.tCaret.iNodeId = right;
    range.tCaret.iOffset = 0; range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    if (mode >= 2) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, right, 0, &child) == XUI_OK);
        range.tCaret.iNodeId = child; range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    }
    if (mode & 1) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}
static void seam_fault_source(unsigned fixture, char** before, char** after,
    const char** payload, uint64_t* caret_at, unsigned* marks)
{
    if (fixture < 18) {
        uint64_t insertion;
        *payload = "Z*[]"; *marks = heading_replace_marks(fixture);
        heading_replace_sources(fixture, "\r\n", "", 1, "Z\\*\\[\\]", before, after, &insertion);
        *caret_at = insertion + strlen("Z\\*\\[\\]");
    } else {
        unsigned format = fixture - 18; const struct seam_format_sample* c = seam_format_samples + format;
        char wanted[4096], word[2048]; int code = !!(c->marks & XUI_DOC_CODE), bold = !!(c->marks & XUI_DOC_BOLD);
        *payload = code ? "Z``" : "Z*[]"; *marks = c->marks;
        if (code) snprintf(word, sizeof(word), "``` keep%s ```", *payload);
        else snprintf(word, sizeof(word), "[keepZ\\*\\[\\]][r]");
        if (format == 2)
            snprintf(wanted, sizeof(wanted), "%s%s%s!cd &lt; __tail__%s\n\n[r]: <raw> 'T'\n", c->prefix, bold ? "__" : "", word, c->suffix);
        else snprintf(wanted, sizeof(wanted), "%s%s%s%s!cd &lt; __tail__%s\n\n[r]: <raw> 'T'\n", c->prefix, bold ? "__" : "", word, bold ? "__" : "", c->suffix);
        *before = delete_ending(c->before, "\r\n", 1); *after = delete_ending(wanted, "\r\n", 1);
        char* keep = strstr(*after, "keep"); TABLE_CHECK(keep);
        *caret_at = (uint64_t)(keep - *after) + 4 + strlen(code ? "Z``" : "Z\\*\\[\\]");
    }
}
static int nonempty_public_faults(void)
{
    unsigned scans = 0; const unsigned cancel = 0;
    for (unsigned fixture = 0; fixture < 24; fixture++)
    for (unsigned endpoint = 0; endpoint < 2; endpoint++) {
        unsigned mode = endpoint ? 3 : 0, marks; const char* text;
        char *before, *after; uint64_t caret_at; long point;
        seam_fault_source(fixture, &before, &after, &text, &caret_at, &marks);
        for (point = 0; point < 4096; point++) {
            table_allocator allocation = {-1, 0};
            xui_document d = table_open(before, &allocation); xui_document_snapshot old, current, restored;
            xui_doc_position_t tail, caret, mapped; xui_document_change_set change = NULL;
            xui_doc_node_info_t typed = {0}; uint64_t left, right, revision, token, at;
            unsigned notifications = 0; int result, quality;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, seam_fault_changed, &notifications, &token) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            allocation.remaining = point;
            result = heading_replace_execute(d, old, left, right, mode, text, &caret, &change);
            allocation.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) && !change &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                    !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(heading_replace_execute(d, old, left, right, mode, text, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) &&
                xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1 && change);
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == marks);
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
                xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                at == (uint64_t)(strstr(after, "!cd") - after) + 1);
            xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentUnsubscribe(d, token);
            xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Nonempty seam public fault fixture=%u cancel=%u mode=%u: %ld points atomic/no-ghost-notifications/poisoned-commit/same-document-retry/source/caret/stable-tail/history/zero-leaks passed\n", fixture, cancel, mode, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 48); puts("Nonempty seam public faults: 48 OOM scans passed"); return 0;
}
static int nonempty_continuations(void)
{
    unsigned checks = 0;
    for (unsigned fixture = 0; fixture < 24; fixture++) for (unsigned mode = 0; mode < 4; mode++)
    for (unsigned compound = 0; compound < 2; compound++) {
        char *before, *joined, *continued; const char* text; unsigned marks; uint64_t insertion;
        seam_fault_source(fixture, &before, &joined, &text, &insertion, &marks);
        continued = malloc(strlen(joined) + 2); TABLE_CHECK(continued);
        memcpy(continued, joined, (size_t)insertion); continued[insertion] = 'Q';
        strcpy(continued + insertion + 1, joined + insertion);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, baseline = NULL, current, restored;
        xui_document_transaction t; xui_doc_position_t tail, caret; uint64_t left, right, at;
        xui_doc_node_info_t typed = {0}; unsigned notifications = 0; uint64_t token; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, seam_fault_changed, &notifications, &token) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        xui_doc_range_t range = seam_fault_range(d, old, left, right, mode);
        TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, text, strlen(text), &caret) == XUI_OK);
        if (!compound) {
            TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && notifications == 1);
            xuiDocumentTxnRelease(t); caret.iRevision = xuiDocumentGetRevision(d);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &baseline) == XUI_OK); table_source(baseline, joined);
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        }
        range.tAnchor = range.tCaret = caret;
        printf("Nonempty continuation fixture=%u mode=%u compound=%u\n", fixture, mode, compound); fflush(stdout);
        TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        TABLE_CHECK(notifications == (compound ? 1u : 2u) && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, continued); integration_equivalent(current, continued); boundary_caret(d, current, &caret);
        typed.iSize = sizeof(typed);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == marks &&
            xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == insertion + 1);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, compound ? before : joined); table_equivalent(restored, compound ? old : baseline);
        xuiDocumentSnapshotRelease(restored);
        if (!compound) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        if (!compound) {
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); table_source(restored, joined);
            table_equivalent(restored, baseline); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        }
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, continued); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
        if (baseline) { table_source(baseline, joined); xuiDocumentSnapshotRelease(baseline); }
        xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(joined); free(continued); checks++;
    }
    TABLE_CHECK(checks == 192); puts("Nonempty continuation: 192 same-draft/post-publication goldens passed"); return 0;
}

#endif
