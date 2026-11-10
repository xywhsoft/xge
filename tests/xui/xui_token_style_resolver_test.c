#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(v) do{++checks;if(!(v)){fprintf(stderr,"token style resolver:%d: %s\n",__LINE__,#v);failed=1;goto cleanup;}}while(0)
static uint32_t paints[4096];static int paintCount;
static xui_draw_text_proc baseText;
static xui_draw_text_spans_proc baseSpans;
static xui_draw_text_shape_range_spans_proc baseShapeSpans;
static int has(uint32_t color){for(int i=0;i<paintCount;++i)if(paints[i]==color)return 1;return 0;}
static int draw(xui_proxy p,xui_draw_context d,const xui_text_item_t* text,xui_rect_t rect,uint32_t color,uint32_t flags)
{if(paintCount<4096)paints[paintCount++]=color;return baseText(p,d,text,rect,color,flags);}
static int spans(xui_proxy p,xui_draw_context d,const xui_text_item_t* text,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* rows,int count)
{for(int i=0;i<count && paintCount<4096;++i)paints[paintCount++]=rows[i].iColor;return baseSpans(p,d,text,rect,color,flags,rows,count);}
static int shapeSpans(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float x,const xui_text_paint_span_t* rows,int count)
{for(int i=0;i<count && paintCount<4096;++i)paints[paintCount++]=rows[i].iColor;return baseShapeSpans(p,d,shape,start,end,rect,color,flags,x,rows,count);}
typedef struct style_state {xui_code_document doc;uint32_t version;int mode,calls,offscreen,hidden,start,end;} style_state;
static int resolve(xui_code_document_t* doc,const xui_code_token_t* token,int* style,void* user)
{
    style_state* s=user;++s->calls;if(token->iStartOffset>=s->hidden)s->offscreen=1;
    if(doc!=s->doc || xuiCodeDocumentGetVersion(doc)!=s->version || token->iStartOffset!=s->start || token->iEndOffset!=s->end)return XUI_ERROR_NOT_FOUND;
    *style=s->mode==2?999999:s->mode==3?XUI_CODE_STYLE_CUSTOM_BASE+1:XUI_CODE_STYLE_CUSTOM_BASE;
    return s->mode==4?XUI_ERROR_NOT_FOUND:XUI_OK;
}
int main(void)
{
    xui_test_proxy_state_t proxy;xui_context ctx=NULL;xui_surface target=NULL;xui_font font=NULL;xui_widget edit=NULL;
    xui_code_edit_desc_t desc={0};xui_rect_i_t damage={0,0,640,160};int failed=0;
    char text[1024]="int alpha = 7; // comment\nconst char* s=\"value\";\n";size_t length=strlen(text);for(int i=0;i<90;++i)text[length++]='\n';int hidden=(int)length;strcpy(text+length,"int hidden=1;\n");
    xuiTestProxyInit(&proxy);baseText=proxy.tProxy.drawText;proxy.tProxy.drawText=draw;baseSpans=proxy.tProxy.drawTextSpans;proxy.tProxy.drawTextSpans=spans;
    baseShapeSpans=proxy.tProxy.drawTextShapeRangeSpans;proxy.tProxy.drawTextShapeRangeSpans=shapeSpans;
    CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,640,160)==XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy,&target,640,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    desc.iSize=sizeof(desc);desc.pFont=font;desc.sText=text;desc.sLanguage="c";
    CHECK(xuiCodeEditCreate(ctx,&edit,&desc)==XUI_OK && xuiSetRootWidget(ctx,edit)==XUI_OK);
    CHECK(xuiWidgetSetRect(edit,(xui_rect_t){0,0,640,160})==XUI_OK);
    xui_code_style_t custom={0};custom.iSize=sizeof(custom);custom.iForeground=0xe10102ffu;
    CHECK(xuiCodeEditSetStyle(edit,XUI_CODE_STYLE_CUSTOM_BASE,&custom)==XUI_OK);custom.iForeground=0;CHECK(xuiCodeEditSetStyle(edit,XUI_CODE_STYLE_CUSTOM_BASE+1,&custom)==XUI_OK);
    xui_style_property_t property={0};property.iSize=sizeof(property);property.sName="codeedit.text.color";property.tValue.iSize=sizeof(property.tValue);property.tValue.iType=XUI_STYLE_VALUE_COLOR;property.tValue.iColor=0x123abcffu;
    CHECK(xuiStyleSetDefault(ctx,&property,1)==XUI_OK);
    style_state state={.doc=xuiCodeEditGetDocument(edit),.mode=1,.hidden=hidden,.start=4,.end=9};state.version=xuiCodeDocumentGetVersion(state.doc);
    xui_code_token_t tokens[128],after[128];int count=0,afterCount=0;
    CHECK(xuiCodeLexerCTokenize(text,(int)strlen(text),tokens,128,&count)==XUI_OK && count>5);
    xui_code_token_buffer buffer=xuiCodeEditGetTokenBuffer(edit);CHECK(buffer);
    CHECK(xuiCodeTokenBufferSetRange(buffer,tokens,count,state.version,0,(int)strlen(text))==XUI_OK);
    CHECK(xuiLayout(ctx)==XUI_OK && xuiRender(ctx,target,&damage,1)==XUI_OK);
    CHECK(xuiCodeTokenBufferGetTokens(buffer,state.version,tokens,128,&count)==XUI_OK);
    int shapes=proxy.iTextShapeCount;
    CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK && !state.calls);
    xui_code_token_style_proc callback=NULL;void* user=NULL;
    CHECK(xuiCodeEditGetTokenStyleResolver(edit,&callback,&user)==XUI_OK && callback==resolve && user==&state);
    paintCount=0;CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(0xe10102ffu));
    CHECK(state.calls>0 && !state.offscreen && proxy.iTextShapeCount==shapes);
    CHECK(xuiCodeTokenBufferGetTokens(buffer,state.version,after,128,&afterCount)==XUI_OK && afterCount==count && !memcmp(tokens,after,(size_t)count*sizeof(*tokens)));
    state.start=(int)(strstr(text,"s=\"")-text);state.end=state.start+1;
    CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK);paintCount=0;
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(0xe10102ffu) && proxy.iTextShapeCount==shapes);
    state.start=4;state.end=9;
    /* The long-line chunk renderer must use the same resolver without reshaping. */
    static char longLine[70001];memset(longLine,' ',sizeof(longLine)-1);memcpy(longLine,"int alpha = 7;",14);
    CHECK(xuiCodeEditSetText(edit,longLine)==XUI_OK);state.version=xuiCodeDocumentGetVersion(state.doc);
    CHECK(xuiCodeLexerCTokenize(longLine,(int)strlen(longLine),tokens,128,&count)==XUI_OK);
    CHECK(xuiCodeTokenBufferSetRange(buffer,tokens,count,state.version,0,(int)strlen(longLine))==XUI_OK);
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK);shapes=proxy.iTextShapeCount;
    CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK);paintCount=0;
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(0xe10102ffu) && proxy.iTextShapeCount==shapes);
    CHECK(xuiCodeTokenBufferGetTokens(buffer,state.version,after,128,&afterCount)==XUI_OK && afterCount==count && !memcmp(tokens,after,(size_t)count*sizeof(*tokens)));
    for(int mode=2;mode<=4;++mode){
        state.mode=mode;CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK);paintCount=0;
        CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && !has(0xe10102ffu) && has(0x123abcffu) && proxy.iTextShapeCount==shapes);
    }
    state.mode=1;state.version++;CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK);paintCount=0;
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && !has(0xe10102ffu));state.version--;
    /* Force the ordinary per-span renderer as well as the retained spans path. */
    proxy.tProxy.drawTextSpans=NULL;CHECK(xuiCodeEditSetTokenStyleResolver(edit,resolve,&state)==XUI_OK);paintCount=0;
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(0xe10102ffu));proxy.tProxy.drawTextSpans=spans;
    CHECK(xuiCodeEditSetTokenStyleResolver(edit,NULL,&state)==XUI_OK);callback=resolve;user=&state;
    CHECK(xuiCodeEditGetTokenStyleResolver(edit,&callback,&user)==XUI_OK && !callback && !user);paintCount=0;
    CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && !has(0xe10102ffu) && has(0x123abcffu));
    CHECK(xuiCodeTokenBufferGetTokens(buffer,state.version,after,128,&afterCount)==XUI_OK && afterCount==count && !memcmp(tokens,after,(size_t)count*sizeof(*tokens)));
    callback=resolve;user=&state;CHECK(xuiCodeEditGetTokenStyleResolver(NULL,&callback,&user)==XUI_ERROR_INVALID_ARGUMENT && !callback && !user);
    CHECK(xuiCodeEditGetTokenStyleResolver(edit,NULL,NULL)==XUI_ERROR_INVALID_ARGUMENT && xuiCodeEditSetTokenStyleResolver(NULL,resolve,&state)==XUI_ERROR_INVALID_ARGUMENT);
    printf("xui_token_style_resolver_test passed: %u checks; visible foreground, lexical cache/CSS fallback, no reshaping, span paths and clearing.\n",checks);
cleanup:
    if(ctx)xuiDestroy(ctx);
    if(target)proxy.tProxy.surfaceDestroy(&proxy.tProxy,target);
    if(font)proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    return failed;
}
