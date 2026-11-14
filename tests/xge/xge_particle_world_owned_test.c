/* Exact native world, CPU only. This test has no language/graphics wrapper. */
#define XRT_MODULE_ALL
#if defined(_WIN32) && !defined(XRT_USE_SHARED)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include "src/xge/particle/xge_particle_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#if defined(PARTICLE_SDK_HOST)
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#elif defined(PARTICLE_SDK_STATIC)
#include <xge_static_xrt_contract.h>
#endif

/* Reuse every existing kernel assertion in managed mode whenever its descriptor
 * has no caller-borrowed collision pointer. Raw collision tests remain raw;
 * they are not evidence for the unfinished owned collision registration. */
#if !defined(PARTICLE_RAW_KERNEL)
static int test_create(xge_particle_world* out,const xge_particle_world_desc_t* desc)
{
 return desc && (desc->pCollision || desc->pCollisionUser) ?
  xgeParticleWorldCreate(out,desc):xgeParticleWorldCreateOwned(out,desc);
}
#define xgeParticleWorldCreate test_create
#endif
#define main particle_kernel_main
#include "test_particle.c"
#undef main
#undef xgeParticleWorldCreate

typedef struct WorldOwner {size_t refs;unsigned holds,drops;bool failHold,failDrop,diagnostic,dropDiagnostic;xge_particle_world world;
 xge_particle_world_desc_t* editDesc;xge_callback_owner_t* editCode;
} WorldOwner;
static bool owner_count(const void* raw,size_t* out){*out=((const WorldOwner*)raw)->refs;return true;}
static bool owner_trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops world_owner_ops={owner_count,owner_trace};
static bool owner_hold(const void* raw)
{
 WorldOwner* owner=(WorldOwner*)raw;++owner->holds;
 if(owner->diagnostic)xrtSetErrorKind(XERR_VALUE);
 if(owner->failHold)return false;
 ++owner->refs;
 if(owner->editDesc)owner->editDesc->pCollisionUser=owner;
 if(owner->editCode)*owner->editCode=(xge_callback_owner_t){0};
 return true;
}
static bool owner_drop(const void* raw)
{
 WorldOwner* owner=(WorldOwner*)raw;assert(owner->refs>1);--owner->refs;++owner->drops;
 if(owner->diagnostic || owner->dropDiagnostic)xrtSetErrorKind(XERR_VALUE);
 return !owner->failDrop;
}
static xge_callback_owner_t code_owner(WorldOwner* owner)
{return (xge_callback_owner_t){{owner,&world_owner_ops},owner_hold,owner_drop};}
static size_t world_refs(xge_particle_world world)
{xrtownershipref ref=xgeParticleWorldOwnership(world);size_t count=0;assert(ref.Ops && ref.Ops->Count(ref.Data,&count));return count;}
static bool see_owner(xrtownershipref ref,void* user)
{assert(ref.Data==user && ref.Ops==&world_owner_ops);return true;}
static xge_particle_world_desc_t small_desc(void)
{
 xge_particle_world_desc_t desc;xgeParticleWorldDescInit(&desc);
 desc.iMaxEffects=4;desc.iMaxParticles=16;desc.iMaxEvents=8;desc.iMaxSpawnPerStep=8;desc.iMaxSubsteps=4;return desc;
}
static void test_constructor_snapshot(void)
{
 xge_particle_world_desc_t desc=small_desc();WorldOwner owner={.refs=1};xge_callback_owner_t code=code_owner(&owner);
 owner.editDesc=&desc;owner.editCode=&code;xge_particle_world world=NULL;
 assert(xgeParticleWorldCreateOwnedWithCode(&world,&desc,&code)==XGE_OK && world);
 assert(world->desc.pCollisionUser==NULL && "Owned constructor input snapshot");
 assert(owner.refs==2 && !code.onDrop && desc.pCollisionUser==&owner);
 assert(xgeParticleWorldFreeChecked(world)==XGE_OK && owner.refs==1 && owner.drops==1);
 puts("ParticleWorld constructor input snapshot passed");
}
static int32 owned_wrong_thread(void* raw)
{
 xge_particle_world world=raw,other=(xge_particle_world)(uintptr_t)1;bool consumed=true;
 assert(xgeParticleUpdate(world,0)==XGE_ERROR_BUSY);
 assert(xgeParticleWorldShareOwned(world,&other)==XGE_ERROR_BUSY && !other);
 assert(xgeParticleWorldReleaseOwnerChecked(world,&consumed)==XGE_ERROR_BUSY && !consumed);
 xge_particle_event_t event={0};event.iType=77;
 assert(xgeParticleEventPoll(world,&event)==XGE_ERROR_BUSY && event.iType==77);
 assert(xgeParticleStats(world).iActiveEffects==0 && xrtGetError());xrtClearError();return 0;
}
static int32 raw_move_thread(void* raw)
{assert(xgeParticleUpdate((xge_particle_world)raw,0)==XGE_OK);return 0;}
static int release_during_visit(const xge_particle_snapshot_t* particle,void* raw)
{
 WorldOwner* owner=raw;assert(particle && owner->world && world_refs(owner->world)==2);
 xrtownershipref ref=xgeParticleWorldOwnership(owner->world);int token=0;
 const xrtownershipadapterv1* adapter=xgeParticleWorldOwnershipAdapterV1(ref);
 const xrtownershipquiescencev1* quiet=xgeParticleWorldOwnershipQuiescenceV1(ref);
 assert(quiet && quiet->Busy(ref.Data) && !adapter->Claim(ref.Data,&token));
 bool consumed=false;assert(xgeParticleWorldReleaseOwnerChecked(owner->world,&consumed)==XGE_OK && consumed);
 assert(owner->refs==2 && world_refs(owner->world)==1);
 assert(xgeParticleStats(owner->world).iLiveParticles==1);
 owner->world=NULL;return 0;
}
static void test_managed_lifetime(void)
{
 xge_particle_world_desc_t desc=small_desc();WorldOwner owner={.refs=1};xge_callback_owner_t code=code_owner(&owner);
 xge_particle_world world=NULL,other=NULL;
 assert(xgeParticleWorldCreateOwnedWithCode(&world,&desc,&code)==XGE_OK && owner.refs==2 && world_refs(world)==1);
 xrtownershipref ref=xgeParticleWorldOwnership(world);assert(ref.Ops->Trace(ref.Data,see_owner,&owner));
 assert(xgeParticleWorldShareOwned(world,&other)==XGE_OK && other==world && world_refs(world)==2);
 bool consumed=false;assert(xgeParticleWorldReleaseOwnerChecked(other,&consumed)==XGE_OK && consumed && world_refs(world)==1);
 xthread* thread=xrtThreadCreate(owned_wrong_thread,world,0);assert(thread && xrtThreadWait(thread)==XWAIT_OK);
 xrtThreadDestroy(thread);assert(world_refs(world)==1);
 xge_particle_emitter_t emitter;xgeParticleEmitterInit(&emitter);emitter.fRate=0;emitter.iMaxParticles=8;
 xge_particle_definition definition=NULL;assert(xgeParticleDefinitionCreate(&definition,&emitter,1)==XGE_OK);
 xge_particle_effect effect=0;assert(xgeParticlePlay(world,definition,NULL,&effect)==XGE_OK);
 assert(xgeParticleState(world,effect)==XGE_PARTICLE_PLAYING);
 uint32_t spawned=0;assert(xgeParticleEmit(world,effect,0,1,&spawned)==XGE_OK && spawned==1);
 xge_particle_event_t event;assert(xgeParticleEventPoll(world,&event)==1);
 owner.world=world;assert(xgeParticleVisit(world,release_during_visit,&owner)==XGE_OK);
 assert(!owner.world && owner.refs==1 && owner.drops==1);
 xgeParticleDefinitionFree(definition);
 assert(xgeParticleWorldCreate(&world,&desc)==XGE_OK && !xgeParticleWorldOwnership(world).Data);
 thread=xrtThreadCreate(raw_move_thread,world,0);assert(thread && xrtThreadWait(thread)==XWAIT_OK);
 xrtThreadDestroy(thread);assert(xgeParticleWorldFreeChecked(world)==XGE_OK);
}
static void test_world_graph(void)
{
 for(int fail=0;fail<2;++fail){
  WorldOwner owner={.refs=1};xge_callback_owner_t code=code_owner(&owner);xge_particle_world world=NULL;
  xge_particle_world_desc_t desc=small_desc();assert(xgeParticleWorldCreateOwnedWithCode(&world,&desc,&code)==XGE_OK);
  xrtownershipref ref=xgeParticleWorldOwnership(world);int token=0;size_t count=0;
  const xrtownershipadapterv1* adapter=xgeParticleWorldOwnershipAdapterV1(ref);
  const xrtownershippreparationv1* prepare=xgeParticleWorldOwnershipPreparationV1(ref);
  const xrtownershipfinalizationv1* finalize=xgeParticleWorldOwnershipFinalizationV1(ref);
  const xrtownershipfinishv2* finish=xgeParticleWorldOwnershipCompletionV2(ref);
  assert(adapter && prepare && finalize && finish && adapter->Claim(ref.Data,&token));
  assert(prepare->Prepare(ref.Data,&token)==XRT_OWNERSHIP_PREPARE_READY);
  assert(xgeParticleUpdate(world,0)==XGE_ERROR_INVALID_STATE);
  adapter->Restore(ref.Data,&token);assert(xgeParticleUpdate(world,0)==XGE_OK);
  assert(adapter->Claim(ref.Data,&token));
  bool complete=false;assert(finalize->CanStep(ref.Data,&token) && finalize->Step(ref.Data,&token,&complete) && complete);
  assert(owner.refs==2 && !xgeParticleWorldOwnershipReadReady(ref));
  adapter->Clear(ref.Data,&token);owner.failDrop=fail!=0;
  assert(finish->V1.Step(ref.Data,&token)==(fail ? XRT_OWNERSHIP_FINISH_FAILED:XRT_OWNERSHIP_FINISH_READY));
  assert(!xrtGetError() && owner.refs==1 && owner.drops==1);
  assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY && owner.drops==1);
  assert(ref.Ops->Count(ref.Data,&count) && count==1 && finish->DropReady(ref.Data));
  assert(finish->DropChecked(ref.Data));
 }
}
static void test_world_failures(void)
{
 xge_particle_world_desc_t desc=small_desc();int failures=0,successes=0;
 for(int budget=0;budget<24;++budget){
  WorldOwner owner={.refs=1};xge_callback_owner_t code=code_owner(&owner);
  xmemdebugsnapshot before,after;xrtClearError();xrtMemDebugSnapshot(&before);
  assert(xrtMemDebugFailAfter(budget));xge_particle_world world=(xge_particle_world)(uintptr_t)1;
  int status=xgeParticleWorldCreateOwnedWithCode(&world,&desc,&code);xrtMemDebugFailClear();xrtClearError();
  if(status==XGE_OK){assert(world && owner.refs==2);assert(xgeParticleWorldFreeChecked(world)==XGE_OK);++successes;}
  else {assert(!world);++failures;}
  assert(owner.refs==1 && owner.holds==1 && owner.drops==1);xrtMemDebugSnapshot(&after);
  assert(before.LiveBytes==after.LiveBytes && before.LiveCount==after.LiveCount);
 }
 assert(failures && successes);
 for(int mode=0;mode<5;++mode){
  WorldOwner owner={.refs=1,.failHold=mode<2,.diagnostic=mode==1 || mode==4};xge_callback_owner_t code=code_owner(&owner);
  xge_particle_world world=NULL;int status=xgeParticleWorldCreateOwnedWithCode(&world,&desc,&code);
  if(mode<2 || mode==4){assert(status==XGE_ERROR_CALLBACK_FAILED && !world && owner.refs==1 && owner.drops==(mode==4 ? 1u:0u));xrtClearError();}
  else {assert(status==XGE_OK && world);owner.failDrop=true;owner.diagnostic=mode==3;
   bool consumed=false;assert(xgeParticleWorldReleaseOwnerChecked(world,&consumed)==XGE_ERROR_CALLBACK_FAILED && consumed);
   assert(owner.refs==1 && owner.drops==1 && (xrtGetError()!=NULL)==owner.diagnostic);xrtClearError();}
 }
 WorldOwner owner={.refs=1};xge_callback_owner_t code=code_owner(&owner);xge_particle_world legacy=NULL;
 assert(xgeParticleWorldCreateOwnedWithCode(&legacy,&desc,&code)==XGE_OK);owner.failDrop=true;
 xgeParticleWorldFree(legacy);assert(xrtGetError() && owner.refs==1 && owner.drops==1);xrtClearError();
 WorldOwner primary={.refs=1,.failDrop=true,.dropDiagnostic=true};xge_callback_owner_t badCode=code_owner(&primary);
 xge_particle_world_desc_t badDesc=desc;badDesc.iMaxEffects=0;
 assert(xgeParticleWorldCreateOwnedWithCode(&legacy,&badDesc,&badCode)==XGE_ERROR_INVALID_ARGUMENT && !legacy);
 assert(!xrtGetError() && primary.refs==1 && primary.drops==1);
 desc.pCollisionUser=&desc;xge_particle_world world=(xge_particle_world)(uintptr_t)1;
 assert(xgeParticleWorldCreateOwned(&world,&desc)==XGE_ERROR_INVALID_ARGUMENT && !world);
}
static void test_world_hot(void)
{
 xge_particle_world_desc_t desc=small_desc();
 for(int managed=0;managed<2;++managed){
  xge_particle_world world=NULL;
  xmemdebugsnapshot coldBefore,coldAfter;xrtMemDebugSnapshot(&coldBefore);
  assert((managed ? xgeParticleWorldCreateOwned(&world,&desc):xgeParticleWorldCreate(&world,&desc))==XGE_OK);
  xrtMemDebugSnapshot(&coldAfter);
  printf("ParticleWorld %s create: allocations=%llu liveBytes=%zu\n",managed?"managed":"raw",
   (unsigned long long)(coldAfter.AllocCount-coldBefore.AllocCount),coldAfter.LiveBytes-coldBefore.LiveBytes);
  xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
  for(int i=0;i<1000;++i){assert(xgeParticleUpdate(world,0)==XGE_OK);assert(xgeParticleStats(world).iLiveParticles==0);}
  xrtMemDebugSnapshot(&after);
  assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
  assert(xgeParticleWorldFreeChecked(world)==XGE_OK);
  printf("ParticleWorld %s prepared update/stats: 1000 pairs, zero heap operations\n",managed?"managed":"raw");
 }
}
#ifndef PARTICLE_WORLD_MAIN
#define PARTICLE_WORLD_MAIN main
#endif
int PARTICLE_WORLD_MAIN(int argc,char** argv)
{
#if defined(PARTICLE_SDK_HOST)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#elif defined(PARTICLE_SDK_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
 assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#endif
 if(argc==2 && !strcmp(argv[1],"--snapshot-only")){test_constructor_snapshot();return 0;}
 assert(particle_kernel_main()==0);assert(xrtMemDebugEnable(true));
 test_constructor_snapshot();test_managed_lifetime();test_world_graph();test_world_failures();test_world_hot();
 xmemdebugsnapshot after;xrtClearError();xrtMemDebugSnapshot(&after);
 assert(!after.LiveCount && !after.DoubleFreeCount && !after.InvalidFreeCount);
 printf("ParticleWorld ownership: native shared refs, active release, graph, checked failures and thread admission passed; rawBytes=%zu managedBytes=%zu\n",sizeof(struct xge_particle_world_t),sizeof(xp_world_owned_t));
 return 0;
}
