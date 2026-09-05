#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
#define CHECK(condition)                                                                                     \
	do                                                                                                       \
	{                                                                                                        \
		checks++;                                                                                            \
		if (!(condition))                                                                                    \
		{                                                                                                    \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);                             \
			exit(1);                                                                                         \
		}                                                                                                    \
	} while (0)
#define OK(call) CHECK((call) == XGE_OK)
#define NEAR(a, b) CHECK(fabs((double)(a) - (double)(b)) < .0001)

typedef struct capture_t
{
	xge_particle_snapshot_t particles[512];
	int count;
} capture_t;
static int collect(const xge_particle_snapshot_t *p, void *user)
{
	capture_t *c = (capture_t *)user;
	CHECK(c->count < 512);
	c->particles[c->count++] = *p;
	return 1;
}
static capture_t capture(xge_particle_world w)
{
	capture_t c;
	memset(&c, 0, sizeof(c));
	OK(xgeParticleVisit(w, collect, &c));
	return c;
}
static xge_particle_world world(void)
{
	xge_particle_world w = NULL;
	xge_particle_world_desc_t d;
	xgeParticleWorldDescInit(&d);
	d.fFixedStep = 1.0f / 64;
	d.iMaxEffects = 8;
	d.iMaxParticles = 4096;
	OK(xgeParticleWorldCreate(&w, &d));
	return w;
}
static xge_particle_definition definition(xge_particle_emitter_t *e, int count)
{
	xge_particle_definition d = NULL;
	OK(xgeParticleDefinitionCreate(&d, e, count));
	return d;
}
static void advance(xge_particle_world w, int count)
{
	int i;
	for (i = 0; i < count; i++)
		OK(xgeParticleUpdate(w, 1.0 / 64));
}
static xge_particle_effect play(xge_particle_world w, xge_particle_definition d)
{
	xge_particle_effect h = 0;
	OK(xgeParticlePlay(w, d, NULL, &h));
	CHECK(h != 0);
	return h;
}
static void test_curves(void)
{
	xge_particle_curve_t c = {0};
	xge_particle_gradient_t g = {0};
	NEAR(xgeParticleCurveEval(&c, .3f, 7), 7);
	c.iCount = 2;
	c.arrKeys[0] = (xge_particle_key_t){0, 2, 0, 0};
	c.arrKeys[1] = (xge_particle_key_t){1, 6, 0, 0};
	NEAR(xgeParticleCurveEval(&c, .5f, 0), 4);
	NEAR(xgeParticleCurveEval(&c, -1, 0), 2);
	NEAR(xgeParticleCurveEval(&c, 2, 0), 6);
	c.iInterpolation = XGE_PARTICLE_CURVE_STEP;
	NEAR(xgeParticleCurveEval(&c, .99f, 0), 2);
	NEAR(xgeParticleCurveEval(&c, 1, 0), 6);
	c.iInterpolation = XGE_PARTICLE_CURVE_HERMITE;
	NEAR(xgeParticleCurveEval(&c, .5f, 0), 4);
	NEAR(xgeParticleCurveEval(&c, NAN, 9), 9);
	g.iCount = 2;
	g.arrKeys[0] = (xge_particle_color_key_t){0, 0xff000000};
	g.arrKeys[1] = (xge_particle_color_key_t){1, 0x0000ffff};
	CHECK(xgeParticleGradientEval(&g, .5f, 0) == 0x80008080);
}
static void test_validation(void)
{
	xge_particle_emitter_t e[2];
	xge_particle_definition d = NULL;
	xgeParticleEmitterInit(e);
	e[1] = e[0];
	e[0].tLife.fMin = 0;
	CHECK(xgeParticleDefinitionCreate(&d, e, 1) == XGE_ERROR_INVALID_ARGUMENT && !d);
	e[0] = e[1];
	e[0].fSpread = NAN;
	CHECK(xgeParticleDefinitionCreate(&d, e, 1) == XGE_ERROR_INVALID_ARGUMENT);
	e[0] = e[1];
	e[0].tSizeOverLife.iCount = 2;
	CHECK(xgeParticleDefinitionCreate(&d, e, 1) == XGE_ERROR_INVALID_ARGUMENT);
	e[0] = e[1];
	e[0].arrSubEmitters[0] = (xge_particle_subemitter_t){1, 1, 0};
	e[1].arrSubEmitters[1] = (xge_particle_subemitter_t){0, 1, 0};
	CHECK(xgeParticleDefinitionCreate(&d, e, 2) == XGE_ERROR_INVALID_ARGUMENT);
	e[1].arrSubEmitters[1].iEmitter = -1;
	d = definition(e, 2);
	CHECK(xgeParticleDefinitionCount(d) == 2);
	xgeParticleDefinitionFree(d);
}
static void test_lifecycle(void)
{
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_world w = world();
	xge_particle_effect h, old;
	xge_particle_event_t event;
	capture_t c;
	uint32_t emitted;
	int finished = 0;
	xgeParticleEmitterInit(&e);
	e.fRate = 64;
	e.tSpeed = (xge_particle_range_t){0, 0};
	d = definition(&e, 1);
	OK(xgeParticleWorldReserve(w, d, 2));
	CHECK(xgeParticleStats(w).iCachedEffects == 2);
	h = play(w, d);
	advance(w, 16);
	CHECK(xgeParticleStats(w).iLiveParticles == 16);
	OK(xgeParticlePause(w, h, 1));
	c = capture(w);
	advance(w, 8);
	NEAR(capture(w).particles[0].fAge, c.particles[0].fAge);
	OK(xgeParticlePause(w, h, 0));
	OK(xgeParticleStop(w, h, 0));
	advance(w, 70);
	CHECK(xgeParticleState(w, h) == XGE_PARTICLE_FINISHED);
	CHECK(!xgeParticleStats(w).iLiveParticles);
	while (xgeParticleEventPoll(w, &event) > 0)
		if (event.iType == XGE_PARTICLE_EVENT_FINISHED)
			finished++;
	CHECK(finished == 1);
	OK(xgeParticleRestart(w, h, 42));
	OK(xgeParticleEmit(w, h, 0, 3, &emitted));
	CHECK(emitted == 3);
	OK(xgeParticleStop(w, h, 1));
	CHECK(!xgeParticleStats(w).iLiveParticles);
	advance(w, 1);
	old = h;
	OK(xgeParticleRelease(w, h));
	h = play(w, d);
	CHECK(h != old);
	CHECK(xgeParticleRestart(w, old, 2) == XGE_ERROR_NOT_FOUND);
	CHECK(xgeParticleUpdate(w, -1) == XGE_ERROR_INVALID_ARGUMENT);
	CHECK(xgeParticleUpdate(w, NAN) == XGE_ERROR_INVALID_ARGUMENT);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_emission_and_steps(void)
{
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_world a = world(), b = world();
	xge_particle_effect h;
	xgeParticleEmitterInit(&e);
	e.fRate = 64;
	e.tLife = (xge_particle_range_t){10, 10};
	e.fDuration = .25f;
	e.bLoop = 1;
	e.iBurstCount = 1;
	e.arrBursts[0] = (xge_particle_burst_t){0, 4};
	d = definition(&e, 1);
	h = play(a, d);
	play(b, d);
	advance(a, 32);
	OK(xgeParticleUpdate(b, .5));
	CHECK(xgeParticleStats(a).iSpawned == 44);
	CHECK(xgeParticleStats(b).iSpawned == 44);
	CHECK(capture(a).count == capture(b).count);
	NEAR(capture(a).particles[0].tPosition.fX, capture(b).particles[0].tPosition.fX);
	OK(xgeParticleStop(a, h, 0));
	advance(a, 16);
	CHECK(xgeParticleStats(a).iSpawned == 44);
	xgeParticleWorldFree(a);
	xgeParticleWorldFree(b);
	xgeParticleDefinitionFree(d);
	{
		xge_particle_world_desc_t desc;
		xgeParticleWorldDescInit(&desc);
		desc.fFixedStep = .125f;
		desc.iMaxSubsteps = 2;
		OK(xgeParticleWorldCreate(&a, &desc));
		OK(xgeParticleUpdate(a, 1));
		CHECK(xgeParticleStats(a).iSteps == 2);
		NEAR(xgeParticleStats(a).fDroppedTime, .75);
		xgeParticleWorldFree(a);
	}
}
static void test_capacity(void)
{
	xge_particle_world_desc_t config;
	xge_particle_emitter_t e;
	xge_particle_world w = NULL;
	xge_particle_definition d;
	xge_particle_effect h;
	uint32_t emitted;
	xgeParticleWorldDescInit(&config);
	config.iMaxEffects = 1;
	config.iMaxParticles = 4;
	config.iMaxEvents = 2;
	config.iMaxSpawnPerStep = 8;
	OK(xgeParticleWorldCreate(&w, &config));
	xgeParticleEmitterInit(&e);
	e.iMaxParticles = 3;
	d = definition(&e, 1);
	h = play(w, d);
	OK(xgeParticleEmit(w, h, 0, 100, &emitted));
	CHECK(emitted == 3);
	CHECK(xgeParticleStats(w).iDroppedParticles == 97);
	CHECK(xgeParticleStats(w).iDroppedEvents == 1);
	CHECK(xgeParticleStats(w).iPendingEvents == 2);
	CHECK(xgeParticlePlay(w, d, NULL, &h) == XGE_ERROR_OUT_OF_MEMORY && h == 0);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_shapes_and_random(void)
{
	int shape, i;
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_world w = world();
	xge_particle_effect h;
	capture_t a, b;
	for (shape = 0; shape <= 5; shape++)
	{
		xgeParticleEmitterInit(&e);
		e.fRate = 0;
		e.iShape = shape;
		e.fRadius = 10;
		e.fInnerRadius = 5;
		e.tShapeSize = (xge_vec2_t){20, 12};
		e.tSpeed = (xge_particle_range_t){0, 0};
		e.fDirection = 0;
		e.fSpread = .5f;
		d = definition(&e, 1);
		h = play(w, d);
		OK(xgeParticleEmit(w, h, 0, 256, NULL));
		a = capture(w);
		CHECK(a.count == 256);
		for (i = 0; i < a.count; i++)
		{
			float x = a.particles[i].tPosition.fX, y = a.particles[i].tPosition.fY, r = hypotf(x, y);
			if (shape == 0)
				NEAR(r, 0);
			if (shape == 1)
				NEAR(y, x * .6f);
			if (shape == 2)
				CHECK(fabsf(x) <= 10 && fabsf(y) <= 6);
			if (shape >= 3)
				CHECK(r <= 10.0001f);
			if (shape == 4)
				CHECK(r >= 4.9999f);
			if (shape == 5)
				CHECK(fabsf(atan2f(y, x)) <= .251f);
		}
		OK(xgeParticleRestart(w, h, 1));
		OK(xgeParticleEmit(w, h, 0, 256, NULL));
		b = capture(w);
		for (i = 0; i < a.count; i++)
		{
			NEAR(a.particles[i].tPosition.fX, b.particles[i].tPosition.fX);
			NEAR(a.particles[i].tPosition.fY, b.particles[i].tPosition.fY);
		}
		OK(xgeParticleRelease(w, h));
		xgeParticleDefinitionFree(d);
	}
	xgeParticleWorldFree(w);
}
static void test_space_distance_and_parameters(void)
{
	int space;
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_world w = world();
	xge_particle_effect h;
	xge_particle_transform_t t = {{10, 20}, {1, 1}, 0};
	xge_rect_t bounds;
	for (space = 0; space < 2; space++)
	{
		xgeParticleEmitterInit(&e);
		e.fRate = 0;
		e.iSpace = space;
		e.tSpeed = (xge_particle_range_t){0, 0};
		e.tSize = (xge_particle_range_t){4, 4};
		d = definition(&e, 1);
		h = play(w, d);
		OK(xgeParticleEmit(w, h, 0, 1, NULL));
		OK(xgeParticleSetTransform(w, h, &t));
		advance(w, 1);
		NEAR(capture(w).particles[0].tPosition.fX, space ? 10 : 0);
		NEAR(capture(w).particles[0].tPosition.fY, space ? 20 : 0);
		OK(xgeParticleBounds(w, h, &bounds));
		NEAR(bounds.fW, 4);
		NEAR(bounds.fH, 4);
		OK(xgeParticleRelease(w, h));
		xgeParticleDefinitionFree(d);
	}
	xgeParticleEmitterInit(&e);
	e.fRate = 0;
	e.fRateOverDistance = 1;
	e.tSpeed = (xge_particle_range_t){0, 0};
	d = definition(&e, 1);
	h = play(w, d);
	t.tPosition = (xge_vec2_t){10, 0};
	OK(xgeParticleSetTransform(w, h, &t));
	advance(w, 1);
	CHECK(capture(w).count == 10);
	NEAR(capture(w).particles[0].tPosition.fX, 1);
	NEAR(capture(w).particles[9].tPosition.fX, 10);
	{
		xge_particle_parameters_t p = {0, 2, 2, 0, 0xff000080};
		capture_t c = capture(w);
		OK(xgeParticleSetParameters(w, h, &p));
		advance(w, 1);
		NEAR(capture(w).particles[0].fAge, c.particles[0].fAge);
		NEAR(capture(w).particles[0].tSize.fX, c.particles[0].tSize.fX * 2);
		CHECK(capture(w).particles[0].iColor == 0xff000080);
	}
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_subemitters(void)
{
	xge_particle_emitter_t e[3];
	xge_particle_definition d;
	xge_particle_world w = world();
	xge_particle_effect h;
	xge_particle_event_t event;
	int i, births = 0, deaths = 0;
	for (i = 0; i < 3; i++)
	{
		xgeParticleEmitterInit(&e[i]);
		e[i].bAutomatic = 0;
		e[i].tSpeed = (xge_particle_range_t){0, 0};
	}
	e[0].tLife = (xge_particle_range_t){.03125f, .03125f};
	e[0].arrSubEmitters[XGE_PARTICLE_EVENT_BIRTH] = (xge_particle_subemitter_t){1, 2, 0};
	e[0].arrSubEmitters[XGE_PARTICLE_EVENT_DEATH] = (xge_particle_subemitter_t){2, 3, 0};
	d = definition(e, 3);
	h = play(w, d);
	OK(xgeParticleEmit(w, h, 0, 1, NULL));
	CHECK(xgeParticleStats(w).iLiveParticles == 3);
	advance(w, 2);
	CHECK(xgeParticleStats(w).iLiveParticles == 5);
	while (xgeParticleEventPoll(w, &event) > 0)
	{
		births += event.iType == XGE_PARTICLE_EVENT_BIRTH;
		deaths += event.iType == XGE_PARTICLE_EVENT_DEATH;
	}
	CHECK(births == 6 && deaths == 1);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static int ground(xge_vec2_t from, xge_vec2_t to, float radius, xge_particle_hit_t *hit, void *user)
{
	(void)user;
	return xgeParticleCollidePlane(from, to, radius, (xge_vec2_t){0, -1}, -1, hit);
}
static void test_collisions(void)
{
	xge_particle_hit_t hit;
	xge_particle_world_desc_t config;
	xge_particle_world w;
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_effect h;
	CHECK(
	    xgeParticleCollidePlane((xge_vec2_t){0, -10}, (xge_vec2_t){0, 10}, 2, (xge_vec2_t){0, -1}, 0, &hit));
	NEAR(hit.fFraction, .4);
	NEAR(hit.tPosition.fY, -2);
	CHECK(xgeParticleCollideRect((xge_vec2_t){-5, 5}, (xge_vec2_t){20, 5}, 1, (xge_rect_t){0, 0, 10, 10},
	                             &hit));
	NEAR(hit.tPosition.fX, -1);
	NEAR(hit.tNormal.fX, -1);
	CHECK(!xgeParticleCollideRect((xge_vec2_t){-5, -5}, (xge_vec2_t){20, -5}, 0, (xge_rect_t){0, 0, 10, 10},
	                              &hit));
	xgeParticleWorldDescInit(&config);
	config.fFixedStep = 1.0f / 64;
	config.pCollision = ground;
	OK(xgeParticleWorldCreate(&w, &config));
	xgeParticleEmitterInit(&e);
	e.fRate = 0;
	e.fDirection = 3.14159265f / 2;
	e.fSpread = 0;
	e.tSpeed = (xge_particle_range_t){128, 128};
	e.iCollision = XGE_PARTICLE_COLLISION_BOUNCE;
	e.fRestitution = 1;
	d = definition(&e, 1);
	h = play(w, d);
	OK(xgeParticleEmit(w, h, 0, 1, NULL));
	advance(w, 1);
	CHECK(xgeParticleStats(w).iCollisions == 1);
	CHECK(capture(w).particles[0].tVelocity.fY < 0);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_noise_and_culling(void)
{
	xge_particle_emitter_t e;
	xge_particle_world w = world();
	xge_particle_definition d;
	xge_particle_effect h;
	xge_particle_play_t p;
	capture_t before, after;
	xgeParticleEmitterInit(&e);
	e.fRate = 64;
	e.fNoiseStrength = 100;
	e.fNoiseScroll = 1;
	d = definition(&e, 1);
	xgeParticlePlayInit(&p);
	p.iCullPolicy = XGE_PARTICLE_CULL_PAUSE;
	OK(xgeParticlePlay(w, d, &p, &h));
	OK(xgeParticlePrewarm(w, h, .5f));
	CHECK(xgeParticleStats(w).iPendingEvents == 0);
	before = capture(w);
	OK(xgeParticleSetVisible(w, h, 0));
	advance(w, 4);
	NEAR(capture(w).particles[0].fAge, before.particles[0].fAge);
	OK(xgeParticleSetVisible(w, h, 1));
	OK(xgeParticleRestart(w, h, 1));
	OK(xgeParticlePrewarm(w, h, .5f));
	after = capture(w);
	CHECK(before.count == after.count);
	NEAR(before.particles[0].tPosition.fX, after.particles[0].tPosition.fX);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static int attempt_mutation(const xge_particle_snapshot_t *p, void *user)
{
	CHECK(xgeParticleRelease((xge_particle_world)user, p->iEffect) == XGE_ERROR_INVALID_STATE);
	return 0;
}
static void test_boundaries(void)
{
	xge_particle_emitter_t e;
	xge_particle_definition d, other;
	xge_particle_world w = world();
	xge_particle_effect h;
	xge_particle_play_t p;
	xge_particle_transform_t t = {{10, 0}, {1, 1}, 0};
	capture_t c;
	xge_rect_t bounds;
	int delayed;
	for (delayed = 0; delayed < 2; delayed++)
	{
		xgeParticleEmitterInit(&e);
		e.fRate = 0;
		e.fRateOverDistance = 1;
		e.fDuration = 1.0f / 128;
		e.fDelay = delayed ? 1.0f / 128 : 0;
		e.bLoop = 0;
		e.tSpeed = (xge_particle_range_t){0, 0};
		d = definition(&e, 1);
		h = play(w, d);
		OK(xgeParticleSetTransform(w, h, &t));
		advance(w, 1);
		c = capture(w);
		CHECK(c.count == 5);
		NEAR(c.particles[0].tPosition.fX, delayed ? 6 : 1);
		NEAR(c.particles[4].tPosition.fX, delayed ? 10 : 5);
		OK(xgeParticleRelease(w, h));
		xgeParticleDefinitionFree(d);
	}
	xgeParticleEmitterInit(&e);
	e.fRate = 64;
	e.fDuration = 1.0f / 32;
	e.bLoop = 0;
	d = definition(&e, 1);
	xgeParticlePlayInit(&p);
	p.iCullPolicy = XGE_PARTICLE_CULL_CLEAR;
	OK(xgeParticlePlay(w, d, &p, &h));
	advance(w, 1);
	CHECK(capture(w).count == 1);
	OK(xgeParticleSetVisible(w, h, 0));
	advance(w, 4);
	CHECK(xgeParticleState(w, h) == XGE_PARTICLE_FINISHED);
	CHECK(!capture(w).count);
	OK(xgeParticleRelease(w, h));
	p.iCullPolicy = XGE_PARTICLE_CULL_PAUSE;
	p.tParameters.fTimeScale = 0;
	OK(xgeParticlePlay(w, d, &p, &h));
	OK(xgeParticleSetVisible(w, h, 0));
	OK(xgeParticleStop(w, h, 1));
	advance(w, 1);
	CHECK(xgeParticleState(w, h) == XGE_PARTICLE_FINISHED);
	OK(xgeParticleRelease(w, h));
	/* Warm a new definition even when all cache slots hold a different one. */
	OK(xgeParticleWorldReserve(w, d, 8));
	other = definition(&e, 1);
	OK(xgeParticleWorldReserve(w, other, 8));
	CHECK(xgeParticleStats(w).iCachedEffects == 8);
	xgeParticleDefinitionFree(other);
	xgeParticleDefinitionFree(d);
	/* Extreme but finite inputs must not overflow dimensions or float-to-int spawn counts. */
	xgeParticleEmitterInit(&e);
	e.fRate = 0;
	e.fRateOverDistance = 1e6f;
	e.fStretch = 1e12f;
	e.fAspect = 1e12f;
	e.tSize = (xge_particle_range_t){1e6f, 1e6f};
	e.tSpeed = (xge_particle_range_t){1e6f, 1e6f};
	e.tSizeOverLife.iCount = 1;
	e.tSizeOverLife.arrKeys[0] = (xge_particle_key_t){0, 1e12f, 0, 0};
	d = definition(&e, 1);
	xgeParticlePlayInit(&p);
	p.tParameters.fRateScale = 1000;
	p.tParameters.fSizeScale = 1000;
	OK(xgeParticlePlay(w, d, &p, &h));
	t.tPosition.fX = 1e12f;
	OK(xgeParticleSetTransform(w, h, &t));
	advance(w, 1);
	OK(xgeParticleBounds(w, h, &bounds));
	CHECK(isfinite(bounds.fW) && isfinite(bounds.fH));
	CHECK(xgeParticleStats(w).iLiveParticles == 512);
	CHECK(xgeParticleStats(w).iDroppedParticles > 1000000);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_visit_and_autorelease(void)
{
	xge_particle_emitter_t e;
	xge_particle_world w = world();
	xge_particle_definition d;
	xge_particle_effect h;
	xge_particle_play_t p;
	xgeParticleEmitterInit(&e);
	e.bAutomatic = 0;
	e.tLife = (xge_particle_range_t){.015625f, .015625f};
	d = definition(&e, 1);
	xgeParticlePlayInit(&p);
	p.bAutoRelease = 1;
	OK(xgeParticlePlay(w, d, &p, &h));
	OK(xgeParticleEmit(w, h, 0, 1, NULL));
	OK(xgeParticleVisit(w, attempt_mutation, w));
	advance(w, 1);
	CHECK(xgeParticleState(w, h) == XGE_ERROR_NOT_FOUND);
	CHECK(xgeParticleStats(w).iCachedEffects == 1);
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
}
static void test_io(void)
{
	const char *text = "{\"version\":1,\"emitters\":[{\"name\":\"smoke\",\"rate\":18,\"colorMin\":\"#"
	                   "22334455\",\"sizeOverLife\":{\"keys\":[[0,1],[1,3]]},\"bursts\":[[0,4]]}]}";
	xge_particle_definition a = NULL, b = NULL;
	xge_particle_emitter_t e;
	char error[160], *first = NULL, *second = NULL;
	size_t n, m;
	int xson;
	OK(xgeParticleDefinitionParse(&a, text, strlen(text), 0, error, sizeof(error)));
	OK(xgeParticleDefinitionGet(a, 0, &e));
	CHECK(!strcmp(e.sName, "smoke") && e.iColorMin == 0x22334455 && e.iBurstCount == 1);
	NEAR(e.fRate, 18);
	for (xson = 0; xson <= 1; xson++)
	{
		OK(xgeParticleDefinitionStringify(a, xson, &first, &n));
		OK(xgeParticleDefinitionParse(&b, first, n, xson, error, sizeof(error)));
		OK(xgeParticleDefinitionStringify(b, xson, &second, &m));
		CHECK(n == m && !memcmp(first, second, n));
		xrtFree(first);
		xrtFree(second);
		xgeParticleDefinitionFree(b);
	}
	xgeParticleDefinitionFree(a);
	text = "{\"version\":2,\"emitters\":[{}]}";
	CHECK(xgeParticleDefinitionParse(&a, text, strlen(text), 0, error, sizeof(error)) != XGE_OK && !a);
	text = "{\"version\":1,\"emitters\":[{\"rtae\":2}]}";
	CHECK(xgeParticleDefinitionParse(&a, text, strlen(text), 0, error, sizeof(error)) != XGE_OK);
	CHECK(strstr(error, "rtae") != NULL);
	text = "{\"version\":1,\"emitters\":[{\"life\":[0,1]}]}";
	CHECK(xgeParticleDefinitionParse(&a, text, strlen(text), 0, error, sizeof(error)) != XGE_OK);
	text = "{\"version\":1,\"emitters\":[{\"colorMax\":\"#gg000000\"}]}";
	CHECK(xgeParticleDefinitionParse(&a, text, strlen(text), 0, error, sizeof(error)) != XGE_OK);
}

#if defined(XRT_FEATURE_MEMORY_DEBUG)
static void test_memory(void)
{
	xge_particle_emitter_t e;
	xge_particle_definition d;
	xge_particle_world w;
	xge_particle_effect h;
	xmemdebugsnapshot before, after;
	int i, phase, success;
	xgeParticleEmitterInit(&e);
	e.fRate = 64;
	e.fNoiseStrength = 30;
	d = definition(&e, 1);
	w = world();
	OK(xgeParticleWorldReserve(w, d, 2));
	h = play(w, d);
	xrtMemDebugSnapshot(&before);
	advance(w, 120);
	xrtMemDebugSnapshot(&after);
	CHECK(after.AllocCount == before.AllocCount && after.ReallocCount == before.ReallocCount);
	OK(xgeParticleRelease(w, h));
	xgeParticleWorldFree(w);
	xgeParticleDefinitionFree(d);
	for (phase = 0; phase < 4; phase++)
	{
		success = 0;
		for (i = 0; i < 4096; i++)
		{
			const char *json = "{\"version\":1,\"emitters\":[{\"rate\":18}]}";
			int result;
			bool fired;
			char *output = NULL;
			d = NULL;
			w = NULL;
			if (phase == 2 || phase == 3)
				d = definition(&e, 1);
			xrtClearError();
			xrtMemDebugSnapshot(&before);
			CHECK(xrtMemDebugFailAfter((uint64)i));
			if (phase == 0)
				result = xgeParticleDefinitionParse(&d, json, strlen(json), 0, NULL, 0);
			else if (phase == 1)
				result = xgeParticleWorldCreate(&w, NULL);
			else if (phase == 2)
			{
				result = xgeParticleWorldCreate(&w, NULL);
				if (result == XGE_OK)
					result = xgeParticlePlay(w, d, NULL, &h);
			}
			else
				result = xgeParticleDefinitionStringify(d, 0, &output, NULL);
			fired = xrtMemDebugFailTriggered();
			xrtMemDebugFailClear();
			xrtFree(output);
			xgeParticleWorldFree(w);
			xrtClearError();
			if (phase < 2)
			{
				xgeParticleDefinitionFree(d);
				d = NULL;
			}
			xrtMemDebugSnapshot(&after);
			if (after.LiveCount != before.LiveCount || after.LiveBytes != before.LiveBytes)
				fprintf(stderr, "OOM phase=%d index=%d result=%d live=%zu/%zu bytes=%zu/%zu\n", phase, i,
				        result, after.LiveCount, before.LiveCount, after.LiveBytes, before.LiveBytes);
			CHECK(after.LiveCount == before.LiveCount);
			CHECK(after.LiveBytes == before.LiveBytes);
			xgeParticleDefinitionFree(d);
			if (!fired)
			{
				OK(result);
				success = 1;
				break;
			}
		}
		CHECK(success);
		printf("OOM phase %d: %d failure points\n", phase, i);
	}
}
#endif
int main(void)
{
#if defined(XRT_FEATURE_MEMORY_DEBUG)
	CHECK(xrtMemDebugEnable(true));
#endif
	test_curves();
	test_validation();
	test_lifecycle();
	test_emission_and_steps();
	test_capacity();
	test_shapes_and_random();
	test_space_distance_and_parameters();
	test_subemitters();
	test_collisions();
	test_noise_and_culling();
	test_boundaries();
	test_visit_and_autorelease();
	test_io();
#if defined(XRT_FEATURE_MEMORY_DEBUG)
	test_memory();
	{
		xmemdebugsnapshot s;
		xrtClearError();
		xrtMemDebugSnapshot(&s);
		CHECK(s.LiveCount == 0);
		CHECK(!s.DoubleFreeCount && !s.InvalidFreeCount && !s.OverflowCount && !s.UnderflowCount);
	}
	CHECK(xrtMemDebugReset());
#endif
	printf("PASS particle runtime: %d checks\n", checks);
	return 0;
}
