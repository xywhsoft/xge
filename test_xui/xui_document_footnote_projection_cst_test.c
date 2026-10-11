#define main table_driver_main
#include "xui_document_table_source_cst_test.c"
#undef main
typedef struct footnote_case {const char* before;const char* after;const char* label;} footnote_case;
static const footnote_case footnote_cases[]={
    {"a[^drop] end\n\n[^drop]: One &amp;\n","a end\n\n[^drop]: One &amp;\n","drop"},
    {"a[^drop] b[^drop]\n\n[^drop]: One &amp;\n","a b[^drop]\n\n[^drop]: One &amp;\n","drop"},
    {"a[^a] b[^b]\n\n[^a]: A &amp;\n[^b]: B &amp;\n","a b[^b]\n\n[^a]: A &amp;\n[^b]: B &amp;\n","a"},
    {"a[^a] b[^b]\n\n[^a]: A [^c].\n[^b]: B [^c].\n[^c]: C [^c].\n","a b[^b]\n\n[^a]: A [^c].\n[^b]: B [^c].\n[^c]: C [^c].\n","a"},
    {"a[^a] b[^b]\n\n[^a]: A [^c].\n[^b]: B &amp;\n[^c]: C [^a].\n","a b[^b]\n\n[^a]: A [^c].\n[^b]: B &amp;\n[^c]: C [^a].\n","a"},
    {"a[^drop] end\n\n[^drop]: One &amp;\n[^drop]: unused duplicate &amp;\n[link]: <https://example.invalid> 'raw'\n","a end\n\n[^drop]: One &amp;\n[^drop]: unused duplicate &amp;\n[link]: <https://example.invalid> 'raw'\n","drop"},
    {"a[^a] b[^b] c[^a]\n\n[^a]: A &amp;\n[^b]: B &amp;\n","a b[^b] c[^a]\n\n[^a]: A &amp;\n[^b]: B &amp;\n","a"}
};
static uint64_t footnote_reference(xui_document_snapshot s,uint64_t id,const char* label)
{
    xui_doc_node_info_t n={0};uint64_t i;n.iSize=sizeof(n);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s,id,&n)==XUI_OK);
    if(n.iKind==XUI_DOC_FOOTNOTE_REF && n.sInfo&&!strcmp(n.sInfo,label))return id;
    for(i=0;i<n.iChildCount;i++){
        uint64_t child,found;TABLE_CHECK(xuiDocumentSnapshotGetChild(s,id,i,&child)==XUI_OK);
        found=footnote_reference(s,child,label);if(found)return found;
    }
    return 0;
}
static char* footnote_wrap(const char* source,unsigned context,const char* ending,int bom)
{
    const char* definitions=strstr(source,"\n\n[^");char head[4096];char *base,*result;size_t bytes,i,at,e=strlen(ending);
    TABLE_CHECK(definitions&&(size_t)(definitions-source)<sizeof(head));
    bytes=(size_t)(definitions-source);memcpy(head,source,bytes);head[bytes]=0;
    base=table_wrap(head,context,ending,bom,0);bytes=strlen(base);
    result=malloc(bytes+strlen(definitions)*2+1);TABLE_CHECK(result);memcpy(result,base,bytes);at=bytes;
    for(i=0;definitions[i];i++){if(definitions[i]=='\n'){memcpy(result+at,ending,e);at+=e;}else result[at++]=definitions[i];}
    result[at]=0;free(base);return result;
}
static char* footnote_source(xui_document_snapshot s)
{
    uint64_t bytes;char* result;TABLE_CHECK(xuiDocumentSnapshotCopySource(s,NULL,0,&bytes)==XUI_OK);
    result=malloc((size_t)bytes+1);TABLE_CHECK(result&&xuiDocumentSnapshotCopySource(s,result,bytes+1,&bytes)==XUI_OK);return result;
}
static int footnote_delete(xui_document d,uint64_t reference,int range,int reverse,xui_document_change_set* changes)
{
    xui_document_transaction t;xui_document_snapshot s;xui_doc_node_info_t node={0},parent={0};uint64_t i,child;
    xui_doc_range_t selected={0};xui_doc_position_t caret;int result;
    if(range){
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);node.iSize=sizeof(node);parent.iSize=sizeof(parent);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s,reference,&node)==XUI_OK&&xuiDocumentSnapshotGetNode(s,node.iParentId,&parent)==XUI_OK);
        for(i=0;i<parent.iChildCount;i++){TABLE_CHECK(xuiDocumentSnapshotGetChild(s,node.iParentId,i,&child)==XUI_OK);if(child==reference)break;}
        TABLE_CHECK(i<parent.iChildCount);xuiDocumentSnapshotRelease(s);
        selected.tAnchor.iSize=sizeof(selected.tAnchor);selected.tAnchor.iDocumentId=xuiDocumentGetIdentity(d);
        selected.tAnchor.iRevision=xuiDocumentGetRevision(d);selected.tAnchor.iNodeId=node.iParentId;
        selected.tAnchor.iKind=XUI_DOC_POSITION_GAP;selected.tAnchor.iOffset=i+(reverse?1:0);
        selected.tCaret=selected.tAnchor;selected.tCaret.iOffset=i+(reverse?0:1);
    }
    result=xuiDocumentBeginTransaction(d,NULL,&t);if(result!=XUI_OK)return result;
    result=range?xuiDocumentTxnReplaceRange(t,&selected,"",0,&caret):xuiDocumentTxnDeleteNode(t,reference);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(t,changes);
    xuiDocumentTxnRelease(t);return result;
}

static void footnote_plain_spans(char* html)
{
    size_t from=0,to=0;
    while(html[from]){
        if(!strncmp(html+from,"<span>",6))from+=6;
        else if(!strncmp(html+from,"</span>",7))from+=7;
        else html[to++]=html[from++];
    }
    html[to]=0;
}
static void footnote_indexes(xui_document_snapshot a,xui_document_snapshot b)
{
    xui_document_source_index ai,bi;xui_doc_source_index_info_t x={0},y={0};
    xui_doc_source_token_t p={0},q={0};uint64_t i=0,j=0,at=0,k;
    x.iSize=sizeof(x);y.iSize=sizeof(y);p.iSize=sizeof(p);q.iSize=sizeof(q);
    TABLE_CHECK(xuiDocumentSnapshotCreateSourceIndex(a,NULL,&ai)==XUI_OK&&xuiDocumentSnapshotCreateSourceIndex(b,NULL,&bi)==XUI_OK);
    TABLE_CHECK(xuiDocumentSourceIndexGetInfo(ai,&x)==XUI_OK&&xuiDocumentSourceIndexGetInfo(bi,&y)==XUI_OK&&x.iSourceBytes==y.iSourceBytes);
    while(at<x.iSourceBytes){
        TABLE_CHECK(i<x.iTokenCount&&j<y.iTokenCount);
        TABLE_CHECK(xuiDocumentSourceIndexGetToken(ai,i,&p)==XUI_OK&&xuiDocumentSourceIndexGetToken(bi,j,&q)==XUI_OK);
        TABLE_CHECK(p.iSourceStart<=at&&q.iSourceStart<=at&&p.iSourceEnd>at&&q.iSourceEnd>at);
        TABLE_CHECK(p.iKind==q.iKind&&p.iDetail==q.iDetail&&p.iFlags==q.iFlags);
        at=p.iSourceEnd<q.iSourceEnd?p.iSourceEnd:q.iSourceEnd;if(at==p.iSourceEnd)i++;if(at==q.iSourceEnd)j++;
    }
    TABLE_CHECK(i==x.iTokenCount&&j==y.iTokenCount);
    for(k=0;k<x.iBindingCount;k++){
        xui_doc_source_binding_t binding={0};xui_doc_node_info_t node={0};binding.iSize=sizeof(binding);node.iSize=sizeof(node);
        TABLE_CHECK(xuiDocumentSourceIndexGetBinding(ai,k,&binding)==XUI_OK&&binding.tRange.iSourceStart<=binding.tRange.iSourceEnd&&binding.tRange.iSourceEnd<=x.iSourceBytes);
        if(binding.tRange.iNodeId)TABLE_CHECK(xuiDocumentSnapshotGetNode(a,binding.tRange.iNodeId,&node)==XUI_OK);
    }
    xuiDocumentSourceIndexRelease(ai);xuiDocumentSourceIndexRelease(bi);
}

static void footnote_golden(unsigned fixture,unsigned context,const char* ending,int bom,int mode)
{
    const footnote_case* c=&footnote_cases[fixture];char *before=footnote_wrap(c->before,context,ending,bom),*expected=footnote_wrap(c->after,context,ending,bom),*actual;
    xui_document d=table_open(before,NULL),oracle,semantic;xui_document_snapshot old,current,parsed,expected_semantic,restored;
    uint64_t ref,ah,bh;char *ahtml,*bhtml;const char* definitions=strstr(before,"[keep]:");int result;
    TABLE_CHECK(definitions);
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
    ref=footnote_reference(old,XUI_DOCUMENT_ROOT,c->label);TABLE_CHECK(ref);
    result=footnote_delete(d,ref,mode!=0,mode==2,NULL);if(result)fprintf(stderr,"footnote fixture=%u context=%u mode=%d result=%d\n",fixture,context,mode,result);
    TABLE_CHECK(result==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);actual=footnote_source(current);
    TABLE_CHECK(strstr(actual,definitions)!=NULL);oracle=table_open(actual,NULL);semantic=table_open(expected,NULL);
    TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&parsed)==XUI_OK&&xuiDocumentAcquireSnapshot(semantic,&expected_semantic)==XUI_OK);
    footnote_indexes(current,parsed);
    TABLE_CHECK(xuiDocumentExportHtml(current,&ahtml,&ah)==XUI_OK&&xuiDocumentExportHtml(expected_semantic,&bhtml,&bh)==XUI_OK);
    footnote_plain_spans(ahtml);footnote_plain_spans(bhtml);ah=strlen(ahtml);bh=strlen(bhtml);
    if(ah!=bh||memcmp(ahtml,bhtml,(size_t)(ah<bh?ah:bh)))fprintf(stderr,"HTML actual=[%s]\nexpected=[%s]\n",ahtml,bhtml);
    TABLE_CHECK(ah==bh&&!memcmp(ahtml,bhtml,(size_t)ah));xuiDocumentFreeBuffer(ahtml);xuiDocumentFreeBuffer(bhtml);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,actual);table_equivalent(restored,current);
    xuiDocumentSnapshotRelease(restored);xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(parsed);xuiDocumentSnapshotRelease(expected_semantic);
    xuiDocumentRelease(semantic);xuiDocumentRelease(oracle);xuiDocumentRelease(d);free(before);free(expected);free(actual);
}
static uint64_t footnote_note(xui_document_snapshot s,const char* label)
{
    xui_doc_node_info_t root={0},node={0};uint64_t i,id;root.iSize=sizeof(root);node.iSize=sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s,XUI_DOCUMENT_ROOT,&root)==XUI_OK);
    for(i=0;i<root.iChildCount;i++){
        TABLE_CHECK(xuiDocumentSnapshotGetChild(s,XUI_DOCUMENT_ROOT,i,&id)==XUI_OK&&xuiDocumentSnapshotGetNode(s,id,&node)==XUI_OK);
        if(node.iKind==XUI_DOC_FOOTNOTE&&node.sInfo&&!strcmp(node.sInfo,label))return id;
    }
    return 0;
}
static void footnote_identity(unsigned fixture,unsigned context,const char* ending,int bom,int mode)
{
    const footnote_case* c=&footnote_cases[fixture];char* before=footnote_wrap(c->before,context,ending,bom),*actual;
    xui_document d=table_open(before,NULL);xui_document_snapshot old,current,restored;xui_document_change_set change;
    xui_doc_position_t caret,mapped;xui_doc_change_info_t info={0};xui_doc_node_info_t node={0};
    uint64_t a,b,ref,source,i,deleted=0;int mapping;
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK);
    a=footnote_note(old,"a");b=footnote_note(old,"b");ref=footnote_reference(old,XUI_DOCUMENT_ROOT,"a");TABLE_CHECK(a&&b&&ref);
    source=(uint64_t)(strstr(before,fixture==6?"B &amp;":"B [^c].")-before)+1;
    TABLE_CHECK(xuiDocumentSourceToPosition(old,source,&caret,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT);
    TABLE_CHECK(footnote_delete(d,ref,mode!=0,mode==2,&change)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
    TABLE_CHECK(footnote_note(current,"b")==b);node.iSize=sizeof(node);
    if(fixture==6)TABLE_CHECK(footnote_note(current,"a")==a);
    else TABLE_CHECK(!footnote_note(current,"a")&&xuiDocumentSnapshotGetNode(current,a,&node)==XUI_ERROR_NOT_FOUND);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(old,a,&node)==XUI_OK&&node.iKind==XUI_DOC_FOOTNOTE);
    TABLE_CHECK(xuiDocumentMapPosition(change,&caret,&mapped,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT&&mapped.iNodeId==caret.iNodeId&&mapped.iOffset==caret.iOffset);
    actual=footnote_source(current);source=(uint64_t)(strstr(actual,fixture==6?"B &amp;":"B [^c].")-actual)+1;
    TABLE_CHECK(xuiDocumentPositionToSource(current,&mapped,&i,&mapping)==XUI_OK&&mapping==XUI_DOC_MAP_EXACT&&i==source);
    info.iSize=sizeof(info);TABLE_CHECK(xuiDocumentChangeSetGetInfo(change,&info)==XUI_OK);
    for(i=0;i<info.iOperationCount;i++)if(info.pOperations[i].iKind==XUI_DOC_OP_DELETE&&info.pOperations[i].iNodeId==a)deleted++;
    TABLE_CHECK(deleted<=1);
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    TABLE_CHECK(footnote_note(restored,"a")==a&&footnote_note(restored,"b")==b);table_equivalent(restored,old);
    xuiDocumentSnapshotRelease(restored);xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(current);xuiDocumentChangeSetRelease(change);xuiDocumentRelease(d);free(before);free(actual);
}
static void footnote_prepare_range(xui_document d,xui_document_snapshot s,uint64_t reference,int reverse,xui_doc_range_t* selected)
{
    xui_doc_node_info_t node={0},parent={0};uint64_t i,child;node.iSize=sizeof(node);parent.iSize=sizeof(parent);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s,reference,&node)==XUI_OK&&xuiDocumentSnapshotGetNode(s,node.iParentId,&parent)==XUI_OK);
    for(i=0;i<parent.iChildCount;i++){TABLE_CHECK(xuiDocumentSnapshotGetChild(s,node.iParentId,i,&child)==XUI_OK);if(child==reference)break;}
    TABLE_CHECK(i<parent.iChildCount);memset(selected,0,sizeof(*selected));selected->tAnchor.iSize=sizeof(selected->tAnchor);
    selected->tAnchor.iDocumentId=xuiDocumentGetIdentity(d);selected->tAnchor.iRevision=xuiDocumentGetRevision(d);
    selected->tAnchor.iNodeId=node.iParentId;selected->tAnchor.iKind=XUI_DOC_POSITION_GAP;selected->tAnchor.iOffset=i+(reverse?1:0);
    selected->tCaret=selected->tAnchor;selected->tCaret.iOffset=i+(reverse?0:1);
}
static int footnote_prepared_command(xui_document d,uint64_t reference,const xui_doc_range_t* range)
{
    xui_document_transaction t;xui_doc_position_t caret;int result=xuiDocumentBeginTransaction(d,NULL,&t);
    if(result!=XUI_OK)return result;
    result=range?xuiDocumentTxnReplaceRange(t,range,"",0,&caret):xuiDocumentTxnDeleteNode(t,reference);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
    xuiDocumentTxnRelease(t);return result;
}
static void footnote_faults(void)
{
    unsigned mode;for(mode=0;mode<3;mode++){
        const footnote_case* c=&footnote_cases[mode?3:5];char* before=footnote_wrap(c->before,mode?4:3,"\r\n",1);
        char* expected=footnote_wrap(c->after,mode?4:3,"\r\n",1);long point;
        for(point=0;point<4096;point++){
            table_allocator f={-1,0};xui_document d=table_open(before,&f),oracle;xui_document_snapshot old,current,full;
            xui_doc_range_t range;uint64_t ref,revision;int result;char *actual,*a,*b;uint64_t an,bn;
            TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
            ref=footnote_reference(old,XUI_DOCUMENT_ROOT,c->label);TABLE_CHECK(ref);revision=xuiDocumentGetRevision(d);
            if(mode)footnote_prepare_range(d,old,ref,mode==2,&range);
            f.remaining=point;result=footnote_prepared_command(d,ref,mode?&range:NULL);f.remaining=-1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result!=XUI_OK){
                TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d));
                table_source(current,before);table_equivalent(current,old);
                TABLE_CHECK(footnote_prepared_command(d,ref,mode?&range:NULL)==XUI_OK);
                xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            }
            actual=footnote_source(current);TABLE_CHECK(strstr(actual,strstr(before,"[keep]:"))!=NULL);
            oracle=table_open(actual,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);footnote_indexes(current,full);
            xuiDocumentSnapshotRelease(full);xuiDocumentRelease(oracle);oracle=table_open(expected,NULL);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK&&xuiDocumentExportHtml(current,&a,&an)==XUI_OK&&xuiDocumentExportHtml(full,&b,&bn)==XUI_OK);
            footnote_plain_spans(a);footnote_plain_spans(b);TABLE_CHECK(!strcmp(a,b));xuiDocumentFreeBuffer(a);xuiDocumentFreeBuffer(b);
            xuiDocumentSnapshotRelease(full);xuiDocumentRelease(oracle);free(actual);
            TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d));
            xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,before);table_equivalent(current,old);
            xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!f.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Generic footnote mode %u: %ld OOM points atomic/retry/Undo/zero leaks\n",mode,point);free(before);free(expected);
    }
}
int main(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned fixture,context,e,bom,mode,total=0;
    for(fixture=0;fixture<sizeof(footnote_cases)/sizeof(*footnote_cases);fixture++)for(context=0;context<5;context++)for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(mode=0;mode<3;mode++){
        footnote_golden(fixture,context,endings[e],bom,(int)mode);total++;
    }
    printf("Generic footnote projection: %u node/forward/reverse range cases; exact definitions, independent semantics, full reparse byte roles/no orphan bindings, snapshots and Undo/Redo passed\n",total);
    for(fixture=3;fixture<=6;fixture+=3)for(context=0;context<5;context++)for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(mode=0;mode<3;mode++)
        footnote_identity(fixture,context,endings[e],(int)bom,(int)mode);
    puts("Generic footnote: 180 surviving IDs, exact caret maps, dropped-node absence, old snapshots, single derived deletion and Undo passed");
    footnote_faults();
    return 0;
}
