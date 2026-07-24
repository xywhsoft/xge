#include "../../xge.h"

#include <math.h>
#include <stdio.h>

static int print_case(const char* name, xge_shape_ex shape, float x, float y, float w, float h)
{
	int intersects = 0;
	int ret = xgeShapeExIntersects(shape, (xge_rect_t){x, y, w, h}, 0.25f, &intersects);

	if ( ret != XGE_OK ) return ret;
	printf("%s\t%d\n", name, intersects);
	return XGE_OK;
}

static int print_scene_case(const char* name, xge_shape_ex_scene scene, float x, float y, float w, float h)
{
	int intersects = 0;
	int ret = xgeShapeExSceneIntersects(scene, (xge_rect_t){x, y, w, h}, 0.25f, &intersects);

	if ( ret != XGE_OK ) return ret;
	printf("%s\t%d\n", name, intersects);
	return XGE_OK;
}

static int print_scene_ex_case(const char* name, xge_shape_ex_scene scene,
	const xge_shape_ex_matrix_t* parent, float x, float y, float w, float h)
{
	int intersects = 0;
	int ret = xgeShapeExSceneIntersectsEx(scene, (xge_rect_t){x, y, w, h}, 0.25f, parent, &intersects);

	if ( ret != XGE_OK ) return ret;
	printf("%s\t%d\n", name, intersects);
	return XGE_OK;
}

static int print_scene_contains_ex(const char* name, xge_shape_ex_scene scene,
	const xge_shape_ex_matrix_t* parent, float x, float y)
{
	int contains = 0;
	int ret = xgeShapeExSceneContainsPointEx(scene, x, y, 0.25f, parent, &contains);

	if ( ret != XGE_OK ) return ret;
	printf("%s\t%d\n", name, contains);
	return XGE_OK;
}

int main(void)
{
	xge_shape_ex rect = NULL;
	xge_shape_ex triangle = NULL;
	xge_shape_ex donut = NULL;
	xge_shape_ex stroke = NULL;
	xge_shape_ex dashed = NULL;
	xge_shape_ex transformed = NULL;
	xge_shape_ex hidden = NULL;
	xge_shape_ex zeroOpacity = NULL;
	xge_shape_ex transparent = NULL;
	xge_shape_ex clipped = NULL;
	xge_shape_ex clipper = NULL;
	xge_shape_ex masked = NULL;
	xge_shape_ex mask = NULL;
	xge_shape_ex sceneChild = NULL;
	xge_shape_ex parentChild = NULL;
	xge_shape_ex composedChild = NULL;
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex_scene parentScene = NULL;
	xge_shape_ex_scene composedScene = NULL;
	xge_shape_ex_matrix_t parentMatrix = {1, 0, 0, 1, 30, 40};
	const float dash[] = {12.0f, 12.0f};
	int ret = XGE_OK;

#define OK(call) do { ret = (call); if ( ret != XGE_OK ) goto done; } while (0)
#define CASE(name, shape, x, y, w, h) do { ret = print_case((name), (shape), (x), (y), (w), (h)); if ( ret != XGE_OK ) goto done; } while (0)

	OK(xgeShapeExCreate(&rect));
	OK(xgeShapeExAppendRect(rect, 20, 20, 40, 30, 0, 0, 1));
	OK(xgeShapeExFillColor(rect, XGE_COLOR_RGBA(220, 40, 60, 255)));

	OK(xgeShapeExCreate(&triangle));
	OK(xgeShapeExMoveTo(triangle, 80, 20));
	OK(xgeShapeExLineTo(triangle, 120, 20));
	OK(xgeShapeExLineTo(triangle, 80, 60));
	OK(xgeShapeExClose(triangle));
	OK(xgeShapeExFillColor(triangle, XGE_COLOR_RGBA(30, 180, 90, 255)));

	OK(xgeShapeExCreate(&donut));
	OK(xgeShapeExAppendRect(donut, 140, 20, 60, 60, 0, 0, 1));
	OK(xgeShapeExAppendRect(donut, 155, 35, 30, 30, 0, 0, 1));
	OK(xgeShapeExFillRule(donut, XGE_SHAPE_EX_FILL_EVEN_ODD));
	OK(xgeShapeExFillColor(donut, XGE_COLOR_RGBA(40, 100, 230, 255)));

	OK(xgeShapeExCreate(&stroke));
	OK(xgeShapeExMoveTo(stroke, 20, 100));
	OK(xgeShapeExLineTo(stroke, 100, 100));
	OK(xgeShapeExStrokeWidth(stroke, 8));
	OK(xgeShapeExStrokeColor(stroke, XGE_COLOR_RGBA(250, 180, 30, 255)));

	OK(xgeShapeExCreate(&dashed));
	OK(xgeShapeExMoveTo(dashed, 20, 125));
	OK(xgeShapeExLineTo(dashed, 100, 125));
	OK(xgeShapeExStrokeWidth(dashed, 6));
	OK(xgeShapeExStrokeColor(dashed, XGE_COLOR_RGBA(250, 180, 30, 255)));
	OK(xgeShapeExStrokeDash(dashed, dash, 2, 0));

	OK(xgeShapeExCreate(&transformed));
	OK(xgeShapeExAppendRect(transformed, 0, 0, 20, 10, 0, 0, 1));
	OK(xgeShapeExFillColor(transformed, XGE_COLOR_RGBA(150, 60, 210, 255)));
	OK(xgeShapeExTransformTranslate(transformed, 130, 105));
	OK(xgeShapeExTransformRotate(transformed, 30.0f * 3.14159265358979323846f / 180.0f));

	OK(xgeShapeExCreate(&hidden));
	OK(xgeShapeExAppendRect(hidden, 20, 155, 30, 20, 0, 0, 1));
	OK(xgeShapeExFillColor(hidden, XGE_COLOR_RGBA(10, 10, 10, 255)));
	OK(xgeShapeExVisible(hidden, 0));

	OK(xgeShapeExCreate(&zeroOpacity));
	OK(xgeShapeExAppendRect(zeroOpacity, 60, 155, 30, 20, 0, 0, 1));
	OK(xgeShapeExFillColor(zeroOpacity, XGE_COLOR_RGBA(10, 10, 10, 255)));
	OK(xgeShapeExOpacity(zeroOpacity, 0));

	OK(xgeShapeExCreate(&transparent));
	OK(xgeShapeExAppendRect(transparent, 100, 155, 30, 20, 0, 0, 1));
	OK(xgeShapeExFillColor(transparent, XGE_COLOR_RGBA(10, 10, 10, 0)));

	OK(xgeShapeExCreate(&clipped));
	OK(xgeShapeExAppendRect(clipped, 140, 145, 50, 40, 0, 0, 1));
	OK(xgeShapeExFillColor(clipped, XGE_COLOR_RGBA(10, 10, 10, 255)));
	OK(xgeShapeExCreate(&clipper));
	OK(xgeShapeExAppendRect(clipper, 140, 145, 20, 40, 0, 0, 1));
	OK(xgeShapeExClipShapeAdd(clipped, clipper));

	OK(xgeShapeExCreate(&masked));
	OK(xgeShapeExAppendRect(masked, 20, 205, 60, 30, 0, 0, 1));
	OK(xgeShapeExFillColor(masked, XGE_COLOR_RGBA(10, 10, 10, 255)));
	OK(xgeShapeExCreate(&mask));
	OK(xgeShapeExAppendRect(mask, 20, 205, 20, 30, 0, 0, 1));
	OK(xgeShapeExFillColor(mask, XGE_COLOR_RGBA(255, 255, 255, 255)));
	OK(xgeShapeExMaskShapeSet(masked, mask, XGE_SHAPE_EX_MASK_ALPHA));

	OK(xgeShapeExSceneCreate(&scene));
	OK(xgeShapeExCreate(&sceneChild));
	OK(xgeShapeExAppendCircle(sceneChild, 170, 220, 12, 12, 1));
	OK(xgeShapeExFillColor(sceneChild, XGE_COLOR_RGBA(20, 120, 200, 255)));
	OK(xgeShapeExSceneAdd(scene, sceneChild));

	OK(xgeShapeExSceneCreate(&parentScene));
	OK(xgeShapeExCreate(&parentChild));
	OK(xgeShapeExAppendRect(parentChild, 0, 0, 20, 10, 0, 0, 1));
	OK(xgeShapeExFillColor(parentChild, XGE_COLOR_RGBA(80, 160, 220, 255)));
	OK(xgeShapeExSceneAdd(parentScene, parentChild));

	OK(xgeShapeExSceneCreate(&composedScene));
	OK(xgeShapeExCreate(&composedChild));
	OK(xgeShapeExAppendRect(composedChild, 0, 0, 20, 10, 0, 0, 1));
	OK(xgeShapeExFillColor(composedChild, XGE_COLOR_RGBA(160, 80, 220, 255)));
	OK(xgeShapeExSceneAdd(composedScene, composedChild));
	OK(xgeShapeExSceneTransformTranslate(composedScene, 5, 7));

	CASE("rect_contained", rect, 0, 0, 80, 80);
	CASE("rect_partial", rect, 50, 40, 30, 30);
	CASE("rect_edge", rect, 59, 49, 2, 2);
	CASE("rect_miss", rect, 0, 0, 10, 10);
	CASE("rect_zero_width", rect, 30, 30, 0, 4);
	CASE("triangle_aabb_only", triangle, 112, 52, 4, 4);
	CASE("triangle_cross", triangle, 96, 32, 4, 4);
	CASE("donut_hole", donut, 165, 45, 8, 8);
	CASE("donut_ring", donut, 145, 45, 8, 8);
	CASE("donut_region_contains", donut, 130, 10, 80, 80);
	CASE("stroke_cross", stroke, 55, 96, 4, 8);
	CASE("stroke_near_miss", stroke, 55, 106, 4, 4);
	CASE("dash_on", dashed, 22, 122, 6, 6);
	CASE("dash_gap", dashed, 35, 122, 6, 6);
	CASE("transform_hit", transformed, 128, 105, 10, 10);
	CASE("transform_aabb_miss", transformed, 147, 105, 4, 4);
	CASE("hidden_hit", hidden, 25, 160, 5, 5);
	CASE("zero_opacity_hit", zeroOpacity, 65, 160, 5, 5);
	CASE("transparent_miss", transparent, 105, 160, 5, 5);
	CASE("clip_inside", clipped, 145, 155, 5, 5);
	CASE("clip_outside", clipped, 175, 155, 5, 5);
	CASE("mask_ignored", masked, 65, 215, 5, 5);
	OK(print_scene_case("scene_hit", scene, 165, 215, 10, 10));
	OK(print_scene_case("scene_miss", scene, 200, 215, 10, 10));
	OK(print_scene_ex_case("scene_parent_hit", parentScene, &parentMatrix, 35, 43, 2, 2));
	OK(print_scene_ex_case("scene_parent_miss", parentScene, &parentMatrix, 5, 3, 2, 2));
	OK(print_scene_contains_ex("scene_parent_contains", parentScene, &parentMatrix, 40, 45));
	OK(print_scene_ex_case("scene_composed_hit", composedScene, &parentMatrix, 40, 50, 2, 2));
	OK(print_scene_ex_case("scene_composed_miss", composedScene, &parentMatrix, 10, 10, 2, 2));
	OK(print_scene_contains_ex("scene_composed_contains", composedScene, &parentMatrix, 45, 52));

done:
	xgeShapeExSceneDestroy(composedScene);
	xgeShapeExDestroy(composedChild);
	xgeShapeExSceneDestroy(parentScene);
	xgeShapeExDestroy(parentChild);
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(sceneChild);
	xgeShapeExDestroy(masked);
	xgeShapeExDestroy(mask);
	xgeShapeExDestroy(clipped);
	xgeShapeExDestroy(clipper);
	xgeShapeExDestroy(transparent);
	xgeShapeExDestroy(zeroOpacity);
	xgeShapeExDestroy(hidden);
	xgeShapeExDestroy(transformed);
	xgeShapeExDestroy(dashed);
	xgeShapeExDestroy(stroke);
	xgeShapeExDestroy(donut);
	xgeShapeExDestroy(triangle);
	xgeShapeExDestroy(rect);
	return ret == XGE_OK ? 0 : 1;
}
