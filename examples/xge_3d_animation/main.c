#include "xge.h"
#include "lib/xrt/xrt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    xge3d_animator *animators[2];xge3d_clip *clips[2];
    int frame,frames,failed,mix;float time;const char *action,*capture;
} demo;
static void release(demo *d)
{
    for (int i=0;i<2;++i) {xge3dAnimatorFree(d->animators[i]);d->animators[i]=NULL;xge3dClipFree(d->clips[i]);d->clips[i]=NULL;}
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;
}
static int actor(demo *d,int index)
{
    char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/rig-%c.gltf",'a'+index);
    xge3d_model *model=NULL;xge3d_node_t root,mesh;int result=xge3dModelLoad(path,&model);
    if (result==XGE_OK) result=xge3dModelInstantiate(d->scene,model,(xge3d_node_t){0},&root);
    if (result==XGE_OK) result=xge3dModelInstanceNode(d->scene,root,17,&mesh);
    if (result==XGE_OK) result=xge3dAnimatorCreate(d->scene,root,&d->animators[index]);
    xge3dModelFree(model);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=index ? 1.3f : -1.3f;
    if (result==XGE_OK) result=xge3dNodeSetTransform(d->scene,root,&t);
    xge3d_material_desc_t md=xge3dMaterialDefault();md.metallic=0;md.roughness=.8f;md.double_sided=1;
    md.base_color[0]=index ? .7f : .08f;md.base_color[1]=index ? .15f : .55f;md.base_color[2]=index ? .2f : .8f;
    for (int k=0;k<3;++k) md.emissive[k]=md.base_color[k]*.08f;
    xge3d_material *material=NULL;if (result==XGE_OK) result=xge3dMaterialCreate(&md,&material);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(d->scene,mesh,material);
    xge3dMaterialFree(material);
    snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/retarget-%c-%s.gltf",'a'+index,d->action);
    if (result==XGE_OK) result=xge3dClipLoad(path,0,&d->clips[index]);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=d->time;
    if (result==XGE_OK) result=xge3dAnimatorSetLayer(d->animators[index],0,d->clips[index],&layer);
    if (d->mix && result==XGE_OK) {
        xge3d_clip *idle=NULL;snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/retarget-%c-idle.gltf",'a'+index);
        result=xge3dClipLoad(path,0,&idle);layer.time=0;layer.weight=0;
        if (result==XGE_OK) result=xge3dAnimatorSetLayer(d->animators[index],1,idle,&layer);
        xge3dClipFree(idle);
    }
    return result;
}
static int frame(void *user)
{
    demo *d=user;int result=XGE_OK;
    if (!d->renderer && (result=xge3dRendererCreate(&d->renderer))!=XGE_OK) goto fail;
    if (d->capture && !d->target && (result=xge3dTargetCreate(640,480,&d->target))!=XGE_OK) goto fail;
    for (int i=0;i<2;++i) {
        if (d->mix) xge3dAnimatorSetLayerWeight(d->animators[i],1,d->frame<180 ? (float)d->frame/180 : 1);
        if ((result=xge3dAnimatorUpdate(d->animators[i],1.f/60))!=XGE_OK) goto fail;
    }
    xge3d_camera_t camera;
    if ((result=xge3dCameraPerspective(&camera,.8f,(float)xgeGetWidth()/xgeGetHeight(),.1f,50))!=XGE_OK) goto fail;
    if ((result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){3,2.7f,7},(xge3d_vec3_t){0,1.15f,0},(xge3d_vec3_t){0,1,0}))!=XGE_OK) goto fail;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=1024;shadows.distance=20;shadows.depth_padding=5;
    xge3d_render_desc_t draw={&camera,NULL,{.08f,.12f,.18f,1},3};draw.shadows=&shadows;draw.exposure=1;
    xge3d_render_stats_t stats;if ((result=xge3dRender(d->renderer,d->scene,&draw,&stats))!=XGE_OK) goto fail;
    if (++d->frame==d->frames || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture) {
            xarray pixels;xrtArrayInit(&pixels,1);if (!xrtArrayResize(&pixels,640*480*4)) {result=XGE_ERROR_OUT_OF_MEMORY;goto fail;}
            draw.target=d->target;result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,640*4);
            if (result==XGE_OK) result=xgeImageSavePNG(d->capture,640,480,pixels.Data,640*4);
            xrtArrayUnit(&pixels);if (result!=XGE_OK) goto fail;
        }
        printf("animation %s: %d frames, %u draws, %u shadow maps, %llu pose/upload bytes\n",d->action,d->frame,stats.draw_calls,stats.shadow_maps,(unsigned long long)stats.upload_bytes);
        release(d);return 1;
    }
    return 0;
fail:
    fprintf(stderr,"animation failed: %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.action="walk"};
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--frames") && i+1<argc) {d.frames=atoi(argv[++i]);if (d.frames<=0) return 2;}
        else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];
        else if (!strcmp(argv[i],"--action") && i+1<argc) d.action=argv[++i];
        else if (!strcmp(argv[i],"--time") && i+1<argc) d.time=(float)atof(argv[++i]);
        else if (!strcmp(argv[i],"--mix")) d.mix=1;
        else return 2;
    }
    xge_desc_t w={0};w.iWidth=640;w.iHeight=480;w.sTitle="XGE independent external actions on T/A rigs";
    if (xgeInit(&w)!=XGE_OK) return 1;
    int result=xge3dSceneCreate(&d.scene);
    for (int i=0;i<2 && result==XGE_OK;++i) result=actor(&d,i);
    xge3d_node_t sun;if (result==XGE_OK) result=xge3dNodeCreate(d.scene,(xge3d_node_t){0},&sun);
    xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){-.5f,-1,-1};light.intensity=2;light.casts_shadow=1;
    if (result==XGE_OK) result=xge3dNodeSetLight(d.scene,sun,&light);
    if (result==XGE_OK) result=xgeRun(frame,&d);
    release(&d);xgeUnit();return result==XGE_OK && !d.failed ? 0 : 1;
}
