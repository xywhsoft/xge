#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int opacity_byte(xge_shape_ex_scene scene)
{
	float opacity = 0.0f;

	if ( xgeShapeExSceneOpacityGet(scene, &opacity) != XGE_OK ) return -1;
	return (int)floorf(opacity * 255.0f + 0.5f);
}

static int dump_identity(const char* name, xge_shape_ex_scene scene)
{
	uint32_t id = 0;
	int refCount = 0;

	if ( xgeShapeExSceneIdGet(scene, &id) != XGE_OK ||
	     xgeShapeExSceneRefCountGet(scene, &refCount) != XGE_OK ) return 0;
	printf("%s %u %d\n", name, (unsigned int)id, refCount);
	return 1;
}

static int shape_command_count(xge_shape_ex shape)
{
	const uint8_t* commands = NULL;
	const xge_vec2_t* points = NULL;
	int commandCount = -1;
	int pointCount = -1;

	if ( xgeShapeExGetPath(shape, &commands, &commandCount, &points, &pointCount) != XGE_OK ) return -1;
	return commandCount;
}

static int dump_scene(const char* name, xge_shape_ex_scene scene)
{
	xge_shape_ex_scene_child_t child;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex clip = NULL;
	xge_shape_ex maskShape = NULL;
	xge_shape_ex_scene maskScene = NULL;
	int count = -1;
	int visible = -1;
	int method = -1;
	int targetType = -1;
	int clipCount = -1;
	int i;

	if ( xgeShapeExSceneGetCount(scene, &count) != XGE_OK ||
	     xgeShapeExSceneVisibleGet(scene, &visible) != XGE_OK ||
	     xgeShapeExSceneTransformGet(scene, &matrix) != XGE_OK ||
	     xgeShapeExSceneMaskGet(scene, &method, &targetType, &maskShape, &maskScene) != XGE_OK ||
	     xgeShapeExSceneClipShapeGetCount(scene, &clipCount) != XGE_OK ) return 0;
	if ( clipCount > 0 && xgeShapeExSceneClipShapeGetAt(scene, 0, &clip) != XGE_OK ) return 0;
	printf("%s %d %d %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g",
		name, count, opacity_byte(scene), visible,
		targetType == XGE_SHAPE_EX_MASK_TARGET_NONE,
		clip == NULL,
		matrix.fA, matrix.fB, matrix.fC, matrix.fD, matrix.fE, matrix.fF);
	for ( i = 0; i < count; ++i ) {
		memset(&child, 0, sizeof(child));
		if ( xgeShapeExSceneChildGetAt(scene, i, &child) != XGE_OK ) return 0;
		printf(" %d", child.iType);
	}
	printf("\n");
	return 1;
}

static int make_rect(xge_shape_ex* shape, float x)
{
	return xgeShapeExCreate(shape) == XGE_OK &&
		xgeShapeExAppendRect(*shape, x, 0.0f, 10.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK;
}

int main(void)
{
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex_scene nested = NULL;
	xge_shape_ex_scene mask = NULL;
	xge_shape_ex_scene clone = NULL;
	xge_shape_ex_scene orderScene = NULL;
	xge_shape_ex_scene orderFront = NULL;
	xge_shape_ex_scene orderMiddle = NULL;
	xge_shape_ex_scene orderEnd = NULL;
	xge_shape_ex_scene orderClone = NULL;
	xge_shape_ex s1 = NULL;
	xge_shape_ex s2 = NULL;
	xge_shape_ex inserted = NULL;
	xge_shape_ex nestedShape = NULL;
	xge_shape_ex clip = NULL;
	xge_shape_ex orderShape1 = NULL;
	xge_shape_ex orderShape2 = NULL;
	xge_shape_ex cloneClip = NULL;
	xge_shape_ex cloneMaskShape = NULL;
	xge_shape_ex_scene cloneMask = NULL;
	xge_shape_ex_scene_child_t child;
	xge_shape_ex_scene_child_t cloneFirst;
	xge_shape_ex_scene_child_t cloneNested;
	xge_shape_ex_matrix_t matrix = {1.25f, 0.5f, -0.25f, 0.75f, 12.0f, 34.0f};
	int method = -1;
	int targetType = -1;
	int cloneCount = -1;
	int nestedCloneCount = -1;
	int removeFirst;
	int removeSecond;

	if ( xgeShapeExSceneRefCountGet(NULL, NULL) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExSceneId(NULL, 1u) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExSceneIdGet(NULL, NULL) != XGE_ERROR_INVALID_ARGUMENT ) return 1;
	if ( xgeShapeExSceneCreate(&scene) != XGE_OK || !dump_scene("default_scene", scene) ||
	     !dump_identity("identity_default", scene) ) return 1;
	if ( xgeShapeExSceneRefCountGet(scene, NULL) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExSceneIdGet(scene, NULL) != XGE_ERROR_INVALID_ARGUMENT ) return 1;
	if ( xgeShapeExSceneId(scene, 0x2468ace0u) != XGE_OK || xgeShapeExSceneAddRef(scene) != XGE_OK ||
	     !dump_identity("identity_configured", scene) ) return 2;
	xgeShapeExSceneDestroy(scene);
	if ( !dump_identity("identity_released", scene) ) return 2;
	if ( xgeShapeExSceneCreate(&nested) != XGE_OK || xgeShapeExSceneCreate(&mask) != XGE_OK ||
	     !make_rect(&s1, 0.0f) || !make_rect(&s2, 40.0f) || !make_rect(&inserted, 20.0f) ||
	     !make_rect(&nestedShape, 60.0f) || !make_rect(&clip, 0.0f) ) return 3;
	if ( xgeShapeExSceneAdd(nested, nestedShape) != XGE_OK ||
	     xgeShapeExSceneAdd(scene, s1) != XGE_OK ||
	     xgeShapeExSceneAddScene(scene, nested) != XGE_OK ||
	     xgeShapeExSceneAdd(scene, s2) != XGE_OK ||
	     xgeShapeExSceneInsert(scene, inserted, s2) != XGE_OK ||
	     xgeShapeExSceneClipShapeAdd(scene, clip) != XGE_OK ||
	     xgeShapeExSceneMaskSceneSet(scene, mask, XGE_SHAPE_EX_MASK_ALPHA) != XGE_OK ||
	     xgeShapeExSceneOpacity(scene, 64.0f / 255.0f) != XGE_OK ||
	     xgeShapeExSceneVisible(scene, 0) != XGE_OK ||
	     xgeShapeExSceneTransformSet(scene, &matrix) != XGE_OK ||
	     !dump_scene("configured_scene", scene) ) return 4;
	if ( xgeShapeExSceneClone(scene, &clone) != XGE_OK || !dump_scene("clone_scene", clone) ||
	     !dump_identity("identity_clone", clone) ) return 5;
	if ( xgeShapeExSceneGetCount(clone, &cloneCount) != XGE_OK ||
	     xgeShapeExSceneGetCount(nested, &nestedCloneCount) != XGE_OK ||
	     xgeShapeExSceneChildGetAt(scene, 0, &child) != XGE_OK ||
	     xgeShapeExSceneChildGetAt(clone, 0, &cloneFirst) != XGE_OK ||
	     xgeShapeExSceneChildGetAt(clone, 1, &cloneNested) != XGE_OK ||
	     xgeShapeExSceneGetCount(cloneNested.pScene, &nestedCloneCount) != XGE_OK ||
	     xgeShapeExSceneClipShapeGetAt(clone, 0, &cloneClip) != XGE_OK ||
	     xgeShapeExSceneMaskGet(clone, &method, &targetType, &cloneMaskShape, &cloneMask) != XGE_OK ) return 6;
	printf("clone_independent %d %d %d %d %d %d\n",
		cloneCount,
		child.pShape != NULL && cloneNested.pScene != NULL,
		child.pShape != cloneFirst.pShape,
		nestedCloneCount,
		cloneClip != clip,
		cloneMask != mask);
	if ( xgeShapeExReset(s1) != XGE_OK ||
	     xgeShapeExAppendRect(s1, 0.0f, 0.0f, 30.0f, 30.0f, 4.0f, 4.0f, 1) != XGE_OK ||
	     xgeShapeExSceneChildGetAt(clone, 0, &cloneFirst) != XGE_OK ) return 7;
	printf("clone_after_source_mutate %d\n", shape_command_count(cloneFirst.pShape));
	removeFirst = xgeShapeExSceneRemove(scene, inserted) == XGE_OK;
	removeSecond = xgeShapeExSceneRemove(scene, inserted) != XGE_OK;
	printf("remove_semantics %d %d\n", removeFirst, removeSecond);
	if ( !dump_scene("after_remove", scene) ) return 8;
	if ( xgeShapeExSceneCreate(&orderScene) != XGE_OK ||
	     xgeShapeExSceneCreate(&orderFront) != XGE_OK ||
	     xgeShapeExSceneCreate(&orderMiddle) != XGE_OK ||
	     xgeShapeExSceneCreate(&orderEnd) != XGE_OK ||
	     !make_rect(&orderShape1, 80.0f) || !make_rect(&orderShape2, 100.0f) ||
	     xgeShapeExSceneAdd(orderScene, orderShape1) != XGE_OK ||
	     xgeShapeExSceneAdd(orderScene, orderShape2) != XGE_OK ||
	     xgeShapeExSceneInsertSceneAt(orderScene, orderMiddle, 1) != XGE_OK ||
	     xgeShapeExSceneInsertSceneAt(orderScene, orderFront, 0) != XGE_OK ||
	     xgeShapeExSceneInsertSceneAt(orderScene, orderEnd, 4) != XGE_OK ||
	     !dump_scene("nested_insert_order", orderScene) ) return 9;
	if ( xgeShapeExSceneRemoveScene(orderScene, orderMiddle) != XGE_OK ||
	     !dump_scene("nested_after_remove", orderScene) ||
	     xgeShapeExSceneInsertSceneAt(orderScene, orderMiddle, 2) != XGE_OK ||
	     !dump_scene("nested_after_reinsert", orderScene) ||
	     xgeShapeExSceneClone(orderScene, &orderClone) != XGE_OK ||
	     !dump_scene("nested_order_clone", orderClone) ) return 10;
	if ( xgeShapeExSceneClear(scene) != XGE_OK || !dump_scene("after_clear", scene) ) return 9;

	xgeShapeExSceneDestroy(orderClone);
	xgeShapeExSceneDestroy(orderScene);
	xgeShapeExSceneDestroy(orderFront);
	xgeShapeExSceneDestroy(orderMiddle);
	xgeShapeExSceneDestroy(orderEnd);
	xgeShapeExSceneDestroy(clone);
	xgeShapeExSceneDestroy(scene);
	xgeShapeExSceneDestroy(nested);
	xgeShapeExSceneDestroy(mask);
	xgeShapeExDestroy(s1);
	xgeShapeExDestroy(s2);
	xgeShapeExDestroy(inserted);
	xgeShapeExDestroy(nestedShape);
	xgeShapeExDestroy(clip);
	xgeShapeExDestroy(orderShape1);
	xgeShapeExDestroy(orderShape2);
	return 0;
}
