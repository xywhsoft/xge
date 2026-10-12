/* Portable shared-result ownership, errors and optional-backend contracts. */
static xui_text_shape_proc shared_base_shape;
static unsigned shared_created,shared_freed,shared_draws;
static int shared_shape_fail,shared_query_fail,shared_query_reject;
static const char* shared_text;
static const int* shared_offsets;
static int shared_bytes;
static int shared_terminal_mode,shared_full_captured;
static unsigned shared_partial_shapes;
static unsigned shared_queries,shared_query_fail_at;
static unsigned shared_variant_queries,shared_variant_fail_at;
static int shared_terminal_fail;
static void shared_free(void* p){shared_freed++;free(p);}
static int shared_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    int result=shared_base_shape(p,item,out);
    if(shared_terminal_mode && shared_full_captured && item->iTextSize==1 && item->sText[0]=='b')shared_partial_shapes++;
    if(result==XUI_OK && item->iTextSize==shared_bytes && !memcmp(item->sText,shared_text,(size_t)shared_bytes)){
        static const float advances[]={32,10,36};
        CHECK(out->iClusterCount==3);
        for(unsigned i=0;i<3;i++)out->pClusters[i].fAdvance=advances[i];
        out->fWidth=78;
        if(item->iFlags & XUI_TEXT_SHAPE_RETAIN_PAINT){
            if(shared_shape_fail){shared_shape_fail=0;xuiTextShapeFree(out);return XUI_ERROR_OUT_OF_MEMORY;}
            out->pPaint=malloc((size_t)shared_bytes+1);CHECK(out->pPaint);memcpy(out->pPaint,shared_text,(size_t)shared_bytes+1);
            out->paintFree=shared_free;out->iPaintBytes=(size_t)shared_bytes+1;shared_created++;
            if(shared_terminal_mode)shared_full_captured=1;
        }
    }
    else if(result==XUI_OK && shared_terminal_mode && item->iTextSize==4 && !memcmp(item->sText,"a -b",4)){
        if(shared_terminal_fail){shared_terminal_fail=0;xuiTextShapeFree(out);return XUI_ERROR_OUT_OF_MEMORY;}
        CHECK(out->iClusterCount==4);out->pClusters[3].fAdvance=36;
        if(shared_terminal_mode==2)out->pClusters[2].fAdvance=0;
        out->fWidth=shared_terminal_mode==2?56:66;
        if(item->iFlags & XUI_TEXT_SHAPE_RETAIN_PAINT){
            out->pPaint=malloc(5);CHECK(out->pPaint);memcpy(out->pPaint,"a -b",5);
            out->paintFree=shared_free;out->iPaintBytes=5;shared_created++;
        }
    }
    return result;
}
static int shared_measure(xui_proxy p,const xui_text_shape_t* shape,int start,int end,xui_vec2_t* size)
{
    static const float advances[]={32,10,36};(void)p;
    int variant=shared_terminal_mode && shape->pPaint && !strcmp(shape->pPaint,"a -b");
    CHECK(shape->pPaint && (variant || !strcmp(shape->pPaint,shared_text)) && start>=0 && end<=(variant?4:shared_bytes) && end>=start);
    memset(size,0,sizeof(*size));
    shared_queries++;
    if(variant){
        shared_variant_queries++;
        if(shared_variant_fail_at && shared_variant_queries==shared_variant_fail_at){shared_variant_fail_at=0;return XUI_ERROR_OUT_OF_MEMORY;}
    }
    if(shared_query_fail_at && shared_queries==shared_query_fail_at){shared_query_fail_at=0;return XUI_ERROR_OUT_OF_MEMORY;}
    if(shared_query_fail){shared_query_fail=0;return XUI_ERROR_OUT_OF_MEMORY;}
    if(shared_query_reject)return XUI_ERROR_UNSUPPORTED;
    if(variant){
        static const float values[]={10,10,10,36};
        for(int i=start;i<end;i++)if(i!=2 || shared_terminal_mode!=2)size->fX+=values[i];
    }else {int start_boundary=0,end_boundary=0;
        for(int i=0;i<4;i++){start_boundary+=start==shared_offsets[i];end_boundary+=end==shared_offsets[i];}
        CHECK(start_boundary==1 && end_boundary==1);
        for(int i=0;i<3;i++)if(shared_offsets[i]>=start && shared_offsets[i+1]<=end)size->fX+=advances[i];
    }
    size->fY=shape->fLineHeight;return XUI_OK;
}
static int shared_draw(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    (void)p;(void)d;(void)rect;(void)color;(void)flags;
    CHECK(shape->pPaint && (!strcmp(shape->pPaint,shared_text) || (shared_terminal_mode && !strcmp(shape->pPaint,"a -b"))) && start>=0 && end<=(shared_terminal_mode?4:shared_bytes) && end>start &&
        offset>=-.5f && offset<=.5f);
    for(int i=0;i<count;i++)CHECK(spans[i].iStart>=start && spans[i].iEnd<=end);
    shared_draws++;return XUI_OK;
}
static void document_shared_span_sample(xui_test_proxy_state_t* state,const char* text,const int* offsets,
    const char* source,const int* source_offsets,unsigned parts,unsigned narrow_groups)
{
    xui_proxy_t p=state->tProxy;xui_context c;xui_font font;xui_surface surface;
    xui_document d;xui_document_transaction txn;xui_document_snapshot snapshot;xui_document_renderer r;
    xui_doc_node_desc_t node={0};uint64_t paragraph,leaf;void* key;
    shared_text=text;shared_offsets=offsets;shared_bytes=offsets[3];
    shared_created=shared_freed=shared_draws=0;
    shared_base_shape=p.textShape;p.textShape=shared_shape;
    if((unsigned char)text[0]==0xd7)p.drawTextSpans=NULL;
    p.textShapeRangeMeasure=shared_measure;p.drawTextShapeRangeSpans=shared_draw;
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK &&
        p.fontLoadFile(&p,&font,"shared-result.ttf",20,0)==XUI_OK && xuiSetDefaultFont(c,font)==XUI_OK &&
        xuiTestSurfaceCreate(state,&surface,160,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
    node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    for(unsigned i=0;i<parts;i++){
        node.iKind=XUI_DOC_TEXT;node.sText=source+source_offsets[i];node.iTextBytes=(uint64_t)(source_offsets[i+1]-source_offsets[i]);node.tAttributes.iTextColor=UINT32_C(0xff314159)+i;
        CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(c,NULL,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
    shared_shape_fail=1;
    CHECK(xuiDocumentRendererLayout(r,45,0,160)==XUI_ERROR_OUT_OF_MEMORY && shared_created==shared_freed);
    shared_query_fail=1;
    CHECK(xuiDocumentRendererLayout(r,45,0,160)==XUI_ERROR_OUT_OF_MEMORY && shared_created==shared_freed);
    {int result=xuiDocumentRendererLayout(r,45,0,160);
    if(result!=XUI_OK || r->blocks[0].paint_seed_count!=1 || r->blocks[0].paint_group_count!=narrow_groups ||
        r->blocks[0].paint_seeds[0].shape.iPaintBytes!=(size_t)shared_bytes+1)
        fprintf(stderr,"Shared source %s: result=%d seeds=%zu groups=%zu bytes=%zu cache=%zu\n",source,result,r->blocks[0].paint_seed_count,r->blocks[0].paint_group_count,
            r->blocks[0].paint_seed_count?r->blocks[0].paint_seeds[0].shape.iPaintBytes:0,r->blocks[0].cache_bytes);
    CHECK(result==XUI_OK && r->blocks[0].paint_seed_count==1 &&
        r->blocks[0].paint_group_count==narrow_groups && r->blocks[0].paint_seeds[0].shape.iPaintBytes==(size_t)shared_bytes+1 &&
        r->blocks[0].cache_bytes>=(size_t)shared_bytes+1+4*sizeof(uint32_t)+3*sizeof(double));
    }
    key=r->blocks[0].paint_seeds[0].shape.pPaint;xuiDocumentRelease(d);
    if(parts>3){
        const doc_render_paint_seed* seed=&r->blocks[0].paint_seeds[0];
        CHECK(r->blocks[0].fragment_count==parts && seed->offsets[0]==0 && seed->offsets[1]==(uint32_t)offsets[1] &&
            seed->offsets[2]==(uint32_t)offsets[1] && seed->offsets[3]==(uint32_t)offsets[2] && seed->offsets[4]==(uint32_t)offsets[3] && seed->offsets[5]==(uint32_t)offsets[3]);
        CHECK(r->blocks[0].fragments[1].width==0 && r->blocks[0].fragments[4].width==0);
    }
    for(unsigned pass=0;pass<3;pass++){
        xui_draw_context draw;
        CHECK(xuiDocumentRendererLayout(r,pass==1?100:45,0,160)==XUI_OK &&
            r->blocks[0].paint_seeds[0].shape.pPaint==key &&
            fabs(r->blocks[0].fragments[0].width-32)<.001 &&
            p.drawBegin(&p,&draw,surface)==XUI_OK && xuiDocumentRendererDraw(r,draw,.25,0,
                (xui_rect_t){0,0,160,160},NULL,0)==XUI_OK && p.drawEnd(&p,draw)==XUI_OK);
    }
    CHECK(shared_draws==2*narrow_groups+1);
    shared_query_reject=1;
    CHECK(xuiDocumentRendererLayout(r,30,0,160)==XUI_OK);
    for(size_t i=0;i<r->blocks[0].paint_group_count;i++)CHECK(!r->blocks[0].paint_groups[i].shared_seed);
    shared_query_reject=0;
    xuiDocumentRendererRelease(r);CHECK(shared_created==shared_freed);
    p.surfaceDestroy(&p,surface);xuiDestroy(c);p.fontDestroy(&p,font);
}
static void document_shared_span_contract(xui_test_proxy_state_t* state)
{
    static const int ascii_offsets[]={0,1,2,3},unicode_offsets[]={0,2,3,5};
    static const int projected_ascii[]={0,1,4,5,6,9},projected_unicode[]={0,2,5,6,8,11};
    static const int shy_ascii[]={0,1,3,4,5,8},shy_unicode[]={0,2,4,5,7,10};
    document_shared_span_sample(state,"a b",ascii_offsets,"a b",ascii_offsets,3,2);
    document_shared_span_sample(state,"\xce\xbb \xce\xbc",unicode_offsets,"\xce\xbb \xce\xbc",unicode_offsets,3,2);
    document_shared_span_sample(state,"\xd7\x90 \xd7\x91",unicode_offsets,"\xd7\x90 \xd7\x91",unicode_offsets,3,2);
    document_shared_span_sample(state,"a b",ascii_offsets,"a\xe2\x80\x8b b\xef\xbb\xbf",projected_ascii,5,2);
    document_shared_span_sample(state,"\xce\xbb \xce\xbc",unicode_offsets,"\xce\xbb\xe2\x80\x8b \xce\xbc\xef\xbb\xbf",projected_unicode,5,2);
    document_shared_span_sample(state,"a b",ascii_offsets,"a\xc2\xad b\xef\xbb\xbf",shy_ascii,5,2);
    document_shared_span_sample(state,"\xce\xbb \xce\xbc",unicode_offsets,"\xce\xbb\xc2\xad \xce\xbc\xef\xbb\xbf",shy_unicode,5,2);
    /* Unicode 17 LB21a excludes BA: the retained space now splits two rows. */
    document_shared_span_sample(state,"\xd7\x90 \xd7\x91",unicode_offsets,"\xd7\x90\xc2\xad \xd7\x91\xef\xbb\xbf",shy_unicode,5,2);
    puts("Document shared span contract: ASCII/Greek/RTL Hebrew and deleted ZWSP/BOM/SHY UTF-8 joint owner, shape/query failure cleanup and retries, duplicate displayed offsets/zero-width controls, scalar byte range rebasing, fractional X, width cache reuse, released Document and unsupported query fallback passed");
}
static void document_shared_shy_variant_sample(xui_test_proxy_state_t* state,int zero)
{
    xui_proxy_t p=state->tProxy;xui_context c;xui_font font;xui_surface surface;
    xui_document d;xui_document_transaction txn;xui_document_snapshot snapshot;xui_document_renderer r;
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};uint64_t paragraph,leaves[4];
    const char* parts[]={"a"," ","\xc2\xad","b"};static const int offsets[]={0,1,2,3};
    shared_text="a b";shared_offsets=offsets;shared_bytes=3;shared_created=shared_freed=shared_draws=shared_partial_shapes=0;
    shared_terminal_mode=zero?2:1;shared_full_captured=0;shared_base_shape=p.textShape;
    p.textShape=shared_shape;p.textShapeRangeMeasure=shared_measure;p.drawTextShapeRangeSpans=shared_draw;
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK && p.fontLoadFile(&p,&font,"shy-cycle.ttf",20,0)==XUI_OK && xuiSetDefaultFont(c,font)==XUI_OK && xuiTestSurfaceCreate(state,&surface,160,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    for(unsigned i=0;i<4;i++){
        node.iKind=XUI_DOC_TEXT;node.sText=parts[i];node.iTextBytes=strlen(parts[i]);node.tAttributes.iTextColor=UINT32_C(0xff314159)+i;
        CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(c,NULL,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);void* key=NULL;
    shared_terminal_fail=1;
    CHECK(xuiDocumentRendererLayout(r,65,0,160)==XUI_ERROR_OUT_OF_MEMORY && shared_created==shared_freed);
    shared_full_captured=0;shared_variant_queries=0;shared_variant_fail_at=3;
    CHECK(xuiDocumentRendererLayout(r,65,0,160)==XUI_ERROR_OUT_OF_MEMORY && shared_variant_queries==3 && shared_created==shared_freed);
    shared_full_captured=0;
    for(unsigned pass=0;pass<3;pass++){
        xui_draw_context draw;xui_doc_position_t at={.iSize=sizeof(at),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=xuiDocumentGetIdentity(d),.iRevision=xuiDocumentGetRevision(d),.iNodeId=leaves[3],.iOffset=1,.iAffinity=XUI_DOC_BEFORE};
        xui_doc_rect_t caret;shared_partial_shapes=0;
        CHECK(xuiDocumentRendererLayout(r,pass==1?80:65,0,160)==XUI_OK && r->blocks[0].paint_seed_count==1 && shared_full_captured && shared_partial_shapes==0);
        if(!key)key=r->blocks[0].paint_seeds[0].shape.pPaint;else CHECK(key==r->blocks[0].paint_seeds[0].shape.pPaint);
        CHECK(xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && fabs(caret.x-(pass==1?78:36))<.001 && fabs(caret.y-(pass==1?0:20))<.001);
        CHECK(r->blocks[0].paint_group_count==(pass==1?1u:2u));
        if(pass!=1)CHECK(r->blocks[0].paint_groups[0].variant_span && r->blocks[0].paint_groups[1].variant_span &&
            r->blocks[0].paint_groups[0].variant==r->blocks[0].paint_groups[1].variant && r->blocks[0].fragments[2].width==(zero?0:10));
        CHECK(p.drawBegin(&p,&draw,surface)==XUI_OK && xuiDocumentRendererDraw(r,draw,.25,0,(xui_rect_t){0,0,160,160},NULL,0)==XUI_OK && p.drawEnd(&p,draw)==XUI_OK);
    }
    xuiDocumentRelease(d);xuiDocumentRendererRelease(r);CHECK(shared_created==shared_freed);
    p.surfaceDestroy(&p,surface);xuiDestroy(c);p.fontDestroy(&p,font);shared_terminal_mode=shared_full_captured=0;
}
static void document_shared_shy_convergence(xui_test_proxy_state_t* state)
{
    document_shared_shy_variant_sample(state,0);document_shared_shy_variant_sample(state,1);
    puts("Document shared SHY convergence: each alternative has its own full insertion pattern, terminal shape/final-query errors propagate and retry cleanly, selected and suffix rows share complete variant glyphs, exact zero-advance marker, suffix caret 36/20, width reuse and complete owner cleanup passed");
}
