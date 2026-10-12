/* ch114 - Async sound load with explicit polling on the UI thread. */
#include "tut_capture.h"
#include "tut_audio_test.h"
static char g_path[512]; static int g_ran,g_ok;
static void draw_scene(void){if(!g_ran){xge_async_request_t request;xge_sound_t sound;memset(&sound,0,sizeof(sound));xgeAsyncRequestInit(&request);if(xgeAudioInit()==XGE_OK&&tut_audio_make_wav(g_path,(int)sizeof(g_path),"xge_tutorial_async.wav")){(void)xgeAsyncThreadingSet(0);if(xgeAsyncSoundLoad(&request,&sound,g_path,NULL,NULL)==XGE_OK&&xgeAsyncPoll(&request)==XGE_ASYNC_READY&&xgeSoundPlay(&sound)==XGE_OK){g_ok=xgeSoundIsPlaying(&sound);(void)xgeSoundStop(&sound);}}xgeSoundFree(&sound);xgeAsyncRequestFree(&request);printf("ch114 async-sound self-test: %s\n",g_ok?"PASS":"FAIL");g_ran=1;}tut_audio_status_panel(g_ok,XGE_COLOR_RGBA(109,179,242,255));}
int main(int argc,char**argv){int r=tut_run(draw_scene,"ch114-async",argc,argv);remove(g_path);return r;}
