/* Headless manual-mode lifecycle regressions; no device, networking or input. */
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
#include <math.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
static xge_scene_t first,second;
static int frees,updates,oldDraws,newDraws,resumes,unitStatus;
static int leave_error(xge_scene scene){(void)scene;return -71;}
static int free_error(xge_scene scene){(void)scene;++frees;return -73;}
static int freed(xge_scene scene){(void)scene;++frees;return XGE_OK;}
static int resumed(xge_scene scene){(void)scene;++resumes;return XGE_OK;}
static int draw_old(xge_scene scene){(void)scene;++oldDraws;return XGE_OK;}
static int draw_new(xge_scene scene){(void)scene;++newDraws;return XGE_OK;}
static int enter_error(xge_scene scene){(void)scene;return -72;}
static int frameStatus;
static xerror* expectedError;
static int first_error(xge_scene scene)
{(void)scene;xrtSetErrorKind(XERR_RANGE);expectedError=xrtErrorRef(xrtGetError());return -73;}
static int second_error(xge_scene scene)
{(void)scene;++resumes;xrtSetErrorKind(XERR_STATE);return -74;}
static int release_heap(xge_scene scene){++frees;free(scene);return XGE_OK;}
static int nested_frame(xge_scene scene,float delta)
{(void)scene;(void)delta;frameStatus=xgeFrame();return XGE_OK;}
static int strategy_update(xge_scene scene,float delta)
{(void)scene;(void)delta;++updates;return xgeSceneUpdateStrategySet(XGE_UPDATE_VARIABLE,0.01f,1);}
static int switch_event(xge_scene scene,const xge_event_t* event)
{(void)scene;(void)event;return xgeSceneSet(&second);}
static int aba_update(xge_scene scene,float delta)
{(void)scene;(void)delta;++updates;assert(xgeScenePush(&second)==XGE_OK);return xgeScenePop();}
static int events;
static int observe_event(xge_scene scene,const xge_event_t* event)
{(void)event;assert(xgeSceneCurrent()==scene);++events;return XGE_OK;}
static int event_lifecycle(xge_scene scene)
{(void)scene;xge_event_t event={0};return xgeSceneDispatchEvent(&event);}
static int configure_enter(xge_scene scene)
{int status=xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED,0.25f,2);return status==XGE_OK ? event_lifecycle(scene):status;}
static int32 foreign_thread(ptr user)
{
    (void)user;assert(xgeSceneSet(&second)==XGE_ERROR_INVALID_STATE);
    assert(!xgeSceneCount() && xrtGetError());xrtClearError();
    assert(!xgeSceneCurrent() && xrtGetError());xrtClearError();
    int mode=123,maximum=456;float step=789;
    xgeSceneUpdateStrategyGet(&mode,&step,&maximum);
    assert(mode==123 && maximum==456 && step==789 && xrtGetError());xrtClearError();return 0;
}
static int switch_update(xge_scene scene,float delta)
{(void)scene;(void)delta;++updates;return xgeSceneSet(&second);}
static int unit_update(xge_scene scene,float delta)
{(void)scene;(void)delta;unitStatus=xgeUnitChecked();return XGE_OK;}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=1 && mode<=23);
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&desc)==XGE_OK);
    if(mode==1){
        first.onLeave=leave_error;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeScenePop()==-71);
        assert(xgeSceneCurrent()==&first && xgeSceneCount()==1 && first.bEntered);
    }else if(mode==2){
        first.onFree=freed;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeSceneSet(&first)==XGE_OK && !frees && xgeSceneCount()==1);
    }else if(mode==3){
        assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeScenePush(&first)!=XGE_OK && xgeSceneCount()==1 && !first.bPaused);
    }else if(mode==4){
        first.onFree=free_error;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeSceneReplace(&second)==-73);
        assert(!xgeSceneCurrent() && !xgeSceneCount() && !second.bEntered && frees==1);
    }else if(mode==5 || mode==9){
        first.onUpdate=switch_update;first.onDraw=draw_old;second.onDraw=draw_new;
        assert(xgeSceneSet(&first)==XGE_OK);
        if(mode==9){assert(xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED,0.000001f,3)==XGE_OK);xgeSleep(1);}
        assert(xgeFrame()==1);
        assert(updates==1 && !oldDraws && !newDraws && xgeSceneCurrent()==&second);
    }else if(mode==6){
        first.onLeave=leave_error;first.onFree=freed;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeSceneSet(&second)==-71);
        assert(xgeSceneCurrent()==&first && xgeSceneCount()==1 && !frees && !second.bEntered);
    }else if(mode==7){
        first.onLeave=leave_error;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeUnitChecked()==-71 && xgeSceneCurrent()==&first && xgeSceneCount()==1);
    }else if(mode==8){
        first.onUpdate=unit_update;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeFrame()==1 && unitStatus!=XGE_OK && xgeSceneCurrent()==&first);
    }else if(mode==10){
        first.onResume=resumed;second.onFree=free_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePush(&second)==XGE_OK);
        assert(xgeScenePop()==-73);
        assert(xgeSceneCurrent()==&first && !first.bPaused && resumes==1);
    }else if(mode==11){
        first.onResume=resumed;second.onEnter=enter_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePush(&second)==-72);
        assert(xgeSceneCurrent()==&first && xgeSceneCount()==1 && !first.bPaused && resumes==1);
    }else if(mode==12){
        first.onFree=freed;second.onEnter=enter_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeSceneSet(&second)==-72);
        assert(!xgeSceneCurrent() && !xgeSceneCount() && frees==1);
    }else if(mode==13){
        first.onUpdate=nested_frame;first.onDraw=draw_old;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeFrame()==1 && frameStatus==XGE_ERROR_BUSY && oldDraws==1);
    }else if(mode==14){
        first.onUpdate=strategy_update;first.onDraw=draw_old;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED,0.000001f,3)==XGE_OK);xgeSleep(1);
        assert(xgeFrame()==1 && updates==1 && !oldDraws);
    }else if(mode==15){
        first.onEvent=switch_event;assert(xgeSceneSet(&first)==XGE_OK);xge_event_t event={0};
        assert(xgeSceneDispatchEvent(&event)==XGE_OK && xgeSceneCurrent()==&second);
    }else if(mode==16){
        first.onResume=second_error;second.onFree=first_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePush(&second)==XGE_OK);
        assert(xgeScenePop()==-73 && resumes==1 && xrtGetError()==expectedError);
        xrtErrorFree(expectedError);expectedError=NULL;xrtClearError();
    }else if(mode==17){
        xge_scene heap=calloc(1,sizeof(*heap));assert(heap);
        heap->onUpdate=switch_update;heap->onDraw=draw_old;heap->onFree=release_heap;
        assert(xgeSceneSet(heap)==XGE_OK && xgeFrame()==1);
        assert(updates==1 && frees==1 && !oldDraws && xgeSceneCurrent()==&second);
    }else if(mode==18){
        assert(xgeSceneSet(&first)==XGE_OK);xthread* worker=xrtThreadCreate(foreign_thread,NULL,0);assert(worker);
        assert(xrtThreadWaitFor(worker,10000)==XWAIT_OK && !xrtThreadExitCode(worker));xrtThreadDestroy(worker);
        assert(xgeSceneCurrent()==&first && xgeSceneCount()==1);
    }else if(mode==19){
        first.onUpdate=aba_update;first.onDraw=draw_old;assert(xgeSceneSet(&first)==XGE_OK);
        assert(xgeFrame()==1 && updates==1 && !oldDraws && xgeSceneCurrent()==&first);
    }else if(mode==20){
        first.onLeave=leave_error;second.onFree=free_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePush(&second)==XGE_OK);
        assert(xgeSceneClear()==-73 && frees==1 && xgeSceneCurrent()==&first && xgeSceneCount()==1);
    }else if(mode==21){
        first.onResume=second_error;second.onFree=free_error;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePush(&second)==XGE_OK);
        assert(xgeScenePop()==-73 && resumes==1 && !xrtGetError());
    }else if(mode==22){
        first.onEnter=configure_enter;first.onEvent=observe_event;
        assert(xgeSceneSet(&first)==XGE_OK && events==1);
        int updateMode=0,maximum=0;float step=0;xgeSceneUpdateStrategyGet(&updateMode,&step,&maximum);
        assert(updateMode==XGE_UPDATE_FIXED && step==0.25f && maximum==2);
    }else{
        first.onLeave=event_lifecycle;first.onEvent=observe_event;
        assert(xgeSceneSet(&first)==XGE_OK && xgeScenePop()==XGE_OK && events==1);
    }
    assert(xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED,NAN,3)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED,INFINITY,3)==XGE_ERROR_INVALID_ARGUMENT);
    first.onLeave=NULL;first.onFree=NULL;second.onFree=NULL;
    assert(xgeUnitChecked()==XGE_OK);
    printf("Scene transition mode %d passed\n",mode);return 0;
}
