#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[64*64*4];
static int frame(void *user)
{
    (void)user;
    xge3d_scene *s=NULL; xge3d_renderer *r=NULL; xge3d_target *t=NULL;
    xge3d_environment *env=NULL; xge3d_mesh *mesh=NULL;
    unsigned char colors[6][64];
    const unsigned char expected[6][4]={{128,0,0,255},{0,128,0,255},{0,0,128,255},
        {128,128,0,255},{128,0,128,255},{0,128,128,255}};
    xge_image_t faces[6]; xge3d_environment_desc_t ed={0}; ed.srgb=1;
    for (int i=0;i<6;++i) {
        for (int p=0;p<16;++p) memcpy(colors[i]+p*4,expected[i],4);
        faces[i]=(xge_image_t){4,4,XGE_PIXEL_RGBA8,16,colors[i],XGE_IMAGE_STRAIGHT_ALPHA};ed.faces[i]=&faces[i];
    }
    CHECK(xge3dEnvironmentCreate(&ed,&env)==XGE_OK);
    memset(colors,0,sizeof(colors));
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dRendererCreate(&r)==XGE_OK && xge3dTargetCreate(64,64,&t)==XGE_OK);
    xge3d_camera_t camera; CHECK(xge3dCameraPerspective(&camera,1,1,.1f,100)==XGE_OK);
    xge3d_render_desc_t d={&camera,t,{0,0,0,1},3,env};
    const xge3d_vec3_t directions[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int i=0;i<6;++i) for (int translated=0;translated<2;++translated) {
        xge3d_vec3_t eye=translated ? (xge3d_vec3_t){100,20,-50} : (xge3d_vec3_t){0};
        xge3d_vec3_t target={eye.x+directions[i].x,eye.y+directions[i].y,eye.z+directions[i].z};
        CHECK(xge3dCameraLookAt(&camera,eye,target,(xge3d_vec3_t){0,i==2 || i==3 ? 0 : 1,i==2 || i==3 ? 1 : 0})==XGE_OK);
        xge3d_render_stats_t stats;
        CHECK(xge3dRender(r,s,&d,&stats)==XGE_OK && stats.draw_calls==1);
        CHECK(stats.upload_bytes==(i==0 && !translated ? 384u : 0u));
        CHECK(xge3dTargetReadPixels(t,pixels,sizeof(pixels),256)==XGE_OK);
        for (int c=0;c<4;++c) CHECK(abs(pixels[(32*64+32)*4+c]-expected[i][c])<=1);
    }
    /* Geometry remains in front of the far-depth sky. */
    xge3d_vertex_t v[3]={0}; v[0].position=(xge3d_vec3_t){-1,-1,-2};
    v[1].position=(xge3d_vec3_t){1,-1,-2}; v[2].position=(xge3d_vec3_t){0,1,-2};
    xge3d_mesh_desc_t md={v,3,NULL,0,0}; xge3d_node_t n;
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK && xge3dNodeCreate(s,(xge3d_node_t){0},&n)==XGE_OK);
    CHECK(xge3dNodeSetMesh(s,n,mesh)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0},(xge3d_vec3_t){0,0,-1},(xge3d_vec3_t){0,1,0})==XGE_OK);
    CHECK(xge3dRender(r,s,&d,NULL)==XGE_OK && xge3dTargetReadPixels(t,pixels,sizeof(pixels),256)==XGE_OK);
    CHECK(pixels[(32*64+32)*4]==255 && pixels[(32*64+32)*4+1]==255);
    CHECK(xgeImageSavePNGEx("artifacts/xge-3d/p2-sky.png",64,64,pixels,256,XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);
    failed=0; puts("3D sky GPU: all six faces, copied input, sRGB, translation invariance, camera rotation and depth occlusion passed");
done:
    xge3dSceneFree(s); xge3dMeshFree(mesh); xge3dEnvironmentFree(env);
    xge3dTargetFree(t); xge3dRendererFree(r); return 1;
}
int main(void)
{
    xge_desc_t d={0}; d.iWidth=d.iHeight=128; d.sTitle="XGE sky GPU verification";
    if (xgeInit(&d)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL); xgeUnit(); return result==XGE_OK ? failed : 1;
}
