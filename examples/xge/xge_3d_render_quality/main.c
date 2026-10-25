#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    xge3d_node_t floor,caster;int aa,casts,receives,frames,frame,failed;const char *capture;
} demo;
static int cube(xge3d_mesh **out)
{
    const xge3d_vec3_t ns[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const xge3d_vec3_t us[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const xge3d_vec3_t vs[]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
    const int cs[]={0,1,2,0,2,3},xs[]={-1,1,1,-1},ys[]={-1,-1,1,1};xge3d_vertex_t v[36]={0};
    for(int f=0;f<6;++f)for(int k=0;k<6;++k){int c=cs[k];v[f*6+k].normal=ns[f];v[f*6+k].position=(xge3d_vec3_t){.5f*(ns[f].x+us[f].x*xs[c]+vs[f].x*ys[c]),.5f*(ns[f].y+us[f].y*xs[c]+vs[f].y*ys[c]),.5f*(ns[f].z+us[f].z*xs[c]+vs[f].z*ys[c])};}
    xge3d_mesh_desc_t md={v,36};return xge3dMeshCreate(&md,out);
}
static int object(demo *d,xge3d_mesh *mesh,xge3d_vec3_t position,xge3d_vec3_t scale,const float *color,int unlit,xge3d_node_t *out)
{
    int result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},out);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=position;t.scale=scale;
    if(!result)result=xge3dNodeSetTransform(d->scene,*out,&t);
    if(!result)result=xge3dNodeSetMesh(d->scene,*out,mesh);
    xge3d_material_desc_t m=xge3dMaterialDefault();memcpy(m.base_color,color,sizeof(m.base_color));m.metallic=0;m.roughness=.7f;m.unlit=unlit;
    m.double_sided=1;m.alpha_mode=color[3]<1?XGE3D_ALPHA_BLEND:XGE3D_ALPHA_OPAQUE;
    m.emissive[0]=m.emissive[1]=m.emissive[2]=unlit?0:.025f;
    xge3d_material *material=NULL;if(!result)result=xge3dMaterialCreate(&m,&material);
    if(!result){result=xge3dNodeSetMaterial(d->scene,*out,material);}xge3dMaterialFree(material);return result;
}
static int settings(demo *d)
{
    int result=xge3dRendererSetAntialiasing(d->renderer,d->aa?XGE3D_AA_FXAA:XGE3D_AA_NONE);
    if(!result)result=xge3dNodeSetShadowFlags(d->scene,d->caster,d->casts?XGE3D_SHADOW_CAST:0);
    if(!result)result=xge3dNodeSetShadowFlags(d->scene,d->floor,d->receives?XGE3D_SHADOW_RECEIVE:0);
    char title[160];snprintf(title,sizeof(title),"XGE: A FXAA %s | C cast %s | R receive %s | Esc exit",d->aa?"ON":"OFF",d->casts?"ON":"OFF",d->receives?"ON":"OFF");xgeSetTitle(title);return result;
}
static void release(demo *d)
{xge3dSceneFree(d->scene);d->scene=NULL;xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;}
static int prepare(demo *d)
{
    int result=xge3dSceneCreate(&d->scene);if(!result)result=xge3dRendererCreate(&d->renderer);xge3d_mesh *mesh=NULL;if(!result)result=cube(&mesh);
    const float gray[]={.5f,.55f,.65f,1},white[]={1,1,1,1},orange[]={.8f,.3f,.05f,1},blue[]={.05f,.25f,1,1};
    if(!result)result=object(d,mesh,(xge3d_vec3_t){0,-.1f,0},(xge3d_vec3_t){10,.2f,8},gray,0,&d->floor);
    if(!result)result=object(d,mesh,(xge3d_vec3_t){-.5f,1,0},(xge3d_vec3_t){1,2,1},orange,0,&d->caster);
    for(int i=0;i<12&&!result;++i){xge3d_node_t pole;result=object(d,mesh,(xge3d_vec3_t){-3.5f+i*.45f,.8f,-2.5f},(xge3d_vec3_t){.045f,1.6f,.045f},white,1,&pole);}
    xge3d_node_t n;
    if(!result)result=object(d,mesh,(xge3d_vec3_t){2,.7f,1},(xge3d_vec3_t){1.5f,1.4f,.04f},blue,1,&n);
    float red[]={1,0,0,.5f};
    if(!result)result=object(d,mesh,(xge3d_vec3_t){2,.7f,1.4f},(xge3d_vec3_t){1.5f,1.4f,.01f},red,1,&n);
    /* Each pane has one face: overlapping surfaces are not a glass volume. */
    if(!result){xge3d_vertex_t v[6]={0};float xy[][2]={{-.5f,-.5f},{.5f,-.5f},{.5f,.5f},{-.5f,-.5f},{.5f,.5f},{-.5f,.5f}};
        for(int i=0;i<6;++i){v[i].position=(xge3d_vec3_t){xy[i][0],xy[i][1],0};v[i].normal.z=1;}
        xge3d_mesh_desc_t desc={v,6};xge3d_mesh *pane=NULL;result=xge3dMeshCreate(&desc,&pane);if(!result)result=xge3dNodeSetMesh(d->scene,n,pane);xge3dMeshFree(pane);}
    xge3dMeshFree(mesh);xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){.6f,-1,-.4f};light.intensity=3;light.casts_shadow=1;
    if(!result){result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&n);}if(!result){result=xge3dNodeSetLight(d->scene,n,&light);}
    if(!result){result=settings(d);}return result;
}
static int frame(void *user)
{
    demo *d=user;int result=0;
    if(!d->renderer){result=prepare(d);if(result){goto failed;}}
    if(xgeKeyPressed('A')){d->aa=!d->aa;result=settings(d);}if(xgeKeyPressed('C')){d->casts=!d->casts;result=settings(d);}if(xgeKeyPressed('R')){d->receives=!d->receives;result=settings(d);}
    int width=xgeGetWidth(),height=xgeGetHeight();if(width<=0||height<=0)return 0;
    xge3d_camera_t camera;if(!result)result=xge3dCameraPerspective(&camera,.8f,(float)width/height,.1f,50);
    if(!result)result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){6,4,8},(xge3d_vec3_t){0,.5f,0},(xge3d_vec3_t){0,1,0});
    xge3d_shadow_settings_t shadow=xge3dShadowDefault();shadow.resolution=1024;shadow.distance=30;shadow.cascades=2;shadow.normal_bias=.025f;
    xge3d_render_desc_t draw={&camera,NULL,{.06f,.08f,.12f,1},3};draw.shadows=&shadow;
    if(!result)result=xge3dRender(d->renderer,d->scene,&draw,NULL);
    if(result){goto failed;}++d->frame;
    if((d->frames&&d->frame>=d->frames)||xgeKeyPressed(XGE_KEY_ESCAPE)){
        if(d->capture){result=xge3dTargetCreate(width,height,&d->target);draw.target=d->target;
            xarray pixels;xrtArrayInit(&pixels,1);if(!result&&!xrtArrayResize(&pixels,(size_t)width*height*4))result=XGE_ERROR_OUT_OF_MEMORY;
            if(!result)result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if(!result)result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,width*4);
            if(!result){result=xgeImageSavePNGEx(d->capture,width,height,pixels.Data,width*4,XGE_IMAGE_PREMULTIPLIED);}xrtArrayUnit(&pixels);}
        if(result){goto failed;}printf("Render demo: %d frames, FXAA %d, cast %d, receive %d\n",d->frame,d->aa,d->casts,d->receives);release(d);return 1;}
    return 0;
failed:fprintf(stderr,"Render demo error %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.aa=1,.casts=1,.receives=1};for(int i=1;i<argc;++i){if(!strcmp(argv[i],"--frames")&&i+1<argc){d.frames=atoi(argv[++i]);if(d.frames<=0)return 2;}
        else if(!strcmp(argv[i],"--capture")&&i+1<argc)d.capture=argv[++i];
        else if(!strcmp(argv[i],"--aa")&&i+1<argc){const char *value=argv[++i];if(strcmp(value,"none")&&strcmp(value,"fxaa"))return 2;d.aa=!strcmp(value,"fxaa");}
        else if(!strcmp(argv[i],"--cast")&&i+1<argc){const char *value=argv[++i];if(strcmp(value,"0")&&strcmp(value,"1"))return 2;d.casts=*value=='1';}
        else if(!strcmp(argv[i],"--receive")&&i+1<argc){const char *value=argv[++i];if(strcmp(value,"0")&&strcmp(value,"1"))return 2;d.receives=*value=='1';}else return 2;}
    xge_desc_t window={.iWidth=960,.iHeight=640,.iTargetFPS=60,.sTitle="XGE render quality"};if(xgeInit(&window))return 1;
    int result=xgeRun(frame,&d);release(&d);xgeUnit();return result||d.failed?1:0;
}
