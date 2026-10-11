#define XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY
#include "xui_document_implicit_reference_split_diagnostic_test.c"
#undef XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY

/* Every edited body and selected definition spelling below is an independent
 * source golden. Framing expands only containers, BOM and physical endings. */
static const struct implicit_format {
    const char *before, *split, *replace, *definitions, *target, *head;
    uint64_t split_at, replace_end;
} implicit_formats[] = {
    {"[abcd] &amp;\n", "[ab][abcd]\n\n[cd][abcd] &amp;\n",
     "[aX][abcd]\n\n[Yd][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "[ab]", 2, 3},
    {"[abcd][] &amp;\n", "[ab][abcd]\n\n[cd][abcd] &amp;\n",
     "[aX][abcd]\n\n[Yd][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "[ab]", 2, 3},
    {"__[abcd]__ &amp;\n", "__[ab][abcd]__\n\n__[cd][abcd]__ &amp;\n",
     "__[aX][abcd]__\n\n__[Yd][abcd]__ &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "[ab]", 2, 3},
    {"[__abcd__][] &amp;\n", "[__ab__][__abcd__]\n\n[__cd__][__abcd__] &amp;\n",
     "[__aX__][__abcd__]\n\n[__Yd__][__abcd__] &amp;\n", "[__abcd__]: <raw> 'Title'\n", "abcd", "[__ab__]", 2, 3},
    {"[ABCD][] &amp;\n", "[AB][abcd]\n\n[CD][abcd] &amp;\n",
     "[AX][abcd]\n\n[YD][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n[ABCD]: <ignored> 'Unused'\n", "ABCD", "[AB]", 2, 3},
    {"[abcd][] &amp;\n", "[ab][abcd]\n\n[cd][abcd] &amp;\n",
     "[aX][abcd]\n\n[Yd][abcd] &amp;\n", "[ \tabcd \t]: <raw> 'Title'\n", "abcd", "[ab]", 2, 3},
    {"[ab cd][] &amp;\n", "[ab][ab cd]\n\n[ cd][ab cd] &amp;\n",
     "[aX][ab cd]\n\n[Ycd][ab cd] &amp;\n", "[ab\ncd]: <raw> 'Title'\n", "ab cd", "[ab]", 2, 3},
    {"[ab\ncd][] &amp;\n", "[a][ab cd]\n\n[b\ncd][ab cd] &amp;\n",
     "[aX][ab cd]\n\n[Y\ncd][ab cd] &amp;\n", "[ab cd]: <raw> 'Title'\n", "ab", "[a]", 1, 2},
    {"[abcd][] &amp;\n", "[ab][abcd]\n\n[cd][abcd] &amp;\n",
     "[aX][abcd]\n\n[Yd][abcd] &amp;\n", "[\xe2\x80\x83" "abcd\xc2\xa0]: <raw> 'Title'\n", "abcd", "[ab]", 2, 3},
    {"[stra\xc3\x9f" "e][] &amp;\n", "[st][STRASSE]\n\n[ra\xc3\x9f" "e][STRASSE] &amp;\n",
     "[sX][STRASSE]\n\n[Ya\xc3\x9f" "e][STRASSE] &amp;\n", "[STRASSE]: <raw> 'Title'\n", "stra\xc3\x9f" "e", "[st]", 2, 3}
};

static char* implicit_global_definitions(char* source, const char* definitions,
    const char* ending, int final)
{
    char* expanded = delete_ending(definitions, ending, 0);
    size_t bytes = strlen(source), d = strlen(expanded), e = strlen(ending);
    char* result = malloc(bytes + 2 * e + d + 1);
    TABLE_CHECK(result && d >= e);
    if (!final) { d -= e; expanded[d] = 0; }
    memcpy(result, source, bytes); memcpy(result + bytes, ending, e);
    memcpy(result + bytes + e, ending, e); memcpy(result + bytes + 2 * e, expanded, d + 1);
    free(expanded); free(source); return result;
}

static void implicit_inserted_separator(char* source, const char* head, const char* ending)
{
    char *content = strstr(source, head), *first, *blank, *end, *trim;
    TABLE_CHECK(content);
    first = strstr(content, ending); TABLE_CHECK(first);
    blank = first + strlen(ending); end = strstr(blank, ending); TABLE_CHECK(end);
    for (trim = blank; trim < end; trim++) TABLE_CHECK(*trim == '>' || *trim == ' ' || *trim == '\t');
    trim = end;
    while (trim > blank && (trim[-1] == ' ' || trim[-1] == '\t')) trim--;
    memmove(trim, end, strlen(end) + 1);
}

static xui_doc_range_t implicit_selection(xui_document_snapshot snapshot, const char* source,
    const char* target, uint64_t left, uint64_t right, int reverse)
{
    xui_doc_range_t range = {0}; xui_doc_node_info_t node = {0}; int quality;
    const char* content = strstr(source, target); TABLE_CHECK(content);
    TABLE_CHECK(xuiDocumentSourceToPosition(snapshot, (uint64_t)(content - source) + left,
        &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && range.tAnchor.iKind == XUI_DOC_POSITION_TEXT);
    TABLE_CHECK(xuiDocumentSourceToPosition(snapshot, (uint64_t)(content - source) + right,
        &range.tCaret, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && range.tCaret.iKind == XUI_DOC_POSITION_TEXT &&
        range.tAnchor.iNodeId == range.tCaret.iNodeId);
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, range.tAnchor.iNodeId, &node) == XUI_OK &&
        node.iKind == XUI_DOC_TEXT && (node.tAttributes.iMarks & XUI_DOC_LINK));
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}

static int direct_split_command(xui_document document, const xui_doc_position_t* at,
    xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction transaction;
    int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnSplitBlock(transaction, at, caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, change);
    else TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
    xuiDocumentTxnRelease(transaction);
    return result;
}

static void implicit_golden(const char* before, const char* after, const char* target,
    uint64_t left, uint64_t right, const char* replacement, int reverse, int direct)
{
    xui_document document = table_open(before, NULL), oracle = table_open(after, NULL);
    xui_document_snapshot old, current, expected, restored; xui_document_change_set change;
    xui_doc_range_t range; xui_doc_position_t caret = {0}, tail, mapped;
    boundary_case operation = {before, after, replacement, 2, left, right}; uint64_t offset; int quality;
    TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
        xuiDocumentMarkSaved(document, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    range = implicit_selection(old, before, target, left, right, reverse);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(boundary_tail(before) - before) + 1,
        &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    TABLE_CHECK((direct ? direct_split_command(document, &range.tCaret, &caret, &change) : boundary_command(document, &range, &operation, &caret, &change)) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
    table_source(current, after); integration_equivalent(current, after); boundary_caret(document, current, &caret);
    formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
    TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
        xuiDocumentPositionToSource(current, &mapped, &offset, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        offset == (uint64_t)(boundary_tail(after) - after) + 1);
    TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document) &&
        xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
    table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
    table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
    table_source(old, before); table_source(current, after); xuiDocumentChangeSetRelease(change);
    xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(oracle); xuiDocumentRelease(document);
}

static void implicit_faults(void)
{
    unsigned mode;
    for (mode = 0; mode < 2; mode++) {
        const struct implicit_case* c = &implicit_cases[mode ? 5 : 3];
        const char* selected_after = "[aX][ab cd]\n\n[Y\ncd][ab cd] &amp;\n\n[ab cd]: <raw> 'Title'\n\nend &amp;\n";
        char *before = delete_ending(c->before, "\r\n", 1),
             *after = delete_ending(mode ? selected_after : c->after, "\r\n", 1);
        boundary_case operation = {before, after, mode ? "X\r\nY" : "\r\n", 2, c->offset, mode ? 2 : c->offset}; long point;
        for (point = 0; point < 4096; point++) {
            table_allocator allocator = {-1, 0}; xui_document document = table_open(before, &allocator);
            xui_document_snapshot old, current, restored; xui_doc_range_t range;
            xui_doc_position_t caret = {0}; uint64_t revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            range = implicit_selection(old, before, mode ? "ab" : c->target, c->offset, mode ? 2 : c->offset, (int)mode);
            revision = xuiDocumentGetRevision(document); allocator.remaining = point;
            result = boundary_command(document, &range, &operation, &caret, NULL); allocator.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(boundary_command(document, &range, &operation, &caret, NULL) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(document) == revision + 1 && xuiDocumentIsDirty(document) &&
                xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(document, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
            table_source(current, after); table_source(old, before); xuiDocumentSnapshotRelease(current);
            xuiDocumentSnapshotRelease(old); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Implicit reference mode %u: %ld OOM points atomic/poisoned commit/same-document retry/one Undo/zero leaks\n", mode, point);
        free(before); free(after);
    }
}

/* The same command correction must apply to Rich documents. Build concrete
 * styled runs with each break type; both direct SplitBlock and a replacement
 * ending at the first Text run must retain the break's format and link fields. */
static void implicit_rich_breaks(void)
{
    unsigned kind, linked, operation, reverse, total = 0;
    for (kind = 0; kind < 2; kind++) for (linked = 0; linked < 2; linked++)
        for (operation = 0; operation < 2; operation++) for (reverse = 0; reverse < 2; reverse++) {
            table_allocator allocator = {-1, 0}; xui_doc_desc_t options = {0}; xui_doc_node_desc_t desc = {0};
            xui_document document; xui_document_transaction transaction; xui_document_snapshot old, current, restored;
            xui_doc_node_info_t info = {0}; xui_doc_range_t range = {0}; xui_doc_position_t caret;
            uint64_t paragraph, text, newline, suffix, bytes, child; char raw[16];
            options.iSize = sizeof(options); options.iProfile = XUI_DOCUMENT_RICH;
            options.onAlloc = table_alloc; options.onFree = table_free; options.pAllocatorUser = &allocator;
            TABLE_CHECK(xuiDocumentCreate(&options, &document) == XUI_OK &&
                xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
            TABLE_CHECK(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, UINT64_MAX, &desc, &paragraph) == XUI_OK);
            desc.iKind = XUI_DOC_TEXT; desc.tAttributes.iMarks = XUI_DOC_BOLD | (linked ? XUI_DOC_LINK : 0);
            desc.sResource = linked ? "/raw" : ""; desc.sTitle = linked ? "Title" : "";
            desc.sText = "ab"; desc.iTextBytes = 2;
            TABLE_CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, UINT64_MAX, &desc, &text) == XUI_OK);
            desc.iKind = kind ? XUI_DOC_HARD_BREAK : XUI_DOC_SOFT_BREAK; desc.sText = ""; desc.iTextBytes = 0;
            TABLE_CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, UINT64_MAX, &desc, &newline) == XUI_OK);
            desc.iKind = XUI_DOC_TEXT; desc.sText = "cd"; desc.iTextBytes = 2;
            TABLE_CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, UINT64_MAX, &desc, &suffix) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
            TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(document);
            range.tAnchor.iRevision = xuiDocumentGetRevision(document); range.tAnchor.iNodeId = text;
            range.tAnchor.iKind = XUI_DOC_POSITION_TEXT; range.tAnchor.iOffset = operation ? 1 : 2;
            range.tCaret = range.tAnchor; range.tCaret.iOffset = 2;
            if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
            if (operation) TABLE_CHECK(xuiDocumentTxnReplaceRange(transaction, &range, "X\nY", 3, &caret) == XUI_OK);
            else {
                TABLE_CHECK(xuiDocumentTxnSplitBlock(transaction, &range.tCaret, &caret) == XUI_OK);
                TABLE_CHECK(xuiDocumentTxnReplaceText(transaction, caret.iNodeId, 0, 0, "Y", 1) == XUI_OK);
                caret.iOffset = 1;
            }
            TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            info.iSize = sizeof(info);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &info) == XUI_OK && info.iKind == XUI_DOC_TEXT &&
                info.tAttributes.iMarks == desc.tAttributes.iMarks && !strcmp(info.sResource, desc.sResource) &&
                !strcmp(info.sTitle, desc.sTitle) && xuiDocumentSnapshotCopyText(current, caret.iNodeId, raw, sizeof(raw), &bytes) == XUI_OK &&
                bytes == 1 && raw[0] == 'Y');
            TABLE_CHECK(xuiDocumentSnapshotGetChild(current, info.iParentId, 1, &child) == XUI_OK && child == newline &&
                xuiDocumentSnapshotGetChild(current, info.iParentId, 2, &child) == XUI_OK && child == suffix);
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            formatted_split_same_semantics(restored, XUI_DOCUMENT_ROOT, old, XUI_DOCUMENT_ROOT); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            formatted_split_same_semantics(restored, XUI_DOCUMENT_ROOT, current, XUI_DOCUMENT_ROOT); xuiDocumentSnapshotRelease(restored); boundary_caret(document, current, &caret);
            xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(document);
            TABLE_CHECK(!allocator.live); total++;
        }
    printf("Rich formatted break split: %u direct/replacement/soft/hard/link/history/zero-leak goldens passed\n", total);
}

static char* local_implicit_wrap(const char* body,const char* definitions,const char* ending,int bom,int final)
{
    size_t a=strlen(body),b=strlen(definitions);char* joined=malloc(a+b+2),*result;TABLE_CHECK(joined);
    memcpy(joined,body,a);joined[a]='\n';memcpy(joined+a+1,definitions,b+1);
    result=table_wrap(joined,4,ending,bom,final);free(joined);return result;
}
static void local_implicit_matrix(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,operation,eol,bom,final,reverse,total=0;
    for(fixture=0;fixture<sizeof(implicit_formats)/sizeof(*implicit_formats);fixture++)
        for(operation=0;operation<3;operation++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++)for(reverse=0;reverse<2;reverse++){
        const struct implicit_format* f=&implicit_formats[fixture];char replacement[8];
        char* before=local_implicit_wrap(f->before,f->definitions,endings[eol],bom,final);
        char* after=local_implicit_wrap(operation==1?f->replace:f->split,f->definitions,endings[eol],bom,final);
        implicit_inserted_separator(after,operation==1?"X":f->head,endings[eol]);
        snprintf(replacement,sizeof(replacement),operation==1?"X%sY":"%s",endings[eol]);
        printf("Local implicit fixture=%u operation=%u EOL=%u BOM=%u EOF=%u reverse=%u\n",fixture,operation,eol,bom,final,reverse);fflush(stdout);
        implicit_golden(before,after,f->target,operation==1?1:f->split_at,operation==1?f->replace_end:f->split_at,replacement,reverse,operation==2);
        free(before);free(after);total++;
    }
    TABLE_CHECK(total==720);printf("Footnote local implicit split matrix: %u direct SplitBlock/ReplaceRange/newline replacement/source/semantic/position/history goldens passed\n",total);
}

static void local_direct_split_faults(void)
{
    const char* ending="\r\n";unsigned fixture;
    for(fixture=0;fixture<2;fixture++){
        const struct implicit_format* f=&implicit_formats[fixture?7:0];
        char* before=local_implicit_wrap(f->before,f->definitions,ending,1,1);
        char* after=local_implicit_wrap(f->split,f->definitions,ending,1,1);long point;
        implicit_inserted_separator(after,f->head,ending);
        for(point=0;point<4096;point++){
            table_allocator allocator={-1,0};xui_document document=table_open(before,&allocator);
            xui_document_snapshot old,current,restored;xui_doc_range_t range;xui_doc_position_t caret;
            uint64_t revision;int result;
            TABLE_CHECK(xuiDocumentClearHistory(document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&old)==XUI_OK && xuiDocumentMarkSaved(document,old)==XUI_OK);
            range=implicit_selection(old,before,f->target,f->split_at,f->split_at,fixture);
            revision=xuiDocumentGetRevision(document);allocator.remaining=point;
            result=direct_split_command(document,&range.tCaret,&caret,NULL);allocator.remaining=-1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document,&current)==XUI_OK);
            if(result!=XUI_OK){
                TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(document)==revision && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                table_source(current,before);table_equivalent(current,old);
                TABLE_CHECK(direct_split_command(document,&range.tCaret,&caret,NULL)==XUI_OK);
                table_source(current,before);xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document,&current)==XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(document)==revision+1 && xuiDocumentIsDirty(document) && xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
            table_source(current,after);integration_equivalent(current,after);boundary_caret(document,current,&caret);
            TABLE_CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document) && xuiDocumentAcquireSnapshot(document,&restored)==XUI_OK);
            table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(document,&restored)==XUI_OK);
            table_source(restored,after);integration_equivalent(restored,after);xuiDocumentSnapshotRelease(restored);
            table_source(old,before);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(document);TABLE_CHECK(!allocator.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Footnote direct SplitBlock fixture %u: %ld OOM points atomic/poisoned commit/same-document retry/one Undo/zero leaks\n",fixture,point);
        free(before);free(after);
    }
}

static char* continuation_wrap(const char* body, const char* definitions,
    int local, const char* ending, int bom, int final)
{
    char *result, *expanded;
    size_t a = strlen(body), b = strlen(definitions), e = strlen(ending), n;
    if (local) {
        char* joined = malloc(a + b + 2); TABLE_CHECK(joined);
        memcpy(joined, body, a); joined[a] = '\n'; memcpy(joined + a + 1, definitions, b + 1);
        result = table_wrap(joined, 4, ending, bom, final); free(joined);
    } else {
        char* framed = table_wrap(body, 0, ending, bom, final);
        expanded = delete_ending(definitions, ending, 0); n = strlen(expanded);
        a = strlen(framed); TABLE_CHECK(n >= e);
        if (!final) { n -= e; expanded[n] = 0; }
        result = malloc(a + 2 * e + n + 1); TABLE_CHECK(result);
        memcpy(result, framed, a); memcpy(result + a, ending, e);
        memcpy(result + a + e, ending, e); memcpy(result + a + 2 * e, expanded, n + 1);
        free(framed); free(expanded);
    }
    return result;
}

static void continuation_separator(char* source, const char* head, const char* ending)
{
    char *p = strstr(source, head), *blank, *end, *trim;
    TABLE_CHECK(p); p = strstr(p, ending); TABLE_CHECK(p);
    blank = p + strlen(ending); end = strstr(blank, ending); TABLE_CHECK(end);
    for (trim = blank; trim < end; trim++) TABLE_CHECK(*trim == ' ' || *trim == '\t');
    memmove(blank, end, strlen(end) + 1);
}

static void direct_continuation_goldens(void)
{
    const struct {
        const char *before, *split, *continued, *definitions, *target, *head, *right, *typed;
        uint64_t offset; uint32_t marks;
    } cases[] = {
        {"[abcd] &amp;\n", "[ab][abcd]\n\n[cd][abcd] &amp;\n",
         "[ab][abcd]\n\n[Zcd][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n",
         "abcd", "[ab]", "[cd]", "[Zcd]", 2, XUI_DOC_LINK},
        {"__[ab\ncd][]__ &amp;\n", "__[a][ab cd]__\n\n__[b\ncd][ab cd]__ &amp;\n",
         "__[a][ab cd]__\n\n__[Zb\ncd][ab cd]__ &amp;\n", "[ab cd]: <raw> 'Title'\n",
         "ab", "[a]", "[b", "[Zb", 1, XUI_DOC_LINK | XUI_DOC_BOLD}
    };
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, local, eol, bom, final, compound, total = 0;
    for (fixture = 0; fixture < sizeof(cases)/sizeof(*cases); fixture++)
        for (local = 0; local < 2; local++) for (eol = 0; eol < 3; eol++)
            for (bom = 0; bom < 2; bom++) for (final = 0; final < 2; final++) for (compound = 0; compound < 2; compound++) {
                char* before = continuation_wrap(cases[fixture].before, cases[fixture].definitions, local, endings[eol], bom, final);
                char* split = continuation_wrap(cases[fixture].split, cases[fixture].definitions, local, endings[eol], bom, final);
                char* continued = continuation_wrap(cases[fixture].continued, cases[fixture].definitions, local, endings[eol], bom, final);
                table_allocator allocator = {-1, 0}; xui_document d = table_open(before, &allocator);
                xui_document_snapshot old, first, current, restored; xui_document_transaction transaction;
                xui_doc_position_t at, caret; xui_doc_node_info_t info = {0}; uint64_t revision, source; int quality;
                continuation_separator(split, cases[fixture].head, endings[eol]);
                continuation_separator(continued, cases[fixture].head, endings[eol]);
                TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
                TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(strstr(before, cases[fixture].target) - before) + cases[fixture].offset,
                    &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at.iKind == XUI_DOC_POSITION_TEXT);
                revision = xuiDocumentGetRevision(d);
                TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &transaction) == XUI_OK && xuiDocumentTxnSplitBlock(transaction, &at, &caret) == XUI_OK);
                if (compound) TABLE_CHECK(xuiDocumentTxnReplaceText(transaction, caret.iNodeId, caret.iOffset, caret.iOffset, "Z", 1) == XUI_OK);
                TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &first) == XUI_OK);
                table_source(first, compound ? continued : split); integration_equivalent(first, compound ? continued : split);
                caret.iRevision = xuiDocumentGetRevision(d);
                TABLE_CHECK(xuiDocumentPositionToSource(first, &caret, &source, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                    source == (uint64_t)(strstr(compound ? continued : split, compound ? cases[fixture].typed : cases[fixture].right) - (compound ? continued : split)) + 1);
                if (!compound) {
                    TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &transaction) == XUI_OK &&
                        xuiDocumentTxnReplaceText(transaction, caret.iNodeId, caret.iOffset, caret.iOffset, "Z", 1) == XUI_OK &&
                        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
                }
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK && xuiDocumentGetRevision(d) == revision + (compound ? 1 : 2));
                table_source(current, continued); integration_equivalent(current, continued);
                caret.iRevision = xuiDocumentGetRevision(d); info.iSize = sizeof(info);
                TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &info) == XUI_OK && info.iKind == XUI_DOC_TEXT &&
                    info.tAttributes.iMarks == cases[fixture].marks && !strcmp(info.sResource, "raw") && !strcmp(info.sTitle, "Title") &&
                    xuiDocumentPositionToSource(current, &caret, &source, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                    source == (uint64_t)(strstr(continued, cases[fixture].typed) - continued) + 1);
                TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                table_source(restored, compound ? before : split); xuiDocumentSnapshotRelease(restored);
                if (!compound) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
                TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
                TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
                if (!compound) {
                    TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); table_source(restored, split); xuiDocumentSnapshotRelease(restored);
                    TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
                }
                TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                table_source(restored, continued); integration_equivalent(restored, continued); xuiDocumentSnapshotRelease(restored);
                table_source(old, before); table_source(first, compound ? continued : split); table_source(current, continued);
                xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(first); xuiDocumentSnapshotRelease(old);
                xuiDocumentRelease(d); TABLE_CHECK(!allocator.live); free(before); free(split); free(continued); total++;
            }
    TABLE_CHECK(total == 96);
    printf("Direct SplitBlock continuation: %u exact caret/same-draft typing/post-publish typing/format/link/source-index/shared-history/zero-leak goldens passed\n", total);
}

int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, operation, context, ending, bom, final, reverse, total = 0;
    for (fixture = 0; fixture < sizeof(implicit_formats)/sizeof(*implicit_formats); fixture++)
        for (operation = 0; operation < 3; operation++) for (context = 0; context < 5; context++)
            for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++)
                for (final = 0; final < 2; final++) for (reverse = 0; reverse < 2; reverse++) {
                    const struct implicit_format* f = &implicit_formats[fixture]; char replacement[8];
                    char* before = table_wrap(f->before, context, endings[ending], (int)bom, (int)final);
                    char* after = table_wrap(operation == 1 ? f->replace : f->split, context, endings[ending], (int)bom, (int)final);
                    before = implicit_global_definitions(before, f->definitions, endings[ending], (int)final);
                    after = implicit_global_definitions(after, f->definitions, endings[ending], (int)final);
                    implicit_inserted_separator(after, operation == 1 ? "X" : f->head, endings[ending]);
                    snprintf(replacement, sizeof(replacement), operation == 1 ? "X%sY" : "%s", endings[ending]);
                    printf("Implicit matrix fixture=%u operation=%u context=%u ending=%u BOM=%u EOF=%u reverse=%u\n",
                        fixture, operation, context, ending, bom, final, reverse); fflush(stdout);
                    implicit_golden(before, after, f->target, operation == 1 ? 1 : f->split_at,
                        operation == 1 ? f->replace_end : f->split_at, replacement, (int)reverse, operation == 2);
                    free(before); free(after); total++;
                }
    for (fixture = 0; fixture < sizeof(implicit_cases)/sizeof(*implicit_cases); fixture++)
        for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++) for (reverse = 0; reverse < 2; reverse++) {
            const struct implicit_case* f = &implicit_cases[fixture];
            char* before = delete_ending(f->before, endings[ending], (int)bom);
            char* after = delete_ending(f->after, endings[ending], (int)bom);
            implicit_golden(before, after, fixture == 5 ? "ab" : f->target, f->offset, f->offset, endings[ending], (int)reverse, 0);
            free(before); free(after); total++;
        }
    printf("Implicit references: %u independent raw/semantics/source-index/stable-tail/caret/one-Undo goldens passed\n", total);
    TABLE_CHECK(total == 3672);
    local_implicit_matrix(); local_direct_split_faults();
    implicit_faults(); implicit_rich_breaks(); direct_continuation_goldens(); return 0;
}
