/* The constructor chain destroys only successful init stages. Radio must
 * clean its own partial text when init fails. Exercise real constructors. */
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
static bool hold(const void* raw){assert(raw==&backendReferences);++backendReferences;return true;}
static bool drop(const void* raw){assert(raw==&backendReferences&&backendReferences>1);--backendReferences;return true;}
static bool count(const void* raw,size_t* n){assert(raw==&backendReferences);*n=backendReferences;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ownerOps={count,trace};
static void same_heap(const xmemdebugsnapshot* before)
{
 xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
 assert(after.LiveCount==before->LiveCount&&after.LiveBytes==before->LiveBytes&&after.InvalidFreeCount==before->InvalidFreeCount&&after.DoubleFreeCount==before->DoubleFreeCount);
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
 assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));xmemdebugsnapshot initial;xrtMemDebugSnapshot(&initial);
 static xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
 const xui_callback_owner_t owner={{&backendReferences,&ownerOps},hold,drop};
 xui_proxy proxy=NULL;xui_context ui=NULL;xui_widget group=NULL,warm=NULL;
 const xui_context_desc_t options={sizeof(options),XUI_SYSTEM_FONT_DISABLED,0,0};
 assert(xuiProxyCreateOwned(&backend.tProxy,&owner,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&options,&ui)==XUI_OK);
 assert(xuiRadioGroupCreate(ui,&group,NULL)==XUI_OK);
 const xui_radio_desc_t desc={.iSize=sizeof(desc),.sText="partial radio text",.bChecked=1};
 assert(xuiRadioGroupAddOption(group,&warm,&desc)==XUI_OK&&xuiWidgetDestroyChecked(warm)==XUI_OK);
 for(int kind=0;kind<3;++kind){unsigned failures=0,successes=0;
  for(int allowance=0;allowance<64;++allowance){xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
   xui_widget output=group;assert(xrtMemDebugFailAfter(allowance));
   int status=kind==0?xuiRadioCreate(ui,&output,&desc):kind==1?xuiRadioGroupAddOption(group,&output,&desc):xuiRadioCreate(ui,&output,NULL);
   xrtMemDebugFailClear();
   if(status==XUI_OK){++successes;assert(output);assert(xuiWidgetDestroyChecked(output)==XUI_OK);}else{++failures;assert(output==NULL);}
   assert(xuiWidgetGetChildCount(group)==0&&xuiRadioGroupGetSelectedIndex(group)==-1);xrtClearError();same_heap(&before);
  }
  assert(failures&&successes);
 }
 assert(xuiDestroyChecked(ui)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendReferences==1);xrtClearError();same_heap(&initial);
 puts("Radio init failure: 192 allocation prefixes, cleared outputs, parent state and exact heap cleanup passed");return 0;
}
