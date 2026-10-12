#define XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
#include "xui_document_footnote_local_reference_diagnostic_test.c"
#undef XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY

typedef struct local_fault {
    table_allocator allocation;
    long cancel_at;
    xui_document document;
} local_fault;
static void* local_allocate(void* user,size_t bytes)
{
    local_fault* f=user;
    if(f->cancel_at==0){f->cancel_at=-1;TABLE_CHECK(f->document && xuiDocumentCancelPrepare(f->document)==XUI_OK);}
    else if(f->cancel_at>0)f->cancel_at--;
    return table_alloc(&f->allocation,bytes);
}
static void local_release(void* user,void* p){local_fault* f=user;table_free(&f->allocation,p);}
static xui_document local_saved(local_fault* f,xui_document_snapshot* saved)
{
    xui_doc_desc_t desc={0};xui_document d;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
    desc.onAlloc=local_allocate;desc.onFree=local_release;desc.pAllocatorUser=f;
    TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,"saved **old**\n",14)==XUI_OK &&
        xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,saved)==XUI_OK && xuiDocumentMarkSaved(d,*saved)==XUI_OK);
    return d;
}
static void local_unchanged(xui_document d,xui_document_snapshot saved,uint64_t revision)
{
    xui_document_snapshot current;TABLE_CHECK(xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) &&
        !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
    table_source(current,"saved **old**\n");table_equivalent(current,saved);xuiDocumentSnapshotRelease(current);
}
static void local_loaded(xui_document d,const struct local_reference_case* c)
{
    xui_document_snapshot current;xui_doc_position_t p;xui_doc_node_info_t n={0};int quality;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);table_source(current,c->source);integration_equivalent(current,c->source);
    TABLE_CHECK(xuiDocumentSourceToPosition(current,(uint64_t)(strstr(c->source,c->target)-c->source)+1,&p,&quality)==XUI_OK && p.iKind==XUI_DOC_POSITION_TEXT);
    n.iSize=sizeof(n);TABLE_CHECK(xuiDocumentSnapshotGetNode(current,p.iNodeId,&n)==XUI_OK && n.tAttributes.iMarks==c->marks &&
        !strcmp(n.sResource,c->destination) && !strcmp(n.sTitle,c->title));xuiDocumentSnapshotRelease(current);
}
static void local_body_edit_faults(void)
{
    const struct body_fault_case {const char *before,*after;} cases[]={
        {"top\n\n[^n]: [a]: <uri> 'title'\n","top\n\n[^n]: a]: <uri> 'title'\n"},
        {"[outside][a]\n\n[^n]: [a]: <uri> 'title'\n\n[a]: /second\n",
         "[outside][a]\n\n[^n]: a]: <uri> 'title'\n\n[a]: /second\n"}
    };
    for(unsigned fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++)for(unsigned mode=0;mode<3;mode++){
        const struct body_fault_case* c=cases+fixture;long point;
        for(point=0;point<10000;point++){
            local_fault f={{-1,0},-1,NULL};xui_doc_desc_t desc={0};xui_document d;
            xui_document_snapshot saved,current,restored;xui_document_transaction t=NULL;xui_document_prepare p=NULL;
            xui_doc_txn_desc_t td={0};xui_doc_source_patch_t patch={0};uint64_t revision;int result;
            desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
            desc.onAlloc=local_allocate;desc.onFree=local_release;desc.pAllocatorUser=&f;
            TABLE_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,c->before,strlen(c->before))==XUI_OK &&
                xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&saved)==XUI_OK && xuiDocumentMarkSaved(d,saved)==XUI_OK);
            revision=xuiDocumentGetRevision(d);td.iSize=sizeof(td);td.iDomain=XUI_DOC_SOURCE;
            patch.iSize=sizeof(patch);patch.iStart=(uint64_t)(strstr(c->before,"[a]:")-c->before);patch.iEnd=patch.iStart+1;patch.sText="";
            if(!mode){
                TABLE_CHECK(xuiDocumentBeginTransaction(d,&td,&t)==XUI_OK);f.allocation.remaining=point;
                result=xuiDocumentTxnReplaceSource(t,patch.iStart,patch.iEnd,"",0);
                if(result==XUI_OK)result=xuiDocumentTxnCommit(t,NULL);
                f.allocation.remaining=-1;
                if(result!=XUI_OK)TABLE_CHECK(xuiDocumentTxnCommit(t,NULL)==result);
                xuiDocumentTxnRelease(t);t=NULL;
            }else{
                TABLE_CHECK(xuiDocumentPrepareSource(d,&td,&patch,1,&p)==XUI_OK);
                if(mode==2){f.document=d;f.cancel_at=point;}else f.allocation.remaining=point;
                result=xuiDocumentPrepareRun(p);f.document=NULL;f.cancel_at=f.allocation.remaining=-1;
            }
            TABLE_CHECK(result==XUI_OK || result==(mode==2?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            if(mode || result!=XUI_OK){
                TABLE_CHECK(xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                    !xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
                table_source(current,c->before);table_equivalent(current,saved);xuiDocumentSnapshotRelease(current);
            }
            if(result!=XUI_OK){
                if(!mode){
                    TABLE_CHECK(xuiDocumentBeginTransaction(d,&td,&t)==XUI_OK && xuiDocumentTxnReplaceSource(t,patch.iStart,patch.iEnd,"",0)==XUI_OK &&
                        xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
                }else{
                    xui_doc_prepare_info_t info={0};xui_document_prepare retry=NULL;info.iSize=sizeof(info);
                    TABLE_CHECK(xuiDocumentPrepareGetInfo(p,&info)==XUI_OK && info.iResult==result && info.iState==XUI_DOC_PREPARE_FAILED &&
                        xuiDocumentPrepareRun(p)==XUI_ERROR_INVALID_STATE);
                    if(mode==2){
                        TABLE_CHECK(xuiDocumentPrepareContinueSource(d,p,NULL,0,&retry)==XUI_DOC_ERROR_STALE && !retry && xuiDocumentCancelPrepare(d)==XUI_OK);
                        xuiDocumentPrepareRelease(p);TABLE_CHECK(xuiDocumentPrepareSource(d,&td,&patch,1,&p)==XUI_OK);
                    }else{
                        TABLE_CHECK(xuiDocumentPrepareContinueSource(d,p,NULL,0,&retry)==XUI_OK);xuiDocumentPrepareRelease(p);p=retry;
                    }
                    TABLE_CHECK(xuiDocumentPrepareRun(p)==XUI_OK);
                }
            }
            if(mode){TABLE_CHECK(xuiDocumentPreparePublish(d,p,NULL)==XUI_OK);xuiDocumentPrepareRelease(p);}
            TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);
            table_source(current,c->after);integration_equivalent(current,c->after);
            {xui_doc_source_info_t info={0};info.iSize=sizeof(info);
                TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(current,&info)==XUI_OK && info.iReferenceDefinitionCount==fixture+1);}
            if(fixture){xui_doc_position_t pos;xui_doc_node_info_t node={0};int quality;node.iSize=sizeof(node);
                TABLE_CHECK(xuiDocumentSourceToPosition(current,2,&pos,&quality)==XUI_OK && pos.iKind==XUI_DOC_POSITION_TEXT &&
                    xuiDocumentSnapshotGetNode(current,pos.iNodeId,&node)==XUI_OK && node.tAttributes.iMarks==XUI_DOC_LINK && !strcmp(node.sResource,"/second"));}
            TABLE_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,c->before);table_equivalent(restored,saved);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,c->after);table_equivalent(restored,current);xuiDocumentSnapshotRelease(restored);table_source(saved,c->before);
            xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(d);TABLE_CHECK(!f.allocation.live);
            if(result==XUI_OK)break;
        }
        TABLE_CHECK(point && point<10000);
        printf("Nested definition removal fixture=%u mode=%u failure points=%ld exact table/index/global resolution/atomic/retry/history/zero leaks passed\n",fixture,mode,point);fflush(stdout);
    }
}
int main(void)
{
    unsigned fixture,mode;long totals[3]={0};
    for(fixture=0;fixture<sizeof(local_reference_cases)/sizeof(*local_reference_cases);fixture++) {
        const struct local_reference_case* c=&local_reference_cases[fixture];long point;
        for(point=0;point<10000;point++) {
            local_fault f={{-1,0},-1,NULL};xui_document_snapshot saved;xui_document d=local_saved(&f,&saved);
            uint64_t revision=xuiDocumentGetRevision(d);int result;f.allocation.remaining=point;
            result=xuiDocumentLoadMarkdown(d,c->source,strlen(c->source));f.allocation.remaining=-1;
            TABLE_CHECK(result==XUI_OK || result==XUI_ERROR_OUT_OF_MEMORY);
            if(result){local_unchanged(d,saved,revision);TABLE_CHECK(xuiDocumentLoadMarkdown(d,c->source,strlen(c->source))==XUI_OK);}
            local_loaded(d,c);table_source(saved,"saved **old**\n");xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(d);TABLE_CHECK(!f.allocation.live);
            if(!result)break;
        }
        TABLE_CHECK(point<10000 && point>0);totals[0]+=point;
        printf("Local reference load fixture=%u OOM points=%ld atomic/retry/old snapshot/zero leaks passed\n",fixture,point);fflush(stdout);
    }
    for(mode=0;mode<2;mode++) for(fixture=0;fixture<sizeof(local_reference_cases)/sizeof(*local_reference_cases);fixture++) {
        const struct local_reference_case* c=&local_reference_cases[fixture];long point;
        for(point=0;point<10000;point++) {
            local_fault f={{-1,0},-1,NULL};xui_document_snapshot saved,current,restored;xui_document d=local_saved(&f,&saved);
            uint64_t revision=xuiDocumentGetRevision(d);xui_document_prepare pending;xui_doc_source_patch_t patch={0};int result;
            patch.iSize=sizeof(patch);patch.iStart=0;patch.iEnd=14;patch.sText=c->source;patch.iTextBytes=strlen(c->source);
            TABLE_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&pending)==XUI_OK);
            if(mode){f.document=d;f.cancel_at=point;}else f.allocation.remaining=point;
            result=xuiDocumentPrepareRun(pending);f.document=NULL;f.cancel_at=f.allocation.remaining=-1;
            TABLE_CHECK(result==XUI_OK || result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            local_unchanged(d,saved,revision);
            if(result){
                xui_doc_prepare_info_t info={0}; xui_document_prepare retry=NULL;
                info.iSize=sizeof(info);
                TABLE_CHECK(xuiDocumentPrepareGetInfo(pending,&info)==XUI_OK && info.iResult==result &&
                    info.iState==XUI_DOC_PREPARE_FAILED);
                /* Run executes a queued handle once. ContinueSource retries its immutable input. */
                TABLE_CHECK(xuiDocumentPrepareRun(pending)==XUI_ERROR_INVALID_STATE);
                if(mode){
                    TABLE_CHECK(xuiDocumentPrepareContinueSource(d,pending,NULL,0,&retry)==XUI_DOC_ERROR_STALE && !retry);
                    TABLE_CHECK(xuiDocumentCancelPrepare(d)==XUI_OK);xuiDocumentPrepareRelease(pending);
                    TABLE_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&pending)==XUI_OK && xuiDocumentPrepareRun(pending)==XUI_OK);
                } else {
                    TABLE_CHECK(xuiDocumentPrepareContinueSource(d,pending,NULL,0,&retry)==XUI_OK);
                    xuiDocumentPrepareRelease(pending);pending=retry;
                    TABLE_CHECK(xuiDocumentPrepareRun(pending)==XUI_OK);
                }
                local_unchanged(d,saved,revision);
            }
            TABLE_CHECK(xuiDocumentPreparePublish(d,pending,NULL)==XUI_OK);xuiDocumentPrepareRelease(pending);local_loaded(d,c);
            TABLE_CHECK(xuiDocumentGetRevision(d)==revision+1 && xuiDocumentAcquireSnapshot(d,&current)==XUI_OK &&
                xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
            table_source(restored,"saved **old**\n");table_equivalent(restored,saved);xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);local_loaded(d,c);table_source(current,c->source);table_source(saved,"saved **old**\n");
            xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(d);TABLE_CHECK(!f.allocation.live);
            if(!result)break;
        }
        TABLE_CHECK(point<10000 && point>0);totals[mode+1]+=point;
        printf("Local reference Prepare fixture=%u mode=%u failure points=%ld atomic/one-shot Run/failed-source continuation/cancelled rejection/one Undo/zero leaks passed\n",fixture,mode,point);fflush(stdout);
    }
    local_body_edit_faults();
    printf("Footnote local reference faults: load OOM=%ld Prepare OOM=%ld cancellation=%ld passed\n",totals[0],totals[1],totals[2]);
    return 0;
}
