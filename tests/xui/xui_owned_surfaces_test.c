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
typedef struct SurfacePayload {
    unsigned* finalized;int status;bool secondary;
    xui_widget same,other; xui_surface replacement; int* sameStatus;int* otherStatus;
    xui_widget indicators;xui_surface expected;
} SurfacePayload;
static int finalizeSurface(xui_proxy proxy,void* raw)
{
    (void)proxy;SurfacePayload* payload=raw;++*payload->finalized;
    if(payload->sameStatus)*payload->sameStatus=xuiImageSetSurface(payload->same,payload->replacement);
    if(payload->otherStatus)*payload->otherStatus=xuiImageSetSurface(payload->other,payload->replacement);
    if(payload->indicators){
        xui_surface left=NULL,right=NULL;
        assert(xuiCheckBoxGetIndicatorSurface(payload->indicators,&left,NULL,&right,NULL)==XUI_OK);
        assert(left==payload->expected && right==payload->expected);
    }
    if(payload->secondary)xrtSetErrorKind(XERR_STATE);
    return payload->status;
}
static xui_surface surfaceCreate(xui_proxy proxy,unsigned* finalized,int status)
{
    xui_surface surface=NULL;void* raw=NULL;
    assert(xuiSurfaceCreateOwned(proxy,sizeof(SurfacePayload),finalizeSurface,&surface,&raw)==XUI_OK);
    *(SurfacePayload*)raw=(SurfacePayload){.finalized=finalized,.status=status};return surface;
}
static size_t refs(xui_surface surface)
{
    size_t value=0;xrtownershipref ref=xuiSurfaceOwnership(surface);xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze));assert(ref.Ops && ref.Ops->Count(ref.Data,&value));
    assert(xrtOwnershipScopeEnd(&freeze));return value;
}
static int surfaceDesc(xui_proxy proxy,xui_surface surface,xui_surface_desc_t* out)
{(void)proxy;if(!surface || !out)return XUI_ERROR_INVALID_ARGUMENT;*out=(xui_surface_desc_t){.iWidth=32,.iHeight=32};return XUI_OK;}
static int generation(xui_proxy proxy,xui_surface surface,uint32_t* out)
{(void)proxy;if(!surface || !out)return XUI_ERROR_INVALID_ARGUMENT;*out=1;return XUI_OK;}
static xui_proxy proxyCreate(xui_test_proxy_state_t* state,Owner* owner)
{
    xuiTestProxyInit(state);state->tProxy.surfaceGetDesc=surfaceDesc;state->tProxy.surfaceGetGeneration=generation;
    xui_callback_owner_t binding=ownerRef(owner);xui_proxy proxy=NULL;
    assert(xuiProxyCreateOwned(&state->tProxy,&binding,&proxy)==XUI_OK);return proxy;
}
static const xui_rect_t rect={0,0,16,16};
static xui_nine_patch_t patch(xui_surface surface)
{return (xui_nine_patch_t){.iSize=sizeof(xui_nine_patch_t),.pSurface=surface,.tSrc={0,0,16,16},.iMode=XUI_NINE_PATCH_STRETCH,.iColor=XUI_COLOR_WHITE};}
static int setImage(xui_widget w,xui_surface s){return xuiImageSetSurface(w,s);}
static int setButton(xui_widget w,xui_surface s)
{
    int result=xuiButtonSetIcon(w,s,rect);if(result!=XUI_OK)return result;
    result=xuiButtonSetBadgeSurface(w,s,rect);if(result!=XUI_OK)return result;
    static const uint32_t states[]={0,XUI_WIDGET_STATE_HOVER,XUI_WIDGET_STATE_ACTIVE,XUI_WIDGET_STATE_FOCUS,XUI_WIDGET_STATE_DISABLED,XUI_BUTTON_STATE_CHECKED};
    xui_nine_patch_t p=patch(s);for(size_t i=0;i<6;++i){result=xuiButtonSetPatch(w,states[i],s ? &p:NULL);if(result!=XUI_OK)return result;}return result;
}
static int setCheck(xui_widget w,xui_surface s)
{int result=xuiCheckBoxSetIndicatorSurface(w,s,rect,s,rect);return result==XUI_OK ? xuiCheckBoxSetMixedIndicatorSurface(w,s,rect):result;}
static int setRadio(xui_widget w,xui_surface s){return xuiRadioSetIndicatorSurface(w,s,rect,s,rect);}
static int setToggle(xui_widget w,xui_surface s){return xuiToggleSetIndicatorSurface(w,s,rect,s,rect);}
static int setJoystick(xui_widget w,xui_surface s)
{
    static const int parts[]={XUI_VIRTUAL_JOYSTICK_PART_BASE,XUI_VIRTUAL_JOYSTICK_PART_BASE_ACTIVE,XUI_VIRTUAL_JOYSTICK_PART_KNOB,XUI_VIRTUAL_JOYSTICK_PART_KNOB_ACTIVE,XUI_VIRTUAL_JOYSTICK_PART_RIPPLE};
    for(size_t i=0;i<5;++i){int result=xuiVirtualJoystickSetSurface(w,parts[i],s,rect);if(result!=XUI_OK)return result;}return XUI_OK;
}
static int setWindow(xui_widget w,xui_surface s){return xuiWindowSetIcon(w,s,rect);}
static int setQr(xui_widget w,xui_surface s){return xuiQrCodeSetIcon(w,s,rect,16);}
static int setBreadcrumb(xui_widget w,xui_surface s){return xuiBreadcrumbSetSeparatorIcon(w,s,rect,16);}
static int setProgress(xui_widget w,xui_surface s)
{xui_nine_patch_t p=patch(s);int result=xuiProgressSetTrackPatch(w,s ? &p:NULL);return result==XUI_OK ? xuiProgressSetFillPatch(w,s ? &p:NULL):result;}
typedef struct SurfaceFamily {const char* name;xui_widget_type(*type)(xui_context);int(*set)(xui_widget,xui_surface);size_t slots;} SurfaceFamily;
static const SurfaceFamily families[]={
    {"Image",xuiImageGetType,setImage,1},{"Button",xuiButtonGetType,setButton,8},
    {"CheckBox",xuiCheckBoxGetType,setCheck,3},{"Radio",xuiRadioGetType,setRadio,2},
    {"Toggle",xuiToggleGetType,setToggle,2},{"Joystick",xuiVirtualJoystickGetType,setJoystick,5},
    {"Window",xuiWindowGetType,setWindow,1},{"QrCode",xuiQrCodeGetType,setQr,1},
    {"Breadcrumb",xuiBreadcrumbGetType,setBreadcrumb,1},{"Progress",xuiProgressGetType,setProgress,2}
};
static unsigned matchingEdges;static xrtownershipref matching;
static bool edge(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==matching.Data && ref.Ops==matching.Ops)++matchingEdges;return true;}
static void allFamilies(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);unsigned finalized=0;
    xui_surface a=surfaceCreate(proxy,&finalized,0),b=surfaceCreate(proxy,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    for(size_t i=0;i<sizeof(families)/sizeof(*families);++i){
        const SurfaceFamily* f=&families[i];xui_widget widget=NULL;xui_widget_type type=f->type(context);assert(type && type->iSurfaceSlotCount==f->slots);
        int result=xuiWidgetCreateTyped(context,type,&widget,NULL);
        if(result!=XUI_OK){fprintf(stderr,"create %s: %d\n",f->name,result);assert(result==XUI_OK);}
        assert(f->set(widget,a)==XUI_OK && refs(a)==1+f->slots);
        assert(f->set(widget,a)==XUI_OK && refs(a)==1+f->slots);
        assert(f->set(widget,b)==XUI_OK && refs(a)==1 && refs(b)==1+f->slots);
        matching=xuiSurfaceOwnership(b);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
        assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==f->slots);assert(xrtOwnershipScopeEnd(&freeze));
        assert(f->set(widget,NULL)==XUI_OK && refs(b)==1);assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
    }
    xuiDestroy(context);assert(xuiSurfaceReleaseChecked(a)==XUI_OK && xuiSurfaceReleaseChecked(b)==XUI_OK && finalized==2);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && owner.refs==1 && drop(&owner));
}
static xui_surface selected;static bool failInit;
static int init(xui_widget widget,void* data,const void* input,void* user)
{
    (void)data;(void)input;(void)user;assert(!widget->bTypeInitialized);
    assert(xuiButtonSetIcon(widget,selected,rect)==XUI_OK && xuiButtonSetIcon(widget,selected,rect)==XUI_OK);
    return failInit ? XUI_ERROR_INVALID_STATE:XUI_OK;
}
static void constructorsAndRejection(void)
{
    xui_test_proxy_state_t state,otherState;Owner owner={.refs=1},otherOwner={.refs=1};
    xui_proxy proxy=proxyCreate(&state,&owner),other=proxyCreate(&otherState,&otherOwner);unsigned finalized=0;
    xui_surface a=surfaceCreate(proxy,&finalized,0),b=surfaceCreate(proxy,&finalized,0),foreign=surfaceCreate(other,&finalized,0);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="owned-surface-init",.pParent=xuiButtonGetType(context),.onInit=init};
    desc.iTypeDataSize=desc.pParent->iTypeDataSize;xui_widget_type type=NULL;assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);selected=a;
    for(int failure=0;failure<2;++failure){
        failInit=failure;xui_widget widget=NULL;int status=xuiWidgetCreateTyped(context,type,&widget,NULL);
        if(failure)assert(status==XUI_ERROR_INVALID_STATE && !widget && refs(a)==1);
        else {assert(status==XUI_OK && widget && refs(a)==2);assert(xuiWidgetDestroyChecked(widget)==XUI_OK && refs(a)==1);}
    }
    xui_widget widget=NULL;assert(xuiCheckBoxCreate(context,&widget,NULL)==XUI_OK);
    assert(xuiCheckBoxSetIndicatorSurface(widget,a,rect,a,rect)==XUI_OK && refs(a)==3);
    assert(xuiCheckBoxSetIndicatorSurface(widget,b,rect,foreign,rect)==XUI_ERROR_INVALID_ARGUMENT);
    xui_surface left=NULL,right=NULL;assert(xuiCheckBoxGetIndicatorSurface(widget,&left,NULL,&right,NULL)==XUI_OK && left==a && right==a);
    assert(refs(a)==3 && refs(b)==1 && refs(foreign)==1 && !context->pActiveResourceUpdates);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && refs(a)==1);
    for(int budget=0;budget<24;++budget){
        xui_image_desc_t image={.iSize=sizeof(image),.pSurface=a};assert(xrtMemDebugFailAfter(budget));
        int status=xuiImageCreate(context,&widget,&image);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(status==XUI_OK)assert(widget && refs(a)==2 && xuiWidgetDestroyChecked(widget)==XUI_OK);else assert(!widget);
        assert(refs(a)==1 && !context->pActiveResourceUpdates);
    }
    xuiDestroy(context);assert(xuiSurfaceReleaseChecked(a)==XUI_OK && xuiSurfaceReleaseChecked(b)==XUI_OK && xuiSurfaceReleaseChecked(foreign)==XUI_OK && finalized==3);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && xuiProxyReleaseChecked(other)==XUI_OK && drop(&owner) && drop(&otherOwner));
}
static void reentryAndRollback(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);unsigned finalized=0;
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    xui_widget first=NULL,other=NULL;assert(xuiImageCreate(context,&first,NULL)==XUI_OK && xuiImageCreate(context,&other,NULL)==XUI_OK);
    xui_surface a=surfaceCreate(proxy,&finalized,0),b=surfaceCreate(proxy,&finalized,0),retiring=surfaceCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);
    int same=-1,different=-1;SurfacePayload* payload=xuiInternalProxyResourcePayload(retiring);
    payload->same=first;payload->other=other;payload->replacement=a;payload->sameStatus=&same;payload->otherStatus=&different;
    assert(xuiImageSetSurface(first,retiring)==XUI_OK && xuiSurfaceReleaseChecked(retiring)==XUI_OK);
    assert(xuiImageSetSurface(first,b)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(same==XUI_ERROR_BUSY && different==XUI_OK && xuiImageGetSurface(first)==b && xuiImageGetSurface(other)==a);
    xui_widget indicator=NULL;assert(xuiCheckBoxCreate(context,&indicator,NULL)==XUI_OK);
    assert(xuiCheckBoxSetIndicatorSurface(indicator,a,rect,a,rect)==XUI_OK);
    xui_surface pending=surfaceCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);payload=xuiInternalProxyResourcePayload(pending);
    payload->indicators=indicator;payload->expected=a;payload->secondary=true;
    xui_widget_type type=xuiCheckBoxGetType(context);xui_surface* slots[2];
    for(size_t i=0;i<2;++i)slots[i]=(xui_surface*)((char*)indicator->pTypeData+type->pSurfaceOffsets[i]);
    xui_surface values[]={pending,pending};xui_surface_slot_update_t updates[2];memset(updates,0,sizeof(updates));
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    assert(xuiInternalSurfaceSlotsPrepare(indicator,2,slots,values,updates)==XUI_OK && xuiInternalSurfaceSlotsPublish(updates,2)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(pending)==XUI_OK);
    assert(xuiInternalSurfaceSlotsEnd(updates,2,false,XUI_ERROR_INVALID_STATE)==XUI_ERROR_INVALID_STATE && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(!context->pActiveResourceUpdates && !context->pCleanupPrefixes);
    xuiDestroy(context);assert(refs(a)==1 && refs(b)==1);
    assert(xuiSurfaceReleaseChecked(a)==XUI_OK && xuiSurfaceReleaseChecked(b)==XUI_OK && finalized==4);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void clearTypeData(xui_widget widget,void* data,void* user)
{(void)user;memset(data,0,widget->pType->iTypeDataSize);}
static void clearParentIcon(xui_widget widget,void* data,void* user)
{(void)user;*(xui_surface*)((char*)data+widget->pType->pParent->pSurfaceOffsets[0])=NULL;}
typedef struct DynamicSlots {xui_surface slots[2];} DynamicSlots;
static int dynamicInit(xui_widget widget,void* data,const void* input,void* user)
{(void)widget;(void)input;(void)user;DynamicSlots* value=data;value->slots[0]=selected;value->slots[1]=selected;return XUI_OK;}
static void dynamicSlots(xui_widget widget,void* data,xui_surface_slot_array_t* out)
{
    (void)widget;static const size_t offsets[]={0};
    *out=(xui_surface_slot_array_t){((DynamicSlots*)data)->slots,2,sizeof(xui_surface),offsets,1};
}
static void inheritedAndDynamicSlots(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);unsigned finalized=0;
    xui_surface a=surfaceCreate(proxy,&finalized,0);selected=a;failInit=false;
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="clear-parent-owned-slots",.pParent=xuiButtonGetType(context),.onInit=init,.onDestroy=clearParentIcon};
    desc.iTypeDataSize=desc.pParent->iTypeDataSize;xui_widget_type type=NULL;xui_widget widget=NULL;
    assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK && xuiWidgetCreateTyped(context,type,&widget,NULL)==XUI_OK && refs(a)==2);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && refs(a)==1);
    desc=(xui_widget_type_desc_t){.iSize=sizeof(desc),.sName="dynamic-owned-slots",.pParent=xuiWidgetGetBaseType(),.iTypeDataSize=sizeof(DynamicSlots),.onInit=dynamicInit,.onDestroy=clearTypeData};
    assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);type->onSurfaceSlots=dynamicSlots;
    assert(xuiWidgetCreateTyped(context,type,&widget,NULL)==XUI_OK && refs(a)==3);
    matching=xuiSurfaceOwnership(a);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==2 && xrtOwnershipScopeEnd(&freeze));
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && refs(a)==1);
    for(int nullable=0;nullable<2;++nullable){
        xui_surface failure=surfaceCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);
        ((SurfacePayload*)xuiInternalProxyResourcePayload(failure))->secondary=true;
        xerror* primary=NULL;if(!nullable){xrtSetErrorKind(XERR_RANGE);primary=xrtTakeError();assert(primary);}
        xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
        xui_cleanup_prefix_t prefix={0},nested={0};
        assert(xuiInternalCleanupPrefixBegin(context,&prefix,XUI_ERROR_INVALID_STATE,primary)==XUI_OK);
        assert(xuiInternalCleanupPrefixBegin(context,&nested,XUI_ERROR_CALLBACK_FAILED,NULL)==XUI_OK);
        assert(nested.status==XUI_ERROR_INVALID_STATE && nested.error==primary);
        assert(xuiInternalCleanupPrefixEnd(&prefix)==XUI_ERROR_INVALID_STATE);
        assert(xuiInternalSurfaceRelease(context,failure)==XUI_ERROR_CALLBACK_FAILED);
        xrtClearError();assert(xuiInternalCleanupPrefixEnd(&nested)==XUI_OK && xuiInternalCleanupPrefixEnd(&prefix)==XUI_OK);
        assert(xuiCallScopeEnd(&outer)==XUI_ERROR_INVALID_STATE && xrtGetError()==primary);
        xrtClearError();xrtErrorFree(primary);
        assert(!context->pCleanupPrefixes);
    }
    xuiDestroy(context);assert(xuiSurfaceReleaseChecked(a)==XUI_OK && finalized==3);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static int failCaps(xui_proxy proxy,xui_proxy_caps_t* caps)
{(void)proxy;(void)caps;return XUI_ERROR_CALLBACK_FAILED;}
static void ownedFactories(void)
{
    xui_test_proxy_state_t state,otherState,badState;Owner owner={.refs=1},otherOwner={.refs=1},badOwner={.refs=1};
    xui_proxy proxy=proxyCreate(&state,&owner),other=proxyCreate(&otherState,&otherOwner);
    xui_context context=(xui_context)(uintptr_t)1;
    assert(xuiCreateOwned(NULL,&context)==XUI_ERROR_INVALID_ARGUMENT && !context);
    xui_proxy_t copy=*proxy;
    assert(xuiCreateOwned(&copy,&context)==XUI_OK && xuiGetProxyOwned(context)==proxy);
    memset(&copy,0,sizeof(copy));
    assert(xuiSetProxyOwned(context,proxy)==XUI_OK && xuiSetProxyOwned(context,other)==XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiSetProxy(context,&state.tProxy)==XUI_ERROR_ALREADY_INITIALIZED && xuiDestroyChecked(context)==XUI_OK);
    for(int budget=0;budget<32;++budget){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(budget));
        int result=xuiCreateOwned(proxy,&context);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(result==XUI_OK)assert(context && xuiGetProxyOwned(context)==proxy && xuiDestroyChecked(context)==XUI_OK);else assert(!context);
        xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    }
    xuiTestProxyInit(&badState);badState.tProxy.getCaps=failCaps;xui_callback_owner_t binding=ownerRef(&badOwner);xui_proxy bad=NULL;
    assert(xuiProxyCreateOwned(&badState.tProxy,&binding,&bad)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xuiCreateOwned(bad,&context)==XUI_ERROR_CALLBACK_FAILED && !context && !xrtGetError());
    xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    unsigned finalized=0;xui_surface surface=surfaceCreate(proxy,&finalized,0);
    assert(xuiCreate(&context)==XUI_OK);xui_widget widget=NULL;assert(xuiImageCreate(context,&widget,NULL)==XUI_OK);
    assert(xuiImageSetSurface(widget,surface)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_ERROR_INVALID_STATE);
    assert(!xuiGetProxyOwned(context) && xuiDestroyChecked(context)==XUI_OK && refs(surface)==1);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && finalized==1);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && xuiProxyReleaseChecked(other)==XUI_OK && xuiProxyReleaseChecked(bad)==XUI_OK);
    assert(drop(&owner) && drop(&otherOwner) && drop(&badOwner));
}
typedef struct RetireState {bool ready,complete;unsigned prepares,destroyed;int status;xui_widget parent;struct RetireState* dependency;} RetireState;
typedef struct RetireData {RetireState* state;} RetireData;
static int retireInit(xui_widget widget,void* raw,const void* input,void* user)
{(void)widget;(void)user;((RetireData*)raw)->state=(RetireState*)input;return XUI_OK;}
static int retireReady(xui_widget widget,const void* raw)
{(void)widget;const RetireData* data=raw;return data->state->ready ? XUI_OK:XUI_ERROR_BUSY;}
static int retirePrepare(xui_widget widget,void* raw,bool* complete)
{(void)widget;RetireState* state=((RetireData*)raw)->state;++state->prepares;*complete=state->complete;return state->status;}
static void retireDestroy(xui_widget widget,void* raw,void* user)
{
    (void)widget;(void)user;RetireState* state=((RetireData*)raw)->state;
    if(state->parent)assert(state->parent->iMagic==XUI_WIDGET_MAGIC && state->parent->pTypeData);
    if(state->dependency)assert(state->dependency->destroyed==1);
    ++state->destroyed;
}
static int failLeafInit(xui_widget widget,void* raw,const void* input,void* user)
{(void)widget;(void)raw;(void)input;(void)user;return XUI_ERROR_INVALID_STATE;}
static void failLeafDestroy(xui_widget widget,void* raw,void* user)
{(void)widget;(void)raw;(void)user;assert(!"failed leaf destructor must not execute");}
static xui_widget_type retireType(xui_context context,const char* name)
{
    xui_widget_type type=NULL;xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName=name,.pParent=xuiWidgetGetBaseType(),.iTypeDataSize=sizeof(RetireData),.onInit=retireInit,.onDestroy=retireDestroy};
    assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);
    type->onDestroyReady=retireReady;type->onDestroyPrepare=retirePrepare;return type;
}
static void retirementClosures(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    xui_widget_type type=retireType(context,"retirement-closure");
    RetireState parent={.ready=true,.complete=true},child={.ready=false,.complete=false};parent.dependency=&child;
    xui_widget a=NULL,b=NULL,branch=NULL;
    assert(xuiWidgetCreateTyped(context,type,&a,&parent)==XUI_OK);
    assert(xuiInternalWidgetCreatePart(a,type,&b,&child)==XUI_OK);child.parent=a;
    assert(a->pFirstOwnedPart==b && b->pInternalOwner==a && b->ppOwnedPartPrevious==&a->pFirstOwnedPart);
    assert(xuiInternalWidgetClaimPart(b,a)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetCreate(context,&branch)==XUI_OK && xuiInternalWidgetInsertPart(a,branch,b,NULL)==XUI_OK);
    /* The part is visually elsewhere. One close must preserve both carriers
     * while it is busy, and one boundary must never spin/retry the same work. */
    assert(xuiWidgetDestroyChecked(a)==XUI_OK && context->pDeferredDestroyHead);
    assert(!parent.destroyed && !child.destroyed && parent.prepares==1 && !child.prepares);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK);
    assert(parent.prepares==2 && !child.prepares);
    child.ready=true;child.status=XUI_ERROR_CALLBACK_FAILED;
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(parent.prepares==3 && child.prepares==1 && !child.destroyed && context->pDeferredDestroyHead);
    child.status=XUI_OK;child.complete=true;
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK);
    assert(parent.destroyed==1 && child.destroyed==1 && !branch->pFirstChild && !context->pDeferredDestroyHead);
    /* A successful parent init stage remains really owned after a leaf fails;
     * no immediate void rollback may erase its temporarily busy payload. */
    RetireState partial={.ready=false,.complete=true};
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="retirement-failed-leaf",.pParent=type,.iTypeDataSize=sizeof(RetireData),.onInit=failLeafInit,.onDestroy=failLeafDestroy};
    xui_widget_type leaf=NULL;assert(xuiWidgetRegisterType(context,&leaf,&desc)==XUI_OK);
    xui_widget failed=(void*)(uintptr_t)1;assert(xuiWidgetCreateTyped(context,leaf,&failed,&partial)==XUI_ERROR_INVALID_STATE && !failed);
    assert(!partial.destroyed && context->pDeferredDestroyHead && context->pDeferredDestroyHead->pInitializedType==type);
    partial.ready=true;assert(xuiCallScopeBegin(context,&scope)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK && partial.destroyed==1);
    assert(xuiDestroyChecked(context)==XUI_OK);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void mixedVisualAndNativeClosure(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    xui_widget_type type=retireType(context,"mixed-menu-like-closure");
    RetireState aState={.ready=true,.complete=true},bState={.ready=true,.complete=true};aState.dependency=&bState;
    xui_widget a=NULL,b=NULL,visualRoot=NULL;
    assert(xuiWidgetCreateTyped(context,type,&a,&aState)==XUI_OK);
    assert(xuiInternalWidgetCreatePart(a,type,&b,&bState)==XUI_OK);bState.parent=a;
    assert(xuiWidgetAddChild(b,a)==XUI_OK); /* Menu owned Popup -> visual Menu. */
    assert(xuiWidgetAddChild(a,b)==XUI_ERROR_INVALID_STATE); /* Internal-part protection still applies. */
    assert(xuiWidgetCreate(context,&visualRoot)==XUI_OK);
    assert(xuiInternalWidgetInsertPart(a,visualRoot,b,NULL)==XUI_OK);
    /* Entering by the visual outside root must retire b while a's TypeData is
     * still alive. The mixed cycle does not invoke either destructor twice. */
    assert(xuiWidgetDestroyChecked(visualRoot)==XUI_OK);
    assert(aState.destroyed==1 && bState.destroyed==1 && !context->pAllWidgets && !context->pDeferredDestroyHead);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static unsigned invalidated;
static void invalidation(void* raw){++*(unsigned*)raw;}
static void requestedContext(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    xui_call_scope_t outer={0},refused={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    bool consumed=true;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    assert(context->bCloseRequested && !context->bDestroyPending && !xuiGetProxyOwned(context));
    assert(xuiCallScopeBegin(context,&refused)==XUI_ERROR_INVALID_STATE);
    invalidated=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(context,&watch,invalidation,&invalidated)==XUI_OK);
    assert(xuiCallScopeEnd(&outer)==XUI_OK && !invalidated);
    assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed && invalidated==1 && !watch.ppPrevious);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void managedRegistry(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);unsigned finalized=0;
    xui_surface a=surfaceCreate(proxy,&finalized,0),b=surfaceCreate(proxy,&finalized,0);
    xui_resource resource=NULL;xui_resource_desc_t desc={.iSize=sizeof(desc),.sName="managed-surface",.iKind=XUI_RESOURCE_SURFACE,.pHandle=a};
    assert(xuiResourceSet(context,&resource,&desc)==XUI_OK && resource && refs(a)==2);
    matching=xuiSurfaceOwnership(a);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==1 && xrtOwnershipScopeEnd(&freeze));
    for(int budget=0;budget<3;++budget){
        xui_resource previous=xuiResourceFind(context,"managed-surface");assert(xrtMemDebugFailAfter(budget));desc.pHandle=b;
        int result=xuiResourceSet(context,&resource,&desc);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(result==XUI_OK){assert(refs(a)==1 && refs(b)==2);desc.pHandle=a;assert(xuiResourceSet(context,&resource,&desc)==XUI_OK);}
        else assert(xuiResourceFind(context,"managed-surface")==previous && refs(a)==2 && refs(b)==1);
    }
    assert(xuiDestroyChecked(context)==XUI_OK && refs(a)==1 && refs(b)==1);
    assert(xuiSurfaceReleaseChecked(a)==XUI_OK && xuiSurfaceReleaseChecked(b)==XUI_OK && finalized==2);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
typedef struct CacheDrawPayload {unsigned id;} CacheDrawPayload;
static unsigned cacheSurfaceFinalized,cacheDrawFinalized,cacheDrawDepth,cacheDrawIds[32],cacheNextDraw;
static xui_widget closeDuringCacheClear;
static int cacheDrawProducerFailure;
static int cacheSurfaceCreate(xui_proxy proxy,xui_surface* out,const xui_surface_desc_t* desc)
{(void)desc;*out=surfaceCreate(proxy,&cacheSurfaceFinalized,XUI_OK);return XUI_OK;}
static void cacheSurfaceDestroy(xui_proxy proxy,xui_surface surface)
{(void)proxy;assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);}
static bool cacheDrawReady(const void* raw)
{const CacheDrawPayload* payload=raw;return cacheDrawDepth && cacheDrawIds[cacheDrawDepth-1]==payload->id;}
static int cacheDrawEndPayload(xui_proxy proxy,void* raw)
{(void)proxy;assert(cacheDrawReady(raw));--cacheDrawDepth;++cacheDrawFinalized;return XUI_OK;}
static int cacheDrawBegin(xui_proxy proxy,xui_draw_context* out,xui_surface surface)
{
    void* raw=NULL;int status=xuiDrawCreateOwnedEx(proxy,surface,sizeof(CacheDrawPayload),cacheDrawEndPayload,cacheDrawReady,out,&raw);
    if(status!=XUI_OK)return status;
    CacheDrawPayload* payload=raw;payload->id=++cacheNextDraw;assert(cacheDrawDepth<32);cacheDrawIds[cacheDrawDepth++]=payload->id;
    return cacheDrawProducerFailure;
}
static int cacheDrawEnd(xui_proxy proxy,xui_draw_context draw)
{(void)proxy;int status=xuiDrawEndChecked(draw);if(status!=XUI_OK && !xuiDrawEnded(draw))return status;bool consumed=false;int released=xuiDrawReleaseOwnerChecked(draw,&consumed);assert(consumed);return status!=XUI_OK ? status:released;}
static int cacheDrawClear(xui_proxy proxy,xui_draw_context draw,xui_rect_t area,uint32_t color)
{
    (void)proxy;(void)draw;(void)area;(void)color;
    if(closeDuringCacheClear){xui_widget widget=closeDuringCacheClear;closeDuringCacheClear=NULL;assert(xuiWidgetDestroyChecked(widget)==XUI_OK);}
    return XUI_OK;
}
static xui_proxy cacheProxyCreate(xui_test_proxy_state_t* state,Owner* owner)
{
    xuiTestProxyInit(state);state->tProxy.surfaceCreate=cacheSurfaceCreate;state->tProxy.surfaceDestroy=cacheSurfaceDestroy;
    state->tProxy.surfaceGetDesc=surfaceDesc;state->tProxy.surfaceGetGeneration=generation;
    state->tProxy.drawBegin=cacheDrawBegin;state->tProxy.drawEnd=cacheDrawEnd;state->tProxy.drawClearRect=cacheDrawClear;
    xui_callback_owner_t binding=ownerRef(owner);xui_proxy proxy=NULL;assert(xuiProxyCreateOwned(&state->tProxy,&binding,&proxy)==XUI_OK);return proxy;
}
static xui_widget cacheWidget(xui_context context)
{xui_widget widget=NULL;assert(xuiWidgetCreate(context,&widget)==XUI_OK && xuiWidgetSetRect(widget,(xui_rect_t){0,0,32,32})==XUI_OK);return widget;}
static void ownedWidgetCaches(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=cacheProxyCreate(&state,&owner);xui_context context=NULL;
    unsigned beforeSurface=cacheSurfaceFinalized,beforeDraw=cacheDrawFinalized;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);xui_widget widget=cacheWidget(context);xui_draw_context draw=NULL;
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_OK && draw && widget->pActiveUpdateDraw==draw);
    assert(xuiDrawAddRef(draw)==XUI_OK); /* The language retained output's real owner. */
    xui_surface surface=xuiWidgetGetCacheSurface(widget,0);assert(surface && refs(surface)==2);
    assert(xuiWidgetUpdateEnd(widget,0,draw)==XUI_OK && xuiDrawEnded(draw) && cacheDrawDepth==0);
    assert(xuiDrawReleaseChecked(draw)==XUI_OK && refs(surface)==1 && cacheDrawFinalized==beforeDraw+1);
    matching=xuiSurfaceOwnership(surface);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==1 && xrtOwnershipScopeEnd(&freeze));
    assert(xuiDestroyChecked(context)==XUI_OK && cacheSurfaceFinalized==beforeSurface+1);
    /* Registry order B,A differs from draw stack B then A. Known non-top
     * waiting must not prevent the later top owner from actually retiring. */
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);xui_widget a=cacheWidget(context),b=cacheWidget(context);
    xui_draw_context bDraw=NULL,aDraw=NULL;
    assert(xuiWidgetUpdateBegin(b,0,0,0,&bDraw)==XUI_OK && xuiWidgetUpdateBegin(a,0,0,0,&aDraw)==XUI_OK);
    bool consumed=false;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed && !cacheDrawDepth);
    assert(cacheDrawFinalized==beforeDraw+3 && cacheSurfaceFinalized==beforeSurface+3);
    /* Independent native Draw activity keeps the actual Context owner. */
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);widget=cacheWidget(context);
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_OK);
    xui_draw_scope_t scope={0};assert(xuiDrawCallScopeBegin(draw,&scope)==XUI_OK);
    consumed=true;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_STATUS_DEFERRED && !consumed && widget->pActiveUpdateDraw==draw);
    assert(xuiDrawCallScopeEnd(&scope)==XUI_OK);
    assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed && !cacheDrawDepth);
    /* A failed producer with a non-NULL Draw leaves no untracked local owner. */
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);widget=cacheWidget(context);cacheDrawProducerFailure=XUI_ERROR_CALLBACK_FAILED;
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_ERROR_CALLBACK_FAILED && !draw && !widget->pActiveUpdateDraw && !cacheDrawDepth && !xrtGetError());
    cacheDrawProducerFailure=XUI_OK;
    assert(xuiDestroyChecked(context)==XUI_OK);
    /* A clear callback can close its own widget. No borrowed output survives
     * the function's operation tail after that Native owner was consumed. */
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);widget=cacheWidget(context);closeDuringCacheClear=widget;
    assert(xuiWidgetUpdateBegin(widget,0,XUI_WIDGET_UPDATE_CLEAR,0,&draw)==XUI_ERROR_INVALID_STATE && !draw && !cacheDrawDepth && !context->pAllWidgets);
    assert(xuiDestroyChecked(context)==XUI_OK);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static int requestedStateEvent(xui_widget widget,const xui_event_t* event,void* user)
{(void)widget;(void)event;++*(unsigned*)user;return XUI_OK;}
static void requestedQueuedState(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);
    xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    assert(xuiWidgetCreate(context,&widget)==XUI_OK && xuiSetRootWidget(context,widget)==XUI_OK);
    unsigned calls=0;assert(xuiWidgetSetEventCallback(widget,requestedStateEvent,&calls)==XUI_OK);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
    assert(xuiWidgetSetVisible(widget,0)==XUI_OK && context->pStateChangeHead);
    assert(xuiRequestClose(context)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK);
    assert(!calls && !context->iOperationDepth && context->pStateChangeHead);
    bool consumed=false;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void nativeScopeAmbient(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();assert(ambient);
    assert(xuiCreateOwned(proxy,&context)==XUI_OK && xrtGetError()==ambient);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK && !xrtGetError());
    assert(xuiCallScopeEnd(&scope)==XUI_OK && xrtGetError()==ambient);
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK && !xrtGetError());
    xerror* first=NULL;int status=xuiInternalCallScopeEndOutcome(&scope,XUI_OK,&first);
    assert(status==XUI_OK && first==ambient && !xrtGetError());xrtSetErrorTake(first);
    xui_widget widget=NULL;assert(xuiWidgetCreate(context,&widget)==XUI_OK && xrtGetError()==ambient);
    assert(xuiWidgetSetCacheStateCount(widget,1)==XUI_OK && xrtGetError()==ambient);
    xui_vec2_t size={99,99};assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&size)==XUI_OK && xrtGetError()==ambient);
    /* A real NULL-diagnostic failure replaces an unrelated old diagnostic;
     * later cleanup diagnostics cannot turn into the primary one. */
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK && !xrtGetError());
    xuiInternalCallScopeRecordOutcome(&scope,XUI_ERROR_INVALID_STATE,NULL);
    xrtSetErrorKind(XERR_STATE);first=NULL;
    status=xuiInternalCallScopeEndOutcome(&scope,XUI_ERROR_INVALID_STATE,&first);
    assert(status==XUI_ERROR_INVALID_STATE && !first && !xrtGetError());
    xrtSetErrorKind(XERR_RANGE);ambient=xrtGetError();assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
    xrtSetErrorKind(XERR_STATE);const xerror* current=xrtGetError();
    xui_call_scope_t child={0};assert(xuiCallScopeBegin(context,&child)==XUI_OK && !xrtGetError());
    assert(xuiCallScopeEnd(&child)==XUI_OK && xrtGetError()==current);
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==current);xrtClearError();
    xrtSetErrorKind(XERR_RANGE);ambient=xrtGetError();
    assert(xuiDestroyChecked(context)==XUI_OK && xrtGetError()==ambient);xrtClearError();
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
typedef struct ScopeThreadProbe {xui_context context;xui_call_scope_t* outer;} ScopeThreadProbe;
static int32 nativeScopeWrongThread(ptr raw)
{
    ScopeThreadProbe* probe=raw;xui_call_scope_t scope={0};
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();assert(ambient);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCallScopeBegin(probe->context,&scope)==XUI_ERROR_BUSY && !scope.pSelf);
    assert(xuiCallScopeEnd(probe->outer)==XUI_ERROR_BUSY && probe->outer->pSelf==probe->outer);
    assert(xrtGetError()==ambient && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();return 0;
}
static void nativeScopeOrder(void)
{
    static const unsigned orders[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    for(unsigned mode=0;mode<3;++mode)for(unsigned order=0;order<6;++order){
        xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();assert(ambient);
        xui_call_scope_t scopes[3]={{0}};assert(xuiCallScopeBegin(context,&scopes[0])==XUI_OK);
        const xerror* primary=ambient;
        if(mode==1){xrtSetErrorKind(XERR_STATE);primary=xrtGetError();assert(primary);}
        assert(xuiCallScopeBegin(context,&scopes[1])==XUI_OK && xuiCallScopeBegin(context,&scopes[2])==XUI_OK);
        if(mode==2){primary=NULL;xuiInternalContextRecordOutcome(context,-42,NULL);}
        assert(xrtMemDebugFailAfter(0));
        for(unsigned step=0;step<3;++step){
            unsigned index=orders[order][step];int expected=mode==2 ? -42:mode==1 && index==0 ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
            assert(xuiCallScopeEnd(&scopes[index])==expected);
        }
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
        assert(xrtGetError()==primary && !context->pCallScopes && !context->iOperationDepth);xrtClearError();
    }
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    ScopeThreadProbe probe={context,&outer};xthread* worker=xrtThreadCreate(nativeScopeWrongThread,&probe,0);assert(worker);
    assert(xrtThreadWaitFor(worker,10000)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(context->iOperationDepth==1 && context->pCallScopes==&outer && xuiCallScopeEnd(&outer)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void cacheIdentityBoundary(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=cacheProxyCreate(&state,&owner);xui_context context=NULL;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);xui_widget widget=cacheWidget(context);xui_draw_context draw=NULL;
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_OK && widget->iCacheEpoch==1);
    assert(xuiWidgetUpdateEnd(widget,0,draw)==XUI_OK);uint32_t epoch=widget->iCacheEpoch;
    assert(xuiWidgetSetCacheStateCount(widget,0)==XUI_OK);
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_OK && widget->iCacheEpoch>epoch);
    assert(xuiWidgetUpdateEnd(widget,0,draw)==XUI_OK);
    xui_surface current=xuiWidgetGetCacheSurface(widget,0);widget->iCacheEpoch=UINT32_MAX;
    assert(xuiWidgetSetRect(widget,(xui_rect_t){0,0,64,64})==XUI_OK);
    assert(xuiWidgetUpdateBegin(widget,0,0,0,&draw)==XUI_ERROR_INVALID_STATE && !draw);
    assert(widget->iCacheEpoch==UINT32_MAX && xuiWidgetGetCacheSurface(widget,0)==current);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static void ownedIconSurfaces(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xui_proxy proxy=proxyCreate(&state,&owner);xui_context context=NULL;unsigned finalized=0;
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);xui_icon_category category=NULL;assert(xuiIconCategoryCreate(context,"owned-icons",NULL,&category)==XUI_OK);
    xui_surface surface=surfaceCreate(proxy,&finalized,0);xui_icon icon=NULL,alias=NULL;
    assert(xuiIconAddSurface(category,"surface",surface,rect,0,NULL,&icon)==XUI_OK && refs(surface)==2);
    assert(xuiIconAddAlias(category,"alias",icon,NULL,&alias)==XUI_OK && refs(surface)==2);
    matching=xuiSurfaceOwnership(surface);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==1 && xrtOwnershipScopeEnd(&freeze));
    assert(xuiIconRemove(category,"surface")==XUI_OK && refs(surface)==2);
    matchingEdges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==1 && xrtOwnershipScopeEnd(&freeze));
    assert(xuiIconRemove(category,"alias")==XUI_OK && refs(surface)==1);
    for(int budget=0;budget<16;++budget){
        assert(xrtMemDebugFailAfter(budget));icon=NULL;int status=xuiIconAddSurface(category,"oom",surface,rect,0,NULL,&icon);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(status==XUI_OK)assert(icon && refs(surface)==2 && xuiIconRemove(category,"oom")==XUI_OK);
        else assert(!icon);
        assert(refs(surface)==1);
    }
    xui_surface transfer=surfaceCreate(proxy,&finalized,0);
    assert(xuiIconAddSurface(category,"transfer",transfer,rect,XUI_ICON_SURFACE_TAKE_OWNERSHIP,NULL,&icon)==XUI_OK && refs(transfer)==1);
    assert(xuiDestroyChecked(context)==XUI_OK && finalized==1 && refs(surface)==1);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && finalized==2);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
static unsigned atlasFinalized;
static int atlasDecompress(xui_proxy proxy,void* output,int capacity,const void* input,int size,int* written)
{(void)proxy;(void)input;assert(size>0 && capacity>0 && output && written);memset(output,0,(size_t)capacity);*written=capacity;return XUI_OK;}
static int atlasRgba(xui_proxy proxy,xui_surface* output,int width,int height,const void* pixels,int stride,uint32_t flags)
{
    (void)flags;assert(width>0 && height>0 && stride>=width*4 && pixels);void* raw=NULL;
    int status=xuiSurfaceCreateOwned(proxy,sizeof(SurfacePayload),finalizeSurface,output,&raw);
    if(status==XUI_OK){*(SurfacePayload*)raw=(SurfacePayload){.finalized=&atlasFinalized};}
    return status;
}
static void ownedBuiltinAtlas(void)
{
    xui_test_proxy_state_t state;Owner owner={.refs=1};xuiTestProxyInit(&state);
    state.tProxy.zstdDecompress=atlasDecompress;state.tProxy.surfaceCreateRGBA=atlasRgba;
    xui_callback_owner_t binding=ownerRef(&owner);xui_proxy proxy=NULL;assert(xuiProxyCreateOwned(&state.tProxy,&binding,&proxy)==XUI_OK);
    for(int budget=-1;budget<12;++budget){
        xui_context context=NULL;assert(xuiCreateOwned(proxy,&context)==XUI_OK);xui_surface atlas=NULL;
        assert(xrtMemDebugFailAfter(budget));int status=xuiBuiltinAssetGetAtlas(context,&atlas);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(status==XUI_OK){
            assert(atlas && refs(atlas)==1);xui_surface repeated=NULL;
            assert(xuiBuiltinAssetGetAtlas(context,&repeated)==XUI_OK && repeated==atlas && refs(atlas)==1);
            matching=xuiSurfaceOwnership(atlas);matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
            assert(xuiContextTraceOwnership(context,edge,NULL) && matchingEdges==1 && xrtOwnershipScopeEnd(&freeze));
        }else assert(!atlas);
        assert(xuiDestroyChecked(context)==XUI_OK);
    }
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    allFamilies();constructorsAndRejection();reentryAndRollback();inheritedAndDynamicSlots();ownedFactories();retirementClosures();mixedVisualAndNativeClosure();requestedContext();requestedQueuedState();managedRegistry();ownedWidgetCaches();nativeScopeAmbient();nativeScopeOrder();cacheIdentityBoundary();ownedIconSurfaces();ownedBuiltinAtlas();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Owned native Surface slots: 10 families/26 slots, constructor and multi-slot atomic rollback, reentry, OOM and NULL primary passed");return 0;
}
