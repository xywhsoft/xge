/* Independent whole-insertion and multi-marker propagation oracle. */
#ifndef TEST_NO_HB
static void document_multiple_shy(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    const char* source="\xce\xbb\xc2\xad\xce\xbc \xce\xbb\xc2\xad\xce\xbc";
    const unsigned offsets[]={0,2,4,6,7,9,11,13},glyphs[]={0,2,4,6};
    const unsigned widths[]={125,160,195,500};
    const unsigned selected_masks[]={3,3,0,0},lines[]={4,4,2,1};
    const char* complete="\xce\xbb-\xce\xbc \xce\xbb-\xce\xbc";
    xui_text_shape_t oracle={0};
    CHECK(original.textShape(p,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,
        .sText=complete,.iTextSize=11,.iFlags=XUI_TEXT_SHAPE_DEFAULT},&oracle)==XUI_OK);
    if(oracle.iClusterCount!=7 || fabs(oracle.fWidth-size*3.05)>=.001){
        fprintf(stderr,"SHY whole variant width=%g expected=%g clusters=%d\n",oracle.fWidth,size*3.05,oracle.iClusterCount);
        for(int i=0;i<oracle.iClusterCount;i++)fprintf(stderr,"cluster %d: %d-%d advance=%g\n",i,oracle.pClusters[i].iTextStart,oracle.pClusters[i].iTextEnd,oracle.pClusters[i].fAdvance);
    }
    CHECK(oracle.iClusterCount==7 && fabs(oracle.fWidth-size*3.05)<.001);
    for(unsigned i=0;i<7;i++){
        /* The fixture also substitutes across the middle space. Shaping each
         * inserted word independently gives a different, incorrect result. */
        const double advances[]={.5,.15,.9,.25,.8,.15,.3};
        CHECK(fabs(oracle.pClusters[i].fAdvance-size*advances[i])<.001);
    }
    xuiTextShapeFree(&oracle);
    for(unsigned form=0;form<5;form++){
        xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;uint64_t leaves[7]={0};
        xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form<2?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN};
        xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={font,font,font,font,font},.iTextColor=UINT32_C(0xff2040c8)};
        CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
        if(form<2){
            xui_document_transaction txn;uint64_t paragraph;
            xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
            CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            for(unsigned i=0;i<(form?7u:1u);i++){
                node.iKind=XUI_DOC_TEXT;node.sText=source+(form?offsets[i]:0);node.iTextBytes=form?offsets[i+1]-offsets[i]:13;
                node.tAttributes.iTextColor=UINT32_C(0xff2040c8)+i*UINT32_C(0x10100);
                CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        }else CHECK(xuiDocumentLoadMarkdown(d,source,13)==XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK &&
            xuiDocumentRendererSetMode(r,form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN)==XUI_OK);
        if(form==4){xui_doc_position_t at=position(snapshot,form,0,0,0,XUI_DOC_AFTER);CHECK(xuiDocumentRendererSetActivePosition(r,&at)==XUI_OK);}
        for(unsigned pass=0;pass<4;pass++){
            xui_draw_context draw;xui_doc_rect_t rects[4][2];unsigned selected=form<3?selected_masks[pass]:0,before;
            CHECK(xuiDocumentRendererLayout(r,size*widths[pass]/100.,0,H)==XUI_OK);
            if(form<3){
                doc_render_block* b=&r->blocks[0];unsigned mask=0;
                CHECK(b->line_count==lines[pass] && !b->partial);
                for(size_t f=0;f<b->fragment_count;f++)if(b->fragments[f].flags & DOC_SOFT_HYPHEN)
                    mask|=(b->fragments[f].flags & DOC_HYPHEN_USED)?(f==1?1u:2u):0;
                CHECK(mask==selected);
                if(selected){
                    CHECK(b->variants && !strcmp(b->variants->text,complete) && b->variants->span_count==1);
                    for(size_t g=0;g<b->paint_group_count;g++)CHECK(b->paint_groups[g].variant==b->variants && b->paint_groups[g].variant_span);
                }
            }
            for(unsigned i=0;i<4;i++){
                unsigned part=glyphs[i],off=offsets[part],local=form==1?0:off;
                xui_doc_position_t a=position(snapshot,form,leaves[form==1?part:0],local,off,XUI_DOC_AFTER);
                xui_doc_position_t z=position(snapshot,form,leaves[form==1?part:0],local+2,off+2,XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(r,&a,&rects[i][0])==XUI_OK && xuiDocumentRendererGetCaretRect(r,&z,&rects[i][1])==XUI_OK &&
                    fabs(fabs(rects[i][1].x-rects[i][0].x)-size*(selected?(i==0?.5:i==1?.9:i==2?.8:.3):(i%2?.9:.8)))<.001);
            }
            before=shapes;raw=cached=0;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(r,draw,.25,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK && paint_route() && shapes==before);
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
            for(unsigned i=0;i<4;i++)literal(p,draw,font,0xe703+(selected?(i==0?4:i==1?3:i==2?2:1):(i%2?3:2)),
                .25+fmin(rects[i][0].x,rects[i][1].x),rects[i][0].y,UINT32_C(0xff2040c8)+(form==1?glyphs[i]:0)*UINT32_C(0x10100));
            for(unsigned i=0;i<2;i++)if(selected & (1u<<i)){
                unsigned part=1+4*i,off=offsets[part],local=form==1?0:off;xui_doc_rect_t rect;
                xui_doc_position_t at=position(snapshot,form,leaves[form==1?part:0],local,off,XUI_DOC_AFTER);
                CHECK(xuiDocumentRendererGetCaretRect(r,&at,&rect)==XUI_OK);
                literal(p,draw,font,0xe702,.25+rect.x,rect.y,UINT32_C(0xff2040c8)+(form==1?part:0)*UINT32_C(0x10100));
            }
            CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets,0,form,9,pass,size);
        }
        xuiDocumentSnapshotRelease(snapshot);xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
    }
}
static void document_long_shy(xui_context context,xui_font font,float size)
{
    const unsigned repeats=5000;size_t bytes=7+(size_t)repeats*7;
    char* text=malloc(bytes+1);xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
    xui_document_transaction txn;uint64_t paragraph,leaf;xui_doc_rect_t caret;
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={font,font,font,font,font}};
    /* Cross the 32 KiB prefix threshold with only one SHY. A late marker may
     * affect earlier glyphs, so this paragraph cannot commit a partial shape. */
    CHECK(text);
    memcpy(text,"\xce\xbb\xc2\xad\xce\xbc ",7);
    for(unsigned i=0;i<repeats;i++)memcpy(text+7+7*i,"\xce\xbb\xce\xbc \xce\xbb",7);
    text[bytes]=0;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK &&
        xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    node.iKind=XUI_DOC_TEXT;node.sText=text;node.iTextBytes=bytes;
    CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK && xuiDocumentTxnCommit(txn,NULL)==XUI_OK);
    xuiDocumentTxnRelease(txn);free(text);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK && xuiDocumentRendererLayout(r,size*1.25,0,80)==XUI_OK &&
        !r->blocks[0].partial && r->blocks[0].variants);
    xui_doc_position_t at=position(snapshot,0,leaf,(unsigned)bytes,(unsigned)bytes,XUI_DOC_BEFORE);
    CHECK(xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && caret.y>80 && r->blocks[0].fragment_count>15000);
    printf("Native SHY long paragraph: %zu source bytes, %zu fragments, complete pattern before publication, deep caret y=%g, size=%g passed\n",
        bytes,r->blocks[0].fragment_count,caret.y,size);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
#endif
