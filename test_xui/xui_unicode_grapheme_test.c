#include "../src/xui_unicode_core.h"
#include "../src/xge_unicode_grapheme.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (case=%u offset=%d)\n",__FILE__,__LINE__,#e,current_case,current_offset); exit(1); } } while (0)
static unsigned current_case;
static int current_offset;
typedef struct reader_t { const unsigned char* text; int length, base; size_t reads; } reader_t;
static int read_byte(void* user,int at,unsigned char* byte)
{
    reader_t* reader=user;
    CHECK(at>=0 && at<reader->length);
    reader->reads++;
    *byte=at<reader->base?'x':reader->text[at-reader->base];
    return 1;
}
static uint32_t decode(const void* text,size_t length,size_t* at)
{
    const unsigned char* bytes=text;
    size_t start=*at,scan=start+1;uint32_t cp=bytes[start],minimum;unsigned need;
    if(cp<128){*at=scan;return cp;}
    if(cp>=0xf0 && cp<0xf5){cp&=7;need=3;minimum=0x10000;}
    else if(cp>=0xe0 && cp<0xf0){cp&=15;need=2;minimum=0x800;}
    else if(cp>=0xc2 && cp<0xe0){cp&=31;need=1;minimum=0x80;}
    else goto invalid;
    while(need--){if(scan>=length || (bytes[scan]&0xc0)!=0x80)goto invalid;cp=(cp<<6)|(bytes[scan++]&63);}
    if(cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff))goto invalid;
    *at=scan;return cp;
invalid:
    *at=start+1;return 0xfffd;
}
static int encode(unsigned cp,char* text)
{
    if(cp<128){text[0]=(char)cp;return 1;}
    if(cp<2048){text[0]=(char)(0xc0|(cp>>6));text[1]=(char)(0x80|(cp&63));return 2;}
    if(cp<65536){text[0]=(char)(0xe0|(cp>>12));text[1]=(char)(0x80|((cp>>6)&63));text[2]=(char)(0x80|(cp&63));return 3;}
    text[0]=(char)(0xf0|(cp>>18));text[1]=(char)(0x80|((cp>>12)&63));text[2]=(char)(0x80|((cp>>6)&63));text[3]=(char)(0x80|(cp&63));return 4;
}
static void check_text(const char* text,int length,const unsigned char* boundaries)
{
    char map[2048];reader_t reader={(const unsigned char*)text,length,0,0};int at;
    CHECK(length>0 && length<(int)sizeof(map));
    __xgeGraphemeMap(text,(size_t)length,map,decode);
    for(at=0;at<length;at++)CHECK((map[at]==XGE_GRAPHEME_BREAK)==!!boundaries[at+1]);
    for(at=0;at<=length;at++){
        int previous=at?at-1:0,next=at<length?at+1:length,clamp=at;
        while(previous>0 && !boundaries[previous])previous--;
        while(next<length && !boundaries[next])next++;
        while(clamp>0 && !boundaries[clamp])clamp--;
        current_offset=at;
        CHECK(xuiInternalTextGraphemePrevRead(read_byte,&reader,length,at)==previous);
        CHECK(xuiInternalTextGraphemeNextRead(read_byte,&reader,length,at)==next);
        CHECK(xuiInternalTextGraphemeClampRead(read_byte,&reader,length,at)==clamp);
        CHECK(xuiInternalTextGraphemePrev(text,length,at)==previous);
        CHECK(xuiInternalTextGraphemeNext(text,length,at)==next);
        CHECK(xuiInternalTextGraphemeClamp(text,length,at)==clamp);
    }
}
static void corpus(const char* path)
{
    FILE* file=fopen(path,"rb");char line[4096];CHECK(file);
    while(fgets(line,sizeof(line),file)){
        char text[2048]={0};unsigned char boundaries[2048]={0};int length=0;char* token;
        char* comment=strchr(line,'#');if(comment)*comment=0;
        token=strtok(line," \t\r\n");if(!token)continue;
        current_case++;
        do {
            if((unsigned char)token[0]==0xc3)boundaries[length]=(unsigned char)token[1]==0xb7;
            else {char* end;unsigned long cp=strtoul(token,&end,16);CHECK(!*end && cp<=0x10ffff && length+4<(int)sizeof(text));length+=encode((unsigned)cp,text+length);}
        }while((token=strtok(NULL," \t\r\n")));
        check_text(text,length,boundaries);
    }
    CHECK(!ferror(file) && current_case==766);fclose(file);
    printf("Unicode 17 official grapheme corpus: %u cases; full forward maps and pointer/callback Next/Prev/Clamp at every UTF-8 byte passed\n",current_case);
}
static void stress(void)
{
    reader_t reader;const char* emoji="\xf0\x9f\x91\xa9";
    char* text=malloc(200001);int i,length=200001;CHECK(text);
    text[0]='a';for(i=1;i<length;i+=2){text[i]=(char)0xcc;text[i+1]=(char)0x81;}
    reader=(reader_t){(unsigned char*)text,length,0,0};
    CHECK(xuiInternalTextGraphemePrevRead(read_byte,&reader,length,length)==0);
    CHECK(xuiInternalTextGraphemeNextRead(read_byte,&reader,length,length/2)==length);
    CHECK(reader.reads<(size_t)length*40);free(text);
    reader=(reader_t){(const unsigned char*)emoji,INT_MAX,INT_MAX-4,0};
    CHECK(xuiInternalTextGraphemePrevRead(read_byte,&reader,INT_MAX,INT_MAX)==INT_MAX-4);
    CHECK(xuiInternalTextGraphemeNextRead(read_byte,&reader,INT_MAX,INT_MAX-3)==INT_MAX);
    CHECK(xuiInternalTextGraphemeClampRead(read_byte,&reader,INT_MAX,INT_MAX-2)==INT_MAX-4);
    CHECK(reader.reads<128);
    reader=(reader_t){(const unsigned char*)"x",INT_MAX,INT_MAX-1,0};
    CHECK(xuiInternalTextGraphemePrevRead(read_byte,&reader,INT_MAX,INT_MAX)==INT_MAX-1);
    CHECK(reader.reads<16);
    puts("Grapheme streaming: 200001-byte combining cluster, bounded ASCII context and INT_MAX virtual UTF-8 offsets passed");
}
static void malformed(void)
{
    const char bytes[]={'a',(char)0x80,(char)0xcc,(char)0x81,'b'};
    const unsigned char boundaries[]={1,1,0,0,1,1};
    char tail[40];unsigned char ends[41];unsigned i;
    check_text(bytes,sizeof(bytes),boundaries);
    memset(tail,0x80,sizeof(tail));memset(ends,1,sizeof(ends));
    check_text(tail,sizeof(tail),ends);
    for(i=0;i<256;i++){
        char text[]={(char)i,'a'};char map[2];unsigned char expected[3]={1,0,1};
        __xgeGraphemeMap(text,2,map,decode);expected[1]=map[0]==XGE_GRAPHEME_BREAK;
        check_text(text,2,expected);
    }
    puts("Malformed UTF-8: per-byte replacement, combining suffix and bounded continuation runs agree with editing navigation");
}
int main(int argc,char** argv)
{
    if(argc==3 && !strcmp(argv[1],"--properties")){
        uint32_t cp;FILE* file=fopen(argv[2],"wb");CHECK(file);
        for(cp=0;cp<=0x10ffff;cp++)CHECK(fputc(__xgeGraphemeProperty(cp),file)!=EOF);
        CHECK(!ferror(file) && fclose(file)==0);return 0;
    }
    corpus(argc>1?argv[1]:"test_xui/data/unicode-17/GraphemeBreakTest.txt");
    malformed();stress();return 0;
}
