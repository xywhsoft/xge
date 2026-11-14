/* Real owned world and existing kernel; no graphics, transport or language shim. */
#define PARTICLE_WORLD_MAIN prior_world_main
#include "xge_particle_world_owned_test.c"
#undef PARTICLE_WORLD_MAIN

typedef struct CollisionOwner {
 size_t refs;unsigned holds,drops,calls;int result,failHolds,failDrops;
 bool bodyError,dropError,closeInBody,closeInHold,reenter,invalidHit;
 const xerror* firstError;xge_particle_world world;xge_callback_owner_t* editDescriptor;
} CollisionOwner;
static bool collision_count(const void* p,size_t* out){*out=((const CollisionOwner*)p)->refs;return true;}
static bool collision_trace(const void* p,xrtownershipvisitor visit,void* user){(void)p;(void)visit;(void)user;return true;}
static const xrtownershipops collision_ops={collision_count,collision_trace};
static bool collision_hold(const void* p)
{
 CollisionOwner* owner=(CollisionOwner*)p;++owner->holds;
 if(owner->failHolds){--owner->failHolds;return false;}
 ++owner->refs;
 if(owner->editDescriptor)*owner->editDescriptor=(xge_callback_owner_t){0};
 if(owner->closeInHold){bool consumed=false;assert(xgeParticleWorldReleaseOwnerChecked(owner->world,&consumed)==XGE_OK && consumed);owner->world=NULL;}
 return true;
}
static bool collision_drop(const void* p)
{
 CollisionOwner* owner=(CollisionOwner*)p;assert(owner->refs>1);--owner->refs;++owner->drops;
 if(owner->dropError)xrtSetErrorKind(XERR_VALUE);
 if(owner->failDrops){--owner->failDrops;return false;}
 return true;
}
static xge_callback_owner_t collision_descriptor(CollisionOwner* owner)
{return (xge_callback_owner_t){{owner,&collision_ops},collision_hold,collision_drop};}
static int collided(xge_vec2_t from,xge_vec2_t to,float radius,xge_particle_hit_t* hit,void* p)
{
 CollisionOwner* owner=p;(void)from;(void)radius;++owner->calls;
 assert(owner->refs==3); /* C test owner + actual registration + invocation snapshot. */
 if(owner->world){
  assert(world_refs(owner->world)==2);
  if(owner->reenter){
   assert(xgeParticleWorldSetCollisionOwned(owner->world,NULL,NULL,NULL)==XGE_ERROR_BUSY);
   assert(xgeParticleUpdate(owner->world,0)==XGE_ERROR_INVALID_STATE);
  }
  if(owner->closeInBody){bool consumed=false;assert(xgeParticleWorldReleaseOwnerChecked(owner->world,&consumed)==XGE_OK && consumed);owner->world=NULL;}
 }
 hit->tPosition=to;hit->tNormal=(xge_vec2_t){0,owner->invalidHit ? 0:-1};hit->fFraction=0.5f;
 if(owner->bodyError){xrtSetErrorKind(XERR_STATE);owner->firstError=xrtGetError();}
 return owner->result;
}
static xge_particle_world collision_world(xge_particle_definition* definition,xge_particle_effect* effect,
 const xge_callback_owner_t* code)
{
 xge_particle_world_desc_t desc=small_desc();desc.fFixedStep=1.0f/64;desc.iMaxSubsteps=8;
 xge_particle_world world=NULL;assert(xgeParticleWorldCreateOwnedWithCode(&world,&desc,code)==XGE_OK);
 xge_particle_emitter_t e;xgeParticleEmitterInit(&e);e.bAutomatic=0;e.fRate=0;e.iMaxParticles=4;
 e.iCollision=XGE_PARTICLE_COLLISION_KILL;e.tLife=(xge_particle_range_t){3600,3600};
 assert(xgeParticleDefinitionCreate(definition,&e,1)==XGE_OK);
 assert(xgeParticlePlay(world,*definition,NULL,effect)==XGE_OK);
 assert(xgeParticleEmit(world,*effect,0,1,NULL)==XGE_OK);return world;
}
static void test_collision_results(void)
{
 for(int mode=0;mode<8;++mode){
  xge_particle_definition definition=NULL;xge_particle_effect effect=0;
  xge_particle_world world=collision_world(&definition,&effect,NULL);
  CollisionOwner owner={.refs=1,.result=mode==0 ? 0:mode==1 ? 1:mode==2 ? 2:mode==3 || mode==4 ? -71:1,.world=world,.reenter=true};
  xge_callback_owner_t desc=collision_descriptor(&owner);
  assert(xgeParticleWorldSetCollisionOwned(world,collided,&owner,&desc)==XGE_OK && owner.refs==2);
  owner.bodyError=mode==4;owner.dropError=mode==3 || mode==4;owner.failDrops=mode==3 || mode==4 || mode==6;
  owner.failHolds=mode==7;owner.invalidHit=mode==5;
  int expected=mode<=1 ? XGE_OK:mode==3 || mode==4 ? -71:mode==5 ? XGE_ERROR_INVALID_ARGUMENT:XGE_ERROR_CALLBACK_FAILED;
  int status=xgeParticleUpdate(world,3.0/64);assert(status==expected);
  if(mode==4)assert(xrtGetError()==owner.firstError);
  else assert(!xrtGetError());
  xrtClearError();owner.dropError=false;
  assert(owner.refs==2 && owner.calls==(mode==7 ? 0u:mode==0 ? 3u:1u));
  assert(!world->busy && !world->suppressEvents);
  xge_particle_stats_t stats=xgeParticleStats(world);
  if(mode>=2){assert(stats.iSteps==0 && stats.fDroppedTime==2.0/64);}
  assert(stats.iCollisions==(mode==1 ? 1u:0u));
  assert(xgeParticleUpdate(world,0)==XGE_OK); /* Failed queued time is not replayed. */
  assert(xgeParticleWorldSetCollisionOwned(world,NULL,NULL,NULL)==XGE_OK && owner.refs==1);
  assert(xgeParticleRestart(world,effect,7)==XGE_OK);
  assert(xgeParticleEmit(world,effect,0,1,NULL)==XGE_OK && xgeParticleUpdate(world,1.0/64)==XGE_OK);
  assert(xgeParticleWorldFreeChecked(world)==XGE_OK);xgeParticleDefinitionFree(definition);
 }
}
static void test_collision_replacement(void)
{
 xge_particle_definition definition=NULL;xge_particle_effect effect=0;
 xge_particle_world world=collision_world(&definition,&effect,NULL);
 CollisionOwner old={.refs=1,.world=world},next={.refs=1,.world=world};
 xge_callback_owner_t a=collision_descriptor(&old),bdesc=collision_descriptor(&next);
 assert(xgeParticleWorldSetCollisionOwned(world,collided,&old,&a)==XGE_OK);
 next.failHolds=1;assert(xgeParticleWorldSetCollisionOwned(world,collided,&next,&bdesc)==XGE_ERROR_CALLBACK_FAILED);
 assert(old.refs==2 && next.refs==1 && xgeParticleUpdate(world,1.0/64)==XGE_OK && old.calls==1);
 old.failDrops=1;next.editDescriptor=&bdesc;
 assert(xgeParticleWorldSetCollisionOwned(world,collided,&next,&bdesc)==XGE_ERROR_CALLBACK_FAILED);
 assert(old.refs==1 && next.refs==2 && !bdesc.onDrop); /* New publication committed despite old Drop failure. */
 assert(xgeParticleUpdate(world,1.0/64)==XGE_OK && next.calls==1);
 assert(xgeParticleWorldSetCollisionOwned(world,NULL,NULL,NULL)==XGE_OK && next.refs==1);
 xgeParticleWorldFree(world);xgeParticleDefinitionFree(definition);
}
static void test_collision_close_and_prewarm(void)
{
 for(int mode=0;mode<5;++mode){
  WorldOwner code={.refs=1};xge_callback_owner_t ownCode=code_owner(&code);
  xge_particle_definition definition=NULL;xge_particle_effect effect=0;
  xge_particle_world world=collision_world(&definition,&effect,&ownCode);
  CollisionOwner owner={.refs=1,.world=world};xge_callback_owner_t desc=collision_descriptor(&owner);
  if(mode==0)owner.closeInHold=true;
  assert(xgeParticleWorldSetCollisionOwned(world,collided,&owner,&desc)==XGE_OK);
  if(mode==0){assert(!owner.world && owner.refs==1 && code.refs==1);}
  else {
   owner.result=mode==1 || mode==2 ? -73:0;owner.closeInBody=mode>=3;
   if(mode==4){owner.failDrops=1;code.failDrop=true;code.diagnostic=true;}
   int status=mode==1 || mode==3 ? xgeParticlePrewarm(world,effect,3.0f/64):xgeParticleUpdate(world,3.0/64);
   assert(status==(mode==1 || mode==2 ? -73:mode==4 ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
   assert(!xrtGetError());
   if(mode>=3)assert(!owner.world && owner.refs==1 && code.refs==1);
   else {assert(!world->busy && !world->suppressEvents);assert(xgeParticleWorldFreeChecked(world)==XGE_OK && owner.refs==1 && code.refs==1);}
  }
  xgeParticleDefinitionFree(definition);
 }
}
static bool trace_collision_owner(xrtownershipref ref,void* raw)
{assert(ref.Data==raw && ref.Ops==&collision_ops);return true;}
static void test_collision_graph_and_hot(void)
{
 xge_particle_definition definition=NULL;xge_particle_effect effect=0;
 xge_particle_world world=collision_world(&definition,&effect,NULL);
 CollisionOwner owner={.refs=1,.world=world};xge_callback_owner_t desc=collision_descriptor(&owner);
 assert(xgeParticleWorldSetCollisionOwned(world,collided,&owner,&desc)==XGE_OK);
 xrtownershipref ref=xgeParticleWorldOwnership(world);assert(ref.Ops->Trace(ref.Data,trace_collision_owner,&owner));
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
 for(int i=0;i<1000;++i)assert(xgeParticleUpdate(world,1.0/64)==XGE_OK);
 xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
 assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
 assert(owner.calls==1000 && owner.holds==1001 && owner.drops==1000 && owner.refs==2);
 const xrtownershipadapterv1* adapter=xgeParticleWorldOwnershipAdapterV1(ref);
 const xrtownershipfinalizationv1* final=xgeParticleWorldOwnershipFinalizationV1(ref);
 const xrtownershipfinishv2* finish=xgeParticleWorldOwnershipCompletionV2(ref);int token=0;bool complete=false;
 assert(adapter->Claim(ref.Data,&token));owner.failDrops=1;
 assert(!final->Step(ref.Data,&token,&complete) && complete && owner.refs==1 && !xrtGetError());
 complete=false;assert(final->Step(ref.Data,&token,&complete) && complete && owner.drops==1001);
 adapter->Clear(ref.Data,&token);assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY);
 assert(finish->DropChecked(ref.Data));xgeParticleDefinitionFree(definition);
 puts("Particle collision owned: 1000 callbacks, independent owner snapshots, zero heap operations");
}
int main(int argc,char** argv)
{
 assert(prior_world_main(argc,argv)==0);
 test_collision_results();test_collision_replacement();test_collision_close_and_prewarm();test_collision_graph_and_hot();
 xrtClearError();xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);assert(!after.LiveCount && !after.DoubleFreeCount && !after.InvalidFreeCount);
 puts("Particle collision ownership: results, first failure, replacement, close, prewarm, graph and hot path passed");return 0;
}
