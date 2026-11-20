#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const unsigned char texture_pixels[4]={32,210,90,255};
static xge3d_renderer *after_frame_renderer;
static xge3d_texture *after_frame_texture;

static void cpu_checks(void)
{
    xge3d_particle_desc_t settings=xge3dParticlesDefault();
    settings.capacity=8;
    settings.life_min=settings.life_max=.5f;
    settings.columns=settings.rows=2;
    settings.fps=4;
    xge3d_particles *particles=NULL;
    assert(xge3dParticlesCreate(&settings,&particles)==XGE_OK);

    xge3d_billboard_batch *empty=NULL,*batch=NULL;
    assert(xge3dParticlesCollectBillboards(particles,XGE3D_BILLBOARD_ALPHA,&empty)==XGE_OK);
    const xge3d_billboard_geometry_t *geometry=NULL;
    size_t count=1;
    assert(xge3dBillboardBatchGetGeometry(empty,&geometry,&count)==XGE_OK && !geometry && !count);
    xge3dBillboardBatchFree(empty);
    assert(xge3dParticlesEmit(particles,3)==XGE_OK);
    assert(xge3dParticlesUpdate(particles,.25f,0)==XGE_OK);
    xge3d_billboard_t legacy[3];
    assert(xge3dParticlesBillboards(particles,NULL,XGE3D_BILLBOARD_ADD,legacy,3,&count)==XGE_OK && count==3);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dParticlesCollectBillboards(particles,XGE3D_BILLBOARD_ADD,&batch)==XGE_ERROR_OUT_OF_MEMORY && !batch);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xge3dParticlesCollectBillboards(particles,-1,&batch)==XGE_ERROR_INVALID_ARGUMENT && !batch);
    assert(xge3dParticlesCollectBillboards(particles,XGE3D_BILLBOARD_ADD,&batch)==XGE_OK);
    assert(xge3dBillboardBatchGetGeometry(batch,&geometry,&count)==XGE_OK && count==3);
    assert(xge3dBillboardBatchAddRef(batch)==XGE_OK);
    xge3dBillboardBatchFree(batch);
    assert(xge3dParticlesUpdate(particles,.9f,0)==XGE_OK);
    xge3dParticlesFree(particles);
    for(size_t i=0;i<count;++i){
        const xge3d_billboard_geometry_t *item=geometry+i;
        const xge3d_billboard_t *expected=legacy+i;
        assert(!memcmp(&item->position,&expected->position,sizeof(item->position)));
        assert(item->width==expected->width && item->height==expected->height &&
            item->rotation==expected->rotation && item->blend==expected->blend && item->depth_test==expected->depth_test);
        assert(item->color.r==expected->color[0] && item->color.g==expected->color[1] &&
            item->color.b==expected->color[2] && item->color.a==expected->color[3]);
        assert(item->uv.u==expected->uv[0] && item->uv.v==expected->uv[1] &&
            item->uv.width==expected->uv[2] && item->uv.height==expected->uv[3]);
    }
    xge3dBillboardBatchFree(batch);
    xge3d_texture *texture=NULL;
    assert(xge3dTextureCreateRGBAOwned(1,1,texture_pixels,3,1,NULL,&texture)==XGE_ERROR_INVALID_ARGUMENT && !texture);
    assert(xge3dTextureCreateRGBAOwned(1,1,texture_pixels,4,1,NULL,&texture)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK && xgeGraphicsOwnedResourceCount()==0);
    puts("PARTICLE_BILLBOARD_NATIVE_CPU_OK");
}

static int frame(void *user)
{
    (void)user;
    xge3d_scene *scene=NULL;
    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    xge3d_particles *particles=NULL;
    xge3d_billboard_batch *batch=NULL;
    xge3d_texture *texture=NULL;
    assert(xge3dSceneCreate(&scene)==XGE_OK);
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreate(128,64,&target)==XGE_OK);
    assert(xge3dTextureCreateRGBAOwned(1,1,texture_pixels,4,1,NULL,&texture)==XGE_OK);
    xge3d_particle_desc_t settings=xge3dParticlesDefault();
    settings.capacity=64;
    settings.life_min=settings.life_max=.5f;
    settings.position.z=-2;
    settings.size_start=settings.size_end=.6f;
    settings.color_start[0]=settings.color_start[1]=settings.color_start[2]=1;
    assert(xge3dParticlesCreate(&settings,&particles)==XGE_OK);
    assert(xge3dParticlesEmit(particles,3)==XGE_OK);
    xge3d_billboard_t legacy[3];
    size_t count;
    assert(xge3dParticlesBillboards(particles,texture,XGE3D_BILLBOARD_ALPHA,legacy,3,&count)==XGE_OK && count==3);
    assert(xge3dParticlesCollectBillboards(particles,XGE3D_BILLBOARD_ALPHA,&batch)==XGE_OK);
    xge3d_camera_t camera;
    assert(xge3dCameraOrthographic(&camera,4,2,.1f,20)==XGE_OK);
    xge3d_render_desc_t render={.camera=&camera,.target=target,.clear_color={0,0,0,1},.clear_flags=3};
    xge3d_render_stats_t stats;
    unsigned char before[128*64*4],after[sizeof(before)];
    assert(xge3dBillboardsSet(renderer,legacy,count)==XGE_OK);
    assert(xge3dRender(renderer,scene,&render,&stats)==XGE_OK && stats.triangles==6);
    assert(xge3dTargetReadPixels(target,before,sizeof(before),512)==XGE_OK);
    assert(before[(32*128+64)*4+1]>100);
    assert(xge3dBillboardsSetBatch(renderer,batch,texture)==XGE_OK);
    assert(xge3dBillboardsSetParticles(renderer,particles,XGE3D_BILLBOARD_ALPHA,texture)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    for(int iteration=0;iteration<64;++iteration){
        assert(xge3dBillboardsSet(renderer,legacy,count)==XGE_OK);
        assert(xge3dBillboardsSetBatch(renderer,batch,texture)==XGE_OK);
        assert(xge3dBillboardsSetParticles(renderer,particles,XGE3D_BILLBOARD_ALPHA,texture)==XGE_OK);
    }
    assert(xrtMemDebugFailAfter(-1) && !xrtGetError());
    assert(xge3dBillboardsSetParticles(renderer,particles,-1,texture)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dParticlesEmit(particles,61)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dBillboardsSetParticles(renderer,particles,XGE3D_BILLBOARD_ALPHA,texture)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xge3d_resource_info_t info={.size=sizeof(info)};
    assert(xge3dTextureResourceInfo(texture,&info)==XGE_OK && info.references==4);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK && xgeGraphicsOwnedResourceCount()==2);
    xge3dBillboardBatchFree(batch);

    xge3d_billboard_geometry_t items[64];
    for(size_t i=0;i<64;++i)items[i]=xge3dBillboardGeometryDefault();
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dBillboardsSetGeometry(renderer,items,64,NULL)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    items[0].width=-1;
    assert(xge3dBillboardsSetGeometry(renderer,items,1,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dRender(renderer,scene,&render,&stats)==XGE_OK && stats.triangles==6);
    assert(xge3dTargetReadPixels(target,after,sizeof(after),512)==XGE_OK);
    assert(!memcmp(before,after,sizeof(before)));
    assert(xge3dParticlesUpdate(particles,1,0)==XGE_OK);
    assert(xge3dBillboardsSetParticles(renderer,particles,XGE3D_BILLBOARD_ALPHA,NULL)==XGE_OK);
    xge3dParticlesFree(particles);
    assert(xgeGraphicsOwnedResourceCount()==1);
    assert(xge3dRender(renderer,scene,&render,&stats)==XGE_OK && stats.triangles==0);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK && xgeGraphicsOwnedResourceCount()==0);
    assert(xge3dRendererCreateOwned(&after_frame_renderer)==XGE_OK);
    assert(xge3dTextureCreateRGBAOwned(1,1,texture_pixels,4,1,NULL,&after_frame_texture)==XGE_OK);
    items[0].width=1;
    items[0].position.z=-2;
    assert(xge3dBillboardsSetGeometry(after_frame_renderer,items,1,after_frame_texture)==XGE_OK);
    assert(xge3dRender(after_frame_renderer,scene,&render,&stats)==XGE_OK && stats.triangles==2);
    assert(xge3dTextureResourceInfo(after_frame_texture,&info)==XGE_OK && info.gpu_payload_bytes>0);
    xge3dTargetFree(target);
    xge3dSceneFree(scene);
    puts("PARTICLE_BILLBOARD_NATIVE_GPU_OK");
    return 1;
}

int main(void)
{
    cpu_checks();
    xge_desc_t desc={.iWidth=128,.iHeight=64,.sTitle="XGE billboard verification"};
    assert(xgeInit(&desc)==XGE_OK);
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==2 && xgeUnitChecked()==XGE_ERROR_BUSY);
    xge3d_resource_info_t info={.size=sizeof(info)};
    assert(xge3dTextureResourceInfo(after_frame_texture,&info)==XGE_OK && info.gpu_payload_bytes==0);
    assert(xge3dRendererReleaseChecked(after_frame_renderer)==XGE_OK);
    assert(xge3dTextureReleaseChecked(after_frame_texture)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0);
    assert(xgeUnitChecked()==XGE_OK);
    return 0;
}
