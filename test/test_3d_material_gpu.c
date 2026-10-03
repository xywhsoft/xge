#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[128*128*4];
static int pixel(int r,int g,int b,int a)
{
    const unsigned char *p=pixels+(64*128+64)*4;
    if (abs(p[0]-r)>2 || abs(p[1]-g)>2 || abs(p[2]-b)>2 || abs(p[3]-a)>2) {
        fprintf(stderr,"center RGBA=%u,%u,%u,%u; expected=%d,%d,%d,%d\n",p[0],p[1],p[2],p[3],r,g,b,a); return 0;
    }
    return 1;
}
static int color_factor(int v,float factor)
{
    float c=v/255.0f, linear=c<=.04045f ? c/12.92f : powf((c+.055f)/1.055f,2.4f);
    linear*=factor;
    c=linear<=.0031308f ? linear*12.92f : 1.055f*powf(linear,1/2.4f)-.055f;
    return (int)lroundf(c*255);
}
static int material(xge3d_scene *s,xge3d_node_t n,const xge3d_material_desc_t *desc)
{
    xge3d_material *m=NULL;
    int result=xge3dMaterialCreate(desc,&m);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(s,n,m);
    xge3dMaterialFree(m); return result;
}
static int texture(unsigned char rgba[4],int srgb,xge3d_texture **out)
{
    xge_image_t image={1,1,XGE_PIXEL_RGBA8,4,rgba,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t desc={&image,srgb,{0}};
    return xge3dTextureCreate(&desc,out);
}
static int frame(void *user)
{
    (void)user;
    xge3d_scene *scene=NULL; xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL; xge3d_mesh *mesh=NULL;
    xge3d_texture *tex=NULL,*red=NULL,*blue=NULL;
    xge3d_node_t node,far_node;
    xge3d_vertex_t vertices[4]={0};
    vertices[0].position=(xge3d_vec3_t){-.8f,-.8f,0}; vertices[1].position=(xge3d_vec3_t){.8f,-.8f,0};
    vertices[2].position=(xge3d_vec3_t){.8f,.8f,0}; vertices[3].position=(xge3d_vec3_t){-.8f,.8f,0};
    uint16_t indices[]={0,1,2,0,2,3};
    xge3d_mesh_desc_t md={vertices,4,indices,6,16};
    CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dRendererCreate(&renderer)==XGE_OK);
    CHECK(xge3dTargetCreate(128,128,&target)==XGE_OK && xge3dMeshCreate(&md,&mesh)==XGE_OK);
    CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&node)==XGE_OK && xge3dNodeSetMesh(scene,node,mesh)==XGE_OK);
    xge3d_camera_t camera;
    CHECK(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};
    unsigned char rgba[]={128,64,32,255};
    CHECK(texture(rgba,1,&tex)==XGE_OK);
    memset(rgba,0,sizeof(rgba)); /* Texture owns a copy before GPU upload. */
    xge3d_material_desc_t d=xge3dMaterialDefault(); d.unlit=1; d.maps[0].texture=tex;
    CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(128,64,32,255));
    d.base_color[0]=d.base_color[1]=d.base_color[2]=.5f;
    CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(color_factor(128,.5f),color_factor(64,.5f),color_factor(32,.5f),255));
    xge3dTextureFree(tex); tex=NULL;
    unsigned char tile[]={255,0,0,255,0,0,255,255};
    xge_image_t tile_image={2,1,XGE_PIXEL_RGBA8,8,tile,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t td={&tile_image,1,{9728,9728,33071,33071}};
    CHECK(xge3dTextureCreate(&td,&tex)==XGE_OK);
    for (int i=0;i<4;++i) vertices[i].uv1[0]=.75f;
    CHECK(xge3dMeshUpdate(mesh,&md)==XGE_OK);
    d=xge3dMaterialDefault(); d.unlit=1; d.maps[0].texture=tex; d.maps[0].texcoord=1;
    CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(0,0,255,255));
    d.maps[0].texcoord=0; d.maps[0].offset[0]=.75f; d.maps[0].rotation=.5f;
    CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(0,0,255,255));
    xge3dTextureFree(tex); tex=NULL;
    unsigned char red_pixel[]={255,0,0,64};
    CHECK(texture(red_pixel,1,&red)==XGE_OK);
    d=xge3dMaterialDefault(); d.unlit=1; d.maps[0].texture=red; d.alpha_mode=XGE3D_ALPHA_MASK;
    draw.clear_color[2]=1;
    CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(0,0,255,255));
    d.alpha_cutoff=.2f; CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(255,0,0,255));
    /* Reverse winding: back face disappears, then double-sided brings it back. */
    indices[1]=2; indices[2]=1; indices[4]=3; indices[5]=2;
    CHECK(xge3dMeshUpdate(mesh,&md)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(0,0,255,255));
    d.double_sided=1; CHECK(material(scene,node,&d)==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(255,0,0,255));
    xge3dTextureFree(red); red=NULL;
    red_pixel[3]=128; unsigned char blue_pixel[]={0,0,255,128};
    CHECK(texture(red_pixel,1,&red)==XGE_OK && texture(blue_pixel,1,&blue)==XGE_OK);
    d=xge3dMaterialDefault(); d.unlit=1; d.double_sided=1; d.alpha_mode=XGE3D_ALPHA_BLEND;
    d.maps[0].texture=red; CHECK(material(scene,node,&d)==XGE_OK);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY; t.position.z=.1f;
    CHECK(xge3dNodeSetTransform(scene,node,&t)==XGE_OK);
    CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&far_node)==XGE_OK && xge3dNodeSetMesh(scene,far_node,mesh)==XGE_OK);
    d.maps[0].texture=blue; CHECK(material(scene,far_node,&d)==XGE_OK);
    t.position.z=-.1f; CHECK(xge3dNodeSetTransform(scene,far_node,&t)==XGE_OK);
    draw.clear_color[2]=0; draw.clear_color[3]=0;
    CHECK(xge3dRender(renderer,scene,&draw,NULL)==XGE_OK && xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixel(128,0,64,192));
    CHECK(xgeImageSavePNGEx("artifacts/xge-3d/p2-alpha.png",128,128,pixels,512,XGE_IMAGE_PREMULTIPLIED)==XGE_OK);
    failed=0;
    puts("3D materials GPU: copied straight pixels, sRGB roundtrip and linear factors, alpha mask, double sides and back-to-front blend order passed");
done:
    xge3dSceneFree(scene); xge3dMeshFree(mesh); xge3dTargetFree(target); xge3dRendererFree(renderer);
    xge3dTextureFree(tex); xge3dTextureFree(red); xge3dTextureFree(blue);
    return 1;
}
int main(void)
{
    xge_desc_t desc={0}; desc.iWidth=180; desc.iHeight=180; desc.sTitle="XGE material GPU verification";
    if (xgeInit(&desc)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL); xgeUnit();
    return result==XGE_OK ? failed : 1;
}
