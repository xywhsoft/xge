/* Reuse the public-API application fixture, not engine private structures. */
#define main integration_example_main
#include "../examples/xge_3d_integration/main.c"
#undef main
#include <windows.h>
#include <psapi.h>
#define REQUIRE(x) do {if (!(x)) {fprintf(stderr,"pressure:%d: %s\n",__LINE__,#x);goto failed;}} while(0)
typedef void (APIENTRY *query_gen)(int,unsigned*);
typedef void (APIENTRY *query_begin)(unsigned,unsigned);
typedef void (APIENTRY *query_end)(unsigned);
typedef void (APIENTRY *query_get)(unsigned,unsigned,unsigned*);
typedef void (APIENTRY *query_get64)(unsigned,unsigned,uint64_t*);
typedef void (APIENTRY *finish_proc)(void);
typedef const unsigned char *(APIENTRY *string_proc)(unsigned);
typedef struct sample {int mode;uint64_t cpu,gpu,interval;uint32_t draws;uint64_t upload;} sample;
typedef struct pending_query {unsigned id;size_t sample;int pending;} pending_query;
typedef struct bench {
    demo app;xge3d_loader *loader;xge3d_request_t job;
    xge3d_node_t extra[2];xge3d_animator *anim[2];
    int phase,cycles,limit,hold,failed,serial,full,queued,texture,vertex;
    uint64_t last,requested,total_load,max_load;const char *output;FILE *file;xarray samples;
    query_gen gen,del;query_begin begin;query_end end;query_get get;query_get64 get64;finish_proc finish;
    pending_query queries[4];
} bench;
static void *gl_function(const char *name)
{
    HMODULE lib=GetModuleHandleA("opengl32.dll");if (!lib) return NULL;
    FARPROC proc=GetProcAddress(lib,name);
    if (!proc) {typedef PROC (WINAPI *get_proc)(LPCSTR);get_proc get=(get_proc)(void*)GetProcAddress(lib,"wglGetProcAddress");if (get) proc=get(name);}
    return (void*)proc;
}
static int collect_queries(bench *b,int force)
{
    if (force) b->finish();
    for (int i=0;i<4;++i) if (b->queries[i].pending) {
        unsigned ready=0;b->get(b->queries[i].id,0x8867,&ready);
        if (ready || force) {sample *s=xrtArrayGet(&b->samples,b->queries[i].sample);b->get64(b->queries[i].id,0x8866,&s->gpu);b->queries[i].pending=0;}
    }
    return XGE_OK;
}
static int memory_sample(bench *b)
{
    b->finish();PROCESS_MEMORY_COUNTERS_EX memory={0};memory.cb=sizeof(memory);
    if (!GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)&memory,sizeof(memory))) return 0;
    /* Both the application and the tested DLL import msvcrt. Its busy heap
     * separates retained CPU allocations from process/driver reservations. */
    typedef intptr_t (*heap_proc)(void);
    heap_proc get_heap=(heap_proc)(void*)GetProcAddress(GetModuleHandleA("msvcrt.dll"),"_get_heap_handle");
    if (!get_heap) return 0;
    HANDLE heap=(HANDLE)get_heap();if (!HeapLock(heap)) return 0;
    PROCESS_HEAP_ENTRY entry={0};uint64_t busy=0,blocks=0;
    while (HeapWalk(heap,&entry)) if (entry.wFlags&PROCESS_HEAP_ENTRY_BUSY) {busy+=entry.cbData;++blocks;}
    DWORD error=GetLastError();HeapUnlock(heap);if (error!=ERROR_NO_MORE_ITEMS) return 0;
    fprintf(b->file,"memory,%d,%llu,%llu,%llu,%llu,%llu\n",b->cycles,(unsigned long long)memory.PrivateUsage,(unsigned long long)memory.WorkingSetSize,(unsigned long long)memory.PeakWorkingSetSize,(unsigned long long)busy,(unsigned long long)blocks);
    fflush(b->file);printf("memory cycle %d: %llu private bytes, %llu CRT busy bytes/%llu blocks\n",b->cycles,(unsigned long long)memory.PrivateUsage,(unsigned long long)busy,(unsigned long long)blocks);fflush(stdout);
    return !ferror(b->file);
}
static int destroy_extra(bench *b)
{
    for (int i=0;i<2;++i) {
        xge3dAnimatorFree(b->anim[i]);b->anim[i]=NULL;
        if (b->extra[i].slot) {int result=xge3dNodeDestroy(b->app.scene,b->extra[i]);if (result!=XGE_OK) return result;b->extra[i]=(xge3d_node_t){0};}
    }
    return XGE_OK;
}
static int cycle(bench *b)
{
    int kind=b->cycles%4;int result=XGE_OK;
    if (!b->job.slot && !b->hold) {
        const char *path=kind==2 ? (b->cycles%8==2 ? "artifacts/xge-3d/fixtures/rig-a.gltf" : "artifacts/xge-3d/fixtures/rig-b.gltf") : "artifacts/xge-3d/fixtures/async.gltf";
        b->requested=xrtClock();result=xge3dLoaderRequest(b->loader,path,&b->job);if (result!=XGE_OK) return result;
        if (!kind) {
            xge3d_request_t stale=b->job;result=xge3dLoaderRelease(b->loader,b->job);b->job=(xge3d_request_t){0};
            xge3d_request_info_t info;if (result!=XGE_OK || xge3dLoaderStatus(b->loader,stale,&info)!=XGE_ERROR_INVALID_ARGUMENT) return XGE_ERROR_INVALID_STATE;
            ++b->queued;++b->cycles;return XGE_OK;
        }
    }
    if (b->hold) {
        for (int i=0;i<2;++i) if ((result=xge3dAnimatorUpdate(b->anim[i],1.f/60))!=XGE_OK) return result;
        if (++b->hold==4) {result=destroy_extra(b);b->hold=0;++b->full;++b->cycles;}
        return result;
    }
    xge3d_request_info_t info;result=xge3dLoaderStatus(b->loader,b->job,&info);if (result!=XGE_OK) return result;
    if (info.state==XGE3D_REQUEST_FAILED) return info.result;
    if (info.state==XGE3D_REQUEST_QUEUED || info.state==XGE3D_REQUEST_LOADING) return XGE_OK;
    xge3d_upload_budget_t budget={kind==1 ? 64 : kind==3 ? 8480 : 256*1024,0,kind==1 ? 2 : kind==3 ? 5 : 100};
    xge3d_upload_stats_t uploads;result=xge3dLoaderPump(b->loader,&budget,&uploads);if (result!=XGE_OK) return result;
    if (uploads.upload_bytes>budget.bytes || uploads.operations>budget.operations || uploads.failed) return XGE_ERROR_INVALID_STATE;
    result=xge3dLoaderStatus(b->loader,b->job,&info);if (result!=XGE_OK) return result;
    if (kind==1 || kind==3) {
        if (info.state!=XGE3D_REQUEST_UPLOADING || info.uploaded_bytes!=budget.bytes || info.uploaded_bytes>=info.total_bytes) return XGE_ERROR_INVALID_STATE;
        result=xge3dLoaderRelease(b->loader,b->job);b->job=(xge3d_request_t){0};
        if (kind==1) ++b->texture;else ++b->vertex;++b->cycles;return result;
    }
    if (info.state!=XGE3D_REQUEST_READY) return XGE_OK;
    uint64_t load=xrtClock()-b->requested;b->total_load+=load;if (load>b->max_load) b->max_load=load;
    xge3d_model *model=NULL;result=xge3dLoaderTake(b->loader,b->job,&model);b->job=(xge3d_request_t){0};
    int rig=b->cycles%8==2 ? 0 : 1;
    for (int i=0;i<2 && result==XGE_OK;++i) {
        result=xge3dModelInstantiate(b->app.scene,model,(xge3d_node_t){0},&b->extra[i]);
        if (result==XGE_OK) result=xge3dNodeSetGlobalPosition(b->app.scene,b->extra[i],(xge3d_dvec3_t){1000000000+(i ? 3.0 : -3.0),0,-1000000002});
        if (result==XGE_OK) result=xge3dAnimatorCreate(b->app.scene,b->extra[i],&b->anim[i]);
        xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.2f;
        if (result==XGE_OK) result=xge3dAnimatorSetLayer(b->anim[i],0,b->app.clips[rig][(b->cycles/4)%6],&layer);
    }
    xge3dModelFree(model);if (result==XGE_OK) b->hold=1;return result;
}
static int measure(bench *b)
{
    int mode=b->serial%4;demo *d=&b->app;xge3d_camera_t camera;
    int result=xge3dCameraPerspective(&camera,.9f,640.f/480,.1f,100);
    if (result==XGE_OK) result=xge3dCameraLookAtGlobal(&camera,d->origin,(xge3d_dvec3_t){1000000010,8,-999999987},(xge3d_dvec3_t){1000000000,1,-1000000000},(xge3d_vec3_t){0,1,0});
    if (result!=XGE_OK) return result;
    xge3d_shadow_settings_t shadows=xge3dShadowDefault();shadows.cascades=2;shadows.resolution=1024;shadows.distance=40;shadows.depth_padding=12;shadows.bias=.005f;shadows.normal_bias=.08f;
    xge3d_render_desc_t desc={&camera,d->target,{0,0,0,1},3,d->sky};desc.shadows=&shadows;desc.exposure=1;
    desc.disable_culling=(mode&1)!=0;desc.disable_instancing=(mode&2)!=0;
    sample s={0};s.mode=mode;uint64_t now=xrtClock();s.interval=now-b->last;b->last=now;
    if (!xrtArrayPush(&b->samples,&s)) return XGE_ERROR_OUT_OF_MEMORY;
    collect_queries(b,0);pending_query *q=NULL;
    for (int i=0;i<4;++i) if (!b->queries[i].pending) {q=&b->queries[i];break;}
    if (q) b->begin(0x88BF,q->id);
    xge3d_render_stats_t stats;now=xrtClock();result=xge3dRender(d->renderer,d->scene,&desc,&stats);
    s.cpu=xrtClock()-now;s.draws=stats.draw_calls;s.upload=stats.upload_bytes;
    if (q) {b->end(0x88BF);q->pending=1;q->sample=b->samples.Count-1;}
    *(sample*)xrtArrayGet(&b->samples,b->samples.Count-1)=s;++b->serial;
    return result;
}
static int pressure_frame(void *user)
{
    bench *b=user;
    if (!b->phase) {
        REQUIRE(frame(&b->app)==0);
        if (b->app.ready==15 && b->app.frame>=32) {
            b->gen=(query_gen)gl_function("glGenQueries");b->del=(query_gen)gl_function("glDeleteQueries");
            b->begin=(query_begin)gl_function("glBeginQuery");b->end=(query_end)gl_function("glEndQuery");
            b->get=(query_get)gl_function("glGetQueryObjectuiv");b->get64=(query_get64)gl_function("glGetQueryObjectui64v");
            b->finish=(finish_proc)gl_function("glFinish");string_proc string=(string_proc)gl_function("glGetString");
            REQUIRE(b->gen && b->del && b->begin && b->end && b->get && b->get64 && b->finish && string);
            printf("GPU: %s | %s | %s\n",string(0x1F00),string(0x1F01),string(0x1F02));
            fflush(stdout);
            for (int i=0;i<4;++i) b->gen(1,&b->queries[i].id);
            REQUIRE(xge3dLoaderCreate(NULL,&b->loader)==XGE_OK);b->last=xrtClock();b->phase=1;
        }
        return 0;
    }
    if (b->phase==1) {
        REQUIRE(measure(b)==XGE_OK);
        if (b->serial==320) {collect_queries(b,1);b->phase=2;b->last=xrtClock();}
        return 0;
    }
    uint64_t now=xrtClock();sample s={.mode=4,.interval=now-b->last};b->last=now;
    REQUIRE(cycle(b)==XGE_OK && frame(&b->app)==0);s.cpu=xrtClock()-now;
    REQUIRE(xrtArrayPush(&b->samples,&s));
    if (b->cycles && b->cycles%100==0 && !b->hold && !b->job.slot) REQUIRE(memory_sample(b));
    if (b->cycles<b->limit) return 0;
    REQUIRE(destroy_extra(b)==XGE_OK && xge3dLoaderFree(b->loader)==XGE_OK);b->loader=NULL;
    REQUIRE(b->app.actions==63 && b->app.moves==1 && b->app.deletes==1 && b->app.rebases==1);
    for (size_t i=0;i<b->samples.Count;++i) {
        sample *p=xrtArrayGet(&b->samples,i);fprintf(b->file,"frame,%d,%llu,%llu,%llu,%u,%llu\n",p->mode,(unsigned long long)p->cpu,(unsigned long long)p->gpu,(unsigned long long)p->interval,p->draws,(unsigned long long)p->upload);
    }
    fprintf(b->file,"loads,%d,%llu,%llu\n",b->full,(unsigned long long)b->total_load,(unsigned long long)b->max_load);
    REQUIRE(!ferror(b->file));
    printf("pressure: %d cycles (%d immediate, %d partial texture, %d partial VBO, %d complete/two animated instances), %zu frames, all actions/rebase/move/delete passed\n",b->cycles,b->queued,b->texture,b->vertex,b->full,b->samples.Count);
    for (int i=0;i<4;++i) b->del(1,&b->queries[i].id);
    release(&b->app);return 1;
failed:
    b->failed=1;destroy_extra(b);xge3dLoaderFree(b->loader);b->loader=NULL;release(&b->app);return 1;
}
int main(int argc,char **argv)
{
    if (argc!=3) return 2;
    bench b={.limit=atoi(argv[1]),.output=argv[2]};if (b.limit<400 || b.limit%100) return 2;
    b.app.last_action=-1;b.app.frames=INT_MAX;xrtArrayInit(&b.app.triangles,sizeof(triangle));xrtArrayInit(&b.samples,sizeof(sample));
    /* Measurement storage must be fixed before the memory baseline; otherwise
     * its own growing frame log looks like an engine leak with constant blocks. */
    if (!xrtArrayReserve(&b.samples,(size_t)b.limit*16+1024)) return 1;
    b.file=fopen(b.output,"wb");if (!b.file) return 1;
    int result=build_scene(&b.app);if (result==XGE_OK) result=xge3dLoaderCreate(NULL,&b.app.loader);
    for (int i=0;i<15 && result==XGE_OK;++i) {
        char path[256];if (i<2) snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/rig-%c.gltf",'a'+i);
        else if (i==2) snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/async.gltf");
        else snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/retarget-%c-%s.gltf",'a'+(i-3)/6,actions[(i-3)%6]);
        result=xge3dLoaderRequest(b.app.loader,path,&b.app.requests[i]);
    }
    xge_desc_t desc={0};desc.iWidth=640;desc.iHeight=480;desc.sTitle="XGE 3D pressure/performance verification";
    if (result==XGE_OK) result=xgeInit(&desc);
    b.app.start=xrtClock();if (result==XGE_OK) result=xgeRun(pressure_frame,&b);
    xge3dLoaderFree(b.loader);destroy_extra(&b);release(&b.app);xrtArrayUnit(&b.samples);xgeUnit();
    if (fclose(b.file)) result=XGE_ERROR;
    return result==XGE_OK && !b.failed ? 0 : 1;
}
