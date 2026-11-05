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
typedef struct Grid Grid;
typedef struct Owner {Grid* grid;int kind;size_t refs,holds,drops,calls;bool failHold,failDrop,dropError;} Owner;
struct Grid {xui_context context;xui_widget root,widget,table;xui_test_proxy_state_t proxy;xui_table_view_cell_output_t* cache;Owner owners[6];bool armed,inHold,changed,closed,voidError;const xerror* primaryDiagnostic;int phase,action,validateResult,configResult;unsigned writes,rawConfigCalls;};
static bool count_ref(const void* raw,size_t* n){*n=((const Owner*)raw)->refs;return *n!=0;}
static bool trace_ref(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static int raw_config(xui_widget,int,int,int,xui_table_grid_editor_config_t*,void*);
static void reenter(Grid* s)
{
    if(s->changed)return;
    s->changed=true;
    if(s->action==1)assert(xuiTableGridSetAdapterOwned(s->widget,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==XUI_OK);
    else if(s->action==2){assert(xuiWidgetDestroyChecked(s->widget)==XUI_OK);s->closed=true;}
    else if(s->action==3)assert(xuiTableGridSetEditorConfig(s->widget,raw_config,s)==XUI_OK);
}
static bool hold(const void* raw){Owner* o=(Owner*)raw;assert(o->refs);++o->holds;if(o->failHold)return false;++o->refs;if(o->grid->armed && o->grid->inHold && o->grid->phase==o->kind)reenter(o->grid);return true;}
static bool drop(const void* raw){Owner* o=(Owner*)raw;assert(o->refs);++o->drops;--o->refs;if(o->failDrop && o->dropError)xrtSetErrorInfo(XERR_VALUE,"secondary-drop",42,"secondary consuming Drop");return !o->failDrop;}
static const xrtownershipops ops={count_ref,trace_ref};
static xui_callback_owner_t owner_ref(Owner* o){return(xui_callback_owner_t){{o,&ops},hold,drop};}
static Grid* entered(void* user,int kind,xui_widget widget){Owner* o=user;assert(o && o->kind==kind && o->refs>1 && o->grid->widget==widget);++o->calls;Grid* s=o->grid;if(s->armed && !s->inHold && s->phase==kind)reenter(s);return s;}
static int rows(xui_widget w,void* user){entered(user,0,w);return 2;}
static int cell(xui_widget w,int row,int col,xui_table_view_cell_output_t* out,void* user){(void)row;(void)col;Grid* s=entered(user,1,w);return xuiTableViewCellOutputUse(out,s->cache);}
static void write_value(xui_widget w,int row,int col,const char* text,int type,void* user){(void)row;(void)col;Grid* s=entered(user,2,w);assert(type==XUI_TABLE_CELL_TYPE_BOOL && !strcmp(text,"true"));assert(xuiTableViewCellOutputSetValue(s->cache,xrtValueBool(true))==XUI_OK);++s->writes;if(s->voidError){xrtSetErrorInfo(XERR_VALUE,"primary-void",41,"primary void callback");s->primaryDiagnostic=xrtGetError();}}
static int validate(xui_widget w,int row,int col,const char* text,int type,void* user){(void)row;(void)col;(void)text;(void)type;return entered(user,3,w)->validateResult;}
static int editor(xui_widget w,int row,int col,const xui_table_view_cell_output_t* out,const xui_table_grid_editor_config_output_t* config,xui_rect_t rect,void* user){(void)row;(void)col;(void)rect;Grid* s=entered(user,4,w);assert(out->iType==XUI_TABLE_CELL_TYPE_CUSTOM && !strcmp(xuiTableViewCellOutputGetText(out).Data,"custom"));
    assert(config->iPrecision==2 && !strcmp(xuiTableGridEditorConfigOutputGetFileFilter(config).Data,"*.xl") && !strcmp(xuiTableGridEditorConfigOutputGetActionText(config).Data,"Choose"));
    assert(xuiTableGridEditorConfigOutputGetEnumCount(config)==2 && xuiTableGridEditorConfigOutputGetPaletteCount(config)==2);
    xstrview text={0};int value=0,enabled=1,separator=1,icon=0;assert(xuiTableGridEditorConfigOutputGetEnumItem(config,1,&text,&value,&enabled,&separator,&icon)==XUI_OK && !strcmp(text.Data,"second") && value==20 && !enabled && !separator);
    uint32_t color=0;assert(xuiTableGridEditorConfigOutputGetPaletteItem(config,1,&color)==XUI_OK && color==0xAABBCCDDu);assert(xuiTableViewCellOutputSetText(s->cache,(xstrview){"locked",6})==XUI_ERROR_BUSY);return 1;}
static int config_callback(xui_widget widget,int row,int col,int type,xui_table_grid_editor_config_output_t* config,void* user)
{
    (void)row;(void)col;(void)type;Grid* s=entered(user,5,widget);config->iPrecision=2;
    char* filter=xrtMalloc(5);assert(filter);memcpy(filter,"*.xl",5);
    assert(xuiTableGridEditorConfigOutputSetFileFilter(config,(xstrview){filter,4})==XUI_OK);memset(filter,'x',4);xrtFree(filter);
    assert(xuiTableGridEditorConfigOutputSetActionText(config,(xstrview){"Choose",6})==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetEnumCount(config,2)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetEnumItem(config,0,(xstrview){"first",5},10,1,0,0)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetEnumItem(config,1,(xstrview){"second",6},20,0,0,0)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetPaletteCount(config,2)==XUI_OK && xuiTableGridEditorConfigOutputSetPaletteItem(config,1,0xAABBCCDDu)==XUI_OK);
    return s->configResult;
}
static int install_config(Grid* s)
{xui_callback_owner_t c=owner_ref(&s->owners[5]);return xuiTableGridSetEditorConfigOwned(s->widget,config_callback,&s->owners[5],&c);}
static int raw_config(xui_widget widget,int row,int col,int type,xui_table_grid_editor_config_t* config,void* user)
{(void)widget;(void)row;(void)col;(void)type;config->pUser=(void*)1;++((Grid*)user)->rawConfigCalls;return 0;}
static void make_grid(Grid* s)
{
 memset(s,0,sizeof(*s));s->validateResult=1;xuiTestProxyInit(&s->proxy);
 assert(xuiCreate(&s->context)==XUI_OK && xuiSetProxy(s->context,&s->proxy.tProxy)==XUI_OK && xuiInputViewport(s->context,400,240)==XUI_OK);
 assert(xuiWidgetCreate(s->context,&s->root)==XUI_OK && xuiSetRootWidget(s->context,s->root)==XUI_OK && xuiWidgetSetRect(s->root,(xui_rect_t){0,0,400,240})==XUI_OK);
 assert(xuiTableGridCreate(s->context,&s->widget,NULL)==XUI_OK && xuiWidgetAddChild(s->root,s->widget)==XUI_OK && xuiWidgetSetRect(s->widget,(xui_rect_t){0,0,320,200})==XUI_OK);s->table=xuiTableGridGetTableView(s->widget);
 xui_table_view_column_t column={.fWidth=100,.iType=XUI_TABLE_CELL_TYPE_BOOL};assert(xuiTableGridSetColumns(s->widget,&column,1)==XUI_OK && xuiTableGridSetRows(s->widget,NULL,2)==XUI_OK);
 assert(xuiTableViewCellOutputCreate(&s->cache)==XUI_OK);s->cache->iType=XUI_TABLE_CELL_TYPE_BOOL;assert(xuiTableViewCellOutputSetValue(s->cache,xrtValueBool(false))==XUI_OK);
 for(int i=0;i<6;++i)s->owners[i]=(Owner){.grid=s,.kind=i,.refs=1};
}
static int install(Grid* s){xui_callback_owner_t a=owner_ref(&s->owners[0]),b=owner_ref(&s->owners[1]),c=owner_ref(&s->owners[2]);return xuiTableGridSetAdapterOwned(s->widget,rows,&s->owners[0],&a,cell,&s->owners[1],&b,write_value,&s->owners[2],&c);}
static void finish(Grid* s){if(!s->closed)assert(xuiWidgetDestroyChecked(s->widget)==XUI_OK);assert(xuiTableViewCellOutputReleaseChecked(s->cache)==XUI_OK);xuiDestroy(s->context);for(int i=0;i<6;++i)assert(s->owners[i].refs==1);xrtClearError();}
static void actual_values(void)
{
 Grid s;make_grid(&s);assert(install(&s)==XUI_OK && xuiLayout(s.context)==XUI_OK);
 xui_table_view_cell_output_t out;xuiTableViewCellOutputInit(&out);assert(xuiTableViewGetCellOwned(s.table,0,0,&out)==XUI_OK);bool value=true;assert(xrtValueGetBool(xuiTableViewCellOutputGetValue(&out),&value) && !value);assert(xuiTableViewCellOutputDisposeChecked(&out)==XUI_OK);
 xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);for(int i=0;i<1000;++i){xuiTableViewCellOutputInit(&out);assert(xuiTableViewGetCellOwned(s.table,0,0,&out)==XUI_OK);assert(xuiTableViewCellOutputDisposeChecked(&out)==XUI_OK);}xrtMemDebugSnapshot(&b);assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
 assert(xuiTableGridBeginEdit(s.widget,0,0)==1 && s.writes==1);value=false;assert(xrtValueGetBool(xuiTableViewCellOutputGetValue(s.cache),&value) && value);
 s.cache->iType=XUI_TABLE_CELL_TYPE_CUSTOM;assert(xuiTableViewCellOutputSetText(s.cache,(xstrview){"custom",6})==XUI_OK);assert(install_config(&s)==XUI_OK);xui_callback_owner_t e=owner_ref(&s.owners[4]);assert(xuiTableGridSetEditorOwned(s.widget,editor,&s.owners[4],&e)==XUI_OK);assert(xuiTableGridBeginEdit(s.widget,0,0)==1 && s.owners[4].calls);assert(xuiTableViewCellOutputSetText(s.cache,(xstrview){"after",5})==XUI_OK);finish(&s);
}
static void config_failures_and_mix(void)
{
    for(int mode=0;mode<3;++mode){
        Grid s;make_grid(&s);assert(install(&s)==XUI_OK && xuiLayout(s.context)==XUI_OK);
        s.cache->iType=XUI_TABLE_CELL_TYPE_CUSTOM;assert(xuiTableViewCellOutputSetText(s.cache,(xstrview){"custom",6})==XUI_OK && install_config(&s)==XUI_OK);
        xui_callback_owner_t e=owner_ref(&s.owners[4]);assert(xuiTableGridSetEditorOwned(s.widget,editor,&s.owners[4],&e)==XUI_OK);
        if(mode==0)s.configResult=1;
        if(mode==1)s.configResult=-79;
        if(mode==2)s.owners[5].failDrop=true;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);
        assert(xuiTableGridBeginEdit(s.widget,0,0)==0 && !s.owners[4].calls);assert(xuiCallScopeEnd(&scope)<0);
        s.owners[5].failDrop=false;xrtClearError();finish(&s);
    }
    Grid s;make_grid(&s);assert(install(&s)==XUI_OK && xuiLayout(s.context)==XUI_OK);
    assert(xuiTableGridSetEditorConfig(s.widget,raw_config,&s)==XUI_OK);xui_callback_owner_t e=owner_ref(&s.owners[4]);size_t holds=s.owners[4].holds;
    assert(xuiTableGridSetEditorOwned(s.widget,editor,&s.owners[4],&e)==XUI_ERROR_UNSUPPORTED && s.owners[4].holds==holds && s.owners[4].refs==1);
    assert(install_config(&s)==XUI_OK && xuiTableGridSetEditorOwned(s.widget,editor,&s.owners[4],&e)==XUI_OK);
    assert(xuiTableGridSetEditorConfig(s.widget,raw_config,&s)==XUI_ERROR_UNSUPPORTED);
    s.cache->iType=XUI_TABLE_CELL_TYPE_CUSTOM;assert(xuiTableViewCellOutputSetText(s.cache,(xstrview){"custom",6})==XUI_OK);
    assert(xuiTableGridBeginEdit(s.widget,0,0)==1 && s.owners[5].calls && s.owners[4].calls && !s.rawConfigCalls);finish(&s);
}
static void diagnostic_priority(void)
{
    for(int secondary=0;secondary<2;++secondary){
        Grid s;make_grid(&s);assert(install(&s)==XUI_OK && xuiLayout(s.context)==XUI_OK);
        s.voidError=true;s.owners[2].failDrop=true;s.owners[2].dropError=secondary;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);
        (void)xuiTableGridBeginEdit(s.widget,0,0);
        assert(s.writes==1 && xrtGetError()==s.primaryDiagnostic && !strcmp(xrtErrorMessage(xrtGetError()),"primary void callback"));
        assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==s.primaryDiagnostic);
        s.owners[2].failDrop=s.owners[2].dropError=false;xrtClearError();finish(&s);
        make_grid(&s);xui_callback_owner_t v=owner_ref(&s.owners[3]);assert(xuiTableGridSetValidateOwned(s.widget,validate,&s.owners[3],&v)==XUI_OK);
        s.owners[3].failDrop=true;s.owners[3].dropError=secondary;scope=(xui_call_scope_t){0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);
        xrtSetErrorInfo(XERR_VALUE,"primary-replace",43,"primary replacement");const xerror* primary=xrtGetError();assert(primary);
        assert(xuiTableGridSetValidateOwned(s.widget,NULL,NULL,NULL)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==primary);
        assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && xrtGetError()==primary);
        s.owners[3].failDrop=s.owners[3].dropError=false;xrtClearError();finish(&s);
    }
}
static void publication_preflight(void)
{
    for(int action=2;action<=3;++action)for(int failedDrop=0;failedDrop<2;++failedDrop){
        Grid s;make_grid(&s);assert(install(&s)==XUI_OK);
        s.phase=4;s.action=action;s.inHold=true;s.armed=true;s.owners[4].failDrop=failedDrop;
        xui_callback_owner_t e=owner_ref(&s.owners[4]);xui_call_scope_t scope={0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);
        int expected=action==2 ? XUI_ERROR_INVALID_STATE:XUI_ERROR_UNSUPPORTED;
        assert(xuiTableGridSetEditorOwned(s.widget,editor,&s.owners[4],&e)==expected && s.changed && s.owners[4].refs==1 && !xrtGetError());
        /* The caller handled the ordinary setter result. Only rollback's
         * consuming Drop failure remains sticky in the parent receipt. */
        assert(xuiCallScopeEnd(&scope)==(failedDrop ? XUI_ERROR_CALLBACK_FAILED:XUI_OK) && !xrtGetError());
        s.owners[4].failDrop=false;finish(&s);
    }
}
static void config_record(void)
{
    xui_table_grid_editor_config_output_t c;xuiTableGridEditorConfigOutputInit(&c);
    const int invalidCounts[]={-1,XUI_COMBOBOX_ITEM_CAPACITY+1,1};
    for(size_t i=0;i<sizeof(invalidCounts)/sizeof(*invalidCounts);++i){
        c._iEnumCount=invalidCounts[i];assert(xrtMemDebugFailAfter(0));
        assert(xuiTableGridEditorConfigOutputSetEnumCount(&c,1)==XUI_ERROR_INVALID_STATE && !xrtMemDebugFailTriggered());
        assert(xrtMemDebugFailAfter(-1));c._iEnumCount=0;
    }
    assert(xuiTableGridEditorConfigOutputSetEnumCount(&c,1)==XUI_OK && xuiTableGridEditorConfigOutputSetEnumItem(&c,0,(xstrview){"old",3},3,1,0,0)==XUI_OK);
    xstrview text;int value,enabled,separator,icon;
    assert(xuiTableGridEditorConfigOutputGetEnumItem(&c,0,&text,&value,&enabled,&separator,&icon)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));assert(xuiTableGridEditorConfigOutputSetEnumItem(&c,0,(xstrview){"new",3},4,1,0,0)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();
    xstrview kept;assert(xuiTableGridEditorConfigOutputGetEnumItem(&c,0,&kept,&value,&enabled,&separator,&icon)==XUI_OK && kept.Data==text.Data && value==3);
    assert(xuiTableGridEditorConfigOutputSetEnumItem(&c,0,text,5,1,0,0)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetActionText(&c,(xstrview){"a\0b",3})==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTableGridEditorConfigOutputGetEnumItem(&c,1,&text,&value,&enabled,&separator,&icon)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTableGridEditorConfigOutputGetEnumItem(&c,0,&kept,&value,&enabled,&separator,&icon)==XUI_OK && value==5);
    const char* stable=kept.Data;
    assert(xuiTableGridEditorConfigOutputSetEnumCount(&c,XUI_COMBOBOX_ITEM_CAPACITY)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputGetEnumItem(&c,0,&kept,&value,&enabled,&separator,&icon)==XUI_OK && kept.Data==stable && value==5);
    assert(xuiTableGridEditorConfigOutputSetEnumCount(&c,1)==XUI_OK);
    assert(xuiTableGridEditorConfigOutputSetEnumCount(&c,0)==XUI_OK && !c._pEnumItems && !c._iEnumCount);
    xuiTableGridEditorConfigOutputDispose(&c);
}
static void validation_failures(void)
{
 for(int mode=0;mode<3;++mode){Grid s;make_grid(&s);assert(install(&s)==XUI_OK && xuiLayout(s.context)==XUI_OK);xui_callback_owner_t v=owner_ref(&s.owners[3]);assert(xuiTableGridSetValidateOwned(s.widget,validate,&s.owners[3],&v)==XUI_OK);if(mode==0)s.validateResult=-77;if(mode==1)s.owners[3].failHold=true;if(mode==2)s.owners[3].failDrop=true;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);assert(xuiTableGridBeginEdit(s.widget,0,0)==0 && s.writes==0);assert(xuiCallScopeEnd(&scope)<0);s.owners[3].failHold=s.owners[3].failDrop=false;xrtClearError();finish(&s);}
}
static void provider_reentry(void)
{
 for(int phase=0;phase<2;++phase)for(int action=1;action<=2;++action)for(int inHold=0;inHold<2;++inHold){Grid s;make_grid(&s);assert(install(&s)==XUI_OK);s.phase=phase;s.action=action;s.inHold=inHold;s.armed=true;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK);xui_table_view_cell_output_t out;xuiTableViewCellOutputInit(&out);int result=phase==0 ? xuiTableViewRefresh(s.table):xuiTableViewGetCellOwned(s.table,0,0,&out);if(!s.changed || result>=0)fprintf(stderr,"provider phase=%d action=%d hold=%d changed=%d result=%d\n",phase,action,inHold,s.changed,result);assert(s.changed && result<0);assert(xuiTableViewCellOutputDisposeChecked(&out)==XUI_OK);(void)xuiCallScopeEnd(&scope);xrtClearError();finish(&s);}
}
static void publication_oom(void)
{
 unsigned failed=0,passed=0;
 for(int budget=0;budget<20;++budget){Grid s;make_grid(&s);assert(xrtMemDebugFailAfter((uint64)budget));int result=install(&s);assert(xrtMemDebugFailAfter(-1));xrtClearError();if(result==XUI_OK)++passed;else{++failed;assert(result==XUI_ERROR_OUT_OF_MEMORY);}finish(&s);}assert(failed && passed);
}
int main(void)
{
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
 diagnostic_priority();publication_preflight();config_record();actual_values();config_failures_and_mix();validation_failures();provider_reentry();publication_oom();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);puts("owned Grid actual cached/value editing, independent callbacks, veto/failed Hold/Drop, OOM and provider reentry passed");return 0;
}
