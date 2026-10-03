/* Literal glyphs independently prove base spans and selected SHY line variants. */
#include "../src/xui_document_layout_internal.h"
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=180,H=180,MAX_PARTS=12 };
#define CHECK(e) do{if(!(e)){fprintf(stderr,"SHY span %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;
static unsigned shapes,raw,cached,cases;
static int paint_route(void)
{
#ifdef TEST_BASIC_RANGE
    return raw>0 && cached==0;
#else
    return raw==0 && cached>0;
#endif
}
static const char* displayed_text;
static int displayed_bytes;
static void* joint_key;
static int capture_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    shapes++;int result=original.textShape(p,item,out);
    if(result==XUI_OK && out->pPaint && displayed_text && item->iTextSize==displayed_bytes && !memcmp(item->sText,displayed_text,(size_t)displayed_bytes))joint_key=out->pPaint;
    return result;
}
static int capture_raw(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{raw++;return original.drawText(p,d,item,rect,color,flags);}
static int capture_spans(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{raw++;return original.drawTextSpans(p,d,item,rect,color,flags,spans,count);}
static int capture_range(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{
    CHECK(shape->pPaint && start>=0 && end<=11 && end>start);
    for(int i=0;i<count;i++)CHECK(spans[i].iStart>=start && spans[i].iEnd<=end);
    cached++;return original.drawTextShapeRangeSpans(p,d,shape,start,end,rect,color,flags,offset,spans,count);
}
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
        if(actual[i]!=expected[i])fprintf(stderr,"SHY RGBA direction=%u form=%u control=%u pass=%u size=%g byte=%u got=%u expected=%u\n",
            direction,form,control,pass,size,i,actual[i],expected[i]);
        CHECK(actual[i]==expected[i]);if(i%4==3 && actual[i])ink++;
    }CHECK(ink>20);cases++;
}
static void document_cases(xui_proxy p,xui_context context,xui_font font,float size,unsigned direction,unsigned pair,xui_surface* targets)
{
    const char* letters=direction==1?(pair?"\xd7\x92 \xd7\x93":"\xd7\x90 \xd7\x91"):(pair?"\xce\xbd \xce\xbe":"\xce\xbb \xce\xbc");
    unsigned base=0xe700+3+5*((direction==1?2:0)+pair);
#ifdef TEST_NO_HB
    for(unsigned control=2;control<3;control++)for(unsigned form=0;form<5;form++){
#else
    for(unsigned control=0;control<3;control++)for(unsigned form=0;form<5;form++){
#endif
        char text[20],display[6];
        if(control==2)snprintf(text,sizeof(text),"%.*s\xc2\xad%s",2,letters,letters+3);
        else if(control)snprintf(text,sizeof(text),"%.*s\xc2\xad%s",3,letters,letters+3);
        else snprintf(text,sizeof(text),"%.*s\xc2\xad%s",2,letters,letters+2);
        snprintf(display,sizeof(display),"%.*s%s",control==2?2:3,letters,letters+3);
        /* Unicode 17 LB21a protects HL (HY|HH), not BA (SHY). Natural
         * Hebrew and Greek under RLO both exercise odd-level candidates;
         * the independently wider terminal still proves candidate rejection. */
        if(direction==2){size_t bytes=strlen(text);memmove(text+3,text,bytes);memcpy(text,"\xe2\x80\xae",3);memcpy(text+3+bytes,"\xe2\x80\xac",4);}
        unsigned offsets[MAX_PARTS]={0},parts=0,glyph_parts[]={direction==2?1u:0u,(control==2?2u:3u)+(direction==2)},hyphen_part=(control==1?2u:1u)+(direction==2);
        while(offsets[parts]<strlen(text)){unsigned at=offsets[parts];offsets[++parts]=at+scalar_bytes((unsigned char)text[at]);}
        CHECK(parts==(control==2?3u:4u)+2*(direction==2));
        uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xffc82040),UINT32_C(0xff20c840)};
        uint64_t leaves[MAX_PARTS]={0},paragraph;xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
        xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form<2?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN};
        xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={font,font,font,font,font},.iTextColor=colors[0]};
        CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
        if(form<2){
            xui_document_transaction txn;xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
            CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            for(unsigned i=0;i<(form?parts:1);i++){
                unsigned from=form?offsets[i]:0;
                node.iKind=XUI_DOC_TEXT;node.sText=text+from;node.iTextBytes=form?offsets[i+1]-from:strlen(text);
                node.tAttributes.iTextColor=form?(i==hyphen_part?colors[1]:i==glyph_parts[1]?colors[2]:colors[0]):colors[0];
                CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        }else CHECK(xuiDocumentLoadMarkdown(d,text,strlen(text))==XUI_OK);
        unsigned mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK && xuiDocumentRendererSetMode(r,mode)==XUI_OK);
        if(form==4){xui_doc_position_t start=position(snapshot,form,0,0,0,XUI_DOC_AFTER);CHECK(xuiDocumentRendererSetActivePosition(r,&start)==XUI_OK);}
        void* key=NULL;displayed_text=display;displayed_bytes=control==2?4:5;joint_key=NULL;
        for(unsigned pass=0;pass<4;pass++){
#ifdef TEST_NO_HB
            const double widths[]={W,size*.6,size*.45,W};double ox=pass%2?.25:0;
            unsigned selected=form<3 && pass==1;
#else
            const double widths[]={W,size*1.13,size*.7,W};double ox=pass%2?.25:0;
            unsigned selected=form<3 && !pair && ((control==1 && pass==1) || (control==2 && (pass==1 || pass==2)));
            unsigned terminal=selected;
#endif
            xui_doc_rect_t rects[2][2];xui_draw_context draw;unsigned before;
            xui_rect_t clip=pass==2?(xui_rect_t){5,0,W-5,H}:(xui_rect_t){0,0,W,H};
            CHECK(xuiDocumentRendererLayout(r,widths[pass],0,H)==XUI_OK);
            if(form<3){
                doc_render_block* block=&r->blocks[0];doc_render_paint_seed* seed=NULL;unsigned used=0;
                for(size_t i=0;i<block->paint_seed_count;i++){
                    doc_render_paint_seed* candidate=&block->paint_seeds[i];
                    if(candidate->offsets && candidate->offsets[candidate->end_fragment-candidate->first_fragment]==(uint32_t)displayed_bytes)seed=candidate;
                }
                CHECK(seed && !seed->source_run && doc_render_seed_shape(block,seed)->pPaint==joint_key);
                for(size_t i=0;i<block->fragment_count;i++)if(block->fragments[i].flags & DOC_SOFT_HYPHEN){
                    if(fabs(block->fragments[i].width-(selected?size*.15:0))>=.001){
                        fprintf(stderr,"SHY marker dir=%u pair=%u form=%u marker=%u pass=%u size=%g fragment=%zu width=%g expected=%g rows=%zu\n",direction,pair,form,control,pass,size,i,block->fragments[i].width,selected?size*.15:0,block->line_count);
                        for(size_t f=0;f<block->fragment_count;f++)fprintf(stderr,"frag %zu width=%g x=%g y=%g flags=%x\n",f,block->fragments[f].width,block->fragments[f].x,block->fragments[f].y,block->fragments[f].flags);
                        for(size_t g=0;g<block->paint_group_count;g++)fprintf(stderr,"group %zu text=%s start=%zu end=%zu shared=%zu variant=%d\n",g,block->paint_groups[g].text,block->paint_groups[g].first_fragment,block->paint_groups[g].end_fragment,block->paint_groups[g].shared_seed,block->paint_groups[g].has_hyphen_context);
                    }
                    CHECK(fabs(block->fragments[i].width-(selected?size*.15:0))<.001);
                    used+=(block->fragments[i].flags & DOC_HYPHEN_USED)!=0;
                }
                CHECK(used==selected);
                for(size_t i=0;i<block->paint_group_count;i++){
                    doc_render_paint_group* group=&block->paint_groups[i];
                    unsigned owns_hyphen=0;
                    for(size_t f=group->first_fragment;f<group->end_fragment;f++)owns_hyphen+=(block->fragments[f].flags & DOC_HYPHEN_USED)!=0;
                    if(selected){
                        CHECK(!group->shared_seed && group->variant &&
                            (group->variant_span || group->shape.pPaint || !group->bytes));
                        if(owns_hyphen)CHECK(strchr(group->variant->text,'-'));
                    }
                    else if(group->bytes){
                        if(group->bidi_level==seed->bidi_level){
                            if(!group->shared_seed)fprintf(stderr,"Unshared dir=%u pair=%u form=%u control=%u pass=%u group=%s fragments=%zu-%zu seed=%zu-%zu level=%u\n",direction,pair,form,control,pass,group->text,group->first_fragment,group->end_fragment,seed->first_fragment,seed->end_fragment,group->bidi_level);
                            CHECK(group->shared_seed);
                        }
                        else {
                            /* UBA L1 changes trailing spaces outside RLO back
                             * to the paragraph level; those spaces own shape. */
                            CHECK(!group->shared_seed && group->shape.pPaint);
                            for(uint64_t b=0;b<group->bytes;b++)CHECK(group->text[b]==' ');
                        }
                    }
                }
            }
            CHECK(joint_key);if(!key || form==3)key=joint_key;else CHECK(key==joint_key);
            if(form==1 && pass==3){
                xui_document_transaction txn;xui_document_change_set change;xui_doc_node_info_t info={.iSize=sizeof(info)};
                CHECK(xuiDocumentSnapshotGetNode(snapshot,leaves[glyph_parts[1]],&info)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
                colors[2]=UINT32_C(0xffc88020);info.tAttributes.iTextColor=colors[2];
                CHECK(xuiDocumentTxnSetAttributes(txn,leaves[glyph_parts[1]],&info.tAttributes)==XUI_OK && xuiDocumentTxnCommit(txn,&change)==XUI_OK);
                xuiDocumentTxnRelease(txn);xuiDocumentSnapshotRelease(snapshot);before=shapes;
                CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,change)==XUI_OK &&
                    xuiDocumentRendererLayout(r,widths[pass],0,H)==XUI_OK && shapes==before && joint_key==key);
                xuiDocumentChangeSetRelease(change);
            }
            if(pass==3){xuiDocumentRelease(d);d=NULL;}
            before=shapes;raw=cached=0;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK && paint_route() && shapes==before);
            for(unsigned i=0;i<2;i++){
                unsigned part=glyph_parts[i],source=offsets[part],local=form==1?0:source;
                xui_doc_position_t start=position(snapshot,form,leaves[form==1?part:0],local,source,XUI_DOC_AFTER);
                xui_doc_position_t end=position(snapshot,form,leaves[form==1?part:0],local+2,source+2,XUI_DOC_BEFORE);
                xui_doc_rect_t *a=&rects[i][0],*b=&rects[i][1];
#ifdef TEST_NO_HB
                double advance=i?.3:.4;
#else
                double advance=i?(selected?.3:.9):terminal?.5:.8;
#endif
                CHECK(xuiDocumentRendererGetCaretRect(r,&start,a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&end,b)==XUI_OK);
                if(fabs(fabs(b->x-a->x)-size*advance)>=.001)fprintf(stderr,"SHY caret dir=%u pair=%u form=%u marker=%u pass=%u size=%g glyph=%u advance=%g expected=%g\n",direction,pair,form,control,pass,size,i,b->x-a->x,size*advance);
                CHECK(fabs(a->y-b->y)<.001 && fabs(fabs(b->x-a->x)-size*advance)<.001);
                for(unsigned side=0;side<2;side++){
                    xui_doc_position_t hit={.iSize=sizeof(hit)};xui_doc_rect_t hit_rect;
                    CHECK(xuiDocumentRendererHitTest(r,a->x+(b->x-a->x)*(side?.8:.2),a->y+a->height*.5,&hit)==XUI_OK &&
                        xuiDocumentRendererGetCaretRect(r,&hit,&hit_rect)==XUI_OK && fabs(hit_rect.x-(side?b->x:a->x))<.001 && fabs(hit_rect.y-a->y)<.001);
                }
            }
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK && p->drawClipSet(p,draw,clip)==XUI_OK);
            for(unsigned i=0;i<2;i++)literal(p,draw,font,
#ifdef TEST_NO_HB
                base+i,
#else
                base+(i?(selected?1:3):terminal?4:2),
#endif
                ox+fmin(rects[i][0].x,rects[i][1].x),rects[i][0].y,colors[form==1?2*i:0]);
            if(selected){
                unsigned source=offsets[hyphen_part],local=form==1?0:source;xui_doc_rect_t a,b;
                xui_doc_position_t start=position(snapshot,form,leaves[form==1?hyphen_part:0],local,source,XUI_DOC_AFTER);
                xui_doc_position_t end=position(snapshot,form,leaves[form==1?hyphen_part:0],local+2,source+2,XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(r,&start,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&end,&b)==XUI_OK && fabs(a.y-b.y)<.001 && fabs(fabs(a.x-b.x)-size*.15)<.001);
                literal(p,draw,font,0xe702,ox+fmin(a.x,b.x),a.y,colors[form==1?1:0]);
            }
            CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets,direction,form,3*pair+control,pass,size);
        }
        xuiDocumentSnapshotRelease(snapshot);xuiDocumentRendererRelease(r);
    }
}
#include "xui_document_shy_variant_cases.h"
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t p=original=xuiProxyXge();xui_context context;xui_font font;xui_surface targets[2];
    xui_surface_desc_t surface={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    p.textShape=capture_shape;p.drawText=capture_raw;p.drawTextSpans=capture_spans;p.drawTextShapeRangeSpans=capture_range;
#ifdef TEST_DRAW_TEXT_ONLY
    p.drawTextSpans=NULL;
#endif
#ifdef TEST_BASIC_RANGE
    p.textShapeRangeMeasure=NULL;p.drawTextShapeRangeSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK && p.fontLoadFile(&p,&font,"test/data/xge_shy_context_fixture.ttf",size,0)==XUI_OK &&
        xuiSetDefaultFont(context,font)==XUI_OK && p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
#ifdef TEST_NO_HB
    for(unsigned direction=0;direction<1;direction++)for(unsigned pair=0;pair<2;pair++)document_cases(&p,context,font,size,direction,pair,targets);
#else
    for(unsigned direction=0;direction<3;direction++)for(unsigned pair=0;pair<2;pair++)document_cases(&p,context,font,size,direction,pair,targets);
    displayed_text=NULL;document_multiple_shy(&p,context,font,size,targets);
    document_long_shy(context,font,size);
#endif
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);xuiDestroy(context);p.fontDestroy(&p,font);xgeQuit();return XGE_OK;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){float size=sizes[i];xge_desc_t desc={.iWidth=W,.iHeight=H,.sTitle="SHY complete spans and terminal variants",.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};
        CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&size)==XGE_OK);xgeUnit();}
#ifdef TEST_BASIC_RANGE
    printf("Native basic text RANGE: %u exact full RGBA cases, both cached range callbacks absent, full base/selected pattern and borrowed glyphs through ordinary drawing passed\n",cases);
#else
#ifdef TEST_NO_HB
    printf("Native no-HB SHY spans: %u exact full RGBA cases, literal cmap advances, selected/rejected markers, Rich whole/split and Markdown visual/source/live, source/caret/hit/cache/colours/clip/fractional X/released Document and zero raw draw/reshape passed\n",cases);
#else
    printf("Native SHY spans: %u exact full RGBA cases, Greek LTR/RLO and Hebrew calt base and narrow/wide terminal substitutions; selected/unselected markers, actual candidate acceptance/rejection, Rich whole/split colours and Markdown visual/source/live, caret/hit/source mapping/cache reuse/colour edits/clip/fractional X/released Document and zero raw draw/reshape passed\n",cases);
#endif
#endif
    return 0;
}
