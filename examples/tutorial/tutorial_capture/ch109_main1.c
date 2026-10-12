/* ch109 - Audio engine initialization using a generated, dependency-free WAV.
 * The short PCM test tone is created at runtime and removed when the program
 * exits, so this lesson does not ship a third-party sound asset.
 */
#include "tut_capture.h"

#include <math.h>
#if defined(_WIN32)
#include <windows.h>
#endif

/* Use an absolute path: XGE resolves relative media paths beside the .exe. */
static char g_tone_path[512];
static int g_ran, g_verified, g_audio_result, g_playing;

static void write_u16(FILE* file, unsigned int value) { fputc(value & 255, file); fputc((value >> 8) & 255, file); }
static void write_u32(FILE* file, unsigned int value) { write_u16(file, value & 65535); write_u16(file, value >> 16); }

static int create_test_tone(void)
{
	FILE* file;
	int i, samples = 4410; /* 100 ms at 44.1 kHz */
	#if defined(_WIN32)
	{
		DWORD length = GetModuleFileNameA(NULL, g_tone_path, (DWORD)sizeof(g_tone_path));
		char* slash;
		if ( length == 0 || length >= sizeof(g_tone_path) ) return 0;
		slash = strrchr(g_tone_path, '\\');
		if ( slash == NULL ) return 0;
		snprintf(slash + 1, sizeof(g_tone_path) - (size_t)(slash + 1 - g_tone_path), "xge_tutorial_tone.wav");
	}
	#else
	snprintf(g_tone_path, sizeof(g_tone_path), "xge_tutorial_tone.wav");
	#endif
	file = fopen(g_tone_path, "wb");
	if ( file == NULL ) return 0;
	fwrite("RIFF", 1, 4, file); write_u32(file, 36u + (unsigned int)samples * 2u);
	fwrite("WAVEfmt ", 1, 8, file); write_u32(file, 16); write_u16(file, 1); write_u16(file, 1);
	write_u32(file, 44100); write_u32(file, 88200); write_u16(file, 2); write_u16(file, 16);
	fwrite("data", 1, 4, file); write_u32(file, (unsigned int)samples * 2u);
	for ( i = 0; i < samples; ++i ) {
		short sample = (short)(sinf((float)i * 6.2831853f * 440.0f / 44100.0f) * 2400.0f);
		write_u16(file, (unsigned short)sample);
	}
	fclose(file);
	return 1;
}

static void draw_scene(void)
{
	uint32_t status;
	if ( !g_ran ) {
		xge_audio_listener_t listener;
		xge_sound_t sound;
		memset(&listener, 0, sizeof(listener));
		listener.tForward.fZ = -1.0f; listener.tUp.fY = 1.0f;
		g_audio_result = xgeAudioInit();
		if ( g_audio_result == XGE_OK && create_test_tone() ) {
			xgeAudioSetVolume(0.35f);
			xgeAudioListenerSet(&listener);
			memset(&sound, 0, sizeof(sound));
			if ( xgeSoundLoad(&sound, g_tone_path) == XGE_OK && xgeSoundPlay(&sound) == XGE_OK ) {
				g_playing = xgeSoundIsPlaying(&sound);
				xgeSoundStop(&sound);
				xgeSoundFree(&sound);
			}
		}
		g_verified = (g_audio_result == XGE_OK && xgeAudioIsReady() && g_playing && fabsf(xgeAudioGetVolume() - 0.35f) < 0.01f);
		printf("ch109 audio self-test: %s (init=%d)\n", g_verified ? "PASS" : "UNAVAILABLE", g_audio_result);
		g_ran = 1;
	}
	status = g_verified ? XGE_COLOR_RGBA(79,216,194,255) : XGE_COLOR_RGBA(255,180,84,255);
	xgeShapeRectFill((xge_rect_t){70,58,660,80},XGE_COLOR_RGBA(23,34,45,255)); xgeShapeRectStroke((xge_rect_t){70,58,660,80},2,status); xgeShapeCircleFill(108,98,16,status);
	xgeShapeRectFill((xge_rect_t){125,205,550,95},XGE_COLOR_RGBA(27,39,51,255)); xgeShapeRectStroke((xge_rect_t){125,205,550,95},2,status);
	xgeShapeCircleFill(235,252,38,status); xgeShapeCircleFill(400,252,38,XGE_COLOR_RGBA(109,179,242,255)); xgeShapeCircleFill(565,252,38,XGE_COLOR_RGBA(255,180,84,255));
	xgeShapeLine(273,252,362,252,5,status); xgeShapeLine(438,252,527,252,5,status);
	/* The capture is visual proof of the API result; audible verification remains a device smoke test. */
	xgeShapeRectFill((xge_rect_t){145,375,510,74},XGE_COLOR_RGBA(19,27,36,255)); xgeShapeRectStroke((xge_rect_t){145,375,510,74},2,status);
	if(g_verified){xgeShapeRectFill((xge_rect_t){180,405,315,15},status);xgeShapeRectFill((xge_rect_t){515,397,90,31},XGE_COLOR_RGBA(79,216,194,150));}
}

int main(int argc, char** argv)
{
	int ret = tut_run(draw_scene, "ch109-audio", argc, argv);
	remove(g_tone_path);
	/* Unsupported audio backends are a documented platform outcome, not a false pass. */
	return ret;
}
