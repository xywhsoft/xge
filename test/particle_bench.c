/* Informational CPU baseline, not a machine-dependent pass/fail threshold. */
#include "../xge.h"
#include <math.h>
#include <stdio.h>

static int run_case(uint32_t count, int noise)
{
	xge_particle_emitter_t emitter;
	xge_particle_definition definition = NULL;
	xge_particle_world world = NULL;
	xge_particle_world_desc_t config;
	xge_particle_effect effect;
	double start, elapsed;
	int frame, result = 1;

	xgeParticleEmitterInit(&emitter);
	emitter.bAutomatic = 0;
	emitter.iMaxParticles = count;
	emitter.tLife = (xge_particle_range_t){60, 60};
	emitter.fNoiseStrength = noise ? 30 : 0;
	emitter.tGravity = (xge_vec2_t){0, 10};
	xgeParticleWorldDescInit(&config);
	config.iMaxEffects = 1;
	config.iMaxParticles = count;
	config.iMaxSpawnPerStep = count;
	config.iMaxEvents = 0;
	config.fFixedStep = 1.0f / 120;
	if (xgeParticleDefinitionCreate(&definition, &emitter, 1) != XGE_OK ||
	    xgeParticleWorldCreate(&world, &config) != XGE_OK ||
	    xgeParticleWorldReserve(world, definition, 1) != XGE_OK ||
	    xgeParticlePlay(world, definition, NULL, &effect) != XGE_OK ||
	    xgeParticleEmit(world, effect, 0, count, NULL) != XGE_OK)
		goto done;

	start = xrtTimer();
	for (frame = 0; frame < 240; frame++)
		if (xgeParticleUpdate(world, config.fFixedStep) != XGE_OK)
			goto done;
	elapsed = xrtTimer() - start;
	if (xgeParticleStats(world).iLiveParticles != count || !isfinite(elapsed))
		goto done;
	printf("particles=%u noise=%d steps=240 average_ms=%.3f\n", count, noise, elapsed * 1000 / 240);
	result = 0;
done:
	xgeParticleWorldFree(world);
	xgeParticleDefinitionFree(definition);
	return result;
}

int main(void)
{
	const uint32_t counts[] = {1000, 10000, 50000};
	unsigned int i;
	for (i = 0; i < sizeof(counts) / sizeof(counts[0]); i++)
		if (run_case(counts[i], 0) || run_case(counts[i], 1))
			return 1;
	return 0;
}
