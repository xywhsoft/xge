#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xge.h"
#include "xrt.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

typedef struct traversal_test {
	xge_shape_ex_scene root;
	xge_shape_ex_scene inner;
	xge_shape_ex_scene unrelated;
	xge_shape_ex shape;
	xge_shape_ex extra;
	int visits;
} traversal_test;

static int stop(const xge_shape_ex_scene_child_t* node, void* user)
{
	assert(node->iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE);
	++*(int*)user;
	return 0;
}

static int visit(const xge_shape_ex_scene_child_t* node, void* user)
{
	traversal_test* t = user;
	++t->visits;
	if (t->visits == 1) {
		assert(node->pScene == t->root && !node->pShape);
		assert(xgeShapeExSceneClear(t->root) == XGE_ERROR_BUSY);
		assert(xgeShapeExSceneClear(t->inner) == XGE_ERROR_BUSY);
		assert(xgeShapeExSceneRemoveScene(t->root, t->inner) == XGE_ERROR_BUSY);
		assert(xgeShapeExSceneRemove(t->inner, t->shape) == XGE_ERROR_BUSY);
		assert(xgeShapeExSceneAdd(t->inner, t->extra) == XGE_ERROR_BUSY);
		assert(xgeShapeExSceneAdd(t->unrelated, t->extra) == XGE_OK);
		assert(xgeShapeExSceneRemove(t->unrelated, t->extra) == XGE_OK);
		int nested = 0;
		assert(xgeShapeExSceneTraverse(t->inner, stop, &nested) == XGE_OK && nested == 1);
		assert(xgeShapeExSceneClear(t->inner) == XGE_ERROR_BUSY);
	} else if (t->visits == 2) {
		assert(node->pScene == t->inner && !node->pShape);
	} else {
		assert(t->visits == 3 && node->pShape == t->shape && !node->pScene);
	}
	return 1;
}

static int count(const xge_shape_ex_scene_child_t* node, void* user)
{
	assert(node->iType == XGE_SHAPE_EX_SCENE_CHILD_SCENE);
	++*(int*)user;
	return 1;
}

static void check_depth(void)
{
	enum { DEEP_SCENES = 40 };
	xge_shape_ex_scene scenes[DEEP_SCENES] = {0};
	for (int i = 0; i < DEEP_SCENES; ++i) {
		assert(xgeShapeExSceneCreate(&scenes[i]) == XGE_OK);
		if (i) assert(xgeShapeExSceneAddScene(scenes[i - 1], scenes[i]) == XGE_OK);
	}
	int visits = 0;
	assert(xgeShapeExSceneTraverse(scenes[0], count, &visits) == XGE_ERROR_INVALID_ARGUMENT);
	assert(visits == 33);
	for (int i = DEEP_SCENES - 1; i >= 0; --i) {
		assert(xgeShapeExSceneClear(scenes[i]) == XGE_OK);
		xgeShapeExSceneDestroy(scenes[i]);
	}
}

int main(void)
{
	traversal_test t = {0};
	assert(xgeShapeExSceneCreate(&t.root) == XGE_OK);
	assert(xgeShapeExSceneCreate(&t.inner) == XGE_OK);
	assert(xgeShapeExSceneCreate(&t.unrelated) == XGE_OK);
	assert(xgeShapeExCreate(&t.shape) == XGE_OK);
	assert(xgeShapeExCreate(&t.extra) == XGE_OK);
	assert(xgeShapeExSceneAdd(t.inner, t.shape) == XGE_OK);
	assert(xgeShapeExSceneAddScene(t.root, t.inner) == XGE_OK);
	assert(xgeShapeExSceneTraverse(t.root, visit, &t) == XGE_OK && t.visits == 3);
	assert(xgeShapeExSceneRemove(t.inner, t.shape) == XGE_OK);
	assert(xgeShapeExSceneRemoveScene(t.root, t.inner) == XGE_OK);
	assert(xgeShapeExSceneAdd(t.root, t.extra) == XGE_OK);
	xgeShapeExDestroy(t.shape);
	xgeShapeExDestroy(t.extra);
	xgeShapeExSceneDestroy(t.inner);
	xgeShapeExSceneDestroy(t.unrelated);
	xgeShapeExSceneDestroy(t.root);
	check_depth();
	puts("XGE_SHAPE_TRAVERSAL_NATIVE_OK");
	return 0;
}
