/* HB's bounded pre/post context can contain only transparent scalars and
 * miss the neighbor that determines joining. Its Arabic and USE joining
 * state machines need the nearest non-T scalar, including ZWJ/ZWNJ.
 * Seed that context using public buffer APIs, then add only the original
 * item with its original absolute clusters. No paragraph copy or vendor
 * mutation is needed. Other buffer-context consumers only test presence.
 * Keep the usual add_utf8 path when its five-scalar window is sufficient. */
#include "xge_unicode_joining.h"
#ifndef XGE_OT_CONTEXT_VISIT
#define XGE_OT_CONTEXT_VISIT() ((void)0)
#define XGE_OT_CONTEXT_VISIT_LOCAL
#endif
static int __xgeOtBufferAddContext(hb_buffer_t* buffer, const xge_text_shape_desc_t* desc,
    const char* begin, const char* end)
{
    const char *text=desc->sContext,*limit=text+desc->iContextSize;
    unsigned offset=(unsigned)(desc->iContextOffset+(begin-desc->sText));
    int bytes=(int)(end-begin),result;
    const char *pre=text+offset,*post=pre+bytes,*pre_seed=pre,*post_seed=post,*scan;
    size_t pre_count=0,post_count=0;
    int pre_found=0,post_found=0;
    uint32_t cp;
    /* Context validity is the caller's contract; decode the immediate
     * neighbors defensively without rescanning unrelated paragraph text. */
    while(pre>text){
        const char *scalar=pre-1,*next;
        while(scalar>text && ((unsigned char)*scalar&0xc0)==0x80)scalar--;
        next=scalar;XGE_OT_CONTEXT_VISIT();pre_count++;
        result=__xgeTextUTF8DecodeBounded(&next,pre,&cp);
        if(result!=XGE_OK || next!=pre)return XGE_ERROR_INVALID_ARGUMENT;
        if(!__xgeJoiningTransparent(cp)){pre_seed=pre;pre_found=1;break;}
        pre=scalar;
    }
    while(post<limit){
        const char* scalar=post;
        XGE_OT_CONTEXT_VISIT();post_count++;
        result=__xgeTextUTF8DecodeBounded(&post,limit,&cp);
        if(result!=XGE_OK)return result;
        if(!__xgeJoiningTransparent(cp)){post_seed=scalar;post_found=1;break;}
    }
    if(!bytes || (!(pre_found && pre_count>5) && !(post_found && post_count>5))){
        hb_buffer_add_utf8(buffer,text,desc->iContextSize,offset,bytes);
    }else{
        unsigned seed=(unsigned)((pre_found && pre_count>5 ? pre_seed : text+offset)-text);
        /* An empty add installs pre-context, as supported by HB for split
         * context/text calls. hb_buffer_add preserves it while clearing
         * post-context. The last empty add restores post-context. */
        hb_buffer_add_utf8(buffer,text,desc->iContextSize,seed,0);
        scan=begin;
        while(scan<end){
            const char* scalar=scan;
            result=__xgeTextUTF8DecodeBounded(&scan,end,&cp);
            if(result!=XGE_OK)return result;
            hb_buffer_add(buffer,cp,offset+(unsigned)(scalar-begin));
        }
        seed=(unsigned)((post_found && post_count>5 ? post_seed : text+offset+bytes)-text);
        hb_buffer_add_utf8(buffer,text,desc->iContextSize,seed,0);
    }
    return hb_buffer_allocation_successful(buffer) ? XGE_OK : XGE_ERROR_OUT_OF_MEMORY;
}
#ifdef XGE_OT_CONTEXT_VISIT_LOCAL
#undef XGE_OT_CONTEXT_VISIT_LOCAL
#undef XGE_OT_CONTEXT_VISIT
#endif
