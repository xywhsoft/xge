/* Creator-thread/shared native lifetime, instantiated over the actual world.
 * No per-operation dispatcher, shadow simulation or forwarding object. */
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
static void xp_world_contents_free(xge_particle_world);
static int xp_world_create(xge_particle_world*,const xge_particle_world_desc_t*,bool);
static xp_world_owned_t* xpWorldStorage(xge_particle_world world)
{return (xp_world_owned_t*)world;}
static xge_data_lifetime_t* xpWorldRoot(xge_particle_world world)
{return world && world->managed ? &xpWorldStorage(world)->lifetime:NULL;}
static bool xpWorldObjectReadable(const void* raw)
{return !((const struct xge_particle_world_t*)raw)->destroyed;}
static bool xpWorldObjectTrace(const void* raw,xrtownershipvisitor visit,void* user)
{
 const xp_world_owned_t* world=raw;
 return !world->codeOwner.onDrop || visit(world->codeOwner.tReference,user);
}
static int xpWorldObjectFinalize(void* raw,bool* complete)
{
 xge_particle_world world=raw;world->destroyed=true;
 xp_world_contents_free(world);*complete=true;return XGE_OK;
}
static int xpWorldOwnerDrop(xge_callback_owner_t owner)
{
 if(!owner.onDrop)return XGE_OK;
 bool ok=owner.onDrop(owner.tReference.Data);
 return ok && !xrtGetError() ? XGE_OK:XGE_ERROR_CALLBACK_FAILED;
}
static int xpWorldObjectFinish(void* raw)
{
 xp_world_owned_t* world=raw;xge_callback_owner_t owner=world->codeOwner;
 world->codeOwner=(xge_callback_owner_t){0};return xpWorldOwnerDrop(owner);
}
static const xge_data_lifetime_type_t xpWorldType={
 offsetof(xp_world_owned_t,lifetime),offsetof(xp_world_owned_t,refs),
 xpWorldObjectReadable,xpWorldObjectTrace,xpWorldObjectFinalize,xpWorldObjectFinish};
#define DO_FN(name) xpWorld##name
#define DO_PUBLIC(name) xgeParticleWorld##name
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(world) xpWorldRoot(world)
#define DO_HANDLE xge_particle_world
#define DO_OBJECT_TYPE xpWorldType
#define DO_GENERATION_TYPE xpWorldType
#include "../core/xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE

typedef struct xp_world_frame {
 xge_data_lifetime_t* root;
 xerror* ambient;
} xp_world_frame;
static int xp_world_enter(xge_particle_world world,xp_world_frame* frame)
{
 *frame=(xp_world_frame){0};
 if(!world || !world->managed)return XGE_OK;
 xerror* ambient=xrtTakeError();xge_data_lifetime_t* root=xpWorldRoot(world);
 int status=xpWorldAcquire(root,true,true);
 if(status!=XGE_OK){xge_data_outcome_t result={0};xpWorldOutcomeBegin(&result,status);return xpWorldOutcomeEnd(&result,ambient);}
 frame->root=root;frame->ambient=ambient;return XGE_OK;
}
static int xp_world_end(xp_world_frame* frame,int status)
{
 if(!frame->root)return status;
 /* State and EventPoll have nonnegative value results, not status codes. */
 int value=status;
 xge_data_outcome_t result={0};xpWorldOutcomeBegin(&result,status<0 ? status:XGE_OK);
 xpWorldOutcomeAdd(&result,xpWorldActivityEnd(frame->root));
 /* The complete world body may now be physically gone. */
 status=xpWorldOutcomeEnd(&result,frame->ambient);
 return status==XGE_OK ? value:status;
}
#define XP_WORLD_ENTER(world) \
 xp_world_frame world_frame={0};int world_admission=xp_world_enter(world,&world_frame); \
 if(world_admission!=XGE_OK)return world_admission
#define XP_WORLD_RETURN(value) return xp_world_end(&world_frame,(value))

static void xp_world_lifetime_init(xge_particle_world world)
{
 if(world->managed){
  xp_world_owned_t* body=xpWorldStorage(world);atomic_init(&body->refs,1u);
  xpWorldInit(&body->lifetime,&xpWorldType,xrtThreadCurrentId());
 }
}
int xgeParticleWorldCreateOwned(xge_particle_world* out,const xge_particle_world_desc_t* desc)
{return xgeParticleWorldCreateOwnedWithCode(out,desc,NULL);}
int xgeParticleWorldCreateOwnedWithCode(xge_particle_world* out,const xge_particle_world_desc_t* desc,
 const xge_callback_owner_t* code)
{
 if(!out)return XGE_ERROR_INVALID_ARGUMENT;
 *out=NULL;
 /* Hold is arbitrary caller code. Capture every input it could replace before
  * invoking it; publication and rollback must consume that exact owner. */
 xge_particle_world_desc_t snapshot;
 if(desc){snapshot=*desc;desc=&snapshot;}
 xge_callback_owner_t incoming=code ? *code:(xge_callback_owner_t){0};
 if((desc && (desc->pCollision || desc->pCollisionUser)) ||
  !((!incoming.tReference.Data && !incoming.tReference.Ops && !incoming.onHold && !incoming.onDrop) ||
   (incoming.tReference.Data && incoming.tReference.Ops && incoming.onHold && incoming.onDrop)))return XGE_ERROR_INVALID_ARGUMENT;
 xerror* ambient=xrtTakeError();bool held=false;int status=XGE_OK;
 if(incoming.onHold){held=incoming.onHold(incoming.tReference.Data);if(!held || xrtGetError())status=XGE_ERROR_CALLBACK_FAILED;}
 xge_data_outcome_t result={0};xpWorldOutcomeBegin(&result,status);
 xge_particle_world world=NULL;
 if(status==XGE_OK){
  status=xp_world_create(&world,desc,true);xpWorldOutcomeAdd(&result,status);
  if(status==XGE_OK){if(held)xpWorldStorage(world)->codeOwner=incoming;*out=world;held=false;}
 }
 if(held)xpWorldOutcomeAdd(&result,xpWorldOwnerDrop(incoming));
 return xpWorldOutcomeEnd(&result,ambient);
}
int xgeParticleWorldShareOwned(xge_particle_world world,xge_particle_world* out)
{
 if(!out)return XGE_ERROR_INVALID_ARGUMENT;
 *out=NULL;xge_data_lifetime_t* root=xpWorldRoot(world);
 int status=xpWorldAdmit(root);
 if(status==XGE_OK && !xpWorldReadable(root))status=XGE_ERROR_INVALID_STATE;
 if(status==XGE_OK)status=xpWorldAcquire(root,true,false);
 if(status==XGE_OK)*out=world;
 return status;
}
int xgeParticleWorldReleaseOwnerChecked(xge_particle_world world,bool* consumed)
{
 if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
 *consumed=false;
 if(!world){*consumed=true;return XGE_OK;}
 if(!world->managed){
  if(world->busy)return XGE_ERROR_BUSY;
  xp_world_contents_free(world);xrtFree(world);*consumed=true;return XGE_OK;
 }
 return xpWorldRelease(xpWorldRoot(world),consumed);
}
int xgeParticleWorldFreeChecked(xge_particle_world world)
{bool consumed=false;return xgeParticleWorldReleaseOwnerChecked(world,&consumed);}
