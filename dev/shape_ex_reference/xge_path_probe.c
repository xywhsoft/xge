#include "xge.h"

#include <stdio.h>

static int dump_path(const char* name, xge_shape_ex shape)
{
	const uint8_t* commands = NULL;
	const xge_vec2_t* points = NULL;
	int commandCount = 0;
	int pointCount = 0;
	int i;

	if ( xgeShapeExGetPath(shape, &commands, &commandCount, &points, &pointCount) != XGE_OK ) return 0;
	printf("%s %d %d", name, commandCount, pointCount);
	for ( i = 0; i < commandCount; i++ ) printf(" %u", (unsigned int)commands[i]);
	for ( i = 0; i < pointCount; i++ ) printf(" %.9g %.9g", points[i].fX, points[i].fY);
	printf("\n");
	return 1;
}

static int reset_shape(xge_shape_ex shape)
{
	return xgeShapeExReset(shape) == XGE_OK;
}

int main(void)
{
	xge_shape_ex shape = NULL;
	const uint8_t appendCommands[4] = {
		XGE_SHAPE_EX_CMD_MOVE_TO, XGE_SHAPE_EX_CMD_LINE_TO,
		XGE_SHAPE_EX_CMD_CUBIC_TO, XGE_SHAPE_EX_CMD_CLOSE
	};
	const xge_vec2_t appendPoints[5] = {
		{1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}, {7.0f, 8.0f}, {9.0f, 10.0f}
	};

	if ( xgeShapeExCreate(&shape) != XGE_OK ) return 1;
	if ( xgeShapeExClose(shape) != XGE_OK || !dump_path("close_empty", shape) ) return 2;
	if ( !reset_shape(shape) || xgeShapeExLineTo(shape, 10.0f, 20.0f) != XGE_OK || !dump_path("line_first", shape) ) return 3;
	if ( !reset_shape(shape) || xgeShapeExCubicTo(shape, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f) != XGE_OK || !dump_path("cubic_first", shape) ) return 4;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 10.0f, 20.0f, 80.0f, 50.0f, 0.0f, 0.0f, 1) != XGE_OK || !dump_path("rect_sharp_cw", shape) ) return 5;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 10.0f, 20.0f, 80.0f, 50.0f, 0.0f, 0.0f, 0) != XGE_OK || !dump_path("rect_sharp_ccw", shape) ) return 6;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 10.0f, 20.0f, 80.0f, 50.0f, 13.0f, 9.0f, 1) != XGE_OK || !dump_path("rect_round_cw", shape) ) return 7;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 10.0f, 20.0f, 80.0f, 50.0f, 13.0f, 9.0f, 0) != XGE_OK || !dump_path("rect_round_ccw", shape) ) return 8;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 10.0f, 20.0f, 30.0f, 18.0f, 90.0f, 40.0f, 1) != XGE_OK || !dump_path("rect_radius_clamp", shape) ) return 9;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, 35.0f, 45.0f, 17.0f, 11.0f, 1) != XGE_OK || !dump_path("circle_cw", shape) ) return 10;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, 35.0f, 45.0f, 17.0f, 11.0f, 0) != XGE_OK || !dump_path("circle_ccw", shape) ) return 11;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, -5.0f, 7.0f, -3.0f, 4.0f, 1) != XGE_OK || !dump_path("circle_negative_rx", shape) ) return 12;
	if ( !reset_shape(shape) || xgeShapeExClose(shape) != XGE_OK || xgeShapeExClose(shape) != XGE_OK || !dump_path("close_twice", shape) ) return 13;
	if ( !reset_shape(shape) || xgeShapeExMoveTo(shape, 1.0f, 2.0f) != XGE_OK || xgeShapeExMoveTo(shape, 3.0f, 4.0f) != XGE_OK || !dump_path("move_twice", shape) ) return 14;
	if ( !reset_shape(shape) || xgeShapeExMoveTo(shape, 0.0f, 0.0f) != XGE_OK || xgeShapeExLineTo(shape, 1.0f, 0.0f) != XGE_OK ||
	     xgeShapeExClose(shape) != XGE_OK || xgeShapeExLineTo(shape, 2.0f, 0.0f) != XGE_OK || !dump_path("line_after_close", shape) ) return 15;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 2.0f, 3.0f, 40.0f, 20.0f, 8.0f, 0.0f, 1) != XGE_OK || !dump_path("rect_rx_only", shape) ) return 16;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 2.0f, 3.0f, 40.0f, 20.0f, 0.0f, 6.0f, 1) != XGE_OK || !dump_path("rect_ry_only", shape) ) return 17;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 2.0f, 3.0f, 40.0f, 20.0f, -5.0f, 7.0f, 1) != XGE_OK || !dump_path("rect_negative_rx", shape) ) return 18;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 2.0f, 3.0f, 40.0f, 20.0f, 5.0f, -7.0f, 1) != XGE_OK || !dump_path("rect_negative_ry", shape) ) return 19;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 20.0f, 30.0f, -40.0f, 18.0f, 5.0f, 4.0f, 0) != XGE_OK || !dump_path("rect_negative_width", shape) ) return 20;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, 5.0f, 6.0f, 0.0f, 8.0f, 1) != XGE_OK || !dump_path("circle_zero_rx", shape) ) return 21;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, 5.0f, 6.0f, 8.0f, 0.0f, 1) != XGE_OK || !dump_path("circle_zero_ry", shape) ) return 22;
	if ( !reset_shape(shape) || xgeShapeExAppendCircle(shape, 5.0f, 6.0f, 8.0f, -3.0f, 0) != XGE_OK || !dump_path("circle_negative_ry", shape) ) return 23;
	if ( !reset_shape(shape) || xgeShapeExAppendRect(shape, 0.0f, 0.0f, 10.0f, 12.0f, 2.0f, 3.0f, 1) != XGE_OK ||
	     xgeShapeExAppendCircle(shape, 20.0f, 8.0f, 4.0f, 6.0f, 0) != XGE_OK || !dump_path("append_mixed", shape) ) return 24;
	if ( !reset_shape(shape) || xgeShapeExAppendPath(shape, appendCommands, 4, appendPoints, 5) != XGE_OK || !dump_path("append_path", shape) ) return 25;
	if ( !reset_shape(shape) || xgeShapeExMoveTo(shape, -1.0f, -2.0f) != XGE_OK ||
	     xgeShapeExAppendPath(shape, appendCommands, 4, appendPoints, 5) != XGE_OK ||
	     xgeShapeExLineTo(shape, 11.0f, 12.0f) != XGE_OK || !dump_path("append_path_continue", shape) ) return 26;

	xgeShapeExDestroy(shape);
	return 0;
}
