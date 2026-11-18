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

    unsigned char pixels[16]={0};
    xge3d_texture *texture=NULL;
    assert(xge3dTextureCreateRGBAOwned(2,2,pixels,sizeof(pixels),0,NULL,&texture)==XGE_OK);
    xge3d_texture_mapping_t mapping=xge3dTextureMappingDefault();
    mapping.texcoord=1;
    assert(xge3dMaterialSetMap(material,XGE3D_MAP_BASE_COLOR,texture,&mapping)==XGE_OK);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK);
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
