#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if XGE_ENABLE_XUI
#include "../xui.h"
#endif



#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int result=1;
static xge3d_mesh *cpu_mesh;
static unsigned char pixels[256*256*4];
static int center(int red, int green, int blue)
{
    const unsigned char *p=pixels+(128*256+128)*4;
    return abs(p[0]-red)<=2 && abs(p[1]-green)<=2 && abs(p[2]-blue)<=2;
}
static const xge3d_vertex_t cube[] = {
    {{-.5f,-.5f,-.5f}},{{.5f,-.5f,-.5f}},{{.5f,.5f,-.5f}},{{-.5f,.5f,-.5f}},
    {{-.5f,-.5f,.5f}},{{.5f,-.5f,.5f}},{{.5f,.5f,.5f}},{{-.5f,.5f,.5f}}
};
static const uint32_t cube_indices[]={4,5,6,4,6,7, 1,0,3,1,3,2, 0,4,7,0,7,3,
    5,1,2,5,2,6, 0,1,5,0,5,4, 7,6,2,7,2,3};

#if XGE_ENABLE_2D
static int mixed_2d(xge3d_renderer *r, xge3d_scene *scene, xge3d_render_desc_t *desc)
{
    xge_render_target_t target={0}; xge_pass_t pass={0};
    int ok=0;
    CHECK(xgeRenderTargetCreate(&target,128,64)==XGE_OK);
    xgePassInit(&pass,&target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(0,0,0,255));
    CHECK(xgePassBegin(&pass)==XGE_OK);
    xgeShapeRectFill((xge_rect_t){8,8,20,20},XGE_COLOR_RGBA(255,0,0,255));
    CHECK(xge3dRender(r,scene,desc,NULL)==XGE_OK);
    xgeShapeRectFill((xge_rect_t){40,8,20,20},XGE_COLOR_RGBA(0,0,255,255));
#if XGE3D_ENABLE_SHADOW
    xge3d_shadow_settings_t bad_shadows=xge3dShadowDefault();bad_shadows.resolution=8;
    xge3d_render_desc_t shadow_failure=*desc;shadow_failure.shadows=&bad_shadows;
    CHECK(xge3dRender(r,scene,&shadow_failure,NULL)==XGE_ERROR_INVALID_ARGUMENT);
#endif
    xge3d_render_desc_t failed=*desc; failed.target=NULL;
    CHECK(xge3dRender(r,scene,&failed,NULL)==XGE_ERROR_UNSUPPORTED);
    xgeShapeRectFill((xge_rect_t){72,8,20,20},XGE_COLOR_RGBA(0,255,0,255));
    CHECK(xgePassEnd(&pass)==XGE_OK);
    CHECK(xgeRenderTargetReadPixels(&target,pixels,512)==XGE_OK);
    CHECK(pixels[(16*128+16)*4]>250 && pixels[(16*128+48)*4+2]>250 && pixels[(16*128+80)*4+1]>250);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p1-mixed-2d.png",128,64,pixels,512)==XGE_OK);
    ok=1;
done:
    if (pass.bActive) xgePassEnd(&pass);
    xgeRenderTargetFree(&target);
    return ok;
}
#endif

#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
static int mixed_xui(xge3d_renderer *r, xge3d_scene *scene, const xge3d_render_desc_t *desc)
{
    xui_context ui=NULL;
    xui_surface surface=NULL;
    xui_widget button=NULL;
    xui_proxy_t proxy=xuiProxyXge();
    int ok=0;
    CHECK(xuiCreate(&ui)==XUI_OK && xuiSetProxy(ui,&proxy)==XUI_OK);
    CHECK(xuiInputViewport(ui,128,64)==XUI_OK);
    xui_surface_desc_t sd={0}; sd.iKind=XUI_SURFACE_KIND_TEXTURE;
    sd.iFormat=XUI_SURFACE_FORMAT_RGBA8; sd.iWidth=128; sd.iHeight=64;
    sd.iFlags=XUI_SURFACE_ALPHA_PREMULTIPLIED|XUI_SURFACE_USAGE_TARGET;
    CHECK(proxy.surfaceCreate(&proxy,&surface,&sd)==XUI_OK);
    xui_button_desc_t bd={0}; bd.iSize=sizeof(bd); bd.sText="";
    CHECK(xuiButtonCreate(ui,&button,&bd)==XUI_OK);
    CHECK(xuiSetRootWidget(ui,button)==XUI_OK);
    CHECK(xuiWidgetArrange(button,(xui_rect_t){0,0,128,64})==XUI_OK);
    CHECK(xuiButtonSetBorder(button,0,0)==XUI_OK);
    uint32_t green=XUI_COLOR_RGBA(0,255,0,255), purple=XUI_COLOR_RGBA(255,0,255,255);
    CHECK(xuiButtonSetColors(button,green,green,green,green,green)==XUI_OK);
    xui_rect_i_t rect={0,0,128,64};
    CHECK(xuiRender(ui,surface,&rect,1)==XUI_OK);
    CHECK(proxy.surfaceReadRGBA(&proxy,surface,pixels,512)==XUI_OK);
    CHECK(pixels[(32*128+64)*4+1]>250);
    CHECK(xge3dRender(r,scene,desc,NULL)==XGE_OK);
    CHECK(xuiButtonSetColors(button,purple,purple,purple,purple,purple)==XUI_OK);
    CHECK(xuiRender(ui,surface,&rect,1)==XUI_OK);
    CHECK(proxy.surfaceReadRGBA(&proxy,surface,pixels,512)==XUI_OK);
    CHECK(pixels[(32*128+64)*4]>250 && pixels[(32*128+64)*4+1]<3 && pixels[(32*128+64)*4+2]>250);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p1-mixed-xui.png",128,64,pixels,512)==XGE_OK);
    ok=1;
done:
    if (ui) xuiDestroy(ui);
    if (surface) proxy.surfaceDestroy(&proxy,surface);
    return ok;
}
#endif

static int frame(void *user)
{
    (void)user;
    xge3d_renderer *r=NULL;
    xge3d_target *target=NULL;
    xge3d_scene *scene=NULL;
    xge3d_mesh *mesh=cpu_mesh, *big_mesh=NULL;
    cpu_mesh=NULL;
    xge3d_vertex_t *big=NULL;
    xge3d_node_t front, back, triangle;
    xge3d_camera_t camera;
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
    xge3d_render_stats_t stats;
    const float red[]={1,0,0,1}, blue[]={0,0,1,1};
    CHECK(xge3dRendererCreate(&r)==XGE_OK);
    CHECK(xge3dTargetCreate(256,256,&target)==XGE_OK);
    CHECK(xge3dSceneCreate(&scene)==XGE_OK);
    xge3d_mesh_desc_t md={cube,8,cube_indices,36,32};
    CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&front)==XGE_OK);
    CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&back)==XGE_OK);
    CHECK(xge3dNodeSetMesh(scene,front,mesh)==XGE_OK && xge3dNodeSetMesh(scene,back,mesh)==XGE_OK);
    xge3dMeshFree(mesh); mesh=NULL;
    CHECK(xge3dNodeSetColor(scene,front,red)==XGE_OK && xge3dNodeSetColor(scene,back,blue)==XGE_OK);
    t.position.z=.6f; CHECK(xge3dNodeSetTransform(scene,front,&t)==XGE_OK);
    t.position.z=-.6f; CHECK(xge3dNodeSetTransform(scene,back,&t)==XGE_OK);
    CHECK(xge3dCameraPerspective(&camera,1,1,.1f,20)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t desc={&camera,target,{0,0,0,1},3};
    /* Front first, back last: draw order must not replace depth testing. */
    int render_result=xge3dRender(r,scene,&desc,&stats);
    if (render_result!=XGE_OK) fprintf(stderr,"render result=%d, draws=%u, triangles=%llu\n",
        render_result,stats.draw_calls,(unsigned long long)stats.triangles);
    CHECK(render_result==XGE_OK && stats.draw_calls==2 && stats.triangles==24);
    CHECK(stats.upload_bytes==sizeof(cube)+sizeof(cube_indices));
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(255,0,0));
    CHECK(xge3dRender(r,scene,&desc,&stats)==XGE_OK && stats.upload_bytes==0);
    CHECK(xge3dNodeSetVisible(scene,front,0)==XGE_OK);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(0,0,255));
    CHECK(xge3dNodeSetVisible(scene,front,1)==XGE_OK);
    t.position.z=.6f; t.scale.x=-1;
    CHECK(xge3dNodeSetTransform(scene,front,&t)==XGE_OK);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(255,0,0));
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p1-cube.png",256,256,pixels,1024)==XGE_OK);
    CHECK(xge3dNodeSetVisible(scene,back,0)==XGE_OK);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;
    t.position.z=3.975f; t.scale=(xge3d_vec3_t){.025f,.025f,.025f};
    CHECK(xge3dNodeSetTransform(scene,front,&t)==XGE_OK);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(0,0,0));
    /* 32-bit indices really address vertices beyond 65535. */
    big=calloc(65539,sizeof(*big)); CHECK(big!=NULL);
    big[65536].position=(xge3d_vec3_t){-.7f,-.7f,0};
    big[65537].position=(xge3d_vec3_t){.7f,-.7f,0};
    big[65538].position=(xge3d_vec3_t){0,.7f,0};
    uint32_t indices[]={65536,65537,65538};
    md=(xge3d_mesh_desc_t){big,65539,indices,3,32};
    CHECK(xge3dMeshCreate(&md,&big_mesh)==XGE_OK);
    CHECK(xge3dNodeCreate(scene,(xge3d_node_t){0},&triangle)==XGE_OK);
    CHECK(xge3dNodeSetMesh(scene,triangle,big_mesh)==XGE_OK);
    CHECK(xge3dNodeSetColor(scene,triangle,blue)==XGE_OK);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(0,0,255));
    indices[2]=65539; CHECK(xge3dMeshUpdate(big_mesh,&md)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),1024)==XGE_OK && center(0,0,255));
    CHECK(xge3dTargetResize(target,-1,64)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dTargetTexture(target)->iWidth==256);
    CHECK(xge3dTargetResize(target,128,64)==XGE_OK);
    CHECK(xge3dRender(r,scene,&desc,NULL)==XGE_OK);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    CHECK(pixels[(32*128+64)*4+2]>250);
#if XGE_ENABLE_2D
    CHECK(mixed_2d(r,scene,&desc));
#endif
#if XGE_ENABLE_XUI && XUI_ENABLE_BUTTON
    CHECK(mixed_xui(r,scene,&desc));
    puts("3D GPU: real XUI button before/after 3D passed");
#endif
    result=0;
    puts("3D GPU: cube occlusion, near clipping, mirrored winding, 32-bit indices, stable upload, transactional resize/update passed");
#if XGE_ENABLE_2D
    puts("3D GPU: queued 2D before/after 3D and failed render state restoration passed");
#endif
done:
    free(big);
    xge3dSceneFree(scene); xge3dMeshFree(mesh); xge3dMeshFree(big_mesh);
    xge3dTargetFree(target); xge3dRendererFree(r);
    return 1;
}
int main(void)
{
    xge_desc_t d={0}; d.iWidth=320; d.iHeight=240; d.sTitle="XGE 3D GPU verification";
    xge3d_mesh_desc_t md={cube,8,cube_indices,36,32};
    if (xge3dMeshCreate(&md,&cpu_mesh)!=XGE_OK) return 1;
    if (xgeInit(&d)!=XGE_OK) { xge3dMeshFree(cpu_mesh); return 1; }
    int run=xgeRun(frame,NULL);
    xgeUnit();
    xge3dMeshFree(cpu_mesh);
    return run==XGE_OK ? result : 1;
}
