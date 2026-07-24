#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROBE_WIDTH 256
#define PROBE_HEIGHT 192

enum probe_kind {
	PROBE_SHAPE = 0,
	PROBE_SHAPE_CLIP,
	PROBE_SHAPE_MASK,
	PROBE_SHAPE_BLEND,
	PROBE_SCENE,
	PROBE_SCENE_CLIP,
	PROBE_SCENE_MASK,
	PROBE_SCENE_OPACITY,
	PROBE_SCENE_EFFECT,
	PROBE_SVG,
	PROBE_SVG_CLIP_MASK,
	PROBE_SVG_FILTER_BLEND,
	PROBE_COUNT
};

typedef struct probe_app_t {
	const char* output_dir;
	int result;
} probe_app_t;

static xge_camera_t probe_camera(void)
{
	xge_camera_t camera = xgeCameraDefault((float)PROBE_WIDTH, (float)PROBE_HEIGHT);

	camera.tPosition = (xge_vec2_t){11.25f, 8.5f};
	camera.tScale = (xge_vec2_t){1.5f, 1.25f};
	camera.tViewport = (xge_rect_t){7.0f, 9.0f, (float)PROBE_WIDTH, (float)PROBE_HEIGHT};
	return camera;
}

static xge_shape_ex_matrix_t probe_camera_matrix(void)
{
	xge_camera_t camera = probe_camera();
	xge_shape_ex_matrix_t matrix;

	matrix.fA = camera.tScale.fX;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = camera.tScale.fY;
	matrix.fE = camera.tViewport.fX - camera.tPosition.fX * camera.tScale.fX;
	matrix.fF = camera.tViewport.fY - camera.tPosition.fY * camera.tScale.fY;
	return matrix;
}

static xge_rect_t project_rect(xge_rect_t rect)
{
	xge_shape_ex_matrix_t matrix = probe_camera_matrix();
	xge_rect_t projected;

	projected.fX = rect.fX * matrix.fA + matrix.fE;
	projected.fY = rect.fY * matrix.fD + matrix.fF;
	projected.fW = rect.fW * matrix.fA;
	projected.fH = rect.fH * matrix.fD;
	return projected;
}

static int create_probe_shape(enum probe_kind kind, xge_shape_ex* out)
{
	static const xge_shape_ex_color_stop_t stops[] = {
		{0.0f, XGE_COLOR_RGBA(63, 197, 224, 255)},
		{0.55f, XGE_COLOR_RGBA(79, 124, 230, 255)},
		{1.0f, XGE_COLOR_RGBA(237, 86, 137, 255)}
	};
	xge_shape_ex shape = NULL;
	xge_shape_ex decoration = NULL;
	int result;

	if ( out == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	result = xgeShapeExCreate(&shape);
	if ( result == XGE_OK ) result = xgeShapeExMoveTo(shape, 28.5f, 31.25f);
	if ( result == XGE_OK ) result = xgeShapeExCubicTo(shape, 45.0f, 12.0f, 91.0f, 17.0f, 112.25f, 39.5f);
	if ( result == XGE_OK ) result = xgeShapeExLineTo(shape, 96.5f, 94.25f);
	if ( result == XGE_OK ) result = xgeShapeExQuadTo(shape, 61.0f, 111.0f, 25.5f, 83.75f);
	if ( result == XGE_OK ) result = xgeShapeExClose(shape);
	if ( result == XGE_OK ) {
		result = xgeShapeExFillLinearGradient(
			shape, 24.0f, 22.0f, 116.0f, 96.0f,
			XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3
		);
	}
	if ( result == XGE_OK ) result = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(245, 247, 251, 255));
	if ( result == XGE_OK ) result = xgeShapeExStrokeWidth(shape, 3.5f);
	if ( result == XGE_OK ) result = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	if ( result == XGE_OK && kind == PROBE_SHAPE_CLIP ) {
		result = xgeShapeExCreate(&decoration);
		if ( result == XGE_OK ) result = xgeShapeExAppendEllipse(decoration, 70.0f, 61.0f, 39.0f, 30.0f, 1);
		if ( result == XGE_OK ) result = xgeShapeExFillColor(decoration, XGE_COLOR_RGBA(255, 255, 255, 255));
		if ( result == XGE_OK ) result = xgeShapeExClipShapeAdd(shape, decoration);
	} else if ( result == XGE_OK && kind == PROBE_SHAPE_MASK ) {
		result = xgeShapeExCreate(&decoration);
		if ( result == XGE_OK ) result = xgeShapeExAppendEllipse(decoration, 79.0f, 62.0f, 38.0f, 28.0f, 1);
		if ( result == XGE_OK ) result = xgeShapeExFillColor(decoration, XGE_COLOR_RGBA(255, 255, 255, 184));
		if ( result == XGE_OK ) result = xgeShapeExMaskShapeSet(shape, decoration, XGE_SHAPE_EX_MASK_ALPHA);
	} else if ( result == XGE_OK && kind == PROBE_SHAPE_BLEND ) {
		result = xgeShapeExBlend(shape, XGE_BLEND_MULTIPLY);
	}
	xgeShapeExDestroy(decoration);
	if ( result != XGE_OK ) {
		xgeShapeExDestroy(shape);
		return result;
	}
	*out = shape;
	return XGE_OK;
}

static int create_probe_scene(enum probe_kind kind, xge_shape_ex_scene* out)
{
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex shape = NULL;
	xge_shape_ex decoration = NULL;
	int result;

	if ( out == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	result = xgeShapeExSceneCreate(&scene);
	if ( result == XGE_OK ) result = xgeShapeExCreate(&shape);
	if ( result == XGE_OK ) result = xgeShapeExAppendRect(shape, 24.5f, 25.25f, 78.0f, 48.0f, 11.0f, 11.0f, 1);
	if ( result == XGE_OK ) result = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(74, 181, 126, 255));
	if ( result == XGE_OK ) result = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(225, 246, 235, 255));
	if ( result == XGE_OK ) result = xgeShapeExStrokeWidth(shape, 2.5f);
	if ( result == XGE_OK ) result = xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);
	shape = NULL;
	if ( result == XGE_OK ) result = xgeShapeExCreate(&shape);
	if ( result == XGE_OK ) result = xgeShapeExAppendCircle(shape, 104.0f, 79.0f, 25.0f, 18.0f, 1);
	if ( result == XGE_OK ) result = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(240, 173, 76, 220));
	if ( result == XGE_OK ) result = xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);
	if ( result == XGE_OK && kind == PROBE_SCENE_CLIP ) {
		result = xgeShapeExCreate(&decoration);
		if ( result == XGE_OK ) result = xgeShapeExAppendRect(decoration, 31.0f, 29.0f, 88.0f, 58.0f, 8.0f, 8.0f, 1);
		if ( result == XGE_OK ) result = xgeShapeExFillColor(decoration, XGE_COLOR_RGBA(255, 255, 255, 255));
		if ( result == XGE_OK ) result = xgeShapeExSceneClipShapeAdd(scene, decoration);
	} else if ( result == XGE_OK && kind == PROBE_SCENE_MASK ) {
		result = xgeShapeExCreate(&decoration);
		if ( result == XGE_OK ) result = xgeShapeExAppendEllipse(decoration, 83.0f, 60.0f, 43.0f, 31.0f, 1);
		if ( result == XGE_OK ) result = xgeShapeExFillColor(decoration, XGE_COLOR_RGBA(255, 255, 255, 196));
		if ( result == XGE_OK ) result = xgeShapeExSceneMaskShapeSet(scene, decoration, XGE_SHAPE_EX_MASK_ALPHA);
	} else if ( result == XGE_OK && kind == PROBE_SCENE_OPACITY ) {
		result = xgeShapeExSceneOpacity(scene, 0.57f);
	} else if ( result == XGE_OK && kind == PROBE_SCENE_EFFECT ) {
		result = xgeShapeExSceneEffectGaussianBlur(
			scene, 1.8f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 100
		);
		if ( result == XGE_OK ) {
			result = xgeShapeExSceneEffectDropShadow(
				scene, XGE_COLOR_RGBA(9, 12, 18, 176), 135.0f, 7.0f, 2.2f, 100
			);
		}
	}
	xgeShapeExDestroy(decoration);
	if ( result != XGE_OK ) {
		xgeShapeExSceneDestroy(scene);
		return result;
	}
	*out = scene;
	return XGE_OK;
}

static int draw_probe(enum probe_kind kind, int world, const char* output)
{
	static const char svg_basic[] =
		"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 75' preserveAspectRatio='none'>"
		"<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='1'>"
		"<stop offset='0' stop-color='#58c8de'/><stop offset='.55' stop-color='#617fdc'/>"
		"<stop offset='1' stop-color='#ed5889'/></linearGradient></defs>"
		"<path d='M8 12C24 0 62 4 84 18L92 56Q58 76 14 59Z' fill='url(#g)' "
		"stroke='#f5f7fb' stroke-width='2'/></svg>";
	static const char svg_clip_mask[] =
		"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 75' preserveAspectRatio='none'>"
		"<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='1'>"
		"<stop offset='0' stop-color='#58c8de'/><stop offset='1' stop-color='#ed5889'/></linearGradient>"
		"<clipPath id='c'><ellipse cx='51' cy='37' rx='39' ry='28'/></clipPath>"
		"<mask id='m' maskUnits='userSpaceOnUse' x='0' y='0' width='100' height='75'>"
		"<rect x='9' y='8' width='74' height='59' rx='12' fill='white' fill-opacity='.78'/></mask></defs>"
		"<path d='M5 9C25 0 70 2 94 22L88 66H10Z' fill='url(#g)' stroke='#f5f7fb' "
		"stroke-width='2' clip-path='url(#c)' mask='url(#m)'/></svg>";
	static const char svg_filter_blend[] =
		"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 75' preserveAspectRatio='none'>"
		"<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='1'>"
		"<stop offset='0' stop-color='#58c8de'/><stop offset='1' stop-color='#ed5889'/></linearGradient>"
		"<filter id='f' x='-20%' y='-25%' width='140%' height='150%'>"
		"<feGaussianBlur stdDeviation='1.6'/></filter></defs>"
		"<rect x='8' y='10' width='70' height='50' rx='12' fill='#50618f'/>"
		"<ellipse cx='58' cy='39' rx='34' ry='24' fill='url(#g)' fill-opacity='.82' "
		"style='mix-blend-mode:multiply' filter='url(#f)'/></svg>";
	const char* svg_text = svg_basic;
	xge_render_target_t target;
	xge_pass_t pass;
	xge_camera_t camera;
	xge_shape_ex_matrix_t camera_matrix;
	xge_shape_ex shape = NULL;
	xge_shape_ex_scene scene = NULL;
	xge_svg svg = NULL;
	xge_rect_t svg_rect = {22.5f, 19.25f, 112.0f, 88.0f};
	unsigned char* pixels = NULL;
	int stride = PROBE_WIDTH * 4;
	int result;

	memset(&target, 0, sizeof(target));
	memset(&pass, 0, sizeof(pass));
	result = xgeRenderTargetCreate(&target, PROBE_WIDTH, PROBE_HEIGHT);
	if ( kind == PROBE_SVG_CLIP_MASK ) svg_text = svg_clip_mask;
	else if ( kind == PROBE_SVG_FILTER_BLEND ) svg_text = svg_filter_blend;
	if ( result == XGE_OK && kind <= PROBE_SHAPE_BLEND ) result = create_probe_shape(kind, &shape);
	if ( result == XGE_OK && kind >= PROBE_SCENE && kind <= PROBE_SCENE_EFFECT ) result = create_probe_scene(kind, &scene);
	if ( result == XGE_OK && kind >= PROBE_SVG ) result = xgeSvgCreate(&svg);
	if ( result == XGE_OK && kind >= PROBE_SVG ) result = xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text));
	if ( result == XGE_OK ) {
		xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(16, 21, 29, 255));
		result = xgePassBegin(&pass);
	}
	if ( result == XGE_OK && world ) {
		camera = probe_camera();
		xgeCameraSet(&camera);
	}
	camera_matrix = probe_camera_matrix();
	if ( result == XGE_OK && kind <= PROBE_SHAPE_BLEND ) {
		result = world ? xgeShapeExDraw(shape, 0.25f) :
			xgeShapeExDrawPxEx(shape, 0.25f, &camera_matrix, 1.0f);
	} else if ( result == XGE_OK && kind >= PROBE_SCENE && kind <= PROBE_SCENE_EFFECT ) {
		result = world ? xgeShapeExSceneDraw(scene, 0.25f) :
			xgeShapeExSceneDrawPxEx(scene, 0.25f, &camera_matrix, 1.0f);
	} else if ( result == XGE_OK && kind >= PROBE_SVG ) {
		if ( !world ) svg_rect = project_rect(svg_rect);
		result = world ? xgeSvgDraw(svg, svg_rect, 0.25f) : xgeSvgDrawPx(svg, svg_rect, 0.25f);
	}
	if ( pass.bActive ) {
		int end_result = xgePassEnd(&pass);
		if ( result == XGE_OK ) result = end_result;
	}
	if ( result == XGE_OK ) {
		pixels = (unsigned char*)malloc((size_t)stride * PROBE_HEIGHT);
		if ( pixels == NULL ) result = XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( result == XGE_OK ) result = xgeRenderTargetReadPixels(&target, pixels, stride);
	if ( result == XGE_OK ) result = xgeImageSavePNG(output, PROBE_WIDTH, PROBE_HEIGHT, pixels, stride);
	free(pixels);
	xgeSvgDestroy(svg);
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(shape);
	xgeRenderTargetFree(&target);
	return result;
}

static int probe_frame(void* user)
{
	static const char* case_names[PROBE_COUNT] = {
		"shape", "shape_clip", "shape_mask", "shape_blend",
		"scene", "scene_clip", "scene_mask", "scene_opacity", "scene_effect",
		"svg", "svg_clip_mask", "svg_filter_blend"
	};
	probe_app_t* app = (probe_app_t*)user;
	char path[1024];
	int i;

	if ( app == NULL || app->output_dir == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	app->result = XGE_OK;
	for ( i = 0; i < PROBE_COUNT * 2; ++i ) {
		enum probe_kind kind = (enum probe_kind)(i / 2);
		int world = (i % 2) == 0;

		snprintf(path, sizeof(path), "%s\\%s_%s.png", app->output_dir,
			case_names[kind], world ? "world" : "px");
		path[sizeof(path) - 1] = '\0';
		app->result = draw_probe(kind, world, path);
		if ( app->result != XGE_OK ) {
			fprintf(stderr, "%s failed: %d\n", path, app->result);
			break;
		}
	}
	xgeQuit();
	return app->result;
}

int main(int argc, char** argv)
{
	xge_desc_t desc;
	probe_app_t app;
	int result;

	if ( argc != 2 ) {
		fprintf(stderr, "usage: xge_world_draw_probe OUTPUT_DIR\n");
		return 2;
	}
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = PROBE_WIDTH;
	desc.iHeight = PROBE_HEIGHT;
	desc.sTitle = "xge_world_draw_probe";
	desc.iFlags = XGE_INIT_OFFSCREEN;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	app.output_dir = argv[1];
	app.result = XGE_ERROR;
	result = xgeInit(&desc);
	if ( result != XGE_OK ) {
		fprintf(stderr, "xgeInit failed: %d\n", result);
		return 1;
	}
	result = xgeRun(probe_frame, &app);
	if ( result == XGE_OK ) result = app.result;
	xgeUnit();
	return result == XGE_OK ? 0 : 1;
}
