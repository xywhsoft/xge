/* CPU-only native Renderer: no browser, engine/window init, graphics or network. */
#define XUI_OWNED_LAYOUT_HELPERS 1
#include "xui_owned_layout_test.c"
#undef XUI_OWNED_LAYOUT_HELPERS
#include "xui_document_ui.h"

typedef struct RendererOwner {
    Owner counted;
    xui_document_renderer renderer;
    xui_font font;
    unsigned calls;
    bool selfClear,failNull,closeContext;
    xui_context context;
} RendererOwner;
static int rendererFont(xui_document_renderer renderer,const char* family,uint32 marks,float size,
    xui_doc_font_output_t* output,void* raw)
{
    (void)family;(void)marks;(void)size;RendererOwner* owner=raw;++owner->calls;
    assert(owner->counted.refs>=3);
    assert(xuiDocumentRendererOwnershipBusy(renderer));
    if(owner->selfClear){assert(xuiDocumentRendererSetFontResolverOwned(renderer,NULL,NULL,NULL)==XUI_OK);assert(owner->counted.refs>=2);}
    if(owner->closeContext)assert(xuiDestroyChecked(owner->context)==XUI_STATUS_DEFERRED);
    if(owner->failNull){xrtClearError();return XUI_ERROR_BACKEND_FAILED;}
    assert(xuiDocumentFontOutputSetFont(output,owner->font)==XUI_OK);return 1;
}
static xui_document rendererDocument(void)
{
    xui_document document=NULL;xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN};
    const char source[]="ordinary text\n\nsecond paragraph\n";
    assert(xuiDocumentCreate(&desc,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,source,sizeof(source)-1)==XUI_OK);
    return document;
}
static void rendererOwnedLifetime(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_document_renderer renderer=NULL;xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={.normal=font}};
    assert(xuiDocumentRendererCreate(p.context,&desc,&renderer)==XUI_OK && refs(xuiFontOwnership(font))==2);
    RendererOwner owner={.counted={.refs=1},.renderer=renderer,.font=font,.selfClear=true};
    xui_callback_owner_t binding=ownerRef(&owner.counted);
    assert(xuiDocumentRendererSetFontResolverOwned(renderer,rendererFont,&owner,&binding)==XUI_OK && owner.counted.refs==2);
    xui_document document=rendererDocument();xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
    assert(xuiDocumentRendererLayout(renderer,640,0,400)==XUI_OK && owner.calls==1 && owner.counted.refs==1);
    unsigned edges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiDocumentRendererTraceOwnership(renderer,layoutVisit,&edges) && edges>=4);assert(xrtOwnershipScopeEnd(&freeze));
    p.ready=false;assert(xuiDocumentRendererInvalidateFonts(renderer)==XUI_ERROR_BUSY);p.ready=true;
    assert(xuiDestroyChecked(p.context)==XUI_OK && xuiDocumentRendererGetContext(renderer)==NULL);
    assert(xuiDocumentRendererLayout(renderer,640,0,400)==XUI_ERROR_INVALID_STATE);
    assert(xuiDocumentRendererReleaseChecked(renderer)==XUI_OK && refs(xuiFontOwnership(font))==1 && p.shapes==p.paintDrops);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    assert(xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1 && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner) && drop(&owner.counted));
}
static void rendererFailureAndContextClose(bool closeContext)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_document_renderer renderer=NULL;xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={.normal=font}};
    assert(xuiDocumentRendererCreate(p.context,&desc,&renderer)==XUI_OK);
    RendererOwner owner={.counted={.refs=1},.renderer=renderer,.font=font,.failNull=!closeContext,.closeContext=closeContext,.context=p.context};
    xui_callback_owner_t binding=ownerRef(&owner.counted);assert(xuiDocumentRendererSetFontResolverOwned(renderer,rendererFont,&owner,&binding)==XUI_OK);
    xui_document document=rendererDocument();xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
    int status=xuiDocumentRendererLayout(renderer,640,0,400);assert(status==(closeContext?XUI_ERROR_INVALID_STATE:XUI_ERROR_BACKEND_FAILED));
    assert(xrtGetError()==NULL && owner.counted.refs==2);
    assert(xuiDocumentRendererReleaseChecked(renderer)==XUI_OK && owner.counted.refs==1);
    assert(xuiDestroyChecked(p.context)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    assert(xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner) && drop(&owner.counted));
}

#ifndef XUI_OWNED_RENDERER_HELPERS
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    rendererOwnedLifetime();rendererFailureAndContextClose(false);rendererFailureAndContextClose(true);
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("unique Renderer: owned callback output, physical snapshot tail, self-clear, exact NULL failure, native cache edges and Context expiry passed");return 0;
}

#endif /* XUI_OWNED_RENDERER_HELPERS */
