#include "xge.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char* message)
{
	if ( !condition ) {
		printf("xge smoke failed: %s\n", message);
		return 0;
	}
	return 1;
}

static int test_shape_ex(void)
{
	xge_shape_ex shape;
	xge_shape_ex_scene scene;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex_color_stop_t stops[2];
	xge_shape_ex_color_stop_t unorderedStops[4];
	xge_rect_t bounds;
	float dash[2] = {4.0f, 2.0f};
	float dashNegative[3] = {4.0f, -2.0f, 1.0f};
	float length;
	int ret;

	shape = NULL;
	ret = xgeShapeExCreate(&shape);
	if ( !check((ret == XGE_OK) && (shape != NULL), "ShapeEx create") ) return 0;
	if ( !check(xgeShapeExMatrixIdentity(&matrix) == XGE_OK, "ShapeEx matrix identity") ) return 0;
	if ( !check((matrix.fA == 1.0f) && (matrix.fB == 0.0f) && (matrix.fC == 0.0f) && (matrix.fD == 1.0f) && (matrix.fE == 0.0f) && (matrix.fF == 0.0f), "ShapeEx matrix identity values") ) return 0;
	if ( !check(xgeShapeExMatrixIdentity(NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix identity rejects null") ) return 0;
	{
		xge_shape_ex_matrix_t translate;
		xge_shape_ex_matrix_t scale;
		xge_shape_ex_matrix_t rotate;
		xge_shape_ex_matrix_t skew;
		xge_shape_ex_matrix_t combined;
		xge_shape_ex_matrix_t inverse;
		xge_shape_ex_matrix_t singular;
		xge_vec2_t matrixPoint;
		xge_vec2_t matrixVector;
		xge_rect_t matrixRect;
		float strokeScale;

		if ( !check(xgeShapeExMatrixTranslate(&translate, 5.0f, 7.0f) == XGE_OK, "ShapeEx matrix translate") ) return 0;
		if ( !check((translate.fA == 1.0f) && (translate.fD == 1.0f) && (translate.fE == 5.0f) && (translate.fF == 7.0f), "ShapeEx matrix translate values") ) return 0;
		if ( !check(xgeShapeExMatrixScale(&scale, 2.0f, 3.0f) == XGE_OK, "ShapeEx matrix scale") ) return 0;
		if ( !check((scale.fA == 2.0f) && (scale.fD == 3.0f) && (scale.fE == 0.0f) && (scale.fF == 0.0f), "ShapeEx matrix scale values") ) return 0;
		if ( !check(xgeShapeExMatrixMultiply(&combined, &translate, &scale) == XGE_OK, "ShapeEx matrix multiply") ) return 0;
		if ( !check((combined.fA == 2.0f) && (combined.fB == 0.0f) && (combined.fC == 0.0f) && (combined.fD == 3.0f) && (combined.fE == 5.0f) && (combined.fF == 7.0f), "ShapeEx matrix multiply values") ) return 0;
		if ( !check(xgeShapeExMatrixMultiply(NULL, &translate, &scale) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix multiply rejects null output") ) return 0;
		if ( !check(xgeShapeExMatrixPoint(&matrixPoint, &combined, (xge_vec2_t){2.0f, 3.0f}) == XGE_OK, "ShapeEx matrix point") ) return 0;
		if ( !check((matrixPoint.fX > 8.999f) && (matrixPoint.fX < 9.001f) && (matrixPoint.fY > 15.999f) && (matrixPoint.fY < 16.001f), "ShapeEx matrix point values") ) return 0;
		if ( !check(xgeShapeExMatrixVector(&matrixVector, &combined, (xge_vec2_t){2.0f, 3.0f}) == XGE_OK, "ShapeEx matrix vector") ) return 0;
		if ( !check((matrixVector.fX > 3.999f) && (matrixVector.fX < 4.001f) && (matrixVector.fY > 8.999f) && (matrixVector.fY < 9.001f), "ShapeEx matrix vector values") ) return 0;
		if ( !check(xgeShapeExMatrixVector(NULL, &combined, (xge_vec2_t){0.0f, 0.0f}) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix vector rejects null") ) return 0;
		if ( !check(xgeShapeExMatrixStrokeScale(&strokeScale, &combined) == XGE_OK, "ShapeEx matrix stroke scale") ) return 0;
		if ( !check((strokeScale > 2.449f) && (strokeScale < 2.450f), "ShapeEx matrix stroke scale values") ) return 0;
		if ( !check(xgeShapeExMatrixStrokeScale(NULL, &combined) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix stroke scale rejects null") ) return 0;
		if ( !check(xgeShapeExMatrixRectBounds(&matrixRect, &combined, (xge_rect_t){1.0f, 2.0f, 3.0f, 4.0f}) == XGE_OK, "ShapeEx matrix rect bounds") ) return 0;
		if ( !check((matrixRect.fX > 6.999f) && (matrixRect.fX < 7.001f) && (matrixRect.fY > 12.999f) && (matrixRect.fY < 13.001f) && (matrixRect.fW > 5.999f) && (matrixRect.fW < 6.001f) && (matrixRect.fH > 11.999f) && (matrixRect.fH < 12.001f), "ShapeEx matrix rect bounds values") ) return 0;
		if ( !check(xgeShapeExMatrixInvert(&inverse, &combined) == XGE_OK, "ShapeEx matrix invert") ) return 0;
		if ( !check(xgeShapeExMatrixPoint(&matrixPoint, &inverse, matrixPoint) == XGE_OK, "ShapeEx matrix inverse point") ) return 0;
		if ( !check((matrixPoint.fX > 1.999f) && (matrixPoint.fX < 2.001f) && (matrixPoint.fY > 2.999f) && (matrixPoint.fY < 3.001f), "ShapeEx matrix inverse point values") ) return 0;
		if ( !check(xgeShapeExMatrixScale(&singular, 0.0f, 1.0f) == XGE_OK, "ShapeEx matrix singular scale") ) return 0;
		if ( !check(xgeShapeExMatrixInvert(&inverse, &singular) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix invert rejects singular") ) return 0;
		if ( !check(xgeShapeExMatrixPoint(NULL, &combined, (xge_vec2_t){0.0f, 0.0f}) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix point rejects null") ) return 0;
		if ( !check(xgeShapeExMatrixRotate(&rotate, 1.57079632679489661923f) == XGE_OK, "ShapeEx matrix rotate") ) return 0;
		if ( !check((rotate.fA > -0.001f) && (rotate.fA < 0.001f) && (rotate.fB > 0.999f) && (rotate.fB < 1.001f) && (rotate.fC > -1.001f) && (rotate.fC < -0.999f) && (rotate.fD > -0.001f) && (rotate.fD < 0.001f), "ShapeEx matrix rotate values") ) return 0;
		if ( !check(xgeShapeExMatrixSkew(&skew, 0.4636476090008061f, 0.24497866312686414f) == XGE_OK, "ShapeEx matrix skew") ) return 0;
		if ( !check((skew.fA == 1.0f) && (skew.fB > 0.249f) && (skew.fB < 0.251f) && (skew.fC > 0.499f) && (skew.fC < 0.501f) && (skew.fD == 1.0f), "ShapeEx matrix skew values") ) return 0;
	}
	if ( !check(xgeShapeExMoveTo(shape, 0.0f, 0.0f) == XGE_OK, "ShapeEx moveTo") ) return 0;
	if ( !check(xgeShapeExLineTo(shape, 10.0f, 0.0f) == XGE_OK, "ShapeEx lineTo") ) return 0;
	if ( !check(xgeShapeExQuadTo(shape, 12.0f, 6.0f, 10.0f, 10.0f) == XGE_OK, "ShapeEx quadTo") ) return 0;
	if ( !check(xgeShapeExCubicTo(shape, 8.0f, 14.0f, 2.0f, 14.0f, 0.0f, 10.0f) == XGE_OK, "ShapeEx cubicTo") ) return 0;
	if ( !check(xgeShapeExClose(shape) == XGE_OK, "ShapeEx close") ) return 0;
	if ( !check(xgeShapeExAppendSvgPath(shape, "M20 20 A8 8 0 0 1 28 28 S40 36 48 28") == XGE_OK, "ShapeEx SVG path") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 4.0f, 4.0f, 18.0f, 12.0f, 3.0f, 3.0f, 1) == XGE_OK, "ShapeEx append rect") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 5.0f, 5.0f, 10.0f, 8.0f, 2.0f, 2.0f, 0) == XGE_OK, "ShapeEx append ccw rounded rect") ) return 0;
	if ( !check(xgeShapeExAppendTriangle(shape, 2.0f, 18.0f, 8.0f, 24.0f, 14.0f, 18.0f, 1) == XGE_OK, "ShapeEx append triangle") ) return 0;
	if ( !check(xgeShapeExAppendTriangle(shape, 16.0f, 18.0f, 22.0f, 24.0f, 28.0f, 18.0f, 0) == XGE_OK, "ShapeEx append ccw triangle") ) return 0;
	if ( !check(xgeShapeExAppendCapsule(shape, 30.0f, 18.0f, 14.0f, 6.0f, 1) == XGE_OK, "ShapeEx append capsule") ) return 0;
	if ( !check(xgeShapeExAppendCapsule(shape, 46.0f, 18.0f, 6.0f, 14.0f, 0) == XGE_OK, "ShapeEx append ccw capsule") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 5.0f, 5.0f, 1) == XGE_OK, "ShapeEx append circle") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 3.0f, 4.0f, 0) == XGE_OK, "ShapeEx append ccw ellipse") ) return 0;
	{
		xge_shape_ex exact = NULL;
		const uint8_t* commands = NULL;
		const xge_vec2_t* points = NULL;
		int commandCount = 0;
		int pointCount = 0;

		ret = xgeShapeExCreate(&exact);
		if ( !check((ret == XGE_OK) && (exact != NULL), "ShapeEx exact primitive create") ) return 0;
		if ( !check(xgeShapeExAppendRect(exact, 1.0f, 2.0f, 4.0f, 3.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx exact rect append") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		ret = xgeShapeExGetPath(exact, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 5) && (pointCount == 4), "ShapeEx exact rect path counts") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((commands[0] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[1] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[2] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[3] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[4] == XGE_SHAPE_EX_CMD_CLOSE), "ShapeEx exact rect commands") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((points[0].fX == 5.0f) && (points[0].fY == 2.0f) && (points[1].fX == 5.0f) && (points[1].fY == 5.0f) && (points[2].fX == 1.0f) && (points[2].fY == 5.0f) && (points[3].fX == 1.0f) && (points[3].fY == 2.0f), "ShapeEx exact rect ThorVG point order") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check(xgeShapeExReset(exact) == XGE_OK, "ShapeEx exact reset") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check(xgeShapeExAppendCircle(exact, 10.0f, 20.0f, 3.0f, 4.0f, 1) == XGE_OK, "ShapeEx exact circle append") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		ret = xgeShapeExGetPath(exact, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 6) && (pointCount == 13), "ShapeEx exact circle path counts") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((commands[0] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[1] == XGE_SHAPE_EX_CMD_CUBIC_TO) && (commands[2] == XGE_SHAPE_EX_CMD_CUBIC_TO) && (commands[3] == XGE_SHAPE_EX_CMD_CUBIC_TO) && (commands[4] == XGE_SHAPE_EX_CMD_CUBIC_TO) && (commands[5] == XGE_SHAPE_EX_CMD_CLOSE), "ShapeEx exact circle commands") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((points[0].fX == 10.0f) && (points[0].fY == 16.0f) && (points[3].fX == 13.0f) && (points[3].fY == 20.0f) && (points[6].fX == 10.0f) && (points[6].fY == 24.0f) && (points[9].fX == 7.0f) && (points[9].fY == 20.0f) && (points[12].fX == 10.0f) && (points[12].fY == 16.0f), "ShapeEx exact circle ThorVG endpoints") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		xgeShapeExDestroy(exact);
	}
	if ( !check(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(1, 2, 3, 4)) == XGE_OK, "ShapeEx fill color") ) return 0;
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	stops[1].fOffset = 1.0f;
	stops[1].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx linear gradient") ) return 0;
	if ( !check(xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) == XGE_OK, "ShapeEx gradient spread") ) return 0;
	matrix.fA = 0.8f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 0.1f;
	matrix.fF = 0.0f;
	if ( !check(xgeShapeExFillGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx gradient transform") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		xge_shape_ex_matrix_t gotGradientMatrix;
		float x1 = -1.0f, y1 = -1.0f, x2 = -1.0f, y2 = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExFillTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx fill type get linear") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_LINEAR_GRADIENT, "ShapeEx fill type get linear value") ) return 0;
		if ( !check(xgeShapeExFillLinearGradientGet(shape, &x1, &y1, &x2, &y2, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx fill linear gradient get") ) return 0;
		if ( !check((x1 == 0.0f) && (y1 == 0.0f) && (x2 == 1.0f) && (y2 == 0.0f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx fill linear gradient get values") ) return 0;
		if ( !check((gotStops != NULL) && (gotStops != stops) && (gotCount == 2) && (gotStops[0].fOffset == 0.0f) && (gotStops[1].fOffset == 1.0f), "ShapeEx fill linear gradient get stops") ) return 0;
		if ( !check(xgeShapeExFillGradientSpreadGet(shape, &gotInt) == XGE_OK, "ShapeEx fill gradient spread get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT, "ShapeEx fill gradient spread get value") ) return 0;
		if ( !check(xgeShapeExFillGradientTransformGet(shape, &gotGradientMatrix) == XGE_OK, "ShapeEx fill gradient transform get") ) return 0;
		if ( !check((gotGradientMatrix.fA == 0.8f) && (gotGradientMatrix.fE == 0.1f), "ShapeEx fill gradient transform get value") ) return 0;
		if ( !check(xgeShapeExFillRadialGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx fill radial gradient get rejects linear") ) return 0;
	}
	unorderedStops[0].fOffset = -0.25f;
	unorderedStops[0].iColor = XGE_COLOR_RGBA(10, 0, 0, 255);
	unorderedStops[1].fOffset = 0.6f;
	unorderedStops[1].iColor = XGE_COLOR_RGBA(20, 0, 0, 255);
	unorderedStops[2].fOffset = 0.2f;
	unorderedStops[2].iColor = XGE_COLOR_RGBA(30, 0, 0, 255);
	unorderedStops[3].fOffset = 1.5f;
	unorderedStops[3].iColor = XGE_COLOR_RGBA(40, 0, 0, 255);
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, unorderedStops, 4) == XGE_OK, "ShapeEx linear gradient unordered stops") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		int gotCount = 0;

		if ( !check(xgeShapeExFillLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, &gotStops, &gotCount) == XGE_OK, "ShapeEx linear gradient unordered stops get") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 4) &&
		            (gotStops[0].fOffset == 0.0f) && (gotStops[0].iColor == unorderedStops[0].iColor) &&
		            (gotStops[1].fOffset == 0.6f) && (gotStops[1].iColor == unorderedStops[1].iColor) &&
		            (gotStops[2].fOffset == 0.6f) && (gotStops[2].iColor == unorderedStops[2].iColor) &&
		            (gotStops[3].fOffset == 1.0f) && (gotStops[3].iColor == unorderedStops[3].iColor), "ShapeEx linear gradient preserves stop order") ) return 0;
	}
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx radial gradient") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		xge_shape_ex_matrix_t gotGradientMatrix;
		float cx = -1.0f, cy = -1.0f, radius = -1.0f, fx = -1.0f, fy = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExFillTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx fill type get radial") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT, "ShapeEx fill type get radial value") ) return 0;
		if ( !check(xgeShapeExFillRadialGradientGet(shape, &cx, &cy, &radius, &fx, &fy, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx fill radial gradient get") ) return 0;
		if ( !check((cx == 0.5f) && (cy == 0.5f) && (radius == 0.5f) && (fx == 0.45f) && (fy == 0.45f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx fill radial gradient get values") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 2) && (gotStops[0].iColor == stops[0].iColor) && (gotStops[1].iColor == stops[1].iColor), "ShapeEx fill radial gradient get stops") ) return 0;
		if ( !check(xgeShapeExFillLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx fill linear gradient get rejects radial") ) return 0;
		if ( !check(xgeShapeExFillGradientSpreadGet(shape, &gotInt) == XGE_OK, "ShapeEx fill radial gradient spread get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_GRADIENT_SPREAD_PAD, "ShapeEx fill radial gradient spread reset") ) return 0;
		if ( !check(xgeShapeExFillGradientTransformGet(shape, &gotGradientMatrix) == XGE_OK, "ShapeEx fill radial gradient transform get") ) return 0;
		if ( !check((gotGradientMatrix.fA == 1.0f) && (gotGradientMatrix.fD == 1.0f) && (gotGradientMatrix.fE == 0.0f), "ShapeEx fill radial gradient transform reset") ) return 0;
	}
	if ( !check(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(1, 2, 3, 4)) == XGE_OK, "ShapeEx fill color reset") ) return 0;
	{
		int gotInt = -1;

		if ( !check(xgeShapeExFillTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx fill type get solid") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_SOLID, "ShapeEx fill type get solid value") ) return 0;
		if ( !check(xgeShapeExFillLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx fill linear gradient get rejects solid") ) return 0;
		if ( !check(xgeShapeExFillTypeGet(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill type get rejects null") ) return 0;
	}
	if ( !check(xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(5, 6, 7, 8)) == XGE_OK, "ShapeEx stroke color") ) return 0;
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke linear gradient") ) return 0;
	if ( !check(xgeShapeExStrokeGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) == XGE_OK, "ShapeEx stroke gradient spread") ) return 0;
	if ( !check(xgeShapeExStrokeGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx stroke gradient transform") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		xge_shape_ex_matrix_t gotGradientMatrix;
		float x1 = -1.0f, y1 = -1.0f, x2 = -1.0f, y2 = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExStrokeTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke type get linear") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_LINEAR_GRADIENT, "ShapeEx stroke type get linear value") ) return 0;
		if ( !check(xgeShapeExStrokeLinearGradientGet(shape, &x1, &y1, &x2, &y2, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx stroke linear gradient get") ) return 0;
		if ( !check((x1 == 0.0f) && (y1 == 0.0f) && (x2 == 1.0f) && (y2 == 0.0f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx stroke linear gradient get values") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 2), "ShapeEx stroke linear gradient get stops") ) return 0;
		if ( !check(xgeShapeExStrokeGradientSpreadGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke gradient spread get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT, "ShapeEx stroke gradient spread get value") ) return 0;
		if ( !check(xgeShapeExStrokeGradientTransformGet(shape, &gotGradientMatrix) == XGE_OK, "ShapeEx stroke gradient transform get") ) return 0;
		if ( !check((gotGradientMatrix.fA == 0.8f) && (gotGradientMatrix.fE == 0.1f), "ShapeEx stroke gradient transform get value") ) return 0;
	}
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, unorderedStops, 4) == XGE_OK, "ShapeEx stroke gradient unordered stops") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		int gotCount = 0;

		if ( !check(xgeShapeExStrokeLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, &gotStops, &gotCount) == XGE_OK, "ShapeEx stroke gradient unordered stops get") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 4) &&
		            (gotStops[0].fOffset == 0.0f) && (gotStops[0].iColor == unorderedStops[0].iColor) &&
		            (gotStops[1].fOffset == 0.6f) && (gotStops[1].iColor == unorderedStops[1].iColor) &&
		            (gotStops[2].fOffset == 0.6f) && (gotStops[2].iColor == unorderedStops[2].iColor) &&
		            (gotStops[3].fOffset == 1.0f) && (gotStops[3].iColor == unorderedStops[3].iColor), "ShapeEx stroke gradient preserves stop order") ) return 0;
	}
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		float cx = -1.0f, cy = -1.0f, radius = -1.0f, fx = -1.0f, fy = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExStrokeTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke type get radial") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT, "ShapeEx stroke type get radial value") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientGet(shape, &cx, &cy, &radius, &fx, &fy, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx stroke radial gradient get") ) return 0;
		if ( !check((cx == 0.5f) && (cy == 0.5f) && (radius == 0.5f) && (fx == 0.45f) && (fy == 0.45f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx stroke radial gradient get values") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 2), "ShapeEx stroke radial gradient get stops") ) return 0;
		if ( !check(xgeShapeExStrokeLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx stroke linear gradient get rejects radial") ) return 0;
	}
	if ( !check(xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(5, 6, 7, 8)) == XGE_OK, "ShapeEx stroke color reset") ) return 0;
	{
		int gotInt = -1;

		if ( !check(xgeShapeExStrokeTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke type get solid") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_SOLID, "ShapeEx stroke type get solid value") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx stroke radial gradient get rejects solid") ) return 0;
		if ( !check(xgeShapeExStrokeTypeGet(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke type get rejects null") ) return 0;
	}
	if ( !check(xgeShapeExStrokeWidth(shape, 2.0f) == XGE_OK, "ShapeEx stroke width") ) return 0;
	if ( !check(xgeShapeExStrokeWidth(shape, -2.0f) == XGE_OK, "ShapeEx negative stroke width clamps") ) return 0;
	if ( !check(xgeShapeExStrokeWidth(shape, 2.0f) == XGE_OK, "ShapeEx stroke width restore") ) return 0;
	if ( !check(xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx stroke cap") ) return 0;
	if ( !check(xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND) == XGE_OK, "ShapeEx stroke join") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 2, 1.0f) == XGE_OK, "ShapeEx stroke dash") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 0, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke dash rejects pattern with zero count") ) return 0;
	if ( !check(xgeShapeExStrokeNonScaling(shape, 1) == XGE_OK, "ShapeEx non-scaling stroke") ) return 0;
	if ( !check(xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx fill rule") ) return 0;
	if ( !check(xgeShapeExPaintOrder(shape, 1) == XGE_OK, "ShapeEx paint order") ) return 0;
	if ( !check(xgeShapeExOpacity(shape, 0.75f) == XGE_OK, "ShapeEx opacity") ) return 0;
	{
		const float* gotDash = NULL;
		uint32_t gotColor = 0;
		float gotFloat = 0.0f;
		float gotDashOffset = 0.0f;
		int gotInt = 0;
		int gotDashCount = 0;

		if ( !check(xgeShapeExFillColorGet(shape, &gotColor) == XGE_OK, "ShapeEx fill color get") ) return 0;
		if ( !check(gotColor == XGE_COLOR_RGBA(1, 2, 3, 4), "ShapeEx fill color get value") ) return 0;
		if ( !check(xgeShapeExStrokeColorGet(shape, &gotColor) == XGE_OK, "ShapeEx stroke color get") ) return 0;
		if ( !check(gotColor == XGE_COLOR_RGBA(5, 6, 7, 8), "ShapeEx stroke color get value") ) return 0;
		if ( !check(xgeShapeExStrokeWidthGet(shape, &gotFloat) == XGE_OK, "ShapeEx stroke width get") ) return 0;
		if ( !check((gotFloat > 1.999f) && (gotFloat < 2.001f), "ShapeEx stroke width get value") ) return 0;
		if ( !check(xgeShapeExStrokeCapGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke cap get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_CAP_ROUND, "ShapeEx stroke cap get value") ) return 0;
		if ( !check(xgeShapeExStrokeJoinGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke join get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_JOIN_ROUND, "ShapeEx stroke join get value") ) return 0;
		if ( !check(xgeShapeExStrokeMiterLimitGet(shape, &gotFloat) == XGE_OK, "ShapeEx stroke miter get") ) return 0;
		if ( !check(gotFloat > 0.0f, "ShapeEx stroke miter get value") ) return 0;
		if ( !check(xgeShapeExStrokeNonScalingGet(shape, &gotInt) == XGE_OK, "ShapeEx non-scaling stroke get") ) return 0;
		if ( !check(gotInt == 1, "ShapeEx non-scaling stroke get value") ) return 0;
		if ( !check(xgeShapeExStrokeDashGet(shape, &gotDash, &gotDashCount, &gotDashOffset) == XGE_OK, "ShapeEx stroke dash get") ) return 0;
		if ( !check((gotDash != NULL) && (gotDash != dash) && (gotDashCount == 2) && (gotDash[0] == 4.0f) && (gotDash[1] == 2.0f) && (gotDashOffset == 1.0f), "ShapeEx stroke dash get value") ) return 0;
		if ( !check(xgeShapeExStrokeDash(shape, dashNegative, 3, -1.0f) == XGE_OK, "ShapeEx stroke dash clamps negatives") ) return 0;
		if ( !check(xgeShapeExStrokeDashGet(shape, &gotDash, &gotDashCount, &gotDashOffset) == XGE_OK, "ShapeEx negative stroke dash get") ) return 0;
		if ( !check((gotDash != NULL) && (gotDashCount == 3) && (gotDash[0] == 4.0f) && (gotDash[1] == 0.0f) && (gotDash[2] == 1.0f) && (gotDashOffset == -1.0f), "ShapeEx negative stroke dash get value") ) return 0;
		if ( !check(xgeShapeExStrokeDash(shape, dash, 2, 1.0f) == XGE_OK, "ShapeEx stroke dash restore") ) return 0;
		if ( !check(xgeShapeExFillRuleGet(shape, &gotInt) == XGE_OK, "ShapeEx fill rule get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_FILL_EVEN_ODD, "ShapeEx fill rule get value") ) return 0;
		if ( !check(xgeShapeExPaintOrderGet(shape, &gotInt) == XGE_OK, "ShapeEx paint order get") ) return 0;
		if ( !check(gotInt == 1, "ShapeEx paint order get value") ) return 0;
		if ( !check(xgeShapeExOpacityGet(shape, &gotFloat) == XGE_OK, "ShapeEx opacity get") ) return 0;
		if ( !check((gotFloat > 0.749f) && (gotFloat < 0.751f), "ShapeEx opacity get value") ) return 0;
		if ( !check(xgeShapeExVisibleGet(shape, &gotInt) == XGE_OK, "ShapeEx visible get") ) return 0;
		if ( !check(gotInt == 1, "ShapeEx visible get default value") ) return 0;
		if ( !check(xgeShapeExVisible(shape, 0) == XGE_OK, "ShapeEx visible false") ) return 0;
		if ( !check(xgeShapeExVisibleGet(shape, &gotInt) == XGE_OK, "ShapeEx visible false get") ) return 0;
		if ( !check(gotInt == 0, "ShapeEx visible false get value") ) return 0;
		if ( !check(xgeShapeExVisible(shape, 1) == XGE_OK, "ShapeEx visible true restore") ) return 0;
		if ( !check(xgeShapeExFillColorGet(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill color get rejects null") ) return 0;
		if ( !check(xgeShapeExStrokeDashGet(NULL, &gotDash, &gotDashCount, &gotDashOffset) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke dash get rejects null shape") ) return 0;
	}
	if ( !check((xgeShapeExGetBounds(shape, 0.25f, &bounds) == XGE_OK) && (bounds.fW > 0.0f) && (bounds.fH > 0.0f), "ShapeEx bounds") ) return 0;
	if ( !check((xgeShapeExGetLength(shape, 0.25f, &length) == XGE_OK) && (length > 0.0f), "ShapeEx length") ) return 0;
	{
		xge_shape_ex_path_measure measure = NULL;
		float measureLength;
		float scaledLength;
		xge_vec2_t measurePoint;
		xge_vec2_t measureTangent;

		ret = xgeShapeExPathMeasureCreate(&measure, shape, NULL, 0.25f);
		if ( !check((ret == XGE_OK) && (measure != NULL), "ShapeEx path measure create") ) return 0;
		if ( !check(xgeShapeExPathMeasureGetLength(measure, &measureLength) == XGE_OK, "ShapeEx path measure length") ) {
			xgeShapeExPathMeasureDestroy(measure);
			return 0;
		}
		if ( !check((measureLength > length - 0.01f) && (measureLength < length + 0.01f), "ShapeEx path measure length values") ) {
			xgeShapeExPathMeasureDestroy(measure);
			return 0;
		}
		if ( !check(xgeShapeExPathMeasureGetPointAtLength(measure, measureLength * 0.5f, &measurePoint, &measureTangent) == XGE_OK, "ShapeEx path measure point") ) {
			xgeShapeExPathMeasureDestroy(measure);
			return 0;
		}
		xgeShapeExPathMeasureDestroy(measure);
		if ( !check(xgeShapeExPathMeasureCreate(NULL, shape, NULL, 0.25f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx path measure create rejects null output") ) return 0;
		if ( !check(xgeShapeExPathMeasureGetLength(NULL, &measureLength) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx path measure length rejects null") ) return 0;
		if ( !check(xgeShapeExPathMeasureGetPointAtLength(NULL, 0.0f, &measurePoint, &measureTangent) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx path measure point rejects null") ) return 0;
		if ( !check(xgeShapeExMatrixScale(&matrix, 2.0f, 2.0f) == XGE_OK, "ShapeEx path measure scale matrix") ) return 0;
		ret = xgeShapeExPathMeasureCreate(&measure, shape, &matrix, 0.25f);
		if ( !check((ret == XGE_OK) && (measure != NULL), "ShapeEx transformed path measure create") ) return 0;
		if ( !check(xgeShapeExPathMeasureGetLength(measure, &scaledLength) == XGE_OK, "ShapeEx transformed path measure length") ) {
			xgeShapeExPathMeasureDestroy(measure);
			return 0;
		}
		if ( !check((scaledLength > measureLength * 1.98f) && (scaledLength < measureLength * 2.02f), "ShapeEx transformed path measure length values") ) {
			xgeShapeExPathMeasureDestroy(measure);
			return 0;
		}
		xgeShapeExPathMeasureDestroy(measure);
	}
	if ( !check(xgeShapeExClipRectSet(shape, (xge_rect_t){1.0f, 2.0f, 8.0f, 9.0f}) == XGE_OK, "ShapeEx clip rect") ) return 0;
	if ( !check(xgeShapeExClipClear(shape) == XGE_OK, "ShapeEx clip clear") ) return 0;
	{
		int applied = -1;

		if ( !check(xgeShapeExStencilClipBegin(NULL, 0.25f, NULL, &applied) == XGE_OK, "ShapeEx stencil clip begin null shape no-op") ) return 0;
		if ( !check(applied == 0, "ShapeEx stencil clip begin null shape applied") ) return 0;
		if ( !check(xgeShapeExStencilClipBegin(shape, 0.25f, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stencil clip begin rejects null applied") ) return 0;
		if ( !check(xgeShapeExStencilClipBeginPx(shape, 0.25f, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stencil clip begin px rejects null applied") ) return 0;
		if ( !check(xgeShapeExStencilClipEnd(0, XGE_OK) == XGE_OK, "ShapeEx stencil clip end no-op") ) return 0;
	}
	{
		xge_shape_ex clip = NULL;
		xge_shape_ex clippedClone = NULL;

		ret = xgeShapeExCreate(&clip);
		if ( !check((ret == XGE_OK) && (clip != NULL), "ShapeEx clip shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(clip, 0.0f, 0.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip shape path") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeAdd(shape, shape) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects self clip shape") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeAdd(shape, clip) == XGE_OK, "ShapeEx clip shape add") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		ret = xgeShapeExClone(shape, &clippedClone);
		if ( !check((ret == XGE_OK) && (clippedClone != NULL), "ShapeEx clone with clip shape") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeClear(shape) == XGE_OK, "ShapeEx clip shape clear") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipClear(clippedClone) == XGE_OK, "ShapeEx cloned clip clear") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		xgeShapeExDestroy(clippedClone);
		xgeShapeExDestroy(clip);
	}
	matrix.fA = 1.0f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 8.0f;
	matrix.fF = 9.0f;
	if ( !check(xgeShapeExTransformSet(shape, &matrix) == XGE_OK, "ShapeEx transform") ) return 0;
	{
		xge_shape_ex_matrix_t gotTransform;

		if ( !check(xgeShapeExTransformGet(shape, &gotTransform) == XGE_OK, "ShapeEx transform get") ) return 0;
		if ( !check((gotTransform.fE == 8.0f) && (gotTransform.fF == 9.0f), "ShapeEx transform get value") ) return 0;
		if ( !check(xgeShapeExTransformGet(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx transform get rejects null") ) return 0;
	}
	if ( !check(xgeShapeExTransformIdentity(shape) == XGE_OK, "ShapeEx transform identity") ) return 0;
	{
		xge_shape_ex transformShape = NULL;
		xge_shape_ex_matrix_t gotTransform;
		xge_rect_t transformBounds;

		ret = xgeShapeExCreate(&transformShape);
		if ( !check((ret == XGE_OK) && (transformShape != NULL), "ShapeEx transform compose shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(transformShape, 0.0f, 0.0f, 4.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx transform compose rect") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformTranslate(transformShape, 10.0f, 20.0f) == XGE_OK, "ShapeEx transform translate") ||
		     !check(xgeShapeExTransformScale(transformShape, 2.0f, 3.0f) == XGE_OK, "ShapeEx transform scale") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformGet(transformShape, &gotTransform) == XGE_OK, "ShapeEx transform compose get") ||
		     !check((gotTransform.fA == 2.0f) && (gotTransform.fD == 3.0f) && (gotTransform.fE == 10.0f) && (gotTransform.fF == 20.0f), "ShapeEx transform compose matrix") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(transformShape, 0.05f, &transformBounds);
		if ( !check((ret == XGE_OK) && (transformBounds.fX > 9.99f) && (transformBounds.fX < 10.01f) && (transformBounds.fY > 19.99f) && (transformBounds.fY < 20.01f) && (transformBounds.fW > 7.99f) && (transformBounds.fW < 8.01f) && (transformBounds.fH > 5.99f) && (transformBounds.fH < 6.01f), "ShapeEx transform compose bounds") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformIdentity(transformShape) == XGE_OK, "ShapeEx transform compose identity") ||
		     !check(xgeShapeExTransformRotate(transformShape, 1.57079632679489661923f) == XGE_OK, "ShapeEx transform rotate") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(transformShape, 0.05f, &transformBounds);
		if ( !check((ret == XGE_OK) && (transformBounds.fX > -2.01f) && (transformBounds.fX < -1.99f) && (transformBounds.fY > -0.01f) && (transformBounds.fY < 0.01f) && (transformBounds.fW > 1.99f) && (transformBounds.fW < 2.01f) && (transformBounds.fH > 3.99f) && (transformBounds.fH < 4.01f), "ShapeEx transform rotate bounds") ||
		     !check(xgeShapeExTransformTranslate(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx transform translate rejects null") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		xgeShapeExDestroy(transformShape);
	}
	if ( !check(xgeShapeExDrawEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw ex rejects null shape") ) return 0;
	if ( !check(xgeShapeExDrawPxEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw px ex rejects null shape") ) return 0;
	{
		xge_shape_ex clone = NULL;
		ret = xgeShapeExClone(shape, &clone);
		if ( !check((ret == XGE_OK) && (clone != NULL), "ShapeEx clone") ) return 0;
		xgeShapeExDestroy(clone);
	}
	scene = NULL;
	ret = xgeShapeExSceneCreate(&scene);
	if ( !check((ret == XGE_OK) && (scene != NULL), "ShapeEx scene create") ) return 0;
	if ( !check(xgeShapeExSceneAdd(scene, shape) == XGE_OK, "ShapeEx scene add") ) return 0;
	if ( !check(xgeShapeExSceneOpacity(scene, 0.5f) == XGE_OK, "ShapeEx scene opacity") ) return 0;
	if ( !check(xgeShapeExSceneTransformSet(scene, &matrix) == XGE_OK, "ShapeEx scene transform") ) return 0;
	{
		xge_shape_ex sceneItem = NULL;
		xge_shape_ex_matrix_t gotSceneTransform;
		float gotOpacity = 0.0f;
		int gotCount = 0;
		int gotVisible = -1;

		if ( !check(xgeShapeExSceneGetCount(scene, &gotCount) == XGE_OK, "ShapeEx scene count get") ) return 0;
		if ( !check(gotCount == 1, "ShapeEx scene count get value") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 0, &sceneItem) == XGE_OK, "ShapeEx scene get at") ) return 0;
		if ( !check(sceneItem == shape, "ShapeEx scene get at value") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 1, &sceneItem) == XGE_ERROR_NOT_FOUND, "ShapeEx scene get at missing") ) return 0;
		if ( !check(sceneItem == NULL, "ShapeEx scene get at missing clears output") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, &gotOpacity) == XGE_OK, "ShapeEx scene opacity get") ) return 0;
		if ( !check((gotOpacity > 0.499f) && (gotOpacity < 0.501f), "ShapeEx scene opacity get value") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, &gotSceneTransform) == XGE_OK, "ShapeEx scene transform get") ) return 0;
		if ( !check((gotSceneTransform.fE == matrix.fE) && (gotSceneTransform.fF == matrix.fF), "ShapeEx scene transform get value") ) return 0;
		if ( !check(xgeShapeExSceneTransformIdentity(scene) == XGE_OK, "ShapeEx scene transform identity") ) return 0;
		if ( !check(xgeShapeExSceneTransformTranslate(scene, 3.0f, 4.0f) == XGE_OK, "ShapeEx scene transform translate") ) return 0;
		if ( !check(xgeShapeExSceneTransformScale(scene, 2.0f, 2.0f) == XGE_OK, "ShapeEx scene transform scale") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, &gotSceneTransform) == XGE_OK, "ShapeEx scene transform compose get") ) return 0;
		if ( !check((gotSceneTransform.fA == 2.0f) && (gotSceneTransform.fD == 2.0f) && (gotSceneTransform.fE == 3.0f) && (gotSceneTransform.fF == 4.0f), "ShapeEx scene transform compose values") ) return 0;
		if ( !check(xgeShapeExSceneTransformIdentity(scene) == XGE_OK, "ShapeEx scene transform rotate reset") ) return 0;
		if ( !check(xgeShapeExSceneTransformRotate(scene, 1.57079632679489661923f) == XGE_OK, "ShapeEx scene transform rotate") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, &gotSceneTransform) == XGE_OK, "ShapeEx scene transform rotate get") ) return 0;
		if ( !check((gotSceneTransform.fA > -0.001f) && (gotSceneTransform.fA < 0.001f) && (gotSceneTransform.fB > 0.999f) && (gotSceneTransform.fB < 1.001f) && (gotSceneTransform.fC > -1.001f) && (gotSceneTransform.fC < -0.999f), "ShapeEx scene transform rotate values") ) return 0;
		if ( !check(xgeShapeExSceneTransformIdentity(scene) == XGE_OK, "ShapeEx scene transform restore identity") ) return 0;
		if ( !check(xgeShapeExSceneTransformSet(scene, &matrix) == XGE_OK, "ShapeEx scene transform restore") ) return 0;
		if ( !check(xgeShapeExSceneTransformTranslate(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform translate rejects null") ) return 0;
		if ( !check(xgeShapeExSceneVisible(scene, 0) == XGE_OK, "ShapeEx scene visible false") ) return 0;
		if ( !check(xgeShapeExSceneVisibleGet(scene, &gotVisible) == XGE_OK, "ShapeEx scene visible get") ) return 0;
		if ( !check(gotVisible == 0, "ShapeEx scene visible get value") ) return 0;
		if ( !check(xgeShapeExSceneVisible(scene, 1) == XGE_OK, "ShapeEx scene visible restore") ) return 0;
		if ( !check(xgeShapeExSceneGetCount(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene count get rejects null") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene get at rejects null") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene opacity get rejects null") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform get rejects null") ) return 0;
	}
	if ( !check(xgeShapeExSceneDrawEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw ex rejects null scene") ) return 0;
	if ( !check(xgeShapeExSceneDrawPxEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw px ex rejects null scene") ) return 0;
	if ( !check(xgeShapeExSceneClear(scene) == XGE_OK, "ShapeEx scene clear") ) return 0;
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(shape);
	{
		xge_shape_ex obbShape = NULL;
		xge_shape_ex_scene obbScene = NULL;
		xge_vec2_t obb[4];
		xge_rect_t obbBounds;
		int intersects = -1;

		ret = xgeShapeExCreate(&obbShape);
		if ( !check((ret == XGE_OK) && (obbShape != NULL), "ShapeEx obb shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(obbShape, 0.0f, 0.0f, 10.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx obb rect") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		matrix.fA = 0.0f;
		matrix.fB = 1.0f;
		matrix.fC = -1.0f;
		matrix.fD = 0.0f;
		matrix.fE = 10.0f;
		matrix.fF = 20.0f;
		if ( !check(xgeShapeExTransformSet(obbShape, &matrix) == XGE_OK, "ShapeEx obb transform") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		ret = xgeShapeExGetOBB(obbShape, 0.05f, obb);
		if ( !check(ret == XGE_OK, "ShapeEx obb get") ||
		     !check((obb[0].fX > 9.99f) && (obb[0].fX < 10.01f) && (obb[0].fY > 19.99f) && (obb[0].fY < 20.01f), "ShapeEx obb point 0") ||
		     !check((obb[1].fX > 9.99f) && (obb[1].fX < 10.01f) && (obb[1].fY > 29.99f) && (obb[1].fY < 30.01f), "ShapeEx obb point 1") ||
		     !check((obb[2].fX > -10.01f) && (obb[2].fX < -9.99f) && (obb[2].fY > 29.99f) && (obb[2].fY < 30.01f), "ShapeEx obb point 2") ||
		     !check((obb[3].fX > -10.01f) && (obb[3].fX < -9.99f) && (obb[3].fY > 19.99f) && (obb[3].fY < 20.01f), "ShapeEx obb point 3") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(obbShape, 0.05f, &obbBounds);
		if ( !check((ret == XGE_OK) && (obbBounds.fX > -10.01f) && (obbBounds.fX < -9.99f) && (obbBounds.fY > 19.99f) && (obbBounds.fY < 20.01f) && (obbBounds.fW > 19.99f) && (obbBounds.fW < 20.01f) && (obbBounds.fH > 9.99f) && (obbBounds.fH < 10.01f), "ShapeEx obb bounds") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		if ( !check(xgeShapeExBoundsIntersects(obbShape, (xge_rect_t){-5.0f, 22.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK, "ShapeEx bounds intersects hit") ||
		     !check(intersects == 1, "ShapeEx bounds intersects hit value") ||
		     !check(xgeShapeExBoundsIntersects(obbShape, (xge_rect_t){20.0f, 20.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK, "ShapeEx bounds intersects miss") ||
		     !check(intersects == 0, "ShapeEx bounds intersects miss value") ||
		     !check(xgeShapeExBoundsIntersects(obbShape, (xge_rect_t){-5.0f, 22.0f, 0.0f, 2.0f}, 0.05f, &intersects) == XGE_OK, "ShapeEx bounds intersects empty rect") ||
		     !check(intersects == 0, "ShapeEx bounds intersects empty rect value") ||
		     !check(xgeShapeExBoundsIntersects(obbShape, (xge_rect_t){-5.0f, 22.0f, 2.0f, 2.0f}, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx bounds intersects rejects null output") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&obbScene);
		if ( !check((ret == XGE_OK) && (obbScene != NULL), "ShapeEx scene obb create") ) {
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneAdd(obbScene, obbShape) == XGE_OK, "ShapeEx scene obb add") ) {
			xgeShapeExSceneDestroy(obbScene);
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		matrix.fA = 1.0f;
		matrix.fB = 0.0f;
		matrix.fC = 0.0f;
		matrix.fD = 1.0f;
		matrix.fE = 100.0f;
		matrix.fF = 0.0f;
		if ( !check(xgeShapeExSceneTransformSet(obbScene, &matrix) == XGE_OK, "ShapeEx scene obb transform") ) {
			xgeShapeExSceneDestroy(obbScene);
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		ret = xgeShapeExSceneGetOBB(obbScene, 0.05f, obb);
		if ( !check(ret == XGE_OK, "ShapeEx scene obb get") ||
		     !check((obb[0].fX > 89.99f) && (obb[0].fX < 90.01f) && (obb[0].fY > 19.99f) && (obb[0].fY < 20.01f), "ShapeEx scene obb point 0") ||
		     !check((obb[2].fX > 109.99f) && (obb[2].fX < 110.01f) && (obb[2].fY > 29.99f) && (obb[2].fY < 30.01f), "ShapeEx scene obb point 2") ) {
			xgeShapeExSceneDestroy(obbScene);
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneBoundsIntersects(obbScene, (xge_rect_t){95.0f, 22.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK, "ShapeEx scene bounds intersects hit") ||
		     !check(intersects == 1, "ShapeEx scene bounds intersects hit value") ||
		     !check(xgeShapeExSceneBoundsIntersects(obbScene, (xge_rect_t){0.0f, 0.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK, "ShapeEx scene bounds intersects miss") ||
		     !check(intersects == 0, "ShapeEx scene bounds intersects miss value") ||
		     !check(xgeShapeExSceneGetOBB(obbScene, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene obb rejects null output") ) {
			xgeShapeExSceneDestroy(obbScene);
			xgeShapeExDestroy(obbShape);
			return 0;
		}
		xgeShapeExSceneDestroy(obbScene);
		xgeShapeExDestroy(obbShape);
	}
	{
		xge_shape_ex strokeShape = NULL;
		xge_rect_t strokeBounds;
		int strokeHit = -1;

		ret = xgeShapeExCreate(&strokeShape);
		if ( !check((ret == XGE_OK) && (strokeShape != NULL), "ShapeEx stroke bounds create") ) return 0;
		if ( !check(xgeShapeExAppendRect(strokeShape, 10.0f, 10.0f, 20.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx stroke bounds rect") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeColor(strokeShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx stroke bounds color") ||
		     !check(xgeShapeExStrokeWidth(strokeShape, 4.0f) == XGE_OK, "ShapeEx stroke bounds width") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(strokeShape, 0.05f, &strokeBounds);
		if ( !check((ret == XGE_OK) &&
		            (strokeBounds.fX > 7.99f) && (strokeBounds.fX < 8.01f) &&
		            (strokeBounds.fY > 7.99f) && (strokeBounds.fY < 8.01f) &&
		            (strokeBounds.fW > 23.99f) && (strokeBounds.fW < 24.01f) &&
		            (strokeBounds.fH > 13.99f) && (strokeBounds.fH < 14.01f), "ShapeEx stroke expands bounds") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		if ( !check(xgeShapeExMatrixScale(&matrix, 2.0f, 3.0f) == XGE_OK, "ShapeEx stroke bounds scale matrix") ||
		     !check(xgeShapeExTransformSet(strokeShape, &matrix) == XGE_OK, "ShapeEx stroke bounds transform") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(strokeShape, 0.05f, &strokeBounds);
		if ( !check((ret == XGE_OK) &&
		            (strokeBounds.fX > 15.09f) && (strokeBounds.fX < 15.12f) &&
		            (strokeBounds.fY > 25.09f) && (strokeBounds.fY < 25.12f) &&
		            (strokeBounds.fW > 49.78f) && (strokeBounds.fW < 49.82f) &&
		            (strokeBounds.fH > 39.78f) && (strokeBounds.fH < 39.82f), "ShapeEx stroke bounds scales by transform") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeNonScaling(strokeShape, 1) == XGE_OK, "ShapeEx non-scaling stroke bounds mode") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(strokeShape, 0.05f, &strokeBounds);
		if ( !check((ret == XGE_OK) &&
		            (strokeBounds.fX > 17.99f) && (strokeBounds.fX < 18.01f) &&
		            (strokeBounds.fY > 27.99f) && (strokeBounds.fY < 28.01f) &&
		            (strokeBounds.fW > 43.99f) && (strokeBounds.fW < 44.01f) &&
		            (strokeBounds.fH > 33.99f) && (strokeBounds.fH < 34.01f), "ShapeEx non-scaling stroke bounds") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		if ( !check(xgeShapeExBoundsIntersects(strokeShape, (xge_rect_t){18.5f, 29.0f, 1.0f, 1.0f}, 0.05f, &strokeHit) == XGE_OK, "ShapeEx stroke bounds intersects") ||
		     !check(strokeHit == 1, "ShapeEx stroke bounds intersects value") ) {
			xgeShapeExDestroy(strokeShape);
			return 0;
		}
		xgeShapeExDestroy(strokeShape);
	}
	{
		xge_shape_ex sceneA = NULL;
		xge_shape_ex sceneB = NULL;
		xge_shape_ex sceneC = NULL;
		xge_shape_ex sceneMissing = NULL;
		xge_shape_ex_scene orderScene = NULL;
		xge_rect_t orderBounds;

		ret = xgeShapeExCreate(&sceneA);
		if ( !check((ret == XGE_OK) && (sceneA != NULL), "ShapeEx scene insert A create") ) return 0;
		ret = xgeShapeExCreate(&sceneB);
		if ( !check((ret == XGE_OK) && (sceneB != NULL), "ShapeEx scene insert B create") ) {
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		ret = xgeShapeExCreate(&sceneC);
		if ( !check((ret == XGE_OK) && (sceneC != NULL), "ShapeEx scene insert C create") ) {
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		ret = xgeShapeExCreate(&sceneMissing);
		if ( !check((ret == XGE_OK) && (sceneMissing != NULL), "ShapeEx scene insert missing create") ) {
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(sceneA, 0.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene insert A rect") ||
		     !check(xgeShapeExAppendRect(sceneB, 20.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene insert B rect") ||
		     !check(xgeShapeExAppendRect(sceneC, 40.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene insert C rect") ||
		     !check(xgeShapeExAppendRect(sceneMissing, 80.0f, 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene insert missing rect") ) {
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&orderScene);
		if ( !check((ret == XGE_OK) && (orderScene != NULL), "ShapeEx scene insert create") ) {
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		if ( !check(xgeShapeExSceneAdd(orderScene, sceneA) == XGE_OK, "ShapeEx scene insert add A") ||
		     !check(xgeShapeExSceneAdd(orderScene, sceneC) == XGE_OK, "ShapeEx scene insert add C") ||
		     !check(xgeShapeExSceneInsert(orderScene, sceneB, sceneC) == XGE_OK, "ShapeEx scene insert B before C") ||
		     !check(xgeShapeExSceneInsert(orderScene, sceneA, sceneA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene insert rejects same shape") ||
		     !check(xgeShapeExSceneInsert(orderScene, sceneMissing, sceneMissing) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene insert rejects same missing shape") ||
		     !check(xgeShapeExSceneInsert(orderScene, sceneB, sceneMissing) == XGE_ERROR_NOT_FOUND, "ShapeEx scene insert before missing") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		{
			xge_shape_ex orderItem = NULL;
			int orderCount = 0;

			if ( !check(xgeShapeExSceneGetCount(orderScene, &orderCount) == XGE_OK, "ShapeEx scene insert count get") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
			if ( !check(orderCount == 3, "ShapeEx scene insert count value") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
			if ( !check((xgeShapeExSceneGetAt(orderScene, 0, &orderItem) == XGE_OK) && (orderItem == sceneA), "ShapeEx scene insert get at A") ||
			     !check((xgeShapeExSceneGetAt(orderScene, 1, &orderItem) == XGE_OK) && (orderItem == sceneB), "ShapeEx scene insert get at B") ||
			     !check((xgeShapeExSceneGetAt(orderScene, 2, &orderItem) == XGE_OK) && (orderItem == sceneC), "ShapeEx scene insert get at C") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
			if ( !check(xgeShapeExSceneGetAt(orderScene, -1, &orderItem) == XGE_ERROR_NOT_FOUND, "ShapeEx scene insert get at negative missing") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
		}
		ret = xgeShapeExSceneGetBounds(orderScene, 0.05f, &orderBounds);
		if ( !check((ret == XGE_OK) && (orderBounds.fX > -0.1f) && (orderBounds.fX < 0.1f) && (orderBounds.fW > 41.9f) && (orderBounds.fW < 42.1f), "ShapeEx scene insert bounds") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		if ( !check(xgeShapeExSceneRemove(orderScene, sceneC) == XGE_OK, "ShapeEx scene remove C") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		{
			int orderCount = 0;

			if ( !check((xgeShapeExSceneGetCount(orderScene, &orderCount) == XGE_OK) && (orderCount == 2), "ShapeEx scene remove count value") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
		}
		ret = xgeShapeExSceneGetBounds(orderScene, 0.05f, &orderBounds);
		if ( !check((ret == XGE_OK) && (orderBounds.fW > 21.9f) && (orderBounds.fW < 22.1f), "ShapeEx scene remove bounds") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		if ( !check(xgeShapeExSceneRemove(orderScene, sceneC) == XGE_ERROR_NOT_FOUND, "ShapeEx scene remove missing") ||
		     !check(xgeShapeExSceneRemove(orderScene, NULL) == XGE_OK, "ShapeEx scene remove null clears") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		ret = xgeShapeExSceneGetBounds(orderScene, 0.05f, &orderBounds);
		if ( !check((ret == XGE_OK) && (orderBounds.fW == 0.0f) && (orderBounds.fH == 0.0f), "ShapeEx scene remove null cleared bounds") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		{
			int orderCount = -1;

			if ( !check((xgeShapeExSceneGetCount(orderScene, &orderCount) == XGE_OK) && (orderCount == 0), "ShapeEx scene remove null cleared count") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
		}
		xgeShapeExSceneDestroy(orderScene);
		xgeShapeExDestroy(sceneMissing);
		xgeShapeExDestroy(sceneC);
		xgeShapeExDestroy(sceneB);
		xgeShapeExDestroy(sceneA);
	}
	{
		xge_shape_ex boundsShape = NULL;
		xge_shape_ex_scene boundsScene = NULL;
		xge_shape_ex_scene boundsClone = NULL;
		xge_rect_t sceneBounds;

		ret = xgeShapeExCreate(&boundsShape);
		if ( !check((ret == XGE_OK) && (boundsShape != NULL), "ShapeEx bounds shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(boundsShape, 0.0f, 0.0f, 10.0f, 6.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx bounds rect") ) {
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		matrix.fA = 1.0f;
		matrix.fB = 0.0f;
		matrix.fC = 0.0f;
		matrix.fD = 1.0f;
		matrix.fE = 5.0f;
		matrix.fF = 7.0f;
		if ( !check(xgeShapeExTransformSet(boundsShape, &matrix) == XGE_OK, "ShapeEx bounds shape transform") ) {
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(boundsShape, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) && (bounds.fX > 4.9f) && (bounds.fX < 5.1f) && (bounds.fY > 6.9f) && (bounds.fY < 7.1f) && (bounds.fW > 9.9f) && (bounds.fW < 10.1f), "ShapeEx transformed bounds") ) {
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&boundsScene);
		if ( !check((ret == XGE_OK) && (boundsScene != NULL), "ShapeEx bounds scene create") ) {
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneAdd(boundsScene, boundsShape) == XGE_OK, "ShapeEx bounds scene add") ) {
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		matrix.fE = 20.0f;
		matrix.fF = 30.0f;
		if ( !check(xgeShapeExSceneTransformSet(boundsScene, &matrix) == XGE_OK, "ShapeEx bounds scene transform") ) {
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		ret = xgeShapeExSceneGetBounds(boundsScene, 0.05f, &sceneBounds);
		if ( !check((ret == XGE_OK) && (sceneBounds.fX > 24.9f) && (sceneBounds.fX < 25.1f) && (sceneBounds.fY > 36.9f) && (sceneBounds.fY < 37.1f) && (sceneBounds.fW > 9.9f) && (sceneBounds.fW < 10.1f), "ShapeEx scene transformed bounds") ) {
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		ret = xgeShapeExSceneClone(boundsScene, &boundsClone);
		if ( !check((ret == XGE_OK) && (boundsClone != NULL), "ShapeEx scene clone") ) {
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneClear(boundsScene) == XGE_OK, "ShapeEx cloned source clear") ) {
			xgeShapeExSceneDestroy(boundsClone);
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		ret = xgeShapeExSceneGetBounds(boundsClone, 0.05f, &sceneBounds);
		if ( !check((ret == XGE_OK) && (sceneBounds.fX > 24.9f) && (sceneBounds.fX < 25.1f) && (sceneBounds.fY > 36.9f) && (sceneBounds.fY < 37.1f), "ShapeEx scene clone keeps bounds") ) {
			xgeShapeExSceneDestroy(boundsClone);
			xgeShapeExSceneDestroy(boundsScene);
			xgeShapeExDestroy(boundsShape);
			return 0;
		}
		xgeShapeExSceneDestroy(boundsClone);
		xgeShapeExSceneDestroy(boundsScene);
		xgeShapeExDestroy(boundsShape);
	}
	{
		xge_shape_ex raw = NULL;
		xge_shape_ex invalidPath = NULL;
		const uint8_t rawCommands[5] = {
			XGE_SHAPE_EX_CMD_CLOSE,
			XGE_SHAPE_EX_CMD_MOVE_TO,
			XGE_SHAPE_EX_CMD_LINE_TO,
			XGE_SHAPE_EX_CMD_CUBIC_TO,
			XGE_SHAPE_EX_CMD_CLOSE
		};
		const xge_vec2_t rawPoints[5] = {
			{100.0f, 100.0f},
			{200.0f, 200.0f},
			{10.0f, 10.0f},
			{20.0f, 20.0f},
			{30.0f, 30.0f}
		};
		const uint8_t* gotCommands = NULL;
		const xge_vec2_t* gotPoints = NULL;
		int gotCommandCount = -1;
		int gotPointCount = -1;
		int i;

		ret = xgeShapeExCreate(&raw);
		if ( !check((ret == XGE_OK) && (raw != NULL), "ShapeEx raw path create") ) return 0;
		if ( !check(xgeShapeExAppendPath(raw, NULL, 0, NULL, 0) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects null") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 0, rawPoints, 5) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects zero commands") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 5, rawPoints, 0) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects zero points") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 5, rawPoints, 4) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects point mismatch") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 5, rawPoints, 5) == XGE_OK, "ShapeEx raw path append") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(raw, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommands != NULL) && (gotPoints != NULL) && (gotCommandCount == 5) && (gotPointCount == 5), "ShapeEx raw path get") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExCreate(&invalidPath);
		if ( !check((ret == XGE_OK) && (invalidPath != NULL), "ShapeEx invalid svg path create") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "Mnan 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects nan path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "Minf 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects inf path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M1e500 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects overflow path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(invalidPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 0) && (gotPointCount == 0), "ShapeEx invalid path leaves empty shape") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M0 0 L2 0") == XGE_OK, "ShapeEx rollback baseline path") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(invalidPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 2) && (gotPointCount == 2), "ShapeEx rollback baseline counts") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M4 4 Lnan 5") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx invalid path rolls back partial append") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(invalidPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 2) && (gotPointCount == 2) && (gotPoints[0].fX == 0.0f) && (gotPoints[1].fX == 2.0f), "ShapeEx invalid path preserves previous path") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		xgeShapeExDestroy(invalidPath);
		ret = xgeShapeExGetPath(raw, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommands != NULL) && (gotPoints != NULL) && (gotCommandCount == 5) && (gotPointCount == 5), "ShapeEx raw path get after invalid path") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		for ( i = 0; i < 5; i++ ) {
			if ( !check(gotCommands[i] == rawCommands[i], "ShapeEx raw path command value") ) {
				xgeShapeExDestroy(raw);
				return 0;
			}
			if ( !check((gotPoints[i].fX == rawPoints[i].fX) && (gotPoints[i].fY == rawPoints[i].fY), "ShapeEx raw path point value") ) {
				xgeShapeExDestroy(raw);
				return 0;
			}
		}
		ret = xgeShapeExGetBounds(raw, 0.25f, &bounds);
		if ( !check((ret == XGE_OK) && (bounds.fW > 0.0f) && (bounds.fH > 0.0f), "ShapeEx raw path bounds") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExReset(raw) == XGE_OK, "ShapeEx raw path reset") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(raw, NULL, &gotCommandCount, NULL, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 0) && (gotPointCount == 0), "ShapeEx raw path reset counts") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		xgeShapeExDestroy(raw);
	}
	{
		xge_shape_ex atomic = NULL;
		int commandCountBefore = -1;
		int pointCountBefore = -1;
		int commandCountAfter = -1;
		int pointCountAfter = -1;

		ret = xgeShapeExCreate(&atomic);
		if ( !check((ret == XGE_OK) && (atomic != NULL), "ShapeEx atomic append create") ) return 0;
		if ( !check(xgeShapeExAppendSvgPath(atomic, "M0 0 L10 0") == XGE_OK, "ShapeEx atomic append baseline") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		ret = xgeShapeExGetPath(atomic, NULL, &commandCountBefore, NULL, &pointCountBefore);
		if ( !check((ret == XGE_OK) && (commandCountBefore == 2) && (pointCountBefore == 2), "ShapeEx atomic append baseline counts") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(atomic, 0.0f, 0.0f, -1.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx rect accepts ThorVG-compatible negative size") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendCircle(atomic, 0.0f, 0.0f, -2.0f, 3.0f, 1) == XGE_OK, "ShapeEx circle accepts ThorVG-compatible negative radius") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		ret = xgeShapeExGetPath(atomic, NULL, &commandCountBefore, NULL, &pointCountBefore);
		if ( !check((ret == XGE_OK) && (commandCountBefore > 2) && (pointCountBefore > 2), "ShapeEx ThorVG-compatible primitive append counts") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendCapsule(atomic, 0.0f, 0.0f, -1.0f, 4.0f, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic capsule rejects invalid size") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendArc(atomic, 0.0f, 0.0f, -2.0f, 3.0f, 0.0f, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic arc rejects invalid radius") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendPie(atomic, 0.0f, 0.0f, 2.0f, -3.0f, 0.0f, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic pie rejects invalid radius") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendChord(atomic, 0.0f, 0.0f, -2.0f, 3.0f, 0.0f, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic chord rejects invalid radius") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		ret = xgeShapeExGetPath(atomic, NULL, &commandCountAfter, NULL, &pointCountAfter);
		if ( !check((ret == XGE_OK) && (commandCountAfter == commandCountBefore) && (pointCountAfter == pointCountBefore), "ShapeEx atomic append preserves path after invalid primitives") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		xgeShapeExDestroy(atomic);
	}
	{
		xge_shape_ex line = NULL;
		float lineLength = 0.0f;
		xge_vec2_t samplePoint;
		xge_vec2_t sampleTangent;

		ret = xgeShapeExCreate(&line);
		if ( !check((ret == XGE_OK) && (line != NULL), "ShapeEx length line create") ) return 0;
		if ( !check(xgeShapeExMoveTo(line, 0.0f, 0.0f) == XGE_OK, "ShapeEx length line moveTo") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		if ( !check(xgeShapeExLineTo(line, 3.0f, 4.0f) == XGE_OK, "ShapeEx length line lineTo") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetLength(line, 0.05f, &lineLength);
		if ( !check((ret == XGE_OK) && (lineLength > 4.99f) && (lineLength < 5.01f), "ShapeEx line length") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetPointAtLength(line, 2.5f, 0.05f, &samplePoint, &sampleTangent);
		if ( !check((ret == XGE_OK) &&
		            (samplePoint.fX > 1.49f) && (samplePoint.fX < 1.51f) &&
		            (samplePoint.fY > 1.99f) && (samplePoint.fY < 2.01f) &&
		            (sampleTangent.fX > 0.59f) && (sampleTangent.fX < 0.61f) &&
		            (sampleTangent.fY > 0.79f) && (sampleTangent.fY < 0.81f), "ShapeEx point at length") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetPointAtLength(line, 50.0f, 0.05f, &samplePoint, &sampleTangent);
		if ( !check((ret == XGE_OK) &&
		            (samplePoint.fX > 2.99f) && (samplePoint.fX < 3.01f) &&
		            (samplePoint.fY > 3.99f) && (samplePoint.fY < 4.01f), "ShapeEx point at length clamp") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		if ( !check(xgeShapeExTrimPath(line, 0.25f, 0.75f, 1) == XGE_OK, "ShapeEx trim path") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetBounds(line, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) &&
		            (bounds.fX > 0.74f) && (bounds.fX < 0.76f) &&
		            (bounds.fY > 0.99f) && (bounds.fY < 1.01f) &&
		            (bounds.fW > 1.49f) && (bounds.fW < 1.51f) &&
		            (bounds.fH > 1.99f) && (bounds.fH < 2.01f), "ShapeEx trim bounds") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		{
			xge_shape_ex clone = NULL;
			ret = xgeShapeExClone(line, &clone);
			if ( !check((ret == XGE_OK) && (clone != NULL), "ShapeEx trim clone") ) {
				xgeShapeExDestroy(line);
				return 0;
			}
			ret = xgeShapeExGetBounds(clone, 0.05f, &bounds);
			if ( !check((ret == XGE_OK) &&
			            (bounds.fX > 0.74f) && (bounds.fX < 0.76f) &&
			            (bounds.fW > 1.49f) && (bounds.fW < 1.51f), "ShapeEx trim clone bounds") ) {
				xgeShapeExDestroy(clone);
				xgeShapeExDestroy(line);
				return 0;
			}
			xgeShapeExDestroy(clone);
		}
		if ( !check(xgeShapeExTrimClear(line) == XGE_OK, "ShapeEx trim clear") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetBounds(line, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) &&
		            (bounds.fX > -0.01f) && (bounds.fX < 0.01f) &&
		            (bounds.fY > -0.01f) && (bounds.fY < 0.01f) &&
		            (bounds.fW > 2.99f) && (bounds.fW < 3.01f) &&
		            (bounds.fH > 3.99f) && (bounds.fH < 4.01f), "ShapeEx trim clear bounds") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		xgeShapeExDestroy(line);
	}
	{
		xge_shape_ex multi = NULL;

		ret = xgeShapeExCreate(&multi);
		if ( !check((ret == XGE_OK) && (multi != NULL), "ShapeEx trim combined create") ) return 0;
		if ( !check(xgeShapeExMoveTo(multi, 0.0f, 0.0f) == XGE_OK, "ShapeEx trim combined move 1") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		if ( !check(xgeShapeExLineTo(multi, 10.0f, 0.0f) == XGE_OK, "ShapeEx trim combined line 1") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		if ( !check(xgeShapeExMoveTo(multi, 20.0f, 0.0f) == XGE_OK, "ShapeEx trim combined move 2") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		if ( !check(xgeShapeExLineTo(multi, 30.0f, 0.0f) == XGE_OK, "ShapeEx trim combined line 2") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		if ( !check(xgeShapeExTrimPath(multi, 0.25f, 0.75f, 0) == XGE_OK, "ShapeEx trim combined") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		ret = xgeShapeExGetBounds(multi, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) &&
		            (bounds.fX > 4.99f) && (bounds.fX < 5.01f) &&
		            (bounds.fW > 19.99f) && (bounds.fW < 20.01f), "ShapeEx trim combined bounds") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		xgeShapeExDestroy(multi);
	}
	{
		xge_shape_ex smooth = NULL;
		xge_rect_t smoothBounds;

		ret = xgeShapeExCreate(&smooth);
		if ( !check((ret == XGE_OK) && (smooth != NULL), "ShapeEx smooth create") ) return 0;
		ret = xgeShapeExAppendSvgPath(smooth, "M0 0 T10 10 20 0");
		if ( !check(ret == XGE_OK, "ShapeEx repeated T parse") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		ret = xgeShapeExGetBounds(smooth, 0.05f, &smoothBounds);
		if ( !check((ret == XGE_OK) && (smoothBounds.fH > 12.0f), "ShapeEx repeated T reflection") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		xgeShapeExDestroy(smooth);
	}
	{
		xge_shape_ex arc = NULL;
		xge_shape_ex arcDegenerate = NULL;
		xge_rect_t arcBounds;
		float lineLength;
		int commandCountBefore = 0;
		int commandCountAfter = 0;
		int pointCountBefore = 0;
		int pointCountAfter = 0;
		const float pi = 3.14159265358979323846f;

		ret = xgeShapeExCreate(&arc);
		if ( !check((ret == XGE_OK) && (arc != NULL), "ShapeEx arc create") ) return 0;
		ret = xgeShapeExCreate(&arcDegenerate);
		if ( !check((ret == XGE_OK) && (arcDegenerate != NULL), "ShapeEx degenerate arc create") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExArcTo(arc, 6.0f, 4.0f, 0.0f, 0, 1, 12.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx arc requires current point") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExMoveTo(arcDegenerate, 0.0f, 0.0f) == XGE_OK, "ShapeEx degenerate arc moveTo") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExGetPath(arcDegenerate, NULL, &commandCountBefore, NULL, &pointCountBefore);
		if ( !check(xgeShapeExArcTo(arcDegenerate, 6.0f, 4.0f, 0.0f, 0, 1, 0.0f, 0.0f) == XGE_OK, "ShapeEx same-point arc no-op") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExGetPath(arcDegenerate, NULL, &commandCountAfter, NULL, &pointCountAfter);
		if ( !check((commandCountAfter == commandCountBefore) && (pointCountAfter == pointCountBefore), "ShapeEx same-point arc keeps path") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExArcTo(arcDegenerate, 0.0f, 4.0f, 0.0f, 0, 1, 4.0f, 0.0f) == XGE_OK, "ShapeEx zero-radius arc lineTo") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		ret = xgeShapeExGetLength(arcDegenerate, 0.05f, &lineLength);
		if ( !check((ret == XGE_OK) && (lineLength > 3.9f) && (lineLength < 4.1f), "ShapeEx zero-radius arc line length") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExDestroy(arcDegenerate);
		ret = xgeShapeExCreate(&arcDegenerate);
		if ( !check((ret == XGE_OK) && (arcDegenerate != NULL), "ShapeEx svg degenerate arc create") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A0 4 0 0 1 4 0 A4 4 0 0 1 4 0") == XGE_OK, "ShapeEx svg degenerate arc parse") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		ret = xgeShapeExGetLength(arcDegenerate, 0.05f, &lineLength);
		if ( !check((ret == XGE_OK) && (lineLength > 3.9f) && (lineLength < 4.1f), "ShapeEx svg degenerate arc length") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 1020 0") == XGE_OK, "ShapeEx svg arc accepts adjacent flags") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 2 0 20 0") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx svg arc rejects invalid large-arc flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 1 2 20 0") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx svg arc rejects invalid sweep flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 1.0 0 20 0") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx svg arc rejects decimal flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExDestroy(arcDegenerate);
		if ( !check(xgeShapeExMoveTo(arc, 0.0f, 0.0f) == XGE_OK, "ShapeEx arc moveTo") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExArcTo(arc, 6.0f, 4.0f, 25.0f, 0, 1, 12.0f, 0.0f) == XGE_OK, "ShapeEx arcTo") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		ret = xgeShapeExGetBounds(arc, 0.05f, &arcBounds);
		if ( !check((ret == XGE_OK) && (arcBounds.fW > 11.5f) && (arcBounds.fH > 1.0f), "ShapeEx arc bounds") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendArc(arc, 0.0f, 24.0f, 8.0f, 5.0f, 0.0f, pi) == XGE_OK, "ShapeEx append arc") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendPie(arc, 24.0f, 24.0f, 8.0f, 6.0f, 0.0f, pi * 0.5f) == XGE_OK, "ShapeEx append pie") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendChord(arc, 48.0f, 24.0f, 8.0f, 6.0f, 0.0f, -pi * 0.5f) == XGE_OK, "ShapeEx append chord") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExMoveTo(arc, 64.0f, 24.0f) == XGE_OK, "ShapeEx zero stroke moveTo") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExLineTo(arc, 64.0f, 24.0f) == XGE_OK, "ShapeEx zero stroke lineTo") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendArc(arc, 0.0f, 0.0f, -1.0f, 5.0f, 0.0f, pi) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx append arc rejects negative radius") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		ret = xgeShapeExGetBounds(arc, 0.05f, &arcBounds);
		if ( !check((ret == XGE_OK) && (arcBounds.fW > 55.0f) && (arcBounds.fH > 29.0f), "ShapeEx arc primitives bounds") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		xgeShapeExDestroy(arc);
	}
	return 1;
}

static int test_svg(void)
{
#define XGE_TEST_RASTER_PNG "iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFElEQVR42mP4z8DwHwyBNBAw/AcAR8oI+FuapL4AAAAASUVORK5CYII="
	static const char svg_text[] =
		"<svg viewBox=\"0,0,32,16\" preserveAspectRatio=\"xMinYMax meet\">"
		"<style>.smokeAccent{fill:#010203;stroke:none}#smokeId{fill:#040506;stroke:none}polygon{fill:#070809;stroke:none}</style>"
		"<style>.smokeCurrent{color:goldenrod;fill:currentColor;stroke:transparent}</style>"
		"<style>path.smokeCompound{fill:#0a0b0c;stroke:none}.smokePair.extra{fill:#0d0e0f;stroke:none}#smokeId2.extra{fill:#101112;stroke:none}</style>"
		"<style>stop.smokeStop{stop-color:#19c37d;stop-opacity:100%}</style>"
		"<style><![CDATA[/* xge smoke */@font-face{font-family:ignore;src:url(ignore)}path.smokeCData{fill:/*inline*/#131415;stroke:none}@media screen{.ignored{fill:red}}]]></style>"
		"<style>path.smokeSpecific{fill:#161718;stroke:none}.smokeSpecific{fill:#000000;stroke:none}.smokeAttr{fill:#191a1b;stroke:none}</style>"
		"<style>*.smokeUniversal{fill:#1c1d1e;stroke:none}</style>"
		"<style>.smokeImportant{fill:#010101;stroke:none}.smokeImportant{fill:#2d2e2f !important}.smokeImportant{fill:#ff0000}.smokeLast{fill:#010101;fill:#323334;stroke:none}</style>"
		"<style>g.smokeScope path.smokeDesc{fill:#515253;stroke:none}g.smokeScope>path.smokeChild{fill:#545556;stroke:none}defs linearGradient>stop.smokeScopedStop{stop-color:#575859;stop-opacity:100%}</style>"
		"<style>rect[probe]{fill:#5a5b5c;stroke:none}rect[data-eq=match]{fill:#5d5e5f;stroke:none}rect[data-word~=beta]{fill:#606162;stroke:none}rect[lang|=en]{fill:#636465;stroke:none}rect[data-prefix^=pre]{fill:#666768;stroke:none}rect[data-suffix$=end]{fill:#696a6b;stroke:none}rect[data-sub*=mid]{fill:#6c6d6e;stroke:none}</style>"
		"<style>.smokeFontEm{font-size:2px;fill:#6f7071;stroke:none}.smokeFontEx{font-size:4px;fill:#727374;stroke:none}</style>"
		"<style>.smokeNotProbe:not(.smokeNotSkip){fill:#767778;stroke:none}.smokeNotProbe.smokeNotSkip{fill:#ff0000;stroke:none}.smokeIsProbe:is(.smokeIsMatch, .smokeUnused){fill:#7a7b7c;stroke:none}.smokeWhereProbe:where(.smokeWhereMatch, .smokeUnused){fill:#7d7e7f;stroke:none}.smokeSiblingPrev+.smokeAdjacentProbe{fill:#808182;stroke:none}.smokeSiblingBase~.smokeGeneralProbe{fill:#838485;stroke:none}.smokeFirstChildProbe:first-child{fill:#868788;stroke:none}.smokeFirstTypeProbe:first-of-type{fill:#898a8b;stroke:none}.smokeNthChildProbe:nth-child(3){fill:#8c8d8e;stroke:none}.smokeNthTypeProbe:nth-of-type(2n){fill:#8f9091;stroke:none}.smokeLastChildProbe:last-child{fill:#929394;stroke:none}.smokeLastTypeProbe:last-of-type{fill:#959697;stroke:none}.smokeOnlyChildProbe:only-child{fill:#98999a;stroke:none}.smokeOnlyTypeProbe:only-of-type{fill:#9b9c9d;stroke:none}.smokeNthLastChildProbe:nth-last-child(2){fill:#9e9fa0;stroke:none}.smokeNthLastTypeProbe:nth-last-of-type(even){fill:#a1a2a3;stroke:none}.smokeEmptyProbe:empty{fill:#a4a5a6;stroke:none}.smokeHasDescProbe:has(path.smokeHasNeedle){fill:#a7a8a9;stroke:none}.smokeHasChildProbe:has(>path.smokeHasChildNeedle){fill:#aaabac;stroke:none}.smokeHasAdjacentProbe:has(+path.smokeHasAdjacentNeedle){fill:#adaeaf;stroke:none}</style>"
		"<defs>"
		"<linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
		"<stop offset=\"0%\" stop-color=\"#ff0000\"/>"
		"<stop offset=\"100%\" style=\"stop-color:#0000ff;stop-opacity:0.8\"/>"
		"</linearGradient>"
		"<radialGradient id=\"rad\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"35%\" fy=\"35%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.95 0.95) skewX(4)\">"
		"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
		"<stop offset=\"100%\" stop-color=\"#112233\"/>"
		"</radialGradient>"
		"<linearGradient id=\"gradAlias\" href=\"#lateStops\" x1=\"100%\" y1=\"0%\" x2=\"0%\" y2=\"100%\"/>"
		"<linearGradient id=\"gradChain\" xlink:href=\"#gradAlias\" spreadMethod=\"repeat\"/>"
		"<linearGradient id=\"lateStops\"><stop offset=\"0%\" stop-color=\"#102030\"/><stop offset=\"100%\" stop-color=\"#f0c020\"/></linearGradient>"
		"<linearGradient id=\"currentStopGrad\" color=\"#abcdef\"><stop offset=\"0\" stop-color=\"currentColor\"/><stop offset=\"1\" style=\"stop-color:currentColor\"/></linearGradient>"
		"<linearGradient id=\"cssStopGrad\"><stop class=\"smokeStop\" offset=\"0\"/><stop class=\"smokeStop\" offset=\"1\"/></linearGradient>"
		"<linearGradient id=\"scopedStopGrad\"><stop class=\"smokeScopedStop\" offset=\"0\"/><stop class=\"smokeScopedStop\" offset=\"1\"/></linearGradient>"
		"<radialGradient id=\"radAlias\" href=\"#lateRadStops\" cx=\"50%\" cy=\"50%\" r=\"60%\"/>"
		"<radialGradient id=\"lateRadStops\"><stop offset=\"0%\" stop-color=\"#ffffff\"/><stop offset=\"100%\" stop-color=\"#203040\"/></radialGradient>"
		"<radialGradient id=\"crossRadStops\" spreadMethod=\"reflect\"><stop offset=\"0%\" stop-color=\"#ff00aa\"/><stop offset=\"100%\" stop-color=\"#00ffaa\"/></radialGradient>"
		"<linearGradient id=\"linearFromRad\" href=\"#crossRadStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\"/>"
		"<linearGradient id=\"crossLinStops\" spreadMethod=\"repeat\"><stop offset=\"0%\" stop-color=\"#ffcc00\"/><stop offset=\"100%\" stop-color=\"#0044ff\"/></linearGradient>"
		"<radialGradient id=\"radFromLinear\" href=\"#crossLinStops\" cx=\"50%\" cy=\"50%\" r=\"50%\"/>"
		"<linearGradient id=\"userLengthGrad\" gradientUnits=\"userSpaceOnUse\" x1=\"24px\" y1=\"4px\" x2=\"32px\" y2=\"4px\"><stop offset=\"0%\" stop-color=\"#11aaee\"/><stop offset=\"100%\" stop-color=\"#ee44aa\"/></linearGradient>"
		"<radialGradient id=\"userLengthRad\" gradientUnits=\"userSpaceOnUse\" cx=\"30px\" cy=\"5px\" r=\"2px\" fx=\"29px\" fy=\"4px\"><stop offset=\"0%\" stop-color=\"#ffffff\"/><stop offset=\"100%\" stop-color=\"#111111\"/></radialGradient>"
		"<pattern id=\"smokePattern\" patternUnits=\"userSpaceOnUse\" width=\"1\" height=\"1\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"1\" fill=\"#0abcde\" stroke=\"none\"/></pattern>"
		"<pattern id=\"smokeMixedPattern\" patternUnits=\"userSpaceOnUse\" width=\"4\" height=\"4\"><rect x=\"0\" y=\"0\" width=\"4\" height=\"4\" fill=\"#f8fafc\" stroke=\"none\"/><text x=\"0.5\" y=\"3\" font-size=\"2\" fill=\"#ff31c7\" stroke=\"none\">T</text><image x=\"2\" y=\"0\" width=\"2\" height=\"2\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 2 2' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='2' height='2' fill='%23f97316'/%3E%3C/svg%3E\"/><image x=\"0\" y=\"2\" width=\"2\" height=\"2\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/></pattern>"
		"<pattern id=\"smokeHrefPattern\" xlink:href=\"#smokeMixedPattern\" patternTransform=\"translate(0.25 0.25)\"/>"
		"<pattern id=\"smokeBoxPercentPattern\" patternUnits=\"objectBoundingBox\" patternContentUnits=\"objectBoundingBox\" width=\"50%\" height=\"50%\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"0.5\" fill=\"#13b8a6\" stroke=\"none\"/></pattern>"
		"<pattern id=\"smokeStrokePattern\" patternUnits=\"userSpaceOnUse\" width=\"2\" height=\"2\"><rect x=\"0\" y=\"0\" width=\"1\" height=\"2\" fill=\"#e879f9\" stroke=\"none\"/></pattern>"
		"<clipPath id=\"smokeClip\"><rect x=\"24\" y=\"12\" width=\"4\" height=\"3\"/></clipPath>"
		"<clipPath id=\"smokeBoxClip\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.1 0)\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"1\"/></g></clipPath>"
		"<clipPath id=\"smokeRuleClip\" clip-rule=\"evenodd\"><g><path d=\"M0 0 H4 V4 H0 Z M1 1 H3 V3 H1 Z\"/></g></clipPath>"
		"<clipPath id=\"smokeMultiClip\"><rect x=\"2\" y=\"20\" width=\"2\" height=\"1\"/><rect x=\"6\" y=\"20\" width=\"2\" height=\"1\"/></clipPath>"
		"<clipPath id=\"smokeOverlapClip\"><rect x=\"12\" y=\"20\" width=\"4\" height=\"1\"/><rect x=\"14\" y=\"20\" width=\"4\" height=\"1\"/></clipPath>"
		"<mask id=\"smokeMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"1\" fill=\"white\" opacity=\"0.75\"/></mask>"
		"<mask id=\"smokeAlphaMask\" maskContentUnits=\"objectBoundingBox\" style=\"mask-type:alpha\"><rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"#000000\"/></mask>"
		"<filter id=\"smokeOffset\"><feOffset dx=\"1\" dy=\"2\"/></filter>"
		"<filter id=\"smokeBoxOffset\" primitiveUnits=\"objectBoundingBox\"><feOffset dx=\"10%\" dy=\"20%\"/></filter>"
		"<filter id=\"smokeDropShadow\"><feDropShadow dx=\"1\" dy=\"1\" stdDeviation=\"0.5 0.75\" flood-color=\"#123456\" flood-opacity=\"50%\"/></filter>"
		"<rect id=\"single\" x=\"0\" y=\"0\" width=\"4\" height=\"3\" fill=\"#223344\"/>"
		"<rect id=\"entitySmoke\" x=\"0\" y=\"0\" width=\"2\" height=\"1\" fill=\"#222324\" stroke=\"none\"/>"
		"<symbol id=\"smokeSymbol\" viewBox=\"-1,-1,2,2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#334455\" stroke=\"none\"/></symbol>"
		"<marker id=\"smokeMarker\" viewBox=\"0 0 4 4\" refX=\"4\" refY=\"2\" markerWidth=\"2\" markerHeight=\"2\" orient=\"auto\"><path d=\"M0 0 L4 2 L0 4 Z\" fill=\"#838485\" stroke=\"none\"/></marker>"
		"<marker id=\"smokePathMarker\" viewBox=\"0 0 4 4\" refX=\"4\" refY=\"2\" markerWidth=\"2\" markerHeight=\"2\" orient=\"auto\"><path d=\"M0 0 L4 2 L0 4 Z\" fill=\"#898a8b\" stroke=\"none\"/></marker>"
		"<marker id=\"smokeReverseMarker\" viewBox=\"0 0 4 4\" refX=\"4\" refY=\"2\" markerWidth=\"2\" markerHeight=\"2\" orient=\"auto-start-reverse\"><path d=\"M0 0 L4 2 L0 4 Z\" fill=\"#2f3031\" stroke=\"none\"/></marker>"
		"<path id=\"smokeTextPathGuide\" d=\"M1 3 C5 1 9 5 13 3\" fill=\"none\" stroke=\"none\"/>"
		"<g id=\"badge\" fill=\"#224466\" stroke=\"#88aacc\" stroke-width=\"1.5\" stroke-dasharray=\"3 1\" paint-order=\"stroke fill\">"
		"<rect x=\"1\" y=\"1\" width=\"8\" height=\"6\" rx=\"1\"/>"
		"<polyline points=\"1,10 6,12 10,10\" fill=\"none\"/>"
		"</g>"
		"</defs>"
		"<g transform=\"translate(1,1)\" fill=\"#224466\" stroke=\"#88aacc\" stroke-width=\"1.5\" stroke-dasharray=\"3 1\" paint-order=\"stroke fill\">"
		"<rect x=\"1\" y=\"1\" width=\"8\" height=\"6\" rx=\"1\" fill=\"#336699\"/>"
		"<polyline points=\"1,10 6,12 10,10\" fill=\"none\"/>"
		"</g>"
		"<use href=\"#single\" x=\"12\" y=\"10\"/>"
		"<use href=\"&#35;entitySmoke\" x=\"12\" y=\"8\"/>"
		"<use xlink:href=\"#badge\" x=\"15\" y=\"0\" opacity=\"0.7\"/>"
		"<use href=\"#smokeSymbol\" x=\"26\" y=\"11\" width=\"4\" height=\"2\"/>"
		"<use href=\"#lateSmokeSymbol\" x=\"1\" y=\"8\" width=\"4\" height=\"2\"/>"
		"<rect x=\"22\" y=\"1\" width=\"8\" height=\"5\" fill=\"url(#grad)\"/>"
		"<circle cx=\"19\" cy=\"12\" r=\"3\" fill=\"url(#rad)\"/>"
		"<rect x=\"30\" y=\"1\" width=\"2\" height=\"2\" fill=\"url(#smokePattern)\" stroke=\"none\"/>"
		"<rect x=\"27\" y=\"4\" width=\"4\" height=\"4\" fill=\"url(#smokeMixedPattern)\" stroke=\"none\"/>"
		"<rect x=\"23\" y=\"4\" width=\"4\" height=\"4\" fill=\"url(#smokeHrefPattern)\" stroke=\"none\"/>"
		"<rect x=\"19\" y=\"4\" width=\"3\" height=\"3\" fill=\"url(#smokeBoxPercentPattern)\" stroke=\"none\"/>"
		"<rect x=\"16\" y=\"4\" width=\"2\" height=\"2\" fill=\"none\" stroke=\"url(#smokeStrokePattern)\" stroke-width=\"1\"/>"
		"<svg x=\"1\" y=\"4\" width=\"4\" height=\"2\" viewBox=\"0,0,40,20\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"40\" height=\"20\" fill=\"#3344aa\" stroke=\"none\"/></svg>"
		"<g visibility=\"hidden\"><path d=\"M10 8 L12 8 L12 9 Z\" fill=\"#5566dd\" stroke=\"none\" visibility=\"visible\"/></g>"
		"<g display=\"none\"><path d=\"M13 8 L15 8 L15 9 Z\" fill=\"#12feef\" stroke=\"none\" visibility=\"visible\"/></g>"
		"<rect x=\"22\" y=\"7\" width=\"8\" height=\"3\" fill=\"url(#gradChain)\"/>"
		"<rect x=\"1\" y=\"6\" width=\"2\" height=\"1\" fill=\"url(#currentStopGrad)\"/>"
		"<rect x=\"4\" y=\"6\" width=\"2\" height=\"1\" fill=\"url(#cssStopGrad)\"/>"
		"<circle cx=\"15\" cy=\"12\" r=\"2\" fill=\"url(#radAlias)\"/>"
		"<rect x=\"25\" y=\"1\" width=\"3\" height=\"1\" fill=\"url(#linearFromRad)\"/>"
		"<circle cx=\"29\" cy=\"3\" r=\"1\" fill=\"url(#radFromLinear)\"/>"
		"<rect x=\"24\" y=\"4\" width=\"4\" height=\"1\" fill=\"url(#userLengthGrad)\"/>"
		"<circle cx=\"30\" cy=\"5\" r=\"1\" fill=\"url(#userLengthRad)\"/>"
		"<path d=\"M2 12 C8 8 12 15 18 11\" fill=\"none\" stroke=\"url(#gradChain)\" stroke-width=\"1.5\"/>"
		"<path d=\"M1 1 H7 V7 H1 Z M3 3 H5 V5 H3 Z\" fill=\"#446688\" fill-rule=\"evenodd\"/>"
		"<path d=\"M12 2 C18 2 18 12 24 12\" fill=\"none\" stroke=\"rgb(255, 64, 32)\" stroke-width=\"2\"/>"
		"<path d=\"M10 6 L12 6 L12 7 Z\" fill=\"rgb(100% 50% 0%)\" stroke=\"none\"/>"
		"<path d=\"M13 6 L15 6 L15 7 Z\" fill=\"rgba(18 52 86 / 100%)\" stroke=\"none\"/>"
		"<path d=\"M16 6 L18 6 L18 7 Z\" fill=\"#0f0f\" stroke=\"none\"/>"
		"<path d=\"M19 6 L21 6 L21 7 Z\" fill=\"#336699ff\" stroke=\"none\"/>"
		"<path d=\"M22 6 L24 6 L24 7 Z\" fill=\"hsl(210 100% 50%)\" stroke=\"none\"/>"
		"<path d=\"M25 6 L27 6 L27 7 Z\" fill=\"hsla(300, 100%, 50%, 1)\" stroke=\"none\"/>"
		"<path d=\"M28 6 L30 6 L30 7 Z\" fill=\"#fedcba\" opacity=\"100%\" stroke=\"none\"/>"
		"<path d=\"M30 6 L32 6 L32 7 Z\" fill=\"#55aa11\" fill-opacity=\"100%\" stroke=\"none\"/>"
		"<path d=\"M0 15 H8\" fill=\"none\" stroke=\"#232425\" stroke-width=\"1\" stroke-dasharray=\"2\"/>"
		"<rect x=\"9\" y=\"12\" width=\"2\" height=\"2\" ry=\"1\" fill=\"#2a2b2c\" stroke=\"none\"/>"
		"<rect x=\"0\" y=\"0\" width=\"-4\" height=\"4\" fill=\"#fe0102\" stroke=\"none\"/>"
		"<circle cx=\"0\" cy=\"0\" r=\"-2\" fill=\"#fe0304\" stroke=\"none\"/>"
		"<circle cx=\"28\" cy=\"8\" r=\"3\" fill=\"green\" opacity=\"0.5\"/>"
		"<rect x=\"23\" y=\"11\" width=\"8\" height=\"5\" fill=\"tomato\" clip-path=\"url(#smokeClip)\"/>"
		"<rect x=\"12\" y=\"1\" width=\"8\" height=\"5\" fill=\"royalblue\" clip-path=\"url(#smokeBoxClip)\"/>"
		"<rect x=\"0\" y=\"24\" width=\"4\" height=\"4\" fill=\"#22cc88\" stroke=\"none\" clip-path=\"url(#smokeRuleClip)\"/>"
		"<rect x=\"2\" y=\"20\" width=\"6\" height=\"1\" fill=\"#00e5a8\" stroke=\"none\" clip-path=\"url(#smokeMultiClip)\"/>"
		"<rect x=\"12\" y=\"20\" width=\"6\" height=\"1\" fill=\"#ff0000\" fill-opacity=\"0.5\" stroke=\"none\" clip-path=\"url(#smokeOverlapClip)\"/>"
		"<rect x=\"20\" y=\"11\" width=\"8\" height=\"4\" fill=\"gold\" mask=\"url(#smokeMask)\"/>"
		"<rect x=\"9\" y=\"20\" width=\"2\" height=\"1\" fill=\"#f012e5\" stroke=\"none\" mask=\"url(#smokeAlphaMask)\"/>"
		"<rect x=\"1\" y=\"22\" width=\"2\" height=\"1\" fill=\"#445566\" stroke=\"none\" filter=\"url(#smokeOffset)\"/>"
		"<text x=\"4\" y=\"23\" font-size=\"2\" fill=\"#667788\" stroke=\"none\" filter=\"url(#smokeBoxOffset)\">F</text>"
		"<rect x=\"13\" y=\"22\" width=\"2\" height=\"1\" fill=\"#8899aa\" stroke=\"none\" filter=\"url(#smokeDropShadow)\"/>"
		"<text x=\"16\" y=\"23\" font-size=\"2\" fill=\"#778899\" stroke=\"none\" filter=\"url(#smokeDropShadow)\">S</text>"
		"<image x=\"7\" y=\"22\" width=\"2\" height=\"2\" filter=\"url(#smokeOffset)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 2 2' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='2' height='2' fill='%2399aabb'/%3E%3C/svg%3E\"/>"
		"<image x=\"10\" y=\"22\" width=\"2\" height=\"2\" filter=\"url(#smokeOffset)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"<image x=\"19\" y=\"22\" width=\"2\" height=\"2\" filter=\"url(#smokeDropShadow)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"<rect x=\"20\" y=\"20\" width=\"2\" height=\"1\" fill=\"#12f0f0\" stroke=\"none\" style=\"opacity:nan;fill-opacity:inf;stroke-opacity:infinity;stroke-width:nan\" transform=\"translate(nan 0)\"/>"
		"<path class=\"smokeAccent\" d=\"M2 14 L4 14 L4 15 Z\"/>"
		"<path id=\"smokeId\" d=\"M6 14 L8 14 L8 15 Z\"/>"
		"<polygon points=\"10,14 12,14 12,15\"/>"
		"<path class=\"smokeCurrent\" d=\"M14 14 L16 14 L16 15 Z\"/>"
		"<path color=\"LightSteelBlue\" fill=\"currentColor\" d=\"M18 14 L20 14 L20 15 Z\"/>"
		"<path class=\"smokeCompound\" d=\"M22 14 L24 14 L24 15 Z\"/>"
		"<path class=\"smokePair extra\" d=\"M25 14 L27 14 L27 15 Z\"/>"
		"<path id=\"smokeId2\" class=\"extra\" d=\"M29 14 L31 14 L31 15 Z\"/>"
		"<path class=\"smokeCData\" d=\"M1 12 L3 12 L3 13 Z\"/>"
		"<path class=\"smokeSpecific\" d=\"M4 12 L6 12 L6 13 Z\"/>"
		"<path class=\"smokeAttr\" fill=\"#000000\" d=\"M7 12 L9 12 L9 13 Z\"/>"
		"<path class=\"smokeUniversal\" d=\"M10 12 L12 12 L12 13 Z\"/>"
		"<path class=\"smokeImportant\" style=\"fill:#ffffff\" d=\"M13 12 L15 12 L15 13 Z\"/>"
		"<path class=\"smokeImportant\" style=\"fill:#353637 !important\" d=\"M16 12 L18 12 L18 13 Z\"/>"
		"<path class=\"smokeLast\" d=\"M19 12 L21 12 L21 13 Z\"/>"
		"<path d=\"M22 12 H24\" transform=\"scale(1.5)\" fill=\"none\" stroke=\"#38393a\" stroke-width=\"1\" vector-effect=\"non-scaling-stroke\"/>"
		"<rect x=\"26\" y=\"12\" width=\"3\" height=\"2\" fill=\"#414243\" stroke=\"#444546\" stroke-width=\"1\" paint-order=\"markers stroke\"/>"
		"<g class=\"smokeScope\"><g><path class=\"smokeDesc\" d=\"M1 16 L3 16 L3 17 Z\"/></g><path class=\"smokeChild\" d=\"M4 16 L6 16 L6 17 Z\"/></g>"
		"<rect x=\"7\" y=\"16\" width=\"2\" height=\"1\" fill=\"url(#scopedStopGrad)\" stroke=\"none\"/>"
		"<rect x=\"9\" y=\"15\" width=\"1\" height=\"1\" probe=\"1\"/><rect x=\"10\" y=\"15\" width=\"1\" height=\"1\" data-eq=\"match\"/><rect x=\"11\" y=\"15\" width=\"1\" height=\"1\" data-word=\"alpha beta\"/><rect x=\"12\" y=\"15\" width=\"1\" height=\"1\" lang=\"en-US\"/><rect x=\"13\" y=\"15\" width=\"1\" height=\"1\" data-prefix=\"prefix\"/><rect x=\"14\" y=\"15\" width=\"1\" height=\"1\" data-suffix=\"the-end\"/><rect x=\"15\" y=\"15\" width=\"1\" height=\"1\" data-sub=\"in-the-middle\"/>"
		"<rect class=\"smokeFontEm\" x=\"16\" y=\"15\" width=\"1em\" height=\"1ex\"/><rect class=\"smokeFontEx\" x=\"19\" y=\"15\" width=\"1ex\" height=\"1\"/>"
		"<path class=\"smokeNotProbe smokeNotSkip\" d=\"M21 16 L22 16 L22 17 Z\"/><path class=\"smokeNotProbe\" d=\"M23 16 L24 16 L24 17 Z\"/><path class=\"smokeIsProbe smokeIsMatch\" d=\"M25 16 L26 16 L26 17 Z\"/><path class=\"smokeWhereProbe smokeWhereMatch\" d=\"M27 16 L28 16 L28 17 Z\"/><path class=\"smokeSiblingPrev\" d=\"M1 17 L2 17 L2 18 Z\"/><path class=\"smokeAdjacentProbe\" d=\"M3 17 L4 17 L4 18 Z\"/><path class=\"smokeSiblingBase\" d=\"M5 17 L6 17 L6 18 Z\"/><path class=\"smokeSiblingSkip\" d=\"M7 17 L8 17 L8 18 Z\"/><path class=\"smokeGeneralProbe\" d=\"M9 17 L10 17 L10 18 Z\"/><g><path class=\"smokeFirstChildProbe\" d=\"M11 17 L12 17 L12 18 Z\"/><path d=\"M13 17 L14 17 L14 18 Z\"/></g><g><rect x=\"15\" y=\"17\" width=\"1\" height=\"1\"/><path class=\"smokeFirstTypeProbe\" d=\"M17 17 L18 17 L18 18 Z\"/></g><g><path d=\"M19 17 L20 17 L20 18 Z\"/><path d=\"M21 17 L22 17 L22 18 Z\"/><path class=\"smokeNthChildProbe\" d=\"M23 17 L24 17 L24 18 Z\"/></g><g><rect x=\"25\" y=\"17\" width=\"1\" height=\"1\"/><path d=\"M27 17 L28 17 L28 18 Z\"/><path class=\"smokeNthTypeProbe\" d=\"M29 17 L30 17 L30 18 Z\"/></g><g><path d=\"M1 18 L2 18 L2 19 Z\"/><path class=\"smokeLastChildProbe\" d=\"M3 18 L4 18 L4 19 Z\"/></g><g><path class=\"smokeLastTypeProbe\" d=\"M5 18 L6 18 L6 19 Z\"/><rect x=\"7\" y=\"18\" width=\"1\" height=\"1\"/></g><g><path class=\"smokeOnlyChildProbe\" d=\"M9 18 L10 18 L10 19 Z\"/></g><g><rect x=\"11\" y=\"18\" width=\"1\" height=\"1\"/><path class=\"smokeOnlyTypeProbe\" d=\"M13 18 L14 18 L14 19 Z\"/></g><g><path d=\"M15 18 L16 18 L16 19 Z\"/><path class=\"smokeNthLastChildProbe\" d=\"M17 18 L18 18 L18 19 Z\"/><path d=\"M19 18 L20 18 L20 19 Z\"/></g><g><path d=\"M21 18 L22 18 L22 19 Z\"/><path class=\"smokeNthLastTypeProbe\" d=\"M23 18 L24 18 L24 19 Z\"/><path d=\"M25 18 L26 18 L26 19 Z\"/></g>"
		"<path class=\"smokeEmptyProbe\" d=\"M27 18 L28 18 L28 19 Z\"/><g class=\"smokeHasDescProbe\"><g><path class=\"smokeHasNeedle\" d=\"M29 18 L30 18 L30 19 Z\"/></g></g><g class=\"smokeHasChildProbe\"><path class=\"smokeHasChildNeedle\" d=\"M1 19 L2 19 L2 20 Z\"/></g><path class=\"smokeHasAdjacentProbe\" d=\"M3 19 L4 19 L4 20 Z\"/><path class=\"smokeHasAdjacentNeedle\" d=\"M5 19 L6 19 L6 20 Z\" fill=\"#010101\" stroke=\"none\"/>"
		"<rect x=\"12\" y=\"12\" width=\"2\" height=\"1\" fill=\"url(#missingPaint) #1f2021\" stroke=\"none\"/>"
		"<line x1=\"29\" y1=\"15\" x2=\"29\" y2=\"15\" stroke=\"#7f8081\" stroke-width=\"1\" stroke-linecap=\"round\" stroke-dasharray=\"2 1\"/>"
		"<line x1=\"30\" y1=\"15\" x2=\"30\" y2=\"15\" stroke=\"#797a7b\" stroke-width=\"1\" stroke-linecap=\"round\"/>"
		"<line x1=\"31\" y1=\"15\" x2=\"31\" y2=\"15\" stroke=\"#7c7d7e\" stroke-width=\"1\" stroke-linecap=\"square\"/>"
		"<line x1=\"1\" y1=\"10\" x2=\"9\" y2=\"10\" stroke=\"#373a3d\" stroke-width=\"1\" stroke-dasharray=\"4 4\" pathLength=\"4\"/>"
		"<g stroke-dasharray=\"2 2\"><line x1=\"10\" y1=\"10\" x2=\"14\" y2=\"10\" stroke=\"#4a4b4c\" stroke-width=\"1\" stroke-dasharray=\" NONE \"/></g>"
		"<line x1=\"1\" y1=\"15\" x2=\"5\" y2=\"15\" stroke=\"#858687\" stroke-width=\"1\" marker-end=\"url(#smokeMarker)\"/>"
		"<line x1=\"18\" y1=\"15\" x2=\"22\" y2=\"15\" stroke=\"#858687\" stroke-width=\"1\" marker-start=\"url(#smokeReverseMarker)\"/>"
		"<polyline points=\"6,15 8,16 10,15\" fill=\"none\" stroke=\"#858687\" stroke-width=\"1\" marker-start=\"url(#smokeMarker)\" marker-mid=\"url(#smokeMarker)\" marker-end=\"url(#smokeMarker)\"/>"
		"<path d=\"M12 15 C13 14 14 16 15 15 L17 15\" fill=\"none\" stroke=\"#8c8d8e\" stroke-width=\"1\" marker-start=\"url(#smokePathMarker)\" marker-mid=\"url(#smokePathMarker)\" marker-end=\"url(#smokePathMarker)\"/>"
		"<text x=\"1\" y=\"4\" font-size=\"3\" fill=\"#8f9091\">OK</text>"
		"<text x=\"16\" y=\"4\" font-size=\"3\" fill=\"#262728\" text-anchor=\"middle\">A</text>"
		"<text x=\"22\" y=\"4\" font-size=\"3\" fill=\"#292a2b\" text-anchor=\"middle\"><tspan>A</tspan><tspan dx=\"1\">B</tspan></text>"
		"<text x=\"24\" y=\"4\" dx=\"2\" dy=\"1\" font-size=\"3\" fill=\"#2c2d2e\">D</text>"
		"<text x=\"6\" y=\"4\" font-size=\"3\" fill=\"#8f9091\"><tspan fill=\"#202122\">TS</tspan><tspan dx=\"2\" fill=\"#232425\">P</tspan></text>"
		"<text x=\"12\" y=\"8\" font-size=\"3\" fill=\"#303132\"><tspan>N<tspan fill=\"#333435\">E</tspan>S</tspan></text>"
		"<text x=\"18\" y=\"8\" font-size=\"3\" fill=\"#36393c\" xml:space=\"preserve\">  S</text>"
		"<text x=\"22\" y=\"9\" font-size=\"3\" fill=\"#424344\" transform=\"rotate(25 22 9)\">R</text>"
		"<text x=\"2\" y=\"12\" font-size=\"2\" fill=\"#434445\" letter-spacing=\"1\" word-spacing=\"2\">A B</text>"
		"<text font-size=\"2\" fill=\"#3f4041\" stroke=\"none\" text-decoration=\"underline\"><textPath href=\"#smokeTextPathGuide\" startOffset=\"10%\" textLength=\"5\" lengthAdjust=\"spacing\">TP</textPath></text>"
		"<text x=\"12\" y=\"12\" font-size=\"2\" fill=\"#b0b1b2\" dominant-baseline=\"middle\">M</text>"
		"<text x=\"14\" y=\"12\" font-size=\"2\" fill=\"#b3b4b5\" alignment-baseline=\"text-before-edge\">T</text>"
		"<text x=\"16\" y=\"12\" font-size=\"2\" fill=\"#b6b7b8\" text-decoration=\"underline overline line-through\">D</text>"
		"<text x=\"18\" y=\"12\" font-size=\"2\" fill=\"#b9babb\" text-decoration-line=\"none\">N</text>"
		"<text x=\"20\" y=\"12\" font-size=\"2\" fill=\"#bcbdbe\" textLength=\"6\" lengthAdjust=\"spacing\">TL</text>"
		"<text x=\"24\" y=\"12\" font-size=\"2\" fill=\"#c0c1c2\" textLength=\"3\" lengthAdjust=\"spacingAndGlyphs\">TG</text>"
		"<text x=\"27\" y=\"12\" font-size=\"2\" fill=\"#c3c4c5\"><tspan textLength=\"4\" lengthAdjust=\"spacing\">TS</tspan></text>"
		"<image id=\"smokeImageUse\" x=\"22\" y=\"13\" width=\"2\" height=\"2\" href=\"data:image/svg+xml,%3Csvg%20viewBox%3D%270%200%2010%2010%27%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%3E%3Crect%20width%3D%2710%27%20height%3D%2710%27%20fill%3D%27%23929394%27%2F%3E%3C%2Fsvg%3E\"/>"
		"<use href=\"#smokeImageUse\" transform=\"translate(-2 0)\"/>"
		"<image x=\"24\" y=\"13\" width=\"2\" height=\"2\" href=\"data:image/svg+xml;utf8,%3Csvg%20viewBox%3D'0%200%2010%2010'%20xmlns%3D'http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg'%3E%3Crect%20width%3D'10'%20height%3D'10'%20fill%3D'%2398999a'%2F%3E%3C%2Fsvg%3E\"/>"
		"<image x=\"26\" y=\"13\" width=\"2\" height=\"2\" href=\"data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PScwIDAgMTAgMTAnIHhtbG5zPSdodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2Zyc+PHJlY3Qgd2lkdGg9JzEwJyBoZWlnaHQ9JzEwJyBmaWxsPScjOTU5Njk3Jy8+PC9zdmc+\"/>"
		"<image id=\"smokeRasterUse\" x=\"28\" y=\"13\" width=\"2\" height=\"2\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"<use href=\"#smokeRasterUse\" transform=\"translate(-2 0)\"/>"
		"<path id=\"smokePathUse\" d=\"M28 13 L30 13 L30 15 Z\" fill=\"#9b9c9d\" stroke=\"none\"/><use href=\"#smokePathUse\" transform=\"translate(-3 0)\"/>"
		"<text id=\"smokeTextUse\" x=\"1\" y=\"8\" font-size=\"4\" fill=\"#a102f3\">Q</text><use href=\"&nbsp;#smokeTextUse&nbsp;\" transform=\"translate(4 0)\"/>"
		"<defs><symbol id=\"lateSmokeSymbol\" viewBox=\"0,0,10,5\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"10\" height=\"5\" fill=\"#778899\" stroke=\"none\"/></symbol></defs>"
		"</svg>";
	static const char svg_units[] =
		"<svg width=\"25.4mm\" height=\"1in\">"
		"<rect x=\"10%\" y=\"25%\" width=\"50%\" height=\"25%\" fill=\"#123456\" stroke=\"#654321\" stroke-width=\"5%\"/>"
		"</svg>";
	static const char svg_font_units[] =
		"<svg width=\"6em\" height=\"4ex\"><style>:root{font-size:12px}</style>"
		"<rect x=\"1em\" y=\"1ex\" width=\"2em\" height=\"2ex\" fill=\"#6f7071\" stroke=\"none\"/>"
		"</svg>";
	static const char svg_filter_region[] =
		"<svg viewBox=\"0 0 16 16\">"
		"<defs>"
		"<filter id=\"boxRegion\" x=\"25%\" y=\"25%\" width=\"50%\" height=\"50%\"><feOffset dx=\"0\" dy=\"0\"/></filter>"
		"<filter id=\"userRegion\" filterUnits=\"userSpaceOnUse\" x=\"1\" y=\"1\" width=\"6\" height=\"6\"><feDropShadow dx=\"1\" dy=\"1\" stdDeviation=\"0.5\"/></filter>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" fill=\"#38bdf8\" filter=\"url(#boxRegion)\"/>"
		"<text x=\"1\" y=\"14\" font-size=\"3\" fill=\"#f97316\" filter=\"url(#userRegion)\">F</text>"
		"<image x=\"8\" y=\"0\" width=\"4\" height=\"4\" filter=\"url(#boxRegion)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 4 4' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='4' height='4' fill='%2300e5a8'/%3E%3C/svg%3E\"/>"
		"<image x=\"8\" y=\"8\" width=\"4\" height=\"4\" filter=\"url(#boxRegion)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"</svg>";
	static const char svg_clip_shapes[] =
		"<svg viewBox=\"0 0 16 16\">"
		"<defs>"
		"<clipPath id=\"vectorClip\">"
		"<path d=\"M1 1 L7 1 L4 6 Z\"/>"
		"<circle cx=\"10\" cy=\"4\" r=\"3\" transform=\"translate(1 0)\"/>"
		"<ellipse cx=\"4\" cy=\"11\" rx=\"3\" ry=\"2\"/>"
		"<polygon points=\"9,9 14,9 14,14 9,14\"/>"
		"<polyline points=\"0,15 4,13 8,15\"/>"
		"<line x1=\"12\" y1=\"1\" x2=\"15\" y2=\"4\"/>"
		"</clipPath>"
		"<clipPath id=\"boxVectorClip\" clipPathUnits=\"objectBoundingBox\">"
		"<path d=\"M0 0 L1 0 L1 1 Z\"/>"
		"<rect x=\"0.1\" y=\"0.1\" width=\"0.8\" height=\"0.8\" rx=\"0.1\"/>"
		"</clipPath>"
		"<path id=\"clipUseShape\" d=\"M1 1 H7 V7 H1 Z\"/>"
		"<clipPath id=\"useVectorClip\"><use href=\"#clipUseShape\" x=\"4\" y=\"4\"/></clipPath>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"16\" height=\"16\" fill=\"#abcdef\" clip-path=\"url(#vectorClip)\"/>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"8\" fill=\"#fedcba\" clip-path=\"url(#boxVectorClip)\"/>"
		"<rect x=\"0\" y=\"0\" width=\"16\" height=\"16\" fill=\"#55aaff\" clip-path=\"url(#useVectorClip)\"/>"
		"</svg>";
	static const char svg_mask_shapes[] =
		"<svg viewBox=\"0 0 16 16\">"
		"<defs>"
		"<mask id=\"vectorMask\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"alpha\">"
		"<path d=\"M1 1 L7 1 L4 6 Z\" fill=\"#000\"/>"
		"<circle cx=\"10\" cy=\"4\" r=\"3\" transform=\"translate(1 0)\" fill=\"#000\"/>"
		"<rect x=\"2\" y=\"8\" width=\"6\" height=\"4\" rx=\"1\" fill=\"#000\"/>"
		"</mask>"
		"<mask id=\"groupStyleMask\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"alpha\"><g fill=\"#000\" opacity=\"0.5\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"8\"/></g></mask>"
		"<path id=\"maskUseShape\" d=\"M0 0 H6 V6 H0 Z\"/>"
		"<mask id=\"useStyleMask\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"alpha\"><use href=\"#maskUseShape\" x=\"9\" y=\"9\" opacity=\"0.5\"/></mask>"
		"<mask id=\"regionUserMask\" maskUnits=\"userSpaceOnUse\" x=\"1\" y=\"1\" width=\"4\" height=\"4\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"alpha\"><rect x=\"0\" y=\"0\" width=\"16\" height=\"16\" fill=\"#000\"/></mask>"
		"<mask id=\"regionBoxMask\" maskUnits=\"objectBoundingBox\" x=\"0.25\" y=\"0.25\" width=\"0.5\" height=\"0.5\" maskContentUnits=\"objectBoundingBox\" mask-type=\"alpha\"><rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"#000\"/></mask>"
		"<mask id=\"boxVectorMask\" maskContentUnits=\"objectBoundingBox\" mask-type=\"alpha\">"
		"<path d=\"M0 0 L1 0 L1 1 Z\" fill=\"#000\"/>"
		"<rect x=\"0.1\" y=\"0.1\" width=\"0.8\" height=\"0.8\" rx=\"0.1\" fill=\"#000\"/>"
		"</mask>"
		"<mask id=\"strokeMask\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"alpha\">"
		"<line x1=\"1\" y1=\"15\" x2=\"15\" y2=\"15\" fill=\"none\" stroke=\"#000\" stroke-width=\"2\" stroke-linecap=\"round\"/>"
		"</mask>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"16\" height=\"16\" fill=\"#abcdef\" mask=\"url(#vectorMask)\"/>"
		"<rect x=\"8\" y=\"0\" width=\"8\" height=\"8\" fill=\"#38bdf8\" mask=\"url(#groupStyleMask)\"/>"
		"<rect x=\"8\" y=\"8\" width=\"8\" height=\"8\" fill=\"#f97316\" mask=\"url(#useStyleMask)\"/>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" fill=\"#a855f7\" mask=\"url(#regionUserMask)\"/>"
		"<rect x=\"8\" y=\"8\" width=\"8\" height=\"8\" fill=\"#14b8a6\" mask=\"url(#regionBoxMask)\"/>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"8\" fill=\"#fedcba\" mask=\"url(#boxVectorMask)\"/>"
		"<text x=\"1\" y=\"14\" font-size=\"3\" fill=\"#ffcc00\" mask=\"url(#vectorMask)\">M</text>"
		"<image x=\"10\" y=\"10\" width=\"4\" height=\"4\" mask=\"url(#vectorMask)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 4 4' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='4' height='4' fill='%2300e5a8'/%3E%3C/svg%3E\"/>"
		"<image x=\"4\" y=\"12\" width=\"4\" height=\"4\" mask=\"url(#boxVectorMask)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"<rect x=\"0\" y=\"14\" width=\"16\" height=\"2\" fill=\"#22c55e\" mask=\"url(#strokeMask)\"/>"
		"</svg>";
	static const char svg_paint_order_markers[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<marker id=\"m\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\">"
		"<circle cx=\"2\" cy=\"2\" r=\"2\" fill=\"#ff0000\"/>"
		"</marker>"
		"</defs>"
		"<path d=\"M3 9 C6 1 10 1 13 9\" fill=\"#00aaee\" stroke=\"#ffcc00\" stroke-width=\"3\" marker-start=\"url(#m)\" marker-mid=\"url(#m)\" marker-end=\"url(#m)\" paint-order=\"markers stroke fill\"/>"
		"<polyline points=\"15,9 18,2 21,9\" fill=\"none\" stroke=\"#ffcc00\" stroke-width=\"3\" marker-start=\"url(#m)\" marker-mid=\"url(#m)\" marker-end=\"url(#m)\" paint-order=\"fill markers stroke\"/>"
		"</svg>";
	static const char svg_paint_order_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g paint-order=\"stroke fill\" fill=\"#38bdf8\" stroke=\"#facc15\" stroke-width=\"3\">"
		"<path d=\"M2 9 C5 1 9 1 12 9\" paint-order=\"bad\"/>"
		"<path d=\"M14 9 C17 1 21 1 22 9\" paint-order=\"normal fill\"/>"
		"</g>"
		"</svg>";
	static const char svg_paint_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"#ef4444\" stroke=\"#38bdf8\" stroke-width=\"2\">"
		"<rect x=\"1\" y=\"2\" width=\"5\" height=\"6\" fill=\"bad\"/>"
		"<rect x=\"8\" y=\"2\" width=\"5\" height=\"6\" stroke=\"bad\"/>"
		"<rect x=\"15\" y=\"2\" width=\"5\" height=\"6\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_width_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"4\" stroke-linecap=\"round\">"
		"<path d=\"M2 3 H10\" stroke-width=\"bad\"/>"
		"<path d=\"M2 6 H10\" stroke-width=\"nan\"/>"
		"<path d=\"M2 9 H10\" stroke-width=\"10%\"/>"
		"<path d=\"M14 3 H22\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_dasharray_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"3 2\">"
		"<path d=\"M2 3 H22\" stroke-dasharray=\"bad\"/>"
		"<path d=\"M2 6 H22\" stroke-dasharray=\"5 bad\"/>"
		"<path d=\"M2 9 H22\" stroke-dasharray=\"-1 2\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_dashoffset_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"3 2\" stroke-dashoffset=\"2\">"
		"<path d=\"M2 3 H22\" stroke-dashoffset=\"bad\"/>"
		"<path d=\"M2 6 H22\" stroke-dashoffset=\"nan\"/>"
		"<path d=\"M2 9 H22\" stroke-dashoffset=\"10%\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_miterlimit_invalid[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-linejoin=\"miter\" stroke-miterlimit=\"8\">"
		"<path d=\"M2 10 L5 2 L8 10\" stroke-miterlimit=\"bad\"/>"
		"<path d=\"M9 10 L12 2 L15 10\" stroke-miterlimit=\"nan\"/>"
		"<path d=\"M16 10 L19 2 L22 10\" stroke-miterlimit=\"-2\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_line_enum_invalid[] =
		"<svg viewBox=\"0 0 24 14\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
		"<path d=\"M2 3 H8\" stroke-linecap=\"bad\"/>"
		"<path d=\"M10 3 H16\"/>"
		"<path d=\"M2 12 L5 5 L8 12\" stroke-linejoin=\"bad\"/>"
		"<path d=\"M12 12 L15 5 L18 12\"/>"
		"</g>"
		"</svg>";
	static const char svg_marker_mixed_content[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<marker id=\"textM\" viewBox=\"0 0 8 6\" refX=\"4\" refY=\"3\" markerWidth=\"8\" markerHeight=\"6\" markerUnits=\"userSpaceOnUse\"><text x=\"4\" y=\"4\" font-size=\"4\" text-anchor=\"middle\" fill=\"#fff\">T</text></marker>"
		"<marker id=\"imageM\" viewBox=\"0 0 6 6\" refX=\"3\" refY=\"3\" markerWidth=\"6\" markerHeight=\"6\" markerUnits=\"userSpaceOnUse\"><image x=\"0\" y=\"0\" width=\"6\" height=\"6\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 6 6' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='6' height='6' fill='%23f97316'/%3E%3C/svg%3E\"/></marker>"
		"</defs>"
		"<polyline points=\"3,3 12,3 21,3\" fill=\"none\" stroke=\"#ffcc00\" stroke-width=\"1\" marker-start=\"url(#textM)\" marker-mid=\"url(#textM)\" marker-end=\"url(#textM)\"/>"
		"<polyline points=\"3,9 12,6 21,9\" fill=\"none\" stroke=\"#00aaee\" stroke-width=\"1\" marker-start=\"url(#imageM)\" marker-mid=\"url(#imageM)\" marker-end=\"url(#imageM)\"/>"
		"</svg>";
	static const char svg_use_markers[] =
		"<svg viewBox=\"0 0 24 14\">"
		"<defs>"
		"<marker id=\"dot\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\"><circle cx=\"2\" cy=\"2\" r=\"2\" fill=\"#ff0000\"/></marker>"
		"</defs>"
		"<path id=\"markedPath\" d=\"M3 3 L12 3 L21 3\" fill=\"none\" stroke=\"#00aaee\" stroke-width=\"1\" marker-start=\"url(#dot)\" marker-mid=\"url(#dot)\" marker-end=\"url(#dot)\"/>"
		"<use href=\"#markedPath\" y=\"5\"/>"
		"<use id=\"markedUse\" href=\"#markedPath\" y=\"10\" opacity=\"0.7\"/>"
		"<use href=\"#markedUse\" y=\"3\" opacity=\"0.5\"/>"
		"</svg>";
	static const char svg_use_inherit[] =
		"<svg viewBox=\"0 0 48 18\">"
		"<defs>"
		"<linearGradient id=\"warmUse\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\"><stop offset=\"0%\" stop-color=\"#f97316\"/><stop offset=\"100%\" stop-color=\"#facc15\"/></linearGradient>"
		"<pattern id=\"tileUse\" patternUnits=\"userSpaceOnUse\" width=\"2\" height=\"2\"><rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#312e81\"/><path d=\"M0 2 L2 0\" stroke=\"#a78bfa\" stroke-width=\"0.5\"/></pattern>"
		"<symbol id=\"plain\" viewBox=\"0 0 8 6\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"6\"/><circle cx=\"6\" cy=\"3\" r=\"2\" fill=\"#ff4d6d\"/></symbol>"
		"<symbol id=\"current\" viewBox=\"0 0 8 6\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"6\" fill=\"currentColor\" fill-opacity=\"0.42\"/><text x=\"4\" y=\"4\" font-size=\"3\" text-anchor=\"middle\" fill=\"currentColor\">C</text></symbol>"
		"<symbol id=\"fixed\" viewBox=\"0 0 8 6\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"6\" fill=\"#facc15\"/></symbol>"
		"</defs>"
		"<use href=\"#plain\" x=\"1\" y=\"1\" width=\"8\" height=\"6\" fill=\"#22c55e\"/>"
		"<use href=\"#current\" x=\"13\" y=\"1\" width=\"8\" height=\"6\" color=\"#38bdf8\"/>"
		"<use href=\"#fixed\" x=\"25\" y=\"1\" width=\"8\" height=\"6\" fill=\"#ffffff\"/>"
		"<use href=\"#plain\" x=\"1\" y=\"10\" width=\"8\" height=\"6\" fill=\"url(#warmUse)\"/>"
		"<use href=\"#plain\" x=\"13\" y=\"10\" width=\"8\" height=\"6\" fill=\"url(#tileUse)\"/>"
		"<use href=\"#plain\" x=\"25\" y=\"10\" width=\"8\" height=\"6\" fill=\"none\" stroke=\"url(#tileUse)\" stroke-width=\"1\"/>"
		"</svg>";
	static const char svg_group_use[] =
		"<svg viewBox=\"0 0 32 16\">"
		"<g id=\"badge\" transform=\"translate(2 2)\">"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"6\" fill=\"#00aaee\"/>"
		"<circle id=\"badgeDot\" cx=\"2\" cy=\"2\" r=\"1\" fill=\"#ff0000\"/>"
		"<text x=\"4\" y=\"5\" font-size=\"3\" text-anchor=\"middle\" fill=\"#fff\">G</text>"
		"<g id=\"badgeNest\" transform=\"translate(6 1)\"><rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#ffcc00\"/></g>"
		"</g>"
		"<use href=\"#badge\" x=\"10\"/>"
		"<use href=\"#badgeDot\" x=\"20\" y=\"8\"/>"
		"<use href=\"#badgeNest\" x=\"24\" y=\"8\"/>"
		"</svg>";
	static const char svg_defs_svg_use[] =
		"<svg viewBox=\"0 0 32 16\">"
		"<defs>"
		"<svg id=\"svgIcon\" viewBox=\"0 0 8 8\" preserveAspectRatio=\"xMidYMid meet\">"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" fill=\"#00aaee\"/>"
		"<circle cx=\"4\" cy=\"4\" r=\"2\" fill=\"#ff0000\"/>"
		"</svg>"
		"<svg id=\"svgWide\" viewBox=\"0 0 8 4\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"4\" fill=\"#ffcc00\"/></svg>"
		"<svg id=\"svgNoViewBox\" width=\"8\" height=\"4\" preserveAspectRatio=\"none\"><path d=\"M1 3 L4 1 L7 3 Z\" fill=\"#22c55e\"/></svg>"
		"</defs>"
		"<use href=\"#svgIcon\" x=\"2\" y=\"2\" width=\"8\" height=\"8\"/>"
		"<use href=\"#svgWide\" x=\"14\" y=\"4\" width=\"14\" height=\"4\"/>"
		"<use href=\"#svgNoViewBox\" x=\"22\" y=\"10\" width=\"8\" height=\"4\"/>"
		"<svg id=\"inlineSvg\" x=\"2\" y=\"11\" width=\"4\" height=\"4\" viewBox=\"0 0 4 4\"><rect x=\"0\" y=\"0\" width=\"4\" height=\"4\" fill=\"#00e5a8\"/><path id=\"inlineGlyph\" d=\"M1 3 L2 1 L3 3 Z\" fill=\"#fff\"/></svg>"
		"<use href=\"#inlineSvg\" x=\"8\" y=\"11\" width=\"4\" height=\"4\"/>"
		"<use href=\"#inlineGlyph\" x=\"14\" y=\"11\"/>"
		"</svg>";
	static const char svg_gradient_stop_offset_invalid[] =
		"<svg viewBox=\"0 0 12 8\">"
		"<defs>"
		"<linearGradient id=\"badStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
		"<stop offset=\"0%\" stop-color=\"#ef4444\"/>"
		"<stop offset=\"40px\" stop-color=\"#f97316\"/>"
		"<stop offset=\"60% trailing\" stop-color=\"#22c55e\"/>"
		"<stop offset=\"75 %\" stop-color=\"#0ea5e9\"/>"
		"<stop offset=\"100%\" stop-color=\"#3b82f6\"/>"
		"</linearGradient>"
		"<radialGradient id=\"badRadial\" cx=\"50%\" cy=\"50%\" r=\"50%\">"
		"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
		"<stop offset=\"middle\" stop-color=\"#a855f7\"/>"
		"<stop offset=\"50%%\" stop-color=\"#facc15\"/>"
		"<stop offset=\"80 %\" stop-color=\"#14b8a6\"/>"
		"<stop offset=\"100%\" stop-color=\"#111827\"/>"
		"</radialGradient>"
		"</defs>"
		"<rect x=\"1\" y=\"1\" width=\"10\" height=\"3\" fill=\"url(#badStops)\"/>"
		"<circle cx=\"6\" cy=\"6\" r=\"2\" fill=\"url(#badRadial)\"/>"
		"</svg>";
	static const char svg_opacity_invalid[] =
		"<svg viewBox=\"0 0 16 8\">"
		"<defs>"
		"<linearGradient id=\"invalidStopOpacity\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
		"<stop offset=\"0%\" stop-color=\"#ef4444\" stop-opacity=\"bad\"/>"
		"<stop offset=\"100%\" stop-color=\"#3b82f6\" style=\"stop-opacity:50 %\"/>"
		"</linearGradient>"
		"</defs>"
		"<g opacity=\"0.35\">"
		"<rect x=\"1\" y=\"1\" width=\"3\" height=\"3\" fill=\"#22c55e\" opacity=\"bad\"/>"
		"<rect x=\"5\" y=\"1\" width=\"3\" height=\"3\" fill=\"#f97316\" fill-opacity=\"bad\"/>"
		"<rect x=\"9\" y=\"1\" width=\"3\" height=\"3\" fill=\"none\" stroke=\"#a855f7\" stroke-width=\"1\" stroke-opacity=\"bad\"/>"
		"<rect x=\"1\" y=\"5\" width=\"12\" height=\"2\" fill=\"url(#invalidStopOpacity)\"/>"
		"</g>"
		"</svg>";
	static const char svg_external_child[] =
		"<svg viewBox=\"0 0 2 2\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#00e5a8\" stroke=\"none\"/>"
		"</svg>";
	static const char svg_external_parent[] =
		"<svg viewBox=\"0 0 8 4\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"0\" y=\"0\" width=\"4\" height=\"4\" href=\"xge_shapeex_svg_external%20child.png\"/>"
		"<image x=\"4\" y=\"0\" width=\"4\" height=\"4\" href=\"xge_shapeex_svg_external%20child.svg\" preserveAspectRatio=\"none\"/>"
		"</svg>";
	static const char svg_external_use_child[] =
		"<svg viewBox=\"0 0 8 4\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<symbol id=\"frag\" viewBox=\"0 0 4 2\"><rect x=\"0\" y=\"0\" width=\"4\" height=\"2\" fill=\"#f97316\"/><circle cx=\"1\" cy=\"1\" r=\"0.75\" fill=\"#fff\"/></symbol>"
		"<symbol id=\"inheritPlain\" viewBox=\"0 0 4 2\"><rect x=\"0\" y=\"0\" width=\"4\" height=\"2\"/><circle cx=\"3\" cy=\"1\" r=\"0.75\" fill=\"#ff4d6d\"/></symbol>"
		"<symbol id=\"inheritCurrent\" viewBox=\"0 0 4 2\"><rect x=\"0\" y=\"0\" width=\"4\" height=\"2\" fill=\"currentColor\"/><path d=\"M1 1.5 L2 0.4 L3 1.5 Z\" fill=\"currentColor\"/></symbol>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"4\" fill=\"#334155\"/>"
		"<path d=\"M1 3 L4 1 L7 3 Z\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_external_use_parent[] =
		"<svg viewBox=\"0 0 28 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<use href=\"xge_shapeex_svg_external_use_child.svg\" x=\"1\" y=\"1\" width=\"8\" height=\"4\"/>"
		"<use href=\"url(xge_shapeex_svg_external_use_child.svg)\" x=\"11\" y=\"1\" width=\"8\" height=\"4\"/>"
		"<use href=\"xge_shapeex_svg_external_use_child.svg#frag\" x=\"1\" y=\"6\" width=\"8\" height=\"2\"/>"
		"<use href=\"url(xge_shapeex_svg_external_use_child.svg#frag)\" x=\"12\" y=\"6\" width=\"4\" height=\"2\"/>"
		"<use href=\"xge_shapeex_svg_external_use_child.svg#missing\" x=\"12\" y=\"6\" width=\"8\" height=\"2\"/>"
		"<use href=\"xge_shapeex_svg_external_use_child.svg#inheritPlain\" x=\"1\" y=\"9\" width=\"6\" height=\"2\" fill=\"#22c55e\"/>"
		"<use href=\"xge_shapeex_svg_external_use_child.svg#inheritCurrent\" x=\"9\" y=\"9\" width=\"6\" height=\"2\" color=\"#38bdf8\"/>"
		"<use href=\"xge_shapeex_svg_external_use_child.svg#inheritPlain\" x=\"17\" y=\"9\" width=\"6\" height=\"2\" fill=\"none\" stroke=\"#e879f9\" stroke-width=\"0.3\"/>"
		"</svg>";
	static const unsigned char external_png_pixels[] = {
		255, 0, 0, 255,     0, 255, 0, 255,
		0, 0, 255, 255,     255, 255, 0, 255
	};
	xge_svg svg;
	xge_svg cached_a;
	xge_svg cached_b;
	xge_svg cached_c;
	xge_rect_t viewbox;
	xge_rect_t viewport;
	const char* cache_path = "build\\xge_shapeex_svg_smoke_tmp.svg";
	const char* external_path = "build\\xge_shapeex_svg_external_parent.svg";
	const char* external_child_svg_path = "build\\xge_shapeex_svg_external child.svg";
	const char* external_child_png_path = "build\\xge_shapeex_svg_external child.png";
	const char* external_use_path = "build\\xge_shapeex_svg_external_use_parent.svg";
	const char* external_use_child_path = "build\\xge_shapeex_svg_external_use_child.svg";
	FILE* file;
	int ret;

	svg = NULL;
	cached_a = NULL;
	cached_b = NULL;
	cached_c = NULL;
	ret = xgeSvgCreate(&svg);
	if ( !check((ret == XGE_OK) && (svg != NULL), "SVG create") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text));
	if ( !check(ret == XGE_OK, "SVG load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fX == 0.0f) && (viewbox.fY == 0.0f) && (viewbox.fW == 32.0f) && (viewbox.fH == 16.0f), "SVG viewBox") ) return 0;
	ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	if ( !check((ret == XGE_OK) && (viewport.fX == 0.0f) && (viewport.fY == 32.0f) && (viewport.fW == 64.0f) && (viewport.fH == 32.0f), "SVG meet viewport") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "none");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){1.0f, 2.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == 1.0f) && (viewport.fY == 2.0f) && (viewport.fW == 64.0f) && (viewport.fH == 64.0f), "SVG none viewport") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "xMaxYMin slice");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == -64.0f) && (viewport.fY == 0.0f) && (viewport.fW == 128.0f) && (viewport.fH == 64.0f), "SVG slice viewport") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "prexMaxYMin slice");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == 0.0f) && (viewport.fY == 16.0f) && (viewport.fW == 64.0f) && (viewport.fH == 32.0f), "SVG invalid aspect prefix uses default") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "slice");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == -32.0f) && (viewport.fY == 0.0f) && (viewport.fW == 128.0f) && (viewport.fH == 64.0f), "SVG aspect slice without align") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "xMaxYMinBAD slice");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == 0.0f) && (viewport.fY == 0.0f) && (viewport.fW == 64.0f) && (viewport.fH == 32.0f), "SVG aspect align prefix match") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_units, (int)strlen(svg_units));
	if ( !check(ret == XGE_OK, "SVG units load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fW > 95.9f) && (viewbox.fW < 96.1f) && (viewbox.fH > 95.9f) && (viewbox.fH < 96.1f), "SVG root length units") ) return 0;
	ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 192.0f, 96.0f}, &viewport);
	if ( !check((ret == XGE_OK) && (viewport.fX > 47.9f) && (viewport.fX < 48.1f) && (viewport.fY == 0.0f) && (viewport.fW > 95.9f) && (viewport.fW < 96.1f) && (viewport.fH > 95.9f) && (viewport.fH < 96.1f), "SVG clear resets aspect") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_units, (int)strlen(svg_font_units));
	if ( !check(ret == XGE_OK, "SVG font units load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fW > 71.9f) && (viewbox.fW < 72.1f) && (viewbox.fH > 23.9f) && (viewbox.fH < 24.1f), "SVG em/ex root length units") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_region, (int)strlen(svg_filter_region));
	if ( !check(ret == XGE_OK, "SVG filter region parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_shapes, (int)strlen(svg_clip_shapes));
	if ( !check(ret == XGE_OK, "SVG vector clip shape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_shapes, (int)strlen(svg_mask_shapes));
	if ( !check(ret == XGE_OK, "SVG vector mask shape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_order_markers, (int)strlen(svg_paint_order_markers));
	if ( !check(ret == XGE_OK, "SVG paint-order markers parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_order_invalid, (int)strlen(svg_paint_order_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid paint-order parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_invalid, (int)strlen(svg_paint_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid paint parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_width_invalid, (int)strlen(svg_stroke_width_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-width parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dasharray_invalid, (int)strlen(svg_stroke_dasharray_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-dasharray parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dashoffset_invalid, (int)strlen(svg_stroke_dashoffset_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-dashoffset parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_miterlimit_invalid, (int)strlen(svg_stroke_miterlimit_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-miterlimit parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_line_enum_invalid, (int)strlen(svg_stroke_line_enum_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke line enum parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_mixed_content, (int)strlen(svg_marker_mixed_content));
	if ( !check(ret == XGE_OK, "SVG marker mixed content parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_markers, (int)strlen(svg_use_markers));
	if ( !check(ret == XGE_OK, "SVG use markers parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_inherit, (int)strlen(svg_use_inherit));
	if ( !check(ret == XGE_OK, "SVG use inherited paint parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_group_use, (int)strlen(svg_group_use));
	if ( !check(ret == XGE_OK, "SVG group use parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_defs_svg_use, (int)strlen(svg_defs_svg_use));
	if ( !check(ret == XGE_OK, "SVG defs svg use parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_gradient_stop_offset_invalid, (int)strlen(svg_gradient_stop_offset_invalid));
	if ( !check(ret == XGE_OK, "SVG gradient invalid stop offset parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_opacity_invalid, (int)strlen(svg_opacity_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid opacity parse") ) return 0;

	ret = xgeImageSavePNG(external_child_png_path, 2, 2, external_png_pixels, 8);
	if ( !check(ret == XGE_OK, "SVG external PNG file write") ) return 0;
	file = fopen(external_child_svg_path, "wb");
	if ( !check(file != NULL, "SVG external child file open") ) return 0;
	if ( fwrite(svg_external_child, 1, strlen(svg_external_child), file) != strlen(svg_external_child) ) {
		fclose(file);
		printf("xge smoke failed: SVG external child file write\n");
		return 0;
	}
	fclose(file);
	file = fopen(external_path, "wb");
	if ( !check(file != NULL, "SVG external parent file open") ) return 0;
	if ( fwrite(svg_external_parent, 1, strlen(svg_external_parent), file) != strlen(svg_external_parent) ) {
		fclose(file);
		printf("xge smoke failed: SVG external parent file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoad(svg, external_path);
	if ( !check(ret == XGE_OK, "SVG external image load") ) return 0;
	file = fopen(external_use_child_path, "wb");
	if ( !check(file != NULL, "SVG external use child file open") ) return 0;
	if ( fwrite(svg_external_use_child, 1, strlen(svg_external_use_child), file) != strlen(svg_external_use_child) ) {
		fclose(file);
		printf("xge smoke failed: SVG external use child file write\n");
		return 0;
	}
	fclose(file);
	file = fopen(external_use_path, "wb");
	if ( !check(file != NULL, "SVG external use parent file open") ) return 0;
	if ( fwrite(svg_external_use_parent, 1, strlen(svg_external_use_parent), file) != strlen(svg_external_use_parent) ) {
		fclose(file);
		printf("xge smoke failed: SVG external use parent file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoad(svg, external_use_path);
	if ( !check(ret == XGE_OK, "SVG external use root load") ) return 0;

	file = fopen(cache_path, "wb");
	if ( !check(file != NULL, "SVG cache file open") ) return 0;
	if ( fwrite(svg_text, 1, strlen(svg_text), file) != strlen(svg_text) ) {
		fclose(file);
		printf("xge smoke failed: SVG cache file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoadCached(cache_path, &cached_a);
	if ( !check((ret == XGE_OK) && (cached_a != NULL), "SVG cached load A") ) return 0;
	ret = xgeSvgLoadCached(cache_path, &cached_b);
	if ( !check((ret == XGE_OK) && (cached_b == cached_a), "SVG cached load B") ) return 0;
	ret = xgeSvgCacheInvalidate(cache_path);
	if ( !check(ret == XGE_OK, "SVG cache invalidate") ) return 0;
	ret = xgeSvgLoadCached(cache_path, &cached_c);
	if ( !check((ret == XGE_OK) && (cached_c != NULL), "SVG cached reload") ) return 0;
	xgeSvgDestroy(cached_a);
	xgeSvgDestroy(cached_b);
	xgeSvgDestroy(cached_c);
	xgeSvgDestroy(svg);
	xgeSvgCacheClear();
	remove(cache_path);
	remove(external_path);
	remove(external_child_svg_path);
	remove(external_child_png_path);
	remove(external_use_path);
	remove(external_use_child_path);
#undef XGE_TEST_RASTER_PNG
	return 1;
}

int main(void)
{
	uint32_t color;
	xge_color_t unpacked;
	xge_frame_stats_t stats;

	color = xgeColorRGBA(1, 2, 3, 4);
	if ( !check(color == 0x01020304u, "color pack") ) return 1;
	unpacked = xgeColorUnpack(color);
	if ( !check((unpacked.fR > 0.003f) && (unpacked.fR < 0.005f) &&
	            (unpacked.fG > 0.007f) && (unpacked.fG < 0.009f) &&
	            (unpacked.fB > 0.011f) && (unpacked.fB < 0.013f) &&
	            (unpacked.fA > 0.015f) && (unpacked.fA < 0.017f), "color unpack") ) return 1;
	stats = xgeFrameStatsGet();
	if ( !check(stats.iFrameCount == 0, "uninitialized frame count") ) return 1;
	if ( !test_shape_ex() ) return 1;
	if ( !test_svg() ) return 1;
	printf("xge smoke passed\n");
	return 0;
}
