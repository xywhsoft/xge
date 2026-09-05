#include "pe_preview.h"
#include "../../../examples/xge_particles/shared_assets.h"
#include <stdlib.h>
#include <string.h>

static int collision(xge_vec2_t from, xge_vec2_t to, float radius, xge_particle_hit_t *hit, void *user)
{
	pe_preview *p = user;
	return p->ground && xgeParticleCollidePlane(from, to, radius, (xge_vec2_t){0, -1}, -570, hit);
}
static xge_particle_transform_t transform(pe_preview *p, int tick)
{
	float time = tick / (float)PE_HZ;
	xge_particle_transform_t t = {{p->origin_x, p->origin_y}, {1, 1}, 0};
	if (p->moving)
	{
		t.tPosition.fX += 180 * sinf(time * 1.7f);
		t.tPosition.fY += 50 * sinf(time * 3.4f);
	}
	return t;
}
static void release_simulation(pe_preview *p)
{
	xgeParticleRendererFree(p->renderer);
	p->renderer = NULL;
	xgeParticleWorldFree(p->world);
	p->world = NULL;
	xgeParticleDefinitionFree(p->definition);
	p->definition = NULL;
	p->effect = 0;
}
int pe_preview_init(pe_preview *p)
{
	int result;
	memset(p, 0, sizeof(*p));
	p->seed = 42;
	p->speed = 1;
	p->range_seconds = 10;
	p->origin_x = 500;
	p->origin_y = 400;
	p->playing = p->grid = p->repeat = 1;
	p->solo = -1;
	result = particle_example_textures(&p->soft, &p->sheet);
	if (result == XGE_OK)
		result = particle_example_ring(&p->shader, &p->ring);
	if (result == XGE_OK)
		result = xgeRenderTargetCreate(&p->target, PE_STAGE_W, PE_STAGE_H);
	p->initialized = result == XGE_OK;
	return result;
}
void pe_preview_free(pe_preview *p)
{
	release_simulation(p);
	for (int i = 0; i < XGE_PARTICLE_MAX_EMITTERS; ++i)
		xgeTextureFree(&p->textures[i]);
	xgeTextureFree(&p->soft);
	xgeTextureFree(&p->sheet);
	xgeShaderFree(&p->shader);
	xgeRenderTargetFree(&p->target);
	p->initialized = 0;
}
void pe_preview_reload_textures(pe_preview *p)
{
	/* Binding references are released before destroying their backing C structs. */
	release_simulation(p);
	for (int i = 0; i < XGE_PARTICLE_MAX_EMITTERS; ++i)
	{
		xgeTextureFree(&p->textures[i]);
		p->texture_paths[i][0] = 0;
	}
}
int pe_preview_rebuild(pe_preview *p, const pe_document *doc, int retain_time)
{
	pe_data data = doc->data;
	xge_particle_world_desc_t world;
	xge_particle_play_t play;
	uint64_t capacity = 0;
	int old_tick = retain_time ? p->target_tick : 0, result, i, j;
	release_simulation(p);
	p->unresolved = 0;
	p->error = 0;
	p->message[0] = 0;
	for (i = 0; i < data.count; ++i)
	{
		capacity += data.emitters[i].iMaxParticles;
		if (p->muted[i] || (p->solo >= 0 && p->solo != i))
			data.emitters[i].bAutomatic = 0;
		for (j = 0; j < 3; ++j)
		{
			int target = data.emitters[i].arrSubEmitters[j].iEmitter;
			if (target >= 0 && (p->muted[target] || (p->solo >= 0 && p->solo != target)))
				data.emitters[i].arrSubEmitters[j].iEmitter = -1;
		}
	}
	if (capacity > PE_PREVIEW_CAPACITY)
	{
		snprintf(p->message, sizeof(p->message), "预览暂停：配置容量 %llu > 预览预算 %d，资产仍可完整保存",
		         (unsigned long long)capacity, PE_PREVIEW_CAPACITY);
		p->error = XGE_ERROR_BUFFER_TOO_SMALL;
		return p->error;
	}
	result = xgeParticleDefinitionCreate(&p->definition, data.emitters, data.count);
	xgeParticleWorldDescInit(&world);
	world.iMaxEffects = 1;
	world.iMaxParticles = (uint32_t)capacity;
	world.iMaxEvents = 512;
	world.iMaxSpawnPerStep = 8192;
	world.fFixedStep = 1.0f / PE_HZ;
	world.pCollision = collision;
	world.pCollisionUser = p;
	if (result == XGE_OK)
		result = xgeParticleWorldCreate(&p->world, &world);
	if (result == XGE_OK)
		result = xgeParticleRendererCreate(&p->renderer, (uint32_t)capacity, 2048);
	for (i = 0; result == XGE_OK && i < data.count; ++i)
	{
		xge_texture texture = NULL;
		char path[PE_PATH] = {0};
		const char *name = data.emitters[i].sTexture;
		if (!strcmp(name, "soft"))
			texture = &p->soft;
		else if (!strcmp(name, "sheet"))
			texture = &p->sheet;
		else if (*name)
		{
			if (pe_path_resolve(doc->path, name, path, sizeof(path)))
			{
				if (strcmp(path, p->texture_paths[i]))
				{
					xgeTextureFree(&p->textures[i]);
					p->texture_paths[i][0] = 0;
					if (xgeTextureLoad(&p->textures[i], path) == XGE_OK)
						strcpy(p->texture_paths[i], path);
				}
				if (p->texture_paths[i][0])
					texture = &p->textures[i];
			}
			if (!texture)
				++p->unresolved;
		}
		name = data.emitters[i].sMaterial;
		if (*name && strcmp(name, "ring"))
			++p->unresolved;
		result = xgeParticleRendererBind(p->renderer, p->definition, i, texture,
		                                 !strcmp(name, "ring") ? &p->ring : NULL);
	}
	xgeParticlePlayInit(&play);
	play.iSeed = p->seed;
	play.tTransform = transform(p, 0);
	if (result == XGE_OK)
		result = xgeParticlePlay(p->world, p->definition, &play, &p->effect);
	p->tick = 0;
	p->target_tick = old_tick;
	p->carry = 0;
	if (result != XGE_OK)
	{
		release_simulation(p);
		p->error = result;
		snprintf(p->message, sizeof(p->message), "预览创建失败：%d", result);
	}
	else if (p->unresolved)
		snprintf(p->message, sizeof(p->message), "%d 个资源未解析：保留原绑定名，以白块 / 默认材质预览",
		         p->unresolved);
	return result;
}
int pe_preview_seek(pe_preview *p, int tick)
{
	xge_particle_transform_t t;
	int result;
	if (!p->world || tick < 0 || tick > PE_PREVIEW_SECONDS * PE_HZ)
		return XGE_ERROR_INVALID_ARGUMENT;
	p->target_tick = tick;
	p->carry = 0;
	if (tick >= p->tick)
		return XGE_OK;
	t = transform(p, 0);
	result = xgeParticleSetTransform(p->world, p->effect, &t);
	if (result == XGE_OK)
		result = xgeParticleRestart(p->world, p->effect, p->seed);
	p->tick = 0;
	return result;
}
int pe_preview_update(pe_preview *p, double delta)
{
	double start = xgeTimer();
	int steps = 0, result = XGE_OK;
	xge_particle_event_t event;
	if (!p->world)
		return XGE_OK;
	if (p->playing && p->tick == p->target_tick)
	{
		p->carry += (float)fmin(delta, .1) * p->speed * PE_HZ;
		steps = (int)p->carry;
		p->carry -= steps;
		p->target_tick += steps;
		if (p->target_tick > (int)(p->range_seconds * PE_HZ))
		{
			if (p->repeat)
				pe_preview_seek(p, 0);
			else
			{
				p->target_tick = (int)(p->range_seconds * PE_HZ);
				p->playing = 0;
			}
		}
	}
	steps = 0;
	/* Seeking is incremental. A long replay can never block the UI for 60 seconds. */
	while (p->tick < p->target_tick && steps++ < 120 && xgeTimer() - start < .006)
	{
		xge_particle_transform_t t = transform(p, p->tick + 1);
		result = xgeParticleSetTransform(p->world, p->effect, &t);
		if (result == XGE_OK)
			result = xgeParticleUpdate(p->world, (double)(1.0f / PE_HZ));
		if (result != XGE_OK)
			break;
		++p->tick;
		while (xgeParticleEventPoll(p->world, &event) > 0)
		{
		}
	}
	return result;
}
int pe_preview_draw(pe_preview *p, const pe_document *doc)
{
	xge_pass_t pass;
	xge_camera_t saved = xgeCameraGet(), camera = saved;
	int result;
	xge_particle_transform_t t = transform(p, p->tick);
	const xge_particle_emitter_t *e = &doc->data.emitters[doc->selected];
	float x = t.tPosition.fX + e->tOffset.fX, y = t.tPosition.fY + e->tOffset.fY;
	xgePassInit(&pass, &p->target, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(16, 22, 33, 255));
	result = xgePassBegin(&pass);
	if (result != XGE_OK)
		return result;
	camera.tPosition = (xge_vec2_t){0, 0};
	camera.tScale = (xge_vec2_t){1, 1};
	camera.fRotation = 0;
	camera.tViewport = (xge_rect_t){0, 0, PE_STAGE_W, PE_STAGE_H};
	camera.iCoordinateMode = XGE_COORD_Y_DOWN;
	xgeCameraSet(&camera);
	if (p->grid)
	{
		for (int i = 0; i <= PE_STAGE_W; i += 50)
			xgeShapeLinePx(i, 0, i, PE_STAGE_H, 1, XGE_COLOR_RGBA(30, 40, 55, 255));
		for (int i = 0; i <= PE_STAGE_H; i += 50)
			xgeShapeLinePx(0, i, PE_STAGE_W, i, 1, XGE_COLOR_RGBA(30, 40, 55, 255));
	}
	if (p->ground)
		xgeShapeLinePx(0, 570, PE_STAGE_W, 570, 2, XGE_COLOR_RGBA(185, 139, 65, 255));
	if (p->world)
		result = xgeParticleRender(p->renderer, p->world, NULL);
	if (p->bounds)
	{
		uint32_t color = XGE_COLOR_RGBA(70, 200, 193, 230);
		xge_rect_t bounds;
		if (p->world && xgeParticleBounds(p->world, p->effect, &bounds) == XGE_OK)
			xgeShapeRectStroke(bounds, 1, color);
		if (e->iShape == XGE_PARTICLE_SHAPE_CIRCLE || e->iShape == XGE_PARTICLE_SHAPE_RING)
		{
			xgeShapeCircleStrokePx(x, y, e->fRadius, 1, color);
			if (e->iShape == XGE_PARTICLE_SHAPE_RING)
				xgeShapeCircleStrokePx(x, y, e->fInnerRadius, 1, color);
		}
		else if (e->iShape == XGE_PARTICLE_SHAPE_RECT)
			xgeShapeRectStroke((xge_rect_t){x - e->tShapeSize.fX * .5f, y - e->tShapeSize.fY * .5f,
			                                e->tShapeSize.fX, e->tShapeSize.fY},
			                   1, color);
		else if (e->iShape == XGE_PARTICLE_SHAPE_LINE)
			xgeShapeLinePx(x - e->tShapeSize.fX * .5f, y - e->tShapeSize.fY * .5f, x + e->tShapeSize.fX * .5f,
			               y + e->tShapeSize.fY * .5f, 1, color);
		xgeShapeLinePx(x - 8, y, x + 8, y, 1, color);
		xgeShapeLinePx(x, y - 8, x, y + 8, 1, color);
	}
	xgeCameraSet(&saved);
	{
		int end = xgePassEnd(&pass);
		if (result == XGE_OK)
			result = end;
	}
	return result;
}
