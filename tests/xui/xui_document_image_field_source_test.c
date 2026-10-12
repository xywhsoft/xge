#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct field_fixture { const char* source; const char* alt; } field_fixture;
static const field_fixture fixtures[] = {
    {"![foo __bar__ &amp; \\!](old 'title')\n", "foo bar & !"},
    {"![`a  b`](old)\n", "a  b"},
    {"![foo ![bar](in)](old)\n", "foo bar"},
    {"![![bar](in)](old)\n", "bar"},
    {"![![](in)](old)\n", ""},
    {"[![foo ![bar](in)](old)](target \"tip\")\n", "foo bar"},
    {"[before ![foo __bar__](old) after](target \"tip\")\n", "foo bar"},
    {"![foo\n bar](old)\n", "foo bar"},
    {"![foo ![`bar\n baz`](in)](old)\n", "foo bar  baz"},
    {"![foo ![bar][inner]][outer]\n\n[inner]: in\n[outer]: old 'title'\n", "foo bar"},
    {"![foo <i title=\"x\">bar</i>](old)\n", "foo <i title=\"x\">bar</i>"},
    {"![foo \\[bar\\] \\*](old)\n", "foo [bar] *"},
    {"![](old)\n", ""},
    {"![foo]()\n", "foo"},
    {"![foo  \n bar](old)\n", "foo bar"},
    {"![foo\\\n bar](old)\n", "foo bar"},
    {"![foo]\n\n[foo]: old 'title'\n", "foo"},
    {"![foo][]\n\n[foo]: old 'title'\n", "foo"},
    {"![foo][UP]\n\n[up]: old 'title'\n", "foo"}
};
typedef struct field_case { char source[4096]; uint64_t size; unsigned fixture; } field_case;
static void field_build(field_case* c, unsigned fixture, unsigned eol, unsigned container, int native)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    const char* head = native ? "\xef\xbb\xbf---\rname: field\r\n...\n\nprior `span`\n\n" : "prior `span`\n\n";
    const char *p = fixtures[fixture].source, *line = p;
    size_t used = strlen(head); unsigned index = 0;
    memset(c, 0, sizeof(*c)); memcpy(c->source, head, used); c->fixture = fixture;
    while (*p) {
        const char* prefix = container == 1 ? "> " : container == 2 ? (index ? "  " : "- ") : "";
        while (*p && *p != '\n') p++;
        memcpy(c->source + used, prefix, strlen(prefix)); used += strlen(prefix);
        memcpy(c->source + used, line, p - line); used += (size_t)(p - line);
        memcpy(c->source + used, endings[eol], strlen(endings[eol])); used += strlen(endings[eol]);
        if (*p) p++;
        line = p; index++;
    }
    strcpy(c->source + used, "\nfollowing  &amp; untouched  \n"); c->size = strlen(c->source);
}
static uint64_t field_image(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t n = {0}; uint64_t i, child, found; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_IMAGE) return id;
    for (i = 0; i < n.iChildCount; i++) { CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); found = field_image(s, child); if (found) return found; }
    return 0;
}
static xui_doc_inline_syntax_t field_syntax(xui_document_snapshot s)
{
    xui_doc_source_info_t info = {0}; xui_doc_inline_syntax_t syntax = {0}; uint64_t i;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(s, &info) == XUI_OK);
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        syntax.iSize = sizeof(syntax); CHECK(xuiDocumentSnapshotGetInlineSyntax(s, i, &syntax) == XUI_OK);
        if (syntax.iKind == XUI_DOC_SYNTAX_IMAGE) return syntax;
    }
    CHECK(0); return syntax;
}
static void field_expect_operations(xui_document_snapshot before, xui_document_change_set change,
    const xui_doc_inline_syntax_t* image_syntax, const xui_doc_node_info_t* node, const xui_doc_image_desc_t* image)
{
    xui_doc_source_info_t info = {0}; xui_doc_change_info_t changes = {0};
    xui_doc_inline_syntax_t destination = {0}, title = {0};
    uint64_t i, parent = UINT64_MAX, starts[2], ends[2], count = 0, seen = 0;
    int has_destination = 0, has_title = 0;
    int uri_changed = strcmp(node->sResource, image->sResource) != 0;
    int title_changed = strcmp(node->sTitle ? node->sTitle : "", image->sTitle ? image->sTitle : "") != 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(before, &info) == XUI_OK);
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t syntax = {0}; syntax.iSize = sizeof(syntax);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(before, i, &syntax) == XUI_OK);
        if (syntax.iKind == XUI_DOC_SYNTAX_IMAGE && syntax.iSourceStart == image_syntax->iSourceStart) parent = i;
        if (parent != UINT64_MAX && syntax.iParentIndex == parent) {
            if (syntax.iKind == XUI_DOC_SYNTAX_DESTINATION) { destination = syntax; has_destination++; }
            if (syntax.iKind == XUI_DOC_SYNTAX_TITLE) { title = syntax; has_title++; }
        }
    }
    CHECK(parent != UINT64_MAX);
    if (image_syntax->iDefinitionIndex != UINT64_MAX) {
        CHECK(!has_destination && !has_title); starts[0] = image_syntax->iContentEnd + 1; ends[0] = image_syntax->iSourceEnd; count = 1;
    } else {
        CHECK(has_destination == 1 && has_title <= 1);
        if (uri_changed) { starts[count] = destination.iSourceStart; ends[count++] = destination.iSourceEnd; }
        if (title_changed && has_title) { starts[count] = title.iSourceStart; ends[count++] = title.iSourceEnd; }
        else if (title_changed && !uri_changed) { starts[count] = ends[count] = destination.iSourceEnd; count++; }
    }
    changes.iSize = sizeof(changes); CHECK(xuiDocumentChangeSetGetInfo(change, &changes) == XUI_OK);
    for (i = 0; i < changes.iOperationCount; i++) if (changes.pOperations[i].iKind == XUI_DOC_OP_SOURCE) {
        const xui_doc_operation_t* op = &changes.pOperations[i]; uint64_t match;
        for (match = 0; match < count; match++) if (op->iOffset == starts[match] && op->iOldLength == ends[match] - starts[match]) break;
        CHECK(match < count && !(seen & (UINT64_C(1) << match))); seen |= UINT64_C(1) << match;
    }
    CHECK(seen == (UINT64_C(1) << count) - 1);
}
static void field_matrix(void)
{
    static const char* uris[] = {"new", "/new(1)?a=2&b=3", "resource.png"};
    static const char* titles[] = {"next", "quoted \"title\" & more", ""};
    unsigned fixture, eol, container, native, dialect, mode, uri, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++)
    for (container = 0; container < 3; container++) for (native = 0; native < 2; native++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (mode = 0; mode < 3; mode++) for (uri = 0; uri < 5; uri++) {
        field_case c; xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_transaction t;
        xui_document_snapshot before, after, expected, restored; xui_document_change_set change;
        xui_doc_inline_syntax_t syntax; xui_doc_node_info_t n = {0}, updated = {0}; xui_doc_image_desc_t image = {0};
        char raw[8192], text[512]; uint64_t id, bytes, suffix;
        field_build(&c, fixture, eol, container, native);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        {
            int result = xuiDocumentLoadMarkdown(d, c.source, c.size);
            if (result != XUI_OK) fprintf(stderr, "image field load fixture=%u eol=%u container=%u native=%u dialect=%u mode=%u: %d\n",
                fixture, eol, container, native, dialect, mode, result);
            CHECK(result == XUI_OK);
        }
        if (native) {
            char* wire; uint64_t wire_bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &wire_bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, wire_bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        if (mode) {
            const char* shift = "earlier `node`\n\n"; uint64_t shift_bytes = strlen(shift);
            if (mode == 1) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, shift, shift_bytes) == XUI_OK &&
                    xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t p = {0}; xui_document_prepare prepare; p.iSize = sizeof(p); p.sText = shift; p.iTextBytes = shift_bytes;
                CHECK(xuiDocumentPrepareSource(d, NULL, &p, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK &&
                    xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(prepare);
            }
            memmove(c.source + shift_bytes, c.source, c.size + 1); memcpy(c.source, shift, shift_bytes); c.size += shift_bytes;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        id = field_image(before, 1); CHECK(id); syntax = field_syntax(before); suffix = syntax.iContentEnd + 1;
        CHECK(suffix <= syntax.iSourceEnd && syntax.iSourceEnd <= c.size && c.source[suffix - 1] == ']');
        n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(before, id, &n) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopyText(before, id, text, sizeof(text), &bytes) == XUI_OK);
        if (bytes != strlen(fixtures[fixture].alt) || strcmp(text, fixtures[fixture].alt))
            fprintf(stderr, "image field alt fixture=%u eol=%u container=%u native=%u dialect=%u mode=%u: '%s' expected '%s'\n",
                fixture, eol, container, native, dialect, mode, text, fixtures[fixture].alt);
        CHECK(bytes == strlen(fixtures[fixture].alt) && !strcmp(text, fixtures[fixture].alt));
        image.iSize = sizeof(image); image.sResource = uri == 3 ? n.sResource : uris[uri < 3 ? uri : 0];
        image.sTitle = uri == 4 ? n.sTitle : titles[uri < 3 ? uri : 0]; image.sAlt = text; image.iAltBytes = bytes;
        image.sLinkTarget = n.tAttributes.iMarks & XUI_DOC_LINK ? n.sLinkTarget : NULL;
        image.sLinkTitle = image.sLinkTarget ? n.sLinkTitle : NULL;
        {
            int result = xuiDocumentBeginTransaction(d, NULL, &t);
            CHECK(result == XUI_OK);
            result = xuiDocumentTxnUpdateImage(t, id, &image);
            if (result != XUI_OK) fprintf(stderr, "image field update fixture=%u eol=%u container=%u native=%u dialect=%u mode=%u uri=%u: %d\n",
                fixture, eol, container, native, dialect, mode, uri, result);
            CHECK(result == XUI_OK);
            result = xuiDocumentTxnCommit(t, &change); CHECK(result == XUI_OK);
            xuiDocumentTxnRelease(t);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, raw, sizeof(raw), &bytes) == XUI_OK);
        if (memcmp(raw, c.source, suffix)) fprintf(stderr, "image field fixture %u rewrote retained alt before suffix %llu\n", fixture, (unsigned long long)suffix);
        CHECK(!memcmp(raw, c.source, suffix));
        CHECK(bytes >= c.size - syntax.iSourceEnd && !memcmp(raw + bytes - (c.size - syntax.iSourceEnd), c.source + syntax.iSourceEnd, c.size - syntax.iSourceEnd));
        if (!uri) {
            const char* replacement = "(<new> \"next\")";
            CHECK(bytes == suffix + strlen(replacement) + c.size - syntax.iSourceEnd && !memcmp(raw + suffix, replacement, strlen(replacement)));
        }
        updated.iSize = sizeof(updated); CHECK(xuiDocumentSnapshotGetNode(after, id, &updated) == XUI_OK &&
            !strcmp(updated.sResource, image.sResource) && !strcmp(updated.sTitle ? updated.sTitle : "", image.sTitle ? image.sTitle : ""));
        expect_text(after, id, fixtures[fixture].alt);
        CHECK(!!(updated.tAttributes.iMarks & XUI_DOC_LINK) == !!(n.tAttributes.iMarks & XUI_DOC_LINK));
        if (image.sLinkTarget) CHECK(updated.sLinkTarget && !strcmp(updated.sLinkTarget, image.sLinkTarget) &&
            !strcmp(updated.sLinkTitle ? updated.sLinkTitle : "", image.sLinkTitle ? image.sLinkTitle : ""));
        field_expect_operations(before, change, &syntax, &n, &image); xuiDocumentChangeSetRelease(change);
        CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, raw, bytes) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); expect_text(after, id, fixtures[fixture].alt); xuiDocumentSnapshotRelease(after); cases++;
    }
    printf("Image field golden: %u retained alt/marks/entities/HTML/nested/reference/EOL bytes, field-only ChangeSet, source-shift/Prepare/native/dialects, whole oracle and history cases.\n", cases);
}
static void field_faults(void)
{
    const unsigned selected[] = {0, 4, 5, 7, 9, 10, 14, 15, 16};
    unsigned fixture, container; long oom = 0, cancelled = 0;
    for (fixture = 0; fixture < sizeof(selected) / sizeof(*selected); fixture++) for (container = 0; container < 3; container++) {
        field_case c; long point; int success = 0;
        field_build(&c, selected[fixture], 1, container, 0);
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_snapshot before, after; xui_document_transaction t = NULL;
            xui_doc_node_info_t n = {0}; xui_doc_image_desc_t image = {0};
            uint64_t id, revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, c.source, c.size) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            id = field_image(before, 1); n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(before, id, &n) == XUI_OK);
            image.iSize = sizeof(image); image.sResource = "new"; image.sTitle = "next";
            image.sAlt = fixtures[selected[fixture]].alt; image.iAltBytes = strlen(image.sAlt);
            image.sLinkTarget = n.tAttributes.iMarks & XUI_DOC_LINK ? n.sLinkTarget : NULL;
            image.sLinkTitle = image.sLinkTarget ? n.sLinkTitle : NULL;
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnUpdateImage(t, id, &image);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            else if (t) CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            a.remaining = -1; xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); oom++;
            } else {
                xui_doc_inline_syntax_t syntax = field_syntax(before); char expected[8192];
                uint64_t prefix = syntax.iContentEnd + 1; const char* suffix = "(<new> \"next\")";
                memcpy(expected, c.source, prefix); strcpy(expected + prefix, suffix); strcpy(expected + prefix + strlen(suffix), c.source + syntax.iSourceEnd);
                test_source(d, expected, 1); expect_text(after, id, image.sAlt);
                CHECK(xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d)); success = 1;
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_prepare p; xui_document_snapshot before, after; xui_doc_source_patch_t edit = {0};
            xui_doc_inline_syntax_t syntax; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, c.source, c.size) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            syntax = field_syntax(before); edit.iSize = sizeof(edit); edit.iStart = syntax.iContentEnd + 1; edit.iEnd = syntax.iSourceEnd;
            edit.sText = "(<new> \"next\")"; edit.iTextBytes = strlen(edit.sText); revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentPrepareSource(d, NULL, &edit, 1, &p) == XUI_OK);
            a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
            if (a.fired) {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1);
                xuiDocumentSnapshotRelease(after); cancelled++;
            } else {
                char expected[8192]; memcpy(expected, c.source, edit.iStart); memcpy(expected + edit.iStart, edit.sText, edit.iTextBytes);
                strcpy(expected + edit.iStart + edit.iTextBytes, c.source + edit.iEnd);
                CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
                test_source(d, expected, 1); success = 1;
            }
            xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    printf("Image field faults: %ld command/publication OOM, %ld Prepare cancellation checkpoints; image/alt/source/identity/revision/history/savepoint atomic and leak-free.\n", oom, cancelled);
}
static void field_noop(void)
{
    unsigned fixture, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) {
        field_case c; xui_document d; xui_document_snapshot before, after; xui_document_transaction t;
        xui_doc_node_info_t n = {0}; xui_doc_image_desc_t image = {0}; char alt[512]; uint64_t id, bytes, revision;
        field_build(&c, fixture, 1, 1, 0); d = test_markdown_open(c.source);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        id = field_image(before, 1); n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(before, id, &n) == XUI_OK &&
            xuiDocumentSnapshotCopyText(before, id, alt, sizeof(alt), &bytes) == XUI_OK);
        image.iSize = sizeof(image); image.sResource = n.sResource; image.sTitle = n.sTitle; image.sAlt = alt; image.iAltBytes = bytes;
        image.sLinkTarget = n.tAttributes.iMarks & XUI_DOC_LINK ? n.sLinkTarget : NULL; image.sLinkTitle = image.sLinkTarget ? n.sLinkTitle : NULL;
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUpdateImage(t, id, &image) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
        test_source(d, c.source, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Image field no-op: %u images retain bytes/revision/identity/history/savepoint.\n", cases);
}
static void field_empty_destination(void)
{
    unsigned profile;
    for (profile = XUI_DOCUMENT_RICH; profile <= XUI_DOCUMENT_MARKDOWN; profile++) {
        xui_doc_desc_t desc = {0}; xui_doc_image_desc_t image = {0}; xui_doc_range_t range; xui_doc_position_t caret;
        xui_document d; xui_document_transaction t; xui_document_snapshot before, after; uint64_t id = 0, revision;
        desc.iSize = sizeof(desc); desc.iProfile = profile; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(d, "text\n\nTail\n", 11) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        range.tAnchor = range.tCaret = test_position(d, 1, XUI_DOC_POSITION_GAP, 0, XUI_DOC_AFTER);
        image.iSize = sizeof(image); image.sResource = ""; image.sAlt = "alt"; image.iAltBytes = 3; image.sTitle = "title";
        revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        if (profile == XUI_DOCUMENT_RICH) {
            CHECK(xuiDocumentTxnInsertImage(t, &range, &image, &id, &caret) == XUI_ERROR_INVALID_ARGUMENT &&
                xuiDocumentTxnCommit(t, NULL) == XUI_ERROR_INVALID_ARGUMENT);
            xuiDocumentTxnRelease(t); CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
        } else {
            CHECK(xuiDocumentTxnInsertImage(t, &range, &image, &id, &caret) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t); CHECK(id); test_source(d, "![alt](<> \"title\")", 0);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); image.sResource = NULL;
            CHECK(xuiDocumentTxnUpdateImage(t, id, &image) == XUI_ERROR_INVALID_ARGUMENT && xuiDocumentTxnCommit(t, NULL) == XUI_ERROR_INVALID_ARGUMENT);
            xuiDocumentTxnRelease(t); image.sResource = "";
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        if (profile == XUI_DOCUMENT_RICH) { inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); }
        else expect_text(after, id, "alt");
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Image field destination contract: Markdown empty URI insertion/update valid, Rich empty and NULL URI rejection atomic.");
}
static void field_collect_images(xui_document_snapshot s, uint64_t id, uint64_t* ids, unsigned* count)
{
    xui_doc_node_info_t n = {0}; uint64_t i, child; n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_IMAGE) { CHECK(*count < 2); ids[(*count)++] = id; }
    for (i = 0; i < n.iChildCount; i++) { CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); field_collect_images(s, child, ids, count); }
}
static void field_compound(void)
{
    const char* source = "before &amp; ![foo __bar__](a) and ![![baz](inner)](b) __tail__\r\n";
    const char* expected = "before &amp; ![foo __bar__](<new> \"next\") and ![![baz](inner)](<> \"second\") __tail__\r\n";
    xui_document d = test_markdown_open(source), oracle; xui_document_snapshot before, after, parsed, restored;
    xui_document_transaction t; xui_doc_image_desc_t image = {0}; uint64_t ids[2]; unsigned count = 0;
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); field_collect_images(before, 1, ids, &count); CHECK(count == 2);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    image.iSize = sizeof(image); image.sResource = "a-long-first-resource"; image.sTitle = "first"; image.sAlt = "foo bar"; image.iAltBytes = 7;
    CHECK(xuiDocumentTxnUpdateImage(t, ids[0], &image) == XUI_OK);
    image.sResource = ""; image.sTitle = "second"; image.sAlt = "baz"; image.iAltBytes = 3; CHECK(xuiDocumentTxnUpdateImage(t, ids[1], &image) == XUI_OK);
    image.sResource = "new"; image.sTitle = "next"; image.sAlt = "foo bar"; image.iAltBytes = 7; CHECK(xuiDocumentTxnUpdateImage(t, ids[0], &image) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); test_source(d, expected, 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); expect_text(after, ids[0], "foo bar"); expect_text(after, ids[1], "baz");
    oracle = test_markdown_open(expected); CHECK(xuiDocumentAcquireSnapshot(oracle, &parsed) == XUI_OK); inc_snapshot_equal(after, parsed);
    xuiDocumentSnapshotRelease(parsed); xuiDocumentRelease(oracle);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); inc_snapshot_equal(before, restored);
    expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); inc_snapshot_equal(after, restored);
    expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Image field compound: three updates across shifted image syntax in one transaction, preserved alt bytes, full oracle and one-step Undo/Redo identity.");
}
int main(void) { field_matrix(); field_faults(); field_noop(); field_empty_destination(); field_compound(); return 0; }
