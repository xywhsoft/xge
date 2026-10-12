#define main table_column_driver_main
#include "xui_document_table_source_cst_test.c"
#undef main
typedef struct reference_case {const char* before;const char* after;unsigned index;int column;} reference_case;
static const reference_case reference_cases[]={
    {"H | B\n:--: | ---:\nx | [^drop]\nlast | &amp;\n\n[^drop]: note &amp;", "H | B\n:--: | ---:\nlast | &amp;\n\n[^drop]: note &amp;",1,0},
    {"H | B\n:--: | ---:\nx | [^drop]\nlast | &amp;\n\n[^drop]: note &amp;", "H |\n:--: |\nx |\nlast |\n\n[^drop]: note &amp;",1,1},
    {"[^drop] | B\n:--: | ---:\nx | y\nlast | &amp;\n\n[^drop]: note &amp;", "x | y\n:--: | ---:\nlast | &amp;\n\n[^drop]: note &amp;",0,0},
    {"H | B\n:--: | ---:\n[^a] | [^b]\n[^b] | [^a]\n\n[^a]: A &amp;\n[^b]: B &amp;", "H | B\n:--: | ---:\n[^b] | [^a]\n\n[^a]: A &amp;\n[^b]: B &amp;",1,0},
    {"H | B\n:--: | ---:\nx | [^a]\ny | [^b]\n\n[^a]: A [^c].\n[^b]: B [^c].\n[^c]: C &amp; [^c].", "H | B\n:--: | ---:\ny | [^b]\n\n[^a]: A [^c].\n[^b]: B [^c].\n[^c]: C &amp; [^c].",1,0},
    {"H | B\n:--: | ---:\nx | [^a]\ny | [^b]\n\n[^a]: A [^c].\n[^b]: B &amp;\n[^c]: C &amp; [^a].", "H | B\n:--: | ---:\ny | [^b]\n\n[^a]: A [^c].\n[^b]: B &amp;\n[^c]: C &amp; [^a].",1,0},
    {"H | B\n:--: | ---:\nx | [^drop]\ny | [^keep]\n\n[^drop]: first &amp;\n[^drop]: unused duplicate &amp;\n[^keep]: retained &amp;", "H | B\n:--: | ---:\ny | [^keep]\n\n[^drop]: first &amp;\n[^drop]: unused duplicate &amp;\n[^keep]: retained &amp;",1,0}
};
static int reference_command_id(xui_document d,uint64_t table,unsigned index,int column)
{
    xui_document_transaction t;int result=xuiDocumentBeginTransaction(d,NULL,&t);if(result!=XUI_OK)return result;
    result=column?xuiDocumentTxnDeleteTableColumn(t,table,index):xuiDocumentTxnDeleteTableRow(t,table,index);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
    xuiDocumentTxnRelease(t);return result;
}
static void reference_golden(const char* before,const char* after,unsigned index,int column)
{
    xui_document d=table_open(before,NULL),full;xui_document_snapshot original,a,b,restored;uint64_t table;int result;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
    table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);result=reference_command_id(d,table,index,column);
    if(result!=XUI_OK)fprintf(stderr,"table reference command result %d\n",result);
    TABLE_CHECK(result==XUI_OK&&xuiDocumentAcquireSnapshot(d,&a)==XUI_OK);table_source(a,after);
    full=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(full,&b)==XUI_OK);table_equivalent(a,b);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,original);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,after);table_equivalent(restored,a);xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(a);xuiDocumentSnapshotRelease(b);xuiDocumentRelease(full);xuiDocumentRelease(d);
}
static uint64_t reference_note(xui_document_snapshot s,const char* label)
{
    xui_doc_node_info_t root={0},node={0};uint64_t i;root.iSize=sizeof(root);node.iSize=sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s,XUI_DOCUMENT_ROOT,&root)==XUI_OK);
    for(i=0;i<root.iChildCount;i++){
        uint64_t id;TABLE_CHECK(xuiDocumentSnapshotGetChild(s,XUI_DOCUMENT_ROOT,i,&id)==XUI_OK&&xuiDocumentSnapshotGetNode(s,id,&node)==XUI_OK);
        if(node.iKind==XUI_DOC_FOOTNOTE && node.sInfo&&!strcmp(node.sInfo,label))return id;
    }
    return 0;
}
static char* reference_wrap(const char* text,unsigned context,const char* ending,int bom,int final)
{
    const char* definitions=strstr(text,"\n\n[^");char head[4096];char *base,*result;size_t bytes,i,at,e=strlen(ending);
    if(context!=4)return table_wrap(text,context,ending,bom,final);
    TABLE_CHECK(definitions && (size_t)(definitions-text)<sizeof(head));
    bytes=(size_t)(definitions-text);memcpy(head,text,bytes);head[bytes]=0;
    base=table_wrap(head,context,ending,bom,0);bytes=strlen(base);result=malloc(bytes+strlen(definitions)*2+e+1);TABLE_CHECK(result);
    memcpy(result,base,bytes);at=bytes;
    for(i=0;definitions[i];i++){if(definitions[i]=='\n'){memcpy(result+at,ending,e);at+=e;}else result[at++]=definitions[i];}
    if(final){memcpy(result+at,ending,e);at+=e;}result[at]=0;free(base);return result;
}
static void reference_identity(unsigned fixture,unsigned context,const char* ending,int bom)
{
    const reference_case* c=&reference_cases[fixture];char* before=reference_wrap(c->before,context,ending,bom,1);
    char* after=reference_wrap(c->after,context,ending,bom,1);xui_document d=table_open(before,NULL);xui_document_snapshot old,current,restored;
    xui_document_transaction t;xui_document_change_set change;xui_doc_position_t caret,mapped;xui_doc_change_info_t info={0};
    xui_doc_node_info_t node={0};uint64_t a,b,table,source,offset;unsigned deleted=0;uint64_t i;int mapping;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK);
    a=reference_note(old,"a");b=reference_note(old,"b");TABLE_CHECK(a&&b);table=table_find(old,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);
    offset=(uint64_t)(strstr(before,fixture==3?"B &amp;":"B [^c].")-before)+1;
    TABLE_CHECK(xuiDocumentSourceToPosition(old,offset,&caret,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT);
    TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK&&xuiDocumentTxnDeleteTableRow(t,table,1)==XUI_OK&&xuiDocumentTxnCommit(t,&change)==XUI_OK);
    xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);
    TABLE_CHECK(reference_note(current,"b")==b);node.iSize=sizeof(node);
    if(fixture==3)TABLE_CHECK(reference_note(current,"a")==a);
    else TABLE_CHECK(!reference_note(current,"a")&&xuiDocumentSnapshotGetNode(current,a,&node)==XUI_ERROR_NOT_FOUND);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(old,a,&node)==XUI_OK&&node.iKind==XUI_DOC_FOOTNOTE);
    TABLE_CHECK(xuiDocumentMapPosition(change,&caret,&mapped,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT&&mapped.iNodeId==caret.iNodeId&&mapped.iOffset==caret.iOffset);
    TABLE_CHECK(xuiDocumentPositionToSource(current,&mapped,&source,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT);
    TABLE_CHECK(source==(uint64_t)(strstr(after,fixture==3?"B &amp;":"B [^c].")-after)+1);
    info.iSize=sizeof(info);TABLE_CHECK(xuiDocumentChangeSetGetInfo(change,&info)==XUI_OK);
    for(i=0;i<info.iOperationCount;i++)if(info.pOperations[i].iKind==XUI_DOC_OP_DELETE&&info.pOperations[i].iNodeId==a)deleted++;
    TABLE_CHECK(deleted<=1);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    TABLE_CHECK(reference_note(restored,"a")==a&&reference_note(restored,"b")==b);table_equivalent(restored,old);
    xuiDocumentSnapshotRelease(restored);xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentChangeSetRelease(change);xuiDocumentRelease(d);free(before);free(after);
}
static void reference_faults(void)
{
    unsigned mode;for(mode=0;mode<2;mode++){
        const reference_case* c=&reference_cases[mode?1:3];char* before=reference_wrap(c->before,mode?3:4,"\r\n",1,1);
        char* after=reference_wrap(c->after,mode?3:4,"\r\n",1,1);long point;
        for(point=0;point<4096;point++){
            table_allocator f={-1,0};xui_document d=table_open(before,&f);xui_document_snapshot original,current;uint64_t revision,table;int result;
            TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&original)==XUI_OK&&xuiDocumentMarkSaved(d,original)==XUI_OK);
            revision=xuiDocumentGetRevision(d);table=table_find(original,XUI_DOCUMENT_ROOT);TABLE_CHECK(table);
            f.remaining=point;result=reference_command_id(d,table,c->index,c->column);f.remaining=-1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result==XUI_OK)table_source(current,after);
            else{
                TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d));
                table_source(current,before);table_equivalent(current,original);
                TABLE_CHECK(reference_command_id(d,table,c->index,c->column)==XUI_OK);
                xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);
                TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d));
            }
            xuiDocumentSnapshotRelease(original);xuiDocumentSnapshotRelease(current);xuiDocumentRelease(d);TABLE_CHECK(!f.live);if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Table reference mode %u: %ld OOM points atomic/retry/Undo/zero leaks\n",mode,point);free(before);free(after);
    }
}
int main(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,context,e,bom,final,total=0;
    for(fixture=0;fixture<sizeof(reference_cases)/sizeof(*reference_cases);fixture++)for(context=0;context<5;context++)
    for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(final=0;final<2;final++){
        const reference_case* c=&reference_cases[fixture];char* before=table_wrap(c->before,context,endings[e],bom,final);
        char* after=table_wrap(c->after,context,endings[e],bom,final);
        reference_golden(before,after,c->index,c->column);free(before);free(after);total++;
        if(context==4){
            before=reference_wrap(c->before,context,endings[e],bom,final);after=reference_wrap(c->after,context,endings[e],bom,final);
            reference_golden(before,after,c->index,c->column);free(before);free(after);total++;
        }
    }
    reference_golden("[^drop] | B\n:--: | ---:\n\n[^drop]: note &amp;","\n[^drop]: note &amp;",0,0);
    printf("Table reference: %u raw goldens/last-use removal/reordering/dependency chains/cycles/duplicate definitions/full HTML/source index/history passed\n",total+1);
    for(fixture=3;fixture<=4;fixture++)for(context=0;context<5;context++)for(e=0;e<3;e++)for(bom=0;bom<2;bom++)reference_identity(fixture,context,endings[e],bom);
    puts("Table reference: 60 surviving footer IDs/caret maps/dropped-node absence/old snapshots/single derived deletion/Undo cases passed");
    reference_faults();return 0;
}
