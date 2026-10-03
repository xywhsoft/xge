/* Whole-paragraph SHY storage must stay bounded even when a narrow line
 * planner tries hundreds of selected hyphens. Inspect retained owners and
 * independently compare callback bytes with the projected source. */
#include "../src/xui_document_layout_internal.h"
static xui_text_shape_proc shy_base_shape;
static xui_draw_text_proc shy_base_draw;
static xui_draw_text_spans_proc shy_base_spans;
static const char* shy_expected;
static size_t shy_expected_bytes;
static unsigned shy_shapes,shy_draws,shy_hyphens;
static int shy_fail;
static void shy_check_item(const xui_text_item_t* item,int paint)
{
    const char* insertion;size_t prefix;
    if(!shy_expected)return;
    if(!item->sContext){CHECK(!paint);return;}
    CHECK(item->iContextSize>=0 && item->iContextOffset>=0 && item->iTextSize>=0 &&
        item->iContextOffset<=item->iContextSize && item->iTextSize<=item->iContextSize-item->iContextOffset);
    if((size_t)item->iContextSize==shy_expected_bytes){
        CHECK(!memcmp(item->sContext,shy_expected,shy_expected_bytes+1));
    }else{
        CHECK((size_t)item->iContextSize==shy_expected_bytes+1);
        insertion=memchr(item->sContext,'-',(size_t)item->iContextSize);CHECK(insertion);
        prefix=(size_t)(insertion-item->sContext);
        CHECK(!memcmp(item->sContext,shy_expected,prefix) &&
            !memcmp(insertion+1,shy_expected+prefix,shy_expected_bytes-prefix+1));
        shy_hyphens++;
    }
    CHECK(!memcmp(item->sText,item->sContext+item->iContextOffset,(size_t)item->iTextSize));
    if(paint)shy_draws++;else shy_shapes++;
}
static int shy_shape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* out)
{
    shy_check_item(item,0);
    if(shy_expected && shy_fail && item->sContext && (size_t)item->iContextSize==shy_expected_bytes+1){
        shy_fail=0;return XUI_ERROR_OUT_OF_MEMORY;
    }
    return shy_base_shape(proxy,item,out);
}
static int shy_draw(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{shy_check_item(item,1);return shy_base_draw(proxy,dc,item,rect,color,flags);}
static int shy_spans(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{shy_check_item(item,1);return shy_base_spans(proxy,dc,item,rect,color,flags,spans,count);}
static void shy_check_storage(xui_document_renderer renderer,int selected)
{
    size_t b,c,g,owners=0,recipes=0,retained=0,allowance=0;uint64_t copied=0;
    for(b=0;b<renderer->count;b++){
        doc_render_block* block=&renderer->blocks[b];
        for(c=0;c<block->context_count;c++){
            doc_shape_context* context=block->contexts[c];
            CHECK(!context->hyphen_borrows && context->bytes==shy_expected_bytes &&
                !memcmp(context->text,shy_expected,shy_expected_bytes+1));
            allowance+=(size_t)context->bytes+2;
            if(context->hyphen_text){owners++;retained+=(size_t)context->bytes+2;copied+=context->hyphen_copied;}
        }
        for(g=0;g<block->paint_group_count;g++){
            doc_render_paint_group* group=&block->paint_groups[g];
            if(group->has_hyphen_context){recipes++;CHECK(group->context && group->context->hyphen_text &&
                group->hyphen_offset<=group->context->bytes && group->context_bytes==group->context->bytes+1);}
        }
    }
    CHECK(owners==1 && retained<=allowance && (!selected || recipes>1));
    CHECK(copied<=64*(uint64_t)shy_expected_bytes);
    printf("SHY owners=%zu retained=%zu allowance=%zu selected recipes=%zu copied=%llu\n",
        owners,retained,allowance,recipes,(unsigned long long)copied);
}
static void document_shy_context(xui_test_proxy_state_t* proxy)
{
    const size_t repetitions=4096;size_t i;unsigned backend,profile;
    char *source=malloc(repetitions*9+1),*display=malloc(repetitions*7+1);CHECK(source && display);
    for(i=0;i<repetitions;i++){
        memcpy(source+i*9,"AV\xc2\xad" "AV\xce\xbb ",9);
        memcpy(display+i*7,"AVAV\xce\xbb ",7);
    }
    source[repetitions*9]=display[repetitions*7]=0;
    shy_base_shape=proxy->tProxy.textShape;shy_base_draw=proxy->tProxy.drawText;shy_base_spans=proxy->tProxy.drawTextSpans;
    proxy->tProxy.textShape=shy_shape;proxy->tProxy.drawText=shy_draw;
    for(backend=0;backend<2;backend++)for(profile=0;profile<2;profile++){
        xui_context context;xui_font font;xui_surface target;xui_document document;xui_document_snapshot snapshot;
        xui_document_renderer lazy,full;xui_doc_desc_t desc={0};uint64_t paragraph,node;
        xui_doc_position_t start,deep;xui_doc_rect_t first,actual,expected;xui_draw_context dc;
        doc_shape_context* owner;
        proxy->tProxy.drawTextSpans=backend?NULL:shy_spans;
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy->tProxy)==XUI_OK &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy,&font,"shy.ttf",20,0)==XUI_OK &&
            xuiSetDefaultFont(context,font)==XUI_OK && xuiTestSurfaceCreate(proxy,&target,320,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
        desc.iSize=sizeof(desc);desc.iProfile=profile?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
        if(profile){
            CHECK(xuiDocumentLoadMarkdown(document,source,repetitions*9)==XUI_OK &&
                xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&node)==XUI_OK);
        }else{
            xui_document_transaction transaction;xui_doc_node_desc_t child={0};
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            child.iSize=sizeof(child);child.iKind=XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&child,&paragraph)==XUI_OK);
            child.iKind=XUI_DOC_TEXT;child.sText="A";child.iTextBytes=1;
            child.tAttributes.iTextColor=XUI_COLOR_RGBA(180,20,40,255);
            CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&child,&node)==XUI_OK);
            child.sText=source+1;child.iTextBytes=repetitions*9-1;
            child.tAttributes.iTextColor=XUI_COLOR_RGBA(20,40,180,255);
            CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&child,&node)==XUI_OK &&
                xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
        }
        start=decoration_position(document,node,profile?2:1);deep=decoration_position(document,node,profile?30002:30001);
        CHECK(xuiDocumentRendererCreate(context,NULL,&lazy)==XUI_OK && xuiDocumentRendererSetSnapshot(lazy,snapshot,NULL)==XUI_OK &&
            xuiDocumentRendererCreate(context,NULL,&full)==XUI_OK && xuiDocumentRendererSetSnapshot(full,snapshot,NULL)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        shy_expected_bytes=repetitions*7-(profile?1:0);display[shy_expected_bytes]=0;shy_expected=display;
        shy_shapes=shy_draws=shy_hyphens=0;
        CHECK(xuiDocumentRendererLayout(lazy,27,0,320)==XUI_OK && xuiDocumentRendererGetCaretRect(lazy,&start,&first)==XUI_OK);
        shy_check_storage(lazy,1);CHECK(shy_shapes && shy_hyphens);
        owner=lazy->blocks[0].contexts[0];
        CHECK(xuiDocumentRendererLayout(full,27,0,1e9)==XUI_OK &&
            xuiDocumentRendererGetCaretRect(full,&deep,&expected)==XUI_OK);
        shy_check_storage(full,1);xuiDocumentRendererRelease(full);
        shy_fail=1;
        CHECK(xuiDocumentRendererGetCaretRect(lazy,&deep,&actual)==XUI_ERROR_OUT_OF_MEMORY && !shy_fail &&
            xuiDocumentRendererGetCaretRect(lazy,&start,&actual)==XUI_OK &&
            fabs(actual.x-first.x)<.001 && fabs(actual.y-first.y)<.001);
        shy_check_storage(lazy,1);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy,&dc,target)==XUI_OK &&
            xuiDocumentRendererDraw(lazy,dc,0,0,(xui_rect_t){0,0,320,160},NULL,0)==XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy,dc)==XUI_OK && shy_draws);
        CHECK(xuiDocumentRendererGetCaretRect(lazy,&deep,&actual)==XUI_OK &&
            fabs(actual.x-expected.x)<.001 && fabs(actual.y-expected.y)<.001);
        shy_check_storage(lazy,1);
        CHECK(xuiDocumentRendererLayout(lazy,55,0,320)==XUI_OK && xuiDocumentRendererGetCaretRect(lazy,&start,&actual)==XUI_OK);
        CHECK(lazy->blocks[0].contexts[0]==owner);shy_check_storage(lazy,0);
        CHECK(xuiDocumentRendererLayout(lazy,27,0,320)==XUI_OK);shy_check_storage(lazy,1);
        /* The renderer owns its snapshot and can draw after the live document
         * has gone away. Each callback must also release its scratch lease. */
        xuiDocumentRelease(document);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy,&dc,target)==XUI_OK &&
            xuiDocumentRendererDraw(lazy,dc,0,0,(xui_rect_t){0,0,320,160},NULL,0)==XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy,dc)==XUI_OK);
        shy_check_storage(lazy,1);xuiDocumentRendererRelease(lazy);shy_expected=NULL;
        display[repetitions*7-1]=' ';display[repetitions*7]=0;
        proxy->tProxy.surfaceDestroy(&proxy->tProxy,target);xuiDestroy(context);proxy->tProxy.fontDestroy(&proxy->tProxy,font);
    }
    proxy->tProxy.textShape=shy_base_shape;proxy->tProxy.drawText=shy_base_draw;proxy->tProxy.drawTextSpans=shy_base_spans;
    free(source);free(display);
    puts("Document SHY context: Rich/Markdown and spans/draw-only, one paragraph scratch owner, literal callback context/item bytes, full/deep geometry, selected-hyphen shape OOM retry, width reuse and snapshot lifetime passed");
}
