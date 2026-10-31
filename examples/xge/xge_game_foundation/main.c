#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct demo {
    xge3d_scene *scene,*screen_scene;xge3d_renderer *renderer,*screen_renderer;xge3d_target *screen,*capture_target;
    xge3d_mesh *cube;xge3d_material *floor_material,*player_material,*wall_material,*effect_material,*screen_material,*panel_material;
    xge3d_texture *floor_texture,*screen_view;xge3d_shader *shader;xge3d_particles *particles;xge_actions *actions;
    xge3d_node_t player,walls[2];xge3d_vec3_t position;float vertical,time;int frame,frames,failed,debug,blocked;const char *capture;
} demo;
static void release(demo *d)
{
    xge3dRendererFree(d->renderer);d->renderer=NULL;xge3dRendererFree(d->screen_renderer);d->screen_renderer=NULL;
    xge3dSceneFree(d->scene);d->scene=NULL;xge3dSceneFree(d->screen_scene);d->screen_scene=NULL;
    xge3dMaterialFree(d->floor_material);d->floor_material=NULL;xge3dMaterialFree(d->player_material);d->player_material=NULL;
    xge3dMaterialFree(d->wall_material);d->wall_material=NULL;xge3dMaterialFree(d->effect_material);d->effect_material=NULL;
    xge3dMaterialFree(d->screen_material);d->screen_material=NULL;xge3dMaterialFree(d->panel_material);d->panel_material=NULL;
    xge3dTextureFree(d->floor_texture);d->floor_texture=NULL;xge3dTextureFree(d->screen_view);d->screen_view=NULL;
    xge3dShaderFree(d->shader);d->shader=NULL;xge3dMeshFree(d->cube);d->cube=NULL;
    xge3dTargetFree(d->screen);d->screen=NULL;xge3dTargetFree(d->capture_target);d->capture_target=NULL;
    xge3dParticlesFree(d->particles);d->particles=NULL;xgeActionsFree(d->actions);d->actions=NULL;
}
static int cube(xge3d_mesh **out)
{
    const float points[8][3]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},{-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int faces[6][4]={{4,5,6,7},{1,0,3,2},{5,1,2,6},{0,4,7,3},{7,6,2,3},{0,1,5,4}},tri[6]={0,1,2,0,2,3};
    const xge3d_vec3_t normals[6]={{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}};const float uv[4][2]={{0,1},{1,1},{1,0},{0,0}};xge3d_vertex_t v[36]={0};
    for(int face=0;face<6;++face){for(int j=0;j<6;++j){int k=tri[j];memcpy(&v[face*6+j].position,points[faces[face][k]],sizeof(xge3d_vec3_t));v[face*6+j].normal=normals[face];memcpy(v[face*6+j].uv,uv[k],sizeof(uv[k]));}}
    xge3d_mesh_desc_t md={v,36};return xge3dMeshCreate(&md,out);
}
static int material(xge3d_material **out,float r,float g,float b,int unlit)
{xge3d_material_desc_t m=xge3dMaterialDefault();m.base_color[0]=r;m.base_color[1]=g;m.base_color[2]=b;m.metallic=0;m.roughness=.8f;m.unlit=unlit;return xge3dMaterialCreate(&m,out);}
static int object(demo *d,xge3d_scene *scene,xge3d_material *m,xge3d_vec3_t p,xge3d_vec3_t scale,int collider,xge3d_node_t *out)
{
    xge3d_node_t node;int result=xge3dNodeCreate(scene,(xge3d_node_t){0},&node);if(result){return result;}xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=p;t.scale=scale;
    result=xge3dNodeSetTransform(scene,node,&t);if(!result){result=xge3dNodeSetMesh(scene,node,d->cube);}if(!result){result=xge3dNodeSetMaterial(scene,node,m);}
    if(!result&&collider){xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_BOX);result=xge3dNodeSetCollider(scene,node,&shape);}if(out){*out=node;}return result;
}
static int prepare(demo *d)
{
    int result=xge3dRendererCreate(&d->renderer);if(!result){result=xge3dRendererCreate(&d->screen_renderer);}if(!result){result=xge3dSceneCreate(&d->scene);}if(!result){result=xge3dSceneCreate(&d->screen_scene);}if(!result){result=cube(&d->cube);}if(result){return result;}
    result=material(&d->floor_material,.7f,.75f,.8f,0);if(!result){result=material(&d->player_material,.1f,.8f,.4f,0);}if(!result){result=material(&d->wall_material,.55f,.3f,.16f,0);}
    if(!result){result=material(&d->effect_material,.5f,.55f,1,0);}if(!result){result=material(&d->screen_material,1,1,1,1);}if(!result){result=material(&d->panel_material,1,1,1,1);}if(result){return result;}
    unsigned char pixels[16*16*4];for(int y=0;y<16;++y){for(int x=0;x<16;++x){unsigned char *p=pixels+(y*16+x)*4;p[0]=p[1]=p[2]=(unsigned char)(((x/2+y/2)&1)?220:100);p[3]=255;}}
    xge_image_t image={.iWidth=16,.iHeight=16,.iStride=64,.iFormat=XGE_PIXEL_RGBA8,.pPixels=pixels};xge3d_texture_desc_t td={.image=&image,.srgb=1};result=xge3dTextureCreate(&td,&d->floor_texture);
    xge3d_material_desc_t m;if(!result){result=xge3dMaterialGetDesc(d->floor_material,&m);}m.maps[0].texture=d->floor_texture;m.maps[0].scale[0]=m.maps[0].scale[1]=4;
    if(!result){result=xge3dMaterialSetDesc(d->floor_material,&m);}if(result){return result;}
    result=object(d,d->scene,d->floor_material,(xge3d_vec3_t){0,-.1f,0},(xge3d_vec3_t){14,.2f,14},1,NULL);
    if(!result){result=object(d,d->scene,d->wall_material,(xge3d_vec3_t){2,.6f,0},(xge3d_vec3_t){.4f,1.2f,6},1,d->walls);}
    if(!result){result=object(d,d->scene,d->wall_material,(xge3d_vec3_t){0,.6f,2},(xge3d_vec3_t){6,1.2f,.4f},1,d->walls+1);}
    d->position=(xge3d_vec3_t){-1,.801f,0};if(!result){result=object(d,d->scene,d->player_material,d->position,(xge3d_vec3_t){.6f,1.6f,.6f},0,&d->player);}
    if(!result){result=object(d,d->scene,d->effect_material,(xge3d_vec3_t){3,1.2f,-2},(xge3d_vec3_t){1,2.4f,1},0,NULL);}if(result){return result;}
    xge3d_shader_desc_t shader={.size=sizeof(shader),.bounds_margin=.15f,
        .vertex="void xge3dVertex(inout vec3 p,inout vec3 n,vec2 uv){float a=p.y*2.0+xge3dParams[0].x;n.y-=n.x*cos(a)*xge3dParams[0].y*2.0;p.x+=sin(a)*xge3dParams[0].y;}",
        .surface="void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){base.rgb*=vec3(0.65+0.35*sin(uv.x*8.0+xge3dParams[0].x),0.7+0.3*sin(uv.y*7.0),1.0);}"};
    result=xge3dShaderCreate(&shader,&d->shader);if(!result){result=xge3dTargetCreate(256,128,&d->screen);}if(!result){result=xge3dTextureFromTarget(d->screen,&d->screen_view);}
    if(!result){result=xge3dMaterialGetDesc(d->panel_material,&m);}m.maps[0].texture=d->screen_view;if(!result){result=xge3dMaterialSetDesc(d->panel_material,&m);}
    if(!result){result=object(d,d->scene,d->panel_material,(xge3d_vec3_t){-3,1.6f,-2},(xge3d_vec3_t){2.4f,1.2f,.1f},0,NULL);}
    if(!result){result=object(d,d->screen_scene,d->screen_material,(xge3d_vec3_t){0},(xge3d_vec3_t){2,1,.1f},0,NULL);}if(result){return result;}
    xge3d_node_t light;result=xge3dNodeCreate(d->scene,(xge3d_node_t){0},&light);xge3d_light_desc_t sun=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);sun.direction=(xge3d_vec3_t){-.5f,-1,-.8f};sun.intensity=4;sun.casts_shadow=1;
    if(!result){result=xge3dNodeSetLight(d->scene,light,&sun);}xge3d_color_settings_t color=xge3dColorDefault();if(!result){result=xge3dRendererSetColor(d->renderer,&color);}
    xge3d_particle_desc_t particles=xge3dParticlesDefault();particles.capacity=128;particles.rate=40;particles.position=d->position;particles.speed_min=.5f;particles.speed_max=1.5f;particles.size_start=.12f;particles.life_min=.3f;particles.life_max=.9f;
    if(!result){result=xge3dParticlesCreate(&particles,&d->particles);}if(!result){result=xgeActionsCreate(2,8,&d->actions);}if(result){return result;}
    const uint32_t keys[4]={'A','D','W','S'};for(int i=0;i<4;++i){xge_action_binding_t b={XGE_ACTION_KEY,0,keys[i],i%2?1:-1};result=xgeActionsBind(d->actions,i/2,&b);if(result){return result;}}
    for(int i=0;i<2;++i){xge_action_binding_t b={XGE_ACTION_GAMEPAD_AXIS,-1,(uint32_t)i,1};result=xgeActionsBind(d->actions,i,&b);if(!result){result=xgeActionsSetDeadzone(d->actions,i,.15f);}if(result){return result;}}
    return XGE_OK;
}
static int frame(void *user)
{
    demo *d=user;int result=0;if(!d->renderer){result=prepare(d);if(result){goto failed;}}
    float dt=d->frames?1.0f/60:fminf(.0333f,fmaxf(0,xgeGetDelta()));d->time+=dt;result=xgeActionsUpdate(d->actions,NULL,NULL);if(result){goto failed;}
    xge_action_state_t x,z;result=xgeActionsGet(d->actions,0,&x);if(!result){result=xgeActionsGet(d->actions,1,&z);}if(result){goto failed;}
    if(d->frames){x.value=d->frame<120?1:0;z.value=d->frame>=120?1:0;}
    xge3d_shape_t body=xge3dShapeDefault(XGE3D_SHAPE_BOX);body.half_extents=(xge3d_vec3_t){.3f,.8f,.3f};xge3d_mat4_t world={{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};world.m[12]=d->position.x;world.m[13]=d->position.y;world.m[14]=d->position.z;
    d->vertical-=9.8f*dt;xge3d_motion_result_t motion={.size=sizeof(motion)};result=xge3dSceneMoveShape(d->scene,&body,&world,(xge3d_vec3_t){x.value*3*dt,d->vertical*dt,z.value*3*dt},NULL,NULL,&motion);if(result){goto failed;}
    d->position.x+=motion.displacement.x;d->position.y+=motion.displacement.y;d->position.z+=motion.displacement.z;
    for(int i=0;i<motion.contact_count;++i){if(motion.normals[i].y>.5f){d->vertical=0;}if(fabsf(motion.normals[i].y)<.5f){++d->blocked;}}
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=d->position;t.scale=(xge3d_vec3_t){.6f,1.6f,.6f};result=xge3dNodeSetTransform(d->scene,d->player,&t);
    float parameters[16]={d->time,.12f};if(!result){result=xge3dMaterialSetShader(d->effect_material,d->shader,parameters);}parameters[1]=0;if(!result){result=xge3dMaterialSetShader(d->screen_material,d->shader,parameters);}if(result){goto failed;}
    unsigned char patch[16];for(int i=0;i<4;++i){patch[i*4]=32;patch[i*4+1]=(unsigned char)(160+80*sinf(d->time*2));patch[i*4+2]=255;patch[i*4+3]=255;}
    xge3d_vec3_t emitter=d->position;emitter.y+=.9f;
    result=xge3dTextureUpdate(d->floor_texture,0,0,2,2,patch,8);if(!result){result=xge3dParticlesSetPosition(d->particles,emitter);}if(!result){result=xge3dParticlesUpdate(d->particles,dt,1);}
    xge3d_billboard_t billboards[128];size_t count;if(!result){result=xge3dParticlesBillboards(d->particles,NULL,XGE3D_BILLBOARD_ADD,billboards,128,&count);}if(!result){result=xge3dBillboardsSet(d->renderer,billboards,count);}if(result){goto failed;}
    if(xgeKeyPressed(XGE_KEY_F1)){d->debug=!d->debug;}xge3dDebugClear(d->renderer);
    if(d->debug){const float green[]={.1f,1,.3f,1},yellow[]={1,.8f,.1f,1};world.m[12]=d->position.x;world.m[13]=d->position.y;world.m[14]=d->position.z;
        result=xge3dDebugShape(d->renderer,&body,&world,green,0);for(int i=0;i<2&&!result;++i){xge3d_shape_t wall=xge3dShapeDefault(XGE3D_SHAPE_BOX);result=xge3dNodeGetWorldMatrix(d->scene,d->walls[i],&world);if(!result){result=xge3dDebugShape(d->renderer,&wall,&world,yellow,1);}}if(result){goto failed;}}
    xge3d_camera_t screen_camera;result=xge3dCameraOrthographic(&screen_camera,2,1,.1f,10);if(!result){result=xge3dCameraLookAt(&screen_camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0});}
    xge3d_render_desc_t screen_draw={&screen_camera,d->screen,{0,0,0,1},3};if(!result){result=xge3dRender(d->screen_renderer,d->screen_scene,&screen_draw,NULL);}if(result){goto failed;}
    int width=xgeGetWidth(),height=xgeGetHeight();if(width<=0||height<=0){return 0;}xge3d_camera_t camera;
    result=xge3dCameraPerspective(&camera,.8f,(float)width/height,.1f,80);if(!result){result=xge3dCameraLookAt(&camera,(xge3d_vec3_t){8,7,10},(xge3d_vec3_t){0,.7f,0},(xge3d_vec3_t){0,1,0});}
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=1024;shadows.cascades=2;shadows.distance=35;
    xge3d_render_desc_t draw={&camera,NULL,{.08f,.13f,.2f,1},3};draw.shadows=&shadows;if(!result){result=xge3dRender(d->renderer,d->scene,&draw,NULL);}if(result){goto failed;}++d->frame;
    if((d->frames&&d->frame>=d->frames)||xgeKeyPressed(XGE_KEY_ESCAPE)){
        if(d->capture){xarray rgba;xrtArrayInit(&rgba,1);result=xge3dTargetCreate(width,height,&d->capture_target);draw.target=d->capture_target;
            if(!result&&!xrtArrayResize(&rgba,(size_t)width*height*4)){result=XGE_ERROR_OUT_OF_MEMORY;}if(!result){result=xge3dRender(d->renderer,d->scene,&draw,NULL);}
            if(!result){result=xge3dTargetReadPixels(d->capture_target,rgba.Data,rgba.Count,width*4);}if(!result){result=xgeImageSavePNGEx(d->capture,width,height,rgba.Data,width*4,XGE_IMAGE_PREMULTIPLIED);}xrtArrayUnit(&rgba);if(result){goto failed;}}
        xge3d_particle_info_t info={.size=sizeof(info)};result=xge3dParticlesInfo(d->particles,&info);if(result){goto failed;}
        printf("Foundation demo: %d frames, %d wall contacts, position %.3f %.3f %.3f, %u live particles, all six public C mechanisms active\n",d->frame,d->blocked,d->position.x,d->position.y,d->position.z,info.alive);release(d);return 1;}
    return 0;
failed:fprintf(stderr,"Foundation demo failed: %d\n",result);d->failed=1;release(d);return 1;
}
int main(int argc,char **argv)
{
    demo d={.debug=1};for(int i=1;i<argc;++i){if(!strcmp(argv[i],"--frames")&&i+1<argc){d.frames=atoi(argv[++i]);if(d.frames<=0){return 2;}}else if(!strcmp(argv[i],"--capture")&&i+1<argc){d.capture=argv[++i];}else return 2;}
    xge_desc_t window={.iWidth=960,.iHeight=640,.iTargetFPS=60,.sTitle="XGE foundation | WASD / gamepad | F1 debug | Esc exit"};if(xgeInit(&window)){return 1;}
    int result=xgeRun(frame,&d);release(&d);xgeUnit();return result||d.failed?1:0;
}
