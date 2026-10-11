/* Real XGE adapters while the Windows clipboard is held open; no native
 * clipboard writes or payload reads. Successful/empty operations use the
 * deterministic proxy. This does not verify physical input. */
#include "../xui_document_ui.h"
#include "../xge.h"
#include "xui_test_proxy.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(e) do { checks++; if (!(e)) { fprintf(stderr,"clipboard line %d: %s (win32=%lu)\n",__LINE__,#e,(unsigned long)GetLastError()); failed=1; goto done; } } while (0)
typedef struct clipboard_lock { HDESK desktop; HANDLE ready,release,thread; int opened; } clipboard_lock;
static DWORD WINAPI hold_clipboard(void* user)
{
    clipboard_lock* lock = user; HWND owner = NULL;
    if (SetThreadDesktop(lock->desktop)) owner = CreateWindowExW(0,L"STATIC",L"Document clipboard lock",WS_POPUP,0,0,0,0,NULL,NULL,GetModuleHandleW(NULL),NULL);
    lock->opened = owner && OpenClipboard(owner);
    SetEvent(lock->ready);
    if (lock->opened) { WaitForSingleObject(lock->release,10000); CloseClipboard(); }
    if (owner) DestroyWindow(owner);
    return 0;
}
static void unlock_clipboard(clipboard_lock* lock)
{
    if (lock->release) SetEvent(lock->release);
    if (lock->thread) { WaitForSingleObject(lock->thread,12000); CloseHandle(lock->thread); }
    if (lock->ready) CloseHandle(lock->ready);
    if (lock->release) CloseHandle(lock->release);
    memset(lock,0,sizeof(*lock));
}
static int lock_clipboard(clipboard_lock* lock, HDESK desktop)
{
    lock->desktop=desktop; lock->ready=CreateEventW(NULL,TRUE,FALSE,NULL); lock->release=CreateEventW(NULL,TRUE,FALSE,NULL);
    if (!lock->ready || !lock->release) return 0;
    lock->thread=CreateThread(NULL,0,hold_clipboard,lock,0,NULL);
    return lock->thread && WaitForSingleObject(lock->ready,5000)==WAIT_OBJECT_0 && lock->opened;
}
static int expected_source(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot=NULL; uint64_t bytes; char actual[256]; int result;
    result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if (result==XUI_OK) result=xuiDocumentSnapshotCopySource(snapshot,actual,sizeof(actual),&bytes);
    xuiDocumentSnapshotRelease(snapshot);
    return result==XUI_OK && bytes==strlen(expected) && !strcmp(actual,expected);
}
static int find_text(xui_document document,const char* text,xui_doc_range_t* range)
{
    xui_document_snapshot snapshot=NULL; uint64_t matches; int result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if (result==XUI_OK) result=xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,text,strlen(text),NULL,range,1,&matches);
    xuiDocumentSnapshotRelease(snapshot); return result==XUI_OK && matches==1;
}
enum { LOCKED_COPY,LOCKED_CUT,LOCKED_PASTE,EMPTY_PASTE,COPY,CUT,PASTE,CHANGED_EMPTY_PASTE };
static xui_clipboard_get_text_proc original_get_text;
static unsigned text_reads;
static int changed_empty_text(xui_proxy proxy,char* text,int capacity)
{
    if (++text_reads==2) { if (capacity>0) text[0]=0; return 0; }
    return original_get_text(proxy,text,capacity);
}
static int exercise(xui_test_proxy_state_t* proxy,xui_proxy native,HDESK desktop,unsigned mode,unsigned operation)
{
    const char* initial="before keep 中文 😀 tail\n"; const char* selected="keep 中文 😀";
    const char* pasted="外部 中文 😀";
    xui_document document=NULL; xui_document_snapshot saved=NULL; xui_widget editor=NULL; xui_context context=NULL;
    xui_doc_desc_t document_desc={.iSize=sizeof(document_desc),.iProfile=mode ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH,.iMarkdownDialect=XUI_MD_GFM};
    xui_doc_editor_desc_t desc={0}; xui_doc_range_t selection,after; uint64_t revision;
    clipboard_lock lock={0}; xui_proxy_t effective=proxy->tProxy;
    char seed[96],text[128],expected[256]; int result,failed=0,has_native_text=0; DWORD sequence=0;
    if (operation<=LOCKED_PASTE) {
        effective.clipboardSetText=native->clipboardSetText;effective.clipboardGetText=native->clipboardGetText;
        effective.clipboardSetItems=native->clipboardSetItems;effective.clipboardGetData=native->clipboardGetData;
    } else if (operation==CHANGED_EMPTY_PASTE) {
        original_get_text=effective.clipboardGetText;text_reads=0;effective.clipboardGetText=changed_empty_text;
    }
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&effective)==XUI_OK);
    CHECK(xuiDocumentCreate(&document_desc,&document)==XUI_OK);
    if (mode) CHECK(xuiDocumentLoadMarkdown(document,initial,strlen(initial))==XUI_OK);
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=document;
    desc.iMode=mode==2 ? XUI_DOC_SOURCE_TEXT : mode==3 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK);
    CHECK(xuiSetRootWidget(context,editor)==XUI_OK && xuiWidgetSetRect(editor,(xui_rect_t){0,0,600,400})==XUI_OK);
    if (!mode) CHECK(xuiDocumentEditorInsertText(editor,initial,strlen(initial))==XUI_OK);
    CHECK(xuiDocumentClearHistory(document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&saved)==XUI_OK);
    CHECK(xuiDocumentMarkSaved(document,saved)==XUI_OK);
    if (mode>=2) {
        CHECK(xuiDocumentViewGetSelection(editor,&selection)==XUI_OK);
        selection.tAnchor.iOffset=strlen("before ");selection.tCaret.iOffset=selection.tAnchor.iOffset+strlen(selected);
    } else CHECK(find_text(document,selected,&selection));
    if (operation%2) { xui_doc_position_t swap=selection.tAnchor;selection.tAnchor=selection.tCaret;selection.tCaret=swap; }
    CHECK(xuiDocumentViewSetSelection(editor,&selection)==XUI_OK);
    revision=xuiDocumentGetRevision(document);
    snprintf(seed,sizeof(seed),"external-%u-%u 中文 😀\r\nsecond",mode,operation);
    CHECK(xuiTestProxySetClipboardText(proxy,operation==EMPTY_PASTE ? "" : operation==PASTE ? pasted : seed)==XUI_OK);
    if (operation<=LOCKED_PASTE) {
        CHECK(lock_clipboard(&lock,desktop));
        CHECK(GetOpenClipboardWindow()!=NULL);
        sequence=GetClipboardSequenceNumber();has_native_text=IsClipboardFormatAvailable(CF_UNICODETEXT);
    }
    result=xuiDocumentEditorExecute(editor,operation==LOCKED_COPY || operation==COPY ? XUI_DOC_EDIT_COPY : operation==LOCKED_CUT || operation==CUT ? XUI_DOC_EDIT_CUT : XUI_DOC_EDIT_PASTE);
    if (operation<=LOCKED_PASTE) {
        CHECK(GetClipboardSequenceNumber()==sequence);
        if (operation==LOCKED_PASTE && has_native_text) {
            int read=native->clipboardGetText(native,text,sizeof(text));
            if (read!=XUI_ERROR_BACKEND_FAILED) fprintf(stderr,"native clipboard read returned %d while held open\n",read);
            CHECK(read==XUI_ERROR_BACKEND_FAILED);
        }
    }
    unlock_clipboard(&lock);
    if (operation<=LOCKED_PASTE || operation==EMPTY_PASTE || operation==COPY || operation==CHANGED_EMPTY_PASTE) {
        int required=operation<=LOCKED_PASTE ? XUI_ERROR_BACKEND_FAILED : XUI_OK;
        if (operation==CHANGED_EMPTY_PASTE) required=XUI_DOC_ERROR_STALE;
        /* With no supported native format, Paste may be a successful no-op.
         * No current user clipboard payload is read to build this fixture. */
        if (operation==LOCKED_PASTE && !has_native_text && result==XUI_OK) required=XUI_OK;
        if (result!=required) fprintf(stderr,"native clipboard mode %u operation %u result=%d expected=%d revision=%llu/%llu\n",mode,operation,result,required,(unsigned long long)xuiDocumentGetRevision(document),(unsigned long long)revision);
        CHECK(result==required);
        if (operation==CHANGED_EMPTY_PASTE) CHECK(text_reads==2);
        CHECK(xuiDocumentGetRevision(document)==revision && !xuiDocumentIsDirty(document));
        CHECK(!xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
        CHECK(xuiDocumentViewGetSelection(editor,&after)==XUI_OK);
        CHECK(after.tAnchor.iOffset==selection.tAnchor.iOffset && after.tCaret.iOffset==selection.tCaret.iOffset);
        CHECK(find_text(document,"keep 中文 😀 tail",&after));
        if (mode) CHECK(expected_source(document,initial));
        if (operation==COPY) CHECK(effective.clipboardGetText(&effective,text,sizeof(text))==(int)strlen(selected) && !strcmp(text,selected));
    } else {
        CHECK(result==XUI_OK && xuiDocumentGetRevision(document)==revision+1 && xuiDocumentIsDirty(document));
        CHECK(xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
        if (operation==CUT) {
            CHECK(effective.clipboardGetText(&effective,text,sizeof(text))==(int)strlen(selected) && !strcmp(text,selected));
            CHECK(!find_text(document,selected,&after));
        } else CHECK(find_text(document,pasted,&after));
        snprintf(expected,sizeof(expected),"before %s tail\n",operation==CUT ? "" : pasted);
        if (mode) CHECK(expected_source(document,expected));
        CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document));
        CHECK(!xuiDocumentCanUndo(document) && xuiDocumentCanRedo(document));
        CHECK(find_text(document,"keep 中文 😀 tail",&after));
        if (mode) CHECK(expected_source(document,initial));
        CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && xuiDocumentIsDirty(document));
        if (mode) CHECK(expected_source(document,expected));
    }
done:
    unlock_clipboard(&lock);
    if (editor) xuiWidgetDestroy(editor);
    xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(document);
    if (context) xuiDestroy(context);
    if (failed) { failures++; fprintf(stderr,"Document clipboard case mode=%u operation=%u failed.\n",mode,operation); }
    return !failed;
}
int main(void)
{
    HDESK desktop=GetThreadDesktop(GetCurrentThreadId());
    xui_test_proxy_state_t proxy; xui_proxy_t native;
    xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL}; DWORD original_sequence=GetClipboardSequenceNumber(),final_sequence;
    HMODULE module=GetModuleHandleW(L"xge.dll"); char runtime[4096];
    int failed=0,xge_ready=0; unsigned modes=0;
    CHECK(module && GetModuleFileNameA(module,runtime,sizeof(runtime))>0);
    printf("Document native clipboard runtime: %s\n",runtime);
    CHECK(xgeInit(&desc)==XGE_OK); xge_ready=1;native=xuiProxyXge();
    xuiTestProxyInit(&proxy);
    for (unsigned mode=0;mode<4;mode++) {
        for (unsigned operation=0;operation<=CHANGED_EMPTY_PASTE;operation++) exercise(&proxy,&native,desktop,mode,operation);
        modes++;
    }
done:
    if (xge_ready) xgeUnit();
    final_sequence=GetClipboardSequenceNumber();
    printf("Document native clipboard: %u modes, %u failed cases, %u checks; native failures while locked, proxy successful/empty paths; clipboard sequence %lu -> %lu.\n",modes,failures,checks,(unsigned long)original_sequence,(unsigned long)final_sequence);
    return failed || failures ? 1 : 0;
}
