#include "../xge.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
static int failed=1;static unsigned char pixels[160*160*4],baseline[sizeof(pixels)];
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;
    xge3d_mesh *quad=NULL,*triangle=NULL,*plane=NULL;xge3d_material *mask=NULL,*lit=NULL;xge3d_texture *texture=NULL;
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(160,160,&target)==XGE_OK);
    xge3d_vertex_t v[4]={0};const float xy[4][2]={{-1,-1},{1,-1},{1,1},{-1,1}};uint16_t indices[]={0,1,2,0,2,3};
    for (int i=0;i<4;++i) {v[i].position=(xge3d_vec3_t){xy[i][0]*.3f,xy[i][1]*.3f,0};v[i].normal.z=1;v[i].uv[0]=(xy[i][0]+1)*.5f;v[i].uv[1]=(xy[i][1]+1)*.5f;}
    xge3d_mesh_desc_t md={v,4,indices,6,16};CHECK(xge3dMeshCreate(&md,&quad)==XGE_OK);
    md.indices=NULL;md.index_count=0;md.vertex_count=3;md.index_bits=0;CHECK(xge3dMeshCreate(&md,&triangle)==XGE_OK);
    for (int i=0;i<4;++i) {v[i].position.x=xy[i][0]*12;v[i].position.y=xy[i][1]*12;v[i].position.z=-1;}
    md=(xge3d_mesh_desc_t){v,4,indices,6,16};CHECK(xge3dMeshCreate(&md,&plane)==XGE_OK);
    unsigned char rgba[]={255,255,255,255,255,255,255,0,255,255,255,0,255,255,255,255};
    xge_image_t image={2,2,XGE_PIXEL_RGBA8,8,rgba,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t td={&image,1,{9728,9728,33071,33071}};
    CHECK(xge3dTextureCreate(&td,&texture)==XGE_OK);
    xge3d_material_desc_t material=xge3dMaterialDefault();material.unlit=1;material.alpha_mode=XGE3D_ALPHA_MASK;material.maps[0].texture=texture;
    CHECK(xge3dMaterialCreate(&material,&mask)==XGE_OK);material=xge3dMaterialDefault();material.metallic=0;
    CHECK(xge3dMaterialCreate(&material,&lit)==XGE_OK);
    xge3d_node_t nodes[120],receiver,light;const float red[]={1,.2f,.1f,1},blue[]={.1f,.2f,1,1};
    for (int i=0;i<120;++i) {
        CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&nodes[i])==XGE_OK && xge3dNodeSetMesh(s,nodes[i],i<100 ? quad : triangle)==XGE_OK);
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){(i%12)-5.5f,(i/12)-4.5f,0};
        if (i<100 && i%2) t.scale.x=-1;
        CHECK(xge3dNodeSetTransform(s,nodes[i],&t)==XGE_OK && xge3dNodeSetMaterial(s,nodes[i],mask)==XGE_OK && xge3dNodeSetColor(s,nodes[i],i<100 ? red : blue)==XGE_OK);
    }
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&receiver)==XGE_OK && xge3dNodeSetMesh(s,receiver,plane)==XGE_OK && xge3dNodeSetMaterial(s,receiver,lit)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);ld.direction=(xge3d_vec3_t){.3f,0,-1};ld.casts_shadow=1;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,14,12,.1f,20)==XGE_OK && xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,6},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.cascades=1;shadows.distance=20;shadows.depth_padding=10;
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};draw.shadows=&shadows;xge3d_render_stats_t batched,individual;
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK);
    printf("Instances: %u calls, %u shadow calls, %u batches, %llu instances, %llu triangles\n",batched.draw_calls,batched.shadow_draw_calls,batched.instanced_draw_calls,(unsigned long long)batched.instances,(unsigned long long)batched.triangles);
    CHECK(batched.visible_meshes==121 && batched.draw_calls==8 && batched.shadow_draw_calls==4 && batched.instanced_draw_calls==6 && batched.instances==240 && batched.triangles==444);
    CHECK(xge3dTargetReadPixels(target,baseline,sizeof(baseline),640)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK && !batched.upload_bytes);
    draw.disable_instancing=1;CHECK(xge3dRender(r,s,&draw,&individual)==XGE_OK && individual.draw_calls==242 && !individual.instanced_draw_calls && individual.triangles==batched.triangles);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),640)==XGE_OK && !memcmp(pixels,baseline,sizeof(pixels)));
    draw.disable_instancing=0;xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){-5.25f,-4.5f,0};
    CHECK(xge3dNodeSetTransform(s,nodes[0],&t)==XGE_OK && xge3dRender(r,s,&draw,&batched)==XGE_OK && batched.upload_bytes==16384);
    CHECK(xge3dTargetReadPixels(target,baseline,sizeof(baseline),640)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK && !batched.upload_bytes);
    draw.disable_instancing=1;CHECK(xge3dRender(r,s,&draw,&individual)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),640)==XGE_OK && !memcmp(pixels,baseline,sizeof(pixels)));
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p5-instances-mask-shadow.png",160,160,pixels,640)==XGE_OK);
    /* The renderer owns palettes, while meshes own independent VAOs. */
    xge3dRendererFree(r);r=NULL;CHECK(xge3dRendererCreate(&r)==XGE_OK);draw.disable_instancing=0;
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),640)==XGE_OK && !memcmp(pixels,baseline,sizeof(pixels)));
    /* Application LOD uses the same selected mesh in color and depth passes. */
    for (int i=1;i<120;++i) CHECK(xge3dNodeSetVisible(s,nodes[i],0)==XGE_OK);
    xge3d_lod_t level={triangle,5};CHECK(xge3dNodeSetLods(s,nodes[0],&level,1)==XGE_OK);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;CHECK(xge3dNodeSetTransform(s,nodes[0],&t)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK && batched.triangles==6 && batched.shadow_draw_calls==2);
    xge3d_ray_t ray={{-.2f,.2f,3},{0,0,-1}};xge3d_hit_t hit;
    CHECK(xge3dSceneRaycast(s,&ray,10,&hit)==XGE_OK && hit.node.slot==nodes[0].slot); /* Level 0 fills the other half. */
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&batched)==XGE_OK && batched.triangles==8);
    level.min_distance=-1;CHECK(xge3dNodeSetLods(s,nodes[0],&level,1)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeSetLods(s,nodes[0],NULL,0)==XGE_OK && xge3dRender(r,s,&draw,&batched)==XGE_OK && batched.triangles==8);
    failed=0;puts("3D instances GPU: 120 static instances, indexed/nonindexed, mirror groups, mask and sun shadows, 242-to-8 submissions with identical pixels, cached/changed palettes, renderer lifetime and application LOD/picking passed");
done:
    xge3dSceneFree(s);xge3dMeshFree(quad);xge3dMeshFree(triangle);xge3dMeshFree(plane);xge3dMaterialFree(mask);xge3dMaterialFree(lit);xge3dTextureFree(texture);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{xge_desc_t d={0};d.iWidth=d.iHeight=180;d.sTitle="XGE GPU instances and LOD verification";if (xgeInit(&d)!=XGE_OK) return 1;int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;}
