/* Real public editing commands; status and pointer lifetime after EndEdit. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xui.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Owner {size_t refs;bool fail;unsigned calls,gone;bool close;xerror* diagnostic;} Owner;
static bool count(const void* raw,size_t* output){*output=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw){++((Owner*)raw)->refs;return true;}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs>1);--owner->refs;
    if(owner->diagnostic)xrtSetErrorTake(xrtErrorRef(owner->diagnostic));
    return !owner->fail;
}
static void gone(void* raw){++((Owner*)raw)->gone;}
static void changed(xui_code_document document,xui_code_range_t range,uint32_t version,void* raw)
{
    Owner* owner=raw;++owner->calls;(void)range;(void)version;
    if(owner->close){bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed && !owner->gone);}
}
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
static int edit(xui_code_document document,xui_code_selection_model selection,int operation)
{
    if(operation==0)return xuiCodeEditingIndentSelection(document,selection,"  ",0);
    if(operation==1)return xuiCodeEditingOutdentSelection(document,selection,2,0);
    return xuiCodeEditingToggleLineComment(document,selection,"//",0);
}
static void bodyFailure(int operation)
{
    unsigned earlierFailures=0;
    for(int allowance=0;allowance<40;++allowance){
        xui_code_document document=NULL;xui_code_selection_model selection=NULL;
        assert(xuiCodeDocumentCreate(&document)==XUI_OK && xuiCodeSelectionCreate(&selection)==XUI_OK);
        assert(xuiCodeDocumentSetText(document,operation==1 ? "  a\n  b":"a\nb")==XUI_OK);
        assert(xuiCodeSelectionSelectAll(selection,document)==XUI_OK);
        Owner owner={.refs=1,.fail=true};
        owner.diagnostic=xrtErrorCreate(XERR_STATE,"editing-cleanup",80,"later EndEdit failure");assert(owner.diagnostic);
        xui_callback_owner_t binding={{&owner,&ops},hold,drop};
        assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&owner,&binding)==XUI_OK);
        assert(xrtMemDebugFailAfter(allowance));
        int status=edit(document,selection,operation);xrtMemDebugFailClear();
        if(status==XUI_ERROR_OUT_OF_MEMORY){
            /* A partially committed first line causes EndEdit to notify even
             * though a later native allocation failed. Preserve that earlier
             * failure, including a NULL diagnostic, ahead of owner's Drop. */
            assert(xrtGetError()!=owner.diagnostic);
            if(owner.calls)++earlierFailures;
        }else assert(status==XUI_ERROR_CALLBACK_FAILED && owner.calls==1 && xrtGetError()==owner.diagnostic);
        xrtClearError();xrtErrorFree(owner.diagnostic);owner.diagnostic=NULL;owner.fail=false;
        assert(xuiCodeDocumentSetChangeCallbackOwned(document,NULL,NULL,NULL)==XUI_OK && owner.refs==1);
        bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed);
        xuiCodeSelectionDestroy(selection);
    }
    assert(earlierFailures>0);
}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=4);
#if defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled()||xrtMemDebugEnable(true));
    if(mode==4){
        for(int operation=0;operation<3;++operation)bodyFailure(operation);
        puts("Editing outcome: earlier body/OOM status and diagnostic precede EndEdit callback failure");return 0;
    }
    for(int operation=0;operation<3;++operation){
        xui_code_document document=NULL;xui_code_selection_model selection=NULL;
        assert(xuiCodeDocumentCreate(&document)==XUI_OK && xuiCodeSelectionCreate(&selection)==XUI_OK);
        assert(xuiCodeDocumentSetText(document,operation==1 ? "  a\n  b":"a\nb")==XUI_OK);
        assert(xuiCodeSelectionSelectAll(selection,document)==XUI_OK);
        xui_code_selection_t before={0};assert(xuiCodeSelectionGetState(selection,&before)==XUI_OK);
        Owner owner={.refs=1,.fail=mode==0 || mode==1,.close=mode==2};
        if(mode==1){owner.diagnostic=xrtErrorCreate(XERR_STATE,"editing-owner",79,"EndEdit owner failed");assert(owner.diagnostic);}
        xui_callback_owner_t binding={{&owner,&ops},hold,drop};xui_lifetime_link_t watch={0};
        assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&owner,&binding)==XUI_OK);
        assert(xuiCodeDocumentLifetimeWatch(document,&watch,gone,&owner)==XUI_OK);
        int status=edit(document,selection,operation);
        int expected=mode<=1 ? XUI_ERROR_CALLBACK_FAILED:mode==2 ? XUI_ERROR_INVALID_STATE:XUI_OK;
        if(status!=expected)fprintf(stderr,"Editing operation %d mode %d: status=%d expected=%d\n",operation,mode,status,expected);
        assert(status==expected && owner.calls==1);
        if(mode==1)assert(xrtGetError()==owner.diagnostic);
        else assert(!xrtGetError());
        xrtClearError();owner.fail=false;owner.close=false;xrtErrorFree(owner.diagnostic);owner.diagnostic=NULL;
        if(mode==2)assert(owner.refs==1 && owner.gone==1 && !watch.ppPrevious);
        else {
            xstrview text={0};assert(xuiCodeDocumentGetTextView(document,&text)==XUI_OK);
            const char* expectedText=operation==0 ? "  a\n  b":operation==1 ? "a\nb":"//a\n//b";
            assert(text.Size==strlen(expectedText) && !memcmp(text.Data,expectedText,text.Size));
            if(mode<=1){xui_code_selection_t after={0};assert(xuiCodeSelectionGetState(selection,&after)==XUI_OK);
                assert(after.iAnchorOffset==before.iAnchorOffset && after.iCaretOffset==before.iCaretOffset);}
            assert(xuiCodeDocumentCanUndo(document));
            assert(xuiCodeDocumentSetChangeCallbackOwned(document,NULL,NULL,NULL)==XUI_OK);
            bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed && owner.gone==1 && owner.refs==1);
        }
        xuiCodeSelectionDestroy(selection);
    }
    printf("Editing outcome mode %d: three EndEdit outcomes preserve status, diagnostics and document lifetime\n",mode);
    return 0;
}
