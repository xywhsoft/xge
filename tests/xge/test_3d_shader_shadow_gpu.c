#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"shader shadow %d: %s\n",__LINE__,#x);goto done;}}while(0)
static int failed=1;static unsigned char pixels[128*128*4],reference[sizeof(pixels)];
#if XGE3D_ENABLE_SHADER && XGE3D_ENABLE_SHADOW
static int render(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d)
{int result=xge3dRender(r,s,d,NULL);return result?result:xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),512);}
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;xge3d_material *ground=NULL,*caster=NULL;xge3d_shader *shader=NULL;
    CHECK(!xge3dSceneCreate(&s)&&!xge3dRendererCreate(&r)&&!xge3dTargetCreate(128,128,&target));
    xge3d_vertex_t v[4]={0};const float xy[][2]={{-1,-1},{1,-1},{1,1},{-1,1}};
    for(int i=0;i<4;++i){v[i].position=(xge3d_vec3_t){xy[i][0],xy[i][1],0};v[i].normal.z=1;
#if XGE3D_ENABLE_ANIMATION
        v[i].weights[0]=1;
#endif
    }uint16_t indices[]={0,1,2,0,2,3};xge3d_mesh_desc_t md={v,4,indices,6,16};CHECK(!xge3dMeshCreate(&md,&mesh));
    xge3d_node_t receiver,blocker,light;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&receiver)&&!xge3dNodeCreate(s,(xge3d_node_t){0},&blocker)&&!xge3dNodeCreate(s,(xge3d_node_t){0},&light));
    CHECK(!xge3dNodeSetMesh(s,receiver,mesh)&&!xge3dNodeSetMesh(s,blocker,mesh));xge3d_material_desc_t m=xge3dMaterialDefault();m.double_sided=1;m.metallic=0;
    CHECK(!xge3dMaterialCreate(&m,&ground)&&!xge3dNodeSetMaterial(s,receiver,ground));m.alpha_mode=XGE3D_ALPHA_MASK;CHECK(!xge3dMaterialCreate(&m,&caster)&&!xge3dNodeSetMaterial(s,blocker,caster));
    xge3d_transform_t ground_transform=XGE3D_TRANSFORM_IDENTITY,blocker_transform=ground_transform;ground_transform.scale=(xge3d_vec3_t){2,2,1};blocker_transform.position=(xge3d_vec3_t){-.5f,0,1};blocker_transform.scale=(xge3d_vec3_t){.25f,.25f,1};
    CHECK(!xge3dNodeSetTransform(s,receiver,&ground_transform)&&!xge3dNodeSetTransform(s,blocker,&blocker_transform));
    xge3d_light_desc_t sun=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);sun.direction=(xge3d_vec3_t){.5f,0,-1};sun.casts_shadow=1;CHECK(!xge3dNodeSetLight(s,light,&sun));
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,4,4,.1f,20)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}));
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=256;shadows.distance=10;shadows.depth_padding=5;
    xge3d_render_desc_t d={&camera,target,{0,0,0,1},3};d.shadows=&shadows;
    xge3d_shader_desc_t desc={.size=sizeof(desc),.bounds_margin=1,.vertex="void xge3dVertex(inout vec3 p,inout vec3 n,vec2 uv){p.x+=xge3dParams[0].x;}",
        .surface="void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){base.a*=xge3dParams[0].y;}"};
    CHECK(!xge3dShaderCreate(&desc,&shader));float params[16]={.5f,1};CHECK(!xge3dMaterialSetShader(caster,shader,params)&&!render(r,s,&d));memcpy(reference,pixels,sizeof(pixels));
    CHECK(!xge3dMaterialSetShader(caster,NULL,NULL));blocker_transform.position.x+=.125f;CHECK(!xge3dNodeSetTransform(s,blocker,&blocker_transform)&&!render(r,s,&d));
    CHECK(!memcmp(reference,pixels,sizeof(pixels)));
    blocker_transform.position.x-=.125f;CHECK(!xge3dNodeSetTransform(s,blocker,&blocker_transform));params[1]=0;
    CHECK(!xge3dMaterialSetShader(caster,shader,params)&&!render(r,s,&d));memcpy(reference,pixels,sizeof(pixels));CHECK(!xge3dNodeSetVisible(s,blocker,0)&&!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    CHECK(!xge3dNodeSetVisible(s,blocker,1)&&!xge3dMaterialSetShader(caster,NULL,NULL));params[0]=0;params[1]=1;
#if XGE3D_ENABLE_ANIMATION
    xge3d_node_t joint;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&joint));xge3d_mat4_t bind={{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
    xge3d_skin_binding_t skin={.joints=&joint,.inverse_bind=&bind,.count=1};CHECK(!xge3dNodeBindSkin(s,blocker,&skin));
    xge3d_transform_t pose=XGE3D_TRANSFORM_IDENTITY;pose.position=(xge3d_vec3_t){-.25f,0,1};CHECK(!xge3dNodeSetTransform(s,joint,&pose)&&!render(r,s,&d));memcpy(reference,pixels,sizeof(pixels));
    CHECK(!xge3dMaterialSetShader(caster,shader,params)&&!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
#endif
    failed=0;puts("Shader shadows: vertex displacement, alpha hooks and skinned deformation match independently placed/hidden/reference objects");
done:
    xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dMaterialFree(ground);xge3dMaterialFree(caster);xge3dShaderFree(shader);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
#endif
int main(void)
{
#if XGE3D_ENABLE_SHADER && XGE3D_ENABLE_SHADOW
    xge_desc_t d={.iWidth=160,.iHeight=160,.sTitle="XGE custom shader shadow checks"};if(xgeInit(&d)){return 1;}int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;
#else
    (void)pixels;(void)reference;(void)failed;puts("Shader/shadows trimmed");return 0;
#endif
}
