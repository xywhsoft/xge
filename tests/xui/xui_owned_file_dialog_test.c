/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct HostEntry {const char* name;xge_host_xrt_proc address;} HostEntry;
static const HostEntry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}return NULL;}


#include "xui_test_proxy.h"

typedef struct State {size_t refs,holds,drops;int action,calls;bool failDrop; xui_file_dialog dialog;const char* directory;const char* expected;} State;
typedef struct User {unsigned magic;State* state;} User;
static bool ownerHold(const void* raw)
{
    State* s=(State*)raw;assert(s->refs);++s->refs;++s->holds;int action=s->action;s->action=0;
    if(action==1)assert(xuiFileDialogSetOpen(s->dialog,1)==XUI_OK);
    if(action==2)assert(xuiFileDialogSetDirectory(s->dialog,"")==XUI_OK);
    if(action==3)assert(xuiFileDialogDestroyChecked(s->dialog)==XUI_OK);
    return true;
}
static bool ownerDrop(const void* raw){State* s=(State*)raw;assert(s->refs);--s->refs;++s->drops;return !s->failDrop;}
static bool ownerCount(const void* raw,size_t* count){*count=((const State*)raw)->refs;return *count!=0;}
static bool ownerTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={ownerCount,ownerTrace};
static xui_callback_owner_t ownerRef(State* s){return(xui_callback_owner_t){{s,&ops},ownerHold,ownerDrop};}
static void result(xui_file_dialog dialog,int value,const char* path,void* raw)
{
    User* user=raw;assert(user->magic==0xCE55U && user->state->refs>=2 && value==XUI_FILE_DIALOG_RESULT_OK);
    State* s=user->state;++s->calls;assert(!strcmp(path,s->expected));
    assert(xuiFileDialogSetOpen(dialog,1)==XUI_OK && xuiFileDialogSetDirectory(dialog,"")==XUI_OK);
    assert(!strcmp(path,s->expected)); /* Root result/entry strings were freed. */
}
static int context(xui_file_dialog dialog,int index,const char* path,int isDirectory,float x,float y,void* raw)
{
    User* user=raw;assert(user->magic==0xCE55U && user->state->refs>=2 && index>=0 && !isDirectory);(void)x;(void)y;
    State* s=user->state;++s->calls;assert(!strcmp(path,s->expected));
    assert(xuiFileDialogSetDirectory(dialog,s->directory)==XUI_OK && xuiFileDialogSetOpen(dialog,0)==XUI_OK);
    assert(!strcmp(path,s->expected));return XUI_EVENT_DISPATCH_STOP;
}
static void invalidated(void* raw){++*(unsigned*)raw;}
typedef struct Fixture {xui_test_proxy_state_t provider;xui_context context;xui_font font;xui_file_dialog dialog;char* dir;char* path;} Fixture;
static void setup(Fixture* f)
{
    memset(f,0,sizeof(*f));xuiTestProxyInit(&f->provider);
    f->dir=xrtDirTemp(NULL,"xui-fd-owned-","");assert(f->dir && strstr(f->dir,"xui-fd-owned-"));
    f->path=xrtPathJoin(f->dir,"owned.txt");assert(f->path && xrtFileWriteAll(f->path,(xbytesview){(const unsigned char*)"body",4}));
    assert(xuiCreate(&f->context)==XUI_OK && xuiSetProxy(f->context,&f->provider.tProxy)==XUI_OK && xuiInputViewport(f->context,800,520)==XUI_OK);
    assert(f->provider.tProxy.fontLoadMemory(&f->provider.tProxy,&f->font,"body",4,14,XUI_FONT_FORMAT_TTF)==XUI_OK && xuiSetDefaultFont(f->context,f->font)==XUI_OK);
    xui_file_dialog_desc_t d={0};d.iSize=sizeof(d);d.pFont=f->font;d.sInitialDir=f->dir;d.iMode=XUI_FILE_DIALOG_MODE_OPEN_FILE;
    assert(xuiFileDialogCreate(f->context,&f->dialog,&d)==XUI_OK && xuiFileDialogSetOpen(f->dialog,1)==XUI_OK);
}
static void teardown(Fixture* f)
{
    if(f->dialog)assert(xuiFileDialogDestroyChecked(f->dialog)==XUI_OK);
    assert(xuiDestroyChecked(f->context)==XUI_OK);f->provider.tProxy.fontDestroy(&f->provider.tProxy,f->font);
    assert(strstr(f->dir,"xui-fd-owned-") && xrtDirRemoveAll(f->dir));xrtFree(f->path);xrtFree(f->dir);xrtClearError();
}
static int find(Fixture* f)
{for(int i=0;i<xuiFileDialogGetEntryCount(f->dialog);++i)if(!strcmp(xuiFileDialogGetEntryName(f->dialog,i),"owned.txt"))return i;return -1;}
static void result_snapshot(void)
{
    Fixture f;setup(&f);State s={.refs=1,.dialog=f.dialog,.directory=f.dir,.expected=f.path};User u={0xCE55U,&s};xui_callback_owner_t owner=ownerRef(&s);
    assert(xuiFileDialogSetResultOwned(f.dialog,result,&u,&owner)==XUI_OK && s.refs==2);
    int index=find(&f);assert(index>=0 && xuiFileDialogSelectIndex(f.dialog,index)==XUI_OK);s.action=1;
    assert(xuiFileDialogCommit(f.dialog)==XUI_OK && s.calls==1 && s.refs==2 && xuiFileDialogIsOpen(f.dialog));
    assert(xuiFileDialogSetResultOwned(f.dialog,NULL,NULL,NULL)==XUI_OK && s.refs==1);teardown(&f);
}
static void context_snapshot(void)
{
    Fixture f;setup(&f);State s={.refs=1,.dialog=f.dialog,.directory=f.dir,.expected=f.path};User u={0xCE55U,&s};xui_callback_owner_t owner=ownerRef(&s);
    assert(xuiFileDialogSetContextMenuOwned(f.dialog,context,&u,&owner)==XUI_OK);
    int index=find(&f);assert(index>=0 && xuiLayout(f.context)==XUI_OK);xui_widget list=xuiFileDialogGetFileListWidget(f.dialog);
    xui_rect_t world=xuiWidgetGetWorldRect(list),item=xuiListViewGetItemRect(list,index);int x=(int)(world.fX+item.fX+8),y=(int)(world.fY+item.fY+item.fH*0.5f);
    s.action=2;
    assert(xuiInputPointerDown(f.context,x,y,XUI_POINTER_BUTTON_RIGHT,XUI_POINTER_BUTTON_RIGHT)==XUI_OK && xuiDispatchPendingEvents(f.context)==XUI_OK);
    assert(xuiInputPointerUp(f.context,x,y,XUI_POINTER_BUTTON_RIGHT,0)==XUI_OK && xuiDispatchPendingEvents(f.context)==XUI_OK);
    assert(s.calls==1 && s.refs==2 && !xuiFileDialogIsOpen(f.dialog));teardown(&f);assert(s.refs==1);
}
static void failed_notification_copy(void)
{
    Fixture f;setup(&f);State s={.refs=1,.dialog=f.dialog,.directory=f.dir,.expected=f.path};User u={0xCE55U,&s};xui_callback_owner_t owner=ownerRef(&s);
    assert(xuiFileDialogSetResultOwned(f.dialog,result,&u,&owner)==XUI_OK);int index=find(&f);assert(index>=0 && xuiFileDialogSelectIndex(f.dialog,index)==XUI_OK);
    size_t held=s.holds;assert(xrtMemDebugFailAfter(1));int status=xuiFileDialogCommit(f.dialog);assert(xrtMemDebugFailAfter(-1));
    assert(status==XUI_ERROR_OUT_OF_MEMORY && s.calls==0 && s.holds==held && s.refs==2);
    xrtClearError();assert(xuiFileDialogGetResult(f.dialog)==XUI_FILE_DIALOG_RESULT_OK && !xuiFileDialogIsOpen(f.dialog));teardown(&f);assert(s.refs==1);
}
static void lifetime_and_closed_hold(void)
{
    Fixture f;setup(&f);State s={.refs=1,.dialog=f.dialog,.directory=f.dir,.expected=f.path};User u={0xCE55U,&s};xui_callback_owner_t owner=ownerRef(&s);unsigned invalid=0;xui_lifetime_link_t watch={0};
    assert(xuiFileDialogLifetimeWatch(f.dialog,&watch,invalidated,&invalid)==XUI_OK);
    assert(xuiFileDialogSetResultOwned(f.dialog,result,&u,&owner)==XUI_OK);int index=find(&f);assert(index>=0 && xuiFileDialogSelectIndex(f.dialog,index)==XUI_OK);s.action=3;
    assert(xuiFileDialogCommit(f.dialog)==XUI_ERROR_INVALID_STATE && invalid==1 && s.calls==0 && s.refs==1);f.dialog=NULL;xrtClearError();teardown(&f);
    setup(&f);watch=(xui_lifetime_link_t){0};invalid=0;assert(xuiFileDialogLifetimeWatch(f.dialog,&watch,invalidated,&invalid)==XUI_OK);
    xui_widget window=xuiFileDialogGetWindowWidget(f.dialog);assert(window && xuiWidgetDestroyChecked(window)==XUI_OK && invalid==1);f.dialog=NULL;teardown(&f);
    setup(&f);watch=(xui_lifetime_link_t){0};invalid=0;assert(xuiFileDialogLifetimeWatch(f.dialog,&watch,invalidated,&invalid)==XUI_OK);
    f.dialog=NULL;assert(xuiDestroyChecked(f.context)==XUI_OK && invalid==1);f.provider.tProxy.fontDestroy(&f.provider.tProxy,f.font);assert(xrtDirRemoveAll(f.dir));xrtFree(f.path);xrtFree(f.dir);xrtClearError();
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    result_snapshot();context_snapshot();failed_notification_copy();lifetime_and_closed_hold();puts("native FileDialog owned callbacks, path snapshot reentry, OOM and unique-manager lifetime passed");return 0;
}
