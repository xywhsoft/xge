/* Private setup selects miniaudio's real no-device engine, then all ownership
 * operations use production XGE functions. No fake mixer or device is used.
 * Include the authoritative aggregate so setup can reach its private state;
 * the remainder of the qualified native archive supplies its other TUs. */
#define XRT_MODULE_ALL
#include "single/xge.c"
/* Release aggregate configuration disables its internal assertions. Test
 * assertions must remain active, including the operations they verify. */
#undef NDEBUG
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* Inspect only at quiescent API boundaries: no production-only debug counter. */
#if defined(XGE_TEST_GRAPHICS_COUPLING_BASELINE) || defined(XGE_TEST_ALLOCATOR_BASELINE)
#define testAudioGroups g_xgeAudioGroupCount
#define testAudioOwners g_xgeGraphicsLifetime.engineOwners
#else
#define testAudioOwners g_xgeGraphicsLifetime.audioOwners
#define testAudioGroups (testAudioOwners-(uint64_t)g_xge.iAudioCount)
#endif
static unsigned char wave[76];
static void begin(void)
{
    assert(xrtMemDebugEnable(true));
    xge_desc_t desc={0};desc.sTitle="CPU-only audio lifetime";assert(xgeInit(&desc)==XGE_OK);
    ma_engine* engine=xrtMalloc(sizeof(*engine));assert(engine);
    ma_engine_config config=ma_engine_config_init();config.noDevice=MA_TRUE;
    config.channels=2;config.sampleRate=8000;
    config.allocationCallbacks.onMalloc=__xgeMaMalloc;
    config.allocationCallbacks.onRealloc=__xgeMaRealloc;
    config.allocationCallbacks.onFree=__xgeMaFree;
    assert(ma_engine_init(&config,engine)==MA_SUCCESS);
    g_xge.pAudioEngine=engine;g_xge.bAudioInitialized=1;
    memcpy(wave,"RIFF",4);wave[4]=68;memcpy(wave+8,"WAVEfmt ",8);
    wave[16]=16;wave[20]=1;wave[22]=2;wave[24]=0x40;wave[25]=0x1f;
    wave[29]=0x7d;wave[32]=4;wave[34]=16;memcpy(wave+36,"data",4);wave[40]=32;
    int16_t samples[16];for(int i=0;i<16;++i)samples[i]=8192;
    memcpy(wave+44,samples,sizeof(samples));
}
static xge_audio_clip* decode(void)
{
    xge_audio_clip* clip=NULL;xge_audio_clip_decode_t d={wave,sizeof(wave),64,NULL,NULL};
    assert(xgeAudioClipDecodeMemory(&d,&clip)==XGE_OK && clip);return clip;
}
static void mix(void)
{
    /* Streaming file pages are real resource-manager jobs. A CPU consumer
     * must yield to those jobs, rather than demand data in its first quantum. */
    for(unsigned attempt=0;attempt<1000;++attempt){
        float output[512];assert(ma_engine_read_pcm_frames((ma_engine*)g_xge.pAudioEngine,output,256,NULL)==MA_SUCCESS);
        for(size_t i=0;i<512;++i)if(fabsf(output[i])>0.01f)return;
        ma_sleep(1);
    }
    assert(!"valid PCM did not arrive within the bounded streaming wait");
}
static void finish(void)
{
#if !defined(XGE_TEST_OLD_BASELINE)
    assert(!g_xge.iAudioCount && !testAudioGroups);
#if !defined(XGE_TEST_GRAPHICS_COUPLING_BASELINE)
    assert(!testAudioOwners);
#endif
    assert(xgeAudioUnitChecked()==XGE_OK && !xgeAudioIsReady());
#else
    xgeAudioUnit();
#endif
    assert(xgeUnitChecked()==XGE_OK);assert(xgeGraphicsOwnedResourceCount()==0);
}
#if !defined(XGE_TEST_OLD_BASELINE)
typedef struct WrongThread {xge_sound sound;xge_audio_group group;xge_audio_clip* clip;} WrongThread;
#if defined(_WIN32)
static DWORD WINAPI wrongThread(void* raw)
#else
static void* wrongThread(void* raw)
#endif
{
    WrongThread* w=raw;
    assert(xgeAudioUnitChecked()==XGE_ERROR_INVALID_STATE);
    assert(xgeSoundFreeChecked(w->sound)==XGE_ERROR_INVALID_STATE);
    assert(xgeAudioGroupFreeChecked(w->group)==XGE_ERROR_INVALID_STATE);
    assert(xgeSoundPlay(w->sound)==XGE_ERROR_INVALID_STATE);
    assert(!xgeSoundAddRef(w->sound));
    xgeAudioGroupSetVolume(w->group,0);assert(xrtGetError());xrtClearError();
    xge_sound_clip_desc_t d={w->clip,0,0,0,w->group};xge_sound rejected=(void*)1;
    assert(xgeSoundCreateFromClipOwned(&rejected,&d)==XGE_ERROR_INVALID_STATE && !rejected);
    return 0;
}
#endif
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=15);begin();
    if(mode==0){
        xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        xgeAudioUnit();assert(xgeAudioIsReady());xrtClearError();
        assert(xgeUnitChecked()==XGE_ERROR_BUSY && g_xge.bInitialized);
        xgeAudioGroupFree(&group);
#if !defined(XGE_TEST_OLD_BASELINE) && !defined(XGE_TEST_GRAPHICS_COUPLING_BASELINE) && !defined(XGE_TEST_ALLOCATOR_BASELINE)
        assert(__xgeAudioOwnerAcquire(&g_xgeGraphicsLifetime)==XGE_OK && !g_xge.iAudioCount);
        assert(xgeAudioUnitChecked()==XGE_ERROR_BUSY && xgeAudioIsReady());
        assert(__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime)==XGE_OK);
#endif
    }
    if(mode==1 || mode==2){
        xge_audio_group_t group={0};xge_sound_t sound={0};
        assert(xgeAudioGroupInit(&group)==XGE_OK);xge_audio_clip* clip=decode();
        xge_sound_clip_desc_t d={clip,0,0,0,&group};assert(xgeSoundFromClip(&sound,&d)==XGE_OK);
        xgeAudioClipFree(clip);
        if(mode==1){xgeAudioUnit();assert(xgeAudioIsReady());xrtClearError();}
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        xgeAudioGroupFree(&group);xrtMemDebugSnapshot(&after);
        assert(!group.pBackend && before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
        assert(xgeUnitChecked()==XGE_ERROR_BUSY && g_xge.bInitialized);
        xgeSoundSetLoop(&sound,1);assert(xgeSoundPlay(&sound)==XGE_OK);mix();
        assert(xgeSoundStop(&sound)==XGE_OK);xgeSoundFree(&sound);
    }
#if !defined(XGE_TEST_OLD_BASELINE)
    if(mode==3){
        FILE* file=fopen("audio-lifetime.wav","wb");assert(file);assert(fwrite(wave,1,sizeof(wave),file)==sizeof(wave));assert(!fclose(file));
        xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        xge_sound sound=NULL;xge_music music=NULL;xge_stream stream=NULL;
        assert(xgeSoundCreateOwned(&sound,"audio-lifetime.wav",&group)==XGE_OK);
        assert(xgeMusicCreateOwned(&music,"audio-lifetime.wav",&group)==XGE_OK);
        assert(xgeStreamCreateOwned(&stream,"audio-lifetime.wav",&group)==XGE_OK);
        xgeAudioGroupFree(&group);assert(g_xge.iAudioCount==3 && testAudioGroups==1);
        assert(xgeAudioUnitChecked()==XGE_ERROR_BUSY);
        xgeSoundSetLoop(sound,1);assert(xgeSoundPlay(sound)==XGE_OK);mix();
        assert(xgeSoundAddRef(sound)==2);xgeSoundFree(sound);assert(sound->iRefCount==1);
        xgeSoundFree(sound);xgeMusicFree(music);xgeStreamClose(stream);assert(!remove("audio-lifetime.wav"));
    }
    if(mode==4){
        xge_audio_clip* clip=decode();xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        xge_sound_clip_desc_t d={clip,0,0,0,&group};int failed=0,passed=0;
        for(unsigned budget=0;budget<40;++budget){
            xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
            uint64_t owners=xgeGraphicsOwnedResourceCount();assert(xrtMemDebugFailAfter(budget));
            xge_sound sound=(xge_sound)(uintptr_t)1;int status=xgeSoundCreateFromClipOwned(&sound,&d);
            xrtMemDebugFailClear();if(status==XGE_OK){++passed;assert(!xrtGetError());xgeSoundFree(sound);}else{++failed;assert(!sound);xrtClearError();}
            xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
            assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
            assert(xgeGraphicsOwnedResourceCount()==owners && !g_xge.iAudioCount && testAudioGroups==1);
        }
        assert(failed && passed);xgeAudioGroupFree(&group);xgeAudioClipFree(clip);
        printf("Audio ownership budgets: %d rejected, %d successful\n",failed,passed);
    }
    if(mode==5 || mode==6){
        xge_audio_clip* clip=decode();xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        xge_sound sound=NULL;xge_sound_clip_desc_t d={clip,0,0,0,&group};assert(xgeSoundCreateFromClipOwned(&sound,&d)==XGE_OK);
        void* soundBackend=sound->pBackend;void* groupBackend=group.pBackend;uint64_t owners=xgeGraphicsOwnedResourceCount();
        if(mode==5){
            WrongThread w={sound,&group,clip};
#if defined(_WIN32)
            HANDLE thread=CreateThread(NULL,0,wrongThread,&w,0,NULL);assert(thread);
            assert(WaitForSingleObject(thread,INFINITE)==WAIT_OBJECT_0);DWORD code=1;assert(GetExitCodeThread(thread,&code) && !code);assert(CloseHandle(thread));
#else
            pthread_t thread;assert(!pthread_create(&thread,NULL,wrongThread,&w));assert(!pthread_join(thread,NULL));
#endif
            assert(group.fVolume==1 && !xrtGetError());
        }else{
            assert(xgeSoundFallbackSet("does-not-exist.wav")==XGE_OK);
            assert(xgeSoundLoadGroup(sound,"ignored.wav",&group)==XGE_ERROR_INVALID_STATE);
            assert(xgeSoundFromClip(sound,&d)==XGE_ERROR_INVALID_STATE);
            assert(xgeAudioGroupInit(&group)==XGE_ERROR_INVALID_STATE);
        }
        assert(sound->pBackend==soundBackend && group.pBackend==groupBackend && sound->iRefCount==1);
        assert(xgeGraphicsOwnedResourceCount()==owners && g_xge.iAudioCount==1 && testAudioGroups==1);
        xgeSoundFree(sound);xgeAudioGroupFree(&group);xgeAudioClipFree(clip);
    }
    if(mode==7 || mode==10 || mode==11){
        FILE* file=fopen("audio-lifetime.wav","wb");assert(file);assert(fwrite(wave,1,sizeof(wave),file)==sizeof(wave));assert(!fclose(file));
        finish();int failed=0,passed=0;
        for(unsigned budget=0;budget<48;++budget){
            xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);begin();
            xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
            assert(xrtMemDebugFailAfter(budget));xge_sound sound=(void*)1;
            int result;
            if(mode==10){xge_music music=NULL;result=xgeMusicCreateOwned(&music,"audio-lifetime.wav",&group);sound=(xge_sound)music;}
            else if(mode==11){xge_stream stream=NULL;result=xgeStreamCreateOwned(&stream,"audio-lifetime.wav",&group);sound=(xge_sound)stream;}
            else result=xgeSoundCreateOwned(&sound,"audio-lifetime.wav",&group);
            xrtMemDebugFailClear();
            if(result==XGE_OK){
                ++passed;assert(!xrtGetError());
                if(mode==10){xgeMusicSetLoop((xge_music)sound,1);assert(xgeMusicPlay((xge_music)sound)==XGE_OK);}
                else if(mode==11){xgeStreamSetLoop((xge_stream)sound,1);assert(xgeStreamPlay((xge_stream)sound)==XGE_OK);}
                else{xgeSoundSetLoop(sound,1);assert(xgeSoundPlay(sound)==XGE_OK);}
                mix();
                if(mode==10)xgeMusicFree((xge_music)sound);
                else if(mode==11)xgeStreamClose((xge_stream)sound);
                else xgeSoundFree(sound);
            }else{++failed;assert(!sound);xrtClearError();}
            assert(!g_xge.iAudioCount && testAudioGroups==1);
            xgeAudioGroupFree(&group);finish();xrtMemDebugSnapshot(&after);
            assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
            assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
        }
        assert(failed && passed);assert(!remove("audio-lifetime.wav"));
        printf("Audio file ownership budgets: %d rejected, %d successful (mode %d)\n",failed,passed,mode);
    }
    if(mode>=13){
        FILE* file=fopen("audio-resident.wav","wb");assert(file);
        assert(fwrite(wave,1,sizeof(wave),file)==sizeof(wave));assert(!fclose(file));
        xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        int failed=0,passed=0;
        for(unsigned budget=0;budget<64;++budget){
            xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
            assert(xrtMemDebugFailAfter(budget));xge_sound sound=NULL;int result;
            if(mode==14){xge_music music=NULL;result=xgeMusicCreateOwned(&music,"audio-resident.wav",&group);sound=(xge_sound)music;}
            else if(mode==15){xge_stream stream=NULL;result=xgeStreamCreateOwned(&stream,"audio-resident.wav",&group);sound=(xge_sound)stream;}
            else result=xgeSoundCreateOwned(&sound,"audio-resident.wav",&group);
            xrtMemDebugFailClear();
            if(result==XGE_OK){++passed;assert(sound && !xrtGetError());assert(xgeSoundFreeChecked(sound)==XGE_OK);}
            else{++failed;assert(!sound);xrtClearError();}
            assert(!g_xge.iAudioCount && testAudioGroups==1);
            xrtMemDebugSnapshot(&after);
            assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
            assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
        }
        assert(failed && passed);xgeAudioGroupFree(&group);assert(!remove("audio-resident.wav"));
        printf("Audio resident budgets: %d rejected, %d successful (mode %d)\n",failed,passed,mode);
    }
    if(mode==8){
        xge_audio_group_t group={0};assert(xgeAudioGroupInit(&group)==XGE_OK);
        assert(xgeGraphicsOwnedResourceCount()==0);
#if !defined(XGE_TEST_GRAPHICS_COUPLING_BASELINE)
        assert(testAudioOwners==1);
#endif
        assert(__xgeGraphicsContextBegin(&g_xgeGraphicsLifetime)==XGE_OK);
        __xgeGraphicsContextCommit(&g_xgeGraphicsLifetime);
        __xgeGraphicsContextFinish(&g_xgeGraphicsLifetime,false);
        assert(xgeUnitChecked()==XGE_ERROR_BUSY && g_xge.bInitialized);
        xgeAudioGroupFree(&group);
        printf("Audio ownership layout: sound=%zu group=%zu, backend additions sound=%zu group=%zu, lifetime=%zu\n",
            sizeof(xge_sound_t),sizeof(xge_audio_group_t),sizeof(__xge_audio_sound)-sizeof(ma_sound),
            sizeof(__xge_audio_group)-sizeof(ma_sound_group),sizeof(xge_graphics_lifetime));
    }
#if !defined(XGE_TEST_GRAPHICS_COUPLING_BASELINE)
    if(mode==9){
        assert(!xrtGetError() && xrtMemDebugFailAfter(0));
        assert(!__xgeMaMalloc(32,NULL));xrtMemDebugFailClear();assert(!xrtGetError());
        xerror* ambient=xrtErrorCreate(XERR_STATE,"audio-allocator",81,"caller error");assert(ambient);xrtSetErrorTake(ambient);
        assert(xrtMemDebugFailAfter(0));assert(!__xgeMaMalloc(32,NULL));xrtMemDebugFailClear();assert(xrtGetError()==ambient);
        unsigned char* memory=__xgeMaMalloc(32,NULL);assert(memory && xrtGetError()==ambient);memset(memory,0x5a,32);
        assert(xrtMemDebugFailAfter(0));assert(!__xgeMaRealloc(memory,64,NULL));xrtMemDebugFailClear();
        assert(xrtGetError()==ambient && memory[0]==0x5a && memory[31]==0x5a);
        assert(!__xgeMaRealloc(memory,0,NULL) && xrtGetError()==ambient);xrtClearError();
#if !defined(XGE_TEST_ALLOCATOR_BASELINE)
        ambient=xrtErrorCreate(XERR_STATE,"audio-allocator",82,"previous error");assert(ambient);xrtSetErrorTake(ambient);
        xerror* held=xrtErrorRef(ambient);xerror* severe=xrtErrorCreate(XERR_STATE,"audio-allocator",83,"allocator validation error");assert(severe);xrtSetErrorTake(severe);
        assert(!__xgeMaAllocationResult(NULL,held) && xrtGetError()==severe);xrtClearError();
#endif
    }
    if(mode==12){
        xerror* ambient=xrtErrorCreate(XERR_STATE,"audio-path",91,"caller error");assert(ambient);xrtSetErrorTake(ambient);
        char* resolved=NULL;assert(__xgePathResolve("audio-lifetime.wav",&resolved)==XGE_OK && resolved && xrtGetError()==ambient);xrtFree(resolved);
        resolved=(char*)(uintptr_t)1;assert(__xgePathResolve("unsupported://file",&resolved)==XGE_ERROR_UNSUPPORTED && !resolved && xrtGetError()==ambient);xrtClearError();
        assert(xrtMemDebugFailAfter(0));assert(__xgePathResolve("audio-lifetime.wav",&resolved)==XGE_ERROR_OUT_OF_MEMORY && !resolved);
        xrtMemDebugFailClear();assert(xrtErrorKind(xrtGetError())==XERR_MEMORY);xrtClearError();
    }
#endif
#endif
    finish();printf("Audio lifetime mode %d passed: real CPU mixer, no device/window/network\n",mode);return 0;
}
