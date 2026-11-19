#define XUI_OWNED_FONTS_HELPERS
#include "xui_owned_fonts_test.c"

typedef struct StateTest {
    xui_widget busyWidget;
    xvalue* expected;
    unsigned reads;
    unsigned busyDrops;
    unsigned replacementDrops;
} StateTest;

static void inspectDestroy(xui_widget widget,void* nativeData,void* user)
{
    StateTest* test=user;
    assert(nativeData==NULL);
    xvalue* state=NULL;
    assert(xuiWidgetGetInstanceState(widget,&state)==XUI_OK && state==test->expected);
    xrtValueRelease(state);
    assert(xuiWidgetSetInstanceState(widget,NULL)==XUI_ERROR_INVALID_STATE);
    ++test->reads;
}

static void inspectReplacement(void* data,void* user)
{
    StateTest* test=data;
    xui_widget widget=user;
    xvalue* value=NULL;
    assert(xuiWidgetGetInstanceState(widget,&value)==XUI_OK && value==test->expected);
    xrtValueRelease(value);
    ++test->replacementDrops;
}

static void inspectBusyState(void* data,void* user)
{
    StateTest* test=data;
    xui_context context=user;
    assert(xuiContextOwnershipBusy(context));
    assert(xuiWidgetSetInstanceState(test->busyWidget,NULL)==XUI_ERROR_BUSY);
    unsigned before=test->reads;
    assert(xuiWidgetDestroyChecked(test->busyWidget)==XUI_OK);
    assert(test->reads==before);
    ++test->busyDrops;
}

static const xvaluehandleops replaceOps={.Drop=inspectReplacement};
static const xvaluehandleops busyOps={.Drop=inspectBusyState};
static xvalue* handle(StateTest* test,void* user,const xvaluehandleops* ops)
{
    void* pointer=test;
    xvalue* result=xrtValueHandleTake(&pointer,ops,user);
    assert(result && !pointer);
    return result;
}

typedef struct StateTrace {xvalue* value;unsigned count;} StateTrace;
static bool visitState(xrtownershipref ref,void* user)
{
    StateTrace* probe=user;
    if(ref.Data==probe->value){
        assert(ref.Ops==xrtValueOwnership(probe->value).Ops);
        ++probe->count;
    }
    return true;
}

int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    Owner owner={.refs=1};
    xui_test_proxy_state_t backend;
    xui_proxy proxy=proxyCreate(&backend,&owner);
    xui_context_desc_t config;
    xuiContextDescDefault(&config);
    config.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    xui_context context=NULL;
    assert(xuiCreateOwnedEx(proxy,&config,&context)==XUI_OK);
    xui_widget ordinary=NULL;
    assert(xuiWidgetCreate(context,&ordinary)==XUI_OK);
    xvalue* output=NULL;
    assert(xuiWidgetGetInstanceState(ordinary,&output)==XUI_ERROR_UNSUPPORTED && !output);
    assert(xuiWidgetSetInstanceState(ordinary,NULL)==XUI_ERROR_UNSUPPORTED);
    assert(xuiWidgetDestroyChecked(ordinary)==XUI_OK);

    StateTest test={0};
    xui_widget_type type=NULL,derived=NULL;
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="state.base",.iFlags=XUI_WIDGET_TYPE_MANAGED_STATE,
        .pUser=&test,.onDestroy=inspectDestroy};
    assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK);
    xui_widget_type_desc_t child={.iSize=sizeof(child),.sName="state.derived",.pParent=type};
    assert(xuiWidgetRegisterType(context,&derived,&child)==XUI_OK);
    xui_widget a=NULL,b=NULL;
    assert(xuiWidgetCreateTyped(context,derived,&a,NULL)==XUI_OK && xuiWidgetCreateTyped(context,type,&b,NULL)==XUI_OK);
    assert(xuiWidgetGetTypeData(a)==NULL && xuiWidgetGetTypeData(b)==NULL);
    assert(xuiWidgetGetInstanceState(a,&output)==XUI_OK && !output);
    assert(xuiWidgetGetInstanceState(b,&output)==XUI_OK && !output);

    xvalue* first=handle(&test,a,&replaceOps);
    xvalue* value=xrtValueInt(42);
    assert(value && xuiWidgetSetInstanceState(a,first)==XUI_OK);
    xrtValueRelease(first);
    test.expected=value;
    assert(xuiWidgetSetInstanceState(a,value)==XUI_OK && test.replacementDrops==1);
    assert(xuiWidgetGetInstanceState(b,&output)==XUI_OK && !output);
    StateTrace edge={value,0};
    assert(xuiContextTraceOwnership(context,visitState,&edge) && edge.count==1);

    xvalue* alternate=xrtValueInt(18);
    assert(alternate);
    xmemdebugsnapshot warmBefore,warmAfter;
    xrtMemDebugSnapshot(&warmBefore);
    assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<1000;++i){
        assert(xuiWidgetGetInstanceState(a,&output)==XUI_OK && output==value);
        xrtValueRelease(output);
        assert(xuiWidgetSetInstanceState(a,alternate)==XUI_OK);
        assert(xuiWidgetSetInstanceState(a,value)==XUI_OK);
    }
    assert(xrtMemDebugFailAfter(-1));
    xrtMemDebugSnapshot(&warmAfter);
    assert(warmBefore.AllocCount==warmAfter.AllocCount && warmBefore.FreeCount==warmAfter.FreeCount &&
        warmBefore.ReallocCount==warmAfter.ReallocCount);
    xrtValueRelease(alternate);
    assert(xuiWidgetDestroyChecked(a)==XUI_OK && test.reads==1);
    xrtValueRelease(value);

    xvalue* object=xrtValueObject();
    xvalue* member=handle(&test,context,&busyOps);
    xstrview key={"busy",4};
    assert(object && xrtValueObjectSet(object,key,member) && xuiWidgetSetInstanceState(b,object)==XUI_OK);
    xrtValueRelease(member);
    test.expected=object;
    test.busyWidget=b;
    assert(xrtValueObjectRemove(object,key) && test.busyDrops==1 && test.reads==1);
    assert(xuiLayout(context)==XUI_OK && test.reads==2);
    xrtValueRelease(object);

    assert(xuiWidgetUnregisterType(derived)==XUI_OK);
    assert(xuiWidgetUnregisterType(type)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    assert(owner.refs==1 && drop(&owner));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Widget instance state identity, busy retirement and 1000 warm pairs passed");
    return 0;
}
