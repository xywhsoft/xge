/* Identical legacy-API workload for before/after DLL comparisons. CPU only. */
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    enum{N=6000};xge_desc_t d={0};d.iRunMode=XGE_RUN_MANUAL;
    if(xgeInit(&d)!=XGE_OK)return 1;
    xge3d_scene *s=NULL;xge3d_mesh *mesh=NULL;xge3d_node_t root,moving;
    xge3d_vertex_t vertices[3]={0};vertices[1].position.x=1;vertices[2].position.y=1;
    xge3d_mesh_desc_t md={vertices,3,NULL,0,0};
    if(xge3dSceneCreate(&s)||xge3dMeshCreate(&md,&mesh)||xge3dNodeCreate(s,(xge3d_node_t){0},&root))return 1;
    xge3d_node_t *nodes=calloc(N,sizeof(*nodes));if(!nodes)return 1;
    for(int i=0;i<N;++i){xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){i%100*3,i/100*3,0};
        if(xge3dNodeCreate(s,root,&nodes[i])||xge3dNodeSetMesh(s,nodes[i],mesh)||xge3dNodeSetTransform(s,nodes[i],&t))return 1;}
    if(xge3dNodeCreate(s,(xge3d_node_t){0},&moving))return 1;
    xge3d_aabb_t box;for(int i=0;i<N;++i)if(xge3dNodeGetBounds(s,nodes[i],0,&box))return 1;
    double started=xrtTimer();
    for(int frame=0;frame<25;++frame){xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=(float)frame;
        if(xge3dNodeSetTransform(s,moving,&t))return 1;
        for(int i=0;i<N;++i)if(xge3dNodeGetBounds(s,nodes[i],0,&box))return 1;}
    double updates=(xrtTimer()-started)*1000;size_t count=0;
    box=(xge3d_aabb_t){{0,0,-1},{10,10,1}};
    if(xge3dSceneQueryAabb(s,&box,NULL,0,&count))return 1;
    started=xrtTimer();size_t sum=0;
    for(int i=0;i<1500;++i){float x=(float)(i%99)*3,y=(float)(i%59)*3;
        box=(xge3d_aabb_t){{x,y,-1},{x+5,y+5,1}};
        if(xge3dSceneQueryAabb(s,&box,NULL,0,&count)){return 1;}sum+=count;}
    double queries=(xrtTimer()-started)*1000;started=xrtTimer();
    if(xge3dNodeDestroy(s,root))return 1;
    double destroy=(xrtTimer()-started)*1000;
    printf("{\"nodes\":%d,\"update_ms\":%.6f,\"query_ms\":%.6f,\"destroy_ms\":%.6f,\"query_hits\":%zu}\n",N,updates,queries,destroy,sum);
    free(nodes);xge3dMeshFree(mesh);xge3dSceneFree(s);xgeUnit();return 0;
}
