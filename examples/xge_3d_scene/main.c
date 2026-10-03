#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct demo {
    xge3d_scene *scene;
    xge3d_renderer *renderer;
    xge3d_target *target;
    xge3d_environment *environment;
    int frames, frame, pressed, failed;
    const char *capture;
} demo;
static void release(demo *d)
{
    xge3dSceneFree(d->scene); d->scene=NULL;
    xge3dEnvironmentFree(d->environment); d->environment=NULL;
    xge3dTargetFree(d->target); d->target=NULL;
    xge3dRendererFree(d->renderer); d->renderer=NULL;
}
static int frame(void *user)
{
    demo *d=user; int result=XGE_OK;
    if (!d->renderer && (result=xge3dRendererCreate(&d->renderer))!=XGE_OK) goto fail;
    if (d->capture && !d->target && (result=xge3dTargetCreate(640,480,&d->target))!=XGE_OK) goto fail;
    xge3d_camera_t camera;
    if ((result=xge3dCameraPerspective(&camera,1,(float)xgeGetWidth()/xgeGetHeight(),.1f,100))!=XGE_OK) goto fail;
    if ((result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,5},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}))!=XGE_OK) goto fail;
    int pressed=xgeMouseDown(0);
    if (pressed && !d->pressed) {
        float x,y; xgeMouseGet(&x,&y); xge3d_ray_t ray; xge3d_hit_t hit;
        if (xge3dCameraScreenRay(&camera,x,y,(float)xgeGetWidth(),(float)xgeGetHeight(),&ray)==XGE_OK &&
            xge3dSceneRaycast(d->scene,&ray,100,&hit)==XGE_OK) {
            const float selected[]={1,.65f,.15f,1}; xge3dNodeSetColor(d->scene,hit.node,selected);
            printf("selected instance %llu, triangle %zu, distance %.3f\n",(unsigned long long)hit.model_root.slot,hit.triangle,hit.distance);
        }
    }
    d->pressed=pressed;
    xge3d_render_desc_t draw={&camera,NULL,{.02f,.03f,.05f,1},3,d->environment};
    xge3d_render_stats_t stats;
    if ((result=xge3dRender(d->renderer,d->scene,&draw,&stats))!=XGE_OK) goto fail;
    if (++d->frame==d->frames || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture) {
            unsigned char *pixels=malloc(640*480*4);
            if (!pixels) { result=XGE_ERROR_OUT_OF_MEMORY; goto fail; }
            draw.target=d->target;
            result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels,640*480*4,640*4);
            if (result==XGE_OK) result=xgeImageSavePNG(d->capture,640,480,pixels,640*4);
            free(pixels); if (result!=XGE_OK) goto fail;
        }
        printf("xge_3d_scene: %d frames, %u draws, %llu triangles\n",d->frame,stats.draw_calls,(unsigned long long)stats.triangles);
        release(d); return 1;
    }
    return 0;
fail:
    fprintf(stderr,"xge_3d_scene failed: %d\n",result); d->failed=1; release(d); return 1;
}
int main(int argc,char **argv)
{
    demo d={0}; const char *uri="artifacts/xge-3d/fixtures/embedded.glb";
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--model") && i+1<argc) uri=argv[++i];
        else if (!strcmp(argv[i],"--frames") && i+1<argc) { d.frames=atoi(argv[++i]); if (d.frames<=0) return 2; }
        else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];
        else { fprintf(stderr,"usage: xge_3d_scene [--model model.glb] [--frames N] [--capture file.png]\n"); return 2; }
    }
    xge_desc_t desc={0}; desc.iWidth=640; desc.iHeight=480; desc.sTitle="XGE model scene - click to select";
    if (xgeInit(&desc)!=XGE_OK) return 1;
    xge3d_model *model=NULL; int result=xge3dSceneCreate(&d.scene);
    if (result==XGE_OK) result=xge3dModelLoad(uri,&model);
    for (int i=0;result==XGE_OK && i<2;++i) {
        xge3d_node_t root; result=xge3dModelInstantiate(d.scene,model,(xge3d_node_t){0},&root);
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY; t.position=(xge3d_vec3_t){-2.8f+i*1.6f,-1,0};
        if (result==XGE_OK) result=xge3dNodeSetTransform(d.scene,root,&t);
    }
    xge3dModelFree(model); /* Instances now retain shared geometry and textures. */
    unsigned char colors[6][4]={{50,65,90,255},{50,65,90,255},{80,120,180,255},
        {20,25,35,255},{45,60,85,255},{45,60,85,255}};
    xge_image_t images[6]; xge3d_environment_desc_t sky={0}; sky.srgb=1;
    for (int i=0;i<6;++i) { images[i]=(xge_image_t){1,1,XGE_PIXEL_RGBA8,4,colors[i],XGE_IMAGE_STRAIGHT_ALPHA}; sky.faces[i]=&images[i]; }
    if (result==XGE_OK) result=xge3dEnvironmentCreate(&sky,&d.environment);
    if (result==XGE_OK) result=xgeRun(frame,&d);
    release(&d); xgeUnit(); return result==XGE_OK && !d.failed ? 0 : 1;
}
