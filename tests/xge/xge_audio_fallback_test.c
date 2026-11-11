/* Private CPU setup, authoritative production aggregate, real miniaudio.
 * Old-source controls stop at assertions, never dereference a freed path. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static const char* filename="audio-fallback.wav";
static ma_result testFallbackReady(ma_job* job)
{ma_atomic_store_32((ma_uint32*)job->data.custom.data0,1);return MA_SUCCESS;}
static void begin(void)
{
    xge_desc_t desc={0};desc.sTitle="CPU fallback";desc.iRunMode=XGE_RUN_MANUAL;
    assert(xgeInit(&desc)==XGE_OK);
    ma_engine* engine=xrtMalloc(sizeof(*engine));assert(engine);
    ma_engine_config config=ma_engine_config_init();config.noDevice=MA_TRUE;config.channels=2;config.sampleRate=8000;
    config.allocationCallbacks.onMalloc=__xgeMaMalloc;config.allocationCallbacks.onRealloc=__xgeMaRealloc;config.allocationCallbacks.onFree=__xgeMaFree;
    assert(ma_engine_init(&config,engine)==MA_SUCCESS);
    /* The default one-worker manager frees its 48-byte thread proxy before
     * processing jobs. Observe actual readiness before heap baselines, rather
     * than racing that legitimate startup free or weakening equality checks. */
    assert(engine->pResourceManager->config.jobThreadCount==1);
    ma_uint32 ready=0;ma_job job=ma_job_init(MA_JOB_TYPE_CUSTOM);
    job.data.custom.proc=testFallbackReady;job.data.custom.data0=(ma_uintptr)&ready;
    assert(ma_resource_manager_post_job(engine->pResourceManager,&job)==MA_SUCCESS);
    unsigned attempt=0;while(!ma_atomic_load_32(&ready) && attempt++<1000)ma_sleep(1);
    assert(ma_atomic_load_32(&ready));g_xge.pAudioEngine=engine;g_xge.bAudioInitialized=1;
}
static void finish(void)
{assert(!g_xge.iAudioCount);assert(xgeAudioUnitChecked()==XGE_OK);assert(xgeUnitChecked()==XGE_OK);}
static void mix(xge_sound sound)
{
    xgeSoundSetLoop(sound,1);assert(xgeSoundPlay(sound)==XGE_OK);
    for(unsigned attempt=0;attempt<1000;++attempt){
        float samples[512];assert(ma_engine_read_pcm_frames((ma_engine*)g_xge.pAudioEngine,samples,256,NULL)==MA_SUCCESS);
        for(size_t i=0;i<512;++i)if(fabsf(samples[i])>0.01f){assert(xgeSoundStop(sound)==XGE_OK);return;}
        ma_sleep(1);
    }
    assert(!"fallback PCM did not arrive");
}
static void testFallbackHeapEqual(const xmemdebugsnapshot* a,const xmemdebugsnapshot* b)
{
    assert(a->LiveCount==b->LiveCount && a->LiveBytes==b->LiveBytes);
    assert(a->InvalidFreeCount==b->InvalidFreeCount && a->DoubleFreeCount==b->DoubleFreeCount);
}
#if !defined(XGE_TEST_FALLBACK_BASELINE)
typedef struct Writers {xatomic32 start;unsigned rounds;} Writers;
static int32 writer(ptr raw)
{
    Writers* state=(Writers*)raw;
    while(!xrtAtomic32Load(&state->start,XMEMORY_ACQUIRE))xrtSleep(1);
    for(unsigned i=0;i<state->rounds;++i){assert(xgeSoundFallbackSet(filename)==XGE_OK);if(i%5==0)xgeSoundFallbackClear();}
    return 0;
}
static void owned(void)
{
    xge_sound sound=(void*)1;assert(xgeSoundFallbackCreateOwned(&sound)==XGE_OK && sound);
    assert(sound->iRefCount==1 && (sound->iFlags&XGE_SOUND_FALLBACK));mix(sound);
    assert(xgeSoundFreeChecked(sound)==XGE_OK);
}
#endif
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=9);
    assert(xrtMemDebugEnable(true));begin();
    unsigned char wave[76]={0};memcpy(wave,"RIFF",4);wave[4]=68;memcpy(wave+8,"WAVEfmt ",8);
    wave[16]=16;wave[20]=1;wave[22]=2;wave[24]=0x40;wave[25]=0x1f;wave[29]=0x7d;wave[32]=4;wave[34]=16;
    memcpy(wave+36,"data",4);wave[40]=32;int16_t samples[16];for(unsigned i=0;i<16;++i)samples[i]=8192;memcpy(wave+44,samples,sizeof(samples));
    FILE* file=fopen(filename,"wb");assert(file && fwrite(wave,1,sizeof(wave),file)==sizeof(wave) && !fclose(file));
    xge_sound warm=NULL;assert(xgeSoundCreateOwned(&warm,filename,NULL)==XGE_OK);mix(warm);assert(xgeSoundFreeChecked(warm)==XGE_OK);finish();
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    if(mode==0){
        assert(xgeSoundFallbackSet(filename)==XGE_OK);begin();xge_sound_t sound={0};
        assert(xgeSoundFallbackGet(&sound)==XGE_OK);assert(sound.iFlags&XGE_SOUND_FALLBACK);mix(&sound);xgeSoundFree(&sound);finish();
    }else if(mode==1){
        assert(xgeSoundFallbackSet(filename)==XGE_OK);assert(xgeUnitChecked()==XGE_OK);
    }else if(mode==2){
        begin();assert(xgeSoundFallbackSet(filename)==XGE_OK);xge_sound_t sound={0};assert(xrtMemDebugFailAfter(0));
        int status=__xgeAudioLoadFallback(&sound,XGE_AUDIO_SOUND,0,NULL,XGE_ERROR_AUDIO_FAILED);
        xrtMemDebugFailClear();assert(status==XGE_ERROR_OUT_OF_MEMORY && !sound.pBackend);xrtClearError();finish();
    }
#if !defined(XGE_TEST_FALLBACK_BASELINE)
    else if(mode==3){
        assert(xgeSoundFallbackSet(filename)==XGE_OK);
        __xge_audio_fallback_path* snapshot=NULL;assert(__xgeSoundFallbackAcquire(&snapshot)==XGE_OK && snapshot);
        xgeSoundFallbackClear();assert(xgeSoundFallbackSet("replacement.wav")==XGE_OK);
        assert(!strcmp(snapshot->text,filename) && snapshot->refs==1);
        __xgeSoundFallbackDrop(snapshot);xgeSoundFallbackClear();
    }else if(mode==4){
        begin();Writers state={XRT_ATOMIC32_INIT(0),512};xthread* a=xrtThreadCreate(writer,&state,0);xthread* b=xrtThreadCreate(writer,&state,0);assert(a && b);
        xrtAtomic32Store(&state.start,1,XMEMORY_RELEASE);
        for(unsigned i=0;i<128;++i){
            xge_sound sound=NULL;int status=xgeSoundFallbackCreateOwned(&sound);
            if(status==XGE_OK){assert(sound && (sound->iFlags&XGE_SOUND_FALLBACK));mix(sound);assert(xgeSoundFreeChecked(sound)==XGE_OK);}
            else assert(status==XGE_ERROR_RESOURCE_FAILED && !sound);
        }
        assert(xrtThreadWait(a)==XWAIT_OK && xrtThreadWait(b)==XWAIT_OK);xrtThreadDestroy(a);xrtThreadDestroy(b);
        assert(xgeSoundFallbackSet(filename)==XGE_OK);owned();finish();
    }else if(mode==5){
        begin();assert(xgeSoundFallbackSet(filename)==XGE_OK);
        assert(__xgeGraphicsCloseBegin(&g_xgeGraphicsLifetime)==XGE_OK);
        assert(xrtMemDebugFailAfter(0));assert(xgeSoundFallbackSet("blocked.wav")==XGE_ERROR_BUSY);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xgeSoundFallbackClear();
        __xgeGraphicsCloseAbort(&g_xgeGraphicsLifetime);
        assert(__xgeGraphicsContextBegin(&g_xgeGraphicsLifetime)==XGE_OK);
        assert(xgeSoundFallbackSet("blocked.wav")==XGE_ERROR_BUSY);xgeSoundFallbackClear();
        __xgeGraphicsContextCommit(&g_xgeGraphicsLifetime);__xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);
        assert(xgeSoundFallbackSet(filename)==XGE_OK);owned();finish();
    }else if(mode==6){
        begin();assert(xgeSoundFallbackSet(NULL)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xgeSoundFallbackCreateOwned(NULL)==XGE_ERROR_INVALID_ARGUMENT);
        xge_sound absent=(void*)1;assert(xgeSoundFallbackCreateOwned(&absent)==XGE_ERROR_RESOURCE_FAILED && !absent);
        assert(xgeSoundFallbackSet(filename)==XGE_OK);assert(xrtMemDebugFailAfter(0));
        assert(xgeSoundFallbackSet("unpublished.wav")==XGE_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();owned();
        xge_sound_t live={0};assert(xgeSoundFallbackGet(&live)==XGE_OK);void* backend=live.pBackend;
        assert(xgeSoundFallbackGet(&live)==XGE_ERROR_INVALID_STATE && live.pBackend==backend);xgeSoundFree(&live);
        (void)xrtSpinLock(&g_xgeGraphicsLifetime.lock);g_xgeAudioFallbackPath->refs=SIZE_MAX;(void)xrtSpinUnlock(&g_xgeGraphicsLifetime.lock);
        absent=(void*)1;assert(xgeSoundFallbackCreateOwned(&absent)==XGE_ERROR_OUT_OF_MEMORY && !absent);
        (void)xrtSpinLock(&g_xgeGraphicsLifetime.lock);g_xgeAudioFallbackPath->refs=1;(void)xrtSpinUnlock(&g_xgeGraphicsLifetime.lock);finish();
    }else if(mode==7){
        begin();assert(xgeSoundFallbackSet(filename)==XGE_OK);unsigned failed=0,passed=0;
        for(unsigned budget=0;budget<64;++budget){
            xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);assert(xrtMemDebugFailAfter(budget));xge_sound sound=(void*)1;
            int status=xgeSoundFallbackCreateOwned(&sound);xrtMemDebugFailClear();
            if(status==XGE_OK){++passed;assert(sound && !xrtGetError());mix(sound);assert(xgeSoundFreeChecked(sound)==XGE_OK);}
            else{++failed;assert(!sound);xrtClearError();}
            xrtMemDebugSnapshot(&b);
            if(a.LiveCount!=b.LiveCount || a.LiveBytes!=b.LiveBytes)fprintf(stderr,"Fallback imbalance budget=%u status=%d: %zu/%zu -> %zu/%zu refs=%zu\n",budget,status,a.LiveCount,a.LiveBytes,b.LiveCount,b.LiveBytes,g_xgeAudioFallbackPath->refs);
            testFallbackHeapEqual(&a,&b);assert(!g_xge.iAudioCount);
        }
        assert(failed && passed);printf("Fallback factory budgets: %u failures, %u successes\n",failed,passed);finish();
    }else if(mode==8){
        begin();assert(xgeSoundFallbackSet(filename)==XGE_OK);xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        xge_sound sound=NULL;assert(xgeSoundCreateOwned(&sound,"missing-source.wav",&group)==XGE_OK && (sound->iFlags&XGE_SOUND_FALLBACK));
        xgeAudioGroupFree(&group);xgeSoundFallbackClear();mix(sound);assert(xgeSoundFreeChecked(sound)==XGE_OK);
        assert(xgeSoundFallbackSet(filename)==XGE_OK);xge_music music=(void*)1;
        assert(xgeMusicCreateOwned(&music,"missing-source.wav",NULL)!=XGE_OK && !music);xrtClearError();finish();
    }else if(mode==9){
        begin();xerror* ambient=xrtErrorCreate(XERR_STATE,"fallback-test",4,"caller diagnostic");assert(ambient);xrtSetErrorTake(ambient);
        assert(xgeSoundFallbackSet(filename)==XGE_OK && xrtGetError()==ambient);owned();assert(xrtGetError()==ambient);xrtClearError();
        __xge_audio_fallback_path* snapshot=NULL;assert(__xgeSoundFallbackAcquire(&snapshot)==XGE_OK);
        finish();assert(!strcmp(snapshot->text,filename) && snapshot->refs==1);__xgeSoundFallbackDrop(snapshot);
    }
#endif
    xrtMemDebugSnapshot(&after);testFallbackHeapEqual(&before,&after);assert(!remove(filename));
    printf("Audio fallback mode %d passed: real CPU mixer, no device/window/network\n",mode);return 0;
}
