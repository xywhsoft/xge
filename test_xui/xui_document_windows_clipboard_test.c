/* Successful native clipboard paths on a newly created, private window station.
 * Never reuse a station or access the interactive clipboard payload. Exit 77
 * means that isolation is unavailable, not that the product passed. */
#include "../xui_document_ui.h"
#include "../xge.h"
#include "xui_test_proxy.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static unsigned checks, cases, peers;
static int payload_attempted;
static const char* selected="keep 中文 😀";
static const char* pasted="外部 中文 😀";
#define CHECK(e) do { checks++; if (!(e)) { fprintf(stderr,"native clipboard line %d: %s (win32=%lu)\n",__LINE__,#e,(unsigned long)GetLastError());failed=1;goto done; } } while(0)

static int administrator_enabled(int* enabled)
{
    SID_IDENTIFIER_AUTHORITY authority=SECURITY_NT_AUTHORITY;PSID sid=NULL;BOOL member=FALSE;
    if (!AllocateAndInitializeSid(&authority,2,SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&sid)) return 0;
    BOOL result=CheckTokenMembership(NULL,sid,&member);FreeSid(sid);
    if (result) *enabled=member!=FALSE;
    return result!=FALSE;
}
static int user_station_security(SECURITY_ATTRIBUTES* attributes,
    SECURITY_DESCRIPTOR* descriptor,TOKEN_USER** user,PACL* acl)
{
    HANDLE token=NULL;DWORD bytes=0;int result=0;
    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)) return 0;
    (void)GetTokenInformation(token,TokenUser,NULL,0,&bytes);
    if (!bytes || !(*user=malloc(bytes))) goto done;
    if (!GetTokenInformation(token,TokenUser,*user,bytes,&bytes)) goto done;
    bytes=sizeof(ACL)+sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD)+GetLengthSid((*user)->User.Sid);
    if (!(*acl=malloc(bytes)) || !InitializeAcl(*acl,bytes,ACL_REVISION) ||
        !AddAccessAllowedAce(*acl,ACL_REVISION,GENERIC_ALL,(*user)->User.Sid) ||
        !InitializeSecurityDescriptor(descriptor,SECURITY_DESCRIPTOR_REVISION) ||
        !SetSecurityDescriptorOwner(descriptor,(*user)->User.Sid,FALSE) ||
        !SetSecurityDescriptorDacl(descriptor,TRUE,*acl,FALSE) ||
        !SetSecurityDescriptorControl(descriptor,SE_DACL_PROTECTED,SE_DACL_PROTECTED)) goto done;
    attributes->nLength=sizeof(*attributes);attributes->lpSecurityDescriptor=descriptor;
    attributes->bInheritHandle=FALSE;result=1;
done:
    CloseHandle(token);return result;
}
static int private_station_is(const WCHAR* expected)
{
    WCHAR actual[128];DWORD needed=0;USEROBJECTFLAGS flags={0};
    const WCHAR* prefix=L"XUI-Document-Clipboard-";
    HWINSTA station=GetProcessWindowStation();
    return expected && !wcsncmp(expected,prefix,wcslen(prefix)) &&
        GetUserObjectInformationW(station,UOI_NAME,actual,sizeof(actual),&needed) &&
        !wcscmp(expected,actual) &&
        GetUserObjectInformationW(station,UOI_FLAGS,&flags,sizeof(flags),&needed) &&
        !(flags.dwFlags&WSF_VISIBLE);
}
static int peer_action(int write,const WCHAR* station)
{
    HWND owner=NULL;HGLOBAL data=NULL;const WCHAR* wide=NULL;int failed=0,opened=0,open_attempted=0,payload=0;
    CHECK(private_station_is(station));
    owner=CreateWindowExW(0,L"STATIC",L"Private clipboard peer",WS_POPUP,
        0,0,0,0,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(owner);
    open_attempted=1;CHECK(OpenClipboard(owner));opened=1;payload=1;
    if (write) {
        const WCHAR* text=L"外部 中文 😀";size_t bytes=(wcslen(text)+1)*sizeof(WCHAR);
        data=GlobalAlloc(GMEM_MOVEABLE,bytes);CHECK(data);
        WCHAR* target=GlobalLock(data);CHECK(target);memcpy(target,text,bytes);GlobalUnlock(data);
        CHECK(EmptyClipboard());CHECK(SetClipboardData(CF_UNICODETEXT,data));data=NULL;
    } else {
        const WCHAR* expected=L"keep 中文 😀";
        data=GetClipboardData(CF_UNICODETEXT);CHECK(data);
        CHECK(GlobalSize(data)>=(wcslen(expected)+1)*sizeof(WCHAR));
        wide=GlobalLock(data);CHECK(wide);
        CHECK(!memcmp(wide,expected,(wcslen(expected)+1)*sizeof(WCHAR)));
    }
done:
    if (wide) GlobalUnlock(data);
    if (write && data) GlobalFree(data);
    if (opened && !CloseClipboard()) failed=1;
    if (owner && !DestroyWindow(owner)) failed=1;
    printf("Private native clipboard peer %s: %s; clipboard_open_attempted=%d; payload_attempted=%d.\n",
        write?"write":"read",failed?"failed":"passed",open_attempted,payload);
    return failed?1:0;
}
static int run_peer(int write,const WCHAR* station)
{
    WCHAR executable[32768],command[33024],desktop[160];STARTUPINFOW startup={0};
    PROCESS_INFORMATION process={0};DWORD code=1;int failed=0,launched=0;
    CHECK(private_station_is(station));
    CHECK(GetModuleFileNameW(NULL,executable,32768)>0);
    CHECK(swprintf(command,33024,L"\"%ls\" --peer-%ls %ls",executable,write?L"write":L"read",station)>0);
    CHECK(swprintf(desktop,160,L"%ls\\Default",station)>0);
    startup.cb=sizeof(startup);startup.lpDesktop=desktop;
    startup.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;startup.wShowWindow=SW_HIDE;
    startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    CHECK(CreateProcessW(executable,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,
        NULL,NULL,&startup,&process));launched=1;peers++;
    CHECK(WaitForSingleObject(process.hProcess,10000)==WAIT_OBJECT_0);
    CHECK(GetExitCodeProcess(process.hProcess,&code) && code==0);
done:
    if (launched) {
        if (WaitForSingleObject(process.hProcess,0)!=WAIT_OBJECT_0) {
            TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,5000);failed=1;
        }
        CloseHandle(process.hThread);CloseHandle(process.hProcess);
    }
    return !failed;
}
static int exercise(xui_proxy native,const WCHAR* station,int mode,int operation)
{
    const char* initial="before keep 中文 😀 tail\n";xui_test_proxy_state_t proxy;
    xui_context context=NULL;xui_widget editor=NULL;xui_document document=NULL;
    xui_document_snapshot saved=NULL;xui_doc_editor_desc_t desc={0};xui_doc_desc_t document_desc={0};
    xui_proxy_t effective;char *before=NULL,*expected=NULL;int first,last,a,b,failed=0;uint64_t revision;
    CHECK(private_station_is(station));
    xuiTestProxyInit(&proxy);effective=proxy.tProxy;
    effective.clipboardSetText=native->clipboardSetText;effective.clipboardGetText=native->clipboardGetText;
    effective.clipboardSetItems=native->clipboardSetItems;effective.clipboardGetData=native->clipboardGetData;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&effective)==XUI_OK);
    document_desc.iSize=sizeof(document_desc);document_desc.iProfile=mode?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
    document_desc.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&document_desc,&document)==XUI_OK);
    if (mode) CHECK(xuiDocumentLoadMarkdown(document,initial,strlen(initial))==XUI_OK);
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=document;
    desc.iMode=mode==1?XUI_DOC_SOURCE_TEXT:mode==3?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK);
    CHECK(xuiSetRootWidget(context,editor)==XUI_OK && xuiWidgetSetRect(editor,(xui_rect_t){0,0,600,400})==XUI_OK);
    if (!mode) CHECK(xuiDocumentEditorInsertText(editor,initial,strlen(initial))==XUI_OK);
    CHECK(xuiDocumentClearHistory(document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&saved)==XUI_OK);
    CHECK(xuiDocumentMarkSaved(document,saved)==XUI_OK);
    const char* text=xuiEditGetText(editor);CHECK(text && strstr(text,selected));
    before=malloc(strlen(text)+1);CHECK(before);strcpy(before,text);
    first=(int)(strstr(before,selected)-before);last=first+(int)strlen(selected);
    CHECK(xuiEditSetSelection(editor,mode%2?last:first,mode%2?first:last)==XUI_OK);
    CHECK(xuiEditGetSelection(editor,&a,&b)==XUI_OK);revision=xuiDocumentGetRevision(document);
    payload_attempted=1;
    if (operation==2) CHECK(run_peer(1,station));
    CHECK(xuiDocumentEditorExecute(editor,operation==0?XUI_DOC_EDIT_COPY:operation==1?XUI_DOC_EDIT_CUT:XUI_DOC_EDIT_PASTE)==XUI_OK);
    if (operation<2) CHECK(run_peer(0,station));
    if (!operation) {
        int after_a,after_b;
        CHECK(!strcmp(xuiEditGetText(editor),before) && xuiDocumentGetRevision(document)==revision);
        CHECK(!xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
        CHECK(xuiEditGetSelection(editor,&after_a,&after_b)==XUI_OK && a==after_a && b==after_b);
    } else {
        const char* replacement=operation==1?"":pasted;
        expected=malloc(strlen(before)+strlen(replacement)+1);CHECK(expected);
        memcpy(expected,before,(size_t)first);strcpy(expected+first,replacement);strcat(expected,before+last);
        CHECK(!strcmp(xuiEditGetText(editor),expected) && xuiDocumentGetRevision(document)==revision+1);
        CHECK(xuiDocumentIsDirty(document) && xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
        CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document));
        CHECK(!strcmp(xuiEditGetText(editor),before) && !xuiDocumentCanUndo(document) && xuiDocumentCanRedo(document));
        CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && xuiDocumentIsDirty(document));
        CHECK(!strcmp(xuiEditGetText(editor),expected));
    }
done:
    free(before);free(expected);xuiDocumentSnapshotRelease(saved);
    if (context) xuiDestroy(context);
    xuiDocumentRelease(document);
    if (!failed) cases++;
    printf("Native clipboard mode=%d operation=%d: %s.\n",mode,operation,failed?"failed":"passed");
    return !failed;
}
int main(int argc,char** argv)
{
    HWINSTA original=GetProcessWindowStation(),station=NULL;
    HDESK original_desktop=GetThreadDesktop(GetCurrentThreadId()),desktop=NULL;
    WCHAR name[128];DWORD before=GetClipboardSequenceNumber(),after=before;
    int failed=0,enabled=0,unavailable=0,created=0,verified=0,station_changed=0,desktop_changed=0,initialized=0,restored=1;
    xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL};xui_proxy_t native;
    SECURITY_ATTRIBUTES security={0};SECURITY_DESCRIPTOR descriptor;TOKEN_USER* security_user=NULL;PACL acl=NULL;
    if (argc==3 && (!strcmp(argv[1],"--peer-read") || !strcmp(argv[1],"--peer-write"))) {
        if (!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,argv[2],-1,name,128)) return 2;
        return peer_action(!strcmp(argv[1],"--peer-write"),name);
    }
    if (argc!=1) return 2;
    CHECK(original && original_desktop && administrator_enabled(&enabled));
    if (!enabled) { unavailable=1;goto done; }
    CHECK(user_station_security(&security,&descriptor,&security_user,&acl));
    CHECK(swprintf(name,128,L"XUI-Document-Clipboard-%lu-%llu",(unsigned long)GetCurrentProcessId(),
        (unsigned long long)GetTickCount64())>0);
    station=CreateWindowStationW(name,CWF_CREATE_ONLY,WINSTA_ALL_ACCESS,&security);CHECK(station);created=1;
    CHECK(SetProcessWindowStation(station));station_changed=1;
    desktop=CreateDesktopW(L"Default",NULL,NULL,0,GENERIC_ALL,&security);CHECK(desktop);
    CHECK(SetThreadDesktop(desktop));desktop_changed=1;
    CHECK(private_station_is(name));verified=1;
    CHECK(xgeInit(&desc)==XGE_OK);initialized=1;native=xuiProxyXge();
    for (int mode=0;mode<4;mode++) for (int operation=0;operation<3;operation++) CHECK(exercise(&native,name,mode,operation));
done:
    if (initialized) xgeUnit();
    if (desktop_changed && !SetThreadDesktop(original_desktop)) { failed=1;restored=0; }
    if (station_changed && !SetProcessWindowStation(original)) { failed=1;restored=0; }
    if (restored) after=GetClipboardSequenceNumber();
    if (desktop && !CloseDesktop(desktop)) failed=1;
    if (station && !CloseWindowStation(station)) failed=1;
    free(acl);free(security_user);
    if (before!=after) failed=1;
    printf("{\"status\":\"%s\",\"administrator_group_enabled\":%s,\"private_station_created\":%s,\"private_station_verified\":%s,"
        "\"clipboard_payload_operation_attempted\":%s,\"interactive_clipboard_payload_accessed\":false,\"interactive_station_restored\":%s,"
        "\"interactive_sequence_before\":%lu,\"interactive_sequence_after\":%lu,\"checks\":%u,\"successful_cases\":%u,\"peer_processes\":%u}\n",
        failed?"failed":unavailable?"isolation_unavailable":"passed",enabled?"true":"false",created?"true":"false",verified?"true":"false",
        payload_attempted?"true":"false",restored?"true":"false",(unsigned long)before,(unsigned long)after,checks,cases,peers);
    return failed?1:unavailable?77:0;
}
