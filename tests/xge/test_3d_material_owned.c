#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static xge3d_scene *scene;
static xge3d_node_t node;
static xge3d_material *material;

typedef struct thread_probe {
    xge3d_material *material;
    xge3d_texture *texture;
    xge3d_shader *shader;
} thread_probe;

static int wrong_thread(void *data)
{
    thread_probe *probe=data;
    bool consumed=true;
    assert(xge3dMaterialReleaseOwnerChecked(probe->material,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dTextureReleaseOwnerChecked(probe->texture,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dShaderReleaseOwnerChecked(probe->shader,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    xge3d_material_params_t params;
    assert(xge3dMaterialGetParams(probe->material,&params)==XGE_ERROR_INVALID_STATE);
    xge3d_material_state_t state={.settings=xge3dMaterialSettingsDefault()};
    assert(xge3dMaterialSetState(probe->material,&state)==XGE_ERROR_INVALID_STATE);
    return 0;
}

static int frame(void *unused)
{
    (void)unused;
    xge3d_shader_desc_t desc={sizeof(desc),NULL,
        "void xge3dSurface(inout vec4 base,inout vec3 emission,vec2 uv){base.rgb=base.gbr*xge3dParams[0].x;}",0};
    xge3d_shader *shader=NULL,*alias=NULL;
    uint64_t owners=xgeGraphicsOwnedResourceCount();
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dShaderCreateOwned(&desc,&shader)==XGE_ERROR_OUT_OF_MEMORY && !shader);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==owners);
    assert(xge3dShaderCreateOwned(&desc,&shader)==XGE_OK);
    assert(xge3dShaderShareOwned(shader,&alias)==XGE_OK && alias==shader);
    float parameters[16]={1};
    assert(xge3dMaterialSetShader(material,alias,parameters)==XGE_OK);
    assert(xge3dShaderReleaseChecked(alias)==XGE_OK);

    xge3d_texture *texture=NULL;
    assert(xge3dMaterialMapTextureOwned(material,XGE3D_MAP_BASE_COLOR,&texture)==XGE_OK && texture);
    thread_probe probe={material,texture,shader};
    xthread *worker=xrtThreadCreate(wrong_thread,&probe,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xge3dShaderReleaseChecked(shader)==XGE_OK);

    xge3d_upload_budget_t budget={.bytes=64,.operations=4};
    xge3d_upload_stats_t prepared;
    assert(xge3dTexturePrepare(texture,&budget,&prepared)==XGE_OK && prepared.completed);
    const unsigned char green[20]={0,255,0,255,0,255,0,255,1,2,3,4,0,255,0,255,0,255,0,255};
    assert(xge3dTextureUpdateRGBA(texture,0,0,2,2,green,sizeof(green)-1,12)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dTextureUpdateRGBA(texture,0,0,2,2,green,sizeof(green),12)==XGE_OK);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK);

    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreateOwned(32,32,&target)==XGE_OK);
    assert(xge3dTextureFromTargetOwned(target,&texture)==XGE_OK);
    assert(xge3dMaterialSetMap(material,XGE3D_MAP_NORMAL,texture,NULL)==XGE_ERROR_UNSUPPORTED);
    xge3d_texture *unset=texture;
    assert(xge3dMaterialMapTextureOwned(material,XGE3D_MAP_NORMAL,&unset)==XGE_OK && !unset);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK);

    const float clear[4]={0,0,0,1};
    xge3d_camera_t camera;
    assert(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    assert(xge3dRenderFrameTo(renderer,scene,&camera,target,clear,3,1,NULL)==XGE_OK);
    unsigned char pixels[32*32*4];
    assert(xge3dTargetReadPixels(target,pixels,sizeof(pixels),0)==XGE_OK);
    size_t center=(16*32+16)*4;
    assert(pixels[center]>200 && pixels[center+1]<32 && pixels[center+2]<32);
    assert(xge3dTargetReleaseChecked(target)==XGE_OK);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="XGE managed 3D material"};
    assert(xgeInit(&window)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMaterialCreateOwned(NULL,&material)==XGE_ERROR_OUT_OF_MEMORY && !material);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==0);
    xge3d_material_params_t params=xge3dMaterialParamsDefault();
    params.unlit=1;
    assert(xge3dMaterialCreateOwned(&params,&material)==XGE_OK);
    params.roughness=-1;
    assert(xge3dMaterialSetParams(material,&params)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dMaterialGetParams(material,&params)==XGE_OK && params.roughness==1);

    unsigned char pixels[24]={0};
    xge_image_t image={.iWidth=2,.iHeight=2,.iStride=12,.iFormat=XGE_PIXEL_RGBA8,.pPixels=pixels};
    xge3d_texture *texture=NULL;
    image.iFlags=XGE_IMAGE_PREMULTIPLIED;
    assert(xge3dTextureCreateFromImageOwned(&image,0,NULL,&texture)==XGE_ERROR_INVALID_ARGUMENT && !texture);
    image.iFlags=0;
    for(int prefix=0;prefix<2;++prefix){
        assert(xrtMemDebugFailAfter(prefix));
        assert(xge3dTextureCreateFromImageOwned(&image,0,NULL,&texture)==XGE_ERROR_OUT_OF_MEMORY && !texture);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        assert(xgeGraphicsOwnedResourceCount()==1);
    }
    assert(xge3dTextureCreateFromImageOwned(&image,0,NULL,&texture)==XGE_OK);
    xge3d_resource_info_t image_info={.size=sizeof(image_info)};
    assert(xge3dTextureResourceInfo(texture,&image_info)==XGE_OK && image_info.cpu_payload_bytes==16);
    xge3d_material_state_t state={.settings=xge3dMaterialSettingsDefault(),.base_color=texture,.emissive=texture};
    state.settings.params.unlit=1;
    state.settings.params.roughness=.5f;
    state.settings.maps[XGE3D_MAP_BASE_COLOR].texcoord=1;
    state.settings.maps[XGE3D_MAP_OCCLUSION].offset[0]=.25f;
    xge3d_material *second=NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMaterialCreateStateOwned(&state,&second)==XGE_ERROR_OUT_OF_MEMORY && !second);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==2);
    assert(xge3dMaterialCreateStateOwned(&state,&second)==XGE_OK);
    assert(xge3dMaterialReleaseChecked(second)==XGE_OK);
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMaterialSetState(material,&state)==XGE_OK);
    assert(xrtMemDebugFailAfter(-1));
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount);
    xge3d_material_state_t invalid={.settings=state.settings};
    invalid.settings.params.roughness=.75f;
    invalid.settings.maps[XGE3D_MAP_EMISSIVE].texcoord=2;
    assert(xge3dMaterialSetState(material,&invalid)==XGE_ERROR_INVALID_ARGUMENT);
    xge3d_material_settings_t settings;
    assert(xge3dMaterialGetSettings(material,&settings)==XGE_OK);
    assert(settings.params.roughness==.5f && settings.maps[XGE3D_MAP_BASE_COLOR].texcoord==1);
    assert(settings.maps[XGE3D_MAP_OCCLUSION].offset[0]==.25f);
    xge3d_texture *emissive=NULL;
    assert(xge3dMaterialMapTextureOwned(material,XGE3D_MAP_EMISSIVE,&emissive)==XGE_OK && emissive==texture);
    assert(xge3dTextureReleaseChecked(emissive)==XGE_OK);
    state.emissive=NULL;
    assert(xge3dMaterialSetState(material,&state)==XGE_OK);
    assert(xge3dMaterialMapTextureOwned(material,XGE3D_MAP_EMISSIVE,&emissive)==XGE_OK && !emissive);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK);
    xge3d_texture_mapping_t mapping;
    assert(xge3dMaterialGetMapping(material,XGE3D_MAP_BASE_COLOR,&mapping)==XGE_OK);
    assert(mapping.texcoord==1 && mapping.scale[0]==1 && mapping.scale[1]==1);

    xge3d_vertex_t vertices[3]={
        {.position={-.8f,-.8f,-2},.normal={0,0,1}},
        {.position={.8f,-.8f,-2},.normal={0,0,1}},
        {.position={0,.8f,-2},.normal={0,0,1}}
    };
    xge3d_mesh *mesh=NULL;
    assert(xge3dMeshCreateOwned(vertices,3,NULL,0,&mesh)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    xge3d_node_t root;
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&root)==XGE_OK);
    assert(xge3dNodeCreate(scene,root,&node)==XGE_OK);
    assert(xge3dNodeSetMesh(scene,node,mesh)==XGE_OK);
    assert(xge3dMeshReleaseChecked(mesh)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dNodeSetMaterialTree(scene,root,material)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xge3d_material *unset=material;
    assert(xge3dNodeGetMaterialOwned(scene,node,&unset)==XGE_OK && !unset);
    assert(xge3dNodeSetMaterialTree(scene,root,material)==XGE_OK);
    assert(xge3dMaterialReleaseChecked(material)==XGE_OK);
    assert(xge3dNodeGetMaterialOwned(scene,node,&material)==XGE_OK && material);
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==3 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dMaterialReleaseChecked(material)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0 && xgeUnitChecked()==XGE_OK);
    puts("MATERIAL3D_NATIVE_GPU_OK");
    return 0;
}
