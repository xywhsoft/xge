/* Production mesh CPU storage with deterministic allocation failures.
 * No GL loader, graphics device or language binding participates. */
#include "include/xge.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static int allocationBudget=-1;
static unsigned liveBlocks;
static void* meshAllocate(size_t size)
{
    if(allocationBudget==0)return NULL;
    if(allocationBudget>0)--allocationBudget;
    void* result=malloc(size);if(result)++liveBlocks;return result;
}
static void meshFree(void* pointer){if(pointer){assert(liveBlocks);--liveBlocks;free(pointer);}}
#define xrtMalloc meshAllocate
#define xrtFree meshFree
#include "src/xge/render/xge_mesh_cpu.inl"
#undef xrtMalloc
#undef xrtFree
int main(void)
{
    xge_mesh_t mesh={0};xge_vertex_t vertices[3]={{0}};
    uint16_t indices[]={0,1,2},bad[]={0,1,3};vertices[1].fX=10;vertices[2].fY=20;
    assert(__xgeMeshSetCPU(&mesh,vertices,3,indices,3)==XGE_OK);
    assert(liveBlocks==2 && mesh.iVertexCount==3 && mesh.iIndexCount==3);
    void* previousVertices=mesh.pVertices;void* previousIndices=mesh.pIndices;
    assert(__xgeMeshSetCPU(&mesh,vertices,3,bad,3)==XGE_ERROR_INVALID_ARGUMENT);
    assert(mesh.pVertices==previousVertices && mesh.pIndices==previousIndices && liveBlocks==2);
    for(int budget=0;budget<2;++budget){
        allocationBudget=budget;
        assert(__xgeMeshSetCPU(&mesh,vertices,3,indices,3)==XGE_ERROR_OUT_OF_MEMORY);
        assert(mesh.pVertices==previousVertices && mesh.pIndices==previousIndices && liveBlocks==2);
        assert(mesh.iVertexCount==3 && mesh.iIndexCount==3);
        assert(!memcmp(mesh.pVertices,vertices,sizeof(vertices)) && !memcmp(mesh.pIndices,indices,sizeof(indices)));
    }
    allocationBudget=-1;
    assert(__xgeMeshSetCPU(&mesh,mesh.pVertices,mesh.iVertexCount,mesh.pIndices,mesh.iIndexCount)==XGE_OK);
    assert(mesh.pVertices!=previousVertices && mesh.pIndices!=previousIndices && liveBlocks==2);
    assert(!memcmp(mesh.pVertices,vertices,sizeof(vertices)) && !memcmp(mesh.pIndices,indices,sizeof(indices)));
    assert(__xgeMeshSetCPU(&mesh,vertices,0,indices,3)==XGE_ERROR_INVALID_ARGUMENT);
    assert(__xgeMeshSetCPU(&mesh,vertices,INT32_MAX,indices,3)==XGE_ERROR_OUT_OF_MEMORY);
    __xgeMeshFreeCPU(&mesh);assert(!liveBlocks && !mesh.pVertices && !mesh.pIndices);
    return 0;
}
