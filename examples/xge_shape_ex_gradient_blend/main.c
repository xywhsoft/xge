#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 576
#define DEMO_H 640
#define CELL_W 192
#define CELL_H 160

typedef struct gradient_blend_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
} gradient_blend_demo_t;

static int draw_solid_rect(float x, float y, float w, float h, uint32_t color)
{
	xge_shape_ex shape = NULL;
	int ret = xgeShapeExCreate(&shape);

	if ( ret == XGE_OK ) ret = xgeShapeExAppendRect(shape, x, y, w, h, 0.0f, 0.0f, 1);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, color);
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_solid_ellipse(float cx, float cy, float rx, float ry, uint32_t color)
{
	xge_shape_ex shape = NULL;
	int ret = xgeShapeExCreate(&shape);

	if ( ret == XGE_OK ) ret = xgeShapeExAppendEllipse(shape, cx, cy, rx, ry, 1);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, color);
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_case(int index)
{
	xge_shape_ex shape = NULL;
	xge_shape_ex_color_stop_t stops[3];
	float x = (float)((index % 3) * CELL_W);
	float y = (float)((index / 3) * CELL_H);
	int ret;

	ret = draw_solid_rect(x, y, (float)CELL_W, (float)CELL_H, XGE_COLOR_RGBA(36, 66, 95, 255));
	if ( ret == XGE_OK ) {
		ret = draw_solid_ellipse(x + 68.0f, y + 82.0f, 46.0f, 46.0f,
			XGE_COLOR_RGBA(244, 197, 66, 255));
	}
	if ( ret != XGE_OK ) return ret;
	ret = xgeShapeExCreate(&shape);
	if ( ret != XGE_OK ) return ret;

	if ( index == 0 ) {
		ret = xgeShapeExAppendRect(shape, x + 36.0f, y + 30.0f, 120.0f, 100.0f, 22.0f, 22.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(237, 66, 100, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.52f, XGE_COLOR_RGBA(255, 209, 102, 180)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(88, 201, 139, 96)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillLinearGradient(shape, x + 36.0f, y + 30.0f, x + 156.0f, y + 130.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_MULTIPLY);
	} else if ( index == 1 ) {
		ret = xgeShapeExAppendEllipse(shape, x + 98.0f, y + 82.0f, 60.0f, 42.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(255, 245, 185, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.58f, XGE_COLOR_RGBA(255, 120, 86, 200)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(137, 91, 255, 96)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillRadialGradient(shape, x + 98.0f, y + 82.0f, 60.0f, x + 80.0f, y + 65.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_SCREEN);
	} else if ( index == 2 ) {
		ret = xgeShapeExMoveTo(shape, x + 24.0f, y + 122.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(shape, x + 42.0f, y + 24.0f, x + 150.0f, y + 24.0f, x + 168.0f, y + 122.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(125, 211, 252, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(239, 82, 118, 170)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(6, 214, 160, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeLinearGradient(shape, x + 24.0f, y + 82.0f, x + 168.0f, y + 82.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 18.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_DIFFERENCE);
	} else if ( index == 3 ) {
		ret = xgeShapeExAppendEllipse(shape, x + 96.0f, y + 80.0f, 52.0f, 34.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(255, 209, 102, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(239, 71, 111, 255)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(131, 56, 236, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillRadialGradient(shape, x + 96.0f, y + 80.0f, 58.0f, x + 86.0f, y + 70.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(125, 211, 252, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(196, 181, 253, 255)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(6, 214, 160, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeLinearGradient(shape, x + 38.0f, y + 80.0f, x + 154.0f, y + 80.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 14.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExPaintOrder(shape, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_HARD_LIGHT);
	} else if ( index == 4 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(249, 115, 22, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(236, 72, 153, 255)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(139, 92, 246, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillLinearGradient(shape, x + 34.0f, y + 36.0f, x + 158.0f, y + 124.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(125, 211, 252, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 12.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else if ( index == 5 ) {
		ret = xgeShapeExAppendEllipse(shape, x + 96.0f, y + 80.0f, 52.0f, 34.0f, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(239, 71, 111, 255));
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(125, 211, 252, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(196, 181, 253, 180)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(6, 214, 160, 96)};
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeRadialGradient(shape, x + 96.0f, y + 80.0f, 64.0f, x + 84.0f, y + 66.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 14.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_SCREEN);
	} else if ( index == 6 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(249, 115, 22, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(236, 72, 153, 255)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(139, 92, 246, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillLinearGradient(shape, x + 34.0f, y + 36.0f, x + 158.0f, y + 124.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else if ( index == 7 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(125, 211, 252, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 12.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else if ( index == 8 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		stops[0] = (xge_shape_ex_color_stop_t){0.0f, XGE_COLOR_RGBA(249, 115, 22, 255)};
		stops[1] = (xge_shape_ex_color_stop_t){0.5f, XGE_COLOR_RGBA(236, 72, 153, 255)};
		stops[2] = (xge_shape_ex_color_stop_t){1.0f, XGE_COLOR_RGBA(139, 92, 246, 255)};
		if ( ret == XGE_OK ) ret = xgeShapeExFillLinearGradient(shape, x + 34.0f, y + 36.0f, x + 158.0f, y + 124.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(128, 128, 128, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 12.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else if ( index == 9 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(236, 72, 153, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(125, 211, 252, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 12.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else if ( index == 10 ) {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(236, 72, 153, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	} else {
		ret = xgeShapeExAppendRect(shape, x + 34.0f, y + 36.0f, 124.0f, 88.0f, 18.0f, 18.0f, 1);
		if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(125, 211, 252, 255));
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, 12.0f);
		if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		if ( ret == XGE_OK ) ret = xgeShapeExBlend(shape, XGE_BLEND_OVERLAY);
	}
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_scene(void)
{
	int i;
	for ( i = 0; i < 12; ++i ) {
		int ret = draw_case(i);
		if ( ret != XGE_OK ) return ret;
	}
	return XGE_OK;
}

static int capture(gradient_blend_demo_t* demo)
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
		printf("xge_shape_ex_gradient_blend capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

static int frame(void* user)
{
	gradient_blend_demo_t* demo = (gradient_blend_demo_t*)user;
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
	gradient_blend_demo_t demo;
	xge_desc_t desc;
	int ret;

	memset(&demo, 0, sizeof(demo));
	snprintf(demo.sCapturePath, sizeof(demo.sCapturePath), "%s",
		(argc > 1) ? argv[1] : "artifacts/xge_shape_ex_gradient_blend.png");
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W;
	desc.iHeight = DEMO_H;
	desc.sTitle = "xge_shape_ex_gradient_blend";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret == XGE_OK ) ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
