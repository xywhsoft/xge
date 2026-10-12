/* A selected marker changes the complete shaping input, including glyphs on
 * later rows. Context and glyph owners are immutable until all rows retire. */
static int doc_layout_variant_marker(const doc_render_block* block,size_t at,size_t first,
    const uint8_t* selected,size_t* group)
{
    if(at>=first)return selected[at-first]!=0;
    if(!(block->fragments[at].flags & DOC_HYPHEN_USED))return 0;
    while(*group<block->paint_group_count && block->paint_groups[*group].end_fragment<=at)(*group)++;
    return *group<block->paint_group_count && block->paint_groups[*group].first_fragment<=at &&
        block->paint_groups[*group].context==block->layout_context;
}
static int doc_layout_variant_build(xui_document_renderer r,const doc_render_block* block,
    size_t first,size_t end,const uint8_t* selected,doc_render_paint_variant** output)
{
    doc_shape_context* context=block->layout_context;
    doc_render_paint_variant* variant=NULL;
    uint32_t* positions=NULL;size_t count=0,group=0,at,i,old=0,written=0,markers=0;
    int result=XUI_OK;
    *output=NULL;
    if(!context || first>=end)return XUI_ERROR_INVALID_STATE;
    for(i=0;i<end;i++)if(doc_layout_variant_marker(block,i,first,selected,&group))count++;
    if(!count)return XUI_OK;
    if(count>(size_t)INT_MAX-context->bytes)return XUI_DOC_ERROR_LIMIT;
    variant=calloc(1,sizeof(*variant));positions=calloc(count,sizeof(*positions));
    if(!variant || !positions){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
    variant->context=context;variant->first_fragment=first;variant->end_fragment=end;
    variant->bytes=context->bytes+(uint32_t)count;
    variant->text=malloc((size_t)variant->bytes+1);
    variant->hyphens=malloc(end-first);
    variant->offsets=calloc(end-first+1,sizeof(*variant->offsets));
    variant->span_map=calloc(end-first,sizeof(*variant->span_map));
    if(!variant->text || !variant->hyphens || !variant->offsets || !variant->span_map){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
    memcpy(variant->hyphens,selected,end-first);group=0;
    for(i=0;i<end;i++)if(doc_layout_variant_marker(block,i,first,selected,&group)){
        size_t position=block->fragments[i].bidi_start;
        if(position>=context->source_bytes || (markers && position<=positions[markers-1])){result=XUI_ERROR_INVALID_STATE;goto fail;}
        positions[markers++]=(uint32_t)position;
    }
    for(i=0;i<count;i++){
        uint32_t position=doc_context_offset(context,positions[i]);
        if(position<old || position>context->bytes){result=XUI_ERROR_INVALID_STATE;goto fail;}
        memcpy(variant->text+written,context->text+old,position-old);written+=position-old;
        variant->text[written++]='-';old=position;
    }
    memcpy(variant->text+written,context->text+old,(size_t)context->bytes-old+1);
    markers=0;
    for(i=first;i<end;i++){
        const doc_fragment* fragment=&block->fragments[i];
        while(markers<count && positions[markers]<fragment->bidi_start)markers++;
        variant->offsets[i-first]=doc_context_offset(context,fragment->bidi_start)+(uint32_t)markers;
        while(markers<count && positions[markers]<fragment->bidi_end)markers++;
        variant->offsets[i+1-first]=doc_context_offset(context,fragment->bidi_end)+(uint32_t)markers;
    }
    if(block->layout_bidi){
        const char* source=xuiInternalTextBidiText(block->layout_bidi);
        size_t bytes=xuiInternalTextBidiTextBytes(block->layout_bidi);char* text;
        if(count>bytes){result=XUI_ERROR_INVALID_STATE;goto fail;}
        text=malloc(bytes-count+1);
        variant->bidi_offsets=calloc(end-first+1,sizeof(*variant->bidi_offsets));
        if(!text || !variant->bidi_offsets){free(text);result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        old=written=0;
        for(i=0;i<count;i++){
            size_t position=positions[i];
            if(position<old || position+2>bytes || memcmp(source+position,"\xc2\xad",2)){
                free(text);result=XUI_ERROR_INVALID_STATE;goto fail;
            }
            memcpy(text+written,source+old,position-old);written+=position-old;
            text[written++]='-';old=position+2;
        }
        memcpy(text+written,source+old,bytes-old+1);
        result=xuiInternalTextBidiCreate(text,bytes-count,XUI_BIDI_AUTO_LTR,&variant->bidi);free(text);
        if(result!=XUI_OK)goto fail;
        markers=0;
        for(i=first;i<end;i++){
            const doc_fragment* fragment=&block->fragments[i];
            while(markers<count && positions[markers]<fragment->bidi_start)markers++;
            variant->bidi_offsets[i-first]=(uint32_t)(fragment->bidi_start-markers);
            while(markers<count && positions[markers]<fragment->bidi_end)markers++;
            variant->bidi_offsets[i+1-first]=(uint32_t)(fragment->bidi_end-markers);
        }
    }
    for(i=first;i<end;i++){
        doc_fragment* fragment=&block->fragments[i];
        fragment->bidi_level=0;
        if(variant->bidi){
            size_t offset=variant->bidi_offsets[i-first];
            if(offset<xuiInternalTextBidiTextBytes(variant->bidi))result=xuiInternalTextBidiLevel(variant->bidi,offset,&fragment->bidi_level);
            else {xui_bidi_paragraph_t paragraph;size_t paragraphs=xuiInternalTextBidiParagraphCount(variant->bidi);
                result=xuiInternalTextBidiParagraph(variant->bidi,paragraphs-1,&paragraph);fragment->bidi_level=paragraph.base;}
            if(result!=XUI_OK)goto fail;
        }
    }
    at=first;
    while(at<end){
        doc_render_paint_group input={0};doc_render_paint_seed* span;int match;
        size_t stop,j;
        if(!doc_layout_paintable(block,&block->fragments[at])){at++;continue;}
        stop=doc_layout_shape_stop(block,at,end,SIZE_MAX,variant);
        result=doc_render_reserve((void**)&variant->spans,&variant->span_capacity,variant->span_count+1,sizeof(*variant->spans));
        if(result!=XUI_OK)goto fail;
        span=&variant->spans[variant->span_count++];memset(span,0,sizeof(*span));
        span->first_fragment=at;span->end_fragment=stop;
        span->widths=calloc(stop-at,sizeof(*span->widths));span->offsets=calloc(stop-at+1,sizeof(*span->offsets));
        if(!span->widths || !span->offsets){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        result=doc_layout_shape_paint(r,block,at,stop,span->widths,at,&input,&match,SIZE_MAX,&span->shape,variant);
        span->font=input.font;span->vertical_marks=input.vertical_marks;span->bidi_level=input.bidi_level;
        free(input.text);free(input.display_ends);
        if(result!=XUI_OK || !match){if(result==XUI_OK)result=XUI_ERROR_UNSUPPORTED;goto fail;}
        for(j=at;j<=stop;j++)span->offsets[j-at]=variant->offsets[j-first]-variant->offsets[at-first];
        for(j=at;j<stop;j++)variant->span_map[j-first]=variant->span_count;
        at=stop;
    }
    free(positions);*output=variant;return XUI_OK;
fail:
    free(positions);doc_layout_variant_free(variant);return result;
}
/* Use the same range and direction as publication. A ligature interior or an
 * L1 direction change gets an owned row shape with the full variant context. */
static int doc_layout_variant_row(xui_document_renderer r,const doc_render_block* block,
    size_t first,size_t end,size_t hyphen,doc_render_paint_variant* variant,
    double* widths,size_t base,doc_render_paint_group* pending,size_t* count,double* total)
{
    size_t at=first;int result;
    *total=0;
    result=doc_layout_bidi_levels(block,first,end,hyphen,variant);
    if(result!=XUI_OK)return result;
    while(at<end){
        doc_render_paint_group local={0};doc_render_paint_group* row=pending?&pending[(*count)++]:&local;
        size_t stop,index,j;int match=1;
        if(!doc_layout_paintable(block,&block->fragments[at])){
            if(pending)(*count)--;
            widths[at-base]=block->fragments[at].width;*total+=widths[at-base];at++;continue;
        }
        stop=doc_layout_shape_stop(block,at,end,hyphen,variant);index=variant->span_map[at-variant->first_fragment];
        result=XUI_ERROR_UNSUPPORTED;
        if(index){
            const doc_render_paint_seed* span=&variant->spans[index-1];
            if(stop<=span->end_fragment && span->bidi_level==block->fragments[at].bidi_level){
                uint32_t start=span->offsets[at-span->first_fragment],finish=span->offsets[stop-span->first_fragment];
                xui_vec2_t measured={0};
                result=start==finish && r->proxy->textShapeRangeMeasure && r->proxy->drawTextShapeRangeSpans?XUI_OK:span->shape.pPaint && r->proxy->textShapeRangeMeasure && r->proxy->drawTextShapeRangeSpans?
                    r->proxy->textShapeRangeMeasure(r->proxy,&span->shape,(int)start,(int)finish,&measured):XUI_ERROR_UNSUPPORTED;
                if(result==XUI_ERROR_UNSUPPORTED){
                    xui_text_item_t item;doc_hyphen_lease lease;xui_text_shape_t selected={0};
                    result=doc_layout_paint_input(r,block,at,stop,row,hyphen,variant);
                    if(result!=XUI_OK)goto row_done;
                    if(row->input_caps & XUI_PROXY_CAP_TEXT_RANGE){
                        row->range_input=1;row->variant_span=index;row->shape_start=start;row->shape_end=finish;
                        result=doc_render_group_item(row,block->runs[doc_fragment_paint_run(&block->fragments[row->origin_fragment])].attrs.sLanguage,&item,&lease);
                        if(result==XUI_OK){result=xuiTextShape(r->context,&item,&selected);doc_context_hyphen_release(&lease);}
                        if(result==XUI_OK){measured.fX=selected.fWidth;measured.fY=selected.fHeight;}
                        xuiTextShapeFree(&selected);
                        if(result!=XUI_OK){row->range_input=0;row->variant_span=0;}
                    }else result=XUI_ERROR_UNSUPPORTED;
                }
                if(result==XUI_OK){
                    double advance=0;
                    if(!isfinite(measured.fX) || measured.fX<0 || !isfinite(measured.fY) || measured.fY<0)return XUI_ERROR_INVALID_STATE;
                    result=row->range_input?XUI_OK:doc_layout_paint_input(r,block,at,stop,row,hyphen,variant);
                    if(result!=XUI_OK)goto row_done;
                    memcpy(widths+at-base,span->widths+at-span->first_fragment,(stop-at)*sizeof(*widths));
                    for(j=at;j<stop;j++)advance+=widths[j-base];
                    row->paint_width=fmax(advance,measured.fX);row->line_height=span->shape.fLineHeight;row->ascent=span->shape.fAscent;
                    row->variant_span=index;row->shape_start=start;row->shape_end=finish;
                }
            }
        }
        if(result==XUI_ERROR_UNSUPPORTED){
            free(row->text);free(row->display_ends);memset(row,0,sizeof(*row));
            result=doc_layout_shape_paint(r,block,at,stop,widths,base,row,&match,hyphen,pending?&row->shape:NULL,variant);
        }
row_done:
        if(!pending){free(row->text);free(row->display_ends);xuiTextShapeFree(&row->shape);}
        if(result!=XUI_OK || !match)return result==XUI_OK?XUI_ERROR_UNSUPPORTED:result;
        for(j=at;j<stop;j++)*total+=widths[j-base];
        at=stop;
    }
    return XUI_OK;
}
