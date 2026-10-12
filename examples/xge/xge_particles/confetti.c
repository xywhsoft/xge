#include "demo.h"
void demo_configure(demo_t *d)
{
	static const uint32_t colors[] = {0xff7869ff, 0x74d9caff, 0xffd475ff, 0xaa9cffff};
	int i;
	d->title = "07 / UI CONFETTI";
	d->features = "Screen-space quads | timed dual bursts | random spin + aspect | gravity + drag | pause / "
	              "drain / clear";
	d->kind = 6;
	d->count = 4;
	d->interval = 4;
	d->origin = (xge_vec2_t){500, 535};
	for (i = 0; i < 4; i++)
	{
		xge_particle_emitter_t *e = &d->emitters[i];
		xgeParticleEmitterInit(e);
		demo_burst(e, 100);
		e->arrBursts[0].fTime = i * .025f;
		e->tOffset = (xge_vec2_t){i % 2 ? 320 : -320, 0};
		e->iShape = XGE_PARTICLE_SHAPE_RECT;
		e->tShapeSize = (xge_vec2_t){25, 12};
		e->fDirection = i % 2 ? -2.15f : -.99f;
		e->fSpread = .75f;
		e->tLife = (xge_particle_range_t){2.5f, 4};
		e->tSpeed = (xge_particle_range_t){290, 470};
		e->tSize = (xge_particle_range_t){5, 12};
		e->fAspect = i % 2 ? .45f : 1.4f;
		e->tAngularVelocity = (xge_particle_range_t){-9, 9};
		e->tRotation = (xge_particle_range_t){0, 2 * DEMO_PI};
		e->tGravity = (xge_vec2_t){0, 275};
		e->fDrag = .5f;
		e->fNoiseStrength = 20;
		e->bScreenSpace = 1;
		e->iColorMin = colors[i];
		e->iColorMax = colors[i];
		e->iLayer = 10;
		demo_curve(&e->tAlphaOverLife, 1, 0);
		e->tAlphaOverLife.arrKeys[0].fTime = .6f;
	}
}
