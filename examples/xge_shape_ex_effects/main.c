#include "../../xge.h"

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 960
#define DEMO_H 640
#define CELL_W 160
#define CELL_H 128
#define CELL_COLUMNS 6
#define CELL_COUNT 30

typedef struct effects_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int iRepeat;
	int bCaptureDone;
} effects_demo_t;

static int add_source_shapes(xge_shape_ex_scene scene)
{
	xge_shape_ex shape = NULL;
	int ret = xgeShapeExCreate(&shape);

	if ( ret == XGE_OK ) ret = xgeShapeExAppendRect(shape, 8.0f, 18.0f, 112.0f, 62.0f, 18.0f, 18.0f, 1);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(74, 177, 230, 255));
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(190, 231, 252, 255));
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 3.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);
	if ( ret != XGE_OK ) return ret;

	shape = NULL;
	ret = xgeShapeExCreate(&shape);
	if ( ret == XGE_OK ) ret = xgeShapeExAppendCircle(shape, 94.0f, 36.0f, 18.0f, 18.0f, 1);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(244, 187, 78, 230));
	if ( ret == XGE_OK ) ret = xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);
	return ret;
}

static int add_effect(xge_shape_ex_scene scene, int index)
{
	if ( index == 0 ) {
		return xgeShapeExSceneEffectGaussianBlur(scene, 2.2f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 100);
	}
	if ( index == 1 ) {
		return xgeShapeExSceneEffectDropShadow(scene, XGE_COLOR_RGBA(0, 0, 0, 190), 135.0f, 12.0f, 3.0f, 100);
	}
	if ( index == 2 ) {
		return xgeShapeExSceneEffectFill(scene, XGE_COLOR_RGBA(234, 83, 126, 220));
	}
	if ( index == 3 ) {
		return xgeShapeExSceneEffectTint(scene, XGE_COLOR_RGBA(20, 32, 66, 255), XGE_COLOR_RGBA(130, 246, 208, 255), 88.0f);
	}
	return xgeShapeExSceneEffectTritone(
		scene,
		XGE_COLOR_RGBA(22, 27, 48, 255),
		XGE_COLOR_RGBA(148, 80, 190, 255),
		XGE_COLOR_RGBA(255, 222, 112, 255),
		24
	);
}

static int set_rotation_about(xge_shape_ex_scene scene, float radians, float center_x, float center_y)
{
	xge_shape_ex_matrix_t transform;
	float sine = sinf(radians);
	float cosine = cosf(radians);

	transform.fA = cosine;
	transform.fB = sine;
	transform.fC = -sine;
	transform.fD = cosine;
	transform.fE = center_x - cosine * center_x + sine * center_y;
	transform.fF = center_y - sine * center_x - cosine * center_y;
	return xgeShapeExSceneTransformSet(scene, &transform);
}

static int create_effect_scene(int index, xge_shape_ex_scene* ppScene)
{
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex_scene child = NULL;
	xge_shape_ex_scene clone = NULL;
	int ret;

	if ( ppScene == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	*ppScene = NULL;
	ret = xgeShapeExSceneCreate(&scene);
	if ( ret != XGE_OK ) return ret;
	if ( (index == 6) || (index == 11) || (index == 22) || (index == 23) ) {
		ret = xgeShapeExSceneCreate(&child);
		if ( ret == XGE_OK ) ret = add_source_shapes(child);
		if ( (ret == XGE_OK) && (index == 6) ) ret = xgeShapeExSceneEffectTint(
			child, XGE_COLOR_RGBA(16, 34, 70, 255), XGE_COLOR_RGBA(246, 210, 96, 255), 72.0f
		);
		if ( (ret == XGE_OK) && (index == 22) ) ret = xgeShapeExSceneEffectGaussianBlur(
			child, 2.5f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 65
		);
		if ( (ret == XGE_OK) && (index == 22) ) ret = set_rotation_about(
			child, 15.0f * (3.14159265358979323846f / 180.0f), 64.0f, 50.0f
		);
		if ( (ret == XGE_OK) && (index == 23) ) ret = xgeShapeExSceneEffectDropShadow(
			child, XGE_COLOR_RGBA(18, 22, 46, 185), 70.0f, 9.0f, 2.7f, 35
		);
		if ( (ret == XGE_OK) && (index == 23) ) {
			xge_shape_ex_matrix_t child_transform;

			child_transform.fA = 0.75f;
			child_transform.fB = 0.0f;
			child_transform.fC = 0.0f;
			child_transform.fD = 1.15f;
			child_transform.fE = 16.0f;
			child_transform.fF = -7.5f;
			ret = xgeShapeExSceneTransformSet(child, &child_transform);
		}
		if ( ret == XGE_OK ) ret = xgeShapeExSceneAddScene(scene, child);
		if ( (ret == XGE_OK) && (index == 6) ) ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 0.7f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 80
		);
		xgeShapeExSceneDestroy(child);
	} else {
		ret = add_source_shapes(scene);
	}
	if ( ret == XGE_OK && index < 5 ) {
		ret = add_effect(scene, index);
	} else if ( ret == XGE_OK && index == 5 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(scene, 1.2f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 85);
		if ( ret == XGE_OK ) ret = xgeShapeExSceneEffectTint(
			scene, XGE_COLOR_RGBA(18, 30, 58, 255), XGE_COLOR_RGBA(116, 238, 204, 255), 76.0f
		);
		if ( ret == XGE_OK ) ret = xgeShapeExSceneEffectTritone(
			scene, XGE_COLOR_RGBA(24, 26, 50, 255), XGE_COLOR_RGBA(146, 82, 188, 255),
			XGE_COLOR_RGBA(250, 220, 120, 255), 48
		);
	} else if ( ret == XGE_OK && index == 7 ) {
		ret = xgeShapeExSceneEffectFill(scene, XGE_COLOR_RGBA(58, 208, 150, 210));
		if ( ret == XGE_OK ) ret = xgeShapeExSceneOpacity(scene, 140.0f / 255.0f);
	} else if ( ret == XGE_OK && index == 8 ) {
		ret = xgeShapeExSceneEffectDropShadow(scene, XGE_COLOR_RGBA(24, 18, 54, 180), 45.0f, 10.0f, 2.4f, 90);
	} else if ( ret == XGE_OK && index == 9 ) {
		ret = xgeShapeExSceneEffectTritone(
			scene, XGE_COLOR_RGBA(20, 30, 56, 255), XGE_COLOR_RGBA(202, 92, 148, 255),
			XGE_COLOR_RGBA(255, 226, 128, 255), 32
		);
		if ( ret == XGE_OK ) ret = xgeShapeExSceneEffectDropShadow(
			scene, XGE_COLOR_RGBA(0, 0, 0, 150), 110.0f, 7.0f, 1.5f, 75
		);
		if ( ret == XGE_OK ) ret = xgeShapeExSceneClone(scene, &clone);
		if ( ret == XGE_OK ) {
			xgeShapeExSceneDestroy(scene);
			scene = clone;
			clone = NULL;
		}
	} else if ( ret == XGE_OK && (index == 10 || index == 11) ) {
		ret = xgeShapeExSceneOpacity(scene, 140.0f / 255.0f);
	} else if ( ret == XGE_OK && index == 12 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_HORIZONTAL, XGE_SHAPE_EX_BORDER_DUPLICATE, 100
		);
	} else if ( ret == XGE_OK && index == 13 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_VERTICAL, XGE_SHAPE_EX_BORDER_DUPLICATE, 100
		);
	} else if ( ret == XGE_OK && index == 14 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_WRAP, 100
		);
	} else if ( ret == XGE_OK && index == 15 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_HORIZONTAL, XGE_SHAPE_EX_BORDER_WRAP, 100
		);
	} else if ( ret == XGE_OK && index == 16 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 20
		);
	} else if ( ret == XGE_OK && index == 17 ) {
		ret = xgeShapeExSceneEffectDropShadow(
			scene, XGE_COLOR_RGBA(16, 20, 38, 190), 135.0f, 10.0f, 3.5f, 20
		);
	} else if ( ret == XGE_OK && (index == 18 || index == 19) ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 2.8f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 70
		);
	} else if ( ret == XGE_OK && (index == 20 || index == 21) ) {
		ret = xgeShapeExSceneEffectDropShadow(
			scene, XGE_COLOR_RGBA(12, 18, 42, 180), 118.0f, 9.0f, 2.6f, 55
		);
	} else if ( ret == XGE_OK && index == 24 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, -2.0f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 70
		);
	} else if ( ret == XGE_OK && index == 25 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 2.8f, 9, XGE_SHAPE_EX_BORDER_DUPLICATE, 70
		);
	} else if ( ret == XGE_OK && index == 26 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_BOTH, 9, 100
		);
	} else if ( ret == XGE_OK && index == 27 ) {
		ret = xgeShapeExSceneEffectGaussianBlur(
			scene, 3.5f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 101
		);
	} else if ( ret == XGE_OK && index == 28 ) {
		ret = xgeShapeExSceneEffectDropShadow(
			scene, XGE_COLOR_RGBA(12, 18, 42, 180), 118.0f, 9.0f, -2.0f, 55
		);
	} else if ( ret == XGE_OK && index == 29 ) {
		ret = xgeShapeExSceneEffectDropShadow(
			scene, XGE_COLOR_RGBA(12, 18, 42, 180), 118.0f, 9.0f, 2.6f, 101
		);
	}
	xgeShapeExSceneDestroy(clone);
	if ( ret != XGE_OK ) {
		xgeShapeExSceneDestroy(scene);
		return ret;
	}
	*ppScene = scene;
	return XGE_OK;
}

static int draw_effects(void)
{
	int i;

	for ( i = 0; i < CELL_COUNT; i++ ) {
		xge_shape_ex_scene scene = NULL;
		xge_shape_ex_matrix_t transform;
		int ret = create_effect_scene(i, &scene);

		if ( ret == XGE_OK ) ret = xgeShapeExMatrixIdentity(&transform);
		if ( ret == XGE_OK ) {
			transform.fE = 16.0f + (float)((i % CELL_COLUMNS) * CELL_W);
			transform.fF = 16.0f + (float)((i / CELL_COLUMNS) * CELL_H);
			if ( i == 8 ) {
				transform.fA = 0.82f;
				transform.fD = 0.82f;
				transform.fE += 14.0f;
				transform.fF += 10.0f;
			} else if ( i == 18 || i == 20 || i == 23 ) {
				float radians = (i == 20 ? -24.0f : (i == 23 ? -12.0f : 24.0f)) *
					(3.14159265358979323846f / 180.0f);
				float sine = sinf(radians);
				float cosine = cosf(radians);
				float center_x = 64.0f;
				float center_y = 50.0f;
				float origin_x = transform.fE;
				float origin_y = transform.fF;

				transform.fA = cosine;
				transform.fB = sine;
				transform.fC = -sine;
				transform.fD = cosine;
				transform.fE = origin_x + center_x - cosine * center_x + sine * center_y;
				transform.fF = origin_y + center_y - sine * center_x - cosine * center_y;
			} else if ( i == 19 ) {
				transform.fA = 1.25f;
				transform.fD = 0.72f;
				transform.fE = (float)((i % CELL_COLUMNS) * CELL_W);
				transform.fF += 14.0f;
			} else if ( i == 21 ) {
				transform.fA = 0.72f;
				transform.fD = 1.18f;
				transform.fE += 32.0f;
				transform.fF += 2.0f;
			} else if ( i == 22 ) {
				transform.fA = 0.90f;
				transform.fD = 0.90f;
				transform.fE += 7.0f;
				transform.fF += 5.0f;
			}
			ret = xgeShapeExSceneTransformSet(scene, &transform);
		}
		if ( ret == XGE_OK ) ret = xgeShapeExSceneDrawPx(scene, 0.25f);
		xgeShapeExSceneDestroy(scene);
		if ( ret != XGE_OK ) return ret;
	}
	return XGE_OK;
}

static int capture(effects_demo_t* demo)
{
	unsigned char* pixels;
	int stride = DEMO_W * 4;
	int ret;
	int x;
	int y;

	if ( demo->bCaptureDone ) return XGE_OK;
	pixels = (unsigned char*)malloc((size_t)stride * DEMO_H);
	if ( pixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&demo->tTarget, pixels, stride);
	if ( ret == XGE_OK ) {
		for ( y = 0; y < DEMO_H; y++ ) {
			unsigned char* row = pixels + (size_t)y * stride;

			for ( x = 0; x < DEMO_W; x++ ) {
				unsigned char* pixel = row + x * 4;
				unsigned int alpha = pixel[3];

				if ( alpha > 0u && alpha < 255u ) {
					unsigned int channel;
					for ( channel = 0; channel < 3u; channel++ ) {
						unsigned int value = ((unsigned int)pixel[channel] * 255u + alpha / 2u) / alpha;
						pixel[channel] = (unsigned char)(value > 255u ? 255u : value);
					}
				}
			}
		}
	}
	if ( ret == XGE_OK ) ret = xgeImageSavePNG(demo->sCapturePath, DEMO_W, DEMO_H, pixels, stride);
	free(pixels);
	if ( ret == XGE_OK ) {
		demo->bCaptureDone = 1;
		printf("xge_shape_ex_effects capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

static int frame(void* user)
{
	effects_demo_t* demo = (effects_demo_t*)user;
	xge_pass_t pass;
	double fStart;
	double fSeconds;
	int i;
	int ret = xgeBegin();

	if ( ret != XGE_OK ) return ret;
	fStart = xgeTimer();
	for ( i = 0; (i < demo->iRepeat) && (ret == XGE_OK); i++ ) {
		xgePassInit(&pass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
		ret = xgePassBegin(&pass);
		if ( ret == XGE_OK ) ret = draw_effects();
		if ( ret == XGE_OK ) ret = xgePassEnd(&pass);
	}
	fSeconds = xgeTimer() - fStart;
	if ( ret == XGE_OK ) {
		printf(
			"xge_shape_ex_effects render-benchmark repeats=%d total_ms=%.3f average_ms=%.3f\n",
			demo->iRepeat, fSeconds * 1000.0, fSeconds * 1000.0 / (double)demo->iRepeat
		);
	}
	if ( ret == XGE_OK ) ret = capture(demo);
	if ( ret == XGE_OK ) ret = xgeEnd();
	if ( demo->bCaptureDone ) xgeQuit();
	return ret;
}

static int parse_positive_int(const char* text, int* value)
{
	char* end;
	long parsed;

	if ( (text == NULL) || (value == NULL) || (*text == '\0') ) return 0;
	end = NULL;
	parsed = strtol(text, &end, 10);
	if ( (end == text) || (*end != '\0') || (parsed <= 0) || (parsed > INT_MAX) ) return 0;
	*value = (int)parsed;
	return 1;
}

int main(int argc, char** argv)
{
	effects_demo_t demo;
	xge_desc_t desc;
	int i;
	int ret;

	memset(&demo, 0, sizeof(demo));
	demo.iRepeat = 1;
	snprintf(demo.sCapturePath, sizeof(demo.sCapturePath), "%s",
		((argc > 1) && (strncmp(argv[1], "--", 2) != 0)) ?
			argv[1] : "artifacts/xge_shape_ex_effects.png");
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--repeat") == 0 ) {
			if ( (++i >= argc) || !parse_positive_int(argv[i], &demo.iRepeat) ) return 2;
		} else if ( strncmp(argv[i], "--repeat=", 9) == 0 ) {
			if ( !parse_positive_int(argv[i] + 9, &demo.iRepeat) ) return 2;
		} else if ( (i != 1) || (strncmp(argv[i], "--", 2) == 0) ) {
			return 2;
		}
	}
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W;
	desc.iHeight = DEMO_H;
	desc.sTitle = "xge_shape_ex_effects";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret == XGE_OK ) ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return ret == XGE_OK ? 0 : 1;
}
