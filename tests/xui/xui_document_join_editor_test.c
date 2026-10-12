#include "tests/xui/xui_document_partial_entity_cases.h"
#include "tests/xui/xui_document_partial_cross_cases.h"
#include "tests/xui/xui_document_nonempty_seam_cases.h"
#include "tests/xui/xui_document_join_heading_cases.h"
#include "xui_document_ui.h"
#include "tests/xui/xui_test_proxy.h"
#include "tests/xui/xui_document_join_source_cases.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "Join Editor line %d: %s\n", __LINE__, #c); exit(2); } } while (0)

static int join_editor_source(xui_document d, const char* expected)
{
    xui_document_snapshot s; char raw[4096]; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotCopySource(s, raw, sizeof(raw), &bytes) == XUI_OK);
    xuiDocumentSnapshotRelease(s); return bytes == strlen(expected) && !memcmp(raw, expected, (size_t)bytes);
}
static uint64_t join_editor_text_bytes(xui_document_snapshot s, uint64_t block, uint64_t children)
{
    uint64_t total = 0, child; xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
    for (uint64_t i = 0; i < children; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, block, i, &child) == XUI_OK && xuiDocumentSnapshotGetNode(s, child, &n) == XUI_OK && n.iKind == XUI_DOC_TEXT);
        total += n.iTextBytes;
    }
    return total;
}
static int join_editor_original(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; unsigned total = 0, differences = 0, rejected = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    const char* endings[] = {"\n", "\r\n", "\r"};
    for (unsigned fixture = 0; fixture < 6; fixture++) for (unsigned eol = 0; eol < 3; eol++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned eof = 0; eof < 2; eof++) for (unsigned action = 0; action < 3; action++) {
        char *before = delete_ending(join_samples[fixture].before, endings[eol], bom),
            *after = delete_ending(join_samples[fixture].after, endings[eol], bom);
        if (eof) {
            before[strlen(before) - strlen(endings[eol])] = 0;
            if (fixture == 5) {
                char* boundary = strstr(after, endings[eol]); CHECK(boundary); size_t width = strlen(endings[eol]);
                memmove(boundary + 1, boundary + width, strlen(boundary + width) + 1); *boundary = '\n';
            } else after[strlen(after) - strlen(endings[eol])] = 0;
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation); xui_widget editor;
        xui_document_snapshot old, current, restored; xui_doc_position_t tail, first, last;
        xui_doc_range_t range, selected; xui_doc_node_info_t n = {0}; xui_doc_editor_desc_t desc = {0};
        uint64_t left, right, child, left_children, junction, revision; int result, quality;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail); n.iSize = sizeof(n);
        CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK && n.iChildCount); left_children = n.iChildCount;
        junction = join_editor_text_bytes(old, left, left_children);
        CHECK(xuiDocumentSnapshotGetChild(old, left, left_children - 1, &child) == XUI_OK && xuiDocumentSnapshotGetNode(old, child, &n) == XUI_OK);
        last = tail; last.iNodeId = child; last.iOffset = n.iTextBytes; last.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentSnapshotGetChild(old, right, 0, &child) == XUI_OK);
        first = tail; first.iNodeId = child; first.iOffset = 0; first.iAffinity = XUI_DOC_BEFORE;
        if (action == 0) range.tAnchor = range.tCaret = first;
        else if (action == 1) range.tAnchor = range.tCaret = last;
        else { range.tAnchor = last; range.tCaret = first; }
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        result = xuiDocumentEditorExecute(editor, action == 0 ? XUI_DOC_EDIT_BACKSPACE : XUI_DOC_EDIT_DELETE);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        printf("Join Editor fixture=%u eol=%u bom=%u eof=%u action=%u result=%d\n", fixture, eol, bom, eof, action, result); fflush(stdout);
        if (result != XUI_OK) {
            CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision &&
                join_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); rejected++;
        } else {
            CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            int equal = join_editor_source(d, after); differences += !equal;
            if (equal) integration_equivalent(current, after);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
                selected.tAnchor.iNodeId == selected.tCaret.iNodeId && selected.tAnchor.iOffset == selected.tCaret.iOffset &&
                xuiDocumentSnapshotGetNode(current, selected.tCaret.iNodeId, &n) == XUI_OK && n.iParentId == left && n.iKind == XUI_DOC_TEXT);
            uint64_t index = 0, item; int found = 0;
            xui_doc_node_info_t block = {0}; block.iSize = sizeof(block); CHECK(xuiDocumentSnapshotGetNode(current, left, &block) == XUI_OK);
            for (; index < block.iChildCount; index++) {
                CHECK(xuiDocumentSnapshotGetChild(current, left, index, &item) == XUI_OK);
                if (item == selected.tCaret.iNodeId) { found = 1; break; }
            }
            CHECK(found && join_editor_text_bytes(current, left, index) + selected.tCaret.iOffset == junction &&
                xuiDocumentPositionToSource(current, &selected.tCaret, &item, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && join_editor_source(d, before) &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            if (equal) CHECK(join_editor_source(d, after));
            for (unsigned mode = 0; mode < 3; mode++) {
                unsigned modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
                CHECK(xuiDocumentViewSetMode(editor, modes[mode]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
                if (equal) CHECK(join_editor_source(d, after));
            }
            xuiDocumentSnapshotRelease(current);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    CHECK(total == 216); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Join Editor strict: %u checks; %u mismatches; %u rejected; semantic junction/source position/one-history/modes/old-snapshot/zero-leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}

static int join_editor_heading(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; unsigned total = 0, differences = 0, rejected = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    const char* endings[] = {"\n", "\r\n", "\r", ""};
    for (unsigned fixture = 0; fixture < sizeof(heading_edge_samples) / sizeof(*heading_edge_samples); fixture++)
    for (unsigned internal = 0; internal < 3; internal++) for (unsigned final = 0; final < 4; final++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned action = 0; action < 3; action++) {
        char *before, *after; heading_edge_sources(fixture, endings[internal], endings[final], bom, &before, &after);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation); xui_widget editor;
        xui_document_snapshot old, current, restored; xui_doc_position_t tail, first, last;
        xui_doc_range_t range, selected; xui_doc_node_info_t n = {0}; xui_doc_editor_desc_t desc = {0};
        uint64_t left, right, child, left_children, junction, revision; int result, quality;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail); n.iSize = sizeof(n);
        CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK && n.iChildCount); left_children = n.iChildCount;
        junction = join_editor_text_bytes(old, left, left_children);
        CHECK(xuiDocumentSnapshotGetChild(old, left, left_children - 1, &child) == XUI_OK && xuiDocumentSnapshotGetNode(old, child, &n) == XUI_OK);
        last = tail; last.iNodeId = child; last.iOffset = n.iTextBytes; last.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentSnapshotGetChild(old, right, 0, &child) == XUI_OK);
        first = tail; first.iNodeId = child; first.iOffset = 0; first.iAffinity = XUI_DOC_BEFORE;
        if (action == 0) range.tAnchor = range.tCaret = first;
        else if (action == 1) range.tAnchor = range.tCaret = last;
        else { range.tAnchor = last; range.tCaret = first; }
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        result = xuiDocumentEditorExecute(editor, action == 0 ? XUI_DOC_EDIT_BACKSPACE : XUI_DOC_EDIT_DELETE);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        printf("Heading Join Editor fixture=%u internal=%u final=%u bom=%u action=%u result=%d\n", fixture, internal, final, bom, action, result); fflush(stdout);
        if (result != XUI_OK) {
            CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision &&
                join_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); rejected++;
        } else {
            CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            int equal = join_editor_source(d, after); differences += !equal;
            if (equal) integration_equivalent(current, after);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
                selected.tAnchor.iNodeId == selected.tCaret.iNodeId && selected.tAnchor.iOffset == selected.tCaret.iOffset &&
                xuiDocumentSnapshotGetNode(current, selected.tCaret.iNodeId, &n) == XUI_OK && n.iParentId == left && n.iKind == XUI_DOC_TEXT);
            uint64_t index = 0, item; int found = 0;
            xui_doc_node_info_t block = {0}; block.iSize = sizeof(block); CHECK(xuiDocumentSnapshotGetNode(current, left, &block) == XUI_OK);
            for (; index < block.iChildCount; index++) {
                CHECK(xuiDocumentSnapshotGetChild(current, left, index, &item) == XUI_OK);
                if (item == selected.tCaret.iNodeId) { found = 1; break; }
            }
            CHECK(found && join_editor_text_bytes(current, left, index) + selected.tCaret.iOffset == junction &&
                xuiDocumentPositionToSource(current, &selected.tCaret, &item, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && join_editor_source(d, before) &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            if (equal) CHECK(join_editor_source(d, after));
            for (unsigned mode = 0; mode < 3; mode++) {
                unsigned modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
                CHECK(xuiDocumentViewSetMode(editor, modes[mode]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
                if (equal) CHECK(join_editor_source(d, after));
            }
            xuiDocumentSnapshotRelease(current);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    CHECK(total == 864); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Heading Join Editor strict: %u checks; %u mismatches; %u rejected; semantic junction/source position/one-history/modes/old-snapshot/zero-leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}

static int join_editor_nonempty(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned total = 0, differences = 0, rejected = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 0; fixture < 24; fixture++) for (unsigned mode = 0; mode < 4; mode++)
    for (unsigned edit_mode = 0; edit_mode < 3; edit_mode++) {
        char *before, *after; const char* text; uint64_t caret_at; unsigned marks;
        seam_fault_source(fixture, &before, &after, &text, &caret_at, &marks);
        uint64_t source_start = 0, source_end = 0;
        if (edit_mode) {
            const char* left_text = strstr(before, "ab"); CHECK(left_text);
            const char* ending = strpbrk(left_text, "\r\n"), *right_text = strstr(before, "!cd");
            CHECK(ending && right_text && ending < right_text);
            source_start = (uint64_t)(ending - before); source_end = (uint64_t)(right_text - before);
            size_t input_bytes = strlen(text), tail_bytes = strlen(before + source_end);
            free(after); after = malloc((size_t)source_start + input_bytes + tail_bytes + 1); CHECK(after);
            memcpy(after, before, (size_t)source_start); memcpy(after + source_start, text, input_bytes);
            strcpy(after + source_start + input_bytes, before + source_end);
            caret_at = source_start + input_bytes;
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation); xui_widget editor;
        xui_document oracle = table_open(after, NULL); xui_document_snapshot old, current, expected, restored;
        xui_doc_position_t tail; xui_doc_range_t range, selected; xui_doc_editor_desc_t desc = {0};
        xui_doc_node_info_t typed = {0}; uint64_t left, right, revision, at; int quality, result;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        join_positions(old, before, &left, &right, &tail); range = seam_fault_range(d, old, left, right, mode);
        if (edit_mode) {
            memset(&range, 0, sizeof(range)); range.tAnchor.iSize = sizeof(range.tAnchor);
            range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = XUI_DOCUMENT_ROOT; range.tAnchor.iKind = XUI_DOC_POSITION_SOURCE; range.tAnchor.iOffset = source_start;
            range.tAnchor.iAffinity = mode >= 2 ? XUI_DOC_BEFORE : XUI_DOC_AFTER;
            range.tCaret = range.tAnchor; range.tCaret.iOffset = source_end;
            if (mode & 1) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        }
        const unsigned input_modes[] = {XUI_DOC_VISUAL, XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN};
        desc.iSize = sizeof(desc); desc.iMode = input_modes[edit_mode];
        desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        xui_event_t event = {0}; event.iSize = sizeof(event); event.iType = XUI_EVENT_TEXT; event.pTarget = editor;
        strcpy(event.sText, text); event.iTextSize = (int)strlen(text);
        result = xuiDispatchEvent(context, &event); CHECK(result == XUI_OK);
        result = xuiDocumentEditorFlush(editor); CHECK(xuiUpdate(context, .016f) == XUI_OK);
        printf("Nonempty Join Editor fixture=%u selection=%u edit_mode=%u result=%d\n", fixture, mode, edit_mode, result); fflush(stdout);
        if (result != XUI_OK) {
            CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision &&
                join_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); rejected++;
        } else {
            CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            int equal = join_editor_source(d, after); differences += !equal;
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            if (equal) integration_equivalent(current, after);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
                selected.tAnchor.iNodeId == selected.tCaret.iNodeId && selected.tAnchor.iOffset == selected.tCaret.iOffset);
            typed.iSize = sizeof(typed);
            if (!edit_mode) CHECK(xuiDocumentSnapshotGetNode(current, selected.tCaret.iNodeId, &typed) == XUI_OK &&
                typed.iKind == XUI_DOC_TEXT && typed.iParentId == left && typed.tAttributes.iMarks == marks);
            else CHECK(selected.tCaret.iKind == XUI_DOC_POSITION_SOURCE && selected.tCaret.iOffset == caret_at);
            CHECK(xuiDocumentPositionToSource(current, &selected.tCaret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            if (equal) CHECK(at == caret_at);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && join_editor_source(d, before) &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            if (equal) CHECK(join_editor_source(d, after));
            const unsigned modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
            for (unsigned i = 0; i < 3; i++) {
                CHECK(xuiDocumentViewSetMode(editor, modes[i]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
                if (equal) CHECK(join_editor_source(d, after));
            }
            xuiDocumentSnapshotRelease(current);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); xuiDocumentRelease(oracle); CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    CHECK(total == 288); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Nonempty Join Editor strict: %u checks; %u mismatches; %u rejected; independent VISUAL semantic andSOURCE-LIVE raw goldens/full-reparse/inherited formats/exact caret/one-history/modes/old-snapshot/zero-leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}

static void partial_editor_error(xui_widget w, int result, void* user)
{ (void)w; *(int*)user = result; }
static int join_editor_partial_cross(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned total = 0, differences = 0, rejected = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    const char* payloads[] = {"Z*[]", "Z``", ""};
    const unsigned input_modes[] = {XUI_DOC_VISUAL, XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN};
    for (unsigned fixture = 0; fixture < 15; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned reverse = 0; reverse < 2; reverse++) for (unsigned edit_mode = 0; edit_mode < 3; edit_mode++) {
        char *before, *after; const char* text = payloads[payload];
        partial_cross_sources(fixture, payload, "\r\n", "", 1, &before, &after);
        uint64_t source_start = (uint64_t)(strstr(before, "keep") - before) + 2,
            source_end = (uint64_t)(strstr(before, "!cd") - before), caret_at;
        if (edit_mode) {
            size_t bytes = strlen(text), tail_bytes = strlen(before + source_end);
            free(after); after = malloc((size_t)source_start + bytes + tail_bytes + 1); CHECK(after);
            memcpy(after, before, (size_t)source_start); memcpy(after + source_start, text, bytes);
            strcpy(after + source_start + bytes, before + source_end); caret_at = source_start + bytes;
        } else {
            const char* word = strstr(after, payload == 2 ? "ke" : "keZ");
            const char* raw_payload = payload == 2 ? "" : (partial_samples[fixture].marks & XUI_DOC_CODE) ? text : payload ? "Z\\`\\`" : "Z\\*\\[\\]";
            if (payload == 2 && fixture == 6) {
                word = strstr(after, "&#101;"); CHECK(word); caret_at = (uint64_t)(word - after) + strlen("&#101;");
            } else { CHECK(word); caret_at = (uint64_t)(word - after) + 2 + strlen(raw_payload); }
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_widget editor;
        xui_doc_position_t tail; xui_doc_range_t range, selected; xui_doc_editor_desc_t desc = {0};
        xui_doc_node_info_t typed = {0}; uint64_t revision, at; int quality, result, event_error = XUI_OK;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range = partial_cross_range(old, before, reverse, &tail);
        if (edit_mode) {
            memset(&range, 0, sizeof(range)); range.tAnchor.iSize = sizeof(range.tAnchor);
            range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = XUI_DOCUMENT_ROOT; range.tAnchor.iKind = XUI_DOC_POSITION_SOURCE; range.tAnchor.iOffset = source_start;
            range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret = range.tAnchor; range.tCaret.iOffset = source_end;
            if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        }
        desc.onError = partial_editor_error; desc.pUser = &event_error;
        desc.iSize = sizeof(desc); desc.iMode = input_modes[edit_mode]; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        if (payload == 2) result = xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE);
        else {
            xui_event_t event = {0}; event.iSize = sizeof(event); event.iType = XUI_EVENT_TEXT; event.pTarget = editor;
            strcpy(event.sText, text); event.iTextSize = (int)strlen(text); result = xuiDispatchEvent(context, &event);
        }
        if (result == XUI_OK && event_error < 0) result = event_error;
        if (result == XUI_OK) result = xuiDocumentEditorFlush(editor);
        if (result == XUI_OK && event_error < 0) result = event_error;
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        printf("Partial cross Editor fixture=%u payload=%u reverse=%u edit_mode=%u result=%d\n", fixture, payload, reverse, edit_mode, result); fflush(stdout);
        if (result != XUI_OK) {
            CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision &&
                join_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); rejected++;
        } else {
            CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            int equal = join_editor_source(d, after); differences += !equal;
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            if (equal) integration_equivalent(current, after);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK && selected.tAnchor.iNodeId == selected.tCaret.iNodeId && selected.tAnchor.iOffset == selected.tCaret.iOffset);
            if (!edit_mode) {
                typed.iSize = sizeof(typed);
                CHECK(xuiDocumentSnapshotGetNode(current, selected.tCaret.iNodeId, &typed) == XUI_OK &&
                    typed.iKind == XUI_DOC_TEXT && typed.tAttributes.iMarks == partial_samples[fixture].marks);
            } else CHECK(selected.tCaret.iKind == XUI_DOC_POSITION_SOURCE && selected.tCaret.iOffset == caret_at);
            CHECK(xuiDocumentPositionToSource(current, &selected.tCaret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            if (equal) CHECK(at == caret_at);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && join_editor_source(d, before) &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            if (equal) CHECK(join_editor_source(d, after));
            for (unsigned i = 0; i < 3; i++) {
                CHECK(xuiDocumentViewSetMode(editor, input_modes[i]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
                if (equal) CHECK(join_editor_source(d, after));
            }
            xuiDocumentSnapshotRelease(current);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); xuiDocumentRelease(oracle); CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    CHECK(total == 270); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Partial cross Editor strict: %u checks; %u mismatches; %u rejected; VISUAL semantic/SOURCE-LIVE raw oracles/real Editor TEXT-DELETE/full-reparse/exact caret/common-history/modes/old-snapshot/zero-leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}

static void entity_editor_error(xui_widget w, int result, void* user)
{ (void)w; *(int*)user = result; }
static int join_editor_partial_entity(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned total = 0, differences = 0, rejected = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    const char* payloads[] = {"Z*[]", "Z``", ""};
    const unsigned input_modes[] = {XUI_DOC_VISUAL, XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN};
    for (unsigned fixture = 0; fixture < 9; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned reverse = 0; reverse < 2; reverse++) for (unsigned edit_mode = 0; edit_mode < 3; edit_mode++) {
        char *before, *after; const char* text = payloads[payload];
        struct entity_sample sample; entity_sources(fixture, payload, "\r\n", "", 1, &sample, &before, &after);
        const char *first_entity = strstr(before, "&NotEqualTilde;"), *last_entity = first_entity, *next_entity;
        CHECK(first_entity);
        while ((next_entity = strstr(last_entity + 1, "&NotEqualTilde;")) != NULL) last_entity = next_entity;
        uint64_t source_start = sample.left_entity ? (uint64_t)(first_entity - before) + 5 : (uint64_t)(strstr(before, "keep") - before) + 2,
            source_end = sample.right_entity ? (uint64_t)(last_entity - before) + 5 : (uint64_t)(strstr(before, "!cd") - before), caret_at;
        if (edit_mode) {
            size_t bytes = strlen(text), tail_bytes = strlen(before + source_end);
            free(after); after = malloc((size_t)source_start + bytes + tail_bytes + 1); CHECK(after);
            memcpy(after, before, (size_t)source_start); memcpy(after + source_start, text, bytes);
            strcpy(after + source_start + bytes, before + source_end); caret_at = source_start + bytes;
        } else {
            const char* word = strstr(after, sample.left_entity ? "ke&#8770;" : "ke");
            const char* raw_payload = payload == 2 ? "" : payload ? "Z\\`\\`" : "Z\\*\\[\\]";
            CHECK(word); caret_at = (uint64_t)(word - after) + strlen(sample.left_entity ? "ke&#8770;" : "ke") + strlen(raw_payload);
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_widget editor;
        xui_doc_position_t tail; xui_doc_range_t range, selected; xui_doc_editor_desc_t desc = {0};
        xui_doc_node_info_t typed = {0}; uint64_t revision, at; int quality, result, event_error = XUI_OK;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range = entity_range(d, old, &sample, reverse, &tail);
        if (edit_mode) {
            memset(&range, 0, sizeof(range)); range.tAnchor.iSize = sizeof(range.tAnchor);
            range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = XUI_DOCUMENT_ROOT; range.tAnchor.iKind = XUI_DOC_POSITION_SOURCE; range.tAnchor.iOffset = source_start;
            range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret = range.tAnchor; range.tCaret.iOffset = source_end;
            if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        }
        desc.onError = entity_editor_error; desc.pUser = &event_error;
        desc.iSize = sizeof(desc); desc.iMode = input_modes[edit_mode]; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        if (payload == 2) result = xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE);
        else {
            xui_event_t event = {0}; event.iSize = sizeof(event); event.iType = XUI_EVENT_TEXT; event.pTarget = editor;
            strcpy(event.sText, text); event.iTextSize = (int)strlen(text); result = xuiDispatchEvent(context, &event);
        }
        if (result == XUI_OK && event_error < 0) result = event_error;
        if (result == XUI_OK) result = xuiDocumentEditorFlush(editor);
        if (result == XUI_OK && event_error < 0) result = event_error;
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        printf("Partial entity Editor fixture=%u payload=%u reverse=%u edit_mode=%u result=%d\n", fixture, payload, reverse, edit_mode, result); fflush(stdout);
        if (result != XUI_OK) {
            CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision &&
                join_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); rejected++;
        } else {
            CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            int equal = join_editor_source(d, after); differences += !equal;
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            if (equal) integration_equivalent(current, after);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK && selected.tAnchor.iNodeId == selected.tCaret.iNodeId && selected.tAnchor.iOffset == selected.tCaret.iOffset);
            if (!edit_mode) {
                typed.iSize = sizeof(typed);
                CHECK(xuiDocumentSnapshotGetNode(current, selected.tCaret.iNodeId, &typed) == XUI_OK &&
                    typed.iKind == XUI_DOC_TEXT && typed.tAttributes.iMarks == sample.marks);
            } else CHECK(selected.tCaret.iKind == XUI_DOC_POSITION_SOURCE && selected.tCaret.iOffset == caret_at);
            CHECK(xuiDocumentPositionToSource(current, &selected.tCaret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
            if (equal) CHECK(at == caret_at);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && join_editor_source(d, before) &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            if (equal) CHECK(join_editor_source(d, after));
            for (unsigned i = 0; i < 3; i++) {
                CHECK(xuiDocumentViewSetMode(editor, input_modes[i]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
                if (equal) CHECK(join_editor_source(d, after));
            }
            xuiDocumentSnapshotRelease(current);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); xuiDocumentRelease(oracle); CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    CHECK(total == 162); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Partial entity Editor strict: %u checks; %u mismatches; %u rejected; VISUAL semantic/SOURCE-LIVE raw oracles/real Editor TEXT-DELETE/full-reparse/exact caret/common-history/modes/old-snapshot/zero-leaks verified\n", total, differences, rejected);
    return differences || rejected ? 1 : 0;
}

int main(void) { int result = join_editor_original(); if (result) return result; result = join_editor_heading(); if (result) return result; result = join_editor_nonempty(); if (result) return result; result = join_editor_partial_cross(); if (result) return result; return join_editor_partial_entity(); }
