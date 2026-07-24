#include "xge.h"

#include <stdio.h>

static void print_matrix(const char* name, xge_shape_ex_matrix_t matrix)
{
	printf("%s %.9f %.9f %.9f %.9f %.9f %.9f\n", name,
		matrix.fA, matrix.fB, matrix.fC, matrix.fD, matrix.fE, matrix.fF);
}

static int matrix_equal(xge_shape_ex_matrix_t a, xge_shape_ex_matrix_t b)
{
	return (a.fA == b.fA) && (a.fB == b.fB) &&
	       (a.fC == b.fC) && (a.fD == b.fD) &&
	       (a.fE == b.fE) && (a.fF == b.fF);
}

int main(void)
{
	xge_shape_ex shape = NULL;
	xge_shape_ex clone = NULL;
	xge_shape_ex_scene scene = NULL;
	xge_shape_ex_scene sceneClone = NULL;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex_matrix_t custom;
	const float quarterPi = 0.78539816339744830962f;
	int locked;
	int resetLocked;
	int cloneLocked;
	int sceneLocked;
	int sceneCloneLocked;

	if ( xgeShapeExCreate(&shape) != XGE_OK ) return 1;
	if ( xgeShapeExTransformTranslate(shape, 1.0f, 2.0f) != XGE_OK ) return 2;
	if ( xgeShapeExTransformScale(shape, 2.0f, 2.0f) != XGE_OK ) return 3;
	if ( xgeShapeExTransformRotate(shape, 0.25f) != XGE_OK ) return 4;
	if ( xgeShapeExTransformTranslate(shape, 155.0f, -155.0f) != XGE_OK ) return 5;
	if ( xgeShapeExTransformRotate(shape, quarterPi) != XGE_OK ) return 6;
	if ( xgeShapeExTransformScale(shape, 4.7f, 4.7f) != XGE_OK ) return 7;
	if ( xgeShapeExTransformGet(shape, &matrix) != XGE_OK ) return 8;
	print_matrix("shape_basic", matrix);

	custom.fA = 1.0f;
	custom.fB = 2.0f;
	custom.fC = 3.0f;
	custom.fD = 4.0f;
	custom.fE = 5.0f;
	custom.fF = 6.0f;
	if ( xgeShapeExTransformSet(shape, &custom) != XGE_OK ) return 9;
	locked = (xgeShapeExTransformTranslate(shape, 9.0f, 10.0f) == XGE_ERROR_INVALID_STATE) &&
	         (xgeShapeExTransformScale(shape, 3.0f, 3.0f) == XGE_ERROR_INVALID_STATE) &&
	         (xgeShapeExTransformRotate(shape, quarterPi) == XGE_ERROR_INVALID_STATE);
	if ( xgeShapeExTransformGet(shape, &matrix) != XGE_OK ) return 10;
	printf("shape_custom_locked %d\n", locked);
	printf("shape_custom_unchanged %d\n", matrix_equal(matrix, custom));
	if ( xgeShapeExReset(shape) != XGE_OK ) return 11;
	resetLocked = xgeShapeExTransformTranslate(shape, 9.0f, 10.0f) == XGE_ERROR_INVALID_STATE;
	printf("shape_reset_locked %d\n", resetLocked);
	if ( xgeShapeExClone(shape, &clone) != XGE_OK ) return 12;
	cloneLocked = xgeShapeExTransformRotate(clone, 0.0f) == XGE_ERROR_INVALID_STATE;
	printf("shape_clone_locked %d\n", cloneLocked);

	if ( xgeShapeExSceneCreate(&scene) != XGE_OK ) return 13;
	if ( xgeShapeExSceneTransformTranslate(scene, 1.0f, 2.0f) != XGE_OK ) return 14;
	if ( xgeShapeExSceneTransformScale(scene, 2.0f, 2.0f) != XGE_OK ) return 15;
	if ( xgeShapeExSceneTransformRotate(scene, 0.25f) != XGE_OK ) return 16;
	if ( xgeShapeExSceneTransformTranslate(scene, 155.0f, -155.0f) != XGE_OK ) return 17;
	if ( xgeShapeExSceneTransformRotate(scene, quarterPi) != XGE_OK ) return 18;
	if ( xgeShapeExSceneTransformScale(scene, 4.7f, 4.7f) != XGE_OK ) return 19;
	if ( xgeShapeExSceneTransformGet(scene, &matrix) != XGE_OK ) return 20;
	print_matrix("scene_basic", matrix);
	if ( xgeShapeExSceneTransformSet(scene, &custom) != XGE_OK ) return 21;
	sceneLocked = xgeShapeExSceneTransformTranslate(scene, 9.0f, 10.0f) == XGE_ERROR_INVALID_STATE;
	printf("scene_custom_locked %d\n", sceneLocked);
	if ( xgeShapeExSceneClone(scene, &sceneClone) != XGE_OK ) return 22;
	sceneCloneLocked = xgeShapeExSceneTransformScale(sceneClone, 1.0f, 1.0f) == XGE_ERROR_INVALID_STATE;
	printf("scene_clone_locked %d\n", sceneCloneLocked);

	xgeShapeExSceneDestroy(sceneClone);
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(clone);
	xgeShapeExDestroy(shape);
	return 0;
}
