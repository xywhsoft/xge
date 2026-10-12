/* ch110 - Short sound lifecycle: load, play, pause, resume and stop.
 * The WAV is generated only for this deterministic tutorial test and deleted.
 */
#include "tut_capture.h"

#include <math.h>
#if defined(_WIN32)
#include <windows.h>
#endif

static char g_path[512];
static int g_ran, g_verified, g_loaded, g_played, g_stopped;

static void wav_u16(FILE* f,unsigned v){fputc(v&255,f);fputc((v>>8)&255,f);} static void wav_u32(FILE* f,unsigned v){wav_u16(f,v&65535);wav_u16(f,v>>16);}
static int make_wav(void)
{
	FILE* f; int i,n=22050;
#if defined(_WIN32)
	DWORD len=GetModuleFileNameA(NULL,g_path,(DWORD)sizeof(g_path));char* slash;
	if(!len||len>=sizeof(g_path)||(slash=strrchr(g_path,'\\'))==NULL)return 0;
	snprintf(slash+1,sizeof(g_path)-(size_t)(slash+1-g_path),"xge_tutorial_lifecycle.wav");
#else
	snprintf(g_path,sizeof(g_path),"xge_tutorial_lifecycle.wav");
#endif
	f=fopen(g_path,"wb");if(!f)return 0;
	fwrite("RIFF",1,4,f);wav_u32(f,36u+(unsigned)n*2u);fwrite("WAVEfmt ",1,8,f);wav_u32(f,16);wav_u16(f,1);wav_u16(f,1);wav_u32(f,44100);wav_u32(f,88200);wav_u16(f,2);wav_u16(f,16);fwrite("data",1,4,f);wav_u32(f,(unsigned)n*2u);
	for(i=0;i<n;i++){short s=(short)(sinf((float)i*6.2831853f*330.0f/44100.0f)*1800.0f);wav_u16(f,(unsigned short)s);}fclose(f);return 1;
}
static void draw_scene(void)
{
	uint32_t c;
	if(!g_ran){xge_sound_t sound;memset(&sound,0,sizeof(sound));if(xgeAudioInit()==XGE_OK&&make_wav()&&xgeSoundLoad(&sound,g_path)==XGE_OK){g_loaded=1;xgeSoundSetLoop(&sound,1);xgeSoundSetVolume(&sound,.25f);if(xgeSoundPlay(&sound)==XGE_OK){g_played=xgeSoundIsPlaying(&sound);(void)xgeSoundPause(&sound);(void)xgeSoundResume(&sound);(void)xgeSoundStop(&sound);g_stopped=!xgeSoundIsPlaying(&sound);}xgeSoundFree(&sound);}g_verified=g_loaded&&g_played&&g_stopped;printf("ch110 sound lifecycle self-test: %s\n",g_verified?"PASS":"FAIL");g_ran=1;}
	c=g_verified?XGE_COLOR_RGBA(79,216,194,255):XGE_COLOR_RGBA(255,107,94,255);xgeShapeRectFill((xge_rect_t){70,58,660,80},XGE_COLOR_RGBA(23,34,45,255));xgeShapeRectStroke((xge_rect_t){70,58,660,80},2,c);xgeShapeCircleFill(108,98,16,c);
	for(int i=0;i<4;i++){float x=105+i*165;xgeShapeRectFill((xge_rect_t){x,245,130,88},XGE_COLOR_RGBA(27,39,51,255));xgeShapeRectStroke((xge_rect_t){x,245,130,88},2,(i<3)?c:XGE_COLOR_RGBA(255,180,84,255));xgeShapeCircleFill(x+65,289,19,(i<3)?c:XGE_COLOR_RGBA(255,180,84,255));if(i<3)xgeShapeLine(x+130,289,x+164,289,4,c);}xgeShapeRectFill((xge_rect_t){145,425,510,24},c);xgeShapeRectFill((xge_rect_t){520,417,85,40},XGE_COLOR_RGBA(255,180,84,255));
}
int main(int argc,char**argv){int r=tut_run(draw_scene,"ch110-sound",argc,argv);remove(g_path);return r;}
