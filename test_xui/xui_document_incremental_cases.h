#include <time.h>
#ifndef XUI_DLL
#include "xui_document_reference_cache_oracle.h"
#endif

/* Shared by standalone and real DLL tests. The oracle starts empty, so it
 * always parses the complete source and cannot use a local candidate. */
static xui_doc_stats_t inc_stats(xui_document d)
{
    xui_doc_stats_t s = {0}; s.iSize = sizeof(s); CHECK(xuiDocumentGetStats(d, &s) == XUI_OK); return s;
}
static void inc_tree_equal(xui_document_snapshot a, uint64_t ai, xui_document_snapshot b, uint64_t bi)
{
    xui_doc_node_info_t x = {0}, y = {0}; uint64_t i; char *xt, *yt;
    x.iSize = y.iSize = sizeof(x);
    CHECK(xuiDocumentSnapshotGetNode(a, ai, &x) == XUI_OK && xuiDocumentSnapshotGetNode(b, bi, &y) == XUI_OK);
    CHECK(x.iKind == y.iKind && x.iChildCount == y.iChildCount && x.iTextBytes == y.iTextBytes);
    CHECK(!memcmp(&x.tAttributes, &y.tAttributes, sizeof(x.tAttributes)));
    CHECK(!strcmp(x.sResource, y.sResource) && !strcmp(x.sInfo, y.sInfo) && !strcmp(x.sTitle, y.sTitle));
    CHECK(!strcmp(x.sLinkTarget, y.sLinkTarget) && !strcmp(x.sLinkTitle, y.sLinkTitle));
    if (x.iSourceStart != y.iSourceStart || x.iSourceEnd != y.iSourceEnd || x.bSourceExact != y.bSourceExact)
        fprintf(stderr, "source mismatch kind=%u/%u node=%llu/%llu source=%llu..%llu exact=%d versus %llu..%llu exact=%d syntax=%llu..%llu versus %llu..%llu\n",
            x.iKind, y.iKind, (unsigned long long)ai, (unsigned long long)bi,
            (unsigned long long)x.iSourceStart, (unsigned long long)x.iSourceEnd, x.bSourceExact,
            (unsigned long long)y.iSourceStart, (unsigned long long)y.iSourceEnd, y.bSourceExact,
            (unsigned long long)x.iSyntaxStart, (unsigned long long)x.iSyntaxEnd,
            (unsigned long long)y.iSyntaxStart, (unsigned long long)y.iSyntaxEnd);
    CHECK(x.iSourceStart == y.iSourceStart && x.iSourceEnd == y.iSourceEnd && x.bSourceExact == y.bSourceExact);
    CHECK(x.iSyntaxStart == y.iSyntaxStart && x.iSyntaxEnd == y.iSyntaxEnd && x.iSourceSegmentCount == y.iSourceSegmentCount);
    xt = malloc((size_t)x.iTextBytes + 1); yt = malloc((size_t)y.iTextBytes + 1); CHECK(xt && yt);
    CHECK(xuiDocumentSnapshotCopyText(a, ai, xt, x.iTextBytes + 1, &i) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyText(b, bi, yt, y.iTextBytes + 1, &i) == XUI_OK && !memcmp(xt, yt, (size_t)x.iTextBytes));
    free(xt); free(yt);
    {
        xui_doc_block_syntax_t p = {0}, q = {0}; int pr, qr;
        p.iSize = q.iSize = sizeof(p);
        pr = xuiDocumentSnapshotGetBlockSyntax(a, ai, &p); qr = xuiDocumentSnapshotGetBlockSyntax(b, bi, &q);
        CHECK(pr == qr && (pr != XUI_OK || !memcmp(&p, &q, sizeof(p))));
        if (pr == XUI_OK) {
            uint64_t at;
            for (at = 0; at < p.iQuotePrefixCount; at++) {
                uint64_t ps, pe, qs, qe;
                CHECK(xuiDocumentSnapshotGetQuotePrefix(a, ai, at, &ps, &pe) == XUI_OK &&
                    xuiDocumentSnapshotGetQuotePrefix(b, bi, at, &qs, &qe) == XUI_OK && ps == qs && pe == qe);
            }
            for (at = 0; at < p.iListIndentCount; at++) {
                xui_doc_list_indent_t ps = {0}, qs = {0}; ps.iSize = qs.iSize = sizeof(ps);
                CHECK(xuiDocumentSnapshotGetListContinuationIndent(a, ai, at, &ps) == XUI_OK &&
                    xuiDocumentSnapshotGetListContinuationIndent(b, bi, at, &qs) == XUI_OK && !memcmp(&ps, &qs, sizeof(ps)));
            }
            for (at = 0; at < p.iCodeIndentCount; at++) {
                xui_doc_code_indent_t ps = {0}, qs = {0}; ps.iSize = qs.iSize = sizeof(ps);
                CHECK(xuiDocumentSnapshotGetCodeIndent(a, ai, at, &ps) == XUI_OK &&
                    xuiDocumentSnapshotGetCodeIndent(b, bi, at, &qs) == XUI_OK && !memcmp(&ps, &qs, sizeof(ps)));
            }
            for (at = 0; at < p.iTableTokenCount; at++) {
                xui_doc_table_token_t ps = {0}, qs = {0}; ps.iSize = qs.iSize = sizeof(ps);
                CHECK(xuiDocumentSnapshotGetTableToken(a, ai, at, &ps) == XUI_OK &&
                    xuiDocumentSnapshotGetTableToken(b, bi, at, &qs) == XUI_OK && !memcmp(&ps, &qs, sizeof(ps)));
            }
        }
    }
    if (x.iKind == XUI_DOC_SOFT_BREAK || x.iKind == XUI_DOC_HARD_BREAK) {
        xui_doc_break_syntax_t p = {0}, q = {0}; int pr, qr; p.iSize = q.iSize = sizeof(p);
        pr = xuiDocumentSnapshotGetBreakSyntax(a, ai, &p); qr = xuiDocumentSnapshotGetBreakSyntax(b, bi, &q);
        CHECK(pr == qr && (pr != XUI_OK || !memcmp(&p, &q, sizeof(p))));
    }
    for (i = 0; i < x.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t p = {0}, q = {0}; p.iSize = q.iSize = sizeof(p);
        CHECK(xuiDocumentSnapshotGetSourceSegment(a, ai, i, &p) == XUI_OK && xuiDocumentSnapshotGetSourceSegment(b, bi, i, &q) == XUI_OK);
        CHECK(!memcmp(&p, &q, sizeof(p)));
    }
    for (i = 0; i < x.iChildCount; i++) {
        uint64_t ac, bc; CHECK(xuiDocumentSnapshotGetChild(a, ai, i, &ac) == XUI_OK && xuiDocumentSnapshotGetChild(b, bi, i, &bc) == XUI_OK);
        inc_tree_equal(a, ac, b, bc);
    }
}
static void inc_snapshot_equal(xui_document_snapshot a, xui_document_snapshot b)
{
    xui_doc_source_info_t x = {0}, y = {0}; uint64_t i;
#ifndef XUI_DLL
    reference_cache_snapshot_equal(a, b);
#endif
    inc_tree_equal(a, 1, b, 1); x.iSize = y.iSize = sizeof(x);
    CHECK(xuiDocumentSnapshotGetSourceInfo(a, &x) == XUI_OK && xuiDocumentSnapshotGetSourceInfo(b, &y) == XUI_OK && !memcmp(&x, &y, sizeof(x)));
    for (i = 0; i < x.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t p = {0}, q = {0}; p.iSize = q.iSize = sizeof(p);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(a, i, &p) == XUI_OK && xuiDocumentSnapshotGetInlineSyntax(b, i, &q) == XUI_OK);
        CHECK(!memcmp(&p, &q, sizeof(p)));
    }
    for (i = 0; i < x.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t p = {0}, q = {0}; p.iSize = q.iSize = sizeof(p);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(a, i, &p) == XUI_OK && xuiDocumentSnapshotGetReferenceDefinition(b, i, &q) == XUI_OK);
        CHECK(!memcmp(&p, &q, sizeof(p)));
    }
    for (i = 0; i < x.iReferenceCandidateCount; i++) {
        xui_doc_inline_syntax_t p = {0}, q = {0}; p.iSize = q.iSize = sizeof(p);
        CHECK(xuiDocumentSnapshotGetReferenceCandidate(a, i, &p) == XUI_OK &&
            xuiDocumentSnapshotGetReferenceCandidate(b, i, &q) == XUI_OK);
        CHECK(!memcmp(&p, &q, sizeof(p)));
    }
}
static uint64_t inc_case(const char* original, unsigned dialect, uint64_t at, uint64_t removed, const char* insert_text, int pending)
{
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0}; xui_document d, oracle;
    xui_document_snapshot old, actual, expected, restored; xui_doc_stats_t before, after;
    size_t bytes = strlen(original), added = strlen(insert_text); char* edited = malloc(bytes - removed + added + 1);
    uint64_t incremental;
    CHECK(edited); memcpy(edited, original, (size_t)at); memcpy(edited + at, insert_text, added);
    memcpy(edited + at + added, original + at + removed, bytes - (size_t)at - (size_t)removed + 1);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, bytes) == XUI_OK);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
    before = inc_stats(d);
    if (pending) {
        xui_document_prepare p; xui_doc_source_patch_t patch = prepare_patch(at, at + removed, insert_text);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
        CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(p);
    } else {
        xui_document_transaction t; tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        {
            int result = xuiDocumentTxnReplaceSource(t, at, at + removed, insert_text, added);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            if (result != XUI_OK) fprintf(stderr, "incremental edit failed result=%d at=%llu removed=%llu replacement=[%s] original=[%s]\n",
                result, (unsigned long long)at, (unsigned long long)removed, insert_text, original);
            CHECK(result == XUI_OK);
        }
        xuiDocumentTxnRelease(t);
    }
    after = inc_stats(d); incremental = after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses;
    CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, edited, bytes - removed + added) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    if (bytes - removed + added < 4096) test_source(d, edited, 1);
    else {
        char* actual_source = malloc(bytes - removed + added + 1); uint64_t copied;
        CHECK(actual_source);
        CHECK(xuiDocumentSnapshotCopySource(actual, actual_source,
            bytes - removed + added + 1, &copied) == XUI_OK &&
            copied == bytes - removed + added &&
            !memcmp(actual_source, edited, (size_t)copied + 1));
        free(actual_source);
    }
    if (strcmp(original, edited)) {
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(old, restored); expect_identity_tree(old, restored, 1); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(actual, restored); expect_identity_tree(actual, restored, 1); xuiDocumentSnapshotRelease(restored);
    }
    xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle); free(edited); return incremental;
}
static void inc_definition_position_oracle(const char* original, const char* edited,
    uint64_t start, uint64_t old_bytes, const char* replacement)
{
    xui_document d = test_markdown_open(original), oracle;
    xui_document_transaction t; xui_document_snapshot before, actual, expected;
    xui_doc_txn_desc_t tx = {0}; size_t i;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, start, start + old_bytes,
        replacement, strlen(replacement)) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    {
        xui_doc_node_info_t old_root = {0}, new_root = {0}; uint64_t child;
        old_root.iSize = new_root.iSize = sizeof(old_root);
        CHECK(xuiDocumentSnapshotGetNode(before, 1, &old_root) == XUI_OK &&
            xuiDocumentSnapshotGetNode(actual, 1, &new_root) == XUI_OK &&
            old_root.iChildCount == new_root.iChildCount);
        for (child = 0; child < old_root.iChildCount; child++) {
            uint64_t old_id, new_id;
            CHECK(xuiDocumentSnapshotGetChild(before, 1, child, &old_id) == XUI_OK &&
                xuiDocumentSnapshotGetChild(actual, 1, child, &new_id) == XUI_OK &&
                old_id == new_id);
        }
    }
    for (i = 0; i <= strlen(edited); i++) {
        unsigned affinity;
        for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++) {
            xui_doc_position_t a, b; xui_doc_node_info_t an = {0}, bn = {0}; int aq, bq;
            CHECK(xuiDocumentSourceToPositionEx(actual, i, affinity, &a, &aq) == XUI_OK);
            CHECK(xuiDocumentSourceToPositionEx(expected, i, affinity, &b, &bq) == XUI_OK);
            CHECK(aq == bq && a.iKind == b.iKind && a.iOffset == b.iOffset);
            an.iSize = sizeof(an); bn.iSize = sizeof(bn);
            CHECK(xuiDocumentSnapshotGetNode(actual, a.iNodeId, &an) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(expected, b.iNodeId, &bn) == XUI_OK);
            CHECK(an.iKind == bn.iKind && an.iSourceStart == bn.iSourceStart &&
                an.iSourceEnd == bn.iSourceEnd);
        }
    }
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
}
static void incremental_markdown_differential(void)
{
    const char* blocks[] = {"alpha &amp; **beta** [label] and `code`\n", "## heading **bold**\n", "first line\nsecond _line_\n",
        "heading\n=======\n", "utf8 \xe4\xb8\xad\xf0\x9f\x98\x80 &NotEqualTilde;\n"};
    const char* edits[] = {"Q", "", "*", "**", "[x](/u)", "[ref]: /a\n", "[^n]: value\n", "\n", "\r", "\r\n", "---", "```", "\n\n", "</div>", "a@b.com", "\\", "&amp;"};
    const char* prefix = "before **one**\n\n";
    const char* suffix = "\n> tail **bold _nested_** &amp; [u](/a)\n\n- item\n- last\n\n```c\na();\n```\n\n![image](/i)\n";
    unsigned dialect, b, e; uint64_t cases = 0, incremental = 0; size_t at;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        for (b = 0; b < sizeof(blocks) / sizeof(*blocks); b++) {
            char original[1024]; snprintf(original, sizeof(original), "%s%s%s", prefix, blocks[b], suffix);
            for (at = 0; at < strlen(blocks[b]); at++) {
                size_t end = at + 1;
                if (((unsigned char)blocks[b][at] & 0xc0) == 0x80) continue;
                while (((unsigned char)blocks[b][end] & 0xc0) == 0x80) end++;
                for (e = 0; e < sizeof(edits) / sizeof(*edits); e++) {
                    uint64_t remove = e & 1 ? end - at : 0;
                    /* Log only if a comparison fails, without making passing runs noisy. */
                    incremental += inc_case(original, dialect, strlen(prefix) + at, remove, edits[e], (int)(cases & 1)); cases++;
                }
            }
        }
    }
    CHECK(incremental > cases / 2);
    /* Global cohorts also update footnote uses when an independent unrelated
     * block can be retained; the complete-load oracle still checks every span. */
    CHECK(inc_case("[ref]: /global\n\nalpha target\n\ntail\n", XUI_MD_EXTENDED, 22, 6, "[ref]", 1) == 1);
    CHECK(inc_case("[^n]: unused\n\nalpha target\n\ntail\n", XUI_MD_EXTENDED, 20, 6, "[^n]", 1) == 1);
    {
        const char* unused = "alpha [ref] target\n\ntail\n\n[ref]: /u\n[^n]: unused\n";
        const char* used_elsewhere = "before[^n]\n\nalpha [ref] target\n\n[ref]: /u\n[^n]: note\n";
        const char* earlier_definition = "before[^n]\n\n[^n]: note\n\nalpha [ref] target\n\n[ref]: /u\n\ntail\n";
        const char* earlier_formatted = "before[^n]\n\n[^n]: *note*\n\nalpha [ref] target\n\n[ref]: /u\n\ntail\n";
        const char* reordered_notes = "a[^b] b[^a]\n\nalpha [ref] target\n\n[ref]: /u\n[^a]: *A*\n[^b]: *B*\n";
        const char* many_earlier_notes = "n[^a] n[^b] n[^c] n[^d]\n\n[^a]: A\n[^b]: B\n[^c]: C\n[^d]: D\n\nalpha [ref] target\n\n[ref]: /u\n\ntail\n";
        const char* new_use = "alpha [ref] target\n\ntail\n\n[ref]: /u\n[^n]: note\n";
        const char* remove_use = "before[^n]\n\nalpha [ref] target\n\n[ref]: /u\n[^n]: note\n";
        const char* footnote_only = "alpha [missing] target\n\n[^n]: unused\n\ntail\n";
        CHECK(inc_case(unused, XUI_MD_EXTENDED,
            (uint64_t)(strstr(unused, "target") - unused), 6, "longer", 0) == 1);
        CHECK(inc_case(used_elsewhere, XUI_MD_EXTENDED,
            (uint64_t)(strstr(used_elsewhere, "target") - used_elsewhere), 6, "longer", 1) == 1);
        CHECK(inc_case(footnote_only, XUI_MD_EXTENDED,
            (uint64_t)(strstr(footnote_only, "target") - footnote_only),
            6, "longer", 0) == 1);
        CHECK(inc_case(earlier_definition, XUI_MD_EXTENDED,
            (uint64_t)(strstr(earlier_definition, "target") - earlier_definition),
            6, "longer", 0) == 1);
        CHECK(inc_case(many_earlier_notes, XUI_MD_EXTENDED,
            (uint64_t)(strstr(many_earlier_notes, "target") - many_earlier_notes),
            6, "longer plain", 1) == 1);
        CHECK(inc_case(earlier_formatted, XUI_MD_EXTENDED,
            (uint64_t)(strstr(earlier_formatted, "target") - earlier_formatted),
            6, "longer", 1) == 1);
        CHECK(!inc_case(reordered_notes, XUI_MD_EXTENDED,
            (uint64_t)(strstr(reordered_notes, "target") - reordered_notes),
            6, "longer", 0));
        CHECK(!inc_case(used_elsewhere, XUI_MD_EXTENDED,
            (uint64_t)(strstr(used_elsewhere, "[^n]") - used_elsewhere), 4, "[^new]", 0));
        CHECK(inc_case(new_use, XUI_MD_EXTENDED,
            (uint64_t)(strstr(new_use, "target") - new_use), 6, "[^n]", 1) == 1);
        CHECK(!inc_case(remove_use, XUI_MD_EXTENDED,
            (uint64_t)(strstr(remove_use, "[^n]") - remove_use), 4, "plain", 0));
    }
    {
        const char* before = "[ref]: /global \"title\"\n\nalpha target\n\n[ref] and tail\n";
        const char* after = "alpha target\n\n[ref] and tail\n\n[ref]: /global \"title\"\n";
        const char* unused = "[^note]: unused\n\nalpha target\n\ntail\n";
        const char* removed_use = "[ref]: /global\n\nalpha [ref] tail\n\nend\n";
        const char* duplicate = "[ref]: /first\n\nmiddle target\n\n[REF]: /ignored\n\n[ref] tail\n";
        const char* linked = "[ref]: /global \"title\"\n\nalpha [ref] target\n\ntail\n";
        const char* linked_after = "alpha [ref] target\n\n[ref]: /global \"title\"\n\ntail\n";
        const char* linked_duplicate = "[ref]: /first\n\nalpha [ref] target\n\n[REF]: /ignored\n\ntail\n";
        const char* linked_image = "before\n\n![alt][img] target\n\n[img]: /image \"title\"\n\nlast\n";
        const char* linked_crlf = "[ref]: /global \"title\"\r\n\r\n[hello][ref] target\r\n\r\ntail\r\n";
        const char* unresolved = "[ref]: /global\n\nalpha [missing] target\n\ntail\n";
        const char* frontmatter = "---\nkey: value\n---\n\nalpha [ref] target\n\n[ref]: /global\n\ntail\n";
        const char* bom = "\xef\xbb\xbf[ref]: /global\n\nalpha [ref] target\n\ntail\n";
        const char* definition_eof = "alpha [ref] target\n\ntail\n\n[ref]: /global";
        const char* unicode_label = "[\xc3\x84]: /global\n\nalpha [\xc3\xa4] target\n\ntail\n";
        const char* entity_destination = "[ref]: /a&amp;b \"t &amp; t\"\n\nalpha **[ref]** target\n\ntail\n";
        CHECK(inc_case(before, XUI_MD_EXTENDED,
            (uint64_t)(strstr(before, "target") - before), 6, "longer plain", 0) == 1);
        CHECK(inc_case(after, XUI_MD_GFM,
            (uint64_t)(strstr(after, "target") - after), 6, "short", 1) == 1);
        CHECK(inc_case(unused, XUI_MD_EXTENDED,
            (uint64_t)(strstr(unused, "target") - unused), 6, "plain", 0) == 1);
        CHECK(inc_case(removed_use, XUI_MD_GFM,
            (uint64_t)(strstr(removed_use, "[ref] tail") - removed_use), 5, "plain", 0) == 1);
        CHECK(inc_case(duplicate, XUI_MD_GFM,
            (uint64_t)(strstr(duplicate, "target") - duplicate), 6, "longer plain", 1) == 1);
        CHECK(inc_case(linked, XUI_MD_GFM,
            (uint64_t)(strstr(linked, "target") - linked), 6, "longer", 0) == 1);
        CHECK(inc_case(linked_after, XUI_MD_GFM,
            (uint64_t)(strstr(linked_after, "target") - linked_after), 6, "short", 1) == 1);
        CHECK(inc_case(linked_duplicate, XUI_MD_GFM,
            (uint64_t)(strstr(linked_duplicate, "target") - linked_duplicate), 6, "longer", 0) == 1);
        CHECK(inc_case(linked_image, XUI_MD_GFM,
            (uint64_t)(strstr(linked_image, "target") - linked_image), 6, "longer", 1) == 1);
        CHECK(inc_case(linked_crlf, XUI_MD_GFM,
            (uint64_t)(strstr(linked_crlf, "target") - linked_crlf), 6, "longer", 0) == 1);
        CHECK(inc_case(unresolved, XUI_MD_GFM,
            (uint64_t)(strstr(unresolved, "target") - unresolved), 6, "longer", 1) == 1);
        CHECK(inc_case(unresolved, XUI_MD_GFM,
            (uint64_t)(strstr(unresolved, "[missing]") - unresolved), 9, "[ref]", 0) == 1);
        CHECK(inc_case(linked, XUI_MD_GFM,
            (uint64_t)(strstr(linked, "[ref] target") - linked), 5, "[missing]", 1) == 1);
        CHECK(inc_case(frontmatter, XUI_MD_EXTENDED,
            (uint64_t)(strstr(frontmatter, "target") - frontmatter), 6, "longer", 0) == 1);
        CHECK(inc_case(bom, XUI_MD_GFM,
            (uint64_t)(strstr(bom, "target") - bom), 6, "longer", 1) == 1);
        CHECK(inc_case(definition_eof, XUI_MD_GFM,
            (uint64_t)(strstr(definition_eof, "target") - definition_eof), 6, "longer", 0) == 1);
        CHECK(inc_case(unicode_label, XUI_MD_GFM,
            (uint64_t)(strstr(unicode_label, "target") - unicode_label), 6, "longer", 1) == 1);
        CHECK(inc_case(entity_destination, XUI_MD_GFM,
            (uint64_t)(strstr(entity_destination, "target") - entity_destination), 6, "longer", 0) == 1);
        CHECK(inc_case(before, XUI_MD_EXTENDED,
            (uint64_t)(strstr(before, "/global") - before), 7, "/changed", 1) == 1);
        CHECK(inc_case(after, XUI_MD_GFM,
            (uint64_t)(strstr(after, "target") - after), 6, "[ref]", 0) == 1);
    }
    CHECK(inc_case("before\r\n\r\nalpha target\r\n\r\ntail **span**\r\n", XUI_MD_EXTENDED, 16, 6, "new", 0) == 1);
    CHECK(inc_case("---\nkey: value\n---\n\nalpha target\n\ntail\n", XUI_MD_EXTENDED, 26, 6, "new", 1) == 1);
    CHECK(inc_case("before\n\nalpha\n\nbeta\n", XUI_MD_EXTENDED, 13, 1, "\r", 0) == 1);
    CHECK(inc_case("before\r\ralpha\r\rbeta\r", XUI_MD_EXTENDED, 8, 0, "\n", 1) == 1);
    CHECK(inc_case("[ref]\n\nalpha\n\ntail\n", XUI_MD_EXTENDED, 7, 5, "[ref]: /new", 1) == 1);
    CHECK(inc_case("[^n]\n\nalpha\n\ntail\n", XUI_MD_EXTENDED, 6, 5, "[^n]: new", 0) == 1);
    {
        const char* candidates = "[missing]\n\nmiddle\n\n[other]\n";
        CHECK(inc_case(candidates, XUI_MD_EXTENDED,
            (uint64_t)(strstr(candidates, "middle") - candidates), 6, "middle!", 0) == 1);
    }
    printf("Markdown incremental differential: %llu edits, %llu local candidates; three dialects, source/tree/spans, full-load oracle and Undo/Redo passed\n",
        (unsigned long long)cases, (unsigned long long)incremental);
}
static void incremental_reference_differential(void)
{
    const char* prefix = "[ref]: /global \"title\"\n\n";
    const char* body = "alpha [ref] [missing] ![image][ref] &amp; **bold** `code` tail\n";
    const char* suffix = "\n[REF]: /ignored\n\nlast\n";
    const char* edits[] = {"Q", "", "[ref]", "[missing]", "[new]: /n", "\n", "\r", "&amp;", "*", "\\", "]", "("};
    unsigned dialect; size_t at, e; uint64_t cases = 0, local = 0;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        char original[512]; snprintf(original, sizeof(original), "%s%s%s", prefix, body, suffix);
        for (at = 0; at < strlen(body); at += 3)
            for (e = 0; e < sizeof(edits) / sizeof(*edits); e++) {
                uint64_t remove = e & 1 ? 1 : 0;
                local += inc_case(original, dialect, strlen(prefix) + at, remove, edits[e], (int)(cases & 1));
                cases++;
            }
    }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown external-definition differential: %llu edits, %llu local; three dialects, full-load oracle and Undo/Redo passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_unused_definition_edit(void)
{
    const char* unused = "[ref]: /unused\n\nalpha plain\n\ntail\n";
    const char* duplicate = "[ref]: /first\n\n[REF]: /later\n\nalpha [ref]\n\ntail\n";
    const char* used = "[ref]: /first\n\nalpha [ref]\n\ntail\n";
    const char* crlf = "before\r\n\r\n[ref]: /unused\r\n\r\nafter\r\n";
    const char* front = "---\nname: demo\n---\n\n[ref]: /unused\n\nbody\n";
    const char* eof = "body\n\n[ref]: /unused";
    const char* alone = "[ref]: /unused\n";
    const char* title = "[ref]: /unused \"title\"\n\nbody\n";
    const char* nested = "before\n\n> [ref]: /unused\n\nbody\n";
    const char* replacements[] = {"x", " ", "<", ">", "(", ")", "[", "]", "\\", "&"};
    unsigned dialect, i; uint64_t cases = 0, local = 0;
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused), 7, "/otherx", 0) == 1);
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused), 7, "/much-longer", 1) == 1);
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused) + 2, 0, "insert", 0) == 1);
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused), 0, "abc", 1) == 1);
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused) + 7, 0, "abc", 0) == 1);
    CHECK(inc_case(unused, XUI_MD_GFM,
        (uint64_t)(strstr(unused, "/unused") - unused), 7, "/x", 0) == 1);
    CHECK(inc_case(duplicate, XUI_MD_GFM,
        (uint64_t)(strstr(duplicate, "/later") - duplicate), 6, "/other", 1) == 1);
    CHECK(inc_case(used, XUI_MD_GFM,
        (uint64_t)(strstr(used, "/first") - used), 6, "/other", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(crlf, "/unused") - crlf), 7, "/otherx", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(crlf, "/unused") - crlf), 7, "/longer-url", 1) == 1);
    CHECK(inc_case(front, XUI_MD_EXTENDED,
        (uint64_t)(strstr(front, "/unused") - front), 7, "/otherx", 1) == 1);
    CHECK(inc_case(front, XUI_MD_EXTENDED,
        (uint64_t)(strstr(front, "/unused") - front), 7, "/x", 0) == 1);
    CHECK(inc_case(eof, XUI_MD_GFM,
        (uint64_t)(strstr(eof, "/unused") - eof), 7, "/otherx", 0) == 1);
    CHECK(inc_case(eof, XUI_MD_GFM,
        (uint64_t)(strstr(eof, "/unused") - eof), 7, "/longer-url", 1) == 1);
    CHECK(inc_case(alone, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(alone, "/unused") - alone), 7, "/otherx", 1) == 1);
    CHECK(inc_case(alone, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(alone, "/unused") - alone), 7, "/x", 0) == 1);
    CHECK(inc_case(title, XUI_MD_GFM,
        (uint64_t)(strstr(title, "title") - title), 5, "other", 0) == 1);
    CHECK(inc_case(title, XUI_MD_GFM,
        (uint64_t)(strstr(title, "title") - title), 5, "longer title", 1) == 1);
    CHECK(inc_case(title, XUI_MD_GFM,
        (uint64_t)(strstr(title, "title") - title), 0, "abc", 0) == 1);
    CHECK(!inc_case(nested, XUI_MD_GFM,
        (uint64_t)(strstr(nested, "/unused") - nested), 7, "/otherx", 1));
    CHECK(inc_case(duplicate, XUI_MD_GFM,
        (uint64_t)(strstr(duplicate, "/later") - duplicate), 6, "/longer-url", 0) == 1);
    {
        const char* later = "[unused]: /short\n\nalpha [active] and [missing]\n\n[active]: /yes\n\n[tail]: /end\n";
        CHECK(inc_case(later, XUI_MD_GFM,
            (uint64_t)(strstr(later, "/short") - later), 6, "/much-longer", 1) == 1);
    }
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        const char* destination = strstr(unused, "/unused"); size_t at;
        for (at = 0; at < 7; at++)
            for (i = 0; i < sizeof(replacements) / sizeof(*replacements); i++) {
                local += inc_case(unused, dialect, (uint64_t)(destination - unused + at),
                    1, replacements[i], (int)(cases & 1)); cases++;
            }
    }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown unused link-definition destination differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
    puts("Markdown unused link-definition destination edit: standalone parse and full-load oracle passed");
}
static void incremental_unused_definition_variable_differential(void)
{
    const char* source = "[unused]: /abcdef\n\nalpha [missing] &amp; **bold**\n\n[active]: /yes\n\nlast [active]\n";
    const char* replacements[] = {"", "q", "long", " ", "[]", "&amp;", "<x>", "?", "/path", "xyz123"};
    const char* destination = strstr(source, "/abcdef");
    uint64_t cases = 0, local = 0; unsigned dialect, r; size_t at;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
        for (at = 0; at < 7; at++)
            for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
                local += inc_case(source, dialect, (uint64_t)(destination - source + at),
                    1, replacements[r], (int)(cases & 1)); cases++;
            }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown unused variable-length definition differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_unused_footnote_body(void)
{
    const char* before = "before\n\n[^n]: plain body\n\nmiddle [missing]\n\nlast\n";
    const char* before_edited = "before\n\n[^n]: plain longer body\n\nmiddle [missing]\n\nlast\n";
    const char* after = "[ref]: /x\n\nalpha [ref]\n\n[^n]: plain body\n\nlast\n";
    const char* several = "first\n\n[^a]: A\n\n[^n]: plain body\n\n[^z]: Z\n\nlast\n";
    const char* used = "before[^n]\n\n[^n]: plain body\n\nlast\n";
    const char* alone = "[^n]: plain body\n";
    const char* crlf = "before\r\n\r\n[^n]: plain body\r\n\r\nlast\r\n";
    const char* duplicate = "[^n]: first\n\n[^n]: plain body\n\nlast\n";
    const char* replacements[] = {"", "x", "longer", " ", "\n", "[^z]", "[x]", ":", "\r", "\t", "&amp;"};
    uint64_t cases = 0, local = 0; size_t i; unsigned r;
    CHECK(inc_case(before, XUI_MD_EXTENDED,
        (uint64_t)(strstr(before, "plain body") - before), 5, "other", 0) == 1);
    CHECK(inc_case(before, XUI_MD_EXTENDED,
        (uint64_t)(strstr(before, "body") - before), 4, "longer body", 1) == 1);
    inc_definition_position_oracle(before, before_edited,
        (uint64_t)(strstr(before, "body") - before), 4, "longer body");
    CHECK(inc_case(after, XUI_MD_EXTENDED,
        (uint64_t)(strstr(after, "plain body") - after), 5, "other", 1) == 1);
    CHECK(inc_case(several, XUI_MD_EXTENDED,
        (uint64_t)(strstr(several, "body") - several), 4, "longer body", 0) == 1);
    CHECK(inc_case(alone, XUI_MD_EXTENDED,
        (uint64_t)(strstr(alone, "body") - alone), 4, "longer body", 1) == 1);
    CHECK(inc_case(crlf, XUI_MD_EXTENDED,
        (uint64_t)(strstr(crlf, "body") - crlf), 4, "longer body", 0) == 1);
    CHECK(inc_case(duplicate, XUI_MD_EXTENDED,
        (uint64_t)(strstr(duplicate, "body") - duplicate), 4, "longer body", 1) == 1);
    CHECK(inc_case(used, XUI_MD_EXTENDED,
        (uint64_t)(strstr(used, "body") - used), 4, "longer body", 0) == 1);
    for (i = 0; i < strlen("plain body"); i++)
        for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
            local += inc_case(before, XUI_MD_EXTENDED,
                (uint64_t)(strstr(before, "plain body") - before + i),
                1, replacements[r], (int)(cases & 1)); cases++;
        }
    CHECK(local > cases / 5 && local <= cases);
    printf("Markdown unused footnote body edit: %llu edits, %llu local; full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_used_footnote_body(void)
{
    const char* simple = "before[^n]\n\n[^n]: plain body\n\nlast\n";
    const char* simple_edited = "before[^n]\n\n[^n]: plain longer body\n\nlast\n";
    const char* rich = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] body\n\nlast\n";
    const char* reordered = "a[^b] b[^a]\n\n[^a]: *A* [ref]\n\n[^b]: B body\n\n[ref]: /u\n\nlast\n";
    const char* reordered_rich = "a[^b] b[^a]\n\n[^a]: ***A*** [ref]\n\n[^b]: **B** [ref] body\n\n[ref]: /u\n\nlast\n";
    const char* reordered_rich_edited = "a[^b] b[^a]\n\n[^a]: ***A*** [ref]\n\n[^b]: **B** [ref] longer body\n\n[ref]: /u\n\nlast\n";
    const char* early = "[^n]: note body\n\nlast[^n]\n";
    const char* candidate = "before[^n]\n\n[^n]: [missing] body\n\nlast\n";
    const char* nested_marks = "before[^n]\n\n[ref]: /u\n\n[^n]: ***bold [ref]*** body\n\nlast\n";
    const char* blocks = "before[^n]\n\n[^n]: intro **bold**\n\n    - one\n    - two\n\nlast\n";
    const char* crlf = "before[^n]\r\n\r\n[^n]: note body\r\n\r\nlast\r\n";
    const char* repeated = "before[^n]\n\n[^n]: first body\n\n[^n]: later\n\nlast\n";
    const char* repeated_use = "before[^n] and again[^n]\n\n[^n]: note body\n\nlast\n";
    const char* unicode = "before[^\xc3\xa4]\n\n[^\xc3\x84]: note body\n\nlast\n";
    const char* nested = "before[^n]\n\n[^n]: body [^other]\n\n[^other]: more\n";
    const char* duplicate = "before[^n]\n\n[^n]: first\n\n[^n]: later body\n";
    const char* replacements[] = {"", "x", "long", " ", "\n", "[ref]", "*", "&amp;", "\r"};
    uint64_t local = 0, cases = 0; size_t i; unsigned r;
    CHECK(inc_case(simple, XUI_MD_EXTENDED,
        (uint64_t)(strstr(simple, "body") - simple), 4, "longer body", 0) == 1);
    inc_definition_position_oracle(simple, simple_edited,
        (uint64_t)(strstr(simple, "body") - simple), 4, "longer body");
    CHECK(inc_case(rich, XUI_MD_EXTENDED,
        (uint64_t)(strstr(rich, "body") - rich), 4, "longer body", 1) == 1);
    CHECK(inc_case(reordered, XUI_MD_EXTENDED,
        (uint64_t)(strstr(reordered, "body") - reordered), 4, "longer body", 0) == 1);
    CHECK(inc_case(reordered, XUI_MD_EXTENDED,
        (uint64_t)(strstr(reordered, "*A*") - reordered) + 1, 1, "longer", 1) == 1);
    CHECK(inc_case(reordered, XUI_MD_EXTENDED,
        (uint64_t)(strstr(reordered, "B body") - reordered), 1, "*bold*", 0) == 1);
    CHECK(inc_case(reordered_rich, XUI_MD_EXTENDED,
        (uint64_t)(strstr(reordered_rich, "body") - reordered_rich), 4, "longer body", 1) == 1);
    inc_definition_position_oracle(reordered_rich, reordered_rich_edited,
        (uint64_t)(strstr(reordered_rich, "body") - reordered_rich), 4, "longer body");
    CHECK(inc_case(reordered_rich, XUI_MD_EXTENDED,
        (uint64_t)(strstr(reordered_rich, "***A***") - reordered_rich) + 3, 1, "longer", 0) == 1);
    CHECK(inc_case(early, XUI_MD_EXTENDED,
        (uint64_t)(strstr(early, "body") - early), 4, "longer body", 1) == 1);
    CHECK(inc_case(candidate, XUI_MD_EXTENDED,
        (uint64_t)(strstr(candidate, "body") - candidate), 4, "longer body", 0) == 1);
    CHECK(inc_case(nested_marks, XUI_MD_EXTENDED,
        (uint64_t)(strstr(nested_marks, "body") - nested_marks), 4, "longer body", 1) == 1);
    CHECK(inc_case(blocks, XUI_MD_EXTENDED,
        (uint64_t)(strstr(blocks, "intro") - blocks), 5, "longer intro", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_EXTENDED,
        (uint64_t)(strstr(crlf, "body") - crlf), 4, "longer body", 1) == 1);
    CHECK(inc_case(repeated, XUI_MD_EXTENDED,
        (uint64_t)(strstr(repeated, "body") - repeated), 4, "longer body", 0) == 1);
    CHECK(inc_case(repeated_use, XUI_MD_EXTENDED,
        (uint64_t)(strstr(repeated_use, "body") - repeated_use), 4, "longer body", 1) == 1);
    CHECK(inc_case(unicode, XUI_MD_EXTENDED,
        (uint64_t)(strstr(unicode, "body") - unicode), 4, "longer body", 0) == 1);
    CHECK(!inc_case(nested, XUI_MD_EXTENDED,
        (uint64_t)(strstr(nested, "body") - nested), 4, "other", 0));
    CHECK(!inc_case(duplicate, XUI_MD_EXTENDED,
        (uint64_t)(strstr(duplicate, "body") - duplicate), 4, "longer", 1));
    for (i = 0; i < strlen("**bold** [ref] body"); i++)
        for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
            local += inc_case(rich, XUI_MD_EXTENDED,
                (uint64_t)(strstr(rich, "**bold** [ref] body") - rich + i),
                1, replacements[r], (int)(cases & 1)); cases++;
        }
    CHECK(local > cases / 3 && local <= cases);
    printf("Markdown used footnote body edit: %llu edits, %llu local; tree, syntax and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_used_footnote_chain(void)
{
    const char* source = "before[^n]\n\n[^n]: plain body\n\nlast\n";
    const char* edited = "before[^n]\n\n[^n]: plain change\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "body") - source);
    xui_document d = test_markdown_open(source), oracle;
    xui_document_transaction t; xui_document_snapshot actual, expected;
    xui_doc_txn_desc_t tx = {0}; xui_doc_stats_t before, after;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 4, "longer", 6) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 6, "change", 6) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 2);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown used footnote chained source edits: both patches local and match full parse");
}
static void incremental_used_footnote_multinote_chain(void)
{
    const char* source = "a[^b] b[^a]\n\n[^a]: *A* body\n\n[^b]: B body\n\nlast\n";
    const char* edited = "a[^b] b[^a]\n\n[^a]: *A* longer body\n\n[^b]: B longer body\n\nlast\n";
    uint64_t first = (uint64_t)(strstr(source, "*A* body") - source) + 4;
    uint64_t second = (uint64_t)(strstr(source, "B body") - source) + 2;
    xui_document d = test_markdown_open(source), oracle;
    xui_document_transaction t; xui_document_snapshot before_snapshot, actual, expected, restored;
    xui_doc_txn_desc_t tx = {0}; xui_doc_stats_t before, after;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentAcquireSnapshot(d, &before_snapshot) == XUI_OK);
    before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, first, first + 4, "longer body", 11) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, second + 7, second + 11,
        "longer body", 11) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 2);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    inc_snapshot_equal(before_snapshot, restored); xuiDocumentSnapshotRelease(restored);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    inc_snapshot_equal(actual, restored); xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(before_snapshot); xuiDocumentSnapshotRelease(actual);
    xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown two-note chained source edits: both local, reordered notes and Undo/Redo match full parse");
}
static void incremental_used_footnote_mixed_chain(void)
{
    const char* source = "prefix plain\n\nmid[^n]\n\n[^n]: note body\n\nlast\n";
    const char* edited = "prefix longer plain\n\nmid[^n]\n\n[^n]: note longer body\n\nlast\n";
    uint64_t first = (uint64_t)(strstr(source, "plain") - source);
    uint64_t second = (uint64_t)(strstr(source, "body") - source);
    xui_document d = test_markdown_open(source), oracle;
    xui_document_transaction t; xui_document_snapshot actual, expected;
    xui_doc_txn_desc_t tx = {0}; xui_doc_stats_t before, after;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, first, first + 5,
        "longer plain", 12) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, second + 7, second + 11,
        "longer body", 11) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 2);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown normal-block then footnote chained edits: both local and match full parse");
}
static void incremental_used_footnote_chain_failures(void)
{
    const char* source = "before[^n]\n\n[^n]: plain body\n\nlast\n";
    const char* edited = "before[^n]\n\n[^n]: plain change\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "body") - source);
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
        xui_doc_txn_desc_t tx = {0}; xui_document d, oracle;
        xui_document_transaction t; xui_document_snapshot old, actual, expected;
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free;
        desc.pAllocatorUser = &a;
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, at, at + 4,
            "longer", 6) == XUI_OK);
        a.remaining = point;
        result = xuiDocumentTxnReplaceSource(t, at, at + 6, "change", 6);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        a.remaining = -1;
        xuiDocumentTxnRelease(t);
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                xuiDocumentGetRevision(d) == revision);
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 2);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected);
            xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown chained used-footnote allocation-failure sweep: %ld points; atomic state and no leaks passed\n", point);
}
static void incremental_used_footnote_prepare_batch(void)
{
    const char* source = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] body\n\nlast\n";
    const char* edited = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] change\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "body") - source);
    xui_doc_source_patch_t patches[2] = {
        prepare_patch(at, at + 4, "longer"),
        prepare_patch(at, at + 6, "change")
    };
    xui_document d = test_markdown_open(source), oracle;
    xui_document_prepare p; xui_document_snapshot actual, expected;
    xui_doc_stats_t before = inc_stats(d), after;
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 2, &p) == XUI_OK);
    CHECK(xuiDocumentPrepareRun(p) == XUI_OK &&
        xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(p);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 128);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown Prepare two-patch footnote batch: one local parse matches full load");
}
static void incremental_used_footnote_prepare_continue(void)
{
    const char* source = "before[^n]\n\n[^n]: plain body\n\nlast\n";
    const char* edited = "before[^n]\n\n[^n]: plain change\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "body") - source);
    xui_doc_source_patch_t first = prepare_patch(at, at + 4, "longer");
    xui_doc_source_patch_t second = prepare_patch(at, at + 6, "change");
    xui_document d = test_markdown_open(source), oracle;
    xui_document_prepare p, continued; xui_document_snapshot actual, expected;
    xui_doc_stats_t before = inc_stats(d), after;
    CHECK(xuiDocumentPrepareSource(d, NULL, &first, 1, &p) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, p, &second, 1, &continued) == XUI_OK);
    CHECK(xuiDocumentPrepareRun(continued) == XUI_OK &&
        xuiDocumentPreparePublish(d, continued, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(continued); xuiDocumentPrepareRelease(p);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 128);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown continued Prepare footnote batch: one local parse matches full load");
}
static void incremental_used_footnote_prepare_batch_guard(void)
{
    const char* source = "a[^b] b[^a]\n\n[^a]: A body\n\n[^b]: B body\n\nlast\n";
    const char* edited = "a[^b] b[^a]\n\n[^a]: A longer body\n\n[^b]: B longer body\n\nlast\n";
    uint64_t first_at = (uint64_t)(strstr(source, "A body") - source) + 2;
    uint64_t second_at = (uint64_t)(strstr(source, "B body") - source) + 2;
    xui_doc_source_patch_t patches[2] = {
        prepare_patch(first_at, first_at + 4, "longer body"),
        prepare_patch(second_at + 7, second_at + 11, "longer body")
    };
    xui_document d = test_markdown_open(source), oracle;
    xui_document_prepare p; xui_document_snapshot actual, expected;
    xui_doc_stats_t before = inc_stats(d), after;
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 2, &p) == XUI_OK);
    CHECK(xuiDocumentPrepareRun(p) == XUI_OK &&
        xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(p);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown cross-footnote Prepare batch: global cohort matches full-load oracle");
}
static void incremental_used_footnote_prepare_batch_failures(void)
{
    const char* source = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] body\n\nlast\n";
    const char* edited = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] change\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "body") - source);
    xui_doc_source_patch_t patches[2] = {
        prepare_patch(at, at + 4, "longer"),
        prepare_patch(at, at + 6, "change")
    };
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
        xui_document d, oracle; xui_document_prepare p = NULL;
        xui_document_snapshot old, actual, expected;
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free;
        desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d); a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, patches, 2, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                xuiDocumentGetRevision(d) == revision);
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected);
            xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        if (p) xuiDocumentPrepareRelease(p);
        xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown Prepare footnote batch allocation-failure sweep: %ld points; atomic state and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL};
        xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free;
        desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, patches, 2, &p) == XUI_OK);
        a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision);
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p);
        xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown Prepare footnote batch cancellation sweep: %ld checkpoints; atomic state and no leaks passed\n", point);
}
static void incremental_used_footnote_budget_guard(void)
{
    size_t capacity = 80000, used = 0; char* source = malloc(capacity);
    uint64_t at;
    CHECK(source != NULL);
    used += (size_t)snprintf(source + used, capacity - used,
        "remote [ref]\n\nbefore[^n]\n\n");
    memset(source + used, '[', 16); used += 16;
    used += (size_t)snprintf(source + used, capacity - used, "\n\n");
    memset(source + used, 'z', 20000); used += 20000;
    used += (size_t)snprintf(source + used, capacity - used, "\n\n[ref]: /");
    memset(source + used, 'a', 50000); used += 50000;
    used += (size_t)snprintf(source + used, capacity - used,
        "\n\n[^n]: body [ note\n");
    CHECK(used < capacity);
    at = (uint64_t)(strstr(source, "body [") - source) + 5;
    CHECK(inc_case(source, XUI_MD_EXTENDED, at, 1, "x", 0) == 1);
    free(source);
    puts("Markdown footnote same-length bracket-budget guard: full fallback and oracle match");
}
static void incremental_single_use_definition_edit(void)
{
    const char* repeated = "[ref]: /first\n\nalpha [ref] and [ref] tail\n\nother\n";
    const char* heading = "[ref]: /first\n\n# heading [ref] tail\n\nother\n";
    const char* image = "[img]: /image\n\n![alt][img] tail\n\nother\n";
    const char* title = "[ref]: /first \"title\"\n\nalpha [ref] tail\n\nother\n";
    const char* duplicate = "[ref]: /first\n\n[REF]: /later\n\nalpha [ref] tail\n\nother\n";
    const char* crlf = "[ref]: /first\r\n\r\nalpha [ref] tail\r\n\r\nother\r\n";
    const char* front = "---\nname: demo\n---\n\n[ref]: /first\n\nalpha [ref] tail\n\nother\n";
    const char* after = "alpha [ref] tail\n\nother\n\n[ref]: /first";
    const char* multi = "[ref]: /first\n\nalpha [ref]\n\nother [ref]\n";
    const char* empty_destination = "[ref]: <>\n\nalpha [ref] tail\n\nother\n";
    const char* empty_title = "[ref]: /x \"\"\n\nalpha [ref] tail\n\nother\n";
    const char* replacements[] = {"x", " ", "<", ">", "(", ")", "[", "]", "\\", "&"};
    uint64_t cases = 0, local = 0; unsigned dialect, i; size_t at;
    CHECK(inc_case(repeated, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(repeated, "/first") - repeated), 6, "/other", 1) == 1);
    CHECK(inc_case(repeated, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(repeated, "/first") - repeated), 6, "/much-longer", 0) == 1);
    CHECK(inc_case(repeated, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(repeated, "/first") - repeated) + 2, 0, "insert", 1) == 1);
    CHECK(inc_case(repeated, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(repeated, "/first") - repeated), 0, "abc", 0) == 1);
    CHECK(inc_case(repeated, XUI_MD_COMMONMARK,
        (uint64_t)(strstr(repeated, "/first") - repeated) + 6, 0, "abc", 1) == 1);
    CHECK(inc_case(heading, XUI_MD_GFM,
        (uint64_t)(strstr(heading, "/first") - heading), 6, "/other", 0) == 1);
    CHECK(inc_case(heading, XUI_MD_GFM,
        (uint64_t)(strstr(heading, "/first") - heading), 6, "/longer-url", 1) == 1);
    CHECK(inc_case(image, XUI_MD_GFM,
        (uint64_t)(strstr(image, "/image") - image), 6, "/other", 1) == 1);
    CHECK(inc_case(image, XUI_MD_GFM,
        (uint64_t)(strstr(image, "/image") - image), 6, "/longer-image", 0) == 1);
    CHECK(inc_case(title, XUI_MD_EXTENDED,
        (uint64_t)(strstr(title, "title") - title), 5, "other", 0) == 1);
    CHECK(inc_case(title, XUI_MD_EXTENDED,
        (uint64_t)(strstr(title, "title") - title), 5, "longer title", 1) == 1);
    CHECK(inc_case(duplicate, XUI_MD_GFM,
        (uint64_t)(strstr(duplicate, "/first") - duplicate), 6, "/other", 1) == 1);
    CHECK(inc_case(duplicate, XUI_MD_GFM,
        (uint64_t)(strstr(duplicate, "/first") - duplicate), 6, "/much-longer", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_GFM,
        (uint64_t)(strstr(crlf, "/first") - crlf), 6, "/other", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_GFM,
        (uint64_t)(strstr(crlf, "/first") - crlf), 6, "/much-longer", 1) == 1);
    CHECK(inc_case(front, XUI_MD_EXTENDED,
        (uint64_t)(strstr(front, "/first") - front), 6, "/other", 1) == 1);
    CHECK(inc_case(front, XUI_MD_EXTENDED,
        (uint64_t)(strstr(front, "/first") - front), 6, "/much-longer", 0) == 1);
    CHECK(inc_case(after, XUI_MD_GFM,
        (uint64_t)(strstr(after, "/first") - after), 6, "/other", 0) == 1);
    CHECK(inc_case(after, XUI_MD_GFM,
        (uint64_t)(strstr(after, "/first") - after), 6, "/much-longer", 1) == 1);
    CHECK(inc_case(multi, XUI_MD_GFM,
        (uint64_t)(strstr(multi, "/first") - multi), 6, "/other", 1) == 1);
    CHECK(inc_case(multi, XUI_MD_GFM,
        (uint64_t)(strstr(multi, "/first") - multi), 6, "/much-longer", 0) == 1);
    CHECK(inc_case(empty_destination, XUI_MD_GFM,
        (uint64_t)(strstr(empty_destination, "<>") - empty_destination) + 1,
        0, "/x", 1) == 1);
    CHECK(inc_case(empty_title, XUI_MD_GFM,
        (uint64_t)(strstr(empty_title, "\"\"") - empty_title) + 1,
        0, "named", 0) == 1);
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        const char* dest = strstr(repeated, "/first");
        for (at = 0; at < 6; at++)
            for (i = 0; i < sizeof(replacements) / sizeof(*replacements); i++) {
                local += inc_case(repeated, dialect,
                    (uint64_t)(dest - repeated + at), 1, replacements[i], (int)(cases & 1)); cases++;
            }
    }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown single-block definition dependency differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_single_use_variable_differential(void)
{
    const char* source = "[ref]: /abcdef\n\nalpha [ref] and ![image][ref] &amp; **bold**\n\nlast\n";
    const char* replacements[] = {"", "q", "long", " ", "[]", "&amp;", "<x>", "?", "/path", "xyz123"};
    const char* destination = strstr(source, "/abcdef");
    uint64_t cases = 0, local = 0; unsigned dialect, r; size_t at;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
        for (at = 0; at < 7; at++)
            for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
                local += inc_case(source, dialect, (uint64_t)(destination - source + at),
                    1, replacements[r], (int)(cases & 1)); cases++;
            }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown single-block variable-length definition differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
    inc_definition_position_oracle(
        "[unused]: /short\n\nalpha [missing]\n\n[active]: /yes\n\nlast [active]\n",
        "[unused]: /much-longer\n\nalpha [missing]\n\n[active]: /yes\n\nlast [active]\n",
        10, 6, "/much-longer");
    inc_definition_position_oracle(
        "[ref]: /first\n\nalpha [ref]\n\nlast\n",
        "[ref]: /much-longer\n\nalpha [ref]\n\nlast\n",
        7, 6, "/much-longer");
    puts("Markdown definition-length source-to-position mapping matches full reload for both affinities");
}
static void incremental_multi_use_definition_differential(void)
{
    const char* source =
        "before [ref] and [ref]\n\n"
        "[ref]: /abcdef \"title\"\n\n"
        "# heading ![image][ref]\n\n"
        "after [ref] tail\n\n"
        "[other]: /unchanged\n\n"
        "last [other]\n";
    const char* replacements[] = {"", "q", "long", " ", "[]", "&amp;", "<x>", "?", "/path", "xyz123"};
    const char* destination = strstr(source, "/abcdef");
    uint64_t cases = 0, local = 0; unsigned dialect, r; size_t at;
    CHECK(destination);
    CHECK(inc_case(source, XUI_MD_COMMONMARK,
        (uint64_t)(destination - source), 7, "/much-longer", 0) == 1);
    CHECK(inc_case(source, XUI_MD_GFM,
        (uint64_t)(strstr(source, "title") - source), 5, "new title", 1) == 1);
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
        for (at = 0; at < 7; at++)
            for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
                local += inc_case(source, dialect, (uint64_t)(destination - source + at),
                    1, replacements[r], (int)(cases & 1)); cases++;
            }
    CHECK(local > cases / 4 && local <= cases);
    printf("Markdown multi-block definition dependency differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
    inc_definition_position_oracle(
        "first [ref]\n\n[ref]: /short\n\nsecond [ref]\n\nthird [ref]\n",
        "first [ref]\n\n[ref]: /much-longer\n\nsecond [ref]\n\nthird [ref]\n",
        20, 6, "/much-longer");
    {
        char many[4096]; size_t used = 0, i;
        used += (size_t)snprintf(many + used, sizeof(many) - used, "[ref]: /short\n\n");
        for (i = 0; i < 129; i++) {
            used += (size_t)snprintf(many + used, sizeof(many) - used, "block %zu [ref]\n\n", i);
            if (i == 31 || i == 32 || i == 63 || i == 64 || i == 127 || i == 128)
                CHECK(inc_case(many, XUI_MD_GFM, 7, 6, "/much-longer", (int)(i & 1)) == 1);
        }
        CHECK(used < sizeof(many));
    }
    puts("Markdown multi-block definition source positions and growing dependency sets match full reload");
}
static void incremental_label_change_differential(void)
{
    const char* source =
        "before [old] and [new]\n\n"
        "[old]: /target\n\n"
        "# heading ![image][old]\n\n"
        "after [new] tail\n";
    const char* duplicate =
        "[old]: /one\n\n[new]: /two\n\nfirst [new]\n\nlast [old]\n";
    const char* earlier =
        "[new]: /two\n\n[old]: /one\n\nfirst [new]\n\nlast [old]\n";
    const char* crlf = "[old]: /one\r\n\r\nfirst [old]\r\n\r\nlast [new]\r\n";
    const char* front = "---\nname: demo\n---\n\n[old]: /one\n\nfirst [old]\n\nlast [new]\n";
    const char* replacements[] = {"", "n", "new", "x", "[]", "&amp;", " ", "NEW", "xyz"};
    const char* label = strstr(source, "[old]:") + 1;
    uint64_t cases = 0, local = 0; unsigned dialect, r; size_t at;
    CHECK(inc_case(source, XUI_MD_COMMONMARK, (uint64_t)(label - source), 3, "new", 0) == 1);
    CHECK(inc_case(duplicate, XUI_MD_GFM, 1, 3, "new", 1) == 1);
    CHECK(inc_case(earlier, XUI_MD_EXTENDED,
        (uint64_t)(strstr(earlier, "[old]:") - earlier) + 1, 3, "new", 0) == 1);
    CHECK(inc_case(crlf, XUI_MD_GFM, 1, 3, "new", 1) == 1);
    CHECK(inc_case(front, XUI_MD_EXTENDED,
        (uint64_t)(strstr(front, "[old]:") - front) + 1, 3, "new", 0) == 1);
    CHECK(!inc_case("[old]: /one\n\n```\n[old]\n```\n\nlast [old]\n",
        XUI_MD_GFM, 1, 3, "new", 0));
    CHECK(inc_case("[Foo]: /one\n\nfirst [FOO]\n\nlast [bar]\n",
        XUI_MD_GFM, 1, 3, "bar", 1) == 1);
    CHECK(inc_case("[\xc3\x84]: /one\n\nfirst [\xc3\xa4]\n\nlast [new]\n",
        XUI_MD_GFM, 1, 2, "new", 0) == 1);
    CHECK(inc_case("[old]: /one\n\nfirst [old]\n\nlast [new]\n",
        XUI_MD_COMMONMARK, 1, 0, "x", 1) == 1);
    CHECK(inc_case("[old]: /one\n\nfirst [old]\n\nlast [new]\n",
        XUI_MD_COMMONMARK, 4, 0, "x", 0) == 1);
    {
        const char* eof = "first [old]\n\nlast [new]\n\n[old]: /one";
        CHECK(inc_case(eof, XUI_MD_GFM,
            (uint64_t)(strstr(eof, "[old]:") - eof) + 1, 3, "new", 1) == 1);
    }
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
        for (at = 0; at < 3; at++)
            for (r = 0; r < sizeof(replacements) / sizeof(*replacements); r++) {
                local += inc_case(source, dialect, (uint64_t)(label - source + at),
                    1, replacements[r], (int)(cases & 1)); cases++;
            }
    CHECK(local > cases / 5 && local <= cases);
    printf("Markdown link-definition label differential: %llu edits, %llu local; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
    inc_definition_position_oracle(
        "first [old]\n\n[old]: /target\n\nsecond [new]\n\nthird [old]\n",
        "first [old]\n\n[new]: /target\n\nsecond [new]\n\nthird [old]\n",
        14, 3, "new");
    {
        char many[4096]; size_t used = 0, i;
        used += (size_t)snprintf(many + used, sizeof(many) - used, "[old]: /target\n\n");
        for (i = 0; i < 129; i++) {
            used += (size_t)snprintf(many + used, sizeof(many) - used, "block %zu [old]\n\n", i);
            if (i == 31 || i == 32 || i == 63 || i == 64 || i == 127 || i == 128)
                CHECK(inc_case(many, XUI_MD_GFM, 1, 3, "new", (int)(i & 1)) == 1);
        }
        CHECK(used < sizeof(many));
    }
    puts("Markdown label changes preserve source positions and unsafe fenced-block fallback");
}
static void incremental_unused_definition_scale(void)
{
    char* source = malloc(160000); size_t used = 0, i;
    xui_document d; xui_document_transaction t; xui_doc_txn_desc_t tx = {0};
    xui_doc_stats_t before, after;
    CHECK(source);
    used += (size_t)snprintf(source + used, 160000 - used, "[ref]: /unused\n\n");
    for (i = 0; i < 1500; i++) {
        int written = snprintf(source + used, 160000 - used,
            "paragraph %zu plain body with repeated text and no local reference\n\n", i);
        CHECK(written > 0 && (size_t)written < 160000 - used); used += (size_t)written;
    }
    CHECK(inc_case(source, XUI_MD_GFM, 7, 7, "/otherx", 1) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 7, 14, "/otherx", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes <= 20);
    printf("Markdown unused link-definition scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d);
    CHECK(inc_case(source, XUI_MD_GFM, 7, 7, "/much-longer", 0) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 7, 14, "/much-longer", 12) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes <= 32);
    printf("Markdown unused variable-length definition scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
static void incremental_multi_use_definition_scale(void)
{
    char* source = malloc(160000); size_t used = 0, i;
    xui_document d; xui_document_transaction t; xui_doc_txn_desc_t tx = {0};
    xui_doc_stats_t before, after;
    CHECK(source);
    used += (size_t)snprintf(source + used, 160000 - used, "[ref]: /global\n\n");
    for (i = 0; i < 1500; i++) {
        int written = snprintf(source + used, 160000 - used,
            (i == 0 || i == 750 || i == 1499) ?
                "paragraph %zu [ref] body with repeated text\n\n" :
                "paragraph %zu plain body with repeated text and no local reference\n\n", i);
        CHECK(written > 0 && (size_t)written < 160000 - used); used += (size_t)written;
    }
    CHECK(inc_case(source, XUI_MD_GFM, 7, 7, "/much-longer", 1) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 7, 14, "/much-longer", 12) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 1000);
    printf("Markdown multi-block definition scale: %zu source bytes, %llu parsed bytes across three distant blocks\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
static void incremental_label_change_scale(void)
{
    char* source = malloc(160000); size_t used = 0, i;
    xui_document d; xui_document_transaction t; xui_doc_txn_desc_t tx = {0};
    xui_doc_stats_t before, after;
    CHECK(source);
    used += (size_t)snprintf(source + used, 160000 - used, "[old]: /global\n\n");
    for (i = 0; i < 1500; i++) {
        int written = snprintf(source + used, 160000 - used,
            (i == 0 || i == 750 || i == 1499) ?
                "paragraph %zu [old] and [new] body\n\n" :
                "paragraph %zu plain body with repeated text and no local reference\n\n", i);
        CHECK(written > 0 && (size_t)written < 160000 - used); used += (size_t)written;
    }
    CHECK(inc_case(source, XUI_MD_GFM, 1, 3, "new", 1) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 1, 4, "new", 3) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 1000);
    printf("Markdown definition-label scale: %zu source bytes, %llu parsed bytes across three distant blocks\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
static void incremental_growing_definition_scale(void)
{
    unsigned fixture;
    for (fixture = 0; fixture < 2; fixture++) {
        char* source = malloc(160000); size_t used = 0;
        unsigned references = 0, i;
        const char* label = fixture ? "old" : "ref";
        const char* replacement = fixture ? "new" : "/longer";
        uint64_t start = fixture ? 1 : 7, removed = fixture ? 3 : 6;
        xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
        xui_document d; xui_document_transaction t;
        xui_doc_stats_t before, after;
        CHECK(source);
        used += (size_t)snprintf(source + used, 160000 - used,
            "%s", fixture ? "[old]: /target\n\n" : "[ref]: /short\n\n");
        for (i = 0; i < 1500; i++) {
            int written = i % 11 == 0 && references < 129 ?
                snprintf(source + used, 160000 - used,
                    "paragraph %u [%s] with nearby text\n\n", i, label) :
                snprintf(source + used, 160000 - used,
                    "paragraph %u plain text with no local reference and more content\n\n", i);
            CHECK(written > 0 && (size_t)written < 160000 - used);
            used += (size_t)written;
            if (i % 11 == 0 && references < 129) references++;
        }
        CHECK(references == 129 && used > 80000);
        CHECK(inc_case(source, XUI_MD_GFM, start, removed, replacement,
            (int)fixture) == 1);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_GFM; desc.bDisableHistory = 1;
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
        before = inc_stats(d);
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, start, start + removed,
            replacement, strlen(replacement)) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); after = inc_stats(d);
        CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
            after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 16000);
        printf("Markdown growing %s definition dependency scale: %zu source bytes, 129 blocks, %llu parsed bytes\n",
            fixture ? "label" : "value", used,
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
        xuiDocumentRelease(d); free(source);
    }
}
static void incremental_container_definition_differential(void)
{
    const char* cases[] = {
        "[ref]: /old\n\nbefore\n\n> quote [ref]\n\nlast\n",
        "[ref]: /old\n\nbefore\n\n- first [ref]\n- second plain\n\nlast\n",
        "[ref]: /old\n\nbefore\n\n| Value |\n| --- |\n| [ref] |\n\nlast\n",
        "before [ref]\n\n> quote [ref]\n\n[ref]: /old\n\nlast\n",
        "[ref]: /old\r\n\r\nbefore\r\n\r\n- first [ref]\r\n- second plain\r\n\r\nlast\r\n",
        "[ref]: /old\n\n- first [ref]\n\n- second [ref]\n\nlast\n",
        "[ref]: /old\n\n1. first [ref]\n2. second ![ref]\n\nlast\n",
        "[ref]: /old\n\n> - nested [ref]\n> - more plain\n\nlast\n",
        "[ref]: /old\n\n- first\n  > nested [ref]\n\nlast\n",
        "[ref]: /old\n\n| Left | Right |\n| :--- | ---: |\n| [ref] | ![ref] |\n\nlast\n",
        "[ref]: /old\n\nbefore [ref]\n\n> quote [ref]\n\n- list [ref]\n\n| Value |\n| --- |\n| [ref] |\n\nlast\n",
        "[ref]: /old\r\n\r\nbefore\r\n\r\n> quote [ref]\r\n\r\nlast\r\n"
    };
    uint64_t count = 0; unsigned i;
    for (i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        const char* source = cases[i];
        const char* destination = strstr(source, "/old");
        const char* label = strstr(source, "[ref]:") + 1;
        CHECK(destination && label);
        uint64_t value = inc_case(source, i == 2 ? XUI_MD_GFM : XUI_MD_EXTENDED,
            (uint64_t)(destination - source), 4, "/new-target", (int)(i & 1));
        uint64_t renamed = inc_case(source, i == 2 ? XUI_MD_GFM : XUI_MD_EXTENDED,
            (uint64_t)(label - source), 3, "new", (int)((i + 1) & 1));
        if (value != 1 || renamed != 1)
            fprintf(stderr, "container definition fixture %u: value=%llu label=%llu\n",
                i, (unsigned long long)value, (unsigned long long)renamed);
        CHECK(value == 1 && renamed == 1);
        count += 2;
    }
    {
        const char* internal = "[ref]: /old\n\n> [inner]: /x\n> quote [ref]\n\nlast\n";
        const char* footnote = "[ref]: /old\n\n> quote [ref] and note[^n]\n\n[^n]: note\n";
        const char* destination = strstr(internal, "/old");
        CHECK(destination && !inc_case(internal, XUI_MD_EXTENDED,
            (uint64_t)(destination - internal), 4, "/new-target", 0));
        CHECK(!inc_case(internal, XUI_MD_EXTENDED, 1, 3, "new", 1));
        destination = strstr(footnote, "/old");
        CHECK(destination && !inc_case(footnote, XUI_MD_EXTENDED,
            (uint64_t)(destination - footnote), 4, "/new-target", 0));
    }
    printf("Markdown container link-definition differential: %llu value/label edits match full reload and Undo/Redo\n",
        (unsigned long long)count);
}
static void incremental_definition_value_failures(void)
{
    const char* sources[] = {
        "[ref]: /unused \"title\"\n\nalpha plain\n\ntail\n",
        "[ref]: /unused \"title\"\n\nalpha [ref] tail\n\nother\n",
        "[ref]: /unused \"title\"\n\nalpha plain\n\ntail\n",
        "[ref]: /unused \"title\"\n\nalpha [ref] tail\n\nother\n",
        "[ref]: /unused \"title\"\n\nalpha [ref] tail\n\nother [ref]\n",
        "[ref]: /unused \"title\"\n\nalpha [ref] tail\n\nother [ref]\n",
        "[old]: /one \"title\"\n\nalpha [old] tail\n\nother [new]\n",
        "[old]: /one \"title\"\n\nalpha [old] tail\n\nother [longer]\n",
        "[ref]: /unused \"title\"\n\n- alpha [ref]\n- beta plain\n\nother\n",
        "[old]: /one \"title\"\n\n> alpha [old] tail\n\nother [new]\n"};
    const char* edited_sources[] = {
        "[ref]: /otherx \"title\"\n\nalpha plain\n\ntail\n",
        "[ref]: /otherx \"title\"\n\nalpha [ref] tail\n\nother\n",
        "[ref]: /much-longer \"title\"\n\nalpha plain\n\ntail\n",
        "[ref]: /much-longer \"title\"\n\nalpha [ref] tail\n\nother\n",
        "[ref]: /otherx \"title\"\n\nalpha [ref] tail\n\nother [ref]\n",
        "[ref]: /much-longer \"title\"\n\nalpha [ref] tail\n\nother [ref]\n",
        "[new]: /one \"title\"\n\nalpha [old] tail\n\nother [new]\n",
        "[longer]: /one \"title\"\n\nalpha [old] tail\n\nother [longer]\n",
        "[ref]: /otherx \"title\"\n\n- alpha [ref]\n- beta plain\n\nother\n",
        "[new]: /one \"title\"\n\n> alpha [old] tail\n\nother [new]\n"};
    const char* replacements[] = {"/otherx", "/otherx", "/much-longer", "/much-longer", "/otherx", "/much-longer", "new", "longer", "/otherx", "new"};
    const uint64_t starts[] = {7, 7, 7, 7, 7, 7, 1, 1, 7, 1};
    const uint64_t removed[] = {7, 7, 7, 7, 7, 7, 3, 3, 7, 3};
    const char* names[] = {"unused", "single-block", "unused-variable",
        "single-block-variable", "multi-block", "multi-block-variable",
        "label", "label-variable", "list-value", "quote-label"};
    unsigned fixture;
    for (fixture = 0; fixture < sizeof(sources) / sizeof(*sources); fixture++) {
    const char* source = sources[fixture]; const char* edited = edited_sources[fixture];
    const char* replacement = replacements[fixture];
    long point; int success = 0;
    for (point = 0; point < 1000 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
        xui_document d, oracle; xui_document_transaction t;
        xui_document_snapshot old, actual, expected; uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK); a.remaining = point;
        result = xuiDocumentTxnReplaceSource(t, starts[fixture],
            starts[fixture] + removed[fixture], replacement, strlen(replacement));
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        a.remaining = -1; xuiDocumentTxnRelease(t);
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown %s link-definition allocation-failure sweep: %ld points; atomic state and no leaks passed\n",
        names[fixture], point);
    success = 0;
    for (point = 0; point < 1000 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(starts[fixture],
            starts[fixture] + removed[fixture], replacement);
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown %s link-definition cancellation sweep: %ld checkpoints; published state and no leaks passed\n",
        names[fixture], point);
    }
}
static void incremental_independent_blocks(void)
{
    const char* code = "before\n\n```c\nalpha\n```\n\nafter\n";
    const char* diagram = "before\n\n```mermaid\nflowchart TD\n```\n\nafter\n";
    const char* html = "before\n\n<div>\nhello\n</div>\n\nafter\n";
    const char* indented = "before\n\n    alpha\n\nafter\n";
    const char* rule = "before\n\n---\n\nafter\n";
    const char* close;
    unsigned dialect;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        CHECK(inc_case(rule, dialect, (uint64_t)(strstr(rule, "---") - rule), 3, "***", 0) == 1);
        CHECK(inc_case(code, dialect, (uint64_t)(strstr(code, "alpha") - code), 5, "beta", 1) == 1);
        close = strstr(strstr(code, "```c") + 4, "```"); CHECK(close);
        CHECK(inc_case(code, dialect, (uint64_t)(close - code), 3, "``", 0) == 0);
        CHECK(inc_case(indented, dialect, (uint64_t)(strstr(indented, "alpha") - indented), 5, "beta", 1) == 1);
        CHECK(inc_case(html, dialect, (uint64_t)(strstr(html, "hello") - html), 5, "world", 0) == 1);
        close = strstr(html, "</div>"); CHECK(close);
        CHECK(inc_case(html, dialect, (uint64_t)(close - html), 6, "", 1) == 0);
        if (dialect == XUI_MD_EXTENDED)
            CHECK(inc_case(diagram, dialect, (uint64_t)(strstr(diagram, "flowchart") - diagram), 9, "graph", 1) == 1);
        CHECK(!inc_case("- first\n\nalpha\n\nlast\n", dialect, 9, 5, "- second", 0));
        (void)inc_case("> first\n\nalpha\n\nlast\n", dialect, 9, 5, "> second", 1);
    }
    CHECK(inc_case("before\n\n```\nalpha\n```\n", XUI_MD_COMMONMARK, 12, 5, "beta", 0) == 1);
    CHECK(!inc_case("before\n\n```\nalpha\n```\n\n", XUI_MD_COMMONMARK, 12, 5, "beta", 0));
    {
        const char* list_crlf = "before\r\n\r\n- one\r\n- two\r\n\r\nafter\r\n";
        const char* code_crlf = "before\r\n\r\n    alpha\r\n\r\nafter\r\n";
        CHECK(inc_case(list_crlf, XUI_MD_COMMONMARK,
            (uint64_t)(strstr(list_crlf, "one") - list_crlf), 3, "ONE", 0) == 1);
        CHECK(inc_case(code_crlf, XUI_MD_EXTENDED,
            (uint64_t)(strstr(code_crlf, "alpha") - code_crlf), 5, "beta", 1) == 1);
    }
    puts("Markdown independent leaf blocks: rule, fenced/indented code, Mermaid and HTML match full loads; ambiguous context falls back");
}
static void incremental_code_newline_provenance(void)
{
    const char* sources[] = {"head **before**\n\n```\n\nalpha\n```\n\nlast\n",
        "head **before**\r\n\r\n```\r\n\r\nalpha\r\n```\r\n\r\nlast\r\n"};
    unsigned i;
    for (i = 0; i < sizeof(sources) / sizeof(sources[0]); i++) {
        const char* source = sources[i]; const char* fence = strstr(source, "```");
        xui_document d = test_markdown_open(source); xui_document_snapshot s;
        xui_doc_node_info_t code = {0}; xui_doc_source_segment_t first = {0}; uint64_t id, content;
        CHECK(fence); content = (uint64_t)(fence - source) + (i ? 5 : 4);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        id = find_kind(s, 1, XUI_DOC_CODE_BLOCK); CHECK(id);
        code.iSize = sizeof(code); first.iSize = sizeof(first);
        CHECK(xuiDocumentSnapshotGetNode(s, id, &code) == XUI_OK && code.iSourceSegmentCount);
        CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, 0, &first) == XUI_OK);
        CHECK(code.iSyntaxStart == (uint64_t)(fence - source) && code.iSourceStart == content);
        CHECK(first.iSourceStart == content && first.iSourceEnd > content);
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    puts("Markdown fenced code blank-line provenance starts inside the current block for LF and CRLF");
}
static void inc_crlf_copy(const char* source, char* target, size_t capacity)
{
    size_t i, j = 0;
    for (i = 0; source[i]; i++) {
        CHECK(j + (source[i] == '\n' ? 2 : 1) < capacity);
        if (source[i] == '\n') target[j++] = '\r';
        target[j++] = source[i];
    }
    target[j] = 0;
}
static void incremental_block_differential(void)
{
    const char* blocks[] = {"---\n", "```c\nalpha\n```\n", "~~~mermaid\na-->b\n~~~\n",
        "<div>\nhello\n</div>\n", "    indented\n", "> first\n> second\n",
        "- first\n- second\n", "| A | B |\n| - | - |\n| x | y |\n"};
    const char* edits[] = {"Q", "", "`", "```", "~", "\n", "\r", "\n\n", "***", "</div>"};
    const char* prefix = "head **before**\n\n";
    const char* suffix = "\ntail _after_\n\nlast\n";
    unsigned dialect, b, e, ending; uint64_t cases = 0, local = 0; size_t at;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
        for (b = 0; b < sizeof(blocks) / sizeof(blocks[0]); b++)
        for (ending = 0; ending < 2; ending++) {
            char raw[256], source[512], block[256], lead[64];
            uint64_t block_local = 0;
            snprintf(raw, sizeof(raw), "%s%s%s", prefix, blocks[b], suffix);
            if (ending) {
                inc_crlf_copy(raw, source, sizeof(source));
                inc_crlf_copy(blocks[b], block, sizeof(block));
                inc_crlf_copy(prefix, lead, sizeof(lead));
            } else {
                strcpy(source, raw); strcpy(block, blocks[b]); strcpy(lead, prefix);
            }
            for (at = 0; at < strlen(block); at++)
                for (e = 0; e < sizeof(edits) / sizeof(edits[0]); e++) {
                    uint64_t removed = (e & 1) ? 1 : 0;
                    block_local += inc_case(source, dialect, strlen(lead) + at, removed, edits[e], (int)(cases & 1));
                    cases++;
                }
            local += block_local;
            if (!ending && (b == 4 || b == 5 || b == 6 || (b == 7 && dialect != XUI_MD_COMMONMARK))) CHECK(block_local);
        }
    CHECK(local > cases / 12);
    printf("Markdown independent block differential: %llu LF/CRLF edits, %llu local candidates; three dialects and full-load oracle passed\n",
        (unsigned long long)cases, (unsigned long long)local);
}
static void incremental_neighbor_failures(void)
{
    const char* source = "- one\n\n```c\nalpha\n```\n\nlast\n";
    const char* edited = "- one\n\n```c\nalQha\n```\n\nlast\n";
    uint64_t at = (uint64_t)(strstr(source, "alpha") - source) + 2;
    long point; int success = 0;
    for (point = 0; point < 2000 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_prepare p = NULL; xui_document_snapshot old, actual, expected;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 1, "Q");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 && inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown neighbor-context allocation-failure sweep: %ld points; atomic source/tree/history and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 2000 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 1, "Q"); int result; uint64_t revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown neighbor-context cancellation sweep: %ld checkpoints; unchanged published snapshots and no leaks passed\n", point);
}
static void incremental_block_versions(void)
{
    const char* versions[] = {
        "before\n\n```c\nalpha\n```\n\nmiddle\n\n- one\n- two\n\nafter\n",
        "before\n\n```c\nalphaX\n```\n\nmiddle\n\n- one\n- two\n\nafter\n",
        "before\n\n```c\nalphaX\n```\n\nmiddle\n\n- oneY\n- two\n\nafter\n",
        "before\n\n```c\nalphaX\n```\n\nmiddle\n\n- oneY\n- two\n\nafterZ\n"
    };
    const char* markers[] = {"alpha", "- one", "after"};
    xui_document d = test_markdown_open(versions[0]); xui_document_snapshot snapshots[4], actual, expected;
    xui_doc_txn_desc_t tx = {0}; uint64_t i, step, before_parses;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[0]) == XUI_OK);
    before_parses = inc_stats(d).iMarkdownIncrementalParses;
    for (step = 1; step < 4; step++) {
        uint64_t at = (uint64_t)(strstr(versions[step - 1], markers[step - 1]) - versions[step - 1]) + strlen(markers[step - 1]);
        const char* added = step == 1 ? "X" : step == 2 ? "Y" : "Z";
        xui_document oracle;
        if (step == 2) {
            xui_document_prepare p; xui_doc_source_patch_t patch = prepare_patch(at, at, added);
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
            CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(p);
        } else {
            xui_document_transaction t;
            CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
            CHECK(xuiDocumentTxnReplaceSource(t, at, at, added, 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t);
        }
        CHECK(inc_stats(d).iMarkdownIncrementalParses == before_parses + step);
        oracle = test_markdown_open(versions[step]);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[step]) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(snapshots[step], expected);
        for (i = 0; i <= strlen(versions[step]); i++) {
            unsigned affinity;
            for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++) {
                xui_doc_position_t a, b; xui_doc_node_info_t an = {0}, bn = {0}; int aq, bq;
                CHECK(xuiDocumentSourceToPositionEx(snapshots[step], i, affinity, &a, &aq) == XUI_OK);
                CHECK(xuiDocumentSourceToPositionEx(expected, i, affinity, &b, &bq) == XUI_OK);
                CHECK(aq == bq && a.iKind == b.iKind && a.iOffset == b.iOffset);
                an.iSize = sizeof(an); bn.iSize = sizeof(bn);
                CHECK(xuiDocumentSnapshotGetNode(snapshots[step], a.iNodeId, &an) == XUI_OK);
                CHECK(xuiDocumentSnapshotGetNode(expected, b.iNodeId, &bn) == XUI_OK);
                CHECK(an.iKind == bn.iKind && an.iSourceStart == bn.iSourceStart && an.iSourceEnd == bn.iSourceEnd);
            }
        }
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    for (step = 3; step; step--) {
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[step - 1]); expect_identity_tree(actual, snapshots[step - 1], 1);
        xuiDocumentSnapshotRelease(actual);
    }
    for (step = 1; step < 4; step++) {
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[step]); expect_identity_tree(actual, snapshots[step], 1);
        xuiDocumentSnapshotRelease(actual);
    }
    for (step = 0; step < 4; step++) xuiDocumentSnapshotRelease(snapshots[step]);
    xuiDocumentRelease(d);
    puts("Markdown block versions: repeated fenced-code/list/paragraph shifts, full-load position oracle and Undo/Redo passed");
}
static void incremental_markdown_chain(void)
{
    const char* source = "before **keep**\n\nalpha &amp; **beta**\n\nafter _keep_\n";
    const char* expected = "before **keep**\n\nalpha! &amp; **gamma**\n\nafter _keep_\n";
    xui_document d = test_markdown_open(source), oracle = test_markdown_open(expected);
    xui_document_prepare p, q; xui_doc_source_patch_t patches[2];
    xui_document_snapshot old, actual, full; xui_doc_stats_t a, b; uint64_t first, last, same;
    patches[0] = prepare_patch((uint64_t)(strstr(source, "alpha") - source) + 5, (uint64_t)(strstr(source, "alpha") - source) + 5, "!");
    patches[1] = prepare_patch((uint64_t)(strstr(source, "beta") - source) + 1, (uint64_t)(strstr(source, "beta") - source) + 5, "gamma");
    CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(old, 1, 0, &first) == XUI_OK && xuiDocumentSnapshotGetChild(old, 1, 2, &last) == XUI_OK);
    a = inc_stats(d); CHECK(xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, p, patches + 1, 1, &q) == XUI_OK); xuiDocumentPrepareRelease(p);
    CHECK(xuiDocumentPrepareRun(q) == XUI_OK && xuiDocumentPreparePublish(d, q, NULL) == XUI_OK); xuiDocumentPrepareRelease(q);
    b = inc_stats(d); CHECK(b.iMarkdownIncrementalParses == a.iMarkdownIncrementalParses + 1);
    CHECK(b.iMarkdownParsedBytes - a.iMarkdownParsedBytes < strlen(expected)); test_source(d, expected, 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    inc_snapshot_equal(actual, full);
    CHECK(xuiDocumentSnapshotGetChild(actual, 1, 0, &same) == XUI_OK && same == first);
    CHECK(xuiDocumentSnapshotGetChild(actual, 1, 2, &same) == XUI_OK && same == last);
    xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(full);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    puts("Markdown incremental chain: accumulated source operations, stable outside NodeIds and reduced parsed bytes passed");
}
static void incremental_provenance_versions(void)
{
    const char* versions[] = {
        "head **one**\n\nmiddle &amp; **two**\n\ntail &amp; _three_\n",
        "Xhead **one**\n\nmiddle &amp; **two**\n\ntail &amp; _three_\n",
        "XXhead **one**\n\nmiddle &amp; **two**\n\ntail &amp; _three_\n",
        "head **one**\n\nmiddle &amp; **two**\n\ntail &amp; _three_\n"
    };
    xui_document d = test_markdown_open(versions[0]);
    xui_document_snapshot snapshots[4] = {0}, actual, expected;
    uint64_t tail_id = 0, first_id = 0, source, before, i;
    int mapping;
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[0]) == XUI_OK);
    for (i = 1; i < 4; i++) {
        xui_document_transaction t; xui_document oracle;
        xui_doc_txn_desc_t tx = {0}; xui_doc_position_t p, q;
        uint64_t tail, text;
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        before = inc_stats(d).iMarkdownIncrementalParses;
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, 0, i == 3 ? 2 : 0, i == 3 ? "" : "X", i == 3 ? 0 : 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(inc_stats(d).iMarkdownIncrementalParses == before + 1);
        oracle = test_markdown_open(versions[i]);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[i]) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(snapshots[i], expected);
        {
            uint64_t at, start = (uint64_t)(strstr(versions[i], "tail") - versions[i]);
            unsigned affinity;
            for (at = start; at <= strlen(versions[i]); at++) for (affinity = 0; affinity < 2; affinity++) {
                xui_doc_position_t a, b; int am, bm;
                CHECK(xuiDocumentSourceToPositionEx(snapshots[i], at, affinity, &a, &am) == XUI_OK);
                CHECK(xuiDocumentSourceToPositionEx(expected, at, affinity, &b, &bm) == XUI_OK);
                CHECK(a.iKind == b.iKind && a.iOffset == b.iOffset && am == bm);
                if (a.iKind == XUI_DOC_POSITION_TEXT) {
                    char ax[128], bx[128]; uint64_t ab, bb;
                    CHECK(xuiDocumentSnapshotCopyText(snapshots[i], a.iNodeId, ax, sizeof(ax), &ab) == XUI_OK);
                    CHECK(xuiDocumentSnapshotCopyText(expected, b.iNodeId, bx, sizeof(bx), &bb) == XUI_OK);
                    CHECK(ab == bb && !memcmp(ax, bx, (size_t)ab));
                }
            }
        }
        CHECK(xuiDocumentSnapshotGetChild(snapshots[i], 1, 2, &tail) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshots[i], tail, 0, &text) == XUI_OK);
        if (i == 1) { tail_id = tail; first_id = text; }
        else CHECK(tail == tail_id && text == first_id);
        p = test_position(d, text, XUI_DOC_POSITION_TEXT, 0, XUI_DOC_AFTER);
        CHECK(xuiDocumentPositionToSource(snapshots[i], &p, &source, &mapping) == XUI_OK);
        CHECK(source == (uint64_t)(strstr(versions[i], "tail") - versions[i]) && mapping == XUI_DOC_MAP_EXACT);
        CHECK(xuiDocumentSourceToPositionEx(snapshots[i], source, XUI_DOC_AFTER, &q, &mapping) == XUI_OK);
        CHECK(q.iNodeId == text && q.iOffset == 0 && mapping == XUI_DOC_MAP_EXACT);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    for (i = 3; i > 0; i--) {
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[i - 1]); xuiDocumentSnapshotRelease(actual);
    }
    for (i = 1; i < 4; i++) {
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[i]); xuiDocumentSnapshotRelease(actual);
    }
    {
        const char* edited = "head **one**\n\nmiddle &amp; **two**\n\nTail &amp; _three_\n";
        xui_document_transaction t; xui_document oracle = test_markdown_open(edited);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t, first_id, 0, 1, "T", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(actual, expected);
        inc_snapshot_equal(snapshots[0], snapshots[3]);
        xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    for (i = 0; i < 4; i++) xuiDocumentSnapshotRelease(snapshots[i]);
    xuiDocumentRelease(d);
    puts("Markdown provenance: repeated suffix rebases, retained snapshots, source mapping, semantic rewrite and Undo/Redo passed");
}
static void incremental_syntax_versions(void)
{
    const char* from[] = {"head", "Xhead", "**bold _nested_**", "~~strike~~",
        "tail", "[a](/a)", "XXhead", "plain"};
    const char* to[] = {"Xhead", "XXhead", "plain", "**strong _inner_**",
        "tail-long", "direct", "head", "_fresh_"};
    char versions[9][512] = {"head **bold _nested_** [link](/x)\n\nmid ~~strike~~ `code`\n\ntail **keep _nested_** [a](/a)\n"};
    xui_document d = test_markdown_open(versions[0]);
    xui_document_snapshot snapshots[9] = {0};
    uint64_t i;
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[0]) == XUI_OK);
    for (i = 0; i < 8; i++) {
        xui_doc_txn_desc_t tx = {0}; xui_document_transaction t;
        xui_document oracle; xui_document_snapshot expected;
        const char* match = strstr(versions[i], from[i]);
        size_t prefix, previous, removed = strlen(from[i]), added = strlen(to[i]);
        uint64_t before = inc_stats(d).iMarkdownIncrementalParses;
        CHECK(match); prefix = (size_t)(match - versions[i]); previous = strlen(versions[i]);
        CHECK(previous - removed + added < sizeof(versions[i + 1]));
        memcpy(versions[i + 1], versions[i], prefix);
        memcpy(versions[i + 1] + prefix, to[i], added);
        memcpy(versions[i + 1] + prefix + added, match + removed, previous - prefix - removed + 1);
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, prefix, prefix + removed, to[i], added) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(inc_stats(d).iMarkdownIncrementalParses == before + 1);
        oracle = test_markdown_open(versions[i + 1]);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshots[i + 1]) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(snapshots[i + 1], expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    /* Revisit each retained version after later edits have split and joined
     * the tagged suffix, then check history in both directions. */
    for (i = 0; i < 9; i++) {
        xui_document oracle = test_markdown_open(versions[i]); xui_document_snapshot expected;
        CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(snapshots[i], expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    for (i = 8; i > 0; i--) {
        xui_document_snapshot actual;
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[i - 1]); xuiDocumentSnapshotRelease(actual);
    }
    for (i = 1; i < 9; i++) {
        xui_document_snapshot actual;
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
        inc_snapshot_equal(actual, snapshots[i]); xuiDocumentSnapshotRelease(actual);
    }
    for (i = 0; i < 9; i++) xuiDocumentSnapshotRelease(snapshots[i]);
    xuiDocumentRelease(d);
    puts("Markdown syntax index: repeated source and parent-index shifts, retained snapshots, full-load oracle and Undo/Redo passed");
}
static void incremental_block_shift_fallback(void)
{
    const char* initial = "head **one**\n\nmiddle _two_\n\ntail [a](/a)\n";
    const char* shifted = "Xhead **one**\n\nmiddle _two_\n\ntail [a](/a)\n";
    const char* merged = "Xhead **one**\n\nmiddle _two_\ntail [a](/a)\n";
    const char* branched = "Xhead **one**\n\nmiddle _two_\n\ntail! [a](/a)\n";
    xui_document d = test_markdown_open(initial), oracle;
    xui_document_snapshot versions[4] = {0}, expected, actual;
    xui_doc_txn_desc_t tx = {0}; xui_document_transaction t;
    uint64_t at, before;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentAcquireSnapshot(d, &versions[0]) == XUI_OK);
    before = inc_stats(d).iMarkdownIncrementalParses;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 0, 0, "X", 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); CHECK(inc_stats(d).iMarkdownIncrementalParses == before + 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &versions[1]) == XUI_OK);
    at = (uint64_t)(strstr(shifted, "_two_\n\n") - shifted) + strlen("_two_");
    before = inc_stats(d).iMarkdownIncrementalParses;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 2, "\n", 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); CHECK(inc_stats(d).iMarkdownIncrementalParses == before + 1);
    oracle = test_markdown_open(merged);
    CHECK(xuiDocumentAcquireSnapshot(d, &versions[2]) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(versions[2], expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
    inc_snapshot_equal(actual, versions[1]); xuiDocumentSnapshotRelease(actual);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
    inc_snapshot_equal(actual, versions[2]); xuiDocumentSnapshotRelease(actual);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    at = (uint64_t)(strstr(shifted, "tail") - shifted) + strlen("tail");
    before = inc_stats(d).iMarkdownIncrementalParses;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at, "!", 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); CHECK(inc_stats(d).iMarkdownIncrementalParses == before + 1);
    oracle = test_markdown_open(branched);
    CHECK(xuiDocumentAcquireSnapshot(d, &versions[3]) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(versions[3], expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    oracle = test_markdown_open(initial);
    CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(versions[0], expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    oracle = test_markdown_open(shifted);
    CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(versions[1], expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    xuiDocumentSnapshotRelease(versions[0]); xuiDocumentSnapshotRelease(versions[1]);
    xuiDocumentSnapshotRelease(versions[2]); xuiDocumentSnapshotRelease(versions[3]); xuiDocumentRelease(d);
    puts("Markdown block shifts: local/full fallback, old snapshots, Undo/Redo and branched local edit match full loads");
}
static void incremental_block_shift_failures(void)
{
    const char* initial = "head **one**\n\nmiddle _two_\n\ntail [a](/a)\n";
    const char* shifted = "Xhead **one**\n\nmiddle _two_\n\ntail [a](/a)\n";
    const char* merged = "Xhead **one**\n\nmiddle _two_\ntail [a](/a)\n";
    uint64_t at = (uint64_t)(strstr(shifted, "_two_\n\n") - shifted) + strlen("_two_");
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
        xui_document d, oracle; xui_document_transaction t; xui_document_prepare p = NULL;
        xui_document_snapshot old, actual, expected; xui_doc_source_patch_t patch = prepare_patch(at, at + 2, "\n");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, initial, strlen(initial)) == XUI_OK);
        tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, 0, 0, "X", 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); test_source(d, shifted, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            oracle = test_markdown_open(merged);
            CHECK(notifications == notices + 1 && xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected);
            xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Markdown block-shift full-fallback allocation-failure sweep: %ld points; atomic source/tree/history and no leaks passed\n", point);
}
static void incremental_markdown_failures(void)
{
    const char* source = "before **keep**\n\nalpha &amp; **beta**\n\nafter _keep_\n";
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p = NULL;
        xui_document_snapshot old, actual; xui_doc_source_patch_t patch = prepare_patch(21, 21, "!");
        uint64_t revision, notices = notifications, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            expect_identity_tree(old, actual, 1); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 && inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Markdown incremental allocation-failure sweep: %ld points; atomic source/tree/history/notification and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual; xui_doc_source_patch_t patch = prepare_patch(21, 21, "!"); int result;
        uint64_t revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            expect_identity_tree(old, actual, 1); xuiDocumentSnapshotRelease(actual);
        } else { CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); success = 1; }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success); printf("Markdown incremental cancellation sweep: %ld checkpoints; unchanged published snapshots and no leaks passed\n", point);
}
static void incremental_definition_failures(void)
{
    const char* source = "[ref]: /global \"title\"\n\nalpha target\n\n[ref] and tail\n\n[^note]: unused\n";
    const char* edited = "[ref]: /global \"title\"\n\nalpha longer plain\n\n[ref] and tail\n\n[^note]: unused\n";
    uint64_t at = (uint64_t)(strstr(source, "target") - source);
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_prepare p = NULL; xui_document_snapshot old, actual, expected;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 && inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown global-definition local edit allocation-failure sweep: %ld points; metadata, atomicity and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown global-definition local edit cancellation sweep: %ld checkpoints; published state and no leaks passed\n", point);
}
static void incremental_definition_scale(void)
{
    xui_document d, oracle; xui_document_transaction t;
    xui_document_snapshot actual, expected; xui_doc_txn_desc_t tx = {0};
    xui_doc_stats_t before, after; char *source = malloc(160000), *edited;
    size_t used = 0, at, i, added = strlen("longer plain");
    CHECK(source);
    used += (size_t)snprintf(source + used, 160000 - used, "[ref]: /global\n\n");
    for (i = 0; i < 1500; i++) {
        int written = snprintf(source + used, 160000 - used,
            i == 750 ? "paragraph %zu plain target with repeated text and no local reference\n\n" :
                "paragraph %zu plain body with repeated text and no local reference\n\n", i);
        CHECK(written > 0 && (size_t)written < 160000 - used); used += (size_t)written;
    }
    used += (size_t)snprintf(source + used, 160000 - used, "[ref]\n\n[^note]: unused\n");
    at = (size_t)(strstr(source, "target") - source);
    edited = malloc(used - 6 + added + 1); CHECK(edited);
    memcpy(edited, source, at); memcpy(edited + at, "longer plain", added);
    memcpy(edited + at + added, source + at + 6, used - at - 6 + 1);
    d = test_markdown_open(source); before = inc_stats(d);
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 6, "longer plain", added) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
    CHECK(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 4096);
    oracle = test_markdown_open(edited);
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle); free(source); free(edited);
    printf("Markdown global-definition scale: %zu source bytes, %llu parsed bytes for a middle-block edit; full-load oracle passed\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
}
static void incremental_reference_budget(void)
{
    char* source = malloc(180000); size_t used = 0, i, at;
    CHECK(source);
    memcpy(source + used, "[r]: /", 6); used += 6;
    memset(source + used, 'a', 2048); used += 2048;
    memcpy(source + used, "\n\n", 2); used += 2;
    for (i = 0; i < 96; i++) { memcpy(source + used, "[r] ", 4); used += 4; }
    at = used; memcpy(source + used, "target\n\n", 8); used += 8;
    for (i = 0; i < 8000; i++) { memcpy(source + used, "padding\n\n", 9); used += 9; }
    source[used] = 0;
    CHECK(used < 180000 && inc_case(source, XUI_MD_GFM, at, 6, "longer", 0) == 1);
    free(source);
    puts("Markdown local reference expansion budget: a larger full document retains the full parser result");
}
static void incremental_global_reference_budget(void)
{
    char* source = malloc(180000); size_t used = 0, i, at;
    CHECK(source);
    memcpy(source + used, "[r]: /", 6); used += 6;
    memset(source + used, 'a', 2048); used += 2048;
    memcpy(source + used, "\n\n", 2); used += 2;
    for (i = 0; i < 520; i++) { memcpy(source + used, "[r] ", 4); used += 4; }
    memcpy(source + used, "\n\n", 2); used += 2;
    for (i = 0; i < 8000; i++) { memcpy(source + used, "padding\n\n", 9); used += 9; }
    memcpy(source + used, "[r] ", 4); used += 4;
    at = used; memcpy(source + used, "target\n\nlast\n", 13); used += 13;
    source[used] = 0;
    CHECK(used < 180000 && inc_case(source, XUI_MD_GFM, at, 6, "longer", 0) == 1);
    free(source);
    puts("Markdown global reference expansion budget: earlier links can exhaust the full parser before a local block");
    {
        char* value_source = malloc(10000); char replacement[52]; size_t n = 0;
        CHECK(value_source);
        memcpy(value_source + n, "[u]: /x\n\n[a]: /", 15); n += 15;
        memset(value_source + n, 'a', 1024); n += 1024;
        memcpy(value_source + n, "\n\n", 2); n += 2;
        for (i = 0; i < 20; i++) { memcpy(value_source + n, "[a] ", 4); n += 4; }
        memcpy(value_source + n, "\n", 1); n++;
        value_source[n] = 0; replacement[0] = '/'; memset(replacement + 1, 'q', 50); replacement[51] = 0;
        CHECK(!inc_case(value_source, XUI_MD_GFM, 5, 2, replacement, 1));
        free(value_source);
    }
    puts("Markdown definition length and full-document reference budget: distant links preserve full-parser semantics");
}
static void incremental_external_definition_scale(void)
{
    char* source = malloc(160000); size_t used = 0, at, i;
    xui_document d; xui_document_transaction t; xui_doc_txn_desc_t tx = {0};
    xui_doc_stats_t before, after;
    CHECK(source);
    used += (size_t)snprintf(source + used, 160000 - used, "[ref]: /global\n\n");
    for (i = 0; i < 1500; i++) {
        int written = snprintf(source + used, 160000 - used,
            i == 750 ? "paragraph %zu [ref] target with repeated text\n\n" :
                "paragraph %zu plain body with repeated text and no local reference\n\n", i);
        CHECK(written > 0 && (size_t)written < 160000 - used); used += (size_t)written;
    }
    at = (size_t)(strstr(source, "target") - source);
    CHECK(inc_case(source, XUI_MD_GFM, at, 6, "longer plain", 1) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 6, "longer plain", 12) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 4096);
    printf("Markdown external link-definition scale: %zu source bytes, %llu parsed bytes for a linked middle block\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d);
    CHECK(inc_case(source, XUI_MD_GFM, 7, 7, "/otherx", 0) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 7, 14, "/otherx", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 4096);
    printf("Markdown single-block definition change scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d);
    CHECK(inc_case(source, XUI_MD_GFM, 7, 7, "/much-longer", 1) == 1);
    d = test_markdown_open(source); before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 7, 14, "/much-longer", 12) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 4096);
    printf("Markdown single-block variable-length definition scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
static void incremental_external_definition_failures(void)
{
    const char* source = "[ref]: /global \"title\"\n\nalpha [ref] target\n\ntail\n";
    const char* edited = "[ref]: /global \"title\"\n\nalpha [ref] longer plain\n\ntail\n";
    uint64_t at = (uint64_t)(strstr(source, "target") - source);
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_prepare p = NULL; xui_document_snapshot old, actual, expected;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 && inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown external link-definition allocation-failure sweep: %ld points; atomic metadata and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK); a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision); test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK); inc_snapshot_equal(old, actual);
            xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown external link-definition cancellation sweep: %ld checkpoints; published state and no leaks passed\n", point);
}
static void incremental_independent_footnote_failures(void)
{
    const char* source = "before[^n]\n\n[^n]: note\n\nalpha [ref] target\n\n[ref]: /u\n\ntail\n";
    const char* edited = "before[^n]\n\n[^n]: note\n\nalpha [ref] longer plain\n\n[ref]: /u\n\ntail\n";
    uint64_t at = (uint64_t)(strstr(source, "target") - source);
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_prepare p = NULL; xui_document_snapshot old, actual, expected;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK &&
            xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 &&
                inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected);
            xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown independent footnote allocation-failure sweep: %ld points; atomic state and no leaks passed\n", point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL};
        xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 6, "longer plain");
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free;
        desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
        a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision);
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p);
        xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown independent footnote cancellation sweep: %ld checkpoints; published state and no leaks passed\n", point);
}
static void incremental_footnote_body_failures(const char* source, const char* edited,
    uint64_t at, const char* kind)
{
    long point; int success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_prepare p = NULL; xui_document_snapshot old, actual, expected;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 4, "longer note");
        uint64_t revision, notices, token; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, old) == XUI_OK &&
            xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
        revision = xuiDocumentGetRevision(d); notices = notifications; a.remaining = point;
        result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
        if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
        if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
        a.remaining = -1;
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                xuiDocumentGetRevision(d) == revision && notifications == notices);
            CHECK(!xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(notifications == notices + 1 &&
                inc_stats(d).iMarkdownIncrementalParses == 1);
            oracle = test_markdown_open(edited);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            inc_snapshot_equal(actual, expected);
            xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected);
            xuiDocumentRelease(oracle); success = 1;
        }
        xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown %s footnote allocation-failure sweep: %ld points; atomic state and no leaks passed\n", kind, point);
    success = 0;
    for (point = 0; point < 1500 && !success; point++) {
        prepare_cancel_allocator a = {0, -1, 0, NULL};
        xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_prepare p; xui_document_snapshot old, actual;
        xui_doc_source_patch_t patch = prepare_patch(at, at + 4, "longer note");
        uint64_t revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free;
        desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
        a.prepare = p; a.remaining = point;
        result = xuiDocumentPrepareRun(p); a.prepare = NULL;
        if (a.fired) {
            CHECK(result == XUI_DOC_ERROR_CANCELLED &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED);
            CHECK(xuiDocumentGetRevision(d) == revision);
            test_source(d, source, 1);
            CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK);
            inc_snapshot_equal(old, actual); xuiDocumentSnapshotRelease(actual);
        } else {
            CHECK(result == XUI_OK &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            CHECK(inc_stats(d).iMarkdownIncrementalParses == 1); success = 1;
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentPrepareRelease(p);
        xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(success);
    printf("Markdown %s footnote cancellation sweep: %ld checkpoints; published state and no leaks passed\n", kind, point);
}
static void incremental_unused_footnote_failures(void)
{
    const char* source = "alpha [ref]\n\n[ref]: /u\n\n[^n]: note\n\nlast\n";
    const char* edited = "alpha [ref]\n\n[ref]: /u\n\n[^n]: longer note\n\nlast\n";
    incremental_footnote_body_failures(source, edited,
        (uint64_t)(strstr(source, "note") - source), "unused");
}
static void incremental_used_footnote_failures(void)
{
    const char* source = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] note\n\nlast\n";
    const char* edited = "before[^n]\n\n[ref]: /u\n\n[^n]: **bold** [ref] longer note\n\nlast\n";
    incremental_footnote_body_failures(source, edited,
        (uint64_t)(strstr(source, "note") - source), "used");
}
static void incremental_unused_footnote_scale(void)
{
    size_t capacity = 180000, used = 0; char* source = malloc(capacity);
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
    xui_document d; xui_document_transaction t; xui_doc_stats_t before, after;
    uint64_t at; unsigned i; int written;
    CHECK(source);
    written = snprintf(source, capacity, "alpha [ref]\n\n[ref]: /u\n\n");
    CHECK(written > 0 && (size_t)written < capacity); used = (size_t)written;
    for (i = 0; i < 1800; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u contains ordinary text.\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    written = snprintf(source + used, capacity - used, "[^n]: plain body\n\n");
    CHECK(written > 0 && (size_t)written < capacity - used);
    at = used + (uint64_t)(strstr(source + used, "body") - (source + used));
    used += (size_t)written;
    for (i = 1800; i < 3600; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u contains ordinary text.\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    CHECK(used > 100000);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.bDisableHistory = 1;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
    before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 4, "longer body", 11) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 128);
    printf("Markdown unused footnote scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
static void incremental_used_footnote_scale(void)
{
    size_t capacity = 1000000, used = 0; char* source = malloc(capacity);
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
    xui_document d, oracle; xui_document_transaction t;
    xui_document_snapshot snap, expected;
    xui_doc_source_info_t info = {0}; xui_doc_stats_t before, after;
    uint64_t at, syntax_count; unsigned i; int written; clock_t begin;
    CHECK(source);
    written = snprintf(source, capacity, "alpha [ref] note[^n]\n\n[ref]: /u\n\n");
    CHECK(written > 0 && (size_t)written < capacity); used = (size_t)written;
    for (i = 0; i < 9000; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u with **bold** and [ref].\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    written = snprintf(source + used, capacity - used, "[^n]: **bold** [ref] body\n\n");
    CHECK(written > 0 && (size_t)written < capacity - used);
    at = used + (uint64_t)(strstr(source + used, "body") - (source + used));
    used += (size_t)written;
    for (i = 9000; i < 18000; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u with **bold** and [ref].\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    CHECK(used > 800000);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.bDisableHistory = 1;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetSourceInfo(snap, &info) == XUI_OK);
    syntax_count = info.iInlineSyntaxCount;
    xuiDocumentSnapshotRelease(snap);
    before = inc_stats(d);
    begin = clock();
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 4, "longer body", 11) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 128 &&
        after.iCurrentBytes < before.iCurrentBytes + 65536);
    printf("Markdown used footnote scale: %zu source bytes, %llu syntax, %llu parsed bytes, %.2f ms, %llu allocs, %+lld current bytes\n",
        used, (unsigned long long)syntax_count,
        (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes),
        1000.0 * (clock() - begin) / CLOCKS_PER_SEC,
        (unsigned long long)(after.iAllocations - before.iAllocations),
        (long long)after.iCurrentBytes - (long long)before.iCurrentBytes);
    memmove(source + at + 11, source + at + 4, used - (size_t)at - 4);
    memcpy(source + at, "longer body", 11); used += 7; source[used] = 0;
    CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK &&
        xuiDocumentLoadMarkdown(oracle, source, used) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(snap, expected);
    xuiDocumentSnapshotRelease(snap); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(oracle);
    xuiDocumentRelease(d); free(source);
}
static void incremental_independent_footnote_scale(void)
{
    size_t capacity = 180000, used = 0; char* source = malloc(capacity);
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
    xui_document d; xui_document_transaction t; xui_doc_stats_t before, after;
    uint64_t at; unsigned i;
    int written;
    CHECK(source);
    written = snprintf(source, capacity, "before[^n]\n\n[^n]: note\n\n");
    CHECK(written > 0 && (size_t)written < capacity); used = (size_t)written;
    for (i = 0; i < 1800; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u contains ordinary text.\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    written = snprintf(source + used, capacity - used, "alpha [ref] target\n\n");
    CHECK(written > 0 && (size_t)written < capacity - used);
    at = used + (uint64_t)(strstr(source + used, "target") - (source + used));
    used += (size_t)written;
    for (i = 1800; i < 3600; i++) {
        written = snprintf(source + used, capacity - used,
            "padding block %04u contains ordinary text.\n\n", i);
        CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    }
    written = snprintf(source + used, capacity - used, "[ref]: /u\n");
    CHECK(written > 0 && (size_t)written < capacity - used); used += (size_t)written;
    CHECK(used > 100000);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.bDisableHistory = 1;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
    before = inc_stats(d);
    CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, at, at + 6, "longer plain", 12) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); after = inc_stats(d);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 512);
    printf("Markdown independent footnote scale: %zu source bytes, %llu parsed bytes\n",
        used, (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    xuiDocumentRelease(d); free(source);
}
