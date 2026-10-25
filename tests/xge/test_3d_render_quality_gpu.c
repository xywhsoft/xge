#include "xge.h"
#define XGE_GL_IMPL
#include "xge_gl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"quality %d: %s (GL %x)\n",__LINE__,#x,xge_glGetError());goto done;}}while(0)
static int failed=1;
static void *load_gl(const char *name)
{
    HMODULE gl=GetModuleHandleW(L"opengl32.dll");FARPROC address=GetProcAddress(gl,name);
    if(!address){typedef PROC (WINAPI *getter)(LPCSTR);getter get;FARPROC raw=GetProcAddress(gl,"wglGetProcAddress");memcpy(&get,&raw,sizeof(get));address=get(name);}
    void *pointer;memcpy(&pointer,&address,sizeof(pointer));return pointer;
}
static unsigned char pixels[128*128*4],before[sizeof(pixels)];
static int encode(float v){return (int)lroundf(255*(v<=.0031308f?12.92f*v:1.055f*powf(v,1/2.4f)-.055f));}
static int close_byte(int a,int b){return abs(a-b)<=2;}
static int center(int r,int g,int b,int a)
{unsigned char *p=pixels+(64*128+64)*4;if(!close_byte(p[0],r)||!close_byte(p[1],g)||!close_byte(p[2],b)||!close_byte(p[3],a)){fprintf(stderr,"pixel %u %u %u %u, expected %d %d %d %d\n",p[0],p[1],p[2],p[3],r,g,b,a);return 0;}return 1;}
static int put_material(xge3d_scene *s,xge3d_node_t n,float r,float g,float b,float a,float emission)
{xge3d_material_desc_t d=xge3dMaterialDefault();d.unlit=1;d.double_sided=1;d.base_color[0]=r;d.base_color[1]=g;d.base_color[2]=b;d.base_color[3]=a;d.alpha_mode=a<1?XGE3D_ALPHA_BLEND:XGE3D_ALPHA_OPAQUE;
 d.emissive[0]=d.emissive[1]=d.emissive[2]=emission;xge3d_material *m=NULL;int result=xge3dMaterialCreate(&d,&m);if(!result)result=xge3dNodeSetMaterial(s,n,m);xge3dMaterialFree(m);return result;}
static int read_target(xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *d)
{int result=xge3dRender(r,s,d,NULL);if(result){fprintf(stderr,"render %d\n",result);return result;}return xge3dTargetReadPixels(d->target,pixels,sizeof(pixels),512);}
static int frame(void *user)
{
    (void)user;xge3d_renderer *r=NULL,*fresh=NULL;xge3d_scene *s=NULL;xge3d_target *target=NULL;xge3d_mesh *mesh=NULL;
    if(!xge_gl_load(load_gl)){return 1;}
    GLuint fbo=0,depth=0,pbo=0,raw_color=0;xge3d_node_t n; xge3d_vertex_t v[3]={0};
    v[0].position=(xge3d_vec3_t){-1,-1,0};v[1].position=(xge3d_vec3_t){3,-1,0};v[2].position=(xge3d_vec3_t){-1,3,0};
    xge3d_mesh_desc_t md={v,3};CHECK(!xge3dRendererCreate(&r)&&!xge3dRendererCreate(&fresh)&&!xge3dSceneCreate(&s)&&!xge3dTargetCreate(128,128,&target));
    CHECK(!xge3dMeshCreate(&md,&mesh)&&!xge3dNodeCreate(s,(xge3d_node_t){0},&n)&&!xge3dNodeSetMesh(s,n,mesh));
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,2,2,.1f,10)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,3},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t d={&camera,target,{0,0,1,1},3};
    CHECK(!put_material(s,n,1,0,0,.5f,0)&&!read_target(r,s,&d)&&center(encode(.5f),0,encode(.5f),255));
    d.clear_color[2]=d.clear_color[3]=0;CHECK(!put_material(s,n,.25f,.5f,1,.5f,0)&&!read_target(r,s,&d)&&center(encode(.25f)/2,encode(.5f)/2,128,128));
    d.clear_color[3]=1;CHECK(!put_material(s,n,0,0,0,.25f,4)&&!read_target(r,s,&d)&&center(255,255,255,255));
    /* Exposure is linear before blending; the public clear color is encoded. */
    d.clear_color[3]=1;d.exposure=2;CHECK(!put_material(s,n,.1f,.1f,.1f,.5f,0)&&!read_target(r,s,&d)&&center(encode(.1f),encode(.1f),encode(.1f),255));d.exposure=0;
    CHECK(!put_material(s,n,1,0,0,1,0)&&!read_target(r,s,&d)&&center(255,0,0,255));
    xge3d_transform_t transform=XGE3D_TRANSFORM_IDENTITY;transform.position.z=-1;CHECK(!xge3dNodeSetTransform(s,n,&transform)&&!put_material(s,n,0,0,1,1,0));
    d.clear_flags=0;CHECK(!read_target(fresh,s,&d)&&center(255,0,0,255));
    d.clear_flags=XGE3D_CLEAR_DEPTH;CHECK(!read_target(fresh,s,&d)&&center(0,0,255,255));
    d.clear_color[0]=.125f;d.clear_color[1]=.25f;d.clear_color[2]=.375f;d.clear_color[3]=.5f;
    d.clear_flags=XGE3D_CLEAR_COLOR;CHECK(!read_target(r,s,&d)&&center(32,64,96,128));
    CHECK(!xge3dNodeSetVisible(s,n,0));d.clear_flags=0;CHECK(!read_target(fresh,s,&d)&&center(32,64,96,128));
    CHECK(!xge3dNodeSetVisible(s,n,1));
    /* Raster edge/transparent silhouette, no stale history after toggling. */
    v[0].position=(xge3d_vec3_t){-.85f,-.9f,0};v[1].position=(xge3d_vec3_t){.93f,-.63f,0};v[2].position=(xge3d_vec3_t){-.23f,.87f,0};CHECK(!xge3dMeshUpdate(mesh,&md));
    CHECK(!put_material(s,n,1,1,1,1,0));d.clear_flags=3;memset(d.clear_color,0,sizeof(d.clear_color));d.clear_color[3]=1;
    CHECK(!read_target(r,s,&d));memcpy(before,pixels,sizeof(pixels));
    CHECK(!xgeImageSavePNG("artifacts/xge-render-quality/aa-none.png",128,128,pixels,512));
#if XGE3D_ENABLE_FXAA
    CHECK(xge3dRendererSetAntialiasing(r,9)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xge3dRendererSetAntialiasing(r,XGE3D_AA_FXAA)&&!read_target(r,s,&d));
    int changed=0,partial=0;for(size_t i=0;i<sizeof(pixels);i+=4){changed+=pixels[i]!=before[i];partial+=pixels[i]>5&&pixels[i]<250;CHECK(pixels[i+3]==255);}
    CHECK(changed>100&&partial>100&&center(255,255,255,255));
    CHECK(!xgeImageSavePNG("artifacts/xge-render-quality/aa-fxaa.png",128,128,pixels,512));
    d.clear_color[3]=0;CHECK(!read_target(r,s,&d));
    for(size_t i=0;i<sizeof(pixels);i+=4)CHECK(pixels[i]<=pixels[i+3]+1&&pixels[i+1]<=pixels[i+3]+1&&pixels[i+2]<=pixels[i+3]+1);
    CHECK(!xge3dRendererSetAntialiasing(r,XGE3D_AA_NONE));d.clear_color[3]=1;CHECK(!read_target(r,s,&d)&&!memcmp(before,pixels,sizeof(pixels)));
    printf("FXAA: %d changed edge pixels, %d partial edge pixels\n",changed,partial);
#endif
    /* Allocate/reallocate while foreign GL state and an unpack PBO are bound. */
    xge3dRendererFree(fresh);fresh=NULL;CHECK(!xge3dRendererCreate(&fresh));
    xge_glGenBuffers(1,&pbo);xge_glBindBuffer(0x88EC,pbo);xge_glBufferData(0x88EC,64,NULL,GL_STATIC_DRAW);
    xge_glEnable(GL_SCISSOR_TEST);xge_glScissor(3,4,5,6);xge_glEnable(GL_BLEND);xge_glDepthMask(GL_FALSE);xge_glDepthFunc(0x0204);xge_glActiveTexture(GL_TEXTURE0+3);
    CHECK(!read_target(fresh,s,&d));d.clear_flags=0;CHECK(!read_target(fresh,s,&d));d.clear_flags=3;GLint value;
    xge_glGetIntegerv(0x88EF,&value);CHECK(value==(GLint)pbo);xge_glGetIntegerv(0x84E0,&value);CHECK(value==GL_TEXTURE0+3);
    xge_glGetIntegerv(GL_SCISSOR_TEST,&value);CHECK(value);xge_glGetIntegerv(0x0B72,&value);CHECK(!value);xge_glGetIntegerv(0x0B74,&value);CHECK(value==0x0204);
    xge_glBindBuffer(0x88EC,0);xge_glDisable(GL_SCISSOR_TEST);xge_glDisable(GL_BLEND);xge_glDepthMask(GL_TRUE);xge_glDepthFunc(GL_LESS);
    CHECK(!xge3dTargetResize(target,96,80)&&!xge3dRender(r,s,&d,NULL)&&!xge3dTargetResize(target,128,128)&&!read_target(r,s,&d));
    /* Borrowed destination FBO, offset viewport, preserve color and depth. */
    xge_glGenFramebuffers(1,&fbo);xge_glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,xge3dTargetTexture(target)->iBackendId,0);
    xge_glGenRenderbuffers(1,&depth);xge_glBindRenderbuffer(GL_RENDERBUFFER,depth);xge_glRenderbufferStorage(GL_RENDERBUFFER,0x81A6,128,128);
    xge_glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);CHECK(xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    d.target=NULL;xge_glViewport(11,13,80,70);CHECK(!xge3dRender(r,s,&d,NULL));d.clear_flags=0;CHECK(!xge3dRender(fresh,s,&d,NULL));
    xge_glViewport(-1,13,80,70);CHECK(xge3dRender(r,s,&d,NULL)==XGE_ERROR_INVALID_ARGUMENT);xge_glViewport(11,13,80,70);
    GLint viewport[4];xge_glGetIntegerv(0x0BA2,viewport);CHECK(viewport[0]==11&&viewport[1]==13&&viewport[2]==80&&viewport[3]==70);
    /* Native depth formats must survive copy-in/copy-out, including packed stencil. */
    const GLenum formats[]={0x81A5,0x81A6,0x8CAC,0x88F0,0x8CAD};
    for(size_t i=0;i<sizeof(formats)/sizeof(formats[0]);++i){xge_glBindFramebuffer(GL_FRAMEBUFFER,fbo);xge_glBindRenderbuffer(GL_RENDERBUFFER,depth);xge_glRenderbufferStorage(GL_RENDERBUFFER,formats[i],128,128);
        CHECK(xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);d.clear_flags=3;int error=xge3dRender(r,s,&d,NULL);
        if(error){fprintf(stderr,"depth format %x result %d\n",formats[i],error);}CHECK(!error);d.clear_flags=0;CHECK(!xge3dRender(fresh,s,&d,NULL));}
    /* An external sRGB attachment stores the same encoded bytes, without double encoding. */
    xge_glGenTextures(1,&raw_color);xge_glBindTexture(GL_TEXTURE_2D,raw_color);xge_glTexImage2D(GL_TEXTURE_2D,0,0x8C43,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    xge_glBindFramebuffer(GL_FRAMEBUFFER,fbo);xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,raw_color,0);
    xge_glEnable(0x8DB9);
    CHECK(!put_material(s,n,.25f,.25f,.25f,1,0));d.clear_flags=3;CHECK(!xge3dRender(r,s,&d,NULL));
    unsigned char sample[4];xge_glReadPixels(51,48,1,1,GL_RGBA,GL_UNSIGNED_BYTE,sample);CHECK(close_byte(sample[0],encode(.25f)));
    d.clear_flags=0;CHECK(!xge3dRender(fresh,s,&d,NULL));xge_glReadPixels(51,48,1,1,GL_RGBA,GL_UNSIGNED_BYTE,sample);CHECK(close_byte(sample[0],encode(.25f)));
    xge_glGetIntegerv(0x8DB9,&value);CHECK(value);xge_glDisable(0x8DB9);
    xge_glBindFramebuffer(GL_FRAMEBUFFER,0);xge_glViewport(0,0,xgeGetWidth(),xgeGetHeight());d.clear_flags=3;CHECK(!xge3dRender(r,s,&d,NULL));
    d.clear_flags=0;CHECK(!xge3dRender(fresh,s,&d,NULL));
    CHECK(!xge_glGetError());failed=0;puts("Render quality: linear blends/exposure/emission, premultiplied output, preserved color/depth, resize, viewport, window and GL-state checks passed");
done:
    xge_glBindBuffer(0x88EC,0);xge_glBindFramebuffer(GL_FRAMEBUFFER,0);
    if(fbo){xge_glDeleteFramebuffers(1,&fbo);}if(depth){xge_glDeleteRenderbuffers(1,&depth);}if(pbo){xge_glDeleteBuffers(1,&pbo);}if(raw_color){xge_glDeleteTextures(1,&raw_color);}
    xge3dSceneFree(s);xge3dMeshFree(mesh);xge3dTargetFree(target);xge3dRendererFree(r);xge3dRendererFree(fresh);return 1;
}
int main(void)
{xge_desc_t d={0};d.iWidth=d.iHeight=180;d.sTitle="XGE linear compositor / FXAA";if(xgeInit(&d))return 1;int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
