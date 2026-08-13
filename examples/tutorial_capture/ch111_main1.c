/* ch111 - Music stream lifecycle, using a generated WAV test fixture. */
#include "tut_capture.h"
#include "tut_audio_test.h"
static char g_path[512]; static int g_ran,g_ok;
static void draw_scene(void){if(!g_ran){xge_music_t music;memset(&music,0,sizeof(music));if(xgeAudioInit()==XGE_OK&&tut_audio_make_wav(g_path,(int)sizeof(g_path),"xge_tutorial_music.wav")&&xgeMusicLoad(&music,g_path)==XGE_OK){xgeMusicSetLoop(&music,1);xgeMusicSetVolume(&music,.30f);if(xgeMusicPlay(&music)==XGE_OK&&xgeMusicIsPlaying(&music)){(void)xgeMusicPause(&music);(void)xgeMusicResume(&music);(void)xgeMusicStop(&music);g_ok=!xgeMusicIsPlaying(&music);}xgeMusicFree(&music);}printf("ch111 music self-test: %s\n",g_ok?"PASS":"FAIL");g_ran=1;}tut_audio_status_panel(g_ok,XGE_COLOR_RGBA(109,179,242,255));}
int main(int argc,char**argv){int r=tut_run(draw_scene,"ch111-music",argc,argv);remove(g_path);return r;}
