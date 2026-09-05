#include "demo.h"

void demo_curve(xge_particle_curve_t *c, float from, float to)
{
	memset(c, 0, sizeof(*c));
	c->iCount = 2;
	c->arrKeys[0] = (xge_particle_key_t){0, from, 0, 0};
	c->arrKeys[1] = (xge_particle_key_t){1, to, 0, 0};
}
void demo_gradient(xge_particle_gradient_t *g, uint32_t from, uint32_t middle, uint32_t to)
{
	memset(g, 0, sizeof(*g));
	g->iCount = 3;
	g->arrKeys[0] = (xge_particle_color_key_t){0, from};
	g->arrKeys[1] = (xge_particle_color_key_t){.35f, middle};
	g->arrKeys[2] = (xge_particle_color_key_t){1, to};
}
void demo_burst(xge_particle_emitter_t *e, uint32_t count)
{
	e->fRate = 0;
	e->bLoop = 0;
	e->fDuration = .25f;
	e->iBurstCount = 1;
	e->arrBursts[0] = (xge_particle_burst_t){0, count};
}
static int demo_error(demo_t *d, int result, const char *operation)
{
	if (result != XGE_OK)
	{
		fprintf(stderr, "%s failed: %d\n", operation, result);
		d->error = result;
		xgeQuit();
	}
	return result;
}
static int demo_collision(xge_vec2_t from, xge_vec2_t to, float radius, xge_particle_hit_t *hit, void *user)
{
	demo_t *d = (demo_t *)user;
	if (d->kind == 0 && xgeParticleCollideRect(from, to, radius, (xge_rect_t){542, 350, 28, 210}, hit))
		return 1;
	return xgeParticleCollidePlane(from, to, radius, (xge_vec2_t){0, -1}, -570, hit);
}
static int demo_trigger(demo_t *d, xge_vec2_t position, int primary)
{
	xge_particle_play_t play;
	xge_particle_effect effect;
	int result, i, count = 0;
	for (i = 0; i < d->activeCount; i++)
		if (xgeParticleState(d->world, d->active[i]) >= 0)
			d->active[count++] = d->active[i];
	d->activeCount = count;
	if (count >= 32)
		return XGE_ERROR_BUFFER_TOO_SMALL;
	xgeParticlePlayInit(&play);
	play.tTransform.tPosition = position;
	play.iSeed = d->seed;
	play.bAutoRelease = !primary;
	play.tParameters.fRateScale = d->density;
	result = xgeParticlePlay(d->world, d->definition, &play, &effect);
	if (result == XGE_OK)
	{
		d->active[d->activeCount++] = effect;
		if (primary)
			d->primary = effect;
		(void)xgeParticleSetVisible(d->world, effect, d->visible);
	}
	return result;
}
static int demo_textures(demo_t *d)
{
	unsigned char pixels[64 * 64 * 4 * 4];
	int x, y, cell, index, result;
	for (cell = 0; cell < 4; cell++)
		for (y = 0; y < 64; y++)
			for (x = 0; x < 64; x++)
			{
				float u = (x - 31.5f) / 31.5f, v = (y - 31.5f) / 31.5f;
				float warp = 1 + .1f * sinf(9 * u + cell * 2) * sinf(7 * v - cell);
				float radius = sqrtf(u * u + v * v) * warp, alpha = powf(fmaxf(0, 1 - radius), 2);
				unsigned char a = (unsigned char)(alpha * 255);
				index = (y * 256 + cell * 64 + x) * 4;
				pixels[index] = pixels[index + 1] = pixels[index + 2] = pixels[index + 3] = a;
			}
	result = xgeTextureCreateRGBA(&d->sheet, 256, 64, pixels);
	if (result != XGE_OK)
		return result;
	for (y = 0; y < 64; y++)
		for (x = 0; x < 64; x++)
		{
			float u = (x - 31.5f) / 31.5f, v = (y - 31.5f) / 31.5f,
			      alpha = powf(fmaxf(0, 1 - u * u - v * v), 2);
			unsigned char a = (unsigned char)(alpha * 255);
			index = (y * 64 + x) * 4;
			pixels[index] = pixels[index + 1] = pixels[index + 2] = pixels[index + 3] = a;
		}
	return xgeTextureCreateRGBA(&d->soft, 64, 64, pixels);
}
static int demo_gpu_init(demo_t *d)
{
	int i, result = demo_textures(d);
	const char *vsBody =
	    "layout(location=0) in vec4 aPos; layout(location=1) in vec2 aUV; layout(location=2) in vec4 aColor; "
	    "uniform vec2 uResolution; out vec2 vUV; out vec4 vColor; void main(){vec2 "
	    "p=aPos.xy/uResolution*2.-1.; gl_Position=vec4(p.x,-p.y,0.,1.);vUV=aUV;vColor=aColor;}";
	const char *fsBody = "in vec2 vUV; in vec4 vColor; uniform vec4 uColor; uniform float uTime; out vec4 "
	                     "FragColor; void main(){vec2 p=vUV*2.-1.;float r=length(p),a=atan(p.y,p.x);float "
	                     "ring=exp(-pow((r-.75)*40.,2.));float inner=exp(-pow((r-.52)*70.,2.));float "
	                     "marks=pow(max(0.,cos(a*12.+uTime)),20.)*exp(-pow((r-.64)*24.,2.));float "
	                     "glow=clamp(ring+inner*.5+marks,0.,1.);FragColor=vColor*uColor*glow;}";
	if (result != XGE_OK)
		return result;
	if (d->kind == 5)
	{
		char header[128], vs[1024], fs[1536];
		result = xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, header, sizeof(header));
		if (result != XGE_OK)
			return result;
		snprintf(vs, sizeof(vs), "%s%s", header, vsBody);
		snprintf(fs, sizeof(fs), "%s%s", header, fsBody);
		result = xgeShaderCreate(&d->shader, vs, fs);
		if (result != XGE_OK)
			return result;
		xgeMaterialInit(&d->material);
		xgeMaterialSetShader(&d->material, &d->shader);
		xgeMaterialSetBlend(&d->material, XGE_BLEND_ADD);
	}
	for (i = 0; i < d->count; i++)
	{
		xge_particle_emitter_t emitter;
		xge_texture texture = NULL;
		result = xgeParticleDefinitionGet(d->definition, i, &emitter);
		if (result != XGE_OK)
			return result;
		if (!strcmp(emitter.sTexture, "soft"))
			texture = &d->soft;
		if (!strcmp(emitter.sTexture, "sheet"))
			texture = &d->sheet;
		result = xgeParticleRendererBind(d->renderer, d->definition, i, texture,
		                                 !strcmp(emitter.sMaterial, "ring") ? &d->material : NULL);
		if (result != XGE_OK)
			return result;
	}
	result = xgeRenderTargetCreate(&d->target, DEMO_W, DEMO_H);
	if (result != XGE_OK)
		return result;
#if defined(_WIN32)
	(void)xgeFontLoad(&d->font, "C:/Windows/Fonts/consola.ttf", 25);
	(void)xgeFontLoad(&d->smallFont, "C:/Windows/Fonts/consola.ttf", 15);
#else
	(void)xgeFontLoad(&d->font, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 25);
	(void)xgeFontLoad(&d->smallFont, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 15);
#endif
	d->ready = 1;
	return XGE_OK;
}
static void demo_background(demo_t *d)
{
	int x, y;
	xgeShapeRectFillPx((xge_rect_t){24, 128, 952, 504}, 0x111f2bff);
	for (x = 48; x < 970; x += 40)
		xgeShapeLine((float)x, 144, (float)x, 615, 1, 0x1c2d38ff);
	for (y = 152; y < 625; y += 40)
		xgeShapeLine(40, (float)y, 960, (float)y, 1, 0x1c2d38ff);
	if (d->kind == 0 || d->kind == 1 || d->kind == 2)
		xgeShapeRectFillPx((xge_rect_t){70, 570, 860, 40}, 0x293b46ff);
	if (d->kind == 0)
	{
		xgeShapeRectFillPx((xge_rect_t){542, 350, 28, 210}, 0x567886ff);
		xgeTextDraw(&d->smallFont, "SWEPT COLLISION TARGET", 595, 395, 0x9ec2d1ff);
	}
	if (d->kind == 1)
	{
		xgeShapeLine(430, 566, 570, 544, 12, 0x815634ff);
		xgeShapeLine(445, 542, 560, 570, 12, 0x523e30ff);
	}
	if (d->kind == 3)
	{
		xgeShapeLine(500, 140, 500, 615, 1, 0x658190ff);
		xgeTextDraw(&d->font, "RAIN", 75, 167, 0x93bdedff);
		xgeTextDraw(&d->font, "SNOW", 555, 167, 0xe0efffff);
	}
	if (d->kind == 4)
	{
		xgeShapeRoundRectFillPx((xge_rect_t){d->origin.fX - 28, d->origin.fY - 22, 56, 24}, 7, 0xd9a56aff);
		xgeShapeCircleFill(d->origin.fX - 18, d->origin.fY + 2, 8, 0x425160ff);
		xgeShapeCircleFill(d->origin.fX + 18, d->origin.fY + 2, 8, 0x425160ff);
	}
	if (d->kind == 6)
	{
		xgeShapeRoundRectFillPx((xge_rect_t){300, 250, 400, 230}, 18, 0x263a51ff);
		xgeTextDraw(&d->font, "LEVEL COMPLETE", 377, 302, 0xf5dfadff);
		xgeTextDraw(&d->smallFont, "SCREEN-SPACE CELEBRATION", 390, 350, 0xa9bccbff);
		xgeShapeRoundRectFillPx((xge_rect_t){410, 402, 180, 40}, 8, 0x326866ff);
		xgeTextDraw(&d->smallFont, "CONTINUE", 462, 415, 0xe0ffeaff);
	}
}
static int demo_capture(demo_t *d)
{
	unsigned char *pixels = (unsigned char *)malloc(DEMO_W * DEMO_H * 4);
	int result;
	if (!pixels)
		return XGE_ERROR_OUT_OF_MEMORY;
	result = xgeRenderTargetReadPixels(&d->target, pixels, DEMO_W * 4);
	if (result == XGE_OK)
		result = xgeImageSavePNG(d->capture, DEMO_W, DEMO_H, pixels, DEMO_W * 4);
	free(pixels);
	return result;
}
static void demo_controls(demo_t *d)
{
	int i;
	if (xgeKeyPressed(XGE_KEY_ESCAPE))
		xgeQuit();
	if (xgeKeyPressed(XGE_KEY_SPACE))
	{
		d->paused = !d->paused;
		for (i = 0; i < d->activeCount; i++)
			(void)xgeParticlePause(d->world, d->active[i], d->paused);
	}
	if (xgeKeyPressed('R'))
	{
		for (i = 0; i < d->activeCount; i++)
			(void)xgeParticleRelease(d->world, d->active[i]);
		d->activeCount = 0;
		d->time = 0;
		d->nextBurst = d->interval;
		d->paused = 0;
		d->spawning = 1;
		d->visible = 1;
		d->origin = d->initialOrigin;
		if (demo_error(d, demo_trigger(d, d->origin, 1), "replay") == XGE_OK && d->warmup > 0)
			(void)demo_error(d, xgeParticlePrewarm(d->world, d->primary, d->warmup), "prewarm");
	}
	if (xgeKeyPressed('D') || xgeKeyPressed('C'))
	{
		d->spawning = 0;
		for (i = 0; i < d->activeCount; i++)
			(void)xgeParticleStop(d->world, d->active[i], xgeKeyPressed('C'));
	}
	if (xgeKeyPressed(XGE_KEY_ENTER) && d->interval > 0 && !d->paused)
		(void)demo_trigger(d, d->origin, 0);
	if (xgeKeyPressed('B'))
		d->showBounds = !d->showBounds;
	if (xgeKeyPressed('V'))
	{
		d->visible = !d->visible;
		for (i = 0; i < d->activeCount; i++)
			(void)xgeParticleSetVisible(d->world, d->active[i], d->visible);
	}
	if (xgeKeyPressed('=') || xgeKeyPressed('-'))
	{
		xge_particle_parameters_t parameters = {1, 1, 1, 1, 0xffffffffu};
		d->density = fminf(4, fmaxf(.25f, d->density + (xgeKeyPressed('=') ? .25f : -.25f)));
		parameters.fRateScale = d->density;
		for (i = 0; i < d->activeCount; i++)
			(void)xgeParticleSetParameters(d->world, d->active[i], &parameters);
	}
}
static int demo_frame(void *user)
{
	demo_t *d = (demo_t *)user;
	xge_pass_t pass;
	xge_draw_t draw;
	xge_particle_event_t event;
	char statsText[256];
	xge_particle_stats_t stats;
	xge_rect_t view = {32, 132, 936, 490};
	int result;
	float dt = d->frames ? 1.0f / 60 : xgeGetDelta();
	if (!d->ready && demo_error(d, demo_gpu_init(d), "GPU setup") != XGE_OK)
		return d->error;
	/* Bounded captures/tests must not consume unrelated live keyboard input. */
	if (!d->frames)
		demo_controls(d);
	if (!d->paused)
	{
		d->time += dt;
		if (d->kind == 4 || d->kind == 5)
		{
			xge_particle_transform_t transform = {{0, 0}, {1, 1}, 0};
			if (d->kind == 4)
				d->origin = (xge_vec2_t){500 + 300 * sinf(d->time * .8f), 430 + 70 * sinf(d->time * 1.6f)};
			transform.tPosition = d->origin;
			if (d->kind == 5)
				transform.fRotation = d->time * .25f;
			(void)xgeParticleSetTransform(d->world, d->primary, &transform);
		}
		if (d->spawning && d->interval > 0 && d->time >= d->nextBurst)
		{
			d->nextBurst = d->time + d->interval;
			(void)demo_trigger(d, d->origin, 0);
		}
		if (demo_error(d, xgeParticleUpdate(d->world, dt), "simulation") != XGE_OK)
			return d->error;
	}
	while (xgeParticleEventPoll(d->world, &event) > 0)
	{ /* Host can spawn sounds/damage here; simulation never calls game code for events. */
	}
	if (d->kind == 5)
		(void)xgeShaderUniform1f(&d->shader, "uTime", d->time);
	result = xgeBegin();
	if (result != XGE_OK)
		return demo_error(d, result, "begin");
	xgePassInit(&pass, &d->target, XGE_PASS_CLEAR_COLOR, 0x0a121bff);
	result = xgePassBegin(&pass);
	if (result != XGE_OK)
		return demo_error(d, result, "pass");
	demo_background(d);
	xgeClipSet(view);
	result = xgeParticleRender(d->renderer, d->world, &view);
	xgeClipClear();
	if (result != XGE_OK)
		return demo_error(d, result, "particle rendering");
	if (d->showBounds)
	{
		xge_rect_t bounds;
		if (xgeParticleBounds(d->world, d->primary, &bounds) == XGE_OK)
			xgeShapeRectStroke(bounds, 1, 0x77f0aaff);
	}
	/* Explicit barrier keeps HUD text above the immediate particle pass. */
	(void)xgeFlush();
	xgeTextDraw(&d->smallFont, "XGE / PARTICLE STUDIES", 32, 24, 0x66d7c1ff);
	xgeTextDraw(&d->font, d->title, 32, 52, 0xf3f6f8ff);
	xgeTextDraw(&d->smallFont, d->features, 32, 93, 0x9bb6caff);
	stats = xgeParticleStats(d->world);
	snprintf(statsText, sizeof(statsText),
	         "LIVE %u   PEAK %u   EFFECTS %u   COLLISIONS %llu   UPDATE %.2f ms", stats.iLiveParticles,
	         stats.iPeakParticles, stats.iActiveEffects, (unsigned long long)stats.iCollisions,
	         stats.fLastUpdateSeconds * 1000);
	xgeTextDraw(&d->smallFont, statsText, 34, 639, 0x79bdb5ff);
	xgeTextDraw(&d->smallFont,
	            "SPACE pause   R replay   D drain   C clear   B bounds   V visibility   +/- rate", 34, 658,
	            0x668399ff);
	result = xgePassEnd(&pass);
	if (result != XGE_OK)
		return demo_error(d, result, "end pass");
	d->frame++;
	if (d->capture && d->frame == d->frames && demo_error(d, demo_capture(d), "capture") != XGE_OK)
		return d->error;
	xgeClear(0x0a121bff);
	memset(&draw, 0, sizeof(draw));
	draw.pTexture = xgeRenderTargetTexture(&d->target);
	draw.tDst = (xge_rect_t){0, 0, DEMO_W, DEMO_H};
	draw.iColor = 0xffffffffu;
	draw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&draw);
	result = xgeEnd();
	if (result != XGE_OK)
		return demo_error(d, result, "end");
	if (d->frames && d->frame >= d->frames)
	{
		printf("%s: live=%u spawned=%llu collisions=%llu\n", d->title, stats.iLiveParticles,
		       (unsigned long long)stats.iSpawned, (unsigned long long)stats.iCollisions);
		xgeQuit();
	}
	return XGE_OK;
}
int main(int argc, char **argv)
{
	demo_t *d = (demo_t *)calloc(1, sizeof(*d));
	xge_desc_t app = {0};
	xge_particle_world_desc_t world;
	const char *exportPath = NULL;
	int i, result = 1, initialized = 0;
	if (!d)
		return 1;
	d->seed = 7;
	d->visible = 1;
	d->density = 1;
	d->spawning = 1;
	demo_configure(d);
	d->initialOrigin = d->origin;
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--help"))
		{
			printf("%s\n--frames N --capture PATH --seed N --config PATH --export PATH\n", d->title);
			result = 0;
			goto done;
		}
		if (i + 1 >= argc)
		{
			fprintf(stderr, "Missing option value\n");
			goto done;
		}
		if (!strcmp(argv[i], "--frames"))
		{
			d->frames = atoi(argv[++i]);
			if (d->frames <= 0)
				goto done;
		}
		else if (!strcmp(argv[i], "--capture"))
			d->capture = argv[++i];
		else if (!strcmp(argv[i], "--seed"))
			d->seed = strtoull(argv[++i], NULL, 10);
		else if (!strcmp(argv[i], "--config"))
			d->config = argv[++i];
		else if (!strcmp(argv[i], "--export"))
			exportPath = argv[++i];
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			goto done;
		}
	}
	if (d->capture && !d->frames)
		d->frames = 180;
	if (d->config)
	{
		char error[256];
		int xson = strstr(d->config, ".xson") != NULL;
		if (xgeParticleDefinitionLoad(&d->definition, d->config, xson, error, sizeof(error)) != XGE_OK)
		{
			fprintf(stderr, "%s\n", error);
			goto done;
		}
		d->count = xgeParticleDefinitionCount(d->definition);
	}
	else if (xgeParticleDefinitionCreate(&d->definition, d->emitters, d->count) != XGE_OK)
	{
		fprintf(stderr, "Invalid example definition\n");
		goto done;
	}
	if (exportPath)
	{
		char *text = NULL;
		size_t size = 0;
		FILE *file;
		if (xgeParticleDefinitionStringify(d->definition, strstr(exportPath, ".xson") != NULL, &text,
		                                   &size) != XGE_OK)
			goto done;
		file = fopen(exportPath, "wb");
		if (file)
		{
			int wrote = fwrite(text, 1, size, file) == size;
			int closed = fclose(file) == 0;
			result = wrote && closed ? 0 : 1;
		}
		xrtFree(text);
		goto done;
	}
	app.iWidth = DEMO_W;
	app.iHeight = DEMO_H;
	app.sTitle = d->title;
	app.iFlags = XGE_INIT_VSYNC;
	if (xgeInit(&app) != XGE_OK)
		goto done;
	initialized = 1;
	xgeParticleWorldDescInit(&world);
	world.iMaxEffects = 32;
	world.iMaxParticles = 16384;
	world.pCollision = demo_collision;
	world.pCollisionUser = d;
	if (xgeParticleWorldCreate(&d->world, &world) != XGE_OK ||
	    xgeParticleRendererCreate(&d->renderer, 16384, 1024) != XGE_OK)
		goto done;
	if (xgeParticleWorldReserve(d->world, d->definition, d->interval > 0 ? 8 : 1) != XGE_OK ||
	    demo_trigger(d, d->origin, 1) != XGE_OK)
		goto done;
	d->nextBurst = d->interval;
	if (d->warmup > 0 && xgeParticlePrewarm(d->world, d->primary, d->warmup) != XGE_OK)
		goto done;
	printf("%s\n%s\n", d->title, d->features);
	result = xgeRun(demo_frame, d);
	result = result == XGE_OK && !d->error ? 0 : 1;
done:
	xgeParticleRendererFree(d->renderer);
	xgeParticleWorldFree(d->world);
	xgeParticleDefinitionFree(d->definition);
	xgeMaterialFree(&d->material);
	xgeShaderFree(&d->shader);
	xgeTextureFree(&d->soft);
	xgeTextureFree(&d->sheet);
	xgeFontFree(&d->font);
	xgeFontFree(&d->smallFont);
	xgeRenderTargetFree(&d->target);
	if (initialized)
		xgeUnit();
	free(d);
	return result;
}
