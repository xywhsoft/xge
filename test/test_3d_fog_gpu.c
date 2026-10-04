#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
static int failed=1;
static int frame(void *user)
{
    (void)user;
    xge3d_scene *scene=NULL;xge3d_renderer *renderer=NULL;xge3d_target *target=NULL;
    xge3d_mesh *mesh=NULL;xge3d_material *material=NULL;unsigned char pixels[128*64*4],reference[sizeof(pixels)];
    CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dRendererCreate(&renderer)==XGE_OK && xge3dTargetCreate(128,64,&target)==XGE_OK);
    xge3d_vertex_t vertices[6]={0};const float xy[6][2]={{-.35f,-.35f},{.35f,-.35f},{.35f,.35f},{-.35f,-.35f},{.35f,.35f},{-.35f,.35f}};
    for (int i=0;i<6;++i) {vertices[i].position=(xge3d_vec3_t){xy[i][0],xy[i][1],0};vertices[i].normal.z=1;}
    xge3d_mesh_desc_t md={vertices,6};CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
    xge3d_material_desc_t desc=xge3dMaterialDefault();desc.unlit=1;desc.base_color[1]=desc.base_color[2]=0;
    CHECK(xge3dMaterialCreate(&desc,&material)==XGE_OK);
    for (int i=0;i<3;++i) {
        xge3d_node_t node;xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){i-1.f,0,-2.f-4*i};
        CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&node)==XGE_OK && xge3dNodeSetMesh(scene,node,mesh)==XGE_OK &&
              xge3dNodeSetMaterial(scene,node,material)==XGE_OK && xge3dNodeSetTransform(scene,node,&t)==XGE_OK);
    }
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,4,2,.1f,20)==XGE_OK &&
        xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,0},(xge3d_vec3_t){0,0,-1},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,reference,sizeof(reference),512)==XGE_OK);
#if XGE3D_ENABLE_FOG
    xge3d_fog_settings_t fog=xge3dFogDefault();fog.color=(xge3d_vec3_t){0,0,1};fog.start=2;fog.end=10;draw.fog=&fog;
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    const unsigned char *near=pixels+(32*128+32)*4,*middle=pixels+(32*128+64)*4,*far=pixels+(32*128+96)*4;
    CHECK(near[0]>252 && near[2]<3 && middle[0]>185 && middle[0]<190 && middle[2]>185 && middle[2]<190 && far[0]<3 && far[2]>252);
    draw.exposure=.5f;CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && far[2]>185 && far[2]<190);
    /* Perspective distances use forward depth rather than radial eye distance. */
    CHECK(xge3dCameraPerspective(&camera,1.1f,2,.1f,20)==XGE_OK);draw.exposure=1;
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixels[(32*128+69)*4]<3 && pixels[(32*128+69)*4+2]>252);
    fog.end=fog.start;CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    fog.end=NAN;CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    fog.end=10;fog.color.x=-1;CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    draw.fog=NULL;CHECK(xge3dCameraOrthographic(&camera,4,2,.1f,20)==XGE_OK && xge3dRender(renderer,scene,&draw,NULL)==XGE_OK &&
        xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && !memcmp(pixels,reference,sizeof(pixels)));
    puts("3D fog: near/middle/far linear colors, sRGB/exposure, perspective/orthographic depth, validation and disabled restoration passed");
#else
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK && !memcmp(pixels,reference,sizeof(pixels)));
    puts("3D fog trimmed: ordinary rendering passed");
#endif
    failed=0;
done:
    xge3dSceneFree(scene);xge3dMeshFree(mesh);xge3dMaterialFree(material);xge3dTargetFree(target);xge3dRendererFree(renderer);return 1;
}
int main(void)
{xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=64;desc.sTitle="XGE fog GPU verification";if (xgeInit(&desc)!=XGE_OK) return 1;int r=xgeRun(frame,NULL);xgeUnit();return r==XGE_OK ? failed : 1;}
