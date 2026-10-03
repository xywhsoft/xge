/* Actual bounded C point reader / outline converter. FontTools supplies the
 * serialized coordinate oracle; stb rasterizes independent flattened glyphs. */
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "../lib/stb/stb_truetype.h"
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while (0)
typedef struct owner_t { void* pointer; } owner_t;
static owner_t owners[8];
static size_t live, calls, fail_at;
static owner_t* owner(void* pointer)
{ size_t i; for(i=0;i<8;i++) if(owners[i].pointer==pointer) return &owners[i]; CHECK(0); return NULL; }
static void* tracked_malloc(size_t bytes)
{ void* p; if(++calls==fail_at)return NULL; p=malloc(bytes); if(p){owner(NULL)->pointer=p;live++;}return p; }
static void* tracked_realloc(void* pointer,size_t bytes)
{ owner_t* entry;void* p;if(!pointer)return tracked_malloc(bytes);if(++calls==fail_at)return NULL;
  entry=owner(pointer);p=realloc(pointer,bytes);if(p)entry->pointer=p;return p; }
static void tracked_free(void* pointer)
{ if(pointer){owner(pointer)->pointer=NULL;CHECK(live);live--;free(pointer);} }
#define xrtMalloc tracked_malloc
#define xrtRealloc tracked_realloc
#define xrtFree tracked_free
#include "../src/xge_font_contours.inl"
#include "../src/xge_font_contour_raster.inl"
static unsigned char* read_file(const char* path,size_t* size)
{ FILE* file=fopen(path,"rb");long length;unsigned char* data;
  CHECK(file && !fseek(file,0,SEEK_END) && (length=ftell(file))>0 && !fseek(file,0,SEEK_SET));
  data=malloc((size_t)length);CHECK(data && fread(data,1,(size_t)length,file)==(size_t)length);fclose(file);*size=(size_t)length;return data; }
static const unsigned char* table(const unsigned char* data,size_t bytes,const char* tag,uint32_t* length)
{ unsigned i,n;CHECK(bytes>=12);n=__xgeTTU16(data+4);CHECK(__xgeTTRange(12,(size_t)n*16,bytes));
  for(i=0;i<n;i++){const unsigned char* record=data+12+i*16;uint32_t at=__xgeTTU32(record+8);
    if(!memcmp(record,tag,4)){*length=__xgeTTU32(record+12);CHECK(__xgeTTRange(at,*length,bytes));return data+at;}}
  CHECK(0);return NULL; }
static void put32(unsigned char* data,uint32_t value)
{data[0]=(unsigned char)(value>>24);data[1]=(unsigned char)(value>>16);data[2]=(unsigned char)(value>>8);data[3]=(unsigned char)value;}
static size_t oracle(const xge_tt_source_t* source)
{
    FILE* file=fopen("test/data/xge_opentype_points_fixture.points","rb");unsigned glyph,index,on,end,previous=UINT_MAX;
    double x,y;size_t checked=0,total=0;int result;
    CHECK(file);
    while((result=fscanf(file,"%u %u %lf %lf %u %u",&glyph,&index,&x,&y,&on,&end))==6){
        xge_tt_point_t point={0};
        if(glyph!=previous && previous!=UINT_MAX)CHECK(!__xgeTTPoint(source,previous,(uint32_t)checked,&point));
        if(glyph!=previous){previous=glyph;checked=0;}
        CHECK(index==checked && __xgeTTPoint(source,glyph,index,&point));
        if(fabs(point.x-x)>1e-8 || fabs(point.y-y)>1e-8)fprintf(stderr,"glyph=%u point=%u actual=%g,%g oracle=%g,%g\n",glyph,index,point.x,point.y,x,y);
        CHECK(fabs(point.x-x)<1e-8 && fabs(point.y-y)<1e-8 && point.on==on && point.end==end);checked++;total++;
    }
    CHECK(result==EOF);fclose(file);
    {xge_tt_point_t point;CHECK(!__xgeTTPoint(source,previous,(uint32_t)checked,&point));
      CHECK(!__xgeTTPoint(source,source->glyphs,0,&point));CHECK(!__xgeTTPoint(source,1,0,&point));}
    return total;
}
static size_t allocations(const xge_tt_source_t* source)
{
    unsigned glyph;size_t failures=0;
    for(glyph=0;glyph<source->glyphs;glyph++){
        stbtt_vertex* vertices=NULL;int count=0;size_t baseline,point;
        calls=0;CHECK(__xgeTTOutline(source,glyph,&vertices,&count)==1);
        baseline=calls;tracked_free(vertices);CHECK(!live);
        for(point=1;point<=baseline;point++){
            calls=0;fail_at=point;vertices=(stbtt_vertex*)(uintptr_t)1;count=99;
            CHECK(__xgeTTOutline(source,glyph,&vertices,&count)==-1 && !vertices && !count && !live);
            fail_at=0;failures++;
        }
    }
    return failures;
}
static size_t bitmaps(const unsigned char* bytes,const xge_tt_source_t* source)
{
    stbtt_fontinfo font;FILE* pairs=fopen("test/data/xge_opentype_points_fixture.pairs","rb");
    unsigned compound,flat;size_t checked=0;const float scales[]={.008f,.016f,.032f,.048f,.096f,.2f};
    CHECK(pairs && stbtt_InitFont(&font,bytes,0));
    while(fscanf(pairs,"%u %u",&compound,&flat)==2){
        unsigned i;stbtt_vertex* vertices=NULL;int count=0;
        CHECK(__xgeTTOutline(source,compound,&vertices,&count)==1);
        for(i=0;i<sizeof(scales)/sizeof(*scales);i++){
            int x0,y0,x1,y1,fw,fh,fx,fy;unsigned char *reference,*actual;stbtt__bitmap bitmap;
            stbtt_GetGlyphBitmapBox(&font,(int)compound,scales[i],scales[i],&x0,&y0,&x1,&y1);
            reference=stbtt_GetGlyphBitmap(&font,scales[i],scales[i],(int)flat,&fw,&fh,&fx,&fy);
            CHECK(reference && fw==x1-x0 && fh==y1-y0 && fx==x0 && fy==y0);
            actual=calloc((size_t)fw,(size_t)fh);CHECK(actual);bitmap=(stbtt__bitmap){fw,fh,fw,actual};
            stbtt_Rasterize(&bitmap,.35f,vertices,count,scales[i],scales[i],0,0,x0,y0,1,NULL);
            if(memcmp(actual,reference,(size_t)fw*(size_t)fh))fprintf(stderr,"raster mismatch composite=%u flat=%u scale=%g\n",compound,flat,scales[i]);
            CHECK(!memcmp(actual,reference,(size_t)fw*(size_t)fh));free(actual);stbtt_FreeBitmap(reference,NULL);checked++;
        }
        tracked_free(vertices);CHECK(!live);
    }
    fclose(pairs);return checked;
}
static void malformed(const xge_tt_source_t* source)
{
    const unsigned char* record;size_t bytes,at,length;unsigned char* data;unsigned char loca[8]={0};
    xge_tt_source_t single=*source;xge_tt_point_t point;
    CHECK(__xgeTTRecord(source,9,&record,&bytes));data=malloc(bytes);CHECK(data);memcpy(data,record,bytes);
    single.glyphs=1;single.glyf=data;single.glyf_bytes=(uint32_t)bytes;single.loca=loca;single.loca_bytes=8;single.loca_format=1;
    for(length=1;length<bytes;length++){
        stbtt_vertex* vertices=NULL;int count;put32(loca+4,(uint32_t)length);
        if(__xgeTTOutline(&single,0,&vertices,&count)==1)CHECK(__xgeTTPoint(&single,0,0,&point));
        else CHECK(!vertices && !count);
        tracked_free(vertices);CHECK(!live);
    }
    put32(loca+4,(uint32_t)bytes);
    at=10+(size_t)__xgeTTU16(data)*2;at+=2+__xgeTTU16(data+at);
    data[at]|=8;data[at+1]=255;CHECK(!__xgeTTPoint(&single,0,0,&point));memcpy(data,record,bytes);
    data[0]=data[1]=255;CHECK(!__xgeTTPoint(&single,0,0,&point));free(data);
    CHECK(__xgeTTRecord(source,23,&record,&bytes));data=malloc(bytes);CHECK(data);memcpy(data,record,bytes);
    single.glyf=data;single.glyf_bytes=(uint32_t)bytes;put32(loca+4,(uint32_t)bytes);
    data[12]=data[13]=0;CHECK(!__xgeTTPoint(&single,0,0,&point));
    memcpy(data,record,bytes);data[10]|=0x18;CHECK(!__xgeTTPoint(&single,0,0,&point));free(data);
    single=*source;single.loca_bytes=1;CHECK(!__xgeTTPoint(&single,9,0,&point));
    single=*source;single.glyf_bytes=1;CHECK(!__xgeTTPoint(&single,9,0,&point));
    single=*source;single.loca_format=2;CHECK(!__xgeTTPoint(&single,9,0,&point));
    {xge_tt_walk_t walk={0};uint32_t count;walk.source=source;walk.work=1;
      CHECK(!__xgeTTWalk(&walk,9,UINT32_MAX,NULL,NULL,&count));}
    /* Deterministic byte mutations run through both readers under sanitizers.
     * Some mutations are valid; every published outline must own all storage. */
    CHECK(__xgeTTRecord(source,9,&record,&bytes));data=malloc(bytes);CHECK(data);
    single=*source;single.glyphs=1;single.glyf=data;single.glyf_bytes=(uint32_t)bytes;
    single.loca=loca;single.loca_bytes=8;single.loca_format=1;put32(loca+4,(uint32_t)bytes);
    for(at=0;at<bytes;at++){
        stbtt_vertex* vertices=NULL;int count,result;memcpy(data,record,bytes);data[at]^=0x81;
        result=__xgeTTOutline(&single,0,&vertices,&count);CHECK(result==0 || result==1);
        if(!result)CHECK(!vertices && !count);
        tracked_free(vertices);CHECK(!live);
    }
    free(data);
}
static void real_simple_glyphs(void)
{
    const char* paths[]={"C:/Windows/Fonts/calibri.ttf","C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"};unsigned f;
    for(f=0;f<sizeof(paths)/sizeof(*paths);f++){
        FILE* exists=fopen(paths[f],"rb");size_t bytes;unsigned char* data;stbtt_fontinfo font;
        const unsigned char *head,*maxp,*glyf,*loca;uint32_t hl,ml,gl,ll,glyph;size_t checked=0;
        xge_tt_source_t source={0};
        if(!exists)continue;
        fclose(exists);data=read_file(paths[f],&bytes);CHECK(stbtt_InitFont(&font,data,0));
        head=table(data,bytes,"head",&hl);maxp=table(data,bytes,"maxp",&ml);
        glyf=table(data,bytes,"glyf",&gl);loca=table(data,bytes,"loca",&ll);
        CHECK(__xgeTTSourceInit(&source,head,hl,maxp,ml,glyf,gl,loca,ll));
        for(glyph=0;glyph<source.glyphs;glyph+=17){
            const unsigned char* record;size_t record_bytes;stbtt_vertex *vertices=NULL,*reference_vertices=NULL;
            int count,reference_count,x0,y0,x1,y1;unsigned char *actual,*reference;stbtt__bitmap bitmap;size_t i;
            CHECK(__xgeTTRecord(&source,glyph,&record,&record_bytes));
            if(record_bytes<10 || __xgeTTS16(record)<=0)continue;
            CHECK(__xgeTTOutline(&source,glyph,&vertices,&count)==1);
            reference_count=stbtt_GetGlyphShape(&font,(int)glyph,&reference_vertices);CHECK(reference_count>0 && count>0);
            stbtt_GetGlyphBitmapBox(&font,(int)glyph,.024f,.024f,&x0,&y0,&x1,&y1);CHECK(x1>=x0 && y1>=y0);
            if(x1==x0 || y1==y0){tracked_free(vertices);stbtt_FreeShape(&font,reference_vertices);continue;}
            actual=calloc((size_t)(x1-x0),(size_t)(y1-y0));reference=calloc((size_t)(x1-x0),(size_t)(y1-y0));CHECK(actual && reference);
            bitmap=(stbtt__bitmap){x1-x0,y1-y0,x1-x0,actual};
            stbtt_Rasterize(&bitmap,.35f,vertices,count,.024f,.024f,0,0,x0,y0,1,NULL);
            bitmap.pixels=reference;stbtt_Rasterize(&bitmap,.35f,reference_vertices,reference_count,.024f,.024f,0,0,x0,y0,1,NULL);
            /* Rotating a closed contour changes floating-point edge summation
             * order. Accept at most one alpha level, never a changed outline. */
            for(i=0;i<(size_t)bitmap.w*(size_t)bitmap.h;i++){
                if(abs((int)actual[i]-(int)reference[i])>1)fprintf(stderr,"real glyph mismatch font=%s glyph=%u pixel=%zu actual=%u stb=%u\n",paths[f],glyph,i,actual[i],reference[i]);
                CHECK(abs((int)actual[i]-(int)reference[i])<=1);
            }
            free(actual);free(reference);tracked_free(vertices);stbtt_FreeShape(&font,reference_vertices);CHECK(!live);checked++;
        }
        printf("Real simple TrueType glyphs: %s, %zu sampled outlines match independent stb raster (alpha difference <=1)\n",paths[f],checked);
        free(data);
    }
}
int main(void)
{
    size_t bytes,failures,pixels,points;unsigned char* data=read_file("test/data/xge_opentype_points_fixture.ttf",&bytes),*long_loca;
    uint32_t head_bytes,maxp_bytes,glyf_bytes,loca_bytes;const unsigned char *head,*maxp,*glyf,*loca;
    xge_tt_source_t source={0},long_source;unsigned glyph;
    head=table(data,bytes,"head",&head_bytes);maxp=table(data,bytes,"maxp",&maxp_bytes);
    glyf=table(data,bytes,"glyf",&glyf_bytes);loca=table(data,bytes,"loca",&loca_bytes);
    CHECK(__xgeTTSourceInit(&source,head,head_bytes,maxp,maxp_bytes,glyf,glyf_bytes,loca,loca_bytes));
    CHECK(!__xgeTTSourceInit(&long_source,head,53,maxp,maxp_bytes,glyf,glyf_bytes,loca,loca_bytes));
    CHECK(!__xgeTTSourceInit(&long_source,head,head_bytes,maxp,5,glyf,glyf_bytes,loca,loca_bytes));
    points=oracle(&source);long_loca=malloc((size_t)(source.glyphs+1)*4);CHECK(long_loca);
    for(glyph=0;glyph<=source.glyphs;glyph++)put32(long_loca+glyph*4,source.loca_format?
        __xgeTTU32(loca+glyph*4):__xgeTTU16(loca+glyph*2)*2);
    long_source=source;long_source.loca=long_loca;long_source.loca_bytes=(source.glyphs+1)*4;long_source.loca_format=1;
    CHECK(oracle(&long_source)==points);failures=allocations(&source);pixels=bitmaps(data,&source);malformed(&source);
    free(long_loca);free(data);CHECK(!live);real_simple_glyphs();
    printf("Actual C TrueType points: %zu FontTools points (short/long loca), original off-curve indices, compound transforms/offsets/nesting/point attachment; %zu independent stb flattened bitmap comparisons, %zu allocation failures and malformed/mutation checks passed\n",points,pixels,failures);
    return 0;
}
