#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static const char g_svg[] =
	"<svg id='root' xmlns='http://www.w3.org/2000/svg' width='160' height='100'>"
	"<defs>"
	"<rect id='source' x='0' y='0' width='12' height='8'/>"
	"<filter id='blur'><feGaussianBlur stdDeviation='1'/></filter>"
	"<clipPath id='leaf-clip'><rect x='42' y='83' width='4' height='3'/></clipPath>"
	"<clipPath id='group-clip-def'><rect x='66' y='83' width='4' height='3'/></clipPath>"
	"<clipPath id='combo-clip'><rect x='90' y='83' width='4' height='3'/></clipPath>"
	"<clipPath id='multi-clip'><rect x='104' y='82' width='8' height='6'/></clipPath>"
	"<clipPath id='transformed-clip' transform='translate(2 0)'><rect x='114' y='83' width='4' height='3'/></clipPath>"
	"<mask id='leaf-mask' maskUnits='userSpaceOnUse' maskContentUnits='userSpaceOnUse' x='54' y='83' width='4' height='3'><rect x='54' y='83' width='4' height='3' fill='white'/></mask>"
	"<mask id='group-mask-def' maskUnits='userSpaceOnUse' maskContentUnits='userSpaceOnUse' x='78' y='83' width='4' height='3'><rect x='78' y='83' width='4' height='3' fill='white'/></mask>"
	"<mask id='combo-mask' maskUnits='userSpaceOnUse' maskContentUnits='userSpaceOnUse' x='88' y='82' width='8' height='6'><rect x='88' y='82' width='8' height='6' fill='white'/></mask>"
	"<mask id='bbox-mask' maskUnits='objectBoundingBox' maskContentUnits='objectBoundingBox' x='0' y='0' width='.5' height='1'><rect x='0' y='0' width='.5' height='1' fill='white'/></mask>"
	"</defs>"
	"<rect id='first' x='1' y='2' width='20' height='10'/>"
	"<g id='group'><circle id='inside' cx='35' cy='12' r='6'/></g>"
	"<use id='instance' href='#source' x='50' y='4'/>"
	"<rect id='filtered' x='70' y='2' width='20' height='10' filter='url(#blur)'/>"
	"<g id='filtered-group' filter='url(#blur)'><rect id='filtered-child' x='94' y='2' width='8' height='8'/></g>"
	"<path id='donut' fill-rule='evenodd' d='M112 20h32v32h-32zM120 28h16v16h-16z'/>"
	"<rect id='rotshape' x='0' y='0' width='10' height='6' transform='translate(20 60) rotate(30)'/>"
	"<g id='rotgroup' transform='translate(60 60) rotate(30)'><rect id='rotchild' x='0' y='0' width='10' height='6'/></g>"
	"<g id='outer' transform='translate(78 60) rotate(20)'><g id='inner' transform='rotate(15) scale(1.5 1)'><rect x='0' y='0' width='10' height='6'/></g></g>"
	"<use id='rotuse' href='#source' opacity='.6' transform='translate(112 60) rotate(30)'/>"
	"<rect id='state-shape' x='2' y='82' width='8' height='6' opacity='.3' transform='translate(4 0)'/>"
	"<g id='state-group' opacity='.4' transform='translate(20 82) rotate(10)'><rect x='0' y='0' width='8' height='6'/></g>"
	"<rect id='clipped' x='40' y='82' width='8' height='6' clip-path='url(#leaf-clip)'/>"
	"<rect id='masked' x='52' y='82' width='8' height='6' mask='url(#leaf-mask)'/>"
	"<g id='clip-group' clip-path='url(#group-clip-def)'><rect x='64' y='82' width='8' height='6'/></g>"
	"<g id='mask-group' mask='url(#group-mask-def)'><rect x='76' y='82' width='8' height='6'/></g>"
	"<rect id='clip-mask-combo' x='88' y='82' width='8' height='6' clip-path='url(#combo-clip)' mask='url(#combo-mask)'/>"
	"<g id='multi-clip-group' clip-path='url(#multi-clip)'><rect x='102' y='82' width='6' height='6'/><rect x='108' y='82' width='6' height='6'/></g>"
	"<rect id='transformed-clipped' x='114' y='82' width='8' height='6' clip-path='url(#transformed-clip)'/>"
	"<rect id='bbox-masked' x='132' y='82' width='8' height='6' mask='url(#bbox-mask)'/>"
	"<g id='zero' opacity='0'><rect id='zero-child' x='104' y='2' width='8' height='8'/></g>"
	"<g id='empty'/>"
	"<g id='duplicate'><rect x='100' y='2' width='10' height='10'/></g>"
	"<circle id='duplicate' cx='130' cy='8' r='5'/>"
	"</svg>";

static int normalized_type(int type)
{
	if ( type == XGE_SVG_PAINT_SHAPE ) return 1;
	if ( (type == XGE_SVG_PAINT_SCENE) || (type == XGE_SVG_PAINT_GROUP) ||
	     (type == XGE_SVG_PAINT_FILTER) ) return 2;
	if ( type == XGE_SVG_PAINT_TEXT ) return 4;
	return 3;
}

static int make_rect(
	float x, float y, float width, float height,
	uint32_t color, xge_shape_ex* ppShape
)
{
	xge_shape_ex shape = NULL;
	int result;

	if ( ppShape == NULL ) return 0;
	*ppShape = NULL;
	result = xgeShapeExCreate(&shape);
	if ( result == XGE_OK ) {
		result = xgeShapeExAppendRect(shape, x, y, width, height, 0.0f, 0.0f, 1);
	}
	if ( result == XGE_OK ) result = xgeShapeExFillColor(shape, color);
	if ( result != XGE_OK ) {
		xgeShapeExDestroy(shape);
		return 0;
	}
	*ppShape = shape;
	return 1;
}

static int dump_size_case(const char* label, const char* text)
{
	xge_svg svg = NULL;
	float width = 0.0f;
	float height = 0.0f;
	int ok;

	if ( xgeSvgCreate(&svg) != XGE_OK ) return 0;
	ok = xgeSvgLoadMemory(svg, text, (int)strlen(text)) == XGE_OK &&
		xgeSvgGetSize(svg, &width, &height) == XGE_OK;
	printf("size_%s %d %.0f %.0f\n", label, ok,
		floorf(width * 1000.0f + 0.5f),
		floorf(height * 1000.0f + 0.5f));
	xgeSvgDestroy(svg);
	return ok;
}

static int dump_origin_case(xge_svg svg)
{
	xge_rect_t viewport;
	float originX = 0.0f;
	float originY = 0.0f;

	if ( xgeSvgSetOrigin(svg, 0.5f, 0.25f) != XGE_OK ||
	     xgeSvgGetOrigin(svg, &originX, &originY) != XGE_OK ||
	     xgeSvgGetDrawViewport(svg, (xge_rect_t){200.0f, 100.0f, 320.0f, 200.0f}, &viewport) != XGE_OK ) return 0;
	printf("origin %.0f %.0f %.0f %.0f %.0f %.0f\n",
		floorf(originX * 1000.0f + 0.5f),
		floorf(originY * 1000.0f + 0.5f),
		floorf(viewport.fX * 1000.0f + 0.5f),
		floorf(viewport.fY * 1000.0f + 0.5f),
		floorf(viewport.fW * 1000.0f + 0.5f),
		floorf(viewport.fH * 1000.0f + 0.5f));
	return xgeSvgSetOrigin(svg, 0.0f, 0.0f) == XGE_OK;
}

static int dump_size_mutation_case(void)
{
	static const char size_svg[] =
		"<svg xmlns='http://www.w3.org/2000/svg' width='160' height='100'>"
		"<rect id='size-first' x='10' y='20' width='30' height='20' fill='#40a0e0'/>"
		"</svg>";
	xge_svg lifecycle = NULL;
	xge_svg svg = NULL;
	xge_svg source = NULL;
	xge_svg clone = NULL;
	xge_svg boundary = NULL;
	xge_svg_paint picture = NULL;
	xge_svg_paint first = NULL;
	float width = 0.0f;
	float height = 0.0f;
	xge_rect_t picture_bounds;
	xge_rect_t first_bounds;
	xge_vec2_t points[4];
	int intersects = 0;
	int i;

	if ( xgeSvgCreate(&lifecycle) != XGE_OK ) goto fail;
	{
		int before_load = xgeSvgGetSize(lifecycle, &width, &height);
		int set_before_load = xgeSvgSetSize(lifecycle, 320.0f, 200.0f);
		int get_after_set = xgeSvgGetSize(lifecycle, &width, &height);
		int load_after_set = xgeSvgLoadMemory(lifecycle, size_svg, (int)strlen(size_svg));
		int get_after_load = xgeSvgGetSize(lifecycle, &width, &height);

		printf("size_lifecycle %d %d %d %d %d %.0f %.0f\n",
			before_load == XGE_ERROR_INVALID_STATE,
			set_before_load == XGE_OK,
			get_after_set == XGE_ERROR_INVALID_STATE,
			load_after_set == XGE_OK,
			get_after_load == XGE_OK,
			floorf(width * 1000.0f + 0.5f),
			floorf(height * 1000.0f + 0.5f));
		if ( load_after_set != XGE_OK || get_after_load != XGE_OK ) goto fail;
	}
	xgeSvgDestroy(lifecycle);
	lifecycle = NULL;

	if ( xgeSvgCreate(&svg) != XGE_OK ||
	     xgeSvgLoadMemory(svg, size_svg, (int)strlen(size_svg)) != XGE_OK ||
	     xgeSvgSetSize(svg, 320.0f, 200.0f) != XGE_OK ||
	     xgeSvgGetSize(svg, &width, &height) != XGE_OK ||
	     xgeSvgPaintGetPicture(svg, &picture) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "size-first", &first) != XGE_OK ||
	     xgeSvgPaintGetBounds(picture, 0.25f, &picture_bounds) != XGE_OK ||
	     xgeSvgPaintGetBounds(first, 0.25f, &first_bounds) != XGE_OK ||
	     xgeSvgPaintIntersects(picture, (xge_rect_t){20, 40, 2, 2}, 0.25f,
		&intersects) != XGE_OK ) goto fail;
	printf("size_positive 1 %.0f %.0f %d %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n",
		floorf(width * 1000.0f + 0.5f), floorf(height * 1000.0f + 0.5f),
		intersects,
		floorf(picture_bounds.fX * 1000.0f + 0.5f),
		floorf(picture_bounds.fY * 1000.0f + 0.5f),
		floorf(picture_bounds.fW * 1000.0f + 0.5f),
		floorf(picture_bounds.fH * 1000.0f + 0.5f),
		floorf(first_bounds.fX * 1000.0f + 0.5f),
		floorf(first_bounds.fY * 1000.0f + 0.5f),
		floorf(first_bounds.fW * 1000.0f + 0.5f),
		floorf(first_bounds.fH * 1000.0f + 0.5f));

	{
		int repeat_set = xgeSvgSetSize(svg, 80.0f, 50.0f);
		int repeat_get = xgeSvgGetSize(svg, &width, &height);
		int repeat_picture_bounds = xgeSvgPaintGetBounds(picture, 0.25f, &picture_bounds);
		int repeat_first_bounds = xgeSvgPaintGetBounds(first, 0.25f, &first_bounds);
		int repeat_picture_obb = xgeSvgPaintGetOBB(picture, 0.25f, points);
		int repeat_first_obb = xgeSvgPaintGetOBB(first, 0.25f, points);
		int repeat_bounds = xgeSvgGetBounds(svg, 0.25f, &first_bounds);
		int repeat_intersects = xgeSvgPaintIntersects(
			picture, (xge_rect_t){5, 10, 2, 2}, 0.25f, &intersects
		);

		printf("size_repeat_status %d %d %d %d %d %d %d %d\n",
			repeat_set == XGE_OK, repeat_get == XGE_OK,
			repeat_picture_bounds == XGE_OK, repeat_first_bounds == XGE_OK,
			repeat_picture_obb == XGE_OK, repeat_first_obb == XGE_OK,
			repeat_bounds == XGE_OK, repeat_intersects == XGE_OK);
		if ( repeat_set != XGE_OK || repeat_get != XGE_OK ||
		     repeat_picture_bounds != XGE_OK || repeat_first_bounds != XGE_OK ) goto fail;
		if ( xgeSvgPaintGetBounds(first, 0.25f, &first_bounds) != XGE_OK ) goto fail;
	}
	printf("size_repeat %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n",
		floorf(width * 1000.0f + 0.5f), floorf(height * 1000.0f + 0.5f),
		floorf(picture_bounds.fX * 1000.0f + 0.5f),
		floorf(picture_bounds.fY * 1000.0f + 0.5f),
		floorf(picture_bounds.fW * 1000.0f + 0.5f),
		floorf(picture_bounds.fH * 1000.0f + 0.5f),
		floorf(first_bounds.fX * 1000.0f + 0.5f),
		floorf(first_bounds.fY * 1000.0f + 0.5f),
		floorf(first_bounds.fW * 1000.0f + 0.5f),
		floorf(first_bounds.fH * 1000.0f + 0.5f));
	xgeSvgDestroy(svg);
	svg = NULL;

	if ( xgeSvgCreate(&svg) != XGE_OK ||
	     xgeSvgLoadMemory(svg, size_svg, (int)strlen(size_svg)) != XGE_OK ||
	     xgeSvgSetSize(svg, 320.0f, 200.0f) != XGE_OK ||
	     xgeSvgSetOrigin(svg, 0.25f, 0.5f) != XGE_OK ||
	     xgeSvgPaintGetPicture(svg, &picture) != XGE_OK ||
	     xgeSvgPaintTransformTranslate(picture, 200.0f, 100.0f) != XGE_OK ||
	     xgeSvgPaintGetBounds(picture, 0.25f, &picture_bounds) != XGE_OK ||
	     xgeSvgPaintGetOBB(picture, 0.25f, points) != XGE_OK ) goto fail;
	printf("size_origin_bounds %.0f %.0f %.0f %.0f",
		floorf(picture_bounds.fX * 1000.0f + 0.5f),
		floorf(picture_bounds.fY * 1000.0f + 0.5f),
		floorf(picture_bounds.fW * 1000.0f + 0.5f),
		floorf(picture_bounds.fH * 1000.0f + 0.5f));
	for ( i = 0; i < 4; i++ ) {
		printf(" %.0f %.0f", floorf(points[i].fX * 1000.0f + 0.5f),
			floorf(points[i].fY * 1000.0f + 0.5f));
	}
	printf("\n");
	xgeSvgDestroy(svg);
	svg = NULL;

	if ( xgeSvgCreate(&source) != XGE_OK ||
	     xgeSvgLoadMemory(source, size_svg, (int)strlen(size_svg)) != XGE_OK ||
	     xgeSvgSetSize(source, 300.0f, 150.0f) != XGE_OK ||
	     xgeSvgClone(source, &clone) != XGE_OK ||
	     xgeSvgSetSize(source, 80.0f, 40.0f) != XGE_OK ||
	     xgeSvgGetSize(clone, &width, &height) != XGE_OK ||
	     xgeSvgPaintGetPicture(clone, &picture) != XGE_OK ||
	     xgeSvgPaintGetByName(clone, "size-first", &first) != XGE_OK ||
	     xgeSvgPaintGetBounds(picture, 0.25f, &picture_bounds) != XGE_OK ||
	     xgeSvgPaintGetBounds(first, 0.25f, &first_bounds) != XGE_OK ) goto fail;
	printf("size_clone %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n",
		floorf(width * 1000.0f + 0.5f), floorf(height * 1000.0f + 0.5f),
		floorf(picture_bounds.fX * 1000.0f + 0.5f),
		floorf(picture_bounds.fY * 1000.0f + 0.5f),
		floorf(picture_bounds.fW * 1000.0f + 0.5f),
		floorf(picture_bounds.fH * 1000.0f + 0.5f),
		floorf(first_bounds.fX * 1000.0f + 0.5f),
		floorf(first_bounds.fY * 1000.0f + 0.5f),
		floorf(first_bounds.fW * 1000.0f + 0.5f),
		floorf(first_bounds.fH * 1000.0f + 0.5f));
	xgeSvgDestroy(clone);
	clone = NULL;
	xgeSvgDestroy(source);
	source = NULL;

	if ( xgeSvgCreate(&boundary) != XGE_OK ||
	     xgeSvgLoadMemory(boundary, size_svg, (int)strlen(size_svg)) != XGE_OK ) goto fail;
	{
		int zero_set = xgeSvgSetSize(boundary, 0.0f, 0.0f);
		int zero_get = xgeSvgGetSize(boundary, &width, &height);
		int zero_preserved = width == 0.0f && height == 0.0f;
		int negative_set = xgeSvgSetSize(boundary, -4.0f, -7.0f);
		int negative_get = xgeSvgGetSize(boundary, &width, &height);
		int negative_preserved = width == -4.0f && height == -7.0f;
		int nonfinite_set = xgeSvgSetSize(boundary, NAN, INFINITY);

		printf("size_boundary %d %d %d %d %d %d %d %d %d\n",
			zero_set == XGE_OK, zero_get == XGE_OK, zero_preserved,
			negative_set == XGE_OK, negative_get == XGE_OK, negative_preserved,
			nonfinite_set == XGE_ERROR_INVALID_ARGUMENT, 1, 1);
	}
	xgeSvgDestroy(boundary);
	return 1;

fail:
	xgeSvgDestroy(boundary);
	xgeSvgDestroy(clone);
	xgeSvgDestroy(source);
	xgeSvgDestroy(svg);
	xgeSvgDestroy(lifecycle);
	return 0;
}

static int dump_paint(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_shape_ex shape = NULL;
	uint32_t id = 0;
	int type = 0;
	int found = xgeSvgPaintGetByName(svg, name, &paint) == XGE_OK;

	if ( !found ) {
		printf("%s 0 0 0 0\n", name);
		return 1;
	}
	if ( xgeSvgPaintIdGet(paint, &id) != XGE_OK ||
	     xgeSvgPaintTypeGet(paint, &type) != XGE_OK ) return 0;
	printf("%s 1 %d %d %d\n", name, normalized_type(type),
		id == xgeShapeExIdFromName(name), xgeSvgPaintShapeGet(paint, &shape) == XGE_OK);
	return 1;
}

static int dump_name(xge_svg svg, const char* label, const char* name)
{
	xge_svg_paint paint = NULL;
	const char* resolved = NULL;
	int found = xgeSvgPaintGetByName(svg, name, &paint) == XGE_OK &&
		xgeSvgPaintNameGet(paint, &resolved) == XGE_OK;

	printf("name_%s %d %s\n", label, found, found ? resolved : "-");
	return 1;
}

static int dump_hierarchy(xge_svg svg, const char* name, const char* parentName)
{
	xge_svg_paint paint = NULL;
	xge_svg_paint parent = NULL;
	xge_svg_paint child = NULL;
	xge_svg owner = NULL;
	uint32_t parentId = 0;
	int childCount = -1;
	int childType = 0;
	int parentMatches;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintOwnerGet(paint, &owner) != XGE_OK ||
	     xgeSvgPaintParentGet(paint, &parent) != XGE_OK ||
	     xgeSvgPaintChildGetCount(paint, &childCount) != XGE_OK ) return 0;
	if ( parentName == NULL ) {
		int parentType = 0;

		parentMatches = (parent != NULL) &&
			xgeSvgPaintTypeGet(parent, &parentType) == XGE_OK &&
			xgeSvgPaintIdGet(parent, &parentId) == XGE_OK &&
			parentType == XGE_SVG_PAINT_SCENE && parentId == 0;
	} else {
		parentMatches = (parent != NULL) &&
			xgeSvgPaintIdGet(parent, &parentId) == XGE_OK &&
			parentId == xgeShapeExIdFromName(parentName);
	}
	if ( childCount > 0 && xgeSvgPaintChildGetAt(paint, 0, &child) == XGE_OK &&
	     xgeSvgPaintTypeGet(child, &childType) == XGE_OK ) {
		childType = normalized_type(childType);
	} else {
		childType = 0;
	}
	printf("hier_%s %d %d %d %d\n", name, parentMatches, childCount, childType, owner == svg);
	return 1;
}

static int dump_bounds(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_rect_t bounds;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintGetBounds(paint, 0.25f, &bounds) != XGE_OK ) return 0;
	printf("bounds_%s %.0f %.0f %.0f %.0f\n", name,
		floorf(bounds.fX * 1000.0f + 0.5f), floorf(bounds.fY * 1000.0f + 0.5f),
		floorf(bounds.fW * 1000.0f + 0.5f), floorf(bounds.fH * 1000.0f + 0.5f));
	return 1;
}

static int dump_intersects(xge_svg svg, const char* label, const char* name, xge_rect_t rect)
{
	xge_svg_paint paint = NULL;
	int intersects = -1;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintIntersects(paint, rect, 0.25f, &intersects) != XGE_OK ) return 0;
	printf("intersects_%s %d\n", label, intersects);
	return 1;
}

static int dump_obb(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_vec2_t points[4];

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintGetOBB(paint, 0.25f, points) != XGE_OK ) return 0;
	printf("obb_%s", name);
	for ( int i = 0; i < 4; i++ ) {
		printf(" %.0f %.0f", floorf(points[i].fX * 1000.0f + 0.5f),
			floorf(points[i].fY * 1000.0f + 0.5f));
	}
	printf("\n");
	return 1;
}

static int dump_state(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_shape_ex_matrix_t matrix;
	float opacity = -1.0f;
	int visible = -1;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintTransformGet(paint, &matrix) != XGE_OK ||
	     xgeSvgPaintOpacityGet(paint, &opacity) != XGE_OK ||
	     xgeSvgPaintVisibleGet(paint, &visible) != XGE_OK ) return 0;
	printf("state_%s %.0f %.0f %.0f %.0f %.0f %.0f %.0f %d\n", name,
		floorf(matrix.fA * 1000.0f + 0.5f), floorf(matrix.fB * 1000.0f + 0.5f),
		floorf(matrix.fC * 1000.0f + 0.5f), floorf(matrix.fD * 1000.0f + 0.5f),
		floorf(matrix.fE * 1000.0f + 0.5f), floorf(matrix.fF * 1000.0f + 0.5f),
		floorf(opacity * 100.0f + 0.5f), visible);
	return 1;
}

static int dump_composition(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_shape_ex maskShape = NULL;
	xge_shape_ex_scene maskScene = NULL;
	xge_shape_ex clipShape = NULL;
	xge_rect_t maskBounds = {0};
	xge_rect_t clipBounds = {0};
	xge_rect_t clipRect = {0};
	int maskMethod = 0;
	int maskType = 0;
	int clipRectEnabled = 0;
	int clipCount = 0;
	int clipPresent = 0;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintMaskGet(paint, &maskMethod, &maskType, &maskShape, &maskScene) != XGE_OK ||
	     xgeSvgPaintClipRectGet(paint, &clipRect, &clipRectEnabled) != XGE_OK ||
	     xgeSvgPaintClipShapeGetCount(paint, &clipCount) != XGE_OK ) return 0;
	if ( maskType == XGE_SHAPE_EX_MASK_TARGET_SHAPE ) {
		xgeShapeExGetBounds(maskShape, 0.25f, &maskBounds);
	} else if ( maskType == XGE_SHAPE_EX_MASK_TARGET_SCENE ) {
		xgeShapeExSceneGetBounds(maskScene, 0.25f, &maskBounds);
	}
	if ( clipRectEnabled ) {
		clipPresent = 1;
		clipBounds = clipRect;
	} else if ( clipCount > 0 &&
	            xgeSvgPaintClipShapeGetAt(paint, 0, &clipShape) == XGE_OK ) {
		clipPresent = 1;
		xgeShapeExGetBounds(clipShape, 0.25f, &clipBounds);
	}
	printf("composition_%s %d %d %.0f %.0f %.0f %.0f %d %.0f %.0f %.0f %.0f\n",
		name, maskMethod, maskType,
		floorf(maskBounds.fX * 1000.0f + 0.5f), floorf(maskBounds.fY * 1000.0f + 0.5f),
		floorf(maskBounds.fW * 1000.0f + 0.5f), floorf(maskBounds.fH * 1000.0f + 0.5f),
		clipPresent,
		floorf(clipBounds.fX * 1000.0f + 0.5f), floorf(clipBounds.fY * 1000.0f + 0.5f),
		floorf(clipBounds.fW * 1000.0f + 0.5f), floorf(clipBounds.fH * 1000.0f + 0.5f));
	return 1;
}

static int dump_clip_mode(xge_svg svg, const char* name)
{
	xge_svg_paint paint = NULL;
	xge_shape_ex clip = NULL;
	int count = 0;
	int mode = -1;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintClipShapeGetCount(paint, &count) != XGE_OK ) return 0;
	if ( count > 0 &&
	     xgeSvgPaintClipShapeGetAtEx(paint, 0, &clip, &mode) != XGE_OK ) return 0;
	printf("clipmode_%s %d %d %d\n", name, count,
		mode == XGE_SHAPE_EX_CLIP_INTERSECT, clip != NULL);
	return 1;
}

typedef struct traversal_state_t {
	uint32_t hash;
	uint32_t ids[64];
	int types[64];
	int count;
	int limit;
} traversal_state_t;

static int visit_paint(xge_svg_paint paint, void* user)
{
	traversal_state_t* state = (traversal_state_t*)user;
	uint32_t id = 0;
	int type = 0;

	if ( xgeSvgPaintIdGet(paint, &id) != XGE_OK ||
	     xgeSvgPaintTypeGet(paint, &type) != XGE_OK ) return 0;
	state->hash = state->hash * 33u + id;
	state->hash = state->hash * 33u + (uint32_t)normalized_type(type);
	if ( state->count < 64 ) {
		state->ids[state->count] = id;
		state->types[state->count] = normalized_type(type);
	}
	state->count++;
	return state->limit <= 0 || state->count < state->limit;
}

static int dump_traversal(xge_svg svg, const char* label, const char* name, int limit)
{
	xge_svg_paint paint = NULL;
	traversal_state_t state = {0};
	int i;

	state.hash = 5381u;
	state.limit = limit;

	if ( xgeSvgPaintGetByName(svg, name, &paint) != XGE_OK ||
	     xgeSvgPaintTraverse(paint, visit_paint, &state) != XGE_OK ) return 0;
	printf("traverse_%s %d %u", label, state.count, state.hash);
	for ( i = 0; i < state.count && i < 64; i++ ) {
		printf(" %d:%u", state.types[i], state.ids[i]);
	}
	printf("\n");
	return 1;
}

static int dump_root_chain(xge_svg svg)
{
	xge_svg_paint root = NULL;
	xge_svg_paint document = NULL;
	xge_svg_paint content = NULL;
	xge_svg_paint rootScene = NULL;
	xge_svg_paint picture = NULL;
	int documentType = 0;
	int contentType = 0;
	int rootSceneType = 0;
	int pictureType = 0;
	int documentChildren = -1;
	int contentChildren = -1;
	int rootSceneChildren = -1;
	int pictureChildren = -1;

	if ( xgeSvgPaintGetByName(svg, "root", &root) != XGE_OK ||
	     xgeSvgPaintParentGet(root, &document) != XGE_OK || document == NULL ||
	     xgeSvgPaintParentGet(document, &content) != XGE_OK || content == NULL ||
	     xgeSvgPaintParentGet(content, &rootScene) != XGE_OK || rootScene == NULL ||
	     xgeSvgPaintParentGet(rootScene, &picture) != XGE_OK || picture == NULL ||
	     xgeSvgPaintTypeGet(document, &documentType) != XGE_OK ||
	     xgeSvgPaintTypeGet(content, &contentType) != XGE_OK ||
	     xgeSvgPaintTypeGet(rootScene, &rootSceneType) != XGE_OK ||
	     xgeSvgPaintTypeGet(picture, &pictureType) != XGE_OK ||
	     xgeSvgPaintChildGetCount(document, &documentChildren) != XGE_OK ||
	     xgeSvgPaintChildGetCount(content, &contentChildren) != XGE_OK ||
	     xgeSvgPaintChildGetCount(rootScene, &rootSceneChildren) != XGE_OK ||
	     xgeSvgPaintChildGetCount(picture, &pictureChildren) != XGE_OK ) return 0;
	printf("root_chain %d %d %d %d %d %d %d %d\n", normalized_type(documentType),
		normalized_type(contentType), normalized_type(rootSceneType), normalized_type(pictureType),
		documentChildren, contentChildren, rootSceneChildren, pictureChildren);
	return 1;
}

static int dump_clone_case(void)
{
	xge_svg source = NULL;
	xge_svg clone = NULL;
	xge_svg_paint sourceFirst = NULL;
	xge_svg_paint cloneFirst = NULL;
	xge_svg_paint filtered = NULL;
	xge_svg_paint masked = NULL;
	xge_rect_t bounds;
	const char* name = NULL;
	float width = 0.0f;
	float height = 0.0f;
	float originX = 0.0f;
	float originY = 0.0f;
	int created;
	int distinct;
	int sizeCopied;
	int originCopied;
	int paintAvailable;
	int sourceReleased;
	int filterAlive;
	int maskAlive;
	int maskTargetType = XGE_SHAPE_EX_MASK_TARGET_NONE;

	if ( xgeSvgCreate(&source) != XGE_OK ||
	     xgeSvgLoadMemory(source, g_svg, (int)strlen(g_svg)) != XGE_OK ||
	     xgeSvgSetOrigin(source, 0.25f, 0.5f) != XGE_OK ||
	     xgeSvgPaintGetByName(source, "first", &sourceFirst) != XGE_OK ) return 0;
	created = xgeSvgClone(source, &clone) == XGE_OK && clone != NULL;
	distinct = created && clone != source;
	sizeCopied = created && xgeSvgGetSize(clone, &width, &height) == XGE_OK &&
		fabsf(width - 160.0f) < 0.001f && fabsf(height - 100.0f) < 0.001f;
	originCopied = created && xgeSvgSetOrigin(source, 0.0f, 0.0f) == XGE_OK &&
		xgeSvgGetOrigin(clone, &originX, &originY) == XGE_OK &&
		fabsf(originX - 0.25f) < 0.001f && fabsf(originY - 0.5f) < 0.001f;
	paintAvailable = sourceFirst != NULL && created &&
		xgeSvgPaintGetByName(clone, "first", &cloneFirst) == XGE_OK;
	xgeSvgDestroy(source);
	source = NULL;
	sourceReleased = created && xgeSvgPaintGetByName(clone, "first", &cloneFirst) == XGE_OK &&
		xgeSvgPaintNameGet(cloneFirst, &name) == XGE_OK && name != NULL &&
		strcmp(name, "first") == 0 &&
		xgeSvgPaintGetBounds(cloneFirst, 0.25f, &bounds) == XGE_OK &&
		fabsf(bounds.fX - 1.0f) < 0.001f && fabsf(bounds.fY - 2.0f) < 0.001f &&
		fabsf(bounds.fW - 20.0f) < 0.001f && fabsf(bounds.fH - 10.0f) < 0.001f;
	filterAlive = created && xgeSvgPaintGetByName(clone, "filtered", &filtered) == XGE_OK &&
		xgeSvgPaintGetBounds(filtered, 0.25f, &bounds) == XGE_OK;
	maskAlive = created && xgeSvgPaintGetByName(clone, "masked", &masked) == XGE_OK &&
		xgeSvgPaintMaskGet(masked, NULL, &maskTargetType, NULL, NULL) == XGE_OK &&
		maskTargetType != XGE_SHAPE_EX_MASK_TARGET_NONE;
	printf("clone %d %d %d %d %d %d %d %d\n", created, distinct, sizeCopied,
		originCopied, paintAvailable, sourceReleased, filterAlive, maskAlive);
	xgeSvgDestroy(clone);
	xgeSvgDestroy(source);
	return created && distinct && sizeCopied && originCopied && paintAvailable && sourceReleased &&
		filterAlive && maskAlive;
}

static int dump_mutation_case(void)
{
	xge_svg svg = NULL;
	xge_svg_paint first = NULL;
	xge_svg_paint transformed = NULL;
	xge_svg_paint group = NULL;
	xge_svg_paint transformedGroup = NULL;
	xge_svg_paint filtered = NULL;
	xge_svg_paint picture = NULL;
	xge_shape_ex_matrix_t matrix;
	float opacity = 0.0f;
	int visible = 0;
	int firstTranslate;
	int firstScale;
	int firstRotate;
	int firstOpacity;
	int firstVisible;
	int firstBlend;
	int pictureTranslate;
	int pictureScale;
	int pictureRotate;
	int pictureOpacity;
	int pictureVisible;
	int pictureBlend;
	int overrideResult;
	int overrideTranslate;
	int invalidBlend;
	int blend = -1;
	int blendGet;
	int identityResult;
	int identityGet;
	int identityTranslate;

	if ( xgeSvgCreate(&svg) != XGE_OK ||
	     xgeSvgLoadMemory(svg, g_svg, (int)strlen(g_svg)) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "first", &first) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "state-shape", &transformed) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "group", &group) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "state-group", &transformedGroup) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "filtered", &filtered) != XGE_OK ||
	     xgeSvgPaintGetPicture(svg, &picture) != XGE_OK ) {
		xgeSvgDestroy(svg);
		return 0;
	}

	firstTranslate = xgeSvgPaintTransformTranslate(first, 7.0f, 9.0f);
	firstScale = xgeSvgPaintTransformScale(first, 2.0f, 2.0f);
	firstRotate = xgeSvgPaintTransformRotate(first, 30.0f * (3.14159265358979323846f / 180.0f));
	firstOpacity = xgeSvgPaintOpacitySet(first, 128.0f / 255.0f);
	firstVisible = xgeSvgPaintVisibleSet(first, 0);
	firstBlend = xgeSvgPaintBlendSet(first, XGE_BLEND_MULTIPLY);
	if ( xgeSvgPaintTransformGet(first, &matrix) != XGE_OK ||
	     xgeSvgPaintOpacityGet(first, &opacity) != XGE_OK ||
	     xgeSvgPaintVisibleGet(first, &visible) != XGE_OK ) return 0;
	printf("mutate_first %d %d %d %d %d %d %.0f %.0f %.0f %.0f %.0f %.0f %.0f %d\n",
		firstTranslate == XGE_OK, firstScale == XGE_OK,
		firstRotate == XGE_OK, firstOpacity == XGE_OK,
		firstVisible == XGE_OK, firstBlend == XGE_OK,
		floorf(matrix.fA * 1000.0f + 0.5f), floorf(matrix.fB * 1000.0f + 0.5f),
		floorf(matrix.fC * 1000.0f + 0.5f), floorf(matrix.fD * 1000.0f + 0.5f),
		floorf(matrix.fE * 1000.0f + 0.5f), floorf(matrix.fF * 1000.0f + 0.5f),
		floorf(opacity * 255.0f + 0.5f), visible);

	printf("mutate_transformed %d %d %d\n",
		xgeSvgPaintTransformTranslate(transformed, 1.0f, 2.0f) == XGE_ERROR_INVALID_STATE,
		xgeSvgPaintTransformScale(transformed, 2.0f, 2.0f) == XGE_ERROR_INVALID_STATE,
		xgeSvgPaintTransformRotate(transformed, 15.0f * (3.14159265358979323846f / 180.0f)) == XGE_ERROR_INVALID_STATE);
	printf("mutate_group %d %d %d %d ",
		xgeSvgPaintTransformTranslate(group, 3.0f, 4.0f) == XGE_OK,
		xgeSvgPaintOpacitySet(group, 96.0f / 255.0f) == XGE_OK,
		xgeSvgPaintVisibleSet(group, 0) == XGE_OK,
		xgeSvgPaintBlendSet(group, XGE_BLEND_SCREEN) == XGE_OK);
	if ( xgeSvgPaintTransformGet(group, &matrix) != XGE_OK ||
	     xgeSvgPaintOpacityGet(group, &opacity) != XGE_OK ||
	     xgeSvgPaintVisibleGet(group, &visible) != XGE_OK ) return 0;
	printf("%.0f %.0f %.0f %d\n",
		floorf(matrix.fE * 1000.0f + 0.5f), floorf(matrix.fF * 1000.0f + 0.5f),
		floorf(opacity * 255.0f + 0.5f), visible);
	printf("mutate_transformed_group %d\n",
		xgeSvgPaintTransformTranslate(transformedGroup, 1.0f, 2.0f) == XGE_ERROR_INVALID_STATE);
	printf("mutate_filter %d %d %d\n",
		xgeSvgPaintOpacitySet(filtered, 64.0f / 255.0f) == XGE_OK,
		xgeSvgPaintVisibleSet(filtered, 0) == XGE_OK,
		xgeSvgPaintBlendSet(filtered, XGE_BLEND_OVERLAY) == XGE_OK);

	pictureTranslate = xgeSvgPaintTransformTranslate(picture, 11.0f, 13.0f);
	pictureScale = xgeSvgPaintTransformScale(picture, 1.5f, 1.5f);
	pictureRotate = xgeSvgPaintTransformRotate(picture, 20.0f * (3.14159265358979323846f / 180.0f));
	pictureOpacity = xgeSvgPaintOpacitySet(picture, 192.0f / 255.0f);
	pictureVisible = xgeSvgPaintVisibleSet(picture, 0);
	pictureBlend = xgeSvgPaintBlendSet(picture, XGE_BLEND_ADD);
	if ( xgeSvgPaintTransformGet(picture, &matrix) != XGE_OK ||
	     xgeSvgPaintOpacityGet(picture, &opacity) != XGE_OK ||
	     xgeSvgPaintVisibleGet(picture, &visible) != XGE_OK ) return 0;
	printf("mutate_picture %d %d %d %d %d %d %.0f %.0f %.0f %.0f %.0f %.0f %.0f %d\n",
		pictureTranslate == XGE_OK, pictureScale == XGE_OK,
		pictureRotate == XGE_OK, pictureOpacity == XGE_OK,
		pictureVisible == XGE_OK, pictureBlend == XGE_OK,
		floorf(matrix.fA * 1000.0f + 0.5f), floorf(matrix.fB * 1000.0f + 0.5f),
		floorf(matrix.fC * 1000.0f + 0.5f), floorf(matrix.fD * 1000.0f + 0.5f),
		floorf(matrix.fE * 1000.0f + 0.5f), floorf(matrix.fF * 1000.0f + 0.5f),
		floorf(opacity * 255.0f + 0.5f), visible);

	matrix = (xge_shape_ex_matrix_t){1.0f, 0.0f, 0.0f, 1.0f, 17.0f, 19.0f};
	overrideResult = xgeSvgPaintTransformSet(picture, &matrix);
	overrideTranslate = xgeSvgPaintTransformTranslate(picture, 1.0f, 2.0f);
	invalidBlend = xgeSvgPaintBlendSet(first, 254);
	printf("mutate_override %d %d %d\n",
		overrideResult == XGE_OK,
		overrideTranslate == XGE_ERROR_INVALID_STATE,
		invalidBlend == XGE_ERROR_INVALID_ARGUMENT);
	blendGet = xgeSvgPaintBlendGet(first, &blend);
	identityResult = xgeSvgPaintTransformIdentity(picture);
	identityGet = xgeSvgPaintTransformGet(picture, &matrix);
	printf("xge_contract_blend_get %d %d\n",
		blendGet == XGE_OK,
		blend == XGE_BLEND_MULTIPLY);
	printf("xge_contract_transform_identity %d %d %d\n",
		identityResult == XGE_OK,
		identityGet == XGE_OK,
		matrix.fA == 1.0f && matrix.fB == 0.0f &&
		matrix.fC == 0.0f && matrix.fD == 1.0f &&
		matrix.fE == 0.0f && matrix.fF == 0.0f);
	identityTranslate = xgeSvgPaintTransformTranslate(picture, 1.0f, 2.0f);
	printf("xge_contract_transform_identity_components %d\n",
		identityTranslate == XGE_OK);
	printf("xge_contract_state_invalid %d %d %d\n",
		xgeSvgPaintBlendGet(NULL, &blend) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintBlendGet(first, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintTransformIdentity(NULL) == XGE_ERROR_INVALID_ARGUMENT);
	xgeSvgDestroy(svg);
	return 1;
}

static int dump_composition_mutation_case(void)
{
	xge_svg svg = NULL;
	xge_svg clone = NULL;
	xge_svg_paint first = NULL;
	xge_svg_paint group = NULL;
	xge_svg_paint clipped = NULL;
	xge_svg_paint masked = NULL;
	xge_svg_paint clipGroup = NULL;
	xge_svg_paint maskGroup = NULL;
	xge_svg_paint cloneFirst = NULL;
	xge_svg_paint cloneGroup = NULL;
	xge_svg_paint cloneMaskGroup = NULL;
	xge_shape_ex clip = NULL;
	xge_shape_ex mask = NULL;
	xge_shape_ex targetShape = NULL;
	xge_shape_ex groupClip = NULL;
	xge_shape_ex sceneMaskShape = NULL;
	xge_shape_ex occupiedClip = NULL;
	xge_shape_ex occupiedMask = NULL;
	xge_shape_ex replacementClip = NULL;
	xge_shape_ex replacementMask = NULL;
	xge_shape_ex_scene sceneMask = NULL;
	xge_shape_ex_scene targetScene = NULL;
	int method = XGE_SHAPE_EX_MASK_NONE;
	int targetType = XGE_SHAPE_EX_MASK_TARGET_NONE;
	int clipCount = 0;
	int clipSet;
	int maskSet;
	int groupClipSet;
	int sceneMaskSet;
	int firstClipOccupied;
	int groupClipOccupied;
	int firstMaskOccupied;
	int groupMaskOccupied;
	int replaceClip;
	int replaceMask;
	int clearClip;
	int clearMask;
	int cloneFirstFound;
	int cloneFirstClip;
	int cloneGroupFound;
	int cloneGroupMask;
	int cloneMaskGroupFound;
	int cloneMaskGroupMask;
	int ok = 0;

	if ( xgeSvgCreate(&svg) != XGE_OK ||
	     xgeSvgLoadMemory(svg, g_svg, (int)strlen(g_svg)) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "first", &first) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "group", &group) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "clipped", &clipped) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "masked", &masked) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "clip-group", &clipGroup) != XGE_OK ||
	     xgeSvgPaintGetByName(svg, "mask-group", &maskGroup) != XGE_OK ||
	     !make_rect(3.0f, 4.0f, 7.0f, 5.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &clip) ||
	     !make_rect(2.0f, 3.0f, 9.0f, 6.0f, XGE_COLOR_RGBA(255, 255, 255, 192), &mask) ) goto cleanup;

	clipSet = xgeSvgPaintClipShapeSet(first, clip);
	maskSet = xgeSvgPaintMaskShapeSet(group, mask, XGE_SHAPE_EX_MASK_ALPHA);
	if ( xgeSvgPaintClipShapeGetCount(first, &clipCount) != XGE_OK ||
	     xgeSvgPaintMaskGet(group, &method, &targetType, &targetShape, NULL) != XGE_OK ) goto cleanup;
	printf("mutate_composition_set %d %d %d %d %d %d\n",
		clipSet == XGE_OK, maskSet == XGE_OK,
		clipCount == 1, method,
		targetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE ? 1 : 0,
		targetShape == mask);
	xgeShapeExDestroy(clip);
	clip = NULL;
	xgeShapeExDestroy(mask);
	mask = NULL;

	if ( !make_rect(64.0f, 82.0f, 6.0f, 6.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &groupClip) ||
	     xgeShapeExSceneCreate(&sceneMask) != XGE_OK ||
	     !make_rect(76.0f, 82.0f, 6.0f, 6.0f, XGE_COLOR_RGBA(255, 255, 255, 208), &sceneMaskShape) ||
	     xgeShapeExSceneAdd(sceneMask, sceneMaskShape) != XGE_OK ) goto cleanup;
	xgeShapeExDestroy(sceneMaskShape);
	sceneMaskShape = NULL;
	groupClipSet = xgeSvgPaintClipShapeSet(clipGroup, groupClip);
	sceneMaskSet = xgeSvgPaintMaskSceneSet(maskGroup, sceneMask, XGE_SHAPE_EX_MASK_LUMA);
	xgeShapeExDestroy(groupClip);
	groupClip = NULL;
	xgeShapeExSceneDestroy(sceneMask);
	sceneMask = NULL;
	clipCount = 0;
	method = XGE_SHAPE_EX_MASK_NONE;
	targetType = XGE_SHAPE_EX_MASK_TARGET_NONE;
	targetScene = NULL;
	if ( xgeSvgPaintClipShapeGetCount(clipGroup, &clipCount) != XGE_OK ||
	     xgeSvgPaintMaskGet(maskGroup, &method, &targetType, NULL, &targetScene) != XGE_OK ) goto cleanup;
	printf("mutate_composition_scene %d %d %d %d %d\n",
		groupClipSet == XGE_OK,
		sceneMaskSet == XGE_OK,
		clipCount == 1,
		targetType == XGE_SHAPE_EX_MASK_TARGET_SCENE ? 2 : 0,
		method);

	if ( !make_rect(0.0f, 0.0f, 2.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &occupiedClip) ||
	     !make_rect(0.0f, 0.0f, 2.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &occupiedMask) ) goto cleanup;
	firstClipOccupied = xgeSvgPaintClipShapeSet(first, occupiedClip);
	groupClipOccupied = xgeSvgPaintClipShapeSet(group, occupiedClip);
	firstMaskOccupied = xgeSvgPaintMaskShapeSet(first, occupiedMask, XGE_SHAPE_EX_MASK_ALPHA);
	groupMaskOccupied = xgeSvgPaintMaskShapeSet(group, occupiedMask, XGE_SHAPE_EX_MASK_LUMA);
	printf("mutate_composition_occupied %d %d %d %d\n",
		firstClipOccupied == XGE_OK,
		groupClipOccupied == XGE_ERROR_INVALID_ARGUMENT,
		firstMaskOccupied == XGE_OK,
		groupMaskOccupied == XGE_ERROR_INVALID_ARGUMENT);
	xgeShapeExDestroy(occupiedClip);
	occupiedClip = NULL;
	xgeShapeExDestroy(occupiedMask);
	occupiedMask = NULL;

	if ( !make_rect(5.0f, 6.0f, 8.0f, 4.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &replacementClip) ||
	     !make_rect(6.0f, 7.0f, 5.0f, 3.0f, XGE_COLOR_RGBA(255, 255, 255, 255), &replacementMask) ) goto cleanup;
	replaceClip = xgeSvgPaintClipShapeSet(clipped, replacementClip);
	replaceMask = xgeSvgPaintMaskShapeSet(masked, replacementMask, XGE_SHAPE_EX_MASK_LUMA);
	xgeShapeExDestroy(replacementClip);
	replacementClip = NULL;
	xgeShapeExDestroy(replacementMask);
	replacementMask = NULL;
	clipCount = 0;
	method = XGE_SHAPE_EX_MASK_NONE;
	if ( xgeSvgPaintClipShapeGetCount(clipped, &clipCount) != XGE_OK ||
	     xgeSvgPaintMaskGet(masked, &method, NULL, NULL, NULL) != XGE_OK ) goto cleanup;
	printf("mutate_composition_replace %d %d %d %d\n",
		replaceClip == XGE_OK, replaceMask == XGE_OK,
		clipCount == 1, method);

	clearClip = xgeSvgPaintClipClear(clipped);
	clearMask = xgeSvgPaintMaskClear(masked);
	clipCount = -1;
	method = -1;
	targetType = -1;
	targetShape = (xge_shape_ex)(uintptr_t)1;
	if ( xgeSvgPaintClipShapeGetCount(clipped, &clipCount) != XGE_OK ||
	     xgeSvgPaintMaskGet(masked, &method, &targetType, &targetShape, NULL) != XGE_OK ) goto cleanup;
	printf("mutate_composition_clear %d %d %d %d\n",
		clearClip == XGE_OK, clearMask == XGE_OK,
		clipCount == 0,
		method == XGE_SHAPE_EX_MASK_NONE &&
		targetType == XGE_SHAPE_EX_MASK_TARGET_NONE && targetShape == NULL);

	if ( xgeSvgClone(svg, &clone) != XGE_OK ) goto cleanup;
	clipCount = 0;
	method = XGE_SHAPE_EX_MASK_NONE;
	targetType = XGE_SHAPE_EX_MASK_TARGET_NONE;
	targetShape = NULL;
	cloneFirstFound = xgeSvgPaintGetByName(clone, "first", &cloneFirst) == XGE_OK;
	cloneFirstClip = cloneFirstFound &&
		xgeSvgPaintClipShapeGetCount(cloneFirst, &clipCount) == XGE_OK && clipCount == 1;
	cloneGroupFound = xgeSvgPaintGetByName(clone, "group", &cloneGroup) == XGE_OK;
	cloneGroupMask = cloneGroupFound &&
		xgeSvgPaintMaskGet(cloneGroup, &method, &targetType, &targetShape, NULL) == XGE_OK &&
		method == XGE_SHAPE_EX_MASK_ALPHA &&
		targetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE && targetShape != NULL;
	method = XGE_SHAPE_EX_MASK_NONE;
	targetType = XGE_SHAPE_EX_MASK_TARGET_NONE;
	targetScene = NULL;
	cloneMaskGroupFound = xgeSvgPaintGetByName(clone, "mask-group", &cloneMaskGroup) == XGE_OK;
	cloneMaskGroupMask = cloneMaskGroupFound &&
		xgeSvgPaintMaskGet(cloneMaskGroup, &method, &targetType, NULL, &targetScene) == XGE_OK &&
		method == XGE_SHAPE_EX_MASK_LUMA &&
		targetType == XGE_SHAPE_EX_MASK_TARGET_SCENE && targetScene != NULL;
	printf("mutate_composition_clone %d %d %d %d %d %d %d\n",
		clone != NULL, cloneFirstFound, cloneFirstClip, cloneGroupFound, cloneGroupMask,
		cloneMaskGroupFound, cloneMaskGroupMask);
	ok = 1;

cleanup:
	xgeShapeExSceneDestroy(sceneMask);
	xgeShapeExDestroy(sceneMaskShape);
	xgeShapeExDestroy(groupClip);
	xgeShapeExDestroy(replacementMask);
	xgeShapeExDestroy(replacementClip);
	xgeShapeExDestroy(occupiedMask);
	xgeShapeExDestroy(occupiedClip);
	xgeShapeExDestroy(mask);
	xgeShapeExDestroy(clip);
	xgeSvgDestroy(clone);
	xgeSvgDestroy(svg);
	return ok;
}

int main(void)
{
	static const char* names[] = {
		"root", "first", "group", "inside", "instance", "filtered",
		"filtered-group", "filtered-child", "zero", "zero-child", "empty",
		"duplicate", "missing"
	};
	xge_svg svg = NULL;
	xge_svg_paint paint = NULL;
	xge_svg_paint picturePaint = NULL;
	xge_svg_paint parentPaint = NULL;
	xge_svg owner = NULL;
	uint32_t pictureId = 1;
	int pictureType = 0;
	int pictureChildren = -1;
	int i;

	if ( xgeSvgCreate(&svg) != XGE_OK ||
	     xgeSvgLoadMemory(svg, g_svg, (int)strlen(g_svg)) != XGE_OK ) return 1;
	{
		int baseCount = 0;
		int addedCount = 0;
		int releasedCount = 0;

		if ( xgeSvgRefCountGet(svg, &baseCount) != XGE_OK ||
		     xgeSvgAddRef(svg) != XGE_OK ||
		     xgeSvgRefCountGet(svg, &addedCount) != XGE_OK ) return 1;
		xgeSvgDestroy(svg);
		if ( xgeSvgRefCountGet(svg, &releasedCount) != XGE_OK ) return 1;
		printf("ownership %d %d %d %d\n",
			addedCount == baseCount + 1,
			releasedCount == baseCount,
			baseCount > 0,
			releasedCount > 0);
	}
	{
		float width = 0.0f;
		float height = 0.0f;
		int alignX = -1;
		int alignY = -1;
		int meetOrSlice = -1;

		if ( xgeSvgGetSize(svg, &width, &height) != XGE_OK ||
		     xgeSvgGetPreserveAspectRatio(svg, &alignX, &alignY, &meetOrSlice) != XGE_OK ) return 1;
		printf("geometry %.0f %.0f %d %d %d\n",
			floorf(width * 1000.0f + 0.5f),
			floorf(height * 1000.0f + 0.5f),
			alignX, alignY, meetOrSlice);
	}
	if ( !dump_size_case("viewbox_only",
		"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 40 20'><rect width='40' height='20'/></svg>") ||
	     !dump_size_case("percent",
		"<svg xmlns='http://www.w3.org/2000/svg' width='50%' height='25%' viewBox='0 0 40 20'><rect width='40' height='20'/></svg>") ||
	     !dump_size_case("content_only",
		"<svg xmlns='http://www.w3.org/2000/svg'><rect x='2' y='3' width='12' height='8'/></svg>") ||
	     !dump_size_case("width_only",
		"<svg xmlns='http://www.w3.org/2000/svg' width='80' viewBox='0 0 40 20'><rect width='40' height='20'/></svg>") ||
	     !dump_size_case("width_no_viewbox",
		"<svg xmlns='http://www.w3.org/2000/svg' width='80'><rect x='2' y='3' width='12' height='8'/></svg>") ||
	     !dump_size_case("height_no_viewbox",
		"<svg xmlns='http://www.w3.org/2000/svg' height='60'><rect x='2' y='3' width='12' height='8'/></svg>") ||
	     !dump_size_case("negative_content",
		"<svg xmlns='http://www.w3.org/2000/svg'><rect x='-7' y='-5' width='12' height='8'/></svg>") ) return 1;
	if ( !dump_origin_case(svg) ) return 1;
	if ( !dump_size_mutation_case() ) return 1;
	if ( !dump_clone_case() ) return 1;
	if ( !dump_mutation_case() ) return 1;
	if ( !dump_composition_mutation_case() ) return 1;
	if ( xgeSvgPaintGetPicture(svg, &picturePaint) != XGE_OK ||
	     xgeSvgPaintIdGet(picturePaint, &pictureId) != XGE_OK ||
	     xgeSvgPaintTypeGet(picturePaint, &pictureType) != XGE_OK ||
	     xgeSvgPaintOwnerGet(picturePaint, &owner) != XGE_OK ||
	     xgeSvgPaintParentGet(picturePaint, &parentPaint) != XGE_OK ||
	     xgeSvgPaintChildGetCount(picturePaint, &pictureChildren) != XGE_OK ) return 1;
	printf("picture %d %d %d %d\n", pictureId == 0,
		normalized_type(pictureType), owner == svg && parentPaint == NULL, pictureChildren);
	for ( i = 0; i < (int)(sizeof(names) / sizeof(names[0])); i++ ) {
		if ( !dump_paint(svg, names[i]) ) return 2;
	}
	if ( !dump_name(svg, "first", "first") ||
	     !dump_name(svg, "group", "group") ||
	     !dump_name(svg, "instance", "instance") ||
	     !dump_name(svg, "duplicate", "duplicate") ||
	     !dump_name(svg, "missing", "missing") ) return 2;
	{
		const char* pictureName = NULL;
		printf("name_picture %d -\n",
			xgeSvgPaintNameGet(picturePaint, &pictureName) == XGE_OK);
	}
	if ( !dump_hierarchy(svg, "root", NULL) ||
	     !dump_hierarchy(svg, "first", "root") ||
	     !dump_hierarchy(svg, "group", "root") ||
	     !dump_hierarchy(svg, "inside", "group") ||
	     !dump_hierarchy(svg, "instance", "root") ||
	     !dump_hierarchy(svg, "filtered", "root") ||
	     !dump_hierarchy(svg, "filtered-group", "root") ||
	     !dump_hierarchy(svg, "filtered-child", "filtered-group") ||
	     !dump_hierarchy(svg, "zero", "root") ||
	     !dump_hierarchy(svg, "duplicate", "root") ) return 3;
	if ( !dump_root_chain(svg) ) return 3;
	if ( !dump_bounds(svg, "first") || !dump_bounds(svg, "group") ||
	     !dump_bounds(svg, "inside") || !dump_bounds(svg, "instance") ) return 4;
	if ( !dump_obb(svg, "first") || !dump_obb(svg, "group") ||
	     !dump_obb(svg, "rotshape") || !dump_obb(svg, "rotgroup") ||
	     !dump_obb(svg, "inner") || !dump_obb(svg, "outer") ||
	     !dump_obb(svg, "rotuse") ) return 5;
	if ( !dump_state(svg, "root") || !dump_state(svg, "state-shape") ||
	     !dump_state(svg, "state-group") || !dump_state(svg, "rotuse") ||
	     !dump_state(svg, "filtered") || !dump_state(svg, "zero") ) return 6;
	{
		xge_shape_ex_matrix_t matrix;
		float opacity = -1.0f;
		int visible = -1;
		if ( xgeSvgPaintTransformGet(picturePaint, &matrix) != XGE_OK ||
		     xgeSvgPaintOpacityGet(picturePaint, &opacity) != XGE_OK ||
		     xgeSvgPaintVisibleGet(picturePaint, &visible) != XGE_OK ) return 6;
		printf("state_picture %.0f %.0f %.0f %.0f %.0f %.0f %.0f %d\n",
			floorf(matrix.fA * 1000.0f + 0.5f), floorf(matrix.fB * 1000.0f + 0.5f),
			floorf(matrix.fC * 1000.0f + 0.5f), floorf(matrix.fD * 1000.0f + 0.5f),
			floorf(matrix.fE * 1000.0f + 0.5f), floorf(matrix.fF * 1000.0f + 0.5f),
			floorf(opacity * 100.0f + 0.5f), visible);
	}
	if ( !dump_composition(svg, "clipped") ||
	     !dump_composition(svg, "masked") ||
	     !dump_composition(svg, "clip-group") ||
	     !dump_composition(svg, "mask-group") ||
	     !dump_composition(svg, "clip-mask-combo") ||
	     !dump_composition(svg, "multi-clip-group") ||
	     !dump_composition(svg, "transformed-clipped") ||
	     !dump_composition(svg, "bbox-masked") ) return 7;
	if ( !dump_clip_mode(svg, "clipped") ||
	     !dump_clip_mode(svg, "clip-group") ||
	     !dump_clip_mode(svg, "clip-mask-combo") ||
	     !dump_clip_mode(svg, "multi-clip-group") ||
	     !dump_clip_mode(svg, "transformed-clipped") ) return 7;
	if ( !dump_traversal(svg, "group", "group", 0) ||
	     !dump_traversal(svg, "root", "root", 0) ||
	     !dump_traversal(svg, "stop", "root", 5) ) return 7;
	if ( !dump_intersects(svg, "first_hit", "first", (xge_rect_t){5, 5, 2, 2}) ||
	     !dump_intersects(svg, "first_miss", "first", (xge_rect_t){24, 5, 2, 2}) ||
	     !dump_intersects(svg, "group_hit", "group", (xge_rect_t){34, 10, 2, 2}) ||
	     !dump_intersects(svg, "group_aabb_miss", "group", (xge_rect_t){29, 6, 1, 1}) ||
	     !dump_intersects(svg, "donut_ring", "donut", (xge_rect_t){114, 22, 2, 2}) ||
	     !dump_intersects(svg, "donut_hole", "donut", (xge_rect_t){124, 32, 2, 2}) ||
	     !dump_intersects(svg, "instance_hit", "instance", (xge_rect_t){52, 6, 2, 2}) ||
	     !dump_intersects(svg, "filtered_hit", "filtered", (xge_rect_t){74, 5, 2, 2}) ||
	     !dump_intersects(svg, "zero_miss", "zero", (xge_rect_t){106, 4, 2, 2}) ) return 7;
	{
		int pictureIntersects = -1;
		if ( xgeSvgPaintIntersects(picturePaint, (xge_rect_t){5, 5, 2, 2}, 0.25f,
			&pictureIntersects) != XGE_OK ) return 7;
		printf("intersects_picture_hit %d\n", pictureIntersects);
	}
	printf("invalid %d %d %d %d %d %d\n",
		xgeSvgPaintGetById(svg, 0, &paint) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintGetByName(svg, "", &paint) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintIdGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintGetPicture(NULL, &paint) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintChildGetAt(picturePaint, 0, &paint) == XGE_ERROR_NOT_FOUND,
		xgeSvgPaintGetBounds(NULL, 0.25f, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	{
		int invalidIntersects = -1;
		printf("intersects_invalid %d %d\n",
			xgeSvgPaintIntersects(NULL, (xge_rect_t){0, 0, 1, 1}, 0.25f, &invalidIntersects) == XGE_ERROR_INVALID_ARGUMENT,
			xgeSvgPaintIntersects(picturePaint, (xge_rect_t){0, 0, 0, 1}, 0.25f, &invalidIntersects) == XGE_OK && invalidIntersects == 0);
	}
	printf("obb_invalid %d\n", xgeSvgPaintGetOBB(NULL, 0.25f, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("state_invalid %d %d %d\n",
		xgeSvgPaintTransformGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintOpacityGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintVisibleGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("name_invalid %d\n",
		xgeSvgPaintNameGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("traverse_invalid %d %d\n",
		xgeSvgPaintTraverse(NULL, visit_paint, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintTraverse(picturePaint, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("composition_invalid %d %d %d %d\n",
		xgeSvgPaintMaskGet(NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintClipRectGet(NULL, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintClipShapeGetCount(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintClipShapeGetAt(NULL, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("ownership_invalid %d\n",
		xgeSvgRefCountGet(NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("geometry_invalid %d %d %d\n",
		xgeSvgGetSize(NULL, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgGetPreserveAspectRatio(NULL, NULL, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgPaintClipShapeGetAtEx(NULL, 0, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("origin_invalid %d %d\n",
		xgeSvgSetOrigin(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgGetOrigin(NULL, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	printf("clone_invalid %d %d\n",
		xgeSvgClone(NULL, &svg) == XGE_ERROR_INVALID_ARGUMENT,
		xgeSvgClone(svg, NULL) == XGE_ERROR_INVALID_ARGUMENT);
	if ( xgeSvgClear(svg) != XGE_OK ) return 8;
	printf("cleared %d\n",
		xgeSvgPaintGetByName(svg, "first", &paint) == XGE_ERROR_NOT_FOUND);
	xgeSvgDestroy(svg);
	return 0;
}
