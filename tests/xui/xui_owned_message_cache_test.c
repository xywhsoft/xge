/* Real MessageList translation unit: unique line-shape scratch, no wrapper. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#include "../../src/xui/core/xui_text_internal.h"
#include "../../src/xui/document/xui_document_layout_internal.h"
#include "../../src/xui/controls/xui_message_list.c"
static xui_widget list;
static bool tryMutator;
static xui_document mutateDocument;
static const void* painted[4096];
static size_t paintSequence,blockedThrough=SIZE_MAX;
static bool messagePaintReady(const void* raw)
{
    const LayoutPaint* paint=raw;
    if(paint->provider->ready)return true;
    for(size_t i=paintSequence;i>0;--i)if(painted[i-1]==raw)return i>blockedThrough;
    return false;
}
static const xui_text_paint_ownership_v1_t messagePaintOps={sizeof(messagePaintOps),XUI_TEXT_PAINT_OWNERSHIP_V1_TAG,messagePaintReady,layoutPaintTrace,layoutPaintDrop};
static int messageFill(xui_proxy proxy,xui_draw_context draw,xui_rect_t rect,uint32_t color)
{(void)proxy;(void)draw;(void)rect;(void)color;return XUI_OK;}
static int messageShape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    int status=layoutShape(proxy,item,shape);
    if(shape->pPaint){assert(paintSequence<4096);painted[paintSequence++]=shape->pPaint;shape->pPaintOwnership=&messagePaintOps;}
    if(mutateDocument){xui_document document=mutateDocument;mutateDocument=NULL;const char text[]="reentrant model **changed**\n";assert(xuiDocumentLoadMarkdown(document,text,sizeof(text)-1)==XUI_OK);}
    if(tryMutator){tryMutator=false;assert(xuiMessageListClear(list)==XUI_ERROR_BUSY);}
    return status;
}
static void setupMessage(LayoutProvider* p,xui_font* font)
{
    layoutSetup(p,font);
    assert(xuiDestroyChecked(p->context)==XUI_OK && xuiProxyReleaseChecked(p->proxy)==XUI_OK);
    p->state.tProxy.textShape=messageShape;p->state.tProxy.drawRectFill=messageFill;xui_callback_owner_t binding=ownerRef(&p->owner);
    assert(xuiProxyCreateOwned(&p->state.tProxy,&binding,&p->proxy)==XUI_OK && xuiCreateOwned(p->proxy,&p->context)==XUI_OK);
    assert(xuiFontReleaseChecked(*font)==XUI_OK);*font=fontCreate(p->proxy,&p->fontDrops,XUI_OK);
    xui_message_node_t node={.iSize=sizeof(node),.sId="original",.sText="before",.iType=XUI_MESSAGE_NODE_OTHER};
    xui_message_list_desc_t desc={.iSize=sizeof(desc),.pFont=*font,.arrNodes=&node,.iNodeCount=1};
    assert(xuiMessageListCreate(p->context,&list,&desc)==XUI_OK);
    assert(xuiWidgetSetRect(list,(xui_rect_t){0,0,500,300})==XUI_OK);
    assert(__xuiMessageLayoutNodes(list,__xuiMessageListGetData(list))==XUI_OK);
}
static void finishMessage(LayoutProvider* p,xui_font font)
{
    assert(xuiDestroyChecked(p->context)==XUI_OK && p->paintDrops==p->shapes);
    assert(refs(xuiFontOwnership(font))==1 && xuiFontReleaseChecked(font)==XUI_OK);
    assert(xuiProxyReleaseChecked(p->proxy)==XUI_OK && drop(&p->owner));list=NULL;
}
static void actualCaretCarrier(bool oom)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    xui_message_list_data_t* data=__xuiMessageListGetData(list);xui_message_node_data_t* node=&data->arrNodes[0];
    xui_text_line_t line={0};assert(xuiTextLayoutGetLine(node->pTextLayout,0,&line)==XUI_OK);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    data->iCacheActivity=2;p.busyNextShape=true;p.oomAfterShape=oom;tryMutator=true;
    int status=__xuiMessageEnsureLineCarets(list,data,node,node->pTextLayout,0,&line);
    xrtMemDebugFailClear();xrtClearError();data->iCacheActivity=0;
    assert(!tryMutator && data->tCaretShape.pPaint && status==(oom ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
    assert((oom && !node->iTextCaretCount) || (!oom && node->iTextCaretCount>0));
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    unsigned drops=p.paintDrops;assert(xuiDestroyChecked(p.context)==XUI_STATUS_DEFERRED && p.paintDrops==drops && !xrtGetError());
    p.ready=true;finishMessage(&p,font);
}
static void actualReplacementWait(void)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);p.ready=false;
    xui_message_node_t replacement={.iSize=sizeof(replacement),.sId="new",.sText="after",.iType=XUI_MESSAGE_NODE_OTHER};
    assert(xuiMessageListSetNodes(list,&replacement,1)==XUI_ERROR_BUSY);
    const xui_message_node_t* old=xuiMessageListGetNode(list,0);assert(old && !strcmp(old->sText,"before"));
    assert(xuiMessageListClear(list)==XUI_ERROR_BUSY && xuiMessageListGetNodeCount(list)==1 && !xrtGetError());
    p.ready=true;assert(xuiMessageListSetNodes(list,&replacement,1)==XUI_OK);
    finishMessage(&p,font);
}
typedef struct MessageRenderOwner {Owner counted;unsigned calls;int result;bool failDrop;} MessageRenderOwner;
static int messageRender(xui_widget widget,int index,const xui_message_node_t* node,xui_draw_context draw,xui_rect_t rect,int state,void* raw)
{
    (void)rect;(void)state;MessageRenderOwner* owner=raw;
    assert(widget==list && index==0 && node->sText && draw && owner->counted.refs==3);
    ++owner->calls;
    assert(xuiMessageListSetNodeRendererOwned(widget,NULL,NULL,NULL)==XUI_OK && owner->counted.refs==2);
    owner->counted.failDrop=owner->failDrop;return owner->result;
}
static void actualOwnedRenderer(int result,bool failedDrop)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    MessageRenderOwner owner={.counted={.refs=1},.result=result,.failDrop=failedDrop};
    xui_callback_owner_t binding=ownerRef(&owner.counted);
    assert(xuiMessageListSetNodeRendererOwned(list,messageRender,&owner,&binding)==XUI_OK && owner.counted.refs==2);
    xui_draw_context draw=NULL;void* payload=NULL;
    assert(xuiDrawCreateOwned(p.proxy,NULL,0,layoutSurfaceFinalize,&draw,&payload)==XUI_OK);
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(p.context,&outer)==XUI_OK);
    int status=__xuiMessageCacheRender(list,draw,0,NULL);
    assert(status==(failedDrop ? XUI_ERROR_CALLBACK_FAILED:result<0 ? result:XUI_OK) && owner.calls==1 && owner.counted.refs==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==(failedDrop ? XUI_ERROR_CALLBACK_FAILED:XUI_OK) && !xrtGetError());
    owner.counted.failDrop=false;assert(drop(&owner.counted));
    assert(xuiDrawReleaseChecked(draw)==XUI_OK);finishMessage(&p,font);
}
static xui_document messageDocument(const char* source)
{
    xui_document document=NULL;xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN};
    assert(xuiDocumentCreateOwned(&desc,&document)==XUI_OK);
    assert(xuiDocumentLoadMarkdown(document,source,strlen(source))==XUI_OK);return document;
}
static xui_message_document_desc_t messageDocumentDesc(xui_document document,xui_font font)
{
    return (xui_message_document_desc_t){.iSize=sizeof(xui_message_document_desc_t),.pDocument=document,
        .tRenderer={.iSize=sizeof(xui_doc_renderer_desc_t),.tFonts={.normal=font}}};
}
static void actualDocumentRetirement(bool lateFailure)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    xui_document document=messageDocument("one **two** and *three* fragments\n\nsecond paragraph\n");
    xui_message_document_desc_t desc=messageDocumentDesc(document,font);
    assert(xuiMessageListSetNodeDocument(list,"original",&desc)==XUI_OK);
    xui_message_list_data_t* data=__xuiMessageListGetData(list);
    assert(data->arrNodes[0].pDocumentBinding && p.shapes>0);
    if(lateFailure){p.dropMakesBusy=true;p.paintStatus=XUI_ERROR_BACKEND_FAILED;}
    else p.ready=false;
    assert(xuiMessageListClear(list)==(lateFailure ? XUI_ERROR_BACKEND_FAILED:XUI_ERROR_BUSY));
    assert(xuiMessageListGetNodeCount(list)==1 && xuiMessageListGetNodeDocument(list,"original")==document && !xrtGetError());
    assert(data->arrNodes[0].pDocumentBinding->bRetiring);
    xui_doc_renderer_stats_t stats={.iSize=sizeof(stats)};
    assert(xuiMessageListGetNodeDocumentRenderStats(list,"original",&stats)==XUI_ERROR_BUSY);
    p.ready=true;p.paintStatus=XUI_OK;assert(xuiMessageListClear(list)==XUI_OK && !xuiMessageListGetNodeCount(list));
    xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);finishMessage(&p,font);
}
static void actualDocumentCandidate(void)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    xui_document document=messageDocument("candidate **bold** and *italic* body\n");
    xui_message_document_desc_t desc=messageDocumentDesc(document,font);
    p.busyNextShape=p.oomAfterShape=true;
    int status=xuiMessageListSetNodeDocument(list,"original",&desc);
    xrtMemDebugFailClear();xrtClearError();assert(status==XUI_ERROR_OUT_OF_MEMORY);
    xui_message_list_data_t* data=__xuiMessageListGetData(list);
    assert(!data->arrNodes[0].pDocumentBinding && data->arrNodes[0].pWorkBinding && data->arrNodes[0].pWorkBinding->pRenderer);
    assert(xuiMessageListClear(list)==XUI_ERROR_BUSY && !xrtGetError());
    p.ready=true;assert(xuiMessageListClear(list)==XUI_OK);xuiDocumentRelease(document);finishMessage(&p,font);
}
static void actualDocumentReplacement(void)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    xui_document a=messageDocument("first **bold** body\n"),b=messageDocument("second *italic* body\n");
    xui_message_document_desc_t da=messageDocumentDesc(a,font),db=messageDocumentDesc(b,font);
    assert(xuiMessageListSetNodeDocument(list,"original",&da)==XUI_OK);blockedThrough=paintSequence;p.ready=false;
    int status=xuiMessageListSetNodeDocument(list,"original",&db);
    assert(status==XUI_ERROR_BUSY && xuiMessageListGetNodeDocument(list,"original")==b);
    xui_message_list_data_t* data=__xuiMessageListGetData(list);
    assert(data->arrNodes[0].pWorkBinding && data->arrNodes[0].pWorkBinding->pDocument==a);
    p.ready=true;blockedThrough=SIZE_MAX;assert(xuiMessageListClear(list)==XUI_OK);
    xuiDocumentRelease(a);xuiDocumentRelease(b);finishMessage(&p,font);
}
static void actualDocumentFontRefresh(bool modelEdit)
{
    LayoutProvider p={0};xui_font font;setupMessage(&p,&font);
    xui_document document=messageDocument("font **bold** body\n");
    xui_message_document_desc_t desc=messageDocumentDesc(document,NULL);
    assert(xuiMessageListSetNodeDocument(list,"original",&desc)==XUI_OK);
    xui_message_list_data_t* data=__xuiMessageListGetData(list);
    xui_message_document_binding_t* binding=data->arrNodes[0].pDocumentBinding;
    xui_font next=fontCreate(p.proxy,&p.fontDrops,XUI_OK);
    if(modelEdit)mutateDocument=document;
    else {blockedThrough=paintSequence;p.ready=false;}
    int status=xuiMessageListSetFont(list,next);
    if(status!=XUI_ERROR_BUSY || xrtGetError())fprintf(stderr,"font refresh model=%d status=%d error=%p work=%p ready=%d\n",modelEdit,status,(const void*)xrtGetError(),(void*)binding->pWorkRenderer,binding->bWorkReady);
    assert(status==XUI_ERROR_BUSY && !xrtGetError());
    if(modelEdit)assert(!mutateDocument && (binding->bNeedsSync || xuiDocumentSnapshotGetRevision(binding->pRenderer->snapshot)==xuiDocumentGetRevision(document)));
    else assert(binding->pWorkRenderer && binding->bWorkReady && binding->pRenderer->retiring);
    p.ready=true;blockedThrough=SIZE_MAX;
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    data->iCacheActivity=2;assert(__xuiMessageLayoutNodes(list,data)==XUI_OK);data->iCacheActivity=0;
    assert(xuiCallScopeEnd(&scope)==XUI_OK && !binding->pWorkRenderer);
    assert(xuiDocumentSnapshotGetRevision(binding->pRenderer->snapshot)==xuiDocumentGetRevision(document));
    assert(binding->pResolvedFont==xuiMessageListGetFont(list));
    assert(xuiMessageListClear(list)==XUI_OK);xuiDocumentRelease(document);finishMessage(&p,font);
    assert(refs(xuiFontOwnership(next))==1 && xuiFontReleaseChecked(next)==XUI_OK);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    actualCaretCarrier(false);actualCaretCarrier(true);actualReplacementWait();
    actualOwnedRenderer(1,false);actualOwnedRenderer(XUI_ERROR_BACKEND_FAILED,false);actualOwnedRenderer(1,true);
    actualDocumentRetirement(false);actualDocumentRetirement(true);actualDocumentCandidate();actualDocumentReplacement();
    actualDocumentFontRefresh(false);actualDocumentFontRefresh(true);
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("MessageList real native owning graph: line-shape scratch, BUSY/partial OOM/reentry, Owned renderer snapshot and local versus cleanup failures, active/draft Document binding retirement and final balance");return 0;
}
