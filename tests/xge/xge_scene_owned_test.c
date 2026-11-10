/* Real native Scene lifetime/transition/owner publication; no OS window. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Owner {
    unsigned refs,calls,events,updates,draws,frees,pauses,resumes,leaves;
    unsigned holds,drops,failHold,failDrop,reenter;
    xge_scene_owned scene,other;
} Owner;
static bool count(const void* value,size_t* output)
{*output=((const Owner*)value)->refs;return true;}
static bool trace(const void* value,xrtownershipvisitor visit,void* user)
{(void)value;(void)visit;(void)user;return true;}
static const xrtownershipops ownerOps={count,trace};
static bool hold(const void* value)
{
    Owner* owner=(Owner*)value;
    if(owner->failHold){--owner->failHold;return false;}
    ++owner->refs;++owner->holds;
    if(owner->reenter==2){owner->reenter=0;assert(xgeSceneUpdateCallbackSetOwned(owner->scene,NULL,NULL,NULL)==XGE_OK);}
    return true;
}
static bool drop(const void* value)
{
    Owner* owner=(Owner*)value;assert(owner->refs>1);--owner->refs;++owner->drops;
    if(owner->failDrop){--owner->failDrop;return false;}return true;
}
static xge_callback_owner_t descriptor(Owner* owner)
{return (xge_callback_owner_t){{owner,&ownerOps},hold,drop};}
static size_t references(xge_scene_owned scene)
{xrtownershipref ref=xgeSceneOwnership(scene);size_t refs=0;assert(ref.Ops && ref.Ops->Count(ref.Data,&refs));return refs;}
static unsigned visited;
static bool visit(xrtownershipref ref,void* expected)
{assert(ref.Ops==&ownerOps && ref.Data==expected);++visited;return true;}
static int entered(void* user){++((Owner*)user)->calls;return XGE_OK;}
static int left(void* user){++((Owner*)user)->leaves;return XGE_OK;}
static int paused(void* user){++((Owner*)user)->pauses;return XGE_OK;}
static int resumed(void* user){++((Owner*)user)->resumes;return XGE_OK;}
static int freed(void* user){++((Owner*)user)->frees;return XGE_OK;}
static int draw(void* user){++((Owner*)user)->draws;return XGE_OK;}
static int fail(void* user){++((Owner*)user)->calls;return -71;}
static int event(const xge_event_t* event,void* user)
{assert(event);++((Owner*)user)->events;return XGE_OK;}
static int update(float delta,void* user)
{
    assert(delta>0);Owner* owner=user;++owner->updates;
    if(owner->reenter==1){
        assert(xgeSceneUpdateCallbackSetOwned(owner->scene,NULL,NULL,NULL)==XGE_OK);
        assert(owner->refs>=2); /* caller and actual callback return snapshot */
    }else if(owner->reenter==3){
        assert(xgeSceneReplaceOwned(owner->other)==XGE_OK);
        bool consumed=false;assert(xgeSceneReleaseOwnerChecked(owner->scene,&consumed)==XGE_OK && consumed);
        assert(owner->refs>=2 && owner->frees==1);owner->scene=NULL;
    }
    return XGE_OK;
}
static int free_fail(void* user){++((Owner*)user)->frees;return -73;}
static int32 foreign_thread(void* user)
{
    xge_scene_owned scene=user;bool output=true,consumed=true;
    assert(xgeSceneEnteredGet(scene,&output)==XGE_ERROR_BUSY && output);
    assert(xgeSceneUpdateCallbackSetOwned(scene,update,NULL,NULL)==XGE_ERROR_BUSY);
    assert(xgeSceneReleaseOwnerChecked(scene,&consumed)==XGE_ERROR_BUSY && !consumed);
    return 0;
}
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;for(size_t i=0;i<sizeof(symbols)/sizeof(*symbols);++i)if(!strcmp(symbols[i].name,name))return symbols[i].proc;return NULL;}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);
    static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&desc)==XGE_OK);
    xge_scene_owned scene=NULL,other=NULL,current=NULL;
    assert(xgeSceneCreateOwned(&scene)==XGE_OK && references(scene)==1);
    assert(xgeSceneCreateOwned(&other)==XGE_OK);
    Owner owner={0};owner.refs=1;owner.scene=scene;owner.other=other;
    xge_callback_owner_t own=descriptor(&owner);
    if(mode==1){
        assert(xgeSceneShareOwned(scene,&current)==XGE_OK && references(scene)==2);
        assert(xgeSceneReleaseChecked(current)==XGE_OK);current=NULL;
        assert(xgeSceneSetOwned(scene)==XGE_OK && references(scene)==2);
        assert(xgeSceneSetOwned(scene)==XGE_OK && references(scene)==2);
        assert(xgeSceneCurrentOwned(&current)==XGE_OK && current==scene && references(scene)==3);
    }else if(mode==2){
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xgeSceneFreeCallbackSetOwned(scene,freed,&owner,&own)==XGE_OK);
        assert(owner.refs==3);
        xrtownershipref ref=xgeSceneOwnership(scene);visited=0;assert(ref.Ops->Trace(ref.Data,visit,&owner) && visited==2);
        assert(xgeSceneSetOwned(scene)==XGE_OK);assert(xgeSceneReleaseChecked(scene)==XGE_OK);scene=NULL;
        assert(xgeFrame()==1 && owner.draws==1);
        assert(xgeSceneCurrentOwned(&current)==XGE_OK && current);
        assert(xgeScenePop()==XGE_OK && owner.frees==1 && owner.refs==3);
    }else if(mode==3){
        assert(xgeSceneEnterCallbackSetOwned(scene,entered,&owner,&own)==XGE_OK);
        assert(xgeSceneLeaveCallbackSetOwned(scene,left,&owner,&own)==XGE_OK);
        assert(xgeScenePauseCallbackSetOwned(scene,paused,&owner,&own)==XGE_OK);
        assert(xgeSceneResumeCallbackSetOwned(scene,resumed,&owner,&own)==XGE_OK);
        assert(xgeSceneEventCallbackSetOwned(scene,event,&owner,&own)==XGE_OK);
        assert(xgeSceneUpdateCallbackSetOwned(scene,update,&owner,&own)==XGE_OK);
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK && owner.calls==1);
        assert(xgeScenePushOwned(other)==XGE_OK && owner.pauses==1);
        bool output=false;assert(xgeScenePausedGet(scene,&output)==XGE_OK && output);
        assert(xgeScenePop()==XGE_OK && owner.resumes==1);
        xge_event_t ev={0};assert(xgeSceneDispatchEvent(&ev)==XGE_OK && owner.events==1);
        assert(xgeFrame()==1 && owner.updates==1 && owner.draws==1);
        assert(xgeSceneClear()==XGE_OK && owner.leaves==1);
    }else if(mode==4){
        assert(xgeSceneLeaveCallbackSetOwned(scene,fail,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK && xgeScenePop()==-71 && references(scene)==2);
        bool output=false;assert(xgeSceneEnteredGet(scene,&output)==XGE_OK && output);
        assert(xgeSceneLeaveCallbackSetOwned(scene,NULL,NULL,NULL)==XGE_OK);
        assert(xgeScenePop()==XGE_OK && references(scene)==1);
    }else if(mode==5){
        assert(xgeSceneEnterCallbackSetOwned(scene,fail,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==-71 && references(scene)==1 && xgeSceneCount()==0);
    }else if(mode==6){
        assert(xgeSceneUpdateCallbackSetOwned(scene,update,&owner,&own)==XGE_OK);
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xgeSceneFreeCallbackSetOwned(scene,freed,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK);owner.reenter=3;
        assert(xgeFrame()==1 && owner.updates==1 && !owner.draws && owner.frees==1 && owner.refs==1);
        scene=NULL;
    }else if(mode==7 || mode==15){
        assert(xgeSceneUpdateCallbackSetOwned(scene,update,&owner,&own)==XGE_OK);
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK);owner.reenter=mode==7 ? 1:2;
        assert(xgeFrame()==1 && owner.updates==1 && !owner.draws && owner.refs==2);
        assert(xgeFrame()==1 && owner.draws==1 && owner.updates==1);
    }else if(mode==8){
        xthread* worker=xrtThreadCreate(foreign_thread,scene,0);assert(worker);
        assert(xrtThreadWaitFor(worker,10000)==XWAIT_OK && !xrtThreadExitCode(worker));xrtThreadDestroy(worker);assert(references(scene)==1);
    }else if(mode==9){
        assert(xrtMemDebugFailAfter(0));current=(xge_scene_owned)(uintptr_t)1;
        assert(xgeSceneCreateOwned(&current)==XGE_ERROR_OUT_OF_MEMORY && !current);
        assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xrtMemDebugFailAfter(0));
        assert(xgeSceneDrawCallbackSetOwned(scene,fail,&owner,&own)==XGE_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
        assert(owner.refs==2 && xgeSceneSetOwned(scene)==XGE_OK && xgeFrame()==1 && owner.draws==1);
        owner.failHold=1;assert(xgeSceneDrawCallbackSetOwned(scene,fail,&owner,&own)!=XGE_OK && owner.refs==2);
    }else if(mode==10){
        assert(xgeSceneFreeCallbackSetOwned(scene,freed,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK);assert(xgeSceneReleaseChecked(scene)==XGE_OK);scene=NULL;
        assert(xgeUnitChecked()==XGE_OK && owner.frees==1 && owner.refs==1);
    }else if(mode==11){
        assert(xgeScenePauseCallbackSetOwned(scene,paused,&owner,&own)==XGE_OK);
        assert(xgeSceneResumeCallbackSetOwned(scene,resumed,&owner,&own)==XGE_OK);
        assert(xgeSceneFreeCallbackSetOwned(other,free_fail,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK && xgeScenePushOwned(other)==XGE_OK);
        assert(xgeScenePop()==-73 && owner.frees==1 && owner.resumes==1 && references(other)==1);
    }else if(mode==12){
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);owner.failDrop=1;
        assert(xgeSceneDrawCallbackSetOwned(scene,NULL,NULL,NULL)!=XGE_OK && owner.refs==1);
        assert(xgeSceneSetOwned(scene)==XGE_OK && xgeFrame()==1 && !owner.draws);
    }else if(mode==13){
        xge_scene_t raw={0};assert(xgeSceneSet(&raw)==XGE_OK);current=scene;
        assert(xgeSceneCurrentOwned(&current)==XGE_ERROR_UNSUPPORTED && !current);
        assert(xgeSceneClear()==XGE_OK);
    }else if(mode==14){
        assert(xgeSceneUpdateCallbackSetOwned(scene,update,&owner,&own)==XGE_OK);
        assert(xgeSceneDrawCallbackSetOwned(scene,draw,&owner,&own)==XGE_OK);
        assert(xgeSceneSetOwned(scene)==XGE_OK);assert(xgeFrame()==1);
        xmemdebugsnapshot start,end;xrtMemDebugSnapshot(&start);
        for(int i=0;i<1000;++i)assert(xgeFrame()==1);
        xrtMemDebugSnapshot(&end);assert(start.AllocCount==end.AllocCount);
        assert(owner.updates==1001 && owner.draws==1001);
    }else if(mode==16){
        xge_scene_owned coded=NULL;
        assert(xgeSceneCreateOwnedWithCode(&coded,&own)==XGE_OK && owner.refs==2);
        xrtownershipref ref=xgeSceneOwnership(coded);visited=0;
        assert(ref.Ops->Trace(ref.Data,visit,&owner) && visited==1);
        xge_scene_owned shared=NULL;assert(xgeSceneShareOwned(coded,&shared)==XGE_OK);
        assert(xgeSceneReleaseChecked(coded)==XGE_OK && owner.refs==2);
        assert(xgeSceneReleaseChecked(shared)==XGE_OK && owner.refs==1);
    }else if(mode==17){
        xge_scene_owned coded=(xge_scene_owned)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(0));
        assert(xgeSceneCreateOwnedWithCode(&coded,&own)==XGE_ERROR_OUT_OF_MEMORY && !coded);
        assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();assert(owner.refs==1);
        owner.failHold=1;assert(xgeSceneCreateOwnedWithCode(&coded,&own)!=XGE_OK && !coded && owner.refs==1);
        assert(xgeSceneCreateOwnedWithCode(&coded,&own)==XGE_OK && owner.refs==2);
        owner.failDrop=1;bool consumed=false;
        assert(xgeSceneReleaseOwnerChecked(coded,&consumed)!=XGE_OK && consumed && owner.refs==1);
    }else if(mode==18){
        xge_scene_owned coded=NULL;assert(xgeSceneCreateOwnedWithCode(&coded,&own)==XGE_OK);
        assert(xgeSceneDrawCallbackSetOwned(coded,draw,&owner,&own)==XGE_OK);
        assert(owner.refs==3);xrtownershipref ref=xgeSceneOwnership(coded);visited=0;
        assert(ref.Ops->Trace(ref.Data,visit,&owner) && visited==2);
        assert(xgeSceneSetOwned(coded)==XGE_OK && xgeSceneReleaseChecked(coded)==XGE_OK);
        assert(xgeFrame()==1 && owner.draws==1 && owner.refs==3);
        assert(xgeSceneClear()==XGE_OK && owner.refs==1);
    }else assert(!"unknown mode");
    if(mode!=10)assert(xgeSceneClear()==XGE_OK);
    if(scene)assert(xgeSceneReleaseChecked(scene)==XGE_OK);
    if(other)assert(xgeSceneReleaseChecked(other)==XGE_OK);
    if(current)assert(xgeSceneReleaseChecked(current)==XGE_OK);
    assert(owner.refs==1 && owner.holds==owner.drops);
    assert(xgeUnitChecked()==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("owned Scene lifetime, stack and callback contract passed");return 0;
}
