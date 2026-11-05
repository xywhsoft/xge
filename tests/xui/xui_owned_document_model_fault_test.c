/* Fault the actual default libc allocator at the link boundary. This never
 * registers a custom allocator as an allegedly safe default model. */
#include "xui_owned_model_host_test.h"
#include "xui_document.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void* __real_malloc(size_t);
void* __real_calloc(size_t,size_t);
void* __real_realloc(void*,size_t);
void __real_free(void*);
static void* allocations[16384];
static size_t live,attempts,failAt;
static bool tracking;
static bool reject(void){return tracking && ++attempts==failAt;}
static void remember(void* p)
{
    if(!tracking || !p)return;
    for(size_t i=0;i<sizeof(allocations)/sizeof(*allocations);++i)if(!allocations[i]){allocations[i]=p;++live;return;}
    assert(!"allocator test capacity exhausted");
}
static void forget(void* p)
{
    if(!tracking || !p)return;
    for(size_t i=0;i<sizeof(allocations)/sizeof(*allocations);++i)if(allocations[i]==p){allocations[i]=NULL;assert(live);--live;return;}
    assert(!"untracked or multiply released default allocation");
}
void* __wrap_malloc(size_t n){if(reject())return NULL;void* p=__real_malloc(n);remember(p);return p;}
void* __wrap_calloc(size_t n,size_t size){if(reject())return NULL;void* p=__real_calloc(n,size);remember(p);return p;}
void* __wrap_realloc(void* old,size_t n)
{
    if(reject())return NULL;
    void* p=__real_realloc(old,n);
    if(p || !n){forget(old);remember(p);}
    return p;
}
void __wrap_free(void* p){forget(p);__real_free(p);}
typedef struct FaultObserver {size_t refs;} FaultObserver;
static bool faultHold(const void* raw){++((FaultObserver*)raw)->refs;return true;}
static bool faultDrop(const void* raw){FaultObserver* owner=(FaultObserver*)raw;assert(owner->refs);--owner->refs;return true;}
static bool faultCount(const void* raw,size_t* count){*count=((const FaultObserver*)raw)->refs;return *count!=0;}
static bool faultTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops faultObserverOps={faultCount,faultTrace};
static int faultNotify(xui_document document,xui_document_change_set changes,void* raw)
{(void)document;(void)changes;assert(((FaultObserver*)raw)->refs==3);return XUI_OK;}

static int modelWorkload(void)
{
    FaultObserver observer={1};
    xui_document document=NULL;
    xui_document_snapshot snapshot=NULL;
    xui_document_source_index index=NULL;
    xui_document_prepare prepare=NULL;
    xui_document_fragment fragment=NULL;
    xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN};
    int status=xuiDocumentCreateOwned(&desc,&document);
    if(status!=XUI_OK)goto done;
    xui_callback_owner_t owner={{&observer,&faultObserverOps},faultHold,faultDrop};uint64_t token=0;
    status=xuiDocumentSubscribeOwned(document,faultNotify,&observer,&owner,&token);if(status!=XUI_OK)goto done;
    status=xuiDocumentLoadMarkdown(document,"hello world\n",12);if(status!=XUI_OK)goto done;
    status=xuiDocumentAcquireSnapshot(document,&snapshot);if(status!=XUI_OK)goto done;
    status=xuiDocumentSnapshotCreateSourceIndex(snapshot,NULL,&index);if(status!=XUI_OK)goto done;
    xui_doc_source_patch_t patch={.iSize=sizeof(patch),.iStart=0,.iEnd=1,.sText="H",.iTextBytes=1};
    status=xuiDocumentPrepareSource(document,NULL,&patch,1,&prepare);if(status!=XUI_OK)goto done;
    status=xuiDocumentPrepareRun(prepare);if(status!=XUI_OK)goto done;
    status=xuiDocumentPreparePublish(document,prepare,NULL);if(status!=XUI_OK)goto done;
    status=xuiDocumentFragmentImportHtmlOwned("<p>hi</p>",9,&fragment);
done:
    if(fragment)assert(xuiDocumentFragmentReleaseChecked(fragment)==XUI_OK);
    if(prepare)assert(xuiDocumentPrepareReleaseChecked(prepare)==XUI_OK);
    if(index)assert(xuiDocumentSourceIndexReleaseChecked(index)==XUI_OK);
    if(snapshot)assert(xuiDocumentSnapshotReleaseChecked(snapshot)==XUI_OK);
    if(document)assert(xuiDocumentReleaseChecked(document)==XUI_OK);
    assert(observer.refs==1);
    return status;
}
static void faultEveryAllocation(void)
{
    tracking=true;failAt=SIZE_MAX;attempts=0;assert(modelWorkload()==XUI_OK);size_t total=attempts;assert(!live);tracking=false;
    for(size_t point=1;point<=total;++point){
        tracking=true;attempts=0;failAt=point;
        (void)modelWorkload();
        assert(attempts>=point && !live);tracking=false;xrtClearError();
    }
    printf("default allocator: %zu independently failed allocation sites, no leaked controls/payloads\n",total);
}
typedef struct SharedSnapshot {xui_document_snapshot snapshot;unsigned iterations;} SharedSnapshot;
static int32 sharedSnapshotThread(void* raw)
{
    const SharedSnapshot* work=raw;
    for(unsigned i=0;i<work->iterations;++i){
        xui_document_snapshot shared=NULL;assert(xuiDocumentSnapshotShareOwned(work->snapshot,&shared)==XUI_OK && shared==work->snapshot);
        assert(xuiDocumentSnapshotReleaseChecked(shared)==XUI_OK);
    }
    return 0;
}
static void concurrentOwners(void)
{
    xui_document document=NULL;xui_document_snapshot snapshot=NULL;
    assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    SharedSnapshot work={snapshot,12000};xthread* threads[4]={0};
    for(unsigned i=0;i<4;++i){threads[i]=xrtThreadCreate(sharedSnapshotThread,&work,0);assert(threads[i]);}
    for(unsigned i=0;i<4;++i){assert(xrtThreadWaitFor(threads[i],-1)==XWAIT_OK);xrtThreadDestroy(threads[i]);}
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));size_t count=0;xrtownershipref ref=xuiDocumentSnapshotOwnership(snapshot);
    assert(ref.Ops->Count(ref.Data,&count) && count==1 && xrtOwnershipScopeEnd(&freeze));
    assert(xuiDocumentSnapshotReleaseChecked(snapshot)==XUI_OK && xuiDocumentReleaseChecked(document)==XUI_OK);
    puts("four actual threads: 48,000 ShareOwned/Release pairs preserve the one external owner");
}
int main(void){modelHostBind();faultEveryAllocation();concurrentOwners();return 0;}
