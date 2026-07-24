#include "xge.h"

#include <math.h>
#include <stdio.h>

static int opacity_byte(xge_shape_ex shape)
{
	float opacity = 0.0f;

	if ( xgeShapeExOpacityGet(shape, &opacity) != XGE_OK ) return -1;
	return (int)floorf(opacity * 255.0f + 0.5f);
}

static int dump_identity(const char* name, xge_shape_ex shape)
{
	uint32_t id = 0;
	int refCount = 0;

	if ( xgeShapeExIdGet(shape, &id) != XGE_OK ||
	     xgeShapeExRefCountGet(shape, &refCount) != XGE_OK ) return 0;
	printf("%s %u %d\n", name, (unsigned int)id, refCount);
	return 1;
}

static int dump_shape(const char* name, xge_shape_ex shape)
{
	const uint8_t* commands = NULL;
	const xge_vec2_t* points = NULL;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex clip = NULL;
	xge_shape_ex maskShape = NULL;
	xge_shape_ex_scene maskScene = NULL;
	uint32_t strokeColor = 0;
	int commandCount = -1;
	int pointCount = -1;
	int fillRule = -1;
	int strokeFirst = -1;
	int visible = -1;
	int clipCount = -1;
	int maskMethod = -1;
	int maskTargetType = -1;
	int strokePresent;

	if ( xgeShapeExGetPath(shape, &commands, &commandCount, &points, &pointCount) != XGE_OK ||
	     xgeShapeExFillRuleGet(shape, &fillRule) != XGE_OK ||
	     xgeShapeExPaintOrderGet(shape, &strokeFirst) != XGE_OK ||
	     xgeShapeExVisibleGet(shape, &visible) != XGE_OK ||
	     xgeShapeExTransformGet(shape, &matrix) != XGE_OK ||
	     xgeShapeExClipShapeGetCount(shape, &clipCount) != XGE_OK ||
	     xgeShapeExMaskGet(shape, &maskMethod, &maskTargetType, &maskShape, &maskScene) != XGE_OK ) return 0;
	if ( clipCount > 0 && xgeShapeExClipShapeGetAt(shape, 0, &clip) != XGE_OK ) return 0;
	strokePresent = xgeShapeExStrokeColorGet(shape, &strokeColor) == XGE_OK;
	printf("%s %d %d %d %d %d %d %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g\n",
		name, commandCount, pointCount, fillRule, strokeFirst, opacity_byte(shape), visible,
		clip != NULL, maskMethod, maskTargetType != XGE_SHAPE_EX_MASK_TARGET_NONE,
		matrix.fA, matrix.fB, matrix.fC, matrix.fD, matrix.fE, matrix.fF);
	printf("%s_stroke %d\n", name, strokePresent);
	return 1;
}

int main(void)
{
	xge_shape_ex shape = NULL;
	xge_shape_ex clip = NULL;
	xge_shape_ex mask = NULL;
	xge_shape_ex clone = NULL;
	xge_shape_ex cloneClip = NULL;
	xge_shape_ex cloneMask = NULL;
	xge_shape_ex_scene cloneMaskScene = NULL;
	xge_shape_ex_matrix_t matrix = {1.25f, 0.5f, -0.25f, 0.75f, 12.0f, 34.0f};
	int cloneMaskMethod = -1;
	int cloneMaskTargetType = -1;

	printf("identity_hash %u %u %u\n",
		(unsigned int)xgeShapeExIdFromName(NULL),
		(unsigned int)xgeShapeExIdFromName(""),
		(unsigned int)xgeShapeExIdFromName("shape-node"));
	if ( xgeShapeExRefCountGet(NULL, NULL) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExId(NULL, 1u) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExIdGet(NULL, NULL) != XGE_ERROR_INVALID_ARGUMENT ) return 1;
	if ( xgeShapeExCreate(&shape) != XGE_OK || !dump_shape("default_shape", shape) ||
	     !dump_identity("identity_default", shape) ) return 1;
	if ( xgeShapeExRefCountGet(shape, NULL) != XGE_ERROR_INVALID_ARGUMENT ||
	     xgeShapeExIdGet(shape, NULL) != XGE_ERROR_INVALID_ARGUMENT ) return 1;
	if ( xgeShapeExId(shape, 0x12345678u) != XGE_OK || xgeShapeExAddRef(shape) != XGE_OK ||
	     !dump_identity("identity_configured", shape) ) return 2;
	xgeShapeExDestroy(shape);
	if ( !dump_identity("identity_released", shape) ) return 2;
	if ( xgeShapeExCreate(&clip) != XGE_OK || xgeShapeExCreate(&mask) != XGE_OK ||
	     xgeShapeExAppendRect(shape, 1.0f, 2.0f, 30.0f, 40.0f, 3.0f, 4.0f, 1) != XGE_OK ||
	     xgeShapeExAppendRect(clip, 2.0f, 3.0f, 20.0f, 25.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExAppendRect(mask, 4.0f, 5.0f, 18.0f, 21.0f, 0.0f, 0.0f, 1) != XGE_OK ||
	     xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD) != XGE_OK ||
	     xgeShapeExPaintOrder(shape, 1) != XGE_OK ||
	     xgeShapeExOpacity(shape, 64.0f / 255.0f) != XGE_OK ||
	     xgeShapeExVisible(shape, 0) != XGE_OK ||
	     xgeShapeExTransformSet(shape, &matrix) != XGE_OK ||
	     xgeShapeExClipShapeAdd(shape, clip) != XGE_OK ||
	     xgeShapeExMaskShapeSet(shape, mask, XGE_SHAPE_EX_MASK_LUMA) != XGE_OK ||
	     !dump_shape("configured_shape", shape) ) return 3;
	if ( xgeShapeExClone(shape, &clone) != XGE_OK || !dump_shape("clone_shape", clone) ||
	     !dump_identity("identity_clone", clone) ) return 4;
	if ( xgeShapeExClipShapeGetAt(clone, 0, &cloneClip) != XGE_OK ||
	     xgeShapeExMaskGet(clone, &cloneMaskMethod, &cloneMaskTargetType, &cloneMask, &cloneMaskScene) != XGE_OK ) return 5;
	printf("clone_deep %d %d\n", cloneClip != clip,
		cloneMaskTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE && cloneMask != NULL && cloneMask != mask);
	if ( xgeShapeExReset(shape) != XGE_OK ||
	     xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_NON_ZERO) != XGE_OK ||
	     xgeShapeExPaintOrder(shape, 0) != XGE_OK ||
	     xgeShapeExOpacity(shape, 200.0f / 255.0f) != XGE_OK ||
	     xgeShapeExVisible(shape, 1) != XGE_OK ||
	     xgeShapeExTransformIdentity(shape) != XGE_OK ||
	     xgeShapeExClipClear(shape) != XGE_OK ||
	     xgeShapeExMaskClear(shape) != XGE_OK ||
	     !dump_shape("source_mutated", shape) ||
	     !dump_shape("clone_after_source_mutate", clone) ) return 6;
	if ( xgeShapeExReset(clone) != XGE_OK || !dump_shape("reset_path_only", clone) ) return 7;
	if ( xgeShapeExClipClear(clone) != XGE_OK || xgeShapeExMaskClear(clone) != XGE_OK ||
	     !dump_shape("clear_decorations", clone) ) return 8;

	xgeShapeExDestroy(clone);
	xgeShapeExDestroy(shape);
	xgeShapeExDestroy(clip);
	xgeShapeExDestroy(mask);
	return 0;
}
