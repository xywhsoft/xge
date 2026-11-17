/* Real 3D Scene body, native ownership and XRT allocator; no graphics device. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#if !defined(XGE_TEST_STATIC)
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;for(size_t i=0;i<sizeof(symbols)/sizeof(*symbols);++i)if(!strcmp(symbols[i].name,name))return symbols[i].proc;return NULL;}
#endif
static size_t references(xge3d_scene* scene)
{
 xrtownershipref ref=xge3dSceneOwnership(scene);size_t count=0;xrtownershipscope freeze={0};
 assert(ref.Data && xrtOwnershipFreezeTryBegin(&freeze));
 assert(ref.Ops->Count(ref.Data,&count) && xrtOwnershipScopeEnd(&freeze));return count;
}
static bool no_edges(xrtownershipref ref,void* user){(void)ref;(void)user;assert(!"unexpected owning edge");return false;}
static void release(xge3d_scene* scene)
{bool consumed=false;assert(xge3dSceneReleaseOwnerChecked(scene,&consumed)==XGE_OK && consumed);}
static xge3d_scene* create(void)
{xge3d_scene* s=NULL;assert(xge3dSceneCreateOwned(&s)==XGE_OK && s && references(s)==1);return s;}
static int32 wrong_thread(void* user)
{
 xge3d_scene* scene=user;bool consumed=true;xge3d_scene* output=(void*)(uintptr_t)1;
 assert(xge3dSceneAccessBegin(scene)==XGE_ERROR_BUSY);
 assert(xge3dSceneAccessEnd(scene)==XGE_ERROR_BUSY);
 assert(xge3dSceneShareOwned(scene,&output)==XGE_ERROR_BUSY && !output);
 assert(xge3dSceneReleaseOwnerChecked(scene,&consumed)==XGE_ERROR_BUSY && !consumed);
 assert(!xge3dSceneOwnershipReadReady(xge3dSceneOwnership(scene)));return 0;
}
static void join(xthread* thread)
{assert(thread && xrtThreadWaitFor(thread,10000)==XWAIT_OK && !xrtThreadExitCode(thread));xrtThreadDestroy(thread);}
static uint64_t ids[4][32];
static int32 many_scenes(void* user)
{
 size_t at=(size_t)(uintptr_t)user;
 for(size_t i=0;i<32;++i){
  xge3d_scene* s=NULL;assert(xge3dSceneCreateOwned(&s)==XGE_OK && s);
  assert(xge3dSceneAccessBegin(s)==XGE_OK);
  xge3d_node_t node;assert(xge3dNodeCreate(s,(xge3d_node_t){0},&node)==XGE_OK);
  ids[at][i]=node.scene;assert(xge3dSceneAccessEnd(s)==XGE_OK);release(s);
 }return 0;
}
static void lifecycle(void)
{
 xge3d_scene* out=(void*)(uintptr_t)1;bool consumed=true;
 assert(xge3dSceneCreateOwned(NULL)==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dSceneShareOwned(NULL,&out)==XGE_ERROR_INVALID_ARGUMENT && !out);
 assert(xge3dSceneAccessBegin(NULL)==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dSceneAccessEnd(NULL)==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dSceneReleaseOwnerChecked(NULL,&consumed)==XGE_ERROR_INVALID_ARGUMENT && !consumed);
 xge3d_scene* s=create();assert(xge3dSceneAccessEnd(s)==XGE_ERROR_INVALID_STATE);
 assert(xge3dSceneShareOwned(s,&out)==XGE_OK && out==s && references(s)==2);
 release(s);assert(references(out)==1);
 join(xrtThreadCreate(wrong_thread,out,0));assert(references(out)==1);release(out);
 s=create();assert(xge3dSceneAccessBegin(s)==XGE_OK && xge3dSceneAccessBegin(s)==XGE_OK && references(s)==3);
 xrtownershipref ref=xge3dSceneOwnership(s);
 const xrtownershipquiescencev1* quiet=xge3dSceneOwnershipQuiescenceV1(ref);assert(quiet && quiet->Busy(ref.Data));
 release(s);assert(references(s)==2);
 assert(xge3dSceneAccessEnd(s)==XGE_OK && references(s)==1);
 xge3d_node_t node;assert(xge3dNodeCreate(s,(xge3d_node_t){0},&node)==XGE_OK);
 assert(xge3dSceneAccessEnd(s)==XGE_OK); /* Last access owns the physical free. */
 s=NULL;assert(xge3dSceneCreate(&s)==XGE_OK && !xge3dSceneOwnership(s).Data);
 assert(xge3dSceneShareOwned(s,&out)==XGE_ERROR_INVALID_ARGUMENT && !out);
 assert(xge3dNodeCreate(s,(xge3d_node_t){0},&node)==XGE_OK);xge3dSceneFree(s);
}
static void ownership(void)
{
 xge3d_scene* s=create();xrtownershipref ref=xge3dSceneOwnership(s);
 const xrtownershipadapterv1* adapter=xge3dSceneOwnershipAdapterV1(ref);
 const xrtownershippreparationv1* prepare=xge3dSceneOwnershipPreparationV1(ref);
 const xrtownershipfinishv2* finish=xge3dSceneOwnershipCompletionV2(ref);
 const xrtownershipfinalizationv1* finalize=xge3dSceneOwnershipFinalizationV1(ref);
 assert(adapter && prepare && finish && finalize && prepare->Adapter==adapter && finish->V1.Adapter==adapter);
 xrtownershipscope freeze={0};int token;
 assert(xrtOwnershipFreezeTryBegin(&freeze) && ref.Ops->Trace(ref.Data,no_edges,NULL));
 assert(adapter->Claim(ref.Data,&token) && !xge3dSceneOwnershipReadReady(ref));
 assert(xrtOwnershipScopeEnd(&freeze));
 xge3d_scene* out=(void*)(uintptr_t)1;assert(xge3dSceneShareOwned(s,&out)==XGE_ERROR_INVALID_STATE && !out);
 assert(xge3dSceneAccessBegin(s)==XGE_ERROR_INVALID_STATE);
 assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Restore(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
 assert(xge3dSceneOwnershipReadReady(ref));
 assert(xge3dSceneAccessBegin(s)==XGE_OK);
 xge3d_node_t parent,child;assert(xge3dNodeCreate(s,(xge3d_node_t){0},&parent)==XGE_OK);
 assert(xge3dNodeCreate(s,parent,&child)==XGE_OK && xge3dNodeSetName(s,child,"owned child")==XGE_OK);
 assert(xge3dSceneAccessEnd(s)==XGE_OK);
 assert(xrtOwnershipFreezeTryBegin(&freeze) && adapter->Claim(ref.Data,&token) && xrtOwnershipScopeEnd(&freeze));
 bool complete=false;assert(finalize->CanStep(ref.Data,&token) && finalize->Step(ref.Data,&token,&complete) && complete);
 assert(!xge3dSceneOwnershipReadReady(ref));
 assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&token);assert(xrtOwnershipScopeEnd(&freeze));
 assert(adapter->Finish(ref.Data,&token));assert(finish->DropReady(ref.Data) && finish->DropChecked(ref.Data));
}
static void hierarchy(void)
{
 xge3d_scene* s=create();assert(xge3dSceneAccessBegin(s)==XGE_OK);
 xge3d_node_t root,child,parent,found;assert(xge3dNodeCreate(s,(xge3d_node_t){0},&root)==XGE_OK);
 assert(xge3dNodeCreate(s,root,&child)==XGE_OK && xge3dSceneNodeCount(s)==2);
 assert(xge3dNodeSetName(s,child,"child")==XGE_OK && !strcmp(xge3dNodeName(s,child),"child"));
 assert(xge3dNodeGetParent(s,child,&parent)==XGE_OK && parent.slot==root.slot);
 assert(xge3dNodeChildCount(s,root)==1 && xge3dNodeChildAt(s,root,0).slot==child.slot);
 assert(xge3dNodeSetParent(s,root,child)==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dNodeFind(s,root,"child",&found)==XGE_OK && found.slot==child.slot);
 assert(xge3dNodeFind(s,root,"missing",&found)==XGE_ERROR_NOT_FOUND && !found.slot && !found.scene);
 xge3d_transform_t transform=XGE3D_TRANSFORM_IDENTITY;transform.position=(xge3d_vec3_t){2,3,4};
 assert(xge3dNodeSetTransform(s,root,&transform)==XGE_OK);
 transform.position=(xge3d_vec3_t){5,6,7};assert(xge3dNodeSetTransform(s,child,&transform)==XGE_OK);
 xge3d_mat4_t world;assert(xge3dNodeGetWorldMatrix(s,child,&world)==XGE_OK && world.m[12]==7 && world.m[13]==9 && world.m[14]==11);
 assert(xge3dSceneSetOrigin(s,(xge3d_dvec3_t){1e12,0,0})==XGE_OK);
 assert(xge3dNodeSetGlobalPosition(s,root,(xge3d_dvec3_t){1e12+.25,0,0})==XGE_OK);
 assert(xge3dNodeGetLocalMatrix(s,root,&world)==XGE_OK && world.m[12]==.25f);
 assert(xge3dSceneSetOrigin(s,(xge3d_dvec3_t){NAN,0,0})==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dNodeSetLayers(s,child,1,2,4)==XGE_OK && xge3dNodeSetVisible(s,child,0)==XGE_OK);
 assert(xge3dNodeDestroy(s,root)==XGE_OK && !xge3dSceneNodeCount(s));
 assert(xge3dNodeSetName(s,child,"stale")==XGE_ERROR_INVALID_ARGUMENT);
 assert(xge3dSceneAccessEnd(s)==XGE_OK);release(s);
}
static void failure_and_cost(void)
{
 xmemdebugsnapshot ca,cb;size_t legacy_bytes,owned_bytes;
 xrtMemDebugSnapshot(&ca);xge3d_scene* measured=NULL;
 assert(xge3dSceneCreate(&measured)==XGE_OK);xrtMemDebugSnapshot(&cb);
 assert(cb.AllocCount-ca.AllocCount==1);legacy_bytes=cb.LiveBytes-ca.LiveBytes;xge3dSceneFree(measured);
 xrtMemDebugSnapshot(&ca);measured=create();xrtMemDebugSnapshot(&cb);
 assert(cb.AllocCount-ca.AllocCount==1);owned_bytes=cb.LiveBytes-ca.LiveBytes;release(measured);
 if(sizeof(void*)==8)assert(owned_bytes==legacy_bytes+32);
 printf("scene native cold: legacy=%zu owned=%zu allocations=1\n",legacy_bytes,owned_bytes);
 xge3d_scene* s=(void*)(uintptr_t)1;assert(xrtMemDebugFailAfter(0));
 assert(xge3dSceneCreateOwned(&s)==XGE_ERROR_OUT_OF_MEMORY && !s && xrtMemDebugFailTriggered());
 xrtMemDebugFailClear();xrtClearError();
 for(int budget=0;budget<12;++budget){
  s=create();assert(xge3dSceneAccessBegin(s)==XGE_OK);xge3d_node_t node;
  assert(xrtMemDebugFailAfter(budget));int status=xge3dNodeCreate(s,(xge3d_node_t){0},&node);
  xrtMemDebugFailClear();xrtClearError();assert(status==XGE_OK || status==XGE_ERROR_OUT_OF_MEMORY);
  assert((status==XGE_OK && xge3dSceneNodeCount(s)==1) || (!node.slot && !xge3dSceneNodeCount(s)));
  assert(xge3dSceneAccessEnd(s)==XGE_OK);release(s);
 }
 s=create();assert(xge3dSceneAccessBegin(s)==XGE_OK);xge3d_node_t node;
 assert(xge3dNodeCreate(s,(xge3d_node_t){0},&node)==XGE_OK && xge3dNodeSetName(s,node,"original")==XGE_OK);
 assert(xrtMemDebugFailAfter(0) && xge3dNodeSetName(s,node,"replacement")==XGE_ERROR_OUT_OF_MEMORY);
 xrtMemDebugFailClear();xrtClearError();assert(!strcmp(xge3dNodeName(s,node),"original"));
 assert(xge3dSceneAccessEnd(s)==XGE_OK);
 xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
 for(unsigned i=0;i<1000;++i){assert(xge3dSceneAccessBegin(s)==XGE_OK);assert(xge3dSceneNodeCount(s)==1);assert(xge3dSceneAccessEnd(s)==XGE_OK);}
 xrtMemDebugSnapshot(&b);assert(a.AllocCount==b.AllocCount);release(s);
 puts("scene native warm: calls=1000 allocations=0");
 xthread* threads[4];for(size_t i=0;i<4;++i)threads[i]=xrtThreadCreate(many_scenes,(void*)(uintptr_t)i,0);
 for(unsigned i=0;i<4;++i)join(threads[i]);
 for(unsigned a1=0;a1<128;++a1)for(unsigned b1=a1+1;b1<128;++b1)assert(ids[a1/32][a1%32]!=ids[b1/32][b1%32]);
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 static const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
 assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 assert(xrtMemDebugEnable(true));xrtClearError();xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 lifecycle();ownership();hierarchy();failure_and_cost();
 xrtClearError();xrtMemDebugSnapshot(&after);
 assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
 assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
 puts("XGE3D_SCENE_OWNED_NATIVE_OK");return 0;
}
