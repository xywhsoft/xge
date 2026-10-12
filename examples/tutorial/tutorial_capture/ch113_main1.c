/* ch113 - Audio group routing: a sound inherits the group's mix volume. */
#include "tut_capture.h"
#include "tut_audio_test.h"
static char g_path[512]; static int g_ran,g_ok;
static void draw_scene(void){if(!g_ran){xge_audio_group_t group;xge_sound_t sound;memset(&group,0,sizeof(group));memset(&sound,0,sizeof(sound));if(xgeAudioInit()==XGE_OK&&xgeAudioGroupInit(&group)==XGE_OK&&tut_audio_make_wav(g_path,(int)sizeof(g_path),"xge_tutorial_group.wav")){xgeAudioGroupSetVolume(&group,.45f);if(xgeSoundLoadGroup(&sound,g_path,&group)==XGE_OK&&xgeSoundPlay(&sound)==XGE_OK&&xgeSoundIsPlaying(&sound)){(void)xgeSoundStop(&sound);g_ok=!xgeSoundIsPlaying(&sound)&&fabsf(xgeAudioGroupGetVolume(&group)-.45f)<.01f;}xgeSoundFree(&sound);}xgeAudioGroupFree(&group);printf("ch113 audio-group self-test: %s\n",g_ok?"PASS":"FAIL");g_ran=1;}tut_audio_status_panel(g_ok,XGE_COLOR_RGBA(255,107,94,255));}
int main(int argc,char**argv){int r=tut_run(draw_scene,"ch113-group",argc,argv);remove(g_path);return r;}
