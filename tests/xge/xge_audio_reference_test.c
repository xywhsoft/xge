/* Resource-reference boundaries without initializing an engine or device. */
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
int main(int argc,char** argv)
{
 assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=4);
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 assert(xrtMemDebugEnable(true));xge_sound_t sound={0};
 if(mode==0){assert(xgeSoundAddRef(NULL)==0);assert(xgeSoundAddRef(&sound)==0 && sound.iRefCount==0);}
 if(mode==1){sound.iRefCount=-1;assert(xgeSoundAddRef(&sound)==0 && sound.iRefCount==-1);}
 if(mode==2){sound.iRefCount=INT_MAX;assert(xgeSoundAddRef(&sound)==0 && sound.iRefCount==INT_MAX);}
 if(mode==3){
  sound.iRefCount=1;assert(xgeSoundAddRef(&sound)==2 && sound.iRefCount==2);
  xgeSoundFree(&sound);assert(sound.iRefCount==1);
  sound.iRefCount=INT_MAX-1;assert(xgeSoundAddRef(&sound)==INT_MAX && sound.iRefCount==INT_MAX);
 }
 if(mode==4){
  sound.iRefCount=1;xerror* ambient=xrtErrorCreate(XERR_STATE,"audio-reference-test",71,"caller error");assert(ambient);xrtSetErrorTake(ambient);
  xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
  for(int i=0;i<1000;++i){assert(xgeSoundAddRef(&sound)==2);xgeSoundFree(&sound);assert(sound.iRefCount==1);}
  sound.iRefCount=INT_MAX;assert(xgeSoundAddRef(&sound)==0 && xrtGetError()==ambient);
  assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
  assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
  assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);xrtClearError();
 }
 assert(!xgeAudioIsReady());printf("Audio reference mode %d: no resurrection/overflow/allocation passed\n",mode);return 0;
}
