#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Application movement policy; the library only returns collision results. */
typedef struct demo {
    xge3d_scene *scene;xge3d_cache *cache;xge3d_renderer *renderer;xge3d_target *target;xge3d_animator *animator;
    xge3d_node_t player,wall;int frame,frames,hits,failed,automatic;const char *capture;
} demo;
static void release(demo *d)
{xge3dCacheFree(d->cache);d->cache=NULL;xge3dAnimatorFree(d->animator);d->animator=NULL;xge3dSceneFree(d->scene);d->scene=NULL;xge3dTargetFree(d->target);d->target=NULL;xge3dRendererFree(d->renderer);d->renderer=NULL;}
static int resolve(const char *uri,xge3d_model **out,void *user){return xge3dCacheLoad(user,uri,out);}
static int cube(xge3d_mesh **out)
{
    const xge3d_vec3_t normals[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    const xge3d_vec3_t us[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
    const xge3d_vec3_t vs[]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
    int corners[]={0,1,2,0,2,3},x[]={-1,1,1,-1},y[]={-1,-1,1,1};xge3d_vertex_t vertices[36]={0};
    for(int f=0;f<6;++f)for(int k=0;k<6;++k){int c=corners[k];vertices[f*6+k].normal=normals[f];
        vertices[f*6+k].position=(xge3d_vec3_t){.5f*(normals[f].x+us[f].x*x[c]+vs[f].x*y[c]),.5f*(normals[f].y+us[f].y*x[c]+vs[f].y*y[c]),.5f*(normals[f].z+us[f].z*x[c]+vs[f].z*y[c])};}
    xge3d_mesh_desc_t desc={vertices,36};return xge3dMeshCreate(&desc,out);
}
static int prepare(demo *d)
{
    int result=xge3dSceneCreate(&d->scene);if(!result)result=xge3dCacheCreate(NULL,&d->cache);
    xge3d_mesh *mesh=NULL;xge3d_material *material=NULL;if(!result)result=cube(&mesh);
    xge3d_material_desc_t md=xge3dMaterialDefault();md.metallic=0;md.roughness=.8f;md.emissive[0]=md.emissive[1]=md.emissive[2]=.035f;
    if(!result)result=xge3dMaterialCreate(&md,&material);
    char *directory=xrtPathAppDir(),*path=directory?xrtPathJoin(directory,"assets/explorer.gltf"):NULL;xrtFree(directory);
    if(!path)result=XGE_ERROR_OUT_OF_MEMORY;
    xge3d_scene_node_desc_t nodes[5];for(int i=0;i<5;++i)nodes[i]=xge3dSceneNodeDefault();
    nodes[0].name="floor";nodes[0].mesh=mesh;nodes[0].material=material;nodes[0].transform.position.y=-.1f;nodes[0].transform.scale=(xge3d_vec3_t){12,.2f,12};
    nodes[1].name="wall";nodes[1].mesh=mesh;nodes[1].material=material;nodes[1].transform.position.y=1;nodes[1].transform.scale=(xge3d_vec3_t){.6f,2,4};
    nodes[2].name="player";nodes[2].model_uri=path;nodes[2].transform.position.x=-3;
    nodes[3].name="shared-A";nodes[3].model_uri=path;nodes[3].transform.position=(xge3d_vec3_t){-3,0,-3};
    nodes[4].name="shared-B";nodes[4].model_uri=path;nodes[4].transform.position=(xge3d_vec3_t){3,0,-3};
    xge3d_scene_desc_t desc={nodes,5,resolve,d->cache};xge3d_node_t root={0},floor;
    if(!result)result=xge3dSceneLoad(d->scene,(xge3d_node_t){0},&desc,&root);
    xge3dMeshFree(mesh);xge3dMaterialFree(material);
    if(!result)result=xge3dNodeFind(d->scene,root,"player",&d->player);
    if(!result)result=xge3dNodeFind(d->scene,root,"wall",&d->wall);
    if(!result)result=xge3dNodeFind(d->scene,root,"floor",&floor);
    xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_BOX);
    if(!result)result=xge3dNodeSetCollider(d->scene,d->wall,&shape);
    if(!result)result=xge3dNodeSetCollider(d->scene,floor,&shape);
    float floor_color[]={.22f,.38f,.46f,1},wall_color[]={.8f,.5f,.18f,1};
    if(!result)result=xge3dNodeSetColor(d->scene,floor,floor_color);
    if(!result)result=xge3dNodeSetColor(d->scene,d->wall,wall_color);
    shape=xge3dShapeDefault(XGE3D_SHAPE_CAPSULE);shape.center.y=1;shape.radius=.25f;shape.half_height=.7f;
    if(!result)result=xge3dNodeSetCollider(d->scene,d->player,&shape);
    if(!result)result=xge3dNodeSetUserData(d->scene,d->player,d);
    xge3d_model *model=NULL;xge3d_clip *clip=NULL;
    if(!result)result=xge3dCacheLoad(d->cache,path,&model);
    if(!result)result=xge3dClipFromModel(model,1,&clip);
    if(!result)result=xge3dAnimatorCreate(d->scene,d->player,&d->animator);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();if(!result)result=xge3dAnimatorSetLayer(d->animator,0,clip,&layer);
    xge3dClipFree(clip);xge3dModelFree(model);xrtFree(path);
    xge3d_node_t sun;xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);light.direction=(xge3d_vec3_t){-.4f,-1,-.6f};light.intensity=3;light.casts_shadow=1;
    if(!result)result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&sun);
    if(!result)result=xge3dNodeSetLight(d->scene,sun,&light);
    return result;
}
static int frame(void *user)
{
    demo *d=user;int result=0;if(xgeGetWidth()<=0||xgeGetHeight()<=0)return 0;
    if(!d->renderer&&(result=xge3dRendererCreate(&d->renderer)))goto fail;
    float dt=d->automatic?1.f/60:fminf(.05f,fmaxf(.001f,xgeGetDelta()));
    if(d->frame==120){xge3d_node_t copy;if((result=xge3dNodeClone(d->scene,d->wall,(xge3d_node_t){0},0,&copy)))goto fail;
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){3,1,-.5f};t.scale=(xge3d_vec3_t){.6f,2,4};
        if((result=xge3dNodeSetTransform(d->scene,copy,&t))||(result=xge3dNodeSetName(d->scene,copy,"wall-copy")))goto fail;}
    xge3d_vec3_t motion={(float)(xgeKeyDown('D')-xgeKeyDown('A')),0,(float)(xgeKeyDown('S')-xgeKeyDown('W'))};
    if(d->automatic)motion=d->frame<120?(xge3d_vec3_t){1,0,0}:d->frame<240?(xge3d_vec3_t){0,0,1}:(xge3d_vec3_t){1,0,0};
    float length=sqrtf(motion.x*motion.x+motion.z*motion.z);if(length>0){motion.x*=2.1f*dt/length;motion.z*=2.1f*dt/length;}
    xge3d_mat4_t world;if((result=xge3dNodeGetWorldMatrix(d->scene,d->player,&world)))goto fail;
    xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_CAPSULE);shape.center.y=1;shape.radius=.25f;shape.half_height=.7f;
    xge3d_collision_filter_t filter={UINT32_MAX,d->player};xge3d_collision_hit_t hit;
    result=xge3dSceneSweepShape(d->scene,&shape,&world,motion,&filter,&hit);float fraction=1;
    if(!result){float distance=sqrtf(motion.x*motion.x+motion.z*motion.z);
        fraction=fmaxf(0,hit.fraction-(distance>0?.001f/distance:0));++d->hits;}
    else if(result!=XGE_ERROR_NOT_FOUND)goto fail;
    xge3d_mat4_t local;if((result=xge3dNodeGetLocalMatrix(d->scene,d->player,&local)))goto fail;
    local.m[12]+=motion.x*fraction;local.m[14]+=motion.z*fraction;
    if((result=xge3dNodeSetMatrix(d->scene,d->player,&local))||(result=xge3dAnimatorUpdate(d->animator,dt)))goto fail;
    xge3d_camera_t camera;if((result=xge3dCameraPerspective(&camera,.9f,(float)xgeGetWidth()/xgeGetHeight(),.1f,100))||
       (result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){8,7,11},(xge3d_vec3_t){0,1,0},(xge3d_vec3_t){0,1,0})))goto fail;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=1024;shadows.distance=35;shadows.normal_bias=.04f;
    xge3d_render_desc_t draw={&camera,NULL,{.13f,.21f,.27f,1},3};draw.shadows=&shadows;
    if((result=xge3dRender(d->renderer,d->scene,&draw,NULL)))goto fail;
    ++d->frame;if((d->frames&&d->frame>=d->frames)||xgeKeyPressed(XGE_KEY_ESCAPE)){
        if(d->capture){int w=xgeGetWidth(),h=xgeGetHeight();xarray pixels;xrtArrayInit(&pixels,1);
            result=xrtArrayResize(&pixels,(size_t)w*h*4)?xge3dTargetCreate(w,h,&d->target):XGE_ERROR_OUT_OF_MEMORY;
            draw.target=d->target;if(!result)result=xge3dRender(d->renderer,d->scene,&draw,NULL);
            if(!result)result=xge3dTargetReadPixels(d->target,pixels.Data,pixels.Count,w*4);
            if(!result){result=xgeImageSavePNG(d->capture,w,h,pixels.Data,w*4);}xrtArrayUnit(&pixels);if(result)goto fail;}
        xge3d_cache_stats_t stats={.size=sizeof(stats)};if((result=xge3dCacheStats(d->cache,&stats)))goto fail;
        if(d->automatic&&d->frames>=360&&(!d->hits||local.m[12]<1||stats.misses!=1||stats.hits<3)){
            fprintf(stderr,"demo counters: hits %d x %.3f misses %llu cache hits %llu\n",d->hits,local.m[12],(unsigned long long)stats.misses,(unsigned long long)stats.hits);result=XGE_ERROR_INVALID_STATE;goto fail;}
        printf("Core demo: %d frames, %d blocked sweeps, one asset/%llu cache hits, player x %.2f, cloned wall and scene-description load\n",d->frame,d->hits,(unsigned long long)stats.hits,local.m[12]);release(d);return 1;}
    return 0;
fail:fprintf(stderr,"Core demo failed %d at frame %d\n",result,d->frame);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={0};for(int i=1;i<argc;++i){if(!strcmp(argv[i],"--frames")&&i+1<argc)d.frames=atoi(argv[++i]);else if(!strcmp(argv[i],"--capture")&&i+1<argc)d.capture=argv[++i];else if(!strcmp(argv[i],"--automatic"))d.automatic=1;else return 2;}
    xge_desc_t window={0};window.iWidth=900;window.iHeight=600;window.iTargetFPS=60;window.sTitle="XGE core: WASD move, query wall, cached models and node clone, Esc exit";
    if(xgeInit(&window)){return 1;}int result=prepare(&d);if(!result){result=xgeRun(frame,&d);}release(&d);xgeUnit();return result||d.failed?1:0;
}
