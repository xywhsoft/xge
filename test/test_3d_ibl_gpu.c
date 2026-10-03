#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[64*64*4];
static int render(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d,xge3d_render_stats_t *stats)
{
    int result=xge3dRender(r,s,d,stats);
    if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),256);
    return result==XGE_OK ? pixels[(32*64+32)*4] : -1;
}
static int material(xge3d_scene *s,xge3d_node_t n,xge3d_material_desc_t *d)
{
    xge3d_material *m=NULL;int result=xge3dMaterialCreate(d,&m);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(s,n,m);
    xge3dMaterialFree(m);return result;
}
static int frame(void *user)
{
    (void)user;
    xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;
    xge3d_environment *env=NULL;xge3d_texture *occlusion=NULL;xge_image_t images[25]={0};
    const char *faces[]={"px","nx","py","ny","pz","nz"};
    xge3d_ibl_desc_t ibl={0};xge3d_cube_level_t levels[3]={0};ibl.prefiltered=levels;ibl.level_count=3;
    for (int f=0;f<6;++f) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/ibl-irr-%s.png",faces[f]);
        CHECK(xgeImageLoadEx(&images[f],path,XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);ibl.irradiance.faces[f]=&images[f];
        for (int l=0;l<3;++l) {
            snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/ibl-spec-%d-%s.png",l,faces[f]);
            CHECK(xgeImageLoadEx(&images[6+l*6+f],path,XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);levels[l].faces[f]=&images[6+l*6+f];
        }
    }
    CHECK(xgeImageLoadEx(&images[24],"artifacts/xge-3d/fixtures/ibl-brdf.png",XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);ibl.brdf=&images[24];
    unsigned char white[]={255,255,255,255};xge_image_t sky_image={1,1,XGE_PIXEL_RGBA8,4,white,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_environment_desc_t sky={0};sky.srgb=1;for (int f=0;f<6;++f) sky.faces[f]=&sky_image;
    CHECK(xge3dEnvironmentCreate(&sky,&env)==XGE_OK && xge3dEnvironmentSetIBL(env,&ibl)==XGE_OK);
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(64,64,&target)==XGE_OK);
    xge3d_vertex_t v[3]={0};v[0].position=(xge3d_vec3_t){-2,-2,0};v[1].position=(xge3d_vec3_t){2,-2,0};v[2].position=(xge3d_vec3_t){0,2,0};
    for (int i=0;i<3;++i) v[i].normal.z=1;
    xge3d_mesh_desc_t md={v,3,NULL,0,0};xge3d_node_t n;
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&n)==XGE_OK && xge3dNodeSetMesh(s,n,mesh)==XGE_OK);
    xge3d_material_desc_t mat=xge3dMaterialDefault();mat.metallic=0;CHECK(material(s,n,&mat)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t d={&camera,target,{0,0,0,1},3,env};d.ibl_intensity=1;
    xge3d_render_stats_t stats;int diffuse=render(r,s,&d,&stats);CHECK(diffuse>=248 && stats.upload_bytes>10000);
    CHECK(render(r,s,&d,&stats)==diffuse && stats.upload_bytes==0);
    mat.metallic=1;CHECK(material(s,n,&mat)==XGE_OK);int rough=render(r,s,&d,NULL);CHECK(rough>100 && rough<180);
    mat.roughness=0;CHECK(material(s,n,&mat)==XGE_OK && render(r,s,&d,NULL)>rough+60);
    unsigned char ao[]={0,0,0,255};xge_image_t ao_image={1,1,XGE_PIXEL_RGBA8,4,ao,XGE_IMAGE_STRAIGHT_ALPHA};
    xge3d_texture_desc_t td={&ao_image,0,{0}};CHECK(xge3dTextureCreate(&td,&occlusion)==XGE_OK);
    mat.maps[3].texture=occlusion;CHECK(material(s,n,&mat)==XGE_OK && render(r,s,&d,NULL)==0);
    mat.occlusion_strength=0;CHECK(material(s,n,&mat)==XGE_OK && render(r,s,&d,NULL)>rough+60);
    mat.maps[3].texture=NULL;mat.roughness=1;CHECK(material(s,n,&mat)==XGE_OK);
    /* Distinct roughness levels prove explicit cube LOD selection. */
    for (int l=0;l<3;++l) for (int f=0;f<6;++f) {
        xge_image_t *image=&images[6+l*6+f];unsigned char *p=image->pPixels;
        for (int i=0;i<image->iWidth*image->iHeight;++i) {p[i*4]=l==0 ? 255 : 0;p[i*4+1]=0;p[i*4+2]=l==0 ? 0 : 255;}
    }
    CHECK(xge3dEnvironmentSetIBL(env,&ibl)==XGE_OK && render(r,s,&d,NULL)==0);
    CHECK(pixels[(32*64+32)*4+2]>100);
    mat.roughness=0;CHECK(material(s,n,&mat)==XGE_OK && render(r,s,&d,NULL)>220);
    ibl.level_count=2;CHECK(xge3dEnvironmentSetIBL(env,&ibl)==XGE_ERROR_INVALID_ARGUMENT && render(r,s,&d,NULL)>220);
    d.ibl_intensity=0;CHECK(render(r,s,&d,NULL)==0);
    d.ibl_intensity=1;CHECK(xge3dEnvironmentSetIBL(env,NULL)==XGE_OK && xge3dRender(r,s,&d,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    failed=0;printf("3D IBL GPU: C offline pipeline, diffuse %d / rough metal %d, roughness LOD, AO, cached upload and transactional replacement passed\n",diffuse,rough);
done:
    xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dEnvironmentFree(env);xge3dTextureFree(occlusion);
    xge3dTargetFree(target);xge3dRendererFree(r);for (int i=0;i<25;++i) xgeImageFree(&images[i]);return 1;
}
int main(void)
{
    xge_desc_t d={0};d.iWidth=d.iHeight=128;d.sTitle="XGE IBL GPU verification";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;
}
