#include "xge.h"
#include "extlibs/xrt/xrt.h"
#define XGE_GL_IMPL
#include "xge_gl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"foundation GPU %d: %s (GL %x)\n",__LINE__,#x,xge_glGetError());goto done;}}while(0)
static int failed=1;static unsigned char pixels[128*128*4],reference[sizeof(pixels)];
static void *load_gl(const char *name)
{HMODULE gl=GetModuleHandleW(L"opengl32.dll");FARPROC p=GetProcAddress(gl,name);if(!p){typedef PROC(WINAPI *getter)(LPCSTR);getter get;FARPROC raw=GetProcAddress(gl,"wglGetProcAddress");memcpy(&get,&raw,sizeof(get));p=get(name);}void *out;memcpy(&out,&p,sizeof(out));return out;}
static int quad(xge3d_mesh **out,float left,float bottom,float right,float top)
{
    xge3d_vertex_t v[4]={0};const float pos[][2]={{left,bottom},{right,bottom},{right,top},{left,top}},uv[][2]={{0,1},{1,1},{1,0},{0,0}};
    for(int i=0;i<4;++i){v[i].position=(xge3d_vec3_t){pos[i][0],pos[i][1],0};v[i].normal.z=1;memcpy(v[i].uv,uv[i],sizeof(v[i].uv));
#if XGE3D_ENABLE_ANIMATION
        v[i].weights[0]=1;
#endif
    }uint16_t indices[]={0,1,2,0,2,3};xge3d_mesh_desc_t d={v,4,indices,6,16};return *out?xge3dMeshUpdate(*out,&d):xge3dMeshCreate(&d,out);
}
static int render(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d,xge3d_render_stats_t *stats)
{int result=xge3dRender(r,s,d,stats);return result?result:xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),512);}
static int encoded(float linear){linear=fmaxf(0,fminf(1,linear));return (int)lroundf(255*(linear<=.0031308f?linear*12.92f:1.055f*powf(linear,1/2.4f)-.055f));}
static int at(int x,int y,float r,float g,float b,float a)
{unsigned char *p=pixels+(y*128+x)*4;float rgb[]={r,g,b};for(int k=0;k<3;++k){if(abs(p[k]-(int)lroundf(encoded(rgb[k])*a))>2){fprintf(stderr,"pixel %d,%d [%d] %d expected %f\n",x,y,k,p[k],encoded(rgb[k])*a);return 0;}}return abs(p[3]-(int)lroundf(a*255))<=2;}
#if XGE3D_ENABLE_DEBUG_DRAW
static int green_count(void){int n=0;for(size_t i=0;i<sizeof(pixels);i+=4){if(pixels[i+1]>200&&pixels[i]<10&&pixels[i+2]<10){++n;}}return n;}
#endif
static int frame(void *user)
{
    (void)user;(void)reference;xge3d_renderer *r=NULL,*source_renderer=NULL;xge3d_scene *s=NULL,*source=NULL;xge3d_target *target=NULL,*source_target=NULL;
    xge3d_mesh *mesh=NULL,*source_mesh=NULL;xge3d_material *m=NULL,*source_material=NULL;xge3d_texture *texture=NULL,*view=NULL;GLuint pbo=0;
#if XGE3D_ENABLE_SHADER
    xge3d_shader *shader=NULL,*bad_shader=NULL;
#endif
    CHECK(xge_gl_load(load_gl));CHECK(!xge3dRendererCreate(&r)&&!xge3dSceneCreate(&s)&&!xge3dTargetCreate(128,128,&target)&&!quad(&mesh,-1,-1,1,1));
    xge3d_node_t node;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&node)&&!xge3dNodeSetMesh(s,node,mesh));
    xge3d_material_desc_t material=xge3dMaterialDefault();material.unlit=material.double_sided=1;CHECK(!xge3dMaterialCreate(&material,&m)&&!xge3dNodeSetMaterial(s,node,m));
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,2,2,.1f,10)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t d={&camera,target,{0,0,0,0},3};xge3d_render_stats_t stats;
    CHECK(!render(r,s,&d,&stats)&&at(64,64,1,1,1,1));
#if XGE3D_ENABLE_DYNAMIC
    unsigned char texels[16]={255,0,0,255,255,0,0,255,255,0,0,255,255,0,0,255};
    xge_image_t image={.iWidth=2,.iHeight=2,.iStride=8,.iFormat=XGE_PIXEL_RGBA8,.pPixels=texels};xge3d_texture_desc_t td={.image=&image,.srgb=1};
    CHECK(!xge3dTextureCreate(&td,&texture));material.maps[0].texture=texture;CHECK(!xge3dMaterialSetDesc(m,&material)&&!render(r,s,&d,&stats)&&at(64,64,1,0,0,1));
    for(int i=0;i<4;++i){texels[i*4]=0;texels[i*4+1]=255;}
    xge_glGenBuffers(1,&pbo);xge_glBindBuffer(0x88EC,pbo);xge_glBufferData(0x88EC,64,NULL,GL_STATIC_DRAW);xge_glPixelStorei(0x0CF2,5);xge_glPixelStorei(0x0CF4,2);xge_glPixelStorei(0x0CF3,1);
    CHECK(!xge3dTextureUpdate(texture,0,0,2,2,texels,8));GLint state;xge_glGetIntegerv(0x88EF,&state);CHECK(state==(GLint)pbo);xge_glGetIntegerv(0x0CF2,&state);CHECK(state==5);
    xge_glGetIntegerv(0x0CF4,&state);CHECK(state==2);xge_glGetIntegerv(0x0CF3,&state);CHECK(state==1);xge_glBindBuffer(0x88EC,0);xge_glPixelStorei(0x0CF2,0);xge_glPixelStorei(0x0CF4,0);xge_glPixelStorei(0x0CF3,0);
    material.base_color[1]=.25f;CHECK(!xge3dMaterialSetDesc(m,&material)&&!render(r,s,&d,&stats)&&at(64,64,0,.25f,0,1));
    material.base_color[1]=1;material.maps[0].texture=NULL;CHECK(!xge3dMaterialSetDesc(m,&material));
    CHECK(!xge3dRendererCreate(&source_renderer)&&!xge3dSceneCreate(&source)&&!xge3dTargetCreate(128,128,&source_target)&&!quad(&source_mesh,-1,0,1,1));
    xge3d_node_t source_node;CHECK(!xge3dNodeCreate(source,(xge3d_node_t){0},&source_node)&&!xge3dNodeSetMesh(source,source_node,source_mesh));
    xge3d_material_desc_t sm=xge3dMaterialDefault();sm.unlit=sm.double_sided=1;sm.alpha_mode=XGE3D_ALPHA_BLEND;sm.base_color[0]=.25f;sm.base_color[1]=.5f;sm.base_color[2]=.75f;sm.base_color[3]=.5f;
    CHECK(!xge3dMaterialCreate(&sm,&source_material)&&!xge3dNodeSetMaterial(source,source_node,source_material));xge3d_render_desc_t sd=d;sd.target=source_target;
    CHECK(!render(source_renderer,source,&sd,&stats));memcpy(reference,pixels,sizeof(pixels));CHECK(!xge3dTextureFromTarget(source_target,&view));
    material.maps[0].texture=view;material.alpha_mode=XGE3D_ALPHA_BLEND;CHECK(!xge3dMaterialSetDesc(m,&material)&&!render(r,s,&d,&stats));
    CHECK(at(64,32,.25f,.5f,.75f,.5f)&&at(64,96,0,0,0,0));for(size_t i=0;i<sizeof(pixels);++i){CHECK(abs((int)pixels[i]-reference[i])<=2);}
    xge3d_render_desc_t feedback=d;feedback.target=source_target;CHECK(xge3dRender(r,s,&feedback,NULL)==XGE_ERROR_INVALID_STATE);
    CHECK(!xge3dTargetReadPixels(source_target,pixels,sizeof(pixels),512)&&!memcmp(pixels,reference,sizeof(pixels)));
    material.maps[1].texture=view;CHECK(xge3dMaterialSetDesc(m,&material)==XGE_ERROR_UNSUPPORTED);material.maps[1].texture=NULL;
    CHECK(xge3dTextureUpdate(view,0,0,1,1,texels,4)==XGE_ERROR_UNSUPPORTED);
    CHECK(!xge3dTargetResize(source_target,64,64));sd.clear_color[0]=0;sd.clear_color[1]=1;sd.clear_color[2]=0;sd.clear_color[3]=1;
    CHECK(!xge3dNodeSetVisible(source,source_node,0)&&!xge3dRender(source_renderer,source,&sd,NULL));
    xge3dTargetFree(source_target);source_target=NULL;CHECK(!render(r,s,&d,&stats)&&at(64,64,0,1,0,1));
    xge3d_resource_info_t resource={.size=sizeof(resource)};CHECK(!xge3dTextureResourceInfo(view,&resource)&&!resource.cpu_payload_bytes&&!resource.gpu_payload_bytes);
    xge3d_upload_budget_t budget={1,1,1};xge3d_upload_stats_t upload;CHECK(!xge3dTexturePrepare(view,&budget,&upload)&&upload.completed&&!upload.operations);
    material.maps[0].texture=NULL;material.alpha_mode=XGE3D_ALPHA_OPAQUE;CHECK(!xge3dMaterialSetDesc(m,&material));
#endif
#if XGE3D_ENABLE_SHADER
    xge3d_shader_desc_t shader_desc={.size=sizeof(shader_desc),.bounds_margin=4,
        .vertex="void xge3dVertex(inout vec3 p,inout vec3 n,vec2 uv){p.x+=xge3dParams[0].x;}",
        .surface="void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){base*=xge3dParams[1];}"};
    CHECK(!xge3dShaderCreate(&shader_desc,&shader));float parameters[16]={0};parameters[0]=.5f;parameters[5]=parameters[7]=1;
    CHECK(!xge3dMaterialSetShader(m,shader,parameters));xge3dShaderFree(shader);shader=NULL;
    CHECK(!render(r,s,&d,&stats)&&at(8,64,0,0,0,0)&&at(96,64,0,1,0,1));
    parameters[0]=NAN;CHECK(xge3dMaterialSetShader(m,NULL,parameters)==XGE_ERROR_INVALID_ARGUMENT&&!render(r,s,&d,&stats)&&at(96,64,0,1,0,1));
    CHECK(!xge3dMaterialSetShader(m,NULL,NULL)&&!render(r,s,&d,&stats)&&at(8,64,1,1,1,1));
    shader_desc.surface="THIS IS INVALID GLSL";CHECK(xge3dShaderCreate(&shader_desc,&bad_shader)!=XGE_OK&&!bad_shader);
    shader_desc.surface="void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){}";CHECK(!xge3dShaderCreate(&shader_desc,&shader));
    parameters[0]=-3;parameters[5]=parameters[7]=1;CHECK(!xge3dMaterialSetShader(m,shader,parameters));
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=3;CHECK(!xge3dNodeSetTransform(s,node,&t)&&!render(r,s,&d,&stats)&&at(64,64,1,1,1,1)&&stats.visible_meshes==1);
    t.position.x=0;CHECK(!xge3dNodeSetTransform(s,node,&t)&&!xge3dMaterialSetShader(m,NULL,NULL));
    xge3d_node_t other;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&other)&&!xge3dNodeSetMesh(s,other,mesh)&&!xge3dNodeSetMaterial(s,other,m));
    parameters[0]=.25f;CHECK(!xge3dMaterialSetShader(m,shader,parameters)&&!render(r,s,&d,&stats)&&stats.instanced_draw_calls==1);memcpy(reference,pixels,sizeof(pixels));
    d.disable_instancing=1;CHECK(!render(r,s,&d,&stats)&&!memcmp(reference,pixels,sizeof(pixels)));d.disable_instancing=0;CHECK(!xge3dNodeDestroy(s,other)&&!xge3dMaterialSetShader(m,NULL,NULL));
#if XGE3D_ENABLE_ANIMATION
    xge3d_node_t joint;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&joint));xge3d_mat4_t inverse_bind={{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
    xge3d_skin_binding_t skin={.joints=&joint,.inverse_bind=&inverse_bind,.count=1};CHECK(!xge3dNodeBindSkin(s,node,&skin));
    t.position.x=.25f;CHECK(!xge3dNodeSetTransform(s,joint,&t)&&!render(r,s,&d,&stats));memcpy(reference,pixels,sizeof(pixels));
    memset(parameters,0,sizeof(parameters));CHECK(!xge3dMaterialSetShader(m,shader,parameters)&&!render(r,s,&d,&stats)&&!memcmp(reference,pixels,sizeof(pixels)));
    CHECK(!xge3dMaterialSetShader(m,NULL,NULL));
#if XGE3D_ENABLE_DEBUG_DRAW
    xge3d_node_t child;CHECK(!xge3dNodeCreate(s,joint,&child));xge3d_transform_t child_pose=XGE3D_TRANSFORM_IDENTITY;child_pose.position.y=.5f;
    CHECK(!xge3dNodeSetTransform(s,child,&child_pose));xge3d_node_t bones[2]={joint,child};xge3d_mat4_t binds[2]={inverse_bind,inverse_bind};skin=(xge3d_skin_binding_t){.joints=bones,.inverse_bind=binds,.count=2};
    CHECK(!xge3dNodeBindSkin(s,node,&skin)&&!xge3dDebugSkeleton(r,s,node,(const float[]){0,1,0,1},0)&&!render(r,s,&d,&stats)&&green_count()>10);xge3dDebugClear(r);
#endif
    CHECK(!xge3dNodeBindSkin(s,node,NULL));
#endif
#endif
#if XGE3D_ENABLE_DEBUG_DRAW
    CHECK(!xge3dNodeSetColor(s,node,(const float[]){1,0,0,1}));float green[]={0,1,0,1};
    CHECK(!xge3dDebugLine(r,(xge3d_vec3_t){-1,0,-1},(xge3d_vec3_t){1,0,-1},green,1)&&!render(r,s,&d,&stats)&&!green_count());
    xge3dDebugClear(r);CHECK(!xge3dDebugLine(r,(xge3d_vec3_t){-1,0,-1},(xge3d_vec3_t){1,0,-1},green,0)&&!render(r,s,&d,&stats)&&green_count()>=120);
    xge3dDebugClear(r);CHECK(!render(r,s,&d,&stats)&&!green_count());
    xge3d_mat4_t transform={{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};xge3d_aabb_t box={{-.5f,-.5f,0},{.5f,.5f,0}};
    CHECK(!xge3dDebugBox(r,&box,&transform,green,0)&&!xge3dDebugAxes(r,&transform,.5f,0)&&!render(r,s,&d,&stats)&&green_count()>20);
    xge3dDebugClear(r);CHECK(xge3dDebugLine(r,(xge3d_vec3_t){NAN,0,0},(xge3d_vec3_t){0},green,0)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xge3dNodeSetColor(s,node,(const float[]){1,1,1,1}));
#endif
#if XGE3D_ENABLE_BILLBOARD
    CHECK(!xge3dNodeSetVisible(s,node,0));xge3d_billboard_t billboards[2]={xge3dBillboardDefault(),xge3dBillboardDefault()};
    billboards[0].color[1]=billboards[0].color[2]=0;billboards[0].color[3]=.5f;billboards[1].color[0]=billboards[1].color[1]=0;billboards[1].color[3]=.5f;billboards[1].position.z=-1;
    CHECK(!xge3dBillboardsSet(r,billboards,2)&&!render(r,s,&d,&stats)&&at(64,64,2.0f/3,0,1.0f/3,.75f)&&stats.draw_calls==1&&stats.triangles==4);
    billboards[0].blend=XGE3D_BILLBOARD_ADD;CHECK(!xge3dBillboardsSet(r,billboards,2)&&!render(r,s,&d,&stats)&&at(64,64,2.0f/3,0,2.0f/3,.75f));
    billboards[0]=xge3dBillboardDefault();billboards[0].position.z=-1;billboards[0].color[0]=billboards[0].color[2]=0;
    CHECK(!xge3dNodeSetVisible(s,node,1)&&!xge3dBillboardsSet(r,billboards,1)&&!render(r,s,&d,&stats)&&at(64,64,1,1,1,1));
    billboards[0].depth_test=0;CHECK(!xge3dBillboardsSet(r,billboards,1)&&!render(r,s,&d,&stats)&&at(64,64,0,1,0,1));
    CHECK(!xge3dNodeSetVisible(s,node,0));billboards[0].depth_test=1;billboards[0].position=(xge3d_vec3_t){0};
    CHECK(!xge3dCameraLookAt(&camera,(xge3d_vec3_t){3,0,0},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})&&!xge3dBillboardsSet(r,billboards,1)&&!render(r,s,&d,&stats)&&at(64,64,0,1,0,1));
    CHECK(!xge3dBillboardsSet(r,billboards,1)&&!render(r,s,&d,&stats));xallocationbudget *quota=NULL;xallocationscope scope={0};
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));CHECK(!xge3dBillboardsSet(r,billboards,1)&&!render(r,s,&d,&stats)&&!scope.Rejections);CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);
    CHECK(!xge3dBillboardsSet(r,NULL,0)&&!render(r,s,&d,&stats)&&at(64,64,0,0,0,0));
#endif
    CHECK(!xge_glGetError());failed=0;puts("Game foundation GPU: dynamic pixels/materials, target view color/alpha/orientation/lifetime, shader hooks/skin/instances/bounds, debug depth and billboard blending passed");
done:
    if(pbo){xge_glBindBuffer(0x88EC,0);xge_glDeleteBuffers(1,&pbo);}
#if XGE3D_ENABLE_SHADER
    xge3dShaderFree(shader);xge3dShaderFree(bad_shader);
#endif
    xge3dSceneFree(source);xge3dRendererFree(source_renderer);xge3dTargetFree(source_target);xge3dMeshFree(source_mesh);xge3dMaterialFree(source_material);
    xge3dRendererFree(r);xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dMaterialFree(m);xge3dTextureFree(texture);xge3dTextureFree(view);xge3dTargetFree(target);return 1;
}
int main(void){xge_desc_t d={.iWidth=160,.iHeight=160,.sTitle="XGE game foundation GPU checks"};if(xgeInit(&d)){return 1;}int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
