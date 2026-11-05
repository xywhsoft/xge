/* CPU-only real Native publishers and dispatch. No engine/device/browser. */
#define main xui_unused_owned_groups_main
#include "xui_owned_groups_test.c"
#undef main
#include "xui_internal.h"
#include "xui_callback_hold_internal.h"
#include "xui_owned_manager.h"

typedef struct HoldOwner HoldOwner;
struct HoldOwner {
    size_t refs;unsigned holds,drops,calls;
    bool fresh,failHold,failDrop,dropDiagnostic,callbackNullFailure;
    xui_widget widget;int slot;
    HoldOwner* replacement;
};
static xui_callback_owner_t holdRef(HoldOwner*);
static int holdEvent(xui_widget,const xui_event_t*,void*);
static int holdSet(HoldOwner* owner)
{
    xui_callback_owner_t ref=holdRef(owner);
    return owner->slot<0 ? xuiWidgetSetEventCallbackOwned(owner->widget,holdEvent,owner,&ref):
        xuiWidgetSetEventHandlerOwned(owner->widget,owner->slot,holdEvent,owner,&ref);
}
static bool holdAcquire(const void* raw)
{
    HoldOwner* owner=(HoldOwner*)raw;assert(owner->refs);++owner->holds;
    if(owner->failHold)return false;
    ++owner->refs;
    if(owner->replacement){HoldOwner* replacement=owner->replacement;owner->replacement=NULL;assert(holdSet(replacement)==XUI_OK);}
    if(owner->fresh)xrtSetErrorKind(XERR_RANGE);
    return true;
}
static bool holdRelease(const void* raw)
{
    HoldOwner* owner=(HoldOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    if(owner->dropDiagnostic)xrtSetErrorKind(XERR_STATE);
    return !owner->failDrop;
}
static bool holdCount(const void* raw,size_t* count){*count=((const HoldOwner*)raw)->refs;return *count!=0;}
static bool holdTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops holdOps={holdCount,holdTrace};
static xui_callback_owner_t holdRef(HoldOwner* owner){return(xui_callback_owner_t){{owner,&holdOps},holdAcquire,holdRelease};}
static int holdEvent(xui_widget widget,const xui_event_t* event,void* raw)
{HoldOwner* owner=raw;assert(widget==owner->widget && event);++owner->calls;return owner->callbackNullFailure ? XUI_ERROR_UNSUPPORTED:XUI_OK;}
static int holdSend(xui_context context,xui_widget widget)
{xui_event_t event={.iSize=sizeof(event),.iType=XUI_EVENT_COMMAND,.pTarget=widget};return xuiDispatchEvent(context,&event);}
static void eventOwnership(int slot,bool reentrant)
{
    xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&widget)==XUI_OK);
    HoldOwner old={.refs=1,.widget=widget,.slot=slot},peer={.refs=1,.widget=widget,.slot=slot},candidate={.refs=1,.widget=widget,.slot=slot,.fresh=true};
    assert(holdSet(&old)==XUI_OK);if(reentrant)candidate.replacement=&peer;
    assert(holdSet(&candidate)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && candidate.refs==1 && candidate.drops==1);xrtClearError();
    assert(holdSend(context,widget)==XUI_OK && old.calls==(reentrant ? 0u:1u) && peer.calls==(reentrant ? 1u:0u) && !candidate.calls);
    HoldOwner* active=reentrant ? &peer:&old;unsigned drops=active->drops;active->fresh=true;
    assert(holdSend(context,widget)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && active->refs==2 && active->drops==drops+1);xrtClearError();active->fresh=false;
    active->failHold=true;drops=active->drops;assert(holdSend(context,widget)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && active->drops==drops);active->failHold=false;
    assert(xuiDestroyChecked(context)==XUI_OK && old.refs==1 && peer.refs==1 && candidate.refs==1);
}
static void eventPrimaryNull(int slot)
{
    xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&widget)==XUI_OK);
    HoldOwner owner={.refs=1,.widget=widget,.slot=slot,.callbackNullFailure=true};assert(holdSet(&owner)==XUI_OK);owner.failDrop=owner.dropDiagnostic=true;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    assert(holdSend(context,widget)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());owner.failDrop=owner.dropDiagnostic=false;
    assert(xuiDestroyChecked(context)==XUI_OK && owner.refs==1);
}
static void groupHeldFailure(void)
{
    xui_context context=NULL;xui_widget widget=NULL;make_group(&context,&widget);
    GroupOwner oldA={.refs=1,.kind=0,.context=context,.widget=widget},oldB=oldA;GroupUser a={0xAA01,&oldA},b={0xBB02,&oldB};
    xui_callback_owner_t ar=group_ref(&oldA),br=group_ref(&oldB);assert(install_group(0,widget,&a,&ar,&b,&br)==XUI_OK);
    HoldOwner first={.refs=1},second={.refs=1,.fresh=true,.failDrop=true,.dropDiagnostic=true};xui_callback_owner_t fr=holdRef(&first),sr=holdRef(&second);
    GroupUser unusedA={0xAA01,&oldA},unusedB={0xBB02,&oldB};
    assert(install_group(0,widget,&unusedA,&fr,&unusedB,&sr)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);
    assert(first.refs==1 && second.refs==1 && first.drops==1 && second.drops==1 && oldA.refs==2 && oldB.refs==2);xrtClearError();
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
    assert(fire_group(0,widget)==XUI_OK && oldA.calls==1 && oldB.calls==1 && xuiCallScopeEnd(&scope)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && oldA.refs==1 && oldB.refs==1);
}
typedef struct HoldManager {xui_owned_manager_t node;xui_callback_owner_t owner;unsigned* finalized;} HoldManager;
static bool managerTrace(const void* raw,xrtownershipvisitor visit,void* user)
{const HoldManager* manager=raw;return !manager->owner.onHold || visit(manager->owner.tReference,user);}
static int managerReady(const void* raw){(void)raw;return XUI_OK;}
static int managerPrepare(void* raw,bool* complete){(void)raw;*complete=true;return XUI_OK;}
static void managerClose(void* raw){(void)raw;}
static int managerFinalize(void* raw)
{HoldManager* manager=raw;int status=xuiInternalCallbackOwnerDrop(manager->node.context,&manager->owner);++*manager->finalized;xrtFree(manager);return status;}
static const xui_owned_manager_ops_t managerOps={managerTrace,managerReady,managerPrepare,managerClose,managerFinalize};
static int managerSet(HoldManager* manager,const xui_callback_owner_t* owner)
{xui_manager_callback_update_t update={0};int status=xuiInternalManagerCallbackUpdateBegin(&manager->node,&manager->owner,owner!=NULL,owner,&update);if(status!=XUI_OK)return status;xuiInternalManagerCallbackUpdatePublish(&update);return xuiInternalManagerCallbackUpdateEnd(&update,XUI_OK);}
static void managerHeldFailure(void)
{
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK);HoldManager* manager=xrtCalloc(1,sizeof(*manager));assert(manager);unsigned finalized=0;manager->finalized=&finalized;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK && xuiInternalManagerRegister(context,&manager->node,&managerOps,manager)==XUI_OK && xuiCallScopeEnd(&outer)==XUI_OK);
    HoldOwner old={.refs=1},candidate={.refs=1,.fresh=true};xui_callback_owner_t oldRef=holdRef(&old),candidateRef=holdRef(&candidate);assert(managerSet(manager,&oldRef)==XUI_OK);
    assert(managerSet(manager,&candidateRef)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && candidate.refs==1 && candidate.drops==1 && old.refs==2);xrtClearError();
    xui_manager_scope_t scope={0};xui_callback_snapshot_t snapshot={0};assert(xuiInternalManagerScopeBegin(&manager->node,&scope)==XUI_OK);
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();assert(xuiInternalManagerCallbackSnapshotBegin(&manager->node,&manager->owner,&snapshot,false)==XUI_OK && xrtGetError()==ambient && old.refs==3);
    xrtClearError();assert(xuiInternalCallbackSnapshotEnd(&snapshot,XUI_OK)==XUI_OK && xuiInternalManagerScopeEnd(&scope,XUI_OK)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && finalized==1 && old.refs==1 && candidate.refs==1);
}
static void isolatedAmbient(void)
{
    HoldOwner owner={.refs=1};xui_callback_owner_t ref=holdRef(&owner);bool held=false;xrtSetErrorKind(XERR_STATE);const xerror* old=xrtGetError();
    assert(xuiInternalCallbackHoldIsolated(&ref,&held)==XUI_OK && held && xrtGetError()==old && owner.refs==2);xrtClearError();assert(holdRelease(&owner));
    owner.failHold=true;assert(xuiInternalCallbackHoldIsolated(&ref,&held)==XUI_ERROR_CALLBACK_FAILED && !held && !xrtGetError() && owner.refs==1);
}
static xstrview cellText(xui_widget widget,int row,int column,const xui_table_view_cell_output_t* cell,void* raw)
{(void)widget;(void)row;(void)column;(void)cell;++((HoldOwner*)raw)->calls;return (xstrview){0};}
static int cellRender(xui_widget widget,int row,int column,const xui_table_view_cell_output_t* cell,xui_draw_context draw,xui_rect_t rect,int state,void* raw)
{(void)widget;(void)row;(void)column;(void)cell;(void)draw;(void)rect;(void)state;++((HoldOwner*)raw)->calls;return XUI_OK;}
static void cellHeldFailure(bool formatter)
{
    xui_table_view_cell_output_t* cell=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK);
    HoldOwner old={.refs=1},candidate={.refs=1,.fresh=true};xui_callback_owner_t oldRef=holdRef(&old),candidateRef=holdRef(&candidate);
    assert((formatter ? xuiTableViewCellOutputSetFormatterOwned(cell,cellText,&old,&oldRef):xuiTableViewCellOutputSetRendererOwned(cell,cellRender,&old,&oldRef))==XUI_OK);
    assert((formatter ? xuiTableViewCellOutputSetFormatterOwned(cell,cellText,&candidate,&candidateRef):xuiTableViewCellOutputSetRendererOwned(cell,cellRender,&candidate,&candidateRef))==XUI_ERROR_CALLBACK_FAILED);
    assert(xrtErrorKind(xrtGetError())==XERR_RANGE && candidate.refs==1 && candidate.drops==1 && old.refs==2);xrtClearError();
    if(formatter){
        xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&widget)==XUI_OK);
        xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK && xuiInternalTableCellReadBegin(cell)==XUI_OK);xui_table_cell_format_t format={0};old.fresh=true;
        int status=xuiInternalTableCellFormatBegin(widget,0,0,cell,&format);assert(status==XUI_ERROR_CALLBACK_FAILED && !old.calls);
        assert(xuiInternalTableCellFormatEnd(&format,status)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && old.refs==2);
        xrtClearError();assert(xuiInternalTableCellReadEnd(cell)==XUI_OK && xuiCallScopeEnd(&outer)==XUI_OK && xuiDestroyChecked(context)==XUI_OK);old.fresh=false;
    }
    assert(xuiTableViewCellOutputReleaseChecked(cell)==XUI_OK && old.refs==1 && candidate.refs==1);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int slot=-1;slot<=XUI_EVENT_COMMAND;slot=slot<0 ? XUI_EVENT_COMMAND:XUI_EVENT_COMMAND+1){eventOwnership(slot,false);eventOwnership(slot,true);eventPrimaryNull(slot);}
    groupHeldFailure();managerHeldFailure();isolatedAmbient();cellHeldFailure(false);cellHeldFailure(true);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Owned Hold outcome: true+fresh diagnostic rejects publication but consumes its real held ref; ambient dispatch and NULL primary preserved");return 0;
}
