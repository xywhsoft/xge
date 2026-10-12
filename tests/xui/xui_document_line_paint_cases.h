/* Portable final-row ownership/error contract; independent of Native pixels. */
typedef struct line_paint_data {int bytes;char text[32];} line_paint_data;
static xui_text_shape_proc line_base_shape;
static xui_draw_text_spans_proc line_base_spans;
static unsigned line_created,line_freed,line_cached,line_raw,line_shapes;
static int line_fail_after,line_reject,line_draw_fail;
static const line_paint_data* line_expected_raw;
static void line_free(void* p){line_freed++;free(p);}
static int line_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    int result;line_shapes++;result=line_base_shape(p,item,out);
    if(result==XUI_OK && (item->iFlags & XUI_TEXT_SHAPE_RETAIN_PAINT)){
        line_paint_data* data;
        if(line_fail_after && !--line_fail_after){xuiTextShapeFree(out);return XUI_ERROR_OUT_OF_MEMORY;}
        CHECK(item->iTextSize>0 && item->iTextSize<(int)sizeof(data->text));
        data=calloc(1,sizeof(*data));CHECK(data);data->bytes=item->iTextSize;
        memcpy(data->text,item->sText,(size_t)data->bytes);
        out->pPaint=data;out->paintFree=line_free;out->iPaintBytes=sizeof(*data);line_created++;
    }
    return result;
}
static int line_draw(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,
    xui_rect_t rect,uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{
    line_paint_data* data=shape->pPaint;
    (void)p;(void)d;(void)rect;(void)color;(void)flags;(void)offset;
    CHECK(data && shape->paintFree==line_free && start==0 && end==data->bytes && end==shape->iTextSize);
    for(int i=0;i<count;i++)CHECK(spans[i].iStart>=0 && spans[i].iEnd<=end);
    if(line_draw_fail)return XUI_ERROR_OUT_OF_MEMORY;
    if(line_reject){line_expected_raw=data;return XUI_ERROR_UNSUPPORTED;}
    line_cached++;return XUI_OK;
}
static int line_raw_draw(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,
    uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{
    CHECK(line_expected_raw && item->iTextSize==line_expected_raw->bytes &&
        !memcmp(item->sText,line_expected_raw->text,(size_t)item->iTextSize));
    line_expected_raw=NULL;line_raw++;return line_base_spans(p,d,item,rect,color,flags,spans,count);
}
static void document_line_paint_contract(xui_test_proxy_state_t* state)
{
    xui_proxy_t p=state->tProxy;xui_context c;xui_font font;xui_surface surface;
    xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
    xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=XUI_DOCUMENT_MARKDOWN};
    const char* text="\xce\xbb \xce\xbb \xce\xbb";
    line_base_shape=p.textShape;line_base_spans=p.drawTextSpans;
    p.textShape=line_shape;p.drawTextSpans=line_raw_draw;p.drawTextShapeRangeSpans=line_draw;
    line_created=line_freed=line_cached=line_raw=line_shapes=0;line_reject=line_draw_fail=0;line_expected_raw=NULL;
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK &&
        p.fontLoadFile(&p,&font,"final-row.ttf",20,0)==XUI_OK && xuiSetDefaultFont(c,font)==XUI_OK &&
        xuiTestSurfaceCreate(state,&surface,160,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK &&
        xuiDocumentCreate(&profile,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,text,strlen(text))==XUI_OK &&
        xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(c,NULL,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
    line_fail_after=2;
    CHECK(xuiDocumentRendererLayout(r,15,0,160)==XUI_ERROR_OUT_OF_MEMORY && line_created>0 && line_created==line_freed);
    CHECK(xuiDocumentRendererLayout(r,15,0,160)==XUI_OK && r->blocks[0].paint_group_count>=3);
    for(size_t i=0;i<r->blocks[0].paint_group_count;i++)CHECK(r->blocks[0].paint_groups[i].shape.pPaint);
    CHECK(r->blocks[0].cache_bytes>=3*sizeof(line_paint_data));xuiDocumentRelease(d);
    for(unsigned pass=0;pass<4;pass++){
        xui_draw_context draw;unsigned shaped,raw=line_raw,cached=line_cached;int result;
        line_reject=pass==1;line_draw_fail=pass==2;
        CHECK(xuiDocumentRendererLayout(r,pass==3?100:15,0,160)==XUI_OK);shaped=line_shapes;
        CHECK(p.drawBegin(&p,&draw,surface)==XUI_OK);
        result=xuiDocumentRendererDraw(r,draw,.25,0,(xui_rect_t){0,0,160,160},NULL,0);
        CHECK(p.drawEnd(&p,draw)==XUI_OK && line_shapes==shaped && !line_expected_raw);
        if(line_draw_fail)CHECK(result==XUI_ERROR_OUT_OF_MEMORY && line_raw==raw && line_cached==cached);
        else if(line_reject)CHECK(result==XUI_OK && line_raw>raw && line_cached==cached);
        else CHECK(result==XUI_OK && line_raw==raw && line_cached>cached);
    }
    xuiDocumentRendererRelease(r);CHECK(line_created==line_freed);
    p.surfaceDestroy(&p,surface);xuiDestroy(c);p.fontDestroy(&p,font);
    puts("Document final-row contract: Unicode owned input, partial OOM cleanup/retry, cache accounting, width reflow, released Document, unsupported-before-paint fallback and real draw error propagation passed");
}
