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

#ifdef XGE_XRT_HOST
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if (!strcmp(name, #symbol)) return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

typedef struct input_owner {
    xui_context context;
    size_t refs;
    int previews;
    int commits;
    int mode;
    const xerror* failure;
} input_owner;

static bool hold(const void* raw)
{
    ++((input_owner*)raw)->refs;
    return true;
}
static bool drop(const void* raw)
{
    input_owner* owner=(input_owner*)raw;
    assert(owner->refs>1);
    --owner->refs;
    return true;
}
static bool count(const void* raw,size_t* refs)
{
    *refs=((const input_owner*)raw)->refs;
    return true;
}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    (void)raw;
    (void)visit;
    (void)user;
    return true;
}

static int respond(xui_widget editor,const xui_code_input_event_t* event,
    xui_code_input_response_t* response,void* user)
{
    input_owner* owner=user;
    assert(owner->refs==3 && event->iSource==XUI_CODE_INPUT_SOURCE_KEYBOARD && event->iTextSize==1);
    xui_call_scope_t scope={0}, rejected={0};
    assert(xuiCallScopeBeginManaged(owner->context,&scope)==XUI_OK);
    assert(xuiCallScopeBeginManaged(owner->context,&rejected)==XUI_ERROR_BUSY);
    assert(xuiCodeEditSetText(editor,"unwanted")==XUI_ERROR_BUSY);
    assert(xuiCodeInputResponseSetFlags(response,UINT32_MAX)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeInputResponseGetFlags(response)==0);
    int result=XUI_OK;
    if(event->iPhase==XUI_CODE_INPUT_PREVIEW){
        ++owner->previews;
        if(owner->mode==1){
            char replacement[]="()";
            xui_code_text_edit_t edit={.iSize=sizeof(edit),.tRange=event->tReplacedRange,.sText=replacement};
            assert(xuiCodeInputResponseSetEdits(response,&edit,1)==XUI_OK);
            replacement[0]='?';
            edit.sText="different";
            assert(xrtMemDebugFailAfter(0));
            assert(xuiCodeInputResponseSetEdits(response,&edit,1)==XUI_ERROR_OUT_OF_MEMORY);
            xrtMemDebugFailClear();
            xrtClearError();
            edit.tRange.iEnd=event->tReplacedRange.iEnd+100;
            assert(xuiCodeInputResponseSetEdits(response,&edit,1)==XUI_ERROR_INVALID_ARGUMENT);
            assert(xuiCodeInputResponseSetSelection(response,1,1)==XUI_OK);
            assert(xuiCodeInputResponseSetFlags(response,xuiCodeInputResponseGetFlags(response) |
                XUI_CODE_INPUT_ACTION_CONSUME)==XUI_OK);
        }
        if(owner->mode==2)result=XUI_ERROR_CALLBACK_FAILED;
    }else{
        assert(event->iPhase==XUI_CODE_INPUT_COMMITTED);
        ++owner->commits;
        if(owner->mode==1){
            assert(!strcmp(xuiCodeDocumentGetText(event->pDocument),"()"));
            assert(event->tSelectionAfter.iCaretOffset==1);
            xui_code_text_edit_t edit={.iSize=sizeof(edit),.tRange={1,1},.sText="Z"};
            assert(xuiCodeInputResponseSetEdits(response,&edit,1)==XUI_OK);
            assert(xuiCodeInputResponseSetSelection(response,2,2)==XUI_OK);
        }
        if(owner->mode==3)result=XUI_ERROR_CALLBACK_FAILED;
    }
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    if(result!=XUI_OK){
        xrtSetErrorInfo(XERR_VALUE,"code-input",207,"original input failure");
        owner->failure=xrtGetError();
        assert(owner->failure);
    }
    return result;
}

static int dispatch(xui_context context,xui_widget editor,uint32_t scalar)
{
    xui_event_t event={.iSize=sizeof(event),.iType=XUI_EVENT_TEXT,.pTarget=editor,.iCodepoint=scalar};
    return xuiDispatchEvent(context,&event);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    xui_context context=NULL;
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateEx(&desc,&context)==XUI_OK);
    assert(xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    xui_widget editor=NULL;
    assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);
    assert(xuiSetRootWidget(context,editor)==XUI_OK);
    assert(xuiSetFocusWidget(context,editor)==XUI_OK);
    assert(xuiWidgetSetRect(editor,(xui_rect_t){0,0,360,180})==XUI_OK);
    assert(xuiInputViewport(context,360,180)==XUI_OK);
    assert(xuiLayout(context)==XUI_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));

    input_owner state={.context=context,.refs=1,.mode=1};
    const xrtownershipops ops={count,trace};
    const xui_callback_owner_t owner={{&state,&ops},hold,drop};
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeEditSetInputHandlerOwned(editor,respond,&state,&owner)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(state.refs==1);
    assert(xuiCodeEditSetInputHandlerOwned(editor,respond,&state,&owner)==XUI_OK);
    assert(state.refs==2);
    assert(dispatch(context,editor,'(')==XUI_OK);
    assert(state.previews==1 && state.commits==1 && state.refs==2);
    xui_code_document document=xuiCodeEditGetDocument(editor);
    assert(!strcmp(xuiCodeEditGetText(editor),"(Z)"));
    assert(xuiCodeDocumentUndo(document)==XUI_OK);
    assert(!strcmp(xuiCodeEditGetText(editor),""));

    state.mode=2;
    assert(dispatch(context,editor,'q')==XUI_ERROR_CALLBACK_FAILED);
    assert(xrtGetError()==state.failure);
    xrtClearError();
    assert(state.previews==2 && state.commits==1 && !strcmp(xuiCodeEditGetText(editor),""));
    state.mode=3;
    assert(dispatch(context,editor,'v')==XUI_ERROR_CALLBACK_FAILED);
    assert(xrtGetError()==state.failure);
    xrtClearError();
    assert(state.previews==3 && state.commits==2 && !strcmp(xuiCodeEditGetText(editor),"v"));
    assert(xuiCodeDocumentUndo(document)==XUI_OK && !strcmp(xuiCodeEditGetText(editor),""));
    assert(xuiCodeEditSetInputHandler(editor,NULL,NULL)==XUI_OK && state.refs==1);
    assert(dispatch(context,editor,'w')==XUI_OK && !strcmp(xuiCodeEditGetText(editor),"w"));
    assert(state.previews==3 && state.commits==2);
    xuiDestroy(context);
    puts("code_edit_owned_input passed");
    return 0;
}