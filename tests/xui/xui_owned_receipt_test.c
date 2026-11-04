/* CPU-only native cleanup receipt regression. Uses the real Context, shared
 * Font/Proxy owners, nested scopes and checked logical-close entry points. */
#define main xui_unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main

typedef struct ReceiptProbe {
    size_t references;
    unsigned fontDrops,proxyDrops,reentries;
    int fontStatus;
    bool proxyFails,reenter;
    xerror* primary;
    xerror* secondary;
    xui_context context;
    xui_call_scope_t* ending;
    xui_test_proxy_state_t state;
} ReceiptProbe;
static bool receiptHold(const void* raw){++((ReceiptProbe*)raw)->references;return true;}
static bool receiptDrop(const void* raw)
{
    ReceiptProbe* p=(void*)raw;assert(p->references);
    if(--p->references)return true;
    ++p->proxyDrops;
    if(p->secondary)xrtSetErrorTake(xrtErrorRef(p->secondary));
    return !p->proxyFails;
}
static bool receiptCount(const void* raw,size_t* output){*output=((const ReceiptProbe*)raw)->references;return *output!=0;}
static const xrtownershipops receiptOps={receiptCount,trace};
static int receiptFontFinalize(xui_proxy proxy,void* payload)
{
    (void)proxy;ReceiptProbe* p=*(ReceiptProbe**)payload;++p->fontDrops;
    if(p->reenter){
        bool consumed=true;assert(xuiDestroyOwnerChecked(p->context,&consumed)==XUI_ERROR_INVALID_STATE && !consumed);
        if(p->ending)assert(xuiCallScopeEnd(p->ending)==XUI_ERROR_INVALID_STATE);
        ++p->reentries;
    }
    if(p->primary)xrtSetErrorTake(xrtErrorRef(p->primary));
    return p->fontStatus;
}
static xerror* receiptError(xerrkind kind)
{xrtSetErrorInfo(kind,"receipt",41,"retained receipt diagnostic");xerror* result=xrtTakeError();assert(result);return result;}
static xui_context receiptCreate(ReceiptProbe* p)
{
    p->references=1;xuiTestProxyInit(&p->state);
    xui_callback_owner_t owner={{p,&receiptOps},receiptHold,receiptDrop};
    xui_proxy proxy=NULL;assert(xuiProxyCreateOwned(&p->state.tProxy,&owner,&proxy)==XUI_OK);
    xui_font font=NULL;void* payload=NULL;
    assert(xuiFontCreateOwned(proxy,sizeof(ReceiptProbe*),receiptFontFinalize,&font,&payload)==XUI_OK);
    *(ReceiptProbe**)payload=p;xui_context context=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    p->context=context;
    assert(xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK && receiptDrop(p));
    return context;
}
static void receiptScopeEmpty(const xui_call_scope_t* scope)
{const xui_call_scope_t empty={0};assert(!memcmp(scope,&empty,sizeof(empty)));}
static void receiptCloseMatrix(void)
{
    unsigned cases=0;
    for(int primary=0;primary<2;++primary)for(int secondary=0;secondary<2;++secondary)
    for(int nesting=0;nesting<3;++nesting)for(int oom=0;oom<2;++oom){
        ReceiptProbe p={.fontStatus=XUI_ERROR_CALLBACK_FAILED,.proxyFails=true,.reenter=true};
        if(primary)p.primary=receiptError(XERR_TYPE);
        if(secondary)p.secondary=receiptError(XERR_STATE);
        xui_context context=receiptCreate(&p);
        xui_call_scope_t outer={0},inner={0};
        if(nesting)assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
        if(nesting==2)assert(xuiCallScopeBegin(context,&inner)==XUI_OK);
        if(nesting)p.ending=&outer;
        if(oom)assert(xrtMemDebugFailAfter(0));
        bool consumed=false;int status=xuiDestroyOwnerChecked(context,&consumed);assert(consumed);
        if(nesting){
            assert(status==XUI_OK && p.fontDrops==0 && p.proxyDrops==0);
            if(nesting==2){assert(xuiCallScopeEnd(&inner)==XUI_OK);receiptScopeEmpty(&inner);}
            status=xuiCallScopeEnd(&outer);receiptScopeEmpty(&outer);
        }
        assert(status==XUI_ERROR_CALLBACK_FAILED && p.fontDrops==1 && p.proxyDrops==1 && p.references==0 && p.reentries==1);
        assert(xrtGetError()==p.primary);
        if(oom){assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();}
        xrtClearError();xrtErrorFree(p.primary);xrtErrorFree(p.secondary);++cases;
    }
    printf("context close matrix: %u exact-first-error cases\n",cases);
}
static void receiptNested(void)
{
    for(int diagnostic=0;diagnostic<2;++diagnostic){
        ReceiptProbe p={.fontStatus=XUI_ERROR_OUT_OF_MEMORY,.proxyFails=true};
        if(diagnostic)p.primary=receiptError(XERR_RANGE);
        p.secondary=receiptError(XERR_STATE);
        xui_context context=receiptCreate(&p);xui_call_scope_t outer={0},inner={0};
        assert(xuiCallScopeBegin(context,&outer)==XUI_OK && xuiCallScopeBegin(context,&inner)==XUI_OK);
        xuiClearFonts(context);assert(p.fontDrops==1 && xrtGetError()==p.primary);
        xrtClearError();assert(xuiCallScopeEnd(&inner)==XUI_ERROR_OUT_OF_MEMORY && xrtGetError()==p.primary);
        receiptScopeEmpty(&inner);xrtClearError();
        assert(xuiDestroyChecked(context)==XUI_OK && !p.proxyDrops);
        assert(xuiCallScopeEnd(&outer)==XUI_ERROR_OUT_OF_MEMORY && xrtGetError()==p.primary);
        assert(p.proxyDrops==1 && !p.references);receiptScopeEmpty(&outer);
        xrtClearError();xrtErrorFree(p.primary);xrtErrorFree(p.secondary);
    }
}
static int receiptDeferredEvent(xui_widget widget,const xui_event_t* event,void* raw)
{
    (void)widget;(void)event;ReceiptProbe* p=raw;++p->reentries;xuiDestroy(p->context);
    assert(!p->fontDrops && !p->proxyDrops);return 0;
}
static void receiptLegacyAndDeferred(void)
{
    for(int deferred=0;deferred<2;++deferred){
        ReceiptProbe p={.fontStatus=XUI_ERROR_CALLBACK_FAILED,.proxyFails=true};
        p.secondary=receiptError(XERR_STATE);xui_context context=receiptCreate(&p);
        if(deferred){
            xui_widget root=NULL;assert(xuiWidgetCreate(context,&root)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
            assert(xuiWidgetSetEventCallback(root,receiptDeferredEvent,&p)==XUI_OK);
            xui_event_t event={.iSize=sizeof(event),.iType=XUI_EVENT_COMMAND,.pTarget=root};
            (void)xuiDispatchEvent(context,&event);assert(p.reentries==1);
        }else xuiDestroy(context);
        assert(p.fontDrops==1 && p.proxyDrops==1 && !p.references && !xrtGetError());xrtErrorFree(p.secondary);
    }
}
static void receiptSuccessAndValidation(void)
{
    xui_call_scope_t scope={0};assert(xuiCallScopeEnd(&scope)==XUI_OK);receiptScopeEmpty(&scope);
    bool consumed=true;assert(xuiDestroyOwnerChecked(NULL,&consumed)==XUI_ERROR_INVALID_ARGUMENT && !consumed);
    ReceiptProbe p={0};xui_context context=receiptCreate(&p);
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK);
    xui_call_scope_t copy=scope;assert(xuiCallScopeEnd(&copy)==XUI_ERROR_INVALID_STATE);
    assert(xuiDestroyOwnerChecked(context,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);receiptScopeEmpty(&scope);
    assert(xuiDestroyChecked(context)==XUI_OK && p.fontDrops==1 && p.proxyDrops==1 && !p.references && !xrtGetError());
}
typedef struct ReceiptCellOwner {
    size_t references;unsigned drops;bool fails;xerror* error;xui_table_cell_scope_t* ending;
} ReceiptCellOwner;
static bool receiptCellHold(const void* raw){++((ReceiptCellOwner*)raw)->references;return true;}
static bool receiptCellDrop(const void* raw)
{
    ReceiptCellOwner* p=(void*)raw;assert(p->references);--p->references;++p->drops;
    if(p->ending)assert(xuiTableViewCellOutputCallScopeEnd(p->ending)==XUI_ERROR_INVALID_STATE);
    if(p->error)xrtSetErrorTake(xrtErrorRef(p->error));
    return !p->fails;
}
static bool receiptCellCount(const void* raw,size_t* out){*out=((const ReceiptCellOwner*)raw)->references;return *out!=0;}
static const xrtownershipops receiptCellOps={receiptCellCount,trace};
static xstrview receiptCellText(xui_widget widget,int row,int column,const xui_table_view_cell_output_t* cell,void* user)
{(void)widget;(void)row;(void)column;(void)cell;(void)user;return (xstrview){0};}
static void receiptCell(void)
{
    for(int final=0;final<2;++final)for(int diagnostic=0;diagnostic<2;++diagnostic)for(int oom=0;oom<2;++oom){
        ReceiptCellOwner owner={.references=1};if(diagnostic)owner.error=receiptError(XERR_RANGE);
        xui_callback_owner_t native={{&owner,&receiptCellOps},receiptCellHold,receiptCellDrop};
        xui_table_view_cell_output_t* cell=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK);
        assert(xuiTableViewCellOutputSetFormatterOwned(cell,receiptCellText,NULL,&native)==XUI_OK);
        owner.fails=true;xui_table_cell_scope_t outer={0},inner={0};
        assert(xuiTableViewCellOutputCallScopeBegin(cell,&outer)==XUI_OK);
        if(!final)assert(xuiTableViewCellOutputCallScopeBegin(cell,&inner)==XUI_OK);
        if(oom)assert(xrtMemDebugFailAfter(0));
        if(final){
            owner.ending=&outer;bool consumed=false;
            assert(xuiTableViewCellOutputReleaseOwnerChecked(cell,&consumed)==XUI_OK && consumed && !owner.drops);
        }else{
            assert(xuiTableViewCellOutputReset(cell)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==owner.error);
            xrtClearError();assert(xuiTableViewCellOutputCallScopeEnd(&inner)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==owner.error);
            xrtClearError();
        }
        assert(xuiTableViewCellOutputCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==owner.error);
        const xui_table_cell_scope_t empty={0};assert(!memcmp(&outer,&empty,sizeof(empty)));
        assert(owner.drops==1 && owner.references==1);
        if(oom){assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();}
        xrtClearError();if(!final)assert(xuiTableViewCellOutputReleaseChecked(cell)==XUI_OK);
        xrtErrorFree(owner.error);
    }
}
typedef struct ReceiptHold {size_t references;xui_context context;unsigned calls;} ReceiptHold;
static bool receiptRejectHold(const void* raw)
{ReceiptHold* p=(void*)raw;++p->calls;assert(xuiDestroyChecked(p->context)==XUI_OK);return false;}
static bool receiptUnusedDrop(const void* raw){(void)raw;assert(!"failed Hold must not Drop");return true;}
static bool receiptHoldCount(const void* raw,size_t* out){*out=((const ReceiptHold*)raw)->references;return *out!=0;}
static const xrtownershipops receiptHoldOps={receiptHoldCount,trace};
static int receiptUnusedEvent(xui_widget widget,const xui_event_t* event,void* user)
{(void)widget;(void)event;(void)user;assert(!"unpublished callback");return 0;}
static void receiptUnusedClick(xui_widget widget,void* user)
{(void)widget;(void)user;assert(!"unpublished callback");}
static void receiptRejectedHold(void)
{
    for(int baseEvent=0;baseEvent<2;++baseEvent){
        ReceiptProbe p={.fontStatus=XUI_ERROR_CALLBACK_FAILED,.proxyFails=true};
        p.primary=receiptError(XERR_TYPE);p.secondary=receiptError(XERR_STATE);
        xui_context context=receiptCreate(&p);xui_widget widget=NULL;
        assert(xuiButtonCreate(context,&widget,NULL)==XUI_OK);
        ReceiptHold holder={1,context,0};
        xui_callback_owner_t owner={{&holder,&receiptHoldOps},receiptRejectHold,receiptUnusedDrop};
        int status=baseEvent ? xuiWidgetSetEventCallbackOwned(widget,receiptUnusedEvent,NULL,&owner):
            xuiButtonSetClickOwned(widget,receiptUnusedClick,NULL,&owner);
        assert(status==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
        assert(holder.calls==1 && p.fontDrops==1 && p.proxyDrops==1 && !p.references);
        xrtErrorFree(p.primary);xrtErrorFree(p.secondary);
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    receiptSuccessAndValidation();receiptCloseMatrix();receiptNested();receiptLegacyAndDeferred();receiptCell();receiptRejectedHold();
    xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    printf("native cleanup receipt: success, nested/refbalance, OOM, reentry, legacy/deferred passed\n");return 0;
}
