#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
static int failed=1;static unsigned char pixels[192*192*4],baseline[sizeof(pixels)];
static xge3d_vec3_t transform(xge3d_mat4_t m,xge3d_vec3_t p,float *w)
{*w=m.m[3]*p.x+m.m[7]*p.y+m.m[11]*p.z+m.m[15];return (xge3d_vec3_t){m.m[0]*p.x+m.m[4]*p.y+m.m[8]*p.z+m.m[12],m.m[1]*p.x+m.m[5]*p.y+m.m[9]*p.z+m.m[13],m.m[2]*p.x+m.m[6]*p.y+m.m[10]*p.z+m.m[14]};}
static int brightness(xge3d_camera_t camera,xge3d_vec3_t point)
{
    float w;xge3d_vec3_t p=transform(camera.view,point,&w);p=transform(camera.projection,p,&w);
    int x=(int)((p.x/w*.5f+.5f)*192),y=(int)((.5f-p.y/w*.5f)*192);
    if (x<0 || y<0 || x>=192 || y>=192) return -1;
    return pixels[(y*192+x)*4];
}
static int frame(void *user)
{
    (void)user;xge3d_terrain *terrain=NULL;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *caster_mesh=NULL;
    float heights[17][17];for (int z=0;z<17;++z) for (int x=0;x<17;++x) heights[z][x]=2+1.5f*sinf(x*.8f)*cosf(z*.7f);
    xge3d_terrain_desc_t desc=xge3dTerrainDefault();desc.heights=heights;desc.width=desc.depth=17;desc.chunk_cells=8;
    CHECK(xge3dTerrainCreate(&desc,&terrain)==XGE_OK && xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(192,192,&target)==XGE_OK);
    xge3d_node_t root;CHECK(xge3dTerrainInstantiate(s,terrain,(xge3d_node_t){0},&root)==XGE_OK);
    const int selected[]={0,1,2,1};
    for (int i=0;i<4;++i) {
        xge3d_node_t node=xge3dSceneNodeAt(s,i+1);xge3d_lod_t levels[2];
        for (int k=1;k<=selected[i];++k) {const xge3d_mesh *m;xge3d_vec3_t position;CHECK(xge3dTerrainChunkMesh(terrain,i,k,&m,&position)==XGE_OK);levels[k-1]=(xge3d_lod_t){(xge3d_mesh*)m,(float)k};}
        CHECK(xge3dNodeSetLods(s,node,levels,selected[i])==XGE_OK);
    }
    xge3d_vertex_t v[4]={0};const float xz[4][2]={{-1,-1},{-1,1},{1,1},{1,-1}};
    for (int i=0;i<4;++i) {v[i].position=(xge3d_vec3_t){8+xz[i][0],10,8+xz[i][1]};v[i].normal.y=1;}
    uint16_t indices[]={0,1,2,0,2,3};xge3d_mesh_desc_t md={v,4,indices,6,16};xge3d_node_t caster,light;
    CHECK(xge3dMeshCreate(&md,&caster_mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&caster)==XGE_OK && xge3dNodeSetMesh(s,caster,caster_mesh)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);ld.direction=(xge3d_vec3_t){.4f,-1,0};ld.casts_shadow=1;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,26,26,.1f,60)==XGE_OK && xge3dCameraLookAt(&camera,(xge3d_vec3_t){22,18,20},(xge3d_vec3_t){8,0,8},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{1,0,1,1},3};xge3d_render_stats_t stats;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.draw_calls==5 && stats.triangles==346);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK);
    xge3d_vec3_t point={11,2,8},normal;for (int i=0;i<8;++i) {CHECK(xge3dTerrainSample(terrain,1,point.x,point.z,&point.y,&normal)==XGE_OK);point.x=8+.4f*(10-point.y);}
    int lit=brightness(camera,point);CHECK(lit>70);
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.cascades=1;shadows.resolution=1024;shadows.distance=50;shadows.depth_padding=20;draw.shadows=&shadows;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.shadow_maps==1 && stats.shadow_draw_calls==5 && stats.triangles==692);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK);int shadowed=brightness(camera,point);printf("Terrain sun: %d -> %d, %llu triangles\n",lit,shadowed,(unsigned long long)stats.triangles);CHECK(shadowed<lit-50);
    /* Interior screen rays must find color even across mixed-level seams. */
    size_t interior=0,holes=0;
    for (int y=0;y<192;++y) for (int x=0;x<192;++x) {
        xge3d_ray_t ray;xge3d_hit_t hit;CHECK(xge3dCameraScreenRay(&camera,x+.5f,y+.5f,192,192,&ray)==XGE_OK);
        if (xge3dSceneRaycast(s,&ray,100,&hit)!=XGE_OK || hit.node.slot==caster.slot || hit.position.x<1 || hit.position.x>15 || hit.position.z<1 || hit.position.z>15) continue;
        ++interior;const unsigned char *p=pixels+(y*192+x)*4;if (p[0]>250 && p[1]<5 && p[2]>250) ++holes;
    }
    CHECK(interior>3000 && !holes);CHECK(xgeImageSavePNG("artifacts/xge-3d/p5-terrain-lod-shadow.png",192,192,pixels,768)==XGE_OK);
    memcpy(baseline,pixels,sizeof(pixels));xge3dTerrainFree(terrain);terrain=NULL;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && !stats.upload_bytes && xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK && !memcmp(pixels,baseline,sizeof(pixels)));
    CHECK(xge3dNodeSetVisible(s,caster,0)==XGE_OK && xge3dRender(r,s,&draw,&stats)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK && brightness(camera,point)>lit-3);
    /* The same chunks receive a perspective spot projection. */
    ld=xge3dLightDefault(XGE3D_LIGHT_SPOT);ld.intensity=1000;ld.range=30;ld.inner_angle=.6f;ld.outer_angle=.8f;ld.direction=(xge3d_vec3_t){0,-1,0};ld.casts_shadow=1;
    xge3d_transform_t transform_light=XGE3D_TRANSFORM_IDENTITY;transform_light.position=(xge3d_vec3_t){8,16,8};
    CHECK(xge3dNodeSetTransform(s,light,&transform_light)==XGE_OK && xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    point=(xge3d_vec3_t){8,heights[8][8],8};CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK);lit=brightness(camera,point);CHECK(lit>70);
    CHECK(xge3dNodeSetVisible(s,caster,1)==XGE_OK && xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.shadow_maps==1 && stats.shadow_draw_calls==5 && xge3dTargetReadPixels(target,pixels,sizeof(pixels),768)==XGE_OK);
    shadowed=brightness(camera,point);printf("Terrain spot: %d -> %d\n",lit,shadowed);CHECK(shadowed<lit-50);
    failed=0;printf("3D terrain GPU: mixed 0/1/2 mesh levels, %zu interior pixels without seam holes, real sun/spot occlusion, same LOD in depth/color and retained/cached resources passed\n",interior);
done:
    xge3dTerrainFree(terrain);xge3dSceneFree(s);xge3dMeshFree(caster_mesh);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{xge_desc_t d={0};d.iWidth=d.iHeight=210;d.sTitle="XGE heightfield LOD and shadow verification";if (xgeInit(&d)!=XGE_OK) return 1;int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;}
