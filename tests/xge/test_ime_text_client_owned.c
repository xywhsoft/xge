/* CPU fixture for the production store and owner transitions. Build with
 * XRT_MODULE_MEMORY_DEBUG to measure warm reads and force staging failures. */
#include "single/xge.c"

#undef NDEBUG
#include <assert.h>

typedef struct TestClient {
    int refs;
    int calls;
    int failHold;
    int failSnapshot;
    int failDrop;
    int variant;
} TestClient;

static bool count(const void* raw,size_t* output)
{
    *output=(size_t)((const TestClient*)raw)->refs;
    return true;
}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    (void)raw;(void)visit;(void)user;
    return true;
}
static const xrtownershipops ownerOps={count,trace};

static bool hold(const void* raw)
{
    TestClient* client=(TestClient*)raw;
    assert(xgeImeSetTextClientOwned(NULL,NULL,NULL)==XGE_ERROR_BUSY);
    if(client->failHold)return false;
    ++client->refs;
    return true;
}

static bool drop(const void* raw)
{
    TestClient* client=(TestClient*)raw;
    assert(client->refs>1);
    --client->refs;
    assert(xgeImeSetTextClientOwned(NULL,NULL,NULL)==XGE_ERROR_BUSY);
    if(client->failDrop){
        xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"ime-drop",0,"owner drop failed"));
        return false;
    }
    return true;
}

static int snapshot(xge_ime_text_output_t* output,void* user)
{
    TestClient* client=user;
    ++client->calls;
    assert(xgeImeRefreshTextClient()==XGE_ERROR_BUSY);
    assert(xgeImeSetTextClient(NULL)==XGE_ERROR_BUSY);
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    static const char text[]={'A','\0',(char)0xc3,(char)0xa9};
    if(client->failSnapshot==1)return XGE_OK;
    if(client->failSnapshot==2)return xgeImeTextOutputSet(output,text,4,3,4,0,4,0);
    if(client->failSnapshot==3)return xgeImeTextOutputSet(output,"\xff",1,0,1,0,1,0);
    int status=xgeImeTextOutputSet(output,text,4,2,4,client->variant ? 20:10,32,47);
    if(status!=XGE_OK)return status;
    assert(xgeImeTextOutputSet(output,"",0,0,0,0,0,0)==XGE_ERROR_BUSY);
    if(client->failSnapshot==4){
        xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"ime-snapshot",1,"snapshot failed"));
        return XGE_ERROR_CALLBACK_FAILED;
    }
    return XGE_OK;
}

#if defined(_WIN32)
static int32 wrong_thread(void* unused)
{
    (void)unused;
    assert(xgeImeSetTextClientOwned(NULL,NULL,NULL)==XGE_ERROR_INVALID_STATE);
    assert(xgeImeRefreshTextClient()==XGE_ERROR_INVALID_STATE);
    return 0;
}

static int raw_snapshot(void* user,xge_ime_text_snapshot_t* output)
{
    (void)user;
    output->sText="raw";
    output->iTextSize=3;
    return XGE_OK;
}
#endif

int main(void)
{
    TestClient old={.refs=1},next={.refs=1,.variant=1};
    xge_callback_owner_t first={{&old,&ownerOps},hold,drop},second={{&next,&ownerOps},hold,drop};
#if defined(_WIN32)
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_ERROR_NOT_INITIALIZED);
#else
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_ERROR_UNSUPPORTED);
    assert(xgeImeTextOutputSet(NULL,NULL,0,0,0,0,0,0)==XGE_ERROR_UNSUPPORTED);
#endif
    assert(old.calls==0 && old.refs==1);
    xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL};
    assert(xgeInit(&desc)==XGE_OK);
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_ERROR_UNSUPPORTED);
    assert(old.calls==0 && old.refs==1);
#if defined(_WIN32)
    /* Private TSF state uses the same buffers and operations as a window.
     * No system text service or desktop interaction is needed here. */
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    __xgeTsfStoreInitialize(store);
    g_xgeWin32Ime.bTsfInitialized=1;
    old.failHold=1;
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_ERROR_INVALID_STATE);
    assert(old.calls==0 && old.refs==1);
    old.failHold=0;
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_ERROR_OUT_OF_MEMORY);
    assert(old.calls==1 && old.refs==1 && store->sText==NULL && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();xrtClearError();
    assert(xgeImeSetTextClientOwned(snapshot,&old,&first)==XGE_OK);
    assert(old.refs==2 && store->iTextLength==3 && store->sText[1]==0 && store->sText[2]==0x00e9);
    assert(store->iSelectionStart==2 && store->iSelectionEnd==3);
    assert(store->iDocumentOffset==10 && store->iDocumentSize==32 && store->iRevision==47);
    assert(xgeImeRefreshTextClient()==XGE_OK);
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i)assert(xgeImeRefreshTextClient()==XGE_OK);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount==before.AllocCount && after.FreeCount==before.FreeCount);
    assert(after.ReallocCount==before.ReallocCount && after.LiveCount==before.LiveCount);
    int calls=old.calls;
    store->bComposing=1;
    assert(xgeImeRefreshTextClient()==XGE_OK && old.calls==calls);
    store->bComposing=0;
    store->iLockType=TS_LF_READ;
    assert(xgeImeRefreshTextClient()==XGE_ERROR_BUSY && old.calls==calls);
    store->iLockType=0;
    xthread* worker=xrtThreadCreate(wrong_thread,NULL,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);

    for(int i=1;i<=3;++i){
        next.failSnapshot=i;
        assert(xgeImeSetTextClientOwned(snapshot,&next,&second)==XGE_ERROR_INVALID_ARGUMENT);
        assert(next.refs==1 && old.refs==2 && store->iDocumentOffset==10);
    }
    next.failSnapshot=4;next.failDrop=1;
    assert(xgeImeSetTextClientOwned(snapshot,&next,&second)==XGE_ERROR_CALLBACK_FAILED);
    assert(next.refs==1 && old.refs==2 && store->iDocumentOffset==10);
    xerror* error=xrtGetError();
    assert(error && strcmp(xrtErrorMessage(error),"snapshot failed")==0);
    xrtClearError();
    next.failSnapshot=0;next.failDrop=0;
    old.failDrop=1;
    assert(xgeImeSetTextClientOwned(snapshot,&next,&second)==XGE_ERROR_INVALID_STATE);
    assert(old.refs==1 && next.refs==2 && store->iDocumentOffset==20);
    xrtClearError();
    assert(xgeImeRefreshTextClient()==XGE_OK);
    xge_ime_text_client_t raw={.iSize=sizeof(raw),.onSnapshot=raw_snapshot};
    assert(xgeImeSetTextClient(&raw)==XGE_OK && next.refs==1);
    assert(store->iTextLength==3 && store->sText[0]==L'r');
    assert(xgeImeSetTextClientOwned(snapshot,&next,&second)==XGE_OK && next.refs==2);
    assert(xgeUnitChecked()==XGE_OK && next.refs==1 && g_xgeWin32Ime.sSnapshotScratch==NULL);
    __xgeTsfUnit();
#else
    (void)second;
    assert(xgeUnitChecked()==XGE_OK);
#endif
    puts("XGE_OWNED_IME_CLIENT_NATIVE_OK");
    return 0;
}
