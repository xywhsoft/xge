#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *path="model-triangle.gltf";
static const char *uri="loader-fixture://model.gltf";
static xge3d_loader *loader;
static xge3d_request_t request;

typedef struct provider_fixture {
    const xge_resource_t *source;
    uint64_t worker;
    unsigned calls,releases;
} provider_fixture;

static int provide(const char *name,void **data,int *size,void *raw)
{
    provider_fixture *fixture=raw;
    assert(!strcmp(name,uri));
    fixture->worker=xrtThreadCurrentId();
    *size=fixture->source->iSize;
    *data=malloc((size_t)*size);
    if(!*data)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(*data,fixture->source->pData,(size_t)*size);
    ++fixture->calls;
    return XGE_OK;
}

static void drop(void *data,void *raw)
{
    provider_fixture *fixture=raw;
    assert(fixture->worker==xrtThreadCurrentId());
    ++fixture->releases;
    free(data);
}

static int32 foreign_thread(void *raw)
{
    xge3d_loader *value=raw;
    bool consumed=true;
    xge3d_request_info_t info;
    assert(xge3dLoaderStatus(value,request,&info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dLoaderReleaseOwnerChecked(value,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

static int frame(void *raw)
{
    (void)raw;
    assert(xgeBegin()==XGE_OK);
    xge3d_request_info_t info;
    xge3d_upload_budget_t upload={.bytes=40,.operations=1};
    uint64_t bytes=0;
    for(size_t step=0;step<100;++step){
        xge3d_upload_stats_t stats;
        assert(xge3dLoaderPump(loader,&upload,&stats)==XGE_OK);
        assert(stats.operations<=1 && stats.upload_bytes<=40);
        bytes+=stats.upload_bytes;
        assert(xge3dLoaderStatus(loader,request,&info)==XGE_OK);
        if(info.state==XGE3D_REQUEST_READY)break;
    }
    assert(info.state==XGE3D_REQUEST_READY && bytes==info.total_bytes);
    xge3d_model *model=NULL,*share=NULL;
    assert(xge3dLoaderTake(loader,request,&model)==XGE_OK && model);
    assert(xge3dLoaderStatus(loader,request,&info)==XGE_ERROR_INVALID_ARGUMENT);
    xrtClearError();
    bool consumed=false;
    assert(xge3dLoaderReleaseOwnerChecked(loader,&consumed)==XGE_OK && consumed);
    loader=NULL;
    assert(xge3dModelShareOwned(model,&share)==XGE_OK && share==model);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    xge3d_model_info_t metadata;
    assert(xge3dModelInfoChecked(share,&metadata)==XGE_OK && metadata.primitive_count==1);
    assert(xge3dModelReleaseChecked(share)==XGE_OK);
    assert(xgeEnd()==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="Managed XGE async loader"};
    assert(xgeInit(&window)==XGE_OK);
    xge3d_loader_desc_t invalid={.max_requests=1025};
    assert(xge3dLoaderCreateOwned(&invalid,&loader)==XGE_ERROR_INVALID_ARGUMENT && !loader);
    assert(!xgeGraphicsOwnedResourceCount());
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    int result=xge3dLoaderCreateOwned(NULL,&loader);
    assert(xrtMemDebugFailAfter(-1));
    assert(result==XGE_ERROR_OUT_OF_MEMORY && !loader && !xgeGraphicsOwnedResourceCount());
    xrtClearError();

    xge_resource_t source={0};
    assert(xgeResourceLoad(path,&source)==XGE_OK && source.iSize>0);
    provider_fixture fixture={.source=&source};
    xge_resource_provider_t provider={.sScheme="loader-fixture",.load=provide,.free=drop,.pUser=&fixture};
    assert(xgeResourceProviderAdd(&provider)==XGE_OK);
    xge3d_loader_desc_t desc={.flags=XGE3D_LOADER_RESOURCE_PROVIDERS};
    assert(xge3dLoaderCreateOwned(&desc,&loader)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);
    xrtClearError();
    assert(xge3dLoaderRequest(loader,uri,&request)==XGE_OK);

    xge3d_request_info_t info={0};
    double end=xrtTimer()+5;
    do {
        assert(xge3dLoaderStatus(loader,request,&info)==XGE_OK);
        if(info.state==XGE3D_REQUEST_CPU_READY || info.state==XGE3D_REQUEST_FAILED)break;
        xrtSleep(1);
    } while(xrtTimer()<end);
    assert(info.state==XGE3D_REQUEST_CPU_READY);
    assert(fixture.calls==1 && fixture.releases==1 && fixture.worker!=xrtThreadCurrentId());
    assert(xgeResourceProviderClearChecked()==XGE_OK);
    xgeResourceFree(&source);
    xthread *thread=xrtThreadCreate(foreign_thread,loader,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);

    result=xgeRun(frame,NULL);
    if(result!=XGE_OK)fprintf(stderr,"xgeRun returned %d, frame completed=%d\n",result,loader==NULL);
    assert(result==XGE_OK);
    assert(!loader && !xgeGraphicsOwnedResourceCount());
    assert(xgeUnitChecked()==XGE_OK);
    puts("MODEL_LOADER3D_NATIVE_GPU_OK");
    return 0;
}
