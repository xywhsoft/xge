/* Independent RTL subset origins and UBA L1 geometry, using literal glyphs. */
static void rtl_range_cases(xui_proxy p,xui_font reference,float size,xui_surface* targets)
{
    static const int ranges[][2]={{0,2},{0,3},{3,5},{2,5},{0,5}};
    static const float shifts[]={0,-.25f,.25f};
    const double advances[]={size*.8,size*.25,size*.9};
    xui_font font;xui_text_shape_t shape={0};char input[6];
    xui_text_paint_span_t spans[]={{sizeof(*spans),0,2,UINT32_C(0xff2040c8)},
        {sizeof(*spans),2,3,UINT32_C(0xffc82040)},{sizeof(*spans),3,5,UINT32_C(0xff20c840)}};
    memcpy(input,SPAN_BODY,sizeof(input));
    CHECK(p->fontLoadFile(p,&font,SPAN_FONT,size,0)==XUI_OK && original.textShape(p,
        &(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=input,.iTextSize=5,
        .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL|XUI_TEXT_SHAPE_RETAIN_PAINT},&shape)==XUI_OK && shape.pPaint);
    memset(input,0,sizeof(input));p->fontDestroy(p,font);
    for(unsigned range=0;range<5;range++)for(unsigned pass=0;pass<3;pass++){
        int start=ranges[range][0],end=ranges[range][1];double width=0,x[3]={0};
        xui_vec2_t measured={0};xui_draw_context draw;unsigned before=shapes;
        xui_rect_t clip={11,0,W-11,H};
        for(int i=2;i>=0;i--)if((int)span_offsets[i]>=start && (int)span_offsets[i+1]<=end){x[i]=width;width+=advances[i];}
        CHECK(original.textShapeRangeMeasure(p,&shape,start,end,&measured)==XUI_OK &&
            fabs(measured.fX-width)<.001 && measured.fY>0);
        CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
            p->drawClipSet(p,draw,clip)==XUI_OK && original.drawTextShapeRangeSpans(p,draw,&shape,start,end,
                (xui_rect_t){7,0,W-7,H},~0u,XUI_TEXT_CLIP,shifts[pass],spans,3)==XUI_OK &&
            p->drawEnd(p,draw)==XUI_OK && shapes==before);
        CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK && p->drawClipSet(p,draw,clip)==XUI_OK);
        for(unsigned i=0;i<3;i++)if(i!=1 && (int)span_offsets[i]>=start && (int)span_offsets[i+1]<=end)
            literal(p,draw,reference,i?SPAN_PUA_CONTEXT_LAST:SPAN_PUA_CONTEXT_FIRST,7+shifts[pass]+x[i],0,spans[i].iColor);
        CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets[0],targets[1]);
    }
    {xui_vec2_t empty={1,1};CHECK(original.textShapeRangeMeasure(p,&shape,2,2,&empty)==XUI_OK && empty.fX==0 && empty.fY>0);}
    xuiTextShapeFree(&shape);
}
static void rtl_mixed_cases(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    static const char text[]="i \xd7\x90 \xd7\x91 i";
    static const unsigned offsets[]={0,1,2,4,5,7,8,9},visual[]={0,1,4,3,2,5,6};
    const double advances[]={size*.6,size*.25,size*.8,size*.25,size*.9,size*.25,size*.6};
    for(unsigned form=0;form<3;form++){
        xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
        xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form==2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH};
        xui_doc_renderer_desc_t desc={0};uint32_t colors[7];void* key=NULL;
        for(unsigned i=0;i<7;i++)colors[i]=form==1?(i%2?UINT32_C(0xff2040c8):UINT32_C(0xffc82040)):UINT32_C(0xff314159);
        CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
        if(form==2)CHECK(xuiDocumentLoadMarkdown(d,text,sizeof(text)-1)==XUI_OK);
        else{
            xui_document_transaction txn;xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};uint64_t paragraph,leaf;
            CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK &&
                xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            for(unsigned i=0;i<(form?7u:1u);i++){
                node.iKind=XUI_DOC_TEXT;node.sText=text+offsets[i];node.iTextBytes=form?offsets[i+1]-offsets[i]:sizeof(text)-1;
                node.tAttributes.iTextColor=colors[i];CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        }
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};desc.iTextColor=colors[0];
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        for(unsigned pass=0;pass<3;pass++){
            double x[7]={0},pen=0,ox=.25;unsigned before;
            xui_draw_context draw;xui_rect_t clip={5,0,W-5,H};doc_render_block* block;
            /* Native advances are floats. Leave a small interior margin while
             * still excluding the next character, rather than asserting an
             * exact binary-float wrap threshold at the 37 px test size. */
            CHECK(xuiDocumentRendererLayout(r,pass==1?size*1.91:W,0,H)==XUI_OK);block=&r->blocks[0];
            CHECK(block->fragment_count==7);
            for(unsigned i=0;i<7;i++){
                unsigned index=pass==1?i:visual[i];if(pass==1 && i==4)pen=0;x[index]=pen;pen+=advances[index];
            }
            for(unsigned i=0;i<7;i++){
                if(fabs(block->fragments[i].width-advances[i])>=.001 || fabs(block->fragments[i].x-x[i])>=.001 ||
                    !(pass==1 && i>=4?block->fragments[i].y>0:fabs(block->fragments[i].y)<.001))
                    fprintf(stderr,"RTL mixed form=%u pass=%u size=%g fragment=%u width=%g/%g x=%g/%g y=%g level=%u cases=%u\n",
                        form,pass,size,i,block->fragments[i].width,advances[i],block->fragments[i].x,x[i],block->fragments[i].y,block->fragments[i].bidi_level,cases);
                CHECK(fabs(block->fragments[i].width-advances[i])<.001 && fabs(block->fragments[i].x-x[i])<.001 &&
                    (pass==1 && i>=4?block->fragments[i].y>0:fabs(block->fragments[i].y)<.001));
            }
            {const doc_render_paint_seed* seed=NULL;
                for(size_t i=0;i<block->paint_seed_count;i++)if(block->paint_seeds[i].first_fragment==2 && block->paint_seeds[i].end_fragment==5)seed=&block->paint_seeds[i];
                CHECK(seed && seed->bidi_level==1 && seed->shape.pPaint);if(!key)key=seed->shape.pPaint;else CHECK(key==seed->shape.pPaint);
            }
            if(pass==1){int reset=0;
                CHECK(block->fragments[3].bidi_level==0);
                for(size_t i=0;i<block->paint_group_count;i++)if(block->paint_groups[i].first_fragment==3){
                    CHECK(block->paint_groups[i].end_fragment==4 && block->paint_groups[i].bidi_level==0 && !block->paint_groups[i].shared_seed);reset++;
                }CHECK(reset==1);
            }
            if(pass==2)xuiDocumentRelease(d);
            before=shapes;raw=retained=0;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK && raw==0 && shapes==before && retained>0);
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK && p->drawClipSet(p,draw,clip)==XUI_OK);
            for(unsigned i=0;i<7;i++)if(i==0 || i==2 || i==4 || i==6)
                literal(p,draw,font,i==2?SPAN_PUA_CONTEXT_FIRST:i==4?SPAN_PUA_CONTEXT_LAST:"\xee\x98\x89",ox+x[i],block->fragments[i].y,colors[i]);
            CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets[0],targets[1]);
        }xuiDocumentRendererRelease(r);
    }
}
