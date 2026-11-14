#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "tests/xui/xui_test_proxy.c"
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
static size_t backendReferences=1;
static bool backendHold(const void* p){assert(p==&backendReferences);++backendReferences;return true;}
static bool backendDrop(const void* p){assert(p==&backendReferences&&backendReferences>1);--backendReferences;return true;}
static bool backendCount(const void* p,size_t* n){assert(p==&backendReferences);*n=backendReferences;return true;}
static bool noTrace(const void* p,xrtownershipvisitor visit,void* user){(void)p;(void)visit;(void)user;return true;}
static const xrtownershipops backendOps={backendCount,noTrace};
typedef struct Fixture{xui_context context;xui_proxy proxy;xui_widget root,a,b;xui_test_proxy_state_t backend;} Fixture;
static void create(Fixture* f)
{
 memset(f,0,sizeof(*f));xuiTestProxyInit(&f->backend);
 const xui_callback_owner_t owner={{&backendReferences,&backendOps},backendHold,backendDrop};
 assert(xuiProxyCreateOwned(&f->backend.tProxy,&owner,&f->proxy)==XUI_OK);
 const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};
 assert(xuiCreateOwnedEx(f->proxy,&desc,&f->context)==XUI_OK);
 assert(xuiWidgetCreate(f->context,&f->root)==XUI_OK&&xuiSetRootWidget(f->context,f->root)==XUI_OK);
 assert(xuiWidgetCreate(f->context,&f->a)==XUI_OK&&xuiWidgetCreate(f->context,&f->b)==XUI_OK);
 assert(xuiWidgetAddChild(f->root,f->a)==XUI_OK&&xuiWidgetAddChild(f->root,f->b)==XUI_OK);
}
static void closeFixture(Fixture* f)
{if(f->context)assert(xuiDestroyChecked(f->context)==XUI_OK);assert(xuiProxyReleaseChecked(f->proxy)==XUI_OK);assert(backendReferences==1);}
enum {NONE,REMOVE,REPLACE,RAW,COMMAND,CLEAR_PEER,DESTROY_WIDGET,CLOSE_CONTEXT,RECURSE,CLEAR_FAIL};
typedef struct Owner{
 unsigned references,holds,drops,calls,released;bool inCall,failHold,failDrop,holdDiagnostic,dropDiagnostic,callbackDiagnostic;
 int action,holdAction,dropAction;xui_context context;xui_widget widget;struct Owner* peer;
} Owner;
static xui_callback_owner_t ownerDesc(Owner*);
static int hotkey(xui_widget,const xui_event_t*,void*);
static void observe(xui_context,const xui_error_info_t*,void*);
static bool hold(const void* p)
{
 Owner* o=(Owner*)p;++o->holds;if(o->failHold)return false;assert(o->references&&!o->released);++o->references;
 int action=o->holdAction;o->holdAction=0;
 if(action==REPLACE){xui_callback_owner_t next=ownerDesc(o->peer);assert(xuiHotKeyRegisterOwned(o->context,o->widget,'A',0,hotkey,o->peer,&next)==XUI_OK);}
 if(action==CLOSE_CONTEXT)xuiDestroy(o->context);
 if(o->holdDiagnostic)xrtSetErrorKind(XERR_RANGE);
 return true;
}
static bool drop(const void* p)
{
 Owner* o=(Owner*)p;assert(o->references&&!o->released);++o->drops;
 if(!--o->references){assert(!o->inCall);++o->released;}
 int action=o->references ? NONE:o->dropAction;if(!o->references)o->dropAction=0;
 if(action==REPLACE){xui_callback_owner_t next=ownerDesc(o->peer);assert(xuiHotKeyRegisterOwned(o->context,o->widget,'A',0,hotkey,o->peer,&next)==XUI_OK);}
 if(o->dropDiagnostic)xrtSetErrorKind(XERR_STATE);
 return !o->failDrop;
}
static bool ownerCount(const void* p,size_t* n){const Owner* o=p;*n=o->references;return o->references!=0;}
static const xrtownershipops ownerOps={ownerCount,noTrace};
static xui_callback_owner_t ownerDesc(Owner* o){return (xui_callback_owner_t){{o,&ownerOps},hold,drop};}
static Owner makeOwner(Fixture* f,xui_widget widget){return (Owner){.references=1,.context=f->context,.widget=widget};}
static void registerHot(Owner* o){xui_callback_owner_t d=ownerDesc(o);assert(xuiHotKeyRegisterOwned(o->context,o->widget,'A',0,hotkey,o,&d)==XUI_OK);}
static void registerError(Owner* o){xui_callback_owner_t d=ownerDesc(o);assert(xuiSetErrorCallbackOwned(o->context,observe,o,&d)==XUI_OK);}
static int press(xui_context context){uint32_t flags=0;int status=xuiInputKeyDownEx(context,'A',0,&flags);assert(status!=XUI_OK||flags==4||flags==0);return status;}
static const xui_error_info_t problem={sizeof(problem),XUI_ERROR_INVALID_ARGUMENT,XUI_ERROR_STAGE_INPUT,1,NULL,{0},"test.operation","test message"};
static int hotkey(xui_widget widget,const xui_event_t* event,void* p)
{
 Owner* o=p;assert(widget==o->widget&&event->iType==XUI_EVENT_HOTKEY&&o->references&&!o->released&&!o->inCall);
 o->inCall=true;++o->calls;const int action=o->action;o->action=NONE;
 if(action==REMOVE||action==CLEAR_PEER)assert(xuiHotKeyUnregister(o->context,o->widget,'A',0)==XUI_OK);
 if(action==REPLACE){xui_callback_owner_t next=ownerDesc(o->peer);assert(xuiHotKeyRegisterOwned(o->context,o->widget,'A',0,hotkey,o->peer,&next)==XUI_OK);}
 if(action==RAW)assert(xuiHotKeyRegister(o->context,o->widget,'A',0,NULL,NULL)==XUI_OK);
 if(action==COMMAND)assert(xuiHotKeyRegisterCommand(o->context,o->widget,'A',0,1,"command",NULL)==XUI_OK);
 if(action==CLEAR_PEER){assert(xuiHotKeyClearWidget(o->peer->widget)==XUI_OK);assert(o->peer->references&&!o->peer->released);}
 if(action==DESTROY_WIDGET)xuiWidgetDestroy(widget);
 if(action==CLOSE_CONTEXT)xuiDestroy(o->context);
 assert(o->references&&!o->released);o->inCall=false;
 if(o->callbackDiagnostic)xrtSetErrorKind(XERR_RANGE);
 return 0;
}
static void observe(xui_context context,const xui_error_info_t* event,void* p)
{
 Owner* o=p;assert(context==o->context&&event->iCode==XUI_ERROR_INVALID_ARGUMENT&&o->references&&!o->released&&!o->inCall);
 o->inCall=true;++o->calls;int action=o->action;o->action=NONE;
 if(action==REMOVE||action==CLEAR_FAIL)assert(xuiSetErrorCallbackOwned(context,NULL,NULL,NULL)==XUI_OK);
 if(action==REPLACE){xui_callback_owner_t next=ownerDesc(o->peer);assert(xuiSetErrorCallbackOwned(context,observe,o->peer,&next)==XUI_OK);}
 if(action==RAW)assert(xuiSetErrorCallback(context,NULL,NULL)==XUI_OK);
 if(action==CLOSE_CONTEXT)xuiDestroy(context);
 if(action==RECURSE)assert(xuiReportError(context,&problem)==XUI_OK);
 if(action==CLEAR_FAIL||o->callbackDiagnostic)xrtSetErrorKind(XERR_RANGE);
 assert(o->references&&!o->released);o->inCall=false;
}
static bool findOwner(xrtownershipref ref,void* p){size_t* hits=((void**)p)[1];if(ref.Data==((void**)p)[0])++*hits;return true;}
static void graph(xui_context context,Owner* owner,size_t expected)
{size_t hits=0;void* args[]={owner,&hits};assert(xuiContextTraceOwnership(context,findOwner,args)&&hits==expected);}
static void hotLifetime(void)
{
 for(int action=REMOVE;action<=CLOSE_CONTEXT;++action){
  if(action==CLEAR_PEER)continue;
  Fixture f;create(&f);Owner o=makeOwner(&f,f.a),next=makeOwner(&f,f.a);o.action=action;o.peer=&next;
  registerHot(&o);assert(drop(&o));graph(f.context,&o,1);assert(press(f.context)==XUI_OK);assert(o.calls==1&&!o.references&&o.released==1);
  if(action==CLOSE_CONTEXT)f.context=NULL;
  if(action==REPLACE){assert(drop(&next));graph(f.context,&next,1);assert(press(f.context)==XUI_OK&&next.calls==1);}
  closeFixture(&f);
  if(next.references)assert(drop(&next));
  assert(!next.references&&next.released==1);
 }
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a),b=makeOwner(&f,f.b);a.action=CLEAR_PEER;a.peer=&b;registerHot(&a);registerHot(&b);assert(drop(&a)&&drop(&b));
 assert(press(f.context)==XUI_OK&&a.calls==1&&b.calls==1&&!a.references&&!b.references);closeFixture(&f);
 create(&f);a=makeOwner(&f,f.a);b=makeOwner(&f,f.a);a.dropAction=REPLACE;a.peer=&b;registerHot(&a);assert(drop(&a));
 assert(xuiHotKeyClearWidget(f.a)==XUI_OK&&!a.references);assert(drop(&b));assert(press(f.context)==XUI_OK&&b.calls==1);closeFixture(&f);assert(!b.references);
}
static void errorLifetime(void)
{
 for(int action=REMOVE;action<=RECURSE;++action){
  if(action!=REMOVE&&action!=REPLACE&&action!=RAW&&action!=CLOSE_CONTEXT&&action!=RECURSE)continue;
  Fixture f;create(&f);Owner a=makeOwner(&f,f.a),b=makeOwner(&f,f.a);a.action=action;a.peer=&b;registerError(&a);assert(drop(&a));graph(f.context,&a,1);
  assert(xuiReportError(f.context,&problem)==XUI_OK&&a.calls==1);
  if(action==CLOSE_CONTEXT)f.context=NULL;
  if(action==RECURSE)assert(a.references==1);else assert(!a.references);
  if(action==REPLACE){assert(drop(&b));assert(xuiReportError(f.context,&problem)==XUI_OK&&b.calls==1);}
  closeFixture(&f);if(b.references)assert(drop(&b));assert(!a.references&&!b.references);
 }
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a);registerError(&a);xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();
 assert(xuiReportError(f.context,&problem)==XUI_OK&&xrtGetError()==ambient);xrtClearError();
 assert(xuiSetErrorCallback(f.context,NULL,NULL)==XUI_OK);assert(drop(&a));
 a=makeOwner(&f,f.a);a.action=CLEAR_FAIL;a.failDrop=true;a.dropDiagnostic=true;registerError(&a);assert(drop(&a)==false);a.failDrop=true;
 assert(xuiReportError(f.context,&problem)==XUI_ERROR_CALLBACK_FAILED&&!a.references&&xrtGetError());xrtClearError();closeFixture(&f);
}
static void failures(void)
{
 unsigned failed=0,success=0;
 for(int budget=0;budget<12;++budget){
  Fixture f;create(&f);Owner old=makeOwner(&f,f.a),candidate=makeOwner(&f,f.a);registerHot(&old);
  xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(budget));
  xui_callback_owner_t d=ownerDesc(&candidate);int status=xuiHotKeyRegisterOwned(f.context,f.a,'A',0,hotkey,&candidate,&d);xrtMemDebugFailClear();xrtClearError();
  if(status!=XUI_OK){++failed;assert(status==XUI_ERROR_OUT_OF_MEMORY&&candidate.references==1&&old.references==2);graph(f.context,&old,1);xrtMemDebugSnapshot(&after);assert(after.LiveCount==before.LiveCount&&after.LiveBytes==before.LiveBytes);}
  else{++success;assert(old.references==1&&candidate.references==2);}
  closeFixture(&f);assert(drop(&old)&&drop(&candidate));
 }
 assert(failed&&success);
 for(int mode=0;mode<3;++mode){
  Fixture f;create(&f);Owner a=makeOwner(&f,f.a);a.failHold=mode==0;a.holdDiagnostic=mode==1;a.holdAction=mode==2?CLOSE_CONTEXT:0;
  xui_callback_owner_t d=ownerDesc(&a);assert(xuiHotKeyRegisterOwned(f.context,f.a,'A',0,hotkey,&a,&d)!=XUI_OK);xrtClearError();assert(a.references==1);
  if(mode==2)f.context=NULL;
  closeFixture(&f);a.failHold=false;a.holdDiagnostic=false;assert(drop(&a));
 }
}
static void batchAndLimits(void)
{
 Fixture f;create(&f);Owner owners[17];
 for(int i=0;i<17;++i){xui_widget widget=NULL;assert(xuiWidgetCreate(f.context,&widget)==XUI_OK);owners[i]=makeOwner(&f,widget);registerHot(&owners[i]);assert(drop(&owners[i]));}
 assert(xrtMemDebugFailAfter(0));assert(press(f.context)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();
 for(int i=0;i<17;++i)assert(!owners[i].calls&&owners[i].references==1);
 /* The real allocated record, with test-only overflow injection. No fake
  * Context or large allocation substitutes for native execution. */
 xui_callback_record_t* record=f.context->pHotkeys[8].pOwned;size_t refs=record->iReferences;record->iReferences=SIZE_MAX;
 assert(press(f.context)==XUI_ERROR_LIMIT_EXCEEDED);record->iReferences=refs;xrtClearError();
 for(int i=0;i<17;++i)assert(!owners[i].calls&&owners[i].references==1);
 assert(press(f.context)==XUI_OK);
 for(int i=0;i<17;++i)assert(owners[i].calls==1);
 closeFixture(&f);for(int i=0;i<17;++i)assert(!owners[i].references);
 create(&f);
 for(int i=0;i<17;++i){owners[i]=makeOwner(&f,f.a);xui_callback_owner_t d=ownerDesc(&owners[i]);assert(xuiHotKeyRegisterOwned(f.context,f.a,'A'+i,0,hotkey,&owners[i],&d)==XUI_OK);assert(drop(&owners[i]));}
 assert(xrtMemDebugFailAfter(0));assert(xuiHotKeyClearWidget(f.a)==XUI_OK);xrtMemDebugFailClear();
 for(int i=0;i<17;++i)assert(!owners[i].references);
 Owner observer=makeOwner(&f,f.a);registerError(&observer);assert(drop(&observer));record=f.context->pErrorOwner;refs=record->iReferences;record->iReferences=SIZE_MAX;
 assert(xuiReportError(f.context,&problem)==XUI_ERROR_LIMIT_EXCEEDED&&!observer.calls);record->iReferences=refs;xrtClearError();
 closeFixture(&f);assert(!observer.references);
}
static unsigned deliveredAfterClose;
static int queuedCallback(xui_widget widget,const xui_event_t* event,void* user)
{(void)widget;(void)event;(void)user;++deliveredAfterClose;return XUI_EVENT_DISPATCH_STOP;}
static void closeErrorPump(void)
{
 Fixture f;create(&f);Owner observer=makeOwner(&f,f.a);observer.action=CLOSE_CONTEXT;registerError(&observer);assert(drop(&observer));
 assert(xuiWidgetSetFocusable(f.a,1)==XUI_OK&&xuiWidgetSetImeMode(f.a,XUI_IME_ENABLED)==XUI_OK&&xuiSetFocusWidget(f.context,f.a)==XUI_OK);
 assert(xuiWidgetSetEventHandler(f.b,XUI_EVENT_COMMAND,queuedCallback,NULL)==XUI_OK);
 xui_ime_composition_t composition={sizeof(composition),"a\0b",3,0,0,0,0,0,0,0,0};
 assert(xuiInputImeCompositionEx(f.context,&composition)==XUI_OK&&xuiCommandDispatch(f.context,f.b,1,"later",NULL)==XUI_OK);
 assert(xuiDispatchPendingEvents(f.context)==XUI_OK&&!deliveredAfterClose&&observer.calls==1&&!observer.references);
 f.context=NULL;closeFixture(&f);
}
static void allocation(void)
{
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a);registerHot(&a);registerError(&a);
 unsigned holds=a.holds;xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 for(int i=0;i<1000;++i){assert(press(f.context)==XUI_OK);assert(xuiReportError(f.context,&problem)==XUI_OK);}
 xrtMemDebugSnapshot(&after);assert(a.holds==holds&&after.AllocCount==before.AllocCount&&after.FreeCount==before.FreeCount&&after.ReallocCount==before.ReallocCount);
 printf("InputOwners native hot: alloc=%llu free=%llu realloc=%llu holds=%u\n",(unsigned long long)(after.AllocCount-before.AllocCount),(unsigned long long)(after.FreeCount-before.FreeCount),(unsigned long long)(after.ReallocCount-before.ReallocCount),a.holds-holds);
 closeFixture(&f);assert(drop(&a));
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));hotLifetime();errorLifetime();failures();batchAndLimits();closeErrorPump();allocation();
 xmemdebugsnapshot end;xrtMemDebugSnapshot(&end);assert(!end.LiveCount&&!end.InvalidFreeCount&&!end.DoubleFreeCount);
 puts("InputOwners: native ownership, snapshots, reentry, diagnostics, OOM and allocation passed");return 0;
}
