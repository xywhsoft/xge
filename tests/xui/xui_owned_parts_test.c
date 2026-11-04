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
#include "../../src/xui/core/xui_internal.h"
static xui_context context;
static xui_widget root,target;
static unsigned protectedCount,initReentries,parentReentries;
static bool closeOwnerInInit;
static xui_widget_type_init_proc originalInit;
static xui_internal_parent_changed_proc originalParent;
static int checked_init(xui_widget widget,void* data,const void* desc,void* user)
{
    assert(widget->pInternalOwner);
    assert(xuiWidgetDestroyChecked(widget)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetGetContext(widget)==context);
    assert(xuiWidgetRemoveFromParent(widget)==XUI_ERROR_INVALID_STATE);
    ++initReentries;
    if(closeOwnerInInit)assert(xuiWidgetDestroyChecked((xui_widget)widget->pInternalOwner)==XUI_OK);
    return originalInit(widget,data,desc,user);
}
static void checked_parent(xui_widget widget,xui_widget oldParent,xui_widget newParent)
{
    (void)oldParent;
    assert(widget->pInternalOwner && newParent);
    assert(xuiWidgetDestroyChecked(widget)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetRemoveFromParent(widget)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetAddChild(target,widget)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetGetParent(widget)==newParent);
    ++parentReentries;
}
static void protect(xui_widget part)
{
    assert(part);
    xui_widget parent=xuiWidgetGetParent(part),previous=xuiWidgetGetPrevSibling(part),next=xuiWidgetGetNextSibling(part);
    int children=parent ? xuiWidgetGetChildCount(parent):0;
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
    assert(xuiWidgetDestroyChecked(part)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetRemoveFromParent(part)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetAddChild(target,part)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetInsertBefore(target,part,NULL)==XUI_ERROR_INVALID_STATE);
    assert(xuiOverlayAttach(context,NULL,part,XUI_LAYER_FLOATING,0)==XUI_ERROR_INVALID_STATE);
    assert(xuiOverlayDetach(part)==XUI_ERROR_INVALID_STATE);
    assert(xuiSetRootWidget(context,part)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetGetContext(part)==context && xuiWidgetGetParent(part)==parent);
    assert(xuiWidgetGetPrevSibling(part)==previous && xuiWidgetGetNextSibling(part)==next);
    assert(!parent || xuiWidgetGetChildCount(parent)==children);
    assert(xuiWidgetSetEnabled(part,1)==XUI_OK && xuiWidgetGetEnabled(part));
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    ++protectedCount;
}
static void actual_composites(void)
{
    xui_widget panel,frame,scroll,popup,menu,tabs,split,carousel,grid,numeric,combo,window,accordion;
    xui_widget_type image=xuiImageGetType(context);
    originalInit=image->onInit;originalParent=image->onParentChanged;image->onInit=checked_init;image->onParentChanged=checked_parent;
    assert(xuiPanelCreate(context,&panel,NULL)==XUI_OK);
    image->onInit=originalInit;image->onParentChanged=originalParent;
    assert(initReentries && parentReentries);
    protect(xuiPanelGetHeaderWidget(panel));protect(xuiPanelGetIconWidget(panel));protect(xuiPanelGetTitleWidget(panel));protect(xuiPanelGetClientWidget(panel));
    assert(xuiScrollFrameCreate(context,&frame,NULL)==XUI_OK);
    protect(xuiScrollFrameGetViewportWidget(frame));protect(xuiScrollFrameGetHScrollBarWidget(frame));protect(xuiScrollFrameGetVScrollBarWidget(frame));protect(xuiScrollFrameGetCornerWidget(frame));
    assert(xuiScrollViewCreate(context,&scroll,NULL)==XUI_OK);
    protect(xuiScrollViewGetFrameWidget(scroll));protect(xuiScrollViewGetContentWidget(scroll));protect(xuiScrollViewGetViewportWidget(scroll));
    assert(xuiPopupCreate(context,&popup,NULL)==XUI_OK);
    protect(xuiPopupGetPanelWidget(popup));protect(xuiPopupGetScrollViewWidget(popup));protect(xuiPopupGetFrameWidget(popup));protect(xuiPopupGetViewportWidget(popup));protect(xuiPopupGetContentWidget(popup));
    assert(xuiMenuCreate(context,&menu,NULL)==XUI_OK);protect(xuiMenuGetPopupWidget(menu));
    assert(xuiTabsCreate(context,&tabs,NULL)==XUI_OK);
    const char* names[]={"one","two"};assert(xuiTabsSetItems(tabs,names,2)==XUI_OK);
    protect(xuiTabsGetTabBarWidget(tabs));protect(xuiTabsGetClientWidget(tabs));protect(xuiTabsGetPageWidget(tabs,0));protect(xuiTabsGetButtonWidget(tabs,0));
    assert(xuiTabsRemovePage(tabs,0)==XUI_OK);
    protect(xuiTabsGetPageWidget(tabs,0));protect(xuiTabsGetButtonWidget(tabs,0));
    assert(xuiSplitLayoutCreate(context,&split,NULL)==XUI_OK && xuiSplitLayoutSetPaneCount(split,2)==XUI_OK);
    protect(xuiSplitLayoutGetPaneWidget(split,0));
    xui_call_scope_t replaced={0};assert(xuiCallScopeBegin(context,&replaced)==XUI_OK);
    xui_widget retired=xuiSplitLayoutGetPaneWidget(split,1);
    assert(xuiSplitLayoutSetPaneCount(split,1)==XUI_OK && xuiSplitLayoutSetPaneCount(split,2)==XUI_OK);
    xui_widget replacement=xuiSplitLayoutGetPaneWidget(split,1);assert(replacement && replacement!=retired);
    assert(xuiCallScopeEnd(&replaced)==XUI_OK && xuiSplitLayoutGetPaneWidget(split,1)==replacement);
    protect(replacement);
    assert(xuiCarouselCreate(context,&carousel,NULL)==XUI_OK && xuiCarouselSetPageCount(carousel,2)==XUI_OK);
    protect(xuiCarouselGetPageWidget(carousel,0));assert(xuiCarouselSetPageCount(carousel,1)==XUI_OK);
    assert(xuiTableGridCreate(context,&grid,NULL)==XUI_OK);protect(xuiTableGridGetTableView(grid));
    assert(xuiNumericInputCreate(context,&numeric,NULL)==XUI_OK);protect(xuiNumericInputGetInputWidget(numeric));
    assert(xuiComboBoxCreate(context,&combo,NULL)==XUI_OK);protect(xuiComboBoxGetMenuWidget(combo));protect(xuiComboBoxGetPopupWidget(combo));
    assert(xuiWindowCreate(context,&window,NULL)==XUI_OK);protect(xuiWindowGetClientWidget(window));protect(xuiWindowGetCloseButtonWidget(window));
    assert(xuiWindowSetOpen(window,1)==XUI_OK && xuiWindowSetOpen(window,0)==XUI_OK);
    assert(xuiAccordionCreate(context,&accordion,NULL)==XUI_OK);
    xui_widget client=NULL;assert(xuiAccordionAddSection(accordion,"one",1,1,&client)==XUI_OK);protect(client);protect(xuiAccordionGetHeaderWidget(accordion,0));assert(xuiAccordionRemoveSection(accordion,0)==XUI_OK);
    xui_widget child=NULL;assert(xuiWidgetCreate(context,&child)==XUI_OK && xuiPanelAddChild(panel,child)==XUI_OK);
    assert(xuiWidgetRemoveFromParent(child)==XUI_OK && xuiWidgetAddChild(target,child)==XUI_OK);
    assert(xuiWidgetDestroyChecked(child)==XUI_OK);
    xui_widget group=NULL,radio=NULL;assert(xuiRadioGroupCreate(context,&group,NULL)==XUI_OK && xuiRadioGroupAddOption(group,&radio,NULL)==XUI_OK);
    assert(xuiWidgetRemoveFromParent(radio)==XUI_OK && xuiWidgetAddChild(target,radio)==XUI_OK && xuiWidgetDestroyChecked(radio)==XUI_OK);
    assert(xuiWidgetDestroyChecked(group)==XUI_OK);
    xui_widget widgets[]={panel,frame,scroll,popup,tabs,split,carousel,grid,numeric,combo,window,accordion};
    for(size_t i=0;i<sizeof(widgets)/sizeof(*widgets);++i)assert(xuiWidgetDestroyChecked(widgets[i])==XUI_OK);
    assert(xuiWidgetDestroyChecked(menu)==XUI_OK);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xui_test_proxy_state_t proxy;xuiTestProxyInit(&proxy);
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    assert(xuiInputViewport(context,640,480)==XUI_OK);
    assert(xuiWidgetCreate(context,&root)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
    assert(xuiWidgetCreate(context,&target)==XUI_OK && xuiWidgetAddChild(root,target)==XUI_OK);
    actual_composites();
    /* Pending Native off-tree roots disarm their fixed owner slots before the
     * parent TypeData is released; tree child slots are cleared in physical tail. */
    for(int i=0;i<20;++i){
        xui_widget combo=NULL;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
        assert(xuiComboBoxCreate(context,&combo,NULL)==XUI_OK);
        xui_widget menu=xuiComboBoxGetMenuWidget(combo),popup=xuiComboBoxGetPopupWidget(combo);
        assert(menu->ppInternalOwnerSlot && popup->ppInternalOwnerSlot);
        assert(xuiWidgetDestroyChecked(combo)==XUI_OK);
        assert(xuiCallScopeEnd(&scope)==XUI_OK);
    }
    xui_widget_type image=xuiImageGetType(context);originalInit=image->onInit;
    image->onInit=checked_init;closeOwnerInInit=true;
    xui_widget closed=NULL;assert(xuiPanelCreate(context,&closed,NULL)!=XUI_OK && closed==NULL);
    closeOwnerInInit=false;image->onInit=originalInit;xrtClearError();
    unsigned failures=0,successes=0;
    for(int budget=0;budget<50;++budget){
        xui_widget panel=NULL;assert(xrtMemDebugFailAfter((uint64)budget));
        int status=xuiPanelCreate(context,&panel,NULL);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(status==XUI_OK){++successes;assert(panel && xuiWidgetDestroyChecked(panel)==XUI_OK);}
        else {++failures;assert(!panel);}
    }
    assert(failures && successes);
    xuiDestroy(context);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    printf("internal composite parts: %u protected getters, pre-init and parent-change reentry rejected; getters/config remain live; ordinary/user-added/RadioGroup mutation preserved\n",protectedCount);
    return 0;
}
