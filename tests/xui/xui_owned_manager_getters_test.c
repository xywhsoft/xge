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
#include "src/xui/core/xui_internal.h"


/* This is a real Context-owned pending peer with an actual retained callback
 * edge. Its semantic Drop destroys the receiver. A raw borrowed-return getter
 * must not unexpectedly execute that unrelated retirement tail. */
typedef struct Peer {
    xui_owned_manager_t node;
    size_t refs,drops;
    bool ready;
    int kind;
    xui_msgbox box;
    xui_toast toast;
    xui_msgtip tip;
    xui_file_dialog dialog;
} Peer;
static bool hold(const void* raw){Peer* p=(Peer*)raw;assert(p->refs);++p->refs;return true;}
static bool drop(const void* raw)
{
    Peer* p=(Peer*)raw;assert(p->refs==2);--p->refs;++p->drops;
    int status=p->kind==0 ? xuiMsgBoxDestroyChecked(p->box):p->kind==1 ? xuiToastDestroyChecked(p->toast):p->kind==2 ? xuiMsgTipDestroyChecked(p->tip):xuiFileDialogDestroyChecked(p->dialog);
    assert(status==XUI_OK);return true;
}
static bool count(const void* raw,size_t* value){*value=((const Peer*)raw)->refs;return true;}
static bool ownerTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops peerOwnerOps={count,ownerTrace};
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){return visit((xrtownershipref){raw,&peerOwnerOps},user);}
static int ready(const void* raw){return ((const Peer*)raw)->ready ? XUI_OK:XUI_ERROR_BUSY;}
static int prepare(void* raw,bool* complete){*complete=((Peer*)raw)->ready;return *complete ? XUI_OK:XUI_STATUS_DEFERRED;}
static void closePeer(void* raw){(void)raw;}
static int finalize(void* raw){return drop(raw) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;}
static const xui_owned_manager_ops_t peerOps={trace,ready,prepare,closePeer,finalize};
static void invalidated(void* raw){++*(unsigned*)raw;}
static int wrongThread(void* raw)
{
    Peer* p=raw;xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();
    if(p->kind==0){xui_msgbox_metrics_t metrics={0};assert(!xuiMsgBoxGetTitle(p->box) && !xuiMsgBoxGetContext(p->box) && xuiMsgBoxGetMetrics(p->box,&metrics)==XUI_ERROR_BUSY);}
    if(p->kind==1){xui_toast_metrics_t metrics={0};assert(!xuiToastGetFont(p->toast) && !xuiToastGetContext(p->toast) && xuiToastGetMetrics(p->toast,&metrics)==XUI_ERROR_BUSY);}
    if(p->kind==2){xui_msgtip_metrics_t metrics={0};assert(!xuiMsgTipGetText(p->tip) && !xuiMsgTipGetContext(p->tip) && xuiMsgTipGetMetrics(p->tip,&metrics)==XUI_ERROR_BUSY);}
    if(p->kind==3)assert(!xuiFileDialogGetDirectory(p->dialog) && !xuiFileDialogGetContext(p->dialog));
    assert(xrtGetError()==ambient && !p->drops);xrtClearError();return 0;
}
static void borrowedGetterDoesNotDrain(int kind)
{
    xui_test_proxy_state_t provider;xuiTestProxyInit(&provider);xui_context context=NULL;xui_font font=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&provider.tProxy)==XUI_OK && xuiInputViewport(context,800,520)==XUI_OK);
    assert(provider.tProxy.fontLoadMemory(&provider.tProxy,&font,"font",4,14,XUI_FONT_FORMAT_TTF)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    Peer peer={.refs=1,.kind=kind};xui_lifetime_link_t watch={0};unsigned invalid=0;char* directory=NULL;
    if(kind==0){xui_msgbox_desc_t d={0};d.iSize=sizeof(d);d.sTitle="borrowed title";d.sMessage="borrowed message";d.pFont=font;assert(xuiMsgBoxCreate(context,&peer.box,&d)==XUI_OK && xuiMsgBoxLifetimeWatch(peer.box,&watch,invalidated,&invalid)==XUI_OK);}
    if(kind==1){xui_toast_desc_t d={0};d.iSize=sizeof(d);d.pFont=font;assert(xuiToastCreate(context,&peer.toast,&d)==XUI_OK && xuiToastLifetimeWatch(peer.toast,&watch,invalidated,&invalid)==XUI_OK);}
    if(kind==2){xui_msgtip_desc_t d={0};d.iSize=sizeof(d);d.sText="borrowed tip";d.pFont=font;assert(xuiMsgTipCreate(context,&peer.tip,&d)==XUI_OK && xuiMsgTipLifetimeWatch(peer.tip,&watch,invalidated,&invalid)==XUI_OK);}
    if(kind==3){directory=xrtDirTemp(NULL,"xui-getter-","");assert(directory);xui_file_dialog_desc_t d={0};d.iSize=sizeof(d);d.pFont=font;d.sInitialDir=directory;assert(xuiFileDialogCreate(context,&peer.dialog,&d)==XUI_OK && xuiFileDialogLifetimeWatch(peer.dialog,&watch,invalidated,&invalid)==XUI_OK);}
    xui_call_scope_t operation={0};assert(xuiCallScopeBegin(context,&operation)==XUI_OK);
    assert(hold(&peer) && xuiInternalManagerRegister(context,&peer.node,&peerOps,&peer)==XUI_OK && xuiInternalManagerRequestClose(&peer.node)==XUI_OK);
    assert(xuiCallScopeEnd(&operation)==XUI_OK && !peer.drops && !invalid);
    peer.ready=true; /* The actual peer owner is now eligible for retirement. */
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();const char* borrowed=NULL;
    if(kind==0){borrowed=xuiMsgBoxGetTitle(peer.box);assert(!peer.drops && !invalid);assert(borrowed && !strcmp(borrowed,"borrowed title") && !strcmp(xuiMsgBoxGetMessage(peer.box),"borrowed message") && xuiMsgBoxGetContext(peer.box)==context);}
    if(kind==1)assert(xuiToastGetFont(peer.toast)==font && xuiToastGetContext(peer.toast)==context && xuiToastGetActiveCount(peer.toast)==0);
    if(kind==2){borrowed=xuiMsgTipGetText(peer.tip);assert(!peer.drops && !invalid);assert(borrowed && !strcmp(borrowed,"borrowed tip") && xuiMsgTipGetContext(peer.tip)==context);}
    if(kind==3){borrowed=xuiFileDialogGetDirectory(peer.dialog);assert(!peer.drops && !invalid);assert(borrowed && !strcmp(borrowed,directory) && xuiFileDialogGetContext(peer.dialog)==context && xuiFileDialogGetEntryCount(peer.dialog)==0);}
    assert(!peer.drops && !invalid && xrtGetError()==ambient);xrtClearError();
    xthread* worker=xrtThreadCreate(wrongThread,&peer,0);assert(worker && xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(!peer.drops && !invalid); /* No query executed user Drop. */
    operation=(xui_call_scope_t){0};assert(xuiCallScopeBegin(context,&operation)==XUI_OK && xuiCallScopeEnd(&operation)==XUI_OK);
    assert(peer.drops==1 && peer.refs==1 && invalid==1 && !peer.node.registered);
    assert(xuiDestroyChecked(context)==XUI_OK);provider.tProxy.fontDestroy(&provider.tProxy,font);
    if(directory){assert(xrtDirRemoveAll(directory));xrtFree(directory);}xrtClearError();
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    for(int kind=0;kind<4;++kind)borrowedGetterDoesNotDrain(kind);
    puts("four pure manager getter families: no pending-owner Drop, stable borrowed result, ambient preservation and creator-first rejection passed");return 0;
}
