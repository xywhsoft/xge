#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRY(x) do{int result=(x);if(result)return result;}while(0)
typedef struct demo {
    xge3d_scene *scene;xge3d_renderer *renderer;xge3d_target *target;
    int tone,grade,bloom,fog,levels,frames,frame,failed;const char *capture;
} demo;
static int cube(xge3d_mesh **out)
{
    const xge3d_vec3_t ns[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const xge3d_vec3_t us[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const xge3d_vec3_t vs[]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
    const int cs[]={0,1,2,0,2,3},xs[]={-1,1,1,-1},ys[]={-1,-1,1,1};xge3d_vertex_t v[36]={0};
    for(int f=0;f<6;++f)for(int k=0;k<6;++k){int c=cs[k];v[f*6+k].normal=ns[f];
        v[f*6+k].position=(xge3d_vec3_t){.5f*(ns[f].x+us[f].x*xs[c]+vs[f].x*ys[c]),.5f*(ns[f].y+us[f].y*xs[c]+vs[f].y*ys[c]),.5f*(ns[f].z+us[f].z*xs[c]+vs[f].z*ys[c])};}
    xge3d_mesh_desc_t md={v,36};return xge3dMeshCreate(&md,out);
}
static int object(demo *d,xge3d_mesh *mesh,xge3d_vec3_t position,xge3d_vec3_t scale,const float *color,float emission)
{
    xge3d_node_t n;TRY(xge3dNodeCreate(d->scene,(xge3d_node_t){0},&n));xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=position;t.scale=scale;
    TRY(xge3dNodeSetTransform(d->scene,n,&t));TRY(xge3dNodeSetMesh(d->scene,n,mesh));
    xge3d_material_desc_t m=xge3dMaterialDefault();memcpy(m.base_color,color,sizeof(m.base_color));m.metallic=.05f;m.roughness=.6f;m.unlit=emission>0;
    for(int i=0;i<3;++i)m.emissive[i]=color[i]*emission;
    xge3d_material *material=NULL;TRY(xge3dMaterialCreate(&m,&material));int result=xge3dNodeSetMaterial(d->scene,n,material);xge3dMaterialFree(material);return result;
}
static int settings(demo *d)
{
    xge3d_color_settings_t color=xge3dColorDefault();if(!d->tone)color.tone_mapping=XGE3D_TONEMAP_NONE;
    if(d->grade){color.gain=(xge3d_vec3_t){1.04f,1,.94f};color.saturation=1.08f;color.contrast=1.04f;}
    TRY(xge3dRendererSetColor(d->renderer,d->tone||d->grade?&color:NULL));
    xge3d_bloom_settings_t bloom=xge3dBloomDefault();bloom.strength=.16f;bloom.levels=d->levels;TRY(xge3dRendererSetBloom(d->renderer,d->bloom?&bloom:NULL));
    xge3d_height_fog_settings_t fog=xge3dHeightFogDefault();fog.color=(xge3d_vec3_t){.12f,.22f,.32f};fog.height=1.2;fog.density=.035f;fog.falloff=.5f;fog.max_opacity=.9f;
    TRY(xge3dRendererSetHeightFog(d->renderer,d->fog?&fog:NULL));
    char title[200];snprintf(title,sizeof(title),"XGE | T tone %d | G grade %d | B bloom %d | H height fog %d | Esc exit",d->tone,d->grade,d->bloom,d->fog);xgeSetTitle(title);return XGE_OK;
}
static void release(demo *d)
{xge3dSceneFree(d->scene);d->scene=NULL;xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;}
static int prepare(demo *d)
{
    TRY(xge3dSceneCreate(&d->scene));TRY(xge3dRendererCreate(&d->renderer));TRY(xge3dRendererSetAntialiasing(d->renderer,XGE3D_AA_FXAA));
    xge3d_renderer_options_t options=xge3dRendererOptionsDefault();options.profiling=1;TRY(xge3dRendererSetOptions(d->renderer,&options));
    xge3d_mesh *mesh=NULL;TRY(cube(&mesh));int result=0;
    const float ground[]={.07f,.11f,.15f,1},wall[]={.3f,.15f,.045f,1},steel[]={.08f,.14f,.2f,1},warm[]={1,.25f,.025f,1},cool[]={.03f,.4f,1,1};
    result=object(d,mesh,(xge3d_vec3_t){0,-.15f,-20},(xge3d_vec3_t){30,.3f,70},ground,0);
    for(int i=0;i<8&&!result;++i){float z=3-i*6.f,h=2+(i%3)*1.2f;
        result=object(d,mesh,(xge3d_vec3_t){-4,h*.5f,z},(xge3d_vec3_t){3,h,3},wall,0);
        if(!result)result=object(d,mesh,(xge3d_vec3_t){4,h*.5f,z-2},(xge3d_vec3_t){3,h,3},steel,0);
        if(!result)result=object(d,mesh,(xge3d_vec3_t){-2.4f,1.6f,z+.4f},(xge3d_vec3_t){.2f,.6f,1.4f},warm,7);
        if(!result)result=object(d,mesh,(xge3d_vec3_t){2.4f,1.6f,z-1.5f},(xge3d_vec3_t){.2f,.6f,1.4f},cool,7);}
    xge3dMeshFree(mesh);if(result)return result;xge3d_node_t sun;TRY(xge3dNodeCreate(d->scene,(xge3d_node_t){0},&sun));
    xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){-.5f,-1,-.4f};light.intensity=2;light.casts_shadow=1;
    TRY(xge3dNodeSetLight(d->scene,sun,&light));return settings(d);
}
static int frame(void *user)
{
    demo *d=user;int result=0;if(!d->renderer){result=prepare(d);if(result)goto failed;}
    int changed=0;if(xgeKeyPressed('T')){d->tone=!d->tone;changed=1;}if(xgeKeyPressed('G')){d->grade=!d->grade;changed=1;}
    if(xgeKeyPressed('B')){d->bloom=!d->bloom;changed=1;}if(xgeKeyPressed('H')){d->fog=!d->fog;changed=1;}if(changed){result=settings(d);if(result)goto failed;}
    int w=xgeGetWidth(),h=xgeGetHeight();if(w<=0||h<=0)return 0;xge3d_camera_t camera;
    result=xge3dCameraPerspective(&camera,.75f,(float)w/h,.1f,150);if(result)goto failed;
    result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){8,5,14},(xge3d_vec3_t){0,1,-12},(xge3d_vec3_t){0,1,0});if(result)goto failed;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=1024;shadows.cascades=2;shadows.distance=60;shadows.normal_bias=.02f;
    xge3d_render_desc_t draw={&camera,NULL,{.05f,.09f,.14f,1},3};draw.shadows=&shadows;result=xge3dRender(d->renderer,d->scene,&draw,NULL);if(result)goto failed;++d->frame;
    if((d->frames&&d->frame>=d->frames)||xgeKeyPressed(XGE_KEY_ESCAPE)){
        if(d->capture){result=xge3dTargetCreate(w,h,&d->target);draw.target=d->target;xarray pixels;xrtArrayInit(&pixels,1);
            if(!result&&!xrtArrayResize(&pixels,(size_t)w*h*4))result=XGE_ERROR_OUT_OF_MEMORY;
            if(!result)result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if(!result)result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,w*4);
            if(!result)result=xgeImageSavePNGEx(d->capture,w,h,pixels.Data,w*4,XGE_IMAGE_PREMULTIPLIED);
            xrtArrayUnit(&pixels);if(result)goto failed;}
        xge3d_bloom_info_t bloom={.size=sizeof(bloom)};xge3d_render_profile_t p={.size=sizeof(p)};
        result=xge3dRendererBloomInfo(d->renderer,&bloom);if(result)goto failed;result=xge3dRendererProfile(d->renderer,&p);if(result)goto failed;
        printf("Light effects demo: %d frames, tone=%d grade=%d bloom=%d fog=%d, bloom %u passes/%llu bytes; GPU sample %llu ns\n",d->frame,d->tone,d->grade,d->bloom,d->fog,bloom.passes,(unsigned long long)bloom.storage_bytes,(unsigned long long)p.gpu_ns);
        release(d);return 1;}
    return 0;
failed:fprintf(stderr,"Light effects demo error %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.tone=1,.grade=1,.bloom=1,.fog=1,.levels=4};for(int i=1;i<argc;++i){
        if(!strcmp(argv[i],"--frames")&&i+1<argc){d.frames=atoi(argv[++i]);if(d.frames<=0)return 2;}
        else if(!strcmp(argv[i],"--capture")&&i+1<argc)d.capture=argv[++i];
        else if(!strcmp(argv[i],"--effects")&&i+1<argc){const char *v=argv[++i];if(strcmp(v,"all")&&strcmp(v,"none"))return 2;d.tone=d.grade=d.bloom=d.fog=!strcmp(v,"all");}
        else if(!strcmp(argv[i],"--levels")&&i+1<argc){d.levels=atoi(argv[++i]);if(d.levels<1||d.levels>6)return 2;}else return 2;}
    xge_desc_t window={.iWidth=960,.iHeight=640,.iTargetFPS=60,.sTitle="XGE light effects"};if(xgeInit(&window))return 1;
    int result=xgeRun(frame,&d);release(&d);xgeUnit();return result||d.failed?1:0;
}
