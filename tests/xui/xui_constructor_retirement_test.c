/* Public Widget constructor fault injection with full Context retirement.
 * Queue/cache capacities retained by a live Context are not resource leaks. */
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
typedef int (*Creator)(xui_context,xui_widget*);
static int create0(xui_context ui,xui_widget* output){return xuiLabelCreate(ui,output,NULL);}
static int create1(xui_context ui,xui_widget* output){return xuiHyperlinkCreate(ui,output,NULL);}
static int create2(xui_context ui,xui_widget* output){return xuiBreadcrumbCreate(ui,output,NULL);}
static int create3(xui_context ui,xui_widget* output){return xuiImageCreate(ui,output,NULL);}
static int create4(xui_context ui,xui_widget* output){return xuiQrCodeCreate(ui,output,NULL);}
static int create5(xui_context ui,xui_widget* output){return xuiPanelCreate(ui,output,NULL);}
static int create6(xui_context ui,xui_widget* output){return xuiSeparatorCreate(ui,output,NULL);}
static int create7(xui_context ui,xui_widget* output){return xuiProgressCreate(ui,output,NULL);}
static int create8(xui_context ui,xui_widget* output){return xuiStepBarCreate(ui,output,NULL);}
static int create9(xui_context ui,xui_widget* output){return xuiChartCreate(ui,output,NULL);}
static int create10(xui_context ui,xui_widget* output){return xuiCodeEditCreate(ui,output,NULL);}
static int create11(xui_context ui,xui_widget* output){return xuiInputCreate(ui,output,NULL);}
static int create12(xui_context ui,xui_widget* output){return xuiTagInputCreate(ui,output,NULL);}
static int create13(xui_context ui,xui_widget* output){return xuiNumericInputCreate(ui,output,NULL);}
static int create14(xui_context ui,xui_widget* output){return xuiTextEditCreate(ui,output,NULL);}
static int create15(xui_context ui,xui_widget* output){return xuiButtonCreate(ui,output,NULL);}
static int create16(xui_context ui,xui_widget* output){return xuiCheckBoxCreate(ui,output,NULL);}
static int create17(xui_context ui,xui_widget* output){return xuiCheckCardCreate(ui,output,NULL);}
static int create18(xui_context ui,xui_widget* output){return xuiRadioCreate(ui,output,NULL);}
static int create19(xui_context ui,xui_widget* output){return xuiRadioGroupCreate(ui,output,NULL);}
static int create20(xui_context ui,xui_widget* output){return xuiToggleCreate(ui,output,NULL);}
static int create21(xui_context ui,xui_widget* output){return xuiScrollBarCreate(ui,output,NULL);}
static int create22(xui_context ui,xui_widget* output){return xuiSliderCreate(ui,output,NULL);}
static int create23(xui_context ui,xui_widget* output){return xuiRangeSliderCreate(ui,output,NULL);}
static int create24(xui_context ui,xui_widget* output){return xuiPageCreate(ui,output,NULL);}
static int create25(xui_context ui,xui_widget* output){return xuiCarouselCreate(ui,output,NULL);}
static int create26(xui_context ui,xui_widget* output){return xuiVirtualJoystickCreate(ui,output,NULL);}
static int create27(xui_context ui,xui_widget* output){return xuiInventoryGridCreate(ui,output,NULL);}
static int create28(xui_context ui,xui_widget* output){return xuiTerminalCreate(ui,output,NULL);}
static int create29(xui_context ui,xui_widget* output){return xuiSplitLayoutCreate(ui,output,NULL);}
static int create30(xui_context ui,xui_widget* output){return xuiTabsCreate(ui,output,NULL);}
static int create31(xui_context ui,xui_widget* output){return xuiAccordionCreate(ui,output,NULL);}
static int create32(xui_context ui,xui_widget* output){return xuiWindowCreate(ui,output,NULL);}
static int create33(xui_context ui,xui_widget* output){return xuiDockPanelCreate(ui,output,NULL);}
static int create34(xui_context ui,xui_widget* output){return xuiScrollFrameCreate(ui,output,NULL);}
static int create35(xui_context ui,xui_widget* output){return xuiScrollViewCreate(ui,output,NULL);}
static int create36(xui_context ui,xui_widget* output){return xuiCanvasCreate(ui,output,NULL);}
static int create37(xui_context ui,xui_widget* output){return xuiListViewCreate(ui,output,NULL);}
static int create38(xui_context ui,xui_widget* output){return xuiMessageListCreate(ui,output,NULL);}
static int create39(xui_context ui,xui_widget* output){return xuiTreeViewCreate(ui,output,NULL);}
static int create40(xui_context ui,xui_widget* output){return xuiTableViewCreate(ui,output,NULL);}
static int create41(xui_context ui,xui_widget* output){return xuiTableGridCreate(ui,output,NULL);}
static int create42(xui_context ui,xui_widget* output){return xuiTimeLineViewCreate(ui,output,NULL);}
static int create43(xui_context ui,xui_widget* output){return xuiPropertyGridCreate(ui,output,NULL);}
static int create44(xui_context ui,xui_widget* output){return xuiPopupCreate(ui,output,NULL);}
static int create45(xui_context ui,xui_widget* output){return xuiMenuCreate(ui,output,NULL);}
static int create46(xui_context ui,xui_widget* output){return xuiMenuBarCreate(ui,output,NULL);}
static int create47(xui_context ui,xui_widget* output){return xuiToolbarCreate(ui,output,NULL);}
static int create48(xui_context ui,xui_widget* output){return xuiStatusBarCreate(ui,output,NULL);}
static int create49(xui_context ui,xui_widget* output){return xuiComboBoxCreate(ui,output,NULL);}
static int create50(xui_context ui,xui_widget* output){return xuiCascaderCreate(ui,output,NULL);}
static int create51(xui_context ui,xui_widget* output){return xuiColorPickerCreate(ui,output,NULL);}
static int create52(xui_context ui,xui_widget* output){return xuiIconPickerCreate(ui,output,NULL);}
static int create53(xui_context ui,xui_widget* output){return xuiDatePickerCreate(ui,output,NULL);}
static int create54(xui_context ui,xui_widget* output){return xuiFlowGraphWidgetCreate(ui,output,NULL);}
static int create55(xui_context ui,xui_widget* output){return xuiWorkflowWidgetCreate(ui,output,NULL);}

static int fullHyperlink(xui_context ui,xui_widget* output)
{const xui_hyperlink_desc_t d={.iSize=sizeof(d),.sText="link",.bUnderline=1};return xuiHyperlinkCreate(ui,output,&d);}
static int fullInput(xui_context ui,xui_widget* output)
{const xui_input_desc_t d={.iSize=sizeof(d),.sText="input",.sPlaceholder="hint",.iMaxLength=32,.bPassword=1};return xuiInputCreate(ui,output,&d);}
static int fullTagInput(xui_context ui,xui_widget* output)
{const char* tags[]={"one","two"};const xui_tag_input_desc_t d={.iSize=sizeof(d),.sPlaceholder="hint",.ppTags=tags,.iTagCount=2};return xuiTagInputCreate(ui,output,&d);}
static int fullTextEdit(xui_context ui,xui_widget* output)
{const xui_text_edit_desc_t d={.iSize=sizeof(d),.sText="first\nsecond",.sPlaceholder="hint"};return xuiTextEditCreate(ui,output,&d);}
static int fullTerminal(xui_context ui,xui_widget* output)
{const xui_terminal_desc_t d={.iSize=sizeof(d),.iColumns=4,.iRows=3,.iScrollbackLimit=8};return xuiTerminalCreate(ui,output,&d);}
struct Case {const char* name;Creator create;};
static const struct Case cases[]={
 {"xuiLabelCreate",create0},
 {"xuiHyperlinkCreate",create1},
 {"xuiBreadcrumbCreate",create2},
 {"xuiImageCreate",create3},
 {"xuiQrCodeCreate",create4},
 {"xuiPanelCreate",create5},
 {"xuiSeparatorCreate",create6},
 {"xuiProgressCreate",create7},
 {"xuiStepBarCreate",create8},
 {"xuiChartCreate",create9},
 {"xuiCodeEditCreate",create10},
 {"xuiInputCreate",create11},
 {"xuiTagInputCreate",create12},
 {"xuiNumericInputCreate",create13},
 {"xuiTextEditCreate",create14},
 {"xuiButtonCreate",create15},
 {"xuiCheckBoxCreate",create16},
 {"xuiCheckCardCreate",create17},
 {"xuiRadioCreate",create18},
 {"xuiRadioGroupCreate",create19},
 {"xuiToggleCreate",create20},
 {"xuiScrollBarCreate",create21},
 {"xuiSliderCreate",create22},
 {"xuiRangeSliderCreate",create23},
 {"xuiPageCreate",create24},
 {"xuiCarouselCreate",create25},
 {"xuiVirtualJoystickCreate",create26},
 {"xuiInventoryGridCreate",create27},
 {"xuiTerminalCreate",create28},
 {"xuiSplitLayoutCreate",create29},
 {"xuiTabsCreate",create30},
 {"xuiAccordionCreate",create31},
 {"xuiWindowCreate",create32},
 {"xuiDockPanelCreate",create33},
 {"xuiScrollFrameCreate",create34},
 {"xuiScrollViewCreate",create35},
 {"xuiCanvasCreate",create36},
 {"xuiListViewCreate",create37},
 {"xuiMessageListCreate",create38},
 {"xuiTreeViewCreate",create39},
 {"xuiTableViewCreate",create40},
 {"xuiTableGridCreate",create41},
 {"xuiTimeLineViewCreate",create42},
 {"xuiPropertyGridCreate",create43},
 {"xuiPopupCreate",create44},
 {"xuiMenuCreate",create45},
 {"xuiMenuBarCreate",create46},
 {"xuiToolbarCreate",create47},
 {"xuiStatusBarCreate",create48},
 {"xuiComboBoxCreate",create49},
 {"xuiCascaderCreate",create50},
 {"xuiColorPickerCreate",create51},
 {"xuiIconPickerCreate",create52},
 {"xuiDatePickerCreate",create53},
 {"xuiFlowGraphWidgetCreate",create54},
 {"xuiWorkflowWidgetCreate",create55},

 {"fullHyperlink",fullHyperlink},{"fullInput",fullInput},{"fullTagInput",fullTagInput},
 {"fullTextEdit",fullTextEdit},{"fullTerminal",fullTerminal}
};
static xui_test_proxy_state_t backend;
static void setup(Creator create,xui_proxy* proxy,xui_context* ui)
{
 const xui_callback_owner_t owner={{&backendReferences,&ownerOps},hold,drop};
 const xui_context_desc_t options={sizeof(options),XUI_SYSTEM_FONT_DISABLED,0,0};xui_widget warm=NULL;
 assert(xuiProxyCreateOwned(&backend.tProxy,&owner,proxy)==XUI_OK&&xuiCreateOwnedEx(*proxy,&options,ui)==XUI_OK);
 assert(create(*ui,&warm)==XUI_OK&&warm&&!xrtGetError());assert(xuiWidgetDestroyChecked(warm)==XUI_OK);
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
 assert(argc==2);int kind=atoi(argv[1]);assert(kind>=0&&kind<(int)(sizeof(cases)/sizeof(*cases)));
 assert(xrtMemDebugEnable(true));xmemdebugsnapshot initial;xrtMemDebugSnapshot(&initial);xuiTestProxyInit(&backend);
 Creator create=cases[kind].create;xui_proxy proxy=NULL;xui_context ui=NULL;xui_widget child=NULL;
 setup(create,&proxy,&ui);xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(create(ui,&child)==XUI_OK&&child&&!xrtGetError());xrtMemDebugSnapshot(&after);
 uint64 bound=(after.AllocCount-before.AllocCount)+(after.ReallocCount-before.ReallocCount)+8;assert(bound>=8&&bound<=8192);
 assert(xuiWidgetDestroyChecked(child)==XUI_OK);dispose(proxy,ui,&initial);
 unsigned failures=0,successes=0,triggered=0,untriggered=0;
 for(uint64 allowance=0;allowance<bound;++allowance){
  proxy=NULL;ui=NULL;setup(create,&proxy,&ui);child=(xui_widget)(void*)&backend;
  assert(xrtMemDebugFailAfter(allowance));int status=create(ui,&child);bool injected=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
  if(injected)++triggered;else ++untriggered;
  if(status==XUI_OK){++successes;assert(child&&!xrtGetError());assert(xuiWidgetDestroyChecked(child)==XUI_OK);}else{++failures;assert(child==NULL);}
  xrtClearError();assert(xuiDestroyChecked(ui)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendReferences==1);xrtClearError();
  xrtMemDebugSnapshot(&after);
  if(after.LiveCount!=initial.LiveCount||after.LiveBytes!=initial.LiveBytes||after.InvalidFreeCount!=initial.InvalidFreeCount||after.DoubleFreeCount!=initial.DoubleFreeCount){
   printf("RETIREMENT %s budget=%llu live=%llu/%llu bytes=%llu/%llu invalid=%llu/%llu double=%llu/%llu\n",cases[kind].name,(unsigned long long)allowance,(unsigned long long)initial.LiveCount,(unsigned long long)after.LiveCount,(unsigned long long)initial.LiveBytes,(unsigned long long)after.LiveBytes,(unsigned long long)initial.InvalidFreeCount,(unsigned long long)after.InvalidFreeCount,(unsigned long long)initial.DoubleFreeCount,(unsigned long long)after.DoubleFreeCount);fflush(stdout);
  }
  same_heap(&initial);
 }
 assert(failures&&successes&&triggered&&untriggered);
 printf("Constructor %s: prefixes=%llu failures=%u successes=%u injected=%u clear=%u exact retirement passed\n",cases[kind].name,(unsigned long long)bound,failures,successes,triggered,untriggered);return 0;
}
