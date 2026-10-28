/* Actual Windows UI Automation client on an MTA worker, with the provider
 * hosted on its UI STA. Renderer/fonts are the deterministic test proxy. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <oleauto.h>
#include <uiautomation.h>
#include "xui_document_ui.h"
#include "../src/xui/accessibility/xui_native_accessibility_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr,"UIA line %d: %s\n",__LINE__,#c); ExitProcess(1); } } while (0)
#define HR(c) do { HRESULT hr_ = (c); if (FAILED(hr_)) { fprintf(stderr,"UIA line %d: HRESULT %08lx: %s\n",__LINE__,(unsigned long)hr_,#c); ExitProcess(1); } } while (0)
enum { CHANGE_MODE=WM_APP+1, UNDO_DOCUMENT, GET_ACTIVATIONS, DESTROY_EDITOR, RECREATE_EDITOR, DESTROY_CONTEXT, GET_CALLBACKS, SAME_DOCUMENT, REPLACE_DOCUMENT, MAKE_ROLE_MATRIX, GET_MATRIX_CLOSED };
typedef struct probe_data {
    HWND window; xui_context context; xui_widget editor, matrix; xui_document document;
    int activations, callbacks, matrix_selected, matrix_expanded, matrix_scrolled, matrix_closed;
    double matrix_value;
} probe_data;
static const char markdown[]="# \xe6\xa0\x87\xe9\xa2\x98\n\n- [ ] todo\n\n[go](https://example.invalid)\n\n| A | B |\n| - | - |\n| x | y |\n";
typedef struct role_probe { const char* name; int role; int control_type; } role_probe;
static const role_probe role_probes[]={
    {"Focused item",XUI_ACCESSIBLE_ROLE_LIST_ITEM,UIA_ListItemControlTypeId},
    {"Tree",XUI_ACCESSIBLE_ROLE_TREE,UIA_TreeControlTypeId},
    {"Tree item",XUI_ACCESSIBLE_ROLE_TREE_ITEM,UIA_TreeItemControlTypeId},
    {"Menu",XUI_ACCESSIBLE_ROLE_MENU,UIA_MenuControlTypeId},
    {"Menu item",XUI_ACCESSIBLE_ROLE_MENU_ITEM,UIA_MenuItemControlTypeId},
    {"Dialog",XUI_ACCESSIBLE_ROLE_DIALOG,UIA_WindowControlTypeId},
    {"Combo",XUI_ACCESSIBLE_ROLE_COMBOBOX,UIA_ComboBoxControlTypeId},
    {"Slider",XUI_ACCESSIBLE_ROLE_SLIDER,UIA_SliderControlTypeId},
    {"Switch",XUI_ACCESSIBLE_ROLE_SWITCH,UIA_CheckBoxControlTypeId},
    {"Radio",XUI_ACCESSIBLE_ROLE_RADIO_BUTTON,UIA_RadioButtonControlTypeId},
    {"Window",XUI_ACCESSIBLE_ROLE_WINDOW,UIA_WindowControlTypeId},
    {"Scroll bar",XUI_ACCESSIBLE_ROLE_SCROLLBAR,UIA_ScrollBarControlTypeId},
    {"Progress",XUI_ACCESSIBLE_ROLE_PROGRESS,UIA_ProgressBarControlTypeId},
    {"Tool bar",XUI_ACCESSIBLE_ROLE_TOOLBAR,UIA_ToolBarControlTypeId},
    {"Separator",XUI_ACCESSIBLE_ROLE_SEPARATOR,UIA_SeparatorControlTypeId},
    {"Grid",XUI_ACCESSIBLE_ROLE_TABLE,UIA_TableControlTypeId},
    {"Grid cell",XUI_ACCESSIBLE_ROLE_CELL,UIA_DataItemControlTypeId},
    {"Embedded",XUI_ACCESSIBLE_ROLE_EMBEDDED_OBJECT,UIA_PaneControlTypeId}
};
static uint64_t role_id(int role)
{
    size_t i;
    for(i=0;i<sizeof(role_probes)/sizeof(role_probes[0]);i++)
        if(role_probes[i].role==role)return (uint64_t)i+2u;
    return 0;
}
typedef struct uia_event_probe {
    IUIAutomationEventHandler event;
    IUIAutomationPropertyChangedEventHandler property;
    LONG refs,selection_events,range_events,expand_events;
} uia_event_probe;
#define EVENT_PROBE(type,member,value) ((uia_event_probe*)((char*)(value)-offsetof(uia_event_probe,member)))
static HRESULT STDMETHODCALLTYPE event_probe_query(IUIAutomationEventHandler* value,REFIID iid,void** out)
{
    uia_event_probe* probe=EVENT_PROBE(IUIAutomationEventHandler,event,value);
    if(!out)return E_POINTER;
    *out=NULL;
    if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IUIAutomationEventHandler))*out=&probe->event;
    else if(IsEqualIID(iid,&IID_IUIAutomationPropertyChangedEventHandler))*out=&probe->property;
    else return E_NOINTERFACE;
    InterlockedIncrement(&probe->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE event_probe_addref(IUIAutomationEventHandler* value)
{return (ULONG)InterlockedIncrement(&EVENT_PROBE(IUIAutomationEventHandler,event,value)->refs);}
static ULONG STDMETHODCALLTYPE event_probe_release(IUIAutomationEventHandler* value)
{return (ULONG)InterlockedDecrement(&EVENT_PROBE(IUIAutomationEventHandler,event,value)->refs);}
static HRESULT STDMETHODCALLTYPE event_probe_event(IUIAutomationEventHandler* value,IUIAutomationElement* sender,EVENTID event_id)
{
    uia_event_probe* probe=EVENT_PROBE(IUIAutomationEventHandler,event,value);(void)sender;
    if(event_id==UIA_Selection_InvalidatedEventId)InterlockedIncrement(&probe->selection_events);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE property_probe_query(IUIAutomationPropertyChangedEventHandler* value,REFIID iid,void** out)
{return event_probe_query(&EVENT_PROBE(IUIAutomationPropertyChangedEventHandler,property,value)->event,iid,out);}
static ULONG STDMETHODCALLTYPE property_probe_addref(IUIAutomationPropertyChangedEventHandler* value)
{return (ULONG)InterlockedIncrement(&EVENT_PROBE(IUIAutomationPropertyChangedEventHandler,property,value)->refs);}
static ULONG STDMETHODCALLTYPE property_probe_release(IUIAutomationPropertyChangedEventHandler* value)
{return (ULONG)InterlockedDecrement(&EVENT_PROBE(IUIAutomationPropertyChangedEventHandler,property,value)->refs);}
static HRESULT STDMETHODCALLTYPE property_probe_event(IUIAutomationPropertyChangedEventHandler* value,
    IUIAutomationElement* sender,PROPERTYID property_id,VARIANT new_value)
{
    uia_event_probe* probe=EVENT_PROBE(IUIAutomationPropertyChangedEventHandler,property,value);(void)sender;
    if(property_id==UIA_RangeValueValuePropertyId && new_value.vt==VT_R8)
        InterlockedIncrement(&probe->range_events);
    if(property_id==UIA_ExpandCollapseExpandCollapseStatePropertyId && new_value.vt==VT_I4)
        InterlockedIncrement(&probe->expand_events);
    return S_OK;
}
static IUIAutomationEventHandlerVtbl event_probe_vtbl={event_probe_query,event_probe_addref,event_probe_release,event_probe_event};
static IUIAutomationPropertyChangedEventHandlerVtbl property_probe_vtbl={property_probe_query,property_probe_addref,property_probe_release,property_probe_event};
static int role_count(xui_widget widget,void* user)
{(void)widget;(void)user;return (int)(sizeof(role_probes)/sizeof(role_probes[0]))+1;}
static int role_get(xui_widget widget,int index,xui_accessible_node_t* node,void* user)
{
    probe_data* data=user;(void)widget;
    if(index<0 || index>=(int)(sizeof(role_probes)/sizeof(role_probes[0]))+1)return XUI_ERROR_INVALID_ARGUMENT;
    node->iId=(uint64_t)index+1;node->tBounds=(xui_rect_t){0,0,640,480};
    if(!index){
        node->iRole=XUI_ACCESSIBLE_ROLE_LIST;node->sName="Role matrix";
        node->iState|=XUI_ACCESSIBLE_STATE_MULTISELECTABLE|XUI_ACCESSIBLE_STATE_SELECTION_REQUIRED;
        return XUI_OK;
    }
    node->iParentId=1;node->iRole=role_probes[index-1].role;node->sName=role_probes[index-1].name;
    node->iActions=XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
    if(index==1){
        node->iState|=XUI_ACCESSIBLE_STATE_FOCUSED|XUI_ACCESSIBLE_STATE_SELECTABLE;
        if(data->matrix_selected)node->iState|=XUI_ACCESSIBLE_STATE_SELECTED;
        node->iActions|=XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)|
            XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW);
    }else if(node->iRole==XUI_ACCESSIBLE_ROLE_TREE_ITEM){
        node->iState|=data->matrix_expanded?XUI_ACCESSIBLE_STATE_EXPANDED:XUI_ACCESSIBLE_STATE_COLLAPSED;
        node->iActions|=XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND)|
            XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE);
    }else if(node->iRole==XUI_ACCESSIBLE_ROLE_SLIDER){
        node->tRange.iSize=sizeof(node->tRange);node->tRange.fMin=0;node->tRange.fMax=10;
        node->tRange.fValue=data->matrix_value;node->tRange.fStep=.5;node->tRange.fPageStep=2;
        node->iActions|=XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE);
    }else if(node->iRole==XUI_ACCESSIBLE_ROLE_WINDOW){
        node->iActions|=XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_CLOSE);
    }else if(node->iRole==XUI_ACCESSIBLE_ROLE_TABLE){
        node->iRowCount=3;node->iColumnCount=4;
    }else if(node->iRole==XUI_ACCESSIBLE_ROLE_CELL){
        node->iParentId=role_id(XUI_ACCESSIBLE_ROLE_TABLE);
        node->iRow=1;node->iColumn=2;node->iRowCount=1;node->iColumnCount=2;
    }
    return XUI_OK;
}
static int role_action(xui_widget widget,uint64_t id,int action,const void* payload,void* user)
{
    probe_data* data=user;int index=(int)id-2;(void)widget;
    if(action==XUI_ACCESSIBLE_ACTION_FOCUS)return XUI_OK;
    if(index<0 || index>=(int)(sizeof(role_probes)/sizeof(role_probes[0])))return XUI_ERROR_NOT_FOUND;
    if(index==0 && action==XUI_ACCESSIBLE_ACTION_SET_SELECTION){
        data->matrix_selected=1;
        return xuiWidgetNotifyAccessibility(widget,XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED,id);
    }
    if(index==0 && action==XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW){data->matrix_scrolled++;return XUI_OK;}
    if(role_probes[index].role==XUI_ACCESSIBLE_ROLE_TREE_ITEM &&
        (action==XUI_ACCESSIBLE_ACTION_EXPAND || action==XUI_ACCESSIBLE_ACTION_COLLAPSE)){
        data->matrix_expanded=action==XUI_ACCESSIBLE_ACTION_EXPAND;
        return xuiWidgetNotifyAccessibility(widget,XUI_ACCESSIBLE_EVENT_STATE_CHANGED,id);
    }
    if(role_probes[index].role==XUI_ACCESSIBLE_ROLE_SLIDER && action==XUI_ACCESSIBLE_ACTION_SET_VALUE){
        const xui_accessible_value_t* value=payload;
        if(!value || value->iSize<sizeof(*value) || value->iType!=XUI_ACCESSIBLE_VALUE_NUMBER ||
            value->fNumber<0 || value->fNumber>10)return XUI_ERROR_INVALID_ARGUMENT;
        data->matrix_value=value->fNumber;
        return xuiWidgetNotifyAccessibility(widget,XUI_ACCESSIBLE_EVENT_VALUE_CHANGED,id);
    }
    if(role_probes[index].role==XUI_ACCESSIBLE_ROLE_WINDOW && action==XUI_ACCESSIBLE_ACTION_CLOSE){
        data->matrix_closed++;return XUI_OK;
    }
    return XUI_ERROR_UNSUPPORTED;
}
static void activated(xui_widget w, uint64_t id, const char* resource, void* user)
{ probe_data* data=user; (void)w; (void)id; CHECK(!strcmp(resource,"https://example.invalid")); data->activations++; }
static void notified(xui_context context, xui_widget widget, const xui_accessibility_event_t* event, void* user)
{ probe_data* data=user; (void)context; (void)widget; CHECK(event->iSize==sizeof(*event)); data->callbacks++; }
static void make_editor(probe_data* data)
{
    xui_doc_editor_desc_t desc={0}; desc.iSize=sizeof(desc); desc.iMode=XUI_DOC_VISUAL;
    desc.tView.iSize=sizeof(desc.tView); desc.tView.pDocument=data->document;
    desc.tView.onActivate=activated; desc.tView.pUser=data;
    CHECK(xuiDocumentEditorCreate(data->context,&desc,&data->editor)==XUI_OK);
    CHECK(xuiSetRootWidget(data->context,data->editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(data->editor,(xui_rect_t){0,0,640,480})==XUI_OK);
    CHECK(xuiLayout(data->context)==XUI_OK);
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    probe_data* data=(probe_data*)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(data) switch(message) {
    case CHANGE_MODE: return xuiDocumentViewSetMode(data->editor,(uint32_t)wp);
    case UNDO_DOCUMENT: return xuiDocumentUndo(data->document,NULL);
    case SAME_DOCUMENT: return xuiDocumentViewSetDocument(data->editor,data->document);
    case REPLACE_DOCUMENT: {
        xui_doc_desc_t desc={0}; xui_document next=NULL;
        xui_accessible_node_t before={0},after={0};
        before.iSize=after.iSize=sizeof(before);
        CHECK(xuiWidgetGetAccessibleNode(data->editor,0,&before)==XUI_OK);
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&desc,&next)==XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(next,markdown,sizeof(markdown)-1)==XUI_OK);
        CHECK(xuiDocumentViewSetDocument(data->editor,next)==XUI_OK);
        xuiDocumentRelease(data->document);data->document=next;
        CHECK(xuiLayout(data->context)==XUI_OK);
        CHECK(xuiWidgetGetAccessibleNode(data->editor,0,&after)==XUI_OK);
        CHECK(before.iId==after.iId && before.iRole==after.iRole);
        return 0;
    }
    case MAKE_ROLE_MATRIX:
        if(data->editor){xuiWidgetDestroy(data->editor);data->editor=NULL;}
        CHECK(xuiWidgetCreate(data->context,&data->matrix)==XUI_OK);
        data->matrix_selected=1;data->matrix_expanded=0;data->matrix_scrolled=0;data->matrix_closed=0;data->matrix_value=3.5;
        CHECK(xuiWidgetSetAccessibilityProvider(data->matrix,role_count,role_get,role_action,data)==XUI_OK);
        CHECK(xuiSetRootWidget(data->context,data->matrix)==XUI_OK);
        CHECK(xuiWidgetSetRect(data->matrix,(xui_rect_t){0,0,640,480})==XUI_OK);
        CHECK(xuiWidgetSetFocusable(data->matrix,1)==XUI_OK);
        CHECK(xuiSetFocusWidget(data->context,data->matrix)==XUI_OK);
        CHECK(xuiLayout(data->context)==XUI_OK);
        return 0;
    case GET_ACTIVATIONS: return data->activations;
    case GET_MATRIX_CLOSED: return data->matrix_closed;
    case GET_CALLBACKS: CHECK(xuiUpdate(data->context,0)==XUI_OK); return data->callbacks;
    case DESTROY_EDITOR: xuiWidgetDestroy(data->editor); data->editor=NULL; return 0;
    case RECREATE_EDITOR: make_editor(data); return 0;
    case DESTROY_CONTEXT: xuiDestroy(data->context); data->context=NULL; data->editor=NULL; return 0;
    default: break;
    }
    return DefWindowProcW(window,message,wp,lp);
}
static IUIAutomationElement* find(IUIAutomation* client,IUIAutomationElement* root,PROPERTYID id,VARIANT value)
{
    IUIAutomationCondition* condition=NULL; IUIAutomationElement* element=NULL;
    HR(IUIAutomation_CreatePropertyCondition(client,id,value,&condition));
    HR(IUIAutomationElement_FindFirst(root,TreeScope_Descendants,condition,&element));
    IUIAutomationCondition_Release(condition); CHECK(element); return element;
}
static IUIAutomationElement* by_type(IUIAutomation* client,IUIAutomationElement* root,int type)
{ VARIANT value; VariantInit(&value); value.vt=VT_I4; value.lVal=type; return find(client,root,UIA_ControlTypePropertyId,value); }
static IUIAutomationElement* by_name(IUIAutomation* client,IUIAutomationElement* root,const wchar_t* name)
{
    VARIANT value; IUIAutomationElement* result; VariantInit(&value); value.vt=VT_BSTR; value.bstrVal=SysAllocString(name);
    CHECK(value.bstrVal); result=find(client,root,UIA_NamePropertyId,value); VariantClear(&value); return result;
}
static void named(IUIAutomationElement* element,const wchar_t* expected)
{ BSTR value=NULL; HR(IUIAutomationElement_get_CurrentName(element,&value)); CHECK(value && !wcscmp(value,expected)); SysFreeString(value); }
static int different_identity(SAFEARRAY* before,SAFEARRAY* after)
{
    LONG low,high,next_low,next_high,index;
    CHECK(before && after && before->cDims==1 && after->cDims==1);
    HR(SafeArrayGetLBound(before,1,&low));HR(SafeArrayGetUBound(before,1,&high));
    HR(SafeArrayGetLBound(after,1,&next_low));HR(SafeArrayGetUBound(after,1,&next_high));
    if(low!=next_low || high!=next_high)return 1;
    for(index=low;index<=high;index++) {
        LONG old_id,new_id;
        HR(SafeArrayGetElement(before,&index,&old_id));HR(SafeArrayGetElement(after,&index,&new_id));
        if(old_id!=new_id)return 1;
    }
    return 0;
}
static void role_matrix_cases(IUIAutomation* client,IUIAutomationElement* host,HWND window_handle)
{
    IUIAutomationElement *root=by_name(client,host,L"Role matrix"),*focused=NULL;
    PROPERTYID properties[]={UIA_RangeValueValuePropertyId,UIA_ExpandCollapseExpandCollapseStatePropertyId};
    uia_event_probe events={0};BOOL has_focus=TRUE,focusable=FALSE;size_t i;DWORD started;
    events.event.lpVtbl=&event_probe_vtbl;events.property.lpVtbl=&property_probe_vtbl;events.refs=1;
    HR(IUIAutomation_AddAutomationEventHandler(client,UIA_Selection_InvalidatedEventId,root,
        TreeScope_Subtree,NULL,&events.event));
    HR(IUIAutomation_AddPropertyChangedEventHandlerNativeArray(client,root,TreeScope_Subtree,
        NULL,&events.property,properties,(int)(sizeof(properties)/sizeof(properties[0]))));
    HR(IUIAutomationElement_get_CurrentHasKeyboardFocus(root,&has_focus));CHECK(!has_focus);
    for(i=0;i<sizeof(role_probes)/sizeof(role_probes[0]);i++){
        wchar_t name[64];int chars=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,role_probes[i].name,-1,name,64);
        IUIAutomationElement* element;CONTROLTYPEID type=0;CHECK(chars>0);
        element=by_name(client,host,name);HR(IUIAutomationElement_get_CurrentControlType(element,&type));
        CHECK(type==role_probes[i].control_type);
        HR(IUIAutomationElement_get_CurrentIsKeyboardFocusable(element,&focusable));CHECK(focusable);
        HR(IUIAutomationElement_get_CurrentHasKeyboardFocus(element,&has_focus));CHECK(has_focus==(i==0));
        if(i==0)focused=element;else IUIAutomationElement_Release(element);
    }
    CHECK(focused);
    {
        IUIAutomationSelectionPattern* selection=NULL;IUIAutomationElementArray* selected=NULL;
        BOOL multiple=FALSE,required=FALSE;int length=0;
        HR(IUIAutomationElement_GetCurrentPattern(root,UIA_SelectionPatternId,(IUnknown**)&selection));CHECK(selection);
        HR(IUIAutomationSelectionPattern_get_CurrentCanSelectMultiple(selection,&multiple));CHECK(multiple);
        HR(IUIAutomationSelectionPattern_get_CurrentIsSelectionRequired(selection,&required));CHECK(required);
        HR(IUIAutomationSelectionPattern_GetCurrentSelection(selection,&selected));CHECK(selected);
        HR(IUIAutomationElementArray_get_Length(selected,&length));CHECK(length==1);
        IUIAutomationElementArray_Release(selected);IUIAutomationSelectionPattern_Release(selection);
    }
    {
        IUIAutomationSelectionItemPattern* selection=NULL;IUIAutomationScrollItemPattern* scroll=NULL;
        BOOL selected=FALSE;
        HR(IUIAutomationElement_GetCurrentPattern(focused,UIA_SelectionItemPatternId,(IUnknown**)&selection));CHECK(selection);
        HR(IUIAutomationSelectionItemPattern_get_CurrentIsSelected(selection,&selected));CHECK(selected);
        HR(IUIAutomationSelectionItemPattern_Select(selection));
        HR(IUIAutomationElement_GetCurrentPattern(focused,UIA_ScrollItemPatternId,(IUnknown**)&scroll));CHECK(scroll);
        HR(IUIAutomationScrollItemPattern_ScrollIntoView(scroll));
        IUIAutomationScrollItemPattern_Release(scroll);IUIAutomationSelectionItemPattern_Release(selection);
    }
    {
        IUIAutomationElement* tree_item=by_name(client,host,L"Tree item");
        IUIAutomationExpandCollapsePattern* expand=NULL;enum ExpandCollapseState state;
        HR(IUIAutomationElement_GetCurrentPattern(tree_item,UIA_ExpandCollapsePatternId,(IUnknown**)&expand));CHECK(expand);
        HR(IUIAutomationExpandCollapsePattern_get_CurrentExpandCollapseState(expand,&state));CHECK(state==ExpandCollapseState_Collapsed);
        HR(IUIAutomationExpandCollapsePattern_Expand(expand));
        HR(IUIAutomationExpandCollapsePattern_get_CurrentExpandCollapseState(expand,&state));CHECK(state==ExpandCollapseState_Expanded);
        HR(IUIAutomationExpandCollapsePattern_Collapse(expand));
        IUIAutomationExpandCollapsePattern_Release(expand);IUIAutomationElement_Release(tree_item);
    }
    {
        IUIAutomationElement* slider=by_name(client,host,L"Slider");IUIAutomationRangeValuePattern* range=NULL;
        double value=0,minimum=0,maximum=0,small=0,large=0;BOOL readonly=TRUE;
        HR(IUIAutomationElement_GetCurrentPattern(slider,UIA_RangeValuePatternId,(IUnknown**)&range));CHECK(range);
        HR(IUIAutomationRangeValuePattern_get_CurrentValue(range,&value));
        HR(IUIAutomationRangeValuePattern_get_CurrentMinimum(range,&minimum));
        HR(IUIAutomationRangeValuePattern_get_CurrentMaximum(range,&maximum));
        HR(IUIAutomationRangeValuePattern_get_CurrentSmallChange(range,&small));
        HR(IUIAutomationRangeValuePattern_get_CurrentLargeChange(range,&large));
        HR(IUIAutomationRangeValuePattern_get_CurrentIsReadOnly(range,&readonly));
        CHECK(value==3.5 && minimum==0 && maximum==10 && small==.5 && large==2 && !readonly);
        HR(IUIAutomationRangeValuePattern_SetValue(range,7.5));
        HR(IUIAutomationRangeValuePattern_get_CurrentValue(range,&value));CHECK(value==7.5);
        IUIAutomationRangeValuePattern_Release(range);IUIAutomationElement_Release(slider);
    }
    {
        IUIAutomationElement* window=by_name(client,host,L"Window");IUIAutomationWindowPattern* pattern=NULL;
        BOOL value=TRUE;enum WindowVisualState visual;enum WindowInteractionState interaction;
        HR(IUIAutomationElement_GetCurrentPattern(window,UIA_WindowPatternId,(IUnknown**)&pattern));CHECK(pattern);
        HR(IUIAutomationWindowPattern_get_CurrentCanMaximize(pattern,&value));CHECK(!value);
        HR(IUIAutomationWindowPattern_get_CurrentCanMinimize(pattern,&value));CHECK(!value);
        HR(IUIAutomationWindowPattern_get_CurrentIsModal(pattern,&value));CHECK(!value);
        HR(IUIAutomationWindowPattern_get_CurrentIsTopmost(pattern,&value));CHECK(!value);
        HR(IUIAutomationWindowPattern_get_CurrentWindowVisualState(pattern,&visual));CHECK(visual==WindowVisualState_Normal);
        HR(IUIAutomationWindowPattern_get_CurrentWindowInteractionState(pattern,&interaction));
        CHECK(interaction==WindowInteractionState_ReadyForUserInteraction);
        HR(IUIAutomationWindowPattern_WaitForInputIdle(pattern,100,&value));CHECK(value);
        HR(IUIAutomationWindowPattern_Close(pattern));
        IUIAutomationWindowPattern_Release(pattern);IUIAutomationElement_Release(window);
    }
    {
        IUIAutomationElement *grid=by_name(client,host,L"Grid"),*cell=by_name(client,host,L"Grid cell");
        IUIAutomationElement *item=NULL,*span_item=NULL,*container=NULL;
        IUIAutomationGridPattern* pattern=NULL;IUIAutomationGridItemPattern* item_pattern=NULL;
        BOOL same=FALSE;int row_count=0,column_count=0,row=0,column=0,row_span=0,column_span=0;
        HR(IUIAutomationElement_GetCurrentPattern(grid,UIA_GridPatternId,(IUnknown**)&pattern));CHECK(pattern);
        HR(IUIAutomationGridPattern_get_CurrentRowCount(pattern,&row_count));CHECK(row_count==3);
        HR(IUIAutomationGridPattern_get_CurrentColumnCount(pattern,&column_count));CHECK(column_count==4);
        HR(IUIAutomationGridPattern_GetItem(pattern,1,2,&item));CHECK(item);
        HR(IUIAutomation_CompareElements(client,item,cell,&same));CHECK(same);
        HR(IUIAutomationGridPattern_GetItem(pattern,1,3,&span_item));CHECK(span_item);
        same=FALSE;HR(IUIAutomation_CompareElements(client,span_item,cell,&same));CHECK(same);
        CHECK(IUIAutomationGridPattern_GetItem(pattern,3,0,&container)==E_INVALIDARG && !container);
        HR(IUIAutomationElement_GetCurrentPattern(cell,UIA_GridItemPatternId,(IUnknown**)&item_pattern));CHECK(item_pattern);
        HR(IUIAutomationGridItemPattern_get_CurrentRow(item_pattern,&row));CHECK(row==1);
        HR(IUIAutomationGridItemPattern_get_CurrentColumn(item_pattern,&column));CHECK(column==2);
        HR(IUIAutomationGridItemPattern_get_CurrentRowSpan(item_pattern,&row_span));CHECK(row_span==1);
        HR(IUIAutomationGridItemPattern_get_CurrentColumnSpan(item_pattern,&column_span));CHECK(column_span==2);
        HR(IUIAutomationGridItemPattern_get_CurrentContainingGrid(item_pattern,&container));CHECK(container);
        same=FALSE;HR(IUIAutomation_CompareElements(client,container,grid,&same));CHECK(same);
        IUIAutomationElement_Release(container);IUIAutomationGridItemPattern_Release(item_pattern);
        IUIAutomationElement_Release(span_item);IUIAutomationElement_Release(item);
        IUIAutomationGridPattern_Release(pattern);IUIAutomationElement_Release(cell);IUIAutomationElement_Release(grid);
    }
    started=GetTickCount();
    while((events.selection_events<1 || events.range_events<1 || events.expand_events<2) &&
        GetTickCount()-started<5000)Sleep(10);
    CHECK(events.selection_events>=1 && events.range_events>=1 && events.expand_events>=2);
    CHECK(SendMessageW(window_handle,GET_MATRIX_CLOSED,0,0)==1);
    HR(IUIAutomation_RemoveAutomationEventHandler(client,UIA_Selection_InvalidatedEventId,root,&events.event));
    HR(IUIAutomation_RemovePropertyChangedEventHandler(client,root,&events.property));
    IUIAutomationElement_Release(focused);IUIAutomationElement_Release(root);
    puts("Actual UIA: roles, virtual focus, Selection/SelectionItem, Window, Grid/GridItem, RangeValue, ExpandCollapse, ScrollItem and events passed");fflush(stdout);
}
static void text_pattern_cases(IUIAutomationElement* document,IUIAutomationElement* heading,
    IUIAutomationTextRange** retained)
{
    IUIAutomationTextPattern* pattern=NULL;
    IUIAutomationTextRange *range=NULL,*clone=NULL,*found=NULL,*child=NULL,*point_range=NULL,*selected=NULL;
    IUIAutomationTextRangeArray *visible=NULL,*selection=NULL;
    IUIAutomationElementArray* children=NULL;
    IUIAutomationElement* enclosing=NULL;
    enum SupportedTextSelection supported=SupportedTextSelection_None;
    RECT bounds={0};POINT point;BSTR text=NULL,needle=NULL;SAFEARRAY* rectangles=NULL;
    VARIANT attribute;BOOL same=FALSE;int length=0,moved=0;
    HR(IUIAutomationElement_GetCurrentPattern(document,UIA_TextPatternId,(IUnknown**)&pattern));CHECK(pattern);
    HR(IUIAutomationTextPattern_get_SupportedTextSelection(pattern,&supported));
    CHECK(supported==SupportedTextSelection_Single);
    HR(IUIAutomationTextPattern_get_DocumentRange(pattern,&range));CHECK(range);
    HR(IUIAutomationTextRange_GetText(range,-1,&text));
    CHECK(text && wcsstr(text,L"\x6807\x9898") && wcsstr(text,L"todo"));
    SysFreeString(text);text=NULL;
    HR(IUIAutomationTextRange_GetText(range,2,&text));CHECK(text && SysStringLen(text)<=2);
    SysFreeString(text);text=NULL;
    HR(IUIAutomationTextRange_Clone(range,&clone));CHECK(clone);
    HR(IUIAutomationTextRange_Compare(range,clone,&same));CHECK(same);
    HR(IUIAutomationTextRange_GetAttributeValue(range,UIA_IsReadOnlyAttributeId,&attribute));
    CHECK(attribute.vt==VT_BOOL && attribute.boolVal==VARIANT_FALSE);VariantClear(&attribute);
    VariantInit(&attribute);attribute.vt=VT_BOOL;attribute.boolVal=VARIANT_FALSE;
    HR(IUIAutomationTextRange_FindAttribute(range,UIA_IsReadOnlyAttributeId,attribute,FALSE,&selected));
    CHECK(selected);IUIAutomationTextRange_Release(selected);selected=NULL;
    needle=SysAllocString(L"\x6807\x9898");CHECK(needle);
    HR(IUIAutomationTextRange_FindText(range,needle,FALSE,FALSE,&found));CHECK(found);
    SysFreeString(needle);needle=NULL;
    HR(IUIAutomationTextRange_GetText(found,-1,&text));CHECK(text && !wcscmp(text,L"\x6807\x9898"));
    SysFreeString(text);text=NULL;
    HR(IUIAutomationTextRange_GetBoundingRectangles(found,&rectangles));
    CHECK(rectangles && rectangles->cDims==1 && rectangles->rgsabound[0].cElements>=4);
    SafeArrayDestroy(rectangles);rectangles=NULL;
    HR(IUIAutomationTextRange_GetEnclosingElement(found,&enclosing));CHECK(enclosing);
    HR(IUIAutomationElement_get_CurrentName(enclosing,&text));CHECK(text && !wcscmp(text,L"Document"));
    SysFreeString(text);text=NULL;IUIAutomationElement_Release(enclosing);enclosing=NULL;
    HR(IUIAutomationTextRange_GetChildren(range,&children));CHECK(children);
    HR(IUIAutomationElementArray_get_Length(children,&length));CHECK(length>0);
    IUIAutomationElementArray_Release(children);children=NULL;
    HR(IUIAutomationTextPattern_RangeFromChild(pattern,heading,&child));CHECK(child);
    HR(IUIAutomationTextRange_GetText(child,-1,&text));CHECK(text && wcsstr(text,L"\x6807\x9898"));
    SysFreeString(text);text=NULL;
    HR(IUIAutomationElement_get_CurrentBoundingRectangle(heading,&bounds));
    point.x=bounds.left+(bounds.right-bounds.left)/2;
    point.y=bounds.top+(bounds.bottom-bounds.top)/2;
    HR(IUIAutomationTextPattern_RangeFromPoint(pattern,point,&point_range));CHECK(point_range);
    HR(IUIAutomationTextRange_ExpandToEnclosingUnit(point_range,TextUnit_Character));
    HR(IUIAutomationTextRange_GetText(point_range,-1,&text));CHECK(text && SysStringLen(text)>0);
    SysFreeString(text);text=NULL;
    HR(IUIAutomationTextRange_Clone(found,&selected));CHECK(selected);
    HR(IUIAutomationTextRange_MoveEndpointByUnit(selected,TextPatternRangeEndpoint_End,
        TextUnit_Character,-1,&moved));CHECK(moved==-1);
    HR(IUIAutomationTextRange_GetText(selected,-1,&text));CHECK(text && !wcscmp(text,L"\x6807"));
    SysFreeString(text);text=NULL;IUIAutomationTextRange_Release(selected);selected=NULL;
    HR(IUIAutomationTextRange_Select(found));
    HR(IUIAutomationTextPattern_GetSelection(pattern,&selection));CHECK(selection);
    HR(IUIAutomationTextRangeArray_get_Length(selection,&length));CHECK(length==1);
    HR(IUIAutomationTextRangeArray_GetElement(selection,0,&selected));CHECK(selected);
    HR(IUIAutomationTextRange_GetText(selected,-1,&text));CHECK(text && !wcscmp(text,L"\x6807\x9898"));
    SysFreeString(text);text=NULL;
    HR(IUIAutomationTextRange_ScrollIntoView(selected,TRUE));
    IUIAutomationTextRange_Release(selected);selected=NULL;
    IUIAutomationTextRangeArray_Release(selection);selection=NULL;
    HR(IUIAutomationTextPattern_GetVisibleRanges(pattern,&visible));CHECK(visible);
    HR(IUIAutomationTextRangeArray_get_Length(visible,&length));CHECK(length==1);
    IUIAutomationTextRangeArray_Release(visible);visible=NULL;
    *retained=clone;clone=NULL;
    IUIAutomationTextRange_Release(point_range);IUIAutomationTextRange_Release(child);
    IUIAutomationTextRange_Release(found);IUIAutomationTextRange_Release(range);
    IUIAutomationTextPattern_Release(pattern);
    puts("Actual UIA: TextPattern UTF-16 text, ranges, search, movement, selection, geometry and children passed");fflush(stdout);
}
static DWORD WINAPI client_thread(void* user)
{
    probe_data* data=user; IUIAutomation* client=NULL; IUIAutomationElement *host=NULL,*document=NULL,*heading=NULL,*task=NULL,*link=NULL,*source=NULL;
    IUIAutomationTogglePattern* toggle=NULL; IUIAutomationInvokePattern* invoke=NULL; IUIAutomationValuePattern* value=NULL;
    IUIAutomationTextRange* retained_text=NULL;
    enum ToggleState state; BSTR text=NULL; HRESULT stale; SAFEARRAY* identity=NULL; BOOL same=FALSE;
    HR(CoInitializeEx(NULL,COINIT_MULTITHREADED));
    HR(CoCreateInstance(&CLSID_CUIAutomation,NULL,CLSCTX_INPROC_SERVER,&IID_IUIAutomation,(void**)&client));
    HR(IUIAutomation_ElementFromHandle(client,data->window,&host)); named(host,L"XUI");
    document=by_type(client,host,UIA_DocumentControlTypeId); named(document,L"Document");
    heading=by_name(client,host,L"\x6807\x9898");
    text_pattern_cases(document,heading,&retained_text);CHECK(retained_text);
    {
        IUIAutomationElement *grid=by_type(client,document,UIA_TableControlTypeId),*cell=NULL,*container=NULL;
        IUIAutomationGridPattern* grid_pattern=NULL;IUIAutomationGridItemPattern* cell_pattern=NULL;
        BOOL same_grid=FALSE;int rows=0,columns=0,row=0,column=0;
        HR(IUIAutomationElement_GetCurrentPattern(grid,UIA_GridPatternId,(IUnknown**)&grid_pattern));CHECK(grid_pattern);
        HR(IUIAutomationGridPattern_get_CurrentRowCount(grid_pattern,&rows));CHECK(rows==2);
        HR(IUIAutomationGridPattern_get_CurrentColumnCount(grid_pattern,&columns));CHECK(columns==2);
        HR(IUIAutomationGridPattern_GetItem(grid_pattern,1,1,&cell));CHECK(cell);
        HR(IUIAutomationElement_GetCurrentPattern(cell,UIA_GridItemPatternId,(IUnknown**)&cell_pattern));CHECK(cell_pattern);
        HR(IUIAutomationGridItemPattern_get_CurrentRow(cell_pattern,&row));CHECK(row==1);
        HR(IUIAutomationGridItemPattern_get_CurrentColumn(cell_pattern,&column));CHECK(column==1);
        HR(IUIAutomationGridItemPattern_get_CurrentContainingGrid(cell_pattern,&container));CHECK(container);
        HR(IUIAutomation_CompareElements(client,container,grid,&same_grid));CHECK(same_grid);
        IUIAutomationElement_Release(container);IUIAutomationGridItemPattern_Release(cell_pattern);
        IUIAutomationElement_Release(cell);IUIAutomationGridPattern_Release(grid_pattern);IUIAutomationElement_Release(grid);
    }
    HR(IUIAutomationElement_GetRuntimeId(heading,&identity)); CHECK(identity && identity->cDims==1);
    task=by_type(client,host,UIA_CheckBoxControlTypeId);
    HR(IUIAutomationElement_GetCurrentPattern(task,UIA_TogglePatternId,(IUnknown**)&toggle)); CHECK(toggle);
    HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_Off);
    HR(IUIAutomationTogglePattern_Toggle(toggle)); HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_On);
    CHECK(SendMessageW(data->window,UNDO_DOCUMENT,0,0)==XUI_OK);
    HR(IUIAutomationTogglePattern_get_CurrentToggleState(toggle,&state)); CHECK(state==ToggleState_Off);
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      HR(IUIAutomation_CompareElements(client,heading,again,&same)); CHECK(same); IUIAutomationElement_Release(again); }
    link=by_type(client,host,UIA_HyperlinkControlTypeId); named(link,L"go");
    HR(IUIAutomationElement_GetCurrentPattern(link,UIA_InvokePatternId,(IUnknown**)&invoke)); CHECK(invoke);
    HR(IUIAutomationInvokePattern_Invoke(invoke)); CHECK(SendMessageW(data->window,GET_ACTIVATIONS,0,0)==1);
    puts("Actual UIA: semantic Document/heading/link/task/table nodes, stable identity, Grid, Toggle/Undo and Invoke passed"); fflush(stdout);
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_SOURCE_TEXT,0)==XUI_OK);
    source=by_type(client,host,UIA_EditControlTypeId);
    HR(IUIAutomationElement_GetCurrentPattern(source,UIA_ValuePatternId,(IUnknown**)&value)); CHECK(value);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text));
    CHECK(text && wcsstr(text,L"# \x6807\x9898") && wcsstr(text,L"[go](https://example.invalid)")); SysFreeString(text); text=NULL;
    HR(IUIAutomationValuePattern_SetValue(value,L"# \x4f60\x597d \xd83d\xde00\n"));
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text)); CHECK(text && !wcscmp(text,L"# \x4f60\x597d \xd83d\xde00\n")); SysFreeString(text); text=NULL;
    CHECK(SendMessageW(data->window,UNDO_DOCUMENT,0,0)==XUI_OK);
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_LIVE_MARKDOWN,0)==XUI_OK);
    HR(IUIAutomationValuePattern_get_CurrentValue(value,&text)); CHECK(text && wcsstr(text,L"# \x6807\x9898")); SysFreeString(text); text=NULL;
    puts("Actual UIA: SOURCE/LIVE raw Markdown, UTF-16 CJK/surrogate pair SetValue and shared Undo passed"); fflush(stdout);
    IUIAutomationValuePattern_Release(value); value=NULL;
    IUIAutomationElement_Release(source); source=NULL;
    CHECK(SendMessageW(data->window,CHANGE_MODE,XUI_DOC_VISUAL,0)==XUI_OK);
    HR(IUIAutomationTextRange_GetText(retained_text,-1,&text));
    CHECK(text && wcsstr(text,L"\x6807\x9898"));SysFreeString(text);text=NULL;
    CHECK(SendMessageW(data->window,GET_CALLBACKS,0,0)>0);
    CHECK(SendMessageW(data->window,SAME_DOCUMENT,0,0)==XUI_OK);
    named(heading,L"\x6807\x9898");
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      HR(IUIAutomation_CompareElements(client,heading,again,&same));CHECK(same);IUIAutomationElement_Release(again); }
    CHECK(SendMessageW(data->window,REPLACE_DOCUMENT,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(document,&text);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    stale=IUIAutomationElement_get_CurrentName(heading,&text);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    stale=IUIAutomationTextRange_GetText(retained_text,-1,&text);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    IUIAutomationTextRange_Release(retained_text);retained_text=NULL;
    stale=IUIAutomationTogglePattern_Toggle(toggle);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);
    stale=IUIAutomationInvokePattern_Invoke(invoke);
    CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);
    IUIAutomationInvokePattern_Release(invoke);invoke=NULL;
    IUIAutomationTogglePattern_Release(toggle);toggle=NULL;
    IUIAutomationElement_Release(link);link=NULL;IUIAutomationElement_Release(task);task=NULL;
    IUIAutomationElement_Release(heading);IUIAutomationElement_Release(document);
    document=by_type(client,host,UIA_DocumentControlTypeId);named(document,L"Document");
    heading=by_name(client,host,L"\x6807\x9898");
    { SAFEARRAY* next_identity=NULL;
      HR(IUIAutomationElement_GetRuntimeId(heading,&next_identity));CHECK(different_identity(identity,next_identity));
      SafeArrayDestroy(identity);identity=next_identity; }
    { IUIAutomationElement* current_task=by_type(client,host,UIA_CheckBoxControlTypeId);
      IUIAutomationTogglePattern* current_toggle=NULL;
      HR(IUIAutomationElement_GetCurrentPattern(current_task,UIA_TogglePatternId,(IUnknown**)&current_toggle));CHECK(current_toggle);
      HR(IUIAutomationTogglePattern_get_CurrentToggleState(current_toggle,&state));CHECK(state==ToggleState_Off);
      IUIAutomationTogglePattern_Release(current_toggle);IUIAutomationElement_Release(current_task); }
    CHECK(SendMessageW(data->window,GET_ACTIVATIONS,0,0)==1);
    puts("Actual UIA: same Document retains identity; replacement reuses semantic root id but invalidates old nodes/actions and gets new runtime ids");fflush(stdout);
    CHECK(SendMessageW(data->window,DESTROY_EDITOR,0,0)==0);
    CHECK(SendMessageW(data->window,RECREATE_EDITOR,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(heading,&text); CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE); if(text)SysFreeString(text);text=NULL;
    { IUIAutomationElement* again=by_name(client,host,L"\x6807\x9898");
      SAFEARRAY* next_identity=NULL;
      HR(IUIAutomationElement_GetRuntimeId(again,&next_identity));CHECK(different_identity(identity,next_identity));
      SafeArrayDestroy(identity);SafeArrayDestroy(next_identity);IUIAutomationElement_Release(again); }
    CHECK(SendMessageW(data->window,MAKE_ROLE_MATRIX,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(heading,&text);CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE);if(text)SysFreeString(text);text=NULL;
    role_matrix_cases(client,host,data->window);
    CHECK(SendMessageW(data->window,DESTROY_CONTEXT,0,0)==0);
    stale=IUIAutomationElement_get_CurrentName(document,&text); CHECK(stale==(HRESULT)UIA_E_ELEMENTNOTAVAILABLE); if(text)SysFreeString(text);
    puts("Actual UIA: user callback retained; destroyed widget/context unavailable; recreation has a new identity passed"); fflush(stdout);
    IUIAutomationElement_Release(heading);
    IUIAutomationElement_Release(document); IUIAutomationElement_Release(host); IUIAutomation_Release(client);
    CoUninitialize(); return 0;
}
static DWORD WINAPI rebind_thread(void* user)
{
    HINSTANCE instance=GetModuleHandleW(NULL);xui_context context=NULL;xui_widget root=NULL;HWND window;
    int result=XUI_ERROR_INVALID_STATE;(void)user;
    window=CreateWindowW(L"XUI.NativeAccessibility.Probe",L"XUI rebind",WS_OVERLAPPEDWINDOW,
        0,0,320,240,NULL,NULL,instance,NULL);
    if(!window)return 1;
    if(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&root)==XUI_OK &&
        xuiSetRootWidget(context,root)==XUI_OK && xuiSetViewportSize(context,320,240)==XUI_OK)
        result=xuiInternalNativeAccessibilityBind(context,window,(xui_rect_t){0,0,320,240});
    if(context)xuiDestroy(context);
    DestroyWindow(window);
    return result==XUI_OK?0:1;
}
int main(void)
{
    probe_data data={0}; xui_test_proxy_state_t proxy; xui_font font=NULL; xui_doc_desc_t desc={0};
    WNDCLASSW type={0}; HANDLE thread; DWORD started=GetTickCount(),result=1; MSG message;
    HR(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED));
    type.lpfnWndProc=window_proc; type.hInstance=GetModuleHandleW(NULL); type.lpszClassName=L"XUI.NativeAccessibility.Probe";
    CHECK(RegisterClassW(&type)); data.window=CreateWindowW(type.lpszClassName,L"XUI Document UIA probe",WS_OVERLAPPEDWINDOW,0,0,800,600,NULL,NULL,type.hInstance,NULL); CHECK(data.window);
    SetWindowLongPtrW(data.window,GWLP_USERDATA,(LONG_PTR)&data);
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&data.context)==XUI_OK);
    CHECK(xuiSetProxy(data.context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(data.context,font)==XUI_OK);
    CHECK(xuiSetViewportSize(data.context,640,480)==XUI_OK); desc.iSize=sizeof(desc); desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc,&data.document)==XUI_OK); CHECK(xuiDocumentLoadMarkdown(data.document,markdown,sizeof(markdown)-1)==XUI_OK);
    make_editor(&data); CHECK(xuiSetAccessibilityEventCallback(data.context,notified,&data)==XUI_OK);
    CHECK(xuiInternalNativeAccessibilityBind(data.context,data.window,(xui_rect_t){0,0,640,480})==XUI_OK);
    thread=CreateThread(NULL,0,client_thread,&data,0,NULL);CHECK(thread);
    while(WaitForSingleObject(thread,0)==WAIT_TIMEOUT) {
        CHECK(GetTickCount()-started<60000);
        (void)MsgWaitForMultipleObjects(1,&thread,FALSE,50,QS_ALLINPUT);
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
    CHECK(GetExitCodeThread(thread,&result));CloseHandle(thread);
    CHECK(result==0);
    thread=CreateThread(NULL,0,rebind_thread,NULL,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,10000)==WAIT_OBJECT_0);
    CHECK(GetExitCodeThread(thread,&result));CloseHandle(thread);CHECK(result==0);
    puts("Native UIA bridge can rebind on a different UI thread after final detach.");
    if(data.context)xuiDestroy(data.context);
    xuiDocumentRelease(data.document);proxy.tProxy.fontDestroy(&proxy.tProxy,font);DestroyWindow(data.window);
    UnregisterClassW(type.lpszClassName,type.hInstance);CoUninitialize();
    puts("Native UIA bridge integration including TextPattern passed; physical screen-reader acceptance remains incomplete.");return 0;
}
