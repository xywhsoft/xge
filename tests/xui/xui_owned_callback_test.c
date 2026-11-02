#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

xui_widget xuiInternalPopupPointerPassthroughOwner(xui_context c,xui_widget w,float x,float y)
{(void)c;(void)x;(void)y;return w;}
int xuiInternalPopupPrepareOwnerPress(xui_context c,xui_widget w,float x,float y)
{(void)c;(void)w;(void)x;(void)y;return XUI_OK;}

enum { READ, REMOVE_PRIMARY, REMOVE_HANDLER, NEST, CLOSE, CLEANUP_CLOSE };
typedef struct Owner {
    unsigned refs,calls,drops,released;
    bool inCall,failHold,failDrop;
    int action;
    xui_context context;
} Owner;
static bool hold(const void* raw)
{Owner* owner=(Owner*)raw;if(owner->failHold)return false;assert(owner->refs);++owner->refs;return true;}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs);++owner->drops;
    if(--owner->refs)return true;
    assert(!owner->inCall);++owner->released;
    if(owner->action==CLEANUP_CLOSE)xuiDestroy(owner->context);
    return !owner->failDrop;
}
static bool count(const void* raw,size_t* output)
{const Owner* owner=raw;*output=owner->refs;return owner->refs!=0;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_callback_owner_t descriptor(Owner* owner)
{return (xui_callback_owner_t){{owner,&ops},hold,drop};}
static int dispatch(xui_context context,xui_widget widget)
{xui_event_t event={0};event.iSize=sizeof(event);event.iType=XUI_EVENT_COMMAND;event.pTarget=widget;return xuiDispatchEvent(context,&event);}
static int callback(xui_widget widget,const xui_event_t* event,void* raw)
{
    Owner* owner=raw;assert(owner->refs>=2 && !owner->released);++owner->calls;
    bool previous=owner->inCall;owner->inCall=true;
    if(owner->action==REMOVE_PRIMARY)assert(xuiWidgetSetEventCallback(widget,NULL,NULL)==XUI_OK);
    else if(owner->action==REMOVE_HANDLER)assert(xuiWidgetSetEventHandler(widget,event->iType,NULL,NULL)==XUI_OK);
    else if(owner->action==NEST && !previous)assert(dispatch(owner->context,widget)==XUI_OK);
    else if(owner->action==CLOSE)xuiDestroy(owner->context);
    assert(owner->refs && !owner->released);owner->inCall=previous;return XUI_OK;
}
static void create(xui_context* context,xui_widget* widget)
{assert(xuiCreate(context)==XUI_OK && xuiWidgetCreate(*context,widget)==XUI_OK);}
static void replacement_and_nested(void)
{
    for(unsigned handler=0;handler<2;++handler){
        xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
        Owner owner={.refs=1,.action=handler?REMOVE_HANDLER:REMOVE_PRIMARY,.context=context};
        xui_callback_owner_t ref=descriptor(&owner);
        xmemdebugsnapshot coldBefore,coldAfter;xrtMemDebugSnapshot(&coldBefore);
        assert((handler?xuiWidgetSetEventHandlerOwned(widget,XUI_EVENT_COMMAND,callback,&owner,&ref):
            xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref))==XUI_OK);
        xrtMemDebugSnapshot(&coldAfter);
        assert(coldAfter.LiveBytes-coldBefore.LiveBytes==sizeof(xui_event_owners_t)+
            (handler?XUI_WIDGET_EVENT_SLOT_COUNT*sizeof(xui_callback_owner_t):0));
        assert(coldAfter.AllocCount-coldBefore.AllocCount==(handler?2u:1u));
        assert(drop(&owner) && owner.refs==1);
        assert(dispatch(context,widget)==XUI_OK && owner.calls==1 && owner.released==1 && !owner.refs);
        assert(dispatch(context,widget)==XUI_OK && owner.calls==1);
        xuiDestroy(context);
    }
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner owner={.refs=1,.action=NEST,.context=context};xui_callback_owner_t ref=descriptor(&owner);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(unsigned i=0;i<100;++i)assert(dispatch(context,widget)==XUI_OK);
    xrtMemDebugSnapshot(&after);
    assert(owner.calls==200 && owner.refs==2 && before.AllocCount==after.AllocCount &&
        before.ReallocCount==after.ReallocCount && before.FreeCount==after.FreeCount);
    assert(drop(&owner));xuiDestroy(context);assert(owner.released==1);
}
static void closing(void)
{
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner owner={.refs=1,.action=CLOSE,.context=context};xui_callback_owner_t ref=descriptor(&owner);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_OK && drop(&owner));
    assert(dispatch(context,widget)==XUI_OK && owner.calls==1 && owner.released==1);
    create(&context,&widget);
    owner=(Owner){.refs=1,.action=CLEANUP_CLOSE,.context=context};ref=descriptor(&owner);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_OK && drop(&owner));
    Owner next={.refs=1,.context=context};xui_callback_owner_t nextRef=descriptor(&next);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&next,&nextRef)==XUI_OK);
    assert(owner.released==1 && next.refs==1 && drop(&next) && next.released==1);
}
static void failures(void)
{
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner owner={.refs=1,.context=context};xui_callback_owner_t ref=descriptor(&owner);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(owner.refs==1 && !widget->onEvent && !widget->pEventOwners);
    owner.failHold=true;assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_ERROR_CALLBACK_FAILED);
    assert(owner.refs==1 && !widget->onEvent && !widget->pEventOwners);owner.failHold=false;
    for(int64_t point=0;point<2;++point){
        assert(xrtMemDebugFailAfter(point));
        assert(xuiWidgetSetEventHandlerOwned(widget,XUI_EVENT_COMMAND,callback,&owner,&ref)==XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert(owner.refs==1 && !widget->pEventOwners && !widget->arrEventHandlers[XUI_EVENT_COMMAND]);
    }
    xui_callback_owner_t bad=ref;bad.onDrop=NULL;
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&bad)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetSetEventCallbackOwned(widget,NULL,&owner,&ref)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_OK && drop(&owner));owner.failDrop=true;
    assert(xuiWidgetSetEventCallback(widget,NULL,NULL)==XUI_ERROR_CALLBACK_FAILED);
    assert(owner.released==1 && !widget->onEvent && !xrtGetError());
    Owner final={.refs=1,.context=context};ref=descriptor(&final);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&final,&ref)==XUI_OK && drop(&final));final.failDrop=true;
    xui_call_scope_t outer={0},inner={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK && xuiCallScopeBegin(context,&inner)==XUI_OK);
    xuiDestroy(context);assert(!final.released);
    assert(xuiCallScopeEnd(&outer)==XUI_OK);
    assert(xuiCallScopeEnd(&inner)==XUI_ERROR_CALLBACK_FAILED && final.released==1 && !xrtGetError());
    assert(!inner.pContext && !inner.pSelf && !inner.pPrevious && !inner.pNext && !inner.iResult && !inner.bEnding);
}
static bool visit(xrtownershipref ref,void* raw)
{unsigned* count=raw;assert(ref.Ops==&ops && ref.Data);++*count;return true;}
static void graph(void)
{
    xui_context context=NULL;xui_widget widget=NULL;create(&context,&widget);
    Owner owner={.refs=1,.context=context};xui_callback_owner_t ref=descriptor(&owner);
    assert(xuiWidgetSetEventCallbackOwned(widget,callback,&owner,&ref)==XUI_OK);
    assert(xuiWidgetSetEventHandlerOwned(widget,XUI_EVENT_COMMAND,callback,&owner,&ref)==XUI_OK && owner.refs==3);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));unsigned edges=0;
    assert(xuiContextTraceEventOwners(context,visit,&edges) && edges==2);
    assert(xrtOwnershipScopeEnd(&freeze));
    xui_call_scope_t call={0};assert(xuiCallScopeBegin(context,&call)==XUI_OK);
    assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextOwnershipBusy(context));
    assert(!xuiContextTraceEventOwners(context,visit,&edges) && edges==2);
    assert(xrtOwnershipScopeEnd(&freeze) && xuiCallScopeEnd(&call)==XUI_OK);
    xuiDestroy(context);assert(owner.refs==1 && drop(&owner) && owner.released==1);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    replacement_and_nested();closing();failures();graph();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("owned XUI events: native return-tail pins, replacement, nesting, close, checked failures, OOM, graph and zero steady-state allocations passed");
    return 0;
}
