#include "demo.h"
void demo_configure(demo_t *d)
{
	xge_particle_emitter_t *e = d->emitters;
	d->title = "04 / RAIN & SNOW";
	d->features =
	    "Rectangle emission | stretch vs spin | kill collision | snowfall noise | prewarm + view culling";
	d->kind = 3;
	d->count = 3;
	d->warmup = 3;
	d->origin = (xge_vec2_t){500, 135};
	xgeParticleEmitterInit(&e[0]);
	e[0].fRate = 320;
	e[0].iMaxParticles = 2048;
	e[0].iShape = XGE_PARTICLE_SHAPE_RECT;
	e[0].tOffset = (xge_vec2_t){-220, 0};
	e[0].tShapeSize = (xge_vec2_t){340, 30};
	e[0].fDirection = 1.4f;
	e[0].fSpread = .04f;
	e[0].tLife = (xge_particle_range_t){.9f, 1.2f};
	e[0].tSpeed = (xge_particle_range_t){420, 600};
	e[0].tSize = (xge_particle_range_t){1, 2};
	e[0].bAlignVelocity = 1;
	e[0].fStretch = .03f;
	e[0].iCollision = XGE_PARTICLE_COLLISION_KILL;
	e[0].iColorMin = e[0].iColorMax = 0x8bbcf399;
	demo_curve(&e[0].tAlphaOverLife, .7f, .3f);
	strcpy(e[0].sTexture, "soft");
	e[0].arrSubEmitters[XGE_PARTICLE_EVENT_COLLISION] = (xge_particle_subemitter_t){2, 2, 0};
	xgeParticleEmitterInit(&e[1]);
	e[1].fRate = 85;
	e[1].iMaxParticles = 1024;
	e[1].iShape = XGE_PARTICLE_SHAPE_RECT;
	e[1].tOffset = (xge_vec2_t){240, 0};
	e[1].tShapeSize = (xge_vec2_t){330, 60};
	e[1].fDirection = DEMO_PI / 2;
	e[1].fSpread = .3f;
	e[1].tLife = (xge_particle_range_t){5, 7};
	e[1].tSpeed = (xge_particle_range_t){50, 85};
	e[1].tSize = (xge_particle_range_t){3, 8};
	e[1].tAngularVelocity = (xge_particle_range_t){-2, 2};
	e[1].fNoiseStrength = 22;
	e[1].fNoiseScroll = .25f;
	e[1].fNoiseFrequency = .02f;
	e[1].iColorMin = 0xa8c4f299;
	e[1].iColorMax = 0xf4fdffff;
	demo_curve(&e[1].tAlphaOverLife, 1, .2f);
	strcpy(e[1].sTexture, "soft");
	xgeParticleEmitterInit(&e[2]);
	e[2].bAutomatic = 0;
	e[2].tLife = (xge_particle_range_t){.12f, .24f};
	e[2].tSpeed = (xge_particle_range_t){15, 50};
	e[2].fSpread = DEMO_PI;
	e[2].tGravity = (xge_vec2_t){0, 160};
	e[2].tSize = (xge_particle_range_t){1, 2};
	e[2].iColorMin = e[2].iColorMax = 0x99caff99;
	demo_curve(&e[2].tAlphaOverLife, 1, 0);
	strcpy(e[2].sTexture, "soft");
}
