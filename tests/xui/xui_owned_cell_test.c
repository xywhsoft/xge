/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct HostEntry {const char* name;xge_host_xrt_proc address;} HostEntry;
static const HostEntry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}return NULL;}

typedef struct CellOwner {
    size_t refs,holds,drops;
    int action;
    bool failHold,failDrop;
    bool* consumed;
    xui_table_view_cell_output_t* cell;
    xui_table_view_cell_output_t* cache;
} CellOwner;
static bool cell_hold(const void* raw)
{
    CellOwner* o=(CellOwner*)raw;assert(o->refs);++o->holds;
    if(o->failHold)return false;
    ++o->refs;
    int action=o->action;o->action=0;
    if(action==1)assert(xuiTableViewCellOutputReset(o->cell)==XUI_OK);
    if(action==2)assert(xuiTableViewCellOutputUse(o->cell,o->cache)==XUI_OK);
    if(action==3)assert(xuiTableViewCellOutputReleaseChecked(o->cell)==XUI_OK);
    return true;
}
static bool cell_drop(const void* raw)
{
    CellOwner* o=(CellOwner*)raw;assert(o->refs);if(o->consumed)assert(*o->consumed);++o->drops;
    if(!--o->refs && o->cell)assert(xuiTableViewCellOutputReleaseChecked(o->cell)==XUI_OK);
    return !o->failDrop;
}
static bool cell_count(const void* raw,size_t* count){*count=((const CellOwner*)raw)->refs;return *count!=0;}
static bool cell_trace(const void* raw,xrtownershipvisitor visit,void* user)
{const CellOwner* o=raw;return !o->cell || visit(xuiTableViewCellOutputOwnership(o->cell),user);}
static const xrtownershipops cell_ops={cell_count,cell_trace};
static xui_callback_owner_t cell_ref(CellOwner* o){return (xui_callback_owner_t){{o,&cell_ops},cell_hold,cell_drop};}
static xstrview cell_text(xui_widget widget,int row,int col,const xui_table_view_cell_output_t* output,void* user)
{(void)widget;(void)row;(void)col;(void)output;CellOwner* o=user;assert(o->refs);char* s=xrtMalloc(4);if(s)memcpy(s,"abc",4);return (xstrview){s,s ? 3u:0u};}
static int cell_render(xui_widget widget,int row,int col,const xui_table_view_cell_output_t* output,xui_draw_context draw,xui_rect_t rect,int state,void* user)
{(void)widget;(void)row;(void)col;(void)output;(void)draw;(void)rect;(void)state;assert(((CellOwner*)user)->refs);return 0;}
static bool visit_count(xrtownershipref ref,void* user){assert(ref.Data && ref.Ops);++*(unsigned*)user;return true;}
static void invalidated(void* raw){++*(unsigned*)raw;}
static void simple_values(void)
{
    xui_table_view_cell_output_t cell;xuiTableViewCellOutputInit(&cell);assert(cell.iRowSpan==1 && cell.iColSpan==1);
    assert(xuiTableViewCellOutputAddRef(&cell)==XUI_ERROR_INVALID_ARGUMENT && !xuiTableViewCellOutputOwnership(&cell).Data);
    assert(xuiTableViewCellOutputSetText(&cell,(xstrview){"abcdef",6})==XUI_OK);
    xstrview old=xuiTableViewCellOutputGetText(&cell);
    assert(xrtMemDebugFailAfter(0));assert(xuiTableViewCellOutputSetText(&cell,(xstrview){"new",3})==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(xuiTableViewCellOutputGetText(&cell).Data==old.Data);
    assert(xuiTableViewCellOutputSetText(&cell,(xstrview){old.Data+1,3})==XUI_OK && !strcmp(xuiTableViewCellOutputGetText(&cell).Data,"bcd"));
    assert(xuiTableViewCellOutputSetText(&cell,(xstrview){"a\0b",3})==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTableViewCellOutputSetText(&cell,(xstrview){"a",SIZE_MAX})==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTableViewCellOutputSetTooltip(&cell,(xstrview){"tip",3})==XUI_OK);
    xvalue* v=xrtValueInt(77);assert(v && xuiTableViewCellOutputSetValue(&cell,v)==XUI_OK);xrtValueRelease(v);
    int64 n=0;assert(xrtValueGetInt(xuiTableViewCellOutputGetValue(&cell),&n) && n==77);
    xui_table_cell_scope_t scope={0};assert(xuiTableViewCellOutputCallScopeBegin(&cell,&scope)==XUI_OK && xuiTableViewCellOutputBusy(&cell));
    assert(xuiTableViewCellOutputDisposeChecked(&cell)==XUI_ERROR_BUSY);assert(xuiTableViewCellOutputCallScopeEnd(&scope)==XUI_OK);assert(xuiTableViewCellOutputCallScopeEnd(&scope)==XUI_OK);
    assert(xuiTableViewCellOutputDisposeChecked(&cell)==XUI_OK && xuiTableViewCellOutputDisposeChecked(&cell)==XUI_OK);
}
static void cache_pin(void)
{
    xui_table_view_cell_output_t* a=NULL;assert(xuiTableViewCellOutputCreate(&a)==XUI_OK);a->iType=5;
    assert(xuiTableViewCellOutputSetText(a,(xstrview){"cached",6})==XUI_OK);
    unsigned invalid=0;xui_lifetime_link_t watch={0};assert(xuiTableViewCellOutputLifetimeWatch(a,&watch,invalidated,&invalid)==XUI_OK);
    xui_table_view_cell_output_t x;xuiTableViewCellOutputInit(&x);
    assert(xuiTableViewCellOutputUse(&x,a)==XUI_OK && x.iType==5);
    assert(xuiTableViewCellOutputGetText(&x).Data==xuiTableViewCellOutputGetText(a).Data);
    assert(!xuiTableViewCellOutputBusy(a)); /* A persistent Use edge is not active execution. */
    assert(xuiTableViewCellOutputSetText(a,(xstrview){"bad",3})==XUI_ERROR_BUSY);
    assert(xuiTableViewCellOutputReset(a)==XUI_ERROR_BUSY);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){assert(xuiTableViewCellOutputReset(&x)==XUI_OK);assert(xuiTableViewCellOutputUse(&x,a)==XUI_OK);}
    xrtMemDebugSnapshot(&after);assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiTableViewCellOutputReleaseChecked(a)==XUI_OK && !invalid);
    assert(!strcmp(xuiTableViewCellOutputGetText(&x).Data,"cached"));
    assert(xuiTableViewCellOutputDisposeChecked(&x)==XUI_OK && invalid==1 && !watch.ppPrevious);
    xui_table_view_cell_output_t automatic;xuiTableViewCellOutputInit(&automatic);xuiTableViewCellOutputInit(&x);
    assert(xuiTableViewCellOutputUse(&x,&automatic)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTableViewCellOutputDisposeChecked(&x)==XUI_OK && xuiTableViewCellOutputDisposeChecked(&automatic)==XUI_OK);
    xui_table_view_cell_output_t *b=NULL,*c=NULL;assert(xuiTableViewCellOutputCreate(&a)==XUI_OK && xuiTableViewCellOutputCreate(&b)==XUI_OK && xuiTableViewCellOutputCreate(&c)==XUI_OK);
    assert(xuiTableViewCellOutputSetText(a,(xstrview){"leaf",4})==XUI_OK);
    assert(xuiTableViewCellOutputUse(b,a)==XUI_OK && xuiTableViewCellOutputUse(c,b)==XUI_OK && c->_pSource==a && a->_iUsePins==2);
    assert(xuiTableViewCellOutputUse(a,c)==XUI_ERROR_BUSY);assert(xuiTableViewCellOutputReleaseChecked(b)==XUI_OK && a->_iUsePins==1);
    assert(xuiTableViewCellOutputReleaseChecked(c)==XUI_OK && a->_iUsePins==0);
    assert(xuiTableViewCellOutputSetText(a,(xstrview){"again",5})==XUI_OK && xuiTableViewCellOutputReleaseChecked(a)==XUI_OK);
}
static void callbacks_and_trace(void)
{
    for(int action=0;action<=3;++action){
        xui_table_view_cell_output_t *cell=NULL,*cache=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK && xuiTableViewCellOutputCreate(&cache)==XUI_OK);
        CellOwner owner={.refs=1,.action=action,.cell=cell,.cache=cache};xui_callback_owner_t ref=cell_ref(&owner);
        int status=xuiTableViewCellOutputSetFormatterOwned(cell,cell_text,&owner,&ref);
        if(action==2){assert(status==XUI_ERROR_BUSY && owner.refs==1);assert(xuiTableViewCellOutputReset(cell)==XUI_OK);}
        else assert(status==XUI_OK);
        if(action!=3){owner.cell=NULL;assert(xuiTableViewCellOutputReleaseChecked(cell)==XUI_OK);}
        else owner.cell=NULL; /* The setter scope kept physical storage through the release hook. */
        assert(owner.refs==1 && cell_drop(&owner));assert(xuiTableViewCellOutputReleaseChecked(cache)==XUI_OK);
    }
    xui_table_view_cell_output_t* cell=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK);
    CellOwner text={.refs=1},render={.refs=1};xui_callback_owner_t a=cell_ref(&text),b=cell_ref(&render);
    text.failHold=true;assert(xuiTableViewCellOutputSetFormatterOwned(cell,cell_text,&text,&a)==XUI_ERROR_CALLBACK_FAILED && text.refs==1);text.failHold=false;
    assert(xuiTableViewCellOutputSetFormatterOwned(cell,cell_text,&text,&a)==XUI_OK && xuiTableViewCellOutputSetRendererOwned(cell,cell_render,&render,&b)==XUI_OK);
    xvalue* v=xrtValueInt(10);assert(v && xuiTableViewCellOutputSetValue(cell,v)==XUI_OK);xrtValueRelease(v);
    xrtownershipref ref=xuiTableViewCellOutputOwnership(cell);size_t count=0;unsigned edges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count) && count==1 && ref.Ops->Trace(ref.Data,visit_count,&edges) && edges==3);assert(xrtOwnershipScopeEnd(&freeze));
    text.failDrop=true;assert(xuiTableViewCellOutputReset(cell)==XUI_ERROR_CALLBACK_FAILED && !cell->_onText && !cell->_onRender && text.refs==1 && render.refs==1);text.failDrop=false;
    assert(xuiTableViewCellOutputReleaseChecked(cell)==XUI_OK && cell_drop(&text) && cell_drop(&render));
}
static void native_adapter(void)
{
    xui_table_view_cell_output_t* cell=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK);
    CellOwner owner={.refs=1,.cell=cell};xui_callback_owner_t binding=cell_ref(&owner);
    assert(xuiTableViewCellOutputAddRef(cell)==XUI_OK); /* real reference owned by owner */
    assert(xuiTableViewCellOutputSetFormatterOwned(cell,cell_text,&owner,&binding)==XUI_OK);
    assert(cell_drop(&owner) && owner.refs==1);assert(xuiTableViewCellOutputReleaseChecked(cell)==XUI_OK);
    xrtownershipref ref=xuiTableViewCellOutputOwnership(cell);const xrtownershipadapterv1* adapter=xuiTableViewCellOutputOwnershipAdapterV1(ref);assert(adapter);
    const xrtownershipquiescencev1* q=xuiTableViewCellOutputOwnershipQuiescenceV1(ref);assert(q && !q->Busy(ref.Data));
    assert(!xuiTableViewCellOutputOwnershipAdapterV1((xrtownershipref){ref.Data,&cell_ops}));
    assert(adapter->Hold(ref.Data));int token=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(adapter->Claim(ref.Data,&token));adapter->Restore(ref.Data,&token);assert(adapter->Claim(ref.Data,&token));assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiTableViewCellOutputSetText(cell,(xstrview){"denied",6})==XUI_ERROR_INVALID_STATE);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
    assert(adapter->Finish(ref.Data,&token) && adapter->Finish(ref.Data,&token));assert(owner.refs==0);
    adapter->Drop(ref.Data);
}

static void consumed_release(void)
{
    bool consumed=true;
    assert(xuiTableViewCellOutputReleaseOwnerChecked(NULL,&consumed)==XUI_ERROR_INVALID_ARGUMENT && !consumed);
    xui_table_view_cell_output_t automatic;xuiTableViewCellOutputInit(&automatic);
    consumed=true;assert(xuiTableViewCellOutputReleaseOwnerChecked(&automatic,&consumed)==XUI_ERROR_INVALID_ARGUMENT && !consumed);
    assert(xuiTableViewCellOutputDisposeChecked(&automatic)==XUI_OK);
    for(int deferred=0;deferred<2;++deferred){
        xui_table_view_cell_output_t* cell=NULL;assert(xuiTableViewCellOutputCreate(&cell)==XUI_OK);
        consumed=false;unsigned invalid=0;xui_lifetime_link_t watch={0};assert(xuiTableViewCellOutputLifetimeWatch(cell,&watch,invalidated,&invalid)==XUI_OK);
        CellOwner owner={.refs=1};xui_callback_owner_t ref=cell_ref(&owner);
        assert(xuiTableViewCellOutputSetFormatterOwned(cell,cell_text,&owner,&ref)==XUI_OK && cell_drop(&owner));
        owner.failDrop=true;owner.consumed=&consumed;
        assert(xuiTableViewCellOutputReleaseOwnerChecked(cell,NULL)==XUI_ERROR_INVALID_ARGUMENT);
        xui_table_view_cell_output_t output;xuiTableViewCellOutputInit(&output);
        if(deferred)assert(xuiTableViewCellOutputUse(&output,cell)==XUI_OK);
        int result=xuiTableViewCellOutputReleaseOwnerChecked(cell,&consumed);
        assert(consumed && result==(deferred ? XUI_OK:XUI_ERROR_CALLBACK_FAILED) && !xrtGetError());
        if(deferred){assert(!invalid && owner.refs==1);assert(xuiTableViewCellOutputReset(&output)==XUI_ERROR_CALLBACK_FAILED);}
        assert(invalid==1 && owner.refs==0 && !xrtGetError());
        assert(xuiTableViewCellOutputDisposeChecked(&output)==XUI_OK);
    }
}
static void preparation_and_use_cycle(void)
{
    xui_table_view_cell_output_t *a=NULL,*b=NULL;assert(xuiTableViewCellOutputCreate(&a)==XUI_OK && xuiTableViewCellOutputCreate(&b)==XUI_OK);
    xrtownershipref ar=xuiTableViewCellOutputOwnership(a),br=xuiTableViewCellOutputOwnership(b);
    const xrtownershipadapterv1 *aa=xuiTableViewCellOutputOwnershipAdapterV1(ar),*ba=xuiTableViewCellOutputOwnershipAdapterV1(br);
    const xrtownershippreparationv1* prep=xuiTableViewCellOutputOwnershipPreparationV1(ar);assert(prep && prep->Adapter==aa && prep->Ready(a));
    xui_table_cell_scope_t scope={0};assert(xuiTableViewCellOutputCallScopeBegin(a,&scope)==XUI_OK && !prep->Ready(a));
    assert(xuiTableViewCellOutputOwnershipAdapterV1(ar)==aa && xuiTableViewCellOutputOwnershipPreparationV1(ar)==prep);
    int token=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));assert(!aa->Claim(a,&token));assert(xrtOwnershipScopeEnd(&freeze));assert(xuiTableViewCellOutputCallScopeEnd(&scope)==XUI_OK);
    /* A owns a callback which really owns B; B owns a persistent Use pin on A.
     * All application owners are removed before the native collector protocol. */
    CellOwner owner={.refs=1,.cell=b};assert(xuiTableViewCellOutputAddRef(b)==XUI_OK);xui_callback_owner_t binding=cell_ref(&owner);
    assert(xuiTableViewCellOutputSetFormatterOwned(a,cell_text,&owner,&binding)==XUI_OK && cell_drop(&owner));
    assert(xuiTableViewCellOutputUse(b,a)==XUI_OK);
    assert(xuiTableViewCellOutputReleaseChecked(a)==XUI_OK && xuiTableViewCellOutputReleaseChecked(b)==XUI_OK);
    assert(!xuiTableViewCellOutputBusy(a) && prep->Ready(a));
    assert(aa->Hold(a) && ba->Hold(b));assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(aa->Claim(a,&token) && ba->Claim(b,&token));assert(xrtOwnershipScopeEnd(&freeze));
    assert(prep->Prepare(a,&token)==XRT_OWNERSHIP_PREPARE_READY);
    assert(xrtOwnershipFreezeTryBegin(&freeze));aa->Clear(a,&token);ba->Clear(b,&token);assert(xrtOwnershipScopeEnd(&freeze));
    assert(aa->Finish(a,&token) && ba->Finish(b,&token) && owner.refs==0);
    aa->Drop(a);ba->Drop(b);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    simple_values();cache_pin();callbacks_and_trace();native_adapter();consumed_release();preparation_and_use_cycle();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("owned Cell: actual native RC/trace/adapter, safe text/value, callback reentry, Use read pins and zero-allocation cache reuse passed");return 0;
}
