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
typedef struct Owner {unsigned refs,calls,released,drops;bool inCall,failHold,failDrop;int kind,action,holdAction;xui_context context;xui_widget widget;struct Owner* other;} Owner;
typedef struct User {unsigned marker;Owner* owner;} User;
static int install(int,xui_widget,User*,const xui_callback_owner_t*);
static int clear(int,xui_widget);
static xui_callback_owner_t descriptor(Owner*);
static bool hold(const void* raw)
{Owner* o=(Owner*)raw;if(o->failHold)return false;assert(o->refs);++o->refs;int action=o->holdAction;o->holdAction=0;if(action==1){User u={0xCE11,o->other};xui_callback_owner_t r=descriptor(o->other);assert(install(o->kind,o->widget,&u,&r)==XUI_OK);/* Replaced before use; the callback will never inspect this temporary user. */}else if(action==2)xuiDestroy(o->context);else if(action==3)assert(xuiInputSetText(o->widget,"reentered") == XUI_OK);return true;}
static bool drop(const void* raw)
{Owner* o=(Owner*)raw;assert(o->refs);++o->drops;if(--o->refs)return true;assert(!o->inCall);++o->released;return !o->failDrop;}
static bool count(const void* raw,size_t* result){const Owner* o=raw;*result=o->refs;return o->refs!=0;}
static bool trace(const void* raw,xrtownershipvisitor visitor,void* user){(void)raw;(void)visitor;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_callback_owner_t descriptor(Owner* owner){return(xui_callback_owner_t){{owner,&ops},hold,drop};}
static bool visit(xrtownershipref edge,void* user){unsigned* n=user;assert(edge.Data && edge.Ops==&ops);++*n;return true;}
static void body(xui_widget widget,void* raw)
{(void)widget;User* u=raw;assert(u && u->marker==0xCE11);Owner* o=u->owner;assert(o->refs>=2 && !o->released);++o->calls;o->inCall=true;if(o->action==1)assert(clear(o->kind,o->widget)==XUI_OK);else if(o->action==2)xuiDestroy(o->context);assert(o->refs && !o->released);o->inCall=false;}
static void callback_xui_accordion_select_proc(xui_widget_t* pWidget, int iIndex, int iId, void* pUser) {body(pWidget, pUser);}
static void callback_xui_cascader_change_proc(xui_widget_t* pWidget, int iLeafIndex, const int* arrValues, int iDepth, void* pUser) {body(pWidget, pUser);}
static void callback_xui_color_picker_change_proc(xui_widget_t* pWidget, uint32_t iColor, void* pUser) {body(pWidget, pUser);}
static void callback_xui_combobox_select_proc(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser) {body(pWidget, pUser);}
static void callback_xui_combobox_text_proc(xui_widget_t* pWidget, const char* sText, void* pUser) {body(pWidget, pUser);}
static void callback_xui_date_picker_proc(xui_widget_t* pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser) {body(pWidget, pUser);}
static void callback_xui_dock_active_proc(xui_widget_t* pWidget, int iPane, int iOldWindow, int iNewWindow, void* pUser) {body(pWidget, pUser);}
static void callback_xui_dock_window_close_proc(xui_widget_t* pWidget, int iWindow, void* pUser) {body(pWidget, pUser);}
static void callback_xui_dock_window_state_proc(xui_widget_t* pWidget, int iWindow, int iOldState, int iNewState, void* pUser) {body(pWidget, pUser);}
static void callback_xui_icon_picker_change_proc(xui_widget_t* pWidget, uint32_t iOldId, uint32_t iNewId, void* pUser) {body(pWidget, pUser);}
static void callback_xui_input_change_proc(xui_widget_t* pWidget, const char* sText, void* pUser) {body(pWidget, pUser);}
static void callback_xui_inventory_activate_proc(xui_widget_t* pWidget, int iSlot, int iButton, void* pUser) {body(pWidget, pUser);}
static void callback_xui_inventory_context_proc(xui_widget_t* pWidget, int iSlot, float fX, float fY, void* pUser) {body(pWidget, pUser);}
static void callback_xui_inventory_select_proc(xui_widget_t* pWidget, int iSlot, void* pUser) {body(pWidget, pUser);}
static void callback_xui_inventory_split_proc(xui_widget_t* pWidget, int iSlot, int iCount, void* pUser) {body(pWidget, pUser);}
static void callback_xui_list_view_select_proc(xui_widget_t* pWidget, int iIndex, void* pUser) {body(pWidget, pUser);}
static void callback_xui_menu_select_proc(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser) {body(pWidget, pUser);}
static void callback_xui_message_list_event_proc(xui_widget_t* pWidget, const xui_message_list_event_t* pEvent, void* pUser) {body(pWidget, pUser);}
static void callback_xui_numeric_input_change_proc(xui_widget_t* pWidget, float fValue, void* pUser) {body(pWidget, pUser);}
static void callback_xui_numeric_input_error_proc(xui_widget_t* pWidget, int bError, void* pUser) {body(pWidget, pUser);}
static void callback_xui_popup_change_proc(xui_widget_t* pWidget, int bOpen, void* pUser) {body(pWidget, pUser);}
static void callback_xui_property_grid_change_proc(xui_widget_t* pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser) {body(pWidget, pUser);}
static void callback_xui_property_grid_select_proc(xui_widget_t* pWidget, int iProperty, const char* sId, void* pUser) {body(pWidget, pUser);}
static void callback_xui_radio_group_change_proc(xui_widget_t* pGroup, xui_widget_t* pSelected, int iSelectedIndex, void* pUser) {body(pGroup, pUser);}
static void callback_xui_scroll_frame_change_proc(xui_widget_t* pWidget, float fOffsetX, float fOffsetY, void* pUser) {body(pWidget, pUser);}
static void callback_xui_scroll_view_change_proc(xui_widget_t* pWidget, float fOffsetX, float fOffsetY, void* pUser) {body(pWidget, pUser);}
static void callback_xui_split_layout_change_proc(xui_widget_t* pWidget, int iDivider, void* pUser) {body(pWidget, pUser);}
static void callback_xui_statusbar_select_proc(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser) {body(pWidget, pUser);}
static void callback_xui_table_grid_change_proc(xui_widget_t* pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser) {body(pWidget, pUser);}
static void callback_xui_table_view_column_resize_proc(xui_widget_t* pWidget, int iColumn, float fWidth, void* pUser) {body(pWidget, pUser);}
static void callback_xui_table_view_hover_proc(xui_widget_t* pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser) {body(pWidget, pUser);}
static void callback_xui_table_view_select_proc(xui_widget_t* pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser) {body(pWidget, pUser);}
static void callback_xui_table_view_sort_proc(xui_widget_t* pWidget, int iColumn, int bDescending, void* pUser) {body(pWidget, pUser);}
static void callback_xui_tabs_select_proc(xui_widget_t* pWidget, int iIndex, void* pUser) {body(pWidget, pUser);}
static void callback_xui_tag_input_change_proc(xui_widget_t* pWidget, int iTagCount, void* pUser) {body(pWidget, pUser);}
static void callback_xui_terminal_data_proc(xui_widget_t* pWidget, const uint8_t* pData, int iSize, void* pUser) {body(pWidget, pUser);}
static void callback_xui_terminal_link_proc(xui_widget_t* pWidget, const char* sUrl, void* pUser) {body(pWidget, pUser);}
static void callback_xui_terminal_resize_proc(xui_widget_t* pWidget, int iColumns, int iRows, void* pUser) {body(pWidget, pUser);}
static void callback_xui_terminal_title_proc(xui_widget_t* pWidget, const char* sTitle, void* pUser) {body(pWidget, pUser);}
static void callback_xui_text_edit_change_proc(xui_widget_t* pWidget, const char* sText, void* pUser) {body(pWidget, pUser);}
static void callback_xui_timeline_layer_selected_proc(xui_widget_t* pWidget, int iLayer, void* pUser) {body(pWidget, pUser);}
static void callback_xui_timeline_selection_proc(xui_widget_t* pWidget, void* pUser) {body(pWidget, pUser);}
static void callback_xui_toolbar_select_proc(xui_widget_t* pWidget, int iIndex, int iValue, void* pUser) {body(pWidget, pUser);}
static void callback_xui_tree_view_select_proc(xui_widget_t* pWidget, int iNodeId, void* pUser) {body(pWidget, pUser);}
static void callback_xui_virtual_joystick_change_proc(xui_widget_t* pWidget, const xui_virtual_joystick_state_t* pState, void* pUser) {body(pWidget, pUser);}
static void callback_xui_window_close_proc(xui_widget_t* pWidget, void* pUser) {body(pWidget, pUser);}
static int make(int k,xui_context c,xui_widget* w)
{switch(k){
case 0:return xuiInputCreate(c,w,NULL);
case 1:return xuiTagInputCreate(c,w,NULL);
case 2:return xuiNumericInputCreate(c,w,NULL);
case 3:return xuiNumericInputCreate(c,w,NULL);
case 4:return xuiTextEditCreate(c,w,NULL);
case 5:return xuiRadioGroupCreate(c,w,NULL);
case 6:return xuiVirtualJoystickCreate(c,w,NULL);
case 7:return xuiInventoryGridCreate(c,w,NULL);
case 8:return xuiInventoryGridCreate(c,w,NULL);
case 9:return xuiInventoryGridCreate(c,w,NULL);
case 10:return xuiInventoryGridCreate(c,w,NULL);
case 11:return xuiTerminalCreate(c,w,NULL);
case 12:return xuiTerminalCreate(c,w,NULL);
case 13:return xuiTerminalCreate(c,w,NULL);
case 14:return xuiTerminalCreate(c,w,NULL);
case 15:return xuiSplitLayoutCreate(c,w,NULL);
case 16:return xuiTabsCreate(c,w,NULL);
case 17:return xuiAccordionCreate(c,w,NULL);
case 18:return xuiWindowCreate(c,w,NULL);
case 19:return xuiDockPanelCreate(c,w,NULL);
case 20:return xuiDockPanelCreate(c,w,NULL);
case 21:return xuiDockPanelCreate(c,w,NULL);
case 22:return xuiScrollFrameCreate(c,w,NULL);
case 23:return xuiScrollViewCreate(c,w,NULL);
case 24:return xuiListViewCreate(c,w,NULL);
case 25:return xuiMessageListCreate(c,w,NULL);
case 26:return xuiTreeViewCreate(c,w,NULL);
case 27:return xuiTableViewCreate(c,w,NULL);
case 28:return xuiTableViewCreate(c,w,NULL);
case 29:return xuiTableViewCreate(c,w,NULL);
case 30:return xuiTableViewCreate(c,w,NULL);
case 31:return xuiTableGridCreate(c,w,NULL);
case 32:return xuiTimeLineViewCreate(c,w,NULL);
case 33:return xuiTimeLineViewCreate(c,w,NULL);
case 34:return xuiPropertyGridCreate(c,w,NULL);
case 35:return xuiPropertyGridCreate(c,w,NULL);
case 36:return xuiPopupCreate(c,w,NULL);
case 37:return xuiMenuCreate(c,w,NULL);
case 38:return xuiMenuBarCreate(c,w,NULL);
case 39:return xuiToolbarCreate(c,w,NULL);
case 40:return xuiStatusBarCreate(c,w,NULL);
case 41:return xuiComboBoxCreate(c,w,NULL);
case 42:return xuiComboBoxCreate(c,w,NULL);
case 43:return xuiCascaderCreate(c,w,NULL);
case 44:return xuiColorPickerCreate(c,w,NULL);
case 45:return xuiIconPickerCreate(c,w,NULL);
case 46:return xuiDatePickerCreate(c,w,NULL);
case 47:return xuiDatePickerCreate(c,w,NULL);
case 48:return xuiDatePickerCreate(c,w,NULL);
case 49:return xuiDatePickerCreate(c,w,NULL);
case 50:return xuiDatePickerCreate(c,w,NULL);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static int install(int k,xui_widget w,User* user,const xui_callback_owner_t* ref)
{switch(k){
case 0:return xuiInputSetChangeOwned(w,callback_xui_input_change_proc,user,ref);
case 1:return xuiTagInputSetChangeOwned(w,callback_xui_tag_input_change_proc,user,ref);
case 2:return xuiNumericInputSetChangeOwned(w,callback_xui_numeric_input_change_proc,user,ref);
case 3:return xuiNumericInputSetErrorChangeOwned(w,callback_xui_numeric_input_error_proc,user,ref);
case 4:return xuiTextEditSetChangeOwned(w,callback_xui_text_edit_change_proc,user,ref);
case 5:return xuiRadioGroupSetChangeOwned(w,callback_xui_radio_group_change_proc,user,ref);
case 6:return xuiVirtualJoystickSetChangeOwned(w,callback_xui_virtual_joystick_change_proc,user,ref);
case 7:return xuiInventoryGridSetSelectCallbackOwned(w,callback_xui_inventory_select_proc,user,ref);
case 8:return xuiInventoryGridSetActivateCallbackOwned(w,callback_xui_inventory_activate_proc,user,ref);
case 9:return xuiInventoryGridSetContextCallbackOwned(w,callback_xui_inventory_context_proc,user,ref);
case 10:return xuiInventoryGridSetSplitCallbackOwned(w,callback_xui_inventory_split_proc,user,ref);
case 11:return xuiTerminalSetInputCallbackOwned(w,callback_xui_terminal_data_proc,user,ref);
case 12:return xuiTerminalSetResizeCallbackOwned(w,callback_xui_terminal_resize_proc,user,ref);
case 13:return xuiTerminalSetTitleCallbackOwned(w,callback_xui_terminal_title_proc,user,ref);
case 14:return xuiTerminalSetLinkCallbackOwned(w,callback_xui_terminal_link_proc,user,ref);
case 15:return xuiSplitLayoutSetChangeOwned(w,callback_xui_split_layout_change_proc,user,ref);
case 16:return xuiTabsSetSelectOwned(w,callback_xui_tabs_select_proc,user,ref);
case 17:return xuiAccordionSetSelectOwned(w,callback_xui_accordion_select_proc,user,ref);
case 18:return xuiWindowSetCloseOwned(w,callback_xui_window_close_proc,user,ref);
case 19:return xuiDockPanelSetWindowStateChangedOwned(w,callback_xui_dock_window_state_proc,user,ref);
case 20:return xuiDockPanelSetActiveChangedOwned(w,callback_xui_dock_active_proc,user,ref);
case 21:return xuiDockPanelSetWindowCloseOwned(w,callback_xui_dock_window_close_proc,user,ref);
case 22:return xuiScrollFrameSetChangeOwned(w,callback_xui_scroll_frame_change_proc,user,ref);
case 23:return xuiScrollViewSetChangeOwned(w,callback_xui_scroll_view_change_proc,user,ref);
case 24:return xuiListViewSetSelectOwned(w,callback_xui_list_view_select_proc,user,ref);
case 25:return xuiMessageListSetEventOwned(w,callback_xui_message_list_event_proc,user,ref);
case 26:return xuiTreeViewSetSelectOwned(w,callback_xui_tree_view_select_proc,user,ref);
case 27:return xuiTableViewSetSortOwned(w,callback_xui_table_view_sort_proc,user,ref);
case 28:return xuiTableViewSetSelectOwned(w,callback_xui_table_view_select_proc,user,ref);
case 29:return xuiTableViewSetColumnResizeOwned(w,callback_xui_table_view_column_resize_proc,user,ref);
case 30:return xuiTableViewSetHoverOwned(w,callback_xui_table_view_hover_proc,user,ref);
case 31:return xuiTableGridSetChangeOwned(w,callback_xui_table_grid_change_proc,user,ref);
case 32:return xuiTimeLineViewSetLayerSelectedOwned(w,callback_xui_timeline_layer_selected_proc,user,ref);
case 33:return xuiTimeLineViewSetSelectionChangeOwned(w,callback_xui_timeline_selection_proc,user,ref);
case 34:return xuiPropertyGridSetSelectOwned(w,callback_xui_property_grid_select_proc,user,ref);
case 35:return xuiPropertyGridSetChangeOwned(w,callback_xui_property_grid_change_proc,user,ref);
case 36:return xuiPopupSetChangeOwned(w,callback_xui_popup_change_proc,user,ref);
case 37:return xuiMenuSetSelectOwned(w,callback_xui_menu_select_proc,user,ref);
case 38:return xuiMenuBarSetSelectOwned(w,callback_xui_menu_select_proc,user,ref);
case 39:return xuiToolbarSetSelectOwned(w,callback_xui_toolbar_select_proc,user,ref);
case 40:return xuiStatusBarSetSelectOwned(w,callback_xui_statusbar_select_proc,user,ref);
case 41:return xuiComboBoxSetSelectOwned(w,callback_xui_combobox_select_proc,user,ref);
case 42:return xuiComboBoxSetTextChangeOwned(w,callback_xui_combobox_text_proc,user,ref);
case 43:return xuiCascaderSetChangeOwned(w,callback_xui_cascader_change_proc,user,ref);
case 44:return xuiColorPickerSetChangeOwned(w,callback_xui_color_picker_change_proc,user,ref);
case 45:return xuiIconPickerSetChangeOwned(w,callback_xui_icon_picker_change_proc,user,ref);
case 46:return xuiDatePickerSetChangingOwned(w,callback_xui_date_picker_proc,user,ref);
case 47:return xuiDatePickerSetChangeOwned(w,callback_xui_date_picker_proc,user,ref);
case 48:return xuiDatePickerSetCommitOwned(w,callback_xui_date_picker_proc,user,ref);
case 49:return xuiDatePickerSetCancelOwned(w,callback_xui_date_picker_proc,user,ref);
case 50:return xuiDatePickerSetClearOwned(w,callback_xui_date_picker_proc,user,ref);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static int clear(int k,xui_widget w)
{switch(k){
case 0:return xuiInputSetChange(w,NULL,NULL);
case 1:return xuiTagInputSetChange(w,NULL,NULL);
case 2:return xuiNumericInputSetChange(w,NULL,NULL);
case 3:return xuiNumericInputSetErrorChange(w,NULL,NULL);
case 4:return xuiTextEditSetChange(w,NULL,NULL);
case 5:return xuiRadioGroupSetChange(w,NULL,NULL);
case 6:return xuiVirtualJoystickSetChange(w,NULL,NULL);
case 7:return xuiInventoryGridSetSelectCallback(w,NULL,NULL);
case 8:return xuiInventoryGridSetActivateCallback(w,NULL,NULL);
case 9:return xuiInventoryGridSetContextCallback(w,NULL,NULL);
case 10:return xuiInventoryGridSetSplitCallback(w,NULL,NULL);
case 11:return xuiTerminalSetInputCallback(w,NULL,NULL);
case 12:return xuiTerminalSetResizeCallback(w,NULL,NULL);
case 13:return xuiTerminalSetTitleCallback(w,NULL,NULL);
case 14:return xuiTerminalSetLinkCallback(w,NULL,NULL);
case 15:return xuiSplitLayoutSetChange(w,NULL,NULL);
case 16:return xuiTabsSetSelect(w,NULL,NULL);
case 17:return xuiAccordionSetSelect(w,NULL,NULL);
case 18:return xuiWindowSetClose(w,NULL,NULL);
case 19:return xuiDockPanelSetWindowStateChanged(w,NULL,NULL);
case 20:return xuiDockPanelSetActiveChanged(w,NULL,NULL);
case 21:return xuiDockPanelSetWindowClose(w,NULL,NULL);
case 22:return xuiScrollFrameSetChange(w,NULL,NULL);
case 23:return xuiScrollViewSetChange(w,NULL,NULL);
case 24:return xuiListViewSetSelect(w,NULL,NULL);
case 25:return xuiMessageListSetEvent(w,NULL,NULL);
case 26:return xuiTreeViewSetSelect(w,NULL,NULL);
case 27:return xuiTableViewSetSort(w,NULL,NULL);
case 28:return xuiTableViewSetSelect(w,NULL,NULL);
case 29:return xuiTableViewSetColumnResize(w,NULL,NULL);
case 30:return xuiTableViewSetHover(w,NULL,NULL);
case 31:return xuiTableGridSetChange(w,NULL,NULL);
case 32:return xuiTimeLineViewSetLayerSelected(w,NULL,NULL);
case 33:return xuiTimeLineViewSetSelectionChange(w,NULL,NULL);
case 34:return xuiPropertyGridSetSelect(w,NULL,NULL);
case 35:return xuiPropertyGridSetChange(w,NULL,NULL);
case 36:return xuiPopupSetChange(w,NULL,NULL);
case 37:return xuiMenuSetSelect(w,NULL,NULL);
case 38:return xuiMenuBarSetSelect(w,NULL,NULL);
case 39:return xuiToolbarSetSelect(w,NULL,NULL);
case 40:return xuiStatusBarSetSelect(w,NULL,NULL);
case 41:return xuiComboBoxSetSelect(w,NULL,NULL);
case 42:return xuiComboBoxSetTextChange(w,NULL,NULL);
case 43:return xuiCascaderSetChange(w,NULL,NULL);
case 44:return xuiColorPickerSetChange(w,NULL,NULL);
case 45:return xuiIconPickerSetChange(w,NULL,NULL);
case 46:return xuiDatePickerSetChanging(w,NULL,NULL);
case 47:return xuiDatePickerSetChange(w,NULL,NULL);
case 48:return xuiDatePickerSetCommit(w,NULL,NULL);
case 49:return xuiDatePickerSetCancel(w,NULL,NULL);
case 50:return xuiDatePickerSetClear(w,NULL,NULL);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static void publication(void)
{
    for(int k=0;k<51;++k){
        printf("publication %d\n",k);fflush(stdout);
        xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK);assert(make(k,c,&w)==XUI_OK);
        Owner o={.refs=1,.kind=k,.context=c,.widget=w};User user={0xCE11,&o};xui_callback_owner_t r=descriptor(&o);
        for(int point=0;point<2;++point){assert(xrtMemDebugFailAfter(point));assert(install(k,w,&user,&r)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(o.refs==1);}
        assert(install(k,w,&user,&r)==XUI_OK && o.refs==2);
        xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextTraceOwnership(c,visit,&edges) && edges==1);assert(xrtOwnershipScopeEnd(&freeze));
        o.failHold=true;assert(install(k,w,&user,&r)==XUI_ERROR_CALLBACK_FAILED && o.refs==2);o.failHold=false;
        assert(clear(k,w)==XUI_OK && o.refs==1);
        Owner nested={.refs=1,.kind=k,.context=c,.widget=w};o.other=&nested;o.holdAction=1;
        assert(install(k,w,&user,&r)==XUI_OK && o.refs==2 && nested.refs==1);
        assert(drop(&nested));assert(drop(&o));xuiDestroy(c);assert(o.released==1 && nested.released==1);
    }
}
static int fire(int k,xui_widget w,unsigned serial)
{
    if(k==1)return xuiTagInputAddTag(w,"tag");
    if(k==3){assert(xuiInputSetText(xuiNumericInputGetInputWidget(w),"invalid")==XUI_OK);return xuiNumericInputCommit(w);}
    if(k==6)return xuiVirtualJoystickSetValue(w,serial%2 ? 0.25f:0.75f,0.0f,1);
    if(k==11)return xuiTerminalInputText(w,"typed");
    if(k==13){int r=xuiTerminalWriteText(w,"\033]2;title\007");return r==XUI_OK ? xuiTerminalFlush(w):r;}
    if(k==36)return xuiPopupSetOpen(w,1);
    xui_accessible_node_t node={.iSize=sizeof(node)};assert(xuiWidgetGetAccessibleNode(w,0,&node)==XUI_OK);
    xui_accessible_value_t value={.iSize=sizeof(value),.iType=k==2 ? XUI_ACCESSIBLE_VALUE_NUMBER:XUI_ACCESSIBLE_VALUE_TEXT,.sText="typed",.fNumber=0.75};
    return xuiWidgetPerformAccessibleAction(w,node.iId,XUI_ACCESSIBLE_ACTION_SET_VALUE,&value);
}
static void publishers(void)
{
    const int kinds[]={0,1,2,3,4,6,11,13,36};
    for(size_t j=0;j<sizeof(kinds)/sizeof(*kinds);++j)for(int action=0;action<3;++action){
        int k=kinds[j];printf("publisher %d action %d\n",k,action);fflush(stdout);
        xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK && make(k,c,&w)==XUI_OK);
        Owner o={.refs=1,.kind=k,.context=c,.widget=w,.action=action};User user={0xCE11,&o};xui_callback_owner_t r=descriptor(&o);assert(install(k,w,&user,&r)==XUI_OK);assert(drop(&o));
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);int result=fire(k,w,0);assert(xuiCallScopeEnd(&scope)==XUI_OK);if(result!=XUI_OK || o.calls!=1){fprintf(stderr,"kind=%d status=%d calls=%u\n",k,result,o.calls);}
        assert(result==XUI_OK && o.calls==1);
        if(action!=2)xuiDestroy(c);
        assert(o.released==1);
    }
    /* A no-allocation real publisher, including selected-owner snapshots. */
    xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK && make(6,c,&w)==XUI_OK);
    Owner o={.refs=1,.kind=6,.context=c,.widget=w};User user={0xCE11,&o};xui_callback_owner_t r=descriptor(&o);assert(install(6,w,&user,&r)==XUI_OK);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire(6,w,0)==XUI_OK);
    xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);for(unsigned i=0;i<1000;++i)assert(fire(6,w,i+1)==XUI_OK);xrtMemDebugSnapshot(&b);assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
    o.failHold=true;unsigned calls=o.calls;assert(fire(6,w,1)==XUI_OK && o.calls==calls);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED);o.failHold=false;
    assert(drop(&o));o.action=1;o.failDrop=true;assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire(6,w,0)==XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED);assert(o.released==1);xuiDestroy(c);
}
static void snapshot_reentry(void)
{
    const int kinds[]={0,1,2,3,4,6,11,13,36};
    for(size_t j=0;j<sizeof(kinds)/sizeof(*kinds);++j){
        int k=kinds[j];xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK && make(k,c,&w)==XUI_OK);
        Owner o={.refs=1,.kind=k,.context=c,.widget=w};User user={0xCE11,&o};xui_callback_owner_t r=descriptor(&o);assert(install(k,w,&user,&r)==XUI_OK);assert(drop(&o));o.holdAction=2;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);(void)fire(k,w,0);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_INVALID_STATE);assert(o.calls==0 && o.released==1);
    }
    for(int k=0;k<51;++k){
        xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK && make(k,c,&w)==XUI_OK);
        Owner o={.refs=1,.kind=k,.context=c,.widget=w,.holdAction=2};User user={0xCE11,&o};xui_callback_owner_t r=descriptor(&o);assert(install(k,w,&user,&r)==XUI_ERROR_INVALID_STATE && o.refs==1);assert(drop(&o) && o.released==1);
    }
}
static void menus(void)
{
    for(int action=0;action<3;++action){
        xui_context c=NULL;xui_widget menu=NULL,bar=NULL;assert(xuiCreate(&c)==XUI_OK && make(37,c,&menu)==XUI_OK && make(38,c,&bar)==XUI_OK);
        Owner original={.refs=1,.kind=37,.context=c,.widget=menu},forward={.refs=1,.kind=38,.context=c,.widget=bar,.action=action};
        User a={0xCE11,&original},b={0xCE11,&forward};xui_callback_owner_t ar=descriptor(&original),br=descriptor(&forward);
        assert(install(37,menu,&a,&ar)==XUI_OK && install(38,bar,&b,&br)==XUI_OK);
        xui_menu_item_t item={.sText="entry",.iType=XUI_MENU_ITEM_NORMAL,.iState=XUI_MENU_ITEM_ENABLED,.iValue=42};assert(xuiMenuAddItem(menu,&item)==XUI_OK && xuiMenuBarAddItem(bar,"File",menu,1)==XUI_OK);
        assert(xuiSetRootWidget(c,bar)==XUI_OK && xuiSetViewportSize(c,640,480)==XUI_OK && xuiLayout(c)==XUI_OK);
        assert(drop(&original) && drop(&forward));
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(xuiMenuBarOpenItem(bar,0)==XUI_OK);
        /* Opening a menubar must retain BOTH real callback owners: no raw copy
         * may release the menu owner or turn it into an untraced saved pointer. */
        assert(original.refs==1 && forward.refs==1);
        xui_accessible_node_t node={.iSize=sizeof(node)};assert(xuiWidgetGetAccessibleNode(menu,1,&node)==XUI_OK);
        assert(xuiWidgetPerformAccessibleAction(menu,node.iId,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL)>=XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_OK);
        assert(forward.calls==1 && original.calls==0);
        if(action!=2){assert(xuiMenuBarClose(bar)==XUI_OK);assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(xuiMenuOpenAt(menu,bar,0,24)==XUI_OK);assert(xuiWidgetPerformAccessibleAction(menu,node.iId,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL)>=XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_OK);assert(original.calls==1);xuiDestroy(c);}
        assert(original.released==1 && forward.released==1);
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);publication();publishers();snapshot_reentry();menus();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);puts("51 exactly typed callback publications: OOM, Hold reentry, checked rejection, trace, raw replacement and context retirement passed");return 0;
}
