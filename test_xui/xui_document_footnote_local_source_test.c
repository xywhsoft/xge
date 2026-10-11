#define XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
#include "xui_document_footnote_local_reference_diagnostic_test.c"
#undef XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY

static const struct local_source_case {
    const char *before, *after, *needle, *replacement, *target, *uri;
    unsigned marks;
} local_source_cases[] = {
    {"use[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /local 'Title'\n",
     "use[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /changed 'Title'\n", "/local", "/changed", "inside", "/changed", XUI_DOC_LINK},
    {"[outside][r]\n\n[^n]:\n    [r]: /hidden 'Title'\n",
     "[outside][r]\n\n[^n]:\n    [r]: /changed 'Title'\n", "/hidden", "/changed", "outside", "/changed", XUI_DOC_LINK},
    {"[outside][r] use[^n]\n\n[^n]:\n    [r]: /first\n\n[r]: /second\n",
     "[outside][r] use[^n]\n\n[^n]:\n    \n\n[r]: /second\n", "[r]: /first", "", "outside", "/second", XUI_DOC_LINK},
    {"[r]: /first\n\nuse[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /second\n",
     "\n\nuse[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /second\n", "[r]: /first", "", "inside", "/second", XUI_DOC_LINK},
    {"[outside][r]\n\n[^n]:\n    [s]: /local\n",
     "[outside][r]\n\n[^n]:\n    [r]: /local\n", "[s]:", "[r]:", "outside", "/local", XUI_DOC_LINK},
    {"[outside][r] use[^n]\n\n[^n]:\n    [r]: /local\n",
     "[outside][r] use\n\n[^n]:\n    [r]: /local\n", "[^n]", "", "outside", "/local", XUI_DOC_LINK},
    {"[outside][r]\n\n[^n]:\n    [r]: /local\n",
     "[outside][r]\n\n[^n]:\n    [s]: /local\n", "[r]:", "[s]:", "outside", "", 0},
    {"use[^n]\n\n[^n]: [r]: /firstline 'Title'\n    \n    [inside][r]\n",
     "use[^n]\n\n[^n]: [r]: /changed 'Title'\n    \n    [inside][r]\n", "/firstline", "/changed", "inside", "/changed", XUI_DOC_LINK},
    {"top\n\n[^n]: [a]: <uri> 'title'\n",
     "top\n\n[^n]: a]: <uri> 'title'\n", "[a]:", "a]:", "top", "", 0},
    {"[outside][a]\n\n[^n]: [a]: <uri> 'title'\n",
     "[outside][a]\n\n[^n]: a]: <uri> 'title'\n", "[a]:", "a]:", "outside", "", 0},
    {"[outside][a]\n\n[^n]: [a]: <uri> 'title'\n\n[a]: /second\n",
     "[outside][a]\n\n[^n]: a]: <uri> 'title'\n\n[a]: /second\n", "[a]: <uri>", "a]: <uri>", "outside", "/second", XUI_DOC_LINK},
    {"[outside][a]\n\n[^n]: a]: <uri> 'title'\n",
     "[outside][a]\n\n[^n]: [a]: <uri> 'title'\n", "a]:", "[a]:", "outside", "uri", XUI_DOC_LINK},
    {"[outside][a]\n\n[^n]: [a]: <uri> 'title'\n",
     "[outside][a]\n\n[^n]: [b]: <uri> 'title'\n", "[a]:", "[b]:", "outside", "", 0},
    {"[outside][a]\n\n[^n]: [a]: <uri> 'title'\n",
     "[outside][a]\n\n[^n]: [a]: <new> 'title'\n", "uri", "new", "outside", "new", XUI_DOC_LINK},
    {"[outside][a]\n\n[^n]: intro\n    \n    [a]: <uri> 'title'\n",
     "[outside][a]\n\n[^n]: intro\n    \n    a]: <uri> 'title'\n", "[a]:", "a]:", "outside", "", 0},
    {"[outside][a] use[^n]\n\n[^n]: [a]: <uri> 'title'\n",
     "[outside][a] use[^n]\n\n[^n]: a]: <uri> 'title'\n", "[a]:", "a]:", "outside", "", 0}
};

static void local_link_golden(xui_document_snapshot snapshot, const char* source,
    const char* target, const char* uri, unsigned marks)
{
    xui_doc_position_t position; xui_doc_node_info_t info = {0}; int quality;
    const char* found = strstr(source, target); TABLE_CHECK(found);
    TABLE_CHECK(xuiDocumentSourceToPosition(snapshot, (uint64_t)(found-source)+1, &position, &quality) == XUI_OK &&
        position.iKind == XUI_DOC_POSITION_TEXT);
    info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == marks && !strcmp(info.sResource, uri));
}

static void local_cross_definition_goldens(void)
{
    const struct cross_definition_case {
        const char *before,*after,*replacement,*target,*uri;
        unsigned marks;
    } goldens[]={
        {"body[^n]\n\n[^n]: left [r] &amp; keep\n\n    [r]: /raw \"T\"\n\n    second &amp; right\n",
         "body[^n]\n\n[^n]: left [r] &amp; JOIN\n\n    MORE &amp; right\n\n    [r]: /raw \"T\"\n\n","JOIN\nMORE","[r]","/raw",XUI_DOC_LINK},
        {"body[^n]\n\n[^n]: left &amp; keep\n\n    [unused]: /raw \"T\"\n\n    second &amp; right\n",
         "body[^n]\n\n[^n]: left &amp; JOIN &amp; right\n\n    [unused]: /raw \"T\"\n\n","JOIN","left","",0},
        {"body[^n]\n\n[^n]: intro\n\n    > left &amp; keep\n    >\n    > [unused]: /raw \"T\"\n    >\n    > second &amp; right\n",
         "body[^n]\n\n[^n]: intro\n\n    > left &amp; JOIN &amp; right\n    >\n    > [unused]: /raw \"T\"\n    >\n","JOIN","left","",0}
    };
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,eol,bom,reverse,checked=0;
    for(fixture=0;fixture<sizeof(goldens)/sizeof(*goldens);fixture++)
        for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++)for(reverse=0;reverse<2;reverse++){
        const struct cross_definition_case* c=&goldens[fixture];
        char* before=delete_ending(c->before,endings[eol],bom);char* after=delete_ending(c->after,endings[eol],bom);
        char* replacement=delete_ending(c->replacement,endings[eol],0);
        table_allocator allocator={-1,0};xui_document d=table_open(before,&allocator);
        xui_document_snapshot old,current,restored;xui_document_transaction t;
        xui_doc_range_t range={0};xui_doc_position_t caret;xui_doc_source_info_t info={0};int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK);
        info.iSize=sizeof(info);TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(old,&info)==XUI_OK && info.iReferenceDefinitionCount==2);
        local_link_golden(old,before,c->target,c->uri,c->marks);
        TABLE_CHECK(xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"keep")-before),&range.tAnchor,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
            xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"second")-before)+6,&range.tCaret,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT);
        if(reverse){xui_doc_position_t swap=range.tAnchor;range.tAnchor=range.tCaret;range.tCaret=swap;}
        TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnReplaceRange(t,&range,replacement,strlen(replacement),&caret)==XUI_OK &&
            xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);
        local_link_golden(current,after,c->target,c->uri,c->marks);
        TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(current,&info)==XUI_OK && info.iReferenceDefinitionCount==2);boundary_caret(d,current,&caret);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,after);integration_equivalent(restored,after);local_link_golden(restored,after,c->target,c->uri,c->marks);
        xuiDocumentSnapshotRelease(restored);table_source(old,before);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(d);TABLE_CHECK(!allocator.live);free(before);free(after);free(replacement);checked++;
    }
    TABLE_CHECK(checked==36);printf("Footnote cross-definition checked=%u untouched definition/source/index/link/caret/history/cleanup checks passed\n",checked);
}

int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, eol, bom, mode, checked = 0;
    for (fixture=0; fixture<sizeof(local_source_cases)/sizeof(*local_source_cases); fixture++)
        for(eol=0;eol<3;eol++) for(bom=0;bom<2;bom++) for(mode=0;mode<2;mode++) {
        const struct local_source_case* c = &local_source_cases[fixture];
        char *before = delete_ending(c->before,endings[eol],bom), *after = delete_ending(c->after,endings[eol],bom);
        const char* found = strstr(before,c->needle);
        table_allocator allocator = {-1,0}; xui_document d = table_open(before,&allocator);
        xui_document_snapshot old, current, restored; xui_doc_txn_desc_t td = {0};
        uint64_t revision, start; size_t middle = strlen(c->replacement)/2;
        TABLE_CHECK(found && xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK &&
            xuiDocumentMarkSaved(d,old)==XUI_OK); revision = xuiDocumentGetRevision(d); start = (uint64_t)(found-before);
        td.iSize=sizeof(td); td.iDomain=XUI_DOC_SOURCE;
        printf("Local source edit fixture=%u eol=%u bom=%u mode=%u\n",fixture,eol,bom,mode); fflush(stdout);
        if(!mode) {
            xui_document_transaction t; TABLE_CHECK(xuiDocumentBeginTransaction(d,&td,&t)==XUI_OK &&
                xuiDocumentTxnReplaceSource(t,start,start+strlen(c->needle),c->replacement,strlen(c->replacement))==XUI_OK &&
                xuiDocumentTxnCommit(t,NULL)==XUI_OK); xuiDocumentTxnRelease(t);
        } else {
            xui_document_prepare first, second; xui_doc_source_patch_t patch={0}; xui_doc_prepare_info_t pi={0};
            patch.iSize=sizeof(patch);patch.iStart=start;patch.iEnd=start+strlen(c->needle);
            patch.sText=c->replacement;patch.iTextBytes=middle;
            TABLE_CHECK(xuiDocumentPrepareSource(d,&td,&patch,1,&first)==XUI_OK && xuiDocumentPrepareRun(first)==XUI_OK &&
                xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
            patch.iStart=patch.iEnd=start+middle;patch.sText=c->replacement+middle;patch.iTextBytes=strlen(c->replacement)-middle;
            TABLE_CHECK(xuiDocumentPrepareContinueSource(d,first,&patch,1,&second)==XUI_OK);
            pi.iSize=sizeof(pi);TABLE_CHECK(xuiDocumentPrepareGetInfo(first,&pi)==XUI_OK && pi.iResult==XUI_DOC_ERROR_STALE);
            TABLE_CHECK(xuiDocumentPrepareRun(second)==XUI_OK && xuiDocumentPreparePublish(d,second,NULL)==XUI_OK);
            xuiDocumentPrepareRelease(first);xuiDocumentPrepareRelease(second);
        }
        TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
        table_source(current,after);integration_equivalent(current,after);local_link_golden(current,after,c->target,c->uri,c->marks);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
            xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,before);table_equivalent(restored,old);
        xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,after);integration_equivalent(restored,after);local_link_golden(restored,after,c->target,c->uri,c->marks);
        xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);
        TABLE_CHECK(!allocator.live);free(before);free(after);checked++;
    }
    TABLE_CHECK(checked==192);
    local_cross_definition_goldens();
    printf("Footnote local source edits checked=%u independent URI/activation/duplicate-order/visibility/source/index/history checks passed\n",checked);
    return 0;
}
