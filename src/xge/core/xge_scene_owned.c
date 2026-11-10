/* Exact typed callbacks over the shared Scene transition kernel. The stack
 * and every active invocation own real references. No raw record escapes
 * from the managed handle; caller-owned C scenes remain a separate API. */
#include <stdatomic.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include "xge_data_lifetime_internal.h"

enum {
    XGE_SCENE_ENTER,XGE_SCENE_LEAVE,XGE_SCENE_PAUSE,XGE_SCENE_RESUME,
    XGE_SCENE_DRAW,XGE_SCENE_FREE,XGE_SCENE_EVENT,XGE_SCENE_UPDATE,
    XGE_SCENE_CALLBACKS
};
struct xge_scene_owned_t {
    atomic_uint refs;
    bool destroyed;
    xge_scene_t raw;
    xge_scene_proc enter,leave,pause,resume,draw,free;
    xge_scene_owned_event_proc event;
    xge_scene_owned_update_proc update;
    void* user[XGE_SCENE_CALLBACKS];
    xge_callback_node_t* owner[XGE_SCENE_CALLBACKS];
    xge_callback_owner_t code;
};
static xge_data_lifetime_t* __xgeOwnedSceneRoot(xge_scene_owned scene)
{return scene ? (xge_data_lifetime_t*)(scene+1):NULL;}
static bool __xgeSceneObjectReadable(const void* data)
{return !((const struct xge_scene_owned_t*)data)->destroyed;}
static bool __xgeSceneObjectTrace(const void*,xrtownershipvisitor,void*);
static int __xgeSceneObjectFinalize(void*,bool*);
static int __xgeSceneObjectFinish(void*);
static const xge_data_lifetime_type_t __xgeOwnedSceneType={
    sizeof(struct xge_scene_owned_t),offsetof(struct xge_scene_owned_t,refs),
    __xgeSceneObjectReadable,__xgeSceneObjectTrace,__xgeSceneObjectFinalize,__xgeSceneObjectFinish};
#define DO_FN(name) __xgeOwnedScene##name
#define DO_PUBLIC(name) xgeScene##name
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(scene) __xgeOwnedSceneRoot(scene)
#define DO_HANDLE xge_scene_owned
#define DO_OBJECT_TYPE __xgeOwnedSceneType
#define DO_GENERATION_TYPE __xgeOwnedSceneType
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE

static bool __xgeSceneObjectTrace(const void* data,xrtownershipvisitor visit,void* user)
{
    const struct xge_scene_owned_t* scene=data;
    if(scene->code.onDrop && !visit(scene->code.tReference,user))return false;
    for(unsigned i=0;i<XGE_SCENE_CALLBACKS;++i)
        if(scene->owner[i] && !visit(scene->owner[i]->tOwner.tReference,user))return false;
    return true;
}
static int __xgeSceneObjectFinalize(void* data,bool* complete)
{
    xge_scene_owned scene=data;xge_callback_node_t* owners[XGE_SCENE_CALLBACKS];
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    scene->destroyed=true;
    memcpy(owners,scene->owner,sizeof(owners));memset(scene->owner,0,sizeof(scene->owner));
    memset(scene->user,0,sizeof(scene->user));
    scene->enter=scene->leave=scene->pause=scene->resume=scene->draw=scene->free=NULL;
    scene->event=NULL;scene->update=NULL;
    __xgeOwnedSceneMutationEnd(&mutation);
    xge_data_outcome_t result={0};__xgeOwnedSceneOutcomeBegin(&result,XGE_OK);
    for(unsigned i=0;i<XGE_SCENE_CALLBACKS;++i)
        __xgeOwnedSceneOutcomeAdd(&result,__xgeCallbackNodeRelease(owners[i]));
    *complete=true;xrtSetErrorTake(result.pError);return result.iStatus;
}
static int __xgeSceneObjectFinish(void* data)
{
    xge_scene_owned scene=data;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t code=scene->code;scene->code=(xge_callback_owner_t){0};
    __xgeOwnedSceneMutationEnd(&mutation);
    /* The resident Drop hook protects the complete native return tail. */
    return __xgeCallbackOwnerDrop(code);
}
static int __xgeSceneStackRetain(xge_scene_owned scene)
{return scene ? __xgeOwnedSceneAcquire(__xgeOwnedSceneRoot(scene),true,false):XGE_OK;}
static int __xgeSceneStackRelease(xge_scene_owned scene)
{
    if(!scene)return XGE_OK;
    bool consumed=false;return __xgeOwnedSceneRelease(__xgeOwnedSceneRoot(scene),&consumed);
}
int xgeSceneCreateOwned(xge_scene_owned* output)
{return xgeSceneCreateOwnedWithCode(output,NULL);}
int xgeSceneCreateOwnedWithCode(xge_scene_owned* output,const xge_callback_owner_t* codeOwner)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;
    if(!__xgeCallbackOwnerValid(codeOwner))return XGE_ERROR_INVALID_ARGUMENT;
    xge_callback_owner_t code=codeOwner ? *codeOwner:(xge_callback_owner_t){0};
    bool held=false;int status=__xgeCallbackOwnerHold(code,&held);
    if(status!=XGE_OK){
        xerror* error=xrtTakeError();
        if(held)__xgeCallbackOutcome(__xgeCallbackOwnerDrop(code),&status,&error);
        xrtSetErrorTake(error);return status;
    }
    xge_scene_owned scene=xrtCalloc(1,sizeof(*scene)+sizeof(xge_data_lifetime_t));
    if(!scene){
        xerror* error=xrtTakeError();status=XGE_ERROR_OUT_OF_MEMORY;
        if(held)__xgeCallbackOutcome(__xgeCallbackOwnerDrop(code),&status,&error);
        xrtSetErrorTake(error);return status;
    }
    atomic_init(&scene->refs,1u);scene->raw.pUser=scene;
    scene->code=code;
    __xgeOwnedSceneInit(__xgeOwnedSceneRoot(scene),&__xgeOwnedSceneType,xrtThreadCurrentId());
    *output=scene;return XGE_OK;
}
int xgeSceneShareOwned(xge_scene_owned scene,xge_scene_owned* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=__xgeSceneStackRetain(scene);
    if(status==XGE_OK && !scene)status=XGE_ERROR_INVALID_ARGUMENT;
    if(status==XGE_OK)*output=scene;
    return status;
}
int xgeSceneReleaseOwnerChecked(xge_scene_owned scene,bool* consumed)
{return __xgeOwnedSceneRelease(__xgeOwnedSceneRoot(scene),consumed);}
int xgeSceneReleaseChecked(xge_scene_owned scene)
{bool consumed=false;return xgeSceneReleaseOwnerChecked(scene,&consumed);}
int xgeSceneSetOwned(xge_scene_owned scene)
{
    int status=scene ? __xgeOwnedSceneAdmit(__xgeOwnedSceneRoot(scene)):XGE_OK;
    return status==XGE_OK ? __xgeSceneSetValue(scene ? &scene->raw:NULL,scene):status;
}
int xgeScenePushOwned(xge_scene_owned scene)
{
    int status=__xgeOwnedSceneAdmit(__xgeOwnedSceneRoot(scene));
    return status==XGE_OK ? __xgeScenePushValue(&scene->raw,scene):status;
}
int xgeSceneReplaceOwned(xge_scene_owned scene)
{
    int status=__xgeOwnedSceneAdmit(__xgeOwnedSceneRoot(scene));
    return status==XGE_OK ? __xgeSceneReplaceValue(&scene->raw,scene):status;
}
int xgeSceneCurrentOwned(xge_scene_owned* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=__xgeSceneAccess();if(status!=XGE_OK)return status;
    if(!g_xge.iSceneStackCount)return XGE_OK;
    xge_scene_owned scene=g_xgeSceneOwners[g_xge.iSceneStackCount-1];
    return scene ? xgeSceneShareOwned(scene,output):XGE_ERROR_UNSUPPORTED;
}
int xgeSceneEnteredGet(xge_scene_owned scene,bool* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeOwnedSceneAdmit(__xgeOwnedSceneRoot(scene));
    if(status==XGE_OK)*output=scene->raw.bEntered!=0;
    return status;
}
int xgeScenePausedGet(xge_scene_owned scene,bool* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeOwnedSceneAdmit(__xgeOwnedSceneRoot(scene));
    if(status==XGE_OK)*output=scene->raw.bPaused!=0;
    return status;
}
static int __xgeOwnedSceneAdmissionEnd(int status,xerror* ambient)
{
    xge_data_outcome_t result={0};__xgeOwnedSceneOutcomeBegin(&result,status);
    return __xgeOwnedSceneOutcomeEnd(&result,ambient);
}
/* Body activity spans the complete callback, owner snapshot and cleanup tail.
 * An invocation can clear/replace its own slot or remove its scene safely. */
static int __xgeOwnedSceneSnapshot(xge_scene_owned scene,unsigned slot,xge_callback_snapshot_t* snapshot)
{
    xge_callback_node_t* node=scene->owner[slot];
    if(!__xgeCallbackNodePin(node))return XGE_ERROR_INVALID_STATE;
    return __xgeCallbackSnapshotBegin(node,true,snapshot);
}
static int __xgeOwnedSceneInvocationEnd(xge_scene_owned scene,xge_callback_snapshot_t* snapshot,int status,xerror* ambient)
{
    if(status>0)status=XGE_ERROR_CALLBACK_FAILED;
    xge_data_outcome_t result={0};__xgeOwnedSceneOutcomeBegin(&result,status);
    __xgeOwnedSceneOutcomeAdd(&result,__xgeCallbackSnapshotEnd(snapshot));
    __xgeOwnedSceneOutcomeAdd(&result,__xgeOwnedSceneActivityEnd(__xgeOwnedSceneRoot(scene)));
    /* ActivityEnd may have physically freed scene. */
    return __xgeOwnedSceneOutcomeEnd(&result,ambient);
}
#define XGE_SCENE_LIFECYCLE(Name,Field,Slot) \
static int __xgeOwnedScene##Name(xge_scene raw) \
{ \
    xge_scene_owned scene=raw->pUser;xerror* ambient=xrtTakeError(); \
    int status=__xgeOwnedSceneAcquire(__xgeOwnedSceneRoot(scene),true,true); \
    if(status!=XGE_OK)return __xgeOwnedSceneAdmissionEnd(status,ambient); \
    xge_scene_proc proc=scene->Field;void* user=scene->user[Slot]; \
    xge_callback_snapshot_t snapshot={0};status=__xgeOwnedSceneSnapshot(scene,Slot,&snapshot); \
    if(status==XGE_OK && proc)status=proc(user); \
    return __xgeOwnedSceneInvocationEnd(scene,&snapshot,status,ambient); \
}
XGE_SCENE_LIFECYCLE(Enter,enter,XGE_SCENE_ENTER)
XGE_SCENE_LIFECYCLE(Leave,leave,XGE_SCENE_LEAVE)
XGE_SCENE_LIFECYCLE(Pause,pause,XGE_SCENE_PAUSE)
XGE_SCENE_LIFECYCLE(Resume,resume,XGE_SCENE_RESUME)
XGE_SCENE_LIFECYCLE(Draw,draw,XGE_SCENE_DRAW)
XGE_SCENE_LIFECYCLE(Free,free,XGE_SCENE_FREE)
#undef XGE_SCENE_LIFECYCLE
static int __xgeOwnedSceneEvent(xge_scene raw,const xge_event_t* event)
{
    xge_scene_owned scene=raw->pUser;xerror* ambient=xrtTakeError();
    int status=__xgeOwnedSceneAcquire(__xgeOwnedSceneRoot(scene),true,true);
    if(status!=XGE_OK)return __xgeOwnedSceneAdmissionEnd(status,ambient);
    xge_scene_owned_event_proc proc=scene->event;void* user=scene->user[XGE_SCENE_EVENT];
    xge_callback_snapshot_t snapshot={0};status=__xgeOwnedSceneSnapshot(scene,XGE_SCENE_EVENT,&snapshot);
    if(status==XGE_OK && proc)status=proc(event,user);
    return __xgeOwnedSceneInvocationEnd(scene,&snapshot,status,ambient);
}
static int __xgeOwnedSceneUpdate(xge_scene raw,float delta)
{
    xge_scene_owned scene=raw->pUser;xerror* ambient=xrtTakeError();
    int status=__xgeOwnedSceneAcquire(__xgeOwnedSceneRoot(scene),true,true);
    if(status!=XGE_OK)return __xgeOwnedSceneAdmissionEnd(status,ambient);
    xge_scene_owned_update_proc proc=scene->update;void* user=scene->user[XGE_SCENE_UPDATE];
    xge_callback_snapshot_t snapshot={0};status=__xgeOwnedSceneSnapshot(scene,XGE_SCENE_UPDATE,&snapshot);
    if(status==XGE_OK && proc)status=proc(delta,user);
    return __xgeOwnedSceneInvocationEnd(scene,&snapshot,status,ambient);
}
/* Six lifecycle slots and the two distinct event/update signatures share
 * publication mechanics, never a function-pointer cast or opcode call. */
#define XGE_SCENE_SETTER(Name,Field,Slot,Type) \
int xgeScene##Name##CallbackSetOwned(xge_scene_owned scene,Type callback,void* user,const xge_callback_owner_t* owner) \
{ \
    xerror* ambient=xrtTakeError(); \
    int status=__xgeOwnedSceneAcquire(__xgeOwnedSceneRoot(scene),true,true); \
    if(status!=XGE_OK)return __xgeOwnedSceneAdmissionEnd(status,ambient); \
    xge_callback_node_t* next=NULL; \
    if((callback && xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE)) || \
       (__xgeSceneTop()==&scene->raw && g_xgeSceneCallbackRevision==UINT64_MAX))status=XGE_ERROR_INVALID_STATE; \
    else status=__xgeCallbackNodePrepare(callback!=NULL,false,owner,&next); \
    xge_data_outcome_t result={0};__xgeOwnedSceneOutcomeBegin(&result,status); \
    if(status==XGE_OK){ \
        xrtownershipscope mutation={0}; \
        if(!xrtOwnershipMutationBegin(&mutation)){ \
            __xgeOwnedSceneOutcomeAdd(&result,XGE_ERROR_INVALID_STATE); \
            __xgeOwnedSceneOutcomeAdd(&result,__xgeCallbackNodeRelease(next)); \
        }else{ \
            xge_callback_node_t* old=scene->owner[Slot];scene->owner[Slot]=next; \
            scene->user[Slot]=callback ? user:NULL;scene->Field=callback; \
            scene->raw.on##Name=callback ? __xgeOwnedScene##Name:NULL; \
            if(__xgeSceneTop()==&scene->raw){++g_xgeSceneCallbackRevision;g_xge.fSceneAccumulator=0;} \
            __xgeOwnedSceneMutationEnd(&mutation); \
            __xgeOwnedSceneOutcomeAdd(&result,__xgeCallbackNodeRelease(old)); \
        } \
    } \
    __xgeOwnedSceneOutcomeAdd(&result,__xgeOwnedSceneActivityEnd(__xgeOwnedSceneRoot(scene))); \
    return __xgeOwnedSceneOutcomeEnd(&result,ambient); \
}
XGE_SCENE_SETTER(Enter,enter,XGE_SCENE_ENTER,xge_scene_proc)
XGE_SCENE_SETTER(Leave,leave,XGE_SCENE_LEAVE,xge_scene_proc)
XGE_SCENE_SETTER(Pause,pause,XGE_SCENE_PAUSE,xge_scene_proc)
XGE_SCENE_SETTER(Resume,resume,XGE_SCENE_RESUME,xge_scene_proc)
XGE_SCENE_SETTER(Draw,draw,XGE_SCENE_DRAW,xge_scene_proc)
XGE_SCENE_SETTER(Free,free,XGE_SCENE_FREE,xge_scene_proc)
XGE_SCENE_SETTER(Event,event,XGE_SCENE_EVENT,xge_scene_owned_event_proc)
XGE_SCENE_SETTER(Update,update,XGE_SCENE_UPDATE,xge_scene_owned_update_proc)
#undef XGE_SCENE_SETTER
