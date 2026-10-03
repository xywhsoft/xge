#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[128*128*4];
static int center(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d)
{
    if (xge3dRender(r,s,d,NULL)!=XGE_OK || xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),512)!=XGE_OK) return -1;
    return pixels[(64*128+64)*4];
}
static int encode(float c)
{ return (int)lroundf(255*(c<=.0031308f ? c*12.92f : 1.055f*powf(c,1/2.4f)-.055f)); }
static int set_material(xge3d_scene *s,xge3d_node_t n,xge3d_material_desc_t *d)
{
    xge3d_material *m=NULL; int result=xge3dMaterialCreate(d,&m);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(s,n,m);
    xge3dMaterialFree(m); return result;
}
static int frame(void *user)
{
    (void)user;
    xge3d_scene *s=NULL; xge3d_renderer *r=NULL; xge3d_target *target=NULL;
    xge3d_mesh *mesh=NULL; xge3d_texture *normal=NULL,*mr=NULL;
    xge3d_node_t quad,light;
    xge3d_vertex_t v[4]={0};
    v[0].position=(xge3d_vec3_t){-1,-1,0};v[1].position=(xge3d_vec3_t){1,-1,0};
    v[2].position=(xge3d_vec3_t){1,1,0};v[3].position=(xge3d_vec3_t){-1,1,0};
    for (int i=0;i<4;++i) { v[i].normal.z=1; v[i].uv[0]=i==1 || i==2; v[i].uv[1]=i>=2; }
    uint16_t indices[]={0,1,2,0,2,3}; xge3d_mesh_desc_t md={v,4,indices,6,16};
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(128,128,&target)==XGE_OK);
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&quad)==XGE_OK);
    CHECK(xge3dNodeSetMesh(s,quad,mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);
    xge3d_material_desc_t mat=xge3dMaterialDefault(); mat.metallic=0;
    CHECK(set_material(s,quad,&mat)==XGE_OK);
    xge3d_camera_t camera; CHECK(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};
    CHECK(center(r,s,&draw)==0);
    xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    int diffuse=encode(.96f/3.14159265f+.04f/(4*3.14159265f));
    CHECK(abs(center(r,s,&draw)-diffuse)<=2);
    ld.direction.z=1; CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK && center(r,s,&draw)==0);
    ld.direction.z=-1; ld.intensity=.5f;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK && abs(center(r,s,&draw)-encode(.5f*(.96f/3.14159265f+.04f/(4*3.14159265f))))<=2);
#if XGE3D_ENABLE_MODEL
    xge3d_model *source=NULL; xge3d_node_t imported;
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/lights.gltf",&source)==XGE_OK);
    CHECK(xge3dModelInstantiate(s,source,(xge3d_node_t){0},&imported)==XGE_OK);
    xge3dModelFree(source);
    CHECK(xge3dNodeSetVisible(s,light,0)==XGE_OK && abs(center(r,s,&draw)-diffuse)<=2);
    CHECK(xge3dNodeDestroy(s,imported)==XGE_OK && xge3dNodeSetVisible(s,light,1)==XGE_OK);
#endif
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY; t.position.z=2;
    CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK);
    ld=xge3dLightDefault(XGE3D_LIGHT_POINT); ld.range=0;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    int near=center(r,s,&draw); CHECK(abs(near-encode(.25f*(.96f/3.14159265f+.04f/(4*3.14159265f))))<=2);
    t.position.z=4; CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK && center(r,s,&draw)<near-20);
    ld.range=1; CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK && center(r,s,&draw)==0);
    ld=xge3dLightDefault(XGE3D_LIGHT_SPOT);t.position.z=2;
    CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK && xge3dNodeSetLight(s,light,&ld)==XGE_OK && abs(center(r,s,&draw)-near)<=2);
    t.position.x=2;CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK && center(r,s,&draw)==0);
    t.position.x=0; CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK);
    ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL); CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    /* Linear normal texture, derivative TBN, scale zero, metallic and roughness maps. */
    unsigned char n[]={255,128,128,255}; xge_image_t image={1,1,XGE_PIXEL_RGBA8,4,n,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t td={&image,0,{0}};
    CHECK(xge3dTextureCreate(&td,&normal)==XGE_OK); mat.maps[2].texture=normal;
    CHECK(set_material(s,quad,&mat)==XGE_OK && center(r,s,&draw)<30);
    mat.normal_scale=0; CHECK(set_material(s,quad,&mat)==XGE_OK && abs(center(r,s,&draw)-diffuse)<=2);
    mat.maps[2].texture=NULL; mat.metallic=1;
    CHECK(set_material(s,quad,&mat)==XGE_OK && abs(center(r,s,&draw)-encode(1/(4*3.14159265f)))<=2);
    unsigned char packed[]={0,255,0,255}; image.pPixels=packed;
    CHECK(xge3dTextureCreate(&td,&mr)==XGE_OK);mat.maps[1].texture=mr;
    CHECK(set_material(s,quad,&mat)==XGE_OK && abs(center(r,s,&draw)-diffuse)<=2);
    mat.maps[1].texture=NULL;mat.metallic=0; mat.roughness=.25f;ld.intensity=.05f;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK && set_material(s,quad,&mat)==XGE_OK);
    int smooth=center(r,s,&draw); mat.roughness=1;
    CHECK(set_material(s,quad,&mat)==XGE_OK && center(r,s,&draw)<smooth-20);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY; t.scale=(xge3d_vec3_t){-2,3,1};
    CHECK(xge3dNodeSetTransform(s,quad,&t)==XGE_OK && center(r,s,&draw)>0);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p3-light.png",128,128,pixels,512)==XGE_OK);
    ld.range=-1; CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_ERROR_INVALID_ARGUMENT);
    for (int i=1;i<XGE3D_MAX_LIGHTS;++i) {
        xge3d_node_t extra; CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&extra)==XGE_OK);
        ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL); CHECK(xge3dNodeSetLight(s,extra,&ld)==XGE_OK);
    }
    CHECK(center(r,s,&draw)>=0);
    xge3d_node_t overflow; CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&overflow)==XGE_OK && xge3dNodeSetLight(s,overflow,&ld)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,NULL)==XGE_ERROR_UNSUPPORTED);
    CHECK(xge3dNodeSetVisible(s,overflow,0)==XGE_OK && center(r,s,&draw)>=0);
    failed=0; puts("3D lights GPU: directional PBR reference, no-light/backlight, inverse square/range, spot cone, normal scale, metallic/roughness maps, mirrored scale and light limit passed");
done:
    xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dTargetFree(target);xge3dRendererFree(r);
    xge3dTextureFree(normal);xge3dTextureFree(mr);return 1;
}
int main(void)
{
    xge_desc_t d={0};d.iWidth=d.iHeight=180;d.sTitle="XGE light GPU verification";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;
}
