#include "xge.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"height fog %d: %s\n",__LINE__,#x);goto done;}}while(0)
static int failed=1;static unsigned char pixels[192*192*4],reference[sizeof(pixels)];
#if XGE3D_ENABLE_HEIGHT_FOG
static int encode(double x){x=fmax(0,fmin(1,x));return (int)lround(255*(x<=.0031308?12.92*x:1.055*pow(x,1/2.4)-.055));}
static int sample(int x,int y,double r,double g,double b)
{const unsigned char *p=pixels+(y*192+x)*4;int e[]={encode(r),encode(g),encode(b),255};for(int i=0;i<4;++i){if(abs(p[i]-e[i])>2){fprintf(stderr,"pixel(%d,%d)[%d]=%u expected %d\n",x,y,i,p[i],e[i]);return 0;}}return 1;}
static double amount(double x,double y,double distance,double density,double falloff)
{double a=falloff*y;double integral=fabs(a)<1e-6?1:(1-exp(-a))/a;return 1-exp(-density*sqrt(x*x+y*y+distance*distance)*integral);}
#endif
static int render(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d)
{int result=xge3dRender(r,s,d,NULL);return result?result:xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),192*4);}
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;xge3d_material *m=NULL;xge3d_node_t nodes[3];
    CHECK(!xge3dSceneCreate(&s)&&!xge3dRendererCreate(&r)&&!xge3dTargetCreate(192,192,&target));
    xge3d_vertex_t vertices[6]={0};const float xy[][2]={{-1,-1},{1,-1},{1,1},{-1,-1},{1,1},{-1,1}};
    for(int i=0;i<6;++i){vertices[i].position=(xge3d_vec3_t){xy[i][0]*.35f,xy[i][1]*.35f,0};vertices[i].normal.z=1;}
    xge3d_mesh_desc_t md={vertices,6};CHECK(!xge3dMeshCreate(&md,&mesh));
    xge3d_material_desc_t material=xge3dMaterialDefault();material.unlit=material.double_sided=1;material.base_color[1]=material.base_color[2]=0;CHECK(!xge3dMaterialCreate(&material,&m));
    for(int i=0;i<3;++i){xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){i-1.f,i-1.f,-6};
        CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&nodes[i])&&!xge3dNodeSetTransform(s,nodes[i],&t)&&!xge3dNodeSetMesh(s,nodes[i],mesh)&&!xge3dNodeSetMaterial(s,nodes[i],m));}
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,4,4,.1f,30)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){0},(xge3d_vec3_t){0,0,-1},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t d={&camera,target,{0,0,0,1},3};CHECK(!render(r,s,&d));memcpy(reference,pixels,sizeof(pixels));
#if XGE3D_ENABLE_HEIGHT_FOG
    xge3d_height_fog_settings_t f=xge3dHeightFogDefault();f.color=(xge3d_vec3_t){0,0,1};f.density=.2f;f.falloff=.7f;f.max_opacity=1;
    CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d));
    for(int i=0;i<3;++i){int x=48+48*i,y=144-48*i;double world_y=(1-(y+.5)/192*2)*2;double a=1-exp(-.2*6*exp(-.7*world_y));CHECK(sample(x,y,1-a,0,a));}
    CHECK(pixels[(144*192+48)*4]<pixels[(48*192+144)*4]);
    f.falloff=0;CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d));double a=1-exp(-1.2);
    for(int i=0;i<3;++i)CHECK(sample(48+48*i,144-48*i,1-a,0,a));
    f.max_opacity=.25f;CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d)&&sample(96,96,.75,0,.25));
    f.density=0;CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    CHECK(!xge3dRendererSetHeightFog(r,NULL)&&!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    f.density=.2f;f.falloff=.7f;f.max_opacity=1;CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d));memcpy(reference,pixels,sizeof(pixels));
    xge3d_height_fog_settings_t bad=f;bad.density=NAN;CHECK(xge3dRendererSetHeightFog(r,&bad)==XGE_ERROR_INVALID_ARGUMENT);
    bad=f;bad.height=INFINITY;CHECK(xge3dRendererSetHeightFog(r,&bad)==XGE_ERROR_INVALID_ARGUMENT);
    bad=f;bad.color.x=-1;CHECK(xge3dRendererSetHeightFog(r,&bad)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    /* Global height remains stable after moving the origin and camera. */
    xge3d_dvec3_t origin={1e9,2e9,-3e9};CHECK(!xge3dSceneSetOrigin(s,origin));
    for(int i=0;i<3;++i)CHECK(!xge3dNodeSetGlobalPosition(s,nodes[i],(xge3d_dvec3_t){origin.x+i-1,origin.y+i-1,origin.z-6}));
    CHECK(!xge3dCameraLookAtGlobal(&camera,origin,origin,(xge3d_dvec3_t){origin.x,origin.y,origin.z-1},(xge3d_vec3_t){0,1,0}));f.height=origin.y;
    CHECK(!xge3dRendererSetHeightFog(r,&f)&&!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    xge3d_dvec3_t shifted={origin.x+100,origin.y-100,origin.z+50};CHECK(!xge3dSceneSetOrigin(s,shifted));
    CHECK(!xge3dCameraLookAtGlobal(&camera,shifted,origin,(xge3d_dvec3_t){origin.x,origin.y,origin.z-1},(xge3d_vec3_t){0,1,0}));
    CHECK(!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    /* Perspective uses the actual ray, including an inclined density integral. */
    CHECK(!xge3dCameraPerspective(&camera,1.2f,1,.1f,30));
    CHECK(!xge3dCameraLookAtGlobal(&camera,shifted,origin,(xge3d_dvec3_t){origin.x,origin.y,origin.z-1},(xge3d_vec3_t){0,1,0})&&!render(r,s,&d));
    for(int i=0;i<3;++i){int x=(int)(96*(1+(i-1)/(6*tan(.6)))),y=(int)(96*(1-(i-1)/(6*tan(.6))));
        double xx=((x+.5)/96-1)*6*tan(.6),yy=(1-(y+.5)/96)*6*tan(.6);a=amount(xx,yy,6,.2,.7);CHECK(sample(x,y,1-a,0,a));}
#if XGE3D_ENABLE_FOG
    xge3d_fog_settings_t distance=xge3dFogDefault();distance.color=(xge3d_vec3_t){0,1,0};distance.start=0;distance.end=12;d.fog=&distance;
    CHECK(!render(r,s,&d));a=amount(.5/96*6*tan(.6),-.5/96*6*tan(.6),6,.2,.7);CHECK(sample(96,96,(1-a)*.5,(1-a)*.5,a));d.fog=NULL;
#endif
    a=amount(.5/96*6*tan(.6),-.5/96*6*tan(.6),6,.2,.7);
    d.exposure=.5f;CHECK(!render(r,s,&d)&&sample(96,96,(1-a)*.5,0,a*.5));
    CHECK(!xgeImageSavePNG("artifacts/xge-light-effects/height-fog.png",192,192,pixels,192*4));
    puts("Height fog GPU: analytic horizontal/inclined rays, perspective/orthographic, density/cap, origin rebase, distance fog/exposure and disable restoration passed");
#else
    CHECK(!render(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));puts("Height fog trimmed: ordinary rendering passed");
#endif
    failed=0;
done:
    xge3dRendererFree(r);xge3dTargetFree(target);xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dMaterialFree(m);return 1;
}
int main(void)
{xge_desc_t d={.iWidth=192,.iHeight=192,.sTitle="XGE analytical height fog GPU verification"};if(xgeInit(&d))return 1;int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
