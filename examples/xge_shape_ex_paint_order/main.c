#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 576
#define DEMO_H 480
#define CELL_W 192
#define CELL_H 160

typedef struct paint_order_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
} paint_order_demo_t;

static int append_open_cubic(xge_shape_ex shape, float x, float y)
{
	int ret = xgeShapeExMoveTo(shape, x + 20.0f, y + 124.0f);
	if ( ret == XGE_OK ) {
		ret = xgeShapeExCubicTo(shape,
			x + 20.0f, y + 28.0f,
			x + 172.0f, y + 28.0f,
			x + 172.0f, y + 124.0f);
	}
	return ret;
}

static int append_open_polyline(xge_shape_ex shape, float x, float y)
{
	int ret = xgeShapeExMoveTo(shape, x + 20.0f, y + 124.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExLineTo(shape, x + 96.0f, y + 24.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExLineTo(shape, x + 172.0f, y + 124.0f);
	return ret;
}

static int append_closed_triangle(xge_shape_ex shape, float x, float y)
{
	int ret = xgeShapeExMoveTo(shape, x + 24.0f, y + 132.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExLineTo(shape, x + 96.0f, y + 20.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExLineTo(shape, x + 168.0f, y + 132.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExClose(shape);
	return ret;
}

static int draw_case(int index)
{
	xge_shape_ex shape = NULL;
	float x = (float)((index % 3) * CELL_W);
	float y = (float)((index / 3) * CELL_H);
	int ret = xgeShapeExCreate(&shape);

	if ( index == 5 || index == 8 ) {
		x += 0.5f;
		y += 0.5f;
	}
	if ( ret == XGE_OK ) {
		if ( index >= 6 ) ret = append_closed_triangle(shape, x, y);
		else if ( index == 2 || index == 3 ) ret = append_open_polyline(shape, x, y);
		else ret = append_open_cubic(shape, x, y);
	}
	if ( (ret == XGE_OK) && (index == 4) ) ret = xgeShapeExClose(shape);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, XGE_COLOR_RGBA(74, 163, 223, 220));
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(239, 82, 118, 255));
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, index >= 6 ? 20.0f : 10.0f);
	if ( ret == XGE_OK ) {
		ret = xgeShapeExPaintOrder(shape,
			index == 0 || index == 2 || index == 4 || index == 5 || index == 7);
	}
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_scene(void)
{
	int i;
	for ( i = 0; i < 9; ++i ) {
		int ret = draw_case(i);
		if ( ret != XGE_OK ) return ret;
	}
	return XGE_OK;
}

static int capture(paint_order_demo_t* demo)
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
		printf("xge_shape_ex_paint_order capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

static int frame(void* user)
{
	paint_order_demo_t* demo = (paint_order_demo_t*)user;
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
	paint_order_demo_t demo;
	xge_desc_t desc;
	int ret;

	memset(&demo, 0, sizeof(demo));
	snprintf(demo.sCapturePath, sizeof(demo.sCapturePath), "%s",
		(argc > 1) ? argv[1] : "artifacts/xge_shape_ex_paint_order.png");
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W;
	desc.iHeight = DEMO_H;
	desc.sTitle = "xge_shape_ex_paint_order";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret == XGE_OK ) ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
