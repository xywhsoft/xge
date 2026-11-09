/* CPU-only provider publication, snapshot lifetime and exact resource bytes. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Owner {xatomic32 refs,calls;int clear,failHold,failDrop,poison;const char* expected;} Owner;
static bool count(const void* data,size_t* result)
{*result=xrtAtomic32Load(&((Owner*)data)->refs,XMEMORY_RELAXED);return true;}
static bool trace(const void* data,xrtownershipvisitor visitor,void* user)
{(void)data;(void)visitor;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* data)
{Owner* owner=(Owner*)data;if(owner->failHold){--owner->failHold;return false;}xrtAtomic32FetchAdd(&owner->refs,1,XMEMORY_RELAXED);return true;}
static bool drop(const void* data)
{Owner* owner=(Owner*)data;assert(xrtAtomic32FetchSub(&owner->refs,1,XMEMORY_RELAXED)>1);if(owner->failDrop){--owner->failDrop;return false;}return true;}
static xge_callback_owner_t descriptor(Owner* owner)
{return (xge_callback_owner_t){{owner,&ops},hold,drop};}
static void init(Owner* owner)
{memset(owner,0,sizeof(*owner));xrtAtomic32Init(&owner->refs,1);xrtAtomic32Init(&owner->calls,0);}
static int read_owned(const xstrview* path,xge_resource_output_t* output,void* user)
{
    Owner* owner=user;xrtAtomic32FetchAdd(&owner->calls,1,XMEMORY_RELAXED);
    assert(path && path->Data && path->Size==strlen(path->Data));
    if(owner->expected)assert(path->Size==strlen(owner->expected) && !memcmp(path->Data,owner->expected,path->Size));
    if(owner->clear){assert(xgeResourceProviderClearChecked()==XGE_OK);assert(xrtAtomic32Load(&owner->refs,XMEMORY_RELAXED)>=2);}
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    if(owner->poison){assert(xgeResourceOutputSet(output,NULL,3)==XGE_ERROR_INVALID_ARGUMENT);return XGE_OK;}
    return xgeResourceOutputSet(output,"a\0b",3);
}
static unsigned rawFrees;
static xge_resource_t* recursiveFree;
static int read_raw(const char* path,void** data,int* size,void* user)
{
    (void)path;(void)user;*data=xrtMalloc(3);if(!*data)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(*data,"a\0b",3);*size=3;xgeResourceProviderClear();return XGE_OK;
}
static void free_raw(void* data,void* user)
{assert(user==(void*)&rawFrees);++rawFrees;if(recursiveFree)xgeResourceFree(recursiveFree);xrtFree(data);}
static int read_empty(const char* path,void** data,int* size,void* user)
{(void)path;(void)user;*data=NULL;*size=0;return XGE_OK;}
static void* read_pack(void* pack,const char* path,uint64_t* bytes,void* user)
{
    assert(pack==&rawFrees && user==&rawFrees);assert(!strcmp(path,"base/path"));
    xgeResourceProviderClear();*bytes=3;void* data=xrtMalloc(3);if(data)memcpy(data,"a\0b",3);return data;
}
static int read_mini(const char* path,void** data,int* size,void* user)
{
    (void)path;(void)user;*data=xrtMalloc(3);if(!*data)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(*data,"a\0b",3);*size=3;xgeMiniProgramUnit();return XGE_OK;
}
static int32 registry_reader(void* unused)
{
    (void)unused;
    for(int i=0;i<2000;++i){
        xge_resource_t value={0};int status=xgeResourceLoad("parallel://empty",&value);
        assert(status==XGE_OK || status==XGE_ERROR_UNSUPPORTED);xgeResourceFree(&value);
    }
    return 0;
}
static void check_payload(xge_resource_t* value)
{assert(value->iSize==3 && value->pData && !memcmp(value->pData,"a\0b",3));}
typedef struct Symbol {const char* name;xge_host_xrt_proc address;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)(Name)},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;size_t low=0,high=sizeof(symbols)/sizeof(*symbols);
    while(low<high){size_t middle=low+(high-low)/2;int order=strcmp(name,symbols[middle].name);
        if(order<0)high=middle;else if(order>0)low=middle+1;else return symbols[middle].address;}
    return NULL;
}

int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;const char* missing=NULL;
    assert(xgeHostXrtBind(&contract,resolve,NULL,&missing)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot overallBefore,overallAfter;xrtMemDebugSnapshot(&overallBefore);
    Owner owner;init(&owner);xge_callback_owner_t own=descriptor(&owner);xge_resource_t value={0};
    assert(xgeResourceProviderAddOwned("not:a:scheme",read_owned,&owner,&own)==XGE_ERROR_INVALID_ARGUMENT);
    owner.failHold=1;assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_ERROR_INVALID_STATE);
    assert(xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);
    xge_resource_t untouched={0};untouched.iSize=73;
    assert(xgeResourceLoad(NULL,&untouched)==XGE_ERROR_INVALID_ARGUMENT && untouched.iSize==73);
    char scheme[]="test";assert(xgeResourceProviderAddOwned(scheme,read_owned,&owner,&own)==XGE_OK);scheme[0]='X';
    assert(xgeInit(NULL)==XGE_OK); /* Pre-init registration survives initialization. */
    assert(xgeResourceLoad("test://nul",&value)==XGE_OK);check_payload(&value);assert(value.iProvider>0);
    assert(xgeResourceProviderClearChecked()==XGE_OK);assert(xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);
    check_payload(&value);xgeResourceFree(&value);assert(xgeUnitChecked()==XGE_OK);
    xerror* ambient=xrtErrorCreate(XERR_STATE,"ambient",1,"keep prior caller diagnostic");
    assert(ambient);xrtSetError(ambient);
    assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_OK && xrtGetError()==ambient);
    assert(xgeResourceLoad("test://ambient",&value)==XGE_OK && xrtGetError()==ambient);
    xgeResourceFree(&value);assert(xgeResourceProviderClearChecked()==XGE_OK && xrtGetError()==ambient);
    xrtClearError();xrtErrorFree(ambient);
    owner.clear=1;assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_OK);
    assert(xgeResourceLoad("test://reenter",&value)==XGE_OK);check_payload(&value);xgeResourceFree(&value);
    assert(xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);owner.clear=0;
    /* Pack root is copied; paths are normalized and traversal does not invoke a callback. */
    char root[]="base";owner.expected="base/path";
    assert(xgeResourceXPackProviderAddOwned(root,read_owned,&owner,&own)==XGE_OK);root[0]='X';
    assert(xgeResourceLoad("res:///path",&value)==XGE_OK);check_payload(&value);xgeResourceFree(&value);
    unsigned calls=xrtAtomic32Load(&owner.calls,XMEMORY_RELAXED);
    assert(xgeResourceLoad("res://../escape",&value)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xrtAtomic32Load(&owner.calls,XMEMORY_RELAXED)==calls);
    assert(xgeResourceProviderClearChecked()==XGE_OK);owner.expected=NULL;
    owner.poison=1;assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_OK);
    assert(xgeResourceLoad("test://invalid",&value)==XGE_ERROR_INVALID_ARGUMENT && !value.pData && !value.free);
    assert(xgeResourceProviderClearChecked()==XGE_OK);owner.poison=0;
    /* Snapshot Hold and final Drop failures do not publish a partial resource. */
    assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_OK);owner.failHold=1;
    assert(xgeResourceLoad("test://hold",&value)==XGE_ERROR_INVALID_STATE && !value.pData);
    owner.failDrop=1;assert(xgeResourceProviderClearChecked()==XGE_ERROR_INVALID_STATE);
    assert(xgeResourceProviderClearChecked()==XGE_OK && xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);
    /* Unit clears registrations even if logical initialization never occurred. */
    assert(xgeResourceProviderAddOwned("test",read_owned,&owner,&own)==XGE_OK);
    assert(xgeUnitChecked()==XGE_OK && xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);
    /* Legacy C callbacks can clear their registry; the returned snapshot owns
     * the exact original free function and copied XPack root until Free. */
    xge_resource_provider_t raw={"raw",read_raw,free_raw,&rawFrees};assert(xgeResourceProviderAdd(&raw)==XGE_OK);
    assert(xgeResourceLoad("raw://self-clear",&value)==XGE_OK);check_payload(&value);recursiveFree=&value;
    xgeResourceFree(&value);recursiveFree=NULL;assert(rawFrees==1);
    raw.load=read_empty;assert(xgeResourceProviderAdd(&raw)==XGE_OK);
    assert(xgeResourceLoad("raw://empty",&value)==XGE_OK && value.free);
    xgeResourceProviderClear();xgeResourceFree(&value);assert(rawFrees==1);
    char packRoot[]="base";xge_xpack_provider_t pack={&rawFrees,packRoot,read_pack,free_raw,&rawFrees};
    assert(xgeResourceXPackProviderAdd(&pack)==XGE_OK);packRoot[0]='X';
    assert(xgeResourceLoad("res://path",&value)==XGE_OK);check_payload(&value);xgeResourceFree(&value);assert(rawFrees==2);
    assert(xgeResourceLoadMemory(NULL,0,&value)==XGE_OK);xgeResourceFree(&value);
    /* Native bridge replacement/removal uses the same immutable registry.
     * Self-removal keeps both registration and payload references intact. */
    assert(xgeMiniProgramInitSimple(64,64,1.0f)==XGE_OK);
    xge_miniprogram_bridge_t bridge={0};bridge.load_resource=read_mini;bridge.free_resource=free_raw;bridge.pUser=&rawFrees;
    assert(xgeMiniProgramSetBridge(&bridge)==XGE_OK);
    assert(xgeResourceLoad("res://mini",&value)==XGE_OK);check_payload(&value);
    xgeResourceFree(&value);assert(rawFrees==3);
    xgeResourceProviderClear();
    xge_resource_provider_t parallel={"parallel",read_empty,NULL,NULL};
    xthread* threads[4];for(int i=0;i<4;++i){threads[i]=xrtThreadCreate((ptr)registry_reader,NULL,0);assert(threads[i]);}
    for(int i=0;i<2000;++i){assert(xgeResourceProviderAdd(&parallel)==XGE_OK);assert(xgeResourceProviderClearChecked()==XGE_OK);}
    for(int i=0;i<4;++i){xrtThreadWait(threads[i]);xrtThreadDestroy(threads[i]);}

    /* One output allocation and no repeated scan; failed Set preserves old bytes. */
    xge_resource_output_t output={0};assert(xgeResourceOutputSet(&output,"a\0b",3)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));assert(xgeResourceOutputSet(&output,"new",3)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
    assert(output.iSize==3 && !memcmp(output.pData,"a\0b",3));xrtFree(output.pData);
    /* Registration and load failure prefixes restore native ownership/storage. */
    for(int pass=0;pass<2;++pass){
        xmemdebugsnapshot baseline={0},after={0};xrtMemDebugSnapshot(&baseline);
        for(int budget=0;budget<12;++budget){
            assert(xrtMemDebugFailAfter(budget));int status=xgeResourceProviderAddOwned("oom",read_owned,&owner,&own);
            xrtMemDebugFailClear();xrtClearError();
            assert(status==XGE_OK || status==XGE_ERROR_OUT_OF_MEMORY);
            if(status==XGE_OK){
                assert(xrtMemDebugFailAfter(budget));status=xgeResourceLoad("oom://bytes",&value);
                xrtMemDebugFailClear();xrtClearError();assert(status==XGE_OK || status==XGE_ERROR_OUT_OF_MEMORY);
                if(status==XGE_OK)check_payload(&value);else assert(!value.pData && !value.free);
                xgeResourceFree(&value);
            }
            assert(xgeResourceProviderClearChecked()==XGE_OK && xrtAtomic32Load(&owner.refs,XMEMORY_RELAXED)==1);
        }
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount==baseline.LiveCount && after.LiveBytes==baseline.LiveBytes);
        assert(after.InvalidFreeCount==baseline.InvalidFreeCount && after.DoubleFreeCount==baseline.DoubleFreeCount);
    }
    xrtClearError();xrtMemDebugSnapshot(&overallAfter);
    assert(overallBefore.LiveCount==overallAfter.LiveCount && overallBefore.LiveBytes==overallAfter.LiveBytes);
    assert(overallBefore.InvalidFreeCount==overallAfter.InvalidFreeCount && overallBefore.DoubleFreeCount==overallAfter.DoubleFreeCount);
    puts("resource providers: exact bytes, reentry, snapshots, rollback and OOM passed");return 0;
}
