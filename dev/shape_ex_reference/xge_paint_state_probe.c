#include "xge.h"

#include <stdint.h>
#include <stdio.h>

static int dump_state(const char* name, xge_shape_ex shape)
{
	const float* dash = (const float*)(uintptr_t)1;
	uint32_t fill = XGE_COLOR_RGBA(201, 202, 203, 204);
	uint32_t stroke = XGE_COLOR_RGBA(201, 202, 203, 204);
	float width;
	float miter;
	float offset = 91.0f;
	int fillType;
	int strokeType;
	int cap;
	int join;
	int dashCount = -1;
	int strokeColorOk;
	int i;

	if ( xgeShapeExFillColorGet(shape, &fill) != XGE_OK ||
	     xgeShapeExFillTypeGet(shape, &fillType) != XGE_OK ||
	     xgeShapeExStrokeTypeGet(shape, &strokeType) != XGE_OK ||
	     xgeShapeExStrokeWidthGet(shape, &width) != XGE_OK ||
	     xgeShapeExStrokeCapGet(shape, &cap) != XGE_OK ||
	     xgeShapeExStrokeJoinGet(shape, &join) != XGE_OK ||
	     xgeShapeExStrokeMiterLimitGet(shape, &miter) != XGE_OK ||
	     xgeShapeExStrokeDashGet(shape, &dash, &dashCount, &offset) != XGE_OK ) return 0;
	strokeColorOk = xgeShapeExStrokeColorGet(shape, &stroke) == XGE_OK;
	printf("%s %u %u %u %u %d %u %u %u %u %d %d %.9g %d %d %.9g %d %.9g",
		name,
		(unsigned int)XGE_COLOR_GET_R(fill), (unsigned int)XGE_COLOR_GET_G(fill),
		(unsigned int)XGE_COLOR_GET_B(fill), (unsigned int)XGE_COLOR_GET_A(fill),
		strokeColorOk,
		(unsigned int)XGE_COLOR_GET_R(stroke), (unsigned int)XGE_COLOR_GET_G(stroke),
		(unsigned int)XGE_COLOR_GET_B(stroke), (unsigned int)XGE_COLOR_GET_A(stroke),
		fillType != XGE_SHAPE_EX_PAINT_SOLID, strokeType != XGE_SHAPE_EX_PAINT_SOLID,
		width, cap, join, miter, dashCount, offset);
	for ( i = 0; i < dashCount; i++ ) printf(" %.9g", dash[i]);
	printf("\n");
	return 1;
}

int main(void)
{
	xge_shape_ex shape = NULL;
	xge_shape_ex clone = NULL;
	xge_shape_ex_color_stop_t stops[3] = {
		{0.0f, XGE_COLOR_RGBA(1, 2, 3, 4)},
		{0.6f, XGE_COLOR_RGBA(10, 20, 30, 40)},
		{1.0f, XGE_COLOR_RGBA(50, 60, 70, 80)}
	};
	float dash[4] = {-2.0f, 3.0f, 0.0f, 4.0f};

	if ( xgeShapeExCreate(&shape) != XGE_OK || !dump_state("default", shape) ) return 1;
	if ( xgeShapeExStrokeWidth(shape, -3.0f) != XGE_OK || !dump_state("width_negative", shape) ) return 2;
	if ( xgeShapeExFillColor(shape, XGE_COLOR_RGBA(11, 22, 33, 44)) != XGE_OK ||
	     xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(55, 66, 77, 88)) != XGE_OK ||
	     !dump_state("solid_colors", shape) ) return 3;
	if ( xgeShapeExFillLinearGradient(shape, 1.0f, 2.0f, 30.0f, 40.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3) != XGE_OK ||
	     !dump_state("fill_gradient", shape) ) return 4;
	if ( xgeShapeExStrokeLinearGradient(shape, 5.0f, 6.0f, 70.0f, 80.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3) != XGE_OK ||
	     !dump_state("stroke_gradient", shape) ) return 5;
	if ( xgeShapeExClone(shape, &clone) != XGE_OK || !dump_state("gradient_clone", clone) ) return 6;
	xgeShapeExDestroy(clone);
	clone = NULL;
	if ( xgeShapeExStrokeWidth(shape, 7.5f) != XGE_OK ||
	     xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND) != XGE_OK ||
	     xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_MITER) != XGE_OK ||
	     xgeShapeExStrokeMiterLimit(shape, 9.0f) != XGE_OK ||
	     xgeShapeExStrokeDash(shape, dash, 4, 5.5f) != XGE_OK ||
	     !dump_state("stroke_properties", shape) ) return 7;
	if ( xgeShapeExReset(shape) != XGE_OK || !dump_state("properties_reset_path", shape) ) return 8;
	if ( xgeShapeExStrokeDash(shape, NULL, 0, 7.0f) != XGE_OK || !dump_state("dash_reset", shape) ) return 9;
	if ( xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(90, 100, 110, 120)) != XGE_OK ||
	     !dump_state("stroke_solid_after_gradient", shape) ) return 10;
	if ( xgeShapeExFillRadialGradientEx(shape, 20.0f, 30.0f, 40.0f, 10.0f, 15.0f, 3.0f,
		XGE_SHAPE_EX_GRADIENT_USER_SPACE, stops, 3) != XGE_OK ||
	     !dump_state("radial_fill_state", shape) ) return 11;

	xgeShapeExDestroy(shape);
	return 0;
}
