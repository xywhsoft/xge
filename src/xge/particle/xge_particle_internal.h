#ifndef XGE_PARTICLE_INTERNAL_H
#define XGE_PARTICLE_INTERNAL_H
#include "xge.h"
#include "../core/xge_data_lifetime_internal.h"

struct xge_particle_definition_t
{
	volatile int32 iRefs;
	xarray emitters;
	uint32_t capacity;
};
typedef struct xp_particle
{
	xge_vec2_t position, velocity, origin, scale;
	float age, life, size, rotation, spin, startFrame;
	uint32_t color;
	uint64_t serial;
	int emitter;
} xp_particle;
typedef struct xp_emitter
{
	xrng rng;
	double time, rateFraction, distanceFraction;
	uint32_t live;
	int started;
} xp_emitter;
typedef struct xp_effect
{
	xge_particle_definition definition;
	xge_particle_effect handle;
	xge_particle_play_t play;
	xge_particle_transform_t previous, current;
	xarray particles;
	xp_emitter emitters[XGE_PARTICLE_MAX_EMITTERS];
	int state, resumeState, visible;
} xp_effect;
typedef struct xp_spawn
{
	int emitter;
	uint32_t count;
	xge_vec2_t position, velocity;
	float inherit;
} xp_spawn;
struct xge_particle_world_t
{
	xge_particle_world_desc_t desc;
	xpool pool;
	xslotmap handles;
	xarray effects, events, spawns;
	uint32_t eventHead, eventCount, stepSpawned;
	uint64_t serial;
	double remainder;
	/* These were two private boolean-valued ints. Their existing eight-byte
	 * slot also fits the mode flags without growing the raw C world. */
	bool busy, suppressEvents, managed, destroyed;
	xge_particle_stats_t stats;
};
typedef struct xp_world_owned_t {
	/* Exact public handle/body at offset zero; only managed Create allocates
	 * this larger record. There is no second simulation or owning wrapper. */
	struct xge_particle_world_t world;
	atomic_uint refs;
	xge_callback_owner_t codeOwner;
	xge_data_lifetime_t lifetime;
} xp_world_owned_t;
#endif
