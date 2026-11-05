/* Actual native renderer carriers, no engine/browser/device initialization. */
#define XUI_OWNED_RENDERER_HELPERS
#include "xui_owned_renderer_test.c"
static void rendererBusyPayload(bool oom,bool lateFailure)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_document_renderer renderer=NULL;xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={.normal=font}};
    assert(xuiDocumentRendererCreate(p.context,&desc,&renderer)==XUI_OK);
    xui_document document=NULL;xui_doc_desc_t dd={.iSize=sizeof(dd),.iProfile=XUI_DOCUMENT_MARKDOWN};
    const char source[]="one **two** and *three* different fragments and words\n\nsecond paragraph\n";
    assert(xuiDocumentCreateOwned(&dd,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,source,sizeof(source)-1)==XUI_OK);
    xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
    if(!lateFailure){p.busyNextShape=true;p.oomAfterShape=oom;}
    int status=xuiDocumentRendererLayout(renderer,220,0,400);
    xrtMemDebugFailClear();xrtClearError();
    if(oom)assert(status==XUI_ERROR_OUT_OF_MEMORY);
    else assert(status==XUI_OK || status==XUI_ERROR_BUSY);
    assert(p.shapes>0);
    if(lateFailure){p.ready=true;p.dropMakesBusy=true;p.paintStatus=XUI_ERROR_BACKEND_FAILED;}
    bool consumed=true;status=xuiDocumentRendererReleaseOwnerChecked(renderer,&consumed);
    if(lateFailure)assert(status==XUI_ERROR_BACKEND_FAILED && !consumed && !xrtGetError());
    else assert(status==XUI_STATUS_DEFERRED && !consumed && !xrtGetError());
    unsigned edges=0;assert(xuiDocumentRendererTraceOwnership(renderer,layoutVisit,&edges) && edges>=2);
    p.ready=true;p.paintStatus=XUI_OK;
    assert(xuiDocumentRendererReleaseChecked(renderer)==XUI_OK && p.paintDrops==p.shapes);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1);
    assert(xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    rendererBusyPayload(true,false);rendererBusyPayload(false,false);rendererBusyPayload(false,true);
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("renderer: successful busy paint, post-producer OOM, late wait and consuming NULL failure preserve all real carriers");return 0;
}
