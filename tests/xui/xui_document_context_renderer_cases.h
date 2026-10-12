static xui_text_shape_proc retained_base_shape;
static xui_draw_text_proc retained_base_draw;
static xui_draw_text_spans_proc retained_base_spans;
static const char *retained_source,*retained_seen;
static int retained_bytes,retained_fail,retained_allow_replace;
static const char* retained_stage;
static unsigned retained_shapes,retained_draws;
static void retained_check(const xui_text_item_t* item,int paint)
{
    int i;
    if(!retained_source)return;
    if(!item->sContext){CHECK(!paint);return;} /* Initial isolated run seeds. */
    if(item->iContextSize!=retained_bytes)fprintf(stderr,"Retained context stage=%s bytes=%d expected=%d\n",retained_stage,item->iContextSize,retained_bytes);
    CHECK(item->iContextSize==retained_bytes && item->iContextOffset>=0 &&
        item->iTextSize>=0 && item->iTextSize<=retained_bytes-item->iContextOffset);
    if(!retained_seen || (retained_allow_replace && item->sContext!=retained_seen)){
        CHECK(!memcmp(item->sContext,retained_source,(size_t)retained_bytes+1));retained_seen=item->sContext;
    }
    if(item->sContext!=retained_seen || memcmp(item->sText,retained_source+item->iContextOffset,(size_t)item->iTextSize))
        fprintf(stderr,"Retained context stage=%s paint=%d offset=%d bytes=%d same owner=%d\n",retained_stage,paint,item->iContextOffset,item->iTextSize,item->sContext==retained_seen);
    CHECK(item->sContext==retained_seen && !memcmp(item->sText,retained_source+item->iContextOffset,(size_t)item->iTextSize));
    for(i=0;i<item->iTextSize;i++){
        int offset=(item->iContextOffset+i)%7;
        CHECK(item->iScript==(offset==2 || offset==3?UINT32_C(0x4772656b):UINT32_C(0x4c61746e)));
    }
    if(paint)retained_draws++;else retained_shapes++;
}
static int retained_shape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* out)
{
    retained_check(item,0);
    if(retained_source && item->sContext && retained_fail){retained_fail=0;return XUI_ERROR_OUT_OF_MEMORY;}
    return retained_base_shape(proxy,item,out);
}
static int retained_draw(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{retained_check(item,1);return retained_base_draw(proxy,dc,item,rect,color,flags);}
static int retained_spans(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{retained_check(item,1);return retained_base_spans(proxy,dc,item,rect,color,flags,spans,count);}
static void document_retained_context(xui_test_proxy_state_t* proxy)
{
    xui_context context;xui_font font;xui_surface target;unsigned profile,backend;int i;
    xui_text_shape_proc shape=proxy->tProxy.textShape;xui_draw_text_proc draw=proxy->tProxy.drawText;
    xui_draw_text_spans_proc spans=proxy->tProxy.drawTextSpans;char *text=malloc(56001),*display=malloc(56001);CHECK(text && display);
    for(i=0;i<8000;i++)memcpy(text+i*7,"a(\xce\xbb)a ",7);
    text[56000]=0;
    retained_base_shape=shape;retained_base_draw=draw;retained_base_spans=spans;
    proxy->tProxy.textShape=retained_shape;proxy->tProxy.drawText=retained_draw;
    for(backend=0;backend<2;backend++)for(profile=0;profile<2;profile++){
        xui_document document;xui_document_snapshot snapshot;xui_document_renderer lazy,full;
        xui_doc_desc_t desc={0};xui_doc_renderer_stats_t stats={0};uint64_t paragraph,node;
        xui_doc_position_t start,deep;xui_doc_rect_t first,actual,expected;xui_draw_context dc;
        const char* context_text;
        proxy->tProxy.drawTextSpans=backend?NULL:retained_spans;
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy->tProxy)==XUI_OK &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy,&font,"retained.ttf",20,0)==XUI_OK &&
            xuiSetDefaultFont(context,font)==XUI_OK && xuiTestSurfaceCreate(proxy,&target,320,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
        desc.iSize=sizeof(desc);desc.iProfile=profile?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
        if(profile){
            CHECK(xuiDocumentLoadMarkdown(document,text,56000)==XUI_OK && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&node)==XUI_OK);
        }else{
            xui_document_transaction transaction;xui_doc_node_desc_t child={0};
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            child.iSize=sizeof(child);child.iKind=XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&child,&paragraph)==XUI_OK);
            child.iKind=XUI_DOC_TEXT;child.sText=text;child.iTextBytes=56000;
            CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&child,&node)==XUI_OK &&
                xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
        }
        start=decoration_position(document,node,2);deep=decoration_position(document,node,42002);
        CHECK(xuiDocumentRendererCreate(context,NULL,&lazy)==XUI_OK && xuiDocumentRendererSetSnapshot(lazy,snapshot,NULL)==XUI_OK &&
            xuiDocumentRendererCreate(context,NULL,&full)==XUI_OK && xuiDocumentRendererSetSnapshot(full,snapshot,NULL)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        retained_bytes=profile?55999:56000;memcpy(display,text,(size_t)retained_bytes);display[retained_bytes]=0;
        retained_source=display;retained_seen=NULL;retained_shapes=retained_draws=0;
        retained_stage="prefix";
        CHECK(xuiDocumentRendererLayout(lazy,240,0,48)==XUI_OK && xuiDocumentRendererGetCaretRect(lazy,&start,&first)==XUI_OK);
        stats.iSize=sizeof(stats);CHECK(xuiDocumentRendererGetStats(lazy,&stats)==XUI_OK && stats.iTextRunBytes<28000 && retained_shapes>0);
        context_text=retained_seen;
        retained_seen=NULL;retained_allow_replace=1;retained_stage="reference-full";
        CHECK(xuiDocumentRendererLayout(full,240,0,1e9)==XUI_OK && xuiDocumentRendererGetCaretRect(full,&deep,&expected)==XUI_OK);
        xuiDocumentRendererRelease(full);retained_seen=context_text;retained_allow_replace=0;retained_stage="extension-failure";
        retained_fail=1;
        CHECK(xuiDocumentRendererGetCaretRect(lazy,&deep,&actual)==XUI_ERROR_OUT_OF_MEMORY && !retained_fail &&
            xuiDocumentRendererGetCaretRect(lazy,&start,&actual)==XUI_OK && fabs(actual.x-first.x)<.001 && fabs(actual.y-first.y)<.001);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy,&dc,target)==XUI_OK &&
            xuiDocumentRendererDraw(lazy,dc,0,0,(xui_rect_t){0,0,320,160},NULL,0)==XUI_OK && proxy->tProxy.drawEnd(&proxy->tProxy,dc)==XUI_OK);
        retained_stage="extension-retry";
        CHECK(xuiDocumentRendererGetCaretRect(lazy,&deep,&actual)==XUI_OK && fabs(actual.x-expected.x)<.001 && fabs(actual.y-expected.y)<.001 &&
            retained_seen==context_text && retained_draws>0);
        retained_stage="width-reflow";
        CHECK(xuiDocumentRendererLayout(lazy,200,0,48)==XUI_OK && xuiDocumentRendererGetCaretRect(lazy,&start,&actual)==XUI_OK &&
            retained_seen==context_text);
        xuiDocumentRendererRelease(lazy);xuiDocumentRelease(document);retained_source=NULL;
        proxy->tProxy.surfaceDestroy(&proxy->tProxy,target);xuiDestroy(context);proxy->tProxy.fontDestroy(&proxy->tProxy,font);
    }
    proxy->tProxy.textShape=shape;proxy->tProxy.drawText=draw;proxy->tProxy.drawTextSpans=spans;
    free(text);free(display);
    puts("Document retained paragraph context: Rich/Markdown and spans/draw-only, 56 KiB cached scripts/brackets across cuts, bounded prefix, cold deep caret/full-layout equality, extension failure geometry/draw/retry and width reflow reuse passed");
}
