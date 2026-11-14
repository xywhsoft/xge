/* Reuse the CPU backend and owned Context fixture, not an observer adapter. */
#define main previousOwnedInputMain
#include "xui_owned_input_callbacks_test.c"
#undef main

static void accessible(xui_context,xui_widget,const xui_accessibility_event_t*,void*);
static xui_callback_owner_t accessOwner(Owner*);
static void registerAccess(Owner* o)
{xui_callback_owner_t d=accessOwner(o);assert(xuiSetAccessibilityEventCallbackOwned(o->context,accessible,o,&d)==XUI_OK);}
static bool accessHold(const void* raw)
{
 Owner* o=(Owner*)raw;++o->holds;if(o->failHold)return false;assert(o->references&&!o->released);++o->references;
 int action=o->holdAction;o->holdAction=NONE;
 if(action==REPLACE)registerAccess(o->peer);
 if(action==CLOSE_CONTEXT)xuiDestroy(o->context);
 if(o->holdDiagnostic)xrtSetErrorKind(XERR_RANGE);
 return true;
}
static bool accessDrop(const void* raw)
{
 Owner* o=(Owner*)raw;assert(o->references&&!o->released);++o->drops;
 if(!--o->references){assert(!o->inCall);++o->released;int action=o->dropAction;o->dropAction=NONE;if(action==REPLACE)registerAccess(o->peer);}
 if(o->dropDiagnostic)xrtSetErrorKind(XERR_STATE);
 return !o->failDrop;
}
static xui_callback_owner_t accessOwner(Owner* o)
{return (xui_callback_owner_t){{o,&ownerOps},accessHold,accessDrop};}
static void accessible(xui_context context,xui_widget widget,const xui_accessibility_event_t* event,void* raw)
{
 Owner* o=raw;assert(context==o->context&&widget==o->widget&&o->references&&!o->released&&!o->inCall);
 assert(event&&event->iSize==sizeof(*event)&&event->iType==XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
 assert(event->iNodeId==UINT64_C(4294967297)&&event->iRevision!=0);
 o->inCall=true;++o->calls;int action=o->action;o->action=NONE;
 if(action==REMOVE)assert(xuiSetAccessibilityEventCallbackOwned(context,NULL,NULL,NULL)==XUI_OK);
 if(action==REPLACE)registerAccess(o->peer);
 if(action==RAW)assert(xuiSetAccessibilityEventCallback(context,NULL,NULL)==XUI_OK);
 if(action==DESTROY_WIDGET)xuiWidgetDestroy(widget);
 if(action==CLOSE_CONTEXT)xuiDestroy(context);
 if(action==RECURSE)assert(xuiWidgetNotifyAccessibility(widget,event->iType,event->iNodeId)==XUI_OK);
 assert(o->references&&!o->released);o->inCall=false;
 if(o->callbackDiagnostic)xrtSetErrorKind(XERR_RANGE);
}
static int notify(xui_widget widget)
{return xuiWidgetNotifyAccessibility(widget,XUI_ACCESSIBLE_EVENT_STATE_CHANGED,UINT64_C(4294967297));}
static void accessLifetime(void)
{
 const int actions[]={REMOVE,REPLACE,RAW,DESTROY_WIDGET,CLOSE_CONTEXT,RECURSE};
 for(size_t i=0;i<sizeof(actions)/sizeof(actions[0]);++i){
  Fixture f;create(&f);Owner o=makeOwner(&f,f.a),next=makeOwner(&f,f.a);o.action=actions[i];o.peer=&next;
  registerAccess(&o);assert(accessDrop(&o));graph(f.context,&o,1);assert(notify(f.a)==XUI_OK&&o.calls==1);
  if(actions[i]==CLOSE_CONTEXT)f.context=NULL;
  if(actions[i]==REPLACE){assert(accessDrop(&next));graph(f.context,&next,1);assert(notify(f.a)==XUI_OK&&next.calls==1);}
  closeFixture(&f);assert(!o.references&&o.released==1);
  if(next.references){assert(accessDrop(&next));}
  assert(!next.references&&next.released==1);
 }
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a),b=makeOwner(&f,f.a);a.dropAction=REPLACE;a.peer=&b;
 registerAccess(&a);assert(accessDrop(&a));assert(xuiSetAccessibilityEventCallback(f.context,NULL,NULL)==XUI_OK&&!a.references);
 assert(accessDrop(&b));graph(f.context,&b,1);assert(notify(f.a)==XUI_OK&&b.calls==1);closeFixture(&f);assert(!b.references);
}
static void accessFailure(void)
{
 for(int budget=0;budget<8;++budget){
  Fixture f;create(&f);Owner a=makeOwner(&f,f.a),b=makeOwner(&f,f.a);registerAccess(&a);
  xui_callback_owner_t d=accessOwner(&b);assert(xrtMemDebugFailAfter((uint64_t)budget));
  int status=xuiSetAccessibilityEventCallbackOwned(f.context,accessible,&b,&d);xrtMemDebugFailClear();xrtErrorFree(xrtTakeError());
  assert(notify(f.a)==XUI_OK);assert((status==XUI_OK&&b.calls==1&&!a.calls)||(status!=XUI_OK&&a.calls==1&&!b.calls));
  closeFixture(&f);assert(accessDrop(&a)&&accessDrop(&b));assert(!a.references&&!b.references);
 }
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a),b=makeOwner(&f,f.a);registerAccess(&a);b.failHold=true;xui_callback_owner_t d=accessOwner(&b);
 assert(xuiSetAccessibilityEventCallbackOwned(f.context,accessible,&b,&d)!=XUI_OK);xrtErrorFree(xrtTakeError());assert(notify(f.a)==XUI_OK&&a.calls==1);
 a.callbackDiagnostic=true;assert(notify(f.a)==XUI_ERROR_CALLBACK_FAILED);xrtErrorFree(xrtTakeError());a.callbackDiagnostic=false;
 assert(accessDrop(&a));a.action=REMOVE;a.failDrop=true;
 assert(notify(f.a)==XUI_ERROR_CALLBACK_FAILED&&!a.references&&a.released==1);xrtErrorFree(xrtTakeError());
 closeFixture(&f);assert(accessDrop(&b));
}
static void accessOverflowAndHot(void)
{
 Fixture f;create(&f);Owner a=makeOwner(&f,f.a);registerAccess(&a);assert(accessDrop(&a));
 xui_callback_record_t* real=f.context->pAccessibilityOwner;assert(real&&real->iReferences==1);real->iReferences=SIZE_MAX;
 assert(notify(f.a)==XUI_ERROR_LIMIT_EXCEEDED&&!a.calls);xrtErrorFree(xrtTakeError());real->iReferences=1;
 assert(notify(f.a)==XUI_OK);xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
 for(unsigned i=0;i<1000;++i)assert(notify(f.a)==XUI_OK);
 xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
 assert(after.AllocCount==before.AllocCount&&after.FreeCount==before.FreeCount&&after.ReallocCount==before.ReallocCount);
 assert(a.calls==1001&&a.holds==1);closeFixture(&f);assert(!a.references);
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnabled()||xrtMemDebugEnable(true));
 accessLifetime();accessFailure();accessOverflowAndHot();
 xmemdebugsnapshot final;xrtMemDebugSnapshot(&final);assert(!final.LiveCount&&!final.InvalidFreeCount&&!final.DoubleFreeCount);
 puts("Accessibility owner: snapshots, replacement/close, graph, OOM, failures, overflow and 1000 zero-heap notifications passed");return 0;
}
