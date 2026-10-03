/* Retained display context is independent of temporary break/Bidi streams. */
#include "xge_unicode_script.h"
#include "xui_text_display.h"
static uint32_t doc_context_decode(const void* source,size_t bytes,size_t* at)
{
    const unsigned char* text=source;size_t start=*at;unsigned cp,n,i;
    if(start>=bytes)return UINT32_MAX;
    if(text[start]<0x80){(*at)++;return text[start];}
    if(text[start]>=0xc2 && text[start]<=0xdf){n=2;cp=text[start]&31;}
    else if(text[start]>=0xe0 && text[start]<=0xef){n=3;cp=text[start]&15;}
    else if(text[start]>=0xf0 && text[start]<=0xf4){n=4;cp=text[start]&7;}
    else return UINT32_MAX;
    if(n>bytes-start)return UINT32_MAX;
    for(i=1;i<n;i++){if((text[start+i]&0xc0)!=0x80)return UINT32_MAX;cp=(cp<<6)|(text[start+i]&63);}
    if((n==3 && cp<0x800) || (n==4 && cp<0x10000) || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff))return UINT32_MAX;
    *at+=n;return cp;
}
static void doc_context_free(doc_shape_context* context)
{
    if(!context)return;
    free(context->hyphen_text);free(context->text);free(context->source_map);free(context->scripts);free(context);
}
static void doc_context_count_copy(doc_shape_context* context,uint64_t bytes)
{
    context->hyphen_copied=bytes>UINT64_MAX-context->hyphen_copied?UINT64_MAX:context->hyphen_copied+bytes;
}
int doc_context_hyphen_borrow(doc_shape_context* context,uint32_t insertion,doc_hyphen_lease* out)
{
    doc_hyphen_lease lease={0};uint32_t old;
    if(!context || !out || !context->hyphen_text || insertion>context->bytes ||
        (insertion<context->bytes && ((unsigned char)context->text[insertion]&0xc0)==0x80))
        return XUI_ERROR_INVALID_ARGUMENT;
    old=context->hyphen_at;
    if(context->hyphen_borrows && old!=insertion){
        lease.owned=malloc((size_t)context->bytes+2);
        if(!lease.owned)return XUI_ERROR_OUT_OF_MEMORY;
        memcpy(lease.owned,context->text,insertion);lease.owned[insertion]='-';
        memcpy(lease.owned+insertion+1,context->text+insertion,(size_t)context->bytes-insertion+1);
        doc_context_count_copy(context,(uint64_t)context->bytes+1);
        lease.text=lease.owned;
    }else{
        if(context->hyphen_borrows==UINT32_MAX)return XUI_DOC_ERROR_LIMIT;
        if(insertion>old)memmove(context->hyphen_text+old,context->hyphen_text+old+1,insertion-old);
        else if(insertion<old)memmove(context->hyphen_text+insertion+1,context->hyphen_text+insertion,old-insertion);
        doc_context_count_copy(context,insertion>old?insertion-old:old-insertion);
        context->hyphen_text[insertion]='-';context->hyphen_at=insertion;context->hyphen_borrows++;
        lease.context=context;lease.text=context->hyphen_text;
    }
    *out=lease;return XUI_OK;
}
void doc_context_hyphen_release(doc_hyphen_lease* lease)
{
    if(!lease)return;
    if(lease->context && lease->context->hyphen_borrows)lease->context->hyphen_borrows--;
    free(lease->owned);memset(lease,0,sizeof(*lease));
}
static uint32_t doc_context_offset(const doc_shape_context* context,size_t source)
{
    return context->source_map?context->source_map[source]:(uint32_t)source;
}
static uint32_t doc_context_script(const doc_shape_context* context,size_t source,int hyphen)
{
    uint32_t offset;
    if(!context || !context->scripts)return 0;
    offset=doc_context_offset(context,source);
    if(hyphen && offset)offset--;
    return __xgeScriptTag(context->scripts[offset<context->bytes?offset:context->bytes-1]);
}
static int doc_context_acquire(doc_render_block* block,uint64_t node,const char* source,
    size_t bytes,const doc_paragraph_span* spans,size_t count,int context_supported,doc_shape_context** out)
{
    doc_shape_context* context=NULL;size_t at=0,display=0,span=0,i;int result,has_shy=0;
    if(!block || !source || !out || (count && !spans))return XUI_ERROR_INVALID_ARGUMENT;
    if(bytes>INT_MAX)return XUI_DOC_ERROR_LIMIT;
    for(i=0;i<count;i++)if(spans[i].start>spans[i].end || spans[i].end>bytes ||
        (i && spans[i].start<spans[i-1].end) ||
        ((spans[i].kind==XUI_DOC_SOFT_BREAK || spans[i].kind==XUI_DOC_HARD_BREAK) && spans[i].end-spans[i].start!=3))
        return XUI_ERROR_INVALID_ARGUMENT;
    if(block->reflow){
        if(block->context_cursor>=block->context_count)return XUI_DOC_ERROR_STALE;
        context=block->contexts[block->context_cursor++];
        if(context->node!=node || context->source_bytes!=bytes || context->context_supported!=!!context_supported)return XUI_DOC_ERROR_STALE;
        *out=context;return XUI_OK;
    }
    context=calloc(1,sizeof(*context));if(!context)return XUI_ERROR_OUT_OF_MEMORY;
    context->node=node;context->source_bytes=bytes;context->context_supported=!!context_supported;
    context->text=malloc(bytes+1);
    /* Identity paragraphs never allocate a boundary map. On the first
     * changed offset, initialize its identity prefix and continue linearly. */
    if(!context->text){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
    while(at<bytes){
        size_t end=at;uint32_t cp;int length;
        while(span<count && spans[span].end<=at)span++;
        if(span<count && spans[span].start==at && spans[span].kind==XUI_DOC_SOFT_BREAK){
            end=(size_t)spans[span].end;context->text[display]=' ';length=1;
        }else if(span<count && spans[span].start==at && spans[span].kind==XUI_DOC_HARD_BREAK){
            end=(size_t)spans[span].end;memcpy(context->text+display,"\xe2\x80\xa8",3);length=3;
        }else{
            cp=doc_context_decode(source,bytes,&end);
            if(cp==UINT32_MAX){result=XUI_ERROR_INVALID_ARGUMENT;goto fail;}
            if(cp==0xad)has_shy=1;
            length=__xuiTextCopyDisplay(source+at,(int)(end-at),NULL);
            if(length<0){result=length;goto fail;}
            if((size_t)length==end-at)memcpy(context->text+display,source+at,(size_t)length);
            else __xuiTextCopyDisplay(source+at,(int)(end-at),context->text+display);
        }
        if(length!=(int)(end-at) && !context->source_map){
            if(bytes+1>SIZE_MAX/sizeof(*context->source_map)){result=XUI_DOC_ERROR_LIMIT;goto fail;}
            context->source_map=malloc((bytes+1)*sizeof(*context->source_map));
            if(!context->source_map){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
            for(i=0;i<=at;i++)context->source_map[i]=(uint32_t)i;
        }
        if(context->source_map)for(i=at;i<end;i++)context->source_map[i]=(uint32_t)(display+(length==(int)(end-at)?i-at:0));
        display+=(unsigned)length;if(context->source_map)context->source_map[end]=(uint32_t)display;at=end;
    }
    if(context->source_map)context->source_map[bytes]=(uint32_t)display;
    context->text[display]=0;context->bytes=(uint32_t)display;
    for(i=0;i<display;i++)if((unsigned char)context->text[i]>=128)break;
    /* ASCII punctuation in its own styled carrier needs the paragraph's
     * Latin script too. A single ASCII carrier is already shaped together,
     * so preserve its allocation-free script fast path. */
    if(display && (i<display || count>1)){
        context->scripts=malloc(display);if(!context->scripts){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        if(!__xgeScriptMap((const unsigned char*)context->text,display,context->scripts,doc_context_decode)){
            result=XUI_ERROR_INVALID_ARGUMENT;goto fail;
        }
    }
    if(has_shy && context_supported){
        if(display>=INT_MAX){result=XUI_DOC_ERROR_LIMIT;goto fail;}
        context->hyphen_text=malloc(display+2);if(!context->hyphen_text){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        context->hyphen_text[0]='-';memcpy(context->hyphen_text+1,context->text,display+1);
        context->hyphen_copied=display+1;
    }
    result=doc_render_reserve((void**)&block->contexts,&block->context_capacity,
        block->context_count+1,sizeof(*block->contexts));if(result!=XUI_OK)goto fail;
    block->contexts[block->context_count++]=context;block->context_cursor++;
    *out=context;return XUI_OK;
fail:
    doc_context_free(context);return result;
}
