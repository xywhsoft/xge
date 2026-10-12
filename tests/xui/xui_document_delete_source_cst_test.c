#define main table_driver_main
#include "xui_document_table_source_cst_test.c"
#undef main
static uint64_t delete_find(xui_document_snapshot s,uint64_t id,uint32_t kind,uint32_t marks)
{
    xui_doc_node_info_t n={0};uint64_t i;n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(s,id,&n)==XUI_OK);
    if(n.iKind==kind&&(!marks||(n.tAttributes.iMarks&marks)==marks)&&
       (kind!=XUI_DOC_FOOTNOTE_REF||(n.sInfo&&!strcmp(n.sInfo,"drop"))))return id;
    for(i=0;i<n.iChildCount;i++){
        uint64_t child,found;TABLE_CHECK(xuiDocumentSnapshotGetChild(s,id,i,&child)==XUI_OK);
        found=delete_find(s,child,kind,marks);if(found)return found;
    }
    return 0;
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

static void delete_golden(const char* before,const char* after,uint32_t kind,uint32_t marks)
{
    xui_document d=table_open(before,NULL),oracle;xui_document_snapshot old,current,full,restored;xui_document_transaction t;xui_document_change_set change;xui_doc_node_info_t target={0},parent={0};xui_doc_position_t gap,mapped,tail,tail_mapped;uint64_t id,index,child,source;int quality;const char* suffix=strstr(before,"end");
    TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK);
    id=delete_find(old,XUI_DOCUMENT_ROOT,kind,marks);TABLE_CHECK(id);
    target.iSize=sizeof(target);parent.iSize=sizeof(parent);TABLE_CHECK(xuiDocumentSnapshotGetNode(old,id,&target)==XUI_OK&&xuiDocumentSnapshotGetNode(old,target.iParentId,&parent)==XUI_OK);
    for(index=0;index<parent.iChildCount;index++){TABLE_CHECK(xuiDocumentSnapshotGetChild(old,target.iParentId,index,&child)==XUI_OK);if(child==id)break;}
    TABLE_CHECK(index<parent.iChildCount);memset(&gap,0,sizeof(gap));gap.iSize=sizeof(gap);gap.iDocumentId=xuiDocumentGetIdentity(d);gap.iRevision=xuiDocumentGetRevision(d);gap.iNodeId=target.iParentId;gap.iKind=XUI_DOC_POSITION_GAP;gap.iOffset=index+1;
    if(suffix)TABLE_CHECK(xuiDocumentSourceToPosition(old,(uint64_t)(suffix-before)+1,&tail,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT);
    TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK&&xuiDocumentTxnDeleteNode(t,id)==XUI_OK&&xuiDocumentTxnCommit(t,&change)==XUI_OK);
    xuiDocumentTxnRelease(t);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);
    if(xuiDocumentSnapshotGetNode(current,target.iParentId,&parent)==XUI_OK){
        TABLE_CHECK(xuiDocumentMapPosition(change,&gap,&mapped,&quality)==XUI_OK);
        if(mapped.iNodeId!=gap.iNodeId||mapped.iKind!=XUI_DOC_POSITION_GAP||mapped.iOffset!=index)fprintf(stderr,"gap old=%llu mapped=%llu expected=%llu quality=%d\n",(unsigned long long)gap.iOffset,(unsigned long long)mapped.iOffset,(unsigned long long)index,quality);
        TABLE_CHECK(mapped.iNodeId==gap.iNodeId&&mapped.iKind==XUI_DOC_POSITION_GAP&&mapped.iOffset==index);
    }
    if(suffix){TABLE_CHECK(xuiDocumentMapPosition(change,&tail,&tail_mapped,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&tail_mapped.iNodeId==tail.iNodeId&&tail_mapped.iOffset==tail.iOffset);
        TABLE_CHECK(xuiDocumentPositionToSource(current,&tail_mapped,&source,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&source==(uint64_t)(strstr(after,"end")-after)+1);}
    oracle=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);
    {char *a,*b;uint64_t an,bn;TABLE_CHECK(xuiDocumentExportHtml(current,&a,&an)==XUI_OK&&xuiDocumentExportHtml(full,&b,&bn)==XUI_OK);
     footnote_plain_spans(a);footnote_plain_spans(b);TABLE_CHECK(!strcmp(a,b));xuiDocumentFreeBuffer(a);xuiDocumentFreeBuffer(b);footnote_indexes(current,full);}
    TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,after);
    xuiDocumentSnapshotRelease(restored);xuiDocumentSnapshotRelease(full);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);
    xuiDocumentRelease(oracle);xuiDocumentChangeSetRelease(change);xuiDocumentRelease(d);
}

typedef struct delete_case {const char* before;const char* after;uint32_t kind,marks;} delete_case;
static const delete_case delete_cases[]={
 {"> a[^drop] end\n\n[^drop]: One &amp;\n","> a end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"- a[^drop] end\n\n[^drop]: One &amp;\n","- a end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> - a[^drop] end\n\n[^drop]: One &amp;\n","> - a end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"  > a[^drop] end\n\n[^drop]: One &amp;\n","  > a end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"-\ta[^drop] end\n\n[^drop]: One &amp;\n","-\ta end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"use [^outer].\n\n[^outer]:\n    a[^drop] end\n\n[^drop]: One &amp;\n","use [^outer].\n\n[^outer]:\n    a end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> a[^drop] end\n> \n> [hidden]: <raw> 'title'\n\n[^drop]: One &amp;\n","> a end\n> \n> [hidden]: <raw> 'title'\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> a ![raw &amp;](<img> 'T') end\n","> a  end\n",XUI_DOC_IMAGE,0},
 {"> a **r&amp;aw** end\n","> a **&amp;aw** end\n",XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a **&amp;** end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a `two &amp;` end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_CODE},
 {"> a `` two &amp; `` end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_CODE},
 {"> a $x + y$ end\n","> a  end\n",XUI_DOC_MATH,0},
{"a ![raw &amp;](<img> 'T') end\n","a  end\n",XUI_DOC_IMAGE,0},
 {"# a ![raw &amp;](<img> 'T') end #\n","# a  end #\n",XUI_DOC_IMAGE,0},
 {"a ![raw &amp;](<img> 'T') end\n======\n","a  end\n======\n",XUI_DOC_IMAGE,0},
 {"> a **[^drop]** end\n\n[^drop]: One &amp;\n","> a  end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> a ***raw*** end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_BOLD|XUI_DOC_ITALIC},
 {"> a [raw][L] end\n\n[L]: <uri> 'Title'\n","> a  end\n\n[L]: <uri> 'Title'\n",XUI_DOC_TEXT,XUI_DOC_LINK},
 {"> a [raw](<uri> 'T') end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_LINK},
 {"> a <https://example.org> end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_LINK},
 {"> a <kbd>raw</kbd> end\n","> a raw</kbd> end\n",XUI_DOC_HTML,0},
 {"> a ~~raw~~ end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_STRIKE},
 {"> a **`raw`** end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_CODE|XUI_DOC_BOLD},
 {"> a [![raw &amp;](<img> 'T')](<url> 'Link') end\n","> a  end\n",XUI_DOC_IMAGE,0},
 {"> a ![raw][I] end\n\n[I]: <img> 'Title'\n","> a  end\n\n[I]: <img> 'Title'\n",XUI_DOC_IMAGE,0},
 {"> a &amp; end\n","> &amp; end\n",XUI_DOC_TEXT,0},
 {"> a **[^drop]** end[^drop]\n\n[^drop]: One &amp;\n","> a  end[^drop]\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> a ***&amp;*** end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_BOLD|XUI_DOC_ITALIC},
 {"> a [**raw**](<uri> 'T') end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_LINK|XUI_DOC_BOLD},
 {"> &amp; end\n","> &#32;end\n",XUI_DOC_TEXT,0},
 {"> [^drop] end\n\n[^drop]: One &amp;\n","> &#32;end\n\n[^drop]: One &amp;\n",XUI_DOC_FOOTNOTE_REF,0},
 {"> ![raw &amp;](<img> 'T') end\n","> &#32;end\n",XUI_DOC_IMAGE,0},
 {"> `` raw `` end\n","> &#32;end\n",XUI_DOC_TEXT,XUI_DOC_CODE},
 {"> **&amp;** end\n","> &#32;end\n",XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> $x+y$ end\n","> &#32;end\n",XUI_DOC_MATH,0},
 {"# ![raw](img) end #\n","# &#32;end #\n",XUI_DOC_IMAGE,0}
};
static char* delete_ending(const char* text,const char* ending,int bom)
{
 size_t n=strlen(text),i,at=bom?3:0,e=strlen(ending);char* result=malloc(n*2+4);TABLE_CHECK(result);
 if(bom)memcpy(result,"\xef\xbb\xbf",3);
 for(i=0;i<n;i++){if(text[i]=='\n'){memcpy(result+at,ending,e);at+=e;}else result[at++]=text[i];}result[at]=0;return result;
}
static int delete_command(xui_document d,uint64_t id)
{
    xui_document_transaction t;int result=xuiDocumentBeginTransaction(d,NULL,&t);
    if(result!=XUI_OK)return result;
    result=xuiDocumentTxnDeleteNode(t,id);if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
    xuiDocumentTxnRelease(t);return result;
}
static void delete_faults(void)
{
    unsigned mode;const unsigned fixtures[]={0,7,11,30};
    for(mode=0;mode<4;mode++){
        const delete_case* c=&delete_cases[fixtures[mode]];char* before=delete_ending(c->before,"\r\n",1),*after=delete_ending(c->after,"\r\n",1);long point;
        for(point=0;point<4096;point++){
            table_allocator f={-1,0};xui_document d=table_open(before,&f),oracle;xui_document_snapshot old,current,full;uint64_t id,revision;int result;
            TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
            id=delete_find(old,XUI_DOCUMENT_ROOT,c->kind,c->marks);TABLE_CHECK(id);revision=xuiDocumentGetRevision(d);
            f.remaining=point;result=delete_command(d,id);f.remaining=-1;TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            if(result!=XUI_OK){
                TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d));
                table_source(current,before);table_equivalent(current,old);TABLE_CHECK(delete_command(d,id)==XUI_OK);
                xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            }
            table_source(current,after);oracle=table_open(after,NULL);TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);footnote_indexes(current,full);
            xuiDocumentSnapshotRelease(full);xuiDocumentRelease(oracle);TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d));
            xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,before);table_equivalent(current,old);
            xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!f.live);if(result==XUI_OK)break;
        }
        TABLE_CHECK(point<4096);printf("Single source deletion mode %u: %ld OOM points atomic/retry/Undo/zero leaks\n",mode,point);free(before);free(after);
    }
}
static char* integration_prefix(const char* source,const char* prefix,int bom)
{
 size_t at=bom?3:0,n=strlen(source),p=strlen(prefix);char* s=malloc(n+p+1);TABLE_CHECK(s);
 memcpy(s,source,at);memcpy(s+at,prefix,p);memcpy(s+at+p,source+at,n-at+1);return s;
}
static void integration_equivalent(xui_document_snapshot current,const char* expected)
{
 xui_document oracle=table_open(expected,NULL);xui_document_snapshot full;char *a,*b;uint64_t an,bn;
 TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);
 TABLE_CHECK(xuiDocumentExportHtml(current,&a,&an)==XUI_OK&&xuiDocumentExportHtml(full,&b,&bn)==XUI_OK);
 footnote_plain_spans(a);footnote_plain_spans(b);TABLE_CHECK(!strcmp(a,b));footnote_indexes(current,full);
 xuiDocumentFreeBuffer(a);xuiDocumentFreeBuffer(b);xuiDocumentSnapshotRelease(full);xuiDocumentRelease(oracle);
}
static void integration_shift(const delete_case* c,const char* ending,int bom,unsigned mode)
{
 char* before=delete_ending(c->before,ending,bom),*after=delete_ending(c->after,ending,bom);
 char* prefix=delete_ending("# shift ä¸­ &amp;\n\n",ending,0);
 char* shifted=integration_prefix(before,prefix,bom),*result_source=integration_prefix(after,prefix,bom);
 xui_document d=table_open(before,NULL);xui_document_snapshot old,mid,current;xui_doc_node_info_t n={0};uint64_t id;
 TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
 id=delete_find(old,XUI_DOCUMENT_ROOT,c->kind,c->marks);TABLE_CHECK(id);
 if(mode<2){
  xui_doc_txn_desc_t td={0};td.iSize=sizeof(td);td.iDomain=XUI_DOC_SOURCE;
  if(!mode){xui_document_transaction t;TABLE_CHECK(xuiDocumentBeginTransaction(d,&td,&t)==XUI_OK&&xuiDocumentTxnReplaceSource(t,bom?3:0,bom?3:0,prefix,strlen(prefix))==XUI_OK&&xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);}
  else{xui_document_prepare p;xui_doc_source_patch_t patch={0};patch.iSize=sizeof(patch);patch.iStart=patch.iEnd=bom?3:0;patch.sText=prefix;patch.iTextBytes=strlen(prefix);
   TABLE_CHECK(xuiDocumentPrepareSource(d,&td,&patch,1,&p)==XUI_OK&&xuiDocumentPrepareRun(p)==XUI_OK&&xuiDocumentPreparePublish(d,p,NULL)==XUI_OK);xuiDocumentPrepareRelease(p);}
  TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&mid)==XUI_OK);table_source(mid,shifted);integration_equivalent(mid,shifted);
  n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(mid,id,&n)==XUI_OK);
 }else{
  char* native;uint64_t bytes;xui_document loaded;
  TABLE_CHECK(xuiDocumentSerialize(old,&native,&bytes)==XUI_OK&&xuiDocumentDeserialize(NULL,native,bytes,&loaded)==XUI_OK);
  xuiDocumentFreeBuffer(native);xuiDocumentRelease(d);d=loaded;
  TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&mid)==XUI_OK);table_source(mid,before);integration_equivalent(mid,before);
  id=delete_find(mid,XUI_DOCUMENT_ROOT,c->kind,c->marks);TABLE_CHECK(id);
 }
 TABLE_CHECK(delete_command(d,id)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
 table_source(current,mode<2?result_source:after);integration_equivalent(current,mode<2?result_source:after);xuiDocumentSnapshotRelease(current);
 TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,mode<2?shifted:before);table_equivalent(current,mid);xuiDocumentSnapshotRelease(current);
 if(mode<2){TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,before);table_equivalent(current,old);xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);}
 TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,mode<2?result_source:after);xuiDocumentSnapshotRelease(current);
 table_source(old,before);xuiDocumentSnapshotRelease(mid);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);
 free(before);free(after);free(prefix);free(shifted);free(result_source);
}
static void integration_compound(const char* ending,int bom)
{
 const char* raw="> a ![raw](<img> 'T') **&amp;** `raw` end\n\n[hidden]: <uri> 'Title'\n";
 const char* wanted="> a    end\n\n[hidden]: <uri> 'Title'\n";
 char* before=delete_ending(raw,ending,bom),*after=delete_ending(wanted,ending,bom);
 xui_document d=table_open(before,NULL);xui_document_snapshot old,current;xui_document_transaction t;uint64_t image,bold,code;
 TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
 image=delete_find(old,XUI_DOCUMENT_ROOT,XUI_DOC_IMAGE,0);bold=delete_find(old,XUI_DOCUMENT_ROOT,XUI_DOC_TEXT,XUI_DOC_BOLD);code=delete_find(old,XUI_DOCUMENT_ROOT,XUI_DOC_TEXT,XUI_DOC_CODE);TABLE_CHECK(image&&bold&&code);
 TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK&&xuiDocumentTxnDeleteNode(t,image)==XUI_OK&&xuiDocumentTxnDeleteNode(t,bold)==XUI_OK&&xuiDocumentTxnDeleteNode(t,code)==XUI_OK&&xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
 TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);xuiDocumentSnapshotRelease(current);
 TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentCanUndo(d)&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,before);table_equivalent(current,old);xuiDocumentSnapshotRelease(current);
 TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);xuiDocumentSnapshotRelease(current);table_source(old,before);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);free(before);free(after);
}

#ifndef XUI_DOC_DELETE_SOURCE_HELPERS_ONLY
int main(void)
{
 const char* endings[]={"\n","\r\n","\r"};unsigned fixture,e,bom,total=0;
 for(fixture=0;fixture<sizeof(delete_cases)/sizeof(*delete_cases);fixture++)for(e=0;e<3;e++)for(bom=0;bom<2;bom++){
  const delete_case* c=&delete_cases[fixture];char* before=delete_ending(c->before,endings[e],(int)bom),*after=delete_ending(c->after,endings[e],(int)bom);
  delete_golden(before,after,c->kind,c->marks);free(before);free(after);total++;
 }
 printf("Single node source deletion: %u independent raw goldens/semantic HTML/byte roles/identity caret and parent-gap maps/Undo/Redo passed\n",total);delete_faults();
 {unsigned fixtures[]={0,7,11,30},f,m,n=0;
  for(f=0;f<4;f++)for(e=0;e<3;e++)for(bom=0;bom<2;bom++)for(m=0;m<3;m++){
   integration_shift(&delete_cases[fixtures[f]],endings[e],(int)bom,m);n++;
  }
  for(e=0;e<3;e++)for(bom=0;bom<2;bom++)integration_compound(endings[e],(int)bom);
  printf("Single-leaf deletion integration: %u SOURCE/Prepare/Native cases plus 6 compound root transactions and shared history passed\n",n);
 }
 return 0;
}

#endif
