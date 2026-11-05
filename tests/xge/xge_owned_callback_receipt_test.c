/* Real shared publisher kernel, no engine/window/device or fake ownership. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <assert.h>
#include <string.h>
#include "src/xge/core/xge_callback_owned.h"

typedef struct Owner {
    unsigned refs,holds,drops,dropFailAt,dropErrorAt;
    bool holdResult,holdError,dropClear;
    xge_callback_node_t* unpublish;
} Owner;
static bool count(const void* raw,size_t* output){*output=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;++owner->holds;
    if(owner->holdResult)++owner->refs;
    if(owner->unpublish){xge_callback_node_t* node=owner->unpublish;owner->unpublish=NULL;assert(__xgeCallbackNodeRelease(node)==XGE_OK);}
    if(owner->holdError)xrtSetErrorKind(XERR_ARGUMENT);
    return owner->holdResult;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs>1);--owner->refs;++owner->drops;
    if(owner->dropClear)xrtClearError();
    if(owner->dropErrorAt==owner->drops)xrtSetErrorKind(XERR_STATE);
    return owner->dropFailAt!=owner->drops;
}
static xge_callback_owner_t descriptor(Owner* owner)
{return (xge_callback_owner_t){{owner,&ops},hold,drop};}
static Owner fresh(void){Owner result={0};result.refs=1;result.holdResult=true;return result;}
static void idle(Owner* owner)
{assert(owner->refs==1 && !xrtAtomic64Load(&g_xgeCallbackActivity,XMEMORY_RELAXED));xrtClearError();}
static void prepareSquares(bool oom)
{
    for(unsigned held=0;held<2;++held)for(unsigned error=0;error<2;++error){
        Owner owner=fresh();owner.holdResult=held;owner.holdError=error;
        xge_callback_owner_t record=descriptor(&owner);xge_callback_node_t* node=(void*)1;
        if(oom)assert(xrtMemDebugFailAfter(0));
        int result=__xgeCallbackNodePrepare(true,false,&record,&node);
        if(oom)xrtMemDebugFailClear();
        bool accepted=held && !error && !oom;
        assert((result==XGE_OK)==accepted);
        if(accepted){assert(node && owner.refs==2);assert(__xgeCallbackNodeRelease(node)==XGE_OK);}
        else{assert(!node);assert(owner.drops==(held ? 1u:0u));}
        idle(&owner);
    }
}
static void snapshotSquares(bool oom)
{
    for(unsigned held=0;held<2;++held)for(unsigned error=0;error<2;++error){
        Owner owner=fresh();xge_callback_owner_t record=descriptor(&owner);xge_callback_node_t* node=NULL;
        assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK);
        assert(__xgeCallbackNodePin(node));owner.holdResult=held;owner.holdError=error;
        if(oom)assert(xrtMemDebugFailAfter(0));
        xge_callback_snapshot_t snapshot={0};int result=__xgeCallbackSnapshotBegin(node,false,&snapshot);
        if(oom)xrtMemDebugFailClear();
        assert((result==XGE_OK)==(held && !error));
        if(result==XGE_OK)assert(__xgeCallbackSnapshotEnd(&snapshot)==XGE_OK);
        else assert(!snapshot.bHeld && !snapshot.bActive);
        xrtClearError();assert(__xgeCallbackNodeRelease(node)==XGE_OK);idle(&owner);
    }
}
static void bodyAndCleanup(void)
{
    Owner owner=fresh();xge_callback_owner_t record=descriptor(&owner);xge_callback_node_t* node=NULL;
    assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK && __xgeCallbackNodePin(node));
    xge_callback_snapshot_t snapshot={0};assert(__xgeCallbackSnapshotBegin(node,false,&snapshot)==XGE_OK);
    xrtSetErrorKind(XERR_ARGUMENT);const xerror* first=xrtGetError();assert(first);
    owner.dropClear=true;assert(__xgeCallbackSnapshotEnd(&snapshot)==XGE_ERROR_INVALID_STATE && xrtGetError()==first);
    xrtClearError();owner.dropClear=false;assert(__xgeCallbackNodeRelease(node)==XGE_OK);idle(&owner);
    owner=fresh();record=descriptor(&owner);assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK && __xgeCallbackNodePin(node));
    snapshot=(xge_callback_snapshot_t){0};assert(__xgeCallbackSnapshotBegin(node,false,&snapshot)==XGE_OK);
    owner.dropErrorAt=owner.drops+1;int result=-71;xerror* nullFirst=NULL;
    __xgeCallbackOutcome(__xgeCallbackSnapshotEnd(&snapshot),&result,&nullFirst);
    assert(result==-71 && !nullFirst && !xrtGetError());
    owner.dropErrorAt=0;assert(__xgeCallbackNodeRelease(node)==XGE_OK);idle(&owner);
}
static void reentrantRelease(void)
{
    Owner owner=fresh();xge_callback_owner_t record=descriptor(&owner);xge_callback_node_t* node=NULL;
    assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK && __xgeCallbackNodePin(node));
    owner.unpublish=node;owner.dropFailAt=1;owner.dropErrorAt=2;
    xge_callback_snapshot_t snapshot={0};assert(__xgeCallbackSnapshotBegin(node,false,&snapshot)==XGE_ERROR_INVALID_STATE);
    assert(!xrtGetError() && owner.drops==2);idle(&owner);
    owner=fresh();record=descriptor(&owner);assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK);
    owner.dropErrorAt=1;xrtAtomic32Store(&g_xgeCallbackShutdown,1,XMEMORY_RELEASE);
    snapshot=(xge_callback_snapshot_t){0};assert(__xgeCallbackSnapshotBegin(node,false,&snapshot)==XGE_ERROR_INVALID_STATE);
    assert(!xrtGetError());xrtAtomic32Store(&g_xgeCallbackShutdown,0,XMEMORY_RELEASE);idle(&owner);
}
int main(void)
{
    prepareSquares(false);prepareSquares(true);snapshotSquares(false);snapshotSquares(true);
    bodyAndCleanup();reentrantRelease();
    Owner owner=fresh();xge_callback_owner_t record=descriptor(&owner);xge_callback_node_t* node=NULL;
    xrtSetErrorKind(XERR_ARGUMENT);const xerror* ambient=xrtGetError();
    assert(__xgeCallbackNodePrepare(true,false,&record,&node)==XGE_OK && node && xrtGetError()==ambient);
    assert(__xgeCallbackNodeRelease(node)==XGE_OK && xrtGetError()==ambient);idle(&owner);
    return 0;
}
