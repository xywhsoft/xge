#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"%d: %s\n",__LINE__,#x);goto done;}}while(0)
static int failed=1;
static unsigned checks;
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;
    xge3d_model *model=NULL;xge3d_animator *anim=NULL;xge3d_clip *clip=NULL;
    xge3d_node_t red,green;xge3d_camera_t camera;unsigned char pixels[96*96*4];
    CHECK(xge3dSceneCreate(&s)==XGE_OK&&xge3dRendererCreate(&r)==XGE_OK&&xge3dTargetCreate(96,96,&target)==XGE_OK);
    xge3d_vertex_t v[3]={0};v[0].position=(xge3d_vec3_t){-.8f,-.8f,0};v[1].position=(xge3d_vec3_t){.8f,-.8f,0};v[2].position=(xge3d_vec3_t){0,.8f,0};
    for(int i=0;i<3;++i){v[i].normal.z=1;v[i].weights[0]=1;}
    xge3d_mesh_desc_t md={v,3,NULL,0,0};CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
    CHECK(xge3dMeshAddRef(mesh)==XGE_OK);xge3dMeshFree(mesh);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&red)==XGE_OK&&xge3dNodeSetMesh(s,red,mesh)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&green)==XGE_OK&&xge3dNodeSetMesh(s,green,mesh)==XGE_OK);
    float red_color[]={1,0,0,1},green_color[]={0,1,0,1};
    CHECK(xge3dNodeSetColor(s,red,red_color)==XGE_OK&&xge3dNodeSetColor(s,green,green_color)==XGE_OK);
    CHECK(xge3dNodeSetLayers(s,red,1,8,1)==XGE_OK&&xge3dNodeSetLayers(s,green,2,16,2)==XGE_OK);
    CHECK(xge3dCameraPerspective(&camera,1,1,.1f,20)==XGE_OK&&xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={0};draw.camera=&camera;draw.target=target;draw.clear_flags=3;
    xge3d_renderer_options_t options=xge3dRendererOptionsDefault();options.render_layers=1;options.profiling=1;
    CHECK(xge3dRendererSetOptions(r,&options)==XGE_OK&&xge3dRender(r,s,&draw,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),96*4)==XGE_OK);
    size_t center=(48*96+48)*4;CHECK(pixels[center]>240&&pixels[center+1]<5);
    options.render_layers=2;CHECK(xge3dRendererSetOptions(r,&options)==XGE_OK&&xge3dRender(r,s,&draw,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),96*4)==XGE_OK&&pixels[center+1]>240&&pixels[center]<5);
    CHECK(xge3dNodeSetVisible(s,red,0)==XGE_OK);
    xge3d_ray_t ray={{0,0,3},{0,0,-1}};xge3d_hit_t hit;xge3d_query_filter_t filter={8,1};
    CHECK(xge3dSceneRaycastEx(s,&ray,20,&filter,&hit)==XGE_OK&&hit.node.slot==red.slot);
    filter.include_hidden=0;CHECK(xge3dSceneRaycastEx(s,&ray,20,&filter,&hit)==XGE_ERROR_NOT_FOUND);
    CHECK(xge3dSceneRaycast(s,&ray,20,&hit)==XGE_OK&&hit.node.slot==green.slot);
    for(int i=0;i<40;++i){xge3d_node_t light;CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);
        xge3d_light_desc_t d=xge3dLightDefault(XGE3D_LIGHT_POINT);d.intensity=10+i;d.range=5;
        CHECK(xge3dNodeSetLight(s,light,&d)==XGE_OK);}
    options.max_lights=4;CHECK(xge3dRendererSetOptions(r,&options)==XGE_OK);
    xge3d_render_profile_t profile={.size=sizeof(profile)};int sampled=0;
    for(int i=0;i<20;++i){CHECK(xge3dRender(r,s,&draw,NULL)==XGE_OK);
        CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),96*4)==XGE_OK);
        CHECK(xge3dRendererProfile(r,&profile)==XGE_OK&&profile.candidate_lights==40&&profile.selected_lights==4&&profile.skipped_lights==36);
        if(profile.gpu_available){CHECK(profile.gpu_ns>0&&profile.gpu_sample>0);sampled=1;}}
    CHECK(sampled); /* Native desktop acceptance; unsupported backends report 0. */
    xge3d_upload_budget_t upload={UINT64_MAX,0,0};xge3d_upload_stats_t uploaded;
    CHECK(xge3dRendererPrepare(r,64,&upload,&uploaded)==XGE_OK&&!uploaded.completed);
    upload.operations=1;CHECK(xge3dRendererPrepare(r,64,&upload,&uploaded)==XGE_OK&&uploaded.completed&&uploaded.operations==1);
    CHECK(xge3dRendererPrepare(r,64,&upload,&uploaded)==XGE_OK&&uploaded.completed&&uploaded.operations==0);
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK);
    xge3d_node_t root;CHECK(xge3dModelInstantiate(s,model,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dAnimatorCreate(s,root,&anim)==XGE_OK&&xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip)==XGE_OK);
    xge3d_node_t joint;CHECK(xge3dAnimatorBoneNode(anim,"Tip",&joint)==XGE_OK);
    xge3d_skin_binding_t binding={&joint,NULL,1};CHECK(xge3dNodeBindSkin(s,green,&binding)==XGE_OK);
    upload.operations=0;CHECK(xge3dNodePrepareSkin(s,green,&upload,&uploaded)==XGE_OK&&!uploaded.completed);
    upload.operations=1;CHECK(xge3dNodePrepareSkin(s,green,&upload,&uploaded)==XGE_OK&&uploaded.completed&&uploaded.upload_bytes==sizeof(xge3d_mat4_t));
    CHECK(xge3dNodePrepareSkin(s,green,&upload,&uploaded)==XGE_OK&&uploaded.completed&&uploaded.operations==0);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK&&xge3dAnimatorUpdate(anim,0)==XGE_OK);
    xge3d_aabb_t before,after;CHECK(xge3dNodeGetBounds(s,green,0,&before)==XGE_OK);
    CHECK(xge3dAnimatorUpdate(anim,.5f)==XGE_OK&&xge3dNodeGetBounds(s,green,0,&after)==XGE_OK&&fabsf(after.min.x-before.min.x-1)<.0001f);
    CHECK(xge3dNodePrepareSkin(s,green,&upload,&uploaded)==XGE_OK&&uploaded.completed&&uploaded.upload_bytes==sizeof(xge3d_mat4_t));
    CHECK(xge3dRender(r,s,&draw,NULL)==XGE_OK);
    xge3d_resource_info_t info={.size=sizeof(info)};CHECK(xge3dModelResourceInfo(model,&info)==XGE_OK&&info.vertices>0&&info.cpu_payload_bytes>0);
    CHECK(xge3dMeshResourceInfo(mesh,&info)==XGE_OK&&info.gpu_payload_bytes==3*sizeof(xge3d_vertex_t));
    CHECK(xgeImageSavePNG("artifacts/xge-3d/foundation-gpu.png",96,96,pixels,96*4)==XGE_OK);
    xge3d_work_budget_t work={1,0};int completed=0;CHECK(xge3dNodeDestroyStep(s,root,&work,&completed)==XGE_OK);
    while(!completed)CHECK(xge3dNodeDestroyStep(s,root,&work,&completed)==XGE_OK);
    /* A removed shared joint fails explicitly, never dereferences stale data. */
    CHECK(xge3dNodeGetBounds(s,green,0,&after)==XGE_ERROR_INVALID_STATE);
    CHECK(xge3dNodeBindSkin(s,green,NULL)==XGE_OK);
    xge3dAnimatorFree(anim);anim=NULL;
    CHECK(xge3dModelAddRef(model)==XGE_OK);xge3d_model *alias=model;
    xge3d_model_retirement *retirement=NULL;
    CHECK(xge3dModelRetireBegin(&model,&retirement)==XGE_OK&&!model&&!retirement);
    model=alias;CHECK(xge3dModelRetireBegin(&model,&retirement)==XGE_OK&&!model&&retirement);
    xge3d_release_budget_t release={0,0};xge3d_retirement_info_t progress={.size=sizeof(progress)};
    completed=0;CHECK(xge3dModelRetireStep(retirement,&release,&completed)==XGE_OK&&!completed);
    release.operations=1;size_t prior=0;
    while(!completed){CHECK(xge3dModelRetireStep(retirement,&release,&completed)==XGE_OK);
        CHECK(xge3dModelRetireInfo(retirement,&progress)==XGE_OK&&progress.operations-prior<=1);prior=progress.operations;}
    CHECK(xge3dModelRetireFree(retirement)==XGE_OK);
    failed=0;printf("Foundation GPU: %u checks passed; GPU sample %llu ns\n",checks,(unsigned long long)profile.gpu_ns);
done:
    xge3dClipFree(clip);xge3dAnimatorFree(anim);xge3dModelFree(model);xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{
    xge_desc_t d={0};d.iWidth=d.iHeight=160;d.sTitle="Foundation GPU verification";
    if(xgeInit(&d)!=XGE_OK){return 1;}int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK?failed:1;
}
