/* Immutable CPU PCM span; valid WAV, no device/window/network. */
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
typedef struct ClipCancelState {
 xge_audio_clip_decode_t* original;
 unsigned polls;
} ClipCancelState;

static int changeDescriptor(void* data)
{
 ClipCancelState* state=data;
 ++state->polls;
 *state->original=(xge_audio_clip_decode_t){0};
 return 0;
}

static void checkSamples(xge_audio_clip* clip,const float* expected,size_t count)
{
 const float* data=NULL;
 size_t size=0;
 assert(xgeAudioClipGetSamples(clip,&data,&size)==XGE_OK && data==expected && size==count);
 for(size_t i=0;i<size;++i)assert(fabsf(data[i]-((int)i-8)/16.0f)<0.000001f);
}

typedef struct ClipWorker {
 xge_audio_clip* owned;
 const float* samples;
 size_t count;
} ClipWorker;

static int32 workerRead(ptr data)
{
 ClipWorker* state=data;
 for(int round=0;round<1000;++round){
  xge_audio_clip* share=NULL;
  assert(xgeAudioClipShareOwned(state->owned,&share)==XGE_OK && share==state->owned);
  checkSamples(share,state->samples,state->count);
  xgeAudioClipFree(share);
 }
 xgeAudioClipFree(state->owned);
 return 0;
}

int main(void)
{
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 unsigned char wave[76]={0};memcpy(wave,"RIFF",4);wave[4]=68;memcpy(wave+8,"WAVEfmt ",8);
 wave[16]=16;wave[20]=1;wave[22]=2;wave[24]=0x40;wave[25]=0x1f;wave[29]=0x7d;wave[32]=4;wave[34]=16;
 memcpy(wave+36,"data",4);wave[40]=32;
 int16_t values[16];for(int i=0;i<16;++i)values[i]=(int16_t)((i-8)*2048);memcpy(wave+44,values,sizeof(values));
 assert(xrtMemDebugEnable(true) && !xgeAudioIsReady());
 xge_audio_clip_decode_t desc={wave,sizeof(wave),128,NULL,NULL};xge_audio_clip* clip=NULL;
 assert(xgeAudioClipDecodeMemory(&desc,&clip)==XGE_OK && clip);xgeAudioClipFree(clip);clip=NULL;
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 ClipCancelState state={&desc,0};
 desc.cancelled=changeDescriptor;
 desc.user=&state;
 assert(xgeAudioClipDecodeMemory(&desc,&clip)==XGE_OK && clip);
 assert(!desc.data && !desc.cancelled && state.polls>=3);
 const float* samples=NULL;size_t count=0;assert(xgeAudioClipGetSamples(clip,&samples,&count)==XGE_OK);
 xge_audio_clip_info_t info=xgeAudioClipInfo(clip);
 assert(samples==xgeAudioClipSamples(clip) && count==16 && count==info.frames*info.channels && count*sizeof(float)==info.bytes);
 xge_audio_clip* retained=NULL;
 assert(xgeAudioClipShareOwned(clip,&retained)==XGE_OK && retained==clip);
 xgeAudioClipFree(clip);clip=NULL;
 memset(wave+44,0,32);assert(xrtMemDebugFailAfter(0));
 for(int round=0;round<1000;++round){
  xge_audio_clip* share=NULL;
  assert(xgeAudioClipShareOwned(retained,&share)==XGE_OK && share==retained);
  checkSamples(share,samples,count);
  xgeAudioClipFree(share);
 }
 assert(!xrtMemDebugFailTriggered() && !xrtGetError());xrtMemDebugFailClear();
 ClipWorker worker={NULL,samples,count};
 assert(xgeAudioClipShareOwned(retained,&worker.owned)==XGE_OK);
 xthread* thread=xrtThreadCreate(workerRead,&worker,0);
 assert(thread);
 for(int round=0;round<1000;++round)checkSamples(retained,samples,count);
 assert(xrtThreadWaitFor(thread,30000)==XWAIT_OK && xrtThreadExitCode(thread)==0);
 xrtThreadDestroy(thread);
 xge_audio_clip* rejected=retained;
 assert(xgeAudioClipShareOwned(NULL,&rejected)==XGE_ERROR_INVALID_ARGUMENT && !rejected);
 assert(xgeAudioClipShareOwned(retained,NULL)==XGE_ERROR_INVALID_ARGUMENT);
 const float* invalid=(const float*)(uintptr_t)1;size_t invalidCount=SIZE_MAX;
 assert(xgeAudioClipGetSamples(NULL,&invalid,&invalidCount)==XGE_ERROR_INVALID_ARGUMENT && !invalid && !invalidCount);
 invalidCount=SIZE_MAX;assert(xgeAudioClipGetSamples(retained,NULL,&invalidCount)==XGE_ERROR_INVALID_ARGUMENT && !invalidCount);
 invalid=(const float*)(uintptr_t)1;assert(xgeAudioClipGetSamples(retained,&invalid,NULL)==XGE_ERROR_INVALID_ARGUMENT && !invalid);
 xgeAudioClipFree(retained);assert(!xgeAudioIsReady() && !xrtGetError());
 xrtMemDebugSnapshot(&after);assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes &&
  after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
 puts("CPU clip span: stable descriptor, exact PCM, concurrent owned references and 1000 zero-allocation shares");
 return 0;
}
