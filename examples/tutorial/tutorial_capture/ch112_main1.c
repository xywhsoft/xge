/* ch112 - Stream open/play/stop test. Streams use the same generated WAV. */
#include "tut_capture.h"
#include "tut_audio_test.h"
static char g_path[512]; static int g_ran,g_ok;
static void draw_scene(void){if(!g_ran){xge_stream_t stream;memset(&stream,0,sizeof(stream));if(xgeAudioInit()==XGE_OK&&tut_audio_make_wav(g_path,(int)sizeof(g_path),"xge_tutorial_stream.wav")&&xgeStreamOpen(&stream,g_path)==XGE_OK){xgeStreamSetLoop(&stream,1);xgeStreamSetVolume(&stream,.20f);xgeStreamSetPosition(&stream,1,0,0);if(xgeStreamPlay(&stream)==XGE_OK&&xgeStreamIsPlaying(&stream)){(void)xgeStreamPause(&stream);(void)xgeStreamResume(&stream);(void)xgeStreamStop(&stream);g_ok=!xgeStreamIsPlaying(&stream);}xgeStreamClose(&stream);}printf("ch112 stream self-test: %s\n",g_ok?"PASS":"FAIL");g_ran=1;}tut_audio_status_panel(g_ok,XGE_COLOR_RGBA(255,180,84,255));}
int main(int argc,char**argv){int r=tut_run(draw_scene,"ch112-stream",argc,argv);remove(g_path);return r;}
