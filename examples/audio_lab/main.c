#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define XGE_GETCWD _getcwd
#define XGE_MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>
#define XGE_GETCWD getcwd
#define XGE_MKDIR(path) mkdir(path, 0755)
#endif
#include "../../xge.h"

typedef struct audio_lab_t {
	xge_audio_group_t tGroup;
	xge_sound_t tSound;
	xge_sound_t tSoundGroup;
	xge_sound_t tFallbackSound;
	xge_sound_t tFallbackCopy;
	xge_music_t tMusic;
	xge_music_t tMusicGroup;
	xge_stream_t tStream;
	xge_stream_t tStreamGroup;
	xge_audio_listener_t tListener;
	char sGeneratedPath[1024];
	const char* sSoundPath;
	const char* sMusicPath;
	const char* sStreamPath;
	int iFrameLimit;
	int bAudioReady;
	int bGroupReady;
	int bGenerated;
	int bFallbackSet;
	int bFallbackLoad;
	int bFallbackGet;
	int bSoundLoaded;
	int bSoundGroupLoaded;
	int bSoundAddRef;
	int bSoundPlay;
	int bSoundPause;
	int bSoundResume;
	int bSoundStop;
	int bMusicLoaded;
	int bMusicGroupLoaded;
	int bMusicPlay;
	int bMusicPause;
	int bMusicResume;
	int bMusicStop;
	int bStreamLoaded;
	int bStreamGroupLoaded;
	int bStreamPlay;
	int bStreamPause;
	int bStreamResume;
	int bStreamStop;
	float fMasterVolume;
	float fGroupVolume;
	float fListenerX;
} audio_lab_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void WriteU16(FILE* pFile, unsigned int iValue)
{
	unsigned char b[2];

	b[0] = (unsigned char)(iValue & 0xFFu);
	b[1] = (unsigned char)((iValue >> 8) & 0xFFu);
	(void)fwrite(b, 1, 2, pFile);
}

static void WriteU32(FILE* pFile, unsigned int iValue)
{
	unsigned char b[4];

	b[0] = (unsigned char)(iValue & 0xFFu);
	b[1] = (unsigned char)((iValue >> 8) & 0xFFu);
	b[2] = (unsigned char)((iValue >> 16) & 0xFFu);
	b[3] = (unsigned char)((iValue >> 24) & 0xFFu);
	(void)fwrite(b, 1, 4, pFile);
}

static int WriteToneWav(const char* sPath)
{
	FILE* pFile;
	const unsigned int iSampleRate = 22050u;
	const unsigned int iSamples = 22050u;
	const unsigned int iDataBytes = iSamples * 2u;
	unsigned int i;

	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	(void)fwrite("RIFF", 1, 4, pFile);
	WriteU32(pFile, 36u + iDataBytes);
	(void)fwrite("WAVE", 1, 4, pFile);
	(void)fwrite("fmt ", 1, 4, pFile);
	WriteU32(pFile, 16u);
	WriteU16(pFile, 1u);
	WriteU16(pFile, 1u);
	WriteU32(pFile, iSampleRate);
	WriteU32(pFile, iSampleRate * 2u);
	WriteU16(pFile, 2u);
	WriteU16(pFile, 16u);
	(void)fwrite("data", 1, 4, pFile);
	WriteU32(pFile, iDataBytes);
	for ( i = 0; i < iSamples; i++ ) {
		int iHalfWave;
		short iSample;

		iHalfWave = (int)((i * 880u / iSampleRate) & 1u);
		iSample = (short)(iHalfWave ? 6000 : -6000);
		WriteU16(pFile, (unsigned short)iSample);
	}
	fclose(pFile);
	return XGE_OK;
}

static int MakeGeneratedPath(char* sOut, int iSize)
{
	char sCwd[768];

	if ( (sOut == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( XGE_GETCWD(sCwd, sizeof(sCwd)) == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	(void)XGE_MKDIR("build");
	snprintf(sOut, (size_t)iSize, "%s/build/audio_lab_tone.wav", sCwd);
	return XGE_OK;
}

static void ParseArgs(audio_lab_t* pLab, int argc, char** argv)
{
	int i;

	pLab->iFrameLimit = ArgInt(getenv("XGE_AUDIO_LAB_FRAMES"), 120);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			pLab->iFrameLimit = ArgInt(argv[++i], pLab->iFrameLimit);
		} else if ( (strcmp(argv[i], "--sound") == 0) && ((i + 1) < argc) ) {
			pLab->sSoundPath = argv[++i];
		} else if ( (strcmp(argv[i], "--music") == 0) && ((i + 1) < argc) ) {
			pLab->sMusicPath = argv[++i];
		} else if ( (strcmp(argv[i], "--stream") == 0) && ((i + 1) < argc) ) {
			pLab->sStreamPath = argv[++i];
		} else if ( argv[i][0] != '-' ) {
			pLab->sSoundPath = argv[i];
			pLab->sMusicPath = argv[i];
			pLab->sStreamPath = argv[i];
		}
	}
}

static int InitAudioLab(audio_lab_t* pLab)
{
	xge_audio_listener_t tListener;
	int iRet;

	iRet = MakeGeneratedPath(pLab->sGeneratedPath, (int)sizeof(pLab->sGeneratedPath));
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "audio-lab stage failed: generated path ret=%d\n", iRet);
		return iRet;
	}
	iRet = WriteToneWav(pLab->sGeneratedPath);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "audio-lab stage failed: write wav ret=%d path=%s\n", iRet, pLab->sGeneratedPath);
		return iRet;
	}
	pLab->bGenerated = 1;
	if ( pLab->sSoundPath == NULL ) {
		pLab->sSoundPath = pLab->sGeneratedPath;
	}
	if ( pLab->sMusicPath == NULL ) {
		pLab->sMusicPath = pLab->sGeneratedPath;
	}
	if ( pLab->sStreamPath == NULL ) {
		pLab->sStreamPath = pLab->sGeneratedPath;
	}
	iRet = xgeAudioInit();
	if ( iRet != XGE_OK ) {
		printf("audio-lab skip: xgeAudioInit ret=%d\n", iRet);
		return XGE_OK;
	}
	pLab->bAudioReady = xgeAudioIsReady();
	xgeAudioSetVolume(0.64f);
	pLab->fMasterVolume = xgeAudioGetVolume();
	memset(&tListener, 0, sizeof(tListener));
	tListener.tPosition.fX = 1.0f;
	tListener.tPosition.fY = 2.0f;
	tListener.tPosition.fZ = 3.0f;
	tListener.tForward.fZ = -1.0f;
	tListener.tUp.fY = 1.0f;
	xgeAudioListenerSet(&tListener);
	pLab->tListener = xgeAudioListenerGet();
	pLab->fListenerX = pLab->tListener.tPosition.fX;
	if ( xgeAudioGroupInit(&pLab->tGroup) != XGE_OK ) {
		fprintf(stderr, "audio-lab stage failed: xgeAudioGroupInit\n");
		return XGE_ERROR_AUDIO_FAILED;
	}
	pLab->bGroupReady = 1;
	xgeAudioGroupSetVolume(&pLab->tGroup, 0.72f);
	pLab->fGroupVolume = xgeAudioGroupGetVolume(&pLab->tGroup);
	xgeAudioGroupFade(&pLab->tGroup, 0.25f, 0.72f, 80);
	if ( xgeSoundFallbackSet(pLab->sGeneratedPath) == XGE_OK ) {
		pLab->bFallbackSet = 1;
	}
	if ( xgeSoundLoad(&pLab->tFallbackSound, "__audio_lab_missing__.wav") == XGE_OK ) {
		pLab->bFallbackLoad = (pLab->tFallbackSound.iFlags & XGE_SOUND_FALLBACK) ? 1 : 0;
	}
	if ( xgeSoundFallbackGet(&pLab->tFallbackCopy) == XGE_OK ) {
		pLab->bFallbackGet = (pLab->tFallbackCopy.iFlags & XGE_SOUND_FALLBACK) ? 1 : 0;
	}
	if ( xgeSoundLoad(&pLab->tSound, pLab->sSoundPath) == XGE_OK ) {
		pLab->bSoundLoaded = 1;
	}
	if ( xgeSoundLoadGroup(&pLab->tSoundGroup, pLab->sSoundPath, &pLab->tGroup) == XGE_OK ) {
		pLab->bSoundGroupLoaded = 1;
		if ( xgeSoundAddRef(&pLab->tSoundGroup) > 1 ) {
			pLab->bSoundAddRef = 1;
		}
		xgeSoundSetLoop(&pLab->tSoundGroup, 1);
		xgeSoundSetVolume(&pLab->tSoundGroup, 0.58f);
		xgeSoundSetPosition(&pLab->tSoundGroup, -1.0f, 0.0f, 0.25f);
		xgeSoundFade(&pLab->tSoundGroup, 0.15f, 0.58f, 80);
		pLab->bSoundPlay = (xgeSoundPlay(&pLab->tSoundGroup) == XGE_OK) && xgeSoundIsPlaying(&pLab->tSoundGroup);
		pLab->bSoundPause = (xgeSoundPause(&pLab->tSoundGroup) == XGE_OK) && (xgeSoundIsPlaying(&pLab->tSoundGroup) == 0);
		pLab->bSoundResume = (xgeSoundResume(&pLab->tSoundGroup) == XGE_OK);
		pLab->bSoundStop = (xgeSoundStop(&pLab->tSoundGroup) == XGE_OK);
	}
	if ( xgeMusicLoad(&pLab->tMusic, pLab->sMusicPath) == XGE_OK ) {
		pLab->bMusicLoaded = 1;
	}
	if ( xgeMusicLoadGroup(&pLab->tMusicGroup, pLab->sMusicPath, &pLab->tGroup) == XGE_OK ) {
		pLab->bMusicGroupLoaded = 1;
		xgeMusicSetLoop(&pLab->tMusicGroup, 1);
		xgeMusicSetVolume(&pLab->tMusicGroup, 0.46f);
		xgeMusicFade(&pLab->tMusicGroup, 0.10f, 0.46f, 80);
		pLab->bMusicPlay = (xgeMusicPlay(&pLab->tMusicGroup) == XGE_OK) && xgeMusicIsPlaying(&pLab->tMusicGroup);
		pLab->bMusicPause = (xgeMusicPause(&pLab->tMusicGroup) == XGE_OK) && (xgeMusicIsPlaying(&pLab->tMusicGroup) == 0);
		pLab->bMusicResume = (xgeMusicResume(&pLab->tMusicGroup) == XGE_OK);
		pLab->bMusicStop = (xgeMusicStop(&pLab->tMusicGroup) == XGE_OK);
	}
	if ( xgeStreamOpen(&pLab->tStream, pLab->sStreamPath) == XGE_OK ) {
		pLab->bStreamLoaded = 1;
	}
	if ( xgeStreamOpenGroup(&pLab->tStreamGroup, pLab->sStreamPath, &pLab->tGroup) == XGE_OK ) {
		pLab->bStreamGroupLoaded = 1;
		xgeStreamSetLoop(&pLab->tStreamGroup, 1);
		xgeStreamSetVolume(&pLab->tStreamGroup, 0.40f);
		xgeStreamSetPosition(&pLab->tStreamGroup, 1.0f, 0.0f, -0.25f);
		xgeStreamFade(&pLab->tStreamGroup, 0.08f, 0.40f, 80);
		pLab->bStreamPlay = (xgeStreamPlay(&pLab->tStreamGroup) == XGE_OK) && xgeStreamIsPlaying(&pLab->tStreamGroup);
		pLab->bStreamPause = (xgeStreamPause(&pLab->tStreamGroup) == XGE_OK) && (xgeStreamIsPlaying(&pLab->tStreamGroup) == 0);
		pLab->bStreamResume = (xgeStreamResume(&pLab->tStreamGroup) == XGE_OK);
		pLab->bStreamStop = (xgeStreamStop(&pLab->tStreamGroup) == XGE_OK);
	}
	if ( pLab->bAudioReady && pLab->bGroupReady && pLab->bFallbackSet && pLab->bFallbackLoad && pLab->bFallbackGet &&
		pLab->bSoundLoaded && pLab->bSoundGroupLoaded && pLab->bSoundAddRef && pLab->bSoundPlay && pLab->bSoundPause && pLab->bSoundResume && pLab->bSoundStop &&
		pLab->bMusicLoaded && pLab->bMusicGroupLoaded && pLab->bMusicPlay && pLab->bMusicPause && pLab->bMusicResume && pLab->bMusicStop &&
		pLab->bStreamLoaded && pLab->bStreamGroupLoaded && pLab->bStreamPlay && pLab->bStreamPause && pLab->bStreamResume && pLab->bStreamStop ) {
		return XGE_OK;
	}
	fprintf(stderr, "audio-lab stage failed: sound=%d/%d play=%d/%d/%d/%d music=%d/%d play=%d/%d/%d/%d stream=%d/%d play=%d/%d/%d/%d fallback=%d/%d/%d\n",
		pLab->bSoundLoaded, pLab->bSoundGroupLoaded, pLab->bSoundPlay, pLab->bSoundPause, pLab->bSoundResume, pLab->bSoundStop,
		pLab->bMusicLoaded, pLab->bMusicGroupLoaded, pLab->bMusicPlay, pLab->bMusicPause, pLab->bMusicResume, pLab->bMusicStop,
		pLab->bStreamLoaded, pLab->bStreamGroupLoaded, pLab->bStreamPlay, pLab->bStreamPause, pLab->bStreamResume, pLab->bStreamStop,
		pLab->bFallbackSet, pLab->bFallbackLoad, pLab->bFallbackGet);
	return XGE_ERROR_AUDIO_FAILED;
}

static void CleanupAudioLab(audio_lab_t* pLab)
{
	xgeStreamClose(&pLab->tStreamGroup);
	xgeStreamClose(&pLab->tStream);
	xgeMusicFree(&pLab->tMusicGroup);
	xgeMusicFree(&pLab->tMusic);
	if ( pLab->bSoundAddRef ) {
		xgeSoundFree(&pLab->tSoundGroup);
		pLab->bSoundAddRef = 0;
	}
	xgeSoundFree(&pLab->tSoundGroup);
	xgeSoundFree(&pLab->tSound);
	xgeSoundFree(&pLab->tFallbackCopy);
	xgeSoundFree(&pLab->tFallbackSound);
	xgeSoundFallbackClear();
	if ( pLab->bGroupReady ) {
		xgeAudioGroupFree(&pLab->tGroup);
		pLab->bGroupReady = 0;
	}
	if ( pLab->bAudioReady ) {
		xgeAudioUnit();
		pLab->bAudioReady = 0;
	}
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	audio_lab_t tLab;
	int iRet;

	memset(&tLab, 0, sizeof(tLab));
	ParseArgs(&tLab, argc, argv);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 1;
	tDesc.iHeight = 1;
	tDesc.sTitle = "XGE Audio Lab";
	tDesc.iFlags = 0;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 0;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("audio-lab init failed: xgeInit ret=%d\n", iRet);
		return 1;
	}
	iRet = InitAudioLab(&tLab);
	if ( iRet != XGE_OK ) {
		CleanupAudioLab(&tLab);
		xgeUnit();
		return 2;
	}
	if ( tLab.bAudioReady ) {
		xgeSleep((uint32_t)(tLab.iFrameLimit > 0 ? tLab.iFrameLimit : 1));
	}
	printf("audio-lab summary generated=%d ready=%d master=%.2f listener_x=%.1f group=%.2f fallback=(%d,%d,%d) sound=(load=%d group=%d addref=%d play=%d pause=%d resume=%d stop=%d) music=(load=%d group=%d play=%d pause=%d resume=%d stop=%d) stream=(load=%d group=%d play=%d pause=%d resume=%d stop=%d) path=%s\n",
		tLab.bGenerated,
		tLab.bAudioReady,
		tLab.fMasterVolume,
		tLab.fListenerX,
		tLab.fGroupVolume,
		tLab.bFallbackSet,
		tLab.bFallbackLoad,
		tLab.bFallbackGet,
		tLab.bSoundLoaded,
		tLab.bSoundGroupLoaded,
		tLab.bSoundAddRef,
		tLab.bSoundPlay,
		tLab.bSoundPause,
		tLab.bSoundResume,
		tLab.bSoundStop,
		tLab.bMusicLoaded,
		tLab.bMusicGroupLoaded,
		tLab.bMusicPlay,
		tLab.bMusicPause,
		tLab.bMusicResume,
		tLab.bMusicStop,
		tLab.bStreamLoaded,
		tLab.bStreamGroupLoaded,
		tLab.bStreamPlay,
		tLab.bStreamPause,
		tLab.bStreamResume,
		tLab.bStreamStop,
		tLab.sGeneratedPath);
	CleanupAudioLab(&tLab);
	xgeUnit();
	return 0;
}
