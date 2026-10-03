#include "../xge.h"
#include "../xui.h"
#include "../xui_document_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"retained paint OOM line %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static bool invalid_free_event(const xmemdebugevent* event,ptr ignored)
{
    (void)ignored;
    if(event->Kind==XMEMDEBUG_INVALID_FREE)fprintf(stderr,"invalid free %s:%u address=%p\n",event->File,event->Line,event->Address);
    return true;
}
static void same_live(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    if(after.LiveCount!=before->LiveCount || after.LiveBytes!=before->LiveBytes ||
        after.InvalidFreeCount!=before->InvalidFreeCount || after.DoubleFreeCount!=before->DoubleFreeCount)
        {fprintf(stderr,"live before=%zu/%zu invalid=%zu double=%zu after=%zu/%zu invalid=%zu double=%zu\n",
            before->LiveCount,before->LiveBytes,before->InvalidFreeCount,before->DoubleFreeCount,
            after.LiveCount,after.LiveBytes,after.InvalidFreeCount,after.DoubleFreeCount);
        xrtMemDebugVisit(invalid_free_event,NULL);}
    CHECK(after.LiveCount==before->LiveCount && after.LiveBytes==before->LiveBytes &&
        after.InvalidFreeCount==before->InvalidFreeCount && after.DoubleFreeCount==before->DoubleFreeCount);
}
static void context_storage(xui_proxy p,xui_font font)
{
    const size_t lengths[]={4096,1024*1024};size_t retained_bytes=0;unsigned context_failed=0;
    for(unsigned pass=0;pass<2;pass++){
        size_t bytes=lengths[pass],offset=bytes/2;char* context=malloc(bytes);
        xui_text_item_t item={0};xui_text_shape_t shape={0};xmemdebugsnapshot before,after;
        xui_vec2_t measured;CHECK(context);memset(context,'a',bytes);
        context[0]=(char)0xce;context[1]=(char)0xbb;memcpy(context+offset,"fi",2);
        item.iSize=sizeof(item);item.pFont=font;item.sText=context+offset;item.iTextSize=2;
        item.sContext=context;item.iContextSize=(int)bytes;item.iContextOffset=(int)offset;
        item.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT;
        CHECK(p->textShape(p,&item,&shape)==XUI_OK && shape.pPaint);xuiTextShapeFree(&shape);
        if(!pass){
            unsigned attempt;
            for(attempt=0;attempt<64;attempt++){
                int result;bool fired;
                xrtClearError();xrtMemDebugSnapshot(&before);CHECK(xrtMemDebugFailAfter(attempt));
                result=p->textShape(p,&item,&shape);fired=xrtMemDebugFailTriggered();
                xrtMemDebugFailClear();xrtClearError();
                if(fired){
                    CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !shape.pPaint);
                    xuiTextShapeFree(&shape);same_live(&before);context_failed++;
                    CHECK(p->textShape(p,&item,&shape)==XUI_OK && shape.pPaint);
                    xuiTextShapeFree(&shape);same_live(&before);
                }else{
                    CHECK(result==XUI_OK && shape.pPaint);xuiTextShapeFree(&shape);same_live(&before);break;
                }
            }
            CHECK(attempt<64 && context_failed>=5);
        }
        xrtClearError();xrtMemDebugSnapshot(&before);
        CHECK(p->textShape(p,&item,&shape)==XUI_OK && shape.pPaint);
        xrtMemDebugSnapshot(&after);
        CHECK(after.LiveBytes-before.LiveBytes==shape.iPaintBytes+
            (size_t)shape.iClusterCount*sizeof(*shape.pClusters)+(size_t)shape.iCaretCount*sizeof(*shape.pCarets));
        if(!pass)retained_bytes=shape.iPaintBytes;
        CHECK(shape.iPaintBytes==retained_bytes && retained_bytes<4096);
        memset(context,0,bytes);free(context);
        CHECK(p->textShapeRangeMeasure(p,&shape,0,2,&measured)==XUI_OK && measured.fX==shape.fWidth && measured.fY>0);
        xuiTextShapeFree(&shape);same_live(&before);
    }
    printf("Native final-row storage: %u context allocation failures/retries; 4 KiB/1 MiB borrowed Unicode contexts retain the same %zu bytes, exact live accounting and query after source release passed\n",context_failed,retained_bytes);
}
static int paint_failure_frame(void* ignored)
{
    enum{W=120,H=80};
    xui_proxy_t p=xuiProxyXge();xui_font font;xui_text_shape_t shape={0};
    xui_surface targets[2];xui_surface_desc_t desc={0};xui_draw_context draw;
    xui_text_paint_span_t spans[20];xmemdebugsnapshot before;
    unsigned char actual[W*H*4],expected[W*H*4];xge_rect_t saved,after;int result;
    (void)ignored;
    CHECK(p.fontLoadFile(&p,&font,"test/data/xge_ascii_context_fixture.ttf",20,0)==XUI_OK);
    CHECK(p.textShape(&p,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,
        .sText="aaaaaaaaaaaaaaaaaaaa",.iTextSize=20,.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT},&shape)==XUI_OK);
    for(int i=0;i<20;i++)spans[i]=(xui_text_paint_span_t){sizeof(*spans),i,i+1,i%2?UINT32_C(0xff2040c8):UINT32_C(0xffc82040)};
    desc.iWidth=W;desc.iHeight=H;desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(p.surfaceCreate(&p,&targets[0],&desc)==XUI_OK && p.surfaceCreate(&p,&targets[1],&desc)==XUI_OK);
    /* Warm atlas/GPU storage before taking a live-allocation baseline. */
    CHECK(p.surfaceClear(&p,targets[1],0)==XUI_OK && p.drawBegin(&p,&draw,targets[1])==XUI_OK &&
        p.drawClipSet(&p,draw,(xui_rect_t){7,6,70,50})==XUI_OK &&
        p.drawTextShapeRangeSpans(&p,draw,&shape,0,20,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,.25f,spans,20)==XUI_OK &&
        p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,targets[1],expected,W*4)==XUI_OK);
    {unsigned ink=0;for(size_t i=3;i<sizeof(expected);i+=4)ink+=expected[i]!=0;CHECK(ink>20);}
    CHECK(p.surfaceClear(&p,targets[0],0)==XUI_OK && p.drawBegin(&p,&draw,targets[0])==XUI_OK &&
        p.drawClipSet(&p,draw,(xui_rect_t){7,6,70,50})==XUI_OK);
    saved=xgeClipGet();xrtClearError();xrtMemDebugSnapshot(&before);CHECK(xrtMemDebugFailAfter(0));
    result=p.drawTextShapeRangeSpans(&p,draw,&shape,0,20,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,.25f,spans,20);
    CHECK(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();after=xgeClipGet();
    CHECK(result==XUI_ERROR_OUT_OF_MEMORY && saved.fX==after.fX && saved.fY==after.fY &&
        saved.fW==after.fW && saved.fH==after.fH);same_live(&before);
    spans[1].iStart=0;
    CHECK(p.drawTextShapeRangeSpans(&p,draw,&shape,0,20,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,.25f,spans,20)==XUI_ERROR_INVALID_ARGUMENT);
    CHECK(p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],actual,W*4)==XUI_OK);
    for(size_t i=0;i<sizeof(actual);i++)CHECK(actual[i]==0);
    spans[1].iStart=1;
    CHECK(p.drawBegin(&p,&draw,targets[0])==XUI_OK && p.drawClipSet(&p,draw,(xui_rect_t){7,6,70,50})==XUI_OK &&
        p.drawTextShapeRangeSpans(&p,draw,&shape,0,20,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,.25f,spans,20)==XUI_OK &&
        p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],actual,W*4)==XUI_OK &&
        !memcmp(actual,expected,sizeof(actual)));
    {
        xui_text_item_t range={.iSize=sizeof(range),.pFont=font,.sText="aaaaaaaaaaaaaaaaaaaa",.iTextSize=20,
            .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RANGE,.iRangeEnd=20,.pShape=&shape,.fDrawOffsetX=.25f};
        CHECK(p.surfaceClear(&p,targets[0],0)==XUI_OK && p.drawBegin(&p,&draw,targets[0])==XUI_OK &&
            p.drawClipSet(&p,draw,(xui_rect_t){7,6,70,50})==XUI_OK);
        for(unsigned attempt=0;attempt<2;attempt++){
            saved=xgeClipGet();xrtClearError();xrtMemDebugSnapshot(&before);CHECK(xrtMemDebugFailAfter(attempt));
            result=p.drawTextSpans(&p,draw,&range,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,spans,20);
            CHECK(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();after=xgeClipGet();
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !memcmp(&saved,&after,sizeof(saved)));same_live(&before);
        }
        spans[1].iStart=0;
        CHECK(p.drawTextSpans(&p,draw,&range,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,spans,20)==XUI_ERROR_INVALID_ARGUMENT);
        CHECK(p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],actual,W*4)==XUI_OK);
        for(size_t i=0;i<sizeof(actual);i++)CHECK(actual[i]==0);
        spans[1].iStart=1;
        CHECK(p.drawBegin(&p,&draw,targets[0])==XUI_OK && p.drawClipSet(&p,draw,(xui_rect_t){7,6,70,50})==XUI_OK &&
            p.drawTextSpans(&p,draw,&range,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,spans,20)==XUI_OK &&
            p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],actual,W*4)==XUI_OK && !memcmp(actual,expected,sizeof(actual)));
        puts("Native basic RANGE paint: both colour-map OOM points and invalid spans leave pixels/clip/live storage unchanged, retry exact RGBA passed");
    }
    xuiTextShapeFree(&shape);p.fontDestroy(&p,font);
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);
    puts("Native retained span paint: heap allocation failure/invalid overlap paint no pixels, preserve GPU clip/live storage, retry exact full RGBA passed");
    xgeQuit();return XGE_OK;
}
static void unicode_document_failures(const char* font_name,const char* text,const char* name,double caret_x,double width,unsigned caret_offset)
{
    xui_proxy_t proxy=xuiProxyXge();xui_context context;xui_font font;
    xui_document document;xui_document_snapshot snapshot;xui_document_renderer renderer;
    xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=XUI_DOCUMENT_MARKDOWN};
    xmemdebugsnapshot before;xui_doc_position_t position={0};uint64_t paragraph,leaf;unsigned attempt,failed=0;
#ifdef TEST_BASIC_RANGE
    proxy.textShapeRangeMeasure=NULL;proxy.drawTextShapeRangeSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,font_name,40,0)==XUI_OK &&
        xuiSetDefaultFont(context,font)==XUI_OK && xuiDocumentCreate(&profile,&document)==XUI_OK &&
        xuiDocumentLoadMarkdown(document,text,strlen(text))==XUI_OK && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&leaf)==XUI_OK);
    position.iSize=sizeof(position);position.iDocumentId=xuiDocumentGetIdentity(document);
    position.iRevision=xuiDocumentGetRevision(document);position.iKind=XUI_DOC_POSITION_TEXT;
    position.iNodeId=leaf;position.iOffset=caret_offset;position.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererCreate(context,NULL,&renderer)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK && xuiDocumentRendererLayout(renderer,width,0,160)==XUI_OK);
    xuiDocumentRendererRelease(renderer);xrtClearError();xrtMemDebugSnapshot(&before);
    for(attempt=0;attempt<256;attempt++){
        int result;bool fired;xui_doc_rect_t caret;
        CHECK(xuiDocumentRendererCreate(context,NULL,&renderer)==XUI_OK && xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
        CHECK(xrtMemDebugFailAfter(attempt));result=xuiDocumentRendererLayout(renderer,width,0,160);
        fired=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        if(fired){CHECK(result==XUI_ERROR_OUT_OF_MEMORY);failed++;
            CHECK(xuiDocumentRendererLayout(renderer,width,0,160)==XUI_OK);
        }else CHECK(result==XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(renderer,&position,&caret)==XUI_OK);
        if(caret.x!=caret_x || caret.y!=0)fprintf(stderr,"Native %s retry caret=%g/%g expected=%g/0 attempt=%u\n",name,caret.x,caret.y,caret_x,attempt);
        CHECK(caret.x==caret_x && caret.y==0);
        xuiDocumentRendererRelease(renderer);xrtClearError();same_live(&before);
        if(!fired)break;
    }
    CHECK(attempt<256 && failed>=5);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);xuiDestroy(context);proxy.fontDestroy(&proxy,font);
    printf("Native %s Document joint seed: %u allocation failures/retries, contextual caret=%g and identical live storage after every released renderer passed\n",name,failed,caret_x);
}
static void range_failures(xui_proxy p,xui_font font,const xui_text_item_t* source)
{
    xui_text_shape_t joint={0},selected={0};xui_text_item_t item=*source;xmemdebugsnapshot before;
    unsigned failures[2]={0};
    CHECK(p->textShape(p,source,&joint)==XUI_OK && joint.pPaint);
    item.pFont=font;item.iFlags|=XUI_TEXT_SHAPE_RANGE;item.iRangeStart=0;item.iRangeEnd=source->iTextSize;
    for(unsigned borrowed=0;borrowed<2;borrowed++){
        item.pShape=borrowed?&joint:NULL;
        CHECK(p->textShape(p,&item,&selected)==XUI_OK && selected.iCaretCount==2 && !selected.pPaint);
        xuiTextShapeFree(&selected);
        unsigned attempt;for(attempt=0;attempt<128;attempt++){
            int result;bool fired;xrtClearError();xrtMemDebugSnapshot(&before);
            CHECK(xrtMemDebugFailAfter(attempt));result=p->textShape(p,&item,&selected);fired=xrtMemDebugFailTriggered();
            xrtMemDebugFailClear();xrtClearError();
            if(fired){CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !selected.pPaint);failures[borrowed]++;}
            else CHECK(result==XUI_OK && selected.iCaretCount==2 && !selected.pPaint);
            xuiTextShapeFree(&selected);same_live(&before);
            CHECK(p->textShape(p,&item,&selected)==XUI_OK && selected.fWidth==joint.fWidth);
            xuiTextShapeFree(&selected);same_live(&before);
            if(!fired)break;
        }
        CHECK(attempt<128 && failures[borrowed]>=2);
    }
    xuiTextShapeFree(&joint);
    printf("Native whole-item RANGE: %u fresh and %u borrowed allocation failures/retries, ligature/GDEF geometry, no escaped borrowed paint and identical live storage passed\n",failures[0],failures[1]);
}
static void ascii_document_failures(void)
{
    xui_proxy_t p=xuiProxyXge();xui_context c;xui_font normal,bold;
    xui_document d;xui_document_snapshot s;xui_document_renderer r;uint64_t paragraph,leaf;
    xmemdebugsnapshot baseline;xui_doc_rect_t caret;unsigned attempt,failures=0;
#ifdef TEST_BASIC_RANGE
    p.textShapeRangeMeasure=NULL;p.drawTextShapeRangeSpans=NULL;
#endif
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK &&
        p.fontLoadFile(&p,&normal,"test/data/xge_ascii_script_fixture.ttf",40,0)==XUI_OK &&
        p.fontLoadFile(&p,&bold,"test/data/xge_ascii_script_fixture.ttf",37,0)==XUI_OK &&
        xuiSetDefaultFont(c,normal)==XUI_OK);
    xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=XUI_DOCUMENT_MARKDOWN};
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={normal,bold,normal,bold,normal}};
    CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,"(**a**)",7)==XUI_OK &&
        xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK &&
        xuiDocumentSnapshotGetChild(s,paragraph,0,&leaf)==XUI_OK);
    xui_doc_position_t at={.iSize=sizeof(at),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=xuiDocumentGetIdentity(d),
        .iRevision=xuiDocumentGetRevision(d),.iNodeId=leaf,.iOffset=1,.iAffinity=XUI_DOC_BEFORE};
    CHECK(xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK &&
        xuiDocumentRendererLayout(r,160,0,120)==XUI_OK);xuiDocumentRendererRelease(r);
    xrtClearError();xrtMemDebugSnapshot(&baseline);
    for(attempt=0;attempt<256;attempt++){
        CHECK(xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
        CHECK(xrtMemDebugFailAfter(attempt));int result=xuiDocumentRendererLayout(r,160,0,120);
        bool fired=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        CHECK(result==(fired?XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        if(fired){failures++;CHECK(xuiDocumentRendererLayout(r,160,0,120)==XUI_OK);}
        CHECK(xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && caret.x==12 && caret.y==0);
        CHECK(xuiDocumentRendererLayout(r,100,0,120)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && caret.x==12);
        xuiDocumentRendererRelease(r);xrtClearError();same_live(&baseline);if(!fired)break;
    }
    CHECK(attempt<256 && failures>=4);
    xuiDocumentSnapshotRelease(s);xuiDocumentRelease(d);xuiDestroy(c);p.fontDestroy(&p,normal);p.fontDestroy(&p,bold);
    printf("Native ASCII cross-font/size Document: %u layout allocation failures, same-renderer retry/reflow, Latin glyph caret 12 and unchanged live storage passed\n",failures);
}
static void word_document_failures(void)
{
    xui_proxy_t p=xuiProxyXge();xui_context c;xui_font normal,bold;
    xui_document d;xui_document_snapshot s;xui_document_renderer r;uint64_t paragraph,leaf;
    xmemdebugsnapshot baseline;xui_doc_rect_t caret;unsigned attempt,failures=0;
#ifdef TEST_BASIC_RANGE
    p.textShapeRangeMeasure=NULL;p.drawTextShapeRangeSpans=NULL;
#endif
    CHECK(xgeFontFallbackSet("test/data/xge_word_context_full.ttf",40)==XGE_OK);
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK &&
        p.fontLoadFile(&p,&normal,"test/data/xge_word_context_primary.ttf",40,0)==XUI_OK &&
        p.fontLoadFile(&p,&bold,"test/data/xge_word_context_primary.ttf",37,0)==XUI_OK &&
        xuiSetDefaultFont(c,normal)==XUI_OK);
    xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=XUI_DOCUMENT_MARKDOWN};
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={normal,bold,normal,bold,normal}};
    CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,"f**f**i",7)==XUI_OK &&
        xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK &&
        xuiDocumentSnapshotGetChild(s,paragraph,0,&leaf)==XUI_OK);
    xui_doc_position_t at={.iSize=sizeof(at),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=xuiDocumentGetIdentity(d),
        .iRevision=xuiDocumentGetRevision(d),.iNodeId=leaf,.iOffset=1,.iAffinity=XUI_DOC_BEFORE};
    CHECK(xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK &&
        xuiDocumentRendererLayout(r,160,0,120)==XUI_OK);xuiDocumentRendererRelease(r);
    xrtClearError();xrtMemDebugSnapshot(&baseline);
    for(attempt=0;attempt<256;attempt++){
        CHECK(xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
        CHECK(xrtMemDebugFailAfter(attempt));int result=xuiDocumentRendererLayout(r,160,0,120);
        bool fired=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        CHECK(result==(fired?XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        if(fired){failures++;CHECK(xuiDocumentRendererLayout(r,160,0,120)==XUI_OK);}
        CHECK(xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && caret.x==16 && caret.y==0);
        CHECK(xuiDocumentRendererLayout(r,100,0,120)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK && caret.x==16);
        xuiDocumentRendererRelease(r);xrtClearError();same_live(&baseline);if(!fired)break;
    }
    CHECK(attempt<256 && failures>=4);
    xuiDocumentSnapshotRelease(s);xuiDocumentRelease(d);xuiDestroy(c);p.fontDestroy(&p,normal);p.fontDestroy(&p,bold);xgeFontFallbackClear();
    printf("Native whole-word cross-font/size Document: %u layout allocation failures, same-renderer retry/reflow, whole-word fallback caret 16 and unchanged live storage passed\n",failures);
}
int main(void)
{
    xui_proxy_t proxy=xuiProxyXge();xui_font font;xui_text_item_t item={0};
    xui_text_shape_t shape={0};xmemdebugsnapshot before,after;unsigned i,failed=0;
    CHECK(xrtMemDebugEnable(true));
    CHECK(xgeFontFallbackSet("test/data/xge_opentype_fixture.ttf",40)==XGE_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_fallback_no_i.ttf",40,0)==XUI_OK);
    item.iSize=sizeof(item);item.pFont=font;item.sText="ffi";item.iTextSize=3;
    item.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT;
    CHECK(proxy.textShape(&proxy,&item,&shape)==XUI_OK && shape.pPaint);
    xuiTextShapeFree(&shape); /* Warm fallback glyph/contour caches. */
    for(i=0;i<64;i++){
        int result;bool fired;
        xrtClearError();xrtMemDebugSnapshot(&before);CHECK(xrtMemDebugFailAfter(i));
        result=proxy.textShape(&proxy,&item,&shape);fired=xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();xrtClearError();
        if(fired){
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !shape.pPaint);
            xuiTextShapeFree(&shape);same_live(&before);failed++;
            CHECK(proxy.textShape(&proxy,&item,&shape)==XUI_OK && shape.pPaint);
            xuiTextShapeFree(&shape);same_live(&before);
        }else{
            CHECK(result==XUI_OK && shape.pPaint);
            xrtMemDebugSnapshot(&after);
            CHECK(after.LiveBytes-before.LiveBytes==shape.iPaintBytes+
                (size_t)shape.iClusterCount*sizeof(*shape.pClusters)+
                (size_t)shape.iCaretCount*sizeof(*shape.pCarets));
            xuiTextShapeFree(&shape);same_live(&before);break;
        }
    }
    CHECK(i<64 && failed>=4);
    context_storage(&proxy,font);
    range_failures(&proxy,font,&item);
    CHECK(proxy.textShape(&proxy,&item,&shape)==XUI_OK && shape.pPaint);
    proxy.fontDestroy(&proxy,font);xgeFontFallbackClear();
    xuiTextShapeFree(&shape);xuiTextShapeFree(&shape);xrtClearError();xrtMemDebugSnapshot(&after);
    CHECK(after.LiveCount==0 && after.LiveBytes==0 && after.InvalidFreeCount==0 && after.DoubleFreeCount==0);
    printf("Native retained paint: %u allocation failures/retries, exact owned-byte accounting, fallback carriers, deferred font release and zero live allocations passed\n",failed);
    unicode_document_failures("test/data/xge_unicode_context_fixture.ttf","\xce\xbb \xce\xbc","Unicode",32,45,2);
    unicode_document_failures("test/data/xge_unicode_context_fixture.ttf","\xce\xbb\xe2\x80\x8b \xce\xbc\xef\xbb\xbf","projected Greek",32,45,2);
    unicode_document_failures("test/data/xge_rtl_context_fixture.ttf","\xd7\x90\xe2\x80\x8b \xd7\x91\xef\xbb\xbf","projected Hebrew",10,45,2);
    unicode_document_failures("test/data/xge_rtl_context_fixture.ttf","\xd7\x90 \xd7\x91","RTL Hebrew",10,45,2);
    unicode_document_failures("test/data/xge_shy_context_fixture.ttf","\xce\xbb\xc2\xad \xce\xbc","unselected SHY Greek",32,45,2);
    /* Unicode 17 permits the SP break; the contextual A stays on the first
     * RTL line, ending at the retained space's x=10. SHY is invisible. */
    unicode_document_failures("test/data/xge_shy_context_fixture.ttf","\xd7\x90\xc2\xad \xd7\x91","unselected SHY Hebrew",10,45,2);
    unicode_document_failures("test/data/xge_shy_context_fixture.ttf","\xce\xbb\xc2\xad\xce\xbc","selected SHY Greek",20,45,2);
    unicode_document_failures("test/data/xge_shy_context_fixture.ttf","\xce\xbd\xc2\xad\xce\xbe","rejected SHY Greek",32,45,2);
    unicode_document_failures("test/data/xge_shy_context_fixture.ttf","\xe2\x80\xae\xce\xbb\xc2\xad\xce\xbc\xe2\x80\xac","selected SHY RLO",6,45,5);
    unicode_document_failures("test/data/xge_shy_future_fixture.ttf","\xce\xbb\xc2\xad\xce\xbc \xce\xbd\xc2\xad\xce\xbe","future SHY pattern",28,28,6);
    ascii_document_failures();
    word_document_failures();
    xrtClearError();xrtMemDebugSnapshot(&after);
    CHECK(after.LiveCount==0 && after.LiveBytes==0 && after.InvalidFreeCount==0 && after.DoubleFreeCount==0);
    {xge_desc_t desc={0};desc.iWidth=120;desc.iHeight=80;
        desc.sTitle="Retained paint OOM";desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
        CHECK(xgeInit(&desc)==XGE_OK && xgeRun(paint_failure_frame,NULL)==XGE_OK);xgeUnit();}
    return 0;
}
