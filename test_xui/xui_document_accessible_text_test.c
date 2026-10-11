#include "../xui_document_ui.h"
#include "../src/xui_accessibility_text_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "Text range line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
static const char unicode[] = "\xe4\xb8\xad\xf0\x9f\x98\x80";
static const char markdown[] = "ab \xe4\xb8\xad\xf0\x9f\x98\x80 e\xcc\x81\n\nsecond\n";
static const char rich[] = "ab \xe4\xb8\xad\xf0\x9f\x98\x80 e\xcc\x81\nsecond";
static void text(xui_internal_accessible_text_range* range, const char* expected)
{
    char buffer[512], small[2] = {'X', 'Y'}; uint64_t bytes;
    CHECK(range->ops->getText(range, NULL, 0, &bytes) == XUI_OK && bytes == strlen(expected));
    if (bytes >= sizeof(small)) {
        CHECK(range->ops->getText(range, small, sizeof(small), &bytes) == XUI_ERROR_BUFFER_TOO_SMALL);
        CHECK(small[0] == 'X' && small[1] == 'Y');
    }
    CHECK(range->ops->getText(range, buffer, sizeof(buffer), &bytes) == XUI_OK);
    if (strcmp(buffer, expected)) fprintf(stderr, "Expected [%s], got [%s]\n", expected, buffer);
    CHECK(!strcmp(buffer, expected));
}
static xui_internal_accessible_text_range* create(xui_widget widget, int kind, uint64_t child, double x, double y)
{
    xui_internal_accessible_text_request request = {kind, child, x, y};
    xui_accessible_node_t root = {0}; xui_internal_accessible_text_range* range = NULL;
    root.iSize = sizeof(root); CHECK(xuiWidgetGetAccessibleNode(widget, 0, &root) == XUI_OK);
    CHECK(xuiInternalAccessibilityCreateTextRange(widget, root.iId, &request, &range) == XUI_OK && range);
    return range;
}
#ifdef _WIN32
static DWORD WINAPI release_worker(void* data)
{ xui_internal_accessible_text_range* range = data; range->ops->release(range); return 0; }
static void release_on_worker(xui_internal_accessible_text_range* range)
{
    HANDLE thread = CreateThread(NULL, 0, release_worker, range, 0, NULL);
    CHECK(thread && WaitForSingleObject(thread, 10000) == WAIT_OBJECT_0); CloseHandle(thread);
}
#else
static void* release_worker(void* data)
{ xui_internal_accessible_text_range* range = data; range->ops->release(range); return NULL; }
static void release_on_worker(xui_internal_accessible_text_range* range)
{ pthread_t thread; CHECK(!pthread_create(&thread, NULL, release_worker, range)); CHECK(!pthread_join(thread, NULL)); }
#endif
static void scenario(int profile, uint32_t mode)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document, replacement; xui_widget editor, view;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0};
    xui_internal_accessible_text_range *whole, *range, *clone, *caret, *point, *child, *reader;
    xui_rect_t rectangles[32], world, content; uint64_t first, last, length, total; int i, child_seen = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(context, 800, 600) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = (uint32_t)profile;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    if (profile == XUI_DOCUMENT_MARKDOWN) CHECK(xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.iMode = mode; ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){20, 30, 200, 50}) == XUI_OK);
    if (profile == XUI_DOCUMENT_RICH) CHECK(xuiDocumentEditorInsertText(editor, rich, strlen(rich)) == XUI_OK);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);
    whole = create(editor, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    text(whole, mode == XUI_DOC_VISUAL ? "ab \xe4\xb8\xad\xf0\x9f\x98\x80 e\xcc\x81\nsecond\n" : markdown);
    CHECK(whole->ops->clone(whole, &range) == XUI_OK);
    CHECK(range->ops->setOffsets(range, 3, 10) == XUI_OK); text(range, unicode);
    CHECK(range->ops->setOffsets(range, 4, 10) == XUI_DOC_ERROR_UTF8); text(range, unicode);
    CHECK(range->ops->setOffsets(range, 10, 3) == XUI_ERROR_INVALID_ARGUMENT); text(range, unicode);
    CHECK(range->ops->getOffsets(range, &first, &last, &length) == XUI_OK && first == 3 && last == 10);
    CHECK(range->ops->clone(range, &clone) == XUI_OK); text(clone, unicode);
    CHECK(range->ops->select(range) == XUI_OK);
    caret = create(editor, XUI_INTERNAL_TEXT_CARET, 0, 0, 0); text(caret, "");
    CHECK(caret->ops->getOffsets(caret, &first, &last, &length) == XUI_OK && first == 10 && last == 10);
    caret->ops->release(caret);
    CHECK(range->ops->getRects(range, rectangles, 32, &total) == XUI_OK && total && total <= 32);
    world = xuiWidgetGetWorldRect(editor); content = xuiWidgetGetContentRect(editor);
    for (i = 0; i < (int)total; i++) {
        CHECK(rectangles[i].fX >= world.fX + content.fX - .01 && rectangles[i].fY >= world.fY + content.fY - .01);
        CHECK(rectangles[i].fX + rectangles[i].fW <= world.fX + content.fX + content.fW + .01);
        CHECK(rectangles[i].fY + rectangles[i].fH <= world.fY + content.fY + content.fH + .01);
    }
    point = create(editor, XUI_INTERNAL_TEXT_POINT, 0, rectangles[0].fX, rectangles[0].fY + rectangles[0].fH / 2);
    CHECK(point->ops->getOffsets(point, &first, &last, &length) == XUI_OK && first == last);
    CHECK(first >= 3 && first <= 10); point->ops->release(point);
    if (mode == XUI_DOC_VISUAL) {
        for (i = 1; i < xuiWidgetGetAccessibleNodeCount(editor); i++) {
            xui_accessible_node_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
            if (node.iRole == XUI_ACCESSIBLE_ROLE_TEXT && node.sValue && strstr(node.sValue, unicode)) {
                child = create(editor, XUI_INTERNAL_TEXT_CHILD, node.iId, 0, 0);
                text(child, node.sValue); child->ops->release(child); child_seen = 1; break;
            }
        }
        CHECK(child_seen);
    }
    CHECK(whole->ops->setOffsets(whole, 0, 0) == XUI_OK && whole->ops->select(whole) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "PRE ", 4) == XUI_OK);
    CHECK(xuiDocumentEditorFlush(editor) == XUI_OK);
    text(range, unicode); text(clone, unicode);
    CHECK(range->ops->getOffsets(range, &first, &last, &length) == XUI_OK && first == 7 && last == 14);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); text(range, unicode); text(clone, unicode);
    CHECK(range->ops->getOffsets(range, &first, &last, &length) == XUI_OK && first == 3 && last == 10);
    CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); text(range, unicode); text(clone, unicode);
    if (profile == XUI_DOCUMENT_MARKDOWN) {
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); text(range, unicode);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK); text(range, unicode);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK); text(range, unicode);
        CHECK(xuiDocumentViewSetMode(editor, mode) == XUI_OK); text(range, unicode);
    }
    CHECK(range->ops->scrollIntoView(range, 1) == XUI_OK);
    CHECK(range->ops->scrollIntoView(range, 0) == XUI_OK);
    CHECK(xuiDocumentViewSetDocument(editor, document) == XUI_OK); text(range, unicode);
    CHECK(range->ops->clone(range, &caret) == XUI_OK); release_on_worker(caret);
    /* A second view of the same Document owns an independent text provider. */
    vd.iSize = sizeof(vd); vd.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    reader = create(view, XUI_INTERNAL_TEXT_DOCUMENT, 0, 0, 0);
    CHECK(reader->ops->setOffsets(reader, 7, 14) == XUI_OK); text(reader, unicode);
    CHECK(xuiDocumentCreate(&desc, &replacement) == XUI_OK);
    CHECK(xuiDocumentViewSetDocument(editor, replacement) == XUI_OK);
    CHECK(range->ops->getOffsets(range, &first, &last, &length) == XUI_DOC_ERROR_STALE);
    CHECK(clone->ops->select(clone) == XUI_DOC_ERROR_STALE);
    text(reader, unicode);
    CHECK(xuiDocumentViewSetDocument(editor, document) == XUI_OK);
    CHECK(range->ops->getText(range, NULL, 0, &length) == XUI_DOC_ERROR_STALE);
    xuiWidgetDestroy(view);
    CHECK(reader->ops->getText(reader, NULL, 0, &length) == XUI_DOC_ERROR_STALE);
    reader->ops->release(reader); range->ops->release(range); whole->ops->release(whole);
    xuiDestroy(context);
    CHECK(clone->ops->getText(clone, NULL, 0, &length) == XUI_DOC_ERROR_STALE);
    release_on_worker(clone); xuiDocumentRelease(document); xuiDocumentRelease(replacement);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Tracked Document text ranges passed: profile=%d mode=%u\n", profile, mode);
}
static void offsets(xui_internal_accessible_text_range* range, uint64_t expected_first, uint64_t expected_last)
{
    uint64_t a,b,n;
    CHECK(range->ops->getOffsets(range,&a,&b,&n)==XUI_OK);
    if (a != expected_first || b != expected_last) fprintf(stderr,
        "Offsets expected %llu..%llu; got %llu..%llu (length %llu)\n",
        (unsigned long long)expected_first, (unsigned long long)expected_last,
        (unsigned long long)a, (unsigned long long)b, (unsigned long long)n);
    CHECK(a==expected_first && b==expected_last && b<=n);
}
static void navigation(int profile, uint32_t mode)
{
    static const char source[]="hello,  world! e\xcc\x81 \xf0\x9f\x98\x80 ababa\n\nSECOND \xe2\x84\xaa \xce\xa3 \xcf\x83 \xcf\x82 a+b\n";
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_doc_desc_t desc={0}; xui_doc_editor_desc_t ed={0}; xui_doc_view_desc_t vd={0};
    xui_document document; xui_widget editor,view; xui_doc_range_t selected;
    xui_internal_accessible_text_range *whole,*range,*peer,*found,*reverse,*reader;
    uint64_t first,last,length; int equal,order,actual,unit;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context)==XUI_OK);
    CHECK(xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(context,font)==XUI_OK);
    desc.iSize=sizeof(desc);desc.iProfile=(uint32_t)profile;
    CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
    if(profile==XUI_DOCUMENT_MARKDOWN)CHECK(xuiDocumentLoadMarkdown(document,source,strlen(source))==XUI_OK);
    ed.iSize=sizeof(ed);ed.iMode=mode;ed.tView.iSize=sizeof(ed.tView);ed.tView.pDocument=document;
    CHECK(xuiDocumentEditorCreate(context,&ed,&editor)==XUI_OK);
    CHECK(xuiSetRootWidget(context,editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,160,60})==XUI_OK);
    if(profile==XUI_DOCUMENT_RICH)CHECK(xuiDocumentEditorInsertText(editor,source,strlen(source)-1)==XUI_OK);
    CHECK(xuiDocumentEditorFlush(editor)==XUI_OK);
    whole=create(editor,XUI_INTERNAL_TEXT_DOCUMENT,0,0,0);
    CHECK(whole->ops->clone(whole,&range)==XUI_OK && whole->ops->clone(whole,&peer)==XUI_OK);
    CHECK(whole->ops->getOffsets(whole,&first,&last,&length)==XUI_OK && first==0 && last==length);
    CHECK(range->ops->setOffsets(range,16,16)==XUI_OK);
    CHECK(range->ops->expand(range,XUI_INTERNAL_TEXT_GRAPHEME)==XUI_OK); offsets(range,15,18);text(range,"e\xcc\x81");
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,1,&actual)==XUI_OK && actual==1);offsets(range,18,19);text(range," ");
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,1,&actual)==XUI_OK && actual==1);offsets(range,19,23);text(range,"\xf0\x9f\x98\x80");
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,-2,&actual)==XUI_OK && actual==-2);offsets(range,15,18);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,0,&actual)==XUI_OK && actual==0);offsets(range,15,18);
    CHECK(range->ops->setOffsets(range,1,4)==XUI_OK && range->ops->expand(range,XUI_INTERNAL_TEXT_WORD)==XUI_OK);
    offsets(range,0,8);text(range,"hello,  ");
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_WORD,1,&actual)==XUI_OK && actual==1);offsets(range,8,15);text(range,"world! ");
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_WORD,-1,&actual)==XUI_OK && actual==-1);offsets(range,0,8);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_WORD,-7,&actual)==XUI_OK && actual==0);offsets(range,0,8);
    CHECK(range->ops->setOffsets(range,11,12)==XUI_OK);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_WORD,1,&actual)==XUI_OK && actual==1);offsets(range,15,24);
    CHECK(range->ops->setOffsets(range,16,16)==XUI_OK);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,-1,&actual)==XUI_OK && actual==-1);offsets(range,15,15);
    CHECK(range->ops->moveEndpoint(range,XUI_INTERNAL_TEXT_END,XUI_INTERNAL_TEXT_GRAPHEME,1,&actual)==XUI_OK && actual==1);offsets(range,15,18);
    CHECK(range->ops->moveEndpoint(range,XUI_INTERNAL_TEXT_START,XUI_INTERNAL_TEXT_GRAPHEME,2,&actual)==XUI_OK && actual==2);offsets(range,19,19);
    CHECK(range->ops->moveEndpoint(range,XUI_INTERNAL_TEXT_END,XUI_INTERNAL_TEXT_GRAPHEME,-2,&actual)==XUI_OK && actual==-2);offsets(range,15,15);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_GRAPHEME,INT_MIN,&actual)==XUI_OK && actual==-15);offsets(range,0,0);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_ALL,INT_MAX,&actual)==XUI_OK && actual==1);offsets(range,length,length);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_ALL,INT_MIN,&actual)==XUI_OK && actual==-1);offsets(range,0,0);
    CHECK(range->ops->expand(range,XUI_INTERNAL_TEXT_ALL)==XUI_OK);offsets(range,0,length);
    CHECK(range->ops->move(range,XUI_INTERNAL_TEXT_ALL,1,&actual)==XUI_OK && actual==0);offsets(range,0,length);
    CHECK(range->ops->setOffsets(range,length,length)==XUI_OK);
    CHECK(range->ops->expand(range,XUI_INTERNAL_TEXT_GRAPHEME)==XUI_OK);offsets(range,length-1,length);
    for(unit=XUI_INTERNAL_TEXT_FORMAT;unit<XUI_INTERNAL_TEXT_ALL;unit++) {
        if(unit==XUI_INTERNAL_TEXT_FORMAT || unit==XUI_INTERNAL_TEXT_WORD || unit==XUI_INTERNAL_TEXT_LINE || unit==XUI_INTERNAL_TEXT_PARAGRAPH)continue;
        CHECK(range->ops->expand(range,unit)==XUI_ERROR_UNSUPPORTED);offsets(range,length-1,length);
        CHECK(range->ops->move(range,unit,2,&actual)==XUI_ERROR_UNSUPPORTED && actual==0);offsets(range,length-1,length);
    }
    CHECK(range->ops->expand(range,99)==XUI_ERROR_INVALID_ARGUMENT);offsets(range,length-1,length);
    CHECK(range->ops->moveEndpoint(range,99,XUI_INTERNAL_TEXT_WORD,2,&actual)==XUI_ERROR_INVALID_ARGUMENT && actual==0);
    CHECK(range->ops->setOffsets(range,3,9)==XUI_OK && peer->ops->setOffsets(peer,3,9)==XUI_OK);
    CHECK(range->ops->compare(range,peer,&equal)==XUI_OK && equal);
    CHECK(range->ops->compareEndpoints(range,XUI_INTERNAL_TEXT_START,peer,XUI_INTERNAL_TEXT_END,&order)==XUI_OK && order==-1);
    CHECK(range->ops->compareEndpoints(range,XUI_INTERNAL_TEXT_END,peer,XUI_INTERNAL_TEXT_START,&order)==XUI_OK && order==1);
    CHECK(range->ops->select(range)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK);
    {xui_doc_position_t swap=selected.tAnchor;selected.tAnchor=selected.tCaret;selected.tCaret=swap;}
    CHECK(xuiDocumentViewSetSelection(editor,&selected)==XUI_OK);
    reverse=create(editor,XUI_INTERNAL_TEXT_SELECTION,0,0,0);
    CHECK(range->ops->compare(range,reverse,&equal)==XUI_OK && equal);offsets(reverse,3,9);
    CHECK(peer->ops->setOffsets(peer,20,23)==XUI_DOC_ERROR_UTF8);
    CHECK(peer->ops->setOffsets(peer,24,29)==XUI_OK);
    CHECK(range->ops->setEndpoint(range,XUI_INTERNAL_TEXT_START,peer,XUI_INTERNAL_TEXT_END)==XUI_OK);offsets(range,29,29);
    CHECK(range->ops->setEndpoint(range,XUI_INTERNAL_TEXT_END,reverse,XUI_INTERNAL_TEXT_START)==XUI_OK);offsets(range,3,3);
    CHECK(range->ops->compareEndpoints(range,99,peer,0,&order)==XUI_ERROR_INVALID_ARGUMENT && order==0);
    CHECK(whole->ops->findText(whole,"aba",3,0,0,&found)==XUI_OK && found);offsets(found,24,27);text(found,"aba");found->ops->release(found);
    CHECK(whole->ops->findText(whole,"aba",3,1,0,&found)==XUI_OK && found);offsets(found,26,29);text(found,"aba");found->ops->release(found);
    CHECK(peer->ops->setOffsets(peer,25,29)==XUI_OK);
    CHECK(peer->ops->findText(peer,"ababa",5,0,0,&found)==XUI_OK && !found);
    CHECK(whole->ops->findText(whole,"a+b",3,0,0,&found)==XUI_OK && found);text(found,"a+b");found->ops->release(found);
    CHECK(whole->ops->findText(whole,"k",1,0,1,&found)==XUI_OK && found);text(found,"\xe2\x84\xaa");found->ops->release(found);
    CHECK(whole->ops->findText(whole,"\xcf\x83",2,0,1,&found)==XUI_OK && found);text(found,"\xce\xa3");found->ops->release(found);
    CHECK(whole->ops->findText(whole,"\xcf\x83",2,1,1,&found)==XUI_OK && found);text(found,"\xcf\x82");found->ops->release(found);
    CHECK(whole->ops->findText(whole,"k",1,0,0,&found)==XUI_OK && !found);
    CHECK(whole->ops->findText(whole,"",0,0,0,&found)==XUI_ERROR_INVALID_ARGUMENT && !found);
    CHECK(whole->ops->findText(whole,"\x80",1,0,0,&found)==XUI_ERROR_INVALID_ARGUMENT && !found);
    CHECK(whole->ops->findText(whole,"x",1,2,0,&found)==XUI_ERROR_INVALID_ARGUMENT && !found);
    vd.iSize=sizeof(vd);vd.pDocument=document;
    CHECK(xuiDocumentViewCreate(context,&vd,&view)==XUI_OK);
    reader=create(view,XUI_INTERNAL_TEXT_DOCUMENT,0,0,0);
    CHECK(range->ops->compare(range,reader,&equal)==XUI_ERROR_INVALID_ARGUMENT && equal==0);
    CHECK(range->ops->setEndpoint(range,0,reader,1)==XUI_ERROR_INVALID_ARGUMENT);offsets(range,3,3);
    CHECK(whole->ops->findText(whole,"ababa",5,0,0,&found)==XUI_OK && found);
    CHECK(range->ops->setOffsets(range,0,0)==XUI_OK && range->ops->select(range)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor,"PRE ",4)==XUI_OK && xuiDocumentEditorFlush(editor)==XUI_OK);
    offsets(found,28,33);text(found,"ababa");
    CHECK(xuiDocumentUndo(document,NULL)==XUI_OK);offsets(found,24,29);
    CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);offsets(found,28,33);
    xuiWidgetDestroy(view);
    CHECK(range->ops->compare(range,reader,&equal)==XUI_DOC_ERROR_STALE && !equal);
    reader->ops->release(reader);whole->ops->release(whole);range->ops->release(range);
    peer->ops->release(peer);reverse->ops->release(reverse);
    xuiDestroy(context);
    CHECK(found->ops->move(found,XUI_INTERNAL_TEXT_WORD,1,&actual)==XUI_DOC_ERROR_STALE && actual==0);
    found->ops->release(found);xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    printf("Document text navigation passed: profile=%d mode=%u\n",profile,mode);
}
#include "xui_document_visual_line_cases.h"
#include "xui_document_text_line_cases.h"
#include "xui_document_text_clip_cases.h"
#include "xui_document_text_paragraph_cases.h"
#include "xui_document_text_prepared_cases.h"
#include "xui_document_text_format_cases.h"
int main(void)
{
    text_format_cases();
    text_format_live_syntax();
    text_prepared_cases();
    text_paragraph_cases();
    text_clip_cases();
    visual_line_cases();
    text_line_cases();
    scenario(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    scenario(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    scenario(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    scenario(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
    navigation(XUI_DOCUMENT_RICH, XUI_DOC_VISUAL);
    navigation(XUI_DOCUMENT_MARKDOWN, XUI_DOC_VISUAL);
    navigation(XUI_DOCUMENT_MARKDOWN, XUI_DOC_SOURCE_TEXT);
    navigation(XUI_DOCUMENT_MARKDOWN, XUI_DOC_LIVE_MARKDOWN);
    puts("Document accessible text range tests passed"); return 0;
}
