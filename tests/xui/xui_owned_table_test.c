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


#include "xui_test_proxy.h"
typedef struct TableState TableState;
typedef struct TableOwner {size_t refs,holds,drops,calls;bool failHold,failDrop;int kind;TableState* state;} TableOwner;
typedef struct TableUser {unsigned marker;TableOwner* owner;} TableUser;
struct TableState {
    xui_context context;xui_widget root,table;xui_test_proxy_state_t proxy;xui_surface target;xui_font font;
    xui_draw_text_proc originalDraw;
    xui_table_view_cell_output_t* cache;
    TableOwner owners[4];TableUser users[4];
    bool cached,armed,done,closed,widgetClosed,inHold;
    int phase,action,nested;unsigned textFreed,draws;
    const void* returnedText;
};
static TableState* drawing;
static bool no_children(const void* data,xrtownershipvisitor visit,void* user){(void)data;(void)visit;(void)user;return true;}
static bool owner_count(const void* raw,size_t* n){*n=((const TableOwner*)raw)->refs;return *n!=0;}
static void change(TableState* s);
static bool owner_hold(const void* raw)
{
    TableOwner* o=(TableOwner*)raw;assert(o->refs);++o->holds;if(o->failHold)return false;++o->refs;
    if(o->state->armed && o->state->inHold && o->state->phase==o->kind)change(o->state);
    return true;
}
typedef struct FindLive {const void* address;bool found;} FindLive;
static bool find_live(const xmemdebugallocation* a,void* raw){FindLive* f=raw;if(a->Address==f->address)f->found=true;return true;}
static bool live(const void* address){FindLive f={address,false};(void)xrtMemDebugVisitLive(find_live,&f);return f.found;}
static bool owner_drop(const void* raw)
{
    TableOwner* o=(TableOwner*)raw;assert(o->refs);++o->drops;
    if(o->kind==2 && o->state->returnedText){assert(!live(o->state->returnedText));o->state->returnedText=NULL;++o->state->textFreed;}
    --o->refs;return !o->failDrop;
}
static const xrtownershipops owner_ops={owner_count,no_children};
static xui_callback_owner_t owner_ref(TableOwner* o){return(xui_callback_owner_t){{o,&owner_ops},owner_hold,owner_drop};}
static TableState* check_user(void* raw,int kind)
{TableUser* u=raw;assert(u && u->marker==0xAB00u+(unsigned)kind && u->owner->kind==kind && u->owner->refs);++u->owner->calls;return u->owner->state;}
static int raw_count(xui_widget widget,void* user){(void)widget;(void)user;return 2;}
static int raw_cell(xui_widget widget,int row,int col,xui_table_view_cell_t* cell,void* user)
{(void)widget;(void)row;(void)col;(void)user;cell->sText="raw";return 1;}
static void change(TableState* s)
{
    if(s->done)return;
    s->done=true;
    if(s->action==1)assert(xuiTableViewSetAdapterOwned(s->table,NULL,NULL,NULL,NULL,NULL,NULL)==XUI_OK);
    else if(s->action==2)assert(xuiTableViewSetAdapter(s->table,raw_count,raw_cell,NULL)==XUI_OK);
    else if(s->action==3){s->widgetClosed=true;xuiWidgetDestroy(s->table);}
    else if(s->action==4){s->closed=true;xuiDestroy(s->context);}
    else if(s->action==5){xui_table_view_cell_output_t out;xuiTableViewCellOutputInit(&out);s->nested=xuiTableViewGetCellOwned(s->table,0,0,&out);assert(xuiTableViewCellOutputDisposeChecked(&out)==XUI_OK);}
}
static int provider_count(xui_widget widget,void* user)
{TableState* s=check_user(user,0);assert(widget==s->table);if(s->armed && !s->inHold && s->phase==0)change(s);return 2;}
static int provider_cell(xui_widget widget,int row,int col,xui_table_view_cell_output_t* output,void* user)
{
    TableState* s=check_user(user,1);assert(widget==s->table);
    if(s->armed && !s->inHold && s->phase==1)change(s);
    if(s->cached)return xuiTableViewCellOutputUse(output,s->cache);
    char local[48];int n=snprintf(local,sizeof(local),"row-%d-%d",row,col);
    int status=xuiTableViewCellOutputSetText(output,(xstrview){local,(size_t)n});memset(local,0xCC,sizeof(local));
    if(status==XUI_OK)status=xuiTableViewCellOutputSetTooltip(output,(xstrview){"owned tooltip",13});
    return status;
}
static xstrview formatter(xui_widget widget,int row,int col,const xui_table_view_cell_output_t* output,void* user)
{
    TableState* s=check_user(user,2);assert(widget==s->table && !xuiTableViewCellOutputGetText(output).Size);
    assert(xuiTableViewCellOutputSetText(s->cache,(xstrview){"blocked",7})==XUI_ERROR_BUSY);
    if(s->armed && !s->inHold && s->phase==2)change(s);
    char local[32];int n=snprintf(local,sizeof(local),"format-%d-%d",row,col);char* text=xrtMalloc((size_t)n+1);
    if(text){memcpy(text,local,(size_t)n+1);s->returnedText=text;}
    return(xstrview){text,text ? (size_t)n:0};
}
static int renderer(xui_widget widget,int row,int col,const xui_table_view_cell_output_t* output,xui_draw_context draw,xui_rect_t rect,int flags,void* user)
{
    (void)row;(void)col;(void)draw;(void)rect;(void)flags;
    TableState* s=check_user(user,3);assert(widget==s->table && xuiTableViewCellOutputGetValue(output)==NULL);
    if(s->armed && !s->inHold && s->phase==3)change(s);
    return 0;
}
static int draw_text(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    TableState* s=drawing;assert(s);
    if(item->sText==s->returnedText){assert(live(s->returnedText) && s->owners[2].refs>1);++s->draws;}
    return s->originalDraw(proxy,draw,item,rect,color,flags);
}
static void make_table(TableState* s,bool cached,bool display)
{
    memset(s,0,sizeof(*s));s->cached=cached;xuiTestProxyInit(&s->proxy);
    s->originalDraw=s->proxy.tProxy.drawText;s->proxy.tProxy.drawText=draw_text;
    assert(xuiCreate(&s->context)==XUI_OK && xuiSetProxy(s->context,&s->proxy.tProxy)==XUI_OK && xuiInputViewport(s->context,400,240)==XUI_OK);
    if(display){assert(s->proxy.tProxy.fontLoadFile(&s->proxy.tProxy,&s->font,"test-font",12,0)==XUI_OK);assert(xuiSetDefaultFont(s->context,s->font)==XUI_OK);}
    assert(xuiWidgetCreate(s->context,&s->root)==XUI_OK && xuiSetRootWidget(s->context,s->root)==XUI_OK && xuiWidgetSetRect(s->root,(xui_rect_t){0,0,400,240})==XUI_OK);
    assert(xuiTableViewCreate(s->context,&s->table,NULL)==XUI_OK && xuiWidgetAddChild(s->root,s->table)==XUI_OK && xuiWidgetSetRect(s->table,(xui_rect_t){10,10,300,200})==XUI_OK);
    xui_table_view_column_t columns[2]={{.fWidth=80},{.fWidth=80}};
    assert(xuiTableViewSetColumns(s->table,columns,2)==XUI_OK && xuiTableViewSetRows(s->table,NULL,2)==XUI_OK);
    assert(xuiTableViewCellOutputCreate(&s->cache)==XUI_OK);
    for(int i=0;i<4;++i){s->owners[i]=(TableOwner){.refs=1,.kind=i,.state=s};s->users[i]=(TableUser){0xAB00u+(unsigned)i,&s->owners[i]};}
    if(display){
        xui_callback_owner_t text=owner_ref(&s->owners[2]),render=owner_ref(&s->owners[3]);
        assert(xuiTableViewCellOutputSetFormatterOwned(s->cache,formatter,&s->users[2],&text)==XUI_OK && xuiTableViewCellOutputSetRendererOwned(s->cache,renderer,&s->users[3],&render)==XUI_OK);
    }else assert(xuiTableViewCellOutputSetText(s->cache,(xstrview){"cached cell",11})==XUI_OK);
}
static int install(TableState* s)
{xui_callback_owner_t count=owner_ref(&s->owners[0]),cell=owner_ref(&s->owners[1]);return xuiTableViewSetAdapterOwned(s->table,provider_count,&s->users[0],&count,provider_cell,&s->users[1],&cell);}
static void drop_table(TableState* s)
{
    if(!s->closed)xuiDestroy(s->context);
    assert(xuiTableViewCellOutputReleaseChecked(s->cache)==XUI_OK);
    for(int i=0;i<4;++i){assert(s->owners[i].refs==1);assert(owner_drop(&s->owners[i]));}
    if(s->target)s->proxy.tProxy.surfaceDestroy(&s->proxy.tProxy,s->target);
    if(s->font)s->proxy.tProxy.fontDestroy(&s->proxy.tProxy,s->font);
}
static int paint(TableState* s)
{
    assert(xuiLayout(s->context)==XUI_OK);
    if(!s->target)assert(xuiTestSurfaceCreate(&s->proxy,&s->target,400,240,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    xui_widget_cache_render_proc callback=NULL;void* user=NULL;xui_widget viewport=xuiTableViewGetViewportWidget(s->table);
    assert(xuiWidgetGetCacheRenderCallback(viewport,&callback,&user)==XUI_OK && callback);
    xui_draw_context draw=NULL;assert(s->proxy.tProxy.drawBegin(&s->proxy.tProxy,&draw,s->target)==XUI_OK);
    drawing=s;
    int result=callback(viewport,draw,0,user);
    drawing=NULL;(void)s->proxy.tProxy.drawEnd(&s->proxy.tProxy,draw);
    return result;
}
static void output_and_cache(void)
{
    for(int cached=0;cached<2;++cached){
        TableState s;make_table(&s,cached,false);assert(install(&s)==XUI_OK);
        xui_table_view_cell_output_t output;xuiTableViewCellOutputInit(&output);
        assert(xuiTableViewGetCellOwned(s.table,0,1,&output)==XUI_OK);
        assert(!strcmp(xuiTableViewCellOutputGetText(&output).Data,cached ? "cached cell":"row-0-1"));
        if(cached){
            xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);for(int i=0;i<1000;++i)assert(xuiTableViewGetCellOwned(s.table,1,1,&output)==XUI_OK);xrtMemDebugSnapshot(&b);
            assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
        }else{
            assert(!strcmp(xuiTableViewCellOutputGetTooltip(&output).Data,"owned tooltip"));
            xstrview old=xuiTableViewCellOutputGetText(&output);assert(xrtMemDebugFailAfter(0));assert(xuiTableViewGetCellOwned(s.table,1,0,&output)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(xuiTableViewCellOutputGetText(&output).Data==old.Data);
        }
        assert(xuiTableViewCellOutputDisposeChecked(&output)==XUI_OK);
        assert(xuiTableViewSetAdapter(s.table,raw_count,raw_cell,NULL)==XUI_OK && s.owners[0].refs==1 && s.owners[1].refs==1);
        xuiTableViewCellOutputInit(&output);assert(xuiTableViewGetCellOwned(s.table,0,0,&output)==XUI_OK && !strcmp(xuiTableViewCellOutputGetText(&output).Data,"raw"));assert(xuiTableViewCellOutputDisposeChecked(&output)==XUI_OK);
        drop_table(&s);
    }
}
static void provider_reentry(void)
{
    for(int phase=0;phase<2;++phase)for(int action=1;action<=5;++action)for(int inHold=0;inHold<2;++inHold){
        TableState s;make_table(&s,false,false);assert(install(&s)==XUI_OK);
        xui_table_view_cell_output_t output;xuiTableViewCellOutputInit(&output);assert(xuiTableViewCellOutputSetText(&output,(xstrview){"preserve",8})==XUI_OK);
        s.phase=phase;s.action=action;s.inHold=inHold;s.armed=true;
        int result=xuiTableViewGetCellOwned(s.table,0,0,&output);assert(s.done);
        if(action==5){if(result!=XUI_OK)fprintf(stderr,"reentry phase=%d inHold=%d nested=%d result=%d\n",phase,inHold,s.nested,result);assert(result==XUI_OK);if(phase==1)assert(s.nested==XUI_ERROR_INVALID_STATE);}
        else assert(result<0 && !strcmp(xuiTableViewCellOutputGetText(&output).Data,"preserve"));
        assert(xuiTableViewCellOutputDisposeChecked(&output)==XUI_OK);drop_table(&s);xrtClearError();
    }
}
static void publication_failures(void)
{
    unsigned failed=0,passed=0;
    for(int budget=0;budget<7;++budget){TableState s;make_table(&s,false,false);assert(xrtMemDebugFailAfter((uint64)budget));int result=install(&s);assert(xrtMemDebugFailAfter(-1));xrtClearError();if(result==XUI_OK)++passed;else{assert(result==XUI_ERROR_OUT_OF_MEMORY && s.owners[0].refs==1 && s.owners[1].refs==1);++failed;}drop_table(&s);}
    assert(failed && passed);
    for(int kind=0;kind<2;++kind){TableState s;make_table(&s,false,false);assert(install(&s)==XUI_OK);s.owners[kind].failHold=true;xui_table_view_cell_output_t out;xuiTableViewCellOutputInit(&out);assert(xuiTableViewGetCellOwned(s.table,0,0,&out)==XUI_ERROR_CALLBACK_FAILED);s.owners[kind].failHold=false;assert(xuiTableViewCellOutputDisposeChecked(&out)==XUI_OK);drop_table(&s);xrtClearError();}
}
static void actual_consumers(void)
{
    TableState s;make_table(&s,true,true);int installed=install(&s);int painted=paint(&s);
    if(installed!=XUI_OK || painted!=XUI_OK || !s.owners[2].calls || !s.owners[3].calls || !s.draws || s.textFreed!=s.owners[2].calls){xui_rect_t rect=xuiTableViewGetViewportRect(s.table);fprintf(stderr,"actual installed=%d painted=%d format=%zu render=%zu draws=%u freed=%u viewport=%d,%d,%d,%d\n",installed,painted,s.owners[2].calls,s.owners[3].calls,s.draws,s.textFreed,rect.fX,rect.fY,rect.fW,rect.fH);}
    assert(installed==XUI_OK && painted==XUI_OK && s.owners[2].calls && s.owners[3].calls && s.draws && s.textFreed==s.owners[2].calls);
    xui_accessible_node_t node={.iSize=sizeof(node)};assert(xuiWidgetGetAccessibleNode(s.table,1,&node)==XUI_OK && !strcmp(node.sName,"format-0-0"));assert(s.textFreed==s.owners[2].calls);
    drop_table(&s);
    for(int phase=2;phase<4;++phase)for(int action=1;action<=4;++action)for(int inHold=0;inHold<2;++inHold){
        make_table(&s,true,true);assert(install(&s)==XUI_OK);assert(xuiLayout(s.context)==XUI_OK);s.phase=phase;s.action=action;s.inHold=inHold;s.armed=true;
        int result=paint(&s);if(!s.done || result>=0)fprintf(stderr,"paint phase=%d action=%d inHold=%d fired=%d result=%d\n",phase,action,inHold,s.done,result);assert(s.done && result<0);assert(!s.returnedText);drop_table(&s);xrtClearError();
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    output_and_cache();provider_reentry();publication_failures();actual_consumers();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("owned Table actual consumers: provider/result ownership, exact cached zero-alloc Use, OOM/reentry/close, formatter free-before-Drop and native rendering/accessibility passed");return 0;
}
