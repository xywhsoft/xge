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
typedef struct TreeState TreeState;
typedef struct TreeOwner {unsigned refs,calls,released,disposed;bool failHold,failDrop;int holdAction;TreeState* state;xui_tree_view_node_output_t* output;} TreeOwner;
typedef struct TreeUser {unsigned marker;TreeOwner* owner;} TreeUser;
struct TreeState {xui_context context;xui_widget tree;int mode,atNode,revision,recursiveResult;bool done;TreeOwner countOwner,nodeOwner;TreeUser countUser,nodeUser;char scratch[64];};
static bool tree_hold(const void* raw)
{TreeOwner* o=(TreeOwner*)raw;if(o->failHold)return false;assert(o->refs);++o->refs;if(o->holdAction){o->holdAction=0;xuiDestroy(o->state->context);}return true;}
static bool tree_drop(const void* raw)
{TreeOwner* o=(TreeOwner*)raw;assert(o->refs);if(o->output){assert(!o->output->_tText.Data && !o->output->_tText.Size);++o->disposed;o->output=NULL;}if(--o->refs)return true;++o->released;return !o->failDrop;}
static bool tree_count_refs(const void* raw,size_t* output){const TreeOwner* o=raw;*output=o->refs;return o->refs!=0;}
static bool tree_trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops tree_ops={tree_count_refs,tree_trace};
static xui_callback_owner_t tree_ref(TreeOwner* o){return(xui_callback_owner_t){{o,&tree_ops},tree_hold,tree_drop};}
static bool tree_visit(xrtownershipref ref,void* raw){assert(ref.Ops==&tree_ops && ref.Data);++*(unsigned*)raw;return true;}
static void mutate(TreeState* s)
{
    if(s->done)return;
    s->done=true;
    if(s->mode==1){xui_tree_view_node_t node={.iId=700,.iParent=-1,.sText="replacement",.bEnabled=1};assert(xuiTreeViewSetNodes(s->tree,&node,1)==XUI_OK);}
    else if(s->mode==2)s->recursiveResult=xuiTreeViewSetAdapter(s->tree,NULL,NULL,NULL);
    else if(s->mode==3)s->recursiveResult=xuiTreeViewRefreshAdapter(s->tree);
    else if(s->mode==4)xuiWidgetDestroy(s->tree);
    else if(s->mode==5)xuiDestroy(s->context);
}
static TreeOwner* check_user(void* raw,unsigned marker)
{TreeUser* u=raw;assert(u && u->marker==marker && u->owner->refs && !u->owner->released);++u->owner->calls;return u->owner;}
static int count_rows(xui_widget w,void* raw)
{TreeOwner* o=check_user(raw,0xAA01);assert(w==o->state->tree);if(!o->state->atNode)mutate(o->state);return o->state->mode==6 ? XUI_ERROR_CALLBACK_FAILED:2;}
static int owned_node(xui_widget w,int index,xui_tree_view_node_output_t* output,void* raw)
{
    TreeOwner* o=check_user(raw,0xBB02);TreeState* s=o->state;assert(w==s->tree);o->output=output;
    assert(output->iId==-1 && output->iParent==-1 && output->bEnabled && !xuiTreeViewNodeOutputGetText(output).Size);
    if(s->atNode)mutate(s);
    output->iId=s->revision*100+index;output->iParent=-1;
    char local[64];int n=snprintf(local,sizeof(local),"row-%d-%d",s->revision,index);assert(n>0);
    int status=xuiTreeViewNodeOutputSetText(output,(xstrview){local,(size_t)n});
    if(status==XUI_OK){xstrview v=xuiTreeViewNodeOutputGetText(output);assert(v.Size==(size_t)n && !memcmp(v.Data,local,v.Size));}
    memset(local,0xCC,sizeof(local)); /* No pointer into a callback local may escape. */
    return status;
}
static int raw_count(xui_widget w,void* raw){(void)w;(void)raw;return 2;}
static int raw_node(xui_widget w,int index,xui_tree_view_node_t* node,void* raw)
{(void)w;TreeState* s=raw;snprintf(s->scratch,sizeof(s->scratch),"row-%d-%d",s->revision,index);node->iId=s->revision*100+index;node->iParent=-1;node->sText=s->scratch;return XUI_OK;}
static void make_tree(TreeState* s)
{
    memset(s,0,sizeof(*s));s->revision=1;assert(xuiCreate(&s->context)==XUI_OK && xuiTreeViewCreate(s->context,&s->tree,NULL)==XUI_OK);
    s->countOwner=(TreeOwner){.refs=1,.state=s};s->nodeOwner=(TreeOwner){.refs=1,.state=s};s->countUser=(TreeUser){0xAA01,&s->countOwner};s->nodeUser=(TreeUser){0xBB02,&s->nodeOwner};
    xui_tree_view_node_t seed={.iId=50,.iParent=-1,.sText="seed",.bEnabled=1};assert(xuiTreeViewSetNodes(s->tree,&seed,1)==XUI_OK);
}
static int install_tree(TreeState* s)
{xui_callback_owner_t a=tree_ref(&s->countOwner),b=tree_ref(&s->nodeOwner);return xuiTreeViewSetAdapterOwned(s->tree,count_rows,&s->countUser,&a,owned_node,&s->nodeUser,&b);}
static void dispose_tree(TreeState* s,bool destroyed)
{if(!destroyed)xuiDestroy(s->context);assert(tree_drop(&s->countOwner) && tree_drop(&s->nodeOwner));assert(s->countOwner.released==1 && s->nodeOwner.released==1);}
static void output_values(void)
{
    xui_tree_view_node_output_t output;xuiTreeViewNodeOutputInit(&output);assert(output.iId==-1 && output.iParent==-1 && output.bEnabled && output.bIconReserved);
    assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){"abcdef",6})==XUI_OK);xstrview old=xuiTreeViewNodeOutputGetText(&output);
    assert(xrtMemDebugFailAfter(0));assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){"new",3})==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(xuiTreeViewNodeOutputGetText(&output).Data==old.Data);
    assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){old.Data+1,3})==XUI_OK);xstrview text=xuiTreeViewNodeOutputGetText(&output);assert(text.Size==3 && !memcmp(text.Data,"bcd",3));
    assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){"a\0b",3})==XUI_ERROR_INVALID_ARGUMENT);assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){"x",SIZE_MAX})==XUI_ERROR_INVALID_ARGUMENT);assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){NULL,1})==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTreeViewNodeOutputSetText(&output,(xstrview){0})==XUI_OK && !xuiTreeViewNodeOutputGetText(&output).Size);xuiTreeViewNodeOutputDispose(&output);xuiTreeViewNodeOutputDispose(&output);
}
static void normal_and_reentry(void)
{
    for(int mode=0;mode<=5;++mode)for(int atNode=0;atNode<2;++atNode){
        TreeState s;make_tree(&s);s.mode=mode;s.atNode=atNode;int result=install_tree(&s);
        if(mode==0 || mode==2 || mode==3){assert(result==XUI_OK && xuiTreeViewGetNodeCount(s.tree)==2);assert(s.nodeOwner.disposed==2);const xui_tree_view_node_t* n=xuiTreeViewGetNodeById(s.tree,100);assert(n && !strcmp(n->sText,"row-1-0"));if(mode)assert(s.recursiveResult==XUI_ERROR_INVALID_STATE);}
        else{assert(result==XUI_ERROR_INVALID_STATE);assert(s.nodeOwner.calls==(unsigned)atNode);if(mode==1)assert(xuiTreeViewGetNodeById(s.tree,700));}
        dispose_tree(&s,mode==5);
    }
}
static void allocation_failures(void)
{
    unsigned successes=0,failures=0;
    for(int budget=0;budget<48;++budget){
        TreeState s;make_tree(&s);assert(xrtMemDebugFailAfter(budget));int result=install_tree(&s);assert(xrtMemDebugFailAfter(-1));xrtClearError();
        if(result==XUI_OK){++successes;assert(xuiTreeViewGetNodeCount(s.tree)==2 && xuiTreeViewGetNodeById(s.tree,100));}
        else{++failures;assert(result==XUI_ERROR_OUT_OF_MEMORY && xuiTreeViewGetNodeCount(s.tree)==1 && xuiTreeViewGetNodeById(s.tree,50));}
        dispose_tree(&s,false);
    }
    assert(successes && failures);
}
static void ownership_failures(void)
{
    TreeState s;make_tree(&s);s.mode=6;assert(install_tree(&s)==XUI_ERROR_CALLBACK_FAILED && xuiTreeViewGetNodeById(s.tree,50));s.mode=0;s.done=false;assert(xuiTreeViewRefreshAdapter(s.tree)==XUI_OK);dispose_tree(&s,false);
    make_tree(&s);s.nodeOwner.failHold=true;assert(install_tree(&s)==XUI_ERROR_CALLBACK_FAILED && s.countOwner.refs==1 && s.nodeOwner.refs==1 && xuiTreeViewGetNodeById(s.tree,50));s.nodeOwner.failHold=false;assert(install_tree(&s)==XUI_OK);
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextTraceOwnership(s.context,tree_visit,&edges) && edges==2);assert(xrtOwnershipScopeEnd(&freeze));
    s.countOwner.failHold=true;assert(xuiTreeViewRefreshAdapter(s.tree)==XUI_ERROR_CALLBACK_FAILED && xuiTreeViewGetNodeById(s.tree,100));s.countOwner.failHold=false;
    s.nodeOwner.failHold=true;assert(xuiTreeViewRefreshAdapter(s.tree)==XUI_ERROR_CALLBACK_FAILED && xuiTreeViewGetNodeById(s.tree,100));s.nodeOwner.failHold=false;
    assert(xuiTreeViewSetAdapter(s.tree,raw_count,raw_node,&s)==XUI_OK && s.countOwner.refs==1 && s.nodeOwner.refs==1);dispose_tree(&s,false);
    for(int atNode=0;atNode<2;++atNode){make_tree(&s);assert(install_tree(&s)==XUI_OK);(atNode ? &s.nodeOwner:&s.countOwner)->holdAction=1;assert(xuiTreeViewRefreshAdapter(s.tree)==XUI_ERROR_INVALID_STATE);dispose_tree(&s,true);}
}
static void equivalent_native_work(void)
{
    TreeState raw,owned;make_tree(&raw);make_tree(&owned);assert(xuiTreeViewSetAdapter(raw.tree,raw_count,raw_node,&raw)==XUI_OK && install_tree(&owned)==XUI_OK);
    xmemdebugsnapshot a,b,c,d;xrtMemDebugSnapshot(&a);for(int i=0;i<10;++i)assert(xuiTreeViewRefreshAdapter(raw.tree)==XUI_OK);xrtMemDebugSnapshot(&b);
    xrtMemDebugSnapshot(&c);for(int i=0;i<10;++i)assert(xuiTreeViewRefreshAdapter(owned.tree)==XUI_OK);xrtMemDebugSnapshot(&d);
    assert(b.AllocCount-a.AllocCount==d.AllocCount-c.AllocCount && b.FreeCount-a.FreeCount==d.FreeCount-c.FreeCount && b.ReallocCount-a.ReallocCount==d.ReallocCount-c.ReallocCount);
    dispose_tree(&raw,false);dispose_tree(&owned,false);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);output_values();normal_and_reentry();allocation_failures();ownership_failures();equivalent_native_work();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);puts("owned Tree output: native text lifetime, dispose-before-Drop, independent closures, atomic visible nodes, OOM/reentry/close and equivalent allocation work passed");return 0;
}
