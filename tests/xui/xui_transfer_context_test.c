#define XRT_MODULE_ALL
#include "xui.h"
#include "xge.h"
#include "src/xui/core/xui_internal.h"
#include "xui_owned_model_host_test.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <string.h>

typedef struct TransferState {size_t refs;int calls,action,published,watched; xui_context context;xui_data_object data;xui_lifetime_link_t watch;xui_test_proxy_state_t proxyState;} TransferState;
static bool ownerCount(const void* raw,size_t* out){*out=((const TransferState*)raw)->refs;return true;}
static bool ownerTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static bool ownerHold(const void* raw){++((TransferState*)raw)->refs;return true;}
static bool ownerDrop(const void* raw){TransferState* state=(TransferState*)raw;assert(state->refs);state->refs--;return true;}
static const xrtownershipops ownerOps={ownerCount,ownerTrace};
static void watched(void* raw){((TransferState*)raw)->watched++;}
static int wrapUnused(const xn_transfer_services_v1* services,void* native,const xn_transfer_native_owner_v1* owner,void** out)
{(void)services;(void)native;(void)owner;(void)out;assert(0);return -1;}
static int publish(xui_proxy proxy,const xui_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    TransferState* state=(TransferState*)((char*)proxy->pUser - offsetof(TransferState,proxyState));assert(services && services->iAbiTag==XN_TRANSFER_SERVICES_V1_TAG);
    state->published++;assert(count==2 && items[0].iDataSize==3 && !memcmp(items[0].pData,"xyz",3));
    assert(items[1].iDataSize==3 && !memcmp(items[1].pData,"a\0b",3));return XUI_OK;
}
static int readProvider(const xstrview* format,xui_data_output_t* output,void* raw)
{
    (void)format;TransferState* state=raw;state->calls++;
    if(state->action==1)return 1;
    if(state->action==2)assert(xuiRequestClose(state->context)==XUI_OK);
    return xuiDataOutputSet(output,"a\0b",3);
}
static void createCase(xui_proxy* proxy,xui_context* context,TransferState* state,bool capability)
{
    xuiTestProxyInit(&state->proxyState);
    /* Native owns a copied immutable function table; these particular tests
     * use only its own pUser callback and generic Widget hit testing. */
    state->proxyState.tProxy.clipboardSetItemsWithTransfer=capability ? publish:NULL;
    xui_callback_owner_t backend={{state,&ownerOps},ownerHold,ownerDrop};
    assert(xuiProxyCreateOwned(&state->proxyState.tProxy,&backend,proxy)==XUI_OK);
    assert(xuiCreateOwned(*proxy,context)==XUI_OK);state->context=*context;
    xui_widget content=NULL;assert(xuiWidgetCreate(*context,&content)==XUI_OK);
    assert(xuiSetRootWidget(*context,content)==XUI_OK);assert(xuiInputViewport(*context,100,100)==XUI_OK);
    assert(xuiContextLifetimeWatch(*context,&state->watch,watched,state)==XUI_OK);
    assert(xuiDataObjectCreateOwned(&state->data)==XUI_OK);
    assert(xuiDataObjectSet(state->data,"binary","xyz",3)==XUI_OK);
    xui_callback_owner_t owner={{state,&ownerOps},ownerHold,ownerDrop};
    assert(xuiDataObjectSetProviderOwned(state->data,"lazy",readProvider,state,&owner)==XUI_OK);
}
typedef struct TraceState {xrtownershipref expected;unsigned found;} TraceState;
static bool visit(xrtownershipref ref,void* user)
{TraceState* state=user;if(ref.Data==state->expected.Data && ref.Ops==state->expected.Ops)state->found++;return true;}
static void clearCase(xui_proxy proxy,xui_context context,TransferState* state)
{
    bool consumed=false;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed);assert(state->watched==1);
    assert(xuiDataObjectReleaseChecked(state->data)==XUI_OK);assert(state->refs==2);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK);assert(state->refs==1);
}
static int eventFailure(xui_widget widget,const xui_event_t* event,void* user)
{(void)widget;(void)user;return event->iType==XUI_EVENT_DRAG_ENTER ? XUI_ERROR_UNSUPPORTED:XUI_OK;}

static void rawBridgeCase(void)
{
    /* Preserve the explicit same-thread manual C bridge, no real window. */
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;desc.iFlags=XGE_INIT_OFFSCREEN;desc.iWidth=100;desc.iHeight=100;
    assert(xgeInit(&desc)==XGE_OK);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK);assert(xuiInputViewport(context,100,100)==XUI_OK);
    xui_widget root=NULL,target=NULL;assert(xuiWidgetCreate(context,&root)==XUI_OK);assert(xuiSetRootWidget(context,root)==XUI_OK);
    assert(xuiWidgetCreate(context,&target)==XUI_OK);assert(xuiWidgetAddChild(root,target)==XUI_OK);
    assert(xuiWidgetSetRect(target,(xui_rect_t){0,0,100,100})==XUI_OK);assert(xuiWidgetSetDropEnabled(target,1)==XUI_OK);
    assert(xuiWidgetSetEventCallback(target,eventFailure,NULL)==XUI_OK);
    assert(xuiProxyXgePumpInputRect(context,(xui_rect_t){0,0,100,100})==XUI_OK);
    xge_data_object data=NULL;assert(xgeDataObjectCreateOwned(&data)==XGE_OK);assert(xgeDataObjectSet(data,"binary","xyz",3)==XGE_OK);
    xge_drag_event_t event={0};event.iSize=sizeof(event);event.iType=XGE_DRAG_EVENT_ENTER;event.fX=5;event.fY=5;event.pData=data;event.iAllowedEffects=XGE_DRAG_EFFECT_COPY;
    uint32_t effect=0xabcdefu;assert(xgeDragEventDispatchChecked(&event,&effect)==XUI_ERROR_UNSUPPORTED);
    assert(effect==0xabcdefu && !xrtGetError());
    xuiDestroy(context);assert(xgeDataObjectReleaseChecked(data)==XGE_OK);assert(xgeUnitChecked()==XGE_OK);
}
int main(void)
{
    modelHostBind();xn_transfer_services_v1 services={sizeof(services),XN_TRANSFER_SERVICES_V1_TAG,XN_TRANSFER_PROCESS_RESIDENT,NULL,NULL,wrapUnused,wrapUnused,wrapUnused};
    for(int action=0;action<4;++action){
        TransferState state={.refs=1,.action=action};xui_proxy proxy=NULL;xui_context context=NULL;createCase(&proxy,&context,&state,action!=3);
        xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
        int result=xuiClipboardCopyDataWithTransfer(context,state.data,&services);
        assert(result==(action==0 ? XUI_OK:action==1 ? XUI_ERROR_CALLBACK_FAILED:action==2 ? XUI_ERROR_INVALID_STATE:XUI_ERROR_UNSUPPORTED));
        assert(!xrtGetError() && xuiCallScopeEnd(&outer)==XUI_OK);assert(state.watched==0);
        assert(state.published==(action==0));assert(state.calls==(action==3 ? 0:action==1 ? 1:2));
        if(action==0){
            xui_widget source=xuiGetRootWidget(context);assert(xuiDragBegin(context,source,state.data,XUI_DRAG_EFFECT_COPY,0)==XUI_OK);
            TraceState trace={xuiDataObjectOwnership(state.data),0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
            assert(xuiInternalContextTraceResources(context,visit,&trace));assert(xrtOwnershipScopeEnd(&freeze));assert(trace.found==1);
            assert(xuiDragCancel(context)==XUI_OK);
            xui_data_object raw=NULL;assert(xuiDataObjectCreate(&raw)==XUI_OK);
            assert(xuiDragBegin(context,source,raw,XUI_DRAG_EFFECT_COPY,0)==XUI_ERROR_INVALID_ARGUMENT);xuiDataObjectRelease(raw);
            assert(xuiProxyXgePumpInputRect(context,(xui_rect_t){0,0,100,100})==XUI_ERROR_NOT_INITIALIZED);
        }
        clearCase(proxy,context,&state);
    }
    rawBridgeCase();return 0;
}
