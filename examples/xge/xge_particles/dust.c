#include "demo.h"
void demo_configure(demo_t *d)
{
	xge_particle_emitter_t *e = d->emitters;
	d->title = "05 / MOVEMENT DUST";
	d->features =
	    "Distance-based emission | world-space trail | velocity inheritance | drag | live rate override";
	d->kind = 4;
	d->count = 2;
	d->origin = (xge_vec2_t){500, 430};
	xgeParticleEmitterInit(&e[0]);
	e[0].fRate = 0;
	e[0].fRateOverDistance = .45f;
	e[0].iShape = XGE_PARTICLE_SHAPE_RECT;
	e[0].tShapeSize = (xge_vec2_t){22, 6};
	e[0].tLife = (xge_particle_range_t){.7f, 1.5f};
	e[0].tSpeed = (xge_particle_range_t){4, 18};
	e[0].fSpread = DEMO_PI;
	e[0].tSize = (xge_particle_range_t){12, 25};
	e[0].fDrag = 2;
	e[0].fInheritVelocity = .1f;
	e[0].tGravity = (xge_vec2_t){0, -8};
	e[0].fNoiseStrength = 14;
	demo_curve(&e[0].tSizeOverLife, .3f, 2.2f);
	demo_gradient(&e[0].tColorOverLife, 0xddba87bb, 0xb5a18677, 0x9d958b00);
	strcpy(e[0].sTexture, "sheet");
	e[0].iColumns = 4;
	xgeParticleEmitterInit(&e[1]);
	e[1].fRate = 0;
	e[1].fRateOverDistance = .12f;
	e[1].iShape = XGE_PARTICLE_SHAPE_LINE;
	e[1].tShapeSize = (xge_vec2_t){30, 0};
	e[1].tLife = (xge_particle_range_t){.4f, .7f};
	e[1].tSpeed = (xge_particle_range_t){10, 30};
	e[1].tSize = (xge_particle_range_t){2, 4};
	e[1].fSpread = 2 * DEMO_PI;
	e[1].fInheritVelocity = .3f;
	e[1].tGravity = (xge_vec2_t){0, 90};
	e[1].tAngularVelocity = (xge_particle_range_t){-6, 6};
	e[1].iColorMin = 0x96724bff;
	e[1].iColorMax = 0xc4a173ff;
	e[1].iLayer = 1;
	demo_curve(&e[1].tAlphaOverLife, 1, 0);
}
