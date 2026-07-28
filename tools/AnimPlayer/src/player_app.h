/*
 * player_app.h - AnimPlayer application state
 */

#ifndef PLAYER_APP_H
#define PLAYER_APP_H

#include "xanim.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct player_app_t {
	xanim_player pPlayer;
	char     sFilePath[512];
	int      bPlaying;
	int      bLoop;
	float    fSpeed;
	int      iCurrentFrame;
	int      iFrameCount;
	int      bShowInfo;
	int      bShowControls;
	int      bFullscreen;
} player_app_t;

int  playerAppInit(player_app_t* pApp, const char* sFile);
void playerAppShutdown(player_app_t* pApp);
int  playerAppUpdate(player_app_t* pApp, float fDelta);

void playerAppTogglePlay(player_app_t* pApp);
void playerAppStop(player_app_t* pApp);
void playerAppStepForward(player_app_t* pApp);
void playerAppStepBackward(player_app_t* pApp);
void playerAppGotoFirst(player_app_t* pApp);
void playerAppGotoLast(player_app_t* pApp);
void playerAppToggleLoop(player_app_t* pApp);
void playerAppSetSpeed(player_app_t* pApp, float fSpeed);
void playerAppCycleSpeed(player_app_t* pApp);

int  playerAppHandleKey(player_app_t* pApp, int iKey);
int  playerAppLoadFile(player_app_t* pApp, const char* sPath);

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_APP_H */
