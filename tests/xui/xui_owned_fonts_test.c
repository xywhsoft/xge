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

#include "src/xui/core/xui_internal.h"
#include "src/xui/core/xui_proxy_resource.h"
#include "xui_test_proxy.h"

typedef struct Owner {size_t refs;bool failDrop;} Owner;
static bool hold(const void* raw){++((Owner*)raw)->refs;return true;}
static bool drop(const void* raw){Owner* value=(Owner*)raw;assert(value->refs);--value->refs;return !value->failDrop;}
static bool count(const void* raw,size_t* out){*out=((const Owner*)raw)->refs;return *out!=0;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ownerOps={count,trace};
static xui_callback_owner_t ownerRef(Owner* owner){return (xui_callback_owner_t){{owner,&ownerOps},hold,drop};}
typedef struct Payload {
    unsigned* finalized;int status;
    xui_widget widget,other; xui_font replacement;int* sameStatus;int* otherStatus;
} Payload;
static int finalize(xui_proxy proxy,void* raw)
{
    (void)proxy;Payload* p=raw;++*p->finalized;
    if(p->sameStatus){
        xui_context context=p->widget->pContext;assert(context->pActiveFontUpdates);
        *p->sameStatus=xuiButtonSetFont(p->widget,p->replacement);
        assert(context->pActiveFontUpdates);
    }
    if(p->otherStatus){
        xui_font_slot_update_t* outer=p->widget->pContext->pActiveFontUpdates;
        *p->otherStatus=xuiLabelSetFont(p->other,p->replacement);
        assert(p->widget->pContext->pActiveFontUpdates==outer);
    }
    return p->status;
}
static xui_font fontCreate(xui_proxy proxy,unsigned* finalized,int status)
{
    xui_font font=NULL;void* raw=NULL;assert(xuiFontCreateOwned(proxy,sizeof(Payload),finalize,&font,&raw)==XUI_OK);
    *(Payload*)raw=(Payload){.finalized=finalized,.status=status};return font;
}
static size_t refs(xrtownershipref ref)
{
    size_t value=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Data && ref.Ops && ref.Ops->Count(ref.Data,&value));assert(xrtOwnershipScopeEnd(&freeze));return value;
}
static int metrics(xui_proxy proxy,xui_font font,xui_font_metrics_t* out)
{
    (void)proxy;if(!font || !out)return XUI_ERROR_INVALID_ARGUMENT;
    *out=(xui_font_metrics_t){.fSize=16,.fAscent=12,.fDescent=4,.fLineHeight=16};return XUI_OK;
}
static int measure(xui_proxy proxy,const xui_text_item_t* item,xui_vec2_t* out)
{
    (void)proxy;if(!item || !item->pFont || !out)return XUI_ERROR_INVALID_ARGUMENT;
    int bytes=item->iTextSize<0 ? (int)strlen(item->sText):item->iTextSize;
    *out=(xui_vec2_t){bytes*8,16};return XUI_OK;
}
static xui_proxy proxyCreate(xui_test_proxy_state_t* state,Owner* owner)
{
    xuiTestProxyInit(state);state->tProxy.fontGetMetrics=metrics;state->tProxy.textMeasure=measure;
    xui_callback_owner_t binding=ownerRef(owner);xui_proxy proxy=NULL;
    assert(xuiProxyCreateOwned(&state->tProxy,&binding,&proxy)==XUI_OK);return proxy;
}
typedef struct FontFamily {
    const char* name;xui_widget_type(*getType)(xui_context);
    int(*setFont)(xui_widget,xui_font);xui_font(*getFont)(xui_widget);size_t slots;
} FontFamily;
static const FontFamily families[]={
    {"xuiCodeEdit",xuiCodeEditGetType,xuiCodeEditSetFont,xuiCodeEditGetFont,1 },
    {"xuiAccordion",xuiAccordionGetType,xuiAccordionSetFont,xuiAccordionGetFont,1 },
    {"xuiBreadcrumb",xuiBreadcrumbGetType,xuiBreadcrumbSetFont,xuiBreadcrumbGetFont,1 },
    {"xuiButton",xuiButtonGetType,xuiButtonSetFont,xuiButtonGetFont,1 },
    {"xuiCarousel",xuiCarouselGetType,xuiCarouselSetFont,xuiCarouselGetFont,1 },
    {"xuiCascader",xuiCascaderGetType,xuiCascaderSetFont,xuiCascaderGetFont,1 },
    {"xuiChart",xuiChartGetType,xuiChartSetFont,xuiChartGetFont,1 },
    {"xuiCheckBox",xuiCheckBoxGetType,xuiCheckBoxSetFont,xuiCheckBoxGetFont,1 },
    {"xuiColorPicker",xuiColorPickerGetType,xuiColorPickerSetFont,xuiColorPickerGetFont,1 },
    {"xuiComboBox",xuiComboBoxGetType,xuiComboBoxSetFont,xuiComboBoxGetFont,1 },
    {"xuiDatePicker",xuiDatePickerGetType,xuiDatePickerSetFont,xuiDatePickerGetFont,1 },
    {"xuiDockPanel",xuiDockPanelGetType,xuiDockPanelSetFont,xuiDockPanelGetFont,1 },
    {"xuiHyperlink",xuiHyperlinkGetType,xuiHyperlinkSetFont,xuiHyperlinkGetFont,1 },
    {"xuiInventoryGrid",xuiInventoryGridGetType,xuiInventoryGridSetFont,xuiInventoryGridGetFont,1 },
    {"xuiLabel",xuiLabelGetType,xuiLabelSetFont,xuiLabelGetFont,1 },
    {"xuiListView",xuiListViewGetType,xuiListViewSetFont,xuiListViewGetFont,1 },
    {"xuiMenu",xuiMenuGetType,xuiMenuSetFont,xuiMenuGetFont,1 },
    {"xuiMenuBar",xuiMenuBarGetType,xuiMenuBarSetFont,xuiMenuBarGetFont,1 },
    {"xuiMessageList",xuiMessageListGetType,xuiMessageListSetFont,xuiMessageListGetFont,1 },
    {"xuiNumericInput",xuiNumericInputGetType,xuiNumericInputSetFont,xuiNumericInputGetFont,1 },
    {"xuiPage",xuiPageGetType,xuiPageSetFont,xuiPageGetFont,1 },
    {"xuiPanel",xuiPanelGetType,xuiPanelSetFont,xuiPanelGetFont,1 },
    {"xuiProgress",xuiProgressGetType,xuiProgressSetFont,xuiProgressGetFont,1 },
    {"xuiPropertyGrid",xuiPropertyGridGetType,xuiPropertyGridSetFont,xuiPropertyGridGetFont,1 },
    {"xuiRadio",xuiRadioGetType,xuiRadioSetFont,xuiRadioGetFont,1 },
    {"xuiStatusBar",xuiStatusBarGetType,xuiStatusBarSetFont,xuiStatusBarGetFont,1 },
    {"xuiStepBar",xuiStepBarGetType,xuiStepBarSetFont,xuiStepBarGetFont,1 },
    {"xuiTableGrid",xuiTableGridGetType,xuiTableGridSetFont,xuiTableGridGetFont,1 },
    {"xuiTableView",xuiTableViewGetType,xuiTableViewSetFont,xuiTableViewGetFont,1 },
    {"xuiTabs",xuiTabsGetType,xuiTabsSetFont,xuiTabsGetFont,1 },
    {"xuiTagInput",xuiTagInputGetType,xuiTagInputSetFont,xuiTagInputGetFont,1 },
    {"xuiTerminal",xuiTerminalGetType,xuiTerminalSetFont,xuiTerminalGetFont,2 },
    {"xuiTimeLineView",xuiTimeLineViewGetType,xuiTimeLineViewSetFont,xuiTimeLineViewGetFont,1 },
    {"xuiToggle",xuiToggleGetType,xuiToggleSetFont,xuiToggleGetFont,1 },
    {"xuiToolbar",xuiToolbarGetType,xuiToolbarSetFont,xuiToolbarGetFont,1 },
    {"xuiTreeView",xuiTreeViewGetType,xuiTreeViewSetFont,xuiTreeViewGetFont,1 },
    {"xuiWindow",xuiWindowGetType,xuiWindowSetFont,xuiWindowGetFont,1 },
    {"xuiIconPicker",xuiIconPickerGetType,xuiIconPickerSetFont,xuiIconPickerGetFont,1 },
    {"xuiInput",xuiInputGetType,xuiInputSetFont,xuiInputGetFont,1 },
    {"xuiTextEdit",xuiTextEditGetType,xuiTextEditSetFont,xuiTextEditGetFont,1 },
};
static void all_families(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);
    unsigned finalized=0;xui_font a=fontCreate(proxy,&finalized,0),b=fontCreate(proxy,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    assert(xuiSetDefaultFont(context,a)==XUI_OK && refs(xuiFontOwnership(a))==2);
    unsigned created=0;
    for(size_t i=0;i<sizeof(families)/sizeof(families[0]);++i){
        const FontFamily* f=&families[i];xui_widget widget=NULL;xui_widget_type type=f->getType(context);assert(type);
        int result=xuiWidgetCreateTyped(context,type,&widget,NULL);
        if(result!=XUI_OK){fprintf(stderr,"family create failed %s status=%d\n",f->name,result);assert(result==XUI_OK);}
        assert(widget);++created;
        assert(f->setFont(widget,a)==XUI_OK && f->getFont(widget)==a);
        assert(f->setFont(widget,b)==XUI_OK && f->getFont(widget)==b);
        assert(refs(xuiFontOwnership(b))>=1+f->slots);
        assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
        assert(refs(xuiFontOwnership(a))==2 && refs(xuiFontOwnership(b))==1);
    }
    assert(created==40);xuiDestroy(context);
    assert(refs(xuiFontOwnership(a))==1 && xuiFontReleaseChecked(a)==XUI_OK && xuiFontReleaseChecked(b)==XUI_OK);
    assert(finalized==2 && xuiProxyReleaseChecked(proxy)==XUI_OK && owner.refs==1 && drop(&owner));
}
static unsigned matchingEdges;
static xrtownershipref matching;
static bool edge(xrtownershipref ref,void* user){(void)user;if(ref.Data==matching.Data && ref.Ops==matching.Ops)++matchingEdges;return true;}
static void context_slots(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);
    unsigned finalized=0;xui_font a=fontCreate(proxy,&finalized,0),b=fontCreate(proxy,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    assert(xuiSetDefaultFont(context,a)==XUI_OK && xuiRegisterFont(context,"one",a)==XUI_OK && xuiRegisterFont(context,"two",a)==XUI_OK);
    assert(refs(xuiFontOwnership(a))==4);
    matching=xuiFontOwnership(a);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==3);assert(xrtOwnershipScopeEnd(&freeze));
    xui_theme_t theme={0};assert(xuiGetTheme(context,&theme)==XUI_OK && theme.pFont==a);
    theme.pFont=b;assert(xuiSetTheme(context,&theme)==XUI_OK && xuiGetDefaultFont(context)==b);
    assert(refs(xuiFontOwnership(a))==3 && refs(xuiFontOwnership(b))==2);
    assert(xuiRegisterFont(context,"one",b)==XUI_OK && refs(xuiFontOwnership(a))==2 && refs(xuiFontOwnership(b))==3);
    xuiClearFonts(context);assert(!xuiGetDefaultFont(context) && !xuiFindFont(context,"one"));
    assert(xuiGetTheme(context,&theme)==XUI_OK && !theme.pFont);
    assert(refs(xuiFontOwnership(a))==1 && refs(xuiFontOwnership(b))==1);
    xuiDestroy(context);assert(xuiFontReleaseChecked(a)==XUI_OK && xuiFontReleaseChecked(b)==XUI_OK && finalized==2);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && owner.refs==1 && drop(&owner));
}
static xui_font selected;static bool failInit;
static int customInit(xui_widget widget,void* data,const void* input,void* user)
{
    (void)data;(void)input;(void)user;
    assert(!widget->bTypeInitialized && xuiButtonSetFont(widget,selected)==XUI_OK);
    assert(xuiButtonSetFont(widget,selected)==XUI_OK);
    return failInit ? XUI_ERROR_INVALID_STATE:XUI_OK;
}
static void initialization_and_rejection(void)
{
    xui_test_proxy_state_t state,otherState;Owner owner={.refs=1},otherOwner={.refs=1};
    xui_proxy proxy=proxyCreate(&state,&owner),other=proxyCreate(&otherState,&otherOwner);
    unsigned finalized=0;xui_font a=fontCreate(proxy,&finalized,0),b=fontCreate(proxy,&finalized,0),foreign=fontCreate(other,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK && xuiSetDefaultFont(context,a)==XUI_OK);
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="owned-font-init",.pParent=xuiButtonGetType(context),.onInit=customInit};
    desc.iTypeDataSize=desc.pParent->iTypeDataSize;
    xui_widget_type type=NULL;assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);selected=b;
    for(int failure=0;failure<2;++failure){
        failInit=failure;xui_widget widget=NULL;int status=xuiWidgetCreateTyped(context,type,&widget,NULL);
        if(failure)assert(status==XUI_ERROR_INVALID_STATE && !widget);
        else{
            assert(status==XUI_OK && widget && xuiButtonGetFont(widget)==b && refs(xuiFontOwnership(b))==2);
            assert(xuiButtonSetFont(widget,foreign)==XUI_ERROR_INVALID_ARGUMENT && xuiButtonGetFont(widget)==b);
            assert(refs(xuiFontOwnership(foreign))==1 && xuiWidgetDestroyChecked(widget)==XUI_OK);
        }
        assert(refs(xuiFontOwnership(a))==2 && refs(xuiFontOwnership(b))==1);xrtClearError();
    }
    xui_button_desc_t foreignDesc={.iSize=sizeof(foreignDesc),.pFont=foreign};xui_widget bad=NULL;
    assert(xuiButtonCreate(context,&bad,&foreignDesc)==XUI_ERROR_INVALID_ARGUMENT && !bad && refs(xuiFontOwnership(foreign))==1);xrtClearError();
    xuiDestroy(context);assert(xuiFontReleaseChecked(a)==XUI_OK && xuiFontReleaseChecked(b)==XUI_OK && xuiFontReleaseChecked(foreign)==XUI_OK && finalized==3);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && xuiProxyReleaseChecked(other)==XUI_OK && drop(&owner) && drop(&otherOwner));
}
static void mutator_stack_and_oom(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);
    unsigned finalized=0;xui_font a=fontCreate(proxy,&finalized,0),b=fontCreate(proxy,&finalized,0),c=fontCreate(proxy,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK && xuiSetDefaultFont(context,a)==XUI_OK);
    xui_widget button=NULL,label=NULL;assert(xuiButtonCreate(context,&button,NULL)==XUI_OK && xuiLabelCreate(context,&label,NULL)==XUI_OK);
    assert(xuiButtonSetFont(button,b)==XUI_OK && !context->pActiveFontUpdates);
    xui_font retiring=fontCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);
    Payload* payload=xuiInternalProxyResourcePayload(retiring);int same=-1,other=-1;
    payload->widget=button;payload->other=label;payload->replacement=c;payload->sameStatus=&same;payload->otherStatus=&other;
    assert(xuiButtonSetFont(button,retiring)==XUI_OK && xuiFontReleaseChecked(retiring)==XUI_OK);
    assert(xuiButtonSetFont(button,b)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(same==XUI_ERROR_BUSY && other==XUI_OK && !context->pActiveFontUpdates && xuiButtonGetFont(button)==b && xuiLabelGetFont(label)==c);
    for(int budget=0;budget<20;++budget){
        xui_widget candidate=NULL;assert(xrtMemDebugFailAfter(budget));
        int status=xuiButtonCreate(context,&candidate,NULL);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(status==XUI_OK)assert(candidate && xuiWidgetDestroyChecked(candidate)==XUI_OK);
        else assert(!candidate);
        assert(!context->pActiveFontUpdates);
    }
    assert(xuiWidgetDestroyChecked(button)==XUI_OK && xuiWidgetDestroyChecked(label)==XUI_OK);
    assert(refs(xuiFontOwnership(a))==2 && refs(xuiFontOwnership(b))==1 && refs(xuiFontOwnership(c))==1);
    xuiDestroy(context);assert(xuiFontReleaseChecked(a)==XUI_OK && xuiFontReleaseChecked(b)==XUI_OK && xuiFontReleaseChecked(c)==XUI_OK);
    assert(finalized==4 && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void cleanup_false_null(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);unsigned finalized=0;
    xui_font font=fontCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);xui_context context=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    assert(xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);xuiDestroy(context);
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && finalized==1 && owner.refs==0 && !xrtGetError());
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    all_families();context_slots();initialization_and_rejection();mutator_stack_and_oom();cleanup_false_null();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Owned native font slots: 40 families, Init exactly once, context aliases/registry, rejection and false-NULL cleanup passed");return 0;
}
