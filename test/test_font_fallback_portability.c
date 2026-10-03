/* Actual C fallback selector, with real HB fonts and a window-free font shim. */
#include "../xge.h"
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include "../lib/libunibreak/src/graphemebreak.h"
#include "../lib/libunibreak/src/unibreakdef.h"
#include "../src/xge_unicode_grapheme.h"
#include "../src/xge_unicode_script.h"
#include "../src/xge_text_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
static size_t owners,calls,fail_at,font_calls;static int fail_font;
static void* fallback_malloc(size_t bytes){void* p;if(++calls==fail_at)return NULL;p=malloc(bytes);if(p)owners++;return p;}
static void fallback_free(void* p){if(p){CHECK(owners);owners--;free(p);}}
#define xrtMalloc fallback_malloc
#define xrtFree fallback_free
#define __xgeOtNextUtf8 ub_get_next_char_utf8
typedef struct fixture_t {hb_blob_t* blob;hb_face_t* face;hb_font_t* font;} fixture_t;
typedef struct xge_glyph_run_backend_t {
    char *pGraphemeBreaks,*pContextGraphemeBreaks;int iContextGraphemeBytes;
    unsigned char* pScriptMap; int iScriptMapBytes,bScriptsChecked;
    const char *pFallbackUnitEnd,*pFallbackWordEnd;xge_font pFallbackUnitFont;
} xge_glyph_run_backend_t;
static void* __xgeFontFace(xge_font font){return font->pBackend;}
static hb_font_t* __xgeFontShapeFont(xge_font font)
{font_calls++;return fail_font?NULL:((fixture_t*)font->pBackend)->font;}
static int __xgeTextUTF8DecodeBounded(const char** at,const char* end,uint32_t* cp)
{size_t length=(size_t)(end-*at),next=0;*cp=ub_get_next_char_utf8((const utf8_t*)*at,length,&next);
 if(*cp==EOS || !next)return XGE_ERROR_INVALID_ARGUMENT;
 *at+=next;return XGE_OK;}
static xge_font __xgeFontResolveCodepoint(xge_font font,uint32_t cp,int* glyph)
{
    unsigned depth;for(depth=0;font && depth<32;depth++,font=font->pFallback){hb_codepoint_t g;
        if(hb_font_get_nominal_glyph(((fixture_t*)font->pBackend)->font,cp,&g) || !font->pFallback){*glyph=(int)g;return font;}}
    return NULL;
}
static int __xgeEmojiMayStart(uint32_t cp){return cp>=0x1f000 || (cp>=0x2000 && cp<=0x2bff) || cp==35 || cp==42 || (cp>=48 && cp<=57);}
static int __xgeOtStrongScript(hb_script_t script)
{return script!=HB_SCRIPT_COMMON && script!=HB_SCRIPT_INHERITED && script!=HB_SCRIPT_UNKNOWN;}
static void __xgeOtGraphemesUtf8(const utf8_t* text,size_t bytes,const char* language,char* breaks)
{(void)language;__xgeGraphemeMap(text,bytes,breaks,ub_get_next_char_utf8);}
#include "../src/xge_text_opentype_fallback.inl"
static void load(xge_font_t* font,const char* name)
{
    char path[160];fixture_t* fixture=calloc(1,sizeof(*fixture));CHECK(fixture);
    snprintf(path,sizeof(path),"test/data/%s.ttf",name);fixture->blob=hb_blob_create_from_file_or_fail(path);CHECK(fixture->blob);
    fixture->face=hb_face_create(fixture->blob,0);fixture->font=hb_font_create(fixture->face);hb_ot_font_set_funcs(fixture->font);
    hb_font_set_scale(fixture->font,1000,1000);memset(font,0,sizeof(*font));font->pBackend=fixture;
}
static void unload(xge_font_t* font)
{fixture_t* fixture=font->pBackend;hb_font_destroy(fixture->font);hb_face_destroy(fixture->face);hb_blob_destroy(fixture->blob);free(fixture);}
static void select_text(xge_font root,const char* text,const unsigned* expected,unsigned n,unsigned flags)
{
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_glyph_run_backend_t backend={0};
    const char *at=text,*end=text+strlen(text);unsigned i=0;size_t baseline;
    desc.sText=text;desc.pFont=root;desc.iFlags=flags;run.iTextSize=(int)strlen(text);run.pBackend=&backend;
    CHECK(__xgeTextContextNormalize(&desc,run.iTextSize,&desc)==XGE_OK);
    calls=0;while(at<end){xge_font selected,wanted=root;const char* scalar=at;uint32_t cp;unsigned j;
        CHECK(i<n && __xgeOtFallbackFont(&desc,&run,scalar,end,&selected)==XGE_OK);
        for(j=0;j<expected[i];j++)wanted=wanted->pFallback;
        if(selected!=wanted)fprintf(stderr,"fallback mismatch scalar %u byte %td text=%s\n",i,scalar-text,text);
        CHECK(selected==wanted);CHECK(__xgeTextUTF8DecodeBounded(&at,end,&cp)==XGE_OK);i++;}
    CHECK(i==n);baseline=calls;fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pScriptMap);CHECK(!owners);
    if(root->pFallback){size_t fault;CHECK(baseline>=1 && baseline<=2);
        for(fault=1;fault<=baseline;fault++){xge_font selected=NULL;
            memset(&backend,0,sizeof(backend));run.pBackend=&backend;calls=0;fail_at=fault;
            CHECK(__xgeOtFallbackFont(&desc,&run,text,end,&selected)==XGE_ERROR_OUT_OF_MEMORY && !backend.pGraphemeBreaks);
            fail_at=0;CHECK(__xgeOtFallbackFont(&desc,&run,text,end,&selected)==XGE_OK && selected);
            fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pScriptMap);CHECK(!owners);
        }}
}
static void select_context(xge_font root,xge_font wanted,const char* text,int offset,int bytes,
    uint32_t script,const char* language)
{
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_glyph_run_backend_t backend={0};
    xge_font selected=NULL;size_t baseline,fault;
    desc.sText=text+offset;desc.pFont=root;desc.iFlags=XGE_TEXT_SHAPE_RTL;
    desc.sContext=text;desc.iContextSize=-1;desc.iContextOffset=offset;desc.iScript=script;desc.sLanguage=language;
    CHECK(__xgeTextContextNormalize(&desc,bytes,&desc)==XGE_OK);run.iTextSize=bytes;run.pBackend=&backend;calls=0;
    CHECK(__xgeOtFallbackFont(&desc,&run,desc.sText,desc.sText+bytes,&selected)==XGE_OK && selected==wanted);
    baseline=calls;CHECK(baseline>=1 && baseline<=3);
    if(script)CHECK(!backend.pScriptMap);
    fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pContextGraphemeBreaks);fallback_free(backend.pScriptMap);CHECK(!owners);
    for(fault=1;fault<=baseline;fault++){
        memset(&backend,0,sizeof(backend));calls=0;fail_at=fault;
        CHECK(__xgeOtFallbackFont(&desc,&run,desc.sText,desc.sText+bytes,&selected)==XGE_ERROR_OUT_OF_MEMORY);
        fail_at=0;CHECK(__xgeOtFallbackFont(&desc,&run,desc.sText,desc.sText+bytes,&selected)==XGE_OK && selected==wanted);
        fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pContextGraphemeBreaks);fallback_free(backend.pScriptMap);CHECK(!owners);
    }
}
static void ascii_context_scripts(void)
{
    const char* contexts[]={"(a)","(a)\n()","()"};
    const int offsets[]={0,4,0};
    const hb_script_t scripts[]={HB_SCRIPT_LATIN,HB_SCRIPT_COMMON,HB_SCRIPT_COMMON};
    for(unsigned i=0;i<3;i++){
        xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_glyph_run_backend_t backend={0};
        desc.sText=contexts[i]+offsets[i];desc.sContext=contexts[i];desc.iContextSize=-1;
        desc.iContextOffset=offsets[i];run.iTextSize=1;run.pBackend=&backend;
        CHECK(__xgeTextContextNormalize(&desc,1,&desc)==XGE_OK);calls=0;fail_at=1;
        CHECK(__xgeOtEnsureScripts(&desc,&run)==XGE_ERROR_OUT_OF_MEMORY && !backend.pScriptMap && !backend.bScriptsChecked && !owners);
        fail_at=0;calls=0;CHECK(__xgeOtEnsureScripts(&desc,&run)==XGE_OK && calls==1 && backend.pScriptMap);
        CHECK(__xgeOtItemScript(&desc,&run,desc.sText,'(')==scripts[i]);
        CHECK(__xgeOtEnsureScripts(&desc,&run)==XGE_OK && calls==1);
        fallback_free(backend.pScriptMap);CHECK(!owners);
    }
    puts("Actual C ASCII subitem scripts: Latin punctuation, mandatory-line reset, all-Common context, script-map OOM/retry and one cached map passed");
}
static void item_coverage(xge_font font,const char* text,int offset,int bytes,uint32_t script,int wanted)
{
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_glyph_run_backend_t backend={0};int covered;
    desc.pFont=font;desc.sContext=text;desc.iContextSize=-1;desc.iContextOffset=offset;
    desc.sText=text+offset;desc.iScript=script;desc.sLanguage="ar";desc.iFlags=XGE_TEXT_SHAPE_RTL;
    CHECK(__xgeTextContextNormalize(&desc,bytes,&desc)==XGE_OK);run.iTextSize=bytes;run.pBackend=&backend;
    CHECK(__xgeOtEnsureScripts(&desc,&run)==XGE_OK && __xgeOtFontCovers(&desc,&run,font,desc.sText,desc.sText+bytes,&covered)==XGE_OK && covered==wanted);
    fallback_free(backend.pScriptMap);CHECK(!owners);
}
int main(void)
{
    xge_font_t full,no_mark,no_base,no_i,no_x_mark,mark_only,composed,script_full,script_probe,context_full,context_probe,medi_probe;
    unsigned one[]={1,1},three[]={1,1,1},zero[]={0,0};
    unsigned partial[]={0,0,1,0},mixed[]={0,0,0,1,1},punct[]={1,1,1,1,0,0,0};unsigned ignored[]={0,0,0};
    load(&full,"xge_opentype_fixture");load(&no_mark,"xge_fallback_no_mark");load(&no_base,"xge_fallback_no_base");
    load(&no_i,"xge_fallback_no_i");load(&no_x_mark,"xge_fallback_no_x_mark");load(&mark_only,"xge_fallback_mark_only");load(&composed,"xge_fallback_composed");
    load(&script_full,"xge_script_fixture");load(&script_probe,"xge_script_probe_tatweel");script_probe.pFallback=&script_full;
    load(&context_full,"xge_context_fixture");load(&context_probe,"xge_context_probe");context_probe.pFallback=&context_full;
    load(&medi_probe,"xge_context_medi_probe");medi_probe.pFallback=&context_full;
    ascii_context_scripts();
    {
        char text[2054];int at=0,i;memcpy(text+at,"\xd8\xa8",2);at+=2;
        for(i=0;i<512;i++){memcpy(text+at,"\xd9\x8e",2);at+=2;}
        memcpy(text+at,"\xd8\xa8",2);at+=2;
        for(i=0;i<512;i++){memcpy(text+at,"\xd9\x8e",2);at+=2;}
        memcpy(text+at,"\xd8\xa8",2);at+=2;
        /* Bounded context does not require NUL; these helpers use -1, so
         * leave the full length authoritative with a separate terminator. */
        {char terminated[2055];memcpy(terminated,text,(size_t)at);terminated[at]=0;
            item_coverage(&medi_probe,terminated,1026,2,0,1);
            item_coverage(&medi_probe,terminated,1026,2,HB_SCRIPT_ARABIC,1);
            select_context(&medi_probe,&context_full,terminated,1026,2,0,"ar");
            select_context(&medi_probe,&context_full,terminated,1026,2,HB_SCRIPT_ARABIC,"ar");}
        select_context(&medi_probe,&context_full,"\xd8\xa8\xe2\x80\x8c\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd8\xa8\xd8\xa8",15,2,HB_SCRIPT_ARABIC,"ar");
        puts("Actual C long joining coverage: medial-only GSUB covers the item after 512 marks; whole-word coverage selects full font, ZWNJ/item coverage and OOM retry passed");
    }
    select_context(&context_probe,&context_probe,"\xd8\xa8\xd8\xa8\xd8\xa8",2,2,0,"ar");
    select_context(&context_probe,&context_probe,"\xd8\xa8\xd8\xa8\xd8\xa8",2,2,HB_SCRIPT_ARABIC,"ar");
    select_context(&context_probe,&context_full,"\xd8\xa8",0,2,0,"ar");
    select_context(&context_probe,&context_probe,"iii",1,1,HB_SCRIPT_LATIN,"tr");
    select_context(&context_probe,&context_full,"iii",1,1,HB_SCRIPT_LATIN,"en");
    no_i.pFallback=&full;
    select_context(&no_i,&full,"ffi",0,1,0,"en");
    select_context(&no_i,&full,"ffi",1,1,HB_SCRIPT_LATIN,"en");
    select_context(&no_i,&no_i,"ff i",0,1,0,"en");
    /* No candidate covers the whole word: preserve ordered item/grapheme
     * fallback rather than treating context as an all-or-nothing failure. */
    no_i.pFallback=&no_x_mark;no_x_mark.pFallback=&mark_only;
    select_context(&no_i,&no_i,"fix",0,1,0,"en");
    select_context(&no_i,&no_x_mark,"fix",1,1,0,"en");
    select_context(&no_i,&no_i,"fix",2,1,0,"en");
    no_x_mark.pFallback=NULL;
    puts("Actual C context coverage: Arabic medial GSUB, isolated fallback, Turkish/English locl, full-context script allocation and per-allocation retry passed");
    select_text(&script_probe,"\xd9\x80\xdc\x90",zero,2,XGE_TEXT_SHAPE_RTL);
    no_mark.pFallback=no_base.pFallback=no_i.pFallback=composed.pFallback=&full;
    select_text(&no_mark,"a\xcc\x81",one,2,0);select_text(&no_base,"a\xcc\x81",one,2,0);
    select_text(&no_i,"ffi",three,3,0);select_text(&no_i,"\xce\xbb\xce\xbc",one,2,0);
    select_text(&no_i,"\xd7\x90\xd7\x91\xd7\x92",three,3,XGE_TEXT_SHAPE_RTL);
    select_text(&no_mark,"\xd7\x90\xd6\xb0",one,2,XGE_TEXT_SHAPE_RTL);
    select_text(&composed,"a\xcc\x81",zero,2,0);
    select_text(&no_i,"ffi.aaa",punct,7,0);select_text(&no_i,"aaa\xce\xbb\xce\xbc",mixed,5,0);
    select_text(&no_mark,"a\xe2\x80\x8d\xef\xb8\x8f",ignored,3,XGE_TEXT_SHAPE_DEFAULT);
    no_i.pFallback=&no_x_mark;select_text(&no_i,"ffix",partial,4,0);
    no_mark.pFallback=&mark_only;select_text(&no_mark,"a\xcc\x81",zero,2,0);
    {
        xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_glyph_run_backend_t backend={0};xge_font selected;
        const size_t bytes=40000;char* text=malloc(bytes+1);size_t i;CHECK(text);
        for(i=0;i<bytes;i++)text[i]="ffix"[i%4];
        text[bytes]=0;
        desc.sText=text;desc.pFont=&no_i;run.iTextSize=(int)bytes;run.pBackend=&backend;font_calls=0;
        CHECK(__xgeTextContextNormalize(&desc,run.iTextSize,&desc)==XGE_OK);
        for(i=0;i<bytes;i++)CHECK(__xgeOtFallbackFont(&desc,&run,text+i,text+bytes,&selected)==XGE_OK);
        CHECK(font_calls<bytes*5);printf("Fallback unmatchable word: %zu bytes, %zu font visits (cached whole-word failure, linear)\n",bytes,font_calls);
        fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pScriptMap);free(text);CHECK(!owners);
        memset(&backend,0,sizeof(backend));desc.sText="a\xcc\x81";desc.pFont=&no_mark;run.iTextSize=3;
        desc.sContext=NULL;desc.iContextSize=0;CHECK(__xgeTextContextNormalize(&desc,run.iTextSize,&desc)==XGE_OK);
        fail_font=1;CHECK(__xgeOtFallbackFont(&desc,&run,desc.sText,desc.sText+3,&selected)==XGE_ERROR_OUT_OF_MEMORY);
        fail_font=0;fallback_free(backend.pGraphemeBreaks);fallback_free(backend.pScriptMap);CHECK(!owners);
    }
    {uint32_t cp;size_t marked=0;for(cp=0;cp<=0x10ffff;cp++)if(__xgeOtIgnorable(cp))marked++;
      CHECK(marked==4174 && !__xgeOtIgnorable(0x301) && __xgeOtIgnorable(0xfe0f) && __xgeOtIgnorable(0x115f));}
    unload(&full);unload(&no_mark);unload(&no_base);unload(&no_i);unload(&no_x_mark);unload(&mark_only);unload(&composed);
    unload(&script_full);unload(&script_probe);
    unload(&context_full);unload(&context_probe);unload(&medi_probe);
    puts("Actual C fallback selector: complete graphemes/words, Latin/Greek/RTL/GPOS fonts, NFC normalization, default ignorables, ordered partial fallback, OOM cleanup and linear unmatched word passed");return 0;
}
