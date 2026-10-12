/* Actual Windows UI Automation client on an MTA worker, with the provider
 * hosted on its UI STA. Renderer/fonts are the deterministic test proxy. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <oleauto.h>
#include <uiautomation.h>
#include "xui_document_ui.h"
#include "../src/xui/accessibility/xui_native_accessibility_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr,"UIA line %d: %s\n",__LINE__,#c); ExitProcess(1); } } while (0)
#define HR(c) do { HRESULT hr_ = (c); if (FAILED(hr_)) { fprintf(stderr,"UIA line %d: HRESULT %08lx: %s\n",__LINE__,(unsigned long)hr_,#c); ExitProcess(1); } } while (0)
enum { CHANGE_MODE=WM_APP+1, UNDO_DOCUMENT, GET_ACTIVATIONS, DESTROY_EDITOR, RECREATE_EDITOR, DESTROY_CONTEXT, GET_CALLBACKS, SAME_DOCUMENT, REPLACE_DOCUMENT };
typedef struct probe_data { HWND window; xui_context context; xui_widget editor; xui_document document; int activations, callbacks; } probe_data;
static const char markdown[]="# \xe6\xa0\x87\xe9\xa2\x98\n\n- [ ] todo\n\n[go](https://example.invalid)\n\n| A | B |\n| - | - |\n| x | y |\n";
static void activated(xui_widget w, uint64_t id, const char* resource, void* user)
{ probe_data* data=user; (void)w; (void)id; CHECK(!strcmp(resource,"https://example.invalid")); data->activations++; }
static void notified(xui_context context, xui_widget widget, const xui_accessibility_event_t* event, void* user)
{ probe_data* data=user; (void)context; (void)widget; CHECK(event->iSize==sizeof(*event)); data->callbacks++; }
static void make_editor(probe_data* data)
{
    xui_doc_editor_desc_t desc={0}; desc.iSize=sizeof(desc); desc.iMode=XUI_DOC_VISUAL;
    desc.tView.iSize=sizeof(desc.tView); desc.tView.pDocument=data->document;
    desc.tView.onActivate=activated; desc.tView.pUser=data;
    CHECK(xuiDocumentEditorCreate(data->context,&desc,&data->editor)==XUI_OK);
    CHECK(xuiSetRootWidget(data->context,data->editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(data->editor,(xui_rect_t){0,0,640,480})==XUI_OK);
    CHECK(xuiLayout(data->context)==XUI_OK);
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    probe_data* data=(probe_data*)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(data) switch(message) {
    case CHANGE_MODE: return xuiDocumentViewSetMode(data->editor,(uint32_t)wp);
    case UNDO_DOCUMENT: return xuiDocumentUndo(data->document,NULL);
    case SAME_DOCUMENT: return xuiDocumentViewSetDocument(data->editor,data->document);
    case REPLACE_DOCUMENT: {
        xui_doc_desc_t desc={0}; xui_document next=NULL;
        xui_accessible_node_t before={0},after={0};
        before.iSize=after.iSize=sizeof(before);
        CHECK(xuiWidgetGetAccessibleNode(data->editor,0,&before)==XUI_OK);
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&desc,&next)==XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(next,markdown,sizeof(markdown)-1)==XUI_OK);
        CHECK(xuiDocumentViewSetDocument(data->editor,next)==XUI_OK);
        xuiDocumentRelease(data->document);data->document=next;
        CHECK(xuiLayout(data->context)==XUI_OK);
        CHECK(xuiWidgetGetAccessibleNode(data->editor,0,&after)==XUI_OK);
        CHECK(before.iId==after.iId && before.iRole==after.iRole);
        return 0;
    }
    case GET_ACTIVATIONS: return data->activations;
    case GET_CALLBACKS: CHECK(xuiUpdate(data->context,0)==XUI_OK); return data->callbacks;
    case DESTROY_EDITOR: xuiWidgetDestroy(data->editor); data->editor=NULL; return 0;
    case RECREATE_EDITOR: make_editor(data); return 0;
    case DESTROY_CONTEXT: xuiDestroy(data->context); data->context=NULL; data->editor=NULL; return 0;
    default: break;
    }
    return DefWindowProcW(window,message,wp,lp);
}
static IUIAutomationElement* find(IUIAutomation* client,IUIAutomationElement* root,PROPERTYID id,VARIANT value)
{
    IUIAutomationCondition* condition=NULL; IUIAutomationElement* element=NULL;
    HR(IUIAutomation_CreatePropertyCondition(client,id,value,&condition));
    HR(IUIAutomationElement_FindFirst(root,TreeScope_Descendants,condition,&element));
    IUIAutomationCondition_Release(condition); CHECK(element); return element;
}
static IUIAutomationElement* by_type(IUIAutomation* client,IUIAutomationElement* root,int type)
{ VARIANT value; VariantInit(&value); value.vt=VT_I4; value.lVal=type; return find(client,root,UIA_ControlTypePropertyId,value); }
static IUIAutomationElement* by_name(IUIAutomation* client,IUIAutomationElement* root,const wchar_t* name)
{
    VARIANT value; IUIAutomationElement* result; VariantInit(&value); value.vt=VT_BSTR; value.bstrVal=SysAllocString(name);
    CHECK(value.bstrVal); result=find(client,root,UIA_NamePropertyId,value); VariantClear(&value); return result;
}
static void named(IUIAutomationElement* element,const wchar_t* expected)
{ BSTR value=NULL; HR(IUIAutomationElement_get_CurrentName(element,&value)); CHECK(value && !wcscmp(value,expected)); SysFreeString(value); }
static int different_identity(SAFEARRAY* before,SAFEARRAY* after)
{
    LONG low,high,next_low,next_high,index;
    CHECK(before && after && before->cDims==1 && after->cDims==1);
    HR(SafeArrayGetLBound(before,1,&low));HR(SafeArrayGetUBound(before,1,&high));
    HR(SafeArrayGetLBound(after,1,&next_low));HR(SafeArrayGetUBound(after,1,&next_high));
    if(low!=next_low || high!=next_high)return 1;
    for(index=low;index<=high;index++) {
        LONG old_id,new_id;
        HR(SafeArrayGetElement(before,&index,&old_id));HR(SafeArrayGetElement(after,&index,&new_id));
        if(old_id!=new_id)return 1;
    }
    return 0;
}
static DWORD WINAPI client_thread(void* user)
{
    probe_data* data=user; IUIAutomation* client=NULL; IUIAutomationElement *host=NULL,*document=NULL,*heading=NULL,*task=NULL,*link=NULL,*source=NULL;
    IUIAutomationTogglePattern* toggle=NULL; IUIAutomationInvokePattern* invoke=NULL; IUIAutomationValuePattern* value=NULL;
    enum ToggleState state; BSTR text=NULL; HRESULT stale; SAFEARRAY* identity=NULL; BOOL same=FALSE;
    HR(CoInitializeEx(NULL,COINIT_MULTITHREADED));
    HR(CoCreateInstance(&CLSID_CUIAutomation,NULL,CLSCTX_INPROC_SERVER,&IID_IUIAutomation,(void**)&client));
    HR(IUIAutomation_ElementFromHandle(client,data->window,&host)); named(host,L"XUI");
    document=by_type(client,host,UIA_DocumentControlTypeId); named(document,L"Document");
    heading=by_name(client,host,L"\x6807\x9898");
    HR(IUIAutomationElement_GetRuntimeId(heading,&identity)); CHECK(identity && identity->cDims==1);
    task=by_type(client,host,UIA_CheckBoxControlTypeId);
    HR(IUIAutomationElement_GetCurrentPattern(task,UIA_TogglePatternId,(IUnknown**)&toggle)); CHECK(toggle);
    HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_Off);
    HR(IUIAutomationTogglePattern_Toggle(toggle)); HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_On);
    CHECK(SendMessageW(data->window,UNDO_DOCUMENT,0,0)==XUI_OK);
    HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_Off);
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      HR(IUIAutomation_CompareElements(client,heading,again,&same)); CHECK(same); IUIAutomationElement_Release(again); }
    link=by_type(client,host,UIA_HyperlinkControlTypeId); named(link,L"go");
    HR(IUIAutomationElement_GetCurrentPattern(link,UIA_InvokePatternId,(IUnknown**)&invoke)); CHECK(invoke);
    HR(IUIAutomationInvokePattern_Invoke(invoke)); CHECK(SendMessageW(data->window,GET_ACTIVATIONS,0,0)==1);
    puts("Actual UIA: semantic Document/heading/link/task nodes, stable identity, Toggle/Undo and Invoke passed"); fflush(stdout);
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_SOURCE_TEXT,0)==XUI_OK);
    source=by_type(client,host,UIA_EditControlTypeId);
    HR(IUIAutomationElement_GetCurrentPattern(source,UIA_ValuePatternId,(IUnknown**)&value)); CHECK(value);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));
    CHECK(text && wcsstr(text,L"# \x6807\x9898") && wcsstr(text,L"[go](https://example.invalid)")); SysFreeString(text); text=NULL;
    HR(IUIAutomationValuePattern_SetValue(value,L"# \x4f60\x597d \xd83d\xde00\n"));
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text)); CHECK(text && !wcscmp(text,L"# \x4f60\x597d \xd83d\xde00\n")); SysFreeString(text); text=NULL;
    CHECK(SendMessageW(data->window,UNDO_DOCUMENT,0,0)==XUI_OK);
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_LIVE_MARKDOWN,0)==XUI_OK);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text)); CHECK(text && wcsstr(text,L"# \x6807\x9898")); SysFreeString(text); text=NULL;
    puts("Actual UIA: SOURCE/LIVE raw Markdown, UTF-16 CJK/surrogate pair SetValue and shared Undo passed"); fflush(stdout);
    IUIAutomationValuePattern_Release(value); value=NULL;
    IUIAutomationElement_Release(source); source=NULL;
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_VISUAL,0)==XUI_OK);
    CHECK(SendMessageW(data->window,GET_CALLBACKS,0,0)>0);
    CHECK(SendMessageW(data->window,SAME_DOCUMENT,0,0)==XUI_OK);
    named(heading,L"\x6807\x9898");
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      HR(IUIAutomation_CompareElements(client,heading,again,&same));CHECK(same);IUIAutomationElement_Release(again); }
    CHECK(SendMessageW(data->window,REPLACE_DOCUMENT,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(document,&text);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    stale=IUIAutomationElement_get_CurrentName(heading,&text);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    stale=IUIAutomationTogglePattern_Toggle(toggle);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);
    stale=IUIAutomationInvokePattern_Invoke(invoke);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);
    IUIAutomationInvokePattern_Release(invoke);invoke=NULL;
    IUIAutomationTogglePattern_Release(toggle);toggle=NULL;
    IUIAutomationElement_Release(link);link=NULL;IUIAutomationElement_Release(task);task=NULL;
    IUIAutomationElement_Release(heading);IUIAutomationElement_Release(document);
    document=by_type(client,host,UIA_DocumentControlTypeId);named(document,L"Document");
    heading=by_name(client,host,L"\x6807\x9898");
    { SAFEARRAY* next_identity=NULL;
      HR(IUIAutomationElement_GetRuntimeId(heading,&next_identity));CHECK(different_identity(identity,next_identity));
      SafeArrayDestroy(identity);identity=next_identity; }
    { IUIAutomationElement* current_task=by_type(client,host,UIA_CheckBoxControlTypeId);
      IUIAutomationTogglePattern* current_toggle=NULL;
      HR(IUIAutomationElement_GetCurrentPattern(current_task,UIA_TogglePatternId,(IUnknown**)&current_toggle));CHECK(current_toggle);
      HR(IUIAutomationTogglePattern_get_CurrentToggleState(current_toggle,&state));CHECK(state==ToggleState_Off);
      IUIAutomationTogglePattern_Release(current_toggle);IUIAutomationElement_Release(current_task); }
    CHECK(SendMessageW(data->window,GET_ACTIVATIONS,0,0)==1);
    puts("Actual UIA: same Document retains identity; replacement reuses semantic root id but invalidates old nodes/actions and gets new runtime ids");fflush(stdout);
    CHECK(SendMessageW(data->window,DESTROY_EDITOR,0,0)==0);
    CHECK(SendMessageW(data->window,RECREATE_EDITOR,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(heading,&text); CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE); if(text)SysFreeString(text);text=NULL;
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      SAFEARRAY* next_identity=NULL;
      HR(IUIAutomationElement_GetRuntimeId(again,&next_identity));CHECK(different_identity(identity,next_identity));
      SafeArrayDestroy(identity);SafeArrayDestroy(next_identity);IUIAutomationElement_Release(again); }
    CHECK(SendMessageW(data->window,DESTROY_CONTEXT,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(document,&text); CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE); if(text)SysFreeString(text);
    puts("Actual UIA: user callback retained; destroyed widget/context unavailable; recreation has a new identity passed"); fflush(stdout);
    IUIAutomationElement_Release(heading);
    IUIAutomationElement_Release(document); IUIAutomationElement_Release(host); IUIAutomation_Release(client);
    CoUninitialize(); return 0;
}
int main(void)
{
    probe_data data={0}; xui_test_proxy_state_t proxy; xui_font font=NULL; xui_doc_desc_t desc={0};
    WNDCLASSW type={0}; HANDLE thread; DWORD started=GetTickCount(),result=1; MSG message;
    HR(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED));
    type.lpfnWndProc=window_proc; type.hInstance=GetModuleHandleW(NULL); type.lpszClassName=L"XUI.NativeAccessibility.Probe";
    CHECK(RegisterClassW(&type)); data.window=CreateWindowW(type.lpszClassName,L"XUI Document UIA probe",WS_OVERLAPPEDWINDOW,0,0,800,600,NULL,NULL,type.hInstance,NULL); CHECK(data.window);
    SetWindowLongPtrW(data.window,GWLP_USERDATA,(LONG_PTR)&data);
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&data.context)==XUI_OK);
    CHECK(xuiSetProxy(data.context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(data.context,font)==XUI_OK);
    CHECK(xuiSetViewportSize(data.context,640,480)==XUI_OK); desc.iSize=sizeof(desc); desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc,&data.document)==XUI_OK); CHECK(xuiDocumentLoadMarkdown(data.document,markdown,sizeof(markdown)-1)==XUI_OK);
    make_editor(&data); CHECK(xuiSetAccessibilityEventCallback(data.context,notified,&data)==XUI_OK);
    CHECK(xuiInternalNativeAccessibilityBind(data.context,data.window,(xui_rect_t){0,0,640,480})==XUI_OK);
    thread=CreateThread(NULL,0,client_thread,&data,0,NULL);CHECK(thread);
    while(WaitForSingleObject(thread,0)==WAIT_TIMEOUT) {
        CHECK(GetTickCount()-started<60000);
        (void)MsgWaitForMultipleObjects(1,&thread,FALSE,50,QS_ALLINPUT);
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
    CHECK(GetExitCodeThread(thread,&result));CloseHandle(thread);
    if(data.context)xuiDestroy(data.context);
    xuiDocumentRelease(data.document);proxy.tProxy.fontDestroy(&proxy.tProxy,font);DestroyWindow(data.window);
    UnregisterClassW(type.lpszClassName,type.hInstance);CoUninitialize();
    CHECK(result==0); puts("Native UIA bridge integration passed; TextPattern and physical screen-reader acceptance remain incomplete.");return 0;
}
