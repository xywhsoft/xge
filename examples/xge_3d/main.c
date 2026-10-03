#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct demo {
    xge3d_scene *scene;
    xge3d_mesh *mesh;
    xge3d_renderer *renderer;
    xge3d_target *target;
    xge3d_node_t cube, child;
    int frames, frame, failed;
    const char *capture;
} demo;
static const xge3d_vertex_t vertices[]={
    {{-.5f,-.5f,-.5f}},{{.5f,-.5f,-.5f}},{{.5f,.5f,-.5f}},{{-.5f,.5f,-.5f}},
    {{-.5f,-.5f,.5f}},{{.5f,-.5f,.5f}},{{.5f,.5f,.5f}},{{-.5f,.5f,.5f}}
};
static const uint16_t indices[]={4,5,6,4,6,7,1,0,3,1,3,2,0,4,7,0,7,3,
    5,1,2,5,2,6,0,1,5,0,5,4,7,6,2,7,2,3};

static void release(demo *d)
{
    xge3dSceneFree(d->scene); d->scene=NULL;
    xge3dMeshFree(d->mesh); d->mesh=NULL;
    xge3dTargetFree(d->target); d->target=NULL;
    xge3dRendererFree(d->renderer); d->renderer=NULL;
}
static int frame(void *user)
{
    demo *d=user;
    int result=XGE_OK;
    if (!d->renderer) {
        if ((result=xge3dRendererCreate(&d->renderer))!=XGE_OK) goto fail;
        if (d->capture && (result=xge3dTargetCreate(640,480,&d->target))!=XGE_OK) goto fail;
    }
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
    float angle=.7f+d->frame*.01f;
    t.rotation=(xge3d_quat_t){0,sinf(angle*.5f),0,cosf(angle*.5f)};
    if ((result=xge3dNodeSetTransform(d->scene,d->cube,&t))!=XGE_OK) goto fail;
    xge3d_camera_t camera;
    if ((result=xge3dCameraPerspective(&camera,1.05f,(float)xgeGetWidth()/xgeGetHeight(),.1f,100))!=XGE_OK) goto fail;
    if ((result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){2,1.6f,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}))!=XGE_OK) goto fail;
    xge3d_render_desc_t desc={&camera,NULL,{.025f,.04f,.07f,1},XGE3D_CLEAR_COLOR|XGE3D_CLEAR_DEPTH};
    if ((result=xge3dRender(d->renderer,d->scene,&desc,NULL))!=XGE_OK) goto fail;
    if (++d->frame==d->frames || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture) {
            desc.target=d->target;
            unsigned char *pixels=malloc(640*480*4);
            if (!pixels) { result=XGE_ERROR_OUT_OF_MEMORY; goto fail; }
            result=xge3dRender(d->renderer,d->scene,&desc,NULL);
            if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels,640*480*4,640*4);
            if (result==XGE_OK) result=xgeImageSavePNG(d->capture,640,480,pixels,640*4);
            free(pixels);
            if (result!=XGE_OK) goto fail;
        }
        release(d);
        printf("xge_3d: %d frames completed\n",d->frame);
        return 1;
    }
    return 0;
fail:
    fprintf(stderr,"xge_3d failed: %d\n",result);
    d->failed=1; release(d); return 1;
}
int main(int argc, char **argv)
{
    demo d={0};
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--frames") && i+1<argc) {
            d.frames=atoi(argv[++i]); if (d.frames<=0) return 2;
        } else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];
        else { fprintf(stderr,"usage: xge_3d [--frames N] [--capture image.png]\n"); return 2; }
    }
    xge3d_mesh_desc_t md={vertices,8,indices,36,16};
    if (xge3dSceneCreate(&d.scene)!=XGE_OK || xge3dMeshCreate(&md,&d.mesh)!=XGE_OK ||
        xge3dNodeCreate(d.scene,(xge3d_node_t){0},&d.cube)!=XGE_OK ||
        xge3dNodeCreate(d.scene,d.cube,&d.child)!=XGE_OK ||
        xge3dNodeSetMesh(d.scene,d.cube,d.mesh)!=XGE_OK || xge3dNodeSetMesh(d.scene,d.child,d.mesh)!=XGE_OK) {
        release(&d); return 1;
    }
    const float orange[]={1,.38f,.08f,1}, cyan[]={.05f,.7f,.95f,1};
    xge3dNodeSetColor(d.scene,d.cube,orange); xge3dNodeSetColor(d.scene,d.child,cyan);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
    t.position=(xge3d_vec3_t){1.1f,.3f,0}; t.scale=(xge3d_vec3_t){.5f,.5f,.5f};
    xge3dNodeSetTransform(d.scene,d.child,&t);
    xge_desc_t desc={0}; desc.iWidth=640; desc.iHeight=480; desc.sTitle="XGE 3D";
    if (xgeInit(&desc)!=XGE_OK) { release(&d); return 1; }
    int result=xgeRun(frame,&d);
    release(&d); xgeUnit();
    return result==XGE_OK && !d.failed ? 0 : 1;
}
