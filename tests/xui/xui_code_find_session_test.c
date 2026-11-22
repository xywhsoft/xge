#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#if defined(XGE_XRT_HOST) && !defined(XGE_STATIC_BUILD)
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

#define CHECK(e) do { ++checks; if(!(e)){fprintf(stderr,"code_find_session:%d: %s\n",__LINE__,#e);failed=1;goto cleanup;} } while(0)
static xui_widget child(xui_context context,xui_widget w,const char* type,const char* label,int* index)
{
    if(!w)return NULL;
    if(xuiWidgetGetType(w)==xuiWidgetFindType(context,type)) {
        const char* text=!strcmp(type,"button") ? xuiButtonGetText(w):!strcmp(type,"checkbox") ? xuiCheckBoxGetText(w):"";
        if((!label || !strcmp(text,label)) && (*index)--==0)return w;
    }
    for(xui_widget c=xuiWidgetGetFirstChild(w);c;c=xuiWidgetGetNextSibling(c)){xui_widget found=child(context,c,type,label,index);if(found)return found;}
    return NULL;
}
static xui_widget control(xui_context context,xui_widget window,const char* type,int label,int index)
{return child(context,window,type,label ? xuiTranslate(context,label):NULL,&index);}
static int editor(xui_context context,xui_widget root,const char* text,xui_widget* out)
{
    xui_code_edit_desc_t desc={0};desc.iSize=sizeof(desc);desc.sText=text;desc.sLanguage="c";
    int result=xuiCodeEditCreate(context,out,&desc);
    if(result==XUI_OK)result=xuiWidgetAddChild(root,*out);
    return result;
}
static void destroyScope(xui_code_find_scope scope,xui_widget editor,const xui_code_find_result_t* result,void* user)
{(void)editor;(void)result;*(int*)user=0;xuiCodeFindScopeDestroy(scope);*(int*)user=result->iEnd>result->iStart;}

typedef struct activate_owner {
    size_t refs;
    int calls, invalidated, status;
    bool deny, failDrop, clear, close;
} activate_owner;

static bool holdActivate(const void* raw)
{
    activate_owner* owner=(void*)raw;
    if(owner->deny)return false;
    ++owner->refs;
    return true;
}
static bool dropActivate(const void* raw)
{
    activate_owner* owner=(void*)raw;
    assert(owner->refs>1);
    --owner->refs;
    return !owner->failDrop;
}
static bool countActivate(const void* raw,size_t* out)
{*out=((const activate_owner*)raw)->refs;return true;}
static bool traceActivate(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops activateOps={countActivate,traceActivate};
static bool findActivateEdge(xrtownershipref ref,void* user)
{return ref.Data==user && ref.Ops==&activateOps;}
static void scopeInvalidated(void* user)
{++((activate_owner*)user)->invalidated;}
static int activate(xui_code_find_scope scope,xui_widget editor,const xui_code_find_result_t* result,void* user)
{
    activate_owner* owner=user;
    ++owner->calls;
    assert(owner->refs==3 && result->pEditor==editor && result->iEnd>result->iStart);
    xstrview preview={0};
    assert(xuiCodeFindResultGetPreviewView(result,&preview)==XUI_OK &&
           preview.Data==result->sPreview && preview.Size==strlen(result->sPreview));
    assert(xuiCodeFindScopeOwnershipBusy(scope) && !xuiCodeFindScopeTraceOwnership(scope,findActivateEdge,owner));
    if(owner->clear){
        assert(xuiCodeFindScopeSetActivateOwned(scope,NULL,NULL,NULL)==XUI_OK && owner->refs==2);
        assert(xuiCodeFindScopeClearEditors(scope)==XUI_OK && strstr(result->sPreview,"alpha")!=NULL);
    }
    if(owner->close){
        bool consumed=false;
        assert(xuiCodeFindScopeDestroyOwnerChecked(scope,&consumed)==XUI_OK && consumed && !owner->invalidated);
        assert(xuiCodeFindScopeSetActivateOwned(scope,NULL,NULL,NULL)==XUI_ERROR_INVALID_STATE);
    }
    return owner->status;
}
static int32 checkFindThread(ptr raw)
{
    xui_code_find_scope_call_t call={0};
    bool consumed=true;
    assert(xuiCodeFindScopeCallBegin(raw,&call)==XUI_ERROR_BUSY && !call.pScope);
    assert(xuiCodeFindScopeSetActivateOwned(raw,NULL,NULL,NULL)==XUI_ERROR_BUSY);
    assert(xuiCodeFindScopeActivateResult(raw,0)==XUI_ERROR_BUSY);
    assert(xuiCodeFindScopeDestroyOwnerChecked(raw,&consumed)==XUI_ERROR_BUSY && !consumed);
    return 0;
}

int main(void)
{
    int failed=0,checks=0,destroyed=0;
    xui_context context=NULL,other=NULL;xui_code_find_scope scope=NULL,next=NULL;
    xui_widget root=NULL,a=NULL,b=NULL,c=NULL; xui_font font=NULL;xui_test_proxy_state_t state;
    xui_widget borrowed=NULL;
    activate_owner active={.refs=1},denied={.refs=1,.deny=true};
    xui_callback_owner_t owner={{&active,&activateOps},holdActivate,dropActivate};
    xui_callback_owner_t rejected={{&denied,&activateOps},holdActivate,dropActivate};
    xui_lifetime_link_t watch={0};
    xuiTestProxyInit(&state);
#if defined(XGE_XRT_HOST) && !defined(XGE_STATIC_BUILD)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    CHECK(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    xui_context_desc_t config;
    xuiContextDescDefault(&config);
    config.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    CHECK(xuiCreateEx(&config,&context)==XUI_OK && xuiSetProxy(context,&state.tProxy)==XUI_OK);
    CHECK(xuiCreateEx(&config,&other)==XUI_OK);
    CHECK(state.tProxy.fontLoadFile(&state.tProxy,&font,"find.ttf",14,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(xuiInputViewport(context,800,600)==XUI_OK && xuiWidgetCreate(context,&root)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
    CHECK(xuiWidgetSetRect(root,(xui_rect_t){0,0,800,600})==XUI_OK && editor(context,root,"alpha\n",&a)==XUI_OK && editor(context,root,"alpha alpha\n",&b)==XUI_OK);
    CHECK(xuiContextGetCodeFindScope(context,a)==NULL && xrtGetError()==NULL);
    CHECK(xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeCreate(&next)==XUI_OK);
    CHECK(xuiCodeEditSetFindScope(a,scope)==XUI_OK && xuiCodeFindScopeGetEditorCount(scope)==1);
    CHECK(xuiCodeFindScopeGetEditor(scope,0)==a && xuiContextGetCodeFindScope(context,a)==scope);
    CHECK(xuiContextGetCodeFindScope(other,a)==NULL && xrtErrorKind(xrtGetError())==XERR_ARGUMENT);
    xrtClearError();
    CHECK(xrtMemDebugFailAfter(0));
    int move=xuiCodeEditSetFindScope(a,next);
    bool allocationFailed=xrtMemDebugFailTriggered();
    xrtMemDebugFailClear();
    CHECK(move==XUI_ERROR_OUT_OF_MEMORY && allocationFailed && xrtErrorKind(xrtGetError())==XERR_MEMORY);
    xrtClearError();
    CHECK(xuiContextGetCodeFindScope(context,a)==scope && xuiCodeFindScopeGetEditorCount(next)==0);
    CHECK(xuiCodeFindScopeSetEditorName(scope,a,"one.c")==XUI_OK && xuiCodeEditSetFindScope(a,scope)==XUI_OK &&
          !strcmp(xuiCodeFindScopeGetEditorName(scope,0),"one.c"));
    CHECK(xuiCodeEditSetFindScope(a,next)==XUI_OK && xuiCodeFindScopeGetEditorCount(scope)==0 &&
          xuiCodeFindScopeGetEditor(next,0)==a && xuiCodeEditGetFindScope(a)==next);
    CHECK(xuiCodeEditSetFindScope(a,NULL)==XUI_OK && xuiCodeFindScopeGetEditorCount(next)==0 &&
          xuiContextGetCodeFindScope(context,a)==NULL && xrtGetError()==NULL);
    CHECK(xuiCodeEditSetFindScope(a,next)==XUI_OK && xuiCodeFindScopeDestroyChecked(next)==XUI_OK);
    next=NULL;
    CHECK(xuiCodeEditGetFindScope(a)==NULL && xuiCodeEditOpenFind(a)==XUI_OK);
    CHECK(xuiCodeFindScopeDestroyChecked(scope)==XUI_OK);
    scope=NULL;
    CHECK(xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeAddEditorNamed(scope,a,"one.c")==XUI_OK && xuiCodeFindScopeAddEditorNamed(scope,b,"two.c")==XUI_OK);
    CHECK(xuiCodeFindScopeGetEditorChecked(scope,context,0,&borrowed)==XUI_OK && borrowed==a);
    CHECK(xuiCodeFindScopeGetEditorChecked(scope,other,0,&borrowed)==XUI_ERROR_INVALID_ARGUMENT && !borrowed);
    CHECK(xuiCodeFindScopeGetEditorChecked(scope,context,-1,&borrowed)==XUI_OK && !borrowed);
    CHECK(xuiCodeFindScopeGetActiveEditorChecked(scope,context,&borrowed)==XUI_OK && !borrowed);
    CHECK(xuiCodeFindScopeGetFindWindowChecked(scope,context,&borrowed)==XUI_OK && !borrowed);
    CHECK(xuiCodeFindScopeGetResultEditorChecked(scope,context,0,&borrowed)==XUI_OK && !borrowed);
    {
        char text[162];
        xui_find_options_t options={0};
        xui_code_find_result_t result;
        int count=0;
        options.iSize=sizeof(options);options.sPattern="alpha";options.iRangeEnd=-1;
        memcpy(text,"alpha ",6);
        memset(text+6,'x',152);
        memcpy(text+158,"\xe4\xbd\xa0",4);
        CHECK(xuiCodeEditSetText(b,text)==XUI_OK && xuiCodeEditFindAll(b,&options,&count)==XUI_OK && count==1);
        CHECK(xuiCodeEditGetFindResult(b,0,&result)==XUI_OK && result.iStart==0 && result.iEnd==5 && strlen(result.sPreview)==158);
        CHECK(xuiCodeFindScopeFindAll(scope,&options,&count)==XUI_OK && count==2);
        CHECK(xuiCodeFindScopeGetResult(scope,1,&result)==XUI_OK && result.pEditor==b && strlen(result.sPreview)==158);
        xstrview preview={0};
        CHECK(xuiCodeFindResultGetPreviewView(&result,&preview)==XUI_OK && preview.Size==158);
        memset(result.sPreview,'x',sizeof(result.sPreview));
        CHECK(xuiCodeFindResultGetPreviewView(&result,&preview)==XUI_ERROR_INVALID_STATE && !preview.Data && !preview.Size);
        result.iSize=0;
        CHECK(xuiCodeFindResultGetPreviewView(&result,&preview)==XUI_ERROR_INVALID_ARGUMENT && !preview.Data);
        CHECK(xuiCodeFindScopeGetResultEditorChecked(scope,context,1,&borrowed)==XUI_OK && borrowed==b);
        CHECK(xuiCodeFindScopeGetResultEditorChecked(scope,other,1,&borrowed)==XUI_ERROR_INVALID_ARGUMENT && !borrowed);
        CHECK(xuiCodeEditSetText(b,"alpha alpha\n")==XUI_OK);
        CHECK(xuiCodeFindScopeGetResultEditorChecked(scope,context,1,&borrowed)==XUI_OK && !borrowed);
    }
    CHECK(xuiCodeEditOpenReplace(a)==XUI_OK);
    xui_widget window=xuiCodeFindScopeGetFindWindow(scope);
    CHECK(window && window==xuiCodeEditGetFindWindow(a) && window==xuiCodeEditGetFindWindow(b));
    CHECK(xuiCodeFindScopeGetFindWindowChecked(scope,context,&borrowed)==XUI_OK && borrowed==window);
    CHECK(xuiCodeFindScopeGetFindWindowChecked(scope,other,&borrowed)==XUI_ERROR_INVALID_ARGUMENT && !borrowed);
    CHECK(xuiCodeFindScopeGetActiveEditorChecked(scope,context,&borrowed)==XUI_OK && borrowed==a);
    CHECK(xuiCodeFindScopeGetActiveEditorChecked(scope,other,&borrowed)==XUI_ERROR_INVALID_ARGUMENT && !borrowed);
    CHECK(xuiCodeFindScopeGetActiveEditorChecked(scope,context,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    xui_widget query=control(context,window,"input",0,0),replacement=control(context,window,"input",0,1),all=control(context,window,"checkbox",XUI_TR_FIND_SCOPE,0);
    CHECK(query && replacement && all && xuiInputSetText(query,"alpha")==XUI_OK && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeEditGetFindResultCount(a)==1);
    CHECK(xuiSetFocusWidget(context,b)==XUI_OK && xuiCodeFindScopeSetActiveEditor(scope,b)==XUI_OK && xuiGetFocusWidget(context)==b && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeEditGetFindResultCount(b)==2);
    CHECK(xuiCodeFindScopeGetActiveEditor(scope)==b && !strcmp(xuiInputGetText(query),"alpha") && xuiCodeFindScopeGetFindWindow(scope)==window);
    CHECK(xuiCheckBoxSetChecked(all,1)==XUI_OK && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==3);
    CHECK(xuiCodeFindScopeSetActiveEditor(scope,NULL)==XUI_OK && xuiCodeFindScopeFindNext(scope,0)==XUI_OK && xuiCodeFindScopeGetActiveEditor(scope)==a);
    CHECK(xuiCodeFindScopeFindNext(scope,0)==XUI_OK && xuiCodeFindScopeGetActiveEditor(scope)==b);
    CHECK(xuiInputSetText(replacement,"changed")==XUI_OK && xuiCodeFindScopeSetActiveEditor(scope,b)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK && xuiUpdate(context,.1f)==XUI_OK && xuiLayout(context)==XUI_OK);
    xui_widget replaceAll=control(context,window,"button",XUI_TR_REPLACE_ALL,0);xui_rect_t rect=xuiWidgetGetWorldRect(replaceAll);float x=rect.fX+rect.fW/2,y=rect.fY+rect.fH/2;
    CHECK(replaceAll && rect.fW>0 && rect.fH>0 && xuiInputPointerDown(context,x,y,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK && xuiInputPointerUp(context,x,y,XUI_POINTER_BUTTON_LEFT,0)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK);
    CHECK(!strcmp(xuiCodeEditGetText(a),"alpha\n") && !strcmp(xuiCodeEditGetText(b),"changed changed\n"));
    CHECK(xuiCodeDocumentUndo(xuiCodeEditGetDocument(b))==XUI_OK && !strcmp(xuiCodeEditGetText(b),"alpha alpha\n"));
    CHECK(xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==3);
    CHECK(xuiCodeDocumentInsert(xuiCodeEditGetDocument(a),0,"prefix ")==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==0 && xuiCodeFindScopeActivateResult(scope,0)!=XUI_OK);
    CHECK(xuiWidgetDestroyChecked(a)==XUI_OK);a=NULL;
    CHECK(xuiCodeFindScopeGetEditorCount(scope)==1 && xuiCodeFindScopeGetFindWindow(scope)==window && xuiWindowIsOpen(window));
    CHECK(xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==2);
    CHECK(xuiCodeFindScopeSetActivateOwned(scope,activate,&active,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiCodeFindScopeSetActivateOwned(scope,activate,&active,&owner)==XUI_OK && active.refs==2);
    CHECK(xuiCodeFindScopeTraceOwnership(scope,findActivateEdge,&active));
    xthread* worker=xrtThreadCreate(checkFindThread,scope,0);
    CHECK(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);
    CHECK(xuiCodeFindScopeSetActivateOwned(scope,activate,&denied,&rejected)==XUI_ERROR_CALLBACK_FAILED && denied.refs==1 && active.refs==2);
    active.status=XUI_ERROR_CALLBACK_FAILED;
    CHECK(xuiCodeFindScopeFindNext(scope,0)==XUI_ERROR_CALLBACK_FAILED && active.calls==1 && active.refs==2);
    CHECK(xuiCodeFindScopeActivateResult(scope,0)==XUI_ERROR_CALLBACK_FAILED && active.calls==2 && active.refs==2);
    active.status=XUI_OK;active.clear=true;
    CHECK(xuiCodeFindScopeActivateResult(scope,0)==XUI_OK && active.calls==3 && active.refs==1 &&
          xuiCodeFindScopeGetResultCount(scope)==0);
    active.clear=false;
    CHECK(xuiCodeFindScopeAddEditor(scope,b)==XUI_OK && xuiCodeFindScopeRefresh(scope)==XUI_OK);
    CHECK(xuiCodeFindScopeSetActivateOwned(scope,activate,&active,&owner)==XUI_OK && active.refs==2);
    CHECK(xuiCodeFindScopeSetActivate(scope,destroyScope,&destroyed)==XUI_OK && xuiCodeFindScopeFindNext(scope,0)==XUI_OK && destroyed);scope=NULL;
    CHECK(active.refs==1 && active.calls==3);
    CHECK(xuiCodeEditGetFindScope(b)==NULL && xuiCodeEditOpenFind(b)==XUI_OK && xuiCodeEditGetFindWindow(b)!=NULL);
    CHECK(xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeAddEditor(scope,b)==XUI_OK);
    xui_find_options_t options={.iSize=sizeof(options),.sPattern="alpha",.iRangeEnd=-1};
    CHECK(xuiCodeFindScopeFindAll(scope,&options,NULL)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==2);
    CHECK(xuiCodeFindScopeLifetimeWatch(scope,&watch,scopeInvalidated,&active)==XUI_OK);
    CHECK(xuiCodeFindScopeSetActivateOwned(scope,activate,&active,&owner)==XUI_OK);
    active.close=true;active.failDrop=true;active.status=XUI_ERROR_INVALID_STATE;
    int outcome=xuiCodeFindScopeActivateResult(scope,0);
    if(active.invalidated)scope=NULL;
    CHECK(outcome==XUI_ERROR_INVALID_STATE && active.calls==4 && active.refs==1 && active.invalidated==1 &&
          xuiCodeEditGetFindScope(b)==NULL);
    CHECK(xuiWidgetDestroyChecked(b)==XUI_OK);b=NULL;
    CHECK(editor(context,root,"last\n",&c)==XUI_OK && xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeAddEditor(scope,c)==XUI_OK && xuiCodeEditOpenFind(c)==XUI_OK);
    CHECK(xuiCodeFindScopeSetActiveEditor(scope,root)==XUI_ERROR_INVALID_ARGUMENT);
    xuiDestroy(context);context=NULL;c=NULL;root=NULL;
    CHECK(xuiCodeFindScopeGetEditorCount(scope)==0 && xuiCodeFindScopeGetFindWindow(scope)==NULL);
    CHECK(xuiCodeFindScopeGetFindWindowChecked(scope,other,&borrowed)==XUI_OK && !borrowed);
    xuiCodeFindScopeDestroy(scope);scope=NULL;
    xuiDestroy(other);other=NULL;
cleanup:
    if(next)xuiCodeFindScopeDestroy(next);
    if(scope)xuiCodeFindScopeDestroy(scope);
    if(context)xuiDestroy(context);
    if(other)xuiDestroy(other);
    if(font)state.tProxy.fontDestroy(&state.tProxy,font);
    if(!failed)printf("code_find_session passed: %d native checks; shared identity, target/focus, scoped navigation, current replacement/undo, stale offsets, editor-first/context-first cleanup and reentrant scope destruction\n",checks);
    return failed;
}
