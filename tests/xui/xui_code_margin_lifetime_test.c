/* Unique model lifetime, actual CodeEdit ownership and deferred Context tail.
 * Callback publication is deliberately not advertised by this foundation. */
#include "xui_code_margin_host.h"
#include "tests/xui/xui_test_proxy.c"
#include <stdio.h>
typedef struct Watch {xui_code_margin_model model;unsigned calls;bool fail;} Watch;
static void invalidated(void* raw)
{
 Watch* event=raw;++event->calls;bool consumed=true;float width=91;
 assert(xuiCodeMarginModelDestroyOwnerChecked(event->model,&consumed)==XUI_ERROR_BUSY&&!consumed);
 assert(xuiCodeMarginModelClearChecked(event->model)==XUI_ERROR_INVALID_STATE);
 assert(xuiCodeMarginModelGetTotalWidth(event->model,&width)==XUI_ERROR_INVALID_STATE&&width==91);
 xui_code_margin_scope_t scope={0};assert(xuiCodeMarginModelCallBegin(event->model,&scope)==XUI_ERROR_INVALID_STATE);
 if(event->fail)xrtSetErrorKind(XERR_STATE);
}
static bool visit(xrtownershipref edge,void* user){(void)edge;(void)user;return true;}
static void equal(const xmemdebugsnapshot* before)
{
 xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
 assert(after.LiveCount==before->LiveCount&&after.LiveBytes==before->LiveBytes&&after.InvalidFreeCount==before->InvalidFreeCount&&after.DoubleFreeCount==before->DoubleFreeCount);
}
static void standalone(void)
{
 xui_code_margin_model model=NULL;assert(xuiCodeMarginModelCreate(&model)==XUI_OK);
 assert(xuiCodeMarginModelLoadDefaults(model,1,1,1,1)==XUI_OK);
 Watch event={model,0,false};xui_lifetime_link_t link={0};
 assert(xuiCodeMarginModelLifetimeWatch(model,&link,invalidated,&event)==XUI_OK);
 assert(xuiCodeMarginModelLifetimeWatch(model,&link,invalidated,&event)==XUI_ERROR_INVALID_STATE);
 xui_code_margin_scope_t outer={0},inner={0};
 assert(xuiCodeMarginModelCallBegin(model,&outer)==XUI_OK&&xuiCodeMarginModelCallBegin(model,&inner)==XUI_OK);
 assert(xuiCodeMarginModelCallBegin(model,&outer)==XUI_ERROR_INVALID_ARGUMENT);
 xui_code_margin_scope_t moved=inner;assert(xuiCodeMarginModelCallEnd(&moved)==XUI_ERROR_INVALID_ARGUMENT);
 assert(xuiCodeMarginModelOwnershipBusy(model)&&!xuiCodeMarginModelTraceOwnership(model,visit,NULL));
 bool consumed=false;assert(xuiCodeMarginModelDestroyOwnerChecked(model,&consumed)==XUI_OK&&consumed&&!event.calls);
 consumed=true;assert(xuiCodeMarginModelDestroyOwnerChecked(model,&consumed)==XUI_ERROR_INVALID_STATE&&!consumed);
 assert(xuiCodeMarginModelGetCount(model)==0&&xuiCodeMarginModelClearChecked(model)==XUI_ERROR_INVALID_STATE);
 assert(xuiCodeMarginModelLoadDefaults(model,1,1,1,1)==XUI_ERROR_INVALID_STATE);
 assert(xuiCodeMarginModelSetWidth(model,1,2)==XUI_ERROR_INVALID_STATE);
 assert(xuiCodeMarginModelCallEnd(&inner)==XUI_ERROR_INVALID_STATE&&!event.calls);
 assert(xuiCodeMarginModelCallEnd(&outer)==XUI_ERROR_INVALID_STATE&&event.calls==1&&!link.ppPrevious);
 assert(xuiCodeMarginModelCallEnd(&outer)==XUI_ERROR_INVALID_ARGUMENT);
 xrtClearError();
 assert(xuiCodeMarginModelCreate(&model)==XUI_OK);event=(Watch){model,0,true};link=(xui_lifetime_link_t){0};
 assert(xuiCodeMarginModelLifetimeWatch(model,&link,invalidated,&event)==XUI_OK);
 consumed=false;assert(xuiCodeMarginModelDestroyOwnerChecked(model,&consumed)==XUI_ERROR_CALLBACK_FAILED&&consumed&&event.calls==1);
 xrtClearError();
}
typedef struct Worker {xui_code_margin_model model;xui_code_margin_scope_t* active;} Worker;
static int32 otherThread(void* raw)
{
 Worker* event=raw;bool consumed=true;float width=91;xui_code_margin_scope_t scope={0};
 assert(xuiCodeMarginModelCallBegin(event->model,&scope)==XUI_ERROR_BUSY&&!scope.pSelf);
 assert(xuiCodeMarginModelCallEnd(event->active)==XUI_ERROR_BUSY&&event->active->pSelf==event->active);
 assert(xuiCodeMarginModelClearChecked(event->model)==XUI_ERROR_BUSY);
 assert(xuiCodeMarginModelSetWidth(event->model,1,2)==XUI_ERROR_BUSY);
 assert(xuiCodeMarginModelGetTotalWidth(event->model,&width)==XUI_ERROR_BUSY&&width==91);
 assert(xuiCodeMarginModelGetCount(event->model)==0&&xuiCodeMarginModelOwnershipBusy(event->model));
 assert(!xuiCodeMarginModelTraceOwnership(event->model,visit,NULL));
 assert(xuiCodeMarginModelDestroyOwnerChecked(event->model,&consumed)==XUI_ERROR_BUSY&&!consumed);return 0;
}
static void threads(void)
{
 xui_code_margin_model model=NULL;assert(xuiCodeMarginModelCreate(&model)==XUI_OK);
 assert(xuiCodeMarginModelLoadDefaults(model,1,1,1,1)==XUI_OK);
 xui_code_margin_scope_t scope={0};assert(xuiCodeMarginModelCallBegin(model,&scope)==XUI_OK);
 Worker event={model,&scope};xthread* worker=xrtThreadCreate(otherThread,&event,0);
 assert(worker&&xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
 assert(xuiCodeMarginModelCallEnd(&scope)==XUI_OK&&!xuiCodeMarginModelOwnershipBusy(model));
 xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));
 assert(xuiCodeMarginModelTraceOwnership(model,visit,NULL));
 // XRT permits balanced same-thread mutations during a frozen inspection.
 // No arbitrary callback or cross-thread wait runs while the freeze is held.
 xui_code_margin_scope_t nested={0};
 assert(xuiCodeMarginModelCallBegin(model,&nested)==XUI_OK&&xuiCodeMarginModelOwnershipBusy(model));
 assert(xuiCodeMarginModelCallEnd(&nested)==XUI_OK);
 assert(xrtOwnershipScopeEnd(&frozen));xrtClearError();
 assert(xuiCodeMarginModelDestroyChecked(model)==XUI_OK);
}
static size_t references=1;
static bool hold(const void* raw){assert(raw==&references);++references;return true;}
static bool drop(const void* raw){assert(raw==&references&&references>1);--references;return true;}
static bool count(const void* raw,size_t* n){assert(raw==&references);*n=references;return true;}
static bool trace(const void* raw,xrtownershipvisitor fn,void* user){(void)raw;(void)fn;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_test_proxy_state_t backend;
static void embedded(int closeContext)
{
 xui_callback_owner_t owner={{&references,&ops},hold,drop};xui_proxy proxy=NULL;xui_context context=NULL;
 const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};xui_widget editor=NULL;
 assert(xuiProxyCreateOwned(&backend.tProxy,&owner,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
 assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);xui_code_margin_model model=xuiCodeEditGetMargins(editor);assert(model);
 bool consumed=true;assert(xuiCodeMarginModelDestroyOwnerChecked(model,&consumed)==XUI_ERROR_INVALID_STATE&&!consumed);
 Watch event={model,0,false};xui_lifetime_link_t link={0};
 assert(xuiCodeMarginModelLifetimeWatch(model,&link,invalidated,&event)==XUI_OK);
 xui_code_margin_scope_t scope={0};assert(xuiCodeMarginModelCallBegin(model,&scope)==XUI_OK);
 assert(xuiContextOwnershipBusy(context));
 int status=closeContext?xuiDestroyChecked(context):xuiWidgetDestroyChecked(editor);
 assert(status==(closeContext?XUI_STATUS_DEFERRED:XUI_OK)&&!event.calls);
 status=xuiCodeMarginModelCallEnd(&scope);
 if(closeContext){
  // Deferred Context close has not consumed its owner. Retry after quiescence.
  assert(status==XUI_OK&&!event.calls);
  assert(xuiDestroyChecked(context)==XUI_OK&&event.calls==1);
 }else{
  assert(status==XUI_ERROR_INVALID_STATE&&event.calls==1);
  xrtClearError();assert(xuiDestroyChecked(context)==XUI_OK);
 }
 assert(xuiProxyReleaseChecked(proxy)==XUI_OK&&references==1);
}
static void hot(void)
{
 xui_code_margin_model model=NULL;assert(xuiCodeMarginModelCreate(&model)==XUI_OK&&xuiCodeMarginModelLoadDefaults(model,1,1,1,1)==XUI_OK);
 xui_code_margin_scope_t warm={0};assert(xuiCodeMarginModelCallBegin(model,&warm)==XUI_OK&&xuiCodeMarginModelCallEnd(&warm)==XUI_OK);
 xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
 for(int i=0;i<1000;++i){xui_code_margin_scope_t scope={0};float width=0;
  assert(xuiCodeMarginModelCallBegin(model,&scope)==XUI_OK&&xuiCodeMarginModelGetTotalWidth(model,&width)==XUI_OK&&width>0);
  assert(xuiCodeMarginModelCallEnd(&scope)==XUI_OK);
 }
 assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
 assert(before.AllocCount==after.AllocCount&&before.ReallocCount==after.ReallocCount&&before.FreeCount==after.FreeCount);
 assert(xuiCodeMarginModelDestroyChecked(model)==XUI_OK);
 printf("Margin model hot:1000 scopes,0 heap; public scope bytes=%zu\n",sizeof(xui_code_margin_scope_t));
}
int main(void)
{
 marginBind();xuiTestProxyInit(&backend);threads();xrtClearError();
 xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);standalone();equal(&before);threads();equal(&before);
 embedded(0);equal(&before);embedded(1);equal(&before);hot();equal(&before);
 puts("Margin lifetime: unique owner, watches, consumed failure, nested scopes, thread/freeze, CodeEdit and Context tails passed");return 0;
}
