#include "../xge.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
static int failed=1;static unsigned char baseline[128*128*4],pixels[sizeof(baseline)];
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *plane=NULL,*quad=NULL;
    xge3d_model *model=NULL;xge3d_animator *animator=NULL;xge3d_clip *clip=NULL;
    xge3d_dvec3_t origin={1e9,2e9,-3e9};xge3d_node_t receiver,caster,light,root,skin;
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dSceneSetOrigin(s,origin)==XGE_OK);
    CHECK(xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(128,128,&target)==XGE_OK);
    xge3d_vertex_t vertices[4]={0};uint16_t indices[]={0,1,2,0,2,3};
    const float xy[4][2]={{-1,-1},{1,-1},{1,1},{-1,1}};
    for (int i=0;i<4;++i) {vertices[i].position=(xge3d_vec3_t){xy[i][0]*2,xy[i][1]*2,0};vertices[i].normal.z=1;}
    xge3d_mesh_desc_t md={vertices,4,indices,6,16};CHECK(xge3dMeshCreate(&md,&plane)==XGE_OK);
    for (int i=0;i<4;++i) {vertices[i].position.x*=.2f;vertices[i].position.y*=.2f;}
    CHECK(xge3dMeshCreate(&md,&quad)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&receiver)==XGE_OK && xge3dNodeSetMesh(s,receiver,plane)==XGE_OK);
    xge3d_material_desc_t material=xge3dMaterialDefault();material.metallic=0;material.double_sided=1;xge3d_material *m=NULL;
    CHECK(xge3dMaterialCreate(&material,&m)==XGE_OK);int attached=xge3dNodeSetMaterial(s,receiver,m);xge3dMaterialFree(m);CHECK(attached==XGE_OK);
    for (int i=0;i<81;++i) {
        xge3d_node_t n;CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&n)==XGE_OK && xge3dNodeSetMesh(s,n,quad)==XGE_OK);
        xge3d_dvec3_t position={origin.x+(i ? 100+i*2 : 3),origin.y,origin.z+2};
        CHECK(xge3dNodeSetGlobalPosition(s,n,position)==XGE_OK);if (!i) caster=n;
    }
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);
    xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);ld.direction=(xge3d_vec3_t){-1.5f,0,-1};ld.casts_shadow=1;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,4,4,.1f,10)==XGE_OK);
    xge3d_dvec3_t eye={origin.x,origin.y,origin.z+5};
    CHECK(xge3dCameraLookAtGlobal(&camera,origin,eye,origin,(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};xge3d_render_stats_t stats,unculled;
    draw.disable_instancing=1; /* Isolate the culling comparison. */
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.visible_meshes==1 && stats.culled_meshes==81 && stats.draw_calls==1);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);int lit=pixels[(64*128+64)*4];CHECK(lit>70);
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=512;shadows.cascades=1;shadows.distance=10;shadows.depth_padding=10;draw.shadows=&shadows;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.visible_meshes==1 && stats.shadow_draw_calls==2);
    CHECK(xge3dTargetReadPixels(target,baseline,sizeof(baseline),512)==XGE_OK && baseline[(64*128+64)*4]<lit-50);
    draw.disable_culling=1;CHECK(xge3dRender(r,s,&draw,&unculled)==XGE_OK && unculled.visible_meshes==82 && !unculled.culled_meshes);
    CHECK(unculled.draw_calls>stats.draw_calls*20 && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && !memcmp(baseline,pixels,sizeof(pixels)));
    draw.disable_culling=0;
    xge3d_ray_t ray={{0,0,4},{0,0,-1}};xge3d_hit_t hit;CHECK(xge3dSceneRaycast(s,&ray,10,&hit)==XGE_OK && hit.node.slot==receiver.slot && hit.distance==4);
    xge3d_dvec3_t shifted={origin.x+32,origin.y-16,origin.z+8};
    CHECK(xge3dSceneSetOrigin(s,shifted)==XGE_OK && xge3dCameraLookAtGlobal(&camera,shifted,eye,origin,(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.visible_meshes==1 && stats.shadow_draw_calls==2 && stats.upload_bytes==0);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    int different=0;for (size_t i=0;i<sizeof(pixels);++i) if (abs((int)pixels[i]-baseline[i])>2) ++different;
    printf("Rebase: %d differing channel bytes\n",different);CHECK(different<128*4);
    ray.origin=(xge3d_vec3_t){-32,16,-4};CHECK(xge3dSceneRaycast(s,&ray,10,&hit)==XGE_OK && hit.node.slot==receiver.slot && hit.distance==4);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p5-culling-shadow-origin.png",128,128,pixels,512)==XGE_OK);
    CHECK(xge3dNodeSetVisible(s,caster,0)==XGE_OK && xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.shadow_draw_calls==1);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && abs(pixels[(64*128+64)*4]-lit)<3);
    /* Bind pose is outside the camera; an animated joint brings geometry in. */
    draw.shadows=NULL;CHECK(xge3dNodeSetVisible(s,receiver,0)==XGE_OK && xge3dSceneSetOrigin(s,origin)==XGE_OK);
    CHECK(xge3dCameraLookAtGlobal(&camera,origin,eye,origin,(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK && xge3dModelInstantiate(s,model,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dModelInstanceNode(s,root,1,&skin)==XGE_OK && xge3dNodeSetGlobalPosition(s,root,(xge3d_dvec3_t){origin.x-4,origin.y,origin.z})==XGE_OK);
    CHECK(xge3dAnimatorCreate(s,root,&animator)==XGE_OK && xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.visible_meshes==0 && stats.draw_calls==0);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.5f;
    CHECK(xge3dAnimatorSetLayer(animator,0,clip,&layer)==XGE_OK && xge3dAnimatorUpdate(animator,0)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.visible_meshes==1 && stats.draw_calls==1);
    CHECK(xge3dTargetReadPixels(target,baseline,sizeof(baseline),512)==XGE_OK);
    draw.disable_culling=1;CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && !memcmp(pixels,baseline,sizeof(pixels)));
    failed=0;printf("3D culling GPU: 82 meshes, off-camera shadow caster, rebased shadow/ray agreement (%d edge bytes), animated envelope visibility and identical disabled-culling pixels passed\n",different);
done:
    xge3dAnimatorFree(animator);xge3dClipFree(clip);xge3dSceneFree(s);xge3dModelFree(model);xge3dMeshFree(plane);xge3dMeshFree(quad);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{xge_desc_t d={0};d.iWidth=d.iHeight=160;d.sTitle="XGE culling / origin verification";if (xgeInit(&d)!=XGE_OK) return 1;int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;}
