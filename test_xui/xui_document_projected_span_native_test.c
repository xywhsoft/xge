/* Deleted controls keep source positions but must not break contextual glyphs. */
#include "../src/xui_document_layout_internal.h"
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=180,H=180,MAX_PARTS=12 };
#define CHECK(e) do{if(!(e)){fprintf(stderr,"projected span %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;
static unsigned shapes,raw,cached,cases;
static const char* displayed_text;
static void* joint_key;
static int capture_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    shapes++;int result=original.textShape(p,item,out);
    if(result==XUI_OK && out->pPaint && displayed_text && item->iTextSize==5 && !memcmp(item->sText,displayed_text,5))joint_key=out->pPaint;
    return result;
}
static int capture_raw(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{raw++;return original.drawText(p,d,item,rect,color,flags);}
static int capture_spans(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{raw++;return original.drawTextSpans(p,d,item,rect,color,flags,spans,count);}
static int capture_range(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{
    CHECK(shape->pPaint && start>=0 && end<=5 && end>start);
    for(int i=0;i<count;i++)CHECK(spans[i].iStart>=start && spans[i].iEnd<=end);
    cached++;return original.drawTextShapeRangeSpans(p,d,shape,start,end,rect,color,flags,offset,spans,count);
}
static xui_font resolve(xui_context c,const char* family,uint32_t marks,float size,void* user)
{(void)c;(void)marks;(void)size;return family && family[0]?*(xui_font*)user:NULL;}
static unsigned scalar_bytes(unsigned char c)
{return c<0x80?1:c<0xe0?2:c<0xf0?3:4;}
static void literal(xui_proxy p,xui_draw_context draw,xui_font font,unsigned cp,double x,double y,uint32_t color)
{
    char text[]={(char)(0xe0|cp>>12),(char)(0x80|((cp>>6)&63)),(char)(0x80|(cp&63)),0};
    xui_rect_t rect={(int)floor(x+.5),(int)floor(y+.5),W,H-(int)floor(y+.5)};
    CHECK(original.drawText(p,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=text,.iTextSize=3,
        .iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=(float)(x-rect.fX)},rect,color,0)==XUI_OK);
}
static xui_doc_position_t position(xui_document_snapshot snapshot,unsigned form,uint64_t leaf,
    unsigned local,unsigned source,uint32_t affinity)
{
    xui_doc_position_t result={.iSize=sizeof(result),.iKind=form<3?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE,
        .iDocumentId=xuiDocumentSnapshotGetIdentity(snapshot),.iRevision=xuiDocumentSnapshotGetRevision(snapshot),
        .iNodeId=form<3?leaf:1,.iOffset=form<3?local:source,.iAffinity=affinity};
    if(form==2){int mapping;uint64_t back=UINT64_MAX;
        CHECK(xuiDocumentSourceToPositionEx(snapshot,source,affinity,&result,&mapping)==XUI_OK &&
            xuiDocumentPositionToSource(snapshot,&result,&back,&mapping)==XUI_OK && back==source);}
    return result;
}
static void equal_pixels(xui_proxy p,xui_surface* targets,unsigned direction,unsigned form,unsigned control,unsigned pass,float size)
{
    unsigned char actual[W*H*4],expected[W*H*4];unsigned ink=0;
    CHECK(p->surfaceReadRGBA(p,targets[0],actual,W*4)==XUI_OK && p->surfaceReadRGBA(p,targets[1],expected,W*4)==XUI_OK);
    for(unsigned i=0;i<sizeof(actual);i++){
        if(actual[i]!=expected[i])fprintf(stderr,"projected RGBA direction=%u form=%u control=%u pass=%u size=%g byte=%u got=%u expected=%u\n",
            direction,form,control,pass,size,i,actual[i],expected[i]);
        CHECK(actual[i]==expected[i]);if(i%4==3 && actual[i])ink++;
    }CHECK(ink>20);cases++;
}
static void document_cases(xui_proxy p,xui_context context,xui_font* fonts,float size,unsigned direction,xui_surface* targets)
{
    const char* letters=direction?"\xd7\x90 \xd7\x91":"\xce\xbb \xce\xbc";
    const unsigned base=direction?0xe600:0xe500;
    for(unsigned control=0;control<4;control++){
        char text[40];
        if(control<2)snprintf(text,sizeof(text),"%.*s%s%s",2,letters,control?"\xe2\x81\xa0":"\xe2\x80\x8b",letters+2);
        else if(control==2)snprintf(text,sizeof(text),"\xef\xbb\xbf%.*s \xe2\x80\x8b%s\xef\xbb\xbf",2,letters,letters+3);
        else snprintf(text,sizeof(text),"%s%.*s \xe2\x80\x8b%s\xe2\x81\xa9",direction?"\xe2\x81\xa7":"\xe2\x81\xa6",2,letters,letters+3);
        unsigned offsets[MAX_PARTS+1]={0},part_count=0,glyph_parts[2]={0};
        while(offsets[part_count]<strlen(text)){
            unsigned at=offsets[part_count],bytes=scalar_bytes((unsigned char)text[at]);
            if(bytes==2 && !memcmp(text+at,letters,2))glyph_parts[0]=part_count;
            if(bytes==2 && !memcmp(text+at,letters+3,2))glyph_parts[1]=part_count;
            CHECK(part_count<MAX_PARTS);offsets[++part_count]=at+bytes;
        }
        for(unsigned form=0;form<5;form++){
            uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xff20c840)};
            uint64_t leaves[MAX_PARTS]={0},paragraph;
            xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
            xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form<2?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN};
            xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={fonts[0],fonts[0],fonts[0],fonts[0],fonts[0]},
                .iTextColor=colors[0],.onFont=resolve,.pUser=&fonts[1]};
            CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
            if(form<2){
                xui_document_transaction txn;xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
                CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
                for(unsigned i=0;i<(form?part_count:1);i++){
                    unsigned from=form?offsets[i]:0,bytes=form?offsets[i+1]-from:(unsigned)strlen(text);
                    node.iKind=XUI_DOC_TEXT;node.sText=text+from;node.iTextBytes=bytes;
                    node.tAttributes.iTextColor=form && i==glyph_parts[1]?colors[1]:colors[0];
                    /* Invisible alternate fonts must not choose the visible span's cache key. */
                    snprintf(node.tAttributes.sFontFamily,sizeof(node.tAttributes.sFontFamily),"%s",form && bytes==3?"hidden":"");
                    CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
                }CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
            }else CHECK(xuiDocumentLoadMarkdown(d,text,strlen(text))==XUI_OK);
            unsigned mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
            CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
                xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK && xuiDocumentRendererSetMode(r,mode)==XUI_OK);
            if(form==4){xui_doc_position_t start=position(snapshot,form,0,0,0,XUI_DOC_AFTER);CHECK(xuiDocumentRendererSetActivePosition(r,&start)==XUI_OK);}
            void* key=NULL;displayed_text=letters;joint_key=NULL;
            for(unsigned pass=0;pass<3;pass++){
                double width=pass==1?size*1.13:W,ox=pass==1?.25:0;unsigned before;
                xui_draw_context draw;xui_rect_t clip=pass==1?(xui_rect_t){5,0,W-5,H}:(xui_rect_t){0,0,W,H};
                CHECK(xuiDocumentRendererLayout(r,width,0,H)==XUI_OK);
                if(form<3){
                    doc_render_block* block=&r->blocks[0];doc_render_paint_seed* joint=NULL;
                    for(size_t i=0;i<block->paint_seed_count;i++){
                        doc_render_paint_seed* seed=&block->paint_seeds[i];
                        if(seed->offsets && seed->offsets[seed->end_fragment-seed->first_fragment]==5)joint=seed;
                    }
                    CHECK(joint && !joint->source_run && joint->font==fonts[0] && doc_render_seed_shape(block,joint)->pPaint==joint_key);
                    for(size_t i=0;i<block->fragment_count;i++){
                        if(block->fragments[i].flags & DOC_INVISIBLE)CHECK(block->fragments[i].width==0);
                    }
                }
                /* SOURCE rows currently discard their caches when width changes. */
                CHECK(joint_key);if(!key || form==3)key=joint_key;else{
                    if(key!=joint_key)fprintf(stderr,"projected cache direction=%u form=%u control=%u pass=%u size=%g\n",direction,form,control,pass,size);
                    CHECK(key==joint_key);
                }
                if(form==1 && pass==2){
                    xui_document_transaction txn;xui_document_change_set change;
                    xui_doc_node_info_t info={.iSize=sizeof(info)};
                    CHECK(xuiDocumentSnapshotGetNode(snapshot,leaves[glyph_parts[1]],&info)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
                    colors[1]=UINT32_C(0xffc88020);info.tAttributes.iTextColor=colors[1];
                    CHECK(xuiDocumentTxnSetAttributes(txn,leaves[glyph_parts[1]],&info.tAttributes)==XUI_OK && xuiDocumentTxnCommit(txn,&change)==XUI_OK);
                    xuiDocumentTxnRelease(txn);xuiDocumentSnapshotRelease(snapshot);before=shapes;
                    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,change)==XUI_OK &&
                        xuiDocumentRendererLayout(r,width,0,H)==XUI_OK && shapes==before && joint_key==key);
                    xuiDocumentChangeSetRelease(change);
                }
                if(pass==2){xuiDocumentRelease(d);d=NULL;}
                before=shapes;raw=cached=0;
                CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                    xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK &&
                    shapes==before && raw==0 && cached>=1u+((pass==1 && form<3)?1u:0u));
                CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK && p->drawClipSet(p,draw,clip)==XUI_OK);
                for(unsigned i=0;i<2;i++){
                    unsigned part=glyph_parts[i],source=offsets[part],local=form==1?0:source;
                    xui_doc_position_t start=position(snapshot,form,leaves[form==1?part:0],local,source,XUI_DOC_AFTER);
                    xui_doc_position_t end=position(snapshot,form,leaves[form==1?part:0],local+2,source+2,XUI_DOC_BEFORE);
                    xui_doc_rect_t a,b;
                    CHECK(xuiDocumentRendererGetCaretRect(r,&start,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&end,&b)==XUI_OK);
                    if(fabs(fabs(b.x-a.x)-size*(i?.9:.8))>=.001)fprintf(stderr,"projected caret direction=%u form=%u control=%u pass=%u size=%g glyph=%u advance=%g\n",direction,form,control,pass,size,i,b.x-a.x);
                    CHECK(fabs(a.y-b.y)<.001 && fabs(fabs(b.x-a.x)-size*(i?.9:.8))<.001);
                    for(unsigned side=0;side<2;side++){
                        xui_doc_position_t hit={.iSize=sizeof(hit)};xui_doc_rect_t hit_rect;
                        double fraction=side?.8:.2;
                        CHECK(xuiDocumentRendererHitTest(r,a.x+(b.x-a.x)*fraction,a.y+a.height*.5,&hit)==XUI_OK &&
                            xuiDocumentRendererGetCaretRect(r,&hit,&hit_rect)==XUI_OK &&
                            fabs(hit_rect.x-(side?b.x:a.x))<.001 && fabs(hit_rect.y-a.y)<.001);
                    }
                    literal(p,draw,fonts[0],base+4+i,ox+fmin(a.x,b.x),a.y,colors[form==1?i:0]);
                }
                CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets,direction,form,control,pass,size);
            }xuiDocumentSnapshotRelease(snapshot);xuiDocumentRendererRelease(r);
        }
    }
}
static void ligature_cases(xui_proxy p,xui_context context,xui_font* fonts,float size,unsigned direction,xui_surface* targets)
{
    const char* text=direction?"\xd7\x92\xe2\x80\x8b\xd7\x93":"\xcf\x86\xe2\x80\x8b\xce\xb1";
    const unsigned offsets[]={0,2,5,7},base=direction?0xe600:0xe500;
    const uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xffc82040)};
    xui_document d;xui_document_snapshot snapshot;xui_document_transaction txn;xui_document_renderer r;
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};uint64_t paragraph,leaf;
    xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={fonts[0],fonts[0],fonts[0],fonts[0],fonts[0]},.iTextColor=colors[0],.onFont=resolve,.pUser=&fonts[1]};
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    for(unsigned i=0;i<3;i++){
        node.iKind=XUI_DOC_TEXT;node.sText=text+offsets[i];node.iTextBytes=offsets[i+1]-offsets[i];node.tAttributes.iTextColor=colors[i==2];
        snprintf(node.tAttributes.sFontFamily,sizeof(node.tAttributes.sFontFamily),"%s",i==1?"hidden":"");
        CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);void* key=NULL;
    for(unsigned pass=0;pass<3;pass++){
        xui_draw_context draw;unsigned before;double ox=.25;
        CHECK(xuiDocumentRendererLayout(r,pass==1?size*.4:W,0,H)==XUI_OK);
        doc_render_block* block=&r->blocks[0];CHECK(block->fragment_count==3 && block->fragments[1].width==0);
        doc_render_paint_seed* seed=NULL;
        for(size_t i=0;i<block->paint_seed_count;i++){
            doc_render_paint_seed* candidate=&block->paint_seeds[i];
            if(candidate->offsets && candidate->offsets[candidate->end_fragment-candidate->first_fragment]==4)seed=candidate;
        }
        CHECK(seed && seed->offsets[0]==0 && seed->offsets[1]==2 && seed->offsets[2]==2 && seed->offsets[3]==4 && !seed->source_run);
        if(!key)key=doc_render_seed_shape(block,seed)->pPaint;else CHECK(key==doc_render_seed_shape(block,seed)->pPaint);
        CHECK(key);
        if(pass==1){
            CHECK(block->fragments[2].y>block->fragments[0].y && fabs(block->fragments[0].width-size*.6)<.001 && fabs(block->fragments[2].width-size*.25)<.001);
            for(size_t i=0;i<block->paint_group_count;i++)if(block->paint_groups[i].bytes)CHECK(!block->paint_groups[i].shared_seed && block->paint_groups[i].shape.pPaint);
        }else CHECK(block->paint_group_count==1 && block->paint_groups[0].shared_seed && fabs(block->fragments[0].width-size*.375)<.001 && fabs(block->fragments[2].width-size*.375)<.001);
        if(pass==2)xuiDocumentRelease(d);
        before=shapes;raw=cached=0;
        CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK && xuiDocumentRendererDraw(r,draw,ox,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK && raw==0 && shapes==before && cached>0);
        CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
        for(unsigned i=0;i<2;i++){
            unsigned side=direction?1-i:i;int boundary=(int)floor(ox+size*.375+.5);
            if(pass!=1)CHECK(p->drawClipSet(p,draw,(xui_rect_t){side?boundary:0,0,side?W-boundary:boundary,H})==XUI_OK);
            literal(p,draw,fonts[0],base+(pass==1?6+i:8),ox+(pass==1?block->fragments[2*i].x:0),pass==1?block->fragments[2*i].y:0,colors[i]);
        }
        CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets,direction,5,4,pass,size);
    }xuiDocumentRendererRelease(r);
}
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t p=original=xuiProxyXge();xui_context context;xui_font fonts[2];xui_surface targets[2];
    xui_surface_desc_t surface={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    p.textShape=capture_shape;p.drawText=capture_raw;p.drawTextSpans=capture_spans;p.drawTextShapeRangeSpans=capture_range;
#ifdef TEST_DRAW_TEXT_ONLY
    p.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK && p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
    for(unsigned direction=0;direction<2;direction++){
        const char* file=direction?"test/data/xge_rtl_context_fixture.ttf":"test/data/xge_unicode_context_fixture.ttf";
        for(unsigned i=0;i<2;i++)CHECK(p.fontLoadFile(&p,&fonts[i],file,size,0)==XUI_OK);
        CHECK(fonts[0]!=fonts[1] && xuiSetDefaultFont(context,fonts[0])==XUI_OK);
        document_cases(&p,context,fonts,size,direction,targets);
        ligature_cases(&p,context,fonts,size,direction,targets);
        for(unsigned i=0;i<2;i++)p.fontDestroy(&p,fonts[i]);
    }
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);xuiDestroy(context);xgeQuit();return XGE_OK;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){float size=sizes[i];xge_desc_t desc={.iWidth=W,.iHeight=H,.sTitle="Projected contextual spans",.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};
        CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&size)==XGE_OK);xgeUnit();}
    printf("Native projected spans: %u exact full RGBA cases, Greek/Hebrew contextual GSUB and ligature interior-cut fallback; ZWSP/WJ/BOM/isolates, Rich full/split hidden font and colour edits, Markdown visual/source/live, caret/hit/source mapping/zero-width controls/visual-live cache reuse/wrap/clip/fractional X/released Document, zero raw draw/reshape passed\n",cases);return 0;
}
