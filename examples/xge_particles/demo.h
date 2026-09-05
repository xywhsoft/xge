#ifndef XGE_PARTICLE_DEMO_H
#define XGE_PARTICLE_DEMO_H
#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEMO_PI 3.14159265358979323846f
#define DEMO_W 1000
#define DEMO_H 680
typedef struct demo_t
{
	const char *title;
	const char *features;
	int kind, count, frames, frame, paused, showBounds, visible, error, ready, spawning;
	float time, interval, nextBurst, warmup, density;
	uint64_t seed;
	xge_vec2_t origin, initialOrigin;
	xge_particle_emitter_t emitters[8];
	xge_particle_definition definition;
	xge_particle_world world;
	xge_particle_renderer renderer;
	xge_particle_effect primary;
	xge_particle_effect active[32];
	int activeCount;
	xge_texture_t soft, sheet;
	xge_shader_t shader;
	xge_material_t material;
	xge_font_t font, smallFont;
	xge_render_target_t target;
	const char *capture;
	const char *config;
} demo_t;
void demo_configure(demo_t *d);
void demo_curve(xge_particle_curve_t *c, float from, float to);
void demo_gradient(xge_particle_gradient_t *g, uint32_t from, uint32_t middle, uint32_t to);
void demo_burst(xge_particle_emitter_t *e, uint32_t count);
#endif
