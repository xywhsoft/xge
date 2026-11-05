#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xge.h"
#include "xui.h"
#include "xge_data_lifetime_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct xge_owned_test {
    xge_data_object target,source;
    int holdFail,fresh,dropFail,dropError,replace,inRead,holds,drops,calls,peer,singleSet,outcome,error,releaseOwner,gap;
} xge_owned_test;
static xge_owned_test* xge_active;
static xge_data_object xge_body(const void* data)
{const xge_data_lifetime_t* root=data;return (xge_data_object)((const char*)root-root->pType->iBodyBytes);}
static bool xge_hold(const void* data)
{
    xge_owned_test* test=xge_active;if(test)++test->holds;
    if(test && test->holdFail){if(test->fresh)xrtSetErrorKind(XERR_RANGE);return false;}
    int status=xgeDataObjectAddRef(xge_body(data));if(status!=XGE_OK)return false;
    if(test && test->peer)assert(xgeDataObjectSet(test->target,"provider","peer",4u)==(test->dropFail ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
    if(test && test->fresh)xrtSetErrorKind(XERR_RANGE);
    return true;
}
static bool xge_drop(const void* data)
{
    xge_owned_test* test=xge_active;if(test){++test->drops;assert(!test->inRead);}
    int status=xgeDataObjectReleaseChecked(xge_body(data));
    if(test && test->dropError)xrtSetErrorKind(XERR_STATE);
    return status==XGE_OK && !(test && test->dropFail);
}
static xge_callback_owner_t xge_owner(xge_data_object object)
{return (xge_callback_owner_t){xgeDataObjectOwnership(object),xge_hold,xge_drop};}
static size_t xge_count(xrtownershipref ref)
{
    xrtownershipscope scope={0};assert(xrtOwnershipFreezeTryBegin(&scope));size_t count=0;
    assert(ref.Data && ref.Ops && ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&scope));return count;
}
static int xge_read(const xstrview* format,xge_data_output_t* output,void* raw)
{
    xge_owned_test* test=raw;++test->calls;test->inRead=1;
    assert(format && format->Size==8 && !memcmp(format->Data,"provider",8));
    if(test->replace)assert(xgeDataObjectSet(test->target,"provider","new",3)==XGE_OK);
    assert(xgeDataOutputWrite(output,SIZE_MAX,"x",1)==XGE_ERROR_INVALID_ARGUMENT);
    int result=XGE_OK;
    if(test->gap)result=xgeDataOutputWrite(output,3,"xy",2);
    else if(test->singleSet){const unsigned char all[]={'a',0,'b'};result=xgeDataOutputSet(output,all,3);}
    else{const unsigned char first[]={'a',0};result=xgeDataOutputWrite(output,0,first,2);if(result==XGE_OK)result=xgeDataOutputWrite(output,2,"b",1);}
    if(test->error)xrtSetErrorKind(XERR_RANGE);
    if(test->outcome)result=test->outcome;
    if(test->releaseOwner)assert(xgeDataObjectReleaseChecked(test->target)==XGE_OK);
    test->inRead=0;return result;
}
static void xge_basic(void)
{
    xge_owned_test test={0};xge_active=&test;assert(xgeDataObjectCreateOwned(&test.target)==XGE_OK);
    assert(xgeDataObjectCreateOwned(&test.source)==XGE_OK);xrtownershipref source=xgeDataObjectOwnership(test.source);
    xge_data_object share=NULL;assert(xgeDataObjectShareOwned(test.target,&share)==XGE_OK && share==test.target);
    assert(xge_count(xgeDataObjectOwnership(test.target))==2);assert(xgeDataObjectReleaseChecked(share)==XGE_OK);
    xge_callback_owner_t owner=xge_owner(test.source);
    assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_OK && xge_count(source)==2);
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();
    assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_OK && xrtGetError()==ambient);xrtClearError();
    size_t size=0;assert(xgeDataObjectGet(test.target,"provider",NULL,0,&size)==XGE_OK && size==3 && xge_count(source)==2);
    unsigned char bytes[3]={0};assert(xgeDataObjectGet(test.target,"provider",bytes,3,&size)==XGE_OK && size==3 && bytes[0]=='a' && !bytes[1] && bytes[2]=='b');
    unsigned char sparse[8];memset(sparse,99,sizeof(sparse));test.gap=1;
    assert(xgeDataObjectGet(test.target,"provider",sparse,sizeof(sparse),&size)==XGE_OK && size==5 && !sparse[0] && !sparse[1] && !sparse[2] && sparse[3]=='x' && sparse[4]=='y' && sparse[5]==99);test.gap=0;
    unsigned char small[2]={99,99};
    assert(xgeDataObjectGet(test.target,"provider",small,2,&size)==XGE_ERROR_BUFFER_TOO_SMALL && size==3 && small[0]=='a' && small[1]==0 && !xrtGetError());
    test.singleSet=1;small[0]=small[1]=99;
    assert(xgeDataObjectGet(test.target,"provider",small,2,&size)==XGE_ERROR_BUFFER_TOO_SMALL && size==3 && small[0]==99 && small[1]==99);test.singleSet=0;
    test.outcome=1;test.dropFail=test.dropError=1;
    assert(xgeDataObjectGet(test.target,"provider",small,2,&size)==XGE_ERROR_CALLBACK_FAILED && !xrtGetError());
    test.outcome=test.dropFail=test.dropError=0;test.error=1;
    assert(xgeDataObjectGet(test.target,"provider",small,2,&size)==XGE_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);test.error=0;xrtClearError();
    test.replace=1;int before=test.drops;assert(xgeDataObjectGet(test.target,"provider",bytes,3,&size)==XGE_OK && test.drops==before+2 && xge_count(source)==1);
    test.replace=0;assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_OK);
    test.holdFail=1;before=test.drops;assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_ERROR_CALLBACK_FAILED && !xrtGetError() && test.drops==before && xge_count(source)==2);test.holdFail=0;
    test.holdFail=test.fresh=1;before=test.drops;
    assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && test.drops==before && xge_count(source)==2);
    xrtClearError();int calls=test.calls;
    assert(xgeDataObjectGet(test.target,"provider",bytes,3,&size)==XGE_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && test.calls==calls && test.drops==before && xge_count(source)==2);
    xrtClearError();test.holdFail=0;
    test.fresh=1;test.peer=1;test.dropFail=1;test.dropError=1;
    assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);
    xrtClearError();test.fresh=test.peer=test.dropFail=test.dropError=0;
    char text[5]={0};assert(xgeDataObjectGet(test.target,"provider",text,4,&size)==XGE_OK && size==4 && !memcmp(text,"peer",4));
    assert(xge_count(source)==1);
    assert(xgeDataObjectSet(test.target,"stable","old",3)==XGE_OK);
    for(int budget=0;budget<2;++budget){
        assert(xrtMemDebugFailAfter(budget));int status=xgeDataObjectSet(test.target,"stable","replacement",11);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert(status==(budget==0 ? XGE_ERROR_OUT_OF_MEMORY:XGE_OK));
        char check[12]={0};assert(xgeDataObjectGet(test.target,"stable",check,sizeof(check),&size)==XGE_OK);
        assert(size==(budget==0 ? 3u:11u) && !memcmp(check,budget==0 ? "old":"replacement",size));
    }
    for(int budget=0;budget<2;++budget){
        int holds=test.holds,drops=test.drops;assert(xrtMemDebugFailAfter(budget));int status=xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(status==(budget==0 ? XGE_ERROR_OUT_OF_MEMORY:XGE_OK));
        if(!budget)assert(test.holds==holds && test.drops==drops && xge_count(source)==1);
    }
    assert(xgeDataObjectReleaseChecked(test.target)==XGE_OK);assert(xgeDataObjectReleaseChecked(test.source)==XGE_OK);
    xge_active=NULL;
}

static void xge_tail(void)
{
    for(int mode=0;mode<2;++mode){
        xge_owned_test test={0};xge_active=&test;
        assert(xgeDataObjectCreateOwned(&test.target)==XGE_OK && xgeDataObjectCreateOwned(&test.source)==XGE_OK);
        xrtownershipref source=xgeDataObjectOwnership(test.source);xge_callback_owner_t owner=xge_owner(test.source);
        assert(xgeDataObjectSetProviderOwned(test.target,"provider",xge_read,&test,&owner)==XGE_OK);
        if(!mode){
            test.releaseOwner=1;unsigned char bytes[3];size_t size=0;
            assert(xgeDataObjectGet(test.target,"provider",bytes,3,&size)==XGE_OK && size==3 && xge_count(source)==1);
        }else{
            test.dropFail=1;bool consumed=false;assert(xrtMemDebugFailAfter(0));
            assert(xgeDataObjectReleaseOwnerChecked(test.target,&consumed)==XGE_ERROR_CALLBACK_FAILED && consumed && !xrtGetError());
            assert(xrtMemDebugFailAfter(-1));assert(xge_count(source)==1);
        }
        assert(xgeDataObjectReleaseChecked(test.source)==XGE_OK);xge_active=NULL;
    }
}
typedef struct xge_graph {xrtownershipref refs[8];size_t count;} xge_graph;
static bool xge_walk(xrtownershipref ref,void* raw)
{
    xge_graph* graph=raw;for(size_t i=0;i<graph->count;++i)if(graph->refs[i].Data==ref.Data && graph->refs[i].Ops==ref.Ops)return true;
    assert(graph->count<8 && xgeDataObjectOwnershipAdapterV1(ref));graph->refs[graph->count++]=ref;
    size_t count=0;assert(ref.Ops->Count(ref.Data,&count) && count==1);return ref.Ops->Trace(ref.Data,xge_walk,graph);
}
static void xge_cycle(void)
{
    xge_owned_test a={0},b={0};assert(xgeDataObjectCreateOwned(&a.target)==XGE_OK);assert(xgeDataObjectCreateOwned(&b.target)==XGE_OK);
    xge_callback_owner_t ab=xge_owner(b.target),ba=xge_owner(a.target);
    xge_active=NULL;assert(xgeDataObjectSetProviderOwned(a.target,"provider",xge_read,&a,&ab)==XGE_OK);
    assert(xgeDataObjectSetProviderOwned(b.target,"provider",xge_read,&b,&ba)==XGE_OK);
    xrtownershipref root=xgeDataObjectOwnership(a.target);assert(xgeDataObjectReleaseChecked(a.target)==XGE_OK && xgeDataObjectReleaseChecked(b.target)==XGE_OK);
    xge_graph graph={0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xge_walk(root,&graph) && graph.count==4);
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipadapterv1* adapter=xgeDataObjectOwnershipAdapterV1(ref);assert(adapter->Hold(ref.Data) && adapter->Claim(ref.Data,&graph));}
    assert(xrtOwnershipScopeEnd(&freeze));
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipfinalizationv1* final=xgeDataObjectOwnershipFinalizationV1(ref);bool complete=false;assert(final->CanStep(ref.Data,&graph) && final->Step(ref.Data,&graph,&complete) && complete);}
    assert(xrtOwnershipFreezeTryBegin(&freeze));for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];xgeDataObjectOwnershipAdapterV1(ref)->Clear(ref.Data,&graph);}assert(xrtOwnershipScopeEnd(&freeze));
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipfinishv2* finish=xgeDataObjectOwnershipCompletionV2(ref);assert(finish->V1.Step(ref.Data,&graph)==XRT_OWNERSHIP_FINISH_READY);}
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];assert(xgeDataObjectOwnershipCompletionV2(ref)->DropChecked(ref.Data));}
}
typedef struct xge_worker { xge_data_object data;int status;bool consumed;xrtownershipref ref;bool drop;} xge_worker;
static int32 xge_worker_proc(void* raw)
{
    xge_worker* worker=raw;
    if(worker->drop){const xrtownershipfinishv2* finish=xgeDataObjectOwnershipCompletionV2(worker->ref);worker->consumed=finish->DropReady(worker->ref.Data) && finish->DropChecked(worker->ref.Data);worker->status=worker->consumed ? XGE_OK:XGE_ERROR_BUSY;}
    else worker->status=xgeDataObjectReleaseOwnerChecked(worker->data,&worker->consumed);
    assert(!xrtGetError());return 0;
}
static void xge_affinity(void)
{
    xge_worker worker={0};assert(xgeDataObjectCreateOwned(&worker.data)==XGE_OK);xthread* thread=xrtThreadCreate(xge_worker_proc,&worker,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(worker.status==XGE_ERROR_BUSY && !worker.consumed);worker.ref=xgeDataObjectOwnership(worker.data);
    const xrtownershipadapterv1* adapter=xgeDataObjectOwnershipAdapterV1(worker.ref);xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(adapter->Hold(worker.ref.Data) && adapter->Claim(worker.ref.Data,&worker));assert(xrtOwnershipScopeEnd(&freeze));
    xge_data_object share=(xge_data_object)(uintptr_t)1;assert(xgeDataObjectShareOwned(worker.data,&share)==XGE_ERROR_INVALID_STATE && !share);assert(xgeDataObjectAddRef(worker.data)==XGE_ERROR_INVALID_STATE);
    assert(xgeDataObjectReleaseOwnerChecked(worker.data,&worker.consumed)==XGE_OK && worker.consumed && xge_count(worker.ref)==1);
    bool complete=false;assert(xgeDataObjectOwnershipFinalizationV1(worker.ref)->Step(worker.ref.Data,&worker,&complete) && complete);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(worker.ref.Data,&worker);assert(xrtOwnershipScopeEnd(&freeze));assert(xgeDataObjectOwnershipCompletionV2(worker.ref)->V1.Step(worker.ref.Data,&worker)==XRT_OWNERSHIP_FINISH_READY);
    worker.drop=true;thread=xrtThreadCreate(xge_worker_proc,&worker,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);assert(worker.status==XGE_OK && worker.consumed);
}


typedef struct xui_owned_test {
    xui_data_object target,source;
    int holdFail,fresh,dropFail,dropError,replace,inRead,holds,drops,calls,peer,singleSet,outcome,error,releaseOwner,gap;
} xui_owned_test;
static xui_owned_test* xui_active;
static xui_data_object xui_body(const void* data)
{const xge_data_lifetime_t* root=data;return (xui_data_object)((const char*)root-root->pType->iBodyBytes);}
static bool xui_hold(const void* data)
{
    xui_owned_test* test=xui_active;if(test)++test->holds;
    if(test && test->holdFail){if(test->fresh)xrtSetErrorKind(XERR_RANGE);return false;}
    int status=xuiDataObjectAddRef(xui_body(data));if(status!=XUI_OK)return false;
    if(test && test->peer)assert(xuiDataObjectSet(test->target,"provider","peer",4u)==(test->dropFail ? XUI_ERROR_CALLBACK_FAILED:XUI_OK));
    if(test && test->fresh)xrtSetErrorKind(XERR_RANGE);
    return true;
}
static bool xui_drop(const void* data)
{
    xui_owned_test* test=xui_active;if(test){++test->drops;assert(!test->inRead);}
    int status=xuiDataObjectReleaseChecked(xui_body(data));
    if(test && test->dropError)xrtSetErrorKind(XERR_STATE);
    return status==XUI_OK && !(test && test->dropFail);
}
static xui_callback_owner_t xui_owner(xui_data_object object)
{return (xui_callback_owner_t){xuiDataObjectOwnership(object),xui_hold,xui_drop};}
static size_t xui_count(xrtownershipref ref)
{
    xrtownershipscope scope={0};assert(xrtOwnershipFreezeTryBegin(&scope));size_t count=0;
    assert(ref.Data && ref.Ops && ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&scope));return count;
}
static int xui_read(const xstrview* format,xui_data_output_t* output,void* raw)
{
    xui_owned_test* test=raw;++test->calls;test->inRead=1;
    assert(format && format->Size==8 && !memcmp(format->Data,"provider",8));
    if(test->replace)assert(xuiDataObjectSet(test->target,"provider","new",3)==XUI_OK);
    assert(xuiDataOutputWrite(output,SIZE_MAX,"x",1)==XUI_ERROR_INVALID_ARGUMENT);
    int result=XUI_OK;
    if(test->gap)result=xuiDataOutputWrite(output,3,"xy",2);
    else if(test->singleSet){const unsigned char all[]={'a',0,'b'};result=xuiDataOutputSet(output,all,3);}
    else{const unsigned char first[]={'a',0};result=xuiDataOutputWrite(output,0,first,2);if(result==XUI_OK)result=xuiDataOutputWrite(output,2,"b",1);}
    if(test->error)xrtSetErrorKind(XERR_RANGE);
    if(test->outcome)result=test->outcome;
    if(test->releaseOwner)assert(xuiDataObjectReleaseChecked(test->target)==XUI_OK);
    test->inRead=0;return result;
}
static void xui_basic(void)
{
    xui_owned_test test={0};xui_active=&test;assert(xuiDataObjectCreateOwned(&test.target)==XUI_OK);
    assert(xuiDataObjectCreateOwned(&test.source)==XUI_OK);xrtownershipref source=xuiDataObjectOwnership(test.source);
    xui_data_object share=NULL;assert(xuiDataObjectShareOwned(test.target,&share)==XUI_OK && share==test.target);
    assert(xui_count(xuiDataObjectOwnership(test.target))==2);assert(xuiDataObjectReleaseChecked(share)==XUI_OK);
    xui_callback_owner_t owner=xui_owner(test.source);
    assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_OK && xui_count(source)==2);
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();
    assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_OK && xrtGetError()==ambient);xrtClearError();
    size_t size=0;assert(xuiDataObjectGet(test.target,"provider",NULL,0,&size)==XUI_OK && size==3 && xui_count(source)==2);
    unsigned char bytes[3]={0};assert(xuiDataObjectGet(test.target,"provider",bytes,3,&size)==XUI_OK && size==3 && bytes[0]=='a' && !bytes[1] && bytes[2]=='b');
    unsigned char sparse[8];memset(sparse,99,sizeof(sparse));test.gap=1;
    assert(xuiDataObjectGet(test.target,"provider",sparse,sizeof(sparse),&size)==XUI_OK && size==5 && !sparse[0] && !sparse[1] && !sparse[2] && sparse[3]=='x' && sparse[4]=='y' && sparse[5]==99);test.gap=0;
    unsigned char small[2]={99,99};
    assert(xuiDataObjectGet(test.target,"provider",small,2,&size)==XUI_ERROR_BUFFER_TOO_SMALL && size==3 && small[0]=='a' && small[1]==0 && !xrtGetError());
    test.singleSet=1;small[0]=small[1]=99;
    assert(xuiDataObjectGet(test.target,"provider",small,2,&size)==XUI_ERROR_BUFFER_TOO_SMALL && size==3 && small[0]==99 && small[1]==99);test.singleSet=0;
    test.outcome=1;test.dropFail=test.dropError=1;
    assert(xuiDataObjectGet(test.target,"provider",small,2,&size)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    test.outcome=test.dropFail=test.dropError=0;test.error=1;
    assert(xuiDataObjectGet(test.target,"provider",small,2,&size)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);test.error=0;xrtClearError();
    test.replace=1;int before=test.drops;assert(xuiDataObjectGet(test.target,"provider",bytes,3,&size)==XUI_OK && test.drops==before+2 && xui_count(source)==1);
    test.replace=0;assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_OK);
    test.holdFail=1;before=test.drops;assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && test.drops==before && xui_count(source)==2);test.holdFail=0;
    test.holdFail=test.fresh=1;before=test.drops;
    assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && test.drops==before && xui_count(source)==2);
    xrtClearError();int calls=test.calls;
    assert(xuiDataObjectGet(test.target,"provider",bytes,3,&size)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && test.calls==calls && test.drops==before && xui_count(source)==2);
    xrtClearError();test.holdFail=0;
    test.fresh=1;test.peer=1;test.dropFail=1;test.dropError=1;
    assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);
    xrtClearError();test.fresh=test.peer=test.dropFail=test.dropError=0;
    char text[5]={0};assert(xuiDataObjectGet(test.target,"provider",text,4,&size)==XUI_OK && size==4 && !memcmp(text,"peer",4));
    assert(xui_count(source)==1);
    assert(xuiDataObjectSet(test.target,"stable","old",3)==XUI_OK);
    for(int budget=0;budget<2;++budget){
        assert(xrtMemDebugFailAfter(budget));int status=xuiDataObjectSet(test.target,"stable","replacement",11);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert(status==(budget==0 ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        char check[12]={0};assert(xuiDataObjectGet(test.target,"stable",check,sizeof(check),&size)==XUI_OK);
        assert(size==(budget==0 ? 3u:11u) && !memcmp(check,budget==0 ? "old":"replacement",size));
    }
    for(int budget=0;budget<2;++budget){
        int holds=test.holds,drops=test.drops;assert(xrtMemDebugFailAfter(budget));int status=xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(status==(budget==0 ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        if(!budget)assert(test.holds==holds && test.drops==drops && xui_count(source)==1);
    }
    assert(xuiDataObjectReleaseChecked(test.target)==XUI_OK);assert(xuiDataObjectReleaseChecked(test.source)==XUI_OK);
    xui_active=NULL;
}

static void xui_tail(void)
{
    for(int mode=0;mode<2;++mode){
        xui_owned_test test={0};xui_active=&test;
        assert(xuiDataObjectCreateOwned(&test.target)==XUI_OK && xuiDataObjectCreateOwned(&test.source)==XUI_OK);
        xrtownershipref source=xuiDataObjectOwnership(test.source);xui_callback_owner_t owner=xui_owner(test.source);
        assert(xuiDataObjectSetProviderOwned(test.target,"provider",xui_read,&test,&owner)==XUI_OK);
        if(!mode){
            test.releaseOwner=1;unsigned char bytes[3];size_t size=0;
            assert(xuiDataObjectGet(test.target,"provider",bytes,3,&size)==XUI_OK && size==3 && xui_count(source)==1);
        }else{
            test.dropFail=1;bool consumed=false;assert(xrtMemDebugFailAfter(0));
            assert(xuiDataObjectReleaseOwnerChecked(test.target,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed && !xrtGetError());
            assert(xrtMemDebugFailAfter(-1));assert(xui_count(source)==1);
        }
        assert(xuiDataObjectReleaseChecked(test.source)==XUI_OK);xui_active=NULL;
    }
}
typedef struct xui_graph {xrtownershipref refs[8];size_t count;} xui_graph;
static bool xui_walk(xrtownershipref ref,void* raw)
{
    xui_graph* graph=raw;for(size_t i=0;i<graph->count;++i)if(graph->refs[i].Data==ref.Data && graph->refs[i].Ops==ref.Ops)return true;
    assert(graph->count<8 && xuiDataObjectOwnershipAdapterV1(ref));graph->refs[graph->count++]=ref;
    size_t count=0;assert(ref.Ops->Count(ref.Data,&count) && count==1);return ref.Ops->Trace(ref.Data,xui_walk,graph);
}
static void xui_cycle(void)
{
    xui_owned_test a={0},b={0};assert(xuiDataObjectCreateOwned(&a.target)==XUI_OK);assert(xuiDataObjectCreateOwned(&b.target)==XUI_OK);
    xui_callback_owner_t ab=xui_owner(b.target),ba=xui_owner(a.target);
    xui_active=NULL;assert(xuiDataObjectSetProviderOwned(a.target,"provider",xui_read,&a,&ab)==XUI_OK);
    assert(xuiDataObjectSetProviderOwned(b.target,"provider",xui_read,&b,&ba)==XUI_OK);
    xrtownershipref root=xuiDataObjectOwnership(a.target);assert(xuiDataObjectReleaseChecked(a.target)==XUI_OK && xuiDataObjectReleaseChecked(b.target)==XUI_OK);
    xui_graph graph={0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xui_walk(root,&graph) && graph.count==4);
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipadapterv1* adapter=xuiDataObjectOwnershipAdapterV1(ref);assert(adapter->Hold(ref.Data) && adapter->Claim(ref.Data,&graph));}
    assert(xrtOwnershipScopeEnd(&freeze));
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipfinalizationv1* final=xuiDataObjectOwnershipFinalizationV1(ref);bool complete=false;assert(final->CanStep(ref.Data,&graph) && final->Step(ref.Data,&graph,&complete) && complete);}
    assert(xrtOwnershipFreezeTryBegin(&freeze));for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];xuiDataObjectOwnershipAdapterV1(ref)->Clear(ref.Data,&graph);}assert(xrtOwnershipScopeEnd(&freeze));
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];const xrtownershipfinishv2* finish=xuiDataObjectOwnershipCompletionV2(ref);assert(finish->V1.Step(ref.Data,&graph)==XRT_OWNERSHIP_FINISH_READY);}
    for(size_t i=0;i<graph.count;++i){xrtownershipref ref=graph.refs[i];assert(xuiDataObjectOwnershipCompletionV2(ref)->DropChecked(ref.Data));}
}
typedef struct xui_worker { xui_data_object data;int status;bool consumed;xrtownershipref ref;bool drop;} xui_worker;
static int32 xui_worker_proc(void* raw)
{
    xui_worker* worker=raw;
    if(worker->drop){const xrtownershipfinishv2* finish=xuiDataObjectOwnershipCompletionV2(worker->ref);worker->consumed=finish->DropReady(worker->ref.Data) && finish->DropChecked(worker->ref.Data);worker->status=worker->consumed ? XUI_OK:XUI_ERROR_BUSY;}
    else worker->status=xuiDataObjectReleaseOwnerChecked(worker->data,&worker->consumed);
    assert(!xrtGetError());return 0;
}
static void xui_affinity(void)
{
    xui_worker worker={0};assert(xuiDataObjectCreateOwned(&worker.data)==XUI_OK);xthread* thread=xrtThreadCreate(xui_worker_proc,&worker,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(worker.status==XUI_ERROR_BUSY && !worker.consumed);worker.ref=xuiDataObjectOwnership(worker.data);
    const xrtownershipadapterv1* adapter=xuiDataObjectOwnershipAdapterV1(worker.ref);xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(adapter->Hold(worker.ref.Data) && adapter->Claim(worker.ref.Data,&worker));assert(xrtOwnershipScopeEnd(&freeze));
    xui_data_object share=(xui_data_object)(uintptr_t)1;assert(xuiDataObjectShareOwned(worker.data,&share)==XUI_ERROR_INVALID_STATE && !share);assert(xuiDataObjectAddRef(worker.data)==XUI_ERROR_INVALID_STATE);
    assert(xuiDataObjectReleaseOwnerChecked(worker.data,&worker.consumed)==XUI_OK && worker.consumed && xui_count(worker.ref)==1);
    bool complete=false;assert(xuiDataObjectOwnershipFinalizationV1(worker.ref)->Step(worker.ref.Data,&worker,&complete) && complete);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(worker.ref.Data,&worker);assert(xrtOwnershipScopeEnd(&freeze));assert(xuiDataObjectOwnershipCompletionV2(worker.ref)->V1.Step(worker.ref.Data,&worker)==XRT_OWNERSHIP_FINISH_READY);
    worker.drop=true;thread=xrtThreadCreate(xui_worker_proc,&worker,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);assert(worker.status==XUI_OK && worker.consumed);
}

int main(void){xge_basic();xui_basic();xge_cycle();xui_cycle();xge_affinity();xui_affinity();xge_tail();xui_tail();puts("Owned DataObject actual graph/cycle/output/holder/claim/affinity passed");return 0;}
