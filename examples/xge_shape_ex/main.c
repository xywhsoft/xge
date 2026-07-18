#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 900
#define DEMO_H 1570

typedef struct xge_shape_ex_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} xge_shape_ex_demo_t;

static xge_rect_t rectf(float x, float y, float w, float h)
{
	xge_rect_t r;
	r.fX = x;
	r.fY = y;
	r.fW = w;
	r.fH = h;
	return r;
}

static void usage(void)
{
	printf("usage: xge_shape_ex [--frames N] [--capture PATH]\n");
}

static int parse_args(xge_shape_ex_demo_t* demo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			demo->iMaxFrames = atoi(argv[++i]);
			if ( demo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			demo->iMaxFrames = atoi(argv[i] + 9);
			if ( demo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[++i]);
			demo->sCapturePath[sizeof(demo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(demo->sCapturePath, sizeof(demo->sCapturePath), "%s", argv[i] + 10);
			demo->sCapturePath[sizeof(demo->sCapturePath) - 1] = '\0';
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			usage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int capture(xge_shape_ex_demo_t* demo)
{
	unsigned char* pixels;
	int stride;
	int ret;

	if ( (demo == NULL) || (demo->sCapturePath[0] == '\0') || demo->bCaptureDone ) {
		return XGE_OK;
	}
	stride = DEMO_W * 4;
	pixels = (unsigned char*)malloc((size_t)stride * DEMO_H);
	if ( pixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&demo->tTarget, pixels, stride);
	if ( ret == XGE_OK ) {
		ret = xgeImageSavePNG(demo->sCapturePath, DEMO_W, DEMO_H, pixels, stride);
	}
	free(pixels);
	if ( ret == XGE_OK ) {
		demo->bCaptureDone = 1;
		printf("xge_shape_ex capture saved: %s\n", demo->sCapturePath);
	}
	return ret;
}

static void draw_panel(float x, float y, float w, float h)
{
	xgeShapeRoundRectDrawPx(rectf(x, y, w, h), 8.0f, XGE_COLOR_RGBA(20, 25, 31, 255), 1.0f, XGE_COLOR_RGBA(66, 80, 94, 255));
}

static void draw_sample_marker(xge_vec2_t point, xge_vec2_t tangent)
{
	xge_shape_ex shape;

	xgeShapeExCreate(&shape);
	xgeShapeExAppendLine(shape, point.fX, point.fY, point.fX + tangent.fX * 18.0f, point.fY + tangent.fY * 18.0f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 49, 168, 255));
	xgeShapeExStrokeWidth(shape, 2.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendCircle(shape, point.fX, point.fY, 4.0f, 4.0f, 1);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(49, 255, 127, 255));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(13, 20, 26, 255));
	xgeShapeExStrokeWidth(shape, 1.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
}

static void draw_path_length_samples(xge_shape_ex shape)
{
	xge_shape_ex_path_measure measure;
	float length;
	float stops[3] = {0.25f, 0.5f, 0.75f};
	int i;

	measure = NULL;
	if ( xgeShapeExPathMeasureCreate(&measure, shape, NULL, 0.25f) != XGE_OK ) {
		return;
	}
	if ( xgeShapeExPathMeasureGetLength(measure, &length) != XGE_OK ) {
		xgeShapeExPathMeasureDestroy(measure);
		return;
	}
	for ( i = 0; i < 3; i++ ) {
		xge_vec2_t point;
		xge_vec2_t tangent;

		if ( xgeShapeExPathMeasureGetPointAtLength(measure, length * stops[i], &point, &tangent) == XGE_OK ) {
			draw_sample_marker(point, tangent);
		}
	}
	xgeShapeExPathMeasureDestroy(measure);
}

static void draw_shape_ex_paths(void)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[3];
	float dash[2] = {14.0f, 8.0f};
	float oddDash[3] = {20.0f, 7.0f, 4.0f};

	xgeShapeExCreate(&shape);
	xgeShapeExMoveTo(shape, 80.0f, 145.0f);
	xgeShapeExCubicTo(shape, 120.0f, 40.0f, 220.0f, 46.0f, 260.0f, 146.0f);
	xgeShapeExLineTo(shape, 214.0f, 118.0f);
	xgeShapeExCubicTo(shape, 188.0f, 70.0f, 132.0f, 70.0f, 106.0f, 118.0f);
	xgeShapeExClose(shape);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(84, 201, 152, 255));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(198, 244, 226, 255));
	xgeShapeExStrokeWidth(shape, 4.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	{
		const uint8_t rawCommands[6] = {
			XGE_SHAPE_EX_CMD_CLOSE,
			XGE_SHAPE_EX_CMD_MOVE_TO,
			XGE_SHAPE_EX_CMD_LINE_TO,
			XGE_SHAPE_EX_CMD_QUAD_TO,
			XGE_SHAPE_EX_CMD_CUBIC_TO,
			XGE_SHAPE_EX_CMD_CLOSE
		};
		const xge_vec2_t rawPoints[7] = {
			{292.0f, 92.0f},
			{332.0f, 58.0f},
			{360.0f, 40.0f},
			{386.0f, 92.0f},
			{402.0f, 132.0f},
			{322.0f, 164.0f},
			{292.0f, 92.0f}
		};

		xgeShapeExCreate(&shape);
		xgeShapeExAppendPath(shape, rawCommands, 6, rawPoints, 7);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(55, 214, 255, 210));
		xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(204, 247, 255, 255));
		xgeShapeExStrokeWidth(shape, 3.0f);
		xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		xgeShapeExDrawPx(shape, 0.25f);
		xgeShapeExDestroy(shape);
	}

	xgeShapeExCreate(&shape);
	{
		const xge_vec2_t polygon[5] = {
			{414.0f, 68.0f},
			{440.0f, 108.0f},
			{424.0f, 150.0f},
			{394.0f, 150.0f},
			{380.0f, 108.0f}
		};

		xgeShapeExAppendPolygon(shape, polygon, 5);
	}
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(237, 91, 126, 205));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 214, 224, 255));
	xgeShapeExStrokeWidth(shape, 3.0f);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 76 280 C 128 224 202 224 254 280 S 380 336 432 280");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(110, 202, 246, 255);
	stops[1].fOffset = 0.55f;
	stops[1].iColor = XGE_COLOR_RGBA(169, 140, 255, 255);
	stops[2].fOffset = 1.0f;
	stops[2].iColor = XGE_COLOR_RGBA(240, 180, 91, 255);
	xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 3);
	xgeShapeExStrokeWidth(shape, 8.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExStrokeDash(shape, dash, 2, 0.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	draw_path_length_samples(shape);
	{
		char svgPathData[512];
		int requiredSize = 0;
		xge_shape_ex exported = NULL;

		if ( (xgeShapeExGetSvgPathData(shape, svgPathData, (int)sizeof(svgPathData), &requiredSize) == XGE_OK) &&
		     (xgeShapeExCreate(&exported) == XGE_OK) ) {
			xgeShapeExAppendSvgPath(exported, svgPathData);
			xgeShapeExFillColor(exported, XGE_COLOR_RGBA(0, 0, 0, 0));
			xgeShapeExStrokeColor(exported, XGE_COLOR_RGBA(123, 224, 177, 210));
			xgeShapeExStrokeWidth(exported, 3.0f);
			xgeShapeExStrokeCap(exported, XGE_SHAPE_EX_CAP_ROUND);
			xgeShapeExStrokeJoin(exported, XGE_SHAPE_EX_JOIN_ROUND);
			xgeShapeExTransformTranslate(exported, 0.0f, 30.0f);
			xgeShapeExDrawPx(exported, 0.25f);
		}
		xgeShapeExDestroy(exported);
	}
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 76 324 C 128 268 202 268 254 324 S 380 380 432 324");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(52, 61, 70, 255));
	xgeShapeExStrokeWidth(shape, 6.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExTrimPath(shape, 0.18f, 0.72f, 0);
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 127, 49, 255));
	xgeShapeExStrokeWidth(shape, 8.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	{
		const xge_vec2_t polyline[2] = {
			{76.0f, 382.0f},
			{432.0f, 382.0f}
		};

		xgeShapeExAppendPolyline(shape, polyline, 2);
	}
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(90, 204, 138, 255));
	xgeShapeExStrokeWidth(shape, 5.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_BUTT);
	xgeShapeExStrokeDash(shape, oddDash, 3, 0.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 76 418 T 148 348 220 418 292 348 364 418");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(254, 220, 186, 255));
	xgeShapeExStrokeWidth(shape, 6.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExMoveTo(shape, 300.0f, 452.0f);
	xgeShapeExArcTo(shape, 52.0f, 34.0f, -18.0f, 0, 1, 414.0f, 452.0f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(120, 121, 122, 255));
	xgeShapeExStrokeWidth(shape, 7.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 394 422 A 0 24 0 0 1 454 422");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 204, 95, 255));
	xgeShapeExStrokeWidth(shape, 5.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendArc(shape, 106.0f, 462.0f, 36.0f, 20.0f, 3.14159265f, 6.28318531f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(123, 124, 125, 255));
	xgeShapeExStrokeWidth(shape, 5.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendPie(shape, 214.0f, 462.0f, 34.0f, 22.0f, 0.10f, 2.45f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(126, 127, 128, 255));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(210, 211, 212, 255));
	xgeShapeExStrokeWidth(shape, 2.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendChord(shape, 322.0f, 462.0f, 34.0f, 22.0f, -2.60f, -0.45f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(129, 130, 131, 255));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(224, 225, 226, 255));
	xgeShapeExStrokeWidth(shape, 2.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendTriangle(shape, 400.0f, 484.0f, 430.0f, 438.0f, 460.0f, 484.0f, 1);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(236, 86, 124, 235));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 196, 210, 255));
	xgeShapeExStrokeWidth(shape, 3.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendCapsule(shape, 0.0f, 0.0f, 78.0f, 44.0f, 1);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(90, 204, 138, 235));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(200, 255, 220, 255));
	xgeShapeExStrokeWidth(shape, 3.0f);
	{
		xge_shape_ex_matrix_t translate;
		xge_shape_ex_matrix_t rotate;
		xge_shape_ex_matrix_t transform;
		xge_rect_t transformedBounds;
		xge_vec2_t guideStart;
		xge_vec2_t guideVector;
		float guideWidth;

		xgeShapeExMatrixTranslate(&translate, 478.0f, 440.0f);
		xgeShapeExMatrixRotate(&rotate, -0.08f);
		xgeShapeExMatrixMultiply(&transform, &translate, &rotate);
		if ( xgeShapeExMatrixRectBounds(&transformedBounds, &transform, rectf(0.0f, 0.0f, 78.0f, 44.0f)) == XGE_OK ) {
			xgeShapeRoundRectStrokePx(transformedBounds, 0.0f, 1.0f, XGE_COLOR_RGBA(118, 140, 160, 170));
		}
		if ( (xgeShapeExMatrixPoint(&guideStart, &transform, (xge_vec2_t){4.0f, 52.0f}) == XGE_OK) &&
		     (xgeShapeExMatrixVector(&guideVector, &transform, (xge_vec2_t){40.0f, 0.0f}) == XGE_OK) ) {
			guideWidth = 2.0f;
			if ( xgeShapeExMatrixStrokeScale(&guideWidth, &transform) == XGE_OK ) {
				guideWidth *= 2.0f;
			}
			xgeShapeLinePx(guideStart.fX, guideStart.fY, guideStart.fX + guideVector.fX, guideStart.fY + guideVector.fY, guideWidth, XGE_COLOR_RGBA(200, 255, 220, 210));
		}
		xgeShapeExDrawPxEx(shape, 0.25f, &transform, 1.0f);
	}
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExMoveTo(shape, 430.0f, 450.0f);
	xgeShapeExLineTo(shape, 430.0f, 450.0f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(121, 122, 123, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExMoveTo(shape, 430.0f, 474.0f);
	xgeShapeExLineTo(shape, 430.0f, 474.0f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(124, 125, 126, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_SQUARE);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExMoveTo(shape, 454.0f, 462.0f);
	xgeShapeExLineTo(shape, 454.0f, 462.0f);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(127, 128, 129, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeDash(shape, dash, 2, 0.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendCircle(shape, 350.0f, 116.0f, 48.0f, 48.0f, 1);
	xgeShapeExAppendCircle(shape, 350.0f, 116.0f, 22.0f, 22.0f, 1);
	xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD);
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(240, 180, 91, 255));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 234, 188, 255));
	xgeShapeExStrokeWidth(shape, 3.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
}

static void draw_shape_ex_scene(void)
{
	xge_shape_ex_scene scene;
	xge_shape_ex shape;
	xge_shape_ex clip;
	xge_shape_ex clip2;
	xge_shape_ex_matrix_t m;
	xge_shape_ex_color_stop_t stops[3];

	clip = NULL;
	clip2 = NULL;
	xgeShapeExSceneCreate(&scene);
	xgeShapeExCreate(&shape);
	xgeShapeExAppendRect(shape, 0.0f, 0.0f, 170.0f, 96.0f, 18.0f, 18.0f, 1);
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(234, 93, 128, 255);
	stops[1].fOffset = 0.55f;
	stops[1].iColor = XGE_COLOR_RGBA(255, 204, 95, 255);
	stops[2].fOffset = 1.0f;
	stops[2].iColor = XGE_COLOR_RGBA(105, 218, 198, 255);
	xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 3);
	m.fA = 0.72f;
	m.fB = 0.0f;
	m.fC = 0.0f;
	m.fD = 1.0f;
	m.fE = 0.14f;
	m.fF = 0.0f;
	xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT);
	xgeShapeExFillGradientTransformSet(shape, &m);
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(255, 214, 224, 255));
	xgeShapeExStrokeWidth(shape, 5.0f);
	xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendEllipse(shape, 86.0f, 48.0f, 38.0f, 28.0f, 1);
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(255, 245, 185, 255);
	stops[1].fOffset = 0.55f;
	stops[1].iColor = XGE_COLOR_RGBA(255, 204, 95, 235);
	stops[2].fOffset = 1.0f;
	stops[2].iColor = XGE_COLOR_RGBA(237, 91, 126, 215);
	xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.55f, 0.38f, 0.34f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 3);
	xgeShapeExBlend(shape, XGE_BLEND_SCREEN);
	xgeShapeExSceneAdd(scene, shape);
	xgeShapeExDestroy(shape);

	m.fA = 1.0f;
	m.fB = 0.0f;
	m.fC = 0.0f;
	m.fD = 1.0f;
	m.fE = 540.0f;
	m.fF = 74.0f;
	xgeShapeExSceneTransformSet(scene, &m);
	xgeShapeExSceneClipRectSet(scene, rectf(-8.0f, -6.0f, 158.0f, 112.0f));
	if ( (xgeShapeExCreate(&clip) == XGE_OK) &&
	     (xgeShapeExAppendRect(clip, 48.0f, 14.0f, 78.0f, 68.0f, 8.0f, 8.0f, 1) == XGE_OK) &&
	     (xgeShapeExCreate(&clip2) == XGE_OK) &&
	     (xgeShapeExAppendEllipse(clip2, 86.0f, 48.0f, 22.0f, 18.0f, 1) == XGE_OK) &&
	     (xgeShapeExClipShapeAddEx(clip, clip2, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK) ) {
		xgeShapeExSceneClipShapeAddEx(scene, clip, XGE_SHAPE_EX_CLIP_SUBTRACT);
	}
	xgeShapeExDestroy(clip2);
	xgeShapeExDestroy(clip);
	xgeShapeExSceneDrawPx(scene, 0.25f);
	xgeShapeExSceneDestroy(scene);
}

static void draw_shape_ex_stroke_joins(void)
{
	xge_shape_ex shape;
	xge_shape_ex_color_stop_t stops[3];

	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	stops[1].fOffset = 0.45f;
	stops[1].iColor = XGE_COLOR_RGBA(96, 210, 242, 255);
	stops[2].fOffset = 1.0f;
	stops[2].iColor = XGE_COLOR_RGBA(236, 91, 126, 255);
	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 520 88 C 570 52 625 52 670 88 S 760 124 804 88");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeRadialGradientEx(shape, 0.5f, 0.45f, 0.72f, 0.22f, 0.28f, 0.14f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 3);
	xgeShapeExStrokeWidth(shape, 16.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 520 185 L 560 130 L 600 185");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(44, 45, 46, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_BUTT);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_MITER);
	xgeShapeExStrokeMiterLimit(shape, 8.0f);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 622 185 L 662 130 L 702 185");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(47, 48, 49, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_BUTT);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);

	xgeShapeExCreate(&shape);
	xgeShapeExAppendSvgPath(shape, "M 724 185 L 764 130 L 804 185");
	xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(50, 51, 52, 255));
	xgeShapeExStrokeWidth(shape, 14.0f);
	xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_BUTT);
	xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_BEVEL);
	xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
}

static void draw_shape_ex_viewport_reverse_stroke(void)
{
	xge_shape_ex shape = NULL;

	xgeViewportSet(rectf(820.0f, 0.0f, 80.0f, (float)DEMO_H));
	if ( (xgeShapeExCreate(&shape) == XGE_OK) &&
	     (xgeShapeExMoveTo(shape, 100.0f, 20.0f) == XGE_OK) &&
	     (xgeShapeExLineTo(shape, 800.0f, 20.0f) == XGE_OK) &&
	     (xgeShapeExLineTo(shape, 100.0f, 20.0f) == XGE_OK) ) {
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(0, 0, 0, 0));
		xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(96, 210, 242, 255));
		xgeShapeExStrokeWidth(shape, 8.0f);
		xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_BUTT);
		xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
		xgeShapeExDrawPx(shape, 0.25f);
	}
	xgeShapeExDestroy(shape);
	xgeViewportClear();
}

static xge_shape_ex make_clip_rect(float x, float y, float w, float h)
{
	xge_shape_ex shape = NULL;

	if ( xgeShapeExCreate(&shape) != XGE_OK ) return NULL;
	if ( xgeShapeExAppendRect(shape, x, y, w, h, 0.0f, 0.0f, 1) != XGE_OK ) {
		xgeShapeExDestroy(shape);
		return NULL;
	}
	return shape;
}

static xge_shape_ex make_clip_ellipse(float cx, float cy, float rx, float ry)
{
	xge_shape_ex shape = NULL;

	if ( xgeShapeExCreate(&shape) != XGE_OK ) return NULL;
	if ( xgeShapeExAppendEllipse(shape, cx, cy, rx, ry, 1) != XGE_OK ) {
		xgeShapeExDestroy(shape);
		return NULL;
	}
	return shape;
}

static xge_shape_ex make_target(float x, float y, float w, float h, uint32_t color)
{
	xge_shape_ex shape = make_clip_rect(x, y, w, h);

	if ( shape != NULL ) xgeShapeExFillColor(shape, color);
	return shape;
}

static void draw_shape_ex_stencil_cases(void)
{
	static const uint32_t colors[10] = {
		XGE_COLOR_RGBA(96, 210, 242, 255), XGE_COLOR_RGBA(239, 91, 126, 255),
		XGE_COLOR_RGBA(95, 208, 151, 255), XGE_COLOR_RGBA(240, 180, 91, 255),
		XGE_COLOR_RGBA(169, 140, 255, 255), XGE_COLOR_RGBA(255, 126, 95, 255),
		XGE_COLOR_RGBA(105, 218, 198, 255), XGE_COLOR_RGBA(231, 117, 162, 255),
		XGE_COLOR_RGBA(125, 177, 255, 255), XGE_COLOR_RGBA(255, 205, 102, 255)
	};
	int index;

	for ( index = 0; index < 10; index++ ) {
		float x = 48.0f + (float)(index % 5) * 166.0f;
		float y = 548.0f + (float)(index / 5) * 104.0f;
		xge_shape_ex target = make_target(x, y, 132.0f, 76.0f, colors[index]);
		xge_shape_ex a = NULL;
		xge_shape_ex b = NULL;
		xge_shape_ex c = NULL;

		if ( target == NULL ) continue;
		if ( index == 0 ) {
			a = make_clip_rect(x + 6.0f, y + 8.0f, 48.0f, 60.0f);
			b = make_clip_rect(x + 78.0f, y + 8.0f, 48.0f, 60.0f);
			if ( a != NULL ) xgeShapeExClipShapeAdd(target, a);
			if ( b != NULL ) xgeShapeExClipShapeAdd(target, b);
		} else if ( index == 1 ) {
			a = make_clip_rect(x + 8.0f, y + 8.0f, 116.0f, 60.0f);
			b = make_clip_ellipse(x + 66.0f, y + 38.0f, 22.0f, 24.0f);
			if ( a != NULL ) xgeShapeExClipShapeAdd(target, a);
			if ( b != NULL ) xgeShapeExClipShapeAddEx(target, b, XGE_SHAPE_EX_CLIP_SUBTRACT);
		} else if ( index == 2 ) {
			a = make_clip_rect(x + 8.0f, y + 8.0f, 116.0f, 60.0f);
			b = make_clip_ellipse(x + 66.0f, y + 38.0f, 42.0f, 24.0f);
			if ( (a != NULL) && (b != NULL) ) xgeShapeExClipShapeAdd(a, b);
			if ( a != NULL ) xgeShapeExClipShapeAdd(target, a);
		} else if ( index == 3 ) {
			a = make_clip_rect(x + 18.0f, y + 8.0f, 96.0f, 60.0f);
			b = make_clip_ellipse(x + 66.0f, y + 38.0f, 34.0f, 24.0f);
			if ( (a != NULL) && (b != NULL) ) xgeShapeExClipShapeAdd(a, b);
			if ( a != NULL ) xgeShapeExClipShapeAddEx(target, a, XGE_SHAPE_EX_CLIP_SUBTRACT);
		} else if ( index == 4 ) {
			a = make_clip_rect(x + 18.0f, y + 8.0f, 96.0f, 60.0f);
			b = make_clip_ellipse(x + 66.0f, y + 38.0f, 28.0f, 22.0f);
			if ( (a != NULL) && (b != NULL) ) xgeShapeExClipShapeAddEx(a, b, XGE_SHAPE_EX_CLIP_SUBTRACT);
			if ( a != NULL ) xgeShapeExClipShapeAddEx(target, a, XGE_SHAPE_EX_CLIP_SUBTRACT);
		} else if ( index == 5 ) {
			a = make_clip_rect(x + 8.0f, y + 8.0f, 72.0f, 60.0f);
			b = make_clip_rect(x + 52.0f, y + 8.0f, 72.0f, 60.0f);
			c = make_clip_ellipse(x + 74.0f, y + 38.0f, 18.0f, 24.0f);
			if ( (b != NULL) && (c != NULL) ) xgeShapeExClipShapeAddEx(b, c, XGE_SHAPE_EX_CLIP_SUBTRACT);
			if ( a != NULL ) xgeShapeExClipShapeAdd(target, a);
			if ( b != NULL ) xgeShapeExClipShapeAdd(target, b);
		} else if ( index == 6 ) {
			xge_shape_ex_matrix_t m;
			a = make_clip_rect(0.0f, 0.0f, 82.0f, 52.0f);
			b = make_clip_ellipse(41.0f, 26.0f, 28.0f, 20.0f);
			if ( (a != NULL) && (b != NULL) ) xgeShapeExClipShapeAdd(a, b);
			if ( a != NULL ) {
				xgeShapeExMatrixTranslate(&m, x + 24.0f, y + 12.0f);
				xgeShapeExTransformSet(a, &m);
				xgeShapeExClipShapeAdd(target, a);
			}
		} else if ( index == 7 ) {
			a = make_clip_ellipse(x + 66.0f, y + 38.0f, 52.0f, 30.0f);
			if ( a != NULL ) {
				xgeShapeExClipRectSet(a, rectf(x + 22.0f, y + 12.0f, 44.0f, 52.0f));
				xgeShapeExClipShapeAdd(target, a);
			}
		} else if ( index == 8 ) {
			xge_shape_ex_scene scene = NULL;
			a = make_clip_rect(x + 16.0f, y + 8.0f, 100.0f, 60.0f);
			b = make_clip_ellipse(x + 66.0f, y + 38.0f, 34.0f, 26.0f);
			if ( b != NULL ) xgeShapeExClipShapeAdd(target, b);
			if ( xgeShapeExSceneCreate(&scene) == XGE_OK ) {
				xgeShapeExSceneAdd(scene, target);
				if ( a != NULL ) xgeShapeExSceneClipShapeAdd(scene, a);
				xgeShapeExSceneDrawPx(scene, 0.25f);
				xgeShapeExSceneDestroy(scene);
			}
		} else {
			int applied = 0;
			a = make_clip_ellipse(x + 66.0f, y + 38.0f, 54.0f, 30.0f);
			b = make_clip_rect(x + 58.0f, y + 4.0f, 16.0f, 68.0f);
			c = make_clip_rect(x + 20.0f, y + 12.0f, 92.0f, 52.0f);
			if ( c != NULL ) xgeShapeExClipShapeAdd(target, c);
			if ( b != NULL ) xgeShapeExClipShapeAddEx(target, b, XGE_SHAPE_EX_CLIP_SUBTRACT);
			if ( (a != NULL) && (xgeShapeExStencilClipBeginPx(a, 0.25f, NULL, &applied) == XGE_OK) ) {
				int drawRet = xgeShapeExDrawPx(target, 0.25f);
				xgeShapeExStencilClipEnd(applied, drawRet);
			}
		}
		if ( index < 8 ) xgeShapeExDrawPx(target, 0.25f);
		xgeShapeExDestroy(c);
		xgeShapeExDestroy(b);
		xgeShapeExDestroy(a);
		xgeShapeExDestroy(target);
	}
}

static void draw_shape_ex_mask_cases(void)
{
	static const int methods[10] = {
		XGE_SHAPE_EX_MASK_ALPHA,
		XGE_SHAPE_EX_MASK_INV_ALPHA,
		XGE_SHAPE_EX_MASK_LUMA,
		XGE_SHAPE_EX_MASK_INV_LUMA,
		XGE_SHAPE_EX_MASK_ADD,
		XGE_SHAPE_EX_MASK_SUBTRACT,
		XGE_SHAPE_EX_MASK_INTERSECT,
		XGE_SHAPE_EX_MASK_DIFFERENCE,
		XGE_SHAPE_EX_MASK_LIGHTEN,
		XGE_SHAPE_EX_MASK_DARKEN
	};
	int index;

	for ( index = 0; index < 10; index++ ) {
		float x = 48.0f + (float)(index % 5) * 166.0f;
		float y = 796.0f + (float)(index / 5) * 104.0f;
		xge_shape_ex source = NULL;
		xge_shape_ex mask = NULL;
		xge_shape_ex_scene sourceScene = NULL;
		xge_shape_ex_scene maskScene = NULL;

		if ( xgeShapeExCreate(&source) != XGE_OK ) continue;
		if ( xgeShapeExCreate(&mask) != XGE_OK ) {
			xgeShapeExDestroy(source);
			continue;
		}
		xgeShapeExAppendEllipse(source, x + 50.0f, y + 38.0f, 34.0f, 27.0f, 1);
		xgeShapeExFillColor(source, XGE_COLOR_RGBA(96, 210, 242, 255));
		xgeShapeExAppendEllipse(mask, x + 86.0f, y + 38.0f, 34.0f, 27.0f, 1);
		xgeShapeExFillColor(mask, XGE_COLOR_RGBA(240, 180, 91, 255));
		if ( index == 7 ) {
			if ( (xgeShapeExSceneCreate(&maskScene) == XGE_OK) &&
			     (xgeShapeExSceneAdd(maskScene, mask) == XGE_OK) &&
			     (xgeShapeExMaskSceneSet(source, maskScene, methods[index]) == XGE_OK) ) {
				xgeShapeExDrawPx(source, 0.25f);
			}
		} else if ( index >= 8 ) {
			if ( (xgeShapeExSceneCreate(&sourceScene) == XGE_OK) &&
			     (xgeShapeExSceneAdd(sourceScene, source) == XGE_OK) ) {
				if ( index == 8 ) {
					if ( (xgeShapeExSceneCreate(&maskScene) == XGE_OK) &&
					     (xgeShapeExSceneAdd(maskScene, mask) == XGE_OK) &&
					     (xgeShapeExSceneMaskSceneSet(sourceScene, maskScene, methods[index]) == XGE_OK) ) {
						xgeShapeExSceneDrawPx(sourceScene, 0.25f);
					}
				} else if ( xgeShapeExSceneMaskShapeSet(sourceScene, mask, methods[index]) == XGE_OK ) {
					xge_shape_ex clip = make_clip_rect(x + 8.0f, y + 4.0f, 136.0f, 68.0f);
					int applied = 0;

					if ( (clip != NULL) && (xgeShapeExStencilClipBeginPx(clip, 0.25f, NULL, &applied) == XGE_OK) ) {
						int drawRet = xgeShapeExSceneDrawPx(sourceScene, 0.25f);
						xgeShapeExStencilClipEnd(applied, drawRet);
					}
					xgeShapeExDestroy(clip);
				}
			}
		} else if ( xgeShapeExMaskShapeSet(source, mask, methods[index]) == XGE_OK ) {
			xgeShapeExDrawPx(source, 0.25f);
		}
		xgeShapeExSceneDestroy(maskScene);
		xgeShapeExSceneDestroy(sourceScene);
		xgeShapeExDestroy(mask);
		xgeShapeExDestroy(source);
	}
}

static void draw_shape_ex_scene_effects(void)
{
	int i;

	for ( i = 0; i < 5; i++ ) {
		xge_shape_ex_scene scene = NULL;
		xge_shape_ex shape = NULL;
		xge_shape_ex_matrix_t transform;

		if ( xgeShapeExSceneCreate(&scene) != XGE_OK ) continue;
		if ( xgeShapeExCreate(&shape) == XGE_OK ) {
			xgeShapeExAppendRect(shape, 8.0f, 18.0f, 112.0f, 62.0f, 18.0f, 18.0f, 1);
			xgeShapeExFillColor(shape, XGE_COLOR_RGBA(74, 177, 230, 255));
			xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(190, 231, 252, 255));
			xgeShapeExStrokeWidth(shape, 3.0f);
			xgeShapeExSceneAdd(scene, shape);
			xgeShapeExDestroy(shape);
		}
		shape = NULL;
		if ( xgeShapeExCreate(&shape) == XGE_OK ) {
			xgeShapeExAppendCircle(shape, 94.0f, 36.0f, 18.0f, 18.0f, 1);
			xgeShapeExFillColor(shape, XGE_COLOR_RGBA(244, 187, 78, 230));
			xgeShapeExSceneAdd(scene, shape);
			xgeShapeExDestroy(shape);
		}
		if ( i == 0 ) {
			xgeShapeExSceneEffectGaussianBlur(scene, 2.2f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 100);
		} else if ( i == 1 ) {
			xgeShapeExSceneEffectDropShadow(scene, XGE_COLOR_RGBA(0, 0, 0, 190), 135.0f, 12.0f, 3.0f, 100);
		} else if ( i == 2 ) {
			xgeShapeExSceneEffectFill(scene, XGE_COLOR_RGBA(234, 83, 126, 220));
		} else if ( i == 3 ) {
			xgeShapeExSceneEffectTint(scene, XGE_COLOR_RGBA(20, 32, 66, 255), XGE_COLOR_RGBA(130, 246, 208, 255), 88.0f);
		} else {
			xgeShapeExSceneEffectTritone(
				scene,
				XGE_COLOR_RGBA(22, 27, 48, 255),
				XGE_COLOR_RGBA(148, 80, 190, 255),
				XGE_COLOR_RGBA(255, 222, 112, 255),
				24
			);
		}
		xgeShapeExMatrixIdentity(&transform);
		transform.fE = 52.0f + (float)i * 160.0f;
		transform.fF = 1060.0f;
		xgeShapeExSceneTransformSet(scene, &transform);
		{
			int drawRet = xgeShapeExSceneDrawPx(scene, 0.25f);
			if ( drawRet != XGE_OK ) printf("xge_shape_ex scene effect %d draw failed: %d\n", i, drawRet);
		}
		xgeShapeExSceneDestroy(scene);
	}
}

static void draw_shape_ex_nested_scenes(void)
{
	xge_shape_ex_scene root = NULL;
	xge_shape_ex_scene left = NULL;
	xge_shape_ex_scene right = NULL;
	xge_shape_ex_scene inner = NULL;
	xge_shape_ex shape = NULL;
	xge_shape_ex_matrix_t transform;
	int drawRet;

	if ( (xgeShapeExSceneCreate(&root) != XGE_OK) ||
	     (xgeShapeExSceneCreate(&left) != XGE_OK) ||
	     (xgeShapeExSceneCreate(&right) != XGE_OK) ||
	     (xgeShapeExSceneCreate(&inner) != XGE_OK) ) goto cleanup;

	if ( xgeShapeExCreate(&shape) == XGE_OK ) {
		xgeShapeExAppendRect(shape, 0.0f, 0.0f, 210.0f, 110.0f, 14.0f, 14.0f, 1);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(50, 180, 140, 255));
		xgeShapeExSceneAdd(left, shape);
		xgeShapeExDestroy(shape);
		shape = NULL;
	}
	if ( xgeShapeExCreate(&shape) == XGE_OK ) {
		xgeShapeExAppendCircle(shape, 150.0f, 55.0f, 30.0f, 30.0f, 1);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(244, 176, 68, 255));
		xgeShapeExSceneAdd(left, shape);
		xgeShapeExDestroy(shape);
		shape = NULL;
	}

	if ( xgeShapeExCreate(&shape) == XGE_OK ) {
		xgeShapeExAppendRect(shape, 0.0f, 0.0f, 210.0f, 110.0f, 14.0f, 14.0f, 1);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(70, 126, 222, 255));
		xgeShapeExSceneAdd(right, shape);
		xgeShapeExDestroy(shape);
		shape = NULL;
	}
	if ( xgeShapeExCreate(&shape) == XGE_OK ) {
		xgeShapeExAppendRect(shape, 0.0f, 0.0f, 100.0f, 50.0f, 8.0f, 8.0f, 1);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeShapeExSceneAdd(inner, shape);
		xgeShapeExDestroy(shape);
		shape = NULL;
	}
	xgeShapeExSceneEffectFill(inner, XGE_COLOR_RGBA(250, 213, 92, 255));
	xgeShapeExMatrixIdentity(&transform);
	transform.fE = 55.0f;
	transform.fF = 30.0f;
	xgeShapeExSceneTransformSet(inner, &transform);
	xgeShapeExSceneAddScene(right, inner);

	xgeShapeExSceneAddScene(root, left);
	xgeShapeExMatrixIdentity(&transform);
	transform.fE = 300.0f;
	xgeShapeExSceneTransformSet(right, &transform);
	xgeShapeExSceneAddScene(root, right);
	if ( xgeShapeExCreate(&shape) == XGE_OK ) {
		xgeShapeExAppendRect(shape, 580.0f, 10.0f, 160.0f, 90.0f, 20.0f, 20.0f, 1);
		xgeShapeExFillColor(shape, XGE_COLOR_RGBA(226, 85, 132, 255));
		xgeShapeExSceneAdd(root, shape);
		xgeShapeExDestroy(shape);
		shape = NULL;
	}
	xgeShapeExMatrixIdentity(&transform);
	transform.fE = 70.0f;
	transform.fF = 1252.0f;
	xgeShapeExSceneTransformSet(root, &transform);
	drawRet = xgeShapeExSceneDrawPx(root, 0.25f);
	if ( drawRet != XGE_OK ) printf("xge_shape_ex nested scene draw failed: %d\n", drawRet);

cleanup:
	xgeShapeExDestroy(shape);
	xgeShapeExSceneDestroy(root);
	xgeShapeExSceneDestroy(left);
	xgeShapeExSceneDestroy(right);
	xgeShapeExSceneDestroy(inner);
}

static void draw_svg(void)
{
	static const char svg_text[] =
		"<svg viewBox=\"0 0 240 150\" preserveAspectRatio=\"xMidYMid meet\">"
		"<defs>"
		"<linearGradient id=\"panelGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.12 0) scale(0.75 1)\">"
		"<stop offset=\"0%\" stop-color=\"#5fd097\"/>"
		"<stop offset=\"50%\" stop-color=\"#f0b45b\"/>"
		"<stop offset=\"100%\" stop-color=\"#a98cff\"/>"
		"</linearGradient>"
		"<radialGradient id=\"markerGlow\" cx=\"50%\" cy=\"45%\" r=\"55%\" fx=\"35%\" fy=\"30%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.94 0.94) skewX(3)\">"
		"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
		"<stop offset=\"65%\" stop-color=\"#f0b45b\"/>"
		"<stop offset=\"100%\" stop-color=\"#ef5b7e\"/>"
		"</radialGradient>"
		"<g id=\"marker\" fill=\"#5fd097\" stroke=\"#d5fff0\" stroke-width=\"3\" paint-order=\"stroke fill\">"
		"<circle cx=\"0\" cy=\"0\" r=\"20\"/>"
		"<path d=\"M-16 0 L0 -24 L16 0 Z\" fill=\"#f0b45b\"/>"
		"</g>"
		"</defs>"
		"<rect x=\"10\" y=\"10\" width=\"220\" height=\"130\" rx=\"20\" fill=\"#27313d\" stroke=\"#85a4c2\" stroke-width=\"4\"/>"
		"<rect x=\"28\" y=\"24\" width=\"184\" height=\"18\" rx=\"4\" fill=\"url(#panelGrad)\"/>"
		"<circle cx=\"190\" cy=\"54\" r=\"24\" fill=\"url(#markerGlow)\" stroke=\"none\"/>"
		"<path d=\"M36 32 H92 V88 H36 Z M52 48 H76 V72 H52 Z\" fill=\"#5fd097\" fill-rule=\"evenodd\"/>"
		"<use href=\"#marker\" x=\"86\" y=\"76\"/>"
		"<use xlink:href=\"#marker\" x=\"168\" y=\"78\" transform=\"scale(1.25)\" opacity=\"0.85\"/>"
		"<path d=\"M45 108 C64 36 118 36 136 108 S204 180 218 78\" fill=\"none\" stroke=\"#a98cff\" stroke-width=\"10\" stroke-linecap=\"round\" stroke-dasharray=\"18 8\"/>"
		"</svg>";
	xge_svg svg;

	if ( xgeSvgCreate(&svg) == XGE_OK ) {
		if ( xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text)) == XGE_OK ) {
			xgeSvgDrawPx(svg, rectf(520.0f, 286.0f, 300.0f, 190.0f), 0.25f);
		}
		xgeSvgDestroy(svg);
	}
}

static void draw_shape_ex_scanline_aa(void)
{
	static const float coordinates[3][8] = {
		{60.0f, 1432.0f, 342.0f, 1512.0f, 60.0f, 1512.0f, 342.0f, 1432.0f},
		{420.0f, 1442.0f, 548.0f, 1502.0f, 420.0f, 1502.0f, 548.0f, 1442.0f},
		{620.0f, 1436.0f, 812.0f, 1508.0f, 620.0f, 1508.0f, 812.0f, 1436.0f}
	};
	static const uint32_t colors[3] = {
		XGE_COLOR_RGBA(251, 113, 133, 255),
		XGE_COLOR_RGBA(96, 210, 242, 255),
		XGE_COLOR_RGBA(169, 140, 255, 210)
	};
	int i;

	for ( i = 0; i < 3; i++ ) {
		xge_shape_ex shape;

		if ( xgeShapeExCreate(&shape) != XGE_OK ) continue;
		xgeShapeExMoveTo(shape, coordinates[i][0], coordinates[i][1]);
		xgeShapeExLineTo(shape, coordinates[i][2], coordinates[i][3]);
		xgeShapeExLineTo(shape, coordinates[i][4], coordinates[i][5]);
		xgeShapeExLineTo(shape, coordinates[i][6], coordinates[i][7]);
		xgeShapeExClose(shape);
		xgeShapeExFillColor(shape, colors[i]);
		xgeShapeExDrawPx(shape, 0.25f);
		xgeShapeExDestroy(shape);
	}
}

static void draw_all(void)
{
	xgeClear(XGE_COLOR_RGBA(13, 17, 22, 255));
	draw_panel(36.0f, 34.0f, 420.0f, 170.0f);
	draw_panel(492.0f, 34.0f, 330.0f, 170.0f);
	draw_panel(36.0f, 236.0f, 420.0f, 250.0f);
	draw_panel(492.0f, 236.0f, 330.0f, 250.0f);
	draw_shape_ex_paths();
	draw_shape_ex_scene();
	draw_shape_ex_stroke_joins();
	draw_shape_ex_viewport_reverse_stroke();
	draw_svg();
	draw_panel(36.0f, 516.0f, 828.0f, 230.0f);
	draw_shape_ex_stencil_cases();
	draw_panel(36.0f, 764.0f, 828.0f, 230.0f);
	draw_shape_ex_mask_cases();
	draw_panel(36.0f, 1012.0f, 828.0f, 200.0f);
	draw_shape_ex_scene_effects();
	draw_panel(36.0f, 1228.0f, 828.0f, 160.0f);
	draw_shape_ex_nested_scenes();
	draw_panel(36.0f, 1404.0f, 828.0f, 132.0f);
	draw_shape_ex_scanline_aa();
}

static int frame(void* user)
{
	xge_shape_ex_demo_t* demo = (xge_shape_ex_demo_t*)user;
	xge_pass_t pass;
	xge_draw_t draw;
	xge_texture texture;
	int ret;

	if ( demo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	xgePassInit(&pass, &demo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(13, 17, 22, 255));
	ret = xgePassBegin(&pass);
	if ( ret != XGE_OK ) return ret;
	draw_all();
	ret = xgePassEnd(&pass);
	if ( ret != XGE_OK ) return ret;
	ret = capture(demo);
	if ( ret != XGE_OK ) return ret;
	xgeClear(XGE_COLOR_RGBA(8, 10, 12, 255));
	memset(&draw, 0, sizeof(draw));
	texture = xgeRenderTargetTexture(&demo->tTarget);
	draw.pTexture = texture;
	draw.tSrc = rectf(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	draw.tDst = rectf(20.0f, 20.0f, (float)DEMO_W, (float)DEMO_H);
	draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	draw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&draw);
	ret = xgeEnd();
	if ( ret != XGE_OK ) return ret;
	demo->iFrame++;
	if ( demo->bCaptureDone || ((demo->iMaxFrames > 0) && (demo->iFrame >= demo->iMaxFrames)) ) {
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_shape_ex_demo_t demo;
	xge_desc_t desc;
	int ret;

	memset(&demo, 0, sizeof(demo));
	ret = parse_args(&demo, argc, argv);
	if ( ret == 1 ) return 0;
	if ( ret != XGE_OK ) {
		usage();
		return 1;
	}
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W + 40;
	desc.iHeight = DEMO_H + 40;
	desc.sTitle = "xge_shape_ex";
	desc.iFlags = XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) {
		printf("xge_shape_ex: xgeInit failed: %d\n", ret);
		return 1;
	}
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret != XGE_OK ) {
		printf("xge_shape_ex: render target create failed: %d\n", ret);
		xgeUnit();
		return 1;
	}
	ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
