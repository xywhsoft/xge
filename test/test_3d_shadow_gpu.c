#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[128*128*4];
static int render(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d,xge3d_render_stats_t *stats)
{
    int result=xge3dRender(r,s,d,stats);
    if (result==XGE_OK) result=xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),512);
    if (result!=XGE_OK) { fprintf(stderr,"render error %d\n",result); return -1; }
    return pixels[(64*128+64)*4];
}
static int material(xge3d_scene *s,xge3d_node_t n,float alpha,int mask)
{
    xge3d_material_desc_t d=xge3dMaterialDefault();d.metallic=0;d.double_sided=1;
    d.base_color[3]=alpha;d.alpha_mode=mask ? XGE3D_ALPHA_MASK : XGE3D_ALPHA_OPAQUE;
    xge3d_material *m=NULL;int result=xge3dMaterialCreate(&d,&m);
    if (result==XGE_OK) result=xge3dNodeSetMaterial(s,n,m);
    xge3dMaterialFree(m);return result;
}
static int frame(void *user)
{
    (void)user;
    xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;
    xge3d_node_t receiver,blocker,light;
    xge3d_vertex_t v[4]={0};
    v[0].position=(xge3d_vec3_t){-1,-1,0};v[1].position=(xge3d_vec3_t){1,-1,0};
    v[2].position=(xge3d_vec3_t){1,1,0};v[3].position=(xge3d_vec3_t){-1,1,0};
    for (int i=0;i<4;++i) v[i].normal.z=1;
    uint16_t indices[]={0,1,2,0,2,3};xge3d_mesh_desc_t md={v,4,indices,6,16};
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(128,128,&target)==XGE_OK);
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&receiver)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&blocker)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&light)==XGE_OK);
    CHECK(xge3dNodeSetMesh(s,receiver,mesh)==XGE_OK && xge3dNodeSetMesh(s,blocker,mesh)==XGE_OK);
    CHECK(material(s,receiver,1,0)==XGE_OK && material(s,blocker,1,0)==XGE_OK);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.scale=(xge3d_vec3_t){2,2,1};
    CHECK(xge3dNodeSetTransform(s,receiver,&t)==XGE_OK);
    t.scale=(xge3d_vec3_t){.2f,.2f,1};t.position=(xge3d_vec3_t){-.5f,0,1};
    CHECK(xge3dNodeSetTransform(s,blocker,&t)==XGE_OK);
    xge3d_light_desc_t ld=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);ld.direction=(xge3d_vec3_t){.5f,0,-1};ld.casts_shadow=1;
    CHECK(xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraOrthographic(&camera,4,4,.1f,20)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};xge3d_render_stats_t stats;
    int lit=render(r,s,&draw,&stats);CHECK(lit>100 && stats.shadow_maps==0);
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.resolution=256;shadows.distance=10;shadows.depth_padding=5;draw.shadows=&shadows;
    for (int c=1;c<=4;++c) {
        shadows.cascades=c;int shade=render(r,s,&draw,&stats);
        fprintf(stdout,"cascade %d: center %d vs lit %d, %u shadow draws\n",c,shade,lit,stats.shadow_draw_calls);
        CHECK(shade>=0 && shade<lit-60 && stats.shadow_maps==(uint32_t)c && stats.shadow_draw_calls==(uint32_t)c*2);
        CHECK(stats.upload_bytes==0);
    }
    CHECK(xge3dNodeSetVisible(s,blocker,0)==XGE_OK && abs(render(r,s,&draw,&stats)-lit)<=2);
    CHECK(xge3dNodeSetVisible(s,blocker,1)==XGE_OK && material(s,blocker,.25f,1)==XGE_OK);
    CHECK(abs(render(r,s,&draw,&stats)-lit)<=2);
    CHECK(material(s,blocker,.75f,1)==XGE_OK && render(r,s,&draw,&stats)<lit-60);
    shadows.filter_radius=2;
    CHECK(render(r,s,&draw,&stats)<lit-40);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p3-cascades.png",128,128,pixels,512)==XGE_OK);
    /* Stable sub-texel camera translation and both sides of a cascade split. */
    shadows.cascades=3;shadows.filter_radius=1;
    CHECK(render(r,s,&draw,&stats)>=0);
    unsigned char previous[sizeof(pixels)];memcpy(previous,pixels,sizeof(pixels));
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){.00005f,0,4},(xge3d_vec3_t){.00005f,0,0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(render(r,s,&draw,&stats)>=0 && !memcmp(previous,pixels,sizeof(pixels)));
    for (int side=0;side<2;++side) {
        float z=side ? 3.41f : 3.39f;
        CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,z},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
        CHECK(render(r,s,&draw,&stats)<lit-60);
    }
    /* Spot shadow follows its light position and cone projection. */
    ld=xge3dLightDefault(XGE3D_LIGHT_SPOT);ld.direction=(xge3d_vec3_t){.5f,0,-1};ld.casts_shadow=1;ld.intensity=10;
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){-1.5f,0,3};
    CHECK(xge3dNodeSetTransform(s,light,&t)==XGE_OK && xge3dNodeSetLight(s,light,&ld)==XGE_OK);
    draw.shadows=NULL;int spot_lit=render(r,s,&draw,&stats);CHECK(spot_lit>100);
    draw.shadows=&shadows;CHECK(render(r,s,&draw,&stats)<spot_lit-60 && stats.shadow_maps==1 && stats.shadow_draw_calls==2);
    CHECK(xge3dNodeSetVisible(s,blocker,0)==XGE_OK && abs(render(r,s,&draw,&stats)-spot_lit)<=2);
    shadows.resolution=8;CHECK(xge3dRender(r,s,&draw,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    shadows.resolution=512;CHECK(render(r,s,&draw,&stats)>=0);
    xge3d_node_t second,third;
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&second)==XGE_OK && xge3dNodeSetTransform(s,second,&t)==XGE_OK && xge3dNodeSetLight(s,second,&ld)==XGE_OK);
    CHECK(render(r,s,&draw,&stats)>=0 && stats.shadow_maps==2);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&third)==XGE_OK && xge3dNodeSetLight(s,third,&ld)==XGE_OK);
    CHECK(xge3dRender(r,s,&draw,NULL)==XGE_ERROR_UNSUPPORTED);
    CHECK(xge3dNodeSetLight(s,third,NULL)==XGE_OK && render(r,s,&draw,&stats)>=0);
    failed=0;puts("3D shadows GPU: 1-4 cascades, visible receiver/caster, shared meshes, alpha mask, PCF, spot projection, invalid settings and resize recovery passed");
done:
    xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{
    xge_desc_t d={0};d.iWidth=d.iHeight=180;d.sTitle="XGE shadow GPU verification";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;
}
