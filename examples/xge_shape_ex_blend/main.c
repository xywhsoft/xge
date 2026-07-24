#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 512
#define DEMO_H 320
#define CELL_W 128
#define CELL_H 64

typedef struct blend_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
} blend_demo_t;

static const int g_blendModes[] = {
	XGE_BLEND_ALPHA,
	XGE_BLEND_MULTIPLY,
	XGE_BLEND_SCREEN,
	XGE_BLEND_OVERLAY,
	XGE_BLEND_DARKEN,
	XGE_BLEND_LIGHTEN,
	XGE_BLEND_COLOR_DODGE,
	XGE_BLEND_COLOR_BURN,
	XGE_BLEND_HARD_LIGHT,
	XGE_BLEND_SOFT_LIGHT,
	XGE_BLEND_DIFFERENCE,
	XGE_BLEND_EXCLUSION,
	XGE_BLEND_HUE,
	XGE_BLEND_SATURATION,
	XGE_BLEND_COLOR,
	XGE_BLEND_LUMINOSITY,
	XGE_BLEND_ADD
};

static int draw_rect(float x, float y, float w, float h, uint32_t color, int blend)
{
	xge_shape_ex shape = NULL;
	int ret = xgeShapeExCreate(&shape);

	if ( ret == XGE_OK ) ret = xgeShapeExAppendRect(shape, x, y, w, h, 0.0f, 0.0f, 1);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, color);
	if ( (ret == XGE_OK) && (blend >= 0) ) ret = xgeShapeExBlend(shape, blend);
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_scene(void)
{
	int count = (int)(sizeof(g_blendModes) / sizeof(g_blendModes[0]));
	int i;

	for ( i = 0; i < count; ++i ) {
		float x = (float)((i % 4) * CELL_W);
		float y = (float)((i / 4) * CELL_H);
		int ret = draw_rect(x + 8.0f, y + 8.0f, 80.0f, 48.0f,
			XGE_COLOR_RGBA(69, 143, 211, 203), -1);

		if ( ret == XGE_OK ) {
			ret = draw_rect(x + 40.0f, y + 16.0f, 80.0f, 40.0f,
				XGE_COLOR_RGBA(231, 71, 151, 179), g_blendModes[i]);
		}
		if ( ret != XGE_OK ) return ret;
	}
	return XGE_OK;
}

static int capture(blend_demo_t* demo)
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
		for ( y = 0; y < DEMO_H; ++y ) {
			unsigned char* row = pixels + (size_t)y * stride;

			for ( x = 0; x < DEMO_W; ++x ) {
				unsigned char* pixel = row + x * 4;
				unsigned int alpha = pixel[3];

				if ( (alpha > 0u) && (alpha < 255u) ) {
					unsigned int channel;
					for ( channel = 0; channel < 3u; ++channel ) {
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
		printf("xge_shape_ex_blend capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

static int frame(void* user)
{
	blend_demo_t* demo = (blend_demo_t*)user;
	xge_pass_t pass;
	int ret = xgeBegin();

	if ( ret != XGE_OK ) return ret;
	xgePassInit(&pass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
	ret = xgePassBegin(&pass);
	if ( ret == XGE_OK ) ret = draw_scene();
	if ( ret == XGE_OK ) ret = xgePassEnd(&pass);
	if ( ret == XGE_OK ) ret = capture(demo);
	if ( ret == XGE_OK ) ret = xgeEnd();
	if ( demo->bCaptureDone ) xgeQuit();
	return ret;
}

int main(int argc, char** argv)
{
	blend_demo_t demo;
	xge_desc_t desc;
	int ret;

	memset(&demo, 0, sizeof(demo));
	snprintf(demo.sCapturePath, sizeof(demo.sCapturePath), "%s",
		(argc > 1) ? argv[1] : "artifacts/xge_shape_ex_blend.png");
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W;
	desc.iHeight = DEMO_H;
	desc.sTitle = "xge_shape_ex_blend";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret == XGE_OK ) ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
