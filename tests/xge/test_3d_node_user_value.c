#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#include <assert.h>
#include <stdio.h>

static const xstrview key={"count",5};

static xvalue *object_with(int64_t number)
{
    xvalue *object=xrtValueObject();
    xvalue *value=xrtValueInt(number);
    assert(object && value && xrtValueObjectSet(object,key,value));
    xrtValueRelease(value);
    return object;
}

static int64_t read_count(xge3d_scene *scene,xge3d_node_t node)
{
    xvalue *copy=NULL;
    int64_t number=0;
    assert(xge3dNodeCopyUserValue(scene,node,&copy)==XGE_OK && copy);
    assert(xrtValueGetInt(xrtValueObjectGet(copy,key),&number));
    xrtValueRelease(copy);
    return number;
}

static bool count_value(xrtownershipref value,void *arg)
{
    assert(value.Data && value.Ops);
    ++*(size_t*)arg;
    return true;
}

static void check_edges(xge3d_scene *scene,size_t expected)
{
    xrtownershipref owner=xge3dSceneOwnership(scene);
    xrtownershipscope freeze={0};
    size_t count=0;
    assert(owner.Data && xrtOwnershipFreezeTryBegin(&freeze));
    assert(owner.Ops->Trace(owner.Data,count_value,&count) && count==expected);
    assert(xrtOwnershipScopeEnd(&freeze));
}

typedef struct thread_probe {
    xge3d_scene *scene;
    xge3d_node_t node;
} thread_probe;

static int wrong_thread(void *arg)
{
    const thread_probe *probe=arg;
    xvalue *out=NULL;
    assert(xge3dNodeCopyUserValue(probe->scene,probe->node,&out)==XGE_ERROR_BUSY && !out);
    assert(xge3dNodeSetUserValue(probe->scene,probe->node,NULL)==XGE_ERROR_BUSY);
    assert(xge3dNodeSetUserValueSnapshot(probe->scene,probe->node,NULL)==XGE_ERROR_BUSY);
    return 0;
}

static void check_prepared_store(void)
{
    xge3d_scene *scene=NULL;
    xge3d_node_t node={0};
    xvalue *first=object_with(3),*second=object_with(5),*copy=NULL;
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dNodeCreate(scene,node,&node)==XGE_OK);

    assert(xrtMemDebugFailAfter(0));
    assert(xge3dNodeSetUserValueSnapshot(scene,node,first)==XGE_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    check_edges(scene,0);
    assert(xge3dNodeCopyUserValue(scene,node,&copy)==XGE_OK && !copy);

    assert(xge3dNodeSetUserValueSnapshot(scene,node,first)==XGE_OK);
    xrtValueRelease(first);
    assert(read_count(scene,node)==3);
    check_edges(scene,1);
    assert(xge3dNodeSetUserValueSnapshot(scene,node,second)==XGE_OK);
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xge3dNodeSetUserValueSnapshot(scene,node,second)==XGE_OK);
    }
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount==before.AllocCount && after.FreeCount==before.FreeCount);
    assert(after.ReallocCount==before.ReallocCount);
    xrtValueRelease(second);
    assert(read_count(scene,node)==5);
    assert(xge3dNodeSetUserValueSnapshot(scene,node,NULL)==XGE_OK);
    check_edges(scene,0);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    check_prepared_store();
    xge3d_scene *scene=NULL;
    xge3d_node_t node={0},child={0},clone={0};
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dNodeCreate(scene,node,&node)==XGE_OK);
    assert(xge3dNodeCreate(scene,node,&child)==XGE_OK);
    xvalue *copy=NULL,*source=object_with(7),*replacement=object_with(17);
    assert(xge3dNodeCopyUserValue(scene,node,&copy)==XGE_OK && !copy);
    check_edges(scene,0);

    int failures=0;
    for(int allowance=0;allowance<32;++allowance){
        assert(xrtMemDebugFailAfter(allowance));
        int result=xge3dNodeSetUserValue(scene,node,source);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        if(result==XGE_OK)break;
        assert(result==XGE_ERROR_OUT_OF_MEMORY);
        check_edges(scene,0);
        ++failures;
    }
    assert(failures>0 && failures<32 && read_count(scene,node)==7);
    assert(xge3dNodeSetUserValue(scene,child,replacement)==XGE_OK);
    assert(xrtValueObjectSet(source,key,xrtValueObjectGet(replacement,key)));
    assert(read_count(scene,node)==7 && read_count(scene,child)==17);
    check_edges(scene,2);

    /* The freeze owner may perform a controlled, balanced mutation on its
     * own thread. The edge table remains readable by the same visitor. */
    xrtownershipref owner=xge3dSceneOwnership(scene);
    xrtownershipscope freeze={0};
    size_t frozen_count=0;
    assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xge3dNodeSetUserValue(scene,node,NULL)==XGE_OK);
    assert(owner.Ops->Trace(owner.Data,count_value,&frozen_count) && frozen_count==1);
    assert(xrtOwnershipScopeEnd(&freeze));
    xrtValueRelease(source);
    source=object_with(7);
    assert(xge3dNodeSetUserValue(scene,node,source)==XGE_OK);
    check_edges(scene,2);

    assert(xrtMemDebugFailAfter(0));
    assert(xge3dNodeSetUserValue(scene,node,replacement)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(read_count(scene,node)==7);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dNodeCopyUserValue(scene,node,&copy)==XGE_ERROR_OUT_OF_MEMORY && !copy);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();

    xvalue *pointer=xrtValuePointer(scene);
    assert(pointer && xge3dNodeSetUserValue(scene,node,pointer)==XGE_ERROR_UNSUPPORTED);
    xrtClearError();
    xrtValueRelease(pointer);
    assert(xrtValueTypeIdBind(source,99));
    assert(xge3dNodeSetUserValue(scene,node,source)==XGE_ERROR_UNSUPPORTED);
    xrtClearError();
    assert(read_count(scene,node)==7);
    thread_probe probe={scene,node};
    xthread *worker=xrtThreadCreate(wrong_thread,&probe,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);

    int raw=42;
    assert(xge3dNodeSetUserData(scene,node,&raw)==XGE_OK);
    assert(xge3dNodeClone(scene,node,(xge3d_node_t){0},XGE3D_CLONE_USER_DATA,&clone)==XGE_OK);
    assert(xge3dNodeUserData(scene,clone)==&raw && read_count(scene,clone)==7);
    check_edges(scene,4);
    assert(xge3dNodeSetUserValue(scene,clone,NULL)==XGE_OK);
    assert(xge3dNodeUserData(scene,clone)==&raw && read_count(scene,node)==7);
    check_edges(scene,3);

    assert(xge3dNodeCopyUserValue(scene,node,&copy)==XGE_OK && copy);
    assert(xrtValueObjectSet(copy,key,xrtValueObjectGet(replacement,key)));
    assert(read_count(scene,node)==7);

    xge3d_work_budget_t budget={.nodes=0};
    int completed=1;
    assert(xge3dNodeDestroyStep(scene,node,&budget,&completed)==XGE_OK && !completed);
    xvalue *stale=NULL;
    assert(xge3dNodeCopyUserValue(scene,child,&stale)==XGE_ERROR_INVALID_ARGUMENT && !stale);
    budget.nodes=1;
    assert(xge3dNodeDestroyStep(scene,node,&budget,&completed)==XGE_OK && !completed);
    check_edges(scene,2);
    assert(xge3dNodeDestroyStep(scene,node,&budget,&completed)==XGE_OK && completed);
    check_edges(scene,1);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);

    int64_t number=0;
    assert(xrtValueGetInt(xrtValueObjectGet(copy,key),&number) && number==17);
    xrtValueRelease(copy);
    xrtValueRelease(source);
    xrtValueRelease(replacement);
    printf("NODE_USER_VALUE_NATIVE_OK failures=%d\n",failures);
    return 0;
}
