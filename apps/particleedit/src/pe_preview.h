#ifndef PE_PREVIEW_H
#define PE_PREVIEW_H
#include "pe_document.h"
#define PE_STAGE_W 1000
#define PE_STAGE_H 680
#define PE_PREVIEW_CAPACITY 100000
#define PE_HZ 120
#define PE_PREVIEW_SECONDS 60
typedef struct pe_preview
{
	xge_particle_world world;
	xge_particle_definition definition;
	xge_particle_renderer renderer;
	xge_particle_effect effect;
	xge_texture_t soft, sheet, textures[XGE_PARTICLE_MAX_EMITTERS];
	char texture_paths[XGE_PARTICLE_MAX_EMITTERS][PE_PATH];
	xge_shader_t shader;
	xge_material_t ring;
	xge_render_target_t target;
	uint64_t seed;
	int tick, target_tick, playing, moving, ground, bounds, grid, repeat, initialized;
	int muted[XGE_PARTICLE_MAX_EMITTERS], solo;
	float speed, range_seconds, carry, origin_x, origin_y;
	int unresolved, error;
	char message[512];
} pe_preview;
int pe_preview_init(pe_preview *p);
void pe_preview_free(pe_preview *p);
int pe_preview_rebuild(pe_preview *p, const pe_document *doc, int retain_time);
int pe_preview_seek(pe_preview *p, int tick);
int pe_preview_update(pe_preview *p, double delta);
int pe_preview_draw(pe_preview *p, const pe_document *doc);
void pe_preview_reload_textures(pe_preview *p);
#endif
