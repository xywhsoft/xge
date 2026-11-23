/* Actual Shape, ShapeScene and SVG code with private refcount boundaries. */
#define XRT_MODULE_ALL
#include "single/xge.c"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

static void checkRetainFailures(xge_shape_ex shape, xge_shape_ex_scene scene)
{
    xge_shape_ex sharedShape = shape;
    xge_shape_ex_scene sharedScene = scene;
    assert(xgeShapeExShareOwned(NULL, &sharedShape) == XGE_ERROR_INVALID_ARGUMENT && !sharedShape);
    assert(xgeShapeExSceneShareOwned(NULL, &sharedScene) == XGE_ERROR_INVALID_ARGUMENT && !sharedScene);
    assert(xgeShapeExShareOwned(shape, NULL) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeShapeExSceneShareOwned(scene, NULL) == XGE_ERROR_INVALID_ARGUMENT);

    const int states[] = { 0, INT_MAX };
    for (size_t index = 0; index < sizeof(states) / sizeof(states[0]); ++index) {
        int count = states[index];
        shape->iRefCount = count;
        scene->iRefCount = count;
        sharedShape = shape;
        sharedScene = scene;
        assert(xgeShapeExAddRef(shape) == XGE_ERROR_INVALID_STATE);
        assert(xgeShapeExSceneAddRef(scene) == XGE_ERROR_INVALID_STATE);
        assert(xgeShapeExShareOwned(shape, &sharedShape) == XGE_ERROR_INVALID_STATE && !sharedShape);
        assert(xgeShapeExSceneShareOwned(scene, &sharedScene) == XGE_ERROR_INVALID_STATE && !sharedScene);
        assert(shape->iRefCount == count && scene->iRefCount == count);
    }

#if XGE_ENABLE_SVG
    xge_svg_def_t definition = {0};
    assert(__xgeSvgDefAddShape(&definition, shape, NULL) == XGE_ERROR_INVALID_STATE);
    assert(!definition.iShapeCount && !definition.iItemCount);
    assert(shape->iRefCount == INT_MAX);
    __xgeSvgDefReset(&definition);
#endif
    shape->iRefCount = 1;
    scene->iRefCount = 1;
}

static void checkHotSharing(xge_shape_ex shape, xge_shape_ex_scene scene)
{
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for (int i = 0; i < 1000; ++i) {
        xge_shape_ex sharedShape = NULL;
        xge_shape_ex_scene sharedScene = NULL;
        assert(xgeShapeExShareOwned(shape, &sharedShape) == XGE_OK && sharedShape == shape);
        assert(xgeShapeExSceneShareOwned(scene, &sharedScene) == XGE_OK && sharedScene == scene);
        xgeShapeExDestroy(sharedShape);
        xgeShapeExSceneDestroy(sharedScene);
    }
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount == after.AllocCount && before.FreeCount == after.FreeCount &&
           before.ReallocCount == after.ReallocCount);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    xge_shape_ex shape = NULL, sharedShape = NULL;
    xge_shape_ex_scene scene = NULL, sharedScene = NULL;
    assert(xgeShapeExCreate(&shape) == XGE_OK && xgeShapeExSceneCreate(&scene) == XGE_OK);
    checkRetainFailures(shape, scene);
    checkHotSharing(shape, scene);

    assert(xgeShapeExId(shape, 17) == XGE_OK && xgeShapeExSceneAdd(scene, shape) == XGE_OK);
    assert(xgeShapeExShareOwned(shape, &sharedShape) == XGE_OK && sharedShape == shape);
    assert(xgeShapeExSceneShareOwned(scene, &sharedScene) == XGE_OK && sharedScene == scene);
    xgeShapeExSceneDestroy(scene);
    xgeShapeExDestroy(shape);
    int count = 0;
    uint32_t id = 0;
    assert(xgeShapeExSceneGetCount(sharedScene, &count) == XGE_OK && count == 1);
    assert(xgeShapeExIdGet(sharedShape, &id) == XGE_OK && id == 17);
    xgeShapeExSceneDestroy(sharedScene);
    assert(xgeShapeExRefCountGet(sharedShape, &count) == XGE_OK && count == 1);
    xgeShapeExDestroy(sharedShape);

    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes &&
           before.InvalidFreeCount == after.InvalidFreeCount && before.DoubleFreeCount == after.DoubleFreeCount);
    puts("Owned Shape/ShapeScene sharing passed");
    return 0;
}
