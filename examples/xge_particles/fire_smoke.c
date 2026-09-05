#include "demo.h"
void demo_configure(demo_t *d)
{
	xge_particle_emitter_t *e = d->emitters;
	d->title = "02 / FIRE & SMOKE";
	d->features =
	    "Multi-emitter effect | line emission | lifetime gradients | flipbook | turbulence + prewarm";
	d->kind = 1;
	d->count = 3;
	d->warmup = 2;
	d->origin = (xge_vec2_t){500, 549};
	xgeParticleEmitterInit(&e[0]);
	e[0].fRate = 38;
	e[0].iShape = XGE_PARTICLE_SHAPE_LINE;
	e[0].tShapeSize = (xge_vec2_t){60, 0};
	e[0].tLife = (xge_particle_range_t){2.5f, 4};
	e[0].tSpeed = (xge_particle_range_t){40, 70};
	e[0].tSize = (xge_particle_range_t){32, 58};
	e[0].fNoiseStrength = 35;
	e[0].fNoiseScroll = .4f;
	e[0].tGravity = (xge_vec2_t){0, -10};
	e[0].fDrag = .2f;
	demo_curve(&e[0].tSizeOverLife, .6f, 3.6f);
	demo_gradient(&e[0].tColorOverLife, 0x6b728000, 0x65707f9a, 0x64758c00);
	strcpy(e[0].sTexture, "sheet");
	e[0].iColumns = 4;
	e[0].fFramesPerSecond = 5;
	e[0].tStartFrame = (xge_particle_range_t){0, 3};
	xgeParticleEmitterInit(&e[1]);
	e[1].fRate = 130;
	e[1].iShape = XGE_PARTICLE_SHAPE_LINE;
	e[1].tShapeSize = (xge_vec2_t){65, 0};
	e[1].tLife = (xge_particle_range_t){.5f, 1.15f};
	e[1].tSpeed = (xge_particle_range_t){80, 130};
	e[1].tSize = (xge_particle_range_t){38, 62};
	e[1].fSpread = .35f;
	e[1].fNoiseStrength = 65;
	e[1].fNoiseScroll = 1.2f;
	e[1].iLayer = 1;
	e[1].iBlend = XGE_BLEND_ADD;
	demo_curve(&e[1].tSizeOverLife, 1.1f, .1f);
	demo_gradient(&e[1].tColorOverLife, 0xfff0a599, 0xff7b28dd, 0xb9210000);
	strcpy(e[1].sTexture, "sheet");
	e[1].iColumns = 4;
	e[1].fFramesPerSecond = 9;
	xgeParticleEmitterInit(&e[2]);
	e[2].fRate = 16;
	e[2].tLife = (xge_particle_range_t){1, 2.5f};
	e[2].tSpeed = (xge_particle_range_t){70, 145};
	e[2].tSize = (xge_particle_range_t){2, 4};
	e[2].fNoiseStrength = 40;
	e[2].iLayer = 2;
	e[2].iBlend = XGE_BLEND_ADD;
	demo_gradient(&e[2].tColorOverLife, 0xffed9fff, 0xff8629dd, 0xff220000);
	strcpy(e[2].sTexture, "soft");
}
