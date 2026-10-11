#include "xge.h"
#include "xui_document_ui.h"
#include <GL/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr,"native probe line %d: %s\n",__LINE__,#e); exit(1); } } while (0)
enum { WIDTH = 640, HEIGHT = 480 };
static const char* destination;
static const char* font_paths[4];
static xui_proxy_t proxy;
static xui_context context;
static xui_doc_font_set_t fonts;
static xui_surface target;
static unsigned char first[WIDTH*HEIGHT*4], changed[WIDTH*HEIGHT*4], pixels[WIDTH*HEIGHT*4];
static unsigned frames, geometry_checks, images;

static void file_path(char* path, size_t capacity, const char* name)
{ CHECK(snprintf(path, capacity, "%s/%s", destination, name) > 0); }
static xui_document_renderer renderer(xui_document document, unsigned mode)
{
    xui_document_renderer result; xui_document_snapshot snapshot;
    xui_doc_renderer_desc_t desc = {0};
    desc.iSize=sizeof(desc); desc.tFonts=fonts; desc.fParagraphGap=10; desc.fLineGap=3;
    desc.iTextColor=XUI_COLOR_RGBA(25,35,55,255); desc.iLinkColor=XUI_COLOR_RGBA(25,90,190,255);
    desc.iTableBorderColor=XUI_COLOR_RGBA(100,120,140,255);
    desc.iTableHeaderColor=XUI_COLOR_RGBA(220,230,240,255);
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    CHECK(xuiDocumentRendererCreate(context,&desc,&result)==XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(result,snapshot,NULL)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererSetMode(result,mode)==XUI_OK);
    return result;
}
static void draw(xui_document_renderer r, unsigned char* rgba, const char* name)
{
    xui_draw_context draw_context; xui_doc_rect_t size, caret; int exact=0;
    xui_doc_position_t hit={0}; unsigned ink=0;
    CHECK(xuiDocumentRendererLayout(r,WIDTH-40,0,HEIGHT-40)==XUI_OK);
    CHECK(xuiDocumentRendererGetSize(r,&size,&exact)==XUI_OK && exact && size.height>10 && size.height<HEIGHT-40);
    CHECK(xuiDocumentRendererHitTest(r,12,12,&hit)==XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(r,&hit,&caret)==XUI_OK);
    CHECK(isfinite(caret.x) && isfinite(caret.y) && caret.height>0 && caret.y>=0 && caret.y<size.height);
    geometry_checks++;
    CHECK(proxy.surfaceClear(&proxy,target,XUI_COLOR_RGBA(255,255,255,255))==XUI_OK);
    CHECK(proxy.drawBegin(&proxy,&draw_context,target)==XUI_OK);
    CHECK(xuiDocumentRendererDraw(r,draw_context,20,20,(xui_rect_t){0,0,WIDTH,HEIGHT},NULL,0)==XUI_OK);
    CHECK(proxy.drawEnd(&proxy,draw_context)==XUI_OK);
    CHECK(proxy.surfaceReadRGBA(&proxy,target,rgba,WIDTH*4)==XUI_OK);
    for(unsigned i=0;i<WIDTH*HEIGHT;i++) {
        unsigned char* p=rgba+4*i;
        CHECK(p[3]==255);
        if(p[0]<220 || p[1]<220 || p[2]<220) ink++;
    }
    CHECK(ink>1000 && ink<WIDTH*HEIGHT/2);
    if(name) {
        char path[1024]; file_path(path,sizeof(path),name);
        CHECK(xgeImageSavePNG(path,WIDTH,HEIGHT,rgba,WIDTH*4)==XGE_OK); images++;
        printf("Native Document image %s: ink=%u, height=%.2f, caret=(%.2f,%.2f,%.2f,%.2f)\n",
            name,ink,size.height,caret.x,caret.y,caret.width,caret.height);
    }
    frames++;
}
static uint64_t add(xui_document_transaction t,uint64_t parent,unsigned kind,const char* text,unsigned marks)
{
    xui_doc_node_desc_t desc={0}; uint64_t id=0;
    desc.iSize=sizeof(desc);desc.iKind=kind;desc.sText=text;desc.iTextBytes=text?strlen(text):0;
    desc.tAttributes.iMarks=marks;desc.tAttributes.iHeadingLevel=kind==XUI_DOC_HEADING?1:0;
    CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&desc,&id)==XUI_OK);return id;
}
static xui_document create(unsigned profile,uint64_t* editable)
{
    xui_doc_desc_t desc={0};xui_document document;xui_document_transaction t;
    desc.iSize=sizeof(desc);desc.iProfile=profile;desc.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
    if(profile==XUI_DOCUMENT_MARKDOWN) {
        static const char markdown[]="# Native Markdown\n\nShared **bold** and *italic* text, with `code`.\n\n- First item\n- Second item\n\n| Platform | Document |\n| --- | --- |\n| Linux | Actual XGE drawing |\n\n> One kernel for rich text and Markdown.\n";
        xui_document_snapshot snapshot;uint64_t heading;
        CHECK(xuiDocumentLoadMarkdown(document,markdown,sizeof(markdown)-1)==XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot,1,0,&heading)==XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot,heading,0,editable)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    } else {
        uint64_t heading,paragraph,list,item;
        CHECK(xuiDocumentBeginTransaction(document,NULL,&t)==XUI_OK);
        heading=add(t,1,XUI_DOC_HEADING,NULL,0);*editable=add(t,heading,XUI_DOC_TEXT,"Native Rich text",0);
        paragraph=add(t,1,XUI_DOC_PARAGRAPH,NULL,0);
        add(t,paragraph,XUI_DOC_TEXT,"Shared ",0);add(t,paragraph,XUI_DOC_TEXT,"bold",XUI_DOC_BOLD);
        add(t,paragraph,XUI_DOC_TEXT," and ",0);add(t,paragraph,XUI_DOC_TEXT,"italic",XUI_DOC_ITALIC);
        add(t,paragraph,XUI_DOC_TEXT," text using the same Document kernel.",0);
        list=add(t,1,XUI_DOC_LIST,NULL,0);
        for(unsigned i=0;i<2;i++) {item=add(t,list,XUI_DOC_LIST_ITEM,NULL,0);paragraph=add(t,item,XUI_DOC_PARAGRAPH,NULL,0);add(t,paragraph,XUI_DOC_TEXT,i?"Second item":"First item",0);}
        CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    }
    return document;
}
static void document_flow(unsigned profile)
{
    uint64_t editable; char path[1024];xui_document document=create(profile,&editable),reload;
    xui_document_renderer r=renderer(document,XUI_DOC_VISUAL);
    xui_document_transaction t;xui_doc_desc_t desc={0};
    draw(r,first,profile==XUI_DOCUMENT_RICH?"native-rich.png":"native-markdown.png");
    xuiDocumentRendererRelease(r);
    CHECK(xuiDocumentBeginTransaction(document,NULL,&t)==XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t,editable,0,6,"Edited",6)==XUI_OK);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    r=renderer(document,XUI_DOC_VISUAL);draw(r,changed,NULL);xuiDocumentRendererRelease(r);
    CHECK(memcmp(first,changed,sizeof(first))!=0);
    CHECK(xuiDocumentUndo(document,NULL)==XUI_OK);
    r=renderer(document,XUI_DOC_VISUAL);draw(r,pixels,NULL);xuiDocumentRendererRelease(r);
    CHECK(memcmp(first,pixels,sizeof(first))==0);
    CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);
    r=renderer(document,XUI_DOC_VISUAL);draw(r,pixels,NULL);xuiDocumentRendererRelease(r);
    CHECK(memcmp(changed,pixels,sizeof(first))==0);
    file_path(path,sizeof(path),profile==XUI_DOCUMENT_RICH?"native-rich.xuidoc":"native-markdown.md");
    CHECK(xuiDocumentSaveFile(document,path,profile==XUI_DOCUMENT_RICH?XUI_DOC_FILE_NATIVE:XUI_DOC_FILE_MARKDOWN)==XUI_OK);
    desc.iSize=sizeof(desc);desc.iProfile=profile;desc.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentOpenFile(path,profile==XUI_DOCUMENT_RICH?XUI_DOC_FILE_NATIVE:XUI_DOC_FILE_MARKDOWN,&desc,0,&reload)==XUI_OK);
    r=renderer(reload,XUI_DOC_VISUAL);draw(r,pixels,NULL);xuiDocumentRendererRelease(r);
    CHECK(memcmp(changed,pixels,sizeof(first))==0);
    if(profile==XUI_DOCUMENT_MARKDOWN) {
        r=renderer(document,XUI_DOC_SOURCE_TEXT);draw(r,pixels,"native-source.png");xuiDocumentRendererRelease(r);
        r=renderer(document,XUI_DOC_LIVE_MARKDOWN);draw(r,pixels,"native-live.png");xuiDocumentRendererRelease(r);
    }
    xuiDocumentRelease(reload);xuiDocumentRelease(document);
}
static int frame(void* unused)
{
    xui_surface_desc_t surface={0}; (void)unused;
    CHECK(glGetString(GL_VERSION) && glGetString(GL_RENDERER));
    printf("Actual XGE GL context: vendor=%s renderer=%s version=%s\n",
        glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
    proxy=xuiProxyXge();CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy,&fonts.normal,font_paths[0],18,0)==XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy,&fonts.bold,font_paths[1],18,0)==XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy,&fonts.italic,font_paths[2],18,0)==XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy,&fonts.monospace,font_paths[3],17,0)==XUI_OK);
    fonts.boldItalic=fonts.bold;CHECK(xuiSetDefaultFont(context,fonts.normal)==XUI_OK);
    surface.iWidth=WIDTH;surface.iHeight=HEIGHT;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&target,&surface)==XUI_OK);
    document_flow(XUI_DOCUMENT_RICH);document_flow(XUI_DOCUMENT_MARKDOWN);
    proxy.surfaceDestroy(&proxy,target);xuiDestroy(context);
    proxy.fontDestroy(&proxy,fonts.normal);proxy.fontDestroy(&proxy,fonts.bold);
    proxy.fontDestroy(&proxy,fonts.italic);proxy.fontDestroy(&proxy,fonts.monospace);
    xgeQuit();return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t desc={0};CHECK(argc==6);destination=argv[1];
    for(unsigned i=0;i<4;i++)font_paths[i]=argv[i+2];
    CHECK(xgeGetBuildFeatures()==XGE_FEATURES);
    printf("Compiled/actual build features match: %u\n",xgeGetBuildFeatures());
    desc.iWidth=WIDTH;desc.iHeight=HEIGHT;desc.sTitle="Document native acceptance";
    desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();
    CHECK(frames==12 && geometry_checks==12 && images==4);
    printf("Native Document: %u real RGBA frames/%u geometry checks/%u PNGs; Rich and Markdown edits, exact Undo/Redo/reload pixels and three modes passed\n",frames,geometry_checks,images);
    return 0;
}
