/* Valid generated PCM only; CPU decode never opens a device or window. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
static unsigned char wave[76];
static void prepare(void)
{
 memcpy(wave,"RIFF",4);wave[4]=68;memcpy(wave+8,"WAVEfmt ",8);
 wave[16]=16;wave[20]=1;wave[22]=2;wave[24]=0x40;wave[25]=0x1f;
 wave[29]=0x7d;wave[32]=4;wave[34]=16;memcpy(wave+36,"data",4);wave[40]=32;
 int16_t samples[16];for(int i=0;i<16;++i)samples[i]=(int16_t)((i-8)*2048);
 memcpy(wave+44,samples,sizeof(samples));
}
int main(void)
{
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 prepare();assert(!xgeAudioIsReady() && xrtMemDebugEnable(true));
 xge_audio_clip_decode_t desc={wave,sizeof(wave),128,NULL,NULL};xge_audio_clip* clip=NULL;
 assert(xgeAudioClipDecodeMemory(&desc,&clip)==XGE_OK && clip);xgeAudioClipFree(clip);clip=NULL;
 assert(xrtMemDebugFailAfter(0));assert(xgeAudioClipDecodeMemory(&desc,&clip)==XGE_ERROR_OUT_OF_MEMORY && !clip);
 xrtMemDebugFailClear();xrtClearError();xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 unsigned refused=0,succeeded=0,optionalFailures=0;
 for(int budget=0;budget<8;++budget){
  assert(xrtMemDebugFailAfter(budget));
  int status=xgeAudioClipDecodeMemory(&desc,&clip),hit=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
  if(status==XGE_OK){
   assert(clip && !xrtGetError());++succeeded;if(hit)++optionalFailures;
   xge_audio_clip_info_t info=xgeAudioClipInfo(clip);assert(info.frames==8 && info.channels==2 && info.sample_rate==8000 && info.bytes==64);
   const float* samples=xgeAudioClipSamples(clip);assert(samples);
   for(int i=0;i<16;++i)assert(fabsf(samples[i]-(i-8)/16.0f)<0.000001f);
  }else{assert(status==XGE_ERROR_OUT_OF_MEMORY && !clip && hit && xrtGetError());++refused;}
  xrtClearError();xgeAudioClipFree(clip);clip=NULL;
 }
 assert(refused==2 && succeeded==6 && optionalFailures==1);
 xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();assert(ambient);
 assert(xrtMemDebugFailAfter(2));assert(xgeAudioClipDecodeMemory(&desc,&clip)==XGE_OK && clip);
 assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();assert(xrtGetError()==ambient);
 xrtClearError();xgeAudioClipFree(clip);assert(!xgeAudioIsReady());
 xrtMemDebugSnapshot(&after);assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes &&
  after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
 printf("CPU audio clip OOM: %u refused, %u succeeded, %u optional failure; ambient error preserved; no device\n",refused,succeeded,optionalFailures);
 return 0;
}
