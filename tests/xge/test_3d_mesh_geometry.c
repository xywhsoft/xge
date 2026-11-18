#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#include <assert.h>
#include <stdio.h>

static const xge3d_vertex_t vertices[] = {
    {.position={1,0,0}, .normal={0,0,1}, .uv={0.25f,0.75f}},
    {.position={0,1,0}, .normal={0,0,1}},
    {.position={0,0,1}, .normal={0,0,1}}
};
static const uint16_t indices16[] = {0,1,2};
static const uint32_t indices32[] = {2,1,0};

static int wrong_thread(void *arg)
{
    const xge3d_vertex_t *data = vertices;
    size_t count = 99;
    assert(xge3dMeshGeometryVertices(arg,&data,&count) == XGE_ERROR_INVALID_STATE);
    assert(data == vertices && count == 99);
    assert(xge3dMeshGeometryIndexBits(arg) == XGE_ERROR_INVALID_STATE);
    assert(xge3dMeshGeometryAddRef(arg) == XGE_ERROR_INVALID_STATE);
    assert(xge3dMeshGeometryReleaseChecked(arg) == XGE_ERROR_INVALID_STATE);
    return 0;
}

int main(void)
{
    xge_desc_t desc = {.iWidth=32, .iHeight=32, .sTitle="Mesh CPU geometry"};
    assert(xgeInit(&desc) == XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge3d_mesh *mesh = NULL;
    assert(xge3dMeshCreateOwned(vertices,3,indices16,3,&mesh) == XGE_OK);
    xge3d_mesh_desc_t raw;
    assert(xge3dMeshGetData(mesh,&raw) == XGE_OK);

    xge3d_mesh_geometry *first = NULL, *alias = NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMeshAcquireGeometry(mesh,&first) == XGE_ERROR_OUT_OF_MEMORY && !first);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xge3dMeshAcquireGeometry(mesh,&first) == XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMeshAcquireGeometry(mesh,&alias) == XGE_OK && alias == first);
    assert(xge3dMeshGeometryReleaseChecked(alias) == XGE_OK);
    assert(xge3dMeshUpdateU32(mesh,vertices,3,indices32,3) == XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();

    const xge3d_vertex_t *data;
    const uint16_t *i16;
    const uint32_t *i32 = indices32;
    size_t count;
    assert(xge3dMeshGeometryVertices(first,&data,&count) == XGE_OK && data == raw.vertices && count == 3);
    assert(xge3dMeshGeometryIndicesU16(first,&i16,&count) == XGE_OK && i16 == raw.indices && count == 3);
    assert(xge3dMeshGeometryIndicesU32(first,&i32,&count) == XGE_ERROR_INVALID_ARGUMENT);
    assert(i32 == indices32 && count == 3);

    xthread *worker = xrtThreadCreate(wrong_thread,first,0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);

    xge3d_vertex_t updated[] = {vertices[0],vertices[1],vertices[2]};
    updated[0].position.x = 4;
    assert(xge3dMeshUpdateU32(mesh,updated,3,indices32,3) == XGE_OK);
    xge3d_mesh_geometry *second = NULL;
    assert(xge3dMeshAcquireGeometry(mesh,&second) == XGE_OK && second != first);
    assert(xge3dMeshGeometryIndexBits(first) == 16 && xge3dMeshGeometryIndexBits(second) == 32);
    assert(xge3dMeshGeometryIndicesU32(second,&i32,&count) == XGE_OK && count == 3 && i32[0] == 2);
    assert(xge3dMeshGeometryVertices(second,&data,&count) == XGE_OK && count == 3 && data[0].position.x == 4);

    assert(xge3dMeshUpdateU16(mesh,vertices,3,NULL,0) == XGE_OK);
    xge3d_mesh_geometry *unindexed = NULL;
    assert(xge3dMeshAcquireGeometry(mesh,&unindexed) == XGE_OK);
    assert(xge3dMeshGeometryIndexBits(unindexed) == 0);
    const uint16_t *empty16 = indices16;
    const uint32_t *empty32 = indices32;
    assert(xge3dMeshGeometryIndicesU16(unindexed,&empty16,&count) == XGE_OK && !empty16 && !count);
    assert(xge3dMeshGeometryIndicesU32(unindexed,&empty32,&count) == XGE_OK && !empty32 && !count);
    assert(xge3dMeshGeometryReleaseChecked(unindexed) == XGE_OK);

    assert(xge3dMeshGeometryAddRef(first) == XGE_OK);
    assert(xge3dMeshGeometryReleaseChecked(first) == XGE_OK);
    assert(xge3dMeshReleaseChecked(mesh) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 0 && xgeUnitChecked() == XGE_OK);
    assert(data[0].position.x == 4 && i32[0] == 2);
    assert(xge3dMeshGeometryVertices(first,&data,&count) == XGE_OK && data == raw.vertices && count == 3);
    assert(data[0].position.x == 1 && data[0].uv[1] == 0.75f && i16[2] == 2);
    assert(xge3dMeshGeometryReleaseChecked(first) == XGE_OK);
    assert(xge3dMeshGeometryReleaseChecked(second) == XGE_OK);
    puts("MESH3D_GEOMETRY_NATIVE_OK");
    return 0;
}
