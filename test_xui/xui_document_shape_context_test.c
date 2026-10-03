/* Exercise the actual retained-context factory with literal projection/script
 * oracles and each allocation failure. Display filtering uses XUI's helper. */
#include "../src/xui_document_layout_internal.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
typedef struct doc_paragraph_span {uint64_t node,start,end;uint32_t kind;size_t first_fragment,end_fragment;} doc_paragraph_span;
static void* live[32];static unsigned live_count,attempts,budget=UINT_MAX;
static void* failing_malloc(size_t bytes)
{
    void* p;if(attempts++>=budget)return NULL;
    p=malloc(bytes);if(p){CHECK(live_count<32);live[live_count++]=p;}return p;
}
static void* failing_calloc(size_t count,size_t bytes)
{void* p=failing_malloc(count*bytes);if(p)memset(p,0,count*bytes);return p;}
static void failing_free(void* p)
{
    unsigned i;if(!p)return;for(i=0;i<live_count;i++)if(live[i]==p)break;
    CHECK(i<live_count);live[i]=live[--live_count];free(p);
}
int doc_render_reserve(void** data,size_t* capacity,size_t count,size_t bytes)
{
    void* p;if(count<=*capacity)return XUI_OK;
    p=failing_malloc(count*bytes);if(!p)return XUI_ERROR_OUT_OF_MEMORY;
    if(*data){memcpy(p,*data,*capacity*bytes);failing_free(*data);}
    *data=p;*capacity=count;return XUI_OK;
}
#define malloc failing_malloc
#define calloc failing_calloc
#define free failing_free
#include "../src/xui_document_shape_context.inl"
#undef malloc
#undef calloc
#undef free
static void clear(doc_render_block* block)
{
    size_t i;for(i=0;i<block->context_count;i++)doc_context_free(block->contexts[i]);
    failing_free(block->contexts);memset(block,0,sizeof(*block));CHECK(!live_count);
}
static void hyphen_context(void)
{
    const char source[]="a\xc2\xad\xce\xbb";
    doc_render_block block={0};doc_shape_context *context=NULL,*out,*sentinel=(void*)(uintptr_t)1;
    doc_hyphen_lease outer={0},same={0},nested={0},unchanged={0};unsigned failures=0;int result;
    for(budget=0;budget<16;budget++){
        attempts=0;out=sentinel;
        result=doc_context_acquire(&block,3,source,5,NULL,0,1,&out);
        if(result==XUI_OK){context=out;break;}
        CHECK(result==XUI_ERROR_OUT_OF_MEMORY && out==sentinel && !block.context_count && !block.context_cursor && !live_count);
        failures++;
    }
    CHECK(failures==6 && context->bytes==3 && context->hyphen_text && !strcmp(context->text,"a\xce\xbb") &&
        !strcmp(context->hyphen_text,"-a\xce\xbb") && context->hyphen_copied==4);
    budget=0;attempts=0;
    CHECK(doc_context_hyphen_borrow(context,1,&outer)==XUI_OK && !strcmp(outer.text,"a-\xce\xbb") && !attempts);
    CHECK(doc_context_hyphen_borrow(context,1,&same)==XUI_OK && same.text==outer.text && context->hyphen_borrows==2 && !attempts);
    unchanged.text="sentinel";nested=unchanged;
    CHECK(doc_context_hyphen_borrow(context,3,&nested)==XUI_ERROR_OUT_OF_MEMORY &&
        !memcmp(&nested,&unchanged,sizeof(nested)) && context->hyphen_borrows==2 && !strcmp(outer.text,"a-\xce\xbb"));
    budget=UINT_MAX;attempts=0;
    CHECK(doc_context_hyphen_borrow(context,3,&nested)==XUI_OK && nested.owned && nested.text!=outer.text &&
        !strcmp(nested.text,"a\xce\xbb-") && !strcmp(outer.text,"a-\xce\xbb") && context->hyphen_at==1 &&
        context->hyphen_borrows==2 && attempts==1);
    doc_context_hyphen_release(&nested);doc_context_hyphen_release(&nested);
    doc_context_hyphen_release(&same);CHECK(context->hyphen_borrows==1);
    doc_context_hyphen_release(&outer);CHECK(!context->hyphen_borrows);
    budget=0;attempts=0;
    CHECK(doc_context_hyphen_borrow(context,3,&outer)==XUI_OK && !strcmp(outer.text,"a\xce\xbb-") && !attempts);
    doc_context_hyphen_release(&outer);
    CHECK(doc_context_hyphen_borrow(context,0,&outer)==XUI_OK && !strcmp(outer.text,"-a\xce\xbb") && !attempts);
    doc_context_hyphen_release(&outer);outer=unchanged;
    CHECK(doc_context_hyphen_borrow(context,2,&outer)==XUI_ERROR_INVALID_ARGUMENT && !memcmp(&outer,&unchanged,sizeof(outer)));
    CHECK(doc_context_hyphen_borrow(context,4,&outer)==XUI_ERROR_INVALID_ARGUMENT && !memcmp(&outer,&unchanged,sizeof(outer)));
    context->hyphen_borrows=UINT32_MAX;
    CHECK(doc_context_hyphen_borrow(context,0,&outer)==XUI_DOC_ERROR_LIMIT && !memcmp(&outer,&unchanged,sizeof(outer)));
    context->hyphen_borrows=0;context->hyphen_copied=UINT64_MAX-1;
    CHECK(doc_context_hyphen_borrow(context,3,&outer)==XUI_OK && context->hyphen_copied==UINT64_MAX);
    doc_context_hyphen_release(&outer);
    block.reflow=1;block.context_cursor=0;out=sentinel;
    CHECK(doc_context_acquire(&block,3,source,5,NULL,0,1,&out)==XUI_OK && out==context && !attempts);
    block.context_cursor=0;out=sentinel;
    CHECK(doc_context_acquire(&block,3,source,5,NULL,0,0,&out)==XUI_DOC_ERROR_STALE && out==sentinel && !attempts);
    clear(&block);budget=UINT_MAX;attempts=0;
    CHECK(doc_context_acquire(&block,3,source,5,NULL,0,0,&context)==XUI_OK && !context->hyphen_text && attempts==5);
    clear(&block);attempts=0;
    CHECK(doc_context_acquire(&block,3,"a\xc2\xad" "b",4,NULL,0,1,&context)==XUI_OK &&
        context->hyphen_text && !context->scripts && attempts==5);
    budget=0;attempts=0;
    CHECK(doc_context_hyphen_borrow(context,1,&outer)==XUI_OK && !strcmp(outer.text,"a-b") && !attempts);
    doc_context_hyphen_release(&outer);
    clear(&block);budget=UINT_MAX;attempts=0;
    /* Move through a MiB paragraph with allocation disabled. The copy counter
     * must equal the initial copy plus the distance traversed, not one full
     * paragraph per insertion. UTF-8 continuation offsets remain invalid. */
    {
        const size_t repetitions=262144;size_t i;char* large=malloc(repetitions*5+1);CHECK(large);
        for(i=0;i<repetitions;i++)memcpy(large+i*5,source,5);
        large[repetitions*5]=0;
        CHECK(doc_context_acquire(&block,3,large,repetitions*5,NULL,0,1,&context)==XUI_OK);
        CHECK(context->bytes==repetitions*3 && context->hyphen_copied==context->bytes+1);
        budget=0;attempts=0;
        for(i=0;i<repetitions;i++){
            uint32_t at=(uint32_t)(i*3+1);
            CHECK(doc_context_hyphen_borrow(context,at,&outer)==XUI_OK && outer.text[at]=='-' &&
                outer.text[at-1]=='a' && (unsigned char)outer.text[at+1]==0xce && !context->hyphen_text[context->bytes+1]);
            doc_context_hyphen_release(&outer);
        }
        CHECK(!attempts && !context->hyphen_borrows && context->hyphen_copied==2*(uint64_t)context->bytes-1);
        for(i=0;i<repetitions;i++)CHECK(!memcmp(context->text+i*3,"a\xce\xbb",3));
        CHECK(doc_context_hyphen_borrow(context,0,&outer)==XUI_OK && outer.text[0]=='-' &&
            !memcmp(outer.text+1,context->text,(size_t)context->bytes+1));
        doc_context_hyphen_release(&outer);clear(&block);free(large);
    }
    budget=UINT_MAX;attempts=0;
    puts("Document SHY context: six factory allocation failures, zero-allocation cursor movement, nested frozen leases/OOM retry, UTF-8/limit rejection, policy reflow and linear copy bound passed");
}
int main(void)
{
    /* SoftBreak shrinks three source bytes to a space; HardBreak keeps three
     * bytes as U+2028. WJ/RLE/PDF disappear; ZWJ/ZWNJ and U+FFFC remain. */
    const char source[]="a   \xce\xbb\xe2\x81\xa0\xe2\x80\xab\xd8\xa8\xe2\x80\xac\xe2\x80\x8d\xe2\x80\x8c\n\n\n\xef\xbf\xbc";
    const char expected[]="a \xce\xbb\xd8\xa8\xe2\x80\x8d\xe2\x80\x8c\xe2\x80\xa8\xef\xbf\xbc";
    doc_paragraph_span spans[]={{.start=1,.end=4,.kind=XUI_DOC_SOFT_BREAK},{.start=23,.end=26,.kind=XUI_DOC_HARD_BREAK}};
    const uint32_t boundaries[][2]={{0,0},{1,1},{4,2},{6,4},{9,4},{12,4},{14,6},{17,6},{20,9},{23,12},{26,15},{29,18}};
    doc_render_block block={0};doc_shape_context *context,*out,*sentinel=(void*)(uintptr_t)1;
    unsigned i,failures=0;int result;
    CHECK(strlen(source)==29 && strlen(expected)==18);
    for(budget=0;budget<16;budget++){
        attempts=0;out=sentinel;
        result=doc_context_acquire(&block,7,source,strlen(source),spans,2,1,&out);
        if(result==XUI_OK){context=out;break;}
        CHECK(result==XUI_ERROR_OUT_OF_MEMORY && out==sentinel && !block.context_count && !block.context_cursor && !live_count);
        failures++;
    }
    CHECK(failures==5 && block.context_count==1 && block.context_cursor==1 && context->source_map && context->scripts &&
        context->bytes==strlen(expected) && !memcmp(context->text,expected,strlen(expected)+1));
    for(i=0;i<sizeof(boundaries)/sizeof(*boundaries);i++)CHECK(doc_context_offset(context,boundaries[i][0])==boundaries[i][1]);
    CHECK(doc_context_script(context,4,0)==UINT32_C(0x4772656b) && doc_context_script(context,12,0)==UINT32_C(0x41726162));
    /* Reflow borrows the exact context; no allocation or script map rebuild.
     * Stale identity/byte length is rejected without publishing an output. */
    block.reflow=1;block.context_cursor=0;budget=0;attempts=0;out=sentinel;
    CHECK(doc_context_acquire(&block,7,source,29,spans,2,1,&out)==XUI_OK && out==context && !attempts);
    block.context_cursor=0;out=sentinel;
    CHECK(doc_context_acquire(&block,8,source,29,spans,2,1,&out)==XUI_DOC_ERROR_STALE && out==sentinel && !attempts);
    block.context_cursor=0;
    CHECK(doc_context_acquire(&block,7,source,28,spans,2,1,&out)==XUI_DOC_ERROR_STALE && out==sentinel && !attempts);
    clear(&block);budget=UINT_MAX;attempts=0;
    CHECK(doc_context_acquire(&block,1,"ASCII",5,NULL,0,1,&context)==XUI_OK && !context->source_map && !context->scripts && attempts==3);
    /* A failed second context must preserve the first context/registry. */
    for(budget=0;budget<16;budget++){
        attempts=0;out=sentinel;
        result=doc_context_acquire(&block,2,source,29,spans,2,1,&out);
        if(result==XUI_OK)break;
        CHECK(result==XUI_ERROR_OUT_OF_MEMORY && out==sentinel && block.context_count==1 &&
            block.context_cursor==1 && block.contexts[0]==context && live_count==3);
    }
    CHECK(budget==5 && block.context_count==2 && block.context_cursor==2);
    clear(&block);budget=UINT_MAX;attempts=0;
    CHECK(doc_context_acquire(&block,1,"\xce\xbb",2,NULL,0,1,&context)==XUI_OK && !context->source_map && context->scripts && attempts==4);
    clear(&block);
    hyphen_context();
    /* All paired brackets resolve from the paragraph, not the item/style. */
    CHECK(doc_context_acquire(&block,1,"a(\xce\xbb)a",6,NULL,0,1,&context)==XUI_OK);
    CHECK(doc_context_script(context,1,0)==UINT32_C(0x4c61746e) && doc_context_script(context,2,0)==UINT32_C(0x4772656b) &&
        doc_context_script(context,4,0)==UINT32_C(0x4c61746e));clear(&block);
    {
        doc_paragraph_span ascii[]={{.start=0,.end=1,.kind=XUI_DOC_TEXT},
            {.start=1,.end=2,.kind=XUI_DOC_TEXT},{.start=2,.end=3,.kind=XUI_DOC_TEXT}};
        for(budget=0;budget<8;budget++){
            attempts=0;out=sentinel;
            result=doc_context_acquire(&block,1,"(a)",3,ascii,3,1,&out);
            if(result==XUI_OK){context=out;break;}
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && out==sentinel && !block.context_count && !live_count);
        }
        CHECK(budget==4 && context->scripts && !context->source_map);
        for(i=0;i<3;i++)CHECK(doc_context_script(context,i,0)==UINT32_C(0x4c61746e));
        block.reflow=1;block.context_cursor=0;budget=0;attempts=0;
        CHECK(doc_context_acquire(&block,1,"(a)",3,ascii,3,1,&out)==XUI_OK && out==context && !attempts);
        clear(&block);budget=UINT_MAX;attempts=0;
    }
    for(i=0;i<4;i++){
        const char* invalid[]={"\xc0\x80","\xed\xa0\x80","\xf4\x90\x80\x80","\xd8"};
        out=sentinel;CHECK(doc_context_acquire(&block,1,invalid[i],strlen(invalid[i]),NULL,0,1,&out)==XUI_ERROR_INVALID_ARGUMENT &&
            out==sentinel && !block.context_count && !live_count);
    }
    out=sentinel;CHECK(doc_context_acquire(&block,1,"",SIZE_MAX,NULL,0,1,&out)==XUI_DOC_ERROR_LIMIT && out==sentinel && !live_count);
    CHECK(doc_context_acquire(&block,1,NULL,0,NULL,0,1,&out)==XUI_ERROR_INVALID_ARGUMENT && out==sentinel);
    {doc_paragraph_span invalid={.start=0,.end=1,.kind=XUI_DOC_SOFT_BREAK};
        CHECK(doc_context_acquire(&block,1,"x",1,&invalid,1,1,&out)==XUI_ERROR_INVALID_ARGUMENT && out==sentinel && !live_count);}
    CHECK(doc_context_acquire(&block,1,"",0,NULL,0,1,&context)==XUI_OK && !context->bytes && context->text[0]==0);
    clear(&block);
    puts("Document retained context factory: literal source/display and script maps, bracket context, five allocation failures/retry, zero-allocation reflow, stale output atomicity, identity/ASCII allocation bounds, invalid UTF-8/limits and empty owner cleanup passed");
    return 0;
}
