/* Shared self-contained WAV fixture for audio tutorial chapters.
 * It generates a short PCM tone next to the executable, where XGE resolves
 * relative media paths, and leaves no binary asset in the repository.
 */
#ifndef TUT_AUDIO_TEST_H
#define TUT_AUDIO_TEST_H

#include <math.h>
#if defined(_WIN32)
#include <windows.h>
#endif

static void tut_audio_u16(FILE* file, unsigned int value) { fputc(value & 255, file); fputc((value >> 8) & 255, file); }
static void tut_audio_u32(FILE* file, unsigned int value) { tut_audio_u16(file, value & 65535); tut_audio_u16(file, value >> 16); }

static int tut_audio_make_wav(char* path, int capacity, const char* name)
{
	FILE* file;
	int i, samples = 22050;
#if defined(_WIN32)
	DWORD length = GetModuleFileNameA(NULL, path, (DWORD)capacity);
	char* slash;
	if ( length == 0 || length >= (DWORD)capacity || (slash = strrchr(path, '\\')) == NULL ) return 0;
	snprintf(slash + 1, (size_t)capacity - (size_t)(slash + 1 - path), "%s", name);
#else
	snprintf(path, (size_t)capacity, "%s", name);
#endif
	file = fopen(path, "wb");
	if ( file == NULL ) return 0;
	fwrite("RIFF", 1, 4, file); tut_audio_u32(file, 36u + (unsigned int)samples * 2u);
	fwrite("WAVEfmt ", 1, 8, file); tut_audio_u32(file, 16); tut_audio_u16(file, 1); tut_audio_u16(file, 1);
	tut_audio_u32(file, 44100); tut_audio_u32(file, 88200); tut_audio_u16(file, 2); tut_audio_u16(file, 16);
	fwrite("data", 1, 4, file); tut_audio_u32(file, (unsigned int)samples * 2u);
	for ( i = 0; i < samples; ++i ) {
		short sample = (short)(sinf((float)i * 6.2831853f * 330.0f / 44100.0f) * 1800.0f);
		tut_audio_u16(file, (unsigned short)sample);
	}
	fclose(file);
	return 1;
}

static void tut_audio_status_panel(int pass, int accent)
{
	uint32_t status = pass ? XGE_COLOR_RGBA(79, 216, 194, 255) : XGE_COLOR_RGBA(255, 107, 94, 255);
	xgeShapeRectFill((xge_rect_t){70, 58, 660, 80}, XGE_COLOR_RGBA(23, 34, 45, 255));
	xgeShapeRectStroke((xge_rect_t){70, 58, 660, 80}, 2.0f, status);
	xgeShapeCircleFill(108, 98, 16, status);
	xgeShapeRectFill((xge_rect_t){135, 230, 530, 90}, XGE_COLOR_RGBA(27, 39, 51, 255));
	xgeShapeRectStroke((xge_rect_t){135, 230, 530, 90}, 2.0f, status);
	xgeShapeCircleFill(230, 275, 35, status);
	xgeShapeLine(270, 275, 505, 275, 6.0f, status);
	xgeShapeCircleFill(550, 275, 35, (uint32_t)accent);
	xgeShapeRectFill((xge_rect_t){175, 405, 450, 28}, status);
	xgeShapeRectFill((xge_rect_t){505, 397, 85, 44}, (uint32_t)accent);
}

#endif /* TUT_AUDIO_TEST_H */
