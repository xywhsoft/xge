/* Explicit capability gates must reject before calling a permissive backend;
 * capabilities are cached when the context accepts its proxy. */
static xui_proxy_get_caps_proc input_caps_base_get;
static xui_text_shape_proc input_caps_base_shape;
static uint32_t input_caps_mask;
static unsigned input_caps_calls;
static xui_draw_text_proc input_caps_base_draw;
static xui_draw_text_spans_proc input_caps_base_spans;
static unsigned input_caps_draws;
static void input_caps_check_draw(const xui_text_item_t* item)
{
    if(item->iTextSize!=4 || memcmp(item->sText,"i\xc3\xa9i",4))return;
    CHECK(!!item->sContext==!!(input_caps_mask & XUI_PROXY_CAP_TEXT_CONTEXT));
    CHECK(!!item->iScript==!!(input_caps_mask & XUI_PROXY_CAP_TEXT_SCRIPT));
    if(item->sContext)CHECK(item->iContextSize==4 && !item->iContextOffset && !memcmp(item->sContext,item->sText,4));
    if(item->iScript)CHECK(item->iScript==UINT32_C(0x4c61746e));
    CHECK(!item->sLanguage && !(item->iFlags & XUI_TEXT_SHAPE_RTL));input_caps_draws++;
}
static int input_caps_draw(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{input_caps_check_draw(item);return input_caps_base_draw(proxy,draw,item,rect,color,flags);}
static int input_caps_spans(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{input_caps_check_draw(item);return input_caps_base_spans(proxy,draw,item,rect,color,flags,spans,count);}
static const uint32_t input_caps_bits[4]={XUI_PROXY_CAP_TEXT_CONTEXT,XUI_PROXY_CAP_TEXT_SCRIPT,
    XUI_PROXY_CAP_TEXT_LANGUAGE,XUI_PROXY_CAP_TEXT_RTL};
static int input_caps_get(xui_proxy proxy,xui_proxy_caps_t* out)
{
    int result=input_caps_base_get(proxy,out);
    if(result==XUI_OK)out->iCaps=(out->iCaps & ~(input_caps_bits[0]|input_caps_bits[1]|input_caps_bits[2]|input_caps_bits[3]))|input_caps_mask;
    return result;
}
static int input_caps_shape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* out)
{input_caps_calls++;return input_caps_base_shape(proxy,item,out);}
static void text_input_capability_cases(xui_test_proxy_state_t* proxy)
{
    input_caps_base_get=proxy->tProxy.getCaps;input_caps_base_shape=proxy->tProxy.textShape;
    proxy->tProxy.getCaps=input_caps_get;proxy->tProxy.textShape=input_caps_shape;
    for(unsigned mask=0;mask<16;mask++){
        xui_context context;xui_font font;xui_proxy_caps_t caps;
        input_caps_mask=0;for(unsigned bit=0;bit<4;bit++)if(mask & (1u<<bit))input_caps_mask|=input_caps_bits[bit];
        uint32_t accepted=input_caps_mask;
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy->tProxy)==XUI_OK &&
            xuiGetProxyCaps(context,&caps)==XUI_OK &&
            (caps.iCaps & (input_caps_bits[0]|input_caps_bits[1]|input_caps_bits[2]|input_caps_bits[3]))==accepted &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy,&font,"capabilities.ttf",20,0)==XUI_OK);
        /* Changing the provider's answer cannot change this context's policy. */
        input_caps_mask=~accepted & (input_caps_bits[0]|input_caps_bits[1]|input_caps_bits[2]|input_caps_bits[3]);
        for(unsigned field=0;field<5;field++){
            xui_text_item_t item={0},before;xui_text_shape_t shape={0};
            item.iSize=sizeof(item);item.pFont=font;item.sText="i";item.iTextSize=1;item.iFlags=XUI_TEXT_SHAPE_DEFAULT;
            if(field==1){item.sContext="ii";item.iContextSize=2;item.iContextOffset=1;}
            if(field==2)item.iScript=UINT32_C(0x4c61746e);
            if(field==3)item.sLanguage="en";
            if(field==4)item.iFlags|=XUI_TEXT_SHAPE_RTL;
            before=item;input_caps_calls=0;
            int supported=!field || (accepted & input_caps_bits[field-1]);
            CHECK(xuiTextShape(context,&item,&shape)==(supported?XUI_OK:XUI_ERROR_UNSUPPORTED));
            CHECK(input_caps_calls==(unsigned)!!supported && !memcmp(&item,&before,sizeof(item)));
            if(!supported)CHECK(!shape.pClusters && !shape.pCarets && !shape.iClusterCount);
            xuiTextShapeFree(&shape);
            item.sLanguage="en--US";input_caps_calls=0;
            CHECK(xuiTextShape(context,&item,&shape)==XUI_ERROR_INVALID_ARGUMENT && !input_caps_calls && !shape.pClusters);
        }
        proxy->tProxy.fontDestroy(&proxy->tProxy,font);xuiDestroy(context);
    }
    input_caps_mask=input_caps_bits[0]|input_caps_bits[1]|input_caps_bits[2]|input_caps_bits[3];
    proxy->tProxy.textShape=NULL;
    {xui_context context;xui_font font;xui_proxy_caps_t caps;xui_text_shape_t shape={0};
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy->tProxy)==XUI_OK &&
            xuiGetProxyCaps(context,&caps)==XUI_OK && !(caps.iCaps & input_caps_mask) &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy,&font,"capabilities.ttf",20,0)==XUI_OK);
        xui_text_item_t item={0};item.iSize=sizeof(item);item.pFont=font;item.sText="i";item.iTextSize=1;
        CHECK(xuiTextShape(context,&item,&shape)==XUI_OK && shape.iClusterCount==1);xuiTextShapeFree(&shape);
        item.sLanguage="en";CHECK(xuiTextShape(context,&item,&shape)==XUI_ERROR_UNSUPPORTED && !shape.pClusters);
        proxy->tProxy.fontDestroy(&proxy->tProxy,font);xuiDestroy(context);}
    proxy->tProxy.textShape=input_caps_base_shape;proxy->tProxy.getCaps=input_caps_base_get;
    puts("Text input capabilities: all 16 independent masks, cached policy, reject-before-callback, invalid input precedence and measurement-only fallback passed");
}
static void document_input_capability_cases(xui_test_proxy_state_t* proxy)
{
    input_caps_base_get=proxy->tProxy.getCaps;input_caps_base_draw=proxy->tProxy.drawText;
    input_caps_base_spans=proxy->tProxy.drawTextSpans;proxy->tProxy.getCaps=input_caps_get;
    proxy->tProxy.drawText=input_caps_draw;
    for(unsigned backend=0;backend<2;backend++)for(unsigned mask=0;mask<4;mask++){
        xui_context context;xui_font font;xui_surface surface;xui_document document;
        xui_document_transaction transaction;xui_document_snapshot snapshot;xui_document_renderer renderer;
        xui_doc_desc_t profile={0};xui_doc_node_desc_t node={0};uint64_t paragraph,leaf;
        xui_doc_renderer_desc_t desc={0};xui_draw_context draw;xui_doc_position_t position={0};xui_doc_rect_t caret;
        input_caps_mask=(mask&1?XUI_PROXY_CAP_TEXT_CONTEXT:0)|(mask&2?XUI_PROXY_CAP_TEXT_SCRIPT:0);
        proxy->tProxy.drawTextSpans=backend?NULL:input_caps_spans;
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy->tProxy)==XUI_OK &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy,&font,"capabilities.ttf",20,0)==XUI_OK &&
            xuiSetDefaultFont(context,font)==XUI_OK && xuiTestSurfaceCreate(proxy,&surface,320,120,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
        profile.iSize=sizeof(profile);profile.iProfile=XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK && xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
        node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
        node.iKind=XUI_DOC_TEXT;node.sText="i\xc3\xa9i";node.iTextBytes=4;
        CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,0,&node,&leaf)==XUI_OK && xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);
        xuiDocumentTxnRelease(transaction);
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&renderer)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererLayout(renderer,320,0,120)==XUI_OK);
        position.iSize=sizeof(position);position.iDocumentId=xuiDocumentGetIdentity(document);
        position.iRevision=xuiDocumentGetRevision(document);position.iNodeId=leaf;position.iKind=XUI_DOC_POSITION_TEXT;position.iOffset=4;
        CHECK(xuiDocumentRendererGetCaretRect(renderer,&position,&caret)==XUI_OK && caret.x>0);
        input_caps_draws=0;CHECK(proxy->tProxy.drawBegin(&proxy->tProxy,&draw,surface)==XUI_OK &&
            xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,320,120},NULL,0)==XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy,draw)==XUI_OK && input_caps_draws==1);
        xuiDocumentRendererRelease(renderer);xuiDocumentRelease(document);
        proxy->tProxy.surfaceDestroy(&proxy->tProxy,surface);proxy->tProxy.fontDestroy(&proxy->tProxy,font);xuiDestroy(context);
    }
    proxy->tProxy.getCaps=input_caps_base_get;proxy->tProxy.drawText=input_caps_base_draw;proxy->tProxy.drawTextSpans=input_caps_base_spans;
    puts("Document input capabilities: independent context/script masks reach final draw unchanged on both backends; ordinary non-ASCII layout and caret passed");
}
