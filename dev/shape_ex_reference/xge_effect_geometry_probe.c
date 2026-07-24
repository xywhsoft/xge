#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int dump_bounds(const char* name, xge_shape_ex_scene scene)
{
	xge_rect_t bounds;

	if ( xgeShapeExSceneGetBounds(scene, 0.05f, &bounds) != XGE_OK ) return 0;
	printf("%s %.9g %.9g %.9g %.9g\n", name,
		bounds.fX, bounds.fY, bounds.fW, bounds.fH);
	return 1;
}

static int dump_obb(const char* name, xge_shape_ex_scene scene)
{
	xge_vec2_t points[4];

	if ( xgeShapeExSceneGetOBB(scene, 0.05f, points) != XGE_OK ) return 0;
	printf("%s %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", name,
		points[0].fX, points[0].fY, points[1].fX, points[1].fY,
		points[2].fX, points[2].fY, points[3].fX, points[3].fY);
	return 1;
}

static int dump_shape_bounds(const char* name, xge_shape_ex shape)
{
	xge_rect_t bounds;

	if ( xgeShapeExGetBounds(shape, 0.05f, &bounds) != XGE_OK ) return 0;
	printf("%s %.9g %.9g %.9g %.9g\n", name,
		bounds.fX, bounds.fY, bounds.fW, bounds.fH);
	return 1;
}

static int dump_shape_obb(const char* name, xge_shape_ex shape)
{
	xge_vec2_t points[4];

	if ( xgeShapeExGetOBB(shape, 0.05f, points) != XGE_OK ) return 0;
	printf("%s %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", name,
		points[0].fX, points[0].fY, points[1].fX, points[1].fY,
		points[2].fX, points[2].fY, points[3].fX, points[3].fY);
	return 1;
}

static int dump_shape_bounds_status(const char* name, xge_shape_ex shape)
{
	xge_rect_t bounds = {0.0f, 0.0f, 0.0f, 0.0f};
	int success = xgeShapeExGetBounds(shape, 0.05f, &bounds) == XGE_OK;

	if ( !success ) bounds = (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( success && (!isfinite(bounds.fX) || !isfinite(bounds.fY) ||
	     !isfinite(bounds.fW) || !isfinite(bounds.fH) ||
	     ((fabsf(bounds.fW) <= 1.0e-6f) && (fabsf(bounds.fH) <= 1.0e-6f))) ) {
		bounds = (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	printf("%s %d %.9g %.9g %.9g %.9g\n", name, success,
		bounds.fX, bounds.fY, bounds.fW, bounds.fH);
	return 1;
}

static int dump_shape_obb_status(const char* name, xge_shape_ex shape)
{
	xge_vec2_t points[4] = {{0.0f, 0.0f}};
	int success = xgeShapeExGetOBB(shape, 0.05f, points) == XGE_OK;
	int pathological = 0;
	int singlePoint = 1;
	int i;

	if ( !success ) memset(points, 0, sizeof(points));
	for ( i = 0; success && (i < 4); i++ ) {
		if ( !isfinite(points[i].fX) || !isfinite(points[i].fY) ||
		     (fabsf(points[i].fX) > 1.0e30f) || (fabsf(points[i].fY) > 1.0e30f) ) {
			pathological = 1;
		}
		if ( (fabsf(points[i].fX - points[0].fX) > 1.0e-6f) ||
		     (fabsf(points[i].fY - points[0].fY) > 1.0e-6f) ) {
			singlePoint = 0;
		}
	}
	if ( success && (pathological || singlePoint) ) memset(points, 0, sizeof(points));
	printf("%s %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", name, success,
		points[0].fX, points[0].fY, points[1].fX, points[1].fY,
		points[2].fX, points[2].fY, points[3].fX, points[3].fY);
	return 1;
}

static int dump_scene_bounds_status(const char* name, xge_shape_ex_scene scene)
{
	xge_rect_t bounds = {0.0f, 0.0f, 0.0f, 0.0f};
	int success = xgeShapeExSceneGetBounds(scene, 0.05f, &bounds) == XGE_OK;

	if ( !success ) bounds = (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	printf("%s %d %.9g %.9g %.9g %.9g\n", name, success,
		bounds.fX, bounds.fY, bounds.fW, bounds.fH);
	return 1;
}

static int dump_scene_obb_status(const char* name, xge_shape_ex_scene scene)
{
	xge_vec2_t points[4] = {{0.0f, 0.0f}};
	int success = xgeShapeExSceneGetOBB(scene, 0.05f, points) == XGE_OK;

	if ( !success ) memset(points, 0, sizeof(points));
	printf("%s %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", name, success,
		points[0].fX, points[0].fY, points[1].fX, points[1].fY,
		points[2].fX, points[2].fY, points[3].fX, points[3].fY);
	return 1;
}

static int make_rect(xge_shape_ex* shape)
{
	return xgeShapeExCreate(shape) == XGE_OK &&
		xgeShapeExAppendRect(*shape, 10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1) == XGE_OK &&
		xgeShapeExFillColor(*shape, XGE_COLOR_RGBA(74, 177, 230, 255)) == XGE_OK;
}

int main(void)
{
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex_scene root = NULL;
	xge_shape_ex_scene child = NULL;
	xge_shape_ex shape = NULL;
	xge_shape_ex childShape = NULL;
	xge_shape_ex decoratedShape = NULL;
	xge_shape_ex shapeClip = NULL;
	xge_shape_ex shapeMask = NULL;
	xge_shape_ex clippedSceneShape = NULL;
	xge_shape_ex maskedSceneShape = NULL;
	xge_shape_ex sceneClip = NULL;
	xge_shape_ex sceneMask = NULL;
	xge_shape_ex_scene clippedScene = NULL;
	xge_shape_ex_scene maskedScene = NULL;
	xge_shape_ex emptyShape = NULL;
	xge_shape_ex moveShape = NULL;
	xge_shape_ex openLine = NULL;
	xge_shape_ex strokedLine = NULL;
	xge_shape_ex buttLine = NULL;
	xge_shape_ex roundLine = NULL;
	xge_shape_ex miterPath = NULL;
	xge_shape_ex transparentRect = NULL;
	xge_shape_ex hiddenShape = NULL;
	xge_shape_ex zeroOpacityShape = NULL;
	xge_shape_ex hiddenChild = NULL;
	xge_shape_ex hiddenSceneShape = NULL;
	xge_shape_ex zeroOpacitySceneShape = NULL;
	xge_shape_ex rotatedShape = NULL;
	xge_shape_ex rotatedSceneShape = NULL;
	xge_shape_ex_scene emptyScene = NULL;
	xge_shape_ex_scene hiddenChildScene = NULL;
	xge_shape_ex_scene hiddenScene = NULL;
	xge_shape_ex_scene zeroOpacityScene = NULL;
	xge_shape_ex_scene rotatedScene = NULL;
	xge_shape_ex_matrix_t transform = {1.2f, 0.0f, 0.0f, 0.8f, 5.0f, -3.0f};

	if ( xgeShapeExSceneCreate(&scene) != XGE_OK || !make_rect(&shape) ||
	     xgeShapeExSceneAdd(scene, shape) != XGE_OK ||
	     !dump_bounds("base_bounds", scene) || !dump_obb("base_obb", scene) ) return 1;
	if ( xgeShapeExSceneEffectGaussianBlur(scene, 2.0f, XGE_SHAPE_EX_BLUR_BOTH,
	     XGE_SHAPE_EX_BORDER_DUPLICATE, 80) != XGE_OK ||
	     !dump_bounds("blur_bounds", scene) || !dump_obb("blur_obb", scene) ) return 2;
	if ( xgeShapeExSceneEffectClear(scene) != XGE_OK ||
	     xgeShapeExSceneEffectDropShadow(scene, XGE_COLOR_RGBA(20, 30, 40, 200),
	     135.0f, 10.0f, 2.0f, 80) != XGE_OK ||
	     !dump_bounds("shadow_bounds", scene) || !dump_obb("shadow_obb", scene) ) return 3;
	if ( xgeShapeExSceneTransformSet(scene, &transform) != XGE_OK ||
	     !dump_bounds("transformed_shadow_bounds", scene) ||
	     !dump_obb("transformed_shadow_obb", scene) ) return 4;

	if ( xgeShapeExSceneCreate(&root) != XGE_OK || xgeShapeExSceneCreate(&child) != XGE_OK ||
	     !make_rect(&childShape) || xgeShapeExSceneAdd(child, childShape) != XGE_OK ||
	     xgeShapeExSceneEffectGaussianBlur(child, 2.0f, XGE_SHAPE_EX_BLUR_BOTH,
	     XGE_SHAPE_EX_BORDER_DUPLICATE, 80) != XGE_OK ||
	     xgeShapeExSceneTransformSet(child, &transform) != XGE_OK ||
	     xgeShapeExSceneAddScene(root, child) != XGE_OK ||
	     !dump_bounds("nested_effect_bounds", root) ||
	     !dump_obb("nested_effect_obb", root) ) return 5;

	if ( !make_rect(&decoratedShape) || xgeShapeExCreate(&shapeClip) != XGE_OK ||
	     xgeShapeExAppendRect(shapeClip, 15.0f, 25.0f, 10.0f, 12.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExClipShapeAdd(decoratedShape, shapeClip) != XGE_OK ||
	     !dump_shape_bounds("shape_clip_bounds", decoratedShape) ||
	     !dump_shape_obb("shape_clip_obb", decoratedShape) ||
	     xgeShapeExClipClear(decoratedShape) != XGE_OK ||
	     xgeShapeExCreate(&shapeMask) != XGE_OK ||
	     xgeShapeExAppendRect(shapeMask, 16.0f, 26.0f, 11.0f, 13.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExFillColor(shapeMask, XGE_COLOR_RGBA(255, 255, 255, 255)) != XGE_OK ||
	     xgeShapeExMaskShapeSet(decoratedShape, shapeMask, XGE_SHAPE_EX_MASK_ALPHA) != XGE_OK ||
	     !dump_shape_bounds("shape_mask_bounds", decoratedShape) ||
	     !dump_shape_obb("shape_mask_obb", decoratedShape) ) return 6;

	if ( xgeShapeExSceneCreate(&clippedScene) != XGE_OK || !make_rect(&clippedSceneShape) ||
	     xgeShapeExSceneAdd(clippedScene, clippedSceneShape) != XGE_OK ||
	     xgeShapeExCreate(&sceneClip) != XGE_OK ||
	     xgeShapeExAppendRect(sceneClip, 15.0f, 25.0f, 10.0f, 12.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExSceneClipShapeAdd(clippedScene, sceneClip) != XGE_OK ||
	     xgeShapeExSceneEffectGaussianBlur(clippedScene, 2.0f, XGE_SHAPE_EX_BLUR_BOTH,
	     XGE_SHAPE_EX_BORDER_DUPLICATE, 80) != XGE_OK ||
	     !dump_bounds("scene_clip_bounds", clippedScene) ||
	     !dump_obb("scene_clip_obb", clippedScene) ) return 7;

	if ( xgeShapeExSceneCreate(&maskedScene) != XGE_OK || !make_rect(&maskedSceneShape) ||
	     xgeShapeExSceneAdd(maskedScene, maskedSceneShape) != XGE_OK ||
	     xgeShapeExCreate(&sceneMask) != XGE_OK ||
	     xgeShapeExAppendRect(sceneMask, 16.0f, 26.0f, 11.0f, 13.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExFillColor(sceneMask, XGE_COLOR_RGBA(255, 255, 255, 255)) != XGE_OK ||
	     xgeShapeExSceneMaskShapeSet(maskedScene, sceneMask, XGE_SHAPE_EX_MASK_ALPHA) != XGE_OK ||
	     xgeShapeExSceneEffectDropShadow(maskedScene, XGE_COLOR_RGBA(20, 30, 40, 200),
	     135.0f, 10.0f, 2.0f, 80) != XGE_OK ||
	     !dump_bounds("scene_mask_bounds", maskedScene) ||
	     !dump_obb("scene_mask_obb", maskedScene) ) return 8;

	if ( xgeShapeExCreate(&emptyShape) != XGE_OK ||
	     !dump_shape_bounds_status("empty_shape_bounds", emptyShape) ||
	     !dump_shape_obb_status("empty_shape_obb", emptyShape) ) return 9;

	if ( xgeShapeExCreate(&moveShape) != XGE_OK ||
	     xgeShapeExMoveTo(moveShape, 7.0f, 9.0f) != XGE_OK ||
	     !dump_shape_bounds_status("move_shape_bounds", moveShape) ||
	     !dump_shape_obb_status("move_shape_obb", moveShape) ) return 10;

	if ( xgeShapeExCreate(&openLine) != XGE_OK ||
	     xgeShapeExMoveTo(openLine, 10.0f, 20.0f) != XGE_OK ||
	     xgeShapeExLineTo(openLine, 40.0f, 60.0f) != XGE_OK ||
	     !dump_shape_bounds_status("open_line_bounds", openLine) ||
	     !dump_shape_obb_status("open_line_obb", openLine) ) return 11;

	if ( xgeShapeExCreate(&strokedLine) != XGE_OK ||
	     xgeShapeExMoveTo(strokedLine, 10.0f, 20.0f) != XGE_OK ||
	     xgeShapeExLineTo(strokedLine, 40.0f, 60.0f) != XGE_OK ||
	     xgeShapeExStrokeColor(strokedLine, XGE_COLOR_RGBA(230, 80, 120, 255)) != XGE_OK ||
	     xgeShapeExStrokeWidth(strokedLine, 6.0f) != XGE_OK ||
	     !dump_shape_bounds_status("stroked_line_bounds", strokedLine) ||
	     !dump_shape_obb_status("stroked_line_obb", strokedLine) ) return 12;

	if ( xgeShapeExCreate(&buttLine) != XGE_OK ||
	     xgeShapeExMoveTo(buttLine, 10.0f, 20.0f) != XGE_OK ||
	     xgeShapeExLineTo(buttLine, 40.0f, 60.0f) != XGE_OK ||
	     xgeShapeExStrokeColor(buttLine, XGE_COLOR_RGBA(230, 80, 120, 255)) != XGE_OK ||
	     xgeShapeExStrokeWidth(buttLine, 6.0f) != XGE_OK ||
	     xgeShapeExStrokeCap(buttLine, XGE_SHAPE_EX_CAP_BUTT) != XGE_OK ||
	     !dump_shape_bounds_status("butt_line_bounds", buttLine) ||
	     !dump_shape_obb_status("butt_line_obb", buttLine) ) return 22;

	if ( xgeShapeExCreate(&roundLine) != XGE_OK ||
	     xgeShapeExMoveTo(roundLine, 10.0f, 20.0f) != XGE_OK ||
	     xgeShapeExLineTo(roundLine, 40.0f, 60.0f) != XGE_OK ||
	     xgeShapeExStrokeColor(roundLine, XGE_COLOR_RGBA(230, 80, 120, 255)) != XGE_OK ||
	     xgeShapeExStrokeWidth(roundLine, 6.0f) != XGE_OK ||
	     xgeShapeExStrokeCap(roundLine, XGE_SHAPE_EX_CAP_ROUND) != XGE_OK ||
	     !dump_shape_bounds_status("round_line_bounds", roundLine) ||
	     !dump_shape_obb_status("round_line_obb", roundLine) ) return 23;

	if ( xgeShapeExCreate(&miterPath) != XGE_OK ||
	     xgeShapeExMoveTo(miterPath, 10.0f, 30.0f) != XGE_OK ||
	     xgeShapeExLineTo(miterPath, 25.0f, 10.0f) != XGE_OK ||
	     xgeShapeExLineTo(miterPath, 40.0f, 30.0f) != XGE_OK ||
	     xgeShapeExStrokeColor(miterPath, XGE_COLOR_RGBA(230, 80, 120, 255)) != XGE_OK ||
	     xgeShapeExStrokeWidth(miterPath, 6.0f) != XGE_OK ||
	     xgeShapeExStrokeCap(miterPath, XGE_SHAPE_EX_CAP_BUTT) != XGE_OK ||
	     xgeShapeExStrokeJoin(miterPath, XGE_SHAPE_EX_JOIN_MITER) != XGE_OK ||
	     xgeShapeExStrokeMiterLimit(miterPath, 8.0f) != XGE_OK ||
	     !dump_shape_bounds_status("miter_path_bounds", miterPath) ||
	     !dump_shape_obb_status("miter_path_obb", miterPath) ) return 24;

	if ( xgeShapeExCreate(&transparentRect) != XGE_OK ||
	     xgeShapeExAppendRect(transparentRect, 10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExFillColor(transparentRect, XGE_COLOR_RGBA(74, 177, 230, 0)) != XGE_OK ||
	     !dump_shape_bounds_status("transparent_rect_bounds", transparentRect) ||
	     !dump_shape_obb_status("transparent_rect_obb", transparentRect) ) return 13;

	if ( !make_rect(&hiddenShape) || xgeShapeExVisible(hiddenShape, 0) != XGE_OK ||
	     !dump_shape_bounds_status("hidden_shape_bounds", hiddenShape) ||
	     !dump_shape_obb_status("hidden_shape_obb", hiddenShape) ) return 14;

	if ( !make_rect(&zeroOpacityShape) || xgeShapeExOpacity(zeroOpacityShape, 0.0f) != XGE_OK ||
	     !dump_shape_bounds_status("zero_opacity_shape_bounds", zeroOpacityShape) ||
	     !dump_shape_obb_status("zero_opacity_shape_obb", zeroOpacityShape) ) return 15;

	if ( xgeShapeExSceneCreate(&emptyScene) != XGE_OK ||
	     !dump_scene_bounds_status("empty_scene_bounds", emptyScene) ||
	     !dump_scene_obb_status("empty_scene_obb", emptyScene) ) return 16;

	if ( xgeShapeExSceneCreate(&hiddenChildScene) != XGE_OK || !make_rect(&hiddenChild) ||
	     xgeShapeExVisible(hiddenChild, 0) != XGE_OK ||
	     xgeShapeExSceneAdd(hiddenChildScene, hiddenChild) != XGE_OK ||
	     !dump_scene_bounds_status("hidden_child_scene_bounds", hiddenChildScene) ||
	     !dump_scene_obb_status("hidden_child_scene_obb", hiddenChildScene) ) return 17;

	if ( xgeShapeExSceneCreate(&hiddenScene) != XGE_OK || !make_rect(&hiddenSceneShape) ||
	     xgeShapeExSceneAdd(hiddenScene, hiddenSceneShape) != XGE_OK ||
	     xgeShapeExSceneVisible(hiddenScene, 0) != XGE_OK ||
	     !dump_scene_bounds_status("hidden_scene_bounds", hiddenScene) ||
	     !dump_scene_obb_status("hidden_scene_obb", hiddenScene) ) return 18;

	if ( xgeShapeExSceneCreate(&zeroOpacityScene) != XGE_OK || !make_rect(&zeroOpacitySceneShape) ||
	     xgeShapeExSceneAdd(zeroOpacityScene, zeroOpacitySceneShape) != XGE_OK ||
	     xgeShapeExSceneOpacity(zeroOpacityScene, 0.0f) != XGE_OK ||
	     !dump_scene_bounds_status("zero_opacity_scene_bounds", zeroOpacityScene) ||
	     !dump_scene_obb_status("zero_opacity_scene_obb", zeroOpacityScene) ) return 19;

	if ( !make_rect(&rotatedShape) ||
	     xgeShapeExTransformRotate(rotatedShape, 0.5235987755982988f) != XGE_OK ||
	     !dump_shape_bounds_status("rotated_shape_bounds", rotatedShape) ||
	     !dump_shape_obb_status("rotated_shape_obb", rotatedShape) ) return 20;

	if ( xgeShapeExSceneCreate(&rotatedScene) != XGE_OK || !make_rect(&rotatedSceneShape) ||
	     xgeShapeExSceneAdd(rotatedScene, rotatedSceneShape) != XGE_OK ||
	     xgeShapeExSceneTransformRotate(rotatedScene, 0.5235987755982988f) != XGE_OK ||
	     !dump_scene_bounds_status("rotated_scene_bounds", rotatedScene) ||
	     !dump_scene_obb_status("rotated_scene_obb", rotatedScene) ) return 21;

	xgeShapeExDestroy(shape);
	xgeShapeExDestroy(childShape);
	xgeShapeExDestroy(decoratedShape);
	xgeShapeExDestroy(shapeClip);
	xgeShapeExDestroy(shapeMask);
	xgeShapeExDestroy(clippedSceneShape);
	xgeShapeExDestroy(maskedSceneShape);
	xgeShapeExDestroy(sceneClip);
	xgeShapeExDestroy(sceneMask);
	xgeShapeExDestroy(emptyShape);
	xgeShapeExDestroy(moveShape);
	xgeShapeExDestroy(openLine);
	xgeShapeExDestroy(strokedLine);
	xgeShapeExDestroy(buttLine);
	xgeShapeExDestroy(roundLine);
	xgeShapeExDestroy(miterPath);
	xgeShapeExDestroy(transparentRect);
	xgeShapeExDestroy(hiddenShape);
	xgeShapeExDestroy(zeroOpacityShape);
	xgeShapeExDestroy(hiddenChild);
	xgeShapeExDestroy(hiddenSceneShape);
	xgeShapeExDestroy(zeroOpacitySceneShape);
	xgeShapeExDestroy(rotatedShape);
	xgeShapeExDestroy(rotatedSceneShape);
	xgeShapeExSceneDestroy(scene);
	xgeShapeExSceneDestroy(root);
	xgeShapeExSceneDestroy(child);
	xgeShapeExSceneDestroy(clippedScene);
	xgeShapeExSceneDestroy(maskedScene);
	xgeShapeExSceneDestroy(emptyScene);
	xgeShapeExSceneDestroy(hiddenChildScene);
	xgeShapeExSceneDestroy(hiddenScene);
	xgeShapeExSceneDestroy(zeroOpacityScene);
	xgeShapeExSceneDestroy(rotatedScene);
	return 0;
}
