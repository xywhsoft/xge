#define XUI_OWNED_FONTS_HELPERS
#include "xui_owned_fonts_test.c"

typedef struct TypeGroup {
    size_t refs;
    xui_context context;
    xui_widget_type parent;
    unsigned calls[9];
    unsigned holds,drops;
    bool checkReentry,failHold,failDrop;
} TypeGroup;

static bool typeHold(const void* raw)
{
    TypeGroup* group=(TypeGroup*)raw;
    ++group->refs;
    ++group->holds;
    if(group->checkReentry){
        xui_widget_type_desc_t nested={.iSize=sizeof(nested),.sName="test.nested"};
        xui_widget_type output=NULL;
        assert(xuiWidgetUnregisterType(group->parent)==XUI_ERROR_BUSY);
        assert(xuiWidgetRegisterType(group->context,&output,&nested)==XUI_ERROR_BUSY && output==NULL);
        assert(xuiWidgetRegisterTypeOwned(group->context,&output,&nested,NULL)==XUI_ERROR_BUSY && output==NULL);
    }
    if(group->failHold)xrtSetErrorKind(XERR_RANGE);
    return true;
}

static bool typeDrop(const void* raw)
{
    TypeGroup* group=(TypeGroup*)raw;
    assert(group->refs>1);
    --group->refs;
    ++group->drops;
    if(group->failDrop)xrtSetErrorKind(XERR_STATE);
    return !group->failDrop;
}

static bool typeCount(const void* raw,size_t* out)
{*out=((const TypeGroup*)raw)->refs;return *out!=0;}

static const xrtownershipops groupOps={typeCount,trace};

static int initType(xui_widget widget,void* data,const void* input,void* user)
{
    TypeGroup* group=user;
    assert(widget && data && input);
    ++group->calls[0];
    *(unsigned*)data=*(const unsigned*)input;
    return XUI_OK;
}

static void destroyType(xui_widget widget,void* data,void* user)
{
    TypeGroup* group=user;
    assert(widget && data && *(unsigned*)data==71);
    ++group->calls[1];
}

static int measureType(xui_widget widget,xui_vec2_t constraint,xui_vec2_t* size,void* user)
{
    TypeGroup* group=user;
    assert(widget && constraint.fX==40);
    ++group->calls[2];
    *size=(xui_vec2_t){19,21};
    return XUI_OK;
}

static int prepareType(xui_widget widget,void* user)
{assert(widget);++((TypeGroup*)user)->calls[3];return XUI_OK;}

static int layoutType(xui_widget widget,xui_rect_t content,void* user)
{assert(widget && content.fW==95);++((TypeGroup*)user)->calls[4];return XUI_OK;}

static int completeType(xui_widget widget,xui_rect_t content,void* user)
{assert(widget && content.fW==95);++((TypeGroup*)user)->calls[5];return XUI_OK;}

static int renderType(xui_widget widget,xui_draw_context draw,uint32_t state,void* user)
{assert(widget && !draw && state==3);++((TypeGroup*)user)->calls[6];return XUI_OK;}

static int updateType(xui_widget widget,float delta,void* user)
{assert(widget && delta==0.25f);++((TypeGroup*)user)->calls[7];return XUI_OK;}

static int cursorType(xui_widget widget,int x,int y,void* user)
{assert(widget && x==4 && y==5);++((TypeGroup*)user)->calls[8];return 1;}

static int legacyPrepare(xui_widget widget,void* data)
{assert(widget && data && *(unsigned*)data==71);return XUI_OK;}

typedef struct TraceCount {const TypeGroup* group;unsigned count;} TraceCount;
static bool visitGroup(xrtownershipref ref,void* user)
{
    TraceCount* state=user;
    if(ref.Data==state->group){
        assert(ref.Ops==&groupOps);
        ++state->count;
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
    Owner proxyOwner={.refs=1};
    xui_test_proxy_state_t proxyState;
    xui_proxy proxy=proxyCreate(&proxyState,&proxyOwner);
    xui_context_desc_t contextDesc;
    xuiContextDescDefault(&contextDesc);
    contextDesc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    xui_context context=NULL;
    assert(xuiCreateOwnedEx(proxy,&contextDesc,&context)==XUI_OK);

    xui_widget_type parent=NULL,owned=NULL,derived=NULL;
    xui_widget_type_desc_t parentDesc={.iSize=sizeof(parentDesc),.sName="test.parent"};
    assert(xuiWidgetRegisterType(context,&parent,&parentDesc)==XUI_OK);
    TypeGroup group={.refs=1,.context=context,.parent=parent,.checkReentry=true};
    const xui_callback_owner_t binding={{&group,&groupOps},typeHold,typeDrop};
    xui_widget_type_desc_t typeDesc={.iSize=sizeof(typeDesc),.sName="test.owner",.pParent=parent,
        .iTypeDataSize=sizeof(unsigned),.pUser=&group,.onInit=initType,.onDestroy=destroyType,
        .onContentMeasure=measureType,.onLayoutPrepare=prepareType,.onLayoutChildren=layoutType,
        .onLayoutComplete=completeType,.onCacheRender=renderType,.onUpdate=updateType,.onQueryCursor=cursorType};
    assert(xuiWidgetRegisterTypeOwned(context,&owned,&typeDesc,NULL)==XUI_ERROR_INVALID_ARGUMENT && !owned);
    group.failHold=true;
    assert(xuiWidgetRegisterTypeOwned(context,&owned,&typeDesc,&binding)==XUI_ERROR_CALLBACK_FAILED && !owned);
    assert(group.refs==1 && group.holds==1 && group.drops==1 && xrtGetError());
    xrtClearError();
    group.failHold=false;

    for(int budget=0;budget<=1;++budget){
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiWidgetRegisterTypeOwned(context,&owned,&typeDesc,&binding)==XUI_ERROR_OUT_OF_MEMORY && !owned);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        assert(group.refs==1 && !xuiWidgetFindType(context,typeDesc.sName));
    }
    assert(xuiWidgetRegisterTypeOwned(context,&owned,&typeDesc,&binding)==XUI_OK && owned);
    assert(group.refs==2 && group.holds==4 && group.drops==3);
    TraceCount edges={&group,0};
    assert(xuiContextTraceOwnership(context,visitGroup,&edges) && edges.count==1);

    xui_widget_type_desc_t childDesc={.iSize=sizeof(childDesc),.sName="test.derived",.pParent=owned,
        .iTypeDataSize=sizeof(unsigned)};
    assert(xuiWidgetRegisterType(context,&derived,&childDesc)==XUI_OK && derived);
    xui_widget widget=NULL;
    const unsigned input=71;
    assert(xuiWidgetCreateTyped(context,derived,&widget,&input)==XUI_OK && widget);
    xui_vec2_t measured={0};
    xui_rect_t area={0,0,95,30};
    assert(widget->onContentMeasure(widget,(xui_vec2_t){40,0},&measured,widget->pContentMeasureUser)==XUI_OK);
    assert(measured.fX==19 && measured.fY==21);
    assert(widget->onLayoutPrepare(widget,widget->pLayoutPrepareUser)==XUI_OK);
    assert(widget->onLayoutChildren(widget,area,widget->pLayoutChildrenUser)==XUI_OK);
    assert(widget->onLayoutComplete(widget,area,widget->pLayoutCompleteUser)==XUI_OK);
    assert(widget->onCacheRender(widget,NULL,3,widget->pCacheRenderUser)==XUI_OK);
    assert(widget->onUpdate(widget,0.25f,widget->pUpdateUser)==XUI_OK);
    assert(widget->onQueryCursor(widget,4,5,widget->pQueryCursorUser)==1);
    edges.count=0;
    assert(xuiContextTraceOwnership(context,visitGroup,&edges) && edges.count==1);
    assert(xuiWidgetUnregisterType(owned)==XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
    for(unsigned i=0;i<9;++i)assert(group.calls[i]==1);
    assert(group.refs==2 && xuiWidgetUnregisterType(derived)==XUI_OK);

    childDesc.onLayoutPrepare=legacyPrepare;
    assert(xuiWidgetRegisterType(context,&derived,&childDesc)==XUI_OK);
    assert(xuiWidgetCreateTyped(context,derived,&widget,&input)==XUI_OK);
    assert(widget->onLayoutPrepare(widget,widget->pLayoutPrepareUser)==XUI_OK && group.calls[3]==1);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && xuiWidgetUnregisterType(derived)==XUI_OK);
    group.failDrop=true;
    assert(xuiWidgetUnregisterType(owned)==XUI_ERROR_CALLBACK_FAILED && xrtGetError());
    xrtClearError();
    assert(group.refs==1 && xuiWidgetFindType(context,typeDesc.sName)==NULL);
    edges.count=0;
    assert(xuiContextTraceOwnership(context,visitGroup,&edges) && edges.count==0);

    group.failDrop=false;
    assert(xuiWidgetRegisterTypeOwned(context,&owned,&typeDesc,&binding)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && group.refs==1 && group.holds==group.drops);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && proxyOwner.refs==1 && drop(&proxyOwner));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Owned WidgetType virtuals, inheritance and traced retirement passed");
    return 0;
}
