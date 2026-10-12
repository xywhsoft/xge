#define XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
#include "xui_document_footnote_local_reference_diagnostic_test.c"
#undef XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
#ifndef XUI_DLL
#include "../src/xui/document/xui_document_internal.h"
#endif

typedef struct list_format {const char *open,*close,*split_close,*definition;unsigned marks;} list_format;
static const list_format list_formats[]={
    {"","","","",0}, {"__","__","__","",XUI_DOC_BOLD},
    {"``","``","``","",XUI_DOC_CODE},
    {"[","]","][abcd]","[abcd]: <raw> 'T'",XUI_DOC_LINK},
    {"[","][]","][abcd]","[abcd]: <raw> 'T'",XUI_DOC_LINK},
    {"[","][r]","][r]","[r]: <raw> 'T'",XUI_DOC_LINK},
    {"[","](<raw> 'T')","](<raw> 'T')","",XUI_DOC_LINK},
    {"[__","__][r]","__][r]","[r]: <raw> 'T'",XUI_DOC_LINK|XUI_DOC_BOLD}
};
static const struct list_context {const char *first,*next;int footnote;} list_contexts[]={
    {"- ","- ",0}, {"> - ","> - ",0}, {"- parent\n  - ","  - ",0},
    {"use[^n]\n\n[^n]:\n    - ","    - ",1}, {"- [x] ","- [ ] ",0},
    {"> - parent\n>   - ",">   - ",0}, {"9) ","10) ",0}, {"9)\t","10)\t",0},
    {"use[^n]\n\n[^n]: - ","    - ",1}, {"use[^n]\n\n[^n]: 9)\t","    10)\t",1}
};
static char* list_source(unsigned format,unsigned context,unsigned split,int after)
{
    const list_format* f=list_formats+format;const struct list_context* c=list_contexts+context;
    const char* word="abcd";char raw[4096];int n;
    if(after)n=snprintf(raw,sizeof(raw),"%s%s%.*s%s\n%s%s%s%s &amp; __keep__\n",
        c->first,f->open,(int)split,word,f->split_close,c->next,f->open,word+split,f->split_close);
    else n=snprintf(raw,sizeof(raw),"%s%sabcd%s &amp; __keep__\n",c->first,f->open,f->close);
    TABLE_CHECK(n>0 && (size_t)n<sizeof(raw));
    if(*f->definition){int added=snprintf(raw+n,sizeof(raw)-(size_t)n,"%s%s%s\n",c->footnote?"    \n":"\n",c->footnote?"    ":"",f->definition);
        TABLE_CHECK(added>0 && (size_t)added<sizeof(raw)-(size_t)n);n+=added;}
    {int added=snprintf(raw+n,sizeof(raw)-(size_t)n,"%s%s[unused]: /hold 'Keep'\n",c->footnote?"    \n":"\n",c->footnote?"    ":"");
        TABLE_CHECK(added>0 && (size_t)added<sizeof(raw)-(size_t)n);}
    char* result=malloc(strlen(raw)+1);TABLE_CHECK(result);strcpy(result,raw);return result;
}
static void list_caret_golden(xui_document d,xui_document_snapshot current,xui_doc_position_t* caret,
    const char* after,unsigned format,unsigned context,unsigned split)
{
    const list_format* f=list_formats+format;const struct list_context* c=list_contexts+context;
    uint64_t source;int quality;xui_doc_node_info_t node={0};char needle[256];node.iSize=sizeof(node);
    int n=snprintf(needle,sizeof(needle),"%s%s%s &amp; __keep__",f->open,"abcd"+split,f->split_close);
    TABLE_CHECK(n>0 && (size_t)n<sizeof(needle));
    const char* right=strstr(after,needle);TABLE_CHECK(right);right+=strlen(f->open);
    caret->iRevision=xuiDocumentGetRevision(d);
    TABLE_CHECK(xuiDocumentPositionToSource(current,caret,&source,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
        source==(uint64_t)(right-after) && caret->iKind==XUI_DOC_POSITION_TEXT && caret->iOffset==0 &&
        xuiDocumentSnapshotGetNode(current,caret->iNodeId,&node)==XUI_OK && node.tAttributes.iMarks==f->marks &&
        !strcmp(node.sResource,(f->marks&XUI_DOC_LINK)?"raw":"") && !strcmp(node.sTitle,(f->marks&XUI_DOC_LINK)?"T":""));
    TABLE_CHECK(xuiDocumentSnapshotGetNode(current,node.iParentId,&node)==XUI_OK && node.iKind==XUI_DOC_PARAGRAPH &&
        xuiDocumentSnapshotGetNode(current,node.iParentId,&node)==XUI_OK && node.iKind==XUI_DOC_LIST_ITEM);
    if(context==4)TABLE_CHECK((node.tAttributes.iFlags&(XUI_DOC_TASK|XUI_DOC_CHECKED))==XUI_DOC_TASK);
    if(context==6 || context==7 || context==9){TABLE_CHECK(xuiDocumentSnapshotGetNode(current,node.iParentId,&node)==XUI_OK &&
        node.iKind==XUI_DOC_LIST && (node.tAttributes.iFlags&XUI_DOC_ORDERED) && node.tAttributes.iListStart==9);}
    (void)c;
}
static void list_goldens(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned format=0;format<sizeof(list_formats)/sizeof(*list_formats);format++)
    for(unsigned context=0;context<sizeof(list_contexts)/sizeof(*list_contexts);context++)
    for(unsigned split=1;split<4;split++)for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++)for(unsigned eof=0;eof<2;eof++){
        char *raw_before=list_source(format,context,split,0),*raw_after=list_source(format,context,split,1);
        char *before=delete_ending(raw_before,endings[eol],bom),*after=delete_ending(raw_after,endings[eol],bom);
        if(eof){before[strlen(before)-strlen(endings[eol])]=0;after[strlen(after)-strlen(endings[eol])]=0;}
        table_allocator allocation={-1,0};xui_document d=table_open(before,&allocation);
        xui_document_snapshot old,current,restored;xui_document_transaction t;xui_doc_position_t at,caret;int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
            xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"abcd")-before)+split,&at,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT && at.iKind==XUI_DOC_POSITION_TEXT);
        printf("List CST format=%u context=%u split=%u eol=%u bom=%u eof=%u\n",format,context,split,eol,bom,eof);fflush(stdout);
        TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
        xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
        table_source(current,after);integration_equivalent(current,after);list_caret_golden(d,current,&caret,after,format,context,split);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!allocation.live);
        free(raw_before);free(raw_after);free(before);free(after);total++;
    }
    TABLE_CHECK(total==2880);printf("List split CST: %u independent spelling/semantic/full-index/exact-caret/task/order/snapshot/single-history/zero-leak goldens passed\n",total);
}
static void list_multiline_goldens(void)
{
    const struct multiline_case {const char *before,*after;} cases[]={
        {"9) [ab\n    cd][r] &amp; __keep__\n\n[r]: <raw> 'T'\n",
         "9) [a][r]\n10) [b\n     cd][r] &amp; __keep__\n\n[r]: <raw> 'T'\n"},
        {"9) [ab\n    cd] &amp; __keep__\n\n[ab cd]: <raw> 'T'\n",
         "9) [a][ab cd]\n10) [b\n     cd][ab cd] &amp; __keep__\n\n[ab cd]: <raw> 'T'\n"},
        {"- [x] [ab\n      cd][r] &amp; __keep__\n\n[r]: <raw> 'T'\n",
         "- [x] [a][r]\n- [ ] [b\n      cd][r] &amp; __keep__\n\n[r]: <raw> 'T'\n"},
        {"use[^n]\n\n[^n]:\n    9) [ab\n        cd] &amp; __keep__\n    \n    [ab cd]: <raw> 'T'\n",
         "use[^n]\n\n[^n]:\n    9) [a][ab cd]\n    10) [b\n         cd][ab cd] &amp; __keep__\n    \n    [ab cd]: <raw> 'T'\n"},
        {"use[^n]\n\n[^n]: -\t__ab\n        cd__ &amp;\n",
         "use[^n]\n\n[^n]: -\t__a__\n    -\t__b\n        cd__ &amp;\n"},
        {"use[^n]\n\n[^n]: 9)\t__ab\n        cd__ &amp;\n",
         "use[^n]\n\n[^n]: 9)\t__a__\n    10)\t__b\n        cd__ &amp;\n"},
        {"use[^long-label]\n\n[^long-label]: 9)\t__ab\n        cd__ &amp;\n",
         "use[^long-label]\n\n[^long-label]: 9)\t__a__\n    10)\t__b\n        cd__ &amp;\n"},
        {"9) [x] __ab\n       cd__ &amp;\n",
         "9) [x] __a__\n10) [ ] __b\n        cd__ &amp;\n"},
        {"use[^n]\n\n[^n]: 9)\t[x]\t__ab\n            cd__ &amp;\n",
         "use[^n]\n\n[^n]: 9)\t[x]\t__a__\n    10)\t[ ]\t__b\n            cd__ &amp;\n"}
    };
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++)for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++){
        char* before=delete_ending(cases[fixture].before,endings[eol],bom);char* after=delete_ending(cases[fixture].after,endings[eol],bom);
        table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_transaction t;
        xui_document_snapshot old,current,restored;xui_doc_position_t at,caret;uint64_t offset;int quality;
        printf("List multiline fixture=%u eol=%u bom=%u\n",fixture,eol,bom);fflush(stdout);
        const char* target=strstr(before,fixture<4?"[ab":"__ab");TABLE_CHECK(target);target+=fixture<4?1:2;
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
            xuiDocumentSourceToPosition(old,(uint64_t)(target-before)+1,&at,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
            xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
        xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);
        caret.iRevision=xuiDocumentGetRevision(d);TABLE_CHECK(xuiDocumentPositionToSource(current,&caret,&offset,&quality)==XUI_OK &&
            quality==XUI_DOC_MAP_EXACT && offset==(uint64_t)(strstr(after,fixture<4?"[b":"__b")-after)+(fixture<4?1:2));
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!a.live);free(before);free(after);total++;
    }
    TABLE_CHECK(total==54);printf("List multiline CST: %u marker-width/continuation-indent/implicit-key/footnote/task/source/index/caret/history goldens passed\n",total);
}
static void list_terminal_goldens(void)
{
    const struct terminal_case {const char *before,*after,*left,*right;unsigned marks;} cases[]={
        {"- __abcd__ &amp; __keep__\n","- __ab__\n- __cd__ &amp; __keep__\n","- __ab__","cd__ &amp;",XUI_DOC_BOLD},
        {"- [x] [abcd](<raw> 'T') &amp; __keep__\n","- [x] [ab](<raw> 'T')\n- [ ] [cd](<raw> 'T') &amp; __keep__\n","- [x] [ab](<raw> 'T')","cd](<raw>",XUI_DOC_LINK},
        {"[abcd]: <raw> 'T'\n\n9) [abcd] &amp; __keep__\n","[abcd]: <raw> 'T'\n\n9) [ab][abcd]\n10) [cd][abcd] &amp; __keep__\n","9) [ab][abcd]","cd][abcd] &amp;",XUI_DOC_LINK},
        {"use[^n]\n\n[^n]: - __abcd__ &amp; __keep__\n","use[^n]\n\n[^n]: - __ab__\n    - __cd__ &amp; __keep__\n","[^n]: - __ab__","cd__ &amp;",XUI_DOC_BOLD}
    };
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++)for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++)for(unsigned eof=0;eof<2;eof++){
        char* before=delete_ending(cases[fixture].before,endings[eol],bom);char* after=delete_ending(cases[fixture].after,endings[eol],bom);
        if(eof){
            char* separator=strstr(after,cases[fixture].left);TABLE_CHECK(separator);separator+=strlen(cases[fixture].left);
            before[strlen(before)-strlen(endings[eol])]=0;after[strlen(after)-strlen(endings[eol])]=0;
            /* Without a terminal ending, the existing command policy uses LF
             * for the new boundary; all pre-existing endings stay untouched. */
            memmove(separator+1,separator+strlen(endings[eol]),strlen(separator+strlen(endings[eol]))+1);*separator='\n';
        }
        table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_transaction t;
        xui_document_snapshot old,current,restored;xui_doc_position_t at,caret;xui_doc_node_info_t node={0};uint64_t offset;int quality;
        const char* body=fixture==2?strstr(before,"9) [abcd]")+4:strstr(before,"abcd");TABLE_CHECK(body);node.iSize=sizeof(node);
        printf("List terminal fixture=%u eol=%u bom=%u eof=%u\n",fixture,eol,bom,eof);fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
            xuiDocumentSourceToPosition(old,(uint64_t)(body-before)+2,&at,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
            xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
        xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);
        caret.iRevision=xuiDocumentGetRevision(d);TABLE_CHECK(xuiDocumentPositionToSource(current,&caret,&offset,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
            offset==(uint64_t)(strstr(after,cases[fixture].right)-after) && caret.iKind==XUI_DOC_POSITION_TEXT && caret.iOffset==0 &&
            xuiDocumentSnapshotGetNode(current,caret.iNodeId,&node)==XUI_OK && node.tAttributes.iMarks==cases[fixture].marks);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!a.live);free(before);free(after);total++;
    }
    TABLE_CHECK(total==48);printf("List terminal CST: %u actual paragraph EOF/first-line footnote/preceding-definition/exact-caret/source/index/history goldens passed\n",total);
}
static void list_continuation_goldens(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned format=0;format<sizeof(list_formats)/sizeof(*list_formats);format++)
    for(unsigned context=0;context<sizeof(list_contexts)/sizeof(*list_contexts);context++)
    for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++)for(unsigned compound=0;compound<2;compound++){
        const list_format* f=list_formats+format;char needle[256];int n=snprintf(needle,sizeof(needle),"%scd%s &amp; __keep__",f->open,f->split_close);
        char *raw_before=list_source(format,context,2,0),*raw_split=list_source(format,context,2,1);
        char *before=delete_ending(raw_before,endings[eol],bom),*split=delete_ending(raw_split,endings[eol],bom),*continued=malloc(strlen(split)+2);
        TABLE_CHECK(n>0 && (size_t)n<sizeof(needle) && continued);char* right=strstr(split,needle);TABLE_CHECK(right);right+=strlen(f->open);
        uint64_t source,anchor=(uint64_t)(right-split);memcpy(continued,split,(size_t)anchor);continued[anchor]='Z';strcpy(continued+anchor+1,split+anchor);
        table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_transaction t;xui_document_snapshot old,first,current,restored;
        xui_doc_position_t at,caret;xui_doc_node_info_t node={0};uint64_t revision;int quality;node.iSize=sizeof(node);
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
            xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"abcd")-before)+2,&at,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT);
        revision=xuiDocumentGetRevision(d);TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK);
        if(compound)TABLE_CHECK(xuiDocumentTxnReplaceText(t,caret.iNodeId,caret.iOffset,caret.iOffset,"Z",1)==XUI_OK);
        TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&first)==XUI_OK);
        table_source(first,compound?continued:split);integration_equivalent(first,compound?continued:split);caret.iRevision=xuiDocumentGetRevision(d);
        TABLE_CHECK(xuiDocumentPositionToSource(first,&caret,&source,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT && source==anchor);
        if(!compound){TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK &&
            xuiDocumentTxnReplaceText(t,caret.iNodeId,caret.iOffset,caret.iOffset,"Z",1)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);}
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK && xuiDocumentGetRevision(d)==revision+(compound?1:2));
        table_source(current,continued);integration_equivalent(current,continued);caret.iRevision=xuiDocumentGetRevision(d);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(current,caret.iNodeId,&node)==XUI_OK && node.iKind==XUI_DOC_TEXT && node.tAttributes.iMarks==f->marks &&
            !strcmp(node.sResource,(f->marks&XUI_DOC_LINK)?"raw":"") && !strcmp(node.sTitle,(f->marks&XUI_DOC_LINK)?"T":"") &&
            xuiDocumentPositionToSource(current,&caret,&source,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT && source==anchor);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,compound?before:split);xuiDocumentSnapshotRelease(restored);if(!compound)TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);
        if(!compound){TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,split);xuiDocumentSnapshotRelease(restored);TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);}
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,continued);table_equivalent(restored,current);
        xuiDocumentSnapshotRelease(restored);table_source(old,before);table_source(first,compound?continued:split);table_source(current,continued);
        xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(first);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!a.live);
        free(raw_before);free(raw_split);free(before);free(split);free(continued);total++;
    }
    TABLE_CHECK(total==960);printf("List continuation CST: %u exact-caret/same-draft/post-publish/format/link/source/index/shared-history/zero-leak goldens passed\n",total);
}
static void list_faults(void)
{
    const unsigned choices[][2]={{3,3},{7,6},{2,4},{7,8},{7,9}};
    for(unsigned fixture=0;fixture<sizeof(choices)/sizeof(*choices);fixture++){
        unsigned format=choices[fixture][0],context=choices[fixture][1],failures=0;long budget;
        char* before=list_source(format,context,2,0);char* after=list_source(format,context,2,1);
        for(budget=0;budget<10000;budget++){
            table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_transaction t;
            xui_document_snapshot old,current,restored;xui_doc_position_t at,caret;uint64_t revision;int quality,result;
            TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
                xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"abcd")-before)+2,&at,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
                xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);revision=xuiDocumentGetRevision(d);a.remaining=budget;
            result=xuiDocumentTxnSplitListItem(t,&at,&caret);if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);a.remaining=-1;
            TABLE_CHECK(result==XUI_OK || result==XUI_ERROR_OUT_OF_MEMORY);
            if(result!=XUI_OK)TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==result);
            xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result!=XUI_OK){
                failures++;TABLE_CHECK(xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current,before);table_equivalent(current,old);xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
                xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            }
            table_source(current,after);integration_equivalent(current,after);list_caret_golden(d,current,&caret,after,format,context,2);
            TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
            xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!a.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(budget<10000 && failures);
        printf("List split fixture=%u OOM points=%u atomic/poisoned commit/same-document retry/one history/zero leaks passed\n",fixture,failures);fflush(stdout);
        free(before);free(after);
    }
}
/* These independent source goldens place the boundary outside the complete
 * paragraph. In particular, an empty item must not inherit an empty link,
 * and typing before a carried loose paragraph creates a separate paragraph. */
static const struct list_edge_case {const char *before,*split[2],*typed[2];} list_edges[]={
    {"- __abcd__\n", {"- \n- __abcd__\n","- __abcd__\n- \n"},
        {"- \n- __Zabcd__\n","- __abcd__\n- Z\n"}},
    {"[r]: <raw> 'T'\n\n- [abcd][r]\n",
        {"[r]: <raw> 'T'\n\n- \n- [abcd][r]\n","[r]: <raw> 'T'\n\n- [abcd][r]\n- \n"},
        {"[r]: <raw> 'T'\n\n- \n- [Zabcd][r]\n","[r]: <raw> 'T'\n\n- [abcd][r]\n- Z\n"}},
    {"use[^n]\n\n[^n]: - __abcd__\n",
        {"use[^n]\n\n[^n]: - \n    - __abcd__\n","use[^n]\n\n[^n]: - __abcd__\n    - \n"},
        {"use[^n]\n\n[^n]: - \n    - __Zabcd__\n","use[^n]\n\n[^n]: - __abcd__\n    - Z\n"}},
    {"- __abcd__\n\n  tail &amp;\n",
        {"- \n  \n- __abcd__\n\n  tail &amp;\n","- __abcd__\n\n- tail &amp;\n"},
        {"- \n  \n- __Zabcd__\n\n  tail &amp;\n","- __abcd__\n\n- Z\n  \n  tail &amp;\n"}},
    {"9) [x] __abcd__\n\n       tail &amp;\n",
        {"9) [x] \n   \n10) [ ] __abcd__\n\n        tail &amp;\n",
         "9) [x] __abcd__\n   \n10) [ ] \n\n        tail &amp;\n"},
        {"9) [x] \n   \n10) [ ] __Zabcd__\n\n        tail &amp;\n",
         "9) [x] __abcd__\n   \n10) [ ] Z\n\n        tail &amp;\n"}},
    {"use[^n]\n\n[^n]:\n    - [abcd][r]\n    \n    [r]: <raw> 'T'\n",
        {"use[^n]\n\n[^n]:\n    - \n    - [abcd][r]\n    \n    [r]: <raw> 'T'\n",
         "use[^n]\n\n[^n]:\n    - [abcd][r]\n    - \n    \n    [r]: <raw> 'T'\n"},
        {"use[^n]\n\n[^n]:\n    - \n    - [Zabcd][r]\n    \n    [r]: <raw> 'T'\n",
         "use[^n]\n\n[^n]:\n    - [abcd][r]\n    - Z\n    \n    [r]: <raw> 'T'\n"}},
    {"9) __abcd__\n\n       code &amp;\n",{NULL,"9) __abcd__\n\n10)     code &amp;\n"},
        {NULL,"9) __abcd__\n\n10) Z\n    \n        code &amp;\n"}},
    {"- __abcd__\n\n  > tail &amp;\n",{NULL,"- __abcd__\n\n- > tail &amp;\n"},
        {NULL,"- __abcd__\n\n- Z\n  \n  > tail &amp;\n"}},
    {"- __abcd__\n\n  + [x] tail &amp;\n",{NULL,"- __abcd__\n\n- + [x] tail &amp;\n"},
        {NULL,"- __abcd__\n\n- Z\n  \n  + [x] tail &amp;\n"}},
    {"use[^n]\n\n[^n]: 9)\t__abcd__\n\n    \t    code &amp;\n",
        {NULL,"use[^n]\n\n[^n]: 9)\t__abcd__\n\n    10)\t    code &amp;\n"},
        {NULL,"use[^n]\n\n[^n]: 9)\t__abcd__\n\n    10)\tZ\n       \t\n       \t    code &amp;\n"}}
};
static xui_doc_position_t list_edge_position(xui_document_snapshot old,const char* before,unsigned gap,unsigned end)
{
    xui_doc_position_t at;int quality;xui_doc_node_info_t node={0};node.iSize=sizeof(node);
    TABLE_CHECK(xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"abcd")-before)+2,&at,&quality)==XUI_OK &&
        quality==XUI_DOC_MAP_EXACT && at.iKind==XUI_DOC_POSITION_TEXT &&
        xuiDocumentSnapshotGetNode(old,at.iNodeId,&node)==XUI_OK && node.iKind==XUI_DOC_TEXT);
    if(gap){at.iKind=XUI_DOC_POSITION_GAP;at.iNodeId=node.iParentId;
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old,at.iNodeId,&node)==XUI_OK);at.iOffset=end?node.iChildCount:0;}
    else at.iOffset=end?4:0;
    return at;
}
static void list_edge_goldens(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned fixture=0;fixture<sizeof(list_edges)/sizeof(*list_edges);fixture++)
    for(unsigned gap=0;gap<2;gap++)for(unsigned end=0;end<2;end++)for(unsigned eol=0;eol<3;eol++)
    for(unsigned bom=0;bom<2;bom++)for(unsigned eof=0;eof<2;eof++)for(unsigned compound=0;compound<2;compound++){
        if(fixture>=6 && !end)continue;
        const struct list_edge_case* c=list_edges+fixture;
        char *before=delete_ending(c->before,endings[eol],bom),*split=delete_ending(c->split[end],endings[eol],bom),
             *typed=delete_ending(c->typed[end],endings[eol],bom);
        if(eof){before[strlen(before)-strlen(endings[eol])]=0;split[strlen(split)-strlen(endings[eol])]=0;typed[strlen(typed)-strlen(endings[eol])]=0;}
        /* A paragraph at EOF without an ending chooses LF for its new boundary. */
        if(eof && fixture<3){
            free(split);free(typed);
            char *raw_split=delete_ending(c->split[end],endings[eol],bom),*raw_typed=delete_ending(c->typed[end],endings[eol],bom);
            raw_split[strlen(raw_split)-strlen(endings[eol])]=0;raw_typed[strlen(raw_typed)-strlen(endings[eol])]=0;
            const char* left=strstr(raw_split,"abcd");
            const char* separator=end?strstr(left+4,endings[eol]):strstr(raw_split,"- ")+2;
            TABLE_CHECK(separator && !memcmp(separator,endings[eol],strlen(endings[eol])));
            uint64_t cut=(uint64_t)(separator-raw_split);size_t size=strlen(raw_split),eol_size=strlen(endings[eol]);
            split=malloc(size+1);TABLE_CHECK(split);memcpy(split,raw_split,(size_t)cut);split[cut]='\n';strcpy(split+cut+1,raw_split+cut+eol_size);
            /* The typed boundary is unchanged by inserting one Z on its right. */
            typed=malloc(strlen(raw_typed)+1);TABLE_CHECK(typed);memcpy(typed,raw_typed,(size_t)cut);typed[cut]='\n';strcpy(typed+cut+1,raw_typed+cut+eol_size);
            free(raw_split);free(raw_typed);
        }
        if(eof && fixture==3 && end){
            const char* payload=strstr(split,"tail &amp;");TABLE_CHECK(payload);
            size_t cut=(size_t)(payload-split);free(typed);typed=malloc(strlen(split)+9);TABLE_CHECK(typed);
            memcpy(typed,split,cut);strcpy(typed+cut,"Z\n  \n  ");strcat(typed,payload);
        }
        if(eof && fixture>=6){
            char* boundary=strchr(typed,'Z');TABLE_CHECK(boundary);boundary++;
            for(unsigned inserted=0;inserted<2;inserted++){
                TABLE_CHECK(!memcmp(boundary,endings[eol],strlen(endings[eol])));
                memmove(boundary+1,boundary+strlen(endings[eol]),strlen(boundary+strlen(endings[eol]))+1);*boundary='\n';
                if(!inserted){boundary=strstr(boundary+1,endings[eol]);TABLE_CHECK(boundary);}
            }
        }
        table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_transaction t;
        xui_document_snapshot old,first,current,restored;xui_doc_position_t at,caret;uint64_t source;int quality;
        printf("List edge fixture=%u gap=%u end=%u eol=%u bom=%u eof=%u compound=%u\n",fixture,gap,end,eol,bom,eof,compound);fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK);
        at=list_edge_position(old,before,gap,end);
        TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK);
        if(compound){xui_doc_range_t selected={caret,caret};TABLE_CHECK(xuiDocumentTxnReplaceRange(t,&selected,"Z",1,&caret)==XUI_OK);}
        TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&first)==XUI_OK);table_source(first,compound?typed:split);integration_equivalent(first,compound?typed:split);
        if(!compound){caret.iRevision=xuiDocumentGetRevision(d);xui_doc_range_t selected={caret,caret};
            TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnReplaceRange(t,&selected,"Z",1,&caret)==XUI_OK &&
                xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);}
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,typed);integration_equivalent(current,typed);
        caret.iRevision=xuiDocumentGetRevision(d);
        TABLE_CHECK(xuiDocumentPositionToSource(current,&caret,&source,&quality)==XUI_OK && quality==XUI_DOC_MAP_EXACT &&
            source==(uint64_t)(strchr(typed,'Z')-typed)+1);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,compound?before:split);xuiDocumentSnapshotRelease(restored);
        if(!compound)TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);if(!compound)TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,typed);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);
        table_source(old,before);table_source(first,compound?typed:split);
        xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(first);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!a.live);
        free(before);free(split);free(typed);total++;
    }
    TABLE_CHECK(total==768);printf("List edge CST: %u whole-paragraph TEXT/GAP/source/reparse/full-index/continued-input/caret/compound-history/zero-leak goldens passed\n",total);
}
typedef struct list_edge_allocator {
    table_allocator allocation;
#ifndef XUI_DLL
    long cancel_after; atomic_int cancellation;
#endif
} list_edge_allocator;
static void* list_edge_alloc(void* user,size_t bytes)
{
    list_edge_allocator* a=user;
#ifndef XUI_DLL
    if(!a->cancel_after)atomic_store(&a->cancellation,XUI_DOC_ERROR_CANCELLED);
    else if(a->cancel_after>0)a->cancel_after--;
#endif
    return table_alloc(&a->allocation,bytes);
}
static void list_edge_free(void* user,void* pointer){list_edge_allocator* a=user;table_free(&a->allocation,pointer);}
static void list_edge_faults(void)
{
    const unsigned fixtures[]={3,4,5,6,9};unsigned modes=1;
#ifndef XUI_DLL
    modes=2;
#endif
    for(unsigned fixture_index=0;fixture_index<sizeof(fixtures)/sizeof(*fixtures);fixture_index++)
    for(unsigned published=0;published<2;published++)for(unsigned cancel=0;cancel<modes;cancel++){
        unsigned fixture=fixtures[fixture_index],failures=0;long point;
        const struct list_edge_case* c=list_edges+fixture;const char* baseline=published?c->split[1]:c->before;
        for(point=0;point<10000;point++){
            list_edge_allocator a={0};a.allocation.remaining=-1;
#ifndef XUI_DLL
            a.cancel_after=-1;atomic_init(&a.cancellation,0);
#endif
            xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
            desc.onAlloc=list_edge_alloc;desc.onFree=list_edge_free;desc.pAllocatorUser=&a;
            xui_document d;xui_document_transaction t;xui_document_snapshot old,current,restored;
            xui_doc_position_t at,initial,caret;uint64_t revision;int result;
            TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,c->before,strlen(c->before))==XUI_OK &&
                xuiDocumentAcquireSnapshot(d,&old)==XUI_OK);at=list_edge_position(old,c->before,fixture&1,1);
            if(published){TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&at)==XUI_OK &&
                xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);xuiDocumentSnapshotRelease(old);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&old)==XUI_OK);at.iRevision=xuiDocumentGetRevision(d);}
            initial=at;TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
                xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);revision=xuiDocumentGetRevision(d);
            if(cancel){
#ifndef XUI_DLL
                t->cancellation=&a.cancellation;a.cancel_after=point;
#endif
            }else a.allocation.remaining=point;
            caret=at;result=published?XUI_OK:xuiDocumentTxnSplitListItem(t,&at,&caret);
            if(result==XUI_OK){xui_doc_range_t selected={caret,caret};result=xuiDocumentTxnReplaceRange(t,&selected,"Z",1,&caret);}
            if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
            a.allocation.remaining=-1;
#ifndef XUI_DLL
            a.cancel_after=-1;
#endif
            TABLE_CHECK(result==XUI_OK || result==(cancel?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            if(result!=XUI_OK)TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==result);
            xuiDocumentTxnRelease(t);
#ifndef XUI_DLL
            atomic_store(&a.cancellation,0);
#endif
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result!=XUI_OK){
                failures++;TABLE_CHECK(xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current,baseline);table_equivalent(current,old);xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);caret=initial;
                if(!published)TABLE_CHECK(xuiDocumentTxnSplitListItem(t,&initial,&caret)==XUI_OK);
                xui_doc_range_t selected={caret,caret};TABLE_CHECK(xuiDocumentTxnReplaceRange(t,&selected,"Z",1,&caret)==XUI_OK &&
                    xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            }
            table_source(current,c->typed[1]);integration_equivalent(current,c->typed[1]);
            TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) &&
                !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,baseline);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,c->typed[1]);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,baseline);
            xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!a.allocation.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<10000 && failures);
        printf("List edge fixture=%u published=%u cancel=%u fault points=%u atomic/poisoned-commit/retry/shared-history/zero-leaks passed\n",
            fixture,published,cancel,failures);fflush(stdout);
    }
}
static const struct list_carry_case {const char *name,*before,*split,*marker;unsigned marker_skip;} list_carries[]={
    {"nested-paragraph","- parent\n  - __abcd__\n\n    tail &amp;\n","- parent\n  - __abcd__\n\n  - tail &amp;\n","\n  - tail",3},
    {"quote-paragraph","> - __abcd__\n>\n>   tail &amp;\n","> - __abcd__\n>\n> - tail &amp;\n","\n> - tail",3},
    {"tab-paragraph","9)\t__abcd__\n\n\t tail &amp;\n","9)\t__abcd__\n\n10)\t tail &amp;\n","\n10)\t",1},
    {"footnote-tab-paragraph","use[^n]\n\n[^n]: 9)\t__abcd__\n\n    \t tail &amp;\n","use[^n]\n\n[^n]: 9)\t__abcd__\n\n    10)\t tail &amp;\n","\n    10)\t",5},
    {"indented-code","9) __abcd__\n\n       code &amp;\n","9) __abcd__\n\n10)     code &amp;\n","\n10) ",1},
    {"quote-block","- __abcd__\n\n  > tail &amp;\n","- __abcd__\n\n- > tail &amp;\n","\n- >",1},
    {"table-block","- __abcd__\n\n  | a &amp; |\n  | --- |\n  | b |\n","- __abcd__\n\n- | a &amp; |\n  | --- |\n  | b |\n","\n- |",1},
    {"nested-list-block","- __abcd__\n\n  + tail &amp;\n","- __abcd__\n\n- + tail &amp;\n","\n- +",1},
    {"tab-multiline-paragraph","9)\t__abcd__\n\n\t tail &amp;\n\t next __raw__\n","9)\t__abcd__\n\n10)\t tail &amp;\n\t  next __raw__\n","\n10)",1},
    {"ordered-extra-space-multiline","9) __abcd__\n\n    tail &amp;\n    next __raw__\n","9) __abcd__\n\n10)  tail &amp;\n      next __raw__\n","\n10)",1},
    {"tab-following-code","9)\t__abcd__\n\n\t tail &amp;\n\n\t    code &amp;\n","9)\t__abcd__\n\n10)\t tail &amp;\n\n\t     code &amp;\n","\n10)",1},
    {"footnote-tab-following-code","use[^n]\n\n[^n]: 9)\t__abcd__\n\n    \t tail &amp;\n\n    \t    code &amp;\n","use[^n]\n\n[^n]: 9)\t__abcd__\n\n    10)\t tail &amp;\n\n    \t     code &amp;\n","\n    10)\t",5},
    {"indented-code-multiline","9) __abcd__\n\n       code &amp;\n       second &amp;\n","9) __abcd__\n\n10)     code &amp;\n        second &amp;\n","\n10)",1},
    {"quote-multiline","- __abcd__\n\n  > tail &amp;\n  > next __raw__\n","- __abcd__\n\n- > tail &amp;\n  > next __raw__\n","\n- ",1},
    {"nested-list-multiline","- __abcd__\n\n  + tail &amp;\n    next __raw__\n","- __abcd__\n\n- + tail &amp;\n    next __raw__\n","\n- ",1},
    {"nested-task-block","- __abcd__\n\n  + [x] tail &amp;\n","- __abcd__\n\n- + [x] tail &amp;\n","\n- +",1},
    {"negative-spaces-paragraph","000009) __abcd__\n\n        tail &amp;\n        next __raw__\n","000009) __abcd__\n\n10) tail &amp;\n    next __raw__\n","\n10)",1},
    {"negative-footnote-spaces-paragraph","use[^n]\n\n[^n]: 000009) __abcd__\n\n            tail &amp;\n            next __raw__\n","use[^n]\n\n[^n]: 000009) __abcd__\n\n    10) tail &amp;\n        next __raw__\n","\n    10)",5},
    {"negative-tab-paragraph","009)\t__abcd__\n\n\t\t tail &amp;\n\t\t next __raw__\n","009)\t__abcd__\n\n10)\t tail &amp;\n\t  next __raw__\n","\n10)",1},
    {"negative-footnote-tab-paragraph","use[^n]\n\n[^n]: 009)\t__abcd__\n\n    \t\t tail &amp;\n    \t\t next __raw__\n","use[^n]\n\n[^n]: 009)\t__abcd__\n\n    10)\t tail &amp;\n    \t  next __raw__\n","\n    10)",5},
    {"negative-spaces-code","000009) __abcd__\n\n            code &amp;\n            second &amp;\n","000009) __abcd__\n\n10)     code &amp;\n        second &amp;\n","\n10)",1},
    {"negative-footnote-spaces-code","use[^n]\n\n[^n]: 000009) __abcd__\n\n                code &amp;\n                second &amp;\n","use[^n]\n\n[^n]: 000009) __abcd__\n\n    10)     code &amp;\n            second &amp;\n","\n    10)",5},
    {"negative-tab-code","009)\t__abcd__\n\n\t\t    code &amp;\n\t\t    second &amp;\n","009)\t__abcd__\n\n10)\t    code &amp;\n\t\tsecond &amp;\n","\n10)",1},
    {"negative-footnote-tab-code","use[^n]\n\n[^n]: 009)\t__abcd__\n\n    \t\t    code &amp;\n    \t\t    second &amp;\n","use[^n]\n\n[^n]: 009)\t__abcd__\n\n    10)\t    code &amp;\n    \t\tsecond &amp;\n","\n    10)",5}
};

static void list_carry_marker(xui_document_snapshot current,const char* split,unsigned fixture,xui_doc_position_t* caret)
{
    const struct list_carry_case* c=list_carries+fixture;xui_doc_node_info_t node={0};xui_doc_block_syntax_t syntax={0};
    const char* marker=strstr(split,c->marker);TABLE_CHECK(marker);marker+=c->marker_skip;
    node.iSize=sizeof(node);syntax.iSize=sizeof(syntax);
    TABLE_CHECK(caret->iKind==XUI_DOC_POSITION_GAP && !caret->iOffset &&
        xuiDocumentSnapshotGetNode(current,caret->iNodeId,&node)==XUI_OK && node.iKind==XUI_DOC_LIST_ITEM &&
        !(node.tAttributes.iFlags&XUI_DOC_TASK) && xuiDocumentSnapshotGetBlockSyntax(current,caret->iNodeId,&syntax)==XUI_OK &&
        syntax.iKind==XUI_DOC_BLOCK_SYNTAX_LIST_ITEM && syntax.iPrimaryStart==(uint64_t)(marker-split) &&
        syntax.iPrimaryEnd==syntax.iPrimaryStart+(marker[0]=='1'?3:1));
    if(fixture==15){
        uint64_t id;TABLE_CHECK(xuiDocumentSnapshotGetChild(current,caret->iNodeId,0,&id)==XUI_OK &&
            xuiDocumentSnapshotGetNode(current,id,&node)==XUI_OK && node.iKind==XUI_DOC_LIST &&
            xuiDocumentSnapshotGetChild(current,id,0,&id)==XUI_OK && xuiDocumentSnapshotGetNode(current,id,&node)==XUI_OK &&
            node.iKind==XUI_DOC_LIST_ITEM && (node.tAttributes.iFlags&(XUI_DOC_TASK|XUI_DOC_CHECKED))==(XUI_DOC_TASK|XUI_DOC_CHECKED));
    }
}
static void list_carry_goldens(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned fixture=0;fixture<sizeof(list_carries)/sizeof(*list_carries);fixture++)for(unsigned gap=0;gap<2;gap++)
    for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++)for(unsigned eof=0;eof<2;eof++){
        const struct list_carry_case* c=list_carries+fixture;
        char *before=delete_ending(c->before,endings[eol],bom),*split=delete_ending(c->split,endings[eol],bom),
            *marker=delete_ending(c->marker,endings[eol],0);
        if(eof){before[strlen(before)-strlen(endings[eol])]=0;split[strlen(split)-strlen(endings[eol])]=0;}
        table_allocator a={-1,0},b={-1,0};xui_document d=table_open(before,&a),oracle=table_open(split,&b);
        xui_document_snapshot old,current,full,restored;xui_document_transaction t;xui_doc_position_t at,caret;
        printf("List carry fixture=%u name=%s gap=%u eol=%u bom=%u eof=%u\n",fixture,c->name,gap,eol,bom,eof);fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK);
        at=list_edge_position(old,before,gap,1);
        TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK &&
            xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK && xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);
        table_source(current,split);integration_equivalent(current,split);formatted_split_same_semantics(current,1,full,1);
        /* Marker needles include one LF in the fixtures; account for the
         * independently encoded source's actual line-ending width. */
        const char* expected=strstr(split,marker);TABLE_CHECK(expected);expected+=c->marker_skip+strlen(endings[eol])-1;
        xui_doc_node_info_t node={0};xui_doc_block_syntax_t syntax={0};node.iSize=sizeof(node);syntax.iSize=sizeof(syntax);
        TABLE_CHECK(caret.iKind==XUI_DOC_POSITION_GAP && !caret.iOffset &&
            xuiDocumentSnapshotGetNode(current,caret.iNodeId,&node)==XUI_OK && node.iKind==XUI_DOC_LIST_ITEM &&
            !(node.tAttributes.iFlags&XUI_DOC_TASK) && xuiDocumentSnapshotGetBlockSyntax(current,caret.iNodeId,&syntax)==XUI_OK &&
            syntax.iPrimaryStart==(uint64_t)(expected-split) && syntax.iPrimaryEnd==syntax.iPrimaryStart+(expected[0]=='1'?3:1));
        if(eol==0)list_carry_marker(current,split,fixture,&caret);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,split);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(full);
        xuiDocumentRelease(d);xuiDocumentRelease(oracle);TABLE_CHECK(!a.live && !b.live);free(before);free(split);free(marker);total++;
    }
    TABLE_CHECK(total==576);printf("List carry CST: %u independent multiline/block/Tab/footnote/source/full-semantics/full-index/marker-owner/task-scope/history/zero-leak goldens passed\n",total);
}
static void list_carry_faults(void)
{
    const unsigned fixtures[]={10,5,15,19,23};unsigned modes=1;
#ifndef XUI_DLL
    modes=2;
#endif
    for(unsigned index=0;index<sizeof(fixtures)/sizeof(*fixtures);index++)for(unsigned cancel=0;cancel<modes;cancel++){
        unsigned fixture=fixtures[index],failures=0;const struct list_carry_case* c=list_carries+fixture;long point;
        for(point=0;point<10000;point++){
            list_edge_allocator a={0};a.allocation.remaining=-1;
#ifndef XUI_DLL
            a.cancel_after=-1;atomic_init(&a.cancellation,0);
#endif
            xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
            desc.onAlloc=list_edge_alloc;desc.onFree=list_edge_free;desc.pAllocatorUser=&a;
            xui_document d;xui_document_snapshot old,current,restored;xui_document_transaction t;xui_doc_position_t at,caret;int result;
            TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,c->before,strlen(c->before))==XUI_OK &&
                xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK);
            at=list_edge_position(old,c->before,fixture&1,1);uint64_t revision=xuiDocumentGetRevision(d);
            TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
            if(cancel){
#ifndef XUI_DLL
                t->cancellation=&a.cancellation;a.cancel_after=point;
#endif
            }else a.allocation.remaining=point;
            result=xuiDocumentTxnSplitListItem(t,&at,&caret);if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
            a.allocation.remaining=-1;
#ifndef XUI_DLL
            a.cancel_after=-1;
#endif
            TABLE_CHECK(result==XUI_OK || result==(cancel?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            if(result!=XUI_OK)TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==result);
            xuiDocumentTxnRelease(t);
#ifndef XUI_DLL
            atomic_store(&a.cancellation,0);
#endif
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result!=XUI_OK){
                failures++;TABLE_CHECK(xuiDocumentGetRevision(d)==revision && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !xuiDocumentIsDirty(d));
                table_source(current,c->before);table_equivalent(current,old);xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnSplitListItem(t,&at,&caret)==XUI_OK &&
                    xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            }
            table_source(current,c->split);integration_equivalent(current,c->split);list_carry_marker(current,c->split,fixture,&caret);
            TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) &&
                !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,c->before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,c->split);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,c->before);
            xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!a.allocation.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<10000 && failures);printf("List carry fixture=%u cancel=%u fault points=%u atomic/retry/marker/history/zero-leaks passed\n",fixture,cancel,failures);fflush(stdout);
    }
}


static void list_gap_tab_goldens(void)
{
    const struct {const char *before,*typed;} gap_tab_cases[]={
        {"- first\n\n-\t    code &amp;\n","- first\n\n- Z\n  \n        code &amp;\n"},
        {"9) first\n\n10)\t     code &amp;\n","9) first\n\n10)\tZ\n   \t\n   \t     code &amp;\n"},
        {"use[^n]\n\n[^n]: - first\n\n    -\t    code &amp;\n","use[^n]\n\n[^n]: - first\n\n    - Z\n      \n            code &amp;\n"},
        {"> - first\n>\n> -\t    code &amp;\n","> - first\n>\n> -\tZ\n>  \t\n>  \t    code &amp;\n"}
    };
    const char* endings[]={"\n","\r\n","\r"};unsigned total=0;
    for(unsigned fixture=0;fixture<sizeof(gap_tab_cases)/sizeof(*gap_tab_cases);fixture++)
    for(unsigned eol=0;eol<3;eol++)for(unsigned bom=0;bom<2;bom++)for(unsigned eof=0;eof<2;eof++){
        char *before=delete_ending(gap_tab_cases[fixture].before,endings[eol],bom),*typed=delete_ending(gap_tab_cases[fixture].typed,endings[eol],bom);
        if(eof){
            before[strlen(before)-strlen(endings[eol])]=0;typed[strlen(typed)-strlen(endings[eol])]=0;
            char* boundary=strchr(typed,'Z');TABLE_CHECK(boundary);boundary++;
            for(unsigned inserted=0;inserted<2;inserted++){
                TABLE_CHECK(!memcmp(boundary,endings[eol],strlen(endings[eol])));
                memmove(boundary+1,boundary+strlen(endings[eol]),strlen(boundary+strlen(endings[eol]))+1);*boundary='\n';
                if(!inserted){boundary=strstr(boundary+1,endings[eol]);TABLE_CHECK(boundary);}
            }
        }
        table_allocator a={-1,0};xui_document d=table_open(before,&a);xui_document_snapshot old,current,restored;
        xui_document_transaction t;xui_doc_position_t at,caret;int quality;uint64_t source_at;xui_doc_node_info_t node={0};node.iSize=sizeof(node);
        printf("List gap Tab fixture=%u eol=%u bom=%u eof=%u\n",fixture,eol,bom,eof);fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&old)==XUI_OK && xuiDocumentMarkSaved(d,old)==XUI_OK &&
            xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before,"code")-before)+2,&at,&quality)==XUI_OK);
        uint64_t id=at.iNodeId;
        for(;;){TABLE_CHECK(xuiDocumentSnapshotGetNode(old,id,&node)==XUI_OK);if(node.iKind==XUI_DOC_LIST_ITEM)break;id=node.iParentId;TABLE_CHECK(id);}
        at.iKind=XUI_DOC_POSITION_GAP;at.iNodeId=id;at.iOffset=0;xui_doc_range_t range={at,at};
        TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnReplaceRange(t,&range,"Z",1,&caret)==XUI_OK &&
            xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,typed);integration_equivalent(current,typed);
        caret.iRevision=xuiDocumentGetRevision(d);TABLE_CHECK(xuiDocumentPositionToSource(current,&caret,&source_at,&quality)==XUI_OK &&
            quality==XUI_DOC_MAP_EXACT && source_at==(uint64_t)(strchr(typed,'Z')-typed)+1);
        TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
        table_source(restored,typed);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(old,before);
        xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!a.live);free(before);free(typed);total++;
    }
    TABLE_CHECK(total==48);printf("List gap Tab CST: %u parser-column/partial-Tab/code-indent/footnote/quote/source/full-index/caret/history/zero-leak goldens passed\n",total);
}

#include "xui_document_list_tight_gap_cases.h"
#include "xui_document_list_block_gap_cases.h"
#include "xui_document_container_gap_cases.h"
#include "xui_document_first_code_gap_cases.h"
#include "xui_document_first_quote_gap_cases.h"
#include "xui_document_first_marker_gap_cases.h"
int main(void){document_first_marker_gap_goldens();document_first_marker_gap_faults();document_first_quote_gap_goldens();document_first_quote_gap_faults();document_first_code_gap_goldens();document_first_code_gap_faults();document_container_gap_goldens();document_container_gap_faults();list_tight_gap_goldens();list_block_gap_goldens();list_block_gap_extended_goldens();list_block_gap_code_tail_goldens();list_block_gap_code_tail_faults();list_tight_gap_faults();list_block_gap_faults();list_gap_tab_goldens();list_carry_goldens();list_carry_faults();list_edge_goldens();list_edge_faults();list_goldens();list_multiline_goldens();list_terminal_goldens();list_continuation_goldens();list_faults();return 0;}
