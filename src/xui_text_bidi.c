#include "../xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_text_bidi.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef XUI_BIDI_TEST_ALLOCATOR
void* xuiBidiTestMalloc(size_t);
void* xuiBidiTestRealloc(void*,size_t);
void xuiBidiTestFree(void*);
#define BIDI_ALLOC xuiBidiTestMalloc
#define BIDI_REALLOC xuiBidiTestRealloc
#define BIDI_FREE xuiBidiTestFree
#else
#define BIDI_ALLOC xrtMalloc
#define BIDI_REALLOC xrtRealloc
#define BIDI_FREE xrtFree
#endif
/* Vendor changes are limited to relative includes and two tested fixes listed
 * in README.xge.md. Route allocations through the engine allocator, disabling
 * the retained TLS scratch pool so failure recovery has explicit ownership. */
#define malloc BIDI_ALLOC
#define realloc BIDI_REALLOC
#define free BIDI_FREE
#define SB_CONFIG_UNITY
#define SB_CONFIG_DISABLE_SCRATCH_MEMORY
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
#include "../lib/sheenbidi/Source/SheenBidi.c"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef malloc
#undef realloc
#undef free

struct xui_text_bidi_t {
    char* text;
    size_t bytes,count,capacity;
    SBAlgorithmRef algorithm;
    SBParagraphRef* paragraphs;
};
const char* xuiInternalTextBidiText(xui_text_bidi b){return b?b->text:NULL;}
static int bidi_boundary(xui_text_bidi b,size_t at)
{
    return at<=b->bytes && (at==b->bytes || ((unsigned char)b->text[at]&0xc0)!=0x80);
}
static int bidi_utf8(const char* text,size_t bytes)
{
    size_t at=0;
    while(at<bytes) {
        unsigned char first=(unsigned char)text[at++]; uint32_t cp; unsigned need,i;
        if(first<0x80)continue;
        if(first>=0xc2 && first<=0xdf) { cp=first&31; need=1; }
        else if(first>=0xe0 && first<=0xef) { cp=first&15; need=2; }
        else if(first>=0xf0 && first<=0xf4) { cp=first&7; need=3; }
        else return 0;
        if(need>bytes-at)return 0;
        for(i=0;i<need;i++) { unsigned char c=(unsigned char)text[at++]; if((c&0xc0)!=0x80)return 0; cp=(cp<<6)|(c&63); }
        if((need==2 && cp<0x800) || (need==3 && cp<0x10000) || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff))return 0;
    }
    return 1;
}
int xuiInternalTextBidiNeedsAnalysis(const char* text,size_t bytes)
{
    SBCodepointSequence sequence; SBUInteger at=0;
    if(!text || bytes>INT_MAX || !bidi_utf8(text,bytes))return 1;
    sequence=(SBCodepointSequence){SBStringEncodingUTF8,text,bytes};
    while(at<bytes) {
        SBBidiType type=SBCodepointGetBidiType(SBCodepointSequenceGetCodepointAt(&sequence,&at));
        switch(type) {
        case SBBidiTypeR: case SBBidiTypeAL: case SBBidiTypeAN:
        case SBBidiTypeLRE: case SBBidiTypeRLE: case SBBidiTypeLRO: case SBBidiTypeRLO:
        case SBBidiTypePDF: case SBBidiTypeLRI: case SBBidiTypeRLI: case SBBidiTypeFSI: case SBBidiTypePDI:
            return 1;
        default: break;
        }
    }
    return 0;
}
void xuiInternalTextBidiFree(xui_text_bidi b)
{
    size_t i;
    if(!b)return;
    for(i=0;i<b->count;i++)SBParagraphRelease(b->paragraphs[i]);
    if(b->algorithm)SBAlgorithmRelease(b->algorithm);
    BIDI_FREE(b->paragraphs); BIDI_FREE(b->text); BIDI_FREE(b);
}
int xuiInternalTextBidiCreate(const char* text,size_t bytes,unsigned base,xui_text_bidi* out)
{
    xui_text_bidi b; SBCodepointSequence sequence; size_t at=0; int result=XUI_OK;
    if(!out)return XUI_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(bytes>INT_MAX)return XUI_DOC_ERROR_LIMIT;
    if(!text || bytes==SIZE_MAX || (base!=0 && base!=1 && base!=XUI_BIDI_AUTO_LTR && base!=XUI_BIDI_AUTO_RTL) || !bidi_utf8(text,bytes))return XUI_ERROR_INVALID_ARGUMENT;
    b=BIDI_ALLOC(sizeof(*b)); if(!b)return XUI_ERROR_OUT_OF_MEMORY; memset(b,0,sizeof(*b));
    b->text=BIDI_ALLOC(bytes+1); if(!b->text) { result=XUI_ERROR_OUT_OF_MEMORY; goto fail; }
    memcpy(b->text,text,bytes); b->text[bytes]=0; b->bytes=bytes;
    if(bytes) {
        sequence=(SBCodepointSequence){SBStringEncodingUTF8,b->text,bytes};
        b->algorithm=SBAlgorithmCreate(&sequence); if(!b->algorithm) { result=XUI_ERROR_OUT_OF_MEMORY; goto fail; }
    }
    while(at<bytes) {
        SBParagraphRef paragraph; size_t length;
        if(b->count==b->capacity) {
            size_t capacity=b->capacity?b->capacity*2:4; void* data;
            if(capacity<b->capacity || capacity>SIZE_MAX/sizeof(*b->paragraphs)) { result=XUI_ERROR_OUT_OF_MEMORY; goto fail; }
            data=BIDI_REALLOC(b->paragraphs,capacity*sizeof(*b->paragraphs));
            if(!data) { result=XUI_ERROR_OUT_OF_MEMORY; goto fail; } b->paragraphs=data; b->capacity=capacity;
        }
        paragraph=SBAlgorithmCreateParagraph(b->algorithm,at,bytes-at,(SBLevel)base);
        if(!paragraph) { result=XUI_ERROR_OUT_OF_MEMORY; goto fail; }
        b->paragraphs[b->count++]=paragraph;
        length=SBParagraphGetLength(paragraph);
        if(!length || length>bytes-at || SBParagraphGetOffset(paragraph)!=at) { result=XUI_ERROR_INVALID_ARGUMENT; goto fail; }
        at+=length;
    }
    *out=b; return XUI_OK;
fail:
    xuiInternalTextBidiFree(b); return result;
}
size_t xuiInternalTextBidiParagraphCount(xui_text_bidi b) { return b?b->count:0; }
size_t xuiInternalTextBidiTextBytes(xui_text_bidi b) { return b?b->bytes:0; }
size_t xuiInternalTextBidiRetainedBytes(xui_text_bidi b)
{
    size_t bytes,i;
    if(!b)return 0;
    bytes=sizeof(*b)+b->bytes+1+b->capacity*sizeof(*b->paragraphs);
    if(b->algorithm)bytes+=sizeof(MemoryList)+sizeof(SBAlgorithm)+b->bytes*sizeof(SBBidiType);
    for(i=0;i<b->count;i++) {
        size_t length=SBParagraphGetLength(b->paragraphs[i]);
        size_t amount=sizeof(MemoryList)+sizeof(SBParagraph)+(length+2)*sizeof(SBLevel);
        if(amount>SIZE_MAX-bytes)return SIZE_MAX;
        bytes+=amount;
    }
    return bytes;
}
int xuiInternalTextBidiParagraph(xui_text_bidi b,size_t index,xui_bidi_paragraph_t* out)
{
    SBParagraphRef p;
    if(!b || !out || index>=b->count)return XUI_ERROR_INVALID_ARGUMENT;
    p=b->paragraphs[index]; out->start=SBParagraphGetOffset(p); out->end=out->start+SBParagraphGetLength(p); out->base=SBParagraphGetBaseLevel(p); return XUI_OK;
}
static size_t bidi_paragraph_at(xui_text_bidi b,size_t at)
{
    size_t low=0,high=b->count;
    while(low<high) { size_t mid=low+(high-low)/2; SBParagraphRef p=b->paragraphs[mid];
        if(SBParagraphGetOffset(p)+SBParagraphGetLength(p)<=at)low=mid+1; else high=mid;
    }
    return low;
}
int xuiInternalTextBidiLevel(xui_text_bidi b,size_t at,uint8_t* out)
{
    SBParagraphRef p; size_t index;
    if(!b || !out || at>=b->bytes || !bidi_boundary(b,at))return XUI_ERROR_INVALID_ARGUMENT;
    index=bidi_paragraph_at(b,at); if(index==b->count)return XUI_ERROR_INVALID_ARGUMENT;
    p=b->paragraphs[index]; *out=SBParagraphGetLevelsPtr(p)[at-SBParagraphGetOffset(p)]; return XUI_OK;
}
int xuiInternalTextBidiRestoreHyphenLevels(xui_text_bidi b,size_t start,size_t hyphen,uint8_t* levels)
{
    size_t index,at; SBParagraphRef paragraph; const SBLevel* original;
    SBCodepointSequence sequence;
    if(!b || !levels || start>hyphen || hyphen>=b->bytes ||
        !bidi_boundary(b,start) || !bidi_boundary(b,hyphen))return XUI_ERROR_INVALID_ARGUMENT;
    index=bidi_paragraph_at(b,start);
    if(index==b->count)return XUI_ERROR_INVALID_ARGUMENT;
    paragraph=b->paragraphs[index];
    if(hyphen>=SBParagraphGetOffset(paragraph)+SBParagraphGetLength(paragraph))return XUI_ERROR_INVALID_ARGUMENT;
    sequence=(SBCodepointSequence){SBStringEncodingUTF8,b->text,b->bytes};
    {SBUInteger offset=hyphen;if(SBCodepointSequenceGetCodepointAt(&sequence,&offset)!=0xad)return XUI_ERROR_INVALID_ARGUMENT;}
    original=SBParagraphGetLevelsPtr(paragraph); at=hyphen;
    while(at>start){
        size_t previous=at-1; SBUInteger offset; SBBidiType type;
        while(previous>start && ((unsigned char)b->text[previous]&0xc0)==0x80)previous--;
        offset=previous;type=SBCodepointGetBidiType(SBCodepointSequenceGetCodepointAt(&sequence,&offset));
        if(type!=SBBidiTypeWS && type!=SBBidiTypeBN && !SBBidiTypeIsFormat(type))break;
        memcpy(levels+previous-start,original+previous-SBParagraphGetOffset(paragraph),at-previous);
        at=previous;
    }
    return XUI_OK;
}
void xuiInternalTextBidiLineFree(xui_bidi_line_t* line)
{
    if(!line)return;
    BIDI_FREE(line->runs); BIDI_FREE(line->mirrors); BIDI_FREE(line->levels); memset(line,0,sizeof(*line));
}
int xuiInternalTextBidiLine(xui_text_bidi b,size_t start,size_t end,xui_bidi_line_t* out)
{
    xui_bidi_line_t candidate={0}; SBLineRef line=NULL; SBMirrorLocatorRef locator=NULL;
    size_t index,i,count,mirrors=0; const SBRun* runs; const SBMirrorAgent* agent; int result=XUI_ERROR_OUT_OF_MEMORY;
    if(!out)return XUI_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));
    if(!b || start>end || !bidi_boundary(b,start) || !bidi_boundary(b,end))return XUI_ERROR_INVALID_ARGUMENT;
    candidate.start=start; candidate.end=end;
    if(start==end) {
        if(b->count) {
            index=start==b->bytes?b->count-1:bidi_paragraph_at(b,start);
            if(index<b->count)candidate.base=SBParagraphGetBaseLevel(b->paragraphs[index]);
        }
        *out=candidate; return XUI_OK;
    }
    index=bidi_paragraph_at(b,start);
    if(index==b->count || end>SBParagraphGetOffset(b->paragraphs[index])+SBParagraphGetLength(b->paragraphs[index]))return XUI_ERROR_INVALID_ARGUMENT;
    candidate.base=SBParagraphGetBaseLevel(b->paragraphs[index]);
    line=SBParagraphCreateLine(b->paragraphs[index],start,end-start); if(!line)goto done;
    count=SBLineGetRunCount(line); runs=SBLineGetRunsPtr(line);
    if(!count || count>SIZE_MAX/sizeof(*candidate.runs))goto done;
    candidate.runs=BIDI_ALLOC(count*sizeof(*candidate.runs)); if(!candidate.runs)goto done; candidate.count=count;
    candidate.levels=BIDI_ALLOC(end-start); if(!candidate.levels)goto done;
    for(i=0;i<count;i++) {
        if(runs[i].offset<start || runs[i].offset>end || runs[i].length>end-runs[i].offset || !runs[i].length ||
           !bidi_boundary(b,runs[i].offset) || !bidi_boundary(b,runs[i].offset+runs[i].length)) { result=XUI_ERROR_INVALID_ARGUMENT; goto done; }
        candidate.runs[i]=(xui_bidi_run_t){runs[i].offset,runs[i].offset+runs[i].length,runs[i].level};
        memset(candidate.levels+runs[i].offset-start,runs[i].level,runs[i].length);
    }
    locator=SBMirrorLocatorCreate(); if(!locator)goto done;
    SBMirrorLocatorLoadLine(locator,line,b->text); agent=SBMirrorLocatorGetAgent(locator);
    while(SBMirrorLocatorMoveNext(locator))mirrors++;
    if(mirrors) {
        if(mirrors>SIZE_MAX/sizeof(*candidate.mirrors))goto done;
        candidate.mirrors=BIDI_ALLOC(mirrors*sizeof(*candidate.mirrors)); if(!candidate.mirrors)goto done;
        SBMirrorLocatorReset(locator); i=0;
        while(SBMirrorLocatorMoveNext(locator)) {
            if(i>=mirrors || agent->index<start || agent->index>=end || !bidi_boundary(b,agent->index)) { result=XUI_ERROR_INVALID_ARGUMENT; goto done; }
            candidate.mirrors[i++]=(xui_bidi_mirror_t){agent->index,agent->codepoint,agent->mirror};
        }
        if(i!=mirrors) { result=XUI_ERROR_INVALID_ARGUMENT; goto done; }
        candidate.mirror_count=i;
    }
    *out=candidate; memset(&candidate,0,sizeof(candidate)); result=XUI_OK;
done:
    if(locator)SBMirrorLocatorRelease(locator);
    if(line)SBLineRelease(line);
    xuiInternalTextBidiLineFree(&candidate); return result;
}

#endif
