/* RANGE is an input contract for every ordinary text callback. It selects
 * glyphs after the full uniform item has been shaped; context alone cannot
 * reproduce substitutions whose input glyphs lie outside the selected row. */
static int __xuiProxyXgeRangeAcquire(xui_proxy proxy,const xui_text_item_t* input,
    xui_text_item_t* item,xui_text_shape_t* owned,const xui_text_shape_t** shape,xui_vec2_t* size)
{
    xui_text_item_t full;xui_proxy_caps_t caps={0};int result;
    const xui_xge_shape_paint* paint;int first,last;
    memset(owned,0,sizeof(*owned));*shape=NULL;memset(size,0,sizeof(*size));
    result=__xuiTextItemNormalize(input,item);if(result!=XUI_OK)return result;
    if(!proxy || !__xuiProxyXgeFontValid(item->pFont) || !(item->iFlags & XUI_TEXT_SHAPE_RANGE))
        return XUI_ERROR_INVALID_ARGUMENT;
    result=__xuiProxyXgeGetCaps(proxy,&caps);if(result!=XUI_OK)return result;
    if((item->sContext && !(caps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT)) ||
        (item->iScript && !(caps.iCaps & XUI_PROXY_CAP_TEXT_SCRIPT)) ||
        (item->sLanguage && !(caps.iCaps & XUI_PROXY_CAP_TEXT_LANGUAGE)) ||
        ((item->iFlags & XUI_TEXT_SHAPE_RTL) && !(caps.iCaps & XUI_PROXY_CAP_TEXT_RTL)))
        return XUI_ERROR_UNSUPPORTED;
    if(item->pShape){
        *shape=item->pShape;
        if(((*shape)->iFlags & ~(XUI_TEXT_SHAPE_RETAIN_PAINT)) !=
            (item->iFlags & ~(XUI_TEXT_SHAPE_RANGE|XUI_TEXT_SHAPE_RETAIN_PAINT)))
            return XUI_ERROR_INVALID_ARGUMENT;
        if(!(*shape)->pPaint || (*shape)->paintFree!=__xuiProxyXgeShapePaintFree)return XUI_ERROR_UNSUPPORTED;
        paint=(*shape)->pPaint;if(paint->font!=item->pFont)return XUI_ERROR_INVALID_ARGUMENT;
    }else{
        full=*item;full.iFlags=(full.iFlags & ~XUI_TEXT_SHAPE_RANGE)|XUI_TEXT_SHAPE_RETAIN_PAINT;
        full.iRangeStart=full.iRangeEnd=0;full.pShape=NULL;
        full.pPaintSpans=NULL;full.iPaintSpanCount=0;
        result=__xuiProxyXgeTextShape(proxy,&full,owned);if(result!=XUI_OK){xuiTextShapeFree(owned);return result;}
        *shape=owned;
    }
    if(item->iRangeStart==item->iRangeEnd){size->fY=(*shape)->fHeight;return XUI_OK;}
    result=__xuiProxyXgeShapeRange(*shape,item->iRangeStart,item->iRangeEnd,&paint,&first,&last);
    if(result==XUI_OK)result=__xuiProxyXgeTextShapeRangeMeasure(proxy,*shape,item->iRangeStart,item->iRangeEnd,size);
    if(result!=XUI_OK)xuiTextShapeFree(owned);
    return result;
}
static int __xuiProxyXgeTextRangeMeasure(xui_proxy proxy,const xui_text_item_t* input,xui_vec2_t* size)
{
    xui_text_item_t item;xui_text_shape_t owned;const xui_text_shape_t* shape;xui_vec2_t exact;int result;
    result=__xuiProxyXgeRangeAcquire(proxy,input,&item,&owned,&shape,&exact);
    if(result==XUI_OK){size->fX=(float)xuiInternalPixelCeil(exact.fX);size->fY=(float)xuiInternalPixelCeil(exact.fY);}
    xuiTextShapeFree(&owned);return result;
}
static int __xuiProxyXgeTextRangeShape(xui_proxy proxy,const xui_text_item_t* input,xui_text_shape_t* output)
{
    xui_text_item_t item;xui_text_shape_t owned;const xui_text_shape_t* shape;xui_vec2_t size;
    int result,i,clusters=0,carets=0,first_cluster=0,end_cluster=0,first_caret=0,end_caret=0,lo,hi;
    result=__xuiProxyXgeRangeAcquire(proxy,input,&item,&owned,&shape,&size);if(result!=XUI_OK)return result;
    output->iSize=sizeof(*output);output->iFlags=item.iFlags;output->iTextSize=item.iRangeEnd-item.iRangeStart;
    output->fWidth=size.fX;output->fHeight=size.fY;output->fAscent=shape->fAscent;
    output->fDescent=shape->fDescent;output->fLineHeight=shape->fLineHeight;
    if(item.iRangeStart!=item.iRangeEnd){
        lo=0;hi=shape->iClusterCount;
        while(lo<hi){int mid=lo+(hi-lo)/2;if(shape->pClusters[mid].iTextStart<item.iRangeStart)lo=mid+1;else hi=mid;}
        first_cluster=lo;end_cluster=lo;
        while(end_cluster<shape->iClusterCount && shape->pClusters[end_cluster].iTextEnd<=item.iRangeEnd)end_cluster++;
        clusters=end_cluster-first_cluster;
        lo=0;hi=shape->iCaretCount;
        while(lo<hi){int mid=lo+(hi-lo)/2;if(shape->pCarets[mid].iTextOffset<item.iRangeStart)lo=mid+1;else hi=mid;}
        first_caret=lo;end_caret=lo;
        while(end_caret<shape->iCaretCount && shape->pCarets[end_caret].iTextOffset<=item.iRangeEnd)end_caret++;
        carets=end_caret-first_caret;
    }
    if(clusters){output->pClusters=xrtCalloc((size_t)clusters,sizeof(*output->pClusters));if(!output->pClusters)goto oom;}
    if(carets){output->pCarets=xrtCalloc((size_t)carets,sizeof(*output->pCarets));if(!output->pCarets)goto oom;}
    for(i=first_cluster;i<end_cluster;i++){
        xui_text_cluster_t* cluster=&output->pClusters[output->iClusterCount++];*cluster=shape->pClusters[i];
        cluster->iTextStart-=item.iRangeStart;cluster->iTextEnd-=item.iRangeStart;
    }
    for(i=first_caret;i<end_caret;i++){
        xui_text_caret_t* caret=&output->pCarets[output->iCaretCount++];*caret=shape->pCarets[i];
        caret->iTextOffset-=item.iRangeStart;
    }
    xuiTextShapeFree(&owned);return XUI_OK;
oom:
    xuiTextShapeFree(output);xuiTextShapeFree(&owned);return XUI_ERROR_OUT_OF_MEMORY;
}
static int __xuiProxyXgeRangePaintFlags(uint32_t flags)
{
    if(flags & ~(XUI_TEXT_ALIGN_CENTER|XUI_TEXT_ALIGN_RIGHT|XUI_TEXT_ALIGN_MIDDLE|
        XUI_TEXT_ALIGN_BOTTOM|XUI_TEXT_CLIP|XUI_TEXT_UNDERLINE|XUI_TEXT_RTL))return XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
static int __xuiProxyXgeRangeDrawAcquired(xui_proxy proxy,xui_draw_context draw,
    const xui_text_item_t* item,const xui_text_shape_t* shape,xui_vec2_t size,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{
    xui_text_paint_span_t small[16],*mapped=small;xui_rect_t placed=rect;int i,result;
    if(count<0 || (count && !spans))return XUI_ERROR_INVALID_ARGUMENT;
    if(item->iPaintSpanCount){
        if(count)return XUI_ERROR_INVALID_ARGUMENT;
        spans=item->pPaintSpans;count=item->iPaintSpanCount;
    }
    result=__xuiProxyXgeRangePaintFlags(flags);if(result!=XUI_OK)return result;
    for(i=0;i<count;i++)if(spans[i].iSize<sizeof(*spans) || spans[i].iStart<0 ||
        spans[i].iEnd<spans[i].iStart || spans[i].iEnd>item->iRangeEnd-item->iRangeStart ||
        (i && spans[i].iStart<spans[i-1].iEnd))return XUI_ERROR_INVALID_ARGUMENT;
    if(!__xuiProxyXgeDrawValid(draw))return XUI_ERROR_INVALID_ARGUMENT;
    if(item->iRangeStart==item->iRangeEnd || rect.fW<=0 || rect.fH<=0 || !XGE_COLOR_GET_A(color))return XUI_OK;
    if(count>(int)(sizeof(small)/sizeof(*small))){
        if((size_t)count>SIZE_MAX/sizeof(*mapped))return XUI_ERROR_OUT_OF_MEMORY;
        mapped=xrtMalloc((size_t)count*sizeof(*mapped));if(!mapped)return XUI_ERROR_OUT_OF_MEMORY;
    }
    for(i=0;i<count;i++){mapped[i]=spans[i];mapped[i].iStart+=item->iRangeStart;mapped[i].iEnd+=item->iRangeStart;}
    if((flags & XUI_TEXT_ALIGN_RIGHT)==XUI_TEXT_ALIGN_RIGHT)placed.fX+=rect.fW-size.fX;
    else if(flags & XUI_TEXT_ALIGN_CENTER)placed.fX+=(rect.fW-size.fX)*.5f;
    if((flags & XUI_TEXT_ALIGN_BOTTOM)==XUI_TEXT_ALIGN_BOTTOM)placed.fY+=rect.fH-size.fY;
    else if(flags & XUI_TEXT_ALIGN_MIDDLE)placed.fY+=(rect.fH-size.fY)*.5f;
    flags &= XUI_TEXT_CLIP|XUI_TEXT_UNDERLINE;
    if(item->iFlags & XUI_TEXT_SHAPE_RTL)flags|=XUI_TEXT_RTL;
    result=__xuiProxyXgeDrawTextShapeRangeSpansClip(proxy,draw,shape,item->iRangeStart,item->iRangeEnd,
        placed,color,flags,item->fDrawOffsetX,mapped,count,rect);
    if(mapped!=small)xrtFree(mapped);
    return result;
}
static int __xuiProxyXgeTextRangeDraw(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* input,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{
    xui_text_item_t item;xui_text_shape_t owned;const xui_text_shape_t* shape;xui_vec2_t size;int result;
    result=__xuiProxyXgeRangeAcquire(proxy,input,&item,&owned,&shape,&size);
    if(result==XUI_OK)result=__xuiProxyXgeRangeDrawAcquired(proxy,draw,&item,shape,size,rect,color,flags,spans,count);
    xuiTextShapeFree(&owned);return result;
}
static int __xuiProxyXgeTextRangeSurfaceDraw(xui_proxy proxy,xui_surface target,const xui_text_item_t* input,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{
    xui_text_item_t item;xui_text_shape_t owned;const xui_text_shape_t* shape;xui_vec2_t size;
    xui_draw_context draw;int result,ended;
    result=__xuiProxyXgeRangeAcquire(proxy,input,&item,&owned,&shape,&size);
    if(result!=XUI_OK)return result;
    result=__xuiProxyXgeRangePaintFlags(flags);
    if(result==XUI_OK && item.iRangeStart!=item.iRangeEnd && rect.fW>0 && rect.fH>0 && XGE_COLOR_GET_A(color)){
        result=__xuiProxyXgeDrawBegin(proxy,&draw,target);
        if(result==XUI_OK){
            result=__xuiProxyXgeRangeDrawAcquired(proxy,draw,&item,shape,size,rect,color,flags,NULL,0);
            ended=__xuiProxyXgeDrawEnd(proxy,draw);if(result==XUI_OK)result=ended;
        }
    }
    xuiTextShapeFree(&owned);return result;
}
