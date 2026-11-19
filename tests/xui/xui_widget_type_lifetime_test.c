#define XUI_OWNED_FONTS_HELPERS
#include "xui_owned_fonts_test.c"

typedef struct TypeWatch {
    xui_context context;
    xui_widget_type type;
    xui_lifetime_link_t* peer;
    unsigned calls;
} TypeWatch;

static void invalidate(void* user)
{
    TypeWatch* watch=user;
    ++watch->calls;
    if(watch->peer)xuiLifetimeUnwatch(watch->peer);
}

static void inspectRetired(void* user)
{
    TypeWatch* watch=user;
    invalidate(user);
    assert(xuiWidgetFindType(watch->context,"test.child")==NULL);
    assert(xuiStyleFindProperty(watch->context,"test.scoped")==0);
    assert(xuiWidgetTypeGetName(watch->type)==NULL);
    xui_lifetime_link_t late={0};
    assert(xuiWidgetTypeLifetimeWatch(watch->type,&late,invalidate,watch)==XUI_ERROR_INVALID_ARGUMENT);
}

static void inspectClosing(void* user)
{
    TypeWatch* watch=user;
    invalidate(user);
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName="test.reentrant"};
    xui_widget_type output=NULL;
    assert(xuiWidgetRegisterType(watch->context,&output,&desc)==XUI_ERROR_INVALID_STATE && output==NULL);
}

static xui_widget_type registerType(xui_context context,const char* name,xui_widget_type parent)
{
    xui_widget_type_desc_t desc={.iSize=sizeof(desc),.sName=name,.pParent=parent};
    xui_widget_type type=NULL;
    assert(xuiWidgetRegisterType(context,&type,&desc)==XUI_OK && type!=NULL);
    return type;
}

static uint32_t registerProperty(xui_context context,const char* name,xui_widget_type type)
{
    xui_style_property_info_t info={.iSize=sizeof(info),.sName=name,.iValueType=XUI_STYLE_VALUE_STRING,.pWidgetType=type,
        .tDefaultValue={.iSize=sizeof(xui_style_value_t),.iType=XUI_STYLE_VALUE_STRING,.sText="default"}};
    uint32_t id=0;
    assert(xuiStyleRegisterProperty(context,&info,&id)==XUI_OK && id!=0);
    return id;
}

int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    xui_test_proxy_state_t state;
    Owner owner={.refs=1};
    xui_proxy proxy=proxyCreate(&state,&owner);
    xui_context context=NULL;
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);

    xui_widget_type base=xuiWidgetGetBaseType();
    xui_widget_type parent=registerType(context,"test.parent",base);
    xui_widget_type child=registerType(context,"test.child",parent);
    assert(xuiWidgetTypeGetParent(child)==parent);
    assert(xuiWidgetUnregisterType(parent)==XUI_ERROR_ALREADY_INITIALIZED);
    const uint32_t id=registerProperty(context,"test.scoped",child);
    const uint32_t global=registerProperty(context,"test.global",NULL);
    xui_style_property_t property={.iSize=sizeof(property),.iPropertyId=id,
        .tValue={.iSize=sizeof(xui_style_value_t),.iType=XUI_STYLE_VALUE_STRING,.sText="type rule"}};
    xui_style_desc_t rule={.iSize=sizeof(rule),.pProperties=&property,.iPropertyCount=1};
    assert(xuiStyleSetType(context,child,&rule)==XUI_OK);
    xui_widget widget=NULL;
    assert(xuiWidgetCreateTyped(context,child,&widget,NULL)==XUI_OK && xuiWidgetResolveStyle(widget)==XUI_OK);
    xui_style_property_t resolved={0};
    assert(xuiWidgetGetResolvedStyleProperty(widget,"test.scoped",&resolved)==XUI_OK);
    assert(!strcmp(resolved.tValue.sText,"type rule"));
    assert(xuiWidgetUnregisterType(child)==XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK);

    xui_lifetime_link_t removed={0},watch={0},baseLink={0},parentLink={0};
    TypeWatch first={context,child,&removed,0};
    TypeWatch peer={context,child,NULL,0};
    TypeWatch permanent={context,base,NULL,0};
    TypeWatch closing={context,parent,NULL,0};
    assert(xuiWidgetTypeLifetimeWatch(child,&removed,invalidate,&peer)==XUI_OK);
    assert(xuiWidgetTypeLifetimeWatch(child,&watch,inspectRetired,&first)==XUI_OK);
    assert(xuiWidgetTypeLifetimeWatch(child,&watch,inspectRetired,&first)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetTypeLifetimeWatch(base,&baseLink,invalidate,&permanent)==XUI_OK && baseLink.ppPrevious==NULL);
    assert(xuiWidgetTypeLifetimeWatch(parent,&parentLink,inspectClosing,&closing)==XUI_OK);

    const uint32_t generation=xuiStyleGetGeneration(context);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetUnregisterType(child)==XUI_OK);
    assert(xrtMemDebugFailAfter(-1));
    assert(first.calls==1 && peer.calls==0 && !watch.ppPrevious && !removed.ppPrevious);
    assert(xuiStyleGetGeneration(context)==generation+1);
    assert(xuiStyleFindProperty(context,"test.global")==global);
    xui_style_property_info_t info={0};
    assert(xuiStyleGetPropertyInfo(context,id,&info)==XUI_ERROR_FILE_NOT_FOUND);
    child=registerType(context,"test.child",parent);
    assert(xuiWidgetCreateTyped(context,child,&widget,NULL)==XUI_OK && xuiWidgetResolveStyle(widget)==XUI_OK);
    assert(xuiWidgetGetResolvedStyleProperty(widget,"test.scoped",&resolved)==XUI_ERROR_FILE_NOT_FOUND);

    xmemdebugsnapshot warmBefore,warmAfter;
    xrtMemDebugSnapshot(&warmBefore);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;i++){
        assert(xuiWidgetFindType(context,"test.child")==child && xuiWidgetGetType(widget)==child);
        assert(xuiWidgetTypeLifetimeWatch(child,&watch,invalidate,&first)==XUI_OK);
        xuiLifetimeUnwatch(&watch);
    }
    assert(xrtMemDebugFailAfter(-1));
    xrtMemDebugSnapshot(&warmAfter);
    assert(warmBefore.AllocCount==warmAfter.AllocCount && warmBefore.FreeCount==warmAfter.FreeCount &&
        warmBefore.ReallocCount==warmAfter.ReallocCount);
    xuiLifetimeUnwatch(&baseLink);

    assert(xuiDestroyChecked(context)==XUI_OK);
    assert(closing.calls==1 && permanent.calls==0 && !parentLink.ppPrevious);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && owner.refs==1 && drop(&owner));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("WidgetType lifetime, registry retirement and 1000 warm watches passed");
    return 0;
}
