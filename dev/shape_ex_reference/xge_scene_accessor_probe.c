#include "xge.h"

#include <stdio.h>
#include <string.h>

typedef struct traverse_state_t {
	int count;
	int stop_after;
	int types[8];
	uint32_t ids[8];
	int root_ref_count;
} traverse_state_t;

static int visit_paint(const xge_shape_ex_scene_child_t* paint, void* data)
{
	traverse_state_t* state = (traverse_state_t*)data;
	uint32_t id = 0;

	if ( paint == NULL || state == NULL || state->count >= 8 ) return 0;
	if ( paint->iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE ) {
		if ( xgeShapeExSceneIdGet(paint->pScene, &id) != XGE_OK ) return 0;
		if ( state->count == 0 &&
		     xgeShapeExSceneRefCountGet(paint->pScene, &state->root_ref_count) != XGE_OK ) return 0;
	} else if ( paint->iType == XGE_SHAPE_EX_SCENE_CHILD_SHAPE ) {
		if ( xgeShapeExIdGet(paint->pShape, &id) != XGE_OK ) return 0;
	} else {
		return 0;
	}
	state->types[state->count] = paint->iType;
	state->ids[state->count] = id;
	state->count++;
	return state->stop_after <= 0 || state->count < state->stop_after;
}

static void dump_state(const char* prefix, const traverse_state_t* state)
{
	int i;

	printf("%s_count %d\n", prefix, state->count);
	printf("%s_types", prefix);
	for ( i = 0; i < state->count; i++ ) printf(" %d", state->types[i]);
	printf("\n");
	printf("%s_ids", prefix);
	for ( i = 0; i < state->count; i++ ) printf(" %u", (unsigned int)state->ids[i]);
	printf("\n");
}

int main(void)
{
	xge_shape_ex_scene root = NULL;
	xge_shape_ex_scene nested = NULL;
	xge_shape_ex_scene deep = NULL;
	xge_shape_ex shapes[4] = {NULL, NULL, NULL, NULL};
	traverse_state_t state;
	int i;

	if ( xgeShapeExSceneCreate(&root) != XGE_OK ||
	     xgeShapeExSceneCreate(&nested) != XGE_OK ||
	     xgeShapeExSceneCreate(&deep) != XGE_OK ) return 1;
	for ( i = 0; i < 4; i++ ) {
		if ( xgeShapeExCreate(&shapes[i]) != XGE_OK ) return 2;
	}
	if ( xgeShapeExSceneId(root, 100u) != XGE_OK ||
	     xgeShapeExId(shapes[0], 101u) != XGE_OK ||
	     xgeShapeExSceneId(nested, 102u) != XGE_OK ||
	     xgeShapeExId(shapes[1], 103u) != XGE_OK ||
	     xgeShapeExSceneId(deep, 104u) != XGE_OK ||
	     xgeShapeExId(shapes[2], 105u) != XGE_OK ||
	     xgeShapeExId(shapes[3], 106u) != XGE_OK ) return 3;
	if ( xgeShapeExSceneAdd(root, shapes[0]) != XGE_OK ||
	     xgeShapeExSceneAdd(nested, shapes[1]) != XGE_OK ||
	     xgeShapeExSceneAdd(deep, shapes[2]) != XGE_OK ||
	     xgeShapeExSceneAddScene(nested, deep) != XGE_OK ||
	     xgeShapeExSceneAddScene(root, nested) != XGE_OK ||
	     xgeShapeExSceneAdd(root, shapes[3]) != XGE_OK ) return 4;

	printf("invalid %d %d\n",
		xgeShapeExSceneTraverse(NULL, visit_paint, &state) == XGE_ERROR_INVALID_ARGUMENT,
		xgeShapeExSceneTraverse(root, NULL, &state) == XGE_ERROR_INVALID_ARGUMENT);
	memset(&state, 0, sizeof(state));
	if ( xgeShapeExSceneTraverse(root, visit_paint, &state) != XGE_OK ) return 5;
	dump_state("full", &state);
	printf("root_ref %d\n", state.root_ref_count);
	memset(&state, 0, sizeof(state));
	state.stop_after = 4;
	if ( xgeShapeExSceneTraverse(root, visit_paint, &state) != XGE_OK ) return 6;
	dump_state("stop", &state);

	xgeShapeExSceneDestroy(root);
	xgeShapeExSceneDestroy(nested);
	xgeShapeExSceneDestroy(deep);
	for ( i = 0; i < 4; i++ ) xgeShapeExDestroy(shapes[i]);
	return 0;
}
