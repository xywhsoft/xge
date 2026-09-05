/* Independent fixed-step CPU particle runtime. No GPU or application globals. */
#include "xge_particle_internal.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include "xge_math2d.h"

#define XP_PI 3.14159265358979323846f
#define XP_LIMIT 1000000u
static int xp_finite(float f)
{
	return isfinite(f) && fabsf(f) <= 1e12f;
}
static int xp_vec_valid(xge_vec2_t p)
{
	return xp_finite(p.fX) && xp_finite(p.fY);
}
static xge_vec2_t xp_add(xge_vec2_t a, xge_vec2_t b)
{
	return (xge_vec2_t){a.fX + b.fX, a.fY + b.fY};
}
static xge_vec2_t xp_sub(xge_vec2_t a, xge_vec2_t b)
{
	return (xge_vec2_t){a.fX - b.fX, a.fY - b.fY};
}
static xge_vec2_t xp_mul(xge_vec2_t a, float s)
{
	return (xge_vec2_t){a.fX * s, a.fY * s};
}
static float xp_dot(xge_vec2_t a, xge_vec2_t b)
{
	return a.fX * b.fX + a.fY * b.fY;
}
static float xp_length(xge_vec2_t a)
{
	return hypotf(a.fX, a.fY);
}
static xge_particle_emitter_t *xp_desc(xp_effect *f, int e)
{
	return &((xge_particle_emitter_t *)f->definition->emitters.Data)[e];
}
static void xp_drop(xge_particle_world w, uint64_t count)
{
	w->stats.iDroppedParticles =
	    UINT64_MAX - w->stats.iDroppedParticles < count ? UINT64_MAX : w->stats.iDroppedParticles + count;
}
static xp_effect *xp_find(xge_particle_world w, xge_particle_effect h)
{
	return w && h ? (xp_effect *)xrtSlotMapGet(&w->handles, h) : NULL;
}
static xge_shape_ex_matrix_t xp_matrix(xge_particle_transform_t t)
{
	float c = cosf(t.fRotation), s = sinf(t.fRotation);
	return (xge_shape_ex_matrix_t){c * t.tScale.fX, s * t.tScale.fX, -s * t.tScale.fY,
	                               c * t.tScale.fY, t.tPosition.fX,  t.tPosition.fY};
}
static xge_vec2_t xp_vector(xge_particle_transform_t t, xge_vec2_t p)
{
	xge_shape_ex_matrix_t m = xp_matrix(t);
	m.fE = m.fF = 0;
	return __xgeMath2DPoint(m, p);
}
static xge_vec2_t xp_inverse(xge_particle_transform_t t, xge_vec2_t p, int point)
{
	float c = cosf(t.fRotation), s = sinf(t.fRotation);
	if (point)
		p = xp_sub(p, t.tPosition);
	return (xge_vec2_t){(c * p.fX + s * p.fY) / t.tScale.fX, (-s * p.fX + c * p.fY) / t.tScale.fY};
}
static int xp_transform_valid(const xge_particle_transform_t *t)
{
	return t && xp_vec_valid(t->tPosition) && xp_vec_valid(t->tScale) && t->tScale.fX >= .0001f &&
	       t->tScale.fY >= .0001f && t->tScale.fX <= 10000 && t->tScale.fY <= 10000 &&
	       xp_finite(t->fRotation);
}
static int xp_parameters_valid(const xge_particle_parameters_t *p)
{
	return p && xp_finite(p->fRateScale) && p->fRateScale >= 0 && p->fRateScale <= 1000 &&
	       xp_finite(p->fSpeedScale) && p->fSpeedScale >= 0 && p->fSpeedScale <= 1000 &&
	       xp_finite(p->fSizeScale) && p->fSizeScale >= 0 && p->fSizeScale <= 1000 &&
	       xp_finite(p->fTimeScale) && p->fTimeScale >= 0 && p->fTimeScale <= 16;
}

void xgeParticleEmitterInit(xge_particle_emitter_t *e)
{
	int i;
	if (!e)
		return;
	memset(e, 0, sizeof(*e));
	e->iMaxParticles = 512;
	e->bAutomatic = 1;
	e->bLoop = 1;
	e->fRate = 30;
	e->fDirection = -XP_PI / 2;
	e->fSpread = XP_PI / 3;
	e->tLife = (xge_particle_range_t){1, 1};
	e->tSpeed = (xge_particle_range_t){20, 40};
	e->tSize = (xge_particle_range_t){8, 12};
	e->fAspect = 1;
	e->iColorMin = e->iColorMax = 0xffffffffu;
	e->iColumns = e->iRows = 1;
	e->iBlend = XGE_BLEND_ALPHA;
	e->fNoiseFrequency = .01f;
	e->fRestitution = .5f;
	for (i = 0; i < 3; i++)
		e->arrSubEmitters[i].iEmitter = -1;
}
void xgeParticleWorldDescInit(xge_particle_world_desc_t *d)
{
	if (!d)
		return;
	memset(d, 0, sizeof(*d));
	d->iMaxEffects = 128;
	d->iMaxParticles = 32768;
	d->iMaxEvents = 4096;
	d->iMaxSpawnPerStep = 4096;
	d->iMaxSubsteps = 64;
	d->fFixedStep = 1.0f / 120;
}
void xgeParticlePlayInit(xge_particle_play_t *p)
{
	if (!p)
		return;
	memset(p, 0, sizeof(*p));
	p->tTransform.tScale = (xge_vec2_t){1, 1};
	p->tParameters = (xge_particle_parameters_t){1, 1, 1, 1, 0xffffffffu};
	p->iSeed = 1;
}
static int xp_curve_valid(const xge_particle_curve_t *c)
{
	int i;
	if (c->iCount < 0 || c->iCount > XGE_PARTICLE_MAX_KEYS || c->iInterpolation < 0 || c->iInterpolation > 2)
		return 0;
	for (i = 0; i < c->iCount; i++)
	{
		const xge_particle_key_t *k = &c->arrKeys[i];
		if (!xp_finite(k->fTime) || k->fTime < 0 || k->fTime > 1 || !xp_finite(k->fValue) ||
		    !xp_finite(k->fInTangent) || !xp_finite(k->fOutTangent) ||
		    (i && k->fTime <= c->arrKeys[i - 1].fTime))
			return 0;
	}
	return 1;
}
float xgeParticleCurveEval(const xge_particle_curve_t *c, float t, float fallback)
{
	int i;
	if (!c || !xp_curve_valid(c) || !isfinite(t) || !c->iCount)
		return fallback;
	if (t <= c->arrKeys[0].fTime)
		return c->arrKeys[0].fValue;
	for (i = 1; i < c->iCount; i++)
		if (t < c->arrKeys[i].fTime)
		{
			const xge_particle_key_t *a = &c->arrKeys[i - 1], *b = &c->arrKeys[i];
			float span = b->fTime - a->fTime, u = (t - a->fTime) / span;
			if (c->iInterpolation == XGE_PARTICLE_CURVE_STEP)
				return a->fValue;
			if (c->iInterpolation == XGE_PARTICLE_CURVE_HERMITE)
			{
				float u2 = u * u, u3 = u2 * u;
				return (2 * u3 - 3 * u2 + 1) * a->fValue + (u3 - 2 * u2 + u) * span * a->fOutTangent +
				       (-2 * u3 + 3 * u2) * b->fValue + (u3 - u2) * span * b->fInTangent;
			}
			return a->fValue + (b->fValue - a->fValue) * u;
		}
	return c->arrKeys[c->iCount - 1].fValue;
}
static int xp_gradient_valid(const xge_particle_gradient_t *g)
{
	int i;
	if (g->iCount < 0 || g->iCount > XGE_PARTICLE_MAX_KEYS)
		return 0;
	for (i = 0; i < g->iCount; i++)
		if (!xp_finite(g->arrKeys[i].fTime) || g->arrKeys[i].fTime < 0 || g->arrKeys[i].fTime > 1 ||
		    (i && g->arrKeys[i].fTime <= g->arrKeys[i - 1].fTime))
			return 0;
	return 1;
}
uint32_t xgeParticleGradientEval(const xge_particle_gradient_t *g, float t, uint32_t fallback)
{
	int i;
	if (!g || !xp_gradient_valid(g) || !isfinite(t) || !g->iCount)
		return fallback;
	if (t <= g->arrKeys[0].fTime)
		return g->arrKeys[0].iColor;
	for (i = 1; i < g->iCount; i++)
		if (t < g->arrKeys[i].fTime)
		{
			const xge_particle_color_key_t *a = &g->arrKeys[i - 1], *b = &g->arrKeys[i];
			return __xgeMathColorLerp(a->iColor, b->iColor, (t - a->fTime) / (b->fTime - a->fTime));
		}
	return g->arrKeys[g->iCount - 1].iColor;
}
static int xp_range_valid(xge_particle_range_t r, float min, float max)
{
	return xp_finite(r.fMin) && xp_finite(r.fMax) && r.fMin >= min && r.fMax <= max && r.fMin <= r.fMax;
}
static int xp_emitter_valid(const xge_particle_emitter_t *e, int count)
{
	int i;
	const float finite[] = {e->fDelay,
	                        e->fDuration,
	                        e->fRate,
	                        e->fRateOverDistance,
	                        e->fRadius,
	                        e->fInnerRadius,
	                        e->fDirection,
	                        e->fSpread,
	                        e->fDrag,
	                        e->fRadialAcceleration,
	                        e->fTangentialAcceleration,
	                        e->fInheritVelocity,
	                        e->fNoiseStrength,
	                        e->fNoiseFrequency,
	                        e->fNoiseScroll,
	                        e->fAspect,
	                        e->fStretch,
	                        e->fFramesPerSecond,
	                        e->fCollisionRadius,
	                        e->fRestitution,
	                        e->fFriction,
	                        e->tTextureRect.fX,
	                        e->tTextureRect.fY,
	                        e->tTextureRect.fW,
	                        e->tTextureRect.fH};
	for (i = 0; i < (int)(sizeof(finite) / sizeof(finite[0])); i++)
		if (!xp_finite(finite[i]))
			return 0;
	if (!memchr(e->sName, 0, sizeof(e->sName)) || !memchr(e->sTexture, 0, sizeof(e->sTexture)) ||
	    !memchr(e->sMaterial, 0, sizeof(e->sMaterial)))
		return 0;
	if (!e->iMaxParticles || e->iMaxParticles > XP_LIMIT || e->iShape < 0 || e->iShape > 5 || e->iSpace < 0 ||
	    e->iSpace > 1 || e->fDelay < 0 || e->fDuration < 0 || (e->fDuration > 0 && e->fDuration < .001f) ||
	    e->fRate < 0 || e->fRate > XP_LIMIT || e->fRateOverDistance < 0 || e->fRateOverDistance > XP_LIMIT ||
	    e->fRadius < 0 || e->fInnerRadius < 0 || e->fInnerRadius > e->fRadius || e->fSpread < 0 ||
	    e->fSpread > 2 * XP_PI || e->fDrag < 0 || e->fNoiseFrequency < 0 || e->fAspect <= 0 ||
	    e->fStretch < 0 || e->fFramesPerSecond < 0 || e->iColumns < 1 || e->iRows < 1 || e->iColumns > 256 ||
	    e->iRows > 256 || e->iCollision < 0 || e->iCollision > 2 || e->fCollisionRadius < 0 ||
	    e->fRestitution < 0 || e->fRestitution > 1 || e->fFriction < 0 || e->fFriction > 1 ||
	    e->iBlend < XGE_BLEND_NONE || e->iBlend > XGE_BLEND_SCREEN || e->iBurstCount < 0 ||
	    e->iBurstCount > XGE_PARTICLE_MAX_BURSTS)
		return 0;
	if (!xp_vec_valid(e->tOffset) || !xp_vec_valid(e->tShapeSize) || !xp_vec_valid(e->tGravity) ||
	    !xp_range_valid(e->tLife, .0001f, 3600) || !xp_range_valid(e->tSpeed, 0, 1e6f) ||
	    !xp_range_valid(e->tSize, 0, 1e6f) || !xp_range_valid(e->tRotation, -1e6f, 1e6f) ||
	    !xp_range_valid(e->tAngularVelocity, -1e6f, 1e6f) || !xp_range_valid(e->tStartFrame, 0, 65535) ||
	    !xp_curve_valid(&e->tSizeOverLife) || !xp_curve_valid(&e->tSpeedOverLife) ||
	    !xp_curve_valid(&e->tAlphaOverLife) || !xp_gradient_valid(&e->tColorOverLife))
		return 0;
	for (i = 0; i < e->iBurstCount; i++)
		if (!xp_finite(e->arrBursts[i].fTime) || e->arrBursts[i].fTime < 0 ||
		    (e->fDuration > 0 && e->arrBursts[i].fTime >= e->fDuration) || e->arrBursts[i].iCount > XP_LIMIT)
			return 0;
	for (i = 0; i < 3; i++)
		if (e->arrSubEmitters[i].iEmitter < -1 || e->arrSubEmitters[i].iEmitter >= count ||
		    e->arrSubEmitters[i].iCount > XP_LIMIT || !xp_finite(e->arrSubEmitters[i].fInheritVelocity))
			return 0;
	return 1;
}
static int xp_dag(const xge_particle_emitter_t *e, int n, int *marks)
{
	int i;
	if (marks[n] == 1)
		return 0;
	if (marks[n] == 2)
		return 1;
	marks[n] = 1;
	for (i = 0; i < 3; i++)
		if (e[n].arrSubEmitters[i].iEmitter >= 0 && e[n].arrSubEmitters[i].iCount &&
		    !xp_dag(e, e[n].arrSubEmitters[i].iEmitter, marks))
			return 0;
	marks[n] = 2;
	return 1;
}
int xgeParticleDefinitionCreate(xge_particle_definition *out, const xge_particle_emitter_t *e, int count)
{
	xge_particle_definition d;
	int i, marks[XGE_PARTICLE_MAX_EMITTERS] = {0};
	uint64_t capacity = 0;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (!e || count < 1 || count > XGE_PARTICLE_MAX_EMITTERS)
		return XGE_ERROR_INVALID_ARGUMENT;
	for (i = 0; i < count; i++)
	{
		if (!xp_emitter_valid(&e[i], count))
			return XGE_ERROR_INVALID_ARGUMENT;
		capacity += e[i].iMaxParticles;
	}
	if (capacity > XP_LIMIT)
		return XGE_ERROR_INVALID_ARGUMENT;
	for (i = 0; i < count; i++)
		if (!xp_dag(e, i, marks))
			return XGE_ERROR_INVALID_ARGUMENT;
	d = (xge_particle_definition)xrtCalloc(1, sizeof(*d));
	if (!d)
		return XGE_ERROR_OUT_OF_MEMORY;
	xrtArrayInit(&d->emitters, sizeof(*e));
	if (!xrtArrayAppend(&d->emitters, e, (size_t)count))
	{
		xrtFree(d);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	d->iRefs = 1;
	d->capacity = (uint32_t)capacity;
	*out = d;
	return XGE_OK;
}
void xgeParticleDefinitionAddRef(xge_particle_definition d)
{
	if (d)
		(void)xrtRefRetain(&d->iRefs);
}
void xgeParticleDefinitionFree(xge_particle_definition d)
{
	if (d && xrtRefRelease(&d->iRefs) == 0)
	{
		xrtArrayUnit(&d->emitters);
		xrtFree(d);
	}
}
int xgeParticleDefinitionCount(xge_particle_definition d)
{
	return d ? (int)d->emitters.Count : 0;
}
int xgeParticleDefinitionGet(xge_particle_definition d, int i, xge_particle_emitter_t *out)
{
	if (!d || !out || i < 0 || (size_t)i >= d->emitters.Count)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = ((xge_particle_emitter_t *)d->emitters.Data)[i];
	return XGE_OK;
}

int xgeParticleWorldCreate(xge_particle_world *out, const xge_particle_world_desc_t *input)
{
	xge_particle_world w;
	xge_particle_world_desc_t d;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	xgeParticleWorldDescInit(&d);
	if (input)
		d = *input;
	if (!d.iMaxEffects || d.iMaxEffects > 65536 || !d.iMaxParticles || d.iMaxParticles > XP_LIMIT ||
	    d.iMaxEvents > XP_LIMIT || !d.iMaxSpawnPerStep || d.iMaxSpawnPerStep > XP_LIMIT || !d.iMaxSubsteps ||
	    d.iMaxSubsteps > 4096 || !xp_finite(d.fFixedStep) || d.fFixedStep < .0001f || d.fFixedStep > 1)
		return XGE_ERROR_INVALID_ARGUMENT;
	w = (xge_particle_world)xrtCalloc(1, sizeof(*w));
	if (!w)
		return XGE_ERROR_OUT_OF_MEMORY;
	w->desc = d;
	xrtPoolInit(&w->pool, sizeof(xp_effect));
	xrtSlotMapInit(&w->handles);
	xrtArrayInit(&w->effects, sizeof(xp_effect *));
	xrtArrayInit(&w->events, sizeof(xge_particle_event_t));
	xrtArrayInit(&w->spawns, sizeof(xp_spawn));
	if (!xrtArrayReserve(&w->effects, d.iMaxEffects) || !xrtSlotMapReserve(&w->handles, d.iMaxEffects) ||
	    !xrtArrayResize(&w->events, d.iMaxEvents) || !xrtArrayReserve(&w->spawns, d.iMaxSpawnPerStep))
	{
		xgeParticleWorldFree(w);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*out = w;
	return XGE_OK;
}
void xgeParticleWorldFree(xge_particle_world w)
{
	size_t i;
	if (!w || w->busy)
		return;
	for (i = 0; i < w->effects.Count; i++)
	{
		xp_effect *f = ((xp_effect **)w->effects.Data)[i];
		xgeParticleDefinitionFree(f->definition);
		xrtArrayUnit(&f->particles);
	}
	xrtArrayUnit(&w->effects);
	xrtArrayUnit(&w->events);
	xrtArrayUnit(&w->spawns);
	xrtSlotMapUnit(&w->handles);
	xrtPoolUnit(&w->pool);
	xrtFree(w);
}
static xp_effect *xp_cache(xge_particle_world w, xge_particle_definition d, int forceNew)
{
	size_t i;
	xp_effect *f = NULL;
	if (!forceNew)
	{
		for (i = 0; i < w->effects.Count; i++)
		{
			xp_effect *item = ((xp_effect **)w->effects.Data)[i];
			if (!item->handle && item->definition == d)
			{
				f = item;
				break;
			}
		}
		if (!f)
			for (i = 0; i < w->effects.Count; i++)
			{
				xp_effect *item = ((xp_effect **)w->effects.Data)[i];
				if (!item->handle)
				{
					f = item;
					break;
				}
			}
	}
	else
	{
		/* Reserve counts already-warm entries; recycle a different definition first. */
		for (i = 0; i < w->effects.Count; i++)
		{
			xp_effect *item = ((xp_effect **)w->effects.Data)[i];
			if (!item->handle && item->definition != d)
			{
				f = item;
				break;
			}
		}
	}
	if (!f)
	{
		if (w->effects.Count >= w->desc.iMaxEffects)
			return NULL;
		f = (xp_effect *)xrtPoolCalloc(&w->pool);
		if (!f)
			return NULL;
		xrtArrayInit(&f->particles, sizeof(xp_particle));
		if (!xrtArrayPush(&w->effects, &f))
		{
			xrtPoolFree(&w->pool, f);
			return NULL;
		}
	}
	if (!xrtArrayReserve(&f->particles, d->capacity))
		return NULL;
	if (f->definition != d)
	{
		xgeParticleDefinitionAddRef(d);
		xgeParticleDefinitionFree(f->definition);
		f->definition = d;
	}
	return f;
}
int xgeParticleWorldReserve(xge_particle_world w, xge_particle_definition d, uint32_t count)
{
	size_t i;
	uint32_t available = 0;
	if (!w || !d || count > w->desc.iMaxEffects)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	for (i = 0; i < w->effects.Count; i++)
	{
		xp_effect *f = ((xp_effect **)w->effects.Data)[i];
		if (!f->handle && f->definition == d)
			available++;
	}
	while (available < count)
	{
		if (!xp_cache(w, d, 1))
			return XGE_ERROR_OUT_OF_MEMORY;
		available++;
	}
	return XGE_OK;
}
static void xp_reset(xge_particle_world w, xp_effect *f, uint64_t seed)
{
	size_t i;
	w->stats.iLiveParticles -= (uint32_t)f->particles.Count;
	xrtArrayClear(&f->particles);
	memset(f->emitters, 0, sizeof(f->emitters));
	for (i = 0; i < f->definition->emitters.Count; i++)
		xrtRngSeed(&f->emitters[i].rng, seed, (uint64_t)i + 1);
	f->play.iSeed = seed;
	f->state = f->resumeState = XGE_PARTICLE_PLAYING;
	f->previous = f->current = f->play.tTransform;
}
int xgeParticlePlay(xge_particle_world w, xge_particle_definition d, const xge_particle_play_t *input,
                    xge_particle_effect *out)
{
	xp_effect *f;
	xge_particle_play_t p;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = 0;
	if (!w || !d)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	xgeParticlePlayInit(&p);
	if (input)
		p = *input;
	if (!xp_transform_valid(&p.tTransform) || !xp_parameters_valid(&p.tParameters) || p.iCullPolicy < 0 ||
	    p.iCullPolicy > 2)
		return XGE_ERROR_INVALID_ARGUMENT;
	f = xp_cache(w, d, 0);
	if (!f)
		return XGE_ERROR_OUT_OF_MEMORY;
	f->handle = xrtSlotMapInsert(&w->handles, f);
	if (!f->handle)
		return XGE_ERROR_OUT_OF_MEMORY;
	f->play = p;
	f->visible = 1;
	xp_reset(w, f, p.iSeed);
	*out = f->handle;
	return XGE_OK;
}
int xgeParticleRelease(xge_particle_world w, xge_particle_effect h)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	w->stats.iLiveParticles -= (uint32_t)f->particles.Count;
	xrtArrayClear(&f->particles);
	xrtSlotMapRemove(&w->handles, h, NULL);
	f->handle = 0;
	return XGE_OK;
}
int xgeParticleRestart(xge_particle_world w, xge_particle_effect h, uint64_t seed)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	xp_reset(w, f, seed);
	return XGE_OK;
}
int xgeParticlePause(xge_particle_world w, xge_particle_effect h, int paused)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy || f->state == XGE_PARTICLE_FINISHED)
		return XGE_ERROR_INVALID_STATE;
	if (paused && f->state != XGE_PARTICLE_PAUSED)
	{
		f->resumeState = f->state;
		f->state = XGE_PARTICLE_PAUSED;
	}
	if (!paused && f->state == XGE_PARTICLE_PAUSED)
	{
		f->state = f->resumeState;
		f->previous = f->current = f->play.tTransform;
	}
	return XGE_OK;
}
int xgeParticleStop(xge_particle_world w, xge_particle_effect h, int clear)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	f->state = XGE_PARTICLE_DRAINING;
	if (clear)
	{
		size_t i;
		w->stats.iLiveParticles -= (uint32_t)f->particles.Count;
		xrtArrayClear(&f->particles);
		for (i = 0; i < f->definition->emitters.Count; i++)
			f->emitters[i].live = 0;
	}
	return XGE_OK;
}
int xgeParticleSetTransform(xge_particle_world w, xge_particle_effect h, const xge_particle_transform_t *t)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	if (!xp_transform_valid(t))
		return XGE_ERROR_INVALID_ARGUMENT;
	f->play.tTransform = *t;
	if (f->state == XGE_PARTICLE_PAUSED)
		f->previous = f->current = *t;
	return XGE_OK;
}
int xgeParticleSetParameters(xge_particle_world w, xge_particle_effect h, const xge_particle_parameters_t *p)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	if (!xp_parameters_valid(p))
		return XGE_ERROR_INVALID_ARGUMENT;
	f->play.tParameters = *p;
	return XGE_OK;
}
int xgeParticleSetVisible(xge_particle_world w, xge_particle_effect h, int visible)
{
	xp_effect *f = xp_find(w, h);
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	f->visible = !!visible;
	return XGE_OK;
}
int xgeParticleState(xge_particle_world w, xge_particle_effect h)
{
	xp_effect *f = xp_find(w, h);
	return f ? f->state : XGE_ERROR_NOT_FOUND;
}

static uint32_t xp_tint(uint32_t a, uint32_t b)
{
	return XGE_COLOR_RGBA((XGE_COLOR_GET_R(a) * XGE_COLOR_GET_R(b) + 127) / 255,
	                      (XGE_COLOR_GET_G(a) * XGE_COLOR_GET_G(b) + 127) / 255,
	                      (XGE_COLOR_GET_B(a) * XGE_COLOR_GET_B(b) + 127) / 255,
	                      (XGE_COLOR_GET_A(a) * XGE_COLOR_GET_A(b) + 127) / 255);
}
static void xp_snapshot(xp_effect *f, const xp_particle *p, xge_particle_snapshot_t *out)
{
	xge_particle_emitter_t *e = xp_desc(f, p->emitter);
	float t = p->age / p->life, size = p->size * f->play.tParameters.fSizeScale *
	                                   fmaxf(0, xgeParticleCurveEval(&e->tSizeOverLife, t, 1));
	float alpha = fminf(1, fmaxf(0, xgeParticleCurveEval(&e->tAlphaOverLife, t, 1)));
	xge_vec2_t scale = e->iSpace == XGE_PARTICLE_SPACE_LOCAL ? f->current.tScale : p->scale;
	memset(out, 0, sizeof(*out));
	out->iEffect = f->handle;
	out->pDefinition = f->definition;
	out->iParticle = p->serial;
	out->iEmitter = p->emitter;
	out->fAge = p->age;
	out->fLife = p->life;
	out->tPosition = e->iSpace == XGE_PARTICLE_SPACE_LOCAL
	                     ? __xgeMath2DPoint(xp_matrix(f->current), p->position)
	                     : p->position;
	out->tVelocity = e->iSpace == XGE_PARTICLE_SPACE_LOCAL ? xp_vector(f->current, p->velocity) : p->velocity;
	out->fRotation =
	    p->rotation + p->spin * p->age + (e->iSpace == XGE_PARTICLE_SPACE_LOCAL ? f->current.fRotation : 0);
	if (e->bAlignVelocity && xp_length(out->tVelocity) > 1e-6f)
		out->fRotation = atan2f(out->tVelocity.fY, out->tVelocity.fX);
	/* Keep public bounds finite even for extreme combinations of valid controls. */
	out->tSize = (xge_vec2_t){
	    (float)fmin(1e12, (double)size * scale.fX * (1 + (double)e->fStretch * xp_length(out->tVelocity))),
	    (float)fmin(1e12, (double)size * e->fAspect * scale.fY)};
	out->fFrame = p->startFrame +
	              (e->fFramesPerSecond > 0 ? p->age * e->fFramesPerSecond : t * (e->iColumns * e->iRows));
	out->iColor = xp_tint(xp_tint(p->color, xgeParticleGradientEval(&e->tColorOverLife, t, 0xffffffffu)),
	                      f->play.tParameters.iTint);
	out->iColor = (out->iColor & ~XGE_COLOR_RGBA(0, 0, 0, 255)) |
	              XGE_COLOR_RGBA(0, 0, 0, (int)(XGE_COLOR_GET_A(out->iColor) * alpha + .5f));
}
static void xp_event(xge_particle_world w, xp_effect *f, int type, const xp_particle *p)
{
	xge_particle_event_t event;
	xge_particle_snapshot_t snapshot;
	memset(&event, 0, sizeof(event));
	event.iType = type;
	event.iEffect = f->handle;
	event.iEmitter = -1;
	if (p)
	{
		xp_snapshot(f, p, &snapshot);
		event.iEmitter = p->emitter;
		event.iParticle = p->serial;
		event.tPosition = snapshot.tPosition;
		event.tVelocity = snapshot.tVelocity;
		if (type < 3)
		{
			xge_particle_subemitter_t link = xp_desc(f, p->emitter)->arrSubEmitters[type];
			if (link.iEmitter >= 0 && link.iCount)
			{
				xp_spawn spawn = {link.iEmitter, link.iCount, event.tPosition, event.tVelocity,
				                  link.fInheritVelocity};
				if (w->spawns.Count < w->desc.iMaxSpawnPerStep)
					xrtArrayPush(&w->spawns, &spawn);
				else
					xp_drop(w, link.iCount);
			}
		}
	}
	if (w->suppressEvents || !w->desc.iMaxEvents)
		return;
	if (w->eventCount >= w->desc.iMaxEvents)
	{
		w->stats.iDroppedEvents++;
		return;
	}
	((xge_particle_event_t *)w->events.Data)[(w->eventHead + w->eventCount) % w->desc.iMaxEvents] = event;
	w->eventCount++;
}
static float xp_random(xrng *rng, xge_particle_range_t r)
{
	return r.fMin + (r.fMax - r.fMin) * (float)xrtRngReal(rng);
}
static xge_vec2_t xp_shape(const xge_particle_emitter_t *e, xrng *rng)
{
	float a, r, u = (float)xrtRngReal(rng), v = (float)xrtRngReal(rng);
	switch (e->iShape)
	{
	case XGE_PARTICLE_SHAPE_LINE:
		return xp_mul(e->tShapeSize, u - .5f);
	case XGE_PARTICLE_SHAPE_RECT:
		return (xge_vec2_t){(u - .5f) * e->tShapeSize.fX, (v - .5f) * e->tShapeSize.fY};
	case XGE_PARTICLE_SHAPE_CIRCLE:
		a = u * 2 * XP_PI;
		r = sqrtf(v) * e->fRadius;
		break;
	case XGE_PARTICLE_SHAPE_RING:
		a = u * 2 * XP_PI;
		r = sqrtf(e->fInnerRadius * e->fInnerRadius +
		          v * (e->fRadius * e->fRadius - e->fInnerRadius * e->fInnerRadius));
		break;
	case XGE_PARTICLE_SHAPE_CONE:
		a = e->fDirection + (u - .5f) * e->fSpread;
		r = sqrtf(v) * e->fRadius;
		break;
	default:
		return (xge_vec2_t){0, 0};
	}
	return (xge_vec2_t){cosf(a) * r, sinf(a) * r};
}
static uint32_t xp_spawn_particles(xge_particle_world w, xp_effect *f, int index, uint64_t count,
                                   const xge_vec2_t *worldCenter, xge_vec2_t inherited, float inheritance,
                                   float dt)
{
	xge_particle_emitter_t *e = xp_desc(f, index);
	xp_emitter *state = &f->emitters[index];
	uint32_t n = (uint32_t)fmin((double)count, (double)(w->desc.iMaxSpawnPerStep - w->stepSpawned)), i;
	n = (uint32_t)fmin(n, e->iMaxParticles - state->live);
	n = (uint32_t)fmin(n, w->desc.iMaxParticles - w->stats.iLiveParticles);
	xp_drop(w, count - n);
	for (i = 0; i < n; i++)
	{
		xp_particle *p = (xp_particle *)xrtArrayAdd(&f->particles, 1);
		float angle, speed;
		xge_vec2_t offset, rootVelocity = {0, 0};
		if (!p)
		{
			xp_drop(w, n - i);
			return i;
		} /* Reserved before Play; defensive OOM boundary. */
		memset(p, 0, sizeof(*p));
		p->emitter = index;
		p->serial = ++w->serial;
		p->life = xp_random(&state->rng, e->tLife);
		p->size = xp_random(&state->rng, e->tSize);
		p->rotation = xp_random(&state->rng, e->tRotation);
		p->spin = xp_random(&state->rng, e->tAngularVelocity);
		p->startFrame = xp_random(&state->rng, e->tStartFrame);
		p->scale = f->current.tScale;
		p->color = __xgeMathColorLerp(e->iColorMin, e->iColorMax, (float)xrtRngReal(&state->rng));
		angle = e->fDirection + ((float)xrtRngReal(&state->rng) - .5f) * e->fSpread;
		speed = xp_random(&state->rng, e->tSpeed) * f->play.tParameters.fSpeedScale;
		p->velocity = (xge_vec2_t){cosf(angle) * speed, sinf(angle) * speed};
		offset = xp_shape(e, &state->rng);
		if (dt > 0)
			rootVelocity =
			    xp_mul(xp_sub(f->current.tPosition, f->previous.tPosition), e->fInheritVelocity / dt);
		rootVelocity = xp_add(rootVelocity, xp_mul(inherited, inheritance));
		if (e->iSpace == XGE_PARTICLE_SPACE_LOCAL)
		{
			p->origin = worldCenter ? xp_inverse(f->current, *worldCenter, 1) : e->tOffset;
			p->position = xp_add(p->origin, offset);
			p->velocity = xp_add(p->velocity, xp_inverse(f->current, rootVelocity, 0));
		}
		else
		{
			p->origin = worldCenter ? *worldCenter : __xgeMath2DPoint(xp_matrix(f->current), e->tOffset);
			p->position = xp_add(p->origin, xp_vector(f->current, offset));
			p->velocity = xp_add(xp_vector(f->current, p->velocity), rootVelocity);
			p->rotation += f->current.fRotation;
		}
		state->live++;
		w->stats.iLiveParticles++;
		w->stats.iSpawned++;
		w->stepSpawned++;
		if (w->stats.iLiveParticles > w->stats.iPeakParticles)
			w->stats.iPeakParticles = w->stats.iLiveParticles;
		xp_event(w, f, XGE_PARTICLE_EVENT_BIRTH, p);
	}
	return n;
}
static void xp_subemit(xge_particle_world w, xp_effect *f, float dt)
{
	size_t i = 0;
	while (i < w->spawns.Count)
	{
		xp_spawn spawn = ((xp_spawn *)w->spawns.Data)[i++];
		xp_spawn_particles(w, f, spawn.emitter, spawn.count, &spawn.position, spawn.velocity, spawn.inherit,
		                   dt);
	}
	xrtArrayClear(&w->spawns);
}
int xgeParticleEmit(xge_particle_world w, xge_particle_effect h, int emitter, uint32_t count,
                    uint32_t *spawned)
{
	xp_effect *f = xp_find(w, h);
	if (spawned)
		*spawned = 0;
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy || f->state != XGE_PARTICLE_PLAYING)
		return XGE_ERROR_INVALID_STATE;
	if (emitter < 0 || (size_t)emitter >= f->definition->emitters.Count)
		return XGE_ERROR_INVALID_ARGUMENT;
	w->busy = 1;
	w->stepSpawned = 0;
	xrtArrayClear(&w->spawns);
	f->current = f->play.tTransform;
	count = xp_spawn_particles(w, f, emitter, count, NULL, (xge_vec2_t){0, 0}, 0, 0);
	xp_subemit(w, f, 0);
	w->busy = 0;
	if (spawned)
		*spawned = count;
	return XGE_OK;
}
static uint32_t xp_hash(uint32_t x)
{
	x ^= x >> 16;
	x *= 0x7feb352du;
	x ^= x >> 15;
	x *= 0x846ca68bu;
	return x ^ (x >> 16);
}
static float xp_noise(float x, float y, uint32_t seed)
{
	float fx = fmodf(x, 65536), fy = fmodf(y, 65536), u, v, a, b, c, d;
	int ix = (int)floorf(fx), iy = (int)floorf(fy);
	u = fx - ix;
	v = fy - iy;
	u = u * u * (3 - 2 * u);
	v = v * v * (3 - 2 * v);
	a = (float)(xp_hash((uint32_t)ix ^ xp_hash((uint32_t)iy + seed)) & 65535) / 32767.5f - 1;
	b = (float)(xp_hash((uint32_t)(ix + 1) ^ xp_hash((uint32_t)iy + seed)) & 65535) / 32767.5f - 1;
	c = (float)(xp_hash((uint32_t)ix ^ xp_hash((uint32_t)(iy + 1) + seed)) & 65535) / 32767.5f - 1;
	d = (float)(xp_hash((uint32_t)(ix + 1) ^ xp_hash((uint32_t)(iy + 1) + seed)) & 65535) / 32767.5f - 1;
	return (a + (b - a) * u) * (1 - v) + (c + (d - c) * u) * v;
}
int xgeParticleCollidePlane(xge_vec2_t from, xge_vec2_t to, float radius, xge_vec2_t normal, float offset,
                            xge_particle_hit_t *hit)
{
	float length = xp_length(normal), a, b, t;
	if (!hit || !xp_vec_valid(from) || !xp_vec_valid(to) || !xp_vec_valid(normal) || !xp_finite(radius) ||
	    radius < 0 || !xp_finite(offset) || length < 1e-6f)
		return 0;
	normal = xp_mul(normal, 1 / length);
	offset /= length;
	a = xp_dot(from, normal) - offset - radius;
	b = xp_dot(to, normal) - offset - radius;
	if (b >= 0 && a >= 0)
		return 0;
	if (a < 0)
	{
		hit->tPosition = xp_add(from, xp_mul(normal, -a));
		t = 0;
	}
	else
	{
		t = a / (a - b);
		hit->tPosition = xp_add(from, xp_mul(xp_sub(to, from), t));
	}
	hit->tNormal = normal;
	hit->fFraction = t;
	return 1;
}
int xgeParticleCollideRect(xge_vec2_t from, xge_vec2_t to, float radius, xge_rect_t rect,
                           xge_particle_hit_t *hit)
{
	float mins[2] = {rect.fX - radius, rect.fY - radius},
	      maxs[2] = {rect.fX + rect.fW + radius, rect.fY + rect.fH + radius};
	float p[2] = {from.fX, from.fY}, v[2] = {to.fX - from.fX, to.fY - from.fY}, enter = 0, leave = 1;
	xge_vec2_t normal = {0, 0};
	int axis;
	if (!hit || !xp_vec_valid(from) || !xp_vec_valid(to) || !xp_finite(radius) || radius < 0 ||
	    !xp_finite(rect.fX) || !xp_finite(rect.fY) || !xp_finite(rect.fW) || !xp_finite(rect.fH) ||
	    rect.fW <= 0 || rect.fH <= 0)
		return 0;
	if (p[0] > mins[0] && p[0] < maxs[0] && p[1] > mins[1] && p[1] < maxs[1])
	{
		float distances[4] = {p[0] - mins[0], maxs[0] - p[0], p[1] - mins[1], maxs[1] - p[1]};
		int side = 0;
		for (axis = 1; axis < 4; axis++)
			if (distances[axis] < distances[side])
				side = axis;
		normal = side == 0   ? (xge_vec2_t){-1, 0}
		         : side == 1 ? (xge_vec2_t){1, 0}
		         : side == 2 ? (xge_vec2_t){0, -1}
		                     : (xge_vec2_t){0, 1};
		hit->tPosition = xp_add(from, xp_mul(normal, distances[side]));
		hit->tNormal = normal;
		hit->fFraction = 0;
		return 1;
	}
	for (axis = 0; axis < 2; axis++)
	{
		float a, b, sign = -1;
		if (fabsf(v[axis]) < 1e-8f)
		{
			if (p[axis] < mins[axis] || p[axis] > maxs[axis])
				return 0;
			continue;
		}
		a = (mins[axis] - p[axis]) / v[axis];
		b = (maxs[axis] - p[axis]) / v[axis];
		if (a > b)
		{
			float swap = a;
			a = b;
			b = swap;
			sign = 1;
		}
		if (a >= enter)
		{
			enter = a;
			normal = axis == 0 ? (xge_vec2_t){sign, 0} : (xge_vec2_t){0, sign};
		}
		if (b < leave)
			leave = b;
		if (enter > leave)
			return 0;
	}
	if (enter < 0 || enter > 1 || xp_length(normal) < .5f || xp_dot(xp_sub(to, from), normal) >= 0)
		return 0;
	hit->fFraction = enter;
	hit->tNormal = normal;
	hit->tPosition = xp_add(from, xp_mul(xp_sub(to, from), enter));
	return 1;
}
static void xp_integrate(xge_particle_world w, xp_effect *f, float dt)
{
	size_t i = 0;
	while (i < f->particles.Count)
	{
		xp_particle *p = &((xp_particle *)f->particles.Data)[i];
		xge_particle_emitter_t *e = xp_desc(f, p->emitter);
		xge_vec2_t old = p->position, radial = xp_sub(p->position, p->origin), force = e->tGravity;
		float length = xp_length(radial), drag = expf(-e->fDrag * dt), speedScale;
		int dead = 0;
		p->age += dt;
		if (p->age >= p->life)
			dead = 1;
		if (!dead)
		{
			if (length > 1e-6f)
			{
				radial = xp_mul(radial, 1 / length);
				force = xp_add(force, xp_mul(radial, e->fRadialAcceleration));
				force = xp_add(force, (xge_vec2_t){-radial.fY * e->fTangentialAcceleration,
				                                   radial.fX * e->fTangentialAcceleration});
			}
			if (e->fNoiseStrength != 0)
			{
				float x = p->position.fX * e->fNoiseFrequency +
				          (float)f->emitters[p->emitter].time * e->fNoiseScroll;
				float y = p->position.fY * e->fNoiseFrequency;
				force = xp_add(
				    force, (xge_vec2_t){xp_noise(x, y, (uint32_t)f->play.iSeed) * e->fNoiseStrength,
				                        xp_noise(x, y, (uint32_t)f->play.iSeed + 31337) * e->fNoiseStrength});
			}
			p->velocity = xp_mul(xp_add(p->velocity, xp_mul(force, dt)), drag);
			speedScale = fmaxf(0, xgeParticleCurveEval(&e->tSpeedOverLife, p->age / p->life, 1));
			p->position = xp_add(p->position, xp_mul(p->velocity, dt * speedScale));
			if (!xp_vec_valid(p->position) || !xp_vec_valid(p->velocity))
				dead = 1;
			if (!dead && e->iCollision && w->desc.pCollision)
			{
				xge_vec2_t from = e->iSpace ? __xgeMath2DPoint(xp_matrix(f->previous), old) : old;
				xge_vec2_t to =
				    e->iSpace ? __xgeMath2DPoint(xp_matrix(f->current), p->position) : p->position;
				xge_vec2_t velocity = e->iSpace ? xp_vector(f->current, p->velocity) : p->velocity;
				xge_particle_hit_t hit;
				xge_particle_snapshot_t snapshot;
				xp_snapshot(f, p, &snapshot);
				memset(&hit, 0, sizeof(hit));
				if (w->desc.pCollision(from, to,
				                       e->fCollisionRadius * fmaxf(snapshot.tSize.fX, snapshot.tSize.fY),
				                       &hit, w->desc.pCollisionUser) &&
				    xp_vec_valid(hit.tPosition) && xp_vec_valid(hit.tNormal) && isfinite(hit.fFraction) &&
				    hit.fFraction >= 0 && hit.fFraction <= 1 && xp_length(hit.tNormal) > 1e-6f)
				{
					float vn;
					hit.tNormal = xp_mul(hit.tNormal, 1 / xp_length(hit.tNormal));
					vn = xp_dot(velocity, hit.tNormal);
					p->position = e->iSpace ? xp_inverse(f->current, hit.tPosition, 1) : hit.tPosition;
					w->stats.iCollisions++;
					xp_event(w, f, XGE_PARTICLE_EVENT_COLLISION, p);
					if (e->iCollision == XGE_PARTICLE_COLLISION_KILL)
						dead = 1;
					else
					{
						velocity = xp_add(xp_mul(xp_sub(velocity, xp_mul(hit.tNormal, vn)), 1 - e->fFriction),
						                  xp_mul(hit.tNormal, vn < 0 ? -vn * e->fRestitution : vn));
						to = xp_add(xp_add(hit.tPosition, xp_mul(hit.tNormal, .001f)),
						            xp_mul(velocity, dt * (1 - hit.fFraction) * speedScale));
						p->position = e->iSpace ? xp_inverse(f->current, to, 1) : to;
						p->velocity = e->iSpace ? xp_inverse(f->current, velocity, 0) : velocity;
					}
				}
			}
		}
		if (dead)
		{
			/* Never forward non-finite state to events or child emitters. */
			if (xp_vec_valid(p->position) && xp_vec_valid(p->velocity))
				xp_event(w, f, XGE_PARTICLE_EVENT_DEATH, p);
			f->emitters[p->emitter].live--;
			w->stats.iLiveParticles--;
			xrtArrayRemoveSwap(&f->particles, i);
		}
		else
			i++;
	}
}
static uint64_t xp_burst_occurrences(double a, double b, double at, double duration, int loop, int first)
{
	double low, high;
	if (loop && duration > 0)
	{
		low = floor((a - at) / duration) + 1;
		if (first && a == at)
			low--;
		if (low < 0)
			low = 0;
		high = floor((b - at) / duration + 1e-9);
		return high >= low ? (uint64_t)(high - low + 1) : 0;
	}
	return ((a < at || (first && a == at)) && b >= at) ? 1 : 0;
}
static void xp_emission(xge_particle_world w, xp_effect *f, int index, float dt)
{
	xge_particle_emitter_t *e = xp_desc(f, index);
	xp_emitter *s = &f->emitters[index];
	double a = s->time - e->fDelay, b = s->time + dt - e->fDelay, active, rate, count;
	int i;
	s->time += dt;
	if (e->bAutomatic && f->state == XGE_PARTICLE_PLAYING)
	{
		active = fmax(0, b) - fmax(0, a);
		if (e->fDuration > 0 && !e->bLoop)
			active = fmin(e->fDuration, fmax(0, b)) - fmin(e->fDuration, fmax(0, a));
		for (i = 0; i < e->iBurstCount; i++)
		{
			uint64_t n =
			    xp_burst_occurrences(a, b, e->arrBursts[i].fTime, e->fDuration, e->bLoop, !s->started) *
			    e->arrBursts[i].iCount;
			if (n)
				xp_spawn_particles(w, f, index, n, NULL, (xge_vec2_t){0, 0}, 0, dt);
		}
		rate = s->rateFraction + active * e->fRate * f->play.tParameters.fRateScale;
		count = floor(rate + 1e-9);
		s->rateFraction = fmax(0, rate - count);
		if (count > 0)
			xp_spawn_particles(w, f, index, (uint64_t)count, NULL, (xge_vec2_t){0, 0}, 0, dt);
		if (active > 0 && e->fRateOverDistance > 0 && f->play.tParameters.fRateScale > 0)
		{
			xge_vec2_t from = __xgeMath2DPoint(xp_matrix(f->previous), e->tOffset),
			           to = __xgeMath2DPoint(xp_matrix(f->current), e->tOffset);
			double density = e->fRateOverDistance * f->play.tParameters.fRateScale;
			xge_vec2_t segment = xp_sub(to, from);
			double begin = fmax(0, -a) / dt, end = begin + active / dt, amount, old = s->distanceFraction,
			       whole;
			uint64_t n, j, limit;
			/* A delayed/ending emitter covers only the active part of this movement. */
			to = xp_add(from, xp_mul(segment, (float)end));
			from = xp_add(from, xp_mul(segment, (float)begin));
			amount = xp_length(xp_sub(to, from)) * density;
			whole = floor(old + amount + 1e-9);
			n = whole >= (double)UINT64_MAX ? UINT64_MAX : (uint64_t)whole;
			limit = n;
			s->distanceFraction = fmax(0, old + amount - whole);
			if (limit > w->desc.iMaxSpawnPerStep - w->stepSpawned)
				limit = w->desc.iMaxSpawnPerStep - w->stepSpawned;
			xp_drop(w, n - limit);
			for (j = 0; j < limit; j++)
			{
				float t = amount > 0 ? (float)((1 - old + j) / amount) : 1;
				xge_vec2_t center = xp_add(from, xp_mul(xp_sub(to, from), fminf(1, t)));
				xp_spawn_particles(w, f, index, 1, &center, (xge_vec2_t){0, 0}, 0, dt);
			}
		}
	}
	s->started = 1;
}
static int xp_future(xp_effect *f)
{
	size_t i;
	int j;
	if (f->state != XGE_PARTICLE_PLAYING)
		return 0;
	for (i = 0; i < f->definition->emitters.Count; i++)
	{
		xge_particle_emitter_t *e = xp_desc(f, (int)i);
		double t = f->emitters[i].time - e->fDelay;
		if (!e->bAutomatic)
			continue;
		if (e->fDuration > 0 && !e->bLoop && t >= e->fDuration)
			continue;
		if (e->fRate > 0 || e->fRateOverDistance > 0)
			return 1;
		if (e->bLoop && e->fDuration > 0 && e->iBurstCount)
			return 1;
		for (j = 0; j < e->iBurstCount; j++)
			if (e->arrBursts[j].iCount && (!f->emitters[i].started || t < e->arrBursts[j].fTime))
				return 1;
	}
	return 0;
}
static void xp_step(xge_particle_world w, xp_effect *f, float dt)
{
	size_t i;
	if (f->state == XGE_PARTICLE_PAUSED || f->state == XGE_PARTICLE_FINISHED)
		return;
	if (f->state == XGE_PARTICLE_DRAINING && !f->particles.Count)
	{
		f->state = XGE_PARTICLE_FINISHED;
		xp_event(w, f, XGE_PARTICLE_EVENT_FINISHED, NULL);
		return;
	}
	if (dt <= 0)
		return;
	if (!f->visible && f->play.iCullPolicy == XGE_PARTICLE_CULL_PAUSE)
		return;
	if (!f->visible && f->play.iCullPolicy == XGE_PARTICLE_CULL_CLEAR)
	{
		w->stats.iLiveParticles -= (uint32_t)f->particles.Count;
		xrtArrayClear(&f->particles);
		for (i = 0; i < f->definition->emitters.Count; i++)
		{
			f->emitters[i].live = 0;
			f->emitters[i].time += dt;
			f->emitters[i].started = 1;
			f->emitters[i].rateFraction = f->emitters[i].distanceFraction = 0;
		}
		if (!xp_future(f))
		{
			f->state = XGE_PARTICLE_FINISHED;
			xp_event(w, f, XGE_PARTICLE_EVENT_FINISHED, NULL);
		}
		return; /* Discard while offscreen, advance clocks, never catch up missed births. */
	}
	xrtArrayClear(&w->spawns);
	xp_integrate(w, f, dt);
	for (i = 0; i < f->definition->emitters.Count; i++)
		xp_emission(w, f, (int)i, dt);
	xp_subemit(w, f, dt);
	if (!f->particles.Count && !xp_future(f))
	{
		f->state = XGE_PARTICLE_FINISHED;
		xp_event(w, f, XGE_PARTICLE_EVENT_FINISHED, NULL);
	}
}
int xgeParticleUpdate(xge_particle_world w, double delta)
{
	uint32_t steps, k;
	size_t i;
	double total, available, start;
	if (!w || !isfinite(delta) || delta < 0 || delta > 3600)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	start = xrtTimer();
	total = w->remainder + delta;
	available = floor(total / w->desc.fFixedStep + 1e-9);
	steps = (uint32_t)fmin(available, w->desc.iMaxSubsteps);
	w->remainder = fmax(0, total - available * w->desc.fFixedStep);
	w->stats.fDroppedTime += (available - steps) * w->desc.fFixedStep;
	w->busy = 1;
	for (k = 0; k < steps; k++)
	{
		w->stepSpawned = 0;
		for (i = 0; i < w->effects.Count; i++)
		{
			xp_effect *f = ((xp_effect **)w->effects.Data)[i];
			float fraction = 1.0f / (float)(steps - k);
			if (!f->handle)
				continue;
			f->previous = f->current;
			f->current.tPosition =
			    xp_add(f->current.tPosition,
			           xp_mul(xp_sub(f->play.tTransform.tPosition, f->current.tPosition), fraction));
			f->current.tScale = xp_add(
			    f->current.tScale, xp_mul(xp_sub(f->play.tTransform.tScale, f->current.tScale), fraction));
			f->current.fRotation += (f->play.tTransform.fRotation - f->current.fRotation) * fraction;
			xp_step(w, f, w->desc.fFixedStep * f->play.tParameters.fTimeScale);
		}
		w->stats.iSteps++;
	}
	w->busy = 0;
	for (i = 0; i < w->effects.Count; i++)
	{
		xp_effect *f = ((xp_effect **)w->effects.Data)[i];
		if (f->handle && f->state == XGE_PARTICLE_FINISHED && f->play.bAutoRelease)
			xgeParticleRelease(w, f->handle);
	}
	w->stats.fLastUpdateSeconds = xrtTimer() - start;
	return XGE_OK;
}
int xgeParticlePrewarm(xge_particle_world w, xge_particle_effect h, float seconds)
{
	xp_effect *f = xp_find(w, h);
	double time = 0;
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (w->busy || f->state != XGE_PARTICLE_PLAYING)
		return XGE_ERROR_INVALID_STATE;
	if (!xp_finite(seconds) || seconds < 0 || seconds > 60 || seconds / w->desc.fFixedStep > 100000)
		return XGE_ERROR_INVALID_ARGUMENT;
	w->busy = 1;
	w->suppressEvents = 1;
	f->previous = f->current = f->play.tTransform;
	while (time < seconds && f->state != XGE_PARTICLE_FINISHED)
	{
		float dt = (float)fmin(w->desc.fFixedStep, seconds - time);
		w->stepSpawned = 0;
		xp_step(w, f, dt * f->play.tParameters.fTimeScale);
		time += dt;
		w->stats.iSteps++;
	}
	w->suppressEvents = 0;
	w->busy = 0;
	return XGE_OK;
}
int xgeParticleVisit(xge_particle_world w, xge_particle_visit_proc visit, void *user)
{
	size_t i, j;
	int keep = 1;
	if (!w || !visit)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	w->busy = 1;
	for (i = 0; i < w->effects.Count && keep; i++)
	{
		xp_effect *f = ((xp_effect **)w->effects.Data)[i];
		if (!f->handle)
			continue;
		for (j = 0; j < f->particles.Count && keep; j++)
		{
			xge_particle_snapshot_t snapshot;
			xp_snapshot(f, &((xp_particle *)f->particles.Data)[j], &snapshot);
			keep = visit(&snapshot, user);
		}
	}
	w->busy = 0;
	return XGE_OK;
}
int xgeParticleBounds(xge_particle_world w, xge_particle_effect h, xge_rect_t *bounds)
{
	xp_effect *f = xp_find(w, h);
	size_t i;
	float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
	if (!f)
		return XGE_ERROR_NOT_FOUND;
	if (!bounds)
		return XGE_ERROR_INVALID_ARGUMENT;
	for (i = 0; i < f->particles.Count; i++)
	{
		xge_particle_snapshot_t s;
		float width, height, c, sine;
		xp_snapshot(f, &((xp_particle *)f->particles.Data)[i], &s);
		c = fabsf(cosf(s.fRotation));
		sine = fabsf(sinf(s.fRotation));
		width = (s.tSize.fX * c + s.tSize.fY * sine) * .5f;
		height = (s.tSize.fX * sine + s.tSize.fY * c) * .5f;
		left = fminf(left, s.tPosition.fX - width);
		right = fmaxf(right, s.tPosition.fX + width);
		top = fminf(top, s.tPosition.fY - height);
		bottom = fmaxf(bottom, s.tPosition.fY + height);
	}
	*bounds = f->particles.Count ? (xge_rect_t){left, top, right - left, bottom - top}
	                             : (xge_rect_t){f->current.tPosition.fX, f->current.tPosition.fY, 0, 0};
	return XGE_OK;
}
int xgeParticleEventPoll(xge_particle_world w, xge_particle_event_t *event)
{
	if (!w || !event)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (w->busy)
		return XGE_ERROR_INVALID_STATE;
	if (!w->eventCount)
		return 0;
	*event = ((xge_particle_event_t *)w->events.Data)[w->eventHead];
	w->eventHead = (w->eventHead + 1) % w->desc.iMaxEvents;
	w->eventCount--;
	return 1;
}
xge_particle_stats_t xgeParticleStats(xge_particle_world w)
{
	xge_particle_stats_t s = {0};
	if (!w)
		return s;
	s = w->stats;
	s.iActiveEffects = (uint32_t)w->handles.Count;
	s.iCachedEffects = (uint32_t)w->effects.Count - s.iActiveEffects;
	s.iPendingEvents = w->eventCount;
	return s;
}
