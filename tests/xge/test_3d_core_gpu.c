#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"core GPU %d: %s\n",__LINE__,#x);goto done;}}while(0)
static int failed=1;
static int frame(void *unused)
{
    (void)unused;xge3d_cache *cache=NULL;xge3d_scene *scene=NULL;xge3d_renderer *renderer=NULL;xge3d_target *target=NULL;
    xge3d_model *model=NULL,*again=NULL,*different=NULL;xge3d_animator *anim=NULL;xge3d_clip *clip=NULL;
    CHECK(!xge3dCacheCreate(NULL,&cache));xge3d_cache_request_t requests[12];
    for(int i=0;i<12;++i)CHECK(!xge3dCacheRequest(cache,"artifacts/xge-3d/fixtures/embedded.gltf",requests+i));
    CHECK(!xge3dCacheCancel(cache,requests[0]));uint64_t bytes=0;int ready=0;double deadline=xrtTimer()+5;
    while(!ready&&xrtTimer()<deadline){xge3d_upload_budget_t budget={32,0,3};xge3d_upload_stats_t stats;
        CHECK(!xge3dCachePump(cache,&budget,&stats)&&stats.upload_bytes<=32&&stats.operations<=3);bytes+=stats.upload_bytes;
        xge3d_request_info_t info;CHECK(!xge3dCacheStatus(cache,requests[1],&info));ready=info.state==XGE3D_REQUEST_READY;CHECK(info.state!=XGE3D_REQUEST_FAILED);xrtSleep(1);}
    CHECK(ready&&!xge3dCacheTake(cache,requests[1],&model));
    for(int i=2;i<12;++i){CHECK(!xge3dCacheTake(cache,requests[i],&again)&&again==model);xge3dModelFree(again);again=NULL;}
    CHECK(!xge3dCacheRelease(cache,requests[0]));xge3d_resource_info_t resource={.size=sizeof(resource)};
    CHECK(!xge3dModelResourceInfo(model,&resource)&&bytes==resource.cpu_payload_bytes);
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/external.gltf",&different));xge3d_material_desc_t m1,m2;
    CHECK(!xge3dMaterialGetDesc(xge3dModelMaterial(model,0),&m1)&&!xge3dMaterialGetDesc(xge3dModelMaterial(different,0),&m2)&&m1.maps[0].texture==m2.maps[0].texture);
    CHECK(!xge3dSceneCreate(&scene)&&!xge3dRendererCreate(&renderer)&&!xge3dTargetCreate(192,128,&target));
    xge3d_node_t roots[3];for(int i=0;i<3;++i){CHECK(!xge3dModelInstantiate(scene,i==2?different:model,(xge3d_node_t){0},roots+i));
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=(i-1)*2.5f;CHECK(!xge3dNodeSetTransform(scene,roots[i],&t));}
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,10,6,.1f,30)&&!xge3dCameraLookAt(&camera,(xge3d_vec3_t){2,1,8},(xge3d_vec3_t){2,1,0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t draw={&camera,target,{.1f,.1f,.1f,1},3};CHECK(!xge3dRender(renderer,scene,&draw,NULL));unsigned char pixels[192*128*4];
    CHECK(!xge3dTargetReadPixels(target,pixels,sizeof(pixels),192*4));size_t colored=0;for(size_t i=0;i<sizeof(pixels);i+=4)colored+=pixels[i]>80||pixels[i+1]>80||pixels[i+2]>80;CHECK(colored>800);
    CHECK(!xgeImageSavePNG("artifacts/xge-core-next/shared-cache.png",192,128,pixels,192*4));
    /* Cache can be destroyed while scene-owned models and textures keep rendering. */
    xge3dModelFree(model);model=NULL;xge3dModelFree(different);different=NULL;CHECK(!xge3dCacheFree(cache));cache=NULL;
    unsigned char after[sizeof(pixels)];CHECK(!xge3dRender(renderer,scene,&draw,NULL)&&!xge3dTargetReadPixels(target,after,sizeof(after),192*4)&&!memcmp(pixels,after,sizeof(pixels)));
    /* A clone keeps independent skin/pose bindings even after deleting the source. */
    CHECK(!xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model));xge3d_node_t actor,copy;
    CHECK(!xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&actor)&&!xge3dNodeClone(scene,actor,(xge3d_node_t){0},0,&copy));xge3dModelFree(model);model=NULL;
    CHECK(!xge3dAnimatorCreate(scene,copy,&anim)&&!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip));xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.5f;
    CHECK(!xge3dAnimatorSetLayer(anim,0,clip,&layer)&&!xge3dAnimatorUpdate(anim,0)&&!xge3dNodeDestroy(scene,actor)&&!xge3dRender(renderer,scene,&draw,NULL));
    CHECK(!xge3dCacheCreate(NULL,&cache));
    /* Failed owner-thread publication rolls back the fully uploaded private
     * model, reports failure rather than completion, and leaves the key retryable. */
    xge3d_cache_request_t rejected;CHECK(!xge3dCacheRequest(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&rejected));
    xge3d_request_info_t rejected_info;deadline=xrtTimer()+5;
    do{CHECK(!xge3dCacheStatus(cache,rejected,&rejected_info));if(rejected_info.state==XGE3D_REQUEST_CPU_READY)break;xrtSleep(1);}while(xrtTimer()<deadline);
    CHECK(rejected_info.state==XGE3D_REQUEST_CPU_READY);xallocationbudget *quota=NULL;xallocationscope scope={0};
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    xge3d_upload_budget_t publish_budget={UINT64_MAX,0,1000};xge3d_upload_stats_t published;int publish_result=xge3dCachePump(cache,&publish_budget,&published);xrtAllocationBudgetLeave(&scope);
    CHECK(!publish_result&&published.failed==1&&!published.completed&&!xge3dCacheStatus(cache,rejected,&rejected_info)&&rejected_info.state==XGE3D_REQUEST_FAILED&&rejected_info.result==XGE_ERROR_OUT_OF_MEMORY);
    xallocationbudgetinfo qi={.Size=sizeof(qi)};CHECK(xrtAllocationBudgetInfo(quota,&qi)&&!qi.LiveBytes);xrtAllocationBudgetFree(quota);CHECK(!xge3dCacheRelease(cache,rejected));
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&again));xge3dModelFree(again);again=NULL;
    /* Repeated partially uploaded cancellation never publishes a stale model. */
    for(int i=0;i<60;++i){xge3d_cache_request_t request;CHECK(!xge3dCacheRequest(cache,"artifacts/xge-3d/fixtures/async.gltf",&request));
        if(i%3){xge3d_request_info_t info;double end=xrtTimer()+5;do{CHECK(!xge3dCacheStatus(cache,request,&info));if(info.state==XGE3D_REQUEST_CPU_READY)break;xrtSleep(1);}while(xrtTimer()<end);
            CHECK(info.state==XGE3D_REQUEST_CPU_READY);xge3d_upload_budget_t budget={i%3==1?64:8480,0,i%3==1?2:5};CHECK(!xge3dCachePump(cache,&budget,NULL));}
        CHECK(!xge3dCacheRelease(cache,request));xge3d_upload_budget_t cleanup={UINT64_MAX,0,1000};CHECK(!xge3dCachePump(cache,&cleanup,NULL));}
    CHECK(!xge3dCacheFree(cache));cache=NULL;failed=0;
    printf("Core GPU: 12 coalesced clients/%llu actual uploaded bytes, shared cross-model textures, retained scene after cache free, cloned skin render and 60 partial cancellations passed\n",(unsigned long long)bytes);
done:
    xge3dCacheFree(cache);xge3dAnimatorFree(anim);xge3dClipFree(clip);xge3dModelFree(model);xge3dModelFree(again);xge3dModelFree(different);
    xge3dSceneFree(scene);xge3dTargetFree(target);xge3dRendererFree(renderer);return 1;
}
int main(void){xge_desc_t d={0};d.iWidth=320;d.iHeight=240;d.sTitle="XGE core query/cache/node verification";if(xgeInit(&d))return 1;int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
