#include "xge.h"

#include <stdio.h>

static void print_stops(const xge_shape_ex_color_stop_t* stops, int count)
{
	int i;

	printf(" %d", count);
	for ( i = 0; i < count; i++ ) {
		printf(" %.9g %u %u %u %u", stops[i].fOffset,
			(unsigned int)XGE_COLOR_GET_R(stops[i].iColor),
			(unsigned int)XGE_COLOR_GET_G(stops[i].iColor),
			(unsigned int)XGE_COLOR_GET_B(stops[i].iColor),
			(unsigned int)XGE_COLOR_GET_A(stops[i].iColor));
	}
	printf("\n");
}

static int dump_linear(const char* name, xge_shape_ex shape, int stroke)
{
	const xge_shape_ex_color_stop_t* stops = NULL;
	xge_shape_ex_matrix_t matrix;
	float x1;
	float y1;
	float x2;
	float y2;
	int units;
	int spread;
	int count;
	int ret;

	if ( stroke ) {
		ret = xgeShapeExStrokeLinearGradientGet(shape, &x1, &y1, &x2, &y2, &units, &stops, &count);
		if ( ret != XGE_OK || xgeShapeExStrokeGradientSpreadGet(shape, &spread) != XGE_OK ||
		     xgeShapeExStrokeGradientTransformGet(shape, &matrix) != XGE_OK ) return 0;
	} else {
		ret = xgeShapeExFillLinearGradientGet(shape, &x1, &y1, &x2, &y2, &units, &stops, &count);
		if ( ret != XGE_OK || xgeShapeExFillGradientSpreadGet(shape, &spread) != XGE_OK ||
		     xgeShapeExFillGradientTransformGet(shape, &matrix) != XGE_OK ) return 0;
	}
	printf("%s 1 %.9g %.9g %.9g %.9g 0 %d %.9g %.9g %.9g %.9g %.9g %.9g",
		name, x1, y1, x2, y2, spread,
		matrix.fA, matrix.fB, matrix.fC, matrix.fD, matrix.fE, matrix.fF);
	print_stops(stops, count);
	return 1;
}

static int dump_radial(const char* name, xge_shape_ex shape)
{
	const xge_shape_ex_color_stop_t* stops = NULL;
	xge_shape_ex_matrix_t matrix;
	float cx;
	float cy;
	float radius;
	float fx;
	float fy;
	float focalRadius;
	int units;
	int spread;
	int count;

	if ( xgeShapeExFillRadialGradientGetEx(shape, &cx, &cy, &radius, &fx, &fy, &focalRadius,
		&units, &stops, &count) != XGE_OK ||
	     xgeShapeExFillGradientSpreadGet(shape, &spread) != XGE_OK ||
	     xgeShapeExFillGradientTransformGet(shape, &matrix) != XGE_OK ) return 0;
	printf("%s 2 %.9g %.9g %.9g %.9g %.9g %d %.9g %.9g %.9g %.9g %.9g %.9g",
		name, cx, cy, radius, fx, fy, spread,
		matrix.fA, matrix.fB, matrix.fC, matrix.fD, matrix.fE, matrix.fF);
	printf(" %.9g", focalRadius);
	print_stops(stops, count);
	return 1;
}

int main(void)
{
	xge_shape_ex shape = NULL;
	xge_shape_ex clone = NULL;
	xge_shape_ex_matrix_t matrix = {1.1f, 4.4f, 2.2f, 5.5f, 3.3f, 6.6f};
	xge_shape_ex_color_stop_t stops[4] = {
		{0.8f, XGE_COLOR_RGBA(1, 2, 3, 4)},
		{-0.2f, XGE_COLOR_RGBA(10, 20, 30, 40)},
		{1.4f, XGE_COLOR_RGBA(50, 60, 70, 80)},
		{0.3f, XGE_COLOR_RGBA(90, 100, 110, 120)}
	};

	if ( xgeShapeExCreate(&shape) != XGE_OK ) return 1;
	if ( xgeShapeExFillLinearGradient(shape, -10.0f, 10.0f, 100.0f, 120.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 4) != XGE_OK ) return 2;
	if ( xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) != XGE_OK ||
	     xgeShapeExFillGradientTransformSet(shape, &matrix) != XGE_OK ||
	     !dump_linear("fill_linear_raw", shape, 0) ) return 3;
	if ( xgeShapeExClone(shape, &clone) != XGE_OK || !dump_linear("fill_linear_clone", clone, 0) ) return 4;
	if ( xgeShapeExFillGradientTransformIdentity(shape) != XGE_OK ||
	     !dump_linear("fill_linear_identity", shape, 0) ||
	     !dump_linear("fill_linear_clone_retained", clone, 0) ) return 4;
	xgeShapeExDestroy(clone);
	clone = NULL;

	if ( xgeShapeExStrokeLinearGradient(shape, -7.0f, 9.0f, 33.0f, 41.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 4) != XGE_OK ) return 5;
	if ( xgeShapeExStrokeGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) != XGE_OK ||
	     xgeShapeExStrokeGradientTransformSet(shape, &matrix) != XGE_OK ||
	     !dump_linear("stroke_linear_raw", shape, 1) ) return 6;
	if ( xgeShapeExStrokeGradientTransformIdentity(shape) != XGE_OK ||
	     !dump_linear("stroke_linear_identity", shape, 1) ) return 6;

	if ( xgeShapeExFillRadialGradientEx(shape, 100.0f, 120.0f, 50.0f, 10.0f, 20.0f, 5.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 4) != XGE_OK ) return 7;
	if ( xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) != XGE_OK ||
	     xgeShapeExFillGradientTransformSet(shape, &matrix) != XGE_OK ||
	     !dump_radial("fill_radial_raw", shape) ) return 8;
	if ( xgeShapeExClone(shape, &clone) != XGE_OK || !dump_radial("fill_radial_clone", clone) ) return 9;

	xgeShapeExDestroy(clone);
	xgeShapeExDestroy(shape);
	return 0;
}
