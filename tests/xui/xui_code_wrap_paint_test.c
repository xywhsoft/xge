/* Verify submitted glyph ranges, not just hit-test geometry. CPU-only. */
#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CHECK(e) do { ++checks; if (!(e)) { fprintf(stderr,"line %d: %s\n",__LINE__,#e); exit(1); } } while (0)
static int checks;
static xui_proxy_t native;
static char painted[65536];
static size_t paintedSize;
static struct { const void* key; char* text; } shapes[512];
static int shapeCount;
static void record(const char* s,int n)
{
    if(n<0)n=(int)strlen(s);
    CHECK(paintedSize+(size_t)n<sizeof(painted));
    memcpy(painted+paintedSize,s,(size_t)n);paintedSize+=(size_t)n;painted[paintedSize]=0;
}
static int draw(xui_proxy p,xui_draw_context d,const xui_text_item_t* t,xui_rect_t r,uint32_t c,uint32_t f)
{record(t->sText,t->iTextSize);return native.drawText(p,d,t,r,c,f);}
static int spans(xui_proxy p,xui_draw_context d,const xui_text_item_t* t,xui_rect_t r,uint32_t c,uint32_t f,const xui_text_paint_span_t* s,int n)
{
    int bytes=t->iTextSize<0?(int)strlen(t->sText):t->iTextSize;
    for(int i=0;i<n;++i)CHECK(s[i].iStart>=0 && s[i].iEnd<=bytes);
    record(t->sText,bytes);return native.drawTextSpans(p,d,t,r,c,f,s,n);
}
static int shape(xui_proxy p,const xui_text_item_t* t,xui_text_shape_t* s)
{
    int status=native.textShape(p,t,s);if(status!=XUI_OK)return status;
    int i;for(i=0;i<shapeCount && shapes[i].key!=s->pClusters;++i){}
    if(i==shapeCount){CHECK(shapeCount<512);++shapeCount;}else free(shapes[i].text);
    int n=t->iTextSize<0?(int)strlen(t->sText):t->iTextSize;
    shapes[i].text=malloc((size_t)n+1);CHECK(shapes[i].text!=NULL);
    memcpy(shapes[i].text,t->sText,(size_t)n);shapes[i].text[n]=0;shapes[i].key=s->pClusters;return status;
}
static void recordShape(const xui_text_shape_t* s,int a,int b)
{
    int i;for(i=0;i<shapeCount && shapes[i].key!=s->pClusters;++i){}
    CHECK(i<shapeCount);CHECK(a>=0 && b>=a && b<=(int)strlen(shapes[i].text));record(shapes[i].text+a,b-a);
}
static int range(xui_proxy p,xui_draw_context d,const xui_text_shape_t* s,int a,int b,xui_rect_t r,uint32_t c,uint32_t f)
{recordShape(s,a,b);return native.drawTextShapeRange(p,d,s,a,b,r,c,f);}
static int rangeSpans(xui_proxy p,xui_draw_context d,const xui_text_shape_t* s,int a,int b,xui_rect_t r,uint32_t c,uint32_t f,float x,const xui_text_paint_span_t* v,int n)
{recordShape(s,a,b);return native.drawTextShapeRangeSpans(p,d,s,a,b,r,c,f,x,v,n);}
static int lex(xui_code_document document,int first,int last,xui_code_token_t* tokens,int capacity,int* count,void* user)
{
    (void)user;int n=0;
    for(int line=first;line<=last;++line){
        int start,end;if(xuiCodeDocumentGetLineRange(document,line,&start,&end)!=XUI_OK)break;
        if(end==start)continue;
        if(tokens && n<capacity)tokens[n]=(xui_code_token_t){.iStartOffset=start,.iEndOffset=end,.iKind=XUI_CODE_TOKEN_KEYWORD,.iStyle=XUI_CODE_TOKEN_KEYWORD};
        ++n;
    }
    *count=n;return XUI_OK;
}

static void runCase(int path,const char* text,float width)
{
    xui_test_proxy_state_t state;xuiTestProxyInit(&state);
    if(path==2)xuiTestProxyEnableRetainedText(&state);
    native=state.tProxy;
    state.tProxy.drawText=draw;state.tProxy.drawTextSpans=path==0?NULL:spans;
    state.tProxy.textShape=shape;
    if(path==2){state.tProxy.drawTextShapeRange=range;state.tProxy.drawTextShapeRangeSpans=rangeSpans;}
    xui_context context=NULL;xui_widget editor=NULL;xui_font font=NULL;xui_surface target=NULL;
    CHECK(xuiCreate(&context)==XUI_OK);CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK);
    CHECK(state.tProxy.fontLoadFile(&state.tProxy,&font,"wrap.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(context,font)==XUI_OK);CHECK(xuiInputViewport(context,width,12000)==XUI_OK);
    xui_code_edit_desc_t desc={.iSize=sizeof(desc),.pFont=font,.sText=text,.bReadonly=1,.bWordWrap=1,.iTabColumns=4};
    CHECK(xuiCodeEditCreate(context,&editor,&desc)==XUI_OK);CHECK(xuiSetRootWidget(context,editor)==XUI_OK);
    xui_code_language_t language={.iSize=sizeof(language),.sId="paint-test",.sName="Paint test",.onLex=lex};
    CHECK(xuiCodeLanguageRegistryRegister(xuiCodeEditGetLanguageRegistry(editor),&language)==XUI_OK);
    CHECK(xuiCodeEditSetLanguage(editor,"paint-test")==XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,width,12000})==XUI_OK);
    CHECK(xuiLayout(context)==XUI_OK);CHECK(xuiUpdate(context,.016f)==XUI_OK);
    CHECK(xuiTestSurfaceCreate(&state,&target,(int)width,12000,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    paintedSize=0;painted[0]=0;int status=xuiRender(context,target,NULL,0);
    if(status!=XUI_OK)fprintf(stderr,"render path=%d status=%d\n",path,status);
    CHECK(status==XUI_OK);
    char expected[65536];size_t n=0;
    for(const char* p=text;*p;++p)if(*p!='\n' && *p!='\t')expected[n++]=*p;
    expected[n]=0;
    if(strcmp(painted,expected))fprintf(stderr,"path=%d width=%.0f expected=%zu painted=%zu\n",path,width,n,paintedSize);
    CHECK(!strcmp(painted,expected));
    /* Changing the width must invalidate both the row index and paint ranges. */
    if(width>500){
        xui_rect_t first,last;CHECK(xuiCodeEditGetTextOffsetRect(editor,0,&first)==XUI_OK);
        const char* newline=strchr(text,'\n');
        if(newline){CHECK(xuiCodeEditGetTextOffsetRect(editor,(int)(newline-text),&last)==XUI_OK);CHECK(first.fY==last.fY);}
        CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,160,12000})==XUI_OK);
        CHECK(xuiLayout(context)==XUI_OK);CHECK(xuiUpdate(context,.016f)==XUI_OK);
        paintedSize=0;painted[0]=0;CHECK(xuiRender(context,target,NULL,0)==XUI_OK);CHECK(!strcmp(painted,expected));
    }
    CHECK(!strcmp(xuiCodeEditGetText(editor),text));
    state.tProxy.surfaceDestroy(&state.tProxy,target);xuiDestroy(context);state.tProxy.fontDestroy(&state.tProxy,font);
    for(int i=0;i<shapeCount;++i)free(shapes[i].text);
    shapeCount=0;
}
int main(void)
{
    const char* shortLines="[Task 2] main.xl / single-file\nCompiler: D:/GIT/x-lang/release/win_x64/xl.exe\n[Task failed, 0 ms] save the input and request a new task\n\n中文与 emoji: 😀 é tail\n";
    char longLine[6000];size_t at=0;
    for(int i=0;i<220;++i){const char* word="alpha beta;gamma/delta ";size_t n=strlen(word);memcpy(longLine+at,word,n);at+=n;}
    strcpy(longLine+at,"last-word\nnext\tline\n");
    for(int path=0;path<3;++path){runCase(path,shortLines,900);runCase(path,shortLines,160);runCase(path,longLine,400);runCase(path,"one-word\nno_spaces\n\ntrailing \n",900);}
    printf("xui_code_wrap_paint_test passed: %d checks\n",checks);return 0;
}
