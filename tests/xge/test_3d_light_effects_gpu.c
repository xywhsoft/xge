#include "xge.h"
#define XGE_GL_IMPL
#include "xge_gl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"effects %d: %s (GL %x)\n",__LINE__,#x,xge_glGetError());goto done;}}while(0)
static int failed=1,width=128,height=128;static unsigned char pixels[128*128*4],reference[sizeof(pixels)];
static void *load_gl(const char *name)
{HMODULE gl=GetModuleHandleW(L"opengl32.dll");FARPROC p=GetProcAddress(gl,name);if(!p){typedef PROC(WINAPI *getter)(LPCSTR);getter get;FARPROC raw=GetProcAddress(gl,"wglGetProcAddress");memcpy(&get,&raw,sizeof(get));p=get(name);}void *out;memcpy(&out,&p,sizeof(out));return out;}
static int encode(float v)
{v=fmaxf(0,fminf(1,v));return (int)lroundf(255*(v<=.0031308f?12.92f*v:1.055f*powf(v,1/2.4f)-.055f));}
static int center(float r,float g,float b,float a)
{unsigned char *p=pixels+((height/2)*width+width/2)*4;int e[]={ (int)lroundf(encode(r)*a),(int)lroundf(encode(g)*a),(int)lroundf(encode(b)*a),(int)lroundf(255*a)};
 for(int i=0;i<4;++i){if(abs(p[i]-e[i])>2){fprintf(stderr,"center[%d] %d expected %d\n",i,p[i],e[i]);return 0;}}return 1;}
static int material(xge3d_scene *s,xge3d_node_t node,float red,float green,float blue,float alpha,float emission)
{xge3d_material_desc_t d=xge3dMaterialDefault();d.unlit=d.double_sided=1;d.base_color[0]=red;d.base_color[1]=green;d.base_color[2]=blue;d.base_color[3]=alpha;
 d.alpha_mode=alpha<1?XGE3D_ALPHA_BLEND:XGE3D_ALPHA_OPAQUE;d.emissive[0]=d.emissive[1]=d.emissive[2]=emission;
 xge3d_material *m=NULL;int result=xge3dMaterialCreate(&d,&m);if(!result)result=xge3dNodeSetMaterial(s,node,m);xge3dMaterialFree(m);return result;}
static int quad(xge3d_mesh **mesh,float radius)
{xge3d_vertex_t v[6]={0};float xy[][2]={{-1,-1},{1,-1},{1,1},{-1,-1},{1,1},{-1,1}};
 for(int i=0;i<6;++i){v[i].position=(xge3d_vec3_t){xy[i][0]*radius,xy[i][1]*radius,0};v[i].normal.z=1;}xge3d_mesh_desc_t d={v,6};return *mesh?xge3dMeshUpdate(*mesh,&d):xge3dMeshCreate(&d,mesh);}
static int read_scene(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d)
{const xge_texture_t *t=xge3dTargetTexture(d->target);width=t->iWidth;height=t->iHeight;int result=xge3dRender(r,s,d,NULL);
 return result?result:xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),width*4);}
static int frame(void *user)
{
    (void)user;xge3d_renderer *r=NULL,*fresh=NULL;xge3d_scene *s=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;xge3d_node_t node;
    GLuint pbo=0;CHECK(xge_gl_load(load_gl));
    CHECK(!xge3dRendererCreate(&r)&&!xge3dSceneCreate(&s)&&!xge3dTargetCreate(128,128,&target)&&!quad(&mesh,1));
    CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&node)&&!xge3dNodeSetMesh(s,node,mesh)&&!material(s,node,.25f,.5f,.75f,1,0));
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,2,2,.1f,10)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t d={&camera,target,{0,0,0,1},3};CHECK(!read_scene(r,s,&d)&&center(.25f,.5f,.75f,1));memcpy(reference,pixels,sizeof(pixels));
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    xge3d_color_settings_t color=xge3dColorDefault();color.tone_mapping=0;CHECK(!xge3dRendererSetColor(r,&color)&&!read_scene(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
#if XGE3D_ENABLE_TONEMAP
    const float values[]={0,.001f,.02f,.18f,.5f,1,4,32};
    for(int mode=1;mode<=2;++mode)for(size_t i=0;i<sizeof(values)/sizeof(values[0]);++i){float x=values[i];
        float y=mode==1?x/(1+x):fminf(1,(x*(2.51f*x+.03f))/(x*(2.43f*x+.59f)+.14f));color.tone_mapping=mode;
        CHECK(!material(s,node,0,0,0,1,x)&&!xge3dRendererSetColor(r,&color)&&!read_scene(r,s,&d)&&center(y,y,y,1));}
#else
    color.tone_mapping=1;CHECK(xge3dRendererSetColor(r,&color)==XGE_ERROR_UNSUPPORTED);
#endif
    color=xge3dColorDefault();color.tone_mapping=0;CHECK(!material(s,node,.25f,.5f,.75f,1,0));
#if XGE3D_ENABLE_COLOR_GRADING
    color.saturation=0;CHECK(!xge3dRendererSetColor(r,&color)&&!read_scene(r,s,&d)&&center(.4649f,.4649f,.4649f,1));
    color.saturation=1;color.gain=(xge3d_vec3_t){2,.5f,1};color.contrast=1.25f;
    CHECK(!xge3dRendererSetColor(r,&color)&&!read_scene(r,s,&d)&&center(.58f,.2675f,.8925f,1));
    d.clear_color[3]=0;CHECK(!material(s,node,.25f,.5f,.75f,.5f,0)&&!read_scene(r,s,&d)&&center(.58f,.2675f,.8925f,.5f));d.clear_color[3]=1;
#else
    color.saturation=0;CHECK(xge3dRendererSetColor(r,&color)==XGE_ERROR_UNSUPPORTED);
#endif
    xge3d_color_settings_t invalid=color;invalid.size=0;CHECK(xge3dRendererSetColor(r,&invalid)==XGE_ERROR_INVALID_ARGUMENT);
    invalid=color;invalid.gain.x=NAN;CHECK(xge3dRendererSetColor(r,&invalid)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xge3dRendererSetColor(r,NULL)&&!material(s,node,.25f,.5f,.75f,1,0)&&!read_scene(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
#endif
#if XGE3D_ENABLE_BLOOM
    xge3d_bloom_info_t info={.size=sizeof(info)};CHECK(!xge3dRendererBloomInfo(r,&info)&&!info.levels&&!info.passes&&!info.storage_bytes);
    xge3d_bloom_settings_t bloom=xge3dBloomDefault();bloom.strength=.5f;bloom.threshold=bloom.knee=0;
    CHECK(!xge3dRendererSetBloom(r,&bloom)&&!xge3dRendererBloomInfo(r,&info)&&!info.storage_bytes);
    CHECK(!read_scene(r,s,&d)&&center(.375f,.75f,1.125f,1)&&!xge3dRendererBloomInfo(r,&info));
    CHECK(info.levels==4&&info.passes==7&&info.storage_bytes==43520);
    xge3d_bloom_settings_t bad=bloom;bad.levels=7;CHECK(xge3dRendererSetBloom(r,&bad)==XGE_ERROR_INVALID_ARGUMENT);
    bad=bloom;bad.threshold=NAN;CHECK(xge3dRendererSetBloom(r,&bad)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!read_scene(r,s,&d)&&center(.375f,.75f,1.125f,1));
    bloom.knee=64;CHECK(!xge3dRendererSetBloom(r,&bloom)&&!read_scene(r,s,&d)&&center(.375f,.75f,1.125f,1));bloom.knee=0;
    bloom.threshold=2;CHECK(!xge3dRendererSetBloom(r,&bloom)&&!read_scene(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    d.clear_flags=0;CHECK(xge3dRender(r,s,&d,NULL)==XGE_ERROR_UNSUPPORTED);d.clear_flags=3;
    CHECK(!xge3dRendererSetBloom(r,NULL)&&!xge3dRendererBloomInfo(r,&info)&&!info.passes&&!info.levels&&!info.storage_bytes);
    CHECK(!read_scene(r,s,&d)&&!memcmp(reference,pixels,sizeof(pixels)));
    /* Transparent glow must extend coverage and remain display-premultiplied. */
    CHECK(!quad(&mesh,.06f)&&!material(s,node,0,0,0,1,8));d.clear_color[3]=0;
    CHECK(!read_scene(r,s,&d));memcpy(reference,pixels,sizeof(pixels));bloom=xge3dBloomDefault();bloom.strength=.5f;
    CHECK(!xge3dRendererSetBloom(r,&bloom)&&!read_scene(r,s,&d));int halo=0;
    for(size_t i=0;i<sizeof(pixels);i+=4){halo+=!reference[i+3]&&pixels[i+3]>0;for(int k=0;k<3;++k)CHECK(pixels[i+k]<=pixels[i+3]+1);}
    CHECK(halo>100);CHECK(!xgeImageSavePNGEx("artifacts/xge-light-effects/bloom.png",128,128,pixels,512,XGE_IMAGE_PREMULTIPLIED));
    CHECK(!material(s,node,0,0,0,1,0));bloom.threshold=0;bloom.knee=64;
    CHECK(!xge3dRendererSetBloom(r,&bloom)&&!read_scene(r,s,&d));
    for(size_t i=0;i<sizeof(pixels);i+=4)if(!reference[i+3])CHECK(!pixels[i+3]);
    CHECK(!material(s,node,0,0,0,1,8));bloom=xge3dBloomDefault();bloom.strength=.5f;CHECK(!xge3dRendererSetBloom(r,&bloom));
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    color=xge3dColorDefault();CHECK(!xge3dRendererSetColor(r,&color));
#endif
#if XGE3D_ENABLE_FXAA
    CHECK(!xge3dRendererSetAntialiasing(r,XGE3D_AA_FXAA)&&!read_scene(r,s,&d));
    for(size_t i=0;i<sizeof(pixels);i+=4)for(int k=0;k<3;++k)CHECK(pixels[i+k]<=pixels[i+3]+1);
    CHECK(!xge3dRendererSetAntialiasing(r,XGE3D_AA_NONE));
#endif
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    CHECK(!xge3dRendererSetColor(r,NULL));
#endif
    CHECK(!quad(&mesh,1)&&!material(s,node,.25f,.5f,.75f,1,0));d.clear_color[3]=1;bloom.threshold=bloom.knee=0;bloom.levels=6;
    CHECK(!xge3dRendererSetBloom(r,&bloom));const int sizes[][2]={{127,95},{1,1},{96,80},{128,128}};
    for(int i=0;i<4;++i){CHECK(!xge3dTargetResize(target,sizes[i][0],sizes[i][1])&&!read_scene(r,s,&d)&&center(.375f,.75f,1.125f,1));
        int count=0,w=sizes[i][0],h=sizes[i][1];uint64_t bytes=0;do{w=(w+1)/2;h=(h+1)/2;bytes+=(uint64_t)w*h*8;++count;}while(count<6&&(w>1||h>1));
        CHECK(!xge3dRendererBloomInfo(r,&info)&&info.levels==(uint32_t)count&&info.passes==(uint32_t)(2*count-1)&&info.storage_bytes==bytes);}
    xallocationbudget *quota=NULL;xallocationscope scope={0};CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    CHECK(!read_scene(r,s,&d)&&!scope.Rejections);CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);
    /* Force pyramid allocation while an unpack PBO and foreign GL state exist. */
    xge_glGenBuffers(1,&pbo);xge_glBindBuffer(0x88EC,pbo);xge_glBufferData(0x88EC,64,NULL,GL_STATIC_DRAW);
    xge_glEnable(GL_SCISSOR_TEST);xge_glScissor(3,4,5,6);xge_glDepthMask(GL_FALSE);xge_glActiveTexture(GL_TEXTURE0+3);
    bloom.levels=3;CHECK(!xge3dRendererSetBloom(r,&bloom)&&!read_scene(r,s,&d));GLint state;
    xge_glGetIntegerv(0x88EF,&state);CHECK(state==(GLint)pbo);xge_glGetIntegerv(0x84E0,&state);CHECK(state==GL_TEXTURE0+3);
    xge_glGetIntegerv(GL_SCISSOR_TEST,&state);CHECK(state);xge_glGetIntegerv(0x0B72,&state);CHECK(!state);
    xge_glBindBuffer(0x88EC,0);xge_glDisable(GL_SCISSOR_TEST);xge_glDepthMask(GL_TRUE);
    /* Lazy shader allocation failure restores GL state and retries cleanly. */
    CHECK(!xge3dRendererCreate(&fresh)&&!read_scene(fresh,s,&d)&&!xge3dRendererSetBloom(fresh,&bloom));
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));int error=xge3dRender(fresh,s,&d,NULL);
    CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);CHECK(error==XGE_ERROR_OUT_OF_MEMORY);
    CHECK(!xge3dRendererBloomInfo(fresh,&info)&&!info.storage_bytes&&!info.passes&&!read_scene(fresh,s,&d));
    bloom.strength=0;CHECK(!xge3dRendererSetBloom(r,&bloom)&&!xge3dRendererBloomInfo(r,&info)&&!info.storage_bytes&&!read_scene(r,s,&d)&&center(.25f,.5f,.75f,1));
    printf("Bloom: %d transparent halo pixels; disabled=0 passes/0 texture bytes\n",halo);
#endif
#if XGE_ENABLE_2D
    /* Later UI is submitted after postprocessing and retains its exact color. */
#if XGE3D_ENABLE_BLOOM
    bloom.strength=.5f;CHECK(!xge3dRendererSetBloom(r,&bloom));
#endif
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    color=xge3dColorDefault();CHECK(!xge3dRendererSetColor(r,&color));
#endif
    d.target=NULL;CHECK(!xge3dRender(r,s,&d,NULL));xgeShapeRectFillPx((xge_rect_t){0,0,20,20},XGE_COLOR_RGBA(0,255,0,255));CHECK(!xgeFlush());
    unsigned char ui[4];xge_glReadPixels(5,xgeGetHeight()-5,1,1,GL_RGBA,GL_UNSIGNED_BYTE,ui);CHECK(ui[0]==0&&ui[1]==255&&ui[2]==0&&ui[3]==255);
#endif
    CHECK(!xge_glGetError());failed=0;puts("Light effects GPU: numerical tone/grade, defaults/toggles, bloom energy/alpha/levels, lazy resources/OOM, resize and GL state passed");
done:
    xge_glBindBuffer(0x88EC,0);if(pbo)xge_glDeleteBuffers(1,&pbo);xge3dRendererFree(fresh);xge3dRendererFree(r);xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dTargetFree(target);return 1;
}
int main(void)
{xge_desc_t d={.iWidth=180,.iHeight=180,.sTitle="XGE lightweight effects GPU verification"};if(xgeInit(&d))return 1;int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
