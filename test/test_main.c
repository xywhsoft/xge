#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check(int condition, const char* message)
{
	if ( !condition ) {
		printf("xge smoke failed: %s\n", message);
		return 0;
	}
	return 1;
}

typedef struct test_svg_resource_provider_state_t {
	const char* sPath;
	int iLoadCount;
	int iFreeCount;
} test_svg_resource_provider_state_t;

static int test_svg_resource_provider_load(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	test_svg_resource_provider_state_t* pState = (test_svg_resource_provider_state_t*)pUser;
	FILE* pFile;
	void* pData;
	long iSize;

	if ( (sURI == NULL) || (ppData == NULL) || (pSize == NULL) || (pState == NULL) ||
	     (pState->sPath == NULL) || (strcmp(sURI, "svgprobe://asset.png") != 0) ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	*ppData = NULL;
	*pSize = 0;
	pFile = fopen(pState->sPath, "rb");
	if ( pFile == NULL ) return XGE_ERROR_FILE_NOT_FOUND;
	if ( (fseek(pFile, 0, SEEK_END) != 0) || ((iSize = ftell(pFile)) <= 0) ||
	     (iSize > INT32_MAX) || (fseek(pFile, 0, SEEK_SET) != 0) ) {
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pData = malloc((size_t)iSize);
	if ( pData == NULL ) {
		fclose(pFile);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(pData, 1, (size_t)iSize, pFile) != (size_t)iSize ) {
		free(pData);
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	fclose(pFile);
	pState->iLoadCount++;
	*ppData = pData;
	*pSize = (int)iSize;
	return XGE_OK;
}

static void test_svg_resource_provider_free(void* pData, void* pUser)
{
	test_svg_resource_provider_state_t* pState = (test_svg_resource_provider_state_t*)pUser;

	free(pData);
	if ( pState != NULL ) pState->iFreeCount++;
}

static int test_shape_ex_external_draw(void* pUser, const xge_shape_ex_matrix_t* pParentMatrix)
{
	(void)pUser;
	return pParentMatrix != NULL ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}

typedef struct test_shape_ex_scene_traverse_state_t {
	int iCount;
	int iStopAfter;
	int iTypes[8];
	uint32_t iIds[8];
	xge_shape_ex_scene pReleaseRoot;
} test_shape_ex_scene_traverse_state_t;

static int test_shape_ex_scene_traverse_visit(
	const xge_shape_ex_scene_child_t* pPaint,
	void* pUser
)
{
	test_shape_ex_scene_traverse_state_t* pState =
		(test_shape_ex_scene_traverse_state_t*)pUser;
	uint32_t iId = 0;

	if ( (pPaint == NULL) || (pState == NULL) ||
	     (pState->iCount >= (int)(sizeof(pState->iTypes) / sizeof(pState->iTypes[0]))) ) {
		return 0;
	}
	if ( pPaint->iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE ) {
		if ( xgeShapeExSceneIdGet(pPaint->pScene, &iId) != XGE_OK ) return 0;
	} else if ( pPaint->iType == XGE_SHAPE_EX_SCENE_CHILD_SHAPE ) {
		if ( xgeShapeExIdGet(pPaint->pShape, &iId) != XGE_OK ) return 0;
	} else {
		return 0;
	}
	pState->iTypes[pState->iCount] = pPaint->iType;
	pState->iIds[pState->iCount] = iId;
	pState->iCount++;
	if ( pState->pReleaseRoot != NULL ) {
		xgeShapeExSceneDestroy(pState->pReleaseRoot);
		pState->pReleaseRoot = NULL;
	}
	return (pState->iStopAfter <= 0) || (pState->iCount < pState->iStopAfter);
}

static int test_shape_ex_scene_traverse(void)
{
	xge_shape_ex_scene root = NULL;
	xge_shape_ex_scene nested = NULL;
	xge_shape_ex_scene deep = NULL;
	xge_shape_ex shapes[4] = {NULL, NULL, NULL, NULL};
	test_shape_ex_scene_traverse_state_t state;
	const int expectedTypes[7] = {
		XGE_SHAPE_EX_SCENE_CHILD_SCENE,
		XGE_SHAPE_EX_SCENE_CHILD_SHAPE,
		XGE_SHAPE_EX_SCENE_CHILD_SCENE,
		XGE_SHAPE_EX_SCENE_CHILD_SHAPE,
		XGE_SHAPE_EX_SCENE_CHILD_SCENE,
		XGE_SHAPE_EX_SCENE_CHILD_SHAPE,
		XGE_SHAPE_EX_SCENE_CHILD_SHAPE
	};
	const uint32_t expectedIds[7] = {100u, 101u, 102u, 103u, 104u, 105u, 106u};
	int ok = 1;
	int i;

	memset(&state, 0, sizeof(state));
	ok = ok && check(xgeShapeExSceneTraverse(NULL, test_shape_ex_scene_traverse_visit, &state) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene traverse rejects null scene");
	ok = ok && check(xgeShapeExSceneCreate(&root) == XGE_OK, "ShapeEx scene traverse root create");
	ok = ok && check(xgeShapeExSceneCreate(&nested) == XGE_OK, "ShapeEx scene traverse nested create");
	ok = ok && check(xgeShapeExSceneCreate(&deep) == XGE_OK, "ShapeEx scene traverse deep create");
	for ( i = 0; ok && (i < 4); i++ ) {
		ok = check(xgeShapeExCreate(&shapes[i]) == XGE_OK, "ShapeEx scene traverse shape create");
	}
	if ( ok ) {
		ok = check(xgeShapeExSceneTraverse(root, NULL, &state) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene traverse rejects null callback") &&
		     check(xgeShapeExSceneId(root, 100u) == XGE_OK, "ShapeEx scene traverse root id") &&
		     check(xgeShapeExId(shapes[0], 101u) == XGE_OK, "ShapeEx scene traverse shape A id") &&
		     check(xgeShapeExSceneId(nested, 102u) == XGE_OK, "ShapeEx scene traverse nested id") &&
		     check(xgeShapeExId(shapes[1], 103u) == XGE_OK, "ShapeEx scene traverse shape B id") &&
		     check(xgeShapeExSceneId(deep, 104u) == XGE_OK, "ShapeEx scene traverse deep id") &&
		     check(xgeShapeExId(shapes[2], 105u) == XGE_OK, "ShapeEx scene traverse shape C id") &&
		     check(xgeShapeExId(shapes[3], 106u) == XGE_OK, "ShapeEx scene traverse shape D id") &&
		     check(xgeShapeExSceneAdd(root, shapes[0]) == XGE_OK, "ShapeEx scene traverse add A") &&
		     check(xgeShapeExSceneAdd(nested, shapes[1]) == XGE_OK, "ShapeEx scene traverse add B") &&
		     check(xgeShapeExSceneAdd(deep, shapes[2]) == XGE_OK, "ShapeEx scene traverse add C") &&
		     check(xgeShapeExSceneAddScene(nested, deep) == XGE_OK, "ShapeEx scene traverse add deep") &&
		     check(xgeShapeExSceneAddScene(root, nested) == XGE_OK, "ShapeEx scene traverse add nested") &&
		     check(xgeShapeExSceneAdd(root, shapes[3]) == XGE_OK, "ShapeEx scene traverse add D");
	}
	if ( ok ) {
		memset(&state, 0, sizeof(state));
		ok = check(xgeShapeExSceneTraverse(root, test_shape_ex_scene_traverse_visit, &state) == XGE_OK, "ShapeEx scene traverse full") &&
		     check(state.iCount == 7, "ShapeEx scene traverse full count");
		for ( i = 0; ok && (i < 7); i++ ) {
			ok = check((state.iTypes[i] == expectedTypes[i]) && (state.iIds[i] == expectedIds[i]), "ShapeEx scene traverse pre-order");
		}
	}
	if ( ok ) {
		memset(&state, 0, sizeof(state));
		state.iStopAfter = 4;
		ok = check(xgeShapeExSceneTraverse(root, test_shape_ex_scene_traverse_visit, &state) == XGE_OK, "ShapeEx scene traverse early stop") &&
		     check(state.iCount == 4, "ShapeEx scene traverse early stop count");
		for ( i = 0; ok && (i < 4); i++ ) {
			ok = check((state.iTypes[i] == expectedTypes[i]) && (state.iIds[i] == expectedIds[i]), "ShapeEx scene traverse early stop order");
		}
	}
	if ( ok ) {
		memset(&state, 0, sizeof(state));
		state.pReleaseRoot = root;
		ok = check(xgeShapeExSceneTraverse(root, test_shape_ex_scene_traverse_visit, &state) == XGE_OK, "ShapeEx scene traverse retains root") &&
		     check((state.iCount == 7) && (state.pReleaseRoot == NULL), "ShapeEx scene traverse retained root result");
		root = NULL;
	}
	xgeShapeExSceneDestroy(root);
	xgeShapeExSceneDestroy(nested);
	xgeShapeExSceneDestroy(deep);
	for ( i = 0; i < 4; i++ ) xgeShapeExDestroy(shapes[i]);
	return ok;
}

static int test_shape_ex(void)
{
	xge_shape_ex shape;
	xge_shape_ex_scene scene;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex_color_stop_t stops[2];
	xge_shape_ex_color_stop_t badStops[2];
	xge_shape_ex_color_stop_t unorderedStops[4];
	xge_rect_t bounds;
	float dash[2] = {4.0f, 2.0f};
	float dashBad[2] = {4.0f, 2.0f};
	float dashNegative[3] = {4.0f, -2.0f, 1.0f};
	float dashOdd[3] = {2.0f, 2.0f, 4.0f};
	float length;
	volatile float invalidZero = 0.0f;
	float invalidNaN;
	float invalidInf;
	int ret;

	invalidNaN = 0.0f / invalidZero;
	invalidInf = 1.0f / invalidZero;
	shape = NULL;
	ret = xgeShapeExCreate(&shape);
	if ( !check((ret == XGE_OK) && (shape != NULL), "ShapeEx create") ) return 0;
	{
		xge_shape_ex moveOnly = NULL;
		xge_vec2_t emptyObb[4] = {
			{1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}
		};

		bounds = (xge_rect_t){1.0f, 1.0f, 1.0f, 1.0f};
		if ( !check(xgeShapeExGetBounds(shape, 0.05f, &bounds) == XGE_ERROR_INVALID_STATE, "ShapeEx empty bounds state") ||
		     !check((bounds.fX == 0.0f) && (bounds.fY == 0.0f) &&
		            (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "ShapeEx empty bounds cleared") ||
		     !check(xgeShapeExGetOBB(shape, 0.05f, emptyObb) == XGE_ERROR_INVALID_STATE, "ShapeEx empty obb state") ||
		     !check((emptyObb[0].fX == 0.0f) && (emptyObb[0].fY == 0.0f) &&
		            (emptyObb[3].fX == 0.0f) && (emptyObb[3].fY == 0.0f), "ShapeEx empty obb cleared") ||
		     !check(xgeShapeExCreate(&moveOnly) == XGE_OK, "ShapeEx move-only create") ||
		     !check(xgeShapeExMoveTo(moveOnly, 7.0f, 9.0f) == XGE_OK, "ShapeEx move-only path") ||
		     !check(xgeShapeExGetBounds(moveOnly, 0.05f, &bounds) == XGE_OK, "ShapeEx move-only bounds state") ||
		     !check((bounds.fX == 7.0f) && (bounds.fY == 9.0f) &&
		            (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "ShapeEx move-only degenerate bounds") ) {
			xgeShapeExDestroy(moveOnly);
			xgeShapeExDestroy(shape);
			return 0;
		}
		xgeShapeExDestroy(moveOnly);
	}
	{
		xge_shape_ex stateShape = NULL;
		xge_shape_ex stateClone = NULL;
		xge_shape_ex noStrokeShape = NULL;
		xge_shape_ex_color_stop_t stateStops[2] = {
			{0.0f, XGE_COLOR_RGBA(1, 2, 3, 4)},
			{1.0f, XGE_COLOR_RGBA(5, 6, 7, 8)}
		};
		const float* stateDash = (const float*)(uintptr_t)1;
		uint32_t stateColor = XGE_COLOR_RGBA(201, 202, 203, 204);
		float stateDashOffset = 91.0f;
		float trimBegin = -1.0f;
		float trimEnd = -1.0f;
		int stateDashCount = -1;
		int trimSimultaneous = -1;
		int trimEnabled = -1;

		if ( !check(xgeShapeExCreate(&stateShape) == XGE_OK, "ShapeEx ThorVG paint state create") ||
		     !check(xgeShapeExFillColorGet(stateShape, &stateColor) == XGE_OK, "ShapeEx ThorVG default fill get") ||
		     !check(stateColor == XGE_COLOR_RGBA(0, 0, 0, 0), "ShapeEx ThorVG default fill transparent") ||
		     !check(xgeShapeExStrokeColorGet(stateShape, &stateColor) == XGE_ERROR_INVALID_STATE, "ShapeEx ThorVG default stroke absent") ||
		     !check(xgeShapeExStrokeDashGet(stateShape, &stateDash, &stateDashCount, &stateDashOffset) == XGE_OK, "ShapeEx ThorVG default dash get") ||
		     !check((stateDash == (const float*)(uintptr_t)1) && (stateDashCount == 0) && (stateDashOffset == 91.0f), "ShapeEx ThorVG absent stroke leaves dash outputs") ||
		     !check(xgeShapeExTrimPathGet(stateShape, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled) == XGE_OK, "ShapeEx ThorVG default trim get") ||
		     !check((trimBegin == 0.0f) && (trimEnd == 1.0f) && (trimSimultaneous == 0) && (trimEnabled == 0), "ShapeEx ThorVG default trim state") ) {
			xgeShapeExDestroy(stateShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeWidth(stateShape, -3.0f) == XGE_OK, "ShapeEx ThorVG negative stroke width") ||
		     !check(xgeShapeExStrokeColorGet(stateShape, &stateColor) == XGE_OK, "ShapeEx ThorVG width creates stroke state") ||
		     !check(stateColor == XGE_COLOR_RGBA(0, 0, 0, 0), "ShapeEx ThorVG created stroke transparent") ||
		     !check(xgeShapeExTrimPathGet(stateShape, NULL, NULL, &trimSimultaneous, &trimEnabled) == XGE_OK, "ShapeEx ThorVG created stroke trim get") ||
		     !check((trimSimultaneous == 1) && (trimEnabled == 0), "ShapeEx ThorVG created stroke trim defaults") ||
		     !check(xgeShapeExFillColor(stateShape, XGE_COLOR_RGBA(11, 22, 33, 44)) == XGE_OK, "ShapeEx ThorVG source fill color") ||
		     !check(xgeShapeExFillLinearGradient(stateShape, 0.0f, 0.0f, 10.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stateStops, 2) == XGE_OK, "ShapeEx ThorVG fill gradient state") ||
		     !check(xgeShapeExFillColorGet(stateShape, &stateColor) == XGE_OK, "ShapeEx ThorVG retained fill color get") ||
		     !check(stateColor == XGE_COLOR_RGBA(11, 22, 33, 44), "ShapeEx ThorVG fill gradient preserves solid") ||
		     !check(xgeShapeExStrokeColor(stateShape, XGE_COLOR_RGBA(55, 66, 77, 88)) == XGE_OK, "ShapeEx ThorVG source stroke color") ||
		     !check(xgeShapeExStrokeLinearGradient(stateShape, 0.0f, 0.0f, 10.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_USER_SPACE, stateStops, 2) == XGE_OK, "ShapeEx ThorVG stroke gradient state") ||
		     !check(xgeShapeExStrokeColorGet(stateShape, &stateColor) == XGE_OK, "ShapeEx ThorVG retained stroke color get") ||
		     !check(stateColor == XGE_COLOR_RGBA(55, 66, 77, 0), "ShapeEx ThorVG stroke gradient clears solid alpha") ||
		     !check(xgeShapeExClone(stateShape, &stateClone) == XGE_OK, "ShapeEx ThorVG paint state clone") ||
		     !check(xgeShapeExStrokeColorGet(stateClone, &stateColor) == XGE_OK, "ShapeEx ThorVG cloned stroke state get") ||
		     !check(stateColor == XGE_COLOR_RGBA(55, 66, 77, 0), "ShapeEx ThorVG clone preserves stroke state") ||
		     !check(xgeShapeExReset(stateClone) == XGE_OK, "ShapeEx ThorVG paint state reset path") ||
		     !check(xgeShapeExFillColorGet(stateClone, &stateColor) == XGE_OK, "ShapeEx ThorVG reset retained fill get") ||
		     !check(stateColor == XGE_COLOR_RGBA(11, 22, 33, 44), "ShapeEx ThorVG reset preserves fill source state") ) {
			xgeShapeExDestroy(stateClone);
			xgeShapeExDestroy(stateShape);
			return 0;
		}
		xgeShapeExDestroy(stateClone);
		if ( !check(xgeShapeExTrimPath(stateShape, 0.0f, 1.0f, 0) == XGE_OK, "ShapeEx ThorVG disabled trim state") ||
		     !check(xgeShapeExStrokeWidth(stateShape, 2.0f) == XGE_OK, "ShapeEx ThorVG existing stroke update") ||
		     !check(xgeShapeExTrimPathGet(stateShape, NULL, NULL, &trimSimultaneous, &trimEnabled) == XGE_OK, "ShapeEx ThorVG retained trim mode get") ||
		     !check((trimSimultaneous == 0) && (trimEnabled == 0), "ShapeEx ThorVG existing stroke keeps trim mode") ||
		     !check(xgeShapeExTrimClear(stateShape) == XGE_OK, "ShapeEx ThorVG trim clear state") ||
		     !check(xgeShapeExTrimPathGet(stateShape, NULL, NULL, &trimSimultaneous, &trimEnabled) == XGE_OK, "ShapeEx ThorVG cleared trim get") ||
		     !check((trimSimultaneous == 1) && (trimEnabled == 0), "ShapeEx ThorVG trim clear default mode") ) {
			xgeShapeExDestroy(stateShape);
			return 0;
		}
		xgeShapeExDestroy(stateShape);

		if ( !check(xgeShapeExCreate(&noStrokeShape) == XGE_OK, "ShapeEx ThorVG no-op trim create") ||
		     !check(xgeShapeExTrimPath(noStrokeShape, 0.0f, 1.0f, 1) == XGE_OK, "ShapeEx ThorVG no-op trim") ||
		     !check(xgeShapeExStrokeColorGet(noStrokeShape, &stateColor) == XGE_ERROR_INVALID_STATE, "ShapeEx ThorVG no-op trim keeps stroke absent") ||
		     !check(xgeShapeExPaintOrder(noStrokeShape, 0) == XGE_OK, "ShapeEx ThorVG order creates stroke state") ||
		     !check(xgeShapeExStrokeColorGet(noStrokeShape, &stateColor) == XGE_OK, "ShapeEx ThorVG order stroke state get") ||
		     !check(xgeShapeExTrimPathGet(noStrokeShape, NULL, NULL, &trimSimultaneous, &trimEnabled) == XGE_OK, "ShapeEx ThorVG order trim state get") ||
		     !check((trimSimultaneous == 1) && (trimEnabled == 0), "ShapeEx ThorVG order stroke trim defaults") ) {
			xgeShapeExDestroy(noStrokeShape);
			return 0;
		}
		xgeShapeExDestroy(noStrokeShape);
	}
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
		volatile float zero = 0.0f;
		float badNaN;
		float badInf;

		badNaN = 0.0f / zero;
		badInf = 1.0f / zero;
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
		if ( !check(xgeShapeExMatrixTranslate(&translate, badNaN, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix translate rejects nan") ) return 0;
		if ( !check(xgeShapeExMatrixScale(&scale, badInf, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix scale rejects inf") ) return 0;
		if ( !check(xgeShapeExMatrixRotate(&rotate, badNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix rotate rejects nan") ) return 0;
		if ( !check(xgeShapeExMatrixSkew(&skew, 0.0f, badInf) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx matrix skew rejects inf") ) return 0;
	}
	{
		xge_shape_ex resetShape = NULL;
		xge_shape_ex resetClip = NULL;
		xge_shape_ex resetMask = NULL;
		xge_shape_ex resetGotClip = NULL;
		xge_shape_ex resetGotMask = NULL;
		const uint8_t* resetCommands = NULL;
		const xge_vec2_t* resetPoints = NULL;
		xge_shape_ex_matrix_t resetTransform;
		uint32_t resetColor = 0;
		float resetFloat = 0.0f;
		int resetCommandCount = -1;
		int resetPointCount = -1;
		int resetInt = -1;
		int resetClipCount = -1;
		int resetMaskMethod = -1;
		int resetMaskTargetType = -1;

		ret = xgeShapeExCreate(&resetShape);
		if ( !check((ret == XGE_OK) && (resetShape != NULL), "ShapeEx reset compat create") ||
		     !check(xgeShapeExCreate(&resetClip) == XGE_OK, "ShapeEx reset compat clip create") ||
		     !check(xgeShapeExCreate(&resetMask) == XGE_OK, "ShapeEx reset compat mask create") ) {
			xgeShapeExDestroy(resetMask);
			xgeShapeExDestroy(resetClip);
			xgeShapeExDestroy(resetShape);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(resetShape, 1.0f, 2.0f, 8.0f, 6.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx reset compat path") ||
		     !check(xgeShapeExAppendRect(resetClip, 2.0f, 3.0f, 4.0f, 3.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx reset compat clip path") ||
		     !check(xgeShapeExAppendRect(resetMask, 3.0f, 2.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx reset compat mask path") ||
		     !check(xgeShapeExFillColor(resetShape, XGE_COLOR_RGBA(11, 22, 33, 44)) == XGE_OK, "ShapeEx reset compat fill") ||
		     !check(xgeShapeExStrokeColor(resetShape, XGE_COLOR_RGBA(55, 66, 77, 88)) == XGE_OK, "ShapeEx reset compat stroke") ||
		     !check(xgeShapeExStrokeWidth(resetShape, 3.0f) == XGE_OK, "ShapeEx reset compat stroke width") ||
		     !check(xgeShapeExStrokeCap(resetShape, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx reset compat stroke cap") ||
		     !check(xgeShapeExStrokeJoin(resetShape, XGE_SHAPE_EX_JOIN_MITER) == XGE_OK, "ShapeEx reset compat stroke join") ||
		     !check(xgeShapeExFillRule(resetShape, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx reset compat fill rule") ||
		     !check(xgeShapeExOpacity(resetShape, 0.6f) == XGE_OK, "ShapeEx reset compat opacity") ||
		     !check(xgeShapeExVisible(resetShape, 0) == XGE_OK, "ShapeEx reset compat visible") ||
		     !check(xgeShapeExTransformTranslate(resetShape, 3.0f, 4.0f) == XGE_OK, "ShapeEx reset compat transform") ||
		     !check(xgeShapeExClipShapeAdd(resetShape, resetClip) == XGE_OK, "ShapeEx reset compat clip") ||
		     !check(xgeShapeExMaskShapeSet(resetShape, resetMask, XGE_SHAPE_EX_MASK_LUMA) == XGE_OK, "ShapeEx reset compat mask") ) {
			xgeShapeExDestroy(resetMask);
			xgeShapeExDestroy(resetClip);
			xgeShapeExDestroy(resetShape);
			return 0;
		}
		if ( !check(xgeShapeExReset(resetShape) == XGE_OK, "ShapeEx reset keeps paint state") ) {
			xgeShapeExDestroy(resetMask);
			xgeShapeExDestroy(resetClip);
			xgeShapeExDestroy(resetShape);
			return 0;
		}
		ret = xgeShapeExGetPath(resetShape, &resetCommands, &resetCommandCount, &resetPoints, &resetPointCount);
		if ( !check((ret == XGE_OK) && (resetCommandCount == 0) && (resetPointCount == 0), "ShapeEx reset clears path only") ) {
			xgeShapeExDestroy(resetMask);
			xgeShapeExDestroy(resetClip);
			xgeShapeExDestroy(resetShape);
			return 0;
		}
		if ( !check(xgeShapeExFillColorGet(resetShape, &resetColor) == XGE_OK, "ShapeEx reset preserves fill get") ||
		     !check(resetColor == XGE_COLOR_RGBA(11, 22, 33, 44), "ShapeEx reset preserves fill value") ||
		     !check(xgeShapeExStrokeColorGet(resetShape, &resetColor) == XGE_OK, "ShapeEx reset preserves stroke get") ||
		     !check(resetColor == XGE_COLOR_RGBA(55, 66, 77, 88), "ShapeEx reset preserves stroke value") ||
		     !check(xgeShapeExStrokeWidthGet(resetShape, &resetFloat) == XGE_OK, "ShapeEx reset preserves stroke width get") ||
		     !check((resetFloat > 2.999f) && (resetFloat < 3.001f), "ShapeEx reset preserves stroke width value") ||
		     !check(xgeShapeExStrokeCapGet(resetShape, &resetInt) == XGE_OK, "ShapeEx reset preserves cap get") ||
		     !check(resetInt == XGE_SHAPE_EX_CAP_ROUND, "ShapeEx reset preserves cap value") ||
		     !check(xgeShapeExStrokeJoinGet(resetShape, &resetInt) == XGE_OK, "ShapeEx reset preserves join get") ||
		     !check(resetInt == XGE_SHAPE_EX_JOIN_MITER, "ShapeEx reset preserves join value") ||
		     !check(xgeShapeExFillRuleGet(resetShape, &resetInt) == XGE_OK, "ShapeEx reset preserves fill rule get") ||
		     !check(resetInt == XGE_SHAPE_EX_FILL_EVEN_ODD, "ShapeEx reset preserves fill rule value") ||
		     !check(xgeShapeExOpacityGet(resetShape, &resetFloat) == XGE_OK, "ShapeEx reset preserves opacity get") ||
		     !check((resetFloat > 0.599f) && (resetFloat < 0.601f), "ShapeEx reset preserves opacity value") ||
		     !check(xgeShapeExVisibleGet(resetShape, &resetInt) == XGE_OK, "ShapeEx reset preserves visible get") ||
		     !check(resetInt == 0, "ShapeEx reset preserves visible value") ||
		     !check(xgeShapeExTransformGet(resetShape, &resetTransform) == XGE_OK, "ShapeEx reset preserves transform get") ||
		     !check((resetTransform.fA == 1.0f) && (resetTransform.fD == 1.0f) && (resetTransform.fE == 3.0f) && (resetTransform.fF == 4.0f), "ShapeEx reset preserves transform value") ||
		     !check(xgeShapeExClipShapeGetCount(resetShape, &resetClipCount) == XGE_OK, "ShapeEx reset preserves clip count get") ||
		     !check((resetClipCount == 1) && (xgeShapeExClipShapeGetAt(resetShape, 0, &resetGotClip) == XGE_OK) && (resetGotClip == resetClip), "ShapeEx reset preserves clip") ||
		     !check(xgeShapeExMaskGet(resetShape, &resetMaskMethod, &resetMaskTargetType, &resetGotMask, NULL) == XGE_OK, "ShapeEx reset preserves mask get") ||
		     !check((resetMaskMethod == XGE_SHAPE_EX_MASK_LUMA) && (resetMaskTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE) && (resetGotMask == resetMask), "ShapeEx reset preserves mask") ) {
			xgeShapeExDestroy(resetMask);
			xgeShapeExDestroy(resetClip);
			xgeShapeExDestroy(resetShape);
			return 0;
		}
		xgeShapeExDestroy(resetShape);
		xgeShapeExDestroy(resetClip);
		xgeShapeExDestroy(resetMask);
	}
	{
		xge_shape_ex closeFirstDirect = NULL;
		xge_shape_ex cubicFirstDirect = NULL;
		xge_shape_ex lineFirstDirect = NULL;
		const uint8_t* closeFirstCommands = NULL;
		const uint8_t* cubicFirstCommands = NULL;
		const uint8_t* lineFirstCommands = NULL;
		const xge_vec2_t* closeFirstPoints = NULL;
		const xge_vec2_t* cubicFirstPoints = NULL;
		const xge_vec2_t* lineFirstPoints = NULL;
		xge_rect_t closeFirstBounds;
		xge_rect_t cubicFirstBounds;
		xge_rect_t lineFirstBounds;
		float cubicFirstLength;
		int closeFirstCommandCount = 0;
		int closeFirstPointCount = 0;
		int cubicFirstContains;
		int cubicFirstCommandCount = 0;
		int cubicFirstPointCount = 0;
		int lineFirstCommandCount = 0;
		int lineFirstPointCount = 0;

		ret = xgeShapeExCreate(&lineFirstDirect);
		if ( !check((ret == XGE_OK) && (lineFirstDirect != NULL), "ShapeEx direct line-first create") ) return 0;
		if ( !check(xgeShapeExLineTo(lineFirstDirect, 6.0f, 7.0f) == XGE_OK, "ShapeEx direct line-first lineTo") ) {
			xgeShapeExDestroy(lineFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetPath(lineFirstDirect, &lineFirstCommands, &lineFirstCommandCount, &lineFirstPoints, &lineFirstPointCount);
		if ( !check((ret == XGE_OK) && (lineFirstCommands != NULL) && (lineFirstPoints != NULL) &&
		            (lineFirstCommandCount == 1) && (lineFirstPointCount == 1) &&
		            (lineFirstCommands[0] == XGE_SHAPE_EX_CMD_LINE_TO) &&
		            (lineFirstPoints[0].fX == 6.0f) && (lineFirstPoints[0].fY == 7.0f),
		            "ShapeEx direct line-first path keeps line") ) {
			xgeShapeExDestroy(lineFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetBounds(lineFirstDirect, 0.05f, &lineFirstBounds);
		if ( !check((ret == XGE_OK) &&
		            (lineFirstBounds.fX > 5.999f) && (lineFirstBounds.fX < 6.001f) &&
		            (lineFirstBounds.fY > 6.999f) && (lineFirstBounds.fY < 7.001f) &&
		            (lineFirstBounds.fW >= 0.0f) && (lineFirstBounds.fW < 0.001f) &&
		            (lineFirstBounds.fH >= 0.0f) && (lineFirstBounds.fH < 0.001f),
		            "ShapeEx direct line-first bounds") ) {
			xgeShapeExDestroy(lineFirstDirect);
			return 0;
		}
		xgeShapeExDestroy(lineFirstDirect);
		ret = xgeShapeExCreate(&cubicFirstDirect);
		if ( !check((ret == XGE_OK) && (cubicFirstDirect != NULL), "ShapeEx direct cubic-first create") ) return 0;
		if ( !check(xgeShapeExCubicTo(cubicFirstDirect, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f) == XGE_OK, "ShapeEx direct cubic-first cubicTo") ) {
			xgeShapeExDestroy(cubicFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetPath(cubicFirstDirect, &cubicFirstCommands, &cubicFirstCommandCount, &cubicFirstPoints, &cubicFirstPointCount);
		if ( !check((ret == XGE_OK) && (cubicFirstCommands != NULL) && (cubicFirstPoints != NULL) &&
		            (cubicFirstCommandCount == 1) && (cubicFirstPointCount == 3) &&
		            (cubicFirstCommands[0] == XGE_SHAPE_EX_CMD_CUBIC_TO) &&
		            (cubicFirstPoints[0].fX == 1.0f) && (cubicFirstPoints[0].fY == 2.0f) &&
		            (cubicFirstPoints[1].fX == 3.0f) && (cubicFirstPoints[1].fY == 4.0f) &&
		            (cubicFirstPoints[2].fX == 5.0f) && (cubicFirstPoints[2].fY == 6.0f),
		            "ShapeEx direct cubic-first path keeps cubic") ) {
			xgeShapeExDestroy(cubicFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetBounds(cubicFirstDirect, 0.05f, &cubicFirstBounds);
		if ( !check((ret == XGE_OK) && (cubicFirstBounds.fW == 0.0f) && (cubicFirstBounds.fH == 0.0f), "ShapeEx direct cubic-first bounds empty") ) {
			xgeShapeExDestroy(cubicFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetLength(cubicFirstDirect, 0.05f, &cubicFirstLength);
		if ( !check((ret == XGE_OK) && (cubicFirstLength == 0.0f), "ShapeEx direct cubic-first length empty") ) {
			xgeShapeExDestroy(cubicFirstDirect);
			return 0;
		}
		ret = xgeShapeExContainsPoint(cubicFirstDirect, 5.0f, 6.0f, 0.05f, &cubicFirstContains);
		if ( !check((ret == XGE_OK) && !cubicFirstContains, "ShapeEx direct cubic-first hit empty") ) {
			xgeShapeExDestroy(cubicFirstDirect);
			return 0;
		}
		xgeShapeExDestroy(cubicFirstDirect);
		ret = xgeShapeExCreate(&closeFirstDirect);
		if ( !check((ret == XGE_OK) && (closeFirstDirect != NULL), "ShapeEx direct close-first create") ) return 0;
		if ( !check(xgeShapeExClose(closeFirstDirect) == XGE_OK, "ShapeEx direct close-first close") ) {
			xgeShapeExDestroy(closeFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetPath(closeFirstDirect, &closeFirstCommands, &closeFirstCommandCount, &closeFirstPoints, &closeFirstPointCount);
		if ( !check((ret == XGE_OK) && (closeFirstCommands != NULL) &&
		            (closeFirstCommandCount == 1) && (closeFirstPointCount == 0) &&
		            (closeFirstCommands[0] == XGE_SHAPE_EX_CMD_CLOSE) && (closeFirstPoints == NULL),
		            "ShapeEx direct close-first path keeps close") ) {
			xgeShapeExDestroy(closeFirstDirect);
			return 0;
		}
		if ( !check(xgeShapeExClose(closeFirstDirect) == XGE_OK, "ShapeEx direct close-first duplicate close") ) {
			xgeShapeExDestroy(closeFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetPath(closeFirstDirect, &closeFirstCommands, &closeFirstCommandCount, &closeFirstPoints, &closeFirstPointCount);
		if ( !check((ret == XGE_OK) && (closeFirstCommandCount == 1) && (closeFirstPointCount == 0) &&
		            (closeFirstCommands[0] == XGE_SHAPE_EX_CMD_CLOSE),
		            "ShapeEx direct close-first duplicate close ignored") ) {
			xgeShapeExDestroy(closeFirstDirect);
			return 0;
		}
		ret = xgeShapeExGetBounds(closeFirstDirect, 0.05f, &closeFirstBounds);
		if ( !check((ret == XGE_OK) && (closeFirstBounds.fW == 0.0f) && (closeFirstBounds.fH == 0.0f), "ShapeEx direct close-first bounds empty") ) {
			xgeShapeExDestroy(closeFirstDirect);
			return 0;
		}
		xgeShapeExDestroy(closeFirstDirect);
	}
	if ( !check(xgeShapeExMoveTo(shape, 0.0f, 0.0f) == XGE_OK, "ShapeEx moveTo") ) return 0;
	if ( !check(xgeShapeExLineTo(shape, 10.0f, 0.0f) == XGE_OK, "ShapeEx lineTo") ) return 0;
	if ( !check(xgeShapeExQuadTo(shape, 12.0f, 6.0f, 10.0f, 10.0f) == XGE_OK, "ShapeEx quadTo") ) return 0;
	if ( !check(xgeShapeExCubicTo(shape, 8.0f, 14.0f, 2.0f, 14.0f, 0.0f, 10.0f) == XGE_OK, "ShapeEx cubicTo") ) return 0;
	if ( !check(xgeShapeExClose(shape) == XGE_OK, "ShapeEx close") ) return 0;
	{
		const uint8_t* closeCommands = NULL;
		const xge_vec2_t* closePoints = NULL;
		int closeCommandCount = 0;
		int closePointCount = 0;

		if ( !check(xgeShapeExClose(shape) == XGE_OK, "ShapeEx duplicate close") ) return 0;
		ret = xgeShapeExGetPath(shape, &closeCommands, &closeCommandCount, &closePoints, &closePointCount);
		if ( !check((ret == XGE_OK) && (closeCommands != NULL) && (closePoints != NULL) &&
		            (closeCommandCount == 5) && (closePointCount == 7) &&
		            (closeCommands[4] == XGE_SHAPE_EX_CMD_CLOSE),
		            "ShapeEx duplicate close ignored") ) return 0;
	}
	if ( !check(xgeShapeExAppendSvgPath(shape, "M20 20 A8 8 0 0 1 28 28 S40 36 48 28") == XGE_OK, "ShapeEx SVG path") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 4.0f, 4.0f, 18.0f, 12.0f, 3.0f, 3.0f, 1) == XGE_OK, "ShapeEx append rect") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 5.0f, 5.0f, 10.0f, 8.0f, 2.0f, 2.0f, 0) == XGE_OK, "ShapeEx append ccw rounded rect") ) return 0;
	if ( !check(xgeShapeExAppendTriangle(shape, 2.0f, 18.0f, 8.0f, 24.0f, 14.0f, 18.0f, 1) == XGE_OK, "ShapeEx append triangle") ) return 0;
	if ( !check(xgeShapeExAppendTriangle(shape, 16.0f, 18.0f, 22.0f, 24.0f, 28.0f, 18.0f, 0) == XGE_OK, "ShapeEx append ccw triangle") ) return 0;
	if ( !check(xgeShapeExAppendLine(shape, 30.0f, 2.0f, 42.0f, 2.0f) == XGE_OK, "ShapeEx append line") ) return 0;
	if ( !check(xgeShapeExAppendLine(shape, invalidInf, 2.0f, 42.0f, 2.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx line rejects inf coordinate") ) return 0;
	{
		xge_vec2_t polylinePoints[3] = {{30.0f, 4.0f}, {36.0f, 10.0f}, {42.0f, 4.0f}};
		xge_vec2_t polygonPoints[4] = {{30.0f, 12.0f}, {42.0f, 12.0f}, {42.0f, 18.0f}, {30.0f, 18.0f}};
		xge_vec2_t badPoints[2] = {{0.0f, 0.0f}, {invalidNaN, 2.0f}};

		if ( !check(xgeShapeExAppendPolyline(shape, polylinePoints, 3) == XGE_OK, "ShapeEx append polyline") ) return 0;
		if ( !check(xgeShapeExAppendPolygon(shape, polygonPoints, 4) == XGE_OK, "ShapeEx append polygon") ) return 0;
		if ( !check(xgeShapeExAppendPolyline(shape, NULL, 3) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx polyline rejects null points") ) return 0;
		if ( !check(xgeShapeExAppendPolygon(shape, badPoints, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx polygon rejects nan point") ) return 0;
	}
	if ( !check(xgeShapeExAppendCapsule(shape, 30.0f, 18.0f, 14.0f, 6.0f, 1) == XGE_OK, "ShapeEx append capsule") ) return 0;
	if ( !check(xgeShapeExAppendCapsule(shape, 46.0f, 18.0f, 6.0f, 14.0f, 0) == XGE_OK, "ShapeEx append ccw capsule") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 5.0f, 5.0f, 1) == XGE_OK, "ShapeEx append circle") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 3.0f, 4.0f, 0) == XGE_OK, "ShapeEx append ccw ellipse") ) return 0;
	if ( !check(xgeShapeExAppendEllipse(shape, 20.0f, 12.0f, 6.0f, 3.0f, 1) == XGE_OK, "ShapeEx append ellipse") ) return 0;
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
		if ( !check((points[1].fX > 11.65684f) && (points[1].fX < 11.65686f) && (points[1].fY == 16.0f) &&
		            (points[2].fX == 13.0f) && (points[2].fY > 17.79085f) && (points[2].fY < 17.79088f),
		            "ShapeEx exact circle ThorVG kappa controls") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((points[0].fX == 10.0f) && (points[0].fY == 16.0f) && (points[3].fX == 13.0f) && (points[3].fY == 20.0f) && (points[6].fX == 10.0f) && (points[6].fY == 24.0f) && (points[9].fX == 7.0f) && (points[9].fY == 20.0f) && (points[12].fX == 10.0f) && (points[12].fY == 16.0f), "ShapeEx exact circle ThorVG endpoints") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check(xgeShapeExReset(exact) == XGE_OK, "ShapeEx exact poly reset") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		{
			if ( !check(xgeShapeExAppendLine(exact, 0.0f, 3.0f, 6.0f, 3.0f) == XGE_OK, "ShapeEx exact line append") ) {
				xgeShapeExDestroy(exact);
				return 0;
			}
		}
		{
			xge_vec2_t exactPolyline[3] = {{1.0f, 1.0f}, {4.0f, 2.0f}, {7.0f, 1.0f}};
			xge_vec2_t exactPolygon[3] = {{2.0f, 6.0f}, {5.0f, 9.0f}, {8.0f, 6.0f}};

			if ( !check(xgeShapeExAppendPolyline(exact, exactPolyline, 3) == XGE_OK, "ShapeEx exact polyline append") ||
			     !check(xgeShapeExAppendPolygon(exact, exactPolygon, 3) == XGE_OK, "ShapeEx exact polygon append") ) {
				xgeShapeExDestroy(exact);
				return 0;
			}
		}
		ret = xgeShapeExGetPath(exact, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 9) && (pointCount == 8), "ShapeEx exact poly path counts") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((commands[0] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[1] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[2] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[3] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[4] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[5] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[6] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[7] == XGE_SHAPE_EX_CMD_LINE_TO) && (commands[8] == XGE_SHAPE_EX_CMD_CLOSE), "ShapeEx exact poly commands") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		if ( !check((points[0].fX == 0.0f) && (points[0].fY == 3.0f) && (points[1].fX == 6.0f) && (points[1].fY == 3.0f) && (points[2].fX == 1.0f) && (points[2].fY == 1.0f) && (points[4].fX == 7.0f) && (points[4].fY == 1.0f) && (points[5].fX == 2.0f) && (points[5].fY == 6.0f) && (points[7].fX == 8.0f) && (points[7].fY == 6.0f), "ShapeEx exact poly point order") ) {
			xgeShapeExDestroy(exact);
			return 0;
		}
		xgeShapeExDestroy(exact);
	}
	{
		xge_shape_ex quad = NULL;
		const uint8_t* commands = NULL;
		const xge_vec2_t* points = NULL;
		uint8_t appendCommands[4] = {
			XGE_SHAPE_EX_CMD_MOVE_TO,
			XGE_SHAPE_EX_CMD_QUAD_TO,
			XGE_SHAPE_EX_CMD_LINE_TO,
			XGE_SHAPE_EX_CMD_CLOSE
		};
		uint8_t badCommands[1] = {XGE_SHAPE_EX_CMD_QUAD_TO};
		xge_vec2_t appendPoints[4] = {
			{0.0f, 0.0f},
			{5.0f, 10.0f},
			{10.0f, 0.0f},
			{12.0f, 0.0f}
		};
		xge_vec2_t badPoints[2] = {{5.0f, 10.0f}, {10.0f, 0.0f}};
		xge_rect_t quadBounds;
		char svgPathData[128];
		char smallPathData[8];
		int commandCount = 0;
		int pointCount = 0;
		int requiredSize = 0;

		ret = xgeShapeExCreate(&quad);
		if ( !check((ret == XGE_OK) && (quad != NULL), "ShapeEx quad command create") ) return 0;
		if ( !check(xgeShapeExMoveTo(quad, 0.0f, 0.0f) == XGE_OK, "ShapeEx quad command move") ||
		     !check(xgeShapeExQuadTo(quad, 5.0f, 10.0f, 10.0f, 0.0f) == XGE_OK, "ShapeEx quad command append") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetPath(quad, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 2) && (pointCount == 3), "ShapeEx quad command path counts") ||
		     !check((commands[0] == XGE_SHAPE_EX_CMD_MOVE_TO) && (commands[1] == XGE_SHAPE_EX_CMD_QUAD_TO), "ShapeEx quad command preserved") ||
		     !check((points[1].fX == 5.0f) && (points[1].fY == 10.0f) && (points[2].fX == 10.0f) && (points[2].fY == 0.0f), "ShapeEx quad command points") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetBounds(quad, 0.01f, &quadBounds);
		if ( !check((ret == XGE_OK) && (quadBounds.fX > -0.01f) && (quadBounds.fX < 0.01f) &&
		            (quadBounds.fW > 9.99f) && (quadBounds.fW < 10.01f) &&
		            (quadBounds.fH > 4.9f) && (quadBounds.fH < 5.1f), "ShapeEx quad command bounds") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		if ( !check(xgeShapeExReset(quad) == XGE_OK, "ShapeEx append quad reset") ||
		     !check(xgeShapeExAppendPath(quad, badCommands, 1, badPoints, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx append quad rejects missing current") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetPath(quad, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 0) && (pointCount == 0), "ShapeEx append quad invalid keeps path empty") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(quad, appendCommands, 4, appendPoints, 4) == XGE_OK, "ShapeEx append quad path") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetPath(quad, &commands, &commandCount, &points, &pointCount);
		if ( !check((ret == XGE_OK) && (commandCount == 4) && (pointCount == 4), "ShapeEx append quad path counts") ||
		     !check(commands[1] == XGE_SHAPE_EX_CMD_QUAD_TO, "ShapeEx append quad command preserved") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetSvgPathData(quad, NULL, 0, &requiredSize);
		if ( !check((ret == XGE_OK) && (requiredSize == (int)strlen("M0 0 Q5 10 10 0 L12 0 Z") + 1), "ShapeEx SVG path data size query") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetSvgPathData(quad, smallPathData, (int)sizeof(smallPathData), &requiredSize);
		if ( !check((ret == XGE_ERROR_BUFFER_TOO_SMALL) && (smallPathData[sizeof(smallPathData) - 1] == '\0'), "ShapeEx SVG path data small buffer") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		ret = xgeShapeExGetSvgPathData(quad, svgPathData, (int)sizeof(svgPathData), &requiredSize);
		if ( !check((ret == XGE_OK) && (strcmp(svgPathData, "M0 0 Q5 10 10 0 L12 0 Z") == 0), "ShapeEx SVG path data export") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		{
			xge_shape_ex roundTrip = NULL;
			const uint8_t* roundCommands = NULL;
			const xge_vec2_t* roundPoints = NULL;
			int roundCommandCount = 0;
			int roundPointCount = 0;

			ret = xgeShapeExCreate(&roundTrip);
			if ( !check((ret == XGE_OK) && (roundTrip != NULL), "ShapeEx SVG path data roundtrip create") ) {
				xgeShapeExDestroy(quad);
				return 0;
			}
			if ( !check(xgeShapeExAppendSvgPath(roundTrip, svgPathData) == XGE_OK, "ShapeEx SVG path data roundtrip parse") ) {
				xgeShapeExDestroy(roundTrip);
				xgeShapeExDestroy(quad);
				return 0;
			}
			ret = xgeShapeExGetPath(roundTrip, &roundCommands, &roundCommandCount, &roundPoints, &roundPointCount);
			if ( !check((ret == XGE_OK) && (roundCommandCount == commandCount) && (roundPointCount == pointCount) &&
			            (roundCommands[1] == XGE_SHAPE_EX_CMD_QUAD_TO) &&
			            (roundPoints[2].fX == points[2].fX) && (roundPoints[2].fY == points[2].fY), "ShapeEx SVG path data roundtrip path") ) {
				xgeShapeExDestroy(roundTrip);
				xgeShapeExDestroy(quad);
				return 0;
			}
			xgeShapeExDestroy(roundTrip);
		}
		ret = xgeShapeExGetBounds(quad, 0.01f, &quadBounds);
		if ( !check((ret == XGE_OK) && (quadBounds.fX > -0.01f) && (quadBounds.fX < 0.01f) &&
		            (quadBounds.fW > 11.99f) && (quadBounds.fW < 12.01f) &&
		            (quadBounds.fH > 4.9f) && (quadBounds.fH < 5.1f), "ShapeEx append quad bounds") ) {
			xgeShapeExDestroy(quad);
			return 0;
		}
		xgeShapeExDestroy(quad);
	}
	{
		xge_shape_ex ellipse = NULL;
		xge_rect_t ellipseBounds;

		ret = xgeShapeExCreate(&ellipse);
		if ( !check((ret == XGE_OK) && (ellipse != NULL), "ShapeEx ellipse alias create") ) return 0;
		if ( !check(xgeShapeExAppendEllipse(ellipse, 12.0f, 18.0f, 5.0f, 3.0f, 1) == XGE_OK, "ShapeEx ellipse alias append") ) {
			xgeShapeExDestroy(ellipse);
			return 0;
		}
		ret = xgeShapeExGetBounds(ellipse, 0.05f, &ellipseBounds);
		if ( !check((ret == XGE_OK) && (ellipseBounds.fX > 6.9f) && (ellipseBounds.fX < 7.1f) && (ellipseBounds.fY > 14.9f) && (ellipseBounds.fY < 15.1f) && (ellipseBounds.fW > 9.9f) && (ellipseBounds.fW < 10.1f) && (ellipseBounds.fH > 5.9f) && (ellipseBounds.fH < 6.1f), "ShapeEx ellipse alias bounds") ) {
			xgeShapeExDestroy(ellipse);
			return 0;
		}
		xgeShapeExDestroy(ellipse);
	}
	{
		xge_shape_ex hit = NULL;
		xge_shape_ex clip = NULL;
		xge_shape_ex clip2 = NULL;
		xge_shape_ex_matrix_t parent;
		int contains = -1;

		ret = xgeShapeExCreate(&hit);
		if ( !check((ret == XGE_OK) && (hit != NULL), "ShapeEx contains create") ) return 0;
		if ( !check(xgeShapeExContainsPoint(hit, 0.0f, 0.0f, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx contains rejects null output") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExContainsPoint(hit, invalidNaN, 0.0f, 0.05f, &contains) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx contains rejects nan point") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx contains visible fill") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains rect append") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 5.0f, 5.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains rect inside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 10.0f, 5.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains rect boundary") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 12.0f, 5.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains rect outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset even odd") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains even odd outer") ||
		     !check(xgeShapeExAppendRect(hit, 2.0f, 2.0f, 6.0f, 6.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains even odd inner") ||
		     !check(xgeShapeExFillRule(hit, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx contains even odd rule") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 1.0f, 1.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains even odd outer area") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 5.0f, 5.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains even odd hole") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExFillRule(hit, XGE_SHAPE_EX_FILL_NON_ZERO) == XGE_OK, "ShapeEx contains non-zero rule") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 5.0f, 5.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains non-zero same winding") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset self intersection") ||
		     !check(xgeShapeExAppendSvgPath(hit, "M0 0L10 10L0 10L10 0Z") == XGE_OK, "ShapeEx contains self intersection path") ||
		     !check(xgeShapeExFillRule(hit, XGE_SHAPE_EX_FILL_NON_ZERO) == XGE_OK, "ShapeEx contains self intersection rule") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 5.0f, 2.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains self intersection upper lobe") ||
		     !check((xgeShapeExContainsPoint(hit, 5.0f, 8.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains self intersection lower lobe") ||
		     !check((xgeShapeExContainsPoint(hit, 1.0f, 5.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains self intersection outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset transform") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains transform rect") ||
		     !check(xgeShapeExTransformTranslate(hit, 5.0f, 7.0f) == XGE_OK, "ShapeEx contains transform translate") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 6.0f, 8.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains transformed inside") ||
		     !check((xgeShapeExContainsPoint(hit, 1.0f, 1.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains transformed outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset parent") ||
		     !check(xgeShapeExTransformIdentity(hit) == XGE_OK, "ShapeEx contains parent transform identity") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains parent rect") ||
		     !check(xgeShapeExMatrixTranslate(&parent, 20.0f, 30.0f) == XGE_OK, "ShapeEx contains parent matrix") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPointEx(hit, 22.0f, 32.0f, 0.05f, &parent, &contains) == XGE_OK) && contains, "ShapeEx contains parent inside") ||
		     !check((xgeShapeExContainsPointEx(hit, 2.0f, 2.0f, 0.05f, &parent, &contains) == XGE_OK) && !contains, "ShapeEx contains parent outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset clip rect") ||
		     !check(xgeShapeExTransformIdentity(hit) == XGE_OK, "ShapeEx contains clip rect transform identity") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clip rect shape") ||
		     !check(xgeShapeExClipRectSet(hit, (xge_rect_t){0.0f, 0.0f, 5.0f, 5.0f}) == XGE_OK, "ShapeEx contains clip rect set") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains clip rect inside") ||
		     !check((xgeShapeExContainsPoint(hit, 6.0f, 4.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains clip rect outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset clip shape") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clip clear") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clip target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains clip create") ||
		     !check(xgeShapeExAppendRect(clip, 2.0f, 2.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clip append") ||
		     !check(xgeShapeExClipShapeAdd(hit, clip) == XGE_OK, "ShapeEx contains clip add") ||
		     !check(xgeShapeExCreate(&clip2) == XGE_OK, "ShapeEx contains clip union create") ||
		     !check(xgeShapeExAppendRect(clip2, 7.0f, 2.0f, 2.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clip union append") ||
		     !check(xgeShapeExClipShapeAdd(hit, clip2) == XGE_OK, "ShapeEx contains clip union add") ) {
			xgeShapeExDestroy(clip2);
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip2);
		clip2 = NULL;
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 3.0f, 3.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains clip shape inside") ||
		     !check((xgeShapeExContainsPoint(hit, 8.0f, 3.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains clip shape union inside") ||
		     !check((xgeShapeExContainsPoint(hit, 1.0f, 1.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains clip shape outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset subtract clip shape") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear subtract clip shape") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains subtract target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains subtract clip create") ||
		     !check(xgeShapeExAppendRect(clip, 3.0f, 3.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains subtract clip append") ||
		     !check(xgeShapeExClipShapeAddEx(hit, clip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx contains subtract clip add") ) {
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 2.0f, 2.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains subtract outside cut") ||
		     !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains subtract inside cut") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset nested subtract clip shape") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear nested subtract clip shape") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains nested subtract target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains nested subtract clip create") ||
		     !check(xgeShapeExAppendRect(clip, 3.0f, 3.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains nested subtract clip append") ||
		     !check(xgeShapeExCreate(&clip2) == XGE_OK, "ShapeEx contains nested subtract keep create") ||
		     !check(xgeShapeExAppendRect(clip2, 5.0f, 5.0f, 1.5f, 1.5f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains nested subtract keep append") ||
		     !check(xgeShapeExClipShapeAddEx(clip, clip2, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx contains nested subtract keep add") ||
		     !check(xgeShapeExClipShapeAddEx(hit, clip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx contains nested subtract clip add") ) {
			xgeShapeExDestroy(clip2);
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip2);
		clip2 = NULL;
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 2.0f, 2.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains nested subtract outside cut") ||
		     !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains nested subtract inside cut") ||
		     !check((xgeShapeExContainsPoint(hit, 5.5f, 5.5f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains nested subtract restored area") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset include subtract clip shape") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear include subtract clip shape") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains include subtract target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains include clip create") ||
		     !check(xgeShapeExAppendRect(clip, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains include clip append") ||
		     !check(xgeShapeExClipShapeAdd(hit, clip) == XGE_OK, "ShapeEx contains include clip add") ||
		     !check(xgeShapeExCreate(&clip2) == XGE_OK, "ShapeEx contains include subtract clip create") ||
		     !check(xgeShapeExAppendRect(clip2, 3.0f, 3.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains include subtract clip append") ||
		     !check(xgeShapeExClipShapeAddEx(hit, clip2, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx contains include subtract clip add") ) {
			xgeShapeExDestroy(clip2);
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip2);
		clip2 = NULL;
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 2.0f, 2.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains include subtract inside include") ||
		     !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains include subtract inside cut") ||
		     !check((xgeShapeExContainsPoint(hit, 12.0f, 2.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains include subtract outside include") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset subtract nested include") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear subtract nested include") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains subtract nested include target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains subtract nested include root create") ||
		     !check(xgeShapeExAppendRect(clip, 3.0f, 3.0f, 12.0f, 12.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains subtract nested include root append") ||
		     !check(xgeShapeExCreate(&clip2) == XGE_OK, "ShapeEx contains subtract nested include child create") ||
		     !check(xgeShapeExAppendRect(clip2, 6.0f, 6.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains subtract nested include child append") ||
		     !check(xgeShapeExClipShapeAdd(clip, clip2) == XGE_OK, "ShapeEx contains subtract nested include child add") ||
		     !check(xgeShapeExClipShapeAddEx(hit, clip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx contains subtract nested include root add") ) {
			xgeShapeExDestroy(clip2);
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip2);
		clip2 = NULL;
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains subtract nested include outside child") ||
		     !check((xgeShapeExContainsPoint(hit, 7.0f, 7.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains subtract nested include inside child") ||
		     !check((xgeShapeExContainsPoint(hit, 17.0f, 7.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains subtract nested include outside root") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset clipped clip shape") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear clipped clip shape") ||
		     !check(xgeShapeExAppendRect(hit, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clipped clip target") ||
		     !check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx contains clipped clip create") ||
		     !check(xgeShapeExAppendRect(clip, 0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx contains clipped clip append") ||
		     !check(xgeShapeExClipRectSet(clip, (xge_rect_t){0.0f, 0.0f, 5.0f, 5.0f}) == XGE_OK, "ShapeEx contains clipped clip rect set") ||
		     !check(xgeShapeExClipShapeAdd(hit, clip) == XGE_OK, "ShapeEx contains clipped clip add") ) {
			xgeShapeExDestroy(clip);
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(clip);
		clip = NULL;
		if ( !check((xgeShapeExContainsPoint(hit, 4.0f, 4.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains clipped clip shape inside") ||
		     !check((xgeShapeExContainsPoint(hit, 7.0f, 4.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains clipped clip shape outside clip rect") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset stroke hit") ||
		     !check(xgeShapeExClipClear(hit) == XGE_OK, "ShapeEx contains clear clip before stroke hit") ||
		     !check(xgeShapeExMoveTo(hit, 0.0f, 0.0f) == XGE_OK, "ShapeEx contains stroke hit move") ||
		     !check(xgeShapeExLineTo(hit, 10.0f, 0.0f) == XGE_OK, "ShapeEx contains stroke hit line") ||
		     !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx contains stroke hit transparent fill") ||
		     !check(xgeShapeExStrokeColor(hit, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx contains stroke hit color") ||
		     !check(xgeShapeExStrokeWidth(hit, 4.0f) == XGE_OK, "ShapeEx contains stroke hit width") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx contains stroke hit cap") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 5.0f, 1.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains stroke-only hit") ||
		     !check((xgeShapeExContainsPoint(hit, 5.0f, 3.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains stroke-only miss") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset miter join hit") ||
		     !check(xgeShapeExMoveTo(hit, 0.0f, 10.0f) == XGE_OK, "ShapeEx contains miter join move") ||
		     !check(xgeShapeExLineTo(hit, 10.0f, 0.0f) == XGE_OK, "ShapeEx contains miter join line 1") ||
		     !check(xgeShapeExLineTo(hit, 20.0f, 10.0f) == XGE_OK, "ShapeEx contains miter join line 2") ||
		     !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx contains miter join transparent fill") ||
		     !check(xgeShapeExStrokeColor(hit, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx contains miter join color") ||
		     !check(xgeShapeExStrokeWidth(hit, 4.0f) == XGE_OK, "ShapeEx contains miter join width") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx contains miter join cap") ||
		     !check(xgeShapeExStrokeJoin(hit, XGE_SHAPE_EX_JOIN_MITER) == XGE_OK, "ShapeEx contains miter join style") ||
		     !check(xgeShapeExStrokeMiterLimit(hit, 8.0f) == XGE_OK, "ShapeEx contains miter join limit") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, -2.5f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains miter join point") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, -3.2f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains miter join outside") ||
		     !check(xgeShapeExStrokeJoin(hit, XGE_SHAPE_EX_JOIN_BEVEL) == XGE_OK, "ShapeEx contains bevel join style") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, -1.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains bevel join point") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, -2.5f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains bevel join outside") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset reverse join") ||
		     !check(xgeShapeExMoveTo(hit, 0.0f, 0.0f) == XGE_OK, "ShapeEx contains reverse join move") ||
		     !check(xgeShapeExLineTo(hit, 10.0f, 0.0f) == XGE_OK, "ShapeEx contains reverse join forward") ||
		     !check(xgeShapeExLineTo(hit, 0.0f, 0.0f) == XGE_OK, "ShapeEx contains reverse join backward") ||
		     !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx contains reverse join transparent fill") ||
		     !check(xgeShapeExStrokeColor(hit, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx contains reverse join stroke") ||
		     !check(xgeShapeExStrokeWidth(hit, 4.0f) == XGE_OK, "ShapeEx contains reverse join width") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx contains reverse join cap") ||
		     !check(xgeShapeExStrokeJoin(hit, XGE_SHAPE_EX_JOIN_ROUND) == XGE_OK, "ShapeEx contains reverse round join") ||
		     !check((xgeShapeExContainsPoint(hit, 11.8f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains reverse round join hit") ||
		     !check((xgeShapeExContainsPoint(hit, 12.2f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains reverse round join miss") ||
		     !check(xgeShapeExStrokeJoin(hit, XGE_SHAPE_EX_JOIN_BEVEL) == XGE_OK, "ShapeEx contains reverse bevel join") ||
		     !check((xgeShapeExContainsPoint(hit, 11.8f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains reverse bevel join miss") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset dash hit") ||
		     !check(xgeShapeExMoveTo(hit, 0.0f, 0.0f) == XGE_OK, "ShapeEx contains dash hit move") ||
		     !check(xgeShapeExLineTo(hit, 20.0f, 0.0f) == XGE_OK, "ShapeEx contains dash hit line") ||
		     !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx contains dash transparent fill") ||
		     !check(xgeShapeExStrokeColor(hit, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx contains dash color") ||
		     !check(xgeShapeExStrokeWidth(hit, 2.0f) == XGE_OK, "ShapeEx contains dash width") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx contains dash cap") ||
		     !check(xgeShapeExStrokeDash(hit, dash, 2, 0.0f) == XGE_OK, "ShapeEx contains dash pattern") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 2.0f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains dash hit") ||
		     !check((xgeShapeExContainsPoint(hit, 5.0f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains dash gap miss") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExStrokeDash(hit, dashOdd, 3, 0.0f) == XGE_OK, "ShapeEx contains odd dash pattern") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(hit, 1.0f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains odd dash first hit") ||
		     !check((xgeShapeExContainsPoint(hit, 3.0f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains odd dash first gap") ||
		     !check((xgeShapeExContainsPoint(hit, 6.0f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains odd dash second hit") ||
		     !check((xgeShapeExContainsPoint(hit, 9.0f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains odd dash second gap") ||
		     !check((xgeShapeExContainsPoint(hit, 11.0f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains odd dash repeated hit") ||
		     !check((xgeShapeExContainsPoint(hit, 14.0f, 0.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains odd dash repeated gap") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		if ( !check(xgeShapeExReset(hit) == XGE_OK, "ShapeEx contains reset zero stroke hit") ||
		     !check(xgeShapeExMoveTo(hit, 10.0f, 10.0f) == XGE_OK, "ShapeEx contains zero stroke move") ||
		     !check(xgeShapeExLineTo(hit, 10.0f, 10.0f) == XGE_OK, "ShapeEx contains zero stroke line") ||
		     !check(xgeShapeExFillColor(hit, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx contains zero stroke transparent fill") ||
		     !check(xgeShapeExStrokeColor(hit, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx contains zero stroke color") ||
		     !check(xgeShapeExStrokeWidth(hit, 6.0f) == XGE_OK, "ShapeEx contains zero stroke width") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx contains zero butt cap") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, 10.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains zero butt cap miss") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx contains zero round cap") ||
		     !check((xgeShapeExContainsPoint(hit, 10.0f, 10.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains zero round cap hit") ||
		     !check((xgeShapeExContainsPoint(hit, 13.2f, 10.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains zero round cap miss") ||
		     !check(xgeShapeExStrokeCap(hit, XGE_SHAPE_EX_CAP_SQUARE) == XGE_OK, "ShapeEx contains zero square cap") ||
		     !check((xgeShapeExContainsPoint(hit, 12.9f, 12.9f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx contains zero square cap hit") ||
		     !check((xgeShapeExContainsPoint(hit, 13.2f, 13.2f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx contains zero square cap miss") ) {
			xgeShapeExDestroy(hit);
			return 0;
		}
		xgeShapeExDestroy(hit);
	}
	{
		xge_shape_ex zeroCap = NULL;
		xge_rect_t zeroBounds;

		ret = xgeShapeExCreate(&zeroCap);
		if ( !check((ret == XGE_OK) && (zeroCap != NULL), "ShapeEx zero cap bounds create") ) return 0;
		if ( !check(xgeShapeExMoveTo(zeroCap, 10.0f, 10.0f) == XGE_OK, "ShapeEx zero cap move") ||
		     !check(xgeShapeExLineTo(zeroCap, 10.0f, 10.0f) == XGE_OK, "ShapeEx zero cap line") ||
		     !check(xgeShapeExFillColor(zeroCap, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx zero cap transparent fill") ||
		     !check(xgeShapeExStrokeColor(zeroCap, XGE_COLOR_RGBA(255, 0, 0, 255)) == XGE_OK, "ShapeEx zero cap stroke color") ||
		     !check(xgeShapeExStrokeWidth(zeroCap, 6.0f) == XGE_OK, "ShapeEx zero cap stroke width") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		if ( !check(xgeShapeExStrokeCap(zeroCap, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx zero cap butt") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		ret = xgeShapeExGetBounds(zeroCap, 0.05f, &zeroBounds);
		if ( !check((ret == XGE_OK) &&
		            (zeroBounds.fX > 9.99f) && (zeroBounds.fX < 10.01f) &&
		            (zeroBounds.fY > 9.99f) && (zeroBounds.fY < 10.01f) &&
		            (zeroBounds.fW >= 0.0f) && (zeroBounds.fW < 0.01f) &&
		            (zeroBounds.fH >= 0.0f) && (zeroBounds.fH < 0.01f), "ShapeEx zero butt cap does not expand bounds") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		if ( !check(xgeShapeExStrokeCap(zeroCap, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx zero cap round") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		ret = xgeShapeExGetBounds(zeroCap, 0.05f, &zeroBounds);
		if ( !check((ret == XGE_OK) &&
		            (zeroBounds.fX > 6.99f) && (zeroBounds.fX < 7.01f) &&
		            (zeroBounds.fY > 6.99f) && (zeroBounds.fY < 7.01f) &&
		            (zeroBounds.fW > 5.99f) && (zeroBounds.fW < 6.01f) &&
		            (zeroBounds.fH > 5.99f) && (zeroBounds.fH < 6.01f), "ShapeEx zero round cap expands bounds") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		if ( !check(xgeShapeExStrokeCap(zeroCap, XGE_SHAPE_EX_CAP_SQUARE) == XGE_OK, "ShapeEx zero cap square") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		ret = xgeShapeExGetBounds(zeroCap, 0.05f, &zeroBounds);
		if ( !check((ret == XGE_OK) &&
		            (zeroBounds.fX > 6.99f) && (zeroBounds.fX < 7.01f) &&
		            (zeroBounds.fY > 6.99f) && (zeroBounds.fY < 7.01f) &&
		            (zeroBounds.fW > 5.99f) && (zeroBounds.fW < 6.01f) &&
		            (zeroBounds.fH > 5.99f) && (zeroBounds.fH < 6.01f), "ShapeEx zero square cap expands bounds") ) {
			xgeShapeExDestroy(zeroCap);
			return 0;
		}
		xgeShapeExDestroy(zeroCap);
	}
	if ( !check(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(1, 2, 3, 4)) == XGE_OK, "ShapeEx fill color") ) return 0;
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	stops[1].fOffset = 1.0f;
	stops[1].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
	badStops[0] = stops[0];
	badStops[1] = stops[1];
	badStops[0].fOffset = invalidNaN;
	if ( !check(xgeShapeExFillLinearGradient(shape, invalidInf, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill linear gradient rejects inf coordinate") ) return 0;
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, badStops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill linear gradient rejects nan stop") ) return 0;
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx linear gradient") ) return 0;
	if ( !check(xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) == XGE_OK, "ShapeEx gradient spread") ) return 0;
	if ( !check(xgeShapeExFillGradientSpread(shape, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill gradient rejects invalid spread") ) return 0;
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, 99, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill linear gradient rejects invalid units") ) return 0;
	matrix.fA = 0.8f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 0.1f;
	matrix.fF = 0.0f;
	if ( !check(xgeShapeExFillGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx gradient transform") ) return 0;
	{
		xge_shape_ex_matrix_t badMatrix = matrix;

		badMatrix.fE = invalidNaN;
		if ( !check(xgeShapeExFillGradientTransformSet(shape, &badMatrix) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill gradient transform rejects nan") ) return 0;
	}
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
		            (gotStops[0].fOffset == -0.25f) && (gotStops[0].iColor == unorderedStops[0].iColor) &&
		            (gotStops[1].fOffset == 0.6f) && (gotStops[1].iColor == unorderedStops[1].iColor) &&
		            (gotStops[2].fOffset == 0.2f) && (gotStops[2].iColor == unorderedStops[2].iColor) &&
		            (gotStops[3].fOffset == 1.5f) && (gotStops[3].iColor == unorderedStops[3].iColor), "ShapeEx linear gradient preserves source stops") ) return 0;
		{
			xge_shape_ex gradientClone = NULL;
			const xge_shape_ex_color_stop_t* cloneStops = NULL;
			int cloneCount = 0;

			if ( !check(xgeShapeExClone(shape, &gradientClone) == XGE_OK, "ShapeEx linear gradient source clone") ) return 0;
			if ( !check(xgeShapeExFillLinearGradientGet(gradientClone, NULL, NULL, NULL, NULL, NULL, &cloneStops, &cloneCount) == XGE_OK,
			            "ShapeEx linear gradient source clone get") ) {
				xgeShapeExDestroy(gradientClone);
				return 0;
			}
			if ( !check((cloneCount == 4) && (cloneStops != NULL) &&
			            (cloneStops[0].fOffset == -0.25f) && (cloneStops[2].fOffset == 0.2f) &&
			            (cloneStops[3].fOffset == 1.5f), "ShapeEx linear gradient clone preserves source stops") ) {
				xgeShapeExDestroy(gradientClone);
				return 0;
			}
			xgeShapeExDestroy(gradientClone);
		}
	}
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx radial gradient") ) return 0;
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, 99, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill radial gradient rejects invalid units") ) return 0;
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, invalidNaN, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill radial gradient rejects nan radius") ) return 0;
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.0f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx fill radial gradient accepts zero radius") ) return 0;
	{
		float radius = -1.0f;

		if ( !check(xgeShapeExFillRadialGradientGet(shape, NULL, NULL, &radius, NULL, NULL, NULL, NULL, NULL) == XGE_OK, "ShapeEx fill zero-radius gradient get") ) return 0;
		if ( !check(radius == 0.0f, "ShapeEx fill zero-radius gradient value") ) return 0;
	}
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx radial gradient restore") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		xge_shape_ex_matrix_t gotGradientMatrix;
		float cx = -1.0f, cy = -1.0f, radius = -1.0f, fx = -1.0f, fy = -1.0f, fr = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExFillTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx fill type get radial") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT, "ShapeEx fill type get radial value") ) return 0;
		if ( !check(xgeShapeExFillRadialGradientGet(shape, &cx, &cy, &radius, &fx, &fy, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx fill radial gradient get") ) return 0;
		if ( !check((cx == 0.5f) && (cy == 0.5f) && (radius == 0.5f) && (fx == 0.45f) && (fy == 0.45f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx fill radial gradient get values") ) return 0;
		if ( !check(xgeShapeExFillRadialGradientGetEx(shape, NULL, NULL, NULL, NULL, NULL, &fr, NULL, NULL, NULL) == XGE_OK, "ShapeEx fill radial gradient get ex default") ) return 0;
		if ( !check(fr == 0.0f, "ShapeEx fill radial gradient default focal radius") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 2) && (gotStops[0].iColor == stops[0].iColor) && (gotStops[1].iColor == stops[1].iColor), "ShapeEx fill radial gradient get stops") ) return 0;
		if ( !check(xgeShapeExFillLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx fill linear gradient get rejects radial") ) return 0;
		if ( !check(xgeShapeExFillGradientSpreadGet(shape, &gotInt) == XGE_OK, "ShapeEx fill radial gradient spread get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_GRADIENT_SPREAD_PAD, "ShapeEx fill radial gradient spread reset") ) return 0;
		if ( !check(xgeShapeExFillGradientTransformGet(shape, &gotGradientMatrix) == XGE_OK, "ShapeEx fill radial gradient transform get") ) return 0;
		if ( !check((gotGradientMatrix.fA == 1.0f) && (gotGradientMatrix.fD == 1.0f) && (gotGradientMatrix.fE == 0.0f), "ShapeEx fill radial gradient transform reset") ) return 0;
	}
	if ( !check(xgeShapeExFillRadialGradientEx(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, 0.12f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx radial gradient focal radius") ) return 0;
	{
		float fr = -1.0f;

		if ( !check(xgeShapeExFillRadialGradientGetEx(shape, NULL, NULL, NULL, NULL, NULL, &fr, NULL, NULL, NULL) == XGE_OK, "ShapeEx fill radial gradient get ex focal radius") ) return 0;
		if ( !check(fr == 0.12f, "ShapeEx fill radial gradient focal radius value") ) return 0;
		if ( !check(xgeShapeExFillRadialGradientEx(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, -0.01f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill radial gradient rejects negative focal radius") ) return 0;
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
	if ( !check(xgeShapeExStrokeLinearGradient(shape, invalidInf, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke linear gradient rejects inf coordinate") ) return 0;
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, badStops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke linear gradient rejects nan stop") ) return 0;
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke linear gradient") ) return 0;
	if ( !check(xgeShapeExStrokeGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) == XGE_OK, "ShapeEx stroke gradient spread") ) return 0;
	if ( !check(xgeShapeExStrokeGradientSpread(shape, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke gradient rejects invalid spread") ) return 0;
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, 99, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke linear gradient rejects invalid units") ) return 0;
	if ( !check(xgeShapeExStrokeGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx stroke gradient transform") ) return 0;
	{
		xge_shape_ex_matrix_t badMatrix = matrix;

		badMatrix.fF = invalidInf;
		if ( !check(xgeShapeExStrokeGradientTransformSet(shape, &badMatrix) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke gradient transform rejects inf") ) return 0;
	}
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
		            (gotStops[0].fOffset == -0.25f) && (gotStops[0].iColor == unorderedStops[0].iColor) &&
		            (gotStops[1].fOffset == 0.6f) && (gotStops[1].iColor == unorderedStops[1].iColor) &&
		            (gotStops[2].fOffset == 0.2f) && (gotStops[2].iColor == unorderedStops[2].iColor) &&
		            (gotStops[3].fOffset == 1.5f) && (gotStops[3].iColor == unorderedStops[3].iColor), "ShapeEx stroke gradient preserves source stops") ) return 0;
	}
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient") ) return 0;
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, 99, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke radial gradient rejects invalid units") ) return 0;
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, invalidNaN, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke radial gradient rejects nan radius") ) return 0;
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.0f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient accepts zero radius") ) return 0;
	{
		float radius = -1.0f;

		if ( !check(xgeShapeExStrokeRadialGradientGet(shape, NULL, NULL, &radius, NULL, NULL, NULL, NULL, NULL) == XGE_OK, "ShapeEx stroke zero-radius gradient get") ) return 0;
		if ( !check(radius == 0.0f, "ShapeEx stroke zero-radius gradient value") ) return 0;
	}
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient restore") ) return 0;
	{
		const xge_shape_ex_color_stop_t* gotStops = NULL;
		float cx = -1.0f, cy = -1.0f, radius = -1.0f, fx = -1.0f, fy = -1.0f, fr = -1.0f;
		int gotCount = 0;
		int gotInt = 0;

		if ( !check(xgeShapeExStrokeTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke type get radial") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT, "ShapeEx stroke type get radial value") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientGet(shape, &cx, &cy, &radius, &fx, &fy, &gotInt, &gotStops, &gotCount) == XGE_OK, "ShapeEx stroke radial gradient get") ) return 0;
		if ( !check((cx == 0.5f) && (cy == 0.5f) && (radius == 0.5f) && (fx == 0.45f) && (fy == 0.45f) && (gotInt == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX), "ShapeEx stroke radial gradient get values") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientGetEx(shape, NULL, NULL, NULL, NULL, NULL, &fr, NULL, NULL, NULL) == XGE_OK, "ShapeEx stroke radial gradient get ex default") ) return 0;
		if ( !check(fr == 0.0f, "ShapeEx stroke radial gradient default focal radius") ) return 0;
		if ( !check((gotStops != NULL) && (gotCount == 2), "ShapeEx stroke radial gradient get stops") ) return 0;
		if ( !check(xgeShapeExStrokeLinearGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx stroke linear gradient get rejects radial") ) return 0;
	}
	if ( !check(xgeShapeExStrokeRadialGradientEx(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, 0.14f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient focal radius") ) return 0;
	{
		float fr = -1.0f;

		if ( !check(xgeShapeExStrokeRadialGradientGetEx(shape, NULL, NULL, NULL, NULL, NULL, &fr, NULL, NULL, NULL) == XGE_OK, "ShapeEx stroke radial gradient get ex focal radius") ) return 0;
		if ( !check(fr == 0.14f, "ShapeEx stroke radial gradient focal radius value") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientEx(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, -0.01f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke radial gradient rejects negative focal radius") ) return 0;
	}
	if ( !check(xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(5, 6, 7, 8)) == XGE_OK, "ShapeEx stroke color reset") ) return 0;
	{
		int gotInt = -1;

		if ( !check(xgeShapeExStrokeTypeGet(shape, &gotInt) == XGE_OK, "ShapeEx stroke type get solid") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_PAINT_SOLID, "ShapeEx stroke type get solid value") ) return 0;
		if ( !check(xgeShapeExStrokeRadialGradientGet(shape, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == XGE_ERROR_NOT_FOUND, "ShapeEx stroke radial gradient get rejects solid") ) return 0;
		if ( !check(xgeShapeExStrokeTypeGet(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke type get rejects null") ) return 0;
	}
	{
		xge_shape_ex strokeDefaults = NULL;
		float gotWidth = -1.0f;
		float gotMiter = -1.0f;
		int gotEnum = -1;

		ret = xgeShapeExCreate(&strokeDefaults);
		if ( !check((ret == XGE_OK) && (strokeDefaults != NULL), "ShapeEx ThorVG stroke defaults create") ) return 0;
		if ( !check(xgeShapeExStrokeWidthGet(strokeDefaults, &gotWidth) == XGE_OK, "ShapeEx ThorVG default stroke width get") ||
		     !check(gotWidth == 0.0f, "ShapeEx ThorVG default stroke width") ||
		     !check(xgeShapeExStrokeCapGet(strokeDefaults, &gotEnum) == XGE_OK, "ShapeEx ThorVG default stroke cap get") ||
		     !check(gotEnum == XGE_SHAPE_EX_CAP_SQUARE, "ShapeEx ThorVG default stroke cap square") ||
		     !check(xgeShapeExStrokeJoinGet(strokeDefaults, &gotEnum) == XGE_OK, "ShapeEx ThorVG default stroke join get") ||
		     !check(gotEnum == XGE_SHAPE_EX_JOIN_BEVEL, "ShapeEx ThorVG default stroke join bevel") ||
		     !check(xgeShapeExStrokeMiterLimitGet(strokeDefaults, &gotMiter) == XGE_OK, "ShapeEx ThorVG default stroke miter get") ||
		     !check((gotMiter > 3.999f) && (gotMiter < 4.001f), "ShapeEx ThorVG default stroke miter") ) {
			xgeShapeExDestroy(strokeDefaults);
			return 0;
		}
		if ( !check(xgeShapeExStrokeWidth(strokeDefaults, 0.0f) == XGE_OK, "ShapeEx ThorVG zero stroke width") ||
		     !check(xgeShapeExStrokeWidthGet(strokeDefaults, &gotWidth) == XGE_OK, "ShapeEx ThorVG zero stroke width get") ||
		     !check(gotWidth == 0.0f, "ShapeEx ThorVG zero stroke width value") ||
		     !check(xgeShapeExStrokeMiterLimit(strokeDefaults, 0.00001f) == XGE_OK, "ShapeEx ThorVG tiny stroke miter") ||
		     !check(xgeShapeExStrokeMiterLimitGet(strokeDefaults, &gotMiter) == XGE_OK, "ShapeEx ThorVG tiny stroke miter get") ||
		     !check((gotMiter > 0.000009f) && (gotMiter < 0.000011f), "ShapeEx ThorVG tiny stroke miter value") ||
		     !check(xgeShapeExStrokeMiterLimit(strokeDefaults, 1000.0f) == XGE_OK, "ShapeEx ThorVG large stroke miter") ||
		     !check(xgeShapeExStrokeMiterLimitGet(strokeDefaults, &gotMiter) == XGE_OK, "ShapeEx ThorVG large stroke miter get") ||
		     !check((gotMiter > 999.9f) && (gotMiter < 1000.1f), "ShapeEx ThorVG large stroke miter value") ) {
			xgeShapeExDestroy(strokeDefaults);
			return 0;
		}
		xgeShapeExDestroy(strokeDefaults);
	}
	if ( !check(xgeShapeExStrokeWidth(shape, 2.0f) == XGE_OK, "ShapeEx stroke width") ) return 0;
	if ( !check(xgeShapeExStrokeWidth(shape, -2.0f) == XGE_OK, "ShapeEx negative stroke width clamps") ) return 0;
	{
		float gotWidth = -1.0f;

		if ( !check(xgeShapeExStrokeWidthGet(shape, &gotWidth) == XGE_OK, "ShapeEx negative stroke width clamp get") ) return 0;
		if ( !check(gotWidth == 0.0f, "ShapeEx negative stroke width clamp value") ) return 0;
	}
	if ( !check(xgeShapeExStrokeWidth(shape, 2.0f) == XGE_OK, "ShapeEx stroke width restore") ) return 0;
	if ( !check(xgeShapeExStrokeWidth(shape, invalidNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke width rejects nan") ) return 0;
	if ( !check(xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx stroke cap") ) return 0;
	if ( !check(xgeShapeExStrokeCap(shape, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke cap rejects invalid enum") ) return 0;
	if ( !check(xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND) == XGE_OK, "ShapeEx stroke join") ) return 0;
	if ( !check(xgeShapeExStrokeJoin(shape, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke join rejects invalid enum") ) return 0;
	if ( !check(xgeShapeExStrokeMiterLimit(shape, -1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke miter rejects negative") ) return 0;
	if ( !check(xgeShapeExStrokeMiterLimit(shape, invalidInf) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke miter rejects inf") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 2, 1.0f) == XGE_OK, "ShapeEx stroke dash") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 0, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke dash rejects pattern with zero count") ) return 0;
	dashBad[0] = invalidNaN;
	if ( !check(xgeShapeExStrokeDash(shape, dashBad, 2, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke dash rejects nan pattern") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 2, invalidInf) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx stroke dash rejects inf offset") ) return 0;
	if ( !check(xgeShapeExStrokeNonScaling(shape, 1) == XGE_OK, "ShapeEx non-scaling stroke") ) return 0;
	if ( !check(xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx fill rule") ) return 0;
	if ( !check(xgeShapeExFillRule(shape, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx fill rule rejects invalid enum") ) return 0;
	if ( !check(xgeShapeExPaintOrder(shape, 1) == XGE_OK, "ShapeEx paint order") ) return 0;
	if ( !check(xgeShapeExOpacity(shape, 0.75f) == XGE_OK, "ShapeEx opacity") ) return 0;
	{
		const float* gotDash = NULL;
		uint32_t gotColor = 0;
		float gotFloat = 0.0f;
		float gotDashOffset = 0.0f;
		int gotInt = 0;
		int gotDashCount = 0;
		int gotBlendSet = 0;

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
		if ( !check((gotDash != NULL) && (gotDashCount == 3) &&
		            (gotDash[0] == 4.0f) && (gotDash[1] == 0.0f) && (gotDash[2] == 1.0f) &&
		            (gotDashOffset == -1.0f), "ShapeEx odd stroke dash stores original count and clamps negatives") ) return 0;
		if ( !check(xgeShapeExStrokeDash(shape, dash, 2, 1.0f) == XGE_OK, "ShapeEx stroke dash restore") ) return 0;
		if ( !check(xgeShapeExFillRuleGet(shape, &gotInt) == XGE_OK, "ShapeEx fill rule get") ) return 0;
		if ( !check(gotInt == XGE_SHAPE_EX_FILL_EVEN_ODD, "ShapeEx fill rule get value") ) return 0;
		if ( !check(xgeShapeExPaintOrderGet(shape, &gotInt) == XGE_OK, "ShapeEx paint order get") ) return 0;
		if ( !check(gotInt == 1, "ShapeEx paint order get value") ) return 0;
		if ( !check(xgeShapeExOpacityGet(shape, &gotFloat) == XGE_OK, "ShapeEx opacity get") ) return 0;
		if ( !check((gotFloat > 0.749f) && (gotFloat < 0.751f), "ShapeEx opacity get value") ) return 0;
		if ( !check(xgeShapeExOpacity(shape, invalidNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx opacity rejects nan") ) return 0;
		if ( !check(xgeShapeExOpacityGet(shape, &gotFloat) == XGE_OK, "ShapeEx opacity after nan get") ) return 0;
		if ( !check((gotFloat > 0.749f) && (gotFloat < 0.751f), "ShapeEx opacity rejects nan preserves value") ) return 0;
		if ( !check(xgeShapeExOpacity(shape, 2.0f) == XGE_OK, "ShapeEx opacity clamps high") ) return 0;
		if ( !check(xgeShapeExOpacityGet(shape, &gotFloat) == XGE_OK, "ShapeEx opacity high get") ) return 0;
		if ( !check((gotFloat > 0.999f) && (gotFloat < 1.001f), "ShapeEx opacity high clamp value") ) return 0;
		if ( !check(xgeShapeExOpacity(shape, -1.0f) == XGE_OK, "ShapeEx opacity clamps low") ) return 0;
		if ( !check(xgeShapeExOpacityGet(shape, &gotFloat) == XGE_OK, "ShapeEx opacity low get") ) return 0;
		if ( !check((gotFloat > -0.001f) && (gotFloat < 0.001f), "ShapeEx opacity low clamp value") ) return 0;
		if ( !check(xgeShapeExOpacity(shape, 0.75f) == XGE_OK, "ShapeEx opacity restore") ) return 0;
		if ( !check(xgeShapeExVisibleGet(shape, &gotInt) == XGE_OK, "ShapeEx visible get") ) return 0;
		if ( !check(gotInt == 1, "ShapeEx visible get default value") ) return 0;
		if ( !check(xgeShapeExVisible(shape, 0) == XGE_OK, "ShapeEx visible false") ) return 0;
		if ( !check(xgeShapeExVisibleGet(shape, &gotInt) == XGE_OK, "ShapeEx visible false get") ) return 0;
		if ( !check(gotInt == 0, "ShapeEx visible false get value") ) return 0;
		if ( !check(xgeShapeExVisible(shape, 1) == XGE_OK, "ShapeEx visible true restore") ) return 0;
		if ( !check(xgeShapeExBlendGet(shape, &gotInt, &gotBlendSet) == XGE_OK, "ShapeEx blend get default") ) return 0;
		if ( !check((gotInt == XGE_BLEND_ALPHA) && (gotBlendSet == 0), "ShapeEx blend default value") ) return 0;
		if ( !check(xgeShapeExBlend(shape, XGE_BLEND_MULTIPLY) == XGE_OK, "ShapeEx blend set") ) return 0;
		if ( !check(xgeShapeExBlendGet(shape, &gotInt, &gotBlendSet) == XGE_OK, "ShapeEx blend get set") ) return 0;
		if ( !check((gotInt == XGE_BLEND_MULTIPLY) && (gotBlendSet == 1), "ShapeEx blend set value") ) return 0;
		if ( !check(xgeShapeExBlend(shape, XGE_BLEND_OVERLAY) == XGE_OK, "ShapeEx overlay blend set") ) return 0;
		if ( !check(xgeShapeExBlend(shape, XGE_BLEND_LUMINOSITY) == XGE_OK, "ShapeEx luminosity blend set") ) return 0;
		if ( !check(xgeShapeExBlendGet(shape, &gotInt, &gotBlendSet) == XGE_OK, "ShapeEx extended blend get") ) return 0;
		if ( !check((gotInt == XGE_BLEND_LUMINOSITY) && (gotBlendSet == 1), "ShapeEx extended blend value") ) return 0;
		if ( !check(xgeShapeExBlend(shape, 999) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx blend rejects invalid enum") ) return 0;
		if ( !check(xgeShapeExBlendGet(shape, &gotInt, &gotBlendSet) == XGE_OK, "ShapeEx blend after invalid get") ) return 0;
		if ( !check((gotInt == XGE_BLEND_LUMINOSITY) && (gotBlendSet == 1), "ShapeEx blend invalid preserves value") ) return 0;
		if ( !check(xgeShapeExBlendClear(shape) == XGE_OK, "ShapeEx blend clear") ) return 0;
		if ( !check(xgeShapeExBlendGet(shape, &gotInt, &gotBlendSet) == XGE_OK, "ShapeEx blend get cleared") ) return 0;
		if ( !check((gotInt == XGE_BLEND_ALPHA) && (gotBlendSet == 0), "ShapeEx blend clear value") ) return 0;
		if ( !check(xgeShapeExBlendGet(NULL, &gotInt, &gotBlendSet) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx blend get rejects null shape") ) return 0;
		if ( !check(xgeShapeExBlendClear(NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx blend clear rejects null shape") ) return 0;
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
	if ( !check(xgeShapeExClipRectSet(shape, (xge_rect_t){1.0f, invalidNaN, 8.0f, 9.0f}) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip rect rejects nan") ) return 0;
	{
		xge_rect_t gotClipRect;
		int gotClipEnabled = -1;

		if ( !check(xgeShapeExClipRectGet(shape, &gotClipRect, &gotClipEnabled) == XGE_OK, "ShapeEx clip rect get") ) return 0;
		if ( !check(gotClipEnabled && (gotClipRect.fX == 1.0f) && (gotClipRect.fY == 2.0f) && (gotClipRect.fW == 8.0f) && (gotClipRect.fH == 9.0f), "ShapeEx clip rect get value") ) return 0;
		if ( !check(xgeShapeExClipRectGet(shape, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip rect get rejects no outputs") ) return 0;
		if ( !check(xgeShapeExClipRectGet(NULL, &gotClipRect, &gotClipEnabled) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip rect get rejects null shape") ) return 0;
	}
	if ( !check(xgeShapeExClipClear(shape) == XGE_OK, "ShapeEx clip clear") ) return 0;
	{
		xge_rect_t gotClipRect;
		int gotClipEnabled = -1;

		if ( !check(xgeShapeExClipRectGet(shape, &gotClipRect, &gotClipEnabled) == XGE_OK, "ShapeEx clip rect get cleared") ) return 0;
		if ( !check((gotClipEnabled == 0) && (gotClipRect.fX == 0.0f) && (gotClipRect.fY == 0.0f) && (gotClipRect.fW == 0.0f) && (gotClipRect.fH == 0.0f), "ShapeEx clip rect cleared value") ) return 0;
	}
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
		xge_shape_ex gotClipShape = NULL;
		xge_rect_t clonedClipBounds;
		int gotClipCount = -1;
		int gotClipMode = -1;

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
		if ( !check(xgeShapeExClipShapeGetCount(shape, &gotClipCount) == XGE_OK, "ShapeEx clip shape count get") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(gotClipCount == 1, "ShapeEx clip shape count get value") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAt(shape, 0, &gotClipShape) == XGE_OK, "ShapeEx clip shape get at") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(gotClipShape == clip, "ShapeEx clip shape get at value") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAtEx(shape, 0, &gotClipShape, &gotClipMode) == XGE_OK, "ShapeEx clip shape get at ex") ||
		     !check((gotClipShape == clip) && (gotClipMode == XGE_SHAPE_EX_CLIP_INTERSECT), "ShapeEx clip shape get at ex value") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAt(shape, 1, &gotClipShape) == XGE_ERROR_NOT_FOUND, "ShapeEx clip shape get at missing") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(gotClipShape == NULL, "ShapeEx clip shape get at missing clears output") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAtEx(shape, 1, &gotClipShape, &gotClipMode) == XGE_ERROR_NOT_FOUND, "ShapeEx clip shape get at ex missing") ||
		     !check((gotClipShape == NULL) && (gotClipMode == XGE_SHAPE_EX_CLIP_INTERSECT), "ShapeEx clip shape get at ex missing clears outputs") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetCount(NULL, &gotClipCount) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip shape count rejects null shape") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAt(shape, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip shape get at rejects null output") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetAtEx(shape, 0, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip shape get at ex rejects no outputs") ||
		     !check(xgeShapeExClipShapeAddEx(shape, clip, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip shape add ex rejects invalid mode") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		ret = xgeShapeExClone(shape, &clippedClone);
		if ( !check((ret == XGE_OK) && (clippedClone != NULL), "ShapeEx clone with clip shape") ) {
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetCount(clippedClone, &gotClipCount) == XGE_OK, "ShapeEx clone clip shape count get") ||
		     !check(gotClipCount == 1, "ShapeEx clone clip shape count value") ||
		     !check(xgeShapeExClipShapeGetAtEx(clippedClone, 0, &gotClipShape, &gotClipMode) == XGE_OK, "ShapeEx clone clip shape get at ex") ||
		     !check(gotClipMode == XGE_SHAPE_EX_CLIP_INTERSECT, "ShapeEx clone clip shape mode value") ||
		     !check(gotClipShape != clip, "ShapeEx clone clip shape is independent") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExReset(clip) == XGE_OK, "ShapeEx source clip reset after clone") ||
		     !check(xgeShapeExAppendRect(clip, 20.0f, 20.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx source clip mutate after clone") ||
		     !check(xgeShapeExGetBounds(gotClipShape, 0.05f, &clonedClipBounds) == XGE_OK, "ShapeEx cloned clip bounds after source mutate") ||
		     !check((clonedClipBounds.fX > -0.1f) && (clonedClipBounds.fX < 0.1f) &&
		            (clonedClipBounds.fY > -0.1f) && (clonedClipBounds.fY < 0.1f) &&
		            (clonedClipBounds.fW > 4.9f) && (clonedClipBounds.fW < 5.1f) &&
		            (clonedClipBounds.fH > 4.9f) && (clonedClipBounds.fH < 5.1f), "ShapeEx cloned clip independent bounds") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeClear(shape) == XGE_OK, "ShapeEx clip shape clear") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeGetCount(shape, &gotClipCount) == XGE_OK, "ShapeEx clip shape count get cleared") ||
		     !check(gotClipCount == 0, "ShapeEx clip shape cleared count value") ) {
			xgeShapeExDestroy(clippedClone);
			xgeShapeExDestroy(clip);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeAddEx(shape, clip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx subtract clip shape add ex") ||
		     !check(xgeShapeExClipShapeGetAtEx(shape, 0, &gotClipShape, &gotClipMode) == XGE_OK, "ShapeEx subtract clip shape get at ex") ||
		     !check((gotClipShape == clip) && (gotClipMode == XGE_SHAPE_EX_CLIP_SUBTRACT), "ShapeEx subtract clip shape get at ex value") ||
		     !check(xgeShapeExClipShapeClear(shape) == XGE_OK, "ShapeEx subtract clip shape clear") ) {
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
	{
		xge_shape_ex clipCycleA = NULL;
		xge_shape_ex clipCycleB = NULL;
		xge_shape_ex clipCycleClone = NULL;
		int clipCycleCloneCount = -1;

		ret = xgeShapeExCreate(&clipCycleA);
		if ( !check((ret == XGE_OK) && (clipCycleA != NULL), "ShapeEx clip cycle A create") ) return 0;
		ret = xgeShapeExCreate(&clipCycleB);
		if ( !check((ret == XGE_OK) && (clipCycleB != NULL), "ShapeEx clip cycle B create") ) {
			xgeShapeExDestroy(clipCycleA);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(clipCycleA, 0.0f, 0.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip cycle A path") ||
		     !check(xgeShapeExAppendRect(clipCycleB, 1.0f, 1.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip cycle B path") ||
		     !check(xgeShapeExClipShapeAdd(clipCycleA, clipCycleB) == XGE_OK, "ShapeEx clip cycle A add") ||
		     !check(xgeShapeExClipShapeAdd(clipCycleB, clipCycleA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx clip cycle reverse add rejected") ) {
			xgeShapeExClipShapeClear(clipCycleA);
			xgeShapeExClipShapeClear(clipCycleB);
			xgeShapeExDestroy(clipCycleB);
			xgeShapeExDestroy(clipCycleA);
			return 0;
		}
		ret = xgeShapeExClone(clipCycleA, &clipCycleClone);
		if ( !check((ret == XGE_OK) && (clipCycleClone != NULL), "ShapeEx clone nested clip after cycle rejection") ||
		     !check(xgeShapeExClipShapeGetCount(clipCycleClone, &clipCycleCloneCount) == XGE_OK, "ShapeEx clone nested clip count get") ||
		     !check(clipCycleCloneCount == 1, "ShapeEx clone nested clip count value") ) {
			xgeShapeExDestroy(clipCycleClone);
			xgeShapeExClipShapeClear(clipCycleA);
			xgeShapeExClipShapeClear(clipCycleB);
			xgeShapeExDestroy(clipCycleB);
			xgeShapeExDestroy(clipCycleA);
			return 0;
		}
		xgeShapeExDestroy(clipCycleClone);
		xgeShapeExClipShapeClear(clipCycleA);
		xgeShapeExClipShapeClear(clipCycleB);
		xgeShapeExDestroy(clipCycleB);
		xgeShapeExDestroy(clipCycleA);
	}
	matrix.fA = 1.0f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 8.0f;
	matrix.fF = 9.0f;
	if ( !check(xgeShapeExTransformSet(shape, &matrix) == XGE_OK, "ShapeEx transform") ) return 0;
	{
		xge_shape_ex_matrix_t badMatrix = matrix;

		badMatrix.fA = invalidNaN;
		if ( !check(xgeShapeExTransformSet(shape, &badMatrix) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx transform set rejects nan") ) return 0;
	}
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
		volatile float zero = 0.0f;
		float badNaN;

		badNaN = 0.0f / zero;
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
		if ( !check(xgeShapeExTransformIdentity(transformShape) == XGE_OK, "ShapeEx transform skew reset") ||
		     !check(xgeShapeExTransformSkew(transformShape, 0.4636476090008061f, 0.0f) == XGE_OK, "ShapeEx transform skew") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformGet(transformShape, &gotTransform) == XGE_OK, "ShapeEx transform skew get") ||
		     !check((gotTransform.fC > 0.499f) && (gotTransform.fC < 0.501f), "ShapeEx transform skew matrix") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(transformShape, 0.05f, &transformBounds);
		if ( !check((ret == XGE_OK) &&
		            (transformBounds.fX > -0.01f) && (transformBounds.fX < 0.01f) &&
		            (transformBounds.fY > -0.01f) && (transformBounds.fY < 0.01f) &&
		            (transformBounds.fW > 4.99f) && (transformBounds.fW < 5.01f) &&
		            (transformBounds.fH > 1.99f) && (transformBounds.fH < 2.01f), "ShapeEx transform skew bounds") ||
		     !check(xgeShapeExTransformSkew(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx transform skew rejects null") ||
		     !check(xgeShapeExTransformSkew(transformShape, badNaN, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx transform skew rejects nan") ) {
			xgeShapeExDestroy(transformShape);
			return 0;
		}
		xgeShapeExDestroy(transformShape);
	}
	{
		xge_shape_ex thorvgTransformShape = NULL;
		xge_shape_ex thorvgTransformClone = NULL;
		xge_shape_ex_scene thorvgTransformScene = NULL;
		xge_shape_ex_scene thorvgTransformSceneClone = NULL;
		xge_shape_ex_matrix_t gotTransform;
		xge_shape_ex_matrix_t customTransform;
		const float quarterPi = 0.78539816339744830962f;
		int transformOk = 1;

		transformOk = transformOk && check(xgeShapeExCreate(&thorvgTransformShape) == XGE_OK, "ShapeEx ThorVG transform state shape create");
		if ( transformOk ) {
			transformOk = transformOk && check(xgeShapeExTransformTranslate(thorvgTransformShape, 1.0f, 2.0f) == XGE_OK, "ShapeEx ThorVG transform initial translate");
			transformOk = transformOk && check(xgeShapeExTransformScale(thorvgTransformShape, 2.0f, 2.0f) == XGE_OK, "ShapeEx ThorVG transform initial scale");
			transformOk = transformOk && check(xgeShapeExTransformRotate(thorvgTransformShape, 0.25f) == XGE_OK, "ShapeEx ThorVG transform initial rotate");
			transformOk = transformOk && check(xgeShapeExTransformTranslate(thorvgTransformShape, 155.0f, -155.0f) == XGE_OK, "ShapeEx ThorVG transform replace translate");
			transformOk = transformOk && check(xgeShapeExTransformRotate(thorvgTransformShape, quarterPi) == XGE_OK, "ShapeEx ThorVG transform replace rotate");
			transformOk = transformOk && check(xgeShapeExTransformScale(thorvgTransformShape, 4.7f, 4.7f) == XGE_OK, "ShapeEx ThorVG transform replace scale");
			transformOk = transformOk && check(xgeShapeExTransformGet(thorvgTransformShape, &gotTransform) == XGE_OK, "ShapeEx ThorVG transform state get");
			transformOk = transformOk && check(
				(gotTransform.fA > 3.32339f) && (gotTransform.fA < 3.32342f) &&
				(gotTransform.fB > 3.32339f) && (gotTransform.fB < 3.32342f) &&
				(gotTransform.fC < -3.32339f) && (gotTransform.fC > -3.32342f) &&
				(gotTransform.fD > 3.32339f) && (gotTransform.fD < 3.32342f) &&
				(gotTransform.fE == 155.0f) && (gotTransform.fF == -155.0f),
				"ShapeEx ThorVG transform state matrix"
			);
		}
		customTransform.fA = 1.0f;
		customTransform.fB = 2.0f;
		customTransform.fC = 3.0f;
		customTransform.fD = 4.0f;
		customTransform.fE = 5.0f;
		customTransform.fF = 6.0f;
		if ( transformOk ) {
			transformOk = transformOk && check(xgeShapeExTransformSet(thorvgTransformShape, &customTransform) == XGE_OK, "ShapeEx ThorVG custom transform set");
			transformOk = transformOk && check(xgeShapeExTransformTranslate(thorvgTransformShape, 9.0f, 10.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx ThorVG custom transform locks translate");
			transformOk = transformOk && check(xgeShapeExTransformScale(thorvgTransformShape, 3.0f, 3.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx ThorVG custom transform locks scale");
			transformOk = transformOk && check(xgeShapeExTransformRotate(thorvgTransformShape, quarterPi) == XGE_ERROR_INVALID_STATE, "ShapeEx ThorVG custom transform locks rotate");
			transformOk = transformOk && check(xgeShapeExTransformSkew(thorvgTransformShape, 0.1f, 0.2f) == XGE_ERROR_INVALID_STATE, "ShapeEx custom transform locks skew extension");
			transformOk = transformOk && check(xgeShapeExReset(thorvgTransformShape) == XGE_OK, "ShapeEx reset preserves custom transform state");
			transformOk = transformOk && check(xgeShapeExTransformTranslate(thorvgTransformShape, 9.0f, 10.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx reset keeps custom transform lock");
			transformOk = transformOk && check(xgeShapeExTransformGet(thorvgTransformShape, &gotTransform) == XGE_OK, "ShapeEx custom transform unchanged get");
			transformOk = transformOk && check(
				(gotTransform.fA == customTransform.fA) && (gotTransform.fB == customTransform.fB) &&
				(gotTransform.fC == customTransform.fC) && (gotTransform.fD == customTransform.fD) &&
				(gotTransform.fE == customTransform.fE) && (gotTransform.fF == customTransform.fF),
				"ShapeEx custom transform unchanged"
			);
			transformOk = transformOk && check(xgeShapeExClone(thorvgTransformShape, &thorvgTransformClone) == XGE_OK, "ShapeEx custom transform clone");
			transformOk = transformOk && check(xgeShapeExTransformRotate(thorvgTransformClone, 0.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx clone preserves custom transform lock");
			transformOk = transformOk && check(xgeShapeExTransformIdentity(thorvgTransformClone) == XGE_OK, "ShapeEx clone custom transform identity");
			transformOk = transformOk && check(xgeShapeExTransformTranslate(thorvgTransformClone, 7.0f, 8.0f) == XGE_OK, "ShapeEx identity unlocks convenience transform");
		}
		if ( transformOk ) {
			transformOk = transformOk && check(xgeShapeExSceneCreate(&thorvgTransformScene) == XGE_OK, "ShapeEx ThorVG transform state scene create");
			transformOk = transformOk && check(xgeShapeExSceneTransformScale(thorvgTransformScene, 2.0f, 3.0f) == XGE_OK, "ShapeEx scene transform initial scale");
			transformOk = transformOk && check(xgeShapeExSceneTransformTranslate(thorvgTransformScene, 10.0f, 20.0f) == XGE_OK, "ShapeEx scene transform initial translate");
			transformOk = transformOk && check(xgeShapeExSceneTransformTranslate(thorvgTransformScene, 30.0f, 40.0f) == XGE_OK, "ShapeEx scene transform replace translate");
			transformOk = transformOk && check(xgeShapeExSceneTransformGet(thorvgTransformScene, &gotTransform) == XGE_OK, "ShapeEx scene transform state get");
			transformOk = transformOk && check(
				(gotTransform.fA == 2.0f) && (gotTransform.fD == 3.0f) &&
				(gotTransform.fE == 30.0f) && (gotTransform.fF == 40.0f),
				"ShapeEx scene transform replaces state"
			);
			transformOk = transformOk && check(xgeShapeExSceneTransformSet(thorvgTransformScene, &customTransform) == XGE_OK, "ShapeEx scene custom transform set");
			transformOk = transformOk && check(xgeShapeExSceneTransformScale(thorvgTransformScene, 1.0f, 1.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx scene custom transform locks convenience transform");
			transformOk = transformOk && check(xgeShapeExSceneClone(thorvgTransformScene, &thorvgTransformSceneClone) == XGE_OK, "ShapeEx scene custom transform clone");
			transformOk = transformOk && check(xgeShapeExSceneTransformTranslate(thorvgTransformSceneClone, 0.0f, 0.0f) == XGE_ERROR_INVALID_STATE, "ShapeEx scene clone preserves custom transform lock");
			transformOk = transformOk && check(xgeShapeExSceneTransformIdentity(thorvgTransformSceneClone) == XGE_OK, "ShapeEx scene clone custom transform identity");
			transformOk = transformOk && check(xgeShapeExSceneTransformRotate(thorvgTransformSceneClone, quarterPi) == XGE_OK, "ShapeEx scene identity unlocks convenience transform");
		}
		xgeShapeExSceneDestroy(thorvgTransformSceneClone);
		xgeShapeExSceneDestroy(thorvgTransformScene);
		xgeShapeExDestroy(thorvgTransformClone);
		xgeShapeExDestroy(thorvgTransformShape);
		if ( !transformOk ) return 0;
	}
	if ( !check(xgeShapeExDrawEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw ex rejects null shape") ) return 0;
	if ( !check(xgeShapeExDrawPxEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw px ex rejects null shape") ) return 0;
	if ( !check(xgeShapeExDraw(NULL, 0.25f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx world draw rejects null shape") ) return 0;
	{
		xge_shape_ex_matrix_t badMatrix = matrix;

		badMatrix.fC = invalidNaN;
		if ( !check(xgeShapeExDrawEx(shape, 0.25f, &badMatrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw ex rejects invalid parent matrix") ||
		     !check(xgeShapeExDrawPxEx(shape, 0.25f, &matrix, invalidNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw px ex rejects invalid parent opacity") ||
		     !check(xgeShapeExDrawEx(shape, invalidNaN, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx draw ex rejects invalid tolerance") ) return 0;
	}
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
		int gotSceneBlend = 0;
		int gotSceneBlendSet = 0;

		if ( !check(xgeShapeExSceneGetCount(scene, &gotCount) == XGE_OK, "ShapeEx scene count get") ) return 0;
		if ( !check(gotCount == 1, "ShapeEx scene count get value") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 0, &sceneItem) == XGE_OK, "ShapeEx scene get at") ) return 0;
		if ( !check(sceneItem == shape, "ShapeEx scene get at value") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 1, &sceneItem) == XGE_ERROR_NOT_FOUND, "ShapeEx scene get at missing") ) return 0;
		if ( !check(sceneItem == NULL, "ShapeEx scene get at missing clears output") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, &gotOpacity) == XGE_OK, "ShapeEx scene opacity get") ) return 0;
		if ( !check((gotOpacity > 0.499f) && (gotOpacity < 0.501f), "ShapeEx scene opacity get value") ) return 0;
		if ( !check(xgeShapeExSceneOpacity(scene, 2.0f) == XGE_OK, "ShapeEx scene opacity clamps high") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, &gotOpacity) == XGE_OK, "ShapeEx scene opacity high get") ) return 0;
		if ( !check((gotOpacity > 0.999f) && (gotOpacity < 1.001f), "ShapeEx scene opacity high clamp value") ) return 0;
		if ( !check(xgeShapeExSceneOpacity(scene, -1.0f) == XGE_OK, "ShapeEx scene opacity clamps low") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, &gotOpacity) == XGE_OK, "ShapeEx scene opacity low get") ) return 0;
		if ( !check((gotOpacity > -0.001f) && (gotOpacity < 0.001f), "ShapeEx scene opacity low clamp value") ) return 0;
		if ( !check(xgeShapeExSceneOpacity(scene, 0.5f) == XGE_OK, "ShapeEx scene opacity restore") ) return 0;
		if ( !check(xgeShapeExSceneOpacity(scene, invalidInf) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene opacity rejects inf") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, &gotOpacity) == XGE_OK, "ShapeEx scene opacity after inf get") ) return 0;
		if ( !check((gotOpacity > 0.499f) && (gotOpacity < 0.501f), "ShapeEx scene opacity rejects inf preserves value") ) return 0;
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
		if ( !check(xgeShapeExSceneTransformIdentity(scene) == XGE_OK, "ShapeEx scene transform skew reset") ) return 0;
		if ( !check(xgeShapeExSceneTransformSkew(scene, 0.4636476090008061f, 0.24497866312686414f) == XGE_OK, "ShapeEx scene transform skew") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, &gotSceneTransform) == XGE_OK, "ShapeEx scene transform skew get") ) return 0;
		if ( !check((gotSceneTransform.fB > 0.249f) && (gotSceneTransform.fB < 0.251f) &&
		            (gotSceneTransform.fC > 0.499f) && (gotSceneTransform.fC < 0.501f), "ShapeEx scene transform skew values") ) return 0;
		if ( !check(xgeShapeExSceneTransformIdentity(scene) == XGE_OK, "ShapeEx scene transform restore identity") ) return 0;
		if ( !check(xgeShapeExSceneTransformSet(scene, &matrix) == XGE_OK, "ShapeEx scene transform restore") ) return 0;
		{
			xge_shape_ex_matrix_t badMatrix = matrix;

			badMatrix.fD = invalidNaN;
			if ( !check(xgeShapeExSceneTransformSet(scene, &badMatrix) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform set rejects nan") ) return 0;
		}
		if ( !check(xgeShapeExSceneTransformTranslate(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform translate rejects null") ) return 0;
		if ( !check(xgeShapeExSceneTransformSkew(NULL, 0.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform skew rejects null") ) return 0;
		if ( !check(xgeShapeExSceneVisible(scene, 0) == XGE_OK, "ShapeEx scene visible false") ) return 0;
		if ( !check(xgeShapeExSceneVisibleGet(scene, &gotVisible) == XGE_OK, "ShapeEx scene visible get") ) return 0;
		if ( !check(gotVisible == 0, "ShapeEx scene visible get value") ) return 0;
		if ( !check(xgeShapeExSceneVisible(scene, 1) == XGE_OK, "ShapeEx scene visible restore") ) return 0;
		if ( !check(xgeShapeExSceneBlendGet(scene, &gotSceneBlend, &gotSceneBlendSet) == XGE_OK, "ShapeEx scene blend get default") ) return 0;
		if ( !check((gotSceneBlend == XGE_BLEND_ALPHA) && (gotSceneBlendSet == 0), "ShapeEx scene blend default value") ) return 0;
		if ( !check(xgeShapeExSceneBlend(scene, XGE_BLEND_SCREEN) == XGE_OK, "ShapeEx scene blend set") ) return 0;
		if ( !check(xgeShapeExSceneBlendGet(scene, &gotSceneBlend, &gotSceneBlendSet) == XGE_OK, "ShapeEx scene blend get set") ) return 0;
		if ( !check((gotSceneBlend == XGE_BLEND_SCREEN) && (gotSceneBlendSet == 1), "ShapeEx scene blend set value") ) return 0;
		if ( !check(xgeShapeExSceneBlend(scene, XGE_BLEND_COLOR_DODGE) == XGE_OK, "ShapeEx scene color-dodge blend set") ) return 0;
		if ( !check(xgeShapeExSceneBlend(scene, XGE_BLEND_LUMINOSITY) == XGE_OK, "ShapeEx scene luminosity blend set") ) return 0;
		if ( !check(xgeShapeExSceneBlend(scene, 999) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene blend rejects invalid enum") ) return 0;
		if ( !check(xgeShapeExSceneBlendClear(scene) == XGE_OK, "ShapeEx scene blend clear") ) return 0;
		if ( !check(xgeShapeExSceneBlendGet(scene, &gotSceneBlend, &gotSceneBlendSet) == XGE_OK, "ShapeEx scene blend get cleared") ) return 0;
		if ( !check((gotSceneBlend == XGE_BLEND_ALPHA) && (gotSceneBlendSet == 0), "ShapeEx scene blend clear value") ) return 0;
		if ( !check(xgeShapeExSceneBlendGet(NULL, &gotSceneBlend, &gotSceneBlendSet) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene blend get rejects null scene") ) return 0;
		if ( !check(xgeShapeExSceneBlendClear(NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene blend clear rejects null scene") ) return 0;
		{
			xge_rect_t gotSceneClipRect;
			xge_rect_t clonedSceneClipBounds;
			xge_shape_ex sceneClip = NULL;
			xge_shape_ex gotSceneClip = NULL;
			xge_shape_ex_scene sceneClipClone = NULL;
			int gotSceneClipEnabled = -1;
			int gotSceneClipCount = -1;
			int gotSceneClipMode = -1;

			if ( !check(xgeShapeExSceneClipRectSet(scene, (xge_rect_t){2.0f, 3.0f, 40.0f, 50.0f}) == XGE_OK, "ShapeEx scene clip rect") ) return 0;
			if ( !check(xgeShapeExSceneClipRectGet(scene, &gotSceneClipRect, &gotSceneClipEnabled) == XGE_OK, "ShapeEx scene clip rect get") ) return 0;
			if ( !check(gotSceneClipEnabled && (gotSceneClipRect.fX == 2.0f) && (gotSceneClipRect.fY == 3.0f) && (gotSceneClipRect.fW == 40.0f) && (gotSceneClipRect.fH == 50.0f), "ShapeEx scene clip rect get value") ) return 0;
			if ( !check(xgeShapeExSceneClipRectGet(scene, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip rect get rejects no outputs") ) return 0;
			if ( !check(xgeShapeExSceneClipRectSet(scene, (xge_rect_t){2.0f, invalidNaN, 40.0f, 50.0f}) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip rect rejects nan") ) return 0;
			ret = xgeShapeExCreate(&sceneClip);
			if ( !check((ret == XGE_OK) && (sceneClip != NULL), "ShapeEx scene clip shape create") ) return 0;
			if ( !check(xgeShapeExAppendRect(sceneClip, 0.0f, 0.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene clip shape path") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipShapeAdd(scene, sceneClip) == XGE_OK, "ShapeEx scene clip shape add") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipShapeGetCount(scene, &gotSceneClipCount) == XGE_OK, "ShapeEx scene clip shape count get") ||
			     !check(gotSceneClipCount == 1, "ShapeEx scene clip shape count value") ||
			     !check(xgeShapeExSceneClipShapeGetAt(scene, 0, &gotSceneClip) == XGE_OK, "ShapeEx scene clip shape get at") ||
			     !check(gotSceneClip == sceneClip, "ShapeEx scene clip shape get at value") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipShapeGetAtEx(scene, 0, &gotSceneClip, &gotSceneClipMode) == XGE_OK, "ShapeEx scene clip shape get at ex") ||
			     !check((gotSceneClip == sceneClip) && (gotSceneClipMode == XGE_SHAPE_EX_CLIP_INTERSECT), "ShapeEx scene clip shape get at ex value") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipShapeGetAt(scene, 1, &gotSceneClip) == XGE_ERROR_NOT_FOUND, "ShapeEx scene clip shape get at missing") ||
			     !check(gotSceneClip == NULL, "ShapeEx scene clip shape get at missing clears output") ||
			     !check(xgeShapeExSceneClipShapeGetCount(NULL, &gotSceneClipCount) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip shape count rejects null scene") ||
			     !check(xgeShapeExSceneClipShapeGetAt(scene, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip shape get at rejects null output") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipShapeGetAtEx(scene, 1, &gotSceneClip, &gotSceneClipMode) == XGE_ERROR_NOT_FOUND, "ShapeEx scene clip shape get at ex missing") ||
			     !check((gotSceneClip == NULL) && (gotSceneClipMode == XGE_SHAPE_EX_CLIP_INTERSECT), "ShapeEx scene clip shape get at ex missing clears outputs") ||
			     !check(xgeShapeExSceneClipShapeGetAtEx(scene, 0, NULL, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip shape get at ex rejects no outputs") ||
			     !check(xgeShapeExSceneClipShapeAddEx(scene, sceneClip, 99) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene clip shape add ex rejects invalid mode") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			ret = xgeShapeExSceneClone(scene, &sceneClipClone);
			if ( !check((ret == XGE_OK) && (sceneClipClone != NULL), "ShapeEx scene clone with clip") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExSceneClipRectGet(sceneClipClone, &gotSceneClipRect, &gotSceneClipEnabled) == XGE_OK, "ShapeEx scene clone clip rect get") ||
			     !check(gotSceneClipEnabled && (gotSceneClipRect.fX == 2.0f) && (gotSceneClipRect.fY == 3.0f), "ShapeEx scene clone clip rect value") ||
			     !check(xgeShapeExSceneClipShapeGetCount(sceneClipClone, &gotSceneClipCount) == XGE_OK, "ShapeEx scene clone clip shape count get") ||
			     !check(gotSceneClipCount == 1, "ShapeEx scene clone clip shape count value") ||
			     !check(xgeShapeExSceneClipShapeGetAtEx(sceneClipClone, 0, &gotSceneClip, &gotSceneClipMode) == XGE_OK, "ShapeEx scene clone clip shape get at ex") ||
			     !check(gotSceneClipMode == XGE_SHAPE_EX_CLIP_INTERSECT, "ShapeEx scene clone clip shape mode value") ||
			     !check(gotSceneClip != sceneClip, "ShapeEx scene clone clip shape is independent") ) {
				xgeShapeExSceneDestroy(sceneClipClone);
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			if ( !check(xgeShapeExReset(sceneClip) == XGE_OK, "ShapeEx scene source clip reset after clone") ||
			     !check(xgeShapeExAppendRect(sceneClip, 30.0f, 30.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene source clip mutate after clone") ||
			     !check(xgeShapeExGetBounds(gotSceneClip, 0.05f, &clonedSceneClipBounds) == XGE_OK, "ShapeEx scene cloned clip bounds after source mutate") ||
			     !check((clonedSceneClipBounds.fX > -0.1f) && (clonedSceneClipBounds.fX < 0.1f) &&
			            (clonedSceneClipBounds.fY > -0.1f) && (clonedSceneClipBounds.fY < 0.1f) &&
			            (clonedSceneClipBounds.fW > 7.9f) && (clonedSceneClipBounds.fW < 8.1f) &&
			            (clonedSceneClipBounds.fH > 7.9f) && (clonedSceneClipBounds.fH < 8.1f), "ShapeEx scene cloned clip independent bounds") ) {
				xgeShapeExSceneDestroy(sceneClipClone);
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			xgeShapeExSceneDestroy(sceneClipClone);
			if ( !check(xgeShapeExSceneClipShapeClear(scene) == XGE_OK, "ShapeEx scene clip shape clear") ||
			     !check(xgeShapeExSceneClipShapeGetCount(scene, &gotSceneClipCount) == XGE_OK, "ShapeEx scene clip shape count cleared get") ||
			     !check(gotSceneClipCount == 0, "ShapeEx scene clip shape cleared count value") ||
			     !check(xgeShapeExSceneClipShapeAddEx(scene, sceneClip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx scene subtract clip shape add ex") ||
			     !check(xgeShapeExSceneClipShapeGetAtEx(scene, 0, &gotSceneClip, &gotSceneClipMode) == XGE_OK, "ShapeEx scene subtract clip shape get at ex") ||
			     !check((gotSceneClip == sceneClip) && (gotSceneClipMode == XGE_SHAPE_EX_CLIP_SUBTRACT), "ShapeEx scene subtract clip shape get at ex value") ||
			     !check(xgeShapeExSceneClipClear(scene) == XGE_OK, "ShapeEx scene clip clear") ||
			     !check(xgeShapeExSceneClipRectGet(scene, &gotSceneClipRect, &gotSceneClipEnabled) == XGE_OK, "ShapeEx scene clip rect cleared get") ||
			     !check(gotSceneClipEnabled == 0, "ShapeEx scene clip rect cleared value") ) {
				xgeShapeExDestroy(sceneClip);
				return 0;
			}
			xgeShapeExDestroy(sceneClip);
		}
		if ( !check(xgeShapeExSceneGetCount(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene count get rejects null") ) return 0;
		if ( !check(xgeShapeExSceneGetAt(scene, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene get at rejects null") ) return 0;
		if ( !check(xgeShapeExSceneOpacityGet(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene opacity get rejects null") ) return 0;
		if ( !check(xgeShapeExSceneTransformGet(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene transform get rejects null") ) return 0;
	}
	if ( !check(xgeShapeExSceneDrawEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw ex rejects null scene") ) return 0;
	if ( !check(xgeShapeExSceneDrawPxEx(NULL, 0.25f, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw px ex rejects null scene") ) return 0;
	if ( !check(xgeShapeExSceneDraw(NULL, 0.25f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene world draw rejects null scene") ) return 0;
	{
		xge_shape_ex_matrix_t badMatrix = matrix;

		badMatrix.fF = invalidNaN;
		if ( !check(xgeShapeExSceneDrawEx(scene, 0.25f, &badMatrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw ex rejects invalid parent matrix") ||
		     !check(xgeShapeExSceneDrawPxEx(scene, 0.25f, &matrix, invalidNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw px ex rejects invalid parent opacity") ||
		     !check(xgeShapeExSceneDrawEx(scene, invalidNaN, &matrix, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene draw ex rejects invalid tolerance") ) return 0;
	}
	if ( !check(xgeShapeExSceneClear(scene) == XGE_OK, "ShapeEx scene clear") ) return 0;
	{
		xge_shape_ex_matrix_t clearTransform;
		float clearOpacity = 0.0f;
		int clearCount = -1;
		int clearVisible = -1;

		if ( !check(xgeShapeExSceneGetCount(scene, &clearCount) == XGE_OK && clearCount == 0, "ShapeEx scene clear removes only children") ||
		     !check(xgeShapeExSceneOpacityGet(scene, &clearOpacity) == XGE_OK && clearOpacity == 0.5f, "ShapeEx scene clear preserves opacity") ||
		     !check(xgeShapeExSceneVisibleGet(scene, &clearVisible) == XGE_OK && clearVisible == 1, "ShapeEx scene clear preserves visibility") ||
		     !check(xgeShapeExSceneTransformGet(scene, &clearTransform) == XGE_OK &&
		            clearTransform.fA == matrix.fA && clearTransform.fB == matrix.fB &&
		            clearTransform.fC == matrix.fC && clearTransform.fD == matrix.fD &&
		            clearTransform.fE == matrix.fE && clearTransform.fF == matrix.fF,
		            "ShapeEx scene clear preserves transform") ) return 0;
	}
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(shape);
	{
		xge_shape_ex effectShape = NULL;
		xge_shape_ex_scene effectScene = NULL;
		xge_shape_ex_scene effectClone = NULL;
		xge_shape_ex_matrix_t effectTransform;
		xge_shape_ex_scene_effect_t effect;
		xge_rect_t effectBounds;
		int effectCount = -1;
		int shapeCount = -1;

		ret = xgeShapeExCreate(&effectShape);
		if ( !check((ret == XGE_OK) && (effectShape != NULL), "ShapeEx effect shape create") ) return 0;
		ret = xgeShapeExSceneCreate(&effectScene);
		if ( !check((ret == XGE_OK) && (effectScene != NULL), "ShapeEx effect scene create") ) {
			xgeShapeExDestroy(effectShape);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(effectShape, 10.0f, 20.0f, 30.0f, 40.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx effect rect") ||
		     !check(xgeShapeExSceneAdd(effectScene, effectShape) == XGE_OK, "ShapeEx effect scene add") ||
		     !check(xgeShapeExSceneEffectGetCount(effectScene, &effectCount) == XGE_OK, "ShapeEx effect count default") ||
		     !check(effectCount == 0, "ShapeEx effect count default value") ||
		     !check(xgeShapeExSceneEffectGaussianBlur(effectScene, 2.0f, XGE_SHAPE_EX_BLUR_HORIZONTAL, XGE_SHAPE_EX_BORDER_DUPLICATE, 80) == XGE_OK, "ShapeEx effect gaussian add") ||
		     !check(xgeShapeExSceneGetBounds(effectScene, 0.05f, &effectBounds) == XGE_OK, "ShapeEx effect gaussian bounds") ||
		     !check((effectBounds.fX > 9.99f) && (effectBounds.fX < 10.01f) &&
		            (effectBounds.fY > 19.99f) && (effectBounds.fY < 20.01f) &&
		            (effectBounds.fW > 29.99f) && (effectBounds.fW < 30.01f) &&
		            (effectBounds.fH > 39.99f) && (effectBounds.fH < 40.01f), "ShapeEx effect gaussian logical bounds") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx effect clear after gaussian") ||
		     !check(xgeShapeExSceneEffectDropShadow(effectScene, XGE_COLOR_RGBA(20, 30, 40, 200), 90.0f, 10.0f, 2.0f, 80) == XGE_OK, "ShapeEx effect shadow add") ||
		     !check(xgeShapeExSceneGetBounds(effectScene, 0.05f, &effectBounds) == XGE_OK, "ShapeEx effect shadow bounds") ||
		     !check((effectBounds.fX > 9.99f) && (effectBounds.fX < 10.01f) &&
		            (effectBounds.fY > 19.99f) && (effectBounds.fY < 20.01f) &&
		            (effectBounds.fW > 29.99f) && (effectBounds.fW < 30.01f) &&
		            (effectBounds.fH > 39.99f) && (effectBounds.fH < 40.01f), "ShapeEx effect shadow logical bounds") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx effect clear before angled shadow") ||
		     !check(xgeShapeExSceneEffectDropShadow(effectScene, XGE_COLOR_RGBA(20, 30, 40, 200), 135.0f, 10.0f, 2.0f, 80) == XGE_OK, "ShapeEx effect angled shadow add") ||
		     !check(xgeShapeExSceneGetBounds(effectScene, 0.05f, &effectBounds) == XGE_OK, "ShapeEx effect angled shadow bounds") ||
		     !check((effectBounds.fX > 9.99f) && (effectBounds.fX < 10.01f) &&
		            (effectBounds.fY > 19.99f) && (effectBounds.fY < 20.01f) &&
		            (effectBounds.fW > 29.99f) && (effectBounds.fW < 30.01f) &&
		            (effectBounds.fH > 39.99f) && (effectBounds.fH < 40.01f), "ShapeEx effect angled shadow logical bounds") ||
		     !check(xgeShapeExMatrixRotate(&effectTransform, -24.0f * (3.14159265358979323846f / 180.0f)) == XGE_OK, "ShapeEx effect negative rotation matrix") ||
		     !check(xgeShapeExSceneTransformSet(effectScene, &effectTransform) == XGE_OK, "ShapeEx effect negative rotation set") ||
		     !check(xgeShapeExSceneGetBounds(effectScene, 0.05f, &effectBounds) == XGE_OK, "ShapeEx effect negative rotation shadow bounds") ||
		     !check((effectBounds.fX > 17.26f) && (effectBounds.fX < 17.29f) &&
		            (effectBounds.fY > 1.99f) && (effectBounds.fY < 2.02f) &&
		            (effectBounds.fW > 43.66f) && (effectBounds.fW < 43.69f) &&
		            (effectBounds.fH > 48.72f) && (effectBounds.fH < 48.76f), "ShapeEx effect negative rotation logical bounds") ||
		     !check(xgeShapeExSceneTransformIdentity(effectScene) == XGE_OK, "ShapeEx effect negative rotation clear") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx effect clear after shadow") ||
		     !check(xgeShapeExSceneEffectGaussianBlur(effectScene, 1.5f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_WRAP, 75) == XGE_OK, "ShapeEx effect gaussian chain add") ||
		     !check(xgeShapeExSceneEffectDropShadow(effectScene, XGE_COLOR_RGBA(5, 10, 15, 180), 135.0f, 6.0f, 1.0f, 60) == XGE_OK, "ShapeEx effect shadow chain add") ||
		     !check(xgeShapeExSceneEffectFill(effectScene, XGE_COLOR_RGBA(220, 40, 80, 200)) == XGE_OK, "ShapeEx effect fill add") ||
		     !check(xgeShapeExSceneEffectTint(effectScene, XGE_COLOR_RGBA(10, 20, 30, 255), XGE_COLOR_RGBA(230, 240, 250, 255), 65.0f) == XGE_OK, "ShapeEx effect tint add") ||
		     !check(xgeShapeExSceneEffectTritone(effectScene, XGE_COLOR_RGBA(20, 30, 40, 255), XGE_COLOR_RGBA(100, 120, 140, 255), XGE_COLOR_RGBA(230, 240, 250, 255), 32) == XGE_OK, "ShapeEx effect tritone add") ||
		     !check(xgeShapeExSceneEffectGetCount(effectScene, &effectCount) == XGE_OK, "ShapeEx effect chain count") ||
		     !check(effectCount == 5, "ShapeEx effect chain count value") ) {
			xgeShapeExSceneDestroy(effectScene);
			xgeShapeExDestroy(effectShape);
			return 0;
		}
		memset(&effect, 0, sizeof(effect));
		if ( !check(xgeShapeExSceneEffectGetAt(effectScene, 0, &effect) == XGE_OK, "ShapeEx effect get gaussian") ||
		     !check((effect.iType == XGE_SHAPE_EX_EFFECT_GAUSSIAN_BLUR) && (effect.iDirection == XGE_SHAPE_EX_BLUR_BOTH) &&
		            (effect.iBorder == XGE_SHAPE_EX_BORDER_WRAP) && (effect.iQuality == 75) &&
		            (effect.fSigma > 1.49f) && (effect.fSigma < 1.51f), "ShapeEx effect gaussian descriptor") ||
		     !check(xgeShapeExSceneEffectGetAt(effectScene, 4, &effect) == XGE_OK, "ShapeEx effect get tritone") ||
		     !check((effect.iType == XGE_SHAPE_EX_EFFECT_TRITONE) && (effect.iBlend == 32), "ShapeEx effect tritone descriptor") ||
		     !check(xgeShapeExSceneEffectGetAt(effectScene, 5, &effect) == XGE_ERROR_NOT_FOUND, "ShapeEx effect get missing") ||
		     !check(effect.iType == 0, "ShapeEx effect get missing clears output") ) {
			xgeShapeExSceneDestroy(effectScene);
			xgeShapeExDestroy(effectShape);
			return 0;
		}
		ret = xgeShapeExSceneClone(effectScene, &effectClone);
		if ( !check((ret == XGE_OK) && (effectClone != NULL), "ShapeEx effect scene clone") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx source effect clear after clone") ||
		     !check(xgeShapeExSceneEffectGetCount(effectClone, &effectCount) == XGE_OK, "ShapeEx cloned effect count") ||
		     !check(effectCount == 5, "ShapeEx cloned effect count value") ||
		     !check(xgeShapeExSceneClear(effectClone) == XGE_OK, "ShapeEx cloned effect scene content clear") ||
		     !check(xgeShapeExSceneGetCount(effectClone, &shapeCount) == XGE_OK, "ShapeEx cloned effect scene shape count") ||
		     !check(shapeCount == 0, "ShapeEx cloned effect scene shape count value") ||
		     !check(xgeShapeExSceneEffectGetCount(effectClone, &effectCount) == XGE_OK, "ShapeEx cloned effects survive scene clear") ||
		     !check(effectCount == 5, "ShapeEx cloned effects survive scene clear value") ||
		     !check(xgeShapeExSceneEffectClear(effectClone) == XGE_OK, "ShapeEx cloned effect clear") ) {
			xgeShapeExSceneDestroy(effectClone);
			xgeShapeExSceneDestroy(effectScene);
			xgeShapeExDestroy(effectShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneEffectGaussianBlur(effectScene, 1.0e30f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 50) == XGE_OK, "ShapeEx effect gaussian accepts large finite sigma") ||
		     !check(xgeShapeExSceneGetBounds(effectScene, 0.05f, &effectBounds) == XGE_OK, "ShapeEx effect large gaussian bounds") ||
		     !check((effectBounds.fX > 9.99f) && (effectBounds.fX < 10.01f) &&
		            (effectBounds.fY > 19.99f) && (effectBounds.fY < 20.01f) &&
		            (effectBounds.fW > 29.99f) && (effectBounds.fW < 30.01f) &&
		            (effectBounds.fH > 39.99f) && (effectBounds.fH < 40.01f), "ShapeEx large effect keeps finite logical bounds") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx effect clear after large gaussian") ||
		     !check(xgeShapeExSceneEffectGaussianBlur(effectScene, -1.0f, 9, 9, 101) == XGE_OK, "ShapeEx effect gaussian normalizes finite parameters") ||
		     !check(xgeShapeExSceneEffectGetAt(effectScene, 0, &effect) == XGE_OK, "ShapeEx normalized gaussian get") ||
		     !check((effect.fSigma == 0.0f) && (effect.iDirection == XGE_SHAPE_EX_BLUR_VERTICAL) &&
		            (effect.iBorder == XGE_SHAPE_EX_BORDER_WRAP) && (effect.iQuality == 100), "ShapeEx normalized gaussian descriptor") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx normalized gaussian clear") ||
		     !check(xgeShapeExSceneEffectDropShadow(effectScene, 0, 90.0f, 1.0f, -1.0f, 101) == XGE_OK, "ShapeEx effect shadow normalizes finite parameters") ||
		     !check(xgeShapeExSceneEffectGetAt(effectScene, 0, &effect) == XGE_OK, "ShapeEx normalized shadow get") ||
		     !check((effect.fSigma == 0.0f) && (effect.iQuality == 100), "ShapeEx normalized shadow descriptor") ||
		     !check(xgeShapeExSceneEffectClear(effectScene) == XGE_OK, "ShapeEx normalized shadow clear") ||
		     !check(xgeShapeExSceneEffectGaussianBlur(effectScene, invalidNaN, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 50) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect gaussian rejects nan") ||
		     !check(xgeShapeExSceneEffectDropShadow(effectScene, 0, invalidNaN, 1.0f, 1.0f, 50) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect shadow rejects nan") ||
		     !check(xgeShapeExSceneEffectTint(effectScene, 0, 0, 101.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect tint rejects intensity") ||
		     !check(xgeShapeExSceneEffectTritone(effectScene, 0, 0, 0, 256) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect tritone rejects blend") ||
		     !check(xgeShapeExSceneEffectGetCount(NULL, &effectCount) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect count rejects null scene") ||
		     !check(xgeShapeExSceneEffectGetCount(effectScene, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect count rejects null output") ||
		     !check(xgeShapeExSceneEffectGetAt(effectScene, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect get rejects null output") ||
		     !check(xgeShapeExSceneEffectClear(NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx effect clear rejects null scene") ) {
			xgeShapeExSceneDestroy(effectClone);
			xgeShapeExSceneDestroy(effectScene);
			xgeShapeExDestroy(effectShape);
			return 0;
		}
		xgeShapeExSceneDestroy(effectClone);
		xgeShapeExSceneDestroy(effectScene);
		xgeShapeExDestroy(effectShape);
	}
	{
		xge_shape_ex_scene rootScene = NULL;
		xge_shape_ex_scene childScene = NULL;
		xge_shape_ex_scene cloneScene = NULL;
		xge_shape_ex rootShape = NULL;
		xge_shape_ex childShape = NULL;
		xge_shape_ex tailShape = NULL;
		xge_shape_ex hitShape = NULL;
		xge_shape_ex cloneChildShape = NULL;
		xge_shape_ex_scene_child_t childInfo;
		xge_shape_ex_scene_child_t cloneChildInfo;
		xge_shape_ex_scene_child_t cloneLeafInfo;
		xge_rect_t nestedBounds;
		xge_rect_t cloneBounds;
		xge_vec2_t nestedObb[4];
		int nestedCount = -1;
		int nestedOk = 1;

		nestedOk = nestedOk && check(xgeShapeExSceneCreate(&rootScene) == XGE_OK, "ShapeEx nested root create");
		nestedOk = nestedOk && check(xgeShapeExSceneCreate(&childScene) == XGE_OK, "ShapeEx nested child create");
		nestedOk = nestedOk && check(xgeShapeExCreate(&rootShape) == XGE_OK, "ShapeEx nested root shape create");
		nestedOk = nestedOk && check(xgeShapeExCreate(&childShape) == XGE_OK, "ShapeEx nested child shape create");
		nestedOk = nestedOk && check(xgeShapeExCreate(&tailShape) == XGE_OK, "ShapeEx nested tail shape create");
		if ( nestedOk ) {
			nestedOk = nestedOk && check(xgeShapeExFillColor(rootShape, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx nested root fill");
			nestedOk = nestedOk && check(xgeShapeExFillColor(childShape, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx nested child fill");
			nestedOk = nestedOk && check(xgeShapeExFillColor(tailShape, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx nested tail fill");
			nestedOk = nestedOk && check(xgeShapeExAppendRect(rootShape, 0.0f, 0.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx nested root rect");
			nestedOk = nestedOk && check(xgeShapeExAppendRect(childShape, 0.0f, 0.0f, 10.0f, 12.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx nested child rect");
			nestedOk = nestedOk && check(xgeShapeExAppendRect(tailShape, 40.0f, 0.0f, 3.0f, 3.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx nested tail rect");
			nestedOk = nestedOk && check(xgeShapeExTransformTranslate(childShape, 2.0f, 3.0f) == XGE_OK, "ShapeEx nested child shape transform");
			nestedOk = nestedOk && check(xgeShapeExSceneTransformTranslate(childScene, 20.0f, 30.0f) == XGE_OK, "ShapeEx nested child scene transform");
			nestedOk = nestedOk && check(xgeShapeExSceneTransformTranslate(rootScene, 5.0f, 7.0f) == XGE_OK, "ShapeEx nested root scene transform");
			nestedOk = nestedOk && check(xgeShapeExSceneAdd(childScene, childShape) == XGE_OK, "ShapeEx nested child add shape");
			nestedOk = nestedOk && check(xgeShapeExSceneAddScene(rootScene, childScene) == XGE_OK, "ShapeEx nested root add scene");
			nestedOk = nestedOk && check(xgeShapeExSceneInsertShapeAt(rootScene, rootShape, 0) == XGE_OK, "ShapeEx nested insert root shape before scene");
			nestedOk = nestedOk && check(xgeShapeExSceneAdd(rootScene, tailShape) == XGE_OK, "ShapeEx nested add tail shape");
		}
		memset(&childInfo, 0, sizeof(childInfo));
		if ( nestedOk ) {
			nestedOk = nestedOk && check(xgeShapeExSceneGetCount(rootScene, &nestedCount) == XGE_OK, "ShapeEx nested child count");
			nestedOk = nestedOk && check(nestedCount == 3, "ShapeEx nested child count value");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(rootScene, 0, &childInfo) == XGE_OK, "ShapeEx nested get root shape child");
			nestedOk = nestedOk && check((childInfo.iType == XGE_SHAPE_EX_SCENE_CHILD_SHAPE) && (childInfo.pShape == rootShape) && (childInfo.pScene == NULL), "ShapeEx nested root shape descriptor");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(rootScene, 1, &childInfo) == XGE_OK, "ShapeEx nested get scene child");
			nestedOk = nestedOk && check((childInfo.iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE) && (childInfo.pScene == childScene) && (childInfo.pShape == NULL), "ShapeEx nested scene descriptor");
			nestedOk = nestedOk && check(xgeShapeExSceneGetAt(rootScene, 1, &hitShape) == XGE_ERROR_NOT_FOUND, "ShapeEx nested shape getter rejects scene child");
			nestedOk = nestedOk && check(hitShape == NULL, "ShapeEx nested shape getter clears output");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(rootScene, 3, &childInfo) == XGE_ERROR_NOT_FOUND, "ShapeEx nested child getter rejects missing");
			nestedOk = nestedOk && check(childInfo.iType == 0, "ShapeEx nested missing child clears output");
			nestedOk = nestedOk && check(xgeShapeExSceneRemove(rootScene, tailShape) == XGE_OK, "ShapeEx nested remove tail shape");
			nestedOk = nestedOk && check(xgeShapeExSceneInsertShapeAt(rootScene, tailShape, 3) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx nested insert rejects invalid index");
			nestedOk = nestedOk && check(xgeShapeExSceneInsertSceneAt(rootScene, rootScene, 0) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx nested insert rejects self cycle");
			nestedOk = nestedOk && check(xgeShapeExSceneAddScene(childScene, rootScene) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx nested insert rejects ancestor cycle");
		}
		if ( nestedOk ) {
			nestedOk = nestedOk && check(xgeShapeExSceneGetBounds(rootScene, 0.05f, &nestedBounds) == XGE_OK, "ShapeEx nested bounds");
			nestedOk = nestedOk && check((nestedBounds.fX > 4.99f) && (nestedBounds.fX < 5.01f) &&
				(nestedBounds.fY > 6.99f) && (nestedBounds.fY < 7.01f) &&
				(nestedBounds.fW > 31.99f) && (nestedBounds.fW < 32.01f) &&
				(nestedBounds.fH > 44.99f) && (nestedBounds.fH < 45.01f), "ShapeEx nested bounds value");
			nestedOk = nestedOk && check(xgeShapeExSceneGetOBB(rootScene, 0.05f, nestedObb) == XGE_OK, "ShapeEx nested obb");
			nestedOk = nestedOk && check((nestedObb[0].fX > 4.99f) && (nestedObb[0].fY > 6.99f) &&
				(nestedObb[2].fX > 36.99f) && (nestedObb[2].fY > 51.99f), "ShapeEx nested obb value");
			nestedOk = nestedOk && check(xgeShapeExSceneHitTest(rootScene, 30.0f, 45.0f, 0.05f, &hitShape) == XGE_OK, "ShapeEx nested hit test");
			nestedOk = nestedOk && check(hitShape == childShape, "ShapeEx nested hit returns leaf shape");
			nestedOk = nestedOk && check(xgeShapeExSceneEffectGaussianBlur(childScene, 2.0f, XGE_SHAPE_EX_BLUR_BOTH, XGE_SHAPE_EX_BORDER_DUPLICATE, 100) == XGE_OK, "ShapeEx nested child effect");
			nestedOk = nestedOk && check(xgeShapeExSceneGetBounds(rootScene, 0.05f, &nestedBounds) == XGE_OK, "ShapeEx nested child effect bounds");
			nestedOk = nestedOk && check((nestedBounds.fX > 4.99f) && (nestedBounds.fX < 5.01f) &&
				(nestedBounds.fY > 6.99f) && (nestedBounds.fY < 7.01f) &&
				(nestedBounds.fW > 31.99f) && (nestedBounds.fW < 32.01f) &&
				(nestedBounds.fH > 44.99f) && (nestedBounds.fH < 45.01f), "ShapeEx nested child effect logical bounds value");
			nestedOk = nestedOk && check(xgeShapeExSceneEffectClear(childScene) == XGE_OK, "ShapeEx nested child effect clear");
			nestedOk = nestedOk && check(xgeShapeExSceneClone(rootScene, &cloneScene) == XGE_OK, "ShapeEx nested deep clone");
		}
		memset(&cloneChildInfo, 0, sizeof(cloneChildInfo));
		memset(&cloneLeafInfo, 0, sizeof(cloneLeafInfo));
		if ( nestedOk ) {
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(cloneScene, 1, &cloneChildInfo) == XGE_OK, "ShapeEx nested clone scene child");
			nestedOk = nestedOk && check((cloneChildInfo.iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE) && (cloneChildInfo.pScene != childScene), "ShapeEx nested clone scene independence");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(cloneChildInfo.pScene, 0, &cloneLeafInfo) == XGE_OK, "ShapeEx nested clone leaf child");
			cloneChildShape = cloneLeafInfo.pShape;
			nestedOk = nestedOk && check((cloneLeafInfo.iType == XGE_SHAPE_EX_SCENE_CHILD_SHAPE) && (cloneChildShape != childShape), "ShapeEx nested clone leaf independence");
			nestedOk = nestedOk && check(xgeShapeExSceneTransformTranslate(childScene, 80.0f, 70.0f) == XGE_OK, "ShapeEx nested mutate source child");
			nestedOk = nestedOk && check(xgeShapeExSceneGetBounds(cloneScene, 0.05f, &cloneBounds) == XGE_OK, "ShapeEx nested clone bounds");
			nestedOk = nestedOk && check((cloneBounds.fX > 4.99f) && (cloneBounds.fX < 5.01f) &&
				(cloneBounds.fY > 6.99f) && (cloneBounds.fY < 7.01f) &&
				(cloneBounds.fW > 31.99f) && (cloneBounds.fW < 32.01f) &&
				(cloneBounds.fH > 44.99f) && (cloneBounds.fH < 45.01f), "ShapeEx nested clone preserves child transform");
			matrix.fA = 0.0f;
			matrix.fB = 1.0f;
			matrix.fC = -1.0f;
			matrix.fD = 0.0f;
			matrix.fE = 10.0f;
			matrix.fF = 20.0f;
			nestedOk = nestedOk && check(xgeShapeExSceneTransformSet(cloneScene, &matrix) == XGE_OK, "ShapeEx nested clone obb transform");
			nestedOk = nestedOk && check(xgeShapeExSceneGetOBB(cloneScene, 0.05f, nestedObb) == XGE_OK, "ShapeEx nested rotated obb");
			nestedOk = nestedOk && check(
				(nestedObb[0].fX > 9.99f) && (nestedObb[0].fX < 10.01f) &&
				(nestedObb[0].fY > 19.99f) && (nestedObb[0].fY < 20.01f) &&
				(nestedObb[1].fX > 9.99f) && (nestedObb[1].fX < 10.01f) &&
				(nestedObb[1].fY > 51.99f) && (nestedObb[1].fY < 52.01f) &&
				(nestedObb[2].fX > -35.01f) && (nestedObb[2].fX < -34.99f) &&
				(nestedObb[2].fY > 51.99f) && (nestedObb[2].fY < 52.01f) &&
				(nestedObb[3].fX > -35.01f) && (nestedObb[3].fX < -34.99f) &&
				(nestedObb[3].fY > 19.99f) && (nestedObb[3].fY < 20.01f),
				"ShapeEx nested rotated obb value"
			);
			nestedOk = nestedOk && check(xgeShapeExSceneRemoveScene(rootScene, childScene) == XGE_OK, "ShapeEx nested remove child scene");
			nestedOk = nestedOk && check(xgeShapeExSceneRemoveScene(rootScene, childScene) == XGE_ERROR_NOT_FOUND, "ShapeEx nested remove missing child scene");
			nestedOk = nestedOk && check(xgeShapeExSceneGetCount(rootScene, &nestedCount) == XGE_OK, "ShapeEx nested count after remove");
			nestedOk = nestedOk && check(nestedCount == 1, "ShapeEx nested count after remove value");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(NULL, 0, &childInfo) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx nested child getter rejects null scene");
			nestedOk = nestedOk && check(xgeShapeExSceneChildGetAt(rootScene, 0, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx nested child getter rejects null output");
		}
		xgeShapeExSceneDestroy(cloneScene);
		xgeShapeExSceneDestroy(rootScene);
		xgeShapeExSceneDestroy(childScene);
		xgeShapeExDestroy(rootShape);
		xgeShapeExDestroy(childShape);
		xgeShapeExDestroy(tailShape);
		if ( !nestedOk ) return 0;
	}
	{
		xge_shape_ex_scene ownerA = NULL;
		xge_shape_ex_scene ownerB = NULL;
		xge_shape_ex_scene child = NULL;
		xge_shape_ex_scene clone = NULL;
		xge_shape_ex_scene parent = NULL;
		xge_shape_ex ownedShape = NULL;
		xge_shape_ex_scene_child_t cloneChild;
		int ownerOk = 1;

		ownerOk = ownerOk && check(xgeShapeExSceneCreate(&ownerA) == XGE_OK, "ShapeEx parent owner A create");
		ownerOk = ownerOk && check(xgeShapeExSceneCreate(&ownerB) == XGE_OK, "ShapeEx parent owner B create");
		ownerOk = ownerOk && check(xgeShapeExSceneCreate(&child) == XGE_OK, "ShapeEx parent child scene create");
		ownerOk = ownerOk && check(xgeShapeExCreate(&ownedShape) == XGE_OK, "ShapeEx parent shape create");
		if ( ownerOk ) {
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, &parent) == XGE_OK, "ShapeEx parent shape initial get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent shape initially detached");
			parent = ownerA;
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, &parent) == XGE_OK, "ShapeEx parent scene initial get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent scene initially detached");
			ownerOk = ownerOk && check(xgeShapeExSceneAdd(ownerA, ownedShape) == XGE_OK, "ShapeEx parent add shape to owner A");
			ownerOk = ownerOk && check(xgeShapeExSceneAddScene(ownerA, child) == XGE_OK, "ShapeEx parent add scene to owner A");
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, &parent) == XGE_OK, "ShapeEx parent shape attached get");
			ownerOk = ownerOk && check(parent == ownerA, "ShapeEx parent shape attached value");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, &parent) == XGE_OK, "ShapeEx parent scene attached get");
			ownerOk = ownerOk && check(parent == ownerA, "ShapeEx parent scene attached value");
			ownerOk = ownerOk && check(xgeShapeExSceneAdd(ownerB, ownedShape) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent rejects shape multi-owner");
			ownerOk = ownerOk && check(xgeShapeExSceneAddScene(ownerB, child) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent rejects scene multi-owner");
			ownerOk = ownerOk && check(xgeShapeExSceneAdd(ownerA, ownedShape) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent rejects duplicate shape membership");
			ownerOk = ownerOk && check(xgeShapeExSceneAddScene(ownerA, child) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent rejects duplicate scene membership");
			ownerOk = ownerOk && check(xgeShapeExSceneRemove(ownerA, ownedShape) == XGE_OK, "ShapeEx parent detach shape");
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, &parent) == XGE_OK, "ShapeEx parent detached shape get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent detached shape value");
			ownerOk = ownerOk && check(xgeShapeExSceneAdd(ownerB, ownedShape) == XGE_OK, "ShapeEx parent reparent shape");
			ownerOk = ownerOk && check(xgeShapeExSceneRemoveScene(ownerA, child) == XGE_OK, "ShapeEx parent detach scene");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, &parent) == XGE_OK, "ShapeEx parent detached scene get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent detached scene value");
			ownerOk = ownerOk && check(xgeShapeExSceneAddScene(ownerB, child) == XGE_OK, "ShapeEx parent reparent scene");
			ownerOk = ownerOk && check(xgeShapeExSceneClone(ownerB, &clone) == XGE_OK, "ShapeEx parent clone owner");
		}
		memset(&cloneChild, 0, sizeof(cloneChild));
		if ( ownerOk ) {
			ownerOk = ownerOk && check(xgeShapeExSceneChildGetAt(clone, 0, &cloneChild) == XGE_OK, "ShapeEx parent clone shape child get");
			ownerOk = ownerOk && check(cloneChild.iType == XGE_SHAPE_EX_SCENE_CHILD_SHAPE, "ShapeEx parent clone shape child type");
			ownerOk = ownerOk && check(xgeShapeExParentGet(cloneChild.pShape, &parent) == XGE_OK, "ShapeEx parent clone shape parent get");
			ownerOk = ownerOk && check(parent == clone, "ShapeEx parent clone shape parent value");
			ownerOk = ownerOk && check(xgeShapeExSceneChildGetAt(clone, 1, &cloneChild) == XGE_OK, "ShapeEx parent clone scene child get");
			ownerOk = ownerOk && check(cloneChild.iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE, "ShapeEx parent clone scene child type");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(cloneChild.pScene, &parent) == XGE_OK, "ShapeEx parent clone scene parent get");
			ownerOk = ownerOk && check(parent == clone, "ShapeEx parent clone scene parent value");
			ownerOk = ownerOk && check(xgeShapeExSceneClear(ownerB) == XGE_OK, "ShapeEx parent clear owner B");
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, &parent) == XGE_OK, "ShapeEx parent clear shape get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent clear detaches shape");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, &parent) == XGE_OK, "ShapeEx parent clear scene get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent clear detaches scene");
			ownerOk = ownerOk && check(xgeShapeExSceneAdd(ownerA, ownedShape) == XGE_OK, "ShapeEx parent destroy add shape");
			ownerOk = ownerOk && check(xgeShapeExSceneAddScene(ownerA, child) == XGE_OK, "ShapeEx parent destroy add scene");
			xgeShapeExSceneDestroy(ownerA);
			ownerA = NULL;
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, &parent) == XGE_OK, "ShapeEx parent destroyed owner shape get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent destroyed owner detaches shape");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, &parent) == XGE_OK, "ShapeEx parent destroyed owner scene get");
			ownerOk = ownerOk && check(parent == NULL, "ShapeEx parent destroyed owner detaches scene");
			ownerOk = ownerOk && check(xgeShapeExParentGet(NULL, &parent) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent shape rejects null paint");
			ownerOk = ownerOk && check(xgeShapeExParentGet(ownedShape, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent shape rejects null output");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(NULL, &parent) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent scene rejects null paint");
			ownerOk = ownerOk && check(xgeShapeExSceneParentGet(child, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx parent scene rejects null output");
		}
		xgeShapeExSceneDestroy(clone);
		xgeShapeExSceneDestroy(ownerB);
		xgeShapeExSceneDestroy(ownerA);
		xgeShapeExSceneDestroy(child);
		xgeShapeExDestroy(ownedShape);
		if ( !ownerOk ) return 0;
	}
	{
		xge_shape_ex_scene ownerA = NULL;
		xge_shape_ex_scene ownerB = NULL;
		xge_shape_ex_scene maskScene = NULL;
		xge_shape_ex_scene parent = NULL;
		xge_shape_ex source = NULL;
		xge_shape_ex clip = NULL;
		xge_shape_ex mask = NULL;
		xge_shape_ex otherSource = NULL;
		xge_shape_ex pendingSource = NULL;
		xge_shape_ex pendingMask = NULL;
		xge_shape_ex gotMask = NULL;
		int decorationOk = 1;

		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&ownerA) == XGE_OK, "ShapeEx decoration owner A create");
		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&ownerB) == XGE_OK, "ShapeEx decoration owner B create");
		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&maskScene) == XGE_OK, "ShapeEx decoration mask scene create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&source) == XGE_OK, "ShapeEx decoration source create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx decoration clip create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&mask) == XGE_OK, "ShapeEx decoration mask create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&otherSource) == XGE_OK, "ShapeEx decoration other source create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&pendingSource) == XGE_OK, "ShapeEx decoration pending source create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&pendingMask) == XGE_OK, "ShapeEx decoration pending mask create");
		if ( decorationOk ) {
			decorationOk = decorationOk && check(xgeShapeExClipShapeAdd(source, clip) == XGE_OK, "ShapeEx decoration detached clip set");
			decorationOk = decorationOk && check(xgeShapeExMaskShapeSet(source, mask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx decoration detached mask set");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration detached clip parent");
			decorationOk = decorationOk && check(xgeShapeExParentGet(mask, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration detached mask parent");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerA, source) == XGE_OK, "ShapeEx decoration source attach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == ownerA, "ShapeEx decoration clip inherits parent");
			decorationOk = decorationOk && check(xgeShapeExParentGet(mask, &parent) == XGE_OK && parent == ownerA, "ShapeEx decoration mask inherits parent");
			decorationOk = decorationOk && check(xgeShapeExClipShapeAdd(otherSource, clip) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx decoration rejects owned clip");
			decorationOk = decorationOk && check(xgeShapeExMaskShapeSet(otherSource, mask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx decoration rejects owned mask");
			decorationOk = decorationOk && check(xgeShapeExMaskShapeSet(source, mask, XGE_SHAPE_EX_MASK_ADD) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx decoration parented mask reset rejects owned target");
			decorationOk = decorationOk && check(xgeShapeExMaskGet(source, NULL, NULL, &gotMask, NULL) == XGE_OK && gotMask == mask, "ShapeEx decoration failed reset preserves mask");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerB, clip) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx decoration target rejects scene membership");
			decorationOk = decorationOk && check(xgeShapeExSceneRemove(ownerA, source) == XGE_OK, "ShapeEx decoration source detach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration detach clears clip parent");
			decorationOk = decorationOk && check(xgeShapeExParentGet(mask, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration detach clears mask parent");
			decorationOk = decorationOk && check(xgeShapeExMaskSceneSet(source, maskScene, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx decoration replace with scene mask");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerB, source) == XGE_OK, "ShapeEx decoration reattach source");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == ownerB, "ShapeEx decoration reattach clip parent");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(maskScene, &parent) == XGE_OK && parent == ownerB, "ShapeEx decoration scene mask inherits parent");
			decorationOk = decorationOk && check(xgeShapeExMaskClear(source) == XGE_OK, "ShapeEx decoration clear scene mask");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(maskScene, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration clear scene mask parent");
			decorationOk = decorationOk && check(xgeShapeExClipShapeClear(source) == XGE_OK, "ShapeEx decoration clear clip");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration clear clip parent");
			decorationOk = decorationOk && check(xgeShapeExMaskShapeSet(source, mask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx decoration set mask while parented");
			decorationOk = decorationOk && check(xgeShapeExClipShapeAdd(source, clip) == XGE_OK, "ShapeEx decoration set clip while parented");
			decorationOk = decorationOk && check(xgeShapeExParentGet(mask, &parent) == XGE_OK && parent == ownerB, "ShapeEx decoration immediate mask parent");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == ownerB, "ShapeEx decoration immediate clip parent");

			decorationOk = decorationOk && check(xgeShapeExMaskShapeSet(pendingSource, pendingMask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx decoration pending mask set");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerA, pendingMask) == XGE_OK, "ShapeEx decoration pending target separately owned");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerB, pendingSource) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx decoration attach transaction rejects occupied target");
			decorationOk = decorationOk && check(xgeShapeExParentGet(pendingSource, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration failed attach keeps source detached");
			decorationOk = decorationOk && check(xgeShapeExParentGet(pendingMask, &parent) == XGE_OK && parent == ownerA, "ShapeEx decoration failed attach preserves target parent");
			decorationOk = decorationOk && check(xgeShapeExSceneRemove(ownerA, pendingMask) == XGE_OK, "ShapeEx decoration release pending target");
			decorationOk = decorationOk && check(xgeShapeExSceneAdd(ownerB, pendingSource) == XGE_OK, "ShapeEx decoration attach after target release");
			decorationOk = decorationOk && check(xgeShapeExParentGet(pendingMask, &parent) == XGE_OK && parent == ownerB, "ShapeEx decoration pending mask inherits parent");
			decorationOk = decorationOk && check(xgeShapeExSceneRemove(ownerB, pendingSource) == XGE_OK, "ShapeEx decoration pending source detach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(pendingMask, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration pending mask detached");

			xgeShapeExSceneDestroy(ownerB);
			ownerB = NULL;
			decorationOk = decorationOk && check(xgeShapeExParentGet(source, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration owner destroy detaches source");
			decorationOk = decorationOk && check(xgeShapeExParentGet(mask, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration owner destroy detaches mask");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx decoration owner destroy detaches clip");
		}
		xgeShapeExSceneDestroy(ownerA);
		xgeShapeExSceneDestroy(ownerB);
		xgeShapeExSceneDestroy(maskScene);
		xgeShapeExDestroy(pendingMask);
		xgeShapeExDestroy(pendingSource);
		xgeShapeExDestroy(otherSource);
		xgeShapeExDestroy(mask);
		xgeShapeExDestroy(clip);
		xgeShapeExDestroy(source);
		if ( !decorationOk ) return 0;
	}
	{
		xge_shape_ex_scene owner = NULL;
		xge_shape_ex_scene source = NULL;
		xge_shape_ex_scene mask = NULL;
		xge_shape_ex_scene clone = NULL;
		xge_shape_ex_scene cloneMask = NULL;
		xge_shape_ex_scene parent = NULL;
		xge_shape_ex clip = NULL;
		xge_shape_ex cloneClip = NULL;
		int decorationOk = 1;

		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&owner) == XGE_OK, "ShapeEx scene decoration owner create");
		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&source) == XGE_OK, "ShapeEx scene decoration source create");
		decorationOk = decorationOk && check(xgeShapeExSceneCreate(&mask) == XGE_OK, "ShapeEx scene decoration mask create");
		decorationOk = decorationOk && check(xgeShapeExCreate(&clip) == XGE_OK, "ShapeEx scene decoration clip create");
		if ( decorationOk ) {
			decorationOk = decorationOk && check(xgeShapeExSceneClipShapeAdd(source, clip) == XGE_OK, "ShapeEx scene decoration clip set");
			decorationOk = decorationOk && check(xgeShapeExSceneMaskSceneSet(source, mask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx scene decoration mask set");
			decorationOk = decorationOk && check(xgeShapeExSceneAddScene(owner, source) == XGE_OK, "ShapeEx scene decoration source attach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration clip inherits parent");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(mask, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration mask inherits parent");
			decorationOk = decorationOk && check(xgeShapeExSceneRemoveScene(owner, source) == XGE_OK, "ShapeEx scene decoration source detach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration clip detaches");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(mask, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration mask detaches");
			decorationOk = decorationOk && check(xgeShapeExSceneClone(source, &clone) == XGE_OK, "ShapeEx scene decoration clone");
			decorationOk = decorationOk && check(xgeShapeExSceneClipShapeGetAt(clone, 0, &cloneClip) == XGE_OK, "ShapeEx scene decoration clone clip get");
			decorationOk = decorationOk && check(xgeShapeExAddRef(cloneClip) == XGE_OK, "ShapeEx scene decoration clone clip retain");
			decorationOk = decorationOk && check(xgeShapeExSceneMaskGet(clone, NULL, NULL, NULL, &cloneMask) == XGE_OK, "ShapeEx scene decoration clone mask get");
			decorationOk = decorationOk && check(xgeShapeExSceneAddRef(cloneMask) == XGE_OK, "ShapeEx scene decoration clone mask retain");
			decorationOk = decorationOk && check(xgeShapeExParentGet(cloneClip, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration clone clip detached");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(cloneMask, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration clone mask detached");
			decorationOk = decorationOk && check(xgeShapeExSceneAddScene(owner, clone) == XGE_OK, "ShapeEx scene decoration clone attach");
			decorationOk = decorationOk && check(xgeShapeExParentGet(cloneClip, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration clone clip parent");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(cloneMask, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration clone mask parent");
			decorationOk = decorationOk && check(xgeShapeExSceneMaskClear(clone) == XGE_OK, "ShapeEx scene decoration clone mask clear");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(cloneMask, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration clone mask clear parent");
			decorationOk = decorationOk && check(xgeShapeExSceneMaskSceneSet(clone, cloneMask, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx scene decoration set mask while parented");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(cloneMask, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration immediate mask parent");
			decorationOk = decorationOk && check(xgeShapeExSceneMaskClear(clone) == XGE_OK, "ShapeEx scene decoration parented mask clear");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(cloneMask, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration parented mask detached");
			decorationOk = decorationOk && check(xgeShapeExSceneClipShapeClear(clone) == XGE_OK, "ShapeEx scene decoration clone clip clear");
			decorationOk = decorationOk && check(xgeShapeExParentGet(cloneClip, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration clone clip clear parent");
			decorationOk = decorationOk && check(xgeShapeExSceneClipShapeAdd(clone, cloneClip) == XGE_OK, "ShapeEx scene decoration set clip while parented");
			decorationOk = decorationOk && check(xgeShapeExParentGet(cloneClip, &parent) == XGE_OK && parent == owner, "ShapeEx scene decoration immediate clip parent");
			xgeShapeExSceneDestroy(owner);
			owner = NULL;
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(clone, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration owner destroy detaches clone");
			decorationOk = decorationOk && check(xgeShapeExParentGet(cloneClip, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration owner destroy detaches clone clip");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(source, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration original remains detached");
			decorationOk = decorationOk && check(xgeShapeExParentGet(clip, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration original clip remains detached");
			decorationOk = decorationOk && check(xgeShapeExSceneParentGet(mask, &parent) == XGE_OK && parent == NULL, "ShapeEx scene decoration original mask remains detached");
		}
		xgeShapeExSceneDestroy(cloneMask);
		xgeShapeExSceneDestroy(clone);
		xgeShapeExDestroy(cloneClip);
		xgeShapeExSceneDestroy(mask);
		xgeShapeExSceneDestroy(source);
		xgeShapeExSceneDestroy(owner);
		xgeShapeExDestroy(clip);
		if ( !decorationOk ) return 0;
	}
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
		xge_shape_ex intersectsShape = NULL;
		xge_shape_ex_scene intersectsScene = NULL;
		xge_shape_ex_matrix_t parentMatrix;
		int intersects = -1;
		int intersectsOk = 0;

		ret = xgeShapeExCreate(&intersectsShape);
		if ( !check((ret == XGE_OK) && (intersectsShape != NULL), "ShapeEx precise intersects create") ) goto shape_ex_intersects_cleanup;
		if ( !check(xgeShapeExFillColor(intersectsShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx precise intersects fill") ||
		     !check(xgeShapeExMoveTo(intersectsShape, 0.0f, 0.0f) == XGE_OK, "ShapeEx precise intersects triangle move") ||
		     !check(xgeShapeExLineTo(intersectsShape, 20.0f, 0.0f) == XGE_OK, "ShapeEx precise intersects triangle line 1") ||
		     !check(xgeShapeExLineTo(intersectsShape, 0.0f, 20.0f) == XGE_OK, "ShapeEx precise intersects triangle line 2") ||
		     !check(xgeShapeExClose(intersectsShape) == XGE_OK, "ShapeEx precise intersects triangle close") ) goto shape_ex_intersects_cleanup;
		if ( !check(xgeShapeExBoundsIntersects(intersectsShape, (xge_rect_t){15.0f, 15.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects coarse bounds hit") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){15.0f, 15.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise intersects rejects AABB-only hit") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){2.0f, 2.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects fill hit") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){2.0f, 2.0f, 0.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise intersects empty query") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){2.0f, 2.0f, 2.0f, 2.0f}, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx precise intersects rejects null output") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){invalidNaN, 2.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx precise intersects rejects nan rect") ) goto shape_ex_intersects_cleanup;

		parentMatrix.fA = 1.0f;
		parentMatrix.fB = 0.0f;
		parentMatrix.fC = 0.0f;
		parentMatrix.fD = 1.0f;
		parentMatrix.fE = 100.0f;
		parentMatrix.fF = 50.0f;
		if ( !check(xgeShapeExIntersectsEx(intersectsShape, (xge_rect_t){102.0f, 52.0f, 2.0f, 2.0f}, 0.05f, &parentMatrix, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects parent transform") ) goto shape_ex_intersects_cleanup;
		parentMatrix.fA = invalidNaN;
		if ( !check(xgeShapeExIntersectsEx(intersectsShape, (xge_rect_t){2.0f, 2.0f, 2.0f, 2.0f}, 0.05f, &parentMatrix, &intersects) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx precise intersects rejects invalid parent transform") ) goto shape_ex_intersects_cleanup;

		if ( !check(xgeShapeExReset(intersectsShape) == XGE_OK, "ShapeEx precise intersects donut reset") ||
		     !check(xgeShapeExFillRule(intersectsShape, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx precise intersects even odd") ||
		     !check(xgeShapeExAppendRect(intersectsShape, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx precise intersects donut outer") ||
		     !check(xgeShapeExAppendRect(intersectsShape, 5.0f, 5.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx precise intersects donut inner") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){8.0f, 8.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise intersects even-odd hole") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){1.0f, 1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects even-odd ring") ) goto shape_ex_intersects_cleanup;

		if ( !check(xgeShapeExReset(intersectsShape) == XGE_OK, "ShapeEx precise intersects stroke reset") ||
		     !check(xgeShapeExFillColor(intersectsShape, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx precise intersects transparent fill") ||
		     !check(xgeShapeExStrokeColor(intersectsShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx precise intersects stroke color") ||
		     !check(xgeShapeExStrokeWidth(intersectsShape, 4.0f) == XGE_OK, "ShapeEx precise intersects stroke width") ||
		     !check(xgeShapeExMoveTo(intersectsShape, 0.0f, 0.0f) == XGE_OK, "ShapeEx precise intersects stroke move") ||
		     !check(xgeShapeExLineTo(intersectsShape, 20.0f, 0.0f) == XGE_OK, "ShapeEx precise intersects stroke line") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects stroke hit") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){8.0f, 4.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise intersects stroke miss") ||
		     !check(xgeShapeExVisible(intersectsShape, 0) == XGE_OK, "ShapeEx precise intersects hidden") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise intersects ignores visibility like ThorVG") ||
		     !check(xgeShapeExOpacity(intersectsShape, 0.0f) == XGE_OK, "ShapeEx precise intersects zero opacity") ||
		     !check(xgeShapeExIntersects(intersectsShape, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise intersects respects opacity") ||
		     !check(xgeShapeExOpacity(intersectsShape, 1.0f) == XGE_OK, "ShapeEx precise intersects restore opacity") ) goto shape_ex_intersects_cleanup;

		ret = xgeShapeExSceneCreate(&intersectsScene);
		if ( !check((ret == XGE_OK) && (intersectsScene != NULL), "ShapeEx precise scene intersects create") ||
		     !check(xgeShapeExSceneAdd(intersectsScene, intersectsShape) == XGE_OK, "ShapeEx precise scene intersects add") ||
		     !check(xgeShapeExSceneIntersects(intersectsScene, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 1, "ShapeEx precise scene intersects hit") ||
		     !check(xgeShapeExSceneIntersects(intersectsScene, (xge_rect_t){8.0f, 4.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise scene intersects miss") ||
		     !check(xgeShapeExSceneIntersects(intersectsScene, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx precise scene intersects rejects null output") ||
		     !check(xgeShapeExSceneOpacity(intersectsScene, 0.0f) == XGE_OK, "ShapeEx precise scene intersects zero opacity") ||
		     !check(xgeShapeExSceneIntersects(intersectsScene, (xge_rect_t){8.0f, -1.0f, 2.0f, 2.0f}, 0.05f, &intersects) == XGE_OK && intersects == 0, "ShapeEx precise scene intersects respects opacity") ) goto shape_ex_intersects_cleanup;
		intersectsOk = 1;

shape_ex_intersects_cleanup:
		xgeShapeExSceneDestroy(intersectsScene);
		xgeShapeExDestroy(intersectsShape);
		if ( !intersectsOk ) return 0;
	}
	{
		xge_shape_ex skewShape = NULL;
		xge_shape_ex_scene skewScene = NULL;
		xge_vec2_t skewObb[4];

		ret = xgeShapeExCreate(&skewShape);
		if ( !check((ret == XGE_OK) && (skewShape != NULL), "ShapeEx skew obb shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(skewShape, 0.0f, 0.0f, 4.0f, 2.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx skew obb rect") ) {
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformSkew(skewShape, 0.4636476090008061f, 0.0f) == XGE_OK, "ShapeEx skew obb transform") ) {
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		ret = xgeShapeExGetOBB(skewShape, 0.05f, skewObb);
		if ( !check(ret == XGE_OK, "ShapeEx skew obb get") ||
		     !check((skewObb[0].fX > -0.01f) && (skewObb[0].fX < 0.01f) && (skewObb[0].fY > -0.01f) && (skewObb[0].fY < 0.01f), "ShapeEx skew obb point 0") ||
		     !check((skewObb[1].fX > 3.99f) && (skewObb[1].fX < 4.01f) && (skewObb[1].fY > -0.01f) && (skewObb[1].fY < 0.01f), "ShapeEx skew obb point 1") ||
		     !check((skewObb[2].fX > 4.99f) && (skewObb[2].fX < 5.01f) && (skewObb[2].fY > 1.99f) && (skewObb[2].fY < 2.01f), "ShapeEx skew obb point 2") ||
		     !check((skewObb[3].fX > 0.99f) && (skewObb[3].fX < 1.01f) && (skewObb[3].fY > 1.99f) && (skewObb[3].fY < 2.01f), "ShapeEx skew obb point 3") ) {
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		if ( !check(xgeShapeExTransformIdentity(skewShape) == XGE_OK, "ShapeEx skew scene obb reset shape") ) {
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&skewScene);
		if ( !check((ret == XGE_OK) && (skewScene != NULL), "ShapeEx skew scene obb create") ) {
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		if ( !check(xgeShapeExSceneAdd(skewScene, skewShape) == XGE_OK, "ShapeEx skew scene obb add") ||
		     !check(xgeShapeExSceneTransformSkew(skewScene, 0.4636476090008061f, 0.24497866312686414f) == XGE_OK, "ShapeEx skew scene obb transform") ) {
			xgeShapeExSceneDestroy(skewScene);
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		ret = xgeShapeExSceneGetOBB(skewScene, 0.05f, skewObb);
		if ( !check(ret == XGE_OK, "ShapeEx skew scene obb get") ||
		     !check((skewObb[0].fX > -0.01f) && (skewObb[0].fX < 0.01f) && (skewObb[0].fY > -0.01f) && (skewObb[0].fY < 0.01f), "ShapeEx skew scene obb point 0") ||
		     !check((skewObb[1].fX > 3.99f) && (skewObb[1].fX < 4.01f) && (skewObb[1].fY > 0.99f) && (skewObb[1].fY < 1.01f), "ShapeEx skew scene obb point 1") ||
		     !check((skewObb[2].fX > 4.99f) && (skewObb[2].fX < 5.01f) && (skewObb[2].fY > 2.99f) && (skewObb[2].fY < 3.01f), "ShapeEx skew scene obb point 2") ||
		     !check((skewObb[3].fX > 0.99f) && (skewObb[3].fX < 1.01f) && (skewObb[3].fY > 1.99f) && (skewObb[3].fY < 2.01f), "ShapeEx skew scene obb point 3") ) {
			xgeShapeExSceneDestroy(skewScene);
			xgeShapeExDestroy(skewShape);
			return 0;
		}
		xgeShapeExSceneDestroy(skewScene);
		xgeShapeExDestroy(skewShape);
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
		            (strokeBounds.fX > 15.99f) && (strokeBounds.fX < 16.01f) &&
		            (strokeBounds.fY > 23.99f) && (strokeBounds.fY < 24.01f) &&
		            (strokeBounds.fW > 47.99f) && (strokeBounds.fW < 48.01f) &&
		            (strokeBounds.fH > 41.99f) && (strokeBounds.fH < 42.01f), "ShapeEx stroke bounds follows anisotropic transform") ) {
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
		xge_shape_ex affineShape = NULL;
		xge_shape_ex squareCapShape = NULL;
		xge_rect_t affineBounds;
		xge_rect_t squareCapBounds;
		xge_shape_ex_matrix_t affineMatrix = {2.0f, 0.5f, 1.0f, 3.0f, 7.0f, -4.0f};

		ret = xgeShapeExCreate(&affineShape);
		if ( !check((ret == XGE_OK) && (affineShape != NULL), "ShapeEx affine stroke bounds create") ) return 0;
		if ( !check(xgeShapeExAppendRect(affineShape, 0.0f, 0.0f, 10.0f, 5.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx affine stroke bounds rect") ||
		     !check(xgeShapeExStrokeColor(affineShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx affine stroke bounds color") ||
		     !check(xgeShapeExStrokeWidth(affineShape, 4.0f) == XGE_OK, "ShapeEx affine stroke bounds width") ||
		     !check(xgeShapeExStrokeJoin(affineShape, XGE_SHAPE_EX_JOIN_ROUND) == XGE_OK, "ShapeEx affine stroke bounds round join") ||
		     !check(xgeShapeExTransformSet(affineShape, &affineMatrix) == XGE_OK, "ShapeEx affine stroke bounds transform") ) {
			xgeShapeExDestroy(affineShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(affineShape, 0.05f, &affineBounds);
		if ( !check((ret == XGE_OK) &&
		            (affineBounds.fX > 2.52f) && (affineBounds.fX < 2.54f) &&
		            (affineBounds.fY > -10.09f) && (affineBounds.fY < -10.07f) &&
		            (affineBounds.fW > 33.93f) && (affineBounds.fW < 33.96f) &&
		            (affineBounds.fH > 32.15f) && (affineBounds.fH < 32.18f), "ShapeEx affine round stroke bounds") ) {
			xgeShapeExDestroy(affineShape);
			return 0;
		}
		xgeShapeExDestroy(affineShape);

		ret = xgeShapeExCreate(&squareCapShape);
		if ( !check((ret == XGE_OK) && (squareCapShape != NULL), "ShapeEx affine square cap bounds create") ) return 0;
		if ( !check(xgeShapeExAppendSvgPath(squareCapShape, "M0 0 L10 0") == XGE_OK, "ShapeEx affine square cap bounds path") ||
		     !check(xgeShapeExStrokeColor(squareCapShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx affine square cap bounds color") ||
		     !check(xgeShapeExStrokeWidth(squareCapShape, 4.0f) == XGE_OK, "ShapeEx affine square cap bounds width") ||
		     !check(xgeShapeExStrokeCap(squareCapShape, XGE_SHAPE_EX_CAP_SQUARE) == XGE_OK, "ShapeEx affine square cap bounds mode") ||
		     !check(xgeShapeExTransformSet(squareCapShape, &affineMatrix) == XGE_OK, "ShapeEx affine square cap bounds transform") ) {
			xgeShapeExDestroy(squareCapShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(squareCapShape, 0.05f, &squareCapBounds);
		if ( !check((ret == XGE_OK) &&
		            (squareCapBounds.fX > 2.52f) && (squareCapBounds.fX < 2.54f) &&
		            (squareCapBounds.fY > -10.09f) && (squareCapBounds.fY < -10.07f) &&
		            (squareCapBounds.fW > 28.93f) && (squareCapBounds.fW < 28.96f) &&
		            (squareCapBounds.fH > 17.15f) && (squareCapBounds.fH < 17.18f), "ShapeEx affine square cap logical bounds") ) {
			xgeShapeExDestroy(squareCapShape);
			return 0;
		}
		xgeShapeExDestroy(squareCapShape);
	}
	{
		xge_shape_ex miterShape = NULL;
		xge_rect_t miterBounds;

		ret = xgeShapeExCreate(&miterShape);
		if ( !check((ret == XGE_OK) && (miterShape != NULL), "ShapeEx miter stroke bounds create") ) return 0;
		if ( !check(xgeShapeExAppendSvgPath(miterShape, "M0 10 L10 0 L20 10") == XGE_OK, "ShapeEx miter stroke bounds path") ) {
			xgeShapeExDestroy(miterShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeColor(miterShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx miter stroke bounds color") ||
		     !check(xgeShapeExStrokeWidth(miterShape, 4.0f) == XGE_OK, "ShapeEx miter stroke bounds width") ||
		     !check(xgeShapeExStrokeCap(miterShape, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx miter stroke bounds cap") ||
		     !check(xgeShapeExStrokeJoin(miterShape, XGE_SHAPE_EX_JOIN_MITER) == XGE_OK, "ShapeEx miter stroke bounds join") ||
		     !check(xgeShapeExStrokeMiterLimit(miterShape, 8.0f) == XGE_OK, "ShapeEx miter stroke bounds limit") ) {
			xgeShapeExDestroy(miterShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(miterShape, 0.05f, &miterBounds);
		if ( !check((ret == XGE_OK) &&
		            (miterBounds.fY < -1.99f) && (miterBounds.fY > -2.01f), "ShapeEx miter stroke logical bounds") ) {
			xgeShapeExDestroy(miterShape);
			return 0;
		}
		xgeShapeExDestroy(miterShape);
	}
	{
		xge_shape_ex mixedMiterShape = NULL;
		xge_rect_t mixedMiterBounds;

		ret = xgeShapeExCreate(&mixedMiterShape);
		if ( !check((ret == XGE_OK) && (mixedMiterShape != NULL), "ShapeEx mixed miter bounds create") ) return 0;
		if ( !check(xgeShapeExAppendSvgPath(mixedMiterShape, "M0 10 L10 0 L20 10 L30 20 L40 24") == XGE_OK, "ShapeEx mixed miter bounds path") ) {
			xgeShapeExDestroy(mixedMiterShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeColor(mixedMiterShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx mixed miter bounds color") ||
		     !check(xgeShapeExStrokeWidth(mixedMiterShape, 4.0f) == XGE_OK, "ShapeEx mixed miter bounds width") ||
		     !check(xgeShapeExStrokeCap(mixedMiterShape, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx mixed miter bounds cap") ||
		     !check(xgeShapeExStrokeJoin(mixedMiterShape, XGE_SHAPE_EX_JOIN_MITER) == XGE_OK, "ShapeEx mixed miter bounds join") ||
		     !check(xgeShapeExStrokeMiterLimit(mixedMiterShape, 3.0f) == XGE_OK, "ShapeEx mixed miter bounds limit") ) {
			xgeShapeExDestroy(mixedMiterShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(mixedMiterShape, 0.05f, &mixedMiterBounds);
		if ( !check((ret == XGE_OK) &&
		            (mixedMiterBounds.fY < -1.99f) && (mixedMiterBounds.fY > -2.01f), "ShapeEx mixed miter logical bounds") ) {
			xgeShapeExDestroy(mixedMiterShape);
			return 0;
		}
		xgeShapeExDestroy(mixedMiterShape);
	}
	{
		xge_shape_ex turnMiterShape = NULL;
		xge_vec2_t turnPoints[3] = {
			{0.0f, 10.0f},
			{10.0f, 0.0f},
			{20.0f, 10.0f}
		};
		xge_rect_t turnBounds;
		int turnHit = -1;

		ret = xgeShapeExCreate(&turnMiterShape);
		if ( !check((ret == XGE_OK) && (turnMiterShape != NULL), "ShapeEx turn miter create") ) return 0;
		if ( !check(xgeShapeExAppendPolygon(turnMiterShape, turnPoints, 3) == XGE_OK, "ShapeEx turn miter polygon") ||
		     !check(xgeShapeExFillColor(turnMiterShape, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx turn miter transparent fill") ||
		     !check(xgeShapeExStrokeColor(turnMiterShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx turn miter color") ||
		     !check(xgeShapeExStrokeWidth(turnMiterShape, 4.0f) == XGE_OK, "ShapeEx turn miter width") ||
		     !check(xgeShapeExStrokeCap(turnMiterShape, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx turn miter cap") ||
		     !check(xgeShapeExStrokeJoin(turnMiterShape, XGE_SHAPE_EX_JOIN_MITER) == XGE_OK, "ShapeEx turn miter join") ||
		     !check(xgeShapeExStrokeMiterLimit(turnMiterShape, 2.0f) == XGE_OK, "ShapeEx turn miter bevel limit") ) {
			xgeShapeExDestroy(turnMiterShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(turnMiterShape, 0.05f, &turnBounds);
		if ( !check((ret == XGE_OK) && (turnBounds.fX > -2.01f) && (turnBounds.fX < -1.99f), "ShapeEx turn miter exceeded limit uses bevel bounds") ||
		     !check(xgeShapeExContainsPoint(turnMiterShape, -3.0f, 11.0f, 0.01f, &turnHit) == XGE_OK, "ShapeEx turn bevel hit query") ||
		     !check(!turnHit, "ShapeEx turn bevel excludes miter tip") ||
		     !check(xgeShapeExStrokeMiterLimit(turnMiterShape, 3.0f) == XGE_OK, "ShapeEx turn miter accepted limit") ) {
			xgeShapeExDestroy(turnMiterShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(turnMiterShape, 0.05f, &turnBounds);
		if ( !check((ret == XGE_OK) && (turnBounds.fX > -2.01f) && (turnBounds.fX < -1.99f), "ShapeEx turn miter accepted limit keeps logical bounds") ||
		     !check(xgeShapeExContainsPoint(turnMiterShape, -3.0f, 11.0f, 0.01f, &turnHit) == XGE_OK, "ShapeEx turn miter hit query") ||
		     !check(turnHit, "ShapeEx turn miter includes tip") ||
		     !check(xgeShapeExContainsPoint(turnMiterShape, 10.0f, -1.0f, 0.01f, &turnHit) == XGE_OK, "ShapeEx turn miter center wedge hit query") ||
		     !check(turnHit, "ShapeEx turn miter includes center wedge") ) {
			xgeShapeExDestroy(turnMiterShape);
			return 0;
		}
		xgeShapeExDestroy(turnMiterShape);
	}
	{
		xge_shape_ex squareCapShape = NULL;
		xge_rect_t squareCapBounds;

		ret = xgeShapeExCreate(&squareCapShape);
		if ( !check((ret == XGE_OK) && (squareCapShape != NULL), "ShapeEx square cap bounds create") ) return 0;
		if ( !check(xgeShapeExAppendSvgPath(squareCapShape, "M0 0 L10 10") == XGE_OK, "ShapeEx square cap bounds path") ) {
			xgeShapeExDestroy(squareCapShape);
			return 0;
		}
		if ( !check(xgeShapeExStrokeColor(squareCapShape, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx square cap bounds color") ||
		     !check(xgeShapeExStrokeWidth(squareCapShape, 4.0f) == XGE_OK, "ShapeEx square cap bounds width") ||
		     !check(xgeShapeExStrokeCap(squareCapShape, XGE_SHAPE_EX_CAP_SQUARE) == XGE_OK, "ShapeEx square cap bounds cap") ) {
			xgeShapeExDestroy(squareCapShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(squareCapShape, 0.05f, &squareCapBounds);
		if ( !check((ret == XGE_OK) &&
		            (squareCapBounds.fX < -1.99f) && (squareCapBounds.fX > -2.01f) &&
		            (squareCapBounds.fY < -1.99f) && (squareCapBounds.fY > -2.01f) &&
		            (squareCapBounds.fW > 13.99f) && (squareCapBounds.fW < 14.01f) &&
		            (squareCapBounds.fH > 13.99f) && (squareCapBounds.fH < 14.01f), "ShapeEx square cap logical diagonal bounds") ) {
			xgeShapeExDestroy(squareCapShape);
			return 0;
		}
		xgeShapeExDestroy(squareCapShape);
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
		if ( !check((ret == XGE_ERROR_INVALID_STATE) &&
		            (orderBounds.fX == 0.0f) && (orderBounds.fY == 0.0f) &&
		            (orderBounds.fW == 0.0f) && (orderBounds.fH == 0.0f), "ShapeEx scene remove null clears bounds state") ) {
			xgeShapeExSceneDestroy(orderScene);
			xgeShapeExDestroy(sceneMissing);
			xgeShapeExDestroy(sceneC);
			xgeShapeExDestroy(sceneB);
			xgeShapeExDestroy(sceneA);
			return 0;
		}
		{
			xge_vec2_t emptyObb[4] = {
				{1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}
			};

			ret = xgeShapeExSceneGetOBB(orderScene, 0.05f, emptyObb);
			if ( !check((ret == XGE_ERROR_INVALID_STATE) &&
			            (emptyObb[0].fX == 0.0f) && (emptyObb[0].fY == 0.0f) &&
			            (emptyObb[3].fX == 0.0f) && (emptyObb[3].fY == 0.0f), "ShapeEx scene remove null clears obb state") ) {
				xgeShapeExSceneDestroy(orderScene);
				xgeShapeExDestroy(sceneMissing);
				xgeShapeExDestroy(sceneC);
				xgeShapeExDestroy(sceneB);
				xgeShapeExDestroy(sceneA);
				return 0;
			}
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
		xge_shape_ex sceneBack = NULL;
		xge_shape_ex sceneFront = NULL;
		xge_shape_ex sceneStroke = NULL;
		xge_shape_ex sceneClip = NULL;
		xge_shape_ex sceneClip2 = NULL;
		xge_shape_ex hitShape = NULL;
		xge_shape_ex_scene hitScene = NULL;
		xge_shape_ex_matrix_t parentMatrix;
		int contains = 0;
		int ok = 0;

		if ( !check(xgeShapeExSceneContainsPoint(NULL, 0.0f, 0.0f, 0.05f, &contains) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene contains rejects null scene") ) return 0;
		ret = xgeShapeExSceneCreate(&hitScene);
		if ( !check((ret == XGE_OK) && (hitScene != NULL), "ShapeEx scene hit create") ) goto shape_ex_scene_hit_cleanup;
		ret = xgeShapeExCreate(&sceneBack);
		if ( !check((ret == XGE_OK) && (sceneBack != NULL), "ShapeEx scene hit back create") ) goto shape_ex_scene_hit_cleanup;
		ret = xgeShapeExCreate(&sceneFront);
		if ( !check((ret == XGE_OK) && (sceneFront != NULL), "ShapeEx scene hit front create") ) goto shape_ex_scene_hit_cleanup;
		if ( !check(xgeShapeExFillColor(sceneBack, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx scene hit back fill") ||
		     !check(xgeShapeExFillColor(sceneFront, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx scene hit front fill") ||
		     !check(xgeShapeExAppendRect(sceneBack, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit back rect") ||
		     !check(xgeShapeExAppendRect(sceneFront, 5.0f, 5.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit front rect") ||
		     !check(xgeShapeExSceneAdd(hitScene, sceneBack) == XGE_OK, "ShapeEx scene hit add back") ||
		     !check(xgeShapeExSceneAdd(hitScene, sceneFront) == XGE_OK, "ShapeEx scene hit add front") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit topmost shape") ||
		     !check((xgeShapeExSceneContainsPoint(hitScene, 6.0f, 6.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx scene contains topmost point") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 1.0f, 1.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneBack), "ShapeEx scene hit back shape") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 40.0f, 40.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit miss") ||
		     !check((xgeShapeExSceneContainsPoint(hitScene, 40.0f, 40.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx scene contains miss") ||
		     !check(xgeShapeExSceneContainsPoint(hitScene, 6.0f, 6.0f, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene contains rejects null output") ||
		     !check(xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, NULL) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene hit rejects null output") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExVisible(sceneFront, 0) == XGE_OK, "ShapeEx scene hit front hidden") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneBack), "ShapeEx scene hit skips hidden shape") ||
		     !check(xgeShapeExVisible(sceneFront, 1) == XGE_OK, "ShapeEx scene hit front restore visible") ||
		     !check(xgeShapeExOpacity(sceneFront, 0.0f) == XGE_OK, "ShapeEx scene hit front transparent") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneBack), "ShapeEx scene hit skips transparent shape") ||
		     !check(xgeShapeExOpacity(sceneFront, 1.0f) == XGE_OK, "ShapeEx scene hit front restore opacity") ||
		     !check(xgeShapeExFillColor(sceneFront, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx scene hit front transparent fill") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneBack), "ShapeEx scene hit skips transparent fill") ||
		     !check(xgeShapeExFillColor(sceneFront, XGE_COLOR_RGBA(0, 0, 0, 255)) == XGE_OK, "ShapeEx scene hit front restore fill") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		ret = xgeShapeExCreate(&sceneStroke);
		if ( !check((ret == XGE_OK) && (sceneStroke != NULL), "ShapeEx scene hit stroke-only create") ) goto shape_ex_scene_hit_cleanup;
		if ( !check(xgeShapeExAppendRect(sceneStroke, 30.0f, 5.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit stroke-only rect") ||
		     !check(xgeShapeExFillColor(sceneStroke, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx scene hit stroke-only transparent fill") ||
		     !check(xgeShapeExStrokeColor(sceneStroke, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx scene hit stroke-only color") ||
		     !check(xgeShapeExStrokeWidth(sceneStroke, 2.0f) == XGE_OK, "ShapeEx scene hit stroke-only width") ||
		     !check(xgeShapeExSceneAdd(hitScene, sceneStroke) == XGE_OK, "ShapeEx scene hit add stroke-only") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 30.5f, 10.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneStroke), "ShapeEx scene hit stroke-only shape") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 35.0f, 10.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit stroke-only center miss") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExSceneVisible(hitScene, 0) == XGE_OK, "ShapeEx scene hit scene hidden") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit skips hidden scene") ||
		     !check(xgeShapeExSceneVisible(hitScene, 1) == XGE_OK, "ShapeEx scene hit scene restore visible") ||
		     !check(xgeShapeExSceneOpacity(hitScene, 0.0f) == XGE_OK, "ShapeEx scene hit scene transparent") ||
		     !check((xgeShapeExSceneContainsPoint(hitScene, 6.0f, 6.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx scene contains skips transparent scene") ||
		     !check(xgeShapeExSceneOpacity(hitScene, 1.0f) == XGE_OK, "ShapeEx scene hit scene restore opacity") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExSceneTransformTranslate(hitScene, 10.0f, 20.0f) == XGE_OK, "ShapeEx scene hit scene transform") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 16.0f, 26.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit transformed topmost") ||
		     !check((xgeShapeExSceneContainsPoint(hitScene, 6.0f, 6.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx scene contains transformed miss") ||
		     !check(xgeShapeExSceneTransformIdentity(hitScene) == XGE_OK, "ShapeEx scene hit scene transform reset") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExMatrixTranslate(&parentMatrix, 100.0f, 200.0f) == XGE_OK, "ShapeEx scene hit parent matrix") ||
		     !check((xgeShapeExSceneHitTestEx(hitScene, 106.0f, 206.0f, 0.05f, &parentMatrix, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit parent transformed topmost") ||
		     !check((xgeShapeExSceneContainsPointEx(hitScene, 106.0f, 206.0f, 0.05f, &parentMatrix, &contains) == XGE_OK) && contains, "ShapeEx scene contains parent transformed") ||
		     !check((xgeShapeExSceneHitTestEx(hitScene, 6.0f, 6.0f, 0.05f, &parentMatrix, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit parent transformed miss") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		parentMatrix.fA = invalidNaN;
		if ( !check(xgeShapeExSceneHitTestEx(hitScene, 106.0f, 206.0f, 0.05f, &parentMatrix, &hitShape) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene hit rejects invalid parent matrix") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExSceneClipRectSet(hitScene, (xge_rect_t){0.0f, 0.0f, 10.0f, 10.0f}) == XGE_OK, "ShapeEx scene hit clip rect") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit inside clip rect") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 12.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit outside clip rect") ||
		     !check(xgeShapeExSceneClipClear(hitScene) == XGE_OK, "ShapeEx scene hit clip clear") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		ret = xgeShapeExCreate(&sceneClip);
		if ( !check((ret == XGE_OK) && (sceneClip != NULL), "ShapeEx scene hit clip create") ) goto shape_ex_scene_hit_cleanup;
		ret = xgeShapeExCreate(&sceneClip2);
		if ( !check((ret == XGE_OK) && (sceneClip2 != NULL), "ShapeEx scene hit clip union create") ) goto shape_ex_scene_hit_cleanup;
		if ( !check(xgeShapeExAppendRect(sceneClip, 0.0f, 0.0f, 8.0f, 8.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit clip rect path") ||
		     !check(xgeShapeExAppendRect(sceneClip2, 9.0f, 5.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit clip union path") ||
		     !check(xgeShapeExSceneClipShapeAdd(hitScene, sceneClip) == XGE_OK, "ShapeEx scene hit clip shape add") ||
		     !check(xgeShapeExSceneClipShapeAdd(hitScene, sceneClip2) == XGE_OK, "ShapeEx scene hit clip union add") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 6.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit inside clip shape") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 10.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit inside clip shape union") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 14.0f, 6.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit outside clip shape") ||
		     !check(xgeShapeExSceneClipClear(hitScene) == XGE_OK, "ShapeEx scene hit clip shape clear") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		if ( !check(xgeShapeExReset(sceneClip) == XGE_OK, "ShapeEx scene hit subtract clip reset") ||
		     !check(xgeShapeExAppendRect(sceneClip, 3.0f, 3.0f, 4.0f, 4.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx scene hit subtract clip path") ||
		     !check(xgeShapeExSceneClipShapeAddEx(hitScene, sceneClip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx scene hit subtract clip add") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 8.0f, 8.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == sceneFront), "ShapeEx scene hit subtract outside cut") ||
		     !check((xgeShapeExSceneHitTest(hitScene, 4.0f, 4.0f, 0.05f, &hitShape) == XGE_OK) && (hitShape == NULL), "ShapeEx scene hit subtract inside cut") ||
		     !check(xgeShapeExSceneClipClear(hitScene) == XGE_OK, "ShapeEx scene hit subtract clip clear") ) {
			goto shape_ex_scene_hit_cleanup;
		}
		ok = 1;

shape_ex_scene_hit_cleanup:
		xgeShapeExSceneDestroy(hitScene);
		xgeShapeExDestroy(sceneClip2);
		xgeShapeExDestroy(sceneClip);
		xgeShapeExDestroy(sceneStroke);
		xgeShapeExDestroy(sceneFront);
		xgeShapeExDestroy(sceneBack);
		if ( !ok ) return 0;
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
		xge_shape_ex clippedBoundsShape = NULL;
		xge_shape_ex_scene clippedBoundsScene = NULL;
		xge_rect_t clippedBounds;
		xge_vec2_t clippedObb[4];

		ret = xgeShapeExCreate(&clippedBoundsShape);
		if ( !check((ret == XGE_OK) && (clippedBoundsShape != NULL), "ShapeEx clipped bounds shape create") ) return 0;
		if ( !check(xgeShapeExAppendRect(clippedBoundsShape, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clipped bounds rect") ||
		     !check(xgeShapeExClipRectSet(clippedBoundsShape, (xge_rect_t){5.0f, 6.0f, 8.0f, 9.0f}) == XGE_OK, "ShapeEx clipped bounds clip rect") ) {
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		ret = xgeShapeExGetBounds(clippedBoundsShape, 0.05f, &clippedBounds);
		if ( !check((ret == XGE_OK) && (clippedBounds.fX > -0.1f) && (clippedBounds.fX < 0.1f) && (clippedBounds.fY > -0.1f) && (clippedBounds.fY < 0.1f) && (clippedBounds.fW > 19.9f) && (clippedBounds.fW < 20.1f) && (clippedBounds.fH > 19.9f) && (clippedBounds.fH < 20.1f), "ShapeEx clipped logical bounds value") ) {
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		ret = xgeShapeExGetOBB(clippedBoundsShape, 0.05f, clippedObb);
		if ( !check(ret == XGE_OK, "ShapeEx clipped obb get") ||
		     !check((clippedObb[0].fX > -0.1f) && (clippedObb[0].fX < 0.1f) && (clippedObb[0].fY > -0.1f) && (clippedObb[0].fY < 0.1f), "ShapeEx clipped logical obb point 0") ||
		     !check((clippedObb[2].fX > 19.9f) && (clippedObb[2].fX < 20.1f) && (clippedObb[2].fY > 19.9f) && (clippedObb[2].fY < 20.1f), "ShapeEx clipped logical obb point 2") ) {
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&clippedBoundsScene);
		if ( !check((ret == XGE_OK) && (clippedBoundsScene != NULL), "ShapeEx clipped scene bounds create") ) {
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		matrix.fA = 1.0f;
		matrix.fB = 0.0f;
		matrix.fC = 0.0f;
		matrix.fD = 1.0f;
		matrix.fE = 10.0f;
		matrix.fF = 20.0f;
		if ( !check(xgeShapeExSceneAdd(clippedBoundsScene, clippedBoundsShape) == XGE_OK, "ShapeEx clipped scene bounds add") ||
		     !check(xgeShapeExSceneTransformSet(clippedBoundsScene, &matrix) == XGE_OK, "ShapeEx clipped scene bounds transform") ||
		     !check(xgeShapeExSceneClipRectSet(clippedBoundsScene, (xge_rect_t){2.0f, 3.0f, 10.0f, 10.0f}) == XGE_OK, "ShapeEx clipped scene bounds clip rect") ) {
			xgeShapeExSceneDestroy(clippedBoundsScene);
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		ret = xgeShapeExSceneGetBounds(clippedBoundsScene, 0.05f, &clippedBounds);
		if ( !check((ret == XGE_OK) && (clippedBounds.fX > 9.9f) && (clippedBounds.fX < 10.1f) && (clippedBounds.fY > 19.9f) && (clippedBounds.fY < 20.1f) && (clippedBounds.fW > 19.9f) && (clippedBounds.fW < 20.1f) && (clippedBounds.fH > 19.9f) && (clippedBounds.fH < 20.1f), "ShapeEx clipped scene logical bounds value") ) {
			xgeShapeExSceneDestroy(clippedBoundsScene);
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		ret = xgeShapeExSceneGetOBB(clippedBoundsScene, 0.05f, clippedObb);
		if ( !check(ret == XGE_OK, "ShapeEx clipped scene obb get") ||
		     !check((clippedObb[0].fX > 9.9f) && (clippedObb[0].fX < 10.1f) && (clippedObb[0].fY > 19.9f) && (clippedObb[0].fY < 20.1f), "ShapeEx clipped scene logical obb point 0") ||
		     !check((clippedObb[2].fX > 29.9f) && (clippedObb[2].fX < 30.1f) && (clippedObb[2].fY > 39.9f) && (clippedObb[2].fY < 40.1f), "ShapeEx clipped scene logical obb point 2") ) {
			xgeShapeExSceneDestroy(clippedBoundsScene);
			xgeShapeExDestroy(clippedBoundsShape);
			return 0;
		}
		xgeShapeExSceneDestroy(clippedBoundsScene);
		xgeShapeExDestroy(clippedBoundsShape);
	}
	{
		xge_shape_ex clipShapeBoundsTarget = NULL;
		xge_shape_ex clipShapeBoundsClip = NULL;
		xge_shape_ex clipShapeBoundsClip2 = NULL;
		xge_shape_ex_scene clipShapeBoundsScene = NULL;
		xge_rect_t clipShapeBounds;
		xge_vec2_t clipShapeObb[4];

		ret = xgeShapeExCreate(&clipShapeBoundsTarget);
		if ( !check((ret == XGE_OK) && (clipShapeBoundsTarget != NULL), "ShapeEx clip shape bounds target create") ) return 0;
		ret = xgeShapeExCreate(&clipShapeBoundsClip);
		if ( !check((ret == XGE_OK) && (clipShapeBoundsClip != NULL), "ShapeEx clip shape bounds clip create") ) {
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExCreate(&clipShapeBoundsClip2);
		if ( !check((ret == XGE_OK) && (clipShapeBoundsClip2 != NULL), "ShapeEx clip shape bounds clip union create") ) {
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(clipShapeBoundsTarget, 0.0f, 0.0f, 20.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip shape bounds target rect") ||
		     !check(xgeShapeExAppendRect(clipShapeBoundsClip, 4.0f, 5.0f, 6.0f, 7.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip shape bounds clip rect") ||
		     !check(xgeShapeExAppendRect(clipShapeBoundsClip2, 12.0f, 5.0f, 4.0f, 7.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx clip shape bounds clip union rect") ||
		     !check(xgeShapeExClipShapeAdd(clipShapeBoundsTarget, clipShapeBoundsClip) == XGE_OK, "ShapeEx clip shape bounds add") ||
		     !check(xgeShapeExClipShapeAdd(clipShapeBoundsTarget, clipShapeBoundsClip2) == XGE_OK, "ShapeEx clip shape bounds union add") ) {
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExGetBounds(clipShapeBoundsTarget, 0.05f, &clipShapeBounds);
		if ( !check((ret == XGE_OK) && (clipShapeBounds.fX > -0.1f) && (clipShapeBounds.fX < 0.1f) && (clipShapeBounds.fY > -0.1f) && (clipShapeBounds.fY < 0.1f) && (clipShapeBounds.fW > 19.9f) && (clipShapeBounds.fW < 20.1f) && (clipShapeBounds.fH > 19.9f) && (clipShapeBounds.fH < 20.1f), "ShapeEx clip shape logical bounds value") ) {
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExGetOBB(clipShapeBoundsTarget, 0.05f, clipShapeObb);
		if ( !check(ret == XGE_OK, "ShapeEx clip shape obb get") ||
		     !check((clipShapeObb[0].fX > -0.1f) && (clipShapeObb[0].fX < 0.1f) && (clipShapeObb[0].fY > -0.1f) && (clipShapeObb[0].fY < 0.1f), "ShapeEx clip shape logical obb point 0") ||
		     !check((clipShapeObb[2].fX > 19.9f) && (clipShapeObb[2].fX < 20.1f) && (clipShapeObb[2].fY > 19.9f) && (clipShapeObb[2].fY < 20.1f), "ShapeEx clip shape logical obb point 2") ) {
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		if ( !check(xgeShapeExClipShapeClear(clipShapeBoundsTarget) == XGE_OK, "ShapeEx subtract clip shape bounds target clear") ||
		     !check(xgeShapeExClipShapeAddEx(clipShapeBoundsTarget, clipShapeBoundsClip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx subtract clip shape bounds add") ||
		     !check(xgeShapeExGetBounds(clipShapeBoundsTarget, 0.05f, &clipShapeBounds) == XGE_OK, "ShapeEx subtract clip shape bounds get") ||
		     !check((clipShapeBounds.fX > -0.1f) && (clipShapeBounds.fX < 0.1f) && (clipShapeBounds.fY > -0.1f) && (clipShapeBounds.fY < 0.1f) &&
		            (clipShapeBounds.fW > 19.9f) && (clipShapeBounds.fW < 20.1f) && (clipShapeBounds.fH > 19.9f) && (clipShapeBounds.fH < 20.1f), "ShapeEx subtract clip shape bounds conservative value") ||
		     !check(xgeShapeExGetOBB(clipShapeBoundsTarget, 0.05f, clipShapeObb) == XGE_OK, "ShapeEx subtract clip shape obb get") ||
		     !check((clipShapeObb[0].fX > -0.1f) && (clipShapeObb[0].fX < 0.1f) && (clipShapeObb[0].fY > -0.1f) && (clipShapeObb[0].fY < 0.1f), "ShapeEx subtract clip shape obb conservative point 0") ) {
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExSceneCreate(&clipShapeBoundsScene);
		if ( !check((ret == XGE_OK) && (clipShapeBoundsScene != NULL), "ShapeEx scene clip shape bounds create") ) {
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		matrix.fA = 1.0f;
		matrix.fB = 0.0f;
		matrix.fC = 0.0f;
		matrix.fD = 1.0f;
		matrix.fE = 10.0f;
		matrix.fF = 20.0f;
		if ( !check(xgeShapeExClipShapeClear(clipShapeBoundsTarget) == XGE_OK, "ShapeEx clip shape bounds target clear") ||
		     !check(xgeShapeExSceneAdd(clipShapeBoundsScene, clipShapeBoundsTarget) == XGE_OK, "ShapeEx scene clip shape bounds add target") ||
		     !check(xgeShapeExSceneClipShapeAdd(clipShapeBoundsScene, clipShapeBoundsClip) == XGE_OK, "ShapeEx scene clip shape bounds add clip") ||
		     !check(xgeShapeExSceneClipShapeAdd(clipShapeBoundsScene, clipShapeBoundsClip2) == XGE_OK, "ShapeEx scene clip shape bounds add clip union") ||
		     !check(xgeShapeExSceneTransformSet(clipShapeBoundsScene, &matrix) == XGE_OK, "ShapeEx scene clip shape bounds transform") ) {
			xgeShapeExSceneDestroy(clipShapeBoundsScene);
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExSceneGetBounds(clipShapeBoundsScene, 0.05f, &clipShapeBounds);
		if ( !check((ret == XGE_OK) && (clipShapeBounds.fX > 9.9f) && (clipShapeBounds.fX < 10.1f) && (clipShapeBounds.fY > 19.9f) && (clipShapeBounds.fY < 20.1f) && (clipShapeBounds.fW > 19.9f) && (clipShapeBounds.fW < 20.1f) && (clipShapeBounds.fH > 19.9f) && (clipShapeBounds.fH < 20.1f), "ShapeEx scene clip shape logical bounds value") ) {
			xgeShapeExSceneDestroy(clipShapeBoundsScene);
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		ret = xgeShapeExSceneGetOBB(clipShapeBoundsScene, 0.05f, clipShapeObb);
		if ( !check(ret == XGE_OK, "ShapeEx scene clip shape obb get") ||
		     !check((clipShapeObb[0].fX > 9.9f) && (clipShapeObb[0].fX < 10.1f) && (clipShapeObb[0].fY > 19.9f) && (clipShapeObb[0].fY < 20.1f), "ShapeEx scene clip shape logical obb point 0") ||
		     !check((clipShapeObb[2].fX > 29.9f) && (clipShapeObb[2].fX < 30.1f) && (clipShapeObb[2].fY > 39.9f) && (clipShapeObb[2].fY < 40.1f), "ShapeEx scene clip shape logical obb point 2") ) {
			xgeShapeExSceneDestroy(clipShapeBoundsScene);
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		if ( !check(xgeShapeExSceneClipShapeClear(clipShapeBoundsScene) == XGE_OK, "ShapeEx scene subtract clip shape bounds clear") ||
		     !check(xgeShapeExSceneClipShapeAddEx(clipShapeBoundsScene, clipShapeBoundsClip, XGE_SHAPE_EX_CLIP_SUBTRACT) == XGE_OK, "ShapeEx scene subtract clip shape bounds add") ||
		     !check(xgeShapeExSceneGetBounds(clipShapeBoundsScene, 0.05f, &clipShapeBounds) == XGE_OK, "ShapeEx scene subtract clip shape bounds get") ||
		     !check((clipShapeBounds.fX > 9.9f) && (clipShapeBounds.fX < 10.1f) && (clipShapeBounds.fY > 19.9f) && (clipShapeBounds.fY < 20.1f) &&
		            (clipShapeBounds.fW > 19.9f) && (clipShapeBounds.fW < 20.1f) && (clipShapeBounds.fH > 19.9f) && (clipShapeBounds.fH < 20.1f), "ShapeEx scene subtract clip shape bounds conservative value") ||
		     !check(xgeShapeExSceneGetOBB(clipShapeBoundsScene, 0.05f, clipShapeObb) == XGE_OK, "ShapeEx scene subtract clip shape obb get") ||
		     !check((clipShapeObb[0].fX > 9.9f) && (clipShapeObb[0].fX < 10.1f) && (clipShapeObb[0].fY > 19.9f) && (clipShapeObb[0].fY < 20.1f), "ShapeEx scene subtract clip shape obb conservative point 0") ) {
			xgeShapeExSceneDestroy(clipShapeBoundsScene);
			xgeShapeExDestroy(clipShapeBoundsClip2);
			xgeShapeExDestroy(clipShapeBoundsClip);
			xgeShapeExDestroy(clipShapeBoundsTarget);
			return 0;
		}
		xgeShapeExSceneDestroy(clipShapeBoundsScene);
		xgeShapeExDestroy(clipShapeBoundsClip2);
		xgeShapeExDestroy(clipShapeBoundsClip);
		xgeShapeExDestroy(clipShapeBoundsTarget);
	}
	{
		xge_shape_ex cubicFirst = NULL;
		xge_shape_ex raw = NULL;
		xge_shape_ex lineFirst = NULL;
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
		const uint8_t rawLineWithoutMove[1] = {
			XGE_SHAPE_EX_CMD_LINE_TO
		};
		const uint8_t rawCubicWithoutMove[1] = {
			XGE_SHAPE_EX_CMD_CUBIC_TO
		};
		const uint8_t rawCloseOnly[1] = {
			XGE_SHAPE_EX_CMD_CLOSE
		};
		xge_vec2_t badRawPoints[5];
		const uint8_t* gotCommands = NULL;
		const xge_vec2_t* gotPoints = NULL;
		volatile float zero = 0.0f;
		float badNaN;
		float badInf;
		int gotCommandCount = -1;
		int gotPointCount = -1;
		int i;

		badNaN = 0.0f / zero;
		badInf = 1.0f / zero;
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
		ret = xgeShapeExCreate(&lineFirst);
		if ( !check((ret == XGE_OK) && (lineFirst != NULL), "ShapeEx raw line-first create") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(lineFirst, rawLineWithoutMove, 1, rawPoints, 1) == XGE_OK, "ShapeEx raw path accepts line without move") ) {
			xgeShapeExDestroy(lineFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(lineFirst, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommands != NULL) && (gotPoints != NULL) &&
		            (gotCommandCount == 1) && (gotPointCount == 1) &&
		            (gotCommands[0] == XGE_SHAPE_EX_CMD_LINE_TO) &&
		            (gotPoints[0].fX == rawPoints[0].fX) && (gotPoints[0].fY == rawPoints[0].fY),
		            "ShapeEx raw line-first keeps line") ) {
			xgeShapeExDestroy(lineFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetBounds(lineFirst, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) &&
		            (bounds.fX > 99.999f) && (bounds.fX < 100.001f) &&
		            (bounds.fY > 99.999f) && (bounds.fY < 100.001f) &&
		            (bounds.fW >= 0.0f) && (bounds.fW < 0.001f) &&
		            (bounds.fH >= 0.0f) && (bounds.fH < 0.001f),
		            "ShapeEx raw line-first bounds") ) {
			xgeShapeExDestroy(lineFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		xgeShapeExDestroy(lineFirst);
		lineFirst = NULL;
		ret = xgeShapeExCreate(&cubicFirst);
		if ( !check((ret == XGE_OK) && (cubicFirst != NULL), "ShapeEx raw cubic-first create") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendPath(cubicFirst, rawCubicWithoutMove, 1, rawPoints, 3) == XGE_OK, "ShapeEx raw path accepts cubic without move") ) {
			xgeShapeExDestroy(cubicFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(cubicFirst, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommands != NULL) && (gotPoints != NULL) &&
		            (gotCommandCount == 1) && (gotPointCount == 3) &&
		            (gotCommands[0] == XGE_SHAPE_EX_CMD_CUBIC_TO) &&
		            (gotPoints[0].fX == rawPoints[0].fX) && (gotPoints[0].fY == rawPoints[0].fY) &&
		            (gotPoints[1].fX == rawPoints[1].fX) && (gotPoints[1].fY == rawPoints[1].fY) &&
		            (gotPoints[2].fX == rawPoints[2].fX) && (gotPoints[2].fY == rawPoints[2].fY),
		            "ShapeEx raw cubic-first keeps cubic") ) {
			xgeShapeExDestroy(cubicFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetBounds(cubicFirst, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "ShapeEx raw cubic-first bounds empty") ) {
			xgeShapeExDestroy(cubicFirst);
			xgeShapeExDestroy(raw);
			return 0;
		}
		xgeShapeExDestroy(cubicFirst);
		cubicFirst = NULL;
		memcpy(badRawPoints, rawPoints, sizeof(rawPoints));
		badRawPoints[1].fX = badNaN;
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 5, badRawPoints, 5) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects nan point") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		memcpy(badRawPoints, rawPoints, sizeof(rawPoints));
		badRawPoints[2].fY = badInf;
		if ( !check(xgeShapeExAppendPath(raw, rawCommands, 5, badRawPoints, 5) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects inf point") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExMoveTo(raw, badNaN, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx move rejects nan point") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(raw, NULL, &gotCommandCount, NULL, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 0) && (gotPointCount == 0), "ShapeEx invalid raw path keeps empty shape") ) {
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
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M0x1 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects hex path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M0x1p2 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects hex float path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M1e 0 L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects incomplete exponent path number") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "L1 1") == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx rejects svg path without move") ) {
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
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M4 4 Lnan 5") == XGE_OK, "ShapeEx ThorVG invalid path keeps move prefix") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M4 4 L") == XGE_OK, "ShapeEx ThorVG incomplete command keeps prefix") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M4 4 Z 1 2") == XGE_OK, "ShapeEx ThorVG ignores bare numbers after close") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(invalidPath, "M4 4 Z M6 6 L8 6") == XGE_OK, "ShapeEx accepts command after close") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(invalidPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 10) && (gotPointCount == 8), "ShapeEx command after close path counts") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(invalidPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 10) && (gotPointCount == 8) && (gotPoints[0].fX == 0.0f) && (gotPoints[1].fX == 2.0f) && (gotPoints[2].fX == 4.0f) && (gotPoints[7].fX == 8.0f), "ShapeEx invalid path preserves ThorVG prefixes") ) {
			xgeShapeExDestroy(invalidPath);
			xgeShapeExDestroy(raw);
			return 0;
		}
		xgeShapeExDestroy(invalidPath);
		{
			xge_shape_ex numericPath = NULL;
			ret = xgeShapeExCreate(&numericPath);
			if ( !check((ret == XGE_OK) && (numericPath != NULL), "ShapeEx numeric svg path create") ) {
				xgeShapeExDestroy(raw);
				return 0;
			}
			if ( !check(xgeShapeExAppendSvgPath(numericPath, "M.5 .25 L5. 1e-2 L10-5") == XGE_OK, "ShapeEx accepts SVG number grammar") ) {
				xgeShapeExDestroy(numericPath);
				xgeShapeExDestroy(raw);
				return 0;
			}
			ret = xgeShapeExGetPath(numericPath, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
			if ( !check((ret == XGE_OK) && (gotCommandCount == 3) && (gotPointCount == 3) &&
				(gotPoints[0].fX > 0.499f) && (gotPoints[0].fX < 0.501f) &&
				(gotPoints[0].fY > 0.249f) && (gotPoints[0].fY < 0.251f) &&
				(gotPoints[1].fX > 4.999f) && (gotPoints[1].fX < 5.001f) &&
				(gotPoints[1].fY > 0.009f) && (gotPoints[1].fY < 0.011f) &&
				(gotPoints[2].fX > 9.999f) && (gotPoints[2].fX < 10.001f) &&
				(gotPoints[2].fY > -5.001f) && (gotPoints[2].fY < -4.999f), "ShapeEx SVG number grammar values") ) {
				xgeShapeExDestroy(numericPath);
				xgeShapeExDestroy(raw);
				return 0;
			}
			xgeShapeExDestroy(numericPath);
		}
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
		if ( !check(xgeShapeExAppendPath(raw, rawCloseOnly, 1, NULL, 0) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx raw path rejects close-only command") ) {
			xgeShapeExDestroy(raw);
			return 0;
		}
		ret = xgeShapeExGetPath(raw, &gotCommands, &gotCommandCount, &gotPoints, &gotPointCount);
		if ( !check((ret == XGE_OK) && (gotCommandCount == 0) && (gotPointCount == 0), "ShapeEx raw path close-only keeps empty") ) {
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
		volatile float zero = 0.0f;
		float badNaN;
		float badInf;

		badNaN = 0.0f / zero;
		badInf = 1.0f / zero;
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
		if ( !check(xgeShapeExAppendArc(atomic, 0.0f, 0.0f, 2.0f, 3.0f, 0.0f, badNaN) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic arc rejects nan angle") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendPie(atomic, 0.0f, 0.0f, 2.0f, -3.0f, 0.0f, 1.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic pie rejects invalid radius") ) {
			xgeShapeExDestroy(atomic);
			return 0;
		}
		if ( !check(xgeShapeExAppendRect(atomic, badInf, 0.0f, 2.0f, 3.0f, 0.0f, 0.0f, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx atomic rect rejects inf position") ) {
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
		float trimBegin;
		float trimEnd;
		int trimSimultaneous;
		int trimEnabled;

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
		if ( !check(xgeShapeExTrimPath(line, invalidNaN, 0.75f, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx trim rejects nan") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		if ( !check(xgeShapeExTrimPathGet(NULL, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx trim get rejects null shape") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExTrimPathGet(line, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled);
		if ( !check((ret == XGE_OK) &&
		            (trimBegin > -0.001f) && (trimBegin < 0.001f) &&
		            (trimEnd > 0.999f) && (trimEnd < 1.001f) &&
		            (trimSimultaneous == 0) && (trimEnabled == 0), "ShapeEx trim get default") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		if ( !check(xgeShapeExTrimPath(line, 0.25f, 0.75f, 1) == XGE_OK, "ShapeEx trim path") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExTrimPathGet(line, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled);
		if ( !check((ret == XGE_OK) &&
		            (trimBegin > 0.249f) && (trimBegin < 0.251f) &&
		            (trimEnd > 0.749f) && (trimEnd < 0.751f) &&
		            (trimSimultaneous == 1) && (trimEnabled == 1), "ShapeEx trim get value") ) {
			xgeShapeExDestroy(line);
			return 0;
		}
		ret = xgeShapeExGetBounds(line, 0.05f, &bounds);
		if ( !check((ret == XGE_OK) &&
		            (bounds.fX > 0.74f) && (bounds.fX < 0.76f) &&
		            (bounds.fY > 0.99f) && (bounds.fY < 1.01f) &&
		            (bounds.fW > 1.49f) && (bounds.fW < 1.51f) &&
		            (bounds.fH > 1.99f) && (bounds.fH < 2.01f), "ShapeEx trim changes geometry bounds") ) {
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
			ret = xgeShapeExTrimPathGet(clone, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled);
			if ( !check((ret == XGE_OK) &&
			            (trimBegin > 0.249f) && (trimBegin < 0.251f) &&
			            (trimEnd > 0.749f) && (trimEnd < 0.751f) &&
			            (trimSimultaneous == 1) && (trimEnabled == 1), "ShapeEx trim clone get value") ) {
				xgeShapeExDestroy(clone);
				xgeShapeExDestroy(line);
				return 0;
			}
			ret = xgeShapeExGetBounds(clone, 0.05f, &bounds);
			if ( !check((ret == XGE_OK) &&
			            (bounds.fX > 0.74f) && (bounds.fX < 0.76f) &&
			            (bounds.fW > 1.49f) && (bounds.fW < 1.51f), "ShapeEx trim clone geometry bounds") ) {
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
		ret = xgeShapeExTrimPathGet(line, &trimBegin, &trimEnd, &trimSimultaneous, &trimEnabled);
		if ( !check((ret == XGE_OK) &&
		            (trimBegin > -0.001f) && (trimBegin < 0.001f) &&
		            (trimEnd > 0.999f) && (trimEnd < 1.001f) &&
		            (trimSimultaneous == 1) && (trimEnabled == 0), "ShapeEx trim get clear value") ) {
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
		            (bounds.fW > 19.99f) && (bounds.fW < 20.01f), "ShapeEx trim combined geometry bounds") ) {
			xgeShapeExDestroy(multi);
			return 0;
		}
		xgeShapeExDestroy(multi);
	}
	{
		xge_shape_ex trimmedStroke = NULL;
		int contains = 0;

		ret = xgeShapeExCreate(&trimmedStroke);
		if ( !check((ret == XGE_OK) && (trimmedStroke != NULL), "ShapeEx trimmed stroke create") ) return 0;
		if ( !check(xgeShapeExMoveTo(trimmedStroke, 0.0f, 0.0f) == XGE_OK, "ShapeEx trimmed stroke move") ||
		     !check(xgeShapeExCubicTo(trimmedStroke, 25.0f, 40.0f, 75.0f, -40.0f, 100.0f, 0.0f) == XGE_OK, "ShapeEx trimmed stroke cubic") ||
		     !check(xgeShapeExFillColor(trimmedStroke, XGE_COLOR_RGBA(0, 0, 0, 0)) == XGE_OK, "ShapeEx trimmed stroke clear fill") ||
		     !check(xgeShapeExStrokeColor(trimmedStroke, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx trimmed stroke color") ||
		     !check(xgeShapeExStrokeWidth(trimmedStroke, 4.0f) == XGE_OK, "ShapeEx trimmed stroke width") ||
		     !check(xgeShapeExStrokeCap(trimmedStroke, XGE_SHAPE_EX_CAP_BUTT) == XGE_OK, "ShapeEx trimmed stroke cap") ||
		     !check(xgeShapeExTrimPath(trimmedStroke, 0.25f, 0.75f, 0) == XGE_OK, "ShapeEx trimmed stroke range") ) {
			xgeShapeExDestroy(trimmedStroke);
			return 0;
		}
		if ( !check((xgeShapeExContainsPoint(trimmedStroke, 5.0f, 5.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx trim excludes leading cubic stroke") ||
		     !check((xgeShapeExContainsPoint(trimmedStroke, 50.0f, 0.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx trim includes middle cubic stroke") ||
		     !check((xgeShapeExContainsPoint(trimmedStroke, 95.0f, -5.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx trim excludes trailing cubic stroke") ) {
			xgeShapeExDestroy(trimmedStroke);
			return 0;
		}
		xgeShapeExDestroy(trimmedStroke);
	}
	{
		xge_shape_ex trimmedFill = NULL;
		int contains = 0;

		ret = xgeShapeExCreate(&trimmedFill);
		if ( !check((ret == XGE_OK) && (trimmedFill != NULL), "ShapeEx trimmed fill create") ) return 0;
		if ( !check(xgeShapeExAppendRect(trimmedFill, 0.0f, 0.0f, 100.0f, 20.0f, 0.0f, 0.0f, 1) == XGE_OK, "ShapeEx trimmed fill rect") ||
		     !check(xgeShapeExFillColor(trimmedFill, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx trimmed fill color") ||
		     !check(xgeShapeExStrokeWidth(trimmedFill, 0.0f) == XGE_OK, "ShapeEx trimmed fill no stroke") ||
		     !check(xgeShapeExTrimPath(trimmedFill, 0.25f, 0.75f, 0) == XGE_OK, "ShapeEx trimmed fill range") ||
		     !check((xgeShapeExContainsPoint(trimmedFill, 5.0f, 10.0f, 0.05f, &contains) == XGE_OK) && contains, "ShapeEx trim closes retained fill path") ||
		     !check((xgeShapeExContainsPoint(trimmedFill, 95.0f, 10.0f, 0.05f, &contains) == XGE_OK) && !contains, "ShapeEx trim excludes removed fill region") ) {
			xgeShapeExDestroy(trimmedFill);
			return 0;
		}
		xgeShapeExDestroy(trimmedFill);
	}
	{
		xge_shape_ex smooth = NULL;
		xge_rect_t smoothBounds;
		const uint8_t* smoothCommands = NULL;
		const xge_vec2_t* smoothPoints = NULL;
		int smoothCommandCount = 0;
		int smoothPointCount = 0;

		ret = xgeShapeExCreate(&smooth);
		if ( !check((ret == XGE_OK) && (smooth != NULL), "ShapeEx smooth create") ) return 0;
		ret = xgeShapeExAppendSvgPath(smooth, "M0 0 Q5 10 10 0 T20 0");
		if ( !check(ret == XGE_OK, "ShapeEx quadratic smooth parse") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		ret = xgeShapeExGetPath(smooth, &smoothCommands, &smoothCommandCount, &smoothPoints, &smoothPointCount);
		if ( !check((ret == XGE_OK) && (smoothCommandCount == 3) && (smoothPointCount == 5), "ShapeEx quadratic smooth path counts") ||
		     !check((smoothCommands[0] == XGE_SHAPE_EX_CMD_MOVE_TO) &&
		            (smoothCommands[1] == XGE_SHAPE_EX_CMD_QUAD_TO) &&
		            (smoothCommands[2] == XGE_SHAPE_EX_CMD_QUAD_TO), "ShapeEx svg quadratic commands preserved") ||
		     !check((smoothPoints[1].fX == 5.0f) && (smoothPoints[1].fY == 10.0f) &&
		            (smoothPoints[2].fX == 10.0f) && (smoothPoints[2].fY == 0.0f) &&
		            (smoothPoints[3].fX == 15.0f) && (smoothPoints[3].fY == -10.0f) &&
		            (smoothPoints[4].fX == 20.0f) && (smoothPoints[4].fY == 0.0f), "ShapeEx svg T reflects quadratic control") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		ret = xgeShapeExGetBounds(smooth, 0.05f, &smoothBounds);
		if ( !check((ret == XGE_OK) && (smoothBounds.fH > 9.5f), "ShapeEx quadratic smooth bounds") ) {
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
		if ( !check((ret == XGE_OK) && (arcDegenerate != NULL), "ShapeEx zero-sweep arc create") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExAppendArc(arcDegenerate, 10.0f, 10.0f, 4.0f, 3.0f, 1.0f, 1.0f) == XGE_OK, "ShapeEx zero-sweep arc no-op") ||
		     !check(xgeShapeExAppendPie(arcDegenerate, 10.0f, 10.0f, 4.0f, 3.0f, 1.0f, 1.0f) == XGE_OK, "ShapeEx zero-sweep pie no-op") ||
		     !check(xgeShapeExAppendChord(arcDegenerate, 10.0f, 10.0f, 4.0f, 3.0f, 1.0f, 1.0f) == XGE_OK, "ShapeEx zero-sweep chord no-op") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExGetPath(arcDegenerate, NULL, &commandCountAfter, NULL, &pointCountAfter);
		if ( !check((commandCountAfter == 0) && (pointCountAfter == 0), "ShapeEx zero-sweep primitives keep path empty") ) {
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
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 2 0 20 0") == XGE_OK, "ShapeEx svg arc keeps prefix before invalid large-arc flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 1 2 20 0") == XGE_OK, "ShapeEx svg arc keeps prefix before invalid sweep flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		if ( !check(xgeShapeExAppendSvgPath(arcDegenerate, "M0 0 A10 10 0 1.0 0 20 0") == XGE_OK, "ShapeEx svg arc keeps prefix before decimal flag") ) {
			xgeShapeExDestroy(arc);
			xgeShapeExDestroy(arcDegenerate);
			return 0;
		}
		xgeShapeExDestroy(arcDegenerate);
		if ( !check(xgeShapeExMoveTo(arc, 0.0f, 0.0f) == XGE_OK, "ShapeEx arc moveTo") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		xgeShapeExGetPath(arc, NULL, &commandCountBefore, NULL, &pointCountBefore);
		if ( !check(xgeShapeExArcTo(arc, 6.0f, 4.0f, 25.0f, 2, 1, 12.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx arc rejects invalid large-arc flag") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		if ( !check(xgeShapeExArcTo(arc, 6.0f, 4.0f, 25.0f, 0, -1, 12.0f, 0.0f) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx arc rejects invalid sweep flag") ) {
			xgeShapeExDestroy(arc);
			return 0;
		}
		xgeShapeExGetPath(arc, NULL, &commandCountAfter, NULL, &pointCountAfter);
		if ( !check((commandCountAfter == commandCountBefore) && (pointCountAfter == pointCountBefore), "ShapeEx invalid arc flags keep path") ) {
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
	{
		static const int maskMethods[] = {
			XGE_SHAPE_EX_MASK_ALPHA,
			XGE_SHAPE_EX_MASK_INV_ALPHA,
			XGE_SHAPE_EX_MASK_LUMA,
			XGE_SHAPE_EX_MASK_INV_LUMA,
			XGE_SHAPE_EX_MASK_ADD,
			XGE_SHAPE_EX_MASK_SUBTRACT,
			XGE_SHAPE_EX_MASK_INTERSECT,
			XGE_SHAPE_EX_MASK_DIFFERENCE,
			XGE_SHAPE_EX_MASK_LIGHTEN,
			XGE_SHAPE_EX_MASK_DARKEN
		};
		static const int expectedSourceOnly[] = {0, 1, 0, 1, 1, 1, 0, 1, 1, 0};
		static const int expectedOverlap[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 1};
		xge_shape_ex maskSource = NULL;
		xge_shape_ex maskTarget = NULL;
		xge_shape_ex maskClone = NULL;
		xge_shape_ex cycleTarget = NULL;
		xge_shape_ex sceneMaskTarget = NULL;
		xge_shape_ex gotMaskShape = NULL;
		xge_shape_ex_scene maskTargetScene = NULL;
		xge_shape_ex_scene maskSceneSource = NULL;
		xge_shape_ex_scene maskSceneClone = NULL;
		xge_shape_ex_scene gotMaskScene = NULL;
		xge_rect_t maskBounds;
		int gotMethod;
		int gotTargetType;
		int contains;
		int i;
		int maskTestOk = 0;

		ret = xgeShapeExCreate(&maskSource);
		if ( !check((ret == XGE_OK) && (maskSource != NULL), "ShapeEx mask source create") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExCreate(&maskTarget);
		if ( !check((ret == XGE_OK) && (maskTarget != NULL), "ShapeEx mask target create") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExCreate(&sceneMaskTarget);
		if ( !check((ret == XGE_OK) && (sceneMaskTarget != NULL), "ShapeEx scene mask target create") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExFillColor(maskSource, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx mask source white") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExFillColor(sceneMaskTarget, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx scene mask target white") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExAppendRect(maskSource, 0, 0, 10, 10, 0, 0, 1) == XGE_OK, "ShapeEx mask source rect") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExAppendRect(maskTarget, 5, 0, 10, 10, 0, 0, 1) == XGE_OK, "ShapeEx mask target rect") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExAppendRect(sceneMaskTarget, 5, 0, 10, 10, 0, 0, 1) == XGE_OK, "ShapeEx scene mask target rect") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExFillColor(maskTarget, XGE_COLOR_RGBA(255, 255, 255, 255)) == XGE_OK, "ShapeEx mask target white") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskGet(maskSource, &gotMethod, &gotTargetType, &gotMaskShape, &gotMaskScene) == XGE_OK, "ShapeEx mask get default") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_NONE) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_NONE) && (gotMaskShape == NULL) && (gotMaskScene == NULL), "ShapeEx mask default value") ) goto shape_ex_mask_cleanup;
		for ( i = 0; i < (int)(sizeof(maskMethods) / sizeof(maskMethods[0])); i++ ) {
			if ( !check(xgeShapeExMaskShapeSet(maskSource, maskTarget, maskMethods[i]) == XGE_OK, "ShapeEx mask mode set") ) goto shape_ex_mask_cleanup;
			if ( !check(xgeShapeExMaskGet(maskSource, &gotMethod, &gotTargetType, &gotMaskShape, NULL) == XGE_OK, "ShapeEx mask mode get") ) goto shape_ex_mask_cleanup;
			if ( !check((gotMethod == maskMethods[i]) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE) && (gotMaskShape == maskTarget), "ShapeEx mask mode value") ) goto shape_ex_mask_cleanup;
			ret = xgeShapeExGetBounds(maskSource, 0.1f, &maskBounds);
			if ( !check((ret == XGE_OK) && (fabsf(maskBounds.fX) < 0.01f) && (fabsf(maskBounds.fW - 10.0f) < 0.01f), "ShapeEx mask mode logical bounds") ) goto shape_ex_mask_cleanup;
			ret = xgeShapeExContainsPoint(maskSource, 2, 5, 0.1f, &contains);
			if ( !check((ret == XGE_OK) && (contains == expectedSourceOnly[i]), "ShapeEx mask source-only hit") ) goto shape_ex_mask_cleanup;
			ret = xgeShapeExContainsPoint(maskSource, 7, 5, 0.1f, &contains);
			if ( !check((ret == XGE_OK) && (contains == expectedOverlap[i]), "ShapeEx mask overlap hit") ) goto shape_ex_mask_cleanup;
		}
		if ( !check(xgeShapeExMaskShapeSet(maskSource, maskSource, XGE_SHAPE_EX_MASK_ALPHA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx mask rejects self cycle") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskShapeSet(maskSource, maskTarget, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx mask reset for clone") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExClone(maskSource, &maskClone);
		if ( !check((ret == XGE_OK) && (maskClone != NULL), "ShapeEx masked clone") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskGet(maskClone, &gotMethod, &gotTargetType, &gotMaskShape, NULL) == XGE_OK, "ShapeEx clone mask get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_ALPHA) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE) && (gotMaskShape != NULL) && (gotMaskShape != maskTarget), "ShapeEx clone deep-copies mask") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExSceneCreate(&maskTargetScene);
		if ( !check((ret == XGE_OK) && (maskTargetScene != NULL), "ShapeEx mask target scene create") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneAdd(maskTargetScene, maskTarget) == XGE_OK, "ShapeEx mask target scene add") ) goto shape_ex_mask_cleanup;
		{
			xge_rect_t externalBounds = {0.0f, 0.0f, 10.0f, 10.0f};
			xge_shape_ex_matrix_t externalMatrix = matrix;

			if ( !check(xgeShapeExMaskCompositeScene(NULL, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_ALPHA, externalBounds, test_shape_ex_external_draw, NULL, 0.25f, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects null scene") ) goto shape_ex_mask_cleanup;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, -1, XGE_BLEND_ALPHA, externalBounds, test_shape_ex_external_draw, NULL, 0.25f, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects method") ) goto shape_ex_mask_cleanup;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_LUMINOSITY + 1, externalBounds, test_shape_ex_external_draw, NULL, 0.25f, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects blend") ) goto shape_ex_mask_cleanup;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_ALPHA, externalBounds, NULL, NULL, 0.25f, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects null draw") ) goto shape_ex_mask_cleanup;
			externalBounds.fX = invalidNaN;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_ALPHA, externalBounds, test_shape_ex_external_draw, NULL, 0.25f, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects nan bounds") ) goto shape_ex_mask_cleanup;
			externalBounds.fX = 0.0f;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_ALPHA, externalBounds, test_shape_ex_external_draw, NULL, invalidNaN, &matrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects nan tolerance") ) goto shape_ex_mask_cleanup;
			externalMatrix.fA = invalidNaN;
			if ( !check(xgeShapeExMaskCompositeScene(maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA, XGE_BLEND_ALPHA, externalBounds, test_shape_ex_external_draw, NULL, 0.25f, &externalMatrix, 1) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx external scene mask rejects nan matrix") ) goto shape_ex_mask_cleanup;
		}
		if ( !check(xgeShapeExMaskSceneSet(maskSource, maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx shape mask scene set") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskGet(maskSource, &gotMethod, &gotTargetType, NULL, &gotMaskScene) == XGE_OK, "ShapeEx shape mask scene get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_ALPHA) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SCENE) && (gotMaskScene == maskTargetScene), "ShapeEx shape mask scene value") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneAdd(maskTargetScene, maskSource) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene add rejects mask cycle") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskClear(maskSource) == XGE_OK, "ShapeEx mask clear before scene cycle") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneAdd(maskTargetScene, maskSource) == XGE_OK, "ShapeEx mask target scene add source") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskSceneSet(maskSource, maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx mask rejects scene cycle") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneRemove(maskTargetScene, maskSource) == XGE_OK, "ShapeEx mask target scene remove source") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExSceneCreate(&maskSceneSource);
		if ( !check((ret == XGE_OK) && (maskSceneSource != NULL), "ShapeEx masked scene create") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneAdd(maskSceneSource, maskSource) == XGE_OK, "ShapeEx masked scene add") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskSceneSet(maskSceneSource, maskSceneSource, XGE_SHAPE_EX_MASK_ALPHA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene mask rejects self cycle") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExCreate(&cycleTarget);
		if ( !check((ret == XGE_OK) && (cycleTarget != NULL), "ShapeEx mask cycle target create") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExAppendRect(cycleTarget, 0, 0, 4, 4, 0, 0, 1) == XGE_OK, "ShapeEx mask cycle target rect") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskSceneSet(cycleTarget, maskSceneSource, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx mask cycle target scene set") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskShapeSet(maskSceneSource, cycleTarget, XGE_SHAPE_EX_MASK_ALPHA) == XGE_ERROR_INVALID_ARGUMENT, "ShapeEx scene mask rejects indirect cycle") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskClear(cycleTarget) == XGE_OK, "ShapeEx mask cycle clear") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskSceneSet(maskSceneSource, maskTargetScene, XGE_SHAPE_EX_MASK_ALPHA) == XGE_OK, "ShapeEx scene mask scene set") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskGet(maskSceneSource, &gotMethod, &gotTargetType, NULL, &gotMaskScene) == XGE_OK, "ShapeEx scene mask scene get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_ALPHA) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SCENE) && (gotMaskScene == maskTargetScene), "ShapeEx scene mask scene value") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskShapeSet(maskSceneSource, sceneMaskTarget, XGE_SHAPE_EX_MASK_ADD) == XGE_OK, "ShapeEx scene mask shape set") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskGet(maskSceneSource, &gotMethod, &gotTargetType, &gotMaskShape, NULL) == XGE_OK, "ShapeEx scene mask shape get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_ADD) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE) && (gotMaskShape == sceneMaskTarget), "ShapeEx scene mask shape value") ) goto shape_ex_mask_cleanup;
		ret = xgeShapeExSceneClone(maskSceneSource, &maskSceneClone);
		if ( !check((ret == XGE_OK) && (maskSceneClone != NULL), "ShapeEx masked scene clone") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskGet(maskSceneClone, &gotMethod, &gotTargetType, &gotMaskShape, NULL) == XGE_OK, "ShapeEx scene clone mask get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_ADD) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_SHAPE) && (gotMaskShape != NULL) && (gotMaskShape != sceneMaskTarget), "ShapeEx scene clone deep-copies mask") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExSceneMaskClear(maskSceneSource) == XGE_OK, "ShapeEx scene mask clear") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskClear(maskSource) == XGE_OK, "ShapeEx shape mask clear") ) goto shape_ex_mask_cleanup;
		if ( !check(xgeShapeExMaskGet(maskSource, &gotMethod, &gotTargetType, NULL, NULL) == XGE_OK, "ShapeEx cleared mask get") ) goto shape_ex_mask_cleanup;
		if ( !check((gotMethod == XGE_SHAPE_EX_MASK_NONE) && (gotTargetType == XGE_SHAPE_EX_MASK_TARGET_NONE), "ShapeEx cleared mask value") ) goto shape_ex_mask_cleanup;
		maskTestOk = 1;

shape_ex_mask_cleanup:
		xgeShapeExDestroy(cycleTarget);
		xgeShapeExSceneDestroy(maskSceneClone);
		xgeShapeExSceneDestroy(maskSceneSource);
		xgeShapeExSceneDestroy(maskTargetScene);
		xgeShapeExDestroy(maskClone);
		xgeShapeExDestroy(sceneMaskTarget);
		xgeShapeExDestroy(maskTarget);
		xgeShapeExDestroy(maskSource);
		if ( !maskTestOk ) return 0;
	}
	return 1;
}

static int test_svg(void)
{
#define XGE_TEST_RASTER_PNG "iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFElEQVR42mP4z8DwHwyBNBAw/AcAR8oI+FuapL4AAAAASUVORK5CYII="
#define XGE_TEST_RASTER_ALPHA_PNG "iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAQSURBVBhXY/jPAEJw8P8/ACHwA/2vep5VAAAAAElFTkSuQmCC"
#define XGE_TEST_RASTER_JPEG "/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAACAAIDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwD1jwfpenzeCdBllsbV5H063ZnaFSWJjXJJxyaKKK+erfxZerPlcR/Gn6v8z//Z"
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
		"<filter id=\"ignored\"><feOffset/><feFlood/><feDropShadow/><feMorphology/><feTile/><feImage/>"
		"<feColorMatrix/><feComponentTransfer/><feComposite/><feBlend/><feMerge/></filter>"
		"</defs>"
		"<rect x=\"2\" y=\"3\" width=\"4\" height=\"5\" fill=\"#38bdf8\" filter=\"url(#ignored)\"/>"
		"</svg>";
	static const char svg_filter_identity_clip[] =
		"<svg viewBox=\"0 0 12 12\">"
		"<defs><filter id=\"identity\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"2\" y=\"2\" width=\"4\" height=\"8\"><feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect x=\"0\" y=\"4\" width=\"10\" height=\"4\" fill=\"#22c55e\" filter=\"url(#identity)\"/>"
		"</svg>";
	static const char svg_filter_gaussian_blur[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<filter id=\"blurBoth\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" x=\"-2\" y=\"-2\" width=\"12\" height=\"12\"><feGaussianBlur stdDeviation=\"0.8 0.8\"/></filter>"
		"<filter id=\"blurX\" primitiveUnits=\"userSpaceOnUse\"><feGaussianBlur stdDeviation=\"1.2 0\"/></filter>"
		"<filter id=\"blurBox\" primitiveUnits=\"objectBoundingBox\"><feGaussianBlur stdDeviation=\"8% 12%\"/></filter>"
		"<filter id=\"blurInvalid\"><feGaussianBlur stdDeviation=\"-1 2\"/></filter>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"24\" height=\"12\" fill=\"#111827\"/>"
		"<circle cx=\"5\" cy=\"5\" r=\"3\" fill=\"#38bdf8\" filter=\"url(#blurBoth)\"/>"
		"<rect x=\"10\" y=\"3\" width=\"5\" height=\"4\" fill=\"#f97316\" filter=\"url(#blurX)\"/>"
		"<text x=\"16\" y=\"8\" font-size=\"4\" fill=\"#22c55e\" filter=\"url(#blurBox)\">B</text>"
		"<rect x=\"20\" y=\"1\" width=\"3\" height=\"3\" fill=\"#e879f9\" filter=\"url(#blurInvalid)\"/>"
		"</svg>";
	static const char svg_filter_active_bounds[] =
		"<svg viewBox=\"0 0 20 20\">"
		"<defs><filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"20\" height=\"20\"><feGaussianBlur stdDeviation=\"2\"/></filter></defs>"
		"<rect x=\"8\" y=\"8\" width=\"4\" height=\"4\" fill=\"#38bdf8\" filter=\"url(#blur)\"/>"
		"</svg>";
	static const char svg_filter_transform_bounds[] =
		"<svg viewBox=\"0 0 40 30\">"
		"<defs><filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"-20\" y=\"-20\" width=\"100\" height=\"100\"><feGaussianBlur stdDeviation=\"1\"/></filter></defs>"
		"<rect x=\"5\" y=\"5\" width=\"10\" height=\"8\" transform=\"scale(2)\" fill=\"#22c55e\" filter=\"url(#blur)\"/>"
		"</svg>";
	static const char svg_filter_group_bounds[] =
		"<svg viewBox=\"0 0 40 30\">"
		"<defs><filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"-20\" y=\"-20\" width=\"100\" height=\"100\"><feGaussianBlur stdDeviation=\"1\"/></filter></defs>"
		"<g transform=\"scale(1.5)\" filter=\"url(#blur)\">"
		"<rect x=\"4\" y=\"4\" width=\"8\" height=\"8\" fill=\"#3b82f6\"/>"
		"<circle cx=\"12\" cy=\"8\" r=\"4\" fill=\"#f97316\"/>"
		"</g></svg>";
	static const char svg_filter_forward_shape[] =
		"<svg viewBox=\"0 0 40 30\">"
		"<rect x=\"10\" y=\"8\" width=\"8\" height=\"6\" fill=\"#22c55e\" filter=\"url(#late)\"/>"
		"<filter id=\"late\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"40\" height=\"30\"><feGaussianBlur stdDeviation=\"1\"/></filter>"
		"</svg>";
	static const char svg_filter_forward_group[] =
		"<svg viewBox=\"0 0 40 30\">"
		"<g filter=\"url(#late)\"><rect x=\"8\" y=\"8\" width=\"8\" height=\"6\" fill=\"#3b82f6\"/>"
		"<circle cx=\"17\" cy=\"11\" r=\"3\" fill=\"#f97316\"/></g>"
		"<filter id=\"late\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"40\" height=\"30\"><feGaussianBlur stdDeviation=\"1\"/></filter>"
		"</svg>";
	static const char svg_filter_def_shape_use[] =
		"<svg viewBox=\"0 0 50 30\"><defs>"
		"<filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"50\" height=\"30\"><feGaussianBlur stdDeviation=\"1\"/></filter>"
		"<path id=\"source\" d=\"M10 8 H18 V14 H10 Z\" fill=\"#22c55e\" filter=\"url(#blur)\"/>"
		"</defs><use href=\"#source\" transform=\"translate(10 0)\"/></svg>";
	static const char svg_filter_def_group_use[] =
		"<svg viewBox=\"0 0 50 30\"><defs>"
		"<filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"50\" height=\"30\"><feGaussianBlur stdDeviation=\"1\"/></filter>"
		"<g id=\"source\" filter=\"url(#blur)\"><rect x=\"4\" y=\"8\" width=\"8\" height=\"6\" fill=\"#3b82f6\"/>"
		"<circle cx=\"13\" cy=\"11\" r=\"3\" fill=\"#f97316\"/></g>"
		"</defs><use href=\"#source\" transform=\"translate(12 0)\"/></svg>";
	static const char svg_filter_object_bbox_numeric_primitive_region[] =
		"<svg viewBox=\"0 0 50 30\"><defs>"
		"<filter id=\"blur\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"objectBoundingBox\" "
		"x=\"0\" y=\"0\" width=\"50\" height=\"30\"><feGaussianBlur stdDeviation=\"0.1\" "
		"x=\"0.18\" y=\"0.08\" width=\"0.62\" height=\"0.84\"/></filter></defs>"
		"<rect x=\"20\" y=\"10\" width=\"12\" height=\"8\" fill=\"#38bdf8\" filter=\"url(#blur)\"/>"
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
	static const char svg_clip_mixed_union[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<clipPath id=\"mixedClip\">"
		"<rect x=\"1\" y=\"1\" width=\"4\" height=\"4\"/>"
		"<circle cx=\"18\" cy=\"6\" r=\"3\"/>"
		"</clipPath>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"24\" height=\"12\" fill=\"#38bdf8\" clip-path=\"url(#mixedClip)\"/>"
		"</svg>";
	static const char svg_clip_multi_mixed_union[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<clipPath id=\"mixedClip\">"
		"<rect x=\"1\" y=\"1\" width=\"4\" height=\"4\"/>"
		"<rect x=\"7\" y=\"1\" width=\"4\" height=\"4\"/>"
		"<circle cx=\"18\" cy=\"6\" r=\"3\"/>"
		"</clipPath>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"24\" height=\"12\" fill=\"#38bdf8\" clip-path=\"url(#mixedClip)\"/>"
		"</svg>";
	static const char svg_clip_nested_clip_path[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<clipPath id=\"innerClip\"><rect x=\"8\" y=\"2\" width=\"5\" height=\"6\"/></clipPath>"
		"<clipPath id=\"outerClip\"><rect x=\"2\" y=\"1\" width=\"18\" height=\"9\" clip-path=\"url(#innerClip)\"/></clipPath>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"24\" height=\"12\" fill=\"#38bdf8\" clip-path=\"url(#outerClip)\"/>"
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
	static const char svg_mask_shapeex_scene[] =
		"<svg viewBox=\"0 0 40 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<mask id=\"m\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"20\" height=\"10\" maskContentUnits=\"userSpaceOnUse\" mask-type=\"Alpha\">"
		"<rect x=\"2\" y=\"2\" width=\"6\" height=\"6\" fill=\"black\"/>"
		"<circle cx=\"14\" cy=\"5\" r=\"3\" fill=\"black\"/>"
		"</mask>"
		"<rect id=\"source\" width=\"20\" height=\"10\" fill=\"#38bdf8\" mask=\"url(#m)\"/>"
		"</defs>"
		"<use href=\"#source\"/><use href=\"#source\" x=\"20\"/>"
		"</svg>";
	static const char svg_mask_shapeex_transform[] =
		"<svg viewBox=\"0 0 30 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<mask id=\"rotateMask\" maskContentUnits=\"objectBoundingBox\" mask-type=\"Alpha\">"
		"<rect x=\"0.2\" y=\"0.4\" width=\"0.6\" height=\"0.2\" fill=\"black\" transform=\"rotate(90 0.5 0.5)\"/>"
		"</mask>"
		"<mask id=\"skewMask\" maskContentUnits=\"objectBoundingBox\" mask-type=\"Alpha\">"
		"<rect x=\"0.1\" y=\"0.2\" width=\"0.3\" height=\"0.4\" fill=\"black\" transform=\"matrix(1 0 1 1 0 0)\"/>"
		"</mask>"
		"</defs>"
		"<rect width=\"10\" height=\"10\" fill=\"#38bdf8\" mask=\"url(#rotateMask)\"/>"
		"<rect x=\"20\" width=\"10\" height=\"10\" fill=\"#34d399\" mask=\"url(#skewMask)\"/>"
		"</svg>";
	static const char svg_mask_external_scene_hit[] =
		"<svg viewBox=\"0 0 110 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<mask id=\"textMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"30\" height=\"12\" mask-type=\"Alpha\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/><rect x=\"14\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/></mask>"
		"<mask id=\"imageMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"40\" y=\"0\" width=\"30\" height=\"12\" mask-type=\"Alpha\"><rect x=\"40\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/><rect x=\"60\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/></mask>"
		"<mask id=\"rasterMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"80\" y=\"0\" width=\"30\" height=\"12\" mask-type=\"Alpha\"><rect x=\"80\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/><rect x=\"100\" y=\"0\" width=\"8\" height=\"12\" fill=\"black\"/></mask>"
		"</defs>"
		"<text x=\"0\" y=\"10\" font-size=\"10\" fill=\"#f59e0b\" mask=\"url(#textMask)\">WWWW</text>"
		"<image x=\"40\" y=\"0\" width=\"30\" height=\"12\" mask=\"url(#imageMask)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 30 12' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='30' height='12' fill='%2338bdf8'/%3E%3C/svg%3E\"/>"
		"<image x=\"80\" y=\"0\" width=\"30\" height=\"12\" preserveAspectRatio=\"none\" mask=\"url(#rasterMask)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"</svg>";
	static const char svg_mask_type_alpha_ignored[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<mask id=\"blackAlpha\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"12\" height=\"6\" mask-type=\"alpha\"><rect x=\"0\" y=\"0\" width=\"6\" height=\"6\" fill=\"#000\"/></mask>"
		"<mask id=\"whiteAlpha\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"12\" height=\"6\" mask-type=\"alpha\"><rect x=\"6\" y=\"0\" width=\"6\" height=\"6\" fill=\"#fff\"/></mask>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"6\" height=\"6\" fill=\"#38bdf8\" mask=\"url(#blackAlpha)\"/>"
		"<rect x=\"6\" y=\"0\" width=\"6\" height=\"6\" fill=\"#22c55e\" mask=\"url(#whiteAlpha)\"/>"
		"</svg>";
	static const char svg_mask_type_upper_alpha[] =
		"<svg viewBox=\"0 0 30 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.alphaMask{mask-type:Alpha}</style><defs>"
		"<mask id=\"attr\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"6\" height=\"6\" mask-type=\"Alpha\"><rect width=\"6\" height=\"6\" fill=\"#000\"/></mask>"
		"<mask id=\"inline\" maskUnits=\"userSpaceOnUse\" x=\"8\" y=\"0\" width=\"6\" height=\"6\" style=\"mask-type:Alpha\"><rect x=\"8\" width=\"6\" height=\"6\" fill=\"#000\"/></mask>"
		"<mask id=\"css\" class=\"alphaMask\" maskUnits=\"userSpaceOnUse\" x=\"16\" y=\"0\" width=\"6\" height=\"6\"><rect x=\"16\" width=\"6\" height=\"6\" fill=\"#000\"/></mask>"
		"<mask id=\"invalid\" maskUnits=\"userSpaceOnUse\" x=\"24\" y=\"0\" width=\"6\" height=\"6\" mask-type=\"ALPHA\"><rect x=\"24\" width=\"6\" height=\"6\" fill=\"#000\"/></mask>"
		"</defs><rect width=\"6\" height=\"6\" mask=\"url(#attr)\"/><rect x=\"8\" width=\"6\" height=\"6\" mask=\"url(#inline)\"/>"
		"<rect x=\"16\" width=\"6\" height=\"6\" mask=\"url(#css)\"/><rect x=\"24\" width=\"6\" height=\"6\" mask=\"url(#invalid)\"/></svg>";
	static const char svg_mask_luminance_batch_32[] =
		"<svg viewBox=\"0 0 100 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<linearGradient id=\"lg\" gradientUnits=\"userSpaceOnUse\" x1=\"50\" y1=\"0\" x2=\"60\" y2=\"0\"><stop offset=\"0\" stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<radialGradient id=\"rg\" gradientUnits=\"userSpaceOnUse\" cx=\"65\" cy=\"5\" r=\"5\"><stop offset=\"0\" stop-color=\"white\"/><stop offset=\"1\" stop-color=\"black\"/></radialGradient>"
		"<rect id=\"maskRef32\" width=\"10\" height=\"10\" fill=\"white\"/>"
		"<mask id=\"black32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"50\" height=\"10\"><rect width=\"50\" height=\"10\" fill=\"black\"/></mask>"
		"<mask id=\"alpha32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"10\" y=\"0\" width=\"10\" height=\"10\" mask-type=\"Alpha\"><rect x=\"10\" width=\"10\" height=\"10\" fill=\"black\"/></mask>"
		"<mask id=\"linear32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"50\" y=\"0\" width=\"10\" height=\"10\"><rect x=\"50\" width=\"10\" height=\"10\" fill=\"url(#lg)\"/></mask>"
		"<mask id=\"radial32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"60\" y=\"0\" width=\"10\" height=\"10\"><rect x=\"60\" width=\"10\" height=\"10\" fill=\"url(#rg)\"/></mask>"
		"<mask id=\"stroke32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"70\" y=\"0\" width=\"10\" height=\"10\"><rect x=\"71\" y=\"1\" width=\"8\" height=\"8\" fill=\"black\" stroke=\"white\" stroke-width=\"2\"/></mask>"
		"<mask id=\"box32\" maskContentUnits=\"objectBoundingBox\"><rect width=\"0.5\" height=\"1\" fill=\"white\"/></mask>"
		"<mask id=\"use32\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"90\" y=\"0\" width=\"10\" height=\"10\"><use href=\"#maskRef32\" x=\"90\"/></mask>"
		"</defs>"
		"<rect width=\"10\" height=\"10\" fill=\"#38bdf8\" mask=\"url(#black32)\"/>"
		"<rect x=\"10\" width=\"10\" height=\"10\" fill=\"#22c55e\" mask=\"url(#alpha32)\"/>"
		"<text x=\"20\" y=\"9\" font-size=\"10\" fill=\"#f59e0b\" mask=\"url(#black32)\">W</text>"
		"<image x=\"30\" width=\"10\" height=\"10\" mask=\"url(#black32)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='10' height='10' fill='%23a78bfa'/%3E%3C/svg%3E\"/>"
		"<image x=\"40\" width=\"10\" height=\"10\" preserveAspectRatio=\"none\" mask=\"url(#black32)\" href=\"data:image/png;base64," XGE_TEST_RASTER_PNG "\"/>"
		"<rect x=\"50\" width=\"10\" height=\"10\" fill=\"#ef4444\" mask=\"url(#linear32)\"/>"
		"<rect x=\"60\" width=\"10\" height=\"10\" fill=\"#14b8a6\" mask=\"url(#radial32)\"/>"
		"<rect x=\"70\" width=\"10\" height=\"10\" fill=\"#e879f9\" mask=\"url(#stroke32)\"/>"
		"<rect x=\"80\" width=\"10\" height=\"10\" fill=\"#f97316\" mask=\"url(#box32)\"/>"
		"<rect x=\"90\" width=\"10\" height=\"10\" fill=\"#84cc16\" mask=\"url(#use32)\"/>"
		"</svg>";
	static const char svg_nested_image_viewport_batch_34[] =
		"<svg viewBox=\"0 0 100 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<mask id=\"a\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"10\" mask-type=\"alpha\"><rect width=\"100\" height=\"10\" fill=\"white\"/></mask>"
		"<mask id=\"l\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"10\"><rect width=\"100\" height=\"10\" fill=\"red\"/></mask>"
		"</defs>"
		"<image x=\"0\" y=\"0\" width=\"10\" height=\"10\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"10\" y=\"0\" width=\"10\" height=\"10\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"20\" y=\"0\" width=\"10\" height=\"10\" mask=\"url(#l)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"30\" y=\"0\" width=\"10\" height=\"10\" preserveAspectRatio=\"none\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"40\" y=\"0\" width=\"10\" height=\"8\" preserveAspectRatio=\"xMinYMin meet\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"50\" y=\"0\" width=\"10\" height=\"8\" preserveAspectRatio=\"xMidYMid meet\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"60\" y=\"0\" width=\"10\" height=\"8\" preserveAspectRatio=\"xMaxYMax meet\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"70\" y=\"0\" width=\"8\" height=\"10\" preserveAspectRatio=\"xMinYMin slice\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect x='5' width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<image x=\"80\" y=\"0\" width=\"10\" height=\"10\" transform=\"translate(1 0)\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/>"
		"<g transform=\"translate(1 0)\"><image x=\"90\" y=\"0\" width=\"9\" height=\"10\" mask=\"url(#a)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 9 10' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='5' height='5' fill='red'/%3E%3C/svg%3E\"/></g>"
		"</svg>";
	static const char svg_image_transform_stroke_batch_35[] =
		"<svg viewBox=\"0 0 100 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image width=\"10\" height=\"10\" transform=\"rotate(10 5 5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<g transform=\"rotate(-10 15 5)\"><image x=\"10\" width=\"10\" height=\"10\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/></g>"
		"<image x=\"20\" width=\"10\" height=\"10\" transform=\"translate(25 5) skewX(10) translate(-25 -5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"30\" width=\"10\" height=\"10\" transform=\"translate(35 5) skewY(10) translate(-35 -5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"40\" width=\"10\" height=\"10\" transform=\"matrix(1 .1 .1 1 -0.5 -4.5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"50\" width=\"10\" height=\"10\" transform=\"translate(1 0)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"60\" width=\"10\" height=\"10\" transform=\"translate(65 5) scale(.9) translate(-65 -5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"70\" width=\"10\" height=\"10\" transform=\"translate(75 5) scale(.8 1) translate(-75 -5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"80\" width=\"10\" height=\"10\" transform=\"rotate(10 85 5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cdefs%3E%3ClinearGradient id='g'%3E%3Cstop stop-color='red'/%3E%3Cstop offset='1' stop-color='blue'/%3E%3C/linearGradient%3E%3C/defs%3E%3Cpath d='M0 5L10 5' fill='none' stroke='url(%23g)' stroke-width='2'/%3E%3C/svg%3E\"/>"
		"<image x=\"90\" width=\"10\" height=\"10\" transform=\"rotate(-10 95 5)\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 10 10' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M0 5L10 5' fill='none' stroke='red' stroke-width='2' stroke-dasharray='100 1'/%3E%3C/svg%3E\"/>"
		"</svg>";
	static const char svg_mask_gradient_batch_33[] =
		"<svg viewBox=\"0 0 400 40\" xmlns=\"http://www.w3.org/2000/svg\"><defs>"
		"<linearGradient id=\"linearUser33\" gradientUnits=\"userSpaceOnUse\" x1=\"0\" y1=\"0\" x2=\"40\" y2=\"0\"><stop stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<radialGradient id=\"radialUser33\" gradientUnits=\"userSpaceOnUse\" cx=\"100\" cy=\"20\" r=\"16\"><stop stop-color=\"white\"/><stop offset=\"1\" stop-color=\"black\"/></radialGradient>"
		"<linearGradient id=\"linearBox33\"><stop stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<linearGradient id=\"linearTransform33\" gradientUnits=\"userSpaceOnUse\" x1=\"320\" y1=\"0\" x2=\"360\" y2=\"40\" gradientTransform=\"rotate(15 340 20)\"><stop stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<linearGradient id=\"strokeGradient33\" gradientUnits=\"userSpaceOnUse\" x1=\"360\" y1=\"0\" x2=\"400\" y2=\"40\"><stop stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<mask id=\"lu33\" maskUnits=\"userSpaceOnUse\" x=\"0\" width=\"40\" height=\"40\"><rect width=\"40\" height=\"40\" fill=\"url(#linearUser33)\"/></mask>"
		"<mask id=\"au33\" maskUnits=\"userSpaceOnUse\" x=\"40\" width=\"40\" height=\"40\" mask-type=\"Alpha\"><rect x=\"40\" width=\"40\" height=\"40\" fill=\"url(#linearUser33)\"/></mask>"
		"<mask id=\"lr33\" maskUnits=\"userSpaceOnUse\" x=\"80\" width=\"40\" height=\"40\"><rect x=\"80\" width=\"40\" height=\"40\" fill=\"url(#radialUser33)\"/></mask>"
		"<mask id=\"ar33\" maskUnits=\"userSpaceOnUse\" x=\"120\" width=\"40\" height=\"40\" mask-type=\"Alpha\"><rect x=\"120\" width=\"40\" height=\"40\" fill=\"url(#radialUser33)\"/></mask>"
		"<mask id=\"lb33\" maskUnits=\"userSpaceOnUse\" x=\"160\" width=\"40\" height=\"40\"><rect x=\"160\" width=\"40\" height=\"40\" fill=\"url(#linearBox33)\"/></mask>"
		"<mask id=\"ab33\" maskUnits=\"userSpaceOnUse\" x=\"200\" width=\"40\" height=\"40\" mask-type=\"Alpha\"><rect x=\"200\" width=\"40\" height=\"40\" fill=\"url(#linearBox33)\"/></mask>"
		"<mask id=\"lf33\" maskUnits=\"userSpaceOnUse\" x=\"240\" width=\"40\" height=\"40\"><rect x=\"240\" width=\"40\" height=\"40\" fill=\"url(#forwardGradient33)\"/></mask>"
		"<mask id=\"af33\" maskUnits=\"userSpaceOnUse\" x=\"280\" width=\"40\" height=\"40\" mask-type=\"Alpha\"><rect x=\"280\" width=\"40\" height=\"40\" fill=\"url(#forwardGradient33)\"/></mask>"
		"<linearGradient id=\"forwardGradient33\" gradientUnits=\"userSpaceOnUse\" x1=\"240\" y1=\"0\" x2=\"320\" y2=\"40\"><stop stop-color=\"black\"/><stop offset=\"1\" stop-color=\"white\"/></linearGradient>"
		"<mask id=\"lt33\" maskUnits=\"userSpaceOnUse\" x=\"320\" width=\"40\" height=\"40\"><rect x=\"320\" width=\"40\" height=\"40\" fill=\"url(#linearTransform33)\"/></mask>"
		"<mask id=\"ls33\" maskUnits=\"userSpaceOnUse\" x=\"360\" width=\"40\" height=\"40\"><rect x=\"368\" y=\"8\" width=\"24\" height=\"24\" fill=\"none\" stroke=\"url(#strokeGradient33)\" stroke-width=\"8\"/></mask>"
		"</defs><g fill=\"#38bdf8\">"
		"<rect width=\"40\" height=\"40\" mask=\"url(#lu33)\"/><rect x=\"40\" width=\"40\" height=\"40\" mask=\"url(#au33)\"/>"
		"<rect x=\"80\" width=\"40\" height=\"40\" mask=\"url(#lr33)\"/><rect x=\"120\" width=\"40\" height=\"40\" mask=\"url(#ar33)\"/>"
		"<rect x=\"160\" width=\"40\" height=\"40\" mask=\"url(#lb33)\"/><rect x=\"200\" width=\"40\" height=\"40\" mask=\"url(#ab33)\"/>"
		"<rect x=\"240\" width=\"40\" height=\"40\" mask=\"url(#lf33)\"/><rect x=\"280\" width=\"40\" height=\"40\" mask=\"url(#af33)\"/>"
		"<rect x=\"320\" width=\"40\" height=\"40\" mask=\"url(#lt33)\"/><rect x=\"360\" width=\"40\" height=\"40\" mask=\"url(#ls33)\"/>"
		"</g></svg>";
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
		"<svg viewBox=\"0 0 36 12\">"
		"<g fill=\"#ef4444\" stroke=\"#38bdf8\" stroke-width=\"2\">"
		"<rect x=\"1\" y=\"2\" width=\"5\" height=\"6\" fill=\"bad\"/>"
		"<rect x=\"8\" y=\"2\" width=\"5\" height=\"6\" stroke=\"bad\"/>"
		"<rect x=\"15\" y=\"2\" width=\"5\" height=\"6\"/>"
		"<rect x=\"22\" y=\"2\" width=\"5\" height=\"6\" fill=\"url(#missingPaint) bad\"/>"
		"<rect x=\"29\" y=\"2\" width=\"5\" height=\"6\" fill=\"url(#missingPaint) #22c55e\"/>"
		"</g>"
		"</svg>";
	static const char svg_paint_invalid_gradient[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<linearGradient id=\"fillGrad\"><stop offset=\"0\" stop-color=\"#ef4444\"/><stop offset=\"1\" stop-color=\"#facc15\"/></linearGradient>"
		"<linearGradient id=\"strokeGrad\"><stop offset=\"0\" stop-color=\"#38bdf8\"/><stop offset=\"1\" stop-color=\"#a78bfa\"/></linearGradient>"
		"</defs>"
		"<g fill=\"url(#fillGrad)\" stroke=\"url(#strokeGrad)\" stroke-width=\"2\">"
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
	static const char svg_length_suffix[] =
		"<svg viewBox=\"0 0 40 20\">"
		"<rect x=\"1junk\" y=\"1junk\" width=\"10junk\" height=\"6junk\" fill=\"#38bdf8\"/>"
		"<circle cx=\"18junk\" cy=\"4junk\" r=\"3junk\" fill=\"#f97316\"/>"
		"<path d=\"M2 14 H30\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"2junk\"/>"
		"<text x=\"1junk\" y=\"19junk\" font-size=\"5junk\" fill=\"#ffffff\">OK</text>"
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
	static const char svg_stroke_dashoffset_percent[] =
		"<svg viewBox=\"0 0 360 240\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"18\" stroke-dasharray=\"80 80\">"
		"<path d=\"M40 70 H320\" stroke-dashoffset=\"50%\"/>"
		"<path d=\"M40 120 H320\" stroke-dashoffset=\"152.97059\"/>"
		"</g>"
		"</svg>";
	static const char svg_path_length[] =
		"<svg viewBox=\"0 0 360 220\">"
		"<defs>"
		"<path id=\"declaredTextPath\" d=\"M40 145 H240\" pathLength=\"100\"/>"
		"<path id=\"actualTextPath\" d=\"M40 180 H240\"/>"
		"</defs>"
		"<path d=\"M40 45 H240\" pathLength=\"100\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"12\" stroke-dasharray=\"25 25\"/>"
		"<text font-size=\"18\"><textPath href=\"#declaredTextPath\" startOffset=\"50\">TEXT</textPath></text>"
		"<text font-size=\"18\"><textPath href=\"#actualTextPath\" startOffset=\"100\">TEXT</textPath></text>"
		"</svg>";
	static const char svg_path_length_dash_hit[] =
		"<svg viewBox=\"0 0 220 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<path d=\"M10 10 H210\" pathLength=\"100\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"4\" stroke-dasharray=\"25 25\" stroke-linecap=\"butt\"/>"
		"</svg>";
	static const char svg_line_path_length_dash_hit[] =
		"<svg viewBox=\"0 0 220 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<line x1=\"10\" y1=\"10\" x2=\"210\" y2=\"10\" pathLength=\"100\" stroke=\"#38bdf8\" stroke-width=\"4\" stroke-dasharray=\"25 25\" stroke-linecap=\"butt\"/>"
		"</svg>";
	static const char svg_stroke_duplicate_invalid[] =
		"<svg viewBox=\"0 0 48 32\">"
		"<g fill=\"none\" stroke-linecap=\"round\">"
		"<path d=\"M2 4 H20\" stroke=\"#38bdf8\" style=\"stroke-width:4px;stroke-width:bad\"/>"
		"<path d=\"M28 4 H46\" stroke=\"#38bdf8\" style=\"stroke-width:bad;stroke-width:4px\"/>"
		"<path d=\"M2 14 L10 7 L20 14\" stroke=\"#a78bfa\" stroke-width=\"4\" stroke-linejoin=\"miter\" style=\"stroke-miterlimit:8;stroke-miterlimit:bad\"/>"
		"<path d=\"M28 14 L36 7 L46 14\" stroke=\"#a78bfa\" stroke-width=\"4\" stroke-linejoin=\"miter\" style=\"stroke-miterlimit:bad;stroke-miterlimit:8\"/>"
		"<path d=\"M2 22 H20\" stroke=\"#22c55e\" stroke-width=\"3\" style=\"stroke-dasharray:5 2;stroke-dasharray:bad\"/>"
		"<path d=\"M28 22 H46\" stroke=\"#22c55e\" stroke-width=\"3\" style=\"stroke-dasharray:bad;stroke-dasharray:5 2\"/>"
		"<path d=\"M2 29 H20\" stroke=\"#f97316\" stroke-width=\"3\" stroke-dasharray=\"5 2\" style=\"stroke-dashoffset:3;stroke-dashoffset:nan\"/>"
		"<path d=\"M28 29 H46\" stroke=\"#f97316\" stroke-width=\"3\" stroke-dasharray=\"5 2\" style=\"stroke-dashoffset:nan;stroke-dashoffset:3\"/>"
		"</g>"
		"</svg>";
	static const char svg_stroke_dash_inherit[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"3 2\" stroke-dashoffset=\"2\">"
		"<path d=\"M2 3 H22\" stroke-dasharray=\"inherit\"/>"
		"<path d=\"M2 6 H22\" stroke-dashoffset=\"inherit\"/>"
		"<path d=\"M2 9 H22\" style=\"stroke-dasharray:inherit;stroke-dashoffset:inherit\"/>"
		"</g>"
		"</svg>";
	static const char svg_style_inherit_core[] =
		"<svg viewBox=\"0 0 32 20\">"
		"<style>.low{fill-rule:nonzero;clip-rule:nonzero;stroke-linecap:butt;stroke-linejoin:miter;stroke-width:1;stroke-opacity:.25;paint-order:normal;stroke-dasharray:none}.high{fill-rule:inherit;clip-rule:inherit;stroke-linecap:inherit;stroke-linejoin:inherit;stroke-width:inherit;stroke-opacity:inherit;paint-order:inherit;stroke-dasharray:inherit}</style>"
		"<g fill=\"#38bdf8\" fill-rule=\"evenodd\" stroke=\"#f97316\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-opacity=\"1\" stroke-dasharray=\"4 2\" paint-order=\"stroke fill\">"
		"<path class=\"low high\" d=\"M2 2h12v12h-12z M5 5h6v6h-6z\"/>"
		"<path class=\"low high\" d=\"M18 14 L23 2 L28 14\" fill=\"none\"/>"
		"</g>"
		"</svg>";
	static const char svg_xml_markup_boundaries[] =
		"<?xml version=\"1.0\"?>"
		"<!DOCTYPE svg [<!ENTITY sample \">\"><!ELEMENT svg ANY>]>"
		"<svg viewBox=\"0 0 24 12\">"
		"<!-- ignored <rect x=\"0\" y=\"0\" width=\"24\" height=\"12\" fill=\"red\"> marker -->"
		"<style><![CDATA[.ok{fill:goldenrod;stroke:none}]]></style>"
		"<rect class=\"ok\" x=\"2\" y=\"2\" width=\"20\" height=\"8\"/>"
		"</svg>";
	static const char svg_xml_entities[] =
		"<svg viewBox=\"0 0 32 16\">"
		"<style>.entityClass{fill:#38bdf8;stroke:none}</style>"
		"<defs><g id=\"entityTarget\"><rect class=\"entityCl&#x61;ss\" fill=\"none\" x=\"1\" y=\"1\" width=\"6\" height=\"6\"/></g></defs>"
		"<use href=\"&#x23;entityTarget\" x=\"2\" y=\"1\"/>"
		"<text x=\"2\" y=\"13\" font-size=\"4\" fill=\"#f97316\" stroke=\"none\">A&amp;B &#60; C &#x3E;</text>"
		"</svg>";
	static const char svg_xml_named_href_entity[] =
		"<svg viewBox=\"0 0 16 8\">"
		"<defs><rect id=\"namedTarget\" x=\"2\" y=\"2\" width=\"6\" height=\"4\"/></defs>"
		"<use href=\"&#035;namedTarget\"/>"
		"</svg>";
	static const char svg_text_baseline_ignored[] =
		"<svg viewBox=\"0 0 64 20\">"
		"<path d=\"M2 12 H62\" fill=\"none\" stroke=\"#334155\"/>"
		"<text x=\"6\" y=\"12\" font-size=\"8\" fill=\"#e2e8f0\">A<tspan baseline-shift=\"super\" fill=\"#38bdf8\">B</tspan><tspan baseline-shift=\"sub\" fill=\"#f97316\">C</tspan><tspan baseline-shift=\"baseline\" fill=\"#e2e8f0\">D</tspan></text>"
		"<text x=\"34\" y=\"12\" font-size=\"8\" fill=\"#a78bfa\"><tspan baseline-shift=\"25%\">P</tspan><tspan baseline-shift=\"-2px\">N</tspan></text>"
		"</svg>";
	static const char svg_symbol_overflow[] =
		"<svg viewBox=\"0 0 64 24\">"
		"<defs>"
		"<symbol id=\"clipSym\" viewBox=\"0 0 10 10\"><rect x=\"-4\" y=\"2\" width=\"18\" height=\"6\" fill=\"#38bdf8\"/></symbol>"
		"<symbol id=\"visibleSym\" viewBox=\"0 0 10 10\" style=\"overflow:visible\"><rect x=\"-4\" y=\"2\" width=\"18\" height=\"6\" fill=\"#f97316\"/></symbol>"
		"</defs>"
		"<use href=\"#clipSym\" x=\"8\" y=\"4\" width=\"16\" height=\"16\"/>"
		"<use href=\"#visibleSym\" x=\"40\" y=\"4\" width=\"16\" height=\"16\"/>"
		"</svg>";
	static const char svg_nested_svg_viewport_clip[] =
		"<svg viewBox=\"0 0 64 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<svg x=\"4\" y=\"4\" width=\"16\" height=\"16\" viewBox=\"0 0 16 16\"><rect x=\"-8\" y=\"4\" width=\"32\" height=\"8\" fill=\"#38bdf8\"/></svg>"
		"<svg x=\"32\" y=\"4\" width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" overflow=\"visible\"><rect x=\"-8\" y=\"4\" width=\"32\" height=\"8\" fill=\"#f43f5e\"/></svg>"
		"<svg x=\"4\" y=\"22\" width=\"16\" height=\"4\"><rect x=\"-8\" y=\"0\" width=\"32\" height=\"4\" fill=\"#22c55e\"/></svg>"
		"</svg>";
	static const char svg_clip_mask_nested_svg[] =
		"<svg viewBox=\"0 0 96 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"clipSvg\"><svg x=\"6\" y=\"4\" width=\"20\" height=\"14\" viewBox=\"0 0 20 14\"><rect x=\"-8\" y=\"2\" width=\"36\" height=\"10\"/></svg></clipPath>"
		"<mask id=\"maskSvg\" maskUnits=\"userSpaceOnUse\" x=\"52\" y=\"4\" width=\"20\" height=\"14\"><svg x=\"52\" y=\"4\" width=\"20\" height=\"14\" viewBox=\"0 0 20 14\"><rect x=\"-8\" y=\"2\" width=\"36\" height=\"10\" fill=\"white\"/></svg></mask>"
		"</defs>"
		"<rect x=\"0\" y=\"0\" width=\"96\" height=\"32\" fill=\"#0f172a\"/>"
		"<rect x=\"0\" y=\"0\" width=\"34\" height=\"24\" fill=\"#22c55e\" clip-path=\"url(#clipSvg)\"/>"
		"<rect x=\"46\" y=\"0\" width=\"34\" height=\"24\" fill=\"#38bdf8\" mask=\"url(#maskSvg)\"/>"
		"</svg>";
	static const char svg_clip_nested_group_ignored[] =
		"<svg viewBox=\"0 0 16 8\">"
		"<defs><clipPath id=\"nestedClip\"><svg><rect width=\"8\" height=\"8\"/></svg></clipPath></defs>"
		"<rect width=\"16\" height=\"8\" clip-path=\"url(#nestedClip)\"/>"
		"</svg>";
	static const char svg_mask_nested_group_position[] =
		"<svg viewBox=\"0 0 16 8\">"
		"<defs><mask id=\"nestedMask\" maskUnits=\"userSpaceOnUse\" x=\"8\" y=\"0\" width=\"8\" height=\"8\"><svg x=\"8\"><rect width=\"8\" height=\"8\" fill=\"white\"/></svg></mask></defs>"
		"<rect x=\"8\" width=\"8\" height=\"8\" mask=\"url(#nestedMask)\"/>"
		"</svg>";
	static const char svg_rect_invalid_radii[] =
		"<svg viewBox=\"0 0 64 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"clip\"><rect x=\"0\" y=\"3\" width=\"18\" height=\"18\" rx=\"-4\"/></clipPath>"
		"<mask id=\"mask\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"64\" height=\"24\"><rect x=\"42\" y=\"4\" width=\"16\" height=\"16\" rx=\"-2\" ry=\"5\" fill=\"white\"/></mask>"
		"</defs>"
		"<rect x=\"2\" y=\"4\" width=\"12\" height=\"16\" rx=\"-3\" fill=\"#38bdf8\"/>"
		"<rect x=\"18\" y=\"4\" width=\"16\" height=\"16\" rx=\"-3\" ry=\"5\" fill=\"#f97316\"/>"
		"<rect x=\"-4\" y=\"4\" width=\"24\" height=\"16\" clip-path=\"url(#clip)\" fill=\"#22c55e\" fill-opacity=\"0.7\"/>"
		"<rect x=\"42\" y=\"4\" width=\"16\" height=\"16\" fill=\"#a855f7\" mask=\"url(#mask)\"/>"
		"</svg>";
	static const char svg_rect_radius_thorvg_compat[] =
		"<svg viewBox=\"0 0 32 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"8\" y=\"4\" width=\"16\" height=\"10\" rx=\"-3\" ry=\"4\" fill=\"#38bdf8\"/>"
		"</svg>";
	static const char svg_clip_rect_radius_thorvg_compat[] =
		"<svg viewBox=\"0 0 32 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><clipPath id=\"clip\"><rect x=\"8\" y=\"4\" width=\"16\" height=\"10\" rx=\"-3\" ry=\"4\"/></clipPath></defs>"
		"<rect x=\"0\" y=\"0\" width=\"32\" height=\"18\" clip-path=\"url(#clip)\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_mask_rect_radius_thorvg_compat[] =
		"<svg viewBox=\"0 0 32 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><mask id=\"mask\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"32\" height=\"18\"><rect x=\"8\" y=\"4\" width=\"16\" height=\"10\" rx=\"-3\" ry=\"4\" fill=\"white\"/></mask></defs>"
		"<rect x=\"0\" y=\"0\" width=\"32\" height=\"18\" mask=\"url(#mask)\" fill=\"#a855f7\"/>"
		"</svg>";
	static const char svg_rect_dash_start_thorvg_compat[] =
		"<svg viewBox=\"0 0 32 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"4\" y=\"4\" width=\"16\" height=\"8\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"4 100\" stroke-linecap=\"butt\"/>"
		"</svg>";
	static const char svg_circle_dash_start_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<circle cx=\"12\" cy=\"12\" r=\"6\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"4 100\" stroke-linecap=\"butt\"/>"
		"</svg>";
	static const char svg_ellipse_dash_start_thorvg_compat[] =
		"<svg viewBox=\"0 0 32 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<ellipse cx=\"14\" cy=\"12\" rx=\"8\" ry=\"4\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\" stroke-dasharray=\"4 100\" stroke-linecap=\"butt\"/>"
		"</svg>";
	static const char svg_points_prefix_thorvg_compat[] =
		"<svg viewBox=\"0 0 48 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<polyline points=\"2,4 14,4 broken 44,4\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\"/>"
		"<polygon points=\"2,10 10,10 10,16 bad 2,16\" fill=\"#22c55e\" stroke=\"none\"/>"
		"<polyline points=\"20,8 32,8 40\" fill=\"none\" stroke=\"#facc15\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_path_prefix_thorvg_compat[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<path d=\"M2 4 L16 4 bad L44 4\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"2\"/>"
		"<path d=\"M2 10 L10 10 L10 16 bad L2 16 Z\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"2\"/>"
		"<path d=\"M22 8 Z 40 8\" fill=\"none\" stroke=\"#facc15\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_points_invalid[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"badClip\"><polygon points=\"0,0 10,bad 10,10\"/></clipPath>"
		"<mask id=\"badMask\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"48\" height=\"18\"><polyline points=\"0,0 8,0 broken\" stroke=\"white\" fill=\"none\"/></mask>"
		"</defs>"
		"<polygon points=\"1,1 8,bad 8,8\" fill=\"#ef4444\"/>"
		"<polyline points=\"1,14 10,14 bad\" fill=\"none\" stroke=\"#ef4444\"/>"
		"<rect x=\"13\" y=\"3\" width=\"8\" height=\"8\" fill=\"#22c55e\"/>"
		"<polygon points=\"25,3 35,3 30,12\" fill=\"#38bdf8\"/>"
		"<polyline points=\"38,4 45,8 38,12\" fill=\"none\" stroke=\"#facc15\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_points_invalid_clip_mask[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"clipMixed\"><polygon points=\"0,0 10,bad 10,10\"/><polygon points=\"2,2 14,2 14,14 2,14\"/></clipPath>"
		"<mask id=\"maskMixed\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"48\" height=\"18\"><polyline points=\"0,0 broken\" stroke=\"white\" fill=\"none\"/><rect x=\"20\" y=\"2\" width=\"12\" height=\"12\" fill=\"white\"/></mask>"
		"</defs>"
		"<rect x=\"0\" y=\"1\" width=\"16\" height=\"16\" fill=\"#38bdf8\" clip-path=\"url(#clipMixed)\"/>"
		"<rect x=\"18\" y=\"1\" width=\"16\" height=\"16\" fill=\"#22c55e\" mask=\"url(#maskMixed)\"/>"
		"<polygon points=\"38,3 46,8 38,13\" fill=\"#facc15\"/>"
		"</svg>";
	static const char svg_path_invalid[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"badClip\"><path d=\"M0 0 Lbad 10 L10 10 Z\"/></clipPath>"
		"<mask id=\"badMask\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"48\" height=\"18\"><path d=\"M0 0 Lnan 4 L4 4 Z\" fill=\"white\"/></mask>"
		"</defs>"
		"<path d=\"M1 1 Lbad 8 L8 8 Z\" fill=\"#ef4444\"/>"
		"<path d=\"M13 3 L21 3 L17 11 Z\" fill=\"#22c55e\"/>"
		"<rect x=\"25\" y=\"3\" width=\"10\" height=\"8\" fill=\"#38bdf8\"/>"
		"<path d=\"M39 4 L46 8 L39 12\" fill=\"none\" stroke=\"#facc15\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_path_incomplete_commands[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"48\" height=\"18\" fill=\"#0f172a\"/>"
		"<path d=\"M4 4 L\" fill=\"none\" stroke=\"#ef4444\" stroke-width=\"2\"/>"
		"<path d=\"M4 9 Z 1 2\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"2\"/>"
		"<path d=\"M4 14 Z M8 14 H20\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_path_number_grammar[] =
		"<svg viewBox=\"0 0 48 18\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"48\" height=\"18\" fill=\"#0f172a\"/>"
		"<path d=\"M0x1 4 H24\" fill=\"none\" stroke=\"#ef4444\" stroke-width=\"2\"/>"
		"<path d=\"M0x1p2 9 H24\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"2\"/>"
		"<path d=\"M.5 14 L5. 14 L10+5 L20 1e1\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_path_quadratic[] =
		"<svg viewBox=\"0 0 220 120\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"220\" height=\"120\" fill=\"#0f172a\"/>"
		"<path d=\"M8 58 Q38 10 68 58 T128 58 T188 58\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"7\" stroke-linecap=\"round\"/>"
		"<path d=\"M8 78 q30 -28 60 0 t60 0 t60 0\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
		"<circle cx=\"38\" cy=\"10\" r=\"3\" fill=\"#22c55e\"/>"
		"<circle cx=\"98\" cy=\"106\" r=\"3\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_stroke_zero_length_caps[] =
		"<svg viewBox=\"0 0 96 36\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"96\" height=\"36\" fill=\"#0f172a\"/>"
		"<g stroke=\"#334155\" stroke-width=\"1\"><path d=\"M18 8 H78\"/><path d=\"M18 18 H78\"/><path d=\"M18 28 H78\"/></g>"
		"<line x1=\"28\" y1=\"8\" x2=\"28\" y2=\"8\" stroke=\"#ef4444\" stroke-width=\"12\" stroke-linecap=\"butt\"/>"
		"<line x1=\"48\" y1=\"18\" x2=\"48\" y2=\"18\" stroke=\"#38bdf8\" stroke-width=\"12\" stroke-linecap=\"round\"/>"
		"<line x1=\"68\" y1=\"28\" x2=\"68\" y2=\"28\" stroke=\"#f97316\" stroke-width=\"12\" stroke-linecap=\"square\"/>"
		"</svg>";
	static const char svg_stroke_zero_length_caps_hit[] =
		"<svg viewBox=\"0 0 96 36\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<line x1=\"28\" y1=\"8\" x2=\"28\" y2=\"8\" stroke=\"#ef4444\" stroke-width=\"12\" stroke-linecap=\"butt\"/>"
		"<line x1=\"48\" y1=\"18\" x2=\"48\" y2=\"18\" stroke=\"#38bdf8\" stroke-width=\"12\" stroke-linecap=\"round\"/>"
		"<line x1=\"68\" y1=\"28\" x2=\"68\" y2=\"28\" stroke=\"#f97316\" stroke-width=\"12\" stroke-linecap=\"square\"/>"
		"</svg>";
	static const char svg_stroke_join_contains[] =
		"<svg viewBox=\"0 0 80 30\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<path d=\"M10 20 L20 10 L30 20\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"4\" stroke-linecap=\"butt\" stroke-linejoin=\"miter\" stroke-miterlimit=\"8\"/>"
		"<path d=\"M50 20 L60 10 L70 20\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"4\" stroke-linecap=\"butt\" stroke-linejoin=\"bevel\"/>"
		"</svg>";
	static const char svg_number_grammar_attrs[] =
		"<svg viewBox=\"0 0 100 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"100\" height=\"24\" fill=\"#0f172a\"/>"
		"<rect x=\"4\" y=\"4\" width=\"10\" height=\"8\" fill=\"#ef4444\" transform=\"translate(0x10,0)\"/>"
		"<rect x=\"4\" y=\"14\" width=\"10\" height=\"6\" fill=\"#f97316\" opacity=\"0x0\"/>"
		"<polyline points=\"0x1,2 24,2\" fill=\"none\" stroke=\"#a855f7\" stroke-width=\"2\"/>"
		"<rect x=\"40\" y=\"4\" width=\"10\" height=\"8\" fill=\"#22c55e\"/>"
		"<rect x=\"60\" y=\"4\" width=\"8\" height=\"8\" fill=\"#38bdf8\" transform=\"translate(1e1,0)\"/>"
		"</svg>";
	static const char svg_length_number_grammar[] =
		"<svg viewBox=\"0 0 80 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"80\" height=\"24\" fill=\"#0f172a\"/>"
		"<rect x=\"0x8\" y=\"4\" width=\"10\" height=\"8\" fill=\"#ef4444\"/>"
		"<rect x=\"24\" y=\"4\" width=\"0x10\" height=\"8\" fill=\"#f97316\"/>"
		"<path d=\"M4 18 H40\" fill=\"none\" stroke=\"#a855f7\" stroke-width=\"0x4\"/>"
		"<rect x=\"50\" y=\"4\" width=\"1e1\" height=\"8\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_circle_ellipse_attrs[] =
		"<svg viewBox=\"0 0 80 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect width=\"80\" height=\"24\" fill=\"#0b1020\"/>"
		"<circle cx=\"10\" cy=\"12\" r=\"7\" rx=\"14\" ry=\"2\" fill=\"#38bdf8\"/>"
		"<ellipse cx=\"30\" cy=\"12\" r=\"7\" fill=\"#ef4444\"/>"
		"<ellipse cx=\"50\" cy=\"12\" rx=\"8\" ry=\"4\" fill=\"#22c55e\"/>"
		"<defs><clipPath id=\"clipCircle\"><circle cx=\"68\" cy=\"12\" r=\"7\" rx=\"13\" ry=\"2\"/></clipPath></defs>"
		"<rect x=\"60\" y=\"4\" width=\"16\" height=\"16\" fill=\"#facc15\" clip-path=\"url(#clipCircle)\"/>"
		"</svg>";
	static const char svg_ellipse_bounds[] =
		"<svg viewBox=\"0 0 64 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<ellipse cx=\"20\" cy=\"14\" rx=\"9\" ry=\"5\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_mix_blend_mode[] =
		"<svg viewBox=\"0 0 64 18\">"
		"<style>.screen{mix-blend-mode:screen}.darken{mix-blend-mode:darken}.lighten{mix-blend-mode:lighten}.inherit{mix-blend-mode:inherit}"
		".overlay{mix-blend-mode:overlay}.dodge{mix-blend-mode:color-dodge}.burn{mix-blend-mode:color-burn}.hard{mix-blend-mode:hard-light}"
		".soft{mix-blend-mode:soft-light}.difference{mix-blend-mode:difference}.exclusion{mix-blend-mode:exclusion}.hue{mix-blend-mode:hue}"
		".saturation{mix-blend-mode:saturation}.color{mix-blend-mode:color}.luminosity{mix-blend-mode:luminosity}</style>"
		"<rect x=\"0\" y=\"0\" width=\"64\" height=\"18\" fill=\"#1e40af\"/>"
		"<rect x=\"4\" y=\"3\" width=\"12\" height=\"12\" fill=\"#f97316\" mix-blend-mode=\"multiply\"/>"
		"<rect class=\"screen\" x=\"16\" y=\"3\" width=\"12\" height=\"12\" fill=\"#22c55e\"/>"
		"<rect class=\"darken\" x=\"28\" y=\"3\" width=\"12\" height=\"12\" fill=\"#facc15\"/>"
		"<rect class=\"lighten\" x=\"40\" y=\"3\" width=\"12\" height=\"12\" fill=\"#ef4444\"/>"
		"<g style=\"mix-blend-mode:normal\"><rect class=\"inherit\" x=\"52\" y=\"3\" width=\"12\" height=\"12\" fill=\"#e2e8f0\"/></g>"
		"<g opacity=\"0\"><rect class=\"overlay dodge burn hard soft difference exclusion hue saturation color luminosity\" width=\"1\" height=\"1\"/></g>"
		"</svg>";
	static const char svg_blend_group_composition[] =
		"<svg viewBox=\"0 0 80 32\" opacity=\"0.85\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<linearGradient id=\"fade\"><stop stop-color=\"#ef4444\"/><stop offset=\"1\" stop-color=\"#3b82f6\"/></linearGradient>"
		"<g id=\"tile\"><rect width=\"8\" height=\"6\"/><circle cx=\"10\" cy=\"3\" r=\"3\"/></g>"
		"</defs>"
		"<g opacity=\"0.5\" mix-blend-mode=\"multiply\" transform=\"translate(4 5)\">"
		"<rect width=\"12\" height=\"8\" fill=\"url(#fade)\"/>"
		"<g mix-blend-mode=\"screen\"><circle cx=\"16\" cy=\"4\" r=\"4\" fill=\"#22c55e\"/></g>"
		"</g>"
		"<use href=\"#tile\" x=\"40\" y=\"10\" opacity=\"0.6\" mix-blend-mode=\"overlay\"/>"
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
	static const char svg_url_reference_invalid[] =
		"<svg viewBox=\"0 0 24 18\">"
		"<defs>"
		"<clipPath id=\"leftClip\"><rect x=\"0\" y=\"0\" width=\"12\" height=\"18\"/></clipPath>"
		"<mask id=\"leftMask\" maskUnits=\"userSpaceOnUse\"><rect x=\"0\" y=\"0\" width=\"12\" height=\"18\" fill=\"white\"/></mask>"
		"<filter id=\"move\" x=\"0\" y=\"0\" width=\"24\" height=\"18\" filterUnits=\"userSpaceOnUse\"><feOffset dx=\"2\" dy=\"0\"/></filter>"
		"</defs>"
		"<g clip-path=\"url(#leftClip)\"><rect x=\"1\" y=\"1\" width=\"20\" height=\"4\" fill=\"#38bdf8\" clip-path=\"bad\"/><rect x=\"1\" y=\"6\" width=\"20\" height=\"4\" fill=\"#f97316\" clip-path=\"none\"/></g>"
		"<g mask=\"url(#leftMask)\"><rect x=\"1\" y=\"11\" width=\"20\" height=\"2\" fill=\"#22c55e\" mask=\"bad\"/><rect x=\"1\" y=\"14\" width=\"20\" height=\"2\" fill=\"#a855f7\" mask=\"none\"/></g>"
		"<g filter=\"url(#move)\"><rect x=\"14\" y=\"1\" width=\"4\" height=\"4\" fill=\"#facc15\" filter=\"bad\"/><rect x=\"14\" y=\"6\" width=\"4\" height=\"4\" fill=\"#ef4444\" filter=\"none\"/></g>"
		"</svg>";
	static const char svg_url_function_thorvg_strict[] =
		"<svg viewBox=\"0 0 180 90\">"
		"<defs>"
		"<linearGradient id=\"caseGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\"><stop offset=\"0%\" stop-color=\"#22c55e\"/><stop offset=\"100%\" stop-color=\"#38bdf8\"/></linearGradient>"
		"<clipPath id=\"caseClip\"><rect x=\"82\" y=\"18\" width=\"36\" height=\"28\"/></clipPath>"
		"<mask id=\"caseMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"126\" y=\"18\" width=\"36\" height=\"28\" mask-type=\"alpha\"><rect x=\"126\" y=\"18\" width=\"36\" height=\"28\" fill=\"#fff\"/></mask>"
		"<filter id=\"caseMove\" filterUnits=\"userSpaceOnUse\" x=\"12\" y=\"54\" width=\"60\" height=\"24\"><feOffset dx=\"8\" dy=\"0\"/></filter>"
		"<marker id=\"caseDot\" markerWidth=\"6\" markerHeight=\"6\" refX=\"3\" refY=\"3\" markerUnits=\"userSpaceOnUse\"><circle cx=\"3\" cy=\"3\" r=\"3\" fill=\"#facc15\"/></marker>"
		"</defs>"
		"<rect x=\"18\" y=\"18\" width=\"46\" height=\"28\" fill=\"URL(#caseGrad)\"/>"
		"<rect x=\"74\" y=\"18\" width=\"52\" height=\"28\" fill=\"#f97316\" clip-path=\"Url(#caseClip)\"/>"
		"<rect x=\"126\" y=\"18\" width=\"36\" height=\"28\" fill=\"#a855f7\" mask=\"uRl(#caseMask)\"/>"
		"<rect x=\"18\" y=\"58\" width=\"28\" height=\"14\" fill=\"#38bdf8\" filter=\"URL(#caseMove)\"/>"
		"<path d=\"M90 64 H140\" fill=\"none\" stroke=\"#e879f9\" stroke-width=\"3\" marker-end=\"Url(#caseDot)\"/>"
		"</svg>";
	static const char svg_transform_thorvg_strict[] =
		"<svg viewBox=\"0 0 180 90\">"
		"<rect x=\"30\" y=\"20\" width=\"30\" height=\"20\" fill=\"#38bdf8\" transform=\"Rotate(0.25turn 45 30)\"/>"
		"<rect x=\"85\" y=\"18\" width=\"24\" height=\"18\" fill=\"#22c55e\" transform=\"skewX(0.78539816339rad)\"/>"
		"<rect x=\"124\" y=\"14\" width=\"20\" height=\"14\" fill=\"#a855f7\" transform=\"translate(12px, 10px)\"/>"
		"</svg>";
	static const char svg_marker_bounds_hit[] =
		"<svg viewBox=\"0 0 32 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<marker id=\"box\" viewBox=\"0 0 6 6\" refX=\"0\" refY=\"3\" markerWidth=\"6\" markerHeight=\"6\" markerUnits=\"userSpaceOnUse\" orient=\"0\">"
		"<rect x=\"0\" y=\"0\" width=\"6\" height=\"6\" fill=\"#22c55e\" stroke=\"none\"/>"
		"</marker>"
		"</defs>"
		"<path d=\"M2 8 H10\" fill=\"none\" stroke=\"#334155\" stroke-width=\"0.5\" marker-end=\"url(#box)\"/>"
		"</svg>";
	static const char svg_marker_quadratic_path[] =
		"<svg viewBox=\"0 0 48 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<marker id=\"box\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\" orient=\"0\">"
		"<rect x=\"0\" y=\"0\" width=\"4\" height=\"4\" fill=\"#22c55e\" stroke=\"none\"/>"
		"</marker>"
		"</defs>"
		"<path d=\"M4 16 Q12 4 20 16 T36 16\" fill=\"none\" stroke=\"none\" marker-start=\"url(#box)\" marker-mid=\"url(#box)\" marker-end=\"url(#box)\"/>"
		"</svg>";
	static const char svg_marker_arc_segment_mid[] =
		"<svg viewBox=\"0 0 48 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<marker id=\"box\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\" orient=\"0\">"
		"<rect x=\"0\" y=\"0\" width=\"4\" height=\"4\" fill=\"#22c55e\" stroke=\"none\"/>"
		"</marker>"
		"</defs>"
		"<path d=\"M4 16 A16 12 0 1 1 36 16 L44 16\" fill=\"none\" stroke=\"none\" marker-mid=\"url(#box)\"/>"
		"</svg>";
	static const char svg_marker_closed_path[] =
		"<svg viewBox=\"0 0 28 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<marker id=\"box\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\" orient=\"0\">"
		"<rect x=\"0\" y=\"0\" width=\"4\" height=\"4\" fill=\"#22c55e\" stroke=\"none\"/>"
		"</marker>"
		"</defs>"
		"<path d=\"M4 4 L20 4 L20 20 Z\" fill=\"none\" stroke=\"none\" marker-mid=\"url(#box)\" marker-end=\"url(#box)\"/>"
		"</svg>";
	static const char svg_marker_invalid_reference[] =
		"<svg viewBox=\"0 0 24 18\">"
		"<defs><marker id=\"dot\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\"><circle cx=\"2\" cy=\"2\" r=\"2\" fill=\"#ff0000\"/></marker></defs>"
		"<g fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"1\" marker-start=\"url(#dot)\" marker-mid=\"url(#dot)\" marker-end=\"url(#dot)\">"
		"<path d=\"M3 4 H12 H21\" marker=\"bad\"/>"
		"<path d=\"M3 9 H12 H21\" marker-start=\"bad\" marker-mid=\"bad\" marker-end=\"bad\"/>"
		"<path d=\"M3 14 H12 H21\"/>"
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
	static const char svg_gradient_stop_style_order[] =
		"<svg viewBox=\"0 0 24 12\">"
		"<defs>"
		"<linearGradient id=\"styleFirst\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" color=\"#22c55e\">"
		"<stop offset=\"0%\" style=\"stop-color:currentColor\" stop-color=\"#ef4444\"/>"
		"<stop offset=\"100%\" stop-color=\"#3b82f6\" style=\"stop-color:#facc15\"/>"
		"</linearGradient>"
		"<linearGradient id=\"attrFirst\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" color=\"#22c55e\">"
		"<stop offset=\"0%\" stop-color=\"#ef4444\" style=\"stop-color:currentColor\"/>"
		"<stop offset=\"100%\" style=\"stop-color:#facc15\" stop-color=\"#3b82f6\"/>"
		"</linearGradient>"
		"<linearGradient id=\"selfColor\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" color=\"#22c55e\">"
		"<stop offset=\"0%\" color=\"#ef4444\" stop-color=\"currentColor\"/>"
		"<stop offset=\"100%\" stop-color=\"#3b82f6\"/>"
		"</linearGradient>"
		"</defs>"
		"<rect x=\"1\" y=\"1\" width=\"22\" height=\"2\" fill=\"url(#styleFirst)\"/>"
		"<rect x=\"1\" y=\"5\" width=\"22\" height=\"2\" fill=\"url(#attrFirst)\"/>"
		"<rect x=\"1\" y=\"9\" width=\"22\" height=\"2\" fill=\"url(#selfColor)\"/>"
		"</svg>";
	static const char svg_gradient_stop_case[] =
		"<svg viewBox=\"0 0 28 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<linearGradient id=\"upperStyle\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
		"<stop offset=\"0\" stop-color=\"#ef4444\" style=\"STOP-COLOR:#22c55e;STOP-OPACITY:1\"/>"
		"<stop offset=\"1\" stop-color=\"#ef4444\" style=\"Stop-Color:#22c55e;Stop-Opacity:1\"/>"
		"</linearGradient>"
		"<linearGradient id=\"currentHalf\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" color=\"#38bdf8\">"
		"<stop offset=\"0\" stop-color=\"#ef4444\" style=\"STOP-COLOR:CURRENTCOLOR;STOP-OPACITY:50%\"/>"
		"<stop offset=\"1\" stop-color=\"#ef4444\" style=\"STOP-COLOR:CURRENTCOLOR;STOP-OPACITY:50%\"/>"
		"</linearGradient>"
		"</defs>"
		"<rect width=\"28\" height=\"10\" fill=\"#0f172a\"/>"
		"<rect x=\"1\" y=\"2\" width=\"10\" height=\"6\" fill=\"url(#upperStyle)\"/>"
		"<rect x=\"15\" y=\"2\" width=\"10\" height=\"6\" fill=\"url(#currentHalf)\"/>"
		"</svg>";
	static const char svg_enum_inherit_invalid[] =
		"<svg viewBox=\"0 0 64 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<linearGradient id=\"baseLinear\" gradientUnits=\"userSpaceOnUse\" spreadMethod=\"repeat\" x1=\"0\" y1=\"0\" x2=\"8\" y2=\"0\">"
		"<stop offset=\"0\" stop-color=\"#ef4444\"/><stop offset=\"50%\" stop-color=\"#facc15\"/><stop offset=\"100%\" stop-color=\"#22c55e\"/>"
		"</linearGradient>"
		"<linearGradient id=\"badLinear\" href=\"#baseLinear\" gradientUnits=\"bad\" spreadMethod=\"bad\"/>"
		"<radialGradient id=\"baseRadial\" gradientUnits=\"userSpaceOnUse\" spreadMethod=\"reflect\" cx=\"48\" cy=\"10\" r=\"4\" fx=\"48\" fy=\"10\">"
		"<stop offset=\"0\" stop-color=\"#ffffff\"/><stop offset=\"100%\" stop-color=\"#3b82f6\"/>"
		"</radialGradient>"
		"<radialGradient id=\"badRadial\" href=\"#baseRadial\" gradientUnits=\"none\" spreadMethod=\"mirror\"/>"
		"<pattern id=\"basePattern\" patternUnits=\"userSpaceOnUse\" patternContentUnits=\"userSpaceOnUse\" width=\"6\" height=\"6\">"
		"<rect width=\"6\" height=\"6\" fill=\"#111827\"/><path d=\"M0 6 L6 0\" stroke=\"#38bdf8\" stroke-width=\"1\"/>"
		"</pattern>"
		"<pattern id=\"badPattern\" href=\"#basePattern\" patternUnits=\"bad\" patternContentUnits=\"bad\"/>"
		"</defs>"
		"<rect x=\"1\" y=\"1\" width=\"20\" height=\"8\" fill=\"url(#badLinear)\"/>"
		"<rect x=\"23\" y=\"1\" width=\"18\" height=\"8\" fill=\"url(#badPattern)\"/>"
		"<rect x=\"43\" y=\"1\" width=\"20\" height=\"8\" fill=\"url(#badRadial)\"/>"
		"<rect x=\"1\" y=\"12\" width=\"62\" height=\"6\" fill=\"#0f172a\"/>"
		"</svg>";
	static const char svg_style_invalid_keywords[] =
		"<svg viewBox=\"0 0 64 36\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".endAnchor{text-anchor:end}.badAnchor{text-anchor:bad}"
		".hidden{display:none}.badDisplay{display:bad}"
		".nonScaling{vector-effect:non-scaling-stroke}.badVector{vector-effect:bad}"
		"</style>"
		"<text class=\"endAnchor badAnchor\" x=\"32\" y=\"8\" font-size=\"6\">END</text>"
		"<rect class=\"hidden badDisplay\" x=\"4\" y=\"12\" width=\"10\" height=\"8\" fill=\"#ef4444\"/>"
		"<rect x=\"18\" y=\"12\" width=\"10\" height=\"8\" fill=\"#22c55e\"/>"
		"<path class=\"nonScaling badVector\" transform=\"translate(4 28) scale(1 3)\" d=\"M0 0 H52\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"3\"/>"
		"</svg>";
	static const char svg_style_duplicate_invalid[] =
		"<svg viewBox=\"0 0 64 36\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><marker id=\"dot\" viewBox=\"0 0 4 4\" refX=\"2\" refY=\"2\" markerWidth=\"4\" markerHeight=\"4\" markerUnits=\"userSpaceOnUse\"><circle cx=\"2\" cy=\"2\" r=\"2\" fill=\"#ff0000\"/></marker></defs>"
		"<style>"
		".paint{fill:#ef4444;fill:bad;stroke:#38bdf8;stroke:bad;stroke-width:2}"
		".hidden{display:none;display:bad}.anchor{text-anchor:end;text-anchor:bad}"
		".line{stroke-linecap:round;stroke-linecap:bad;stroke-linejoin:round;stroke-linejoin:bad}"
		".rule{fill-rule:evenodd;fill-rule:bad}"
		"</style>"
		"<rect class=\"paint\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"hidden\" x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/>"
		"<text class=\"anchor\" x=\"34\" y=\"7\" font-size=\"6\">END</text>"
		"<path class=\"line\" d=\"M2 18 L8 12 L14 18\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"2\"/>"
		"<path class=\"rule\" d=\"M22 12h10v8h-10z M25 15h4v2h-4z\" fill=\"#a78bfa\"/>"
		"<path d=\"M2 30 H18 H34\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"1\" style=\"marker-start:url(#dot);marker-start:bad;marker-mid:url(#dot);marker-mid:bad;marker-end:url(#dot);marker-end:bad\"/>"
		"</svg>";
	static const char svg_text_style_duplicate_invalid[] =
		"<svg viewBox=\"0 0 96 42\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><linearGradient id=\"g\"><stop offset=\"0\" style=\"stop-color:#ef4444;stop-color:bad\"/><stop offset=\"1\" style=\"stop-color:bad;stop-color:#38bdf8\"/></linearGradient></defs>"
		"<style>"
		".vis{visibility:hidden;visibility:bad}.blend{mix-blend-mode:multiply;mix-blend-mode:bad}.blendLate{mix-blend-mode:bad;mix-blend-mode:multiply}"
		".base{dominant-baseline:middle;dominant-baseline:bad}.baseLate{alignment-baseline:bad;alignment-baseline:middle}"
		".letter{letter-spacing:2px;letter-spacing:bad}.letterLate{letter-spacing:bad;letter-spacing:2px}"
		".word{word-spacing:4px;word-spacing:bad}.wordLate{word-spacing:bad;word-spacing:4px}.space{white-space:pre;white-space:bad}"
		"</style>"
		"<rect x=\"1\" y=\"1\" width=\"14\" height=\"6\" fill=\"url(#g)\"/>"
		"<rect class=\"vis\" x=\"18\" y=\"1\" width=\"8\" height=\"6\" fill=\"#ef4444\"/>"
		"<rect x=\"30\" y=\"1\" width=\"14\" height=\"6\" fill=\"#1d4ed8\"/><circle class=\"blend\" cx=\"37\" cy=\"4\" r=\"5\" fill=\"#f97316\"/>"
		"<rect x=\"48\" y=\"1\" width=\"14\" height=\"6\" fill=\"#1d4ed8\"/><circle class=\"blendLate\" cx=\"55\" cy=\"4\" r=\"5\" fill=\"#f97316\"/>"
		"<path d=\"M1 16 H94\" stroke=\"#475569\"/><text class=\"base\" x=\"2\" y=\"16\" font-size=\"6\">M</text><text class=\"baseLate\" x=\"10\" y=\"16\" font-size=\"6\">M</text>"
		"<text x=\"20\" y=\"16\" font-size=\"6\">A<tspan style=\"baseline-shift:super;baseline-shift:bad\">B</tspan><tspan style=\"baseline-shift:bad;baseline-shift:sub\">C</tspan></text>"
		"<text class=\"letter\" x=\"2\" y=\"28\" font-size=\"6\">ABC</text><text class=\"letterLate\" x=\"26\" y=\"28\" font-size=\"6\">ABC</text>"
		"<text class=\"word\" x=\"50\" y=\"28\" font-size=\"6\">A B</text><text class=\"wordLate\" x=\"72\" y=\"28\" font-size=\"6\">A B</text>"
		"<text class=\"space\" x=\"2\" y=\"38\" font-size=\"6\">A   B</text>"
		"</svg>";
	static const char svg_color_function_grammar[] =
		"<svg viewBox=\"0 0 96 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".modernRgb{fill:rgb(56 189 248)}.commaRgb{fill:rgba(34,197,94,1)}.modernHsl{fill:hsl(270 75% 60% / 100%)}"
		".badRgbCommaSpace{fill:#22c55e;fill:rgb(255, 0 0)}.badRgbSpaceComma{fill:#a855f7;fill:rgb(255 0, 0)}"
		".badRgbBareAlpha{fill:#f97316;fill:rgba(255 0 0 .5)}.badHslCommaSpace{fill:#38bdf8;fill:hsl(120, 100% 50%)}"
		"</style>"
		"<rect class=\"modernRgb\" x=\"2\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"commaRgb\" x=\"16\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"modernHsl\" x=\"30\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badRgbCommaSpace\" x=\"44\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badRgbSpaceComma\" x=\"58\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badRgbBareAlpha\" x=\"72\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badHslCommaSpace\" x=\"2\" y=\"17\" width=\"10\" height=\"8\"/>"
		"</svg>";
	static const char svg_css_color_source_replay[] =
		"<svg viewBox=\"0 0 56 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".fillReplay{fill:#22c55e;fill:rgb(300,0,0)}"
		".fillNone{fill:#38bdf8;fill:none}"
		".strokeReplay{fill:none;stroke:none;stroke:rgb(0,300,0);stroke-width:2}"
		".currentReplay{color:#eab308;color:rgb(0,0,300);fill:currentColor}"
		".importantReplay{fill:#84cc16!important;fill:rgb(300,0,0)!important}"
		"</style>"
		"<rect class=\"fillReplay\" x=\"1\" y=\"2\" width=\"8\" height=\"8\"/>"
		"<rect class=\"fillNone\" x=\"12\" y=\"2\" width=\"8\" height=\"8\"/>"
		"<rect class=\"strokeReplay\" x=\"24\" y=\"2\" width=\"8\" height=\"8\"/>"
		"<rect class=\"currentReplay\" x=\"36\" y=\"2\" width=\"8\" height=\"8\"/>"
		"<rect class=\"importantReplay\" x=\"47\" y=\"2\" width=\"8\" height=\"8\"/>"
		"</svg>";
	static const char svg_hsl_grammar[] =
		"<svg viewBox=\"0 0 96 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".degUpper{fill:hsl(120DEG 100% 50%)}.turnUpper{fill:hsl(.5TURN 100% 50%)}"
		".radUpper{fill:hsl(3.141592653589793RAD 100% 50%)}.gradUpper{fill:hsl(100GRAD 100% 50%)}"
		".commaUpper{fill:hsla(300DEG,100%,50%,1)}.badBareSat{fill:#22c55e;fill:hsl(120 100 50%)}"
		".badBareLight{fill:#f97316;fill:hsl(120 100% 50)}"
		"</style>"
		"<rect class=\"degUpper\" x=\"2\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"turnUpper\" x=\"16\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"radUpper\" x=\"30\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"gradUpper\" x=\"44\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"commaUpper\" x=\"58\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badBareSat\" x=\"72\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<rect class=\"badBareLight\" x=\"2\" y=\"17\" width=\"10\" height=\"8\"/>"
		"</svg>";
	static const char svg_css_property_case[] =
		"<svg viewBox=\"0 0 64 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.upper{FILL:#38bdf8;STROKE:#e2e8f0;STROKE-WIDTH:2px}.text{Font-Size:8px;Fill:#f97316;Text-Anchor:middle}.important{fill:#334155 !important;FILL:#22c55e !IMPORTANT}</style>"
		"<rect class=\"upper\" x=\"2\" y=\"2\" width=\"12\" height=\"8\"/>"
		"<rect class=\"important\" x=\"20\" y=\"2\" width=\"12\" height=\"8\"/>"
		"<circle cx=\"44\" cy=\"6\" r=\"5\" style=\"FILL:#a78bfa;Stroke:#f8fafc;Stroke-Width:2px\"/>"
		"<text class=\"text\" x=\"32\" y=\"24\">MID</text>"
		"</svg>";
	static const char svg_css_keyword_case[] =
		"<svg viewBox=\"0 0 96 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".hide{display:NONE}.vis{visibility:HIDDEN}.anchor{text-anchor:END;fill:#f8fafc;font-size:6px}"
		".rule{fill-rule:EVENODD;fill:#a78bfa}.line{fill:NONE;stroke:#38bdf8;stroke-width:5;stroke-linecap:ROUND;stroke-linejoin:BEVEL;vector-effect:NON-SCALING-STROKE}"
		".paint{fill:#22c55e;stroke:#f97316;stroke-width:4;paint-order:STROKE FILL}.clipnone{clip-path:NONE}.space{white-space:PRE}"
		"</style>"
		"<rect width=\"96\" height=\"32\" fill=\"#0f172a\"/>"
		"<rect class=\"hide\" x=\"2\" y=\"2\" width=\"10\" height=\"8\" fill=\"#ef4444\"/>"
		"<rect class=\"vis\" x=\"14\" y=\"2\" width=\"10\" height=\"8\" fill=\"#ef4444\"/>"
		"<rect class=\"clipnone\" x=\"2\" y=\"14\" width=\"10\" height=\"8\" fill=\"#22c55e\"/>"
		"<path class=\"rule\" d=\"M24 4 H44 V24 H24 Z M30 10 H38 V18 H30 Z\"/>"
		"<path class=\"line\" d=\"M52 20 L62 8 L72 20\"/>"
		"<text class=\"anchor space\" x=\"92\" y=\"12\">END</text>"
		"<rect class=\"paint\" x=\"76\" y=\"18\" width=\"12\" height=\"8\"/>"
		"</svg>";
	static const char svg_css_attribute_case_flag[] =
		"<svg viewBox=\"0 0 80 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		"[data-state=\"ready\" i]{fill:#22c55e}[data-state=\"ready\"]{stroke:#ef4444;stroke-width:2}"
		"[data-tags~=\"beta\" i]{fill:#38bdf8}[lang|=\"en\" i]{fill:#a78bfa}"
		"[data-code^=\"ab\" i]{stroke:#f97316;stroke-width:2}[data-code$=\"yz\" i]{fill:#facc15}[data-code*=\"34\" i]{opacity:.55}"
		"[data-state=\"READY\" s]{stroke:#f8fafc;stroke-width:2}"
		"</style>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" data-state=\"READY\"/>"
		"<rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" data-tags=\"Alpha BETA\"/>"
		"<rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" lang=\"EN-us\"/>"
		"<rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" data-code=\"AB1234YZ\"/>"
		"<rect x=\"2\" y=\"12\" width=\"8\" height=\"6\" data-state=\"Ready\"/>"
		"<rect x=\"14\" y=\"12\" width=\"8\" height=\"6\" data-state=\"ready\"/>"
		"<rect x=\"26\" y=\"12\" width=\"8\" height=\"6\" data-code=\"zz1234qq\"/>"
		"</svg>";
	static const char svg_css_identifier_escape[] =
		"<svg viewBox=\"0 0 80 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".foo\\:bar{fill:#38bdf8}#id\\31 {stroke:#f8fafc;stroke-width:2}"
		".hex\\32 0{fill:#f97316}[data\\-state=\"ready\"]{fill:#22c55e}"
		"[data-code=\"A\\2e B\"]{stroke:#a78bfa;stroke-width:2}"
		"</style>"
		"<rect class=\"foo:bar\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect id=\"id1\" x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#334155\"/>"
		"<rect class=\"hex20\" x=\"26\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect data-state=\"ready\" x=\"38\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect data-code=\"A.B\" x=\"50\" y=\"2\" width=\"8\" height=\"6\" fill=\"#111827\"/>"
		"</svg>";
	static const char svg_css_attribute_escape[] =
		"<svg viewBox=\"0 0 80 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		"rect{fill:#334155}"
		"[data-code=A\\20 B]{fill:#38bdf8}[data-quote=\"A\\\"B\"]{stroke:#f8fafc;stroke-width:2}"
		"[data-tags~=\\62 eta]{fill:#22c55e}[lang|=zh\\2d Hans]{fill:#a855f7}"
		"</style>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" data-code=\"A B\" data-quote='A\"B'/>"
		"<rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" data-tags=\"alpha beta\"/>"
		"<rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" lang=\"zh-Hans\"/>"
		"<rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" data-code=\"A20B\"/>"
		"</svg>";
	static const char svg_css_url_escape[] =
		"<svg viewBox=\"0 0 100 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<linearGradient id=\"grad1\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\"><stop offset=\"0\" stop-color=\"#22c55e\"/><stop offset=\"1\" stop-color=\"#38bdf8\"/></linearGradient>"
		"<clipPath id=\"clip-a\"><rect x=\"28\" y=\"3\" width=\"10\" height=\"8\"/></clipPath>"
		"<mask id=\"mask1\"><rect x=\"0\" y=\"0\" width=\"100\" height=\"28\" fill=\"white\"/></mask>"
		"<filter id=\"shadow1\"><feDropShadow dx=\"1\" dy=\"1\" stdDeviation=\"0\" flood-color=\"#a78bfa\" flood-opacity=\"1\"/></filter>"
		"<marker id=\"arrow1\" viewBox=\"0 0 6 6\" refX=\"6\" refY=\"3\" markerWidth=\"5\" markerHeight=\"5\" orient=\"auto\"><path d=\"M0 0 L6 3 L0 6 Z\" fill=\"#f97316\"/></marker>"
		"</defs>"
		"<style>"
		".grad{fill:url(#grad\\31)}.clip{clip-path:url(\"#clip\\2d a\")}.mask{mask:url('#mask\\31')}"
		".filter{filter:url(#shadow\\31)}.mark{marker-end:url(#arrow\\31)}"
		"</style>"
		"<rect class=\"grad\" x=\"2\" y=\"3\" width=\"16\" height=\"8\"/>"
		"<rect class=\"clip\" x=\"26\" y=\"1\" width=\"18\" height=\"12\" fill=\"#22c55e\"/>"
		"<rect class=\"mask\" x=\"50\" y=\"3\" width=\"10\" height=\"8\" fill=\"#38bdf8\"/>"
		"<rect class=\"filter\" x=\"66\" y=\"3\" width=\"8\" height=\"8\" fill=\"#facc15\"/>"
		"<path class=\"mark\" d=\"M82 7 H94\" fill=\"none\" stroke=\"#f8fafc\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_css_nth_spacing[] =
		"<svg viewBox=\"0 0 96 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		"rect{fill:#22c55e}rect:nth-child(2n + 1){fill:#38bdf8}rect:nth-child(2 n){fill:#ef4444}"
		"circle{fill:#facc15}circle:nth-of-type(n + 2){fill:#a855f7}circle:nth-of-type(2 n){fill:#ef4444}"
		"</style>"
		"<rect x=\"2\" y=\"3\" width=\"10\" height=\"8\"/><rect x=\"16\" y=\"3\" width=\"10\" height=\"8\"/><rect x=\"30\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<circle cx=\"8\" cy=\"20\" r=\"4\"/><circle cx=\"22\" cy=\"20\" r=\"4\"/><circle cx=\"36\" cy=\"20\" r=\"4\"/>"
		"</svg>";
	static const char svg_css_pseudo_case[] =
		"<svg viewBox=\"0 0 96 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		"rect{fill:#334155}rect:NTH-CHILD(ODD){fill:#38bdf8}rect:NOT(.skip){stroke:#f8fafc;stroke-width:1}"
		"g:HAS(.mark) rect.target{fill:#22c55e}circle:IS(.hot){fill:#f97316}path:WHERE(.line){stroke:#a855f7;stroke-width:2}"
		"</style>"
		"<rect x=\"2\" y=\"3\" width=\"10\" height=\"8\"/><rect class=\"skip\" x=\"16\" y=\"3\" width=\"10\" height=\"8\"/>"
		"<g><circle class=\"mark\" cx=\"36\" cy=\"7\" r=\"3\" fill=\"#facc15\"/><rect class=\"target\" x=\"44\" y=\"3\" width=\"10\" height=\"8\"/></g>"
		"<circle class=\"hot\" cx=\"10\" cy=\"20\" r=\"4\"/><path class=\"line\" d=\"M24 20 H52\" fill=\"none\"/>"
		"</svg>";
	static const char svg_css_media_rule[] =
		"<svg viewBox=\"0 0 80 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".base{fill:#334155}"
		"@media print{.printOnly{fill:#ef4444}}"
		"@media speech{.speechOnly{fill:#ef4444}}"
		"@media all{.allOnly{fill:#38bdf8}}"
		"@media screen{.screenOnly{fill:#22c55e}}"
		"@media not print{.notPrint{stroke:#f8fafc;stroke-width:2}}"
		"@media screen{@media all{.nestedMedia{fill:#f97316}}}"
		"</style>"
		"<rect class=\"base allOnly\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"base screenOnly\" x=\"14\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"base notPrint\" x=\"26\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"base nestedMedia\" x=\"38\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"base printOnly\" x=\"50\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"base speechOnly\" x=\"62\" y=\"2\" width=\"8\" height=\"6\"/>"
		"</svg>";
	static const char svg_css_thorvg_supported_selectors[] =
		"<svg viewBox=\"0 0 40 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>rect{fill:#22c55e;stroke:none}.hot{fill:#38bdf8;stroke:none}circle.hot{fill:#f97316;stroke:none}</style>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<rect class=\"hot\" x=\"14\" y=\"2\" width=\"8\" height=\"6\"/>"
		"<circle class=\"hot\" cx=\"30\" cy=\"5\" r=\"3\"/>"
		"</svg>";
	static const char svg_css_complex_selectors_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 40 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.target{fill:none;stroke:none}rect[probe]{fill:#22c55e}g .target{fill:#38bdf8}rect.target:hover{fill:#f97316}@media all{.target{fill:#a78bfa}}</style>"
		"<g><rect class=\"target\" probe=\"1\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/></g>"
		"</svg>";
	static const char svg_style_first_only_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.target{fill:#22c55e;stroke:none}</style>"
		"<style>.target{fill:none;stroke:none}</style>"
		"<rect class=\"target\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"</svg>";
	static const char svg_style_first_in_unsupported_container_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<metadata><style>.target{fill:#22c55e;stroke:none}</style></metadata>"
		"<style>.target{fill:none;stroke:none}</style>"
		"<rect class=\"target\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/>"
		"</svg>";
	static const char svg_switch_conditional[] =
		"<svg viewBox=\"0 0 80 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<switch>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\" requiredExtensions=\"xge.unsupported\"/>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#22c55e\"/>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/>"
		"</switch>"
		"<switch>"
		"<g requiredFeatures=\"http://www.w3.org/TR/SVG11/feature#Shape\"><rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\"/></g>"
		"<rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/>"
		"</switch>"
		"<switch>"
		"<rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\" systemLanguage=\"zz\"/>"
		"<rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#f97316\"/>"
		"</switch>"
		"<switch>"
		"<g requiredFeatures=\"http://www.w3.org/TR/SVG11/feature#Animation\"><rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></g>"
		"<g><rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" fill=\"#a78bfa\"/></g>"
		"</switch>"
		"</svg>";
	static const char svg_switch_style_scope[] =
		"<svg viewBox=\"0 0 40 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<switch>"
		"<g requiredExtensions=\"xge.unsupported\"><style>.target{fill:#ef4444}</style></g>"
		"<g><style>.target{fill:#22c55e;stroke:#f8fafc;stroke-width:1}</style></g>"
		"</switch>"
		"<rect class=\"target\" x=\"4\" y=\"4\" width=\"16\" height=\"8\" fill=\"#334155\"/>"
		"</svg>";
	static const char svg_switch_selector_scope[] =
		"<svg viewBox=\"0 0 40 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.subject{fill:#22c55e}.subject:has(.leaked){fill:#ef4444}</style>"
		"<g class=\"subject\">"
		"<switch>"
		"<g requiredExtensions=\"xge.unsupported\"><rect class=\"leaked\" x=\"4\" y=\"4\" width=\"16\" height=\"8\"/></g>"
		"<g><rect x=\"4\" y=\"4\" width=\"16\" height=\"8\"/></g>"
		"</switch>"
		"</g>"
		"</svg>";
	static const char svg_switch_container_inherit[] =
		"<svg viewBox=\"0 0 60 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"0\" y=\"0\" width=\"60\" height=\"24\" fill=\"#0f172a\"/>"
		"<switch fill=\"#22c55e\" stroke=\"#f8fafc\" stroke-width=\"1\" transform=\"translate(10 4)\">"
		"<g requiredExtensions=\"xge.unsupported\"><rect x=\"4\" y=\"4\" width=\"16\" height=\"8\" fill=\"#ef4444\"/></g>"
		"<g><rect x=\"4\" y=\"4\" width=\"16\" height=\"8\"/></g>"
		"</switch>"
		"</svg>";
	static const char svg_conditional_processing_attrs[] =
		"<svg viewBox=\"0 0 80 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<g requiredExtensions=\"xge.unsupported\"><style>.target{fill:#ef4444}</style><rect class=\"leaked\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/></g>"
		"<style>.target{fill:#22c55e}.subject:has(.leaked){fill:#ef4444}</style>"
		"<g class=\"subject\"><rect class=\"target\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/></g>"
		"<g requiredFeatures=\"http://www.w3.org/TR/SVG11/feature#Animation\"><rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></g>"
		"<g systemLanguage=\"zz\"><rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></g>"
		"<g systemLanguage=\"zh\"><rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\"/></g>"
		"</svg>";
	static const char svg_switch_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 48 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<switch fill=\"#22c55e\" transform=\"translate(20 0)\">"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\" requiredExtensions=\"xge.unsupported\"/>"
		"<rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\" systemLanguage=\"zz\"/>"
		"</switch>"
		"</svg>";
	static const char svg_conditional_attrs_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 48 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<g requiredExtensions=\"xge.unsupported\"><rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></g>"
		"<g requiredFeatures=\"http://www.w3.org/TR/SVG11/feature#Animation\"><rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#22c55e\"/></g>"
		"<g systemLanguage=\"zz\"><rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\"/></g>"
		"</svg>";
	static const char svg_anchor_container[] =
		"<svg viewBox=\"0 0 100 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><clipPath id=\"anchorClip\"><a transform=\"translate(60 6)\"><rect x=\"0\" y=\"0\" width=\"14\" height=\"10\"/></a></clipPath></defs>"
		"<a id=\"anchorGroup\" transform=\"translate(10 6)\" fill=\"#22c55e\" stroke=\"#f8fafc\" stroke-width=\"1\"><rect x=\"0\" y=\"0\" width=\"18\" height=\"10\"/></a>"
		"<use href=\"#anchorGroup\" transform=\"translate(30 0)\"/>"
		"<rect x=\"56\" y=\"2\" width=\"24\" height=\"18\" fill=\"#ef4444\" clip-path=\"url(#anchorClip)\"/>"
		"</svg>";
	static const char svg_metadata_skip[] =
		"<svg viewBox=\"0 0 80 28\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<metadata><style>.target{fill:#ef4444}</style><rect class=\"leaked\" x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></metadata>"
		"<title><rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></title>"
		"<desc><style>.target{stroke:#ef4444;stroke-width:4}</style></desc>"
		"<script><rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></script>"
		"<style>.target{fill:#22c55e}.subject:has(.leaked){fill:#ef4444}</style>"
		"<g class=\"subject\"><rect class=\"target\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/></g>"
		"<rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\"/>"
		"</svg>";
	static const char svg_unsupported_container_children_thorvg_compat[] =
		"<svg viewBox=\"0 0 72 16\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<metadata><rect x=\"2\" y=\"2\" width=\"8\" height=\"6\" fill=\"#ef4444\"/></metadata>"
		"<title><rect x=\"14\" y=\"2\" width=\"8\" height=\"6\" fill=\"#22c55e\"/></title>"
		"<desc><rect x=\"26\" y=\"2\" width=\"8\" height=\"6\" fill=\"#38bdf8\"/></desc>"
		"<script><rect x=\"38\" y=\"2\" width=\"8\" height=\"6\" fill=\"#f97316\"/></script>"
		"<foreignObject><rect x=\"50\" y=\"2\" width=\"8\" height=\"6\" fill=\"#a78bfa\"/></foreignObject>"
		"</svg>";
	static const char svg_css_complex_selector_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 12\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.target{fill:none;stroke:none}svg>.target{fill:#22c55e}</style>"
		"<metadata><rect class=\"target\" x=\"2\" y=\"2\" width=\"8\" height=\"6\"/></metadata>"
		"</svg>";
	static const char svg_xlink_href[] =
		"<svg viewBox=\"0 0 90 28\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
		"<defs>"
		"<linearGradient id=\"baseGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\"><stop offset=\"0\" stop-color=\"#22c55e\"/><stop offset=\"1\" stop-color=\"#38bdf8\"/></linearGradient>"
		"<linearGradient id=\"aliasGrad\" xlink:href=\"#baseGrad\"/>"
		"<pattern id=\"basePattern\" patternUnits=\"userSpaceOnUse\" width=\"4\" height=\"4\"><rect width=\"4\" height=\"4\" fill=\"#111827\"/><rect width=\"2\" height=\"2\" fill=\"#facc15\"/></pattern>"
		"<pattern id=\"aliasPattern\" xlink:href=\"#basePattern\"/>"
		"<g id=\"badge\" fill=\"#22c55e\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"6\"/></g>"
		"<path id=\"labelPath\" d=\"M48 18 H76\"/>"
		"</defs>"
		"<rect x=\"2\" y=\"2\" width=\"12\" height=\"8\" fill=\"url(#aliasGrad)\"/>"
		"<use xlink:href=\"#badge\" x=\"18\" y=\"2\"/>"
		"<rect x=\"30\" y=\"2\" width=\"12\" height=\"8\" fill=\"url(#aliasPattern)\"/>"
		"<text font-size=\"5\" fill=\"#cbd5e1\"><textPath xlink:href=\"#labelPath\">xlink</textPath></text>"
		"</svg>";
	static const char svg_radial_gradient_fr[] =
		"<svg viewBox=\"0 0 12 8\">"
		"<defs>"
		"<radialGradient id=\"frFill\" cx=\"50%\" cy=\"50%\" r=\"55%\" fx=\"32%\" fy=\"44%\" fr=\"14%\">"
		"<stop offset=\"0%\" stop-color=\"#f8fafc\"/>"
		"<stop offset=\"55%\" stop-color=\"#38bdf8\"/>"
		"<stop offset=\"100%\" stop-color=\"#1e3a8a\"/>"
		"</radialGradient>"
		"<radialGradient id=\"frStroke\" cx=\"50%\" cy=\"50%\" r=\"55%\" fx=\"68%\" fy=\"42%\" fr=\"10%\">"
		"<stop offset=\"0%\" stop-color=\"#fff7ed\"/>"
		"<stop offset=\"100%\" stop-color=\"#c2410c\"/>"
		"</radialGradient>"
		"</defs>"
		"<rect x=\"1\" y=\"1\" width=\"4\" height=\"4\" fill=\"url(#frFill)\"/>"
		"<circle cx=\"8.5\" cy=\"3.3\" r=\"2\" fill=\"none\" stroke=\"url(#frStroke)\" stroke-width=\"0.7\"/>"
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
	static const char svg_opacity_duplicate_invalid[] =
		"<svg viewBox=\"0 0 48 20\">"
		"<defs><linearGradient id=\"g\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\"><stop offset=\"0\" stop-color=\"#ef4444\" style=\"stop-opacity:.25;stop-opacity:bad\"/><stop offset=\"1\" stop-color=\"#3b82f6\" style=\"stop-opacity:50%;stop-opacity:nan\"/></linearGradient></defs>"
		"<rect x=\"1\" y=\"1\" width=\"8\" height=\"6\" fill=\"#22c55e\" style=\"opacity:.35;opacity:bad\"/>"
		"<rect x=\"12\" y=\"1\" width=\"8\" height=\"6\" fill=\"#f97316\" style=\"fill-opacity:.35;fill-opacity:bad\"/>"
		"<rect x=\"24\" y=\"2\" width=\"8\" height=\"4\" fill=\"none\" stroke=\"#a855f7\" stroke-width=\"2\" style=\"stroke-opacity:.35;stroke-opacity:bad\"/>"
		"<rect x=\"1\" y=\"12\" width=\"42\" height=\"5\" fill=\"url(#g)\"/>"
		"</svg>";
	static const char svg_font_size_invalid[] =
		"<svg viewBox=\"0 0 48 18\">"
		"<g font-size=\"7\" fill=\"#38bdf8\">"
		"<text x=\"1\" y=\"7\">OK</text>"
		"<text x=\"14\" y=\"7\" font-size=\"bad\">BAD</text>"
		"<text x=\"28\" y=\"7\" font-size=\"nan\">NAN</text>"
		"<text x=\"1\" y=\"16\" font-size=\"-4\">NEG</text>"
		"<text x=\"18\" y=\"16\" font-size=\"\">EMPTY</text>"
		"<text x=\"36\" y=\"16\" font-size=\"0\">ZERO</text>"
		"</g>"
		"</svg>";
	static const char svg_font_size_style_ignored[] =
		"<svg viewBox=\"0 0 70 24\">"
		"<g font-size=\"6\" fill=\"#38bdf8\">"
		"<text x=\"1\" y=\"9\" style=\"font-size:9px;font-size:bad\">KEEP9</text>"
		"<text x=\"1\" y=\"20\" style=\"font-size:bad;font-size:9px\">LATE9</text>"
		"<text x=\"38\" y=\"9\" style=\"font-size:7px;font-size:-4px\">KEEP7</text>"
		"<text x=\"38\" y=\"20\" style=\"font-size:6px;font-size:nan\">KEEP6</text>"
		"</g>"
		"</svg>";
	static const char svg_font_size_invalid_bounds[] =
		"<svg viewBox=\"0 0 48 18\">"
		"<text x=\"1\" y=\"12\" font-size=\"bad\">BAD</text>"
		"</svg>";
	static const char svg_font_size_style_ignored_bounds[] =
		"<svg viewBox=\"0 0 48 18\">"
		"<g font-size=\"4\"><text x=\"1\" y=\"14\" style=\"font-size:4px;font-size:bad\">MMMM</text></g>"
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
	static const char svg_stroke_miter_turn_limit_2[] =
		"<svg viewBox=\"-8 -8 36 26\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<polygon points=\"0,10 10,0 20,10\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"4\" stroke-linejoin=\"miter\" stroke-miterlimit=\"2\"/>"
		"</svg>";
	static const char svg_stroke_miter_turn_limit_3[] =
		"<svg viewBox=\"-8 -8 36 26\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<polygon points=\"0,10 10,0 20,10\" fill=\"none\" stroke=\"#38bdf8\" stroke-width=\"4\" stroke-linejoin=\"miter\" stroke-miterlimit=\"3\"/>"
		"</svg>";
	static const char svg_external_parent_literal[] =
		"<svg viewBox=\"0 0 8 4\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"0\" y=\"0\" width=\"4\" height=\"4\" href=\"xge_shapeex_svg_external child.png\"/>"
		"<image x=\"4\" y=\"0\" width=\"4\" height=\"4\" href=\"xge_shapeex_svg_external child.svg\" preserveAspectRatio=\"none\"/>"
		"</svg>";
	static const char svg_image_data_uri_policy[] =
		"<svg viewBox=\"0 0 20 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"0\" y=\"0\" width=\"20\" height=\"6\" fill=\"#111827\"/>"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%204%204%22%3E%3Crect%20width%3D%224%22%20height%3D%224%22%20fill%3D%22%2322c55e%22%2F%3E%3C%2Fsvg%3E\"/>"
		"<image x=\"6\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/gif;base64,AAAA\"/>"
		"<image x=\"11\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/png;utf8,AAAA\"/>"
		"<image x=\"16\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;charset=utf-8,%3Csvg%2F%3E\"/>"
		"</svg>";
	static const char svg_image_svg_data_charset[] =
		"<svg viewBox=\"0 0 20 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"0\" y=\"0\" width=\"20\" height=\"6\" fill=\"#111827\"/>"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;charset=utf-8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%204%204%22%3E%3Crect%20width%3D%224%22%20height%3D%224%22%20fill%3D%22%2322c55e%22%2F%3E%3C%2Fsvg%3E\"/>"
		"<image x=\"6\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;CHARSET=UTF-8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%204%204%22%3E%3Crect%20width%3D%224%22%20height%3D%224%22%20fill%3D%22%2338bdf8%22%2F%3E%3C%2Fsvg%3E\"/>"
		"<image x=\"11\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;charset=utf-8;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA0IDQiPjxyZWN0IHdpZHRoPSI0IiBoZWlnaHQ9IjQiIGZpbGw9IiNmOTczMTYiLz48L3N2Zz4=\"/>"
		"</svg>";
	static const char svg_image_svg_utf8_bounds[] =
		"<svg viewBox=\"0 0 20 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%204%204%22%3E%3Crect%20width%3D%224%22%20height%3D%224%22%2F%3E%3C%2Fsvg%3E\"/>"
		"</svg>";
	static const char svg_image_svg_charset_bounds[] =
		"<svg viewBox=\"0 0 20 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;charset=utf-8,%3Csvg%2F%3E\"/>"
		"<image x=\"11\" y=\"1\" width=\"4\" height=\"4\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;charset=utf-8;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA0IDQiPjxyZWN0IHdpZHRoPSI0IiBoZWlnaHQ9IjQiLz48L3N2Zz4=\"/>"
		"</svg>";
	static const char svg_image_svg_fragment_ignored_bounds[] =
		"<svg viewBox=\"0 0 20 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%3Cdefs%3E%3Crect%20id%3D%22frag%22%20width%3D%224%22%20height%3D%224%22%2F%3E%3C%2Fdefs%3E%3C%2Fsvg%3E#frag\"/>"
		"</svg>";
	static const char svg_pattern_overflow_hidden_bounds[] =
		"<svg viewBox=\"0 0 20 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><pattern id=\"p\" patternUnits=\"userSpaceOnUse\" width=\"10\" height=\"8\"><rect x=\"8\" y=\"1\" width=\"8\" height=\"4\"/></pattern></defs>"
		"<rect x=\"0\" y=\"0\" width=\"16\" height=\"8\" fill=\"url(#p)\"/>"
		"</svg>";
	static const char svg_pattern_overflow_visible_bounds[] =
		"<svg viewBox=\"0 0 20 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><pattern id=\"p\" patternUnits=\"userSpaceOnUse\" width=\"10\" height=\"8\" overflow=\"visible\"><rect x=\"8\" y=\"1\" width=\"8\" height=\"4\"/></pattern></defs>"
		"<rect x=\"0\" y=\"0\" width=\"16\" height=\"8\" fill=\"url(#p)\"/>"
		"</svg>";
	static const char svg_pattern_overflow_visible_far_bounds[] =
		"<svg viewBox=\"0 0 20 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><pattern id=\"p\" patternUnits=\"userSpaceOnUse\" width=\"10\" height=\"8\" overflow=\"visible\"><rect x=\"-15\" y=\"1\" width=\"4\" height=\"4\"/></pattern></defs>"
		"<rect x=\"0\" y=\"0\" width=\"10\" height=\"8\" fill=\"url(#p)\"/>"
		"</svg>";
	static const char svg_pattern_stroke_ignored_bounds[] =
		"<svg viewBox=\"0 0 20 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><pattern id=\"p\" patternUnits=\"userSpaceOnUse\" width=\"4\" height=\"4\"><rect width=\"4\" height=\"4\" fill=\"white\"/></pattern></defs>"
		"<rect x=\"2\" y=\"2\" width=\"8\" height=\"4\" fill=\"none\" stroke=\"url(#p)\" stroke-width=\"2\"/>"
		"</svg>";
	static const char svg_clip_paint_ignored_bounds[] =
		"<svg viewBox=\"0 0 20 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><clipPath id=\"c\"><rect x=\"2\" y=\"1\" width=\"5\" height=\"4\" fill=\"none\" stroke=\"none\"/></clipPath></defs>"
		"<rect x=\"0\" y=\"0\" width=\"12\" height=\"8\" fill=\"#38bdf8\" clip-path=\"url(#c)\"/>"
		"</svg>";
	static const char svg_clip_use_rule_ignored[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<path id=\"evenoddClip\" clip-rule=\"evenodd\" d=\"M1 1H9V5H1Z M3 2H7V4H3Z\"/>"
		"<clipPath id=\"c\"><use href=\"#evenoddClip\"/></clipPath>"
		"</defs>"
		"<rect x=\"3\" y=\"2\" width=\"4\" height=\"2\" fill=\"#ef4444\" clip-path=\"url(#c)\"/>"
		"<rect x=\"10\" y=\"1\" width=\"1\" height=\"1\" fill=\"#22c55e\"/>"
		"</svg>";
	static const char svg_use_ancestor_ignored[] =
		"<svg viewBox=\"0 0 16 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<g id=\"ancestor\"><rect x=\"1\" y=\"2\" width=\"4\" height=\"3\"/><use href=\"#ancestor\" x=\"8\"/></g>"
		"</svg>";
	static const char svg_use_cycle_ignored[] =
		"<svg viewBox=\"0 0 24 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<g id=\"cycleA\"><rect x=\"1\" y=\"1\" width=\"3\" height=\"2\"/><use href=\"#cycleB\"/></g>"
		"<g id=\"cycleB\"><circle cx=\"3\" cy=\"3\" r=\"2\"/><use href=\"#cycleA\"/></g>"
		"</defs>"
		"<use href=\"#cycleA\"/><rect x=\"20\" y=\"2\" width=\"2\" height=\"2\"/>"
		"</svg>";
	static const char svg_nested_gradient_ignored[] =
		"<svg viewBox=\"0 0 16 7\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><linearGradient id=\"outer\"><stop offset=\"0\" stop-color=\"#38bdf8\"/>"
		"<radialGradient id=\"nested\"><stop offset=\"0\" stop-color=\"white\"/><stop offset=\"1\" stop-color=\"orange\"/></radialGradient>"
		"<stop offset=\"1\" stop-color=\"#a78bfa\"/></linearGradient></defs>"
		"<rect x=\"1\" y=\"1\" width=\"4\" height=\"4\" fill=\"url(#outer)\"/>"
		"<rect x=\"10\" y=\"1\" width=\"4\" height=\"4\" fill=\"url(#nested)\"/>"
		"</svg>";
	static const char svg_font_face_test_data[] =
		"AAEAAAAOAIAAAwBgR0RFRgAfABEAAAVYAAAAIkdQT1NEdkx1AAAFfAAAACBHU1VCuPq49AAABZwAAAAqT1MvMm47aeoAAAFoAAAAYGNtYXAAZwDjAAAB9AAAAERnYXNwAAAAEAAABVAAAAAIZ2x5Zg0hKAYAAAJQAAAA7GhlYWQlgaIRAAAA7AAAADZoaGVhC1YE0gAAASQAAAAkaG10eDT2AL8AAAHIAAAALGxvY2EBRwF7AAACOAAAABhtYXhwAA0ABQAAAUgAAAAgbmFtZSqeQTUAAAM8AAAB8nBvc3T+kQCMAAAFMAAAACAAAQAAAAIAQlUCGRxfDzz1AA0H0AAAAADbNNVSAAAAAOZ7gjoAZAAABEwFeAAAAAYAAgAAAAAAAAABAAAHbP4+AAAFXAAAASwD6AABAAAAAAAAAAAAAAAAAAAACwABAAAACwAEAAEAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAQEcgGQAAUAAAUUBLAAAACWBRQEsAAAArwAjAJsAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAE5PTkUAwAAgAFoHbP4+AAAJCwHvAAAAAQAAAAAECgWmAAAAIAADBVwAvwUUAAAFFAAABRQAAAUUAAAFFAAABRQAAAUUAAAFFAAABRQAAAHmAAAAAAACAAAAAwAAABQAAwABAAAAFAAEADAAAAAIAAgAAgAAACAARgBa//8AAAAgAEEAWP///+r/wP+vAAEAAAAAAAAAAAAAAAAADQAaACcANABBAE4AXABpAHYAdgABAGQAAARMBXgAAwAAMyERIWQD6PwYBXgAAAEAZAAAA4QEsAADAAAzIREhZAMg/OAEsAAAAQBkAMgETAV4AAMAADchESFkA+j8GMgEsAABAMgAAARMBRQAAwAAMyERIcgDhPx8BRQAAAEAZABkA+gFeAADAAA3IREhZAOE/HxkBRQAAQCgAAAEEAV4AAMAADMhESGgA3D8kAV4AAABAGQBLARMBXgAAwAAEyERIWQD6PwYASwETAAAAQDIAAAD6AV4AAMAADMhESHIAyD84AV4AAABAGQAAARMBEwAAwAAMyERIWQD6PwYBEwAAAAABwBaAAMAAQQJAAAAqgAAAAMAAQQJAAEAFgCqAAMAAQQJAAIADgDAAAMAAQQJAAMAOgDOAAMAAQQJAAQAJgEIAAMAAQQJAAUARgEuAAMAAQQJAAYAJAF0AEMAbwBwAHkAcgBpAGcAaAB0ACAAMgAwADEANQAgAFQAaABlACAAUAB1AGIAbABpAGMAIABTAGEAbgBzACAAUAByAG8AagBlAGMAdAAgAEEAdQB0AGgAbwByAHMAIAAoAGgAdAB0AHAAcwA6AC8ALwBnAGkAdABoAHUAYgAuAGMAbwBtAC8AdQBzAHcAZABzAC8AcAB1AGIAbABpAGMALQBzAGEAbgBzACkAUAB1AGIAbABpAGMAIABTAGEAbgBzAFIAZQBnAHUAbABhAHIAMgAuADAAMAAxADsATgBPAE4ARQA7AFAAdQBiAGwAaQBjAFMAYQBuAHMALQBSAGUAZwB1AGwAYQByAFAAdQBiAGwAaQBjACAAUwBhAG4AcwAgAFIAZQBnAHUAbABhAHIAVgBlAHIAcwBpAG8AbgAgADIALgAwADAAMQA7ACAAdAB0AGYAYQB1AHQAbwBoAGkAbgB0ACAAKAB2ADEALgA4AC4AMwApAFAAdQBiAGwAaQBjAFMAYQBuAHMALQBSAGUAZwB1AGwAYQByAAAAAwAAAAAAAP6OAIwAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAH//wAPAAEAAAAMAAAAAAAAAAIAAwABAAEAAQADAAUAAQAIAAkAAQAAAAEAAAAKABwAHgABREZMVAAIAAQAAAAA//8AAAAAAAAAAQAAAAoAJgAoAAJERkxUAA5sYXRuABgABAAAAAD//wAAAAAAAAAAAAAAAA==";
	static const char svg_font_face_fallback[] =
		"<svg viewBox=\"0 0 180 64\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<text x=\"10\" y=\"50\" font-family=\"UnknownFace\" font-size=\"44\">ABCDE</text>"
		"</svg>";
	static const char svg_font_face_malformed[] =
		"<svg viewBox=\"0 0 180 64\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>@font-face{font-family:'BrokenFace';src:url('data:font/ttf;base64,not-valid-@@');}</style>"
		"<text x=\"10\" y=\"50\" font-family=\"BrokenFace\" font-size=\"44\">ABCDE</text>"
		"</svg>";
	static const char svg_use_data_uri_case[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<use x=\"1\" y=\"1\" width=\"4\" height=\"4\" href=\"DATA:IMAGE/SVG+XML;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%204%204%22%3E%3Crect%20width%3D%224%22%20height%3D%224%22%20fill%3D%22%2322c55e%22%2F%3E%3C%2Fsvg%3E\"/>"
		"</svg>";
	static const char svg_use_data_uri_fragment[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<use x=\"2\" y=\"1\" width=\"6\" height=\"3\" href=\"data:image/svg+xml;utf8,%3Csvg%20viewBox%3D%220%200%208%204%22%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%3Cdefs%3E%3Csymbol%20id%3D%22frag%22%20viewBox%3D%220%200%204%202%22%3E%3Crect%20width%3D%224%22%20height%3D%222%22%2F%3E%3C%2Fsymbol%3E%3C%2Fdefs%3E%3C%2Fsvg%3E#frag\"/>"
		"</svg>";
	static const char svg_image_data_uri_fragment[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"2\" y=\"1\" width=\"6\" height=\"4\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;utf8,%3Csvg%20viewBox%3D%220%200%208%204%22%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%3E%3Cdefs%3E%3Csymbol%20id%3D%22frag%22%20viewBox%3D%220%200%204%202%22%3E%3Crect%20width%3D%224%22%20height%3D%222%22%20fill%3D%22%2322c55e%22%2F%3E%3C%2Fsymbol%3E%3C%2Fdefs%3E%3C%2Fsvg%3E#frag\"/>"
		"</svg>";
	static const char svg_use_bare_id[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><rect id=\"bareTarget\" x=\"1\" y=\"1\" width=\"4\" height=\"4\"/></defs>"
		"<use href=\"bareTarget\" x=\"1\" y=\"1\"/>"
		"<rect x=\"9\" y=\"1\" width=\"1\" height=\"1\"/>"
		"</svg>";
	static const char svg_use_url_local_ignored[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs><rect id=\"target\" x=\"1\" y=\"1\" width=\"4\" height=\"4\"/></defs>"
		"<use href=\"url(#target)\"/>"
		"<rect x=\"9\" y=\"1\" width=\"1\" height=\"1\"/>"
		"</svg>";
	static const char svg_use_descendant_element_transform[] =
		"<svg viewBox=\"0 0 20 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<g transform=\"translate(5 0)\"><rect id=\"child\" x=\"1\" y=\"1\" width=\"2\" height=\"2\"/></g>"
		"<use href=\"#child\" x=\"10\" y=\"4\"/>"
		"</svg>";
	static const char svg_use_descendant_group_transform[] =
		"<svg viewBox=\"0 0 20 10\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<g transform=\"translate(5 0)\"><g id=\"childGroup\" transform=\"translate(2 0)\"><rect width=\"2\" height=\"2\"/></g></g>"
		"<use href=\"#childGroup\" x=\"10\" y=\"4\"/>"
		"</svg>";
	static const char svg_image_jpg_data_uri[] =
		"<svg viewBox=\"0 0 12 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"1\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/jpg;base64," XGE_TEST_RASTER_JPEG "\"/>"
		"<image x=\"7\" y=\"1\" width=\"4\" height=\"4\" href=\"data:image/jpeg;base64," XGE_TEST_RASTER_JPEG "\"/>"
		"</svg>";
	static const char svg_bounds_smoke[] =
		"<svg viewBox=\"0 0 100 50\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"0\" y=\"0\" width=\"99\" height=\"50\" display=\"none\"/>"
		"<rect x=\"10\" y=\"11\" width=\"20\" height=\"7\"/>"
		"<g transform=\"translate(30 5)\"><rect x=\"4\" y=\"6\" width=\"8\" height=\"3\"/></g>"
		"<text x=\"45\" y=\"20\" font-size=\"8\">Hi</text>"
		"</svg>";
	static const char svg_contains_smoke[] =
		"<svg viewBox=\"0 0 20 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"1\" y=\"1\" width=\"6\" height=\"6\" fill=\"#f00\"/>"
		"<rect x=\"4\" y=\"4\" width=\"8\" height=\"8\" fill=\"none\" stroke=\"#00f\" stroke-width=\"2\"/>"
		"<rect x=\"14\" y=\"1\" width=\"4\" height=\"4\" fill=\"#0f0\" display=\"none\"/>"
		"</svg>";
	static const char svg_visibility_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 8\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<rect x=\"1\" y=\"1\" width=\"4\" height=\"4\" fill=\"#ef4444\" visibility=\"hidden\"/>"
		"<g visibility=\"hidden\"><rect x=\"8\" y=\"1\" width=\"4\" height=\"4\" fill=\"#22c55e\" visibility=\"visible\"/></g>"
		"<rect x=\"15\" y=\"1\" width=\"4\" height=\"4\" fill=\"#38bdf8\" style=\"visibility:hidden;visibility:bad\"/>"
		"</svg>";
	static const char svg_vector_effect_ignored_thorvg_compat[] =
		"<svg viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<path d=\"M2 4 H18\" transform=\"scale(1 3)\" fill=\"none\" stroke=\"#ef4444\" stroke-width=\"2\" vector-effect=\"non-scaling-stroke\"/>"
		"</svg>";
	static const char svg_nested_bounds_smoke[] =
		"<svg viewBox=\"0 0 100 50\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"10\" y=\"10\" width=\"40\" height=\"20\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%2010%2010%22%3E%3Crect%20x%3D%222%22%20y%3D%223%22%20width%3D%224%22%20height%3D%225%22%2F%3E%3C%2Fsvg%3E\"/>"
		"</svg>";
	static const char svg_nested_contains_smoke[] =
		"<svg viewBox=\"0 0 100 50\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"10\" y=\"10\" width=\"40\" height=\"20\" preserveAspectRatio=\"none\" href=\"data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20viewBox%3D%220%200%2010%2010%22%3E%3Ccircle%20cx%3D%225%22%20cy%3D%225%22%20r%3D%222%22%20fill%3D%22%2300f%22%2F%3E%3C%2Fsvg%3E\"/>"
		"</svg>";
	static const char svg_raster_contains_smoke[] =
		"<svg viewBox=\"0 0 20 20\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image x=\"2\" y=\"2\" width=\"8\" height=\"8\" preserveAspectRatio=\"none\" href=\"data:image/png;base64," XGE_TEST_RASTER_ALPHA_PNG "\"/>"
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
	static const char svg_external_image_fragment_parent[] =
		"<svg viewBox=\"0 0 14 6\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<image href=\"xge_shapeex_svg_external_use_child.svg#frag\" x=\"2\" y=\"1\" width=\"8\" height=\"3\" preserveAspectRatio=\"none\"/>"
		"</svg>";
	static const char svg_css_source_batch_16_classes[] =
		"<svg viewBox=\"0 0 64 32\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.paint{fill:#00f}.empty{fill:none}.hidden{display:none}.hidden{display:inline}.late{fill:none}.late{fill:#00f}.solid{stroke-dasharray:none}</style>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" class=\"paint empty\"/>"
		"<rect x=\"10\" y=\"0\" width=\"8\" height=\"8\" class=\"empty paint\"/>"
		"<rect x=\"20\" y=\"0\" width=\"8\" height=\"8\" class=\"hidden\" fill=\"#00f\"/>"
		"<rect x=\"30\" y=\"0\" width=\"8\" height=\"8\" class=\"late\"/>"
		"<g fill=\"none\" stroke=\"#00f\" stroke-width=\"4\" stroke-dasharray=\"8 8\"><path d=\"M0 20 H48\" class=\"solid\"/></g>"
		"</svg>";
	static const char svg_css_source_batch_16_filter[] =
		"<svg viewBox=\"0 0 128 64\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.blur{filter:url(#blur)}</style>"
		"<defs><filter id=\"blur\" filterUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"64\"><feGaussianBlur stdDeviation=\"4\"/></filter></defs>"
		"<rect x=\"32\" y=\"16\" width=\"64\" height=\"32\" class=\"blur\" fill=\"#00f\"/>"
		"</svg>";
	static const char svg_css_source_batch_16_miter[] =
		"<svg viewBox=\"0 0 128 64\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.join{stroke-miterlimit:1}</style>"
		"<path d=\"M24 56 L64 8 L104 56\" class=\"join\" fill=\"none\" stroke=\"#00f\" stroke-width=\"8\" stroke-linejoin=\"miter\"/>"
		"</svg>";
	static const char svg_css_source_batch_17_important_merge[] =
		"<svg viewBox=\"0 0 80 40\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".paint{fill:none!important}.paint{fill:#00f}"
		".move{transform:translate(8 0)!important}.move{transform:translate(40 0)}"
		".dash{stroke-dasharray:4 4!important}.dash{stroke-dasharray:16 8}"
		".current{color:#000f!important}.current{color:#0000}"
		"</style>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" class=\"paint\" style=\"fill:none\"/>"
		"<rect x=\"0\" y=\"12\" width=\"8\" height=\"8\" class=\"move\" fill=\"#00f\"/>"
		"<path d=\"M0 30 H80\" class=\"dash\" fill=\"none\" stroke=\"#00f\" stroke-width=\"2\"/>"
		"<rect x=\"60\" y=\"0\" width=\"8\" height=\"8\" class=\"current\" fill=\"currentColor\"/>"
		"</svg>";
	static const char svg_css_source_batch_18_important_precedence[] =
		"<svg viewBox=\"0 0 80 40\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".firstOpacity{opacity:0}.firstOpacity{opacity:1!important}"
		".firstDisplay{display:none}.firstDisplay{display:inline!important}"
		"</style>"
		"<rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" class=\"firstOpacity\" fill=\"#00f\"/>"
		"<rect x=\"10\" y=\"0\" width=\"8\" height=\"8\" class=\"firstDisplay\" fill=\"#00f\"/>"
		"<rect x=\"20\" y=\"0\" width=\"8\" height=\"8\" style=\"fill:none!important;fill:#00f\"/>"
		"<rect x=\"30\" y=\"0\" width=\"8\" height=\"8\" style=\"fill:none!important;fill:#00f!important\"/>"
		"<rect x=\"40\" y=\"0\" width=\"8\" height=\"8\" fill=\"#00f\" style=\"opacity:0!important;opacity:1\"/>"
		"<rect x=\"0\" y=\"12\" width=\"8\" height=\"8\" fill=\"#00f\" style=\"transform:translate(56 0)!important;transform:translate(8 0)\"/>"
		"<path d=\"M0 30 H80\" fill=\"none\" stroke=\"#00f\" stroke-width=\"2\" style=\"stroke-dasharray:4 4!important;stroke-dasharray:16 8\"/>"
		"</svg>";
	static const char svg_css_source_batch_19_url_order[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".clipTone{clip-path:url(#leftClip)}"
		".maskTone{mask:url(#leftMask)}"
		".tagTone{clip-path:url(#leftClip)}rect.tagTone{clip-path:url(#rightClip)}"
		".leftTone{mask:url(#leftMask)}.rightTone{mask:url(#rightMask)}"
		"</style>"
		"<defs>"
		"<clipPath id=\"leftClip\"><rect width=\"64\" height=\"100\"/></clipPath>"
		"<clipPath id=\"rightClip\"><rect x=\"64\" width=\"64\" height=\"100\"/></clipPath>"
		"<mask id=\"leftMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"<mask id=\"rightMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect x=\"64\" width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"</defs>"
		"<rect y=\"0\" class=\"clipTone\" clip-path=\"url(#rightClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"10\" clip-path=\"url(#rightClip)\" class=\"clipTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"20\" class=\"clipTone\" style=\"clip-path:url(#rightClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"30\" style=\"clip-path:url(#rightClip)\" class=\"clipTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"40\" class=\"maskTone\" mask=\"url(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"50\" mask=\"url(#rightMask)\" class=\"maskTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"60\" class=\"maskTone\" style=\"mask:url(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"70\" style=\"mask:url(#rightMask)\" class=\"maskTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"80\" class=\"tagTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"90\" class=\"leftTone rightTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"</svg>";
	static const char svg_css_source_batch_20_url_mutation[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>"
		".clipNone{clip-path:url(#leftClip);clip-path:none}"
		".clipInvalid{clip-path:url(#leftClip);clip-path:invalid}"
		".maskNone{mask:url(#leftMask);mask:none}"
		".maskInvalid{mask:url(#leftMask);mask:invalid}"
		".clipImportantNone{clip-path:url(#leftClip);clip-path:none!important}"
		"</style>"
		"<defs>"
		"<clipPath id=\"leftClip\"><rect width=\"64\" height=\"100\"/></clipPath>"
		"<mask id=\"leftMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"</defs>"
		"<rect y=\"0\" class=\"clipNone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"10\" class=\"clipInvalid\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"20\" class=\"maskNone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"30\" class=\"maskInvalid\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"40\" style=\"clip-path:url(#leftClip);clip-path:none\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"50\" style=\"clip-path:url(#leftClip);clip-path:invalid\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"60\" style=\"mask:url(#leftMask);mask:none\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"70\" style=\"mask:url(#leftMask);mask:invalid\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"80\" class=\"clipImportantNone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"90\" style=\"mask:url(#leftMask);mask:none!important\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"</svg>";
	static const char svg_css_source_batch_21_late_style[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<defs>"
		"<clipPath id=\"leftClip\"><rect width=\"64\" height=\"100\"/></clipPath>"
		"<clipPath id=\"rightClip\"><rect x=\"64\" width=\"64\" height=\"100\"/></clipPath>"
		"<mask id=\"leftMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"<mask id=\"rightMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect x=\"64\" width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"</defs>"
		"<rect y=\"0\" class=\"clipTone\" clip-path=\"url(#rightClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"10\" class=\"maskTone\" mask=\"url(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"20\" class=\"clipTone\" style=\"clip-path:url(#rightClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"30\" class=\"maskTone\" style=\"mask:url(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"40\" clip-path=\"url(#rightClip)\" class=\"clipTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"50\" style=\"mask:url(#rightMask)\" class=\"maskTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"60\" class=\"fillLate\" fill=\"#f00\" width=\"128\" height=\"8\"/>"
		"<rect y=\"70\" class=\"moveLate\" transform=\"translate(64 0)\" width=\"64\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"80\" class=\"leftTone rightTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"90\" class=\"tagTone\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<style>"
		".clipTone{clip-path:url(#leftClip)}.maskTone{mask:url(#leftMask)}"
		".fillLate{fill:none}.moveLate{transform:translate(0 0)}"
		".leftTone{clip-path:url(#leftClip)}.rightTone{clip-path:url(#rightClip)}"
		".tagTone{clip-path:url(#leftClip)}rect.tagTone{clip-path:url(#rightClip)}"
		"</style>"
		"</svg>";
	static const char svg_css_source_batch_22_url_prefix[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.clipQuoted{clip-path:urlfoo('#leftClip')}.maskQuoted{mask:urlfoo(\"#leftMask\")}</style>"
		"<defs>"
		"<clipPath id=\"leftClip\"><rect width=\"64\" height=\"100\"/></clipPath>"
		"<clipPath id=\"rightClip\"><rect x=\"64\" width=\"64\" height=\"100\"/></clipPath>"
		"<mask id=\"leftMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"<mask id=\"rightMask\" maskUnits=\"userSpaceOnUse\" maskContentUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"128\" height=\"100\"><rect x=\"64\" width=\"64\" height=\"100\" fill=\"#fff\"/></mask>"
		"</defs>"
		"<rect y=\"0\" clip-path=\"urlx(#leftClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"10\" mask=\"urlx(#leftMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"20\" clip-path=\"url (#leftClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"30\" mask=\"url (#leftMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"40\" style=\"clip-path:url(#leftClip);clip-path:urlx(#rightClip)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"50\" style=\"mask:url(#leftMask);mask:urlx(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"60\" class=\"clipQuoted\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"70\" class=\"maskQuoted\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"80\" clip-path=\"url(#leftClip)\" clip-path=\"url(#missing\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"<rect y=\"90\" mask=\"url(#leftMask)\" mask=\"URL(#rightMask)\" width=\"128\" height=\"8\" fill=\"#00f\"/>"
		"</svg>";
	static const char svg_css_source_batch_23_paint_url_prefix[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.fillTone{fill:urlfoo('#empty')}.strokeTone{stroke:urlfoo(\"#paint\")}</style>"
		"<defs>"
		"<linearGradient id=\"empty\"/>"
		"<linearGradient id=\"paint\"><stop stop-color=\"#fff\"/><stop offset=\"1\" stop-color=\"#fff\"/></linearGradient>"
		"</defs>"
		"<rect y=\"0\" width=\"128\" height=\"8\" fill=\"urlx(#empty)\"/>"
		"<path d=\"M0 14H128\" fill=\"none\" stroke=\"urlx(#paint)\" stroke-width=\"8\"/>"
		"<rect y=\"20\" width=\"128\" height=\"8\" fill=\"url (#empty)\"/>"
		"<path d=\"M0 34H128\" fill=\"none\" stroke=\"url (#paint)\" stroke-width=\"8\"/>"
		"<rect y=\"40\" width=\"128\" height=\"8\" style=\"fill:urlfoo('#empty')\"/>"
		"<path d=\"M0 54H128\" fill=\"none\" stroke-width=\"8\" style=\"stroke:urlfoo('#paint')\"/>"
		"<rect class=\"fillTone\" y=\"60\" width=\"128\" height=\"8\"/>"
		"<path class=\"strokeTone\" d=\"M0 74H128\" fill=\"none\" stroke-width=\"8\"/>"
		"<rect y=\"80\" width=\"128\" height=\"8\" fill=\"url(#empty)\" fill=\"url(#missing\"/>"
		"<path d=\"M0 94H128\" fill=\"none\" stroke=\"url(#paint)\" stroke=\"URL(#missing)\" stroke-width=\"8\"/>"
		"</svg>";
	static const char svg_css_source_batch_24_filter_url_prefix[] =
		"<svg viewBox=\"0 0 128 100\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>.filterClass{filter:urlfoo('#leftFilter')}</style>"
		"<defs><filter id=\"leftFilter\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"64\" height=\"100\"><feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect y=\"0\" width=\"128\" height=\"8\" fill=\"#00f\" filter=\"urlx(#leftFilter)\"/>"
		"<g filter=\"urlx(#leftFilter)\"><rect y=\"10\" width=\"128\" height=\"8\" fill=\"#00f\"/></g>"
		"<rect y=\"20\" width=\"128\" height=\"8\" fill=\"#00f\" filter=\"url (#leftFilter)\"/>"
		"<rect y=\"30\" width=\"128\" height=\"8\" fill=\"#00f\" filter=\"urlfoo('#leftFilter')\"/>"
		"<rect y=\"40\" width=\"128\" height=\"8\" fill=\"#00f\" style=\"filter:urlx(#leftFilter)\"/>"
		"<rect y=\"50\" width=\"128\" height=\"8\" fill=\"#00f\" style=\"filter:urlfoo('#leftFilter')\"/>"
		"<rect y=\"60\" width=\"128\" height=\"8\" fill=\"#00f\" filter=\"url(#leftFilter)\" filter=\"url(#missing\"/>"
		"<rect y=\"70\" width=\"128\" height=\"8\" fill=\"#00f\" style=\"filter:url(#leftFilter);filter:url(#missing\"/>"
		"<rect y=\"80\" width=\"128\" height=\"8\" fill=\"#00f\" filter=\"url(#leftFilter)\" filter=\"URL(#missing)\"/>"
		"<rect y=\"90\" width=\"128\" height=\"8\" fill=\"#00f\" class=\"filterClass\"/>"
		"</svg>";
	static const char svg_source_batch_25_filter_duplicate_id[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"wrong\" id=\"right\" filterUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"50\" height=\"100\">"
		"<feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect width=\"100\" height=\"100\" fill=\"#00f\" filter=\"url(#right)\"/></svg>";
	static const char svg_source_batch_25_filter_duplicate_region[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"100\" "
		"x=\"50\" y=\"40\" width=\"30\" height=\"20\"><feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect width=\"100\" height=\"100\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_filter_user_default_region[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\"><feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect width=\"100\" height=\"100\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_filter_units_monotonic[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"objectBoundingBox\" filterUnits=\"userSpaceOnUse\" "
		"x=\"0\" y=\"0\" width=\"50\" height=\"100\"><feGaussianBlur stdDeviation=\"0\"/></filter></defs>"
		"<rect width=\"100\" height=\"100\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_primitive_units_monotonic[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" primitiveUnits=\"objectBoundingBox\" "
		"x=\"0\" y=\"0\" width=\"100\" height=\"100\"><feGaussianBlur x=\"0%\" y=\"0%\" width=\"50%\" height=\"100%\" stdDeviation=\"0.001\"/></filter></defs>"
		"<rect x=\"20\" y=\"40\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_primitive_duplicate_region[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"objectBoundingBox\" "
		"x=\"0\" y=\"0\" width=\"100\" height=\"100\"><feGaussianBlur "
		"x=\"0%\" y=\"0%\" width=\"100%\" height=\"100%\" x=\"50%\" y=\"25%\" width=\"50%\" height=\"50%\" stdDeviation=\"0.001\"/></filter></defs>"
		"<rect x=\"20\" y=\"20\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_std_duplicate[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"100\">"
		"<feGaussianBlur stdDeviation=\"2\" stdDeviation=\"0\"/></filter></defs>"
		"<rect x=\"40\" y=\"40\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_std_negative_reset[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"100\">"
		"<feGaussianBlur stdDeviation=\"2\" stdDeviation=\"-1 4\"/></filter></defs>"
		"<rect x=\"40\" y=\"40\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_std_double_comma[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"100\">"
		"<feGaussianBlur stdDeviation=\"0,,2\"/></filter></defs>"
		"<rect x=\"40\" y=\"40\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const char svg_source_batch_25_std_em[] =
		"<svg viewBox=\"0 0 100 100\"><defs>"
		"<filter id=\"f\" filterUnits=\"userSpaceOnUse\" primitiveUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"100\" height=\"100\">"
		"<feGaussianBlur stdDeviation=\"0.001em\"/></filter></defs>"
		"<rect x=\"40\" y=\"40\" width=\"20\" height=\"20\" fill=\"#00f\" filter=\"url(#f)\"/></svg>";
	static const unsigned char external_png_pixels[] = {
		255, 0, 0, 255,     0, 255, 0, 255,
		0, 0, 255, 255,     255, 255, 0, 255
	};
	static const char svg_provider_image[] =
		"<svg viewBox=\"0 0 16 12\"><image x=\"3\" y=\"2\" width=\"4\" height=\"5\" "
		"preserveAspectRatio=\"none\" href=\"svgprobe://asset.png\"/></svg>";
	xge_svg svg;
	xge_svg cached_a;
	xge_svg cached_b;
	xge_svg cached_c;
	xge_rect_t viewbox;
	xge_rect_t viewport;
	xge_rect_t bounds;
	char font_face_svg[4096];
	unsigned char raster_pixels[16];
	xge_texture_t raster_texture;
	xge_resource_provider_t asset_provider;
	test_svg_resource_provider_state_t asset_provider_state;
	float custom_font_width;
	int contains;
	const char* cache_path = "build\\xge_shapeex_svg_smoke_tmp.svg";
	const char* external_path = "build\\xge_shapeex_svg_external_parent.svg";
	const char* external_literal_path = "build\\xge_shapeex_svg_external_literal_parent.svg";
	const char* external_child_svg_path = "build\\xge_shapeex_svg_external child.svg";
	const char* external_child_png_path = "build\\xge_shapeex_svg_external child.png";
	const char* external_use_path = "build\\xge_shapeex_svg_external_use_parent.svg";
	const char* external_use_child_path = "build\\xge_shapeex_svg_external_use_child.svg";
	const char* external_image_fragment_path = "build\\xge_shapeex_svg_external_image_fragment_parent.svg";
	FILE* file;
	int ret;

	svg = NULL;
	cached_a = NULL;
	cached_b = NULL;
	cached_c = NULL;
	memset(raster_pixels, 0, sizeof(raster_pixels));
	memset(&raster_texture, 0, sizeof(raster_texture));
	ret = xgeSvgRasterizeMemory(svg_text, (int)strlen(svg_text), 2, 2, raster_pixels, 0);
	if ( !check(ret == XGE_ERROR_NOT_INITIALIZED, "SVG rasterize memory requires initialized renderer") ) return 0;
	ret = xgeSvgTextureLoadMemory(&raster_texture, svg_text, (int)strlen(svg_text), 2, 2);
	if ( !check(ret == XGE_ERROR_NOT_INITIALIZED, "SVG texture memory load requires initialized renderer") ) return 0;
	ret = xgeSvgRasterizeMemory(NULL, 0, 2, 2, raster_pixels, 0);
	if ( !check(ret == XGE_ERROR_INVALID_ARGUMENT, "SVG rasterize memory rejects invalid source") ) return 0;
	ret = xgeSvgTextureLoadMemory(NULL, svg_text, (int)strlen(svg_text), 2, 2);
	if ( !check(ret == XGE_ERROR_INVALID_ARGUMENT, "SVG texture memory load rejects null texture") ) return 0;
	ret = xgeSvgCreate(&svg);
	if ( !check((ret == XGE_OK) && (svg != NULL), "SVG create") ) return 0;
	ret = xgeSvgDraw(NULL, (xge_rect_t){0.0f, 0.0f, 32.0f, 16.0f}, 0.25f);
	if ( !check(ret == XGE_ERROR_INVALID_ARGUMENT, "SVG world draw rejects null picture") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text));
	if ( !check(ret == XGE_OK, "SVG load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fX == 0.0f) && (viewbox.fY == 0.0f) && (viewbox.fW == 32.0f) && (viewbox.fH == 16.0f), "SVG viewBox") ) return 0;
	ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	if ( !check((ret == XGE_OK) && (viewport.fX == 0.0f) && (viewport.fY == 32.0f) && (viewport.fW == 64.0f) && (viewport.fH == 32.0f), "SVG meet viewport") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_bounds_smoke, (int)strlen(svg_bounds_smoke));
	if ( !check(ret == XGE_OK, "SVG bounds load memory") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 9.9f) && (bounds.fX < 10.1f) && (bounds.fY > 10.9f) && (bounds.fY < 11.1f) && (bounds.fW > 31.9f) && (bounds.fW < 70.1f) && (bounds.fH > 6.9f) && (bounds.fH < 20.1f), "SVG local bounds") ) return 0;
	ret = xgeSvgGetDrawBounds(svg, (xge_rect_t){0.0f, 0.0f, 200.0f, 100.0f}, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 19.9f) && (bounds.fX < 20.1f) && (bounds.fY > 21.9f) && (bounds.fY < 22.1f) && (bounds.fW > 63.9f) && (bounds.fW < 140.1f) && (bounds.fH > 13.9f) && (bounds.fH < 40.1f), "SVG draw bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_contains_smoke, (int)strlen(svg_contains_smoke));
	if ( !check(ret == XGE_OK, "SVG contains load memory") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG contains local fill hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 11.0f, 11.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG contains local stroke-only hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 16.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG contains hidden miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 18.0f, 18.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG contains local miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 2.0f, 0.05f, NULL);
	if ( !check(ret == XGE_ERROR_INVALID_ARGUMENT, "SVG contains rejects null output") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_visibility_ignored_thorvg_compat, (int)strlen(svg_visibility_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG visibility ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores hidden visibility") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores inherited hidden visibility") ) return 0;
	ret = xgeSvgContainsPoint(svg, 17.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores invalid visibility") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_vector_effect_ignored_thorvg_compat, (int)strlen(svg_vector_effect_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG vector-effect ignored parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fY < 10.4f) && (bounds.fH > 3.3f), "SVG ThorVG ignores non-scaling-stroke") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_contains_smoke, (int)strlen(svg_contains_smoke));
	if ( !check(ret == XGE_OK, "SVG contains reload memory") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){100.0f, 50.0f, 200.0f, 100.0f}, 160.0f, 60.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG contains draw fill hit") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){100.0f, 50.0f, 200.0f, 100.0f}, 205.0f, 105.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG contains draw stroke-only hit") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){100.0f, 50.0f, 200.0f, 100.0f}, 110.0f, 60.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG contains draw viewport gutter miss") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){100.0f, 50.0f, 200.0f, 100.0f}, 99.0f, 60.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG contains draw root clip miss") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){100.0f, 50.0f, -1.0f, 100.0f}, 160.0f, 60.0f, 0.05f, &contains);
	if ( !check(ret == XGE_ERROR_INVALID_ARGUMENT, "SVG draw contains rejects invalid rect") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_nested_bounds_smoke, (int)strlen(svg_nested_bounds_smoke));
	if ( !check(ret == XGE_OK, "SVG nested bounds load memory") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 17.9f) && (bounds.fX < 18.1f) && (bounds.fY > 15.9f) && (bounds.fY < 16.1f) && (bounds.fW > 15.9f) && (bounds.fW < 16.1f) && (bounds.fH > 9.9f) && (bounds.fH < 10.1f), "SVG nested local bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_nested_contains_smoke, (int)strlen(svg_nested_contains_smoke));
	if ( !check(ret == XGE_OK, "SVG nested contains load memory") ) return 0;
	ret = xgeSvgContainsPoint(svg, 30.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested contains child hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 22.5f, 16.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG nested contains bounds corner miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_raster_contains_smoke, (int)strlen(svg_raster_contains_smoke));
	if ( !check(ret == XGE_OK, "SVG raster contains load memory") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG raster contains alpha hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 7.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG raster contains transparent pixel miss") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 40.0f, 40.0f}, 6.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG raster draw contains alpha hit") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 40.0f, 40.0f}, 14.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG raster draw contains transparent pixel miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text));
	if ( !check(ret == XGE_OK, "SVG reload smoke after bounds") ) return 0;
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
	if ( !check((ret == XGE_OK) && (viewbox.fW > 59.9f) && (viewbox.fW < 60.1f) && (viewbox.fH > 19.9f) && (viewbox.fH < 20.1f), "SVG em/ex root length units use ThorVG default font size") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_region, (int)strlen(svg_filter_region));
	if ( !check(ret == XGE_OK, "SVG ThorVG unsupported filters ignored load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) && (bounds.fY > 2.9f) && (bounds.fY < 3.1f) && (bounds.fW > 3.9f) && (bounds.fW < 4.1f) && (bounds.fH > 4.9f) && (bounds.fH < 5.1f), "SVG ThorVG unsupported filters preserve source bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG unsupported filters preserve source hit") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_gaussian_blur, (int)strlen(svg_filter_gaussian_blur));
	if ( !check(ret == XGE_OK, "SVG filter gaussian blur parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_active_bounds, (int)strlen(svg_filter_active_bounds));
	if ( !check(ret == XGE_OK, "SVG active gaussian effect load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 3.9f) && (bounds.fX < 4.1f) && (bounds.fY > 3.9f) && (bounds.fY < 4.1f) && (bounds.fW > 11.9f) && (bounds.fW < 12.1f) && (bounds.fH > 11.9f) && (bounds.fH < 12.1f), "SVG active gaussian ThorVG kernel bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG active gaussian visual bounds hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG active gaussian outside output miss") ) return 0;
	ret = xgeSvgGetDrawBounds(svg, (xge_rect_t){0.0f, 0.0f, 40.0f, 40.0f}, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 7.9f) && (bounds.fX < 8.1f) && (bounds.fY > 7.9f) && (bounds.fY < 8.1f) && (bounds.fW > 23.9f) && (bounds.fW < 24.1f) && (bounds.fH > 23.9f) && (bounds.fH < 24.1f), "SVG active gaussian ThorVG kernel draw bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_transform_bounds, (int)strlen(svg_filter_transform_bounds));
	if ( !check(ret == XGE_OK, "SVG transformed gaussian effect load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 5.9f) && (bounds.fX < 6.1f) && (bounds.fY > 5.9f) && (bounds.fY < 6.1f) && (bounds.fW > 27.9f) && (bounds.fW < 28.1f) && (bounds.fH > 23.9f) && (bounds.fH < 24.1f), "SVG transformed gaussian ThorVG kernel bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG transformed gaussian visual bounds hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG transformed gaussian outside miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_group_bounds, (int)strlen(svg_filter_group_bounds));
	if ( !check(ret == XGE_OK, "SVG group gaussian effect load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 3.9f) && (bounds.fX < 4.1f) && (bounds.fY > 3.9f) && (bounds.fY < 4.1f) && (bounds.fW > 21.9f) && (bounds.fW < 22.1f) && (bounds.fH > 15.9f) && (bounds.fH < 16.1f), "SVG group gaussian ThorVG kernel bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_forward_shape, (int)strlen(svg_filter_forward_shape));
	if ( !check(ret == XGE_OK, "SVG forward gaussian shape reference load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 8.9f) && (bounds.fX < 9.1f) &&
	           (bounds.fY > 6.9f) && (bounds.fY < 7.1f) &&
	           (bounds.fW > 9.9f) && (bounds.fW < 10.1f) &&
	           (bounds.fH > 7.9f) && (bounds.fH < 8.1f), "SVG forward gaussian ThorVG kernel shape bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 9.5f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG forward gaussian ThorVG kernel visual hit") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_forward_group, (int)strlen(svg_filter_forward_group));
	if ( !check(ret == XGE_OK, "SVG forward gaussian group reference load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 6.9f) && (bounds.fX < 7.1f) &&
	           (bounds.fY > 6.9f) && (bounds.fY < 7.1f) &&
	           (bounds.fW > 13.9f) && (bounds.fW < 14.1f) &&
	           (bounds.fH > 7.9f) && (bounds.fH < 8.1f), "SVG forward gaussian ThorVG kernel group bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_def_shape_use, (int)strlen(svg_filter_def_shape_use));
	if ( !check(ret == XGE_OK, "SVG defs filtered shape use load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 18.9f) && (bounds.fX < 19.1f) &&
	           (bounds.fY > 6.9f) && (bounds.fY < 7.1f) &&
	           (bounds.fW > 9.9f) && (bounds.fW < 10.1f) &&
	           (bounds.fH > 7.9f) && (bounds.fH < 8.1f), "SVG defs filtered shape use ThorVG kernel bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_def_group_use, (int)strlen(svg_filter_def_group_use));
	if ( !check(ret == XGE_OK, "SVG defs filtered group use load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 14.9f) && (bounds.fX < 15.1f) &&
	           (bounds.fY > 6.9f) && (bounds.fY < 7.1f) &&
	           (bounds.fW > 13.9f) && (bounds.fW < 14.1f) &&
	           (bounds.fH > 7.9f) && (bounds.fH < 8.1f), "SVG defs filtered group use ThorVG kernel bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_object_bbox_numeric_primitive_region, (int)strlen(svg_filter_object_bbox_numeric_primitive_region));
	if ( !check(ret == XGE_OK, "SVG object bbox numeric primitive region load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG object bbox numeric primitive region uses user coordinates") ) return 0;
	ret = xgeSvgContainsPoint(svg, 24.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG object bbox numeric primitive region clips output") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_filter_identity_clip, (int)strlen(svg_filter_identity_clip));
	if ( !check(ret == XGE_OK, "SVG identity gaussian filter load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) && (bounds.fY > 3.9f) && (bounds.fY < 4.1f) && (bounds.fW > 3.9f) && (bounds.fW < 4.1f) && (bounds.fH > 3.9f) && (bounds.fH < 4.1f), "SVG identity gaussian applies filter region") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG identity gaussian clipped hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG identity gaussian clipped miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_shapes, (int)strlen(svg_clip_shapes));
	if ( !check(ret == XGE_OK, "SVG vector clip shape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_mixed_union, (int)strlen(svg_clip_mixed_union));
	if ( !check(ret == XGE_OK, "SVG mixed clip path union parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 0.9f) && (bounds.fX < 1.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 19.8f) && (bounds.fW < 20.2f) && (bounds.fH > 7.8f) && (bounds.fH < 8.2f), "SVG mixed clip path union bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mixed clip path rect contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 18.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mixed clip path circle contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG mixed clip path gap misses") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_multi_mixed_union, (int)strlen(svg_clip_multi_mixed_union));
	if ( !check(ret == XGE_OK, "SVG multi mixed clip path union parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 0.9f) && (bounds.fX < 1.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 19.8f) && (bounds.fW < 20.2f) && (bounds.fH > 7.8f) && (bounds.fH < 8.2f), "SVG multi mixed clip path union bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG multi mixed clip path first rect contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 9.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG multi mixed clip path second rect contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 18.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG multi mixed clip path circle contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG multi mixed clip path gap misses") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_nested_clip_path, (int)strlen(svg_clip_nested_clip_path));
	if ( !check(ret == XGE_OK, "SVG nested clip path parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 17.9f) && (bounds.fW < 18.1f) && (bounds.fH > 8.9f) && (bounds.fH < 9.1f), "SVG nested clip path ThorVG bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested clip path inner contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested clip path ThorVG outer-only contains") ) return 0;
	ret = xgeSvgContainsPoint(svg, 21.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG nested clip path ThorVG outside misses") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_shapes, (int)strlen(svg_mask_shapes));
	if ( !check(ret == XGE_OK, "SVG vector mask shape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_shapeex_scene, (int)strlen(svg_mask_shapeex_scene));
	if ( !check(ret == XGE_OK, "SVG ShapeEx scene mask parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) &&
	           (bounds.fY > 1.9f) && (bounds.fY < 2.1f) &&
	           (bounds.fW > 34.9f) && (bounds.fW < 35.1f) &&
	           (bounds.fH > 5.9f) && (bounds.fH < 6.1f), "SVG ShapeEx scene mask bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx scene mask first component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ShapeEx scene mask component gap miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 14.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx scene mask second component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 23.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx scene mask use transform hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 30.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ShapeEx scene mask use transform gap miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 34.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx scene mask use transform second hit") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_shapeex_transform, (int)strlen(svg_mask_shapeex_transform));
	if ( !check(ret == XGE_OK, "SVG ShapeEx transformed rect mask parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 3.9f) && (bounds.fX < 4.1f) &&
	           (bounds.fY > 1.9f) && (bounds.fY < 2.1f) &&
	           (bounds.fW > 25.9f) && (bounds.fW < 26.1f) &&
	           (bounds.fH > 5.9f) && (bounds.fH < 6.1f), "SVG ShapeEx transformed rect mask bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx rotated rect mask hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ShapeEx rotated rect mask rejects old AABB area") ) return 0;
	ret = xgeSvgContainsPoint(svg, 25.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ShapeEx skewed rect mask hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 22.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ShapeEx skewed rect mask rejects old AABB area") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_external_scene_hit, (int)strlen(svg_mask_external_scene_hit));
	if ( !check(ret == XGE_OK, "SVG external scene masks parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG text scene mask first component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 11.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG text scene mask component gap miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 16.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG text scene mask second component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 44.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested image scene mask first component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 54.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG nested image scene mask component gap miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested image scene mask second component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 84.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG raster scene mask first component hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 94.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG raster scene mask component gap miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 104.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG raster scene mask second component hit") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_type_alpha_ignored, (int)strlen(svg_mask_type_alpha_ignored));
	if ( !check(ret == XGE_OK, "SVG mask-type alpha ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG mask-type alpha black uses luminance") ) return 0;
	ret = xgeSvgContainsPoint(svg, 9.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mask-type alpha white uses luminance") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_type_upper_alpha, (int)strlen(svg_mask_type_upper_alpha));
	if ( !check(ret == XGE_OK, "SVG mask-type uppercase Alpha parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mask-type uppercase Alpha attribute uses alpha") ) return 0;
	ret = xgeSvgContainsPoint(svg, 11.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mask-type uppercase Alpha inline style uses alpha") ) return 0;
	ret = xgeSvgContainsPoint(svg, 19.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG mask-type uppercase Alpha CSS class is not copied") ) return 0;
	ret = xgeSvgContainsPoint(svg, 27.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG invalid mask-type uses luminance") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_luminance_batch_32, (int)strlen(svg_mask_luminance_batch_32));
	if ( !check(ret == XGE_OK, "SVG batch 32 luminance masks parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 default black luminance mask misses") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 32 explicit Alpha black mask hits") ) return 0;
	ret = xgeSvgContainsPoint(svg, 25.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 text luminance method propagates") ) return 0;
	ret = xgeSvgContainsPoint(svg, 35.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 nested image luminance method propagates") ) return 0;
	ret = xgeSvgContainsPoint(svg, 45.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 raster luminance method propagates") ) return 0;
	ret = xgeSvgContainsPoint(svg, 55.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 linear gradient mask follows ThorVG empty paint") ) return 0;
	ret = xgeSvgContainsPoint(svg, 65.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 32 radial gradient mask follows ThorVG empty paint") ) return 0;
	ret = xgeSvgContainsPoint(svg, 71.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 32 luminance stroke mask remains visible") ) return 0;
	ret = xgeSvgContainsPoint(svg, 82.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 32 object bounding box mask resolves") ) return 0;
	ret = xgeSvgContainsPoint(svg, 95.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 32 use mask preserves real paint") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_gradient_batch_33, (int)strlen(svg_mask_gradient_batch_33));
	if ( !check(ret == XGE_OK, "SVG batch 33 gradient masks parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 30.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 luminance linear user mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 60.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 alpha linear user mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 100.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 luminance radial user mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 140.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 alpha radial user mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 190.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 luminance object box mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 220.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 alpha object box mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 270.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 luminance forward gradient mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 300.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 alpha forward gradient mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 350.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 transformed gradient mask is empty") ) return 0;
	ret = xgeSvgContainsPoint(svg, 365.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 33 gradient stroke mask is empty") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_nested_image_viewport_batch_34, (int)strlen(svg_nested_image_viewport_batch_34));
	if ( !check(ret == XGE_OK, "SVG batch 34 nested image viewport parse") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "none");
	if ( !check(ret == XGE_OK, "SVG batch 34 root nonuniform viewport") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 2.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 unmasked nested image keeps local viewport") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 12.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 alpha masked nested image keeps local viewport") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 22.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 luminance masked nested image keeps local viewport") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 32.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 none aspect composes before parent scale") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 44.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 vector image ignores xMin meet like ThorVG") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 50.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 vector image ignores xMid meet like ThorVG") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 60.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 vector image ignores xMax meet like ThorVG") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 74.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 vector image ignores slice like ThorVG") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 83.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 image transform composes with child viewport") ) return 0;
	ret = xgeSvgDrawContainsPoint(svg, (xge_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, 93.5f, 25.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 34 group transform composes with child viewport") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_transform_stroke_batch_35, (int)strlen(svg_image_transform_stroke_batch_35));
	if ( !check(ret == XGE_OK, "SVG batch 35 transformed image strokes parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 rotated image suppresses solid stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 rotated group suppresses image stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 25.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 skewX image suppresses stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 35.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 skewY image suppresses stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 45.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 affine image suppresses stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 56.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 35 translated image preserves stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 65.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 35 uniform scale image preserves stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 75.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG batch 35 nonuniform scale image preserves stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 85.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 rotated image suppresses gradient stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 95.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG batch 35 rotated image suppresses dashed stroke") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_order_markers, (int)strlen(svg_paint_order_markers));
	if ( !check(ret == XGE_OK, "SVG paint-order markers parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_order_invalid, (int)strlen(svg_paint_order_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid paint-order parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_invalid, (int)strlen(svg_paint_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid paint parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_paint_invalid_gradient, (int)strlen(svg_paint_invalid_gradient));
	if ( !check(ret == XGE_OK, "SVG invalid gradient paint parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_width_invalid, (int)strlen(svg_stroke_width_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-width parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_length_suffix, (int)strlen(svg_length_suffix));
	if ( !check(ret == XGE_OK, "SVG length suffix parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dasharray_invalid, (int)strlen(svg_stroke_dasharray_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-dasharray parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG invalid dasharray becomes solid") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG dasharray keeps valid prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 9.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG negative dasharray becomes solid") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dashoffset_invalid, (int)strlen(svg_stroke_dashoffset_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-dashoffset parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG invalid dashoffset defaults to zero") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG non-finite dashoffset rejects dashed stroke") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dashoffset_percent, (int)strlen(svg_stroke_dashoffset_percent));
	if ( !check(ret == XGE_OK, "SVG stroke-dashoffset percent parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_length, (int)strlen(svg_path_length));
	if ( !check(ret == XGE_OK, "SVG pathLength parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_length_dash_hit, (int)strlen(svg_path_length_dash_hit));
	if ( !check(ret == XGE_OK, "SVG pathLength dash hit parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG pathLength ignored first dash") ) return 0;
	ret = xgeSvgContainsPoint(svg, 45.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG pathLength ignored first gap") ) return 0;
	ret = xgeSvgContainsPoint(svg, 70.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG pathLength ignored second dash") ) return 0;
	ret = xgeSvgContainsPoint(svg, 95.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG pathLength ignored second gap") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_line_path_length_dash_hit, (int)strlen(svg_line_path_length_dash_hit));
	if ( !check(ret == XGE_OK, "SVG line pathLength dash hit parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG line pathLength ignored first dash") ) return 0;
	ret = xgeSvgContainsPoint(svg, 45.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG line pathLength ignored first gap") ) return 0;
	ret = xgeSvgContainsPoint(svg, 70.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG line pathLength ignored second dash") ) return 0;
	ret = xgeSvgContainsPoint(svg, 95.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG line pathLength ignored second gap") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_duplicate_invalid, (int)strlen(svg_stroke_duplicate_invalid));
	if ( !check(ret == XGE_OK, "SVG duplicate invalid stroke parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG duplicate miter keeps trailing invalid zero") ) return 0;
	ret = xgeSvgContainsPoint(svg, 36.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG duplicate miter keeps trailing valid value") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_dash_inherit, (int)strlen(svg_stroke_dash_inherit));
	if ( !check(ret == XGE_OK, "SVG stroke dash inherit parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_style_inherit_core, (int)strlen(svg_style_inherit_core));
	if ( !check(ret == XGE_OK, "SVG style inherit core parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_xml_markup_boundaries, (int)strlen(svg_xml_markup_boundaries));
	if ( !check(ret == XGE_OK, "SVG XML markup boundary parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_xml_entities, (int)strlen(svg_xml_entities));
	if ( !check(ret == XGE_OK, "SVG XML entity parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG numeric attribute entities remain unresolved") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_xml_named_href_entity, (int)strlen(svg_xml_named_href_entity));
	if ( !check(ret == XGE_OK, "SVG ThorVG named href entity parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG decimal href entity resolves bare id") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text_baseline_ignored, (int)strlen(svg_text_baseline_ignored));
	if ( !check(ret == XGE_OK, "SVG ignored text baseline attrs parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_symbol_overflow, (int)strlen(svg_symbol_overflow));
	if ( !check(ret == XGE_OK, "SVG symbol overflow parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 36.0f, 12.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG symbol ignores style overflow") ) return 0;
	ret = xgeSvgContainsPoint(svg, 42.0f, 12.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG symbol keeps clipped content") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_nested_svg_viewport_clip, (int)strlen(svg_nested_svg_viewport_clip));
	if ( !check(ret == XGE_OK, "SVG nested svg as group parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -8.1f) && (bounds.fX < -7.9f) && (bounds.fY > -0.1f) && (bounds.fY < 0.1f) && (bounds.fW > 31.9f) && (bounds.fW < 32.1f) && (bounds.fH > 11.9f) && (bounds.fH < 12.1f), "SVG nested svg as group bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, -4.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG nested svg as group hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 40.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG nested svg ignores viewport offset") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_mask_nested_svg, (int)strlen(svg_clip_mask_nested_svg));
	if ( !check(ret == XGE_OK, "SVG clip/mask nested svg parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_nested_group_ignored, (int)strlen(svg_clip_nested_group_ignored));
	if ( !check(ret == XGE_OK, "SVG nested clip group parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG clip ignores nested group geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_nested_group_position, (int)strlen(svg_mask_nested_group_position));
	if ( !check(ret == XGE_OK, "SVG nested mask group parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG nested mask ignores svg viewport offset") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_rect_invalid_radii, (int)strlen(svg_rect_invalid_radii));
	if ( !check(ret == XGE_OK, "SVG invalid rect radii parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 1.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG invalid single rect radius keeps left geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_rect_radius_thorvg_compat, (int)strlen(svg_rect_radius_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG rect radius parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG rect negative rx keeps declared geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_rect_radius_thorvg_compat, (int)strlen(svg_clip_rect_radius_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG clip rect radius parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG clip rect negative rx keeps declared geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mask_rect_radius_thorvg_compat, (int)strlen(svg_mask_rect_radius_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG mask rect radius parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG mask rect negative rx keeps declared geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_rect_dash_start_thorvg_compat, (int)strlen(svg_rect_dash_start_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG rect dash start parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG rect dash starts at top-left") ) return 0;
	ret = xgeSvgContainsPoint(svg, 16.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG rect dash top edge gap") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_circle_dash_start_thorvg_compat, (int)strlen(svg_circle_dash_start_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG circle dash start parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 17.7f, 13.8f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG circle dash starts at right") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG circle top gap") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_ellipse_dash_start_thorvg_compat, (int)strlen(svg_ellipse_dash_start_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG ellipse dash start parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 21.4f, 13.6f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ellipse dash starts at right") ) return 0;
	ret = xgeSvgContainsPoint(svg, 14.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG ellipse top gap") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_points_prefix_thorvg_compat, (int)strlen(svg_points_prefix_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG points prefix parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG points prefix polyline hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 44.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG points bad suffix ignored") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 12.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG points prefix polygon hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 26.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG points dangling coordinate ignored after prefix") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_prefix_thorvg_compat, (int)strlen(svg_path_prefix_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG path prefix parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG path prefix line hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 44.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG path bad suffix ignored") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 10.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG path keeps prefix before bad command") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_points_invalid, (int)strlen(svg_points_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid points parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_points_invalid_clip_mask, (int)strlen(svg_points_invalid_clip_mask));
	if ( !check(ret == XGE_OK, "SVG invalid points clip mask parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_invalid, (int)strlen(svg_path_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid path parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_incomplete_commands, (int)strlen(svg_path_incomplete_commands));
	if ( !check(ret == XGE_OK, "SVG incomplete path command parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_number_grammar, (int)strlen(svg_path_number_grammar));
	if ( !check(ret == XGE_OK, "SVG path number grammar parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_path_quadratic, (int)strlen(svg_path_quadratic));
	if ( !check(ret == XGE_OK, "SVG quadratic path parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_zero_length_caps, (int)strlen(svg_stroke_zero_length_caps));
	if ( !check(ret == XGE_OK, "SVG zero length stroke caps parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_zero_length_caps_hit, (int)strlen(svg_stroke_zero_length_caps_hit));
	if ( !check(ret == XGE_OK, "SVG zero length stroke caps hit parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 28.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG zero butt cap contains miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 48.0f, 18.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG zero round cap contains hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 70.9f, 30.9f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG zero square cap contains hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 75.0f, 35.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG zero square cap contains miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_join_contains, (int)strlen(svg_stroke_join_contains));
	if ( !check(ret == XGE_OK, "SVG stroke join contains parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 7.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG miter join contains hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 6.7f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG miter join contains miss") ) return 0;
	ret = xgeSvgContainsPoint(svg, 60.0f, 9.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG bevel join contains hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 60.0f, 7.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG bevel join contains miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_miter_turn_limit_2, (int)strlen(svg_stroke_miter_turn_limit_2));
	if ( !check(ret == XGE_OK, "SVG turn miter bevel limit parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -2.01f) && (bounds.fX < -1.99f), "SVG turn miter exceeded limit uses bevel bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, -3.0f, 11.0f, 0.01f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG turn bevel excludes miter tip") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_miter_turn_limit_3, (int)strlen(svg_stroke_miter_turn_limit_3));
	if ( !check(ret == XGE_OK, "SVG turn miter accepted limit parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -4.84f) && (bounds.fX < -4.81f), "SVG turn miter accepted limit extends bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, -3.0f, 11.0f, 0.01f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG turn miter includes tip") ) return 0;
	ret = xgeSvgContainsPoint(svg, 10.0f, -1.0f, 0.01f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG turn miter includes center wedge") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_number_grammar_attrs, (int)strlen(svg_number_grammar_attrs));
	if ( !check(ret == XGE_OK, "SVG attr number grammar parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_length_number_grammar, (int)strlen(svg_length_number_grammar));
	if ( !check(ret == XGE_OK, "SVG length number grammar parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_circle_ellipse_attrs, (int)strlen(svg_circle_ellipse_attrs));
	if ( !check(ret == XGE_OK, "SVG circle ellipse attr parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_ellipse_bounds, (int)strlen(svg_ellipse_bounds));
	if ( !check(ret == XGE_OK, "SVG ellipse bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 10.9f) && (bounds.fX < 11.1f) && (bounds.fY > 8.9f) && (bounds.fY < 9.1f) && (bounds.fW > 17.9f) && (bounds.fW < 18.1f) && (bounds.fH > 9.9f) && (bounds.fH < 10.1f), "SVG ellipse bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_mix_blend_mode, (int)strlen(svg_mix_blend_mode));
	if ( !check(ret == XGE_OK, "SVG mix-blend-mode parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_blend_group_composition, (int)strlen(svg_blend_group_composition));
	if ( !check(ret == XGE_OK, "SVG blend group composition parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 3.9f) && (bounds.fX < 4.1f) &&
	           (bounds.fY > 4.9f) && (bounds.fY < 5.1f) && (bounds.fW > 48.9f) &&
	           (bounds.fW < 49.1f) && (bounds.fH > 10.9f) && (bounds.fH < 11.1f),
	           "SVG blend group recursive bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 6.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG blend group recursive hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 50.0f, 13.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG blend use recursive hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 30.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG blend group recursive miss") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_miterlimit_invalid, (int)strlen(svg_stroke_miterlimit_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke-miterlimit parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 0.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG invalid miter becomes zero bevel") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 0.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG NaN miter remains unbounded") ) return 0;
	ret = xgeSvgContainsPoint(svg, 19.0f, 0.5f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG negative miter preserves inheritance") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_stroke_line_enum_invalid, (int)strlen(svg_stroke_line_enum_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid stroke line enum parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 0.75f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG invalid stroke-linecap defaults to butt") ) return 0;
	ret = xgeSvgContainsPoint(svg, 16.75f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inherited stroke-linecap remains round") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_url_reference_invalid, (int)strlen(svg_url_reference_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid URL reference parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG child clip none preserves group clip") ) return 0;
	ret = xgeSvgContainsPoint(svg, 18.0f, 15.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG child mask none preserves group mask") ) return 0;
	ret = xgeSvgContainsPoint(svg, 14.5f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG group filter is not inherited by child") ) return 0;
	ret = xgeSvgContainsPoint(svg, 19.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG invalid child filter does not inherit group filter") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_url_function_thorvg_strict, (int)strlen(svg_url_function_thorvg_strict));
	if ( !check(ret == XGE_OK, "SVG ThorVG strict url function parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 78.0f, 22.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG url function is case-sensitive for clip-path") ) return 0;
	ret = xgeSvgContainsPoint(svg, 20.0f, 60.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG url function is case-sensitive for filter") ) return 0;
	ret = xgeSvgContainsPoint(svg, 142.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG url function is case-sensitive for markers") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_transform_thorvg_strict, (int)strlen(svg_transform_thorvg_strict));
	if ( !check(ret == XGE_OK, "SVG ThorVG strict transform parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 31.0f, 21.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG transform names are case-sensitive") ) return 0;
	ret = xgeSvgContainsPoint(svg, 88.0f, 22.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG transform angle units ignored") ) return 0;
	ret = xgeSvgContainsPoint(svg, 130.0f, 18.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG transform length units ignored") ) return 0;
	ret = xgeSvgContainsPoint(svg, 150.0f, 32.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG invalid transform does not move geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_bounds_hit, (int)strlen(svg_marker_bounds_hit));
	if ( !check(ret == XGE_OK, "SVG ignored marker parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 8.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ignored marker does not add hit geometry") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_quadratic_path, (int)strlen(svg_marker_quadratic_path));
	if ( !check(ret == XGE_OK, "SVG ignored quadratic marker path parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_arc_segment_mid, (int)strlen(svg_marker_arc_segment_mid));
	if ( !check(ret == XGE_OK, "SVG ignored arc marker parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_closed_path, (int)strlen(svg_marker_closed_path));
	if ( !check(ret == XGE_OK, "SVG ignored closed path marker parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_invalid_reference, (int)strlen(svg_marker_invalid_reference));
	if ( !check(ret == XGE_OK, "SVG ignored invalid marker reference parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_marker_mixed_content, (int)strlen(svg_marker_mixed_content));
	if ( !check(ret == XGE_OK, "SVG ignored marker mixed content parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_markers, (int)strlen(svg_use_markers));
	if ( !check(ret == XGE_OK, "SVG ignored use markers parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_inherit, (int)strlen(svg_use_inherit));
	if ( !check(ret == XGE_OK, "SVG use inherited paint parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_group_use, (int)strlen(svg_group_use));
	if ( !check(ret == XGE_OK, "SVG group use parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_defs_svg_use, (int)strlen(svg_defs_svg_use));
	if ( !check(ret == XGE_OK, "SVG defs svg use parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_gradient_stop_offset_invalid, (int)strlen(svg_gradient_stop_offset_invalid));
	if ( !check(ret == XGE_OK, "SVG gradient invalid stop offset parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_gradient_stop_style_order, (int)strlen(svg_gradient_stop_style_order));
	if ( !check(ret == XGE_OK, "SVG gradient stop style order parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_gradient_stop_case, (int)strlen(svg_gradient_stop_case));
	if ( !check(ret == XGE_OK, "SVG gradient stop CSS case parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_enum_inherit_invalid, (int)strlen(svg_enum_inherit_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid enum inherit parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_style_invalid_keywords, (int)strlen(svg_style_invalid_keywords));
	if ( !check(ret == XGE_OK, "SVG invalid style keywords parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 8.0f, 16.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG invalid display keyword defaults visible") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_style_duplicate_invalid, (int)strlen(svg_style_duplicate_invalid));
	if ( !check(ret == XGE_OK, "SVG duplicate invalid style parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 16.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG stylesheet duplicate keeps first display declaration") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text_style_duplicate_invalid, (int)strlen(svg_text_style_duplicate_invalid));
	if ( !check(ret == XGE_OK, "SVG duplicate invalid text style parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 22.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores visibility style") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_color_function_grammar, (int)strlen(svg_color_function_grammar));
	if ( !check(ret == XGE_OK, "SVG color function grammar parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_color_source_replay, (int)strlen(svg_css_color_source_replay));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS color replay parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS invalid RGB preserves fill") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG CSS none disables fill") ) return 0;
	ret = xgeSvgContainsPoint(svg, 24.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS invalid RGB reactivates stroke") ) return 0;
	ret = xgeSvgContainsPoint(svg, 28.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG CSS stroke remains unfilled") ) return 0;
	ret = xgeSvgContainsPoint(svg, 39.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS invalid RGB preserves currentColor") ) return 0;
	ret = xgeSvgContainsPoint(svg, 50.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS important color replay") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_hsl_grammar, (int)strlen(svg_hsl_grammar));
	if ( !check(ret == XGE_OK, "SVG HSL grammar parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_property_case, (int)strlen(svg_css_property_case));
	if ( !check(ret == XGE_OK, "SVG CSS property case parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_keyword_case, (int)strlen(svg_css_keyword_case));
	if ( !check(ret == XGE_OK, "SVG CSS keyword case parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_attribute_case_flag, (int)strlen(svg_css_attribute_case_flag));
	if ( !check(ret == XGE_OK, "SVG CSS attribute case flag parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_identifier_escape, (int)strlen(svg_css_identifier_escape));
	if ( !check(ret == XGE_OK, "SVG CSS identifier escape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_attribute_escape, (int)strlen(svg_css_attribute_escape));
	if ( !check(ret == XGE_OK, "SVG CSS attribute escape parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_url_escape, (int)strlen(svg_css_url_escape));
	if ( !check(ret == XGE_OK, "SVG CSS URL escape parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG CSS escaped paint URL remains unresolved") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_nth_spacing, (int)strlen(svg_css_nth_spacing));
	if ( !check(ret == XGE_OK, "SVG CSS nth spacing parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_pseudo_case, (int)strlen(svg_css_pseudo_case));
	if ( !check(ret == XGE_OK, "SVG CSS pseudo case parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_media_rule, (int)strlen(svg_css_media_rule));
	if ( !check(ret == XGE_OK, "SVG CSS media rule parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_thorvg_supported_selectors, (int)strlen(svg_css_thorvg_supported_selectors));
	if ( !check(ret == XGE_OK, "SVG ThorVG supported CSS selectors parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS tag selector applies") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS class selector applies") ) return 0;
	ret = xgeSvgContainsPoint(svg, 30.0f, 5.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS tag class selector applies") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_16_classes, (int)strlen(svg_css_source_batch_16_classes));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 16 class semantics parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG CSS later class token overwrites earlier token") ) return 0;
	ret = xgeSvgContainsPoint(svg, 14.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS class token order is preserved") ) return 0;
	ret = xgeSvgContainsPoint(svg, 24.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG duplicate class display keeps first declaration") ) return 0;
	ret = xgeSvgContainsPoint(svg, 34.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG duplicate class fill keeps last declaration") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 20.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG CSS empty dash array does not replace inherited dash") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_16_filter, (int)strlen(svg_css_source_batch_16_filter));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 16 filter semantics parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 31.9f) && (bounds.fX < 32.1f) &&
	            (bounds.fY > 15.9f) && (bounds.fY < 16.1f) && (bounds.fW > 63.9f) &&
	            (bounds.fW < 64.1f) && (bounds.fH > 31.9f) && (bounds.fH < 32.1f),
	            "SVG ThorVG CSS filter property is not copied") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_16_miter, (int)strlen(svg_css_source_batch_16_miter));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 16 miter semantics parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fY < 2.0f), "SVG ThorVG CSS miter limit property is not copied") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_17_important_merge, (int)strlen(svg_css_source_batch_17_important_merge));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 17 important merge parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG class keeps last fill value and important state") ) return 0;
	ret = xgeSvgContainsPoint(svg, 44.0f, 16.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG class keeps last transform value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 16.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG class transform replaces earlier important value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG class dash merge first on segment") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG class dash merge first gap") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG class dash merge appended on segment") ) return 0;
	ret = xgeSvgContainsPoint(svg, 26.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG class dash merge appended gap") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS rejects four-channel short hex color") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_18_important_precedence, (int)strlen(svg_css_source_batch_18_important_precedence));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 18 important precedence parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 4.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG class opacity keeps first value with later important") ) return 0;
	ret = xgeSvgContainsPoint(svg, 14.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG class display keeps first value with later important") ) return 0;
	ret = xgeSvgContainsPoint(svg, 24.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline important fill blocks later normal value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 34.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline later important fill replaces earlier important value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 44.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline important opacity blocks later normal value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 60.0f, 16.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline important transform blocks later normal value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 16.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline normal transform does not replace important value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline important dash first segment") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 30.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline important dash blocks later normal pattern") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_19_url_order, (int)strlen(svg_css_source_batch_19_url_order));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 19 URL order parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class-before-attribute clip keeps later attribute") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class-before-attribute clip rejects earlier class URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG attribute-before-class clip keeps later class") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG attribute-before-class clip rejects earlier attribute URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class-before-inline clip keeps later inline style") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class-before-inline clip rejects earlier class URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline-before-class clip keeps later class") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline-before-class clip rejects earlier inline URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class-before-attribute mask keeps later attribute") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class-before-attribute mask rejects earlier class URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG attribute-before-class mask keeps later class") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG attribute-before-class mask rejects earlier attribute URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class-before-inline mask keeps later inline style") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class-before-inline mask rejects earlier class URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline-before-class mask keeps later class") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline-before-class mask rejects earlier inline URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG tag.class clip rule applies after class replay") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG tag.class clip rule replaces generic class URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG multi-class mask follows class token order") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG multi-class mask replaces earlier class URL") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_20_url_mutation, (int)strlen(svg_css_source_batch_20_url_mutation));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 20 URL mutation parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class clip URL survives later none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class clip none does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class clip URL survives later invalid value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class invalid clip does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class mask URL survives later none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class mask none does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class mask URL survives later invalid value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class invalid mask does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline clip URL survives later none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline clip none does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline clip URL survives later invalid value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline invalid clip does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline mask URL survives later none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline mask none does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline mask URL survives later invalid value") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline invalid mask does not clear earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class clip URL survives important none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class important clip none does not clear URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline mask URL survives important none") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline important mask none does not clear URL") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_21_late_style, (int)strlen(svg_css_source_batch_21_late_style));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 21 late style parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class clip replaces later presentation attribute") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class clip rejects presentation attribute URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class mask replaces later presentation attribute") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class mask rejects presentation attribute URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class clip replaces later inline style") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class clip rejects inline URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class mask replaces later inline style") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class mask rejects inline URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class clip applies after earlier presentation attribute") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class clip replaces earlier presentation URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class mask applies after earlier inline style") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class mask replaces earlier inline URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class fill overwrites presentation fill") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed class transform replaces presentation position") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed class transform overwrites presentation transform") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed multi-class clip follows token order") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed multi-class clip replaces earlier token") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG postponed tag.class clip overrides generic class") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG postponed tag.class clip replaces generic class URL") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_22_url_prefix, (int)strlen(svg_css_source_batch_22_url_prefix));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 22 URL prefix parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG clip accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG lowercase clip url prefix resolves referenced geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mask accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG lowercase mask url prefix resolves referenced geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG clip accepts space after lowercase url prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG spaced clip url prefix preserves clipping") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG mask accepts space after lowercase url prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG spaced mask url prefix preserves masking") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline extended clip URL replaces earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline extended clip URL rejects earlier geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline extended mask URL replaces earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline extended mask URL rejects earlier geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class clip accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class quoted clip URL resolves referenced geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class mask accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class quoted mask URL resolves referenced geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG malformed lowercase clip URL clears earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG malformed lowercase clip URL restores full geometry") ) return 0;
	ret = xgeSvgContainsPoint(svg, 32.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG uppercase mask URL preserves earlier lowercase URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG uppercase mask URL does not replace earlier lowercase URL") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_23_paint_url_prefix, (int)strlen(svg_css_source_batch_23_paint_url_prefix));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 23 paint URL prefix parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG fill accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG stroke accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG fill accepts space after lowercase url prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG stroke accepts space after lowercase url prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline fill accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG inline stroke accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG class fill accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG class stroke accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG malformed lowercase fill URL clears earlier paint URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 64.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG uppercase stroke URL preserves earlier lowercase paint URL") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_source_batch_24_filter_url_prefix, (int)strlen(svg_css_source_batch_24_filter_url_prefix));
	if ( !check(ret == XGE_OK, "SVG ThorVG CSS source batch 24 filter URL prefix parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG filter accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 14.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG group filter accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 24.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG filter accepts space after lowercase url prefix") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 34.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG filter accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 44.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline filter accepts lowercase url prefix extension") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 54.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG inline filter accepts quoted extended URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 64.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG malformed lowercase filter URL clears earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 74.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG malformed inline filter URL clears earlier URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 84.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG uppercase filter URL preserves earlier lowercase URL") ) return 0;
	ret = xgeSvgContainsPoint(svg, 96.0f, 94.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG CSS copier omits filter") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_filter_duplicate_id, (int)strlen(svg_source_batch_25_filter_duplicate_id));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 duplicate filter id parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -0.1f) && (bounds.fX < 0.1f) &&
	           (bounds.fY > -0.1f) && (bounds.fY < 0.1f) && (bounds.fW > 49.9f) && (bounds.fW < 50.1f) &&
	           (bounds.fH > 99.9f) && (bounds.fH < 100.1f), "SVG filter id uses last source attribute") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_filter_duplicate_region, (int)strlen(svg_source_batch_25_filter_duplicate_region));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 duplicate filter region parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 49.9f) && (bounds.fX < 50.1f) &&
	           (bounds.fY > 39.9f) && (bounds.fY < 40.1f) && (bounds.fW > 29.9f) && (bounds.fW < 30.1f) &&
	           (bounds.fH > 19.9f) && (bounds.fH < 20.1f), "SVG filter region uses last source attributes") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_filter_user_default_region, (int)strlen(svg_source_batch_25_filter_user_default_region));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 user default filter region parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -0.1f) && (bounds.fX < 0.1f) &&
	           (bounds.fY > -0.1f) && (bounds.fY < 0.1f) && (bounds.fW > 1.09f) && (bounds.fW < 1.11f) &&
	           (bounds.fH > 1.09f) && (bounds.fH < 1.11f), "SVG user-space filter keeps ThorVG raw default region") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_filter_units_monotonic, (int)strlen(svg_source_batch_25_filter_units_monotonic));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 filter units parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -0.1f) && (bounds.fX < 0.1f) &&
	           (bounds.fW > 49.9f) && (bounds.fW < 50.1f), "SVG filter units retain user space once selected") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_primitive_units_monotonic, (int)strlen(svg_source_batch_25_primitive_units_monotonic));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 primitive units parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 19.9f) && (bounds.fX < 20.1f) &&
	           (bounds.fW > 9.9f) && (bounds.fW < 10.1f), "SVG primitive units retain object bounds once selected") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_primitive_duplicate_region, (int)strlen(svg_source_batch_25_primitive_duplicate_region));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 duplicate primitive region parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 29.9f) && (bounds.fX < 30.1f) &&
	           (bounds.fY > 24.9f) && (bounds.fY < 25.1f) && (bounds.fW > 9.9f) && (bounds.fW < 10.1f) &&
	           (bounds.fH > 9.9f) && (bounds.fH < 10.1f), "SVG primitive region uses last source attributes") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_std_duplicate, (int)strlen(svg_source_batch_25_std_duplicate));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 duplicate deviation parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 39.9f) && (bounds.fX < 40.1f) &&
	           (bounds.fY > 39.9f) && (bounds.fY < 40.1f) && (bounds.fW > 19.9f) && (bounds.fW < 20.1f) &&
	           (bounds.fH > 19.9f) && (bounds.fH < 20.1f), "SVG gaussian deviation uses last source attribute") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_std_negative_reset, (int)strlen(svg_source_batch_25_std_negative_reset));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 negative deviation reset parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 39.9f) && (bounds.fX < 40.1f) &&
	           (bounds.fY > 39.9f) && (bounds.fY < 40.1f) && (bounds.fW > 19.9f) && (bounds.fW < 20.1f) &&
	           (bounds.fH > 19.9f) && (bounds.fH < 20.1f), "SVG negative duplicate deviation resets both axes") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_std_double_comma, (int)strlen(svg_source_batch_25_std_double_comma));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 double comma deviation parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 39.9f) && (bounds.fX < 40.1f) &&
	           (bounds.fY > 39.9f) && (bounds.fY < 40.1f) && (bounds.fW > 19.9f) && (bounds.fW < 20.1f) &&
	           (bounds.fH > 19.9f) && (bounds.fH < 20.1f), "SVG gaussian parser skips only one comma per value") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_source_batch_25_std_em, (int)strlen(svg_source_batch_25_std_em));
	if ( !check(ret == XGE_OK, "SVG ThorVG source batch 25 em deviation parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 39.9f) && (bounds.fX < 40.1f) &&
	           (bounds.fY > 39.9f) && (bounds.fY < 40.1f) && (bounds.fW > 19.9f) && (bounds.fW < 20.1f) &&
	           (bounds.fH > 19.9f) && (bounds.fH < 20.1f), "SVG gaussian parser accepts ThorVG em number suffix with zero-radius kernel") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_complex_selectors_ignored_thorvg_compat, (int)strlen(svg_css_complex_selectors_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG complex CSS selectors ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG ignores complex CSS selectors") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_style_first_only_thorvg_compat, (int)strlen(svg_style_first_only_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG first style only parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores later style nodes") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_style_first_in_unsupported_container_thorvg_compat, (int)strlen(svg_style_first_in_unsupported_container_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG first style in unsupported container parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG first style in unsupported container applies") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_switch_conditional, (int)strlen(svg_switch_conditional));
	if ( !check(ret == XGE_OK, "SVG switch conditional parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_switch_style_scope, (int)strlen(svg_switch_style_scope));
	if ( !check(ret == XGE_OK, "SVG switch style scope parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_switch_selector_scope, (int)strlen(svg_switch_selector_scope));
	if ( !check(ret == XGE_OK, "SVG switch selector scope parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_switch_container_inherit, (int)strlen(svg_switch_container_inherit));
	if ( !check(ret == XGE_OK, "SVG switch container inheritance parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_conditional_processing_attrs, (int)strlen(svg_conditional_processing_attrs));
	if ( !check(ret == XGE_OK, "SVG conditional processing attr parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_switch_ignored_thorvg_compat, (int)strlen(svg_switch_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG switch ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG switch ignores requiredExtensions") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG switch ignores systemLanguage") ) return 0;
	ret = xgeSvgContainsPoint(svg, 23.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG switch attributes ignored") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_conditional_attrs_ignored_thorvg_compat, (int)strlen(svg_conditional_attrs_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG conditional attrs ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores requiredExtensions attr") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores requiredFeatures attr") ) return 0;
	ret = xgeSvgContainsPoint(svg, 27.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG ignores systemLanguage attr") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_anchor_container, (int)strlen(svg_anchor_container));
	if ( !check(ret == XGE_OK, "SVG anchor container parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 1.0f, 1.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG anchor children remain in current parent") ) return 0;
	ret = xgeSvgContainsPoint(svg, 50.0f, 40.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG anchor attributes are ignored") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_metadata_skip, (int)strlen(svg_metadata_skip));
	if ( !check(ret == XGE_OK, "SVG metadata skip parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_unsupported_container_children_thorvg_compat, (int)strlen(svg_unsupported_container_children_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG unsupported container children parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG metadata children render") ) return 0;
	ret = xgeSvgContainsPoint(svg, 15.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG title children render") ) return 0;
	ret = xgeSvgContainsPoint(svg, 27.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG desc children render") ) return 0;
	ret = xgeSvgContainsPoint(svg, 39.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG script children render") ) return 0;
	ret = xgeSvgContainsPoint(svg, 51.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG foreignObject children render") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_css_complex_selector_ignored_thorvg_compat, (int)strlen(svg_css_complex_selector_ignored_thorvg_compat));
	if ( !check(ret == XGE_OK, "SVG ThorVG complex selector ignored parse") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG ignores child combinator selector") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_xlink_href, (int)strlen(svg_xlink_href));
	if ( !check(ret == XGE_OK, "SVG xlink href parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_radial_gradient_fr, (int)strlen(svg_radial_gradient_fr));
	if ( !check(ret == XGE_OK, "SVG radial gradient focal radius parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_opacity_invalid, (int)strlen(svg_opacity_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid opacity parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_opacity_duplicate_invalid, (int)strlen(svg_opacity_duplicate_invalid));
	if ( !check(ret == XGE_OK, "SVG duplicate invalid opacity parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_size_invalid, (int)strlen(svg_font_size_invalid));
	if ( !check(ret == XGE_OK, "SVG invalid font-size parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_size_style_ignored, (int)strlen(svg_font_size_style_ignored));
	if ( !check(ret == XGE_OK, "SVG duplicate invalid style font-size parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_size_invalid_bounds, (int)strlen(svg_font_size_invalid_bounds));
	if ( !check(ret == XGE_OK, "SVG invalid presentation font-size parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG invalid presentation font-size suppresses text") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_size_style_ignored_bounds, (int)strlen(svg_font_size_style_ignored_bounds));
	if ( !check(ret == XGE_OK, "SVG style and group font-size ignored parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW > 19.9f) && (bounds.fW < 20.1f) &&
	            (bounds.fH > 9.9f) && (bounds.fH < 10.1f),
	            "SVG style and group font-size use ThorVG default text size") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_data_uri_policy, (int)strlen(svg_image_data_uri_policy));
	if ( !check(ret == XGE_OK, "SVG image data URI policy parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_svg_data_charset, (int)strlen(svg_image_svg_data_charset));
	if ( !check(ret == XGE_OK, "SVG image SVG data URI charset parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_svg_utf8_bounds, (int)strlen(svg_image_svg_utf8_bounds));
	if ( !check(ret == XGE_OK, "SVG image UTF-8 data URI parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX == 1.0f) && (bounds.fY == 1.0f) && (bounds.fW == 4.0f) && (bounds.fH == 4.0f), "SVG image UTF-8 data URI bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_svg_charset_bounds, (int)strlen(svg_image_svg_charset_bounds));
	if ( !check(ret == XGE_OK, "SVG image charset data URI parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX == 11.0f) && (bounds.fY == 1.0f) && (bounds.fW == 4.0f) && (bounds.fH == 4.0f), "SVG image ThorVG charset encoding bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_svg_fragment_ignored_bounds, (int)strlen(svg_image_svg_fragment_ignored_bounds));
	if ( !check(ret == XGE_OK, "SVG image data URI fragment parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG image ThorVG data URI fragment ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_pattern_overflow_hidden_bounds, (int)strlen(svg_pattern_overflow_hidden_bounds));
	if ( !check(ret == XGE_OK, "SVG pattern overflow hidden bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG pattern hidden paint ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_pattern_overflow_visible_bounds, (int)strlen(svg_pattern_overflow_visible_bounds));
	if ( !check(ret == XGE_OK, "SVG pattern overflow visible bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG pattern visible paint ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_pattern_overflow_visible_far_bounds, (int)strlen(svg_pattern_overflow_visible_far_bounds));
	if ( !check(ret == XGE_OK, "SVG pattern overflow visible far bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG pattern far paint ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_pattern_stroke_ignored_bounds, (int)strlen(svg_pattern_stroke_ignored_bounds));
	if ( !check(ret == XGE_OK, "SVG pattern stroke ignored bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG pattern stroke ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_paint_ignored_bounds, (int)strlen(svg_clip_paint_ignored_bounds));
	if ( !check(ret == XGE_OK, "SVG clip paint ignored bounds parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 4.9f) && (bounds.fW < 5.1f) && (bounds.fH > 3.9f) && (bounds.fH < 4.1f), "SVG clip paint ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_clip_use_rule_ignored, (int)strlen(svg_clip_use_rule_ignored));
	if ( !check(ret == XGE_OK, "SVG ignored clip-rule on referenced clip parse") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_ancestor_ignored, (int)strlen(svg_use_ancestor_ignored));
	if ( !check(ret == XGE_OK, "SVG ancestor use parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 0.9f) && (bounds.fX < 1.1f) && (bounds.fY > 1.9f) && (bounds.fY < 2.1f) && (bounds.fW > 3.9f) && (bounds.fW < 4.1f) && (bounds.fH > 2.9f) && (bounds.fH < 3.1f), "SVG ThorVG ancestor use ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_cycle_ignored, (int)strlen(svg_use_cycle_ignored));
	if ( !check(ret == XGE_OK, "SVG circular use parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 19.9f) && (bounds.fX < 20.1f) && (bounds.fY > 1.9f) && (bounds.fY < 2.1f) && (bounds.fW > 1.9f) && (bounds.fW < 2.1f) && (bounds.fH > 1.9f) && (bounds.fH < 2.1f), "SVG ThorVG circular use discarded bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_nested_gradient_ignored, (int)strlen(svg_nested_gradient_ignored));
	if ( !check(ret == XGE_OK, "SVG nested gradient parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 0.9f) && (bounds.fX < 1.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 3.9f) && (bounds.fW < 4.1f) && (bounds.fH > 3.9f) && (bounds.fH < 4.1f), "SVG ThorVG nested gradient ignored bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 12.0f, 3.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG nested gradient ignored hit") ) return 0;
	ret = snprintf(font_face_svg, sizeof(font_face_svg),
		"<svg viewBox=\"0 0 180 64\" xmlns=\"http://www.w3.org/2000/svg\">"
		"<style>@font-face{font-family:'XgeTiny';src:url('data:font/ttf;base64,%s');}</style>"
		"<text x=\"10\" y=\"50\" font-family=\"XgeTiny\" font-size=\"44\">ABCDE</text>"
		"</svg>", svg_font_face_test_data);
	if ( !check((ret > 0) && (ret < (int)sizeof(font_face_svg)), "SVG embedded font fixture build") ) return 0;
	ret = xgeSvgLoadMemory(svg, font_face_svg, (int)strlen(font_face_svg));
	if ( !check(ret == XGE_OK, "SVG embedded TTF font-face parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW > 125.0f) && (bounds.fW < 145.0f), "SVG embedded TTF font selected with EM scaling") ) return 0;
	custom_font_width = bounds.fW;
	ret = xgeSvgClear(svg);
	if ( !check(ret == XGE_OK, "SVG embedded font resources clear") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_face_fallback, (int)strlen(svg_font_face_fallback));
	if ( !check(ret == XGE_OK, "SVG unknown font fallback parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (fabsf(bounds.fW - custom_font_width) > 2.0f), "SVG font cache cleared and unknown family falls back") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_font_face_malformed, (int)strlen(svg_font_face_malformed));
	if ( !check(ret == XGE_OK, "SVG malformed embedded font ignored") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (fabsf(bounds.fW - custom_font_width) > 2.0f), "SVG malformed embedded font uses fallback") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_data_uri_case, (int)strlen(svg_use_data_uri_case));
	if ( !check(ret == XGE_OK, "SVG ThorVG ignored use SVG data URI parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG ignored use SVG data URI bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_data_uri_fragment, (int)strlen(svg_use_data_uri_fragment));
	if ( !check(ret == XGE_OK, "SVG ThorVG ignored use SVG data URI fragment parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG ignored use SVG data URI fragment bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_data_uri_fragment, (int)strlen(svg_image_data_uri_fragment));
	if ( !check(ret == XGE_OK, "SVG image SVG data URI fragment parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG ignored image SVG data URI fragment bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG ignored image SVG data URI fragment hit") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_bare_id, (int)strlen(svg_use_bare_id));
	if ( !check(ret == XGE_OK, "SVG ThorVG bare use id parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 1.9f) && (bounds.fX < 2.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 7.9f) && (bounds.fW < 8.1f) && (bounds.fH > 4.9f) && (bounds.fH < 5.1f), "SVG ThorVG bare use id bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_url_local_ignored, (int)strlen(svg_use_url_local_ignored));
	if ( !check(ret == XGE_OK, "SVG ThorVG URL use id ignored parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 8.9f) && (bounds.fX < 9.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 0.9f) && (bounds.fW < 1.1f) && (bounds.fH > 0.9f) && (bounds.fH < 1.1f), "SVG ThorVG URL use id ignored bounds") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_descendant_element_transform, (int)strlen(svg_use_descendant_element_transform));
	if ( !check(ret == XGE_OK, "SVG descendant element use transform parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 5.9f) && (bounds.fX < 6.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 6.9f) && (bounds.fW < 7.1f) && (bounds.fH > 5.9f) && (bounds.fH < 6.1f), "SVG descendant element use excludes ancestor transform") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_use_descendant_group_transform, (int)strlen(svg_use_descendant_group_transform));
	if ( !check(ret == XGE_OK, "SVG descendant group use transform parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 6.9f) && (bounds.fX < 7.1f) && (bounds.fY > -0.1f) && (bounds.fY < 0.1f) && (bounds.fW > 6.9f) && (bounds.fW < 7.1f) && (bounds.fH > 5.9f) && (bounds.fH < 6.1f), "SVG descendant group use excludes ancestor transform") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_image_jpg_data_uri, (int)strlen(svg_image_jpg_data_uri));
	if ( !check(ret == XGE_OK, "SVG image JPG data URI parse") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 0.9f) && (bounds.fX < 1.1f) && (bounds.fY > 0.9f) && (bounds.fY < 1.1f) && (bounds.fW > 9.9f) && (bounds.fW < 10.1f) && (bounds.fH > 3.9f) && (bounds.fH < 4.1f), "SVG image JPG data URI bounds") ) return 0;
	ret = xgeImageSavePNG(external_child_png_path, 2, 2, external_png_pixels, 8);
	if ( !check(ret == XGE_OK, "SVG external PNG file write") ) return 0;
	memset(&asset_provider, 0, sizeof(asset_provider));
	memset(&asset_provider_state, 0, sizeof(asset_provider_state));
	asset_provider_state.sPath = external_child_png_path;
	asset_provider.sScheme = "svgprobe";
	asset_provider.load = test_svg_resource_provider_load;
	asset_provider.free = test_svg_resource_provider_free;
	asset_provider.pUser = &asset_provider_state;
	xgeResourceProviderClear();
	ret = xgeResourceProviderAdd(&asset_provider);
	if ( !check(ret == XGE_OK, "SVG custom asset provider register") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_provider_image, (int)strlen(svg_provider_image));
	if ( !check(ret == XGE_OK, "SVG custom asset provider image load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > 2.9f) && (bounds.fX < 3.1f) &&
	            (bounds.fY > 1.9f) && (bounds.fY < 2.1f) &&
	            (bounds.fW > 3.9f) && (bounds.fW < 4.1f) &&
	            (bounds.fH > 4.9f) && (bounds.fH < 5.1f),
	            "SVG custom asset provider image bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 5.0f, 4.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG custom asset provider image hit") ) return 0;
	if ( !check((asset_provider_state.iLoadCount == 1) && (asset_provider_state.iFreeCount == 1),
	            "SVG custom asset provider resource lifetime") ) return 0;
	xgeResourceProviderClear();
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
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG encoded external images ignored bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG encoded external images ignored hit") ) return 0;
	file = fopen(external_literal_path, "wb");
	if ( !check(file != NULL, "SVG literal external image parent file open") ) return 0;
	if ( fwrite(svg_external_parent_literal, 1, strlen(svg_external_parent_literal), file) != strlen(svg_external_parent_literal) ) {
		fclose(file);
		printf("xge smoke failed: SVG literal external image parent file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoad(svg, external_literal_path);
	if ( !check(ret == XGE_OK, "SVG literal external image load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fX > -0.1f) && (bounds.fX < 0.1f) && (bounds.fY > -0.1f) && (bounds.fY < 0.1f) && (bounds.fW > 3.9f) && (bounds.fW < 4.1f) && (bounds.fH > 3.9f) && (bounds.fH < 4.1f), "SVG ThorVG literal raster loads and external SVG is ignored") ) return 0;
	ret = xgeSvgContainsPoint(svg, 2.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && contains, "SVG ThorVG literal external raster hit") ) return 0;
	ret = xgeSvgContainsPoint(svg, 6.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG literal external SVG ignored hit") ) return 0;
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
	if ( !check(ret == XGE_OK, "SVG ThorVG ignored external use load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG ignored external use bounds") ) return 0;
	file = fopen(external_image_fragment_path, "wb");
	if ( !check(file != NULL, "SVG external image fragment parent file open") ) return 0;
	if ( fwrite(svg_external_image_fragment_parent, 1, strlen(svg_external_image_fragment_parent), file) != strlen(svg_external_image_fragment_parent) ) {
		fclose(file);
		printf("xge smoke failed: SVG external image fragment parent file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoad(svg, external_image_fragment_path);
	if ( !check(ret == XGE_OK, "SVG external image fragment load") ) return 0;
	ret = xgeSvgGetBounds(svg, 0.05f, &bounds);
	if ( !check((ret == XGE_OK) && (bounds.fW == 0.0f) && (bounds.fH == 0.0f), "SVG ThorVG ignored external image fragment bounds") ) return 0;
	ret = xgeSvgContainsPoint(svg, 3.0f, 2.0f, 0.05f, &contains);
	if ( !check((ret == XGE_OK) && !contains, "SVG ThorVG ignored external image fragment hit") ) return 0;
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
	remove(external_literal_path);
	remove(external_child_svg_path);
	remove(external_child_png_path);
	remove(external_use_path);
	remove(external_use_child_path);
	remove(external_image_fragment_path);
#undef XGE_TEST_RASTER_PNG
#undef XGE_TEST_RASTER_ALPHA_PNG
#undef XGE_TEST_RASTER_JPEG
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
	if ( !test_shape_ex_scene_traverse() ) return 1;
	if ( !test_shape_ex() ) return 1;
	if ( !test_svg() ) return 1;
	printf("xge smoke passed\n");
	return 0;
}
