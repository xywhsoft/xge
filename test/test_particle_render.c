#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct render_test_t
{
	int failed, checks;
} render_test_t;
#define VERIFY(condition)                                                                                    \
	do                                                                                                       \
	{                                                                                                        \
		test->checks++;                                                                                      \
		if (!(condition))                                                                                    \
		{                                                                                                    \
			fprintf(stderr, "GPU FAIL line %d: %s\n", __LINE__, #condition);                                 \
			test->failed = 1;                                                                                \
			goto cleanup;                                                                                    \
		}                                                                                                    \
	} while (0)
#define SUCCESS(call) VERIFY((call) == XGE_OK)
static int close_channel(unsigned char actual, int expected)
{
	return abs((int)actual - expected) <= 2;
}
static int render_frame(void *user)
{
	render_test_t *test = (render_test_t *)user;
	xge_particle_emitter_t emitter[2];
	xge_particle_definition definition = NULL;
	xge_particle_world world = NULL;
	xge_particle_renderer renderer = NULL, small = NULL;
	xge_particle_effect effect = 0;
	xge_particle_play_t play;
	xge_render_target_t target = {0};
	xge_texture_t texture = {0};
	xge_shader_t shader = {0};
	xge_material_t material;
	xge_pass_t pass;
	xge_frame_stats_t before, after;
	xge_camera_t originalCamera = xgeCameraGet();
	unsigned char white[4] = {255, 255, 255, 255}, pixels[64 * 64 * 4], *center = &pixels[(32 * 64 + 32) * 4];
	int begun = 0;
	const char *vertex = "#version 330 core\nlayout(location=0) in vec4 aPos;uniform vec2 uResolution;void "
	                     "main(){vec2 p=aPos.xy/uResolution*2.-1.;gl_Position=vec4(p.x,-p.y,0.,1.);}";
	const char *fragment = "#version 330 core\nout vec4 FragColor;void main(){FragColor=vec4(0.,0.5,0.,1.);}";
	xgeMaterialInit(&material);
	SUCCESS(xgeBegin());
	begun = 1;
	SUCCESS(xgeRenderTargetCreate(&target, 64, 64));
	SUCCESS(xgeParticleWorldCreate(&world, NULL));
	SUCCESS(xgeParticleRendererCreate(&renderer, 1024, 128));
	SUCCESS(xgeParticleRendererCreate(&small, 1, 1));
	xgeParticleEmitterInit(&emitter[0]);
	emitter[0].fRate = 0;
	emitter[0].bAutomatic = 0;
	emitter[0].iMaxParticles = 1024;
	emitter[0].tSpeed = (xge_particle_range_t){0, 0};
	emitter[0].tSize = (xge_particle_range_t){20, 20};
	emitter[0].iColorMin = emitter[0].iColorMax = 0xff000080;
	emitter[1] = emitter[0];
	emitter[1].iColorMin = emitter[1].iColorMax = 0x00ff0080;
	emitter[1].iOrder = 1;
	SUCCESS(xgeParticleDefinitionCreate(&definition, emitter, 2));
	xgeParticlePlayInit(&play);
	play.tTransform.tPosition = (xge_vec2_t){32, 32};
	SUCCESS(xgeParticlePlay(world, definition, &play, &effect));
	SUCCESS(xgeParticleEmit(world, effect, 0, 1, NULL));
	xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
	SUCCESS(xgePassBegin(&pass));
	xgeBlendSet(XGE_BLEND_MULTIPLY);
	SUCCESS(xgeParticleRender(renderer, world, NULL));
	VERIFY(xgeBlendGet() == XGE_BLEND_MULTIPLY);
	SUCCESS(xgePassEnd(&pass));
	SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
	VERIFY(close_channel(center[0], 128) && center[1] == 0 && center[2] == 0 && center[3] == 255);
	/* Explicit texture binding + painter order, deliberately emit green before red. */
	SUCCESS(xgeParticleRestart(world, effect, 1));
	SUCCESS(xgeParticleEmit(world, effect, 1, 1, NULL));
	SUCCESS(xgeParticleEmit(world, effect, 0, 1, NULL));
	SUCCESS(xgeTextureCreateRGBA(&texture, 1, 1, white));
	SUCCESS(xgeParticleRendererBind(renderer, definition, 1, &texture, NULL));
	VERIFY(texture.iRefCount == 2);
	xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
	SUCCESS(xgePassBegin(&pass));
	SUCCESS(xgeParticleRender(renderer, world, NULL));
	SUCCESS(xgePassEnd(&pass));
	SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
	VERIFY(close_channel(center[0], 64) && close_channel(center[1], 128) && center[2] == 0);
	SUCCESS(xgeParticleRendererUnbind(renderer, definition));
	VERIFY(texture.iRefCount == 1);
	/* Capacity errors must be reported before drawing any particle. */
	xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
	SUCCESS(xgePassBegin(&pass));
	VERIFY(xgeParticleRender(small, world, NULL) == XGE_ERROR_BUFFER_TOO_SMALL);
	SUCCESS(xgePassEnd(&pass));
	SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
	VERIFY(center[0] == 0 && center[1] == 0);
	/* 500 particles use four batches, also with a custom material, not 500 draw calls. */
	SUCCESS(xgeParticleRestart(world, effect, 1));
	SUCCESS(xgeParticleEmit(world, effect, 0, 500, NULL));
	SUCCESS(xgeShaderCreate(&shader, vertex, fragment));
	xgeMaterialSetShader(&material, &shader);
	SUCCESS(xgeParticleRendererBind(renderer, definition, 0, &texture, &material));
	VERIFY(shader.iRefCount == 2);
	xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
	SUCCESS(xgePassBegin(&pass));
	before = xgeFrameStatsGet();
	SUCCESS(xgeParticleRender(renderer, world, NULL));
	after = xgeFrameStatsGet();
	VERIFY(after.iDrawCallCount - before.iDrawCallCount == 4);
	VERIFY(after.iBatchCount - before.iBatchCount == 4);
	SUCCESS(xgePassEnd(&pass));
	SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
	VERIFY(center[0] == 0 && close_channel(center[1], 128) && center[2] == 0);
	/* Draw-only culling leaves simulation untouched. */
	{
		xge_rect_t view = {1000, 1000, 20, 20};
		before = xgeFrameStatsGet();
		SUCCESS(xgeParticleRender(renderer, world, &view));
		after = xgeFrameStatsGet();
		VERIFY(before.iDrawCallCount == after.iDrawCallCount &&
		       xgeParticleStats(world).iLiveParticles == 500);
	}
	SUCCESS(xgeParticleRendererUnbind(renderer, definition));
	VERIFY(shader.iRefCount == 1 && texture.iRefCount == 1);
	/* The shared material path still accepts an ordinary quad after a larger batch. */
	{
		xge_draw_t draw = {0};
		draw.pTexture = &texture;
		draw.tDst = (xge_rect_t){22, 22, 20, 20};
		draw.iColor = 0xffffffffu;
		draw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
		SUCCESS(xgePassBegin(&pass));
		xgeBlendSet(XGE_BLEND_ALPHA);
		xgeMaterialDraw(&material, &draw);
		SUCCESS(xgePassEnd(&pass));
		SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
		VERIFY(center[0] == 0 && close_channel(center[1], 128));
	}
	/* A two-cell nearest-filtered atlas switches UVs over life. */
	SUCCESS(xgeParticleRelease(world, effect));
	xgeParticleDefinitionFree(definition);
	definition = NULL;
	xgeTextureFree(&texture);
	{
		const unsigned char atlas[8] = {255, 0, 0, 255, 0, 255, 0, 255};
		xge_camera_t camera = {0};
		xge_sampler_t sampler = xgeSamplerDefault();
		int frame;
		SUCCESS(xgeTextureCreateRGBA(&texture, 2, 1, atlas));
		sampler.iMinFilter = sampler.iMagFilter = XGE_FILTER_NEAREST;
		SUCCESS(xgeTextureSetSampler(&texture, &sampler));
		emitter[0].iColorMin = emitter[0].iColorMax = 0xffffffffu;
		emitter[0].iColumns = 2;
		emitter[0].bScreenSpace = 1;
		SUCCESS(xgeParticleDefinitionCreate(&definition, emitter, 1));
		SUCCESS(xgeParticlePlay(world, definition, &play, &effect));
		SUCCESS(xgeParticleEmit(world, effect, 0, 1, NULL));
		SUCCESS(xgeParticleRendererBind(renderer, definition, 0, &texture, NULL));
		/* A nonidentity camera must not move a screen-space emitter. */
		camera.tPosition = (xge_vec2_t){1000, 1000};
		camera.tScale = (xge_vec2_t){1, 1};
		xgeCameraSet(&camera);
		for (frame = 0; frame < 2; frame++)
		{
			if (frame)
				SUCCESS(xgeParticleUpdate(world, .6));
			xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, 0x000000ff);
			SUCCESS(xgePassBegin(&pass));
			SUCCESS(xgeParticleRender(renderer, world, NULL));
			SUCCESS(xgePassEnd(&pass));
			SUCCESS(xgeRenderTargetReadPixels(&target, pixels, 64 * 4));
			VERIFY(close_channel(center[frame], 255) && center[1 - frame] <= 2);
		}
		xgeCameraSet(&originalCamera);
	}
cleanup:
	xgeCameraSet(&originalCamera);
	xgeParticleRendererFree(small);
	xgeParticleRendererFree(renderer);
	xgeParticleWorldFree(world);
	xgeParticleDefinitionFree(definition);
	xgeMaterialFree(&material);
	xgeShaderFree(&shader);
	xgeTextureFree(&texture);
	xgeRenderTargetFree(&target);
	if (begun)
		(void)xgeEnd();
	xgeQuit();
	return test->failed ? XGE_ERROR_INVALID_STATE : XGE_OK;
}
int main(void)
{
	xge_desc_t app = {0};
	render_test_t test = {0};
	int result;
	app.iWidth = 128;
	app.iHeight = 128;
	app.sTitle = "XGE particle GPU regression";
	if (xgeInit(&app) != XGE_OK)
		return 1;
	result = xgeRun(render_frame, &test);
	xgeUnit();
	if (result != XGE_OK || test.failed)
		return 1;
	printf("PASS particle GPU: %d checks (pixels, alpha, order, batches, ownership)\n", test.checks);
	return 0;
}
