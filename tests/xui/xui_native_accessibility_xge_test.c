/* Actual XGE window, fonts, GL drawing and system UIA client. The regular
 * XGE input pump must install the provider; no explicit bridge binding. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <oleauto.h>
#include <uiautomation.h>
#include "xge.h"
#include "xui_document_ui.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if(!(c)){fprintf(stderr,"Native XGE UIA line %d: %s\n",__LINE__,#c);ExitProcess(1);} } while(0)
#define HR(c) do { HRESULT h_=(c);if(FAILED(h_)){fprintf(stderr,"Native XGE UIA line %d: %08lx: %s\n",__LINE__,(unsigned long)h_,#c);ExitProcess(1);} } while(0)
enum { WIDTH=800,HEIGHT=480,COMMAND_UNDO=1,COMMAND_LIVE=2 };
static const char markdown[]="# Native Markdown\n\nShared **bold** and `code`.\n\n- First item\n- Second item\n\nOne Document kernel.\n";
typedef struct native_probe {
    HWND window;HANDLE worker;DWORD started;LONG command,completed,finished;
    xui_context context;xui_widget root,rich,source;xui_document rich_document,md_document;
    xui_proxy_t proxy;xui_doc_font_set_t fonts;xui_surface target;
    unsigned frames;const char* image;
} native_probe;
static void request(native_probe* p,LONG command)
{
    DWORD started=GetTickCount();InterlockedExchange(&p->completed,0);InterlockedExchange(&p->command,command);
    while(!InterlockedCompareExchange(&p->completed,0,0)){CHECK(GetTickCount()-started<10000);Sleep(1);}
}
static IUIAutomationElement* named(IUIAutomation* client,IUIAutomationElement* root,const wchar_t* name)
{
    VARIANT value;IUIAutomationCondition* condition=NULL;IUIAutomationElement* result=NULL;
    VariantInit(&value);value.vt=VT_BSTR;value.bstrVal=SysAllocString(name);CHECK(value.bstrVal);
    HR(IUIAutomation_CreatePropertyCondition(client,UIA_NamePropertyId,value,&condition));VariantClear(&value);
    HR(IUIAutomationElement_FindFirst(root,TreeScope_Descendants,condition,&result));
    IUIAutomationCondition_Release(condition);CHECK(result);return result;
}
static DWORD WINAPI client_thread(void* user)
{
    native_probe* p=user;IUIAutomation* client=NULL;IUIAutomationElement *host=NULL,*rich=NULL,*source=NULL,*heading=NULL;
    IUIAutomationValuePattern* value=NULL;BSTR text=NULL;CONTROLTYPEID role;RECT bounds;
    HR(CoInitializeEx(NULL,COINIT_MULTITHREADED));
    HR(CoCreateInstance(&CLSID_CUIAutomation,NULL,CLSCTX_INPROC_SERVER,&IID_IUIAutomation,(void**)&client));
    HR(IUIAutomation_ElementFromHandle(client,p->window,&host));
    rich=named(client,host,L"Native rich editor");source=named(client,host,L"Native Markdown editor");
    HR(IUIAutomationElement_get_CurrentControlType(rich,&role));CHECK(role==UIA_DocumentControlTypeId);
    HR(IUIAutomationElement_get_CurrentControlType(source,&role));CHECK(role==UIA_EditControlTypeId);
    HR(IUIAutomationElement_get_CurrentBoundingRectangle(rich,&bounds));CHECK(bounds.right-bounds.left>300 && bounds.bottom-bounds.top>300);
    HR(IUIAutomationElement_get_CurrentBoundingRectangle(source,&bounds));CHECK(bounds.right-bounds.left>300 && bounds.bottom-bounds.top>300);
    heading=named(client,rich,L"Native Rich text");
    HR(IUIAutomationElement_GetCurrentPattern(source,UIA_ValuePatternId,(IUnknown**)&value));CHECK(value);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));CHECK(text && wcsstr(text,L"# Native Markdown") && wcsstr(text,L"**bold**"));SysFreeString(text);text=NULL;
    HR(IUIAutomationValuePattern_SetValue(value,L"# \x4f60\x597d \xd83d\xde00\n\nNative system edit.\n"));
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));CHECK(text && !wcscmp(text,L"# \x4f60\x597d \xd83d\xde00\n\nNative system edit.\n"));SysFreeString(text);text=NULL;
    request(p,COMMAND_UNDO);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));CHECK(text && wcsstr(text,L"# Native Markdown") && wcsstr(text,L"**bold**"));SysFreeString(text);text=NULL;
    request(p,COMMAND_LIVE);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));CHECK(text && wcsstr(text,L"# Native Markdown"));SysFreeString(text);
    puts("Actual XGE UIA: automatic input-pump binding, Rich/Markdown roles, native bounds, UTF-16 SetValue, shared Undo and LIVE passed");fflush(stdout);
    IUIAutomationValuePattern_Release(value);IUIAutomationElement_Release(heading);
    IUIAutomationElement_Release(source);IUIAutomationElement_Release(rich);IUIAutomationElement_Release(host);IUIAutomation_Release(client);
    CoUninitialize();InterlockedExchange(&p->finished,1);return 0;
}
static uint64_t add(xui_document_transaction t,uint64_t parent,uint32_t kind,const char* text)
{
    xui_doc_node_desc_t desc={0};uint64_t id=0;desc.iSize=sizeof(desc);desc.iKind=kind;
    desc.sText=text;desc.iTextBytes=text?strlen(text):0;desc.tAttributes.iHeadingLevel=kind==XUI_DOC_HEADING?1:0;
    CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&desc,&id)==XUI_OK);return id;
}
static void setup(native_probe* p)
{
    xui_surface_desc_t surface={0};xui_doc_desc_t document={0};xui_doc_editor_desc_t editor={0};xui_document_transaction transaction;uint64_t heading,paragraph;
    p->window=(HWND)xgePlatformNativeHandle();CHECK(p->window && IsWindow(p->window));ShowWindow(p->window,SW_HIDE);
    printf("Actual XGE GL: vendor=%s renderer=%s version=%s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
    CHECK(glGetString(GL_VERSION));p->proxy=xuiProxyXge();
    CHECK(xuiCreate(&p->context)==XUI_OK);CHECK(xuiSetProxy(p->context,&p->proxy)==XUI_OK);
    CHECK(p->proxy.fontLoadFile(&p->proxy,&p->fonts.normal,"C:\\Windows\\Fonts\\segoeui.ttf",18,XUI_FONT_FORMAT_TTF)==XUI_OK);
    CHECK(p->proxy.fontLoadFile(&p->proxy,&p->fonts.bold,"C:\\Windows\\Fonts\\segoeuib.ttf",18,XUI_FONT_FORMAT_TTF)==XUI_OK);
    CHECK(p->proxy.fontLoadFile(&p->proxy,&p->fonts.monospace,"C:\\Windows\\Fonts\\consola.ttf",17,XUI_FONT_FORMAT_TTF)==XUI_OK);
    p->fonts.italic=p->fonts.normal;p->fonts.boldItalic=p->fonts.bold;
    CHECK(xuiSetDefaultFont(p->context,p->fonts.normal)==XUI_OK);CHECK(xuiSetViewportSize(p->context,WIDTH,HEIGHT)==XUI_OK);
    CHECK(xuiWidgetCreate(p->context,&p->root)==XUI_OK);CHECK(xuiSetRootWidget(p->context,p->root)==XUI_OK);
    CHECK(xuiWidgetSetRect(p->root,(xui_rect_t){0,0,WIDTH,HEIGHT})==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&p->rich_document)==XUI_OK);
    CHECK(xuiDocumentBeginTransaction(p->rich_document,NULL,&transaction)==XUI_OK);
    heading=add(transaction,1,XUI_DOC_HEADING,NULL);add(transaction,heading,XUI_DOC_TEXT,"Native Rich text");
    paragraph=add(transaction,1,XUI_DOC_PARAGRAPH,NULL);add(transaction,paragraph,XUI_DOC_TEXT,"Shared Document transactions, rendering and history.");
    CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
    document.iSize=sizeof(document);document.iProfile=XUI_DOCUMENT_MARKDOWN;document.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&document,&p->md_document)==XUI_OK);CHECK(xuiDocumentLoadMarkdown(p->md_document,markdown,sizeof(markdown)-1)==XUI_OK);
    editor.iSize=sizeof(editor);editor.tView.iSize=sizeof(editor.tView);editor.tView.pDocument=p->rich_document;editor.iMode=XUI_DOC_VISUAL;
    editor.tView.tRenderer.iSize=sizeof(editor.tView.tRenderer);editor.tView.tRenderer.tFonts=p->fonts;
    CHECK(xuiDocumentEditorCreate(p->context,&editor,&p->rich)==XUI_OK);
    CHECK(xuiWidgetSetAccessibleName(p->rich,"Native rich editor")==XUI_OK);
    CHECK(xuiWidgetSetRect(p->rich,(xui_rect_t){20,20,360,440})==XUI_OK);CHECK(xuiWidgetAddChild(p->root,p->rich)==XUI_OK);
    editor.tView.pDocument=p->md_document;editor.iMode=XUI_DOC_SOURCE_TEXT;
    CHECK(xuiDocumentEditorCreate(p->context,&editor,&p->source)==XUI_OK);
    CHECK(xuiWidgetSetAccessibleName(p->source,"Native Markdown editor")==XUI_OK);
    CHECK(xuiWidgetSetRect(p->source,(xui_rect_t){420,20,360,440})==XUI_OK);CHECK(xuiWidgetAddChild(p->root,p->source)==XUI_OK);
    surface.iWidth=WIDTH;surface.iHeight=HEIGHT;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(p->proxy.surfaceCreate(&p->proxy,&p->target,&surface)==XUI_OK);
}
static void capture(native_probe* p)
{
    unsigned char* pixels=malloc(WIDTH*HEIGHT*4);unsigned left=0,right=0;CHECK(pixels);
    CHECK(p->proxy.surfaceReadRGBA(&p->proxy,p->target,pixels,WIDTH*4)==XUI_OK);
    for(int y=20;y<460;y++)for(int x=20;x<780;x++) {
        unsigned char* pixel=pixels+((size_t)y*WIDTH+x)*4;
        if(pixel[3]>200 && (pixel[0]<200 || pixel[1]<200 || pixel[2]<200)) {
            if(x<380)left++;
            if(x>=420)right++;
        }
    }
    CHECK(left>500 && right>1000);CHECK(xgeImageSavePNG(p->image,WIDTH,HEIGHT,pixels,WIDTH*4)==XGE_OK);
    printf("Native XGE RGBA accepted: Rich ink=%u Markdown ink=%u frames=%u image=%s\n",left,right,p->frames,p->image);fflush(stdout);free(pixels);
}
static int frame(void* user)
{
    native_probe* p=user;xui_rect_i_t damage={0,0,WIDTH,HEIGHT};LONG command;
    CHECK(GetTickCount()-p->started<60000);if(!p->context)setup(p);
    CHECK(xgeBegin()==XGE_OK);CHECK(xuiProxyXgePumpInput(p->context)==XUI_OK);
    command=InterlockedExchange(&p->command,0);
    if(command==COMMAND_UNDO)CHECK(xuiDocumentUndo(p->md_document,NULL)==XUI_OK);
    if(command==COMMAND_LIVE)CHECK(xuiDocumentViewSetMode(p->source,XUI_DOC_LIVE_MARKDOWN)==XUI_OK);
    if(command)InterlockedExchange(&p->completed,1);
    CHECK(xuiLayout(p->context)==XUI_OK);CHECK(xuiUpdate(p->context,.016f)==XUI_OK);
    CHECK(p->proxy.surfaceClear(&p->proxy,p->target,XUI_COLOR_WHITE)==XUI_OK);
    CHECK(xuiRender(p->context,p->target,&damage,1)==XUI_OK);CHECK(xgeEnd()==XGE_OK);p->frames++;
    if(!p->worker){p->worker=CreateThread(NULL,0,client_thread,p,0,NULL);CHECK(p->worker);}
    if(InterlockedCompareExchange(&p->finished,0,0)){capture(p);xgeQuit();}
    return XGE_OK;
}
int main(int argc,char** argv)
{
    native_probe p={0};xge_desc_t desc={0};DWORD result=1;CHECK(argc==2);p.image=argv[1];p.started=GetTickCount();
    desc.iWidth=WIDTH;desc.iHeight=HEIGHT;desc.sTitle="Document native UIA acceptance";desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK);CHECK(xgeRun(frame,&p)==XGE_OK);CHECK(p.worker && WaitForSingleObject(p.worker,10000)==WAIT_OBJECT_0);
    CHECK(GetExitCodeThread(p.worker,&result) && result==0);CloseHandle(p.worker);
    p.proxy.surfaceDestroy(&p.proxy,p.target);xuiDestroy(p.context);xuiDocumentRelease(p.rich_document);xuiDocumentRelease(p.md_document);
    p.proxy.fontDestroy(&p.proxy,p.fonts.normal);p.proxy.fontDestroy(&p.proxy,p.fonts.bold);p.proxy.fontDestroy(&p.proxy,p.fonts.monospace);xgeUnit();
    puts("Native XGE automatic UIA integration passed; TextPattern and physical reader acceptance remain incomplete.");return 0;
}
