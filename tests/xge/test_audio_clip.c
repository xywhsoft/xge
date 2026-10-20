#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xge.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static int checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"CLIP %d: %s\n",__LINE__,#x);}}while(0)
static unsigned char wave[44+32];
static int cancelled(void *user){int *calls=user;return ++*calls>1;}
static DWORD WINAPI references(void *user){xge_audio_clip *clip=user;for(int i=0;i<5000;++i){if(xgeAudioClipAddRef(clip))return 1;xgeAudioClipFree(clip);}return 0;}
static xge_audio_clip *clip;
static int frame(void *unused){(void)unused;int error=xgeAudioInit();CHECK(!error);if(error)return 1;
    xge_audio_listener_t listener=xgeAudioListenerGet();xge_audio_group_t group={0};CHECK(!xgeAudioGroupInit(&group));if(!group.pBackend)return 1;xgeAudioGroupSetVolume(&group,0);
    xge_sound_t a={0},b={0},bad={0};xge_sound_clip_desc_t desc={clip,1,2,0,&group};CHECK(!xgeSoundFromClip(&a,&desc));desc.first_frame=3;desc.frame_count=4;CHECK(!xgeSoundFromClip(&b,&desc));
    CHECK(a.pBackend&&b.pBackend&&a.pBackend!=b.pBackend);desc.first_frame=8;CHECK(xgeSoundFromClip(&bad,&desc)==XGE_ERROR_INVALID_ARGUMENT&&!bad.pBackend);desc.first_frame=0;desc.frame_count=9;CHECK(xgeSoundFromClip(&bad,&desc)==XGE_ERROR_INVALID_ARGUMENT&&!bad.pBackend);
    CHECK(xgeSoundFromClip(&a,&desc)==XGE_ERROR_INVALID_STATE);xgeAudioClipFree(clip);clip=NULL;xgeSoundSetLoop(&a,1);xgeSoundSetLoop(&b,1);CHECK(!xgeSoundPlay(&a)&&!xgeSoundPlay(&b));CHECK(xgeSoundIsPlaying(&a)&&xgeSoundIsPlaying(&b));CHECK(!xgeSoundPause(&a)&&!xgeSoundIsPlaying(&a)&&xgeSoundIsPlaying(&b));CHECK(!xgeSoundResume(&a));CHECK(!xgeSoundStop(&a)&&!xgeSoundStop(&b));xgeSoundFree(&a);xgeSoundFree(&b);
    xge_audio_listener_t after=xgeAudioListenerGet();CHECK(!memcmp(&listener,&after,sizeof(after)));CHECK(xgeAudioIsReady());xgeAudioGroupFree(&group);return 1;
}
int main(void){memcpy(wave,"RIFF",4);wave[4]=68;memcpy(wave+8,"WAVEfmt ",8);wave[16]=16;wave[20]=1;wave[22]=2;wave[24]=0x40;wave[25]=0x1f;wave[28]=0;wave[29]=0x7d;wave[32]=4;wave[34]=16;memcpy(wave+36,"data",4);wave[40]=32;
    int16_t samples[16];for(int i=0;i<16;++i)samples[i]=(int16_t)((i-8)*2048);memcpy(wave+44,samples,sizeof(samples));xge_audio_clip_decode_t desc={wave,sizeof(wave),64,NULL,NULL};CHECK(!xgeAudioIsReady());CHECK(!xgeAudioClipDecodeMemory(&desc,&clip));xge_audio_clip_info_t info=xgeAudioClipInfo(clip);CHECK(info.frames==8&&info.channels==2&&info.sample_rate==8000&&info.bytes==64);
    const float *pcm=xgeAudioClipSamples(clip);for(int i=0;i<16;++i)CHECK(pcm&&fabsf(pcm[i]-samples[i]/32768.f)<1e-6f);memset(wave+44,0,32);CHECK(pcm&&fabsf(pcm[0]+.5f)<1e-6f);CHECK(!xgeAudioIsReady());
    HANDLE workers[4];for(int i=0;i<4;++i){workers[i]=CreateThread(NULL,0,references,clip,0,NULL);CHECK(workers[i]!=NULL);}for(int i=0;i<4;++i)if(workers[i]){DWORD code=1;WaitForSingleObject(workers[i],INFINITE);CHECK(GetExitCodeThread(workers[i],&code)&&code==0);CloseHandle(workers[i]);}
    xge_audio_clip *bad=(void*)1;desc.max_decoded_bytes=63;CHECK(xgeAudioClipDecodeMemory(&desc,&bad)==XGE_ERROR_OUT_OF_MEMORY&&!bad);desc.max_decoded_bytes=64;int calls=0;desc.cancelled=cancelled;desc.user=&calls;CHECK(xgeAudioClipDecodeMemory(&desc,&bad)==XGE_ERROR_INVALID_STATE&&!bad&&calls>1);desc.cancelled=NULL;desc.data="bad";desc.bytes=3;CHECK(xgeAudioClipDecodeMemory(&desc,&bad)<0&&!bad);
    xge_desc_t window={.sTitle="XGE shared PCM clip verification",.iWidth=160,.iHeight=120,.iTargetFPS=60};int e=xgeInit(&window);CHECK(!e);if(!e){ShowWindow((HWND)xgePlatformNativeHandle(),SW_HIDE);CHECK(!xgeRun(frame,NULL));}xgeAudioClipFree(clip);xgeUnit();printf("Audio clip: %d checks, %d failures; CPU decode, exact samples, memory limit, cancellation, 20000 shared references and independent real voices\n",checks,failures);return failures?1:0;}
