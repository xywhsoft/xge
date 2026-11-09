/* Allocate and validate before publishing: Update may alias its own current
 * storage, and failed allocation must leave a drawable previous mesh intact. */
static void __xgeMeshFreeCPU(xge_mesh mesh)
{
    xrtFree(mesh->pVertices);mesh->pVertices=NULL;
    xrtFree(mesh->pIndices);mesh->pIndices=NULL;
}
static int __xgeMeshSetCPU(xge_mesh mesh,const xge_vertex_t* vertices,int vertexCount,
    const uint16_t* indices,int indexCount)
{
    if(!mesh || !vertices || vertexCount<=0 || !indices || indexCount<=0)return XGE_ERROR_INVALID_ARGUMENT;
    if(vertexCount>INT32_MAX/(int)sizeof(*vertices) || indexCount>INT32_MAX/(int)sizeof(*indices))return XGE_ERROR_OUT_OF_MEMORY;
    for(int i=0;i<indexCount;++i)if(indices[i]>=vertexCount)return XGE_ERROR_INVALID_ARGUMENT;
    size_t vertexBytes=(size_t)vertexCount*sizeof(*vertices),indexBytes=(size_t)indexCount*sizeof(*indices);
    void* newVertices=xrtMalloc(vertexBytes);
    if(!newVertices)return XGE_ERROR_OUT_OF_MEMORY;
    void* newIndices=xrtMalloc(indexBytes);
    if(!newIndices){xrtFree(newVertices);return XGE_ERROR_OUT_OF_MEMORY;}
    memcpy(newVertices,vertices,vertexBytes);memcpy(newIndices,indices,indexBytes);
    __xgeMeshFreeCPU(mesh);
    mesh->pVertices=newVertices;mesh->pIndices=newIndices;
    mesh->iVertexCount=vertexCount;mesh->iIndexCount=indexCount;
    return XGE_OK;
}
