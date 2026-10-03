#include "../xge.h"
#include <math.h>
#include <stdio.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
#define NEAR(a,b) (fabsf((a)-(b))<.001f)
int main(void)
{
    /* Padded uint16 rows, values too close to survive an 8-bit conversion. */
    uint16_t heights[10][20];for (int z=0;z<10;++z) for (int x=0;x<20;++x) heights[z][x]=(uint16_t)(50000+x+z*3);
    xge3d_terrain_desc_t d=xge3dTerrainDefault();d.heights=heights;d.width=17;d.depth=10;d.stride=sizeof(heights[0]);d.format=XGE3D_HEIGHT_U16;d.height_scale=.001f;d.height_offset=-50;d.chunk_cells=4;
    xge3d_terrain *t=NULL;CHECK(xge3dTerrainCreate(&d,&t)==XGE_OK);
    xge3d_terrain_info_t info=xge3dTerrainInfo(t);CHECK(info.chunk_count==12 && info.width==17 && info.depth==10 && info.lod_count==3);
    float height;xge3d_vec3_t normal;
    CHECK(xge3dTerrainSample(t,0,1,0,&height,&normal)==XGE_OK && height>.0009f && height<.0011f);
    heights[0][1]=0;CHECK(xge3dTerrainSample(t,0,1,0,&height,&normal)==XGE_OK && height>.0009f);
    for (int lod=0;lod<3;++lod) for (int z=0;z<=36;++z) for (int x=0;x<=64;++x) {
        float px=x*.25f,pz=z*.25f;CHECK(xge3dTerrainSample(t,lod,px,pz,&height,&normal)==XGE_OK && NEAR(height,px*.001f+pz*.003f) && normal.y>.99f);
    }
    CHECK(xge3dTerrainSample(t,0,16.01f,0,&height,&normal)==XGE_ERROR_NOT_FOUND && height==0);
    CHECK(xge3dTerrainSample(t,-1,0,0,&height,&normal)==XGE_ERROR_INVALID_ARGUMENT);
    xge3d_scene *s=NULL;xge3d_node_t root;CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dTerrainInstantiate(s,t,(xge3d_node_t){0},&root)==XGE_OK && xge3dSceneNodeCount(s)==13);
    xge3d_node_t chunk_node=xge3dSceneNodeAt(s,1);const xge3d_mesh *borrow;xge3d_vec3_t position;
    CHECK(xge3dTerrainChunkMesh(t,0,0,&borrow,&position)==XGE_OK && position.x==2 && position.z==2);
    xge3d_mesh_desc_t mesh;CHECK(xge3dMeshGetData(borrow,&mesh)==XGE_OK && mesh.vertex_count==41 && mesh.index_count==192);
    CHECK(xge3dTerrainChunkMesh(t,0,2,&borrow,&position)==XGE_OK && xge3dMeshGetData(borrow,&mesh)==XGE_OK && mesh.vertex_count==8 && mesh.index_count==30);
    CHECK(xge3dTerrainChunkMesh(t,12,0,&borrow,&position)==XGE_ERROR_INVALID_ARGUMENT && !borrow);
    xge3dTerrainFree(t);t=NULL;
    CHECK(xge3dNodeGetMesh(s,chunk_node,&borrow)==XGE_OK && xge3dMeshGetData(borrow,&mesh)==XGE_OK);
    xge3d_ray_t ray={{1,.5f,1},{0,-3,0}};xge3d_hit_t hit;
    CHECK(xge3dSceneRaycast(s,&ray,1,&hit)==XGE_OK && NEAR(hit.position.y,.004f));
    CHECK(xge3dNodeDestroy(s,root)==XGE_OK && !xge3dSceneNodeCount(s));xge3dSceneFree(s);

    /* Non-planar cell: diagonal interpolation differs from bilinear height. */
    float floats[7][10];for (int z=0;z<7;++z) for (int x=0;x<10;++x) floats[z][x]=x*z*.3f;
    d=xge3dTerrainDefault();d.heights=floats;d.width=10;d.depth=7;d.chunk_cells=4;d.skirt_depth=20;CHECK(xge3dTerrainCreate(&d,&t)==XGE_OK);
    CHECK(xge3dTerrainSample(t,0,.5f,.5f,&height,&normal)==XGE_OK && NEAR(height,.15f)); /* Bilinear would be .075. */
    CHECK(xge3dTerrainSample(t,1,.5f,.5f,&height,&normal)==XGE_OK && NEAR(height,.3f));
    for (int lod=0;lod<3;++lod) for (int z=0;z<6;++z) for (int x=0;x<9;++x) {
        float px=x+.27f,pz=z+.64f;CHECK(xge3dTerrainSample(t,lod,px,pz,&height,&normal)==XGE_OK);
        ray=(xge3d_ray_t){{px,100,pz},{0,-7,0}};size_t chunk;
        CHECK(xge3dTerrainRaycast(t,lod,&ray,110,&hit,&chunk)==XGE_OK && NEAR(hit.position.y,height) && NEAR(hit.distance,100-height) && hit.normal.y>0 && chunk<6);
        CHECK(fabsf(hit.normal.x-normal.x)<.0001f && fabsf(hit.normal.z-normal.z)<.0001f);
    }
    ray=(xge3d_ray_t){{-1,-1,2},{1,0,0}};size_t chunk;
    CHECK(xge3dTerrainRaycast(t,0,&ray,10,&hit,&chunk)==XGE_OK && NEAR(hit.position.x,0) && hit.normal.x<-.99f);
    ray.origin=(xge3d_vec3_t){-1,30,2};CHECK(xge3dTerrainRaycast(t,0,&ray,10,&hit,&chunk)==XGE_ERROR_NOT_FOUND && chunk==SIZE_MAX);
    xge3dTerrainFree(t);t=NULL;
    floats[0][0]=NAN;CHECK(xge3dTerrainCreate(&d,&t)==XGE_ERROR_INVALID_ARGUMENT && !t);floats[0][0]=0;
    d.stride=1;CHECK(xge3dTerrainCreate(&d,&t)==XGE_ERROR_INVALID_ARGUMENT);d.stride=0;
    d.chunk_cells=5;CHECK(xge3dTerrainCreate(&d,&t)==XGE_ERROR_INVALID_ARGUMENT);d.chunk_cells=4;
    d.lod_distances[1]=1;CHECK(xge3dTerrainCreate(&d,&t)==XGE_ERROR_INVALID_ARGUMENT);
    puts("3D terrain CPU: copied padded uint16 precision/float data, partial chunks/three mesh levels, retained scene lifetime, actual diagonal height/normal and surface/skirt ray agreement, invalid inputs passed");return 0;
}
