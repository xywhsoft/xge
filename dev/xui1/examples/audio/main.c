#include "../../xge.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	xge_audio_group_t tGroup;
	xge_sound_t tSound;
	const char* sPath;
	int iRet;

	if ( argc < 2 ) {
		printf("usage: xge_audio <sound.wav|sound.ogg|sound.mp3>\n");
		return 1;
	}
	sPath = argv[1];

	tDesc.iWidth = 1;
	tDesc.iHeight = 1;
	tDesc.sTitle = "XGE Audio";
	tDesc.iFlags = 0;
	tDesc.iRunMode = XGE_RUN_MANUAL;
	tDesc.iTargetFPS = 0;
	tDesc.pNativeWindow = 0;
	tDesc.pUser = 0;

	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xgeInit failed: %d\n", iRet);
		return 2;
	}

	iRet = xgeAudioInit();
	if ( iRet != XGE_OK ) {
		printf("xgeAudioInit failed: %d\n", iRet);
		xgeUnit();
		return 3;
	}

	iRet = xgeAudioGroupInit(&tGroup);
	if ( iRet != XGE_OK ) {
		printf("xgeAudioGroupInit failed: %d\n", iRet);
		xgeUnit();
		return 4;
	}
	xgeAudioGroupSetVolume(&tGroup, 0.8f);

	iRet = xgeSoundLoadGroup(&tSound, sPath, &tGroup);
	if ( iRet != XGE_OK ) {
		printf("xgeSoundLoadGroup failed: %d\n", iRet);
		xgeAudioGroupFree(&tGroup);
		xgeUnit();
		return 5;
	}

	xgeSoundSetLoop(&tSound, 1);
	xgeSoundSetPosition(&tSound, 0.0f, 0.0f, 0.0f);
	iRet = xgeSoundPlay(&tSound);
	if ( iRet != XGE_OK ) {
		printf("xgeSoundPlay failed: %d\n", iRet);
		xgeSoundFree(&tSound);
		xgeAudioGroupFree(&tGroup);
		xgeUnit();
		return 6;
	}

	printf("playing: %s\npress Enter to stop...\n", sPath);
	getchar();

	xgeSoundStop(&tSound);
	xgeSoundFree(&tSound);
	xgeAudioGroupFree(&tGroup);
	xgeUnit();
	return 0;
}
