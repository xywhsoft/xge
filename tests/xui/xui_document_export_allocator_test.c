/* Public export allocation contract, real writer ownership and OOM rollback. */
#include "xui_document.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
_Static_assert(XUI_DOCUMENT_EXPORT_XRT_ALLOCATOR==1,"Document export allocator contract");
static char* writer_result;
char* __real_xrtJsonWriterTake(xjsonwriter*,size_t*);
char* __wrap_xrtJsonWriterTake(xjsonwriter* writer,size_t* size)
{ writer_result=__real_xrtJsonWriterTake(writer,size);return writer_result; }
typedef struct allocation_search {const void* data;size_t bytes;bool found;} allocation_search;
static bool find_allocation(const xmemdebugallocation* value,void* context)
{
    allocation_search* wanted=context;
    if(value->Address==wanted->data) {assert(value->Size>=wanted->bytes);wanted->found=true;}
    return true;
}
static void check_storage(char* data,uint64_t size)
{
    assert(data && size<SIZE_MAX && data[size]==0);
    allocation_search wanted={data,(size_t)size+1,false};
    xrtMemDebugVisitLive(find_allocation,&wanted);assert(wanted.found);
}
static void adopt(char* data,uint64_t size)
{
    check_storage(data,size);
    bytes owned=(bytes)data;
    xvalue* value=xrtValueBytesTake(&owned,(size_t)size);assert(value && !owned);
    xbytesview view={0};assert(xrtValueGetBytes(value,&view));
    assert(view.Data==(bytes)data && view.Size==size);
    xrtValueRelease(value);
}
static void balanced(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot now;xrtMemDebugSnapshot(&now);
    assert(now.LiveCount==before->LiveCount && now.LiveBytes==before->LiveBytes);
    assert(now.DoubleFreeCount==before->DoubleFreeCount && now.InvalidFreeCount==before->InvalidFreeCount);
}
static uint64_t node(xui_document_transaction txn,uint64_t parent,unsigned kind,const char* text)
{
    xui_doc_node_desc_t desc={0};uint64_t id=0;
    desc.iSize=sizeof(desc);desc.iKind=kind;desc.sText=text;desc.iTextBytes=text?strlen(text):0;
    assert(xuiDocumentTxnInsertNode(txn,parent,UINT64_MAX,&desc,&id)==XUI_OK);return id;
}
static xui_document_snapshot snapshot;
static xui_document_fragment fragment;
static xui_doc_range_t selected;
static uint64_t table;
static int produce(unsigned kind,char** data,uint64_t* size)
{
    switch(kind) {
    case 0:return xuiDocumentSerialize(snapshot,data,size);
    case 1:return xuiDocumentExportHtml(snapshot,data,size);
    case 2:return xuiDocumentSnapshotCopyPlainText(snapshot,data,size);
    case 3:return xuiDocumentSnapshotCopyRange(snapshot,&selected,data,size);
    case 4:return xuiDocumentSnapshotCopyTableMatrix(snapshot,table,0,0,1,2,data,size);
    case 5:return xuiDocumentFragmentSerialize(fragment,data,size);
    default:return xuiDocumentFragmentExportHtml(fragment,data,size);
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));
    xui_document document=NULL;xui_document_transaction txn=NULL;
    assert(xuiDocumentCreate(NULL,&document)==XUI_OK);
    assert(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    uint64_t paragraph=node(txn,XUI_DOCUMENT_ROOT,XUI_DOC_PARAGRAPH,NULL);
    node(txn,paragraph,XUI_DOC_TEXT,"Unicode \xe4\xb8\xad & < >");
    table=node(txn,XUI_DOCUMENT_ROOT,XUI_DOC_TABLE,NULL);
    uint64_t row=node(txn,table,XUI_DOC_ROW,NULL);
    for(unsigned i=0;i<2;++i) {
        uint64_t cell=node(txn,row,XUI_DOC_CELL,NULL);
        uint64_t p=node(txn,cell,XUI_DOC_PARAGRAPH,NULL);
        node(txn,p,XUI_DOC_TEXT,i?"right":"left");
    }
    assert(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    selected.tAnchor.iSize=selected.tCaret.iSize=sizeof(selected.tAnchor);
    selected.tAnchor.iDocumentId=selected.tCaret.iDocumentId=xuiDocumentGetIdentity(document);
    selected.tAnchor.iRevision=selected.tCaret.iRevision=xuiDocumentGetRevision(document);
    selected.tAnchor.iKind=selected.tCaret.iKind=XUI_DOC_POSITION_GAP;
    selected.tAnchor.iNodeId=selected.tCaret.iNodeId=XUI_DOCUMENT_ROOT;
    selected.tCaret.iOffset=1;
    assert(xuiDocumentFragmentCreateRange(snapshot,&selected,&fragment)==XUI_OK);
    char* data=NULL;uint64_t size=0;
    assert(produce(0,&data,&size)==XUI_OK && data==writer_result);
    adopt(data,size);
    for(unsigned kind=0;kind<7;++kind) {assert(produce(kind,&data,&size)==XUI_OK);adopt(data,size);}
    for(unsigned kind=0;kind<7;++kind) {
        unsigned failed=0,succeeded=0;
        for(uint64_t budget=0;budget<64;++budget) {
            xrtClearError();xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
            data=(char*)1;size=UINT64_MAX;assert(xrtMemDebugFailAfter(budget));
            int result=produce(kind,&data,&size);
            xrtMemDebugFailClear();
            if(result==XUI_OK) {check_storage(data,size);xuiDocumentFreeBuffer(data);++succeeded;}
            else {assert(data==NULL && size==0);++failed;}
            xrtClearError();balanced(&before);
        }
        assert(failed && succeeded);
        printf("Document export %u: 64 allocation budgets, %u failures, %u successes\n",kind,failed,succeeded);
    }
    xuiDocumentFragmentRelease(fragment);xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    xrtClearError();xmemdebugsnapshot done;xrtMemDebugSnapshot(&done);
    assert(!done.LiveCount && !done.LiveBytes && !done.InvalidFreeCount && !done.DoubleFreeCount);
    puts("Document exports: resident allocator, writer pointer identity, zero-copy bytes and OOM rollback passed");
    return 0;
}
