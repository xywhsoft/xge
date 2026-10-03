#include "../xge.h"
#include <stdio.h>
#include <math.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;static unsigned char pixels[128*128*4];
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_model *model=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;
    xge3d_node_t roots[2],meshes[2],tips[2];xge3d_animator *animators[2]={0};xge3d_clip *clip=NULL;xge3d_mesh *receiver_mesh=NULL;
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK && xge3dModelInfo(model).skin_count==1);
    CHECK(xge3dModelSkinJointNode(model,0,1)==3 && xge3dModelSkinJointNode(model,0,2)==SIZE_MAX);
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(128,128,&target)==XGE_OK);
    CHECK(xge3dRendererMaxJoints(r)>=256);
    const float colors[2][4]={{1,0,0,1},{0,1,0,1}};
    for (int i=0;i<2;++i) {
        CHECK(xge3dModelInstantiate(s,model,(xge3d_node_t){0},&roots[i])==XGE_OK);
        CHECK(xge3dModelInstanceNode(s,roots[i],1,&meshes[i])==XGE_OK && xge3dModelInstanceNode(s,roots[i],3,&tips[i])==XGE_OK);
        CHECK(xge3dNodeSetColor(s,meshes[i],colors[i])==XGE_OK);
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=i ? 2 : -2;
        CHECK(xge3dNodeSetTransform(s,roots[i],&t)==XGE_OK);
        CHECK(xge3dAnimatorCreate(s,roots[i],&animators[i])==XGE_OK);
    }
    xge3dModelFree(model);model=NULL;
    const xge3d_mat4_t *matrices=NULL;size_t count=0;
    CHECK(xge3dNodeSkinMatrices(s,meshes[0],&matrices,&count)==XGE_OK && count==2 && fabsf(matrices[1].m[12]+2)<.0001f);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,8,4,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};xge3d_render_stats_t stats;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.upload_bytes==256);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && pixels[(64*128+64)*4]==0);
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip)==XGE_OK);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.5f;
    CHECK(xge3dAnimatorSetLayer(animators[0],0,clip,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],0)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.upload_bytes==128);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && pixels[(64*128+64)*4]>250);
    CHECK(pixels[(64*128+104)*4+1]>250);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.upload_bytes==0);
    xge3d_ray_t ray={{0,0,3},{0,0,-1}};xge3d_hit_t hit;
    CHECK(xge3dSceneRaycast(s,&ray,10,&hit)==XGE_OK && hit.model_root.slot==roots[0].slot);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p4-skin.png",128,128,pixels,512)==XGE_OK);
    /* The same independently posed palette drives the shadow depth pass. */
    xge3d_vertex_t v[4]={0};
    v[0].position=(xge3d_vec3_t){-4,-2,-1};v[1].position=(xge3d_vec3_t){4,-2,-1};
    v[2].position=(xge3d_vec3_t){4,2,-1};v[3].position=(xge3d_vec3_t){-4,2,-1};
    for (int i=0;i<4;++i) v[i].normal.z=1;
    uint16_t indices[]={0,1,2,0,2,3};xge3d_mesh_desc_t md={v,4,indices,6,16};xge3d_node_t receiver,light;
    CHECK(xge3dMeshCreate(&md,&receiver_mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&receiver)==XGE_OK);
    CHECK(xge3dNodeSetMesh(s,receiver,receiver_mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);
    xge3d_material_desc_t material=xge3dMaterialDefault();material.metallic=0;material.double_sided=1;
    xge3d_material *m=NULL;CHECK(xge3dMaterialCreate(&material,&m)==XGE_OK);
    int attached=xge3dNodeSetMaterial(s,receiver,m);xge3dMaterialFree(m);CHECK(attached==XGE_OK);
    xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);ld.direction=(xge3d_vec3_t){.5f,0,-1};ld.casts_shadow=1;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK && xge3dRender(r,s,&draw,&stats)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);int lit=pixels[(64*128+76)*4];CHECK(lit>100);
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=512;shadows.cascades=1;shadows.distance=10;shadows.depth_padding=5;draw.shadows=&shadows;
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.shadow_draw_calls==3 && stats.upload_bytes==0);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && pixels[(64*128+76)*4]<lit-60);
    layer.time=0;CHECK(xge3dAnimatorSetLayer(animators[0],0,clip,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],0)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.upload_bytes==128);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && abs(pixels[(64*128+76)*4]-lit)<=2);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p4-skin-shadow.png",128,128,pixels,512)==XGE_OK);
    CHECK(xge3dNodeDestroy(s,roots[0])==XGE_OK && xge3dRender(r,s,&draw,&stats)==XGE_OK && stats.upload_bytes==0);
    failed=0;puts("3D skin GPU: inverse binds, mesh-transform cancellation, independent motion/instances, pose-only upload, deformed picking and animated shadow depth passed");
done:
    xge3dAnimatorFree(animators[0]);xge3dAnimatorFree(animators[1]);xge3dClipFree(clip);xge3dMeshFree(receiver_mesh);
    xge3dSceneFree(s);xge3dModelFree(model);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{
    xge_desc_t d={0};d.iWidth=d.iHeight=180;d.sTitle="XGE skin GPU verification";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;
}
