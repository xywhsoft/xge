/* Exact Doc View/Editor/Message consumers, CPU fake font/render only.
 * No engine/window/browser/GPU/network initialization. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#include "../../src/xui/document/xui_document_view.c"
#include "../../src/xui/document/xui_document_edit_adapter.c"
#include "../../src/xui/document/xui_document_editor.c"
#include "../../src/xui/controls/xui_message_list.c"

static const char resourceName[]="doc-owned-resource";
typedef struct DocUiTest {LayoutProvider base;xui_font font;xui_document document;xui_doc_node_id link,task;xui_widget widget;int kind;} DocUiTest;
static void docUiSetup(DocUiTest* t,int kind)
{
    memset(t,0,sizeof(*t));t->kind=kind;layoutSetup(&t->base,&t->font);
    assert(xuiDocumentCreate(NULL,&t->document)==XUI_OK);xui_document_transaction txn=NULL;
    assert(xuiDocumentBeginTransaction(t->document,NULL,&txn)==XUI_OK);
    xui_doc_node_id para=0; xui_doc_node_desc_t desc={.iSize=sizeof(desc),.iKind=XUI_DOC_PARAGRAPH};
    assert(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&desc,&para)==XUI_OK);
    desc=(xui_doc_node_desc_t){.iSize=sizeof(desc),.iKind=XUI_DOC_TEXT,.sText="click",.iTextBytes=5,.sResource=resourceName};desc.tAttributes.iMarks=XUI_DOC_LINK;
    assert(xuiDocumentTxnInsertNode(txn,para,XUI_DOCUMENT_APPEND,&desc,&t->link)==XUI_OK);
    xui_doc_node_id list=0;desc=(xui_doc_node_desc_t){.iSize=sizeof(desc),.iKind=XUI_DOC_LIST};
    assert(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&desc,&list)==XUI_OK);
    desc=(xui_doc_node_desc_t){.iSize=sizeof(desc),.iKind=XUI_DOC_LIST_ITEM};desc.tAttributes.iFlags=XUI_DOC_TASK;
    assert(xuiDocumentTxnInsertNode(txn,list,XUI_DOCUMENT_APPEND,&desc,&t->task)==XUI_OK && xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    xui_doc_font_set_t fonts={t->font,t->font,t->font,t->font,t->font};
    xui_doc_view_desc_t view={.iSize=sizeof(view),.pDocument=t->document,.tRenderer={.iSize=sizeof(xui_doc_renderer_desc_t),.tFonts=fonts}};
    if(kind==0)assert(xuiDocumentViewCreate(t->base.context,&view,&t->widget)==XUI_OK);
    else if(kind==1){xui_doc_editor_desc_t ed={.iSize=sizeof(ed),.tView=view,.iMode=XUI_DOC_VISUAL,.iAsyncSourceThresholdBytes=UINT64_MAX};assert(xuiDocumentEditorCreate(t->base.context,&ed,&t->widget)==XUI_OK);}
    else{
        xui_message_node_t node={.iSize=sizeof(node),.sId="message",.sText="fallback",.iType=XUI_MESSAGE_NODE_OTHER};
        xui_message_list_desc_t listDesc={.iSize=sizeof(listDesc),.pFont=t->font,.arrNodes=&node,.iNodeCount=1};
        assert(xuiMessageListCreate(t->base.context,&t->widget,&listDesc)==XUI_OK);
        xui_message_document_desc_t binding={.iSize=sizeof(binding),.pDocument=t->document,.tRenderer=view.tRenderer};
        assert(xuiMessageListSetNodeDocument(t->widget,"message",&binding)==XUI_OK);
    }
    assert(xuiWidgetSetRect(t->widget,(xui_rect_t){0,0,600,300})==XUI_OK);
}
static void docUiFinish(DocUiTest* t)
{
    assert(xuiDestroyChecked(t->base.context)==XUI_OK);xuiDocumentRelease(t->document);
    assert(xuiFontReleaseChecked(t->font)==XUI_OK && xuiProxyReleaseChecked(t->base.proxy)==XUI_OK && drop(&t->base.owner));
}
typedef struct DocUiOwner {
    size_t refs;unsigned holds,drops,calls;
    DocUiTest* test;struct DocUiOwner* peer;struct DocUiOwner* installOnHold;
    const xstrview* current;
    bool inside,clear,closeWidget,closeContext,replaceBinding;
    bool requestOnHold,failHold,failDrop,secondary,reenter,reenterDrop,primary,expectClearFailure,freshHold;
} DocUiOwner;
static xui_message_document_binding_t* docUiBinding(DocUiTest* t)
{return __xuiMessageListGetData(t->widget)->arrNodes[0].pDocumentBinding;}
static void docUiActivate(xui_widget,xui_doc_node_id,const xstrview*,void*);
static void docUiError(xui_widget,int,void*);
static void docUiToggle(xui_widget,xui_doc_node_id,int,void*);
static xui_callback_owner_t docUiRef(DocUiOwner*);
static int docUiSet(DocUiOwner* owner,bool clear)
{
    xui_callback_owner_t ref=docUiRef(owner);DocUiTest* t=owner->test;
    if(t->kind==0)return xuiDocumentViewSetActivateOwned(t->widget,clear ? NULL:docUiActivate,clear ? NULL:owner,clear ? NULL:&ref);
    if(t->kind==1)return xuiDocumentEditorSetErrorOwned(t->widget,clear ? NULL:docUiError,clear ? NULL:owner,clear ? NULL:&ref);
    if(t->kind==2)return xuiMessageListSetNodeDocumentActivateOwned(t->widget,"message",clear ? NULL:docUiActivate,clear ? NULL:owner,clear ? NULL:&ref);
    return xuiMessageListSetNodeDocumentTaskToggleOwned(t->widget,"message",clear ? NULL:docUiToggle,clear ? NULL:owner,clear ? NULL:&ref);
}
static bool docUiHold(const void* raw)
{
    DocUiOwner* owner=(DocUiOwner*)raw;assert(owner->refs);++owner->holds;
    if(owner->failHold)return false;
    ++owner->refs;
    if(owner->requestOnHold){owner->requestOnHold=false;assert(xuiRequestClose(owner->test->base.context)==XUI_OK);}
    if(owner->reenter){owner->reenter=false;int status=docUiSet(owner,true);assert(status==XUI_OK || (owner->test->kind>=2 && status==XUI_ERROR_BUSY));}
    if(owner->installOnHold){DocUiOwner* replacement=owner->installOnHold;owner->installOnHold=NULL;assert(docUiSet(replacement,false)==XUI_OK);}
    if(owner->freshHold)xrtSetErrorKind(XERR_RANGE);
    return true;
}
static bool docUiDrop(const void* raw)
{
    DocUiOwner* owner=(DocUiOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    if(owner->current){assert(owner->current->Size==sizeof(resourceName)-1 && !memcmp(owner->current->Data,resourceName,sizeof(resourceName)-1));if(!owner->inside)owner->current=NULL;}
    if(owner->reenterDrop){owner->reenterDrop=false;int status=docUiSet(owner,true);assert(status==XUI_OK || status==XUI_ERROR_INVALID_ARGUMENT || status==XUI_ERROR_INVALID_STATE);}
    bool failed=owner->failDrop;owner->failDrop=false;if(failed && owner->secondary)xrtSetErrorKind(XERR_STATE);return !failed;
}
static bool docUiCount(const void* raw,size_t* count){*count=((const DocUiOwner*)raw)->refs;return *count!=0;}
static bool docUiTrace(const void* raw,xrtownershipvisitor visit,void* user)
{const DocUiOwner* owner=raw;if(!owner->peer)return true;xui_callback_owner_t ref=docUiRef(owner->peer);return visit(ref.tReference,user);}
static const xrtownershipops docUiOps={docUiCount,docUiTrace};
static xui_callback_owner_t docUiRef(DocUiOwner* owner){return(xui_callback_owner_t){{owner,&docUiOps},docUiHold,docUiDrop};}
static void docUiCallback(DocUiOwner* owner)
{
    assert(owner->refs>=2);++owner->calls;
    if(owner->primary)xrtSetErrorKind(XERR_RANGE);
    if(owner->clear){int status=docUiSet(owner,true);assert(status==(owner->expectClearFailure ? XUI_ERROR_CALLBACK_FAILED:XUI_OK));}
    if(owner->replaceBinding){
        owner->replaceBinding=false;
        int status=xuiMessageListSetNodeDocument(owner->test->widget,"message",NULL);assert(status==XUI_ERROR_BUSY);
        xui_message_list_data_t* data=__xuiMessageListGetData(owner->test->widget);assert(data && data->arrNodes[0].pWorkBinding && data->arrNodes[0].pWorkBinding->iCallbackActivity);
    }
    if(owner->closeWidget)assert(xuiWidgetDestroyChecked(owner->test->widget)==XUI_OK);
    if(owner->closeContext)assert(xuiRequestClose(owner->test->base.context)==XUI_OK);
}
static void docUiActivate(xui_widget widget,xui_doc_node_id id,const xstrview* text,void* raw)
{DocUiOwner* owner=raw;assert(widget==owner->test->widget && id==owner->test->link);owner->current=text;owner->inside=true;docUiCallback(owner);assert(text->Data[0]=='d');owner->inside=false;}
static void docUiError(xui_widget widget,int error,void* raw)
{DocUiOwner* owner=raw;assert(widget==owner->test->widget && error==XUI_ERROR_UNSUPPORTED);docUiCallback(owner);}
static void docUiToggle(xui_widget widget,xui_doc_node_id id,int checked,void* raw)
{DocUiOwner* owner=raw;assert(widget==owner->test->widget && id==owner->test->task && checked);docUiCallback(owner);}
static int docUiSend(DocUiTest* t)
{
    if(t->kind==0)return doc_view_accessible_action(t->widget,t->link,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL,doc_view_get(t->widget));
    if(t->kind==1){doc_editor_emit_error(t->widget,doc_editor_get(t->widget),XUI_ERROR_UNSUPPORTED);return xrtGetError() ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;}
    xui_message_document_binding_t* binding=docUiBinding(t);xui_document_snapshot snapshot=NULL;assert(xuiDocumentAcquireSnapshot(t->document,&snapshot)==XUI_OK);
    int status=t->kind==2 ? __xuiMessageBindingActivate(binding,snapshot,t->link):__xuiMessageBindingToggle(binding,t->task,1);
    xuiDocumentSnapshotRelease(snapshot);return status;
}
static void ownedLifetime(int kind,int action)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t,.clear=action==0,.closeWidget=action==1,.closeContext=action==2,.replaceBinding=action==3};
    assert(docUiSet(&owner,false)==XUI_OK && owner.refs==2);
    int status=docUiSend(&t);assert(status==XUI_OK || status==XUI_ERROR_INVALID_STATE);assert(owner.calls==1 && !owner.current);
    if(action==0 || action==1)assert(owner.refs==1);
    if(action==2)assert(owner.refs==2);
    docUiFinish(&t);assert(owner.refs==1 && docUiDrop(&owner));
}
static void publisherFailures(int kind)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t};
    owner.failHold=true;assert(docUiSet(&owner,false)==XUI_ERROR_CALLBACK_FAILED && owner.refs==1);owner.failHold=false;
    owner.requestOnHold=true;assert(docUiSet(&owner,false)==XUI_ERROR_INVALID_STATE && owner.refs==1);docUiFinish(&t);assert(docUiDrop(&owner));
}
static void cleanupNull(int kind)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t};assert(docUiSet(&owner,false)==XUI_OK);
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);owner.failDrop=true;
    int status=docUiSend(&t);assert(status==XUI_ERROR_CALLBACK_FAILED || (kind==1 && status==XUI_OK));
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());docUiFinish(&t);assert(owner.refs==1 && docUiDrop(&owner));
}
static DocUiOwner* targetOwner;static unsigned tracedOwner;
static bool docUiVisit(xrtownershipref ref,void* raw)
{(void)raw;if(ref.Data==targetOwner && ref.Ops==&docUiOps)++tracedOwner;return true;}
static void twoContexts(int kind)
{
    DocUiTest a,b;docUiSetup(&a,kind);docUiSetup(&b,kind);DocUiOwner x={.refs=1,.test=&a},y={.refs=1,.test=&b};
    x.peer=&y;y.peer=&x;assert(docUiHold(&x) && docUiHold(&y));
    assert(docUiSet(&x,false)==XUI_OK && docUiSet(&y,false)==XUI_OK);
    xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));targetOwner=&x;tracedOwner=0;
    assert(xuiContextTraceOwnership(a.base.context,docUiVisit,NULL) && tracedOwner==1);tracedOwner=0;assert(xuiContextTraceOwnership(b.base.context,docUiVisit,NULL) && !tracedOwner);
    targetOwner=&y;tracedOwner=0;assert(docUiTrace(&x,docUiVisit,NULL) && tracedOwner==1);assert(xrtOwnershipScopeEnd(&frozen));
    docUiFinish(&a);assert(x.refs==2 && y.refs==3 && docUiSend(&b)==XUI_OK && y.calls==1);docUiFinish(&b);
    x.peer=y.peer=NULL;assert(docUiDrop(&x) && docUiDrop(&y) && docUiDrop(&x) && docUiDrop(&y));
}
static void reentrantOwners(int kind)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t,.reenter=true};
    assert(docUiSet(&owner,false)==XUI_OK && owner.refs==2 && !owner.reenter);
    owner.reenter=true;assert(docUiSend(&t)==XUI_OK && owner.calls==1 && owner.refs==1 && !owner.current);
    assert(docUiSet(&owner,false)==XUI_OK && owner.refs==2);
    owner.reenterDrop=true;assert(docUiSet(&owner,false)==XUI_OK && owner.refs==1 && !owner.reenterDrop);
    int emptyStatus=docUiSend(&t);assert((emptyStatus==XUI_OK || (kind==0 && emptyStatus==XUI_ERROR_UNSUPPORTED)) && owner.calls==1);docUiFinish(&t);assert(docUiDrop(&owner));
}
static void primaryAndSecondary(int kind)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t,.clear=true,.primary=true,.expectClearFailure=true,.secondary=true};
    assert(docUiSet(&owner,false)==XUI_OK);owner.failDrop=true;xui_call_scope_t outer={0};assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);
    assert(docUiSend(&t)==XUI_ERROR_CALLBACK_FAILED && owner.refs==1 && xrtErrorKind(xrtGetError())==XERR_RANGE);
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);xrtClearError();docUiFinish(&t);assert(docUiDrop(&owner));
    docUiSetup(&t,kind);owner=(DocUiOwner){.refs=1,.test=&t,.requestOnHold=true,.failDrop=true,.secondary=true};assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);
    assert(docUiSet(&owner,false)==XUI_ERROR_INVALID_STATE && owner.refs==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());docUiFinish(&t);assert(docUiDrop(&owner));
}
static void setterOom(int kind)
{
    unsigned successes=0;
    for(int budget=0;budget<9;++budget){
        DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t};xui_call_scope_t outer={0};assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);
        assert(xrtMemDebugFailAfter(budget));int status=docUiSet(&owner,false);xrtMemDebugFailClear();xrtClearError();
        if(status==XUI_OK){++successes;assert(owner.refs==2);}else assert(owner.refs==1 && status==XUI_ERROR_OUT_OF_MEMORY);
        assert(xuiCallScopeEnd(&outer)==XUI_OK);if(status==XUI_OK)assert(docUiSend(&t)==XUI_OK && owner.calls==1);
        docUiFinish(&t);assert(owner.refs==1 && docUiDrop(&owner));
    }
    assert(successes);
}
static void freshHoldRejected(int kind,bool replace)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner old={.refs=1,.test=&t},peer={.refs=1,.test=&t},candidate={.refs=1,.test=&t,.freshHold=true};
    assert(docUiSet(&old,false)==XUI_OK);if(replace)candidate.installOnHold=&peer;
    assert(docUiSet(&candidate,false)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE && candidate.refs==1 && candidate.drops==1);
    xrtClearError();assert(old.refs==(replace ? 1u:2u) && peer.refs==(replace ? 2u:1u));
    assert(docUiSend(&t)==XUI_OK && old.calls==(replace ? 0u:1u) && peer.calls==(replace ? 1u:0u) && !candidate.calls);
    docUiFinish(&t);assert(old.refs==1 && peer.refs==1 && docUiDrop(&old) && docUiDrop(&peer) && docUiDrop(&candidate));
}
static void snapshotHoldOutcomes(int kind)
{
    DocUiTest t;docUiSetup(&t,kind);DocUiOwner owner={.refs=1,.test=&t};assert(docUiSet(&owner,false)==XUI_OK);
    xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();int status=docUiSend(&t);
    assert(status==(kind==1 ? XUI_ERROR_CALLBACK_FAILED:XUI_OK) && owner.calls==1 && xrtGetError()==ambient);xrtClearError();
    owner.freshHold=owner.failDrop=owner.secondary=true;xui_call_scope_t outer={0};assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);
    unsigned drops=owner.drops;assert(docUiSend(&t)==XUI_ERROR_CALLBACK_FAILED && owner.calls==1 && owner.refs==2 && owner.drops==drops+1);
    assert(xrtErrorKind(xrtGetError())==XERR_RANGE && xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && xrtErrorKind(xrtGetError())==XERR_RANGE);xrtClearError();
    owner.freshHold=owner.secondary=false;owner.failHold=true;drops=owner.drops;
    assert(xuiCallScopeBegin(t.base.context,&outer)==XUI_OK);status=docUiSend(&t);
    assert(status==(kind==1 ? XUI_OK:XUI_ERROR_CALLBACK_FAILED) && owner.calls==1 && owner.drops==drops && owner.refs==2 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==(kind==1 ? XUI_ERROR_CALLBACK_FAILED:XUI_OK) && !xrtGetError());owner.failHold=false;
    docUiFinish(&t);assert(owner.refs==1 && docUiDrop(&owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int kind=0;kind<4;++kind){ownedLifetime(kind,0);ownedLifetime(kind,1);ownedLifetime(kind,2);if(kind>=2)ownedLifetime(kind,3);publisherFailures(kind);cleanupNull(kind);twoContexts(kind);reentrantOwners(kind);primaryAndSecondary(kind);setterOom(kind);freshHoldRejected(kind,false);freshHoldRejected(kind,true);snapshotHoldOutcomes(kind);}
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("R13 four actual Doc UI Owned families: consumer snapshots / Body pins / close / first NULL / independent Context edges balanced");return 0;
}
