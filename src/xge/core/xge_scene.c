/* One transition state machine for caller-owned scenes. The stack publishes
 * activation changes before onFree, which may physically free a C record.
 * Dispatch revisions never wrap, so a callback cannot cause pointer ABA. */
static uint64_t g_xgeSceneRevision;
static xge_scene_owned g_xgeSceneOwners[XGE_SCENE_STACK_MAX];
static int __xgeSceneStackRetain(xge_scene_owned);
static int __xgeSceneStackRelease(xge_scene_owned);
/* Lifecycle callbacks may configure scheduling without nesting a stack
 * transition. Keep that dispatch invalidation independent of stack budgets. */
static uint64_t g_xgeSceneStrategyRevision;
static uint64_t g_xgeSceneCallbackRevision;

typedef struct xge_scene_outcome {
    int status;
    xerror* first;
    xerror* ambient;
} xge_scene_outcome;

static int __xgeSceneThread(void)
{
    (void)xrtSpinLock(&g_xgeGraphicsLifetime.lock);
    uint64_t creator=g_xgeGraphicsLifetime.creatorThread;
    bool initializing=g_xgeGraphicsLifetime.initializing;
    (void)xrtSpinUnlock(&g_xgeGraphicsLifetime.lock);
    if(creator && creator!=xrtThreadCurrentId())return XGE_ERROR_INVALID_STATE;
    return initializing ? XGE_ERROR_BUSY:XGE_OK;
}
static int __xgeSceneAccess(void)
{
    int status=__xgeSceneThread();
    if(status!=XGE_OK)return status;
    if(!g_xge.bInitialized)return XGE_ERROR_NOT_INITIALIZED;
    if(xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return XGE_ERROR_UNSUPPORTED;
    return XGE_OK;
}
static int __xgeSceneAdmission(unsigned changes)
{
    int status=__xgeSceneAccess();
    if(status!=XGE_OK)return status;
    if(g_xge.iSceneTransitionDepth)return XGE_ERROR_UNSUPPORTED;
    return changes>UINT64_MAX-g_xgeSceneRevision ? XGE_ERROR_INVALID_STATE:XGE_OK;
}
static xge_scene __xgeSceneTop(void)
{return g_xge.iSceneStackCount ? g_xge.arrSceneStack[g_xge.iSceneStackCount-1]:NULL;}
static bool __xgeSceneContains(xge_scene scene)
{
    for(int i=0;i<g_xge.iSceneStackCount;++i)
        if(g_xge.arrSceneStack[i]==scene)return true;
    return false;
}
static xge_scene_outcome __xgeSceneBegin(void)
{
    ++g_xge.iSceneTransitionDepth;
    return (xge_scene_outcome){XGE_OK,NULL,xrtTakeError()};
}
static void __xgeSceneRemember(xge_scene_outcome* outcome,int status)
{__xgeCallbackOutcome(status,&outcome->status,&outcome->first);}
static int __xgeSceneOutcomeEnd(xge_scene_outcome* outcome)
{
    if(outcome->status!=XGE_OK){xrtErrorFree(outcome->ambient);xrtSetErrorTake(outcome->first);}
    else xrtSetErrorTake(outcome->ambient);
    return outcome->status;
}
static int __xgeSceneEnd(xge_scene_outcome* outcome)
{--g_xge.iSceneTransitionDepth;return __xgeSceneOutcomeEnd(outcome);}
static int __xgeSceneCall(xge_scene scene,xge_scene_lifecycle_proc callback)
{
    if(!scene || !callback)return XGE_OK;
    if(!__xgeCallbackActivityEnter(true))return XGE_ERROR_BUSY;
    int status=callback(scene);
    __xgeCallbackActivityLeave();
    return status==XGE_OK && xrtGetError() ? XGE_ERROR_INVALID_STATE:status;
}
static int __xgeSceneEnter(xge_scene scene)
{
    int status=__xgeSceneCall(scene,scene->onEnter);
    if(status==XGE_OK){scene->bEntered=1;scene->bPaused=0;}
    return status;
}
static int __xgeSceneLeave(xge_scene scene)
{
    int status=scene->bEntered ? __xgeSceneCall(scene,scene->onLeave):XGE_OK;
    if(status==XGE_OK){scene->bEntered=0;scene->bPaused=0;}
    return status;
}
static int __xgeScenePause(xge_scene scene)
{
    if(!scene || scene->bPaused)return XGE_OK;
    int status=__xgeSceneCall(scene,scene->onPause);
    if(status==XGE_OK)scene->bPaused=1;
    return status;
}
static int __xgeSceneResume(xge_scene scene)
{
    if(!scene || !scene->bPaused)return XGE_OK;
    int status=__xgeSceneCall(scene,scene->onResume);
    if(status==XGE_OK)scene->bPaused=0;
    return status;
}
static xge_scene_owned __xgeSceneUnpublish(void)
{
    int index=--g_xge.iSceneStackCount;
    xge_scene_owned owner=g_xgeSceneOwners[index];
    g_xgeSceneOwners[index]=NULL;g_xge.arrSceneStack[index]=NULL;
    ++g_xgeSceneRevision;g_xge.fSceneAccumulator=0;return owner;
}
/* False means Leave rejected removal: its original entry remains published.
 * True consumes the entry even when onFree or the following Resume failed. */
static bool __xgeSceneRemove(xge_scene_outcome* outcome,bool resume)
{
    xge_scene scene=__xgeSceneTop();
    int status=__xgeSceneLeave(scene);__xgeSceneRemember(outcome,status);
    if(status!=XGE_OK)return false;
    xge_scene_lifecycle_proc release=scene->onFree;
    xge_scene_owned owner=__xgeSceneUnpublish();
    __xgeSceneRemember(outcome,__xgeSceneCall(scene,release));
    __xgeSceneRemember(outcome,__xgeSceneStackRelease(owner));
    /* Never inspect scene after onFree: a raw C callback may have freed it. */
    if(resume)__xgeSceneRemember(outcome,__xgeSceneResume(__xgeSceneTop()));
    return true;
}
/* Consumes an already reserved stack owner on both outcomes. */
static void __xgeSceneAppend(xge_scene_outcome* outcome,xge_scene scene,xge_scene_owned owner)
{
    g_xgeSceneOwners[g_xge.iSceneStackCount]=owner;
    g_xge.arrSceneStack[g_xge.iSceneStackCount++]=scene;
    ++g_xgeSceneRevision;g_xge.fSceneAccumulator=0;
    int status=__xgeSceneEnter(scene);__xgeSceneRemember(outcome,status);
    if(status!=XGE_OK)__xgeSceneRemember(outcome,__xgeSceneStackRelease(__xgeSceneUnpublish()));
}
static void __xgeSceneClearBody(xge_scene_outcome* outcome)
{
    while(g_xge.iSceneStackCount)
        if(!__xgeSceneRemove(outcome,false))break;
}
/* Unit has already closed creation/reentrant admission. It calls this before
 * clearing other publishers or marking the initialized engine unavailable. */
static int __xgeSceneClear(void)
{
    if((unsigned)g_xge.iSceneStackCount>UINT64_MAX-g_xgeSceneRevision)return XGE_ERROR_INVALID_STATE;
    xge_scene_outcome outcome=__xgeSceneBegin();
    __xgeSceneClearBody(&outcome);return __xgeSceneEnd(&outcome);
}
int xgeSceneClear(void)
{
    int status=__xgeSceneAdmission(XGE_SCENE_STACK_MAX);
    return status==XGE_OK ? __xgeSceneClear():status;
}
static int __xgeSceneSetValue(xge_scene scene,xge_scene_owned owner)
{
    int status=__xgeSceneAdmission(XGE_SCENE_STACK_MAX+2u);
    if(status!=XGE_OK)return status;
    if(scene && __xgeSceneContains(scene))
        return g_xge.iSceneStackCount==1 && __xgeSceneTop()==scene ? XGE_OK:XGE_ERROR_INVALID_STATE;
    xge_scene_outcome outcome=__xgeSceneBegin();
    status=__xgeSceneStackRetain(owner);__xgeSceneRemember(&outcome,status);
    if(status==XGE_OK){
        __xgeSceneClearBody(&outcome);
        if(outcome.status==XGE_OK && scene){__xgeSceneAppend(&outcome,scene,owner);owner=NULL;}
        __xgeSceneRemember(&outcome,__xgeSceneStackRelease(owner));
    }
    return __xgeSceneEnd(&outcome);
}
static int __xgeScenePushValue(xge_scene scene,xge_scene_owned owner)
{
    int status=__xgeSceneAdmission(2);
    if(status!=XGE_OK)return status;
    if(!scene)return XGE_ERROR_INVALID_ARGUMENT;
    if(__xgeSceneContains(scene))return XGE_ERROR_INVALID_STATE;
    if(g_xge.iSceneStackCount>=XGE_SCENE_STACK_MAX)return XGE_ERROR_OUT_OF_MEMORY;
    xge_scene_outcome outcome=__xgeSceneBegin();xge_scene previous=__xgeSceneTop();
    status=__xgeSceneStackRetain(owner);__xgeSceneRemember(&outcome,status);
    if(status==XGE_OK){
        status=__xgeScenePause(previous);__xgeSceneRemember(&outcome,status);
        if(status==XGE_OK){
            __xgeSceneAppend(&outcome,scene,owner);owner=NULL;
            if(outcome.status!=XGE_OK)__xgeSceneRemember(&outcome,__xgeSceneResume(previous));
        }
        __xgeSceneRemember(&outcome,__xgeSceneStackRelease(owner));
    }
    return __xgeSceneEnd(&outcome);
}
int xgeScenePop(void)
{
    int status=__xgeSceneAdmission(1);
    if(status!=XGE_OK)return status;
    if(!g_xge.iSceneStackCount)return XGE_ERROR_INVALID_ARGUMENT;
    xge_scene_outcome outcome=__xgeSceneBegin();
    (void)__xgeSceneRemove(&outcome,true);return __xgeSceneEnd(&outcome);
}
static int __xgeSceneReplaceValue(xge_scene scene,xge_scene_owned owner)
{
    int status=__xgeSceneAdmission(3);
    if(status!=XGE_OK)return status;
    if(!scene)return XGE_ERROR_INVALID_ARGUMENT;
    if(__xgeSceneTop()==scene)return XGE_OK;
    if(__xgeSceneContains(scene))return XGE_ERROR_INVALID_STATE;
    if(!g_xge.iSceneStackCount)return __xgeScenePushValue(scene,owner);
    xge_scene_outcome outcome=__xgeSceneBegin();
    status=__xgeSceneStackRetain(owner);__xgeSceneRemember(&outcome,status);
    if(status==XGE_OK){
        if(__xgeSceneRemove(&outcome,false)){
            if(outcome.status==XGE_OK){__xgeSceneAppend(&outcome,scene,owner);owner=NULL;}
            if(outcome.status!=XGE_OK)__xgeSceneRemember(&outcome,__xgeSceneResume(__xgeSceneTop()));
        }
        __xgeSceneRemember(&outcome,__xgeSceneStackRelease(owner));
    }
    return __xgeSceneEnd(&outcome);
}
xge_scene xgeSceneCurrent(void)
{
    if(__xgeSceneThread()!=XGE_OK){xrtSetErrorKind(XERR_STATE);return NULL;}
    return g_xge.bInitialized ? __xgeSceneTop():NULL;
}
int xgeSceneCount(void)
{
    if(__xgeSceneThread()!=XGE_OK){xrtSetErrorKind(XERR_STATE);return 0;}
    return g_xge.bInitialized ? g_xge.iSceneStackCount:0;
}
int xgeSceneDispatchEvent(const xge_event_t* event)
{
    int status=__xgeSceneAccess();
    if(status!=XGE_OK)return status;
    if(!event)return XGE_ERROR_INVALID_ARGUMENT;
    xge_scene scene=__xgeSceneTop();
    if(!scene || !scene->onEvent)return XGE_OK;
    xerror* ambient=xrtTakeError();
    if(!__xgeCallbackActivityEnter(false)){xrtSetErrorTake(ambient);return XGE_ERROR_BUSY;}
    status=scene->onEvent(scene,event);
    __xgeCallbackActivityLeave();
    xge_scene_outcome outcome={XGE_OK,NULL,ambient};__xgeSceneRemember(&outcome,status);
    return __xgeSceneOutcomeEnd(&outcome);
}
int xgeSceneUpdateStrategySet(int mode,float step,int maximum)
{
    int status=__xgeSceneAccess();
    if(status!=XGE_OK)return status;
    if((mode!=XGE_UPDATE_VARIABLE && mode!=XGE_UPDATE_FIXED) || !isfinite(step) || step<=0 || maximum<=0)
        return XGE_ERROR_INVALID_ARGUMENT;
    if(g_xgeSceneStrategyRevision==UINT64_MAX)return XGE_ERROR_INVALID_STATE;
    g_xge.iSceneUpdateMode=mode;g_xge.fSceneFixedStep=step;g_xge.iSceneMaxUpdates=maximum;
    g_xge.fSceneAccumulator=0;++g_xgeSceneStrategyRevision;return XGE_OK;
}
void xgeSceneUpdateStrategyGet(int* mode,float* step,int* maximum)
{
    if(__xgeSceneThread()!=XGE_OK){xrtSetErrorKind(XERR_STATE);return;}
    if(mode)*mode=g_xge.iSceneUpdateMode;
    if(step)*step=g_xge.fSceneFixedStep;
    if(maximum)*maximum=g_xge.iSceneMaxUpdates;
}
static int __xgeSceneFrame(void)
{
    int status=__xgeSceneAdmission(0);
    if(status!=XGE_OK)return status;
    if(g_xgeSceneStepping)return XGE_ERROR_BUSY;
    xge_scene scene=__xgeSceneTop();if(!scene)return XGE_OK;
    uint64_t revision=g_xgeSceneRevision;
    uint64_t strategyRevision=g_xgeSceneStrategyRevision;
    uint64_t callbackRevision=g_xgeSceneCallbackRevision;
    float delta=g_xge.fDelta;
    xge_scene_outcome outcome={XGE_OK,NULL,xrtTakeError()};
    g_xgeSceneStepping=true;
    if(scene->onUpdate){
        int maximum=g_xge.iSceneUpdateMode==XGE_UPDATE_FIXED ? g_xge.iSceneMaxUpdates:1;
        bool fixed=g_xge.iSceneUpdateMode==XGE_UPDATE_FIXED;
        if(fixed)g_xge.fSceneAccumulator+=delta;
        int updates=0;
        while(updates<maximum && (!fixed || g_xge.fSceneAccumulator>=g_xge.fSceneFixedStep)){
            g_xge.fDelta=fixed ? g_xge.fSceneFixedStep:delta;
            if(!__xgeCallbackActivityEnter(false)){__xgeSceneRemember(&outcome,XGE_ERROR_BUSY);goto finish;}
            status=scene->onUpdate(scene,g_xge.fDelta);
            __xgeCallbackActivityLeave();__xgeSceneRemember(&outcome,status);
            /* Check identity BEFORE any further access to the old record or
             * accumulator. A transition/strategy change owns the next frame. */
            if(outcome.status!=XGE_OK || revision!=g_xgeSceneRevision || strategyRevision!=g_xgeSceneStrategyRevision || callbackRevision!=g_xgeSceneCallbackRevision)goto finish;
            if(fixed)g_xge.fSceneAccumulator-=g_xge.fSceneFixedStep;
            ++updates;
        }
        if(fixed && updates==maximum)g_xge.fSceneAccumulator=0;
    }
    g_xge.fDelta=delta;
    __xgeSceneRemember(&outcome,__xgeSceneCall(scene,scene->onDraw));
finish:
    g_xge.fDelta=delta;g_xgeSceneStepping=false;
    return __xgeSceneOutcomeEnd(&outcome);
}

int xgeSceneSet(xge_scene scene){return __xgeSceneSetValue(scene,NULL);}
int xgeScenePush(xge_scene scene){return __xgeScenePushValue(scene,NULL);}
int xgeSceneReplace(xge_scene scene){return __xgeSceneReplaceValue(scene,NULL);}
#include "xge_scene_owned.c"
