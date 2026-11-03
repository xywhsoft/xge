/* Share the native owner fixture; this gate runs the new sparse publishers. */
#define main xui_owned_event_regression_main
#include "xui_owned_callback_test.c"
#undef main

enum { LAYOUT_CLEAR=100,LAYOUT_CLOSE,LAYOUT_NEST };
static unsigned prepared,children,completed;
static int measure_owned(xui_widget widget,xui_vec2_t constraint,xui_vec2_t* output,void* user)
{
    (void)constraint;Owner* owner=user;assert(owner->refs>=2 && !owner->released);
    bool nested=owner->inCall;owner->inCall=true;++owner->calls;
    if(owner->action==LAYOUT_CLEAR)assert(xuiWidgetSetContentMeasureCallback(widget,NULL,NULL)==XUI_OK);
    else if(owner->action==LAYOUT_CLOSE)xuiDestroy(owner->context);
    else if(owner->action==LAYOUT_NEST && !nested){xui_vec2_t size={0};assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&size)==XUI_OK);}
    *output=(xui_vec2_t){12,13};assert(owner->refs && !owner->released);owner->inCall=nested;return XUI_OK;
}
typedef struct MeasureUser {unsigned marker;Owner* owner;} MeasureUser;
static int measure_with_distinct_user(xui_widget widget,xui_vec2_t constraint,xui_vec2_t* output,void* raw)
{
    MeasureUser* user=raw;assert(user->marker==0xCE11U);
    return measure_owned(widget,constraint,output,user->owner);
}
static int prepare_owned(xui_widget widget,void* user)
{(void)widget;Owner* owner=user;assert(owner->refs>=2 && !owner->released);++owner->calls;++prepared;return XUI_OK;}
static int rect_owned(xui_widget widget,xui_rect_t rect,void* user)
{(void)widget;(void)rect;Owner* owner=user;assert(owner->refs>=2 && !owner->released);++owner->calls;++completed;return XUI_OK;}
static int children_owned(xui_widget widget,xui_rect_t rect,void* user)
{
    Owner* owner=user;assert(owner->refs>=2 && !owner->released);++owner->calls;++children;
    for(xui_widget child=widget->pFirstChild;child;child=child->pNextSibling)
        if(xuiLayoutArrangeChild(widget,child,rect)!=XUI_OK)return XUI_ERROR_INVALID_STATE;
    return XUI_OK;
}
static void measure_lifetimes(void)
{
    for(int action=LAYOUT_CLEAR;action<=LAYOUT_NEST;++action){
        xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
        Owner owner={.refs=1,.context=context,.action=action};xui_callback_owner_t ref=descriptor(&owner);
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_owned,&owner,&ref)==XUI_OK);
        xrtMemDebugSnapshot(&after);assert(after.AllocCount-before.AllocCount==2 &&
            after.LiveBytes-before.LiveBytes==sizeof(xui_event_owners_t)+sizeof(xui_callback_binding_t));
        assert(drop(&owner));xui_vec2_t output={0};
        assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&output)==XUI_OK);
        assert(owner.calls==(action==LAYOUT_NEST ? 2u : 1u));
        if(action!=LAYOUT_CLOSE){xuiDestroy(context);}
        assert(owner.released==1 && !owner.refs);
    }
}
static void sparse_graph_and_calls(void)
{
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner owner={.refs=1,.context=context};xui_callback_owner_t ref=descriptor(&owner);
    MeasureUser user={0xCE11U,&owner};
    assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_with_distinct_user,&user,&ref)==XUI_OK);
    assert(xuiWidgetSetLayoutPrepareCallbackOwned(widget,prepare_owned,&owner,&ref)==XUI_OK);
    assert(xuiWidgetSetLayoutChildrenCallbackOwned(widget,children_owned,&owner,&ref)==XUI_OK);
    assert(xuiWidgetSetLayoutCompleteCallbackOwned(widget,rect_owned,&owner,&ref)==XUI_OK && owner.refs==5);
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,visit,&edges) && edges==4);
    assert(xrtOwnershipScopeEnd(&freeze));
    xui_widget child=NULL;assert(xuiWidgetCreate(context,&child)==XUI_OK && xuiWidgetAddChild(widget,child)==XUI_OK);
    assert(xuiSetViewportSize(context,200,100)==XUI_OK && xuiSetRootWidget(context,widget)==XUI_OK);
    assert(xuiLayout(context)==XUI_OK && prepared && children && completed);
    xui_vec2_t output={0};assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&output)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(unsigned i=0;i<200;++i){assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&output)==XUI_OK && output.fX==12 && output.fY==13);}
    xrtMemDebugSnapshot(&after);assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(drop(&owner));xuiDestroy(context);assert(owner.released==1);
}
static void sparse_registration_reentry(void)
{
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner nested={.refs=1,.context=context};Owner owner={.refs=1,.context=context,.widget=widget,.nested=&nested,.holdAction=1};
    xui_callback_owner_t ref=descriptor(&owner);
    assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_owned,&owner,&ref)==XUI_OK && owner.refs==2 && nested.refs==2);
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,visit,&edges) && edges==2);assert(xrtOwnershipScopeEnd(&freeze));
    xuiDestroy(context);assert(owner.refs==1 && nested.refs==1 && drop(&owner) && drop(&nested));
    for(int action=3;action<=4;++action){
        create(&context,&widget);owner=(Owner){.refs=1,.context=context,.widget=widget,.holdAction=action};ref=descriptor(&owner);
        assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_owned,&owner,&ref)==XUI_ERROR_INVALID_STATE);
        assert(owner.refs==1 && drop(&owner));if(action==3)xuiDestroy(context);
    }
}
static void sparse_failures(void)
{
    for(int64_t point=0;point<2;++point){
        xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
        Owner owner={.refs=1,.context=context};xui_callback_owner_t ref=descriptor(&owner);
        assert(xrtMemDebugFailAfter(point));
        assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_owned,&owner,&ref)==XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(owner.refs==1 && !widget->onContentMeasure && !widget->pEventOwners);
        xuiDestroy(context);assert(drop(&owner));
    }
    for(unsigned holdFailure=0;holdFailure<2;++holdFailure){
        xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
        Owner owner={.refs=1,.context=context,.action=holdFailure ? READ : LAYOUT_CLEAR};xui_callback_owner_t ref=descriptor(&owner);
        assert(xuiWidgetSetContentMeasureCallbackOwned(widget,measure_owned,&owner,&ref)==XUI_OK && drop(&owner));
        if(holdFailure)owner.failHold=true;else owner.failDrop=true;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(context,&scope)==XUI_OK);xui_vec2_t output={0};
        assert(xuiWidgetMeasureContent(widget,(xui_vec2_t){100,100},&output)==XUI_OK);
        assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
        xuiDestroy(context);assert(owner.released==1);
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    measure_lifetimes();sparse_graph_and_calls();sparse_registration_reentry();sparse_failures();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("owned layout callbacks: exact native publishers, graph, reentry, return-tail pins, OOM, checked cleanup and 200 allocation-free measurements passed");return 0;
}
