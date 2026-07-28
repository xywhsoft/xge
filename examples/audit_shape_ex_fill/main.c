#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIT_W 900
#define AUDIT_H 1000
#define BG_COLOR XGE_COLOR_RGBA(20, 25, 31, 255)

typedef struct audit_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} audit_demo_t;

static xge_rect_t rectf(float x, float y, float w, float h)
{
	xge_rect_t r;
	r.fX = x; r.fY = y; r.fW = w; r.fH = h;
	return r;
}

static int parse_args(audit_demo_t* demo, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			demo->iMaxFrames = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			demo->iMaxFrames = atoi(argv[i] + 9);
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[++i]);
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[i] + 10);
		}
	}
	return XGE_OK;
}

static int capture(audit_demo_t* demo)
{
	unsigned char* pixels;
	int stride, ret;
	if ( (demo == NULL) || (demo->sCapturePath[0] == '\0') || demo->bCaptureDone ) return XGE_OK;
	stride = AUDIT_W * 4;
	pixels = (unsigned char*)malloc((size_t)stride * AUDIT_H);
	if ( pixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&demo->tTarget, pixels, stride);
	if ( ret == XGE_OK ) ret = xgeImageSavePNG(demo->sCapturePath, AUDIT_W, AUDIT_H, pixels, stride);
	free(pixels);
	if ( ret == XGE_OK ) { demo->bCaptureDone = 1; printf("capture saved: %s\n", demo->sCapturePath); }
	return ret;
}

/* Build a star path (self-intersecting pentagram) for fill rule testing */
static void build_star_path(xge_shape_ex shape, float cx, float cy, float r)
{
	int i;
	float angle;
	xge_vec2_t pts[5];
	for ( i = 0; i < 5; i++ ) {
		angle = (float)i * 4.0f * 3.14159265f / 5.0f - 1.5708f;
		pts[i].fX = cx + cosf(angle) * r;
		pts[i].fY = cy + sinf(angle) * r;
	}
	xgeShapeExMoveTo(shape, pts[0].fX, pts[0].fY);
	for ( i = 1; i < 5; i++ ) xgeShapeExLineTo(shape, pts[i].fX, pts[i].fY);
	xgeShapeExClose(shape);
}

/* Build two overlapping rectangles for fill rule testing */
static void build_overlapping_rects(xge_shape_ex shape, float x, float y)
{
	xgeShapeExAppendRect(shape, x, y, 60.0f, 60.0f, 0, 0, 1);
	xgeShapeExAppendRect(shape, x + 30.0f, y + 30.0f, 60.0f, 60.0f, 0, 0, 1);
}

/* Section 1: Fill rule non-zero vs even-odd */
static void draw_fill_rules(float fBaseY)
{
	xge_shape_ex shape;
	uint32_t iColor = XGE_COLOR_RGBA(100, 200, 255, 255);

	/* Non-zero star */
	xgeShapeExCreate(&shape);
	build_star_path(shape, 80.0f, fBaseY + 50.0f, 40.0f);
	xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_NON_ZERO);
	xgeShapeExFillColor(shape, iColor);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Even-odd star */
	xgeShapeExCreate(&shape);
	build_star_path(shape, 220.0f, fBaseY + 50.0f, 40.0f);
	xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD);
	xgeShapeExFillColor(shape, iColor);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Non-zero overlapping rects */
	xgeShapeExCreate(&shape);
	build_overlapping_rects(shape, 320.0f, fBaseY + 10.0f);
	xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_NON_ZERO);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(255, 150, 80, 255));
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Even-odd overlapping rects */
	xgeShapeExCreate(&shape);
	build_overlapping_rects(shape, 460.0f, fBaseY + 10.0f);
	xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(255, 150, 80, 255));
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

/* Section 2: Linear gradient */
static void draw_linear_gradients(float fBaseY)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[3];

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(0, 255, 0, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);

	/* Horizontal linear gradient */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 30.0f, fBaseY, 200.0f, 80.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 30.0f, 0, 230.0f, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Vertical linear gradient */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 260.0f, fBaseY, 200.0f, 80.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 0, fBaseY, 0, fBaseY + 80.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Diagonal linear gradient */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 490.0f, fBaseY, 200.0f, 80.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 490.0f, fBaseY, 690.0f, fBaseY + 80.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

/* Section 3: Radial gradient */
static void draw_radial_gradients(float fBaseY)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[2];

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(255, 0, 128, 255);

	/* Centered radial */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 30.0f, fBaseY, 150.0f, 100.0f, 0, 0, 1);
	xgeShapeExFillRadialGradient(shape, 105.0f, fBaseY + 50.0f, 60.0f, 105.0f, fBaseY + 50.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Offset focal radial */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 220.0f, fBaseY, 150.0f, 100.0f, 0, 0, 1);
	xgeShapeExFillRadialGradient(shape, 295.0f, fBaseY + 50.0f, 60.0f, 270.0f, fBaseY + 30.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Radial with focal radius */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 410.0f, fBaseY, 150.0f, 100.0f, 0, 0, 1);
	xgeShapeExFillRadialGradientEx(shape, 485.0f, fBaseY + 50.0f, 60.0f, 485.0f, fBaseY + 50.0f, 20.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

/* Section 4: Gradient spread modes */
static void draw_gradient_spread(float fBaseY)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[2];

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 200, 0, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(0, 100, 255, 255);

	/* Pad */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 30.0f, fBaseY, 250.0f, 60.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 80.0f, 0, 180.0f, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_PAD);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Reflect */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 310.0f, fBaseY, 250.0f, 60.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 360.0f, 0, 460.0f, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Repeat */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 590.0f, fBaseY, 250.0f, 60.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 640.0f, 0, 740.0f, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

/* Section 5: Paint order */
static void draw_paint_order(float fBaseY)
{
	xge_shape_ex shape;
	uint32_t iFill = XGE_COLOR_RGBA(80, 180, 255, 255);
	uint32_t iStroke = XGE_COLOR_RGBA(255, 80, 80, 255);

	/* Fill first (default) - stroke overlaps fill edge */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 50.0f, fBaseY, 120.0f, 80.0f, 10.0f, 10.0f, 1);
	xgeShapeExFillColor(shape, iFill);
	xgeShapeExStrokeColor(shape, iStroke);
	xgeShapeExStrokeWidth(shape, 8.0f);
	xgeShapeExPaintOrder(shape, 0); /* fill first */
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Stroke first - fill overlaps stroke inner edge */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 250.0f, fBaseY, 120.0f, 80.0f, 10.0f, 10.0f, 1);
	xgeShapeExFillColor(shape, iFill);
	xgeShapeExStrokeColor(shape, iStroke);
	xgeShapeExStrokeWidth(shape, 8.0f);
	xgeShapeExPaintOrder(shape, 1); /* stroke first */
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Fill only (no stroke set) */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 450.0f, fBaseY, 120.0f, 80.0f, 10.0f, 10.0f, 1);
	xgeShapeExFillColor(shape, iFill);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Stroke only (no fill set) */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 650.0f, fBaseY, 120.0f, 80.0f, 10.0f, 10.0f, 1);
	xgeShapeExStrokeColor(shape, iStroke);
	xgeShapeExStrokeWidth(shape, 4.0f);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

/* Section 6: Object bounding box gradient units */
static void draw_gradient_units(float fBaseY)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[2];

	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(200, 50, 255, 255);
	stops[1].fOffset = 1.0f; stops[1].iColor = XGE_COLOR_RGBA(50, 255, 150, 255);

	/* User space gradient on rect */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 50.0f, fBaseY, 180.0f, 70.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 50.0f, 0, 230.0f, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* Object bounding box gradient on same rect */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 280.0f, fBaseY, 180.0f, 70.0f, 0, 0, 1);
	xgeShapeExFillLinearGradient(shape, 0.0f, 0, 1.0f, 0, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);

	/* OBB gradient on circle */
	xgeShapeExCreate(&shape);
	xgeShapeExAppendCircle(shape, 580.0f, fBaseY + 35.0f, 40.0f, 40.0f, 1);
	xgeShapeExFillLinearGradient(shape, 0.0f, 0, 1.0f, 0, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2);
	xgeShapeExDraw(shape, 0.5f);
	xgeShapeExDestroy(shape);
}

static void draw_marker(float x, float y)
{
	xgeShapeRectFill(rectf(x, y, 4.0f, 4.0f), XGE_COLOR_RGBA(255, 255, 255, 100));
}

static int frame(void* user)
{
	audit_demo_t* demo = (audit_demo_t*)user;
	xge_pass_t tPass;
	int ret;

	if ( demo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;

	xgePassInit(&tPass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&tPass);
	if ( ret != XGE_OK ) return ret;

	/* Section 1: Fill rules (y=10..110) */
	draw_marker(10.0f, 5.0f);
	draw_fill_rules(10.0f);

	/* Section 2: Linear gradients (y=130..220) */
	draw_marker(10.0f, 125.0f);
	draw_linear_gradients(130.0f);

	/* Section 3: Radial gradients (y=240..350) */
	draw_marker(10.0f, 235.0f);
	draw_radial_gradients(240.0f);

	/* Section 4: Gradient spread (y=370..440) */
	draw_marker(10.0f, 365.0f);
	draw_gradient_spread(370.0f);

	/* Section 5: Paint order (y=460..550) */
	draw_marker(10.0f, 455.0f);
	draw_paint_order(460.0f);

	/* Section 6: Gradient units (y=570..650) */
	draw_marker(10.0f, 565.0f);
	draw_gradient_units(570.0f);

	ret = xgePassEnd(&tPass);
	if ( ret != XGE_OK ) return ret;
	ret = capture(demo);
	if ( ret != XGE_OK ) return ret;
	xgeEnd();
	demo->iFrame++;
	if ( demo->bCaptureDone || ((demo->iMaxFrames > 0) && (demo->iFrame >= demo->iMaxFrames)) ) xgeQuit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	audit_demo_t tDemo;
	xge_desc_t tDesc;
	int ret;

	memset(&tDemo, 0, sizeof(tDemo));
	ret = parse_args(&tDemo, argc, argv);
	if ( ret != XGE_OK ) { fprintf(stderr, "invalid arguments\n"); return 1; }

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = AUDIT_W;
	tDesc.iHeight = AUDIT_H;
	tDesc.sTitle = "audit_shape_ex_fill";
	tDesc.iFlags = XGE_INIT_OFFSCREEN;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;

	ret = xgeInit(&tDesc);
	if ( ret != XGE_OK ) { fprintf(stderr, "xgeInit failed: %d\n", ret); return 1; }
	ret = xgeRenderTargetCreate(&tDemo.tTarget, AUDIT_W, AUDIT_H);
	if ( ret != XGE_OK ) { xgeUnit(); return 1; }

	ret = xgeRun(frame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
