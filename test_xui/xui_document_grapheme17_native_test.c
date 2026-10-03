#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned sample, form;
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (sample=%u form=%u)\n",__FILE__,__LINE__,#e,sample,form); exit(1); } } while (0)
typedef struct sample_t { const char* text; unsigned first; } sample_t;
static const sample_t samples[] = {
    {"a\xe2\x80\x8d" "b",4},
    {"\xf0\x9f\x91\xa9\xe2\x80\x8d" "a",7},
    {"\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\x95X",9},
    {"\xe1\xac\x93\xe1\xad\x84\xe1\xac\x93X",9},
    {"\xf0\x96\xb5\xa3\xf0\x96\xb5\xa7X",8},
    {"\xea\xb0\x80\xf0\x96\xb5\xa3X",7},
    {"\xf0\x91\xbc\x82" "aX",5}
};
static xui_doc_position_t position(xui_document document,uint64_t node,uint64_t offset,unsigned mode)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iDocumentId=xuiDocumentGetIdentity(document);
    p.iRevision=xuiDocumentGetRevision(document);p.iKind=mode==XUI_DOC_VISUAL?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE;
    p.iNodeId=mode==XUI_DOC_VISUAL?node:1;p.iOffset=offset;p.iAffinity=XUI_DOC_AFTER;return p;
}
static void plain(xui_document document,const char* expected)
{
    xui_document_snapshot snapshot;char* text;uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
        xuiDocumentSnapshotCopyPlainText(snapshot,&text,&bytes)==XUI_OK);
    if(strcmp(text,expected))fprintf(stderr,"plain differs: bytes=%llu expected=%zu\n",(unsigned long long)bytes,strlen(expected));
    CHECK(bytes==strlen(expected) && !strcmp(text,expected));xuiDocumentFreeBuffer(text);xuiDocumentSnapshotRelease(snapshot);
}
static unsigned scalar_bytes(unsigned char head)
{return head<0x80?1:head<0xe0?2:head<0xf0?3:4;}

static int frame(void* user)
{
    xui_proxy_t proxy=xuiProxyXge();xui_context context;xui_font font;
    xui_surface target; xui_surface_desc_t surface={0}; xui_rect_i_t damage={0,0,400,240};
    float size=*(float*)user;unsigned cases=0;
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans=NULL;
#endif
    CHECK(proxy.fontLoadFile(&proxy,&font,"test/data/xge_opentype_fixture.ttf",size,0)==XUI_OK &&
        xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    surface.iWidth=400;surface.iHeight=240;surface.iFlags=XUI_SURFACE_USAGE_TARGET;
    CHECK(proxy.surfaceCreate(&proxy,&target,&surface)==XUI_OK);
    for(sample=0;sample<sizeof(samples)/sizeof(*samples);sample++)for(form=0;form<5;form++){
        const char* text=samples[sample].text;unsigned length=(unsigned)strlen(text),at=0,parts=0,mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
        uint64_t ids[16]={0};xui_document document;xui_widget editor;xui_doc_editor_desc_t desc={0};
        xui_doc_range_t range={0},moved;xui_event_t key={0};char expected[64],full[64];
        xui_doc_desc_t profile={0};xui_rect_t caret;xui_doc_position_t hit;xui_document_snapshot snapshot;
        profile.iSize=sizeof(profile);profile.iProfile=form>=2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK);
        if(form>=2){uint64_t paragraph;CHECK(xuiDocumentLoadMarkdown(document,text,length)==XUI_OK &&
            xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&ids[0])==XUI_OK);xuiDocumentSnapshotRelease(snapshot);parts=1;
        }else{
            xui_document_transaction transaction;xui_doc_node_desc_t node={0};uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            while(at<length){unsigned bytes=form?scalar_bytes((unsigned char)text[at]):length;
                node.iKind=XUI_DOC_TEXT;node.sText=text+at;node.iTextBytes=bytes;
                node.tAttributes.iTextColor=parts&1?XUI_COLOR_RGBA(200,30,40,255):XUI_COLOR_RGBA(30,40,200,255);
                CHECK(parts<16 && xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,&ids[parts])==XUI_OK);
                parts++;at+=bytes;
            }
            CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
        }
        desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=document;
        desc.tView.tRenderer.iSize=sizeof(desc.tView.tRenderer);desc.tView.tRenderer.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
        CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK && xuiSetRootWidget(context,editor)==XUI_OK &&
            xuiWidgetSetRect(editor,(xui_rect_t){0,0,400,240})==XUI_OK && xuiInputViewport(context,400,240)==XUI_OK &&
            xuiDocumentViewSetMode(editor,mode)==XUI_OK && xuiUpdate(context,.016f)==XUI_OK);
        range.tAnchor=range.tCaret=position(document,mode==XUI_DOC_VISUAL?ids[0]:0,0,mode);
        CHECK(xuiDocumentViewSetSelection(editor,&range)==XUI_OK);
        key.iSize=sizeof(key);key.iType=XUI_EVENT_KEY_DOWN;key.pTarget=editor;key.iKey=XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context,&key)==XUI_OK && xuiDocumentViewGetSelection(editor,&moved)==XUI_OK);
        {
            char* selected;uint64_t bytes;range.tCaret=moved.tCaret;
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot,&range,&selected,&bytes)==XUI_OK);
            if(bytes!=samples[sample].first)fprintf(stderr,"first step bytes=%llu expected=%u\n",(unsigned long long)bytes,samples[sample].first);
            CHECK(bytes==samples[sample].first && !memcmp(selected,text,(size_t)bytes));
            xuiDocumentFreeBuffer(selected);xuiDocumentSnapshotRelease(snapshot);
        }
        caret=xuiEditGetCaretRect(editor);
        CHECK(caret.fW>0 && caret.fH>0 && xuiDocumentViewHitTest(editor,caret.fX,caret.fY+caret.fH*.5f,&hit)==XUI_OK);
        {
            xui_doc_range_t clicked=range;char* selected;uint64_t bytes;
            clicked.tCaret=hit;
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot,&clicked,&selected,&bytes)==XUI_OK &&
                bytes==samples[sample].first && !memcmp(selected,text,(size_t)bytes));
            xuiDocumentFreeBuffer(selected);xuiDocumentSnapshotRelease(snapshot);
        }
        CHECK(xuiRender(context,target,&damage,1)==XUI_OK);
        snprintf(expected,sizeof(expected),"%s\n",text+samples[sample].first);snprintf(full,sizeof(full),"%s\n",text);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BACKSPACE)==XUI_OK);plain(document,expected);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(document,full);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);plain(document,expected);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(document,full);
        range.tAnchor=range.tCaret=position(document,mode==XUI_DOC_VISUAL?ids[0]:0,0,mode);
        CHECK(xuiDocumentViewSetSelection(editor,&range)==XUI_OK && xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_DELETE)==XUI_OK);plain(document,expected);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(document,full);
        CHECK(xuiSetRootWidget(context,NULL)==XUI_OK);xuiWidgetDestroy(editor);
        xuiDocumentRelease(document);cases++;
    }
    proxy.surfaceDestroy(&proxy,target);xuiDestroy(context);proxy.fontDestroy(&proxy,font);
    printf("Native Document Unicode 17: %u cases at size=%g; ordinary ZWJ, Indic, new conjoining properties, mixed scripts, split colors, Rich/Markdown/Source/Live, GPU draw/hit, movement, Delete/Backspace and Undo/Redo passed\n",cases,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t engine={0};float sizes[]={40,37};unsigned i;
    engine.iWidth=400;engine.iHeight=240;engine.sTitle="Document Unicode 17";engine.iFlags=XGE_INIT_OFFSCREEN;engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<2;i++){CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}
    return 0;
}
