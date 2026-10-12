/* Real-font geometry and GPU pixels for a ligature carrying an attached mark.
 * Included by both native OpenType and Bidi probes (also DrawText-only). */
static xui_doc_position_t mark_stop(xui_document d,unsigned variant,const uint64_t* ids,
    int rtl,unsigned stop)
{
    static const uint64_t offsets[2][4]={{0,1,4,5},{0,4,6,8}};
    unsigned node=variant ? (stop==3?2:stop) : 0;
    if(variant==1)node=stop==0?0:stop==1?(rtl?2:1):3;
    return pos(d,ids[node],variant?(stop==3?(rtl?2:1):0):offsets[rtl][stop]);
}
static void document_ligature_marks(xui_context ctx,xui_proxy proxy,
    const xui_doc_renderer_desc_t* desc,const xui_surface* targets,float size,int rtl)
{
    const char* full=rtl?"\xd7\x90\xd6\xb0\xd7\x91\xd7\x92":"ff\xcc\x81i";
    const char* md=rtl?"\xd7\x90\xd6\xb0*\xd7\x91*\xd7\x92":"f*f\xcc\x81*i";
    const char* parts[4]={rtl?"\xd7\x90":"f",rtl?"\xd6\xb0":"f",rtl?"\xd7\x91":"\xcc\x81",rtl?"\xd7\x92":"i"};
    const uint32_t colors[]={XUI_COLOR_RGBA(20,40,200,255),XUI_COLOR_RGBA(200,20,40,255),XUI_COLOR_RGBA(20,200,40,255)};
    const double distances[2][4]={{0,.32,.87,.9},{0,.13,.68,.9}};
    xui_document d[3];xui_document_renderer renderers[3];uint64_t ids[3][4]={{0}};
    xui_doc_rect_t stops[3][4];unsigned char* pixels[3];unsigned v,k,pass,colored[3]={0};
    for(v=0;v<3;v++) {
        xui_doc_desc_t profile={0};xui_document_snapshot snapshot;uint64_t paragraph;
        profile.iSize=sizeof(profile);profile.iProfile=XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(v==2?&profile:NULL,&d[v])==XUI_OK);
        if(v==2) {
            CHECK(xuiDocumentLoadMarkdown(d[v],md,strlen(md))==XUI_OK && xuiDocumentAcquireSnapshot(d[v],&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK);
            for(k=0;k<3;k++)CHECK(xuiDocumentSnapshotGetChild(snapshot,paragraph,k,&ids[v][k])==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction;
            CHECK(xuiDocumentBeginTransaction(d[v],NULL,&transaction)==XUI_OK);paragraph=insert(transaction,1,NULL,0);
            if(v==0)ids[v][0]=insert(transaction,paragraph,full,0);
            else for(k=0;k<4;k++)ids[v][k]=insert(transaction,paragraph,parts[k],
                colors[k==0?0:k==1?(rtl?0:1):k==2?1:2]);
            CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
        }
        CHECK(xuiDocumentAcquireSnapshot(d[v],&snapshot)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&renderers[v])==XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderers[v],snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        pixels[v]=malloc(W*H*4);CHECK(pixels[v]);
    }
    for(pass=0;pass<3;pass++)for(v=0;v<3;v++) {
        xui_draw_context draw;
        CHECK(xuiDocumentRendererLayout(renderers[v],pass==1?size*.91+1:W,0,H)==XUI_OK);
        if(pass==1 && !rtl) {
            xui_doc_rect_t extent;int exact;
            CHECK(xuiDocumentRendererGetSize(renderers[v],&extent,&exact)==XUI_OK && exact && extent.width>=size*.99);
        }
        for(k=0;k<4;k++) {
            xui_doc_position_t p=mark_stop(d[v],v,ids[v],rtl,k);
            double expected=(rtl?-1:1)*size*distances[rtl][k];
            CHECK(xuiDocumentRendererGetCaretRect(renderers[v],&p,&stops[v][k])==XUI_OK);
            if(fabs(stops[v][k].x-stops[v][0].x-expected)>.001)
                fprintf(stderr,"Ligature mark caret rtl=%d v=%u stop=%u delta=%.3f expected=%.3f\n",rtl,v,k,
                    stops[v][k].x-stops[v][0].x,expected);
            CHECK(fabs(stops[v][k].x-stops[v][0].x-expected)<.001 && fabs(stops[v][k].y-stops[v][0].y)<.001);
            if(k>0 && k<3) {
                xui_doc_position_t hit;xui_doc_rect_t clicked;
                CHECK(xuiDocumentRendererHitTest(renderers[v],stops[v][k].x+(rtl?.01:-.01),stops[v][k].y+10,&hit)==XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderers[v],&hit,&clicked)==XUI_OK && fabs(clicked.x-stops[v][k].x)<.001);
            }
        }
        {
            xui_doc_range_t range={mark_stop(d[v],v,ids[v],rtl,0),mark_stop(d[v],v,ids[v],rtl,1)};
            xui_doc_rect_t rects[4];uint64_t count;
            CHECK(xuiDocumentRendererGetRangeRects(renderers[v],&range,rects,4,&count)==XUI_OK && count==1 &&
                fabs(rects[0].width-size*distances[rtl][1])<.001);
        }
        CHECK(proxy->surfaceClear(proxy,targets[v],0)==XUI_OK && proxy->drawBegin(proxy,&draw,targets[v])==XUI_OK &&
            xuiDocumentRendererDraw(renderers[v],draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy->drawEnd(proxy,draw)==XUI_OK &&
            proxy->surfaceReadRGBA(proxy,targets[v],pixels[v],W*4)==XUI_OK);
    }
    for(k=0;k<W*H;k++) {
        unsigned char* p=pixels[1]+k*4;unsigned zone;double x=k%W+.5;
        if(p[3]!=pixels[0][k*4+3] || p[3]!=pixels[2][k*4+3]) {
            xui_text_shape_t shape={0};xui_doc_rect_t extent;int exact;
            CHECK(xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=desc->tFonts.normal, .sText=full, .iTextSize=(int)strlen(full), .iFlags=XUI_TEXT_SHAPE_DEFAULT |
                (rtl?XUI_TEXT_SHAPE_RTL:0)}, &shape)==XUI_OK &&
                xuiDocumentRendererGetSize(renderers[0],&extent,&exact)==XUI_OK);
            fprintf(stderr,"Ligature mark shape width=%.3f uniform extent=%.3f\n",shape.fWidth,extent.width);xuiTextShapeFree(&shape);
            fprintf(stderr,"Ligature mark alpha rtl=%d at %u,%u uniform=%u split=%u md=%u\n",rtl,k%W,k/W,
                pixels[0][k*4+3],p[3],pixels[2][k*4+3]);
        }
        CHECK(p[3]==pixels[0][k*4+3] && p[3]==pixels[2][k*4+3]);if(p[3]<128)continue;
        zone=rtl?(x>=stops[1][1].x?0:x>=stops[1][2].x?1:2):(x<stops[1][1].x?0:x<stops[1][2].x?1:2);
        CHECK((zone==0 && p[2]>p[0]+80 && p[2]>p[1]+80) || (zone==1 && p[0]>p[1]+80 && p[0]>p[2]+80) ||
            (zone==2 && p[1]>p[0]+80 && p[1]>p[2]+80));colored[zone]++;
    }
    CHECK(colored[0]>10 && colored[1]>10 && colored[2]>10);
    if(!rtl) {
        /* A viewport showing only GPOS ink past the logical end must paint
         * it too, even though the logical text rectangle is offscreen. */
        xui_draw_context draw;unsigned ink=0;float left=(float)ceil(size*.9+1);
        CHECK(proxy->surfaceClear(proxy,targets[0],0)==XUI_OK && proxy->drawBegin(proxy,&draw,targets[0])==XUI_OK &&
            xuiDocumentRendererDraw(renderers[0],draw,0,0,(xui_rect_t){left,0,W-left,H},NULL,0)==XUI_OK &&
            proxy->drawEnd(proxy,draw)==XUI_OK && proxy->surfaceReadRGBA(proxy,targets[0],pixels[0],W*4)==XUI_OK);
        for(k=0;k<W*H;k++)if(k%W>=(unsigned)left) {
            CHECK(pixels[0][k*4+3]==pixels[1][k*4+3]);ink+=pixels[0][k*4+3]>128;
        }
        CHECK(ink>0);
    }
    CHECK(xuiDocumentLoadMarkdown(d[2],full,strlen(full))==XUI_OK);
    for(v=0;v<2;v++) {
        xui_document_snapshot snapshot;xui_document_renderer raw;xui_doc_position_t p=pos(d[2],1,0);xui_doc_rect_t origin,caret;
        xui_draw_context draw;
        const uint64_t offsets[2][4]={{0,1,4,5},{0,4,6,8}};p.iKind=XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentAcquireSnapshot(d[2],&snapshot)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&raw)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(raw,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentRendererSetMode(raw,v?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_SOURCE_TEXT)==XUI_OK &&
            (!v || xuiDocumentRendererSetActivePosition(raw,&p)==XUI_OK) && xuiDocumentRendererLayout(raw,W,0,H)==XUI_OK &&
            xuiDocumentRendererGetCaretRect(raw,&p,&origin)==XUI_OK);
        for(k=1;k<4;k++) {
            p.iOffset=offsets[rtl][k];CHECK(xuiDocumentRendererGetCaretRect(raw,&p,&caret)==XUI_OK &&
                fabs(caret.x-origin.x-(rtl?-1:1)*size*distances[rtl][k])<.001);
        }
        CHECK(proxy->surfaceClear(proxy,targets[2],0)==XUI_OK && proxy->drawBegin(proxy,&draw,targets[2])==XUI_OK &&
            xuiDocumentRendererDraw(raw,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
            proxy->drawEnd(proxy,draw)==XUI_OK && proxy->surfaceReadRGBA(proxy,targets[2],pixels[2],W*4)==XUI_OK);
        for(k=0;k<W*H;k++)CHECK(pixels[2][k*4+3]==pixels[1][k*4+3]);
        xuiDocumentRendererRelease(raw);
    }
    {
        xui_doc_editor_desc_t editor_desc={0};xui_widget editor;xui_doc_range_t selection;
        editor_desc.iSize=sizeof(editor_desc);editor_desc.tView.iSize=sizeof(editor_desc.tView);
        editor_desc.tView.pDocument=d[0];editor_desc.tView.tRenderer=*desc;
        selection.tAnchor=selection.tCaret=mark_stop(d[0],0,ids[0],rtl,0);
        CHECK(xuiDocumentEditorCreate(ctx,&editor_desc,&editor)==XUI_OK && xuiSetRootWidget(ctx,editor)==XUI_OK &&
            xuiWidgetSetRect(editor,(xui_rect_t){0,0,W,H})==XUI_OK && xuiLayout(ctx)==XUI_OK &&
            xuiSetFocusWidget(ctx,editor)==XUI_OK && xuiDocumentViewSetSelection(editor,&selection)==XUI_OK);
        for(k=1;k<4;k++) {
            xui_doc_position_t expected=mark_stop(d[0],0,ids[0],rtl,k);
            CHECK(xuiInputKeyDown(ctx,rtl?XUI_KEY_LEFT:XUI_KEY_RIGHT,0)==XUI_OK &&
                xuiDocumentViewGetSelection(editor,&selection)==XUI_OK && selection.tCaret.iOffset==expected.iOffset);
        }
        selection.tAnchor=selection.tCaret=mark_stop(d[0],0,ids[0],rtl,rtl?1:2);
        CHECK(xuiDocumentViewSetSelection(editor,&selection)==XUI_OK && xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BACKSPACE)==XUI_OK);
        plain(d[0],rtl?"\xd7\x91\xd7\x92\n":"fi\n");
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(d[0],rtl?"\xd7\x90\xd6\xb0\xd7\x91\xd7\x92\n":"ff\xcc\x81i\n");
        CHECK(xuiSetRootWidget(ctx,NULL)==XUI_OK);xuiWidgetDestroy(editor);
    }
    for(v=0;v<3;v++){free(pixels[v]);xuiDocumentRendererRelease(renderers[v]);xuiDocumentRelease(d[v]);}
    printf("Document ligature+mark %s: GDEF/GPOS carets, cross-node graphemes, Rich/Markdown/Source/Live, GPU alpha/color, reflow and logical deletion/Undo passed\n",rtl?"RTL":"LTR");
}
