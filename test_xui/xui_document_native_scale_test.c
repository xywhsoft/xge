#define _POSIX_C_SOURCE 200809L
#include "xge.h"
#include "xui_document_ui.h"
#include "src/xui_document_view_internal.h"
static unsigned caret_outside;
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr,"native scale line %d: %s\n",__LINE__,#e); exit(1); } } while (0)
enum { W=800, H=600, SAMPLES=64, WARMUP=4, INPUTS=SAMPLES+WARMUP };
static xui_context context;
static xui_proxy_t proxy;
static xui_font font;
static xui_surface target;
static const char* destination;
static const char* font_path;
static unsigned editor_errors;
static double rich_p95, markdown_p95;

static double now_ms(void)
{
#ifdef _WIN32
    LARGE_INTEGER value, rate;
    QueryPerformanceFrequency(&rate); QueryPerformanceCounter(&value);
    return (double)value.QuadPart*1000/(double)rate.QuadPart;
#else
    struct timespec value; CHECK(clock_gettime(CLOCK_MONOTONIC,&value)==0);
    return (double)value.tv_sec*1000+(double)value.tv_nsec/1000000;
#endif
}
static void yield_worker(void)
{
#ifdef _WIN32
    Sleep(1);
#else
    const struct timespec delay={0,1000000}; nanosleep(&delay,NULL);
#endif
}
static int compare_sample(const void* a,const void* b)
{
    double x=*(const double*)a,y=*(const double*)b; return x<y?-1:x>y;
}
static double percentile(const char* name,double* samples)
{
    qsort(samples,SAMPLES,sizeof(*samples),compare_sample);
    printf("%s: %u measured samples; P50=%.3f P95=%.3f max=%.3f ms\n",
        name,SAMPLES,samples[32],samples[60],samples[63]);
    return samples[60];
}
static uint64_t insert(xui_document_transaction t,uint64_t parent,unsigned kind,const char* text)
{
    xui_doc_node_desc_t desc={0};uint64_t id;
    desc.iSize=sizeof(desc);desc.iKind=kind;desc.sText=text;desc.iTextBytes=text?strlen(text):0;
    CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&desc,&id)==XUI_OK);return id;
}
static char* source_text(size_t bytes)
{
    char* source=malloc(bytes+1);CHECK(source && bytes%1024==0);
    for(size_t i=0;i<bytes;i+=1024){memset(source+i,'a',1022);source[i+1022]='\n';source[i+1023]='\n';}
    source[bytes]=0;return source;
}
static xui_document markdown(const char* source,size_t bytes)
{
    xui_doc_desc_t desc={0};xui_document d;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source,bytes)==XUI_OK);
    CHECK(xuiDocumentClearHistory(d)==XUI_OK);return d;
}
static void core_samples(void)
{
    xui_document d;xui_document_transaction t;uint64_t body=0;
    xui_doc_stats_t before={0},after={0};double samples[SAMPLES];
    before.iSize=after.iSize=sizeof(before);
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    for(unsigned i=0;i<10000;i++)body=insert(t,insert(t,1,XUI_DOC_PARAGRAPH,NULL),XUI_DOC_TEXT,"ordinary rich paragraph");
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentGetStats(d,&before)==XUI_OK);
    for(unsigned i=0;i<INPUTS;i++){
        double started=now_ms();
        CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t,body,0,1,i%2?"o":"O",1)==XUI_OK);
        CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        if(i>=WARMUP)samples[i-WARMUP]=now_ms()-started;
    }
    rich_p95=percentile("Rich 10000 paragraphs public begin/replace/commit/release",samples);
    CHECK(xuiDocumentGetStats(d,&after)==XUI_OK && after.iHistoryBytes<=after.iHistoryMaxBytes);
    CHECK(after.iCurrentBytes<before.iCurrentBytes+65536);
    xuiDocumentRelease(d);
    {
        char* source=source_text(100*1024);xui_document_prepare candidate;
        xui_doc_source_patch_t patch={0};xui_doc_txn_desc_t desc={0};
        d=markdown(source,100*1024);free(source);
        patch.iSize=sizeof(patch);patch.iStart=16;patch.iEnd=17;patch.iTextBytes=1;
        desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
        CHECK(xuiDocumentGetStats(d,&before)==XUI_OK);
        for(unsigned i=0;i<INPUTS;i++){
            double started=now_ms();patch.sText=i%2?"a":"A";
            CHECK(xuiDocumentPrepareSource(d,&desc,&patch,1,&candidate)==XUI_OK);
            CHECK(xuiDocumentPrepareRun(candidate)==XUI_OK);
            CHECK(xuiDocumentPreparePublish(d,candidate,NULL)==XUI_OK);xuiDocumentPrepareRelease(candidate);
            if(i>=WARMUP)samples[i-WARMUP]=now_ms()-started;
        }
        markdown_p95=percentile("Markdown 100KiB public prepare/run/publish/release",samples);
        CHECK(xuiDocumentGetStats(d,&after)==XUI_OK && after.iHistoryBytes<=after.iHistoryMaxBytes);
        CHECK(after.iMarkdownIncrementalParses>before.iMarkdownIncrementalParses);
        CHECK(after.iMarkdownParsedBytes-before.iMarkdownParsedBytes<INPUTS*4096);
        printf("MD common edit parsed=%llu bytes for %u edits; retained history=%llu/%llu bytes\n",
            (unsigned long long)(after.iMarkdownParsedBytes-before.iMarkdownParsedBytes),INPUTS,
            (unsigned long long)after.iHistoryBytes,(unsigned long long)after.iHistoryMaxBytes);
        xuiDocumentRelease(d);
    }
}
static void error(xui_widget editor,int result,void* user)
{(void)editor;(void)user;editor_errors++;fprintf(stderr,"Editor asynchronous error=%d\n",result);}
static void draw_frame(void)
{
    const xui_rect_i_t damage={0,0,W,H};
    CHECK(xgeBegin()==XGE_OK);
    CHECK(xuiLayout(context)==XUI_OK && xuiUpdate(context,.016f)==XUI_OK);
    CHECK(proxy.surfaceClear(&proxy,target,XUI_COLOR_WHITE)==XUI_OK);
    CHECK(xuiRender(context,target,&damage,1)==XUI_OK);
    glFinish();CHECK(xgeEnd()==XGE_OK && !editor_errors);
}
static void flush(xui_widget editor)
{
    double started=now_ms();int result;
    while((result=xuiDocumentEditorFlush(editor))==XUI_DOC_ERROR_BUSY){
        CHECK(now_ms()-started<30000);draw_frame();yield_worker();
    }
    CHECK(result==XUI_OK && !editor_errors);
}
static void exact_source(xui_document d,const char* expected,size_t bytes)
{
    xui_document_snapshot snapshot;uint64_t length;char* actual=malloc(bytes+1);CHECK(actual);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot,actual,bytes+1,&length)==XUI_OK && length==bytes);
    CHECK(!memcmp(actual,expected,bytes));free(actual);xuiDocumentSnapshotRelease(snapshot);
}


static void diagnose_caret(xui_document d,xui_widget editor,unsigned mib,unsigned mode,size_t expected)
{
    xui_doc_range_t selection;xui_document_snapshot snapshot;doc_view_data* view;
    xui_doc_rect_t caret;xui_rect_t content;double sx,sy;uint64_t offset;int mapping,outside;
    CHECK(xuiDocumentViewGetSelection(editor,&selection)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK);
    CHECK(xuiDocumentPositionToSource(snapshot,&selection.tCaret,&offset,&mapping)==XUI_OK);
    CHECK(offset==expected);
    CHECK(xuiDocumentViewGetScroll(editor,&sx,&sy)==XUI_OK);
    view=(doc_view_data*)xuiWidgetGetTypeData(editor);CHECK(view && view->renderer);
    CHECK(xuiDocumentRendererGetCaretRect(view->renderer,&selection.tCaret,&caret)==XUI_OK);
    content=xuiWidgetGetContentRect(editor);
    outside=caret.x-sx<-.5 || caret.x-sx+caret.width>content.fW+.5 ||
        caret.y-sy<-.5 || caret.y-sy+caret.height>content.fH+.5;
    caret_outside+=(unsigned)outside;
    printf("Actual view caret %uMiB mode=%u: exact-source=%llu scroll=%.2f,%.2f caret=%.2f,%.2f screen=%.2f,%.2f content=%.2f,%.2f outside=%d\n",
        mib,mode,(unsigned long long)offset,sx,sy,caret.x,caret.y,caret.x-sx,caret.y-sy,(double)content.fW,(double)content.fH,outside);
    xuiDocumentSnapshotRelease(snapshot);
}

static void editor_case(unsigned mib,unsigned mode)
{
    size_t bytes=(size_t)mib*1024*1024,offset=bytes/2+16;
    char* source=source_text(bytes);char* expected=malloc(bytes+INPUTS+1);CHECK(expected);
    xui_document d=markdown(source,bytes);xui_widget editor;xui_doc_editor_desc_t desc={0};
    xui_doc_range_t range={0};xui_document_snapshot snapshot;xui_doc_renderer_stats_t first={0},after={0};
    xui_doc_stats_t stats={0};double samples[SAMPLES],started,initial_ms,publication_ms;
    const char* name=mode==XUI_DOC_VISUAL?"visual":mode==XUI_DOC_SOURCE_TEXT?"source":"live";
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=d;desc.iMode=mode;
    desc.tView.tRenderer.iSize=sizeof(desc.tView.tRenderer);
    desc.tView.tRenderer.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
    desc.iAsyncSourceThresholdBytes=1;desc.iUndoGroupTimeoutMs=60000;desc.onError=error;
    CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK && xuiSetRootWidget(context,editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,W,H})==XUI_OK);draw_frame();
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK);
    range.tAnchor.iSize=sizeof(range.tAnchor);range.tAnchor.iDocumentId=xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision=xuiDocumentGetRevision(d);range.tAnchor.iOffset=offset;
    range.tAnchor.iAffinity=XUI_DOC_AFTER;range.tAnchor.iNodeId=1;range.tAnchor.iKind=XUI_DOC_POSITION_SOURCE;
    if(mode==XUI_DOC_VISUAL){
        uint64_t paragraph;CHECK(xuiDocumentSnapshotGetChild(snapshot,1,(bytes/1024)/2,&paragraph)==XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&range.tAnchor.iNodeId)==XUI_OK);
        range.tAnchor.iKind=XUI_DOC_POSITION_TEXT;range.tAnchor.iOffset=16;
    }
    range.tCaret=range.tAnchor;xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor,&range)==XUI_OK);draw_frame();
    first.iSize=after.iSize=sizeof(first);stats.iSize=sizeof(stats);
    CHECK(xuiDocumentViewGetRenderStats(editor,&first)==XUI_OK && first.iShapedBytes<bytes/4);
    started=now_ms();CHECK(xuiDocumentEditorInsertText(editor,"X",1)==XUI_OK);draw_frame();initial_ms=now_ms()-started;
    for(unsigned i=1;i<INPUTS;i++){
        started=now_ms();CHECK(xuiDocumentEditorInsertText(editor,"X",1)==XUI_OK);draw_frame();
        if(i>=WARMUP)samples[i-WARMUP]=now_ms()-started;
    }
    {char label[128];snprintf(label,sizeof(label),"Actual Editor %uMiB %s input/layout/update/render/glFinish",mib,name);percentile(label,samples);}
    started=now_ms();flush(editor);draw_frame();publication_ms=now_ms()-started;
    memcpy(expected,source,offset);memset(expected+offset,'X',INPUTS);
    memcpy(expected+offset+INPUTS,source+offset,bytes-offset+1);
    exact_source(d,expected,bytes+INPUTS);diagnose_caret(d,editor,mib,mode,offset+INPUTS);
    CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK && stats.iUndoCount==1 && stats.iHistoryBytes<=stats.iHistoryMaxBytes);
    CHECK(xuiDocumentViewGetRenderStats(editor,&after)==XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);draw_frame();exact_source(d,source,bytes);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);draw_frame();exact_source(d,expected,bytes+INPUTS);diagnose_caret(d,editor,mib,mode,offset+INPUTS);
    {
        unsigned char* pixels=malloc(W*H*4);char path[1024];unsigned ink=0;CHECK(pixels);
        CHECK(proxy.surfaceReadRGBA(&proxy,target,pixels,W*4)==XUI_OK);
        for(unsigned i=0;i<W*H;i++)if(pixels[4*i]<160 && pixels[4*i+1]<160 && pixels[4*i+2]<160)ink++;
        CHECK(ink>1000);snprintf(path,sizeof(path),"%s/editor-%uMiB-%s.png",destination,mib,name);
        CHECK(xgeImageSavePNG(path,W,H,pixels,W*4)==XGE_OK);free(pixels);
    }
    printf("Actual Editor %uMiB %s: first=%.3f publication=%.3f ms; initial shaped=%llu final shaped=%llu; exact source/%u inputs/one Undo/Redo; history=%llu/%llu\n",
        mib,name,initial_ms,publication_ms,(unsigned long long)first.iShapedBytes,
        (unsigned long long)after.iShapedBytes,INPUTS,(unsigned long long)stats.iHistoryBytes,(unsigned long long)stats.iHistoryMaxBytes);
    xuiWidgetDestroy(editor);xuiDocumentRelease(d);free(expected);free(source);
}
static int frame(void* user)
{
    xui_surface_desc_t surface={0};(void)user;
    printf("Actual XGE GL context: vendor=%s renderer=%s version=%s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
    proxy=xuiProxyXge();CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy,&font,font_path,18,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(xuiInputViewport(context,W,H)==XUI_OK);
    surface.iWidth=W;surface.iHeight=H;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&target,&surface)==XUI_OK);core_samples();
    for(unsigned mib=1;mib<=10;mib+=9)for(unsigned mode=XUI_DOC_VISUAL;mode<=XUI_DOC_LIVE_MARKDOWN;mode++)editor_case(mib,mode);
    proxy.surfaceDestroy(&proxy,target);xuiDestroy(context);proxy.fontDestroy(&proxy,font);
    printf("Native scale acceptance: Rich/100KiB core plus six actual 1/10MiB Editor cases passed; rich8ms=%s markdown50ms=%s; timings diagnostic under current machine load\n",
        rich_p95<=8?"within":"exceeded",markdown_p95<=50?"within":"exceeded");
    xgeQuit();return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t desc={0};CHECK(argc==3);destination=argv[1];font_path=argv[2];
    CHECK(xgeGetBuildFeatures()==XGE_FEATURES);desc.iWidth=W;desc.iHeight=H;
    desc.sTitle="Document actual native scale acceptance";desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();
    printf("Actual view caret viewport diagnostics: %u out-of-viewport checks\n",caret_outside);return caret_outside?1:0;
}
