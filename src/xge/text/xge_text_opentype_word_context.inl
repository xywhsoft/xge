/* Font coverage uses complete contextual graphemes/words; final shaping still
 * consumes only the caller's item. No context glyphs escape into its output. */
static int __xgeOtContextGraphemes(const xge_text_shape_desc_t* desc,
    xge_glyph_run_t* run,const char** out)
{
    xge_glyph_run_backend_t* backend=run->pBackend;
    if(!desc->iContextOffset && desc->iContextSize==run->iTextSize){
        *out=backend->pGraphemeBreaks;return XGE_OK;
    }
    if(!backend->pContextGraphemeBreaks){
        backend->pContextGraphemeBreaks=xrtMalloc((size_t)desc->iContextSize);
        if(!backend->pContextGraphemeBreaks)return XGE_ERROR_OUT_OF_MEMORY;
        backend->iContextGraphemeBytes=desc->iContextSize;
        __xgeOtGraphemesUtf8((const utf8_t*)desc->sContext,(size_t)desc->iContextSize,NULL,backend->pContextGraphemeBreaks);
    }
    *out=backend->pContextGraphemeBreaks;return XGE_OK;
}
static const char* __xgeOtContextHead(const char* text,const char* map,const char* at)
{
    while(at>text && map[at-text-1]!=GRAPHEMEBREAK_BREAK)at--;
    return at;
}
static const char* __xgeOtContextEnd(const char* text,const char* map,const char* at,const char* limit)
{
    at++;while(at<limit && map[at-text-1]!=GRAPHEMEBREAK_BREAK)at++;
    return at;
}
static int __xgeOtContextWordUnit(const xge_text_shape_desc_t* desc,xge_glyph_run_t* run,
    const char* begin,const char* end,hb_script_t wanted,int target,int* word)
{
    xge_glyph_run_backend_t* backend=run->pBackend;const char* scan=begin;uint32_t head=0;
    *word=1;
    while(scan<end){
        uint32_t cp;int result=__xgeTextUTF8DecodeBounded(&scan,end,&cp);
        if(result!=XGE_OK)return result;
        if(!head)head=cp;
        if(!__xgeOtWordScalar(cp)
#if XGE_ENABLE_EMOJI
            || ((desc->iFlags & XGE_TEXT_SHAPE_EMOJI) && __xgeEmojiMayStart(cp) && !__xgeOtIgnorable(cp))
#endif
        )*word=0;
    }
    if(!target && *word){
        /* Match the complete-item selector: the resolved paragraph map has
         * already applied Script_Extensions; an explicit script overrides it. */
        hb_script_t actual=desc->iScript?wanted:backend->pScriptMap?
            (hb_script_t)__xgeScriptTag(backend->pScriptMap[begin-desc->sContext]):
            hb_unicode_script(hb_unicode_funcs_get_default(),head);
        if(__xgeOtStrongScript(actual) && __xgeOtStrongScript(wanted) && actual!=wanted)*word=0;
    }
    return XGE_OK;
}
static int __xgeOtContextWord(const xge_text_shape_desc_t* desc,xge_glyph_run_t* run,
    const char* begin,const char** first,const char** last)
{
    const char *map,*text=desc->sContext,*limit=text+desc->iContextSize;
    const char *at=text+desc->iContextOffset+(begin-desc->sText),*head,*tail,*previous,*next;
    const char* scalar=begin;uint32_t cp;int word,result;
    result=__xgeOtContextGraphemes(desc,run,&map);if(result!=XGE_OK)return result;
    head=__xgeOtContextHead(text,map,at);tail=__xgeOtContextEnd(text,map,head,limit);
    result=__xgeTextUTF8DecodeBounded(&scalar,desc->sText+run->iTextSize,&cp);if(result!=XGE_OK)return result;
    hb_script_t script=__xgeOtItemScript(desc,run,begin,cp);
    result=__xgeOtContextWordUnit(desc,run,head,tail,script,1,&word);if(result!=XGE_OK)return result;
    if(word){
        while(head>text){
            previous=__xgeOtContextHead(text,map,head-1);
            result=__xgeOtContextWordUnit(desc,run,previous,head,script,0,&word);if(result!=XGE_OK)return result;
            if(!word)break;
            head=previous;
        }
        while(tail<limit){
            next=__xgeOtContextEnd(text,map,tail,limit);
            result=__xgeOtContextWordUnit(desc,run,tail,next,script,0,&word);if(result!=XGE_OK)return result;
            if(!word)break;
            tail=next;
        }
    }
    *first=head;*last=tail;return XGE_OK;
}
static int __xgeOtPickContextFont(const xge_text_shape_desc_t* desc,xge_glyph_run_t* run,
    const char* begin,const char* word_begin,const char* word_end,const char* item_end,xge_font* out)
{
    xge_text_shape_desc_t context=*desc;context.sText=desc->sContext;
    context.iTextSize=desc->iContextSize;context.iContextOffset=0;
    *out=NULL;
    xge_font candidate=desc->pFont;
    for(unsigned depth=0;candidate && depth<32;depth++,candidate=candidate->pFallback){
        int covered,result=__xgeOtFontCovers(&context,run,candidate,word_begin,word_end,&covered);
        if(result!=XGE_OK)return result;
        if(covered){
            /* NFC/GSUB covering a whole grapheme does not prove that an
             * independently requested slice can be drawn in that font. */
            result=__xgeOtFontCovers(desc,run,candidate,begin,item_end,&covered);
            if(result!=XGE_OK)return result;
            if(covered){*out=candidate;return XGE_OK;}
        }
    }
    return XGE_OK;
}
