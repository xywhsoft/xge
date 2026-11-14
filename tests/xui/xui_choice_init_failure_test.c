/* Built-in init stages release their own partial resources on failure.
 * Preserve the generic successful-stage-only destroy chain. */
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
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));xmemdebugsnapshot initial;xrtMemDebugSnapshot(&initial);
 static xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
 const xui_callback_owner_t owner={{&backendReferences,&ownerOps},hold,drop};
 const xui_context_desc_t options={sizeof(options),XUI_SYSTEM_FONT_DISABLED,0,0};
 xui_proxy proxy=NULL;xui_context ui=NULL;
 assert(xuiProxyCreateOwned(&backend.tProxy,&owner,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&options,&ui)==XUI_OK);
 const xui_button_desc_t button={.iSize=sizeof(button),.sText="partial button"};
 const xui_checkbox_desc_t checkbox={.iSize=sizeof(checkbox),.sText="partial checkbox"};
 const xui_toggle_desc_t toggle={.iSize=sizeof(toggle),.sText="partial toggle",.sUncheckedText="off",.sCheckedText="on"};
 const xui_radio_desc_t radio={.iSize=sizeof(radio),.sText="partial radio"};
 for(int kind=0;kind<4;++kind)for(int defaults=0;defaults<2;++defaults){
  xui_widget warm=NULL;
  int status=kind==0?xuiButtonCreate(ui,&warm,defaults?NULL:&button):kind==1?xuiCheckBoxCreate(ui,&warm,defaults?NULL:&checkbox):kind==2?xuiToggleCreate(ui,&warm,defaults?NULL:&toggle):xuiRadioCreate(ui,&warm,defaults?NULL:&radio);
  assert(status==XUI_OK&&warm&&xuiWidgetDestroyChecked(warm)==XUI_OK);
  unsigned failed=0,passed=0;
  for(int allowance=0;allowance<64;++allowance){
   xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);xui_widget child=warm;assert(xrtMemDebugFailAfter(allowance));
   status=kind==0?xuiButtonCreate(ui,&child,defaults?NULL:&button):kind==1?xuiCheckBoxCreate(ui,&child,defaults?NULL:&checkbox):kind==2?xuiToggleCreate(ui,&child,defaults?NULL:&toggle):xuiRadioCreate(ui,&child,defaults?NULL:&radio);
   xrtMemDebugFailClear();xrtClearError();
   if(status==XUI_OK){++passed;assert(child&&xuiWidgetDestroyChecked(child)==XUI_OK);}else{++failed;assert(child==NULL);}
   same_heap(&before);
  }
  assert(passed&&failed);
 }
 assert(xuiDestroyChecked(ui)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendReferences==1);xrtClearError();same_heap(&initial);
 puts("Choice init failure: 512 allocation prefixes, NULL outputs and exact heap cleanup passed");return 0;
}
