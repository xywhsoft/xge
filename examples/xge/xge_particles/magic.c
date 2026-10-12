#include "demo.h"
void demo_configure(demo_t *d)
{
	xge_particle_emitter_t *e = d->emitters;
	d->title = "06 / MAGIC HALO";
	d->features = "Ring emission | local-space rotation | radial + tangential force | custom batched shader "
	              "| Hermite curve";
	d->kind = 5;
	d->count = 3;
	d->warmup = 1;
	d->origin = (xge_vec2_t){500, 375};
	xgeParticleEmitterInit(&e[0]);
	e[0].fRate = 140;
	e[0].iShape = XGE_PARTICLE_SHAPE_RING;
	e[0].fRadius = 155;
	e[0].fInnerRadius = 145;
	e[0].iSpace = XGE_PARTICLE_SPACE_LOCAL;
	e[0].tLife = (xge_particle_range_t){.8f, 1.8f};
	e[0].tSpeed = (xge_particle_range_t){0, 10};
	e[0].fSpread = 2 * DEMO_PI;
	e[0].tSize = (xge_particle_range_t){3, 9};
	e[0].fRadialAcceleration = -22;
	e[0].fTangentialAcceleration = 110;
	e[0].fDrag = .35f;
	e[0].iBlend = XGE_BLEND_ADD;
	demo_gradient(&e[0].tColorOverLife, 0x69f5dfff, 0x80a2ffaa, 0xc981ff00);
	strcpy(e[0].sTexture, "soft");
	demo_curve(&e[0].tSizeOverLife, .1f, 1);
	e[0].tSizeOverLife.iInterpolation = XGE_PARTICLE_CURVE_HERMITE;
	e[0].tSizeOverLife.arrKeys[0].fOutTangent = 3;
	e[0].tSizeOverLife.arrKeys[1].fInTangent = -2;
	xgeParticleEmitterInit(&e[1]);
	demo_burst(&e[1], 1);
	e[1].tLife = (xge_particle_range_t){3600, 3600};
	e[1].tSpeed = (xge_particle_range_t){0, 0};
	e[1].tSize = (xge_particle_range_t){360, 360};
	e[1].iSpace = XGE_PARTICLE_SPACE_LOCAL;
	e[1].iLayer = 1;
	e[1].iBlend = XGE_BLEND_ADD;
	e[1].iColorMin = e[1].iColorMax = 0x6bb5e0aa;
	strcpy(e[1].sMaterial, "ring");
	xgeParticleEmitterInit(&e[2]);
	e[2].fRate = 30;
	e[2].iShape = XGE_PARTICLE_SHAPE_CIRCLE;
	e[2].fRadius = 90;
	e[2].tLife = (xge_particle_range_t){1, 2};
	e[2].tSpeed = (xge_particle_range_t){12, 30};
	e[2].tSize = (xge_particle_range_t){3, 7};
	e[2].fNoiseStrength = 18;
	e[2].iBlend = XGE_BLEND_SCREEN;
	e[2].iLayer = 2;
	demo_gradient(&e[2].tColorOverLife, 0xc4a1ffff, 0x96d3f7bb, 0x90caff00);
	strcpy(e[2].sTexture, "soft");
}
