#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
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
static unsigned errors,delivered;
static void observer(xui_context context,const xui_error_info_t* error,void* user)
{(void)user;assert(error->iCode==XUI_ERROR_INVALID_ARGUMENT);++errors;xuiDestroy(context);}
static int command(xui_widget widget,const xui_event_t* event,void* user)
{(void)widget;(void)event;(void)user;++delivered;return XUI_EVENT_DISPATCH_STOP;}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));Fixture f;create(&f);
 assert(xuiSetErrorCallback(f.context,observer,NULL)==XUI_OK);
 assert(xuiWidgetSetFocusable(f.a,1)==XUI_OK&&xuiWidgetSetImeMode(f.a,XUI_IME_ENABLED)==XUI_OK&&xuiSetFocusWidget(f.context,f.a)==XUI_OK);
 assert(xuiWidgetSetEventHandler(f.b,XUI_EVENT_COMMAND,command,NULL)==XUI_OK);
 xui_ime_composition_t composition={sizeof(composition),"a\0b",3,0,0,0,0,0,0,0,0};
 assert(xuiInputImeCompositionEx(f.context,&composition)==XUI_OK&&xuiCommandDispatch(f.context,f.b,1,"later",NULL)==XUI_OK);
 int status=xuiDispatchPendingEvents(f.context);f.context=NULL;closeFixture(&f);
 printf("Error-close pump: status=%d queued=%u errors=%u\n",status,delivered,errors);
 xmemdebugsnapshot end;xrtMemDebugSnapshot(&end);assert(!end.LiveCount&&!end.InvalidFreeCount&&!end.DoubleFreeCount);
 if(status!=XUI_OK||delivered||errors!=1)return 76;
 puts("Error-close pump stopped queued dispatch");return 0;
}
