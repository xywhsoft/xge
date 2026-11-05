/* Actual Editor worker plus production View/Editor consumers. Only the parse
 * call is delayed by a CPU test barrier. No engine/window/browser/GPU exists. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui_document_ui.h"
#include <stdatomic.h>
static atomic_bool workerEntered,workerRelease,closeReturned;
static int delayedPrepareRun(xui_document_prepare);
#define xuiDocumentPrepareRun delayedPrepareRun
#define XUI_OWNED_DOC_UI_HELPERS
#include "xui_owned_doc_ui_callbacks_test.c"
#undef xuiDocumentPrepareRun
static int delayedPrepareRun(xui_document_prepare prepare)
{
    atomic_store_explicit(&workerEntered,true,memory_order_release);
    while(!atomic_load_explicit(&workerRelease,memory_order_acquire))xrtThreadYield();
    return xuiDocumentPrepareRun(prepare);
}
static void waitEntered(void)
{
    uint64_t end=xgeClockMicroseconds()+UINT64_C(5000000);
    while(!atomic_load_explicit(&workerEntered,memory_order_acquire)){
        assert(xgeClockMicroseconds()<end);xrtThreadYield();
    }
}
static void contextGone(void* raw){++*(unsigned*)raw;}
static void workerSetup(DocUiTest* test)
{
    memset(test,0,sizeof(*test));layoutSetup(&test->base,&test->font);
    xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN};
    assert(xuiDocumentCreateOwned(&desc,&test->document)==XUI_OK && xuiDocumentLoadMarkdown(test->document,"abc\n",4)==XUI_OK);
    xui_doc_editor_desc_t editor={.iSize=sizeof(editor),.iMode=XUI_DOC_SOURCE_TEXT,.iAsyncSourceThresholdBytes=1};
    editor.tView=(xui_doc_view_desc_t){.iSize=sizeof(editor.tView),.pDocument=test->document,
        .tRenderer={.iSize=sizeof(xui_doc_renderer_desc_t),.tFonts={test->font,test->font,test->font,test->font,test->font}}};
    assert(xuiDocumentEditorCreate(test->base.context,&editor,&test->widget)==XUI_OK);
    doc_editor_data* data=doc_editor_get(test->widget);assert(data);
    xui_doc_range_t range=data->view.selection;
    atomic_store(&workerEntered,false);atomic_store(&workerRelease,false);atomic_store(&closeReturned,false);
    assert(doc_editor_prepare_replace(data,&range,"X",1,1)==XUI_OK && data->worker && data->input);
    doc_input_schedule(data->worker,data->input);waitEntered();
}
static void workerFinish(DocUiTest* test)
{
    assert(xuiDocumentReleaseChecked(test->document)==XUI_OK);
    assert(xuiFontReleaseChecked(test->font)==XUI_OK && xuiProxyReleaseChecked(test->base.proxy)==XUI_OK && drop(&test->base.owner));
}
static void checkedNonblocking(void)
{
    DocUiTest test;workerSetup(&test);unsigned gone=0;xui_lifetime_link_t watch={0};
    assert(xuiContextLifetimeWatch(test.base.context,&watch,contextGone,&gone)==XUI_OK);
    bool consumed=true;
    assert(xuiDestroyOwnerChecked(test.base.context,&consumed)==XUI_STATUS_DEFERRED && !consumed && !gone);
    assert(!xuiInternalDestroyWaitAllowed(test.base.context) && !xrtGetError());
    doc_input_worker* worker=doc_editor_get(test.widget) ? doc_editor_get(test.widget)->worker:NULL;
    /* Normal Widget business is closed, but the actual owned carrier remains
     * in the Context's native directory until the proc has truly returned. */
    if(!worker)worker=((doc_editor_data*)test.widget->pTypeData)->worker;
    assert(worker && !doc_input_worker_finished(worker));atomic_store(&workerRelease,true);
    assert(xrtThreadWaitFor(worker->thread,5000)==XWAIT_OK);
    consumed=false;assert(xuiDestroyOwnerChecked(test.base.context,&consumed)==XUI_OK && consumed && gone==1);
    workerFinish(&test);
}
static int32 releaseAfterCloseStarts(void* raw)
{
    (void)raw;xrtSleep(20);assert(!atomic_load(&closeReturned));atomic_store_explicit(&workerRelease,true,memory_order_release);return 0;
}
static size_t actualDocumentRefs(xui_document document)
{
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));xrtownershipref ref=xuiDocumentOwnership(document);size_t count=0;
    assert(ref.Ops->Count(ref.Data,&count) && xrtOwnershipScopeEnd(&freeze));return count;
}
static void legacyVoidSynchronouslyCompletes(bool widgetOnly)
{
    DocUiTest test;workerSetup(&test);unsigned gone=0;xui_lifetime_link_t watch={0};
    assert(xuiContextLifetimeWatch(test.base.context,&watch,contextGone,&gone)==XUI_OK && actualDocumentRefs(test.document)==2);
    xthread* release=xrtThreadCreate(releaseAfterCloseStarts,NULL,0);assert(release);
    if(widgetOnly)xuiWidgetDestroy(test.widget);else xuiDestroy(test.base.context);
    atomic_store(&closeReturned,true);assert(xrtThreadWaitFor(release,5000)==XWAIT_OK);xrtThreadDestroy(release);
    assert(actualDocumentRefs(test.document)==1 && gone==(widgetOnly ? 0u:1u));
    if(widgetOnly)assert(xuiDestroyChecked(test.base.context)==XUI_OK && gone==1);
    workerFinish(&test);
}
typedef struct ModeProbe {
    xui_context outer,peer;
    bool inner;
    bool* peerReady;
    unsigned* visits;
} ModeProbe;
static int modeInit(xui_widget widget,void* raw,const void* desc,void* user)
{(void)widget;(void)user;*(ModeProbe*)raw=*(const ModeProbe*)desc;return XUI_OK;}
static int modePrepare(xui_widget widget,void* raw,bool* complete)
{
    ModeProbe* probe=raw;++*probe->visits;
    if(probe->inner){
        assert(!xuiInternalDestroyWaitAllowed(widget->pContext));
        if(!*probe->peerReady)assert(xuiInternalDestroyWaitAllowed(probe->outer));
        *complete=*probe->peerReady;return *complete ? XUI_OK:XUI_STATUS_DEFERRED;
    }
    assert(xuiInternalDestroyWaitAllowed(widget->pContext));
    xui_internal_destroy_scope_t inherited={0};
    xuiInternalDestroyScopeBegin(probe->peer,XUI_DESTROY_SYNC_WORKER,&inherited);
    bool consumed=true;assert(xuiDestroyOwnerChecked(probe->peer,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    assert(xuiInternalDestroyWaitAllowed(probe->peer) && xuiInternalDestroyWaitAllowed(widget->pContext));
    xuiInternalDestroyScopeEnd(&inherited);assert(!xuiInternalDestroyWaitAllowed(probe->peer));
    /* The same Context also restores explicit nested stack modes exactly. */
    xui_internal_destroy_scope_t checked={0},nestedVoid={0};
    xuiInternalDestroyScopeBegin(widget->pContext,XUI_DESTROY_NONBLOCKING,&checked);assert(!xuiInternalDestroyWaitAllowed(widget->pContext));
    xuiInternalDestroyScopeBegin(widget->pContext,XUI_DESTROY_SYNC_WORKER,&nestedVoid);assert(xuiInternalDestroyWaitAllowed(widget->pContext));
    xuiInternalDestroyScopeEnd(&nestedVoid);assert(!xuiInternalDestroyWaitAllowed(widget->pContext));
    xuiInternalDestroyScopeEnd(&checked);assert(xuiInternalDestroyWaitAllowed(widget->pContext));
    *complete=true;return XUI_OK;
}
static void modeCreate(xui_context context,const ModeProbe* probe)
{
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="model-mode-probe",.iTypeDataSize=sizeof(*probe),.onInit=modeInit};
    xui_widget_type type=NULL;assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);
    type->onDestroyPrepare=modePrepare;xui_widget widget=NULL;assert(xuiWidgetCreateTyped(context,type,&widget,probe)==XUI_OK);
}
static void exactModeScopes(void)
{
    LayoutProvider outer={0},peer={0};xui_font outerFont=NULL,peerFont=NULL;
    layoutSetup(&outer,&outerFont);layoutSetup(&peer,&peerFont);
    unsigned visits=0;bool peerReady=false;
    ModeProbe a={outer.context,peer.context,false,&peerReady,&visits},b={outer.context,peer.context,true,&peerReady,&visits};
    modeCreate(outer.context,&a);modeCreate(peer.context,&b);
    assert(!xuiInternalDestroyWaitAllowed(outer.context) && !xuiInternalDestroyWaitAllowed(peer.context));
    xuiDestroy(outer.context);assert(visits>=2 && !xuiInternalDestroyWaitAllowed(peer.context));
    peerReady=true;assert(xuiDestroyChecked(peer.context)==XUI_OK);
    assert(xuiFontReleaseChecked(outerFont)==XUI_OK && xuiProxyReleaseChecked(outer.proxy)==XUI_OK && drop(&outer.owner));
    assert(xuiFontReleaseChecked(peerFont)==XUI_OK && xuiProxyReleaseChecked(peer.proxy)==XUI_OK && drop(&peer.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    checkedNonblocking();legacyVoidSynchronouslyCompletes(false);legacyVoidSynchronouslyCompletes(true);exactModeScopes();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("actual Editor worker: checked DEFERRED retains full carrier; C void Context/Widget destruction waits outside graph/mutex scopes and physically completes");return 0;
}
