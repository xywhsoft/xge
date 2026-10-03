#include "xge.h"
#include "lib/xrt/xrt.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct demo {
    xge3d_scene *scene; xge3d_renderer *renderer; xge3d_target *target;
    xge3d_environment *environment; xge3d_node_t sun;
    int frame,frames,failed,use_ibl;const char *capture;
} demo;
static void release(demo *d)
{
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dEnvironmentFree(d->environment);d->environment=NULL;
    xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;
}
static int attach(demo *d,xge3d_mesh *mesh,xge3d_vec3_t position,xge3d_vec3_t scale,float metallic,float roughness)
{
    xge3d_node_t node;xge3d_material *material=NULL;int result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&node);
    xge3d_material_desc_t m=xge3dMaterialDefault();m.metallic=metallic;m.roughness=roughness;
    m.base_color[0]=.7f;m.base_color[1]=.35f;m.base_color[2]=.1f;
    if (result==XGE_OK) result=xge3dMaterialCreate(&m,&material);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(d->scene,node,material);
    if (result==XGE_OK) result=xge3dNodeSetMesh(d->scene,node,mesh);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=position;t.scale=scale;
    if (result==XGE_OK) result=xge3dNodeSetTransform(d->scene,node,&t);
    xge3dMaterialFree(material);return result;
}
static int load_ibl(xge3d_environment *env)
{
    const char *faces[]={"px","nx","py","ny","pz","nz"};xge_image_t images[25]={0};
    xge3d_cube_level_t levels[3]={0};xge3d_ibl_desc_t ibl={0};ibl.prefiltered=levels;ibl.level_count=3;ibl.brdf=&images[24];
    int result=XGE_OK;
    for (int f=0;f<6 && result==XGE_OK;++f) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/ibl-irr-%s.png",faces[f]);
        result=xgeImageLoadEx(&images[f],path,XGE_IMAGE_STRAIGHT_ALPHA);ibl.irradiance.faces[f]=&images[f];
        for (int l=0;l<3 && result==XGE_OK;++l) {
            snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/ibl-spec-%d-%s.png",l,faces[f]);
            result=xgeImageLoadEx(&images[6+l*6+f],path,XGE_IMAGE_STRAIGHT_ALPHA);levels[l].faces[f]=&images[6+l*6+f];
        }
    }
    if (result==XGE_OK) result=xgeImageLoadEx(&images[24],"artifacts/xge-3d/fixtures/ibl-brdf.png",XGE_IMAGE_STRAIGHT_ALPHA);
    if (result==XGE_OK) result=xge3dEnvironmentSetIBL(env,&ibl);
    for (int i=0;i<25;++i) xgeImageFree(&images[i]);
    return result;
}
static int frame(void *user)
{
    demo *d=user;int result=XGE_OK;
    if (!d->renderer && (result=xge3dRendererCreate(&d->renderer))!=XGE_OK) goto fail;
    if (d->capture && !d->target && (result=xge3dTargetCreate(640,480,&d->target))!=XGE_OK) goto fail;
    /* Daylight parameters belong to this application. */
    float angle=.65f+d->frame*.02f;
    xge3d_light_desc_t sun=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);
    sun.direction=(xge3d_vec3_t){sinf(angle),-1,-cosf(angle)};sun.intensity=2;sun.casts_shadow=1;
    if ((result=xge3dNodeSetLight(d->scene,d->sun,&sun))!=XGE_OK) goto fail;
    xge3d_camera_t camera;
    if ((result=xge3dCameraPerspective(&camera,1,(float)xgeGetWidth()/xgeGetHeight(),.1f,50))!=XGE_OK) goto fail;
    if ((result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){4,3,6},(xge3d_vec3_t){0,.3f,0},(xge3d_vec3_t){0,1,0}))!=XGE_OK) goto fail;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.distance=20;shadows.resolution=1024;shadows.depth_padding=10;
    xge3d_render_desc_t draw={&camera,NULL,{0,0,0,1},3,d->environment};draw.shadows=&shadows;draw.ibl_intensity=d->use_ibl ? .15f : 0;
    draw.exposure=.8f+.2f*cosf(angle);xge3d_render_stats_t stats;
    if ((result=xge3dRender(d->renderer,d->scene,&draw,&stats))!=XGE_OK) goto fail;
    if (++d->frame==d->frames || xgeKeyPressed(XGE_KEY_ESCAPE)) {
        if (d->capture) {
            xarray pixels;xrtArrayInit(&pixels,1);
            if (!xrtArrayResize(&pixels,640*480*4)) { result=XGE_ERROR_OUT_OF_MEMORY;goto fail; }
            draw.target=d->target;result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,640*4);
            if (result==XGE_OK) result=xgeImageSavePNG(d->capture,640,480,pixels.Data,640*4);
            xrtArrayUnit(&pixels);if (result!=XGE_OK) goto fail;
        }
        printf("lighting: %d frames, %u draws, %u shadow maps, %llu uploaded bytes\n",d->frame,stats.draw_calls,stats.shadow_maps,(unsigned long long)stats.upload_bytes);
        release(d);return 1;
    }
    return 0;
fail:
    fprintf(stderr,"lighting failed: %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={0};
    for (int i=1;i<argc;++i) {
        if (!strcmp(argv[i],"--frames") && i+1<argc) {d.frames=atoi(argv[++i]);if (d.frames<=0) return 2;}
        else if (!strcmp(argv[i],"--capture") && i+1<argc) d.capture=argv[++i];
        else if (!strcmp(argv[i],"--ibl")) d.use_ibl=1;
        else return 2;
    }
    xge_desc_t window={0};window.iWidth=640;window.iHeight=480;window.sTitle="XGE PBR and sun shadows";
    if (xgeInit(&window)!=XGE_OK) return 1;
    int result=xge3dSceneCreate(&d.scene);xge3d_mesh *mesh=NULL;
    const xge3d_vec3_t points[]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
        {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int indices[]={4,5,6,4,6,7,1,0,3,1,3,2,0,4,7,0,7,3,5,1,2,5,2,6,0,1,5,0,5,4,7,6,2,7,2,3};
    const xge3d_vec3_t normals[]={{0,0,1},{0,0,-1},{-1,0,0},{1,0,0},{0,-1,0},{0,1,0}};
    xge3d_vertex_t v[36]={0};for (int i=0;i<36;++i) {v[i].position=points[indices[i]];v[i].normal=normals[i/6];}
    xge3d_mesh_desc_t md={v,36,NULL,0,0};if (result==XGE_OK) result=xge3dMeshCreate(&md,&mesh);
    if (result==XGE_OK) result=attach(&d,mesh,(xge3d_vec3_t){-.7f,.5f,0},(xge3d_vec3_t){1,1,1},0,.7f);
    if (result==XGE_OK) result=attach(&d,mesh,(xge3d_vec3_t){.7f,.35f,.2f},(xge3d_vec3_t){.7f,.7f,.7f},1,.2f);
    if (result==XGE_OK) result=attach(&d,mesh,(xge3d_vec3_t){0,-.1f,0},(xge3d_vec3_t){8,.2f,8},0,1);
    xge3dMeshFree(mesh);
    if (result==XGE_OK) result=xge3dNodeCreate(d.scene,(xge3d_node_t){0},&d.sun);
    unsigned char colors[6][4]={{60,85,120,255},{60,85,120,255},{95,145,210,255},
        {20,30,45,255},{60,85,120,255},{60,85,120,255}};
    xge_image_t faces[6];xge3d_environment_desc_t sky={0};sky.srgb=1;
    for (int i=0;i<6;++i) {faces[i]=(xge_image_t){1,1,XGE_PIXEL_RGBA8,4,colors[i],XGE_IMAGE_STRAIGHT_ALPHA};sky.faces[i]=&faces[i];}
    if (result==XGE_OK) result=xge3dEnvironmentCreate(&sky,&d.environment);
    if (result==XGE_OK && d.use_ibl) result=load_ibl(d.environment);
    if (result==XGE_OK) result=xgeRun(frame,&d);
    release(&d);xgeUnit();return result==XGE_OK && !d.failed ? 0 : 1;
}
