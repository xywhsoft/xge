#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRY(x) do{int result=(x);if(result)return result;}while(0)
typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    xge3d_animator *animator;xge3d_clip *clips[4];
    int frame,frames,autopilot,action,overlay,paused,layer_paused,failed,completed,events;
    double previous;const char *capture;
} demo;
static int action(demo *d,int index,float fade)
{
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=index!=3;
    TRY(xge3dAnimatorPlay(d->animator,0,d->clips[index],&layer,fade));d->action=index;
    xge3d_animation_event_t end={0,100,(float)xge3dClipInfo(d->clips[index]).duration};
    return xge3dAnimatorSetEvents(d->animator,0,&end,1);
}
static void release(demo *d)
{
    xge3dAnimatorFree(d->animator);d->animator=NULL;for(int i=0;i<4;++i){xge3dClipFree(d->clips[i]);d->clips[i]=NULL;}
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;
}
static int prepare(demo *d)
{
    TRY(xge3dSceneCreate(&d->scene));TRY(xge3dRendererCreate(&d->renderer));
    TRY(xge3dRendererSetAntialiasing(d->renderer,XGE3D_AA_FXAA));
    xge3d_model *model=NULL;TRY(xge3dModelLoad("examples/xge/xge_3d_walk/assets/explorer.gltf",&model));
    xge3d_node_t root;int result=xge3dModelInstantiate(d->scene,model,(xge3d_node_t){0},&root);
    if(!result)result=xge3dAnimatorCreate(d->scene,root,&d->animator);
    for(int i=0;i<4&&!result;++i)result=xge3dClipFromModel(model,i,&d->clips[i]);
    /* Masks use model source-node indices. This upper-body overlay is merely
       an example; the library knows no character/gameplay state. */
    xarray mask;xrtArrayInit(&mask,sizeof(float));size_t count=xge3dModelInfo(model).node_count;
    if(!result&&!xrtArrayResize(&mask,count))result=XGE_ERROR_OUT_OF_MEMORY;
    for(size_t i=0;i<count&&!result;++i){const char *name=xge3dModelNodeName(model,i);
        ((float*)mask.Data)[i]=name&&(!strncmp(name,"arm",3)||!strncmp(name,"forearm",7)||!strncmp(name,"hand",4))?1:0;}
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.mask=(const float*)mask.Data;layer.mask_count=count;layer.weight=0;layer.speed=.7f;
    if(!result)result=xge3dAnimatorSetLayer(d->animator,1,d->clips[3],&layer);
    if(!result)result=xge3dAnimatorSetLayerMode(d->animator,1,XGE3D_ANIMATION_ADDITIVE);
    xrtArrayUnit(&mask);xge3dModelFree(model);if(result)return result;
    TRY(action(d,0,0));
    xge3d_vertex_t vertices[6]={0};const int x[]={-1,1,1,-1,1,-1},z[]={1,1,-1,1,-1,-1};
    for(int i=0;i<6;++i){vertices[i].position=(xge3d_vec3_t){x[i]*4.f,0,z[i]*4.f};vertices[i].normal.y=1;}
    xge3d_mesh_desc_t md={vertices,6};xge3d_mesh *mesh=NULL;TRY(xge3dMeshCreate(&md,&mesh));xge3d_node_t floor;
    result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&floor);if(!result)result=xge3dNodeSetMesh(d->scene,floor,mesh);xge3dMeshFree(mesh);if(result)return result;
    xge3d_material_desc_t material=xge3dMaterialDefault();material.base_color[0]=.12f;material.base_color[1]=.18f;material.base_color[2]=.23f;material.metallic=0;
    xge3d_material *m=NULL;TRY(xge3dMaterialCreate(&material,&m));result=xge3dNodeSetMaterial(d->scene,floor,m);xge3dMaterialFree(m);if(result)return result;
    xge3d_node_t sun;TRY(xge3dNodeCreate(d->scene,(xge3d_node_t){0},&sun));
    xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){-.5f,-1,-.6f};light.intensity=3;light.casts_shadow=1;
    return xge3dNodeSetLight(d->scene,sun,&light);
}
static int controls(demo *d)
{
    if(d->autopilot){
        if(d->frame==60)TRY(action(d,1,.3f));
        if(d->frame==70)TRY(action(d,2,.35f)); /* Interrupt an unfinished blend. */
        if(d->frame==120){d->overlay=1;TRY(xge3dAnimatorFadeLayer(d->animator,1,.7f,.3f));}
        if(d->frame==150||d->frame==180){d->paused=d->frame==150;TRY(xge3dAnimatorSetPaused(d->animator,d->paused));}
        if(d->frame==210||d->frame==240){d->layer_paused=d->frame==210;TRY(xge3dAnimatorSetLayerPaused(d->animator,1,d->layer_paused));}
        if(d->frame==270)TRY(action(d,3,.2f));
        if(d->frame==360){d->overlay=0;TRY(xge3dAnimatorPlay(d->animator,1,NULL,NULL,.3f));}
    }else{
        for(int i=0;i<4;++i)if(xgeKeyPressed('1'+i))TRY(action(d,i,.25f));
        if(xgeKeyPressed('P')){d->paused=!d->paused;TRY(xge3dAnimatorSetPaused(d->animator,d->paused));}
        if(xgeKeyPressed('L')){d->layer_paused=!d->layer_paused;TRY(xge3dAnimatorSetLayerPaused(d->animator,1,d->layer_paused));}
        if(xgeKeyPressed('O')){d->overlay=!d->overlay;TRY(xge3dAnimatorFadeLayer(d->animator,1,d->overlay?.7f:0,.3f));}
    }
    xge3d_playback_info_t state={.size=sizeof(state)};TRY(xge3dAnimatorGetPlayback(d->animator,0,&state));
    if(state.finished){++d->completed;TRY(action(d,0,.25f));}return XGE_OK;
}
static int frame(void *user)
{
    demo *d=user;int result=0;if(!d->renderer){result=prepare(d);if(result)goto failed;}
    result=controls(d);if(result)goto failed;
    double now=xgeTimer();float dt=d->autopilot?1.f/60:d->previous?(float)(now-d->previous):0;d->previous=now;if(dt>.05f)dt=.05f;
    result=xge3dAnimatorUpdate(d->animator,dt);if(result)goto failed;
    xge3d_animation_event_t event;while(xge3dAnimatorNextEvent(d->animator,&event)==XGE_OK)++d->events;
    xge3d_playback_info_t state={.size=sizeof(state)};result=xge3dAnimatorGetPlayback(d->animator,0,&state);if(result)goto failed;
    char title[240];snprintf(title,sizeof(title),"1 idle  2 walk  3 run  4 jump | P pause %d  O overlay %d  L layer pause %d | time %.2f gain %.2f fade %.2f",d->paused,d->overlay,d->layer_paused,state.time,state.weight,state.fade_remaining);xgeSetTitle(title);
    int width=xgeGetWidth(),height=xgeGetHeight();if(width<=0||height<=0)return 0;
    xge3d_camera_t camera;result=xge3dCameraPerspective(&camera,.7f,(float)width/height,.1f,30);if(result)goto failed;
    result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){2.8f,2.1f,4},(xge3d_vec3_t){0,.85f,0},(xge3d_vec3_t){0,1,0});if(result)goto failed;
    xge3d_shadow_settings_t shadow=xge3dShadowDefault();shadow.resolution=1024;shadow.distance=15;shadow.cascades=1;
    xge3d_render_desc_t draw={&camera,NULL,{.045f,.065f,.09f,1},3};draw.shadows=&shadow;
    result=xge3dRender(d->renderer,d->scene,&draw,NULL);if(result)goto failed;++d->frame;
    if((d->frames&&d->frame>=d->frames)||xgeKeyPressed(XGE_KEY_ESCAPE)){
        if(d->capture){result=xge3dTargetCreate(width,height,&d->target);draw.target=d->target;xarray pixels;xrtArrayInit(&pixels,1);
            if(!result&&!xrtArrayResize(&pixels,(size_t)width*height*4))result=XGE_ERROR_OUT_OF_MEMORY;
            if(!result)result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if(!result)result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,width*4);
            if(!result)result=xgeImageSavePNGEx(d->capture,width,height,pixels.Data,width*4,XGE_IMAGE_PREMULTIPLIED);
            xrtArrayUnit(&pixels);}
        if(result)goto failed;
        printf("Playback demo: %d frames, %d completed one-shot, %d events\n",d->frame,d->completed,d->events);
        if(d->autopilot&&d->frame>=420&&(!d->completed||!d->events)){result=XGE_ERROR_INVALID_STATE;goto failed;}
        release(d);return 1;}
    return 0;
failed:fprintf(stderr,"Playback demo error %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={0};for(int i=1;i<argc;++i){if(!strcmp(argv[i],"--frames")&&i+1<argc){d.frames=atoi(argv[++i]);if(d.frames<=0)return 2;d.autopilot=1;}
        else if(!strcmp(argv[i],"--auto"))d.autopilot=1;else if(!strcmp(argv[i],"--capture")&&i+1<argc)d.capture=argv[++i];else return 2;}
    xge_desc_t window={.iWidth=900,.iHeight=640,.iTargetFPS=60,.sTitle="XGE playback"};if(xgeInit(&window))return 1;
    int result=xgeRun(frame,&d);release(&d);xgeUnit();return result||d.failed?1:0;
}
