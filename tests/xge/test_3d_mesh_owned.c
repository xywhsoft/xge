#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static const xge3d_vertex_t triangle[]={
    {.position={-.8f,-.8f,-2},.normal={0,0,1}},
    {.position={.8f,-.8f,-2},.normal={0,0,1}},
    {.position={0,.8f,-2},.normal={0,0,1}}
};
static const uint16_t indices[]={0,1,2};
static const uint32_t indices32[]={0,1,2}, invalid32[]={0,1,3};
static xge3d_mesh *mesh;
static xge3d_scene *scene;
static xge3d_node_t node;

static xge3d_resource_info_t info(xge3d_mesh *value)
{
    xge3d_resource_info_t result={.size=sizeof(result)};
    assert(xge3dMeshResourceInfo(value,&result)==XGE_OK);
    return result;
}

static int wrong_thread(void *arg)
{
    xge3d_mesh *value=arg,*alias=value;
    bool consumed=true;
    xge3d_vertex_t item;
    assert(xge3dMeshVertexAt(value,0,&item)==XGE_ERROR_INVALID_STATE);
    assert(xge3dMeshShareOwned(value,&alias)==XGE_ERROR_INVALID_STATE && !alias);
    assert(xge3dMeshReleaseOwnerChecked(value,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dMeshUpdateU16(value,triangle,3,indices,3)==XGE_ERROR_INVALID_STATE);
    return 0;
}

static int frame(void *unused)
{
    (void)unused;
    xge3d_upload_budget_t budget={.operations=1,.bytes=8};
    xge3d_upload_stats_t upload={0};
    assert(xge3dMeshPrepare(mesh,&budget,&upload)==XGE_OK);
    assert(upload.operations==1 && !upload.completed);
    budget.bytes=1024;
    budget.operations=4;
    assert(xge3dMeshPrepare(mesh,&budget,&upload)==XGE_OK && upload.completed);

    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMeshUpdateU32(mesh,triangle,3,indices32,3)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(info(mesh).cpu_payload_bytes==sizeof(triangle)+sizeof(indices));
    assert(xge3dMeshUpdateU32(mesh,triangle,3,invalid32,3)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dMeshUpdateU32(mesh,triangle,3,indices32,3)==XGE_OK);
    assert(info(mesh).cpu_payload_bytes==sizeof(triangle)+sizeof(indices32));

    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    xge3d_camera_t camera;
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreateOwned(32,32,&target)==XGE_OK);
    assert(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    xge3d_render_stats_t stats;
    float clear[4]={0,0,0,1};
    assert(xge3dRenderFrameTo(renderer,scene,&camera,target,clear,3,1,&stats)==XGE_OK);
    assert(stats.triangles==1);
    unsigned char pixels[32*32*4];
    assert(xge3dTargetReadPixels(target,pixels,sizeof(pixels),0)==XGE_OK);
    size_t center=(16*32+16)*4;
    assert(pixels[center] || pixels[center+1] || pixels[center+2]);
    assert(xge3dTargetReleaseChecked(target)==XGE_OK);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t desc={.iWidth=64,.iHeight=64,.sTitle="XGE managed 3D mesh"};
    assert(xgeInit(&desc)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dMeshCreateOwned(triangle,3,indices,3,&mesh)==XGE_ERROR_OUT_OF_MEMORY && !mesh);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==0);

    xge3d_mesh *raw=NULL,*promoted=NULL;
    xge3d_mesh_desc_t raw_desc={triangle,3,indices32,3,32};
    assert(xge3dMeshCreate(&raw_desc,&raw)==XGE_OK);
    assert(xge3dMeshShareOwned(raw,&promoted)==XGE_OK && promoted==raw);
    xge3dMeshFree(raw);
    assert(info(promoted).references==1 && xgeGraphicsOwnedResourceCount()==1);
    assert(xge3dMeshReleaseChecked(promoted)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0);

    assert(xge3dMeshCreateOwned(triangle,3,indices,3,&mesh)==XGE_OK);
    assert(info(mesh).gpu_payload_bytes==0);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dNodeCreate(scene,(xge3d_node_t){0},&node)==XGE_OK);
    xge3d_mesh *alias=mesh;
    assert(xge3dNodeGetMeshOwned(scene,node,&alias)==XGE_OK && !alias);
    assert(xge3dNodeSetMesh(scene,node,mesh)==XGE_OK);
    assert(xge3dNodeGetMeshOwned(scene,node,&alias)==XGE_OK && alias==mesh);
    assert(info(mesh).references==3 && xgeGraphicsOwnedResourceCount()==1);

    xthread *worker=xrtThreadCreate(wrong_thread,mesh,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(info(mesh).references==3);
    assert(xge3dMeshReleaseChecked(alias)==XGE_OK);
    assert(xgeRun(frame,NULL)==XGE_OK);

    assert(xge3dMeshReleaseChecked(mesh)==XGE_OK);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dNodeGetMeshOwned(scene,node,&alias)==XGE_OK && alias==mesh);
    assert(info(alias).references==2 && info(alias).gpu_payload_bytes==0);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    xge3d_vertex_t vertex;
    uint32_t index;
    assert(xge3dMeshVertexAt(alias,2,&vertex)==XGE_OK && vertex.position.y==.8f);
    assert(xge3dMeshIndexAt(alias,2,&index)==XGE_OK && index==2);
    assert(xge3dMeshIndexAt(alias,3,&index)==XGE_ERROR_NOT_FOUND);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dMeshReleaseChecked(alias)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0 && xgeUnitChecked()==XGE_OK);
    puts("MESH3D_NATIVE_GPU_OK");
    return 0;
}
