/* An owned glyph result keeps the XUI font carrier alive, including its
 * fallback carrier. XGE's run holds native references but borrows their struct
 * addresses; freeing the wrapper before the run would leave dangling pointers. */
typedef struct xui_xge_shape_paint {
    xge_glyph_run_t run;
    xui_font font;
} xui_xge_shape_paint;
static void __xuiProxyXgeShapePaintFree(void* owned)
{
    xui_xge_shape_paint* paint=owned;
    xui_font font=paint->font;
    xgeGlyphRunFree(&paint->run);
    if(--font->iPaintRefs==0 && font->bDestroyPending)__xuiProxyXgeFontRelease(font);
    xrtFree(paint);
}
static int __xuiProxyXgeShapeGlyphAt(const xge_glyph_run_t* run,int offset)
{
    int lo=0,hi=run->iGlyphCount;
    while(lo<hi){int mid=lo+(hi-lo)/2;
        if(run->pGlyphs[mid].iCluster<(uint32_t)offset)lo=mid+1;else hi=mid;
    }
    return lo;
}
static int __xuiProxyXgeShapeRange(const xui_text_shape_t* shape,int start,int end,
    const xui_xge_shape_paint** output,int* first_out,int* last_out)
{
    const xui_xge_shape_paint* paint;
    int first,last,i;
    if(!shape || shape->iSize<sizeof(*shape) || start<0 || end<start || end>shape->iTextSize)
        return XUI_ERROR_INVALID_ARGUMENT;
    if(!shape->pPaint || shape->paintFree!=__xuiProxyXgeShapePaintFree)return XUI_ERROR_UNSUPPORTED;
    paint=shape->pPaint;
    if(end>paint->run.iTextSize)return XUI_ERROR_INVALID_ARGUMENT;
    first=__xuiProxyXgeShapeGlyphAt(&paint->run,start);
    last=__xuiProxyXgeShapeGlyphAt(&paint->run,end);
    /* A caret can divide a ligature but the underlying glyph cannot be
     * submitted twice on separate rows. Reject before any GPU/clip mutation. */
    if(start!=end && (first==last || paint->run.pGlyphs[first].iCluster!=(uint32_t)start ||
        paint->run.pGlyphs[last-1].iClusterEnd!=(uint32_t)end))return XUI_ERROR_UNSUPPORTED;
    for(i=first;i<last;i++)if(paint->run.pGlyphs[i].iFlags & XGE_GLYPH_POSITION_LINE_BREAK)
        return XUI_ERROR_UNSUPPORTED;
    *output=paint;*first_out=first;*last_out=last;return XUI_OK;
}
/* Glyphs remain in logical cluster order. An RTL subset keeps the whole run's
 * visual positions, so its last cluster supplies the subset's left origin.
 * Use the cluster's first glyph, not a mark's position within that cluster. */
static float __xuiProxyXgeShapeRangeOrigin(const xge_glyph_run_t* run,int first,int last)
{
    if(!(run->iFlags & XGE_TEXT_SHAPE_RTL) || first==last ||
        (first==0 && last==run->iGlyphCount))return 0;
    return run->pGlyphs[__xuiProxyXgeShapeGlyphAt(run,(int)run->pGlyphs[last-1].iCluster)].fVisualX;
}
static int __xuiProxyXgeTextShapeRangeMeasure(xui_proxy proxy,const xui_text_shape_t* shape,
    int start,int end,xui_vec2_t* size)
{
    const xui_xge_shape_paint* paint;int first,last,i,result;float pen=0,right=0;
    if(!size)return XUI_ERROR_INVALID_ARGUMENT;
    memset(size,0,sizeof(*size));
    if(!proxy)return XUI_ERROR_INVALID_ARGUMENT;
    result=__xuiProxyXgeShapeRange(shape,start,end,&paint,&first,&last);
    if(result!=XUI_OK)return result;
    if((paint->run.iFlags & XGE_TEXT_SHAPE_RTL) && start==0 && end==paint->run.iTextSize){
        size->fX=paint->run.fWidth;size->fY=paint->run.fHeight;return XUI_OK;
    }
    float origin=__xuiProxyXgeShapeRangeOrigin(&paint->run,first,last);
    for(i=first;i<last;i++){
        const xge_glyph_position_t* glyph=&paint->run.pGlyphs[i];
        float edge,x=(paint->run.iFlags & XGE_TEXT_SHAPE_RTL)?glyph->fVisualX-origin:pen;
        if(glyph->iItemKind==XGE_TEXT_ITEM_EMOJI)edge=x+glyph->fOffsetX+glyph->fEmojiWidth;
        else{
            xge_glyph_metrics_t metrics;
            result=xgeFontGlyphGetByIndex(glyph->pFont,glyph->iGlyph,&metrics);
            if(result!=XGE_OK)return result;
            edge=x+glyph->fOffsetX+metrics.fX1;
        }
        if(edge>right)right=edge;
        pen+=glyph->fAdvanceX;
    }
    size->fX=fmaxf(pen,right);size->fY=paint->run.fHeight;return XUI_OK;
}
static int __xuiProxyXgeDrawTextShapeRangeSpansClip(xui_proxy proxy,xui_draw_context draw,
    const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,
    uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count,xui_rect_t clip_rect)
{
    const xui_xge_shape_paint* paint;xge_glyph_run_t view;
    xge_text_paint_span_t small[16],*native=small;
    xge_rect_t saved={0},clip;
    int first,last,i,had_clip=0,clipped=0,result;
    float origin;
    if(!proxy || !(offset>=-.5f && offset<=.5f) || count<0 || (count && !spans))return XUI_ERROR_INVALID_ARGUMENT;
    result=__xuiProxyXgeShapeRange(shape,start,end,&paint,&first,&last);
    if(result!=XUI_OK)return result;
    if(flags & ~(XUI_TEXT_CLIP|XUI_TEXT_UNDERLINE|XUI_TEXT_RTL))return XUI_ERROR_UNSUPPORTED;
    for(i=0;i<count;i++)if(spans[i].iSize<sizeof(*spans) || spans[i].iStart<0 ||
        spans[i].iEnd<spans[i].iStart || spans[i].iEnd>shape->iTextSize ||
        (i && spans[i].iStart<spans[i-1].iEnd))return XUI_ERROR_INVALID_ARGUMENT;
    if(!__xuiProxyXgeDrawValid(draw))return XUI_ERROR_INVALID_ARGUMENT;
    if(start==end || rect.fW<=0 || rect.fH<=0 || !XGE_COLOR_GET_A(color))return XUI_OK;
    view=paint->run;view.pGlyphs+=first;view.iGlyphCount=last-first;
    origin=__xuiProxyXgeShapeRangeOrigin(&paint->run,first,last);
    if(count>(int)(sizeof(small)/sizeof(*small))){
        if((size_t)count>SIZE_MAX/sizeof(*native))return XUI_ERROR_OUT_OF_MEMORY;
        native=xrtMalloc((size_t)count*sizeof(*native));if(!native)return XUI_ERROR_OUT_OF_MEMORY;
    }
    for(i=0;i<count;i++)native[i]=(xge_text_paint_span_t){sizeof(*native),spans[i].iStart,spans[i].iEnd,spans[i].iColor};
    /* The range view borrows glyph/font storage only for this draw. Preserve
     * original cluster/caret offsets for colour spans, and rebase the RTL draw
     * origin without mutating retained glyphs or snapping the shifted origin. */
    if(flags & XUI_TEXT_CLIP){
        float left,right,top,bottom;
        saved=xgeClipGet();had_clip=saved.fW>0 && saved.fH>0;
        clip=__xuiProxyXgeRect(clip_rect);
        if(had_clip){
            left=fmaxf(clip.fX,saved.fX);right=fminf(clip.fX+clip.fW,saved.fX+saved.fW);
            top=fmaxf(clip.fY,saved.fY);bottom=fminf(clip.fY+clip.fH,saved.fY+saved.fH);
            clip=(xge_rect_t){left,top,right-left,bottom-top};
        }
        if(clip.fW<=0 || clip.fH<=0){if(native!=small)xrtFree(native);return XUI_OK;}
        xgeFlush();xgeClipSet(clip);clipped=1;
    }else xgeFlush();
    xgeGlyphRunDrawSpans(&view,(float)rect.fX+offset-origin,rect.fY,color,
        XGE_DRAW_SCREEN_SPACE | (offset!=0 || origin!=0?XGE_DRAW_TEXT_SUBPIXEL_X:0),native,count);
    if(flags & XUI_TEXT_UNDERLINE){
        xge_font_metrics_t metrics;float width=0;
        for(i=0;i<view.iGlyphCount;i++)width+=view.pGlyphs[i].fAdvanceX;
        if(xgeFontGetMetrics(&paint->font->tFont,&metrics)==XGE_OK){
            float y=rect.fY+view.fAscent+metrics.fUnderlinePosition;
            xgeShapeLinePx((float)rect.fX+offset,y,(float)rect.fX+offset+width,y,metrics.fUnderlineThickness,color);
        }
    }
    xgeFlush();
    if(clipped){if(had_clip)xgeClipSet(saved);else xgeClipClear();}
    if(native!=small)xrtFree(native);
    __xuiProxyXgeDrawMarkDirty(draw);return XUI_OK;
}
static int __xuiProxyXgeDrawTextShapeRangeSpans(xui_proxy proxy,xui_draw_context draw,
    const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,
    uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{
    return __xuiProxyXgeDrawTextShapeRangeSpansClip(proxy,draw,shape,start,end,rect,color,flags,offset,spans,count,rect);
}
static int __xuiProxyXgeDrawTextShapeRange(xui_proxy proxy,xui_draw_context draw,
    const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    return __xuiProxyXgeDrawTextShapeRangeSpans(proxy,draw,shape,start,end,rect,color,flags,0,NULL,0);
}
