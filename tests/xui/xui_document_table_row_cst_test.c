#define main table_column_driver_main
#include "xui_document_table_source_cst_test.c"
#undef main

typedef struct row_case {const char* before;const char* after;unsigned row;int insert;} row_case;
static const row_case row_cases[]={
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "H\\|A | B\n:--: | ---:\n|  |  |\none | `a|b`\nlast | &amp;",1,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "H\\|A | B\n:--: | ---:\none | `a|b`\n|  |  |\nlast | &amp;",2,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;\n|  |  |",3,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "H\\|A | B\n:--: | ---:\nlast | &amp;",1,0},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "H\\|A | B\n:--: | ---:\none | `a|b`",2,0},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "|  |  |\n:--: | ---:\nH\\|A | B\none | `a|b`\nlast | &amp;",0,1},
    {"H\\|A | B\n:--: | ---:\none | `a|b`\nlast | &amp;", "one | `a|b`\n:--: | ---:\nlast | &amp;",0,0},
    {"| A | B |\n| :--: | ---: |", "| A | B |\n| :--: | ---: |\n|  |  |",1,1},
    {"| A | B |\n| :--: | ---: |", "|  |  |\n| :--: | ---: |\n| A | B |",0,1},
    {"|H\\|A|\tB| \t\n|:--:|\t---:|\t \none|\t`a|b` \t", "one|\t`a|b` \t\n|:--:|\t---:|\t ",0,0},
    {"| A | B |\n| :--: | ---: |\n| x |", "| A | B |\n| :--: | ---: |\n| x |\n|  |  |",2,1},
    {"| A | B |\n| :--: | ---: |\n| x | y | ignored &amp; |", "| A | B |\n| :--: | ---: |\n|  |  |\n| x | y | ignored &amp; |",1,1},
    {"| A | B |\n| :--: | ---: |\n|", "| A | B |\n| :--: | ---: |\n|  |  |\n|",1,1},
    {"| A | B | C |\n| :--: | ---: | --- |\n| x | y |\n| p | q | r |", "| x | y |  |\n| :--: | ---: | --- |\n| p | q | r |",0,0},
    {"| A | B |\n| :--: | ---: |\n| x\n| p | q |", "| x|  |\n| :--: | ---: |\n| p | q |",0,0},
    {"| A | B |\n| :--: | ---: |\n| x | y | ignored &amp; |\n| p | q |", "| x | y |\n| :--: | ---: |\n| p | q |",0,0},
    {"| A | B |\n| :--: | ---: |\n|\n| p | q |", "|  |  |\n| :--: | ---: |\n| p | q |",0,0},
    {"A | B | C\n:--: | ---: | ---\nx | y\np | q | r", "x | y|  |\n:--: | ---: | ---\np | q | r",0,0},
    {"A | B\n:--: | ---:\nx | y | ignored\np | q", "x | y |\n:--: | ---:\np | q",0,0}
};
static int row_command_id(xui_document d,uint64_t table,unsigned row,int insert)
{
    xui_document_transaction t;int result=xuiDocumentBeginTransaction(d,NULL,&t);if(result!=XUI_OK)return result;
    result=insert?xuiDocumentTxnInsertTableRow(t,table,row):xuiDocumentTxnDeleteTableRow(t,table,row);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
    xuiDocumentTxnRelease(t);return result;
}
static int row_command(xui_document d,unsigned row,int insert)
{
    xui_document_snapshot s;uint64_t table;TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);
    table=table_find(s,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);xuiDocumentSnapshotRelease(s);return row_command_id(d,table,row,insert);
}
static void row_golden(const char* before,const char* after,unsigned row,int insert)
{
    xui_document d=table_open(before,NULL),full;xui_document_snapshot a,b,original,restored;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    TABLE_CHECK(row_command(d,row,insert)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&a)==XUI_OK);table_source(a,after);
    full=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&b)==XUI_OK);table_equivalent(a,b);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,original);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,after);table_equivalent(restored,a);xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(a);xuiDocumentSnapshotRelease(b);xuiDocumentRelease(full);xuiDocumentRelease(d);
}
/* Physical prefixes and line endings are expanded equally; edited row bytes
 * above are independent literal goldens, not output from the implementation. */
static char* row_eof_wrap(const char* table,unsigned context,const char* ending,int bom,int final)
{
    char* source=table_wrap(table,context,ending,bom,final);char* tail=strstr(source,"end &amp;");size_t e=strlen(ending);
    TABLE_CHECK(tail);tail-=2*e;*tail=0;if(final){size_t n=strlen(source);memcpy(source+n,ending,e+1);}return source;
}
static const row_case extra_rows[]={
    {"H | B\n:--: | ---:","",0,0},
    {"H | B\r\n:--: | ---:\r\n","",0,0},
    {"\xef\xbb\xbfH | B\n:--: | ---:","\xef\xbb\xbf",0,0},
    {"> H | B\n> :--: | ---:","> ",0,0},
    {"> H | B\r\n> :--: | ---:\r\n","> \r\n",0,0},
    {"- H | B\n  :--: | ---:","- ",0,0},
    {"> - H | B\n>   :--: | ---:","> - ",0,0},
    {"use [^n].\n\n[^n]:\n    H | B\n    :--: | ---:","use [^n].\n\n[^n]:\n    ",0,0},
    {"> H | B\r\n> :--: | ---:\r> one | two\n> last | &amp;","> H | B\r\n> :--: | ---:\r> |  |  |\n> one | two\n> last | &amp;",1,1},
    {"> H | B\r\n> :--: | ---:\r> one | two\n> last | &amp;","> |  |  |\r\n> :--: | ---:\r> H | B\r\n> one | two\n> last | &amp;",0,1},
    {"> H | B\r\n> :--: | ---:\r> one | two\n> last | &amp;","> one | two\n> :--: | ---:\r> last | &amp;",0,0},
    {"-\tH | B\n\t:--: | ---:\n\tone | two", "-\tH | B\n\t:--: | ---:\n\t|  |  |\n\tone | two",1,1},
    {">\tH | B\n>\t:--: | ---:\n>\tone | two", ">\tone | two\n>\t:--: | ---:",0,0},
    {">\t-\tH | B\n>\t\t:--: | ---:\n>\t\tone | two", ">\t-\t|  |  |\n>\t\t:--: | ---:\n>\t\tH | B\n>\t\tone | two",0,1}
};
static void row_extra(void)
{
    unsigned i;for(i=0;i<sizeof(extra_rows)/sizeof(*extra_rows);i++){
        const row_case* c=&extra_rows[i];row_golden(c->before,c->after,c->row,c->insert);
    }
    puts("Table row: 14 direct BOM/whole-table deletion/empty container/mixed EOL/Tab goldens passed");
}
static void row_prefix_history(unsigned context,const char* ending,int bom,int prepare)
{
    char* before=table_wrap(row_cases[5].before,context,ending,bom,1);char* after=table_wrap(row_cases[5].after,context,ending,bom,1);
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
    TABLE_CHECK(row_command(d,0,1)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,shifted_after);
    full=table_open(shifted_after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&oracle)==XUI_OK);table_equivalent(current,oracle);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(oracle);xuiDocumentRelease(full);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,shifted_before);xuiDocumentSnapshotRelease(current);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_equivalent(current,original);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(original);xuiDocumentRelease(d);free(before);free(after);free(shifted_before);free(shifted_after);
}
static void row_compound(unsigned context)
{
    const char* result="|  |  |  |\n:--: | --- | ---:\none |  | `a|b`\nlast |  | &amp;";
    char* before=table_wrap(row_cases[0].before,context,"\r\n",1,1);char* after=table_wrap(result,context,"\r\n",1,1);
    xui_document d=table_open(before,NULL),full;xui_document_snapshot original,current,oracle;xui_document_transaction t;uint64_t table;xui_doc_stats_t stats={0};
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table&&xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    TABLE_CHECK(xuiDocumentTxnInsertTableRow(t,table,0)==XUI_OK&&xuiDocumentTxnInsertTableColumn(t,table,1)==XUI_OK&&xuiDocumentTxnDeleteTableRow(t,table,1)==XUI_OK&&xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    stats.iSize=sizeof(stats);TABLE_CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK&&stats.iUndoCount==1);
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);full=table_open(after,NULL);
    TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&oracle)==XUI_OK);table_equivalent(current,oracle);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(oracle);xuiDocumentRelease(full);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_equivalent(current,original);
    xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(original);xuiDocumentRelease(d);free(before);free(after);
}
static void row_faults(void)
{
    unsigned mode;for(mode=0;mode<6;mode++){
        const row_case* c=&row_cases[mode==0?1:mode==1?4:mode==2?5:mode==3?6:mode==4?13:15];
        char* before=table_wrap(c->before,mode&1?4:3,"\r\n",1,1);char* after=table_wrap(c->after,mode&1?4:3,"\r\n",1,1);long point;
        for(point=0;point<4096;point++){
            table_allocator f={-1,0};xui_document d=table_open(before,&f);xui_document_snapshot original,current;uint64_t revision,table;int result,undo;
            TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
            revision=xuiDocumentGetRevision(d);table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);undo=xuiDocumentCanUndo(d);
            f.remaining=point;result=row_command_id(d,table,c->row,c->insert);f.remaining=-1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result==XUI_OK)table_source(current,after);
            else{
                TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&xuiDocumentCanUndo(d)==undo);
                table_source(current,before);table_equivalent(current,original);
                TABLE_CHECK(row_command_id(d,table,c->row,c->insert)==XUI_OK);
                xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);
                TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d));
            }
            xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!f.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Table row mode %u: %ld OOM points atomic, retry/Undo and zero leaks\n",mode,point);free(before);free(after);
    }
}
/* Table payloads and edited payloads above remain original independent
 * literal goldens. Only physical container prefixes and EOLs are expanded. */
static char* deep_table_wrap(const char* table,unsigned kind,unsigned depth,const char* ending,int bom,int final)
{
    const char* begin="lead &amp;\n\n";
    const char* tail="\n\nend &amp;\n\n[keep]: <raw> 'title'";
    size_t lines=1,i,used=0,at=0,e=strlen(ending),bytes;
    char *lf,*result;unsigned line=0,j;
    for(i=0;table[i];i++)if(table[i]=='\n')lines++;
    bytes=strlen(begin)+strlen(table)+strlen(tail)+lines*depth*5+2;
    lf=malloc(bytes);TABLE_CHECK(lf);memcpy(lf+at,begin,strlen(begin));at+=strlen(begin);
    for(i=0;table[i];){
        for(j=0;j<depth;j++){
            const char* prefix=kind?(line?"     ":"123) "):">  ";
            size_t n=strlen(prefix);memcpy(lf+at,prefix,n);at+=n;
        }
        while(table[i]&&table[i]!='\n')lf[at++]=table[i++];
        if(table[i]){lf[at++]='\n';i++;line++;}
    }
    memcpy(lf+at,tail,strlen(tail));at+=strlen(tail);if(final)lf[at++]='\n';lf[at]=0;
    result=malloc(at*2+4);TABLE_CHECK(result);
    if(bom){memcpy(result,"\xef\xbb\xbf",3);used=3;}
    for(i=0;i<at;i++){
        if(lf[i]=='\n'){memcpy(result+used,ending,e);used+=e;}
        else result[used++]=lf[i];
    }
    result[used]=0;free(lf);return result;
}
static void deep_table_scope(xui_document_snapshot s,uint64_t table,unsigned kind,unsigned depth)
{
    xui_doc_node_info_t n={0};unsigned quotes=0,lists=0,items=0,steps=0;uint64_t id=table;
    while(id){
        n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(s,id,&n)==XUI_OK);
        quotes+=n.iKind==XUI_DOC_QUOTE;lists+=n.iKind==XUI_DOC_LIST;items+=n.iKind==XUI_DOC_LIST_ITEM;
        if(n.iKind==XUI_DOC_LIST)TABLE_CHECK((n.tAttributes.iFlags&XUI_DOC_ORDERED)&&n.tAttributes.iListStart==123);
        if(id==XUI_DOCUMENT_ROOT)break;
        id=n.iParentId;TABLE_CHECK(++steps<512);
    }
    TABLE_CHECK(id==XUI_DOCUMENT_ROOT);
    TABLE_CHECK(kind?(!quotes&&lists==depth&&items==depth):(quotes==depth&&!lists&&!items));
}
static void deep_table_golden(const char* before,const char* after,unsigned index,int insert,int row,unsigned kind,unsigned depth)
{
    xui_document d=table_open(before,NULL),full;
    xui_document_snapshot original,current,oracle,restored;
    uint64_t table,lead,tail,root_count; xui_doc_node_info_t root={0},node={0}; xui_doc_stats_t stats={0}; int result;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK);
    TABLE_CHECK(xuiDocumentMarkSaved(d,original)==XUI_OK);table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);
    deep_table_scope(original,table,kind,depth);
    root.iSize=sizeof(root);TABLE_CHECK(xuiDocumentSnapshotGetNode(original,XUI_DOCUMENT_ROOT,&root)==XUI_OK);
    root_count=root.iChildCount;TABLE_CHECK(root_count==3);
    TABLE_CHECK(xuiDocumentSnapshotGetChild(original,XUI_DOCUMENT_ROOT,0,&lead)==XUI_OK);
    TABLE_CHECK(xuiDocumentSnapshotGetChild(original,XUI_DOCUMENT_ROOT,root_count-1,&tail)==XUI_OK);
    node.iSize=sizeof(node);TABLE_CHECK(xuiDocumentSnapshotGetNode(original,lead,&node)==XUI_OK&&node.iKind==XUI_DOC_PARAGRAPH);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(original,tail,&node)==XUI_OK&&node.iKind==XUI_DOC_PARAGRAPH);
    result=row?row_command_id(d,table,index,insert):table_command_id(d,table,index,insert);
    if(result!=XUI_OK)fprintf(stderr,"deep table command result=%d row=%d kind=%u depth=%u index=%u insert=%d\n",result,row,kind,depth,index,insert);
    TABLE_CHECK(result==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(current,lead,&node)==XUI_OK&&node.iKind==XUI_DOC_PARAGRAPH);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(current,tail,&node)==XUI_OK&&node.iKind==XUI_DOC_PARAGRAPH);
    TABLE_CHECK(table_find(current,XUI_DOCUMENT_ROOT)==table);deep_table_scope(current,table,kind,depth);
    stats.iSize=sizeof(stats);TABLE_CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK&&stats.iUndoCount==1);
    full=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&oracle)==XUI_OK);table_equivalent(current,oracle);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,original);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);
    table_source(original,before);
    xuiDocumentSnapshotRelease(oracle);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(original);
    xuiDocumentRelease(full);xuiDocumentRelease(d);
}

static void deep_table_declared_limit(void)
{
    char* source=deep_table_wrap(cases[0].before,0,127,"\r\n",1,1);
    xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot before,after;uint64_t revision;int result;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
    TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&before)==XUI_OK);
    revision=xuiDocumentGetRevision(d);result=xuiDocumentLoadMarkdown(d,source,strlen(source));
    if(result!=XUI_DOC_ERROR_LIMIT)fprintf(stderr,"Expected declared128-depth LIMIT, actual=%d\n",result);
    TABLE_CHECK(result==XUI_DOC_ERROR_LIMIT&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentCanUndo(d));
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK);table_source(before,"");table_source(after,"");
    table_equivalent(before,after);xuiDocumentSnapshotRelease(before);xuiDocumentSnapshotRelease(after);xuiDocumentRelease(d);free(source);
    puts("Deep table qualification:127 quote levels plus table descendants hit declared128-depth LIMIT atomically; no published source/revision/history change");
}

static void row_deep_matrix(void)
{
    const char* endings[]={"\n","\r\n","\r"};
    const unsigned depths[2][4]={{1,84,85,86},{1,52,53,54}};
    unsigned kind,level,e,bom,final,i,total=0;
    deep_table_declared_limit();
    TABLE_CHECK(sizeof(cases)/sizeof(*cases)==21&&sizeof(row_cases)/sizeof(*row_cases)==19);
    for(kind=0;kind<2;kind++)for(level=0;level<4;level++){
        unsigned depth=depths[kind][level];
        for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++){
            for(i=0;i<sizeof(cases)/sizeof(*cases);i++){
                const table_case* c=&cases[i];char* before=deep_table_wrap(c->before,kind,depth,endings[e],bom,final);
                char* after=deep_table_wrap(c->after,kind,depth,endings[e],bom,final);
                deep_table_golden(before,after,c->column,c->insert,0,kind,depth);free(before);free(after);total++;
            }
            for(i=0;i<sizeof(row_cases)/sizeof(*row_cases);i++){
                const row_case* c=&row_cases[i];char* before=deep_table_wrap(c->before,kind,depth,endings[e],bom,final);
                char* after=deep_table_wrap(c->after,kind,depth,endings[e],bom,final);
                deep_table_golden(before,after,c->row,c->insert,1,kind,depth);free(before);free(after);total++;
            }
        }
        printf("Deep table kind=%u depth=%u:480 independent row/column/EOL/BOM/terminal goldens, scope/stable IDs/full-source-index/old-snapshot/shared-history passed\n",kind,depth);
        fflush(stdout);
    }
    TABLE_CHECK(total==3840);
    puts("Deep table:3840 independent source goldens, deep parser-qualified scope, stable outside/table IDs, full reparse/source index and one root Undo/Redo passed");
    return;
}

int main(int argc,char** argv)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,context,e,bom,final,eof,total=0,first=0,last=(unsigned)(sizeof(row_cases)/sizeof(*row_cases));
    if(argc==2&&!strcmp(argv[1],"deep")){row_deep_matrix();return 0;}
    if(argc==2&&!strcmp(argv[1],"ragged")){first=13;last=14;}else TABLE_CHECK(argc==1);
    for(fixture=first;fixture<last;fixture++)for(context=0;context<5;context++)
    for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++)for(eof=0;eof<2;eof++){
        const row_case* c=&row_cases[fixture];char* before=eof?row_eof_wrap(c->before,context,endings[e],bom,final):table_wrap(c->before,context,endings[e],bom,final);
        char* after=eof?row_eof_wrap(c->after,context,endings[e],bom,final):table_wrap(c->after,context,endings[e],bom,final);
        /* Removing the final physical row keeps the preceding row EOL. */
        if(fixture==4 && eof && !final){size_t n=strlen(after),bytes=strlen(endings[e]);memcpy(after+n,endings[e],bytes+1);}
        row_golden(before,after,c->row,c->insert);free(before);free(after);total++;
    }
    printf("Table row source: %u independent raw goldens/full HTML/source index/history passed\n",total);
    if(first)return 0;
    row_extra();
    for(context=0;context<5;context++){
        row_compound(context);
        for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++)row_prefix_history(context,endings[e],bom,final);
    }
    puts("Table row: 5 mixed row/column root transactions/one Undo, 60 SOURCE/Prepare prefix shifts and shared history passed");
    row_faults();row_deep_matrix();return 0;
}
