#include "src/xui_text_bidi.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
typedef union allocation_header { max_align_t alignment; struct { size_t bytes; } info; } allocation_header;
static size_t allocations,live,live_bytes,fail_at;
void* xuiBidiTestMalloc(size_t bytes)
{
    allocation_header* h;
    if(++allocations==fail_at || bytes>SIZE_MAX-sizeof(*h))return NULL;
    h=malloc(sizeof(*h)+bytes); if(!h)return NULL;
    h->info.bytes=bytes; live++; live_bytes+=bytes; return h+1;
}
void xuiBidiTestFree(void* data)
{
    allocation_header* h;
    if(!data)return;
    h=(allocation_header*)data-1; CHECK(live && live_bytes>=h->info.bytes);
    live--; live_bytes-=h->info.bytes; free(h);
}
void* xuiBidiTestRealloc(void* data,size_t bytes)
{
    allocation_header *h,*next; size_t old;
    if(!data)return xuiBidiTestMalloc(bytes);
    h=(allocation_header*)data-1; old=h->info.bytes;
    if(++allocations==fail_at || bytes>SIZE_MAX-sizeof(*h))return NULL;
    next=realloc(h,sizeof(*h)+bytes); if(!next)return NULL;
    next->info.bytes=bytes; live_bytes=live_bytes-old+bytes; return next+1;
}
static xui_text_bidi create(const char* text,unsigned base)
{
    xui_text_bidi b=NULL;size_t previous=live_bytes;
    CHECK(xuiInternalTextBidiCreate(text,strlen(text),base,&b)==XUI_OK && b);
    CHECK(xuiInternalTextBidiRetainedBytes(b)==live_bytes-previous && xuiInternalTextBidiTextBytes(b)==strlen(text));return b;
}
static uint8_t line_level(const xui_bidi_line_t* line,size_t byte)
{
    size_t i;
    for(i=0;i<line->count;i++)if(byte>=line->runs[i].start && byte<line->runs[i].end) {
        CHECK(line->levels && line->levels[byte-line->start]==line->runs[i].level);
        return line->runs[i].level;
    }
    CHECK(0); return 0;
}
static void contracts(void)
{
    xui_text_bidi b=NULL; xui_bidi_paragraph_t p; xui_bidi_line_t line={0}; uint8_t level;
    const char* invalid[]={"\xc0\xaf","\xed\xa0\x80","\xf4\x90\x80\x80","\xe2\x82","\x80","\xe0\x80\xa0"}; size_t i;
    CHECK(!xuiInternalTextBidiNeedsAnalysis("hello \xe4\xb8\xad\xe6\x96\x87",12));
    CHECK(xuiInternalTextBidiNeedsAnalysis("\xd7\x90",2));
    CHECK(xuiInternalTextBidiNeedsAnalysis("\xd8\xa7",2));
    CHECK(xuiInternalTextBidiNeedsAnalysis("\xe2\x81\xa7x\xe2\x81\xa9",7));
    for(i=0;i<sizeof(invalid)/sizeof(*invalid);i++)CHECK(xuiInternalTextBidiCreate(invalid[i],strlen(invalid[i]),0,&b)==XUI_ERROR_INVALID_ARGUMENT && !b);
    CHECK(xuiInternalTextBidiCreate(NULL,0,0,&b)==XUI_ERROR_INVALID_ARGUMENT && !b);
    CHECK(xuiInternalTextBidiCreate("x",1,2,&b)==XUI_ERROR_INVALID_ARGUMENT && !b);
    CHECK(xuiInternalTextBidiCreate("",(size_t)INT_MAX+1,0,&b)==XUI_DOC_ERROR_LIMIT && !b);
    b=create("",XUI_BIDI_AUTO_RTL); CHECK(!xuiInternalTextBidiParagraphCount(b));
    CHECK(xuiInternalTextBidiLine(b,0,0,&line)==XUI_OK && !line.count);
    CHECK(xuiInternalTextBidiLevel(b,0,&level)==XUI_ERROR_INVALID_ARGUMENT); xuiInternalTextBidiFree(b);
    CHECK(xuiInternalTextBidiCreate("a\0b",3,0,&b)==XUI_OK);
    CHECK(xuiInternalTextBidiLevel(b,2,&level)==XUI_OK && !level); xuiInternalTextBidiFree(b);
    {
        char text[]="(\xd7\x90\xd7\x91)";
        b=create(text,XUI_BIDI_AUTO_LTR); memset(text,'x',sizeof(text)-1);
        CHECK(xuiInternalTextBidiParagraph(b,0,&p)==XUI_OK && p.start==0 && p.end==6 && p.base==1);
        CHECK(xuiInternalTextBidiLine(b,0,6,&line)==XUI_OK && line.count==1 && line.runs[0].level==1 && line.mirror_count==2);
        CHECK(line.mirrors[0].offset==0 && line.mirrors[0].original=='(' && line.mirrors[0].mirrored==')');
        CHECK(line.mirrors[1].offset==5 && line.mirrors[1].mirrored=='(');
        CHECK(xuiInternalTextBidiLevel(b,2,&level)==XUI_ERROR_INVALID_ARGUMENT);
        xuiInternalTextBidiFree(b); /* Line arrays are independently owned. */
        CHECK(line.runs[0].end==6 && line.mirrors[1].offset==5); xuiInternalTextBidiLineFree(&line);
    }
    b=create("a\xe2\x80\xab \xd7\x90 \xe2\x80\xac b",0);
    CHECK(xuiInternalTextBidiLevel(b,7,&level)==XUI_OK && level==1);
    CHECK(xuiInternalTextBidiLine(b,0,8,&line)==XUI_OK && line_level(&line,7)==0);
    CHECK(xuiInternalTextBidiLevel(b,7,&level)==XUI_OK && level==1); /* L1 never mutates paragraph levels. */
    xuiInternalTextBidiLineFree(&line); xuiInternalTextBidiFree(b);
    b=create("\t\xe2\x80\xab\xe2\x81\xa9\xd7\x90",XUI_BIDI_LTR);
    CHECK(xuiInternalTextBidiLine(b,0,9,&line)==XUI_OK && line_level(&line,0)==0 && line_level(&line,4)==1 && line_level(&line,7)==1);
    for(i=0;i<line.count;i++)CHECK((line.runs[i].start!=5 && line.runs[i].start!=6) && (line.runs[i].end!=5 && line.runs[i].end!=6));
    xuiInternalTextBidiLineFree(&line); xuiInternalTextBidiFree(b);
    b=create("\xd7\x90\r\nA\n\xd8\xa8",XUI_BIDI_AUTO_LTR);
    CHECK(xuiInternalTextBidiParagraphCount(b)==3);
    CHECK(xuiInternalTextBidiParagraph(b,0,&p)==XUI_OK && p.start==0 && p.end==4 && p.base==1);
    CHECK(xuiInternalTextBidiParagraph(b,1,&p)==XUI_OK && p.start==4 && p.end==6 && !p.base);
    CHECK(xuiInternalTextBidiParagraph(b,2,&p)==XUI_OK && p.start==6 && p.end==8 && p.base==1);
    CHECK(xuiInternalTextBidiLine(b,0,6,&line)==XUI_ERROR_INVALID_ARGUMENT && !line.count);
    CHECK(xuiInternalTextBidiLine(b,0,1,&line)==XUI_ERROR_INVALID_ARGUMENT && !line.count);
    CHECK(xuiInternalTextBidiLine(b,8,8,&line)==XUI_OK && !line.count); xuiInternalTextBidiFree(b);
    CHECK(!live && !live_bytes);
}
static void allocation_sweep(const char* text,size_t line_end)
{
    xui_text_bidi stable=create(text,XUI_BIDI_AUTO_LTR); xui_bidi_line_t retained={0};
    size_t stable_live,stable_bytes,point,create_points,line_points,create_failures=0,line_failures=0,recovered=0;
    uint32_t retained_mirror;
    xui_text_bidi baseline=NULL; xui_bidi_line_t baseline_line={0};
    CHECK(xuiInternalTextBidiLine(stable,0,line_end,&retained)==XUI_OK && retained.mirror_count); retained_mirror=retained.mirrors[0].mirrored; stable_live=live; stable_bytes=live_bytes;
    allocations=0; CHECK(xuiInternalTextBidiCreate(text,strlen(text),XUI_BIDI_AUTO_LTR,&baseline)==XUI_OK); create_points=allocations; xuiInternalTextBidiFree(baseline);
    allocations=0; CHECK(xuiInternalTextBidiLine(stable,0,line_end,&baseline_line)==XUI_OK); line_points=allocations; xuiInternalTextBidiLineFree(&baseline_line);
    for(point=1;point<=create_points;point++) {
        xui_text_bidi candidate=NULL; int result;
        allocations=0; fail_at=point; result=xuiInternalTextBidiCreate(text,strlen(text),XUI_BIDI_AUTO_LTR,&candidate); fail_at=0;
        if(result==XUI_OK) { recovered++; CHECK(xuiInternalTextBidiParagraphCount(candidate)==xuiInternalTextBidiParagraphCount(stable)); xuiInternalTextBidiFree(candidate); }
        else { CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !candidate); create_failures++; }
        if(live!=stable_live || live_bytes!=stable_bytes)fprintf(stderr,"Create OOM point %zu: live %zu/%zu bytes %zu/%zu\n",point,live,stable_live,live_bytes,stable_bytes);
        CHECK(live==stable_live && live_bytes==stable_bytes && retained.mirrors[0].mirrored==retained_mirror);
    }
    CHECK(create_failures>10);
    for(point=1;point<=line_points;point++) {
        xui_bidi_line_t candidate={0}; int result;
        allocations=0; fail_at=point; result=xuiInternalTextBidiLine(stable,0,line_end,&candidate); fail_at=0;
        if(result==XUI_OK) { size_t i; recovered++; CHECK(candidate.count==retained.count && candidate.mirror_count==retained.mirror_count);
            for(i=0;i<retained.count;i++)CHECK(candidate.runs[i].start==retained.runs[i].start && candidate.runs[i].end==retained.runs[i].end && candidate.runs[i].level==retained.runs[i].level);
            xuiInternalTextBidiLineFree(&candidate); }
        else { CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !candidate.runs && !candidate.mirrors && !candidate.count); line_failures++; }
        CHECK(live==stable_live && live_bytes==stable_bytes && retained.mirror_count);
    }
    CHECK(line_failures>=4);
    xuiInternalTextBidiLineFree(&retained); xuiInternalTextBidiFree(stable); CHECK(!live && !live_bytes);
    printf("Bidi allocation rollback: create=%zu/%zu, line=%zu/%zu, recovered=%zu; no live allocations\n",create_failures,create_points,line_failures,line_points,recovered);
}
static void allocation_failures(void)
{
    char complex[4096]; size_t at=0,i,line_end;
    allocation_sweep("(\xd7\x90\xd7\x91)\nA\nB\nC\nD\nE\nF",6);
    /* Deep isolate overflow, paired-bracket limit, expanding queues and
     * paragraph-vector reallocations participate in the failure sweep. */
    for(i=0;i<127;i++) { memcpy(complex+at,"\xe2\x81\xa7",3); at+=3; }
    for(i=0;i<100;i++)complex[at++]='(';
    memcpy(complex+at,"\xd7\x90 A\xd7\x91",6); at+=6;
    for(i=0;i<100;i++)complex[at++]=')';
    for(i=0;i<127;i++) { memcpy(complex+at,"\xe2\x81\xa9",3); at+=3; }
    line_end=at; memcpy(complex+at,"\nA\nB\nC\nD\nE\nF",13); at+=13; complex[at]=0;
    allocation_sweep(complex,line_end);
}
#define MAX_CP 8192
static unsigned encode(uint32_t cp,char* text)
{
    if(cp<0x80) { text[0]=(char)cp; return 1; }
    if(cp<0x800) { text[0]=(char)(0xc0|(cp>>6)); text[1]=(char)(0x80|(cp&63)); return 2; }
    if(cp<0x10000) { text[0]=(char)(0xe0|(cp>>12)); text[1]=(char)(0x80|((cp>>6)&63)); text[2]=(char)(0x80|(cp&63)); return 3; }
    text[0]=(char)(0xf0|(cp>>18)); text[1]=(char)(0x80|((cp>>12)&63)); text[2]=(char)(0x80|((cp>>6)&63)); text[3]=(char)(0x80|(cp&63)); return 4;
}
static size_t boundary_index(const size_t* positions,size_t count,size_t at)
{
    size_t low=0,high=count+1;
    while(low<high) { size_t mid=low+(high-low)/2; if(positions[mid]<at)low=mid+1; else high=mid; }
    CHECK(low<=count && positions[low]==at); return low;
}
static void conformance(const char* path)
{
    FILE* input=fopen(path,"rb"); char *record=malloc(1024*1024),text[MAX_CP*4];
    size_t positions[MAX_CP+1],order[MAX_CP],cases=0,source_line=0;
    int expected[MAX_CP]; uint8_t actual[MAX_CP];
    CHECK(input && record);
    while(fgets(record,1024*1024,input)) {
        char *fields[5],*cursor,*end; size_t count=0,bytes=0,i,r,ordered=0,expected_order=0; unsigned base; long resolved;
        xui_text_bidi b=NULL; xui_bidi_line_t line={0}; xui_bidi_paragraph_t paragraph;
        source_line++; if(record[0]=='#' || record[0]=='\r' || record[0]=='\n')continue;
        fields[0]=record;
        for(i=1;i<5;i++) { cursor=strchr(fields[i-1],';'); CHECK(cursor); *cursor=0; fields[i]=cursor+1; }
        cursor=fields[0];
        while(1) { unsigned long cp=strtoul(cursor,&end,16); if(cursor==end)break; CHECK(count<MAX_CP && cp<=0x10ffff); positions[count++]=bytes; bytes+=encode((uint32_t)cp,text+bytes); cursor=end; }
        positions[count]=bytes; CHECK(count); base=(unsigned)strtoul(fields[1],NULL,10); CHECK(base<=2); if(base==2)base=XUI_BIDI_AUTO_LTR;
        resolved=strtol(fields[2],NULL,10); cursor=fields[3];
        for(i=0;i<count;i++) {
            while(*cursor==' ' || *cursor=='\t')cursor++;
            if(*cursor=='x') { expected[i]=-1; cursor++; }
            else { expected[i]=(int)strtol(cursor,&end,10); CHECK(end!=cursor); cursor=end; }
        }
        CHECK(xuiInternalTextBidiCreate(text,bytes,base,&b)==XUI_OK);
        CHECK(xuiInternalTextBidiParagraphCount(b)==1 && xuiInternalTextBidiParagraph(b,0,&paragraph)==XUI_OK && paragraph.base==resolved);
        CHECK(xuiInternalTextBidiLine(b,0,bytes,&line)==XUI_OK); memset(actual,255,count);
        for(r=0;r<line.count;r++) {
            size_t start=boundary_index(positions,count,line.runs[r].start),stop=boundary_index(positions,count,line.runs[r].end);
            for(i=start;i<stop;i++) { CHECK(actual[i]==255); actual[i]=line.runs[r].level; }
            if(line.runs[r].level&1) { for(i=stop;i>start;) { --i; if(expected[i]>=0) { CHECK(ordered<count); order[ordered++]=i; } } }
            else { for(i=start;i<stop;i++)if(expected[i]>=0) { CHECK(ordered<count); order[ordered++]=i; } }
        }
        for(i=0;i<count;i++)if(expected[i]>=0 && actual[i]!=expected[i]) {
            fprintf(stderr,"BidiCharacterTest line %zu, cp %zu: level %u expected %d\n",source_line,i,actual[i],expected[i]); exit(1);
        }
        cursor=fields[4];
        while(1) { unsigned long index=strtoul(cursor,&end,10); if(end==cursor)break;
            if(expected_order>=ordered || order[expected_order]!=index) {
                fprintf(stderr,"BidiCharacterTest line %zu, visual %zu: got %zu expected %lu\n",source_line,expected_order,expected_order<ordered?order[expected_order]:SIZE_MAX,index); exit(1);
            }
            expected_order++; cursor=end;
        }
        CHECK(expected_order==ordered); cases++;
        xuiInternalTextBidiLineFree(&line); xuiInternalTextBidiFree(b); CHECK(!live && !live_bytes);
    }
    CHECK(!ferror(input) && cases==91707); fclose(input); free(record);
    printf("Unicode 17 BidiCharacterTest: %zu cases, paragraph bases, L1 levels and L2 visual order passed\n",cases);
}
static void class_conformance(const char* path)
{
    static const struct { const char* name; uint32_t cp; } classes[]={
        {"L",0x41},{"R",0x5d0},{"AL",0x628},{"EN",0x31},{"ES",0x2b},{"ET",0x24},
        {"AN",0x661},{"CS",0x2c},{"NSM",0x300},{"BN",0xad},{"B",0x2029},{"S",9},
        {"WS",0x20},{"ON",0x21},{"LRE",0x202a},{"LRO",0x202d},{"RLE",0x202b},{"RLO",0x202e},
        {"PDF",0x202c},{"LRI",0x2066},{"RLI",0x2067},{"FSI",0x2068},{"PDI",0x2069}};
    FILE* input=fopen(path,"rb"); char record[65536],text[MAX_CP*4];
    size_t positions[MAX_CP+1],order[MAX_CP],expected_order[MAX_CP],levels=0,reordered=0,cases=0,source_line=0;
    int expected[MAX_CP]; uint8_t actual[MAX_CP];
    CHECK(input);
    while(fgets(record,sizeof(record),input)) {
        char *cursor,*end,*separator; size_t count=0,bytes=0,i,r; unsigned bitset,bit;
        source_line++; if(record[0]=='#' || record[0]=='\r' || record[0]=='\n')continue;
        if(!strncmp(record,"@Levels:",8)) {
            cursor=record+8; levels=0;
            while(1) { while(*cursor==' ' || *cursor=='\t')cursor++;
                if(*cursor=='x') { CHECK(levels<MAX_CP); expected[levels++]=-1; cursor++; }
                else { long value=strtol(cursor,&end,10); if(end==cursor)break; CHECK(levels<MAX_CP); expected[levels++]=(int)value; cursor=end; }
            }
            continue;
        }
        if(!strncmp(record,"@Reorder:",9)) {
            cursor=record+9; reordered=0;
            while(1) { unsigned long value=strtoul(cursor,&end,10); if(cursor==end)break; CHECK(reordered<MAX_CP); expected_order[reordered++]=(size_t)value; cursor=end; }
            continue;
        }
        if(record[0]=='@')continue;
        separator=strchr(record,';'); CHECK(separator); *separator=0; bitset=(unsigned)strtoul(separator+1,NULL,16); CHECK(bitset && !(bitset&~7u));
        cursor=strtok(record," \t\r\n");
        while(cursor) {
            for(i=0;i<sizeof(classes)/sizeof(*classes);i++)if(!strcmp(cursor,classes[i].name))break;
            CHECK(i<sizeof(classes)/sizeof(*classes) && count<MAX_CP); positions[count++]=bytes; bytes+=encode(classes[i].cp,text+bytes); cursor=strtok(NULL," \t\r\n");
        }
        CHECK(count==levels && count); positions[count]=bytes;
        for(bit=1;bit<=4;bit*=2)if(bitset&bit) {
            xui_text_bidi b=NULL; xui_bidi_line_t line={0}; size_t ordered=0;
            unsigned base=bit==1?XUI_BIDI_AUTO_LTR:bit==2?XUI_BIDI_LTR:XUI_BIDI_RTL;
            CHECK(xuiInternalTextBidiCreate(text,bytes,base,&b)==XUI_OK && xuiInternalTextBidiLine(b,0,bytes,&line)==XUI_OK); memset(actual,255,count);
            for(r=0;r<line.count;r++) {
                if((line.runs[r].start<bytes && ((unsigned char)text[line.runs[r].start]&0xc0)==0x80) ||
                   (line.runs[r].end<bytes && ((unsigned char)text[line.runs[r].end]&0xc0)==0x80)) {
                    fprintf(stderr,"BidiTest line %zu base %u: run %zu..%zu level %u splits UTF-8\n",source_line,base,line.runs[r].start,line.runs[r].end,line.runs[r].level); exit(1);
                }
                size_t start=boundary_index(positions,count,line.runs[r].start),stop=boundary_index(positions,count,line.runs[r].end);
                for(i=start;i<stop;i++) { CHECK(actual[i]==255); actual[i]=line.runs[r].level; }
                if(line.runs[r].level&1) { for(i=stop;i>start;) { --i; if(expected[i]>=0) { CHECK(ordered<count); order[ordered++]=i; } } }
                else { for(i=start;i<stop;i++)if(expected[i]>=0) { CHECK(ordered<count); order[ordered++]=i; } }
            }
            for(i=0;i<count;i++)if(expected[i]>=0 && actual[i]!=expected[i]) {
                fprintf(stderr,"BidiTest line %zu, base %u, cp %zu: level %u expected %d\n",source_line,base,i,actual[i],expected[i]); exit(1);
            }
            CHECK(ordered==reordered);
            for(i=0;i<ordered;i++)if(order[i]!=expected_order[i]) {
                fprintf(stderr,"BidiTest line %zu, base %u, visual %zu: got %zu expected %zu\n",source_line,base,i,order[i],expected_order[i]); exit(1);
            }
            cases++; xuiInternalTextBidiLineFree(&line); xuiInternalTextBidiFree(b); CHECK(!live && !live_bytes);
        }
    }
    CHECK(!ferror(input) && cases==770241); fclose(input);
    printf("Unicode 17 BidiTest: %zu class/direction cases, L1 levels and L2 visual order passed\n",cases);
}
static void discretionary_levels(void)
{
    const char* samples[]={"\xe2\x80\xae" "A \xc2\xad" "B\xe2\x80\xac",
        "\xe2\x80\xae" "A\t \xc2\xad" "B\xe2\x80\xac"};
    for(unsigned i=0;i<2;i++){
        xui_text_bidi bidi=NULL; xui_bidi_line_t line={0}; uint8_t original;
        size_t hyphen=5+i;
        CHECK(xuiInternalTextBidiCreate(samples[i],strlen(samples[i]),XUI_BIDI_LTR,&bidi)==XUI_OK &&
            xuiInternalTextBidiLine(bidi,0,hyphen+2,&line)==XUI_OK);
        CHECK(line.levels[hyphen-1]==0 && xuiInternalTextBidiLevel(bidi,hyphen-1,&original)==XUI_OK && original==1);
        CHECK(xuiInternalTextBidiRestoreHyphenLevels(bidi,0,hyphen,line.levels)==XUI_OK && line.levels[hyphen-1]==1);
        if(i)CHECK(line.levels[4]==0); /* A segment separator keeps its L1 reset. */
        CHECK(line.levels[hyphen]==0 && xuiInternalTextBidiLevel(bidi,hyphen-1,&original)==XUI_OK && original==1);
        CHECK(xuiInternalTextBidiRestoreHyphenLevels(bidi,0,hyphen+1,line.levels)==XUI_ERROR_INVALID_ARGUMENT &&
            xuiInternalTextBidiRestoreHyphenLevels(bidi,0,3,line.levels)==XUI_ERROR_INVALID_ARGUMENT &&
            xuiInternalTextBidiRestoreHyphenLevels(bidi,0,hyphen,NULL)==XUI_ERROR_INVALID_ARGUMENT);
        xuiInternalTextBidiLineFree(&line);xuiInternalTextBidiFree(bidi);CHECK(!live && !live_bytes);
    }
    puts("Selected SHY levels: preceding whitespace restored, segment separator retains L1, paragraph immutable and complete cleanup passed");
}
int main(int argc,char** argv)
{
    CHECK(argc>=1 && argc<=3); contracts(); discretionary_levels(); allocation_failures(); if(argc>1)conformance(argv[1]); if(argc>2)class_conformance(argv[2]);
    puts("Bidi bridge: strict UTF-8, copied lifetime, byte boundaries, CRLF, L1 immutability, mirrors and failure rollback passed"); return 0;
}
