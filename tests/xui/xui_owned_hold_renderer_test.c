/* Actual unique Renderer and owning Font output; no browser/device startup. */
#define XUI_OWNED_RENDERER_HELPERS
#include "xui_owned_renderer_test.c"
#undef XUI_OWNED_RENDERER_HELPERS
typedef struct RendererHold {
    size_t refs;unsigned holds,drops,calls;
    bool fresh,fail,dropFailure;
    xui_font font;
} RendererHold;
static bool rendererHoldAcquire(const void* raw)
{RendererHold* owner=(RendererHold*)raw;++owner->holds;if(owner->fail)return false;++owner->refs;if(owner->fresh)xrtSetErrorKind(XERR_RANGE);return true;}
static bool rendererHoldDrop(const void* raw)
{RendererHold* owner=(RendererHold*)raw;assert(owner->refs);--owner->refs;++owner->drops;if(owner->dropFailure)xrtSetErrorKind(XERR_STATE);return !owner->dropFailure;}
static bool rendererHoldCount(const void* raw,size_t* count){*count=((const RendererHold*)raw)->refs;return *count!=0;}
static const xrtownershipops rendererHoldOps={rendererHoldCount,trace};
static xui_callback_owner_t rendererHoldRef(RendererHold* owner)
{return(xui_callback_owner_t){{owner,&rendererHoldOps},rendererHoldAcquire,rendererHoldDrop};}
static int rendererHoldFont(xui_document_renderer renderer,const char* family,uint32 marks,float size,xui_doc_font_output_t* output,void* raw)
{(void)renderer;(void)family;(void)marks;(void)size;RendererHold* owner=raw;++owner->calls;assert(xuiDocumentFontOutputSetFont(output,owner->font)==XUI_OK);return 1;}
static void rendererFreshOutcomes(void)
{
    LayoutProvider provider={0};xui_font font;layoutSetup(&provider,&font);
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={.normal=font}};xui_document_renderer renderer=NULL;assert(xuiDocumentRendererCreate(provider.context,&desc,&renderer)==XUI_OK);
    RendererHold old={.refs=1,.font=font},candidate={.refs=1,.font=font,.fresh=true,.dropFailure=true};xui_callback_owner_t oldRef=rendererHoldRef(&old),candidateRef=rendererHoldRef(&candidate);
    assert(xuiDocumentRendererSetFontResolverOwned(renderer,rendererHoldFont,&old,&oldRef)==XUI_OK);
    assert(xuiDocumentRendererSetFontResolverOwned(renderer,rendererHoldFont,&candidate,&candidateRef)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && candidate.refs==1 && candidate.drops==1 && old.refs==2);xrtClearError();
    xui_document document=rendererDocument();xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
    old.fresh=true;unsigned drops=old.drops;assert(xuiDocumentRendererLayout(renderer,640,0,400)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && !old.calls && old.refs==2 && old.drops==drops+1);xrtClearError();old.fresh=false;
    assert(xuiDocumentRendererLayout(renderer,640,0,400)==XUI_OK && old.calls>0);
    unsigned calls=old.calls;assert(xuiDocumentRendererInvalidateFonts(renderer)==XUI_OK);old.fail=true;drops=old.drops;
    assert(xuiDocumentRendererLayout(renderer,640,0,400)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && old.calls==calls && old.drops==drops);old.fail=false;
    assert(xuiDocumentRendererReleaseChecked(renderer)==XUI_OK && xuiDestroyChecked(provider.context)==XUI_OK && old.refs==1);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    assert(xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(provider.proxy)==XUI_OK && drop(&provider.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);rendererFreshOutcomes();xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("Renderer fresh Hold outcome: candidate rollback, actual consumer snapshot rollback, falseNULL no extra Drop passed");return 0;
}
