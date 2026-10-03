#include "../xui_document_ui.h"
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=180,H=180 };
#define CHECK(e) do{if(!(e)){fprintf(stderr,"text range %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;
static unsigned pixels,documents;
static int reduced_caps(xui_proxy p,xui_proxy_caps_t* caps)
{int result=original.getCaps(p,caps);caps->iCaps &= ~XUI_PROXY_CAP_TEXT_RANGE;return result;}
#ifndef TEST_NO_HB
static int no_retained_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    int result=original.textShape(p,item,shape);
    if(result==XUI_OK && shape->pPaint){shape->paintFree(shape->pPaint);shape->pPaint=NULL;shape->paintFree=NULL;shape->iPaintBytes=0;}
    return result;
}
#endif
static void equal(xui_proxy p,xui_surface* targets)
{
    unsigned char a[W*H*4],b[W*H*4];unsigned ink=0;
    CHECK(p->surfaceReadRGBA(p,targets[0],a,W*4)==XUI_OK && p->surfaceReadRGBA(p,targets[1],b,W*4)==XUI_OK);
    for(unsigned i=0;i<sizeof(a);i++){
        if(a[i]!=b[i])fprintf(stderr,"range RGBA case=%u byte=%u actual=%u expected=%u\n",pixels,i,a[i],b[i]);
        CHECK(a[i]==b[i]);if(i%4==3 && a[i])ink++;
    }
    CHECK(ink>20);pixels++;
}
static void range_cases(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    const char text[]="\xce\xbb-\xce\xbc \xce\xbd-\xce\xbe";
    const int starts[]={0,3,6,0},ends[]={2,5,9,11};
#ifdef TEST_NO_HB
    const unsigned directions=1;
    (void)size;
#else
    const unsigned directions=2;
#endif
    for(unsigned direction=0;direction<directions;direction++){
        xui_text_item_t full={.iSize=sizeof(full),.pFont=font,.sText=text,.iTextSize=11,
            .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT|(direction?XUI_TEXT_SHAPE_RTL:0)};
        xui_text_shape_t joint={0};CHECK(p->textShape(p,&full,&joint)==XUI_OK && joint.pPaint);
#ifndef TEST_NO_HB
        if(!direction)CHECK(fabsf(joint.pClusters[0].fAdvance-size*1.15f)<.002f);
#endif
        for(unsigned part=0;part<4;part++)for(unsigned borrowed=0;borrowed<2;borrowed++){
            xui_text_item_t item=full;item.iFlags|=XUI_TEXT_SHAPE_RANGE;
            item.iRangeStart=starts[part];item.iRangeEnd=ends[part];item.pShape=borrowed?&joint:NULL;
            item.fDrawOffsetX=borrowed?-.375f:.375f;
            xui_text_shape_t selected={0};xui_vec2_t measured,exact;
            CHECK(xuiTextShape(context,&item,&selected)==XUI_OK && selected.iTextSize==ends[part]-starts[part]);
            CHECK(original.textShapeRangeMeasure(p,&joint,starts[part],ends[part],&exact)==XUI_OK);
            CHECK(fabsf(selected.fWidth-exact.fX)<.002f && p->textMeasure(p,&item,&measured)==XUI_OK && measured.fX==ceilf(exact.fX));
            int n=0;for(int i=0;i<joint.iClusterCount;i++)if(joint.pClusters[i].iTextStart>=starts[part] && joint.pClusters[i].iTextEnd<=ends[part]){
                CHECK(n<selected.iClusterCount && selected.pClusters[n].iTextStart==joint.pClusters[i].iTextStart-starts[part] &&
                    selected.pClusters[n].iTextEnd==joint.pClusters[i].iTextEnd-starts[part] &&
                    selected.pClusters[n].fAdvance==joint.pClusters[i].fAdvance);n++;
            }CHECK(n==selected.iClusterCount && !selected.pPaint);xuiTextShapeFree(&selected);
            for(unsigned method=0;method<5;method++){
                xui_rect_t rect={11.25f,7.5f,140,64};xui_draw_context draw;
                uint32_t color=XUI_COLOR_RGBA(40,100,220,255),flags=XUI_TEXT_CLIP|XUI_TEXT_UNDERLINE|
                    (method==2 || method==4?XUI_TEXT_ALIGN_RIGHT|XUI_TEXT_ALIGN_BOTTOM:0);
                xui_text_paint_span_t local={sizeof(local),0,ends[part]-starts[part],XUI_COLOR_RGBA(220,40,80,255)};
                item.pPaintSpans=method>=3?&local:NULL;item.iPaintSpanCount=method>=3?1:0;
                CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->surfaceClear(p,targets[1],0)==XUI_OK);
                if(method==2 || method==4)CHECK(p->textDraw(p,targets[0],&item,rect,color,flags)==XUI_OK);
                else{CHECK(p->drawBegin(p,&draw,targets[0])==XUI_OK);
                    if(method==1)CHECK(p->drawTextSpans(p,draw,&item,rect,color,flags,&local,1)==XUI_OK);
                    else CHECK(p->drawText(p,draw,&item,rect,color,flags)==XUI_OK);
                    CHECK(p->drawEnd(p,draw)==XUI_OK);}
                CHECK(p->drawBegin(p,&draw,targets[1])==XUI_OK);
                xui_rect_t placed=rect;if(method==2 || method==4){placed.fX+=rect.fW-exact.fX;placed.fY+=rect.fH-exact.fY;}
                /* Reference uses a separately retained full result, exact
                 * global colour offsets and the original caller clip. */
                CHECK(p->drawClipSet(p,draw,rect)==XUI_OK);
                xui_text_paint_span_t global=local;global.iStart+=starts[part];global.iEnd+=starts[part];
                CHECK(original.drawTextShapeRangeSpans(p,draw,&joint,starts[part],ends[part],placed,color,
                    XUI_TEXT_UNDERLINE|(direction?XUI_TEXT_RTL:0),item.fDrawOffsetX,method==1 || method>=3?&global:NULL,method==1 || method>=3?1:0)==XUI_OK);
                CHECK(p->drawClipClear(p,draw)==XUI_OK && p->drawEnd(p,draw)==XUI_OK);equal(p,targets);
            }
        }
        xui_text_item_t empty=full;empty.iFlags|=XUI_TEXT_SHAPE_RANGE;empty.iRangeStart=empty.iRangeEnd=5;
        xui_text_shape_t zero={0};CHECK(p->textShape(p,&empty,&zero)==XUI_OK && !zero.iClusterCount && !zero.fWidth);xuiTextShapeFree(&zero);
        xui_text_item_t bad=empty;bad.iRangeStart=1;bad.iRangeEnd=2;
        xui_vec2_t measured;xui_draw_context draw;
        CHECK(xuiTextShape(context,&bad,&zero)==XUI_ERROR_INVALID_ARGUMENT &&
            p->textShape(p,&bad,&zero)==XUI_ERROR_INVALID_ARGUMENT && p->textMeasure(p,&bad,&measured)==XUI_ERROR_INVALID_ARGUMENT);
        CHECK(p->drawBegin(p,&draw,targets[0])==XUI_OK);
        CHECK(p->drawText(p,draw,&bad,(xui_rect_t){0,0,W,H},~0u,0)==XUI_ERROR_INVALID_ARGUMENT &&
            p->drawTextSpans(p,draw,&bad,(xui_rect_t){0,0,W,H},~0u,0,NULL,0)==XUI_ERROR_INVALID_ARGUMENT &&
            p->textDraw(p,targets[1],&bad,(xui_rect_t){0,0,W,H},~0u,0)==XUI_ERROR_INVALID_ARGUMENT);
        CHECK(p->drawEnd(p,draw)==XUI_OK);
        xui_proxy_t reduced=*p;reduced.getCaps=reduced_caps;xui_context limited;
        CHECK(xuiCreate(&limited)==XUI_OK && xuiSetProxy(limited,&reduced)==XUI_OK);
        CHECK(xuiTextShape(limited,&empty,&zero)==XUI_ERROR_UNSUPPORTED);xuiDestroy(limited);
        reduced=*p;reduced.textShape=NULL;
        CHECK(xuiCreate(&limited)==XUI_OK && xuiSetProxy(limited,&reduced)==XUI_OK);
        CHECK(xuiTextShape(limited,&empty,&zero)==XUI_ERROR_UNSUPPORTED);xuiDestroy(limited);
        xuiTextShapeFree(&joint);
    }
}
#ifndef TEST_NO_HB
static void ligature_cases(xui_proxy p,xui_context context,float size,xui_surface target)
{
    xui_font font;xui_text_shape_t joint={0},selected={0};xui_text_item_t full={.iSize=sizeof(full),
        .sText="xffiy",.iTextSize=5,.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT};
    CHECK(p->fontLoadFile(p,&font,"test/data/xge_opentype_fixture.ttf",size,0)==XUI_OK);full.pFont=font;
    CHECK(p->textShape(p,&full,&joint)==XUI_OK && joint.pPaint);
    for(unsigned borrowed=0;borrowed<2;borrowed++){
        xui_text_item_t item=full;item.iFlags|=XUI_TEXT_SHAPE_RANGE;item.iRangeStart=1;item.iRangeEnd=4;item.pShape=borrowed?&joint:NULL;
        CHECK(xuiTextShape(context,&item,&selected)==XUI_OK && selected.iClusterCount==1 && selected.iTextSize==3 && selected.iCaretCount==2);
        CHECK(selected.pCarets[0].iTextOffset==1 && selected.pCarets[1].iTextOffset==2 &&
            fabsf(selected.pCarets[0].fAdvance-size*.25f)<.002f && fabsf(selected.pCarets[1].fAdvance-size*.8f)<.002f);
        xuiTextShapeFree(&selected);item.iRangeEnd=2;xui_vec2_t measured;xui_draw_context draw;
        CHECK(xuiTextShape(context,&item,&selected)==XUI_ERROR_UNSUPPORTED &&
            p->textMeasure(p,&item,&measured)==XUI_ERROR_UNSUPPORTED);
        CHECK(p->drawBegin(p,&draw,target)==XUI_OK &&
            p->drawText(p,draw,&item,(xui_rect_t){0,0,W,H},~0u,0)==XUI_ERROR_UNSUPPORTED &&
            p->drawTextSpans(p,draw,&item,(xui_rect_t){0,0,W,H},~0u,0,NULL,0)==XUI_ERROR_UNSUPPORTED &&
            p->textDraw(p,target,&item,(xui_rect_t){0,0,W,H},~0u,0)==XUI_ERROR_UNSUPPORTED && p->drawEnd(p,draw)==XUI_OK);
        item.iRangeStart=item.iRangeEnd=2;
        CHECK(p->textShape(p,&item,&selected)==XUI_OK && !selected.iClusterCount && !selected.iCaretCount && !selected.fWidth);
        xuiTextShapeFree(&selected);
    }
    xuiTextShapeFree(&joint);p->fontDestroy(p,font);
    puts("Native whole-item RANGE: rebased GDEF ligature carets, fresh/borrowed interior rejection through all five callbacks and empty interior passed");
}
static void document_cases(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    const char text[]="\xce\xbb\xc2\xad\xce\xbc \xce\xbd\xc2\xad\xce\xbe";
    const unsigned starts[]={0,2,4,7,9,11},ends[]={2,4,6,9,11,13};
    const double ems[]={.4,0,.3,.5,.15,.3},rows[]={0,0,0,1,1,2};
    for(unsigned form=0;form<2;form++){
        xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;uint64_t leaf=0;
        xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH};
        CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
        if(form)CHECK(xuiDocumentLoadMarkdown(d,text,13)==XUI_OK);
        else{
            xui_document_transaction txn;uint64_t paragraph;
            xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
            CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK &&
                xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            node.iKind=XUI_DOC_TEXT;node.sText=text;node.iTextBytes=13;
            CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK && xuiDocumentTxnCommit(txn,NULL)==XUI_OK);
            xuiDocumentTxnRelease(txn);
        }
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,NULL,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK && xuiDocumentRendererLayout(r,size*.7f,0,H)==XUI_OK);
        for(unsigned i=0;i<6;i++){
            xui_doc_position_t at={.iSize=sizeof(at),.iKind=XUI_DOC_POSITION_TEXT,
                .iDocumentId=xuiDocumentSnapshotGetIdentity(snapshot),.iRevision=xuiDocumentSnapshotGetRevision(snapshot),.iNodeId=leaf};
            xui_doc_rect_t a,z;
            if(form){int mapping;CHECK(xuiDocumentSourceToPositionEx(snapshot,starts[i],XUI_DOC_AFTER,&at,&mapping)==XUI_OK);}
            else{at.iOffset=starts[i];at.iAffinity=XUI_DOC_AFTER;}
            CHECK(xuiDocumentRendererGetCaretRect(r,&at,&a)==XUI_OK);
            if(form){int mapping;CHECK(xuiDocumentSourceToPositionEx(snapshot,ends[i],XUI_DOC_BEFORE,&at,&mapping)==XUI_OK);}
            else{at.iOffset=ends[i];at.iAffinity=XUI_DOC_BEFORE;}
            CHECK(xuiDocumentRendererGetCaretRect(r,&at,&z)==XUI_OK);
            CHECK(fabs(fabs(z.x-a.x)-size*ems[i])<.002 && fabs(a.y-size*rows[i])<.002);
        }
        CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->surfaceClear(p,targets[1],0)==XUI_OK);
        xui_draw_context draw;CHECK(p->drawBegin(p,&draw,targets[0])==XUI_OK &&
            xuiDocumentRendererDraw(r,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK);
        CHECK(p->drawBegin(p,&draw,targets[1])==XUI_OK);
        const char* expected[]={"\xce\xbb\xce\xbc","\xce\xbd-","\xce\xbe"};
        for(unsigned row=0;row<3;row++)CHECK(original.drawText(p,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,
            .sText=expected[row],.iTextSize=-1,.iFlags=XUI_TEXT_SHAPE_DEFAULT},(xui_rect_t){0,(int)floor(row*size+.5),W,H},XUI_COLOR_RGBA(32,36,44,255),0)==XUI_OK);
        CHECK(p->drawEnd(p,draw)==XUI_OK);equal(p,targets);documents++;
        xuiDocumentRendererRelease(r);xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(d);
    }
}
#endif
static int frame(void* user)
{
    float size=*(float*)user;original=xuiProxyXge();xui_proxy_t proxy=original;xui_context context;xui_font font;
    xui_surface targets[2];xui_surface_desc_t desc={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    proxy.textShapeRangeMeasure=NULL;proxy.drawTextShapeRangeSpans=NULL;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_shy_future_fixture.ttf",size,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(proxy.surfaceCreate(&proxy,&targets[0],&desc)==XUI_OK && proxy.surfaceCreate(&proxy,&targets[1],&desc)==XUI_OK);
    range_cases(&proxy,context,font,size,targets);
#ifndef TEST_NO_HB
    ligature_cases(&proxy,context,size,targets[0]);
    for(unsigned missing=0;missing<7;missing++){
        proxy=original;if(missing==1 || (missing>=3 && missing<6))proxy.textShapeRangeMeasure=NULL;
        if(missing==2 || (missing>=3 && missing<6))proxy.drawTextShapeRangeSpans=NULL;
        if(missing==4)proxy.drawTextSpans=NULL;
        if(missing>=5)proxy.textShape=no_retained_shape;
        xui_context doc_context;CHECK(xuiCreate(&doc_context)==XUI_OK && xuiSetProxy(doc_context,&proxy)==XUI_OK &&
            xuiSetDefaultFont(doc_context,font)==XUI_OK);
        document_cases(&proxy,doc_context,font,size,targets);xuiDestroy(doc_context);
    }
#endif
    proxy.surfaceDestroy(&proxy,targets[0]);proxy.surfaceDestroy(&proxy,targets[1]);xuiDestroy(context);proxy.fontDestroy(&proxy,font);
    printf("XUI whole-item range: %u exact RGBA cases, %u complete-pattern Document cases at %g passed\n",pixels,documents,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t desc={.iWidth=W,.iHeight=H,.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}return 0;
}
