#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct thread_test {
    xge3d_scene *scene;
    xge3d_node_t node;
    xge3d_skin_palette *palette;
} thread_test;

static int check_wrong_thread(void *raw)
{
    thread_test *test = raw;
    const xge3d_mat4_t *data = NULL;
    size_t count = 3;
    xge3d_skin_palette *discard = test->palette;
    assert(xge3dNodeAcquireSkinPalette(test->scene,test->node,&discard) == XGE_ERROR_BUSY && !discard);
    assert(xge3dSkinPaletteMatrices(test->palette,&data,&count) == XGE_ERROR_INVALID_STATE);
    assert(!data && !count);
    assert(xge3dSkinPaletteAddRef(test->palette) == XGE_ERROR_INVALID_STATE);
    assert(xge3dSkinPaletteReleaseChecked(test->palette) == XGE_ERROR_INVALID_STATE);
    return 0;
}

int main(void)
{
    const xge_desc_t desc = {.iWidth=32, .iHeight=32, .sTitle="Skin palette snapshots"};
    const xge3d_vertex_t triangle[] = {
        {.position={0,0,0}, .weights={1}},
        {.position={1,0,0}, .weights={1}},
        {.position={0,1,0}, .joints={1}, .weights={1}},
    };
    xge3d_mat4_t moved = {{1,0,0,0, 0,1,0,0, 0,0,1,0, 3,0,0,1}};
    xge3d_scene *scene = NULL;
    xge3d_mesh *mesh = NULL;
    xge3d_node_t body, joints[2];
    xge3d_skin_palette *before = NULL, *after = NULL;
    const xge3d_mat4_t *old_data, *new_data, *raw_data;
    size_t count;
    assert(xgeInit(&desc) == XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xge3dSceneCreateOwned(&scene) == XGE_OK);
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&body) == XGE_OK);
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&joints[0]) == XGE_OK);
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&joints[1]) == XGE_OK);
    assert(xge3dNodeAcquireSkinPalette(scene,body,&before) == XGE_ERROR_NOT_FOUND && !before);
    assert(xge3dMeshCreateOwned(triangle,3,NULL,0,&mesh) == XGE_OK);
    assert(xge3dNodeSetMesh(scene,body,mesh) == XGE_OK);
    assert(xge3dMeshReleaseChecked(mesh) == XGE_OK);
    assert(xge3dNodeBindSkinArrays(scene,body,joints,2,NULL,0) == XGE_OK);
    assert(xge3dNodeAcquireSkinPalette(scene,body,&before) == XGE_OK);
    assert(xge3dSkinPaletteMatrices(before,&old_data,&count) == XGE_OK && count == 2);
    assert(xge3dNodeSkinMatrices(scene,body,&raw_data,&count) == XGE_OK && count == 2);
    assert(old_data != raw_data && !memcmp(old_data,raw_data,2*sizeof(*old_data)));

    thread_test test = {scene,body,before};
    xthread *thread = xrtThreadCreate(check_wrong_thread,&test,0);
    assert(thread && xrtThreadWait(thread) == XWAIT_OK);
    xrtThreadDestroy(thread);

    assert(xge3dNodeSetMatrix(scene,joints[1],&moved) == XGE_OK);
    xmemdebugsnapshot start, end;
    xrtMemDebugSnapshot(&start);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dNodeAcquireSkinPalette(scene,body,&after) == XGE_ERROR_OUT_OF_MEMORY && !after);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xrtMemDebugSnapshot(&end);
    assert(start.LiveCount == end.LiveCount && start.LiveBytes == end.LiveBytes);
    assert(xge3dNodeAcquireSkinPalette(scene,body,&after) == XGE_OK);
    assert(xge3dSkinPaletteMatrices(after,&new_data,&count) == XGE_OK && count == 2);
    assert(new_data[0].m[12] == 0 && new_data[1].m[12] == 3);
    assert(old_data[1].m[12] == 0);
    assert(xge3dSkinPaletteAddRef(after) == XGE_OK);
    assert(xge3dSkinPaletteReleaseChecked(after) == XGE_OK);
    assert(xge3dNodeDestroy(scene,body) == XGE_OK);
    xge3d_skin_palette *stale = before;
    assert(xge3dNodeAcquireSkinPalette(scene,body,&stale) == XGE_ERROR_INVALID_ARGUMENT && !stale);
    assert(xge3dSceneReleaseChecked(scene) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 0 && xgeUnitChecked() == XGE_OK);
    assert(old_data[1].m[12] == 0 && new_data[1].m[12] == 3);
    assert(xge3dSkinPaletteReleaseChecked(before) == XGE_OK);
    assert(xge3dSkinPaletteReleaseChecked(after) == XGE_OK);
    puts("SKIN_PALETTE3D_NATIVE_OK");
    return 0;
}
