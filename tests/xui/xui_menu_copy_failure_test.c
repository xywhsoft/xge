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


#include <stdlib.h>
static xui_test_proxy_state_t backend;
static const xui_menu_item_t seed[]={
 {.sText="one",.sShortcut="Ctrl+1",.iType=XUI_MENU_ITEM_NORMAL,.iState=XUI_MENU_ITEM_ENABLED,.iValue=1},
 {.sText="two",.sShortcut="Ctrl+2",.iType=XUI_MENU_ITEM_NORMAL,.iState=XUI_MENU_ITEM_ENABLED,.iValue=2}
};
static int operation(xui_widget menu,int kind)
{
 char text[]="replacement",shortcut[]="Alt+N";
 xui_menu_item_t items[2]={{.sText=text,.sShortcut=shortcut,.iType=XUI_MENU_ITEM_NORMAL,.iState=XUI_MENU_ITEM_ENABLED,.iValue=7},
 {.sText="second",.sShortcut="F2",.iType=XUI_MENU_ITEM_NORMAL,.iState=XUI_MENU_ITEM_ENABLED,.iValue=8}};
 if(kind==2){items[0]=*xuiMenuGetItem(menu,1);items[1]=*xuiMenuGetItem(menu,0);items[0].iValue=99;}
 if(kind==3){items[0]=*xuiMenuGetItem(menu,0);items[0].iValue=99;}
 if(kind==4)items[0].sShortcut=NULL;
 if(kind==5)items[0].sText=NULL;
 unsigned char saved[sizeof(items)];memcpy(saved,items,sizeof(items));
 int status=(kind==1||kind==3)?xuiMenuAddItem(menu,items):xuiMenuSetItems(menu,items,2);
 assert(memcmp(saved,items,sizeof(items))==0&&!strcmp(text,"replacement")&&!strcmp(shortcut,"Alt+N"));
 return status;
}
static void check(xui_widget menu,int kind,bool committed)
{
 int count=committed&&(kind==1||kind==3)?3:2;assert(xuiMenuGetItemCount(menu)==count);
 if(!committed){
  for(int i=0;i<2;++i){const xui_menu_item_t* item=xuiMenuGetItem(menu,i);assert(item&&item->iValue==seed[i].iValue&&!strcmp(item->sText,seed[i].sText)&&!strcmp(item->sShortcut,seed[i].sShortcut));}
  return;
 }
 int index=(kind==1||kind==3)?2:0;const xui_menu_item_t* item=xuiMenuGetItem(menu,index);assert(item);
 const char* text=kind==2?"two":kind==3?"one":kind==5?NULL:"replacement";
 const char* shortcut=kind==2?"Ctrl+2":kind==3?"Ctrl+1":kind==4?NULL:"Alt+N";
 assert((text?item->sText&&!strcmp(item->sText,text):item->sText==NULL)&&(shortcut?item->sShortcut&&!strcmp(item->sShortcut,shortcut):item->sShortcut==NULL));
 assert(item->iValue==((kind==2||kind==3)?99:7));
}
static void setup(int kind,xui_proxy* proxy,xui_context* ui,xui_widget* menu)
{
 const xui_callback_owner_t owner={{&backendReferences,&ownerOps},hold,drop};const xui_context_desc_t options={sizeof(options),XUI_SYSTEM_FONT_DISABLED,0,0};
 assert(xuiProxyCreateOwned(&backend.tProxy,&owner,proxy)==XUI_OK&&xuiCreateOwnedEx(*proxy,&options,ui)==XUI_OK&&xuiMenuCreate(*ui,menu,NULL)==XUI_OK);
 assert(xuiMenuSetItems(*menu,seed,2)==XUI_OK&&operation(*menu,kind)==XUI_OK&&xuiMenuSetItems(*menu,seed,2)==XUI_OK);
}
static void dispose(xui_proxy proxy,xui_context ui,const xmemdebugsnapshot* initial)
{assert(xuiDestroyChecked(ui)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendReferences==1);xrtClearError();same_heap(initial);}
int main(int argc,char**argv)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(argc==2);int kind=atoi(argv[1]);assert(kind>=0&&kind<6);assert(xrtMemDebugEnable(true));xmemdebugsnapshot initial,before,after;xrtMemDebugSnapshot(&initial);xuiTestProxyInit(&backend);
 xui_proxy proxy=NULL;xui_context ui=NULL;xui_widget menu=NULL;setup(kind,&proxy,&ui,&menu);xrtMemDebugSnapshot(&before);assert(operation(menu,kind)==XUI_OK&&!xrtGetError());xrtMemDebugSnapshot(&after);uint64 bound=after.AllocCount-before.AllocCount+after.ReallocCount-before.ReallocCount+8;assert(bound>=8&&bound<=8192);dispose(proxy,ui,&initial);
 unsigned failures=0,successes=0,triggered=0,untriggered=0;
 for(uint64 allowance=0;allowance<bound;++allowance){
  proxy=NULL;ui=NULL;menu=NULL;setup(kind,&proxy,&ui,&menu);assert(xrtMemDebugFailAfter(allowance));int status=operation(menu,kind);bool injected=xrtMemDebugFailTriggered();xrtMemDebugFailClear();if(injected)++triggered;else ++untriggered;
  if(status==XUI_OK){++successes;assert(!xrtGetError());check(menu,kind,true);}else{++failures;check(menu,kind,false);}
  xrtClearError();dispose(proxy,ui,&initial);
 }
 assert(failures&&successes&&triggered&&untriggered);
 printf("Menu strings kind=%d: prefixes=%llu failures=%u successes=%u caller/old-state preservation and exact retirement passed\n",kind,(unsigned long long)bound,failures,successes);return 0;
}
