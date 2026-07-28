#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIT_W 900
#define AUDIT_H 900
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
	xge_rect_t r; r.fX = x; r.fY = y; r.fW = w; r.fH = h; return r;
}

static int parse_args(audit_demo_t* d, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && i+1 < argc ) d->iMaxFrames = atoi(argv[++i]);
		else if ( strncmp(argv[i], "--frames=", 9) == 0 ) d->iMaxFrames = atoi(argv[i]+9);
		else if ( strcmp(argv[i], "--capture") == 0 && i+1 < argc ) snprintf(d->sCapturePath, 260, "%s", argv[++i]);
		else if ( strncmp(argv[i], "--capture=", 10) == 0 ) snprintf(d->sCapturePath, 260, "%s", argv[i]+10);
	}
	return XGE_OK;
}

static int capture(audit_demo_t* d)
{
	unsigned char* px; int stride, ret;
	if ( !d || !d->sCapturePath[0] || d->bCaptureDone ) return XGE_OK;
	stride = AUDIT_W * 4;
	px = (unsigned char*)malloc((size_t)stride * AUDIT_H);
	if ( !px ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&d->tTarget, px, stride);
	if ( ret == XGE_OK ) ret = xgeImageSavePNG(d->sCapturePath, AUDIT_W, AUDIT_H, px, stride);
	free(px);
	if ( ret == XGE_OK ) { d->bCaptureDone = 1; printf("capture saved: %s\n", d->sCapturePath); }
	return ret;
}

/* Build a zigzag path for join testing */
static void build_zigzag(xge_shape_ex s, float x, float y)
{
	xgeShapeExMoveTo(s, x, y + 40);
	xgeShapeExLineTo(s, x + 30, y);
	xgeShapeExLineTo(s, x + 60, y + 40);
	xgeShapeExLineTo(s, x + 90, y);
	xgeShapeExLineTo(s, x + 120, y + 40);
}

/* Build an open line for cap testing */
static void build_open_line(xge_shape_ex s, float x, float y)
{
	xgeShapeExMoveTo(s, x, y);
	xgeShapeExLineTo(s, x + 120, y);
}

/* Section 1: Stroke joins */
static void draw_joins(float fBaseY)
{
	xge_shape_ex s;
	uint32_t col = XGE_COLOR_RGBA(255, 200, 50, 255);
	int joins[] = {XGE_SHAPE_EX_JOIN_MITER, XGE_SHAPE_EX_JOIN_ROUND, XGE_SHAPE_EX_JOIN_BEVEL};
	float offsets[] = {30.0f, 200.0f, 370.0f};
	int i;
	for ( i = 0; i < 3; i++ ) {
		xgeShapeExCreate(&s);
		build_zigzag(s, offsets[i], fBaseY);
		xgeShapeExStrokeColor(s, col);
		xgeShapeExStrokeWidth(s, 6.0f);
		xgeShapeExStrokeJoin(s, joins[i]);
		xgeShapeExDraw(s, 0.5f);
		xgeShapeExDestroy(s);
	}
	/* Acute angle miter test */
	xgeShapeExCreate(&s);
	xgeShapeExMoveTo(s, 560, fBaseY + 40);
	xgeShapeExLineTo(s, 620, fBaseY);
	xgeShapeExLineTo(s, 625, fBaseY + 40);
	xgeShapeExStrokeColor(s, XGE_COLOR_RGBA(255, 100, 100, 255));
	xgeShapeExStrokeWidth(s, 6.0f);
	xgeShapeExStrokeJoin(s, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExStrokeMiterLimit(s, 2.0f);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Section 2: Stroke caps */
static void draw_caps(float fBaseY)
{
	xge_shape_ex s;
	uint32_t col = XGE_COLOR_RGBA(100, 255, 180, 255);
	int caps[] = {XGE_SHAPE_EX_CAP_BUTT, XGE_SHAPE_EX_CAP_ROUND, XGE_SHAPE_EX_CAP_SQUARE};
	float ys[] = {0, 30, 60};
	int i;
	for ( i = 0; i < 3; i++ ) {
		xgeShapeExCreate(&s);
		build_open_line(s, 50.0f, fBaseY + ys[i]);
		xgeShapeExStrokeColor(s, col);
		xgeShapeExStrokeWidth(s, 10.0f);
		xgeShapeExStrokeCap(s, caps[i]);
		xgeShapeExDraw(s, 0.5f);
		xgeShapeExDestroy(s);
	}
}

/* Section 3: Dash patterns */
static void draw_dashes(float fBaseY)
{
	xge_shape_ex s;
	uint32_t col = XGE_COLOR_RGBA(200, 150, 255, 255);
	float dash1[] = {10, 5};
	float dash2[] = {20, 5, 5, 5};
	float dash3[] = {2, 4};
	float dash4[] = {15, 10, 5, 10};
	float* patterns[] = {dash1, dash2, dash3, dash4};
	int counts[] = {2, 4, 2, 4};
	float ys[] = {0, 30, 60, 90};
	int i;
	for ( i = 0; i < 4; i++ ) {
		xgeShapeExCreate(&s);
		xgeShapeExMoveTo(s, 30, fBaseY + ys[i]);
		xgeShapeExLineTo(s, 400, fBaseY + ys[i]);
		xgeShapeExStrokeColor(s, col);
		xgeShapeExStrokeWidth(s, 4.0f);
		xgeShapeExStrokeCap(s, XGE_SHAPE_EX_CAP_ROUND);
		xgeShapeExStrokeDash(s, patterns[i], counts[i], 0);
		xgeShapeExDraw(s, 0.5f);
		xgeShapeExDestroy(s);
	}
	/* Dash with offset */
	xgeShapeExCreate(&s);
	xgeShapeExMoveTo(s, 450, fBaseY + 0);
	xgeShapeExLineTo(s, 850, fBaseY + 0);
	xgeShapeExStrokeColor(s, XGE_COLOR_RGBA(255, 180, 80, 255));
	xgeShapeExStrokeWidth(s, 4.0f);
	xgeShapeExStrokeDash(s, dash1, 2, 7.5f);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Section 4: Stroke width variations */
static void draw_widths(float fBaseY)
{
	xge_shape_ex s;
	uint32_t col = XGE_COLOR_RGBA(80, 200, 255, 255);
	float widths[] = {0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 12.0f};
	int i;
	for ( i = 0; i < 6; i++ ) {
		xgeShapeExCreate(&s);
		xgeShapeExMoveTo(s, 30 + (float)i * 145, fBaseY);
		xgeShapeExLineTo(s, 30 + (float)i * 145 + 100, fBaseY + 60);
		xgeShapeExStrokeColor(s, col);
		xgeShapeExStrokeWidth(s, widths[i]);
		xgeShapeExStrokeCap(s, XGE_SHAPE_EX_CAP_ROUND);
		xgeShapeExDraw(s, 0.5f);
		xgeShapeExDestroy(s);
	}
}

/* Section 5: Stroke gradient */
static void draw_stroke_gradient(float fBaseY)
{
	xge_shape_ex s;
	xge_shape_ex_color_stop_t stops[3];
	stops[0].fOffset = 0.0f; stops[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	stops[1].fOffset = 0.5f; stops[1].iColor = XGE_COLOR_RGBA(0, 255, 0, 255);
	stops[2].fOffset = 1.0f; stops[2].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);

	/* Linear gradient stroke on path */
	xgeShapeExCreate(&s);
	xgeShapeExMoveTo(s, 30, fBaseY + 30);
	xgeShapeExQuadTo(s, 200, fBaseY - 20, 400, fBaseY + 30);
	xgeShapeExQuadTo(s, 600, fBaseY + 80, 800, fBaseY + 30);
	xgeShapeExStrokeLinearGradient(s, 30, 0, 800, 0, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3);
	xgeShapeExStrokeWidth(s, 6.0f);
	xgeShapeExStrokeCap(s, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

/* Section 6: Non-scaling stroke */
static void draw_non_scaling(float fBaseY)
{
	xge_shape_ex s;
	xge_shape_ex_matrix_t mat;
	uint32_t col = XGE_COLOR_RGBA(255, 255, 100, 255);

	/* Normal stroke with scale transform */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 60, 40, 0, 0, 1);
	xgeShapeExStrokeColor(s, col);
	xgeShapeExStrokeWidth(s, 2.0f);
	memset(&mat, 0, sizeof(mat));
	mat.fA = 2.0f; mat.fD = 2.0f; mat.fE = 50; mat.fF = fBaseY;
	xgeShapeExTransformSet(s, &mat);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);

	/* Non-scaling stroke with same transform */
	xgeShapeExCreate(&s);
	xgeShapeExAppendRect(s, 0, 0, 60, 40, 0, 0, 1);
	xgeShapeExStrokeColor(s, XGE_COLOR_RGBA(100, 255, 100, 255));
	xgeShapeExStrokeWidth(s, 2.0f);
	xgeShapeExStrokeNonScaling(s, 1);
	memset(&mat, 0, sizeof(mat));
	mat.fA = 2.0f; mat.fD = 2.0f; mat.fE = 300; mat.fF = fBaseY;
	xgeShapeExTransformSet(s, &mat);
	xgeShapeExDraw(s, 0.5f);
	xgeShapeExDestroy(s);
}

static void draw_marker(float x, float y)
{
	xgeShapeRectFill(rectf(x, y, 4, 4), XGE_COLOR_RGBA(255, 255, 255, 100));
}

static int frame(void* user)
{
	audit_demo_t* demo = (audit_demo_t*)user;
	xge_pass_t tPass;
	int ret;
	if ( !demo ) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	xgePassInit(&tPass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, BG_COLOR);
	ret = xgePassBegin(&tPass);
	if ( ret != XGE_OK ) return ret;

	draw_marker(10, 5); draw_joins(10);
	draw_marker(10, 75); draw_caps(80);
	draw_marker(10, 160); draw_dashes(165);
	draw_marker(10, 270); draw_widths(280);
	draw_marker(10, 360); draw_stroke_gradient(365);
	draw_marker(10, 430); draw_non_scaling(440);

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
	if ( ret != XGE_OK ) return 1;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = AUDIT_W; tDesc.iHeight = AUDIT_H;
	tDesc.sTitle = "audit_shape_ex_stroke";
	tDesc.iFlags = XGE_INIT_OFFSCREEN;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&tDesc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&tDemo.tTarget, AUDIT_W, AUDIT_H);
	if ( ret != XGE_OK ) { xgeUnit(); return 1; }
	ret = xgeRun(frame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
