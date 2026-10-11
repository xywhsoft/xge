#ifndef XUI_DOCUMENT_JOIN_HEADING_CASES_H
#define XUI_DOCUMENT_JOIN_HEADING_CASES_H
#include "test_xui/xui_document_join_source_cases.h"

static const struct heading_join_sample { const char *name, *before, *after; } heading_join_samples[] = {
    {"atx-open-left", "## ab &amp; __keep__\n\n!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n",
     "## ab &amp; __keep__!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n"},
    {"atx-closed-left", "## ab &amp; __keep__ ##\n\n!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n",
     "## ab &amp; __keep__!cd &lt; **tail** ##\n\n[unused]: <hold> 'Keep'\n"},
    {"setext-left", "ab &amp; __keep__\n===\n\n!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n",
     "ab &amp; __keep__!cd &lt; **tail**\n===\n\n[unused]: <hold> 'Keep'\n"},
    {"atx-right", "ab &amp; __keep__\n\n### !cd &lt; **tail** ###\n\n[unused]: <hold> 'Keep'\n",
     "ab &amp; __keep__!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n"},
    {"setext-right", "ab &amp; __keep__\n\n!cd &lt; **tail**\n---\n\n[unused]: <hold> 'Keep'\n",
     "ab &amp; __keep__!cd &lt; **tail**\n\n[unused]: <hold> 'Keep'\n"},
    {"atx-both", "# ab &amp; __keep__ #\n\n## !cd &lt; **tail** ##\n\n[unused]: <hold> 'Keep'\n",
     "# ab &amp; __keep__!cd &lt; **tail** #\n\n[unused]: <hold> 'Keep'\n"},
    {"loose-list", "+ ab &amp; __keep__\n  \n  !cd &lt; **tail**\n  \n  end &amp;\n",
     "+ ab &amp; __keep__!cd &lt; **tail**\n  \n  end &amp;\n"},
    {"list-quote", "+ > ab &amp; __keep__\n  >\n  > !cd &lt; **tail**\n  >\n  > end &amp;\n",
     "+ > ab &amp; __keep__!cd &lt; **tail**\n  >\n  > end &amp;\n"},
    {"tab-list", "+ ab &amp; __keep__\n\t\n\t!cd &lt; **tail**\n\t\n\tend &amp;\n",
     "+ ab &amp; __keep__!cd &lt; **tail**\n\t\n\tend &amp;\n"},
    {"quote-atx-closed", "> ## ab &amp; __keep__ ##\n>\n> !cd &lt; **tail**\n>\n> end &amp;\n",
     "> ## ab &amp; __keep__!cd &lt; **tail** ##\n>\n> end &amp;\n"}
};

static void heading_join_goldens(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(heading_join_samples) / sizeof(*heading_join_samples); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned mode = 0; mode < 5; mode++) {
        char *before = delete_ending(heading_join_samples[fixture].before, endings[eol], bom),
            *after = delete_ending(heading_join_samples[fixture].after, endings[eol], bom);
        if (eof) {
            before[strlen(before) - strlen(endings[eol])] = 0;
            after[strlen(after) - strlen(endings[eol])] = 0;
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change;
        xui_doc_position_t tail, mapped, caret; uint64_t left, right, at; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        printf("Heading join golden fixture=%u eol=%u bom=%u eof=%u mode=%u\n", fixture, eol, bom, eof, mode); fflush(stdout);
        TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
        TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
            xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            at == (uint64_t)(strstr(after, "!cd") - after) + 1);
        xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 600);
    printf("Heading join matrix: %u independent whole-source/full-semantic/index/stable-tail/caret/common-history/zero-leak goldens passed\n", total);
}
static void heading_join_faults(void)
{
    for (unsigned fixture = 0; fixture < sizeof(heading_join_samples) / sizeof(*heading_join_samples); fixture++)
    for (unsigned mode = 0; mode < 2; mode++) {
        char *before = delete_ending(heading_join_samples[fixture].before, "\r\n", 1),
            *after = delete_ending(heading_join_samples[fixture].after, "\r\n", 1); long point;
        for (point = 0; point < 4096; point++) {
            table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
            xui_document_snapshot old, current, restored; xui_doc_position_t tail, caret;
            uint64_t left, right, revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            allocation.remaining = point; result = join_execute(d, old, left, right, mode, &caret, NULL); allocation.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !xuiDocumentIsDirty(d));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, NULL) == XUI_OK); table_source(current, before);
                xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentCanUndo(d) && xuiDocumentIsDirty(d));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096); printf("Heading join source fixture=%u mode=%u: %ld OOM points atomic/poisoned commit/same-document retry/history/zero leaks passed\n", fixture, mode, point);
        free(before); free(after);
    }
}





#include "test_xui/xui_document_join_source_cases.h"

enum { HEADING_EDGE_HIDDEN = 1, HEADING_EDGE_SETEXT = 2 };
static const struct heading_edge_sample {
    const char *name, *before, *after; unsigned flags;
} heading_edge_samples[] = {
    {"atx-left", "# ab &amp; __keep__ #\n\n!cd &lt; **tail**\n",
     "# ab &amp; __keep__!cd &lt; **tail** #\n", 0},
    {"setext-left", "ab &amp; __keep__\n===\n\n!cd &lt; **tail**\n",
     "ab &amp; __keep__!cd &lt; **tail**\n===\n", 0},
    {"atx-right", "ab &amp; __keep__\n\n### !cd &lt; **tail** ###\n",
     "ab &amp; __keep__!cd &lt; **tail**\n", 0},
    {"setext-right", "ab &amp; __keep__\n\n!cd &lt; **tail**\n---\n",
     "ab &amp; __keep__!cd &lt; **tail**\n", 0},
    {"both-headings", "# ab &amp; __keep__ #\n\n## !cd &lt; **tail** ##\n",
     "# ab &amp; __keep__!cd &lt; **tail** #\n", 0},
    {"quote-atx", "> # ab &amp; __keep__ #\n>\n> !cd &lt; **tail**\n",
     "> # ab &amp; __keep__!cd &lt; **tail** #\n", 0},
    {"quote-setext", "> ab &amp; __keep__\n> ===\n>\n> !cd &lt; **tail**\n",
     "> ab &amp; __keep__!cd &lt; **tail**\n> ===\n", 0},
    {"quote-hidden-atx", "> # [ab][r] &amp; #\n>\n> [r]: <raw> 'T'\n>\n> !cd &lt; __keep__\n",
     "> # [ab][r] &amp;!cd &lt; __keep__ #\n>\n> [r]: <raw> 'T'\n>\n", HEADING_EDGE_HIDDEN},
    {"quote-hidden-setext", "> [ab][r] &amp;\n> ===\n>\n> [r]: <raw> 'T'\n>\n> !cd &lt; __keep__\n",
     "> [ab][r] &amp;!cd &lt; __keep__\n> ===\n>\n> [r]: <raw> 'T'\n>\n", HEADING_EDGE_HIDDEN | HEADING_EDGE_SETEXT},
    {"footnote-first-atx", "use[^n]\n\n[^n]: # ab &amp; __keep__ #\n    \n    !cd &lt; **tail**\n",
     "use[^n]\n\n[^n]: # ab &amp; __keep__!cd &lt; **tail** #\n", 0},
    {"footnote-setext", "use[^n]\n\n[^n]:\n    ab &amp; __keep__\n    ===\n    \n    !cd &lt; **tail**\n",
     "use[^n]\n\n[^n]:\n    ab &amp; __keep__!cd &lt; **tail**\n    ===\n", 0},
    {"list-quote-atx", "+ > # ab &amp; __keep__ #\n  >\n  > !cd &lt; **tail**\n",
     "+ > # ab &amp; __keep__!cd &lt; **tail** #\n", 0}
};

static void heading_edge_replace_ending(char* at, size_t old_bytes, const char* ending)
{
    size_t bytes = strlen(ending);
    memmove(at + bytes, at + old_bytes, strlen(at + old_bytes) + 1);
    memcpy(at, ending, bytes);
}
static void heading_edge_sources(unsigned fixture, const char* internal,
    const char* final, unsigned bom, char** before, char** after)
{
    const struct heading_edge_sample* sample = heading_edge_samples + fixture;
    *before = delete_ending(sample->before, internal, bom);
    *after = delete_ending(sample->after, internal, bom);
    size_t width = strlen(internal);
    heading_edge_replace_ending(*before + strlen(*before) - width, width, final);
    if (sample->flags & HEADING_EDGE_HIDDEN) {
        char* at = strpbrk(strstr(*after, "!cd"), "\r\n"); TABLE_CHECK(at);
        if (sample->flags & HEADING_EDGE_SETEXT) {
            at = strpbrk(at + width, "\r\n"); TABLE_CHECK(at);
        }
        /* At EOF only this separating LF is new. Every moved definition and
         * its original gap keeps its original byte spelling and endings. */
        heading_edge_replace_ending(at, width, *final ? final : "\n");
    } else heading_edge_replace_ending(*after + strlen(*after) - width, width, final);
}

static void heading_edge_goldens(void)
{
    const char* endings[] = {"\n", "\r\n", "\r", ""}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(heading_edge_samples) / sizeof(*heading_edge_samples); fixture++)
    for (unsigned internal = 0; internal < 3; internal++) for (unsigned final = 0; final < 4; final++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned mode = 0; mode < 5; mode++) {
        char *before, *after; heading_edge_sources(fixture, endings[internal], endings[final], bom, &before, &after);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change;
        xui_doc_position_t tail, mapped, caret; uint64_t left, right, at; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        printf("Heading edge golden fixture=%u name=%s internal=%u final=%u bom=%u mode=%u\n",
            fixture, heading_edge_samples[fixture].name, internal, final, bom, mode); fflush(stdout);
        TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
        TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
            xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            at == (uint64_t)(strstr(after, "!cd") - after) + 1);
        xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 1440);
    printf("Heading edge matrix: %u independent EOF/mixed-ending/BOM/forward-reverse TEXT-GAP/full-source/semantic/index/stable-tail/history goldens passed\n", total);
}

#endif
