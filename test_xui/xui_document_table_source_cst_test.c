#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TABLE_CHECK(x) do { if(!(x)){fprintf(stderr,"table CST line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
typedef struct table_case {const char* before;const char* after;unsigned column;int insert;} table_case;
static const table_case cases[]={
    {"H\\|A | B\n:--: | ---:\none | `a|b`", "H\\|A |  | B\n:--: | --- | ---:\none |  | `a|b`",1,1},
    {"| H\\|A | B |\n| :--: | ---: |\n| one | `a|b` |", "| H\\|A |  | B |\n| :--: | --- | ---: |\n| one |  | `a|b` |",1,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`", "|  |H\\|A | B\n| --- |:--: | ---:\n|  |one | `a|b`",0,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`", "H\\|A | B|  |\n:--: | ---:| --- |\none | `a|b`|  |",2,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`", "| B\n| ---:\n| `a|b`",0,0},
    {"H\\|A | B\n:--: | ---:\none | `a|b`", "H\\|A |\n:--: |\none |",1,0},
    {"| H\\|A | B\n| :--: | ---:\n| one | `a|b`", "| H\\|A |  | B\n| :--: | --- | ---:\n| one |  | `a|b`",1,1},
    {"H\\|A | B |\n:--: | ---: |\none | `a|b` |", " B |\n ---: |\n `a|b` |",0,0},
    {"|H\\|A|\tB|\n|:--:|\t---:|\n|one|\t`a|b`|", "|H\\|A|  |\tB|\n|:--:| --- |\t---:|\n|one|  |\t`a|b`|",1,1},
    {"| A | B | C |\n| :--: | ---: | --- |\n| x | y |", "| A |  | B | C |\n| :--: | --- | ---: | --- |\n| x |  | y |",1,1},
    {"| A | B | C |\n| :--: | ---: | --- |\n| x | y |", "| A | B | C |  |\n| :--: | ---: | --- | --- |\n| x | y |",3,1},
    {"| A | B | C |\n| :--: | ---: | --- |\n| x | y |", "| A | B |\n| :--: | ---: |\n| x | y |",2,0},
    {"| A | B |\n| :--: | ---: |\n| x | y | ignored &amp; |", "| A | B |  |\n| :--: | ---: | --- |\n| x | y |  | ignored &amp; |",2,1},
    {"| A | B |\n| :--: | ---: |\n| x | y | ignored &amp; |", "| B |\n| ---: |\n| y | ignored &amp; |",0,0},
    {"| A | B |\n| :--: | ---: |\n| x |", "| A |  | B |\n| :--: | --- | ---: |\n| x |",1,1},
    {"| A | B |\n| :--: | ---: |\n| x |", "| B |\n| ---: |\n||",0,0},
    {"A |  | B\n:--: | --- | ---:\nx |  | y", "|  | B\n --- | ---:\n|  | y",0,0},
    {"A |  | B\n:--: | --- | ---:\nx |  | y", "A |  |\n:--: | --- \nx |  |",2,0},
    {"| A | B |\n| :--: | ---: |\n|", "| A |  | B |\n| :--: | --- | ---: |\n|",1,1},
    {"| A | B |\n| :--: | ---: |\n|", "| B |\n| ---: |\n|",0,0},
    {"| A | B |\n| :--: | ---: |\n| x", "| B |\n| ---: |\n|",0,0}
};
typedef struct table_allocator {long remaining;unsigned live;} table_allocator;
static void* table_alloc(void* user,size_t size)
{table_allocator* a=user;void* p;if(!a->remaining)return NULL;if(a->remaining>0)a->remaining--;p=malloc(size);if(p)a->live++;return p;}
static void table_free(void* user,void* p)
{table_allocator* a=user;TABLE_CHECK(a->live);a->live--;free(p);}
static xui_document table_open(const char* source,table_allocator* allocator)
{
    xui_document d;xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
    if(allocator){desc.onAlloc=table_alloc;desc.onFree=table_free;desc.pAllocatorUser=allocator;}
    TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source,strlen(source))==XUI_OK);return d;
}
static uint64_t table_find(xui_document_snapshot s,uint64_t id)
{
    xui_doc_node_info_t n={0};uint64_t i;n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(s,id,&n)==XUI_OK);
    if(n.iKind==XUI_DOC_TABLE)return id;
    for(i=0;i<n.iChildCount;i++){uint64_t child,found;TABLE_CHECK(xuiDocumentSnapshotGetChild(s,id,i,&child)==XUI_OK);found=table_find(s,child);if(found)return found;}
    return 0;
}
static void table_source(xui_document_snapshot s,const char* expected)
{
    uint64_t bytes;char* source;TABLE_CHECK(xuiDocumentSnapshotCopySource(s,NULL,0,&bytes)==XUI_OK);
    source=malloc((size_t)bytes+1);TABLE_CHECK(source && xuiDocumentSnapshotCopySource(s,source,bytes+1,&bytes)==XUI_OK);
    if(bytes!=strlen(expected)||memcmp(source,expected,(size_t)bytes))fprintf(stderr,"actual=[%s]\nexpected=[%s]\n",source,expected);
    TABLE_CHECK(bytes==strlen(expected)&&!memcmp(source,expected,(size_t)bytes));free(source);
}
static void table_node_role(xui_document_snapshot a,uint64_t aid,xui_document_snapshot b,uint64_t bid)
{
    xui_doc_node_info_t x={0},y={0};x.iSize=sizeof(x);y.iSize=sizeof(y);
    TABLE_CHECK(!!aid==!!bid);if(!aid)return;
    TABLE_CHECK(xuiDocumentSnapshotGetNode(a,aid,&x)==XUI_OK && xuiDocumentSnapshotGetNode(b,bid,&y)==XUI_OK);
    TABLE_CHECK(x.iKind==y.iKind && x.iTextBytes==y.iTextBytes && x.iSourceStart==y.iSourceStart && x.iSourceEnd==y.iSourceEnd &&
        x.iSyntaxStart==y.iSyntaxStart && x.iSyntaxEnd==y.iSyntaxEnd && x.bSourceExact==y.bSourceExact);
}
static void table_equivalent(xui_document_snapshot a,xui_document_snapshot b)
{
    xui_document_source_index ai,bi;xui_doc_source_index_info_t x={0},y={0};uint64_t i,end=0;char *ah,*bh;uint64_t an,bn;
    TABLE_CHECK(xuiDocumentExportHtml(a,&ah,&an)==XUI_OK && xuiDocumentExportHtml(b,&bh,&bn)==XUI_OK && an==bn && !memcmp(ah,bh,(size_t)an));
    xuiDocumentFreeBuffer(ah);xuiDocumentFreeBuffer(bh);
    TABLE_CHECK(xuiDocumentSnapshotCreateSourceIndex(a,NULL,&ai)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(b,NULL,&bi)==XUI_OK);
    x.iSize=sizeof(x);y.iSize=sizeof(y);TABLE_CHECK(xuiDocumentSourceIndexGetInfo(ai,&x)==XUI_OK && xuiDocumentSourceIndexGetInfo(bi,&y)==XUI_OK);
    TABLE_CHECK(x.iSourceBytes==y.iSourceBytes && x.iRecordCount==y.iRecordCount && x.iTokenCount==y.iTokenCount && x.iBindingCount==y.iBindingCount);
    for(i=0;i<x.iRecordCount;i++){
        xui_doc_source_record_t p={0},q={0};p.iSize=sizeof(p);q.iSize=sizeof(q);
        TABLE_CHECK(xuiDocumentSourceIndexGetRecord(ai,i,&p)==XUI_OK && xuiDocumentSourceIndexGetRecord(bi,i,&q)==XUI_OK);
        table_node_role(a,p.iNodeId,b,q.iNodeId);p.iNodeId=q.iNodeId=0;TABLE_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    for(i=0;i<x.iTokenCount;i++){
        xui_doc_source_token_t p={0},q={0};p.iSize=sizeof(p);q.iSize=sizeof(q);
        TABLE_CHECK(xuiDocumentSourceIndexGetToken(ai,i,&p)==XUI_OK && xuiDocumentSourceIndexGetToken(bi,i,&q)==XUI_OK);
        TABLE_CHECK(p.iSourceStart==end&&p.iSourceEnd>p.iSourceStart);end=p.iSourceEnd;
        TABLE_CHECK(p.iKind>=XUI_DOC_SOURCE_TOKEN_CONTENT && p.iKind<=XUI_DOC_SOURCE_TOKEN_BREAK_MARKER);
        table_node_role(a,p.iNodeId,b,q.iNodeId);p.iNodeId=q.iNodeId=0;TABLE_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    TABLE_CHECK(end==x.iSourceBytes);
    for(i=0;i<x.iBindingCount;i++){
        xui_doc_source_binding_t p={0},q={0};p.iSize=sizeof(p);q.iSize=sizeof(q);
        TABLE_CHECK(xuiDocumentSourceIndexGetBinding(ai,i,&p)==XUI_OK && xuiDocumentSourceIndexGetBinding(bi,i,&q)==XUI_OK);
        table_node_role(a,p.tRange.iNodeId,b,q.tRange.iNodeId);p.tRange.iNodeId=q.tRange.iNodeId=0;TABLE_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    xuiDocumentSourceIndexRelease(ai);xuiDocumentSourceIndexRelease(bi);
}
/* Context and line-ending expansion do not derive the edited bytes. Every
 * column result above is an independent, concrete source golden. */
static char* table_wrap(const char* table,unsigned context,const char* ending,int bom,int final)
{
    char lf[4096],*source;size_t at=0,i,line=0,used=0;
    const char* begin=context==4?"lead &amp;\n\nuse [^n].\n\n[^unused]: keep &amp;\n\n[^n]:\n":"lead &amp;\n\n";
    const char* tail="\n\nend &amp;\n\n[keep]: <raw> 'title'";
    size_t bytes=strlen(begin);memcpy(lf+at,begin,bytes);at+=bytes;
    for(i=0;table[i];){
        const char* prefix=context==1?"> ":context==2?(line?"  ":"- "):context==3?(line?">   ":"> - "):context==4?"    ":"";
        bytes=strlen(prefix);memcpy(lf+at,prefix,bytes);at+=bytes;
        while(table[i]&&table[i]!='\n')lf[at++]=table[i++];
        if(table[i]){lf[at++]='\n';i++;line++;}
    }
    bytes=strlen(tail);memcpy(lf+at,tail,bytes);at+=bytes;if(final)lf[at++]='\n';lf[at]=0;
    source=malloc(at*2+4);TABLE_CHECK(source);if(bom){memcpy(source,"\xef\xbb\xbf",3);used=3;}
    for(i=0;i<at;i++){if(lf[i]=='\n'){bytes=strlen(ending);memcpy(source+used,ending,bytes);used+=bytes;}else source[used++]=lf[i];}
    source[used]=0;return source;
}
static int table_command_id(xui_document d,uint64_t table,unsigned column,int insert)
{
    xui_document_transaction t;int result;
    result=xuiDocumentBeginTransaction(d,NULL,&t);if(result!=XUI_OK)return result;
    result=insert?xuiDocumentTxnInsertTableColumn(t,table,column):xuiDocumentTxnDeleteTableColumn(t,table,column);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
    xuiDocumentTxnRelease(t);return result;
}
static int table_command(xui_document d,unsigned column,int insert)
{
    xui_document_snapshot s;uint64_t table;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);table=table_find(s,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);xuiDocumentSnapshotRelease(s);
    return table_command_id(d,table,column,insert);
}
static void table_golden(const char* before,const char* after,unsigned column,int insert)
{
    xui_document d=table_open(before,NULL),full; xui_document_snapshot a,b,original,restored;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    TABLE_CHECK(table_command(d,column,insert)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&a)==XUI_OK);table_source(a,after);
    full=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&b)==XUI_OK);table_equivalent(a,b);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,original);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,after);table_equivalent(restored,a);xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(a);xuiDocumentSnapshotRelease(b);xuiDocumentRelease(full);xuiDocumentRelease(d);
}
static void table_faults(void)
{
    unsigned mode;for(mode=0;mode<2;mode++){
        char* before=table_wrap(cases[mode?13:0].before,mode?4:3,"\r\n",1,1);
        char* after=table_wrap(cases[mode?13:0].after,mode?4:3,"\r\n",1,1);long point;
        for(point=0;point<4096;point++){
            table_allocator f={-1,0};xui_document d=table_open(before,&f);xui_document_snapshot original,current;uint64_t revision,table;int result,can_undo;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);revision=xuiDocumentGetRevision(d);
            table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);can_undo=xuiDocumentCanUndo(d);
            f.remaining=point;result=table_command_id(d,table,cases[mode?13:0].column,cases[mode?13:0].insert);f.remaining=-1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result==XUI_OK)table_source(current,after);
            else{TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&xuiDocumentCanUndo(d)==can_undo);table_source(current,before);table_equivalent(current,original);}
            xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!f.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Table source %s: %ld OOM points atomic, saved state/history unchanged and zero leaks\n",mode?"delete":"insert",point);free(before);free(after);
    }
}
static char* table_prepend(const char* source,int bom,const char* ending)
{
    size_t bytes=strlen(source),e=strlen(ending),at=bom?3:0;char* result=malloc(bytes+6+2*e+1);TABLE_CHECK(result);
    memcpy(result,source,at);memcpy(result+at,"prefix",6);memcpy(result+at+6,ending,e);memcpy(result+at+6+e,ending,e);
    memcpy(result+at+6+2*e,source+at,bytes-at+1);return result;
}
static void table_prefix_history(unsigned context,const char* ending,int bom,int prepare)
{
    char* before=table_wrap(cases[0].before,context,ending,bom,1);char* after=table_wrap(cases[0].after,context,ending,bom,1);
    char* shifted_before=table_prepend(before,bom,ending);char* shifted_after=table_prepend(after,bom,ending);char prefix[16];size_t n;
    xui_document d=table_open(before,NULL),full;xui_document_snapshot original,current,oracle;uint64_t offset=bom?3:0;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    n=(size_t)snprintf(prefix,sizeof(prefix),"prefix%s%s",ending,ending);TABLE_CHECK(n<sizeof(prefix));
    if(prepare){
        xui_document_prepare p;xui_doc_source_patch_t patch={0};patch.iSize=sizeof(patch);patch.iStart=patch.iEnd=offset;patch.sText=prefix;patch.iTextBytes=n;
        TABLE_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&p)==XUI_OK&&xuiDocumentPrepareRun(p)==XUI_OK&&xuiDocumentPreparePublish(d,p,NULL)==XUI_OK);xuiDocumentPrepareRelease(p);
    }else{
        xui_document_transaction t;xui_doc_txn_desc_t desc={0};desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
        TABLE_CHECK(xuiDocumentBeginTransaction(d,&desc,&t)==XUI_OK&&xuiDocumentTxnReplaceSource(t,offset,offset,prefix,n)==XUI_OK&&xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    }
    TABLE_CHECK(table_command(d,1,1)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,shifted_after);
    full=table_open(shifted_after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&oracle)==XUI_OK);table_equivalent(current,oracle);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(oracle);xuiDocumentRelease(full);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,shifted_before);xuiDocumentSnapshotRelease(current);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_equivalent(current,original);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(original);xuiDocumentRelease(d);free(before);free(after);free(shifted_before);free(shifted_after);
}
static void table_compound(unsigned context)
{
    const char* result="|  | B\n --- | ---:\n|  | `a|b`";
    char* before=table_wrap(cases[0].before,context,"\r\n",1,1);char* after=table_wrap(result,context,"\r\n",1,1);
    xui_document d=table_open(before,NULL),full;xui_document_snapshot original,current,oracle;xui_document_transaction t;uint64_t table;xui_doc_stats_t stats={0};
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table&&xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    TABLE_CHECK(xuiDocumentTxnInsertTableColumn(t,table,1)==XUI_OK&&xuiDocumentTxnDeleteTableColumn(t,table,0)==XUI_OK&&xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    stats.iSize=sizeof(stats);TABLE_CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK&&stats.iUndoCount==1);
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);full=table_open(after,NULL);
    TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&oracle)==XUI_OK);table_equivalent(current,oracle);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(oracle);xuiDocumentRelease(full);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_equivalent(current,original);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(original);xuiDocumentRelease(d);free(before);free(after);
}
int main(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,context,e,bom,final,total=0;
    for(fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++)for(context=0;context<5;context++)
    for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++){
        char* before=table_wrap(cases[fixture].before,context,endings[e],bom,final);
        char* after=table_wrap(cases[fixture].after,context,endings[e],bom,final);
        table_golden(before,after,cases[fixture].column,cases[fixture].insert);free(before);free(after);total++;
    }
    printf("Table source: %u independent raw goldens, full HTML/source records/partition/bindings and Undo/Redo passed\n",total);
    for(context=0;context<5;context++){
        table_compound(context);
        for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++)table_prefix_history(context,endings[e],bom,final);
    }
    puts("Table source: 5 two-command root transactions/one Undo and 60 SOURCE/Prepare prefix shifts with shared source/structural history passed");
    table_faults();return 0;
}
