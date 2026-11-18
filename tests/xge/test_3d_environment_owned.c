#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#define GL_CALL WINAPI
#else
#include <dlfcn.h>
#define GL_CALL
#endif

static xge3d_environment *environment;

static void *load_gl(const char *name)
{
#ifdef _WIN32
    HMODULE module=GetModuleHandleW(L"opengl32.dll");
    FARPROC proc=GetProcAddress(module,name);
    if(!proc){
        typedef PROC (WINAPI *lookup)(LPCSTR);
        FARPROC raw=GetProcAddress(module,"wglGetProcAddress");
        lookup get;
        memcpy(&get,&raw,sizeof(get));
        proc=get(name);
    }
    void *address;
    memcpy(&address,&proc,sizeof(address));
    return address;
#else
    static void *module;
    if(!module)module=dlopen("libGL.so.1",RTLD_NOW|RTLD_LOCAL);
    return module?dlsym(module,name):NULL;
#endif
}

static int wrong_thread(void *raw)
{
    xge3d_environment *value=raw,*alias=value;
    bool consumed=true;
    assert(xge3dEnvironmentShareOwned(value,&alias)==XGE_ERROR_INVALID_STATE && !alias);
    assert(xge3dEnvironmentReleaseOwnerChecked(value,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dEnvironmentClearIBL(value)==XGE_ERROR_INVALID_STATE);
    return 0;
}

static void check_unpack(xge3d_renderer *renderer,xge3d_scene *scene,const xge3d_camera_t *camera,
    xge3d_frame_options_t *options,xge3d_target *target,xge3d_render_stats_t *stats)
{
    typedef void (GL_CALL *buffers)(int,unsigned int*);
    typedef void (GL_CALL *bind_buffer)(unsigned int,unsigned int);
    typedef void (GL_CALL *buffer_data)(unsigned int,ptrdiff_t,const void*,unsigned int);
    typedef void (GL_CALL *set_store)(unsigned int,int);
    typedef void (GL_CALL *get_integer)(unsigned int,int*);
    buffers generate=(buffers)load_gl("glGenBuffers");
    buffers remove=(buffers)load_gl("glDeleteBuffers");
    bind_buffer bind=(bind_buffer)load_gl("glBindBuffer");
    buffer_data allocate=(buffer_data)load_gl("glBufferData");
    set_store store=(set_store)load_gl("glPixelStorei");
    get_integer get=(get_integer)load_gl("glGetIntegerv");
    assert(generate&&remove&&bind&&allocate&&store&&get);
    const unsigned int keys[]={0x0CF5,0x0CF2,0x0CF3,0x0CF4,0x88EF};
    int old[5],desired[]={8,11,2,3,0},restored;
    for(int i=0;i<5;++i)get(keys[i],&old[i]);
    unsigned int pbo;
    generate(1,&pbo);
    assert(pbo);
    bind(0x88EC,pbo);
    allocate(0x88EC,2048,NULL,0x88E4);
    desired[4]=(int)pbo;
    for(int i=0;i<4;++i)store(keys[i],desired[i]);
    assert(xge3dRenderFrameConfigured(renderer,scene,camera,options,target,environment,stats)==XGE_OK);
    for(int i=0;i<5;++i){get(keys[i],&restored);assert(restored==desired[i]);}
    for(int i=0;i<4;++i)store(keys[i],old[i]);
    bind(0x88EC,(unsigned int)old[4]);
    remove(1,&pbo);
}

static int frame(void *unused)
{
    (void)unused;
    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    xge3d_scene *scene=NULL;
    xge3d_mesh *mesh=NULL;
    xge3d_material *material=NULL;
    xge3d_node_t node,light_node;
    xge3d_camera_t camera;
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreateOwned(64,64,&target)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    xge3d_vertex_t vertices[3]={
        {.position={-.8f,-.8f,-3},.normal={0,0,1}},
        {.position={.8f,-.8f,-3},.normal={0,0,1}},
        {.position={0,.8f,-3},.normal={0,0,1}}
    };
    xge3d_material_params_t material_params=xge3dMaterialParamsDefault();
    material_params.metallic=0;
    assert(xge3dMaterialCreateOwned(&material_params,&material)==XGE_OK);
    assert(xge3dMeshCreateOwned(vertices,3,NULL,0,&mesh)==XGE_OK);
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&node)==XGE_OK);
    assert(xge3dNodeSetMesh(scene,node,mesh)==XGE_OK);
    assert(xge3dNodeSetMaterial(scene,node,material)==XGE_OK);
    assert(xge3dMeshReleaseChecked(mesh)==XGE_OK);
    assert(xge3dMaterialReleaseChecked(material)==XGE_OK);

    xge3d_frame_options_t options=xge3dFrameOptionsDefault();
    options.clear_color[3]=1;
    options.ibl_intensity=1;
    xge3d_render_stats_t stats;
    check_unpack(renderer,scene,&camera,&options,target,&stats);
    assert(stats.triangles>=1 && stats.upload_bytes>0);
    unsigned char pixels[64*64*4];
    assert(xge3dTargetReadPixels(target,pixels,sizeof(pixels),0)==XGE_OK);
    size_t center=(32*64+32)*4;
    assert(pixels[2]>220 && pixels[center]>220 && pixels[center+1]<32 && pixels[center+2]<32);
    assert(xge3dRenderFrameConfigured(renderer,scene,&camera,&options,target,environment,&stats)==XGE_OK);
    assert(stats.upload_bytes==0);

    options.enable_shadows=1;
    options.shadows.resolution=64;
    options.shadows.cascades=1;
    options.enable_fog=1;
    options.fog=(xge3d_fog_settings_t){.color={0,1,0},.start=.5f,.end=1};
    options.disable_culling=1;
    options.disable_instancing=1;
    xge3d_light_desc_t light=xge3dLightDefault(XGE3D_LIGHT_DIRECTIONAL);
    light.casts_shadow=1;
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&light_node)==XGE_OK);
    assert(xge3dNodeSetLight(scene,light_node,&light)==XGE_OK);
    assert(xge3dRenderFrameConfigured(renderer,scene,&camera,&options,target,environment,&stats)==XGE_OK);
    assert(stats.shadow_maps==1 && !stats.instanced_draw_calls);
    assert(xge3dTargetReadPixels(target,pixels,sizeof(pixels),0)==XGE_OK);
    assert(pixels[2]>220 && pixels[center]<32 && pixels[center+1]>220 && pixels[center+2]<32);
    assert(xge3dEnvironmentClearIBL(environment)==XGE_OK);
    assert(xge3dRenderFrameConfigured(renderer,scene,&camera,&options,target,environment,&stats)==XGE_ERROR_INVALID_ARGUMENT);
    assert(!stats.draw_calls);
    assert(xge3dTargetReleaseChecked(target)==XGE_OK);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="XGE managed environment"};
    assert(xgeInit(&window)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    unsigned char sky[24]={0},red[24]={0},mips[120]={0},lut[4]={0,0,0,255};
    for(size_t i=0;i<sizeof(sky);i+=4){sky[i+2]=sky[i+3]=255;red[i]=red[i+3]=255;}
    for(size_t i=0;i<sizeof(mips);i+=4)mips[i+3]=255;
    assert(xge3dEnvironmentCreateRGBAOwned(1,sky,sizeof(sky)-1,1,&environment)==XGE_ERROR_INVALID_ARGUMENT && !environment);
    assert(xrtMemDebugFailAfter(1));
    assert(xge3dEnvironmentCreateRGBAOwned(1,sky,sizeof(sky),1,&environment)==XGE_ERROR_OUT_OF_MEMORY && !environment);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==0);
    assert(xge3dEnvironmentCreateRGBAOwned(1,sky,sizeof(sky),1,&environment)==XGE_OK);
    xge3d_environment *alias=NULL;
    assert(xge3dEnvironmentShareOwned(environment,&alias)==XGE_OK && alias==environment);
    assert(xge3dEnvironmentReleaseChecked(environment)==XGE_OK);
    environment=alias;
    assert(xge3dEnvironmentSetIBLRGBA(environment,1,red,sizeof(red),2,mips,sizeof(mips),1,1,lut,sizeof(lut))==XGE_OK);
    assert(xge3dEnvironmentSetIBLRGBA(environment,1,red,sizeof(red),2,mips,sizeof(mips)-1,1,1,lut,sizeof(lut))==XGE_ERROR_INVALID_ARGUMENT);
    assert(xrtMemDebugFailAfter(1));
    assert(xge3dEnvironmentSetIBLRGBA(environment,1,sky,sizeof(sky),2,mips,sizeof(mips),1,1,lut,sizeof(lut))==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xthread *worker=xrtThreadCreate(wrong_thread,environment,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dEnvironmentReleaseChecked(environment)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0 && xgeUnitChecked()==XGE_OK);
    puts("ENVIRONMENT3D_NATIVE_GPU_OK");
    return 0;
}
