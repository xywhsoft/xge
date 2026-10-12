#define XUI_DOC_DELETE_SOURCE_HELPERS_ONLY
#include "xui_document_delete_source_cst_test.c"
#undef XUI_DOC_DELETE_SOURCE_HELPERS_ONLY
static uint64_t gap_last(xui_document_snapshot s,uint64_t id,uint32_t kind,uint32_t marks)
{
 xui_doc_node_info_t n={0};uint64_t i,result=0;n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(s,id,&n)==XUI_OK);
 if(n.iKind==kind&&(!marks||(n.tAttributes.iMarks&marks)==marks))result=id;
 for(i=0;i<n.iChildCount;i++){uint64_t child,found;TABLE_CHECK(xuiDocumentSnapshotGetChild(s,id,i,&child)==XUI_OK);found=gap_last(s,child,kind,marks);if(found)result=found;}return result;
}
static xui_doc_range_t gap_selection(xui_document d,xui_document_snapshot s,uint64_t first,uint64_t last,int reverse,uint64_t* offset)
{
 xui_doc_node_info_t a={0},b={0},parent={0};uint64_t i,child,left=UINT64_MAX,right=UINT64_MAX;xui_doc_range_t r;
 a.iSize=sizeof(a);b.iSize=sizeof(b);parent.iSize=sizeof(parent);TABLE_CHECK(xuiDocumentSnapshotGetNode(s,first,&a)==XUI_OK&&xuiDocumentSnapshotGetNode(s,last,&b)==XUI_OK&&a.iParentId==b.iParentId);
 TABLE_CHECK(xuiDocumentSnapshotGetNode(s,a.iParentId,&parent)==XUI_OK);
 for(i=0;i<parent.iChildCount;i++){TABLE_CHECK(xuiDocumentSnapshotGetChild(s,parent.iId,i,&child)==XUI_OK);if(child==first)left=i;if(child==last)right=i;}
 TABLE_CHECK(left!=UINT64_MAX&&right!=UINT64_MAX&&left<=right);
 memset(&r,0,sizeof(r));r.tAnchor.iSize=sizeof(r.tAnchor);r.tAnchor.iDocumentId=xuiDocumentGetIdentity(d);r.tAnchor.iRevision=xuiDocumentGetRevision(d);r.tAnchor.iNodeId=a.iParentId;r.tAnchor.iKind=XUI_DOC_POSITION_GAP;r.tAnchor.iOffset=left;r.tCaret=r.tAnchor;r.tCaret.iOffset=right+1;r.tCaret.iAffinity=XUI_DOC_AFTER;
 if(reverse){xui_doc_position_t swap=r.tAnchor;r.tAnchor=r.tCaret;r.tCaret=swap;}if(offset)*offset=left;return r;
}
static void gap_golden(const char* before,const char* after,uint32_t first_kind,uint32_t first_marks,uint32_t last_kind,uint32_t last_marks,int multiple,int reverse)
{
 xui_document d=table_open(before,NULL);xui_document_snapshot old,current,restored;xui_document_transaction t;xui_document_change_set change;xui_doc_range_t range;xui_doc_position_t caret={0},tail,mapped;uint64_t first,last,left,source;int quality;const char* suffix=strstr(before,"end");
 TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
 first=delete_find(old,XUI_DOCUMENT_ROOT,first_kind,first_marks);last=multiple?gap_last(old,XUI_DOCUMENT_ROOT,last_kind,last_marks):first;TABLE_CHECK(first&&last);
 range=gap_selection(d,old,first,last,reverse,&left);
 TABLE_CHECK(suffix&&strstr(after,"end")&&xuiDocumentSourceToPosition(old,(uint64_t)(suffix-before)+1,&tail,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT);
 TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK&&xuiDocumentTxnReplaceRange(t,&range,"",0,&caret)==XUI_OK&&xuiDocumentTxnCommit(t,&change)==XUI_OK);xuiDocumentTxnRelease(t);
 TABLE_CHECK(caret.iKind==XUI_DOC_POSITION_GAP&&caret.iNodeId==range.tAnchor.iNodeId&&caret.iOffset==left&&caret.iAffinity==XUI_DOC_BEFORE);
 TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);
 TABLE_CHECK(xuiDocumentMapPosition(change,&tail,&mapped,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&mapped.iNodeId==tail.iNodeId&&mapped.iOffset==tail.iOffset);
 TABLE_CHECK(xuiDocumentPositionToSource(current,&mapped,&source,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&source==(uint64_t)(strstr(after,"end")-after)+1);
 TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentCanUndo(d)&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
 TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,after);xuiDocumentSnapshotRelease(restored);table_source(old,before);
 xuiDocumentChangeSetRelease(change);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);
}
static const delete_case gap_extra[]={
 {"> a ![](<img> 'T') end\n","> a  end\n",XUI_DOC_IMAGE,0},
 {"a\nend\n","aend\n",XUI_DOC_SOFT_BREAK,0},
 {"> a\n> end\n","> aend\n",XUI_DOC_SOFT_BREAK,0},
 {"a  \nend\n","aend\n",XUI_DOC_HARD_BREAK,0},
 {"> a  \n> end\n","> aend\n",XUI_DOC_HARD_BREAK,0}
};
typedef struct gap_multi {const char* before,*after;uint32_t first_kind,first_marks,last_kind,last_marks;}gap_multi;
static const gap_multi multi[]={
 {"a ![img](<uri> 'T') **&amp;** end\n","a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a ![img](<uri> 'T') **&amp;** end\n","> a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"- a ![img](<uri> 'T') **&amp;** end\n","- a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> - a ![img](<uri> 'T') **&amp;** end\n","> - a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"ref[^outer]\n\n[^outer]:\n    a ![img](<uri> 'T') **&amp;** end\n","ref[^outer]\n\n[^outer]:\n    a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a **r&amp;aw** end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_BOLD,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a `raw` $x+y$ end\n","> a  end\n",XUI_DOC_TEXT,XUI_DOC_CODE,XUI_DOC_MATH,0},
 {"> ![img](u) **&amp;** end\n","> &#32;end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a ![img](u)\n> **&amp;** end\n","> a  end\n",XUI_DOC_IMAGE,0,XUI_DOC_TEXT,XUI_DOC_BOLD},
 {"> a\n> ![img](u) end\n","> a end\n",XUI_DOC_SOFT_BREAK,0,XUI_DOC_IMAGE,0}
};

static int gap_command(xui_document d,const xui_doc_range_t* range,xui_doc_position_t* caret)
{
 xui_document_transaction t;int result=xuiDocumentBeginTransaction(d,NULL,&t);
 if(result!=XUI_OK)return result;
 result=xuiDocumentTxnReplaceRange(t,range,"",0,caret);
 if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
 /* A failed operation or publication poisons this transaction. Repeating the
  * commit must never publish its private, partly constructed draft. */
 if(result!=XUI_OK)TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==result);
 xuiDocumentTxnRelease(t);return result;
}
static void gap_faults(void)
{
 unsigned mode;
 for(mode=0;mode<4;mode++){
  const delete_case* single=mode==0?&delete_cases[0]:mode==1?&gap_extra[0]:&gap_extra[4];
  const gap_multi* group=&multi[7];
  char* before=delete_ending(mode==3?group->before:single->before,"\r\n",1);
  char* after=delete_ending(mode==3?group->after:single->after,"\r\n",1);long point;
  for(point=0;point<4096;point++){
   table_allocator f={-1,0};xui_document d=table_open(before,&f);xui_document_snapshot old,current,restored;
   xui_doc_range_t range;xui_doc_position_t caret={0};uint64_t first,last,left,revision;int result;
   TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
   first=delete_find(old,XUI_DOCUMENT_ROOT,mode==3?group->first_kind:single->kind,mode==3?group->first_marks:single->marks);
   last=mode==3?gap_last(old,XUI_DOCUMENT_ROOT,group->last_kind,group->last_marks):first;TABLE_CHECK(first&&last);
   range=gap_selection(d,old,first,last,(int)(mode%2),&left);revision=xuiDocumentGetRevision(d);
   f.remaining=point;result=gap_command(d,&range,&caret);f.remaining=-1;
   TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
   if(result!=XUI_OK){
    TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d)&&!xuiDocumentCanRedo(d));
    table_source(current,before);table_equivalent(current,old);
    TABLE_CHECK(gap_command(d,&range,&caret)==XUI_OK);table_source(current,before);
    xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
   }
   TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1&&xuiDocumentIsDirty(d)&&xuiDocumentCanUndo(d)&&!xuiDocumentCanRedo(d));
   TABLE_CHECK(caret.iNodeId==range.tAnchor.iNodeId&&caret.iKind==XUI_DOC_POSITION_GAP&&caret.iOffset==left&&caret.iAffinity==XUI_DOC_BEFORE);
   table_source(current,after);integration_equivalent(current,after);table_source(old,before);
   TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d)&&xuiDocumentCanRedo(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
   table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
   TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,after);integration_equivalent(restored,after);xuiDocumentSnapshotRelease(restored);
   table_source(current,after);table_source(old,before);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!f.live);
   if(result==XUI_OK)break;
  }
  TABLE_CHECK(point<4096);printf("Inline GAP mode %u: %ld OOM points atomic/poisoned commit/same-document retry/Undo/Redo/zero leaks\n",mode,point);free(before);free(after);
 }
}


typedef struct mixed_case {const char* before;const char* after[2];}mixed_case;
static const mixed_case mixed_cases[]={
 {"lead &amp; ![](<img> 'T') end\n",{"le end\n","lead &amp; end\n"}},
 {"> lead &amp; ![](<img> 'T') end\n",{"> le end\n","> lead &amp; end\n"}},
 {"- lead &amp; ![](<img> 'T') end\n",{"- le end\n","- lead &amp; end\n"}},
 {"> - lead &amp; ![](<img> 'T') end\n",{"> - le end\n","> - lead &amp; end\n"}},
 {"ref[^outer]\n\n[^outer]:\n    lead &amp; ![](<img> 'T') end\n",{"ref[^outer]\n\n[^outer]:\n    le end\n","ref[^outer]\n\n[^outer]:\n    lead &amp; end\n"}},
 {"# lead &amp; ![](<img> 'T') end #\n",{"# le end #\n","# lead &amp; end #\n"}},
 {"lead &amp; ![](<img> 'T') end\n======\n",{"le end\n======\n","lead &amp; end\n======\n"}},
 {"> lead &amp; **![](<img> 'T')** end\n",{"> le end\n","> lead &amp; end\n"}}
};
static xui_doc_range_t mixed_selection(xui_document d,xui_document_snapshot s,const char* before,unsigned mode,int reverse)
{
 uint64_t image=delete_find(s,XUI_DOCUMENT_ROOT,XUI_DOC_IMAGE,0);xui_doc_range_t range=gap_selection(d,s,image,image,0,NULL);int quality;
 if(!mode)TABLE_CHECK(xuiDocumentSourceToPosition(s,(uint64_t)(strstr(before,"lead")-before)+2,&range.tAnchor,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT);
 else TABLE_CHECK(xuiDocumentSourceToPosition(s,(uint64_t)(strstr(before," end")-before)+1,&range.tCaret,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT);
 if(reverse){xui_doc_position_t swap=range.tAnchor;range.tAnchor=range.tCaret;range.tCaret=swap;}return range;
}
static void mixed_caret(xui_document d,xui_document_snapshot current,xui_doc_position_t* caret,const char* after,unsigned mode)
{
 uint64_t source,expected=(uint64_t)((mode?strstr(after," end")+1:strstr(after,"le end")+2)-after);int quality;
 /* The public structural-command contract returns a draft caret; after Commit
  * its revision is set to the committed revision before snapshot lookup. */
 caret->iRevision=xuiDocumentGetRevision(d);
 TABLE_CHECK(caret->iKind==XUI_DOC_POSITION_TEXT||caret->iKind==XUI_DOC_POSITION_GAP);
 /* GAP is a structural boundary: the API conservatively reports APPROXIMATE.
  * Its actual byte boundary is still checked against the independent golden. */
 TABLE_CHECK(xuiDocumentPositionToSource(current,caret,&source,&quality)==XUI_OK&&
  quality==(caret->iKind==XUI_DOC_POSITION_GAP?XUI_DOC_MAP_APPROXIMATE:XUI_DOC_MAP_EXACT)&&source==expected);
}
static void mixed_golden(const char* before,const char* after,unsigned mode,int reverse)
{
 xui_document d=table_open(before,NULL);xui_document_snapshot old,current,restored;xui_document_transaction t;xui_document_change_set change;
 xui_doc_range_t range;xui_doc_position_t caret={0},tail,mapped;uint64_t source;int quality;
 TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);
 range=mixed_selection(d,old,before,mode,reverse);
 TABLE_CHECK(xuiDocumentSourceToPosition(old,(uint64_t)(strstr(before," end")-before)+2,&tail,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT);
 TABLE_CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK&&xuiDocumentTxnReplaceRange(t,&range,"",0,&caret)==XUI_OK&&xuiDocumentTxnCommit(t,&change)==XUI_OK);xuiDocumentTxnRelease(t);
 TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,after);integration_equivalent(current,after);mixed_caret(d,current,&caret,after,mode);
 TABLE_CHECK(xuiDocumentMapPosition(change,&tail,&mapped,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&mapped.iNodeId==tail.iNodeId&&mapped.iOffset==tail.iOffset-mode);
 TABLE_CHECK(xuiDocumentPositionToSource(current,&mapped,&source,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&source==(uint64_t)(strstr(after," end")-after)+2);
 TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentCanUndo(d)&&!xuiDocumentIsDirty(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
 TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,after);integration_equivalent(restored,after);xuiDocumentSnapshotRelease(restored);
 table_source(old,before);table_source(current,after);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentChangeSetRelease(change);xuiDocumentRelease(d);
}
static void mixed_faults(void)
{
 unsigned mode;
 for(mode=0;mode<2;mode++){
  const mixed_case* c=&mixed_cases[mode?7:4];char* before=delete_ending(c->before,"\r\n",1),*after=delete_ending(c->after[mode],"\r\n",1);long point;
  for(point=0;point<4096;point++){
   table_allocator f={-1,0};xui_document d=table_open(before,&f);xui_document_snapshot old,current,restored;xui_doc_range_t range;xui_doc_position_t caret={0};uint64_t revision;int result;
   TABLE_CHECK(xuiDocumentClearHistory(d)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&old)==XUI_OK&&xuiDocumentMarkSaved(d,old)==XUI_OK);range=mixed_selection(d,old,before,mode,(int)mode);revision=xuiDocumentGetRevision(d);
   f.remaining=point;result=gap_command(d,&range,&caret);f.remaining=-1;TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
   if(result!=XUI_OK){
    TABLE_CHECK(result==XUI_ERROR_OUT_OF_MEMORY&&xuiDocumentGetRevision(d)==revision&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d)&&!xuiDocumentCanRedo(d));table_source(current,before);table_equivalent(current,old);
    TABLE_CHECK(gap_command(d,&range,&caret)==XUI_OK);table_source(current,before);xuiDocumentSnapshotRelease(current);TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
   }
   TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1&&xuiDocumentIsDirty(d)&&xuiDocumentCanUndo(d)&&!xuiDocumentCanRedo(d));table_source(current,after);integration_equivalent(current,after);mixed_caret(d,current,&caret,after,mode);
   TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK&&!xuiDocumentIsDirty(d)&&!xuiDocumentCanUndo(d)&&xuiDocumentCanRedo(d)&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,before);table_equivalent(restored,old);xuiDocumentSnapshotRelease(restored);
   TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK&&xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);table_source(restored,after);integration_equivalent(restored,after);xuiDocumentSnapshotRelease(restored);
   table_source(current,after);table_source(old,before);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);xuiDocumentRelease(d);TABLE_CHECK(!f.live);if(result==XUI_OK)break;
  }
  TABLE_CHECK(point<4096);printf("Mixed TEXT/GAP mode %u: %ld OOM points atomic/poisoned commit/retry/Undo/Redo/zero leaks\n",mode,point);free(before);free(after);
 }
}
static void mixed_matrix(void)
{
 const char* endings[]={"\n","\r\n","\r"};unsigned f,m,e,b,r,total=0;
 for(f=0;f<sizeof(mixed_cases)/sizeof(*mixed_cases);f++)for(m=0;m<2;m++)for(e=0;e<3;e++)for(b=0;b<2;b++)for(r=0;r<2;r++){
  char* before=delete_ending(mixed_cases[f].before,endings[e],(int)b),*after=delete_ending(mixed_cases[f].after[m],endings[e],(int)b);
  fprintf(stderr,"Mixed range fixture=%u side=%u ending=%u BOM=%u reverse=%u\n",f,m,e,b,r);mixed_golden(before,after,m,(int)r);free(before);free(after);total++;
 }
 mixed_faults();printf("Mixed TEXT/GAP: %u raw goldens/full reparse/byte roles/stable exact tail/draft caret/one Undo passed\n",total);
}


#ifndef XUI_DOC_INLINE_RANGE_HELPERS_ONLY
int main(void)
{
 const char* endings[]={"\n","\r\n","\r"};unsigned f,e,b,r,total=0,mtotal=0;
 for(f=0;f<sizeof(delete_cases)/sizeof(*delete_cases)+sizeof(gap_extra)/sizeof(*gap_extra);f++)for(e=0;e<3;e++)for(b=0;b<2;b++)for(r=0;r<2;r++){
  const delete_case* c=f<sizeof(delete_cases)/sizeof(*delete_cases)?&delete_cases[f]:&gap_extra[f-sizeof(delete_cases)/sizeof(*delete_cases)];char* before=delete_ending(c->before,endings[e],(int)b),*after=delete_ending(c->after,endings[e],(int)b);
  fprintf(stderr,"Gap source fixture=%u ending=%u BOM=%u reverse=%u\n",f,e,b,r);
  gap_golden(before,after,c->kind,c->marks,c->kind,c->marks,0,(int)r);free(before);free(after);total++;
 }
 for(f=0;f<sizeof(multi)/sizeof(*multi);f++)for(e=0;e<3;e++)for(b=0;b<2;b++)for(r=0;r<2;r++){
  const gap_multi* c=&multi[f];char* before=delete_ending(c->before,endings[e],(int)b),*after=delete_ending(c->after,endings[e],(int)b);
  fprintf(stderr,"Multi gap source fixture=%u ending=%u BOM=%u reverse=%u\n",f,e,b,r);
  gap_golden(before,after,c->first_kind,c->first_marks,c->last_kind,c->last_marks,1,(int)r);free(before);free(after);mtotal++;
 }
 gap_faults();
 mixed_matrix();
 printf("Inline GAP deletion: %u single and %u contiguous multi-node raw goldens/semantics/byte roles/stable exact tail/caret/one Undo passed\n",total,mtotal);return 0;
}

#endif
