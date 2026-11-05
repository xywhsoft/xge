/* Exact UI model owning slots and the explicit raw interoperability boundary. */
#define XUI_OWNED_DOC_UI_HELPERS
#include "xui_owned_doc_ui_callbacks_test.c"
#define main unusedStandaloneModelMain
#include "xui_owned_document_model_test.c"
#undef main

static void rawSnapshotIsNotLeaf(void)
{
    DocUiTest test;docUiSetup(&test,0);xui_document raw=NULL;xui_document_snapshot snapshot=NULL;
    assert(xuiDocumentCreate(NULL,&raw)==XUI_OK && xuiDocumentAcquireSnapshot(raw,&snapshot)==XUI_OK);
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={test.font,test.font,test.font,test.font,test.font}};
    xui_document_renderer renderer=NULL;assert(xuiDocumentRendererCreate(test.base.context,&desc,&renderer)==XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));unsigned edges=0;
    assert(!xuiDocumentRendererTraceOwnership(renderer,layoutVisit,&edges) && xrtOwnershipScopeEnd(&freeze));
    assert(xuiDocumentRendererReleaseChecked(renderer)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(raw);docUiFinish(&test);
}
static void claimedBuiltinBacklinkCancels(int kind)
{
    DocUiTest test;docUiSetup(&test,kind);xrtownershipref ref=xuiDocumentOwnership(test.document);
    const xrtownershipadapterv1* adapter=xuiDocumentModelOwnershipAdapterV1(ref);assert(adapter && adapter->Hold(ref.Data));
    int token=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(adapter->Claim(ref.Data,&token) && xrtOwnershipScopeEnd(&freeze));
    assert(xuiDestroyChecked(test.base.context)==XUI_OK && !test.document->observers);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Restore(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiDocumentModelOwnershipCompletionV2(ref)->DropChecked(ref.Data));
    /* Restoring the still-external Document must never call freed View or
     * Message Binding userdata through an uncancelled Native backlink. */
    observerCommit(test.document);assert(xuiDocumentReleaseChecked(test.document)==XUI_OK);
    assert(xuiFontReleaseChecked(test.font)==XUI_OK && xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
static void contextLastModelReportsNull(int kind,bool secondary)
{
    DocUiTest test;docUiSetup(&test,kind);
    ModelObserver observer={.refs=1,.document=test.document,.outcome=1};
    xui_callback_owner_t owner=observerRef(&observer);
    assert(xuiDocumentSubscribeOwned(test.document,observerNotify,&observer,&owner,&observer.token)==XUI_OK);
    observerCommit(test.document);assert(xuiDocumentGetNotificationFailureStatus(test.document)==1 && !xrtGetError());
    assert(xuiDocumentReleaseChecked(test.document)==XUI_OK); /* UI owns the last Document. */
    observer.dropFailure=observer.dropError=secondary;
    bool consumed=false;assert(xuiDestroyOwnerChecked(test.base.context,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed && observer.refs==1 && !xrtGetError());
    assert(xuiFontReleaseChecked(test.font)==XUI_OK && xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
static void closeContextOnChange(xui_document document,xui_document_change_set change,void* raw)
{
    (void)document;(void)change;doc_view_data* view=raw;
    xuiDestroy(view->widget->pContext);
}
static void modelContextGone(void* raw){++*(unsigned*)raw;}
static void internalObserverTailFailureIsLatched(void)
{
    DocUiTest test;docUiSetup(&test,0);DocUiOwner callback={.refs=1,.test=&test};
    assert(docUiSet(&callback,false)==XUI_OK);callback.failDrop=true;
    unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(test.base.context,&watch,modelContextGone,&gone)==XUI_OK);
    doc_view_get(test.widget)->onChanged=closeContextOnChange;
    observerCommit(test.document);
    assert(gone==1 && callback.refs==1 && !xrtGetError());
    assert(xuiDocumentGetNotificationFailureCount(test.document)==1 && xuiDocumentGetNotificationFailureStatus(test.document)==XUI_ERROR_CALLBACK_FAILED);
    assert(xuiDocumentCheckNotifications(test.document,true)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(xuiDocumentReleaseChecked(test.document)==XUI_OK && xuiFontReleaseChecked(test.font)==XUI_OK && xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);rawSnapshotIsNotLeaf();
    for(int kind=0;kind<3;++kind){claimedBuiltinBacklinkCancels(kind);contextLastModelReportsNull(kind,false);contextLastModelReportsNull(kind,true);}internalObserverTailFailureIsLatched();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("UI model slots: raw inputs are opaque, claimed Native backlinks cancel without reacquire, final pending notification failure survives Context teardown with primary NULL");return 0;
}
