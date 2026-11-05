/* Native unique CodeDocument lifecycle/observer kernel. No UI device/window. */
#define main xui_unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#include "src/xui/code/xui_code_document_internal.h"
typedef struct DocLife {
    xui_code_document document;
    xui_context context;
    xui_widget owner;
    unsigned prepare,edited,invalidated,ownerGone;
    int failure;
    bool closeOwner,closeContext;
    xui_lifetime_link_t documentWatch,widgetWatch;
} DocLife;
static void documentGone(void* raw)
{DocLife* state=raw;++state->invalidated;state->document=NULL;}
static void documentOwnerGone(void* raw)
{
    DocLife* state=raw;++state->ownerGone;
    assert(xuiInternalCodeDocumentDestroyOwned(state->owner,state->document)==XUI_OK);
}
static int prepareDocument(xui_code_document document,int start,int end,const char* text,int bytes,bool reset,void* raw)
{
    (void)start;(void)end;(void)text;(void)bytes;(void)reset;DocLife* state=raw;++state->prepare;
    assert(xuiCodeDocumentOwnershipBusy(document));
    assert(xuiCodeDocumentInsert(document,0,"nested")==XUI_ERROR_BUSY);
    if(state->closeOwner)assert(xuiWidgetDestroyChecked(state->owner)==XUI_OK);
    if(state->closeContext)assert(xuiDestroyChecked(state->context)==XUI_OK);
    xrtClearError();return state->failure;
}
static void editedDocument(xui_code_document document,int start,int end,int newEnd,int firstLine,int lastLine,int newLastLine,int reset,void* raw)
{
    (void)document;(void)start;(void)end;(void)newEnd;(void)firstLine;(void)lastLine;(void)newLastLine;(void)reset;
    ++((DocLife*)raw)->edited;
}
static void codeDocumentPrepareAndHistory(void)
{
    DocLife state={0};assert(xuiCreate(&state.context)==XUI_OK && xuiCodeDocumentCreate(&state.document)==XUI_OK);
    assert(xuiCodeDocumentSetText(state.document,"abcd")==XUI_OK);
    assert(xuiCodeDocumentLifetimeWatch(state.document,&state.documentWatch,documentGone,&state)==XUI_OK);
    assert(xuiInternalCodeDocumentSetEditObserverChecked(state.document,state.context,prepareDocument,editedDocument,&state)==XUI_OK);
    state.failure=XUI_ERROR_BACKEND_FAILED;
    assert(xuiCodeDocumentReplace(state.document,1,2,"X")==XUI_ERROR_BACKEND_FAILED && !strcmp(xuiCodeDocumentGetText(state.document),"abcd") && !state.edited && xrtGetError()==NULL);
    state.failure=XUI_OK;assert(xuiCodeDocumentReplace(state.document,1,2,"X")==XUI_OK && !strcmp(xuiCodeDocumentGetText(state.document),"aXcd") && state.edited==1);
    state.failure=XUI_ERROR_BUSY;assert(xuiCodeDocumentUndo(state.document)==XUI_ERROR_BUSY && !strcmp(xuiCodeDocumentGetText(state.document),"aXcd"));
    state.failure=XUI_OK;assert(xuiCodeDocumentUndo(state.document)==XUI_OK && !strcmp(xuiCodeDocumentGetText(state.document),"abcd"));
    assert(xuiCodeDocumentRedo(state.document)==XUI_OK && !strcmp(xuiCodeDocumentGetText(state.document),"aXcd"));
    assert(xuiDestroyChecked(state.context)==XUI_OK); /* weak observer link expires */
    assert(xuiCodeDocumentInsert(state.document,0,"Q")==XUI_OK && !strcmp(xuiCodeDocumentGetText(state.document),"QaXcd"));
    assert(xuiCodeDocumentDestroyChecked(state.document)==XUI_OK && state.invalidated==1);
}
static void codeDocumentOwnerCloseInPrepare(void)
{
    DocLife state={0};assert(xuiCreate(&state.context)==XUI_OK && xuiWidgetCreate(state.context,&state.owner)==XUI_OK);
    assert(xuiCodeDocumentCreate(&state.document)==XUI_OK && xuiCodeDocumentSetText(state.document,"old")==XUI_OK);
    assert(xuiInternalCodeDocumentClaimOwner(state.document,state.owner)==XUI_OK);
    bool consumed=true;assert(xuiCodeDocumentDestroyOwnerChecked(state.document,&consumed)==XUI_ERROR_INVALID_STATE && !consumed);
    assert(xuiCodeDocumentLifetimeWatch(state.document,&state.documentWatch,documentGone,&state)==XUI_OK);
    assert(xuiWidgetLifetimeWatch(state.owner,&state.widgetWatch,documentOwnerGone,&state)==XUI_OK);
    assert(xuiInternalCodeDocumentSetEditObserverChecked(state.document,state.context,prepareDocument,editedDocument,&state)==XUI_OK);
    state.closeOwner=true;
    assert(xuiCodeDocumentReplace(state.document,0,3,"new")==XUI_ERROR_INVALID_STATE);
    assert(state.ownerGone==1 && state.invalidated==1 && !state.document && !state.edited);
    assert(xuiDestroyChecked(state.context)==XUI_OK);
}
static void codeDocumentCloseDuringChange(xui_code_document document,xui_code_range_t range,uint32 version,void* raw)
{
    (void)range;(void)version;DocLife* state=raw;bool consumed=false;
    assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed && !state->invalidated);
}
static void codeDocumentDestroyInsideNotification(void)
{
    DocLife state={0};assert(xuiCodeDocumentCreate(&state.document)==XUI_OK);
    assert(xuiCodeDocumentLifetimeWatch(state.document,&state.documentWatch,documentGone,&state)==XUI_OK);
    assert(xuiCodeDocumentSetChangeCallback(state.document,codeDocumentCloseDuringChange,&state)==XUI_OK);
    assert(xuiCodeDocumentInsert(state.document,0,"x")==XUI_ERROR_INVALID_STATE && state.invalidated==1 && !state.document);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    codeDocumentPrepareAndHistory();codeDocumentOwnerCloseInPrepare();codeDocumentDestroyInsideNotification();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("CodeDocument native kernel: unique owner guard, stable observer, pre-edit rejection, undo/redo, weak Context and active deferred physical destruction passed");return 0;
}
