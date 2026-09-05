/* Rendering adapter: stable painter order, adjacent compatible runs, retained buffers. */
#include "xge_particle_internal.h"

typedef struct xp_binding_key
{
	uint64_t definition, emitter;
} xp_binding_key;
typedef struct xp_binding
{
	xge_particle_definition definition;
	xge_texture texture;
	xge_material_t material;
	int emitter;
} xp_binding;
typedef struct xp_render_item
{
	xge_draw_t draw;
	xp_binding *binding;
	int layer, order, blend;
	uint64_t serial;
} xp_render_item;
struct xge_particle_renderer_t
{
	xmap lookup;
	xarray bindings, items;
	xge_texture_t white;
	xge_sprite_batch_t batch;
	uint32_t capacity, batchCapacity;
	xge_particle_world world;
	xge_particle_definition cachedDefinition;
	xp_binding *cachedBindings[XGE_PARTICLE_MAX_EMITTERS];
	const xge_rect_t *view;
	int busy, overflow;
};
static xp_binding_key xp_binding_key_make(xge_particle_definition definition, int emitter)
{
	xp_binding_key key;
	key.definition = (uint64_t)(uintptr_t)definition;
	key.emitter = (uint64_t)emitter;
	return key;
}
static xp_binding *xp_binding_find(xge_particle_renderer r, xge_particle_definition d, int emitter)
{
	xp_binding_key key = xp_binding_key_make(d, emitter);
	size_t *index = (size_t *)xrtMapGet(&r->lookup, (xbytesview){(const unsigned char *)&key, sizeof(key)});
	return index ? &((xp_binding *)r->bindings.Data)[*index] : NULL;
}
static void xp_binding_free(xp_binding *binding)
{
	xge_shader shader = binding->material.pShader;
	xgeMaterialFree(&binding->material);
	xgeShaderFree(shader);
	xgeTextureFree(binding->texture);
	xgeParticleDefinitionFree(binding->definition);
	memset(binding, 0, sizeof(*binding));
}
int xgeParticleRendererCreate(xge_particle_renderer *out, uint32_t capacity, uint32_t batchCapacity)
{
	xge_particle_renderer r;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (!capacity || capacity > 1000000 || !batchCapacity || batchCapacity > 65536)
		return XGE_ERROR_INVALID_ARGUMENT;
	r = (xge_particle_renderer)xrtCalloc(1, sizeof(*r));
	if (!r)
		return XGE_ERROR_OUT_OF_MEMORY;
	r->capacity = capacity;
	r->batchCapacity = batchCapacity;
	xrtMapInit(&r->lookup, sizeof(size_t));
	xrtArrayInit(&r->bindings, sizeof(xp_binding));
	xrtArrayInit(&r->items, sizeof(xp_render_item));
	if (!xrtArrayReserve(&r->items, capacity))
	{
		xgeParticleRendererFree(r);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*out = r;
	return XGE_OK;
}
void xgeParticleRendererFree(xge_particle_renderer r)
{
	size_t i;
	if (!r || r->busy)
		return;
	for (i = 0; i < r->bindings.Count; i++)
		xp_binding_free(&((xp_binding *)r->bindings.Data)[i]);
	xgeSpriteBatchFree(&r->batch);
	xgeTextureFree(&r->white);
	xrtMapUnit(&r->lookup);
	xrtArrayUnit(&r->bindings);
	xrtArrayUnit(&r->items);
	xrtFree(r);
}
int xgeParticleRendererBind(xge_particle_renderer r, xge_particle_definition d, int emitter,
                            xge_texture texture, const xge_material_t *material)
{
	xp_binding binding, *old;
	size_t index;
	xp_binding_key key = xp_binding_key_make(d, emitter);
	if (!r || !d || emitter < 0 || emitter >= xgeParticleDefinitionCount(d))
		return XGE_ERROR_INVALID_ARGUMENT;
	if (r->busy)
		return XGE_ERROR_INVALID_STATE;
	if (!texture && material)
		texture = material->pTexture;
	if (texture && texture->iRefCount <= 0)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (material && (!material->pShader || material->pShader->iRefCount <= 0 ||
	                 (material->pTexture && material->pTexture != texture) ||
	                 (material->pTexture2 && material->pTexture2->iRefCount <= 0) ||
	                 (material->pTexture3 && material->pTexture3->iRefCount <= 0)))
		return XGE_ERROR_INVALID_ARGUMENT;
	memset(&binding, 0, sizeof(binding));
	xgeMaterialInit(&binding.material);
	binding.definition = d;
	binding.emitter = emitter;
	binding.texture = texture;
	xgeParticleDefinitionAddRef(d);
	if (texture && xgeTextureAddRef(texture) <= 0)
	{
		binding.texture = NULL;
		xp_binding_free(&binding);
		return XGE_ERROR_INVALID_STATE;
	}
	if (material)
	{
		if (xgeShaderAddRef(material->pShader) <= 0)
		{
			xp_binding_free(&binding);
			return XGE_ERROR_INVALID_STATE;
		}
		binding.material.pShader = material->pShader;
		binding.material.iColor = material->iColor;
		binding.material.tPipeline = material->tPipeline;
		xgeMaterialSetTexture2(&binding.material, material->pTexture2);
		xgeMaterialSetTexture3(&binding.material, material->pTexture3);
	}
	old = xp_binding_find(r, d, emitter);
	if (old)
	{
		xp_binding_free(old);
		*old = binding;
		return XGE_OK;
	}
	for (index = 0; index < r->bindings.Count; index++)
		if (!((xp_binding *)r->bindings.Data)[index].definition)
			break;
	if (index == r->bindings.Count && !xrtArrayAdd(&r->bindings, 1))
	{
		xp_binding_free(&binding);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(&((xp_binding *)r->bindings.Data)[index], 0, sizeof(binding));
	if (!xrtMapSet(&r->lookup, (xbytesview){(const unsigned char *)&key, sizeof(key)}, &index))
	{
		xp_binding_free(&binding);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	((xp_binding *)r->bindings.Data)[index] = binding;
	return XGE_OK;
}
int xgeParticleRendererUnbind(xge_particle_renderer r, xge_particle_definition d)
{
	size_t i;
	if (!r || !d)
		return XGE_ERROR_INVALID_ARGUMENT;
	if (r->busy)
		return XGE_ERROR_INVALID_STATE;
	for (i = 0; i < r->bindings.Count; i++)
	{
		xp_binding *b = &((xp_binding *)r->bindings.Data)[i];
		if (b->definition == d)
		{
			xp_binding_key key = xp_binding_key_make(d, b->emitter);
			xrtMapRemove(&r->lookup, (xbytesview){(const unsigned char *)&key, sizeof(key)});
			xp_binding_free(b);
		}
	}
	return XGE_OK;
}
static int xp_render_collect(const xge_particle_snapshot_t *s, void *user)
{
	xge_particle_renderer r = (xge_particle_renderer)user;
	xp_effect *effect = (xp_effect *)xrtSlotMapGet(&r->world->handles, s->iEffect);
	xge_particle_emitter_t *e = &((xge_particle_emitter_t *)s->pDefinition->emitters.Data)[s->iEmitter];
	xp_render_item item;
	xge_texture texture;
	float radius;
	int frame;
	if (!effect->visible || !XGE_COLOR_GET_A(s->iColor) || !isfinite(s->tSize.fX) || !isfinite(s->tSize.fY) ||
	    !isfinite(s->tPosition.fX) || !isfinite(s->tPosition.fY) || s->tSize.fX <= 0 || s->tSize.fY <= 0)
		return 1;
	radius = .5f * hypotf(s->tSize.fX, s->tSize.fY);
	if (r->view && (s->tPosition.fX + radius < r->view->fX || s->tPosition.fY + radius < r->view->fY ||
	                s->tPosition.fX - radius > r->view->fX + r->view->fW ||
	                s->tPosition.fY - radius > r->view->fY + r->view->fH))
		return 1;
	if (r->items.Count >= r->capacity)
	{
		r->overflow = 1;
		return 0;
	}
	if (r->cachedDefinition != s->pDefinition)
	{
		int i;
		r->cachedDefinition = s->pDefinition;
		for (i = 0; i < xgeParticleDefinitionCount(s->pDefinition); i++)
			r->cachedBindings[i] = xp_binding_find(r, s->pDefinition, i);
	}
	memset(&item, 0, sizeof(item));
	item.binding = r->cachedBindings[s->iEmitter];
	texture = item.binding && item.binding->texture ? item.binding->texture : &r->white;
	item.draw.pTexture = texture;
	item.draw.tSrc = e->tTextureRect;
	if (!item.draw.tSrc.fW)
		item.draw.tSrc.fW = (float)texture->iWidth;
	if (!item.draw.tSrc.fH)
		item.draw.tSrc.fH = (float)texture->iHeight;
	item.draw.tSrc.fW /= e->iColumns;
	item.draw.tSrc.fH /= e->iRows;
	frame = (int)fmodf(floorf(s->fFrame), (float)(e->iColumns * e->iRows));
	item.draw.tSrc.fX += (frame % e->iColumns) * item.draw.tSrc.fW;
	item.draw.tSrc.fY += (frame / e->iColumns) * item.draw.tSrc.fH;
	item.draw.tDst = (xge_rect_t){s->tPosition.fX, s->tPosition.fY, s->tSize.fX, s->tSize.fY};
	item.draw.tOrigin = xp_mul(s->tSize, .5f);
	item.draw.fRotation = s->fRotation;
	/* XGE blend modes consume premultiplied RGB, including vertex opacity. */
	{
		uint32_t a = XGE_COLOR_GET_A(s->iColor);
		item.draw.iColor = XGE_COLOR_RGBA((XGE_COLOR_GET_R(s->iColor) * a + 127) / 255,
		                                  (XGE_COLOR_GET_G(s->iColor) * a + 127) / 255,
		                                  (XGE_COLOR_GET_B(s->iColor) * a + 127) / 255, a);
	}
	item.draw.iFlags = e->bScreenSpace ? XGE_DRAW_SCREEN_SPACE : 0;
	item.layer = e->iLayer;
	item.order = e->iOrder;
	item.blend = e->iBlend;
	item.serial = s->iParticle;
	if (!xrtArrayPush(&r->items, &item))
	{
		r->overflow = 1;
		return 0;
	}
	return 1;
}
static int xp_render_compare(const void *left, const void *right)
{
	const xp_render_item *a = (const xp_render_item *)left, *b = (const xp_render_item *)right;
	if (a->layer != b->layer)
		return a->layer < b->layer ? -1 : 1;
	if (a->order != b->order)
		return a->order < b->order ? -1 : 1;
	return a->serial < b->serial ? -1 : a->serial > b->serial;
}
int xgeParticleRender(xge_particle_renderer r, xge_particle_world w, const xge_rect_t *view)
{
	static const unsigned char white[4] = {255, 255, 255, 255};
	size_t i;
	int result, oldBlend, blend = -1;
	xp_binding *binding = NULL;
	xge_texture texture = NULL;
	if (!r || !w ||
	    (view && (!isfinite(view->fX) || !isfinite(view->fY) || !isfinite(view->fW) || !isfinite(view->fH) ||
	              view->fW < 0 || view->fH < 0)))
		return XGE_ERROR_INVALID_ARGUMENT;
	if (r->busy || w->busy)
		return XGE_ERROR_INVALID_STATE;
	if (!g_xge.bSokolRunning)
		return XGE_ERROR_NOT_INITIALIZED;
	if (g_xge.bRenderThreadEnabled)
		return XGE_ERROR_UNSUPPORTED;
	if (!r->white.iRefCount)
	{
		result = xgeTextureCreateRGBA(&r->white, 1, 1, white);
		if (result != XGE_OK)
			return result;
	}
	if (!r->batch.pVertices)
	{
		result = xgeSpriteBatchInit(&r->batch, &r->white, (int)r->batchCapacity, 0);
		if (result != XGE_OK)
			return result;
	}
	r->busy = 1;
	r->world = w;
	r->view = view;
	r->overflow = 0;
	r->cachedDefinition = NULL;
	xrtArrayClear(&r->items);
	xgeSpriteBatchClear(&r->batch);
	result = xgeParticleVisit(w, xp_render_collect, r);
	if (r->overflow)
		result = XGE_ERROR_BUFFER_TOO_SMALL;
	if (result != XGE_OK)
	{
		r->busy = 0;
		r->world = NULL;
		r->view = NULL;
		return result;
	}
	xrtArraySort(&r->items, xp_render_compare);
	result = xgeFlush();
	oldBlend = xgeBlendGet();
	for (i = 0; result == XGE_OK && i < r->items.Count; i++)
	{
		xp_render_item *item = &((xp_render_item *)r->items.Data)[i];
		if (item->binding != binding || item->blend != blend || item->draw.pTexture != texture ||
		    r->batch.iCount >= r->batch.iCapacity)
		{
			result = xgeSpriteBatchFlushMaterial(
			    &r->batch, binding && binding->material.pShader ? &binding->material : NULL);
			if (result != XGE_OK)
				break;
			binding = item->binding;
			texture = item->draw.pTexture;
			blend = item->blend;
			result = xgeSpriteBatchSetTexture(&r->batch, texture);
			xgeBlendSet(blend);
			if (result != XGE_OK)
				break;
		}
		result = xgeSpriteBatchAdd(&r->batch, &item->draw);
	}
	if (result == XGE_OK)
		result = xgeSpriteBatchFlushMaterial(
		    &r->batch, binding && binding->material.pShader ? &binding->material : NULL);
	xgeSpriteBatchClear(&r->batch);
	/* Do not keep an extra borrowed application texture alive after unbinding. */
	(void)xgeSpriteBatchSetTexture(&r->batch, &r->white);
	xgeBlendSet(oldBlend);
	r->busy = 0;
	r->world = NULL;
	r->view = NULL;
	return result;
}
