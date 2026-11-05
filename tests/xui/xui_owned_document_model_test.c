/* Default-provenance model and exact Native graph, CPU-only and window-free. */
#include "xui_owned_model_host_test.h"
#include "xui_document.h"
#include "src/xui/document/xui_document_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct RawAllocatorProbe {unsigned allocated,freed;} RawAllocatorProbe;
static void* rawAlloc(void* user,size_t bytes){++((RawAllocatorProbe*)user)->allocated;return malloc(bytes);}
static void rawFree(void* user,void* pointer){++((RawAllocatorProbe*)user)->freed;free(pointer);}
static void defaultOriginOnly(void)
{
    RawAllocatorProbe probe={0};xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_RICH,.onAlloc=rawAlloc,.onFree=rawFree,.pAllocatorUser=&probe};
    xui_document document=(xui_document)(uintptr_t)1;
    assert(xuiDocumentCreateOwned(&desc,&document)==XUI_ERROR_INVALID_ARGUMENT && !document && !probe.allocated && !probe.freed);
    desc.onAlloc=NULL;desc.onFree=NULL;document=(xui_document)(uintptr_t)1;
    assert(xuiDocumentCreateOwned(&desc,&document)==XUI_ERROR_INVALID_ARGUMENT && !document && !probe.allocated);
    desc.onAlloc=rawAlloc;desc.onFree=rawFree;assert(xuiDocumentCreate(&desc,&document)==XUI_OK && probe.allocated && !xuiDocumentOwnership(document).Data);
    xuiDocumentRelease(document);assert(probe.allocated==probe.freed);
}

static size_t modelCountOf(xrtownershipref ref)
{
    assert(ref.Data && ref.Ops);size_t count=0;xrtownershipscope frozen={0};
    assert(xrtOwnershipFreezeTryBegin(&frozen));assert(ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&frozen));return count;
}
static bool modelVisit(xrtownershipref ref,void* raw)
{assert(ref.Data && ref.Ops && xuiDocumentModelOwnershipAdapterV1(ref));++*(unsigned*)raw;return true;}
/* Explicit macro arguments keep the native ABI/type checks visible. */
#define CHECK_MODEL_SHARE(Name,Handle,Value,Ownership,Release) do { \
    Handle shared=NULL;size_t before=modelCountOf(Ownership(Value)); \
    assert(Name(Value,&shared)==XUI_OK && shared==Value && modelCountOf(Ownership(Value))==before+1); \
    assert(Release(shared)==XUI_OK && modelCountOf(Ownership(Value))==before); \
} while(0)
static void richModel(xui_document* document,xui_document_snapshot* snapshot,xui_doc_node_id* text,xui_document_change_set* change)
{
    assert(xuiDocumentCreateOwned(NULL,document)==XUI_OK);xui_document_transaction transaction=NULL;
    assert(xuiDocumentBeginTransaction(*document,NULL,&transaction)==XUI_OK && modelCountOf(xuiDocumentOwnership(*document))==2);
    assert(modelCountOf(xuiDocumentTxnOwnership(transaction))==1);
    xui_doc_node_id paragraph=0;xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
    assert(xuiDocumentTxnInsertNode(transaction,XUI_DOCUMENT_ROOT,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    node=(xui_doc_node_desc_t){.iSize=sizeof(node),.iKind=XUI_DOC_TEXT,.sText="abc",.iTextBytes=3};
    assert(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,text)==XUI_OK);
    assert(xuiDocumentTxnCommit(transaction,change)==XUI_OK && xuiDocumentTxnReleaseChecked(transaction)==XUI_OK);
    assert(xuiDocumentAcquireSnapshot(*document,snapshot)==XUI_OK);
}
static void allModelKinds(void)
{
    xui_document raw=NULL;assert(xuiDocumentCreate(NULL,&raw)==XUI_OK && !xuiDocumentIsOwned(raw) && !xuiDocumentOwnership(raw).Data);
    bool consumed=true;assert(xuiDocumentReleaseOwnerChecked(raw,&consumed)==XUI_ERROR_INVALID_ARGUMENT && !consumed);xuiDocumentRelease(raw);
    xui_document document=NULL;xui_document_snapshot snapshot=NULL;xui_doc_node_id text=0;xui_document_change_set change=NULL;
    richModel(&document,&snapshot,&text,&change);
    assert(xuiDocumentIsOwned(document) && modelCountOf(xuiDocumentOwnership(document))==1 && modelCountOf(xuiDocumentSnapshotOwnership(snapshot))==1 && modelCountOf(xuiDocumentChangeSetOwnership(change))==1);
    xuiDocumentRetain(document);assert(modelCountOf(xuiDocumentOwnership(document))==2);xuiDocumentRelease(document);assert(modelCountOf(xuiDocumentOwnership(document))==1);
    xui_doc_position_t start={.iSize=sizeof(start),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=xuiDocumentGetIdentity(document),.iRevision=xuiDocumentGetRevision(document),.iNodeId=text,.iOffset=0,.iAffinity=XUI_DOC_AFTER};
    xui_doc_range_t range={.tAnchor=start,.tCaret=start};range.tCaret.iOffset=3;
    xui_document_fragment fragment=NULL;assert(xuiDocumentFragmentCreateRange(snapshot,&range,&fragment)==XUI_OK && modelCountOf(xuiDocumentFragmentOwnership(fragment))==1);
    CHECK_MODEL_SHARE(xuiDocumentShareOwned,xui_document,document,xuiDocumentOwnership,xuiDocumentReleaseChecked);
    CHECK_MODEL_SHARE(xuiDocumentSnapshotShareOwned,xui_document_snapshot,snapshot,xuiDocumentSnapshotOwnership,xuiDocumentSnapshotReleaseChecked);
    CHECK_MODEL_SHARE(xuiDocumentChangeSetShareOwned,xui_document_change_set,change,xuiDocumentChangeSetOwnership,xuiDocumentChangeSetReleaseChecked);
    CHECK_MODEL_SHARE(xuiDocumentFragmentShareOwned,xui_document_fragment,fragment,xuiDocumentFragmentOwnership,xuiDocumentFragmentReleaseChecked);
    xui_document wrong=(xui_document)(uintptr_t)1;
    assert(xuiDocumentShareOwned((xui_document)snapshot,&wrong)==XUI_ERROR_INVALID_ARGUMENT && !wrong);
    unsigned edges=0;xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));xrtownershipref ref=xuiDocumentFragmentOwnership(fragment);
    assert(ref.Ops->Trace(ref.Data,modelVisit,&edges) && edges==1 && xrtOwnershipScopeEnd(&frozen));
    assert(xuiDocumentFragmentReleaseChecked(fragment)==XUI_OK && xuiDocumentChangeSetReleaseChecked(change)==XUI_OK && xuiDocumentSnapshotReleaseChecked(snapshot)==XUI_OK && xuiDocumentReleaseChecked(document)==XUI_OK);
    xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN};
    assert(xuiDocumentCreateOwned(&desc,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,"abc\n",4)==XUI_OK && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    xui_document_source_index index=NULL;assert(xuiDocumentSnapshotCreateSourceIndex(snapshot,NULL,&index)==XUI_OK && modelCountOf(xuiDocumentSourceIndexOwnership(index))==1 && modelCountOf(xuiDocumentSnapshotOwnership(snapshot))==2);
    CHECK_MODEL_SHARE(xuiDocumentSourceIndexShareOwned,xui_document_source_index,index,xuiDocumentSourceIndexOwnership,xuiDocumentSourceIndexReleaseChecked);
    assert(xuiDocumentSourceIndexReleaseChecked(index)==XUI_OK && modelCountOf(xuiDocumentSnapshotOwnership(snapshot))==1);
    xui_doc_source_patch_t patch={.iSize=sizeof(patch),.iStart=0,.iEnd=1,.sText="Z",.iTextBytes=1};xui_document_prepare prepare=NULL;
    assert(xuiDocumentPrepareSource(document,NULL,&patch,1,&prepare)==XUI_OK && modelCountOf(xuiDocumentPrepareOwnership(prepare))==2);
    CHECK_MODEL_SHARE(xuiDocumentPrepareShareOwned,xui_document_prepare,prepare,xuiDocumentPrepareOwnership,xuiDocumentPrepareReleaseChecked);
    edges=0;assert(xrtOwnershipFreezeTryBegin(&frozen));ref=xuiDocumentOwnership(document);assert(ref.Ops->Trace(ref.Data,modelVisit,&edges) && edges==1 && xrtOwnershipScopeEnd(&frozen));
    assert(xuiDocumentPrepareRun(prepare)==XUI_OK && xuiDocumentPreparePublish(document,prepare,NULL)==XUI_OK && modelCountOf(xuiDocumentPrepareOwnership(prepare))==1);
    assert(xuiDocumentPrepareReleaseChecked(prepare)==XUI_OK && xuiDocumentSnapshotReleaseChecked(snapshot)==XUI_OK && xuiDocumentReleaseChecked(document)==XUI_OK);
}
static void claimedShare(void)
{
    xui_document document=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);
    xrtownershipref ref=xuiDocumentOwnership(document);const xrtownershipadapterv1* adapter=xuiDocumentModelOwnershipAdapterV1(ref);
    assert(adapter && adapter->Hold(ref.Data));int token=0;xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze) && adapter->Claim(ref.Data,&token) && xrtOwnershipScopeEnd(&freeze));
    xui_document shared=(xui_document)(uintptr_t)1;xui_document_snapshot snapshot=(xui_document_snapshot)(uintptr_t)1;
    assert(xuiDocumentShareOwned(document,&shared)==XUI_ERROR_INVALID_STATE && !shared);
    assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_ERROR_INVALID_STATE && !snapshot);
    assert(xuiDocumentGetRevision(document)==1);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Restore(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
    const xrtownershipfinishv2* completion=xuiDocumentModelOwnershipCompletionV2(ref);
    assert(completion && completion->DropChecked(ref.Data) && xuiDocumentReleaseChecked(document)==XUI_OK);
    assert(xuiDocumentCreate(NULL,&document)==XUI_OK);shared=(xui_document)(uintptr_t)1;
    assert(xuiDocumentShareOwned(document,&shared)==XUI_ERROR_INVALID_ARGUMENT && !shared);xuiDocumentRelease(document);
}
static void uniqueTxnPinsDoNotKeepWriter(bool claimed)
{
    xui_document document=NULL;xui_document_transaction transaction=NULL;
    assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK && xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
    xrtownershipref ref=xuiDocumentTxnOwnership(transaction);const xrtownershipadapterv1* adapter=xuiDocumentModelOwnershipAdapterV1(ref);
    assert(adapter && adapter->Hold(ref.Data) && modelCountOf(ref)==2);int token=0;xrtownershipscope freeze={0};
    if(claimed){assert(xrtOwnershipFreezeTryBegin(&freeze));assert(adapter->Claim(ref.Data,&token) && xrtOwnershipScopeEnd(&freeze));}
    bool consumed=false;assert(xuiDocumentTxnReleaseOwnerChecked(transaction,&consumed)==XUI_OK && consumed);
    assert(!document->writer && modelCountOf(ref)==1 && !xuiDocumentModelOwnershipReadReady(ref));
    assert(xrtOwnershipFreezeTryBegin(&freeze));unsigned edges=0;assert(ref.Ops->Trace(ref.Data,modelVisit,&edges) && !edges);assert(xrtOwnershipScopeEnd(&freeze));
    if(claimed){
        const xrtownershipfinalizationv1* finalization=xuiDocumentModelOwnershipFinalizationV1(ref);bool complete=false;
        assert(finalization->Step(ref.Data,&token,&complete) && complete);
        assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
        const xrtownershipfinishv2* finish=xuiDocumentModelOwnershipCompletionV2(ref);
        assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY && finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY);
    }
    xui_document_transaction replacement=NULL;assert(xuiDocumentBeginTransaction(document,NULL,&replacement)==XUI_OK && xuiDocumentTxnReleaseChecked(replacement)==XUI_OK);
    assert(xuiDocumentModelOwnershipCompletionV2(ref)->DropChecked(ref.Data));assert(xuiDocumentReleaseChecked(document)==XUI_OK);
}
static void activeTxnWaits(void)
{
    xui_document document=NULL;xui_document_transaction transaction=NULL;
    assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK && xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
    doc_model_call call={0};assert(doc_model_call_begin(&transaction->owned,&call)==XUI_OK);
    bool consumed=true;assert(xuiDocumentTxnReleaseOwnerChecked(transaction,&consumed)==XUI_STATUS_DEFERRED && !consumed && document->writer==transaction);
    assert(doc_model_call_end(&call,XUI_OK)==XUI_OK && modelCountOf(xuiDocumentTxnOwnership(transaction))==1);
    assert(xuiDocumentTxnReleaseChecked(transaction)==XUI_OK && !document->writer && xuiDocumentReleaseChecked(document)==XUI_OK);
}
typedef struct WrongModelThread {xui_document document;xui_document_transaction transaction;int status;bool consumed;} WrongModelThread;
static int32 wrongModelRelease(void* raw)
{
    WrongModelThread* test=raw;test->consumed=true;
    test->status=test->transaction ? xuiDocumentTxnReleaseOwnerChecked(test->transaction,&test->consumed):xuiDocumentReleaseOwnerChecked(test->document,&test->consumed);
    assert(!xrtGetError());return 0;
}
static void wrongThreadCleanupIsFailure(void)
{
    WrongModelThread test={0};assert(xuiDocumentCreateOwned(NULL,&test.document)==XUI_OK);
    xthread* thread=xrtThreadCreate(wrongModelRelease,&test,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(test.status==XUI_ERROR_BUSY && !test.consumed && modelCountOf(xuiDocumentOwnership(test.document))==1);
    assert(xuiDocumentBeginTransaction(test.document,NULL,&test.transaction)==XUI_OK);xrtownershipref ref=xuiDocumentTxnOwnership(test.transaction);
    assert(xuiDocumentModelOwnershipAdapterV1(ref)->Hold(ref.Data));thread=xrtThreadCreate(wrongModelRelease,&test,0);
    assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(test.status==XUI_ERROR_BUSY && !test.consumed && test.document->writer==test.transaction && modelCountOf(ref)==2);
    assert(xuiDocumentTxnReleaseChecked(test.transaction)==XUI_OK && xuiDocumentModelOwnershipCompletionV2(ref)->DropChecked(ref.Data));
    assert(xuiDocumentReleaseChecked(test.document)==XUI_OK);
}
typedef struct ModelObserver {
    size_t refs;unsigned holds,drops,calls;
    xui_document document;uint64_t token;
    struct ModelObserver* peer;
    const xerror* diagnostic; /* Borrowed from this CPU fixture's C stack. */
    int outcome;
    bool failHold,freshHold,dropFailure,dropError,error,remove,clearBatch;
} ModelObserver;
static xui_callback_owner_t observerRef(ModelObserver*);
static int observerNotify(xui_document,xui_document_change_set,void*);
static bool observerHold(const void* raw)
{
    ModelObserver* observer=(ModelObserver*)raw;++observer->holds;if(observer->failHold)return false;++observer->refs;
    if(observer->peer){ModelObserver* peer=observer->peer;observer->peer=NULL;xui_callback_owner_t ref=observerRef(peer);assert(xuiDocumentSubscribeOwned(peer->document,observerNotify,peer,&ref,&peer->token)==XUI_OK);}
    if(observer->freshHold)xrtSetErrorKind(XERR_RANGE);
    return true;
}
static bool observerDrop(const void* raw)
{ModelObserver* observer=(ModelObserver*)raw;assert(observer->refs);--observer->refs;++observer->drops;if(observer->dropError)xrtSetErrorKind(XERR_STATE);return !observer->dropFailure;}
static bool observerCount(const void* raw,size_t* count){*count=((const ModelObserver*)raw)->refs;return *count!=0;}
static bool observerTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops observerOps={observerCount,observerTrace};
static xui_callback_owner_t observerRef(ModelObserver* observer){return (xui_callback_owner_t){{observer,&observerOps},observerHold,observerDrop};}
static int observerNotify(xui_document document,xui_document_change_set change,void* raw)
{
    ModelObserver* observer=raw;assert(document==observer->document && observer->refs>=3 && xuiDocumentChangeSetOwnership(change).Data);++observer->calls;
    if(observer->clearBatch){observer->clearBatch=false;assert(xuiDocumentCheckNotifications(document,true)==XUI_ERROR_CALLBACK_FAILED);xrtClearError();}
    if(observer->remove)assert(xuiDocumentUnsubscribeChecked(document,observer->token)==XUI_OK);
    if(observer->diagnostic)xrtSetErrorTake(xrtErrorRef(observer->diagnostic));
    if(observer->error)xrtSetErrorKind(XERR_RANGE);
    return observer->outcome;
}
static void observerCommit(xui_document document)
{
    xui_document_transaction transaction=NULL;assert(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
    xui_doc_node_id inserted=0;assert(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&inserted)==XUI_OK);
    assert(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK && xuiDocumentTxnReleaseChecked(transaction)==XUI_OK);
}
static void rawObserver(xui_document document,xui_document_change_set change,void* user)
{(void)document;(void)change;(void)user;}
static void callbackPublication(void)
{
    xui_document document=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);uint64_t token=99;
    assert(xuiDocumentSubscribe(document,rawObserver,NULL,&token)==XUI_ERROR_INVALID_STATE && !token);
    ModelObserver candidate={.refs=1,.document=document,.freshHold=true},peer={.refs=1,.document=document};candidate.peer=&peer;
    xui_callback_owner_t ref=observerRef(&candidate);
    assert(xuiDocumentSubscribeOwned(document,observerNotify,&candidate,&ref,&token)==XUI_ERROR_CALLBACK_FAILED && !token && candidate.refs==1 && candidate.drops==1 && peer.refs==2 && xrtErrorKind(xrtGetError())==XERR_RANGE);xrtClearError();
    candidate.freshHold=false;candidate.failHold=true;unsigned drops=candidate.drops;
    assert(xuiDocumentSubscribeOwned(document,observerNotify,&candidate,&ref,&token)==XUI_ERROR_CALLBACK_FAILED && !token && candidate.drops==drops && !xrtGetError());
    observerCommit(document);assert(peer.calls==1 && !candidate.calls && !xuiDocumentGetNotificationFailureCount(document));
    assert(xuiDocumentReleaseChecked(document)==XUI_OK && candidate.refs==1 && peer.refs==1);
}
static void notificationLatch(int outcome,bool error,bool observe,bool secondary)
{
    xui_document document=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);ModelObserver observer={.refs=1,.document=document,.outcome=outcome,.error=error};
    xui_callback_owner_t ref=observerRef(&observer);assert(xuiDocumentSubscribeOwned(document,observerNotify,&observer,&ref,&observer.token)==XUI_OK);
    observerCommit(document);assert(observer.calls==1 && xuiDocumentGetRevision(document)==2 && xuiDocumentGetNotificationFailureCount(document)==1 && xuiDocumentGetNotificationFailureRevision(document)==2);
    assert(xuiDocumentGetNotificationFailureStatus(document)==outcome);
    assert((error && xrtErrorKind(xrtGetError())==XERR_RANGE) || (!error && !xrtGetError()));xrtClearError();
    if(observe){
        assert(xuiDocumentCheckNotifications(document,false)==XUI_ERROR_CALLBACK_FAILED && xuiDocumentGetNotificationFailureCount(document)==1);xrtClearError();
        assert(xuiDocumentCheckNotifications(document,true)==XUI_ERROR_CALLBACK_FAILED && !xuiDocumentGetNotificationFailureCount(document));
        assert((error && xrtErrorKind(xrtGetError())==XERR_RANGE) || (!error && !xrtGetError()));xrtClearError();
    }
    observer.dropFailure=observer.dropError=secondary;
    int status=xuiDocumentReleaseChecked(document);
    assert(status==(observe && !secondary ? XUI_OK:XUI_ERROR_CALLBACK_FAILED) && observer.refs==1);
    if(!observe)assert((error && xrtErrorKind(xrtGetError())==XERR_RANGE) || (!error && !xrtGetError()));
    xrtClearError();
}
static void notificationReentry(void)
{
    xui_document document=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);ModelObserver observer={.refs=1,.document=document,.outcome=XUI_ERROR_UNSUPPORTED};
    xui_callback_owner_t ref=observerRef(&observer);assert(xuiDocumentSubscribeOwned(document,observerNotify,&observer,&ref,&observer.token)==XUI_OK);
    observerCommit(document);assert(xuiDocumentGetNotificationFailureCount(document)==1);observer.clearBatch=true;
    observerCommit(document);assert(observer.calls==2 && xuiDocumentGetNotificationFailureCount(document)==1 && xuiDocumentGetNotificationFailureRevision(document)==3);
    assert(xuiDocumentCheckNotifications(document,true)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    observer.outcome=XUI_OK;observer.remove=true;observerCommit(document);
    assert(observer.calls==3 && observer.refs==1 && !xuiDocumentGetNotificationFailureCount(document));
    observerCommit(document);assert(observer.calls==3 && xuiDocumentReleaseChecked(document)==XUI_OK);
}
static void uniqueTxnFinishFailureConsumesOnce(void)
{
    xui_document document=NULL;xui_document_transaction transaction=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);
    ModelObserver observer={.refs=1,.document=document,.outcome=XUI_ERROR_UNSUPPORTED};xui_callback_owner_t owner=observerRef(&observer);
    assert(xuiDocumentSubscribeOwned(document,observerNotify,&observer,&owner,&observer.token)==XUI_OK);
    assert(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);xui_doc_node_id inserted=0;
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};assert(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&inserted)==XUI_OK);
    assert(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK && !xrtGetError() && xuiDocumentReleaseChecked(document)==XUI_OK);
    xrtownershipref ref=xuiDocumentTxnOwnership(transaction);const xrtownershipadapterv1* adapter=xuiDocumentModelOwnershipAdapterV1(ref);assert(adapter->Hold(ref.Data));
    bool consumed=false;assert(xuiDocumentTxnReleaseOwnerChecked(transaction,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed && !xrtGetError() && observer.refs==1);
    assert(modelCountOf(ref)==1 && !xuiDocumentModelOwnershipReadReady(ref));
    assert(xuiDocumentModelOwnershipCompletionV2(ref)->DropChecked(ref.Data) && observer.refs==1);
}
typedef struct ModelErrorEdges {xrtownershipref target;unsigned errors,observers;} ModelErrorEdges;
static bool modelErrorVisit(xrtownershipref ref,void* raw)
{
    ModelErrorEdges* edges=raw;
    if(ref.Data==edges->target.Data && ref.Ops==edges->target.Ops){assert(xrtErrorOwnershipAdapterV1(ref));++edges->errors;}
    else{assert(ref.Ops==&observerOps);++edges->observers;}
    return true;
}
static void notificationErrorGraph(void)
{
    xerror* cause=xrtErrorCreate(XERR_STATE,"model-test",41,"cause");assert(cause);
    xerror* error=xrtErrorWrap(cause,XERR_RANGE,"model-test",42,"first dynamic notification failure");assert(error);
    xrtownershipref errorRef=xrtErrorOwnership(error),causeRef=xrtErrorOwnership(cause);assert(errorRef.Data && causeRef.Data);
    xui_document document=NULL;assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);
    ModelObserver observer={.refs=1,.document=document,.outcome=XUI_ERROR_UNSUPPORTED,.diagnostic=error};
    xui_callback_owner_t owner=observerRef(&observer);assert(xuiDocumentSubscribeOwned(document,observerNotify,&observer,&owner,&observer.token)==XUI_OK);
    observerCommit(document);xrtClearError();assert(modelCountOf(errorRef)==2 && modelCountOf(causeRef)==2);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));ModelErrorEdges edges={.target=errorRef};xrtownershipref doc=xuiDocumentOwnership(document);
    assert(doc.Ops->Trace(doc.Data,modelErrorVisit,&edges) && edges.errors==1 && edges.observers==1);
    edges=(ModelErrorEdges){.target=causeRef};assert(errorRef.Ops->Trace(errorRef.Data,modelErrorVisit,&edges) && edges.errors==1 && !edges.observers);
    assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiDocumentCheckNotifications(document,true)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==error);xrtClearError();assert(modelCountOf(errorRef)==1);
    assert(xuiDocumentReleaseChecked(document)==XUI_OK && observer.refs==1);xrtErrorFree(error);xrtErrorFree(cause);
}
int main(void)
{
    modelHostBind();
    defaultOriginOnly();allModelKinds();claimedShare();uniqueTxnPinsDoNotKeepWriter(false);uniqueTxnPinsDoNotKeepWriter(true);activeTxnWaits();wrongThreadCleanupIsFailure();callbackPublication();notificationLatch(XUI_ERROR_UNSUPPORTED,false,true,false);notificationLatch(XUI_ERROR_UNSUPPORTED,false,false,true);notificationLatch(XUI_ERROR_UNSUPPORTED,true,false,true);notificationLatch(7,false,true,false);notificationLatch(1,false,false,false);notificationReentry();notificationErrorGraph();uniqueTxnFinishFailureConsumesOnce();
    puts("default Owned Document: seven real lifetimes, graph, physical Owned notification status and exact post-commit latch passed");return 0;
}
